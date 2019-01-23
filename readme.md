# tokendapppub: a bancor based utility-enhanced token protocol

* Read the Whitepaper at first: [English](https://github.com/Dappub/tokendapppub/blob/master/Bancor%20based%20utility-enhanced%20token%20protocol.pdf)/[Chinese](https://github.com/Dappub/tokendapppub/blob/master/%E5%9F%BA%E4%BA%8E%E7%8F%AD%E6%9F%AF%E7%9A%84%E5%AE%9E%E7%94%A8%E5%A2%9E%E5%BC%BA%E5%9E%8B%E9%80%9A%E8%AF%81%E5%8D%8F%E8%AE%AE.pdf)
* Introduction about this DApp on Medium: [tokendapppub](https://medium.com/@DappPub/tokendapppub-1-b6143c6039e3)

### Usage (buy,sell,transfer,publish)

publish your DApp token by yourself. And then, you can buy/sell and transfer your token on the website.

### Buy Example

```js
contract: eosio.token
action: transfer
data: {
    "to": "tokendapppub",
    "from": "yourself",
    "quantity": "10.0000 EOS",
    "memo": "TPT-referrer:godofdapppub"
}
```

### Sell Example

```js
contract: tokendapppub
action: sell
data: {
    "from": "yourself",
    "quantity": "100.0000 PUB"
}
```



### Consume Example

If you are a developer and you want to pushlish a utility token for consuming on your DApp. after you publish your token, you should wirte a contract to do this. after you set your code on EOS mainnet, when user consume your token, we will notify your contract by `require_receiption` in tokendapppub contract. The consume Richard Contract is [here](https://github.com/Dappub/tokendapppub/blob/master/tokendapppub.consume_rc.md).

* example.hpp

```c++
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
```

* example.cpp

```c++
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
```

### Token Exchange

[![tokenpocket](https://raw.githubusercontent.com/Dappub/logo/master/partners/tpdex-small.png)](https://tpdex.io/#/)

![bitportal](https://raw.githubusercontent.com/Dappub/logo/master/partners/bitportal.png)

![51token](https://raw.githubusercontent.com/Dappub/logo/master/partners/51token.png)

### Block Explorer

[![eospark](https://raw.githubusercontent.com/Dappub/tokendapppub/master/logo/EOSpark.png)](https://eospark.com/MainNet/account/tokendapppub)

[![eosflare](https://raw.githubusercontent.com/Dappub/tokendapppub/master/logo/eosflare-logo-512.png)](https://eosflare.io/account/tokendapppub)
