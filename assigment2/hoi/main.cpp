#include "openssl/md5.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>

int main(int count, char *args[]) {

  const size_t BLOCK_SIZE = 512 / 8;

  if (count <= 3)
    throw std::runtime_error("wrong number of arguments");


  unsigned char initial_block[BLOCK_SIZE];
  std::fill(initial_block, initial_block + BLOCK_SIZE, 0);

  for (auto i = 0; args[1][i] != '\0'; i+=2) {
    std::sscanf(&args[1][i], "%2hhx", &initial_block[i/2]);
  }

  unsigned char md[16];

  MD5(initial_block, BLOCK_SIZE, md);


  for (auto i = 0; i < 16; ++i) {
    std::cout << std::hex << static_cast<int>(md[i]);
  }
  std::cout << std::endl;




  // Hash einlesen
  // in 512 bit block packen

  // für alle n den md5 bilden
  // reducen

  // die kleinesten EInträge zurückgeben




  return 0;
}