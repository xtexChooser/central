#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <limits.h>
using namespace std;

unsigned int n, m, q;
int *A, *B;
unsigned int l1, r1, l2, r2;

#define c(i, j) (A[i] * B[j])

int game() {
  unsigned int x, y;
  if (l1 == r1 && l2 == r2)
    return c(l1, l2);
  if (l1 == r1) {
    // x = l1时，寻找最小值
    int sc = c(l1, l2);
    for (y = l2; y <= r2; y++) {
      sc = min(sc, c(l1, y));
    }
    return sc;
  } else if (l2 == r2) {
    // y = l2时，寻找最小值
    int sc = c(l1, l2);
    for (x = l1; x <= r1; x++) {
      sc = max(sc, c(x, l2));
    }
    return sc;
  }
  // 查找A的可能性
  int maxA, minA, maxNegA = INT_MIN, minPosA = INT_MAX;
  for (x = l1; x <= r1; x++) {
    int v = A[x];
    maxA = max(maxA, v);
    minA = min(minA, v);
    if (v < 0) {
      maxNegA = max(maxNegA, v);
    } else {
      minPosA = min(minPosA, v);
    }
  }
  // 每一列的最小值中最大的一列
  int cnt = INT_MIN;
  unsigned int fx = l1, fy = l2;
  for (x = l1; x <= r1; x++) {
    int mp = c(x, l2);
    for (y = l2; y <= r2; y++) {
      mp = min(mp, c(x, y));
    }
    // printf("FX %d,%d \n", x, mp);
    if (mp > cnt) {
      cnt = mp;
      fx = x;
    }
  }
  cnt = INT_MAX;
  for (y = l2; y <= r2; y++) {
    int mp = c(x, l2);
    for (x = l1; x <= r1; x++) {
      mp = max(mp, c(x, y));
    }
    if (mp < cnt) {
      cnt = mp;
      fy = y;
    }
  }
  // printf("FXY %d,%d \n", fx, fy);
  return c(fx, fy);
}

int main() {
  int i, j;
  cin >> n >> m >> q;
  A = (int *)malloc(sizeof(int) * (n + 1));
  B = (int *)malloc(sizeof(int) * (m + 1));
  // mins = (int *)malloc(sizeof(int) * max((n + 1), (m + 1)));
  // C = (int *)malloc(sizeof(int) * n * m);
  for (i = 1; i <= n; i++) {
    cin >> A[i];
  }
  for (i = 1; i <= m; i++) {
    cin >> B[i];
  }
  // cout << "---  \n";
  for (i = 1; i <= n; i++) {
    for (j = 1; j <= m; j++) {
      // c(i, j) = A[i] * B[j];
      //  cout << c(i, j) << " ";
    }
    // cout << "\n";
  }
  // cout << "---\n";
  for (i = 0; i < q; i++) {
    cin >> l1 >> r1 >> l2 >> r2;
    cout << game() << "\n";
  }
  /*free(A);
  free(B);*/
  return 0;
}
