import json
import sys
import rlp
import binascii
from web3 import Web3
from eth_hash.auto import keccak
from hashlib import sha256
from ecdsa import SigningKey, SECP256k1, util # sudo pip3 install ecdsa
import sha3   # sudo pip3 install pysha3
#from hashlib import sha256

def canonize(s):
  # n = 115792089237316195423570985008687907852837564279074904382605163141518161494337
  n = SECP256k1.order
  #s = # int.from_bytes(s_bytes, byteorder='big')
  if s > n//2:
    s = n - s
    print ('s is normalized:', s)
  return s

class funcs(object):
    def __init__(self, name, inputs):
        self.name = name
        self.inputs = inputs
   # pass

def abi_offset (sabi, fu, args):
  # jabi = json.loads(sabi)
  # offset = 

  # there is bug if there are methods with same name, and ofsset added to count all
  # for item in jabi:
  #  if fu == item ["name"]:
  #      for arg in item["inputs"]:
  #         offset += 32

  # return offset 
  return 32 * args

def str2abi(txt):
    out = ""
    if len(txt) == 0:
      return out

    if len(txt) > 1 and  txt[0] == '"' and  txt[-1] == '"':
      txt = txt[1:-1]
    else:
      if len(txt) > 1 and  txt[0] == "'" and  txt[-1] == "'":
        txt = txt[1:-1]

    for ch in txt:
      out += ch.encode('utf-8').hex()

    if len(out) % 64 != 0:
      for x in range(64 - len(out) % 64):
        out += "0"

    return  out

def abi_function_parser(sabi):
  jabi = json.loads(sabi)
  flist = []

  for item in jabi:
    f = item ["name"]
    plist = []
    #print("item: " + f)
    for arg in item["inputs"]:
      el = {arg["name"]: arg["type"]}
      plist.append(el) # maybe { name, arg["type"]}
      #print("element: ") 
      #print(el)

    flist.append(funcs(f,plist))
  
  #print('---')
  #for item in flist:
      #print(f"{item.name}")
      #for i in item.inputs:
        #print(f" {i}")
  #print("len(flist): " + str(len(flist)))
  return flist

# get hash by fn name i.e. 'get_output()'
def funhash (funame, abi):
  fl = abi_function_parser(abi)
  
  funame = funame.replace('(', ',')
  funame = funame.replace(')', '')
  # print ("edited func name: ", funame)

  fspiltted = funame.split(',')

  fname = fspiltted[0]
  pcount = len(fspiltted) - 1
  if len(fspiltted) == 2 and len(fspiltted[1]) == 0:
    pcount = 0
  #print (f"func  name: {fname} params count {pcount}")

  fabi = ""
  
  for item in fl:
    #print("fl.item :" + item.name + " " + str(len(item.inputs)))
    # print(len(item.inputs))
    if fname == item.name:
      #print("item :" + item.name + " " + str(len(item.inputs)))
      if len(item.inputs) == pcount:   
        #print(f"item ->: {item.name}")
        for i in item.inputs:
          #print("i=", str(i))
          el = i.items()
          k = ""
          v = ""
          for k,v in el:
              pass
              #print (k, v)
          fabi += v
          fabi += ","       

        tohash = fname + "(" + fabi[:-1] + ")"
        tohash_bytes = tohash.encode('ascii')
        abi_keccak = "0x" + keccak(tohash_bytes).hex()
        #print('===--')
        #print (f"Method {tohash} hash {abi_keccak}")
        return abi_keccak[:10]
  #print('==---')
  return ""

