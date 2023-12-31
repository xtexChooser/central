#include <bits/stdc++.h>

#define MAXM 10001
#define MAXN 101

int dp[MAXN][MAXM] = {0}, a[MAXM];
int n, m;

int main() {
  using namespace std;
  ios::sync_with_stdio(false);
  cin.tie(0);

  scanf("%d%d", &n, &m);
  for (int i = 1; i <= n; ++i)
    scanf("%d", &a[i]);
  for (int i = 1; i <= n; ++i)
    for (int j = 1; j <= m; ++j) {
      int p = a[i];
      int *lr = dp[i - 1];
      if (j < p)
        dp[i][j] = lr[j];
      else if (j == p)
        dp[i][j] = lr[j] + 1;
      else if (j > p)
        dp[i][j] = lr[j] + lr[j - p];
    }
  printf("%d\n", dp[n][m]);
  return 0;
}
