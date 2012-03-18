#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "buffer.h"

struct buffer_t {
  char* data;
  int size;
  int capacity; // reall + 1 as we always slap a 0 on end
};

buffer* buffer_new(int capacity) {
  buffer *buf = (buffer*) malloc(sizeof(buffer));
  assert(buf);
  buf->data = (char*) malloc(capacity + 1);
  buf->data[capacity] = 0;
  buf->size = 0;
  buf->capacity = capacity;
  return buf;
}

void buffer_destroy(buffer *buf) {
  free(buf->data);
  free(buf);
}

int buffer_size(buffer *buf) {
  return buf->size;
}

void buffer_set_data(buffer *buf, void *new_data, int new_data_sz) {
  if (buf->capacity > (new_data_sz + 1)) {
    buf->size = new_data_sz;
    memcpy(buf->data, new_data, new_data_sz);
    buf->data[new_data_sz] = 0;
  }
  else {
    realloc(buf->data, new_data_sz + 1);
    assert(buf);
    buf->capacity = new_data_sz;
    buf->size = new_data_sz;
    buf->data[new_data_sz] = 0;
    memcpy(buf->data, new_data, new_data_sz);
  }
}

void buffer_append_str(buffer *buf, char *str) {
  char* new_str = malloc(strlen(buf->data) + strlen(str) + 1);
  memcpy(new_str, buf->data, strlen(buf->data));
  memcpy(new_str + strlen(buf->data), str, strlen(str));
  new_str[strlen(buf->data) + strlen(str)] = '\0';
  buffer_set_data(buf, new_str, strlen(new_str) + 1);
  free(new_str);
}

char* buffer_get_str(buffer* buf) {
  return (char*) buf->data;
}
