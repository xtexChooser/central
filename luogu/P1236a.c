#include <math.h>
#include <stdio.h>

int main() {
  int a, b, c, d;
  scanf("%d %d %d %d", &a, &b, &c, &d);
  int a1 = 0, b1 = 0, c1 = 0, d1 = 0;
  int num[4];
  int i = 0;
  for (; i < 4; i++) {
    int min = 1000;
    if (a < min && !a1)
      min = a;
    if (b < min && !b1)
      min = b;
    if (c < min && !c1)
      min = c;
    if (d < min && !d1)
      min = d;
    if (!a1 && min == a)
      a1 = 1;
    else if (!b1 && min == b)
      b1 = 1;
    else if (!c1 && min == c)
      c1 = 1;
    else
      d1 = 1;
    num[i] = min;
  }
  a = num[0], b = num[1], c = num[2], d = num[3];
  printf("%d %d %d %d", a, b, c, d);
  return 0;
}
