#include <bits/stdc++.h>
#define ll long long
using namespace std;
const int mxn = 2505;
vector<int> g[mxn], ng[mxn];
ll n, m, k;
ll a[mxn];
int dist[mxn][mxn];
inline void bfs(int x) {
  queue<int> q;
  for (; q.size();)
    q.pop();
  q.push(x);
  memset(dist[x], 63, sizeof(dist[x]));
  dist[x][x] = 0;
  for (; q.size();) {
    int u = q.front();
    q.pop();
    for (int v : g[u])
      if (dist[x][v] > dist[x][u] + 1) {
        dist[x][v] = dist[x][u] + 1;
        q.push(v);
      }
  }
}
int col[mxn];
ll dp[mxn], ans;
inline ll dfs(int x, int d) {
  if (dp[x] != -1)
    return dp[x];
  dp[x] = -5000000000000000000ll;
  if (d == 4) {
    if (dist[1][x] <= k + 1)
      dp[x] = a[x];
    else
      dp[x] = -5000000000000000000ll;
    return dp[x];
  }
  for (int y : ng[x])
    if (col[y] == d + 1)
      dp[x] = max(dp[x], dfs(y, d + 1) + a[x]);
  return dp[x];
}
int main() {
  clock_t st = clock();
  ios_base::sync_with_stdio(false);
  srand(1919810);
  cin >> n >> m >> k;
  for (int i = 2; i <= n; ++i)
    cin >> a[i];
  for (int i = 1, u, v; i <= m; ++i) {
    cin >> u >> v;
    g[u].push_back(v);
    g[v].push_back(u);
  }
  for (int i = 1; i <= n; ++i)
    bfs(i);
  for (int i = 1; i <= n; ++i)
    for (int j = i + 1; j <= n; ++j)
      if (dist[i][j] <= k + 1) {
        ng[i].push_back(j);
        ng[j].push_back(i);
      }
  for (int ee = 0; ee < 10000; ++ee) {
    if (clock() - st > 1.98 * CLOCKS_PER_SEC)
      break;
    if (rand() % 3 == 1)
      rand();
    for (int i = 2; i <= n; ++i)
      col[i] = rand() % 4 + 1;
    memset(dp, -1, sizeof(dp));
    dfs(1, 0);
    ans = max(ans, dp[1]);
  }
  cout << ans << "\n";
  return 0;
}
