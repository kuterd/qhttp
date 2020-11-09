#include <stdio.h>

#include "chunk_builder.h"

int main(int argc, char *args[]) {
  struct chunk_builder builder;
  cb_init(&builder);
  
  cb_push(&builder, CHUNK_FROM_CONST_STRING("Merhaba"));
  cb_push(&builder, CHUNK_FROM_CONST_STRING(" "));
  cb_push(&builder, CHUNK_FROM_CONST_STRING("Dünya"));
  
  struct chunk ch = cb_build(&builder);
  printf("%.*s\n", ch.length, ch.data);
}
