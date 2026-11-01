# Marlin TWRP diagnostic module

This directory contains the first, intentionally non-invasive, recovery-module
milestone for Pixel XL (`marlin`).

## Exact ABI inputs

The module is built only after all of these agree:

- recovery kernel source commit:
  `cfa2bd22f4bbbefaf950923a2eda3200b38f245b`
  from `bigbiff/android_kernel_google_marlin`;
- recovered kernel config from the booted TWRP image;
- generated `Module.symvers` from that exact source and config;
- module vermagic:
  `3.18.122-gcfa2bd22 SMP preempt mod_unload modversions aarch64`.

`CONFIG_MODVERSIONS=y` makes this mandatory. A stock Android Marlin kernel or
an arbitrary Linux 3.18 tree is not an acceptable build target.

## Phase 0 behavior

`marlin_diag.ko` has no MMIO accesses and does not touch storage, CPUs,
interrupts, DMA, IOMMUs, payload memory, or Android partitions. Its module
init/exit functions only use `pr_info()` to announce load/unload. It exists to
verify the recovered ABI before any trampoline logic is considered.

## Build host

The initial exact ABI tree was prepared on the Ubuntu build host with Android
GCC 4.9 cross compilers. The old Linux 3.18 host DTC requires `-fcommon` with
modern host GCC. The resulting module's SHA-256 and vermagic must be checked
before a temporary TWRP load test.

## Phase 1 result

Phase 1 loaded and unloaded successfully in temporary TWRP. It performed only
read-only scheduler/resource inspection and measured:

```text
online CPUs: 0, 1, 2, 3
boot CPU MPIDR: 0x80000000
total pages: 964002
recovery kernel code: 0x80080000–0x8174dfff
recovery kernel data: 0x817ee000–0x81cc1fff
upper available RAM: 0x91700000–0x17e54ffff
APSS watchdog: 0x09830000–0x09830fff
```

The module initially attempted to use `__cpu_logical_map`; `MODPOST` reported
that it is not exported, so that build was not loaded. The committed Phase 1
module uses exported symbols only.

## Load-test boundary

A test still requires explicit confirmation because `insmod` executes code in
recovery-kernel context. The module is designed to be non-invasive, but a
kernel ABI mismatch can still cause a crash. It must only be pushed to a
**temporary** TWRP boot, loaded once, verified in `/proc/kmsg` or recovery
log, unloaded, and followed by a normal reboot to Android. Never install it
to any persistent partition.
