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

"""Classes and utilities to support Rock Ridge extensions."""

import bisect
import struct

from pycdlib import dates
from pycdlib import pycdlibexception
from pycdlib import utils

# For mypy annotations
if False:  # pylint: disable=using-constant-test
    from typing import Dict, List, Optional  # NOQA pylint: disable=unused-import
    # NOTE: this has to be here to avoid circular deps
    from pycdlib import dr  # NOQA pylint: disable=unused-import,cyclic-import

SU_ENTRY_VERSION = 1
ALLOWED_DR_SIZE = 254
TF_FLAGS = 0x0e
EXT_ID_109 = b'RRIP_1991A'
EXT_DES_109 = b'THE ROCK RIDGE INTERCHANGE PROTOCOL PROVIDES SUPPORT FOR POSIX FILE SYSTEM SEMANTICS'
EXT_SRC_109 = b'PLEASE CONTACT DISC PUBLISHER FOR SPECIFICATION SOURCE.  SEE PUBLISHER IDENTIFIER IN PRIMARY VOLUME DESCRIPTOR FOR CONTACT INFORMATION.'
EXT_ID_112 = b'IEEE_P1282'
EXT_DES_112 = b'THE IEEE P1282 PROTOCOL PROVIDES SUPPORT FOR POSIX FILE SYSTEM SEMANTICS'
EXT_SRC_112 = b'PLEASE CONTACT THE IEEE STANDARDS DEPARTMENT, PISCATAWAY, NJ, USA FOR THE P1282 SPECIFICATION'


class RRSPRecord:
    """
    A class that represents a Rock Ridge Sharing Protocol record.  This record
    indicates that the sharing protocol is in use, and how many bytes to skip
    prior to parsing a Rock Ridge entry out of a directory record.
    """
    __slots__ = ('_initialized', 'bytes_to_skip')

    FMT = '=BBBBB'

    def __init__(self):
        # type: () -> None
        self._initialized = False

    def parse(self, rrstr):
        # type: (bytes) -> None
        """
        Parse a Rock Ridge Sharing Protocol record out of a string.

        Parameters:
         rrstr - The string to parse the record out of.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('SP record already initialized')

        (su_len, su_entry_version_unused, check_byte1, check_byte2,
         self.bytes_to_skip) = struct.unpack_from(self.FMT, rrstr[:7], 2)

        # We assume that the caller has already checked the su_entry_version,
        # so we don't bother.

        if su_len != RRSPRecord.length():
            raise pycdlibexception.PyCdlibInvalidISO('Invalid length on rock ridge extension')
        if check_byte1 != 0xbe or check_byte2 != 0xef:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid check bytes on rock ridge extension')

        self._initialized = True

    def new(self, bytes_to_skip):
        # type: (int) -> None
        """
        Create a new Rock Ridge Sharing Protocol record.

        Parameters:
        bytes_to_skip - The number of bytes to skip.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('SP record already initialized')

        self.bytes_to_skip = bytes_to_skip
        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate a string representing the Rock Ridge Sharing Protocol record.

        Parameters:
         None.
        Returns:
         String containing the Rock Ridge record.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('SP record not initialized')

        return b'SP' + struct.pack(self.FMT, RRSPRecord.length(),
                                   SU_ENTRY_VERSION, 0xbe, 0xef,
                                   self.bytes_to_skip)

    @staticmethod
    def length():
        # type: () -> int
        """
        Static method to return the length of the Rock Ridge Sharing Protocol
        record.

        Parameters:
         None.
        Returns:
         The length of this record in bytes.
        """
        return 7


class RRRRRecord:
    """
    A class that represents a Rock Ridge Rock Ridge record.  This optional
    record indicates which other Rock Ridge fields are present.
    """
    __slots__ = ('_initialized', 'rr_flags')

    FMT = '=BBB'

    def __init__(self):
        # type: () -> None
        self.rr_flags = 0
        self._initialized = False

    def parse(self, rrstr):
        # type: (bytes) -> None
        """
        Parse a Rock Ridge Rock Ridge record out of a string.

        Parameters:
         rrstr - The string to parse the record out of.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('RR record already initialized')

        (su_len, su_entry_version_unused,
         self.rr_flags) = struct.unpack_from(self.FMT, rrstr[:5], 2)

        # We assume that the caller has already checked the su_entry_version,
        # so we don't bother.

        if su_len != RRRRRecord.length():
            raise pycdlibexception.PyCdlibInvalidISO('Invalid length on rock ridge extension')

        self._initialized = True

    def new(self):
        # type: () -> None
        """
        Create a new Rock Ridge Rock Ridge record.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('RR record already initialized')

        self.rr_flags = 0
        self._initialized = True

    def append_field(self, fieldname):
        # type: (str) -> None
        """
        Mark a field as present in the Rock Ridge records.

        Parameters:
         fieldname - The name of the field to mark as present; should be one
                     of 'PX', 'PN', 'SL', 'NM', 'CL', 'PL', 'RE', or 'TF'.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('RR record not initialized')

        field_to_bit = {
            'PX': 0,
            'PN': 1,
            'SL': 2,
            'NM': 3,
            'CL': 4,
            'PL': 5,
            'RE': 6,
            'TF': 7
        }

        try:
            self.rr_flags |= (1 << field_to_bit[fieldname])
        except KeyError:
            raise pycdlibexception.PyCdlibInternalError('Unknown RR field name %s' % (fieldname))  # pylint: disable=raise-missing-from

    def record(self):
        # type: () -> bytes
        """
        Generate a string representing the Rock Ridge Rock Ridge record.

        Parameters:
         None.
        Returns:
         String containing the Rock Ridge record.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('RR record not initialized')

        return b'RR' + struct.pack(self.FMT, RRRRRecord.length(),
                                   SU_ENTRY_VERSION, self.rr_flags)

    @staticmethod
    def length():
        # type: () -> int
        """
        Static method to return the length of the Rock Ridge Rock Ridge
        record.

        Parameters:
         None.
        Returns:
         The length of this record in bytes.
        """
        return 5


class RRCERecord:
    """
    A class that represents a Rock Ridge Continuation Entry record.  This
    record represents additional information that did not fit in the standard
    directory record.
    """
    __slots__ = ('_initialized', 'bl_cont_area', 'offset_cont_area',
                 'len_cont_area')

    FMT = '<BBLLLLLL'

    def __init__(self):
        # type: () -> None
        self._initialized = False

    def parse(self, rrstr):
        # type: (bytes) -> None
        """
        Parse a Rock Ridge Continuation Entry record out of a string.

        Parameters:
         rrstr - The string to parse the record out of.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('CE record already initialized')

        (su_len, su_entry_version_unused, bl_cont_area_le, bl_cont_area_be,
         offset_cont_area_le, offset_cont_area_be,
         len_cont_area_le, len_cont_area_be) = struct.unpack_from(self.FMT, rrstr[:28], 2)

        # We assume that the caller has already checked the su_entry_version,
        # so we don't bother.

        if su_len != RRCERecord.length():
            raise pycdlibexception.PyCdlibInvalidISO('Invalid length on rock ridge extension')

        if bl_cont_area_le != utils.swab_32bit(bl_cont_area_be):
            raise pycdlibexception.PyCdlibInvalidISO('CE record big and little endian continuation area do not agree')

        if offset_cont_area_le != utils.swab_32bit(offset_cont_area_be):
            raise pycdlibexception.PyCdlibInvalidISO('CE record big and little endian continuation area offset do not agree')

        if len_cont_area_le != utils.swab_32bit(len_cont_area_be):
            raise pycdlibexception.PyCdlibInvalidISO('CE record big and little endian continuation area length do not agree')

        self.bl_cont_area = bl_cont_area_le
        self.offset_cont_area = offset_cont_area_le
        self.len_cont_area = len_cont_area_le

        self._initialized = True

    def new(self):
        # type: () -> None
        """
        Create a new Rock Ridge Continuation Entry record.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('CE record already initialized')

        self.bl_cont_area = 0  # This will get set during reshuffle_extents
        self.offset_cont_area = 0  # This will get set during reshuffle_extents
        self.len_cont_area = 0  # This will be calculated based on fields put in

        self._initialized = True

    def update_extent(self, extent):
        # type: (int) -> None
        """
        Update the extent for this CE record.

        Parameters:
         extent - The new extent for this CE record.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('CE record not initialized')

        self.bl_cont_area = extent

    def update_offset(self, offset):
        # type: (int) -> None
        """
        Update the offset for this CE record.

        Parameters:
         extent - The new offset for this CE record.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('CE record not initialized')

        self.offset_cont_area = offset

    def add_record(self, length):
        # type: (int) -> None
        """
        Add some more length to this CE record.  Used when a new record is going
        to get recorded into the CE (rather than the DR).

        Parameters:
         length - The length to add to this CE record.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('CE record not initialized')

        self.len_cont_area += length

    def record(self):
        # type: () -> bytes
        """
        Generate a string representing the Rock Ridge Continuation Entry record.

        Parameters:
         None.
        Returns:
         String containing the Rock Ridge record.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('CE record not initialized')

        return b'CE' + struct.pack(self.FMT,
                                   RRCERecord.length(),
                                   SU_ENTRY_VERSION,
                                   self.bl_cont_area,
                                   utils.swab_32bit(self.bl_cont_area),
                                   self.offset_cont_area,
                                   utils.swab_32bit(self.offset_cont_area),
                                   self.len_cont_area,
                                   utils.swab_32bit(self.len_cont_area))

    @staticmethod
    def length():
        # type: () -> int
        """
        Static method to return the length of the Rock Ridge Continuation Entry
        record.

        Parameters:
         None.
        Returns:
         The length of this record in bytes.
        """
        return 28


