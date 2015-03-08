#include "ace/INET_Addr.h"
#include "ace/SOCK_Stream.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/Log_Msg.h"

int ACE_TMAIN (int, ACE_TCHAR *[])
{
    ACE_INET_Addr port_to_listen (50000);
    ACE_SOCK_Acceptor acceptor;

    if (acceptor.open (port_to_listen, 1) == -1)
      ACE_ERROR_RETURN ((LM_ERROR,
                         ACE_TEXT ("%p\n"),
                         ACE_TEXT ("acceptor.open")),
                        100);

    while (1) {
        char buffer[4096];
        ssize_t bytes_received;
        ACE_SOCK_Stream peer;

        if (acceptor.accept (peer) == -1)
            ACE_ERROR_RETURN ((LM_ERROR,
                               ACE_TEXT ("(%P|%t) Failed to accept ")
                               ACE_TEXT ("client connection\n")),
                               100);

        while ((bytes_received = peer.recv (buffer, sizeof(buffer))) != -1) {
            if (bytes_received == 0) break;
            write(1, buffer, bytes_received);
            peer.send_n (buffer, bytes_received);
        }

        peer.close ();
    }

    return (0);
}

