#include <cstdio>

int as = 0, os = 0;

int eval(bool peek);

int main() {
  printf("%d\n", eval(false));
  printf("%d %d\n", as, os);
  return 0;
}

int eval(bool peek) {
  char l, op, r;
  int lv, rv;
  while (scanf("%c", &l) != EOF) {
    switch (l) {
    case '(':
      lv = eval(peek);
      break;
    case ')':
      return lv;
    case '0':
      lv = 0;
      break;
    case '1':
      lv = 1;
      break;
    }
    scanf("%c%c", &op, &r);
    if ((op == '&' && lv == 0) || (op == '|' && lv == 1)) {
      if (!peek) {
        if (op == '&')
          as++;
        else
          os++;
        peek = true;
      }
    }
    switch (r) {
    case '(':
      rv = eval(peek);
      break;
    case '0':
      rv = 0;
      break;
    case '1':
      rv = 1;
      break;
    }
    if (op == '&') {
      lv = lv & rv;
    } else {
      lv = lv | rv;
    }
  }
  return lv;
}