class RRPXRecord:
    """
    A class that represents a Rock Ridge POSIX File Attributes record.  This
    record contains information about the POSIX file mode, file links,
    user ID, group ID, and serial number of a directory record.
    """
    __slots__ = ('_initialized', 'posix_file_mode', 'posix_file_links',
                 'posix_user_id', 'posix_group_id', 'posix_serial_number')

    FMT = '<BBLLLLLLLL'

    def __init__(self):
        # type: () -> None
        self.posix_file_mode = 0
        self.posix_file_links = 1
        self.posix_user_id = 0
        self.posix_group_id = 0
        self.posix_serial_number = 0

        self._initialized = False

    def parse(self, rrstr):
        # type: (bytes) -> int
        """
        Parse a Rock Ridge POSIX File Attributes record out of a string.

        Parameters:
         rrstr - The string to parse the record out of.
        Returns:
         The length of the record in bytes.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('PX record already initialized')

        (su_len, su_entry_version_unused, posix_file_mode_le, posix_file_mode_be,
         posix_file_links_le, posix_file_links_be, posix_file_user_id_le,
         posix_file_user_id_be, posix_file_group_id_le,
         posix_file_group_id_be) = struct.unpack_from(self.FMT, rrstr[:38], 2)

        # We assume that the caller has already checked the su_entry_version,
        # so we don't bother.

        if posix_file_mode_le != utils.swab_32bit(posix_file_mode_be):
            raise pycdlibexception.PyCdlibInvalidISO('PX record big and little-endian file mode do not agree')

        if posix_file_links_le != utils.swab_32bit(posix_file_links_be):
            raise pycdlibexception.PyCdlibInvalidISO('PX record big and little-endian file links do not agree')

        if posix_file_user_id_le != utils.swab_32bit(posix_file_user_id_be):
            raise pycdlibexception.PyCdlibInvalidISO('PX record big and little-endian file user ID do not agree')

        if posix_file_group_id_le != utils.swab_32bit(posix_file_group_id_be):
            raise pycdlibexception.PyCdlibInvalidISO('PX record big and little-endian file group ID do not agree')

        # In Rock Ridge 1.09 and 1.10, there is no serial number so the su_len
        # is 36, while in Rock Ridge 1.12, there is an 8-byte serial number so
        # su_len is 44.
        if su_len == 36:
            posix_file_serial_number_le = 0
        elif su_len == 44:
            (posix_file_serial_number_le,
             posix_file_serial_number_be) = struct.unpack_from('<LL',
                                                               rrstr[:44], 36)
            if posix_file_serial_number_le != utils.swab_32bit(posix_file_serial_number_be):
                raise pycdlibexception.PyCdlibInvalidISO('PX record big and little-endian file serial number do not agree')
        else:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid length on Rock Ridge PX record')

        self.posix_file_mode = posix_file_mode_le
        self.posix_file_links = posix_file_links_le
        self.posix_user_id = posix_file_user_id_le
        self.posix_group_id = posix_file_group_id_le
        self.posix_serial_number = posix_file_serial_number_le

        self._initialized = True

        return su_len

    def new(self, mode):
        # type: (int) -> None
        """
        Create a new Rock Ridge POSIX File Attributes record.

        Parameters:
         mode - The Unix file mode for this record.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('PX record already initialized')

        self.posix_file_mode = mode
        self.posix_file_links = 1
        self.posix_user_id = 0
        self.posix_group_id = 0
        self.posix_serial_number = 0

        self._initialized = True

    def record(self, rr_version):
        # type: (str) -> bytes
        """
        Generate a string representing the Rock Ridge POSIX File Attributes
        record.

        Parameters:
         rr_version - The Rock Ridge version to use.
        Returns:
         String containing the Rock Ridge record.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('PX record not initialized')

        outlist = [b'PX', struct.pack(self.FMT, RRPXRecord.length(rr_version),
                                      SU_ENTRY_VERSION, self.posix_file_mode,
                                      utils.swab_32bit(self.posix_file_mode),
                                      self.posix_file_links,
                                      utils.swab_32bit(self.posix_file_links),
                                      self.posix_user_id,
                                      utils.swab_32bit(self.posix_user_id),
                                      self.posix_group_id,
                                      utils.swab_32bit(self.posix_group_id))]
        if rr_version == '1.12':
            outlist.append(struct.pack('<LL', self.posix_serial_number,
                                       utils.swab_32bit(self.posix_serial_number)))
        # The rr_version can never be "wrong" at this point; if it was, it would
        # have thrown an exception earlier when calling length().  So just skip
        # any potential checks here.

        return b''.join(outlist)

    @staticmethod
    def length(rr_version):
        # type: (str) -> int
        """
        Static method to return the length of the Rock Ridge POSIX File
        Attributes record.

        Parameters:
         rr_version - The version of Rock Ridge in use; must be '1.09', '1.10',
                      or '1.12'.
        Returns:
         The length of this record in bytes.
        """
        if rr_version in ('1.09', '1.10'):
            return 36
        if rr_version == '1.12':
            return 44

        # This should never happen
        raise pycdlibexception.PyCdlibInternalError('Invalid rr_version')


class RRERRecord:
    """A class that represents a Rock Ridge Extensions Reference record."""
    __slots__ = ('_initialized', 'ext_id', 'ext_des', 'ext_src', 'ext_ver')

    FMT = '=BBBBBB'

    def __init__(self):
        # type: () -> None
        self.ext_id = b''
        self.ext_des = b''
        self.ext_src = b''
        self._initialized = False

    def parse(self, rrstr):
        # type: (bytes) -> None
        """
        Parse a Rock Ridge Extensions Reference record out of a string.

        Parameters:
         rrstr - The string to parse the record out of.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('ER record already initialized')

        (su_len, su_entry_version_unused, len_id, len_des, len_src,
         self.ext_ver) = struct.unpack_from(self.FMT, rrstr[:8], 2)

        # We assume that the caller has already checked the su_entry_version,
        # so we don't bother.

        # Ensure that the length isn't crazy
        if su_len > len(rrstr):
            raise pycdlibexception.PyCdlibInvalidISO('Length of ER record much too long')

        # Also ensure that the combination of len_id, len_des, and len_src
        # doesn't overrun su_len; because of the check above, this means it
        # can't overrun len(rrstr) either
        total_length = len_id + len_des + len_src
        if total_length > su_len:
            raise pycdlibexception.PyCdlibInvalidISO('Combined length of ER ID, des, and src longer than record')

        fmtstr = '=%ds%ds%ds' % (len_id, len_des, len_src)
        (self.ext_id, self.ext_des,
         self.ext_src) = struct.unpack_from(fmtstr, rrstr, 8)

        self._initialized = True

    def new(self, ext_id, ext_des, ext_src):
        # type: (bytes, bytes, bytes) -> None
        """
        Create a new Rock Ridge Extensions Reference record.

        Parameters:
         ext_id - The extension identifier to use.
         ext_des - The extension descriptor to use.
         ext_src - The extension specification source to use.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('ER record already initialized')

        self.ext_id = ext_id
        self.ext_des = ext_des
        self.ext_src = ext_src
        self.ext_ver = 1

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate a string representing the Rock Ridge Extensions Reference
        record.

        Parameters:
         None.
        Returns:
         String containing the Rock Ridge record.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('ER record not initialized')

        return b'ER' + struct.pack(self.FMT,
                                   RRERRecord.length(self.ext_id, self.ext_des, self.ext_src),
                                   SU_ENTRY_VERSION,
                                   len(self.ext_id),
                                   len(self.ext_des),
                                   len(self.ext_src),
                                   self.ext_ver) + self.ext_id + self.ext_des + self.ext_src

    @staticmethod
    def length(ext_id, ext_des, ext_src):
        # type: (bytes, bytes, bytes) -> int
        """
        Static method to return the length of the Rock Ridge Extensions Reference
        record.

        Parameters:
         ext_id - The extension identifier to use.
         ext_des - The extension descriptor to use.
         ext_src - The extension specification source to use.
        Returns:
         The length of this record in bytes.
        """
        return 8 + len(ext_id) + len(ext_des) + len(ext_src)


class RRESRecord:
    """A class that represents a Rock Ridge Extension Selector record."""
    __slots__ = ('_initialized', 'extension_sequence')

    FMT = '=BBB'

    def __init__(self):
        # type: () -> None
        self.extension_sequence = 0
        self._initialized = False

    def parse(self, rrstr):
        # type: (bytes) -> None
        """
        Parse a Rock Ridge Extension Selector record out of a string.

        Parameters:
         rrstr - The string to parse the record out of.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('ES record already initialized')

        # We assume that the caller has already checked the su_entry_version,
        # so we don't bother.

        (su_len, su_entry_version_unused,
         self.extension_sequence) = struct.unpack_from(self.FMT, rrstr[:5], 2)
        if su_len != RRESRecord.length():
            raise pycdlibexception.PyCdlibInvalidISO('Invalid length on rock ridge extension')

        self._initialized = True

    def new(self, extension_sequence):
        # type: (int) -> None
        """
        Create a new Rock Ridge Extension Selector record.

        Parameters:
         extension_sequence - The sequence number of this extension.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('ES record already initialized')

        self.extension_sequence = extension_sequence
        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate a string representing the Rock Ridge Extension Selector record.

        Parameters:
         None.
        Returns:
         String containing the Rock Ridge record.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('ES record not initialized')

        return b'ES' + struct.pack(self.FMT,
                                   RRESRecord.length(),
                                   SU_ENTRY_VERSION,
                                   self.extension_sequence)

    @staticmethod
    def length():
        # type: () -> int
        """
        Static method to return the length of the Rock Ridge Extensions Selector
        record.

        Parameters:
         None.
        Returns:
         The length of this record in bytes.
        """
        return 5


class RRPNRecord:
    """
    A class that represents a Rock Ridge POSIX Device Number record.  This
    record represents a device major and minor special file.
    """
    __slots__ = ('_initialized', 'dev_t_high', 'dev_t_low')

    FMT = '<BBLLLL'

    def __init__(self):
        # type: () -> None
        self.dev_t_high = 0
        self.dev_t_low = 0
        self._initialized = False

    def parse(self, rrstr):
        # type: (bytes) -> None
        """
        Parse a Rock Ridge POSIX Device Number record out of a string.

        Parameters:
         rrstr - The string to parse the record out of.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('PN record already initialized')

        (su_len, su_entry_version_unused, dev_t_high_le, dev_t_high_be,
         dev_t_low_le, dev_t_low_be) = struct.unpack_from(self.FMT, rrstr[:20], 2)

        # We assume that the caller has already checked the su_entry_version,
        # so we don't bother.

        if su_len != RRPNRecord.length():
            raise pycdlibexception.PyCdlibInvalidISO('Invalid length on rock ridge extension')

        if dev_t_high_le != utils.swab_32bit(dev_t_high_be):
            raise pycdlibexception.PyCdlibInvalidISO('Dev_t high little-endian does not match big-endian')

        if dev_t_low_le != utils.swab_32bit(dev_t_low_be):
            raise pycdlibexception.PyCdlibInvalidISO('Dev_t low little-endian does not match big-endian')

        self.dev_t_high = dev_t_high_le
        self.dev_t_low = dev_t_low_le

        self._initialized = True

    def new(self, dev_t_high, dev_t_low):
        # type: (int, int) -> None
        """
        Create a new Rock Ridge POSIX device number record.

        Parameters:
         dev_t_high - The high-order 32-bits of the device number.
         dev_t_low - The low-order 32-bits of the device number.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('PN record already initialized')

        self.dev_t_high = dev_t_high
        self.dev_t_low = dev_t_low

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate a string representing the Rock Ridge POSIX Device Number
        record.

        Parameters:
         None.
        Returns:
         String containing the Rock Ridge record.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('PN record not initialized')

        return b'PN' + struct.pack(self.FMT,
                                   RRPNRecord.length(),
                                   SU_ENTRY_VERSION,
                                   self.dev_t_high,
                                   utils.swab_32bit(self.dev_t_high),
                                   self.dev_t_low,
                                   utils.swab_32bit(self.dev_t_low))

    @staticmethod
    def length():
        # type: () -> int
        """
        Static method to return the length of the Rock Ridge POSIX Device Number
        record.

        Parameters:
         None.
        Returns:
         The length of this record in bytes.
        """
        return 20


class RRSLRecord:
    """
    A class that represents a Rock Ridge Symbolic Link record.  This record
    represents some or all of a symbolic link.  For a symbolic link, Rock Ridge
    specifies that each component (part of path separated by /) be in a separate
    component entry, and individual components may be split across multiple
    Symbolic Link records.  This class takes care of all of those details.
    """
    __slots__ = ('_initialized', 'symlink_components', 'flags')

    class Component:
        """A class that represents one component of a Symbolic Link Record."""
        __slots__ = ('flags', 'curr_length', 'data')

        def __init__(self, flags, length, data):
            # type: (int, int, bytes) -> None
            if flags not in (0, 1, 2, 4, 8):
                raise pycdlibexception.PyCdlibInternalError('Invalid Rock Ridge symlink flags 0x%x' % (flags))

            if (flags & (1 << 1) or flags & (1 << 2) or flags & (1 << 3)) and length != 0:
                raise pycdlibexception.PyCdlibInternalError('Rock Ridge symlinks to dot, dotdot, or root should have zero length')

            # A Component can't both be a continuation and one of dot, dotdot,
            # or root, but this case is caught by the initial flags check so we
            # don't check for it again here.

            self.flags = flags
            self.curr_length = length
            self.data = data

        def name(self):
            # type: () -> bytes
            """
            Retrieve the human-readable name of this component.

            Parameters:
             None.
            Returns:
             Human readable name of this component.
            """
            if self.flags & (1 << 1):
                return b'.'
            if self.flags & (1 << 2):
                return b'..'
            if self.flags & (1 << 3):
                return b'/'

            return self.data

        def is_continued(self):
            # type: () -> bool
            """
            Determine whether this component is continued in the next component.

            Parameters:
             None.
            Returns:
             True if this component is continued in the next component, False otherwise.
            """
            return self.flags & (1 << 0) != 0

        def record(self):
            # type: () -> bytes
            """
            Return the representation of this component that is suitable for
            writing to disk.

            Parameters:
             None.
            Returns:
             Representation of this compnent suitable for writing to disk.
            """
            if self.flags & (1 << 1):
                return struct.pack('=BB', (1 << 1), 0)
            if self.flags & (1 << 2):
                return struct.pack('=BB', (1 << 2), 0)
            if self.flags & (1 << 3):
                return struct.pack('=BB', (1 << 3), 0)

            return struct.pack('=BB', self.flags, self.curr_length) + self.data

        def set_continued(self):
            # type: () -> None
            """
            Set the continued flag on this component.

            Parameters:
             None.
            Returns:
             Nothing.
            """
            self.flags |= (1 << 0)

        def __eq__(self, other):
            # type: (object) -> bool
            if not isinstance(other, RRSLRecord.Component):
                return NotImplemented
            return self.flags == other.flags and self.curr_length == other.curr_length and self.data == other.data

        def __ne__(self, other):
            # type: (object) -> bool
            return not self.__eq__(other)

        @staticmethod
        def length(symlink_component):
            # type: (bytes) -> int
            """
            Static method to compute the length of one symlink component.

            Parameters:
             symlink_component - String representing one symlink component.
            Returns:
             Length of symlink component plus overhead.
            """
            length = 2
            if symlink_component not in (b'.', b'..', b'/'):
                length += len(symlink_component)

            return length

        @staticmethod
        def factory(name):
            # type: (bytes) -> RRSLRecord.Component
            """
            A static method to create a new, valid Component given a human
            readable name.

            Parameters:
             name - The name to create the Component from.
            Returns:
             A new Component object representing this name.
            """
            if name == b'.':
                flags = 1 << 1
                length = 0
            elif name == b'..':
                flags = 1 << 2
                length = 0
            elif name == b'/':
                flags = 1 << 3
                length = 0
            else:
                flags = 0
                length = len(name)

            # Theoretically, this factory method could be passed a name
            # that wouldn't fit into either this SL record or into a single
            # component.  However, the only caller of this factory method
            # (add_component(), below) already checks to make sure this
            # name would fit into the SL record, and the job of making sure
            # everything fits into an SL record really belongs there.
            # Further, we recognize that an SL record and a component
            # record both use an 8-bit quantity for the length, so there is
            # never a time when something would fit into the SL record but
            # would not fit into a component.  Thus, we elide any length
            # checks here.
            return RRSLRecord.Component(flags, length, name)

    def __init__(self):
        # type: () -> None
        self.symlink_components = []  # type: List[RRSLRecord.Component]
        self.flags = 0
        self._initialized = False

    def parse(self, rrstr):
        # type: (bytes) -> None
        """
        Parse a Rock Ridge Symbolic Link record out of a string.

        Parameters:
         rrstr - The string to parse the record out of.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('SL record already initialized')

        (su_len, su_entry_version_unused,
         self.flags) = struct.unpack_from('=BBB', rrstr[:5], 2)

        # We assume that the caller has already checked the su_entry_version,
        # so we don't bother.

        cr_offset = 5
        data_len = su_len - 5
        while data_len > 0:
            (cr_flags, len_cp) = struct.unpack_from('=BB',
                                                    rrstr[:cr_offset + 2],
                                                    cr_offset)

            data_len -= 2
            cr_offset += 2

            self.symlink_components.append(self.Component(cr_flags, len_cp,
                                                          rrstr[cr_offset:cr_offset + len_cp]))

            # FIXME: if this is the last component in this SL record,
            # but the component continues on in the next SL record, we will
            # fail to record this bit.  We should fix that.

            cr_offset += len_cp
            data_len -= len_cp

        self._initialized = True

    def new(self):
        # type: () -> None
        """
        Create a new Rock Ridge Symbolic Link record.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('SL record already initialized')

        self._initialized = True

    def add_component(self, symlink_comp):
        # type: (bytes) -> None
        """
        Add a new component to this symlink record.

        Parameters:
         symlink_comp - The string to add to this symlink record.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('SL record not initialized')

        if (self.current_length() + RRSLRecord.Component.length(symlink_comp)) > 255:
            raise pycdlibexception.PyCdlibInvalidInput('Symlink would be longer than 255')

        self.symlink_components.append(self.Component.factory(symlink_comp))

    def current_length(self):
        # type: () -> int
        """
        Calculate the current length of this symlink record.

        Parameters:
         None.
        Returns:
         Length of this symlink record.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('SL record not initialized')

        strlist = []
        for comp in self.symlink_components:
            strlist.append(comp.name())

        return RRSLRecord.length(strlist)

    def record(self):
        # type: () -> bytes
        """
        Generate a string representing the Rock Ridge Symbolic Link record.

        Parameters:
         None.
        Returns:
         String containing the Rock Ridge record.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('SL record not initialized')

        outlist = [b'SL', struct.pack('=BBB',
                                      self.current_length(),
                                      SU_ENTRY_VERSION,
                                      self.flags)]
        for comp in self.symlink_components:
            outlist.append(comp.record())

        return b''.join(outlist)

    def name(self):
        # type: () -> bytes
        """
        Generate a string that contains all components of the symlink.

        Parameters:
         None
        Returns:
         String containing all components of the symlink.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('SL record not initialized')

        outlist = []  # type: List[bytes]
        continued = False
        for comp in self.symlink_components:
            name = comp.name()
            if name == b'/':
                outlist = []
                continued = False
                name = b''

            if not continued:
                outlist.append(name)
            else:
                outlist[-1] += name

            continued = comp.is_continued()

        return b'/'.join(outlist)

    def set_continued(self):
        # type: () -> None
        """
        Set this SL record as continued in the next System Use Entry.

        Parameters:
         None
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('SL record not initialized')

        self.flags |= (1 << 0)

    def set_last_component_continued(self):
        # type: () -> None
        """
        Set the previous component of this SL record to continued.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('SL record not initialized')

        if not self.symlink_components:
            raise pycdlibexception.PyCdlibInternalError('Trying to set continued on a non-existent component!')

        self.symlink_components[-1].set_continued()

    def last_component_continued(self):
        # type: () -> bool
        """
        Determines whether the previous component of this SL record is a
        continued one or not.

        Parameters:
         None.
        Returns:
         True if the previous component of this SL record is continued, False
         otherwise.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('SL record not initialized')

        if not self.symlink_components:
            raise pycdlibexception.PyCdlibInternalError('Trying to get continued on a non-existent component!')

        return self.symlink_components[-1].is_continued()

    @staticmethod
    def header_length():
        # type: () -> int
        """
        Static method to return the length of a Rock Ridge Symbolic Link
        header.

        Parameters:
         None
        Returns:
         The length of the RRSLRecord header.
        """
        return 5

    @staticmethod
    def maximum_component_area_length():
        # type: () -> int
        """
        Static method to return the absolute maximum length a Rock Ridge
        Symbolic Link component area can be.

        Parameters:
         None
        Returns:
         The maximum length a Symbolic Link component area can be.
        """
        return 255 - RRSLRecord.header_length()

    @staticmethod
    def length(symlink_components):
        # type: (List[bytes]) -> int
        """
        Static method to return the length of the Rock Ridge Symbolic Link
        record.

        Parameters:
         symlink_components - A list containing a string for each of the
                              symbolic link components.
        Returns:
         The length of this record in bytes.
        """
        length = RRSLRecord.header_length()
        for comp in symlink_components:
            length += RRSLRecord.Component.length(comp)
        return length


