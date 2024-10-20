#ifndef WEB3E_H
#define WEB3E_H

#ifndef USE_LINUX

#include <DefineEth.h>
#include <RpcEth.h>
#include <Eth.h>

#ifdef ESP32_MODEL
#include <WiFi.h>
#include <HTTPClient.h>
#else
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#endif

#else
#include "../RpcEth/DefineEth.h"
#include "../RpcEth/RpcEth.h"
#include "Eth.h"
#endif

#include <string>

class Web3
{
public:
  Web3() {};
  Web3(Eth et) { eth = et; };
#ifndef USE_LINUX
  Web3(WiFiClient client, std::string provider, int id) : wifi_(client), provider_(provider), id_counter_(id)
  {
    log_printf("Provider: %s rpc id: %d\n", provider_.c_str(), id);
    Eth et(client, provider, id);
    eth = et;
  };
#endif

  bool setProvider(std::string provider)
  {
    provider_ = provider;
    return 0;
  };
  std::string currentProvider() { return provider_; };

  Eth eth;
  std::string provider_;
#ifndef USE_LINUX
  WiFiClient wifi_;
#endif
  int id_counter_;
};
#endif
