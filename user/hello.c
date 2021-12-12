#include "kernel/types.h"
#include "user/user.h"
#include "kernel/riscv.h"

int
main(int argc, char *argv[]) {
  hello();

  printf("[user] hello\n");
  intr_off(); // crash
  printf("[user] interrupt off\n");
  intr_on();
  printf("[user] interrupt on\n");
  exit(0);
}
