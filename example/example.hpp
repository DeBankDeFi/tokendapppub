#include <eosiolib/eosio.hpp>
#include <eosiolib/symbol.hpp>
#include <eosiolib/asset.hpp>

#include <math.h>

using namespace eosio;
using namespace std;

const symbol_type EXAMPLE_SYMBOL = S(4, EXAMPLE);

class example: public contract {
public:
    example(account_name self):
            contract(self)
    {};
    void consume(account_name from, asset quantity, string memo);
    void receipt(account_name from, asset quantity);
};

#ifdef ABIGEN
    EOSIO_ABI(example, (receipt))
#endif