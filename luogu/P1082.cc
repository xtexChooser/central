#include <bits/stdc++.h>

#define ll long long

inline ll rdll() {
  using namespace std;
  ll v = 0;
  char c = getchar();
  while (!isdigit(c))
    c = getchar();
  while (isdigit(c)) {
    v = (v << 3) + (v << 1) + (c ^ '0');
    c = getchar();
  }
  return v;
}

inline ll exgcd(ll a, ll b, ll &x, ll &y) {
  using namespace std;
  x = 1, y = 0;
  ll x1 = 0, y1 = 1, a1 = a, b1 = b;
  while (b1) {
    ll q = a1 / b1;
    tie(x, x1) = make_tuple(x1, x - q * x1);
    tie(y, y1) = make_tuple(y1, y - q * y1);
    tie(a1, b1) = make_tuple(b1, a1 - q * b1);
  }
  return a1;
}

int main() {
  ll a = rdll();
  ll b = rdll();
  ll x, y;
  exgcd(a, b, x, y);
  x = (x % b + b) % b;
  printf("%lld\n", x);
  return 0;
}
