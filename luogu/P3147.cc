#include <algorithm>
#include <bits/stdc++.h>

#define u unsigned
#define ull unsigned long long
#define ll long long
#define MAXN 280000

namespace thisisanamespace {
using namespace std;
u N;
u n[MAXN];
u f[MAXN][MAXN];
u sol() {
  for (u i = 1; i <= N; ++i)
    n[i] += n[i - 1];
  for (u i = 1; i < N; ++i)
    for (u j = i; j <= N; ++j)
      for (u k = i; k <= j; ++k)
        f[i][j] = max(f[i][j], f[i][k] + f[k + 1][j]);
  return f[1][N];
}
} // namespace thisisanamespace

int main() {
  using namespace thisisanamespace;
  ios::sync_with_stdio(false);
  cin.tie(0);
  scanf("%u", &N);
  for (u i = 1; i <= N; ++i)
    scanf("%u", n + i);
  printf("%u\n", sol());
  return 0;
}
