#!/usr/bin/env python3

"""Converts an archive to make it go into the secondary ROM of the Nintendo DSi.

The archive is modified so that, when linking it, all its contents are recognized
as DSi-only code by the linker script.
This works by renaming the archive entries to make their base name end with .twl.

This command takes an existing archive as its first argument and, optionaly, an
output archive where the result will be written.
If the second argument is not provided, the input archive is erased with the result,
if there is any change.
"""

# ruff: noqa: Q000

from __future__ import annotations

import os
import pathlib
import struct
import sys
import tempfile
import typing

if typing.TYPE_CHECKING:
    import io


class ArchiveError(Exception):
    """An error from Archive format."""

class Entry: # pylint: disable=too-many-instance-attributes
    """Represents an entry from an Archive file."""

    __slots__ = ('identifier', 'timestamp', 'owner_id', 'group_id', # noqa: RUF023
                 'mode', 'size', 'original_offset', 'data')

    def __init__(self, identifier: bytes, timestamp: int | None, # noqa: PLR0913 pylint: disable=too-many-arguments,too-many-positional-arguments
                 owner_id: int | None, group_id: int | None,
                 mode: int | None, size: int) -> None:
        """Initialize an Entry from its parsed attributes."""
        self.identifier = identifier
        self.timestamp = timestamp
        self.owner_id = owner_id
        self.group_id = group_id
        self.mode = mode
        self.size = size
        self.original_offset: int|None = None
        self.data: bytearray|None = None

    def map_name(self, names: dict[int, bytes] | None) -> None:
        """Retrieve the long name from the provided nametable.

        The identifier is modified in place.
        :param names: the nametable where to look at
        """
        if self.identifier in {b'/', b'//'}:
            return

        if not self.identifier.startswith(b'/'):
            return

        if names is None:
            msg = "Missing long names entry"
            raise ArchiveError(msg)

        self.identifier = names[int(self.identifier[1:])]

    def unmap_name(self, names: bytearray) -> None:
        """Store a long identifier in the provided nametable.

        Names shorter or equal than 16 don't need it.
        :param names: the nametable where to place the long name
        """
        if len(self.identifier) <= 16: # noqa: PLR2004
            return

        identifier = self.identifier
        self.identifier = b'/' + str(len(names)).encode('ascii')
        names.extend(identifier + b'\n')

    def make_twl(self) -> bool:
        """Transform the identifier in a TWL one.

        This means adding .twl at the end of the base name.
        :returns: if the name was changed
        """
        if b'.' not in self.identifier:
            return False

        base, ext = self.identifier.rsplit(b'.', maxsplit=1)
        if base.endswith(b'.twl'):
            return False

        self.identifier = base + b'.twl.' + ext
        return True

    def tobytes(self) -> bytes:
        """Render the entry as bytes for writing."""
        return (self.identifier[:16].ljust(16) +
                gen_int(self.timestamp, 12) +
                gen_int(self.owner_id, 6) +
                gen_int(self.group_id, 6) +
                gen_int(self.mode, 8) +
                gen_int(self.size, 10) + b'`\n')

    def __repr__(self) -> str:
        """Return a represation of the Entry."""
        return (f"Entry({self.identifier!r}, {self.timestamp}, {self.owner_id}, "
                f"{self.group_id}, {self.mode}, {self.size})")

def parse_int(v: bytes) -> int | None:
    """Parse an int from a byte string.

    :param v: the value as a byte string
    :returns: the value as an int or None if the string was empty
    """
    v = v.rstrip()
    return int(v) if v else None

def gen_int(v: int | None, sz: int) -> bytes:
    """Render an int into a byte string.

    :param v: the value as an int or None
    :param sz: the size of the resulting byte string
    :returns: the value as a byte string
    """
    if v is None:
        return b' ' * sz
    sv = str(v).encode('ascii')
    return sv[:sz].ljust(sz)

def read_nametable(f: io.BufferedReader, size: int) -> dict[int, bytes]:
    """Read and parse a name table.

    :param f: the archive to read the names from
    :param size: the size of the table
    :returns: a mapping between the name index (the offset) and the name
    """
    names = {}
    data = f.read(size)
    i = 0
    while True:
        ni = data.find(b'\n', i)
        if ni == -1:
            break
        names[i] = data[i:ni]
        i = ni + 1
    return names

