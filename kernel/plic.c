#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "disk.h"

//
// the riscv Platform Level Interrupt Controller (PLIC).
//

#ifdef XV6_BOARD_VISIONFIVE2
// Some firmware/platform combinations expose S-mode PLIC contexts as:
// 1) interleaved with M-mode (context = 2*hart+1), or
// 2) S-only contiguous (context = hart).
// Probe both layouts so IRQ routing works across variants.
#define PLIC_ENABLE_BASE   (PLIC + 0x2000)
#define PLIC_CONTEXT_BASE  (PLIC + 0x200000)
#define PLIC_ENABLE_STRIDE 0x80
#define PLIC_CTX_STRIDE    0x1000

static inline volatile uint32*
plic_enable_ctx(int ctx)
{
  return (volatile uint32*)(PLIC_ENABLE_BASE + ctx * PLIC_ENABLE_STRIDE);
}

static inline volatile uint32*
plic_ctx_threshold(int ctx)
{
  return (volatile uint32*)(PLIC_CONTEXT_BASE + ctx * PLIC_CTX_STRIDE);
}

static inline volatile uint32*
plic_ctx_claim(int ctx)
{
  return (volatile uint32*)(PLIC_CONTEXT_BASE + ctx * PLIC_CTX_STRIDE + 4);
}

static int plic_claim_ctx_cpu[NCPU];

static int
plic_ctx_candidates(int h, int *out)
{
  int n = 0;
  // Common layouts observed across platforms/firmware.
  out[n++] = 2 * h + 1; // interleaved S-context
  out[n++] = h;         // contiguous S-only context
  out[n++] = 2 * h;     // alternate interleaved base
  return n;
}
#endif

void
plicinit(void)
{
#ifdef XV6_BOARD_VISIONFIVE2
  // Dynamic bring-up: allow a broad source range, then mask
  // non-UART lines as we identify them.
  for(int irq = 1; irq < 1024; irq++)
    *(uint32*)(PLIC + irq*4) = 1;
#else
  // set desired IRQ priorities non-zero (otherwise disabled).
  *(uint32*)(PLIC + UART0_IRQ*4) = 1;
#if DISK_HAS_IRQ
  *(uint32*)(PLIC + DISK_IRQ*4) = 1;
#endif
#endif
}

void
plicinithart(void)
{
#ifdef XV6_BOARD_VISIONFIVE2
  int h = hartid();
  int ctx[3];
  int nctx = plic_ctx_candidates(h, ctx);
  for(int i = 0; i < nctx; i++){
    volatile uint32 *ena = plic_enable_ctx(ctx[i]);
    for(int w = 0; w < 32; w++)
      ena[w] = 0xffffffffU;
    *plic_ctx_threshold(ctx[i]) = 0;
  }
#else
  int hart = hartid();
  volatile uint32 *senable = (uint32*)PLIC_SENABLE(hart);
  // Some boards use IRQ IDs >= 32; index into the right enable register.
  senable[UART0_IRQ / 32] |= (1U << (UART0_IRQ % 32));
#if DISK_HAS_IRQ
  senable[DISK_IRQ / 32] |= (1U << (DISK_IRQ % 32));
#endif

  // set this hart's S-mode priority threshold to 0.
  *(uint32*)PLIC_SPRIORITY(hart) = 0;
#endif
}

// ask the PLIC what interrupt we should serve.
int
plic_claim(void)
{
#ifdef XV6_BOARD_VISIONFIVE2
  int h = hartid();
  int cpu = cpuid();
  int ctx[3];
  int nctx = plic_ctx_candidates(h, ctx);
  for(int i = 0; i < nctx; i++){
    int irq = *plic_ctx_claim(ctx[i]);
    if(irq){
      plic_claim_ctx_cpu[cpu] = ctx[i];
      return irq;
    }
  }
  return 0;
#else
  int hart = hartid();
  int irq = *(uint32*)PLIC_SCLAIM(hart);
  return irq;
#endif
}

// tell the PLIC we've served this IRQ.
void
plic_complete(int irq)
{
#ifdef XV6_BOARD_VISIONFIVE2
  int cpu = cpuid();
  int ctx = plic_claim_ctx_cpu[cpu];
  *plic_ctx_claim(ctx) = irq;
#else
  int hart = hartid();
  *(uint32*)PLIC_SCLAIM(hart) = irq;
#endif
}

void
plic_mask_irq(int irq)
{
#ifdef XV6_BOARD_VISIONFIVE2
  int h = hartid();
  int ctx[3];
  int nctx = plic_ctx_candidates(h, ctx);
  uint32 bit = (1U << (irq % 32));
  *(uint32*)(PLIC + irq*4) = 0;
  for(int i = 0; i < nctx; i++){
    volatile uint32 *ena = plic_enable_ctx(ctx[i]);
    ena[irq / 32] &= ~bit;
  }
#else
  int h = hartid();
  volatile uint32 *senable = (uint32*)PLIC_SENABLE(h);
  uint32 bit = (1U << (irq % 32));
  senable[irq / 32] &= ~bit;
#endif
}
