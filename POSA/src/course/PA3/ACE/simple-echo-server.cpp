// This is a very simple reactive server that doesn't do anything
// heroic wrt error handling or handling flow controlled connections.
#include "ace/Acceptor.h"
#include "ace/Svc_Handler.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/SOCK_Stream.h"

// Set the port number here
static const u_short ECHO_SERVER_PORT = ACE_DEFAULT_SERVER_PORT;

// This class plays the concrete event handler role in the Reactor
// pattern.
class Echo_Svc_Handler 
    // ACE_SOCK_Stream plays the role of a wrapper facade in the
    // Wrapper Facade pattern.
  : public ACE_Svc_Handler <ACE_SOCK_Stream, ACE_NULL_SYNCH>
{
public:
  // This hook method is dispatched by the ACE Reactor when data shows
  // up from a client.
  int handle_input (ACE_HANDLE /* ignore */)
  {
    char buf[BUFSIZ];
    // Read a chunk of data from the client.
    ssize_t bytes_read = peer ().recv (buf, sizeof buf);

    if (bytes_read <= 0)
      return -1;
    // Send the entire chunk of data back to the client.  Note that
    // this will block if the connection to the client is flow
    // controlled.
    else if (peer ().send_n (buf, bytes_read) <= 0)
      ACE_ERROR_RETURN ((LM_ERROR,
       "(%P|%t) send failed\n"),
        -1);
    else
      return 0;
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
