#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "stringops.h"

#define STRLEN_TEST(x) assert(strlen((x)) == strlen_fast((x)))

int main(int argc, char **argv) {
  STRLEN_TEST("foo");
  STRLEN_TEST("this is a longer string to test");
  STRLEN_TEST("the quick brown fox jumped over the lazy dog");
  assert(strcmp_fast("foo", "foo") == 0);
  assert(strcmp_fast("the quick brown fox jumped over the lazy dog",
        "the quick brown fox jumped over the lazy dog") == 0);
  assert(strcmp("foo", "bar") > 0);
  assert(strcmp_fast("foo", "bar") == 1);
  assert(strcmp_fast("foo", "fof") == 3);
  //assert(strcmp_fast("bar", "foo") == -1);
  //assert(strcmp_fast("the quick brown fox jumped over the lazy dog",
  //      "the quick brown fox jumped over the smart dog") == -5);
  printf("strcmp foo bar == %d\n", strcmp_fast("foo", "bar"));
  printf("strcmp bar foo == %d\n", strcmp_fast("bar", "foo"));
  printf("strcmp foo fof == %d\n", strcmp_fast("foo", "fof"));
  printf("strcmp smart lazy == %d\n", strcmp_fast(
        "the quick brown fox jumped over the lazy dog",
        "the quick brown fox jumped over the smart dog"));
  return 0;
}
