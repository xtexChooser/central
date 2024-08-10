#include <bits/stdc++.h>

int main() {
  using namespace std;
  ios::sync_with_stdio(false);
  cin.tie(0);
  int n, m;
  scanf("%d%d", &n, &m);
  int a = 0, b = 0;
  for (int i = 1; i <= n; ++i) {
    for (int j = 1; j <= n; ++j) {
      int l;
      scanf("%d", &l);
      if (l == 0)
        continue;
      if (m == j)
        ++b;
      if (i == m)
        ++a;
    }
  }
  printf("%d %d %d\n", a, b, a + b);
}
