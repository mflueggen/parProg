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
#include <algorithm>
#include <string>     // std::string, std::stoull
#include <pthread.h>
#include <vector>

struct range {
  range(unsigned __int128 from, unsigned __int128 to, unsigned __int128 sum)
    : from{from}, to{to}, sum{sum} {}
  unsigned __int128 from;
  unsigned __int128 to;
  unsigned __int128 sum;
};

void* slow_sum(void* args) {
#ifdef LOG
    std::ofstream logfile;
    logfile.open ((*((InfInt *)index)).toString() + ".txt");
    logfile << "Thread " << *((InfInt *)index) << "\n";
#endif
    auto* r = static_cast<range*>(args);
    r->sum = 0;
    for (unsigned __int128 i = r->from; i <= r->to; ++i) {
        r->sum += i;
    }
#ifdef LOG
    logfile << (*start) + *((InfInt *)index)*(*junk_size) << "-" << upper_bound << "\n";
    logfile << "Result: " << result << "\n";
    logfile.close();
#endif
    return nullptr;
}

int main(int argc, char *argv[]) {
#ifdef TIMER
    clock_t begin = clock();
#endif
  const auto thread_count = std::stoull(argv[1]);
  const auto start = std::stoull(argv[2]);
  const auto end = std::stoull(argv[3]);


#ifdef DEV
    std::cout << "Threadcount: " << thread_count << std::endl;
    std::cout << "Calculating sum from " << start << " to " << end  << std::endl;
#endif

    // The smallest junks would be 2 numbers. However, due to the overhead for managing threads
    // compared to the simple operation '+', we assume that it is the best solution to make the
    // chunks as big as possible. The '+1' distributes the remainder to all threads instead of just the last thread.
    const unsigned __int128 chunk_size = ((end - start) / thread_count) + 1;

    // Start each thread with a specific range to summate
    std::vector<pthread_t> threads(thread_count);
    std::vector<range> ranges;
    ranges.reserve(thread_count);


    unsigned __int128 start_of_range = start;
    unsigned __int128 end_of_range = 0;
    for (auto i = 0ul; i < thread_count; ++i) {
      end_of_range = start_of_range + chunk_size;
      if (end_of_range >= end) {
        end_of_range = end;
      }
      ranges.emplace_back(start_of_range, end_of_range, 0);
      start_of_range = end_of_range + 1;
      int return_code = pthread_create(&threads[i], nullptr, slow_sum, &ranges[i]);
      if (return_code != 0) {
#ifdef DEV
          std::cout << "Error while creating thread. Code: " << return_code << std::endl;
#endif
          exit(-1);
      }
    }



    // We assume that the total amount of threads is not big enough for further parallelism. (overhead > summation)
    unsigned __int128 sum = 0;
    for (auto i = 0ul; i < thread_count; ++i) {
      pthread_join(threads[i], nullptr);
      sum += ranges[i].sum;
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



#ifdef TIMER
    clock_t finish = clock();
    double elapsed_secs = double(finish - begin) / CLOCKS_PER_SEC;
    std::cout << "Took " << elapsed_secs << " seconds" << std::endl;
#endif

  return 0;
}