#ifndef ETH_SIGH_H
#define ETH_SIGH_H

#include <string>
#include <algorithm>

#include <DefineEth.h>
#include <hexutils.h>
#include <RLP.h>
#include <sha3.h>

#ifdef ESP32_MODEL
#include <esp_random.h>
#endif

#ifdef USE_TREZOR
#include <Crypt.h>
#endif

#ifdef USE_UECC
#include <uECC.h>
const struct uECC_Curve_t *curve = uECC_secp256k1();
#ifdef USE_LINUX
static int RNG(uint8_t *dest, unsigned size)
{
  *dest = 123;
  return 1;
}
#else

#ifdef ESP32_MODEL
#define RND_ADC_PORT 34
#else
#define RND_ADC_PORT 0
#endif

#ifdef SW_RNG
static int RNG(uint8_t *dest, unsigned size)
{
  uint8_t irnd = 123;
#ifdef ESP32_MODEL
  irnd = (uint8_t)esp_random();
  // irnd = (uint8_t) random(256);
#else
  irnd = (uint8_t)random(256);
#endif
  *dest = irnd;
  return 1;
}
#else

// to speed up can be used hw rnd wodule or return fixed number or pseudo random
static int RNG(uint8_t *dest, unsigned size)
{
  // Use the least-significant bits from the ADC for an unconnected pin (or connected to a source of
  // random noise). This can take a long time to generate random data if the result of analogRead(0)
  // doesn't change very frequently.
  while (size)
  {
    uint8_t val = 0;
    for (unsigned i = 0; i < 8; ++i)
    {
      int init = analogRead(RND_ADC_PORT);
      int count = 0;
      while (analogRead(RND_ADC_PORT) == init)
      {
        ++count;
      }

      if (count == 0)
      {
        val = (val << 1) | (init & 0x01);
      }
      else
      {
        val = (val << 1) | (count & 0x01);
      }
    }
    *dest = val;
    ++dest;
    --size;
  }
  // NOTE: it would be a good idea to hash the resulting random data using SHA-256 or similar.
  return 1;
}
#endif

#endif

std::string subBigHex(std::string a, std::string b)
{
  std::string s;
  std::reverse(a.begin(), a.end());
  std::reverse(b.begin(), b.end());
  uint8_t carry = 0;

  for (int i = 0; i < a.length(); i++)
  {
    std::string x{a[i]};
    std::string y{b[i]};
    char *pX;
    char *pY;
    int ai = std::strtol(x.c_str(), &pX, 16) - carry;
    int bi = std::strtol(y.c_str(), &pY, 16);
    uint8_t c = 0;
    if (ai < bi)
    {
      c = 16 + ai - bi;
      carry = 1;
    }
    else
    {
      c = ai - bi;
      carry = 0;
    }
    char ch[sizeof(int) * 4 + 1];
    sprintf(ch, "%x", c);
    // log_printf("\n %c - %c => %s (%d - %d)", a[i], b[i], ch, ai, bi);
    std::string dig(ch);

    s.append(ch);
  }
  std::reverse(s.begin(), s.end());
  log_printf("%s\n", s.c_str());
  return s;
}

#endif

#ifdef USE_SECP256K1
#ifdef USE_LINUX
#include "../secp256k1-embedded/src/ethc.h"
#else
#include <ethc.h>
#endif
struct eth_ecdsa_signature sign;
#endif

