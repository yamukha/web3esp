#ifndef WEB3E_H
#define WEB3E_H

#include <DefineEth.h>
#include <RpcEth.h>

#ifdef ESP32_MODEL
#include <WiFi.h>
#include <HTTPClient.h>
#else
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#endif

#include <Eth.h>

#include <string>

class Web3 {
public:
        Web3(){/*Eth eth;*/};
        Web3(WiFiClient client, std::string provider, int id) : wifi_(client), provider_(provider), id_counter_(id)
        {
          log_printf("Provider: %s rpc id: %d\n", provider_.c_str(), id); 
          Eth et(client, provider, id);
          eth = et;
        };

    bool setProvider(std::string provider) {provider_ = provider; return 0;};
    std::string currentProvider() {return provider_;};

    Eth eth;
    std::string provider_;
    WiFiClient wifi_;
    int id_counter_;

};
#endif

