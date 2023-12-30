#include <bits/stdc++.h>

namespace a {
    using namespace std;
    #define MAXN 100005
    struct ed {
        unsigned int t, w;
    };
    struct nod {
        unsigned int wx = 0;
        vector<ed> edg;
    } n[MAXN];
    bool vis[MAXN];
    void dfs(unsigned int i) {
        vis[i] = true;
        unsigned int cwx = n[i].wx;
        for (auto e : n[i].edg) {
            if (vis[e.t]) continue;
            n[e.t].wx = cwx ^ e.w;
            dfs(e.t);
        }
    }
}

int main() {
    using namespace a;
    unsigned int N, M;
    scanf("%u", &N);
    for (unsigned int i = 1; i < N; ++ i) {
        unsigned int u, v, w;
        scanf("%u%u%u", &u, &v, &w);
        ed e;
        e.t = v; e.w = w;
        n[u].edg.push_back(e);
        e.t = u;
        n[v].edg.push_back(e);
    }
    dfs(1);
    scanf("%u", &M);
    for (unsigned int i = 0; i < M; ++ i) {
        unsigned int u, v;
        scanf("%u%u", &u, &v);
        unsigned int ans = n[u].wx ^ n[v].wx;
        printf("%u\n", ans);
    }
    return 0;
}

