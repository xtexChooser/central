#include <bits/stdc++.h>

#define ll long long
#define u unsigned
#define ull u ll

#define gc getchar
inline ull read() {
  ull v = 0;
  char ch = gc();
  while (!isdigit(ch))
    ch = gc();
  for (; isdigit(ch); ch = gc())
    v = (v << 3) + (v << 1) + (ch ^ '0');
  return v;
}

namespace aGarden {
using namespace std;

#define MAXN 1001
#define ANSMOD 998244353
#define ansmod(x) ((x) % ANSMOD)

class State {
public:
  bool a[MAXN][MAXN];
  int sum[MAXN][MAXN];
  u n, m, c, f;
  inline void sol();
} s;

inline void State::sol() {
  for (u i = 0; i < n; ++i) {
    bool *au = a[i];
    for (u j = 0; j < m; ++j)
      au[j] = (bool)(gc() & 1);
    gc(); // NL
    int s = -1;
    int *su = sum[i];
    for (int j = m - 1; j >= 0; --j) {
      if (au[j]) {
        su[j] = -1;
        s = -1;
      } else {
        su[j] = ++s;
      }
    }
  }
  //	for (u i = 0; i < n; ++ i) {
  //		for (u j = 0; j < m; ++ j)
  //			printf("%d ", sum[i][j]);
  //		putchar('\n');
  //	}
  int ansc = 0, ansf = 0;
  for (u x = 0; x < m; ++x) {
    int ccont = 0, fcont = 0;
    for (u y = 0; y < n; ++y) {
      int suy = sum[y][x];
      if (suy == -1) {
        ccont = 0;
        fcont = 0;
        continue;
      }
      int fsuy = y != 0 ? sum[y - 1][x] : 0;
      if (fsuy == -1)
        fsuy = 0;
      int newc = ccont * suy;
      ansc = ansmod(ansc + newc);
      ansf = ansmod(ansf + fcont);

      ccont = ansmod(ccont + fsuy);
      fcont = ansmod(fcont + newc);
    }
  }
  printf("%d %d\n", c ? ansc : 0, f ? ansf : 0);
}

} // namespace aGarden

int main() {
  using namespace aGarden;
  u T = read();
  read(); // id
  while (T) {
    memset(&s, 0, sizeof(s));
    s.n = read();
    s.m = read();
    s.c = read();
    s.f = read();
    s.sol();
    --T;
  }
  return 0;
}
