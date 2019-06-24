#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <cstring>

int main() {
  std::string line;
  while(std::getline(std::cin, line)) {
    unsigned char buffer[64] = {0};
    if(line.length() <= 55) {
        std::copy(line.begin(), line.end(), buffer);
    } else {
      std::cerr << "The input dictionary file contains password which are longer than 55 characters" << std::endl;
    }
    // add 1 bit and pad the rest with 0s
    buffer[line.length()] = 0x80;
    // store as 64 bit little endian integer the lenght of the original message in bits
    uint64_t line_len;
    line_len = line.length() * 8;
    memcpy(&buffer[56], &line_len, 8);
    fwrite(buffer, 1, 64, stdout);
  }
}