class RRALRecord:
    """
    A class that represents an Arbitrary Attribute Interchange Protocol record.
    This is an unoffical extension by libisofs: https://dev.lovelyhq.com/libburnia/libisofs/src/commit/d297ce3aed5935e469bb108a36b7d6e31763a075/doc/susp_aaip_2_0.txt
    The goal of this record is to allow arbitrary attributes with arbitrary
    name/value pairs in the SUSP record.  It is split up much like an SL record,
    so a lot of the code is copied from that class.
    """
    __slots__ = ('_initialized', 'flags', 'components')

    class Component:
        """A class that represents one component of an Arbitrary Attribute."""
        __slots__ = ('flags', 'curr_length', 'data')

        def __init__(self, flags, length, data):
            # type: (int, int, bytes) -> None
            if flags not in (0, 1):
                raise pycdlibexception.PyCdlibInternalError('Invalid Arbitrary Attribute flags 0x%x' % (flags))

            self.flags = flags
            self.curr_length = length
            self.data = data

        def record(self):
            # type: () -> bytes
            """
            Return the representation of this component that is suitable for
            writing to disk.

            Parameters:
             None.
            Returns:
             Representation of this compnent suitable for writing to disk.
            """
            return struct.pack('=BB', self.flags, self.curr_length) + self.data

        def set_continued(self):
            # type: () -> None
            """
            Set the continued flag on this component.

            Parameters:
             None.
            Returns:
             Nothing.
            """
            self.flags |= (1 << 0)

        @staticmethod
        def length(attr):
            # type: (bytes) -> int
            """
            Method to compute the length of a component.

            Parameters:
             None.
            Returns:
             Length of this component plus overhead.
            """
            return 2 + len(attr)

        @staticmethod
        def factory(component):
            # type: (bytes) -> RRALRecord.Component
            """
            A static method to create a new, valid Component given an attribute.

            Parameters:
             component - The string to create the Component from.
            Returns:
             A new Component object representing this string.
            """
            # Theoretically, this factory method could be passed a name
            # that wouldn't fit into either this AL record or into a single
            # component.  However, the only caller of this factory method
            # (add_component(), below) already checks to make sure this
            # name would fit into the AL record, and the job of making sure
            # everything fits into an AL record really belongs there.
            # Further, we recognize that an AL record and a component
            # record both use an 8-bit quantity for the length, so there is
            # never a time when something would fit into the AL record but
            # would not fit into a component.  Thus, we elide any length
            # checks here.
            return RRALRecord.Component(0, len(component), component)

    def __init__(self):
        # type: () -> None
        self.flags = 0
        self.components = []  # type: List[RRALRecord.Component]
        self._initialized = False

    def parse(self, rrstr):
        # type: (bytes) -> None
        """
        Parse an Arbitrary Attribute record out of a string.

        Parameters:
         rrstr - The string to parse the record out of.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('AL record already initialized')

        (su_len, su_entry_version_unused,
         self.flags) = struct.unpack_from('=BBB', rrstr[:5], 2)

        # We assume that the caller has already checked the su_entry_version,
        # so we don't bother.

        cr_offset = 5
        data_len = su_len - 5
        while data_len > 0:
            (cr_flags, len_cp) = struct.unpack_from('=BB',
                                                    rrstr[:cr_offset + 2],
                                                    cr_offset)

            data_len -= 2
            cr_offset += 2

            self.components.append(self.Component(cr_flags, len_cp,
                                                  rrstr[cr_offset:cr_offset + len_cp]))

            # FIXME: if this is the last component in this AL record,
            # but the component continues on in the next AL record, we will
            # fail to record this bit.  We should fix that.

            cr_offset += len_cp
            data_len -= len_cp

        self._initialized = True

    def current_length(self):
        # type: () -> int
        """
        Calculate the current length of this Arbitrary Attribute record.

        Parameters:
         None.
        Returns:
         Length of this Arbitrary Attribute record.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('AL record not initialized')

        strlist = []
        for comp in self.components:
            strlist.append(comp.data)

        return RRALRecord.length(strlist)

    def record(self):
        # type: () -> bytes
        """
        Generate a string representing the Arbitrary Attribute record.

        Parameters:
         None.
        Returns:
         String containing the Arbitrary Attribute record.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('AL record not initialized')

        outlist = [b'AL', struct.pack('=BBB',
                                      self.current_length(),
                                      SU_ENTRY_VERSION,
                                      self.flags)]
        for comp in self.components:
            outlist.append(comp.record())

        return b''.join(outlist)

    def new(self):
        # type: () -> None
        """
        Create a new Arbitrary Attribute record.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('AL record already initialized')

        self._initialized = True

    def set_continued(self):
        # type: () -> None
        """
        Set this AL record as continued in the next System Use Entry.

        Parameters:
         None
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('AL record not initialized')

        self.flags |= (1 << 0)

    def set_last_component_continued(self):
        # type: () -> None
        """
        Set the previous component of this AL record to continued.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('AL record not initialized')

        if not self.components:
            raise pycdlibexception.PyCdlibInternalError('Trying to set continued on a non-existent component!')

        self.components[-1].set_continued()

    def add_component(self, comp):
        # type: (bytes) -> None
        """
        Add a new component to this Arbitrary Attribute record.

        Parameters:
         comp - The string to add to this Arbitrary Attribute record.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('AL record not initialized')

        if (self.current_length() + RRALRecord.Component.length(comp)) > 255:
            raise pycdlibexception.PyCdlibInvalidInput('Attribute would be longer than 255')

        self.components.append(self.Component.factory(comp))

    @staticmethod
    def header_length():
        # type: () -> int
        """
        Static method to return the length of an Arbitrary Attribute header.

        Parameters:
         None
        Returns:
         The length of the RRALRecord header.
        """
        return 5

    @staticmethod
    def maximum_component_area_length():
        # type: () -> int
        """
        Static method to return the absolute maximum length an Arbitrary
        Attribute component area can be.

        Parameters:
         None
        Returns:
         The maximum length an Arbitrary Attribute component area can be.
        """
        return 255 - RRALRecord.header_length()

    @staticmethod
    def length(attrs):
        # type: (List[bytes]) -> int
        """
        Static method to return the length of a list of attributes.

        Parameters:
         attrs - A list of attributes.
        Returns:
         The length of the entire record in bytes.
        """
        length = RRALRecord.header_length()
        for attr in attrs:
            length += RRALRecord.Component.length(attr)
        return length


class RRNMRecord:
    """A class that represents a Rock Ridge Alternate Name record."""
    __slots__ = ('_initialized', 'posix_name_flags', 'posix_name')

    FMT = '=BBB'

    def __init__(self):
        # type: () -> None
        self._initialized = False
        self.posix_name_flags = 0
        self.posix_name = b''

    def parse(self, rrstr):
        # type: (bytes) -> None
        """
        Parse a Rock Ridge Alternate Name record out of a string.

        Parameters:
         rrstr - The string to parse the record out of.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('NM record already initialized')

        (su_len, su_entry_version_unused,
         self.posix_name_flags) = struct.unpack_from(self.FMT, rrstr[:5], 2)

        # We assume that the caller has already checked the su_entry_version,
        # so we don't bother.

        name_len = su_len - 5
        if (self.posix_name_flags & 0x7) not in (0, 1, 2, 4):
            raise pycdlibexception.PyCdlibInvalidISO('Invalid Rock Ridge NM flags')

        if name_len != 0:
            if (self.posix_name_flags & (1 << 1)) or (self.posix_name_flags & (1 << 2)) or (self.posix_name_flags & (1 << 5)):
                raise pycdlibexception.PyCdlibInvalidISO('Invalid name in Rock Ridge NM entry (0x%x %d)' % (self.posix_name_flags, name_len))
            self.posix_name += rrstr[5:5 + name_len]

        self._initialized = True

    def new(self, rr_name):
        # type: (bytes) -> None
        """
        Create a new Rock Ridge Alternate Name record.

        Parameters:
         rr_name - The name for the new record.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('NM record already initialized')

        self.posix_name = rr_name
        self.posix_name_flags = 0

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate a string representing the Rock Ridge Alternate Name record.

        Parameters:
         None.
        Returns:
         String containing the Rock Ridge record.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('NM record not initialized')

        return b'NM' + struct.pack(self.FMT,
                                   RRNMRecord.length(self.posix_name),
                                   SU_ENTRY_VERSION,
                                   self.posix_name_flags) + self.posix_name

    def set_continued(self):
        # type: () -> None
        """
        Mark this alternate name record as continued.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('NM record not initialized')

        self.posix_name_flags |= (1 << 0)

    @staticmethod
    def length(rr_name):
        # type: (bytes) -> int
        """
        Static method to return the length of the Rock Ridge Alternate Name
        record.

        Parameters:
         rr_name - The name to use.
        Returns:
         The length of this record in bytes.
        """
        return 5 + len(rr_name)


