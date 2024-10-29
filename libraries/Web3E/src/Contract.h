#ifndef CONTRACT_H
#define CONTRACT_H

#include <algorithm>
#include <string>
#include <DefineEth.h>
#include <sha3.h>
#include <hexutils.h>
#include <set>

#ifdef USE_JSON_OR_JSON11
#ifdef USE_JSON11

#ifdef USE_LINUX
#include "../json11/json11.hpp"
#endif

#ifdef USE_ARDUINO
#include <json11.hpp>
#endif

#endif
#endif

struct CallData {
    std::string stat;
    std::string dynamic;
    std::string fhash;
};

// smart contract method signature model
struct ScmSig {
    std::string fname; // smart contract method name
    std::vector <std::pair<std::string, std::string>> fargs; // smart contract method args
};

class Contract {
  public:
#ifdef USE_JSON
  std::vector <ScmSig> abiParser(const std::string & sabi)
  {
    using json = nlohmann::json;
    std::vector <ScmSig> flist;
    json jabi = json::parse(sabi);

    for (auto& item : jabi)
    {
        std::string fname = item ["name"];
        log_printf("%s\n", fname.c_str());
        std::vector <std::pair<std::string, std::string>> plist; // args|parameters list
        for (auto& arg : item["inputs"]) {
             std::string aname = arg ["name"];
             std::string atype = arg ["type"];
             auto el = std::make_pair(aname, atype);
             log_printf("%s %s\n", aname.c_str(), atype.c_str());
             plist.push_back(el);
        }
        ScmSig scm;
        scm.fname = fname;
        scm.fargs = plist;
        flist.push_back(scm);
    }
    scmSig_ =  flist;
    return flist;
  };
#endif

#ifdef USE_JSON11
  std::vector <ScmSig> abiParser(const std::string & sabi)
  {
    std::vector <ScmSig> flist;
    std::string err;
    auto jabi = json11::Json::parse(sabi,err).array_items();

    for (auto& item : jabi)
    {
        std::string fname = item ["name"].string_value();
        log_printf("%s\n", fname.c_str());
        std::vector <std::pair<std::string, std::string>> plist; // args|parameters list
        for (auto& arg : item["inputs"].array_items()) {
             std::string aname = arg ["name"].string_value();
             std::string atype = arg ["type"].string_value();
             auto el = std::make_pair(aname, atype);
             log_printf("%s %s\n", aname.c_str(), atype.c_str());
             plist.push_back(el);
        }
        ScmSig scm;
        scm.fname = fname;
        scm.fargs = plist;
        flist.push_back(scm);
    }

    scmSig_ =  flist; // TODO: move to constructor
    return flist;
  }
#endif

  std::vector <ScmSig> scmSig_;

  void AllFunctions(std::vector <ScmSig> scmSig) {
    for (auto& item : scmSig) {
      auto i =  &item  - &scmSig[0];
      log_printf("method %ld:  %s\n", i ,item.fname.c_str());
    }
  };
  
  template<typename ... Args>
  std::string buildMethod( const std::string& format, Args ... args )
  {
    int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    if( size_s <= 0 )
      log_printf("Error during smart contract formatting\n");
    
    auto size = static_cast<size_t>( size_s );
    std::unique_ptr<char[]> buf( new char[ size ] );
    std::snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // remove '\0'
  }

  /*
  std::vector<std::string> split (const std::string &s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss (s);
    std::string item;

    while (getline (ss, item, delim)) {
      result.push_back (item);
    }

    return result;
  }
  */

  std::vector<std::string> split(const std::string& str, char delim) {
    std::vector<std::string> strings;
    size_t start;
    size_t end = 0;
    while ((start = str.find_first_not_of(delim, end)) != std::string::npos) {
        end = str.find(delim, start);
        strings.push_back(str.substr(start, end - start));
    }
    return strings;
  }

