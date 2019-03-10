#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <malloc.h>
#include <fcntl.h>
#include <string.h>

#include "stringops.h"

#define NOISE_SIZE 32
#define NOISE_CHARS "qwertyuiop[]\\asdfghjkl;'zxcvbnm,./`1234567890--=~!@#$%^&*()_+QWERTYUIOP{}|ASDFGHJKL:\"ZXCVBNM<>?"

int file_size(const char *name);
void read_noise(char *buf);
int read_flag(const char *name, char *buf);
char *readline();

int main(int argc, char **argv) {
  const char *flag_file = "./flag.txt";
  int buffer_size = file_size(flag_file);
  if (buffer_size == -1) {
    return -1;
  }
  buffer_size += NOISE_SIZE + 1;
  char *buf = malloc(buffer_size);
  if (buf == (void *)-1) {
    return -1;
  }
  memset(buf, 0, buffer_size);
  read_noise(buf);
  read_flag(flag_file, buf+NOISE_SIZE);
  int tries = strlen_fast(buf) * 7;
  do {
    printf("%d> ", tries);
    fflush(stdout);
    char *line = readline();
    if (!line) {
      return -1;
    }
    int rv = strcmp_fast(buf, line);
    if (rv == 0) {
      printf("Well done.\n");
      return 0;
    }
    printf("Ah ah ah, you didn't say the magic word! [%d]\n", rv);
  } while(--tries);
  return 0;
}


int file_size(const char *name) {
  struct stat info;
  int rv = stat(name, &info);
  if (rv != 0)
    return -1;
  return info.st_size;
}


void read_noise(char *buf) {
  int fd = open("/dev/urandom", O_RDONLY);
  int noise_bytes = 0;
  int noise_chars_len = strlen_fast(NOISE_CHARS);
  while (noise_bytes < NOISE_SIZE) {
    unsigned int b;
    do {
      read(fd, &b, sizeof(b));
      b &= 0xff;
      if (b < noise_chars_len)
        break;
    } while(1);
    buf[noise_bytes++] = NOISE_CHARS[b];
  }
  close(fd);
}

int read_flag(const char *name, char *buf) {
  int len = file_size(name);
  int done = 0;
  if (len < 0)
    return -1;
  int fd = open(name, O_RDONLY);
  if (fd < 0)
    return -1;
  while (done < len) {
    int r = read(fd, buf+done, len-done);
    if (r < 0) {
      close(fd);
      return -1;
    }
    done += r;
  }
  if (buf[done-1] == '\n')
    buf[done-1] = '\0';
  close(fd);
  return done;
}

char *readline() {
  int done = 0;
  static char buf[256];
  while (done < sizeof(buf)) {
    int r = read(STDIN_FILENO, buf+done, sizeof(buf)-done);
    if (r < 0)
      return NULL;
    done += r;
    if (buf[done-1] == '\n') {
      buf[done-1] = '\0';
      break;
    }
  }
  buf[sizeof(buf) - 1] = '\0';
  return buf;
}
