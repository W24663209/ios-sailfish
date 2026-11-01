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

## Load-test boundary

A test still requires explicit confirmation because `insmod` executes code in
recovery-kernel context. The module is designed to be non-invasive, but a
kernel ABI mismatch can still cause a crash. It must only be pushed to a
**temporary** TWRP boot, loaded once, verified in `/proc/kmsg` or recovery
log, unloaded, and followed by a normal reboot to Android. Never install it
to any persistent partition.
