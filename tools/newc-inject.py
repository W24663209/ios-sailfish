#!/usr/bin/env python3
"""Inject a single file into a newc CPIO archive without rebuilding it."""
import stat

ALIGN = 4
HEADER_SIZE = 110


def align(value):
    return (value + ALIGN - 1) & -ALIGN


def entry(name, data, mode=0o100755):
    name_b = name.encode() + b"\0"
    fields = [0, mode, 0, 0, 1, 0, len(data), 0, 0, 0, 0, len(name_b), 0]
    header = b"070701" + b"".join(f"{field:08x}".encode() for field in fields)
    assert len(header) == HEADER_SIZE
    blob = header + name_b
    blob += b"\0" * (align(len(blob)) - len(blob))
    blob += data
    blob += b"\0" * (align(len(blob)) - len(blob))
    return blob


def find_trailer(blob):
    off = 0
    while off + HEADER_SIZE <= len(blob):
        head = blob[off:off + HEADER_SIZE]
        if head[:6] != b"070701":
            raise ValueError(f"invalid newc magic at {off:#x}")
        values = [int(head[6 + 8*i:14 + 8*i], 16) for i in range(13)]
        size, namesize = values[6], values[11]
        name_off = off + HEADER_SIZE
        name = blob[name_off:name_off + namesize]
        next_off = align(name_off + namesize) + align(size)
        if name == b"TRAILER!!!\0":
            return off, next_off
        off = next_off
    raise ValueError("newc TRAILER!!! not found")


def inject(blob, name, data, mode=0o100755):
    trailer, end = find_trailer(blob)
    if end != len(blob):
        # newc permits final alignment bytes only.
        if any(blob[end:]):
            raise ValueError("unexpected nonzero bytes after newc trailer")
    return blob[:trailer] + entry(name, data, mode) + blob[trailer:]
