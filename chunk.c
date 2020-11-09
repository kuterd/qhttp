#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "chunk.h"
#include "debug.h"

int chunk_findChar(struct chunk ch, char c) {
  for (int i = 0; i < ch.length; i++) {
    if (ch.data[i] == c)
      return i;
  }
  return -1;
}

struct chunk chunk_fromString(char *str) {
  return (struct chunk) {
    .length = strlen(str),
    .data = str
  };
}


struct chunk chunk_clone(struct chunk chunk) {
  struct chunk result;
  result.data = (char*)malloc(chunk.length);
  result.length = chunk.length;

  memcpy(result.data, chunk.data, chunk.length);
  
  return result;
}


int chunk_cmp(struct chunk a, struct chunk b) {
  return a.length == b.length &&
    memcmp(a.data, a.data, a.length) == 0;
}

int chunk_skip_safe(struct chunk *c, unsigned int skip) {
  if (c->length <= skip) {
    q_error("chunk", "safe skip failed  %d", c->length); 
    return 0;
  }
  c->length -= skip;
  c->data += skip;

  return 1;
}

#if 0
/*
struct chunk chunk_fromChunkArray(unsigned int n, struct chunk *chunks) {
  
}
*/

//%@ : chunk
struct chunk chunk_format(char *format, ...) {
  va_list args;
  va_start(args, format);

  int capacity = 10;
  int usage = 0;
  struct chunk *chunks = (struct chunk*)malloc(sizeof(struct chunk) * capacity);
  
  for (int i = 0; format[i] != '\0'; i++) {
    if (format[i] == '%') {
      struct chunk cChunk = (struct chunk){};
      if (i > 0) {
	
      }
      i++;

      if (format[i] == '@') {

      }

      if (cChunk.length > 0) {
	
      }
    }
  }
  struct chunk result = 
  free(chunks);
}
#endif
