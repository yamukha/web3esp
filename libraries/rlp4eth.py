import rlp
import sys
import pycoin
from ecdsa import SigningKey, SECP256k1, util
import random
import sha3 # pysha3
import binascii
import hashlib
import mbedtls
from eth_hash.auto import keccak
from hashlib import sha256
from eth_keys import keys
import requests
import json
import secp256k1

# ref. https://lsongnotes.wordpress.com/2018/01/14/signing-an-ethereum-transaction-the-hard-way/

def canonize(s):
  # n = 115792089237316195423570985008687907852837564279074904382605163141518161494337
  n = SECP256k1.order
  #s = # int.from_bytes(s_bytes, byteorder='big')
  if s > n//2:
    s = n - s
    print ('s is normalized:', s)
  return s

def int_bytes (number):
  if number < 0x100 :
      return 1
  elif number < 0x10000 and number > 0xff :
      return 2
  elif number < 0x1000000 and number > 0xffff :
      return 3
  else :
      return 4

tx_message = list()
tx_unsigned = list()
host =  'http://127.0.0.1:8545'
deterministic = False
priv_key = '0x2caa8ee480bc8b2fcdf8499027e53a6d5c577a53aa5be6239a328a0fb7fabfac'

# example to use: python3 rlp4eth.py privKey ethHost
# python3 rlp4eth.py 4f3edf983ac636a65a842ce7c78d9aa706d3b113bce9c46f30d7d21715b23b1d http://192.168.0.106:8545

if len(sys.argv) == 2 :
   priv_key = sys.argv[1]
   print ("Private key to sign: ", priv_key)
elif len(sys.argv) == 3 :
   priv_key = sys.argv[1]
   print ("Private key to sign: ", priv_key)
   host = sys.argv[2]
   print ("Host address: ",  host)
elif len(sys.argv) > 3 :
   priv_key = sys.argv[1]
   print ("Private key to sign: ", priv_key)
   host = sys.argv[2]
   print ("Host : ", host)
   if sys.argv[3].lower() == 'true' :
     deterministic = True
else :
   print ("Priv key: " + str(len(priv_key)))
   print (priv_key)
   print (host)
   print ("deterministic =", deterministic)

private_key_64 = priv_key.encode()
private_key_bytes = binascii.unhexlify(private_key_64)

pk = keys.PrivateKey(private_key_bytes)
pub_key = pk.public_key
print("Pub key: " + pub_key.to_hex())

addr = pub_key.to_checksum_address()
print ("Own address: ", addr)

# https://ethereum.stackexchange.com/questions/70584/how-to-build-a-simple-transaction
# https://www.programcreek.com/python/example/104140/rlp.encode

tx_nonce = 0
# Get nonce for given address
#curl 192.168.0.102:8545 -X POST -H "Content-Type: application/json" --data '{"jsonrpc":"2.0","method":"eth_getTransactionCount","params":["0x90F8bf6A479f320ead074411a4B0e7944Ea8c9C1"]}'
req = '{"jsonrpc":"2.0","method":"eth_getTransactionCount","params":["'+addr+'"]}'
print (req)
r = requests.post(host, data= req)
print("Get nonce response: ", r.text)
resp_json =  json.loads(r.text)
resp_nonce = resp_json ['result']
tx_nonce = int(resp_nonce,16)

chain_id = 1337 # 0x0539 ganache chain Id  of test blockchain network
# Get Chain_id
#curl 192.168.0.102:8545 -X POST -H "Content-Type: application/json"  --data '{"method":"eth_chainId","params":[],"id":1,"jsonrpc":"2.0"}'
req = '{"jsonrpc":"2.0","method":"eth_chainId","params":[]}'
print (req)
r = requests.post(host, data= req)
print("Get chain Id response:", r.text)
resp_json =  json.loads(r.text)
resp_chain_id = resp_json ['result']
chain_id = int(resp_chain_id,16)

tx_gasPrice = 0x04a817c800;
tx_gasLimit = 0x1e8480;
tx_to = 0xFFcf8FDEE72ac11b5c542428B35EEF5769C409f0;
tx_value = 0x0186a0;
tx_data=0x0;
tx_v = chain_id
tx_r = 0
tx_s = 0

print ("nonce:", tx_nonce, "price:", tx_gasPrice, "limit:", tx_gasLimit, "to:", tx_to, "value:", tx_value, "chain_id:", tx_v, "r,v:", tx_r, tx_s)
encoded = rlp.encode([tx_nonce, tx_gasPrice, tx_gasLimit, tx_to, tx_value, tx_data, tx_v, tx_r, tx_s])
print ("PLP encoded inital values: \n", encoded.hex())

secret=int(priv_key, 16)
sk = SigningKey.from_secret_exponent(secret, curve = SECP256k1)

# Get hash from unsigned RLP

print ("Keccak hash digest for unsigned msg:")
hk = keccak(encoded) #txu_raw_bytes)
print (hk.hex())

signed_det = sk.sign_digest_deterministic(hk, hashfunc = sha3.keccak_256) # hashlib.sha3_256  # hashfunc=hashlib.sha256)
print ("Signed with deterministic digest:")
print (signed_det.hex())

signed = sk.sign(encoded, hashfunc=sha3.keccak_256) # at random curve point
print ("Signed undeterministic:")
print (signed.hex())

if deterministic == True :
   signed = signed_det
   print ("Use deterministic signature")

order = SECP256k1.order
tx_r,tx_s = util.sigdecode_string(signed, order) #
tx_s = canonize(tx_s)

r = tx_r.to_bytes(32,'big').hex()
s = tx_s.to_bytes(32,'big').hex()
r_bytes = tx_s.to_bytes(32,'big')

recovery_id = 0
if r_bytes[31] % 2 ==  0:
   recovery_id = 1

# chain_id * 2 + 35 + {0,1} is parity of y for curve point, where chain_id = 1337 for private chain;
# ref. https://github.com/ethereum/EIPs/blob/master/EIPS/eip-155.md
v = chain_id * 2  + 35 + recovery_id
print ("Recovery_id, v: ", recovery_id , v)
print ("r,s:", r, s)

encoded_raw = rlp.encode([tx_nonce, tx_gasPrice, tx_gasLimit, tx_to, tx_value, tx_data, v, tx_r, tx_s])
print ("Raw transaction: ")
print (encoded_raw.hex())
# https://medium.com/@LucasJennings/a-step-by-step-guide-to-generating-raw-ethereum-transactions-c3292ad36ab4

# curl 192.168.0.102:8545 -X POST -H "Content-Type: application/json" --data '{"jsonrpc":"2.0","method":"eth_sendRawTransaction","params":[""],"id":58}'
req = '{"jsonrpc":"2.0","method":"eth_sendRawTransaction","params":["'+'0x'+encoded_raw.hex()+'"]}'
print ("Sent:\n" + req)
r = requests.post(host, data= req)
print("Raw transaction response:\n" +  r.text)
