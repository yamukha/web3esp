// Linux use case test to create and sign eth raw transaction
// python3 ethraw.py [rpcId] [ethHost] [privKey]
// python3 ethraw.py 1 http://192.168.0.106:8545 4f3edf983ac636a65a842ce7c78d9aa706d3b113bce9c46f30d7d21715b23b1d
// ./ecdsa nonce chainId rpcId privKey
// ./ecdsa 0x1 0x539 1 4f3edf983ac636a65a842ce7c78d9aa706d3b113bce9c46f30d7d21715b23b1d | tail -n 1

#include <stdio.h>
#include <string.h>

#include "RpcEth/ethsign.h"
#include "RpcEth/JsonUtilsEth.h"

int main(int argc, char *argv[])
{
    std::string nonce_str = "0x0";
    std::string chain_str = "0x539";
    std::string rpcid_str = "0";
    std::string key = "4f3edf983ac636a65a842ce7c78d9aa706d3b113bce9c46f30d7d21715b23b1d";

    for (int i = 0; i < argc; ++i)
    {
        if (i == 1)
            nonce_str = argv[i];
        if (i == 2)
            chain_str = argv[i];
        if (i == 3)
            rpcid_str = argv[i];
        if (i == 4)
            key = argv[i];
    }

    log_printf("Chain ID: %s Nonce: %s\n", chain_str.c_str(), nonce_str.c_str());

    RLP rlp;
    struct TX tx;
    tx.nonce = nonce_str;
    tx.gasPrice = "0x04a817c800";
    tx.gasLimit = "0x1e8480";
    tx.to = "0xFFcf8FDEE72ac11b5c542428B35EEF5769C409f0";
    tx.value = "0x0386a0";
    tx.data = "0";
    tx.v = chain_str; // chainID; //"0x0539"; // chain_id + 35 + {0,1} is parity of y for curve point, where chain_id = 1337 for private chain; ref. https://github.com/ethereum/EIPs/blob/master/EIPS/eip-155.md
    tx.r = "0x00";    // then be calculated i.e. "0x5bdcbfcfd8b8d113b678bd34e8d2cc7cdcdd77e9c2189beafc5a64207fc53b3b";
    tx.s = "0x00";    // then be calculated i.e. "0x5c5b1c73e65e9a4e9c73b13e3825f517efcc35eac11958c7f314b57c39006738";

    uint8_t privkey[KEYS_SIZE];
    rlp.hex2bin(key.c_str(), (char *)privkey);
#ifdef DUMMY_KEYS
    log_printf("privkey:\n");
    print_hex(privkey, KEYS_SIZE);
    log_printf("\n");
#endif
    uint8_t pubkey[64];
    if (ethPubKey(pubkey, privkey) > 0)
    {
        log_printf("public_key() OK:\n");
        print_hex(pubkey, PUB_KEY_SIZE);
        log_printf("\n");
    }
    else
    {
        log_printf("\npublic key err.:\n");
    }

    std::string eth_account = ethAddr(privkey);
    log_printf("address by eth_account(): %s\n", eth_account.c_str());

    std::string enc_raw = ethSign(tx.nonce, chain_str, tx.gasPrice, tx.gasLimit, tx.to, tx.value, tx.data, privkey, pubkey);
    log_printf("\nRaw transaction:\n%s\n", enc_raw.c_str());

    log_printf("'%s'\n", ethJson("eth_sendRawTransaction", enc_raw, std::atoi(rpcid_str.c_str())).c_str());

    // Send raw transaction and receive respoce
    // curl 192.168.0.103:7545 -X POST -H "Content-Type: application/json" --data '{"jsonrpc":"2.0","method":"eth_sendRawTransaction","params":[""]}'
}
