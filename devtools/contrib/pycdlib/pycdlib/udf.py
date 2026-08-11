# Copyright (C) 2018-2022  Chris Lalancette <clalancette@gmail.com>

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

"""Classes to support UDF."""

import io
import logging
import random
import struct
import time

from pycdlib import pycdlibexception
from pycdlib import utils

# For mypy annotations
if False:  # pylint: disable=using-constant-test
    from typing import List, Optional, Tuple, Type, Union  # NOQA pylint: disable=unused-import
    # NOTE: this import has to be here to avoid circular deps
    from pycdlib import inode  # NOQA pylint: disable=unused-import

# This is the CRC CCITT table generated with a polynomial of 0x11021 and
# 16-bits.  The following code will re-generate the table:
#
# def _bytecrc(crc, poly, n):
#    mask = 1<<(n-1)
#    for i in range(8):
#        if crc & mask:
#            crc = (crc << 1) ^ poly
#        else:
#            crc = crc << 1
#    mask = (1<<n) - 1
#    crc = crc & mask
#    return crc
#
# def _mkTable(poly, n):
#    mask = (1<<n) - 1
#    poly = poly & mask
#    table = [_bytecrc(i<<(n-8),poly,n) for i in range(256)]
#    return table

crc_ccitt_table = (0, 4129, 8258, 12387, 16516, 20645, 24774, 28903, 33032,
                   37161, 41290, 45419, 49548, 53677, 57806, 61935, 4657, 528,
                   12915, 8786, 21173, 17044, 29431, 25302, 37689, 33560, 45947,
                   41818, 54205, 50076, 62463, 58334, 9314, 13379, 1056, 5121,
                   25830, 29895, 17572, 21637, 42346, 46411, 34088, 38153,
                   58862, 62927, 50604, 54669, 13907, 9842, 5649, 1584, 30423,
                   26358, 22165, 18100, 46939, 42874, 38681, 34616, 63455, 59390,
                   55197, 51132, 18628, 22757, 26758, 30887, 2112, 6241, 10242,
                   14371, 51660, 55789, 59790, 63919, 35144, 39273, 43274, 47403,
                   23285, 19156, 31415, 27286, 6769, 2640, 14899, 10770, 56317,
                   52188, 64447, 60318, 39801, 35672, 47931, 43802, 27814, 31879,
                   19684, 23749, 11298, 15363, 3168, 7233, 60846, 64911, 52716,
                   56781, 44330, 48395, 36200, 40265, 32407, 28342, 24277, 20212,
                   15891, 11826, 7761, 3696, 65439, 61374, 57309, 53244, 48923,
                   44858, 40793, 36728, 37256, 33193, 45514, 41451, 53516, 49453,
                   61774, 57711, 4224, 161, 12482, 8419, 20484, 16421, 28742,
                   24679, 33721, 37784, 41979, 46042, 49981, 54044, 58239, 62302,
                   689, 4752, 8947, 13010, 16949, 21012, 25207, 29270, 46570,
                   42443, 38312, 34185, 62830, 58703, 54572, 50445, 13538, 9411,
                   5280, 1153, 29798, 25671, 21540, 17413, 42971, 47098, 34713,
                   38840, 59231, 63358, 50973, 55100, 9939, 14066, 1681, 5808,
                   26199, 30326, 17941, 22068, 55628, 51565, 63758, 59695,
                   39368, 35305, 47498, 43435, 22596, 18533, 30726, 26663, 6336,
                   2273, 14466, 10403, 52093, 56156, 60223, 64286, 35833, 39896,
                   43963, 48026, 19061, 23124, 27191, 31254, 2801, 6864, 10931,
                   14994, 64814, 60687, 56684, 52557, 48554, 44427, 40424, 36297,
                   31782, 27655, 23652, 19525, 15522, 11395, 7392, 3265, 61215,
                   65342, 53085, 57212, 44955, 49082, 36825, 40952, 28183, 32310,
                   20053, 24180, 11923, 16050, 3793, 7920)


_logger = logging.getLogger('pycdlib')


def crc_ccitt(data):
    # type: (bytes) -> int
    """
    Calculate the CRC over a range of bytes using the CCITT polynomial.

    Parameters:
     data - The array of bytes to calculate the CRC over.
    Returns:
     The CCITT CRC of the data.
    """
    crc = 0
    for x in data:
        crc = crc_ccitt_table[x ^ ((crc >> 8) & 0xFF)] ^ ((crc << 8) & 0xFF00)

    return crc


def _ostaunicode(src):
    # type: (str) -> bytes
    """Internal function to create an OSTA byte string from a source string."""
    bytename = src

    try:
        enc = bytename.encode('latin-1')
        encbyte = b'\x08'
    except (UnicodeEncodeError, UnicodeDecodeError):
        enc = bytename.encode('utf-16_be')
        encbyte = b'\x10'
    return encbyte + enc


def _ostaunicode_zero_pad(src, fulllen):
    # type: (str, int) -> bytes
    """
    Internal function to create a zero-padded Identifier byte string from a
    source string.

    Parameters:
     src - The src string to start from.
     fulllen - The padded out length of the result.
    Returns:
     A full identifier byte string containing the source string.
    """
    byte_src = _ostaunicode(src)
    return byte_src + b'\x00' * (fulllen - 1 - len(byte_src)) + (struct.pack('=B', len(byte_src)))


class BEAVolumeStructure:
    """
    A class representing a UDF Beginning Extended Area Volume Structure
    (ECMA-167, Part 2, 9.2).
    """
    __slots__ = ('_initialized', 'orig_extent_loc', 'new_extent_loc')

    FMT = '=B5sB2041s'

    def __init__(self):
        # type: () -> None
        self.new_extent_loc = -1
        self._initialized = False

    def parse(self, data, extent):
        # type: (bytes, int) -> None
        """
        Parse the passed in data into a UDF BEA Volume Structure.

        Parameters:
         data - The data to parse.
         extent - The extent that this descriptor currently lives at.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('BEA Volume Structure already initialized')

        (structure_type, standard_ident, structure_version,
         reserved_unused) = struct.unpack_from(self.FMT, data, 0)

        if structure_type != 0:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid structure type')

        if standard_ident != b'BEA01':
            raise pycdlibexception.PyCdlibInvalidISO('Invalid standard identifier')

        if structure_version != 1:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid structure version')

        self.orig_extent_loc = extent

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF BEA Volume Structure.

        Parameters:
         None.
        Returns:
         A string representing this UDF BEA Volume Strucutre.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('BEA Volume Structure not initialized')
        return struct.pack(self.FMT, 0, b'BEA01', 1, b'\x00' * 2041)

    def new(self):
        # type: () -> None
        """
        Create a new UDF BEA Volume Structure.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('BEA Volume Structure already initialized')

        self._initialized = True

    def extent_location(self):
        # type: () -> int
        """
        Get the extent location of this UDF BEA Volume Structure.

        Parameters:
         None.
        Returns:
         Integer extent location of this UDF BEA Volume Structure.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF BEA Volume Structure not initialized')

        if self.new_extent_loc < 0:
            return self.orig_extent_loc
        return self.new_extent_loc

    def set_extent_location(self, extent):
        # type: (int) -> None
        """
        Set the new location for this UDF BEA Volume Structure.

        Parameters:
         extent - The new extent location to set for this UDF BEA Volume Structure.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Volume Descriptor is not initialized')
        self.new_extent_loc = extent


class NSRVolumeStructure:
    """A class representing a UDF NSR Volume Structure (ECMA-167, Part 3, 9.1)."""
    __slots__ = ('_initialized', 'orig_extent_loc', 'new_extent_loc',
                 'standard_ident')

    FMT = '=B5sB2041s'

    def __init__(self):
        # type: () -> None
        self.new_extent_loc = -1
        self._initialized = False

    def parse(self, data, extent):
        # type: (bytes, int) -> None
        """
        Parse the passed in data into a UDF NSR Volume Structure.

        Parameters:
         data - The data to parse.
         extent - The extent that this descriptor currently lives at.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF NSR Volume Structure already initialized')

        (structure_type, self.standard_ident, structure_version,
         reserved_unused) = struct.unpack_from(self.FMT, data, 0)

        if structure_type != 0:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid structure type')

        if self.standard_ident not in (b'NSR02', b'NSR03'):
            raise pycdlibexception.PyCdlibInvalidISO('Invalid standard identifier')

        if structure_version != 1:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid structure version')

        self.orig_extent_loc = extent

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF NSR Volume Structure.

        Parameters:
         None.
        Returns:
         A string representing this UDF BEA Volume Strucutre.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF NSR Volume Structure not initialized')
        return struct.pack(self.FMT, 0, self.standard_ident, 1, b'\x00' * 2041)

    def new(self, version):
        # type: (int) -> None
        """
        Create a new UDF NSR Volume Structure.

        Parameters:
         version - The version of the NSR Volume Structure to create; only 2
                   and 3 are supported.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF NSR Volume Structure already initialized')

        if version == 2:
            self.standard_ident = b'NSR02'
        elif version == 3:
            self.standard_ident = b'NSR03'
        else:
            raise pycdlibexception.PyCdlibInternalError('Invalid NSR version requested')

        self._initialized = True

    def extent_location(self):
        # type: () -> int
        """
        Get the extent location of this UDF NSR Volume Structure.

        Parameters:
         None.
        Returns:
         Integer extent location of this UDF NSR Volume Structure.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF NSR Volume Structure not initialized')

        if self.new_extent_loc < 0:
            return self.orig_extent_loc
        return self.new_extent_loc

    def set_extent_location(self, extent):
        # type: (int) -> None
        """
        Set the new location for this UDF NSR Volume Structure.

        Parameters:
         extent - The new extent location to set for this UDF NSR Volume Structure.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Volume Descriptor is not initialized')
        self.new_extent_loc = extent


class TEAVolumeStructure:
    """
    A class representing a UDF Terminating Extended Area Volume Structure
    (ECMA-167, Part 2, 9.3).
    """
    __slots__ = ('_initialized', 'orig_extent_loc', 'new_extent_loc')

    FMT = '=B5sB2041s'

    def __init__(self):
        # type: () -> None
        self.new_extent_loc = -1
        self._initialized = False

    def parse(self, data, extent):
        # type: (bytes, int) -> None
        """
        Parse the passed in data into a UDF TEA Volume Structure.

        Parameters:
         data - The data to parse.
         extent - The extent that this descriptor currently lives at.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('TEA Volume Structure already initialized')

        (structure_type, standard_ident, structure_version,
         reserved_unused) = struct.unpack_from(self.FMT, data, 0)

        if structure_type != 0:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid structure type')

        if standard_ident != b'TEA01':
            raise pycdlibexception.PyCdlibInvalidISO('Invalid standard identifier')

        if structure_version != 1:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid structure version')

        self.orig_extent_loc = extent

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF TEA Volume Structure.

        Parameters:
         None.
        Returns:
         A string representing this UDF TEA Volume Strucutre.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF TEA Volume Structure not initialized')
        return struct.pack(self.FMT, 0, b'TEA01', 1, b'\x00' * 2041)

    def new(self):
        # type: () -> None
        """
        Create a new UDF TEA Volume Structure.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF TEA Volume Structure already initialized')

        self._initialized = True

    def extent_location(self):
        # type: () -> int
        """
        Get the extent location of this UDF TEA Volume Structure.

        Parameters:
         None.
        Returns:
         Integer extent location of this UDF TEA Volume Structure.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF TEA Volume Structure not initialized')

        if self.new_extent_loc < 0:
            return self.orig_extent_loc
        return self.new_extent_loc

    def set_extent_location(self, extent):
        # type: (int) -> None
        """
        Set the new location for this UDF TEA Volume Structure.

        Parameters:
         extent - The new extent location to set for this UDF TEA Volume Structure.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Volume Descriptor is not initialized')
        self.new_extent_loc = extent


class UDFBootDescriptor:
    """A class representing a UDF Boot Descriptor (ECMA-167, Part 2, 9.4)."""
    __slots__ = ('_initialized', 'architecture_type', 'boot_identifier',
                 'boot_extent_loc', 'boot_extent_len', 'load_address',
                 'start_address', 'desc_creation_time', 'flags', 'boot_use',
                 'orig_extent_loc', 'new_extent_loc')

    FMT = '<B5sBB32s32sLLQQ12sH32s1906s'

    def __init__(self):
        # type: () -> None
        self.new_extent_loc = -1
        self._initialized = False

    def parse(self, data, extent):
        # type: (bytes, int) -> None
        """
        Parse the passed in data into a UDF Boot Descriptor.

        Parameters:
         data - The data to parse.
         extent - The extent that this descriptor currently lives at.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Boot Descriptor already initialized')

        (structure_type, standard_ident, structure_version, reserved1,
         architecture_type, boot_ident, self.boot_extent_loc,
         self.boot_extent_len, self.load_address, self.start_address,
         desc_creation_time, self.flags, reserved2,
         self.boot_use) = struct.unpack_from(self.FMT, data, 0)

        if structure_type != 0:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid structure type')

        if standard_ident != b'BOOT2':
            raise pycdlibexception.PyCdlibInvalidISO('Invalid standard identifier')

        if structure_version != 1:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid structure version')

        if reserved1 != 0:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid reserved1')

        if self.flags > 1:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid flags (must be 0 or 1)')

        if reserved2 != b'\x00' * 32:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid reserved2')

        self.architecture_type = UDFEntityID()
        self.architecture_type.parse(architecture_type)

        self.boot_identifier = UDFEntityID()
        self.boot_identifier.parse(boot_ident)

        self.desc_creation_time = UDFTimestamp()
        self.desc_creation_time.parse(desc_creation_time)

        self.orig_extent_loc = extent

        self._initialized = True

    def new(self):
        # type: () -> None
        """
        Create a new Boot Descriptor.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Boot Descriptor already initialized')

        self.flags = 0  # FIXME: allow the user to set this

        self.architecture_type = UDFEntityID()
        self.architecture_type.new(0)  # FIXME: allow the user to set this

        self.boot_identifier = UDFEntityID()
        self.boot_identifier.new(0)  # FIXME: allow the user to set this

        self.boot_extent_loc = 0  # FIXME: allow the user to set this
        self.boot_extent_len = 0  # FIXME: allow the user to set this
        self.load_address = 0  # FIXME: allow the user to set this
        self.start_address = 0  # FIXME: allow the user to set this

        self.desc_creation_time = UDFTimestamp()
        self.desc_creation_time.new(time.time())

        self.flags = 0  # FIXME: allow the user to set this
        self.boot_use = b'\x00' * 1906  # FIXME: allow the user to set this

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF Boot Descriptor.

        Parameters:
         None.
        Returns:
         A string representing this UDF Boot Descriptor.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Boot Descriptor not initialized')

        return struct.pack(self.FMT, 0, b'BOOT2', 1, 0,
                           self.architecture_type.record(),
                           self.boot_identifier.record(),
                           self.boot_extent_loc, self.boot_extent_len,
                           self.load_address, self.start_address,
                           self.desc_creation_time.record(), self.flags,
                           b'\x00' * 32, self.boot_use)

    def extent_location(self):
        # type: () -> int
        """
        Get the extent location of this UDF Boot Descriptor.

        Parameters:
         None.
        Returns:
         Integer extent location of this UDF Boot Descriptor.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Boot Descriptor not initialized')

        if self.new_extent_loc < 0:
            return self.orig_extent_loc
        return self.new_extent_loc

    def set_extent_location(self, extent):
        # type: (int) -> None
        """
        Set the new location for this UDF Boot Descriptor.

        Parameters:
         extent - The new extent location to set for this UDF Boot Descriptor.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This UDF Boot Descriptor is not initialized')
        self.new_extent_loc = extent


def _compute_csum(data):
    # type: (bytes) -> int
    """
    Compute a simple checksum over the given data.

    Parameters:
     data - The data to compute the checksum over.
    Returns:
     The checksum.
    """
    csum = 0
    for byte in data:
        csum += byte
    csum -= data[4]

    return csum % 256


