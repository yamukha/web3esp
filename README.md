# Solution to submit transaction to ETH chain using microcontroller like ESP8266 or ESP32

# Inplemented extrinsic methods
Transfer balance as Raw Transaction\
Smart contracts: ref. libraries/contracts/README.md

# Implementetiaon
Refference pyton scripts: web3eth.py (high level) and rlp4eth.py (low level, based on secp256k1 library for ECDSA sign)\
C++ Linux (supports micro-ECC and secp256k1) and ESP platform (supports micro-ECC, secp256k1 and Trezor).\
Library for ECDSA is defined in Defines.h:\
ESP8266: micro-ECC,\
ESP32: micro-ECC, secp256k1 and Trezor,\
Linux:  micro-ECC and secp256k1 (Trezor is not supported).\
micro-ECC based implementation is the most robust (EPS32 with secp256k1 and Trezor reboots after some count of transactions).

# Dependency
Python: imported libs in scripts\
C++: sudo apt install libsecp256k1-dev libjsoncpp-dev

# Build 
1. Linux (ref. Deploy section to proper configuration):\
run build script: 
 ./build.sh
2. ESP32 (Arduino IDE under Windows) or arduino-cli (as Linux command line, ref to CI sripts)

# Testing chain: ganache
Install ganache-cli, note: maybe need to install proper vesion of nmp\
Run ganache as deterministic chain node (with the same private key every new start:)\
ganache-cli -d -h 0.0.0.0
In case of smart contract usage is needed to upload it, i.e. by Remix to local host node.

# Test cases:
1. Test on Linux and python implementation (ref. to cripts code):
2. Test on Linux C++ implementation:\
build linux (using ./build.sh) binary and then run python script:\
python3 ethraw.py [rpcId] [ethHost] [privKey]\
i.e.\
python3 ethraw.py 1 http://192.168.0.106:8545 4f3edf983ac636a65a842ce7c78d9aa706d3b113bce9c46f30d7d21715b23b1d
3. Test with ESP implementation:\
Ref. to deploy section.\
Note: ganache chain node and ESP should be in the same subnet
4. Unit tests for RPL and ABI serialization:\
python3 test_rpl.py

# CI
There are in .github/workflows/ files:\
web3esp.yml \
to check in local PC run:\
act -j esp

# Deploy to  ESP
1. Set up in DefineEth.h Linux or ESP mode (target ESP8266 or ESP32), proper ECDSA library and it entropy settings.
2. Set up in DefineEth.h ethereum host address and port, i.e. ETHURL  "http://192.168.0.106:8545"
3. Copy template Private.h to PrivateEth_dummy.h set proper STASSID, STAPSK and PRIV_ETH_KEY macro definitionss.
4. Uncomment #define DUMMY_KEYS in Arduino ino file to use own credentials from PrivateEth_dummy.h
4. Build for selected target.
5. Flash target.
6. Run and check output in command line terminalr or in serial monitor.

# Online checkers:
https://www.ethereumdecoder.com/ - to check transaction online\
https://emn178.github.io/online-tools/keccak_256.html  -- check online as hex

https://www.ethereumdecoder.com/                              -- RPL online decoder to check\
https://toolkit.klaytn.foundation/transaction/rlpDecode       -- online rlp\
https://codechain-io.github.io/rlp-debugger/                  -- onlie rlp\
https://toolkit.abdk.consulting/ethereum                                       

# Some of main used sources:
https://github.com/brainhub/SHA3IUF                       -- keccak-256 hash\
https://github.com/kmackay/micro-ecc                      -- modified to provide v for recid calculation\
https://github.com/diybitcoinhardware/secp256k1-embedded  -- secp256k1 port for small embedded systems\
https://github.com/kvhnuke/Ethereum-Arduino               -- RLP for ESP\
https://github.com/AlphaWallet/Web3E                      -- Trezor for ESP\
https://github.com/mhw0/libethc                           -- secp256k1 wrapper
