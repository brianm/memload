#include <libmemcached/memcached.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "buffer.h"


static void die(char *msg) {
  fputs(msg, stderr);
  fputs("\n", stderr);
  exit(1);
}

static int usage() {
  puts("Usage: bt_load -s <server> [-s <server> ...] [-v] [-c <count>] [-H <hash_algorithm>]");
  puts("  Pass list of serialization file names to stdin, one per line");
  return 0;
}


int main(int argc, char *argv[]) {

  // temp vars
  memcached_return_t rs;
  int ch;

  // command line flags
  int buf_size = sysconf(_SC_PAGESIZE);
  FILE *in = stdin;
  int verbose = 0;
  int max_count = -1;
  char *delimiter = "\t";
  buffer *servers = buffer_new(sysconf(_SC_PAGESIZE));
  buffer *hash = buffer_new(0);

  while ((ch = getopt(argc, argv, "S:H:s:d:vc:?h")) != -1) {
    switch (ch) {
    case 's':
      buffer_append_str(servers, "--SERVER=");
      buffer_append_str(servers, optarg);
      buffer_append_str(servers, " ");
      break;
    case 'H':
      buffer_append_str(hash, "--HASH=");
      buffer_append_str(hash, optarg);
      buffer_append_str(hash, " ");
      break;
    case 'v':
      verbose = 1;
      break;
    case 'c':
      max_count = atoi(optarg);
      break;
    case 'S':
      buf_size = atoi(optarg);
      break;
    case 'd':
      delimiter = optarg;
      break;
    case 'h':
    case '?':
    default:
      return usage();
    }
  }
  argc -= optind;
  argv += optind;

  if (buffer_size(servers) == 0) {
    return usage();
  }
  // if anythign left on command line then treat it as file to read
  if (argc > 0) {
    in = fopen(argv[0], "r");
  }

  char* config = malloc(  strlen("--NOREPLY --BUFFER-REQUESTS")
                        + buffer_size(servers)
                        + buffer_size(hash)
                        + 1);
  strcat(strcat(strcpy(config, buffer_get_str(hash)),
                buffer_get_str(servers)), 
         "--NOREPLY --BUFFER-REQUESTS");

  if (verbose) {printf("[%s]\n", config);}

  memcached_st* m = memcached(config, strlen(config));

  char line[buf_size];
  char *parsed;
  while ((parsed = fgets(line, buf_size, in))) {
    char *key = strsep(&parsed, delimiter);
    char *value = strsep(&parsed, delimiter);
    if (value[strlen(value) - 1] =='\n') {
      value[strlen(value) - 1] = '\0';
    }
    rs = memcached_set(m, 
                         key, strlen(key),
                         value, strlen(value),
                         (time_t)0,
                         (uint32_t)0);
    if (!memcached_success(rs)) {  
      printf("error '%s' trying to insert %s => %s\n", 
             memcached_strerror(m, rs),key, value);
      return 1;
    }

    if (--max_count == 0) { 
      rs = memcached_flush_buffers(m);
      if (!memcached_success(rs)) {  
        puts(memcached_strerror(m, rs)); 
        return 1;
      }

      memcached_quit(m);
      memcached_free(m);
      return 0; 
    }
  }

  rs = memcached_flush_buffers(m);
  if (!memcached_success(rs)) {  
    puts(memcached_strerror(m, rs)); 
    return 1;
  }

  memcached_quit(m);
  memcached_free(m);
  return 0;
}
