# Copyright (C) 2015-2021  Chris Lalancette <clalancette@gmail.com>

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

"""Class to support ISO9660 Path Table Records."""

import struct

from pycdlib import pycdlibexception
from pycdlib import utils

# For mypy annotations
if False:  # pylint: disable=using-constant-test
    from typing import Type  # NOQA pylint: disable=unused-import


class PathTableRecord:
    """A class that represents a single ISO9660 Path Table Record."""
    __slots__ = ('_initialized', 'len_di', 'xattr_length', 'extent_location',
                 'parent_directory_num', 'directory_identifier', 'dirrecord')

    FMT = '<BBLH'

    def __init__(self):
        # type: () -> None
        self._initialized = False

    def parse(self, data):
        # type: (bytes) -> None
        """
        Parse an ISO9660 Path Table Record out of a string.

        Parameters:
         data - The string to parse.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Path Table Record already initialized')

        (self.len_di, self.xattr_length, self.extent_location,
         self.parent_directory_num) = struct.unpack_from(self.FMT, data[:8], 0)

        if self.len_di % 2 != 0:
            self.directory_identifier = data[8:-1]
        else:
            self.directory_identifier = data[8:]
        self.dirrecord = None
        self._initialized = True

    def _record(self, ext_loc, parent_dir_num):
        # type: (int, int) -> bytes
        """
        An internal method to generate a string representing this Path Table Record.

        Parameters:
         ext_loc - The extent location to place in this Path Table Record.
         parent_dir_num - The parent directory number to place in this Path Table
                          Record.
        Returns:
         A string representing this Path Table Record.
        """
        return struct.pack(self.FMT, self.len_di, self.xattr_length,
                           ext_loc, parent_dir_num) + self.directory_identifier + b'\x00' * (self.len_di % 2)

    def record_little_endian(self):
        # type: () -> bytes
        """
        Generate a string representing the little endian version of this Path
        Table Record.

        Parameters:
         None.
        Returns:
         A string representing the little endian version of this Path Table Record.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Path Table Record not initialized')

        return self._record(self.extent_location, self.parent_directory_num)

    def record_big_endian(self):
        # type: () -> bytes
        """
        Generate a string representing the big endian version of
        this Path Table Record.

        Parameters:
         None.
        Returns:
         A string representing the big endian version of this Path Table Record.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Path Table Record not initialized')

        return self._record(utils.swab_32bit(self.extent_location),
                            utils.swab_16bit(self.parent_directory_num))

    @classmethod
    def record_length(cls, len_di):
        # type: (Type[PathTableRecord], int) -> int
        """
        A class method to calculate the length of a Path Table Record.

        Parameters:
         len_di - The length of the name for this Path Directory Record.
        Returns:
         The total length that a Path Directory Record with this name would occupy.
        """
        return struct.calcsize(cls.FMT) + len_di + (len_di % 2)

    def _new(self, name, parent_dir_num):
        # type: (bytes, int) -> None
        """
        An internal method to create a new Path Table Record.

        Parameters:
         name - The name for this Path Table Record.
         parent_dir_num - The directory number of the parent of this Path Table
                          Record.
        Returns:
         Nothing.
        """
        self.len_di = len(name)
        self.xattr_length = 0  # FIXME: we don't support xattr for now
        self.parent_directory_num = parent_dir_num
        self.directory_identifier = name
        self._initialized = True

    def new_root(self):
        # type: () -> None
        """
        Create a new root Path Table Record.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Path Table Record already initialized')

        self._new(b'\x00', 1)

    def new_dir(self, name):
        # type: (bytes) -> None
        """
        Create a new Path Table Record.

        Parameters:
         name - The name for this Path Table Record.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Path Table Record already initialized')

        # Zero for the parent dir num is bogus, but that will get fixed later.
        self._new(name, 0)

    def update_extent_location(self, extent_loc):
        # type: (int) -> None
        """
        Update the extent location for this Path Table Record.

        Parameters:
         extent_loc - The new extent location.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Path Table Record not initialized')

        self.extent_location = extent_loc

    def update_parent_directory_number(self, parent_dir_num):
        # type: (int) -> None
        """
        Update the parent directory number for this Path Table Record.

        Parameters:
         parent_dir_num - The new parent directory number to assign to this PTR.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Path Table Record not initialized')
        self.parent_directory_num = parent_dir_num

    def equal_to_be(self, be_record):
        # type: (PathTableRecord) -> bool
        """
        Compare a little-endian path table record to its big-endian counterpart.
        This is used to ensure that the ISO is sane.

        Parameters:
         be_record - The big-endian object to compare with the little-endian
                     object.
        Returns:
         True if this record is equal to the big-endian record passed in,
         False otherwise.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Path Table Record not initialized')

        if be_record.len_di != self.len_di or \
           be_record.xattr_length != self.xattr_length or \
           utils.swab_32bit(be_record.extent_location) != self.extent_location or \
           utils.swab_16bit(be_record.parent_directory_num) != self.parent_directory_num or \
           be_record.directory_identifier != self.directory_identifier:
            return False
        return True