class UDFTag:
    """A class representing a UDF Descriptor Tag (ECMA-167, Part 3, 7.2)."""
    __slots__ = ('_initialized', 'tag_ident', 'desc_version',
                 'tag_serial_number', 'tag_location', 'desc_crc_length')

    FMT = '<HHBBHHHL'

    def __init__(self):
        # type: () -> None
        self.desc_crc_length = -1
        self._initialized = False

    def parse(self, data, extent):
        # type: (bytes, int) -> None
        """
        Parse the passed in data into a UDF Descriptor tag.

        Parameters:
         data - The data to parse.
         extent - The extent to compare against for the tag location.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Tag already initialized')

        (self.tag_ident, self.desc_version, tag_checksum, reserved,
         self.tag_serial_number, desc_crc, self.desc_crc_length,
         self.tag_location) = struct.unpack_from(self.FMT, data, 0)

        if reserved != 0:
            raise pycdlibexception.PyCdlibInvalidISO('Reserved data not 0!')

        if _compute_csum(data[:16]) != tag_checksum:
            raise pycdlibexception.PyCdlibInvalidISO('Tag checksum does not match!')

        if self.tag_location != extent:
            # In theory, we should abort (throw an exception) if we see that a
            # tag location that doesn't match an actual location.  However, we
            # have seen UDF ISOs in the wild (most notably PS2 GT4 ISOs) that
            # have an invalid tag location for the second anchor and File Set
            # Terminator.  So that we can support those ISOs, just silently
            # fix it up.  We lose a little bit of detection of whether this is
            # "truly" a UDFTag, but it is really not a big risk.
            self.tag_location = extent

        if self.desc_version not in (2, 3):
            raise pycdlibexception.PyCdlibInvalidISO('Tag version not 2 or 3')

        if (len(data) - 16) < self.desc_crc_length:
            raise pycdlibexception.PyCdlibInternalError('Not enough bytes to compute CRC')

        if desc_crc != crc_ccitt(data[16:16 + self.desc_crc_length]):
            raise pycdlibexception.PyCdlibInvalidISO('Tag CRC does not match!')

        self._initialized = True

    def record(self, crc_bytes):
        # type: (bytes) -> bytes
        """
        Generate the string representing this UDF Descriptor Tag.

        Parameters:
         crc_bytes - The string to compute the CRC over.
        Returns:
         A string representing this UDF Descriptor Tag.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Descriptor Tag not initialized')

        crc_byte_len = len(crc_bytes)
        if self.desc_crc_length >= 0:
            crc_byte_len = self.desc_crc_length

        # We need to compute the checksum, but we'll do that by first creating
        # the output buffer with the csum field set to 0, computing the csum,
        # and then setting that record back as usual.
        rec = struct.pack(self.FMT, self.tag_ident, self.desc_version,
                          0, 0, self.tag_serial_number,
                          crc_ccitt(crc_bytes[:crc_byte_len]),
                          crc_byte_len, self.tag_location)
        csum = _compute_csum(rec)

        ba = bytearray(rec)
        ba[4] = csum

        return bytes(ba)

    def new(self, tag_ident, tag_serial=0):
        # type: (int, int) -> None
        """
        Create a new UDF Descriptor Tag.

        Parameters:
         tag_ident - The tag identifier number for this tag.
         tag_serial - The tag serial number for this tag.
        Returns:
         Nothing
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Tag already initialized')

        self.tag_ident = tag_ident
        self.desc_version = 2
        self.tag_serial_number = tag_serial
        self.tag_location = 0  # This will be set later.

        self._initialized = True

    def __eq__(self, other):
        # type: (object) -> bool
        if not isinstance(other, UDFTag):
            return NotImplemented
        return self.tag_ident == other.tag_ident and \
            self.desc_version == other.desc_version and \
            self.tag_serial_number == other.tag_serial_number and \
            self.tag_location == other.tag_location and \
            self.desc_crc_length == other.desc_crc_length


class UDFAnchorVolumeStructure:
    """A class representing a UDF Anchor Volume Structure (ECMA-167, Part 3, 10.2)."""
    __slots__ = ('_initialized', 'orig_extent_loc', 'new_extent_loc',
                 'main_vd', 'reserve_vd', 'desc_tag')

    FMT = '=16s8s8s480s'

    def __init__(self):
        # type: () -> None
        self.new_extent_loc = -1
        self._initialized = False

    def parse(self, data, extent, desc_tag):
        # type: (bytes, int, UDFTag) -> None
        """
        Parse the passed in data into a UDF Anchor Volume Structure.

        Parameters:
         data - The data to parse.
         extent - The extent that this descriptor currently lives at.
         desc_tag - A UDFTag object that represents the Descriptor Tag.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Anchor Volume Structure already initialized')

        (tag_unused, main_vd,
         reserve_vd, reserved_unused) = struct.unpack_from(self.FMT, data, 0)

        self.main_vd = UDFExtentAD()
        self.main_vd.parse(main_vd)

        self.reserve_vd = UDFExtentAD()
        self.reserve_vd.parse(reserve_vd)

        self.desc_tag = desc_tag

        self.orig_extent_loc = extent

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF Anchor Volume Structure.

        Parameters:
         None.
        Returns:
         A string representing this UDF Anchor Volume Structure.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Anchor Volume Descriptor not initialized')

        rec = struct.pack(self.FMT, b'\x00' * 16, self.main_vd.record(),
                          self.reserve_vd.record(), b'\x00' * 480)[16:]

        return self.desc_tag.record(rec) + rec

    def extent_location(self):
        # type: () -> int
        """
        Get the extent location of this UDF Anchor Volume Structure.

        Parameters:
         None.
        Returns:
         Integer extent location of this UDF Anchor Volume Structure.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Anchor Volume Structure not initialized')

        if self.new_extent_loc < 0:
            return self.orig_extent_loc
        return self.new_extent_loc

    def new(self):
        # type: () -> None
        """
        Create a new UDF Anchor Volume Structure.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Anchor Volume Structure already initialized')

        self.desc_tag = UDFTag()
        self.desc_tag.new(2)  # FIXME: let the user set serial_number

        self.main_vd = UDFExtentAD()
        self.main_vd.new(32768, 0)  # The location will get set later.

        self.reserve_vd = UDFExtentAD()
        self.reserve_vd.new(32768, 0)  # The location will get set later.

        self._initialized = True

    def set_extent_location(self, new_location, main_vd_extent, reserve_vd_extent):
        # type: (int, int, int) -> None
        """
        Set a new location for this Anchor Volume Structure.

        Parameters:
         new_location - The new extent that this Anchor Volume Structure should be located at.
         main_vd_extent - The extent containing the main Volume Descriptors.
         reserve_vd_extent - The extent containing the reserve Volume Descriptors.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Anchor Volume Structure not initialized')

        self.new_extent_loc = new_location
        self.desc_tag.tag_location = new_location
        self.main_vd.extent_location = main_vd_extent
        self.reserve_vd.extent_location = reserve_vd_extent

    def __ne__(self, other):
        # type: (object) -> bool
        return not self == other

    def __eq__(self, other):
        # type: (object) -> bool
        if not isinstance(other, UDFAnchorVolumeStructure):
            return NotImplemented
        return self.main_vd.extent_location == other.main_vd.extent_location and self.reserve_vd.extent_location == other.reserve_vd.extent_location


class UDFVolumeDescriptorPointer:
    """A class representing a UDF Volume Descriptor Pointer (ECMA-167, Part 3, 10.3)."""
    __slots__ = ('initialized', 'orig_extent_loc', 'new_extent_loc',
                 'vol_seqnum', 'next_vol_desc_seq_extent', 'desc_tag')

    FMT = '<16sL8s484s'

    def __init__(self):
        # type: () -> None
        self.new_extent_loc = -1
        self.initialized = False

    def parse(self, data, extent, desc_tag):
        # type: (bytes, int, UDFTag) -> None
        """
        Parse the passed in data into a UDF Volume Descriptor Pointer.

        Parameters:
         data - The data to parse.
         extent - The extent that this descriptor currently lives at.
         desc_tag - A UDFTag object that represents the Descriptor Tag.
        Returns:
         Nothing.
        """
        if self.initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Volume Descriptor Pointer already initialized')

        (tag_unused, self.vol_seqnum, next_vol_extent,
         reserved_unused) = struct.unpack_from(self.FMT, data, 0)

        self.next_vol_desc_seq_extent = UDFExtentAD()
        self.next_vol_desc_seq_extent.parse(next_vol_extent)

        self.desc_tag = desc_tag

        self.orig_extent_loc = extent

        self.initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF Volume Descriptor Pointer.

        Parameters:
         None.
        Returns:
         A string representing this UDF Volume Descriptor Pointer.
        """
        if not self.initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Volume Descriptor Pointer not initialized')

        rec = struct.pack(self.FMT, b'\x00' * 16, self.vol_seqnum,
                          self.next_vol_desc_seq_extent.record(), b'\x00' * 484)[16:]

        return self.desc_tag.record(rec) + rec

    def extent_location(self):
        # type: () -> int
        """
        Get the extent location of this UDF Volume Descriptor Pointer.

        Parameters:
         None.
        Returns:
         Integer extent location of this UDF Volume Descriptor Pointer.
        """
        if not self.initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Volume Descriptor Pointer not initialized')

        if self.new_extent_loc < 0:
            return self.orig_extent_loc
        return self.new_extent_loc

    def new(self):
        # type: () -> None
        """
        Create a new UDF Volume Descriptor Pointer.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if self.initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Volume Descriptor Pointer already initialized')

        self.desc_tag = UDFTag()
        self.desc_tag.new(3)  # FIXME: let the user set serial_number

        self.vol_seqnum = 0  # FIXME: let the user set this

        self.next_vol_desc_seq_extent = UDFExtentAD()
        self.next_vol_desc_seq_extent.new(0, 0)  # FIXME: let the user set this

        self.new_extent_loc = 0  # This will be set later

        self.initialized = True

    def set_extent_location(self, new_location):
        # type: (int) -> None
        """
        Set the new location for this UDF Volume Descriptor Pointer.

        Parameters:
         new_location - The new extent this UDF Volume Descriptor Pointer should be located at.
        Returns:
         Nothing.
        """
        if not self.initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Volume Descriptor Pointer not initialized')

        self.new_extent_loc = new_location
        self.desc_tag.tag_location = new_location


class UDFTimestamp:
    """A class representing a UDF timestamp (ECMA-167, Part 1, 7.3)."""
    __slots__ = ('_initialized', 'year', 'month', 'day', 'hour', 'minute',
                 'second', 'centiseconds', 'hundreds_microseconds',
                 'microseconds', 'timetype', 'tz')

    FMT = '<BBHBBBBBBBB'

    def __init__(self):
        # type: () -> None
        self._initialized = False

    def parse(self, data):
        # type: (bytes) -> None
        """
        Parse the passed in data into a UDF Timestamp.

        Parameters:
         data - The data to parse.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Timestamp already initialized')

        (tz, timetype, self.year, self.month, self.day, self.hour, self.minute,
         self.second, self.centiseconds, self.hundreds_microseconds,
         self.microseconds) = struct.unpack_from(self.FMT, data, 0)

        self.timetype = timetype >> 4

        def twos_comp(val, bits):
            # type: (int, int) -> int
            """Compute the 2's complement of int value val"""
            if (val & (1 << (bits - 1))) != 0:  # if sign bit is set e.g., 8bit: 128-255
                val = val - (1 << bits)         # compute negative value
            return val                          # return positive value as is
        self.tz = twos_comp(((timetype & 0xf) << 8) | tz, 12)
        if self.tz < -1440 or self.tz > 1440:
            if self.tz != -2047:
                raise pycdlibexception.PyCdlibInvalidISO('Invalid UDF timezone')

        if self.year < 1 or self.year > 9999:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid UDF year')
        if self.month < 1 or self.month > 12:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid UDF month')
        if self.day < 1 or self.day > 31:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid UDF day')
        if self.hour < 0 or self.hour > 23:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid UDF hour')
        if self.minute < 0 or self.minute > 59:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid UDF minute')
        if self.second < 0 or self.second > 59:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid UDF second')

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF Timestamp.

        Parameters:
         None.
        Returns:
         A string representing this UDF Timestamp.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Timestamp not initialized')

        tmp = ((1 << 16) - 1) & self.tz
        newtz = tmp & 0xff
        newtimetype = ((tmp >> 8) & 0x0f) | (self.timetype << 4)

        return struct.pack(self.FMT, newtz, newtimetype, self.year, self.month,
                           self.day, self.hour, self.minute, self.second,
                           self.centiseconds, self.hundreds_microseconds,
                           self.microseconds)

    def new(self, date_seconds):
        # type: (float) -> None
        """
        Create a new UDF Timestamp.

        Parameters:
         date_seconds - Time and date, in seconds since the epoch, to use for
                        this time stamp record.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Timestamp already initialized')

        local = time.localtime(date_seconds)

        self.tz = utils.gmtoffset_from_tm(date_seconds, local)
        # FIXME: for the timetype, 0 is UTC, 1 is local, 2 is 'agreement'.
        # let the user set this.
        self.timetype = 1
        self.year = local.tm_year
        self.month = local.tm_mon
        self.day = local.tm_mday
        self.hour = local.tm_hour
        self.minute = local.tm_min
        self.second = local.tm_sec
        self.centiseconds = 0
        self.hundreds_microseconds = 0
        self.microseconds = 0

        self._initialized = True

    def __eq__(self, other):
        # type: (object) -> bool
        if not isinstance(other, UDFTimestamp):
            return NotImplemented
        return self.year == other.year and self.month == other.month and \
            self.day == other.day and self.hour == other.hour and \
            self.minute == other.minute and self.second == other.second and \
            self.centiseconds == other.centiseconds and \
            self.hundreds_microseconds == other.hundreds_microseconds and \
            self.microseconds == other.microseconds and \
            self.timetype == other.timetype and self.tz == other.tz


class UDFEntityID:
    """A class representing a UDF Entity ID (ECMA-167, Part 1, 7.4)."""
    __slots__ = ('_initialized', 'flags', 'identifier', 'suffix')

    FMT = '=B23s8s'

    def __init__(self):
        # type: () -> None
        self._initialized = False

    def parse(self, data):
        # type: (bytes) -> None
        """
        Parse the passed in data into a UDF Entity ID.

        Parameters:
         data - The data to parse.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Entity ID already initialized')

        (self.flags, self.identifier, self.suffix) = struct.unpack_from(self.FMT, data, 0)

        if self.flags > 3:
            raise pycdlibexception.PyCdlibInvalidISO('UDF Entity ID flags must be between 0 and 3')

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF Entity ID.

        Parameters:
         None.
        Returns:
         A string representing this UDF Entity ID.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Entity ID not initialized')

        return struct.pack(self.FMT, self.flags, self.identifier, self.suffix)

    def new(self, flags, identifier=b'', suffix=b''):
        # type: (int, bytes, bytes) -> None
        """
        Create a new UDF Entity ID.

        Parameters:
         flags - The flags to set for this Entity ID.
         identifier - The identifier to set for this Entity ID.
         suffix - The suffix to set for this Entity ID.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Entity ID already initialized')

        if flags > 3:
            raise pycdlibexception.PyCdlibInvalidInput('UDF Entity ID flags must be between 0 and 3')

        if len(identifier) > 23:
            raise pycdlibexception.PyCdlibInvalidInput('UDF Entity ID identifier must be less than 23 characters')

        if len(suffix) > 8:
            raise pycdlibexception.PyCdlibInvalidInput('UDF Entity ID suffix must be less than 8 characters')

        self.flags = flags
        self.identifier = identifier + b'\x00' * (23 - len(identifier))
        self.suffix = suffix + b'\x00' * (8 - len(suffix))

        self._initialized = True

    def __eq__(self, other):
        # type: (object) -> bool
        if not isinstance(other, UDFEntityID):
            return NotImplemented
        return self.flags == other.flags and self.identifier == other.identifier and self.suffix == other.suffix


class UDFCharspec:
    """A class representing a UDF charspec (ECMA-167, Part 1, 7.2.1)."""
    __slots__ = ('_initialized', 'set_type', 'set_information')

    FMT = '=B63s'

    def __init__(self):
        # type: () -> None
        self._initialized = False

    def parse(self, data):
        # type: (bytes) -> None
        """
        Parse the passed in data into a UDF Charspec.

        Parameters:
         data - The data to parse.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Charspec already initialized')

        (self.set_type,
         self.set_information) = struct.unpack_from(self.FMT, data, 0)

        if self.set_type > 8:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid charset parsed; only 0-8 supported')

        self._initialized = True

    def new(self, set_type, set_information):
        # type: (int, bytes) -> None
        """
        Create a new UDF Charspc.

        Parameters:
         set_type - The integer set type.  Must be between 0 and 8.
         set_information - Additional set information.  Must be less than 64
                           bytes.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Charspec already initialized')

        if set_type > 8:
            raise pycdlibexception.PyCdlibInvalidInput('Invalid charset specified; only 0-8 supported')

        if len(set_information) > 63:
            raise pycdlibexception.PyCdlibInvalidInput('Invalid charset information; exceeds maximum size of 63')

        self.set_type = set_type

        self.set_information = set_information + b'\x00' * (63 - len(set_information))

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF Charspec.

        Parameters:
         None.
        Returns:
         A string representing this UDF Charspec.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Charspec not initialized')

        return struct.pack(self.FMT, self.set_type, self.set_information)

    def __eq__(self, other):
        # type: (object) -> bool
        if not isinstance(other, UDFCharspec):
            return NotImplemented
        return self.set_type == other.set_type and self.set_information == other.set_information


class UDFExtentAD:
    """A class representing a UDF Extent Descriptor (ECMA-167, Part 3, 7.1)."""
    __slots__ = ('_initialized', 'extent_length', 'extent_location')

    FMT = '<LL'

    def __init__(self):
        # type: () -> None
        self._initialized = False

    def parse(self, data):
        # type: (bytes) -> None
        """
        Parse the passed in data into a UDF Extent AD.

        Parameters:
         data - The data to parse.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Extent descriptor already initialized')
        (self.extent_length,
         self.extent_location) = struct.unpack_from(self.FMT, data, 0)

        if self.extent_length >= 0x3fffffff:
            raise pycdlibexception.PyCdlibInvalidISO('UDF Extent descriptor length must be less than 0x3fffffff')

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF Extent AD.

        Parameters:
         None.
        Returns:
         A string representing this UDF Extent AD.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Extent AD not initialized')

        return struct.pack(self.FMT, self.extent_length, self.extent_location)

    def new(self, length, blocknum):
        # type: (int, int) -> None
        """
        Create a new UDF Short AD.

        Parameters:
         length - The length of the data in the allocation.
         blocknum - The logical block number the allocation starts at.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Extent AD already initialized')

        if length >= 0x3fffffff:
            raise pycdlibexception.PyCdlibInternalError('UDF Extent AD length must be less than 0x3fffffff')

        self.extent_length = length
        self.extent_location = blocknum

        self._initialized = True

    def __eq__(self, other):
        # type: (object) -> bool
        if not isinstance(other, UDFExtentAD):
            return NotImplemented
        return self.extent_length == other.extent_length and self.extent_location == other.extent_location


class UDFPrimaryVolumeDescriptor:
    """A class representing a UDF Primary Volume Descriptor (ECMA-167, Part 3, 10.1)."""
    __slots__ = ('_initialized', 'orig_extent_loc', 'new_extent_loc',
                 'vol_desc_seqnum', 'desc_num', 'vol_ident', 'vol_set_ident',
                 'desc_char_set', 'explanatory_char_set', 'vol_abstract',
                 'vol_copyright', 'implementation_use',
                 'predecessor_vol_desc_location', 'desc_tag', 'recording_date',
                 'app_ident', 'impl_ident', 'max_interchange_level',
                 'interchange_level', 'flags')

    FMT = '<16sLL32sHHHHLL128s64s64s8s8s32s12s32s64sLH22s'

    def __init__(self):
        # type: () -> None
        self.new_extent_loc = -1
        self._initialized = False

    def parse(self, data, extent, desc_tag):
        # type: (bytes, int, UDFTag) -> None
        """
        Parse the passed in data into a UDF Primary Volume Descriptor.

        Parameters:
         data - The data to parse.
         extent - The extent that this descriptor currently lives at.
         desc_tag - A UDFTag object that represents the Descriptor Tag.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Primary Volume Descriptor already initialized')

        (tag_unused, self.vol_desc_seqnum, self.desc_num, self.vol_ident,
         vol_seqnum, max_vol_seqnum, self.interchange_level,
         self.max_interchange_level, char_set_list,
         max_char_set_list, self.vol_set_ident, desc_char_set,
         explanatory_char_set, vol_abstract, vol_copyright, app_ident,
         recording_date, impl_ident, self.implementation_use,
         self.predecessor_vol_desc_location, self.flags,
         reserved) = struct.unpack_from(self.FMT, data, 0)

        self.desc_tag = desc_tag

        if vol_seqnum != 1:
            raise pycdlibexception.PyCdlibInvalidISO('Only DVD Read-Only disks are supported')
        if max_vol_seqnum != 1:
            raise pycdlibexception.PyCdlibInvalidISO('Only DVD Read-Only disks are supported')
        if self.interchange_level not in (2, 3):
            raise pycdlibexception.PyCdlibInvalidISO('Unsupported interchange level (only 2 and 3 supported)')
        if char_set_list != 1:
            raise pycdlibexception.PyCdlibInvalidISO('Only DVD Read-Only disks are supported')
        if max_char_set_list != 1:
            raise pycdlibexception.PyCdlibInvalidISO('Only DVD Read-Only disks are supported')
        if self.flags not in (0, 1):
            raise pycdlibexception.PyCdlibInvalidISO('Invalid UDF flags')

        if reserved != b'\x00' * 22:
            raise pycdlibexception.PyCdlibInvalidISO('UDF Primary Volume Descriptor reserved data not 0')

        self.desc_char_set = UDFCharspec()
        self.desc_char_set.parse(desc_char_set)

        self.explanatory_char_set = UDFCharspec()
        self.explanatory_char_set.parse(explanatory_char_set)

        self.vol_abstract = UDFExtentAD()
        self.vol_abstract.parse(vol_abstract)

        self.vol_copyright = UDFExtentAD()
        self.vol_copyright.parse(vol_copyright)

        self.recording_date = UDFTimestamp()
        self.recording_date.parse(recording_date)

        self.app_ident = UDFEntityID()
        self.app_ident.parse(app_ident)

        self.impl_ident = UDFEntityID()
        self.impl_ident.parse(impl_ident)

        self.orig_extent_loc = extent

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF Primary Volume Descriptor.

        Parameters:
         None.
        Returns:
         A string representing this UDF Primary Volume Descriptor.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Primary Volume Descriptor not initialized')

        rec = struct.pack(self.FMT, b'\x00' * 16,
                          self.vol_desc_seqnum, self.desc_num,
                          self.vol_ident, 1, 1, self.interchange_level,
                          self.max_interchange_level, 1, 1, self.vol_set_ident,
                          self.desc_char_set.record(),
                          self.explanatory_char_set.record(),
                          self.vol_abstract.record(),
                          self.vol_copyright.record(),
                          self.app_ident.record(), self.recording_date.record(),
                          self.impl_ident.record(), self.implementation_use,
                          self.predecessor_vol_desc_location, self.flags,
                          b'\x00' * 22)[16:]
        return self.desc_tag.record(rec) + rec

    def extent_location(self):
        # type: () -> int
        """
        Get the extent location of this UDF Primary Volume Descriptor.

        Parameters:
         None.
        Returns:
         Integer extent location of this UDF Primary Volume Descriptor.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Primary Volume Descriptor not initialized')

        if self.new_extent_loc < 0:
            return self.orig_extent_loc
        return self.new_extent_loc

    def new(self):
        # type: () -> None
        """
        Create a new UDF Primary Volume Descriptor.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Primary Volume Descriptor already initialized')

        self.desc_tag = UDFTag()
        self.desc_tag.new(1)  # FIXME: let the user set serial_number

        self.vol_desc_seqnum = 0  # FIXME: let the user set this
        self.desc_num = 0  # FIXME: let the user set this
        self.vol_ident = _ostaunicode_zero_pad('CDROM', 32)
        # According to UDF 2.60, 2.2.2.5, the VolumeSetIdentifier should have
        # at least the first 16 characters be a unique value.  Further, the
        # first 8 bytes of that should be a time value in ASCII hexadecimal
        # representation.  To make it truly unique, we use that time plus a
        # random value, all ASCII encoded.
        unique = format(int(time.time()), '08x') + format(random.getrandbits(26), '08x')
        self.vol_set_ident = _ostaunicode_zero_pad(unique, 128)
        self.desc_char_set = UDFCharspec()
        self.desc_char_set.new(0, b'OSTA Compressed Unicode')  # FIXME: let the user set this
        self.explanatory_char_set = UDFCharspec()
        self.explanatory_char_set.new(0, b'OSTA Compressed Unicode')  # FIXME: let the user set this
        self.vol_abstract = UDFExtentAD()
        self.vol_abstract.new(0, 0)    # FIXME: let the user set this
        self.vol_copyright = UDFExtentAD()
        self.vol_copyright.new(0, 0)  # FIXME: let the user set this
        self.app_ident = UDFEntityID()
        self.app_ident.new(0)
        self.recording_date = UDFTimestamp()
        self.recording_date.new(time.time())
        self.impl_ident = UDFEntityID()
        self.impl_ident.new(0, b'*pycdlib')
        self.implementation_use = b'\x00' * 64  # FIXME: let the user set this
        self.predecessor_vol_desc_location = 0  # FIXME: let the user set this
        self.interchange_level = 2
        self.max_interchange_level = 2
        self.flags = 0

        self._initialized = True

    def set_extent_location(self, new_location):
        # type: (int) -> None
        """
        Set the new location for this UDF Primary Volume Descriptor.

        Parameters:
         new_location - The extent that this Primary Volume Descriptor should be located at.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Primary Volume Descriptor not initialized')

        self.new_extent_loc = new_location
        self.desc_tag.tag_location = new_location

    def __eq__(self, other):
        # type: (object) -> bool
        if not isinstance(other, UDFPrimaryVolumeDescriptor):
            return NotImplemented
        return self.vol_desc_seqnum == other.vol_desc_seqnum and \
            self.desc_num == other.desc_num and \
            self.vol_ident == other.vol_ident and \
            self.vol_set_ident == other.vol_set_ident and \
            self.desc_char_set == other.desc_char_set and \
            self.explanatory_char_set == other.explanatory_char_set and \
            self.vol_abstract == other.vol_abstract and \
            self.vol_copyright == other.vol_copyright and \
            self.implementation_use == other.implementation_use and \
            self.predecessor_vol_desc_location == other.predecessor_vol_desc_location and \
            self.desc_tag == other.desc_tag and \
            self.recording_date == other.recording_date and \
            self.app_ident == other.app_ident and \
            self.impl_ident == other.impl_ident and \
            self.max_interchange_level == other.max_interchange_level and \
            self.interchange_level == other.interchange_level and \
            self.flags == other.flags


class UDFImplementationUseVolumeDescriptorImplementationUse:
    """
    A class representing the Implementation Use field of the Implementation Use
    Volume Descriptor.
    """
    __slots__ = ('_initialized', 'char_set', 'log_vol_ident', 'lv_info1',
                 'lv_info2', 'lv_info3', 'impl_ident', 'impl_use')

    FMT = '=64s128s36s36s36s32s128s'

    def __init__(self):
        # type: () -> None
        self._initialized = False

    def parse(self, data):
        # type: (bytes) -> None
        """
        Parse the passed in data into a UDF Implementation Use Volume
        Descriptor Implementation Use field.

        Parameters:
         data - The data to parse.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Implementation Use Volume Descriptor Implementation Use field already initialized')

        (char_set, self.log_vol_ident, self.lv_info1, self.lv_info2,
         self.lv_info3, impl_ident,
         self.impl_use) = struct.unpack_from(self.FMT, data, 0)

        self.char_set = UDFCharspec()
        self.char_set.parse(char_set)

        self.impl_ident = UDFEntityID()
        self.impl_ident.parse(impl_ident)

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF Implementation Use Volume
        Descriptor Implementation Use field.

        Parameters:
         None.
        Returns:
         A string representing this UDF Implementation Use Volume Descriptor.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Implementation Use Volume Descriptor Implementation Use field not initialized')

        return struct.pack(self.FMT, self.char_set.record(), self.log_vol_ident,
                           self.lv_info1, self.lv_info2, self.lv_info3,
                           self.impl_ident.record(), self.impl_use)

    def new(self):
        # type: () -> None
        """
        Create a new UDF Implementation Use Volume Descriptor Implementation Use
        field.

        Parameters:
         None:
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Implementation Use Volume Descriptor Implementation Use field already initialized')

        self.char_set = UDFCharspec()
        self.char_set.new(0, b'OSTA Compressed Unicode')  # FIXME: let the user set this
        self.log_vol_ident = _ostaunicode_zero_pad('CDROM', 128)
        self.lv_info1 = b'\x00' * 36
        self.lv_info2 = b'\x00' * 36
        self.lv_info3 = b'\x00' * 36
        self.impl_ident = UDFEntityID()
        self.impl_ident.new(0, b'*pycdlib', b'')
        self.impl_use = b'\x00' * 128

        self._initialized = True

    def __eq__(self, other):
        # type: (object) -> bool
        if not isinstance(other, UDFImplementationUseVolumeDescriptorImplementationUse):
            return NotImplemented
        return self.char_set == other.char_set and \
            self.log_vol_ident == other.log_vol_ident and \
            self.lv_info1 == other.lv_info1 and \
            self.lv_info2 == other.lv_info2 and \
            self.lv_info3 == other.lv_info3 and \
            self.impl_ident == other.impl_ident and \
            self.impl_use == other.impl_use


class UDFImplementationUseVolumeDescriptor:
    """A class representing a UDF Implementation Use Volume Structure (ECMA-167, Part 3, 10.4)."""
    __slots__ = ('_initialized', 'orig_extent_loc', 'new_extent_loc',
                 'vol_desc_seqnum', 'impl_use', 'desc_tag', 'impl_ident')

    FMT = '<16sL32s460s'

    def __init__(self):
        # type: () -> None
        self.new_extent_loc = -1
        self._initialized = False

    def parse(self, data, extent, desc_tag):
        # type: (bytes, int, UDFTag) -> None
        """
        Parse the passed in data into a UDF Implementation Use Volume
        Descriptor.

        Parameters:
         data - The data to parse.
         extent - The extent that this descriptor currently lives at.
         desc_tag - A UDFTag object that represents the Descriptor Tag.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Implementation Use Volume Descriptor already initialized')

        (tag_unused, self.vol_desc_seqnum, impl_ident,
         impl_use) = struct.unpack_from(self.FMT, data, 0)

        self.desc_tag = desc_tag

        self.impl_ident = UDFEntityID()
        self.impl_ident.parse(impl_ident)
        if self.impl_ident.identifier[:12] != b'*UDF LV Info':
            raise pycdlibexception.PyCdlibInvalidISO("Implementation Use Identifier not '*UDF LV Info'")

        self.impl_use = UDFImplementationUseVolumeDescriptorImplementationUse()
        self.impl_use.parse(impl_use)

        self.orig_extent_loc = extent

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF Implementation Use Volume
        Descriptor.

        Parameters:
         None.
        Returns:
         A string representing this UDF Implementation Use Volume Descriptor.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Implementation Use Volume Descriptor not initialized')

        rec = struct.pack(self.FMT, b'\x00' * 16,
                          self.vol_desc_seqnum, self.impl_ident.record(),
                          self.impl_use.record())[16:]
        return self.desc_tag.record(rec) + rec

    def extent_location(self):
        # type: () -> int
        """
        Get the extent location of this UDF Implementation Use Volume
        Descriptor.

        Parameters:
         None.
        Returns:
         Integer extent location of this UDF Implementation Use Volume
         Descriptor.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Implementation Use Volume Descriptor not initialized')

        if self.new_extent_loc < 0:
            return self.orig_extent_loc
        return self.new_extent_loc

    def new(self):
        # type: () -> None
        """
        Create a new UDF Implementation Use Volume Descriptor.

        Parameters:
         None:
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Implementation Use Volume Descriptor already initialized')

        self.desc_tag = UDFTag()
        self.desc_tag.new(4)  # FIXME: let the user set serial_number

        self.vol_desc_seqnum = 1

        self.impl_ident = UDFEntityID()
        self.impl_ident.new(0, b'*UDF LV Info', b'\x02\x01')

        self.impl_use = UDFImplementationUseVolumeDescriptorImplementationUse()
        self.impl_use.new()

        self._initialized = True

    def set_extent_location(self, new_location):
        # type: (int) -> None
        """
        Set the new location for this UDF Implementation Use Volume Descriptor.

        Parameters:
         new_location - The new extent this UDF Implementation Use Volume Descriptor should be located at.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Implementation Use Volume Descriptor not initialized')

        self.new_extent_loc = new_location
        self.desc_tag.tag_location = new_location

    def __eq__(self, other):
        # type: (object) -> bool
        if not isinstance(other, UDFImplementationUseVolumeDescriptor):
            return NotImplemented
        return self.vol_desc_seqnum == other.vol_desc_seqnum and \
            self.impl_use == other.impl_use and \
            self.desc_tag == other.desc_tag and \
            self.impl_ident == other.impl_ident


class UDFPartitionHeaderDescriptor:
    """A class representing a UDF Partition Header Descriptor."""
    __slots__ = ('_initialized', 'unalloc_space_table', 'unalloc_space_bitmap',
                 'partition_integrity_table', 'freed_space_table',
                 'freed_space_bitmap')

    FMT = '=8s8s8s8s8s88s'

    def __init__(self):
        # type: () -> None
        self._initialized = False

    def parse(self, data):
        # type: (bytes) -> None
        """
        Parse the passed in data into a UDF Partition Header Descriptor.

        Parameters:
         data - The data to parse.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Partition Header Descriptor already initialized')

        (unalloc_space_table, unalloc_space_bitmap, partition_integrity_table,
         freed_space_table, freed_space_bitmap,
         reserved_unused) = struct.unpack_from(self.FMT, data, 0)

        self.unalloc_space_table = UDFShortAD()
        self.unalloc_space_table.parse(unalloc_space_table)

        self.unalloc_space_bitmap = UDFShortAD()
        self.unalloc_space_bitmap.parse(unalloc_space_bitmap)

        self.partition_integrity_table = UDFShortAD()
        self.partition_integrity_table.parse(partition_integrity_table)

        self.freed_space_table = UDFShortAD()
        self.freed_space_table.parse(freed_space_table)

        self.freed_space_bitmap = UDFShortAD()
        self.freed_space_bitmap.parse(freed_space_bitmap)

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF Partition Header Descriptor.

        Parameters:
         None.
        Returns:
         A string representing this UDF Partition Header Descriptor.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Partition Header Descriptor not initialized')

        return struct.pack(self.FMT, self.unalloc_space_table.record(),
                           self.unalloc_space_bitmap.record(),
                           self.partition_integrity_table.record(),
                           self.freed_space_table.record(),
                           self.freed_space_bitmap.record(),
                           b'\x00' * 88)

    def new(self):
        # type: () -> None
        """
        Create a new UDF Partition Header Descriptor.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Partition Header Descriptor already initialized')

        self.unalloc_space_table = UDFShortAD()
        self.unalloc_space_table.new(0)

        self.unalloc_space_bitmap = UDFShortAD()
        self.unalloc_space_bitmap.new(0)

        self.partition_integrity_table = UDFShortAD()
        self.partition_integrity_table.new(0)

        self.freed_space_table = UDFShortAD()
        self.freed_space_table.new(0)

        self.freed_space_bitmap = UDFShortAD()
        self.freed_space_bitmap.new(0)

        self._initialized = True

    def __eq__(self, other):
        # type: (object) -> bool
        if not isinstance(other, UDFPartitionHeaderDescriptor):
            return NotImplemented
        return self.unalloc_space_table == other.unalloc_space_table and \
            self.unalloc_space_bitmap == other.unalloc_space_bitmap and \
            self.partition_integrity_table == other.partition_integrity_table and \
            self.freed_space_table == other.freed_space_table and \
            self.freed_space_bitmap == other.freed_space_bitmap


class UDFPartitionVolumeDescriptor:
    """A class representing a UDF Partition Volume Structure (ECMA-167, Part 3, 10.5)."""
    __slots__ = ('_initialized', 'orig_extent_loc', 'new_extent_loc',
                 'vol_desc_seqnum', 'part_flags', 'part_num', 'access_type',
                 'part_start_location', 'part_length', 'implementation_use',
                 'desc_tag', 'part_contents', 'impl_ident', 'part_contents_use')

    FMT = '<16sLHH32s128sLLL32s128s156s'

    def __init__(self):
        # type: () -> None
        self.new_extent_loc = -1
        self._initialized = False

    def parse(self, data, extent, desc_tag):
        # type: (bytes, int, UDFTag) -> None
        """
        Parse the passed in data into a UDF Partition Volume Descriptor.

        Parameters:
         data - The data to parse.
         extent - The extent that this descriptor currently lives at.
         desc_tag - A UDFTag object that represents the Descriptor Tag.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Partition Volume Descriptor already initialized')

        (tag_unused, self.vol_desc_seqnum, self.part_flags, self.part_num,
         part_contents, part_contents_use, self.access_type,
         self.part_start_location, self.part_length, impl_ident,
         self.implementation_use, reserved_unused) = struct.unpack_from(self.FMT, data, 0)

        if self.part_flags not in (0, 1):
            raise pycdlibexception.PyCdlibInvalidISO('Invalid partition flags')

        self.desc_tag = desc_tag

        self.part_contents = UDFEntityID()
        self.part_contents.parse(part_contents)
        if self.part_contents.identifier[:6] not in (b'+FDC01', b'+CD001', b'+CDW02', b'+NSR02', b'+NSR03'):
            raise pycdlibexception.PyCdlibInvalidISO("Partition Contents Identifier not '+FDC01', '+CD001', '+CDW02', '+NSR02', or '+NSR03'")

        if self.access_type > 0x1f:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid UDF partition access type')

        self.part_contents_use = UDFPartitionHeaderDescriptor()
        self.part_contents_use.parse(part_contents_use)

        self.impl_ident = UDFEntityID()
        self.impl_ident.parse(impl_ident)

        self.orig_extent_loc = extent

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF Partition Volume Descriptor.

        Parameters:
         None.
        Returns:
         A string representing this UDF Partition Volume Descriptor.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Partition Volume Descriptor not initialized')

        rec = struct.pack(self.FMT, b'\x00' * 16,
                          self.vol_desc_seqnum, self.part_flags,
                          self.part_num, self.part_contents.record(),
                          self.part_contents_use.record(), self.access_type,
                          self.part_start_location, self.part_length,
                          self.impl_ident.record(), self.implementation_use,
                          b'\x00' * 156)[16:]
        return self.desc_tag.record(rec) + rec

    def extent_location(self):
        # type: () -> int
        """
        Get the extent location of this UDF Partition Volume Descriptor.

        Parameters:
         None.
        Returns:
         Integer extent location of this UDF Partition Volume Descriptor.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Partition Volume Descriptor not initialized')

        if self.new_extent_loc < 0:
            return self.orig_extent_loc
        return self.new_extent_loc

    def new(self, version):
        # type: (int) -> None
        """
        Create a new UDF Partition Volume Descriptor.

        Parameters:
         version - The version of to make this partition; must be 2 or 3.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Partition Volume Descriptor already initialized')

        self.desc_tag = UDFTag()
        self.desc_tag.new(5)  # FIXME: let the user set serial_number

        self.vol_desc_seqnum = 2
        self.part_flags = 1  # FIXME: how should we set this?
        self.part_num = 0  # FIXME: how should we set this?

        self.part_contents = UDFEntityID()
        if version == 2:
            self.part_contents.new(2, b'+NSR02')
        elif version == 3:
            self.part_contents.new(2, b'+NSR03')
        else:
            raise pycdlibexception.PyCdlibInternalError('Invalid NSR version requested')

        self.part_contents_use = UDFPartitionHeaderDescriptor()
        self.part_contents_use.new()

        self.access_type = 1
        self.part_start_location = 0  # This will get set later
        self.part_length = 3  # This will get set later

        self.impl_ident = UDFEntityID()
        self.impl_ident.new(0, b'*pycdlib')

        self.implementation_use = b'\x00' * 128  # FIXME: let the user set this

        self._initialized = True

    def set_extent_location(self, new_location):
        # type: (int) -> None
        """
        Set the location of this UDF Partition Volume Descriptor.

        Parameters:
         new_location - The new extent this UDF Partition Volume Descriptor should be located at.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Partition Volume Descriptor not initialized')
        self.new_extent_loc = new_location
        self.desc_tag.tag_location = new_location

    def set_start_location(self, new_location):
        # type: (int) -> None
        """
        Set the location of the start of the UDF partition.

        Parameters:
         new_location - The new extent the UDF partition should start at.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Partition Volume Descriptor not initialized')
        self.part_start_location = new_location

    def __eq__(self, other):
        # type: (object) -> bool
        if not isinstance(other, UDFPartitionVolumeDescriptor):
            return NotImplemented
        return self.vol_desc_seqnum == other.vol_desc_seqnum and \
            self.part_flags == other.part_flags and \
            self.part_num == other.part_num and \
            self.access_type == other.access_type and \
            self.part_start_location == other.part_start_location and \
            self.part_length == other.part_length and \
            self.implementation_use == other.implementation_use and \
            self.desc_tag == other.desc_tag and \
            self.part_contents == other.part_contents and \
            self.impl_ident == other.impl_ident and \
            self.part_contents_use == other.part_contents_use


