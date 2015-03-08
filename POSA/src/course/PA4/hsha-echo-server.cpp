// This server implements an echo server based on the
// Half-Sync/Half-Async pattern and the associated ACE frameworks
#include <assert.h>
#include "ace/Acceptor.h"
#include "ace/Svc_Handler.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/SOCK_Stream.h"

// Make it easier to do wholesale replacement of the IPC mechanisms
// via C++ traits.
typedef ACE_SOCK_Acceptor IPC_ACCEPTOR;

// Set the port number here.
static const u_short ECHO_SERVER_PORT = 50000; //ACE_DEFAULT_SERVER_PORT;

// Amount of time to wait for the client to send data.
static const int TIMEOUT_SECS = 30;

// Stores a string version of the current thread id into buffer and
// returns the size of this thread id in bytes.  This method has been
// added to ACE as of version 6.1.9.
static ssize_t
thr_id (char buffer[], size_t buffer_length)
{
#if defined (ACE_WIN32)
#if defined (ACE_HAS_SNPRINTF)
  return ACE_OS::snprintf (buffer,
                           buffer_length,
                           "%u",
                           static_cast <unsigned> (ACE_OS::thr_self ()));
#else
  return ACE_OS::sprintf (buffer,
                          "%u",
                          static_cast <unsigned> (ACE_OS::thr_self ()));
#endif /* ACE_HAS_SNPRINTF */
#else
  ACE_hthread_t t_id;
  ACE_OS::thr_self (t_id);
#if defined (ACE_HAS_SNPRINTF)
  return ACE_OS::snprintf (buffer,
                           buffer_length,
                           "%lu",
                           (unsigned long) t_id);
#else
  return ACE_OS::sprintf (buffer,
                          "%lu",
                          (unsigned long) t_id);
#endif /* ACE_HAS_SNPRINTF */
#endif /* WIN32 */
}

/**
 * @class Echo_Command
 *
 * @brief This class is used by the Asynchronous Layer
 * (Echo_Svc_Handler) to enqueue a Command in the Queueing Layer
 * (ACE_Message_Queue) that is then executed by a thread in the
 * Synchronous Layer (Echo_Task).
 *
 * This class implements the Command pattern.
 */
template <typename PEER_STREAM>
class Echo_Command : public ACE_Message_Block
{
public:
  Echo_Command (ACE_Svc_Handler <PEER_STREAM, ACE_NULL_SYNCH> *svc_handler,
                ACE_Message_Block *client_input)
    : svc_handler_ (svc_handler)
  {
    // Attach the client_input on the message continuation chain.
    this->cont (client_input);
  }

  // Accessor for the service handler pointer.
  ACE_Svc_Handler<PEER_STREAM, ACE_NULL_SYNCH> *svc_handler (void)
  {
    return svc_handler_;
  }

  // When an Echo_Command is executed it echos the data back to the
  // client after first prepending the thread id.
  virtual int execute (void)
  {
    // The client_input is attached via the message continuation
    // chain.
    ACE_Message_Block *client_input = this->cont ();
    char buffer[BUFSIZ];
    // If you aren't using ACE 6.1.9 or later please replace
    // ACE_OS::thr_id() with a call to the static thr_id() method
    // above.
    ssize_t len = thr_id (buffer, sizeof buffer - 1);

    // Terminate the buffer with a newline since that's what's
    // expected by the client.
    buffer[len++] = '\n';

    // We use an iovec to optimize the domain-crossing overhead.
    iovec iov[2] = 
      {
        { buffer, len },
        { client_input->rd_ptr (), client_input->length () }
      };

    // Efficiently echo the data back to the client.
    if (svc_handler_->peer ().sendv_n (iov, 2) == -1)
      ACE_ERROR_RETURN ((LM_ERROR,
                         ACE_TEXT ("(%P|%t) %p\n"),
                         ACE_TEXT ("sendv_n failed")),
                        -1);
    else
      return 0;
  }

private:
  // Pointer to our service handler.
  ACE_Svc_Handler <PEER_STREAM, ACE_NULL_SYNCH> *svc_handler_;
};

/**
 * @class Echo_Task
 *
 * @brief Contains a thread pool that dequeues client input from a
 * synchronized message queue and then echos the data received from
 * the client back to the client using blocking I/O.
 *
 * This class plays the Synchronous Layer and Queueing Layer role in
 * the Half-Sync/Half-Async pattern pattern.
 */
