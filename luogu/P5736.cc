#include <bits/stdc++.h>
using namespace std;

int n;

vector<int> pri;
bool not_prime[100005];

int main() {
  scanf("%u", &n);
  not_prime[0] = true;
  not_prime[1] = true;
  for (int i = 2; i <= 100000; ++i) {
    if (!not_prime[i])
      pri.push_back(i);
    for (int pri_j : pri) {
      if (i * pri_j > 100000)
        break;
      not_prime[i * pri_j] = true;
      if (i % pri_j == 0)
        break;
    }
  }
  for (int i = 0; i < n; ++i) {
    int a;
    scanf("%u", &a);
    if (!not_prime[a])
      printf("%u ", a);
  }
  putchar('\n');

  return 0;
}
