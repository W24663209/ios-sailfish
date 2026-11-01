#!/usr/bin/env python3
"""Build a temporary Marlin TWRP image with a byte-preserving CPIO hook.

The original verified LZMA stream is decompressed, then the sole newc CPIO
change is insertion of /sbin/runatboot.sh before TRAILER!!!.  The original
kernel, appended DTB table, boot header geometry, and every existing CPIO
entry retain their original bytes.

Output is for `fastboot boot` only. Never flash it.
"""
import argparse
import hashlib
import importlib.util
import struct
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location("newc_inject", HERE / "newc-inject.py")
newc = importlib.util.module_from_spec(spec)
spec.loader.exec_module(newc)

HOOK = b'''#!/sbin/sh
# Temporary non-persistent Marlin TWRP hook.
{
    echo "marlin-twrp-hook=1"
    echo "uid=$(id)"
    echo "selinux=$(getenforce 2>/dev/null || true)"
    echo "kernel=$(uname -a)"
} > /tmp/marlin-twrp-hook.txt
chmod 0644 /tmp/marlin-twrp-hook.txt
exit 0
'''


def align(value, page):
    return (value + page - 1) // page * page


def parse(blob):
    if blob[:8] != b"ANDROID!":
        raise SystemExit("not an Android boot image")
    ksize, _, rsize, _, ssize, _, _, page, version = struct.unpack_from("<9I", blob, 8)
    if version != 0 or page != 4096:
        raise SystemExit(f"requires legacy v0 / 4 KiB image, got {version}/{page}")
    koff = page
    kernel = blob[koff:koff + ksize]
    roff = koff + align(ksize, page)
    ramdisk = blob[roff:roff + rsize]
    soff = roff + align(rsize, page)
    return kernel, ramdisk, blob[soff:soff + ssize], page


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("reference", type=Path)
    ap.add_argument("output", type=Path)
    args = ap.parse_args()

    source = args.reference.read_bytes()
    kernel, ramdisk, second, page = parse(source)
    raw = subprocess.check_output(["xz", "--format=lzma", "-dc"], input=ramdisk)
    trailer, _ = newc.find_trailer(raw)
    modified = newc.inject(raw, "sbin/runatboot.sh", HOOK)
    new_ramdisk = subprocess.check_output(["xz", "--format=lzma", "-zc"], input=modified)

    header = bytearray(source[:page])
    struct.pack_into("<I", header, 16, len(new_ramdisk))
    digest = hashlib.sha1()
    for part in (kernel, new_ramdisk, second):
        digest.update(part)
        digest.update(struct.pack("<I", len(part)))
    header[576:608] = digest.digest().ljust(32, b"\0")
    out = bytes(header)
    out += kernel + b"\0" * (align(len(kernel), page) - len(kernel))
    out += new_ramdisk + b"\0" * (align(len(new_ramdisk), page) - len(new_ramdisk))
    out += second + b"\0" * (align(len(second), page) - len(second))
    args.output.write_bytes(out)

    # Verify the original stream is an exact prefix until its trailer and the
    # only payload inserted into decoded CPIO is the intended script entry.
    out_kernel, out_ramdisk, out_second, out_page = parse(out)
    check = subprocess.check_output(["xz", "--format=lzma", "-dc"], input=out_ramdisk)
    expected = raw[:trailer] + newc.entry("sbin/runatboot.sh", HOOK) + raw[trailer:]
    if out_kernel != kernel or out_second != second or out_page != page or check != expected:
        raise SystemExit("output validation failed")
    print(f"{args.output}: original kernel/DTBs retained; one CPIO hook inserted at {trailer:#x}")
    print(f"ramdisk {len(ramdisk)} -> {len(new_ramdisk)} bytes")
    print(f"sha256 {hashlib.sha256(out).hexdigest()}")


if __name__ == "__main__":
    main()
