#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdio.h>

using namespace std;

int n, Q, k; // n=ifaces Q=pkts k=TTL
int *cost;
int *v; // cost
int *path;

int calc(int s, int t, int pos);

int main() {
  int i, i1;

  cin >> n >> Q >> k;
  k = min(k, n);

  v = (int *)malloc(sizeof(int) * n);
  for (i = 0; i < n; i++) {
    cin >> v[i];
  }
  cost = (int *)malloc(sizeof(int) * n * n);
  memset(cost, 0, sizeof(int) * n * n);
  for (i = 0; i < n - 1; i++) {
    int a, b;
    cin >> a >> b;
    a--;
    b--;
    cost[a * n + b] = v[a];
    cost[b * n + a] = v[b];
  }
  path = (int *)malloc(sizeof(int) * k);
  for (i = 0; i < Q; i++) {
    int s, t;
    cin >> s >> t;
    s--;
    t--;
    memset(path, 0, sizeof(int) * k);
    path[0] = s;
    printf("OUT: %d\n", v[s] + calc(s, t, 1));
    cout << "PATH: ";
    for (i1 = 0; i1 < k; i1++) {
      cout << path[i1] + 1 << " ";
    }
    cout << ";\n";
  }
  return 0;
}

int calc(int s, int t, int pos) {
  if (s == t)
    return 0;
  if (cost[s * n + t] != 0) {
    return v[t];
  }
  int i, i1;
  int cost0 = 0xffffffff;
  for (i = 0; i < n; i++) {
    if (cost[s * n + i] == 0)
      continue;
    path[pos] = i;
    int ok = 1;
    for (i1 = 0; i1 < pos; i1++) {
      if (path[i1] == i) {
        ok = 0;
        break;
      }
    }
    if (ok) {
      if (pos < k - 1) {
        cost0 = min(cost0, v[i] + calc(i, t, pos + 1));
      }
    }
  }
  return cost0;
}
