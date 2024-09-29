#!/bin/bash

if [[ "$1" == "" ]]
then
JLIB="json"
else
JLIB=$1
fi

echo $JLIB

if [[ "$JLIB" == "jsoncpp" ]]
then
   # g++ esp_ecsda.c rlp/RLP.cpp sha3/sha3.cpp micro-ecc/uECC.c RpcEth/DefineEth.h RpcEth/hexutils.h RpcEth/ethsign.h RpcEth/JsonUtilsEth.h -o ecdsa -fpermissive -ljsoncpp -IRpcEth  -Irlp -Isha3 -Imicro-ecc -DUSE_LINUX -DDEBUG_PRINT
   g++ esp_ecsda.c rlp/RLP.cpp sha3/sha3.cpp micro-ecc/uECC.c RpcEth/DefineEth.h RpcEth/hexutils.h RpcEth/ethsign.h RpcEth/JsonUtilsEth.h secp256k1-embedded/src/ethc.cpp -o ecdsa -fpermissive -lsecp256k1 -ljsoncpp -IRpcEth  -Irlp -Isha3 -Imicro-ecc -DUSE_LINUX -DDEBUG_PRINT
fi

if [[ "$JLIB" == "json11" ]]
then
   # g++ esp_ecsda.c json11/json11.cpp rlp/RLP.cpp sha3/sha3.cpp micro-ecc/uECC.c RpcEth/DefineEth.h RpcEth/hexutils.h RpcEth/ethsign.h RpcEth/JsonUtilsEth.h -o ecdsa -fpermissive -IRpcEth  -Irlp -Isha3 -Imicro-ecc -DUSE_LINUX -DDEBUG_PRINT -DUSE_JSON11
   g++ esp_ecsda.c json11/json11.cpp rlp/RLP.cpp sha3/sha3.cpp micro-ecc/uECC.c RpcEth/DefineEth.h RpcEth/hexutils.h RpcEth/ethsign.h RpcEth/JsonUtilsEth.h secp256k1-embedded/src/ethc.cpp -o ecdsa -fpermissive -lsecp256k1 -IRpcEth  -Irlp -Isha3 -Isecp256k1-embedded/src/include -Imicro-ecc -DUSE_LINUX -DUSE_JSON11 -DDEBUG_PRINT
fi

if [[ "$JLIB" == "json" ]] # nlohmann::json 
then
   # g++ esp_ecsda.c rlp/RLP.cpp sha3/sha3.cpp micro-ecc/uECC.c RpcEth/DefineEth.h RpcEth/hexutils.h RpcEth/ethsign.h RpcEth/JsonUtilsEth.h -o ecdsa -fpermissive -IRpcEth  -Irlp -Isha3 -Imicro-ecc -DUSE_LINUX -DDEBUG_PRINT -DUSE_JSON
   g++ esp_ecsda.c rlp/RLP.cpp sha3/sha3.cpp micro-ecc/uECC.c RpcEth/DefineEth.h RpcEth/hexutils.h RpcEth/ethsign.h RpcEth/JsonUtilsEth.h secp256k1-embedded/src/ethc.cpp -o ecdsa -fpermissive -lsecp256k1 -IRpcEth  -Irlp -Isha3  -Isecp256k1-embedded/src/include -Imicro-ecc -DUSE_LINUX -DDEBUG_PRINT -DUSE_JSON -DDEBUG_PRINT
fi

ls -la ./ecdsa