#include <libmemcached/memcached.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

static void die(char *msg) {
  fputs(msg, stderr);
  fputs("\n", stderr);
  exit(1);
}

static int usage() {
  puts("Usage: memload -s <server> [<input> ...]");
  puts("  -s <server>     : server name, port optional (ie localhost:11211)");
  puts("                    use multiple times for multiple servers");
  puts("  -d <delimiters> : string of delimiters, defaults to tab");
  puts("  -c <count>      : exit after loading count entries, useful for testing");
  puts("  -H <hash>       : hash algorithm to use instead of libmemcached's default");
  puts("  -v              : verbose output");
  puts("  -h, -?          : show this help output");
  puts("  <input>         : file to take input from, otherwise will take from stdin");
  puts("                    multiple files will be processed in order if specified");
  return 0;
}


static int process_file(FILE *in, 
                        int buf_size, 
                        memcached_st *m, 
                        char *delimiter, 
                        int verbose,
                        int *max_count) {
  char line[buf_size];
  char *parsed;
  memcached_return_t rs;

  while ((parsed = fgets(line, buf_size, in))) {
    char *key = strsep(&parsed, delimiter);
    assert(key);
    char *value = strsep(&parsed, delimiter);
    assert(value);
    if (value[strlen(value) - 1] =='\n') {
      value[strlen(value) - 1] = '\0';
    }
    rs = memcached_set(m, 
                         key, strlen(key),
                         value, strlen(value),
                         (time_t)0,
                         (uint32_t)0);
    if (verbose) {
      memcached_server_instance_st srv = memcached_server_by_key(m, key, strlen(key), &rs);
      const char *srv_name = memcached_server_name(srv);
      int srv_port = memcached_server_port(srv);
      printf("'%s' => '%s' at %s:%d\n", key, value, srv_name, srv_port);
    }
    if (!memcached_success(rs)) {  
      printf("error '%s' trying to insert %s => %s\n", 
             memcached_strerror(m, rs),key, value);
      return 1;
    }

    if (--(*max_count) == 0) { 
      return 2; 
    }
  }
  return 0;
}


int main(int argc, char *argv[]) {

  // temp vars
  memcached_return_t rs;
  int ch;

  // command line flags
  int buf_size = sysconf(_SC_PAGESIZE);
  int verbose = 0;
  int very_verbose = 0;
  int max_count = -1;
  char *delimiter = "\t";
  char *init_config = "--NOREPLY --BUFFER-REQUESTS";
  char *config = malloc(strlen(init_config));
  memcpy(config, init_config, strlen(init_config));

  int len;
  while ((ch = getopt(argc, argv, "S:H:s:d:Vvc:?h")) != -1) {
    switch (ch) {
    case 's':
      len = strlen(config);
      config = realloc(config, strlen(config) + strlen(optarg) + 9 + 2);
      assert(config);
      
      memcpy(config + len, " ", 1); // replace null with space
      memcpy(config + len + 1, "--SERVER=", 9); // append --SERVER=
      memcpy(config + len + 1 + 9, optarg, strlen(optarg)); // append optarg
      config[len + 1 + 9 + strlen(optarg)] = '\0'; // null terminate
      break;
    case 'H':
      len = strlen(config);
      config = realloc(config, strlen(config) + strlen(optarg) + 6 + 2);
      assert(config);
      
      memcpy(config + len, " ", 1); // replace null with space
      memcpy(config + len + 1, "--HASH=", 7); // append --HASH=
      memcpy(config + len + 1 + 7, optarg, strlen(optarg)); // append optarg
      config[len + 1 + 7 + strlen(optarg)] = '\0'; // null terminate
      break;
    case 'V':
      very_verbose = 1;
      // fallthrough
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

  if (!strcmp(init_config, config)) {
    // no config was specified
    return usage();
  }

  argc -= optind;
  argv += optind;

  if (very_verbose) {printf("libmemcached config = '%s'\n", config);}

  memcached_st* m = memcached(config, strlen(config));
  assert(m);

  char line[buf_size];
  char *parsed;

  // if anythign left on command line then treat it as file to read
  if (argc > 0) {
    while (argc > 0) {
      FILE *in;
      in = fopen(argv[0], "r");
      if(!process_file(in, buf_size, m, delimiter, verbose, &max_count)) {
        goto ESCAPE;
      }
      fclose(in);
      argc--;
      argv++;
    }
  }
  else {
    if (!process_file(stdin, buf_size, m, delimiter, verbose, &max_count)) {
      goto ESCAPE;
    }
  }

  ESCAPE:
  rs = memcached_flush_buffers(m);
  if (!memcached_success(rs)) {  
    puts(memcached_strerror(m, rs)); 
    return 1;
  }

  memcached_quit(m);
  memcached_free(m);
  return 0;
}
