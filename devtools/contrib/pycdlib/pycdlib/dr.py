# Copyright (C) 2015-2022  Chris Lalancette <clalancette@gmail.com>

# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation;
# version 2.1 of the License.

# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

"""
The class to support ISO9660 Directory Records.
"""

import bisect
import struct

from pycdlib import dates
from pycdlib import inode
from pycdlib import pycdlibexception
from pycdlib import rockridge
from pycdlib import utils

# For mypy annotations
if False:  # pylint: disable=using-constant-test
    from typing import Any, IO, List, Optional, Tuple, Union  # NOQA pylint: disable=unused-import
    # NOTE: these imports have to be here to avoid circular deps
    from pycdlib import headervd  # NOQA pylint: disable=unused-import,cyclic-import
    from pycdlib import path_table_record  # NOQA pylint: disable=unused-import


class XARecord:
    """
    A class that represents an ISO9660 Extended Attribute record as defined
    in the Philips Yellow Book standard.
    """
    __slots__ = ('_initialized', '_group_id', '_user_id', '_attributes',
                 '_filenum', '_pad_size')

    FMT = '=HHH2sB5s'

    def __init__(self):
        # type: () -> None
        self._pad_size = 0
        self._initialized = False

    def parse(self, xastr, len_fi):
        # type: (bytes, int) -> bool
        """
        Parse an Extended Attribute Record out of a string.

        Parameters:
         xastr - The string to parse.
         len_fi - The length of the file identifier for this record.
        Returns:
         True if the data contains an XA record, False otherwise.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This XARecord is already initialized')

        even_size = len_fi + (len_fi % 2)
        # In a "typical" XA record, the record immediately follows the DR
        # record (but comes before the Rock Ridge record, if this is a Rock
        # Ridge ISO).  However, we have seen ISOs (Windows 98 SE) that put some
        # padding between the end of the DR record and the XA record.  As far
        # as I can tell, that padding is the size of the file identifier,
        # but rounded up to the nearest even number.  We check both places for
        # the XA record.
        for offset in (0, even_size):
            parse_str = xastr[offset:]
            if len(parse_str) < struct.calcsize(self.FMT):
                return False

            (self._group_id, self._user_id, self._attributes, signature,
             self._filenum, unused) = struct.unpack_from(self.FMT, parse_str, 0)
            if signature != b'XA':
                continue

            if unused != b'\x00\x00\x00\x00\x00':
                raise pycdlibexception.PyCdlibInvalidISO('Unused fields should be 0')

            self._pad_size = offset
            break
        else:
            return False

        self._initialized = True

        return True

    def new(self):
        # type: () -> None
        """
        Create a new Extended Attribute Record.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This XARecord is already initialized')

        # FIXME: we should allow the user to set these
        self._group_id = 0
        self._user_id = 0
        self._attributes = 0
        self._filenum = 0
        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Record this Extended Attribute Record.

        Parameters:
         None.
        Returns:
         A string representing this Extended Attribute Record.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This XARecord is not initialized')

        return b'\x00' * self._pad_size + struct.pack(self.FMT, self._group_id,
                                                      self._user_id,
                                                      self._attributes,
                                                      b'XA', self._filenum,
                                                      b'\x00' * 5)

    @staticmethod
    def length():
        # type: () -> int
        """
        A static method to return the size of an Extended Attribute Record.

        Parameters:
         None.
        Returns:
         The size of an Extended Attribute Record.
        """
        return 14


