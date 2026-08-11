# Copyright (C) 2019-2021  Chris Lalancette <clalancette@gmail.com>

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

"""Facade classes to make the main PyCdlib object easier to use."""

from pycdlib import dr
from pycdlib import pycdlibexception
from pycdlib import udf as udfmod
from pycdlib import utils

# For mypy annotations
if False:  # pylint: disable=using-constant-test
    from typing import BinaryIO, Generator, Optional, Tuple  # NOQA pylint: disable=unused-import
    # NOTE: these imports have to be here to avoid circular deps
    from pycdlib import pycdlib  # NOQA pylint: disable=unused-import,cyclic-import
    from pycdlib import pycdlibio  # NOQA pylint: disable=unused-import


def iso_path_to_rr_name(iso_path, interchange_level, is_dir):
    # type: (str, int, bool) -> str
    """
    Take an absolute ISO path and generate a corresponding Rock Ridge basename.

    Parameters:
     iso_path - The absolute iso_path to generate a Rock Ridge name from.
     interchange_level - The interchange level at which to operate.
     is_dir - Whether this will be a directory or not.
    Returns:
     The Rock Ridge name as a string.
    """

    if iso_path[0] != '/':
        raise pycdlibexception.PyCdlibInvalidInput("iso_path must start with '/'")

    namesplit = utils.split_path(utils.normpath(iso_path))
    iso_name = namesplit.pop()

    if is_dir:
        rr_name = utils.mangle_dir_for_iso9660(iso_name.decode('utf-8'),
                                               interchange_level)
    else:
        basename, ext = utils.mangle_file_for_iso9660(iso_name.decode('utf-8'),
                                                      interchange_level)
        rr_name = '.'.join([basename, ext])

    return rr_name


