#include <string>
#include <cstdlib>

#include <Arduino.h>

#include <DefineEth.h>
#include <RpcEth.h>
#include <ethsign.h>

#ifdef ESP32_MODEL
#include "WiFi.h"
#include <HTTPClient.h>
#include <esp_task_wdt.h>
#include <esp_random.h>
#else
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#endif

// WiFi credentials and private keys are hidden in Private.h file
// #define DUMMY_KEYS
#ifdef DUMMY_KEYS
#include <PrivateEth_dummy.h>
#else
#include <PrivateEth.h>
#endif

#define MIN_URL_SIZE 7
#define READ_UART_TIMEOUT 3

std::string chainID = "";
uint8_t priv_key[KEYS_SIZE] = {0};
uint8_t pub_key[PUB_KEY_SIZE] = {0};
int cnt = 0;
std::string eth_account = "";

void setup()
{
  Serial.begin(115200);
  delay(100);
  Serial.println("\nAwaiting new URL for " + String(READ_UART_TIMEOUT) + " s");

#ifdef ESP32_MODEL
  esp_task_wdt_init(WDT_TIMEOUT, true);
  esp_task_wdt_add(NULL);
#else
  ESP.wdtEnable(WDT_TIMEOUT);
#endif

  std::vector<uint8_t> pkb = hex2bytes(PRIV_ETH_KEY);
  std::copy(pkb.begin(), pkb.end(), priv_key);
#ifdef DUMMY_KEYS
  log_printf("\nprivate key:\n");
  print_hex(priv_key, KEYS_SIZE);
  log_printf("\n");
#endif

#ifdef USE_UECC
  uECC_set_rng(&RNG);
  log_printf("Used uECC \n");
#endif
#ifdef USE_SECP256K1
  log_printf("Used SECP256K1 \n");
#endif
#ifdef USE_TREZOR
  log_printf("Used Trezor \n");
#endif
  if (ethPubKey(pub_key, priv_key) > 0)
  {
    log_printf("public_key() OK:\n");
    print_hex(pub_key, PUB_KEY_SIZE);
    log_printf("\n");
  }
  else
  {
    log_printf("public key err.:\n");
  }
  eth_account = ethAddr(priv_key);
  log_printf("Eth address: %s", eth_account.c_str());

  Serial.printf("\nConnecting\n");
  WiFi.begin(STASSID, STAPSK);
  Serial.printf("Trying to connected to SSID: %s \n", STASSID);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nConnected and get IP address: ");
  Serial.println(WiFi.localIP());
  delay(5);
}

void loop()
{
  Serial.printf("WiFi ");
  if ((WiFi.status() == WL_CONNECTED))
  {
    log_printf("rpc id  %d\n", cnt);
    WiFiClient client;

    std::string nonce = "";

    if ("" == chainID)
    {
      chainID = rpcEth(client, ETHURL, "eth_chainId", "", cnt);
      cnt++;
    };

    nonce = rpcEth(client, ETHURL, "eth_getTransactionCount", eth_account, cnt);
    cnt++;

    int sign_result = 0;
    std::string raw_transaction = "";

    if ("" != nonce)
    {
      std::string price = "0x04a817c800";
      std::string limit = "0x1e8480";
      std::string to = "0xFFcf8FDEE72ac11b5c542428B35EEF5769C409f0";
      std::string value = "0x0386a0";
      std::string data = "";

      raw_transaction = ethSign(nonce, chainID, price, limit, to, value, data, priv_key, pub_key);
      log_printf("Nonce: %s, Raw transaction:\n%s\n", nonce.c_str(), raw_transaction.c_str());
    }

    if (raw_transaction != "")
    {
      std::string resRpc = rpcEth(client, ETHURL, "eth_sendRawTransaction", raw_transaction, cnt);
      cnt++;
    }
    delay(1000);
  }
}
