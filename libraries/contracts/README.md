# Smart Contracts Support
There is implemented solution to create, sign and transmit smart contracts directly from ESP8266/ESP32\
Limited set of ABI types allows to submit to blockchain sensor data, alarms, short messages etc.

# Supported ABI types
- static types: address, bool, (u)int<M> (up 64 bits), bytes<M>
- dynamic types: string (with size up 32 bytes in two notations:
 list/array of of chars, i.e.: [0 0x2 33 42]\
 as string literal, i.e: "test string"\
 Examples to build different smart contract calldata can be seen in ../esp_ecsda.c in comments

# Example of Smart Contract Usage
As example used testing smart contract g5.sol\
You can create own one using this for reference by following steps from bellow.

## Used Tools
Remix: https://remix-project.org/
Ganache: https://archive.trufflesuite.com/ganache/ or ganache-cli: https://docs.nethereum.com/en/latest/ethereum-and-clients/ganache-cli/

## Tools Setup
Install Remix IDE and ganache/ganache-cli
Run Remix IDE 
Run ganache/ganache-cli

## Deploy smart contract to test net
Open g5.sol in Remix IDE
Compile and deploy it to real or test net (i.e. to ganache). Remember address of smart contract.

From Remix copy json describing ABI (g5.abi) and put it to to C header file (contract_g5.h).

# Testing
## Environment 
Test environment is the same as in testing section of main README

## Usage of  Smart Contract Class (create, load ABI, build, sign, send)
For ESP32/ESP8266 use rpcjson.ino as the main file.\
For Linux platform use esp_ecsda.c as a main file.\
To use smart contract set up isContract to true.\
Call smart contract constructor, extract methods from it's ABI description and build calldata for one of selected method, i.e.:
```
Contract c;
std::vector <ScmSig> scmSig = c.abiParser(contract_g5);
std::string bytes = "[" + std::to_string(cnt/256) + " " + std::to_string(cnt%256) + "]"; // use counter current value
std::string m = c.buildMethod("%s(%s)","set_bytes", bytes.c_str());
auto cd = c.doCall(m);
```
Fill up tx.* fields (i.e. with contract address from Deploy section).\
Created smart contract transaction will be signed and sent to blockchain.

## Test on the Target
Build binary (and flash to ESP if it is targeted platform) and run.

For Linux platform\
build:
```
./build.sh
```
run:
```
python3 ethraw.py
```