class RRCLRecord:
    """
    A class that represents a Rock Ridge Child Link record.  This record
    represents the logical block where a deeply nested directory was relocated
    to.
    """
    __slots__ = ('_initialized', 'child_log_block_num')

    FMT = '<BBLL'

    def __init__(self):
        # type: () -> None
        self.child_log_block_num = 0
        self._initialized = False

    def parse(self, rrstr):
        # type: (bytes) -> None
        """
        Parse a Rock Ridge Child Link record out of a string.

        Parameters:
         rrstr - The string to parse the record out of.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('CL record already initialized')

        # We assume that the caller has already checked the su_entry_version,
        # so we don't bother.

        (su_len, su_entry_version_unused,
         child_log_block_num_le, child_log_block_num_be) = struct.unpack_from(self.FMT, rrstr[:12], 2)
        if su_len != RRCLRecord.length():
            raise pycdlibexception.PyCdlibInvalidISO('Invalid length on rock ridge extension')

        if child_log_block_num_le != utils.swab_32bit(child_log_block_num_be):
            raise pycdlibexception.PyCdlibInvalidISO('Little endian block num does not equal big endian; corrupt ISO')
        self.child_log_block_num = child_log_block_num_le

        self._initialized = True

    def new(self):
        # type: () -> None
        """
        Create a new Rock Ridge Child Link record.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('CL record already initialized')

        self.child_log_block_num = 0  # This gets set later

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate a string representing the Rock Ridge Child Link record.

        Parameters:
         None.
        Returns:
         String containing the Rock Ridge record.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('CL record not initialized')

        return b'CL' + struct.pack(self.FMT,
                                   RRCLRecord.length(),
                                   SU_ENTRY_VERSION,
                                   self.child_log_block_num,
                                   utils.swab_32bit(self.child_log_block_num))

    def set_log_block_num(self, bl):
        # type: (int) -> None
        """
        Set the logical block number for the child.

        Parameters:
         bl - Logical block number of the child.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('CL record not initialized')

        self.child_log_block_num = bl

    @staticmethod
    def length():
        # type: () -> int
        """
        Static method to return the length of the Rock Ridge Child Link
        record.

        Parameters:
         None.
        Returns:
         The length of this record in bytes.
        """
        return 12


class RRPLRecord:
    """
    A class that represents a Rock Ridge Parent Link record.  This record
    represents the logical block where a deeply nested directory was located
    from.
    """
    __slots__ = ('_initialized', 'parent_log_block_num')

    FMT = '<BBLL'

    def __init__(self):
        # type: () -> None
        self.parent_log_block_num = 0
        self._initialized = False

    def parse(self, rrstr):
        # type: (bytes) -> None
        """
        Parse a Rock Ridge Parent Link record out of a string.

        Parameters:
         rrstr - The string to parse the record out of.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('PL record already initialized')

        # We assume that the caller has already checked the su_entry_version,
        # so we don't bother.

        (su_len, su_entry_version_unused,
         parent_log_block_num_le, parent_log_block_num_be) = struct.unpack_from(self.FMT, rrstr[:12], 2)
        if su_len != RRPLRecord.length():
            raise pycdlibexception.PyCdlibInvalidISO('Invalid length on rock ridge extension')
        if parent_log_block_num_le != utils.swab_32bit(parent_log_block_num_be):
            raise pycdlibexception.PyCdlibInvalidISO('Little endian block num does not equal big endian; corrupt ISO')
        self.parent_log_block_num = parent_log_block_num_le

        self._initialized = True

    def new(self):
        # type: () -> None
        """
        Generate a string representing the Rock Ridge Parent Link record.

        Parameters:
         None.
        Returns:
         String containing the Rock Ridge record.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('PL record already initialized')

        self.parent_log_block_num = 0  # This will get set later

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate a string representing the Rock Ridge Child Link record.

        Parameters:
         None.
        Returns:
         String containing the Rock Ridge record.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('PL record not initialized')

        return b'PL' + struct.pack(self.FMT,
                                   RRPLRecord.length(),
                                   SU_ENTRY_VERSION,
                                   self.parent_log_block_num,
                                   utils.swab_32bit(self.parent_log_block_num))

    def set_log_block_num(self, bl):
        # type: (int) -> None
        """
        Set the logical block number for the parent.

        Parameters:
         bl - Logical block number of the parent.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('PL record not initialized')

        self.parent_log_block_num = bl

    @staticmethod
    def length():
        # type: () -> int
        """
        Static method to return the length of the Rock Ridge Parent Link
        record.

        Parameters:
         None.
        Returns:
         The length of this record in bytes.
        """
        return 12


class RRTFRecord:
    """
    A class that represents a Rock Ridge Time Stamp record.  This record
    represents the creation timestamp, the access time timestamp, the
    modification time timestamp, the attribute change time timestamp, the
    backup time timestamp, the expiration time timestamp, and the effective time
    timestamp.  Each of the timestamps can be selectively enabled or disabled.
    Additionally, the timestamps can be configured to be Directory Record
    style timestamps (7 bytes) or Volume Descriptor style timestamps (17 bytes).
    """
    __slots__ = ('_initialized', 'creation_time', 'access_time',
                 'modification_time', 'attribute_change_time', 'backup_time',
                 'expiration_time', 'effective_time', 'time_flags')

    FIELDNAMES = ('creation_time', 'access_time', 'modification_time',
                  'attribute_change_time', 'backup_time', 'expiration_time',
                  'effective_time')

    def __init__(self):
        # type: () -> None
        for fieldname in self.FIELDNAMES:
            setattr(self, fieldname, None)

        self.time_flags = 0
        self._initialized = False

    def parse(self, rrstr):
        # type: (bytes) -> None
        """
        Parse a Rock Ridge Time Stamp record out of a string.

        Parameters:
         rrstr - The string to parse the record out of.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('TF record already initialized')

        # We assume that the caller has already checked the su_entry_version,
        # so we don't bother.

        (su_len, su_entry_version_unused,
         self.time_flags) = struct.unpack_from('=BBB', rrstr[:5], 2)
        if su_len < 5:
            raise pycdlibexception.PyCdlibInvalidISO('Not enough bytes in the TF record')

        tflen = 7
        if self.time_flags & (1 << 7):
            tflen = 17

        offset = 5
        for index, fieldname in enumerate(self.FIELDNAMES):
            if self.time_flags & (1 << index):
                if tflen == 7:
                    setattr(self, fieldname, dates.DirectoryRecordDate())
                elif tflen == 17:
                    setattr(self, fieldname, dates.VolumeDescriptorDate())
                getattr(self, fieldname).parse(rrstr[offset:offset + tflen])
                offset += tflen

        self._initialized = True

    def new(self, time_flags, date_seconds):
        # type: (int, float) -> None
        """
        Create a new Rock Ridge Time Stamp record.

        Parameters:
         time_flags - The flags to use for this time stamp record.
         date_seconds - Time and date, in seconds since the epoch, to use for
                        this time stamp record.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('TF record already initialized')

        self.time_flags = time_flags

        tflen = 7
        if self.time_flags & (1 << 7):
            tflen = 17

        for index, fieldname in enumerate(self.FIELDNAMES):
            if self.time_flags & (1 << index):
                if tflen == 7:
                    setattr(self, fieldname, dates.DirectoryRecordDate())
                elif tflen == 17:
                    setattr(self, fieldname, dates.VolumeDescriptorDate())
                getattr(self, fieldname).new(date_seconds)

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate a string representing the Rock Ridge Time Stamp record.

        Parameters:
         None.
        Returns:
         String containing the Rock Ridge record.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('TF record not initialized')

        outlist = [b'TF', struct.pack('=BBB',
                                      RRTFRecord.length(self.time_flags),
                                      SU_ENTRY_VERSION,
                                      self.time_flags)]
        for fieldname in self.FIELDNAMES:
            field = getattr(self, fieldname)
            if field is not None:
                outlist.append(field.record())

        return b''.join(outlist)

    @staticmethod
    def length(time_flags):
        # type: (int) -> int
        """
        Static method to return the length of the Rock Ridge Time Stamp
        record.

        Parameters:
         time_flags - Integer representing the flags to use.
        Returns:
         The length of this record in bytes.
        """
        tf_each_size = 7
        if time_flags & (1 << 7):
            tf_each_size = 17
        time_flags &= 0x7f
        tf_num = 0
        while time_flags:
            time_flags &= time_flags - 1
            tf_num += 1

        return 5 + tf_each_size * tf_num


class RRSFRecord:
    """
    A class that represents a Rock Ridge Sparse File record.  This record
    represents the full file size of a sparsely-populated file.
    """
    __slots__ = ('_initialized', 'virtual_file_size_high',
                 'virtual_file_size_low', 'table_depth')

    def __init__(self):
        # type: () -> None
        self.table_depth = None  # type: Optional[int]
        self.virtual_file_size_low = 0
        self.virtual_file_size_high = None  # type: Optional[int]
        self._initialized = False

    def parse(self, rrstr):
        # type: (bytes) -> None
        """
        Parse a Rock Ridge Sparse File record out of a string.

        Parameters:
         rrstr - The string to parse the record out of.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('SF record already initialized')

        # We assume that the caller has already checked the su_entry_version,
        # so we don't bother.

        (su_len,
         su_entry_version_unused) = struct.unpack_from('=BB', rrstr[:4], 2)

        if su_len == 12:
            # This is a Rock Ridge version 1.10 SF Record, which is 12 bytes.
            (virtual_file_size_le, virtual_file_size_be) = struct.unpack_from('<LL', rrstr[:12], 4)
            if virtual_file_size_le != utils.swab_32bit(virtual_file_size_be):
                raise pycdlibexception.PyCdlibInvalidISO('Virtual file size little-endian does not match big-endian')
            self.virtual_file_size_low = virtual_file_size_le
        elif su_len == 21:
            # This is a Rock Ridge version 1.12 SF Record, which is 21 bytes.
            (virtual_file_size_high_le, virtual_file_size_high_be, virtual_file_size_low_le,
             virtual_file_size_low_be, self.table_depth) = struct.unpack_from('<LLLLB', rrstr[:21], 4)
            if virtual_file_size_high_le != utils.swab_32bit(virtual_file_size_high_be):
                raise pycdlibexception.PyCdlibInvalidISO('Virtual file size high little-endian does not match big-endian')

            if virtual_file_size_low_le != utils.swab_32bit(virtual_file_size_low_be):
                raise pycdlibexception.PyCdlibInvalidISO('Virtual file size low little-endian does not match big-endian')
            self.virtual_file_size_low = virtual_file_size_low_le
            self.virtual_file_size_high = virtual_file_size_high_le
        else:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid length on Rock Ridge SF record (expected 12 or 21)')

        self._initialized = True

    def new(self, file_size_high, file_size_low, table_depth):
        # type: (Optional[int], int, Optional[int]) -> None
        """
        Create a new Rock Ridge Sparse File record.

        Parameters:
         file_size_high - The high-order 32-bits of the file size.
         file_size_low - The low-order 32-bits of the file size.
         table_depth - The maximum virtual file size.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('SF record already initialized')

        self.virtual_file_size_high = file_size_high
        self.virtual_file_size_low = file_size_low
        self.table_depth = table_depth

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate a string representing the Rock Ridge Sparse File record.

        Parameters:
         None.
        Returns:
         String containing the Rock Ridge record.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('SF record not initialized')

        length = 12
        if self.virtual_file_size_high is not None:
            length = 21
        ret = b'SF' + struct.pack('=BB', length, SU_ENTRY_VERSION)
        if self.virtual_file_size_high is not None and self.table_depth is not None:
            ret += struct.pack('<LLLLB',
                               self.virtual_file_size_high,
                               utils.swab_32bit(self.virtual_file_size_high),
                               self.virtual_file_size_low,
                               utils.swab_32bit(self.virtual_file_size_low),
                               self.table_depth)
        else:
            ret += struct.pack('<LL',
                               self.virtual_file_size_low,
                               utils.swab_32bit(self.virtual_file_size_low))

        return ret

    @staticmethod
    def length(rr_version):
        # type: (str) -> int
        """
        Static method to return the length of the Rock Ridge Sparse File
        record.

        Parameters:
         rr_version - The version of Rock Ridge in use; must be '1.10' or '1.12'.
        Returns:
         The length of this record in bytes.
        """
        if rr_version == '1.10':
            return 12
        if rr_version == '1.12':
            return 21

        # This should never happen
        raise pycdlibexception.PyCdlibInternalError('Invalid rr_version')


