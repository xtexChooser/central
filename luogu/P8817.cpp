#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <vector>

using namespace std;

int n, m, k;
#define MAXN 2501
#define MAXM 10001
int s[MAXN];       // 点分数
int d[MAXN][MAXN]; // 距离+1

int main() {
  int i, x, y, z, a;
  clock_t time = clock();

  cin >> n >> m >> k;
  for (i = 1 + 1; i <= n; i++) {
    cin >> s[i];
  }
  for (i = 1; i <= m; i++) {
    cin >> x >> y;
    d[x][y] = 1;
    d[y][x] = 1;
  }
  for (a = 1; a <= k; a++) {
    for (x = 1; x <= n; x++) {
      for (y = 1; y <= n; y++) {
        if (d[x][y] != a)
          continue;
        // x -> y 间存在路径
        int r = k + 1 - a;
        for (z = 1; z <= n; z++) {
          int dyz = d[y][z];
          if (!dyz || dyz >= r)
            continue;
          // y -> z
          int dxz = d[x][z];
          d[x][z] = min(dxz == 0 ? 1000 : dxz, a + dyz);
        }
      }
    }
  }
  int max = -1;
  for (x = 2; x <= n; x++) {
    for (y = 2; y <= n; y++) {
      if (x == y || !d[x][y])
        continue;
      for (z = 2; z <= n; z++) {
        if (x == z || y == z || !d[y][z])
          continue;
        for (a = 2; a <= n; a++) {
          if (x == a || y == a || z == a || !d[z][a] || !d[a][1])
            continue;
          int sum = s[x] + s[y] + s[z] + s[a];
          max = std::max(max, sum);
          if ((clock() - time) > (int)(1.99f * CLOCKS_PER_SEC)) {
            cout << max << "\n";
            return 0;
          }

          // cout << d[x][y] << " ";
        }
      }
    }
  }
  cout << max << "\n";
  return 0;
}