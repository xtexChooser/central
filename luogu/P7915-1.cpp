#include <bits/stdc++.h>
#include <algorithm>

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
        for (int t = 0; t < T; ++ t) {
                int n, n2;
                scanf("%d\n", &n);
                n2 = n * 2;
                vector<int> a(n2);
                for (int i = 0; i < n2; ++ i) scanf("%d\n", &a[i]);
                vector<char> s; s.reserve(n2);
                while (true) {
                        if (!s.empty())
                                switch (s.back()) {
                                        case 'L': s.pop_back(); s.push_back('R'); break;
                                        case 'R':
                                                while (s.back() == 'R') s.pop_back();
                                                if (s.empty()) printf("-1\n");
                                                else { s.pop_back(); s.push_back('R'); break; }
                                }
                        else
                                s.push_back('L');
                        if (s.empty()) break;
                        while (s.size() != n2 - 1) s.push_back('L');
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
                                break;
                        }
                }
        }
        return 0;
}
