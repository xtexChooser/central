#include <cstdio>

int main() {
  long long a, b;
  scanf("%lld %lld", &a, &b);
  long long res = 1;
  for (; b > 0; b--) {
    res *= a;
    if (res > 1000000000L) {
      printf("-1");
      return 0;
    }
  }
  if (res > 1000000000L) {
    printf("-1");
  } else {
    printf("%lld", res);
  }
  return 0;
}
