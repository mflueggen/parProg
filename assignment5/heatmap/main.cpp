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

#include <mpi.h>

#include "utils.hpp"

void save_heatmap(const std::vector<std::vector<float>>& heatmap, const char* coord_file) {
  std::vector<coordinate> coords;

  // load coords file if given
  if (coord_file)
    coords = load_coords(coord_file);

  std::ofstream output_file("output.txt");
  output_file << std::setprecision(4) << std::fixed;

  // output heatmap
  if (coords.empty()) {
    // print result
    for (uint32_t y = 0; y < heatmap.size(); ++y) {
      for (uint32_t x = 1; x < heatmap[y].size() - 1; ++x) {
        const auto value = heatmap[y][x];
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
      output_file << heatmap[coord.y][coord.x + 1] << '\n';
    }
  }

  output_file.close();
}

int main(int argc, char *argv[]) {

  if (argc > 6) throw std::runtime_error("Wrong number of arguments");
  if (argc < 5) throw std::runtime_error("Wrong number of arguments");

  // Initialize values
  const auto arg_width = std::stoi(argv[1]) + 2;
  const auto arg_height = std::stoi(argv[2]);
  const auto arg_rounds = std::stoi(argv[3]);

  // Initialize the MPI environment
  MPI_Init(NULL, NULL);

  // Get the number of processes
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  // Get the rank of the process
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  std::vector<std::vector<float>> heatmaps[2];

  // vector of all hotspots
  std::vector<hotspot> hotspots = load_hotspots(argv[4]);

  // Decompose problem
  const auto problem_height = (arg_height + world_size - 1) / world_size;
  const auto start_row = std::min(world_rank * problem_height, arg_height);
  const auto end_row = std::min(start_row + problem_height, arg_height);
  const auto height = end_row - start_row + 2;

//  std::cout << "world_size: " << world_size  << " world_rank: " << world_rank  << " height: " << height << std::endl;

  // Initialize _decomposed heatmap
  for (auto i = 0; i < 2; ++i) {
    heatmaps[i].resize(height);
    for (auto &row : heatmaps[i])
      row.resize(arg_width);
  }

  // keep only relevant hotspots and activate them
  for (auto it = hotspots.begin(); it != hotspots.end(); ) {
    const auto& hotspot = *it;
    if (hotspot.y >= start_row && hotspot.y < end_row) {
      if (hotspot.start_round == 0) {
        heatmaps[0][hotspot.y - start_row + 1][hotspot.x + 1] = 1.0f;
      }
      ++it;
    }
    else {
      it = hotspots.erase(it);
    }
  }

  for (auto round = 1u; round <= arg_rounds; ++round) {
    const auto current_heatmap_index = round % 2;
    const auto old_heatmap_index = (current_heatmap_index + 1) % 2;

    // sende Ränder an anderen Rank bzw. empfange sie von dort.
    // old_heatmap_index sind die Berechnungen der letzten Runde.
    // sende old heatmap index bzw. empfange old_heatmap_index

    // rank 0 sendet unterste Zeile an rank 1
    // rank 0 wartet auf oberste Zeile von rank 1

    // gerader rank sendet erst
    // ungerader rank empfängt zuerst

    if (world_rank == 0) {
      // erstes element
      if (world_size > 1) {
        MPI_Send(heatmaps[old_heatmap_index][height - 2].data(), arg_width, MPI_FLOAT, world_rank + 1, 0, MPI_COMM_WORLD);
        MPI_Recv(heatmaps[old_heatmap_index][height - 1].data(), arg_width, MPI_FLOAT, world_rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      }
    } else if (world_rank == world_size - 1) {
      // letztes element
      MPI_Send(heatmaps[old_heatmap_index][1].data(), arg_width, MPI_FLOAT, world_rank - 1, 0, MPI_COMM_WORLD);
      MPI_Recv(heatmaps[old_heatmap_index][0].data(), arg_width, MPI_FLOAT, world_rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    } else if (world_rank & 1) {
      // in between ungerade
      MPI_Recv(heatmaps[old_heatmap_index][0].data(), arg_width, MPI_FLOAT, world_rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      MPI_Recv(heatmaps[old_heatmap_index][height - 1].data(), arg_width, MPI_FLOAT, world_rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      MPI_Send(heatmaps[old_heatmap_index][1].data(), arg_width, MPI_FLOAT, world_rank - 1, 0, MPI_COMM_WORLD);
      MPI_Send(heatmaps[old_heatmap_index][height - 2].data(), arg_width, MPI_FLOAT, world_rank + 1, 0, MPI_COMM_WORLD);
    } else {
      // in between gerade
      MPI_Send(heatmaps[old_heatmap_index][1].data(), arg_width, MPI_FLOAT, world_rank - 1, 0, MPI_COMM_WORLD);
      MPI_Send(heatmaps[old_heatmap_index][height - 2].data(), arg_width, MPI_FLOAT, world_rank + 1, 0, MPI_COMM_WORLD);

      MPI_Recv(heatmaps[old_heatmap_index][0].data(), arg_width, MPI_FLOAT, world_rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      MPI_Recv(heatmaps[old_heatmap_index][height - 1].data(), arg_width, MPI_FLOAT, world_rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }


    // führe Berechnung durch
    for (auto row = 1; row < height - 1; ++row) {
      for (auto col = 1; col < arg_width - 1; ++col) {
        double sum = 0;
        // for the 3x3 matrix around coord
        for (auto y = row - 1; y <= row + 1; ++y) {
          for (auto x = col - 1; x <= col + 1; ++x) {
            sum += heatmaps[old_heatmap_index][y][x];
          }
        }
        heatmaps[current_heatmap_index][row][col] = sum / 9.0;
      }
    }

    // set active hotspots back to one
    for (auto h = 0; h < hotspots.size(); ++h) {
      const auto &hotspot = hotspots[h];
      if (round >= hotspot.start_round && round < hotspot.end_round)
        heatmaps[current_heatmap_index][hotspot.y - start_row + 1][hotspot.x + 1] = 1.0;
    }
  }

  // wait for workers to terminate

  const auto current_heatmap_index = arg_rounds % 2;

  if (world_rank == 0) {
    std::vector<std::vector<float>> result(arg_height);

    // copy rows from local heatmap
    for (auto i = 1; i < height - 1; ++i) {
      result[i - 1] = heatmaps[current_heatmap_index][i];
    }

    for (auto i = height - 1; i <= arg_height; ++i) {
      std::vector<float> row(arg_width);
      MPI_Recv(row.data(), arg_width, MPI_FLOAT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      const auto row_index = static_cast<int>(row[0]);
      result[row_index] = row;
    }

    save_heatmap(result, argc == 6 ? argv[5] : nullptr);
  } else {
    for (auto i = 1; i < height - 1; ++i) {
      auto& row = heatmaps[current_heatmap_index][i];
      row[0] = start_row + i - 1;
      MPI_Send(row.data(), arg_width, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
    }
  }


  // Finalize the MPI environment.
  MPI_Finalize();
  return 0;
}
