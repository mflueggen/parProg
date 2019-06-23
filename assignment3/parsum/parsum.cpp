#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cmath>

#ifdef TIMER
#include <ctime>
#endif

#ifdef __APPLE__
#include "OpenCL/cl2.hpp"
#else
#include "CL/cl2.hpp"
#endif

struct Sizes {
  uint64_t local;
  uint64_t global;
};

int roundDown(uint64_t numToRound, uint64_t multiple)
{
    if (multiple == 0)
        return numToRound;

    return numToRound - (numToRound % multiple);
}

void CheckError (cl_int error, const std::string& msg)
{
    if (error != CL_SUCCESS) {
        std::cerr << "OpenCL call failed with error " << error << ": " << msg << std::endl;
        std::exit (1);
    }
}

std::string LoadKernel (const char* name)
{
    std::ifstream in(name);
    std::string result (
            (std::istreambuf_iterator<char> (in)),
            std::istreambuf_iterator<char> ());
    return result;
}

cl::Program BuildProgram (const std::string& source,
                          const cl::Context& context)
{

    cl_int error = 0;
    cl::Program program(context, source);
    CheckError (error, "Create Program");

    error = program.build();
    if (error == CL_BUILD_PROGRAM_FAILURE) {
        cl::vector<cl::Device> devices;
        devices = context.getInfo<CL_CONTEXT_DEVICES>();
        CheckError(!devices.empty() ? CL_SUCCESS : -1, "");
        std::string log = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0], nullptr);
        std::cerr << "Build Log:\n" << log << std::endl;
        exit(-1);
    }


    return program;
}

// This function searches for a optimal combination of Global Work Item Size and Work Group Size
Sizes FindWorkItemSizes(uint64_t start, uint64_t end) {
    uint64_t maxGroupSize = 8192; // This ensures that we never have a range bigger than 32 bit in a single work item
    Sizes sizes = {256, maxGroupSize}; //fix numbers based on test machine but small enough to run on development hardware

    uint64_t maximumItemSize = ((end - start + 1) / 2); // add 2 numbers per work item
    uint64_t preferredItemSize = roundDown(maximumItemSize, sizes.local); //round down to a multiple of the work group size

    if (preferredItemSize == 0)
    { //We have less than 256 numbers to add. Do it in one work group and round down to the next power of 2.
        uint64_t pow2ItemSize = pow(2, floor(log(maximumItemSize)/log(2)));
        if (pow2ItemSize == 0) pow2ItemSize = 1;
        sizes.local = sizes.global = pow2ItemSize;
        return sizes;
    }

    if (preferredItemSize < sizes.global) {
        sizes.global = preferredItemSize;
    }

    return sizes;
}