std::string ethSign(std::string nonce, std::string chain, std::string price, std::string limit, std::string to, std::string value, std::string data, uint8_t priv_key[KEYS_SIZE], uint8_t pub_key[PUB_KEY_SIZE])
{
  int sign_result = 0;
  RLP rlp;
  struct TX tx;
  std::string rsv = "";

  tx.nonce = nonce;
  tx.gasPrice = price;
  tx.gasLimit = limit;
  tx.to = to;
  tx.value = value;
  tx.data = data;

  tx.v = chain;
  tx.r = "0x00"; // then be calculated i.e. "0x5bdcbfcfd8b8d113b678bd34e8d2cc7cdcdd77e9c2189beafc5a64207fc53b3b";
  tx.s = "0x00"; // then be calculated i.e. "0x5c5b1c73e65e9a4e9c73b13e3825f517efcc35eac11958c7f314b57c39006738";

  std::string enc = rlp.bytesToHex(rlp.encode(tx, true)); // initial PLR with chain ID, r = 0, s =0
  std::string header = rlp.LengthHeader(enc);
  std::string ser = header + enc;

#ifdef DEBUG_PRINT
  log_printf("RPL of unsigned transaction, no header: size = %ld\n0x%s\n", strlen(enc.c_str()), enc.c_str());
  log_printf("RPL encoded len : 0x%s\n", header.c_str());
  log_printf("RPL of unsigned transaction with header: size = %ld\n0x%s", strlen(ser.c_str()), ser.c_str());
#endif

  uint8_t hash256[KEYS_SIZE];
  uint8_t buf[256];
  rlp.hex2bin(ser.c_str(), (char *)buf);
  do_keccak256(hash256, buf, strlen(ser.c_str()) / 2);

#ifdef DEBUG_PRINT
  log_printf("\nhash_keccak:\n");
  print_hex(hash256, KEYS_SIZE);
  log_printf("\n");
#endif

  std::string r = "";
  std::string s = "";
  int recid = 0;
  int recid1 = 0;

#ifdef USE_SECP256K1
#ifdef DEBUG_PRINT
  log_printf("SECP256K1 ecdsa_sign_secp256k1():");
#endif
  int sign_err = ecdsa_sign_secp256k1(&sign, priv_key, (const uint8_t *)hash256);
  if (1 > sign_err)
  {
    log_printf("Cannot sign with ecdsa_sign_secp256k1() %d\n", sign_err);
  }
  else
  {
#ifdef DEBUG_PRINT
    log_printf("\nrecid %d\n", sign.recid);
    print_hex(sign.r, KEYS_SIZE);
    print_hex(sign.s, KEYS_SIZE);
    log_printf("\nSigned by secp256k1\n");
#endif

    r = arr_to_hex_str(sign.r, KEYS_SIZE);
    s = arr_to_hex_str(sign.s, KEYS_SIZE);
    recid = sign.recid;
    sign_result = 1;
  }
#endif

#ifdef USE_UECC
  uECC_set_rng(&RNG);
  const struct uECC_Curve_t *curve = uECC_secp256k1();
  uint8_t sig[PUB_KEY_SIZE_65] = {0};
#ifdef DEBUG_PRINT
  log_printf("uECC_sign():");
#endif
  if (!uECC_sign(priv_key, hash256, sizeof(hash256), sig, curve))
  {
    log_printf("uECC_sign() failed\n");
  }
  else
  {
#ifdef DEBUG_PRINT
    print_hex(sig, PUB_KEY_SIZE);
#endif
  }
  if (!uECC_verify(pub_key, hash256, sizeof(hash256), sig, curve))
  {
    log_printf("\nuECC_verify() failed\n");
  }
  else
  {
    r = arr_to_hex_str(sig, KEYS_SIZE);
    s = arr_to_hex_str(&sig[KEYS_SIZE], KEYS_SIZE);
    recid = 0; // if LSB of R.Y is is even then 0 else 1
    if (sig[PUB_KEY_SIZE] % 2 != 0)
    {
      recid = 1;
    }

    // https://www.rapidtables.com/convert/number/hex-to-decimal.html
    // https://www.boxentriq.com/code-breaking/big-number-calculator
    // SECP256k1.order   = FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141
    //                     115792089237316195423570985008687907852837564279074904382605163141518161494337
    // SECP256k1.order/2 = 7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF5D576E7357A4501DDFE92F46681B20A0
    //                     57896044618658097711785492504343953926418782139537452191302581570759080747168
    // if (s > n/2) -> normalise s = n - s
    if (s > "7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF5D576E7357A4501DDFE92F46681B20A0")
    {
      log_printf("\ns is bigger then SECP256k1.order, normalized s:\n");
      std::string N = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141";
      std::string s_old = s;
      s = subBigHex(N, s_old);
      recid = recid ^ 1;
      log_printf("recid flipped to %d\n", recid);
    }

#ifdef DEBUG_PRINT
    log_printf("\nuECC_verify() OK\n");
    log_printf("recig: %d \n", recid);
#endif
    sign_result = 1;
  }
#endif

#ifdef USE_TREZOR
  uint8_t sign_ecdsa[PUB_KEY_SIZE_65] = {0};
#ifdef DEBUG_PRINT
  log_printf("Trezor signECDSA():");
#endif
  if (!signECDSA(priv_key, hash256, sign_ecdsa))
  {
    log_printf("\nCannot sign with signECDSA()\n");
  }
  else
  {
    //log_printf("\nSigned by Trezor\n");
    r = arr_to_hex_str(sign_ecdsa, KEYS_SIZE);
    s = arr_to_hex_str(&sign_ecdsa[KEYS_SIZE], KEYS_SIZE);
    recid = sign_ecdsa[PUB_KEY_SIZE];
    sign_result = 1;
  }
#endif
  if (0 != sign_result)
  {
    char buffer[sizeof(int) * 8 + 1];

    // Convert hex like "0x539" to int from i.e. {"jsonrpc":"2.0","result":"0x539"}
    char *pEnd;
    int chain_id_int = std::strtol(chain.c_str(), &pEnd, 16);
    int chain_id0 = chain_id_int * 2 + 35 + recid;
    sprintf(buffer, "0x%x", chain_id0);

#ifdef DEBUG_PRINT
    log_printf("Final v: %s recid: %d\n", buffer, recid);
    log_printf("Final r: %s\n", r.c_str());
    log_printf("Final s: %s\n", s.c_str());
#endif
    // set r and s instead of zeros and calculated v
    tx.r = r.c_str();
    tx.s = s.c_str();
    tx.v = buffer;
    if (tx.r.size() > PUB_KEY_SIZE || tx.s.size() > PUB_KEY_SIZE)
    {
      log_printf("\nr or s is more then 32\n");
      return rsv;
    }

    // Final rlp encode with updated  v,s,r
    std::string enc_raw = rlp.bytesToHex(rlp.encode(tx, false));
#ifdef DEBUG_PRINT
    log_printf("\nRPL final transaction:\n0x%s\n", enc_raw.c_str());
#endif
    rsv = "0x" + enc_raw;
    return rsv;
  }
  else
  {
    log_printf("Raw transacion creation failed\n");
    return rsv;
  }
}