class PyCdlibISO9660:
    """The class representing the PyCdlib ISO9660 facade."""

    __slots__ = ('pycdlib_obj',)

    def __init__(self, pycdlib_obj):
        # type: (pycdlib.PyCdlib) -> None
        self.pycdlib_obj = pycdlib_obj

    def get_file_from_iso(self, local_path, iso_path):
        # type: (str, str) -> None
        """
        Fetch a single file from the ISO via an absolute ISO path and write it
        out to a local file.

        Parameters:
         local_path - The local file to write to.
         iso_path - The absolute ISO9660 path to lookup on the ISO.
        Returns:
         Nothing.
        """
        self.pycdlib_obj.get_file_from_iso(local_path, iso_path=iso_path)

    def get_file_from_iso_fp(self, outfp, iso_path):
        # type: (BinaryIO, str) -> None
        """
        Fetch a single file from the ISO via an absolute ISO path and write it
        out to the file object.

        Parameters:
         outfp - The file object to write data to.
         iso_path - The absolute ISO9660 path to lookup on the ISO.
        Returns:
         Nothing.
        """
        self.pycdlib_obj.get_file_from_iso_fp(outfp, iso_path=iso_path)

    def add_fp(self, fp, length, iso_path):
        # type: (BinaryIO, int, str) -> None
        """
        Add a file to the ISO.  While using this facade, a file will only be
        added to the ISO9660 context (and by extension, the Rock Ridge
        context).  If the ISO is a Rock Ridge one, then a Rock Ridge name will
        be generated from the ISO path.  For more control over which contexts
        a file shows up in, use the 'add_hard_link' API and/or use the regular
        PyCdlib object (not this facade).  Note that the caller must ensure
        that 'fp' remains open for the lifetime of the PyCdlib object, as the
        PyCdlib class uses the file descriptor internally when writing
        (mastering) the ISO.  To have PyCdlib manage this automatically, use
        'add_file' instead.

        Parameters:
         fp - The file object to use for the contents of the new file.
         length - The length of the data for the new file.
         iso_path - The ISO9660 absolute path to the file destination on the ISO.
        Returns:
         Nothing.
        """
        rr_name = None
        if self.pycdlib_obj.has_rock_ridge():
            rr_name = iso_path_to_rr_name(iso_path, self.pycdlib_obj.interchange_level, False)
        self.pycdlib_obj.add_fp(fp, length, iso_path=iso_path, rr_name=rr_name)

    def add_file(self, filename, iso_path):
        # type: (str, str) -> None
        """
        Add a file to the ISO.  While using this facade, a file will only be
        added to the ISO9660 context (and by extension, the Rock Ridge
        context).  If the ISO is a Rock Ridge one, then a Rock Ridge name will
        be generated from the ISO path.  For more control over which contexts
        in which a file shows up, use the 'add_hard_link' API and/or use the
        regular PyCdlib object (not this facade).

        Parameters:
         filename - The filename to use for the data contents for the new file.
         iso_path - The ISO9660 absolute path to the file destination on the ISO.
        Returns:
         Nothing.
        """
        rr_name = None
        if self.pycdlib_obj.has_rock_ridge():
            rr_name = iso_path_to_rr_name(iso_path, self.pycdlib_obj.interchange_level, False)
        self.pycdlib_obj.add_file(filename, iso_path=iso_path, rr_name=rr_name)

    def add_directory(self, iso_path):
        # type: (str) -> None
        """
        Add a directory to the ISO9660 context (and by extension, the Rock
        Ridge context).  If the ISO is a Rock Ridge one, then a Rock Ridge name
        will be generated from the ISO path.  For more control over which
        contexts in which a directory shows up, use the regular PyCdlib object
        (not this facade).

        Parameters:
         iso_path - The ISO9660 absolute path to use for the directory.
        Returns:
         Nothing.
        """
        rr_name = None
        if self.pycdlib_obj.has_rock_ridge():
            rr_name = iso_path_to_rr_name(iso_path, self.pycdlib_obj.interchange_level, True)
        self.pycdlib_obj.add_directory(iso_path=iso_path, rr_name=rr_name)

    def rm_file(self, iso_path):
        # type: (str) -> None
        """
        Remove a file from the ISO.  This removes the data and the listing of
        the file from all contexts.  Due to some complexities of the ISO format,
        removal of zero-byte files from all contexts does not automatically
        happen, so this method may need to be called on more than one facade for
        zero-byte files.

        Parameters:
         iso_path - The path to the file to remove.
        Returns:
         Nothing.
        """
        self.pycdlib_obj.rm_file(iso_path=iso_path)

    def rm_directory(self, iso_path):
        # type: (str) -> None
        """
        Remove a directory from the ISO.  This removes the directory from
        just the ISO9660 context (and by extension, the Rock Ridge context).
        The directory must be empty.

        Parameters:
         iso_path - The path to the directory to remove.
        Returns:
         Nothing.
        """
        self.pycdlib_obj.rm_directory(iso_path=iso_path)

    def list_children(self, iso_path):
        # type: (str) -> Generator
        """
        Generate a list of all of the file/directory objects in the
        specified location on the ISO.

        Parameters:
         iso_path - The absolute path on the ISO to list the children for.
        Yields:
         Children of this path.
        Returns:
         Nothing.
        """
        return self.pycdlib_obj.list_children(iso_path=iso_path)

    def get_record(self, iso_path):
        # type: (str) -> dr.DirectoryRecord
        """
        Get the directory record for a particular ISO path.

        Parameters:
         iso_path - The absolute path on the ISO9660 filesystem to get the
                    record for.
        Returns:
         A dr.DirectoryRecord object that represents the path.
        """
        ret = self.pycdlib_obj.get_record(iso_path=iso_path)
        if not isinstance(ret, dr.DirectoryRecord):
            raise pycdlibexception.PyCdlibInternalError('Invalid return type from get_record')
        return ret

    def walk(self, iso_path):
        # type: (str) -> Generator
        """
        Walk the entries on the ISO, starting at the given ISO path.  Similar
        to os.walk(), yield a 3-tuple of (path-to-here, dirlist, filelist)
        for each directory level.

        Parameters:
         iso_path - The absolute ISO path to the starting entry on the ISO.
        Yields:
         3-tuples of (path-to-here, dirlist, filelist)
        Returns:
         Nothing.
        """
        return self.pycdlib_obj.walk(iso_path=iso_path)

    def open_file_from_iso(self, iso_path):
        # type: (str) -> pycdlibio.PyCdlibIO
        """
        Open a file for reading in a context manager.  This allows the user to
        operate on the file in user-defined chunks (utilizing the read() method
        of the returned context manager).

        Parameters:
         iso_path - The absolute ISO path to the file on the ISO.
        Returns:
         A PyCdlibIO object allowing access to the file.
        """
        return self.pycdlib_obj.open_file_from_iso(iso_path=iso_path)


