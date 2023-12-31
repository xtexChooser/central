#include <algorithm>
#include <bits/stdc++.h>
#include <limits.h>
#define MAXN 100001
#define MAXM 100001
#define CONT continue
#define F(i, b, e) for (i = b; i <= e; ++i)
// #define int long long

int n, m, q;
long long A[MAXN], B[MAXM];

namespace ut {
inline long long min(long long(*arr), int l, int r) {
  long long v = INT_MAX, i;
  F(i, l, r) v = std::min(v, arr[i]);
  return v;
}
inline long long max(long long(*arr), int l, int r) {
  long long v = INT_MIN, i;
  F(i, l, r) v = std::max(v, arr[i]);
  return v;
}
char buf[1 << 20], *p1, *p2;
inline char gc() {
  return (p1 == p2 &&
                  (p2 = (p1 = buf) + fread(buf, 1, 1 << 20, stdin), p1 == p2)
              ? EOF
              : *p1++);
}
inline long long gi() {
  long long x = 0;
  int w = 1;
  char ch = 0;
  while (ch < '0' || ch > '9') {
    if (ch == '-')
      w = -1;
    ch = gc();
  }
  while (ch >= '0' && ch <= '9') {
    x = x * 10 + (ch - '0');
    ch = gc();
  }
  return x * w;
}
} // namespace ut

int main() {
  using namespace std;
  std::ios::sync_with_stdio(false);
  std::cin.tie(0);
  n = ut::gi();
  m = ut::gi();
  q = ut::gi();
  int i, r;
  long long a, b, c;
  F(i, 1, n) A[i] = ut::gi();
  F(i, 1, m) B[i] = ut::gi();
  F(r, 1, q) {
    int l1, r1, l2, r2;
    if (r != 1)
      cout << '\n';
    l1 = ut::gi();
    r1 = ut::gi();
    l2 = ut::gi();
    r2 = ut::gi();
    // cout << "A:" << l1 << ".." << r1 << '\n';
    // cout << "B:" << l2 << ".." << r2 << '\n';
    if (l1 == r1) {
      if (l2 == r2) {
        cout << (A[l1] * B[l2]);
        CONT;
      }
      a = A[l1];
      if (a == 0)
        cout << '0';
      else if (a > 0)
        cout << (a * ut::min(B, l2, r2));
      else
        cout << (a * ut::max(B, l2, r2));
      CONT;
    }
    if (l2 == r2) {
      a = B[l2];
      if (a == 0)
        cout << '0';
      else if (a > 0)
        cout << (a * ut::max(A, l1, r1));
      else
        cout << (a * ut::min(A, l1, r1));
      CONT;
    }
    long long mina = INT_MAX, minb = INT_MAX, maxa = INT_MIN, maxb = INT_MIN,
              mpa = INT_MAX, mna = INT_MIN, mpb = INT_MAX, mnb = INT_MIN;
    int za = 0, zb = 0;
    F(i, l1, r1) {
      a = A[i];
      mina = min(mina, a);
      maxa = max(maxa, a);
      if (a < 0)
        mna = max(mna, a);
      else if (a > 0)
        mpa = min(mpa, a);
      else
        za = 1;
    }
    F(i, l2, r2) {
      a = B[i];
      minb = min(minb, a);
      maxb = max(maxb, a);
      if (a < 0)
        mnb = max(mnb, a);
      else if (a > 0)
        mpb = min(mpb, a);
      else
        zb = 1;
    }
    // cout << "A: " << mina << ".." << maxa << "\n";
    // cout << "B: " << minb << ".." << maxb << "\n";
    // A=0，0，A<0，B选最大，A>0，B选最小
    // B=0，0，B<0，A选最小，B>0，A选最大
    if (minb > 0) {
      // B只有正数，A选最大
      cout << (minb * maxa);
      CONT;
    }
    if (minb == 0) {
      // A<0，B选正数，A>=0，B选0
      cout << '0';
      CONT;
    }
    if (maxb <= 0) { // minB < 0
      // B <= 0，A选最小
      cout << (mina * (mina > 0 ? minb : maxb));
      CONT;
    }
    // minB < 0, maxB > 0
    if (mina > 0) {
      // A>0，B选最小值
      cout << (minb * (minb > 0 ? maxa : minb));
      CONT;
    }
    if (mina == 0) {
      // A >= 0，B选最小，B<0
      // cout << minb * mina; CONT;
      cout << '0';
      CONT;
    }
    // minA < 0, minB < 0, maxB > 0
    if (maxa <= 0) {
      // A<0，B选最大>0，
      cout << (maxa * maxb);
      CONT;
    }
    // minA < 0, maxA > 0, minB < 0, maxB > 0
    // A<=0，B最大>0，C<0
    // A>0，B选最小<0，C<0
    /* if (zb) {
            // B可选0，A>0，maxC <= 0，A<=0，B选最大，C<0
            cout << '0'; CONT;
    } */
    if (za || zb) {
      a = mina * maxb; // A<0，B>0
      b = maxa * minb; // A>0，B<0
      cout << (max(max(a, b), 0LL));
      CONT;
    }
    cout << (max(max(max(mna * maxb, mpa * minb), mnb * mina), mpb * maxa));
    CONT;
    // minA < 0, maxA > 0, minB < 0, maxB > 0, A != 0, B != 0
  }
  cout << '\n';
  return 0;
}
