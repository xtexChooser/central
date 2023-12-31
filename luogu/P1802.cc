#include <algorithm>
#include <bits/stdc++.h>
#define u unsigned
#define ull unsigned long long
#define MAXN 1005
#define MAXX 1005

u n, x;
ull lose[MAXN], win[MAXN], use[MAXN], dp[MAXN][MAXX];

int main() {
  using namespace std;
  scanf("%u%u", &n, &x);
  for (u i = 1; i <= n; ++i)
    scanf("%llu%llu%llu", lose + i, win + i, use + i);
  for (u i = 1; i <= n; ++i) {
    for (u j = 0; j <= x; ++j) {
      if (j < use[i]) {
        dp[i][j] = dp[i - 1][j] + lose[i];
      } else {
        dp[i][j] = max(dp[i - 1][j] + lose[i], dp[i - 1][j - use[i]] + win[i]);
      }
    }
  }
  printf("%llu\n", 5ll * dp[n][x]);
  return 0;
}
