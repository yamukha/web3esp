import subprocess, sys
import json

def isHex(s):
    try:
        int(s, 16)
        return True
    except ValueError:
        return False

def checkTxRpl (inTx, outTx, is_signed, is_contract):
  testRes = True
  sTx = outTx [1:-1].split(',')
  
  # compare nonce
  if int("0x"+sTx[0][1:-1], 16 ) != int(inTx[0], 16):
    print(f'nonce {sTx[0]} != {inTx[0]}')
    testRes = False
   
  # compare gasPrice
  if int("0x"+sTx[1][1:-1], 16 ) != int(inTx[3], 16):
    print(f'gasPrice {sTx[1]} != {inTx[3]}')
    testRes = False
  
  # compare value
  if not is_contract:
    if int("0x"+sTx[4][1:-1], 16 ) != int(inTx[5], 16):
      print(f'value {sTx[4]} != {inTx[5]}')
      testRes = False
  else:
    if sTx[4][1:-1] != "":
      print(f's {sTx[4]} != ""')
      testRes = False
  
  # compare to
  if "0x"+sTx[3][1:-1].lower() != inTx[4].lower():
    print(f'to {sTx[3]} != {inTx[4]}')
    testRes = False
  
  if not is_signed:
    # compare r
    if sTx[7][1:-1] != "":
      print(f'r {sTx[7]} != ""')
      testRes = False 
    # compare s
    if sTx[8][1:-1] != "":
      print(f's {sTx[8]} != ""')
      testRes = False
    # compare v
    if int("0x"+sTx[6][1:-1], 16 ) != int(inTx[1], 16):
      print(f'chainID {sTx[6]} != {inTx[1]}')
      testRes = False
  else:
    # compare r
    if len(sTx[7][1:-1]) != 64:
      print(f'r size of {sTx[7]} is {len(sTx[7][1:-1])} and not == 64')
      testRes = False 
    # compare s
    if len(sTx[8][1:-1]) != 64:
      print(f's size of {sTx[8]} is {len(sTx[8][1:-1])} and not == 64')
      testRes = False
    # compare v chainID
    v0 = int(inTx[1], 16) * 2 + 35
    v1  = v0 + 1
    vTx = int("0x"+sTx[6][1:-1], 16 ) 
    if vTx != v0 and vTx != v1:
      print(f'chainID {sTx[6]} != {hex(v0)} or {hex(v1)}')
      testRes = False

    # compare data
    if not is_contract:
      if not isHex("0x"+sTx[4][1:-1]):
        print(f'data {sTx[4]} is not hex ')
        testRes = False
      #else:
      # ? TODO test ABI

  return testRes

grep_unsigned = " | grep -A1 'RPL of unsigned transaction' | tail -1"
grep_signed   = " | grep -A1 'RPL final transaction:' | tail -1"
grep_call     = " | grep -A1 'Calldata:' | tail -1"

testOK = True

# nonce, chainId, privKey, gasPrice, to , value, 
classicTx = ["0xfffeff", "0x539", "4f3edf983ac636a65a842ce7c78d9aa706d3b113bce9c46f30d7d21715b23b1d", "0x04a817c800", "0xFFcf8FDEE72ac11b5c542428B35EEF5769C409f0", "0x0386a0"]
contractTx = ["0xfffeff", "0x539", "4f3edf983ac636a65a842ce7c78d9aa706d3b113bce9c46f30d7d21715b23b1d", "0x77359400", "0xe78a0f7e598cc8b0bb87894b0f60dd2a88d6a8ab", ""]

# cmd = "./ecdsa " + classicTx[0] + " " + classicTx[1] + " " + classicTx[2] + " " + " 'set_output(3)'
#                      nonce              chainId       rpcId       pKey
cmd = "./ecdsa " + classicTx[0] + " " + classicTx[1] + " 12 " + classicTx[2] + " " + grep_unsigned
res = subprocess.check_output (cmd, shell = True, executable="/bin/bash")
rpl = str(res,'utf-8').rstrip()
print(f'RPL: {rpl}')

rlpcmd = "rlp decode " + rpl
rsp = subprocess.check_output (rlpcmd, shell = True, executable="/bin/bash")
tx = str(rsp,'utf-8').rstrip()
print(f'TX: {tx}')

# not signed, not smart contract call
if not checkTxRpl(classicTx, tx, False, False):
  print('Failed unsigned classic transaction RPL encoding')
  testOK = False

