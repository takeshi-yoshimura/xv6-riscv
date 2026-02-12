#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "disk.h"

//
// the riscv Platform Level Interrupt Controller (PLIC).
//

void
plicinit(void)
{
  // set desired IRQ priorities non-zero (otherwise disabled).
  *(uint32*)(PLIC + UART0_IRQ*4) = 1;
#if DISK_HAS_IRQ
  *(uint32*)(PLIC + DISK_IRQ*4) = 1;
#endif
}

void
plicinithart(void)
{
  int hart = cpuid();
  volatile uint32 *senable = (uint32*)PLIC_SENABLE(hart);
  // Some boards use IRQ IDs >= 32; index into the right enable register.
  senable[UART0_IRQ / 32] |= (1U << (UART0_IRQ % 32));
#if DISK_HAS_IRQ
  senable[DISK_IRQ / 32] |= (1U << (DISK_IRQ % 32));
#endif

  // set this hart's S-mode priority threshold to 0.
  *(uint32*)PLIC_SPRIORITY(hart) = 0;
}

// ask the PLIC what interrupt we should serve.
int
plic_claim(void)
{
  int hart = cpuid();
  int irq = *(uint32*)PLIC_SCLAIM(hart);
  return irq;
}

// tell the PLIC we've served this IRQ.
void
plic_complete(int irq)
{
  int hart = cpuid();
  *(uint32*)PLIC_SCLAIM(hart) = irq;
}