class UDFType0PartitionMap:
    """A class representing a UDF Type 0 Partition Map (ECMA-167, Part 3, 10.7)."""
    __slots__ = ('_initialized', 'data')

    FMT = '=BB'

    def __init__(self):
        # type: () -> None
        self._initialized = False

    def parse(self, data):
        # type: (bytes) -> None
        """
        Parse the passed in data into a UDF Type 0 Partition Map.

        Parameters:
         data - The data to parse.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Type 0 Partition Map already initialized')

        (map_type, map_length) = struct.unpack_from(self.FMT, data, 0)

        if map_type != 0:
            raise pycdlibexception.PyCdlibInvalidISO('UDF Type 0 Partition Map type is not 0')

        if map_length != len(data):
            raise pycdlibexception.PyCdlibInvalidISO('UDF Type 0 Partition Map length does not equal data length')

        self.data = data[2:]

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF Type 0 Partition Map.

        Parameters:
         None.
        Returns:
         A string representing this UDF Type 0 Partition Map.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Type 0 Partition Map not initialized')

        return struct.pack(self.FMT, 0, 2 + len(self.data)) + self.data

    def new(self):
        # type: () -> None
        """
        Create a new UDF Type 0 Partition Map.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Type 0 Partition Map already initialized')

        self.data = b''  # FIXME: let the user set this

        self._initialized = True


class UDFType1PartitionMap:
    """A class representing a UDF Type 1 Partition Map (ECMA-167, Part 3, 10.7)."""
    __slots__ = ('_initialized', 'part_num', 'vol_seqnum')

    FMT = '<BBHH'

    def __init__(self):
        # type: () -> None
        self._initialized = False

    def parse(self, data):
        # type: (bytes) -> None
        """
        Parse the passed in data into a UDF Type 1 Partition Map.

        Parameters:
         data - The data to parse.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Type 1 Partition Map already initialized')

        (map_type, map_length, self.vol_seqnum,
         self.part_num) = struct.unpack_from(self.FMT, data, 0)

        if map_type != 1:
            raise pycdlibexception.PyCdlibInvalidISO('UDF Type 1 Partition Map type is not 1')
        if map_length != 6:
            raise pycdlibexception.PyCdlibInvalidISO('UDF Type 1 Partition Map length is not 6')

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF Type 1 Partition Map.

        Parameters:
         None.
        Returns:
         A string representing this UDF Type 1 Partition Map.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Type 1 Partition Map not initialized')

        return struct.pack(self.FMT, 1, 6, self.vol_seqnum, self.part_num)

    def new(self):
        # type: () -> None
        """
        Create a new UDF Type 1 Partition Map.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Type 1 Partition Map already initialized')

        self.part_num = 0  # FIXME: let the user set this
        self.vol_seqnum = 1  # FIXME: let the user set this

        self._initialized = True


