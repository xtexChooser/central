#include <algorithm>
#include <bits/stdc++.h>

int main() {
  using namespace std;
  int x, n;
  cin >> x >> n;
  int d = 0;
  // 对齐到周一
  if (x > 5) {
    n -= (8 - x);
    x = 1;
  }
  if (x != 1) {
    int rem = 6 - x;
    d += 250 * rem;
    n -= (8 - x);
    x = 1;
  }
  while (n >= 7) {
    n -= 7;
    d += 5 * 250;
  }
  d += min(n, 5) * 250;
  cout << d;
  return 0;
}
