#include <bits/stdc++.h>
//#include <algorithm>

//#define AINLINE __attribute__((always_inline))
#define INLINE inline
#define I int
#define U unsigned int
#define YLKL(x) __builtin_expect(x, 1)
#define NLKL(x) __builtin_expect(x, 0)

#define MAXN 8

namespace aLock {
	using namespace std;
	U n;
	U pat[MAXN + 10][5];
	
	/*INLINE U sol1() {
		U same = 0;
		for (U i = 1; i <= n; ++ i) {
			U *p = &pat[i][0];
			for (U j = i + 1; j <= n; ++ j) if (memcmp(p, &pat[j][0], sizeof (U[5])))
				++ same;
		}
		U uniq = n - same;
		if (uniq == 1) {
			return 45; 
		}
		return 0;
	}
	
	INLINE U sol2() {
		return 0;
	}
	
	INLINE U sol() {
		return sol1() + sol2();
	}*/
	
	INLINE U sol() {
		U ans = 0;
		U a[5];
		#define FORA(pos) for (a[pos] = 0; a[pos] <= 9; ++ a[pos])
		FORA(0) { FORA(1) { FORA(2) { FORA(3) { FORA(4) {
			bool succ = true;
			for (U i = 1; i <= n; ++ i) {
				U *p = &pat[i][0];
				U diff = 0;
				for (U pos = 0; pos <= 4; ++ pos) diff += p[pos] != a[pos];
				if (NLKL(diff == 2)) {
					U diffpos[2] = { 100, 100 };
					for (U pos = 0; pos <= 4; ++ pos) if (p[pos] != a[pos])
						diffpos[diffpos[0] != 100] = pos;
					I diffval[2] = {
						(((I) p[diffpos[0]]) - ((I) p[diffpos[1]])),
						(((I) a[diffpos[0]]) - ((I) a[diffpos[1]]))
					};
					if (YLKL(diffpos[0] != diffpos[1] - 1
						|| (diffval[0] != diffval[1] && diffval[1] - diffval[0] != 10 && diffval[0] - diffval[1] != 10))) {
						succ = false; 
						break;
					}
				} else if (YLKL(diff != 1)) {
					succ = false;
					break;
				}
			}
			if (succ) ++ ans;
		} } } } }
		return ans;
	}
}

int main() {
	using namespace aLock;
	using namespace std;
	std::ios::sync_with_stdio(false);
	std::cin.tie(0);
	
	#ifndef ONLINE_JUDGE
	freopen("lock.in", "r", stdin);
	freopen("lock.out", "w", stdout);
	#endif
	
	scanf("%u", &n);
	for (U i = 1; i <= n; ++ i) {
		U *p = &pat[i][0];
		scanf("%u%u%u%u%u", p, p + 1, p + 2, p + 3, p + 4);
	}
	printf("%u\n", sol());
	return 0;
} 

/*
 * 2023/10/21 14:27-15:37 CST
 * Çî¾Ù 
 * 1 & 2 passed 
*/

