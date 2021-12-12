#include "types.h"
#include "riscv.h"
#include "defs.h"

uint64
sys_hello(void)
{
    printf("[kernel] hello\n");
    intr_off();
    printf("[kernel] interrupt off\n");
    intr_on();
    printf("[kernel] interrupt on\n");
    return 0;
}
