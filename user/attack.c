#include "kernel/types.h"
#include "user/user.h"
#include "kernel/riscv.h"

int main() {
  char *end=sbrk(PGSIZE*17)+16*PGSIZE;
  write(2,end+32,8);
  exit(1);
  exit(0);
}