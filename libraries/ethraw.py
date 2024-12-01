import requests
import subprocess, sys
import json
from eth_keys import keys
from eth_utils import decode_hex

eth_host = "http://192.168.0.106:8545"
privKey = "4f3edf983ac636a65a842ce7c78d9aa706d3b113bce9c46f30d7d21715b23b1d"
contract = ""
sc_abi = ""
grep = ""
chain = "0x539"
nonce = "42"

# for usual transaction:
# python3 ethraw.py 1 http://192.168.0.106:8545 4f3edf983ac636a65a842ce7c78d9aa706d3b113bce9c46f30d7d21715b23b1d
# for smart contract:
# python3 ethraw.py 1 http://192.168.0.106:8545 4f3edf983ac636a65a842ce7c78d9aa706d3b113bce9c46f30d7d21715b23b1d  "'set_allowed(0xD028ec274Ef548253a90c930647b74C830Ed4b4F,true)'"

# examples for ABI and RPL testing 
# python3 ethraw.py 1 http://192.168.0.106:8545 4f3edf983ac636a65a842ce7c78d9aa706d3b113bce9c46f30d7d21715b23b1d  "'set_output(3)'" contracts/g5.abi rpl
# python3 ethraw.py 1 http://192.168.0.106:8545 4f3edf983ac636a65a842ce7c78d9aa706d3b113bce9c46f30d7d21715b23b1d  "'set_allowed(0xD028ec274Ef548253a90c930647b74C830Ed4b4F,true)'" contracts/g5.abi abi
# python3 ethraw.py 1 http://192.168.0.106:8545 4f3edf983ac636a65a842ce7c78d9aa706d3b113bce9c46f30d7d21715b23b1d  "'set_string(\"some parameter maximum value = \"'" contracts/g5.abi
# python3 ethraw.py 1 http://192.168.0.106:8545 4f3edf983ac636a65a842ce7c78d9aa706d3b113bce9c46f30d7d21715b23b1d  "'set_string(\"some parameter maximum value = \",5'" contracts/g5.abi
# python3 ethraw.py 1 http://192.168.0.106:8545 4f3edf983ac636a65a842ce7c78d9aa706d3b113bce9c46f30d7d21715b23b1d  "'set_bytes([0 1 0x2 0x03 4 5 6 7 8 9 10 11 12 13 14])'" contracts/g5.abi abi

cnt = 0
if len (sys.argv) > 1:
   cnt = sys.argv[1]

if len (sys.argv) > 2 :
   eth_host = sys.argv[2]

if len (sys.argv) > 3 :
   privKey = sys.argv[3]

if len (sys.argv) > 4 :
   contract = sys.argv[4]
   print ("Called contract: ", contract)

if len (sys.argv) > 5 :
   sc_abi = sys.argv[5]
   print ("Contract ABI: ", sc_abi)

if len (sys.argv) > 6 :
   grep = sys.argv[6]
   print ("Grep: ", grep)

# private key -> pubkey -> eth address
pkb = decode_hex(privKey)
priv_key = keys.PrivateKey(pkb)
pub_key = priv_key.public_key
print("Pub key: " + pub_key.to_hex())
eth_id = pub_key.to_checksum_address()
print ("Own address: ", eth_id)

prefix = "0x"
privKey = privKey.replace(prefix, "")

js_nonce = '{"id":2,"jsonrpc":"2.0","method":"eth_getTransactionCount", "params":["' + eth_id + '"]}'
print (js_nonce)
if grep != "abi" and grep != "rpl":
  r = requests.post(eth_host, data = js_nonce)
  print (r.json())
  js_dict = r.json()
  nonce = js_dict.get('result') # cpp app as arg 1

if grep != "abi" and grep != "rpl":
  js_chain = '{"id":1,"jsonrpc":"2.0","method":"eth_chainId"}'
  print (js_chain)
  r = requests.post(eth_host, data = js_chain)
  print (r.json())
  js_dict = r.json()
  chain = js_dict.get('result') # cpp app as arg 2

gcmd = ' | tail -n 1'
if grep == "abi":
  gcmd = " | grep -A1 'Calldata:' | tail -1"
if grep == "rpl":
  gcmd = " | grep -A1 'RPL of unsigned transaction with' | tail -1 "

#sign by C++ app to test
if contract == "":
  cmd = './ecdsa ' + nonce + ' ' + chain +  ' ' + str(cnt) + ' ' +  privKey  + ' | tail -n 1'
else:
  if grep != "abi" and grep != "rpl":
    cmd = './ecdsa ' + nonce + ' ' + chain +  ' ' + str(cnt) + ' ' +  privKey  +  ' ' + contract + ' ' + sc_abi + ' | tail -n 1'
  else:
    cmd = './ecdsa ' + nonce + ' ' + chain +  ' ' + str(cnt) + ' ' +  privKey  +  ' ' + contract + ' ' + sc_abi + gcmd
print (cmd)

res = subprocess.check_output (cmd, shell = True, executable="/bin/bash") #, stdout = subprocess.STDOUT)
js = res.decode("utf-8")

if grep == "abi" or grep == "rpl":
  print (js)
else:   
  js0 = js[1:-2]
  print (js0)
  #if grep != "abi" and grep != "rpl":
  r = requests.post(eth_host, data = js0)
  print(r.json())