template <typename PEER_STREAM>
class Echo_Task
  : public ACE_Task <ACE_MT_SYNCH>
{
public:
  // Size of the thread pool used by the Synchronous Layer.
  static const int THREAD_POOL_SIZE = 4;

  // Set the "high water mark" to 20000k.
  static const int HIGH_WATER_MARK = 1024 * 20000;

  // Default constructor sets the HIGH_WATER_MARK of the underlying
  // ACE_Message_Queue.
  virtual int open (void * = 0)
  {
    // Set the high water mark, which is limits the amount of data
    // that will be used to buffer client input pending succesfully
    // echoing back to the client.
    msg_queue ()->high_water_mark (HIGH_WATER_MARK);

    // Activate the task to run a thread pool.
    if (activate (THR_DETACHED,
                  THREAD_POOL_SIZE) == -1)
      ACE_ERROR_RETURN ((LM_ERROR,
                         ACE_TEXT ("(%P|%t) activate failed\n")),
                        -1);
    return 0;
  }

  // This hook method runs in each thread of control in the thread pool.
  virtual int svc (void) 
  {
    // Block until there's a message available on the queue.
    for (ACE_Message_Block *message_block;
         getq (message_block) != -1; 
         )
      {
        if (Echo_Command<PEER_STREAM> *echo_command =
            dynamic_cast<Echo_Command<PEER_STREAM> *>(message_block))
          {
            // Execute the echo_command to echo the data back to the
            // client.
            echo_command->execute ();

            // Resuem the service handler so that it can receive more
            // client input.
            echo_command->svc_handler ()->resume_handler ();

            // Deallocate the dynamically allocated memory.
            echo_command->release ();
          }
      }
    return 0;
  }

  // Enqueue the client_input in the ACE_Message_Queue.
  virtual int put (ACE_Message_Block *client_input,
                   ACE_Time_Value *timeout = 0)
  {
    return putq (client_input, timeout);
  }
};

/**
 * @class Echo_Svc_Handler
 *
 * @brief Echos data received from the client back to the client using
 * nonblocking I/O.
 *
 * This class plays the concrete event handler role in the Reactor
 * pattern and the service handler role in the Acceptor/Connector
 * pattern.
 */
