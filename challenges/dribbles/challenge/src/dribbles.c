#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>

const int MAX_READ = 1024;
char mask[sizeof(void *)] = {0};

typedef struct {
  int is_masked;
  union {
    void *ptr;
    char mask[sizeof(void *)];
  } data;
} masked_ptr;

__attribute__((constructor))
void set_mask() {
  for(int i=0;i<sizeof(void *);i++)
    mask[i] = 0x55;
}

int disable_buffering() {
  setbuf(stdout, NULL);
  return 0;
}

int chcwd() {
  char buf[256];
  char *dir;
  int rv = readlink("/proc/self/exe", buf, sizeof(buf)/sizeof(char));
  if (rv < 0 || rv >= sizeof(buf)) {
    return -1;
  }
  dir = dirname(buf);
  chdir(dir);
}

int print_buffer(const void *addr, const size_t bytes) {
  int np;
  for (np=0;np<bytes;np++) {
    if (!(np % 16)) {
      if (np)
        printf("\n");
      printf("%p: ", addr + np);
    }
    if (!(np % 8))
      printf(" ");
    printf("%02x ", (*(((char *)addr) + np)) & 0xFF);
  }
  printf("\n");
}

int read_flag(masked_ptr **mp) {
  *mp = malloc(sizeof(masked_ptr));
  int fd = open("flag.txt", O_RDONLY);
  if (fd < 0) {
    return -1;
  }

  (*mp)->data.ptr = mmap(NULL, 4096, PROT_READ, MAP_PRIVATE, fd, 0);
  if ((*mp)->data.ptr == MAP_FAILED) {
    return -1;
  }

  (*mp)->is_masked = 1;
  for(int i=0;i<sizeof(void *);i++)
    (*mp)->data.mask[i] ^= mask[i];
}

int readline(char *buf, size_t len) {
  // read line and strip newline
  int br = 0;
  while (br < len-1) {
    int rv = read(STDIN_FILENO, buf + br, 1);
    if (rv < 0)
      return rv;
    if (buf[br] == '\n' || buf[br] == '\r') {
      buf[br] = '\0';
      return br;
    }
    br += rv;
  }
  buf[len-1] = '\0';
  return br;
}

int offer_symbols() {
  int sym_max = 3;
  int sym;
  char buf[256];
  int rv;
  void *ptr;

  for(sym = 0; sym < sym_max; sym++) {
    printf("[%d/%d] Resolve symbol> ", sym+1, sym_max);
    if ((rv = readline(buf, sizeof(buf)/sizeof(char))) <= 0)
      return rv;
    dlerror();
    ptr = dlsym(RTLD_DEFAULT, buf);
    if (!ptr && dlerror()) {
      printf("Symbol not found!\n");
      return -1;
    }
    printf("%s (buf@%p) = %p\n", buf, buf, ptr);
  }
  return 0;
}

int offer_memory() {
  void *ptr;
  char buf[256];
  int rv;
  size_t sz;
  char *end;
  char *off;

  while (1) {
    printf("Provide address and number of bytes to read> ");
    if ((rv = readline(buf, sizeof(buf)/sizeof(char))) <= 0)
      return rv;
    ptr = (void *)strtoull(buf, &end, 0);
    if (end == buf) {
      printf("Invalid address.\n");
      return -1;
    }
    off = end;
    while (*off == ' ' || *off == ',' || *off == '\t')
      off++;
    if (*off == '\0') {
      printf("No length provided.");
      return -1;
    }
    sz = strtoull(off, &end, 0);
    if (off == end) {
      printf("Invalid length.\n");
      return -1;
    }
    if (sz > MAX_READ) {
      printf("Invalid length.\n");
      return -1;
    }
    print_buffer(ptr, sz);
  }

  return 0;
}

int main(int argc, char **argv) {
  masked_ptr *mp;
  disable_buffering();
  chcwd();
  if (read_flag(&mp) == -1) {
    return -1;
  }
  if (offer_symbols() < 0) {
    return -1;
  }
  return offer_memory();
}
