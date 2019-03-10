#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define TARGET "/home/ctf/genius"
#define SIZE 10032

#define disable_buffering(_fd) setvbuf(_fd, NULL, _IONBF, 0)

int decode_character(char code) {
  switch(code) {
    case 'A': return 0x0;
    case 'P': return 0x1;
    case 'Z': return 0x2;
    case 'L': return 0x3;
    case 'G': return 0x4;
    case 'I': return 0x5;
    case 'T': return 0x6;
    case 'Y': return 0x7;
    case 'E': return 0x8;
    case 'O': return 0x9;
    case 'X': return 0xA;
    case 'U': return 0xB;
    case 'K': return 0xC;
    case 'S': return 0xD;
    case 'V': return 0xE;
    case 'N': return 0xF;
  }
  return -1;
}

void decode(char *code, uint16_t *address, uint8_t *value) {
  uint8_t n0, n1, n2, n3, n4, n5;

  n0 = decode_character(code[0]);
  if(n0 < 0) {
    printf("Invalid character in code: %c\n", code[0]);
    exit(1);
  }

  n1 = decode_character(code[1]);
  if(n1 < 0) {
    printf("Invalid character in code: %c\n", code[1]);
    exit(1);
  }

  n2 = decode_character(code[2]);
  if(n2 < 0) {
    printf("Invalid character in code: %c\n", code[2]);
    exit(1);
  }

  n3 = decode_character(code[3]);
  if(n3 < 0) {
    printf("Invalid character in code: %c\n", code[3]);
    exit(1);
  }

  n4 = decode_character(code[4]);
  if(n4 < 0) {
    printf("Invalid character in code: %c\n", code[4]);
    exit(1);
  }

  n5 = decode_character(code[5]);
  if(n5 < 0) {
    printf("Invalid character in code: %c\n", code[5]);
    exit(1);
  }

  *address =
    ((n1 & 8) << 4)  |
    ((n2 & 7) << 4)  |
    ((n3 & 7) << 12) |
     (n3 & 8)        |
    ((n4 & 8) << 8)  |
     (n4 & 7)        |
    ((n5 & 7) << 8);

  *value =
     (n0 & 7) |
    ((n0 & 8) << 4) |
    ((n1 & 7) << 4) |
     (n5 & 8);

#if 0
  printf("address = %p\n", *address);
  printf("value = %x\n", *value);
#endif
}

int main(int argc, char *argv[]) {
  char buffer[8];
  char file_buffer[SIZE];

  disable_buffering(stdout);
  disable_buffering(stderr);

  printf("Welcome to the Game Genius interface!\n");
  printf("Loading game...\n");
  sleep(1);
  printf("...\n");
  sleep(1);
  FILE *f = fopen(TARGET, "rb");
  if(!f) {
    printf("Couldn't open game data: %s\n", TARGET);
    exit(1);
  }

  if(fread(file_buffer, 1, SIZE, f) != SIZE) {
    printf("Error reading game data: %s\n", TARGET);
    exit(1);
  }
  fclose(f);

  printf("Loaded!\n");
  printf("\n");
  printf("Please enter your first Game Genius code, or press <enter> to\n");
  printf("continue!\n\n");

  fgets(buffer, 8, stdin);
  if(buffer[0] != '\n') {
    uint16_t address;
    uint8_t value;
    decode(buffer, &address, &value);

    printf("Applying patch...\n");
    sleep(1);
    printf("...\n");
    sleep(1);

    if(address > SIZE) {
      printf("Bad address!\n");
      exit(1);
    }

    printf("...\n");
    sleep(1);
    file_buffer[address] = value;
    printf("Done!\n");
    printf("\n");


    printf("Please enter your second code, or press <enter>\n");
    fgets(buffer, 8, stdin);
    if(buffer[0] != '\n') {
      uint16_t address;
      uint8_t value;
      decode(buffer, &address, &value);

      printf("Applying patch...\n");
      sleep(1);
      printf("...\n");
      sleep(1);

      if(address > SIZE) {
        printf("Bad address!\n");
        exit(1);
      }

      printf("...\n");
      sleep(1);
      file_buffer[address] = value;
      printf("Done!\n");
    }
  }

  char *temp = tempnam(NULL, NULL);
  FILE *f2 = fopen(temp, "wb");
  fwrite(file_buffer, 1, SIZE, f2);
  fclose(f2);
  chmod(temp, 0755);

  pid_t pid = fork();

  if(!pid) {
    execl(temp, NULL);
  }

  waitpid(pid, NULL, 0);
  unlink(temp);
}
