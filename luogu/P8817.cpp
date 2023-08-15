#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

using namespace std;

int n, m, k;
int al;
int *sc;
int *pt;
int *ps;

int main() {
  int i, x, y, l;

  cin >> n >> m >> k;
  al = (n + 1);
  sc = (int *)malloc(sizeof(int) * al);
  sc[1] = 0;
  pt = (int *)malloc(sizeof(int) * (al + 1) * (al + 1));
  ps = (int *)malloc(sizeof(int) * (al + 1) * (al + 1));
  for (i = 0; i < (al * al); i++) {
    pt[i] = 0xffff;
    ps[i] = 0;
  }
  for (i = 2; i <= n; i++) {
    cin >> sc[i];
  }
  for (i = 0; i <= n; i++) {
    pt[n * al + n] = 0;
  }
  for (i = 0; i < m; i++) {
    int a, b;
    cin >> a >> b;
    pt[a * al + b] = 0;
    pt[b * al + a] = 0;
  }
  for (i = 0; i <= k; i++) {
    for (x = 0; x < al; x++) {
      for (y = 0; y < al; y++) {
        if (x != y && pt[x * al + y] == i) {
          // x to y possible
          // extend all routes
          for (l = 0; l < al; l++) {
            if (y != l && pt[y * al + l] != 0xffff) {
              pt[x * al + l] = min(pt[x * al + l], pt[y * al + l] + i + 1);
            }
          }
        }
      }
    }
  }
  /*for (x = 1; x < al; x++) {
    for (y = 1; y < al; y++) {
      cout << pt[x * al + y] << " ";
    }
    cout << "\n";
  }*/
  for (x = 0; x < al; x++) {
    for (y = 0; y < al; y++) {
      pt[x * al + y] = pt[x * al + y] <= k;
    }
  }
  /*cout << "---\n";
  for (x = 1; x < al; x++) {
    for (y = 1; y < al; y++) {
      cout << pt[x * al + y] << " ";
    }
    cout << "\n";
  }*/
  /*for (x = 1; x < al; x++) {
    for (y = 1; y < al; y++) {
      cout << pt[x * al + y] << " ";
    }
    cout << "\n";
  }*/
  int a, b, c, d;
  int scs = -1;
  for (a = 0; a < al; a++) {
    if (!pt[1 * al + a])
      continue;
    for (b = 0; b < al; b++) {
      if (a == b || !pt[a * al + b])
        continue;
      for (c = 0; c < al; c++) {
        if (c == a || c == b || !pt[b * al + c])
          continue;
        for (d = 0; d < al; d++) {
          if (d == a || d == b || d == c || !pt[c * al + d] || !pt[d * al + 1])
            continue;
          int s = sc[a] + sc[b] + sc[c] + sc[d];
          if (s == 30) {
            // printf("%d %d %d %d\n", a, b, c, d);
          }
          scs = max(scs, s);
        }
      }
    }
  }
  cout << scs;
  free(sc);
  free(pt);
  free(ps);
  return 0;
}