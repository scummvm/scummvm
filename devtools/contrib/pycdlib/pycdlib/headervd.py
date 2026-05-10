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

"""Implementation of header Volume Descriptors for Ecma-119/ISO9660."""

import struct
import time

from pycdlib import dates
from pycdlib import dr
from pycdlib import pycdlibexception
from pycdlib import rockridge
from pycdlib import utils

# For mypy annotations
if False:  # pylint: disable=using-constant-test
    from typing import List, Tuple  # NOQA pylint: disable=unused-import

VOLUME_DESCRIPTOR_TYPE_BOOT_RECORD = 0
VOLUME_DESCRIPTOR_TYPE_PRIMARY = 1
VOLUME_DESCRIPTOR_TYPE_SUPPLEMENTARY = 2
VOLUME_DESCRIPTOR_TYPE_VOLUME_PARTITION = 3
VOLUME_DESCRIPTOR_TYPE_SET_TERMINATOR = 255

allzero = b'\x00' * 2048


class PrimaryOrSupplementaryVD:
    """
    A class representing a Primary or Supplementary Volume Descriptor on this
    ISO.  These are the first things on the ISO that are parsed, and contain all
    of the basic information about the ISO.
    """
    __slots__ = ('rr_ce_blocks', 'system_identifier', 'volume_identifier',
                 'path_table_location_le', 'optional_path_table_location_le',
                 'path_table_location_be', 'optional_path_table_location_be',
                 'volume_set_identifier', 'copyright_file_identifier',
                 'abstract_file_identifier', 'bibliographic_file_identifier',
                 'file_structure_version', 'application_use', 'set_size',
                 'publisher_identifier', 'preparer_identifier',
                 'application_identifier', 'volume_creation_date',
                 'volume_modification_date', 'volume_expiration_date',
                 'volume_effective_date', '_vd_type', 'escape_sequences',
                 'flags', 'version', '_initialized', 'space_size',
                 'log_block_size', 'root_dir_record', 'path_tbl_size',
                 'path_table_num_extents', 'seqnum', 'new_extent_loc',
                 'orig_extent_loc', 'encoding')

    FMT = '<B5sBB32s32sQLL32sHHHHHHLLLLLL34s128s128s128s128s37s37s37s17s17s17s17sBB512s653s'

    def __init__(self, vd_type):
        # type: (int) -> None
        self._initialized = False
        self.space_size = 0
        self.log_block_size = 0
        self.root_dir_record = dr.DirectoryRecord()
        self.path_tbl_size = 0
        self.path_table_num_extents = 0
        self.seqnum = 0
        self.new_extent_loc = -1
        # Only used for PVD
        self.rr_ce_blocks = []  # type: List[rockridge.RockRidgeContinuationBlock]

        self._vd_type = vd_type

    def parse(self, vd, extent_loc):
        # type: (bytes, int) -> None
        """
        Parse a Volume Descriptor out of a string.

        Parameters:
         vd - The string containing the Volume Descriptor.
         extent_loc - The location on the ISO of this Volume Descriptor.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Primary Volume Descriptor is already initialized')

        ################ PVD VERSION ######################
        (descriptor_type, identifier, self.version, self.flags,
         self.system_identifier, self.volume_identifier, unused1,
         space_size_le, space_size_be, self.escape_sequences, set_size_le,
         set_size_be, seqnum_le, seqnum_be, logical_block_size_le,
         logical_block_size_be, path_table_size_le, path_table_size_be,
         self.path_table_location_le, self.optional_path_table_location_le,
         self.path_table_location_be, self.optional_path_table_location_be,
         root_dir_record, self.volume_set_identifier, pub_ident_str,
         prepare_ident_str, app_ident_str, self.copyright_file_identifier,
         self.abstract_file_identifier, self.bibliographic_file_identifier,
         vol_create_date_str, vol_mod_date_str, vol_expire_date_str,
         vol_effective_date_str, self.file_structure_version, unused2,
         self.application_use, zero_unused) = struct.unpack_from(self.FMT, vd, 0)

        # According to Ecma-119, 8.4.1, the primary volume descriptor type
        # should be 1.
        if descriptor_type != self._vd_type:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid volume descriptor')
        # According to Ecma-119, 8.4.2, the identifier should be 'CD001'.
        if identifier != b'CD001':
            raise pycdlibexception.PyCdlibInvalidISO('invalid CD isoIdentification')
        # According to Ecma-119, 8.4.3, the version should be 1 (or 2 for
        # ISO9660:1999)
        expected_versions = [1]
        if self._vd_type == VOLUME_DESCRIPTOR_TYPE_SUPPLEMENTARY:
            expected_versions.append(2)
        if self.version not in expected_versions:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid volume descriptor version %d' % (self.version))
        # According to Ecma-119, 8.4.4, the first flags field should be 0 for a Primary.
        if self._vd_type == VOLUME_DESCRIPTOR_TYPE_PRIMARY and self.flags != 0:
            raise pycdlibexception.PyCdlibInvalidISO('PVD flags field is not zero')
        # According to Ecma-119, 8.4.5, the first unused field (after the
        # system identifier and volume identifier) should be 0.
        if unused1 != 0:
            raise pycdlibexception.PyCdlibInvalidISO('data in 2nd unused field not zero')
        # According to Ecma-119, 8.4.9, the escape sequences for a PVD should
        # be 32 zero-bytes.  However, we have seen ISOs in the wild (Fantastic
        # Night Dreams - Cotton Original (Japan).cue from the psx redump
        # collection) that don't have this set to 0, so allow anything here.

        # According to Ecma-119, 8.4.30, the file structure version should be 1.
        # However, we have seen ISOs in the wild that that don't have this
        # properly set to one.  In those cases, forcibly set it to one and let
        # it pass.
        if self._vd_type == VOLUME_DESCRIPTOR_TYPE_PRIMARY:
            if self.file_structure_version != 1:
                self.file_structure_version = 1
        elif self._vd_type == VOLUME_DESCRIPTOR_TYPE_SUPPLEMENTARY:
            if self.file_structure_version not in (1, 2):
                raise pycdlibexception.PyCdlibInvalidISO('File structure version expected to be 1')
        # According to Ecma-119, 8.4.31, the second unused field should be 0.
        if unused2 != 0:
            raise pycdlibexception.PyCdlibInvalidISO('data in 2nd unused field not zero')
        # According to Ecma-119, the last 653 bytes of the VD should be all 0.
        # However, we have seen ISOs in the wild that do not follow this, so
        # relax the check.

        # Check to make sure that the little-endian and big-endian versions
        # of the parsed data agree with each other.
        if space_size_le != utils.swab_32bit(space_size_be):
            raise pycdlibexception.PyCdlibInvalidISO('Little-endian and big-endian space size disagree')
        self.space_size = space_size_le

        if set_size_le != utils.swab_16bit(set_size_be):
            raise pycdlibexception.PyCdlibInvalidISO('Little-endian and big-endian set size disagree')
        self.set_size = set_size_le

        if seqnum_le != utils.swab_16bit(seqnum_be):
            raise pycdlibexception.PyCdlibInvalidISO('Little-endian and big-endian seqnum disagree')
        self.seqnum = seqnum_le

        if logical_block_size_le != utils.swab_16bit(logical_block_size_be):
            raise pycdlibexception.PyCdlibInvalidISO('Little-endian and big-endian logical block size disagree')
        self.log_block_size = logical_block_size_le

        if path_table_size_le != utils.swab_32bit(path_table_size_be):
            raise pycdlibexception.PyCdlibInvalidISO('Little-endian and big-endian path table size disagree')
        self.path_tbl_size = path_table_size_le

        self.path_table_location_be = utils.swab_32bit(self.path_table_location_be)

        # Here we calculate the number of extents that the path tables take.
        # This is simply the difference between the beginning of the BE path table
        # and the LE path table. We have to calculate it like this because while
        # most ISOs in the wild use 4096 between these groups, some ISOs do not.
        # FIXME: In point of fact, this is *also* wrong.  There is no guarantee
        # that these two are adjacent, so calculating the difference between them
        # is kind of meaningless.  The right answer here is to get rid of
        # self.path_table_num_extents altogether, though it is unclear to me how
        # to specify to reshuffle_extents exactly how much space we should allocate
        # for the path tables.
        self.path_table_num_extents = self.path_table_location_be - self.path_table_location_le

        self.encoding = 'ascii'
        if self.escape_sequences in (b'%/@'.ljust(32, b'\x00'), b'%/C'.ljust(32, b'\x00'), b'%/E'.ljust(32, b'\x00')):
            self.encoding = 'utf-16_be'

        self.publisher_identifier = FileOrTextIdentifier()
        self.publisher_identifier.parse(pub_ident_str)
        self.preparer_identifier = FileOrTextIdentifier()
        self.preparer_identifier.parse(prepare_ident_str)
        self.application_identifier = FileOrTextIdentifier()
        self.application_identifier.parse(app_ident_str)
        self.volume_creation_date = dates.VolumeDescriptorDate()
        self.volume_creation_date.parse(vol_create_date_str)
        self.volume_modification_date = dates.VolumeDescriptorDate()
        self.volume_modification_date.parse(vol_mod_date_str)
        self.volume_expiration_date = dates.VolumeDescriptorDate()
        self.volume_expiration_date.parse(vol_expire_date_str)
        self.volume_effective_date = dates.VolumeDescriptorDate()
        self.volume_effective_date.parse(vol_effective_date_str)
        self.root_dir_record.parse(self, root_dir_record, None)

        self.orig_extent_loc = extent_loc

        self._initialized = True

    def new(self, flags, sys_ident, vol_ident, set_size, seqnum, log_block_size,
            vol_set_ident, pub_ident_str, preparer_ident_str, app_ident_str,
            copyright_file, abstract_file, bibli_file, vol_expire_date,
            app_use, xa, version, escape_sequence):
        # type: (int, bytes, bytes, int, int, int, bytes, bytes, bytes, bytes, bytes, bytes, bytes, float, bytes, bool, int, bytes) -> None
        """
        Create a new Volume Descriptor.

        Parameters:
         flags - The flags to set for this Volume Descriptor (must be 0 for a
                 Primay Volume Descriptor).
         sys_ident - The system identification string to use on the new ISO.
         vol_ident - The volume identification string to use on the new ISO.
         set_size - The size of the set of ISOs this ISO is a part of.
         seqnum - The sequence number of the set of this ISO.
         log_block_size - The logical block size to use for the ISO.  While
                          ISO9660 technically supports sizes other than 2048
                          (the default), this almost certainly doesn't work.
         vol_set_ident - The volume set identification string to use on the
                         new ISO.
         pub_ident_str - The publisher identification string to use on the new ISO.
         preparer_ident_str - The preparer identification string to use on the new
                              ISO.
         app_ident_str - The application identification string to use on the new
                         ISO.
         copyright_file - The name of a file at the root of the ISO to use as
                          the copyright file.
         abstract_file - The name of a file at the root of the ISO to use as the
                         abstract file.
         bibli_file - The name of a file at the root of the ISO to use as the
                      bibliographic file.
         vol_expire_date - The date that this ISO will expire at.
         app_use - Arbitrary data that the application can stuff into the
                   primary volume descriptor of this ISO.
         xa - Whether to embed XA data into the volume descriptor.
         version - What version to assign to the header (ignored).
         escape_sequence - The escape sequence to assign to this volume
                           descriptor (must be empty for a PVD, or empty or a
                           valid Joliet escape sequence for an SVD).
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Primary Volume Descriptor is already initialized')

        self.encoding = 'ascii'
        if self._vd_type == VOLUME_DESCRIPTOR_TYPE_PRIMARY:
            if flags != 0:
                raise pycdlibexception.PyCdlibInvalidInput('Non-zero flags not allowed for a PVD')
            if escape_sequence != b'':
                raise pycdlibexception.PyCdlibInvalidInput('Non-empty escape sequence not allowed for a PVD')
            if version != 1:
                raise pycdlibexception.PyCdlibInvalidInput('Only version 1 supported for a PVD')
            self.escape_sequences = b'\x00' * 32
        elif self._vd_type == VOLUME_DESCRIPTOR_TYPE_SUPPLEMENTARY:
            if version not in (1, 2):
                raise pycdlibexception.PyCdlibInvalidInput('Only version 1 and version 2 supported for a Supplementary Volume Descriptor')
            if escape_sequence in (b'%/@', b'%/C', b'%/E'):
                self.encoding = 'utf-16_be'
            self.escape_sequences = escape_sequence.ljust(32, b'\x00')

        self.file_structure_version = version
        self.version = version
        self.flags = 0

        if len(sys_ident) > 32:
            raise pycdlibexception.PyCdlibInvalidInput('The system identifer has a maximum length of 32')
        self.system_identifier = utils.encode_space_pad(sys_ident, 32, self.encoding)

        if len(vol_ident) > 32:
            raise pycdlibexception.PyCdlibInvalidInput('The volume identifier has a maximum length of 32')
        self.volume_identifier = utils.encode_space_pad(vol_ident, 32, self.encoding)

        # The space_size is the number of extents (2048-byte blocks) in the
        # ISO.  We know we will at least have the system area (16 extents),
        # and this VD (1 extent) to start with; the rest will be added later.
        self.space_size = 17
        self.set_size = set_size
        if seqnum > set_size:
            raise pycdlibexception.PyCdlibInvalidInput('Sequence number must be less than or equal to set size')
        self.seqnum = seqnum
        self.log_block_size = log_block_size
        # The path table size is in bytes, and is always at least 10 bytes
        # (for the root directory record).
        self.path_tbl_size = 10
        self.path_table_num_extents = utils.ceiling_div(self.path_tbl_size, 4096) * 2
        # By default the Little Endian Path Table record starts at extent 19
        # (right after the Volume Terminator).
        self.path_table_location_le = 19
        # By default the Big Endian Path Table record starts at extent 21
        # (two extents after the Little Endian Path Table Record).
        self.path_table_location_be = 21
        # FIXME: we don't support the optional path table location right now
        self.optional_path_table_location_le = 0
        self.optional_path_table_location_be = 0
        self.root_dir_record.new_root(self, seqnum, self.log_block_size, time.time())

        if len(vol_set_ident) > 128:
            raise pycdlibexception.PyCdlibInvalidInput('The maximum length for the volume set identifier is 128')
        self.volume_set_identifier = utils.encode_space_pad(vol_set_ident, 128, self.encoding)

        self.publisher_identifier = FileOrTextIdentifier()
        self.publisher_identifier.new(utils.encode_space_pad(pub_ident_str, 128, self.encoding))

        self.preparer_identifier = FileOrTextIdentifier()
        self.preparer_identifier.new(utils.encode_space_pad(preparer_ident_str, 128, self.encoding))

        self.application_identifier = FileOrTextIdentifier()
        self.application_identifier.new(utils.encode_space_pad(app_ident_str, 128, self.encoding))

        self.copyright_file_identifier = utils.encode_space_pad(copyright_file, 37, self.encoding)
        self.abstract_file_identifier = utils.encode_space_pad(abstract_file, 37, self.encoding)
        self.bibliographic_file_identifier = utils.encode_space_pad(bibli_file, 37, self.encoding)

        now = time.time()
        self.volume_creation_date = dates.VolumeDescriptorDate()
        self.volume_creation_date.new(now)
        # We make a valid volume modification date here, but it will get
        # overwritten during record().
        self.volume_modification_date = dates.VolumeDescriptorDate()
        self.volume_modification_date.new(now)
        self.volume_expiration_date = dates.VolumeDescriptorDate()
        self.volume_expiration_date.new(vol_expire_date)
        self.volume_effective_date = dates.VolumeDescriptorDate()
        self.volume_effective_date.new(now)

        if xa:
            if len(app_use) > 141:
                raise pycdlibexception.PyCdlibInvalidInput('Cannot have XA and an app_use of > 140 bytes')
            self.application_use = app_use.ljust(141, b' ')
            self.application_use += b'CD-XA001' + b'\x00' * 18
            self.application_use = self.application_use.ljust(512, b' ')
        else:
            if len(app_use) > 512:
                raise pycdlibexception.PyCdlibInvalidInput('The maximum length for the application use is 512')
            self.application_use = app_use.ljust(512, b' ')

        self._initialized = True

    def copy(self, orig):
        # type: (PrimaryOrSupplementaryVD) -> None
        """
        Populate and initialize this VD object from the contents of an old VD.

        Parameters:
         orig_pvd - The original VD to copy data from.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Volume Descriptor is already initialized')

        self.version = orig.version
        self.flags = orig.flags
        self.system_identifier = orig.system_identifier
        self.volume_identifier = orig.volume_identifier
        self.escape_sequences = orig.escape_sequences
        self.space_size = orig.space_size
        self.set_size = orig.set_size
        self.seqnum = orig.seqnum
        self.log_block_size = orig.log_block_size
        self.path_tbl_size = orig.path_tbl_size
        self.path_table_location_le = orig.path_table_location_le
        self.optional_path_table_location_le = orig.optional_path_table_location_le
        self.path_table_location_be = orig.path_table_location_be
        self.optional_path_table_location_be = orig.optional_path_table_location_be
        # Root dir record is a DirectoryRecord object, and we want this copy to
        # hold onto exactly the same reference as the original
        self.root_dir_record = orig.root_dir_record
        self.volume_set_identifier = orig.volume_set_identifier
        # publisher_identifier is a FileOrTextIdentifier object, and we want
        # this copy to hold onto exactly the same reference as the original
        self.publisher_identifier = orig.publisher_identifier
        # preparer_identifier is a FileOrTextIdentifier object, and we want this
        # copy to hold onto exactly the same reference as the original
        self.preparer_identifier = orig.preparer_identifier
        # application_identifier is a FileOrTextIdentifier object, and we want
        # this copy to hold onto exactly the same reference as the original
        self.application_identifier = orig.application_identifier
        self.copyright_file_identifier = orig.copyright_file_identifier
        self.abstract_file_identifier = orig.abstract_file_identifier
        self.bibliographic_file_identifier = orig.bibliographic_file_identifier
        # volume_creation_date is a VolumeDescriptorDate object, and we want
        # this copy to hold onto exactly the same reference as the original
        self.volume_creation_date = orig.volume_creation_date
        # volume_expiration_date is a VolumeDescriptorDate object, and we want
        # this copy to hold onto exactly the same reference as the original
        self.volume_expiration_date = orig.volume_expiration_date
        # volume_effective_date is a VolumeDescriptorDate object, and we want
        # this copy to hold onto exactly the same reference as the original
        self.volume_effective_date = orig.volume_effective_date
        self.file_structure_version = orig.file_structure_version
        self.application_use = orig.application_use
        self.encoding = orig.encoding

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate the string representing this Volume Descriptor.

        Parameters:
         None.
        Returns:
         A string representing this Volume Descriptor.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Volume Descriptor is not initialized')

        vol_mod_date = dates.VolumeDescriptorDate()
        vol_mod_date.new(time.time())

        return struct.pack(self.FMT,
                           self._vd_type,
                           b'CD001',
                           self.version,
                           self.flags,
                           self.system_identifier,
                           self.volume_identifier,
                           0,
                           self.space_size,
                           utils.swab_32bit(self.space_size),
                           self.escape_sequences,
                           self.set_size,
                           utils.swab_16bit(self.set_size),
                           self.seqnum,
                           utils.swab_16bit(self.seqnum),
                           self.log_block_size,
                           utils.swab_16bit(self.log_block_size),
                           self.path_tbl_size,
                           utils.swab_32bit(self.path_tbl_size),
                           self.path_table_location_le,
                           self.optional_path_table_location_le,
                           utils.swab_32bit(self.path_table_location_be),
                           self.optional_path_table_location_be,
                           self.root_dir_record.record(),
                           self.volume_set_identifier,
                           self.publisher_identifier.record(),
                           self.preparer_identifier.record(),
                           self.application_identifier.record(),
                           self.copyright_file_identifier,
                           self.abstract_file_identifier,
                           self.bibliographic_file_identifier,
                           self.volume_creation_date.record(),
                           vol_mod_date.record(),
                           self.volume_expiration_date.record(),
                           self.volume_effective_date.record(),
                           self.file_structure_version, 0, self.application_use,
                           b'\x00' * 653)

    def track_rr_ce_entry(self, extent, offset, length):
        # type: (int, int, int) -> rockridge.RockRidgeContinuationBlock
        """
        Start tracking a new Rock Ridge Continuation Entry entry in this Volume
        Descriptor, at the extent, offset, and length provided.  Since Rock
        Ridge Continuation Blocks are shared across multiple Rock Ridge
        Directory Records, the most logical place to track them is in the PVD.
        This method is expected to be used during parse time, when an extent,
        offset and length are already assigned to the entry.

        Parameters:
         extent - The extent that this Continuation Entry lives at.
         offset - The offset within the extent that this Continuation Entry
                  lives at.
         length - The length of this Continuation Entry.
        Returns:
         The object representing the block in which the Continuation Entry was
         placed in.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Primary Volume Descriptor is not initialized')

        for block in self.rr_ce_blocks:
            if block.extent_location() == extent:
                break
        else:
            # We didn't find it in the list, add it
            block = rockridge.RockRidgeContinuationBlock(extent, self.log_block_size)
            self.rr_ce_blocks.append(block)

        block.track_entry(offset, length)

        return block

    def add_rr_ce_entry(self, length):
        # type: (int) -> Tuple[bool, rockridge.RockRidgeContinuationBlock, int]
        """
        Add a new Rock Ridge Continuation Entry to this PVD; see
        track_rr_ce_entry() above for why we track these in the PVD.  This
        method is used to add a new Continuation Entry anywhere it fits in the
        list of Continuation Blocks.  If it doesn't fit in any of the existing
        blocks, a new block for it is allocated.

        Parameters:
         length - The length of the Continuation Entry that should be added.
        Returns:
         A 3-tuple consisting of whether we added a new block, the object
         representing the block that this entry was added to, and the offset
         within the block that the entry was added to.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Primary Volume Descriptor is not initialized')

        added_block = False
        for block in self.rr_ce_blocks:
            offset = block.add_entry(length)
            if offset is not None:
                break
        else:
            # We didn't find a block this would fit in; add one.
            block = rockridge.RockRidgeContinuationBlock(0, self.log_block_size)
            self.rr_ce_blocks.append(block)
            offset = block.add_entry(length)
            added_block = True

        return (added_block, block, offset)

    def clear_rr_ce_entries(self):
        # type: () -> None
        """
        Clear out all of the extent locations of all Rock Ridge Continuation
        Entries that the PVD is tracking.  This can be used to reset all data
        before assigning new data.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Primary Volume Descriptor is not initialized')

        for block in self.rr_ce_blocks:
            block.set_extent_location(-1)

    def path_table_size(self):
        # type: () -> int
        """
        Get the path table size of the Volume Descriptor.

        Parameters:
         None.
        Returns:
         Path table size in bytes.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Volume Descriptor is not initialized')

        return self.path_tbl_size

    def add_to_space_size(self, addition_bytes):
        # type: (int) -> None
        """
        Add bytes to the space size tracked by this Volume Descriptor.

        Parameters:
         addition_bytes - The number of bytes to add to the space size.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Volume Descriptor is not initialized')
        # The 'addition' parameter is expected to be in bytes, but the space
        # size we track is in extents.  Round up to the next extent.
        self.space_size += utils.ceiling_div(addition_bytes, self.log_block_size)

    def remove_from_space_size(self, removal_bytes):
        # type: (int) -> None
        """
        Remove bytes from the volume descriptor.

        Parameters:
         removal_bytes - The number of bytes to remove.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Volume Descriptor is not initialized')
        # The 'removal' parameter is expected to be in bytes, but the space
        # size we track is in extents.  Round up to the next extent.
        self.space_size -= utils.ceiling_div(removal_bytes, self.log_block_size)

    def root_directory_record(self):
        # type: () -> dr.DirectoryRecord
        """
        Get a handle to this Volume Descriptor's root directory record.

        Parameters:
         None.
        Returns:
         DirectoryRecord object representing this Volume Descriptor's root
         directory record.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Volume Descriptor is not initialized')

        return self.root_dir_record

    def logical_block_size(self):
        # type: () -> int
        """
        Get this Volume Descriptor's logical block size.

        Parameters:
         None.
        Returns:
         Size of this Volume Descriptor's logical block size in bytes.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Volume Descriptor is not initialized')

        return self.log_block_size

    def add_to_ptr_size(self, ptr_size):
        # type: (int) -> bool
        """
        Add the space for a path table record to the volume descriptor.

        Parameters:
         ptr_size - The length of the Path Table Record being added to this Volume Descriptor.
        Returns:
         True if extents need to be added to the Volume Descriptor, False otherwise.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Volume Descriptor is not initialized')

        # First add to the path table size.
        self.path_tbl_size += ptr_size
        if (utils.ceiling_div(self.path_tbl_size, 4096) * 2) > self.path_table_num_extents:
            # If we overflowed the path table size, then we need to update the
            # space size.  Since we always add two extents for the little and
            # two for the big, add four total extents.  The locations will be
            # fixed up during reshuffle_extents.
            self.path_table_num_extents += 2
            return True
        return False

    def remove_from_ptr_size(self, ptr_size):
        # type: (int) -> bool
        """
        Remove the space for a path table record from the volume descriptor.

        Parameters:
         ptr_size - The length of the Path Table Record being removed from this Volume Descriptor.
        Returns:
         True if extents need to be removed from the Volume Descriptor, False otherwise.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Volume Descriptor is not initialized')

        # Next remove from the Path Table Record size.
        self.path_tbl_size -= ptr_size
        new_extents = utils.ceiling_div(self.path_tbl_size, 4096) * 2

        need_remove_extents = False
        if new_extents > self.path_table_num_extents:
            # This should never happen.
            raise pycdlibexception.PyCdlibInvalidInput('Extent number should never grow when removing PTR')
        if new_extents < self.path_table_num_extents:
            self.path_table_num_extents -= 2
            need_remove_extents = True

        return need_remove_extents

    def sequence_number(self):
        # type: () -> int
        """
        Get this Volume Descriptor's sequence number.

        Parameters:
         None.
        Returns:
         This Volume Descriptor's sequence number.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Volume Descriptor is not initialized')

        return self.seqnum

    def copy_sizes(self, othervd):
        # type: (PrimaryOrSupplementaryVD) -> None
        """
        Copy the path_tbl_size, path_table_num_extents, and space_size from
        another volume descriptor.

        Parameters:
         othervd - The other volume descriptor to copy from.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Volume Descriptor is not initialized')

        self.space_size = othervd.space_size
        self.path_tbl_size = othervd.path_tbl_size
        self.path_table_num_extents = othervd.path_table_num_extents

    def extent_location(self):
        # type: () -> int
        """
        Get this Volume Descriptor's extent location.

        Parameters:
         None.
        Returns:
         Integer of this Volume Descriptor's extent location.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Volume Descriptor is not initialized')

        if self.new_extent_loc < 0:
            return self.orig_extent_loc
        return self.new_extent_loc

    def set_extent_location(self, extent):
        # type: (int) -> None
        """
        Set the new location for this PVD/SVD.

        Parameters:
         extent - The new extent location to set for this PVD/SVD.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Volume Descriptor is not initialized')
        self.new_extent_loc = extent

    def is_pvd(self):
        # type: () -> bool
        """
        Determine whether this Volume Descriptor is a Primary one.

        Parameters:
         None.
        Returns:
         True if this Volume Descriptor is Primary, False otherwise.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Volume Descriptor is not initialized')
        return self._vd_type == VOLUME_DESCRIPTOR_TYPE_PRIMARY

    def __ne__(self, other):
        return self.version != other.version or self.flags != other.flags or \
            self.system_identifier != other.system_identifier or \
            self.volume_identifier != other.volume_identifier or \
            self.escape_sequences != other.escape_sequences or \
            self.space_size != other.space_size or \
            self.set_size != other.set_size or self.seqnum != other.seqnum or \
            self.log_block_size != other.log_block_size or \
            self.path_tbl_size != other.path_tbl_size or \
            self.path_table_location_le != other.path_table_location_le or \
            self.optional_path_table_location_le != other.optional_path_table_location_le or \
            self.path_table_location_be != other.path_table_location_be or \
            self.optional_path_table_location_be != other.optional_path_table_location_be or \
            self.root_dir_record != other.root_dir_record or \
            self.volume_set_identifier != other.volume_set_identifier or \
            self.publisher_identifier != other.publisher_identifier or \
            self.preparer_identifier != other.preparer_identifier or \
            self.application_identifier != other.application_identifier or \
            self.copyright_file_identifier != other.copyright_file_identifier or \
            self.abstract_file_identifier != other.abstract_file_identifier or \
            self.bibliographic_file_identifier != other.bibliographic_file_identifier or \
            self.volume_creation_date != other.volume_creation_date or \
            self.volume_modification_date != other.volume_modification_date or \
            self.volume_expiration_date != other.volume_expiration_date or \
            self.volume_effective_date != other.volume_effective_date or \
            self.file_structure_version != other.file_structure_version or \
            self.application_use != other.application_use


