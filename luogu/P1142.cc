#include <algorithm>
#include <bits/stdc++.h>
#include <limits>
#include <vector>

class Point {
public:
  int x, y;
};

int main() {
  using namespace std;
  int n;
  scanf("%d", &n);
  vector<Point> p(n);
  int ans = numeric_limits<int>::min();
  for (int i = 0; i < n; ++i)
    scanf("%d%d", &p[i].x, &p[i].y);
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j) {
      if (__builtin_expect(i == j, 0))
        continue;
      double dx = p[i].x - p[j].x;
      double dy = p[i].y - p[j].y;
      int online = 2;
      for (int k = 0; k < n; ++k) {
        if (__builtin_expect(k == i || k == j, 0))
          continue;
        double dx1 = p[i].x - p[k].x;
        double dy1 = p[i].y - p[k].y;
        if (__builtin_expect(dx1 / dy1 == dx / dy, 0))
          ++online;
      }
      ans = max(ans, online);
    }
  printf("%d", ans);
  return 0;
}
