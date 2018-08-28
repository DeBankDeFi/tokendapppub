#include <boost/test/unit_test.hpp>
#include <eosio/testing/tester.hpp>
#include <eosio/chain/abi_serializer.hpp>
#include "Runtime/Runtime.h"
#include "eosio.contracts/tests/eosio.system_tester.hpp"

#include <fc/variant_object.hpp>
#include "tokendapppub_test.hpp"

using namespace eosio::testing;
using namespace eosio;
using namespace eosio::chain;
using namespace eosio::testing;
using namespace fc;
using namespace std;

using mvo = fc::mutable_variant_object;

#define SN(X) (string_to_symbol(0, #X) >> 8)

const account_name GOD_ACCOUNT = N(godofdapppub);
const symbol_name PUB_SYMBOL_NAME = SN(PUB);
const symbol_type PUB_SYMBOL = S(4, PUB);
const asset NEW_GAME_CONSOME = asset(1000000, PUB_SYMBOL);

class tokendapppub_test : public eosio_system::eosio_system_tester {
	public:

		tokendapppub_test() {
			produce_blocks(2);

			const asset large_asset = core_from_string("80.0000");
			create_account_with_resources( N(tokendapppub), config::system_account_name, core_from_string("10.0000"), false, large_asset, large_asset );

			produce_blocks(2);

			set_code( N(tokendapppub), tokendapppub_wasm() );
			set_abi( N(tokendapppub), tokendapppub_abi().data() );

			produce_blocks();

			set_authority(
					N(tokendapppub),
					"active",
					authority(
						1,
						vector<key_weight>{{get_private_key("eosio", "active").get_public_key(), 1}},
						vector<permission_level_weight>{{{N(tokendapppub), config::eosio_code_name}, 1}}
						),
					"owner",
					{ { N(tokendapppub), "active" } }, { get_private_key( N(tokendapppub), "active" ) }
					);

			const auto& accnt = control->db().get<account_object,by_name>( N(tokendapppub) );
			abi_def abi;
			BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
			abi_ser.set_abi(abi, abi_serializer_max_time);
		}

		void transfer( name from, name to, const asset& amount, string memo, name manager = config::system_account_name ) {
			base_tester::push_action(N(eosio.token), N(transfer), manager, mvo()
					("from", from)
					("to", to)
					("quantity", amount)
					("memo", memo)
					);
		}

        action_result hellodapppub(asset base_eos_quantity, asset maximum_stake, asset option_quantity, uint32_t lock_up_period, uint8_t base_fee_percent, uint8_t init_fee_percent) {
            return base_tester::push_action(N(tokendapppub), N(hellodapppub),
                vector<permission_level>{{N(godofdapppub),config::active_name}},
                mvo()
                    ("base_eos_quantity", base_eos_quantity)
                    ("maximum_stake", maximum_stake)
                    ("option_quantity", option_quantity)
                    ("lock_up_period", lock_up_period)
                    ("base_fee_percent", base_fee_percent)
                    ("init_fee_percent", init_fee_percent)
            )
        }

		abi_serializer abi_ser;
};

BOOST_AUTO_TEST_SUITE(tokendapppub_tests)

	BOOST_FIXTURE_TEST_CASE( newtoken_tests, tokendapppub_test ) try {
		const asset large_asset = core_from_string("80.0000");
		create_account_with_resources( N(godofdapppub), config::system_account_name, core_from_string("10.0000"), false, large_asset, large_asset );
		produce_blocks();
        BOOST_REQUIRE_EQUAL(success(), hellodapppub(core_from_string("10000.0000"), asset(100000000, PUB_SYMBOL), asset(0, PUB_SYMBOL), 0, 0, 0));
        produce_blocks();
    } FC_LOG_AND_RETHROW()

BOOST_AUTO_TEST_SUITE_END()