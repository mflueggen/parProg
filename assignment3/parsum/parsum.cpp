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

int roundDown(int numToRound, int multiple)
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
    uint64_t maxGroupSize = 8192;
    uint64_t maxItemSize = maxGroupSize*maxGroupSize*maxGroupSize; //This ensures that we never have a range bigger than 32 bit in a single work item
    Sizes sizes = {256, maxItemSize}; //fix numbers based on test machine

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

    // select device
    cl::vector<cl::Device> devices;
    platforms[0].getDevices(CL_DEVICE_TYPE_GPU, &devices);

    if (devices.empty()) {
        std::cerr << "No OpenCL devices found" << std::endl;
        return 1;
    } else {
#ifdef DEV
        std::cout << "Found " << devices.size() << " device(s)" << std::endl;
#endif
    }
#ifdef DEV
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

    std::vector<cl_uint> starts(workGroupSizes.global);
    std::vector<cl_uint> ranges(workGroupSizes.global);
    const uint64_t chunk_size = ceil(((double)(end - start) / workGroupSizes.global));
    uint64_t start_of_range = start;
    uint64_t end_of_range = 0;
    for (auto i = 0ul; i < workGroupSizes.global; ++i) {
        if(start_of_range > end)
        { // some idle kernels... (not enough time....)
            starts[i] = 0;
            ranges[i] = 0;
            continue;
        }
        end_of_range = start_of_range + chunk_size;
        if (end_of_range >= end) {
            end_of_range = end;
        }
        starts[i] = start_of_range;
        ranges[i] = end_of_range - start_of_range;
        start_of_range = end_of_range + 1;
    }


    // Create memory buffers
    cl::Buffer startsBuffer(context,
                       CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                       sizeof (cl_uint) * (workGroupSizes.global),
                       starts.data(), &error);
    CheckError (error, "Create startsBuffer");

    cl::Buffer rangesBuffer(context,
                            CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                            sizeof (cl_uint) * (workGroupSizes.global),
                            ranges.data(), &error);
    CheckError (error, "Create rangesBuffer");

    uint numberWorkGroups = workGroupSizes.global / workGroupSizes.local;

    std::vector<cl_uint> output(numberWorkGroups);
    cl::Buffer outputBuffer(context,
                       CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                       sizeof (cl_uint) * (numberWorkGroups),
                       output.data(), &error);
    CheckError (error, "Create outputBuffer");

    cl::CommandQueue queue(context, devices[0], 0, &error);
    CheckError (error, "create queue");
    // Set arguments to kernel
    kernel.setArg(0, startsBuffer);
    kernel.setArg(1, rangesBuffer);
    kernel.setArg(2, outputBuffer);
    kernel.setArg(3, sizeof(cl_uint) * workGroupSizes.local, nullptr);

    cl::NDRange globalWorkSize(workGroupSizes.global);
    cl::NDRange local(workGroupSizes.local);


    error = queue.enqueueNDRangeKernel(kernel, cl::NullRange, globalWorkSize, local);
    CheckError (error, "Run kernel");

    error = queue.enqueueReadBuffer(outputBuffer, CL_TRUE, 0, sizeof (cl_uint) * numberWorkGroups, output.data ());
    CheckError(error, "read result");


    uint64_t sum = 0;
    for (cl_uint & i : output)
        sum += i;

    std::cout << sum << std::endl;

    //No Release due to RAII

#ifdef TIMER
    clock_t finish = clock();
    double elapsed_secs = double(finish - begin) / CLOCKS_PER_SEC;
    std::cout << "Took " << elapsed_secs << " seconds" << std::endl;
#endif
}















////#define DEV
////#define TIMER
////#define LOG
//
//#ifdef TIMER
//#include <ctime>
//#endif
//
//#ifdef LOG
//#include <fstream>
//#endif
//
//#include <iostream>
//#include <algorithm>
//#include <string>     // std::string, std::stoull
//#include <pthread.h>
//#include <vector>
//
//struct range {
//  range(unsigned __int128 from, unsigned __int128 to, unsigned __int128 sum)
//    : from{from}, to{to}, sum{sum} {}
//  unsigned __int128 from;
//  unsigned __int128 to;
//  unsigned __int128 sum;
//};
//
//void* slow_sum(void* args) {
//#ifdef LOG
//    std::ofstream logfile;
//    logfile.open ((*((InfInt *)index)).toString() + ".txt");
//    logfile << "Thread " << *((InfInt *)index) << "\n";
//#endif
//    auto* r = static_cast<range*>(args);
//    r->sum = 0;
//    for (unsigned __int128 i = r->from; i <= r->to; ++i) {
//        r->sum += i;
//    }
//#ifdef LOG
//    logfile << (*start) + *((InfInt *)index)*(*junk_size) << "-" << upper_bound << "\n";
//    logfile << "Result: " << result << "\n";
//    logfile.close();
//#endif
//    return nullptr;
//}
//
//int main(int argc, char *argv[]) {
//#ifdef TIMER
//    clock_t begin = clock();
//#endif
//  const auto thread_count = std::stoull(argv[1]);
//  const auto start = std::stoull(argv[2]);
//  const auto end = std::stoull(argv[3]);
//
//
//#ifdef DEV
//    std::cout << "Threadcount: " << thread_count << std::endl;
//    std::cout << "Calculating sum from " << start << " to " << end  << std::endl;
//#endif
//
//    // The smallest junks would be 2 numbers. However, due to the overhead for managing threads
//    // compared to the simple operation '+', we assume that it is the best solution to make the
//    // chunks as big as possible. The '+1' distributes the remainder to all threads instead of just the last thread.
//    const unsigned __int128 chunk_size = ((end - start) / thread_count) + 1;
//
//    // Start each thread with a specific range to summate
//    std::vector<pthread_t> threads(thread_count);
//    std::vector<range> ranges;
//    ranges.reserve(thread_count);
//
//
//    unsigned __int128 start_of_range = start;
//    unsigned __int128 end_of_range = 0;
//    for (auto i = 0ul; i < thread_count; ++i) {
//      end_of_range = start_of_range + chunk_size;
//      if (end_of_range >= end) {
//        end_of_range = end;
//      }
//      ranges.emplace_back(start_of_range, end_of_range, 0);
//      start_of_range = end_of_range + 1;
//      int return_code = pthread_create(&threads[i], nullptr, slow_sum, &ranges[i]);
//      if (return_code != 0) {
//#ifdef DEV
//          std::cout << "Error while creating thread. Code: " << return_code << std::endl;
//#endif
//          exit(-1);
//      }
//    }
//
//
//
//    // We assume that the total amount of threads is not big enough for further parallelism. (overhead > summation)
//    unsigned __int128 sum = 0;
//    for (auto i = 0ul; i < thread_count; ++i) {
//      pthread_join(threads[i], nullptr);
//      sum += ranges[i].sum;
//    }
//
//    std::vector<int> sum_as_string;
//
//    while (sum > 0) {
//      int last_char = sum % 10;
//      sum /= 10;
//      sum_as_string.push_back(last_char);
//    }
//
//    if (sum_as_string.empty())
//      std::cout << 0;
//    else
//    {
//      for (auto it = sum_as_string.rbegin(); it != sum_as_string.rend(); ++it) {
//        std::cout << *it;
//      }
//    }
//
//  std::cout << std::endl;
//
//
//
//#ifdef TIMER
//    clock_t finish = clock();
//    double elapsed_secs = double(finish - begin) / CLOCKS_PER_SEC;
//    std::cout << "Took " << elapsed_secs << " seconds" << std::endl;
//#endif
//
//  return 0;
//}