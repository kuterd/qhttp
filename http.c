#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>

#include "list.h"
#include "eventloop.h"
#include "chunk_builder.h"
#include "general.h"
#include "debug.h"

int read_line(struct chunk ch, struct chunk *currentLine, int *isDone, int maxLength) {
  int i;
  *isDone = 0;
  for (i = 0; i < ch.length; i++) {
    char c = ch.data[i];
    if (c == '\n') {
      *isDone = 1;
      break;
    } else if ( c != '\r') {
      if (currentLine->length == maxLength) return -1;
      currentLine->data[currentLine->length++] = c;
    }
  }
  return i + 1;
}

#define HTTP_KNOWN_METHOD_LIST(o)   \
  o(HEAD)                           \
  o(OPTIONS)                        \
  o(GET)                            \
  o(POST)                           \
  o(PUT)                            \
  o(DELETE)                         \
  o(TRACE)                          \
  o(CONNECT)

enum http_method {
   UNKNOWN,
   HTTP_KNOWN_METHOD_LIST(MAKE_ENUM)
};

#define MAKE_CHUNK(n) chunk_fromConstStr(#n),
struct chunk kHTTPMethodNameChunks[] = {
   MAKE_CHUNK(UNKNOWN)
   HTTP_KNOWN_METHOD_LIST(MAKE_CHUNK)
};

const char *kHTTPMethodNames[] = {
   MAKE_STRING(UNKNOWN)
   HTTP_KNOWN_METHOD_LIST(MAKE_STRING)
};

struct http_request {
  enum http_method method;

  struct chunk url;
  struct chunk method_chunk;
  struct chunk version;
  struct list_head headers;
};

struct http_data {
  struct chunk currentLine;
  struct http_request request;
  int isFirstLine;
};

struct key_value_node {
  struct chunk key, value;
  struct list_head list;
};

int readTokenUntil(struct chunk *result, struct chunk *data, char end) {
  int length = chunk_findChar(*data, end);

  if (length < 1)
    return Q_ERROR;
  *result = chunk_clone((struct chunk) {
      .length = length,
      .data = data->data
  });

  if (!chunk_skip_safe(data, length + 1))
    return Q_ERROR;
  return Q_OK;
}

//TODO: Replace assertions with errors
int http_parseRequestLine(struct http_request *req, struct chunk line) {
  req->method = UNKNOWN;
  if(!readTokenUntil(&req->method_chunk, &line, ' '))
    return Q_ERROR;


  for (int i = 1; i < sizeof(kHTTPMethodNameChunks); i++) {
    if (chunk_cmp(req->method_chunk, kHTTPMethodNameChunks[i])) {
	req->method = i;
	break;
    }
  }

  if(!readTokenUntil(&req->url, &line, ' '))
    return Q_ERROR;
  return Q_OK;
}

int http_parseHeader(struct http_request *req, struct chunk line) {
  struct chunk key;
  q_info("http", "Line %.*s", line.length, line.data);

  if(!readTokenUntil(&key, &line, ':'))
    return Q_ERROR;
  if(!chunk_skip_safe(&line, 1))
    return Q_ERROR;

  struct key_value_node *node = (struct key_value_node*)malloc(sizeof(struct key_value_node));
  node->key = key;
  q_info("http", "key %.*s", key.length, key.data);

  node->value = chunk_clone(line);

  list_add_tail(&req->headers, &node->list);

  return Q_OK;
}

void http_request_dump(struct http_request *request) {
  q_info("http", "Request method: %s", kHTTPMethodNames[request->method]);
  q_info("http", "Request URL: %.*s", request->url.length, request->url.data);

  LIST_FOR_EACH(&request->headers) {
    struct key_value_node *n = containerof(c, struct key_value_node, list);
    q_info("http", "Header %.*s", n->key.length, n->key.data);
  }
}

void http_onResponseWriten(struct fd_handle *handle) {
  close(handle->fd);
}


#define MAX_LINE_LENGTH 3000
void http_onData(struct fd_handle *handle, struct chunk ch) {
  assert(ch.length > 0);

  struct http_data *h_data = (struct http_data*)handle->watcher.userData;

  for (int isDone; ch.length != 0;) {
    int consumed = read_line(ch, &h_data->currentLine, &isDone, MAX_LINE_LENGTH);
    assert(consumed >= 0);

    ch.data += consumed;
    ch.length -= consumed;

    assert(ch.length >= 0);

    if (isDone) {
      if (h_data->isFirstLine) {
	h_data->isFirstLine = 0;

	q_info("http_parser", "Request Line: %.*s", h_data->currentLine.length, h_data->currentLine.data);
	http_parseRequestLine(&h_data->request, h_data->currentLine);


      } else if (h_data->currentLine.length != 0) {
	http_parseHeader(&h_data->request, h_data->currentLine);
      } else {
	http_request_dump(&h_data->request);
	q_info("http_parser", "Request complete");

	struct chunk_builder builder;
	cb_init(&builder);

	cb_push(&builder, chunk_fromConstStr("HTTP/1.1 200 OK\r\n"));
	cb_push(&builder, chunk_fromConstStr("Content-Type: text/html\r\n\r\n"));

	cb_push(&builder, chunk_fromConstStr("<h1>Request Method:"));
	cb_push(&builder, kHTTPMethodNameChunks[h_data->request.method]);
	cb_push(&builder, chunk_fromConstStr("</h2><p> URL: "));

	cb_push(&builder, h_data->request.url);
	cb_push(&builder, chunk_fromConstStr("</h2>"));

	LIST_FOR_EACH(&h_data->request.headers) {
	  q_info("http", "header");
	  struct key_value_node *kv = containerof(c, struct key_value_node, list);
	  cb_push(&builder, chunk_fromConstStr("<h3>Header: "));
	  cb_push(&builder, kv->key);
	  cb_push(&builder, chunk_fromConstStr("/"));
	  cb_push(&builder, kv->value);
	  cb_push(&builder, chunk_fromConstStr("</h3>"));
	}

	struct chunk response = cb_build(&builder);
	cb_free(&builder);

	q_info("http", "Response: %.*s", response.length, response.data);

	el_write(handle, response, http_onResponseWriten);
      }
      h_data->currentLine.length = 0;
    }
  }
}


void http_onClose(struct fd_handle *handle) {
  struct http_data *h_data = (struct http_data*)handle->watcher.userData;
  free(h_data->currentLine.data);
  free(h_data);
}


struct fd_watcher http_init() {
  struct http_data *data =
    (struct http_data*)malloc(sizeof(struct http_data));

  *data = (struct http_data) {};
  data->currentLine.data = (char*)malloc(MAX_LINE_LENGTH);
  data->currentLine.length = 0;
  data->isFirstLine = 1;

  data->request = (struct http_request) {};
  LIST_INIT(&data->request.headers);

  struct fd_watcher result = (struct fd_watcher) {};
  result.onData = http_onData;
  result.onClose = http_onClose;
  result.userData = (void*)data;
  return result;
}
