//
// Created by James Brown on 2018/09/13.
//

#ifndef ARDUINO_WEB3_CRYPTO_H
#define ARDUINO_WEB3_CRYPTO_H

#include "Trezor/secp256k1.h"
#include "Trezor/ecdsa.h"
#include <vector>
#include <string.h>

#include <stdint.h>


using namespace std;

int signECDSA(uint8_t *privateKey, uint8_t *digest, uint8_t *result);

#endif // ARDUINO_WEB3_CRYPTO_H
