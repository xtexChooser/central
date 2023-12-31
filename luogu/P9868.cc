#include <bits/stdc++.h>
// #include <algorithm>

namespace noipExpand {
using namespace std;
// #define MAXN 3005
// #define MAXM 3005
#define MAXN 4096
#define MAXM 4096
#define LETTERS ('z' - 'a')
int n, m;
char s[MAXN][MAXM];
char smin[MAXN][MAXM];
char smax[MAXN][MAXM];

template <typename V> struct fastVec {
  V val[MAXN];
  unsigned int cnt;

  inline void add(const V v) { val[cnt++] = v; }
};
typedef fastVec<unsigned int> FVUI;
FVUI jump[32];
// FVUI jumpmin[32];

// Dictionary Order Compare, if s1 >= s2
inline bool CSDO(char *s1, char *s2) {
  char *cmps1 = s1, *cmps2 = s2;
  bool ret = true;
  for (unsigned int k = 0; k < m; ++k) {
    char cmps1c = *cmps1, cmps2c = *cmps2;
    if (cmps1c < cmps2c) {
      ret = false;
      break;
    }
    if (cmps1c > cmps2c)
      break;
    ++cmps1;
    ++cmps2;
  }
  return ret;
}
} // namespace noipExpand

int main() {
  using namespace noipExpand;
  ios::sync_with_stdio(false);
  cin.tie(0);

  scanf("%d%d\n", &n, &m);
  if (n == 1) {
    printf("1\n");
    return 0;
  }

  for (int i = 1; i <= n; ++i) {
    char *buf = s[i];
    fread(buf, m, 1, stdin);
    while (getchar() != '\n')
      ;
    jump[buf[0] - 'a'].add(i);
  }

  for (int i = 1; i <= n; ++i) {
    char *st = s[i];
    unsigned int count[32];
    memset(count, 0, sizeof(count));
    for (int j = 0; j < m; ++j)
      ++count[st[j] - 'a'];

    unsigned int ptr = 0;
    char *stmin = smin[i];
    for (int j = 0; j <= LETTERS; ++j)
      for (int k = 0; k < count[j]; ++k)
        stmin[ptr++] = 'a' + j;
    char *stmax = smax[i];
    ptr = 0;
    for (int j = LETTERS; j >= 0; --j)
      for (int k = 0; k < count[j]; ++k)
        stmax[ptr++] = 'a' + j;
    // printf("%s\n", stmin);
    // jumpmin[stmin[0] - 'a'].add(stmin);
  }

  // for (int j = 0; j <= LETTERS; ++j)
  // 	printf("%d ", jump[j].cnt);
  // putchar('\n');
  // FILE *f = fopen("dict4.ans", "r");

  for (int i = 1; i <= n; ++i) {
    // char *st = s[i];
    char *stmin = smin[i];
    // char *stmax = smax[i];

    bool ans = true;
    // unsigned int fc = stmin[0] - 'a';
    for (unsigned int j = 1; j <= n; ++j) {
      if (j == i)
        continue;
      if (!CSDO(smax[j], stmin)) {
        ans = false;
        break;
      }
    }
    // unsigned int lowersat = 0xffff;
    // for (unsigned int j = 0; j < fc; ++j)
    // {
    // 	FVUI *jm = &jump[j];
    // 	if (jm->cnt != 0)
    // 	{
    // 		for (unsigned int k = 0; k < jm->cnt; ++ k) {

    // 		}
    // 		ans = false;
    // 	}
    // }
    // bool a1 = ans;

    // bool gtSameFC = false;
    // FVUI *jm = &jump[fc];
    // unsigned int *val = &jm->val[0];
    // for (unsigned int j = 0; j < jm->cnt; ++j)
    // {
    // 	char *ss = s[*val];
    // 	if (ss != st && !CSDO(ss, stmin))
    // 	{
    // 		ans = false;
    // 		gtSameFC = true;
    // 		break;
    // 	}
    // 	++val;
    // }

    // if (!ans)
    // 	if (lowersat != 0xffff && !gtSameFC)
    // 		if (CSDO(smax[jump[lowersat].val[0]], st))
    // 			ans = true;

    // if (!ans)
    // {
    // 	if (lowersat == 0xffff && gtSameFC)
    // 	{
    // 		ans = true;
    // 		gtSameFC = false;
    // 		jm = &jump[fc];
    // 		val = &jm->val[0];
    // 		for (unsigned int j = 0; j < jm->cnt; ++j)
    // 		{
    // 			char *ss = s[*val];
    // 			if (ss != st && !CSDO(ss, stmin))
    // 			{
    // 				ans = false;
    // 				char *vsm = smax[*val];
    // 				if (gtSameFC || !(vsm[0] < fc || (vsm[0] == fc
    // && CSDO(vsm, st))))
    // 				{
    // 					ans = false;
    // 					break;
    // 				}
    // 				else
    // 				{
    // 					gtSameFC = true;
    // 				}
    // 			}
    // 			++val;
    // 		}
    // 	}
    // }

    putchar('0' | ans);
    // if (('0' | ans) != fgetc(f))
    // 	printf("\nWRONG: %d calc: %d %d\n", i, ans, 0);
  }

  putchar('\n');

  return 0;
}

/**
 * 2023/11/18 8:30 - 9:57 CST
 * 不仔細看題導致白寫了一堆
 * 4 examples passed
 * NOI Linux passed
 * 什麽考試時間往後推遲結束一分鐘？有用嗎？（
 */

/*
gf-noi@gd-noi:/media/sf_public$ ulimit -v 524288
gf-noi@gd-noi:/media/sf_public$ g++ -O2 --std=c++14 --static dict.cpp -o dict
dict.cpp: In function ‘int main()’:
dict.cpp:58:9: warning: ignoring return value of ‘FILE* freopen(const char*,
const char*, FILE*)’, declared with attribute warn_unused_result
[-Wunused-result] 58 |  freopen("dict.in", "r", stdin); |
~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~ dict.cpp:59:9: warning: ignoring return value of
‘FILE* freopen(const char*, const char*, FILE*)’, declared with attribute
warn_unused_result [-Wunused-result] 59 |  freopen("dict.out", "w", stdout); |
~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~ dict.cpp:64:7: warning: ignoring return value
of ‘int scanf(const char*, ...)’, declared with attribute warn_unused_result
[-Wunused-result] 64 |  scanf("%d%d\n", &n, &m); |  ~~~~~^~~~~~~~~~~~~~~~~~
dict.cpp:74:8: warning: ignoring return value of ‘size_t fread(void*, size_t,
size_t, FILE*)’, declared with attribute warn_unused_result [-Wunused-result] 74
|   fread(buf, m, 1, stdin); |   ~~~~~^~~~~~~~~~~~~~~~~~
gf-noi@gd-noi:/media/sf_public$ for i in 1 2 3 4; do cp "dict$i.in" dict.in;
time ./dict; diff "dict$i.ans" dict.out; done

real	0m0.017s
user	0m0.000s
sys	0m0.008s

real	0m0.017s
user	0m0.000s
sys	0m0.005s

real	0m0.021s
user	0m0.004s
sys	0m0.002s

real	0m0.253s
user	0m0.103s
sys	0m0.112s

*/
