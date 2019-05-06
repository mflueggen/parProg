//#define DEV
//#define TIMER
//#define LOG

#ifdef TIMER
#include <ctime>
#endif

#ifdef LOG
#include <fstream>
#endif

#include "InfInt.h"
#include <iostream>
#include <string>     // std::string, std::stoull
#include <pthread.h>

InfInt *junk_size;
InfInt *thread_count;
InfInt *start;
InfInt *end;

void *slow_sum(void *index) {
#ifdef LOG
    std::ofstream logfile;
    logfile.open ((*((InfInt *)index)).toString() + ".txt");
    logfile << "Thread " << *((InfInt *)index) << "\n";
#endif
    InfInt result = 0;
    InfInt upper_bound = (*start) + *((InfInt *)index)*(*junk_size) + (*junk_size) - 1;
    if (upper_bound > (*end)) {
        upper_bound = (*end);
    }
    for (InfInt i = (*start) + *((InfInt *)index)*(*junk_size); i <= upper_bound; ++i) {
        result += i;
    }
#ifdef LOG
    logfile << (*start) + *((InfInt *)index)*(*junk_size) << "-" << upper_bound << "\n";
    logfile << "Result: " << result << "\n";
    logfile.close();
#endif
    *((InfInt *)index) = result;
    pthread_exit(nullptr);
    return nullptr;
}

int main(int argc, char *argv[]) {
#ifdef TIMER
    clock_t begin = clock();
#endif
    InfInt local_thread_count = (InfInt) std::stoull(argv[1]);
    thread_count = &local_thread_count;
    InfInt local_start = (InfInt) std::stoull(argv[2]);
    start = &local_start;
    InfInt local_end = (InfInt) std::stoull(argv[3]);
    end = &local_end;

#ifdef DEV
    std::cout << "Threadcount: " << thread_count << std::endl;
    std::cout << "Calculating sum from " << start << " to " << end  << std::endl;
#endif

    InfInt local_junk_size = (((*end) - (*start)) / (*thread_count).toInt()) + 1;  // +1 to distribute the remainder to all threads
    junk_size = &local_junk_size;

    // Start each thread with a specific range to summate
    pthread_t threads[(*thread_count).toInt()];
    InfInt thread_com[(*thread_count).toInt()];
    for (uint64_t j = 0; j < (*thread_count).toInt(); ++j) {
        thread_com[j] = j;
        int return_code = pthread_create(&threads[j], nullptr, slow_sum, (void *)&thread_com[j]);
        if (return_code != 0) {
#ifdef DEV
            std::cout << "Error while creating thread. Code: " << return_code << std::endl;
#endif
            exit(-1);
        }
    }

    // We assume that the total amount of threads is not big enough for further parallelism. (overhead > summation)
    InfInt result = 0;
    for (int i = 0; i < (*thread_count).toInt(); ++i) {
        void *res;
        pthread_join(threads[i], &res);
        result += thread_com[i];
#ifdef DEV
        std::cout << "Teilergebnis " << i <<": " << (InfInt )results[i] << std::endl;
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