class PyCdlibJoliet:
    """The class representing the PyCdlib Joliet facade."""

    __slots__ = ('pycdlib_obj',)

    def __init__(self, pycdlib_obj):
        # type: (pycdlib.PyCdlib) -> None
        if not pycdlib_obj.has_joliet():
            raise pycdlibexception.PyCdlibInvalidInput('Can only instantiate a Joliet facade for a Joliet ISO')
        self.pycdlib_obj = pycdlib_obj

    def get_file_from_iso(self, local_path, joliet_path):
        # type: (str, str) -> None
        """
        Fetch a single file from the ISO via an absolute Joliet path and write
        it out to a local file.

        Parameters:
         local_path - The local file to write to.
         joliet_path - The absolute Joliet path to lookup on the ISO (exclusive
                       with iso_path, rr_path, and udf_path).
        Returns:
         Nothing.
        """
        self.pycdlib_obj.get_file_from_iso(local_path, joliet_path=joliet_path)

    def get_file_from_iso_fp(self, outfp, joliet_path):
        # type: (BinaryIO, str) -> None
        """
        Fetch a single file from the ISO via an absolute Joliet path and write
        it out to the file object.

        Parameters:
         outfp - The file object to write data to.
         joliet_path - The absolute Joliet path to lookup on the ISO.
        Returns:
         Nothing.
        """
        self.pycdlib_obj.get_file_from_iso_fp(outfp, joliet_path=joliet_path)

    def add_fp(self, fp, length, joliet_path):
        # type: (BinaryIO, int, str) -> None
        """
        Add a file to the ISO.  While using this facade, a file will only be
        added to the Joliet context.  For more control over which contexts
        a file shows up in, use the 'add_hard_link' API and/or use the regular
        PyCdlib object (not this facade).  Note that the caller must ensure
        that 'fp' remains open for the lifetime of the PyCdlib object, as the
        PyCdlib class uses the file descriptor internally when writing
        (mastering) the ISO.  To have PyCdlib manage this automatically, use
        'add_file' instead.

        Parameters:
         fp - The file object to use for the contents of the new file.
         length - The length of the data for the new file.
         joliet_path - The Joliet absolute path to the file destination on the ISO.
        Returns:
         Nothing.
        """

        # Since Joliet is a totally separate namespace from ISO9660/Rock Ridge,
        # there is no reliable way to find the ISO9660 path from the Joliet
        # path.  Thus, when using the Joliet facade, any updates to Joliet will
        # only be reflected on the Joliet side of the ISO.
        self.pycdlib_obj.add_fp(fp, length, joliet_path=joliet_path)

    def add_file(self, filename, joliet_path):
        # type: (str, str) -> None
        """
        Add a file to the ISO.  While using this facade, a file will only be
        added to the Joliet context.  For more control over which contexts
        in which a file shows up, use the 'add_hard_link' API and/or use the
        regular PyCdlib object (not this facade).

        Parameters:
         filename - The filename to use for the data contents for the new file.
         joliet_path - The Joliet absolute path to the file destination on the ISO.
        Returns:
         Nothing.
        """

        # Since Joliet is a totally separate namespace from ISO9660/Rock Ridge,
        # there is no reliable way to find the ISO9660 path from the Joliet
        # path.  Thus, when using the Joliet facade, any updates to Joliet will
        # only be reflected on the Joliet side of the ISO.
        self.pycdlib_obj.add_file(filename, joliet_path=joliet_path)

    def add_directory(self, joliet_path):
        # type: (str) -> None
        """
        Add a directory to the Joliet context.  For more control over which
        contexts in which a directory shows up, use the regular PyCdlib object
        (not this facade).

        Parameters:
         iso_path - The ISO9660 absolute path to use for the directory.
        Returns:
         Nothing.
        """
        self.pycdlib_obj.add_directory(joliet_path=joliet_path)

    def rm_file(self, joliet_path):
        # type: (str) -> None
        """
        Remove a file from the ISO.  This removes the data and the listing of
        the file from all contexts.  Due to some complexities of the ISO format,
        removal of zero-byte files from all contexts does not automatically
        happen, so this method may need to be called on more than one facade for
        zero-byte files.

        Parameters:
         joliet_path - The path to the file to remove.
        Returns:
         Nothing.
        """
        self.pycdlib_obj.rm_file(joliet_path=joliet_path)

    def rm_directory(self, joliet_path):
        # type: (str) -> None
        """
        Remove a directory from the ISO.  This removes the directory from
        just the Joliet context.  The directory must be empty.

        Parameters:
         joliet_path - The path to the directory to remove.
        Returns:
         Nothing.
        """
        self.pycdlib_obj.rm_directory(joliet_path=joliet_path)

    def list_children(self, joliet_path):
        # type: (str) -> Generator
        """
        Generate a list of all of the file/directory objects in the
        specified location on the ISO.

        Parameters:
         joliet_path - The absolute path on the Joliet portion of the ISO to list the children for.
        Yields:
         Children of this path.
        Returns:
         Nothing.
        """
        return self.pycdlib_obj.list_children(joliet_path=joliet_path)

    def get_record(self, joliet_path):
        # type: (str) -> dr.DirectoryRecord
        """
        Get the directory record for a particular Joliet path.

        Parameters:
         joliet_path - The absolute path on the Joliet filesystem to get the
                    record for.
        Returns:
         A dr.DirectoryRecord object that represents the path.
        """
        ret = self.pycdlib_obj.get_record(joliet_path=joliet_path)
        if not isinstance(ret, dr.DirectoryRecord):
            raise pycdlibexception.PyCdlibInternalError('Invalid return type from get_record')
        return ret

    def walk(self, joliet_path):
        # type: (str) -> Generator
        """
        Walk the entries on the Joliet portion of the ISO, starting at the given
        Joliet path.  Similar to os.walk(), yield a 3-tuple of
        (path-to-here, dirlist, filelist) for each directory level.

        Parameters:
         joliet_path - The absolute Joliet path to the starting entry on the ISO.
        Yields:
         3-tuples of (path-to-here, dirlist, filelist)
        Returns:
         Nothing.
        """
        return self.pycdlib_obj.walk(joliet_path=joliet_path)

    def open_file_from_iso(self, joliet_path):
        # type: (str) -> pycdlibio.PyCdlibIO
        """
        Open a file for reading in a context manager.  This allows the user to
        operate on the file in user-defined chunks (utilizing the read() method
        of the returned context manager).

        Parameters:
         joliet_path - The absolute Joliet path to the file on the ISO.
        Returns:
         A PyCdlibIO object allowing access to the file.
        """
        return self.pycdlib_obj.open_file_from_iso(joliet_path=joliet_path)


