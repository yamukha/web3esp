#ifndef JSON_UTILS_ETH_H
#define JSON_UTILS_ETH_H

#include <string>
#include <DefineEth.h>

#ifdef USE_JSON_OR_JSON11
#ifdef USE_JSON11

#ifdef USE_LINUX
typedef std::string String;
#include "../json11/json11.hpp"
#endif
#ifdef USE_ARDUINO
#include <json11.hpp>
#endif

constexpr char usedJson [] = "json11";

String ethJson(std::string method, std::string param, uint64_t id_cnt)
{
   String js;

   if ("" != param)
   {
      json11::Json payload = json11::Json::object ({
        {"jsonrpc", "2.0"},
        {"id", (double)id_cnt},
        {"method", method},
        {"params", json11::Json::array{param}},
      });
      js = payload.dump().c_str();
   }  else {
      json11::Json payload = json11::Json::object ({
        {"jsonrpc", "2.0"},
        {"id", (double)id_cnt},
        {"method", method},
      });
      js = payload.dump().c_str();
   }

#ifdef DEBUG_PRINT
         log_printf("used library: %s\n", usedJson);
         //log_printf("raw tx: %s\n", js.c_str());
#endif
   return js;
}

std::string parseJsonResult(const String &payload) {
    std::string w = "";
    std::string err;
    std::string payload_ = payload.c_str();
    auto jt = json11::Json::parse (payload_,err);
    if (jt.object_items().count("result") > 0)
      w = jt["result"].string_value();
    return w;
}

#else

#ifdef USE_LINUX
typedef std::string String;
#include "../json/json.hpp"
#endif
#ifdef USE_ARDUINO
#include <json.hpp>
#endif
constexpr char usedJson [] = "nlohmann";

String ethJson(std::string method, std::string param, uint64_t id_cnt)
{
   String js;
   nlohmann::json payload;

   payload["jsonrpc"] = "2.0";
   payload["id"] = id_cnt;
   payload["method"] = method;
   if ("" != param)
   {
      payload["params"] [0] = param;
   }

   js = payload.dump().c_str();

#ifdef DEBUG_PRINT
         log_printf("used library: %s\n", usedJson);
         //log_printf("raw tx: %s\n", js.c_str());
#endif
   return js;
}

std::string parseJsonResult(const String &payload) {
   std::string w = "";
    std::string payload_ = payload.c_str();
    auto jt =  nlohmann::json::parse (payload_);
    if (jt.contains("result")) 
       w = jt["result"];
    return w;
}

#endif

#else

#ifdef USE_LINUX
#include <jsoncpp/json/json.h>
#endif

#ifdef USE_ARDUINO
#include <Arduino_JSON.h>
#endif

#ifdef USE_LINUX
typedef std::string String;
String ethJson(std::string method, std::string param, uint64_t id_cnt)
{
   String js;
   // JSONVar params;
   Json::Value payload;
   payload["jsonrpc"] = "2.0";
   payload["id"] = (double)id_cnt;
   payload["method"] = method;
   if ("" != param)
   {
      Json::Value &params = payload["params"];
      params.append(param);
   }
   Json::FastWriter writer;
   js = writer.write(payload);
   if (!js.empty() && js[js.length() - 1] == '\n')
   {
      js.erase(js.length() - 1);
   }
   return js;
}
#endif

#ifdef USE_ARDUINO
String ethJson(std::string method, std::string param, uint64_t id_cnt)
{
   String js;
   JSONVar params;
   if ("" != param)
   {
      params[0] = param.c_str();
   }

   JSONVar payload;
   payload["jsonrpc"] = "2.0";
   payload["id"] = (double)id_cnt; // to increment
   payload["method"] = method.c_str();
   if ("" != param)
   {
      payload["params"] = params;
   }
   js = JSON.stringify(payload);
   return js;
}

// parse smth. like: {"id":3,"jsonrpc":"2.0","result":""}
std::string parseJsonResult(const String &payload)
{
   std::string result = "";
   JSONVar myObject = JSON.parse(payload);
   if (JSON.typeof(myObject) == "undefined")
   {
      log_printf("Parsing input failed!"); // TODO log_err
   }
   else
   {
      if (myObject.hasOwnProperty("result"))
      {
#ifdef DEBUG_PRINT
         log_printf("json result: %s\n", (const char *)myObject["result"]);
#endif
         result = std::string((const char *)myObject["result"]);
      }
   }
   return result;
}
//  USE_ARDUINO
#endif
// USE_JSON_OR_JSON11
#endif
// guard
#endif