def pvd_factory(sys_ident, vol_ident, set_size, seqnum, log_block_size,
                vol_set_ident, pub_ident_str, preparer_ident_str,
                app_ident_str, copyright_file, abstract_file, bibli_file,
                vol_expire_date, app_use, xa):
    # type: (bytes, bytes, int, int, int, bytes, bytes, bytes, bytes, bytes, bytes, bytes, float, bytes, bool) -> PrimaryOrSupplementaryVD
    """
    An internal function to create a Primary Volume Descriptor.

    Parameters:
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
     xa - Whether to add the ISO9660 Extended Attribute extensions to this
          ISO.  The default is False.
    Returns:
     The newly created Primary Volume Descriptor.
    """
    pvd = PrimaryOrSupplementaryVD(VOLUME_DESCRIPTOR_TYPE_PRIMARY)
    pvd.new(0, sys_ident, vol_ident, set_size, seqnum, log_block_size,
            vol_set_ident, pub_ident_str, preparer_ident_str,
            app_ident_str, copyright_file, abstract_file, bibli_file,
            vol_expire_date, app_use, xa, 1, b'')
    return pvd


def enhanced_vd_factory(sys_ident, vol_ident, set_size, seqnum,
                        log_block_size, vol_set_ident, pub_ident_str,
                        preparer_ident_str, app_ident_str, copyright_file,
                        abstract_file, bibli_file, vol_expire_date, app_use,
                        xa):
    # type: (bytes, bytes, int, int, int, bytes, bytes, bytes, bytes, bytes, bytes, bytes, float, bytes, bool) -> PrimaryOrSupplementaryVD
    """
    An internal function to create an Enhanced Volume Descriptor for ISO 1999.

    Parameters:
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
     xa - Whether to add the ISO9660 Extended Attribute extensions to this
          ISO.  The default is False.
    Returns:
     The newly created Enhanced Volume Descriptor.
    """
    svd = PrimaryOrSupplementaryVD(VOLUME_DESCRIPTOR_TYPE_SUPPLEMENTARY)
    svd.new(0, sys_ident, vol_ident, set_size, seqnum, log_block_size,
            vol_set_ident, pub_ident_str, preparer_ident_str,
            app_ident_str, copyright_file, abstract_file, bibli_file,
            vol_expire_date, app_use, xa, 2, b'')
    return svd


