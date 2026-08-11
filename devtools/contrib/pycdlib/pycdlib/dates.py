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
Classes and utilities for ISO date support.
"""

import functools
import struct
import time

from pycdlib import pycdlibexception
from pycdlib import utils


@functools.lru_cache(maxsize=256)
def string_to_timestruct(input_string):
    # type: (bytes) -> time.struct_time
    """
    A cacheable function to take an input string and decode it into a
    time.struct_time from the time module.  If the string cannot be decoded
    because of an illegal value, then the all-zero time.struct_time will be
    returned instead.

    Parameters:
     input_string - The string to attempt to parse.
    Returns:
     A time.struct_time object representing the time.
    """
    try:
        timestruct = time.strptime(input_string.decode('utf-8'), VolumeDescriptorDate.TIME_FMT)
    except ValueError:
        # Ecma-119, 8.4.26.1 specifies that if the string was all the digit
        # zero, with the last byte 0, the time wasn't specified.  In that
        # case, time.strptime() with our format will raise a ValueError.
        # In practice we have found that some ISOs specify various wacky
        # things in this field, so if we see *any* ValueError, we just
        # assume the date is unspecified and go with that.
        timestruct = time.struct_time((0, 0, 0, 0, 0, 0, 0, 0, 0))

    return timestruct


class DirectoryRecordDate:
    """
    A class to represent a Directory Record date as described in Ecma-119
    section 9.1.5.  The Directory Record date consists of the number of years
    since 1900, the month, the day of the month, the hour, the minute, the
    second, and the offset from GMT in 15 minute intervals.  There are two main
    ways to use this class: either to instantiate and then parse a string to
    fill in the fields (the parse() method), or to create a new entry with a
    tm structure (the new() method).
    """
    FMT = '=BBBBBBb'

    __slots__ = ('_initialized', 'years_since_1900', 'month', 'day_of_month',
                 'hour', 'minute', 'second', 'gmtoffset')

    def __init__(self):
        # type: () -> None
        self._initialized = False

    def parse(self, datestr):
        # type: (bytes) -> None
        """
        Parse a Directory Record date out of a string.

        Parameters:
         datestr - The string to parse the date out of.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Directory Record Date already initialized')

        (self.years_since_1900, self.month, self.day_of_month, self.hour,
         self.minute, self.second,
         self.gmtoffset) = struct.unpack_from(self.FMT, datestr, 0)

        self._initialized = True

    def new(self, tm):
        # type: (float) -> None
        """
        Create a new Directory Record Date.  tm is expected to be a float number
        of seconds from the epoch, which will be used to fill in this Volume
        Descriptor Date object.

        Parameters:
         tm - Seconds since the epoch to base the new DirectoryRecordDate off of.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Directory Record Date already initialized')

        # This algorithm was ported from cdrkit, genisoimage.c:iso9660_date()
        local = time.localtime(tm)
        self.years_since_1900 = local.tm_year - 1900
        self.month = local.tm_mon
        self.day_of_month = local.tm_mday
        self.hour = local.tm_hour
        self.minute = local.tm_min
        self.second = local.tm_sec
        self.gmtoffset = utils.gmtoffset_from_tm(tm, local)
        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Return a string representation of the Directory Record date.

        Parameters:
         None.
        Returns:
         A string representing this Directory Record Date.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Directory Record Date not initialized')

        return struct.pack(self.FMT, self.years_since_1900, self.month,
                           self.day_of_month, self.hour, self.minute,
                           self.second, self.gmtoffset)

    def __ne__(self, other):
        return self.years_since_1900 != other.years_since_1900 or \
            self.month != other.month or self.day_of_month != other.day_of_month or \
            self.hour != other.hour or self.minute != other.minute or \
            self.second != other.second or self.gmtoffset != other.gmtoffset


class VolumeDescriptorDate:
    """
    A class to represent a Volume Descriptor Date as described in Ecma-119
    section 8.4.26.1.  The Volume Descriptor Date consists of a year (from 1 to
    9999), month (from 1 to 12), day of month (from 1 to 31), hour (from 0
    to 23), minute (from 0 to 59), second (from 0 to 59), hundredths of second,
    and offset from GMT in 15-minute intervals (from -48 to +52) fields.  There
    are two main ways to use this class: either to instantiate and then parse a
    string to fill in the fields (the parse() method), or to create a new entry
    with a tm structure (the new() method).
    """

    TIME_FMT = '%Y%m%d%H%M%S'

    EMPTY_STRING = b'0' * 16 + b'\x00'

    __slots__ = ('_initialized', 'year', 'month', 'dayofmonth', 'hour',
                 'minute', 'second', 'hundredthsofsecond', 'gmtoffset',
                 'date_str')

    def __init__(self):
        # type: () -> None
        self._initialized = False

    def parse(self, datestr):
        # type: (bytes) -> None
        """
        Parse a Volume Descriptor Date out of a string.  A string of all zeros
        is valid, which means that the date in this field was not specified.

        Parameters:
         datestr - The string to be parsed.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Volume Descriptor Date object is already initialized')

        if len(datestr) != 17:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid ISO9660 date string')

        timestruct = string_to_timestruct(datestr[:-3])
        self.year = timestruct.tm_year
        self.month = timestruct.tm_mon
        self.dayofmonth = timestruct.tm_mday
        self.hour = timestruct.tm_hour
        self.minute = timestruct.tm_min
        self.second = timestruct.tm_sec
        if timestruct.tm_year == 0 and timestruct.tm_mon == 0 and timestruct.tm_mday == 0 and timestruct.tm_hour == 0 and timestruct.tm_min == 0 and timestruct.tm_sec == 0:
            self.hundredthsofsecond = 0
            self.gmtoffset = 0
            self.date_str = self.EMPTY_STRING
        else:
            try:
                self.hundredthsofsecond = int(datestr[14:16])
            except ValueError:
                # We've seen ISOs in the wild (made by MagicISO) that fill
                # hundredthsofseconds with b'\x00\x00'.  Handle that here.
                self.hundredthsofsecond, = struct.unpack('>H', datestr[14:16])
            self.gmtoffset, = struct.unpack_from('=b', datestr, 16)
            self.date_str = datestr

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Return the date string for this object.

        Parameters:
         None.
        Returns:
         Date as a string.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Volume Descriptor Date is not initialized')

        return self.date_str

    def new(self, tm):
        # type: (float) -> None
        """
        Create a new Volume Descriptor Date.  If tm is 0.0, then this Volume
        Descriptor Date will be full of zeros (meaning not specified).  If tm
        is not 0.0, it is expected to be a float number of seconds from the
        epoch, which will be used to fill in this Volume Descriptor Date object.

        Parameters:
         tm - Seconds since the epoch to base the new VolumeDescriptorDate off
              of, or 0.0 for an empty VolumeDescriptorDate.
        Returns:
          Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Volume Descriptor Date object is already initialized')

        if tm != 0.0:
            local = time.localtime(tm)
            self.year = local.tm_year
            self.month = local.tm_mon
            self.dayofmonth = local.tm_mday
            self.hour = local.tm_hour
            self.minute = local.tm_min
            self.second = local.tm_sec
            self.hundredthsofsecond = 0
            self.gmtoffset = utils.gmtoffset_from_tm(tm, local)
            self.date_str = time.strftime(self.TIME_FMT, local).encode('utf-8') + '{:0<2}'.format(self.hundredthsofsecond).encode('utf-8') + struct.pack('=b', self.gmtoffset)
        else:
            self.year = 0
            self.month = 0
            self.dayofmonth = 0
            self.hour = 0
            self.minute = 0
            self.second = 0
            self.hundredthsofsecond = 0
            self.gmtoffset = 0
            self.date_str = self.EMPTY_STRING

        self._initialized = True

    def __ne__(self, other):
        return self.year != other.year or self.month != other.month or \
            self.dayofmonth != other.dayofmonth or self.hour != other.hour or \
            self.minute != other.minute or self.second != other.second or \
            self.hundredthsofsecond != other.hundredthsofsecond or \
            self.gmtoffset != other.gmtoffset or self.date_str != other.date_str
