#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>

#include "eventloop.h"
#include "http.h"
#include "debug.h"

#define MAX_EVENTS 10

struct worker_thread_data {
  struct event_loop loop;
  int threadID;
};
  

void setNonBlock(int fd) {
  fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

void* workerThreadMain(void *arg) {
  struct worker_thread_data *td = (struct worker_thread_data*)arg;
  
  for (;;) {
    el_poll(&td->loop);
  }
}


int serverFD = -1;

int initServerSocket(int port) {
  serverFD = socket(AF_INET, SOCK_STREAM, 0);
  
  struct sockaddr_in server_addr = {
    .sin_family = AF_INET,
    .sin_addr = htonl(INADDR_ANY),
    .sin_port = htons(port),
  };
  
  int e;
  e = bind(serverFD, (struct sockaddr*)(&server_addr), sizeof(struct sockaddr_in));
  if (e < 0) {
    q_perror("server", "Failed to bind socket");
    exit(1);
  }

  e = listen(serverFD, 100);
  if (e < 0) {
    q_perror("server", "Failed to listen socket");
    exit(1);
  }
  
}

void c_signal(int signal) {
  q_info("server",  "Recived %s. Closing socket", strsignal(signal));
 
  if (serverFD >= 0)
    close(serverFD);

  serverFD = -1;
  exit(1);
}

int main(int argc, char *args[]) {
  if (argc != 2) {
    printf("Usage: %s <port>\n", args[0]);
    exit(1);
  }

  int port = atoi(args[1]);
  
  initServerSocket(port);
  q_info("server",  "Listening port %d", port);
  
  signal(SIGINT, c_signal);  
  
  int round_robin = 0;
  struct worker_thread_data worker_threads[4];
  for (int i = 0; i < 4; i++) {
    el_init(&worker_threads[i].loop);
    worker_threads[i].threadID = i;

    pthread_t thread;
    pthread_create(&thread, NULL, workerThreadMain, (void*)&(worker_threads[i]));
  }
  
  for (;;) {
    struct sockaddr cliaddr;
    socklen_t cliaddr_len;
    int cliFD = accept(serverFD, NULL, NULL);

    if (cliFD > 0) {
      q_info("server",  "New connection from");
      struct fd_watcher http = http_init();
      struct fd_handle *fd = el_createHandle(&worker_threads[round_robin].loop, cliFD, http);
      round_robin = ++round_robin % 4;
    } else {
      q_perror("server", "Accept failed");
    }
    
  }
  
}
