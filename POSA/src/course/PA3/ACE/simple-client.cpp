// This is a simple client that uses the ACE Socket wrapper facades to
// establish a connection with the server and send/recv data.

#include <algorithm>
#include <string>
#include <stdio.h>
#include <assert.h>
#include "ace/OS.h"
#include "ace/SOCK_Connector.h"
#include "ace/Task.h"
#include "ace/Log_Msg.h"

// It's easy to change the type of connector by changing this typedef.
typedef ACE_SOCK_Connector IPC_CONNECTOR;

static const int BUFFER_SIZE = BUFSIZ * 1000;

// Number of iterations.
static const int MAX_ITERATIONS = 50;

// Number of seconds to wait for the results to come back from the server.
static const int TIMEOUT_SECS = 2;

// Establish a connection to the server.
template <typename CONNECTOR> int
connect (CONNECTOR connector,
         typename CONNECTOR::PEER_ADDR addr,
         typename CONNECTOR::PEER_STREAM &peer)
{ 
  return connector.connect (peer, addr);
}

template <typename PEER_STREAM>
class Base_Task : public ACE_Task<ACE_NULL_SYNCH>
{
public:
  Base_Task (PEER_STREAM &peer,
             const std::string &buffer,
             size_t iterations)
    : peer_ (peer),
      buffer_ (buffer),
      iterations_ (iterations)
  {
  }

public:
  // IPC stream to receive data.
  PEER_STREAM &peer_;

  // Buffer that was sent to the server.
  const std::string &buffer_;

  // Number of iterations to run.
  const int iterations_;
};

// Send data to the server.

template <typename PEER_STREAM>
class Send_Task : public Base_Task<PEER_STREAM>
{
public:
  Send_Task (PEER_STREAM &peer,
             const std::string &buffer,
             size_t iterations)
    : Base_Task<PEER_STREAM> (peer, buffer, iterations)
  {}

  virtual int svc (void)
  {
    // Wait up to TIMEOUT_SECS to send the buffer to the client.
    ACE_Time_Value timeout (TIMEOUT_SECS);

    for (size_t i = 0; i < this->iterations_; ++i)
      {
        // Perform a timed send to transmit all the data to the server.
        ssize_t bytes_sent = this->peer_.send_n (this->buffer_.c_str (),
                                                 this->buffer_.length (),
                                                 &timeout);
        if (bytes_sent <= 0) 
          ACE_ERROR_RETURN ((LM_ERROR,
                             "%s\n",
                             "send_n failed"),
                            -1);
      }
    return 0;
  }
};

// Recv data echoed from the server.

template <typename PEER_STREAM>
class Recv_Task : public Base_Task<PEER_STREAM>
{
public:
  Recv_Task (PEER_STREAM &peer,
             const std::string &buffer,
             size_t iterations)
    : Base_Task<PEER_STREAM> (peer, buffer, iterations)
  {}

  virtual int svc (void)
  {
    char buf[BUFFER_SIZE];
      
    size_t bytes_read = 0;

    for (size_t i = 0; i < this->iterations_; ++i)
      {
        // Wait up to TIMEOUT_SECS to receive the buffer from the
        // server.
        ACE_Time_Value timeout (TIMEOUT_SECS);

        // Perform a timed receive to obtain the all data echoed back
        // from the server.
        switch (this->peer_.recv_n (buf,
                                    this->buffer_.length (),
                                    0,
                                    &timeout,
                                    &bytes_read))
          {
          case -1:
            ACE_ERROR_RETURN ((LM_ERROR,
                               "%p\n",
                               "recv_n failed"),
                              -1);
          case 0:
            // FALLTHRU
          default:
            // Make sure that what was sent is what was echoed back.
            assert (bytes_read == this->buffer_.length ());
            assert (std::equal (this->buffer_.begin (), this->buffer_.end (), buf));
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("(%P|%t) server echoed back the data correctly\n")));
          }
      }
  }
};

int 
main (int argc, char *argv[]) 
{ 
  // Parse command-line parameters (if any are given).
  std::string server_hostname = argc > 1 ? argv[1] : "localhost";
  size_t iterations = argc > 2 ? ACE_OS::atoi (argv[2]) : MAX_ITERATIONS;

  IPC_CONNECTOR connector;
  // Note the ues of C++ traits to generalize the type of IPC mechanism used.
  IPC_CONNECTOR::PEER_ADDR addr (ACE_DEFAULT_SERVER_PORT,
                                 server_hostname.c_str());
  IPC_CONNECTOR::PEER_STREAM peer;

  std::string buffer;

  // Initialize the buffer with random data.
  ACE_OS::srand (ACE_OS::time (0));
  std::generate_n (std::back_inserter (buffer),
                   BUFFER_SIZE,
                   rand);

  // Establish the connection and send/recv the data to/from the
  // server.
  if (connect (connector, addr, peer) == -1)
    ACE_ERROR_RETURN ((LM_ERROR,
                       ACE_TEXT ("(%P|%t) connect failed\n")),
                      -1);
  Send_Task<IPC_CONNECTOR::PEER_STREAM> send_task (peer, buffer, iterations);
  Recv_Task<IPC_CONNECTOR::PEER_STREAM> recv_task (peer, buffer, iterations);

  // Launch the tasks as active objects to send/recv data.
  if (send_task.activate () == -1
      || recv_task.activate () == -1)
    ACE_ERROR_RETURN ((LM_ERROR,
                       ACE_TEXT ("(%P|%t) activate failed\n")),
                      -1);

  // Barrier synchronization to wait for both tasks to finish.
  ACE_Thread_Manager::instance ()->wait ();

  return 0;
}
