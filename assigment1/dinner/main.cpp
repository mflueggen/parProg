#include <unistd.h>

#include <iostream>
#include <vector>
#include <fstream>

struct philosopher {

  philosopher(uint64_t dinners_eaten, pthread_mutex_t& lock_on_left_fork, pthread_mutex_t& lock_on_right_fork)
    : dinners_eaten{dinners_eaten}, lock_on_left_fork{lock_on_left_fork}, lock_on_right_fork{lock_on_right_fork} {};

  uint64_t dinners_eaten;
  pthread_mutex_t& lock_on_left_fork;
  pthread_mutex_t& lock_on_right_fork;
};


bool _run;


void *philosopher_thread(void *args) {

  auto* phil = static_cast<philosopher*>(args);

  while (_run) {
    pthread_mutex_lock(&phil->lock_on_left_fork);
    pthread_mutex_lock(&phil->lock_on_right_fork);
    ++phil->dinners_eaten;
    pthread_mutex_unlock(&phil->lock_on_right_fork);
    pthread_mutex_unlock(&phil->lock_on_left_fork);
  }

  return nullptr;
}


int main(int argc, char* argv[]) {
  const auto philosopher_count = std::stoi(argv[1]);
  const auto runtime_in_sec = std::stoi(argv[2]);

  if (philosopher_count < 2) {
    throw  std::runtime_error("At least 2 philosophers are required.");
  }


  std::vector<philosopher> philosophers;
  std::vector<pthread_t> threads(philosopher_count);
  std::vector<pthread_mutex_t> forks(philosopher_count);

  for (auto& mutex : forks) {
    pthread_mutex_init(&mutex, nullptr);
  }

  _run = true;

  philosophers.emplace_back(0, forks[forks.size() - 1], forks[0]);
  pthread_create(&threads[0], nullptr, philosopher_thread, &philosophers[0]);

  for (auto i = 1u; i < philosopher_count; ++i) {
    // left_lock
    // right_lock
    philosophers.emplace_back(0, forks[i - 1], forks[i]);
    pthread_create(&threads[i], nullptr, philosopher_thread, &philosophers[i]);
  }

  sleep(runtime_in_sec);
  _run = false;

  for (auto& thread : threads) {
    pthread_join(thread, nullptr);
  }

  for (auto& fork : forks) {
    pthread_mutex_destroy(&fork);
  }

  std::ofstream output_file ("output.txt");

  for (const auto& phil : philosophers) {
    output_file << phil.dinners_eaten << ';';
  }

  output_file.seekp(-1, std::ios_base::cur);
  output_file << '\n';

  output_file.close();

  return 0;
}