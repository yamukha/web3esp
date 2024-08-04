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
   g++ esp_ecsda.c rlp/RLP.cpp sha3/sha3.cpp micro-ecc/uECC.c RpcEth/DefineEth.h RpcEth/hexutils.h RpcEth/ethsign.h RpcEth/JsonUtilsEth.h -o ecdsa -fpermissive -ljsoncpp -IRpcEth  -Irlp -Isha3 -Imicro-ecc -DUSE_LINUX -DDEBUG_PRINT
fi

if [[ "$JLIB" == "json11" ]]
then
   g++ esp_ecsda.c json11/json11.cpp rlp/RLP.cpp sha3/sha3.cpp micro-ecc/uECC.c RpcEth/DefineEth.h RpcEth/hexutils.h RpcEth/ethsign.h RpcEth/JsonUtilsEth.h -o ecdsa -fpermissive -IRpcEth  -Irlp -Isha3 -Imicro-ecc -DUSE_LINUX -DDEBUG_PRINT -DUSE_JSON11
fi

if [[ "$JLIB" == "json" ]] # nlohmann::json 
then
   g++ esp_ecsda.c rlp/RLP.cpp sha3/sha3.cpp micro-ecc/uECC.c RpcEth/DefineEth.h RpcEth/hexutils.h RpcEth/ethsign.h RpcEth/JsonUtilsEth.h -o ecdsa -fpermissive -IRpcEth  -Irlp -Isha3 -Imicro-ecc -DUSE_LINUX -DDEBUG_PRINT -DUSE_JSON
fi

ls -la ./ecdsa