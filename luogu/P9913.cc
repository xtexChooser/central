#include <bits/stdc++.h>

int main() {
  int T;
  scanf("%u", &T);
  for (int i = 0; i < T; ++i) {
    int n;
    scanf("%u", &n);
    printf(__builtin_popcount(n) == 1 ? "Yes\n" : "No\n");
  }
  return 0;
}
