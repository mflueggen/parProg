#include <unistd.h>

#include <algorithm>
#include <atomic>
#include <iostream>
#include <vector>
#include <fstream>


struct philosopher {
  uint32_t id;
  uint64_t dinners_eaten;
  // true, if the left fork is clean and can be used
  std::atomic<bool> left_clean;
  // true if the right fork is clean and can be used
  std::atomic<bool> right_clean;
};

// Each fork is modeled as a mutex
std::vector<pthread_mutex_t> _forks;
std::vector<philosopher*> _philosophers;

// global variable to signal when the philosophers should stop eating.
std::atomic<bool> _run;

/**
 * Thread to model a philosopher.
 * A philosopher will wait until the fork to the left and right is clean. Then he will grab (get a lock)
 * both forks as soon as possible.
 * After dinner, he will mark both forks as dirty to himself and mark them as clean for his neighbors.
 * @param args philosopher*
 * @return nullptr
 */
void *philosopher_thread(void *args) {

  auto* phil = static_cast<philosopher*>(args);
  //std::cout << "Starting " << phil->id << std::endl;
  // calculate relevant forks for the current philosopher
  auto* lock_on_left_fork = &_forks[phil->id];
  auto* lock_on_right_fork = &_forks[(phil->id + _philosophers.size() - 1) % _philosophers.size()];

  // calculate relevant neighbors to the philosopher. The neighbors are necessary because the philosopher
  // will inform his two neighbors when he is done eating my marking their forks as clean.
  auto* left_neighbor = _philosophers[(phil->id + 1) % _philosophers.size()];
  auto* right_neighbor = _philosophers[(phil->id + _philosophers.size() - 1) % _philosophers.size()];

  // We use the lefty righty approach to prevent deadlocks
  // This does not circumvent the starvation problem.
  if (phil->id == 0) {
    std::swap(lock_on_left_fork, lock_on_right_fork);
  }


  // endless loop until the main thread sets _run to false.
  while (_run) {

    // both forks need to be clean before the philosopher will grab them.
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

    // after dinner, both forks are dirty.
    // don't use them until both neighbors have marked them as clean again.
    phil->left_clean = false;
    phil->right_clean = false;

    if (pthread_mutex_unlock(lock_on_right_fork)) {
      std::cout << "Phil " << phil->id << ": Could not unlock right fork." << std::endl;
    }
    if (pthread_mutex_unlock(lock_on_left_fork)) {
      std::cout << "Phil " << phil->id << ": Could not unlock left fork." << std::endl;
    }

    // inform neighbors that they can use the forks.
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
    // Memory will be freed as the program terminates ;-)
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

  // Give the philosophers time to eat.
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
