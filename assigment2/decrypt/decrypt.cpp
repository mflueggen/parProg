//#define TIMER
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
    std::unordered_map<std::string, std::vector<std::string>> user_map; // password_hash -> [username]
    std::string line;
    while(std::getline(pwFile, line)) {
        std::string user = line.substr(0, line.find(':'));
        std::string password = line.substr(line.find(':') + 1);
        user_map[password].emplace_back(user);
    }

    // aggregate all users with the same salt. Do this separately to improve memory layout for caching (not too sure if this works)
    std::unordered_map<std::string, std::set<std::string>> salt_map; // salt -> [password_hash]
    for(const auto& password: user_map) {  // Some users probably used the same salt. This means we only need to calculate the password once. This overhead will decrease performance if all/majority of users use a unique salt.
        salt_map[password.first.substr(0,2)].insert(password.first);
    }

    // load dict
    // load passwords
    std::vector<std::string> words;
    std::string word;
    std::string previousWord;
    while(dictFile >> word) {
        if (common_8_prefix(word, previousWord)) {
            continue;  // skip word with equal first 8 chars. DES only reads first 8 chars of word. Assumption: dict is sorted
        }
        previousWord = word;
        words.emplace_back(word);
    }

    //TODO Test performance with parallel for (better work distirbution/less overhead?)
    // Include tests for: outer loop only, nested loop, sub task generation

    // We are using a task based approach since it allows openmp to start the processing while still reading the input dictionary.
    // Furthermore it prevents a huge block of words in the memory (like a array) which propably improves caching.
    std::unordered_map<std::string, std::string> decrypt_map; // username -> password
    std::set<std::string> solved_salts;
    std::unordered_map<std::string, uint8_t> solutions_per_salt;
#pragma omp parallel for default(none) shared(user_map, decrypt_map, words, salt_map, solved_salts, solutions_per_salt)
        for (int i=0; i < words.size(); ++i)
        {
            std::string word = words[i];
            struct crypt_data cryptData;
            cryptData.initialized = 0;
            for (auto & password : salt_map) {
                #pragma omp flush  // get the latest values to minimize calculations. Slight performance increase according to tests.
                if(solved_salts.find(password.first) != solved_salts.end())
                    continue;
                char *pw = crypt_r(word.c_str(), password.first.c_str(), &cryptData);
                if (password.second.find(pw) != password.second.end()) {
                    for(const auto& user: user_map[pw]) {
                        decrypt_map[user] = word;
                    }
                    solutions_per_salt[password.first]++;
                    if (solutions_per_salt[password.first] >= password.second.size())
                        solved_salts.insert(password.first);
                    continue;
                }
                pw = crypt_r((word + '0').c_str(), password.first.c_str(), &cryptData);
                if (password.second.find(pw) != password.second.end()) {
                    for(const auto& user: user_map[pw]) {
                        decrypt_map[user] = word + '0';
                    }
                    solutions_per_salt[password.first]++;
                    if (solutions_per_salt[password.first] >= password.second.size())
                        solved_salts.insert(password.first);
                    continue;
                }
                pw = crypt_r((word + '1').c_str(), password.first.c_str(), &cryptData);
                if (password.second.find(pw) != password.second.end()) {
                    for(const auto& user: user_map[pw]) {
                        decrypt_map[user] = word + '1';
                    }
                    solutions_per_salt[password.first]++;
                    if (solutions_per_salt[password.first] >= password.second.size())
                        solved_salts.insert(password.first);
                    continue;
                }
                pw = crypt_r((word + '2').c_str(), password.first.c_str(), &cryptData);
                if (password.second.find(pw) != password.second.end()) {
                    for(const auto& user: user_map[pw]) {
                        decrypt_map[user] = word + '2';
                    }
                    solutions_per_salt[password.first]++;
                    if (solutions_per_salt[password.first] >= password.second.size())
                        solved_salts.insert(password.first);
                    continue;
                }
                pw = crypt_r((word + '3').c_str(), password.first.c_str(), &cryptData);
                if (password.second.find(pw) != password.second.end()) {
                    for(const auto& user: user_map[pw]) {
                        decrypt_map[user] = word + '3';
                    }
                    solutions_per_salt[password.first]++;
                    if (solutions_per_salt[password.first] >= password.second.size())
                        solved_salts.insert(password.first);
                    continue;
                }
                pw = crypt_r((word + '4').c_str(), password.first.c_str(), &cryptData);
                if (password.second.find(pw) != password.second.end()) {
                    for(const auto& user: user_map[pw]) {
                        decrypt_map[user] = word + '4';
                    }
                    solutions_per_salt[password.first]++;
                    if (solutions_per_salt[password.first] >= password.second.size())
                        solved_salts.insert(password.first);
                    continue;
                }
                pw = crypt_r((word + '5').c_str(), password.first.c_str(), &cryptData);
                if (password.second.find(pw) != password.second.end()) {
                    for(const auto& user: user_map[pw]) {
                        decrypt_map[user] = word + '5';
                    }
                    solutions_per_salt[password.first]++;
                    if (solutions_per_salt[password.first] >= password.second.size())
                        solved_salts.insert(password.first);
                    continue;
                }
                pw = crypt_r((word + '6').c_str(), password.first.c_str(), &cryptData);
                if (password.second.find(pw) != password.second.end()) {
                    for(const auto& user: user_map[pw]) {
                        decrypt_map[user] = word + '6';
                    }
                    solutions_per_salt[password.first]++;
                    if (solutions_per_salt[password.first] >= password.second.size())
                        solved_salts.insert(password.first);
                    continue;
                }
                pw = crypt_r((word + '7').c_str(), password.first.c_str(), &cryptData);
                if (password.second.find(pw) != password.second.end()) {
                    for(const auto& user: user_map[pw]) {
                        decrypt_map[user] = word + '7';
                    }
                    solutions_per_salt[password.first]++;
                    if (solutions_per_salt[password.first] >= password.second.size())
                        solved_salts.insert(password.first);
                    continue;
                }
                pw = crypt_r((word + '8').c_str(), password.first.c_str(), &cryptData);
                if (password.second.find(pw) != password.second.end()) {
                    for(const auto& user: user_map[pw]) {
                        decrypt_map[user] = word + '8';
                    }
                    solutions_per_salt[password.first]++;
                    if (solutions_per_salt[password.first] >= password.second.size())
                        solved_salts.insert(password.first);
                    continue;
                }
                pw = crypt_r((word + '9').c_str(), password.first.c_str(), &cryptData);
                if (password.second.find(pw) != password.second.end()) {
                    for(const auto& user: user_map[pw]) {
                        decrypt_map[user] = word + '9';
                    }
                    solutions_per_salt[password.first]++;
                    if (solutions_per_salt[password.first] >= password.second.size())
                        solved_salts.insert(password.first);
                    continue;
                }
            }
        }

    for (const auto& user: decrypt_map) {
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
