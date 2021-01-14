#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

int seed = 0;

int my_rand() {
  seed = (214013 * seed + 2531011) & 0x7fffffff;
  return seed >> 16;
}

void gen_password(uint8_t buffer[12]) {
  uint32_t i;

  for(i = 0; i < 12; i++) {
    buffer[i] = my_rand() % 0xFF;
  }
}

void print_hex(uint8_t *hex) {
  uint32_t i;
  for(i = 0; i < 12; i++) {
    printf("%02x", hex[i]);
  }
}

uint8_t to_h(char c) {
  c = toupper(c);

  if(c >= '0' && c <= '9') {
    return c - '0';
  } else if(c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  }

  printf("Error: not a hex character: %c\n", c);
  exit(1);
}

int main(int argc, char *argv[]) {
  uint32_t i;
  uint8_t *generated_pw = malloc(12);
  uint8_t *desired = malloc(12);

  if(argc < 2) {
    printf("Usage: %s <pw>\n", argv[0]);
    exit(1);
  }

  if(strlen(argv[1]) != 24) {
    printf("Usage: %s <24 character / 12 byte pw>\n", argv[0]);
    exit(1);
  }

  for(i = 0; i < 24; i+= 2) {
    desired[i/2] = (to_h(argv[1][i]) << 4) | to_h(argv[1][i+1]);
  }

  int index;
  for(index = 0x0; index < 0x7FFFFFFF; index++) {
    seed = index;

    gen_password(generated_pw);

    if(!memcmp(generated_pw, desired, 12)) {
      printf("Found it! Seed = %d\n", index);
      gen_password(generated_pw);
      printf("Next password: \n");
      print_hex(generated_pw);
      printf("\n");
      exit(0);
    }

    if((index % 0x100000) == 0) {
      printf("Current status: 0x%08x / 0x7FFFFFFF\n", index);
#if 0
      print_hex(desired);
      printf(" != ");
      print_hex(generated_pw);
      printf("\n");
#endif
    }
  }

  return 0;
}
