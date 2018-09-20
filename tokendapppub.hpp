#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/types.hpp>
#include <eosiolib/action.hpp>
#include <eosiolib/symbol.hpp>
#include <eosiolib/time.hpp>
#include <eosiolib/singleton.hpp>
#include <eosiolib/transaction.hpp>
#include <eosiolib/currency.hpp>

#include <math.h>

using namespace eosio;
using namespace std;

#define SN(X) (string_to_symbol(0, #X) >> 8)

const account_name BANK_RESERVES = N(bankreserves);
const account_name GOD_ACCOUNT = N(godofdapppub);
const symbol_name PUB_SYMBOL_NAME = SN(PUB);
const symbol_type PUB_SYMBOL = S(4, PUB);
const asset NEW_GAME_CONSOME = asset(1000000, PUB_SYMBOL);
const uint32_t MAX_PERIOD = 100ul * 365 * 24 * 60 * 60;

class tokendapppub: public contract {
public:
    tokendapppub(account_name self):
            contract(self)
    {};
    void reg(account_name from, string memo);
    void buy(account_name from, account_name to, asset quantity, string memo);
    void sell(account_name from, asset quantity);
    void consume(account_name from, asset quantity, string memo);
    void hellodapppub(asset base_eos_quantity, asset maximum_stake, asset option_quantity, uint32_t lock_up_period,
                      uint8_t base_fee_percent, uint8_t init_fee_percent, uint64_t refer_fee, uint32_t start_time);
    void destroy(string name);
    void claim(string name, bool sell);
    void newtoken(account_name from, asset base_eos_quantity, asset maximum_stake, asset option_quantity,
                  uint32_t lock_up_period, uint8_t base_fee_percent, uint8_t init_fee_percent,
                  uint64_t refer_fee, uint32_t start_time);
    void transfer(account_name from, account_name to, asset quantity, string memo);
    void receipt(account_name from, string type, asset in, asset out, asset fee);
    void detail(string tokenname, string dappname, string logo, string website, string social,
                string community, string medium, string github, account_name contract, string memo);
    void setreferfee(string name_str, uint64_t refer_fee);
    void settrans(string name_str, uint64_t trans);
    // for eosio.token
    void create(account_name issuer, asset maximum_supply);
    void issue(account_name to, asset quantity, string memo);
private:
    void reserve(asset quantity) {
        action(
            permission_level{_self, N(active)},
            N(eosio.token),
            N(transfer),
            make_tuple(_self, BANK_RESERVES, quantity * 85 / 100, string("tokendapppub reserve https://dapp.pub"))
        ).send();
    }

    // @abi table stat i64
    struct cur_stats {
        asset          supply;
        asset          max_supply;
        account_name   issuer;

        uint64_t primary_key()const { return supply.symbol.name(); }
    };
    typedef eosio::multi_index<N(stat), cur_stats> stats;

    symbol_name _string_to_symbol_name(const char* str) {
        return string_to_symbol(0, str) >> 8;
    }
    // @abi table games i64
    struct st_game {
        symbol_type symbol;
        account_name owner;
        int128_t base_eos;
        int64_t base_stake;
        int64_t base_option;
        int64_t deserved_option;
        int64_t claimed_option;
        int128_t eos;
        int64_t stake;
        uint32_t lock_up_period;
        uint32_t start_time;
        uint8_t base_fee_percent;
        uint8_t init_fee_percent;

        uint8_t _fee_percent() {
            if ((init_fee_percent == base_fee_percent) || (now() >= start_time + lock_up_period)) {
                return base_fee_percent;
            }
            const double INIT = init_fee_percent;
            const double BASE = base_fee_percent;
            const double NOW = now();
            const double START = start_time;
            const double PERIOD = lock_up_period;

            return uint8_t(ceil(2 * PERIOD * (INIT - BASE) / ((NOW-START) + PERIOD) + 2 * BASE - INIT));
        }