def joliet_vd_factory(joliet, sys_ident, vol_ident, set_size, seqnum,
                      log_block_size, vol_set_ident, pub_ident_str,
                      preparer_ident_str, app_ident_str, copyright_file,
                      abstract_file, bibli_file, vol_expire_date, app_use, xa):
    # type: (int, bytes, bytes, int, int, int, bytes, bytes, bytes, bytes, bytes, bytes, bytes, float, bytes, bool) -> PrimaryOrSupplementaryVD
    """
    An internal function to create an Joliet Volume Descriptor.

    Parameters:
     joliet - The joliet version to use, one of 1, 2, or 3.
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
     xa - Whether to add the ISO9660 Extended Attribute extensions to this
          ISO.  The default is False.
    Returns:
     The newly created Joliet Volume Descriptor.
    """
    if joliet == 1:
        escape_sequence = b'%/@'
    elif joliet == 2:
        escape_sequence = b'%/C'
    elif joliet == 3:
        escape_sequence = b'%/E'
    else:
        raise pycdlibexception.PyCdlibInvalidInput('Invalid Joliet level; must be 1, 2, or 3')

    svd = PrimaryOrSupplementaryVD(VOLUME_DESCRIPTOR_TYPE_SUPPLEMENTARY)
    svd.new(0, sys_ident, vol_ident, set_size, seqnum, log_block_size,
            vol_set_ident, pub_ident_str, preparer_ident_str, app_ident_str,
            copyright_file, abstract_file,
            bibli_file, vol_expire_date, app_use, xa, 1, escape_sequence)
    return svd


