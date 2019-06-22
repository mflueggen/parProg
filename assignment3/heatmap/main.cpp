#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "utils.hpp"

#include <CL/cl.hpp>

#define get_h(h, y, x) heatmaps[h * width * height + y * width + x]

void check_err(const cl_int err_code, const std::string function_name) {
  if (err_code) {
    std::cout << function_name << " failed with err_code " << err_code << "." << std::endl;
    exit(0);
  }
}

int main(int argc, char *argv[]) {
  if (argc > 6) throw std::runtime_error("Wrong number of arguments");
  if (argc < 5) throw std::runtime_error("Wrong number of arguments");

  // Initialize values
  const cl_ushort width = std::stoi(argv[1]) + 2;
  const cl_ushort height = std::stoi(argv[2]) + 2;
  const cl_ushort rounds = std::stoi(argv[3]);

  const size_t SIZE_OF_HEATMAP = width * height * 2;

  // we manage two heatmaps. Each round the "active" heatmap is updated based on the values of the other "old" heatmap
  // that way we don't need to synchronize data bounderies between threads.
  // a heatmap is modeled as an one dimensional array. Using _width and _height we can compute either the two
  // dimensional coordinate from the given index or compute the index given the coordinate.
  std::vector<cl_float> heatmaps(SIZE_OF_HEATMAP);

  // vector of all hotspots
  std::vector<hotspot> hotspots = load_hotspots(argv[4]);
  std::vector<coordinate> coords;

  // load coords file if given
  if (argc == 6)
    coords = load_coords(argv[5]);

  for (auto& h : hotspots) {
    // account for "zero border" around the heatmap and shift hotspot
    ++h.x;
    ++h.y;
    if (h.start_round == 0)
      // activate hotspot on "old" heatmap
      get_h(0, h.y, h.x) = 1.0;
  }

  cl_int ret;
  std::vector<cl::Platform> platforms;
  ret = cl::Platform::get(&platforms);
  check_err(ret, "cl::Platform::get(&platforms)");

#ifndef NDEBUG
  std::string platform_info;
  std::cout << "Platforms:" << std::endl;
  for (const auto& p : platforms) {
    p.getInfo(CL_PLATFORM_NAME, &platform_info);
    std::cout << platform_info << std::endl;
  }
#endif
  std::vector<cl::Device> devices;
  ret =  platforms.front().getDevices(CL_DEVICE_TYPE_DEFAULT, &devices);
  check_err(ret, "platforms.front().getDevices");

  const auto& device = devices.front();
  const auto context = cl::Context(device, nullptr, nullptr, nullptr, &ret);
  check_err(ret, "cl::Context");

  auto buf_heatmaps = cl::Buffer(context, CL_MEM_READ_WRITE, SIZE_OF_HEATMAP * sizeof(cl_float), nullptr, &ret);
  check_err(ret, "clCreateBuffer(buf_heatmaps)");

  auto buf_hotspots = cl::Buffer(context, CL_MEM_READ_ONLY, hotspots.size() * sizeof(hotspot), nullptr, &ret);
  check_err(ret, "clCreateBuffer(buf_hotspots)");

  auto command_queue = cl::CommandQueue(context, device, 0, &ret);
  check_err(ret, "cl::CommandQueue");


  ret = cl::copy(command_queue, heatmaps.begin(), heatmaps.end(), buf_heatmaps);
  check_err(ret, "copy(buf_heatmaps)");

  ret = cl::copy(command_queue, hotspots.begin(), hotspots.end(), buf_hotspots);
  check_err(ret, "copy(buf_heatmaps)");

  ret = command_queue.enqueueWriteBuffer(buf_hotspots, CL_TRUE, 0, hotspots.size() * sizeof(hotspot),
    hotspots.data(), nullptr, nullptr);
  check_err(ret, "command_queue.enqueueWriteBuffer(buf_hotspots)");

  std::ifstream kernel_file_stream("kernel.c");
  std::string kernel_src_string((std::istreambuf_iterator<char>(kernel_file_stream)),
                                std::istreambuf_iterator<char>());
  kernel_file_stream.close();

  const auto program = cl::Program(context, kernel_src_string, true, &ret);
  check_err(ret, "cl::Program");

  auto kernel =
    cl::make_kernel<cl_ushort, cl_ushort, cl_ushort, cl::Buffer&, cl_ushort, cl::Buffer&>(program, "simulate", &ret);
  check_err(ret, "cl::make_kernel");

  for (unsigned short round = 1; round <= rounds; ++round) {
    auto event = kernel(cl::EnqueueArgs(command_queue,
                                              cl::NDRange(1, 1),
                                              cl::NDRange(width-2u, height-2u), cl::NDRange(1, 1)),
                              width, height, round, buf_heatmaps, hotspots.size(), buf_hotspots);
    ret= event.wait();
    check_err(ret, "event.wait()");
  }

  ret = cl::copy(command_queue, buf_heatmaps, begin(heatmaps), end(heatmaps));
  check_err(ret, "cl::copy");

  std::ofstream output_file("output.txt");
  output_file << std::setprecision(4) << std::fixed;

  // output heatmap
  if (coords.empty()) {
    // print result
    for (uint32_t y = 1; y < height - 1; ++y) {
      for (uint32_t x = 1; x < width - 1; ++x) {
        const auto value = get_h(rounds % 2, y, x);
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
      output_file << get_h(rounds % 2, coord.y + 1, coord.x + 1) << '\n';
    }
  }

  output_file.close();



/*




#ifdef BENCHMARK
  const auto end = std::chrono::high_resolution_clock::now();
  std::cout << "Runtime: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms"
            << std::endl;
#endif

  std::ofstream output_file("output.txt");
  output_file << std::setprecision(4) << std::fixed;

  // output heatmap
  if (coords.empty()) {
    // print result
    for (uint32_t y = 1; y < height - 1 ; ++y) {
      for (uint32_t x = 1; x < width - 1; ++x) {
        const auto value = get_h(rounds % 2, y, x);
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
      output_file << get_h(rounds % 2, coord.y + 1, coord.x + 1) << '\n';
    }
  }

  output_file.close();
  clFlush(command_queue);
  clFinish(command_queue);
  clReleaseKernel(kernel);
  clReleaseProgram(program);
  clReleaseMemObject(buf_hotspots);
  clReleaseMemObject(buf_heatmaps);
  clReleaseCommandQueue(command_queue);
  clReleaseContext(context);*/

  return 0;
}
