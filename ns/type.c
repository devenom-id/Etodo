#include <stdio.h>
#include <ctype.h>
#include <string.h>

void ch() {
  char ch = '$';
  printf("is alnum: %d\n", isalnum(ch));
  printf("is cntrl: %d\n", iscntrl(ch));
  printf("is graph: %d\n", isgraph(ch));
  printf("is ascii: %d\n", isascii(ch));
  printf("is punct: %d\n", ispunct(ch));
}

void len() {
  char* buff = NULL;
  printf("%d\n", strlen(buff));
}

int main() {
  len();
}
