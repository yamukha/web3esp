// Linux use case test to create and sign eth raw transaction
// python3 ethraw.py [rpcId] [ethHost] [privKey]
// python3 ethraw.py 1 http://192.168.0.106:8545 4f3edf983ac636a65a842ce7c78d9aa706d3b113bce9c46f30d7d21715b23b1d
// ./ecdsa nonce chainId rpcId privKey
// ./ecdsa 0x1 0x539 1 4f3edf983ac636a65a842ce7c78d9aa706d3b113bce9c46f30d7d21715b23b1d | tail -n 1

#include <random>
#include <stdio.h>
#include <string.h>

#define FORCE_SMART_CONTRACT
// #define LOW_LEVEL_SIGNER

#include "RpcEth/ethsign.h"
#include "RpcEth/JsonUtilsEth.h"
#include "Web3E/src/Contract.h"
#include "Web3E/src/Eth.h"
#include "Web3E/src/Web3e.h"
#include "contracts/contract_g5.h"

#ifdef DUMMY_KEYS
#include "RpcEth/PrivateEth_dummy.h"
#else
#include "RpcEth/PrivateEth.h"
#endif

// auto cnt = 0LLU; // max. to encode is 1234567890123444556LLU;

int main(int argc, char *argv[])
{
    std::string nonce = "0x0";
    std::string chainID = "0x539";
    std::string rpcid_str = "0";
    std::string key = PRIV_ETH_KEY;
    std::string scFunc = "'get_output()'";
    std::string scArg1 = "42";
    std::string scArg2 = "3";
    bool isContract = false;

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> distr(0, 256*256 - 1);

    int cnt =  distr(rng);

    for (int i = 0; i < argc; ++i)
    {
        if (i == 1)
            nonce = argv[i];
        if (i == 2)
            chainID = argv[i];
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
        tx.nonce = nonce;
        tx.gasPrice = "0x04a817c800";
        tx.to = "0xFFcf8FDEE72ac11b5c542428B35EEF5769C409f0"; // ETH address
        tx.value = "0x0386a0";
        tx.data = std::to_string(cnt).c_str();
        tx.v = chainID;
    }
    else
    {
        log_printf("Smart contract mode\n");
        Contract c;
        std::vector <ScmSig> scmSig = c.abiParser(contract_g5);
        c.AllFunctions(scmSig);
        std::string m = c.buildMethod("%s(%s,%s)", "set_allowed", "0xD028ec274Ef548253a90c930647b74C830Ed4b4F", "true");
        //std::string m = c.buildMethod("%s(%s,%llu)","set_string","'some parameter maximum value = '", cnt);
        //std::string m = c.buildMethod("%s(%s)","set_string","'some parameter maximum value = '");
        //std::string m = c.buildMethod("%s(%llu,%llu)","set_output", 3, cnt);
        //std::string m = c.buildMethod("%s(%llu)","set_output", 3);
        // std::string m = c.buildMethod("%s(%s)","set_bytes", "'one'");
        //std::string m = c.buildMethod("%s(%s)","set_bytes", "'hello bytes 42'");
        // std::string bytes = "[" + std::to_string(cnt/256) + " " + std::to_string(cnt%256) + " 0 1 0x2 0x03 4 5 6 7 8 9 10 11 12 13 14 15 16 0x11 18 19 20 21 22 23 24 25 26 27 28 29 30 fg 0x100 256 31]";
        // std::string bytes = "[" + std::to_string(cnt/256) + " " + std::to_string(cnt%256) + " 0 1 0x2 0x03 4 5 6 7 8 9 10 11]";   
        // log_printf("Bytes: %s len:%ld\n", bytes.c_str(), bytes.size());
        // std::string m = c.buildMethod("%s(%s)","set_bytes", bytes.c_str());
        //std::string m = c.buildMethod("%s(%s)","set_bytes", "[0 1 0x2 0x03 4 5 6 7 8 9 10 11 12 13 14 15 16 0x11 18 19 20 21 22 23 24 25 26 27 28 29 30 fg 0x100 256 31]");
        //std::string m = c.buildMethod("%s(%s)","set_bool", "true");
        //std::string m = c.buildMethod("%s", "get_output()";

        // std::string m = "set_string(\"some 'parameter maximum value = \",1234567890123444556)";
        // std::string m = "set_string(\"me\")";
        // std::string m = "set_output(3,42)";
        // std::string m = "set_output(3)";
        // std::string m = "set_bytes('one')";
        // std::string m = "set_bytes([a 1 0 0xr2 0x3 0xff 0x7 00 127 000 255 256 0x09 xx])";
        // std::string m = "set_bytes([0 1 0x2 0x03 4 5 6 7 8 9 10 11 12 13 14 15 16 0x11 18 19 20 21 22 23 24 25 26 27 28 29 30 fg 0x100 256 31])";
        // std::string m = "set_bool(true)";
        // std::string m = "get_output()";

        log_printf("Called method: %s\n", m.c_str());

        //log_printf("Chain ID: %s Nonce: %s\n", chainID.c_str(), nonce.c_str());
        tx.nonce = nonce;
        tx.gasPrice = "0x77359400";  // 2000000000 -> 0x77359400
        tx.to =   "0xe78a0f7e598cc8b0bb87894b0f60dd2a88d6a8ab";  // smart contract address
        // tx.to =   "0x3254dffcd3277c0b1660f6d42efbb754edababc2b"; // smart contract address
        // tx.to = "0xD028ec274Ef548253a90c930647b74C830Ed4b4F";
        tx.data = c.doCall(m); // size 200 is max
        tx.v = chainID; // chain_id + 35 + {0,1} is parity of y for curve point, where chain_id = 1337 for private chain; ref. https://github.com/ethereum/EIPs/blob/master/EIPS/eip-155.md
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

#ifdef LOW_LEVEL_SIGNER
    std::string raw_transaction = ethSign(tx.nonce, tx.v, tx.gasPrice, tx.gasLimit, tx.to, tx.value, tx.data, privkey, pubkey);
    log_printf("\nRaw eth low level signed transaction:\n%s\n", raw_transaction.c_str());
    log_printf("'%s'\n", ethJson("eth_sendRawTransaction", raw_transaction, std::atoi(rpcid_str.c_str())).c_str());
#else
    Eth eth;
    Web3 web3(eth);
    // Prepare ETH Accounts
    web3.eth.accounts.privateKeyToAccount(key);
    auto accs = web3.eth.getAccounts();
    if (accs.size() > 0)
      {
        log_printf("Accounts count %d, default candidate %s\n", (int)accs.size(), accs[0].c_str());
        web3.eth.defaultAccount = accs[0];
        log_printf("ETH defaultAccount(): %s\n", web3.eth.defaultAccount.c_str());
        std::string raw_transaction = web3.eth.signTransaction (tx,  web3.eth.defaultAccount);
        log_printf("\nRaw eth signed transaction:\n%s\n", raw_transaction.c_str());
        log_printf("'%s'\n", ethJson("eth_sendRawTransaction", raw_transaction, std::atoi(rpcid_str.c_str())).c_str());
      }
      else {
        log_printf("\nNo any ETH accounts found\n");
      }
#endif

    // Send raw transaction and receive respoce
    // curl 192.168.0.103:7545 -X POST -H "Content-Type: application/json" --data '{"jsonrpc":"2.0","method":"eth_sendRawTransaction","params":[""]}'
}
