import requests
import subprocess, sys
import json
from eth_keys import keys
from eth_utils import decode_hex

eth_host = "http://192.168.0.106:8545"
privKey = "4f3edf983ac636a65a842ce7c78d9aa706d3b113bce9c46f30d7d21715b23b1d"

# python3 ethraw.py 1 http://192.168.0.106:8545 4f3edf983ac636a65a842ce7c78d9aa706d3b113bce9c46f30d7d21715b23b1d
cnt = 0
if len (sys.argv) > 1:
   cnt = sys.argv[1]

if len (sys.argv) > 2 :
   eth_host = sys.argv[2]

if len (sys.argv) > 3 :
   privKey = sys.argv[3]

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
r = requests.post(eth_host, data = js_nonce)
print (r.json())
js_dict = r.json()
nonce = js_dict.get('result') # cpp app as arg 1

js_chain = '{"id":1,"jsonrpc":"2.0","method":"eth_chainId"}'
print (js_chain)
r = requests.post(eth_host, data = js_chain)
print (r.json())
js_dict = r.json()
chain = js_dict.get('result') # cpp app as arg 2

#sign by C++ app to test
cmd = './ecdsa ' + nonce + ' ' + chain +  ' ' + str(cnt) + ' ' +  privKey  + ' | tail -n 1'
print (cmd)
res = subprocess.check_output (cmd, shell = True, executable="/bin/bash") #, stdout = subprocess.STDOUT)
js = res.decode("utf-8")

js0 = js[1:-2]
print (js0)
r = requests.post(eth_host, data = js0)
print(r.json())
