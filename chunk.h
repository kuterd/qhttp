#ifndef CHUNK_H
#define CHUNK_H

#include <assert.h>
#include "chunk.h"

#define chunk_fromConstStr(str)					\
  (struct chunk) { .length = sizeof(str) - 1, .data = str }

#define chunk_skip(ch, of) assert((ch).length > (of)); (ch).length -= (of); (ch).data += of;

struct chunk {
  char *data;
  unsigned int length;
};

int chunk_findChar(struct chunk ch, char c);
struct chunk chunk_fromString(char *str);
struct chunk chunk_clone(struct chunk chunk);
int chunk_cmp(struct chunk a, struct chunk b);
int chunk_skip_safe(struct chunk *c, unsigned int skip);



#endif
