#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg)
{
  perror(msg);
  exit(1);
}

int main(int argc, char *argv[])
{
  int sockfd, newsockfd;
  socklen_t clilen;
  char buffer[256];
  struct sockaddr_in serv_addr, cli_addr;
  int n;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    error("ERROR opening socket");
  }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(50000);

  if (bind(sockfd, (struct sockaddr *) &serv_addr,
           sizeof(serv_addr)) < 0) {
    error("ERROR on binding");
  }

  while(1) {

    listen(sockfd, 5);

    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, 
                       (struct sockaddr *) &cli_addr, 
                        &clilen);
    if (newsockfd < 0) {
      error("ERROR on accept");
    }

    bzero(buffer,256);
    while ((n = read(newsockfd, buffer, sizeof(buffer))) != -1) {
      if (n == 0) break;
      write(newsockfd, buffer, n);
    }

    close(newsockfd);
  }

  close(sockfd);

  return 0; 
}