class UDFType2PartitionMap:
    """A class representing a UDF Type 2 Partition Map (ECMA-167, Part 3, 10.7)."""
    __slots__ = ('_initialized', 'part_ident')

    FMT = '=BB62s'

    def __init__(self):
        # type: () -> None
        self._initialized = False

    def parse(self, data):
        # type: (bytes) -> None
        """
        Parse the passed in data into a UDF Type 2 Partition Map.

        Parameters:
         data - The data to parse.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Type 2 Partition Map already initialized')

        (map_type, map_length, self.part_ident) = struct.unpack_from(self.FMT, data, 0)

        if map_type != 2:
            raise pycdlibexception.PyCdlibInvalidISO('UDF Type 2 Partition Map type is not 2')
        if map_length != 64:
            raise pycdlibexception.PyCdlibInvalidISO('UDF Type 2 Partition Map length is not 64')

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF Type 2 Partition Map.

        Parameters:
         None.
        Returns:
         A string representing this UDF Type 2 Partition Map.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Type 2 Partition Map not initialized')

        return struct.pack(self.FMT, 2, 64, self.part_ident)

    def new(self):
        # type: () -> None
        """
        Create a new UDF Partition Map.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Type 2 Partition Map already initialized')

        self.part_ident = b'\x00' * 62  # FIXME: let the user set this

        self._initialized = True


class UDFExtendedAD:
    """A class representing a UDF Extended Allocation Descriptor (ECMA-167, Part 4, 14.14.3)."""
    __slots__ = ('_initialized', 'extent_length', 'recorded_length',
                 'information_length', 'extent_location', 'impl_use')

    FMT = '<LLL6s2s'

    def __init__(self):
        # type: () -> None
        self._initialized = False

    def parse(self, data):
        # type: (bytes) -> None
        """
        Parsed the passed in data into a UDF Extended Allocation Descriptor.

        Parameters:
         data - The data to parse.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Extended Allocation descriptor already initialized')

        (self.extent_length, self.recorded_length, self.information_length,
         extent_location, self.impl_use) = struct.unpack_from(self.FMT, data, 0)

        self.extent_location = UDFLBAddr()
        self.extent_location.parse(extent_location)

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF Extended Allocation Descriptor.

        Parameters:
         None.
        Returns:
         A string representing this UDF Extended Allocation Descriptor.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Extended Allocation Descriptor not initialized')

        return struct.pack(self.FMT, self.extent_length, self.recorded_length,
                           self.information_length,
                           self.extent_location.record(), self.impl_use)

    def new(self):
        # type: () -> None
        """
        Create a new UDF Extended AD.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Extended Allocation Descriptor already initialized')

        self.extent_length = 0  # FIXME: let the user set this
        self.recorded_length = 0  # FIXME: let the user set this
        self.information_length = 0  # FIXME: let the user set this
        self.extent_location = UDFLBAddr()
        self.extent_location.new(0)
        self.impl_use = b'\x00\x00'

        self._initialized = True


class UDFShortAD:
    """A class representing a UDF Short Allocation Descriptor (ECMA-167, Part 4, 14.14.1)."""
    __slots__ = ('_initialized', 'extent_length', 'log_block_num', 'offset',
                 'extent_type')

    FMT = '<LL'

    def __init__(self):
        # type: () -> None
        self.offset = 0
        self._initialized = False

    def parse(self, data):
        # type: (bytes) -> None
        """
        Parse the passed in data into a UDF Short AD.

        Parameters:
         data - The data to parse.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Short Allocation descriptor already initialized')

        (self.extent_length,
         self.log_block_num) = struct.unpack_from(self.FMT, data, 0)

        self.extent_length = self.extent_length & 0x3FFFFFFF
        self.extent_type = (self.extent_length & 0xc0000000) >> 30

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF Short AD.

        Parameters:
         None.
        Returns:
         A string representing this UDF Short AD.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Short AD not initialized')

        length = self.extent_length | (self.extent_type << 30)
        return struct.pack(self.FMT, length, self.log_block_num)

    def new(self, length):
        # type: (int) -> None
        """
        Create a new UDF Short AD.

        Parameters:
         length - The length of the data in the allocation.
         blocknum - The logical block number the allocation starts at.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Short AD already initialized')

        if length > 0x3fffffff:
            raise pycdlibexception.PyCdlibInternalError('UDF Short AD length must be less than or equal to 0x3fffffff')

        self.extent_length = length
        self.extent_type = 0  # FIXME: let the user set this
        self.log_block_num = 0  # this will get set later

        self._initialized = True

    def set_extent_location(self, new_location, tag_location):  # pylint: disable=unused-argument
        # type: (int, int) -> None
        """
        Set the location fields of this UDF Short AD.

        Parameters:
         new_location - The new relative extent that this UDF Short AD references.
         tag_location - The new absolute extent that this UDF Short AD references.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Short AD not initialized')

        self.log_block_num = tag_location

    def length(self):
        # type: () -> int
        """
        Method to return the length of the UDF Short Allocation Descriptor.

        Parameters:
         None.
        Returns:
         The length of this descriptor in bytes.
        """
        return 8

    def __eq__(self, other):
        # type: (object) -> bool
        if not isinstance(other, UDFShortAD):
            return NotImplemented
        return self.extent_length == other.extent_length and self.log_block_num == other.log_block_num


class UDFLongAD:
    """
    A class representing a UDF Long Allocation Descriptor (ECMA-167, Part 4,
    14.14.2).
    """
    __slots__ = ('_initialized', 'extent_length', 'log_block_num',
                 'part_ref_num', 'impl_use', 'offset')

    FMT = '<LLH6s'

    def __init__(self):
        # type: () -> None
        self.offset = 0
        self._initialized = False

    def parse(self, data):
        # type: (bytes) -> None
        """
        Parse the passed in data into a UDF Long AD.

        Parameters:
         data - The data to parse.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Long Allocation descriptor already initialized')
        (self.extent_length, self.log_block_num, self.part_ref_num,
         self.impl_use) = struct.unpack_from(self.FMT, data, 0)

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF Long AD.

        Parameters:
         None.
        Returns:
         A string representing this UDF Long AD.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Long AD not initialized')

        return struct.pack(self.FMT, self.extent_length, self.log_block_num,
                           self.part_ref_num, self.impl_use)

    def new(self, length, blocknum):
        # type: (int, int) -> None
        """
        Create a new UDF Long AD.

        Parameters:
         length - The length of the data in the allocation.
         blocknum - The logical block number the allocation starts at.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Long AD already initialized')

        self.extent_length = length
        self.log_block_num = blocknum
        self.part_ref_num = 0  # FIXME: let the user set this
        self.impl_use = b'\x00' * 6

        self._initialized = True

    def set_extent_location(self, new_location, tag_location):
        # type: (int, int) -> None
        """
        Set the location fields of this UDF Long AD.

        Parameters:
         new_location - The new relative extent that this UDF Long AD references.
         tag_location - The new absolute extent that this UDF Long AD references.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Long AD not initialized')

        self.log_block_num = tag_location
        self.impl_use = b'\x00\x00' + struct.pack('<L', new_location)

    def length(self):
        # type: () -> int
        """
        Method to return the length of the UDF Long Allocation Descriptor.

        Parameters:
         None.
        Returns:
         The length of this descriptor in bytes.
        """
        return 16

    def __eq__(self, other):
        # type: (object) -> bool
        if not isinstance(other, UDFLongAD):
            return NotImplemented
        return self.extent_length == other.extent_length and \
            self.log_block_num == other.log_block_num and \
            self.part_ref_num == other.part_ref_num and \
            self.impl_use == other.impl_use


class UDFInlineAD:
    """
    A class representing a UDF Inline Allocation Descriptor.  This isn't
    explicitly defined in the specification, but is a convenient structure
    to use for ICBTag flags type 3 Allocation Descriptors.
    """
    __slots__ = ('_initialized', 'extent_length', 'log_block_num', 'offset')

    def __init__(self):
        # type: () -> None
        self._initialized = False

    def parse(self, extent_length, log_block_num, offset):
        # type: (int, int, int) -> None
        """
        Create a new UDF Inline AD from the given data.

        Parameters:
         data - The data to parse.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Inline Allocation Descriptor already initialized')
        self.extent_length = extent_length
        self.log_block_num = log_block_num
        self.offset = offset

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF Inline AD.

        Parameters:
         None.
        Returns:
         A string representing this UDF Inline AD.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Inline AD not initialized')

        return b''

    def new(self, extent_length, log_block_num, offset):
        # type: (int, int, int) -> None
        """
        Create a new UDF Inline AD.

        Parameters:
         extent_length - The length of the data in the allocation.
         log_block_num - The logical block number the allocation starts at.
         offset - The offset the allocation starts at.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Inline AD already initialized')

        self.extent_length = extent_length
        self.log_block_num = log_block_num
        self.offset = offset

        self._initialized = True

    def set_extent_location(self, new_location, tag_location):  # pylint: disable=unused-argument
        # type: (int, int) -> None
        """
        Set the location fields of this UDF Inline AD.

        Parameters:
         new_location - The new relative extent that this UDF Inline AD references.
         tag_location - The new absolute extent that this UDF Inline AD references.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Inline AD not initialized')

        self.log_block_num = tag_location

    def length(self):
        # type: () -> int
        """
        Method to return the length of the UDF Inline Allocation Descriptor.

        Parameters:
         None.
        Returns:
         The length of this descriptor in bytes.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Inline AD not initialized')
        return self.extent_length


class UDFLogicalVolumeDescriptor:
    """A class representing a UDF Logical Volume Descriptor (ECMA-167, Part 3, 10.6)."""
    __slots__ = ('_initialized', 'orig_extent_loc', 'new_extent_loc',
                 'vol_desc_seqnum', 'desc_char_set', 'logical_vol_ident',
                 'implementation_use', 'integrity_sequence', 'desc_tag',
                 'domain_ident', 'impl_ident', 'partition_maps',
                 'logical_volume_contents_use')

    FMT = '<16sL64s128sL32s16sLL32s128s8s72s'

    def __init__(self):
        # type: () -> None
        self.new_extent_loc = -1
        self.partition_maps = []  # type: List[Union[UDFType0PartitionMap, UDFType1PartitionMap, UDFType2PartitionMap]]
        self._initialized = False

    def parse(self, data, extent, desc_tag):
        # type: (bytes, int, UDFTag) -> None
        """
        Parse the passed in data into a UDF Logical Volume Descriptor.

        Parameters:
         data - The data to parse.
         extent - The extent that this descriptor currently lives at.
         desc_tag - A UDFTag object that represents the Descriptor Tag.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Logical Volume Descriptor already initialized')

        (tag_unused, self.vol_desc_seqnum, desc_char_set,
         self.logical_vol_ident, logical_block_size, domain_ident,
         logical_volume_contents_use, map_table_length, num_partition_maps,
         impl_ident, self.implementation_use, integrity_sequence,
         partition_maps) = struct.unpack_from(self.FMT, data, 0)

        self.desc_tag = desc_tag

        self.desc_char_set = UDFCharspec()
        self.desc_char_set.parse(desc_char_set)

        if logical_block_size != 2048:
            raise pycdlibexception.PyCdlibInvalidISO('Volume Descriptor block size is not 2048')

        self.domain_ident = UDFEntityID()
        self.domain_ident.parse(domain_ident)
        if self.domain_ident.identifier[:19] != b'*OSTA UDF Compliant':
            raise pycdlibexception.PyCdlibInvalidISO("Volume Descriptor Identifier not '*OSTA UDF Compliant'")

        if map_table_length >= len(partition_maps):
            raise pycdlibexception.PyCdlibInvalidISO('Map table length greater than size of partition map data; ISO corrupt')

        self.impl_ident = UDFEntityID()
        self.impl_ident.parse(impl_ident)

        self.integrity_sequence = UDFExtentAD()
        self.integrity_sequence.parse(integrity_sequence)

        offset = 0
        map_table_length_left = map_table_length
        for p_unused in range(0, num_partition_maps):
            # The generic partition map starts with 1 byte for the type and
            # 1 byte for the length.
            (map_type, map_len) = struct.unpack_from('=BB', partition_maps, offset)
            if map_len > len(partition_maps[offset:]):
                raise pycdlibexception.PyCdlibInvalidISO('Partition map goes beyond end of data, ISO corrupt')
            if map_len > map_table_length_left:
                raise pycdlibexception.PyCdlibInvalidISO('Partition map goes beyond map_table_length left, ISO corrupt')

            if map_type == 0:
                partmap0 = UDFType0PartitionMap()
                partmap0.parse(partition_maps[offset:offset + map_len])
                self.partition_maps.append(partmap0)
            elif map_type == 1:
                partmap1 = UDFType1PartitionMap()
                partmap1.parse(partition_maps[offset:offset + map_len])
                self.partition_maps.append(partmap1)
            elif map_type == 2:
                partmap2 = UDFType2PartitionMap()
                partmap2.parse(partition_maps[offset:offset + map_len])
                self.partition_maps.append(partmap2)
            else:
                raise pycdlibexception.PyCdlibInvalidISO('Unsupported partition map type')

            offset += map_len
            map_table_length_left -= map_len

        self.logical_volume_contents_use = UDFLongAD()
        self.logical_volume_contents_use.parse(logical_volume_contents_use)

        self.orig_extent_loc = extent

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF Logical Volume Descriptor.

        Parameters:
         None.
        Returns:
         A string representing this UDF Logical Volume Descriptor.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Logical Volume Descriptor not initialized')

        all_partmaps = b''
        for part in self.partition_maps:
            all_partmaps += part.record()

        partmap_pad = io.BytesIO()
        utils.zero_pad(partmap_pad, len(all_partmaps), 72)

        rec = struct.pack(self.FMT, b'\x00' * 16,
                          self.vol_desc_seqnum, self.desc_char_set.record(),
                          self.logical_vol_ident, 2048,
                          self.domain_ident.record(),
                          self.logical_volume_contents_use.record(),
                          len(all_partmaps), len(self.partition_maps),
                          self.impl_ident.record(), self.implementation_use,
                          self.integrity_sequence.record(),
                          all_partmaps + partmap_pad.getvalue())[16:]
        return self.desc_tag.record(rec) + rec

    def extent_location(self):
        # type: () -> int
        """
        Get the extent location of this UDF Logical Volume
        Descriptor.

        Parameters:
         None.
        Returns:
         Integer extent location of this UDF Logical Volume Descriptor.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Logical Volume Descriptor not initialized')

        if self.new_extent_loc < 0:
            return self.orig_extent_loc
        return self.new_extent_loc

    def new(self):
        # type: () -> None
        """
        Create a new UDF Logical Volume Descriptor.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Logical Volume Descriptor already initialized')

        self.desc_tag = UDFTag()
        self.desc_tag.new(6)  # FIXME: let the user set serial_number

        self.vol_desc_seqnum = 3
        self.desc_char_set = UDFCharspec()
        self.desc_char_set.new(0, b'OSTA Compressed Unicode')  # FIXME: let the user set this

        self.logical_vol_ident = _ostaunicode_zero_pad('CDROM', 128)

        self.domain_ident = UDFEntityID()
        self.domain_ident.new(0, b'*OSTA UDF Compliant', b'\x02\x01\x03')

        self.logical_volume_contents_use = UDFLongAD()
        self.logical_volume_contents_use.new(4096, 0)

        self.impl_ident = UDFEntityID()
        self.impl_ident.new(0, b'*pycdlib')

        self.implementation_use = b'\x00' * 128  # FIXME: let the user set this

        self.integrity_sequence = UDFExtentAD()
        self.integrity_sequence.new(4096, 0)  # The location will get set later.

        self._initialized = True

    def add_partition_map(self, partmaptype):
        # type: (int) -> None
        """
        Add a new partition map to this UDF Logical Volume Descriptor.

        Parameters:
         partmaptype - Must be 0, 1, or 2.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Logical Volume Descriptor not initialized')

        partmap = None  # type: Optional[Union[UDFType0PartitionMap, UDFType1PartitionMap, UDFType2PartitionMap]]

        if partmaptype == 0:
            partmap = UDFType0PartitionMap()
        elif partmaptype == 1:
            partmap = UDFType1PartitionMap()
        elif partmaptype == 2:
            partmap = UDFType2PartitionMap()
        else:
            raise pycdlibexception.PyCdlibInternalError('UDF Partition map type must be 0, 1, or 2')
        partmap.new()

        all_partmaps = b''
        for part in self.partition_maps:
            all_partmaps += part.record()

        if len(all_partmaps) > 72:
            raise pycdlibexception.PyCdlibInternalError('Too many UDF partition maps')

        self.partition_maps.append(partmap)

    def set_extent_location(self, new_location):
        # type: (int) -> None
        """
        Set the location of this UDF Logical Volume Descriptor.

        Parameters:
         new_location - The new extent this UDF Logical Volume Descriptor should be located at.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Logical Volume Descriptor not initialized')

        self.new_extent_loc = new_location
        self.desc_tag.tag_location = new_location

    def set_integrity_location(self, integrity_extent):
        # type: (int) -> None
        """
        Set the location of the UDF Integrity sequence that this descriptor
        references.

        Parameters:
         integrity_extent - The new extent that the UDF Integrity sequence
                            should start at.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Logical Volume Descriptor not initialized')

        self.integrity_sequence.extent_location = integrity_extent

    def __eq__(self, other):
        # type: (object) -> bool
        if not isinstance(other, UDFLogicalVolumeDescriptor):
            return NotImplemented
        return self.vol_desc_seqnum == other.vol_desc_seqnum and \
            self.desc_char_set == other.desc_char_set and \
            self.logical_vol_ident == other.logical_vol_ident and \
            self.implementation_use == other.implementation_use and \
            self.integrity_sequence == other.integrity_sequence and \
            self.desc_tag == other.desc_tag and \
            self.domain_ident == other.domain_ident and \
            self.impl_ident == other.impl_ident and \
            self.logical_volume_contents_use == other.logical_volume_contents_use


class UDFUnallocatedSpaceDescriptor:
    """A class representing a UDF Unallocated Space Descriptor (ECMA-167, Part 3, 10.8)."""
    __slots__ = ('_initialized', 'orig_extent_loc', 'new_extent_loc',
                 'vol_desc_seqnum', 'desc_tag', 'num_alloc_descriptors',
                 'alloc_descs')

    FMT = '<16sLL488s'

    def __init__(self):
        # type: () -> None
        self.new_extent_loc = -1
        self.alloc_descs = []  # type: List[UDFExtentAD]
        self._initialized = False

    def parse(self, data, extent, desc_tag):
        # type: (bytes, int, UDFTag) -> None
        """
        Parse the passed in data into a UDF Unallocated Space Descriptor.

        Parameters:
         data - The data to parse.
         extent - The extent that this descriptor currently lives at.
         desc_tag - A UDFTag object that represents the Descriptor Tag.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Unallocated Space Descriptor already initialized')

        (tag_unused, self.vol_desc_seqnum,
         self.num_alloc_descriptors,
         alloc_descs) = struct.unpack_from(self.FMT, data, 0)

        self.desc_tag = desc_tag

        if self.num_alloc_descriptors * 8 > len(alloc_descs):
            raise pycdlibexception.PyCdlibInvalidISO('Too many allocation descriptors')

        for num in range(0, self.num_alloc_descriptors):
            offset = num * 8
            extent_ad = UDFExtentAD()
            extent_ad.parse(alloc_descs[offset:offset + 8])
            self.alloc_descs.append(extent_ad)

        self.orig_extent_loc = extent

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF Unallocated Space Descriptor.

        Parameters:
         None.
        Returns:
         A string representing this UDF Unallocated Space Descriptor.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Unallocated Space Descriptor not initialized')

        alloc_desc_bytes = b''
        for desc in self.alloc_descs:
            alloc_desc_bytes += desc.record()
        alloc_desc_bytes += b'\x00' * (488 - len(alloc_desc_bytes))

        rec = struct.pack(self.FMT, b'\x00' * 16,
                          self.vol_desc_seqnum, self.num_alloc_descriptors,
                          alloc_desc_bytes)[16:]
        return self.desc_tag.record(rec) + rec

    def extent_location(self):
        # type: () -> int
        """
        Get the extent location of this UDF Unallocated Space Descriptor.

        Parameters:
         None.
        Returns:
         Integer extent location of this UDF Unallocated Space Descriptor.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Unallocated Space Descriptor not initialized')

        if self.new_extent_loc < 0:
            return self.orig_extent_loc
        return self.new_extent_loc

    def new(self):
        # type: () -> None
        """
        Create a new UDF Unallocated Space Descriptor.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Unallocated Space Descriptor already initialized')

        self.desc_tag = UDFTag()
        self.desc_tag.new(7)  # FIXME: let the user set serial_number

        self.vol_desc_seqnum = 4

        self.num_alloc_descriptors = 0

        self._initialized = True

    def set_extent_location(self, new_location):
        # type: (int) -> None
        """
        Set the location of this UDF Unallocated Space Descriptor.

        Parameters:
         new_location - The new extent this UDF Unallocated Space Descriptor should be located at.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Unallocated Space Descriptor not initialized')

        self.new_extent_loc = new_location
        self.desc_tag.tag_location = new_location

    def __eq__(self, other):
        # type: (object) -> bool
        if not isinstance(other, UDFUnallocatedSpaceDescriptor):
            return NotImplemented
        return self.vol_desc_seqnum == other.vol_desc_seqnum and \
            self.desc_tag == other.desc_tag and \
            self.num_alloc_descriptors == other.num_alloc_descriptors


