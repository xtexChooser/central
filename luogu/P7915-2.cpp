#include <bits/stdc++.h>
#include <algorithm>
#include <ctime>

int main() {
        using namespace std;
        #ifndef ONLINE_JUDGE
        freopen("palin1.in", "r", stdin);
        freopen("palin1.out", "w", stdout);
        #endif
        std::ios::sync_with_stdio(false);
        std::cin.tie(0);
        int T;
        scanf("%d\n", &T);
        srand(114514);
        clock_t cl = clock();
        clock_t clu = (0.98*CLOCKS_PER_SEC) / T;
        for (int t = 0; t < T; ++ t) {
                int n, n2;
                scanf("%d\n", &n);
                n2 = n * 2;
                vector<int> a(n2);
                for (int i = 0; i < n2; ++ i) scanf("%d\n", &a[i]);
                vector<char> s(n2 - 1);//; s.reserve(n2);
                bool succ = false;
                while ((clock() - cl) < clu) {
                        for (int i = 0; i < n2 - 1; ++ i) s[i] = ((rand() & 1) ? 'L' : 'R');
                        vector<int> b; b.reserve(n2);
                        
                        vector<int>::iterator afront = a.begin();
                        vector<int>::iterator aend = a.end();
                        
                        for (vector<char>::iterator it = s.begin(); it != s.end(); ++ it) {
                                switch (*it) {
                                        case 'L': b.push_back(*(afront ++)); break;
                                        case 'R': b.push_back(*(-- aend)); break;
                                }
                                // printf("%c ", *it);
                        }
                        b.push_back(*(afront ++));
                        int i;
                        for (i = 1; i <= n; ++ i) {
                                if (b[i - 1] != b[n2 - i]) break;
                        }
                        if (i > n) {
                                for (vector<char>::iterator it = s.begin(); it != s.end(); ++ it)
                                        putchar(*it);
                                printf("L\n");
                                succ = true;
                                break;
                        }
                }
                if (!succ) printf("-1\n");
        }
        return 0;
}
