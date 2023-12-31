#include <bits/stdc++.h>

int main() {
  using namespace std;
  float d;
  cin >> d;
  float st = 2;
  int ans = 0;
  while (d >= 0) {
    d -= st;
    st *= 0.98;
    ans++;
  }
  cout << ans;
  return 0;
}
