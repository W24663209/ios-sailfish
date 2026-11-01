# Pixel XL (`marlin`) board evidence

This record captures values extracted read-only from the connected Pixel XL
(`fastboot product: marlin`, bootloader `8996-012001-1710040120`) during the
initial port. It is input to the `--marlin` ADT profile and must be updated
from a fresh stock DTB if the device firmware changes.

## Source artifacts

- Active-slot image: `boot_a`, read through temporary official TWRP using
  `dd if=/dev/block/by-name/boot_a`; SHA-256:
  `a488b47964cd4d8bbae2803b0d1d81c3c9d83382c07f449d690caecff2d72c2a`.
- Selected ABL DTB: MSM8996pro v1.1 + PMI8996 Marlin A, selected from the
  image's appended DTB table by the device's ABL.
- Runtime Android map: LineageOS 16-derived kernel `3.18.122-gcfa2bd22f4b`.

## Confirmed common MSM8996 resources

| Resource | Value | Evidence |
| --- | --- | --- |
| GIC distributor | `0x09bc0000`, 64 KiB | active DTB `interrupt-controller@09bc0000` |
| GIC redistributor | `0x09c00000`, 1 MiB, stride `0x40000` | same DT node |
| UFS host | `0x00624000`, 0x2500 bytes | active DTB `ufshc@624000`, `/proc/iomem` |
| Nanohub QUP6 SPI | `0x0757a000` | active DTB `spi@757a000` |
| Nanohub control | reset 38, boot0 130, wake 24, IRQ1 123, IRQ2 117, CS 27 | active `sensorhub@0` |
| Touch QUP3 I2C | `0x07577000`, Synaptics DSX addr `0x20` | active DTB |
| Touch pins | IRQ 125, reset 89, AP/sensor-hub switch 80, SDA 48, SCL 47 | active DTB |
| Ramoops | `0x83500000`, 1 MiB; console `0x83540000`, 512 KiB | active DTB + `/proc/iomem` |
| Continuous splash | `0xc0001000`, `0x31ff000` bytes | active DTB |
| LK DTB handoff | `0x82000000` | ABL persistent log |

## Board-specific items deliberately disabled

The selected panel is `qcom,mdss_dsi_samsung_s6e3ha3_wqhd_cmd`. It is a WQHD
command-mode panel (display coordinates 1440 x 2560), not the Sailfish 1080 x
1920 panel. The current Marlin ADT must remain headless: no `sweet,display`,
no framebuffer kickoff register, no panel DCS commands, and no GPU scanout.

Likewise, touch, keys, battery, Wi-Fi, and nanohub are not enabled by the
Marlin XNU policy patch until each path has a dedicated Marlin profile.

## Boot-container status

No custom Marlin container is currently bootable. The official temporary TWRP
image fastboot-boots. Repacking its gzip kernel or LZMA ramdisk has not
reliably returned to recovery, so do not test custom `fastboot boot` images
until its exact container constraints are reproduced. No `fastboot flash` has
been used.
