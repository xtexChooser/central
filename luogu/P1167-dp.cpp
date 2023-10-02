/**
 * 呃呃呃原来是模拟啊
 * 原来不用dp啊（dp在时间长的时候会OOM
 */

#include <algorithm>
#include <bits/stdc++.h>
#include <cstdio>
/*#include <cstdlib>
#include <unistd.h>*/
#define MXN 5000
#define MXT 100000

int n, W[MXN + 1];
int sy, sM, sd, sh, sm, ey, eM, ed, eh, em;
long long fd, ft;

int dp[2][MXT + 1];

using namespace std;

int days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

int main() {
  std::ios::sync_with_stdio(false);
  scanf("%d\n", &n);
  int i;
  for (i = 1; i <= n; i++) {
    scanf("%d\n", &W[i]);
  }
  scanf("%d-%d-%d-%d:%d\n", &sy, &sM, &sd, &sh, &sm);
  scanf("%d-%d-%d-%d:%d", &ey, &eM, &ed, &eh, &em);

  while (sy != ey || sM != eM) {
    fd += days[sM];
    if (((sy % 4 == 0 && sy % 100 != 0) || (sy % 400 == 0)) && sM == 2)
      fd++; // 闰年
    sM++;
    if (sM == 13) {
      sM = 1;
      sy++;
    }
  }
  fd += ed - sd;
  ft = (((fd * 24) + (eh - sh)) * 60) + (em - sm);

  // dp
  unsigned long long x, y;
  int row = 0;
  for (x = 1; x <= n; x++) {
    dp[row][0] = dp[1-row][0] + (W[x] == 0 ? 1 : 0);
    for (y = 1; y <= ft; y++) {
      if (y < W[x]) {
        dp[row][y] = dp[1-row][y];
      } else {
        dp[row][y] = max(dp[1-row][y], dp[1-row][y - W[x]] + 1);
      }
    }
    row = 1 - row;
  }
  /*for (x = 0; x <= n; x++) {
    for (y = 0; y <= ft; y++) {
      printf("%d ", dp[row][y]);
    }
    printf("\n");
  }*/
  printf("%d\n", dp[row][ft]);

  /*int id = getpid();
  char s[32];
  sprintf(s, "cat /proc/%d/status",id);
  system(s);*/

  return 0;
}
