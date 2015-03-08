// -std=c++0x
#include <string>
#include "ace/Task.h"
#include "ace/Message_Block.h"
#include "ace/Thread_Semaphore.h"
#include "ace/Log_Msg.h"

// This class plays the role of the "Abstract Class" in the Template
// Method pattern.

class Base_Task : public ACE_Task<ACE_NULL_SYNCH>
{
public:
  Base_Task (size_t max_iterations,
             const std::string string_to_print,
             ACE_Thread_Semaphore &ping,
             ACE_Thread_Semaphore &pong)
    : max_iterations_ (max_iterations),
      string_to_print_ (string_to_print),
      ping_semaphore_ (ping),
      pong_semaphore_ (pong)
  {
  }

  // Virtual hook methods that determine the ping/pong protocol.
  virtual void acquire (void) = 0;
  virtual void release (void) = 0;

  // Main event loop that runs in a separate thread of control.  Note
  // the use of the Template Method pattern's "template method" here.
  virtual int svc (void) 
  {
    for (int loops_done = 1;; ++loops_done)
      {
        acquire ();
        // Print the message.
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("(%P|%t) %s! (%d)\n"),
                    string_to_print_.c_str(),
                    loops_done));
        release ();
        
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
  ACE_Thread_Semaphore &ping_semaphore_;
  ACE_Thread_Semaphore &pong_semaphore_;
};

// Class that uses semaphores to implement the "ping" function.  This
// plays the role of the "Concrete Class" in the Template Method
// pattern.

class Ping_Task : public Base_Task
{
public:
  Ping_Task (size_t max_iterations, 
             ACE_Thread_Semaphore &ping,
             ACE_Thread_Semaphore &pong)
    : Base_Task (max_iterations, "Ping!", ping, pong)
  {
  }

  // Hook method for "ping" acquire.
  virtual void acquire (void)
  {
    ping_semaphore_.acquire ();
  }

  // Hook method for "ping" release.
  virtual void release (void)
  {
    pong_semaphore_.release ();
  }
  

};

// Class that uses semaphores to implement the "pong" function.  This
// plays the role of the "Concrete Class" in the Template Method
// pattern.

class Pong_Task : public Base_Task
{
public:
  Pong_Task (size_t max_iterations, 
             ACE_Thread_Semaphore &ping,
             ACE_Thread_Semaphore &pong)
    : Base_Task (max_iterations, "Pong!", ping, pong)
  {
  }

  // Hook method for "pong" acquire.
  virtual void acquire (void)
  {
    pong_semaphore_.acquire ();
  }

  // Hook method for "pong" release.
  virtual void release (void)
  {
    ping_semaphore_.release ();
  }
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
  std::auto_ptr<Ping_Task> ping (new Ping_Task (MAX_ITERATIONS, ping_semaphore, pong_semaphore));
  std::auto_ptr<Pong_Task> pong (new Pong_Task (MAX_ITERATIONS, ping_semaphore, pong_semaphore));

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
