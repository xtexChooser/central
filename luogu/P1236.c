#include <math.h>
#include <stdio.h>

void find24(int a, int b, int c, int d) {

}

int main() {
  int a, b, c, d;
  for (a = 1; a <= 9; a++) {
    for (b = a; b <= 9; b++) {
      for (c = b; c <= 9; c++) {
        for (d = d; d <= 9; d++) {
            find24(a,b,c,d);
        }
      }
    }
  }
  return 0;
}
