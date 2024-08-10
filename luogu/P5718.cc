#include <bits/stdc++.h>

int main() {
  using namespace std;
  ios::sync_with_stdio(false);
  cin.tie(0);
  unsigned int n;
  scanf("%u", &n);
  unsigned int m = 1005;
  for (unsigned int i = 0; i < n; ++i) {
    unsigned int v;
    scanf("%u", &v);
    m = min(m, v);
  }
  printf("%u\n", m);
  return 0;
}