def docalldata(fcall,abi):
  fl = abi_function_parser(abi) #fl = abi_function_parser(abi_g5)
  stat = ""
  dyna = ""  
  funame = fcall
  funame = funame.replace('(', ',')
  funame = funame.replace(')', '')

  fspiltted = funame.split(',')

  fname = fspiltted[0]
  pcount = len(fspiltted) - 1  
  offset = abi_offset(abi, fname, 0)  # dynamic part start address  offset = abi_offset(abi_g5, fname)

  #print ("func  name: " + fname  + " params count: " + str(pcount) + " offset " + str(offset))
  #print('==-==')
  cnt = 0
  for item in fl:
    if fname == item.name:
      #print(item.name)
      #print(len(item.inputs))
      if len(item.inputs) == pcount:          
        for it, par in enumerate(item.inputs):
          # print(par)
          el = par.items()
          for k,v in el:
             cnt = cnt + 1    
             # print (k, v, cnt)          
             if v in static_types:
                if v in int_types:
                  x = f"{int(fspiltted[cnt]):064X}" # if static data len is 32 bytes or 64 digits places in hex format
                  stat += x
                  print( "parametr '" + k + "' has static type '" + v + "' with value " +  x)
                if v in byte_types:
                  d = fspiltted[cnt]
                  x = str2abi(d)
                  stat += x
                  print( "parametr '" + k + "' has static type '" + v + "' with value " +  x)
                if v in bool_types:
                  if fspiltted[cnt] == "true":
                    x = f"{1:064X}"
                    stat += x
                    print( "parametr '" + k + "' has static type '" + v + "' with value " +  x)
                  else:
                    x = f"{0:064X}"
                    stat += x
                    print( "parametr '" + k + "' has static type '" + v + "' with value " +  x) 
             else :
                print (offset) # if dynamic add 32 bytes or 64 digits places in hex format
                x = f"{offset:064X}"
                stat += x
                d = fspiltted[cnt]
                de = str2abi(d)  
                #print( "parametr '" + k + "' has dynamic type '" + v + "' with offset " + str(offset) + ", data " + d)
                #print( "encoded: " + de)
                #print( "length: " + str(len(d)-2))
                dlx = f"{(len(d)-2):064X}" # length of dynamic block
                dyna += dlx + de
                dynamic_obj_size = 0
                offset = offset + dynamic_obj_size
                # TODO  ?
        return stat,dyna # if signature matched

  return stat,dyna

static_types = ["uint8", "uint16", "uint24", "uint32", "uint40", "uint48", "uint56", "uint64",  
  "uint72", "uint80", "uint88", "uint96", "uint104", "uint112", "uint120", "uint128",
  "uint136", "uint144", "uint152", "uint160", "uint168", "uint176", "uint184", "uint192",
  "uint200", "uint208", "uint216", "uint224", "uint232", "uint240", "uint248", "uint256",
  "int8", "int16", "int24", "int32", "int40", "int48", "int56", "int64",  
  "int72", "int80", "int88", "int96", "int104", "int112", "int120", "int128",
  "int136", "int144", "int152", "int160", "int168", "int176", "int184", "int192",
  "int200", "int208", "int216", "int224", "int232", "int240", "int248", "int256",
  "address", "uint", "int", "bool", "fixed", "ufixed", "function",
  "bytes1","bytes2", "bytes3", "bytes4", "bytes5", "bytes5", "bytes7", "bytes8",
  "bytes9","bytes10", "bytes11", "byte12", "bytes13", "bytes14", "bytes15", "bytes16",
  "bytes17","bytes18", "bytes19", "bytes20", "bytes21", "bytes22", "bytes23", "bytes24",
  "bytes25","bytes26", "bytes27", "bytes28", "bytes29", "bytes30", "bytes31", "bytes32" ]

int_types = ["uint",
  "uint8", "uint16", "uint24", "uint32", "uint40", "uint48", "uint56", "uint64",  
  "uint72", "uint80", "uint88", "uint96", "uint104", "uint112", "uint120", "uint128",
  "uint136", "uint144", "uint152", "uint160", "uint168", "uint176", "uint184", "uint192",
  "uint200", "uint208", "uint216", "uint224", "uint232", "uint240", "uint248", "uint256"]

uint_types = ["int",
  "int8", "int16", "int24", "int32", "int40", "int48", "int56", "int64",  
  "int72", "int80", "int88", "int96", "int104", "int112", "int120", "int128",
  "int136", "int144", "int152", "int160", "int168", "int176", "int184", "int192",
  "int200", "int208", "int216", "int224", "int232", "int240", "int248", "int256"]

byte_types = [
  "bytes1","bytes2", "bytes3", "bytes4", "bytes5", "bytes5", "bytes7", "bytes8",
  "bytes9","bytes10", "bytes11", "byte12", "bytes13", "bytes14", "bytes15", "bytes16",
  "bytes17","bytes18", "bytes19", "bytes20", "bytes21", "bytes22", "bytes23", "bytes24",
  "bytes25","bytes26", "bytes27", "bytes28", "bytes29", "bytes30", "bytes31", "bytes32"]

bool_types = ["bool"]
address_types = ["address"]
fixed_types = ["fixed", "ufixed"]
function_types = ["function"]

  # note:  
  # fixed8x1 ... ufixed128x18,  etc.:   8 <= M <= 256, M % 8 == 0, and 0 < N <= 80
  # ufixed8x1 ... ufixed128x18, etx.:   8 <= M <= 256, M % 8 == 0, and 0 < N <= 80
  # TODO: expected to be detected as fixed by parser

