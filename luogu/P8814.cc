#include <cstdio>
#include <math.h>

int main() {
  int times = 0;
  scanf("%d\n", &times);
  for (; times > 0; times--) {
    long long n, e, d;
    scanf("%lld %lld %lld\n", &n, &e, &d);
    long long pqSum = n - e * d + 2, pqMul = n;
    long long pqDecSq = pqSum * pqSum - 4 * pqMul;
    double pqDecF = sqrt(pqDecSq);
    long long pqDec = (long long)pqDecF;
    if (pqDecSq < 0 || pqDec < 0 || (pqDecF - pqDec) != 0 ||
        (pqSum - pqDec) % 2 != 0) {
      printf("NO\n");
      continue;
    }
    long long q = (pqSum - pqDec) / 2;
    long long p = pqSum - q;
    if (p > q) {
      long long p0 = p;
      p = q;
      q = p0;
    }
    printf("%lld %lld\n", p, q);
  }
  return 0;
}