class UDFTerminatingDescriptor:
    """A class representing a UDF Terminating Descriptor (ECMA-167, Part 3, 10.9)."""
    __slots__ = ('initialized', 'orig_extent_loc', 'new_extent_loc',
                 'desc_tag')

    FMT = '=16s496s'

    def __init__(self):
        # type: () -> None
        self.new_extent_loc = -1
        self.initialized = False

    def parse(self, extent, desc_tag):
        # type: (int, UDFTag) -> None
        """
        Parse the passed in data into a UDF Terminating Descriptor.

        Parameters:
         extent - The extent that this descriptor currently lives at.
         desc_tag - A UDFTag object that represents the Descriptor Tag.
        Returns:
         Nothing.
        """
        if self.initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Terminating Descriptor already initialized')

        self.desc_tag = desc_tag

        self.orig_extent_loc = extent

        self.initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF Terminating Descriptor.

        Parameters:
         None.
        Returns:
         A string representing this UDF Terminating Descriptor.
        """
        if not self.initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Terminating Descriptor not initialized')

        rec = struct.pack(self.FMT, b'\x00' * 16, b'\x00' * 496)[16:]
        return self.desc_tag.record(rec) + rec

    def extent_location(self):
        # type: () -> int
        """
        Get the extent location of this UDF Terminating Descriptor.

        Parameters:
         None.
        Returns:
         Integer extent location of this UDF Terminating Descriptor.
        """
        if not self.initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Terminating Descriptor not initialized')

        if self.new_extent_loc < 0:
            return self.orig_extent_loc
        return self.new_extent_loc

    def new(self):
        # type: () -> None
        """
        Create a new UDF Terminating Descriptor.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if self.initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Terminating Descriptor already initialized')

        self.desc_tag = UDFTag()
        self.desc_tag.new(8)  # FIXME: let the user set serial_number

        self.initialized = True

    def set_extent_location(self, new_location, tag_location=-1):
        # type: (int, int) -> None
        """
        Set the location of this UDF Terminating Descriptor.

        Parameters:
         new_location - The new extent this UDF Terminating Descriptor should be located at.
         tag_location - The tag location to set for this UDF Terminator Descriptor.
        Returns:
         Nothing.
        """
        if not self.initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Terminating Descriptor not initialized')

        self.new_extent_loc = new_location
        if tag_location < 0:
            tag_location = new_location
        self.desc_tag.tag_location = tag_location


class UDFLogicalVolumeHeaderDescriptor:
    """A class representing a UDF Logical Volume Header Descriptor (ECMA-167, Part 4, 14.15)."""
    __slots__ = ('_initialized', 'unique_id')

    FMT = '<Q24s'

    def __init__(self):
        # type: () -> None
        self._initialized = False

    def parse(self, data):
        # type: (bytes) -> None
        """
        Parse the passed in data into a UDF Logical Volume Header Descriptor.

        Parameters:
         data - The data to parse.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Logical Volume Header Descriptor already initialized')
        (self.unique_id, reserved_unused) = struct.unpack_from(self.FMT, data, 0)

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF Logical Volume Header
        Descriptor.

        Parameters:
         None.
        Returns:
         A string representing this UDF Logical Volume Header Descriptor.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Logical Volume Header Descriptor not initialized')

        return struct.pack(self.FMT, self.unique_id, b'\x00' * 24)

    def new(self):
        # type: () -> None
        """
        Create a new UDF Logical Volume Header Descriptor.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Logical Volume Header Descriptor already initialized')

        self.unique_id = 261

        self._initialized = True


class UDFLogicalVolumeImplementationUse:
    """A class representing a UDF Logical Volume Implementation Use."""
    __slots__ = ('_initialized', 'num_files', 'num_dirs',
                 'min_udf_read_revision', 'min_udf_write_revision',
                 'max_udf_write_revision', 'impl_id', 'impl_use')

    FMT = '<32sLLHHH'

    def __init__(self):
        # type: () -> None
        self._initialized = False

    def parse(self, data):
        # type: (bytes) -> None
        """
        Parse the passed in data into a UDF Logical Volume Implementation Use.

        Parameters:
         data - The data to parse.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Logical Volume Implementation Use already initialized')

        (impl_id, self.num_files, self.num_dirs, self.min_udf_read_revision,
         self.min_udf_write_revision,
         self.max_udf_write_revision) = struct.unpack_from(self.FMT, data, 0)

        self.impl_id = UDFEntityID()
        self.impl_id.parse(impl_id)

        self.impl_use = data[46:]

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF Logical Volume Implementation
        Use.

        Parameters:
         None.
        Returns:
         A string representing this UDF Logical Volume Implementation Use.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Logical Volume Implementation Use not initialized')

        return struct.pack(self.FMT, self.impl_id.record(),
                           self.num_files, self.num_dirs,
                           self.min_udf_read_revision,
                           self.min_udf_write_revision,
                           self.max_udf_write_revision) + self.impl_use

    def new(self):
        # type: () -> None
        """
        Create a new UDF Logical Volume Implementation Use.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Logical Volume Implementation Use already initialized')

        self.impl_id = UDFEntityID()
        self.impl_id.new(0, b'*pycdlib')

        self.num_files = 0
        self.num_dirs = 1
        self.min_udf_read_revision = 258
        self.min_udf_write_revision = 258
        self.max_udf_write_revision = 258

        self.impl_use = b'\x00' * 378  # FIXME: let the user set this

        self._initialized = True


class UDFLogicalVolumeIntegrityDescriptor:
    """A class representing a UDF Logical Volume Integrity Descriptor (ECMA-167, Part 3, 10.10)."""
    __slots__ = ('_initialized', 'orig_extent_loc', 'new_extent_loc',
                 'length_impl_use', 'free_space_tables', 'size_tables',
                 'desc_tag', 'recording_date', 'logical_volume_contents_use',
                 'logical_volume_impl_use', 'next_integrity_extent',
                 'integrity_type', 'num_partitions')

    FMT = '<16s12sL8s32sLL432s'

    def __init__(self):
        # type: () -> None
        self.new_extent_loc = -1
        self.free_space_tables = []  # type: List[int]
        self.size_tables = []  # type: List[int]
        self._initialized = False

    def parse(self, data, extent, desc_tag):
        # type: (bytes, int, UDFTag) -> None
        """
        Parse the passed in data into a UDF Logical Volume Integrity Descriptor.

        Parameters:
         data - The data to parse.
         extent - The extent that this descriptor currently lives at.
         desc_tag - A UDFTag object that represents the Descriptor Tag.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Logical Volume Integrity Descriptor already initialized')

        (tag_unused, recording_date, self.integrity_type,
         next_integrity_extent, logical_volume_contents_use, self.num_partitions,
         self.length_impl_use, end) = struct.unpack_from(self.FMT, data, 0)

        self.desc_tag = desc_tag

        self.recording_date = UDFTimestamp()
        self.recording_date.parse(recording_date)

        if self.integrity_type not in (0, 1):
            raise pycdlibexception.PyCdlibInvalidISO('Logical Volume Integrity Type not 0 or 1')

        self.next_integrity_extent = UDFExtentAD()
        self.next_integrity_extent.parse(next_integrity_extent)

        self.logical_volume_contents_use = UDFLogicalVolumeHeaderDescriptor()
        self.logical_volume_contents_use.parse(logical_volume_contents_use)

        end_offset = 0
        for part_unused in range(0, self.num_partitions):
            free_space, = struct.unpack_from('<L', end[:end_offset + 4], end_offset)
            self.free_space_tables.append(free_space)
            end_offset += 4
        for part_unused in range(0, self.num_partitions):
            size, = struct.unpack_from('<L', end[:end_offset + 4], end_offset)
            self.size_tables.append(size)
            end_offset += 4

        if len(end[end_offset:]) < self.length_impl_use:
            raise pycdlibexception.PyCdlibInvalidISO('UDF Logical Volume Integrity specified an implementation use that is too large')

        self.logical_volume_impl_use = UDFLogicalVolumeImplementationUse()
        self.logical_volume_impl_use.parse(end[end_offset:])

        self.orig_extent_loc = extent

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF Logical Volume Integrity
        Descriptor.

        Parameters:
         None.
        Returns:
         A string representing this UDF Logical Volume Integrity Descriptor.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Logical Volume Integrity Descriptor not initialized')

        end = b''
        for table in self.free_space_tables:
            end += struct.pack('<L', table)
        for table in self.size_tables:
            end += struct.pack('<L', table)
        end += self.logical_volume_impl_use.record()

        rec = struct.pack(self.FMT, b'\x00' * 16,
                          self.recording_date.record(), self.integrity_type,
                          self.next_integrity_extent.record(),
                          self.logical_volume_contents_use.record(),
                          self.num_partitions,
                          self.length_impl_use, end)[16:]
        return self.desc_tag.record(rec) + rec

    def extent_location(self):
        # type: () -> int
        """
        Get the extent location of this UDF Logical Volume Integrity Descriptor.

        Parameters:
         None.
        Returns:
         Integer extent location of this UDF Logical Volume Integrity Descriptor.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Logical Volume Integrity Descriptor not initialized')

        if self.new_extent_loc < 0:
            return self.orig_extent_loc
        return self.new_extent_loc

    def new(self):
        # type: () -> None
        """
        Create a new UDF Logical Volume Integrity Descriptor.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Logical Volume Integrity Descriptor already initialized')

        self.desc_tag = UDFTag()
        self.desc_tag.new(9)  # FIXME: let the user set serial_number

        self.recording_date = UDFTimestamp()
        self.recording_date.new(time.time())

        self.integrity_type = 1  # FIXME: let the user set this

        self.length_impl_use = 46
        self.free_space_tables = [0]  # FIXME: let the user set this
        self.size_tables = [3]  # FIXME: let the user set this
        self.num_partitions = 1  # FIXME: let the user set this

        self.next_integrity_extent = UDFExtentAD()
        self.next_integrity_extent.new(0, 0)  # FIXME: let the user set this

        self.logical_volume_contents_use = UDFLogicalVolumeHeaderDescriptor()
        self.logical_volume_contents_use.new()

        self.logical_volume_impl_use = UDFLogicalVolumeImplementationUse()
        self.logical_volume_impl_use.new()

        self._initialized = True

    def set_extent_location(self, new_location):
        # type: (int) -> None
        """
        Set the location of this UDF Logical Volume Integrity Descriptor.

        Parameters:
         new_location - The new extent this UDF Logical Volume Integrity Descriptor should be located at.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Logical Volume Integrity Descriptor not initialized')

        self.new_extent_loc = new_location
        self.desc_tag.tag_location = new_location


class UDFFileSetDescriptor:
    """A class representing a UDF File Set Descriptor (ECMA-167, Part 4, 14.1)."""
    __slots__ = ('_initialized', 'orig_extent_loc', 'new_extent_loc',
                 'file_set_num', 'log_vol_char_set', 'log_vol_ident',
                 'file_set_char_set', 'file_set_ident', 'copyright_file_ident',
                 'abstract_file_ident', 'desc_tag', 'recording_date',
                 'domain_ident', 'root_dir_icb', 'next_extent',
                 'system_stream_dir_icb')

    FMT = '<16s12sHHLLLL64s128s64s32s32s32s16s32s16s16s32s'

    def __init__(self):
        # type: () -> None
        self.new_extent_loc = -1
        self._initialized = False

    def parse(self, data, extent, desc_tag):
        # type: (bytes, int, UDFTag) -> None
        """
        Parse the passed in data into a UDF File Set Descriptor.

        Parameters:
         data - The data to parse.
         extent - The extent that this descriptor currently lives at.
         desc_tag - A UDFTag object that represents the Descriptor Tag.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF File Set Descriptor already initialized')

        (tag_unused, recording_date, interchange_level, max_interchange_level,
         char_set_list, max_char_set_list, self.file_set_num, file_set_desc_num,
         log_vol_char_set, self.log_vol_ident, file_set_char_set,
         self.file_set_ident, self.copyright_file_ident,
         self.abstract_file_ident, root_dir_icb, domain_ident, next_extent,
         system_stream_dir_icb, reserved_unused) = struct.unpack_from(self.FMT, data, 0)

        self.desc_tag = desc_tag

        self.recording_date = UDFTimestamp()
        self.recording_date.parse(recording_date)

        if interchange_level != 3:
            raise pycdlibexception.PyCdlibInvalidISO('Only DVD Read-Only disks are supported')
        if max_interchange_level != 3:
            raise pycdlibexception.PyCdlibInvalidISO('Only DVD Read-Only disks are supported')
        if char_set_list != 1:
            raise pycdlibexception.PyCdlibInvalidISO('Only DVD Read-Only disks are supported')
        if max_char_set_list != 1:
            raise pycdlibexception.PyCdlibInvalidISO('Only DVD Read-Only disks are supported')
        if file_set_desc_num != 0:
            raise pycdlibexception.PyCdlibInvalidISO('Only DVD Read-Only disks are supported')

        self.log_vol_char_set = UDFCharspec()
        self.log_vol_char_set.parse(log_vol_char_set)

        self.file_set_char_set = UDFCharspec()
        self.file_set_char_set.parse(file_set_char_set)

        self.domain_ident = UDFEntityID()
        self.domain_ident.parse(domain_ident)
        if self.domain_ident.identifier[:19] != b'*OSTA UDF Compliant':
            raise pycdlibexception.PyCdlibInvalidISO("File Set Descriptor Identifier not '*OSTA UDF Compliant'")

        self.root_dir_icb = UDFLongAD()
        self.root_dir_icb.parse(root_dir_icb)

        self.next_extent = UDFLongAD()
        self.next_extent.parse(next_extent)

        self.system_stream_dir_icb = UDFLongAD()
        self.system_stream_dir_icb.parse(system_stream_dir_icb)

        self.orig_extent_loc = extent

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF File Set Descriptor.

        Parameters:
         None.
        Returns:
         A string representing this UDF File Set Descriptor.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF File Set Descriptor not initialized')

        rec = struct.pack(self.FMT, b'\x00' * 16,
                          self.recording_date.record(), 3, 3, 1, 1,
                          self.file_set_num, 0, self.log_vol_char_set.record(),
                          self.log_vol_ident, self.file_set_char_set.record(),
                          self.file_set_ident, self.copyright_file_ident,
                          self.abstract_file_ident, self.root_dir_icb.record(),
                          self.domain_ident.record(), self.next_extent.record(),
                          self.system_stream_dir_icb.record(), b'\x00' * 32)[16:]
        return self.desc_tag.record(rec) + rec

    def extent_location(self):
        # type: () -> int
        """
        Get the extent location of this UDF File Set Descriptor.

        Parameters:
         None.
        Returns:
         Integer extent location of this UDF File Set Descriptor.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF File Set Descriptor not initialized')

        if self.new_extent_loc < 0:
            return self.orig_extent_loc
        return self.new_extent_loc

    def new(self):
        # type: () -> None
        """
        Create a new UDF File Set Descriptor.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF File Set Descriptor already initialized')

        self.desc_tag = UDFTag()
        self.desc_tag.new(256)  # FIXME: let the user set serial_number

        self.recording_date = UDFTimestamp()
        self.recording_date.new(time.time())

        self.domain_ident = UDFEntityID()
        self.domain_ident.new(0, b'*OSTA UDF Compliant', b'\x02\x01\x03')

        self.root_dir_icb = UDFLongAD()
        self.root_dir_icb.new(2048, 2)

        self.file_set_num = 0
        self.log_vol_char_set = UDFCharspec()
        self.log_vol_char_set.new(0, b'OSTA Compressed Unicode')  # FIXME: let the user set this
        self.log_vol_ident = _ostaunicode_zero_pad('CDROM', 128)
        self.file_set_char_set = UDFCharspec()
        self.file_set_char_set.new(0, b'OSTA Compressed Unicode')  # FIXME: let the user set this
        self.file_set_ident = _ostaunicode_zero_pad('CDROM', 32)
        self.copyright_file_ident = b'\x00' * 32  # FIXME: let the user set this
        self.abstract_file_ident = b'\x00' * 32  # FIXME: let the user set this

        self.next_extent = UDFLongAD()
        self.next_extent.new(0, 0)

        self.system_stream_dir_icb = UDFLongAD()
        self.system_stream_dir_icb.new(0, 0)

        self._initialized = True

    def set_extent_location(self, new_location):
        # type: (int) -> None
        """
        Set the location of this UDF File Set Descriptor.

        Parameters:
         new_location - The new extent this UDF File Set Descriptor should be located at.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF File Set Descriptor not initialized')

        self.new_extent_loc = new_location


class UDFLBAddr:
    """A class reprenting a UDF lb_addr (ECMA-167, Part 4, 7.1)."""
    __slots__ = ('_initialized', 'logical_block_num', 'part_ref_num')

    FMT = '<LH'

    def __init__(self):
        # type: () -> None
        self._initialized = False

    def parse(self, data):
        # type: (bytes) -> None
        """
        Parse the passed in data into a UDF lb_addr.

        Parameters:
         data - The data to parse
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF LBAddr already initialized')

        (self.logical_block_num,
         self.part_ref_num) = struct.unpack_from(self.FMT, data, 0)

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF LBAddr.

        Parameters:
         None.
        Returns:
         A string representing this UDF LBAddr.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF LBAddr not initialized')
        return struct.pack(self.FMT, self.logical_block_num, self.part_ref_num)

    def new(self, logical_block_num):
        # type: (int) -> None
        """
        Create a new UDF LBAddr.

        Parameters:
         logical_block_num - The logical block number to assign.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF LBAddr already initialized')

        self.logical_block_num = logical_block_num
        self.part_ref_num = 0

        self._initialized = True


class UDFICBTag:
    """A class representing a UDF ICB Tag (ECMA-167, Part 4, 14.6)."""
    __slots__ = ('_initialized', 'prior_num_direct_entries', 'strategy_type',
                 'strategy_param', 'max_num_entries', 'file_type', 'parent_icb',
                 'flags')

    FMT = '<LHHHBB6sH'

    def __init__(self):
        # type: () -> None
        self._initialized = False

    def parse(self, data):
        # type: (bytes) -> None
        """
        Parse the passed in data into a UDF ICB Tag.

        Parameters:
         data - The data to parse.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF ICB Tag already initialized')

        (self.prior_num_direct_entries, self.strategy_type, self.strategy_param,
         self.max_num_entries, reserved, self.file_type,
         parent_icb, self.flags) = struct.unpack_from(self.FMT, data, 0)

        if self.strategy_type not in (4, 4096):
            raise pycdlibexception.PyCdlibInvalidISO('UDF ICB Tag invalid strategy type')

        if reserved != 0:
            raise pycdlibexception.PyCdlibInvalidISO('UDF ICB Tag reserved not 0')

        self.parent_icb = UDFLBAddr()
        self.parent_icb.parse(parent_icb)

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF ICB Tag.

        Parameters:
         None.
        Returns:
         A string representing this UDF ICB Tag.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF ICB Tag not initialized')

        return struct.pack(self.FMT, self.prior_num_direct_entries,
                           self.strategy_type, self.strategy_param,
                           self.max_num_entries, 0, self.file_type,
                           self.parent_icb.record(), self.flags)

    def new(self, file_type):
        # type: (str) -> None
        """
        Create a new UDF ICB Tag.

        Parameters:
         file_type - What file type this represents, one of 'dir', 'file',
                     or 'symlink'.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF ICB Tag already initialized')

        self.prior_num_direct_entries = 0  # FIXME: let the user set this
        self.strategy_type = 4
        self.strategy_param = 0  # FIXME: let the user set this
        self.max_num_entries = 1
        if file_type == 'dir':
            self.file_type = 4
        elif file_type == 'file':
            self.file_type = 5
        elif file_type == 'symlink':
            self.file_type = 12
        else:
            raise pycdlibexception.PyCdlibInternalError("Invalid file type for ICB; must be one of 'dir', 'file', or 'symlink'")

        self.parent_icb = UDFLBAddr()
        self.parent_icb.new(0)
        self.flags = 560  # hex 0x230 == binary 0010 0011 0000

        self._initialized = True


