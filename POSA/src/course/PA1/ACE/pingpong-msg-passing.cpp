#include <string>
#include "ace/Task.h"
#include "ace/Message_Block.h"
#include "ace/Log_Msg.h"

const int BUFSIZE = 10;
const int MAX_ITERATIONS = 4;

typedef ACE_Task<ACE_MT_SYNCH> MT_Task;

class PingPong_Task : public MT_Task 
{
public:
  // Initialize whether we're a "ping" or a "pong".
  PingPong_Task (const std::string &to_say)
    : string_to_print_ (to_say),
      neighbor_task_(0) 
  {
  }

  // Inititialize with neighbor task pointer.
  virtual int open (void *neighbor_task) {
    neighbor_task_ = static_cast<MT_Task *>(neighbor_task);
    return this->activate (); // Become an active object.
  }
    
  // Main loop
  virtual int svc (void) 
  {
    for (int loops_done = 0;;)
      {
        ACE_Message_Block *mb = 0;

        // Wait for a message.
        if (getq (mb) == -1)
          break;
        else if (mb->length () == 0)
          break; // Exit on 0 sized message.
        else
          {
            if (++loops_done > MAX_ITERATIONS)
              {
                // Send empty message to finish neighbor task.
                ACE_Message_Block *mb_send = 0;
                ACE_NEW_RETURN (mb_send,
                                ACE_Message_Block,
                                -1);
                neighbor_task_->putq (mb_send); 
                break;
              } 
            else 
              {
                // Print the message.
                ACE_DEBUG ((LM_DEBUG,
                            ACE_TEXT ("(%P|%t) %s! (%d)\n"),
                            string_to_print_.c_str(),
                            loops_done));

                // Send non-empty message to neighbor.
                ACE_Message_Block *mb_send = 0;
                ACE_NEW_RETURN (mg_send,
                                ACE_Message_Block (string_to_print_.size ()),
                                -1);
                mb_send->copy (string_to_print_.c_str (), string_to_print_.size ());
                neighbor_task_->putq (mb_send);
              }
          }
        // Release memory.
        mb->release ();
      }
    return 0;
  }

private:
  // Word to say by a particular process
  std::string string_to_print_; 

  // Whom to pass the baton to (other task pointer).
  MT_Task *neighbor_task_; 
};

int 
main (int argc, char *argv[]) 
{
  // Create tasks.
  std::auto_ptr<MT_Task> ping (new PingPong_Task ("Ping"));
  std::auto_ptr<MT_Task> pong (new PingPong_Task ("Pong"));

  // Init tasks to let them know about each other.
  ping->open (pong.get ());
  pong->open (ping.get ());

  // Run tasks waiting for messages.
  ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("(%P|%t) Ready...Set...Go!\n")));

  // Start ping-pong (need any non-zero message to activate a thread).
  ACE_Message_Block *starter;
  ACE_NEW_RETURN (starter, 
                  ACE_Message_Block (BUFSIZE),
                  -1);
  mb1->copy ("", 1);
  ping->putq (starter);

  // Barrier synchronization to wait for both tasks to finish.
  ACE_Thread_Manager::instance ()->wait ();

  // Time to finish.
  ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("(%P|%t) Done!\n")));

  return 0;
}
