#!/bin/bash
# Build the Pixel XL (marlin) *diagnostic* shim.
#
# It uses a genuine marlin stock boot image as the container and only produces
# an image for `fastboot boot`; it never flashes a partition.  A payload is
# optional.  Until the Marlin XNU/ADT profile is verified, do not pass one:
# the no-payload image validates the boot-image/shim/ramoops return path first.
#
# Usage: WDT_SECS=15 shim/build-marlin.sh MARLIN_STOCK_BOOT.img [PAYLOAD]
# Output: shim/out/marlin/marlin-diag-boot.img
set -euo pipefail

stock=$(realpath "$1")
payload=${2:+$(realpath "$2")}

# v0 is intentional: first-gen Pixel/XL boot images use a 4 KiB v0 header
# with appended DTBs.  Refuse a wrong source image before producing output.
python3 -I - "$stock" <<'PY'
import struct, sys
b = open(sys.argv[1], 'rb').read(608)
if b[:8] != b'ANDROID!':
    raise SystemExit('not an Android boot image')
header_version = struct.unpack_from('<I', b, 40)[0]
page_size = struct.unpack_from('<I', b, 36)[0]
cmdline = b[64:576].rstrip(b'\0')
if header_version != 0 or page_size != 4096:
    raise SystemExit(f'need a first-generation Pixel XL v0/4096 boot image; got header={header_version} page={page_size}')
if b'marlin' not in cmdline:
    raise SystemExit('refusing a non-marlin stock boot image (cmdline lacks androidboot.hardware=marlin)')
PY

cd "$(dirname "$0")"
X=aarch64-linux-gnu-
command -v ${X}gcc >/dev/null || X=aarch64-elf-
CFLAGS="-O2 -ffreestanding -nostdlib -fno-pic -mgeneral-regs-only -mstrict-align -fno-stack-protector -Wall -Wextra -DMARLIN -DWDT_SECS=${WDT_SECS:-15}"
out=out/marlin
mkdir -p "$out"
for f in start.S pixel.c pstore.c console.c fdt.c loader.c; do
    ${X}gcc $CFLAGS -c "$f" -o "$out/${f%.*}.o"
done
abs=$(${X}readelf -r "$out"/{pixel,pstore,console,fdt,loader}.o | grep -c ABS64 || true)
[ "$abs" = 0 ] || { echo "error: $abs absolute relocations in C objects" >&2; exit 1; }
${X}gcc $CFLAGS -static -T shim.ld -Wl,--build-id=none -Wl,--no-warn-rwx-segments -Wl,-N \
    "$out"/{start,pixel,pstore,console,fdt,loader}.o -o "$out/shim.elf"
${X}objcopy -O binary "$out/shim.elf" "$out/Image"
size=$(${X}nm "$out/shim.elf" | awk '$3=="_end"{print $1}')
truncate -s $((16#$size)) "$out/Image"
[ -z "${payload:-}" ] || cat "$payload" >> "$out/Image"
../tools/mkboot-v0.sh "$stock" "$out/Image" "$out/marlin-diag-boot.img"
echo "Built $out/marlin-diag-boot.img"
echo "Diagnostic only: use fastboot boot (never fastboot flash)."
