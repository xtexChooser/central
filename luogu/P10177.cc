#include <bits/stdc++.h>
#include <limits>

int readint() {
  int v = 0;
  char ch = getchar();
  while (!isdigit(ch))
    ch = getchar();
  while (__builtin_expect(isdigit(ch), 1)) {
    v = (v << 3) + (v << 1) + (ch ^ '0');
    ch = getchar();
  }
  return v;
}

int a[1000005];

int main() {
  using namespace std;
  ios::sync_with_stdio(false);
  cin.tie(0);
  int n = readint();
  for (int i = 0; i < n; ++i)
    a[i] = readint();
  std::sort(a, a + n);
  int mx = numeric_limits<int>::min();
  for (int i = 0; i < n - 1; ++i) {
    mx = max(mx, a[i] - a[i + 1]);
  }
  printf("%d\n", mx);
  return 0;
}
