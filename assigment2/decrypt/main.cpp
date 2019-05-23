//#define TIMER
//#include <omp.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
#include <crypt.h>
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
    std::vector<PW> cryptPw;
    std::string line;
    while(std::getline(pwFile, line)) {
        std::string user = line.substr(0, line.find(':'));
        std::string password = line.substr(line.find(':') + 1);
        std::string salt = password.substr(0,2);
        cryptPw.emplace_back(user, salt, password);
    }

    //TODO Test performance with parallel for (better work distirbution/less overhead?)

    std::string word;
    std::string previousWord;
#pragma omp parallel default(none) shared(dictFile, cryptPw, word, previousWord)
    {
        #pragma omp single
        while (dictFile >> word) {
            if (common_8_prefix(word, previousWord)) {
                continue;  // skip word with equal first 8 chars. DES only reads first 8 chars of word
            }
            previousWord = word;
            #pragma omp task default(none) shared(cryptPw) firstprivate(word)
            {
                //TODO optimize for branch predictor -> <1% branch misses according to perf
                struct crypt_data cryptData;
                cryptData.initialized = 0;
                for (auto password = cryptPw.begin(); password < cryptPw.end(); ++password) {
                    #pragma omp flush //TODO Test if this improves speed on bigger machine as well
                    if ((*password).cracked) {
                        continue;
                    }
                    char *pw = crypt_r(word.c_str(), (*password).salt.c_str(), &cryptData);
                    if ((*password).cryptPw == pw) {
                        (*password).cracked = true;
                        (*password).clearPw = word;
                        continue;
                    }
                    pw = crypt_r((word + '0').c_str(), (*password).salt.c_str(), &cryptData);
                    if ((*password).cryptPw == pw) {
                        (*password).cracked = true;
                        (*password).clearPw = word + '0';
                        continue;
                    }
                    pw = crypt_r((word + '1').c_str(), (*password).salt.c_str(), &cryptData);
                    if ((*password).cryptPw == pw) {
                        (*password).cracked = true;
                        (*password).clearPw = word + '1';
                        continue;
                    }
                    pw = crypt_r((word + '2').c_str(), (*password).salt.c_str(), &cryptData);
                    if ((*password).cryptPw == pw) {
                        (*password).cracked = true;
                        (*password).clearPw = word + '2';
                        continue;
                    }
                    pw = crypt_r((word + '3').c_str(), (*password).salt.c_str(), &cryptData);
                    if ((*password).cryptPw == pw) {
                        (*password).cracked = true;
                        (*password).clearPw = word + '3';
                        continue;
                    }
                    pw = crypt_r((word + '4').c_str(), (*password).salt.c_str(), &cryptData);
                    if ((*password).cryptPw == pw) {
                        (*password).cracked = true;
                        (*password).clearPw = word + '4';
                        continue;
                    }
                    pw = crypt_r((word + '5').c_str(), (*password).salt.c_str(), &cryptData);
                    if ((*password).cryptPw == pw) {
                        (*password).cracked = true;
                        (*password).clearPw = word + '5';
                        continue;
                    }
                    pw = crypt_r((word + '6').c_str(), (*password).salt.c_str(), &cryptData);
                    if ((*password).cryptPw == pw) {
                        (*password).cracked = true;
                        (*password).clearPw = word + '6';
                        continue;
                    }
                    pw = crypt_r((word + '7').c_str(), (*password).salt.c_str(), &cryptData);
                    if ((*password).cryptPw == pw) {
                        (*password).cracked = true;
                        (*password).clearPw = word + '7';
                        continue;
                    }
                    pw = crypt_r((word + '8').c_str(), (*password).salt.c_str(), &cryptData);
                    if ((*password).cryptPw == pw) {
                        (*password).cracked = true;
                        (*password).clearPw = word + '8';
                        continue;
                    }
                    pw = crypt_r((word + '9').c_str(), (*password).salt.c_str(), &cryptData);
                    if ((*password).cryptPw == pw) {
                        (*password).cracked = true;
                        (*password).clearPw = word + '9';
                        continue;
                    }
                }
            }
        }
        #pragma omp taskwait
    }

    for (auto password = cryptPw.begin(); password < cryptPw.end(); ++password) {
        if ((*password).cracked)
            std::cout << (*password).user << ";" << (*password).clearPw << std::endl;
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