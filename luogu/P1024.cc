#include <bits/stdc++.h>

int main() {
  using namespace std;
  cin.tie(0);

  double a, b, c, d;
  cin >> a >> b >> c >> d;
  for (double x = -100.0; x <= 100.0; x += 0.001) {
    double x2 = x * x;
    double x3 = x2 * x;
    if (fabs(x3 * a + x2 * b + x * c + d) < 0.0001)
      cout << fixed << setprecision(2) << x << " ";
  }
  putchar('\n');

  return 0;
}
