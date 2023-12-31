#include <stdio.h>

__uint128_t S(unsigned long long n) {
  __uint128_t a, i;
  a = 1;
  i = 1;
  for (; i <= n; i++) {
    a *= i;
  }
  return a;
}

int main() {
  unsigned long long n, i = 1;
  __int128_t sum = 0;
  scanf("%lld", &n);
  for (; i <= n; i++) {
    sum += S(i);
  }
  char buf[100];
  int ptr = 98;
  buf[99] = 0;
  while (sum > 0) {
    buf[ptr] = '0' + (((unsigned int)sum) % 10);
    sum /= 10;
    ptr--;
  }
  printf("%s\n", &buf[ptr + 1]);
  return 0;
}
