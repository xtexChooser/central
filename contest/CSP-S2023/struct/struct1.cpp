// 2023/11/26 调，+15pts
#include <bits/stdc++.h>
#include <algorithm>
#include <map>

//#define AINLINE __attribute__((always_inline))
#define INLINE inline
#define I int
#define U unsigned int
#define ULL unsigned long long
#define YLKL(x) __builtin_expect(x, 1)
#define NLKL(x) __builtin_expect(x, 0)

#define STR unsigned long long
#define Sbyte 34298
#define Sshort 8358525
#define Sint 5765
#define Slong 203144
/*printf("#define Sbyte %llu\n", hashStr("byte"));
printf("#define Sshort %llu\n", hashStr("short"));
printf("#define Sint %llu\n", hashStr("int"));
printf("#define Slong %llu\n", hashStr("long"));*/

#define ceilTo(val, mod) (((val) % (mod) == 0) ? (val) : ((val) - ((val) % (mod)) + (mod)))

namespace aStruct {
	using namespace std;
	U n;
	map<STR, const char*> destr;
	
	INLINE STR readStr() {
		char ch = getchar();
		STR s = 0;
		char *c = (char*) malloc(11);
		char *cp = c;
		while (ch == ' ' || ch == '\n') ch = getchar();
		while (ch != ' ' && ch != '\n') {
			s = (s * 26) + (ch - 'a');
			*(cp++) = ch;
			ch = getchar();
		}
		*(cp++) = 0;
		destr[s] = c;
		return s;
	}
	
	INLINE STR frStr(char until) {
		char ch = getchar();
		STR s = 0;
		while (ch == ' ' || ch == '\n') ch = getchar();
		while (ch != ' ' && ch != '\n' && ch != until) {
			s = (s * 26) + (ch - 'a');
			ch = getchar();
		}
		return s;
	}
	
	INLINE STR frStr1(char until, char *last) {
		char ch = getchar();
		STR s = 0;
		while (ch == ' ' || ch == '\n') ch = getchar();
		while (ch != ' ' && ch != '\n' && ch != until) {
			s = (s * 26) + (ch - 'a');
			ch = getchar();
		}
		*last = ch;
		return s;
	}
	
	INLINE STR hashStr(const char *c) {
		char ch = *(c ++);
		STR s = 0;
		while (ch != ' ' && ch != '\n' && ch != 0) {
			s = (s * 26) + (ch - 'a');
			ch = *(c ++);
		}
		return s;
	}
	
	struct Typ;
	
	struct Member {
		STR name;
		ULL pos, size;
		Typ *typ;
	}; 
	
	struct Typ {
		STR name;
		ULL size, align;
		U memberCnt;
		Member *members;
	};
	map<STR, Typ*> typs;
	
	struct El {
		STR name;
		ULL addr;
		Typ *typ;
	};
	list<El*> els;
	ULL addralloc = 0;
	
	INLINE void layoutTyp(Typ *t) {
		U k = t->memberCnt;
		Member *m;
		
		ULL align = 0;
		m = t->members;
		for (U i = 0; i < k; ++ i) {
			align = max(align, m->typ->align); 
			++ m;
		}
		t->align = align;
		
		ULL size = 0;
		m = t->members;
		for (U i = 0; i < k; ++ i) {
			ULL msize = m->typ->size;
			m->size = msize;
			ULL talign = m->typ->align;
			if (size % talign != 0) size += talign - (size % talign);
			m->pos = size;
			size += msize;
			++ m;
		}
		t->size = size;
	}
	
	INLINE void do1() { // 定义类型 
		STR n; U k;
		n = readStr(); scanf("%u", &k);
		Typ *t = (Typ *) malloc(sizeof (Typ));
		t->name = n;
		t->memberCnt = k;
		Member *m = (Member *) malloc(sizeof (Member) * k);
		t->members = m;
		typs[n] = t;
		for (U i = 0; i < k; ++ i) {
			STR t, n;
			t = frStr(0); n = readStr();
			m->name = n;
			m->typ = typs[t];
			++m;
		}
		layoutTyp(t);
		printf("%llu %llu\n", t->size, t->align);
	}
	
