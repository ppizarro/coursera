#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main (int argc, char * argv [])
{
  int fd;
  struct sockaddr_in srvr;

  memset (&srvr, 0, sizeof(srvr));
  srvr.sin_family      = AF_INET;
  srvr.sin_addr.s_addr = inet_addr ("127.0.0.1");
  srvr.sin_port        = htons (50000);

  fd = socket (AF_INET, SOCK_STREAM, 0);
  assert (fd >= 0);

  if (connect(fd,(struct sockaddr *) &srvr, sizeof(srvr)) < 0) {
    perror("ERROR connecting");
    exit(0);
  }

  int bc;
  char buf[64];
  memset (buf, 0, sizeof(buf));

  write (fd, "uptime\n", 7);
  bc = read (fd, buf, sizeof(buf));

  write (1, buf, bc);

  close (fd);

  exit (0);
}

