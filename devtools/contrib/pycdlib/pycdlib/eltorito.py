# Copyright (C) 2015-2019  Chris Lalancette <clalancette@gmail.com>

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

"""Classes to support El Torito."""

import logging
import struct

from pycdlib import pycdlibexception
from pycdlib import utils

# For mypy annotations
if False:  # pylint: disable=using-constant-test
    from typing import List, Union  # NOQA pylint: disable=unused-import
    # NOTE: these imports have to be here to avoid circular deps
    from pycdlib import dr  # NOQA pylint: disable=unused-import
    from pycdlib import headervd  # NOQA pylint: disable=unused-import
    from pycdlib import inode    # NOQA pylint: disable=unused-import
    from pycdlib import udf as udfmod  # NOQA pylint: disable=unused-import


_logger = logging.getLogger('pycdlib')


class EltoritoBootInfoTable:
    """
    A class that represents an El Torito Boot Info Table.  The Boot Info Table
    is an optional table that may be patched into the boot file at offset 8,
    and is 64-bytes long.
    """
    __slots__ = ('_initialized', 'orig_len', 'csum', 'vd', 'inode')

    def __init__(self):
        # type: () -> None
        self._initialized = False

    def parse(self, vd, datastr, ino):
        # type: (headervd.PrimaryOrSupplementaryVD, bytes, inode.Inode) -> bool
        """
        Parse a boot info table out of a string.

        Parameters:
         vd - The Volume Descriptor associated with this Boot Info Table.
         datastr - The string to parse the boot info table out of.
         ino - The Inode associated with the boot file.
        Returns:
         True if this is a valid El Torito Boot Info Table, False otherwise.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Eltorito Boot Info Table is already initialized')
        # http://xpt.sourceforge.net/techdocs/media/cd/cd09-BootableCDs/
        # suggests that this is all little-endian, so we'll take its
        # word for it for now.
        (pvd_extent, rec_extent, self.orig_len,
         self.csum) = struct.unpack_from('<LLLL', datastr, 0)

        if pvd_extent != vd.extent_location() or rec_extent != ino.extent_location():
            return False

        self.vd = vd
        self.inode = ino
        self._initialized = True

        return True

    def new(self, vd, ino, orig_len, csum):
        # type: (headervd.PrimaryOrSupplementaryVD, inode.Inode, int, int) -> None
        """
        Create a new boot info table.

        Parameters:
         vd - The volume descriptor to associate with this boot info table.
         ino - The Inode associated with this Boot Info Table.
         orig_len - The original length of the file before the boot info table was patched into it.
         csum - The checksum for the boot file, starting at the byte after the boot info table.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Eltorito Boot Info Table is already initialized')
        self.vd = vd
        self.orig_len = orig_len
        self.csum = csum
        self.inode = ino
        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate a string representing this boot info table.

        Parameters:
         None.
        Returns:
         A string representing this boot info table.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This Eltorito Boot Info Table not initialized')

        return struct.pack('<LLLL', self.vd.extent_location(),
                           self.inode.extent_location(), self.orig_len,
                           self.csum) + b'\x00' * 40

    @staticmethod
    def header_length():
        # type: () -> int
        """
        Static method to return the length of the boot info table header
        (ignoring the 40 bytes of padding).

        Parameters:
         None.
        Returns:
         An integer describing the length of the boot info table header.
        """
        return 16