class FileOrTextIdentifier:
    """
    A class to represent a file or text identifier as specified in Ecma-119
    section 8.4.20 (Primary Volume Descriptor Publisher Identifier),
    section 8.4.21 (Primary Volume Descriptor Data Preparer Identifier),
    and section 8.4.22 (Primary Volume Descriptor Application Identifier).  This
    identifier can either be a text string or the name of a file.  If it is a
    file, then the first byte will be 0x5f, the file should exist in the root
    directory record, and the file should be ISO level 1 interchange compliant
    (no more than 8 characters for the name and 3 characters for the extension).
    There are two main ways to use this class: either to instantiate and then
    parse a string to fill in the fields (the parse() method), or to create a
    new entry with a text string and whether this is a filename or not (the
    new() method).
    """
    __slots__ = ('_initialized', 'text')

    def __init__(self):
        # type: () -> None
        self._initialized = False

    def parse(self, ident_str):
        # type: (bytes) -> None
        """
        Parse a file or text identifier out of a string.

        Parameters:
          ident_str  - The string to parse the file or text identifier from.
        Returns:
          Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This File or Text identifier is already initialized')
        self.text = ident_str

        # FIXME: we do not support a file identifier here.  In the future, we
        # might want to implement this.

        self._initialized = True

    def new(self, text):
        # type: (bytes) -> None
        """
        Create a new file or text identifier.

        Parameters:
          text   - The text to store into the identifier.
        Returns:
          Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This File or Text identifier is already initialized')

        if len(text) != 128:
            raise pycdlibexception.PyCdlibInvalidInput('Length of text must be 128')

        self.text = text

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Returns the file or text identification string suitable for recording.

        Parameters:
          None.
        Returns:
          The text representing this identifier.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This File or Text identifier is not initialized')
        return self.text

    def __eq__(self, other):
        # type: (object) -> bool
        if not isinstance(other, FileOrTextIdentifier):
            return NotImplemented
        return self.text == other.text

    def __ne__(self, other):
        # type: (object) -> bool
        equal = self.__eq__(other)
        if equal == NotImplemented:
            return NotImplemented
        return not equal


