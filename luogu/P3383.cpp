#include <bits/stdc++.h>
using namespace std;

int n, q;

vector<int> pri;
bool not_prime[100000005];

int main() {
  scanf("%u%u", &n, &q);
  not_prime[0] = true;
  not_prime[1] = true;
  for (int i = 2; i <= n; ++i) {
    if (!not_prime[i])
      pri.push_back(i);
    for (int pri_j : pri) {
      if (i * pri_j > n)
        break;
      not_prime[i * pri_j] = true;
      if (i % pri_j == 0)
        break;
    }
  }
  for (int i = 0; i < q; ++i) {
    int a;
    scanf("%u", &a);
    printf("%u\n", pri[a - 1]);
  }

  return 0;
}