int main(int argc, char *argv[])
{
#ifdef TIMER
    clock_t begin = clock();
#endif
    const auto start = std::stoull(argv[1]);
    const auto end = std::stoull(argv[2]);

    //select platform
    cl::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    if (platforms.empty()) {
        std::cerr << "No OpenCL platform found" << std::endl;
        return 1;
    } else {
#ifdef DEV
        std::cout << "Found " << platforms.size() << " platform(s)" << std::endl;
#endif
    }

#ifdef DEV
    for (auto & platform : platforms) {
        std::cout << "\t- " << platform.getInfo<CL_PLATFORM_NAME>() << std::endl;
    }
#endif

    // select device from platform with cpu as recommended in lecture
    cl::vector<cl::Device> devices;
    for (auto & platform : platforms) {
        platform.getDevices(CL_DEVICE_TYPE_CPU, &devices);
        if (!devices.empty()) {
            break;
        }
    }
    if (devices.empty()) {
        for (auto & platform : platforms) {
            platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
            if (!devices.empty()) {
                break;
            }
        }
    }
    if (devices.empty()) {
        std::cerr << "No proper OpenCL device found. Abort." << std::endl;
        return 1;
    }

#ifdef DEV
    std::cout << "Found " << devices.size() << " device(s)" << std::endl;
    for (auto & device : devices) {
        std::cout << "\t- " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
    }
#endif

    // create a context using the platform and devices
    cl_context_properties cps[3] = {
            CL_CONTEXT_PLATFORM,
            (cl_context_properties)(platforms[0])(),
            0
    };
    cl_int error = CL_SUCCESS;
    cl::Context context(devices, cps, nullptr, nullptr, &error);
    CheckError (error, "Create Context");

#ifdef DEV
    std::cout << "Context created" << std::endl;
#endif

    cl::Program program = BuildProgram (LoadKernel ("parsum.cl"),
                                        context);

    cl::Kernel kernel(program, "parsum", &error);
    CheckError (error, "Create kernel");

    Sizes workGroupSizes = FindWorkItemSizes(start, end);

    std::vector<cl_uint> starts(workGroupSizes.global * 2);
    std::vector<cl_uint> ranges(workGroupSizes.global);
    const uint64_t chunk_size = ((end - start) / workGroupSizes.global);
    uint64_t start_of_range = start;
    uint64_t end_of_range = 0;
    for (auto i = 0ul; i < workGroupSizes.global * 2; i += 2) {
        if(start_of_range > end)
        { // some idle kernels... (not enough time....)
            starts[i] = 0;
            starts[i+1] = 0;
            ranges[i/2] = 0;
            continue;
        }
        end_of_range = start_of_range + chunk_size;
        if (end_of_range >= end) {
            end_of_range = end;
        }
        starts[i] = static_cast<uint32_t>(start_of_range);
        starts[i+1] = static_cast<uint32_t>(start_of_range>>32);
        ranges[i/2] = end_of_range - start_of_range;
        start_of_range = end_of_range + 1;
    }

    // Create memory buffers
    cl::Buffer startsBuffer(context,
                       CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                       sizeof (cl_uint) * (workGroupSizes.global * 2),
                       starts.data(), &error);
    CheckError (error, "Create startsBuffer");

    cl::Buffer rangesBuffer(context,
                            CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                            sizeof (cl_uint) * (workGroupSizes.global),
                            ranges.data(), &error);
    CheckError (error, "Create rangesBuffer");

    uint numberWorkGroups = workGroupSizes.global / workGroupSizes.local;

    std::vector<cl_uint> output(numberWorkGroups * 4);
    cl::Buffer outputBuffer(context,
                       CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                       sizeof (cl_uint) * (numberWorkGroups * 4), //possible 128 bit
                       output.data(), &error);
    CheckError (error, "Create outputBuffer");

    cl::CommandQueue queue(context, devices[0], 0, &error);
    CheckError (error, "create queue");
    // Set arguments to kernel
    kernel.setArg(0, startsBuffer);
    kernel.setArg(1, rangesBuffer);
    kernel.setArg(2, outputBuffer);
    kernel.setArg(3, sizeof(cl_uint) * workGroupSizes.local * 4, nullptr);

    cl::NDRange globalWorkSize(workGroupSizes.global);
    cl::NDRange local(workGroupSizes.local);


    error = queue.enqueueNDRangeKernel(kernel, cl::NullRange, globalWorkSize, local);
    CheckError (error, "Run kernel");

    error = queue.enqueueReadBuffer(outputBuffer, CL_TRUE, 0, sizeof (cl_uint) * numberWorkGroups * 4, output.data ());
    CheckError(error, "read result");


    unsigned __int128 sum = 0;
    for (uint64_t i = 0; i < output.size(); i += 4) {
        unsigned __int128 val = ((unsigned __int128) output[i + 0]) << 0  |
                                ((unsigned __int128) output[i + 1]) << 32 |
                                ((unsigned __int128) output[i + 2]) << 64 |
                                ((unsigned __int128) output[i + 3]) << 96;
        sum += val;
    }


    std::vector<int> sum_as_string;

    while (sum > 0) {
      int last_char = sum % 10;
      sum /= 10;
      sum_as_string.push_back(last_char);
    }

    if (sum_as_string.empty())
      std::cout << 0;
    else
    {
      for (auto it = sum_as_string.rbegin(); it != sum_as_string.rend(); ++it) {
        std::cout << *it;
      }
    }

  std::cout << std::endl;

#ifdef DEV
    for (const auto &out: output) {
        std::cout << out << std::endl;
    }
#endif

    //No Release due to RAII

#ifdef TIMER
    clock_t finish = clock();
    double elapsed_secs = double(finish - begin) / CLOCKS_PER_SEC;
    std::cout << "Took " << elapsed_secs << " seconds" << std::endl;
#endif
}
