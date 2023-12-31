#include <bits/stdc++.h>
#include <vector>

using namespace std;

int n;

int main() {
  cin >> n;
  int i, v;
  vector<int> list;

  for (i = 1; i <= n; i++) {
    scanf("%d", &v);

    list.insert(upper_bound(list.begin(), list.end(), v), v);

    if (i % 2 == 1) {
      printf("%d\n", list[(i - 1) / 2]);
    }
  }
  return 0;
}