print(f"static_types: \n {static_types}")

print(f"usage:\n python3 g5_contract.py [abi_method] [abi_file] [contract_addr] [host]")
print(f"example:\n python3 g5_contract.py 'set_bytes(\"one\")'")
# python3 g5_contract.py [abi_method] [abi_file] [contract_addr] [host]
# python3 g5_contract.py [abi_method] [abi_file] [contract_addr] [host] 

fcall = 'get_output()' 
#fcall =  'set_output(3)'
#fcall = "set_output(1,12345678901234445566)"
#fcall = 'set_string("me")'
#fcall = 'set_string("me",3)'
#fcall = 'set_string("me",1234567890123444556)' # uint64_t
#fcall = 'set_bytes("one")'
#fcall = 'set_bool(true)'

if len(sys.argv) > 1:
  fcall = sys.argv[1]

abi_file = "contracts/g5.abi"
if len(sys.argv) > 2:
  abi_file = sys.argv[2]

#contract_address_g5 = '0xe78a0f7e598cc8b0bb87894b0f60dd2a88d6a8ab' 
contract_address_g5 =  '0xd9145CCE52D386f254917e481eB44e9943F39138'
#contract_address_g5 = '0x9561c133dd8580860b6b7e504bc5aa500f0f06a7'
#contract_address_g5 = '0x59d3631c86bbe35ef041872d502f218a39fba150'
#contract_address_g5 = '0x254dffcd3277c0b1660f6d42efbb754edababc2b'
#contract_address_g5 = '0xd833215cbcc3f914bd1c9ece3ee7bf8b14f841bb'
#contract_address_g5 = '0xe982e462b094850f12af94d21d470e21be9d0e9c'
#contract_address_g5 = '0x6b3f2e907db696f63405eec10ae2a3bf9907c703'
if len(sys.argv) > 3:
  contract_address_g5 = sys.argv[3]

# ganache-cli -d -h 0.0.0.0 -p 7585
if len(sys.argv) > 4:
  host = sys.argv[4]
host = "http://192.168.0.106:8545"  # http://127.0.0.1:8545 for localhost

print(f"fcall: {fcall} , abi_file: {abi_file} , contract_addr{contract_address_g5} , host: {host}")

#Load contract
fg5abi = open(abi_file, "r")
abi_g5 = fg5abi.read()
fg5abi.close()

G5_ABI = json.loads(abi_g5)
fl = abi_function_parser(abi_g5)
print("In contact abi calls count: " + str(len(fl)))

# with dynamic paramaters
# step 0: build object from ABI json, i.e. contract_g5.functions.set_string("me",3).
# step 1: get signature of function, convert types to "canonic" (TODO if this done in Remix automatelly)
# step 2: get hash of signature with "canonic" types
# step 3: serialize pameters
#           get offset (32 * number of papameters) for 1st possible dynamic parameter in bytes from the start of the arguments block
#           if element static - put in place parameter value, 
#           if element dynamic - put it place offset and fill up parameters, recalculate new offset taking into account dynamic part
# step 4: conactenate serialized static parameters and offsets with dymanic parameters block
# step 5: conactenate funcion hash to serialized parameters


# step 1: hash of canonic function signature 
# TODO find function to fit signature

# step 2: hash of canonic function signature 
# tohash_bytes = tohash.encode('ascii')
# abi_keccak = "0x" + keccak(tohash_bytes).hex()
ftohash = fcall #"set_string(string,uint256)"
print("function to be hashed: " + ftohash) 
fhashed = funhash(ftohash,abi_g5)
#if fhashed == "":
#  fhashed = '0xbb057cc9' # TODO

print ("hash function " + ftohash + ": " + fhashed)

# step 3:  serialize pameters
stat, dyna = docalldata(fcall,abi_g5)
print ("statis parameters with dynamic data ofsset, if applicable:")
print (stat)
if len(dyna) > 0:
  print ("dynamic parameters, size followed by data:")
  print (dyna)

#steps 4,5: conactenate serialized static parameters and offsets with dymanic parameters block
# hardocded
#abia_p1 = '0000000000000000000000000000000000000000000000000000000000000001'
#abia_p2 = '0000000000000000000000000000000000000000000000000000000000000003'
# fhash  = fhashed + abia_p1 + abia_p2
fhash2 = fhashed + stat + dyna
# print (fhash)
print("calldata:")
print (fhash2)

