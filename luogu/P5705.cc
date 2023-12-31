#include <bits/stdc++.h>
#include <stack>

int main() {
  using namespace std;
  stack<char> st;
  char ch;
  while ((ch = getchar()) != '\n')
    if (isdigit(ch) || ch == '.')
      st.push(ch);
  while (!st.empty()) {
    putchar(st.top());
    st.pop();
  }
  return 0;
}
