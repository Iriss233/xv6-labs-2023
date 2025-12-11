#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

void printhex(uint64 x) {
  char buf[17];
  for(int i = 15; i >= 0; i--) {
    int digit = x & 0xf;
    buf[i] = digit < 10 ? '0' + digit : 'a' + digit - 10;
    x >>= 4;
  }
  buf[16] = '\0';
  write(1, "0x", 2);
  write(1, buf, 16);
}

int
main(int argc, char *argv[])
{
  if(argc != 2){
    write(1, "Usage: secret the-secret\n", 25);
    exit(1);
  }
  
  char *end = sbrk(PGSIZE*32);
  write(1, "secret: base=", 13);
  printhex((uint64)end);
  write(1, "\n", 1);
  
  end = end + 9 * PGSIZE;
  write(1, "secret: page9=", 14);
  printhex((uint64)end);
  write(1, "\n", 1);
  
  strcpy(end, "my very very very secret pw is:   ");
  strcpy(end+32, argv[1]);
  
  write(1, "secret: password='", 18);
  write(1, end+32, 8);
  write(1, "'\n", 2);
  
  exit(0);
}