class EltoritoValidationEntry:
    """
    A class that represents an El Torito Validation Entry.  El Torito requires
    that the first entry in the El Torito Boot Catalog be a validation entry.
    """
    __slots__ = ('_initialized', 'platform_id', 'id_string', 'checksum')

    # An El Torito validation entry consists of:
    # Offset 0x0:       Header ID (0x1)
    # Offset 0x1:       Platform ID (0 for x86, 1 for PPC, 2 for Mac)
    # Offset 0x2-0x3:   Reserved, must be 0
    # Offset 0x4-0x1b:  ID String for manufacturer of CD
    # Offset 0x1c-0x1d: Checksum of all bytes.
    # Offset 0x1e:      Key byte 0x55
    # Offset 0x1f:      Key byte 0xaa
    FMT = '<BBH24sHBB'

    def __init__(self):
        # type: () -> None
        self._initialized = False

    @staticmethod
    def _checksum(data):
        # type: (bytes) -> int
        """
        A static method to compute the checksum on the ISO.  Note that this is
        *not* a 1's complement checksum; when an addition overflows, the carry
        bit is discarded, not added to the end.

        Parameters:
         data - The data to compute the checksum over.
        Returns:
         The checksum of the data.
        """
        if isinstance(data, bytes):
            myord = int
        else:
            myord = ord
        csum = 0
        for i, val in enumerate(data):
            short = (myord(val) << (8 * (i % 2))) & 0xffff
            # Because we are looping through bytes, and because we know the
            # above conversion can shift us up by a maximum of 8 bits, we know
            # that the sign bit is at offset 15.  If it is signed, OR with
            # 0xffff0000 to make sure it is sign-extended.
            adder = short
            if short & (1 << 15):
                adder |= 0xffff0000
            csum += adder
        return (-csum) & 0xffff

    def parse(self, valstr):
        # type: (bytes) -> None
        """
        Parse an El Torito Validation Entry out of a string.

        Parameters:
         valstr - The string to parse the El Torito Validation Entry out of.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('El Torito Validation Entry already initialized')

        (header_id, self.platform_id, reserved_unused, self.id_string,
         self.checksum, keybyte1,
         keybyte2) = struct.unpack_from(self.FMT, valstr, 0)

        if header_id != 1:
            raise pycdlibexception.PyCdlibInvalidISO('El Torito Validation entry header ID not 1')

        if self.platform_id not in (0, 1, 2, 0xef):
            raise pycdlibexception.PyCdlibInvalidISO('El Torito Validation entry platform ID not valid')

        if keybyte1 != 0x55:
            raise pycdlibexception.PyCdlibInvalidISO('El Torito Validation entry first keybyte not 0x55')
        if keybyte2 != 0xaa:
            raise pycdlibexception.PyCdlibInvalidISO('El Torito Validation entry second keybyte not 0xaa')

        # Now that we've done basic checking, calculate the checksum of the
        # validation entry and make sure it is right.
        if self._checksum(valstr) != 0:
            raise pycdlibexception.PyCdlibInvalidISO('El Torito Validation entry checksum not correct')

        self._initialized = True

    def new(self, platform_id):
        # type: (int) -> None
        """
        Create a new El Torito Validation Entry.

        Parameters:
         platform_id - The platform ID to set for this validation entry.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('El Torito Validation Entry already initialized')
        if platform_id not in (0, 1, 2, 0xef):
            raise pycdlibexception.PyCdlibInvalidInput('Invalid platform ID (must be one of 0, 1, 2, or 0xef)')

        self.platform_id = platform_id
        self.id_string = b'\x00' * 24  # FIXME: let the user set this
        self.checksum = 0
        self.checksum = self._checksum(self._record())
        self._initialized = True

    def _record(self):
        # type: () -> bytes
        """
        An internal method to generate a string representing this El Torito
        Validation Entry.

        Parameters:
         None.
        Returns:
         String representing this El Torito Validation Entry.
        """
        return struct.pack(self.FMT, 1, self.platform_id, 0, self.id_string,
                           self.checksum, 0x55, 0xaa)

    def record(self):
        # type: () -> bytes
        """
        Generate a string representing this El Torito Validation Entry.

        Parameters:
         None.
        Returns:
         String representing this El Torito Validation Entry.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('El Torito Validation Entry not initialized')

        return self._record()


class EltoritoEntry:
    """A class that represents an El Torito Entry (Initial or Section)."""
    __slots__ = ('_initialized', 'inode', 'boot_indicator',
                 'boot_media_type', 'load_segment', 'system_type',
                 'sector_count', 'load_rba', 'selection_criteria_type',
                 'selection_criteria')

    # An El Torito entry consists of:
    # Offset 0x0:      Boot indicator (0x88 for bootable, 0x00 for
    #                  non-bootable)
    # Offset 0x1:      Boot media type.  One of 0x0 for no emulation,
    #                  0x1 for 1.2M diskette emulation, 0x2 for 1.44M
    #                  diskette emulation, 0x3 for 2.88M diskette
    #                  emulation, or 0x4 for Hard Disk emulation.
    # Offset 0x2-0x3:  Load Segment - if 0, use traditional 0x7C0.
    # Offset 0x4:      System Type - copy of Partition Table byte 5
    # Offset 0x5:      Unused, must be 0
    # Offset 0x6-0x7:  Sector Count - Number of virtual sectors to store
    #                  during initial boot.
    # Offset 0x8-0xb:  Load RBA - Start address of virtual disk.
    # For Initial Entry:
    # Offset 0xc-0x1f: Unused, must be 0.
    # For Section Entry:
    # Offset 0xc:      Selection criteria type
    # Offset 0xd-0x1f: Selection critera
    FMT = '<BBHBBHLB19s'
    MEDIA_NO_EMUL = 0
    MEDIA_12FLOPPY = 1
    MEDIA_144FLOPPY = 2
    MEDIA_288FLOPPY = 3
    MEDIA_HD_EMUL = 4

    def __init__(self):
        # type: () -> None
        self._initialized = False

    def parse(self, valstr):
        # type: (bytes) -> None
        """
        Parse an El Torito Entry out of a string.

        Parameters:
         valstr - The string to parse the El Torito Entry out of.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('El Torito Entry already initialized')

        (self.boot_indicator, self.boot_media_type, self.load_segment,
         self.system_type, unused1, self.sector_count, self.load_rba,
         self.selection_criteria_type,
         self.selection_criteria) = struct.unpack_from(self.FMT, valstr, 0)

        if self.boot_indicator not in (0x88, 0x00):
            raise pycdlibexception.PyCdlibInvalidISO('Invalid El Torito initial entry boot indicator')
        if self.boot_media_type > 4:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid El Torito boot media type')

        # FIXME: check that the system type matches the partition table

        if unused1 != 0:
            raise pycdlibexception.PyCdlibInvalidISO('El Torito unused field must be 0')

        # According to the specification, the El Torito unused end field (bytes
        # 0xc - 0x1f, unused2 field) should be all zero.  However, we have found
        # ISOs in the wild where that is not the case, so skip that particular
        # check here.

        self._initialized = True

    def new(self, sector_count, load_seg, media_name, system_type, bootable):
        # type: (int, int, str, int, bool) -> None
        """
        Create a new El Torito Entry.

        Parameters:
         sector_count - The number of sectors to assign to this El Torito Entry.
         load_seg - The load segment address of the boot image.
         media_name - The name of the media type, one of 'noemul', 'floppy', or 'hdemul'.
         system_type - The partition type to assign to the entry.
         bootable - Whether this entry is bootable.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('El Torito Entry already initialized')

        if media_name == 'noemul':
            media_type = self.MEDIA_NO_EMUL
        elif media_name == 'floppy':
            if sector_count == 2400:
                media_type = self.MEDIA_12FLOPPY
            elif sector_count == 2880:
                media_type = self.MEDIA_144FLOPPY
            elif sector_count == 5760:
                media_type = self.MEDIA_288FLOPPY
            else:
                raise pycdlibexception.PyCdlibInvalidInput('Invalid sector count for floppy media type; must be 2400, 2880, or 5760')
            # With floppy booting, the sector_count always ends up being 1
            sector_count = 1
        elif media_name == 'hdemul':
            media_type = self.MEDIA_HD_EMUL
            # With HD emul booting, the sector_count always ends up being 1
            sector_count = 1
        else:
            raise pycdlibexception.PyCdlibInvalidInput("Invalid media name '%s'" % (media_name))

        if bootable:
            self.boot_indicator = 0x88
        else:
            self.boot_indicator = 0
        self.boot_media_type = media_type
        self.load_segment = load_seg
        self.system_type = system_type
        self.sector_count = sector_count
        self.load_rba = 0  # This will get set later
        self.selection_criteria_type = 0  # FIXME: allow the user to set this
        self.selection_criteria = b''.ljust(19, b'\x00')

        self._initialized = True

    def get_rba(self):
        # type: () -> int
        """
        Get the load_rba for this El Torito Entry.

        Parameters:
         None.
        Returns:
         The load RBA for this El Torito Entry.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('El Torito Entry not initialized')

        return self.load_rba

    def set_data_location(self, current_extent, tag_location):  # pylint: disable=unused-argument
        # type: (int, int) -> None
        """
        Update the extent (and RBA) for this entry.

        Parameters:
         current_extent - The new extent to set for this entry.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('El Torito Entry not initialized')

        self.load_rba = current_extent

    def set_inode(self, ino):
        # type: (inode.Inode) -> None
        """
        Set the Inode associated with this El Torito Entry.

        Parameters:
         ino - The Inode object corresponding to this entry.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('El Torito Entry not initialized')
        self.inode = ino

    def record(self):
        # type: () -> bytes
        """
        Generate a string representing this El Torito Entry.

        Parameters:
         None.
        Returns:
         String representing this El Torito Entry.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('El Torito Entry not initialized')

        return struct.pack(self.FMT, self.boot_indicator, self.boot_media_type,
                           self.load_segment, self.system_type, 0,
                           self.sector_count, self.load_rba,
                           self.selection_criteria_type,
                           self.selection_criteria)

    def length(self):
        # type: () -> int
        """
        Get the length, in bytes, of this El Torito Entry.

        Parameters:
         None.
        Returns:
         An integer representing the length in bytes of this El Torito Entry.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('El Torito Entry not initialized')
        # According to El Torito, the sector count is in virtual sectors, which
        # are defined to be 512 bytes.
        return self.sector_count * 512

    def set_data_length(self, length):
        # type: (int) -> None
        """
        Set the length of data for this El Torito Entry.

        Parameters:
         length - The new length for the El Torito Entry.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('El Torito Entry not initialized')
        self.sector_count = utils.ceiling_div(length, 512)


class EltoritoSectionHeader:
    """A class that represents an El Torito Section Header."""
    __slots__ = ('_initialized', 'header_indicator', 'platform_id',
                 'num_section_entries', 'id_string', 'section_entries')

    FMT = '<BBH28s'

    def __init__(self):
        # type: () -> None
        self._initialized = False
        self.section_entries = []  # type: List[EltoritoEntry]

    def parse(self, valstr):
        # type: (bytes) -> None
        """
        Parse an El Torito section header from a string.

        Parameters:
         valstr - The string to parse.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('El Torito Section Header already initialized')

        (self.header_indicator, self.platform_id, self.num_section_entries,
         self.id_string) = struct.unpack_from(self.FMT, valstr, 0)

        self._initialized = True

    def new(self, id_string, platform_id):
        # type: (bytes, int) -> None
        """
        Create a new El Torito section header.

        Parameters:
         id_string - The ID to use for this section header.
         platform_id - The platform ID for this section header.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('El Torito Section Header already initialized')

        # We always assume this is the last section, until we are told otherwise
        # via set_record_not_last.
        self.header_indicator = 0x91
        self.platform_id = platform_id
        self.num_section_entries = 0
        self.id_string = id_string
        self._initialized = True

    def add_parsed_entry(self, entry):
        # type: (EltoritoEntry) -> None
        """
        Add a parsed entry to the list of entries of this header. If the number
        of parsed entries exceeds what was expected from the initial parsing of
        the header, this method will throw an Exception.

        Parameters:
         entry - The EltoritoEntry object to add to the list of entries.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('El Torito Section Header not initialized')

        if len(self.section_entries) >= self.num_section_entries:
            raise pycdlibexception.PyCdlibInvalidISO('El Torito section had more entries than expected by section header; ISO is corrupt')

        self.section_entries.append(entry)

    def add_new_entry(self, entry):
        # type: (EltoritoEntry) -> None
        """
        Add a completely new entry to the list of entries of this header.

        Parameters:
         entry - The new EltoritoEntry object to add to the list of entries.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('El Torito Section Header not initialized')

        self.num_section_entries += 1

        self.section_entries.append(entry)

    def set_record_not_last(self):
        # type: () -> None
        """
        Set this Section Header so that it is *not* the last one in the Boot
        Catalog; this is used when a new header is added.

        Parameters:
         None.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('El Torito Section Header not initialized')

        self.header_indicator = 0x90

    def record(self):
        # type: () -> bytes
        """
        Get a string representing this El Torito section header.

        Parameters:
         None.
        Returns:
         A string representing this El Torito section header.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('El Torito Section Header not initialized')

        outlist = [struct.pack(self.FMT, self.header_indicator,
                               self.platform_id, self.num_section_entries,
                               self.id_string)]

        for entry in self.section_entries:
            outlist.append(entry.record())

        return b''.join(outlist)


class EltoritoBootCatalog:
    """
    A class that represents an El Torito Boot Catalog.  The boot catalog is the
    basic unit of El Torito, and is expected to contain a validation entry,
    an initial entry, and zero or more section entries.
    """
    __slots__ = ('_initialized', 'dirrecords', 'br', 'initial_entry',
                 'validation_entry', 'sections', 'standalone_entries', 'state')

    EXPECTING_VALIDATION_ENTRY = 1
    EXPECTING_INITIAL_ENTRY = 2
    EXPECTING_SECTION_HEADER_OR_DONE = 3

    def __init__(self, br):
        # type: (headervd.BootRecord) -> None
        self.dirrecords = []  # type: List[Union[dr.DirectoryRecord, udfmod.UDFFileEntry]]
        self._initialized = False
        self.br = br
        self.initial_entry = EltoritoEntry()
        self.validation_entry = EltoritoValidationEntry()
        self.sections = []  # type: List[EltoritoSectionHeader]
        self.standalone_entries = []  # type: List[EltoritoEntry]
        self.state = self.EXPECTING_VALIDATION_ENTRY

    def parse(self, valstr):
        # type: (bytes) -> bool
        """
        Parse an El Torito Boot Catalog out of a string.

        Parameters:
         valstr - The string to parse the El Torito Boot Catalog out of.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('El Torito Boot Catalog already initialized')

        if self.state == self.EXPECTING_VALIDATION_ENTRY:
            # The first entry in an El Torito boot catalog is the Validation
            # Entry.  A Validation entry consists of 32 bytes (described in
            # detail in the parse_eltorito_validation_entry() method).
            self.validation_entry.parse(valstr)
            self.state = self.EXPECTING_INITIAL_ENTRY
        elif self.state == self.EXPECTING_INITIAL_ENTRY:
            # The next entry is the Initial/Default entry.  An Initial/Default
            # entry consists of 32 bytes (described in detail in the
            # parse_eltorito_initial_entry() method).
            self.initial_entry.parse(valstr)
            self.state = self.EXPECTING_SECTION_HEADER_OR_DONE
        else:
            val = bytes(bytearray([valstr[0]]))
            if val == b'\x00':
                # An empty entry tells us we are done parsing El Torito.  Do
                # some sanity checks.
                last_section_index = len(self.sections) - 1
                for index, sec in enumerate(self.sections):
                    if sec.num_section_entries != len(sec.section_entries):
                        raise pycdlibexception.PyCdlibInvalidISO('El Torito section header specified %d entries, only saw %d' % (sec.num_section_entries, len(sec.section_entries)))
                    if index != last_section_index:
                        if sec.header_indicator != 0x90:
                            raise pycdlibexception.PyCdlibInvalidISO('Intermediate El Torito section header not properly specified')
                    # In theory, we should also make sure that the very last
                    # section has a header_indicator of 0x91.  However, we
                    # have seen ISOs in the wild (FreeBSD 11.0 amd64) in which
                    # this is not the case, so we skip that check.
                self._initialized = True
            elif val in (b'\x90', b'\x91'):
                # A Section Header Entry
                section_header = EltoritoSectionHeader()
                section_header.parse(valstr)
                self.sections.append(section_header)
            elif val in (b'\x88', b'\x00'):
                # A Section Entry. According to El Torito 2.4, a Section Entry
                # must follow a Section Header, but we have seen ISOs in the
                # wild that do not follow this (Mageia 4 ISOs, for instance).
                # To deal with this, we get a little complicated here.  If there
                # is a previous section header, and the length of the entries
                # attached to it is less than the number of entries it should
                # have, then we attach this entry to that header.  If there is
                # no previous section header, or if the previous section header
                # is already 'full', then we make this a standalone entry.
                secentry = EltoritoEntry()
                secentry.parse(valstr)
                if self.sections and len(self.sections[-1].section_entries) < self.sections[-1].num_section_entries:
                    self.sections[-1].add_parsed_entry(secentry)
                else:
                    self.standalone_entries.append(secentry)
            elif val == b'\x44':
                # A Section Entry Extension
                self.sections[-1].section_entries[-1].selection_criteria += valstr[2:]
            else:
                raise pycdlibexception.PyCdlibInvalidISO('Invalid El Torito Boot Catalog entry')

        return self._initialized

    def new(self, br, ino, sector_count, load_seg, media_name, system_type,
            platform_id, bootable):
        # type: (headervd.BootRecord, inode.Inode, int, int, str, int, int, bool) -> None
        """
        Create a new El Torito Boot Catalog.

        Parameters:
         br - The boot record that this El Torito Boot Catalog is associated with.
         ino - The Inode to associate with the initial entry.
         sector_count - The number of sectors for the initial entry.
         load_seg - The load segment address of the boot image.
         media_name - The name of the media type, one of 'noemul', 'floppy', or 'hdemul'.
         system_type - The partition type the entry should be.
         platform_id - The platform id to set in the validation entry.
         bootable - Whether this entry should be bootable.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('El Torito Boot Catalog already initialized')

        # Create the El Torito validation entry
        self.validation_entry.new(platform_id)

        self.initial_entry.new(sector_count, load_seg, media_name, system_type,
                               bootable)
        self.initial_entry.set_inode(ino)
        ino.linked_records.append((self.initial_entry, False))

        self.br = br

        self._initialized = True

    def add_section(self, ino, sector_count, load_seg, media_name, system_type,
                    efi, bootable):
        # type: (inode.Inode, int, int, str, int, bool, bool) -> None
        """
        Add an section header and entry to this Boot Catalog.

        Parameters:
         ino - The Inode object to associate with the new Entry.
         sector_count - The number of sectors to assign to the new Entry.
         load_seg - The load segment address of the boot image.
         media_name - The name of the media type, one of 'noemul', 'floppy', or 'hdemul'.
         system_type - The type of partition this entry should be.
         efi - Whether this section is an EFI section.
         bootable - Whether this entry should be bootable.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('El Torito Boot Catalog not initialized')

        # The Eltorito Boot Catalog can only be 2048 bytes (1 extent).  By
        # default, the first 64 bytes are used by the Validation Entry and the
        # Initial Entry, which leaves 1984 bytes.  Each section takes up 32
        # bytes for the Section Header and 32 bytes for the Section Entry, for
        # a total of 64 bytes, so we can have a maximum of 1984/64 = 31
        # sections.
        if len(self.sections) == 31:
            raise pycdlibexception.PyCdlibInvalidInput('Too many El Torito sections')

        sec = EltoritoSectionHeader()
        platform_id = self.validation_entry.platform_id
        if efi:
            platform_id = 0xef
        sec.new(b'\x00' * 28, platform_id)

        secentry = EltoritoEntry()
        secentry.new(sector_count, load_seg, media_name, system_type, bootable)
        secentry.set_inode(ino)
        ino.linked_records.append((secentry, False))

        sec.add_new_entry(secentry)

        if self.sections:
            self.sections[-1].set_record_not_last()

        self.sections.append(sec)

    def record(self):
        # type: () -> bytes
        """
        Generate a string representing this El Torito Boot Catalog.

        Parameters:
         None.
        Returns:
         A string representing this El Torito Boot Catalog.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('El Torito Boot Catalog not initialized')

        outlist = [self.validation_entry.record(), self.initial_entry.record()]

        for sec in self.sections:
            outlist.append(sec.record())

        for entry in self.standalone_entries:
            outlist.append(entry.record())

        return b''.join(outlist)

    def add_dirrecord(self, rec):
        # type: (Union[dr.DirectoryRecord, udfmod.UDFFileEntry]) -> None
        """
        Set the Directory Record associated with this Boot Catalog.

        Parameters:
         rec - The DirectoryRecord object to associate with this Boot Catalog.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('El Torito Boot Catalog not initialized')

        self.dirrecords.append(rec)

    def _extent_location(self):
        # type: () -> int
        """
        An internal method to get the extent location of this Boot Catalog.

        Parameters:
         None.
        Returns:
         The extent location of this Boot Catalog.
        """
        return struct.unpack_from('<L', self.br.boot_system_use[:4], 0)[0]

    def extent_location(self):
        # type: () -> int
        """
        Get the extent location of this El Torito Boot Catalog.

        Parameters:
         None.
        Returns:
         Integer extent location of this El Torito Boot Catalog.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('El Torito Boot Catalog not initialized')

        return self._extent_location()

    def update_catalog_extent(self, current_extent):
        # type: (int) -> None
        """
        Update the extent associated with this Boot Catalog.

        Parameters:
         current_extent - New extent to associate with this Boot Catalog
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('El Torito Boot Catalog not initialized')

        packed = struct.pack('<L', current_extent)

        ba = bytearray(self.br.boot_system_use)
        ba[0] = packed[0]
        ba[1] = packed[1]
        ba[2] = packed[2]
        ba[3] = packed[3]

        self.br.update_boot_system_use(bytes(ba))


