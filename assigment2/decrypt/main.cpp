#include <unistd.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>

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

int main(int argc, char *argv[]) {
#ifndef GEN_TEST
    std::ifstream pwFile(argv[1]);
    std::ifstream dictFile(argv[2]);

    // load dictionary
    std::vector<std::string> dict;
    std::string word;
    //TODO filter all password longer than 8 chars and only use the common prefix. DES implementation of crypt only considers first 8 byte.
    while (dictFile >> word) {
        dict.emplace_back(word);
    }

    // load passwords
    std::vector<PW> cryptPw;
    std::string line;
    while(std::getline(pwFile, line)) {
        std::string user = line.substr(0, line.find(':'));
        std::string password = line.substr(line.find(':') + 1);
        std::string salt = password.substr(0,2);
        cryptPw.emplace_back(user, salt, password);
    }

    for (auto password = cryptPw.begin(); password != cryptPw.end(); ++password) {
        for (auto word = dict.begin(); word != dict.end(); ++word) {

            //TODO schleifen anders durchgehen?? (nicht einfach änderbar!)

            //TODO wertvergleich so bauen, dass die branch prediction meistens richtig liegt

            //TODO optional nach erstem Treffer abbrechen
            char *pw = crypt((*word).c_str(), (*password).salt.c_str());
            char *pw0 = crypt(((*word) + '0').c_str(), (*password).salt.c_str());
            char *pw1 = crypt(((*word) + '1').c_str(), (*password).salt.c_str());
            char *pw2 = crypt(((*word) + '2').c_str(), (*password).salt.c_str());
            char *pw3 = crypt(((*word) + '3').c_str(), (*password).salt.c_str());
            char *pw4 = crypt(((*word) + '4').c_str(), (*password).salt.c_str());
            char *pw5 = crypt(((*word) + '5').c_str(), (*password).salt.c_str());
            char *pw6 = crypt(((*word) + '6').c_str(), (*password).salt.c_str());
            char *pw7 = crypt(((*word) + '7').c_str(), (*password).salt.c_str());
            char *pw8 = crypt(((*word) + '8').c_str(), (*password).salt.c_str());
            char *pw9 = crypt(((*word) + '9').c_str(), (*password).salt.c_str());
            //            std::cout << (*word) + '0' << std::endl;
            if (    (*password).cryptPw != pw &&
                    (*password).cryptPw != pw1 &&
                    (*password).cryptPw != pw2 &&
                    (*password).cryptPw != pw3 &&
                    (*password).cryptPw != pw4 &&
                    (*password).cryptPw != pw5 &&
                    (*password).cryptPw != pw6 &&
                    (*password).cryptPw != pw7 &&
                    (*password).cryptPw != pw8 &&
                    (*password).cryptPw != pw9 &&
                    (*password).cryptPw != pw0     ) {
                //No password matched -> continue
                // We assume a "jump never" branch prediction
                continue;
            }
            if ((*password).cryptPw == pw) {
                std::cout << (*password).user << ";" << *word << std::endl;
                break;
            }
            if ((*password).cryptPw == pw0) {
                std::cout << (*password).user << ";" << (*word) + '0' << std::endl;
                break;
            }
            if ((*password).cryptPw == pw1) {
                std::cout << (*password).user << ";" << (*word) + '1' << std::endl;
                break;
            }
            if ((*password).cryptPw == pw2) {
                std::cout << (*password).user << ";" << (*word) + '2' << std::endl;
                break;
            }
            if ((*password).cryptPw == pw3) {
                std::cout << (*password).user << ";" << (*word) + '3' << std::endl;
                break;
            }
            if ((*password).cryptPw == pw4) {
                std::cout << (*password).user << ";" << (*word) + '4' << std::endl;
                break;
            }
            if ((*password).cryptPw == pw5) {
                std::cout << (*password).user << ";" << (*word) + '5' << std::endl;
                break;
            }
            if ((*password).cryptPw == pw6) {
                std::cout << (*password).user << ";" << (*word) + '6' << std::endl;
                break;
            }
            if ((*password).cryptPw == pw7) {
                std::cout << (*password).user << ";" << (*word) + '7' << std::endl;
                break;
            }
            if ((*password).cryptPw == pw8) {
                std::cout << (*password).user << ";" << (*word) + '8' << std::endl;
                break;
            }
            if ((*password).cryptPw == pw9) {
                std::cout << (*password).user << ";" << (*word) + '9' << std::endl;
                break;
            }
        }
    }

//    // Print passwords
//    for (auto i = cryptPw.begin();  i != cryptPw.end() ; ++i) {
//        std::cout << (*i).user << " has the password " << (*i).cryptPw << " with salt " << (*i).salt << std::endl;
//    }

//    // Print dictionary
//    for(auto i = dict.begin(); i != dict.end(); ++i)
//    {
//        std::cout << *i << std::endl;
//    }

//    cryptPw.emplace_back("sw", "swhdjrtjktzf");
//    std::cout << crypt("Ankündigen","Ah") << std::endl;
//    std::cout << cryptPw[0].salt << std::endl;
#else
    std::cout << crypt("Halloo","/B") << std::endl;
#endif
    return 0;
}