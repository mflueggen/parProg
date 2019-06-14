#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120

#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#ifdef __APPLE__
#include "OpenCL/cl2.hpp"
#else
#include "CL/cl2.hpp"
#endif

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

int main ()
{
    //select platform
    cl::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    if (platforms.empty()) {
        std::cerr << "No OpenCL platform found" << std::endl;
        return 1;
    } else {
        std::cout << "Found " << platforms.size() << " platform(s)" << std::endl;
    }

    for (auto & platform : platforms) {
        std::cout << "\t- " << platform.getInfo<CL_PLATFORM_NAME>() << std::endl;
    }

    // select device
    cl::vector<cl::Device> devices;
    platforms[0].getDevices(CL_DEVICE_TYPE_GPU, &devices);

    if (devices.empty()) {
        std::cerr << "No OpenCL devices found" << std::endl;
        return 1;
    } else {
        std::cout << "Found " << devices.size() << " device(s)" << std::endl;
    }

    for (auto & device : devices) {
        std::cout << "\t- " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
    }

    // create a context using the platform and devices
    cl_context_properties cps[3] = {
            CL_CONTEXT_PLATFORM,
            (cl_context_properties)(platforms[0])(),
            0
    };
    cl_int error = CL_SUCCESS;
    cl::Context context(devices, cps, nullptr, nullptr, &error);
    CheckError (error, "Create Context");

    std::cout << "Context created" << std::endl;

    cl::Program program = BuildProgram (LoadKernel ("kernels/saxpy.cl"),
                                        context);

    cl::Kernel kernel(program, "SAXPY", &error);
    CheckError (error, "Create kernel");

    // Prepare some test data
    static const size_t testDataSize = 1 << 10;
    std::vector<float> a (testDataSize), b (testDataSize);
    for (int i = 0; i < testDataSize; ++i) {
        a [i] = static_cast<float> (1);
        b [i] = static_cast<float> (1);
    }

    // Create memory buffers
    cl::Buffer aBuffer(context,
            CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
            sizeof (float) * (testDataSize),
            a.data(), &error);
    CheckError (error, "Create aBuffer");

    cl::Buffer bBuffer(context,
                       CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                       sizeof (float) * (testDataSize),
                       b.data(), &error);
    CheckError (error, "Create bBuffer");

    cl::CommandQueue queue(context, devices[0], 0, &error);
    CheckError (error, "create queue");
    // Set arguments to kernel
    kernel.setArg(0, aBuffer);
    kernel.setArg(1, bBuffer);
    static const float two = 2.0f;
    kernel.setArg(2, two);

    cl::NDRange globalWorkSize(testDataSize);
    cl::NDRange local(cl::NullRange);
    error = queue.enqueueNDRangeKernel(kernel, cl::NullRange, globalWorkSize, local);
    CheckError (error, "Run kernel");

    error = queue.enqueueReadBuffer(bBuffer, CL_TRUE, 0, sizeof (float) * testDataSize, b.data ());
    CheckError(error, "read result");

    for (float & i : b)
        std::cout << i << ' ';

    //No Release due to RAII
}