class VolumeDescriptorSetTerminator:
    """
    A class that represents a Volume Descriptor Set Terminator.  The VDST
    signals the end of volume descriptors on the ISO.
    """
    __slots__ = ('_initialized', 'orig_extent_loc', 'new_extent_loc')

    FMT = '=B5sB2041s'

    def __init__(self):
        # type: () -> None
        self.new_extent_loc = -1
        self._initialized = False

    def parse(self, vd, extent_loc):
        # type: (bytes, int) -> None
        """
        Parse a Volume Descriptor Set Terminator out of a string.

        Parameters:
         vd - The string to parse.
         extent_loc - The extent this VDST is currently located at.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Volume Descriptor Set Terminator already initialized')

        (descriptor_type, identifier, version,
         zero_unused) = struct.unpack_from(self.FMT, vd, 0)

        # According to Ecma-119, 8.3.1, the volume descriptor set terminator
        # type should be 255
        if descriptor_type != VOLUME_DESCRIPTOR_TYPE_SET_TERMINATOR:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid VDST descriptor type')
        # According to Ecma-119, 8.3.2, the identifier should be 'CD001'
        if identifier != b'CD001':
            raise pycdlibexception.PyCdlibInvalidISO('Invalid VDST identifier')
        # According to Ecma-119, 8.3.3, the version should be 1
        # However, we've seen ISOs in the wild (mostly those created by
        # makeps3iso at https://github.com/bucanero/ps3iso-utils) that set the
        # VDST to 0, so accept that as well.
        if version not in (0, 1):
            raise pycdlibexception.PyCdlibInvalidISO('Invalid VDST version')
        # According to Ecma-119, 8.3.4, the rest of the terminator should be 0;
        # however, we have seen ISOs in the wild that put stuff into this field.
        # Just ignore it.

        self.orig_extent_loc = extent_loc

        self._initialized = True

    def new(self):
        # type: () -> None
        """
        Create a new Volume Descriptor Set Terminator.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Volume Descriptor Set Terminator already initialized')

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate a string representing this Volume Descriptor Set Terminator.

        Parameters:
         None.
        Returns:
         String representing this Volume Descriptor Set Terminator.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Volume Descriptor Set Terminator not initialized')
        return struct.pack(self.FMT, VOLUME_DESCRIPTOR_TYPE_SET_TERMINATOR,
                           b'CD001', 1, b'\x00' * 2041)

    def extent_location(self):
        # type: () -> int
        """
        Get this Volume Descriptor Set Terminator's extent location.

        Parameters:
         None.
        Returns:
         Integer extent location.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Volume Descriptor Set Terminator not initialized')

        if self.new_extent_loc < 0:
            return self.orig_extent_loc
        return self.new_extent_loc

    def set_extent_location(self, extent):
        # type: (int) -> None
        """
        Set the new location for this Volume Descriptor Set Terminator.

        Parameters:
         extent - The new extent location to set for this Volume Descriptor Set Terminator.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Volume Descriptor is not initialized')
        self.new_extent_loc = extent


