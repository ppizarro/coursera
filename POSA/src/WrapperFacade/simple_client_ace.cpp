#include <ace/INET_Addr.h>
#include <ace/SOCK_Stream.h>
#include <ace/SOCK_Connector.h>
#include <ace/Log_Msg.h>

int ACE_TMAIN (int, ACE_TCHAR *[])
{
  ACE_INET_Addr srvr (50000, "127.0.0.1");

  ACE_SOCK_Connector connector;
  ACE_SOCK_Stream peer;

  if (-1 == connector.connect (peer, srvr))
    ACE_ERROR_RETURN ((LM_ERROR,
                       ACE_TEXT ("%p\n"),
                       ACE_TEXT ("connect")), 1);

  int bc;
  char buf[64];
  int tentativas = 10;

  while(tentativas--) {
    peer.send_n ("uptime\n", 7);
    bc = peer.recv (buf, sizeof(buf));
    write (1, buf, bc);
    usleep(1000000);
  }
  peer.close ();

  return (0);
}
