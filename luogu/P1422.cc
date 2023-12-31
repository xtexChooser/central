// #pragma GCC optimize("O3")
// #pragma G++ optimize("O3")
#include <algorithm>
#include <bits/stdc++.h>

int main() {
  int n;
  float p;
  std::cin >> n;
  int t1;
  t1 = std::min(n, 150);
  n -= t1;
  p += t1 * 0.4463f;
  t1 = std::min(n, 250);
  n -= t1;
  p += t1 * 0.4663f;
  p += n * 0.5663f;
  int m = p * 100;
  if (m % 10 >= 5)
    m += 10;
  m /= 10;

  std::cout << (m / 10);
  if (m % 10 != 0)
    std::cout << '.' << (m % 10);
  std::cout << '\n';
  return 0;
}
