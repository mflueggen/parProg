#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <metal_pipeline/operator_registry.hpp>
#include <metal_pipeline/data_source.hpp>
#include <metal_pipeline/data_sink.hpp>
#include <metal_pipeline/pipeline_definition.hpp>
#include <metal_pipeline/pipeline_runner.hpp>

//Our 2D-Map type
typedef std::vector <std::array<unsigned char, 64>> map;

//A typical hotspot position
struct hotspot {
  int x;
  int y;

  hotspot(int xInit, int yInit) :
    x(xInit),
    y(yInit) {}
};

//We model the rounds as a Vector containting a Vector of hotspots.
typedef std::vector <std::vector<hotspot>> schedule;

map heatmap;
map heatmap_results;

map initializeHeatmap(const int height) {
  std::array<unsigned char, 64> row = {0};
  return map(height, row);
}

//FYI: ifstreams are not const :(
schedule makeRounds(std::ifstream &hotspotsFile, int width, int height) {
  int maxRounds = 1;
  schedule HotspotSchedule(maxRounds);
  std::string line;

  //Ignore the first line
  std::getline(hotspotsFile, line);

  //process all the following lines
  while (std::getline(hotspotsFile, line)) {
    std::stringstream ss(line);

    int xpos;
    int ypos;
    int startRound;
    int endRound;

    ss >> xpos;
    if (ss.peek() == ',') ss.ignore();
    ss >> ypos;
    if (ss.peek() == ',') ss.ignore();
    ss >> startRound;
    if (ss.peek() == ',') ss.ignore();
    ss >> endRound;

    // Skip this hotspot if it is outside the bounds
    if (xpos >= width || ypos >= height) {
      continue;
    }

    //dynamically make the rounds bigger if we find a hotspot
    //that ends later than our current schedule.
    if (endRound > maxRounds) {
      maxRounds = endRound;
      HotspotSchedule.resize(maxRounds);
    }

    //Make a list of all the Hotspots being active in every round
    for (int i = startRound; i < endRound; i++) {
      HotspotSchedule[i].push_back(hotspot(xpos, ypos));
    }
  }
  return HotspotSchedule;
}



void Simulate(const schedule &HotspotSchedule, const unsigned numberOfRounds) {
  std::cout << "In simulate" << std::endl;
  metal::OperatorRegistry registry(std::getenv("IMAGE_TARGET"));
  std::cout << "Registry initialized" << std::endl;

  auto dataSource = std::make_shared<metal::HostMemoryDataSource>(heatmap.data(), heatmap.size() * 64);
  std::cout << "dataSource loaded" << std::endl;
  auto dataSink = std::make_shared<metal::HostMemoryDataSink>(heatmap_results.data(), heatmap_results.size() * 64);
  std::cout << "dataSink loaded" << std::endl;

  std::cout << "getting transformer" << std::endl;
  auto transformer = registry.operators().at("heatmap");
  std::cout << "received transformer" << std::endl;

  if (transformer == nullptr) {
    std::cout << "Transformer is nullptr" << std::endl;
  }

  std::vector <std::shared_ptr<metal::AbstractOperator>> operators = {dataSource, transformer, dataSink};

  std::cout << "Creating PipelineDefinition" << std::endl;
  auto pipeline = std::make_shared<metal::PipelineDefinition>(operators);
  std::cout << "PipelineDefinition created" << std::endl;

  metal::ProfilingPipelineRunner runner(pipeline, 0);
  runner.selectOperatorForProfiling(transformer);

  for (unsigned currentRound = 0; currentRound < numberOfRounds; currentRound++) {
    //as long as we have hotspots schedule place hotspots
    if (currentRound < HotspotSchedule.size())
      for (const hotspot &h : HotspotSchedule[currentRound])
        heatmap[h.y][h.x] = 255;

    runner.run(currentRound == numberOfRounds - 1 /* = finalize, obtains profiling results */);

    heatmap.swap(heatmap_results);

    dataSource->setSource(heatmap.data());
    dataSink->setDest(heatmap_results.data());
  }

  std::cout << runner.formatProfilingResults();
}

int main(int argc, char *argv[]) {
  std::cout << "Program started" << std::endl;
  // check args
  if ((argc < 4) || (argc > 5)) {
    std::cout << "Usage: ./heatmap height rounds hotspotfile [coordinate-measurementfile]" << std::endl;
    return EXIT_FAILURE;
  }

  //Initialize our map and the result_buffer map then
  unsigned width = 64;
  unsigned height = atoi(argv[1]);
  heatmap = initializeHeatmap(height);
  std::cout << "Heatmap initialized" << std::endl;
  heatmap_results = heatmap;

  std::ifstream hotspots;
  hotspots.open(argv[3], std::ifstream::in);

  // check if hotspots file exists
  if (!hotspots) {
    std::cout << "The file '" << argv[3] << "' does not exist" << std::endl;
    return EXIT_FAILURE;
  }

  schedule HotspotSchedule = makeRounds(hotspots, width, height);
  std::cout << "rounds made" << std::endl;

  unsigned numberOfRounds = atoi(argv[2]);

  std::cout << "Calling Simulate" << std::endl;
  Simulate(HotspotSchedule, numberOfRounds);
  std::cout << "Simulate called successfully" << std::endl;

  if (numberOfRounds < HotspotSchedule.size())
    for (const hotspot &h : HotspotSchedule[numberOfRounds])
      heatmap[h.y][h.x] = 255;

  //make the output.txt file
  std::ofstream output;
  output.open("output.txt");

  //check if we are working with coordinate-files
  bool using_coords_file = (argc == 5);
  if (!using_coords_file) {
    //test-print the result heatmap
    for (unsigned j = 0; j < heatmap.size(); j++) {
      for (unsigned i = 0; i < width; i++) {
        unsigned const value = heatmap[j][i];
        if (value > 230) {
          output << "X";
        } else {
          output << 10 * value / 255;
        }
      }
      output << std::endl;
    }
  } else {
    std::ifstream coords_file;
    coords_file.open(argv[4], std::ifstream::in);

    //ignore the first line
    std::string line;
    std::getline(coords_file, line);
    line = "";

    //read position and write result.
    while (std::getline(coords_file, line)) {
      std::stringstream ss(line);
      int x;
      int y;

      ss >> x;
      if (ss.peek() == ',') ss.ignore();
      ss >> y;

      output << heatmap[x][y] << std::endl;
    }
  }
  return EXIT_SUCCESS;
}
