// This is a more sophisticated reactive server that has better error
// handling, especially wrt not blocking due to flow controlled
// connections.

#include <assert.h>
#include "ace/Acceptor.h"
#include "ace/Svc_Handler.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/SOCK_Stream.h"

// Set the port number here.
static const u_short ECHO_SERVER_PORT = ACE_DEFAULT_SERVER_PORT;

// Amount of time to wait for the client to send data.
static const int TIMEOUT_SECS = 30;

// Set the "high water mark" to 20000k.
static const int HIGH_WATER_MARK = 1024 * 20000;

/**
 * @class Echo_Svc_Handler
 *
 * @brief Echos data received from the client back to the client using
 * the ACE_Reactor's support for nonblocking I/O.
 *
 * This class plays the concrete event handler role in the Reactor
 * pattern and the service handler role in the Acceptor/Connector
 * pattern.
 */
class Echo_Svc_Handler 
    // ACE_SOCK_Stream plays the role of a wrapper facade in the
    // Wrapper Facade pattern.
  : public ACE_Svc_Handler <ACE_SOCK_Stream, ACE_NULL_SYNCH>
{
public:
  // Hook method called by the ACE_Acceptor to activate the service
  // handler.
  virtual int open (void *acceptor)
  {
    // Set the high water mark, which is limits the amount of data
    // that will be used to buffer client input pending succesfully
    // echoing back to the client.
    msg_queue ()->high_water_mark (HIGH_WATER_MARK);

    // Schedule a timeout to guard against clients that connect, but
    // then don't send data.
    if (reactor ()->schedule_timer (this,
                                    0,
                                    ACE_Time_Value (TIMEOUT_SECS)) == -1)
      ACE_ERROR_RETURN ((LM_DEBUG,
                         ACE_TEXT ("(%P|%t) schedule_timer failed\n")),
                        -1);
    // Set the underlying socket handle into non-blocking mode.
    else if (peer ().enable (ACE_NONBLOCK) == -1)
      ACE_ERROR_RETURN ((LM_DEBUG,
                         ACE_TEXT ("(%P|%t) enable failed\n")),
                        -1);
    else
      // Forward to the parent class's open() method, which registers
      // this service handler for reactive dispatch when data arrives.
      return ACE_Svc_Handler <ACE_SOCK_Stream, ACE_NULL_SYNCH>::open (acceptor);
  }

  // This hook method is used to shutdown the service handler if the
  // client doesn't send data for several seconds.
  virtual int handle_timeout (ACE_Time_Value const &, void const*)
  {
    // Instruct the reactor to remove this service handler and shut it
    // down.
    reactor ()->remove_handler (this, ACE_Event_Handler::READ_MASK);
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
            return -1;
          case 0:
            return 0;
          default:
            // Initiate the nonblocking echo back to the client.
            if (put (client_input) == -1)
              ACE_ERROR_RETURN ((LM_ERROR,
                                  ACE_TEXT ("(%P|%t) %p\n"),
                                  ACE_TEXT ("put failed")),
                                 -1);
            else
              return 0;
          }
      }
  }

  // Hook method dispatched by the reactor when flow control abates on
  // the connection to the client.
  virtual int handle_output (ACE_HANDLE /* ignore */)
  {
    // Windows and UNIX select()/sockets are different wrt "output"
    // events, as discussed in the comments for these methods below.
#if defined (ACE_WIN32)
    return handle_windows_output ();
#else /* !defined (ACE_WIN32) */
    return handle_unix_output ();
#endif /* ACE_WIN32 */
  }

  // Receive the next chunk of data from the client without blocking.
  int recv (ACE_Message_Block *&client_input)
  {
    ACE_NEW_RETURN (client_input,
                    ACE_Message_Block (BUFSIZ),
                    -1);

    // Read the data from the client.  We just do a single read so
    // that it doesn't block.
    ssize_t bytes_read = peer ().recv (client_input->wr_ptr (), 
                                       BUFSIZ);
    switch (bytes_read)
      {
      case -1:
        if (errno == EWOULDBLOCK)
          return 0;
        else
          // FALLTHRU
      case 0:
        // Instruct the reactor to shutdown the service handler since
        // the client closed the connection.
        return -1;
      default: 
        // The recv() succeeded, set the write pointer at 1 past the
        // end of the client input.
        client_input->wr_ptr (bytes_read);

        return bytes_read;
      }
  }

  // Echo the input back to the client (may queue if flow control
  // occurs).
  int put (ACE_Message_Block *client_input,
           ACE_Time_Value * /* ignored */ = 0)
  {
    if (msg_queue ()->is_empty ())
      // Try to echo the input back to the client *without* blocking!
      return nonblocking_put (client_input);
    // If we have queued up messages due to flow control then just
    // enqueue and return.
    else if (msg_queue ()->enqueue_tail
             (client_input,
              (ACE_Time_Value *) &ACE_Time_Value::zero) == -1)
      ACE_ERROR_RETURN ((LM_ERROR,
                         ACE_TEXT ("(%P|%t) %p\n"),
                         ACE_TEXT ("enqueue_tail")),
                        -1);
    else
      return 0;
  }

  // Try to echo the input back to the client via a non-blocking send
  // that doesn't block if the connetion is flow controlled.  If we
  // are unable to send the entire input the remainder is re-queued at
  // the *front* of the queue to preserve order.
  ssize_t nonblocking_put (ACE_Message_Block *client_input)
  {
    // If we don't send all the data to the client (e.g., due to flow
    // control), then re-queue the remainder at the head of the
    // message queue and ask the ACE_Reactor to inform us (via
    // handle_output()) when it is possible to try again.
    ssize_t bytes_sent = nonblocking_send (client_input);

    if (bytes_sent == -1)
      // -1 is returned only when things have really gone wrong (i.e.,
      // not when flow control occurs), so we close down.
      ACE_ERROR_RETURN ((LM_ERROR,
                         ACE_TEXT ("(%P|%t) %p\n"),
                         ACE_TEXT ("nonblocking_send")),
                        -1);
    else if (errno == EWOULDBLOCK)
      {
        // We didn't manage to send everything, so we need to plcae
        // the client input in *front* of the queue to preserve order.
        if (msg_queue ()->enqueue_head
            (client_input,
             (ACE_Time_Value *) &ACE_Time_Value::zero) == -1)
          ACE_ERROR_RETURN ((LM_ERROR,
                             ACE_TEXT ("(%P|%t) %p\n"),
                             ACE_TEXT ("enqueue_head")),
                            -1);
        // Tell the ACE_Reactor to call us back when we can send again
        // (i.e., when flow control abates on the connection).
        else if (reactor ()->schedule_wakeup
                 (this,
                  ACE_Event_Handler::WRITE_MASK) == -1)
          ACE_ERROR_RETURN ((LM_ERROR,
                             ACE_TEXT ("(%P|%t) %p\n"),
                             ACE_TEXT ("schedule_wakeup")),
                            -1);
        return 0;
      }
    else
      return bytes_sent;
  }

  ssize_t nonblocking_send (ACE_Message_Block *client_input)
  {
    ssize_t bytes_to_send = client_input->length ();
    ssize_t bytes_sent = peer ().send (client_input->rd_ptr (),
                                       bytes_to_send);
    if (bytes_sent == -1)
      {
        if (errno == EWOULDBLOCK)
          return 0; // Not really an error - just TCP connection flow
                    // control.
        else 
          return -1; // This is a real error.
      }
    else if (bytes_sent < bytes_to_send)
      {
        // Re-adjust read pointer to skip over the data we did send.
        client_input->rd_ptr (bytes_sent);
        errno = EWOULDBLOCK;
      }
    else // if (bytes_sent == bytes_to_send)
      {
        // The whole event is sent, we now decrement the reference
        // count (which deletes itself with it reaches 0).
        client_input->release ();
        errno = 0;
      }
    return bytes_sent;
  }

  // Reschedule the connection timer.
  int reschedule_timer (void)
  {
    // Cancel the existing timer..
    if (reactor ()->cancel_timer (this) == -1)
      ACE_ERROR_RETURN ((LM_DEBUG,
                         ACE_TEXT ("(%P|%t) cancel_timer failed\n")),
                        -1);
    // .. and reschdule it for TIMEOUT_SECS.
    else if (reactor ()->schedule_timer (this,
                                    0,
                                    ACE_Time_Value (TIMEOUT_SECS)) == -1)
      ACE_ERROR_RETURN ((LM_DEBUG,
                         ACE_TEXT ("(%P|%t) schedule_timer failed\n")),
                        -1);
    else
      return 0;
  }

  // UNIX sockets/select triggers multiple "you can write now"
  // signals, so we have to explicitly tell the ACE_Reactor not to
  // notify us anymore (at least until there are new events queued
  // up).
  int handle_unix_output (void)
  {
    ACE_Message_Block *client_input = 0;

    // The list had better not be empty, otherwise there's a bug!
    if (msg_queue ()->dequeue_head
        (client_input,
         (ACE_Time_Value *) &ACE_Time_Value::zero) != -1)
      {
        // Try to send the without blocking.
        switch (nonblocking_put (client_input))
          {
          case 0:           // Partial send.
            assert (errno == EWOULDBLOCK);

            // Didn't write everything this time, come back later...
            return 0;
          case -1:
            // We are responsible for releasing the client input if
            // failures occur.
            client_input->release ();
            ACE_ERROR_RETURN ((LM_ERROR,
                               ACE_TEXT ("(%P|%t) %p\n"),
                               ACE_TEXT ("transmission failure")),
                              -1);
          default: 
            // If we succeeded in writing all the data (or we did not
            // fail due to EWOULDBLOCK) then check if there are more
            // events on the ACE_Message_Queue.  If there aren't, tell
            // the ACE_Reactor not to notify us anymore (at least
            // until there are new events queued up).
            if (msg_queue ()->is_empty ()
                && (reactor ()->cancel_wakeup
                    (this, ACE_Event_Handler::WRITE_MASK) == -1))
              ACE_ERROR_RETURN ((LM_ERROR,
                                 ACE_TEXT ("(%P|%t) %p\n"),
                                 ACE_TEXT ("cancel_wakeup")),
                                -1);
            return 0;
          }
      }
    else
      ACE_ERROR_RETURN ((LM_ERROR,
                         ACE_TEXT ("(%P|%t) %p\n"),
                         ACE_TEXT ("dequeue_head - handle_output called but nothing in message queue")),
                        -1);
  }

  // Unlike UNIX, Windows Winsock doesn't trigger multiple "you can
  // write now" signals, so we have to assume that we can continue to
  // write until we get another EWOULDBLOCK.
  int handle_windows_output (void)
  {
    ACE_Message_Block *client_input = 0;

    // When the receiver blocked, we started adding to the service
    // handler's message queue. At this time, we registered a callback
    // with the reactor to tell us when the TCP layer signalled that
    // we could continue to send messages to the consumer. However,
    // Winsock only sends this notification ONCE, so we have to assume
    // at the application level, that we can continue to send until we
    // get any subsequent blocking signals from the receiver's buffer.

    // We cancel the wakeup callback we set earlier.
    if (reactor ()->cancel_wakeup
        (this, ACE_Event_Handler::WRITE_MASK) == -1)
      ACE_ERROR_RETURN ((LM_ERROR,
                         ACE_TEXT ("(%P|%t) %p\n"),
                         ACE_TEXT ("cancel_wakeup")),
                        -1);

    // The list had better not be empty, otherwise there's a bug!
    while (msg_queue ()->dequeue_head
           (client_input,
            (ACE_Time_Value *) &ACE_Time_Value::zero) != -1)
      {
        switch (nonblocking_put (client_input))
          {
          case -1:                // Error sending message to consumer.
            {
              // We are responsible for releasing an ACE_Message_Block
              // if failures occur.
              client_input->release ();

              ACE_ERROR_RETURN ((LM_ERROR,
                                 ACE_TEXT ("(%P|%t) %p\n"),
                                 ACE_TEXT ("transmission failure")),
                                -1);
            }
          case 0:                 // Partial Send - we got flow controlled by the receiver
            {
              // Re-schedule a wakeup call from the reactor when the
              // flow control conditions abate.
              if (reactor ()->schedule_wakeup
                  (this,
                   ACE_Event_Handler::WRITE_MASK) == -1)
                ACE_ERROR_RETURN ((LM_ERROR,
                                   ACE_TEXT ("(%P|%t) %p\n"),
                                   ACE_TEXT ("schedule_wakeup()")),
                                  -1);

              // Didn't write everything this time, come back later...
              return 0;
            }
          default: 
            // Sent the whole thing.
            return 0;
          }
      }
  }
};

// The ACE_SOCK_Acceptor plays role of the wrapper facade in the
// Wrapper Facade pattern.  The ACE_Acceptor plays role of the
// acceptor in the Acceptor-Connector pattern.

typedef ACE_Acceptor <Echo_Svc_Handler,
                      ACE_SOCK_Acceptor> Echo_Acceptor;

int
main (int argc, char *argv[])
{
  // This object plays the role of the reactor in the Reactor pattern.
  ACE_Reactor reactor;

  // This object play the role of an acceptor in the
  // Acceptor-Connector pattern.
  Echo_Acceptor echo_acceptor;

  // Listen on ECHO_SERVER_PORT and register with the reactor.
  if (echo_acceptor.open (ACE_INET_Addr (ECHO_SERVER_PORT),
                          &reactor) == -1)
    return 1;

  // Run the reactor event loop, which blocks.
  reactor.run_reactor_event_loop ();

  return 0;
}
