import subprocess, sys
import json
import binascii

import json

from dataclasses import asdict, dataclass
from typing import Optional

import rlp
from eth_typing import HexStr
from eth_utils import keccak, to_bytes
from rlp.sedes import Binary, big_endian_int, binary
from web3 import Web3
from web3.auto import w3

import eth_keys

class Transaction(rlp.Serializable):
    fields = [
        ("nonce", big_endian_int),
        ("gas_price", big_endian_int),
        ("gas", big_endian_int),
        ("to", Binary.fixed_length(20, allow_empty=True)),
        ("value", big_endian_int),
        ("data", binary),
        ("v", big_endian_int),
        ("r", big_endian_int),
        ("s", big_endian_int),
    ]


@dataclass
class DecodedTx:
    hash_tx: str
    from_: str
    to: Optional[str]
    nonce: int
    gas: int
    gas_price: int
    value: int
    data: str
    chain_id: int
    r: str
    s: str
    v: int

def hex_to_bytes(data: str) -> bytes:
    return to_bytes(hexstr=HexStr(data))

def decode_raw_tx(raw_tx: str):
    tx = rlp.decode(hex_to_bytes(raw_tx), Transaction)
    hash_tx = Web3.to_hex(keccak(hex_to_bytes(raw_tx)))
    from_ = w3.eth.account.recover_transaction(raw_tx)
    to = w3.to_checksum_address(tx.to) if tx.to else None
    data = w3.to_hex(tx.data)
    r = hex(tx.r)
    s = hex(tx.s)
    chain_id = (tx.v - 35) // 2 if tx.v % 2 else (tx.v - 36) // 2
    return DecodedTx(hash_tx, from_, to, tx.nonce, tx.gas, tx.gas_price, tx.value, data, chain_id, r, s, tx.v)

def getRawAndMsg (str):
  splitted =  grepped.split('--')
  # print(f'msg: {splitted[0]}')
  # print(f'raw: {splitted[1]}')
  
  msg_str = splitted[0].split('0x')[1]
  # print(f'msg: {msg_str}')

  splitter = 'Raw eth signed transaction:'
  raw_str = splitted[2].split('0x')[1]
  # print(f'raw: {raw_str}')
  return (raw_str.strip(), msg_str.strip())

def verifyRaw(pk, vk, msg, raw):
  tx = decode_raw_tx(raw)
  signerPrivKey = eth_keys.keys.PrivateKey(pk)
  signerPubKey = signerPrivKey.public_key
  signature = signerPrivKey.sign_msg(msg)
  signature.r = int(tx.r, 16)
  signature.s = int(tx.s, 16)
  signature.v = tx.v - tx.chain_id * 2 - 35

  print('Signature: [r = {0}, s = {1}, v = {2}]'.format(hex(signature.r), hex(signature.s), hex(signature.v)))
  vks = '0x' + vk.hex()
  # print(f'pub: {vks}')
  # recover pub key from RLP encoded message
  recoveredPubKey = signature.recover_public_key_from_msg(msg)
  # print(f'pub: {recoveredPubKey}')
  vk_rec = str(recoveredPubKey)
  if vks.strip() != vk_rec.strip() :
    print('Recovered public key is not equal to expected')
    return False

  # uECC_verify(pub_key, hash256, sizeof(hash256), sig, curve))  -> uECC_verify() OK
  valid = signerPubKey.verify_msg(msg, signature)
  # print("Signature valid?", valid)

  return valid

chainId = "0x539"
pks = '4f3edf983ac636a65a842ce7c78d9aa706d3b113bce9c46f30d7d21715b23b1d'
vks = 'e68acfc0253a10620dff706b0a1b1f1f5833ea3beb3bde2250d5f271f3563606672ebc45e0b7ea2e816ecb70ca03137b1c9476eec63d4632e990020b7b6fba39'
pk = bytes.fromhex(pks)
vk = bytes.fromhex(vks)

grep_raw = ' | grep -e "Final s" -e "Final r" -e "signed transaction:"  -e "transaction with header:" -A 1'
# grep_raw = ' | grep -e "signed transaction:"  -e "transaction with header:" -A 1'

testOK = True

# nonce, chainId, privKey, gasPrice, to , value, 
classicTx =  ["0xfffeff", chainId, pks, "0x04a817c800", "0xFFcf8FDEE72ac11b5c542428B35EEF5769C409f0", "0x0386a0"]
contractTx = ["0xfffeff", chainId, pks, "0x77359400", "0xe78a0f7e598cc8b0bb87894b0f60dd2a88d6a8ab", ""]

# cmd = "./ecdsa " + classicTx[0] + " " + classicTx[1] + " " + classicTx[2] + " " + " 'set_output(3)'
#                      nonce              chainId       rpcId       pKey
cmd = "./ecdsa " + classicTx[0] + " " + classicTx[1] + " 12 " + classicTx[2] + " " + grep_raw
res = subprocess.check_output (cmd, shell = True, executable="/bin/bash")
grepped = str(res,'utf-8').rstrip()
#print(f'grepped: {grepped}')

(raw_str, msg_str) = getRawAndMsg (grepped)
msg = bytes.fromhex(msg_str)
print(f'raw: {raw_str}')
print(f'msg: {msg_str}')

# verify transaction without smart contract
if not verifyRaw(pk, vk, msg, raw_str):
  print('! Failed unsigned classic transaction RPL encoding')
  testOK = False

# exit()

# verify transaction without smart contract
conracts = [
    "'get_output()'", 
    "'set_output(3)'", 
    "'set_output(3,15)'",
    "'set_bool(true)'",  "'set_bool(false)'",
    "'set_allowed(0xD028ec274Ef548253a90c930647b74C830Ed4b4F,true)'",
    "\"set_bytes('one')\"",
    "\"set_bytes('hello bytes 42')\"",
    "\"set_bytes([0 1 0x2 0x03 4 5 6 7 8 9 10 11 12 13 14])\"",
    "\"set_bytes([0 1 0x2 0x03 4 5 6 7 8 9 10 11 12 13 14 15 16 0x11 18 19 20 21 22 23 24 25 26 27 28 29 30 fg 0x100 256 31])\"",
    "\"set_string('Hello robo')\"",
    "\"set_string('some parameter maximum value = ',1234567890123444556)\""
]

for sm_call in conracts:
    #                      nonce              chainId       rpcId       pKey            contract method
    cmd = "./ecdsa " + classicTx[0] + " " + classicTx[1] + " 12 " + classicTx[2] + " " + sm_call + grep_raw
    res = subprocess.check_output (cmd, shell = True, executable="/bin/bash")
    grepped = str(res,'utf-8').rstrip()
    # print(f'raw : {grepped}')
   
    (raw_str, msg_str) = getRawAndMsg (grepped)
    msg = bytes.fromhex(msg_str)
    print(f'raw: {raw_str}')
    print(f'msg: {msg_str}')

    # verify transaction without smart contract
    if not verifyRaw(pk, vk, msg, raw_str):
      print('! Failed unsigned classic transaction RPL encoding')
      testOK = False

print(f'Test OK: {testOK}')
if  testOK:
    exit(0)
else:
   exit(1)