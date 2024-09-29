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

  std::vector<std::string> split (const std::string &s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss (s);
    std::string item;

    while (getline (ss, item, delim)) {
      result.push_back (item);
    }

    return result;
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
   
std::set<std::string> static_types = {"int", "int256", "uint256"};

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

CallData doCall (std::string& signature) {
    CallData calldata;
    std::string stat = "";
    std::string dyna = "";

    int args = -1;
    auto fname = split(signature, '(');
    
    // get count of arguments
    if (signature.find("()") != std::string::npos){
      args = 0;
    } else 
      args = std::count(signature.begin(),signature.end(),',') + 1;

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
                    if ( static_types.count(it.second)) {
                        std::string val = "0";
                        if (it.second.find("int") != std::string::npos) { // TODO for any int,uint float etc.
                          auto number = std::stoll(params[prm_counter], nullptr, 10);  // get real parameter
                          val = number_to_hex_str32(number);
                        }
                        
                        // max. 1234567890123444556 -> 000000000000000000000000000000000000000000000000112210F47DE9514C
                        log_printf("Parameter '%s' has static type '%s' with value %s\n", it.first.c_str(), it.second.c_str(), val.c_str());                        
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

}; // class 

// guard
#endif
