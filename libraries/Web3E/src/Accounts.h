#ifndef ACCOUNTS_H
#define ACCOUNTS_H

// ethPubKey() and ethAddr
#include <ethsign.h>

#include <string>
#include <vector>
#include <map>

class Account {
public:
    Account () {};

    std::string address_ = "";
    std::string privateKey = "";
    // std::string publicKey = "";
    // keys as bytes arrays
    uint8_t priv_key_[KEYS_SIZE] = {0};
    uint8_t pub_key_[PUB_KEY_SIZE] = {0};
};

class Accounts {
public:
    Accounts() {};
    Account getAccount(std::string address)
    {
        Account account;
        for (auto const& x : storage_)
        {
            if ( x.first == address )
            {
                account = x.second;
                //log_printf("account: %s\nsecret: %s\n", account.address_.c_str(), account.privateKey.c_str());
                return account;
            }
        }
        return account;
    };

    // create Account and save into storage
    Account privateKeyToAccount (std::string privKey) {
        Account account;
        account.privateKey = privKey;
        std::vector<uint8_t> pkb = hex2bytes(privKey.c_str());
        std::copy(pkb.begin(), pkb.end(), account.priv_key_);
        std::string account_address = ethAddr(account.priv_key_);
        account.address_ = account_address;
        ethPubKey(account.pub_key_, account.priv_key_);
        storage_.insert({account_address, account});
        return account;
    };

    std::vector <std::string> accounts;
    std::map <std::string, Account> storage_ {};
};
#endif