class PyCdlibRockRidge:
    """The class representing the PyCdlib Rock Ridge facade."""

    __slots__ = ('pycdlib_obj',)

    def __init__(self, pycdlib_obj):
        # type: (pycdlib.PyCdlib) -> None
        if not pycdlib_obj.has_rock_ridge():
            raise pycdlibexception.PyCdlibInvalidInput('Can only instantiate a Rock Ridge facade for a Rock Ridge ISO')
        self.pycdlib_obj = pycdlib_obj

    def get_file_from_iso(self, local_path, rr_path):
        # type: (str, str) -> None
        """
        Fetch a single file from the ISO via an absolute Rock Ridge path and
        write it out to a local file.

        Parameters:
         local_path - The local file to write to.
         rr_path - The absolute Rock Ridge path to lookup on the ISO.
        Returns:
         Nothing.
        """
        self.pycdlib_obj.get_file_from_iso(local_path, rr_path=rr_path)

    def get_file_from_iso_fp(self, outfp, rr_path):
        # type: (BinaryIO, str) -> None
        """
        Fetch a single file from the ISO via an absolute Rock Ridge path and
        write it out to the file object.

        Parameters:
         outfp - The file object to write data to.
         joliet_path - The absolute Joliet path to lookup on the ISO.
        Returns:
         Nothing.
        """
        self.pycdlib_obj.get_file_from_iso_fp(outfp, rr_path=rr_path)

    def _rr_path_to_iso_path(self, rr_path):
        # type: (str) -> str
        """
        An internal method to convert an ISO9660 path to a Rock Ridge one. This
        is accomplished by finding the Rock Ridge directory record, then
        reconstructing the ISO path by walking up to the root.

        Parameters:
         rr_path - The Rock Ridge path to generate an ISO9660 path for.
        Returns:
         The ISO9660 path corresponding to the Rock Ridge path.
        """
        if rr_path[0] != '/':
            raise pycdlibexception.PyCdlibInvalidInput("rr_path must start with '/'")

        record = self.pycdlib_obj._find_rr_record(utils.normpath(rr_path))  # pylint: disable=protected-access
        if record.is_root:
            iso_path = b'/'
        else:
            iso_path = b''
            parent = record  # type: Optional[dr.DirectoryRecord]
            while parent is not None:
                if not parent.is_root:
                    iso_path = b'/' + parent.file_identifier() + iso_path
                parent = parent.parent

        return iso_path.decode('utf-8')

    def _rr_path_to_iso_path_and_rr_name(self, rr_path, is_dir):
        # type: (str, bool) -> Tuple[str, str]
        """
        An internal method to split a Rock Ridge absolute path into an absolute
        ISO9660 path and a Rock Ridge name.  This is accomplished by finding the
        Rock Ridge directory record of the parent, then reconstructing the ISO
        parent path by walking up to the root.

        Parameters:
         rr_path - The absolute Rock Ridge path to generate for.
         is_dir - Whether this path is a directory or a file.
        Returns:
         A tuple where the first element is the absolute ISO9660 path of the
         parent, and the second element is the Rock Ridge name.
        """
        if rr_path[0] != '/':
            raise pycdlibexception.PyCdlibInvalidInput("rr_path must start with '/'")

        namesplit = utils.split_path(utils.normpath(rr_path))
        rr_name = namesplit.pop()
        rr_parent_path = b'/' + b'/'.join(namesplit)
        parent_record = self.pycdlib_obj._find_rr_record(rr_parent_path)  # pylint: disable=protected-access
        if parent_record.is_root:
            iso_parent_path = b'/'
        else:
            iso_parent_path = b''
            parent = parent_record  # type: Optional[dr.DirectoryRecord]
            while parent is not None:
                if not parent.is_root:
                    iso_parent_path = b'/' + parent.file_identifier() + iso_parent_path
                parent = parent.parent

        if is_dir:
            iso_name = utils.mangle_dir_for_iso9660(rr_name.decode('utf-8'),
                                                    self.pycdlib_obj.interchange_level)
        else:
            basename, ext = utils.mangle_file_for_iso9660(rr_name.decode('utf-8'),
                                                          self.pycdlib_obj.interchange_level)
            iso_name = '.'.join([basename, ext])

        iso_path = iso_parent_path.decode('utf-8') + '/' + iso_name

        return iso_path, rr_name.decode('utf-8')

    def add_fp(self, fp, length, rr_path, file_mode):
        # type: (BinaryIO, int, str, int) -> None
        """
        Add a file to the ISO.  While using this facade, a file will only be
        added to the Rock Ridge context (and by extension, the ISO9660
        context).  For more control over which contexts a file shows up in, use
        the 'add_hard_link' API and/or use the regular PyCdlib object (not this
        facade).  Note that the caller must ensure that 'fp' remains open for the
        lifetime of the PyCdlib object, as the PyCdlib class uses the file
        descriptor internally when writing (mastering) the ISO.  To have PyCdlib
        manage this automatically, use 'add_file' instead.

        Parameters:
         fp - The file object to use for the contents of the new file.
         length - The length of the data for the new file.
         rr_path - The Rock Ridge absolute path to the file destination on the
                   ISO.
         file_mode - The POSIX file_mode to apply to this file.  If this is None
                     (the default), the permissions from the original file are
                     used.
        Returns:
         Nothing.
        """
        iso_path, rr_name = self._rr_path_to_iso_path_and_rr_name(rr_path, False)
        self.pycdlib_obj.add_fp(fp, length, iso_path, rr_name=rr_name,
                                file_mode=file_mode)

    def add_file(self, filename, rr_path, file_mode):
        # type: (str, str, int) -> None
        """
        Add a file to the ISO.  While using this facade, a file will only be
        added to the Rock Ridge context (and by extension, the ISO9660
        context).  For more control over which contexts in which a file shows up,
        use the 'add_hard_link' API and/or use the regular PyCdlib object (not
        this facade).

        Parameters:
         filename - The filename to use for the data contents for the new file.
         rr_path - The Rock Ridge absolute path to the file destination on the ISO.
         file_mode - The POSIX file_mode to apply to this file.  If this is None
                     (the default), the permissions from the original file are
                     used.
        Returns:
         Nothing.
        """
        iso_path, rr_name = self._rr_path_to_iso_path_and_rr_name(rr_path, False)
        self.pycdlib_obj.add_file(filename, iso_path, rr_name=rr_name,
                                  file_mode=file_mode)

    def add_directory(self, rr_path, file_mode):
        # type: (str, int) -> None
        """
        Add a directory to the Rock Ridge context (and by extension, the ISO9660
        context).  For more control over which contexts in which a directory
        shows up, use the regular PyCdlib object (not this facade).

        Parameters:
         rr_path - The Rock Ridge absolute path to use for the directory.
         file_mode - The POSIX file_mode to apply to this file.  If this is None
                     (the default), the permissions from the original file are
                     used.
        Returns:
         Nothing.
        """
        iso_path, rr_name = self._rr_path_to_iso_path_and_rr_name(rr_path, True)
        self.pycdlib_obj.add_directory(iso_path=iso_path, rr_name=rr_name,
                                       file_mode=file_mode)

    def rm_file(self, rr_path):
        # type: (str) -> None
        """
        Remove a file from the ISO.  This removes the data and the listing of
        the file from all contexts.  Due to some complexities of the ISO format,
        removal of zero-byte files from all contexts does not automatically
        happen, so this method may need to be called on more than one facade for
        zero-byte files.

        Parameters:
         rr_path - The path to the file to remove.
        Returns:
         Nothing.
        """
        self.pycdlib_obj.rm_file(self._rr_path_to_iso_path(rr_path))

    def rm_directory(self, rr_path):
        # type: (str) -> None
        """
        Remove a directory from the ISO.  This removes the directory from
        just the Rock Ridge context (and by extension, the ISO9660 context).
        The directory must be empty.

        Parameters:
         rr_path - The path to the directory to remove.
        Returns:
         Nothing.
        """
        self.pycdlib_obj.rm_directory(self._rr_path_to_iso_path(rr_path))

    def add_symlink(self, rr_symlink_path, rr_target_path):
        # type: (str, str) -> None
        """
        Add a symlink from rr_symlink_name to the rr_path.

        Parameters:
         rr_symlink_path - The absolute Rock Ridge path of the symlink itself on
                           the ISO.
         rr_target_path - The Rock Ridge path of the entry on the ISO that the
                          symlink points to.
        Returns:
         Nothing.
        """
        symlink_path, rr_symlink_name = self._rr_path_to_iso_path_and_rr_name(rr_symlink_path,
                                                                              False)
        self.pycdlib_obj.add_symlink(symlink_path,
                                     rr_symlink_name=rr_symlink_name,
                                     rr_path=rr_target_path)

    def list_children(self, rr_path):
        # type: (str) -> Generator
        """
        Generate a list of all of the file/directory objects in the
        specified location on the ISO.

        Parameters:
         rr_path - The absolute path on the ISO to list the children for.
        Yields:
         Children of this path.
        Returns:
         Nothing.
        """
        return self.pycdlib_obj.list_children(rr_path=rr_path)

    def get_record(self, rr_path):
        # type: (str) -> dr.DirectoryRecord
        """
        Get the directory record for a particular ISO path.

        Parameters:
         rr_path - The absolute path on the Rock Ridge filesystem to get the
                   record for.
        Returns:
         A dr.DirectoryRecord object that represents the path.
        """
        ret = self.pycdlib_obj.get_record(rr_path=rr_path)
        if not isinstance(ret, dr.DirectoryRecord):
            raise pycdlibexception.PyCdlibInternalError('Invalid return type from get_record')
        return ret

    def walk(self, rr_path):
        # type: (str) -> Generator
        """
        Walk the entries on the ISO, starting at the given Rock Ridge path.
        Similar to os.walk(), yield a 3-tuple of
        (path-to-here, dirlist, filelist) for each directory level.

        Parameters:
         rr_path - The absolute Rock Ridge path to the starting entry on the ISO.
        Yields:
         3-tuples of (path-to-here, dirlist, filelist)
        Returns:
         Nothing.
        """
        return self.pycdlib_obj.walk(rr_path=rr_path)

    def open_file_from_iso(self, rr_path):
        # type: (str) -> pycdlibio.PyCdlibIO
        """
        Open a file for reading in a context manager.  This allows the user to
        operate on the file in user-defined chunks (utilizing the read() method
        of the returned context manager).

        Parameters:
         rr_path - The absolute Rock Ridge path to the file on the ISO.
        Returns:
         A PyCdlibIO object allowing access to the file.
        """
        return self.pycdlib_obj.open_file_from_iso(rr_path=rr_path)