template <typename PEER_STREAM>
class Echo_Svc_Handler 
    // ACE_SOCK_Stream plays the role of a wrapper facade in the
    // Wrapper Facade pattern.
  : public ACE_Svc_Handler <PEER_STREAM, ACE_NULL_SYNCH>
{
public:
  // This constructor takes a default parameter of 0 so that it will
  // work properly with the make_svc_handler() method in the
  // Echo_Acceptor.
  Echo_Svc_Handler (ACE_Thread_Manager* thr_mgr = 0,
                    ACE_Reactor *reactor = 0,
                    Echo_Task<PEER_STREAM> *echo_task = 0)
    : ACE_Svc_Handler <PEER_STREAM, ACE_NULL_SYNCH> (thr_mgr, 0, reactor),
      echo_task_ (echo_task),
      client_input_ (0)
  {
  }

  // Hook method called by the ACE_Acceptor to activate the service
  // handler.
  virtual int open (void *acceptor)
  {
    // Schedule a timeout to guard against clients that connect by
    // don't send data.
    if (this->reactor ()->schedule_timer (this,
                                          0,
                                          ACE_Time_Value (TIMEOUT_SECS)) == -1)
      ACE_ERROR_RETURN ((LM_ERROR,
                         ACE_TEXT ("(%P|%t) schedule_timer failed\n")),
                        -1);
    else
      // Forward to the parent class's open() method, which registers
      // this service handler for reactive dispatch when data arrives.
      return ACE_Svc_Handler <ACE_SOCK_Stream, ACE_NULL_SYNCH>::open (acceptor);
  }

  // Close down the event handler.
  virtual int handle_close (ACE_HANDLE h,
                            ACE_Reactor_Mask m)
  {
    return ACE_Svc_Handler <ACE_SOCK_Stream, ACE_NULL_SYNCH>::handle_close (h, m);
  }

  // This hook method is used to shutdown the service handler if the
  // client doesn't send data for several seconds.
  virtual int handle_timeout (ACE_Time_Value const &, void const*)
  {
    // Instruct the reactor to remove this service handler and shut it
    // down.
    this->reactor ()->remove_handler (this, ACE_Event_Handler::READ_MASK);
    return 0;
  }

  // This hook method is dispatched by the ACE Reactor when data shows
  // up from a client.
  virtual int handle_input (ACE_HANDLE /* ignore */)
  {
    ACE_Message_Block *client_input = 0;

    // Reschedule the connection timer.
    if (reschedule_timer () == -1)
      ACE_ERROR_RETURN ((LM_ERROR,
                         ACE_TEXT ("(%P|%t) %p\n"),
                         ACE_TEXT ("reschedule_timer failed")),
                        -1);
    else
      {
        // Receive data from the client.
        switch (recv (client_input))
          {
          case -1:
          case 0:
            return -1;
          default:
            if (errno != EWOULDBLOCK)
              {
                // Create a command that will echo the input back the
                // client.
                Echo_Command<ACE_SOCK_Stream> *echo_command = 0;
                ACE_NEW_RETURN (echo_command,
                                Echo_Command<ACE_SOCK_Stream> (this,
                                                               client_input),
                                -1);

                // Don't receive any more input on this connection
                // until we've echoed the data back to the client.
                suspend_handler ();

                // Use the put() hook method to enqueue the
                // echo_command in the Queueing layer.
                if (echo_task_->put (echo_command) == -1)
                  {
                    ACE_ERROR ((LM_ERROR,
                                ACE_TEXT ("(%P|%t) %p\n"),
                                ACE_TEXT ("put failed")));
                    echo_command->release ();
                    return -1;
                  }
              }
            return 0;
          }
      }
  }

  // Receive the next chunk of data from the client.
  int recv (ACE_Message_Block *&client_input)
  {
    if (client_input_ == 0)
      ACE_NEW_RETURN (client_input_,
                      ACE_Message_Block (BUFSIZ),
                      -1);

    // Read the data from the client.  We just do a single read so
    // that it doesn't block.
    ssize_t bytes_read = this->peer ().recv (client_input_->wr_ptr (), 
                                             BUFSIZ);
    switch (bytes_read)
      {
      case -1:
        // FALLTHRU
      case 0:
        // Instruct the reactor to shutdown the service handler.
        return -1;

      default: 
        // The recv() succeeded, set the write pointer at 1 past the
        // end of the client input.
        client_input_->wr_ptr (bytes_read);

        if (contains_endofline (client_input))
          {
            // You fill in here.
          }
        else
          errno = EWOULDBLOCK;

        return bytes_read;  
      }
  }

  // Reschedule the connection timer.
  int reschedule_timer (void)
  {
    // Cancel the existing timer..
    if (this->reactor ()->cancel_timer (this) == -1)
      ACE_ERROR_RETURN ((LM_ERROR,
                         ACE_TEXT ("(%P|%t) cancel_timer failed\n")),
                        -1);
    // .. and reschdule it for TIMEOUT_SECS.
    else if (this->reactor ()->schedule_timer (this,
                                               0,
                                               ACE_Time_Value (TIMEOUT_SECS)) == -1)
      ACE_ERROR_RETURN ((LM_ERROR,
                         ACE_TEXT ("(%P|%t) schedule_timer failed\n")),
                        -1);
    else
      return 0;
  }

  // Suspend the event handler.
  int suspend_handler (void)
  {
    return this->reactor ()->suspend_handler (this);
  }

  // Resume the event handler.
  int resume_handler (void)
  {
    return this->reactor ()->resume_handler (this);
  }

  // Checks to see if the next chunk of data received from the client
  // contains the end-of-line marker.
  bool contains_endofline (ACE_Message_Block *client_input)
  {
    // You fill in here...
    return true;
  }

private:
  // Pointer to the thread pool that plays the role of the synchronous
  // layer in the Half-Sync/Half-Async pattern.
  Echo_Task<PEER_STREAM> *echo_task_;

  // Contains the message fragment(s) received from the connected client.
  ACE_Message_Block *client_input_;
};

