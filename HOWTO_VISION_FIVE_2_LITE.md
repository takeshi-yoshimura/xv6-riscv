# HOWTO: Run This xv6 Repository on VisionFive 2 Lite

This repository already includes a VisionFive2-specific board path (`BOARD=visionfive2`).

## 1. What this repo does for VisionFive2

- Builds with `-DXV6_BOARD_VISIONFIVE2`.
- Uses an embedded RAM disk image (`fs.img`) linked into the kernel.
- Enters xv6 in Supervisor mode (intended for OpenSBI/U-Boot boot flow).

Because of the embedded RAM disk, you only need to boot one file on the board: `kernel/kernel`.

## 2. Host prerequisites

Install on your build host:

- `make`
- `riscv64` cross toolchain with binutils and gcc (for example `riscv64-linux-gnu-*`)

Check toolchain quickly:

```bash
riscv64-linux-gnu-gcc --version
riscv64-linux-gnu-ld --version
```

## 3. Build xv6 for VisionFive2

From this repository root:

```bash
make clean
make visionfive2 -j$(nproc)
```

Expected output artifacts:

- `kernel/kernel` (RISC-V ELF64 executable, entry at `0x80000000`)
- `fs.img` (already embedded into `kernel/kernel` for this board)

## 4. Prepare serial console

Connect the board UART to your host and open a serial terminal at:

- `115200` baud
- `8N1`
- no flow control

Example with `screen`:

```bash
screen /dev/ttyUSB0 115200
```

## 5. Copy kernel to boot media

Copy `kernel/kernel` onto a partition U-Boot can read (FAT or ext4 on SD/eMMC/USB).

Example filename on media:

- `/xv6/kernel`

## 6. Boot from U-Boot prompt

At the U-Boot prompt on VisionFive2 Lite, load and start the ELF kernel.

For FAT partition (`mmc 0:1` shown as an example):

```bash
fatload mmc 0:1 ${kernel_addr_r} xv6/kernel
bootelf ${kernel_addr_r}
```

For ext4 partition:

```bash
ext4load mmc 0:1 ${kernel_addr_r} /xv6/kernel
bootelf ${kernel_addr_r}
```

## 7. Expected boot signs

On serial console you should see early/debug logs like:

- `[early] s-mode start`
- `xv6 kernel is booting`
- `[dbg] userinit`

Then xv6 shell should become usable.

## 8. Troubleshooting

1. No output on serial:
- Re-check serial port/device and 115200 8N1 settings.
- Ensure you are connected to the board's debug UART, not a different UART.

2. `Unknown command` / load fails in U-Boot:
- Verify media/partition and path (`xv6/kernel`).
- Use `mmc list`, `mmc dev`, `fatls` or `ext4ls` to locate the file.

3. Kernel loads but does not start:
- Confirm you used `bootelf ${kernel_addr_r}`.
- Rebuild with `make clean && make visionfive2`.

## 9. Quick command summary

```bash
# host
make clean
make visionfive2 -j$(nproc)

# u-boot (example)
fatload mmc 0:1 ${kernel_addr_r} xv6/kernel
bootelf ${kernel_addr_r}
```
