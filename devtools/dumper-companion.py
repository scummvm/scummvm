#!/usr/bin/env python3
#
# prerequisites: pip3 install machfs
# This file contains tests. They can be run with $ pytest dumper-companion.py

from typing import Tuple

import argparse
from binascii import crc_hqx
from pathlib import Path
from struct import pack

import machfs


def file_to_macbin(f: machfs.File, name:str, encoding:str ) -> bytes:
    name = name.encode("mac_roman").decode(encoding)
    oldFlags = f.flags >> 8
    newFlags = f.flags & 8
    macbin = pack(
        ">xB63s4s4sBxHHHBxIIIIHB14xIHBB",
        len(name),
        name.encode(encoding),
        f.type,
        f.creator,
        oldFlags,
        0,
        0,
        0,
        f.locked,
        len(f.data),
        len(f.rsrc),
        f.crdate,
        f.mddate,
        0,
        newFlags,
        0,
        0,
        129,
        129,
    )
    macbin += pack(">H2x", crc_hqx(macbin, 0))
    if f.data:
        macbin += f.data
        macbin += b"\x00" * (128 - len(f.data) % 128)

    if f.rsrc:
        macbin += f.rsrc
        macbin += b"\x00" * (128 - len(f.rsrc) % 128)

    return macbin


def escape_string(s: str) -> str:
    new_name = [
        "\x80" + chr(0x80 + ord(i)) if i in '/":*[]+|\\?%<>,;=' or ord(i) < 0x20 else i
        for i in s
    ]
    return "".join(new_name)


def decode_name(s: str, encoding:str) -> str:
    s = s.encode("mac_roman").decode(encoding)
    s = escape_string(s)
    return s.encode("punycode").decode("ascii")

def generate_punyencoded_path(destination_dir: Path, encoding:str, hpath: Tuple[str]) -> Path:
    upath = destination_dir

    for el in hpath:
        if decode_name(el, encoding=encoding) != el + "-":
            decoded = decode_name(el, encoding=encoding)
            upath /= "xn--" + decode_name(el, encoding=encoding)
        else:
            upath /= el
    return upath


def extract_volume(source_volume: Path, destination_dir:Path, encoding:str) -> None:
    print(f"Loading {source_volume} ...")
    vol = machfs.Volume()
    vol.read(source_volume.read_bytes())

    destination_dir.mkdir(parents=True)

    for hpath, obj in vol.iter_paths():
        upath = generate_punyencoded_path(destination_dir, encoding, hpath)

        if isinstance(obj, machfs.Folder):
            upath.mkdir(exist_ok=True)
        else:
            print(upath)
            file = obj.data
            if obj.rsrc:
                file = file_to_macbin(obj, hpath[-1], encoding=encoding)
            upath.write_bytes(file)


if __name__ == "__main__":
    args = argparse.ArgumentParser()
    args.add_argument(
        "-e",
        metavar="ENCODING",
        type=str,
        default="mac_roman",
        help="String encoding (see https://docs.python.org/3/library/codecs.html#standard-encodings)",
    )
    args.add_argument("src", metavar="INPUT", type=Path, help="Disk image")
    args.add_argument("dir", metavar="OUTPUT", type=Path, help="Destination folder")
    parsed_args = args.parse_args()

    extract_volume(parsed_args.src, parsed_args.dir, parsed_args.e)

def test_decode_name():
    checks = [["Icon\r", "Icon-ea2f"]]
    for input, expected in checks:
        assert decode_name(input, "mac_roman") == expected

def test_escape_string():
    checks = [["\r", "\x80\x8d"]]
    for input, expected in checks:
        assert escape_string(input) == expected