//#define DEV
//#define TIMER
//#define LOG

#ifdef TIMER
#include <ctime>
#endif

#ifdef LOG
#include <fstream>
#endif

#include <iostream>
#include <string>     // std::string, std::stoull
#include <pthread.h>

uint64_t junk_size;
uint64_t thread_count;
uint64_t start;
uint64_t end;

void *slow_sum(void *index) {
#ifdef LOG
    std::ofstream logfile;
    logfile.open (std::to_string(*((uint64_t *)index)) + ".txt");
    logfile << "Thread " << *((uint64_t *)index) << "\n";
#endif
    uint64_t result = 0;
    uint64_t upper_bound = start + *((uint64_t *)index)*junk_size + junk_size - 1;
    if (upper_bound > end) {
        upper_bound = end;
    }
    for (uint64_t i = start + *((uint64_t *)index)*junk_size; i <= upper_bound; ++i) {
        result += i;
    }
#ifdef LOG
    logfile << start + *((uint64_t *)index)*junk_size << "-" << upper_bound << "\n";
    logfile << "Result: " << result << "\n";
    logfile.close();
#endif
    *((uint64_t *)index) = result;
    pthread_exit(nullptr);
    return nullptr;
}


int main(int argc, char *argv[]) {
#ifdef TIMER
    clock_t begin = clock();
#endif
    thread_count = (uint64_t) std::stoull(argv[1]);
    start = (uint64_t) std::stoull(argv[2]);
    end = (uint64_t) std::stoull(argv[3]);

#ifdef DEV
    std::cout << "Threadcount: " << thread_count << std::endl;
    std::cout << "Calculating sum from " << start << " to " << end  << std::endl;
#endif

    junk_size = ((end - start) / thread_count) + 1;  // +1 to distribute the remainder to all threads

    // Start each thread with a specific range to summate
    pthread_t threads[thread_count];
    uint64_t thread_com[thread_count];
    for (uint64_t j = 0; j < thread_count; ++j) {
        thread_com[j] = j;
        int return_code = pthread_create(&threads[j], NULL, slow_sum, (void *)&thread_com[j]);
        if (return_code != 0) {
#ifdef DEV
            std::cout << "Error while creating thread. Code: " << return_code << std::endl;
#endif
            exit(-1);
        }
    }

    // We assume that the total amount of threads is not big enough for further parallelism. (overhead > summation)
    uint64_t result = 0;
    for (int i = 0; i < thread_count; ++i) {
        void *res;
        pthread_join(threads[i], &res);
        result += thread_com[i];
#ifdef DEV
        std::cout << "Teilergebnis " << i <<": " << (uint64_t )results[i] << std::endl;
#endif
    }

    std::cout << result << std::endl;

#ifdef TIMER
    clock_t finish = clock();
    double elapsed_secs = double(finish - begin) / CLOCKS_PER_SEC;
    std::cout << "Took " << elapsed_secs << " seconds" << std::endl;
#endif

    pthread_exit(nullptr);
}