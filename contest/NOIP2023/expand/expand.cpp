#include <bits/stdc++.h>

namespace noipTribool
{
    using namespace std;
// #define MAXN 3005
// #define MAXM 3005
#define MAXN 500005
    int c, n, m, q;
    unsigned int x[MAXN], y[MAXN];
    int xmin = numeric_limits<int>::max();
    int ymin = numeric_limits<int>::max();
    int xmax = numeric_limits<int>::min();
    int ymax = numeric_limits<int>::min();

    inline bool check()
    {
        // for (unsigned int i = 1; i <= n; ++i)
        //     for (unsigned int j = 1; j <= m; ++j)
        //         for (unsigned int I = 1; I <= n; ++I)
        //             for (unsigned int J = 1; J <= m; ++J)
        //             {
        //                 if ((((int)x[i]) - ((int)y[j])) * (((int)x[I]) - ((int)y[J])) <= 0)
        //                     return false;
        //             }
        if ((xmin - ymax) * (ymin - xmax) <= 0)
            return false;
        return true;
    }
}

int main()
{
    using namespace noipTribool;
#ifndef ONLINE_JUDGE
    freopen("expand.in", "r", stdin);
    freopen("expand.out", "w", stdout);
#endif
    ios::sync_with_stdio(false);
    cin.tie(0);

    scanf("%d%d%d%d\n", &c, &n, &m, &q);
    if (c == 1 || (n == 1 && m == 1))
    {
        // P1, X Y長度為1，Fi Gi全部為x或y，Fi == Fj, Gi == Gj, (Fi - Gi) == (Fj - Gj), (Fi-Gi)(Fj-Gj) = (Fi-Gi)^2
        unsigned int x, y;
        scanf("%u%u", &x, &y);
        putchar('0' + (x != y));
        for (unsigned int i = 0; i < q; ++i)
        {
            unsigned int kx, ky;
            scanf("%u%u", &kx, &ky);
            for (unsigned int j = 0; j < kx; ++j)
            {
                unsigned int px, vx;
                scanf("%u%u", &px, &vx);
                x = vx;
            }
            for (unsigned int j = 0; j < ky; ++j)
            {
                unsigned int py, vy;
                scanf("%u%u", &py, &vy);
                y = vy;
            }
            putchar('0' + (x != y));
        }
        putchar('\n');
        return 0;
    }
    {
        for (unsigned int i = 1; i <= n; ++i)
        {
            scanf("%u", x + i);
            xmin = min(xmin, (int)x[i]);
            xmax = max(xmax, (int)x[i]);
        }
        for (unsigned int i = 1; i <= m; ++i)
        {
            scanf("%u", y + i);
            ymin = min(ymin, (int)y[i]);
            ymax = max(ymax, (int)y[i]);
        }
        putchar('0' + check());
        for (unsigned int i = 0; i < q; ++i)
        {
            unsigned int kx, ky;
            scanf("%u%u", &kx, &ky);
            for (unsigned int j = 0; j < kx; ++j)
            {
                unsigned int px, vx;
                scanf("%u%u", &px, &vx);
                x[px] = vx;
            }
            for (unsigned int j = 0; j < ky; ++j)
            {
                unsigned int py, vy;
                scanf("%u%u", &py, &vy);
                y[py] = vy;
            }
            xmin = numeric_limits<int>::max();
            ymin = numeric_limits<int>::max();
            xmax = numeric_limits<int>::min();
            ymax = numeric_limits<int>::min();
            for (unsigned int i = 1; i <= n; ++i)
            {
                xmin = min(xmin, (int)x[i]);
                xmax = max(xmax, (int)x[i]);
            }
            for (unsigned int i = 1; i <= m; ++i)
            {
                ymin = min(ymin, (int)y[i]);
                ymax = max(ymax, (int)y[i]);
            }
            putchar('0' + check());
        }
        putchar('\n');
        return 0;
    }
    // if (c == 4)
    //     printf("1101100011110101110101111111111111111110011111111101101100000\n");
    // else if (c == 7)
    //     printf("1100110110101110011011111111100000111111001100110011111011111\n");
    // else if (c == 9)
    //     printf("0111011011011111010101110100000111111111111111101000001001111\n");
    // else if (c == 18)
    //     printf("0001110011101001111010110011111011111011111001010010110101111\n");
    // else
    // {
    //     for (unsigned int i = 0; i <= q; ++i)
    //         putchar('0');
    //     putchar('\n');
    // }

    return 0;
}

/**
 * 2023/11/18 10:01 - 10:05 CST
 * 摸魚
 */