def set_nametable(entries: list[Entry], names: bytearray) -> int:
    """Set a new name table for the archive.

    :param entries: the entries list used to find the table
    :param names: the names byte array to set
    :returns: the size offset between the old and the new name tables
    """
    if len(names) == 0:
        # New nametable is empty: remove the old one if it exists
        for i, entry in enumerate(entries):
            if entry != b'//':
                continue
            del entries[i]
            return -(60 + entry.size + entry.size & 1)
        return 0

    # First entry may be the AR map
    i = 0
    if len(entries) > i and entries[i].identifier == b'/':
        i += 1

    # The second entry is an existing name table: replace it
    if len(entries) > i and entries[i].identifier == b'//':
        entries[i].data = names
        oldsize = entries[i].size
        newsize = len(names)
        entries[i].size = newsize
        return (newsize + (newsize & 1)) - (oldsize + (oldsize & 1))

    # This is a new entry
    entry = Entry(b'//', None, None, None, None, len(names))
    entry.data = names
    entries.insert(i, entry)
    return 60 + entry.size + entry.size & 1

def fixup_armap(f: io.BufferedReader, entries: list[Entry], offset: int) -> None:
    """Fix the AR index map present at the start.

    When patching the name table, we get an offset which must be
    applied back to all entries in the index.
    :param f: the input file from where to read the index
    :param entries: the entries list used to find the index
    :param offset: the offset to apply to every entry in the list
    """
    if len(entries) == 0:
        return
    if entries[0].identifier != b'/':
        return

    entry = entries[0]

    if entry.original_offset is None:
        msg = "Entry has no offset into original file"
        raise RuntimeError(msg)

    f.seek(entry.original_offset)
    data = bytearray(entry.size)
    f.readinto(data)

    map_size, = struct.unpack('>L', data[:4])
    for i in range(map_size):
        value, = struct.unpack('>L', data[4+i*4:4+i*4+4])
        value += offset
        data[4+i*4:4+i*4+4] = struct.pack('>L', value)

    entry.data = data

def read_entry(f: io.BufferedReader) -> Entry | None:
    """Read an entry from the archive.

    :param f: the input file
    :returns: an Entry or None if EOF is reached
    """
    file_header = f.read(60)
    if not file_header:
        return None

    identifier, timestamp, owner_id, group_id, mode, size, end = struct.unpack(
            '16s12s6s6s8s10s2s', file_header)
    if end != b'`\n':
        msg = "Invalid file entry end"
        raise ArchiveError(msg)

    identifier = identifier.rstrip()
    timestamp = parse_int(timestamp)
    owner_id = parse_int(owner_id)
    group_id = parse_int(group_id)
    mode = parse_int(mode)
    size = parse_int(size)
    if size is None:
        msg = "Invalid size in entry"
        raise ArchiveError(msg)

    return Entry(identifier, timestamp, owner_id, group_id, mode, size)

def read_entries(f: io.BufferedReader) -> list[Entry]:
    """Read the archive entries.

    Also parse the nametable to give entries their long name.
    :param f: the input file
    :returns: a list of the entries from the file
    """
    names = None
    entries = []

    magic = f.read(8)
    if magic != b'!<arch>\n':
        msg = "Invalid archive magic"
        raise ArchiveError(msg)

    while True:
        entry = read_entry(f)
        if entry is None:
            break

        entry.map_name(names)
        entry.original_offset = f.tell()
        entries.append(entry)

        if entry.identifier == b'//':
            names = read_nametable(f, entry.size)
        else:
            f.seek(entry.size, os.SEEK_CUR)
        f.read(entry.size & 1)

    return entries

def write_entries(f: io.BufferedReader, of: io.BufferedWriter,
                  entries: list[Entry]) -> None:
    """Write all the archive entries to the output file.

    It either uses the data linked to the entry or the original data
    in the input file.
    :param f: the input file
    :param of: the output file
    :param entries: the list of entries which must be written
    """
    of.write(b'!<arch>\n')
    for entry in entries:
        of.write(entry.tobytes())
        if entry.data is not None:
            of.write(entry.data)
        elif entry.original_offset is not None:
            f.seek(entry.original_offset)
            of.write(f.read(entry.size))
        else:
            msg = "Entry has no data nor offset into original file"
            raise RuntimeError(msg)
        if entry.size & 0x1:
            of.write(b'\n')

def main() -> None:
    """Do the job."""
    tf = None
    archive_file = pathlib.Path(sys.argv[1])
    with archive_file.open('rb') as f:
        entries = read_entries(f)
        names = bytearray()
        changed = False
        for entry in entries:
            changed |= entry.make_twl()
            entry.unmap_name(names)

        offset = set_nametable(entries, names)
        fixup_armap(f, entries, offset)

        if len(sys.argv) >= 3: # noqa: PLR2004
            h: pathlib.Path|int = pathlib.Path(sys.argv[2])
        else:
            if not changed:
                return
            h, tf_ = tempfile.mkstemp(dir=archive_file.parent)
            tf = pathlib.Path(tf_)
        with open(h, 'wb') as of: # noqa: PTH123
            write_entries(f, of, entries)

    if tf is not None:
        tf.replace(archive_file)

if __name__ == '__main__':
    main()
