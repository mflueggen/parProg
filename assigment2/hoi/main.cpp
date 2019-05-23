#include <endian.h>

#include "openssl/md5.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <vector>


//#define MY_LITTLE_ENDIAN_SYSTEM

template <typename T>
struct uint512 {
  static const int16_t BLOCKS = 64 / sizeof(T);

  T data[BLOCKS];
  T last_value = 0;


  uint512 add(T value) const {
    uint512 result = *this;
    result.inc(value);
    return result;
  }

  void inc(T value) {

    if (data[BLOCKS - 1] <= std::numeric_limits<T>::max() - value) {
      data[BLOCKS - 1] = this->data[BLOCKS - 1] + value;
      return;
    }

    data[BLOCKS - 1] = this->data[BLOCKS - 1] + value;

    for (int16_t b = BLOCKS - 2; b >= 0; --b) {
      ++data[b];
      if (data[b] != 0) break;
    }
  }

  void incremental_inc(T value) {
    inc(value - last_value);
    last_value = value;
  }


  void inc() {
    for (auto b = BLOCKS - 1; b >=0; --b) {
      ++data[b];
      if(data[b] != 0) break;
    }
  }

  unsigned char* c_data() const {
    return (unsigned char*)(data);
  }

  void reverse_byte_order(unsigned char* dest) {
    for (uint16_t b = 0; b < BLOCKS; ++b){
      for (auto p = 0; p < sizeof(T); ++p) {
        dest[b * BLOCKS + p] = ((unsigned char*)(&data[b]))[sizeof(T) - p - 1];
      }
    }
  }

  void set_byte(uint16_t position, unsigned char byte) {
    const auto block = BLOCKS - position / sizeof(T) - 1;
#ifdef MY_LITTLE_ENDIAN_SYSTEM
    const auto position_in_block = position % sizeof(T);
#else
    const auto position_in_block = sizeof(T) - position % sizeof(T) - 1;
#endif
    *((unsigned char*)(&data[block]) + position_in_block) = byte;
  }
};


int main(int count, char *args[]) {

#ifdef MY_LITTLE_ENDIAN_SYSTEM
  std::cout << "using little endian mode." << std::endl;
#endif


  using T = uint32_t;


  if (count <= 3)
    throw std::runtime_error("wrong number of arguments");


  uint512<T> initial_data;
  std::fill(initial_data.data, initial_data.data + uint512<T>::BLOCKS, 0);

  uint8_t pos = 63;
  unsigned char c;
  for (auto i = 0; args[1][i] != '\0'; i+=2) {
    std::sscanf(&args[1][i], "%2hhx", &c);
    initial_data.set_byte(pos, c);
    pos--;
  }

  const uint64_t N = std::atol(args[2]);
  auto start = std::chrono::high_resolution_clock::now();
  // idee berechne alle blöcke beforehand
//  std::vector<uint512> precomputed_data;
//  precomputed_data.resize(N);
//
//  for (auto i = 0ul; i < N; ++i) {
//    std::memcpy(precomputed_data[i].initial_data, initial_data.initial_data, uint512::BLOCKS);
//    initial_data.inc();
//  }
//
  auto end = std::chrono::high_resolution_clock::now();
//  std::cout << "Precompute initial_data: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()  << " ms" << std::endl;



  start = std::chrono::high_resolution_clock::now();
  std::vector<std::array<unsigned char, 16>> md5s;
  md5s.resize(N);
#ifdef MY_LITTLE_ENDIAN_SYSTEM
  unsigned char buffer[64];
#pragma omp parallel for default(none) shared(md5s) private(initial_data, buffer)
  for (auto i = 0ul; i < N; ++i) {
    initial_data.incremental_inc(i);
    initial_data.reverse_byte_order(buffer);
    MD5(buffer, 64, md5s[i].data());
  }
#else
#pragma omp parallel for default(none) shared(md5s) private(initial_data)
  for (auto i = 0ul; i < N; ++i) {
    initial_data.incremental_inc(i);
    MD5(initial_data.c_data(), 64, md5s[i].data());
  }
#endif

  end = std::chrono::high_resolution_clock::now();
  std::cout << "Compute md5s: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()  << " ms" << std::endl;

  for (auto j = 0; j < N; ++j) {
    for (auto i = 0; i < 16; ++i) {
      std::cout << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(md5s[j][i]);
    }
    std::cout << std::endl;
  }




  return 0;
}