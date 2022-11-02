#!/usr/bin/env python

"""
This script generates subtitles for Toonstrack cutscenes
in a single SUBTITLES.PAK file from given directory of .SBV subtitles

Usage:
```
pakdir.py SUBTITLES_DIR
````
"""

import struct
from itertools import chain

from submaker import sbv2tss

def write_uint32_le(number):
    return struct.pack('<I', number)

def calculate_index_length(pak_index):
    return sum(len(write_index_entry(fname, 0)) for fname in pak_index)

def write_index_entry(fname, offset):
    return write_uint32_le(offset) + fname.encode() + b'\00'

def generate_index(data_files):
    end = ('\00\00\00\00', b'')
    pak_index, rdata = zip(*chain(data_files, (end,)))
    off = calculate_index_length(pak_index)
    for fname, fdata in zip(pak_index, rdata):
        yield write_index_entry(fname, off), fdata
        off += len(fdata)

def create_entry(filename):
    name, _ = os.path.splitext(os.path.basename(filename))
    return name + '.tss', ''.join(sbv2tss(filename)).encode()

if __name__ == "__main__":
    import os
    import glob
    import sys

    if len(sys.argv) < 2:
        print("Usage: pakdir.py SUBTITLES_DIR")
        exit(1)

    paths = sys.argv[1:]
    paths = (os.path.join(path, '*.sbv') for path in paths)
    files = sorted(set(chain.from_iterable(glob.iglob(r) for r in paths)))
    print(files)
    index, data = zip(*generate_index(create_entry(filename) for filename in files))
    with open('SUBTITLES.PAK', 'wb') as output:
        output.write(b''.join(index))
        output.write(b''.join(data))
