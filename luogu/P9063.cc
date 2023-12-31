#include <bits/stdc++.h>

int main() {
  int T;
  scanf("%d", &T);
  unsigned long long n;
  for (auto i = 0; i < T; ++i) {
    scanf("%llu", &n);
    printf("%s\n", (n & 1) ? "Yes" : "No");
  }
  return 0;
}
