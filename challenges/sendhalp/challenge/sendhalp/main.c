#include <windows.h>
#include <stdio.h>

typedef int (*myexport)(char*, char*);

int main(int argc, char *argv[]) {
  HMODULE dll = LoadLibraryA(".\\libsendhalp.dll");
  myexport encrypt, decrypt;
  if(!dll) {
    fprintf(stderr, "Couldn't load the .dll!");
    exit(1);
  }
  printf("dll: %p\n", dll);

  encrypt = (myexport) GetProcAddress(dll, "encrypt");
  if(!encrypt) {
    fprintf(stderr, "Can't find encrypt function!");
    exit(1);
  }
  printf("encrypt: %p\n", encrypt);

  encrypt(argv[1], argv[2]);

  return 0;
}