class PyCdlibUDF:
    """The class representing the PyCdlib UDF facade."""

    __slots__ = ('pycdlib_obj',)

    def __init__(self, pycdlib_obj):
        # type: (pycdlib.PyCdlib) -> None
        if not pycdlib_obj.has_udf():
            raise pycdlibexception.PyCdlibInvalidInput('Can only instantiate a UDF facade for a UDF ISO')
        self.pycdlib_obj = pycdlib_obj

    def get_file_from_iso(self, local_path, udf_path):
        # type: (str, str) -> None
        """
        Fetch a single file from the ISO via an absolute UDF path and write it
        out to a local file.

        Parameters:
         local_path - The local file to write to.
         udf_path - The absolute UDF path to lookup on the ISO.
        Returns:
         Nothing.
        """
        self.pycdlib_obj.get_file_from_iso(local_path, udf_path=udf_path)

    def get_file_from_iso_fp(self, outfp, udf_path):
        # type: (BinaryIO, str) -> None
        """
        Fetch a single file from the ISO via an absolute UDF path and write it
        out to the file object.

        Parameters:
         outfp - The file object to write data to.
         udf_path - The absolute UDF path to lookup on the ISO.
        Returns:
         Nothing.
        """
        self.pycdlib_obj.get_file_from_iso_fp(outfp, udf_path=udf_path)

    def add_fp(self, fp, length, udf_path):
        # type: (BinaryIO, int, str) -> None
        """
        Add a file to the ISO.  While using this facade, a file will only be
        added to the UDF context.  For more control over which contexts
        a file shows up in, use the 'add_hard_link' API and/or use the regular
        PyCdlib object (not this facade).  Note that the caller must ensure
        that 'fp' remains open for the lifetime of the PyCdlib object, as the
        PyCdlib class uses the file descriptor internally when writing
        (mastering) the ISO.  To have PyCdlib manage this automatically, use
        'add_file' instead.

        Parameters:
         fp - The file object to use for the contents of the new file.
         length - The length of the data for the new file.
         udf_path - The UDF absolute path to the file destination on the ISO.
        Returns:
         Nothing.
        """
        self.pycdlib_obj.add_fp(fp, length, udf_path=udf_path)

    def add_file(self, filename, udf_path):
        # type: (str, str) -> None
        """
        Add a file to the ISO.  While using this facade, a file will only be
        added to the UDF context.  For more control over which contexts
        in which a file shows up, use the 'add_hard_link' API and/or use the
        regular PyCdlib object (not this facade).

        Parameters:
         filename - The filename to use for the data contents for the new file.
         udf_path - The UDF absolute path to the file destination on the ISO.
        Returns:
         Nothing.
        """
        self.pycdlib_obj.add_file(filename, udf_path=udf_path)

    def add_directory(self, udf_path):
        # type: (str) -> None
        """
        Add a directory to the UDF context.  For more control over which
        contexts in which a directory shows up, use the regular PyCdlib object
        (not this facade).

        Parameters:
         udf_path - The UDF absolute path to use for the directory.
        Returns:
         Nothing.
        """
        self.pycdlib_obj.add_directory(udf_path=udf_path)

    def rm_file(self, udf_path):
        # type: (str) -> None
        """
        Remove a file from the ISO.  This removes the data and the listing of
        the file from all contexts.  Due to some complexities of the ISO format,
        removal of zero-byte files from all contexts does not automatically
        happen, so this method may need to be called on more than one facade for
        zero-byte files.

        Parameters:
         udf_path - The path to the file to remove.
        Returns:
         Nothing.
        """
        self.pycdlib_obj.rm_file(udf_path=udf_path)

    def rm_directory(self, udf_path):
        # type: (str) -> None
        """
        Remove a directory from the ISO.  This removes the directory from
        just the UDF context.  The directory must be empty.

        Parameters:
         udf_path - The path to the directory to remove.
        Returns:
         Nothing.
        """
        self.pycdlib_obj.rm_directory(udf_path=udf_path)

    def add_symlink(self, udf_symlink_path, udf_target):
        # type: (str, str) -> None
        """
        Add a symlink from udf_symlink_path to udf_target.

        Parameters:
         udf_symlink_path - The UDF path of the symlink itself on the ISO.
         udf_target - The UDF name of the entry on the ISO that the symlink
                      points to.
        Returns:
         Nothing.
        """
        self.pycdlib_obj.add_symlink(udf_symlink_path=udf_symlink_path,
                                     udf_target=udf_target)

    def list_children(self, udf_path):
        # type: (str) -> Generator
        """
        Generate a list of all of the file/directory objects in the
        specified location on the ISO.

        Parameters:
         udf_path - The absolute path on the ISO to list the children for.
        Yields:
         Children of this path.
        Returns:
         Nothing.
        """
        return self.pycdlib_obj.list_children(udf_path=udf_path)

    def get_record(self, udf_path):
        # type: (str) -> udfmod.UDFFileEntry
        """
        Get the directory record for a particular UDF path.

        Parameters:
         udf_path - The absolute path on the UDF filesystem to get the
                    record for.
        Returns:
         A udf.UDFFileEntry object that represents the path.
        """
        ret = self.pycdlib_obj.get_record(udf_path=udf_path)
        if not isinstance(ret, udfmod.UDFFileEntry):
            raise pycdlibexception.PyCdlibInternalError('Invalid return type from get_record')
        return ret

    def walk(self, udf_path):
        # type: (str) -> Generator
        """
        Walk the entries on the ISO, starting at the given UDF path.  Similar
        to os.walk(), yield a 3-tuple of (path-to-here, dirlist, filelist)
        for each directory level.

        Parameters:
         udf_path - The absolute UDF path to the starting entry on the ISO.
        Yields:
         3-tuples of (path-to-here, dirlist, filelist)
        Returns:
         Nothing.
        """
        return self.pycdlib_obj.walk(udf_path=udf_path)

    def open_file_from_iso(self, udf_path):
        # type: (str) -> pycdlibio.PyCdlibIO
        """
        Open a file for reading in a context manager.  This allows the user to
        operate on the file in user-defined chunks (utilizing the read() method
        of the returned context manager).

        Parameters:
         udf_path - The absolute UDF path to the file on the ISO.
        Returns:
         A PyCdlibIO object allowing access to the file.
        """
        return self.pycdlib_obj.open_file_from_iso(udf_path=udf_path)