// get ETH account address from pub key hash
std::string eth_address(uint8_t *pub_hash)
{
  uint8_t hashed[20];
  memcpy(hashed, &pub_hash[HASH_OFFSET], ETH_ADDR_SIZE);
  std::string eth_addr = arr_to_hex_str(hashed, ETH_ADDR_SIZE);
  // logf("\neth address %s\n", eth_addr.c_str());
  return "0x" + eth_addr;
}

// get ETH public key from private key
int ethPubKey(uint8_t *pub_key, uint8_t *priv_key)
{
  int ret = -1;
#ifdef USE_SECP256K1
#ifdef DEBUG_PRINT
  log_printf("Used secp256k1 ");
#endif
  if (ecdsa_get_pubkey_secp256k1(pub_key, priv_key) > 0)
  {
#ifdef DEBUG_PRINT
    log_printf("pub key\n");
    print_hex(pub_key, PUB_KEY_SIZE);
#endif
    return 1;
  }
  else
  {
    log_printf("ecdsa_get_pubkey_secp256k1() error!\n");
  }
#endif

#ifdef USE_TREZOR
#ifdef DEBUG_PRINT
  log_printf("Used Trezor ");
#endif
  const ecdsa_curve *trezor_curve = &secp256k1;
  uint8_t t_pub_key[PUB_KEY_SIZE_65] = {0};
  ecdsa_get_public_key65(trezor_curve, priv_key, t_pub_key); // is void in lib
#ifdef DEBUG_PRINT
  log_printf("\npub key [65]:\n");
  print_hex(t_pub_key, PUB_KEY_SIZE_65);
  log_printf("\n");
#endif
  memcpy(pub_key, &t_pub_key[1], PUB_KEY_SIZE);
#ifdef DEBUG_PRINT
  log_printf("pub key\n");
  print_hex(pub_key, PUB_KEY_SIZE);
  log_printf("\n");
#endif
  return 1;
#endif

#ifdef USE_UECC
#ifdef DEBUG_PRINT
  log_printf("Used uECC ");
#endif
  if (uECC_compute_public_key(priv_key, pub_key, curve) == 1)
  {
#ifdef DEBUG_PRINT
    log_printf("\nuECC_compute_public_key() OK, pub_key:\n");
    print_hex(pub_key, PUB_KEY_SIZE);
    log_printf("\n");
#endif
    return 1;
  }
  else
  {
    log_printf("uECC_compute_public_key() error!\n");
  }
#endif
  return ret;
}

// get ETH account address from pub key
std::string ethAddr(uint8_t *priv_key)
{
  std::string eth_account = "";
  uint8_t pub_key[PUB_KEY_SIZE] = {0};
  if (ethPubKey(pub_key, priv_key) > 0)
  {
    uint8_t pub_hash[KEYS_SIZE];
    do_keccak256(pub_hash, pub_key, PUB_KEY_SIZE);
#ifdef DEBUG_PRINT
    log_printf("\npub key hash:\n");
    print_hex(pub_hash, KEYS_SIZE);
#endif
    eth_account = eth_address(pub_hash);
#ifdef DEBUG_PRINT
    log_printf("\nETH address: %s\n", eth_account.c_str());
#endif
  }
  else
  {
    log_printf("Pub key error!\n");
  }
  return eth_account;
}

#endif