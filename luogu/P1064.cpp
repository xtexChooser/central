#include <algorithm>
#include <bits/stdc++.h>

#define u unsigned
#define ull unsigned long long
#define MAXM 65
#define MAXN 32005

u n, m;
u v[MAXM][3], p[MAXM][3], f[MAXN];

int main() {
  using namespace std;
  ios::sync_with_stdio(false);
  cin.tie(0);

  scanf("%u%u", &n, &m);
  for (u i = 1; i <= m; ++i) {
    u V, P, Q;
    scanf("%u%u%u", &V, &P, &Q);
    if (Q == 0) {
      v[i][0] = V;
      p[i][0] = P;
    } else {
      if (v[Q][1] == 0) {
        v[Q][1] = V;
        p[Q][1] = P;
      } else {
        v[Q][2] = V;
        p[Q][2] = P;
      }
    }
  }
  for (u i = 1; i <= m; ++i) {
    u *vi = v[i], *pi = p[i];
    for (int j = n; j >= 0; --j) {
      u *fj = f + j;
      if (j >= vi[0])
        *fj = max(*fj, f[j - vi[0]] + vi[0] * pi[0]);
      if (j >= vi[0] + vi[1])
        *fj = max(*fj, f[j - vi[0] - vi[1]] + vi[0] * pi[0] + vi[1] * pi[1]);
      if (j >= vi[0] + vi[2])
        *fj = max(*fj, f[j - vi[0] - vi[2]] + vi[0] * pi[0] + vi[2] * pi[2]);
      if (j >= vi[0] + vi[1] + vi[2])
        *fj = max(*fj, f[j - vi[0] - vi[1] - vi[2]] + vi[0] * pi[0] +
                           vi[1] * pi[1] + vi[2] * pi[2]);
    }
  }
  printf("%u\n", f[n]);
  return 0;
}
