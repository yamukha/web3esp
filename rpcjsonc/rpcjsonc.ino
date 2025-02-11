#include <string>
#include <cstdlib>

#include <Arduino.h>

#include <DefineEth.h>
#include <RpcEth.h>
#include <Web3e.h>
#include <ethsign.h>
#include <Contract.h>
#include <contract_g5.h>

#ifdef ESP32_MODEL
#include <WiFi.h>
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

std::string chainID = "";
int cnt = 0;
Web3 web3;
bool web3init = false;
Contract c;
std::vector<ScmSig> scmSig = c.abiParser(contract_g5);

void setup()
{
  Serial.begin(115200);
  delay(100);

#ifdef ESP32_MODEL
  esp_task_wdt_init(WDT_TIMEOUT, true);
  esp_task_wdt_add(NULL);
#else
  ESP.wdtEnable(WDT_TIMEOUT);
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

  log_printf("used library: %s\n", usedJson);

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

  c.AllFunctions(scmSig);
}

void loop()
{
  Serial.printf("WiFi ");
  if ((WiFi.status() == WL_CONNECTED))
  {
    log_printf("rpc id  %d\n", cnt);
    WiFiClient client;

    if (!web3init) 
    {
      Web3 web(client, ETHURL, cnt);
      web3 = web;
      // Prepare ETH Accounts
      web3.eth.accounts.privateKeyToAccount(PRIV_ETH_KEY);
      auto accs = web3.eth.getAccounts();
      if (accs.size() > 0)
      {
        log_printf("Accounts count %d, default candidate %s\n", accs.size(), accs[0].c_str());
        web3.eth.defaultAccount = accs[0];
        log_printf("ETH defaultAccount(): %s\n", web3.eth.defaultAccount.c_str());
      }
      else {
        log_printf("\nNo any ETH accounts found\n");
      }
      web3init = true;
    }

    std::string nonce = "";

    if ("" == chainID)
    {
      chainID = web3.eth.getChainId();
      if ("" == chainID)
        log_printf("!Cannot get chainID!\n");
    };

    if ("" != chainID) {
      nonce = web3.eth.getTransactionCount(web3.eth.defaultAccount);
    }

    int sign_result = 0;
    std::string raw_transaction = "";

    if ("" != nonce)
    {
      // web3.eth.signTransaction (tx, address);
      // where: address is signer address to get it's pub and secret keys from accounts storage
      //        tx is transaction object:
      struct TX tx;
      bool isContract = true;
      if (!isContract) // simple Transaction
      {
        tx.nonce = nonce;
        tx.gasPrice = "0x04a817c800";
        tx.to = "0xFFcf8FDEE72ac11b5c542428B35EEF5769C409f0"; // ETH address
        tx.value = "0x0386a0";
        tx.data = std::to_string(cnt).c_str();
        tx.v = chainID;
        raw_transaction = web3.eth.signTransaction(tx, web3.eth.defaultAccount);
      }
      else // smart contract call
      {
        std::string m = c.buildMethod("%s(%s,%s)", "set_allowed", "0xD028ec274Ef548253a90c930647b74C830Ed4b4F", "true");
        // std::string m = c.buildMethod("%s(%s,%llu)", "set_string", "'some parameter maximum value = '", cnt);
        // std::string m = c.buildMethod("%s(%s)","set_bytes", "[0 1 0x2 0x03 4 5 6 7 8 9 10 11 12 13 14 15 16 0x11 18 19 20 21 22 23 24 25 26 27 28 29 30 fg 0x100 256 31]");
        // std::string bytes =  "[" + std::to_string(cnt%256) + " " + std::to_string(cnt/256) + " 1 0x2 0x03 4 5 6 7 8 9 10 11 12 13 14 15 16 0x11 18 19 20 21 22 23 24 25 26 27 28 29 30 fg 0x100]";
        // std::string m = c.buildMethod("%s(%s)","set_bytes", bytes.c_str());
        log_printf("Called method: %s\n", m.c_str());

        tx.nonce = nonce;
        tx.gasPrice = "0x77359400";  // 2000000000 -> 0x77359400
        tx.to = "0xd9145CCE52D386f254917e481eB44e9943F39138"; // smart contract address
        tx.data = c.doCall(m); // size 200 is max
        tx.v = chainID; // chainID; //"0x0539"; // chain_id + 35 + {0,1} is parity of y for curve point, where chain_id = 1337 for private chain; ref. https://github.com/ethereum/EIPs/blob/master/EIPS/eip-155.md
        raw_transaction = web3.eth.signTransaction (tx,  web3.eth.defaultAccount);
      }
      log_printf("Is contract %d, nonce: %s, raw transaction:\n%s\n", isContract, nonce.c_str(), raw_transaction.c_str());
    }
    else
      log_printf("!Cannot get nonce!\n");

    if (raw_transaction != "")
    {
      std::string resRpc = web3.eth.sendSignedTransaction(raw_transaction);
    }
    else
      log_printf("!Cannot send transaction!\n");

    delay(1000);
    cnt++;
  }
}
