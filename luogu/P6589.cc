#include <algorithm>
#include <bits/stdc++.h>
#include <limits>

#define LKL(x) __builtin_expect(x, 1)
#define ULKL(x) __builtin_expect(x, 0)
#define FOR(v, f, t) for (auto v = f; v <= t; ++v)

#define MAXN 80005

namespace thetree {
using namespace std;
typedef unsigned int uint;

uint n, q, k;
struct node {
  uint parent;
  set<uint> edge;
  uint *dist;
  uint *maxpath, *minpath;
};
node g[MAXN];

inline void init() {
  FOR(i, 1, n) {
    uint *dist = g[i].dist = (uint *)malloc(sizeof(uint) * (n + 1));
    memset(dist, 0, sizeof(uint) * (n + 1));
    uint *maxpath = g[i].maxpath = (uint *)malloc(sizeof(uint) * (n + 1));
    uint *minpath = g[i].minpath = (uint *)malloc(sizeof(uint) * (n + 1));
    maxpath[i] = i;
    minpath[i] = i;
    queue<uint> bfs;
    bfs.push(i);
    while (!bfs.empty()) {
      uint node = bfs.back();
      bfs.pop();
      auto &nod = g[node];
      if (dist[nod.parent] == 0 && nod.parent != i) {
        dist[nod.parent] = dist[node] + 1;
        maxpath[nod.parent] = max(nod.parent, maxpath[node]);
        minpath[nod.parent] = min(nod.parent, minpath[node]);
        bfs.push(nod.parent);
      }
      for (auto ch : nod.edge) {
        if (dist[ch] == 0 && ch != i) {
          dist[ch] = dist[node] + 1;
          maxpath[ch] = max(ch, maxpath[node]);
          minpath[ch] = min(ch, minpath[node]);
          bfs.push(ch);
        }
      }
    }
  }
}

inline bool check(uint l, uint r) {
  FOR(u, l, r) {
    FOR(v, u + 1, r) {
      if (g[u].dist[v] <= k)
        continue;
      if (g[u].maxpath[v] > r || g[u].minpath[v] < l)
        continue;
      //				if (l == 1 && r == 2)
      //					printf("a: %u, b: %u %u\n", u,
      //v, g[u].dist[v]);
      return false;
    }
  }
  return true;
}

inline void sol(uint l, uint r) {
  uint ansa = 0, ansb = 0;
  FOR(a, l, r) {
    FOR(b, a, r) {
      if (check(a, b)) {
        ++ansa;
        ansb += b - a + 1;
      }
    }
  }
  printf("%u %u\n", ansa, ansb);
}
} // namespace thetree

int main() {
  using namespace thetree;
  ios::sync_with_stdio(false);
  cin.tie(0);

  scanf("%u%u%u", &n, &q, &k);
  FOR(i, 2, n) {
    uint u, v;
    scanf("%u%u", &u, &v);
    g[v].parent = u;
    g[u].edge.insert(v);
  }
  init();

  FOR(i, 1, q) {
    uint l, r;
    scanf("%u%u", &l, &r);
    sol(l, r);
  }

  return 0;
}