class UDFFileEntry:
    """A class representing a UDF File Entry (ECMA-167, Part 4, 14.9)."""
    __slots__ = ('_initialized', 'orig_extent_loc', 'new_extent_loc', 'uid',
                 'gid', 'perms', 'file_link_count', 'info_len', 'hidden',
                 'log_block_recorded', 'unique_id', 'len_extended_attrs',
                 'desc_tag', 'icb_tag', 'alloc_descs', 'fi_descs', 'parent',
                 'access_time', 'mod_time', 'attr_time', 'extended_attr_icb',
                 'impl_ident', 'extended_attrs', 'file_ident', 'inode')

    FMT = '<16s20sLLLHBBLQQ12s12s12sL16s32sQLL'

    def __init__(self):
        # type: () -> None
        self.alloc_descs = []  # type: List[Union[UDFShortAD, UDFLongAD, UDFInlineAD]]
        self.fi_descs = []  # type: List[UDFFileIdentifierDescriptor]
        self._initialized = False
        self.parent = None  # type: Optional[UDFFileEntry]
        self.hidden = False
        self.file_ident = None  # type: Optional[UDFFileIdentifierDescriptor]
        self.inode = None  # type: Optional[inode.Inode]
        self.new_extent_loc = -1

    def parse(self, data, extent, parent, desc_tag):
        # type: (bytes, int, Optional[UDFFileEntry], UDFTag) -> None
        """
        Parse the passed in data into a UDF File Entry.

        Parameters:
         data - The data to parse.
         extent - The extent that this descriptor currently lives at.
         parent - The parent File Entry for this file (may be None).
         desc_tag - A UDFTag object that represents the Descriptor Tag.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF File Entry already initialized')

        (tag_unused, icb_tag, self.uid, self.gid, self.perms, self.file_link_count,
         record_format, record_display_attrs, record_len, self.info_len,
         self.log_block_recorded, access_time, mod_time, attr_time, checkpoint,
         extended_attr_icb, impl_ident, self.unique_id, self.len_extended_attrs,
         len_alloc_descs) = struct.unpack_from(self.FMT, data, 0)

        self.desc_tag = desc_tag

        self.icb_tag = UDFICBTag()
        self.icb_tag.parse(icb_tag)

        if record_format != 0:
            raise pycdlibexception.PyCdlibInvalidISO('File Entry record format is not 0')

        if record_display_attrs != 0:
            raise pycdlibexception.PyCdlibInvalidISO('File Entry record display attributes is not 0')

        if record_len != 0:
            raise pycdlibexception.PyCdlibInvalidISO('File Entry record length is not 0')

        self.access_time = UDFTimestamp()
        self.access_time.parse(access_time)

        self.mod_time = UDFTimestamp()
        self.mod_time.parse(mod_time)

        self.attr_time = UDFTimestamp()
        self.attr_time.parse(attr_time)

        if checkpoint != 1:
            raise pycdlibexception.PyCdlibInvalidISO('Only DVD Read-only disks supported')

        self.extended_attr_icb = UDFLongAD()
        self.extended_attr_icb.parse(extended_attr_icb)

        self.impl_ident = UDFEntityID()
        self.impl_ident.parse(impl_ident)

        offset = struct.calcsize(self.FMT)
        self.extended_attrs = data[offset:offset + self.len_extended_attrs]

        offset += self.len_extended_attrs

        self.alloc_descs = _parse_allocation_descriptors(self.icb_tag.flags,
                                                         data[offset:],
                                                         len_alloc_descs,
                                                         offset, extent)
        self.orig_extent_loc = extent

        self.parent = parent

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF File Entry.

        Parameters:
         None.
        Returns:
         A string representing this UDF File Entry.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF File Entry not initialized')

        len_alloc_descs = 0
        for desc in self.alloc_descs:
            len_alloc_descs += desc.length()

        rec = struct.pack(self.FMT, b'\x00' * 16,
                          self.icb_tag.record(), self.uid, self.gid,
                          self.perms, self.file_link_count, 0, 0, 0,
                          self.info_len, self.log_block_recorded,
                          self.access_time.record(), self.mod_time.record(),
                          self.attr_time.record(), 1,
                          self.extended_attr_icb.record(),
                          self.impl_ident.record(), self.unique_id,
                          self.len_extended_attrs, len_alloc_descs)[16:]
        rec += self.extended_attrs
        for desc in self.alloc_descs:
            rec += desc.record()

        return self.desc_tag.record(rec) + rec

    def extent_location(self):
        # type: () -> int
        """
        Get the extent location of this UDF File Entry.

        Parameters:
         None.
        Returns:
         Integer extent location of this UDF File Entry.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF File Entry not initialized')

        if self.new_extent_loc < 0:
            return self.orig_extent_loc
        return self.new_extent_loc

    def new(self, length, file_type, parent, log_block_size):
        # type: (int, str, Optional[UDFFileEntry], int) -> None
        """
        Create a new UDF File Entry.

        Parameters:
         length - The (starting) length of this UDF File Entry; this is ignored
                  if this is a symlink.
         file_type - The type that this UDF File entry represents; one of 'dir',
                     'file', or 'symlink'.
         parent - The parent UDF File Entry for this UDF File Entry.
         log_block_size - The logical block size for extents.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF File Entry already initialized')

        if file_type not in ('dir', 'file', 'symlink'):
            raise pycdlibexception.PyCdlibInternalError("UDF File Entry file type must be one of 'dir', 'file', or 'symlink'")

        self.desc_tag = UDFTag()
        self.desc_tag.new(261)  # FIXME: let the user set serial_number

        self.icb_tag = UDFICBTag()
        self.icb_tag.new(file_type)

        self.uid = 4294967295  # Really -1, which means unset
        self.gid = 4294967295  # Really -1, which means unset
        if file_type == 'dir':
            self.perms = 5285
            self.file_link_count = 0
            self.info_len = 0
            self.log_block_recorded = 1
            # The position is bogus, but will get set
            # properly once reshuffle_extents is called.
            short_ad = UDFShortAD()
            short_ad.new(length)
            self.alloc_descs.append(short_ad)
        else:
            self.perms = 4228
            self.file_link_count = 1
            self.info_len = length
            self.log_block_recorded = utils.ceiling_div(length, log_block_size)
            len_left = length
            while len_left > 0:
                # According to Ecma-167 14.14.1.1, the least-significant 30 bits
                # of the allocation descriptor length field specify the length
                # (the most significant two bits are properties which we don't
                # currently support).  In theory we should then split files
                # into 2^30 = 0x40000000, but all implementations I've seen
                # split it into smaller.  cdrkit/cdrtools uses 0x3ffff800, and
                # Windows uses 0x3ff00000.  To be more compatible with cdrkit,
                # we'll choose their number of 0x3ffff800.
                alloc_len = min(len_left, 0x3ffff800)
                short_ad = UDFShortAD()
                short_ad.new(alloc_len)
                self.alloc_descs.append(short_ad)
                len_left -= alloc_len

        self.access_time = UDFTimestamp()
        self.access_time.new(time.time())

        self.mod_time = UDFTimestamp()
        self.mod_time.new(time.time())

        self.attr_time = UDFTimestamp()
        self.attr_time.new(time.time())

        self.extended_attr_icb = UDFLongAD()
        self.extended_attr_icb.new(0, 0)

        self.impl_ident = UDFEntityID()
        self.impl_ident.new(0, b'*pycdlib')

        self.unique_id = 0  # this will get set later
        self.len_extended_attrs = 0  # FIXME: let the user set this

        self.extended_attrs = b''

        self.parent = parent

        self._initialized = True

    def set_extent_location(self, new_location, tag_location):
        # type: (int, int) -> None
        """
        Set the location of this UDF File Entry.

        Parameters:
         new_location - The new extent this UDF File Entry should be located at.
         tag_location - The new relative extent this UDF File Entry should be located at.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF File Entry not initialized')

        self.new_extent_loc = new_location
        self.desc_tag.tag_location = tag_location
        self.unique_id = new_location

    def add_file_ident_desc(self, new_fi_desc, logical_block_size):
        # type: (UDFFileIdentifierDescriptor, int) -> int
        """
        Add a new UDF File Identifier Descriptor to this UDF File
        Entry.

        Parameters:
         new_fi_desc - The new UDF File Identifier Descriptor to add.
         logical_block_size - The logical block size to use.
        Returns:
         The number of extents added due to adding this File Identifier Descriptor.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF File Entry not initialized')

        if self.icb_tag.file_type != 4:
            raise pycdlibexception.PyCdlibInvalidInput('Can only add a UDF File Identifier to a directory')

        self.fi_descs.append(new_fi_desc)

        num_bytes_to_add = UDFFileIdentifierDescriptor.length(len(new_fi_desc.fi))

        old_num_extents = 0
        # If info_len is 0, then this is a brand-new File Entry, and thus the
        # number of extents it is using is 0.
        if self.info_len > 0:
            old_num_extents = utils.ceiling_div(self.info_len, logical_block_size)

        self.info_len += num_bytes_to_add
        new_num_extents = utils.ceiling_div(self.info_len, logical_block_size)

        self.log_block_recorded = new_num_extents

        self.alloc_descs[0].extent_length = self.info_len
        if new_fi_desc.is_dir():
            self.file_link_count += 1

        return new_num_extents - old_num_extents

    def remove_file_ident_desc_by_name(self, name, logical_block_size):
        # type: (bytes, int) -> int
        """
        Remove a UDF File Identifier Descriptor from this UDF File Entry.

        Parameters:
         name - The name of the UDF File Identifier Descriptor to remove.
         logical_block_size - The logical block size to use.
        Returns:
         The number of extents removed due to removing this File Identifier Descriptor.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF File Entry not initialized')

        tmp_fi_desc = UDFFileIdentifierDescriptor()
        tmp_fi_desc.isparent = False
        tmp_fi_desc.fi = name

        # If flags bit 3 is set, the entries are sorted.
        desc_index = len(self.fi_descs)
        for index, fi_desc in enumerate(self.fi_descs):
            if fi_desc.fi == name:
                desc_index = index
                break
        if desc_index == len(self.fi_descs) or self.fi_descs[desc_index].fi != name:
            raise pycdlibexception.PyCdlibInvalidInput('Cannot find file to remove')

        this_desc = self.fi_descs[desc_index]
        if this_desc.is_dir():
            if this_desc.file_entry is None:
                raise pycdlibexception.PyCdlibInternalError('No UDF File Entry for UDF File Descriptor')
            if len(this_desc.file_entry.fi_descs) > 1:
                raise pycdlibexception.PyCdlibInvalidInput('Directory must be empty to use rm_directory')
            self.file_link_count -= 1

        old_num_extents = utils.ceiling_div(self.info_len, logical_block_size)
        self.info_len -= UDFFileIdentifierDescriptor.length(len(this_desc.fi))
        new_num_extents = utils.ceiling_div(self.info_len, logical_block_size)
        self.alloc_descs[0].extent_length = self.info_len

        del self.fi_descs[desc_index]

        return old_num_extents - new_num_extents

    def set_data_location(self, current_extent, start_extent):  # pylint: disable=unused-argument
        # type: (int, int) -> None
        """
        Set the location of the data that this UDF File Entry points to.

        Parameters:
         current_extent - Unused
         start_extent - The starting extent for this data location.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF File Entry not initialized')

        current_assignment = start_extent
        for desc in self.alloc_descs:
            desc.log_block_num = current_assignment
            current_assignment += utils.ceiling_div(desc.extent_length, 2048)

    def get_data_length(self):
        # type: () -> int
        """
        Get the length of the data that this UDF File Entry points to.

        Parameters:
         None.
        Returns:
         The length of the data that this UDF File Entry points to.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF File Entry not initialized')
        return self.info_len

    def set_data_length(self, length):
        # type: (int) -> None
        """
        Set the length of the data that this UDF File Entry points to.

        Parameters:
         length - The new length for the data.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF File Entry not initialized')

        len_diff = length - self.info_len
        if len_diff > 0:
            # If we are increasing the length, update the last alloc_desc up
            # to the max of 0x3ffff800, and throw an exception if we overflow.
            new_len = self.alloc_descs[-1].extent_length + len_diff
            if new_len > 0x3ffff800:
                raise pycdlibexception.PyCdlibInvalidInput('Cannot increase the size of a UDF file beyond the current descriptor')
            self.alloc_descs[-1].extent_length = new_len
        elif len_diff < 0:
            # We are decreasing the length.  It's possible we are removing one
            # or more alloc_descs, so run through the list updating all of the
            # descriptors and remove any we no longer need.
            len_left = length
            alloc_descs_needed = 0
            index = 0
            while len_left > 0:
                this_len = min(len_left, 0x3ffff800)
                alloc_descs_needed += 1
                self.alloc_descs[index].extent_length = this_len
                index += 1
                len_left -= this_len

            self.alloc_descs = self.alloc_descs[:alloc_descs_needed]

        self.info_len = length

    def is_file(self):
        # type: () -> bool
        """
        Determine whether this UDF File Entry points to a file.

        Parameters:
         None.
        Returns:
         True if this UDF File Entry points to a file, False otherwise.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF File Entry not initialized')
        return self.icb_tag.file_type == 5

    def is_symlink(self):
        # type: () -> bool
        """
        Determine whether this UDF File Entry points to a symlink.

        Parameters:
         None.
        Returns:
         True if this UDF File Entry points to a symlink, False otherwise.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF File Entry not initialized')
        return self.icb_tag.file_type == 12

    def is_dir(self):
        # type: () -> bool
        """
        Determine whether this UDF File Entry points to a directory.

        Parameters:
         None.
        Returns:
         True if this UDF File Entry points to a directory, False otherwise.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF File Entry not initialized')
        return self.icb_tag.file_type == 4

    def file_identifier(self):
        # type: () -> bytes
        """
        Get the name of this UDF File Entry as a byte string.

        Parameters:
         None.
        Returns:
         The UDF File Entry as a byte string.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF File Entry not initialized')

        if self.file_ident is None:
            return b'/'

        return self.file_ident.fi

    def find_file_ident_desc_by_name(self, currpath):
        # type: (bytes) -> UDFFileIdentifierDescriptor
        """
        Find a UDF File Identifier descriptor by its name.

        Parameters:
         currpath - The UTF-8 encoded name to look up.
        Returns:
         The UDF File Identifier descriptor corresponding to the passed in name.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF File Entry not initialized')

        # If this is a directory or it is an empty directory, just skip
        # all work.
        if self.icb_tag.file_type != 4 or not self.fi_descs:
            raise pycdlibexception.PyCdlibInvalidInput('Could not find path')

        tmp = currpath.decode('utf-8')
        try:
            latin1_currpath = tmp.encode('latin-1')
        except (UnicodeDecodeError, UnicodeEncodeError):
            latin1_currpath = b''
        ucs2_currpath = tmp.encode('utf-16_be')

        child = None

        for fi_desc in self.fi_descs:
            if latin1_currpath and fi_desc.encoding == 'latin-1':
                eq = fi_desc.fi == latin1_currpath
            else:
                eq = fi_desc.fi == ucs2_currpath

            if eq:
                child = fi_desc
                break

        if child is None:
            raise pycdlibexception.PyCdlibInvalidInput('Could not find path')

        return child

    def track_file_ident_desc(self, file_ident):
        # type: (UDFFileIdentifierDescriptor) -> None
        """
        Start tracking a UDF File Identifier descriptor in this UDF File Entry.
        Both 'tracking' and 'addition' add the identifier to the list of file
        identifiers, but tracking doees not expand or otherwise modify the UDF
        File Entry.

        Parameters:
         file_ident - The UDF File Identifier Descriptor to start tracking.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF File Entry not initialized')

        self.fi_descs.append(file_ident)

    def is_dot(self):
        # type: () -> bool
        """
        A dummy method to determine whether this is a 'dot' entry.  Since this
        concept doesn't exist in UDF, it always returns False.

        Parameters:
         None.
        Returns:
         False.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF File Entry not initialized')
        return False

    def is_dotdot(self):
        # type: () -> bool
        """
        A dummy method to determine whether this is a 'dotdot' entry.  While UDF
        has the concept of 'parent' identifiers that are roughly equivalent,
        pycdlib doesn't attach UDF File Entries to them, so this method always
        return False.

        Parameters:
         None.
        Returns:
         False.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF File Entry not initialized')
        return False


class UDFFileIdentifierDescriptor:
    """A class representing a UDF File Identifier Descriptor (ECMA-167, Part 4, 14.4)."""
    __slots__ = ('_initialized', 'orig_extent_loc', 'new_extent_loc',
                 'desc_tag', 'file_characteristics', 'len_fi', 'len_impl_use',
                 'fi', 'isdir', 'isparent', 'icb', 'impl_use', 'file_entry',
                 'encoding', 'parent')

    FMT = '<16sHBB16sH'

    def __init__(self):
        # type: () -> None
        self.file_entry = None  # type: Optional[UDFFileEntry]
        self._initialized = False
        self.fi = b''
        self.encoding = ''
        self.isparent = False
        self.isdir = False
        self.parent = None  # type: Optional[UDFFileEntry]
        self.new_extent_loc = -1

    @classmethod
    def length(cls, namelen):
        # type: (Type[UDFFileIdentifierDescriptor], int) -> int
        """
        A class method to calculate the size this UDFFileIdentifierDescriptor
        would take up.

        Parameters:
         cls - The class to use (always UDFFileIdentifierDescriptor).
         namelen - The length of the name.
        Returns:
         The length that the UDFFileIdentifierDescriptor would take up.
        """
        if namelen > 0:
            namelen += 1
        to_add = struct.calcsize(cls.FMT) + namelen
        return to_add + UDFFileIdentifierDescriptor.pad(to_add)

    @staticmethod
    def pad(val):
        # type: (int) -> int
        """
        A static method to calculate the amount of padding necessary for this
        UDF File Identifer Descriptor.

        Parameters:
         val - The amount of non-padded space this UDF File Identifier
               Descriptor uses.
        Returns:
         The amount of padding necessary to make this a compliant UDF File
         Identifier Descriptor.
        """
        return (4 * ((val + 3) // 4)) - val

    def parse(self, data, extent, desc_tag, parent):
        # type: (bytes, int, UDFTag, UDFFileEntry) -> int
        """
        Parse the passed in data into a UDF File Identifier Descriptor.

        Parameters:
         data - The data to parse.
         extent - The extent that this descriptor currently lives at.
         desc_tag - A UDFTag object that represents the Descriptor Tag.
         parent - The UDF File Entry representing the parent.
        Returns:
         The number of bytes this descriptor consumed.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF File Identifier Descriptor already initialized')

        (tag_unused, file_version_num, self.file_characteristics,
         self.len_fi, icb, self.len_impl_use) = struct.unpack_from(self.FMT, data, 0)

        self.desc_tag = desc_tag

        if file_version_num != 1:
            raise pycdlibexception.PyCdlibInvalidISO('File Identifier Descriptor file version number not 1')

        if self.file_characteristics & 0x2:
            self.isdir = True

        if self.file_characteristics & 0x8:
            self.isparent = True

        self.icb = UDFLongAD()
        self.icb.parse(icb)

        start = struct.calcsize(self.FMT)
        end = start + self.len_impl_use
        self.impl_use = data[start:end]

        start = end
        end = start + self.len_fi
        # The very first byte of the File Identifier describes whether this is
        # an 8-bit or 16-bit encoded string; this corresponds to whether we
        # encode with 'latin-1' or with 'utf-16_be'.  We save that off because
        # we have to write the correct thing out when we record.
        if not self.isparent:
            encoding = bytes(bytearray([data[start]]))
            if encoding == b'\x08':
                self.encoding = 'latin-1'
            elif encoding == b'\x10':
                self.encoding = 'utf-16_be'
            else:
                raise pycdlibexception.PyCdlibInvalidISO('Only UDF File Identifier Descriptor Encodings 8 or 16 are supported')

            start += 1

            self.fi = data[start:end]

        self.orig_extent_loc = extent

        self.parent = parent

        self._initialized = True

        return end + UDFFileIdentifierDescriptor.pad(end)

    def is_dir(self):
        # type: () -> bool
        """
        Determine if this File Identifier represents a directory.

        Parameters:
         None.
        Returns:
         True if this File Identifier represents a directory, False otherwise.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF File Identifier Descriptor not initialized')
        return self.isdir

    def is_parent(self):
        # type: () -> bool
        """
        Determine if this File Identifier is a 'parent' (essentially ..).

        Parameters:
         None.
        Returns:
         True if this File Identifier is a parent, False otherwise.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF File Identifier Descriptor not initialized')
        return self.isparent

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF File Identifier Descriptor.

        Parameters:
         None.
        Returns:
         A string representing this UDF File Identifier Descriptor.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF File Identifier Descriptor not initialized')

        if self.len_fi > 0:
            if self.encoding == 'latin-1':
                prefix = b'\x08'
            elif self.encoding == 'utf-16_be':
                prefix = b'\x10'
            else:
                raise pycdlibexception.PyCdlibInternalError('Invalid UDF encoding; this should not happen')

            fi = prefix + self.fi
        else:
            fi = b''
        rec = struct.pack(self.FMT, b'\x00' * 16, 1,
                          self.file_characteristics, self.len_fi,
                          self.icb.record(),
                          self.len_impl_use) + self.impl_use + fi + b'\x00' * UDFFileIdentifierDescriptor.pad(struct.calcsize(self.FMT) + self.len_impl_use + self.len_fi)
        return self.desc_tag.record(rec[16:]) + rec[16:]

    def extent_location(self):
        # type: () -> int
        """
        Get the extent location of this UDF File Identifier.

        Parameters:
         None.
        Returns:
         Integer extent location of this UDF File Identifier.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF File Identifier not initialized')

        if self.new_extent_loc < 0:
            return self.orig_extent_loc
        return self.new_extent_loc

    def new(self, isdir, isparent, name, parent):
        # type: (bool, bool, bytes, Optional[UDFFileEntry]) -> None
        """
        Create a new UDF File Identifier.

        Parameters:
         isdir - Whether this File Identifier is a directory.
         isparent - Whether this File Identifier is a parent (..).
         name - The name for this File Identifier.
         parent - The UDF File Entry representing the parent.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF File Identifier already initialized')

        self.desc_tag = UDFTag()
        self.desc_tag.new(257)  # FIXME: let the user set serial_number

        self.icb = UDFLongAD()
        self.icb.new(2048, 2)

        self.isdir = isdir
        self.isparent = isparent
        self.file_characteristics = 0
        if self.isdir:
            self.file_characteristics |= 0x2
        if self.isparent:
            self.file_characteristics |= 0x8
        self.len_impl_use = 0  # FIXME: let the user set this

        self.impl_use = b''

        self.len_fi = 0
        if not isparent:
            bytename = name.decode('utf-8')
            try:
                self.fi = bytename.encode('latin-1')
                self.encoding = 'latin-1'
            except UnicodeEncodeError:
                self.fi = bytename.encode('utf-16_be')
                self.encoding = 'utf-16_be'
            self.len_fi = len(self.fi) + 1

        self.parent = parent

        self._initialized = True

    def set_extent_location(self, new_location, tag_location):
        # type: (int, int) -> None
        """
        Set the location of this UDF File Identifier Descriptor.  Note that
        many UDF File Identifier Descriptors may have the same starting extent.

        Parameters:
         new_location - The new extent this UDF File Identifier Descriptor should be located at.
         tag_location - The new relative extent this UDF File Identifier Descriptor should be located at.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF File Identifier not initialized')

        self.new_extent_loc = new_location
        self.desc_tag.tag_location = tag_location

    def set_icb(self, new_location, tag_location):
        # type: (int, int) -> None
        """
        Set the location of the data that this UDF File Identifier Descriptor
        points at.  The data can either be for a directory or for a file.

        Parameters:
         new_location - The new extent this UDF File Identifier Descriptor data lives at.
         tag_location - The new relative extent this UDF File Identifier Descriptor data lives at.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF File Identifier not initialized')

        self.icb.set_extent_location(new_location, tag_location)

    def __lt__(self, other):
        # type: (UDFFileIdentifierDescriptor) -> bool
        if self.isparent:
            if other.isparent:
                return False
            return True
        if other.isparent:
            return False

        return self.fi < other.fi

    def __eq__(self, other):
        # type: (object) -> bool
        if not isinstance(other, UDFFileIdentifierDescriptor):
            return NotImplemented

        if self.isparent:
            if other.isparent:
                return True
            return False
        if other.isparent:
            return False

        return self.fi == other.fi


