#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "disk.h"

volatile static int started = 0;

// start() jumps here in supervisor mode on all CPUs.
void
main()
{
#ifdef XV6_BOARD_VISIONFIVE2
  // On VF2/OpenSBI, boot hart is not guaranteed to be hart 0.
  // Let the first hart entering main() run global initialization.
  if(__sync_bool_compare_and_swap(&started, 0, -1)){
#else
  if(cpuid() == 0){
#endif
    consoleinit();
    printfinit();
    printf("[dbg] console ready\n");
    printf("\n");
    printf("xv6 kernel is booting\n");
    printf("\n");
    printf("[dbg] kinit\n");
    kinit();         // physical page allocator
    printf("[dbg] kvminit\n");
    kvminit();       // create kernel page table
    printf("[dbg] kvminithart\n");
    kvminithart();   // turn on paging
    printf("[dbg] procinit\n");
    procinit();      // process table
    printf("[dbg] trapinit\n");
    trapinit();      // trap vectors
    printf("[dbg] trapinithart\n");
    trapinithart();  // install kernel trap vector
    printf("[dbg] plicinit\n");
    plicinit();      // set up interrupt controller
    printf("[dbg] plicinithart\n");
    plicinithart();  // ask PLIC for device interrupts
    printf("[dbg] plic ready\n");
    printf("[dbg] binit\n");
    binit();         // buffer cache
    printf("[dbg] iinit\n");
    iinit();         // inode table
    printf("[dbg] fileinit\n");
    fileinit();      // file table
    printf("[dbg] disk_init\n");
    disk_init();
    printf("[dbg] disk backend ready\n");
    printf("[dbg] userinit\n");
    userinit();      // first user process
    __sync_synchronize();
    started = 1;
  } else {
#ifdef XV6_BOARD_VISIONFIVE2
    while(started != 1)
#else
    while(started == 0)
#endif
      ;
    __sync_synchronize();
    printf("hart %d starting\n", cpuid());
    kvminithart();    // turn on paging
    trapinithart();   // install kernel trap vector
    plicinithart();   // ask PLIC for device interrupts
  }

  scheduler();        
}
