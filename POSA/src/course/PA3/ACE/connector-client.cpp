// This is a more sophisticated client that uses the connector role in
// the Acceptor-Connector pattern to establish a connection with the
// server and send/recv data.

#include <algorithm>
#include <string>
#include <stdio.h>
#include <assert.h>
#include "ace/OS.h"
#include "ace/SOCK_Connector.h"
#include "ace/Connector.h"
#include "ace/Barrier.h"
#include "ace/Atomic_Op.h"
#include "ace/Log_Msg.h"

// It's easy to change the type of connector by changing this typedef.
typedef ACE_SOCK_Connector IPC_CONNECTOR;

class Echo_Client_Handler 
  : public ACE_Svc_Handler<IPC_CONNECTOR::PEER_STREAM,
                           ACE_MT_SYNCH>
{
public:
  // Size of the buffer to send to the server.
  static const int BUFFER_SIZE = BUFSIZ * 1000;

  // Number of iterations to send the buffer to the server.
  static const int MAX_ITERATIONS = 50;

  // Number of seconds to wait for the results to come back from the server.
  static const int TIMEOUT_SECS = 2;

  Echo_Client_Handler (void)
    : iterations_ (MAX_ITERATIONS),
      sender_ (0),
      barrier_ (2)
  {
  }

  // This method is called back by the ACE_Connector once the
  // connection to the server is established.  It is used to
  // initialize the service handler.
  virtual int open (void *)
  {
    // Initialize the buffer with random data.
    ACE_OS::srand (ACE_OS::time (0));
    std::generate_n (std::back_inserter (buffer_),
                     BUFFER_SIZE,
                     ::rand);

    // Spawn two threads: one to send data to the server and the other
    // to receive data echoed back by the server.
    if (activate (THR_JOINABLE, 2) == -1)
      ACE_ERROR_RETURN ((LM_ERROR,
                         ACE_TEXT ("(%P|%t) activate failed\n")),
                        -1);
  }

  // This hook method is called back by the ACE Task framework each
  // time a thread exits from its svc() hook method.
  virtual int close (u_long /* flags */)
  {
    // Only close down the connection if we're the last remaining
    // thread.
    if (thr_count () == 1)
      ACE_Svc_Handler<IPC_CONNECTOR::PEER_STREAM, ACE_MT_SYNCH>::handle_close ();
  }

  virtual int svc (void)
  {
    // Differentiate between the two threads by using an atomic counter.
    if (++sender_ == 1)
      return do_send ();
    else
      return do_recv ();
  }

  // Send the data to the server.
  int do_send (void)
  {
    // Wait up to TIMEOUT_SECS to send the buffer to the client.
    ACE_Time_Value timeout (TIMEOUT_SECS);

    for (size_t i = 0; i < iterations_; ++i)
      {
        // Perform a timed send to transmit all the data to the server.
        ssize_t bytes_sent = peer_.send_n (buffer_.c_str (),
                                           buffer_.length (),
                                           &timeout);
        if (bytes_sent <= 0) 
          ACE_ERROR_RETURN ((LM_ERROR,
                             ACE_TEXT ("(%P|%t) %s\n"),
                             ACE_TEXT ("send_n failed")),
                            -1);
      }

    return barrier_.wait ();
  }

  // Receive the data echoed back from the server.
  int do_recv (void)
  {
    char buf[BUFFER_SIZE];
      
    size_t bytes_read = 0;

    for (size_t i = 0; i < iterations_; ++i)
      {
        // Wait up to TIMEOUT_SECS to receive the buffer from the
        // server.
        ACE_Time_Value timeout (TIMEOUT_SECS);

        // Perform a timed receive to obtain the all data echoed back
        // from the server.
        switch (peer_.recv_n (buf,
                              buffer_.length (),
                              0,
                              &timeout,
                              &bytes_read))
          {
          case -1:
            ACE_ERROR_RETURN ((LM_ERROR,
                               ACE_TEXT ("(%P|%t) %p\n"),
                               ACE_TEXT ("recv_n failed")),
                              -1);
          case 0:
            // FALLTHRU
          default:
            // Make sure that what was sent is what was echoed back.
            assert (bytes_read == buffer_.length ());
            assert (std::equal (buffer_.begin (), buffer_.end (), buf));
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("(%P|%t) server echoed back the data correctly\n")));
          }
      }
    return barrier_.wait ();
  }

public:
  // Buffer to send to the server.
  std::string buffer_;

  // Number of iterations to run.
  const int iterations_;

  // Used to keep track of whether the svc() method should play the
  // role of a sender or receiver.
  ACE_Atomic_Op<ACE_Thread_Mutex, int> sender_;

  // Used to wait for both sender and receiver threads to finish.
  ACE_Barrier barrier_;
};

// The IPC_CONNECTOR plays role of the wrapper facade in the Wrapper
// Facade pattern.  The ACE_Connector plays role of the connector in
// the Acceptor-Connector pattern.
typedef ACE_Connector <Echo_Client_Handler,
                       IPC_CONNECTOR> Echo_Connector;

int 
main (int argc, char *argv[]) 
{ 
  // Parse command-line parameters (if any are given).
  std::string server_hostname = argc > 1 ? argv[1] : "localhost";

  // Address of the server.
  IPC_CONNECTOR::PEER_ADDR addr (ACE_DEFAULT_SERVER_PORT,
                                 server_hostname.c_str());

  // This object play the role of a connector in the
  // Acceptor-Connector pattern.
  Echo_Connector connector;

  // This service handler is automatically created by the
  // Echo_Connector when the connection is established.
  Echo_Client_Handler *client_handler = 0;

  // Use the Connector to establish the connection with the server and
  // then send/recv the data to/from the server.
  if (connector.connect (client_handler, addr) == -1)
    ACE_ERROR_RETURN ((LM_ERROR,
                       ACE_TEXT ("(%P|%t) connect failed\n")),
                      -1);

  // Barrier synchronization to wait for both threads in the
  // client_handler to finish.
  client_handler->wait ();

  return 0;
}
