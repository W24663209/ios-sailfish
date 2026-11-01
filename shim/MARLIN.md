# Pixel XL / marlin bring-up

This directory now has an **explicit, diagnostic-only** `marlin` shim path.
It is not an iOS/XNU boot image yet.

## Safety contract

- It emits a file intended only for `fastboot boot`; never use `fastboot flash`.
- It accepts only a first-generation Pixel XL stock boot image: Android boot header
  v0, 4096-byte pages, and `androidboot.hardware=marlin` in the kernel command
  line. It refuses Sailfish and unrelated images.
- The initial shim leaves the XL panel kickoff disabled. Its diagnostics are
  written to the shared Android ramoops console at `0x83540000`, then it resets.
- Do not attach an XNU payload until the runtime DT/memory handoff and panel mode
  are proven on this device.

## What is shared with sailfish

AOSP's MSM8996 HTC device tree puts both phones behind the same common board
profile for the boot-image layout, UFS, QCA6174/CNSS PCIe, nanohub QUP6 wiring,
and ramoops reservation. In particular, both use a 4 KiB v0 boot image with
appended DTBs; LK tags are at `0x82600000`; and the ramoops console starts at
`0x83540000`.

## What differs

Marlin's panel is 1440x2560 and needs a different DSI command-mode setup from
Sailfish's 1080x1920 panel. Its touch controller uses the same I2C bus and GPIOs
but a different coordinate range/configuration. Marlin also has a distinct DTB
board ID and revision pin overlays. Therefore it must use a Marlin stock boot
container and a dedicated ADT/XNU board profile.

## Current first milestone

Once `MARLIN_STOCK_BOOT.img` is available, build a no-payload diagnostic image:

```sh
WDT_SECS=15 shim/build-marlin.sh MARLIN_STOCK_BOOT.img
fastboot boot shim/out/marlin/marlin-diag-boot.img
```

It will reset after 15 seconds. After Android has restarted, obtain the ramoops
log through Android or a temporarily booted recovery. A successful first log
shows `PIXEL SHIM: loader`, the LK DTB address, and a controlled reset. This
validates only the safe boot-container/shim path; it does not imply XNU is safe
to boot.
