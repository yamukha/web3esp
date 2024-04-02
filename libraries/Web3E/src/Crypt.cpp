//
// Created by James Brown on 2018/09/13.
//

// #include <Arduino.h>
#include "Crypt.h"
//#include "Util.h"
#include <Trezor/secp256k1.h>
#include <Trezor/ecdsa.h>
#include <vector>
#include <string.h>

#define SIGNATURE_LENGTH 64

using namespace std;

int signECDSA(uint8_t *privateKey, uint8_t *digest, uint8_t *result)
// if (!uECC_sign(privkey, (const uint8_t *)hash, sizeof(hash), sig, curve))
{
    const ecdsa_curve *curve = &secp256k1;
    uint8_t pby;
    int res = 0;
    bool allZero = true;

    for (int i = 0; i < 32; i++)
        if (privateKey[i] != 0)
            allZero = false;
    // for (int i = 0; i < ETHERS_PRIVATEKEY_LENGTH; i++) if (privateKey[i] != 0) allZero = false;
    for (int i = 0; i < 32; i++) if (privateKey[i] != 0) allZero = false;
    if (allZero == true)
    {
        //Serial.println("Private key not set, generate a private key (eg use Metamask) and call Contract::SetPrivateKey with it.");
    }
    else
    {
        res = ecdsa_sign_digest(curve, privateKey, digest, result, &pby, NULL);
        result[64] = pby;
        res = 1;
    }

    return res;
}