        void _profit_eos(int64_t eos_amount) {
            eosio_assert(eos_amount > 0, "amount of EOS profit should be bigger than 0");
            eosio_assert(stake < base_stake, "cannot profit when no one holds stake");
            const double BASE_STAKE = base_stake;
            const double BASE_EOS = base_eos;
            const double STAKE = stake;
            const double EOS = eos;
            const double PROFIT = eos_amount;
            eos = int128_t((EOS - BASE_EOS + PROFIT) * BASE_STAKE / (BASE_STAKE - STAKE));
            base_eos = int128_t((EOS - BASE_EOS + PROFIT) * STAKE / (BASE_STAKE - STAKE));
        }

        void _issue_stake(int64_t stake_amount) {
            eosio_assert(stake_amount > 0, "amount of stake issuance should be bigger than zero");

            if (stake == base_stake) {
                base_stake += stake_amount;
                stake += stake_amount;
                claimed_option += stake_amount;
                return;
            }

            eosio_assert(stake < base_stake, "stake should be less than base_stake");
            const double BASE_STAKE = base_stake;
            const double BASE_EOS = base_eos;
            const double STAKE = stake;
            const double EOS = eos;
            const double NEW_STAKE = stake_amount;
            eos = int128_t((BASE_STAKE + NEW_STAKE) * (EOS - BASE_EOS) / (BASE_STAKE + NEW_STAKE - STAKE));
            base_eos = int128_t((STAKE) * (EOS - BASE_EOS) / (BASE_STAKE + NEW_STAKE - STAKE));
            base_stake += stake_amount;
        }

        void _consume_stake(int64_t stake_amount) {
            eosio_assert(stake_amount > 0, "amount of comsumed stake should be bigger than zero");
            eosio_assert(stake + stake_amount < base_stake, "cannot comsume all remaining stake");
            const double BASE_STAKE = base_stake;
            const double BASE_EOS = base_eos;
            const double STAKE = stake;
            const double EOS = eos;
            const double CONSUME = stake_amount;
            eos = int128_t((EOS - BASE_EOS) * BASE_STAKE / (BASE_STAKE - STAKE - CONSUME));
            base_eos = int128_t((EOS - BASE_EOS) * (STAKE + CONSUME) / (BASE_STAKE - STAKE - CONSUME));
            stake += stake_amount;
        }

        void _update_option() {
            // no option or all options has been deserved.
            if ((base_option == 0) || (deserved_option == base_option)) {
                return;
            }

            int64_t last_deserved_option = deserved_option;
            
            const double NOW = now();
            const double START = start_time;
            const double PERIOD = lock_up_period;
            const double BASE = base_option;
            if ((NOW - START) >= PERIOD) {
                deserved_option = base_option;
            } else {
                deserved_option = int64_t(BASE * (NOW - START) / PERIOD);
            }

            if (deserved_option == last_deserved_option) {
                return;
            }

            int64_t new_deserved_option = deserved_option - last_deserved_option;
            _issue_stake(new_deserved_option);
        }

        void _check() {
            eosio_assert(base_eos > 0, "failed to check base_eos should be bigger than zero");
            eosio_assert(stake > 0, "failed to check stake should be bigger than zero");
            eosio_assert(eos >= base_eos, "failed to check eos is bigger than base_eos");
            eosio_assert(base_stake >= stake, "failed to check base_stake is bigger than stake");
        }

        void consume(int64_t stake_amount) {
            _update_option();
            _consume_stake(stake_amount);
            _check();
        }

        void profit(int64_t eos_amount) {
            _update_option();
            _profit_eos(eos_amount);
            _check();
        }

        int64_t buy(int64_t eos_amount) {
            _update_option();

            const double STAKE = double(stake);
            const double EOS = double(eos);
            const double IN  = double(eos_amount);

            int64_t stake_amount = int64_t((IN*STAKE) / (IN+EOS));

            eos += eos_amount;
            stake -= stake_amount;
            _check();
            return stake_amount;
        }

