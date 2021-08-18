#!/usr/bin/env python3
#
# prerequisites: pip3 install machfs
#
# Development information:
# This file contains tests. They can be run with:
#  $ pytest dumper-companion.py
#
# Code is formatted with black

import argparse
import io
import os
import sys
from binascii import crc_hqx
from pathlib import Path
from struct import pack, unpack
from typing import Any, List, Tuple

import machfs

if sys.platform == "darwin":
    try:
        import xattr
    except ImportError:
        print("xattr is required for the 'mac' mode to work\n")


def file_to_macbin(f: machfs.File, name: str, encoding: str) -> bytes:
    oldFlags = f.flags >> 8
    newFlags = f.flags & 8
    macbin = pack(
        ">xB63s4s4sBxHHHBxIIIIHB14xIHBB",
        len(name),  # TODO: shouldn't this be the encoded file length?
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
        f.crdate,  # TODO: dates are wrong, investigate
        f.mddate,  # TODO: dates are wrong, investigate
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
    return new_name


def punyencode(orig: str) -> str:
    s = escape_string(orig)
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
    return 0


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

    destination_dir.mkdir(parents=True, exist_ok=True)

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
    return 0


def has_resource_fork(dirpath: str, filename: str) -> bool:
    """
    Check if file has a resource fork

    Ease of compatibility between macOS and linux
    """
    filepath = os.path.join(dirpath, filename)
    return os.path.exists(os.path.join(filepath, "..namedfork/rsrc"))


def collect_forks(args: argparse.Namespace) -> None:
    """
    Collect resource forks and move them to a macbinary file

    - combine them with the data fork when it's available
    - punyencode the filename when requested
    """
    directory: Path = args.dir
    punify: bool = args.punycode
    count_resources = 0
    count_renames = 0
    for dirpath, _, filenames in os.walk(directory):
        for filename in filenames:
            if has_resource_fork(dirpath, filename):
                print(f"Resource in {filename}")
                count_resources += 1
                resource_filename = filename + "/..namedfork/rsrc"
                to_filename = filename

                filepath = os.path.join(dirpath, filename)
                resourcepath = os.path.join(dirpath, resource_filename)

                if punify:
                    tmp = punyencode(to_filename)
                    if tmp != to_filename:
                        print(f"Renamed {to_filename} to {tmp}")
                        count_renames += 1
                    to_filename = tmp

                to_filepath = os.path.join(dirpath, filename)

                file = machfs.File()

                # Set the file times and convert them to Mac epoch
                info = os.stat(filename)
                file.crdate = 2082844800 + int(info.st_birthtime)
                file.mddate = 2082844800 + int(info.st_mtime)

                # Get info on creator and type
                try:
                    finderInfo = xattr.xattr(filepath)["com.apple.FinderInfo"][0:8]
                except (IOError, OSError) as e:
                    print(f"Error getting type and creator for: {filename}")
                    return 1

                file.type, file.creator = unpack("4s4s", finderInfo)

                with open(resourcepath, "rb") as rsrc:
                    file.rsrc = rsrc.read()
                with open(filepath, "rb") as data:
                    file.data = data.read()
                with open(to_filepath, "wb") as to_file:
                    to_file.write(
                        file_to_macbin(file, to_filename, encoding="mac_roman")
                    )

                    if to_filename != filename:
                        os.remove(filepath)  # Remove the original file

                    os.utime(
                        to_filepath,
                        (info.st_mtime, info.st_mtime),
                    )
            elif punify:
                punified_filename = punyencode(filename)
                if punified_filename != filename:
                    print(f"Renamed {to_filename} to {punified_filename}")
                    count_renames += 1
                    os.rename(
                        os.path.join(dirpath, tmp),
                        os.path.join(dirpath, punified_filename),
                    )

    print(f"Macbinary {count_resources}, Renamed {count_renames} files")
    return 0


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

    parser_iso.add_argument("src", metavar="INPUT", type=Path, help="Disk image")
    parser_iso.add_argument(
        "dir", metavar="OUTPUT", type=Path, help="Destination folder"
    )
    parser_iso.set_defaults(func=extract_volume)

    parser_str = subparsers.add_parser("str", help="Punyencode strings or standard in")
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

    if sys.platform == "darwin":
        parser_macbinary = subparsers.add_parser(
            "mac",
            help="MacOS only: Operate in MacBinary encoding mode. Recursively encode all resource forks in the current directory",
        )
        parser_macbinary.add_argument(
            "--punycode", action="store_true", help="encode pathnames into punycode"
        )
        parser_macbinary.add_argument(
            "dir", metavar="directory", type=Path, help="input directory"
        )
        parser_macbinary.set_defaults(func=collect_forks)

    return parser


if __name__ == "__main__":
    parser = generate_parser()
    args = parser.parse_args()
    exit(args.func(args))

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
    checks = [
        ["Icon\r", "xn--Icon-ja6e"],
        ["バッドデイ(Power PC)", "xn--(Power PC)-jx4ilmwb1a7h"],
    ]
    for input, expected in checks:
        assert punyencode(input) == expected


def test_escape_string():
    checks = [["\r", "\x81\x8d"], ["\x81", "\x81\x79\x81"]]
    for input, expected in checks:
        assert escape_string(input) == expected