class RRRERecord:
    """
    A class that represents a Rock Ridge Relocated Directory record.  This
    record is used to mark an entry as having been relocated because it was
    deeply nested.
    """
    __slots__ = ('_initialized',)

    FMT = '=BB'

    def __init__(self):
        # type: () -> None
        self._initialized = False

    def parse(self, rrstr):
        # type: (bytes) -> None
        """
        Parse a Rock Ridge Relocated Directory record out of a string.

        Parameters:
         rrstr - The string to parse the record out of.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('RE record already initialized')

        (su_len,
         su_entry_version_unused) = struct.unpack_from(self.FMT, rrstr[:4], 2)

        # We assume that the caller has already checked the su_entry_version,
        # so we don't bother.

        if su_len != 4:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid length on rock ridge extension')

        self._initialized = True

    def new(self):
        # type: () -> None
        """
        Create a new Rock Ridge Relocated Directory record.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('RE record already initialized')

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate a string representing the Rock Ridge Relocated Directory
        record.

        Parameters:
         None.
        Returns:
         String containing the Rock Ridge record.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('RE record not initialized')

        return b'RE' + struct.pack(self.FMT,
                                   RRRERecord.length(),
                                   SU_ENTRY_VERSION)

    @staticmethod
    def length():
        # type: () -> int
        """
        Static method to return the length of the Rock Ridge Relocated Directory
        record.

        Parameters:
         None.
        Returns:
         The length of this record in bytes.
        """
        return 4


class RRSTRecord:
    """
    A class that represents a Rock Ridge System Terminator record.  This
    record is used to terminate the SUSP/Rock Ridge records in a Directory
    Entry.
    """
    __slots__ = ('_initialized',)

    FMT = '=BB'

    def __init__(self):
        # type: () -> None
        self._initialized = False

    def parse(self, rrstr):
        # type: (bytes) -> None
        """
        Parse a Rock Ridge System Terminator record out of a string.

        Parameters:
         rrstr - The string to parse the record out of.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('ST record already initialized')

        (su_len,
         su_entry_version_unused) = struct.unpack_from(self.FMT, rrstr[:4], 2)

        # We assume that the caller has already checked the su_entry_version,
        # so we don't bother.

        if su_len != 4:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid length on rock ridge extension')

        self._initialized = True

    def new(self):
        # type: () -> None
        """
        Create a new Rock Ridge System Terminator record.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('ST record already initialized')

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate a string representing the Rock Ridge System Terminator
        record.

        Parameters:
         None.
        Returns:
         String containing the Rock Ridge record.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('ST record not initialized')

        return b'ST' + struct.pack(self.FMT,
                                   RRSTRecord.length(),
                                   SU_ENTRY_VERSION)

    @staticmethod
    def length():
        # type: () -> int
        """
        Static method to return the length of the Rock Ridge System Terminator
        record.

        Parameters:
         None.
        Returns:
         The length of this record in bytes.
        """
        return 4


