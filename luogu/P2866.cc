#include <algorithm>
#include <bits/stdc++.h>

int main() {
  using namespace std;
  stack<long long> st;
  long long N, ans = 0;
  scanf("%lld", &N);
  for (long long i = 0; i < N; i++) {
    long long a;
    scanf("%lld", &a);
    while (!st.empty() && st.top() <= a)
      st.pop();
    ans += st.size();
    st.push(a);
  }
  printf("%lld\n", ans);
  return 0;
}
