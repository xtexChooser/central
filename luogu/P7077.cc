#include <algorithm>
#include <bits/stdc++.h>

// #define INLINE __attribute__((always_inline))
#define INLINE inline
#define LIKELY(x) __builtin_expect(x, 1)
#define UNLIKELY(x) __builtin_expect(x, 0)
#define FOR(var, begin, end) for (uint var = begin; var <= end; ++var)

#define MAXN 100001
#define MOD 998244353
#define uint unsigned int
#define valT "%llu"
#define val unsigned long long

namespace call {
using namespace std;

class Fn {
public:
  uint type;
  INLINE constexpr Fn(uint type) : type(type){};
  virtual void run() = 0;
};

class Fn1 : public Fn {
public:
  val *idx;
  val add;
  INLINE constexpr Fn1(val *index, val add) : Fn(1), idx(index), add(add){};
  void run();
};

class Fn2 : public Fn {
public:
  val mul;
  INLINE constexpr Fn2(val mul) : Fn(2), mul(mul){};
  void run();
};

class Fn3 : public Fn {
public:
  uint *funcs;
  INLINE Fn3(uint *funcs) : Fn(3), funcs(funcs){};
  INLINE ~Fn3() { free(funcs); };
  void run();
};

uint n, m;
val value[MAXN];
Fn *fns[MAXN];
Fn3 *entry;

INLINE Fn *readFn();
INLINE Fn1 *readFn1();
INLINE Fn2 *readFn2();
INLINE Fn3 *readFn3();
INLINE void mergeFn3();

Fn *readFn() {
  uint T;
  scanf("%u", &T);
  switch (T) {
  case 1:
    return readFn1();
  case 2:
    return readFn2();
  case 3:
    return readFn3();
  }
  throw "?";
}

Fn1 *readFn1() {
  uint P;
  val V;
  scanf("%u" valT, &P, &V);
  return new Fn1(value + P, V);
}

Fn2 *readFn2() {
  val V;
  scanf(valT, &V);
  return new Fn2(V);
}

Fn3 *readFn3() {
  uint C, *g;
  scanf("%u", &C);
  g = (uint *)malloc(sizeof(uint) * (C + 1));
  FOR(i, 0, C - 1) scanf("%u", g + i);
  g[C] = 0;
  return new Fn3(g);
}

void Fn1::run() { *idx = (*idx + add) % MOD; }

void Fn2::run() { FOR(i, 1, n) value[i] = (value[i] * mul) % MOD; }

void Fn3::run() {
  for (uint *fn = funcs; *fn != 0; ++fn)
    fns[*fn]->run();
}

void mergeFn3() {
  FOR(i, 1, m) if (UNLIKELY(fns[i]->type == 3)) {
    Fn3 *fn = static_cast<Fn3 *>(fns[i]);
    Fn *fn1 = fns[fn->funcs[0]];
    if (LIKELY(fn1->type != 2))
      continue;
    val mul = static_cast<Fn2 *>(fn1)->mul;
    for (uint *f = fn->funcs + 1; *f != 0; ++f) {
      Fn *fnf = fns[*f];
      if (LIKELY(fnf->type != 2)) {
        mul = 0;
        break;
      } else {
        mul = (mul * static_cast<Fn2 *>(fnf)->mul) % MOD;
      }
    }
    if (LIKELY(mul == 0))
      break;
    // delete fn;
    fns[i] = new Fn2(mul);
  }
}
} // namespace call

int main() {
  using namespace call;
  std::ios::sync_with_stdio(false);
  std::cin.tie(0);

#ifndef ONLINE_JUDGE
  freopen("call.in", "r", stdin);
  freopen("call.out", "w", stdout);
#endif

  scanf("%u", &n);
  FOR(i, 1, n) scanf(valT, value + i);
  value[n + 1] = 0;
  scanf("%u", &m);
  FOR(i, 1, m) fns[i] = readFn();
  mergeFn3();
  entry = readFn3();

  entry->run();
  FOR(i, 1, n) printf(valT " ", value[i]);
  putchar('\n');

  return 0;
}