	INLINE void do2() { // 定义元素
		STR t, n;
		t = frStr(0); n = readStr();
		Typ *typ = typs[t];
		El *el = (El *) malloc(sizeof (El));
		if (addralloc % typ->align != 0) addralloc += typ->align - (addralloc % typ->align);
		el->name = n;
		el->addr = addralloc;
		el->typ = typ;
		els.push_back(el);
		addralloc += typ->size;
		printf("%llu\n", el->addr);
	}
	
	INLINE void do3() { // 元素到地址 
		char lastCh;
		STR e = frStr1('.', &lastCh);
		El *el;
		for (auto eli = els.begin(); eli != els.end(); ++ eli) {
			if ((*eli)->name == e) {
				el = *eli;
				break;
			}
		}
		ULL addr = el->addr;
		if (lastCh != '.') {
			printf("%llu\n", addr);
			return;
		}
		e = frStr1('.', &lastCh);
		Typ *t = el->typ;
		while (true) {
			Member *m = t->members;
			U k = t->memberCnt;
			for (U i = 0; i < k; ++ i) {
				if (m->name == e) {
					t = m->typ;
					addr += m->pos;
					break;
				}
				++ m;
			}
			if (lastCh != '.') break;
			e = frStr1('.', &lastCh);
		}
		printf("%llu\n", addr);
	}
	
	INLINE void do4() { // 地址到元素
		ULL addr;
		scanf("%llu", &addr);
		if (els.empty()) {
			printf("ERR\n"); return;
		} 
		El *el = nullptr;
		for (auto eli = els.begin(); eli != els.end(); ++ eli) {
			El *m = *eli;
			if (NLKL(m->addr <= addr && addr < m->addr + m->typ->size)) {
				el = m;
				break;
			}
		}
		if (el == nullptr)  {
			printf("ERR\n"); return;
		}
		addr -= el->addr;
		Typ *t = el->typ;
		ULL parts[1000];
		U part = 0;
		parts[part ++] = el->name; 
		while (addr != 0) {
			Member *m = t->members;
			U k = t->memberCnt;
			if (k == 0) break; // Primitives
			U ia;
			for (ia = 0; ia < k; ++ ia) {
				if (NLKL(m->pos <= addr && addr < m->pos + m->size)) break;
				++ m;
			}
			if (ia >= k) {
				printf("ERR\n"); return;
			}
			addr -= m->pos;
			parts[part ++] = m->name; 
			t = m->typ;
		}
		for (U i = 0; i < part; ++ i) {
			if (i != 0) putchar('.'); 
			printf("%s", destr[parts[i]]);
		}
		putchar('\n');
	}
	
	INLINE void doOp() {
		U op;
		scanf("%u", &op);
		switch (op) {
			case 1: do1(); break;
			case 2: do2(); break;
			case 3: do3(); break;
			case 4: do4(); break;
			default: throw "?";
		}
	}
}

int main() {
	using namespace aStruct;
	using namespace std;
	std::ios::sync_with_stdio(false);
	std::cin.tie(0);
	
	#ifndef ONLINE_JUDGE
	freopen("struct.in", "r", stdin);
	freopen("struct.out", "w", stdout);
	#endif
	
	#define DECL_PRIMITIVE_TYPE(sh, s, sz) { \
		destr[sh] = s; \
		Typ *t = (Typ *) malloc(sizeof (Typ)); \
		typs[sh] = t; \
		t->name = sh; t->size = sz; t->align = sz; t->memberCnt = 0; \
	}
	DECL_PRIMITIVE_TYPE(Sbyte, "byte", 1);
	DECL_PRIMITIVE_TYPE(Sshort, "short", 2);
	DECL_PRIMITIVE_TYPE(Sint, "int", 4);
	DECL_PRIMITIVE_TYPE(Slong, "long", 8);
	
	scanf("%u", &n);
	for (U i = 0; i < n; ++ i) doOp();
	
	return 0;
} 

/*
 * 2023/10/21 16:48- CST
*/

