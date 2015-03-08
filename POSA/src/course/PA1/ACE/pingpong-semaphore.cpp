// -std=c++0x
#include <string>
#include "ace/Task.h"
#include "ace/Message_Block.h"
#include "ace/Thread_Semaphore.h"
#include "ace/Log_Msg.h"

// This class implements the ping pong algorithm.

class Play_PingPong_Task : public ACE_Task<ACE_NULL_SYNCH>
{
public:
  Play_PingPong_Task (size_t max_iterations,
                      const std::string string_to_print,
                      ACE_Thread_Semaphore &my_semaphore,
                      ACE_Thread_Semaphore &other_semaphore)
    : max_iterations_ (max_iterations),
      string_to_print_ (string_to_print),
      my_semaphore_ (my_semaphore),
      other_semaphore_ (other_semaphore)
  {
  }

  // Main event loop that runs in a separate thread of control.
  virtual int svc (void) 
  {
    for (int loops_done = 1;; ++loops_done)
      {
        my_semaphore_.acquire ();
        // Print the message.
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("(%P|%t) %s! (%d)\n"),
                    string_to_print_.c_str(),
                    loops_done));
        other_semaphore_.release ();
        
        if (loops_done == max_iterations_)
          return true; // Bail out when we're done.
      }
  }

protected:
  // Number of iterations to run.
  size_t max_iterations_;

  // Word to say by a particular process.
  std::string string_to_print_; 

  // Synchronizers to ensure the threads alternate between pings and
  // pongs.
  ACE_Thread_Semaphore &my_semaphore_;
  ACE_Thread_Semaphore &other_semaphore_;
};

int 
main (int argc, char *argv[]) 
{
  const size_t MAX_ITERATIONS = 4;

  // Create the ping and pong semaphores that control alternation
  // between threads.
  ACE_Thread_Semaphore ping_semaphore (1); // Starts unlocked
  ACE_Thread_Semaphore pong_semaphore (0); // Starts locked

  // Create the ping and pong tasks.
  std::auto_ptr<Play_PingPong_Task> ping (new Play_PingPong_Task (MAX_ITERATIONS,
                                                                  "Ping!",
                                                                  ping_semaphore,
                                                                  pong_semaphore));
  std::auto_ptr<Play_PingPong_Task> pong (new Play_PingPong_Task (MAX_ITERATIONS,
                                                                  "Pong!",
                                                                  pong_semaphore,
                                                                  ping_semaphore));

  ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("(%P|%t) Ready...Set...Go!\n")));

  // Make the ping and pong tasks run as "active objects," which will
  // call their svc() hook method.
  ping->activate ();
  pong->activate ();

  // Barrier synchronization to wait for both tasks to finish.
  ACE_Thread_Manager::instance ()->wait ();

  ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("(%P|%t) Done!\n")));
  return 0;
}
