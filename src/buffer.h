#ifndef _BUFFER_H_
#define _BUFFER_H_

typedef struct buffer_t buffer;

buffer* buffer_new(int size); 
void    buffer_destroy(buffer *buf);

int buffer_size(buffer *buf);

void    buffer_set_data(buffer *buf, void *new_data, int new_data_sz);

char*   buffer_get_str(buffer* buf);
void    buffer_append_str(buffer* buf, char* appendee);
#endif
