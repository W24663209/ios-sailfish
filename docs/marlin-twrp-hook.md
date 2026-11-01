# Marlin TWRP runtime hook

Pixel XL Marlin's custom Linux-image handoff is not yet solved.  However, the
verified official TWRP image has a tested recovery userspace extension point:
`/sbin/runatboot.sh`.

`tools/mkmarlin-twrp-cpio-hook.py` adds only that script to a copy of the
verified TWRP ramdisk.  It must only be used with `fastboot boot`, never
`fastboot flash`.

## Verified behavior

On the connected Marlin device, a hook image with no kernel or DTB changes:

- was accepted through `fastboot boot`;
- booted to TWRP with recovery ADB available;
- created `/tmp/marlin-twrp-hook.txt` as root in `u:r:recovery:s0`;
- logged `Running boot script...` and exit code zero in `/tmp/recovery.log`;
- ran with SELinux permissive.

The builder verifies that the original TWRP kernel and appended DTB table are
byte-identical in its output.  The decoded CPIO archive is also byte-identical
until its `TRAILER!!!` record, where it inserts exactly one executable file.

## Current scope

The hook is suitable for **Linux/TWRP-side diagnostics and payload staging**.
It is not an XNU handoff.  Linux owns page tables, CPUs, interrupts, DMA and
memory allocation after recovery has booted; the running kernel has no kexec
support.  A module-based handoff would need the exact TWRP kernel source,
configuration and `Module.symvers` due to `CONFIG_MODVERSIONS=y`, followed by
a dedicated CPU/IRQ/DMA/IOMMU teardown design.

Do not use the hook to write Android partitions. Its default script writes only
under `/tmp`.
