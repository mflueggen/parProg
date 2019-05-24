#define TIMER
//#include <omp.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
#include <crypt.h>
#include <unordered_map>
#include <set>

#ifdef TIMER
#include <ctime>
#endif

//#define GEN_TEST

struct PW {
    std::string user;
    std::string salt;
    std::string cryptPw;
    bool cracked;
    std::string clearPw;

    PW(const std::string &_user, const std::string &_salt, const std::string &_cryptPw) {
        user = _user;
        salt = _salt;
        cryptPw = _cryptPw;
        cracked = false;
        clearPw = "";
    }
};

bool common_8_prefix(std::string const& word, std::string const& previousWord) {
    return word.length() > 8 &&  // only check if the new word is longer than 8 chars
            previousWord.length() >= 8 &&  // previous word needs at least 8 byte
           std::equal(previousWord.begin(), previousWord.begin() + 8, word.begin());
}

int main(int argc, char *argv[]) {
#ifdef TIMER
    clock_t begin = clock();
#endif
#ifndef GEN_TEST
    std::ifstream pwFile(argv[1]);
    std::ifstream dictFile(argv[2]);

    // load passwords
    //TODO some users may have the same salt (test if the overhead is bigger)
    std::unordered_map<std::string, std::vector<std::string>> user_map; // password_hash -> [username]
    std::string line;
    std::unordered_map<std::string, std::set<std::string>> salt_map; // salt -> [password_hash]
    while(std::getline(pwFile, line)) {
        std::string user = line.substr(0, line.find(':'));
        std::string password = line.substr(line.find(':') + 1);
        std::string salt = password.substr(0,2);
        user_map[password].emplace_back(user);
        salt_map[salt].insert(password);
    }

    //TODO Test performance with parallel for (better work distirbution/less overhead?)
    // Include tests for: outer loop only, nested loop, sub task generation

    std::string word;
    std::string previousWord;
    std::unordered_map<std::string, std::string> decrypt_map; // username -> password
#pragma omp parallel default(none) shared(dictFile, user_map, decrypt_map, word, previousWord, salt_map)
    {
        #pragma omp single
        while (dictFile >> word) {
            if (common_8_prefix(word, previousWord)) {
                continue;  // skip word with equal first 8 chars. DES only reads first 8 chars of word. Assumption: dict is sorted
            }
            previousWord = word;
            #pragma omp task default(none) shared(user_map, decrypt_map, salt_map) firstprivate(word)
            {
                //TODO optimize for branch predictor -> <1% branch misses according to perf
                struct crypt_data cryptData;
                cryptData.initialized = 0;
                for (auto password = salt_map.begin(); password != salt_map.end(); ++password) {
                    char *pw = crypt_r(word.c_str(), password->first.c_str(), &cryptData);
                    if (password->second.find(pw) != password->second.end()) {
                        for(auto user: user_map[pw]) {
                            decrypt_map[user] = word;
                        }
                        continue;
                    }
                    pw = crypt_r((word + '0').c_str(), password->first.c_str(), &cryptData);
                    if (password->second.find(pw) != password->second.end()) {
                        for(auto user: user_map[pw]) {
                            decrypt_map[user] = word + '0';
                        }
                        continue;
                    }
                    pw = crypt_r((word + '1').c_str(), password->first.c_str(), &cryptData);
                    if (password->second.find(pw) != password->second.end()) {
                        for(auto user: user_map[pw]) {
                            decrypt_map[user] = word + '1';
                        }
                        continue;
                    }
                    pw = crypt_r((word + '2').c_str(), password->first.c_str(), &cryptData);
                    if (password->second.find(pw) != password->second.end()) {
                        for(auto user: user_map[pw]) {
                            decrypt_map[user] = word + '2';
                        }
                        continue;
                    }
                    pw = crypt_r((word + '3').c_str(), password->first.c_str(), &cryptData);
                    if (password->second.find(pw) != password->second.end()) {
                        for(auto user: user_map[pw]) {
                            decrypt_map[user] = word + '3';
                        }
                        continue;
                    }
                    pw = crypt_r((word + '4').c_str(), password->first.c_str(), &cryptData);
                    if (password->second.find(pw) != password->second.end()) {
                        for(auto user: user_map[pw]) {
                            decrypt_map[user] = word + '4';
                        }
                        continue;
                    }
                    pw = crypt_r((word + '5').c_str(), password->first.c_str(), &cryptData);
                    if (password->second.find(pw) != password->second.end()) {
                        for(auto user: user_map[pw]) {
                            decrypt_map[user] = word + '5';
                        }
                        continue;
                    }
                    pw = crypt_r((word + '6').c_str(), password->first.c_str(), &cryptData);
                    if (password->second.find(pw) != password->second.end()) {
                        for(auto user: user_map[pw]) {
                            decrypt_map[user] = word + '6';
                        }
                        continue;
                    }
                    pw = crypt_r((word + '7').c_str(), password->first.c_str(), &cryptData);
                    if (password->second.find(pw) != password->second.end()) {
                        for(auto user: user_map[pw]) {
                            decrypt_map[user] = word + '7';
                        }
                        continue;
                    }
                    pw = crypt_r((word + '8').c_str(), password->first.c_str(), &cryptData);
                    if (password->second.find(pw) != password->second.end()) {
                        for(auto user: user_map[pw]) {
                            decrypt_map[user] = word + '8';
                        }
                        continue;
                    }
                    pw = crypt_r((word + '9').c_str(), password->first.c_str(), &cryptData);
                    if (password->second.find(pw) != password->second.end()) {
                        for(auto user: user_map[pw]) {
                            decrypt_map[user] = word + '9';
                        }
                        continue;
                    }
                }
            }
        }
        #pragma omp taskwait
    }

    for (auto user: decrypt_map) {
        std::cout << user.first << ";" << user.second << std::endl;
    }

#else
    std::cout << crypt("Ansitz5","/B") << std::endl;
#endif
#ifdef TIMER
    clock_t finish = clock();
    double elapsed_secs = double(finish - begin) / CLOCKS_PER_SEC;
    std::cout << "Took " << elapsed_secs << " seconds" << std::endl;
#endif
    return 0;
}