        int64_t sell(int64_t stake_amount) {
            _update_option();

            const double STAKE = double(stake);
            const double EOS = double(eos);
            const double IN  = double(stake_amount);

            int64_t eos_amount = int64_t((IN*EOS)/(STAKE+IN));

            eos -= eos_amount;
            stake += stake_amount;
            _check();
            return eos_amount;
        }

        int64_t fee(int64_t eos_amount) {
            int64_t fee = 0;
            int64_t fee_percent = _fee_percent();
            if (fee_percent > 0 && stake < base_stake) {
                fee = (eos_amount * fee_percent + 99) / 100;
            }
            return fee;
        }

        int64_t claim() {
            _update_option();
            int64_t new_claimed = deserved_option - claimed_option;
            claimed_option = deserved_option;
            _check();
            return new_claimed;
        }
    };
    typedef singleton<N(games), st_game> tb_games;

    void new_game(account_name owner, asset base_eos_quantity, asset maximum_stake, asset option_quantity,
                  uint32_t lock_up_period, uint8_t base_fee_percent, uint8_t init_fee_percent, uint32_t start_time) {
        symbol_type symbol = maximum_stake.symbol;
        eosio_assert(symbol.is_valid(), "invalid symbol name");
        eosio_assert(symbol == option_quantity.symbol, "maximum stake and option quantity should be the same symbol type");

        tb_games game_sgt(_self, symbol.name());
        eosio_assert(!game_sgt.exists(), "game has started before");
        eosio_assert(base_eos_quantity.symbol == CORE_SYMBOL, "base_eos must be core token");
        eosio_assert((base_eos_quantity.amount > 0) && (base_eos_quantity.amount <= 1000000000ll*10000), "invalid amount of base EOS pool");
        eosio_assert(maximum_stake.is_valid(), "invalid maximum stake");
        eosio_assert((maximum_stake.amount > 0) && (maximum_stake.amount <= 10000000000000ll*10000), "invalid amount of maximum supply");
        eosio_assert((option_quantity.amount >= 0) && (option_quantity.amount <= maximum_stake.amount), "invalid amount of option");
        eosio_assert(lock_up_period <= MAX_PERIOD, "invalid lock up period");
        eosio_assert((base_fee_percent >= 0) && (base_fee_percent <= 99), "invalid fee percent");
        eosio_assert((init_fee_percent >= base_fee_percent) && (init_fee_percent <=99), "invalid init fee percent");
        eosio_assert(start_time <= now() + 180 * 24 * 60 * 60, "the token issuance must be within six months");

        game_sgt.set(st_game{
            .symbol = symbol,
            .owner = owner,
            .base_stake = maximum_stake.amount - option_quantity.amount,
            .base_eos = base_eos_quantity.amount,
            .base_option = option_quantity.amount,
            .deserved_option = 0,
            .claimed_option = 0,
            .lock_up_period = lock_up_period,
            .eos = base_eos_quantity.amount,
            .stake = maximum_stake.amount - option_quantity.amount,
            .base_fee_percent = base_fee_percent,
            .init_fee_percent = init_fee_percent,
            .start_time = max(start_time, now()),
        }, owner);
    }

    asset game_buy(symbol_name name, int64_t eos_amount) {
        eosio_assert(eos_amount > 0, "eos amount should be bigger than 0");

        tb_games game_sgt(_self, name);
        eosio_assert(game_sgt.exists(), "game not found by this symbol name");

        st_game game = game_sgt.get();
        eosio_assert(now() > game.start_time, "the token issuance has not yet begun");

        int64_t stake_amount = game.buy(eos_amount);

        eosio_assert(stake_amount > 0, "stake amount should be bigger than 0");
        eosio_assert(stake_amount < game.base_stake, "stake amount overflow");

        game_sgt.set(game, game.owner);

        return asset(stake_amount, game.symbol);
    }

