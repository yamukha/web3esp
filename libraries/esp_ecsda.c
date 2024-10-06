// Linux use case test to create and sign eth raw transaction
// python3 ethraw.py [rpcId] [ethHost] [privKey]
// python3 ethraw.py 1 http://192.168.0.106:8545 4f3edf983ac636a65a842ce7c78d9aa706d3b113bce9c46f30d7d21715b23b1d
// ./ecdsa nonce chainId rpcId privKey
// ./ecdsa 0x1 0x539 1 4f3edf983ac636a65a842ce7c78d9aa706d3b113bce9c46f30d7d21715b23b1d | tail -n 1

#include <stdio.h>
#include <string.h>

#define FORCE_SMART_CONTRACT

#include "RpcEth/ethsign.h"
#include "RpcEth/JsonUtilsEth.h"
#include "Web3E/src/Contract.h"
#include "contracts/contract_g5.h"

auto cnt = 0LLU; // max. to encode is 1234567890123444556LLU;

int main(int argc, char *argv[])
{
    std::string nonce_str = "0x0";
    std::string chain_str = "0x539";
    std::string rpcid_str = "0";
    std::string key = "4f3edf983ac636a65a842ce7c78d9aa706d3b113bce9c46f30d7d21715b23b1d";
    std::string scFunc = "'get_output()'";
    std::string scArg1 = "42";
    std::string scArg2 = "3";
    bool isContract = false;

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
        if (i == 5) {
            scFunc = argv[i];
            isContract = true;
        }
        if (i == 6)
           scArg1 = argv[i];
        if (i == 7)
           scArg2 = argv[i];
    }

    RLP rlp;
    struct TX tx;
#ifdef FORCE_SMART_CONTRACT
    isContract = true; // true to forse smart contract
#endif
    if (!isContract) // simple Transaction
    {
        log_printf("Simple transaction mode\n");
        tx.nonce = nonce_str;
        tx.gasPrice = "0x04a817c800";
        tx.gasLimit = "0x1e8480";
        tx.to = "0xFFcf8FDEE72ac11b5c542428B35EEF5769C409f0"; // ETH address
        tx.value = "0x0386a0";
        tx.data = std::to_string(cnt).c_str();
        
    }
    else
    {
        log_printf("Smart contract mode\n");
        Contract c;
        std::vector <ScmSig> scmSig = c.abiParser(contract_g5);
        c.AllFunctions(scmSig);    
        //std::string m = c.buildMethod("%s(%s,%llu)","set_string","'some parameter maximum value = '", cnt);
        //std::string m = c.buildMethod("%s(%s)","set_string","'some parameter maximum value = '");
        //std::string m = c.buildMethod("%s(%llu,%llu)","set_output", 3, cnt);
        //std::string m = c.buildMethod("%s(%llu)","set_output", 3);
        std::string m = c.buildMethod("%s(%s)","set_bytes", "'one'");
        //std::string m = c.buildMethod("%s(%s)","set_bytes", "'hello bytes 42'");
        //std::string m = c.buildMethod("%s(%s)","set_bool", "true");
        //std::string m = c.buildMethod("%s", "get_output()";

        //std::string m = "set_string(\"some 'parameter maximum value = \",1234567890123444556)";
        // std::string m = "set_string(\"me\")";
        // std::string m = "set_output(3,42)";
        // std::string m = "set_output(3)";
        // std::string m = "set_bytes('one')";
        // std::string m = "set_bool(true)";
        //std::string m = "get_output()";

        log_printf("Called method: %s\n", m.c_str());

        auto fhash = c.funcHash(m);
        CallData cd = c.doCall(m);
        log_printf("Chain ID: %s Nonce: %s\n", chain_str.c_str(), nonce_str.c_str());
        std::string calldata = fhash + cd.stat + cd.dynamic;
        log_printf("Calldata: \n%s\nCalldata len: %lu\n", calldata.c_str(), calldata.size());

        //for (auto& item : scmSig) {
        //    auto i =  &item  - &scmSig[0];
        //   log_printf("method %ld:  %s\n", i ,item.fname.c_str());
        //}

        tx.nonce = nonce_str;
        tx.gasPrice = "0x77359400";  // 2000000000 -> 0x77359400
        tx.gasLimit = "0x1e8480"; // 2000000 -> "0x1e8480"
        // tx.to =   "0xe78a0f7e598cc8b0bb87894b0f60dd2a88d6a8ab";  // smart contract address
        // tx.to =   "0x254dffcd3277c0b1660f6d42efbb754edababc2b"; // smart contract address
        tx.to = "0xd9145CCE52D386f254917e481eB44e9943F39138"; // smart contract address
        tx.value = "0"; // zero for smart contract //"0x0386a0";
        tx.data = calldata; // size 200 is max
        tx.v = chain_str; // chainID; //"0x0539"; // chain_id + 35 + {0,1} is parity of y for curve point, where chain_id = 1337 for private chain; ref. https://github.com/ethereum/EIPs/blob/master/EIPS/eip-155.md
        tx.r = "0x00";    // then be calculated i.e. "0x5bdcbfcfd8b8d113b678bd34e8d2cc7cdcdd77e9c2189beafc5a64207fc53b3b";
        tx.s = "0x00";    // then be calculated i.e. "0x5c5b1c73e65e9a4e9c73b13e3825f517efcc35eac11958c7f314b57c39006738";
    }
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