cmd = "./ecdsa " + classicTx[0] + " " + classicTx[1] + " " + classicTx[2] + " " + grep_signed
res = subprocess.check_output (cmd, shell = True, executable="/bin/bash")
rpl = str(res,'utf-8').rstrip()
print(f'RPL: {rpl}')

rlpcmd = "rlp decode " + rpl
rsp = subprocess.check_output (rlpcmd, shell = True, executable="/bin/bash")
tx = str(rsp,'utf-8').rstrip()
print(f'TX: {tx}')

# signed, not smart contract call
if not checkTxRpl(classicTx, tx, True, False):
  print('Failed signed classic transaction RPL encoding')
  testOK = False

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
    cmd = "./ecdsa " + classicTx[0] + " " + classicTx[1] + " 12 " + classicTx[2] + " " + sm_call + grep_unsigned
    res = subprocess.check_output (cmd, shell = True, executable="/bin/bash")
    rpl = str(res,'utf-8').rstrip()
    print(f'RPL: {rpl}')

    rlpcmd = "rlp decode " + rpl
    rsp = subprocess.check_output (rlpcmd, shell = True, executable="/bin/bash")
    tx = str(rsp,'utf-8').rstrip()
    print(f'TX: {tx}')

    # not signed, smart contract call
    if not checkTxRpl(contractTx, tx, False, True):
      print('Failed for unsigned smart contract transaction RPL encoding')
      testOK = False

    cmd = "./ecdsa " + classicTx[0] + " " + classicTx[1] + " 12 " + classicTx[2] + " " + sm_call + grep_signed
    res = subprocess.check_output (cmd, shell = True, executable="/bin/bash")
    rpl = str(res,'utf-8').rstrip()
    print(f'RPL: {rpl}')

    rlpcmd = "rlp decode " + rpl
    rsp = subprocess.check_output (rlpcmd, shell = True, executable="/bin/bash")
    tx = str(rsp,'utf-8').rstrip()
    print(f'TX: {tx}')

    # signed, smart contract call
    if not checkTxRpl(contractTx, tx, True, True):
      print('Failed for signed smart contract transaction RPL encoding')
      testOK = False

#   contract       types     hash 
abi_conracts = [
    ["'get_output()'", "", "0x54f8a2f2"] , 
    ["'set_output(3)'", "uint256", "0xe4db51aa"], 
    ["'set_output(3,15)'", "uint256,uint256", "0x2d043b1b"],
    ["'set_bool(true)'", "bool", "0xa66162a6"],  ["'set_bool(false)'", "bool", "0xa66162a6"],
    ["'set_allowed(0xD028ec274Ef548253a90c930647b74C830Ed4b4F,true)'", "address,bool", "0x6d075314"],
    ["\"set_bytes('one')\"", "bytes14", "0x4126b45c"],
    ["\"set_bytes('hello bytes 42')\"",  "bytes14", "0x4126b45c"],
    ["\"set_bytes([0 1 0x2 0x03 4 5 6 7 8 9 10 11 12 13 14])\"",  "bytes14", "0x4126b45c"],
    ["\"set_bytes([0 1 0x2 0x03 4 5 6 7 8 9 10 11 12 13 14 15 16 0x11 18 19 20 21 22 23 24 25 26 27 28 29 30 fg 0x100 256 31])\"",  "bytes14", "0x4126b45c"],
    ["\"set_string('Hello robo')\"", "string", "0xc9615770"],
    ["\"set_string('some parameter maximum value = ',1234567890123444556)\"", "string,uint256", "0xa4965edc"]
]

for sm_call in abi_conracts:
    cmd = "./ecdsa " + classicTx[0] + " " + classicTx[1] + " 12 " + classicTx[2] + " " + sm_call[0] + grep_call
    res = subprocess.check_output (cmd, shell = True, executable="/bin/bash") #, stdout = subprocess.STDOUT)
    abi = str(res,'utf-8').rstrip()
    print(abi)

    abicmd = "abi-decode -d " + abi + " -t function:" + sm_call[1]
    
    rsp = subprocess.check_output (abicmd, shell = True, executable="/bin/bash")
    cd = str(rsp,'utf-8').rstrip()
    print(cd)
    sc_res = cd.split("\n")
    sc_call = sc_res[0].split(":")
    sc_hash = sc_call[1].strip()
    if sc_hash != sm_call[2]:
      print(f'hash {sc_hash} != {sm_call[2]}')
      testOK = False

if  testOK:
    exit(0)
else:
   exit(1)