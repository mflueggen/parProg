#include <unistd.h>
#include <pthread.h>

#include <algorithm>
#include <atomic>
#include <iostream>
#include <vector>
#include <fstream>


struct philosopher {
  uint32_t id;
  uint64_t dinners_eaten;
  std::atomic<bool> left_clean;
  std::atomic<bool> right_clean;
};


std::vector<pthread_mutex_t> _forks;
std::vector<philosopher*> _philosophers;
bool _run;


void *philosopher_thread(void *args) {

  auto* phil = static_cast<philosopher*>(args);
  //std::cout << "Starting " << phil->id << std::endl;
  auto* lock_on_left_fork = &_forks[phil->id];
  auto* lock_on_right_fork = &_forks[(phil->id + _philosophers.size() - 1) % _philosophers.size()];

  auto* left_neighbor = _philosophers[(phil->id + 1) % _philosophers.size()];
  auto* right_neighbor = _philosophers[(phil->id + _philosophers.size() - 1) % _philosophers.size()];

  if (phil->id == 0) {
    std::swap(lock_on_left_fork, lock_on_right_fork);
  }


  while (_run) {

    if (!phil->right_clean || !phil->left_clean) {
      continue;
    }

    if (pthread_mutex_lock(lock_on_left_fork)) {
      std::cout << "Phil " << phil->id << ": Could not lock left fork." << std::endl;
    }
    if (pthread_mutex_lock(lock_on_right_fork)) {
      std::cout << "Phil " << phil->id << ": Could not lock right fork." << std::endl;
    }

    ++phil->dinners_eaten;
    phil->left_clean = false;
    phil->right_clean = false;

    if (pthread_mutex_unlock(lock_on_right_fork)) {
      std::cout << "Phil " << phil->id << ": Could not unlock right fork." << std::endl;
    }
    if (pthread_mutex_unlock(lock_on_left_fork)) {
      std::cout << "Phil " << phil->id << ": Could not unlock left fork." << std::endl;
    }

    left_neighbor->right_clean = true;
    right_neighbor->left_clean = true;
  }

  //std::cout << "Shutting down " << phil->id << std::endl;
  return nullptr;
}


int main(int argc, char* argv[]) {
  const auto philosopher_count = std::stoi(argv[1]);
  const auto runtime_in_sec = std::stoi(argv[2]);

  if (philosopher_count < 2) {
    throw  std::runtime_error("At least 2 philosophers are required.");
  }


  std::vector<pthread_t> threads(philosopher_count);
   _forks.resize(philosopher_count);
   _philosophers.resize(philosopher_count);

  for (auto& fork : _forks) {
    pthread_mutex_init(&fork, nullptr);
  }

  _run = true;

  for (auto i = 0u; i < philosopher_count; ++i) {
    _philosophers[i] = new philosopher();
    _philosophers[i]->id = i;
    _philosophers[i]->dinners_eaten = 0;
    _philosophers[i]->left_clean = true;
    _philosophers[i]->right_clean = true;
  }

  for (auto i = 0u; i < philosopher_count; ++i) {
    if (pthread_create(&threads[i], nullptr, philosopher_thread, _philosophers[i])) {
      std::cout << "pthread_create failed for philosopher " << i << "." << std::endl;
    }
  }

  sleep(runtime_in_sec);
  _run = false;

  for (auto& thread : threads) {
    pthread_join(thread, nullptr);
  }

  for (auto& fork : _forks) {
    pthread_mutex_destroy(&fork);
  }

  std::ofstream output_file ("output.txt");

  for (const auto& phil : _philosophers) {
    output_file << phil->dinners_eaten << ';';
  }

  output_file.seekp(-1, std::ios_base::cur);
  output_file << '\n';

  output_file.close();

  return 0;
}
