#include "chunk_builder.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "debug.h"

#define INITIAL_SIZE 10
void cb_init(struct chunk_builder *builder) {
  builder->chunks = (struct chunk*)
    malloc(sizeof(struct chunk) * INITIAL_SIZE);
  builder->capacity = INITIAL_SIZE;
  builder->usage = 0;
}

void cb_push(struct chunk_builder *builder, struct chunk chunk) {
  builder->usage++;
  if (builder->usage > builder->capacity) {
    builder->capacity += 10;
    builder->chunks = (struct chunk*)
      realloc(builder->chunks, sizeof(struct chunk) * builder->capacity);
  }
  
  builder->chunks[builder->usage - 1] = chunk;
  builder->totalLength += chunk.length;
  assert(builder->usage <= builder->capacity);
}

struct chunk cb_build(struct chunk_builder *builder) {
  struct chunk result;
  result.length = 0;
  result.data = (char*)malloc(builder->totalLength);
  for (int i = 0; i < builder->usage; i++) {
    memcpy(result.data + result.length, builder->chunks[i].data, builder->chunks[i].length);
    result.length += builder->chunks[i].length;
  }
  return result;
}

void cb_free(struct chunk_builder *builder) {
  free(builder->chunks);
}
