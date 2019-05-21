#include "openssl/md5.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <vector>


struct uint512 {
  static const size_t SIZE = 512 / 8;

  unsigned char data[SIZE];

  inline void inc() {
    for (auto b = SIZE - 1;; --b) {
      ++data[b];
      if(data[b] != 0) break;
      if (b == 0) break;
    }
  }
};


int main(int count, char *args[]) {


  if (count <= 3)
    throw std::runtime_error("wrong number of arguments");


  uint512 data;
  std::fill(data.data, data.data + uint512::SIZE, 0);

  for (auto i = 0; args[1][i] != '\0'; i+=2) {
    std::sscanf(&args[1][i], "%2hhx", &data.data[i/2]);
  }

  const uint64_t N = std::atol(args[2]);
  auto start = std::chrono::high_resolution_clock::now();
  // idee berechne alle blöcke beforehand
  std::vector<uint512> precomputed_data;
  precomputed_data.resize(N);

  for (auto i = 0ul; i < N; ++i) {
    std::memcpy(precomputed_data[i].data, data.data, uint512::SIZE);
    data.inc();
  }

  auto end = std::chrono::high_resolution_clock::now();
  std::cout << "Precompute data: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()  << " ms" << std::endl;



  start = std::chrono::high_resolution_clock::now();
  std::vector<std::array<unsigned char, 16>> md5s;
  md5s.resize(N);
#pragma omp parallel for default(none) shared(precomputed_data, md5s)
  for (auto i = 0ul; i < N; ++i) {
    MD5((unsigned char*)(precomputed_data[i].data), uint512::SIZE, md5s[i].data());
  }
  end = std::chrono::high_resolution_clock::now();
  std::cout << "Compute md5s: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()  << " ms" << std::endl;

//  for (auto j = 0; j < N; ++j) {
//    for (auto i = 0; i < 16; ++i) {
//      std::cout << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(md5s[j][i]);
//    }
//    std::cout << std::endl;
//  }




  // Hash einlesen
  // in 512 bit block packen

  // für alle n den md5 bilden
  // reducen

  // die kleinesten EInträge zurückgeben




  return 0;
}