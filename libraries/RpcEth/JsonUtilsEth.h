#ifndef JSON_UTILS_ETH_H
#define JSON_UTILS_ETH_H

#include <string>

#ifdef USE_LINUX
#include <jsoncpp/json/json.h>
#endif

#ifdef USE_ARDUINO
#include <Arduino_JSON.h>
#endif

#include <DefineEth.h>

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
#endif

#endif
