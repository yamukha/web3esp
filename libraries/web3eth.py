import json
import sys
import rlp
import binascii
from web3 import Web3

# ganache-cli -h 0.0.0.0 -d
#Available Accounts
#==================
#(0) 0x90F8bf6A479f320ead074411a4B0e7944Ea8c9C1 (100 ETH)
#(1) 0xFFcf8FDEE72ac11b5c542428B35EEF5769C409f0 (100 ETH)

#Private Keys
#==================
#(0) 0x4f3edf983ac636a65a842ce7c78d9aa706d3b113bce9c46f30d7d21715b23b1d
#(1) 0x6cbed15c793ce57650b9877cf6fa156fbef513c4e6134f022a85b1ffdd59b2a1

# ref. python examples:
# https://www.polarsparc.com/xhtml/GanacheSolidityPython.html
# https://web3py.readthedocs.io/en/stable/transactions.html
# https://habr.com/ru/articles/674204/  -> get nonce, get privkey from mnemonics etc.
# https://web3py.readthedocs.io/en/stable/web3.eth.html  -> web3.eth API and.properties  like chain_id  gas_price etc.
# ref RLP:
# https://www.ethereumdecoder.com/      -> online RLP decoder to test constructed transaction
# https://ethereum.org/ru/developers/docs/data-structures-and-encoding/rlp/ -> RLP how to
# https://medium.com/@LucasJennings/a-step-by-step-guide-to-generating-raw-ethereum-transactions-c3292ad36ab4

host = "http://192.168.0.103:8545"  # http://127.0.0.1:8545 for localhost

w3 = Web3(Web3.HTTPProvider(host))
w3_status = w3.is_connected()

if w3_status == False:
    print ("Cannot connect to host: " + host)
    exit()

from_addr = '0x90F8bf6A479f320ead074411a4B0e7944Ea8c9C1'
to_addr   = '0xFFcf8FDEE72ac11b5c542428B35EEF5769C409f0'
#to_addr1  = '0x7917bc33eea648809c285607579c9919Fb864f8f'  # test
#to_ad = bytearray.fromhex(to_addr1 [2:]) # test

balance = w3.eth.get_balance(from_addr)
print ("balance:")
print (balance)

print ("Is connected to host: " + host + " <- " + str(w3_status))
print ("Is valid address from: " + from_addr + " <- " + str(w3.is_checksum_address(from_addr)))
print ("Is valid address to: " + to_addr +  " <- " + str(w3.is_checksum_address(to_addr)))

from_priv_key = '0x4f3edf983ac636a65a842ce7c78d9aa706d3b113bce9c46f30d7d21715b23b1d'
#from_priv_key = '0x00d862c318d05de0a1c25242c21989e15e35e70c55996fbc4238cd2f2f6a8f62' # test
acct = w3.eth.account.from_key(from_priv_key)

w3.eth.default_account = acct.address # '0x90F8bf6A479f320ead074411a4B0e7944Ea8c9C1'
print (w3.eth.default_account)

weis_to_send = 100000
#weis_to_send = 0x03BAF82D03A000 # test
gas_limit = 2000000
#gas_limit = 0x5208 # test
gas_price =  w3.eth.gas_price
#gas_price = 0x0BA43B7400 # test
print ("Gas price: " + str(gas_price))

nonce = w3.eth.get_transaction_count(from_addr)
print ("nonce: " + str(nonce))
#nonce = 0 # test

chain_id = w3.eth.chain_id
#chain_id = 1 # test
print ("chain Id: " + str(chain_id))

txn_json = {
  #'from': from_addr, # is optional in json
  'to': to_addr,
  'value': weis_to_send,
  'gas': gas_limit,
  'gasPrice': gas_price,
  'nonce': nonce,
  'chainId': chain_id,
   #'r': 0, # sign up error: unrecognized field
   #'s': 0  # sign up error: unrecognized field
}
print ("Transaction as Json:\n" + str(txn_json))
#print ("Transaction as bytes:\n")
#print (txn_json)

#encoded =  rlp.encode([ hex(txn_json['nonce']), hex(txn_json['gasPrice']), hex(txn_json['gas']),  hex(txn_json['value']), 0x0, 0x1, 0x0, 0x0 ])

#encoded = rlp.encode([nonce, gas_price, gas_limit, to_ad, weis_to_send, 0x0,0x1,0x0,0x0])
#print (encoded.hex())

#exit()

#from_priv_key = '0x4f3edf983ac636a65a842ce7c78d9aa706d3b113bce9c46f30d7d21715b23b1d'
signed_txn = w3.eth.account.sign_transaction(txn_json, from_priv_key)
#signed_tx =  w3.eth.sign(w3.eth.default_account,hexstr="0xea3d8504a817c800831e848094ffcf8fdee72ac11b5c542428b35eef5769c409f0830186a080820a958080")
#signed = w3.eth.sign_transaction(txn_json) # not supported

print (signed_txn)
print ('Raw transaction :\n' + signed_txn.rawTransaction.hex())
#print (signed_tx.hex())

if len(sys.argv) > 1 :
   print (sys.argv[1])
   print ("key to sign: \n" + from_priv_key)
   keccak =  w3.to_hex(w3.keccak(signed_txn.rawTransaction))
   print ("keccak hash:\n" + keccak)
   exit(0)
#else:


#tx_raw = '0xf86a808504a817c800831e848094ffcf8fdee72ac11b5c542428b35eef5769c409f0830186a080820a95a01781691241d8e12c80b1f0b01b781a36ef473495081bb072c00f0c3aec1febcba028661ce7e924d8e36d32d1be670ec74bee1125ae006a9e9d280998728b952c5d'
#signed_txn.rawTransaction = bytearray.fromhex(tx_raw[2:])
#txn_hash = w3.eth.send_raw_transaction(tx_raw)
keccak =  w3.to_hex(w3.keccak(signed_txn.rawTransaction))
print ("keccak hash :\n" + keccak)
txn_hash = w3.eth.send_raw_transaction(signed_txn.rawTransaction)

print ('Transaction hash: ' + txn_hash.hex())

txn = w3.eth.get_transaction(txn_hash)

#print (txn)
txn_json = json.loads(w3.to_json(txn))
print  ('Transaction details: \n' + str(txn_json))
