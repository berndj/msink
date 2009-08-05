#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <libgen.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

/**
 * Setup a multicast socket 
 */
int setupSocket(struct in_addr laddr_, struct sockaddr_in *msin_) {
  int fd;
  struct ip_mreq mreq;
  int yes = 1;

  fd = socket(PF_INET, SOCK_DGRAM, 0);

  memset(&mreq, 0, sizeof(mreq));
  mreq.imr_multiaddr.s_addr = msin_->sin_addr.s_addr;
  mreq.imr_interface.s_addr = laddr_.s_addr;

  if (setsockopt(fd,
                 SOL_SOCKET,
                 SO_REUSEADDR,
                 &yes, sizeof (yes)) < 0) {
    perror("setsockopt SO_REUSEADDR");
    return -1;
  }
  if(bind(fd,
          (struct sockaddr *)msin_,
	  sizeof(*msin_)) < 0) {
    perror ("bind");
    return -1;
  }
  if (setsockopt(fd,
                 IPPROTO_IP,
                 IP_ADD_MEMBERSHIP,
                 &mreq, sizeof (mreq)) < 0) {
    perror("setsockopt IP_ADD_MEMBERSHIP");
    return -1;
  }

  return fd;

}

/**
 * Print command usage
 */
void printUsage(char *argv[]) {
  fprintf(stderr, "Usage: %s -m mcast-addr -p port [-b interface-addr] \n",
	  basename(argv[0]));
  fprintf(stderr, "  -m, --mcast=MCAST_ADDRESS\n");
  fprintf(stderr, "  -b, --bind=LOCAL_ADDRESS\n");
  fprintf(stderr, "  -p, --port=PORT\n");
  fprintf(stderr, "  -h, --help\n");
}

/**
 * Print version information
 */
void printVersion(char *argv[]) {
  fprintf(stderr, "%s version %s\n", argv[0], VERSION);
  fprintf(stderr,"Copyright (C) 2007  Bernd Jungblut\n");
  fprintf(stderr,"This program comes with NO WARRANTY, to the extent permitted by law.\n");
  fprintf(stderr,"You may redistribute copies of this program under the terms of\n");
  fprintf(stderr,"the GNU General Public License.\n");
}

/**
 * The main function  
 */
int main(int argc, char *argv[]) {

  int fd, opt;
  struct in_addr laddr, maddr;
  struct sockaddr_in sin;
  unsigned short port=0;
  char message[255];

  laddr.s_addr = 0;
  maddr.s_addr = 0;

  /* check for command line switches */
  while (1) {
    int option_index = 0;
    static const struct option long_options[] = {
      { "mcast",     required_argument,      NULL, 'm' },
      { "bind",      required_argument,      NULL, 'b' },
      { "port",      required_argument,      NULL, 'p' },
      { "help",      no_argument,            NULL, 'h' },
      { "version",   no_argument,            NULL, 'v' },
      { 0, 0, 0, 0 },
    };

    opt = getopt_long(argc, argv, "m:b:p:hv",
		    long_options, &option_index);

    if (opt == -1) {
      break;
    }
    switch (opt) {
    case 'm':
      inet_pton(AF_INET, optarg, &maddr);
      break;
    case 'b':
      inet_pton(AF_INET, optarg, &laddr);
      break;
    case 'p':
      port = atoi(optarg);
      break;
    case 'h':
      printUsage(argv);
      exit(EXIT_SUCCESS);
      break;
    case 'v':
      printVersion(argv);
      exit(EXIT_SUCCESS);
      break;
    default:
      printUsage(argv);
      exit(EXIT_FAILURE);
      break;
    }

  }

  if (maddr.s_addr == 0 || port == 0) {
    printUsage(argv);
    exit(EXIT_FAILURE);  
  }

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = maddr.s_addr;
  sin.sin_port = htons(port);

  if (-1 == (fd = setupSocket(laddr, &sin))) {
    perror("setupSocket");
    exit(EXIT_FAILURE);
  }

  while (1) {
    unsigned int sin_len = sizeof(sin);
    memset(message, 0, sizeof(message));
    if (recvfrom(fd, message, sizeof(message), 0,
                   (struct sockaddr *) &sin, &sin_len) == -1) {
        perror ("recvfrom");
    }
    printf ("%s",message);

  }
  exit(EXIT_SUCCESS);
}
