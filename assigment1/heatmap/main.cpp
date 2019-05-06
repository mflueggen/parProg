#include <pthread.h>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "utils.hpp"

uint32_t _width;
uint32_t _height;
uint32_t _rounds;
std::vector<double> _heatmaps[2];
std::vector<hotspot> _hotspots;
pthread_barrier_t _barrier;

void *heatmap_worker_thread(void *args) {
  auto *field_range = static_cast<range *>(args);

  std::vector<hotspot> hotspots;

  // identify and activate all hotspots
  for (const auto& h : _hotspots) {
    if (field_range->in(h.x, h.y, _width)) {
      hotspots.push_back(h);
    }
  }

  for (auto round = 1u; round <= _rounds; ++round) {
    const auto current_heatmap_index = round % 2;
    const auto old_heatmap_index = (current_heatmap_index + 1) % 2;

    for (auto i = field_range->from; i < field_range->to; ++i) {
      const auto coord = coordinate::index_to_coord(i, _width);
      double sum = 0;
      for (int64_t y = static_cast<int64_t>(coord.y) - 1; y <= static_cast<int64_t>(coord.y) + 1; ++y) {
        for (int64_t x = static_cast<int64_t>(coord.x) - 1; x <= static_cast<int64_t>(coord.x) + 1; ++x) {
          const auto index = coordinate::coord_to_index({static_cast<uint32_t>(x), static_cast<uint32_t>(y)}, _width,
                                                        _height);
          if (index < std::numeric_limits<uint32_t>::max()) {
            sum += _heatmaps[old_heatmap_index][index];
          }
        }
      }
      _heatmaps[current_heatmap_index][i] = sum / 9.0;
    }

    // set active hotspots back to one
    for (const auto& h : hotspots) {
      if (round >= h.start_round &&  round < h.end_round)
        _heatmaps[current_heatmap_index][coordinate::coord_to_index({h.x, h.y}, _width, _height)] = 1.0;
    }
    pthread_barrier_wait(&_barrier);
  }

  return nullptr;
}


int main(int argc, char *argv[]) {
  if (argc > 6) throw std::runtime_error("Wrong number of arguments");
  if (argc < 5) throw std::runtime_error("Wrong number of arguments");

  // Initialize values
  _width = std::stoi(argv[1]);
  _height = std::stoi(argv[2]);
  _rounds = std::stoi(argv[3]);

  _hotspots = load_hotspots(argv[4]);
  std::vector<coordinate> coords;

  if (argc == 6)
    coords = load_coords(argv[5]);

  // Initialize _heatmaps
  _heatmaps[0].resize(_width * _height);
  _heatmaps[1].resize(_width * _height);

  for (const auto& h : _hotspots) {
    if (h.start_round == 0)
      // activate hotspot on "old" heatmap
      _heatmaps[0][coordinate::coord_to_index({h.x, h.y}, _width, _height)] = 1.0;
  }

  const auto worker_thread_count = std::max(std::thread::hardware_concurrency(), 5u);
  pthread_barrier_init(&_barrier, nullptr, worker_thread_count);
  std::vector<pthread_t> threads(worker_thread_count);
  std::vector<range> ranges(worker_thread_count);
  const auto range_size = (_width * _height) / worker_thread_count;


  for (int i = 0; i < worker_thread_count; ++i) {
    ranges[i].from = i * range_size;
    ranges[i].to = std::min(ranges[i].from + range_size, _width * _height);
    pthread_create(&threads[i], nullptr, heatmap_worker_thread, &ranges[i]);
  }

  for (int i = 0; i < worker_thread_count; ++i) {
    pthread_join(threads[i], nullptr);
  }

  pthread_barrier_destroy(&_barrier);

  std::ofstream output_file ("output.txt");
  output_file << std::setprecision(std::numeric_limits<double>::max_digits10);

  if (coords.empty()) {
    // print result
    auto index = 0u;
    for (uint32_t y = 0; y < _height; ++y) {
      for (uint32_t x = 0; x < _width; ++x) {
        const auto value = _heatmaps[_rounds % 2][index];
        if (value > 0.9) {
          output_file << 'X';
        } else {
          output_file << static_cast<uint32_t>((value + 0.09) * 10.0) % 10;
        }
        ++index;
      }
      output_file << '\n';
    }
  }
  else {
    for (const auto& coord : coords) {
      output_file << _heatmaps[_rounds % 2][coordinate::coord_to_index(coord, _width, _height)] << '\n';
    }
  }

  output_file.close();

  return 0;
}