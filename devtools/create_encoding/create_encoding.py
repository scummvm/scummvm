#!/usr/bin/env python3
# encoding: utf-8

import struct
from hanzi_charmap import simplified_charmap, traditional_charmap

def merge_ranges(*ranges):
    # Use a set for efficient lookup
    result = set()
    for rng in ranges:
        result.update(rng)
    return result

def processtable(inputfilename, outfile, highrange, lowrange):
    with open(inputfilename) as f:
        res = dict(((x << 8) | y, 0) for x in highrange for y in lowrange)

        for line in f:
            if line.startswith('#'):
                continue
            splits = line.split()
            if len(splits[0]) <= 5:
                continue
            key = int(splits[0], 16)
            val = int(splits[1], 16)
            high = (key >> 8) & 0xff
            low = key & 0xff
            if high not in highrange or low not in lowrange:
                continue
            res[key] = val

    for (k, v) in sorted(res.items()):
        outfile.write(struct.pack("<H", v))

def process_hanzi_t2s(outfile):
    l = min(len(traditional_charmap), len(simplified_charmap))
    outfile.write(struct.pack("<i", l))
    for i in range(l):
        outfile.write(struct.pack("<HH", ord(traditional_charmap[i]), ord(simplified_charmap[i])))
    
encdat = open("encoding.dat", "wb")
encdat.write(b'SCVMENCD')
# version
encdat.write(struct.pack("<i", 0))

HIGH = 'high'
LOW = 'low'
FILE = 'file'

tables = [
    {
        FILE: "CP932.TXT",
        HIGH: merge_ranges(range(0x81, 0x85), range(0x87, 0xa0), range(0xe0, 0xef), range(0xfa, 0xfd)),
        LOW: range(0x40, 0x100)
    },
    {
        FILE: "CP949.TXT",
        HIGH: range(0x81, 0xFF),
        LOW: merge_ranges(range(0x41, 0x5b), range(0x61, 0x7b), range(0x81, 0xFF))
    },
    {
        FILE: "CP950.TXT",
        HIGH: range(0xA1, 0xFA),
        LOW: merge_ranges(range(0x40, 0x7f), range(0xa1, 0xff))
    },
    {
        FILE: "JOHAB.TXT",
        HIGH: range(0x84, 0xD4),
        LOW: merge_ranges(range(0x41, 0x7f), range(0x81, 0xff))
    },
    {
        FILE: "CP936.TXT",
        HIGH: range(0x81, 0xFF),
        LOW: merge_ranges(range(0x40, 0x7f), range(0x80, 0xff))
    },
    {
        FILE: "hanzi_charmap.py"
    },
]

# number of tables
encdat.write(struct.pack("<i", len(tables)))

curofs = 16 + 4 * len(tables)

for v in tables:
    encdat.write(struct.pack("<i", curofs))
    if v[FILE] == "hanzi_charmap.py":
        curofs += min(len(traditional_charmap), len(simplified_charmap)) * 4 + 4
    else:
        curofs += len(v[HIGH]) * len(v[LOW]) * 2

for v in tables:
    if v[FILE] == "hanzi_charmap.py":
        process_hanzi_t2s(encdat)
    else:
        processtable(v[FILE], encdat, v[HIGH], v[LOW])

