#ifndef _KERNEL_DISK_H_
#define _KERNEL_DISK_H_

#ifdef XV6_BOARD_VISIONFIVE2
#define DISK_HAS_IRQ 0
#define DISK_IRQ 0
#define disk_init() ramdisk_init()
#define disk_rw(b, write) ramdisk_rw((b), (write))
#define disk_intr() ramdisk_intr()
#else
#define DISK_HAS_IRQ 1
#define DISK_IRQ VIRTIO0_IRQ
#define disk_init() virtio_disk_init()
#define disk_rw(b, write) virtio_disk_rw((b), (write))
#define disk_intr() virtio_disk_intr()
#endif

#endif
