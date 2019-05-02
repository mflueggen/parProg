#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "utils.hpp"



int main(int argc, char *argv[]) {
  if (argc > 6) throw std::runtime_error("Wrong number of arguments");
  if (argc < 5) throw std::runtime_error("Wrong number of arguments");

  const auto width = std::stoi(argv[1]);
  const auto height = std::stoi(argv[2]);
  const auto rounds =  std::stoi(argv[3]);

  const auto hotspots = load_hotspots(argv[4]);
  std::vector<coord> coords;

  if (argc == 6)
    coords = load_coords(argv[5]);


  return 0;
}