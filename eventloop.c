#include <sys/epoll.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include "eventloop.h"
#include "debug.h"

void el_init(struct event_loop *el) {
  el->fd = epoll_create1(0);
}

struct chunk_to_write {
  unsigned int writen;
  fd_callback onWriten;
  struct chunk chunk;
  
  struct list_head list;
};


//TODO: better handle write result <= 0
void _el_tryWrite(struct fd_handle *handle, unsigned int *writen, struct chunk ch) {
  do {
    int wr = write(handle->fd, ch.data + *writen, ch.length - *writen);
    q_info("eventloop", "Handle %d writen %d bytes\n", handle->fd, wr);
    if (wr > 0) *writen += wr;
  } while (*writen < ch.length && (errno == EAGAIN || errno != EWOULDBLOCK));
  handle->isWritable = (errno == EAGAIN || errno != EWOULDBLOCK);
}

void _el_update(struct fd_handle *handle, int create) {
  struct epoll_event event;
  event.events = EPOLLET | EPOLLRDHUP | EPOLLHUP | EPOLLERR;
  event.data.ptr = handle;
  
  if (handle->watcher.onData)
    event.events |= EPOLLIN;
  if (handle->watcher.onWritable || handle->writeList.next != &handle->writeList)
    event.events |= EPOLLOUT;

  if (event.events != handle->events) {
    epoll_ctl(handle->el->fd, create ? EPOLL_CTL_ADD : EPOLL_CTL_MOD, handle->fd, &event);  
    handle->events = event.events;
  }
}

void el_update(struct fd_handle *handle) {
  _el_update(handle, 0);  
}

void _handle_close(struct fd_handle *handle ) {
  if (handle->watcher.onClose)
    handle->watcher.onClose(handle);

  for (struct list_head *c = handle->writeList.next, *cc = c->next;
       c != &handle->writeList; c = cc, cc = c->next) {
    struct chunk_to_write *node = containerof(c, struct chunk_to_write, list);
    free(node->chunk.data);
    free(c);
  }
  
  free(handle);
}

void _handle_write(struct fd_handle *handle) {
  if (handle->watcher.onWritable)
    handle->watcher.onWritable(handle);
      
  handle->isWritable = 1;
  for (struct list_head *c = handle->writeList.next, *cc = c->next;
       c != &handle->writeList; c = cc, cc = c->next) {
    struct chunk_to_write *node = containerof(c, struct chunk_to_write, list);
    _el_tryWrite(handle, &node->writen, node->chunk);
    
    if (node->writen == node->chunk.length) {
      if (node->onWriten)
	node->onWriten(handle);
      free(node->chunk.data);
      free(node);
    } else {
      handle->isWritable = 0;
      break;
    }
    
    el_update(handle);
  }
}

#define READ_BUFFER_SIZE 1000
void el_poll(struct event_loop *el) {
  struct epoll_event event_buffer[100];
  int ev_count = epoll_wait(el->fd, event_buffer, 100, -1);

  if (ev_count < 0){
    q_perror("eventloop","epoll_wait failed");
    return;
  }
  
  char read_buffer[READ_BUFFER_SIZE];
  for (int i = 0; i < ev_count; i++) {
    struct epoll_event e = event_buffer[i];
    struct fd_handle *handle = (struct fd_handle*)e.data.ptr;

    if (e.events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
      _handle_close(handle);
      continue;
    }
    
    if (e.events & EPOLLIN && handle->watcher.onData) {
      do {
	int l = read(handle->fd, read_buffer, READ_BUFFER_SIZE);
	handle->watcher.onData(handle, (struct chunk){.length = l, .data = read_buffer});
      } while (errno == EAGAIN);
    }
    
    if (e.events & EPOLLOUT) {
      _handle_write(handle);
    }
  }  
}

// Takes ownership of the chunk
// Can only be called from the thread that event loop belongs to.
void el_write(struct fd_handle *handle, struct chunk ch, fd_callback onWriten) {
  assert(ch.length > 0);

  int writen = 0;
  if (handle->isWritable) {
    _el_tryWrite(handle, &writen, ch);
  }

  if (writen != ch.length) {
    struct chunk_to_write *node = (struct chunk_to_write*)malloc(sizeof(struct chunk_to_write));
    node->chunk = ch;
    node->writen = writen;
    node->onWriten = onWriten;
    
    list_add_tail(&handle->writeList, &node->list);
  } else if (onWriten) {
    onWriten(handle);
    free(ch.data);
  }

  el_update(handle);
}
  


struct fd_handle* el_createHandle(struct event_loop *el, int fd, struct fd_watcher watcher)  {
  struct fd_handle *result = (struct fd_handle*)calloc(sizeof(struct fd_handle), 1);
  *result = (struct fd_handle) {};
  result->el = el;
  result->fd = fd;

  LIST_INIT(&result->writeList);
  
  result->watcher = watcher;
  
  _el_update(result, 1);
  
  return result;
}
