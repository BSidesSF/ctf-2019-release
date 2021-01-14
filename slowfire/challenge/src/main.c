#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define PORT 4141

#define CHECK(x) do { \
  int rv = (x); \
  if (rv < 0) \
    exit(rv); \
} while(0);

int setup_socket(uint16_t port);
int accept_and_fork(int sock);
int handle_client(int fd);
int read_line_safe(int fd, char *buf, size_t buf_size);
int write_string(int fd, char *str);
void invert_case(char *str);

char name[32];
char _[sizeof(name)];

int main(int argc, char **argv) {
  int s = setup_socket(PORT);
  while(1) {
    int fd = accept_and_fork(s);
    if (fd > 0) {
      return handle_client(fd);
    } else if (fd < 0) {
      return -1;
    }
  };
  return 0;
}

void invert_case(char *str) {
  int len = strlen(str);
  char case_bit = 'A' ^ 'a';
  for (int i=0;i<len;i++) {
    if ((str[i] >= 'A' && str[i] <= 'Z') || (str[i] >= 'a' && str[i] <= 'z'))
      str[i] ^= case_bit;
  }
}

int handle_client(int fd) {
  char msg[1024];
  char *prompt = "Enter your name> ";
  CHECK(write_string(fd, prompt));
  CHECK(read_line_safe(fd, name, sizeof(name)));
  prompt = "Enter message> ";
  CHECK(write_string(fd, prompt));
  CHECK(read_line_safe(fd, msg, sizeof(msg)));
  invert_case(msg);
  prompt = "Greetings ";
  CHECK(write_string(fd, prompt));
  CHECK(write_string(fd, name));
  prompt = "!\n  Your converted message is:\n";
  CHECK(write_string(fd, prompt));
  CHECK(write_string(fd, msg));
  return 0;
}

int write_string(int fd, char *str) {
  int total = 0, rv = 0;
  int max = strlen(str);
  do {
    rv = write(fd, str + total, max - total);
    if (rv < 1)
      return rv;
    total += rv;
  } while(total < max);
  return total;
}

int read_line_safe(int fd, char *buf, size_t buf_size) {
  memset(buf, 0, buf_size);
  int rb = 0;
  int total = 0;
  do {
    rb = read(fd, buf+total, buf_size-1);
    if (rb < 1) {
      break;
    }
    total += rb;
    if (buf[total-1] == '\n') {
      buf[total-1] = '\0';
      break;
    }
  } while(total < buf_size);
  buf[total] = '\0';
  return total;
}

int setup_socket(uint16_t port) {
  int s = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in sa = {0};
  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  int enable = 1;
  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
    perror("setsockopt(SO_REUSEADDR) failed");
    exit(-1);
  }
  if (bind(s, (struct sockaddr *)&sa, sizeof(sa))) {
    perror("bind");
    exit(-1);;
  }
  listen(s, 10);
  return s;
}

int accept_and_fork(int sock) {
  int fd = accept(sock, NULL, NULL);
  pid_t p = fork();
  if (p == 0)
    return fd;
  if (p == -1)
    return -1;
  return 0;
}
