#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include <assert.h>

#define IPV4_ADDR_BUF_MAX_SZ sizeof("000.000.000.000")
#define IPV4_NET_BUF_MAX_SZ IPV4_ADDR_BUF_MAX_SZ + sizeof("/00")

// global static buffer for formatting
static char dst[IPV4_ADDR_BUF_MAX_SZ];

typedef struct Network {
  uint8_t  prefix; // prefix length
  uint32_t mask;   // mask resulting from the prefix length
  uint32_t addr;   // BASE address of the network (masked bits are set to 0)
} NetworkT;

void printHelp() {
  fputs("ipv4expand, v1.0 - Martijn Fleuren\n\n  -i  Omit network and broadcast address for prefixes <= 30\n", stderr);
}

/**
 * Compact ipv4 network address and prefix parser.
 *
 * Will scan the dotted-decimal notation from left to right, parsing each octet
 * as a decimal in the variable @b@. When it encounters a dot, or the
 * end-of-string, it will shift-left the value in @ret@ and add (bitwise or) the
 * value of b to it. 
 *
 * The resulting address will be in network-endian order (big)
 */
void parseNetwork(char * src, NetworkT * network) {
  uint32_t ret = 0;
  uint8_t  b   = 0;

  char * c;
  for (c = src; *c != '\0'; ++c) {
    if (*c == '/') {
      ++c;
      break;
    }

    if (*c == '.') {
      ret = (ret << 8) | b;
      b = 0;
    } else {
      b = (b * 10) + (*c - '0');
    }
  }
  
  network->addr   = (ret << 8) + b;
  network->prefix = (uint8_t) atoi(c);
  network->mask   = ~((1 << (32 - network->prefix)) - 1);
  network->addr  &= network->mask;

}


/**
 * TODO: I don't like the look of this function
 */
void toDotted(uint32_t addr) {
  uint8_t 
      a = 0
    , b = 0
    , c = 0
    , d = 0
    ;

  a = (addr >> 24) & 0xff;
  b = (addr >> 16) & 0xff;
  c = (addr >>  8) & 0xff;
  d = (addr >>  0) & 0xff;

  snprintf(dst, IPV4_ADDR_BUF_MAX_SZ, "%u.%u.%u.%u", a, b, c, d);
}

/**
 * Create an uninitialized a new network struct
 */
NetworkT * newNetwork() {
  NetworkT * network = (NetworkT *) calloc(1, sizeof(NetworkT));

  network->prefix = 0;
  network->mask   = 0;
  network->addr   = 0;

  return network;
}

/**
 * Free a network struct
 */
void freeNetwork(NetworkT * network) {
  if (network != NULL)
    free(network);
}

void printNetwork(NetworkT * network) {
  if (network == NULL)
    return;

  toDotted(network->addr);

  printf("network %s/%u\n", dst, network->prefix);
}

/**
 * Read an ip range in dotted-decimal notation with a prefix length from stdin,
 * and generate a list of ips within that range. optionally the network and
 * broadcast addresses are omitted from the output.
 *
 * The network address is specified as the address which has all unmasked bits
 * set to 0.
 *
 * The broadcast address is specified as the address which has all unmasked bits
 * set to 1.
 *
 * This option has no effect for /32 and /31 networks because /32 networks are
 * usually denoting a single address which would then be the broadcast AND the
 * network address which is senseless. /31 networks only have 2 members, which
 * would be omitted if we'd enforce the omission. Therefore this option only
 * makes sense in networks with a prefix length of /30 or less.
 */
int main(int argc, char ** argv) {

  char input[IPV4_NET_BUF_MAX_SZ];
  int omitNetworkAndBroadcastAddr = 0;

  size_t 
      lo = 0
    , hi = 0
    ;

  char c;

  while ((c = (char) getopt(argc, argv, "ih")) != (char) -1) {
    switch (c) {
      case 'i': 
          omitNetworkAndBroadcastAddr = 1;
        break;

      case 'h': // print help
        printHelp();
        exit(0);

      default:
        fprintf(stderr, "Invalid option, ignoring. pass -h to see help\n");
        break;
    }
  }

  // This check has to be done according to the GNU getopt specification
  if (optind != argc) {
    printf("You gave more options than have been processed.\n");
  }

  fgets(input, IPV4_NET_BUF_MAX_SZ, stdin);

	NetworkT * network = newNetwork();
  parseNetwork(input, network);

  if (network->prefix >= 31) {
    toDotted(network->addr);
    printf("%s\n", dst);
  }

  if (network->prefix == 31) {
    toDotted(network->addr + 1);
    printf("%s\n", dst);
  }

  if (network->prefix <= 30) {
    lo = omitNetworkAndBroadcastAddr;
    hi = ~network->mask - lo;

    for (size_t i = lo; i <= hi; ++i) {
      toDotted(network->addr + i);
      printf("%s\n", dst);
    }
  }

  free(network);

  return EXIT_SUCCESS;
}
