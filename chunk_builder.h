#ifndef CHUNK_BUILDER_H
#define CHUNK_BUILDER_H

#include "chunk.h"

struct chunk_builder {
  struct chunk *chunks;
  unsigned int capacity, usage;
  unsigned int totalLength;
};

void cb_init(struct chunk_builder *builder);
void cb_push(struct chunk_builder *builder, struct chunk chunk);
struct chunk cb_build(struct chunk_builder *builder);
void cb_free(struct chunk_builder *builder); 

#endif
