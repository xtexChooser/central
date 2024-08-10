#include <bits/stdc++.h>
#include <limits>

int mp[1005][1005];

int main() {
  using namespace std;
  ios::sync_with_stdio(false);
  cin.tie(0);

  int N, M;
  scanf("%d%d", &N, &M);
  char ch;
  for (int i = 1; i <= N; ++i)
    for (int j = 1; j <= M; ++j) {
      while ((ch = getchar()) != 'R' && ch != 'F')
        ;
      mp[i][j] = (ch == 'F');
    }

  for (int i = 0; i <= N; ++i) {
    int sum = 0;
    int *iptr = &mp[i][0];
    for (int j = M; j > 0; --j) {
      int *ptr = iptr + j;
      int k = *ptr;
      if (k) {
        *ptr = ++sum;
      } else {
        sum = 0;
      }
    }
  }

  // 这里不能用 int min......
  int ans = 0;
  for (int i = 1; i <= N; ++i)
    for (int j = 1; j <= M; ++j) {
      if (mp[j][i]) {
        int w = numeric_limits<int>::max();
        int h = 1;
        for (int k = j; k <= M; ++k) {
          int wd = mp[k][i];
          if (wd) {
            w = min(w, wd);
            ans = max(ans, w * h);
            ++h;
          } else {
            break;
          }
        }
      }
    }
  printf("%d\n", 3 * ans);

  return 0;
}