# calldata set_string("me",3)
#print ('set_string("me")')
#me = "0xc9615770000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000000026d65000000000000000000000000000000000000000000000000000000000000"
#print (me)
#print ('set_string("me",3)')
#me3 = "0xa4965edc0000000000000000000000000000000000000000000000000000000000000040000000000000000000000000000000000000000000000000000000000000000300000000000000000000000000000000000000000000000000000000000000026d65000000000000000000000000000000000000000000000000000000000000"
#print (me3)

# exit()

w3 = Web3(Web3.HTTPProvider(host))
#w3abi_keccak =  w3.to_hex(w3.keccak(text="set_string(string,uint256)"))
#fw3hashed = w3abi_keccak[:10]
#print ("by w3 : "  + fw3hashed)

w3_status = w3.is_connected()

if w3_status == False:
    print ("Cannot connect to host: " + host)
    exit()

# initialize ganache contract
contract_address_g5 = w3.to_checksum_address(contract_address_g5)
contract_g5 = w3.eth.contract(contract_address_g5, abi=G5_ABI)

# see all possible funtions
all_functions = contract_g5.all_functions()
print(f"All functions of ganache contract:\n{all_functions}")
#print(contract_g5.functions.get_output().call()) 
#print(contract_g5.functions.set_output(3).call()) 
#print(contract_g5.functions.set_string("me",3).call()) 

# ========================
to_addr = '0xFFcf8FDEE72ac11b5c542428B35EEF5769C409f0'

from_priv_key = '0x4f3edf983ac636a65a842ce7c78d9aa706d3b113bce9c46f30d7d21715b23b1d'

from_account=w3.eth.account.from_key(from_priv_key)
from_addr=from_account.address # from_addr = '0x90F8bf6A479f320ead074411a4B0e7944Ea8c9C1'

w3.eth.default_account = from_addr
print (f"Default account: {w3.eth.default_account}")

balance = w3.eth.get_balance(from_addr)
print (f"balance: {balance}")

# print ("Is connected to host: " + host + " <- " + str(w3_status))
# print ("Is valid address from: " + from_addr + " <- " + str(w3.is_checksum_address(from_addr)))
# print ("Is valid address to: " + to_addr +  " <- " + str(w3.is_checksum_address(to_addr)))

weis_to_send = 100000   #weis_to_send = 0x03BAF82D03A000 # test
gas_limit = 2000000     #gas_limit = 0x5208 # test
gas_price =  w3.eth.gas_price #gas_price = 0x0BA43B7400 # test
print ("Gas price: " + str(gas_price))

nonce = w3.eth.get_transaction_count(from_addr)
print ("nonce: " + str(nonce)) #nonce = 0 # test

chain_id = w3.eth.chain_id  #chain_id = 1 # test
print ("chain Id: " + str(chain_id))

txn_json = {
  #'from': from_addr, # is optional in json
  'to': to_addr,
  'value': weis_to_send,
  'gas': gas_limit,
  'gasPrice': gas_price,
  'nonce': nonce,
  'chainId': chain_id,
   #'data': None,  # for simple transaction , and serialized abi of smart contract
   #'r': 0, # sign up error: unrecognized field
   #'s': 0  # sign up error: unrecognized field
}

# exit()

txn_json = {
'value': 0, 
'gas': gas_limit,
'gasPrice': gas_price,
 # 'maxFeePerGas': 1093240212, 'maxPriorityFeePerGas': 1000000000, 
 'chainId': chain_id, 
 # 'from': '0x90F8bf6A479f320ead074411a4B0e7944Ea8c9C1', 
 'nonce': nonce, 
 'to': contract_address_g5, 
 'data': fhash2,  # fhash or fhash2
 }

print ("Call function: \n" + str(txn_json))
#print ("Transaction as bytes:\n")

#encoded =  rlp.encode([ hex(txn_json['nonce']), hex(txn_json['gasPrice']), hex(txn_json['gas']),  hex(txn_json['value']), 0x0, 0x1, 0x0, 0x0 ])
#print (encoded.hex())
#encoded =  rlp.encode([ hex(txn_json['nonce']), hex(txn_json['gasPrice']), hex(txn_json['gas']),  hex(txn_json['value']),  hex(txn_json['data']), 0x1, 0x0, 0x0 ])

tx_v = chain_id
tx_r = 0
tx_s = 0
#https://github.com/hyperledger/besu/issues/1240
#Legacy Transactions:
#rlp([nonce, gasPrice, gasLimit, to, value, data]) // unsigned
#rlp([nonce, gasPrice, gasLimit, to, value, data, v, r, s]) // signed
encoded = rlp.encode([nonce, gas_price, gas_limit,  bytearray.fromhex(contract_address_g5[2:]), 0x0, bytearray.fromhex(fhash2[2:]), tx_v, tx_r, tx_s])
print (f"RLP unsigned, no v,r,s: \n{encoded.hex()}")

