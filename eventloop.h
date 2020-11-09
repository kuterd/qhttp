#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include "chunk.h"
#include "list.h"

struct fd_handle;
typedef void (*fd_callback)(struct fd_handle *handle);
typedef void (*fd_onData)(struct fd_handle *handle, struct chunk ch);
typedef void (*fd_onNewConnection)(struct fd_handle *handle, int clientFD);

struct fd_watcher {
  fd_callback onWritable;
  fd_onData onData;
  fd_callback onClose;
  fd_callback onAttached;
  fd_onNewConnection onNewConnection;
  void *userData;
};

struct fd_handle {
  struct event_loop *el;
  struct fd_watcher watcher;
  
  struct list_head writeList;
  
  int fd;
  int isWritable : 1;
  int isServer : 1;
  int events;
  
};

struct fd_handle* el_createHandle(struct event_loop *el, int fd, struct fd_watcher watcher);

void el_update(struct fd_handle *handle);
void el_write(struct fd_handle *handle, struct chunk ch, fd_callback onWriten);

struct event_loop {
  int fd;
};

void el_init(struct event_loop *el);
void el_poll(struct event_loop *el);

//void el_setWritablityReciver(struct event_loop *el, struct fd_handle *handle, onWritable writable);
#endif
