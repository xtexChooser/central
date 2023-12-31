#include <algorithm>
#include <bits/stdc++.h>

bool sol(int n, int a[], char first) {
  using namespace std;
  int first_n = a[first == 'L' ? 0 : (n * 2 - 1)];
  deque<int> left, right;
  if (first == 'L') {
    int idx = 1;
    for (; a[idx] != first_n; ++idx)
      left.push_back(a[idx]);
    for (++idx; idx < 2 * n; ++idx)
      right.push_front(a[idx]);
  } else {
    int idx = 0;
    for (; a[idx] != first_n; ++idx)
      left.push_back(a[idx]);
    for (++idx; idx < 2 * n - 1; ++idx)
      right.push_front(a[idx]);
  }
  char step[n * 2];
  int stepf = 0, stepe = n * 2 - 1;
  step[stepf++] = first;
  step[stepe--] = 'L';
  while (!left.empty() || !right.empty()) {
    if (!left.empty()) {
      int val = left.front();
      if (left.size() > 1 && left.back() == val) {
        left.pop_front();
        left.pop_back();
        step[stepf++] = 'L';
        step[stepe--] = 'L';
        continue;
      } else if (!right.empty() && right.back() == val) {
        left.pop_front();
        right.pop_back();
        step[stepf++] = 'L';
        step[stepe--] = 'R';
        continue;
      }
    }
    if (!right.empty()) {
      int val = right.front();
      if (!left.empty() && left.back() == val) {
        right.pop_front();
        left.pop_back();
        step[stepf++] = 'R';
        step[stepe--] = 'L';
        continue;
      } else if (right.size() > 1 && right.back() == val) {
        right.pop_front();
        right.pop_back();
        step[stepf++] = 'R';
        step[stepe--] = 'R';
        continue;
      }
    }
    return false;
  }
  for (int i = 0; i < n * 2; ++i)
    putchar(step[i]);
  putchar('\n');
  return true;
}

int main() {
  using namespace std;
#ifndef ONLINE_JUDGE
  freopen("palin1.in", "r", stdin);
  freopen("palin1.out", "w", stdout);
#endif
  std::ios::sync_with_stdio(false);
  std::cin.tie(0);
  int T;
  scanf("%d\n", &T);
  for (int t = 0; t < T; ++t) {
    int n;
    scanf("%d\n", &n);
    int a[n * 2];
    for (int i = 0; i < n * 2; ++i)
      scanf("%d\n", &a[i]);
    if (!(sol(n, a, 'L') || sol(n, a, 'R')))
      printf("-1\n");
  }
  return 0;
}
