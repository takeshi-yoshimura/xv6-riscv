#include "types.h"
#include "param.h"
#include "riscv.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

extern uchar _binary_fs_img_start[];
extern uchar _binary_fs_img_end[];

static struct spinlock ramdisk_lock;

void
ramdisk_init(void)
{
  initlock(&ramdisk_lock, "ramdisk");
}

void
ramdisk_rw(struct buf *b, int write)
{
  uint64 off = (uint64)b->blockno * BSIZE;
  uint64 nbytes = (uint64)(_binary_fs_img_end - _binary_fs_img_start);

  if(off + BSIZE > nbytes)
    panic("ramdisk_rw");

  acquire(&ramdisk_lock);
  if(write)
    memmove(_binary_fs_img_start + off, b->data, BSIZE);
  else
    memmove(b->data, _binary_fs_img_start + off, BSIZE);
  release(&ramdisk_lock);
}

void
ramdisk_intr(void)
{
  panic("ramdisk_intr");
}
