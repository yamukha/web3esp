/* wrapper for secp256k1 library based on ethc 
*  https://github.com/mhw0/libethc
*/
#include <stdint.h>

#ifndef ETHC_H
#define ETHC_H

struct eth_ecdsa_signature {
  uint8_t r[32];
  uint8_t s[32];
  int recid;
};

int ecdsa_sign_secp256k1(struct eth_ecdsa_signature *dest, const uint8_t *privkey, const uint8_t *bytes32);
int ecdsa_get_pubkey_secp256k1(uint8_t *pubKey, const uint8_t *privkey);

#endif