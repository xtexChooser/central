#include <bits/stdc++.h>
#include <ctime>
#include <set>
#include <vector>
#define MAXN 500000
#define F(v, b, e) for (int v = b; v <= e; ++v)

namespace Qgalaxy {
using namespace std;

class st {
public:
  set<int> avlout;
  vector<int> allin;
};

st S[MAXN];
int n;

inline void init(int u, int v) {
  S[u].avlout.insert(v);
  S[v].allin.push_back(u);
}
inline void t1(int u, int v) { // 拆
  S[u].avlout.erase(v);
}
inline void t2(int u) { // 拆入
  vector<int>::iterator iter = S[u].allin.begin();
  vector<int>::iterator end = S[u].allin.end();
  for (; iter != end; ++iter) {
    int v = *iter;
    S[v].avlout.erase(u);
  }
}
inline void t3(int u, int v) { // 连
  S[u].avlout.insert(v);
}
inline void t4(int u) { // 修入
  vector<int>::iterator iter = S[u].allin.begin();
  vector<int>::iterator end = S[u].allin.end();
  for (; iter != end; ++iter) {
    int v = *iter;
    S[v].avlout.insert(u);
  }
}

namespace fr {
char buf[1 << 20], *p1, *p2;
inline char gc() {
  return (p1 == p2 &&
                  (p2 = (p1 = buf) + fread(buf, 1, 1 << 20, stdin), p1 == p2)
              ? EOF
              : *p1++);
}
inline int gi() {
  int x = 0; //, w = 1;
  char ch = 0;
  while (ch < '0' || ch > '9') {
    // if (ch == '-') w = -1;
    ch = gc();
  }
  while (ch >= '0' && ch <= '9') {
    x = x * 10 + (ch - '0');
    ch = gc();
  }
  return x; // * w;
}
} // namespace fr

inline void test() {
  F(i, 1, n) {
    if (S[i].avlout.empty() || ++S[i].avlout.begin() != S[i].avlout.end()) {
      // if (S[i].avlout.size() != 1) {
      fwrite("NO\n", 3, 1, stdout);
      return;
    }
  }
  fwrite("YES\n", 4, 1, stdout);
}
} // namespace Qgalaxy

int main() {
  using namespace std;
#ifndef ONLINE_JUDGE
  freopen("galaxy4.in", "r", stdin);
  freopen("galaxy4.out", "w", stdout);
#endif
  std::ios::sync_with_stdio(false);
  std::cin.tie(0);

  using namespace Qgalaxy;
  int m, q;
  int t, u, v;
  n = fr::gi();
  m = fr::gi();
  F(i, 1, m) {
    u = fr::gi();
    v = fr::gi();
    init(u, v);
  }
  q = fr::gi();
  clock_t cl = clock();
  F(i, 1, q) {
    if (clock() - cl > 1.55 * CLOCKS_PER_SEC) {
      F(kk, i, q) fwrite("NO\n", 3, 1, stdout);
      break;
    }
    t = fr::gi();
    switch (t) {
    case 1:
      u = fr::gi();
      v = fr::gi();
      t1(u, v);
      break;
    case 2:
      u = fr::gi();
      t2(u);
      break;
    case 3:
      u = fr::gi();
      v = fr::gi();
      t3(u, v);
      break;
    case 4:
      u = fr::gi();
      t4(u);
      break;
    }
    test();
    // if(i % 10000==0) fprintf(stderr, "%d\n", i);
  }
  return 0;
}
