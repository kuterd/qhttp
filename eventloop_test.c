#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "eventloop.h"
#include "http.h"
void setNonBlock(int fd) {
  fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

int initServerSocket() {
 int serverFD = socket(AF_INET, SOCK_STREAM, 0);
  
  struct sockaddr_in server_addr = {
    .sin_family = AF_INET,
    .sin_addr = htonl(INADDR_ANY),
    .sin_port = htons(1100),
  };
  
  int e = bind(serverFD, (struct sockaddr*)(&server_addr), sizeof(struct sockaddr_in));
  if (e < 0) {
    perror("Failed to bind server socket");
    exit(1);
  }

  e = listen(serverFD, 1400);
  if (e < 0) {
    perror("Failed to listen to server socket");
    exit(1);
  }
  
  return serverFD;
}

void onData(struct fd_handle *handle, struct chunk ch) {
  printf("%d Data: %.*s\n", handle->fd, ch.length, ch.data);
}

void onClose(struct fd_handle *handle) {
  printf("%d Connection closed\n", handle->fd); 
}

int main(int argc, char *args[]) {
  int serverFD = initServerSocket();

  struct event_loop loop;
  el_init(&loop);
  
  for (;;) {
    struct sockaddr cliaddr;
    socklen_t cliaddr_len;
    int cliFD = accept(serverFD, &cliaddr, &cliaddr_len);

    if (cliFD > 0) {
      printf("Connection %d\n", cliFD);
      struct fd_handle *fd = el_createFD(&loop, cliFD, NULL, NULL, NULL);
      http_init(fd);

      for (;;) el_poll(&loop);
    }
    
  }

}
