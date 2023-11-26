#include <bits/stdc++.h>

namespace noipTribool
{
    using namespace std;
#define MAXN 100005
#define MAXM 100005
    unsigned int testid, testcnt;
    unsigned int n, m;
    unsigned int vals[MAXN];

#define isdigit(x) ((x) >= '0' && (x) <= '9')
    inline unsigned int rdu32()
    {
        unsigned int v = 0;
        char c = getchar();
        while (!isdigit(c))
            c = getchar();
        while (isdigit(c))
        {
            v = (v << 3) + (v << 1) + (c ^ '0');
            c = getchar();
        }
        return v;
    }

    struct xextstack
    {
        unsigned int buf[MAXN];
        unsigned int ext[MAXN];
        unsigned int ptr;

        inline bool empty()
        {
            return ptr == 0;
        }
        inline unsigned int top()
        {
            return buf[ptr - 1];
        }
        inline void pop()
        {
            ext[buf[--ptr]] = 0;
        }
        inline void push(unsigned int v)
        {
            buf[ptr++] = v;
            ext[v] = ptr;
        }
        inline bool contains(unsigned int v)
        {
            return ext[v] != 0;
        }
    } exst;

    unsigned int solP34()
    {
        memset(vals, 0, (n + 5) * sizeof(unsigned int));
        unsigned int ans = 0;
        for (unsigned int i = 0; i < m; ++i)
        {
            char v = getchar();
            unsigned int var;
            // scanf("%u\n", &var);
            var = rdu32();
            if (v == 'U' && vals[var] == 0)
            {
                ++ans;
                vals[var] = 1;
            }
        }
        return ans;
    }

    // // bad
    // unsigned int solP56()
    // {
    //     memset(vals, 0, (n + 5) * sizeof(unsigned int));
    //     unsigned int ans = 0;
    //     for (unsigned int i = 0; i < m; ++i)
    //     {
    //         char v = getchar();
    //         unsigned int var;
    //         if (v == 'U')
    //         {
    //             scanf("%u\n", &var);
    //             vals[var] = 1;
    //         }
    //         else
    //         {
    //             unsigned int j;
    //             scanf("%u%u\n", &var, &j);
    //             if (vals[var] != 1 && var != j)
    //             {
    //                 unsigned int ov = vals[j];
    //                 if (ov == 1)
    //                     vals[var] = 1;
    //                 else
    //                     vals[var] = 2 + j;
    //             }
    //         }
    //     }
    //     for (unsigned int i = 1; i <= n; ++i)
    //     {
    //         unsigned int ov = vals[i];
    //         if (ov >= 2)
    //         {
    //             exst.push(i);
    //             while (!exst.empty())
    //             {
    //                 unsigned int si = exst.top();
    //                 // printf("p %u %u\n", i, si);
    //                 // fflush(stdout);
    //                 unsigned int sv = vals[si];
    //                 if (sv < 2)
    //                     exst.pop();
    //                 else if (sv - 2 == i && exst.ptr != 1)
    //                 {
    //                     vals[si] = 0;
    //                     exst.pop();
    //                 }
    //                 else
    //                 {
    //                     unsigned int av = vals[sv - 2];
    //                     if (av <= 2)
    //                         vals[si] = av;
    //                     else
    //                     {
    //                         if (exst.contains(sv - 2))
    //                         {
    //                             vals[si] = 0;
    //                             continue;
    //                         }
    //                         exst.push(sv - 2);
    //                     }
    //                 }
    //             }
    //             // while (ov >= 2)
    //             // {
    //             //     if (ov - 2 == i)
    //             //     {
    //             //         ov = 0;
    //             //         break;
    //             //     }
    //             //     printf("ref %u %u\n", i, ov - 2);
    //             //     fflush(stdout);
    //             //     ov = vals[ov - 2];
    //             // }
    //         }
    //         if (ov)
    //             ++ans;
    //     }
    //     return ans;
    // }

