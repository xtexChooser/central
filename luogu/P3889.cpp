#include <algorithm>
#include <bits/stdc++.h>
#include <limits>
// #define uint unsigned int
#define LKL(x) __builtin_expect(x, 1)
#define ULKL(x) __builtin_expect(x, 0)
#define FOR(v, f, t) for (auto v = f; v <= t; ++v)

#define MAXN 100005

namespace sushi {
using namespace std;
typedef unsigned int uint;

int fastgcd(int a, int b) {
  if (ULKL(a == b))
    return a;
  if (ULKL(a == 0))
    return b;
  if (ULKL(b == 0))
    return a;
  if (ULKL(a % 2 == 0 && b % 2 == 0))
    return 2 * fastgcd(a >> 1, b >> 1);
  else if (a % 2 == 0)
    return fastgcd(a >> 1, b);
  else if (b % 2 == 0)
    return fastgcd(a, b >> 1);
  else
    return fastgcd(abs(a - b), min(a, b));
}

uint fastgcd(uint a, uint b) {
  if (ULKL(a == b))
    return a;
  if (ULKL(a == 0))
    return b;
  if (ULKL(b == 0))
    return a;
  if (ULKL(a & 1 == 0 && b & 1 == 0))
    return fastgcd(a >> 1, b >> 1) << 1;
  else if (a & 1 == 0)
    return fastgcd(a >> 1, b);
  else if (b & 1 == 0)
    return fastgcd(a, b >> 1);
  else
    return fastgcd((a > b) ? (a - b) : (b - a), min(a, b));
}

uint N, M;
uint a[MAXN], maxa = 0;
set<uint> comm[MAXN];

inline void init() {
  FOR(i, 1, N) {
    uint av = a[i];
    maxa = max(maxa, av);
    uint sroot = sqrt(av);
    FOR(j, 2, sroot) if (av % j == 0) {
      comm[j].insert(i);
      comm[av / j].insert(i);
    }
    comm[av].insert(i);
  }
  //		for (auto i = maxa; i > 0; -- i)
  //			for (auto v : comm[i])
  //				printf("%u %u\n", i, a[v]);
}

inline uint sol(uint l, uint r) {
  if (ULKL(l == 1 && r == N))
    return 0;
  uint ans = 1;
  //		FOR(i, l, r) {
  //			FOR(j, 1, l - 1)
  //				ans = max(ans, fastgcd(a[i], a[j]));
  //			FOR(j, r + 1, N)
  //				ans = max(ans, fastgcd(a[i], a[j]));
  //		}
  for (auto i = maxa; i > 0; --i) {
    auto &commi = comm[i];
    if (commi.empty())
      continue;
    auto lb = commi.lower_bound(l);
    if (lb != commi.end() && *lb <= r) {
      if (lb != commi.begin()) {
        ans = max(ans, i);
        continue;
      }
      do {
        ++lb;
      } while (*lb <= r && lb != commi.end());
      if (lb != commi.end()) {
        ans = max(ans, i);
        continue;
      }
    }
  }
  return ans;
}
} // namespace sushi

int main() {
  using namespace sushi;
  ios::sync_with_stdio(false);
  cin.tie(0);

  scanf("%u", &N);
  FOR(i, 1, N) scanf("%u", a + i);
  init();

  scanf("%u", &M);
  FOR(i, 1, M) {
    uint l, r;
    scanf("%u%u", &l, &r);
    printf("%u\n", sol(l, r));
  }

  return 0;
}
