
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <poll.h>
#include <ctype.h>
#include <limits.h>


#define BUFFSIZE 1024
#define PORT 8080

#define SEND 1
#define RECV 0
