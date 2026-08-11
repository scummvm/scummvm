# Copyright (C) 2019-2022  Chris Lalancette <clalancette@gmail.com>

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

"""PyCdlibIO class."""

import io

from pycdlib import inode
from pycdlib import pycdlibexception

# For mypy annotations
if False:  # pylint: disable=using-constant-test
    import collections.abc  # NOQA pylint: disable=unused-import
    import ctypes  # NOQA pylint: disable=unused-import
    from mmap import mmap  # NOQA pylint: disable=unused-import
    import pickle  # NOQA pylint: disable=unused-import
    from typing import Any, Optional, Union  # NOQA pylint: disable=unused-import


class PyCdlibIO(io.RawIOBase):
    """
    The class that implements the user-facing python io-style context manager.
    Since ISOs are generally only readable, this is only a readable context
    manager.
    """
    __slots__ = ('_ctxt', '_fp', '_length', '_offset', '_open', '_startpos')

    def __init__(self, ino, logical_block_size):
        # type: (inode.Inode, int) -> None
        super(PyCdlibIO, self).__init__()  # pylint: disable=super-with-arguments
        self._ctxt = inode.InodeOpenData(ino, logical_block_size)
        self._open = True

    def __enter__(self):
        # _fp is the real file descriptor.  _length is the logical length
        # of the file.  _offset is the logical offset of this context
        # into the file.  _startpos is the absolute offset of the start of
        # this file into the backing file.
        (self._fp, self._length) = self._ctxt.__enter__()
        self._startpos = self._fp.tell()
        self._offset = 0
        return self

    def read(self, size=None):
        # type: (Optional[int]) -> bytes
        """
        Read and return up to size bytes.

        Parameters:
         size - Optional parameter to read size number of bytes; if None or
                negative, all remaining bytes in the file will be read
        Returns:
         The number of bytes requested or the rest of the data left in the file,
         whichever is smaller.  If the file is at or past EOF, returns an empty
         bytestring.
        """
        if not self._open:
            raise pycdlibexception.PyCdlibInvalidInput('I/O operation on closed file.')

        if self._offset >= self._length:
            return b''

        if size is None or size < 0:
            data = self.readall()
        else:
            readsize = min(self._length - self._offset, size)
            data = self._fp.read(readsize)
            self._offset += readsize

        return data

    def readall(self):
        # type: () -> bytes
        """
        Read and return the remaining bytes in the file.

        Parameters:
         None.
        Returns:
         The rest of the data left in the file.  If the file is at or past EOF,
         returns an empty bytestring.
        """
        if not self._open:
            raise pycdlibexception.PyCdlibInvalidInput('I/O operation on closed file.')

        readsize = self._length - self._offset
        if readsize > 0:
            data = self._fp.read(readsize)
            self._offset += readsize
        else:
            data = b''

        return data

    def readinto(self, b):
        # type: (collections.abc.Buffer) -> int
        if not self._open:
            raise pycdlibexception.PyCdlibInvalidInput('I/O operation on closed file.')

        readsize = self._length - self._offset
        if readsize > 0:
            mv = memoryview(b)
            m = mv.cast('B')
            readsize = min(readsize, len(m))
            data = self._fp.read(readsize)
            n = len(data)
            m[:n] = data
        else:
            n = 0

        return n

    def seek(self, offset, whence=0):
        # type: (int, int) -> int
        """
        Change the stream position to byte offset offset.  The offset is
        interpreted relative to the position indicated by whence.  Valid values
        for whence are:

        * 0 -- start of stream (the default); offset should be zero or positive
        * 1 -- current stream position; offset may be negative
        * 2 -- end of stream; offset is usually negative

        Parameters:
         offset - The byte offset to seek to.
         whence - The position in the file to start from (0 for start, 1 for
                  current, 2 for end)
        Returns:
         The new absolute position.
        """
        if not self._open:
            raise pycdlibexception.PyCdlibInvalidInput('I/O operation on closed file.')

        if isinstance(offset, float):
            raise pycdlibexception.PyCdlibInvalidInput('an integer is required')

        if whence == 0:
            # From beginning of file
            if offset < 0:
                raise pycdlibexception.PyCdlibInvalidInput('Invalid offset value (must be positive)')

            if offset < self._length:
                self._fp.seek(self._startpos + offset, 0)

            self._offset = offset
        elif whence == 1:
            # From current file position
            if self._offset + offset < 0:
                raise pycdlibexception.PyCdlibInvalidInput('Invalid offset value (cannot seek before start of file)')

            if self._offset + offset < self._length:
                self._fp.seek(self._startpos + self._offset + offset, 0)

            self._offset += offset
        elif whence == 2:
            # From end of file
            if offset < 0 and abs(offset) > self._length:
                raise pycdlibexception.PyCdlibInvalidInput('Invalid offset value (cannot seek before start of file)')

            if self._length + offset < self._length:
                self._fp.seek(self._startpos + self._length + offset, 0)

            self._offset = self._length + offset
        else:
            raise pycdlibexception.PyCdlibInvalidInput('Invalid value for whence (options are 0, 1, and 2)')

        return self._offset

    def tell(self):
        # type: () -> int
        """
        Return the current stream position.

        Parameters:
         None.
        Returns:
         The current stream position.
        """
        if not self._open:
            raise pycdlibexception.PyCdlibInvalidInput('I/O operation on closed file.')
        return self._offset

    def length(self):
        # type: () -> int
        """
        Return the length of the current file.

        Parameters:
         None.
        Returns:
         The length of the file.
        """
        if not self._open:
            raise pycdlibexception.PyCdlibInvalidInput('I/O operation on closed file.')
        return self._length

    def readable(self):
        # type: () -> bool
        """
        Determine whether this file is readable.

        Parameters:
         None.
        Returns:
         True in all cases.
        """
        if not self._open:
            raise pycdlibexception.PyCdlibInvalidInput('I/O operation on closed file.')
        return True

    def seekable(self):
        # type: () -> bool
        """
        Determine whether this file is seekable.

        Parameters:
         None.
        Returns:
         True in all cases.
        """
        if not self._open:
            raise pycdlibexception.PyCdlibInvalidInput('I/O operation on closed file.')
        return True

    def close(self):
        # type: () -> None
        """
        Close this file stream.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        self._open = False
        self._ctxt.__exit__()

    def __exit__(self, *args):
        self._ctxt.__exit__()
