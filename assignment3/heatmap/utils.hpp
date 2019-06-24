#pragma once

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <CL/cl.hpp>

struct hotspot {
  cl_ushort x;
  cl_ushort y;
  cl_ushort start_round;
  cl_ushort end_round;
};

struct slim_hotspot {
  cl_ushort start_round;
  cl_ushort end_round;
};

struct coordinate {
  coordinate(uint32_t x, uint32_t y) : x{x}, y{y} {}
  uint32_t x;
  uint32_t y;

  static coordinate index_to_coord(uint32_t index, uint32_t width) {
    const auto y = index / width;
    return coordinate{index - width * y, y};
  }

  static uint32_t coord_to_index(const coordinate c, uint32_t width, uint32_t height) {
    if (c.x >= width)
      return std::numeric_limits<uint32_t>::max();
    if (c.y >= height)
      return std::numeric_limits<uint32_t>::max();
    return c.y * width + c.x;
  }
};


std::vector<std::string> split(const std::string& str, const char delimiter) {

  std::vector<std::string> words;
  auto word_start = 0ul;
  auto current_delimiter = str.find(delimiter, 0);

  do {
    if (current_delimiter == std::string::npos) {
      words.push_back(str.substr(word_start, str.length() - word_start));
      break;
    }

    words.push_back(str.substr(word_start, current_delimiter - word_start));
    word_start = current_delimiter + 1;
    current_delimiter = str.find(delimiter, word_start);
  } while(true);

  return words;
}

std::vector<hotspot> load_hotspots(const char* filename) {

  std::ifstream file(filename);

  std::string line;
  std::getline(file, line);
  std::vector<hotspot> hotspots;
  while (std::getline(file, line)) {
    const auto words = split(line, ',');
    if (words.size() == 4) {
      hotspot h;
      h.x = std::stoi(words[0]);
      h.y = std::stoi(words[1]);
      h.start_round = std::stoi(words[2]);
      h.end_round = std::stoi(words[3]);
      hotspots.emplace_back(h);
    }
  }

  return hotspots;
}

std::vector<coordinate> load_coords(const char* filename) {

  std::ifstream file(filename);

  std::string line;
  std::getline(file, line);
  std::vector<coordinate> coords;
  while (std::getline(file, line)) {
    const auto words = split(line, ',');
    if (words.size() == 2)
      coords.emplace_back(std::stoi(words[0]), std::stoi(words[1]));
  }

  return coords;
}
