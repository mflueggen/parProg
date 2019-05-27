#include <algorithm>
#include <parallel/algorithm>
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

#include "md5.hpp"

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

  void reverse_byte_order(unsigned char* dest) const {
    for (uint16_t b = 0; b < BLOCKS; ++b){
      for (auto p = 0; p < sizeof(T); ++p) {
        dest[b * sizeof(T) + p] = ((unsigned char*)(&data[b]))[sizeof(T) - p - 1];
      }
    }
  }

  void set_byte(uint16_t position, unsigned char byte) {
    const auto block = BLOCKS - position / sizeof(T) - 1;
#ifdef CONVERT_TO_BIG_ENDIAN
    const auto position_in_block = position % sizeof(T);
#else
    const auto position_in_block = sizeof(T) - position % sizeof(T) - 1;
#endif
    *((unsigned char*)(&data[block]) + position_in_block) = byte;
  }
};


int main(int count, char *args[]) {

  using T = uint64_t;

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


#ifdef BENCHMARK
  auto start = std::chrono::high_resolution_clock::now();
#endif
  std::vector<std::array<unsigned char, 16>> md5s;
  md5s.resize(N);
#ifdef CONVERT_TO_BIG_ENDIAN
  unsigned char buffer[64];
#pragma omp parallel for default(none) shared(md5s) firstprivate(initial_data) private(buffer)
  for (auto i = 0ul; i < N; ++i) {
    initial_data.incremental_inc(i);
    // this line does not seem to influence the execution time
    initial_data.reverse_byte_order(buffer);
    __md5_buffer((char*)buffer, 64, md5s[i].data());
  }
#else
#pragma omp parallel for default(none) shared(md5s) firstprivate(initial_data)
  for (auto i = 0ul; i < N; ++i) {
    initial_data.incremental_inc(i);
    __md5_buffer((char*)initial_data.c_data(), 64, md5s[i].data());
  }
#endif

#ifdef BENCHMARK
  auto end = std::chrono::high_resolution_clock::now();
  std::cout << "Compute md5s: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()  << " ms" << std::endl;
#endif

#ifdef BENCHMARK
  start = std::chrono::high_resolution_clock::now();
#endif


  // Sorting (let's see, if we actually need to sort the whole list)
  std::vector<uint64_t> queries;

  for (auto i = 3; i < count; ++i) {
    queries.push_back(atol(args[i]));
  }

  std::sort(queries.begin(), queries.end());

  std::nth_element(md5s.begin(), md5s.begin() + queries[queries.size() - 1], md5s.end(), [](const std::array<unsigned char, 16>& a, const std::array<unsigned char, 16>& b) {
    for (auto i = 0; i < 16; ++i) {
      if (a[i] < b[i]) return true;
      if (a[i] > b[i]) return false;
    }
    return false;
  });


  for (int i = queries.size() - 2; i >= 0; --i) {
    std::nth_element(md5s.begin(), md5s.begin() + queries[i], md5s.begin() + queries[i + 1], [](const std::array<unsigned char, 16>& a, const std::array<unsigned char, 16>& b) {
      for (auto i = 0; i < 16; ++i) {
        if (a[i] < b[i]) return true;
        if (a[i] > b[i]) return false;
      }
      return false;
    });
  }


#ifdef BENCHMARK
  end = std::chrono::high_resolution_clock::now();
  std::cout << "Sort took: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()  << " ms" << std::endl;
#endif

  // output
  for (auto j = 3; j < count; ++j) {
    const auto query = atol(args[j]);
    for (auto i = 0; i < 16; ++i) {
      std::cout << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(md5s[query][i]);
    }
    std::cout << '\n';
  }

  return 0;
}