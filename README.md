Utility for bulk loading memcached. Reads key and value from stdin or passed files and parses them as key/value pairs with a configurable delimiter. First column is the key, second column is the value.

	Usage: memload -s <server> [<input> ...]
	  -s <server>     : server name, port optional (ie localhost:11211)
	                    use multiple times for multiple servers
	  -d <delimiters> : string of delimiters, defaults to tab
	  -c <count>      : exit after loading count entries, useful for testing
	  -H <hash>       : hash algorithm to use instead of libmemcached's default
	  -v              : verbose output
	  -h, -?          : show this help output
	  <input>         : file to take input from, otherwise will take from stdin
	                    multiple files will be processed in order if specified

Requires [libmemcached](http://libmemcached.org/). See the Makefile to change library location etc as there is no autoconf, the thing is too simple to warrant ;-)