class UDFSpaceBitmapDescriptor:
    """A class representing a UDF Space Bitmap Descriptor."""
    __slots__ = ('_initialized', 'num_bits', 'num_bytes', 'bitmap',
                 'new_extent_loc', 'orig_extent_loc', 'desc_tag')

    FMT = '<16sLL24s'

    def __init__(self):
        # type: () -> None
        self.new_extent_loc = -1
        self._initialized = False

    def parse(self, data, extent_loc, desc_tag):
        # type: (bytes, int, UDFTag) -> None
        """
        Parse the passed in data into a UDF Space Bitmap Descriptor.

        Parameters:
         data - The data to parse.
         extent_loc - The extent location this UDF Space Bitmap Descriptor
                      lives at.
         desc_tag - The UDFTag describing this UDF Space Bitmap Descriptor.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Space Bitmap Descriptor already initialized')

        (tag_unused, self.num_bits, self.num_bytes,
         self.bitmap) = struct.unpack_from(self.FMT, data, 0)

        self.orig_extent_loc = extent_loc

        self.desc_tag = desc_tag

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF Space Bitmap Descriptor.

        Parameters:
         None.
        Returns:
         A string representing this UDF Space Bitmap Descriptor.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Space Bitmap Descriptor not initialized')

        rec = struct.pack(self.FMT, b'\x00' * 16, self.num_bits, self.num_bytes,
                          self.bitmap)[16:]

        return self.desc_tag.record(rec) + rec

    def new(self):
        # type: () -> None
        """
        Create a new UDF Space Bitmap Descriptor.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Space Bitmap Descriptor already initialized')

        self.num_bits = 0
        self.num_bytes = 0
        self.bitmap = b''

        self.desc_tag = UDFTag()
        self.desc_tag.new(264)

        self._initialized = True

    def extent_location(self):
        # type: () -> int
        """
        Get the extent location of this UDF Space Bitmap Descriptor.

        Parameters:
         None.
        Returns:
         Integer extent location of this UDF Space Bitmap Descriptor.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Space Bitmap Descriptor not initialized')

        if self.new_extent_loc < 0:
            return self.orig_extent_loc
        return self.new_extent_loc

    def set_extent_location(self, extent):
        # type: (int) -> None
        """
        Set the new location for this UDF Space Bitmap Descriptor.

        Parameters:
         extent - The new extent location to set for this UDF Space Bitmap Descriptor.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Space Bitmap Descriptor not initialized')
        self.new_extent_loc = extent


class UDFAllocationExtentDescriptor:
    """A class representing a UDF Space Bitmap Descriptor (ECMA-167, Part 4, 14.5)."""
    __slots__ = ('_initialized', 'prev_allocation_extent_loc',
                 'len_allocation_descs', 'new_extent_loc', 'orig_extent_loc',
                 'desc_tag')

    FMT = '<16sLL'

    def __init__(self):
        # type: () -> None
        self.new_extent_loc = -1
        self._initialized = False

    def parse(self, data, extent_loc, desc_tag):
        # type: (bytes, int, UDFTag) -> None
        """
        Parse the passed in data into a UDF Allocation Extent Descriptor.

        Parameters:
         data - The data to parse.
         extent_loc - The extent location that this UDF Allocation Extent
                      Descriptor lives at.
         desc_tag - The UDF Tag associated with this UDF Allocation Extent
                    Descriptor.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Allocation Extent Descriptor already initialized')

        (tag_unused, self.prev_allocation_extent_loc,
         self.len_allocation_descs) = struct.unpack_from(self.FMT, data, 0)

        self.orig_extent_loc = extent_loc

        self.desc_tag = desc_tag

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF Allocation Extent Descriptor.

        Parameters:
         None.
        Returns:
         A string representing this UDF Allocation Extent Descriptor.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Allocation Extent Descriptor not initialized')

        rec = struct.pack(self.FMT, b'\x00' * 16,
                          self.prev_allocation_extent_loc,
                          self.len_allocation_descs)[16:]

        return self.desc_tag.record(rec) + rec

    def new(self):
        # type: () -> None
        """
        Create a new UDF Allocation Extent Descriptor.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Allocation Extent Descriptor already initialized')

        self.desc_tag = UDFTag()
        self.desc_tag.new(258)

        self.prev_allocation_extent_loc = 0  # FIXME: allow these to be set
        self.len_allocation_descs = 0  # FIXME: allow these to be set

        self._initialized = True

    def extent_location(self):
        # type: () -> int
        """
        Get the extent location of this UDF Allocation Extent Descriptor.

        Parameters:
         None.
        Returns:
         Integer extent location of this UDF Allocation Extent Descriptor.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Allocation Extent Descriptor not initialized')

        if self.new_extent_loc < 0:
            return self.orig_extent_loc
        return self.new_extent_loc

    def set_extent_location(self, extent):
        # type: (int) -> None
        """
        Set the new location for this UDF Allocation Extent Descriptor.

        Parameters:
         extent - The new extent location to set for this UDF Allocation Extent Descriptor.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Allocation Extent Descriptor not initialized')
        self.new_extent_loc = extent


class UDFIndirectEntry:
    """A class representing a UDF Indirect Entry (ECMA-167, Part 4, 14.7)."""
    __slots__ = ('_initialized', 'icb_tag', 'indirect_icb', 'desc_tag')

    FMT = '=16s20s16s'

    def __init__(self):
        # type: () -> None
        self._initialized = False

    def parse(self, data, desc_tag):
        # type: (bytes, UDFTag) -> None
        """
        Parse the passed in data into a UDF Indirect Entry.

        Parameters:
         data - The data to parse.
         desc_tag - The UDF Tag associated with this UDF Allocation Extent
                    Descriptor.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Indirect Entry already initialized')

        (tag_unused, icb_tag, indirect_icb) = struct.unpack_from(self.FMT, data, 0)

        self.desc_tag = desc_tag

        self.icb_tag = UDFICBTag()
        self.icb_tag.parse(icb_tag)

        self.indirect_icb = UDFLongAD()
        self.indirect_icb.parse(indirect_icb)

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF Indirect Entry.

        Parameters:
         None.
        Returns:
         A string representing this UDF Indirect Entry.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Indirect Entry not initialized')

        rec = struct.pack(self.FMT, b'\x00' * 16,
                          self.icb_tag.record(), self.indirect_icb.record())[16:]

        return self.desc_tag.record(rec) + rec

    def new(self, file_type):
        # type: (str) -> None
        """
        Create a new UDF Indirect Entry.

        Parameters:
         file_type - The type that this UDF File entry represents; one of 'dir',
                     'file', or 'symlink'.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Indirect Entry already initialized')

        self.desc_tag = UDFTag()
        self.desc_tag.new(259)  # FIXME: let the user set serial_number

        self.icb_tag = UDFICBTag()
        self.icb_tag.new(file_type)

        self.indirect_icb = UDFLongAD()
        self.indirect_icb.new(0, 0)

        self._initialized = True


class UDFTerminalEntry:
    """A class representing a UDF Terminal Entry (ECMA-167, Part 4, 14.8)."""
    __slots__ = ('_initialized', 'icb_tag', 'desc_tag')

    FMT = '=16s20s'

    def __init__(self):
        # type: () -> None
        self._initialized = False

    def parse(self, data, tag):
        # type: (bytes, UDFTag) -> None
        """
        Parse the passed in data into a UDF Terminal Entry.

        Parameters:
         data - The data to parse.
         desc_tag - The UDF Tag associated with this UDF Terminal Entry.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Terminal Entry already initialized')

        (tag_unused, icb_tag) = struct.unpack_from(self.FMT, data, 0)

        self.desc_tag = tag

        self.icb_tag = UDFICBTag()
        self.icb_tag.parse(icb_tag)

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF Terminal Entry.

        Parameters:
         None.
        Returns:
         A string representing this UDF Terminal Entry.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Terminal Entry not initialized')

        rec = struct.pack(self.FMT, b'\x00' * 16,
                          self.icb_tag.record())[16:]

        return self.desc_tag.record(rec) + rec

    def new(self, file_type):
        # type: (str) -> None
        """
        Create a new UDF Terminal Entry.

        Parameters:
         file_type - The type that this UDF Terminal entry represents; one of
                     'dir', 'file', or 'symlink'.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Terminal Entry already initialized')

        self.desc_tag = UDFTag()
        self.desc_tag.new(260)  # FIXME: let the user set serial_number

        self.icb_tag = UDFICBTag()
        self.icb_tag.new(file_type)

        self._initialized = True


class UDFExtendedAttributeHeaderDescriptor:
    """A class representing a UDF Extended Attribute Header Descriptor (ECMA-167, Part 4, 14.10.1)."""
    __slots__ = ('_initialized', 'impl_attr_loc', 'app_attr_loc',
                 'icb_tag', 'desc_tag')

    FMT = '<16sLL'

    def __init__(self):
        # type: () -> None
        self._initialized = False

    def parse(self, data, desc_tag):
        # type: (bytes, UDFTag) -> None
        """
        Parse the passed in data into a UDF Extended Attribute Header Descriptor.

        Parameters:
         data - The data to parse.
         desc_tag - The UDF Tag associated with this UDF Extended Attribute
                    Header Descriptor.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Extended Attribute Header Descriptor already initialized')

        (tag_unused, self.impl_attr_loc,
         self.app_attr_loc) = struct.unpack_from(self.FMT, data, 0)

        self.desc_tag = desc_tag

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF Extended Attribute Header
        Descriptor.

        Parameters:
         None.
        Returns:
         A string representing this UDF Extended Attribute Header Descriptor.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Extended Attribute Header Descriptor not initialized')

        rec = struct.pack(self.FMT, b'\x00' * 16,
                          self.impl_attr_loc, self.app_attr_loc)[16:]

        return self.desc_tag.record(rec) + rec

    def new(self):
        # type: () -> None
        """
        Create a new UDF Extended Attribute Header Descriptor.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Extended Attribute Header Descriptor already initialized')

        self.desc_tag = UDFTag()
        self.desc_tag.new(262)  # FIXME: let the user set serial_number

        self.impl_attr_loc = 0
        self.app_attr_loc = 0

        self._initialized = True


class UDFUnallocatedSpaceEntry:
    """A class representing a UDF Unallocated Space Entry (ECMA-167, Part 4, 14.11)."""
    __slots__ = ('_initialized', 'alloc_descs', 'icb_tag', 'desc_tag')

    FMT = '<16s20sL'

    def __init__(self):
        # type: () -> None
        self.alloc_descs = []  # type: List[Union[UDFShortAD, UDFLongAD, UDFInlineAD]]
        self._initialized = False

    def parse(self, data, extent, desc_tag):
        # type: (bytes, int, UDFTag) -> None
        """
        Parse the passed in data into a UDF Unallocated Space Entry.

        Parameters:
         data - The data to parse.
         extent - The extent associated with this UDF Unallocated Space Entry.
         desc_tag - The UDF Tag associated with this UDF Unallocated Space
                    Entry.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Unallocated Space Entry already initialized')

        (tag_unused, icb_tag,
         len_alloc_descs) = struct.unpack_from(self.FMT, data, 0)

        self.desc_tag = desc_tag

        self.icb_tag = UDFICBTag()
        self.icb_tag.parse(icb_tag)

        offset = 16 + 20 + 4

        self.alloc_descs = _parse_allocation_descriptors(self.icb_tag.flags,
                                                         data[offset:],
                                                         len_alloc_descs,
                                                         offset, extent)

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF Unallocated Space Entry.

        Parameters:
         None.
        Returns:
         A string representing this UDF Unallocated Space Entry.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Unallocated Space Entry not initialized')

        len_alloc_descs = 0
        for desc in self.alloc_descs:
            len_alloc_descs += desc.length()

        rec = struct.pack(self.FMT, b'\x00' * 16,
                          self.icb_tag.record(), len_alloc_descs)[16:]

        for desc in self.alloc_descs:
            rec += desc.record()

        return self.desc_tag.record(rec) + rec

    def new(self, file_type):
        # type: (str) -> None
        """
        Create a new UDF Unallocated Space Entry.

        Parameters:
         file_type - The type that this UDF Space Entry represents; one of
                     'dir', 'file', or 'symlink'.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Unallocated Space Entry already initialized')

        self.desc_tag = UDFTag()
        self.desc_tag.new(263)  # FIXME: let the user set serial_number

        self.icb_tag = UDFICBTag()
        self.icb_tag.new(file_type)

        self._initialized = True


class UDFPartitionIntegrityEntry:
    """A class representing a UDF Partition Integrity Entry (ECMA-167, Part 4, 14.13)."""
    __slots__ = ('_initialized', 'integrity_type', 'timestamp', 'impl_ident',
                 'impl_use', 'icb_tag', 'desc_tag')

    FMT = '=16s20s12sB175s32s256s'

    def __init__(self):
        # type: () -> None
        self._initialized = False

    def parse(self, data, desc_tag):
        # type: (bytes, UDFTag) -> None
        """
        Parse the passed in data into a UDF Partition Integrity Entry.

        Parameters:
         data - The data to parse.
         desc_tag - The UDF Tag associated with this UDF Partition Integrity
                    Entry.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Partition Integrity Entry already initialized')

        (tag_unused, icb_tag, record_date, self.integrity_type,
         reserved_unused, impl_ident,
         self.impl_use) = struct.unpack_from(self.FMT, data, 0)

        self.desc_tag = desc_tag

        self.icb_tag = UDFICBTag()
        self.icb_tag.parse(icb_tag)

        self.timestamp = UDFTimestamp()
        self.timestamp.parse(record_date)

        self.impl_ident = UDFEntityID()
        self.impl_ident.parse(impl_ident)

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF Partition Integrity Entry.

        Parameters:
         None.
        Returns:
         A string representing this UDF Partition Integrity Entry.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Partition Integrity Entry not initialized')

        rec = struct.pack(self.FMT, b'\x00' * 16,
                          self.icb_tag.record(), self.timestamp.record(),
                          self.integrity_type, b'\x00' * 175,
                          self.impl_ident.record(), self.impl_use)[16:]

        return self.desc_tag.record(rec) + rec

    def new(self, file_type):
        # type: (str) -> None
        """
        Create a new UDF Partition Integrity Entry.

        Parameters:
         file_type - The type that this UDF Space Entry represents; one of
                     'dir', 'file', or 'symlink'.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Partition Integrity Entry already initialized')

        self.desc_tag = UDFTag()
        self.desc_tag.new(265)  # FIXME: let the user set serial_number

        self.icb_tag = UDFICBTag()
        self.icb_tag.new(file_type)

        self.integrity_type = 1

        self.timestamp = UDFTimestamp()
        self.timestamp.new(time.time())

        self.impl_ident = UDFEntityID()
        self.impl_ident.new(0)

        self.impl_use = b'\x00' * 256

        self._initialized = True


