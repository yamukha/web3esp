/*
  TX.h - Transaction library for TX functions
*/
#ifndef TX_h
#define TX_h
#include <string>
struct TX
{
  TX() : r("0"), s("0"), data("0"), value("0"), gasLimit("0x1e8480") {};
  std::string nonce;
  std::string gasPrice;
  std::string gasLimit; // 2000000 -> "0x1e8480"
  std::string to;
  std::string value;
  std::string data;
  std::string v;
  std::string r;
  std::string s;
};

#endif
