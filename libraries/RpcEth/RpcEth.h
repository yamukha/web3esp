#ifndef RPC_ETH_H
#define RPC_ETH_H

#include <DefineEth.h>

#ifdef ESP32_MODEL
#include <WiFi.h>
#include <HTTPClient.h>
#else
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#endif

#include <string>
#include <JsonUtilsEth.h>

std::string rpcEth (WiFiClient wifi, std::string url, std::string method, std::string param, uint64_t cnt) {
     HTTPClient http;    
     http.begin(wifi, url.c_str());
     http.addHeader("Content-Type", "application/json");
     log_printf("[HTTP]+POST:\n"); 
     JSONVar params; 
     String jsonString;
     std::string rpc_result = "";
      
     /*
     if ( "" == param){
       jsonString = ethJsonNoParam(method, cnt); 
     } else { 
       jsonString = ethJsonWithParam(method, param, cnt);
     }
     */
     jsonString = ethJson(method, param, cnt);
     Serial.println(jsonString);      

     // Send raw transaction ETH by curl ... -data '{"jsonrpc":"2.0","method":"eth_sendRawTransaction","params":["0x.."]}'
     int httpCode = http.POST(jsonString);

     if (httpCode > 0) {
         log_printf("[HTTP]+POST code: %d\n", httpCode);
         if (httpCode == HTTP_CODE_OK) {
              const String& payload = http.getString();
              Serial.println(payload);
              std::string rpc_result = parseJsonResult(payload);
              return rpc_result;
         }
    }
    return rpc_result;
}

// EthRpc rpcProvider(client, eth_url, PRIV_KEY, SS58_ADR, id_counter);
// EthsRpc (WiFiClient client, std::string url, std::string key, std::string ss58adr, uint64_t id)
#endif