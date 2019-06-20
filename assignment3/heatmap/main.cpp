#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "utils.hpp"

extern "C" {
#include <CL/cl.h>
}

#define get_h(h, y, x) heatmaps[h * width * height + y * width + x]

void check_err(const cl_int err_code, const std::string function_name) {
  if (err_code) {
    std::cout << function_name << " failed." << std::endl;
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
  cl_float* heatmaps = new cl_float[SIZE_OF_HEATMAP];
  std::memset(heatmaps, 0, sizeof(cl_float) * SIZE_OF_HEATMAP);

  // vector of all hotspots
  std::vector<hotspot> hotspots = load_hotspots(argv[4]);
  std::vector<coordinate> coords;

  // load coords file if given
  if (argc == 6)
    coords = load_coords(argv[5]);

  for (auto& h : hotspots) {
    ++h.x;
    ++h.y;
    if (h.start_round == 0)
      // activate hotspot on "old" heatmap
      get_h(0, h.y, h.x) = 1.0;
  }

  cl_platform_id platform_id;
  cl_int ret;

  ret = clGetPlatformIDs(1, &platform_id, nullptr);
  check_err(ret, "clGetPlatformIDs");

  cl_device_id device_id;
  ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, nullptr);
  check_err(ret, "clGetDeviceIDs");

  cl_context context = clCreateContext(nullptr, 1, &device_id, nullptr, nullptr, &ret);
  check_err(ret, "clCreateContext");

  cl_command_queue command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
  check_err(ret, "clCreateCommandQueue");

  // Create memory buffers on the device for each vector
  cl_mem buf_heatmaps = clCreateBuffer(context, CL_MEM_READ_WRITE, SIZE_OF_HEATMAP * sizeof(cl_float), nullptr, &ret);
  check_err(ret, "clCreateBuffer(buf_heatmaps)");

  cl_mem buf_hotspots = clCreateBuffer(context, CL_MEM_READ_ONLY, hotspots.size() * sizeof(hotspot), nullptr, &ret);
  check_err(ret, "clCreateBuffer(buf_hotspots)");

  // Copy the lists A and B to their respective memory buffers
  ret = clEnqueueWriteBuffer(command_queue, buf_heatmaps, CL_TRUE, 0,
                             SIZE_OF_HEATMAP * sizeof(cl_float), heatmaps, 0, nullptr, nullptr);
  check_err(ret, "clEnqueueWriteBuffer(buf_heatmaps)");
  ret = clEnqueueWriteBuffer(command_queue, buf_hotspots, CL_TRUE, 0,
                             hotspots.size() * sizeof(hotspot), hotspots.data(), 0, nullptr, nullptr);
  check_err(ret, "clEnqueueWriteBuffer(buf_hotspots)");


  std::ifstream kernel_file_stream("kernel.c");
  std::string kernel_src_string((std::istreambuf_iterator<char>(kernel_file_stream)),
                  std::istreambuf_iterator<char>());
  kernel_file_stream.close();
  const auto* kernel_src = kernel_src_string.c_str();

  std::cout << kernel_src << std::endl;

  // Create a program from the kernel source
  cl_program program = clCreateProgramWithSource(context, 1, &kernel_src, nullptr, &ret);
  check_err(ret, "clCreateProgramWithSource");

  // Build the program
  ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
  check_err(ret, "clBuildProgram");


  // Create the OpenCL kernel
  cl_kernel kernel = clCreateKernel(program, "simulate", &ret);
  check_err(ret, "clCreateKernel");

  // Set the arguments of the kernel
  ret = clSetKernelArg(kernel, 0, sizeof(cl_ushort), &width);
  check_err(ret, "clSetKernelArg(width)");
  ret = clSetKernelArg(kernel, 1, sizeof(cl_ushort), &height);
  check_err(ret, "clSetKernelArg(height)");
  ret = clSetKernelArg(kernel, 2, sizeof(cl_ushort), &rounds);
  check_err(ret, "clSetKernelArg(rounds)");
  ret = clSetKernelArg(kernel, 3, sizeof(cl_mem), &buf_heatmaps);
  check_err(ret, "clSetKernelArg(buf_heatmaps)");
  const cl_ushort num_hotspots = hotspots.size();
  ret = clSetKernelArg(kernel, 4, sizeof(cl_ushort), &num_hotspots);
  check_err(ret, "clSetKernelArg(num_hotspots)");
  ret = clSetKernelArg(kernel, 5, sizeof(cl_mem), &buf_hotspots);
  check_err(ret, "clSetKernelArg(num_hotspots)");


#ifdef BENCHMARK
  const auto start = std::chrono::high_resolution_clock::now();
#endif

  const size_t global_work_offset[] = {1, 1};
  const size_t global_item_size[] = {width - 2ul, height - 2ul};
  const size_t local_item_size[] = {3, 3};

  // Heigh+2 und width + 2 muss durch 3 teilbar sein

  ret = clEnqueueNDRangeKernel(command_queue, kernel, 2, global_work_offset,
                               global_item_size, nullptr, 0, nullptr, nullptr);
  check_err(ret, "clEnqueueNDRangeKernel");

  // Read the memory buffer C on the device to the local variable C
  ret = clEnqueueReadBuffer(command_queue, buf_heatmaps, CL_TRUE, 0,
                            SIZE_OF_HEATMAP * sizeof(cl_float), heatmaps, 0, NULL, NULL);
  check_err(ret, "clEnqueueReadBuffer");


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
  clReleaseContext(context);

  return 0;
}
