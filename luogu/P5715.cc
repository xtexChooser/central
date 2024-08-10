#include <bits/stdc++.h>
int main() {
  using namespace std;
  ios::sync_with_stdio(false);
  cin.tie(0);
  int a, b, c;
  scanf("%d%d%d", &a, &b, &c);
  if (c < b)
    swap(c, b);
  if (b < a)
    swap(a, b);
  if (c < b)
    swap(c, b);
  printf("%d %d %d\n", a, b, c);
  return 0;
}
