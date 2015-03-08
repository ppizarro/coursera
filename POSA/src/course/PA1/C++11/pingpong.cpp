// -std=c++0x
#include <string>
#include <iostream>
#include <thread>
#include <mutex>

// Make up for the lack of a std::semaphore in C++11.
class semaphore {
public:
  semaphore (int i): n(i) {}

  // Implement the semaphore "P" operation.
  void acquire () {
    std::unique_lock <std::mutex> lock (m);
    while(n == 0) 
      cond.wait(lock);
    --n;
  }

  // Implement the semaphore "V" operation.
  void release () {
    std::unique_lock <std::mutex> lock (m);
    ++n;
    cond.notify_one();
  }

private:
  std::mutex m;
  std::condition_variable cond;
  int n;
};

// Run the ping/pong algorithm.

void 
play_pingpong (size_t max_iterations,
               const std::string string_to_print,
               semaphore &my_semaphore,
               semaphore &other_semaphore)
{
  for (int loops_done = 1;; ++loops_done)
    {
      my_semaphore.acquire ();
      // Print the message.
      std::cout << string_to_print << "(" << loops_done << ")" << std::endl;
      other_semaphore.release ();
      if (loops_done == max_iterations)
        return;       // Bail out when we're done.
    }
}

// Some useful constants.
static const int num_threads = 2;
static const size_t max_iterations = 4;

int 
main (int argc, char *argv[]) 
{
  std::thread t[num_threads];
  std::cout << "Ready...Set...Go!" << std::endl;

  // Create the ping and pong semaphores that control alternation
  // between threads.
  semaphore ping_semaphore (1); // Starts unlocked
  semaphore pong_semaphore (0); // Starts locked

  // Create the ping and pong threads.
  t[0] = std::thread(play_pingpong,
                     max_iterations,
                     "Ping!",
                     std::ref (ping_semaphore),
                     std::ref (pong_semaphore));

  t[1] = std::thread(play_pingpong,
                     max_iterations,
                     "Pong!",
                     std::ref (pong_semaphore),
                     std::ref (ping_semaphore));

  // Barrier synchronization to join the ping and pong threads with
  // the main thread.
  for (int i = 0; i < num_threads; ++i) 
    t[i].join ();


  std::cout << "Done!" << std::endl;
  return 0;
}
