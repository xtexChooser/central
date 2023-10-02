#include <algorithm>
#include <bits/stdc++.h>
#include <cstdio>
/*#include <cstdlib>
#include <unistd.h>*/
#define MXN 5000

int n, W[MXN + 1];
int sy, sM, sd, sh, sm, ey, eM, ed, eh, em;
long long fd, ft;

int sel[MXN + 1];

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
  int ans = 0;
  int min = 0; // 高16为W,低16为i
  int st = 0;
  while (st < ft) {
    min = 1000000000;
    int maxT = ft - st;
    for (i = 1; i <= n; i++) {
      if (!sel[i] && (W[i] << 16) < min && W[i] <= maxT) {
        min = (W[i] << 16) | i;
      }
    }
    if (min == 1000000000)
      break;
    i = min & 0xffff;
    sel[i] = 1;
    st += W[i];
    ans++;
  }
  printf("%d\n", ans);

  /*int id = getpid();
  char s[32];
  sprintf(s, "cat /proc/%d/status",id);
  system(s);*/

  return 0;
}
