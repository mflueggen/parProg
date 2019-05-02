#define DEV

#ifdef DEV
#include <iostream>
#endif

#include <string>     // std::string, std::stoi
#ifdef DEV
    #include "pthread.h"
#else
    #include <pthread.h>
#endif

int main(int argc, char *argv[]) {
    const uint32_t thread_count = (uint32_t) std::stoi(argv[1]);
    const uint32_t start = (uint32_t) std::stoi(argv[2]);
    const uint32_t end = (uint32_t) std::stoi(argv[3]);

#ifdef DEV
    std::cout << "Threadcount: " << thread_count << std::endl;
    std::cout << "Calculating sum from " << start << " to " << end  << std::endl;
#endif

    uint32_t diff = end - start;
    uint32_t junk_size = diff / thread_count + 1;  // +1 to distribute the remainder to all threads

#ifdef DEV
    std::cout << "Doing the following parallel junks:" << std::endl;

    for (int i = 0; i < thread_count-1; ++i) {
        std::cout << start + i*junk_size << "-" << start + i*junk_size + junk_size - 1 << std::endl;
    }
    std::cout << start + (thread_count-1)*junk_size << "-" << end << std::endl;
#endif



    return 0;
}