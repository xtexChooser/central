#include <bits/stdc++.h>

int main() {
  using namespace std;
  int n;
  scanf("%d", &n);
  for (int i = 1; i <= n; ++i) {
    printf("%d\n", max((n - i) * 2, (i - 1) * 2));
  }
  return 0;
}
