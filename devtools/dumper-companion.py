#!/usr/bin/env python3
#
# prerequisites: pip3 install machfs
# This file contains tests. They can be run with:
#  $ pytest dumper-companion.py

import argparse
import io
from binascii import crc_hqx
from pathlib import Path
from struct import pack
from typing import Any, List, Tuple

import machfs


def file_to_macbin(f: machfs.File, name: str, encoding: str) -> bytes:
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
    new_name = ""
    for char in s:
        if char == "\x81":
            new_name += "\x81\x79"
        if char in '/":*[]+|\\?%<>,;=' or ord(char) < 0x20:
            new_name += "\x81" + chr(0x80 + ord(char))
        else:
            new_name += char
    return "".join(new_name)


def punyencode(orig: str, encoding: str = "mac_roman") -> str:
    s = orig.encode("mac_roman").decode(encoding)
    s = escape_string(s)
    encoded = s.encode("punycode").decode("ascii")
    # punyencoding adds an '-' at the end when there are no special chars
    # don't use it for comparing
    if orig != encoded[:-1]:
        return "xn--" + encoded
    return orig


def encode_string(args: argparse.Namespace) -> None:
    if args.string:
        var = args.string
    if args.stdin:
        var = input()
    print(punyencode(var))


def generate_punyencoded_path(
    destination_dir: Path, encoding: str, hpath: Tuple[str]
) -> Path:
    """Convert a filepath to a punyencoded one"""
    upath = destination_dir

    for el in hpath:
        upath /= punyencode(el, encoding=encoding)
    return upath


def extract_volume(args: argparse.Namespace) -> None:
    source_volume: Path = args.src
    destination_dir: Path = args.dir
    encoding: str = args.e

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


def generate_parser() -> argparse.ArgumentParser:
    """
    Generate the parser

    The parser is split into multiple subparsers.
    One for each mode we support.

    Each subparser has a default function that handles that mode.
    """
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers()
    parser_iso = subparsers.add_parser("iso", help="Dump hfs isos")

    parser_iso.add_argument(
        "-e",
        metavar="ENCODING",
        type=str,
        default="mac_roman",
        help="String encoding (see https://docs.python.org/3/library/codecs.html#standard-encodings)",
    )
    parser_iso.add_argument("src", metavar="INPUT", type=Path, help="Disk image")
    parser_iso.add_argument(
        "dir", metavar="OUTPUT", type=Path, help="Destination folder"
    )
    parser_iso.set_defaults(func=extract_volume)

    parser_str = subparsers.add_parser("str", help="Punyencode strings")
    parser_str.add_argument(
        "--stdin", action="store_true", help="Convert stdin to punycode"
    )
    parser_str.add_argument(
        "string",
        metavar="STRING",
        type=str,
        help="Convert string to punycode",
        nargs="?",
    )
    parser_str.set_defaults(func=encode_string)

    return parser


if __name__ == "__main__":
    parser = generate_parser()
    args = parser.parse_args()
    args.func(args)

### Test functions


def call_test_parser(input_args: List[str]) -> Any:
    """Helper function to call the parser"""
    parser = generate_parser()
    args = parser.parse_args(input_args)
    args.func(args)


def test_encode_string(capsys):
    call_test_parser(["str", "Icon\r"])
    captured = capsys.readouterr()
    assert captured.out == "xn--Icon-ja6e\n"


def test_encode_stdin(capsys, monkeypatch):
    monkeypatch.setattr("sys.stdin", io.StringIO("Icon\r"))
    call_test_parser(["str", "--stdin"])
    captured = capsys.readouterr()
    assert captured.out == "xn--Icon-ja6e\n"


def test_decode_name():
    checks = [["Icon\r", "xn--Icon-ja6e"]]
    for input, expected in checks:
        assert punyencode(input, "mac_roman") == expected


def test_escape_string():
    checks = [["\r", "\x81\x8d"], ["\x81", "\x81\x79\x81"]]
    for input, expected in checks:
        assert escape_string(input) == expected
