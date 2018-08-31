#include "example.hpp"

void example::consume(account_name from, asset quantity, string memo) {
    require_auth(from);
    eosio_assert(quantity.is_valid(), "invalid quantity");
    eosio_assert(quantity.symbol == EXAMPLE_SYMBOL, "symbol mismatch");

    // TODO: your business logic

    action(
            permission_level{_self, N(active)},
            _self,
            N(receipt),
            make_tuple(from, quantity)
    ).send();
}

void example::receipt(account_name from, asset quantity) {
    require_auth(_self);
}

extern "C" {
    void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
        example thiscontract(receiver);

        if((code == N(tokendapppub)) && (action == N(consume))) {
            execute_action(&thiscontract, &example::consume);
            return;
        }

        if (code != receiver) return;

        switch (action) {
            EOSIO_API(example, (receipt))
        };
        eosio_exit(0);
    }
}