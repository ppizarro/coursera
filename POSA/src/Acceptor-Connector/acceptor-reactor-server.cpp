#include <ace/Auto_Ptr.h>
#include <ace/Log_Msg.h>

#include <ace/INET_Addr.h>
#include <ace/SOCK_Acceptor.h>
#include <ace/SOCK_Stream.h>

#include <ace/Acceptor.h>
#include <ace/Reactor.h>
#include <ace/Svc_Handler.h>

#include <ace/Null_Condition.h>
#include <ace/Message_Block.h>
#include <ace/Message_Queue.h>

class ClientService : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH>
{
  typedef ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> super;

public:
  ~ClientService()
  {
      printf("Destrutor do ClientService...\n");
  }

  int open (void * = 0);

  // Called when input is available from the client.
  virtual int handle_input (ACE_HANDLE fd = ACE_INVALID_HANDLE);

  // Called when output is possible.
  virtual int handle_output (ACE_HANDLE fd = ACE_INVALID_HANDLE);

  // Called when this handler is removed from the ACE_Reactor.
  virtual int handle_close (ACE_HANDLE handle,
                            ACE_Reactor_Mask close_mask);
};

typedef ACE_Acceptor<ClientService, ACE_SOCK_ACCEPTOR> ClientAcceptor;

#if defined (ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION)
template class ACE_Acceptor<ClientService, ACE_SOCK_ACCEPTOR>;
template class ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH>;
#elif defined (ACE_HAS_TEMPLATE_INSTANTIATION_PRAGMA)
#pragma instantiate ACE_Acceptor<ClientService, ACE_SOCK_ACCEPTOR>
#pragma instantiate \
       ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH>
#endif /* ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION */

int
ClientService::open (void *p)
{
  if (super::open (p) == -1)
    return -1;

  ACE_TCHAR peer_name[MAXHOSTNAMELEN];
  ACE_INET_Addr peer_addr;
  if (this->peer ().get_remote_addr (peer_addr) == 0 &&
      peer_addr.addr_to_string (peer_name, MAXHOSTNAMELEN) == 0)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("(%P|%t) Connection from %s\n"),
                peer_name));
  return 0;
}

int
ClientService::handle_input (ACE_HANDLE)
{
  const size_t INPUT_SIZE = 4096;
  char buffer[INPUT_SIZE];
  ssize_t recv_cnt, send_cnt;

  recv_cnt = this->peer ().recv (buffer, sizeof(buffer));
  if (recv_cnt <= 0) {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("(%P|%t) Connection closed\n")));
      return -1;
  }

  send_cnt = this->peer ().send (buffer, static_cast <size_t>(recv_cnt));
  if (send_cnt == recv_cnt)
    return 0;

  if (send_cnt == -1 && ACE_OS::last_error () != EWOULDBLOCK)
    ACE_ERROR_RETURN ((LM_ERROR,
                       ACE_TEXT ("(%P|%t) %p\n"),
                       ACE_TEXT ("send")),
                      0);

  if (send_cnt == -1)
    send_cnt = 0;

  ACE_Message_Block *mb;
  size_t remaining = static_cast <size_t> (recv_cnt - send_cnt);

  ACE_NEW_RETURN (mb, ACE_Message_Block (&buffer[send_cnt], remaining), -1);

  int output_off = this->msg_queue ()->is_empty ();
  ACE_Time_Value nowait (ACE_OS::gettimeofday ());

  if (this->putq (mb, &nowait) == -1) {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("(%P|%t) %p; discarding data\n"),
                  ACE_TEXT ("enqueue failed")));
      mb->release ();
      return 0;
  }

  if (output_off)
    return this->reactor ()->register_handler (this, ACE_Event_Handler::WRITE_MASK);
  return 0;
}

int
ClientService::handle_output (ACE_HANDLE)
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
      mb->rd_ptr (static_cast <size_t> (send_cnt));
    }
    
    if (mb->length () > 0) {
      this->ungetq (mb);
      break;
    }

    mb->release ();
  }
  return (this->msg_queue ()->is_empty ()) ? -1 : 0;
}

int
ClientService::handle_close (ACE_HANDLE h, ACE_Reactor_Mask mask)
{
  if (mask == ACE_Event_Handler::WRITE_MASK)
    return 0;
  printf("handle_close....\n");
  return super::handle_close (h, mask);
}

class LoopStopper : public ACE_Event_Handler
{
public:
  LoopStopper (int signum = SIGINT);

  // Called when object is signaled by OS.
  virtual int handle_signal (int signum,
                             siginfo_t * = 0,
                             ucontext_t * = 0);
};

LoopStopper::LoopStopper (int signum)
{
  ACE_Reactor::instance ()->register_handler (signum, this);
}

int
LoopStopper::handle_signal (int, siginfo_t *, ucontext_t *)
{
  ACE_Reactor::instance ()->end_reactor_event_loop ();
  return 0;
}

int ACE_TMAIN (int, ACE_TCHAR *[])
{
  ACE_INET_Addr port_to_listen (50000);
  ClientAcceptor acceptor;
  LoopStopper stopper;

  if (acceptor.open (port_to_listen) == -1)
    return 1;

  ACE_Reactor::instance ()->run_reactor_event_loop ();

  return (0);
}
