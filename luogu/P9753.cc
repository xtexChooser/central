#include <bits/stdc++.h>
// #include <algorithm>

#define INLINE inline
#define I int
#define U unsigned int
#define C char
#define B bool
#define YLKL(x) __builtin_expect(x, 1)
#define NLKL(x) __builtin_expect(x, 0)

// #define MAXN 2000000
#define MAXN 8010

namespace aGame {
using namespace std;
U n;
C s[MAXN + 10];

INLINE U sol() {
  U ans = 0;
  B *dp = (B *)malloc(sizeof(B) * (MAXN / 2) *
                      MAXN); // �� len (MAXN / 2) �� start (MAXN)
  for (U len = 2, lenpos = 0; len <= n; len += 2, ++lenpos) {
    B *dpl = &dp[lenpos * MAXN];
#define DP(l, st) dp[((l) * MAXN) + (st)]
    if (NLKL(len == 2)) {
      for (U start = 0; start <= n - len; ++start) {
        if (dpl[start] = s[start] == s[start + 1])
          ++ans;
      }
    } else {
      for (U start = 0; start <= n - len; ++start) {
#define SETYES                                                                 \
  {                                                                            \
    dpl[start] = 1;                                                            \
    ++ans;                                                                     \
    continue;                                                                  \
  }
        if (NLKL(len % 4 == 0)) {
          U hl = (lenpos - 1) / 2; // l / 2
          if (DP(hl, start) && DP(hl, start + (len / 2)))
            SETYES;
        }
        if (DP(lenpos - 1, start + 1) && s[start] == s[start + len - 1])
          SETYES;
        if (s[start] == s[start + 1] && DP(lenpos - 1, start + 2))
          SETYES;
        if (DP(lenpos - 1, start) && s[start + len - 1] == s[start + len - 2])
          SETYES;
#undef SETYES
      }
    }
#undef DP
  }
  return ans;
}
} // namespace aGame

int main() {
  using namespace aGame;
  using namespace std;
  std::ios::sync_with_stdio(false);
  std::cin.tie(0);

#ifndef ONLINE_JUDGE
  freopen("game.in", "r", stdin);
  freopen("game.out", "w", stdout);
#endif

  scanf("%u\n", &n);
  fread(s, n, 1, stdin);
  printf("%u\n", sol());
  return 0;
}

/*
 * 2023/10/21 15:42-16:44 ��
 * DP
 * f(start, len) (len % 2 == 0, st <= n - len)
 *   f(st, 2) = s[st] == s[st + 1] // cc
 *   f(st, l) = or {
 *     f(st, l / 2) && f(st + (l / 2), l / 2) // accabccb, l = 2 ** n
 *     f(st + 1, l - 2) && s[st] == s[st + l - 1] // acca
 *     f(st, 2) && f(st + 2, l - 2) // aaacca
 *     f(st, l - 2) && f(st + l - 2, 2) // accaaa
 *   }
 */
