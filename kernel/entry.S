        # qemu -kernel loads the kernel at 0x80000000
        # and causes each hart (i.e. CPU) to jump there.
        # kernel.ld causes the following code to
        # be placed at 0x80000000.
.section .text
.global _entry
_entry:
        # set up a stack for C.
        # stack0 is declared in start.c,
        # with a 4096-byte stack per CPU.
        # sp = stack0 + ((hartid + 1) * 4096)
        la sp, stack0
#ifdef XV6_BOARD_VISIONFIVE2
        # In U-Boot/OpenSBI flows on VF2, tp is initialized with hartid.
        mv a1, tp
#else
        csrr a1, mhartid
#endif
        li t0, 1024*4
        addi a1, a1, 1
        mul t0, t0, a1
        add sp, sp, t0
        # jump to start() in start.c
        call start
spin:
        j spin
