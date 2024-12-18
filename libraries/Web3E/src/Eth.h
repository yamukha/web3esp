#ifndef ETH_PROVIDER_H
#define ETH_PROVIDER_H

#ifndef USE_LINUX

#ifdef ESP32_MODEL
#include <WiFi.h>
#include <HTTPClient.h>
#else
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#endif

#include <Accounts.h>
#include <DefineEth.h>
// ethSign()
#include <ethsign.h>

#else
#include "Accounts.h"
#include "../RpcEth/DefineEth.h"
// ethSign()
#include "../RpcEth/ethsign.h"
#endif

#include <string>
#include <vector>
#include <algorithm>

class Eth
{
public:
  Eth() {};
#ifndef USE_LINUX
  Eth(WiFiClient client, std::string provider, int id) : wifi_(client), provider_(provider), cnt_(id) {
                                                         };
#endif
  std::string getChainId()
  {
    std::string chain_id = "1";
#ifndef USE_LINUX
    chain_id = rpcEth(wifi_, provider_, "eth_chainId", "", cnt_);
#endif
    cnt_++;
    return chain_id;
  };
  std::string getTransactionCount(std::string eth_account)
  {
    std::string nonce = "0";
#ifndef USE_LINUX
    nonce = rpcEth(wifi_, provider_, "eth_getTransactionCount", eth_account, cnt_);
#endif
    cnt_++;
    return nonce;
  };
  std::string sendSignedTransaction(std::string raw_transaction)
  {
    std::string hash = "";
#ifndef USE_LINUX
    hash = rpcEth(wifi_, provider_, "eth_sendRawTransaction", raw_transaction, cnt_);
#endif
    cnt_++;
    return hash;
  };
  std::string signTransaction(std::string& nonce, std::string &chain, std::string &price, std::string &limit, std::string &to, std::string &value, std::string &data, std::string &address)
  {
    std::string raw_transaction = "";
    for (auto const &x : accounts.storage_)
    {
      if (x.first == address)
      {
        log_printf("Signer %s\n", x.first.c_str());
        uint8_t prik[KEYS_SIZE] = {0};
        std::vector<uint8_t> priK = {0};
        priK.assign(x.second.priv_key_, x.second.priv_key_ + KEYS_SIZE);
        std::copy(priK.begin(), priK.end(), prik);

        uint8_t pubk[PUB_KEY_SIZE] = {0};
        std::vector<uint8_t> pubK = {0};
        pubK.assign(x.second.pub_key_, x.second.pub_key_ + PUB_KEY_SIZE);
        std::copy(pubK.begin(), pubK.end(), pubk);

        raw_transaction = ethSign(nonce, chain, price, limit, to, value, data, prik, pubk);
        return raw_transaction;
      }
    }
    return "";
  };
  std::string signTransaction(struct TX &tx, std::string &address)
  {
    return signTransaction(tx.nonce, tx.v, tx.gasPrice, tx.gasLimit, tx.to, tx.value, tx.data, address);
  };

  Accounts accounts;
  std::vector<std::string> getAccounts()
  {
    accounts_.clear();
    for (auto const &x : accounts.storage_)
      accounts_.push_back(x.second.address_);
    return accounts_;
  };

  std::string defaultAccount = ""; // some ETH address
  std::vector<std::string> accounts_;
  std::string provider_;
#ifndef USE_LINUX
  WiFiClient wifi_;
#endif
  int cnt_ = 0;
};
#endif
