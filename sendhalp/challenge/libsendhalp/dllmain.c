#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <stdlib.h>
#pragma comment(lib, "IPHLPAPI.lib")

/* Define this line to solve */
//#define HAX

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

#define N 249

#define SWAP(a,b) (a ^= b ^= a ^= b)

int KSA(char *key, size_t len, unsigned char *S) {
  int j = 0;
  int i;

  for(i = 0; i < N; i++)
    S[i] = i;

  for(i = 0; i < N; i++) {
    j = (j + S[i] + key[i % len]) % N;

    SWAP(S[i], S[j]);
  }

  return 0;
}

int PRGA(unsigned char *S, char *plaintext, unsigned char *ciphertext) {

  int i = 0;
  int j = 0;
  size_t n, len;

  for(n = 0, len = strlen(plaintext); n < len; n++) {
    int rnd;

    i = (i + 1) % N;
    j = (j + S[i]) % N;

    SWAP(S[i], S[j]);
    rnd = S[(S[i] + S[j]) % N];

    ciphertext[n] = rnd ^ plaintext[n];
  }

  return 0;
}

int RC4(unsigned char *key1, size_t length1, unsigned char *key2, size_t length2, unsigned char *key3, size_t length3, char *plaintext, unsigned char *ciphertext) {

  unsigned char S[N];

  KSA(key1, length1, S);
  KSA(key2, length2, S);
  KSA(key3, length3, S);


  PRGA(S, plaintext, ciphertext);

  return 0;
}

#ifdef HAX
void get_cpuid(char cpuid[13]) {
  strcpy(cpuid, "AMDisbetter!");
}

void get_hostname(char *name, DWORD *length) {
  strcpy(name, "FLAGSVR");
  *length = 7;
}
#else
void get_cpuid(char cpuid[13]) {
  DWORD b, c, d;

  __asm{
    xor eax, eax
    cpuid
    mov b, ebx
    mov c, ecx
    mov d, edx
  }

  *((DWORD*)(cpuid + 0)) = b;
  *((DWORD*)(cpuid + 4)) = d;
  *((DWORD*)(cpuid + 8)) = c;
  cpuid[12] = 0;
}

void get_hostname(char *name, DWORD *length) {
  GetComputerNameA(name, length);
}
#endif

void get_mac(char address[6]) {
  PIP_ADAPTER_INFO pAdapterInfo;
  DWORD dwRetVal = 0;

  ULONG ulOutBufLen = sizeof (IP_ADAPTER_INFO);
  pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC(sizeof (IP_ADAPTER_INFO));
  if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
    FREE(pAdapterInfo);
    pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC(ulOutBufLen);
  }
  GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
  memcpy(address, pAdapterInfo->Address, pAdapterInfo->AddressLength);
  FREE(pAdapterInfo);
}

int __declspec(dllexport) encrypt(char *in, char *out) {
  HANDLE in_file, out_file;
  DWORD file_size, dummy;
  char *in_data, *out_data;
  unsigned char mac[12];
  unsigned char hostname[128];
  DWORD hostname_length = 128;
  unsigned char cpuid[13];

  in_file = CreateFileA(in, GENERIC_READ, 0, 0, OPEN_ALWAYS, 0, 0);
  if(in_file == INVALID_HANDLE_VALUE) {
    fprintf(stderr, "Couldn't open file! %d\n", GetLastError());
    return 1;
  }

  out_file = CreateFileA(out, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
  if(out_file == INVALID_HANDLE_VALUE) {
    fprintf(stderr, "Couldn't open file! %d\n", GetLastError());
    return 1;
  }

  file_size = GetFileSize(in_file, NULL);
  in_data = MALLOC(file_size+1);
  out_data = MALLOC(file_size+1);

  ReadFile(in_file, in_data, file_size, &dummy, NULL);
  in_data[file_size] = 0;

  /* TODO: Encryption */
	get_mac(mac);
  fprintf(stderr, "TODO(remove_debugging) MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  get_hostname(hostname, &hostname_length);
  fprintf(stderr, "TODO(remove_debugging) Hostname: %s (%d)\n", hostname, hostname_length);

  get_cpuid(cpuid);
  fprintf(stderr, "TODO(remove_debugging) CPUID: %s\n", cpuid);

  RC4(mac, 6, hostname, hostname_length, cpuid, 12, in_data, out_data);

  WriteFile(out_file, out_data, file_size, &dummy, NULL);

  FREE(out_data);
  FREE(in_data);
  CloseHandle(out_file);
  CloseHandle(in_file);

  return 0;
}

int __declspec(dllexport) decrypt(char *in, char *out) {
  return encrypt(in, out);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
  /* No need to do anything. */
  return TRUE;
}