#include <bits/stdc++.h>

int tbl[2][13] = {{0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0},
                  {1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1}};

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(0);
  unsigned int T;
  scanf("%d", &T);
  for (unsigned int i = 0; i < T; ++i) {
    int a;
    scanf("%d", &a);
    printf("%d\n", tbl[i % 2][((a - 1) % 13)]);
  }
  return 0;
}
