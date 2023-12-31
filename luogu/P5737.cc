#include <bits/stdc++.h>

int main() {
  using namespace std;
  int x, y;
  scanf("%u%u", &x, &y);
  vector<int> years;
  for (int a = x; a <= y; ++a) {
    if ((a % 400 == 0) || (a % 4 == 0 && a % 100 != 0))
      years.push_back(a);
  }
  printf("%d\n", years.size());
  for (auto a : years) {
    printf("%d ", a);
  }
  putchar('\n');
  return 0;
}
