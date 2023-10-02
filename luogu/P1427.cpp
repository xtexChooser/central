#include <bits/stdc++.h>
#include <stack>

int main() {
  using namespace std;
  std::stack<int> s;
  while (true) {
    int n;
    cin >> n;
    if (n != 0) {
      s.push(n);
    } else {
      while (!s.empty()) {
        int v = s.top();
        cout << v << ' ';
        s.pop();
      }
      break;
    }
  }
  return 0;
}
