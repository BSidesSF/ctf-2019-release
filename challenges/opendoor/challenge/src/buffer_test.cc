#include "buffer.h"
#include <stdio.h>
int main(int argc, char **argv) {
  auto v = new std::vector<std::uint8_t>();
  v->push_back(0x41);
  v->push_back(0x42);
  v->push_back(0x43);
  v->push_back(0x44);
  auto b = opendoor::Buffer(std::shared_ptr<std::vector<std::uint8_t>>(v));
  printf("%x\n", b.read<uint32_t>());
  b.write(0x55);
  printf("%x\n", b.read<uint32_t>());
  auto s = std::string("hello world");
  b.write(&s);
  printf("\"%s\"\n", b.read<std::string>().c_str());
}
