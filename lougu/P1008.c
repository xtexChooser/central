#include <stdio.h>

void sol(int i[9], int pos) {
  i[pos] = 1;
  for (; i[pos] <= 9; i[pos]++) {
    int idx = 0;
    for (; idx < pos; idx++) {
      if (i[idx] == i[pos]) {
        idx = -1;
        break;
      }
    }
    if (idx == -1)
      continue;
    if (pos == 8) {
      int n[3];
      n[0] = i[0] * 100 + i[1] * 10 + i[2];
      n[1] = i[3] * 100 + i[4] * 10 + i[5];
      n[2] = i[6] * 100 + i[7] * 10 + i[8];
      if (n[1] == n[0] * 2 && n[2] == n[0] * 3) {
        printf("%d %d %d\n", n[0], n[1], n[2]);
      }
    } else {
      sol(i, pos + 1);
    }
  }
}

int main() {
  int i[9];
  sol(i, 0);
  return 0;
}
