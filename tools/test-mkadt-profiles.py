#!/usr/bin/env python3
"""Regression checks for Sailfish and headless Marlin ADT profiles."""
import importlib.util
import os
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import adt  # noqa: E402

spec = importlib.util.spec_from_file_location("mkadt", HERE / "mkadt.py")
mkadt = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mkadt)


def props(node):
    return {key: value for key, flags, value in node.props}


def find(node, name):
    if props(node).get("name") == name.encode() + b"\0":
        return node
    for child in node.children:
        hit = find(child, name)
        if hit:
            return hit
    return None


def child_names(node):
    return {props(child)["name"].rstrip(b"\0").decode() for child in node.children}


def u32(value):
    return int.from_bytes(value, "little")


def main():
    seed = bytes(range(64))
    sailfish = mkadt.build(seed=seed, pixel1=True)
    marlin = mkadt.build(seed=seed, marlin=True)

    root = props(marlin)
    compat = root["compatible"].split(b"\0")
    assert b"ios-sailfish,msm8996-marlin" in compat
    assert b"qcom,msm8996" in compat
    assert props(marlin)["model"] == b"iPhone10,1\0"

    fb = find(marlin, "sweet-fbcon")
    assert fb is not None
    fbp = props(fb)
    assert u32(fbp["width"]) == 1440
    assert u32(fbp["height"]) == 2560
    assert u32(fbp["bits-per-pixel"]) == 24
    assert "kickoff-reg" not in fbp

    armio = find(marlin, "arm-io")
    assert armio is not None
    names = child_names(armio)
    assert "disp0" not in names
    assert "wlan" not in names
    assert names == {"gic", "wdt"}

    sf_armio = find(sailfish, "arm-io")
    sf_fb = find(sailfish, "sweet-fbcon")
    assert {"gic", "wdt", "disp0", "wlan"} <= child_names(sf_armio)
    assert "kickoff-reg" in props(sf_fb)

    try:
        mkadt.build(seed=seed, qemu=True, marlin=True)
    except ValueError:
        pass
    else:
        raise AssertionError("qemu + marlin must be rejected")
    print("mkadt Sailfish/Marlin profile checks: OK")


if __name__ == "__main__":
    main()
