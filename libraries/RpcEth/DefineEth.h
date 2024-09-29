#ifndef DEFINE_ETH_H
#define DEFINE_ETH_H

#ifndef USE_LINUX
#define USE_JSON11
// #define USE_JSON
#endif

#ifdef USE_JSON11
#define USE_JSON_OR_JSON11
#undef USE_JSON
#endif
#ifdef USE_JSON
#define USE_JSON_OR_JSON11
#undef USE_JSON11
#endif

#ifndef USE_LINUX
#define USE_ARDUINO

#ifdef USE_ARDUINO
// ESP32 or ES8266 in other case
// #define ESP32_MODEL
#endif
#ifdef ESP32_MODEL
// #define USE_TREZOR
// #define USE_SECP256K1
#define USE_UECC
#else
// Only uECC is supported for ESP8266
#define USE_UECC
#endif
// for Linux
#else
#define USE_UECC
// #define USE_SECP256K1
// #define USE_TREZOR -- TREZOR is not supported for Linux platform
#endif

#ifndef LOG_PRINTF
#ifdef USE_ARDUINO
#define log_printf(...) Serial.printf(__VA_ARGS__)
#else
#define log_printf(...) printf(__VA_ARGS__)
#endif
#endif

#define WDT_TIMEOUT 3000
// SW random generator based on rand() else HW based on ADC for ESP
#define SW_RNG

#define KEYS_SIZE 32
#define SIGNATURE_SIZE 64
#define HASH_OFFSET 12
#define ETH_ADDR_SIZE 20

#define PUB_KEY_SIZE 64
#define PUB_KEY_SIZE_65 65
#define ETHURL "http://192.168.0.106:8545"
// #define DEBUG_PRINT

#endif