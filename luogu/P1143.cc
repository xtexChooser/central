#include <bits/stdc++.h>

int main() {
  using namespace std;
  ios::sync_with_stdio(false);
  cin.tie(0);
  unsigned int n, m;
  scanf("%u\n", &n);
  unsigned int num = 0;
  while (true) {
    char c = getchar();
    if (c == '\r' || c == '\n')
      break;
    if (c >= 'A')
      c = c - 'A' + 10;
    else
      c = c ^ '0';
    num = num * n + c;
  }
  scanf("%u", &m);
  char buf[128];
  unsigned i = 0;
  while (num > 0) {
    unsigned int b = num % m;
    char c = b > 9 ? b + 'A' - 10 : b ^ '0';
    buf[++i] = c;
    num = num / m;
  }
  while (i > 0) {
    putchar(buf[i--]);
  }
  putchar('\n');
  return 0;
}
