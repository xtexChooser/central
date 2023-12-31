#include <bits/stdc++.h>

int main() {
  int s, x;
  std::cin >> s >> x;
  /*char ans = (((x * x) - (s * s)) >= (3.5*3.5)) ? 'y' : 'n';
  std::cout << ans;*/
  float spd = 7;
  float m = 0;
  while (m < s - x) {
    m += spd;
    spd *= 0.98;
  }
  m += spd;
  std::cout << ((m > s + x) ? 'n' : 'y') << '\n';
  return 0;
}
