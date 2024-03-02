#include <bits/stdc++.h>

int main() {
  unsigned int N;
  scanf("%u", &N);
  unsigned int ans;
  ans = (N * (N + 1) / 2 * (N + 2));
  printf("%u\n", ans);
  return 0;
}
