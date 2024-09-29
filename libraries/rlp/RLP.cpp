/*
  RLP.cpp - RLP library for RLP functions
*/
#include "RLP.h"
using namespace std;

string RLP::encode(string s)
{
  if (s.size() == 1 && (unsigned char)s[0] == 0)
    return encodeLength(0, 128);

  if (s.size() == 1 && (unsigned char)s[0] < 128)
    return s;
  else
  {
    return encodeLength(s.size(), 128) + s;
  }
}

string RLP::encodeStrLong(string str)
{
  std::string temp = "";
  int bytes = 0;
  if (str.length() < 256)
    bytes = 1;
  else if (str.length() > 256 && str.length() < 256 * 256)
    bytes = 2;

  temp = (char)(RLP_BLOCK_PREFIX_B7 + bytes);

  if (bytes == 1)
  {
    std::string msb = "";
    msb = (char)(str.length());
    return temp + msb + str;
  }
  else // data lenth limited to 2 bytes
  {
    std::string msb = "";
    std::string lsb = "";
    msb = (char)(str.length() / 256);
    lsb = (char)(str.length() % 256);
    return temp + msb + str;
  }
}

string RLP::encode(TX tx, bool toSign) // or to send
{
  if (tx.data.length() < RLP_BLOCK_0_TO_55)
  {
    string serialized = hexToRlpEncode(tx.nonce) +
                        hexToRlpEncode(tx.gasPrice) +
                        hexToRlpEncode(tx.gasLimit) +
                        hexToRlpEncode(tx.to) +
                        hexToRlpEncode(tx.value) +
                        hexToRlpEncode(tx.data) +
                        hexToRlpEncode(tx.v) +
                        hexToRlpEncode(tx.r) +
                        hexToRlpEncode(tx.s);

    if (toSign) // serialized without header
      return serialized;
    else // with header for signed raw transaction
      return hexToBytes(encodeLength(serialized.length(), 192)) + serialized;
  }
  else
  {
    string serialized = hexToRlpEncode(tx.nonce) +
                        hexToRlpEncode(tx.gasPrice) +
                        hexToRlpEncode(tx.gasLimit) +
                        hexToRlpEncode(tx.to) +
                        hexToRlpEncode(tx.value) +
                        hexToRlpEncodeStrLong(tx.data) +
                        hexToRlpEncode(tx.v) +
                        hexToRlpEncode(tx.r) +
                        hexToRlpEncode(tx.s);

    if (toSign) // serialized without header
      return serialized;
    else // with header for signed raw transaction
      return hexToBytes(encodeLength(serialized.length(), 192)) + serialized;
  }
}

string RLP::hexToBytes(string s)
{
  char inp[s.length()] = {};
  memcpy(inp, s.c_str(), s.length());
  char dest[sizeof(inp) / 2] = {};
  hex2bin(inp, dest);
  return string(dest, sizeof(dest));
}
string RLP::hexToRlpEncode(string s)
{
  s = removeHexFormatting(s);
  if (1 == s.length() % 2) // fix to add extra leading zero
    s = "0" + s;

  return encode(hexToBytes(s));
}

string RLP::hexToRlpEncodeStrLong(string &s)
{
  s = removeHexFormatting(s);
  if (1 == s.length() % 2) // fix to add extra leading zero
    s = "0" + s;

  return encodeStrLong(hexToBytes(s));
}

string RLP::removeHexFormatting(string s)
{
  if (s[0] == '0' && s[1] == 'x')
    return s.substr(2, s.length() - 2);
  return s;
}

string RLP::encodeLength(int len, int offset)
{
  string temp;
  if (len < 56)
  {
    temp = (char)(len + offset);
    return temp;
  }
  else
  {
    string hexLength = intToHex(len);
    int lLength = hexLength.length() / 2;
    string fByte = intToHex(offset + 55 + lLength);
    return fByte + hexLength;
  }
}

string RLP::LengthHeader(std::string &str)
{
  string temp;
  if (str.length() / 2 < RLP_BLOCK_0_TO_55)
  {
    int encLength = RLP_BLOCK_PREFIX_C0 + str.length() / 2;
    // printf("Encoded Length %x\n", encLength);
    return intToHex(encLength);
  }
  else
  {
    int bytes = 0;
    if (str.length() / 2 < 256)
      bytes = 1;
    else if (str.length() / 2 > 256 && str.length() / 2 < 256 * 256)
      bytes = 2;

    int encLength = RLP_BLOCK_PREFIX_F7 + bytes;
    temp = intToHex(encLength) + intToHex(str.length() / 2);
    // printf("Encoded Length %s\n", temp.c_str());
    return temp;
  }
}

string RLP::intToHex(int n)
{
  stringstream stream;
  stream << std::hex << n;
  string result(stream.str());
  if (result.size() % 2)
    result = "0" + result;
  return result;
}
string RLP::bytesToHex(string input)
{
  static const char *const lut = "0123456789ABCDEF";
  size_t len = input.length();
  std::string output;
  output.reserve(2 * len);
  for (size_t i = 0; i < len; ++i)
  {
    const unsigned char c = input[i];
    output.push_back(lut[c >> 4]);
    output.push_back(lut[c & 15]);
  }
  return output;
}
int RLP::char2int(char input)
{
  if (input >= '0' && input <= '9')
    return input - '0';
  if (input >= 'A' && input <= 'F')
    return input - 'A' + 10;
  if (input >= 'a' && input <= 'f')
    return input - 'a' + 10;
  return -1;
}
void RLP::hex2bin(const char *src, char *target)
{
  while (*src && src[1])
  {
    *(target++) = char2int(*src) * 16 + char2int(src[1]);
    src += 2;
  }
}
