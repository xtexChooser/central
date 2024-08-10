#include <bits/stdc++.h>

int rdi() {
  int v = 0;
  char ch;
  while (!isdigit(ch = getchar()))
    ;
  while (isdigit(ch)) {
    v = (v << 3) + (v << 1) + (ch ^ '0');
    ch = getchar();
  }
  return v;
}

int main() {
  using namespace std;
  ios::sync_with_stdio(false);
  cin.tie(0);
  int T = rdi();
  for (int r = 0; r < T; ++r) {
    int n = rdi();
    for (int i = 0; i < n; ++i)
      for (int j = 0; j < n; ++j)
        rdi();
    for (int i = 0; i < n; ++i)
      for (int j = 0; j < n; ++j)
        rdi();
    for (int i = 0; i < n; ++i)
      for (int j = 0; j < n; ++j)
        rdi();
    printf("No\n");
  }
  return 0;
}
