#include <bits/stdc++.h>

#define valid(x) ((x) < -2147483648 || (x) > 2147483647)

int main() {
  using namespace std;
  long long xmin, xmax, ymin, ymax;
  scanf("%lld%lld%lld%lld", &xmin, &xmax, &ymin, &ymax);
  if (valid(xmin * ymin) || valid(xmax * ymin) || valid(xmin * ymax) ||
      valid(xmax * ymax)) {
    printf("long long ");
  }
  printf("int\n");
  return 0;
}