secret=int(from_priv_key, 16)
sk = SigningKey.from_secret_exponent(secret, curve = SECP256k1)
hk = keccak =  w3.to_hex(w3.keccak(encoded))
print (hk)
signed_det = sk.sign_digest_deterministic( bytearray.fromhex(hk[2:]), hashfunc = sha3.keccak_256) # hashlib.sha3_256  # hashfunc=hashlib.sha256)
print ("Signed with deterministic digest:")
print (signed_det.hex())

order = SECP256k1.order
tx_r,tx_s = util.sigdecode_string(signed_det, order) #
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

encoded = rlp.encode([nonce, gas_price, gas_limit,  bytearray.fromhex(contract_address_g5[2:]), 0x0, bytearray.fromhex(fhash2[2:]), v, tx_r, tx_s])
print (f"RLP signed: \n{encoded.hex()}")

#exit()

signed_txn = w3.eth.account.sign_transaction(txn_json, from_priv_key)
#signed_tx =  w3.eth.sign(w3.eth.default_account,hexstr="0xea3d8504a817c800831e848094ffcf8fdee72ac11b5c542428b35eef5769c409f0830186a080820a958080")
print (signed_txn)
print ('Raw transaction :\n' + signed_txn.rawTransaction.hex())
#print (signed_tx.hex())

#exit()

tx_raw = encoded #'0xf86a808504a817c800831e848094ffcf8fdee72ac11b5c542428b35eef5769c409f0830186a080820a95a01781691241d8e12c80b1f0b01b781a36ef473495081bb072c00f0c3aec1febcba028661ce7e924d8e36d32d1be670ec74bee1125ae006a9e9d280998728b952c5d'
#signed_txn.rawTransaction = bytearray.fromhex(tx_raw[2:])
txn_hash = w3.eth.send_raw_transaction(tx_raw)
keccak =  w3.to_hex(w3.keccak(signed_txn.rawTransaction))
print (f"keccak hash:      {keccak}")  # the same as transaction hash 
txn = w3.eth.get_transaction(txn_hash)

#txn_hash = w3.eth.send_raw_transaction(signed_txn.rawTransaction)
#print (f'Transaction hash: {txn_hash.hex()}')
#txn = w3.eth.get_transaction(txn_hash)

#print (txn)
txn_json = json.loads(w3.to_json(txn))
print  ('Transaction details: \n' + str(txn_json))

tx_receipt = w3.eth.wait_for_transaction_receipt(txn_hash)
print(f"Contract TX receipt:\n{tx_receipt}") # Optional

# do by web3
# https://github.com/soos3d/call-smart-contract-functions-using-web3.py
# nonce = w3.eth.get_transaction_count(from_addr)
# call_function = contract_g5.functions.get_output().build_transaction({"chainId": chain_id, "from": from_addr, "nonce": nonce})
# call_function = contract_g5.functions.set_output(1).build_transaction({"chainId": chain_id, "from": from_addr, "nonce": nonce})
# call_function = contract_g5.functions.set_string("me",2).build_transaction({"chainId": chain_id, "from": from_addr, "nonce": nonce})
# print (f"Web3 call function:\n{call_function}")

# Call function:
# {'value': 0, 'gas': 21858, 'maxFeePerGas': 1093240212, 'maxPriorityFeePerGas': 1000000000, 'chainId': 1337, 
# 'from': '0x90F8bf6A479f320ead074411a4B0e7944Ea8c9C1', 'nonce': 23, 
# 'to': '0xe78A0F7E598Cc8b0Bb87894B0F60dD2a88d6a8Ab', 'data': '0x54f8a2f2'}
# i.e. 0x54f8a2f2 - is method ID. without parameters
# https://emn178.github.io/online-tools/keccak_256.html - get_output() -> 54f8a2f2e25ebc32f74a6992431cc4a8fd1f1a388cdde3f6ee6f5d23a9796268

#signed_tx = w3.eth.account.sign_transaction(call_function, private_key=from_priv_key)
#send_tx = w3.eth.send_raw_transaction(signed_tx.rawTransaction)
#tx_receipt = w3.eth.wait_for_transaction_receipt(send_tx)
#print(f"Contract TX receipt:\n{tx_receipt}") # Optional