def vdst_factory():
    # type: () -> VolumeDescriptorSetTerminator
    """
    An internal function to create a new Volume Descriptor Set Terminator.

    Parameters:
     None.
    Returns:
     The newly created Volume Descriptor Set Terminator.
    """
    vdst = VolumeDescriptorSetTerminator()
    vdst.new()
    return vdst


class BootRecord:
    """A class representing an ISO9660 Boot Record."""
    __slots__ = ('_initialized', 'boot_system_identifier', 'boot_identifier',
                 'boot_system_use', 'orig_extent_loc', 'new_extent_loc')

    FMT = '=B5sB32s32s1977s'

    def __init__(self):
        # type: () -> None
        self.new_extent_loc = -1
        self._initialized = False

    def parse(self, vd, extent_loc):
        # type: (bytes, int) -> None
        """
        Parse a Boot Record out of a string.

        Parameters:
         vd - The string to parse the Boot Record out of.
         extent_loc - The extent location this Boot Record is current at.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Boot Record already initialized')

        (descriptor_type, identifier, version,
         self.boot_system_identifier, self.boot_identifier,
         self.boot_system_use) = struct.unpack_from(self.FMT, vd, 0)

        # According to Ecma-119, 8.2.1, the boot record type should be 0
        if descriptor_type != VOLUME_DESCRIPTOR_TYPE_BOOT_RECORD:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid boot record descriptor type')
        # According to Ecma-119, 8.2.2, the identifier should be 'CD001'
        if identifier != b'CD001':
            raise pycdlibexception.PyCdlibInvalidISO('Invalid boot record identifier')
        # According to Ecma-119, 8.2.3, the version should be 1
        if version != 1:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid boot record version')

        self.orig_extent_loc = extent_loc

        self._initialized = True

    def new(self, boot_system_id):
        # type: (bytes) -> None
        """
        Create a new Boot Record.

        Parameters:
         boot_system_id - The system identifier to associate with this Boot
                          Record.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Boot Record already initialized')

        self.boot_system_identifier = boot_system_id.ljust(32, b'\x00')
        self.boot_identifier = b'\x00' * 32
        self.boot_system_use = b'\x00' * 1977  # This will be set later

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate a string representing this Boot Record.

        Parameters:
         None.
        Returns:
         A string representing this Boot Record.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Boot Record not initialized')

        return struct.pack(self.FMT, VOLUME_DESCRIPTOR_TYPE_BOOT_RECORD,
                           b'CD001', 1, self.boot_system_identifier,
                           self.boot_identifier, self.boot_system_use)

    def update_boot_system_use(self, boot_sys_use):
        # type: (bytes) -> None
        """
        Update the boot system use field of this Boot Record.

        Parameters:
         boot_sys_use - The new boot system use field for this Boot Record.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Boot Record not initialized')

        if len(boot_sys_use) != 1977:
            raise pycdlibexception.PyCdlibInternalError('Boot system use field must be 1977 bytes')

        self.boot_system_use = boot_sys_use

    def extent_location(self):
        # type: () -> int
        """
        Get the extent location of this Boot Record.

        Parameters:
         None.
        Returns:
         Integer extent location of this Boot Record.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('Boot Record not initialized')

        if self.new_extent_loc < 0:
            return self.orig_extent_loc
        return self.new_extent_loc

    def set_extent_location(self, extent):
        # type: (int) -> None
        """
        Set the new location for this Boot Record.

        Parameters:
         extent - The new extent location to set for this Boot Record.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Volume Descriptor is not initialized')
        self.new_extent_loc = extent


class VersionVolumeDescriptor:
    """
    A class representing a Version Volume Descriptor.  This volume descriptor is
    not mentioned in any of the standards, but is included by genisoimage, so it
    is modeled here.
    """
    __slots__ = ('_initialized', 'orig_extent_loc', 'new_extent_loc', '_data')

    def __init__(self):
        # type: () -> None
        self.new_extent_loc = -1
        self._initialized = False

    def parse(self, data, extent):
        # type: (bytes, int) -> bool
        """
        Do a parse of a Version Volume Descriptor.  This consists of seeing
        whether the data is either all zero or starts with 'MKI', and if so,
        setting the extent location of the Version Volume Descriptor properly.

        Parameters:
         data - The potential version data.
         extent - The location of the extent on the original ISO of this
                  Version Volume Descriptor.
        Returns:
         True if the data passed in is a Version Descriptor, False otherwise.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Version Volume Descriptor is already initialized')

        if data[:3] == b'MKI' or data == allzero:
            # OK, we have a version descriptor.
            self._data = data
            self.orig_extent_loc = extent
            self._initialized = True
            return True

        return False

    def new(self, log_block_size):
        # type: (int) -> None
        """
        Create a new Version Volume Descriptor.

        Parameters:
         log_block_size - The size of one extent.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Version Volume Descriptor is already initialized')

        self._data = b'\x00' * log_block_size
        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate a string representing this Version Volume Descriptor.  Note that
        right now, this is always a string of zeros.

        Parameters:
         log_block_size - The logical block size to use when generating this string.
        Returns:
         A string representing this Version Volume Descriptor.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Version Volume Descriptor is not initialized')

        return self._data

    def extent_location(self):
        # type: () -> int
        """
        Get the extent location of this Version Volume Descriptor.

        Parameters:
         None.
        Returns:
         An integer representing the extent location of this Version Volume
         Descriptor.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Version Volume Descriptor is not initialized')

        if self.new_extent_loc < 0:
            return self.orig_extent_loc
        return self.new_extent_loc

    def set_extent_location(self, extent):
        # type: (int) -> None
        """
        Set the new location for this Version Volume Descriptor.

        Parameters:
         extent - The new extent location to set for this Version Volume Descriptor.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Volume Descriptor is not initialized')
        self.new_extent_loc = extent


def version_vd_factory(log_block_size):
    # type: (int) -> VersionVolumeDescriptor
    """
    An internal function to create a new Version Volume Descriptor.

    Parameters:
     log_block_size - The size of one extent.
    Returns:
     The newly created Version Volume Descriptor.
    """
    version_vd = VersionVolumeDescriptor()
    version_vd.new(log_block_size)
    return version_vd