    tuple<asset, asset> game_sell(symbol_name name, int64_t stake_amount) {
        eosio_assert(stake_amount > 0, "stake amount should be bigger than 0");

        tb_games game_sgt(_self, name);
        eosio_assert(game_sgt.exists(), "game not found by this symbol_name");

        st_game game = game_sgt.get();
        eosio_assert(now() > game.start_time, "the token issuance has not yet begun");

        int64_t eos_amount = game.sell(stake_amount);
        eosio_assert(eos_amount > 0, "must reserve a positive amount");

        int64_t eos_fee = game.fee(eos_amount);
        eosio_assert(eos_fee >= 0, "fee amount must be a Non-negative");
        if (eos_fee > 0) {
            game.profit(eos_fee);
        }

        int64_t reserve_amount = eos_amount - eos_fee;
        eosio_assert(reserve_amount > 0, "must reserve a positive amount");

        game_sgt.set(game, game.owner);

        return make_tuple(asset(reserve_amount, CORE_SYMBOL), asset(eos_amount, CORE_SYMBOL));
    }

    asset game_claim(symbol_name name) {
        tb_games game_sgt(_self, name);
        eosio_assert(game_sgt.exists(), "game not found by this symbol_name");
        st_game game = game_sgt.get();
        eosio_assert(now() > game.start_time, "the token issuance has not yet begun");

        int64_t claimed = game.claim();
        eosio_assert(claimed > 0, "claimed stake should be bigger than zero");
        game_sgt.set(game, game.owner);
        return asset(claimed, game.symbol);
    }

    void game_consume(symbol_name name, int64_t stake_amount) {
        eosio_assert(stake_amount > 0, "consume stake amount should be bigger than 0");

        tb_games game_sgt(_self, name);
        eosio_assert(game_sgt.exists(), "game not found by this symbol_name");

        st_game game = game_sgt.get();
        eosio_assert(now() > game.start_time, "the token issuance has not yet begun");
        eosio_assert(stake_amount < game.base_stake - game.stake, "consume too much stake");

        game.consume(stake_amount);

        game_sgt.set(game, game.owner);

        require_recipient(game.owner);
    }

    void game_profit(symbol_name name, int64_t eos_amount) {
        eosio_assert(eos_amount > 0, "profit eos amount should be bigger than 0");

        tb_games game_sgt(_self, name);
        eosio_assert(game_sgt.exists(), "game not found by this symbol_name");

        st_game game = game_sgt.get();
        eosio_assert(now() > game.start_time, "the token issuance has not yet begun");
        eosio_assert(game.stake < game.base_stake, "cannot profit when no one holds stake");
        
        game.profit(eos_amount);

        game_sgt.set(game, game.owner);
    }

    // @abi table refer i64
    struct st_refer {
        uint64_t fee_percent;

        int64_t fee(int64_t eos_amount) {
            return int64_t((eos_amount * fee_percent + 9999) / 10000);
        }
    };
    typedef singleton<N(refer), st_refer> tb_refer;

    void set_refer_fee(symbol_name name, uint64_t refer_fee, account_name payer) {
        eosio_assert(refer_fee < 10000, "invalid refer fee");
        tb_refer refer_sgt(_self, name);
        eosio_assert(!refer_sgt.exists(), "cannot update exist refer fee");
        refer_sgt.set(st_refer{
            .fee_percent = refer_fee,
        }, payer);
    }

    // @abi table trans i64
    struct st_trans {
        uint64_t trans;
        bool transactable() {
            return trans == 1;
        }
    };
    typedef singleton<N(trans), st_trans> tb_trans;

    void set_trans(symbol_name name, uint64_t trans, account_name payer) {
        eosio_assert(trans == 0 || trans == 1, "trans should be bool");
        tb_trans trans_sgt(_self, name);
        trans_sgt.set(st_trans{
            .trans = trans,
        }, payer);
    }

    // @abi table accounts i64
    struct account {
        asset balance;
        uint64_t primary_key() const {return balance.symbol.name();}
    };
    typedef multi_index<N(accounts), account> accounts;
};

#ifdef ABIGEN
    EOSIO_ABI(tokendapppub, (settrans)(setreferfee)(detail)(issue)(create)(reg)(receipt)(transfer)(sell)(consume)(destroy)(claim)(newtoken)(hellodapppub))
#endif