    unsigned int solP12()
    {
        memset(vals, 0, (n + 5) * sizeof(unsigned int));
        // memset(vals, 0, sizeof(vals));
        // memset(&exst, 0, sizeof(exst));
        unsigned int ans = 0;
#define AMK(h, l) ((h << 18) | l)
#define AGH(v) (v >> 18)
#define AGL(v) (v & 0b111111111111111111)
#define uT 0
#define uU 1
#define uF 2
#define tf(x) (2 - x)
#define ex12(x) ((x) == 2 ? 1 : 2) // 2 = reverted
        for (unsigned int k = 0; k < m; ++k)
        {
            char v = getchar();
            unsigned int i, j;
            if (v != '+' && v != '-')
                // scanf("%u\n", &i);
                i = rdu32();
            else
            // scanf("%u%u\n", &i, &j);
            {
                i = rdu32();
                j = rdu32();
            }
            if (v == 'T')
                vals[i] = uT;
            else if (v == 'F')
                vals[i] = uF;
            else if (v == 'U')
                vals[i] = uU;
            else
            {
                bool reverted = (v == '-');
                unsigned int ov = vals[j];
                unsigned int vi;
                if (ov < 3) // j constant
                    vals[i] = vi = AMK(1 + reverted, j);
                else // j connected
                    vals[i] = vi = (reverted ? AMK(ex12(AGH(ov)), AGL(ov)) : ov);
                if (vi == AMK(1, i))
                    vals[i] = vi = uT;
                // else if (vi == AMK(2, i))
                //     vals[i] = vi = uU;
            }
        }
        // printf("infer\n");
        // fflush(stdout);
        for (unsigned int i = 1; i <= n; ++i)
        {
            {
                unsigned int vi = vals[i];
                unsigned int vih = AGH(vi), vil = AGL(vi);
                if (vih == 0)
                    continue;
                bool reverted = (vih == 2);
                if (vals[vil] < 3)
                {
                    vals[i] = (reverted ? tf(vals[vil]) : vals[vil]);
                    continue;
                }
            }
            { // search
                // printf("search %u\n", i);
                // fflush(stdout);
                exst.push(i);
                while (!exst.empty())
                {
                    unsigned int si = exst.top();
                    // printf("p %u\n", si);
                    // fflush(stdout);
                    unsigned int vi = vals[si];
                    unsigned int vih = AGH(vi), vil = AGL(vi);
                    bool reverted = (vih == 2);
                    if (vals[vil] < 3)
                    {
                        vals[si] = (reverted ? tf(vals[vil]) : vals[vil]);
                        exst.pop();
                    }
                    else
                    {
                        if (exst.contains(vil))
                        {
                            unsigned int a = exst.ext[vil] - 1;
                            reverted = false;
                            for (; a < exst.ptr; ++a)
                                if (AGH(vals[exst.buf[a]]) == 2)
                                    reverted = !reverted;
                            // printf("rev %u %u\n", si, reverted);
                            vals[si] = (reverted ? uU : uT);
                            exst.pop();
                            continue;
                        }
                        exst.push(vil);
                    }
                }
            }
        }
        for (unsigned int i = 1; i <= n; ++i)
        {
            // printf("%u = %u + %u\n", i, AGH(vals[i]), AGL(vals[i]));
            if (vals[i] == uU)
                ++ans;
        }
#undef AMK
#undef AGH
#undef AGL
#undef uT
#undef uU
#undef uF
#undef tf
#undef ex12
        return ans;
    }

    unsigned int sol()
    {
        if (testid == 1 || testid == 2)
            return solP12();
        if (testid == 3 || testid == 4)
            return solP34();
        // if (testid == 5 || testid == 6)
        //     return solP56();
        return solP12();
    }
}

int main()
{
    using namespace noipTribool;
#ifndef ONLINE_JUDGE
    freopen("tribool.in", "r", stdin);
    freopen("tribool.out", "w", stdout);
#endif
    ios::sync_with_stdio(false);
    cin.tie(0);

    // scanf("%u%u", &testid, &testcnt);
    testid = rdu32();
    testcnt = rdu32();
    for (unsigned i = 0; i < testcnt; ++i)
    {
        // scanf("%u%u\n", &n, &m);
        n = rdu32();
        m = rdu32();
        printf("%u\n", sol());
    }

    return 0;
}

/**
 * 2023/11/18 10:07 - 12:17 CST
 * 呃呃呃
 * example 1, 2, 4 passed
 */

/*
gf-noi@gd-noi:/media/sf_public$ ulimit -v 524288
gf-noi@gd-noi:/media/sf_public$ g++ -O2 --std=c++14 --static tribool.cpp -o tribool
tribool.cpp: In function ‘int main()’:
tribool.cpp:289:12: warning: ignoring return value of ‘FILE* freopen(const char*, const char*, FILE*)’, declared with attribute warn_unused_result [-Wunused-result]
  289 |     freopen("tribool.in", "r", stdin);
      |     ~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~
tribool.cpp:290:12: warning: ignoring return value of ‘FILE* freopen(const char*, const char*, FILE*)’, declared with attribute warn_unused_result [-Wunused-result]
  290 |     freopen("tribool.out", "w", stdout);
      |     ~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~
gf-noi@gd-noi:/media/sf_public$ for i in 1 2 3 4; do cp "tribool$i.in" tribool.in; time ./tribool; diff "tribool$i.ans" tribool.out; done

real	0m0.015s
user	0m0.000s
sys	0m0.007s

real	0m0.011s
user	0m0.000s
sys	0m0.004s

real	0m0.012s
user	0m0.000s
sys	0m0.005s
3,5c3,5
< 206
< 15
< 275
---
> 109
> 14
> 254

real	0m0.069s
user	0m0.009s
sys	0m0.047s

*/