def hdmbrcheck(disk_mbr, sector_count, bootable):
    # type: (bytes, int, bool) -> int
    """
    A function to sanity check an El Torito Hard Drive Master Boot Record (HDMBR).
    On success, it returns the system_type (also known as the partition type) that
    should be fed into the rest of the El Torito methods.  On failure, it raises
    an exception.

    Parameters:
     disk_mbr - The data to look in.
     sector_count - The number of sectors expected in the MBR.
     bootable - Whether this MBR is bootable.
    Returns:
     The system (or partition) type the should be fed into the rest of El Torito.
    """
    # The MBR that we want to see to do hd emulation boot for El Torito is a standard
    # x86 MBR, documented here:
    # https://en.wikipedia.org/wiki/Master_boot_record#Sector_layout
    #
    # In brief, it should consist of 512 bytes laid out like:
    # Offset 0x0 - 0x1BD:   Bootstrap code area
    # Offset 0x1BE - 0x1CD: Partition entry 1
    # Offset 0x1CE - 0x1DD: Partition entry 2
    # Offset 0x1DE - 0x1ED: Partition entry 3
    # Offset 0x1EE - 0x1FD: Partition entry 4
    # Offset 0x1FE:         0x55
    # Offset 0x1FF:         0xAA
    #
    # Each partition entry above should consist of:
    # Offset 0x0: Active (bit 7 set) or inactive (all zeros)
    # Offset 0x1 - 0x3: CHS address of first sector in partition
    #   Offset 0x1: Head
    #   Offset 0x2: Sector in bits 0-5, bits 6-7 are high bits of of cylinder
    #   Offset 0x3: bits 0-7 of cylinder
    # Offset 0x4: Partition type (almost all of these are valid, see https://en.wikipedia.org/wiki/Partition_type)
    # Offset 0x5 - 0x7: CHS address of last sector in partition (same format as first sector)
    # Offset 0x8 - 0xB: LBA of first sector in partition
    # Offset 0xC - 0xF: number of sectors in partition

    PARTITION_TYPE_UNUSED = 0x0

    PARTITION_STATUS_ACTIVE = 0x80

    (bootstrap_unused, part1, part2, part3, part4, keybyte1,
     keybyte2) = struct.unpack_from('=446s16s16s16s16sBB', disk_mbr, 0)

    if keybyte1 != 0x55 or keybyte2 != 0xAA:
        raise pycdlibexception.PyCdlibInvalidInput('Invalid magic on HD MBR')

    parts = [part1, part2, part3, part4]
    system_type = PARTITION_TYPE_UNUSED
    for part in parts:
        (status, s_head, s_seccyl, s_cyl, parttype, e_head, e_seccyl, e_cyl,
         lba_unused, num_sectors_unused) = struct.unpack('=BBBBBBBBLL', part)

        if parttype == PARTITION_TYPE_UNUSED:
            continue

        if system_type != PARTITION_TYPE_UNUSED:
            raise pycdlibexception.PyCdlibInvalidInput('Boot image has multiple partitions')

        if bootable and status != PARTITION_STATUS_ACTIVE:
            _logger.warning('Warning: partition not marked active')

        cyl = ((s_seccyl & 0xC0) << 10) | s_cyl
        sec = s_seccyl & 0x3f
        if cyl != 0 or s_head != 1 or sec != 1:
            _logger.warning('Warning: partition does not start at 0/1/1')

        cyl = ((e_seccyl & 0xC0) << 10) | e_cyl
        sec = e_seccyl & 0x3f
        geometry_sectors = (cyl + 1) * (e_head + 1) * sec

        if sector_count != geometry_sectors:
            _logger.warning('Warning: image size does not match geometry')

        system_type = parttype

    if system_type == PARTITION_TYPE_UNUSED:
        raise pycdlibexception.PyCdlibInvalidInput('Boot image has no partitions')

    return system_type
