#include <stdio.h>
#include <unistd.h>
#include <memory>
#include <stdint.h>

#include "common.h"
#include "conn.h"
#include "crypter.h"

namespace opendoor {

  static void usage() {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "\t-p <port>: Set port for listening.\n");
    fprintf(stderr, "\t-n: Disable encryption.\n");
    exit(-1);
  }

  int parse_flags(
      std::shared_ptr<opendoor::ConnectionPool> pool, int argc, char **argv) {
    int c, port;
    char *buf;
    while ((c = getopt(argc, argv, "np:")) != -1) {
      switch(c) {
        case 'p':
          port = strtol(optarg, &buf, 0);
          if (buf == optarg || port < 0 || port > UINT16_MAX) {
            fprintf(stderr, "Invalid port specified.\n");
            usage();
            return -1;
          }
          DBG("Setting port to %d.", port);
          pool->set_port((short)port);
          break;
        case 'n':
          DBG("Installing plaintext crypter.");
          pool->install_crypter(std::shared_ptr<Crypter>(
                dynamic_cast<Crypter *>(new PlainCrypter(true))));
          break;
        case '?':
          fprintf(stderr, "Invalid option: -%c\n", optopt);
          usage();
          return -1;
      }
    }
    return 0;
  }

  int main_loop(std::shared_ptr<opendoor::ConnectionPool> pool) {
    if (pool->listen()) {
      fprintf(stderr, "Failed listening.\n");
      return 1;
    }
    while(1) {
      // TODO: error checking
      pool->iterate();
    }
    return 0;
  }

} // namespace opendoor

int main(int argc, char **argv) {
  std::shared_ptr<opendoor::ConnectionPool> pool
    = std::make_shared<opendoor::ConnectionPool>();
  pool->install_crypter(std::shared_ptr<opendoor::Crypter>(
        dynamic_cast<opendoor::Crypter *>(new opendoor::AESCrypter())));
  if (opendoor::parse_flags(pool, argc, argv))
    return -1;
  DBG("Starting main loop.");
  return opendoor::main_loop(pool);
}

