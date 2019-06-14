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

//std::string GetDeviceName (cl_device_id id)
//{
//    size_t size = 0;
//    clGetDeviceInfo (id, CL_DEVICE_NAME, 0, nullptr, &size);
//
//    std::string result;
//    result.resize (size);
//    clGetDeviceInfo (id, CL_DEVICE_NAME, size,
//                     const_cast<char*> (result.data ()), nullptr);
//
//    return result;
//}

void CheckError (cl_int error, std::string msg)
{
    if (error != CL_SUCCESS) {
        std::cerr << "OpenCL call failed with error " << error << ": " << msg << std::endl;
        std::exit (1);
    }
}

//std::string LoadKernel (const char* name)
//{
//    std::ifstream in(name);
//    std::string result (
//            (std::istreambuf_iterator<char> (in)),
//            std::istreambuf_iterator<char> ());
//    return result;
//}
//
//cl_program BuildProgram (const std::string& source,
//                          cl_context context)
//{
//
//    cl_int error = 0;
//    cl::Program program(context, source.c_str());
//    CheckError (error, "Create Program");
//
//    error = program.build();
//    if (error == CL_BUILD_PROGRAM_FAILURE) {
//        cl::vector<cl::Device> devices;
//        devices = context.getInfo<CL_CONTEXT_DEVICES>();
//        CheckError(devices.size() > 0 ? CL_SUCCESS : -1, devices.size() > 0);
//        std::string log = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0], NULL);
//        std::cerr << "Build Log: " << log << std::endl;
//    }
//
//
//    return program;
//}

int main ()
{
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

//
//    // http://www.khronos.org/registry/cl/sdk/1.1/docs/man/xhtml/clCreateContext.html
//    const cl_context_properties contextProperties [] =
//            {
//                    CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties> (platformIds [0]),
//                    0, 0
//            };
//
//    cl_int error = CL_SUCCESS;
//    cl_context context = clCreateContext (contextProperties, deviceIdCount,
//                                          deviceIds.data (), nullptr, nullptr, &error);
//    CheckError (error, "Create Context");
//
//    std::cout << "Context created" << std::endl;
//
//    cl_program program = BuildProgram (LoadKernel ("kernels/saxpy.cl"),
//                                        context);
//
//
//
//    cl_kernel kernel = clCreateKernel (program, "SAXPY", &error);
//    CheckError (error, "Create kernel");
//
//    // Prepare some test data
//    static const size_t testDataSize = 1 << 10;
//    std::vector<float> a (testDataSize), b (testDataSize);
//    for (int i = 0; i < testDataSize; ++i) {
//        a [i] = static_cast<float> (1);
//        b [i] = static_cast<float> (1);
//    }
//
//    cl_mem aBuffer = clCreateBuffer (context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
//                                     sizeof (float) * (testDataSize),
//                                     a.data (), &error);
//    CheckError (error, "Create aBuffer");
//
//    cl_mem bBuffer = clCreateBuffer (context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
//                                     sizeof (float) * (testDataSize),
//                                     b.data (), &error);
//    CheckError (error, "create bBuffer");
//
//    // http://www.khronos.org/registry/cl/sdk/1.1/docs/man/xhtml/clCreateCommandQueue.html
//    cl_command_queue queue = clCreateCommandQueue (context, deviceIds [0],
//                                                   0, &error);
//    CheckError (error, "create queue");
//
//    clSetKernelArg (kernel, 0, sizeof (cl_mem), &aBuffer);
//    clSetKernelArg (kernel, 1, sizeof (cl_mem), &bBuffer);
//    static const float two = 2.0f;
//    clSetKernelArg (kernel, 2, sizeof (float), &two);
//
//    // http://www.khronos.org/registry/cl/sdk/1.1/docs/man/xhtml/clEnqueueNDRangeKernel.html
//    const size_t globalWorkSize [] = { testDataSize, 0, 0 };
//    CheckError (clEnqueueNDRangeKernel (queue, kernel, 1,
//                                        nullptr,
//                                        globalWorkSize,
//                                        nullptr,
//                                        0, nullptr, nullptr), "Run kernel");
//
//    // Get the results back to the host
//    // http://www.khronos.org/registry/cl/sdk/1.1/docs/man/xhtml/clEnqueueReadBuffer.html
//    CheckError (clEnqueueReadBuffer (queue, bBuffer, CL_TRUE, 0,
//                                     sizeof (float) * testDataSize,
//                                     b.data (),
//                                     0, nullptr, nullptr), "read result");
//
//    for (float & i : b)
//        std::cout << i << ' ';
//
//    clReleaseCommandQueue (queue);
//
//    clReleaseMemObject (bBuffer);
//    clReleaseMemObject (aBuffer);
//
//    clReleaseKernel (kernel);
//    clReleaseProgram (program);
//
//    clReleaseContext (context);

//    https://anteru.net/blog/2012/getting-started-with-opencl-part-3/
}