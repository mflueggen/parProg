#define DEV

#include <iostream>
#include <string>     // std::string, std::stoi
#include <pthread.h>


void *slow_sum(void *range) {
    uint32_t result = 0;
    for (int i = ((uint32_t *)range)[0]; i <= ((uint32_t *)range)[1]; ++i) {
        result += i;
    }
    return (void *)result;
}


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
#endif
    uint32_t ranges[thread_count][2];
    for (int i = 0; i < thread_count-1; ++i) {
        ranges[i][0] = start + i*junk_size; // including the lower bound
        ranges[i][1] = start + i*junk_size + junk_size - 1; // -1 for excluding the upper bound
#ifdef DEV
        std::cout << start + i*junk_size << "-" << start + i*junk_size + junk_size - 1 << std::endl;
#endif
    }
    ranges[thread_count-1][0] = start + (thread_count-1)*junk_size;
    ranges[thread_count-1][1] = end;
#ifdef DEV
    std::cout << start + (thread_count-1)*junk_size << "-" << end << std::endl;

    std::cout << "start threads" << std::endl;
#endif

    pthread_t threads[thread_count];
    for (int j = 0; j < thread_count; ++j) {
        int return_code = pthread_create(&threads[j], NULL, slow_sum, (void *)ranges[j]);
        if (return_code != 0) {
#ifdef DEV
            std::cout << "Error while creating thread. Code: " << return_code << std::endl;
#endif
            exit(-1);
        }
    }

    // Start each thread with a specific range to summate
    void *results[thread_count];
    for (int i = 0; i < thread_count; ++i) {
        pthread_join(threads[i], &results[i]);
#ifdef DEV
        std::cout << "Teilergebnis " << i <<": " << (uint32_t )results[i] << std::endl;
#endif
    }

    // We assume that the total amount of threads is not big enough for further parallelism. (overhead > summation)
    uint32_t final_result = 0;
    for (int i = 0; i < thread_count; ++i) {
        final_result += (uint32_t) results[i];
    }
    std::cout << final_result << std::endl;
    pthread_exit(nullptr);
}