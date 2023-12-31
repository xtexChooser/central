#include <algorithm>
#include <bits/stdc++.h>
#include <limits>
#include <vector>

int main() {
  using namespace std;
  unsigned n, m;
  scanf("%u%u", &n, &m);
  --m;
  vector<int> a;
  for (auto i = 0; i < n; ++i) {
    int v;
    scanf("%d", &v);
    a.insert(lower_bound(a.begin(), a.end(), v), v);
  }
  int ans = numeric_limits<int>::max();
  for (auto i = 0; i < n - m; ++i) {
    int k = a[i + m] - a[i];
    ans = min(ans, k);
  }
  printf("%d\n", ans);
  return 0;
}
