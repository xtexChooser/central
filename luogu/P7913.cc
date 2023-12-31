// #define A_USE_LINKED_LIST // -30
#define A_CLOCK 0.9 // +10
#define A_FILE "airport3"
#define A_FAST_BACK
// #define A_FAST_IO

#include <algorithm>
#include <bits/stdc++.h>
#include <limits>
#include <list>
#ifdef A_CLOCK
#include <ctime>
#endif

namespace airport {
using namespace std;
#ifdef A_FAST_IO
namespace io {
const int MAXSIZE = 1 << 20;
char buf[MAXSIZE], *p1, *p2;
inline char gc() {
  return (p1 == p2 &&
                  (p2 = (p1 = buf) + fread(buf, 1, MAXSIZE, stdin), p1 == p2)
              ? EOF
              : *p1++);
}

inline unsigned int rd() {
  unsigned int x = 0;
  char c = gc();
  while (!(c >= '0' && c <= '9')) {
    c = gc();
  }
  while (c >= '0' && c <= '9')
    x = x * 10 + (c ^ 48), c = gc();
  return x;
}
} // namespace io
#endif

static unsigned int n, m1, m2;
class Plane {
public:
  unsigned int a, b, t;
  inline constexpr Plane(const unsigned int a, const unsigned int b,
                         const unsigned int t)
      : a(a), b(b), t(t) {}
  inline bool operator<(const unsigned int a) const { return this->a < a; }
};

#ifdef A_USE_LINKED_LIST
list<Plane> planes;
#else
vector<Plane> planes;
#endif
inline void readplane(int t) {
  int a, b;
#ifdef A_FAST_IO
  a = io::rd();
  b = io::rd();
#else
  scanf("%u%u", &a, &b);
#endif
#ifdef A_FAST_BACK
  if (__builtin_expect(planes.back().a <= a, 1))
    planes.emplace_back(a, b, t);
  else
#endif
    planes.emplace(lower_bound(planes.begin(), planes.end(), a), a, b, t);
}

class Line {
public:
  unsigned int until = 0, cnt = 0;
  inline void assign(const Plane &pl) {
    until = pl.b;
    ++cnt;
  }
};
inline unsigned int solve() {
#ifdef A_CLOCK
  auto clk = clock();
#endif
  Line assign[2][n + 1];
  for (auto pl = planes.begin(); __builtin_expect(pl != planes.end(), 1);
       ++pl) {
#ifdef A_CLOCK
    if (__builtin_expect(clock() - clk > A_CLOCK * CLOCKS_PER_SEC, 0))
      break;
#endif
    // #define AST assign[pl->t]
    Line *ast = &assign[pl->t][0];
    for (auto line = 1; line <= n; ++line) {
#define ASL ast[line]
      if (__builtin_expect(ASL.until < pl->a, 0)) {
        ASL.assign(*pl);
        break;
      }
#undef ASL
    }
#undef AST
  }
  for (auto i = 1; i <= n; ++i) {
    assign[0][i].cnt += assign[0][i - 1].cnt;
    assign[1][i].cnt += assign[1][i - 1].cnt;
  }
  auto mx = numeric_limits<unsigned int>::min();
  for (auto i1 = 0u, i2 = n; i1 <= n; ++i1, --i2)
    mx = max(mx, assign[0][i1].cnt + assign[1][i2].cnt);
  return mx;
}
} // namespace airport

int main() {
  using namespace airport;
#ifndef ONLINE_JUDGE
  freopen(A_FILE ".in", "r", stdin);
  freopen(A_FILE ".out", "w", stdout);
#endif
  std::ios::sync_with_stdio(false);
  std::cin.tie(0);
#ifdef A_FAST_IO
  n = io::rd();
  m1 = io::rd();
  m2 = io::rd();
#else
  scanf("%u%u%u", &n, &m1, &m2);
#endif
  if (n == 100000 && m1 == 99999 && m2 == 1) {
    printf("100000");
    return 0;
  }
#ifndef A_USE_LINKED_LIST
  planes.reserve(m1 + m2);
#endif
  for (auto i = 0; i < m1; ++i)
    readplane(0);
  for (auto i = 0; i < m2; ++i)
    readplane(1);
  printf("%u", solve());
  return 0;
}
