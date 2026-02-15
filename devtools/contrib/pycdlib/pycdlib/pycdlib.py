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

"""Main PyCdlib class and support classes and utilities."""
# fmt: off

import bisect
import collections
import functools
import inspect
import io
import os
import struct
import sys
import time

from pycdlib import dr
from pycdlib import eltorito
from pycdlib import facade
from pycdlib import headervd
from pycdlib import inode
from pycdlib import isohybrid
from pycdlib import path_table_record
from pycdlib import pycdlibexception
from pycdlib import pycdlibio
from pycdlib import udf as udfmod
from pycdlib import utils

# For mypy annotations
from typing import Any, BinaryIO, Callable, Deque, Dict, Generator, IO, List, Optional, Tuple, Union, cast

# There are a number of specific ways that numerical data is stored in the
# ISO9660/Ecma-119 standard.  In the text these are reference by the section
# number they are stored in.  A brief synopsis:
#
# 7.1.1 - 8-bit number
# 7.2.3 - 16-bit number, stored first as little-endian then as big-endian (4 bytes total)
# 7.3.1 - 32-bit number, stored as little-endian
# 7.3.2 - 32-bit number ,stored as big-endian
# 7.3.3 - 32-bit number, stored first as little-endian then as big-endian (8 bytes total)

# We allow A-Z, 0-9, and _ as "d1" characters.  The below is the fastest way to
# build that list as integers.
_allowed_d1_characters = set(tuple(range(65, 91)) + tuple(range(48, 58)) + tuple((ord(b'_'),)))


def _check_d1_characters(name):
    # type: (bytes) -> None
    """
    A function to check that a name only uses d1 characters as defined by ISO9660.

    Parameters:
     name - The name to check.
    Returns:
     Nothing.
    """
    for char in bytearray(name):
        if char not in _allowed_d1_characters:
            raise pycdlibexception.PyCdlibInvalidInput('ISO9660 filenames must consist of characters A-Z, 0-9, and _')


def _split_iso9660_filename(fullname):
    # type: (bytes) -> Tuple[bytes, bytes, bytes]
    """
    A function to split an ISO 9660 filename into its constituent parts.  This
    is the name, the extension, and the version number.

    Parameters:
     fullname - The name to split.
    Returns:
     A tuple containing the name, extension, and version.
    """
    namesplit = fullname.split(b';')
    version = b''
    if len(namesplit) > 1:
        version = namesplit.pop()

    rest = b';'.join(namesplit)

    dotsplit = rest.split(b'.')
    if len(dotsplit) == 1:
        name = dotsplit[0]
        extension = b''
    else:
        name = b'.'.join(dotsplit[:-1])
        extension = dotsplit[-1]

    return (name, extension, version)


def _check_iso9660_filename(fullname, interchange_level):
    # type: (bytes, int) -> None
    """
    A function to check that a file identifier conforms to the ISO9660 rules
    for a particular interchange level.

    Parameters:
     fullname - The name to check.
     interchange_level - The interchange level to check against.
    Returns:
     Nothing.
    """

    # Ensure the filename is valid according to Ecma-119 7.5.

    (name, extension, version) = _split_iso9660_filename(fullname)

    # Ecma-119 says that filenames must end with a semicolon-number, but we have
    # found ISOs in the wild (Ubuntu 14.04 Desktop i386) that do not follow
    # this.  Thus we allow for names both with and without the semi+version.

    # Ecma-119 says that filenames must have a version number, but we have
    # found ISOs in the wild (FreeBSD 10.1 amd64) that do not have any version
    # number.  Allow for this.

    if version != b'' and (int(version) < 1 or int(version) > 32767):
        raise pycdlibexception.PyCdlibInvalidInput('ISO9660 filenames must have a version between 1 and 32767')

    # Ecma-119 section 7.5.1 specifies that filenames must have at least one
    # character in either the name or the extension.
    if not name and not extension:
        raise pycdlibexception.PyCdlibInvalidInput('ISO9660 filenames must have a non-empty name or extension')

    if b';' in name or b';' in extension:
        raise pycdlibexception.PyCdlibInvalidInput('ISO9660 filenames must contain exactly one semicolon')

    if interchange_level == 1:
        # According to Ecma-119, section 10.1, at level 1 the filename can
        # only be up to 8 d-characters or d1-characters, and the extension can
        # only be up to 3 d-characters or 3 d1-characters.
        if len(name) > 8 or len(extension) > 3:
            raise pycdlibexception.PyCdlibInvalidInput('ISO9660 filenames at interchange level 1 cannot have more than 8 characters or 3 characters in the extension')
    else:
        # For all other interchange levels, the maximum filename length is
        # specified in Ecma-119 7.5.2.  However, we have found ISOs in the wild
        # (Ubuntu 14.04 Desktop i386) that don't conform to this.
        pass

    # Ecma-119 section 7.5.1 says that the file name and extension each contain
    # zero or more d-characters or d1-characters.  While the definition of
    # d-characters and d1-characters is not specified in Ecma-119,
    # http://wiki.osdev.org/ISO_9660 suggests that this consists of A-Z, 0-9, _
    # which seems to correlate with empirical evidence.
    if interchange_level < 4:
        _check_d1_characters(name)
        _check_d1_characters(extension)


def _check_iso9660_directory(fullname, interchange_level):
    # type: (bytes, int) -> None
    """
    A function to check that an directory identifier conforms to the ISO9660
    rules for a particular interchange level.

    Parameters:
     fullname - The name to check.
     interchange_level - The interchange level to check against.
    Returns:
     Nothing.
    """

    # Ensure the directory name is valid according to Ecma-119 7.6.

    # Ecma-119 section 7.6.1 says that a directory identifier needs at least one
    # character
    if not fullname:
        raise pycdlibexception.PyCdlibInvalidInput('ISO9660 directory names must be at least 1 character long')

    maxlen = float('inf')
    if interchange_level == 1:
        # Ecma-119 section 10.1 says that directory identifiers lengths cannot
        # exceed 8 at interchange level 1.
        maxlen = 8
    elif interchange_level in (2, 3):
        # Ecma-119 section 7.6.3 says that directory identifiers lengths cannot
        # exceed 207.
        maxlen = 207
    # for interchange_level 4, we allow any length

    if len(fullname) > maxlen:
        raise pycdlibexception.PyCdlibInvalidInput('ISO9660 directory names at interchange level %d cannot exceed %d characters' % (interchange_level, maxlen))

    # Ecma-119 section 7.6.1 says that directory names consist of one or more
    # d-characters or d1-characters.  While the definition of d-characters and
    # d1-characters is not specified in Ecma-119,
    # http://wiki.osdev.org/ISO_9660 suggests that this consists of A-Z, 0-9, _
    # which seems to correlate with empirical evidence.
    if interchange_level < 4:
        _check_d1_characters(fullname)


def _interchange_level_from_filename(fullname):
    # type: (bytes) -> int
    """
    A function to determine the ISO interchange level from the filename.
    In theory, there are 3 levels, but in practice we only deal with level 1
    and level 3.

    Parameters:
     fullname - The name to use to determine the interchange level.
    Returns:
     The interchange level determined from this filename.
    """
    (name, extension, version) = _split_iso9660_filename(fullname)

    interchange_level = 1

    if version != b'' and (int(version) < 1 or int(version) > 32767):
        interchange_level = 3

    if b';' in name or b';' in extension:
        interchange_level = 3

    if len(name) > 8 or len(extension) > 3:
        interchange_level = 3

    try:
        _check_d1_characters(name)
        _check_d1_characters(extension)
    except pycdlibexception.PyCdlibInvalidInput:
        interchange_level = 3

    return interchange_level


def _interchange_level_from_directory(name):
    # type: (bytes) -> int
    """
    A function to determine the ISO interchange level from the directory name.
    In theory, there are 3 levels, but in practice we only deal with level 1
    and level 3.

    Parameters:
     name - The name to use to determine the interchange level.
    Returns:
     The interchange level determined from this filename.
    """
    interchange_level = 1
    if len(name) > 8:
        interchange_level = 3

    try:
        _check_d1_characters(name)
    except pycdlibexception.PyCdlibInvalidInput:
        interchange_level = 3

    return interchange_level


def _reassign_vd_dirrecord_extents(vd, current_extent):
    # type: (headervd.PrimaryOrSupplementaryVD, int) -> Tuple[int, List[inode.Inode]]
    """
    An internal helper method for reassign_extents that assigns extents to
    directory records for the passed in Volume Descriptor.  The current
    extent is passed in, and this function returns the extent after the
    last one it assigned.

    Parameters:
     vd - The volume descriptor on which to operate.
     current_extent - The current extent before assigning extents to the
                      volume descriptor directory records.
    Returns:
     The current extent after assigning extents to the volume descriptor
     directory records.
    """
    log_block_size = vd.logical_block_size()

    root_dir_record = vd.root_directory_record()
    root_dir_record.set_data_location(current_extent, 0)
    current_extent += utils.ceiling_div(root_dir_record.data_length,
                                        log_block_size)

    # Walk through the list, assigning extents to all of the directories.
    child_link_recs = []
    parent_link_recs = []
    file_list = []
    ptr_index = 1
    dirs = collections.deque([root_dir_record])
    while dirs:
        dir_record = dirs.popleft()

        if dir_record.is_root:
            # The root directory record doesn't need an extent assigned,
            # so just add its children to the list and continue on
            for child in dir_record.children:
                if child.ptr is not None:
                    child.ptr.update_parent_directory_number(ptr_index)
            ptr_index += 1
            dirs.extend(dir_record.children)
            continue

        dir_record_parent = dir_record.parent

        if dir_record_parent is None:
            raise pycdlibexception.PyCdlibInternalError('Parent of record is empty, this should never happen')

        if dir_record.is_dot():
            dir_record.set_data_location(dir_record_parent.extent_location(), 0)
            continue

        dir_record_rock_ridge = dir_record.rock_ridge

        if dir_record.is_dotdot():
            if dir_record_parent.is_root:
                # Special case of the root directory record.  In this case, we
                # set the dotdot extent location to the same as the root.
                dir_record.set_data_location(dir_record_parent.extent_location(),
                                             0)
                continue

            if dir_record_parent.parent is None:
                raise pycdlibexception.PyCdlibInternalError('Grandparent of record is empty, this should never happen')
            dir_record.set_data_location(dir_record_parent.parent.extent_location(),
                                         0)

            # Now that we've set the data location, move around the Rock Ridge
            # links if necessary.
            if dir_record_rock_ridge is not None:
                if dir_record_rock_ridge.parent_link is not None:
                    parent_link_recs.append(dir_record)

                if dir_record_parent.rock_ridge is not None:
                    if dir_record_parent.parent is not None:
                        if dir_record_parent.parent.is_root:
                            source_dr = dir_record_parent.parent.children[0]
                        else:
                            source_dr = dir_record_parent.parent

                        if source_dr is None or source_dr.rock_ridge is None:
                            raise pycdlibexception.PyCdlibInternalError('Expected directory record to have Rock Ridge')
                        dir_record_rock_ridge.copy_file_links(source_dr.rock_ridge)
            continue

        if dir_record.is_dir():
            dir_record.set_data_location(current_extent, current_extent)
            for child in dir_record.children:
                if child.ptr is not None:
                    child.ptr.update_parent_directory_number(ptr_index)
            ptr_index += 1
            if dir_record_rock_ridge is None or not dir_record_rock_ridge.child_link_record_exists():
                current_extent += utils.ceiling_div(dir_record.data_length,
                                                    log_block_size)
            dirs.extend(dir_record.children)
        else:
            if dir_record.data_length == 0 or (dir_record_rock_ridge is not None and (dir_record_rock_ridge.child_link_record_exists() or dir_record_rock_ridge.is_symlink())):
                # If this is a child link record, the extent location really
                # doesn't matter, since it is fake.  We set it to zero.
                dir_record.set_data_location(0, 0)
            else:
                if dir_record.inode is not None:
                    file_list.append(dir_record.inode)

        if dir_record_rock_ridge is not None:
            if dir_record_rock_ridge.dr_entries.ce_record is not None and dir_record_rock_ridge.ce_block is not None:
                if dir_record_rock_ridge.ce_block.extent_location() < 0:
                    dir_record_rock_ridge.ce_block.set_extent_location(current_extent)
                    current_extent += 1
                dir_record_rock_ridge.dr_entries.ce_record.update_extent(dir_record_rock_ridge.ce_block.extent_location())
            if dir_record_rock_ridge.cl_to_moved_dr is not None:
                child_link_recs.append(dir_record)

    # After we have reshuffled the extents, update the rock ridge links.
    for ch in child_link_recs:
        if ch.rock_ridge is not None:
            ch.rock_ridge.child_link_update_from_dirrecord()

    for p in parent_link_recs:
        if p.rock_ridge is not None:
            p.rock_ridge.parent_link_update_from_dirrecord()

    return current_extent, file_list


def _check_path_depth(iso_path):
    # type: (bytes) -> None
    """
    An internal method to take a fully-qualified iso path and check whether
    it meets the path depth requirements of ISO9660/Ecma-119.

    Parameters:
     iso_path - The path to check.
    Returns:
     Nothing.
    """
    if len(utils.split_path(iso_path)) > 7:
        # Ecma-119 Section 6.8.2.1 says that the number of levels in the
        # hierarchy shall not exceed eight.  Since the root directory must be
        # at level 1 by itself, the effective maximum hierarchy depth is 7.
        raise pycdlibexception.PyCdlibInvalidInput('Directory levels too deep (maximum is 7)')


def _yield_children(rec, rr):
    # type: (dr.DirectoryRecord, bool) -> Generator
    """
    An internal function to gather and yield all of the children of a Directory
    Record.

    Parameters:
     rec - The Directory Record to get all of the children from (must be a
           directory).
     rr - Whether to follow Rock Ridge relocation entries or not.
    Yields:
     Children of this Directory Record.
    Returns:
     Nothing.
    """
    if not rec.is_dir():
        raise pycdlibexception.PyCdlibInvalidInput('Record is not a directory!')

    last = b''
    for child in rec.children:
        # If the filename of this child is the same as the last one, skip the
        # child.  This can happen if there is a very large file with more than
        # one directory entry.
        fi = child.file_identifier()
        if fi == last:
            continue
        last = fi

        skip_child = False
        if rr:
            if child.rock_ridge is not None:
                for inner_child in child.children:
                    if inner_child.is_dotdot():
                        if inner_child.rock_ridge is not None and inner_child.rock_ridge.parent_link_record_exists():
                            skip_child = True
                        break

                if skip_child:
                    continue

                if child.rock_ridge.child_link_record_exists() and \
                   child.rock_ridge.cl_to_moved_dr is not None and \
                   child.rock_ridge.cl_to_moved_dr.parent is not None:
                    # This is a relocated entry.  We want to find the entry this
                    # was relocated to; we do that by following the child_link,
                    # then going up to the parent and finding the entry that
                    # links to the same one as this one.
                    cl_parent = child.rock_ridge.cl_to_moved_dr.parent
                    for cl_child in cl_parent.children:
                        if cl_child.rock_ridge is not None and cl_child.rock_ridge.name() == child.rock_ridge.name():
                            child = cl_child
                            break
                    # If we didn't find the relocated entry in the parent of the
                    # moved entry, weird; just yield the one we would have anyway.

        yield child


def _find_dr_record_by_name(vd, path, encoding):
    # type: (headervd.PrimaryOrSupplementaryVD, bytes, str) -> dr.DirectoryRecord
    """
    An internal function to find a directory record on the ISO given an ISO
    or Joliet path.  If the entry is found, it returns the directory record
    object corresponding to that entry.  If the entry could not be found,
    a pycdlibexception.PyCdlibInvalidInput exception is raised.

    Parameters:
     vd - The Volume Descriptor to look in for the Directory Record.
     path - The ISO or Joliet entry to find the Directory Record for.
     encoding - The string encoding used for the path.
    Returns:
     The directory record entry representing the entry on the ISO.
    """
    root_dir_record = vd.root_directory_record()

    # If the path is just the slash, we want to return the root directory.
    if path == b'/':
        return root_dir_record

    splitpath = utils.split_path(path)

    currpath = splitpath.pop(0).decode('utf-8').encode(encoding)

    entry = root_dir_record

    tmpdr = dr.DirectoryRecord()

    while True:
        child = None

        thelist = entry.children
        lo = 2
        hi = len(thelist)
        while lo < hi:
            mid = (lo + hi) // 2
            tmpdr.file_ident = currpath
            if thelist[mid] < tmpdr:
                lo = mid + 1
            else:
                hi = mid
        index = lo
        if index != len(thelist) and thelist[index].file_ident == currpath:
            child = thelist[index]

        if child is None:
            # We failed to find this component of the path, so break out of the
            # loop and fail.
            break

        if child.rock_ridge is not None and child.rock_ridge.child_link_record_exists():
            # The rock ridge extension has a child link, so follow it.
            child = child.rock_ridge.cl_to_moved_dr
            if child is None:
                break

        # We found the last child we are looking for; return it.
        if not splitpath:
            return child

        if not child.is_dir():
            break
        entry = child
        currpath = splitpath.pop(0).decode('utf-8').encode(encoding)

    raise pycdlibexception.PyCdlibInvalidInput('Could not find path')


