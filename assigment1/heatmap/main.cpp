#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

struct hotspot {
  hotspot(uint32_t x, uint32_t y, uint32_t start_round, uint32_t end_round)
    : x{x}, y{y}, start_round{start_round}, end_round{end_round} {}


  void print() const {
    std::cout << "x = " << x
              << ", y = " << y
              << ", start_round = " << start_round
              << ", end_round " << end_round
              << std::endl;
  }

  uint32_t x;
  uint32_t y;
  uint32_t start_round;
  uint32_t end_round;
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
    if (words.size() == 4)
      hotspots.emplace_back(std::stoi(words[0]), std::stoi(words[1]), std::stoi(words[2]), std::stoi(words[3]));
  }

  return hotspots;
}

int main(int argc, char *argv[]) {
  if (argc > 6) throw std::runtime_error("Wrong number of arguments");
  if (argc < 5) throw std::runtime_error("Wrong number of arguments");

  const auto width = std::stoi(argv[1]);
  const auto height = std::stoi(argv[2]);
  const auto rounds =  std::stoi(argv[3]);

  const auto hotspots = load_hotspots(argv[4]);

  for (const auto& h : hotspots)
    h.print();


  return 0;
}