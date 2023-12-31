#include <bits/stdc++.h>
#include <numeric>

#define MAXN 100010
#define C_USE_VEC

#define EP __builtin_expect

namespace P8211 {
using namespace std;
typedef unsigned int uint;

template <typename V> struct dsu {
  vector<V> pa;

  inline explicit dsu(V size) : pa(size) { iota(pa.begin(), pa.end(), 0); }
  //	inline V find(V x) { return pa[x] == x ? x : find(pa[x]); }
  inline V find(V x) { return pa[x] == x ? x : pa[x] = find(pa[x]); }
  inline void unite(V x, V y) { pa[find(x)] = find(y); }
};

struct fastMemAllocator {
  size_t rem = 0;
  void *ptr = nullptr;
  static const uint BATCH = 1024 * 1024 * 16;
  inline void *alloc(size_t sz) {
    if (rem < sz) {
      rem = BATCH;
      ptr = malloc(BATCH);
    }
    rem -= sz;
    void *ret = ptr;
    ptr = (void *)((size_t)ptr + sz);
    return ret;
  }
  template <typename T> inline T *get() {
    return static_cast<T *>(alloc(sizeof(T)));
  }
} fastalloc;

struct brks {
  uint cnt, need;
  //	inline bool operator<(const brks o) const {
  //		return cnt < o.cnt;
  //	}
  inline bool operator<(const brks *o) const { return cnt < o->cnt; }
  inline bool operator<(const brks &o) const { return cnt < o.cnt; }
};

uint maxA = 0, maxB = 0, max0A = 0;
#ifdef C_USE_VEC
vector<brks> allbrk;
#else
list<brks> allbrk;
#endif

inline void addbrks(uint cnt, uint need) {
  if (need == 0)
    max0A = max(max0A, cnt);

  maxA = max(maxA, cnt);
  maxB = max(maxB, need);
  brks b;
  b.cnt = cnt;
  b.need = need;
  allbrk.insert(lower_bound(allbrk.begin(), allbrk.end(), b), b);
}

inline uint cntbrks(uint f, uint t, bool *outsucc) {
  uint ans = 0;
  brks stub;
  stub.cnt = f + 1;
  auto p = lower_bound(allbrk.begin(), allbrk.end(), &stub);
  //	printf("%u %u %u\n", p, allbrk.end(), p->cnt);
  bool succ = (p != allbrk.end() && p->cnt <= t);
  *outsucc = succ;
  while (succ) {
    ans += p->need;
    ++p;
    succ = p != allbrk.end() && p->cnt <= t;
  }
  return ans;
}

inline uint asks(int d) {
  uint ans = 0, m = 0;
  //	for (auto a=allbrk.begin(); a != allbrk.end(); ++a) {
  //		printf("@%u ", &a);
  //		printf("%u\n", a->cnt);
  //	}
  while (d > 0) {
    ++ans;
    uint oldm = m;
    m += d;
    if (EP(oldm > (--allbrk.end())->cnt, 0)) {
      if (m < max0A) {
        uint n = (max0A - m);
        if (n % d == 0)
          ans += n / d;
        else
          ans += (n / d + 1);
      }
      break;
    } else {
      bool succ;
      uint c = cntbrks(oldm, m, &succ);
      //			printf("%u->%u step %u brk %u succ %u\n", oldm,
      //m, d, c, succ);
      d -= c;
      if (!succ)
        break;
    }
  }
  return ans;
}

}; // namespace P8211

int main() {
  using namespace P8211;
  ios::sync_with_stdio(false);
  cin.tie(0);

  uint T;
  scanf("%u", &T);
  for (uint i = 0; i < T; ++i) {
    uint op;
    scanf("%u", &op);
    if (op == 1) {
      uint a, b;
      scanf("%u%u", &a, &b);
      addbrks(a, b);
    } else {
      uint d;
      scanf("%u", &d);
      printf("%u\n", asks(d));
    }
  }
  return 0;
}