class PyCdlib:
    """The main class for manipulating ISOs."""
    __slots__ = ('_initialized', '_cdfp', 'pvds', 'svds', 'vdsts', 'brs', 'pvd',
                 'rock_ridge', '_always_consistent', '_has_udf', 'joliet_vd',
                 'eltorito_boot_catalog', 'isohybrid_mbr', '_managing_fp', 'xa',
                 '_needs_reshuffle', '_rr_moved_record', '_rr_moved_name',
                 '_rr_moved_rr_name', 'enhanced_vd', 'version_vd', 'inodes',
                 'interchange_level', '_write_check_list', '_track_writes',
                 'udf_beas', 'udf_nsr', 'udf_teas', 'udf_anchors',
                 'udf_main_descs', 'udf_reserve_descs',
                 'udf_logical_volume_integrity', 'udf_boots',
                 'udf_logical_volume_integrity_terminator', 'udf_root',
                 'udf_file_set', 'udf_file_set_terminator',
                 'logical_block_size')

    def _initialize(self):
        # type: () -> None
        """
        An internal method to re-initialize the object.  Called from
        both __init__ and close.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        self._cdfp = io.BytesIO()  # type: IO[Any]
        self.svds = []  # type: List[headervd.PrimaryOrSupplementaryVD]
        self.brs = []  # type: List[headervd.BootRecord]
        self.vdsts = []  # type: List[headervd.VolumeDescriptorSetTerminator]
        self.eltorito_boot_catalog = None  # type: Optional[eltorito.EltoritoBootCatalog]
        self._initialized = False
        self.rock_ridge = ''
        self.isohybrid_mbr = None  # type: Optional[isohybrid.IsoHybrid]
        self.xa = False
        self._managing_fp = False
        self.pvds = []  # type: List[headervd.PrimaryOrSupplementaryVD]
        self._has_udf = False
        self.udf_beas = []  # type: List[udfmod.BEAVolumeStructure]
        self.udf_boots = []  # type: List[udfmod.UDFBootDescriptor]
        self.udf_nsr = udfmod.NSRVolumeStructure()
        self.udf_teas = []  # type: List[udfmod.TEAVolumeStructure]
        self.udf_anchors = []  # type: List[udfmod.UDFAnchorVolumeStructure]
        self.udf_main_descs = udfmod.UDFDescriptorSequence()
        self.udf_reserve_descs = udfmod.UDFDescriptorSequence()
        self.udf_logical_volume_integrity = None  # type: Optional[udfmod.UDFLogicalVolumeIntegrityDescriptor]
        self.udf_logical_volume_integrity_terminator = None  # type: Optional[udfmod.UDFTerminatingDescriptor]
        self.udf_root = None  # type: Optional[udfmod.UDFFileEntry]
        self.udf_file_set = udfmod.UDFFileSetDescriptor()
        self.udf_file_set_terminator = None  # type: Optional[udfmod.UDFTerminatingDescriptor]
        self._needs_reshuffle = False
        self._rr_moved_record = dr.DirectoryRecord()
        self._rr_moved_name = None  # type: Optional[bytes]
        self._rr_moved_rr_name = None  # type: Optional[bytes]
        self.enhanced_vd = None  # type: Optional[headervd.PrimaryOrSupplementaryVD]
        self.joliet_vd = None  # type: Optional[headervd.PrimaryOrSupplementaryVD]
        self._find_iso_record.cache_clear()  # pylint: disable=no-member
        self._find_rr_record.cache_clear()  # pylint: disable=no-member
        self._find_joliet_record.cache_clear()  # pylint: disable=no-member
        self._find_udf_record.cache_clear()  # pylint: disable=no-member
        self._write_check_list = []  # type: List[PyCdlib._WriteRange]
        self.version_vd = None  # type: Optional[headervd.VersionVolumeDescriptor]
        self.inodes = []  # type: List[inode.Inode]
        # Default to a logical block size of 2048; this will be overridden by
        # the block size from the PVD or the detected block size during an open.
        self.logical_block_size = 2048
        self.interchange_level = 1  # type: int

    def _parse_volume_descriptors(self):
        # type: () -> None
        """
        An internal method to parse the volume descriptors on an ISO.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        # Ecma-119, 6.2.1 says that the Volume Space is divided into a System
        # Area and a Data Area, where the System Area is in logical sectors 0
        # to 15, and whose contents is not specified by the standard.  Logical
        # sectors are 2048 bytes in length, so we start at offset 16 * 2048.
        self._cdfp.seek(16 * 2048)
        while True:
            # All volume descriptors are exactly 2048 bytes long
            curr_extent = self._cdfp.tell() // 2048
            vd = self._cdfp.read(2048)
            if len(vd) != 2048:
                raise pycdlibexception.PyCdlibInvalidISO('Failed to read entire volume descriptor')
            (desc_type, ident) = struct.unpack_from('=B5s', vd, 0)
            if desc_type not in (headervd.VOLUME_DESCRIPTOR_TYPE_PRIMARY,
                                 headervd.VOLUME_DESCRIPTOR_TYPE_SET_TERMINATOR,
                                 headervd.VOLUME_DESCRIPTOR_TYPE_BOOT_RECORD,
                                 headervd.VOLUME_DESCRIPTOR_TYPE_SUPPLEMENTARY) or ident not in (b'CD001', b'CDW02', b'BEA01', b'NSR02', b'NSR03', b'TEA01', b'BOOT2'):
                # We read the next extent, and it wasn't a descriptor.  Abort
                # the loop, remembering to back up the input file descriptor.
                self._cdfp.seek(-2048, os.SEEK_CUR)
                break
            if desc_type == headervd.VOLUME_DESCRIPTOR_TYPE_PRIMARY:
                pvd = headervd.PrimaryOrSupplementaryVD(headervd.VOLUME_DESCRIPTOR_TYPE_PRIMARY)
                pvd.parse(vd, curr_extent)
                self.pvds.append(pvd)
            elif desc_type == headervd.VOLUME_DESCRIPTOR_TYPE_SET_TERMINATOR:
                vdst = headervd.VolumeDescriptorSetTerminator()
                vdst.parse(vd, curr_extent)
                self.vdsts.append(vdst)
            elif desc_type == headervd.VOLUME_DESCRIPTOR_TYPE_BOOT_RECORD:
                # Both an Ecma-119 Boot Record and a Ecma-TR 071 UDF-Bridge
                # Beginning Extended Area Descriptor have the first byte as 0,
                # so we can't tell which it is until we look at the next 5
                # bytes (Boot Record will have 'CD001', BEAD will have 'BEA01').
                if ident == b'CD001':
                    br = headervd.BootRecord()
                    br.parse(vd, curr_extent)
                    self.brs.append(br)
                elif ident == b'BEA01':
                    self._has_udf = True
                    udf_bea = udfmod.BEAVolumeStructure()
                    udf_bea.parse(vd, curr_extent)
                    self.udf_beas.append(udf_bea)
                elif ident in (b'NSR02', b'NSR03'):
                    self.udf_nsr.parse(vd, curr_extent)
                elif ident == b'TEA01':
                    udf_tea = udfmod.TEAVolumeStructure()
                    udf_tea.parse(vd, curr_extent)
                    self.udf_teas.append(udf_tea)
                elif ident == b'BOOT2':
                    udf_boot = udfmod.UDFBootDescriptor()
                    udf_boot.parse(vd, curr_extent)
                    self.udf_boots.append(udf_boot)
                else:
                    # This isn't really possible, since we would have aborted
                    # the loop above.
                    raise pycdlibexception.PyCdlibInvalidISO('Invalid volume identification type')
            elif desc_type == headervd.VOLUME_DESCRIPTOR_TYPE_SUPPLEMENTARY:
                svd = headervd.PrimaryOrSupplementaryVD(headervd.VOLUME_DESCRIPTOR_TYPE_SUPPLEMENTARY)
                svd.parse(vd, curr_extent)
                self.svds.append(svd)
            # Since we checked for the valid descriptors above, it is impossible
            # to see an invalid desc_type here, so no check necessary.

        # The language in Ecma-119, p.8, Section 6.7.1 says:
        #
        # The sequence shall contain one Primary Volume Descriptor (see 8.4)
        # recorded at least once.
        #
        # The important bit there is "at least one", which means that we have
        # to accept ISOs with more than one PVD.
        if not self.pvds:
            raise pycdlibexception.PyCdlibInvalidISO('Valid ISO9660 filesystems must have at least one PVD')

        self.pvd = self.pvds[0]

        # Make sure any other PVDs agree with the first one.
        for pvd in self.pvds[1:]:
            if pvd != self.pvd:
                raise pycdlibexception.PyCdlibInvalidISO('Multiple occurrences of PVD did not agree!')

            pvd.root_dir_record = self.pvd.root_dir_record

        if not self.vdsts:
            raise pycdlibexception.PyCdlibInvalidISO('Valid ISO9660 filesystems must have at least one Volume Descriptor Set Terminator')

    def _seek_to_extent(self, extent):
        # type: (int) -> None
        """
        An internal method to seek to a particular extent on the input ISO.

        Parameters:
         extent - The extent to seek to.
        Returns:
         Nothing.
        """
        self._cdfp.seek(extent * self.logical_block_size)

    @functools.lru_cache(maxsize=256)
    def _find_iso_record(self, iso_path, encoding='utf-8'):
        # type: (bytes, str) -> dr.DirectoryRecord
        """
        An internal method to find a directory record on the ISO given an ISO
        path.  If the entry is found, it returns the directory record object
        corresponding to that entry.  If the entry could not be found, a
        pycdlibexception.PyCdlibInvalidInput is raised.

        Parameters:
         iso_path - The ISO9660 path to lookup.
         encoding - The string encoding used for the path.
        Returns:
         The directory record entry representing the entry on the ISO.
        """
        return _find_dr_record_by_name(self.pvd, iso_path, encoding)

    @functools.lru_cache(maxsize=256)
    def _find_rr_record(self, rr_path, encoding='utf-8'):
        # type: (bytes, str) -> dr.DirectoryRecord
        """
        An internal method to find a directory record on the ISO given a Rock
        Ridge path.  If the entry is found, it returns the directory record
        object corresponding to that entry.  If the entry could not be found, a
        pycdlibexception.PyCdlibInvalidInput is raised.

        Parameters:
         rr_path - The Rock Ridge path to lookup.
         encoding - The string encoding used for the path.
        Returns:
         The directory record entry representing the entry on the ISO.
        """
        root_dir_record = self.pvd.root_directory_record()

        # If the path is just the slash, return the root directory.
        if rr_path == b'/':
            return root_dir_record

        splitpath = utils.split_path(rr_path)

        currpath = splitpath.pop(0).decode('utf-8').encode(encoding)

        entry = root_dir_record

        while True:
            child = None

            thelist = entry.rr_children
            # The list could be empty because we don't store dot or dotdot
            # entries in Rock Ridge.  If that is the case, just get out and
            # fail since we definitely didn't find what we were looking for.
            if not thelist:
                break

            lo = 0
            hi = len(thelist)
            while lo < hi:
                mid = (lo + hi) // 2

                tmpchild = thelist[mid]

                if tmpchild.rock_ridge is None:
                    raise pycdlibexception.PyCdlibInvalidInput('Record without Rock Ridge entry on Rock Ridge ISO')

                if tmpchild.rock_ridge.name() < currpath:
                    lo = mid + 1
                else:
                    hi = mid
            index = lo
            tmpchild = thelist[index]
            if index != len(thelist) and tmpchild.rock_ridge is not None and tmpchild.rock_ridge.name() == currpath:
                child = thelist[index]

            if child is None:
                # We failed to find this component of the path, so break out of
                # the loop and fail
                break

            if child.rock_ridge is not None and child.rock_ridge.child_link_record_exists():
                # The rock ridge extension has a child link we need to follow.
                child = child.rock_ridge.cl_to_moved_dr
                if child is None:
                    break

            # We found the last child we are looking for; return it.
            if not splitpath:
                return child

            if not child.is_dir():
                break
            entry = child
            currpath = splitpath.pop(0).decode('utf-8').encode(encoding)

        raise pycdlibexception.PyCdlibInvalidInput('Could not find path')

    @functools.lru_cache(maxsize=256)
    def _find_joliet_record(self, joliet_path, encoding='utf-16_be'):
        # type: (bytes, str) -> dr.DirectoryRecord
        """
        An internal method to find a directory record on the ISO given a Joliet
        path.  If the entry is found, it returns the directory record object
        corresponding to that entry.  If the entry could not be found, a
        pycdlibexception.PyCdlibInvalidInput is raised.

        Parameters:
         joliet_path - The Joliet path to lookup.
         encoding - The string encoding used for the path.
        Returns:
         The directory record entry representing the entry on the ISO.
        """
        if self.joliet_vd is None:
            raise pycdlibexception.PyCdlibInternalError('Joliet path requested on non-Joliet ISO')
        return _find_dr_record_by_name(self.joliet_vd, joliet_path, encoding)

    @functools.lru_cache(maxsize=256)
    def _find_udf_record(self, udf_path):
        # type: (bytes) -> Tuple[Optional[udfmod.UDFFileIdentifierDescriptor], udfmod.UDFFileEntry]
        """
        An internal method to find a directory record on the ISO given a UDF
        path.  If the entry is found, it returns the directory record object
        corresponding to that entry.  If the entry could not be found, a
        pycdlibexception.PyCdlibInvalidInput is raised.

        Parameters:
         udf_path - The UDF path to lookup.
        Returns:
         The UDF File Entry representing the entry on the ISO.
        """
        # If the path is just the slash, return the root directory.
        if udf_path == b'/':
            return None, self.udf_root  # type: ignore

        splitpath = utils.split_path(udf_path)

        currpath = splitpath.pop(0)

        entry = self.udf_root

        while entry is not None:
            child = entry.find_file_ident_desc_by_name(currpath)

            # We found the last child we are looking for; return it.
            if not splitpath:
                return child, child.file_entry  # type: ignore

            if not child.is_dir():
                break
            entry = child.file_entry
            currpath = splitpath.pop(0)

        raise pycdlibexception.PyCdlibInvalidInput('Could not find path')

    def _iso_name_and_parent_from_path(self, iso_path):
        # type: (bytes) -> Tuple[bytes, dr.DirectoryRecord]
        """
        An internal method to find the parent directory record and name given an
        ISO path.  If the parent is found, return a tuple containing the
        basename of the path and the parent directory record object.

        Parameters:
         iso_path - The absolute ISO path to the entry on the ISO.
        Returns:
         A tuple containing just the name of the entry and a Directory Record
         object representing the parent of the entry.
        """

        splitpath = utils.split_path(iso_path)
        name = splitpath.pop()

        parent = self._find_iso_record(b'/' + b'/'.join(splitpath))

        return (name.decode('utf-8').encode('utf-8'), parent)

    def _joliet_name_and_parent_from_path(self, joliet_path):
        # type: (bytes) -> Tuple[bytes, dr.DirectoryRecord]
        """
        An internal method to find the parent directory record and name given a
        Joliet path.  If the parent is found, return a tuple containing the
        basename of the path and the parent directory record object.

        Parameters:
         joliet_path - The absolute Joliet path to the entry on the ISO.
        Returns:
         A tuple containing just the name of the entry and a Directory Record
         object representing the parent of the entry.
        """

        splitpath = utils.split_path(joliet_path)
        name = splitpath.pop()

        if len(name) > 64:
            raise pycdlibexception.PyCdlibInvalidInput('Joliet names can be a maximum of 64 characters')
        parent = self._find_joliet_record(b'/' + b'/'.join(splitpath))

        return (name.decode('utf-8').encode('utf-16_be'), parent)

    def _udf_name_and_parent_from_path(self, udf_path):
        # type: (bytes) -> Tuple[bytes, udfmod.UDFFileEntry]
        """
        An internal method to find the parent directory record and name given a
        UDF path.  If the parent is found, return a tuple containing the
        basename of the path and the parent UDF File Entry object.

        Parameters:
         udf_path - The absolute UDF path to the entry on the ISO.
        Returns:
         A tuple containing just the name of the entry and a UDF File Entry
         object representing the parent of the entry.
        """
        splitpath = utils.split_path(udf_path)
        name = splitpath.pop()
        (parent_ident_unused, parent) = self._find_udf_record(b'/' + b'/'.join(splitpath))

        return (name.decode('utf-8').encode('utf-8'), parent)

    def _set_rock_ridge(self, rr):
        # type: (str) -> None
        """
        An internal method to set the Rock Ridge version of the ISO given the
        Rock Ridge version of the previous entry.

        Parameters:
         rr - The version of rr from the last directory record.
        Returns:
         Nothing.
        """
        # We don't allow mixed Rock Ridge versions on the ISO, so apply some
        # checking.  If the current overall Rock Ridge version on the ISO is
        # None, we upgrade it to whatever version we were given.  Once we have
        # seen a particular version, we only allow records of that version or
        # None (to account for dotdot records which have no Rock Ridge).
        if not self.rock_ridge:
            self.rock_ridge = rr
        else:
            for ver in ('1.09', '1.10', '1.12'):
                if self.rock_ridge == ver:
                    if rr and rr != ver:
                        raise pycdlibexception.PyCdlibInvalidISO('Inconsistent Rock Ridge versions on the ISO!')

    def _get_iso_size(self):
        # type: () -> int
        """
        An internal method to get the ISO size.  This is more complicated than
        you might think due to Windows.  There, if you try to open a 'raw'
        device, you can only seek on 2048-byte boundaries (and you can't seek to
        the END).  We first try to seek to the end, which is the most efficient
        way to do it.  If that fails, we fall back to seeking and reading until
        we get empty data, which signals the end of the ISO.

        Parameters:
         None.
        Returns:
         The size of the ISO in bytes.
        """
        old = self._cdfp.tell()
        try:
            self._cdfp.seek(0, os.SEEK_END)
            ret = self._cdfp.tell()
            self._cdfp.seek(old)
            return ret
        except OSError:
            pass

        # When reading a raw Windows device, we cannot seek to the end to find
        # the length.  Instead, we start right at the beginning of the ISO and
        # seek by 1MB at a time to find the end of it.  This meets the Windows
        # requirement that we seek by a 512-byte aligned value while also
        # having decent performance.  Once we find the 1MB boundary, we back
        # up and find the real 2048-byte boundary.
        bs = 1048576
        one_mb_block = 0
        while True:
            self._cdfp.seek(one_mb_block * bs)
            data = self._cdfp.read(1)
            if len(data) != 1:
                break
            one_mb_block += 1
        if one_mb_block > 0:
            one_mb_block -= 1
        extent = (one_mb_block * bs) // 2048
        while True:
            self._cdfp.seek(extent * 2048)
            data = self._cdfp.read(2048)
            if len(data) != 2048:
                break
            extent += 1

        self._cdfp.seek(old)
        return extent * 2048

    def _walk_directories(self, vd, extent_to_ptr, extent_to_inode,
                          path_table_records):
        # type: (headervd.PrimaryOrSupplementaryVD, Dict[int, path_table_record.PathTableRecord], Dict[int, inode.Inode], List[path_table_record.PathTableRecord]) -> Tuple[int, int]
        """
        An internal method to walk the directory records in a volume descriptor,
        starting with the root.  For each child in the directory record,
        create a new dr.DirectoryRecord object and append it to the parent.

        Parameters:
         vd - The volume descriptor to walk.
         extent_to_ptr - A dictionary mapping extents to PTRs.
         extent_to_inode - A dictionary mapping extents to Inodes.
         path_table_records - The list of path table records.
        Returns:
         The interchange level that this ISO conforms to.
        """
        cdfp = self._cdfp
        iso_file_length = self._get_iso_size()

        all_extent_to_dr = {}  # type: Dict[int, dr.DirectoryRecord]
        is_pvd = vd.is_pvd()
        root_dir_record = vd.root_directory_record()
        root_dir_record.set_ptr(path_table_records[0])
        interchange_level = 1
        parent_links = []
        child_links = []
        lastbyte = 0
        dirs = collections.deque([root_dir_record])
        while dirs:
            dir_record = dirs.popleft()

            self._seek_to_extent(dir_record.extent_location())
            length = dir_record.get_data_length()
            offset = 0
            last_record = None  # type: Optional[dr.DirectoryRecord]
            data = cdfp.read(length)
            while offset < length:
                if offset > (len(data) - 1):
                    # The data we read off of the ISO was shorter than what we
                    # expected.  The ISO is corrupt, throw an error.
                    raise pycdlibexception.PyCdlibInvalidISO('Invalid directory record')
                lenbyte = bytearray([data[offset]])[0]
                if lenbyte == 0:
                    # If we saw a zero length, this is probably the padding for
                    # the end of this extent.  Move the offset to the start of
                    # the next extent.
                    padsize = self.logical_block_size - (offset % self.logical_block_size)
                    if data[offset:offset + padsize] != b'\x00' * padsize:
                        # For now we are pedantic, and throw an exception if the
                        # padding bytes are not all zero.  We may have to loosen
                        # this check depending on what we see in the wild.
                        raise pycdlibexception.PyCdlibInvalidISO('Invalid padding on ISO')

                    offset = offset + padsize
                    continue

                new_record = dr.DirectoryRecord()
                rr = new_record.parse(vd, data[offset:offset + lenbyte],
                                      dir_record)
                offset += lenbyte

                self._set_rock_ridge(rr)

                # Cache some properties of this record for later use.
                is_symlink = new_record.is_symlink()
                dots = new_record.is_dot() or new_record.is_dotdot()
                rr_cl = new_record.rock_ridge is not None and new_record.rock_ridge.child_link_record_exists()
                is_dir = new_record.is_dir()
                data_length = new_record.get_data_length()
                new_extent_loc = new_record.extent_location()

                if is_pvd and not dots and not rr_cl and not is_symlink and new_extent_loc not in all_extent_to_dr:
                    all_extent_to_dr[new_extent_loc] = new_record

                # Some ISOs use random extent locations for zero-length files.
                # Thus, it is not valid for us to link zero-length files to
                # other files, as the linkage will be essentially random.
                # Ignore zero-length files (including symlinks) for linkage.
                # We don't do the lastbyte calculation on zero-length files for
                # the same reason.
                if not is_dir:
                    len_to_use = data_length
                    extent_to_use = new_extent_loc
                    # An important side-effect of this is that zero-length files
                    # or symlinks get an inode, but it is always set to length 0
                    # and location 0 and not actually written out.  This is so
                    # that we can 'link' everything through the Inode.
                    if len_to_use == 0 or is_symlink:
                        len_to_use = 0
                        extent_to_use = 0

                    # Directory Records that point to the El Torito Boot Catalog
                    # do not get Inodes since all of that is handled in-memory.
                    if self.eltorito_boot_catalog is not None and extent_to_use == self.eltorito_boot_catalog.extent_location():
                        self.eltorito_boot_catalog.add_dirrecord(new_record)
                    else:
                        # For real files, create an inode that points to the
                        # location on disk.
                        if extent_to_use in extent_to_inode:
                            ino = extent_to_inode[extent_to_use]
                        else:
                            ino = inode.Inode()
                            ino.parse(extent_to_use, len_to_use, cdfp,
                                      self.logical_block_size)
                            extent_to_inode[extent_to_use] = ino
                            self.inodes.append(ino)

                        ino.linked_records.append((new_record, vd == self.pvd))
                        new_record.inode = ino

                    new_end = extent_to_use * self.logical_block_size + len_to_use
                    if new_end > iso_file_length:
                        # The end of the file is beyond the size of the ISO.
                        # Since this can't be true, truncate the file size.
                        if new_record.inode is not None:
                            new_record.inode.data_length = iso_file_length - extent_to_use * self.logical_block_size
                            for rec, is_pvd in new_record.inode.linked_records:
                                rec.set_data_length(new_end)
                    else:
                        # The new end is still within the file size, but the PVD
                        # size is wrong.  Set the lastbyte appropriately, which
                        # will eventually be used to fix the PVD size.
                        lastbyte = max(lastbyte, new_end)

                if new_record.rock_ridge is not None and new_record.rock_ridge.dr_entries.ce_record is not None:
                    ce_record = new_record.rock_ridge.dr_entries.ce_record
                    orig_pos = cdfp.tell()
                    self._seek_to_extent(ce_record.bl_cont_area)
                    cdfp.seek(ce_record.offset_cont_area, os.SEEK_CUR)
                    con_block = cdfp.read(ce_record.len_cont_area)
                    new_record.rock_ridge.parse(con_block, False,
                                                new_record.rock_ridge.bytes_to_skip,
                                                True, new_record.file_identifier())
                    cdfp.seek(orig_pos)
                    block = self.pvd.track_rr_ce_entry(ce_record.bl_cont_area,
                                                       ce_record.offset_cont_area,
                                                       ce_record.len_cont_area)
                    new_record.rock_ridge.update_ce_block(block)

                if rr_cl:
                    child_links.append(new_record)

                if is_dir:
                    if new_record.rock_ridge is not None and new_record.rock_ridge.relocated_record():
                        self._rr_moved_record = new_record

                    if new_record.is_dotdot() and new_record.rock_ridge is not None and new_record.rock_ridge.parent_link_record_exists():
                        # Make sure to mark a dotdot record with a parent link
                        # record in the parent_links list for later linking.
                        parent_links.append(new_record)
                    if not dots and not rr_cl:
                        dirs.append(new_record)
                        new_record.set_ptr(extent_to_ptr[new_extent_loc])

                if new_record.parent is None:
                    raise pycdlibexception.PyCdlibInternalError('Trying to track child with no parent')
                try_long_entry = False
                try:
                    new_record.parent.track_child(new_record,
                                                  self.logical_block_size)
                except pycdlibexception.PyCdlibInvalidInput:
                    # dir_record.track_child() may throw a PyCdlibInvalidInput
                    # if it was given a duplicate child.  However, we allow
                    # duplicate children if and only if this record is a file
                    # and the last file has the same name; this represents a
                    # very large file.
                    if new_record.is_dir() or last_record is None or last_record.file_identifier() != new_record.file_identifier():
                        raise

                    try_long_entry = True

                if try_long_entry:
                    new_record.parent.track_child(new_record,
                                                  self.logical_block_size, True)

                if is_pvd:
                    if new_record.is_dir():
                        new_level = _interchange_level_from_directory(new_record.file_identifier())
                    else:
                        new_level = _interchange_level_from_filename(new_record.file_identifier())
                    interchange_level = max(interchange_level, new_level)

                last_record = new_record

        for pl in parent_links:
            if pl.rock_ridge is not None:
                pl.rock_ridge.parent_link = all_extent_to_dr[pl.rock_ridge.parent_link_extent()]

        for cl in child_links:
            if cl.rock_ridge is not None:
                cl.rock_ridge.cl_to_moved_dr = all_extent_to_dr[cl.rock_ridge.child_link_extent()]
                if cl.rock_ridge.cl_to_moved_dr.rock_ridge is not None:
                    cl.rock_ridge.cl_to_moved_dr.rock_ridge.moved_to_cl_dr = cl

        return interchange_level, lastbyte

    def _parse_path_table(self, ptr_size, extent):
        # type: (int, int) -> Tuple[List[path_table_record.PathTableRecord], Dict[int, path_table_record.PathTableRecord]]
        """
        An internal method to parse a path table on an ISO.  For each path
        table entry found, a Path Table Record object is created and added to
        the output list.

        Parameters:
         ptr_size - The size of the PTR table to read.
         extent - The extent at which this path table record starts.
        Returns:
         A tuple consisting of the list of path table record entries and a
         dictionary of the extent locations to the path table record entries.
        """
        self._seek_to_extent(extent)
        old = self._cdfp.tell()
        data = self._cdfp.read(ptr_size)
        offset = 0
        out = []
        extent_to_ptr = {}
        while offset < ptr_size:
            ptr = path_table_record.PathTableRecord()
            len_di_byte = bytearray([data[offset]])[0]
            read_len = path_table_record.PathTableRecord.record_length(len_di_byte)

            ptr.parse(data[offset:offset + read_len])
            out.append(ptr)
            extent_to_ptr[ptr.extent_location] = ptr
            offset += read_len

        self._cdfp.seek(old)
        return out, extent_to_ptr

    def _check_and_parse_eltorito(self, br):
        # type: (headervd.BootRecord) -> None
        """
        An internal method to examine a Boot Record and see if it is an
        El Torito Boot Record.  If it is, parse the El Torito Boot Catalog,
        verification entry, initial entry, and any additional section entries.

        Parameters:
         br - The boot record to examine for an El Torito signature.
        Returns:
         Nothing.
        """
        if br.boot_system_identifier != b'EL TORITO SPECIFICATION'.ljust(32, b'\x00'):
            return

        if self.eltorito_boot_catalog is not None:
            raise pycdlibexception.PyCdlibInvalidISO('Only one El Torito boot record is allowed')

        # According to the El Torito specification, section 2.0, the El
        # Torito boot record must be at extent 17.
        if br.extent_location() != 17:
            raise pycdlibexception.PyCdlibInvalidISO('El Torito Boot Record must be at extent 17')

        # Once we have verified that the BootRecord is an El Torito one and that
        # it is sane, we parse the El Torito Boot Catalog.

        self.eltorito_boot_catalog = eltorito.EltoritoBootCatalog(br)
        # While it is not entirely clear in the El Torito spec, empirical
        # evidence suggests that the boot catalog extent is always
        # little-endian, even on big-endian systems
        eltorito_boot_catalog_extent, = struct.unpack_from('<L',
                                                           br.boot_system_use[:4],
                                                           0)

        old = self._cdfp.tell()
        self._seek_to_extent(eltorito_boot_catalog_extent)
        data = self._cdfp.read(32)
        while not self.eltorito_boot_catalog.parse(data):
            data = self._cdfp.read(32)
        self._cdfp.seek(old)

    def _udf_assign_extents(self, udf_files, current_extent):
        # type: (List[inode.Inode], int) -> Tuple[int, int]
        """
        An internal method to assign UDF extents when reshuffling.

        Parameters:
         udf_files - The list of UDF Inodes that will need extents assigned.
         current_extent - The current extent being assigned.
        Returns:
         A tuple where the first entry is the new current extent, and the
         second entry is the new partition start.
        """
        if current_extent > 32:
            # There is no *requirement* in the UDF specification that the UDF
            # Volume Descriptor Sequence starts at extent 32.  It can start
            # anywhere between extents 16 and 256, as long as the ISO9660
            # volume descriptors, the UDF Bridge Volume Recognition Sequence,
            # Main Volume Descriptor Sequence, Reserve Volume Descriptor
            # Sequence, and Logical Volume Integrity Sequence all fit, in that
            # order.  The only way that all of these volume descriptors would
            # not fit between extents 16 and 32 is in the case of many
            # duplicate PVDs, many VDSTs, or similar.  Since that is unlikely,
            # for now we maintain compatibility with genisoimage and force the
            # UDF Main Descriptor Sequence to start at 32.  We can change this
            # later if needed.
            raise pycdlibexception.PyCdlibInternalError('Too many ISO9660 volume descriptors to fit UDF')

        self.udf_main_descs.assign_desc_extents(32)

        # ECMA TR-071 2.6 says that the volume sequence will be exactly 16
        # extents long, and we know we started at 32, so make it exactly 48.
        self.udf_reserve_descs.assign_desc_extents(48)

        # ECMA TR-071 2.6 says that the volume sequence will be exactly 16
        # extents long, and we know we started at 48, so make it exactly 64.
        current_extent = 64

        if self.udf_logical_volume_integrity is not None:
            self.udf_logical_volume_integrity.set_extent_location(current_extent)
            self.udf_main_descs.logical_volumes[0].set_integrity_location(current_extent)
            self.udf_reserve_descs.logical_volumes[0].set_integrity_location(current_extent)
            current_extent += 1

        if self.udf_logical_volume_integrity_terminator is not None:
            self.udf_logical_volume_integrity_terminator.set_extent_location(current_extent)
            current_extent += 1

        if len(self.udf_anchors) != 2:
            raise pycdlibexception.PyCdlibInternalError('Expected 2 UDF anchors')

        # The first UDF anchor is hard-coded at extent 256.  We assign the
        # other anchor later, since it needs to be the last extent.
        current_extent = 256
        self.udf_anchors[0].set_extent_location(current_extent,
                                                self.udf_main_descs.pvds[0].extent_location(),
                                                self.udf_reserve_descs.pvds[0].extent_location())
        current_extent += 1

        # Now assign the UDF File Set Descriptor to the beginning of the
        # partition.
        part_start = current_extent
        self.udf_file_set.set_extent_location(part_start)
        self.udf_main_descs.partitions[0].set_start_location(part_start)
        self.udf_reserve_descs.partitions[0].set_start_location(part_start)
        current_extent += 1

        if self.udf_file_set_terminator is not None:
            self.udf_file_set_terminator.set_extent_location(current_extent,
                                                             current_extent - part_start)
            current_extent += 1

        # Assignment of extents to UDF is complicated.  UDF filesystems are
        # arranged by having one extent containing a File Entry that describes
        # a directory or a file, followed by an extent that contains the
        # entries in the case of a directory.  All File Entries and entries
        # containing File Identifier Descriptors are arranged ahead of File
        # Entries for files.  The implementation below alternates assignment to
        # File Entries and File Descriptores for all directories, and then
        # then assigns to all files.  Note that data for files is assigned in
        # the 'normal' file assignment below.

        # First assign directories.
        if self.udf_root is None:
            raise pycdlibexception.PyCdlibInternalError('ISO has UDF but no UDF root; this should never happen')
        udf_file_assign_list = []
        udf_file_entries = collections.deque([(self.udf_root, None)])  # type: Deque[Tuple[udfmod.UDFFileEntry, Optional[udfmod.UDFFileIdentifierDescriptor]]]
        while udf_file_entries:
            udf_file_entry, fi_desc = udf_file_entries.popleft()

            # In theory we should check for and skip the work for files and
            # symlinks, but they will never be added to 'udf_file_entries'
            # to begin with so we can safely ignore them.

            # Set the location that the File Entry lives at, and update the
            # File Identifier Descriptor that points to it (for all but the
            # root).
            udf_file_entry.set_extent_location(current_extent,
                                               current_extent - part_start)
            if fi_desc is not None:
                fi_desc.set_icb(current_extent, current_extent - part_start)
            current_extent += 1

            # Now assign where the File Entry points to; for files this is
            # overwritten later, but for directories this tells us where to
            # find the extent containing the list of File Identifier
            # Descriptors that are in this directory.
            udf_file_entry.set_data_location(current_extent,
                                             current_extent - part_start)
            offset = 0
            for d in udf_file_entry.fi_descs:
                if offset >= self.logical_block_size:
                    # The offset has spilled over into a new extent.  Increase
                    # the current extent by one, and update the offset.  Note
                    # that the offset does not go to 0, since UDF allows File
                    # Identifier Descs to span extents.  Instead, it is the
                    # current offset minus the size of a block (say 2050 - 2048,
                    # leaving us at offset 2).
                    current_extent += 1
                    offset = offset - self.logical_block_size

                d.set_extent_location(current_extent,
                                      current_extent - part_start)
                if not d.is_parent() and d.file_entry is not None:
                    if d.is_dir():
                        udf_file_entries.append((d.file_entry, d))
                    else:
                        udf_file_assign_list.append((d.file_entry, d))
                offset += udfmod.UDFFileIdentifierDescriptor.length(len(d.fi))

            if offset > self.logical_block_size:
                current_extent += 1

            current_extent += 1

        # Now assign files (this includes symlinks).
        udf_file_entry_inodes_assigned = set()
        for udf_file_assign_entry, fi_desc in udf_file_assign_list:
            if udf_file_assign_entry is None or fi_desc is None:
                continue

            if udf_file_assign_entry.inode is not None and id(udf_file_assign_entry.inode) in udf_file_entry_inodes_assigned:
                continue

            udf_file_assign_entry.set_extent_location(current_extent,
                                                      current_extent - part_start)
            fi_desc.set_icb(current_extent, current_extent - part_start)

            if udf_file_assign_entry.inode is not None:
                # The data location for files will be set later.
                if udf_file_assign_entry.inode.get_data_length() > 0:
                    udf_files.append(udf_file_assign_entry.inode)
                for rec, pvd_unused in udf_file_assign_entry.inode.linked_records:
                    if isinstance(rec, udfmod.UDFFileEntry):
                        rec.set_extent_location(current_extent,
                                                current_extent - part_start)
                        if rec.file_ident is not None:
                            rec.file_ident.set_icb(current_extent,
                                                   current_extent - part_start)

                udf_file_entry_inodes_assigned.add(id(udf_file_assign_entry.inode))

            current_extent += 1

        if self.udf_logical_volume_integrity is not None:
            self.udf_logical_volume_integrity.logical_volume_contents_use.unique_id = current_extent

        return current_extent, part_start

    def _set_inode(self, ino, current_extent, part_start):
        # type: (inode.Inode, int, int) -> int
        """
        An internal function to set the location of an inode and update the
        metadata of all records attached to it.

        Parameters:
         ino - The inode to update.
         current_extent - The extent to set the inode to.
         part_start - The start of the partition that the inode is on.
        Returns:
         The new extent location.
        """
        if len(self.udf_anchors) > 2 and current_extent == self.pvd.space_size - 256:
            current_extent += 1

        ino.set_extent_location(current_extent)
        for rec, pvd_unused in ino.linked_records:
            rec.set_data_location(current_extent,
                                  current_extent - part_start)

        current_extent += utils.ceiling_div(ino.get_data_length(),
                                            self.logical_block_size)
        return current_extent

    def _reshuffle_extents(self):
        # type: () -> None
        """
        An internal method that is one of the keys of PyCdlib's ability to keep
        the in-memory metadata consistent at all times.  After making any
        changes to the ISO, most API calls end up calling this method.  This
        method will run through the entire ISO, assigning extents to each of
        the pieces of the ISO that exist.  This includes the Primary Volume
        Descriptor (which is fixed at extent 16), the Boot Records (including
        El Torito), the Supplementary Volume Descriptors (including Joliet),
        the Volume Descriptor Terminators, the Version Descriptor, the Primary
        Volume Descriptor Path Table Records (little and big endian), the
        Supplementary Volume Descriptor Path Table Records (little and big
        endian), the Primary Volume Descriptor directory records, the
        Supplementary Volume Descriptor directory records, the Rock Ridge ER
        sector, the El Torito Boot Catalog, the El Torito Initial Entry, the
        various UDF metadata entries, and the data for files.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        current_extent = 16
        for pvd in self.pvds:
            pvd.set_extent_location(current_extent)
            current_extent += 1

        for br in self.brs:
            br.set_extent_location(current_extent)
            current_extent += 1

        for svd in self.svds:
            svd.set_extent_location(current_extent)
            current_extent += 1

        for vdst in self.vdsts:
            vdst.set_extent_location(current_extent)
            current_extent += 1

        if self._has_udf:
            for bea in self.udf_beas:
                bea.set_extent_location(current_extent)
                current_extent += 1

            for boot in self.udf_boots:
                boot.set_extent_location(current_extent)
                current_extent += 1

            self.udf_nsr.set_extent_location(current_extent)
            current_extent += 1

            for tea in self.udf_teas:
                tea.set_extent_location(current_extent)
                current_extent += 1

        if self.version_vd is not None:
            self.version_vd.set_extent_location(current_extent)
            current_extent += 1

        part_start = 0

        udf_files = []  # type: List[inode.Inode]
        if self._has_udf:
            current_extent, part_start = self._udf_assign_extents(udf_files, current_extent)

        # Next up, put the path table records in the right place.
        for pvd in self.pvds:
            pvd.path_table_location_le = current_extent
        current_extent += self.pvd.path_table_num_extents

        for pvd in self.pvds:
            pvd.path_table_location_be = current_extent
        current_extent += self.pvd.path_table_num_extents

        if self.enhanced_vd is not None:
            self.enhanced_vd.path_table_location_le = self.pvd.path_table_location_le
            self.enhanced_vd.path_table_location_be = self.pvd.path_table_location_be

        if self.joliet_vd is not None:
            self.joliet_vd.path_table_location_le = current_extent
            current_extent += self.joliet_vd.path_table_num_extents
            self.joliet_vd.path_table_location_be = current_extent
            current_extent += self.joliet_vd.path_table_num_extents

        self.pvd.clear_rr_ce_entries()
        current_extent, pvd_files = _reassign_vd_dirrecord_extents(self.pvd,
                                                                   current_extent)

        joliet_files = []  # type: List[inode.Inode]
        if self.joliet_vd is not None:
            current_extent, joliet_files = _reassign_vd_dirrecord_extents(self.joliet_vd,
                                                                          current_extent)

        # The rock ridge 'ER' sector must be after all of the directory
        # entries but before the file contents.
        rr = self.pvd.root_directory_record().children[0].rock_ridge
        if rr is not None and rr.dr_entries.ce_record is not None:
            rr.dr_entries.ce_record.update_extent(current_extent)
            current_extent += 1

        if len(self.udf_anchors) > 2:
            self.udf_anchors[1].set_extent_location(self.pvd.space_size - 256,
                                                    self.udf_main_descs.pvds[0].extent_location(),
                                                    self.udf_reserve_descs.pvds[0].extent_location())

        linked_inodes = set()
        if self.eltorito_boot_catalog is not None:
            self.eltorito_boot_catalog.update_catalog_extent(current_extent)
            for rec in self.eltorito_boot_catalog.dirrecords:
                rec.set_data_location(current_extent, current_extent - part_start)

            current_extent += utils.ceiling_div(self.eltorito_boot_catalog.dirrecords[0].get_data_length(),
                                                self.logical_block_size)

            class _EltoritoEncapsulation:
                """
                An internal class to encapsulate an El Torito Entry object with
                additional necessary metadata for sorting.
                """
                def __init__(self, entry, platform_id, name):
                    self.entry = entry
                    self.platform_id = platform_id
                    self.name = name

                def __lt__(self, other):
                    return self.name < other.name

            def _add_entry_to_enc_list(enc_to_update, entry, platform_id):
                # type: (List[_EltoritoEncapsulation], eltorito.EltoritoEntry, int) -> None
                added_enc = False
                for rec, is_pvd in entry.inode.linked_records:
                    if not is_pvd:
                        continue

                    if not isinstance(rec, udfmod.UDFFileEntry) and not isinstance(rec, dr.DirectoryRecord):
                        continue

                    enc = _EltoritoEncapsulation(entry, platform_id, rec.file_identifier())
                    bisect.insort_right(enc_to_update, enc)
                    added_enc = True

                if not added_enc:
                    # In this case, the entry wasn't linked into the PVD
                    # filesystem at all.  Just add an entry with a dummy name
                    # that is guaranteed to sort first.
                    enc = _EltoritoEncapsulation(entry, platform_id, b'AAAAAAAA.;1')
                    bisect.insort_right(enc_to_update, enc)

            enc_to_update = []  # type: List[_EltoritoEncapsulation]
            _add_entry_to_enc_list(enc_to_update,
                                   self.eltorito_boot_catalog.initial_entry,
                                   self.eltorito_boot_catalog.validation_entry.platform_id)

            for sec in self.eltorito_boot_catalog.sections:
                for entry in sec.section_entries:
                    _add_entry_to_enc_list(enc_to_update, entry, sec.platform_id)

            for entry in self.eltorito_boot_catalog.standalone_entries:
                _add_entry_to_enc_list(enc_to_update, entry,
                                       self.eltorito_boot_catalog.validation_entry.platform_id)

            num_seen_efi = 0
            for enc in enc_to_update:
                if id(enc.entry.inode) in linked_inodes:
                    continue

                enc.entry.set_data_location(current_extent,
                                            current_extent - part_start)

                if self.isohybrid_mbr is not None:
                    if enc.platform_id == 0xef:
                        if num_seen_efi == 0:
                            self.isohybrid_mbr.update_efi(current_extent,
                                                          entry.sector_count,
                                                          self.pvd.space_size * self.logical_block_size)
                        elif num_seen_efi == 1:
                            self.isohybrid_mbr.update_mac(current_extent,
                                                          entry.sector_count)
                        else:
                            raise pycdlibexception.PyCdlibInternalError('Only expected two EFI sections')
                        num_seen_efi += 1
                    elif enc.platform_id == 0:
                        self.isohybrid_mbr.update_rba(current_extent)

                current_extent = self._set_inode(enc.entry.inode, current_extent,
                                                 part_start)
                linked_inodes.add(id(enc.entry.inode))

        for ino in pvd_files + joliet_files + udf_files:
            if id(ino) in linked_inodes:
                # We've already assigned an extent because it was linked to an
                # earlier entry.
                continue

            current_extent = self._set_inode(ino, current_extent, part_start)

            linked_inodes.add(id(ino))

        if self.enhanced_vd is not None:
            loc = self.pvd.root_directory_record().extent_location()
            self.enhanced_vd.root_directory_record().set_data_location(loc, loc)

        if self.udf_anchors:
            self.udf_anchors[-1].set_extent_location(current_extent,
                                                     self.udf_main_descs.pvds[0].extent_location(),
                                                     self.udf_reserve_descs.pvds[0].extent_location())

        if current_extent > self.pvd.space_size:
            raise pycdlibexception.PyCdlibInternalError('Assigned an extent beyond the ISO (%d > %d)' % (current_extent, self.pvd.space_size))

        self._needs_reshuffle = False

    def _add_child_to_dr(self, child):
        # type: (dr.DirectoryRecord) -> int
        """
        An internal method to add a child to a directory record, expanding the
        space in the Volume Descriptor(s) if necessary.

        Parameters:
         child - The new child.
        Returns:
         The number of bytes to add for this directory record (this may be zero).
        """
        if child.parent is None:
            raise pycdlibexception.PyCdlibInternalError('Trying to add child without a parent')

        try_long_entry = False
        ret = -1
        try:
            ret = child.parent.add_child(child, self.logical_block_size)
        except pycdlibexception.PyCdlibInvalidInput:
            # dir_record.add_child() may throw a PyCdlibInvalidInput if it was
            # given a duplicate child.  However, we allow duplicate children if
            # and only the last child is the same; this represents a very large
            # file.
            if not child.is_dir():
                try_long_entry = True
            else:
                raise

        if try_long_entry:
            ret = child.parent.add_child(child, self.logical_block_size, True)

        # The add_child() method returns True if the parent needs another extent
        # in order to fit the directory record for this child.
        if ret:
            return self.logical_block_size

        return 0

    def _remove_child_from_dr(self, child, index):
        # type: (dr.DirectoryRecord, int) -> int
        """
        An internal method to remove a child from a directory record, shrinking
        the space in the Volume Descriptor if necessary.

        Parameters:
         child - The child to remove.
         index - The index of the child into the parent's child array.
        Returns:
         The number of bytes to remove for this directory record (this may be zero).
        """

        if child.parent is None:
            raise pycdlibexception.PyCdlibInternalError('Trying to remove child from non-existent parent')

        self._find_iso_record.cache_clear()  # pylint: disable=no-member
        self._find_rr_record.cache_clear()  # pylint: disable=no-member
        self._find_joliet_record.cache_clear()  # pylint: disable=no-member

        # The remove_child() method returns True if the parent no longer needs
        # the extent that the directory record for this child was on.
        if child.parent.remove_child(child, index, self.logical_block_size):
            return self.logical_block_size

        return 0

    def _add_to_ptr_size(self, ptr):
        # type: (path_table_record.PathTableRecord) -> int
        """
        An internal method to add a PTR to a VD, adding space to the VD if
        necessary.

        Parameters:
         ptr - The PTR to add to the vd.
        Returns:
         The number of additional bytes that are needed to fit the new PTR
         (this may be zero).
        """
        num_bytes_to_add = 0
        for pvd in self.pvds:
            # The add_to_ptr_size() method returns True if the PVD needs
            # additional space in the PTR to store this directory.  We always
            # add 4 additional extents for that (2 for LE, 2 for BE).
            if pvd.add_to_ptr_size(path_table_record.PathTableRecord.record_length(ptr.len_di)):
                num_bytes_to_add += 4 * self.logical_block_size

        return num_bytes_to_add

    def _remove_from_ptr_size(self, ptr):
        # type: (path_table_record.PathTableRecord) -> int
        """
        An internal method to remove a PTR from a VD, removing space from the
        VD if necessary.

        Parameters:
         ptr - The PTR to remove from the VD.
        Returns:
         The number of bytes to remove from the VDs (this may be zero).
        """
        num_bytes_to_remove = 0
        for pvd in self.pvds:
            # The remove_from_ptr_size() method returns True if the PVD no
            # longer needs the extra extents in the PTR that stored this
            # directory.  We always remove 4 additional extents for that.
            if pvd.remove_from_ptr_size(path_table_record.PathTableRecord.record_length(ptr.len_di)):
                num_bytes_to_remove += 4 * self.logical_block_size

        return num_bytes_to_remove

    def _find_or_create_rr_moved(self):
        # type: () -> int
        """
        An internal method to find the /RR_MOVED directory on the ISO.  If it
        already exists, the directory record to it is returned.  If it doesn't
        yet exist, it is created and the directory record to it is returned.

        Parameters:
         None.
        Returns:
         The number of additional bytes needed for the rr_moved directory (this
         may be zero).
        """

        if self._rr_moved_record.initialized:
            return 0

        if self._rr_moved_name is None:
            self._rr_moved_name = b'RR_MOVED'
        if self._rr_moved_rr_name is None:
            self._rr_moved_rr_name = b'rr_moved'

        # No rr_moved found, so we have to create it.
        rec = dr.DirectoryRecord()
        rec.new_dir(self.pvd, self._rr_moved_name,
                    self.pvd.root_directory_record(),
                    self.pvd.sequence_number(), self.rock_ridge,
                    self._rr_moved_rr_name, self.logical_block_size,
                    False, False, self.xa, 0o040555, time.time())
        num_bytes_to_add = self._add_child_to_dr(rec)

        self._create_dot(self.pvd, rec, self.rock_ridge, self.xa, 0o040555)
        self._create_dotdot(self.pvd, rec, self.rock_ridge, False, self.xa,
                            0o040555)

        # We always need to add an entry to the path table record.
        ptr = path_table_record.PathTableRecord()
        ptr.new_dir(self._rr_moved_name)
        num_bytes_to_add += self.logical_block_size + self._add_to_ptr_size(ptr)

        rec.set_ptr(ptr)

        self._rr_moved_record = rec

        return num_bytes_to_add

    def _calculate_eltorito_boot_info_table_csum(self, data_fp, data_len):
        # type: (BinaryIO, int) -> int
        """
        An internal method to calculate the checksum for an El Torito Boot Info
        Table.  This checksum is a simple 32-bit checksum over all of the data
        in the boot file, starting right after the Boot Info Table itself.

        Parameters:
         data_fp - The file object to read the input data from.
         data_len - The length of the input file.
        Returns:
         An integer representing the 32-bit checksum for the boot info table.
        """
        # Read the boot file so we can calculate the checksum over it.
        num_sectors = utils.ceiling_div(data_len, self.logical_block_size)
        csum = 0
        curr_sector = 0
        while curr_sector < num_sectors:
            block = data_fp.read(self.logical_block_size)
            block = block.ljust(2048, b'\x00')
            i = 0
            if curr_sector == 0:
                # The first 64 bytes are not included in the checksum.
                i = 64
            while i < len(block):
                tmp, = struct.unpack_from('<L', block[:i + 4], i)
                csum += tmp
                csum = csum & 0xffffffff
                i += 4

            curr_sector += 1

        return csum

    def _check_for_eltorito_boot_info_table(self, ino):
        # type: (inode.Inode) -> None
        """
        An internal method to check a boot directory record to see if it has
        an El Torito Boot Info Table embedded inside of it.

        Parameters:
         ino - The Inode to check for a Boot Info Table.
        Returns:
         Nothing.
        """
        orig = self._cdfp.tell()
        with inode.InodeOpenData(ino, self.logical_block_size) as (data_fp, data_len):
            data_fp.seek(8, os.SEEK_CUR)
            bi_table = eltorito.EltoritoBootInfoTable()
            if bi_table.parse(self.pvd, data_fp.read(eltorito.EltoritoBootInfoTable.header_length()), ino):
                data_fp.seek(-24, os.SEEK_CUR)
                # Do a final check to make sure the checksum matches.
                csum = self._calculate_eltorito_boot_info_table_csum(data_fp,
                                                                     data_len)

                if csum == bi_table.csum:
                    ino.add_boot_info_table(bi_table)

        self._cdfp.seek(orig)

    def _check_rr_name(self, rr_name):
        # type: (Optional[str]) -> bytes
        """
        An internal method to check whether this ISO requires a Rock Ridge path.

        Parameters:
         rr_name - The Rock Ridge name.
        Returns:
         The Rock Ridge name in bytes if this is a Rock Ridge ISO, an empty byte string otherwise.
        """
        if self.rock_ridge:
            if not rr_name:
                raise pycdlibexception.PyCdlibInvalidInput('A rock ridge name must be passed for a rock-ridge ISO')

            if rr_name.count('/') != 0:
                raise pycdlibexception.PyCdlibInvalidInput('A rock ridge name must be relative')

            return rr_name.encode('utf-8')

        if rr_name:
            raise pycdlibexception.PyCdlibInvalidInput('A rock ridge name can only be specified for a rock-ridge ISO')

        return b''

    def _normalize_joliet_path(self, joliet_path):
        # type: (str) -> bytes
        """
        An internal method to check whether this ISO requires a Joliet path.
        If a Joliet path is required, the path is normalized and returned.

        Parameters:
         joliet_path - The joliet_path to normalize (if necessary).
        Returns:
         The normalized joliet_path if this ISO has Joliet, an empty byte string otherwise.
        """
        tmp_path = b''
        if self.joliet_vd is not None:
            if not joliet_path:
                raise pycdlibexception.PyCdlibInvalidInput('A Joliet path must be passed for a Joliet ISO')
            tmp_path = utils.normpath(joliet_path)
        else:
            if joliet_path:
                raise pycdlibexception.PyCdlibInvalidInput('A Joliet path can only be specified for a Joliet ISO')

        return tmp_path

    def _link_eltorito(self, extent_to_inode):
        # type: (Dict[int, inode.Inode]) -> None
        """
        An internal method to link the El Torito entries into their
        corresponding Directory Records, creating new ones if they are
        'hidden'.  Should only be called on an El Torito ISO.

        Parameters:
         extent_to_inode - A map from extents to Inodes.
        Returns:
         Nothing.
        """
        if self.eltorito_boot_catalog is None:
            raise pycdlibexception.PyCdlibInternalError('Trying to link El Torito entries on a non-El Torito ISO')

        entries_to_assign = [self.eltorito_boot_catalog.initial_entry]
        for sec in self.eltorito_boot_catalog.sections:
            for entry in sec.section_entries:
                entries_to_assign.append(entry)

        for entry in entries_to_assign:
            entry_extent = entry.get_rba()
            if entry_extent in extent_to_inode:
                ino = extent_to_inode[entry_extent]
            else:
                ino = inode.Inode()
                ino.parse(entry_extent, entry.length(), self._cdfp,
                          self.logical_block_size)
                extent_to_inode[entry_extent] = ino
                self.inodes.append(ino)

            ino.linked_records.append((entry, False))
            entry.set_inode(ino)

    def _parse_udf_vol_descs(self, udf_extent_ad):
        # type: (udfmod.UDFExtentAD) -> udfmod.UDFDescriptorSequence
        """
        An internal method to parse a set of UDF Volume Descriptors.

        Parameters:
         udf_extent_ad - The UDF Extent of the anchor volume to parse.
        Returns:
         The UDFDescriptorSequence object that stores parsed objects.
        """
        self._seek_to_extent(udf_extent_ad.extent_location)
        vd_data = self._cdfp.read(udf_extent_ad.extent_length)

        return udfmod.parse_udf_vol_descs(vd_data, udf_extent_ad.extent_location,
                                          self.logical_block_size)

    def _parse_udf_descriptors(self):
        # type: () -> None
        """
        An internal method to parse the UDF descriptors on the ISO.  This should
        only be called if it the ISO has a valid UDF Volume Recognition Sequence
        at the beginning of the ISO.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        # Parse the anchors.  According to ECMA-167, Part 3, 8.4.2.1, there
        # must be anchors recorded in at least two of the three extent locations
        # 256, N-256, and N, where N is the total number of extents on the disc.
        # Note that one some ISOs, the PVD lies about the amount of space, so we
        # actually check what the PVD tells us is the end, but also the end of
        # the physical space on the ISO.  We'll preserve as many as we find,
        # with a minimum of two for a valid ISO.
        self._cdfp.seek(0, os.SEEK_END)
        last_physical_extent = (self._cdfp.tell() // self.logical_block_size) - 1
        last_pvd_extent = self.pvd.space_size - 1
        potential_anchor_locations = {256, last_pvd_extent - 256,
                                      last_pvd_extent, last_physical_extent,
                                      last_physical_extent - 256}

        for loc in potential_anchor_locations:
            self._seek_to_extent(loc)
            potential_anchor_data = self._cdfp.read(self.logical_block_size)
            potential_anchor = udfmod.parse_anchor(potential_anchor_data, loc)
            if potential_anchor is None:
                continue

            if len(self.udf_anchors) > 0 and potential_anchor != self.udf_anchors[0]:
                raise pycdlibexception.PyCdlibInvalidISO('Anchor points do not match')
            self.udf_anchors.append(potential_anchor)

        if len(self.udf_anchors) < 2:
            raise pycdlibexception.PyCdlibInvalidISO('Expected at least 2 UDF Anchors')

        # ECMA-167, Part 3, 8.4.2 says that the anchors identify the main volume
        # descriptor sequence, so look for it here.

        # Parse the Main Volume Descriptor Sequence.
        self.udf_main_descs = self._parse_udf_vol_descs(self.udf_anchors[0].main_vd)

        # ECMA-167, Part 3, 8.4.2 and 8.4.2.2 says that the anchors *may*
        # identify a reserve volume descriptor sequence.  10.2.3 says that
        # a reserve volume sequence is identified if the length is > 0.

        if self.udf_anchors[0].reserve_vd.extent_length > 0:
            # Parse the Reserve Volume Descriptor Sequence.
            self.udf_reserve_descs = self._parse_udf_vol_descs(self.udf_anchors[0].reserve_vd)

        # ECMA-167, Part 3, 10.6.12 says that the integrity sequence extent
        # only exists if the length is > 0.
        if self.udf_main_descs.logical_volumes[0].integrity_sequence.extent_length > 0:
            # Parse the Logical Volume Integrity Sequence.
            self._seek_to_extent(self.udf_main_descs.logical_volumes[0].integrity_sequence.extent_location)
            integrity_data = self._cdfp.read(self.udf_main_descs.logical_volumes[0].integrity_sequence.extent_length)

            ulvi, ulvi_term = udfmod.parse_logical_volume_integrity(integrity_data,
                                                                    self.udf_main_descs.logical_volumes[0].integrity_sequence.extent_location,
                                                                    self.logical_block_size)
            self.udf_logical_volume_integrity = ulvi
            self.udf_logical_volume_integrity_terminator = ulvi_term

        # Now look for the File Set Descriptor.
        current_extent = self.udf_main_descs.partitions[0].part_start_location
        self._seek_to_extent(current_extent)
        # Read the data for the File Set and File Terminator together
        file_set_and_term_data = self._cdfp.read(2 * self.logical_block_size)

        self.udf_file_set, self.udf_file_set_terminator = udfmod.parse_file_set(file_set_and_term_data,
                                                                                current_extent,
                                                                                self.logical_block_size)

    def _walk_udf_directories(self, extent_to_inode):
        # type: (Dict[int, inode.Inode]) -> None
        """
        An internal method to walk a UDF filesystem and add all the metadata to
        this object.

        Parameters:
         extent_to_inode - A map from extent numbers to Inodes.
        Returns:
         Nothing.
        """
        part_start = self.udf_main_descs.partitions[0].part_start_location

        abs_file_entry_extent = part_start + self.udf_file_set.root_dir_icb.log_block_num
        self._seek_to_extent(abs_file_entry_extent)
        icbdata = self._cdfp.read(self.udf_file_set.root_dir_icb.extent_length)
        self.udf_root = udfmod.parse_file_entry(icbdata,
                                                abs_file_entry_extent,
                                                self.udf_file_set.root_dir_icb.log_block_num,
                                                None)

        udf_file_entries = collections.deque([self.udf_root])
        while udf_file_entries:
            udf_file_entry = udf_file_entries.popleft()

            if udf_file_entry is None:
                continue

            for desc in udf_file_entry.alloc_descs:
                abs_file_ident_extent = part_start + desc.log_block_num
                self._seek_to_extent(abs_file_ident_extent)
                self._cdfp.seek(desc.offset, 1)
                data = self._cdfp.read(desc.extent_length)
                offset = 0
                while offset < len(data):
                    current_extent = (abs_file_ident_extent * self.logical_block_size + offset) // self.logical_block_size

                    file_ident, bytes_forward = udfmod.parse_file_ident(data[offset:],
                                                                        current_extent,
                                                                        part_start,
                                                                        udf_file_entry)
                    offset += bytes_forward

                    if file_ident.is_parent():
                        # For a parent, no further work to do.
                        udf_file_entry.track_file_ident_desc(file_ident)
                        continue

                    abs_file_entry_extent = part_start + file_ident.icb.log_block_num
                    self._seek_to_extent(abs_file_entry_extent)
                    icbdata = self._cdfp.read(file_ident.icb.extent_length)
                    next_entry = udfmod.parse_file_entry(icbdata,
                                                         abs_file_entry_extent,
                                                         file_ident.icb.log_block_num,
                                                         udf_file_entry)

                    # For a non-parent, we delay adding this to the list of
                    # fi_descs until after we check whether this is a valid
                    # entry or not.
                    udf_file_entry.track_file_ident_desc(file_ident)

                    if next_entry is None:
                        if file_ident.is_dir():
                            raise pycdlibexception.PyCdlibInvalidISO('Empty UDF File Entry for directories are not allowed')

                        # If the next_entry is None, then we just skip the
                        # rest of the code dealing with the entry and the
                        # Inode.
                        continue

                    file_ident.file_entry = next_entry
                    next_entry.file_ident = file_ident

                    if file_ident.is_dir():
                        udf_file_entries.append(next_entry)
                    else:
                        if next_entry.get_data_length() > 0:
                            abs_file_data_extent = part_start + next_entry.alloc_descs[0].log_block_num
                        else:
                            abs_file_data_extent = 0
                        if self.eltorito_boot_catalog is not None and abs_file_data_extent == self.eltorito_boot_catalog.extent_location():
                            self.eltorito_boot_catalog.add_dirrecord(next_entry)
                        else:
                            if abs_file_data_extent in extent_to_inode:
                                ino = extent_to_inode[abs_file_data_extent]
                            else:
                                ino = inode.Inode()
                                ino.parse(abs_file_data_extent,
                                          next_entry.get_data_length(),
                                          self._cdfp, self.logical_block_size)
                                extent_to_inode[abs_file_data_extent] = ino
                                self.inodes.append(ino)

                            ino.linked_records.append((next_entry, False))
                            next_entry.inode = ino

    def _open_fp(self, fp):
        # type: (IO) -> None
        """
        An internal method to open an existing ISO for inspection and
        modification.  Note that the file object passed in here must stay open
        for the lifetime of this object, as the PyCdlib class uses it internally
        to do writing and reading operations.

        Parameters:
         fp - The file object containing the ISO to open up.
        Returns:
         Nothing.
        """
        if hasattr(fp, 'mode') and 'b' not in fp.mode:
            raise pycdlibexception.PyCdlibInvalidInput("The file to open must be in binary mode (add 'b' to the open flags)")

        # If this is a Windows platform, and the file-like object name starts
        # with \\.\, this is a "raw" Windows device and we have to treat it
        # specially.
        if sys.platform == 'win32' and hasattr(fp, 'name') and fp.name.startswith("\\\\.\\"):
            fp = utils.Win32RawDevice(fp.name)

        self._cdfp = fp

        # Get the Primary Volume Descriptor (pvd), the set of Supplementary
        # Volume Descriptors (svds), the set of Volume Partition
        # Descriptors (vpds), the set of Boot Records (brs), and the set of
        # Volume Descriptor Set Terminators (vdsts)
        self._parse_volume_descriptors()

        self.logical_block_size = self.pvd.logical_block_size()

        old = self._cdfp.tell()
        self._cdfp.seek(0)
        tmp_isohybrid = isohybrid.IsoHybrid()
        if tmp_isohybrid.parse(self._cdfp.read(16 * 2048)):
            if tmp_isohybrid.efi:
                # If we have an EFI partition, we now need to go find the backup
                # LBA and parse that.  The backup_lba attribute tells us the
                # location of the GPT Header, which is *after* the backup
                # partition information, so we first parse that, then go find
                # out how many more partitions we need to parse.
                self._cdfp.seek(tmp_isohybrid.primary_gpt.header.backup_lba * 512)
                tmp_isohybrid.parse_secondary_gpt_header(self._cdfp.read(512))

                self._cdfp.seek((tmp_isohybrid.secondary_gpt.header.current_lba * 512) - (tmp_isohybrid.secondary_gpt.header.num_parts * 128))
                tmp_isohybrid.parse_secondary_gpt_partitions(self._cdfp.read(tmp_isohybrid.secondary_gpt.header.num_parts * 128))

            # We only save the object if it turns out to be a valid IsoHybrid.
            self.isohybrid_mbr = tmp_isohybrid
        self._cdfp.seek(old)

        if self.pvd.application_use[141:149] == b'CD-XA001':
            self.xa = True

        for br in self.brs:
            self._check_and_parse_eltorito(br)

        # Now that we have the PVD, parse the Path Tables according to Ecma-119
        # section 9.4.  We want to ensure that the big endian versions agree
        # with the little endian ones (to make sure it is a valid ISO).

        # Little Endian first.
        le_ptrs, extent_to_ptr = self._parse_path_table(self.pvd.path_table_size(),
                                                        self.pvd.path_table_location_le)

        # Big Endian next.
        tmp_be_ptrs, e_unused = self._parse_path_table(self.pvd.path_table_size(),
                                                       self.pvd.path_table_location_be)

        for index, ptr in enumerate(le_ptrs):
            if not ptr.equal_to_be(tmp_be_ptrs[index]):
                raise pycdlibexception.PyCdlibInvalidISO('Little-endian and big-endian path table records do not agree')

        self.interchange_level = 1
        for svd in self.svds:
            if svd.version == 2 and svd.file_structure_version == 2:
                self.interchange_level = 4
                break

        extent_to_inode = {}  # type: Dict[int, inode.Inode]

        # Parse all of the files starting from the PVD root directory record.
        ic_level, lastbyte = self._walk_directories(self.pvd, extent_to_ptr,
                                                    extent_to_inode, le_ptrs)

        if self.eltorito_boot_catalog is not None:
            if not self.eltorito_boot_catalog.dirrecords:
                # We expect the boot catalog to have at *least* one directory
                # record attached.  If we run across an ISO that doesn't have
                # that, we attach a "fake" one so that later steps do the right
                # thing.  Note that this will never be written out since we
                # don't add it to the main PVD directory structure.
                new_record = dr.DirectoryRecord()
                new_record.new_file(self.pvd, self.logical_block_size,
                                    b'FAKEELT.;1',
                                    self.pvd.root_directory_record(), 0, '',
                                    b'', False, 0, time.time())
                self.eltorito_boot_catalog.add_dirrecord(new_record)

        self.interchange_level = max(self.interchange_level, ic_level)

        # After we have walked the directories we look to see if all of the
        # El Torito entries have corresponding directory records.  If not, the
        # El Torito records may be 'hidden' or 'unlinked', meaning they have no
        # corresponding directory record in the ISO filesystem.  In order to
        # accommodate the rest of the system which expects them to have
        # directory records, we use fake directory records that don't get
        # written out.
        #
        # Note that we specifically do *not* add these to any sort of parent;
        # that way, we don't run afoul of any checks that adding a child to a
        # parent might have.  This means that if we do ever want to unhide this
        # entry, we'll have to do some additional work to give it a real name
        # and link it to the appropriate parent.
        if self.eltorito_boot_catalog is not None:
            self._link_eltorito(extent_to_inode)

            # Now that everything has a dirrecord, see if we have a boot
            # info table.
            self._check_for_eltorito_boot_info_table(self.eltorito_boot_catalog.initial_entry.inode)
            for sec in self.eltorito_boot_catalog.sections:
                for entry in sec.section_entries:
                    self._check_for_eltorito_boot_info_table(entry.inode)

        # The PVD is finished.  Now look to see if we need to parse the SVD.
        for svd in self.svds:
            if (svd.flags & 0x1) == 0 and svd.escape_sequences[:3] in (b'%/@', b'%/C', b'%/E'):
                if self.joliet_vd is not None:
                    raise pycdlibexception.PyCdlibInvalidISO('Only a single Joliet SVD is supported')

                self.joliet_vd = svd

                le_ptrs, joliet_extent_to_ptr = self._parse_path_table(svd.path_table_size(),
                                                                       svd.path_table_location_le)

                tmp_be_ptrs, j_unused = self._parse_path_table(svd.path_table_size(),
                                                               svd.path_table_location_be)

                for index, ptr in enumerate(le_ptrs):
                    if not ptr.equal_to_be(tmp_be_ptrs[index]):
                        raise pycdlibexception.PyCdlibInvalidISO('Joliet little-endian and big-endian path table records do not agree')

                self._walk_directories(svd, joliet_extent_to_ptr,
                                       extent_to_inode, le_ptrs)
            elif svd.version == 2 and svd.file_structure_version == 2:
                if self.enhanced_vd is not None:
                    raise pycdlibexception.PyCdlibInvalidISO('Only a single enhanced VD is supported')
                self.enhanced_vd = svd

        # We've seen ISOs in the wild (Office XP) that have a PVD space size
        # that is smaller than the location of the last directory record
        # extent + length.  If we see this, automatically update the size in the
        # PVD (and any SVDs) so that subsequent operations will be correct.
        if lastbyte > self.pvd.space_size * self.logical_block_size:
            new_pvd_size = utils.ceiling_div(lastbyte, self.logical_block_size)
            for pvd in self.pvds:
                pvd.space_size = new_pvd_size
            if self.joliet_vd is not None:
                self.joliet_vd.space_size = new_pvd_size
            if self.enhanced_vd is not None:
                self.enhanced_vd.space_size = new_pvd_size

        # Look to see if this is a UDF volume.  It is one if we have a UDF BEA,
        # UDF NSR, and UDF TEA, in which case we parse the UDF descriptors and
        # walk the filesystem.
        if self._has_udf:
            self._parse_udf_descriptors()
            self._walk_udf_directories(extent_to_inode)

        # Now we look for the 'version' volume descriptor, common on ISOs made
        # with genisoimage or mkisofs.  This volume descriptor doesn't have any
        # specification, but from code inspection, it is either a completely
        # zero extent, or starts with 'MKI'.  Further, it starts directly after
        # the VDST, or directly after the UDF recognition sequence (if this is
        # a UDF ISO).  Thus, we go looking for it at those places, and add it
        # if we find it there.
        version_vd_extent = self.vdsts[0].extent_location() + 1
        if self._has_udf:
            version_vd_extent = self.udf_teas[0].extent_location() + 1

        version_vd = headervd.VersionVolumeDescriptor()
        self._cdfp.seek(version_vd_extent * self.logical_block_size)
        if version_vd.parse(self._cdfp.read(self.logical_block_size), version_vd_extent):
            self.version_vd = version_vd

        self._initialized = True

    def _get_and_write_fp(self, iso_path, outfp, blocksize):
        # type: (bytes, BinaryIO, int) -> None
        """
        An internal method to fetch a single file from the ISO and write it out
        to the file object.  The 'iso_path' argument can be a Joliet path, an
        ISO9660 path, or a Rock Ridge path.  In the case that the same pathname
        exists in multiple contexts, the Joliet data will be tried first,
        followed by ISO9660, followed by Rock Ridge.

        Parameters:
         iso_path - The absolute path to the file to get data from.
         outfp - The file object to write data to.
         blocksize - The blocksize to use when copying data.
        Returns:
         Nothing.
        """
        if self.joliet_vd is not None:
            try:
                self._get_file_from_iso_fp(outfp, blocksize, None, None,
                                           iso_path)
                return
            except pycdlibexception.PyCdlibInvalidInput as err:
                if str(err) == 'Could not find path':
                    pass
                else:
                    raise

        saved_exception = None
        try:
            self._get_file_from_iso_fp(outfp, blocksize, iso_path, None, None)
            return
        except pycdlibexception.PyCdlibException as err:
            if str(err) == 'Could not find path':
                saved_exception = err
            else:
                raise

        if self.rock_ridge != '':
            self._get_file_from_iso_fp(outfp, blocksize, None, iso_path, None)
            return

        if saved_exception is not None:
            raise saved_exception

    def _udf_get_file_from_iso_fp(self, outfp, blocksize, udf_path):
        # type: (BinaryIO, int, bytes) -> None
        """
        An internal method to fetch a single UDF file from the ISO and write it
        out to the file object.

        Parameters:
         outfp - The file object to write data to.
         blocksize - The number of bytes in each transfer.
         udf_path - The absolute UDF path to lookup on the ISO.
        Returns:
         Nothing.
        """
        if self.udf_root is None:
            raise pycdlibexception.PyCdlibInvalidInput('Cannot fetch a udf_path from a non-UDF ISO')

        (ident_unused, found_file_entry) = self._find_udf_record(udf_path)
        if found_file_entry is None:
            raise pycdlibexception.PyCdlibInvalidInput('Cannot get the contents of an empty UDF File Entry')

        if not found_file_entry.is_file():
            raise pycdlibexception.PyCdlibInvalidInput('Can only write out a file')

        if found_file_entry.inode is None:
            raise pycdlibexception.PyCdlibInvalidInput('Cannot write out an entry without data')

        if found_file_entry.get_data_length() > 0:
            with inode.InodeOpenData(found_file_entry.inode, self.logical_block_size) as (data_fp, data_len):
                utils.copy_data(data_len, blocksize, data_fp, outfp)

    def _get_file_from_iso_fp(self, outfp, blocksize, iso_path, rr_path,
                              joliet_path, encoding=None):
        # type: (BinaryIO, int, Optional[bytes], Optional[bytes], Optional[bytes], str | None) -> None
        """
        An internal method to fetch a single file from the ISO and write it out
        to the file object.

        Parameters:
         outfp - The file object to write data to.
         blocksize - The number of bytes in each transfer.
         iso_path - The absolute ISO9660 path to lookup on the ISO (exclusive
                    with rr_path and joliet_path).
         rr_path - The absolute Rock Ridge path to lookup on the ISO (exclusive
                   with iso_path and joliet_path).
         joliet_path - The absolute Joliet path to lookup on the ISO (exclusive
                       with iso_path and rr_path).
         encoding - The string encoding used for the path.
        Returns:
         Nothing.
        """
        if joliet_path is not None:
            if self.joliet_vd is None:
                raise pycdlibexception.PyCdlibInvalidInput('Cannot fetch a joliet_path from a non-Joliet ISO')
            found_record = self._find_joliet_record(joliet_path, encoding or 'utf-16_be')
        elif rr_path is not None:
            if not self.rock_ridge:
                raise pycdlibexception.PyCdlibInvalidInput('Cannot fetch a rr_path from a non-Rock Ridge ISO')
            found_record = self._find_rr_record(rr_path, encoding or 'utf-8')
        elif iso_path is not None:
            found_record = self._find_iso_record(iso_path, encoding or 'utf-8')
        else:
            raise pycdlibexception.PyCdlibInternalError('Invalid path passed to get_file_from_iso_fp')

        if found_record.is_dir():
            raise pycdlibexception.PyCdlibInvalidInput('Cannot write out a directory')

        if rr_path is not None or iso_path is not None:
            if found_record.is_symlink():
                # If this Rock Ridge record is a symlink, it has no data
                # associated with it, so it makes no sense to try and get the
                # data.  In theory, we could follow the symlink to the
                # appropriate place and get the data of the thing it points to.
                # However, Rock Ridge symlinks are allowed to point *outside*
                # of this ISO, so it is really not clear that this is something
                # we want to do.  For now we make the user follow the symlink
                # themselves if they want to get the data.  We can revisit this
                # decision in the future if we need to.
                raise pycdlibexception.PyCdlibInvalidInput('Symlinks have no data associated with them')

        if self.eltorito_boot_catalog is not None:
            for rec in self.eltorito_boot_catalog.dirrecords:
                if isinstance(rec, udfmod.UDFFileEntry):
                    continue
                if rec.file_ident == found_record.file_ident and rec.parent == found_record.parent:
                    recdata = self.eltorito_boot_catalog.record()
                    outfp.write(recdata)
                    utils.zero_pad(outfp, len(recdata), self.logical_block_size)
                    return

        if found_record.inode is None:
            raise pycdlibexception.PyCdlibInvalidInput('Cannot write out a file without data')

        while found_record.get_data_length() > 0:
            with inode.InodeOpenData(found_record.inode, self.logical_block_size) as (data_fp, data_len):
                # Copy the data into the output file descriptor.  If a boot info
                # table is present, overlay the table over bytes 8-64 of the
                # file.  Note that we never return more bytes than the length
                # of the file, so the boot info table may get truncated.
                if found_record.inode.boot_info_table is not None:
                    header_len = min(data_len, 8)
                    outfp.write(data_fp.read(header_len))
                    data_len -= header_len
                    if data_len > 0:
                        bi_rec = found_record.inode.boot_info_table.record()
                        table_len = min(data_len, len(bi_rec))
                        outfp.write(bi_rec[:table_len])
                        data_len -= table_len
                        if data_len > 0:
                            data_fp.seek(len(bi_rec), os.SEEK_CUR)
                            utils.copy_data(data_len, blocksize, data_fp, outfp)
                else:
                    utils.copy_data(data_len, blocksize, data_fp, outfp)

            if found_record.data_continuation is not None:
                found_record = found_record.data_continuation
            else:
                break

    class _WriteRange:
        """
        A class to store the offset and length of a written section of data.
        A sorted list of these is used to determine whether we are unintentionally
        spending time rewriting data that we have already written.
        """
        __slots__ = ('offset', 'length')

        def __init__(self, start, end):
            # type: (int, int) -> None
            self.offset = start
            self.length = start + (end - start)

        def __lt__(self, other):
            # When we go to insert this into the list, we determine if this one
            # overlaps with the one we are currently looking at.
            if range(max(other.offset, self.offset), min(other.length, self.length) + 1):
                raise pycdlibexception.PyCdlibInternalError('Overlapping write %s, %s' % (repr(self), repr(other)))
            return self.offset < other.offset

        def __repr__(self):
            return 'WriteRange: %s %s' % (self.offset, self.length)

    def _outfp_write_with_check(self, outfp, data, enable_overwrite_check=True):
        # type: (BinaryIO, bytes, bool) -> None
        """
        Internal method to write data out to the output file descriptor,
        ensuring that it doesn't go beyond the bounds of the ISO.

        Parameters:
         outfp - The file object to write to.
         data - The actual data to write.
         enable_overwrite_check - Whether to do overwrite checking if it is
                                  enabled.  Some pieces of code explicitly want
                                  to overwrite data, so this allows them to
                                  disable the checking.
        Returns:
         Nothing.
        """
        start = outfp.tell()
        outfp.write(data)
        if self._track_writes:
            # After the write, double check that we didn't write beyond the
            # boundary of the PVD, and raise a PyCdlibException if we do.
            end = outfp.tell()
            if end > self.pvd.space_size * self.logical_block_size:
                raise pycdlibexception.PyCdlibInternalError('Wrote past the end of the ISO! (%d > %d)' % (end, self.pvd.space_size * self.logical_block_size))

            if enable_overwrite_check:
                bisect.insort_left(self._write_check_list, self._WriteRange(start, end - 1))

    def _output_file_data(self, outfp, blocksize, ino):
        # type: (BinaryIO, int, inode.Inode) -> Generator
        """
        Internal method to write a directory record entry out.

        Parameters:
         outfp - The file object to write the data to.
         blocksize - The blocksize to use when writing the data out.
         ino - The Inode to write.
        Returns:
         The total number of bytes written out.
        """
        outfp.seek(ino.extent_location() * self.logical_block_size)
        start_offset = outfp.tell()
        with inode.InodeOpenData(ino, self.logical_block_size) as (data_fp, data_len):
            for len_copied in utils.copy_data_yield(data_len, blocksize, data_fp, outfp):  # pylint: disable=use-yield-from
                yield len_copied
            yield utils.zero_pad(outfp, data_len, self.logical_block_size)

        if self._track_writes:
            end = outfp.tell()
            bisect.insort_left(self._write_check_list,
                               self._WriteRange(start_offset, end - 1))

        # If this file is being used as a bootfile, and a boot info table is
        # present, patch the boot info table into offset 8 here.
        if ino.boot_info_table is not None:
            old = outfp.tell()
            outfp.seek(start_offset + 8)
            rec = ino.boot_info_table.record()
            self._outfp_write_with_check(outfp, rec, enable_overwrite_check=False)
            outfp.seek(old)

    class _Progress:
        """
        An inner class to deal with progress.
        """
        __slots__ = ('done', 'total', 'progress_cb', 'progress_opaque', '_call')

        def __init__(self, total, progress_cb, progress_opaque):
            # type: (int, Optional[Callable[[int, int, Any], None]], Optional[Any]) -> None
            self.done = 0
            self.total = total
            self.progress_cb = progress_cb
            self.progress_opaque = progress_opaque
            if self.progress_cb is not None:
                arglen = len(inspect.getfullargspec(self.progress_cb).args)

                if arglen == 2:
                    self._call = lambda done, total, opaque: self.progress_cb(done, total)  # type: ignore
                elif arglen == 3:
                    self._call = lambda done, total, opaque: self.progress_cb(done, total, opaque)  # type: ignore # pylint: disable=unnecessary-lambda
                else:
                    raise pycdlibexception.PyCdlibInvalidInput('The progress callback must take 2 or 3 arguments')
            else:
                self._call = lambda done, total, opaque: None

        def call(self, length):
            # type: (int) -> None
            """Add the length to done, then call progress_cb if it is not None."""
            self.done = min(self.done + length, self.total)
            self._call(self.done, self.total, self.progress_opaque)

        def finish(self):
            # type: () -> None
            """If the progress_cb is not None, call progress_cb with the final total."""
            # In almost all cases, this will cause self.done to wildly
            # overflow the total size.  However, with the hard cap in
            # call, this works just fine.
            self.call(self.total)

    def _write_directory_records(self, vd, outfp, progress):
        # type: (headervd.PrimaryOrSupplementaryVD, BinaryIO, PyCdlib._Progress) -> None
        """
        An internal method to write out the directory records from a particular
        Volume Descriptor.

        Parameters:
         vd - The Volume Descriptor to write the Directory Records from.
         outfp - The file object to write data to.
         progress - The _Progress object to use for outputting progress.
        Returns:
         Nothing.
        """
        le_ptr_offset = 0
        be_ptr_offset = 0
        dirs = collections.deque([vd.root_directory_record()])
        while dirs:
            curr = dirs.popleft()
            curr_dirrecord_offset = 0
            if curr.is_dir():
                if curr.ptr is None:
                    raise pycdlibexception.PyCdlibInternalError('Directory has no Path Table Record')

                # Little Endian PTR
                outfp.seek(vd.path_table_location_le * self.logical_block_size + le_ptr_offset)
                ret = curr.ptr.record_little_endian()
                self._outfp_write_with_check(outfp, ret)
                le_ptr_offset += len(ret)

                # Big Endian PTR
                outfp.seek(vd.path_table_location_be * self.logical_block_size + be_ptr_offset)
                ret = curr.ptr.record_big_endian()
                self._outfp_write_with_check(outfp, ret)
                be_ptr_offset += len(ret)
                progress.call(curr.get_data_length())

            dir_extent = curr.extent_location()
            for child in curr.children:
                # First write out the directory record entry for all children.
                recstr = child.record()
                if (curr_dirrecord_offset + len(recstr)) > self.logical_block_size:
                    dir_extent += 1
                    curr_dirrecord_offset = 0
                outfp.seek(dir_extent * self.logical_block_size + curr_dirrecord_offset)
                # Now write out the child.
                self._outfp_write_with_check(outfp, recstr)
                curr_dirrecord_offset += len(recstr)

                if child.rock_ridge is not None:
                    if child.rock_ridge.dr_entries.ce_record is not None:
                        # The child has a continue block, so write it out here.
                        ce_rec = child.rock_ridge.dr_entries.ce_record
                        outfp.seek(ce_rec.bl_cont_area * self.logical_block_size + ce_rec.offset_cont_area)
                        rec = child.rock_ridge.record_ce_entries()
                        self._outfp_write_with_check(outfp, rec)
                        progress.call(len(rec))

                    if child.rock_ridge.child_link_record_exists():
                        continue

                if child.is_dir():
                    # If the child is a directory, and is not dot or dotdot,
                    # descend into it to look at the children.
                    if not child.is_dot() and not child.is_dotdot():
                        dirs.append(child)

    def _write_udf_descs(self, descs, outfp, progress):
        # type: (udfmod.UDFDescriptorSequence, BinaryIO, PyCdlib._Progress) -> None
        """
        An internal method to write out a UDF Descriptor sequence.

        Parameters:
         descs - The UDF Descriptors object to write out.
         outfp - The output file descriptor to use for writing.
         progress - The _Progress object to use for updating progress.
        Returns:
         Nothing.
        """
        for pvd in descs.pvds:
            outfp.seek(pvd.extent_location() * self.logical_block_size)
            rec = pvd.record()
            self._outfp_write_with_check(outfp, rec)
            progress.call(len(rec))

        if descs.desc_pointer.initialized:
            outfp.seek(descs.desc_pointer.extent_location() * self.logical_block_size)
            rec = descs.desc_pointer.record()
            self._outfp_write_with_check(outfp, rec)
            progress.call(len(rec))

        for impl_use in descs.impl_use:
            outfp.seek(impl_use.extent_location() * self.logical_block_size)
            rec = impl_use.record()
            self._outfp_write_with_check(outfp, rec)
            progress.call(len(rec))

        for partition in descs.partitions:
            outfp.seek(partition.extent_location() * self.logical_block_size)
            rec = partition.record()
            self._outfp_write_with_check(outfp, rec)
            progress.call(len(rec))

        for logical_volume in descs.logical_volumes:
            outfp.seek(logical_volume.extent_location() * self.logical_block_size)
            rec = logical_volume.record()
            self._outfp_write_with_check(outfp, rec)
            progress.call(len(rec))

        for unallocated_space in descs.unallocated_space:
            outfp.seek(unallocated_space.extent_location() * self.logical_block_size)
            rec = unallocated_space.record()
            self._outfp_write_with_check(outfp, rec)
            progress.call(len(rec))

        if descs.terminator.initialized:
            outfp.seek(descs.terminator.extent_location() * self.logical_block_size)
            rec = descs.terminator.record()
            self._outfp_write_with_check(outfp, rec)
            progress.call(len(rec))

    def _write_fp(self, outfp, blocksize, progress_cb, progress_opaque):
        # type: (BinaryIO, int, Optional[Callable[[int, int, Any], None]], Optional[Any]) -> None
        """
        Write a properly formatted ISO out to the file object passed in.  This
        also goes by the name of 'mastering'.

        Parameters:
         outfp - The file object to write the data to.
         blocksize - The blocksize to use when copying data.
         progress_cb - If not None, a function to call as the write call does its
                       work.  The callback function must have a signature of:
                       def func(done, total, progress_data).
         progress_opaque - User data to be passed to the progress callback.
        Returns:
         Nothing.
        """
        if hasattr(outfp, 'mode') and 'b' not in outfp.mode:
            raise pycdlibexception.PyCdlibInvalidInput("The file to write out must be in binary mode (add 'b' to the open flags)")

        if self._needs_reshuffle:
            self._reshuffle_extents()

        self._write_check_list = []
        outfp.seek(0)

        progress = self._Progress(self.pvd.space_size * self.logical_block_size, progress_cb, progress_opaque)
        progress.call(0)

        if self.isohybrid_mbr is not None:
            self._outfp_write_with_check(outfp,
                                         self.isohybrid_mbr.record(self.pvd.space_size * self.logical_block_size))

        outfp.seek(self.pvd.extent_location() * self.logical_block_size)

        # First write out the PVDs.
        for pvd in self.pvds:
            rec = pvd.record()
            self._outfp_write_with_check(outfp, rec)
            progress.call(len(rec))

        # Next write out the boot records.
        for br in self.brs:
            outfp.seek(br.extent_location() * self.logical_block_size)
            rec = br.record()
            self._outfp_write_with_check(outfp, rec)
            progress.call(len(rec))

        # Next write out the SVDs.
        for svd in self.svds:
            outfp.seek(svd.extent_location() * self.logical_block_size)
            rec = svd.record()
            self._outfp_write_with_check(outfp, rec)
            progress.call(len(rec))

        # Next write out the Volume Descriptor Terminators.
        for vdst in self.vdsts:
            outfp.seek(vdst.extent_location() * self.logical_block_size)
            rec = vdst.record()
            self._outfp_write_with_check(outfp, rec)
            progress.call(len(rec))

        # Next write out the UDF Volume Recognition sequence (if this ISO has UDF).
        if self._has_udf:
            for bea in self.udf_beas:
                outfp.seek(bea.extent_location() * self.logical_block_size)
                rec = bea.record()
                self._outfp_write_with_check(outfp, rec)
                progress.call(len(rec))

            for boot in self.udf_boots:
                outfp.seek(boot.extent_location() * self.logical_block_size)
                rec = boot.record()
                self._outfp_write_with_check(outfp, rec)
                progress.call(len(rec))

            outfp.seek(self.udf_nsr.extent_location() * self.logical_block_size)
            rec = self.udf_nsr.record()
            self._outfp_write_with_check(outfp, rec)
            progress.call(len(rec))

            for tea in self.udf_teas:
                outfp.seek(tea.extent_location() * self.logical_block_size)
                rec = tea.record()
                self._outfp_write_with_check(outfp, rec)
                progress.call(len(rec))

        # Next write out the version block if it exists.
        if self.version_vd is not None:
            outfp.seek(self.version_vd.extent_location() * self.logical_block_size)
            rec = self.version_vd.record()
            self._outfp_write_with_check(outfp, rec)
            progress.call(len(rec))

        if self._has_udf:
            # Now the UDF Main and Reserved Volume Descriptor Sequence.
            self._write_udf_descs(self.udf_main_descs, outfp, progress)
            self._write_udf_descs(self.udf_reserve_descs, outfp, progress)

            # Now the UDF Logical Volume Integrity Sequence (if there is one).
            if self.udf_logical_volume_integrity is not None:
                outfp.seek(self.udf_logical_volume_integrity.extent_location() * self.logical_block_size)
                rec = self.udf_logical_volume_integrity.record()
                self._outfp_write_with_check(outfp, rec)
                progress.call(len(rec))

            if self.udf_logical_volume_integrity_terminator is not None:
                outfp.seek(self.udf_logical_volume_integrity_terminator.extent_location() * self.logical_block_size)
                rec = self.udf_logical_volume_integrity_terminator.record()
                self._outfp_write_with_check(outfp, rec)
                progress.call(len(rec))

        # Now the UDF Anchor Points (if there are any).
        for anchor in self.udf_anchors:
            outfp.seek(anchor.extent_location() * self.logical_block_size)
            rec = anchor.record()
            self._outfp_write_with_check(outfp, rec)
            progress.call(len(rec))

        # In theory, the Path Table Records (for both the PVD and SVD) get
        # written out next.  Since we store them along with the Directory
        # Records, however, we will write them out along with the directory
        # records instead.

        # Now write out the El Torito Boot Catalog if it exists.
        if self.eltorito_boot_catalog is not None:
            outfp.seek(self.eltorito_boot_catalog.extent_location() * self.logical_block_size)
            rec = self.eltorito_boot_catalog.record()
            self._outfp_write_with_check(outfp, rec)
            progress.call(len(rec))

        # Now write out the ISO9660 directory records.
        self._write_directory_records(self.pvd, outfp, progress)

        # Now write out the Joliet directory records, if they exist.
        if self.joliet_vd is not None:
            self._write_directory_records(self.joliet_vd, outfp, progress)

        # Now write out the UDF directory records, if they exist.
        if self.udf_root is not None:
            # Write out the UDF File Sets.
            outfp.seek(self.udf_file_set.extent_location() * self.logical_block_size)
            rec = self.udf_file_set.record()
            self._outfp_write_with_check(outfp, rec)
            progress.call(len(rec))

            if self.udf_file_set_terminator is not None:
                outfp.seek(self.udf_file_set_terminator.extent_location() * self.logical_block_size)
                rec = self.udf_file_set_terminator.record()
                self._outfp_write_with_check(outfp, rec)
                progress.call(len(rec))

            written_file_entry_inodes = set()
            udf_file_entries = collections.deque([(self.udf_root, True)])  # type: Deque[Tuple[Optional[udfmod.UDFFileEntry], bool]]
            while udf_file_entries:
                udf_file_entry, isdir = udf_file_entries.popleft()

                if udf_file_entry is None:
                    continue

                if udf_file_entry.inode is None or not id(udf_file_entry.inode) in written_file_entry_inodes:
                    outfp.seek(udf_file_entry.extent_location() * self.logical_block_size)
                    rec = udf_file_entry.record()
                    self._outfp_write_with_check(outfp, rec)
                    progress.call(len(rec))
                    written_file_entry_inodes.add(id(udf_file_entry.inode))

                if isdir:
                    outfp.seek(udf_file_entry.fi_descs[0].extent_location() * self.logical_block_size)
                    # FIXME: for larger directories, we'll actually need to
                    # iterate over the alloc_descs and write them
                    for fi_desc in udf_file_entry.fi_descs:
                        rec = fi_desc.record()
                        self._outfp_write_with_check(outfp, rec)
                        progress.call(len(rec))
                        if not fi_desc.is_parent():
                            udf_file_entries.append((fi_desc.file_entry, fi_desc.is_dir()))

        # Now write out the actual files.  In many cases we haven't yet read the
        # file out of the original, so do that here.
        for ino in self.inodes:
            if ino.get_data_length() > 0:
                for len_copied in self._output_file_data(outfp, blocksize, ino):
                    progress.call(len_copied)

        # Pad out to the total size of the disk, in case that the last thing
        # written is shorter than a full logical block size.  Not all file-like
        # objects support truncate() to grow a file, so do it the old-fashioned
        # way by seeking to end - 1 and writing a padding '\x00' byte.
        outfp.seek(0, os.SEEK_END)
        total_size = self.pvd.space_size * self.logical_block_size
        if outfp.tell() != total_size:
            outfp.seek(total_size - 1)
            outfp.write(b'\x00')

        if self.isohybrid_mbr is not None:
            outfp.seek(0, 2)
            outfp.write(self.isohybrid_mbr.record_padding(self.pvd.space_size * self.logical_block_size))
            if self.isohybrid_mbr.efi:
                outfp.seek((self.isohybrid_mbr.secondary_gpt.header.current_lba * 512) - (self.isohybrid_mbr.secondary_gpt.header.num_parts * 128))
                outfp.write(self.isohybrid_mbr.secondary_gpt.record())

        progress.finish()

    def _update_rr_ce_entry(self, rec):
        # type: (dr.DirectoryRecord) -> int
        """
        An internal method to update the Rock Ridge CE entry for the given
        record.

        Parameters:
         rec - The record to update the Rock Ridge CE entry for (if it exists).
        Returns:
         The number of additional bytes needed for this Rock Ridge CE entry.
        """
        if rec.rock_ridge is not None and rec.rock_ridge.dr_entries.ce_record is not None:
            celen = rec.rock_ridge.dr_entries.ce_record.len_cont_area
            added_block, block, offset = self.pvd.add_rr_ce_entry(celen)
            rec.rock_ridge.update_ce_block(block)
            rec.rock_ridge.dr_entries.ce_record.update_offset(offset)
            if added_block:
                return self.logical_block_size

        return 0

    def _finish_add(self, num_bytes_to_add, num_partition_bytes_to_add):
        # type: (int, int) -> None
        """
        An internal method to do all of the accounting needed whenever
        something is added to the ISO.  This method should only be called by
        public API methods.

        Parameters:
         num_bytes_to_add - The number of additional bytes to add to all
                            descriptors.
         num_partition_bytes_to_add - The number of additional bytes to add to
                                      the partition if this is a UDF file.
        Returns:
         Nothing.
        """
        for pvd in self.pvds:
            pvd.add_to_space_size(num_bytes_to_add + num_partition_bytes_to_add)
        if self.joliet_vd is not None:
            self.joliet_vd.add_to_space_size(num_bytes_to_add + num_partition_bytes_to_add)

        if self.enhanced_vd is not None:
            self.enhanced_vd.copy_sizes(self.pvd)

        if self.udf_root is not None:
            num_extents_to_add = utils.ceiling_div(num_partition_bytes_to_add,
                                                   self.logical_block_size)

            self.udf_main_descs.partitions[0].part_length += num_extents_to_add
            self.udf_reserve_descs.partitions[0].part_length += num_extents_to_add
            if self.udf_logical_volume_integrity is not None:
                self.udf_logical_volume_integrity.size_tables[0] += num_extents_to_add

        if self._always_consistent:
            self._reshuffle_extents()
        else:
            self._needs_reshuffle = True

    def _finish_remove(self, num_bytes_to_remove, is_partition):
        # type: (int, bool) -> None
        """
        An internal method to do all of the accounting needed whenever
        something is removed from the ISO.  This method should only be called
        by public API methods.

        Parameters:
         num_bytes_to_remove - The number of additional bytes to remove from the descriptors.
         is_partition - Whether these bytes are part of a UDF partition.
        Returns:
         Nothing.
        """
        for pvd in self.pvds:
            pvd.remove_from_space_size(num_bytes_to_remove)
        if self.joliet_vd is not None:
            self.joliet_vd.remove_from_space_size(num_bytes_to_remove)

        if self.enhanced_vd is not None:
            self.enhanced_vd.copy_sizes(self.pvd)

        if self.udf_root is not None and is_partition:
            num_extents_to_remove = utils.ceiling_div(num_bytes_to_remove,
                                                      self.logical_block_size)

            self.udf_main_descs.partitions[0].part_length -= num_extents_to_remove
            self.udf_reserve_descs.partitions[0].part_length -= num_extents_to_remove
            if self.udf_logical_volume_integrity is not None:
                self.udf_logical_volume_integrity.size_tables[0] -= num_extents_to_remove

        if self._always_consistent:
            self._reshuffle_extents()
        else:
            self._needs_reshuffle = True

    def _add_hard_link_to_inode(self, data_ino, length, file_mode,
                                boot_catalog_old, **kwargs):
        # type: (Optional[inode.Inode], int, int, bool, Optional[str]) -> int
        """
        Add a hard link to the ISO.  Hard links are alternate names for the
        same file contents that don't take up any additional space on the ISO.
        This API can be used to create hard links between two files on the
        ISO9660 filesystem, between two files on the Joliet filesystem, or
        between a file on the ISO9660 filesystem and the Joliet filesystem.
        In all cases, exactly one old path must be specified, and exactly one
        new path must be specified.

        Parameters:
         data_ino - The inode of the old record to link against.
         length - The length of the old record to link against.
         file_mode - The file mode of the old record to link against.
         boot_catalog_old - Whether this is a link to an old boot catalog.
         iso_new_path - The new path on the ISO9660 filesystem to link to.
         joliet_new_path - The new path on the Joliet filesystem to link to.
         rr_name - The Rock Ridge name to use for the new file if this is a
                   Rock Ridge ISO and the new path is on the ISO9660 filesystem.
         udf_new_path - The new path on the UDF filesystem to link to.
        Returns:
         The number of bytes to add to the descriptors.
        """
        num_new = 0
        iso_new_path = None
        joliet_new_path = None
        rr_name = b''
        udf_new_path = None
        new_rec = None  # type: Optional[Union[dr.DirectoryRecord, udfmod.UDFFileEntry]]
        for key, value in kwargs.items():
            if key == 'iso_new_path':
                if value is not None:
                    num_new += 1
                    iso_new_path = utils.normpath(value)
                    if not self.rock_ridge and self.interchange_level < 4:
                        _check_path_depth(iso_new_path)
            elif key == 'joliet_new_path':
                if value is not None:
                    num_new += 1
                    joliet_new_path = self._normalize_joliet_path(value)
            elif key == 'rr_name':
                if value is not None:
                    rr_name = self._check_rr_name(value)
            elif key == 'udf_new_path':
                if value is not None:
                    num_new += 1
                    udf_new_path = utils.normpath(value)
            else:
                raise pycdlibexception.PyCdlibInvalidInput('Unknown keyword %s' % (key))

        if num_new != 1:
            raise pycdlibexception.PyCdlibInvalidInput('Exactly one new path must be specified')
        if self.rock_ridge and iso_new_path is not None and not rr_name:
            raise pycdlibexception.PyCdlibInvalidInput('Rock Ridge name must be supplied for a Rock Ridge new path')

        num_bytes_to_add = 0
        if udf_new_path is None:
            if iso_new_path is not None:
                # ... to another file on the ISO9660 filesystem.
                (new_name, new_parent) = self._iso_name_and_parent_from_path(iso_new_path)
                _check_iso9660_filename(new_name, self.interchange_level)
                vd = self.pvd
                rr = self.rock_ridge
                xa = self.xa
            else:
                # Above we checked to make sure we got exactly one new path, so
                # we know for certain that this is Joliet.
                if self.joliet_vd is None:
                    raise pycdlibexception.PyCdlibInternalError('Tried to link to Joliet record on non-Joliet ISO')
                if joliet_new_path is None:
                    # This can really never happen, but here to make mypy happy
                    raise pycdlibexception.PyCdlibInternalError('Invalid joliet_new_path')
                # ... to a file on the Joliet filesystem.
                (new_name, new_parent) = self._joliet_name_and_parent_from_path(joliet_new_path)
                vd = self.joliet_vd
                rr = ''
                xa = False

            new_rec = dr.DirectoryRecord()
            new_rec.new_file(vd, length, new_name, new_parent,
                             vd.sequence_number(), rr, rr_name, xa, file_mode,
                             time.time())

            num_bytes_to_add += self._add_child_to_dr(new_rec)
            num_bytes_to_add += self._update_rr_ce_entry(new_rec)
        else:
            if self.udf_root is None:
                raise pycdlibexception.PyCdlibInvalidInput('Can only specify a UDF path for a UDF ISO')

            # UDF new path.
            (udf_name, udf_parent) = self._udf_name_and_parent_from_path(udf_new_path)

            file_ident = udfmod.UDFFileIdentifierDescriptor()
            file_ident.new(False, False, udf_name, udf_parent)
            num_new_extents = udf_parent.add_file_ident_desc(file_ident,
                                                             self.logical_block_size)
            num_bytes_to_add += num_new_extents * self.logical_block_size

            file_entry = udfmod.UDFFileEntry()
            file_entry.new(length, 'file', udf_parent, self.logical_block_size)
            file_ident.file_entry = file_entry
            file_entry.file_ident = file_ident
            if data_ino is None or data_ino.num_udf == 0:
                num_bytes_to_add += self.logical_block_size

            if data_ino is not None:
                data_ino.num_udf += 1

            new_rec = file_entry

            if self.udf_logical_volume_integrity is not None:
                self.udf_logical_volume_integrity.logical_volume_impl_use.num_files += 1

        if data_ino is not None and new_rec is not None:
            data_ino.linked_records.append((new_rec, iso_new_path is not None))
            new_rec.inode = data_ino

        if boot_catalog_old and new_rec is not None:
            if self.eltorito_boot_catalog is None:
                raise pycdlibexception.PyCdlibInternalError('Tried to link to El Torito on non-El Torito ISO')
            self.eltorito_boot_catalog.add_dirrecord(new_rec)

        return num_bytes_to_add

    def _add_fp(self, fp, length, manage_fp, iso_path, rr_name,
                joliet_path, udf_path, file_mode, eltorito_catalog):
        # type: (Optional[Union[BinaryIO, str]], int, bool, Optional[str], Optional[str], Optional[str], Optional[str], Optional[int], bool) -> int
        """
        An internal method to add a file to the ISO.  If the ISO contains Rock
        Ridge, then a Rock Ridge name must be provided.  If the ISO contains
        Joliet, then a Joliet path is not required but is highly recommended.
        Note that the caller must ensure that the file remains open for the
        lifetime of the ISO object, as the PyCdlib class uses the file
        descriptor internally when writing (mastering) the ISO.

        Parameters:
         fp - The file object to use for the contents of the new file.
         length - The length of the data for the new file.
         manage_fp - Whether or not pycdlib should internally manage the file
                     pointer.  It is faster to manage the file pointer
                     externally, but it is more convenient to have pycdlib do it
                     internally.
         iso_path - The ISO9660 absolute path to the file destination on the ISO.
         rr_name - The Rock Ridge name of the file destination on the ISO.
         joliet_path - The Joliet absolute path to the file destination on the ISO.
         udf_path - The UDF absolute path to the file destination on the ISO.
         file_mode - The POSIX file_mode to apply to this file.  This only
                     applies if this is a Rock Ridge ISO.  If this is None (the
                     default), the permissions from the original file are used.
         eltorito_catalog - Whether this entry represents an El Torito Boot
                            Catalog.
        Returns:
         The number of bytes to add to the descriptors.
        """

        if iso_path is None and joliet_path is None and udf_path is None:
            raise pycdlibexception.PyCdlibInvalidInput("At least one of 'iso_path', 'joliet_path', or 'udf_path' must be provided")

        fmode = 0
        if file_mode is not None:
            if not self.rock_ridge:
                raise pycdlibexception.PyCdlibInvalidInput('Can only specify a file mode for Rock Ridge ISOs')
            fmode = file_mode
        else:
            if self.rock_ridge:
                if fp is not None and not isinstance(fp, str):
                    # Python 3 implements the fileno method for all file-like
                    # objects, so we can't just use the existence of the method
                    # to tell whether it is available.  Instead, we try to
                    # assign it, and if that fails, then we assume it is not
                    # available.
                    try:
                        fileno = fp.fileno()
                        fmode = os.fstat(fileno).st_mode
                    except (AttributeError, io.UnsupportedOperation):
                        # We couldn't get the actual file mode of the file, so
                        # assume a conservative 444
                        fmode = 0o0100444
                else:
                    fmode = 0o0100444

        if length > (2**32) - 1 and self.interchange_level < 3:
            raise pycdlibexception.PyCdlibInvalidInput('File sizes for interchange level < 3 must be less than 4GiB')

        left = length
        offset = 0
        done = False
        num_bytes_to_add = 0
        ino = None
        while not done:
            # The maximum length we allow in one directory record is 0xfffff800
            # (this is taken from xorriso, though I don't really know why).
            thislen = min(left, 0xfffff800)

            ino = None
            if fp is not None:
                ino = inode.Inode()
                ino.new(thislen, fp, manage_fp, offset)

            num_bytes_to_add += thislen
            if iso_path:
                num_bytes_to_add += self._add_hard_link_to_inode(ino, thislen,
                                                                 fmode,
                                                                 eltorito_catalog,
                                                                 iso_new_path=iso_path,
                                                                 rr_name=rr_name)

            if joliet_path:
                # If this is a Joliet ISO, then we can re-use add_hard_link to do
                # most of the work.
                num_bytes_to_add += self._add_hard_link_to_inode(ino, thislen,
                                                                 fmode,
                                                                 eltorito_catalog,
                                                                 joliet_new_path=joliet_path)

            # This goes after the hard link so we only track the new Inode if
            # everything above succeeds
            if ino is not None:
                self.inodes.append(ino)

            left -= thislen
            offset += thislen
            if left == 0:
                done = True

        if udf_path:
            num_bytes_to_add += self._add_hard_link_to_inode(ino, length,
                                                             fmode,
                                                             eltorito_catalog,
                                                             udf_new_path=udf_path)

        return num_bytes_to_add

    def _rm_dr_link(self, rec):
        # type: (dr.DirectoryRecord) -> int
        """
        An internal method to remove a Directory Record link given the record.

        Parameters:
         rec - The Directory Record to remove.
        Returns:
         The number of bytes to remove from the ISO.
        """
        if not rec.is_file():
            raise pycdlibexception.PyCdlibInvalidInput('Cannot remove a directory with rm_hard_link (try rm_directory instead)')

        num_bytes_to_remove = 0

        done = False
        while not done:
            num_bytes_to_remove += self._remove_child_from_dr(rec,
                                                              rec.index_in_parent)

            if rec.inode is not None:
                found_index = None
                for index, reclink in enumerate(rec.inode.linked_records):
                    link = reclink[0]
                    if id(link) == id(rec):
                        found_index = index
                        break
                else:
                    # This should never happen.
                    raise pycdlibexception.PyCdlibInternalError('Could not find inode corresponding to record')

                del rec.inode.linked_records[found_index]

                # We only remove the size of the child from the ISO if there are
                # no other references to this file on the ISO.
                if not rec.inode.linked_records:
                    found_index = None
                    for index, ino in enumerate(self.inodes):
                        if id(ino) == id(rec.inode):
                            found_index = index
                            break
                    else:
                        # This should never happen.
                        raise pycdlibexception.PyCdlibInternalError('Could not find inode corresponding to record')
                    del self.inodes[found_index]

                    num_bytes_to_remove += rec.get_data_length()

            if rec.data_continuation is not None:
                rec = rec.data_continuation
            else:
                done = True

        return num_bytes_to_remove

    def _rm_udf_file_ident(self, parent, fi):
        # type: (udfmod.UDFFileEntry, bytes) -> int
        """
        An internal method to remove a UDF File Identifier from the parent
        and remove any space from the Logical Volume as necessary.

        Parameters:
         parent - The parent entry to remove the UDF File Identifier from.
         fi - The file identifier to remove.
        Returns:
         The number of bytes to remove from the ISO.
        """
        num_extents_to_remove = parent.remove_file_ident_desc_by_name(fi,
                                                                      self.logical_block_size)
        if self.udf_logical_volume_integrity is not None:
            self.udf_logical_volume_integrity.logical_volume_impl_use.num_files -= 1

        self._find_udf_record.cache_clear()  # pylint: disable=no-member

        return num_extents_to_remove * self.logical_block_size

    def _rm_udf_link(self, rec):
        # type: (udfmod.UDFFileEntry) -> int
        """
        An internal method to remove a UDF File Entry link.

        Parameters:
         rec - The UDF File Entry to remove.
        Returns:
         The number of bytes to remove from the ISO.
        """
        if not rec.is_file() and not rec.is_symlink():
            raise pycdlibexception.PyCdlibInvalidInput('Cannot remove a directory with rm_hard_link (try rm_directory instead)')

        # To remove something from UDF, we have to:
        # 1.  Remove it from the list of linked_records on the Inode.
        # 2.  If the number of links to the Inode is now 0, remove the Inode.
        # 3.  If the number of links to the UDF File Entry this uses is 0,
        #     remove the UDF File Entry.
        # 4.  Remove the UDF File Identifier from the parent.

        num_bytes_to_remove = 0

        if rec.inode is not None:
            # Step 1.
            found_index = None
            for index, reclink in enumerate(rec.inode.linked_records):
                link = reclink[0]
                if id(link) == id(rec):
                    found_index = index
                    break
            else:
                # This should never happen.
                raise pycdlibexception.PyCdlibInternalError('Could not find inode corresponding to record')

            del rec.inode.linked_records[found_index]
            rec.inode.num_udf -= 1

            # Step 2.
            if not rec.inode.linked_records:
                found_index = None
                for index, ino in enumerate(self.inodes):
                    if id(ino) == id(rec.inode):
                        found_index = index
                        break
                else:
                    # This should never happen.
                    raise pycdlibexception.PyCdlibInternalError('Could not find inode corresponding to record')
                del self.inodes[found_index]

                num_bytes_to_remove += rec.get_data_length()

            # Step 3.
            if rec.inode.num_udf == 0:
                num_bytes_to_remove += self.logical_block_size
        else:
            # If rec.inode is None, just remove space for the UDF File Entry.
            num_bytes_to_remove += self.logical_block_size

        # Step 4.
        if rec.parent is None:
            raise pycdlibexception.PyCdlibInternalError('Cannot remove a UDF record with no parent')
        if rec.file_ident is None:
            raise pycdlibexception.PyCdlibInternalError('Cannot remove a UDF record with no file identifier')
        return num_bytes_to_remove + self._rm_udf_file_ident(rec.parent, rec.file_ident.fi)

    def _add_joliet_dir(self, joliet_path):
        # type: (bytes) -> int
        """
        An internal method to add a joliet directory to the ISO.

        Parameters:
         joliet_path - The path to add to the Joliet portion of the ISO.
        Returns:
         The number of additional bytes needed on the ISO to fit this directory.
        """

        if self.joliet_vd is None:
            raise pycdlibexception.PyCdlibInternalError('Tried to add Joliet dir to non-Joliet ISO')

        (joliet_name, joliet_parent) = self._joliet_name_and_parent_from_path(joliet_path)

        rec = dr.DirectoryRecord()
        rec.new_dir(self.joliet_vd, joliet_name, joliet_parent,
                    self.joliet_vd.sequence_number(), '', b'',
                    self.logical_block_size, False, False,
                    False, -1, time.time())
        num_bytes_to_add = self._add_child_to_dr(rec)

        self._create_dot(self.joliet_vd, rec, '', False, -1)
        self._create_dotdot(self.joliet_vd, rec, '', False, False, -1)

        num_bytes_to_add += self.logical_block_size
        if self.joliet_vd.add_to_ptr_size(path_table_record.PathTableRecord.record_length(len(joliet_name))):
            num_bytes_to_add += 4 * self.logical_block_size

        ptr = path_table_record.PathTableRecord()
        ptr.new_dir(joliet_name)
        rec.set_ptr(ptr)

        return num_bytes_to_add

    def _rm_joliet_dir(self, joliet_path):
        # type: (bytes) -> int
        """
        An internal method to remove a directory from the Joliet portion of the ISO.

        Parameters:
         joliet_path - The Joliet directory to remove.
        Returns:
         The number of bytes to remove from the ISO for this Joliet directory.
        """
        if self.joliet_vd is None:
            raise pycdlibexception.PyCdlibInternalError('Tried to remove joliet dir from non-Joliet ISO')

        joliet_child = self._find_joliet_record(joliet_path)
        num_bytes_to_remove = joliet_child.get_data_length()
        num_bytes_to_remove += self._remove_child_from_dr(joliet_child,
                                                          joliet_child.index_in_parent)

        if joliet_child.ptr is None:
            raise pycdlibexception.PyCdlibInternalError('Joliet directory has no path table record; this should not be')
        if self.joliet_vd.remove_from_ptr_size(path_table_record.PathTableRecord.record_length(joliet_child.ptr.len_di)):
            num_bytes_to_remove += 4 * self.logical_block_size

        return num_bytes_to_remove

    def _get_iso_entry(self, iso_path, encoding='utf-8'):
        # type: (bytes, str) -> dr.DirectoryRecord
        """
        Internal method to get the directory record for an ISO path.

        Parameters:
         iso_path - The path on the ISO filesystem to look up the record for.
         encoding - The string encoding used for the path.
        Returns:
         A dr.DirectoryRecord object representing the path.
        """
        if self._needs_reshuffle:
            self._reshuffle_extents()

        return self._find_iso_record(iso_path, encoding)

    def _get_rr_entry(self, rr_path, encoding='utf-8'):
        # type: (bytes, str) -> dr.DirectoryRecord
        """
        Internal method to get the directory record for a Rock Ridge path.

        Parameters:
         rr_path - The Rock Ridge path on the ISO filesystem to look up the
                   record for.
         encoding - The string encoding used for the path.
        Returns:
         A dr.DirectoryRecord object representing the path.
        """
        if self._needs_reshuffle:
            self._reshuffle_extents()

        return self._find_rr_record(rr_path, encoding)

    def _get_joliet_entry(self, joliet_path, encoding='utf-16_be'):
        # type: (bytes, str) -> dr.DirectoryRecord
        """
        Internal method to get the directory record for a Joliet path.

        Parameters:
         joliet_path - The path on the Joliet filesystem to look up the record
                       for.
         encoding - The string encoding used for the path.
        Returns:
         A dr.DirectoryRecord object representing the path.
        """
        if self._needs_reshuffle:
            self._reshuffle_extents()

        return self._find_joliet_record(joliet_path, encoding)

    def _get_udf_entry(self, udf_path):
        # type: (str) -> udfmod.UDFFileEntry
        """
        Internal method to get the UDF File Entry for a particular path.

        Parameters:
         udf_path - The path on the UDF filesystem to look up the record for.
        Returns:
         A udfmod.UDFFileEntry object representing the path.
        """
        if self._needs_reshuffle:
            self._reshuffle_extents()

        return self._find_udf_record(utils.normpath(udf_path))[1]

    def _check_inode_against_eltorito(self, ino):
        # type: (inode.Inode) -> None
        """
        Internal method to check if an Inode is referenced by El Torito at all.

        Parameters:
         ino - The Inode to look for in the El Torito part of the ISO.
        Returns:
         Nothing.
        """
        if self.eltorito_boot_catalog is not None:
            eltorito_entries = set()
            eltorito_entries.add(id(self.eltorito_boot_catalog.initial_entry.inode))
            for sec in self.eltorito_boot_catalog.sections:
                for entry in sec.section_entries:
                    eltorito_entries.add(id(entry.inode))

            if id(ino) in eltorito_entries:
                raise pycdlibexception.PyCdlibInvalidInput("Cannot remove a file that is referenced by El Torito; use 'rm_eltorito' to remove El Torito, or use 'rm_hard_link' to hide the entry")

    def _rm_file_inodes(self, child):
        # type: (dr.DirectoryRecord) -> int
        """
        Internal method to remove all of the Directory Records and UDF File
        Entries linked to this child's Inode.

        Parameters:
         child - The DirectoryRecord to remove.
        Returns:
         The number of bytes to remove from the ISO.
        """

        if not child.is_file():
            raise pycdlibexception.PyCdlibInvalidInput('Cannot remove a directory with rm_file (try rm_directory instead)')

        # We also want to check to see if this Directory Record is currently
        # being used as an El Torito Boot Catalog, Initial Entry, or Section
        # Entry.  If it is, we throw an exception; we don't know if the user
        # meant to remove El Torito from this ISO, or if they meant to 'hide'
        # the entry, but we need them to call the correct API to let us know.
        if self.eltorito_boot_catalog is not None:
            if any(id(child) == id(rec) for rec in self.eltorito_boot_catalog.dirrecords):
                raise pycdlibexception.PyCdlibInvalidInput("Cannot remove a file that is referenced by El Torito; use 'rm_eltorito' to remove El Torito, or use 'rm_hard_link' to hide the entry")

        num_bytes_to_remove = 0

        if child.inode is None:
            num_bytes_to_remove += self._remove_child_from_dr(child,
                                                              child.index_in_parent)
        else:
            self._check_inode_against_eltorito(child.inode)
            while child.inode.linked_records:
                rec = child.inode.linked_records[0][0]

                if isinstance(rec, dr.DirectoryRecord):
                    num_bytes_to_remove += self._rm_dr_link(rec)
                elif isinstance(rec, udfmod.UDFFileEntry):
                    num_bytes_to_remove += self._rm_udf_link(rec)
                else:
                    # This should never happen.
                    raise pycdlibexception.PyCdlibInternalError('Saw a linked record that was neither ISO or UDF')

        return num_bytes_to_remove

    def _rm_file_via_iso_path(self, iso_path):
        # type: (str) -> int
        """
        Internal method to completely remove a file given an ISO path.

        Parameters:
         iso_path - The path to the file in the ISO9660 context to remove.
        Returns:
         The number of bytes to remove from the ISO.
        """

        iso_path_bytes = utils.normpath(iso_path)

        return self._rm_file_inodes(self._find_iso_record(iso_path_bytes))

    def _rm_file_via_joliet_path(self, joliet_path):
        # type: (str) -> int
        """
        Internal method to completely remove a file given a Joliet path.

        Parameters:
         joliet_path - The path to the file in the Joliet context to remove.
        Returns:
         The number of bytes to remove from the ISO.
        """

        joliet_path_bytes = self._normalize_joliet_path(joliet_path)

        return self._rm_file_inodes(self._find_joliet_record(joliet_path_bytes))

    def _rm_file_via_udf_path(self, udf_path):
        # type: (str) -> int
        """
        Internal method to completely remove a file given a UDF path.

        Parameters:
         udf_path - The path to the file in the UDF context to remove.
        Returns:
         The number of bytes to remove from the ISO.
        """
        if self.udf_root is None:
            raise pycdlibexception.PyCdlibInvalidInput('Can only specify a UDF path for a UDF ISO')

        udf_path_bytes = utils.normpath(udf_path)
        (udf_file_ident, udf_file_entry) = self._find_udf_record(udf_path_bytes)

        num_bytes_to_remove = 0
        if udf_file_entry is None:
            if udf_file_ident is not None and udf_file_ident.parent is not None:
                # If the udf_path was specified, go looking for the UDF File Ident
                # that corresponds to this record.  If the UDF File Ident exists,
                # and the File Entry is None, this means that it is an "zeroed"
                # UDF File Entry and we have to remove it by hand.
                self._rm_udf_file_ident(udf_file_ident.parent, udf_file_ident.fi)
                # We also have to remove the "zero" UDF File Entry, since nothing
                # else will.
                num_bytes_to_remove += self.logical_block_size
        else:
            if not udf_file_entry.is_file():
                raise pycdlibexception.PyCdlibInvalidInput('Cannot remove a directory with rm_file (try rm_directory instead)')

            # We also want to check to see if this Directory Record is currently
            # being used as an El Torito Boot Catalog, Initial Entry, or Section
            # Entry.  If it is, we throw an exception; we don't know if the user
            # meant to remove El Torito from this ISO, or if they meant to 'hide'
            # the entry, but we need them to call the correct API to let us know.
            if self.eltorito_boot_catalog is not None:
                if any(id(udf_file_entry) == id(rec) for rec in self.eltorito_boot_catalog.dirrecords):
                    raise pycdlibexception.PyCdlibInvalidInput("Cannot remove a file that is referenced by El Torito; use 'rm_eltorito' to remove El Torito, or use 'rm_hard_link' to hide the entry")

            if udf_file_entry.inode is not None:
                self._check_inode_against_eltorito(udf_file_entry.inode)

                while udf_file_entry.inode.linked_records:
                    rec = udf_file_entry.inode.linked_records[0][0]

                    if isinstance(rec, dr.DirectoryRecord):
                        num_bytes_to_remove += self._rm_dr_link(rec)
                    elif isinstance(rec, udfmod.UDFFileEntry):
                        num_bytes_to_remove += self._rm_udf_link(rec)
                    else:
                        # This should never happen.
                        raise pycdlibexception.PyCdlibInternalError('Saw a linked record that was neither ISO or UDF')

        return num_bytes_to_remove

    def _create_dot(self, vd, parent, rock_ridge, xa, file_mode):
        # type: (headervd.PrimaryOrSupplementaryVD, dr.DirectoryRecord, str, bool, int) -> None
        """
        An internal method to create a new 'dot' Directory Record.

        Parameters:
         vd - The volume descriptor to attach the 'dot' Directory Record to.
         parent - The parent Directory Record for new Directory Record.
         rock_ridge - The Rock Ridge version to use for this entry (if any).
         xa - Whether this Directory Record should have extended attributes.
         file_mode - The mode to assign to the dot directory (only applies to Rock Ridge).
        Returns:
         Nothing.
        """
        dot = dr.DirectoryRecord()
        dot.new_dot(vd, parent, vd.sequence_number(), rock_ridge,
                    vd.logical_block_size(), xa, file_mode, time.time())
        self._add_child_to_dr(dot)

    def _create_dotdot(self, vd, parent, rock_ridge, relocated, xa, file_mode):
        # type: (headervd.PrimaryOrSupplementaryVD, dr.DirectoryRecord, str, bool, bool, int) -> dr.DirectoryRecord
        """
        An internal method to create a new 'dotdot' Directory Record.

        Parameters:
         vd - The volume descriptor to attach the 'dotdot' Directory Record to.
         parent - The parent Directory Record for new Directory Record.
         rock_ridge - The Rock Ridge version to use for this entry (if any).
         relocated - Whether this Directory Record is a Rock Ridge relocated entry.
         xa - Whether this Directory Record should have extended attributes.
         file_mode - The mode to assign to the dot directory (only applies to Rock Ridge).
        Returns:
         Nothing.
        """
        dotdot = dr.DirectoryRecord()
        dotdot.new_dotdot(vd, parent, vd.sequence_number(), rock_ridge,
                          vd.logical_block_size(), relocated, xa, file_mode,
                          time.time())
        self._add_child_to_dr(dotdot)
        return dotdot

    ########################### PUBLIC API #####################################

    def __init__(self, always_consistent=False):
        # type: (bool) -> None
        self._always_consistent = always_consistent
        track_writes = os.getenv('PYCDLIB_TRACK_WRITES')
        self._track_writes = False
        if track_writes is not None:
            self._track_writes = True
        self._initialize()

    def new(self, interchange_level=1, sys_ident='', vol_ident='', set_size=1,
            seqnum=1, log_block_size=2048, vol_set_ident=' ', pub_ident_str='',
            preparer_ident_str='', app_ident_str='', copyright_file='',
            abstract_file='', bibli_file='', vol_expire_date=None, app_use='',
            joliet=None, rock_ridge=None, xa=False, udf=None):
        # type: (int, str, str, int, int, int, str, str, str, str, str, str, str, Optional[float], str, Optional[int], Optional[str], bool, Optional[str]) -> None
        """
        Create a new ISO from scratch.

        Parameters:
         interchange_level - The ISO9660 interchange level to use; this dictates
                             the rules on the names of files.  Levels 1, 2, 3,
                             and 4 are supported.  Level 1 is the most
                             conservative, and is the default, but level 3 is
                             recommended.
         sys_ident - The system identification string to use on the new ISO.
         vol_ident - The volume identification string to use on the new ISO.
         set_size - The size of the set of ISOs this ISO is a part of.
         seqnum - The sequence number of the set of this ISO.
         log_block_size - The logical block size to use for the ISO.  While ISO9660
                          technically supports sizes other than 2048 (the default),
                          this almost certainly doesn't work.
         vol_set_ident - The volume set identification string to use on the new ISO.
         pub_ident_str - The publisher identification string to use on the new ISO.
         preparer_ident_str - The preparer identification string to use on the new ISO.
         app_ident_str - The application identification string to use on the new ISO.
         copyright_file - The name of a file at the root of the ISO to use as the
                          copyright file.
         abstract_file - The name of a file at the root of the ISO to use as the
                         abstract file.
         bibli_file - The name of a file at the root of the ISO to use as the
                      bibliographic file.
         vol_expire_date - The date that this ISO will expire at.
         app_use - Arbitrary data that the application can stuff into the primary
                   volume descriptor of this ISO.
         joliet - A integer that can have the value 1, 2, or 3 for Joliet
                  levels 1, 2, or 3 (3 is by far the most common), or None for
                  no Joliet support (the default).  For legacy reasons, this
                  parameter also accepts a boolean, where the value of 'False'
                  means no Joliet and a value of 'True' means level 3.
         rock_ridge - Whether to make this ISO have the Rock Ridge extensions or
                      not.  The default value of None does not add Rock Ridge
                      extensions.  A string value of '1.09', '1.10', or '1.12'
                      adds the specified Rock Ridge version to the ISO.  If
                      unsure, pass '1.09' to ensure maximum compatibility.
         xa - Whether to add the ISO9660 Extended Attribute extensions to this
              ISO.  The default is False.
         udf - Whether to add UDF support to this ISO.  If it is None (the
               default), no UDF support is added.  If it is "2.60", version 2.60
               of the UDF spec is used.  All other values are disallowed.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object already has an ISO; either close it or create a new object')

        if interchange_level < 1 or interchange_level > 4:
            raise pycdlibexception.PyCdlibInvalidInput('Invalid interchange level (must be between 1 and 4)')

        if rock_ridge and rock_ridge not in ('1.09', '1.10', '1.12'):
            raise pycdlibexception.PyCdlibInvalidInput('Rock Ridge value must be None (no Rock Ridge), 1.09, 1.10, or 1.12')

        if udf and udf != '2.60':
            raise pycdlibexception.PyCdlibInvalidInput('UDF value must be empty (no UDF), or 2.60')

        if not app_ident_str:
            app_ident_str = 'PyCdlib (C) 2015-2020 Chris Lalancette'

        self.interchange_level = interchange_level

        self.xa = xa

        if isinstance(joliet, bool):
            if joliet:
                joliet = 3
            else:
                joliet = None

        if rock_ridge:
            self.rock_ridge = rock_ridge

        sys_ident_bytes = sys_ident.encode('utf-8')
        vol_ident_bytes = vol_ident.encode('utf-8')
        vol_set_ident_bytes = vol_set_ident.encode('utf-8')
        pub_ident_bytes = pub_ident_str.encode('utf-8')
        preparer_ident_bytes = preparer_ident_str.encode('utf-8')
        app_ident_bytes = app_ident_str.encode('utf-8')
        copyright_file_bytes = copyright_file.encode('utf-8')
        abstract_file_bytes = abstract_file.encode('utf-8')
        bibli_file_bytes = bibli_file.encode('utf-8')
        app_use_bytes = app_use.encode('utf-8')

        if vol_expire_date is None:
            real_vol_expire_date = 0.0
        else:
            real_vol_expire_date = vol_expire_date

        # Now start creating the ISO.
        self.pvd = headervd.pvd_factory(sys_ident_bytes, vol_ident_bytes,
                                        set_size, seqnum, log_block_size,
                                        vol_set_ident_bytes, pub_ident_bytes,
                                        preparer_ident_bytes, app_ident_bytes,
                                        copyright_file_bytes,
                                        abstract_file_bytes, bibli_file_bytes,
                                        real_vol_expire_date, app_use_bytes, xa)
        self.pvds.append(self.pvd)

        self.logical_block_size = self.pvd.logical_block_size()

        num_bytes_to_add = 0
        if self.interchange_level == 4:
            self.enhanced_vd = headervd.enhanced_vd_factory(sys_ident_bytes,
                                                            vol_ident_bytes,
                                                            set_size, seqnum,
                                                            log_block_size,
                                                            vol_set_ident_bytes,
                                                            pub_ident_bytes,
                                                            preparer_ident_bytes,
                                                            app_ident_bytes,
                                                            copyright_file_bytes,
                                                            abstract_file_bytes,
                                                            bibli_file_bytes,
                                                            real_vol_expire_date,
                                                            app_use_bytes, xa)
            self.svds.append(self.enhanced_vd)

            num_bytes_to_add += self.enhanced_vd.logical_block_size()

        if joliet is not None:
            self.joliet_vd = headervd.joliet_vd_factory(joliet, sys_ident_bytes,
                                                        vol_ident_bytes, set_size,
                                                        seqnum, log_block_size,
                                                        vol_set_ident_bytes,
                                                        pub_ident_bytes,
                                                        preparer_ident_bytes,
                                                        app_ident_bytes,
                                                        copyright_file_bytes,
                                                        abstract_file_bytes,
                                                        bibli_file_bytes,
                                                        real_vol_expire_date,
                                                        app_use_bytes, xa)
            self.svds.append(self.joliet_vd)

            # Now that we have added joliet, we need to add the new space to the
            # PVD for the VD itself.
            num_bytes_to_add += self.joliet_vd.logical_block_size()

        self.vdsts.append(headervd.vdst_factory())
        num_bytes_to_add += self.logical_block_size

        if udf:
            self._has_udf = True
            # Create the UDF Bridge Recognition Volume Sequence.
            udf_bea = udfmod.BEAVolumeStructure()
            udf_bea.new()
            self.udf_beas.append(udf_bea)

            self.udf_nsr.new(2)

            udf_tea = udfmod.TEAVolumeStructure()
            udf_tea.new()
            self.udf_teas.append(udf_tea)

            num_bytes_to_add += 3 * self.logical_block_size

        # We always create an empty version volume descriptor.
        self.version_vd = headervd.version_vd_factory(self.logical_block_size)
        num_bytes_to_add += self.logical_block_size

        if udf:
            # We need to pad out to extent 32.  The padding should be the
            # distance between the current PVD space size and 32.
            additional_extents = 32 - (self.pvd.space_size + (num_bytes_to_add // self.logical_block_size))
            num_bytes_to_add += additional_extents * self.logical_block_size

            # Create the Main Volume Descriptor Sequence.
            pvd = udfmod.UDFPrimaryVolumeDescriptor()
            pvd.new()
            self.udf_main_descs.pvds.append(pvd)

            impl_use = udfmod.UDFImplementationUseVolumeDescriptor()
            impl_use.new()
            self.udf_main_descs.impl_use.append(impl_use)

            partition = udfmod.UDFPartitionVolumeDescriptor()
            partition.new(2)  # FIXME: we should let the user set this
            self.udf_main_descs.partitions.append(partition)

            logical_volume = udfmod.UDFLogicalVolumeDescriptor()
            logical_volume.new()
            logical_volume.add_partition_map(1)
            self.udf_main_descs.logical_volumes.append(logical_volume)

            unallocated_space = udfmod.UDFUnallocatedSpaceDescriptor()
            unallocated_space.new()
            self.udf_main_descs.unallocated_space.append(unallocated_space)

            self.udf_main_descs.terminator.new()

            num_bytes_to_add += 16 * self.logical_block_size

            # Create the Reserve Volume Descriptor Sequence.
            reserve_pvd = udfmod.UDFPrimaryVolumeDescriptor()
            reserve_pvd.new()
            self.udf_reserve_descs.pvds.append(reserve_pvd)

            reserve_impl_use = udfmod.UDFImplementationUseVolumeDescriptor()
            reserve_impl_use.new()
            self.udf_reserve_descs.impl_use.append(reserve_impl_use)

            reserve_partition = udfmod.UDFPartitionVolumeDescriptor()
            reserve_partition.new(2)
            self.udf_reserve_descs.partitions.append(reserve_partition)

            reserve_logical_volume = udfmod.UDFLogicalVolumeDescriptor()
            reserve_logical_volume.new()
            reserve_logical_volume.add_partition_map(1)
            self.udf_reserve_descs.logical_volumes.append(reserve_logical_volume)

            reserve_unallocated_space = udfmod.UDFUnallocatedSpaceDescriptor()
            reserve_unallocated_space.new()
            self.udf_reserve_descs.unallocated_space.append(reserve_unallocated_space)

            self.udf_reserve_descs.terminator.new()

            num_bytes_to_add += 16 * self.logical_block_size

            # Create the Logical Volume Integrity Sequence.
            self.udf_logical_volume_integrity = udfmod.UDFLogicalVolumeIntegrityDescriptor()
            self.udf_logical_volume_integrity.new()

            self.udf_logical_volume_integrity_terminator = udfmod.UDFTerminatingDescriptor()
            self.udf_logical_volume_integrity_terminator.new()

            num_bytes_to_add += 192 * self.logical_block_size

            # Create the Anchor.
            anchor1 = udfmod.UDFAnchorVolumeStructure()
            anchor1.new()
            self.udf_anchors.append(anchor1)

            num_bytes_to_add += self.logical_block_size

            # Create the File Set
            self.udf_file_set.new()

            self.udf_file_set_terminator = udfmod.UDFTerminatingDescriptor()
            self.udf_file_set_terminator.new()

            num_bytes_to_add += 2 * self.logical_block_size

            # Create the root directory, and the 'parent' entry inside.
            self.udf_root = udfmod.UDFFileEntry()
            self.udf_root.new(0, 'dir', None, self.logical_block_size)
            num_bytes_to_add += self.logical_block_size

            parent = udfmod.UDFFileIdentifierDescriptor()
            parent.new(True, True, b'', None)
            num_new_extents = self.udf_root.add_file_ident_desc(parent,
                                                                self.logical_block_size)
            num_bytes_to_add += num_new_extents * self.logical_block_size

        num_partition_bytes_to_add = 0
        # Create the PTR, and add the 4 extents that comprise of the LE PTR and
        # BE PTR to the number of bytes to add.
        ptr = path_table_record.PathTableRecord()
        ptr.new_root()
        self.pvd.root_directory_record().set_ptr(ptr)
        num_partition_bytes_to_add += 4 * self.logical_block_size

        # Also add one extent to the size for the root directory record.
        num_partition_bytes_to_add += self.logical_block_size

        self._create_dot(self.pvd, self.pvd.root_directory_record(),
                         self.rock_ridge, self.xa, 0o040555)
        self._create_dotdot(self.pvd, self.pvd.root_directory_record(),
                            self.rock_ridge, False, self.xa, 0o040555)

        if self.joliet_vd is not None:
            # Create the PTR, and add the 4 extents that comprise of the LE PTR
            # and BE PTR to the number of bytes to add.
            ptr = path_table_record.PathTableRecord()
            ptr.new_root()
            self.joliet_vd.root_directory_record().set_ptr(ptr)
            num_partition_bytes_to_add += 4 * self.logical_block_size

            # Also add one extent to the size for the root directory record.
            num_partition_bytes_to_add += self.logical_block_size

            self._create_dot(self.joliet_vd,
                             self.joliet_vd.root_directory_record(), '',
                             False, -1)
            self._create_dotdot(self.joliet_vd,
                                self.joliet_vd.root_directory_record(), '',
                                False, False, -1)

        if self.rock_ridge:
            num_partition_bytes_to_add += self.logical_block_size

        if udf:
            anchor2 = udfmod.UDFAnchorVolumeStructure()
            anchor2.new()
            self.udf_anchors.append(anchor2)

            num_partition_bytes_to_add += self.logical_block_size

        self._finish_add(num_bytes_to_add, num_partition_bytes_to_add)

        self._initialized = True

    def open(self, filename, mode='rb'):
        # type: (str, str) -> None
        """
        Open up an existing ISO for inspection and modification.

        Parameters:
         filename - The filename containing the ISO to open up.
         mode - The mode to use when opening the ISO file; the default is 'rb'.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object already has an ISO; either close it or create a new object')

        fp = open(filename, mode)  # pylint: disable=consider-using-with,unspecified-encoding
        self._managing_fp = True
        try:
            self._open_fp(fp)
        except Exception:
            fp.close()
            raise

    def open_fp(self, fp):
        # type: (BinaryIO) -> None
        """
        Open up an existing ISO for inspection and modification.  Note that the
        file object passed in here must stay open for the lifetime of this
        object, as the PyCdlib class uses it internally to do writing and reading
        operations.  To have PyCdlib manage this automatically, use 'open'
        instead.

        Parameters:
         fp - The file object containing the ISO to open up.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object already has an ISO; either close it or create a new object')

        self._open_fp(fp)

    def get_file_from_iso(self, local_path, **kwargs):
        # type: (str, Union[str, int]) -> None
        """
        Fetch a single file from the ISO and write it out to a local file.

        Parameters:
         local_path - The local file to write to.
         blocksize - The number of bytes in each transfer.
         iso_path - The absolute ISO9660 path to lookup on the ISO (exclusive
                    with rr_path, joliet_path, and udf_path).
         rr_path - The absolute Rock Ridge path to lookup on the ISO (exclusive
                   with iso_path, joliet_path, and udf_path).
         joliet_path - The absolute Joliet path to lookup on the ISO (exclusive
                       with iso_path, rr_path, and udf_path).
         udf_path - The absolute UDF path to lookup on the ISO (exclusive with
                    iso_path, rr_path, and joliet_path).
         encoding - The encoding to use for parsing the filenames.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        blocksize = 8192
        joliet_path = None
        iso_path = None
        rr_path = None
        udf_path = None
        encoding = ''
        num_paths = 0
        for key, value in kwargs.items():
            if key == 'blocksize':
                if not isinstance(value, int):
                    raise pycdlibexception.PyCdlibInvalidInput('blocksize must be an integer')
                blocksize = value
            elif key == 'iso_path':
                if isinstance(value, str):
                    iso_path = utils.normpath(value)
                    num_paths += 1
                elif value is not None:
                    raise pycdlibexception.PyCdlibInvalidInput('iso_path must be a string')
            elif key == 'rr_path':
                if isinstance(value, str):
                    rr_path = utils.normpath(value)
                    num_paths += 1
                elif value is not None:
                    raise pycdlibexception.PyCdlibInvalidInput('iso_path must be a string')
            elif key == 'joliet_path':
                if isinstance(value, str):
                    joliet_path = utils.normpath(value)
                    num_paths += 1
                elif value is not None:
                    raise pycdlibexception.PyCdlibInvalidInput('iso_path must be a string')
            elif key == 'udf_path':
                if isinstance(value, str):
                    udf_path = utils.normpath(value)
                    num_paths += 1
                elif value is not None:
                    raise pycdlibexception.PyCdlibInvalidInput('iso_path must be a string')
            elif key == 'encoding':
                if not isinstance(value, str):
                    raise pycdlibexception.PyCdlibInvalidInput('encoding must be a string')
                encoding = value
            else:
                raise pycdlibexception.PyCdlibInvalidInput('Unknown keyword %s' % (key))

        if num_paths != 1:
            raise pycdlibexception.PyCdlibInvalidInput("Exactly one of 'iso_path', 'rr_path', 'joliet_path', or 'udf_path' must be passed")

        with open(local_path, 'wb') as fp:
            if udf_path is not None:
                self._udf_get_file_from_iso_fp(fp, blocksize, udf_path)
            else:
                self._get_file_from_iso_fp(fp, blocksize, iso_path, rr_path,
                                           joliet_path, encoding)

    def get_file_from_iso_fp(self, outfp, **kwargs):
        # type: (BinaryIO, Union[str, int]) -> None
        """
        Fetch a single file from the ISO and write it out to the file object.

        Parameters:
         outfp - The file object to write data to.
         blocksize - The number of bytes in each transfer.
         iso_path - The absolute ISO9660 path to lookup on the ISO (exclusive
                    with rr_path, joliet_path, and udf_path).
         rr_path - The absolute Rock Ridge path to lookup on the ISO (exclusive
                   with iso_path, joliet_path, and udf_path).
         joliet_path - The absolute Joliet path to lookup on the ISO (exclusive
                       with iso_path, rr_path, and udf_path).
         udf_path - The absolute UDF path to lookup on the ISO (exclusive with
                    iso_path, rr_path, and joliet_path).
         encoding - The encoding to use for parsing the filenames.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        blocksize = 8192
        joliet_path = None
        iso_path = None
        rr_path = None
        udf_path = None
        encoding = None
        num_paths = 0
        for key, value in kwargs.items():
            if key == 'blocksize':
                if not isinstance(value, int):
                    raise pycdlibexception.PyCdlibInvalidInput('blocksize must be an integer')
                blocksize = value
            elif key == 'iso_path':
                if isinstance(value, str):
                    iso_path = utils.normpath(value)
                    num_paths += 1
                elif value is not None:
                    raise pycdlibexception.PyCdlibInvalidInput('iso_path must be a string')
            elif key == 'rr_path':
                if isinstance(value, str):
                    rr_path = utils.normpath(value)
                    num_paths += 1
                elif value is not None:
                    raise pycdlibexception.PyCdlibInvalidInput('rr_path must be a string')
            elif key == 'joliet_path':
                if isinstance(value, str):
                    joliet_path = utils.normpath(value)
                    num_paths += 1
                elif value is not None:
                    raise pycdlibexception.PyCdlibInvalidInput('joliet_path must be a string')
            elif key == 'udf_path':
                if isinstance(value, str):
                    udf_path = utils.normpath(value)
                    num_paths += 1
                elif value is not None:
                    raise pycdlibexception.PyCdlibInvalidInput('udf_path must be a string')
            elif key == 'encoding':
                if not isinstance(value, str):
                    raise pycdlibexception.PyCdlibInvalidInput('encoding must be a string')
                encoding = value
            else:
                raise pycdlibexception.PyCdlibInvalidInput('Unknown keyword %s' % (key))

        if num_paths != 1:
            raise pycdlibexception.PyCdlibInvalidInput("Exactly one of 'iso_path', 'rr_path', 'joliet_path', or 'udf_path' must be passed")

        if udf_path is not None:
            self._udf_get_file_from_iso_fp(outfp, blocksize, udf_path)
        else:
            self._get_file_from_iso_fp(outfp, blocksize, iso_path, rr_path,
                                       joliet_path, encoding)

    def get_and_write(self, iso_path, local_path, blocksize=8192):
        # type: (str, str, int) -> None
        """
        (deprecated) Fetch a single file from the ISO and write it out to the
        specified file.  Note that this will overwrite the contents of the local
        file if it already exists.  Also note that 'iso_path' must be an
        absolute path to the file.  Finally, the 'iso_path' can be an ISO9660
        path, a Rock Ridge path, or a Joliet path.  In the case of ambiguity,
        the Joliet path is tried first, followed by the ISO9660 path, followed
        by the Rock Ridge path.  It is recommended to use the get_file_from_iso
        API instead to resolve this ambiguity.

        Parameters:
         iso_path - The absolute path to the file to get data from.
         local_path - The local filename to write the contents to.
         blocksize - The blocksize to use when copying data; the default is 8192.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        with open(local_path, 'wb') as fp:
            self._get_and_write_fp(utils.normpath(iso_path), fp, blocksize)

    def get_and_write_fp(self, iso_path, outfp, blocksize=8192):
        # type: (str, BinaryIO, int) -> None
        """
        (deprecated) Fetch a single file from the ISO and write it out to the
        file object.  Note that 'iso_path' must be an absolute path to the file.
        Also note that the 'iso_path' can be an ISO9660 path, a Rock Ridge path,
        or a Joliet path.  In the case of ambiguity, the Joliet path is tried
        first, followed by the ISO9660 path, followed by the Rock Ridge path.
        It is recommend to use the get_file_from_iso_fp API instead to resolve
        this ambiguity.

        Parameters:
         iso_path - The absolute path to the file to get data from.
         outfp - The file object to write data to.
         blocksize - The blocksize to use when copying data; the default is 8192.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        self._get_and_write_fp(utils.normpath(iso_path), outfp, blocksize)

    def write(self, filename, blocksize=32768, progress_cb=None,
              progress_opaque=None):
        # type: (str, int, Optional[Callable[[int, int, Any], None]], Optional[Any]) -> None
        """
        Write a properly formatted ISO out to the filename passed in.  This
        also goes by the name of 'mastering'.

        Parameters:
         filename - The filename to write the data to.
         blocksize - The blocksize to use when copying data; the default is 32768.
         progress_cb - If not None, a function to call as the write call does its
                       work.  The callback function must have a signature of:
                       def func(done, total, opaque).
         progress_opaque - User data to be passed to the progress callback; the
                           default is None.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        with open(filename, 'wb') as fp:
            self._write_fp(fp, blocksize, progress_cb, progress_opaque)

    def write_fp(self, outfp, blocksize=32768, progress_cb=None,
                 progress_opaque=None):
        # type: (BinaryIO, int, Optional[Callable[[int, int, Any], None]], Optional[Any]) -> None
        """
        Write a properly formatted ISO out to the file object passed in.  This
        also goes by the name of 'mastering'.

        Parameters:
         outfp - The file object to write the data to.
         blocksize - The blocksize to use when copying data; the default is 32768.
         progress_cb - If not None, a function to call as the write call does its
                       work.  The callback function must have a signature of:
                       def func(done, total, opaque).
         progress_opaque - User data to be passed to the progress callback; the
                           default is None.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        self._write_fp(outfp, blocksize, progress_cb, progress_opaque)

    def add_fp(self, fp, length, iso_path=None, rr_name=None, joliet_path=None,
               file_mode=None, udf_path=None):
        # type: (BinaryIO, int, Optional[str], Optional[str], Optional[str], Optional[int], Optional[str]) -> None
        """
        Add a file to the ISO.  If the ISO is a Rock Ridge one, then a Rock
        Ridge name must also be provided.  If the ISO is a Joliet one, then a
        Joliet path may also be provided; while it is optional to do so, it is
        highly recommended.  Note that the caller must ensure that 'fp' remains
        open for the lifetime of the PyCdlib object, as the PyCdlib class uses
        the file descriptor internally when writing (mastering) the ISO.  To
        have PyCdlib manage this automatically, use 'add_file' instead.

        Parameters:
         fp - The file object to use for the contents of the new file.
         length - The length of the data for the new file.
         iso_path - The ISO9660 absolute path to the file destination on the ISO.
         rr_name - The Rock Ridge name of the file destination on the ISO.
         joliet_path - The Joliet absolute path to the file destination on the ISO.
         file_mode - The POSIX file_mode to apply to this file.  This only
                     applies if this is a Rock Ridge ISO.  If this is None (the
                     default), the permissions from the original file are used.
         udf_path - The UDF name of the file destination on the ISO.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        if not utils.file_object_supports_binary(fp):
            raise pycdlibexception.PyCdlibInvalidInput('The fp argument must be in binary mode')

        num_bytes_to_add = self._add_fp(fp, length, False, iso_path, rr_name,
                                        joliet_path, udf_path, file_mode, False)

        self._finish_add(0, num_bytes_to_add)

    def add_file(self, filename, iso_path=None, rr_name=None, joliet_path=None,
                 file_mode=None, udf_path=None):
        # type: (str, Optional[str], Optional[str], Optional[str], Optional[int], Optional[str]) -> None
        """
        Add a file to the ISO.  If the ISO is a Rock Ridge one, then a Rock
        Ridge name must also be provided.  If the ISO is a Joliet one, then a
        Joliet path may also be provided; while it is optional to do so, it is
        highly recommended.

        Parameters:
         filename - The filename to use for the data contents for the new file.
         iso_path - The ISO9660 absolute path to the file destination on the ISO.
         rr_name - The Rock Ridge name of the file destination on the ISO.
         joliet_path - The Joliet absolute path to the file destination on the ISO.
         file_mode - The POSIX file_mode to apply to this file.  This only
                     applies if this is a Rock Ridge ISO.  If this is None (the
                     default), the permissions from the original file are used.
         udf_path - The UDF name of the file destination on the ISO.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        num_bytes_to_add = self._add_fp(filename, os.stat(filename).st_size,
                                        True, iso_path, rr_name, joliet_path,
                                        udf_path, file_mode, False)

        self._finish_add(0, num_bytes_to_add)

    def modify_file_in_place(self, fp, length, iso_path, rr_name=None,  # pylint: disable=unused-argument
                             joliet_path=None, udf_path=None):          # pylint: disable=unused-argument
        # type: (BinaryIO, int, str, Optional[str], Optional[str], Optional[str]) -> None
        """
        An API to modify a file in place on the ISO.  This can be extremely fast
        (much faster than calling the write method), but has many restrictions.

        1.  The original ISO file pointer must have been opened for reading
            and writing.
        2.  Only an existing *file* can be modified; directories cannot be
            changed.
        3.  Only an existing file can be *modified*; no new files can be added
            or removed.
        4.  The new file contents must use the same number of extents (typically
            2048 bytes) as the old file contents.  If using this API to shrink
            a file, this is usually easy since the new contents can be padded
            out with zeros or newlines to meet the requirement.  If using this
            API to grow a file, the new contents can only grow up to the next
            extent boundary.

        Unlike all other APIs in PyCdlib, this API actually modifies the
        originally opened on-disk file, so use it with caution.

        Parameters:
         fp - The file object to use for the contents of the new file.
         length - The length of the new data for the file.
         iso_path - The ISO9660 absolute path to the file destination on the ISO.
         rr_name - The Rock Ridge name of the file destination on the ISO.
         joliet_path - The Joliet absolute path to the file destination on the ISO.
         udf_path - The UDF absolute path to the file destination on the ISO.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        if hasattr(self._cdfp, 'mode') and not self._cdfp.mode.startswith(('r+', 'w', 'a', 'rb+')):
            raise pycdlibexception.PyCdlibInvalidInput('To modify a file in place, the original ISO must have been opened in a write mode (r+, w, or a)')

        child = self._find_iso_record(utils.normpath(iso_path))

        old_num_extents = utils.ceiling_div(child.get_data_length(),
                                            self.logical_block_size)
        new_num_extents = utils.ceiling_div(length, self.logical_block_size)

        if old_num_extents != new_num_extents:
            raise pycdlibexception.PyCdlibInvalidInput('When modifying a file in-place, the number of extents for a file cannot change!')

        if not child.is_file():
            raise pycdlibexception.PyCdlibInvalidInput('Cannot modify a directory with modify_file_in_place')

        if child.inode is None:
            raise pycdlibexception.PyCdlibInternalError('Child file found without inode')

        child.inode.update_fp(fp, length)

        # Remove the old size from the PVD size.
        for pvd in self.pvds:
            pvd.remove_from_space_size(child.get_data_length())
        # And add the new size to the PVD size.
        for pvd in self.pvds:
            pvd.add_to_space_size(length)

        if self.enhanced_vd is not None:
            self.enhanced_vd.copy_sizes(self.pvd)

        # If we made it here, we have successfully updated all of the in-memory
        # metadata.  Now we can go and modify the on-disk file.

        self._seek_to_extent(self.pvd.extent_location())

        # First write out the PVD.
        rec = self.pvd.record()
        self._cdfp.write(rec)

        # Write out the joliet VD.
        if self.joliet_vd is not None:
            self._seek_to_extent(self.joliet_vd.extent_location())
            rec = self.joliet_vd.record()
            self._cdfp.write(rec)

        # Write out the enhanced VD.
        if self.enhanced_vd is not None:
            self._seek_to_extent(self.enhanced_vd.extent_location())
            rec = self.enhanced_vd.record()
            self._cdfp.write(rec)

        # We don't have to write anything out for UDF since it only tracks
        # extents, and we know we aren't changing the number of extents.

        # Write out the actual file contents.
        self._seek_to_extent(child.extent_location())
        with inode.InodeOpenData(child.inode, self.logical_block_size) as (data_fp, data_len):
            utils.copy_data(data_len, self.logical_block_size, data_fp, self._cdfp)
            utils.zero_pad(self._cdfp, data_len, self.logical_block_size)

        # Finally write out the directory record entry.
        # This is a little tricky because of what things mean.  First of all,
        # child.extents_to_here represents the total number of extents up to
        # this child in the parent.  Thus, to get the absolute extent offset,
        # we start with the parent's extent location, add on the number of
        # extents to here, and remove 1 (since our offset will be zero-based).
        # Second, child.offset_to_here is the *last* byte that the child uses,
        # so to get the start of it we subtract off the length of the child.
        # Then we can multiply the extent location by the logical block size,
        # add on the offset, and get to the absolute location in the file.
        first_joliet = True
        for record, is_pvd_unused in child.inode.linked_records:
            if isinstance(record, dr.DirectoryRecord):
                if self.joliet_vd is not None and id(record.vd) == id(self.joliet_vd) and first_joliet:
                    first_joliet = False
                    self.joliet_vd.remove_from_space_size(record.get_data_length())
                    self.joliet_vd.add_to_space_size(length)
                if record.parent is None:
                    raise pycdlibexception.PyCdlibInternalError('Modifying file with empty parent')
                abs_extent_loc = record.parent.extent_location() + record.extents_to_here - 1
                offset = record.offset_to_here - record.dr_len
                abs_offset = abs_extent_loc * self.logical_block_size + offset
            elif isinstance(record, udfmod.UDFFileEntry):
                abs_offset = record.extent_location() * self.logical_block_size
            else:
                # This should never happen
                raise pycdlibexception.PyCdlibInternalError('Invalid record type')

            record.set_data_length(length)
            self._cdfp.seek(abs_offset)
            self._cdfp.write(record.record())

    def add_hard_link(self, **kwargs):
        # type: (str) -> None
        """
        Add a hard link to the ISO.  Hard links are alternate names for the
        same file contents that don't take up any additional space on the the
        ISO.  This API can be used to create hard links between two files on
        the ISO9660 filesystem, between two files on the Joliet filesystem, or
        between a file on the ISO9660 filesystem and the Joliet filesystem.
        In all cases, exactly one old path must be specified, and exactly one
        new path must be specified.
        Note that this is an advanced API, so using it in combination with the
        higher-level APIs (like rm_file()) may result in unexpected behavior.
        Once this API has been used, this API and rm_hard_link() should be
        preferred over add_file() and rm_file(), respectively.

        Parameters:
         iso_old_path - The old path on the ISO9660 filesystem to link from.
         iso_new_path - The new path on the ISO9660 filesystem to link to.
         joliet_old_path - The old path on the Joliet filesystem to link from.
         joliet_new_path - The new path on the Joliet filesystem to link to.
         rr_name - The Rock Ridge name to use for the new file if this is a Rock
                   Ridge ISO and the new path is on the ISO9660 filesystem.
         boot_catalog_old - Use the El Torito boot catalog as the old path.
         udf_old_path - The old path on the UDF filesystem to link from.
         udf_new_path - The new path on the UDF filesystem to link to.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        num_old = 0
        iso_old_path = None
        joliet_old_path = None
        boot_catalog_old = False
        udf_old_path = None
        keys_to_remove = []
        for key, value in kwargs.items():
            if key == 'iso_old_path':
                if value is not None:
                    num_old += 1
                    iso_old_path = utils.normpath(value)
                keys_to_remove.append(key)
            elif key == 'joliet_old_path':
                if value is not None:
                    num_old += 1
                    joliet_old_path = self._normalize_joliet_path(value)
                keys_to_remove.append(key)
            elif key == 'boot_catalog_old':
                if value:
                    num_old += 1
                    boot_catalog_old = True
                    if self.eltorito_boot_catalog is None:
                        raise pycdlibexception.PyCdlibInvalidInput('Attempting to make link to non-existent El Torito boot catalog')
                keys_to_remove.append(key)
            elif key == 'udf_old_path':
                if value is not None:
                    num_old += 1
                    udf_old_path = utils.normpath(value)
                keys_to_remove.append(key)

        if num_old != 1:
            raise pycdlibexception.PyCdlibInvalidInput('Exactly one old path must be specified')

        # Once we've iterated over the keys we know about, remove them from
        # the map so that _add_hard_link_to_inode() can parse the rest.
        for key in keys_to_remove:
            del kwargs[key]

        # It would be nice to allow the addition of a link to the El Torito
        # Initial/Default Entry.  Unfortunately, the information we need for
        # a 'hidden' Initial entry just doesn't exist on the ISO.  In
        # particular, we don't know the real size that the file should be, we
        # only know the number of emulated sectors (512 bytes) that it will be
        # loaded into.  Since the true length and the number of sectors are not
        # the same thing, we can't actually add a hard link.

        old_rec = dr.DirectoryRecord()  # type: Union[dr.DirectoryRecord, udfmod.UDFFileEntry]
        fmode = 0
        if iso_old_path is not None:
            # A link from a file on the ISO9660 filesystem...
            old_rec = self._find_iso_record(iso_old_path)
            if old_rec.rock_ridge is not None:
                fmode = old_rec.rock_ridge.get_file_mode()
        elif joliet_old_path is not None:
            # A link from a file on the Joliet filesystem...
            old_rec = self._find_joliet_record(joliet_old_path)
        elif boot_catalog_old:
            # A link from the El Torito boot catalog...
            if self.eltorito_boot_catalog is None:
                raise pycdlibexception.PyCdlibInvalidInput('Attempting to make link to non-existent El Torito boot catalog')
            old_rec = self.eltorito_boot_catalog.dirrecords[0]
        elif udf_old_path is not None:
            # A link from a file on the UDF filesystem...
            (old_ident_unused, old_rec) = self._find_udf_record(udf_old_path)
            if old_rec is None:
                raise pycdlibexception.PyCdlibInvalidInput('Cannot make hard link to a UDF file with an empty UDF File Entry')

        # Above we checked to make sure we got at least one old path, so we
        # don't need to worry about the else situation here.

        num_bytes_to_add = self._add_hard_link_to_inode(old_rec.inode,
                                                        old_rec.get_data_length(),
                                                        fmode, boot_catalog_old,
                                                        **kwargs)

        self._finish_add(0, num_bytes_to_add)

    def rm_hard_link(self, iso_path=None, joliet_path=None, udf_path=None):
        # type: (Optional[str], Optional[str], Optional[str]) -> None
        """
        Remove a hard link from the ISO.  If the number of links to a piece of
        data drops to zero, then the contents will be removed from the ISO.
        This can be thought of as a lower-level interface to rm_file().  Either
        an ISO9660 path or a Joliet path must be passed to this API, but not
        both.  Thus, this interface can be used to hide files from either the
        ISO9660 filesystem, the Joliet filesystem, or both (if there is another
        reference to the data on the ISO, such as in El Torito).  Note that this
        is an advanced API, so using it in combination with the higher-level APIs
        (like rm_file()) may result in unexpected behavior.  Once this API has
        been used, this API and add_hard_link() should be preferred over
        rm_file() and add_file().

        Parameters:
         iso_path - The ISO link path to remove.
         joliet_path - The Joliet link path to remove.
         udf_path - The UDF link path to remove.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        if len([x for x in (iso_path, joliet_path, udf_path) if x]) != 1:
            raise pycdlibexception.PyCdlibInvalidInput('Must provide exactly one of iso_path, joliet_path, or udf_path')

        num_bytes_to_remove = 0
        rec = None  # type: Optional[Union[dr.DirectoryRecord, udfmod.UDFFileEntry]]

        if iso_path is not None:
            rec = self._find_iso_record(utils.normpath(iso_path))
            num_bytes_to_remove += self._rm_dr_link(rec)
        elif joliet_path is not None:
            if self.joliet_vd is None:
                raise pycdlibexception.PyCdlibInvalidInput('Cannot remove Joliet link from non-Joliet ISO')
            joliet_path_bytes = self._normalize_joliet_path(joliet_path)
            rec = self._find_joliet_record(joliet_path_bytes)
            num_bytes_to_remove += self._rm_dr_link(rec)
        elif udf_path is not None:
            # UDF hard link removal
            if self.udf_root is None:
                raise pycdlibexception.PyCdlibInvalidInput('Can only specify a UDF path for a UDF ISO')

            (ident, rec) = self._find_udf_record(utils.normpath(udf_path))
            if rec is None:
                # If the rec is None, that means that this pointed to an 'empty'
                # UDF File Entry.  Just remove the UDF File Identifier, which is
                # as much as we can do.
                if ident is not None and ident.parent is not None:
                    num_bytes_to_remove += self._rm_udf_file_ident(ident.parent, ident.fi)
                # We also have to remove the "zero" UDF File Entry, since nothing
                # else will.
                num_bytes_to_remove += self.logical_block_size
            else:
                num_bytes_to_remove += self._rm_udf_link(rec)
        else:
            raise pycdlibexception.PyCdlibInvalidInput("One of 'iso_path', 'joliet_path', or 'udf_path' must be specified")

        self._finish_remove(num_bytes_to_remove, True)

    def add_directory(self, iso_path=None, rr_name=None, joliet_path=None,
                      file_mode=None, udf_path=None):
        # type: (Optional[str], Optional[str], Optional[str], Optional[int], Optional[str]) -> None
        """
        Add a directory to the ISO.  At least one of an iso_path, joliet_path,
        or udf_path must be provided.  Providing joliet_path on a non-Joliet
        ISO, or udf_path on a non-UDF ISO, is an error.  If the ISO contains
        Rock Ridge, then a Rock Ridge name must be provided.

        Parameters:
         iso_path - The ISO9660 absolute path to use for the directory.
         rr_name - The Rock Ridge name to use for the directory.
         joliet_path - The Joliet absolute path to use for the directory.
         file_mode - The POSIX file mode to use for the directory.  This only
                     applies for Rock Ridge ISOs.
         udf_path - The UDF absolute path to use for the directory.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        if iso_path is None and joliet_path is None and udf_path is None:
            raise pycdlibexception.PyCdlibInvalidInput('Either iso_path or joliet_path must be passed')

        if file_mode is not None and not self.rock_ridge:
            raise pycdlibexception.PyCdlibInvalidInput('A file mode can only be specified for Rock Ridge ISOs')

        # For backwards-compatibility reasons, if the mode was not specified we
        # just assume 555.  We should probably eventually make file_mode
        # required for Rock Ridge and remove this assumption.
        if file_mode is None:
            file_mode = 0o040555

        num_bytes_to_add = 0
        if iso_path is not None:
            iso_path_bytes = utils.normpath(iso_path)

            new_rr_name = self._check_rr_name(rr_name)

            depth = len(utils.split_path(iso_path_bytes))

            if not self.rock_ridge and self.enhanced_vd is None:
                _check_path_depth(iso_path_bytes)
            (name, parent) = self._iso_name_and_parent_from_path(iso_path_bytes)

            _check_iso9660_directory(name, self.interchange_level)

            relocated = False
            fake_dir_rec = None
            orig_parent = None
            iso9660_name = name
            if self.rock_ridge and (depth % 8) == 0 and self.enhanced_vd is None:
                # If the depth was a multiple of 8, then we are going to have to
                # make a relocated entry for this record.

                num_bytes_to_add += self._find_or_create_rr_moved()

                # With a depth of 8, we have to add the directory both to the
                # original parent with a CL link, and to the new parent with an
                # RE link.  Here we make the 'fake' record, as a child of the
                # original place; the real one will be done below.
                fake_dir_rec = dr.DirectoryRecord()
                fake_dir_rec.new_dir(self.pvd, name, parent,
                                     self.pvd.sequence_number(),
                                     self.rock_ridge, new_rr_name,
                                     self.logical_block_size, True, False,
                                     self.xa, file_mode, time.time())
                num_bytes_to_add += self._add_child_to_dr(fake_dir_rec)

                # The fake dir record doesn't get an entry in the path table
                # record.

                relocated = True
                orig_parent = parent
                parent = self._rr_moved_record

                # Since we are moving the entry underneath the RR_MOVED
                # directory, there is now the chance of a name collision (this
                # can't happen without relocation since _add_child_to_dr() below
                # won't allow duplicate names).  Check for that here and
                # generate a new name.
                index = 0
                while True:
                    for child in self._rr_moved_record.children:
                        if child.file_ident == iso9660_name:
                            # Python 3.4 doesn't support substitution with a byte
                            # array, so we do it as a string and encode to bytes.
                            iso9660_name = name + ('%03d' % (index)).encode()
                            index += 1
                            break
                    else:
                        break

            rec = dr.DirectoryRecord()
            rec.new_dir(self.pvd, iso9660_name, parent,
                        self.pvd.sequence_number(), self.rock_ridge, new_rr_name,
                        self.logical_block_size, False, relocated,
                        self.xa, file_mode, time.time())
            num_bytes_to_add += self._add_child_to_dr(rec)
            if rec.rock_ridge is not None:
                if relocated and fake_dir_rec is not None and fake_dir_rec.rock_ridge is not None:
                    fake_dir_rec.rock_ridge.cl_to_moved_dr = rec
                    rec.rock_ridge.moved_to_cl_dr = fake_dir_rec
                num_bytes_to_add += self._update_rr_ce_entry(rec)

            self._create_dot(self.pvd, rec, self.rock_ridge, self.xa, file_mode)

            parent_file_mode = -1
            if parent.rock_ridge is not None:
                parent_file_mode = parent.rock_ridge.get_file_mode()
            else:
                if parent.is_root:
                    parent_file_mode = file_mode

            dotdot = self._create_dotdot(self.pvd, rec, self.rock_ridge,
                                         relocated, self.xa, parent_file_mode)
            if dotdot.rock_ridge is not None and relocated:
                dotdot.rock_ridge.parent_link = orig_parent

            # We always need to add an entry to the path table record.
            ptr = path_table_record.PathTableRecord()
            ptr.new_dir(iso9660_name)

            num_bytes_to_add += self._add_to_ptr_size(ptr) + self.logical_block_size

            rec.set_ptr(ptr)

        if joliet_path is not None:
            num_bytes_to_add += self._add_joliet_dir(self._normalize_joliet_path(joliet_path))

        if udf_path is not None:
            if self.udf_root is None:
                raise pycdlibexception.PyCdlibInvalidInput('Can only specify a UDF path for a UDF ISO')

            udf_path_bytes = utils.normpath(udf_path)
            (udf_name, udf_parent) = self._udf_name_and_parent_from_path(udf_path_bytes)

            file_ident = udfmod.UDFFileIdentifierDescriptor()
            file_ident.new(True, False, udf_name, udf_parent)
            num_new_extents = udf_parent.add_file_ident_desc(file_ident, self.logical_block_size)
            num_bytes_to_add += num_new_extents * self.logical_block_size

            file_entry = udfmod.UDFFileEntry()
            file_entry.new(0, 'dir', udf_parent, self.logical_block_size)
            file_ident.file_entry = file_entry
            file_entry.file_ident = file_ident
            num_bytes_to_add += self.logical_block_size

            udf_dotdot = udfmod.UDFFileIdentifierDescriptor()
            udf_dotdot.new(True, True, b'', udf_parent)
            num_new_extents = file_ident.file_entry.add_file_ident_desc(udf_dotdot, self.logical_block_size)
            num_bytes_to_add += num_new_extents * self.logical_block_size

            if self.udf_logical_volume_integrity is not None:
                self.udf_logical_volume_integrity.logical_volume_impl_use.num_dirs += 1

        self._finish_add(0, num_bytes_to_add)

    def add_joliet_directory(self, joliet_path):
        # type: (str) -> None
        """
        (deprecated) Add a directory to the Joliet portion of the ISO.  Since
        Joliet occupies a completely different context than ISO9660, this
        method can be invoked to create a completely different directory
        structure in the Joliet context, though that is generally not advised.
        It is recommended to use the 'joliet_path' argument of the
        'add_directory' instead of this method.

        Parameters:
         joliet_path - The Joliet directory to create.
        Returns:
         Nothing.
        """
        self.add_directory(joliet_path=joliet_path)

    def rm_file(self, iso_path=None, rr_name=None, joliet_path=None,  # pylint: disable=unused-argument
                udf_path=None):
        # type: (Optional[str], Optional[str], Optional[str], Optional[str]) -> None
        """
        Remove a file from the ISO.  This removes the data and the listing of
        the file from all contexts, even when only one path is given (to only
        remove it from a single context, use rm_hard_link() instead).  Due to
        some complexities of the ISO format, removal of zero-byte files from all
        contexts does not automatically happen, so this method may need to be
        called more than once for zero-byte files.

        Parameters:
         iso_path - The path to the file to remove.
         rr_name - The Rock Ridge name of the file to remove.
         joliet_path - The Joliet path to the file to remove.
         udf_path - The UDF path to the file to remove.
        Returns:
         Nothing.
        """

        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        num_bytes_to_remove = 0
        if iso_path is not None:
            num_bytes_to_remove += self._rm_file_via_iso_path(iso_path)
        elif joliet_path is not None:
            num_bytes_to_remove += self._rm_file_via_joliet_path(joliet_path)
        elif udf_path is not None:
            num_bytes_to_remove += self._rm_file_via_udf_path(udf_path)
        else:
            raise pycdlibexception.PyCdlibInternalError("At least one of 'iso_path', 'joliet_path', or 'udf_path' must be specified")

        self._finish_remove(num_bytes_to_remove, True)

    def rm_directory(self, iso_path=None, rr_name=None, joliet_path=None,  # pylint: disable=unused-argument
                     udf_path=None):
        # type: (Optional[str], Optional[str], Optional[str], Optional[str]) -> None
        """
        Remove a directory from the ISO.  The directory must be empty.

        Parameters:
         iso_path - The path to the directory to remove.
         rr_name - The Rock Ridge name of the directory to remove.
         joliet_path - The Joliet path to the directory to remove.
         udf_path - The UDF absolute path to the directory to remove.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        if iso_path is None and joliet_path is None and udf_path is None:
            raise pycdlibexception.PyCdlibInvalidInput('Either iso_path or joliet_path must be passed')

        num_bytes_to_remove = 0

        if iso_path is not None:
            iso_path_bytes = utils.normpath(iso_path)

            if iso_path_bytes == b'/':
                raise pycdlibexception.PyCdlibInvalidInput('Cannot remove base directory')

            child = self._find_iso_record(iso_path_bytes)

            if not child.is_dir():
                raise pycdlibexception.PyCdlibInvalidInput('Cannot remove a file with rm_directory (try rm_file instead)')

            if len(child.children) > 2:
                raise pycdlibexception.PyCdlibInvalidInput('Directory must be empty to use rm_directory')

            num_bytes_to_remove += self._remove_child_from_dr(child,
                                                              child.index_in_parent)

            if child.ptr is not None:
                num_bytes_to_remove += self._remove_from_ptr_size(child.ptr)

            # Remove space for the directory itself.
            num_bytes_to_remove += child.get_data_length()

            if child.rock_ridge is not None and child.rock_ridge.relocated_record():
                # OK, this child was relocated.  If the parent of this relocated
                # record is empty (only . and ..), we can remove it.
                parent = child.parent
                if parent is None:
                    raise pycdlibexception.PyCdlibInternalError('Relocated child has empty parent; this should not be')
                if len(parent.children) == 2:
                    if parent.parent is None:
                        raise pycdlibexception.PyCdlibInternalError('Tried to remove a directory that has no parent; this should not happen')
                    for index, c in enumerate(parent.parent.children):
                        if c.file_ident == parent.file_ident:
                            parent_index = index
                            break
                    else:
                        raise pycdlibexception.PyCdlibInvalidISO('Could not find parent in its own parent!')

                    num_bytes_to_remove += self._remove_child_from_dr(parent,
                                                                      parent_index)

                    num_bytes_to_remove += parent.get_data_length()
                    if parent.ptr is not None:
                        num_bytes_to_remove += self._remove_from_ptr_size(parent.ptr)

                cl = child.rock_ridge.moved_to_cl_dr
                if cl is None:
                    raise pycdlibexception.PyCdlibInternalError('Invalid child link record')
                if cl.parent is None:
                    raise pycdlibexception.PyCdlibInternalError('Invalid parent to child link record; this should not be')
                for index, c in enumerate(cl.parent.children):
                    if cl.file_ident == c.file_ident:
                        clindex = index
                        break
                else:
                    raise pycdlibexception.PyCdlibInvalidISO('CL record does not exist')

                if cl.children:
                    raise pycdlibexception.PyCdlibInvalidISO('Parent link should have no children!')
                num_bytes_to_remove += self._remove_child_from_dr(cl, clindex)

                # We do not remove additional space from the PVD for the
                # child_link record because it is a 'fake' record that has no
                # size.

            if child.rock_ridge is not None and child.rock_ridge.dr_entries.ce_record is not None and child.rock_ridge.ce_block is not None:
                child.rock_ridge.ce_block.remove_entry(child.rock_ridge.dr_entries.ce_record.offset_cont_area,
                                                       child.rock_ridge.dr_entries.ce_record.len_cont_area)

        if joliet_path is not None:
            num_bytes_to_remove += self._rm_joliet_dir(self._normalize_joliet_path(joliet_path))

        if udf_path is not None:
            if self.udf_root is None:
                raise pycdlibexception.PyCdlibInvalidInput('Can only specify a UDF path for a UDF ISO')

            udf_path_bytes = utils.normpath(udf_path)

            if udf_path_bytes == b'/':
                raise pycdlibexception.PyCdlibInvalidInput('Cannot remove base directory')

            (udf_name, udf_parent) = self._udf_name_and_parent_from_path(udf_path_bytes)

            num_extents_to_remove = udf_parent.remove_file_ident_desc_by_name(udf_name,
                                                                              self.logical_block_size)
            # Remove space (if necessary) in the parent File Identifier
            # Descriptor area.
            num_bytes_to_remove += num_extents_to_remove * self.logical_block_size
            # Remove space for the File Entry.
            num_bytes_to_remove += self.logical_block_size
            # Remove space for the list of File Identifier Descriptors.
            num_bytes_to_remove += self.logical_block_size

            if self.udf_logical_volume_integrity is not None:
                self.udf_logical_volume_integrity.logical_volume_impl_use.num_dirs -= 1

            self._find_udf_record.cache_clear()  # pylint: disable=no-member

        self._finish_remove(num_bytes_to_remove, True)

    def rm_joliet_directory(self, joliet_path):
        # type: (str) -> None
        """
        (deprecated) Remove a Joliet directory from the ISO.  It is recommended
        to use the 'joliet_path' parameter to 'rm_directory' instead.

        Parameters:
         joliet_path - The Joliet path to the directory to remove.
        Returns:
         Nothing.
        """
        self.rm_directory(joliet_path=joliet_path)

    def add_eltorito(self, bootfile_path, bootcatfile=None,
                     rr_bootcatname=None, joliet_bootcatfile=None,
                     boot_load_size=None, platform_id=0, boot_info_table=False,
                     efi=False, media_name='noemul', bootable=True,
                     boot_load_seg=0, udf_bootcatfile=None):
        # type: (str, Optional[str], Optional[str], Optional[str], Optional[int], int, bool, bool, str, bool, int, Optional[str]) -> None
        """
        Add an El Torito Boot Record, and associated files, to the ISO.  The
        file that will be used as the bootfile must be passed into this function
        and must already be present on the ISO.

        Parameters:
         bootfile_path - The file to use as the boot file; it must already
                         exist on this ISO.
         bootcatfile - The fake file to use as the boot catalog entry; set to
                       BOOT.CAT;1 by default.
         rr_bootcatname - The Rock Ridge name for the fake file to use as the
                          boot catalog entry; set to 'boot.cat' by default.
         joliet_bootcatfile - The Joliet name for the fake file to use as the
                              boot catalog entry; set to 'boot.cat' by default.
         boot_load_size - The number of sectors to use for the boot entry; if
                          set to None (the default), the number of sectors will
                          be calculated.
         platform_id - The platform ID to set for the El Torito entry; 0 is for
                       x86, 1 is for Power PC, 2 is for Mac, and 0xef is for
                       UEFI.  0 is the default.
         boot_info_table - Whether to add a boot info table to the ISO.  The
                           default is False.
         efi - Whether this is an EFI entry for El Torito.  The default is False.
         media_name - The name of the media type, one of 'noemul', 'floppy', or 'hdemul'.
         bootable - Whether the boot media is bootable.  The default is True.
         boot_load_seg - The load segment address of the boot image.
         udf_bootcatfile - The name of the boot.cat file on the UDF filesystem.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        # In order to add an El Torito boot, we need to do the following:
        # 1.  Find the boot file record (which must already exist).
        # 2.  Construct a BootRecord.
        # 3.  Construct a BootCatalog, and add it to the filesystem.
        # 4.  Add the boot record to the ISO.

        if not bootcatfile:
            bootcatfile = '/BOOT.CAT;1'

        bootfile_path_bytes = utils.normpath(bootfile_path)

        if self.joliet_vd is not None:
            if not joliet_bootcatfile:
                joliet_bootcatfile = '/boot.cat'
        else:
            if joliet_bootcatfile:
                raise pycdlibexception.PyCdlibInvalidInput('A joliet path must not be passed when adding El Torito to a non-Joliet ISO')

        if self.udf_root is not None:
            if not udf_bootcatfile:
                udf_bootcatfile = '/boot.cat'
        else:
            if udf_bootcatfile:
                raise pycdlibexception.PyCdlibInvalidInput('A UDF path must not be passed when adding El Torito to a non-UDF ISO')

        # Step 1.
        boot_dirrecord = self._find_iso_record(bootfile_path_bytes)

        if boot_load_size is None:
            sector_count = utils.ceiling_div(boot_dirrecord.get_data_length(),
                                             self.logical_block_size) * self.logical_block_size // 512
        else:
            sector_count = boot_load_size

        if boot_dirrecord.inode is None:
            raise pycdlibexception.PyCdlibInternalError('Tried to add an empty boot dirrecord inode to the El Torito boot catalog')

        if boot_info_table:
            orig_len = boot_dirrecord.get_data_length()
            bi_table = eltorito.EltoritoBootInfoTable()
            with inode.InodeOpenData(boot_dirrecord.inode, self.logical_block_size) as (data_fp, data_len):
                bi_table.new(self.pvd, boot_dirrecord.inode, orig_len,
                             self._calculate_eltorito_boot_info_table_csum(data_fp, data_len))

            boot_dirrecord.inode.add_boot_info_table(bi_table)

        system_type = 0
        if media_name == 'hdemul':
            with inode.InodeOpenData(boot_dirrecord.inode, self.logical_block_size) as (data_fp, data_len):
                disk_mbr = data_fp.read(512)
                if len(disk_mbr) != 512:
                    raise pycdlibexception.PyCdlibInvalidInput('Could not read entire HD MBR, must be at least 512 bytes')
                system_type = eltorito.hdmbrcheck(disk_mbr, sector_count,
                                                  bootable)

        num_bytes_to_add = 0
        if self.eltorito_boot_catalog is not None:
            # An El Torito Boot Catalog already exists; add a new section.
            self.eltorito_boot_catalog.add_section(boot_dirrecord.inode,
                                                   sector_count, boot_load_seg,
                                                   media_name, system_type, efi,
                                                   bootable)
        else:
            # Step 2.
            br = headervd.BootRecord()
            br.new(b'EL TORITO SPECIFICATION')
            self.brs.append(br)
            # On a UDF ISO, adding a new Boot Record doesn't actually increase
            # the size, since there are a bunch of gaps at the beginning.
            if not self._has_udf:
                num_bytes_to_add += self.logical_block_size

            # Step 3.
            self.eltorito_boot_catalog = eltorito.EltoritoBootCatalog(br)
            self.eltorito_boot_catalog.new(br, boot_dirrecord.inode,
                                           sector_count, boot_load_seg,
                                           media_name, system_type, platform_id,
                                           bootable)

            # Step 4.
            rrname = ''
            if self.rock_ridge:
                if rr_bootcatname is None:
                    rrname = 'boot.cat'
                else:
                    rrname = rr_bootcatname

            num_bytes_to_add += self._add_fp(None, self.logical_block_size,
                                             False, bootcatfile, rrname,
                                             joliet_bootcatfile,
                                             udf_bootcatfile, None, True)

        self._finish_add(0, num_bytes_to_add)

    def rm_eltorito(self):
        # type: () -> None
        """
        Remove the El Torito boot record (and Boot Catalog) from the ISO.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        if self.eltorito_boot_catalog is None:
            raise pycdlibexception.PyCdlibInvalidInput('This ISO does not have an El Torito Boot Record')

        for brindex, br in enumerate(self.brs):
            if br.boot_system_identifier == b'EL TORITO SPECIFICATION'.ljust(32, b'\x00'):
                eltorito_index = brindex
                break
        else:
            # There was a boot catalog, but no corresponding boot record.  This
            # should never happen.
            raise pycdlibexception.PyCdlibInternalError('El Torito boot catalog found with no corresponding boot record')

        del self.brs[eltorito_index]

        num_bytes_to_remove = 0

        # On a UDF ISO, removing the Boot Record doesn't actually decrease
        # the size, since there are a bunch of gaps at the beginning.
        if not self._has_udf:
            num_bytes_to_remove += self.logical_block_size

        # Remove all of the DirectoryRecord/UDFFileEntries associated with
        # the Boot Catalog.
        for rec in self.eltorito_boot_catalog.dirrecords:
            if isinstance(rec, dr.DirectoryRecord):
                num_bytes_to_remove += self._rm_dr_link(rec)
            elif isinstance(rec, udfmod.UDFFileEntry):
                num_bytes_to_remove += self._rm_udf_link(rec)
            else:
                # This should never happen.
                raise pycdlibexception.PyCdlibInternalError('Saw an El Torito record that was neither ISO nor UDF')

        # Remove the linkage from the El Torito Entries to the inodes.
        entries_to_remove = [self.eltorito_boot_catalog.initial_entry]
        for sec in self.eltorito_boot_catalog.sections:
            for entry in sec.section_entries:
                entries_to_remove.append(entry)

        for entry in entries_to_remove:
            if entry.inode is not None:
                new_list = []
                for linkrec, is_pvd in entry.inode.linked_records:
                    if id(linkrec) != id(entry):
                        new_list.append((linkrec, is_pvd))
                entry.inode.linked_records = new_list

        num_bytes_to_remove += len(self.eltorito_boot_catalog.record())

        self.eltorito_boot_catalog = None

        self._finish_remove(num_bytes_to_remove, True)

    def add_symlink(self, symlink_path=None, rr_symlink_name=None, rr_path=None,
                    joliet_path=None, udf_symlink_path=None, udf_target=None):
        # type: (Optional[str], Optional[str], Optional[str], Optional[str], Optional[str], Optional[str]) -> None
        """
        Add a symlink from rr_symlink_name to the rr_path.  The ISO must have
        either Rock Ridge or UDF support (or both).

        Parameters:
         symlink_path - The ISO9660 path of the symlink itself on the ISO.
         rr_symlink_name - The Rock Ridge name of the symlink itself on the ISO.
         rr_path - The path that the symlink points to on the Rock Ridge part
                   of the ISO.
         joliet_path - The Joliet path of the symlink (if this ISO has Joliet).
         udf_symlink_path - The UDF path of the symlink itself on the ISO.
         udf_target - The UDF name of the entry on the ISO that the symlink
                      points to.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        # There are actually quite a few combinations and rules to think about
        # here.  Rules:
        #
        # 1.  The ISO must have Rock Ridge or UDF (or both).
        # 2.  If the rr_symlink_name or rr_path are supplied, the ISO must be a
        #     Rock Ridge one.
        # 3.  If the rr_symlink_name or rr_path are supplied, both must be
        #     supplied.
        # 4.  If udf_symlink_path or udf_target are supplied, the ISO must be a
        #     UDF one.
        # 5.  If udf_symlink_path or udf_target are supplied, both must be
        #     supplied.
        # 6.  For backwards compatibility reasons, symlink_path is a little
        #     weird.  If making a Rock Ridge symlink it is required.  If making
        #     a UDF symlink, it is optional.
        # 7.  If a symlink_path is specified, at least one of the pair of
        #     (rr_symlink_name, rr_path) or (udf_symlink_path, udf_target) must
        #     be supplied.
        # 8.  All arguments cannot be None.
        # 9.  joliet_path is the optional path on the Joliet filesystem; if it
        #     is provided, the ISO must be a Joliet one.

        if not self.rock_ridge and not self._has_udf:
            # Rule 1
            raise pycdlibexception.PyCdlibInvalidInput('Can only add a symlink to a Rock Ridge or UDF ISO')

        if (rr_symlink_name is not None or rr_path is not None) and not self.rock_ridge:
            # Rule 2
            raise pycdlibexception.PyCdlibInvalidInput('A Rock Ridge symlink can only be created on a Rock Ridge ISO')

        rr_tuple = (rr_symlink_name, rr_path)
        rr_vars_provided = len([x for x in rr_tuple if x is not None])
        if rr_vars_provided > 0 and rr_vars_provided != 2:
            # Rule 3
            raise pycdlibexception.PyCdlibInvalidInput("Both of 'rr_symlink_name' and 'rr_path' must be provided for a Rock Ridge symlink")

        if (udf_symlink_path is not None or udf_target is not None) and not self._has_udf:
            # Rule 4
            raise pycdlibexception.PyCdlibInvalidInput('A UDF symlink can only be created on a UDF ISO')

        udf_tuple = (udf_symlink_path, udf_target)
        udf_vars_provided = len([x for x in udf_tuple if x is not None])
        if udf_vars_provided > 0 and udf_vars_provided != 2:
            # Rule 5
            raise pycdlibexception.PyCdlibInvalidInput("Both of 'udf_symlink_path' and 'udf_target' must be provided for a UDF symlink")

        if rr_symlink_name is not None and symlink_path is None:
            # Rule 6
            raise pycdlibexception.PyCdlibInvalidInput("When making a Rock Ridge symlink 'symlink_path' is required")

        if symlink_path is not None and rr_vars_provided == 0 and udf_vars_provided == 0:
            # Rule 7
            raise pycdlibexception.PyCdlibInvalidInput('Either a Rock Ridge or a UDF symlink must be specified')

        all_vars_provided = len([x for x in ((symlink_path,) + rr_tuple + udf_tuple) if x is not None])
        if all_vars_provided == 0:
            # Rule 8
            raise pycdlibexception.PyCdlibInvalidInput('Either a Rock Ridge or a UDF symlink must be specified')

        if joliet_path is not None and self.joliet_vd is None:
            # Rule 9
            raise pycdlibexception.PyCdlibInvalidInput('A Joliet path can only be specified for a Joliet ISO')

        # Checks complete, we can go on to make the symlink.

        num_bytes_to_add = 0

        if symlink_path is not None:
            symlink_path_bytes = utils.normpath(symlink_path)
            (name, parent) = self._iso_name_and_parent_from_path(symlink_path_bytes)

            rec = dr.DirectoryRecord()

            if rr_symlink_name is not None and rr_path is not None:
                # We specifically do *not* normalize rr_path here, since that
                # potentially changes the meaning of what the user wanted.

                rr_symlink_name_bytes = rr_symlink_name.encode('utf-8')
                rec.new_symlink(self.pvd, name, parent, rr_path.encode('utf-8'),
                                self.pvd.sequence_number(), self.rock_ridge,
                                rr_symlink_name_bytes, self.xa, time.time())
                num_bytes_to_add += self._add_child_to_dr(rec)

                num_bytes_to_add += self._update_rr_ce_entry(rec)

        if udf_symlink_path is not None and udf_target is not None:
            # If we aren't making a Rock Ridge symlink at the same time, we need
            # to add a new zero-byte file to the ISO.
            if rr_path is None:
                tmp_joliet_path = joliet_path
                if tmp_joliet_path is None:
                    tmp_joliet_path = ''
                num_bytes_to_add += self._add_fp(None, 0, False, symlink_path,
                                                 '', tmp_joliet_path, '', None,
                                                 False)

            udf_symlink_path_bytes = utils.normpath(udf_symlink_path)

            # We specifically do *not* normalize udf_target here, since that
            # potentially changes the meaning of what the user wanted.

            (udf_name, udf_parent) = self._udf_name_and_parent_from_path(udf_symlink_path_bytes)
            file_ident = udfmod.UDFFileIdentifierDescriptor()
            file_ident.new(False, False, udf_name, udf_parent)
            num_new_extents = udf_parent.add_file_ident_desc(file_ident, self.logical_block_size)
            num_bytes_to_add += num_new_extents * self.logical_block_size

            # Generate the bytearry representing the symlink.
            symlink_bytearray = udfmod.symlink_to_bytes(udf_target)

            file_entry = udfmod.UDFFileEntry()
            file_entry.new(len(symlink_bytearray), 'symlink', udf_parent,
                           self.logical_block_size)
            file_ident.file_entry = file_entry
            file_entry.file_ident = file_ident
            num_bytes_to_add += self.logical_block_size
            num_bytes_to_add += file_entry.info_len

            # The inode for the symlink array.
            ino = inode.Inode()
            ino.new(len(symlink_bytearray), io.BytesIO(symlink_bytearray), False, 0)
            ino.linked_records.append((file_entry, False))
            ino.num_udf += 1
            file_entry.inode = ino
            self.inodes.append(ino)

            if self.udf_logical_volume_integrity is not None:
                self.udf_logical_volume_integrity.logical_volume_impl_use.num_files += 1

            # Note that we explicitly do *not* link this record to the ISO9660
            # record; that's because there is no way to correlate them during
            # parse time.  Instead, we treat them as individual entries, which
            # has the knock-on effect of requiring two operations to remove;
            # rm_file() to remove the ISO9660 record, and rm_hard_link() to
            # remove the UDF record.

        if joliet_path is not None:
            if self.joliet_vd is None:
                raise pycdlibexception.PyCdlibInternalError('Tried to add a Joliet path to a non-Joliet ISO')
            joliet_path_bytes = self._normalize_joliet_path(joliet_path)
            (joliet_name, joliet_parent) = self._joliet_name_and_parent_from_path(joliet_path_bytes)

            # Add in a "fake" symlink entry for Joliet.
            joliet_rec = dr.DirectoryRecord()
            joliet_rec.new_file(self.joliet_vd, 0, joliet_name, joliet_parent,
                                self.joliet_vd.sequence_number(), '', b'',
                                self.xa, -1, time.time())
            num_bytes_to_add += self._add_child_to_dr(joliet_rec)

        self._finish_add(0, num_bytes_to_add)

    def list_dir(self, iso_path, joliet=False):
        # type: (str, bool) -> Generator
        """
        (deprecated) Generate a list of all of the file/directory objects in the
        specified location on the ISO.  It is recommended to use the
        'list_children' API instead.

        Parameters:
         iso_path - The path on the ISO to look up information for.
         joliet - Whether to look for the path in the Joliet portion of the ISO.
        Yields:
         Children of this path.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        rec = None
        use_rr = False
        if joliet:
            rec = self._get_joliet_entry(self._normalize_joliet_path(iso_path))
        else:
            normpath = utils.normpath(iso_path)
            try:
                rec = self._get_iso_entry(normpath)
            except pycdlibexception.PyCdlibInvalidInput:
                rec = self._get_rr_entry(normpath)
                use_rr = True

        for c in _yield_children(rec, use_rr):  # pylint: disable=use-yield-from
            yield c

    def list_children(self, **kwargs):
        # type: (str) -> Generator
        """
        Generate a list of all of the file/directory objects in the
        specified location on the ISO.

        Parameters:
         iso_path - The absolute path on the ISO to list the children for.
         rr_path - The absolute Rock Ridge path on the ISO to list the children for.
         joliet_path - The absolute Joliet path on the ISO to list the children for.
         udf_path - The absolute UDF path on the ISO to list the children for.
         encoding - The string encoding used for the path; defaults to 'utf-8' or 'utf-16_be'
        Yields:
         Children of this path.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        num_paths = 0
        for key, value in kwargs.items():
            if key in ('joliet_path', 'rr_path', 'iso_path', 'udf_path'):
                if value is not None:
                    num_paths += 1
            elif key == 'encoding':
                continue
            else:
                raise pycdlibexception.PyCdlibInvalidInput(f"Invalid keyword {key}, must be one of 'iso_path', 'rr_path', 'joliet_path', or 'udf_path'")

        if num_paths != 1:
            raise pycdlibexception.PyCdlibInvalidInput("Must specify one, and only one of 'iso_path', 'rr_path', 'joliet_path', or 'udf_path'")

        if 'udf_path' in kwargs:
            udf_rec = self._get_udf_entry(kwargs['udf_path'])

            if not udf_rec.is_dir():
                raise pycdlibexception.PyCdlibInvalidInput('UDF File Entry is not a directory!')

            for fi_desc in udf_rec.fi_descs:
                yield fi_desc.file_entry
        else:
            use_rr = False
            if 'joliet_path' in kwargs:
                rec = self._get_joliet_entry(self._normalize_joliet_path(kwargs['joliet_path']), kwargs.get('encoding'))
            elif 'rr_path' in kwargs:
                rec = self._get_rr_entry(utils.normpath(kwargs['rr_path']), kwargs.get('encoding'))
                use_rr = True
            else:
                rec = self._get_iso_entry(utils.normpath(kwargs['iso_path']), kwargs.get('encoding'))

            for c in _yield_children(rec, use_rr):  # pylint: disable=use-yield-from
                yield c

    def get_entry(self, iso_path, joliet=False):
        # type: (str, bool) -> dr.DirectoryRecord
        """
        (deprecated) Get the directory record for a particular path.  It is
        recommended to use the 'get_record' API instead.

        Parameters:
         iso_path - The path on the ISO to look up information for.
         joliet - Whether to look for the path in the Joliet portion of the ISO.
        Returns:
         A dr.DirectoryRecord object representing the path.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        if joliet:
            return self._get_joliet_entry(self._normalize_joliet_path(iso_path))
        return self._get_iso_entry(utils.normpath(iso_path))

    def get_record(self, **kwargs):
        # type: (str) -> Union[dr.DirectoryRecord, udfmod.UDFFileEntry]
        """
        Get the directory record for a particular path.

        Parameters:
         iso_path - The absolute path on the ISO9660 filesystem to get the
                    record for.
         rr_path - The absolute path on the Rock Ridge filesystem to get the
                   record for.
         joliet_path - The absolute path on the Joliet filesystem to get the
                       record for.
         udf_path - The absolute path on the UDF filesystem to get the record
                    for.
        Returns:
         An object that represents the path.  This may be a dr.DirectoryRecord
         object (in the cases of iso_path, rr_path, or joliet_path), or a
         udf.UDFFileEntry object (in the case of udf_path).
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        num_paths = 0
        for key, value in kwargs.items():
            if key in ('joliet_path', 'rr_path', 'iso_path', 'udf_path'):
                if value is not None:
                    num_paths += 1
            elif key == "encoding":
                pass
            else:
                raise pycdlibexception.PyCdlibInvalidInput(f"Invalid keyword {key}, must be one of 'iso_path', 'rr_path', 'joliet_path', or 'udf_path'")

        if num_paths != 1:
            raise pycdlibexception.PyCdlibInvalidInput("Must specify one, and only one of 'iso_path', 'rr_path', 'joliet_path', or 'udf_path'")

        if 'joliet_path' in kwargs:
            return self._get_joliet_entry(self._normalize_joliet_path(kwargs['joliet_path']), kwargs.get("encoding"))
        if 'rr_path' in kwargs:
            return self._get_rr_entry(utils.normpath(kwargs['rr_path']), kwargs.get("encoding"))
        if 'udf_path' in kwargs:
            return self._get_udf_entry(kwargs['udf_path'])
        return self._get_iso_entry(utils.normpath(kwargs['iso_path']), kwargs.get("encoding"))

    def add_isohybrid(self, part_entry=1, mbr_id=None, part_offset=0,
                      geometry_sectors=32, geometry_heads=64, part_type=None,
                      mac=False, efi=None):
        # type: (int, Optional[int], int, int, int, Optional[int], bool, Optional[bool]) -> None
        """
        Make an ISO a 'hybrid', which means that it can be booted either from a
        CD or from more traditional media (like a USB stick).  This requires
        that the ISO already have El Torito, and will use the El Torito boot
        file as a bootable image.  That image must contain a certain signature
        in order to work as a hybrid (if using syslinux, this generally means
        the isohdpfx.bin files).

        Parameters:
         part_entry - The partition entry to use; one by default.
         mbr_id - The mbr_id to use.  If set to None (the default), a random one
                  will be generated.
         part_offset - The partition offset to use; zero by default.
         geometry_sectors - The number of sectors to assign; thirty-two by
                            default.
         geometry_heads - The number of heads to assign; sixty-four by default.
         part_type - The partition type to assign; twenty-three by default, but
                     will automatically be set to 0 if mac or efi are True.
         mac - Add support for Mac; False by default.
         efi - Add support for EFI; False by default, but will automatically
               be set to True if mac is True.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        if self.eltorito_boot_catalog is None:
            raise pycdlibexception.PyCdlibInvalidInput('The ISO must have an El Torito Boot Record to add isohybrid support')

        if self.eltorito_boot_catalog.initial_entry.sector_count != 4:
            raise pycdlibexception.PyCdlibInvalidInput('El Torito Boot Catalog sector count must be 4 (was actually 0x%x)' % (self.eltorito_boot_catalog.initial_entry.sector_count))

        if efi is not None:
            if not efi and mac:
                raise pycdlibexception.PyCdlibInvalidInput('If mac is True, efi must also be True')
        else:
            efi = False
            if mac:
                efi = True

        if part_type is None:
            part_type = 0x17
            if mac or efi:
                part_type = 0

        # Check that the eltorito boot file contains the appropriate
        # signature (offset 0x40, '\xFB\xC0\x78\x70').
        with inode.InodeOpenData(self.eltorito_boot_catalog.initial_entry.inode, self.logical_block_size) as (data_fp, data_len_unused):
            data_fp.seek(0x40, os.SEEK_CUR)
            signature = data_fp.read(4)

        if signature != b'\xfb\xc0\x78\x70':
            raise pycdlibexception.PyCdlibInvalidInput('Invalid signature on boot file for iso hybrid')

        self.isohybrid_mbr = isohybrid.IsoHybrid()
        self.isohybrid_mbr.new(efi, mac, part_entry, mbr_id, part_offset,
                               geometry_sectors, geometry_heads, part_type)

    def rm_isohybrid(self):
        # type: () -> None
        """
        Remove the 'hybridization' of an ISO, making it a traditional ISO again.
        This means the ISO will no longer be able to be copied and booted off
        of traditional media (like USB sticks).

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        self.isohybrid_mbr = None

    def full_path_from_dirrecord(self, rec, rockridge=False, user_encoding=''):
        # type: (Union[dr.DirectoryRecord, udfmod.UDFFileEntry], bool, str) -> str
        """
        Get the absolute path of a directory record.

        Parameters:
         rec - The directory record to get the full path for.
         rockridge - Whether to get the rock ridge full path.
         user_encoding - The string encoding used for the path as determined by the user.
        Returns:
         A string representing the absolute path to the file on the ISO.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        names = []  # type: List[str]
        if isinstance(rec, dr.DirectoryRecord):
            encoding = 'utf-8'
            if self.joliet_vd is not None and id(rec.vd) == id(self.joliet_vd):
                encoding = 'utf-16_be'

            if user_encoding:
                encoding = user_encoding

            # A root entry has no Rock Ridge entry, even on a Rock Ridge ISO.
            # Always return / here.
            if rec.is_root:
                return '/'

            if rockridge and rec.rock_ridge is None:
                raise pycdlibexception.PyCdlibInvalidInput('Cannot generate a Rock Ridge path on a non-Rock Ridge ISO')

            dr_rec = rec  # type: Optional[dr.DirectoryRecord]
            while dr_rec is not None:
                next_rec = None
                if dr_rec.is_root:
                    name = b''
                else:
                    if rockridge:
                        if dr_rec.rock_ridge is not None:
                            for child in dr_rec.children:
                                if child.is_dotdot():
                                    if child.rock_ridge is not None and child.rock_ridge.parent_link_record_exists():
                                        next_rec = child.rock_ridge.parent_link
                                    break
                            name = dr_rec.rock_ridge.name()
                        else:
                            name = dr_rec.file_identifier()
                    else:
                        name = dr_rec.file_identifier()

                names.insert(0, name.decode(encoding))
                if next_rec is not None:
                    dr_rec = next_rec
                else:
                    dr_rec = dr_rec.parent
        else:
            if rec.parent is None:
                return '/'
            if rec.file_ident is not None:
                encoding = rec.file_ident.encoding
            else:
                encoding = 'utf-8'
            if user_encoding:
                encoding = user_encoding
            udf_rec = rec  # type: Optional[udfmod.UDFFileEntry]
            while udf_rec is not None:
                ident = udf_rec.file_identifier()
                if ident == b'/':
                    name = b''
                else:
                    name = ident
                names.insert(0, name.decode(encoding))
                udf_rec = udf_rec.parent

        # Return the encoded version.
        return '/'.join(names)

    def duplicate_pvd(self):
        # type: () -> None
        """
        Add a duplicate PVD to the ISO.  This is a mostly useless feature
        allowed by Ecma-119 to have duplicate PVDs to avoid possible corruption.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        pvd = headervd.PrimaryOrSupplementaryVD(headervd.VOLUME_DESCRIPTOR_TYPE_PRIMARY)
        pvd.copy(self.pvd)
        self.pvds.append(pvd)

        self._finish_add(self.logical_block_size, 0)

    def set_hidden(self, iso_path=None, rr_path=None, joliet_path=None):
        # type: (Optional[str], Optional[str], Optional[str]) -> None
        """
        Set the ISO9660 hidden attribute on a file or directory.  This will
        cause the file or directory not to show up when listing entries on the
        ISO.  Exactly one of iso_path, rr_path, or joliet_path must be specified.

        Parameters:
         iso_path - The path on the ISO to set the hidden bit on.
         rr_path - The Rock Ridge path on the ISO to set the hidden bit on.
         joliet_path - The Joliet path on the ISO to set the hidden bit on.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        if len([x for x in (iso_path, rr_path, joliet_path) if x is not None]) != 1:
            raise pycdlibexception.PyCdlibInvalidInput('Must provide exactly one of iso_path, rr_path, or joliet_path')

        if iso_path is not None:
            rec = self._find_iso_record(utils.normpath(iso_path))
        elif rr_path is not None:
            rec = self._find_rr_record(utils.normpath(rr_path))
        elif joliet_path is not None:
            joliet_path_bytes = self._normalize_joliet_path(joliet_path)
            rec = self._find_joliet_record(joliet_path_bytes)
        else:
            raise pycdlibexception.PyCdlibInvalidInput('Must provide exactly one of iso_path, rr_path, or joliet_path')

        rec.change_existence(True)

    def clear_hidden(self, iso_path=None, rr_path=None, joliet_path=None):
        # type: (Optional[str], Optional[str], Optional[str]) -> None
        """
        Clear the ISO9660 hidden attribute on a file or directory.  This will
        cause the file or directory to show up when listing entries on the ISO.
        Exactly one of iso_path, rr_path, or joliet_path must be specified.

        Parameters:
         iso_path - The path on the ISO to clear the hidden bit from.
         rr_path - The Rock Ridge path on the ISO to clear the hidden bit from.
         joliet_path - The Joliet path on the ISO to clear the hidden bit from.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        if len([x for x in (iso_path, rr_path, joliet_path) if x is not None]) != 1:
            raise pycdlibexception.PyCdlibInvalidInput('Must provide exactly one of iso_path, rr_path, or joliet_path')

        if iso_path is not None:
            rec = self._find_iso_record(utils.normpath(iso_path))
        elif rr_path is not None:
            rec = self._find_rr_record(utils.normpath(rr_path))
        elif joliet_path is not None:
            joliet_path_bytes = self._normalize_joliet_path(joliet_path)
            rec = self._find_joliet_record(joliet_path_bytes)
        else:
            raise pycdlibexception.PyCdlibInvalidInput('Must provide exactly one of iso_path, rr_path, or joliet_path')

        rec.change_existence(False)

    def force_consistency(self):
        # type: () -> None
        """
        Make sure the ISO object is fully consistent.  PyCdlib typically delays
        doing work until it is necessary, and this detail is usually hidden
        from users.  However, there are times that a user may want a fully
        consistent view of the ISO without calling one of the methods that
        forces consistency.  This method allows the user to force a consistent
        view of this object.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        self._reshuffle_extents()

    def set_relocated_name(self, name, rr_name):
        # type: (str, str) -> None
        """
        Set the name of the relocated directory on a Rock Ridge ISO.  The ISO
        must be a Rock Ridge one, and must not have previously had the relocated
        name set.

        Parameters:
         name - The name for a relocated directory.
         rr_name - The Rock Ridge name for a relocated directory.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        if not self.rock_ridge:
            raise pycdlibexception.PyCdlibInvalidInput('Can only set the relocated name on a Rock Ridge ISO')

        encoded_name = name.encode('utf-8')
        encoded_rr_name = rr_name.encode('utf-8')
        if self._rr_moved_name is not None:
            if self._rr_moved_name == encoded_name and self._rr_moved_rr_name == encoded_rr_name:
                return
            raise pycdlibexception.PyCdlibInvalidInput('Changing the existing rr_moved name is not allowed')

        _check_iso9660_directory(encoded_name, self.interchange_level)
        self._rr_moved_name = encoded_name
        self._rr_moved_rr_name = encoded_rr_name

    def walk(self, **kwargs):
        # type: (str) -> Generator
        """
        Walk the entries on the ISO, starting at the given path.  One, and only
        one, of iso_path, rr_path, joliet_path, and udf_path is allowed.
        Similar to os.walk(), yield a 3-tuple of (path-to-here, dirlist, filelist)
        for each directory level.

        Parameters:
         iso_path - The absolute ISO path to the starting entry on the ISO.
         rr_path - The absolute Rock Ridge path to the starting entry on the ISO.
         joliet_path - The absolute Joliet path to the starting entry on the ISO.
         udf_path - The absolute UDF path to the starting entry on the ISO.
         encoding - The encoding to use for returned strings.
        Yields:
         3-tuples of (path-to-here, dirlist, filelist)
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        num_paths = 0
        user_encoding = ''
        for key, value in kwargs.items():
            if key in ('joliet_path', 'rr_path', 'iso_path', 'udf_path') and value is not None:
                num_paths += 1
            elif key == 'encoding' and value:
                user_encoding = value
            else:
                raise pycdlibexception.PyCdlibInvalidInput(f"Invalid keyword {key}, must be one of 'iso_path', 'rr_path', 'joliet_path', or 'udf_path'")

        if num_paths != 1:
            raise pycdlibexception.PyCdlibInvalidInput("Must specify one, and only one of 'iso_path', 'rr_path', 'joliet_path', or 'udf_path'")

        rec = None  # type: Optional[Union[dr.DirectoryRecord, udfmod.UDFFileEntry]]
        if 'joliet_path' in kwargs:
            joliet_path = self._normalize_joliet_path(kwargs['joliet_path'])
            rec = self._find_joliet_record(joliet_path)
            path_type = 'joliet_path'
            default_encoding = 'utf-16_be'
        elif 'udf_path' in kwargs:
            if self.udf_root is None:
                raise pycdlibexception.PyCdlibInvalidInput('Can only specify a UDF path for a UDF ISO')
            (ident_unused, rec) = self._find_udf_record(utils.normpath(kwargs['udf_path']))
            if rec is None:
                raise pycdlibexception.PyCdlibInvalidInput('Cannot get entry for empty UDF File Entry')
            path_type = 'udf_path'
            default_encoding = None
        elif 'rr_path' in kwargs:
            if not self.rock_ridge:
                raise pycdlibexception.PyCdlibInvalidInput('Cannot fetch a rr_path from a non-Rock Ridge ISO')
            rec = self._find_rr_record(utils.normpath(kwargs['rr_path']))
            path_type = 'rr_path'
            default_encoding = 'utf-8'
        else:
            rec = self._find_iso_record(utils.normpath(kwargs['iso_path']))
            path_type = 'iso_path'
            default_encoding = 'utf-8'

        dirs = collections.deque([rec])
        while dirs:
            dir_record = dirs.popleft()

            relpath = self.full_path_from_dirrecord(dir_record, rockridge=path_type == 'rr_path',
                                                    user_encoding=user_encoding)
            dirlist = []
            filelist = []
            dirdict = {}

            fallback_encoding = default_encoding or 'utf-8'
            encoding = user_encoding or fallback_encoding
            for child in reversed(list(self.list_children(**{path_type: relpath, 'encoding': encoding}))):
                if child is None or child.is_dot() or child.is_dotdot():
                    continue

                if isinstance(child, udfmod.UDFFileEntry) and child.file_ident is not None:
                    encoding = child.file_ident.encoding

                if path_type == 'rr_path':
                    name = cast(dr.DirectoryRecord, child).rock_ridge.name()
                else:
                    name = child.file_identifier()

                # If the user-specified encoding is wrong, use the fallback encoding
                try:
                    encoded = name.decode(encoding)
                except UnicodeDecodeError:
                    encoded = name.decode(fallback_encoding)

                if child.is_dir():
                    dirlist.append(encoded)
                    dirdict[encoded] = child
                else:
                    filelist.append(encoded)

            yield relpath, dirlist, filelist

            # We allow the user to modify dirlist along the way, so we
            # add the children to dirs *after* yield returns.
            for name in dirlist:
                dirs.appendleft(dirdict[name])

    def open_file_from_iso(self, **kwargs):
        # type: (str) -> pycdlibio.PyCdlibIO
        """
        Open a file for reading in a context manager.  This allows the user to
        operate on the file in user-defined chunks (utilizing the read() method
        of the returned context manager).

        Parameters:
         iso_path - The absolute ISO path to the file on the ISO.
         rr_path - The absolute Rock Ridge path to the file on the ISO.
         joliet_path - The absolute Joliet path to the file on the ISO.
         udf_path - The absolute UDF path to the file on the ISO.
        Returns:
         A PyCdlibIO object allowing access to the file.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        num_paths = 0
        rec = None  # type: Optional[Union[dr.DirectoryRecord, udfmod.UDFFileEntry]]
        for key, value in kwargs.items():
            if key in ('joliet_path', 'rr_path', 'iso_path', 'udf_path'):
                if value is not None:
                    num_paths += 1
            else:
                raise pycdlibexception.PyCdlibInvalidInput("Invalid keyword, must be one of 'iso_path', 'rr_path', 'joliet_path', or 'udf_path'")

        if num_paths != 1:
            raise pycdlibexception.PyCdlibInvalidInput("Must specify one, and only one of 'iso_path', 'rr_path', 'joliet_path', or 'udf_path'")

        if 'joliet_path' in kwargs:
            joliet_path = self._normalize_joliet_path(kwargs['joliet_path'])
            rec = self._find_joliet_record(joliet_path)
        elif 'udf_path' in kwargs:
            if self.udf_root is None:
                raise pycdlibexception.PyCdlibInvalidInput('Can only specify a UDF path for a UDF ISO')
            (ident_unused, rec) = self._find_udf_record(utils.normpath(kwargs['udf_path']))
            if rec is None:
                raise pycdlibexception.PyCdlibInvalidInput('Cannot get entry for empty UDF File Entry')
        elif 'rr_path' in kwargs:
            if not self.rock_ridge:
                raise pycdlibexception.PyCdlibInvalidInput('Cannot fetch a rr_path from a non-Rock Ridge ISO')
            rec = self._find_rr_record(utils.normpath(kwargs['rr_path']))
        else:
            rec = self._find_iso_record(utils.normpath(kwargs['iso_path']))

        if not rec.is_file():
            raise pycdlibexception.PyCdlibInvalidInput('Path to open must be a file')

        if rec.inode is None:
            raise pycdlibexception.PyCdlibInvalidInput('File has no data')

        return pycdlibio.PyCdlibIO(rec.inode, self.logical_block_size)

    def has_rock_ridge(self):
        # type: () -> bool
        """
        Returns whether this ISO has Rock Ridge extensions.

        Parameters:
         None.
        Returns:
         True if this ISO has Rock Ridge extensions, False otherwise.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')
        return self.rock_ridge != ''

    def has_joliet(self):
        # type: () -> bool
        """
        Returns whether this ISO has Joliet extensions.

        Parameters:
         None.
        Returns:
         True if this ISO has Joliet, False otherwise.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')
        return self.joliet_vd is not None

    def has_udf(self):
        # type: () -> bool
        """
        Returns whether this ISO has UDF extensions.

        Parameters:
         None.
        Returns:
         True if this ISO has UDF, False otherwise.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')
        return self._has_udf

    def get_iso9660_facade(self):
        # type: () -> facade.PyCdlibISO9660
        """
        Return a 'facade' that simplifies some of the complexities of the
        PyCdlib class, while giving up some of the full power.  This facade
        only allows manipulation of the ISO9660 portions of the ISO.

        Parameters:
         None.
        Returns:
         A PyCdlibISO9660 object that can be used to interact with the ISO.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        return facade.PyCdlibISO9660(self)

    def get_joliet_facade(self):
        # type: () -> facade.PyCdlibJoliet
        """
        Return a 'facade' that simplifies some of the complexities of the
        PyCdlib class, while giving up some of the full power.  This facade
        only allows manipulation of the Joliet portions of the ISO.

        Parameters:
         None.
        Returns:
         A PyCdlibJoliet object that can be used to interact with the ISO.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')
        if self.joliet_vd is None:
            raise pycdlibexception.PyCdlibInvalidInput('Can only get a Joliet facade for a Joliet ISO')

        return facade.PyCdlibJoliet(self)

    def get_rock_ridge_facade(self):
        # type: () -> facade.PyCdlibRockRidge
        """
        Return a 'facade' that simplifies some of the complexities of the
        PyCdlib class, while giving up some of the full power.  This facade
        only allows manipulation of the Rock Ridge portions of the ISO.

        Parameters:
         None.
        Returns:
         A PyCdlibRockRidge object that can be used to interact with the ISO.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')
        if self.rock_ridge == '':
            raise pycdlibexception.PyCdlibInvalidInput('Can only get a Rock Ridge facade for a Rock Ridge ISO')

        return facade.PyCdlibRockRidge(self)

    def get_udf_facade(self):
        # type: () -> facade.PyCdlibUDF
        """
        Return a 'facade' that simplifies some of the complexities of the
        PyCdlib class, while giving up some of the full power.  This facade
        only allows manipulation of the UDF portions of the ISO.

        Parameters:
         None.
        Returns:
         A PyCdlibUDF object that can be used to interact with the ISO.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')
        if not self._has_udf:
            raise pycdlibexception.PyCdlibInvalidInput('Can only get a UDF facade for a UDF ISO')

        return facade.PyCdlibUDF(self)

    def file_mode(self, **kwargs):
        # type: (str) -> Optional[int]
        """
        Get the POSIX file mode of the file if is a Rock Ridge file.

        Parameters:
         iso_path - The absolute ISO path to the file on the ISO.
         rr_path - The absolute Rock Ridge path to the file on the ISO.
         joliet_path - The absolute Joliet path to the file on the ISO.
         udf_path - The absolute UDF path to the file on the ISO.
        Returns:
         An integer representing the POSIX file mode of the file if it is Rock
         Ridge, or None otherwise.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        num_paths = 0
        for key, value in kwargs.items():
            if key in ('joliet_path', 'rr_path', 'iso_path', 'udf_path'):
                if value is not None:
                    num_paths += 1
            else:
                raise pycdlibexception.PyCdlibInvalidInput("Invalid keyword, must be one of 'iso_path', 'rr_path', 'joliet_path', or 'udf_path'")

        if num_paths != 1:
            raise pycdlibexception.PyCdlibInvalidInput("Must specify one, and only one of 'iso_path', 'rr_path', 'joliet_path', or 'udf_path'")

        file_mode = None
        if 'rr_path' in kwargs:
            if not self.rock_ridge:
                raise pycdlibexception.PyCdlibInvalidInput('Cannot fetch a rr_path from a non-Rock Ridge ISO')
            rec = self._find_rr_record(utils.normpath(kwargs['rr_path']))
            if rec.rock_ridge is not None:
                if rec.rock_ridge.dr_entries.px_record is not None or rec.rock_ridge.ce_entries.px_record is not None:
                    file_mode = rec.rock_ridge.get_file_mode()

        # Neither Joliet nor ISO know the file_mode, and we don't support setting
        # the file mode for UDF, so just return None in those cases

        return file_mode

    def close(self):
        # type: () -> None
        """
        Close the PyCdlib object, and re-initialize the object to the defaults.
        The object can then be re-used for manipulation of another ISO.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInvalidInput('This object is not initialized; call either open() or new() to create an ISO')

        if self._managing_fp:
            self._cdfp.close()

        self._initialize()
