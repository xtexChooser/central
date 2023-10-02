#include <bits/stdc++.h>

#define MXN 100
int a[MXN + 1];

int main() {
  using namespace std;
  int n;
  cin >> n;
  int i, k;
  for (i = 1; i <= n; ++i) {
    cin >> a[i];
    int c = 0;
    for (k = 1; k < i; ++k) {
      if (a[k] < a[i])
        c++;
    }
    cout << c << ' ';
  }
  return 0;
}