class DirectoryRecord:
    """A class that represents an ISO9660 directory record."""
    __slots__ = ('initialized', 'new_extent_loc', 'ptr', 'extents_to_here',
                 'offset_to_here', 'data_continuation', 'vd', 'children',
                 'rr_children', 'inode', '_printable_name', 'date',
                 'index_in_parent', 'dr_len', 'xattr_len', 'file_flags',
                 'file_unit_size', 'interleave_gap_size', 'len_fi', 'isdir',
                 'orig_extent_loc', 'data_length', 'seqnum', 'is_root',
                 'parent', 'rock_ridge', 'xa_record', 'file_ident')

    FILE_FLAG_EXISTENCE_BIT = 0
    FILE_FLAG_DIRECTORY_BIT = 1
    FILE_FLAG_ASSOCIATED_FILE_BIT = 2
    FILE_FLAG_RECORD_BIT = 3
    FILE_FLAG_PROTECTION_BIT = 4
    FILE_FLAG_MULTI_EXTENT_BIT = 7

    FMT = '<BBLLLL7sBBBHHB'

    def __init__(self):
        # type: () -> None
        self.initialized = False
        self.new_extent_loc = -1
        self.ptr = None  # type: Optional[path_table_record.PathTableRecord]
        self.extents_to_here = 1
        self.offset_to_here = 0
        self.data_continuation = None  # type: Optional[DirectoryRecord]
        self.children = []  # type: List[DirectoryRecord]
        self.rr_children = []  # type: List[DirectoryRecord]
        self.index_in_parent = -1
        self.is_root = False
        self.isdir = False
        self.rock_ridge = None  # type: Optional[rockridge.RockRidge]
        self.xa_record = None  # type: Optional[XARecord]
        self.inode = None  # type: Optional[inode.Inode]

    def parse(self, vd, record, parent):
        # type: (headervd.PrimaryOrSupplementaryVD, bytes, Optional[DirectoryRecord]) -> str
        """
        Parse a directory record out of a string.

        Parameters:
         vd - The Volume Descriptor this record is part of.
         record - The string to parse for this record.
         parent - The parent of this record.
        Returns:
         The Rock Ridge version as a string if this Directory Record has Rock
         Ridge, '' otherwise.
        """
        if self.initialized:
            raise pycdlibexception.PyCdlibInternalError('Directory Record already initialized')

        if len(record) > 255:
            # Since the length is supposed to be 8 bits, this should never
            # happen.
            raise pycdlibexception.PyCdlibInvalidISO('Directory record longer than 255 bytes!')

        # According to http://www.dubeyko.com/development/FileSystems/ISO9960/ISO9960.html,
        # the xattr_len is the number of bytes at the *beginning* of the file
        # extent.  Since this is only a byte, it is necessarily limited to 255
        # bytes.
        (self.dr_len, self.xattr_len, extent_location_le, extent_location_be,
         data_length_le, data_length_be_unused, dr_date, self.file_flags,
         self.file_unit_size, self.interleave_gap_size, seqnum_le, seqnum_be,
         self.len_fi) = struct.unpack_from(self.FMT, record[:33], 0)

        # In theory we should have a check here that checks to make sure that
        # the length of the record we were passed in matches the data record
        # length.  However, we have seen ISOs in the wild where this is
        # incorrect, so we elide the check here.

        if extent_location_le != utils.swab_32bit(extent_location_be):
            raise pycdlibexception.PyCdlibInvalidISO('Little-endian (%d) and big-endian (%d) extent location disagree' % (extent_location_le, utils.swab_32bit(extent_location_be)))
        self.orig_extent_loc = extent_location_le

        # Theoretically, we should check to make sure that the little endian
        # data length is the same as the big endian data length.  In practice,
        # though, we've seen ISOs where this is wrong.  Skip the check, and just
        # pick the little-endian as the 'actual' size, and hope for the best.

        self.data_length = data_length_le

        if seqnum_le != utils.swab_16bit(seqnum_be):
            raise pycdlibexception.PyCdlibInvalidISO('Little-endian and big-endian seqnum disagree')
        self.seqnum = seqnum_le

        self.date = dates.DirectoryRecordDate()
        self.date.parse(dr_date)

        # OK, we've unpacked what we can from the beginning of the string.  Now
        # we have to use the len_fi to get the rest.

        self.parent = parent
        self.vd = vd

        if self.parent is None:
            self.is_root = True

            # A root directory entry should always be exactly 34 bytes.
            # However, we have seen ISOs in the wild that get this wrong, so we
            # elide a check for it.

            self.file_ident = bytes(bytearray([record[33]]))

            # A root directory entry should always have 0 as the identifier.
            # However, we have seen ISOs in the wild that don't have this set
            # properly to 0.  In that case, we override what we parsed out from
            # the original with the correct value (\x00), and hope for the best.
            if self.file_ident != b'\x00':
                self.file_ident = b'\x00'
            self.isdir = True
        else:
            record_offset = 33
            self.file_ident = record[record_offset:record_offset + self.len_fi]
            record_offset += self.len_fi
            if self.file_flags & (1 << self.FILE_FLAG_DIRECTORY_BIT):
                self.isdir = True

            if self.len_fi % 2 == 0:
                record_offset += 1

        if self.is_root:
            self._printable_name = '/'.encode(vd.encoding)
        elif self.file_ident == b'\x00':
            self._printable_name = '.'.encode(vd.encoding)
        elif self.file_ident == b'\x01':
            self._printable_name = '..'.encode(vd.encoding)
        else:
            self._printable_name = self.file_ident

        if self.parent is not None:
            xa_rec = XARecord()
            if xa_rec.parse(record[record_offset:], self.len_fi):
                self.xa_record = xa_rec
                record_offset += len(self.xa_record.record())

            if len(record[record_offset:]) >= 2 and \
               record[record_offset:record_offset + 2] in (b'SP', b'RR', b'CE', b'PX', b'ER', b'ES', b'PN', b'SL', b'NM', b'CL', b'PL', b'TF', b'SF', b'RE', b'AL'):
                self.rock_ridge = rockridge.RockRidge()

                is_first_dir_record_of_root = False

                if self.parent.is_root:
                    if self.file_ident == b'\x00':
                        is_first_dir_record_of_root = True
                        bytes_to_skip = 0
                    else:
                        if not self.parent.children:
                            raise pycdlibexception.PyCdlibInvalidISO('Parent has no dot child')
                        if self.parent.children[0].rock_ridge is None:
                            raise pycdlibexception.PyCdlibInvalidISO('Dot child does not have Rock Ridge; ISO is corrupt')
                        bytes_to_skip = self.parent.children[0].rock_ridge.bytes_to_skip
                else:
                    if self.parent.rock_ridge is None:
                        raise pycdlibexception.PyCdlibInvalidISO('Parent does not have Rock Ridge; ISO is corrupt')
                    bytes_to_skip = self.parent.rock_ridge.bytes_to_skip

                self.rock_ridge.parse(record[record_offset:],
                                      is_first_dir_record_of_root,
                                      bytes_to_skip,
                                      False,
                                      self._printable_name)

        if self.xattr_len != 0:
            if self.file_flags & (1 << self.FILE_FLAG_RECORD_BIT):
                raise pycdlibexception.PyCdlibInvalidISO('Record Bit not allowed with Extended Attributes')
            if self.file_flags & (1 << self.FILE_FLAG_PROTECTION_BIT):
                raise pycdlibexception.PyCdlibInvalidISO('Protection Bit not allowed with Extended Attributes')

        if self.rock_ridge is None:
            ret = ''
        else:
            ret = self.rock_ridge.rr_version

        self.initialized = True

        return ret

    def _rr_new(self, rr_version, rr_name, rr_symlink_target, rr_relocated_child,
                rr_relocated, rr_relocated_parent, file_mode, date_seconds):
        # type: (str, bytes, bytes, bool, bool, bool, int, float) -> None
        """
        Internal method to add Rock Ridge to a Directory Record.

        Parameters:
         rr_version - A string containing the version of Rock Ridge to use for
                      this record.
         rr_name - The Rock Ridge name to associate with this directory record.
         rr_symlink_target - The target for the symlink, if this is a symlink
                             record (otherwise, None).
         rr_relocated_child - True if this is a directory record for a rock
                              ridge relocated child.
         rr_relocated - True if this is a directory record for a relocated
                        entry.
         rr_relocated_parent - True if this is a directory record for a rock
                               ridge relocated parent.
         file_mode - The Unix file mode for this Rock Ridge entry.
         date_seconds - Time and date, in seconds since the epoch, to use for
                        this directory record.
        Returns:
         Nothing.
        """

        if self.parent is None:
            raise pycdlibexception.PyCdlibInternalError('Invalid call to create new Rock Ridge on root directory')

        self.rock_ridge = rockridge.RockRidge()
        is_first_dir_record_of_root = self.file_ident == b'\x00' and self.parent.is_root
        bytes_to_skip = 0
        if self.xa_record is not None:
            bytes_to_skip = XARecord.length()
        self.dr_len = self.rock_ridge.new(is_first_dir_record_of_root, rr_name,
                                          file_mode, rr_symlink_target,
                                          rr_version, rr_relocated_child,
                                          rr_relocated, rr_relocated_parent,
                                          bytes_to_skip, self.dr_len, {}, date_seconds)

        # For files, we are done
        if not self.isdir:
            return

        # If this is a directory, we have to manipulate the file links
        # appropriately.
        if self.parent.is_root:
            if self.file_ident in (b'\x00', b'\x01'):
                # For the dot and dotdot children of the root, add one
                # directly to their Rock Ridge links.
                self.rock_ridge.add_to_file_links()
            else:
                # For all other children of the root, make sure to add one
                # to each of the dot and dotdot entries.
                if len(self.parent.children) < 2:
                    raise pycdlibexception.PyCdlibInvalidISO('Expected at least 2 children of the root directory record, saw %d' % (len(self.parent.children)))

                if self.parent.children[0].rock_ridge is None:
                    raise pycdlibexception.PyCdlibInvalidISO('Dot child of directory has no Rock Ridge; ISO is corrupt')
                self.parent.children[0].rock_ridge.add_to_file_links()

                if self.parent.children[1].rock_ridge is None:
                    raise pycdlibexception.PyCdlibInvalidISO('Dot-dot child of directory has no Rock Ridge; ISO is corrupt')
                self.parent.children[1].rock_ridge.add_to_file_links()
        else:
            if self.parent.rock_ridge is None:
                raise pycdlibexception.PyCdlibInvalidISO('Parent of the entry did not have Rock Ridge, ISO is corrupt')

            if self.file_ident == b'\x00':
                # If we are adding the dot directory, increment the parent
                # file links and our file links.
                self.parent.rock_ridge.add_to_file_links()
                self.rock_ridge.add_to_file_links()
            elif self.file_ident == b'\x01':
                # If we are adding the dotdot directory, copy the file links
                # from the dot directory of the grandparent.
                if self.parent.parent is None:
                    raise pycdlibexception.PyCdlibInternalError('Grandparent of the entry did not exist; this cannot be')
                if not self.parent.children:
                    raise pycdlibexception.PyCdlibInvalidISO('Grandparent of the entry did not have a dot entry; ISO is corrupt')
                if self.parent.parent.children[0].rock_ridge is None:
                    raise pycdlibexception.PyCdlibInvalidISO('Grandparent dotdot entry did not have Rock Ridge; ISO is corrupt')
                self.rock_ridge.copy_file_links(self.parent.parent.children[0].rock_ridge)
            else:
                # For all other entries, increment the parents file links
                # and the parents dot file links.
                self.parent.rock_ridge.add_to_file_links()

                if not self.parent.children:
                    raise pycdlibexception.PyCdlibInvalidISO('Parent of the entry did not have a dot entry; ISO is corrupt')
                if self.parent.children[0].rock_ridge is None:
                    raise pycdlibexception.PyCdlibInvalidISO('Dot child of the parent did not have a dot entry; ISO is corrupt')
                self.parent.children[0].rock_ridge.add_to_file_links()

    def _new(self, vd, name, parent, seqnum, isdir, length, xa, date_seconds):
        # type: (headervd.PrimaryOrSupplementaryVD, bytes, Optional[DirectoryRecord], int, bool, int, bool, float) -> None
        """
        Internal method to create a new Directory Record.

        Parameters:
         vd - The Volume Descriptor this record is part of.
         name - The name for this directory record.
         parent - The parent of this directory record.
         seqnum - The sequence number to associate with this directory record.
         isdir - Whether this directory record represents a directory.
         length - The length of the data for this directory record.
         xa - True if this is an Extended Attribute record.
         date_seconds - Time and date, in seconds since the epoch, to use for
                        this directory record.
        Returns:
         Nothing.
        """

        # Adding a new time should really be done when we are going to write
        # the ISO (in record()).  Ecma-119 9.1.5 says:
        #
        # 'This field shall indicate the date and the time of the day at which
        # the information in the Extent described by the Directory Record was
        # recorded.'
        #
        # We create it here just to have something in the field, but we'll
        # redo the whole thing when we are mastering.
        self.date = dates.DirectoryRecordDate()
        self.date.new(date_seconds)

        if length > 2**32 - 1:
            raise pycdlibexception.PyCdlibInvalidInput('Maximum supported file length is 2^32-1')

        self.data_length = length

        self.file_ident = name

        self.isdir = isdir

        self.seqnum = seqnum
        # For a new directory record entry, there is no original_extent_loc,
        # so we leave it at None.
        self.orig_extent_loc = None
        self.len_fi = len(self.file_ident)
        self.dr_len = struct.calcsize(self.FMT) + self.len_fi

        # From Ecma-119, 9.1.6, the file flag bits are:
        #
        # Bit 0 - Existence - 0 for existence known, 1 for hidden
        # Bit 1 - Directory - 0 for file, 1 for directory
        # Bit 2 - Associated File - 0 for not associated, 1 for associated
        # Bit 3 - Record - 0=structure not in xattr, 1=structure in xattr
        # Bit 4 - Protection - 0=no owner and group, 1=owner and group in xattr
        # Bit 5 - Reserved
        # Bit 6 - Reserved
        # Bit 7 - Multi-extent - 0=final directory record, 1=not final directory record
        self.file_flags = 0
        if self.isdir:
            self.file_flags |= (1 << self.FILE_FLAG_DIRECTORY_BIT)
        self.file_unit_size = 0  # FIXME: we don't support setting file unit size for now
        self.interleave_gap_size = 0  # FIXME: we don't support setting interleave gap size for now
        self.xattr_len = 0  # FIXME: we don't support xattrs for now

        self.parent = parent
        if parent is None:
            # If no parent, then this is the root
            self.is_root = True

        if xa:
            self.xa_record = XARecord()
            self.xa_record.new()
            self.dr_len += XARecord.length()

        self.dr_len += (self.dr_len % 2)

        if self.is_root:
            self._printable_name = '/'.encode(vd.encoding)
        elif self.file_ident == b'\x00':
            self._printable_name = '.'.encode(vd.encoding)
        elif self.file_ident == b'\x01':
            self._printable_name = '..'.encode(vd.encoding)
        else:
            self._printable_name = self.file_ident

        self.vd = vd

        self.initialized = True

    def new_symlink(self, vd, name, parent, rr_target, seqnum, rock_ridge,
                    rr_name, xa, date_seconds):
        # type: (headervd.PrimaryOrSupplementaryVD, bytes, DirectoryRecord, bytes, int, str, bytes, bool, float) -> None
        """
        Create a new symlink Directory Record.  This implies that the new
        record will be Rock Ridge.

        Parameters:
         vd - The Volume Descriptor this record is part of.
         name - The name for this directory record.
         parent - The parent of this directory record.
         rr_target - The symlink target for this directory record.
         seqnum - The sequence number for this directory record.
         rock_ridge - The version of Rock Ridge to use for this directory record.
         rr_name - The Rock Ridge name for this directory record.
         xa - True if this is an Extended Attribute record.
         date_seconds - Time and date, in seconds since the epoch, to use for
                        this symlink.
        Returns:
         Nothing.
        """
        if self.initialized:
            raise pycdlibexception.PyCdlibInternalError('Directory Record already initialized')

        self._new(vd, name, parent, seqnum, False, 0, xa, date_seconds)
        if rock_ridge:
            self._rr_new(rock_ridge, rr_name, rr_target, False, False, False,
                         0o0120555, date_seconds)

    def new_file(self, vd, length, isoname, parent, seqnum, rock_ridge, rr_name,
                 xa, file_mode, date_seconds):
        # type: (headervd.PrimaryOrSupplementaryVD, int, bytes, DirectoryRecord, int, str, bytes, bool, int, float) -> None
        """
        Create a new file Directory Record.

        Parameters:
         vd - The Volume Descriptor this record is part of.
         length - The length of the data.
         isoname - The name for this directory record.
         parent - The parent of this directory record.
         seqnum - The sequence number for this directory record.
         rock_ridge - Whether to make this a Rock Ridge directory record.
         rr_name - The Rock Ridge name for this directory record.
         xa - True if this is an Extended Attribute record.
         file_mode - The POSIX file mode for this entry.
         date_seconds - Time and date, in seconds since the epoch, to use for
                        this file.
        Returns:
         Nothing.
        """
        if self.initialized:
            raise pycdlibexception.PyCdlibInternalError('Directory Record already initialized')

        self._new(vd, isoname, parent, seqnum, False, length, xa, date_seconds)
        if rock_ridge:
            self._rr_new(rock_ridge, rr_name, b'', False, False, False,
                         file_mode, date_seconds)

    def new_root(self, vd, seqnum, log_block_size, date_seconds):
        # type: (headervd.PrimaryOrSupplementaryVD, int, int, float) -> None
        """
        Create a new root Directory Record.

        Parameters:
         vd - The Volume Descriptor this record is part of.
         seqnum - The sequence number for this directory record.
         log_block_size - The logical block size to use.
         date_seconds - Time and date, in seconds since the epoch, to use for
                        this root.
        Returns:
         Nothing.
        """
        if self.initialized:
            raise pycdlibexception.PyCdlibInternalError('Directory Record already initialized')

        self._new(vd, b'\x00', None, seqnum, True, log_block_size, False,
                  date_seconds)

    def new_dot(self, vd, parent, seqnum, rock_ridge, log_block_size, xa,
                file_mode, date_seconds):
        # type: (headervd.PrimaryOrSupplementaryVD, DirectoryRecord, int, str, int, bool, int, float) -> None
        """
        Create a new 'dot' Directory Record.

        Parameters:
         vd - The Volume Descriptor this record is part of.
         parent - The parent of this directory record.
         seqnum - The sequence number for this directory record.
         rock_ridge - Whether to make this a Rock Ridge directory record.
         log_block_size - The logical block size to use.
         xa - True if this is an Extended Attribute record.
         file_mode - The POSIX file mode to set for this directory.
         date_seconds - Time and date, in seconds since the epoch, to use for
                        this dot record.
        Returns:
         Nothing.
        """
        if self.initialized:
            raise pycdlibexception.PyCdlibInternalError('Directory Record already initialized')

        self._new(vd, b'\x00', parent, seqnum, True, log_block_size, xa, date_seconds)
        if rock_ridge:
            self._rr_new(rock_ridge, b'', b'', False, False, False, file_mode,
                         date_seconds)

    def new_dotdot(self, vd, parent, seqnum, rock_ridge, log_block_size,
                   rr_relocated_parent, xa, file_mode, date_seconds):
        # type: (headervd.PrimaryOrSupplementaryVD, DirectoryRecord, int, str, int, bool, bool, int, float) -> None
        """
        Create a new 'dotdot' Directory Record.

        Parameters:
         vd - The Volume Descriptor this record is part of.
         parent - The parent of this directory record.
         seqnum - The sequence number for this directory record.
         rock_ridge - Whether to make this a Rock Ridge directory record.
         log_block_size - The logical block size to use.
         rr_relocated_parent - True if this is a Rock Ridge relocated parent.
         xa - True if this is an Extended Attribute record.
         file_mode - The POSIX file mode to set for this directory.
         date_seconds - Time and date, in seconds since the epoch, to use for
                        this dotdot record.
        Returns:
         Nothing.
        """
        if self.initialized:
            raise pycdlibexception.PyCdlibInternalError('Directory Record already initialized')

        self._new(vd, b'\x01', parent, seqnum, True, log_block_size, xa, date_seconds)
        if rock_ridge:
            self._rr_new(rock_ridge, b'', b'', False, False, rr_relocated_parent,
                         file_mode, date_seconds)

    def new_dir(self, vd, name, parent, seqnum, rock_ridge, rr_name,
                log_block_size, rr_relocated_child, rr_relocated, xa, file_mode,
                date_seconds):
        # type: (headervd.PrimaryOrSupplementaryVD, bytes, DirectoryRecord, int, str, bytes, int, bool, bool, bool, int, float) -> None
        """
        Create a new directory Directory Record.

        Parameters:
         vd - The Volume Descriptor this record is part of.
         name - The name for this directory record.
         parent - The parent of this directory record.
         seqnum - The sequence number for this directory record.
         rock_ridge - Whether to make this a Rock Ridge directory record.
         rr_name - The Rock Ridge name for this directory record.
         log_block_size - The logical block size to use.
         rr_relocated_child - True if this is a Rock Ridge relocated child.
         rr_relocated - True if this is a Rock Ridge relocated entry.
         xa - True if this is an Extended Attribute record.
         file_mode - The POSIX file mode to set for this directory.
         date_seconds - Time and date, in seconds since the epoch, to use for
                        this directory record.
        Returns:
         Nothing.
        """
        if self.initialized:
            raise pycdlibexception.PyCdlibInternalError('Directory Record already initialized')

        self._new(vd, name, parent, seqnum, True, log_block_size, xa,
                  date_seconds)
        if rock_ridge:
            self._rr_new(rock_ridge, rr_name, b'', rr_relocated_child,
                         rr_relocated, False, file_mode, date_seconds)
            if rr_relocated_child and self.rock_ridge:
                # Relocated Rock Ridge entries are not exactly treated as
                # directories, so fix things up here.
                self.isdir = False
                self.file_flags = 0
                self.rock_ridge.add_to_file_links()

    def change_existence(self, is_hidden):
        # type: (bool) -> None
        """
        Change the ISO9660 existence flag of this Directory Record.

        Parameters:
         is_hidden - True if this Directory Record should be hidden,
                     False otherwise.
        Returns:
         Nothing.
        """
        if not self.initialized:
            raise pycdlibexception.PyCdlibInternalError('Directory Record not initialized')

        if is_hidden:
            self.file_flags |= (1 << self.FILE_FLAG_EXISTENCE_BIT)
        else:
            self.file_flags &= ~(1 << self.FILE_FLAG_EXISTENCE_BIT)

    def _recalculate_extents_and_offsets(self, index, logical_block_size):
        # type: (int, int) -> Tuple[int, int]
        """
        Internal method to recalculate the extents and offsets associated with
        children of this directory record.

        Parameters:
         index - The index at which to start the recalculation.
         logical_block_size - The block size to use for comparisons.
        Returns:
         A tuple where the first element is the total number of extents required
         by the children and where the second element is the offset into the
         last extent currently being used.
        """
        if index == 0:
            dirrecord_offset = 0
            num_extents = 1
        else:
            dirrecord_offset = self.children[index - 1].offset_to_here
            num_extents = self.children[index - 1].extents_to_here

        for i in range(index, len(self.children)):
            c = self.children[i]
            dirrecord_len = c.dr_len
            if (dirrecord_offset + dirrecord_len) > logical_block_size:
                num_extents += 1
                dirrecord_offset = 0
            dirrecord_offset += dirrecord_len
            c.extents_to_here = num_extents
            c.offset_to_here = dirrecord_offset
            c.index_in_parent = i

        return num_extents, dirrecord_offset

    def _add_child(self, child, logical_block_size, allow_duplicate,
                   check_overflow):
        # type: (DirectoryRecord, int, bool, bool) -> bool
        """
        An internal method to add a child to this object.  Note that this is
        called both during parsing and when adding a new object to the system,
        so it shouldn't have any functionality that is not appropriate for both.

        Parameters:
         child - The child directory record object to add.
         logical_block_size - The size of a logical block for this volume
                              descriptor.
         allow_duplicate - Whether to allow duplicate names, as there are
                           situations where duplicate children are allowed.
         check_overflow - Whether to check for overflow; if we are parsing, we
                          don't want to do this.
        Returns:
         True if adding this child caused the directory to overflow into another
         extent, False otherwise.
        """
        if not self.isdir:
            raise pycdlibexception.PyCdlibInvalidInput('Trying to add a child to a record that is not a directory')

        # First ensure that this is not a duplicate.  For speed purposes, we
        # recognize that bisect_left will always choose an index to the *left*
        # of a duplicate child.  Thus, to check for duplicates we only need to
        # see if the child to be added is a duplicate with the entry that
        # bisect_left returned.
        index = bisect.bisect_left(self.children, child)
        if index != len(self.children) and self.children[index].file_ident == child.file_ident:
            if not self.children[index].is_associated_file() and not child.is_associated_file():
                if not (self.rock_ridge is not None and self.file_identifier() == b'RR_MOVED'):
                    if not allow_duplicate:
                        raise pycdlibexception.PyCdlibInvalidInput('Failed adding duplicate name to parent')

                    self.children[index].data_continuation = child
                    self.children[index].file_flags |= (1 << self.FILE_FLAG_MULTI_EXTENT_BIT)
                    index += 1
        self.children.insert(index, child)

        if child.rock_ridge is not None and not child.is_dot() and not child.is_dotdot():
            lo = 0
            hi = len(self.rr_children)
            while lo < hi:
                mid = (lo + hi) // 2
                rr = self.rr_children[mid].rock_ridge
                if rr is not None:
                    if rr.name() < child.rock_ridge.name():
                        lo = mid + 1
                    else:
                        hi = mid
                else:
                    raise pycdlibexception.PyCdlibInternalError('Expected all children to have Rock Ridge, but one did not')
            rr_index = lo

            self.rr_children.insert(rr_index, child)

        # We now have to check if we need to add another logical block.
        # We have to iterate over the entire list again, because where we
        # placed this last entry may rearrange the empty spaces in the blocks
        # that we've already allocated.
        num_extents, offset_unused = self._recalculate_extents_and_offsets(index,
                                                                           logical_block_size)

        overflowed = False
        if check_overflow and (num_extents * logical_block_size > self.data_length):
            overflowed = True
            # When we overflow our data length, we always add a full block.
            self.data_length += logical_block_size
            # We also have to make sure to update the length of the dot child,
            # as that should always reflect the length.
            self.children[0].data_length = self.data_length
            # We also have to update all of the dotdot entries.  If this is
            # the root directory record (no parent), we first update the root
            # dotdot entry.  In all cases, we update the dotdot entry of all
            # children that are directories.
            if self.parent is None:
                self.children[1].data_length = self.data_length

            for c in self.children:
                if not c.is_dir():
                    continue
                if len(c.children) > 1:
                    c.children[1].data_length = self.data_length

        return overflowed

    def add_child(self, child, logical_block_size, allow_duplicate=False):
        # type: (DirectoryRecord, int, bool) -> bool
        """
        Add a new child to this directory record.

        Parameters:
         child - The child directory record object to add.
         logical_block_size - The size of a logical block for this volume
                              descriptor.
         allow_duplicate - Whether to allow duplicate names, as there are
                           situations where duplicate children are allowed.
        Returns:
         True if adding this child caused the directory to overflow into another
         extent, False otherwise.
        """
        if not self.initialized:
            raise pycdlibexception.PyCdlibInternalError('Directory Record not initialized')

        return self._add_child(child, logical_block_size, allow_duplicate, True)

    def track_child(self, child, logical_block_size, allow_duplicate=False):
        # type: (DirectoryRecord, int, bool) -> None
        """
        Track an existing child of this directory record.

        Parameters:
         child - The child directory record object to add.
         logical_block_size - The size of a logical block for this volume
                              descriptor.
         allow_duplicate - Whether to allow duplicate names, as there are
                           situations where duplicate children are allowed.
        Returns:
         Nothing.
        """
        if not self.initialized:
            raise pycdlibexception.PyCdlibInternalError('Directory Record not initialized')

        self._add_child(child, logical_block_size, allow_duplicate, False)

    def remove_child(self, child, index, logical_block_size):
        # type: (DirectoryRecord, int, int) -> bool
        """
        Remove a child from this Directory Record.

        Parameters:
         child - The child DirectoryRecord object to remove.
         index - The index of the child into this DirectoryRecord children list.
         logical_block_size - The size of a logical block on this volume
                              descriptor.
        Returns:
         True if removing this child caused an underflow, False otherwise.
        """
        if not self.initialized:
            raise pycdlibexception.PyCdlibInternalError('Directory Record not initialized')

        if index < 0:
            # This should never happen
            raise pycdlibexception.PyCdlibInternalError('Invalid child index to remove')

        # Unfortunately, Rock Ridge specifies that a CL 'directory' is replaced
        # by a *file*, not another directory.  Thus, we can't just depend on
        # whether this child is marked as a directory by the file flags during
        # parse time.  Instead, we check if this is either a true directory,
        # or a Rock Ridge CL entry, and in either case try to manipulate the
        # file links.
        if child.rock_ridge is not None:
            if child.isdir or child.rock_ridge.child_link_record_exists():
                if len(self.children) < 2:
                    raise pycdlibexception.PyCdlibInvalidISO('Expected a dot and dotdot entry, but missing; ISO is corrupt')
                if self.children[0].rock_ridge is None or self.children[1].rock_ridge is None:
                    raise pycdlibexception.PyCdlibInvalidISO('Missing Rock Ridge entry on dot or dotdot; ISO is corrupt')

                if self.parent is None:
                    self.children[0].rock_ridge.remove_from_file_links()
                    self.children[1].rock_ridge.remove_from_file_links()
                else:
                    if self.rock_ridge is None:
                        raise pycdlibexception.PyCdlibInvalidISO('Child has Rock Ridge, but parent does not; ISO is corrupt')
                    self.rock_ridge.remove_from_file_links()

                    self.children[0].rock_ridge.remove_from_file_links()

        del self.children[index]

        # We now have to check if we need to remove a logical block.
        # We have to iterate over the entire list again, because where we
        # removed this last entry may rearrange the empty spaces in the blocks
        # that we've already allocated.
        num_extents, dirrecord_offset = self._recalculate_extents_and_offsets(index,
                                                                              logical_block_size)

        underflow = False
        total_size = (num_extents - 1) * logical_block_size + dirrecord_offset
        if (self.data_length - total_size) > logical_block_size:
            self.data_length -= logical_block_size
            # We also have to make sure to update the length of the dot child,
            # as that should always reflect the length.
            self.children[0].data_length = self.data_length
            # We also have to update all of the dotdot entries.  If this is
            # the root directory record (no parent), we first update the root
            # dotdot entry.  In all cases, we update the dotdot entry of all
            # children that are directories.
            if self.parent is None:
                self.children[1].data_length = self.data_length

            for c in self.children:
                if not c.is_dir():
                    continue
                if len(c.children) > 1:
                    c.children[1].data_length = self.data_length
            underflow = True

        return underflow

    def is_dir(self):
        # type: () -> bool
        """
        Determine whether this Directory Record is a directory.

        Parameters:
         None.
        Returns:
         True if this DirectoryRecord object is a directory, False otherwise.
        """
        if not self.initialized:
            raise pycdlibexception.PyCdlibInternalError('Directory Record not initialized')
        return self.isdir

    def is_file(self):
        # type: () -> bool
        """
        Determine whether this Directory Record is a file.

        Parameters:
         None.
        Returns:
         True if this DirectoryRecord object is a file, False otherwise.
        """
        if not self.initialized:
            raise pycdlibexception.PyCdlibInternalError('Directory Record not initialized')
        return not self.isdir

    def is_symlink(self):
        # type: () -> bool
        """
        Determine whether this Directory Record is a Rock Ridge
        symlink.  If using this to distinguish between symlinks, files, and
        directories, it is important to call this API *first*; symlinks are
        also considered files.

        Parameters:
         None.
        Returns:
         True if this Directory Record object is a symlink, False otherwise.
        """
        if not self.initialized:
            raise pycdlibexception.PyCdlibInternalError('Directory Record not initialized')
        return self.rock_ridge is not None and self.rock_ridge.is_symlink()

    def is_dot(self):
        # type: () -> bool
        """
        Determine whether this Directory Record is a 'dot' entry.

        Parameters:
         None.
        Returns:
         True if this DirectoryRecord object is a 'dot' entry, False otherwise.
        """
        if not self.initialized:
            raise pycdlibexception.PyCdlibInternalError('Directory Record not initialized')
        return self.file_ident == b'\x00'

    def is_dotdot(self):
        # type: () -> bool
        """
        Determine whether this Directory Record is a 'dotdot' entry.

        Parameters:
         None.
        Returns:
         True if this DirectoryRecord object is a 'dotdot' entry, False otherwise.
        """
        if not self.initialized:
            raise pycdlibexception.PyCdlibInternalError('Directory Record not initialized')
        return self.file_ident == b'\x01'

    def directory_record_length(self):
        # type: () -> int
        """
        Determine the length of this Directory Record.

        Parameters:
         None.
        Returns:
         The length of this Directory Record.
        """
        if not self.initialized:
            raise pycdlibexception.PyCdlibInternalError('Directory Record not initialized')
        return self.dr_len

    def _extent_location(self):
        """
        An internal method to get the location of this Directory Record on the
        ISO.

        Parameters:
         None.
        Returns:
         Extent location of this Directory Record on the ISO.
        """
        if self.new_extent_loc < 0:
            return self.orig_extent_loc
        return self.new_extent_loc

    def extent_location(self):
        # type: () -> int
        """
        Get the location of this Directory Record on the ISO.

        Parameters:
         None.
        Returns:
         Extent location of this Directory Record on the ISO.
        """
        if not self.initialized:
            raise pycdlibexception.PyCdlibInternalError('Directory Record not initialized')
        return self._extent_location()

    def file_identifier(self):
        # type: () -> bytes
        """
        Get the identifier of this Directory Record.

        Parameters:
         None.
        Returns:
         String representing the identifier of this Directory Record.
        """
        if not self.initialized:
            raise pycdlibexception.PyCdlibInternalError('Directory Record not initialized')
        return self._printable_name

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this Directory Record.

        Parameters:
         None.
        Returns:
         String representing this Directory Record.
        """
        if not self.initialized:
            raise pycdlibexception.PyCdlibInternalError('Directory Record not initialized')

        padlen = struct.calcsize(self.FMT) + self.len_fi
        padstr = b'\x00' * (padlen % 2)

        extent_loc = self._extent_location()

        xa_rec = b''
        if self.xa_record is not None:
            xa_rec = self.xa_record.record()
        rr_rec = b''
        if self.rock_ridge is not None:
            rr_rec = self.rock_ridge.record_dr_entries()

        outlist = [struct.pack(self.FMT, self.dr_len, self.xattr_len,
                               extent_loc, utils.swab_32bit(extent_loc),
                               self.data_length, utils.swab_32bit(self.data_length),
                               self.date.record(), self.file_flags,
                               self.file_unit_size, self.interleave_gap_size,
                               self.seqnum, utils.swab_16bit(self.seqnum),
                               self.len_fi) + self.file_ident + padstr + xa_rec + rr_rec]

        outlist.append(b'\x00' * (len(outlist[0]) % 2))

        return b''.join(outlist)

    def is_associated_file(self):
        # type: () -> bool
        """
        Determine whether this file is 'associated' with another file
        on the ISO.

        Parameters:
         None.
        Returns:
         True if this file is associated with another file on the ISO, False
         otherwise.
        """
        if not self.initialized:
            raise pycdlibexception.PyCdlibInternalError('Directory Record not initialized')

        return self.file_flags & (1 << self.FILE_FLAG_ASSOCIATED_FILE_BIT)

    def set_ptr(self, ptr):
        # type: (path_table_record.PathTableRecord) -> None
        """
        Set the Path Table Record associated with this Directory Record.

        Parameters:
         ptr - The path table record to associate with this Directory Record.
        Returns:
         Nothing.
        """
        if not self.initialized:
            raise pycdlibexception.PyCdlibInternalError('Directory Record not initialized')

        self.ptr = ptr

    def set_data_location(self, current_extent, tag_location):  # pylint: disable=unused-argument
        # type: (int, int) -> None
        """
        Set the new extent location that the data for this Directory Record
        should live at.

        Parameters:
         current_extent - The new extent.
        Returns:
         Nothing.
        """
        if not self.initialized:
            raise pycdlibexception.PyCdlibInternalError('Directory Record not initialized')

        self.new_extent_loc = current_extent
        if self.ptr is not None:
            self.ptr.update_extent_location(current_extent)

    def get_data_length(self):
        # type: () -> int
        """
        Get the length of the data that this Directory Record points to.

        Parameters:
         None.
        Returns:
         The length of the data that this Directory Record points to.
        """
        if not self.initialized:
            raise pycdlibexception.PyCdlibInternalError('Directory Record not initialized')
        if self.inode is not None:
            return self.inode.get_data_length()
        return self.data_length

    def set_data_length(self, length):
        # type: (int) -> None
        """
        Set the length of the data that this Directory Record points to.

        Parameters:
         length - The new length for the data.
        Returns:
         The length of the data that this Directory Record points to.
        """
        if not self.initialized:
            raise pycdlibexception.PyCdlibInternalError('Directory Record not initialized')
        self.data_length = length

    ############# START BACKWARDS COMPATIBILITY ###############################
    # We have a few downstream users that are using 'data_fp',
    # 'original_data_location', 'DATA_ON_ORIGINAL_ISO', 'DATA_IN_EXTERNAL_FP',
    # and 'fp_offset' directly.  For backwards compatibility
    # we define properties here that access these.  Note that this won't work
    # in all circumstances, but is good enough for a read-only client.

    @property
    def data_fp(self):
        # type: () -> Optional[Union[IO[Any], str]]
        """Backwards compatibility property for 'data_fp'."""
        if self.inode is None:
            return None
        return self.inode.data_fp

    @property
    def original_data_location(self):
        # type: () -> Optional[int]
        """Backwards compatibility property for 'original_data_location'."""
        if self.inode is None:
            return None
        return self.inode.original_data_location

    @property
    def DATA_ON_ORIGINAL_ISO(self):
        # type: () -> int
        """Backwards compatibility property for 'DATA_ON_ORIGINAL_ISO'."""
        return inode.Inode.DATA_ON_ORIGINAL_ISO

    @property
    def DATA_IN_EXTERNAL_FP(self):
        # type: () -> int
        """Backwards compatibility property for 'DATA_IN_EXTERNAL_FP'."""
        return inode.Inode.DATA_IN_EXTERNAL_FP

    @property
    def fp_offset(self):
        # type: () -> Optional[int]
        """Backwards compatibility property for 'fp_offset'."""
        if self.inode is None:
            return None
        return self.inode.fp_offset

    ############# END BACKWARDS COMPATIBILITY #################################

    def __lt__(self, other):
        # This method is used for the bisect.insort_left() when adding a child.
        # It needs to return whether self is less than other.  Here we use the
        # ISO9660 sorting order which is essentially:
        #
        # 1.  The \x00 is always the 'dot' record, and is always first.
        # 2.  The \x01 is always the 'dotdot' record, and is always second.
        # 3.  Other entries are sorted lexically; this does not exactly match
        #     the sorting method specified in Ecma-119, but does OK for now.
        #
        # Ecma-119 Section 9.3 specifies that we need to pad out the shorter of
        # the two files with 0x20 (spaces), then compare byte-by-byte until
        # they differ.  However, we can more easily just do the string equality
        # comparison, since it will always be the case that 0x20 will be less
        # than any of the other allowed characters in the strings.
        if self.file_ident == b'\x00':
            if other.file_ident == b'\x00':
                return False
            return True
        if other.file_ident == b'\x00':
            return False

        if self.file_ident == b'\x01':
            if other.file_ident == b'\x00':
                return False
            return True

        if other.file_ident == b'\x01':
            # If self.file_ident was '\x00', it would have been caught above.
            return False
        return self.file_ident < other.file_ident

    def __ne__(self, other):
        # type: (object) -> bool
        if not isinstance(other, DirectoryRecord):
            return NotImplemented
        # Note that we very specifically do not check the extent_location when
        # comparing directory records.  In a lazy-extent assigning world, the
        # extents are not reliable, so we just rely on the rest of the fields to
        # tell us if two directory records are the same.
        return self.dr_len != other.dr_len or self.xattr_len != other.xattr_len or \
            self.data_length != other.data_length or self.date != other.date or \
            self.file_flags != other.file_flags or \
            self.file_unit_size != other.file_unit_size or \
            self.interleave_gap_size != other.interleave_gap_size or \
            self.seqnum != other.seqnum or self.len_fi != other.len_fi or \
            self.file_ident != other.file_ident

    def __eq__(self, other):
        # type: (object) -> bool
        return not self.__ne__(other)
