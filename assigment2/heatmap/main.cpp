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
  const auto width = std::stoi(argv[1]);
  const auto height = std::stoi(argv[2]);
  const auto rounds = std::stoi(argv[3]);

  // we manage two heatmaps. Each round the "active" heatmap is updated based on the values of the other "old" heatmap
  // that way we don't need to synchronize data bounderies between threads.
  // a heatmap is modeled as an one dimensional array. Using _width and _height we can compute either the two
  // dimensional coordinate from the given index or compute the index given the coordinate.
  std::vector<double> heatmaps[2];

  // vector of all hotspots
  std::vector<hotspot> hotspots = load_hotspots(argv[4]);
  std::vector<coordinate> coords;

  // load coords file if given
  if (argc == 6)
    coords = load_coords(argv[5]);

  // Initialize _heatmaps
  heatmaps[0].resize(width * height);
  heatmaps[1].resize(width * height);

#ifdef TIMER
  const auto start = std::chrono::high_resolution_clock::now();
#endif

  for (const auto& h : hotspots) {
    if (h.start_round == 0)
      // activate hotspot on "old" heatmap
      heatmaps[0][coordinate::coord_to_index({h.x, h.y}, width, height)] = 1.0;
  }



  for (auto round = 1u; round <= rounds; ++round) {
    const auto current_heatmap_index = round % 2;
    const auto old_heatmap_index = (current_heatmap_index + 1) % 2;

#ifdef WITH_OMP
#pragma omp parallel for default(none) shared(heatmaps)
#endif
    for (auto i = 0ul; i < width * height; ++i) {
      const auto coord = coordinate::index_to_coord(i, width);
      double sum = 0;
      // for the 3x3 matrix around coord
      for (auto y = static_cast<int64_t>(coord.y) - 1; y <= static_cast<int64_t>(coord.y) + 1; ++y) {
        for (auto x = static_cast<int64_t>(coord.x) - 1; x <= static_cast<int64_t>(coord.x) + 1; ++x) {
          const auto index = coordinate::coord_to_index({static_cast<uint32_t>(x), static_cast<uint32_t>(y)}, width,
                                                        height);
          if (index < std::numeric_limits<uint32_t>::max()) {
            sum += heatmaps[old_heatmap_index][index];
          }
        }
      }
      heatmaps[current_heatmap_index][i] = sum / 9.0;
    }

    // set active hotspots back to one
    for (auto h = 0; h < hotspots.size(); ++h) {
      const auto& hotspot = hotspots[h];
      if (round >= hotspot.start_round && round < hotspot.end_round)
        heatmaps[current_heatmap_index][coordinate::coord_to_index({hotspot.x, hotspot.y}, width, height)] = 1.0;
    }

    // wait until all threads are done working so we can safely switch the active and old heatmap.
    // ---> barrier here
  }

#ifdef TIMER

  const auto end = std::chrono::high_resolution_clock::now();

  std::cout << "Runtime: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms"
            << std::endl;
#endif

  std::ofstream output_file("output.txt");
  output_file << std::setprecision(4) << std::fixed;

  // output heatmap
  if (coords.empty()) {
    // print result
    auto index = 0u;
    for (uint32_t y = 0; y < height; ++y) {
      for (uint32_t x = 0; x < width; ++x) {
        const auto value = heatmaps[rounds % 2][index];
        if (value > 0.9) {
          output_file << 'X';
        } else {
          output_file << static_cast<uint32_t>((value + 0.09) * 10.0) % 10;
        }
        ++index;
      }
      output_file << '\n';
    }
  } else {
    for (const auto& coord : coords) {
      output_file << heatmaps[rounds % 2][coordinate::coord_to_index(coord, width, height)] << '\n';
    }
  }

  output_file.close();

  return 0;
}