/**
 * @class Echo_Svc_Handler
 *
 * @brief This class is used by the ACE_Strategy_Acceptor to pass the
 * Echo_Task pointer to the service handler constructor so there's no
 * need to subclass ACE_Acceptor.
 *
 * This class is an example of the Strategy pattern.
 */
template <typename TASK, typename SVC_HANDLER>
class Echo_Svc_Handler_Creation_Strategy
  : public ACE_Creation_Strategy<SVC_HANDLER> 
{
public:
  Echo_Svc_Handler_Creation_Strategy (ACE_Reactor *reactor, TASK *task)
    : ACE_Creation_Strategy<SVC_HANDLER> (0, reactor),
      task_ (task)
  { 
  }

  // A factory method that creates a SVC_HANDLER that holds a pointer
  // to the TASK, which contains the thread pool used by the
  // Synchronous Layer in the Half-Sync/Half-Async pattern.
  virtual int make_svc_handler (SVC_HANDLER *&sh) 
  {
    ACE_NEW_RETURN (sh,
                    SVC_HANDLER (0, this->reactor_, task_),
                    -1);
    return 0;
  }

private:
  // Pointer to the thread pool used by the Synchronous Layer in the
  // Half-Sync/Half-Async pattern.
  TASK *task_;
};

// Defines the thread pool used by the Synchronous Layer in the
// Half-Sync/Half-Async pattern.
typedef Echo_Task<IPC_ACCEPTOR::PEER_STREAM> ECHO_TASK;

// Service handler used to pass complete messages containing client
// input that can consist of multiple recv() calls to the ECHO_TASK
// via its synchronized message queue, which then processes the input
// in its thread pool and echoes the results back to the client.
typedef Echo_Svc_Handler<IPC_ACCEPTOR::PEER_STREAM> ECHO_SVC_HANDLER;

// Configure the ACE_Strategy_Acceptor factory that accepts
// connections and creates/activates ECHO_SVC_HANDLERS to process data
// on the connections.  It can is passed a handler creation strategy
// from the ECHO_SVC_HANDLER parameter. It is also configured to use
// ACE socket wrapper facade.
// 
// This class plays the role of the concrete acceptor role in the
// Acceptor/Connector pattern.  The ACE_SOCK_Acceptor plays role of
// the wrapper facade in the Wrapper Facade pattern.
typedef ACE_Strategy_Acceptor<ECHO_SVC_HANDLER, IPC_ACCEPTOR> ECHO_ACCEPTOR;

// A ECHO_SVC_HANDLER creation strategy passed to the ECHO_ACCEPTOR.
// It is used pass the ECHO_TASK pointer to the ECHO_SVC_HANDLER
// instances created by the ECHO_ACCEPTOR.
typedef Echo_Svc_Handler_Creation_Strategy<ECHO_TASK,
                                           ECHO_SVC_HANDLER> 
                                           ECHO_SVC_HANDLER_CREATION_STRATEGY;

int
main (int argc, char *argv[])
{
  // This object plays the role of the reactor in the Reactor pattern.
  ACE_Reactor reactor;

  // The thread pool that plays the role of the synchronous layer in
  // the Half-Sync/Half-Async pattern.
  ECHO_TASK echo_task;

  // Activate the thread pool.
  if (echo_task.open () == -1)
    ACE_ERROR_RETURN ((LM_ERROR,
                       ACE_TEXT ("(%P|%t) %p\n"),
                       ACE_TEXT ("task open failed")),
                      1);

  // This object play the role of an acceptor in the
  // Acceptor-Connector pattern.
  ECHO_ACCEPTOR echo_acceptor;

  // Instantiate a creation strategy that's passed to the
  // echo_acceptor.
  ECHO_SVC_HANDLER_CREATION_STRATEGY creation_strategy (&reactor, 
                                                        &echo_task);

  // Listen on ECHO_SERVER_PORT and register the echo_acceptor with
  // the reactor.
  if (echo_acceptor.open (ACE_INET_Addr (ECHO_SERVER_PORT),
                          &reactor,
                          &creation_strategy) == -1)
    ACE_ERROR_RETURN ((LM_ERROR,
                       ACE_TEXT ("(%P|%t) %p\n"),
                       ACE_TEXT ("acceptor open failed")),
                      1);

  // Run the reactor event loop, which blocks.
  reactor.run_reactor_event_loop ();

  return 0;
}
