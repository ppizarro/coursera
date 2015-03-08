#include "ace/Reactor.h"
#include "ace/INET_Addr.h"
#include "ace/SOCK_Stream.h"
#include "ace/SOCK_Connector.h"
#include "ace/Connector.h"
#include "ace/Svc_Handler.h"
#include "ace/Reactor_Notification_Strategy.h"

class Client : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH>
{
  typedef ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> super;

public:
  Client () : notifier_ (0, this, ACE_Event_Handler::WRITE_MASK), iterations_(0) {}

  virtual int open (void * = 0);

  // Called when input is available from the client.
  virtual int handle_input (ACE_HANDLE fd = ACE_INVALID_HANDLE);

  // Called when output is possible.
  virtual int handle_output (ACE_HANDLE fd = ACE_INVALID_HANDLE);

  // Called when a timer expires.
  virtual int handle_timeout (const ACE_Time_Value &current_time,
                              const void *act = 0);

private:
  enum { ITERATIONS = 5 };
  int iterations_;
  ACE_Reactor_Notification_Strategy notifier_;
};

int Client::open (void *p)
{
  ACE_Time_Value iter_delay (2);   // Two seconds
  if (super::open (p) == -1)
    return -1;

  this->notifier_.reactor (this->reactor ());

  this->msg_queue ()->notification_strategy (&this->notifier_);

  return this->reactor ()->schedule_timer (this, 0, ACE_Time_Value::zero, iter_delay);
}

int Client::handle_input (ACE_HANDLE)
{
  char buf[64];
  ssize_t recv_cnt = this->peer ().recv (buf, sizeof (buf) - 1);
  if (recv_cnt > 0) {
    ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("%*C"),
                static_cast<int> (recv_cnt),
                buf));
    return 0;
  }

  if (recv_cnt == 0 || ACE_OS::last_error () != EWOULDBLOCK) {
    this->reactor ()->end_reactor_event_loop ();
    return -1;
  }
  return 0;
}

int Client::handle_timeout(const ACE_Time_Value &, const void *)
{
  if (this->iterations_ >= ITERATIONS) {
      this->peer ().close_writer ();
      return 0;
  }

  ACE_Message_Block *mb;
  char msg[128];
  ACE_OS::sprintf (msg, "Iteration %d\n", this->iterations_++);
  ACE_NEW_RETURN (mb, ACE_Message_Block (msg), -1);
  ACE_DEBUG ((LM_DEBUG, ACE_TEXT (msg)));
  this->putq (mb);
  return 0;
}

int Client::handle_output (ACE_HANDLE)
{
  ACE_Message_Block *mb;
  ACE_Time_Value nowait (ACE_OS::gettimeofday ());
  while (-1 != this->getq (mb, &nowait)) {
    ssize_t send_cnt = this->peer ().send (mb->rd_ptr (), mb->length ());
    if (send_cnt == -1) {
        ACE_ERROR ((LM_ERROR,
                    ACE_TEXT ("(%P|%t) %p\n"),
                    ACE_TEXT ("send")));
    } else {
      mb->rd_ptr (static_cast<size_t> (send_cnt));
    }

    if (mb->length () > 0) {
      this->ungetq (mb);
      break;
    }

    mb->release ();
  }

  if (this->msg_queue ()->is_empty ()) {
    this->reactor ()->cancel_wakeup (this, ACE_Event_Handler::WRITE_MASK);
  } else {
    this->reactor ()->schedule_wakeup (this, ACE_Event_Handler::WRITE_MASK);
  }
  return 0;
}

int ACE_TMAIN (int, ACE_TCHAR *[])
{
  Client *pc = 0;
  ACE_INET_Addr port_to_connect (50000, "127.0.0.1");
  ACE_Connector<Client, ACE_SOCK_CONNECTOR> connector;

  if (connector.connect (pc, port_to_connect) == -1)
    ACE_ERROR_RETURN ((LM_ERROR, ACE_TEXT ("%p\n"),
                       ACE_TEXT ("connect")), 1);

  ACE_Reactor::instance ()->run_reactor_event_loop ();
  return (0);
}

