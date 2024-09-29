/*
  RLP.h - RLP library for RLP functions
*/
#ifndef RLP_h
#define RLP_h
#include <stdio.h>
#include <string>
#include <iomanip>
#include <sstream>
#include <memory.h>
#include "TX.h"

#define RLP_BLOCK_PREFIX_C0 192
#define RLP_BLOCK_PREFIX_B7 183
#define RLP_BLOCK_PREFIX_F7 247
#define RLP_BLOCK_0_TO_55 56

class RLP
{
public:
	std::string encode(std::string);
	std::string encodeStrLong(std::string);
	std::string encode(struct TX, bool);
	std::string encodeLength(int, int);
	std::string intToHex(int);
	std::string bytesToHex(std::string);
	std::string removeHexFormatting(std::string);
	std::string hexToRlpEncode(std::string);
	std::string hexToRlpEncodeStrLong(std::string &);
	std::string hexToBytes(std::string);
	std::string LengthHeader(std::string &);
	int char2int(char);
	void hex2bin(const char *, char *);
	void testPub();

private:
};

#endif
