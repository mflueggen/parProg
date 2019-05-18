#include <unistd.h>
#include <iostream>
#include <fstream>
#include <vector>

struct PW {
    std::string user;
    std::string salt;
    std::string cryptPw;

    PW(const std::string &_user, const std::string &_salt, const std::string &_cryptPw) {
        user = _user;
        salt = _salt;
        cryptPw = _cryptPw;
    }
};

int main(int argc, char *argv[]) {
    std::ifstream pwFile(argv[1]);
    std::ifstream dictFile(argv[2]);

    std::vector<std::string> dict;
    std::string word;
    while (dictFile >> word) {
        dict.emplace_back(word);
    }

    std::vector<PW> cryptPw;
    std::string line;
    while(std::getline(pwFile, line)) {
        std::string user = line.substr(0, line.find(':'));
        std::string password = line.substr(line.find(':') + 1);
        std::string salt = password.substr(0,2);
        cryptPw.emplace_back(user, salt, password);
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
////    std::cout << crypt("Ankündigen","Ah") << std::endl;
//    std::cout << cryptPw[0].salt << std::endl;
    return 0;
}