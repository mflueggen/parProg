#include <pthread.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "utils.hpp"

int main(int argc, char *argv[]) {
  if (argc > 6) throw std::runtime_error("Wrong number of arguments");
  if (argc < 5) throw std::runtime_error("Wrong number of arguments");

  // Initialize values
  const auto width = std::stoi(argv[1]) + 2;
  const auto height = std::stoi(argv[2]) + 2;
  const auto rounds = std::stoi(argv[3]);

  using T = float;

  // we manage two heatmaps. Each round the "active" heatmap is updated based on the values of the other "old" heatmap
  // that way we don't need to synchronize data bounderies between threads.
  // a heatmap is modeled as an one dimensional array. Using _width and _height we can compute either the two
  // dimensional coordinate from the given index or compute the index given the coordinate.
  std::vector<std::vector<T>> heatmaps[2];

  // vector of all hotspots
  std::vector<hotspot> hotspots = load_hotspots(argv[4]);
  std::vector<coordinate> coords;

  // load coords file if given
  if (argc == 6)
    coords = load_coords(argv[5]);

  // Initialize _heatmaps
  heatmaps[0].resize(height);
  for(auto& v : heatmaps[0])
    v.resize(width);

  heatmaps[1].resize(height);
  for(auto& v : heatmaps[1])
    v.resize(width);


//  const auto start = std::chrono::high_resolution_clock::now();

  for (const auto& h : hotspots) {
    if (h.start_round == 0)
      // activate hotspot on "old" heatmap
      heatmaps[0][h.y + 1][h.x + 1] = 1.0;
  }



  for (auto round = 1u; round <= rounds; ++round) {
    const auto current_heatmap_index = round % 2;
    const auto old_heatmap_index = (current_heatmap_index + 1) % 2;

#ifdef WITH_OMP
#pragma omp parallel for default(none) shared(heatmaps)
#endif
    for (auto row = 1; row < height - 1; ++row) {
      for (auto col = 1; col < width - 1; ++col) {
        double sum = 0;
        // for the 3x3 matrix around coord
        for (auto y = row -1; y <= row + 1; ++y) {
          for (auto x = col - 1; x <= col + 1; ++x) {
            sum += heatmaps[old_heatmap_index][y][x];
          }
        }
        heatmaps[current_heatmap_index][row][col] = sum / 9.0;
      }
    }

    // set active hotspots back to one
    for (auto h = 0; h < hotspots.size(); ++h) {
      const auto& hotspot = hotspots[h];
      if (round >= hotspot.start_round && round < hotspot.end_round)
        heatmaps[current_heatmap_index][hotspot.y + 1][hotspot.x + 1] = 1.0;
    }

    // wait until all threads are done working so we can safely switch the active and old heatmap.
    // ---> barrier here
  }

//  const auto end = std::chrono::high_resolution_clock::now();
//
//  std::cout << "Runtime: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms"
//            << std::endl;

  std::ofstream output_file("output.txt");
  output_file << std::setprecision(4) << std::fixed;

  // output heatmap
  if (coords.empty()) {
    // print result
    for (uint32_t y = 1; y < height - 1; ++y) {
      for (uint32_t x = 1; x < width - 1; ++x) {
        const auto value = heatmaps[rounds % 2][y][x];
        if (value > 0.9) {
          output_file << 'X';
        } else {
          output_file << static_cast<uint32_t>((value + 0.09) * 10.0) % 10;
        }
      }
      output_file << '\n';
    }
  } else {
    for (const auto& coord : coords) {
      output_file << heatmaps[rounds % 2][coord.y + 1][coord.x + 1] << '\n';
    }
  }

  output_file.close();

  return 0;
}