class RRPDRecord:
    """
    A class that represents a Rock Ridge Platform Dependent record.  This
    record is used to add platform-specific information to a Directory
    Entry, and may also be used as a terminator for Rock Ridge entries.
    """
    __slots__ = ('_initialized', 'padding')

    FMT = '=BB'

    def __init__(self):
        # type: () -> None
        self._initialized = False

    def parse(self, rrstr):
        # type: (bytes) -> None
        """
        Parse a Rock Ridge Platform Dependent record out of a string.

        Parameters:
         rrstr - The string to parse the record out of.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('PD record already initialized')

        (su_len_unused,
         su_entry_version_unused) = struct.unpack_from(self.FMT, rrstr[:4], 2)

        self.padding = rrstr[4:]

        # We assume that the caller has already checked the su_entry_version,
        # so we don't bother.

        self._initialized = True

    def new(self):
        # type: () -> None
        """
        Create a new Rock Ridge Platform Dependent record.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('PD record already initialized')

        self._initialized = True
        self.padding = b''

    def record(self):
        # type: () -> bytes
        """
        Generate a string representing the Rock Ridge Platform Dependent
        record.

        Parameters:
         None.
        Returns:
         String containing the Rock Ridge record.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('PD record not initialized')

        return b'PD' + struct.pack(self.FMT, RRPDRecord.length(self.padding),
                                   SU_ENTRY_VERSION) + self.padding

    @staticmethod
    def length(padding):
        # type: (bytes) -> int
        """
        Static method to return the length of the Rock Ridge Platform Dependent
        record.

        Parameters:
         padding - The padding bytes that this record will use.
        Returns:
         The length of this record in bytes.
        """
        return 4 + len(padding)


class RockRidgeEntries:
    """
    A simple class container to hold a long list of possible Rock Ridge
    records.
    """
    __slots__ = ('sp_record', 'rr_record', 'ce_record', 'px_record',
                 'er_record', 'es_records', 'pn_record', 'sl_records',
                 'nm_records', 'cl_record', 'pl_record', 'tf_record',
                 'sf_record', 're_record', 'st_record', 'pd_records',
                 'al_records')

    def __init__(self):
        # type: () -> None
        self.sp_record = None  # type: Optional[RRSPRecord]
        self.rr_record = None  # type: Optional[RRRRRecord]
        self.ce_record = None  # type: Optional[RRCERecord]
        self.px_record = None  # type: Optional[RRPXRecord]
        self.er_record = None  # type: Optional[RRERRecord]
        self.es_records = []  # type: List[RRESRecord]
        self.pn_record = None  # type: Optional[RRPNRecord]
        self.sl_records = []  # type: List[RRSLRecord]
        self.nm_records = []  # type: List[RRNMRecord]
        self.cl_record = None  # type: Optional[RRCLRecord]
        self.pl_record = None  # type: Optional[RRPLRecord]
        self.tf_record = None  # type: Optional[RRTFRecord]
        self.sf_record = None  # type: Optional[RRSFRecord]
        self.re_record = None  # type: Optional[RRRERecord]
        self.st_record = None  # type: Optional[RRSTRecord]
        self.pd_records = []  # type: List[RRPDRecord]
        self.al_records = []  # type: List[RRALRecord]


# This is the class that implements the Rock Ridge extensions for PyCdlib.  The
# Rock Ridge extensions are a set of extensions for embedding POSIX semantics
# on an ISO9660 filesystem.  Rock Ridge works by utilizing the 'System Use'
# area of the directory record to store additional metadata about files.  This
# includes things like POSIX users, groups, ctime, mtime, atime, etc., as well
# as the ability to have directory structures deeper than 8 and filenames longer
# than 8.3.  Rock Ridge depends on the System Use and Sharing Protocol (SUSP),
# which defines some standards on how to use the System Area.

class RockRidge:
    """A class representing Rock Ridge entries."""
    __slots__ = ('_initialized', 'dr_entries', 'ce_entries', 'cl_to_moved_dr',
                 'moved_to_cl_dr', 'parent_link', 'rr_version', 'ce_block',
                 'bytes_to_skip', '_full_name')

    def __init__(self):
        # type: () -> None
        self.dr_entries = RockRidgeEntries()
        self.ce_entries = RockRidgeEntries()
        self.cl_to_moved_dr = None  # type: Optional[dr.DirectoryRecord]
        self.moved_to_cl_dr = None  # type: Optional[dr.DirectoryRecord]
        self.parent_link = None  # type: Optional[dr.DirectoryRecord]
        self.rr_version = ''
        self.ce_block = None  # type: Optional[RockRidgeContinuationBlock]
        self._initialized = False

    def has_entry(self, name):
        # type: (str) -> bool
        """
        An internal method to tell if we have already parsed an entry of the
        named type.

        Parameters:
         name - The name of the entry to check.
        Returns:
         True if we have already parsed an entry of the named type, False
         otherwise.
        """
        return getattr(self.dr_entries, name) or getattr(self.ce_entries, name)

    def parse(self, record, is_first_dir_record_of_root, bytes_to_skip,
              continuation, dr_name):
        # type: (bytes, bool, int, bool, bytes) -> None
        """
        Method to parse a rock ridge record.

        Parameters:
         record - The record to parse.
         is_first_dir_record_of_root - Whether this is the first directory
                                       record of the root directory record;
                                       certain Rock Ridge entries are only
                                       valid there.
         bytes_to_skip - The number of bytes to skip at the beginning of the
                         record.
         continuation - Whether the new entries should go in the continuation
                        list or in the DR list.
         dr_name - The original DirectoryRecord name, only to be returned in the
                   case that the Rock Ridge NM records are completely empty.
        Returns:
         Nothing.
        """
        # Note that we very explicitly do not check if self._initialized is True
        # here; this can be called multiple times in the case where there is
        # a continuation entry.

        if continuation:
            entry_list = self.ce_entries
        else:
            entry_list = self.dr_entries

        self.bytes_to_skip = bytes_to_skip
        offset = bytes_to_skip
        left = len(record)
        px_record_length = None
        has_es_record = False
        sf_record_length = None
        er_id = None
        while True:
            if left == 0:
                break
            if left == 1:
                # There may be a padding byte on the end.
                if bytes(bytearray([record[offset]])) != b'\x00':
                    raise pycdlibexception.PyCdlibInvalidISO('Invalid pad byte')
                break
            if left < 4:
                raise pycdlibexception.PyCdlibInvalidISO('Not enough bytes left in the System Use field')

            (rtype, su_len, su_entry_version) = struct.unpack_from('=2sBB', record[:offset + 4], offset)
            if su_entry_version != SU_ENTRY_VERSION:
                raise pycdlibexception.PyCdlibInvalidISO('Invalid RR version %d!' % su_entry_version)
            if su_len == 0:
                raise pycdlibexception.PyCdlibInvalidISO('Zero size for Rock Ridge entry length')

            recslice = record[offset:]

            if rtype in (b'SP', b'RR', b'CE', b'PX', b'ST', b'ER',
                         b'PN', b'CL', b'PL', b'RE', b'TF', b'SF'):
                recname = rtype.decode('utf-8').lower() + '_record'
                if self.has_entry(recname):
                    raise pycdlibexception.PyCdlibInvalidISO('Only single %s record supported' % (rtype.decode('utf-8')))

            if rtype == b'SP':
                if left < 7 or not is_first_dir_record_of_root:
                    raise pycdlibexception.PyCdlibInvalidISO('Invalid SUSP SP record')

                # OK, this is the first Directory Record of the root
                # directory, which means we should check it for the SUSP/RR
                # extension, which is exactly 7 bytes and starts with 'SP'.

                entry_list.sp_record = RRSPRecord()
                entry_list.sp_record.parse(recslice)
            elif rtype == b'RR':
                entry_list.rr_record = RRRRRecord()
                entry_list.rr_record.parse(recslice)
            elif rtype == b'CE':
                entry_list.ce_record = RRCERecord()
                entry_list.ce_record.parse(recslice)
            elif rtype == b'PX':
                entry_list.px_record = RRPXRecord()
                px_record_length = entry_list.px_record.parse(recslice)
            elif rtype == b'PD':
                pd = RRPDRecord()
                pd.parse(recslice)
                entry_list.pd_records.append(pd)
            elif rtype == b'ST':
                entry_list.st_record = RRSTRecord()
                entry_list.st_record.parse(recslice)
            elif rtype == b'ER':
                entry_list.er_record = RRERRecord()
                entry_list.er_record.parse(recslice)
                er_id = entry_list.er_record.ext_id
            elif rtype == b'ES':
                es = RRESRecord()
                es.parse(recslice)
                entry_list.es_records.append(es)
                has_es_record = True
            elif rtype == b'PN':
                entry_list.pn_record = RRPNRecord()
                entry_list.pn_record.parse(recslice)
            elif rtype == b'SL':
                new_sl_record = RRSLRecord()
                new_sl_record.parse(recslice)
                entry_list.sl_records.append(new_sl_record)
            elif rtype == b'NM':
                new_nm_record = RRNMRecord()
                new_nm_record.parse(recslice)
                entry_list.nm_records.append(new_nm_record)
            elif rtype == b'CL':
                entry_list.cl_record = RRCLRecord()
                entry_list.cl_record.parse(recslice)
            elif rtype == b'PL':
                entry_list.pl_record = RRPLRecord()
                entry_list.pl_record.parse(recslice)
            elif rtype == b'RE':
                entry_list.re_record = RRRERecord()
                entry_list.re_record.parse(recslice)
            elif rtype == b'TF':
                entry_list.tf_record = RRTFRecord()
                entry_list.tf_record.parse(recslice)
            elif rtype == b'SF':
                entry_list.sf_record = RRSFRecord()
                entry_list.sf_record.parse(recslice)
                sf_record_length = len(recslice)
            elif rtype == b'AL':
                new_al_record = RRALRecord()
                new_al_record.parse(recslice)
                entry_list.al_records.append(new_al_record)
            else:
                raise pycdlibexception.PyCdlibInvalidISO('Unknown SUSP record')
            offset += su_len
            left -= su_len

        # Now let's determine the version of Rock Ridge that we have (1.09,
        # 1.10, or 1.12).  Unfortunately, there is no direct information from
        # Rock Ridge, so we infer it from what is present.  In an ideal world,
        # the following table would tell us:
        #
        # | Feature/Rock Ridge version |      1.09     |      1.10     |      1.12     |
        # +----------------------------+---------------+---------------+---------------+
        # |    Has RR Record?          | True or False |     False     |     False     |
        # |    Has ES Record?          |     False     |     False     | True or False |
        # |    Has SF Record?          |     False     | True or False | True or False |
        # |    PX Record length        |      36       |      36       |      44       |
        # |    SF Record length        |     N/A       |      12       |      21       |
        # |    ER Desc string          |  RRIP_1991A   |  RRIP_1991A   |  IEEE_P1282   |
        # +----------------------------+---------------+---------------+---------------+
        #
        # While that is a good start, we don't live in an ideal world.  In
        # particular, we've seen ISOs in the wild (OpenSolaris 2008) that put an
        # RR record into an otherwise 1.12 Rock Ridge entry.  So we'll use the
        # above as a hint, and allow for some wiggle room.

        if px_record_length == 44 or sf_record_length == 21 or has_es_record or er_id == EXT_ID_112:
            self.rr_version = '1.12'
        else:
            # Not 1.12, so either 1.09 or 1.10.
            if sf_record_length == 12:
                self.rr_version = '1.10'
            else:
                self.rr_version = '1.09'

        namelist = [nm.posix_name for nm in self.dr_entries.nm_records]
        namelist.extend([nm.posix_name for nm in self.ce_entries.nm_records])
        if len(namelist) > 0:
            self._full_name = b''.join(namelist)
        else:
            self._full_name = dr_name

        self._initialized = True

    def _record(self, entries):
        # type: (RockRidgeEntries) -> bytes
        """
        Return a string representing the Rock Ridge entry.

        Parameters:
         entries - The dr_entries or ce_entries to generate a record for.
        Returns:
         A string representing the Rock Ridge entry.
        """

        outlist = []
        if entries.sp_record is not None:
            outlist.append(entries.sp_record.record())

        if entries.rr_record is not None:
            outlist.append(entries.rr_record.record())

        for nm_record in entries.nm_records:
            outlist.append(nm_record.record())

        if entries.px_record is not None:
            outlist.append(entries.px_record.record(self.rr_version))

        for sl_record in entries.sl_records:
            outlist.append(sl_record.record())

        if entries.tf_record is not None:
            outlist.append(entries.tf_record.record())

        if entries.cl_record is not None:
            outlist.append(entries.cl_record.record())

        if entries.pl_record is not None:
            outlist.append(entries.pl_record.record())

        if entries.re_record is not None:
            outlist.append(entries.re_record.record())

        for es_record in entries.es_records:
            outlist.append(es_record.record())

        if entries.er_record is not None:
            outlist.append(entries.er_record.record())

        for al_record in entries.al_records:
            outlist.append(al_record.record())

        if entries.ce_record is not None:
            outlist.append(entries.ce_record.record())

        for pd_record in entries.pd_records:
            outlist.append(pd_record.record())

        if entries.st_record is not None:
            outlist.append(entries.st_record.record())

        if entries.sf_record is not None:
            outlist.append(entries.sf_record.record())

        return b''.join(outlist)

    def record_dr_entries(self):
        # type: () -> bytes
        """
        Return a string representing the Rock Ridge entries in the Directory
        Record.

        Parameters:
         None.
        Returns:
         A string representing the Rock Ridge entry.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Rock Ridge extension not initialized')

        return self._record(self.dr_entries)

    def record_ce_entries(self):
        # type: () -> bytes
        """
        Return a string representing the Rock Ridge entries in the Continuation
        Entry.

        Parameters:
         None.
        Returns:
         A string representing the Rock Ridge entry.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Rock Ridge extension not initialized')

        return self._record(self.ce_entries)

    def _new_symlink(self, symlink_path, curr_dr_len):
        # type: (bytes, int) -> int
        """
        An internal method to add the appropriate symlink record(s) to the ISO.

        Parameters:
         symlink_path - The absolute symlink path to add to the ISO.
         curr_dr_len - The current directory record length.
        Returns:
         The new directory record length.
        """
        # This is more complicated than I realized.  There are up to 3 layers
        # of maximum length:
        # 1.  If we are still using the directory record, then we are
        #     subject to the maximum length left in the directory record.
        # 2.  The SL entry length is an 8-bit number, so we may need multiple
        #     SL entries in order to encode all of the components.
        # 3.  The Component header is also an 8-bit number, so we may need
        #     multiple SL records to record this component.
        #
        # Note that the component header length can never be longer than the SL
        # entry length.  Thus, we are reduced to 2 lengths to worry about.

        if curr_dr_len + RRSLRecord.length(symlink_path.split(b'/')) > ALLOWED_DR_SIZE:
            if self.dr_entries.ce_record is None:
                return -1

        curr_sl = RRSLRecord()
        curr_sl.new()

        sl_rec_header_len = RRSLRecord.header_length()

        thislen = RRSLRecord.length([b'a'])
        if curr_dr_len + thislen < ALLOWED_DR_SIZE:
            # There is enough room in the directory record for at least
            # part of the symlink
            curr_comp_area_length = ALLOWED_DR_SIZE - curr_dr_len - sl_rec_header_len
            self.dr_entries.sl_records.append(curr_sl)
            curr_dr_len += sl_rec_header_len
            sl_in_dr = True
        else:
            # Not enough room in the directory record, so proceed to
            # the continuation entry directly.
            curr_comp_area_length = RRSLRecord.maximum_component_area_length()
            self.ce_entries.sl_records.append(curr_sl)
            if self.dr_entries.ce_record is not None:
                self.dr_entries.ce_record.add_record(sl_rec_header_len)
            sl_in_dr = False

        for index, comp in enumerate(symlink_path.split(b'/')):
            special = False
            if index == 0 and comp == b'':
                comp = b'/'
                special = True
                mincomp = comp
            elif comp == b'.':
                special = True
                mincomp = comp
            elif comp == b'..':
                special = True
                mincomp = comp
            else:
                mincomp = b'a'

            offset = 0
            done = False
            while not done:
                minimum = RRSLRecord.Component.length(mincomp)
                if minimum > curr_comp_area_length:
                    # There wasn't enough room in the last SL record
                    # for more data.  Set the 'continued' flag on the old
                    # SL record, and then create a new one.
                    curr_sl.set_continued()
                    if offset != 0:
                        # If we need to continue this particular
                        # *component* in the next SL record, then we
                        # also need to mark the curr_sl's last component
                        # header as continued.
                        curr_sl.set_last_component_continued()

                    curr_sl = RRSLRecord()
                    curr_sl.new()
                    self.ce_entries.sl_records.append(curr_sl)
                    curr_comp_area_length = RRSLRecord.maximum_component_area_length()
                    if self.dr_entries.ce_record is not None:
                        self.dr_entries.ce_record.add_record(sl_rec_header_len)
                    sl_in_dr = False

                if special:
                    complen = minimum
                    length = 0
                    compslice = comp
                else:
                    complen = RRSLRecord.Component.length(comp[offset:])
                    if complen > curr_comp_area_length:
                        length = curr_comp_area_length - 2
                    else:
                        length = complen
                    compslice = comp[offset:offset + length]

                curr_sl.add_component(compslice)

                if sl_in_dr:
                    curr_dr_len += RRSLRecord.Component.length(compslice)
                else:
                    if self.dr_entries.ce_record is not None:
                        self.dr_entries.ce_record.add_record(RRSLRecord.Component.length(compslice))

                offset += length

                curr_comp_area_length = curr_comp_area_length - length - 2

                if special:
                    done = True
                else:
                    if offset >= len(comp):
                        done = True

        return curr_dr_len

    def _new_attributes(self, attributes, curr_dr_len):
        # type: (Dict[bytes, bytes], int) -> int
        """
        An internal method to add arbitrary attributes to the ISO.

        Parameters:
         attributes - A dictionary of attributes to add to the ISO.
         curr_dr_len - The current directory record length.
        Returns:
         The new directory record length.
        """
        attr_list = list(attributes.keys()) + list(attributes.values())
        if curr_dr_len + RRALRecord.length(attr_list) > ALLOWED_DR_SIZE:
            if self.dr_entries.ce_record is None:
                return -1

        curr_al = RRALRecord()
        curr_al.new()

        al_rec_header_len = RRALRecord.header_length()

        thislen = RRALRecord.length([b'a'])
        if curr_dr_len + thislen < ALLOWED_DR_SIZE:
            # There is enough room in the directory record for at least
            # part of one of the attributes.
            curr_comp_area_length = ALLOWED_DR_SIZE - curr_dr_len - al_rec_header_len
            self.dr_entries.al_records.append(curr_al)
            curr_dr_len += al_rec_header_len
            al_in_dr = True
        else:
            # Not enough room in the directory record, so proceed to
            # the continuation entry directly.
            curr_comp_area_length = RRALRecord.maximum_component_area_length()
            if self.dr_entries.ce_record is not None:
                self.dr_entries.ce_record.add_record(al_rec_header_len)
            self.ce_entries.al_records.append(curr_al)
            al_in_dr = False

        for attr in attr_list:
            offset = 0
            done = False
            while not done:
                minimum = RRALRecord.Component.length(b'a')
                if minimum > curr_comp_area_length:
                    # There wasn't enough room in the last AL record
                    # for more data.  Set the 'continued' flag on the old
                    # AL record, and then create a new one.
                    curr_al.set_continued()
                    if offset != 0:
                        # If we need to continue this particular
                        # *component* in the next AL record, then we
                        # also need to mark the curr_al's last component
                        # header as continued.
                        curr_al.set_last_component_continued()

                    curr_al = RRALRecord()
                    curr_al.new()
                    self.ce_entries.al_records.append(curr_al)
                    curr_comp_area_length = RRALRecord.maximum_component_area_length()
                    if self.dr_entries.ce_record is not None:
                        self.dr_entries.ce_record.add_record(al_rec_header_len)
                    al_in_dr = False

                complen = RRALRecord.Component.length(attr[offset:])
                if complen > curr_comp_area_length:
                    length = curr_comp_area_length - 2
                else:
                    length = complen
                compslice = attr[offset:offset + length]

                curr_al.add_component(compslice)

                if al_in_dr:
                    curr_dr_len += RRALRecord.Component.length(compslice)
                else:
                    if self.dr_entries.ce_record is not None:
                        self.dr_entries.ce_record.add_record(RRALRecord.Component.length(compslice))

                offset += length

                curr_comp_area_length = curr_comp_area_length - length - 2

                if offset >= len(attr):
                    done = True

        return curr_dr_len

    def _add_name(self, rr_name, curr_dr_len):
        # type: (bytes, int) -> int
        """
        An internal method to add the appropriate name records to the ISO.

        Parameters:
         rr_name - The Rock Ridge name to add to the ISO.
         curr_dr_len - The current directory record length.
        Returns:
         The new directory record length.
        """
        # The length we are putting in this object (as opposed to the
        # continuation entry) is the maximum, minus how much is already in the
        # DR, minus 5 for the NM metadata.  We know that at least part of the
        # NM record will always fit in this DR.  That's because the DR is a
        # maximum size of 255, and the ISO9660 fields uses a maximum of 34 bytes
        # for metadata and 8+1+3+1+5 (8 for name, 1 for dot, 3 for extension,
        # 1 for semicolon, and 5 for version number, allowed up to 32767), which
        # leaves the System Use entry with 255 - 34 - 18 = 203 bytes.  Before
        # this record, the only records we ever put in place could be the SP or
        # the RR record, and the combination of them is never > 203, so we will
        # always put some NM data in here.

        len_here = ALLOWED_DR_SIZE - curr_dr_len - 5
        if len_here < len(rr_name):
            # If there isn't room in the DR entry for the entire name, we know
            # we need a CE record to fit it.
            if self.dr_entries.ce_record is None:
                return -1

            len_here = max(len_here, 0)

        curr_nm = None
        if len_here > 0:
            curr_nm = RRNMRecord()
            curr_nm.new(rr_name[:len_here])
            self.dr_entries.nm_records.append(curr_nm)
            curr_dr_len += RRNMRecord.length(rr_name[:len_here])

        offset = len_here
        while offset < len(rr_name):
            if self.dr_entries.ce_record is None:
                return -1

            if curr_nm is not None:
                curr_nm.set_continued()

            # We clip the length for this NM entry to 250, as that is
            # the maximum possible size for an NM entry.
            length = min(len(rr_name[offset:]), 250)

            curr_nm = RRNMRecord()
            curr_nm.new(rr_name[offset:offset + length])
            self.ce_entries.nm_records.append(curr_nm)
            self.dr_entries.ce_record.add_record(RRNMRecord.length(rr_name[offset:offset + length]))

            offset += length

        return curr_dr_len

    def _assign_entries(self, is_first_dir_record_of_root, rr_name, file_mode,
                        symlink_path, rr_relocated_child, rr_relocated,
                        rr_relocated_parent, bytes_to_skip, curr_dr_len,
                        attributes, date_seconds):
        # type: (bool, bytes, int, bytes, bool, bool, bool, int, int, Dict[bytes, bytes], float) -> int
        """
        Assign Rock Ridge entries to the appropriate DR or CE record.

        Parameters:
         is_first_dir_record_of_root - Whether this is the first directory
                                       record of the root directory record;
                                       certain Rock Ridge entries are only
                                       valid there.
         rr_name - The alternate name for this Rock Ridge entry.
         file_mode - The Unix file mode for this Rock Ridge entry.
         symlink_path - The path to the target of the symlink, or None if this
                        is not a symlink.
         rr_relocated_child - Whether this is a relocated child entry.
         rr_relocated - Whether this is a relocated entry.
         rr_relocated_parent - Whether this is a relocated parent entry.
         bytes_to_skip - The number of bytes to skip for the record.
         curr_dr_len - The current length of the directory record; this is used
                       when figuring out whether a continuation entry is needed.
         attributes - Arbitrary attributes to add to the Rock Ridge entry.
         date_seconds - Time and date, in seconds, to use for this Rock Ridge
                        record.
        Returns:
         The length of the directory record after the Rock Ridge extension has
         been added, or -1 if the entry will not fit.
        """
        # For SP Record
        if is_first_dir_record_of_root:
            new_sp = RRSPRecord()
            new_sp.new(bytes_to_skip)
            thislen = RRSPRecord.length()
            if curr_dr_len + thislen > ALLOWED_DR_SIZE:
                if self.dr_entries.ce_record is None:
                    # In reality, this can never happen.  If the SP record pushes
                    # us over the DR limit, then there is no room for a CE record
                    # either, and we are going to fail.  We leave this in place
                    # both for consistency with other records and to keep mypy
                    # happy.
                    return -1
                self.dr_entries.ce_record.add_record(thislen)
                self.ce_entries.sp_record = new_sp
            else:
                curr_dr_len += thislen
                self.dr_entries.sp_record = new_sp

        # For RR Record
        rr_record = None
        if self.rr_version == '1.09':
            rr_record = RRRRRecord()
            rr_record.new()
            thislen = RRRRRecord.length()
            if curr_dr_len + thislen > ALLOWED_DR_SIZE:
                if self.dr_entries.ce_record is None:
                    # In reality, this can never happen.  If the RR record pushes
                    # us over the DR limit, then there is no room for a CE record
                    # either, and we are going to fail.  We leave this in place
                    # both for consistency with other records and to keep mypy
                    # happy.
                    return -1
                self.dr_entries.ce_record.add_record(thislen)
                self.ce_entries.rr_record = rr_record
            else:
                curr_dr_len += thislen
                self.dr_entries.rr_record = rr_record

        # For NM record
        if rr_name:
            curr_dr_len = self._add_name(rr_name, curr_dr_len)
            if curr_dr_len < 0:
                return -1

            if rr_record is not None:
                rr_record.append_field('NM')

        # For PX record
        new_px = RRPXRecord()
        new_px.new(file_mode)
        thislen = RRPXRecord.length(self.rr_version)
        if curr_dr_len + thislen > ALLOWED_DR_SIZE:
            if self.dr_entries.ce_record is None:
                return -1
            self.dr_entries.ce_record.add_record(thislen)
            self.ce_entries.px_record = new_px
        else:
            curr_dr_len += thislen
            self.dr_entries.px_record = new_px

        if rr_record is not None:
            rr_record.append_field('PX')

        # For SL record
        if symlink_path:
            curr_dr_len = self._new_symlink(symlink_path, curr_dr_len)
            if curr_dr_len < 0:
                return -1

            if rr_record is not None:
                rr_record.append_field('SL')

        # For TF record
        new_tf = RRTFRecord()
        new_tf.new(TF_FLAGS, date_seconds)
        thislen = RRTFRecord.length(TF_FLAGS)
        if curr_dr_len + thislen > ALLOWED_DR_SIZE:
            if self.dr_entries.ce_record is None:
                return -1
            self.dr_entries.ce_record.add_record(thislen)
            self.ce_entries.tf_record = new_tf
        else:
            curr_dr_len += thislen
            self.dr_entries.tf_record = new_tf

        if rr_record is not None:
            rr_record.append_field('TF')

        # For CL record
        if rr_relocated_child:
            new_cl = RRCLRecord()
            new_cl.new()
            thislen = RRCLRecord.length()
            if curr_dr_len + thislen > ALLOWED_DR_SIZE:
                if self.dr_entries.ce_record is None:
                    return -1
                self.dr_entries.ce_record.add_record(thislen)
                self.ce_entries.cl_record = new_cl
            else:
                curr_dr_len += thislen
                self.dr_entries.cl_record = new_cl

            if rr_record is not None:
                rr_record.append_field('CL')

        # For RE record
        if rr_relocated:
            new_re = RRRERecord()
            new_re.new()
            thislen = RRRERecord.length()
            if curr_dr_len + thislen > ALLOWED_DR_SIZE:
                if self.dr_entries.ce_record is None:
                    return -1
                self.dr_entries.ce_record.add_record(thislen)
                self.ce_entries.re_record = new_re
            else:
                curr_dr_len += thislen
                self.dr_entries.re_record = new_re

            if rr_record is not None:
                rr_record.append_field('RE')

        # For PL record
        if rr_relocated_parent:
            new_pl = RRPLRecord()
            new_pl.new()
            thislen = RRPLRecord.length()
            if curr_dr_len + thislen > ALLOWED_DR_SIZE:
                if self.dr_entries.ce_record is None:
                    return -1
                self.dr_entries.ce_record.add_record(thislen)
                self.ce_entries.pl_record = new_pl
            else:
                curr_dr_len += thislen
                self.dr_entries.pl_record = new_pl

            if rr_record is not None:
                rr_record.append_field('PL')

        # For ER record
        if is_first_dir_record_of_root:
            new_er = RRERRecord()
            if self.rr_version in ('1.09', '1.10'):
                new_er.new(EXT_ID_109, EXT_DES_109, EXT_SRC_109)
                thislen = RRERRecord.length(EXT_ID_109, EXT_DES_109, EXT_SRC_109)
            else:
                # Assume 1.12
                new_er.new(EXT_ID_112, EXT_DES_112, EXT_SRC_112)
                thislen = RRERRecord.length(EXT_ID_112, EXT_DES_112, EXT_SRC_112)

            if curr_dr_len + thislen > ALLOWED_DR_SIZE:
                if self.dr_entries.ce_record is None:
                    return -1
                self.dr_entries.ce_record.add_record(thislen)
                self.ce_entries.er_record = new_er
            else:
                curr_dr_len += thislen
                self.dr_entries.er_record = new_er

        # For AL record
        if attributes:
            curr_dr_len = self._new_attributes(attributes, curr_dr_len)
            if curr_dr_len < 0:
                return -1

        return curr_dr_len

    def new(self, is_first_dir_record_of_root, rr_name, file_mode,
            symlink_path, rr_version, rr_relocated_child, rr_relocated,
            rr_relocated_parent, bytes_to_skip, curr_dr_len, attributes,
            date_seconds):
        # type: (bool, bytes, int, bytes, str, bool, bool, bool, int, int, Dict[bytes, bytes], float) -> int
        """
        Create a new Rock Ridge record.

        Parameters:
         is_first_dir_record_of_root - Whether this is the first directory
                                       record of the root directory record;
                                       certain Rock Ridge entries are only
                                       valid there.
         rr_name - The alternate name for this Rock Ridge entry.
         file_mode - The Unix file mode for this Rock Ridge entry.
         symlink_path - The path to the target of the symlink, or None if this
                        is not a symlink.
         rr_version - The version of Rock Ridge to use; must be '1.09', '1.10',
                      or '1.12'.
         rr_relocated_child - Whether this is a relocated child entry.
         rr_relocated - Whether this is a relocated entry.
         rr_relocated_parent - Whether this is a relocated parent entry.
         bytes_to_skip - The number of bytes to skip for the record.
         curr_dr_len - The current length of the directory record; this is used
                       when figuring out whether a continuation entry is needed.
         attributes - Arbitrary attributes to add to the record.  This is a
                      non-standard extension, so use with care.
         date_seconds - Time and date, in seconds, to use for this Rock Ridge
                        record.
        Returns:
         The length of the directory record after the Rock Ridge extension has
         been added.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Rock Ridge extension already initialized')

        if rr_version not in ('1.09', '1.10', '1.12'):
            raise pycdlibexception.PyCdlibInvalidInput('Only Rock Ridge versions 1.09, 1.10, and 1.12 are implemented')

        self.rr_version = rr_version

        new_dr_len = self._assign_entries(is_first_dir_record_of_root, rr_name,
                                          file_mode, symlink_path,
                                          rr_relocated_child, rr_relocated,
                                          rr_relocated_parent, bytes_to_skip,
                                          curr_dr_len, attributes, date_seconds)

        if new_dr_len < 0:
            self.dr_entries = RockRidgeEntries()
            self.ce_entries = RockRidgeEntries()

            self.dr_entries.ce_record = RRCERecord()
            self.dr_entries.ce_record.new()
            curr_dr_len += RRCERecord.length()

            new_dr_len = self._assign_entries(is_first_dir_record_of_root,
                                              rr_name, file_mode, symlink_path,
                                              rr_relocated_child, rr_relocated,
                                              rr_relocated_parent, bytes_to_skip,
                                              curr_dr_len, attributes, date_seconds)
            if new_dr_len < 0:
                raise pycdlibexception.PyCdlibInternalError('Could not assign Rock Ridge entries')

        if new_dr_len > ALLOWED_DR_SIZE:
            raise pycdlibexception.PyCdlibInternalError('Rock Ridge entry increased DR length too far')

        new_dr_len += (new_dr_len % 2)

        namelist = [nm.posix_name for nm in self.dr_entries.nm_records]
        namelist.extend([nm.posix_name for nm in self.ce_entries.nm_records])
        self._full_name = b''.join(namelist)

        self._initialized = True

        return new_dr_len

    def add_to_file_links(self):
        # type: () -> None
        """
        Increment the number of POSIX file links on this entry by one.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Rock Ridge extension not initialized')

        if self.dr_entries.px_record is None:
            if self.ce_entries.px_record is None:
                raise pycdlibexception.PyCdlibInvalidInput('No Rock Ridge file links')
            self.ce_entries.px_record.posix_file_links += 1
        else:
            self.dr_entries.px_record.posix_file_links += 1

    def remove_from_file_links(self):
        # type: () -> None
        """
        Decrement the number of POSIX file links on this entry by one.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Rock Ridge extension not initialized')

        if self.dr_entries.px_record is None:
            if self.ce_entries.px_record is None:
                raise pycdlibexception.PyCdlibInvalidInput('No Rock Ridge file links')
            self.ce_entries.px_record.posix_file_links -= 1
        else:
            self.dr_entries.px_record.posix_file_links -= 1

    def copy_file_links(self, src):
        # type: (RockRidge) -> None
        """
        Copy the number of file links from the source Rock Ridge entry into
        this Rock Ridge entry.

        Parameters:
         src - The source Rock Ridge entry to copy from.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Rock Ridge extension not initialized')

        # First, get the src data
        if src.dr_entries.px_record is None:
            if src.ce_entries.px_record is None:
                raise pycdlibexception.PyCdlibInvalidInput('No Rock Ridge file links')
            num_links = src.ce_entries.px_record.posix_file_links
        else:
            num_links = src.dr_entries.px_record.posix_file_links

        # Now apply it to this record.
        if self.dr_entries.px_record is None:
            if self.ce_entries.px_record is None:
                raise pycdlibexception.PyCdlibInvalidInput('No Rock Ridge file links')
            self.ce_entries.px_record.posix_file_links = num_links
        else:
            self.dr_entries.px_record.posix_file_links = num_links

    def get_file_mode(self):
        # type: () -> int
        """
        Get the POSIX file mode bits for this Rock Ridge entry.

        Parameters:
         None.
        Returns:
         The POSIX file mode bits for this Rock Ridge entry.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Rock Ridge extension not initialized')

        if self.dr_entries.px_record is None:
            if self.ce_entries.px_record is None:
                raise pycdlibexception.PyCdlibInvalidInput('No Rock Ridge file mode')
            return self.ce_entries.px_record.posix_file_mode

        return self.dr_entries.px_record.posix_file_mode

    def name(self):
        # type: () -> bytes
        """
        Get the alternate name from this Rock Ridge entry.

        Parameters:
         None.
        Returns:
         The alternate name from this Rock Ridge entry.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Rock Ridge extension not initialized')

        return self._full_name

    def _is_symlink(self):
        # type: () -> bool
        """Internal method to determine whether this Rock Ridge entry is a symlink."""
        return len(self.dr_entries.sl_records) > 0 or len(self.ce_entries.sl_records) > 0

    def is_symlink(self):
        # type: () -> bool
        """
        Determine whether this Rock Ridge entry describes a symlink.

        Parameters:
         None.
        Returns:
         True if this Rock Ridge entry describes a symlink, False otherwise.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Rock Ridge extension not initialized')

        return self._is_symlink()

    def symlink_path(self):
        # type: () -> bytes
        """
        Get the path as a string of the symlink target of this Rock Ridge entry
        (if this is a symlink).

        Parameters:
         None.
        Returns:
         Symlink path as a string.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Rock Ridge extension not initialized')

        if not self._is_symlink():
            raise pycdlibexception.PyCdlibInvalidInput('Entry is not a symlink!')

        outlist = []
        saved = b''
        for rec in self.dr_entries.sl_records + self.ce_entries.sl_records:
            if rec.last_component_continued():
                saved += rec.name()
            else:
                saved += rec.name()
                outlist.append(saved)
                saved = b''

        if saved != b'':
            raise pycdlibexception.PyCdlibInvalidISO('Saw a continued symlink record with no end; ISO is probably malformed')

        return b'/'.join(outlist)

    def child_link_record_exists(self):
        # type: () -> bool
        """
        Determine whether this Rock Ridge entry has a child link record (used
        for relocating deep directory records).

        Parameters:
         None.
        Returns:
         True if this Rock Ridge entry has a child link record, False otherwise.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Rock Ridge extension not initialized')

        return self.dr_entries.cl_record is not None or self.ce_entries.cl_record is not None

    def child_link_update_from_dirrecord(self):
        # type: () -> None
        """
        Update the logical extent number stored in the child link record (if
        there is one), from the directory record entry that was stored in
        the child_link member.  This is used at the end of reshuffling extents
        to properly update the child link records.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Rock Ridge extension not initialized')

        if self.cl_to_moved_dr is None:
            raise pycdlibexception.PyCdlibInvalidInput('No child link found!')

        if self.dr_entries.cl_record is not None:
            self.dr_entries.cl_record.set_log_block_num(self.cl_to_moved_dr.extent_location())
        elif self.ce_entries.cl_record is not None:
            self.ce_entries.cl_record.set_log_block_num(self.cl_to_moved_dr.extent_location())
        else:
            raise pycdlibexception.PyCdlibInvalidInput('Could not find child link record!')

    def child_link_extent(self):
        # type: () -> int
        """
        Get the extent of the child of this entry if it has one.

        Parameters:
         None.
        Returns:
         The logical block number of the child if it exists.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Rock Ridge extension not initialized')

        if self.dr_entries.cl_record is not None:
            return self.dr_entries.cl_record.child_log_block_num
        if self.ce_entries.cl_record is not None:
            return self.ce_entries.cl_record.child_log_block_num

        raise pycdlibexception.PyCdlibInternalError('Asked for child extent for non-existent child record')

    def parent_link_record_exists(self):
        # type: () -> bool
        """
        Determine whether this Rock Ridge entry has a parent link record (used
        for relocating deep directory records).

        Parameters:
         None:
        Returns:
         True if this Rock Ridge entry has a parent link record, False otherwise.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Rock Ridge extension not initialized')

        return self.dr_entries.pl_record is not None or self.ce_entries.pl_record is not None

    def parent_link_update_from_dirrecord(self):
        # type: () -> None
        """
        Update the logical extent number stored in the parent link record (if
        there is one), from the directory record entry that was stored in
        the parent_link member.  This is used at the end of reshuffling extents
        to properly update the parent link records.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Rock Ridge extension not initialized')

        if self.parent_link is None:
            raise pycdlibexception.PyCdlibInvalidInput('No parent link found!')

        if self.dr_entries.pl_record is not None:
            self.dr_entries.pl_record.set_log_block_num(self.parent_link.extent_location())
        elif self.ce_entries.pl_record is not None:
            self.ce_entries.pl_record.set_log_block_num(self.parent_link.extent_location())
        else:
            raise pycdlibexception.PyCdlibInvalidInput('Could not find parent link record!')

    def parent_link_extent(self):
        # type: () -> int
        """
        Get the extent of the parent of this entry if it has one.

        Parameters:
         None.
        Returns:
         The logical block number of the parent if it exists.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Rock Ridge extension not initialized')

        if self.dr_entries.pl_record is not None:
            return self.dr_entries.pl_record.parent_log_block_num
        if self.ce_entries.pl_record is not None:
            return self.ce_entries.pl_record.parent_log_block_num

        raise pycdlibexception.PyCdlibInternalError('Asked for parent extent for non-existent parent record')

    def relocated_record(self):
        # type: () -> bool
        """
        Determine whether this Rock Ridge entry has a relocated record (used for
        relocating deep directory records).

        Parameters:
         None.
        Returns:
         True if this Rock Ridge entry has a relocated record, False otherwise.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Rock Ridge extension not initialized')

        return self.dr_entries.re_record is not None or self.ce_entries.re_record is not None

    def update_ce_block(self, block):
        # type: (RockRidgeContinuationBlock) -> None
        """
        Update the Continuation Entry block object used by this Rock Ridge Record.

        Parameters:
         block - The new block object.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Rock Ridge extension not initialized')

        self.ce_block = block


class RockRidgeContinuationEntry:
    """
    A class representing one 'abstract' Rock Ridge Continuation Entry.
    These entries are strictly for keeping tabs of the offset and size
    of each entry in a continuation block; they have no smarts beyond that.
    """
    __slots__ = ('_offset', '_length')

    def __init__(self, offset, length):
        # type: (int, int) -> None
        self._offset = offset
        self._length = length

    @property
    def offset(self):
        # type: () -> int
        """
        Property method to return the offset of this entry.

        Parameters:
         None.
        Returns:
         The offset of this entry.
        """
        return self._offset

    @property
    def length(self):
        # type: () -> int
        """
        Property method to return the length of this entry.

        Parameters:
         None.
        Returns:
         The length of this entry.
        """
        return self._length

    def __lt__(self, other):
        return self._offset < other.offset


class RockRidgeContinuationBlock:
    """
    A class representing one 'abstract' Rock Ridge Continuation Block.
    A Continuation Block is one extent holding many Rock Ridge Continuation
    Entries.  However, this is just used for tracking how many entries will
    fit in one block; all tracking of the actual data must be done elsewhere.
    """
    __slots__ = ('_extent', '_max_block_size', '_entries')

    def __init__(self, extent, max_block_size):
        # type: (int, int) -> None
        self._extent = extent
        self._max_block_size = max_block_size
        self._entries = []  # type: List[RockRidgeContinuationEntry]

    def extent_location(self):
        # type: () -> int
        """
        Get the extent location that this block resides at.

        Parameters:
         None.
        Returns:
         The extent location that this block resides at.
        """
        return self._extent

    def set_extent_location(self, loc):
        # type: (int) -> None
        """
        Set the extent location that this block resides at.

        Parameters:
         loc - The new extent location.
        Returns:
         Nothing.
        """
        self._extent = loc

    def track_entry(self, offset, length):
        # type: (int, int) -> None
        """
        Track an already allocated entry in this Rock Ridge Continuation Block.

        Parameters:
         offset - The offset at which to place the entry.
         length - The length of the entry to track.
        Returns:
         Nothing.
        """
        newlen = offset + length - 1
        for entry in self._entries:
            thislen = entry.offset + entry.length - 1
            overlap = range(max(entry.offset, offset), min(thislen, newlen) + 1)
            if overlap:
                raise pycdlibexception.PyCdlibInvalidISO('Overlapping CE regions on the ISO')

        # OK, there were no overlaps with existing entries.  Let's see if
        # the new entry fits at the end.
        if offset + length > self._max_block_size:
            raise pycdlibexception.PyCdlibInvalidISO('No room in continuation block to track entry')

        # We passed all of the checks; add the new entry to track in.
        bisect.insort_left(self._entries, RockRidgeContinuationEntry(offset, length))

    def add_entry(self, length):
        # type: (int) -> int
        """
        Add a new entry to this Rock Ridge Continuation Block.  This method
        attempts to find a gap that fits the new length anywhere within this
        Continuation Block.  If successful, it returns the offset at which
        it placed this entry.  If unsuccessful, it returns None.

        Parameters:
         length - The length of the entry to find a gap for.
        Returns:
         The offset the entry was placed at, or None if no gap was found.
        """
        offset = -1
        # Need to find a gap
        for index, entry in enumerate(self._entries):
            if index == 0:
                if entry.offset != 0 and length <= entry.offset:
                    # We can put it at the beginning!
                    offset = 0
                    break
            else:
                lastentry = self._entries[index - 1]
                lastend = lastentry.offset + lastentry.length - 1
                gapsize = entry.offset - lastend - 1
                if gapsize >= length:
                    # We found a spot for it!
                    offset = lastend + 1
                    break
        else:
            # We reached the end without finding a gap for it.  Look at the last
            # entry and see if there is room at the end.
            if self._entries:
                lastentry = self._entries[-1]
                lastend = lastentry.offset + lastentry.length - 1
                left = self._max_block_size - lastend - 1
                if left >= length:
                    offset = lastend + 1
            else:
                if self._max_block_size >= length:
                    offset = 0

        if offset >= 0:
            bisect.insort_left(self._entries,
                               RockRidgeContinuationEntry(offset, length))

        return offset

    def remove_entry(self, offset, length):
        # type: (int, int) -> None
        """
        Given an offset and length, find and remove the entry in this block
        that corresponds.

        Parameters:
         offset - The offset of the entry to look for.
         length - The length of the entry to look for.
        Returns:
         Nothing.
        """
        for index, entry in enumerate(self._entries):
            if entry.offset == offset and entry.length == length:
                del self._entries[index]
                break
        else:
            raise pycdlibexception.PyCdlibInternalError('Could not find an entry for the RR CE entry in the CE block!')