class UDFExtendedFileEntry:
    """A class representing a UDF Extended File Entry (ECMA-167, Part 4, 14.17)."""
    __slots__ = ('_initialized', 'uid', 'gid', 'permissions', 'file_link_count',
                 'record_format', 'record_display_attrs', 'record_len',
                 'info_len', 'obj_size', 'log_blocks_recorded',
                 'access_time', 'mod_time', 'creation_time', 'impl_ident',
                 'attr_time', 'checkpoint', 'extended_attr_icb',
                 'stream_icb', 'extended_attrs', 'unique_id', 'alloc_descs',
                 'len_extended_attrs', 'icb_tag', 'desc_tag')

    FMT = '<16s20sLLLHBBLQQQ12s12s12s12sL4s16s16s32sQLL'

    def __init__(self):
        # type: () -> None
        self.alloc_descs = []  # type: List[Union[UDFShortAD, UDFLongAD, UDFInlineAD]]
        self._initialized = False

    def parse(self, data, extent, desc_tag):
        # type: (bytes, int, UDFTag) -> None
        """
        Parse the passed in data into a UDF Extended File Entry.

        Parameters:
         data - The data to parse.
         extent - The extent this Extended File Entry lives at.
         desc_tag - The UDF Tag associated with this UDF Extended File Entry.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Extended File Entry already initialized')

        (tag_unused, icb_tag, self.uid, self.gid, self.permissions,
         self.file_link_count, self.record_format, self.record_display_attrs,
         self.record_len, self.info_len, self.obj_size, self.log_blocks_recorded,
         access_time, mod_time, creation_time, attr_time,
         self.checkpoint, reserved_unused, extended_attr_icb, stream_icb,
         impl_ident, self.unique_id, self.len_extended_attrs,
         len_alloc_descs) = struct.unpack_from(self.FMT, data, 0)

        self.desc_tag = desc_tag

        self.icb_tag = UDFICBTag()
        self.icb_tag.parse(icb_tag)

        self.access_time = UDFTimestamp()
        self.access_time.parse(access_time)

        self.mod_time = UDFTimestamp()
        self.mod_time.parse(mod_time)

        self.creation_time = UDFTimestamp()
        self.creation_time.parse(creation_time)

        self.attr_time = UDFTimestamp()
        self.attr_time.parse(attr_time)

        self.extended_attr_icb = UDFLongAD()
        self.extended_attr_icb.parse(extended_attr_icb)

        self.stream_icb = UDFLongAD()
        self.stream_icb.parse(stream_icb)

        self.impl_ident = UDFEntityID()
        self.impl_ident.parse(impl_ident)

        offset = struct.calcsize(self.FMT)
        self.extended_attrs = data[offset:offset + self.len_extended_attrs]

        offset += self.len_extended_attrs

        self.alloc_descs = _parse_allocation_descriptors(self.icb_tag.flags,
                                                         data[offset:],
                                                         len_alloc_descs,
                                                         offset, extent)

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this UDF Extended File Entry.

        Parameters:
         None.
        Returns:
         A string representing this UDF Extended File Entry.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Extended File Entry not initialized')

        len_alloc_descs = 0
        for desc in self.alloc_descs:
            len_alloc_descs += desc.length()

        rec = struct.pack(self.FMT, b'\x00' * 16,
                          self.icb_tag.record(), self.uid, self.gid,
                          self.permissions, self.file_link_count,
                          self.record_format, self.record_display_attrs,
                          self.record_len, self.info_len, self.obj_size,
                          self.log_blocks_recorded, self.access_time.record(),
                          self.mod_time.record(), self.creation_time.record(),
                          self.attr_time.record(), self.checkpoint, b'\x00' * 4,
                          self.extended_attr_icb.record(),
                          self.stream_icb.record(), self.impl_ident.record(),
                          self.unique_id, self.len_extended_attrs,
                          len_alloc_descs)[16:]
        rec += self.extended_attrs
        for desc in self.alloc_descs:
            rec += desc.record()

        return self.desc_tag.record(rec) + rec

    def new(self, file_type, length, log_block_size):
        # type: (str, int, int) -> None
        """
        Create a new UDF Extended File Entry.

        Parameters:
         file_type - The type that this UDF Space Entry represents; one of
                     'dir', 'file', or 'symlink'.
         length - The (starting) length of this UDF File Entry; this is ignored
                  if this is a symlink.
         log_block_size - The logical block size for extents.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('UDF Extended File Entry already initialized')

        self.desc_tag = UDFTag()
        self.desc_tag.new(266)  # FIXME: let the user set serial_number

        self.icb_tag = UDFICBTag()
        self.icb_tag.new(file_type)

        self.uid = 4294967295  # Really -1, which means unset
        self.gid = 4294967295  # Really -1, which means unset
        if file_type == 'dir':
            self.permissions = 5285
            self.file_link_count = 0
            self.info_len = 0
            self.log_blocks_recorded = 1
            # The position is bogus, but will get set
            # properly once reshuffle_extents is called.
            short_ad = UDFShortAD()
            short_ad.new(length)
            self.alloc_descs.append(short_ad)
        else:
            self.permissions = 4228
            self.file_link_count = 1
            self.info_len = length
            self.log_blocks_recorded = utils.ceiling_div(length, log_block_size)
            len_left = length
            while len_left > 0:
                # According to Ecma-167 14.14.1.1, the least-significant 30 bits
                # of the allocation descriptor length field specify the length
                # (the most significant two bits are properties which we don't
                # currently support).  In theory we should then split files
                # into 2^30 = 0x40000000, but all implementations I've seen
                # split it into smaller.  cdrkit/cdrtools uses 0x3ffff800, and
                # Windows uses 0x3ff00000.  To be more compatible with cdrkit,
                # we'll choose their number of 0x3ffff800.
                alloc_len = min(len_left, 0x3ffff800)
                # The position is bogus, but will get set
                # properly once reshuffle_extents is called.
                short_ad = UDFShortAD()
                short_ad.new(alloc_len)
                self.alloc_descs.append(short_ad)
                len_left -= alloc_len

        self.access_time = UDFTimestamp()
        self.access_time.new(time.time())

        self.mod_time = UDFTimestamp()
        self.mod_time.new(time.time())

        self.attr_time = UDFTimestamp()
        self.attr_time.new(time.time())

        self.extended_attr_icb = UDFLongAD()
        self.extended_attr_icb.new(0, 0)

        self.impl_ident = UDFEntityID()
        self.impl_ident.new(0, b'*pycdlib')

        self.unique_id = 0  # this will get set later
        self.len_extended_attrs = 0  # FIXME: let the user set this

        self.extended_attrs = b''

        self._initialized = True


def symlink_to_bytes(symlink_target):
    # type: (str) -> bytes
    """
    Generate UDF symlink data from a Unix-like path.

    Parameters:
     symlink_target - The Unix-like path that is the symlink.
    Returns:
     The UDF data corresponding to the symlink.
    """
    symlink_data = bytearray()
    for comp in symlink_target.split('/'):
        if comp == '':
            # If comp is empty, then we know this is the leading slash
            # and we should make an absolute entry (double slashes and
            # such are weeded out by the earlier utils.normpath).
            symlink_data.extend(b'\x02\x00\x00\x00')
        elif comp == '.':
            symlink_data.extend(b'\x04\x00\x00\x00')
        elif comp == '..':
            symlink_data.extend(b'\x03\x00\x00\x00')
        else:
            symlink_data.extend(b'\x05')
            ostaname = _ostaunicode(comp)
            symlink_data.append(len(ostaname))
            symlink_data.extend(b'\x00\x00')
            symlink_data.extend(ostaname)

    return symlink_data


def _parse_allocation_descriptors(flags, data, length, start_offset, extent):
    # type: (int, bytes, int, int, int) -> List[Union[UDFShortAD, UDFLongAD, UDFInlineAD]]
    """
    Generate a list of allocation descriptors from the data.

    Parameters:
     flags - The flags describing which kind of allocation descriptors are in
             the data.
     data - The data to parse.
     length - The length of the data.
     start_offset - The start offset of the allocation descriptor data in the
                    overall data.
     extent - The extent at which the data lives.
    Returns:
     The list of allocation descriptors.
    """
    alloc_descs = []  # type: List[Union[UDFShortAD, UDFLongAD, UDFInlineAD]]

    offset = 0

    # Now we need to create the allocation descriptors.  How they are
    # represented changes depending on bits 0-2 of the icb_tag.flags field:
    # 0 = short_ad
    # 1 = long_ad
    # 2 = extended_ad
    # 3 = single descriptor spanning entire length of the Allocation
    # Descriptors field of this File Entry.
    if (flags & 0x7) == 0:
        while offset < length:
            short_ad = UDFShortAD()
            short_ad.parse(data[offset:])
            alloc_descs.append(short_ad)
            offset += short_ad.length()
    elif (flags & 0x7) == 1:
        while offset < length:
            long_ad = UDFLongAD()
            long_ad.parse(data[offset:])
            alloc_descs.append(long_ad)
            offset += long_ad.length()
    elif (flags & 0x7) == 2:
        raise pycdlibexception.PyCdlibInternalError('UDF Allocation Descriptor of type 2 (Extended) not yet supported')
    elif (flags & 0x7) == 3:
        inline_ad = UDFInlineAD()
        inline_ad.parse(length, extent, start_offset)
        alloc_descs.append(inline_ad)
    else:
        raise pycdlibexception.PyCdlibInvalidISO('UDF Allocation Descriptor type invalid')

    return alloc_descs


class UDFDescriptorSequence:
    '''
    A class to represent a UDF Descriptor Sequence.
    '''
    __slots__ = ('pvds', 'impl_use', 'partitions', 'logical_volumes',
                 'unallocated_space', 'terminator', 'desc_pointer')

    def __init__(self):
        # type: () -> None
        self.pvds = []  # type: List[UDFPrimaryVolumeDescriptor]
        self.impl_use = []  # type: List[UDFImplementationUseVolumeDescriptor]
        self.partitions = []  # type: List[UDFPartitionVolumeDescriptor]
        self.logical_volumes = []  # type: List[UDFLogicalVolumeDescriptor]
        self.unallocated_space = []  # type: List[UDFUnallocatedSpaceDescriptor]
        self.terminator = UDFTerminatingDescriptor()
        self.desc_pointer = UDFVolumeDescriptorPointer()

    def append_to_list(self, which, desc):
        # type: (str, Union[UDFPrimaryVolumeDescriptor, UDFImplementationUseVolumeDescriptor, UDFPartitionVolumeDescriptor, UDFLogicalVolumeDescriptor, UDFUnallocatedSpaceDescriptor]) -> None
        '''
        Append a descriptor to the list of descriptors, checking that
        there are no duplicates.

        Parameters:
         which - Which list to append to.
         desc - The descriptor to check and append.
        Returns:
         Nothing.
        '''
        # ECMA-167, Part 3, 8.4.2 says that all Volume Descriptors
        # with the same sequence numbers should have the same contents.
        # Check that here.
        vols = getattr(self, which)
        for vol in vols:
            if vol.vol_desc_seqnum == desc.vol_desc_seqnum:
                if vol != desc:
                    raise pycdlibexception.PyCdlibInvalidISO('Descriptors with same sequence number do not have the same contents')

        vols.append(desc)

    def assign_desc_extents(self, start_extent):
        # type: (int) -> None
        '''
        A method to assign a consecutive sequence of extents for the UDF
        Descriptors, starting at the given extent.

        Parameters:
         start_extent - The starting extent to assign from.
        Returns:
         Nothing.
        '''
        current_extent = start_extent

        for pvd in self.pvds:
            pvd.set_extent_location(current_extent)
            current_extent += 1

        if self.desc_pointer.initialized:
            self.desc_pointer.set_extent_location(current_extent)
            current_extent += 1

        for impl_use in self.impl_use:
            impl_use.set_extent_location(current_extent)
            current_extent += 1

        for partition in self.partitions:
            partition.set_extent_location(current_extent)
            current_extent += 1

        for logical_volume in self.logical_volumes:
            logical_volume.set_extent_location(current_extent)
            current_extent += 1

        for unallocated_space in self.unallocated_space:
            unallocated_space.set_extent_location(current_extent)
            current_extent += 1

        if self.terminator.initialized:
            self.terminator.set_extent_location(current_extent)
            current_extent += 1


def parse_udf_vol_descs(vd_data, extent, logical_block_size):
    # type: (bytes, int, int) -> UDFDescriptorSequence
    """
    An internal method to parse a set of UDF Volume Descriptors.

    Parameters:
     vd_data - The data to parse.
     extent - The extent at which to start parsing.
     logical_block_size - The logical block size for this ISO.
    Returns:
     The UDFDescriptorSequence object that stores parsed objects.
    """

    # Parse the data.  Since the sequence doesn't have to be in any set order,
    # and since some of the entries may be missing, we parse the Descriptor
    # Tag (the first 16 bytes) to find out what kind of descriptor it is,
    # then construct the correct type based on that.  We keep going until we
    # see a Terminating Descriptor.

    descs = UDFDescriptorSequence()
    offset = 0
    current_extent = extent
    done = False
    while not done:
        desc_tag = UDFTag()
        desc_tag.parse(vd_data[offset:], current_extent)
        if desc_tag.tag_ident == 1:
            pvd = UDFPrimaryVolumeDescriptor()
            pvd.parse(vd_data[offset:offset + 512], current_extent, desc_tag)
            descs.append_to_list('pvds', pvd)
        elif desc_tag.tag_ident == 3:
            descs.desc_pointer.parse(vd_data[offset:offset + 512],
                                     current_extent, desc_tag)
            done = True
        elif desc_tag.tag_ident == 4:
            impl_use = UDFImplementationUseVolumeDescriptor()
            impl_use.parse(vd_data[offset:offset + 512],
                           current_extent, desc_tag)
            descs.append_to_list('impl_use', impl_use)
        elif desc_tag.tag_ident == 5:
            partition = UDFPartitionVolumeDescriptor()
            partition.parse(vd_data[offset:offset + 512],
                            current_extent, desc_tag)
            descs.append_to_list('partitions', partition)
        elif desc_tag.tag_ident == 6:
            logical_volume = UDFLogicalVolumeDescriptor()
            logical_volume.parse(vd_data[offset:offset + 512],
                                 current_extent, desc_tag)
            descs.append_to_list('logical_volumes', logical_volume)
        elif desc_tag.tag_ident == 7:
            unallocated_space = UDFUnallocatedSpaceDescriptor()
            unallocated_space.parse(vd_data[offset:offset + 512],
                                    current_extent, desc_tag)
            descs.append_to_list('unallocated_space', unallocated_space)
        elif desc_tag.tag_ident == 8:
            descs.terminator.parse(current_extent, desc_tag)
            done = True
        elif desc_tag.tag_ident == 0:
            # This would be an unrecorded sector, so we are done.
            done = True
        else:
            raise pycdlibexception.PyCdlibInvalidISO('UDF Tag identifier not %d' % (desc_tag.tag_ident))

        offset += logical_block_size
        current_extent += 1

    if not descs.pvds:
        raise pycdlibexception.PyCdlibInvalidISO('At least one UDF Primary Volume Descriptor required')

    saw_zero_desc_num = False
    for pvd in descs.pvds:
        if pvd.desc_num == 0:
            if saw_zero_desc_num:
                raise pycdlibexception.PyCdlibInvalidISO('Only one UDF Primary Volume Descriptor can have a descriptor number 0')
            saw_zero_desc_num = True

    return descs


def parse_anchor(anchor_data, anchor_location):
    # type: (bytes, int) -> Optional[UDFAnchorVolumeStructure]
    """
    An internal method to parse data that may potentially be a UDF Anchor.

    Parameters:
     anchor_data - The data to parse.
     anchor_location - The extent location of the data.
    Returns:
     A UDFAnchorVolumeStructure if the data represents an anchor, None otherwise.
    """
    anchor_tag = UDFTag()
    try:
        anchor_tag.parse(anchor_data, anchor_location)
    except pycdlibexception.PyCdlibInvalidISO:
        return None

    if anchor_tag.tag_ident != 2:
        return None

    anchor = UDFAnchorVolumeStructure()
    anchor.parse(anchor_data, anchor_location, anchor_tag)
    return anchor


def parse_logical_volume_integrity(integrity_data, current_extent, logical_block_size):
    # type: (bytes, int, int) -> Tuple[UDFLogicalVolumeIntegrityDescriptor, Optional[UDFTerminatingDescriptor]]
    """
    An internal method to parse Logical Volume Integrity data, and an optional
    Logical Volume Integrity Terminator.

    Parameters:
     integrity_data - The data to parse.
     current_extent - The extent location of the data.
     logical_block_size - The logical block size of the ISO.
    Returns:
     A tuple where the first item is a UDFLogicalVolumeIntegrityDescriptor, and
     the second item is a optional UDFTerminatorDescriptor.
    """
    desc_tag = UDFTag()
    desc_tag.parse(integrity_data, current_extent)
    if desc_tag.tag_ident != 9:
        raise pycdlibexception.PyCdlibInvalidISO('UDF Volume Integrity Tag identifier not 9')
    logical_volume_integrity = UDFLogicalVolumeIntegrityDescriptor()
    logical_volume_integrity.parse(integrity_data[:512], current_extent, desc_tag)

    logical_volume_integrity_terminator = None
    offset = logical_block_size
    if len(integrity_data) >= (offset + logical_block_size):
        desc_tag = UDFTag()
        desc_tag.parse(integrity_data[offset:], current_extent + 1)
        if desc_tag.tag_ident != 8:
            raise pycdlibexception.PyCdlibInvalidISO('UDF Logical Volume Integrity Terminator Tag identifier not 8')
        logical_volume_integrity_terminator = UDFTerminatingDescriptor()
        logical_volume_integrity_terminator.parse(current_extent + 1, desc_tag)

    return logical_volume_integrity, logical_volume_integrity_terminator


def parse_file_set(file_set_and_term_data, current_extent, logical_block_size):
    # type: (bytes, int, int) -> Tuple[UDFFileSetDescriptor, UDFTerminatingDescriptor]
    """
    An internal method to parse File Set data, and the File Set Terminator.

    Parameters:
     file_set_and_term_data - The data to parse.
     current_extent - The extent location of the data.
     logical_block_size - The logical block size of the ISO.
    Returns:
     A tuple where the first item is a UDFFileSetDescriptor, and the second item
     is a UDFTerminatorDescriptor.
    """
    desc_tag = UDFTag()
    desc_tag.parse(file_set_and_term_data[:logical_block_size], 0)
    if desc_tag.tag_ident != 256:
        raise pycdlibexception.PyCdlibInvalidISO('UDF File Set Tag identifier not 256')
    file_set = UDFFileSetDescriptor()
    file_set.parse(file_set_and_term_data[:logical_block_size],
                   current_extent, desc_tag)

    (tag_ident,) = struct.unpack_from('<H', file_set_and_term_data, logical_block_size)
    file_set_terminator = UDFTerminatingDescriptor()
    if tag_ident == 8:
        desc_tag = UDFTag()
        desc_tag.parse(file_set_and_term_data[logical_block_size:], 1)
        file_set_terminator.parse(current_extent + 1, desc_tag)
    else:
        # In this case, the UDF ISO had an invalid File Set Terminator Tag.
        # But this isn't fatal, so log a warning and continue on.
        _logger.warning('Missing UDF File Set Terminator, continuing')
        file_set_terminator.new()
        file_set_terminator.orig_extent_loc = current_extent + 1
        file_set_terminator.desc_tag.tag_location = 1

    return file_set, file_set_terminator


def parse_file_entry(icbdata, abs_file_entry_extent, icb_log_block_num, parent):
    # type: (bytes, int, int, Optional[UDFFileEntry]) -> Optional[UDFFileEntry]
    """
    An internal method to parse a single UDF File Entry and return the
    corresponding object.

    Parameters:
     icbdata - The data to parse.
     abs_file_entry_extent - The extent number the file entry starts at.
     icb_log_block_num - The ICB logical block number.
     parent - The parent of the UDF File Entry.
    Returns:
     A UDF File Entry object corresponding to the on-disk File Entry.
    """
    if all(v == 0 for v in bytearray(icbdata)):
        # We have seen ISOs in the wild (Windows 2008 Datacenter Enterprise
        # Standard SP2 x86 DVD) where the UDF File Identifier points to a
        # UDF File Entry of all zeros.  In those cases, we just keep the
        # File Identifier, and keep the UDF File Entry blank.
        return None

    desc_tag = UDFTag()
    desc_tag.parse(icbdata, icb_log_block_num)
    if desc_tag.tag_ident != 261:
        raise pycdlibexception.PyCdlibInvalidISO('UDF File Entry Tag identifier not 261')

    file_entry = UDFFileEntry()
    file_entry.parse(icbdata, abs_file_entry_extent, parent, desc_tag)

    return file_entry


def parse_file_ident(data, current_extent, part_start, udf_file_entry):
    # type: (bytes, int, int, UDFFileEntry) -> Tuple[UDFFileIdentifierDescriptor, int]
    """
    An internal method to parse a single UDF File Identifier and return the
    corresponding object.

    Parameters:
     data - The data to parse.
     current_extent - The extent number the data is located at.
     part_start - The start of the logical partition.
     udf_file_entry - The UDF File Entry that corresponds with this File
                      Identifier.
    Returns:
     A tuple where the first item is a UDFFileIdentifierDescriptor, and the
     second item is the number of bytes the descriptor consumed.
    """
    desc_tag = UDFTag()
    desc_tag.parse(data, current_extent - part_start)
    if desc_tag.tag_ident != 257:
        raise pycdlibexception.PyCdlibInvalidISO('UDF File Identifier Tag identifier not 257')
    file_ident = UDFFileIdentifierDescriptor()
    bytes_forward = file_ident.parse(data, current_extent, desc_tag, udf_file_entry)

    return file_ident, bytes_forward
