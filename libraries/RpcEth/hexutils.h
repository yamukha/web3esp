// hex utilities to convert and print

#ifndef HEXUTILS_H
#define HEXUTILS_H

#include <string>
#include <vector>
#include <sstream>

#include <DefineEth.h>

std::vector<uint8_t> hex2bytes (std::string hex) {
    std::vector<uint8_t> bytes;
    for (unsigned int i = 0; i < hex.length(); i += 2) {
      std::string byteString = hex.substr(i, 2);
      uint8_t byte = (uint8_t) strtol(byteString.c_str(), NULL, 16);
      bytes.push_back(byte);
    }
    return bytes;
}

void byte_to_hex(uint8_t b, char s[23])
{
    unsigned i = 1;
    s[0] = s[1] = '0';
    s[2] = '\0';
    while (b)
    {
        unsigned t = b & 0x0f;
        if (t < 10)
        {
            s[i] = '0' + t;
        }
        else
        {
            s[i] = 'a' + t - 10;
        }
        i--;
        b >>= 4;
    }
}

std::string arr_to_hex_str(uint8_t *arr, size_t len)
{
    std::string s = "";
    size_t i;
    for (i = 0; i < len; i++)
    {
        char sb[2];
        byte_to_hex(arr[i], sb);
        s += sb;
    }
    return s;
}

void print_hex(const uint8_t *data, size_t data_len)
{
    for (int i = 0; i < data_len; i++)
    {
        log_printf("%02x", data[i]);
    }
}

void int_to_hex(int num, char *arr)
{
    sprintf(arr, "0x%x", num);
}

std::string int_to_hex_str(int n)
{
    std::stringstream stream;
    stream << std::hex << n;
    std::string result(stream.str());
    if (result.size() % 2)
        result = "0x" + result;
    return result;
}

#endif
