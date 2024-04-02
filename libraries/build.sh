#!/bin/bash

g++ esp_ecsda.c rlp/RLP.cpp sha3/sha3.cpp micro-ecc/uECC.c RpcEth/DefineEth.h RpcEth/hexutils.h RpcEth/ethsign.h RpcEth/JsonUtilsEth.h -o ecdsa -fpermissive -ljsoncpp -IRpcEth  -Irlp -Isha3 -Imicro-ecc -DUSE_LINUX -DDEBUG_PRINT