  std::string funcHash (std::string &signature) {
    std::string fhash = "";
    bool found = false;
    int args = -1;
    auto fname = split(signature, '(');
    
    // get count of arguments
    if (signature.find("()") != std::string::npos){
      args = 0;
    } else 
      args = std::count(signature.begin(),signature.end(),',') + 1;

    if (scmSig_.size() > 0) {
        for (auto& item : scmSig_) {
            // log_printf("Check method  %s\n", item.fname.c_str());
            if (item.fname == fname[0] && item.fargs.size() == args) {
                found = true;
                std::string method = item.fname + '(';
                if (item.fargs.size() > 0) {
                    for (auto& arg : item.fargs) {
                        method += arg.second + ',';
                    }
                    method.pop_back();
                    method += ')';
                } else  
                    method += ')';
                // calculate method hash: https://emn178.github.io/online-tools/keccak_256.html    
                uint8_t pub_hash[KEYS_SIZE];
                do_keccak256(pub_hash, (const uint8_t*) method.c_str(), method.size());
                fhash = arr_to_hex_str(pub_hash, 4);
                log_printf("Method '%s' hash 0x%s\n", method.c_str(), fhash.c_str());
            } // if found             
        } // for
    } else 
      log_printf("Methods list is empty \n");

    if (!found) 
       log_printf("Cannot find method %s\n",fname[0].c_str());

    return fhash;
  };

// static_types
std::set<std::string> int_types = {"uint",                                               // as uint256
  "uint8", "uint16", "uint24", "uint32", "uint40", "uint48", "uint56", "uint64",
  "uint72", "uint80", "uint88", "uint96", "uint104", "uint112", "uint120", "uint128",
  "uint136", "uint144", "uint152", "uint160", "uint168", "uint176", "uint184", "uint192",
  "uint200", "uint208", "uint216", "uint224", "uint232", "uint240", "uint248", "uint256" };

std::set<std::string> uint_types = {"int",                                              // as int256
  "int8", "int16", "int24", "int32", "int40", "int48", "int56", "int64",
  "int72", "int80", "int88", "int96", "int104", "int112", "int120", "int128",
  "int136", "int144", "int152", "int160", "int168", "int176", "int184", "int192",
  "int200", "int208", "int216", "int224", "int232", "int240", "int248", "int256",
};

std::set<std::string> byte_types = {
  "bytes1","bytes2", "bytes3", "bytes4", "bytes5", "bytes5", "bytes7", "bytes8",
  "bytes9","bytes10", "bytes11", "byte12", "bytes13", "bytes14", "bytes15", "bytes16",
  "bytes17","bytes18", "bytes19", "bytes20", "bytes21", "bytes22", "bytes23", "bytes24",
  "bytes25","bytes26", "bytes27", "bytes28", "bytes29", "bytes30", "bytes31", "bytes32" };

std::set<std::string> bool_types = {"bool" }; // uint8
std::set<std::string> address_types = {"address"}; // as uint160
std::set<std::string> fixed_types = {"fixed", "ufixed"}; // as fixed128x18 from fixed<M>x<N> , ufixed128x18 from ufixed<M>x<N>
std::set<std::string> function_types = {"function"}; // bytes24

std::string number_to_hex_str32(uint64_t n)
{
    std::string result = "0000000000000000000000000000000000000000000000000000000000000000";
    
    std::stringstream stream;
    stream << std::uppercase << std::hex << n;
    std::string res(stream.str());
    if (res.size() < 64) {
        result.resize(64 - res.size());
        result += res;
    } else 
       result = res;

    return result;
};

std::string string_to_hex_str32(std::string &str)
{
    std::string result = "0000000000000000000000000000000000000000000000000000000000000000";
    
    std::stringstream stream;
    for (int i = 0; i < str.length(); i++)
      stream << std::hex << (int)str[i];

    std::string res(stream.str());
    if (res.size() < 64) {
        result.resize(64 - res.size());
        res += result ;
    }

    return res;
};

std::string bytes_to_hex_str32(std::string &str)
{
    std::string result = "0000000000000000000000000000000000000000000000000000000000000000";

    std::vector <std::string> v = split (str, ' ');
    std::string bytes = "";
    int i = 0;
    for (auto& b : v) {
      // check 0..0xFF & extends with leading zero
      if (b.size() > 2 && b.size() <= 4 && (b[0] == '0' && (b[1] == 'x' || b[1] == 'X') ) ) {
        if (b == "0x0" || b == "0x00") {
            bytes += "00";
        } else {
          char* p_end{};
          const long numb = std::strtol(b.c_str(), &p_end, 16);
          if (numb > 0 && numb <= 0xFF) {
            b.erase(b.begin());
            b.erase(b.begin());
            // extend with leading zero
            if (b.size() == 1)
              bytes += "0" + b;
            else
              bytes += b;
          } else {
            log_printf("Error: not in range 0..0xFF, cannot convert with strtol(%s)\n",b.c_str());
          }
        }
      } else if (b.size() > 0 && b.size() < 4) {
      // check 0..255 & decimal to hex & extends with leading zero
        if (b == "0" || b == "00"  || b == "000" ) {
            bytes += "00";
        } else {
          char* p_end{};
          const long numb = std::strtol(b.c_str(), &p_end, 10);
          if (numb > 0 && numb <= 0xFF) {
            char buff[4];
            std::sprintf(buff, "%02X", unsigned (numb));
            std::string hexstr(buff);
            bytes += hexstr;
          } else {
            log_printf("Error: not in range 0..255, cannot convert with strtol(%s)\n",b.c_str());
          }
        }
      }
      else {
        log_printf("Error: cannot convert %s\n",b.c_str());
      }
      if (bytes.size() >= 64) {
        if (b != v[v.size()-1])
          log_printf("Warn: some extra bytes can be lost after '%s' at position %d \n", b.c_str(), i);
        break;
      };
      i++;
    }
    std::string res = bytes;
    if (res.size() < 64) {
        result.resize(64 - res.size());
        res += result ;
    }

    return res;
};

CallData doCallData (std::string& method) {
    CallData calldata;
    std::string stat = "";
    std::string dyna = "";

    int args = -1;
    auto fname = split(method, '(');
    
    // get count of arguments
    if (method.find("()") != std::string::npos){
      args = 0;
    } else 
      args = std::count(method.begin(),method.end(),',') + 1;

    log_printf("Method has  %d args\n", args);

    uint64_t offset = 0;
    if (args > 0) {
      // get offset for dynamic part
      offset = 32 * args;
      log_printf("Dynamic part offset %lu\n", offset);

      auto parameters = split(fname[1], ')');  // kick off )
      auto params = split(parameters[0], ','); 
      for (auto& param : params) {
        log_printf("Parameter: %s\n", param.c_str());
      }
      int prm_counter = 0;
     
      if (scmSig_.size() > 0) {
        for (auto& item : scmSig_) {
            if (item.fname == fname[0] && item.fargs.size() == args) {
                log_printf("Matched %s with %d args\n", fname[0].c_str(), args);
                
                for (auto& it: item.fargs) {
                    if ( int_types.count(it.second) || uint_types.count(it.second) || bool_types.count(it.second)
                        || byte_types.count(it.second) || address_types.count(it.second)) {
                        std::string val = "0";
                        if (uint_types.count(it.second)) { //  for any of uint
                          auto number = std::stoull(params[prm_counter], nullptr, 10); // get signed parameter
                          val = number_to_hex_str32(number); // max. 1234567890123444556 -> 000000000000000000000000000000000000000000000000112210F47DE9514C
                        } else if (int_types.count(it.second)) { // for any of int
                          auto number = std::stoll(params[prm_counter], nullptr, 10);  // get signed parameter
                          val = number_to_hex_str32(number);
                        } else if (bool_types.count(it.second)) { // for bool
                          log_printf("Parameter bool: %s\n",it.second.c_str());
                          if (params[prm_counter] == "true") {
                            val = number_to_hex_str32(1);
                          } else {
                            val = number_to_hex_str32(0);
                        }
                        } else if (byte_types.count(it.second)) { // for any of bytes<M>, 0 < M <= 32, i.e. bytes3 = "abc"
                          std::string str = params[prm_counter];  // get real parameter
                          // if smth. like [0x0 0x42 ...] or [0 33 ...] handle as list of chars
                          if (str[0] == '[' && str[str.length() - 1] == ']') {  // handle as list of chars
                            str.pop_back();
                            str.erase(str.begin());
                            val = bytes_to_hex_str32(str);
                          } else {
                            if (str[0] == '"' || str[0] == '\'') {  // remove quotes
                              str.pop_back();
                              str.erase(str.begin());
                            }
                            log_printf("Parameter value is: %s\n", str.c_str());
                            val = string_to_hex_str32(str);       // "me" -> 6d65000000000000000000000000000000000000000000000000000000000000
                          }
                        }

                        log_printf("Parameter '%s' has static type '%s' with value %s\n", it.first.c_str(), it.second.c_str(), val.c_str());
                        if (it.second.find("bytes") != std::string::npos ) {
                          // get actual lenth to expected in bytes type
                          std::string bytes_cnt = it.second.substr(5); // length of "bytes"
                          log_printf("Expected size of type: %s %s\n", it.second.c_str(), bytes_cnt.c_str());
                          int bsz{std::atoi(bytes_cnt.c_str())};
                          if (bsz > 0) {
                            for (bsz; bsz < 32; bsz++) {
                              if (val[bsz *2] != '0' || val[bsz *2 +1] != '0'){
                                log_printf("%d:%c%c ",bsz, val[bsz *2], val[bsz * 2 + 1]);
                              };
                              val[bsz *2] = '0';
                              val[bsz *2 + 1] = '0';
                            };
                            log_printf("\n");
                          };
                        }
                        stat += val;
                        log_printf("Encoded static:\n%s\n", stat.c_str());
                        prm_counter++; 
                    }
                    else {
                        auto off = number_to_hex_str32(offset);
                        stat += off;
                        std::string str = "";                   
                        if (it.second == "string") {
                          str = params[prm_counter]; // get real parameter
                          if (str[0] == '"' || str[0] == '\'') {  // remove quotes  
                            str.pop_back();
                            str.erase(str.begin());
                          }
                          log_printf("Parameter value is: %s\n", str.c_str());
                        }
                        auto val = string_to_hex_str32(str);                  // "me" -> 6d65000000000000000000000000000000000000000000000000000000000000
                        auto dlx = number_to_hex_str32(str.size());
                        log_printf("Parameter '%s' has dynamic type '%s' with value %s\n", it.first.c_str(), it.second.c_str(), val.c_str());
                        dyna += dlx + val;
                        log_printf("Encoded dynamic:\n%s\n", dyna.c_str());
                        uint32_t dynamic_obj_size = 0;
                        offset += dynamic_obj_size;
                        prm_counter++;
                    }
                }
            }
        }
      }
    }
    auto cd = stat + dyna;
    log_printf("Encoded:\n%s\n", cd.c_str());
    calldata.stat = stat;
    calldata.dynamic = dyna;
    return calldata;
};

std::string doCall (std::string& method) {
  std::string data = "";
  std::string fhash = funcHash(method);
  if (fhash != "") {
    CallData cd = doCallData(method);
    data = fhash + cd.stat + cd.dynamic;
    log_printf("Calldata: \n%s\nCalldata len: %lu\n", data.c_str(), data.size());
  }
  return data;
}

}; // class 

// guard
#endif
