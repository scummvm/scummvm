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

"""Implementation of ISO hybrid support."""

import random
import struct
import uuid

from pycdlib import pycdlibexception

# For mypy annotations
if False:  # pylint: disable=using-constant-test
    from typing import List, Optional, Tuple  # NOQA pylint: disable=unused-import

APM_PARTS = 3
GPT_SIZE = 128 // 4 + 2

crc32_table = (0, 0x77073096, 0xEE0E612C, 0x990951BA,
               0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
               0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
               0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
               0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
               0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
               0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
               0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
               0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
               0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
               0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
               0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
               0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
               0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
               0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
               0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
               0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
               0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
               0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
               0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
               0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
               0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
               0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
               0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
               0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
               0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
               0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
               0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
               0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
               0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
               0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
               0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
               0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
               0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
               0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
               0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
               0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
               0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
               0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
               0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
               0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
               0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
               0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
               0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
               0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
               0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
               0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
               0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
               0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
               0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
               0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
               0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
               0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
               0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
               0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
               0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
               0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
               0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
               0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
               0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
               0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
               0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
               0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
               0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D)


def crc32(data):
    # type: (bytes) -> int
    """
    Calculate the CRC32 over a range of bytes.

    Parameters:
     data - The array of bytes to calculate the CRC32 over.
    Returns:
     The CRC32 of the data.
    """
    crc = 0xFFFFFFFF
    for x in data:
        crc = ((crc >> 8) & 0x00FFFFFF) ^ crc32_table[(crc ^ x) & 0xFF]

    return crc ^ 0xffffffff


class APMPartHeader:
    """A class that represents an APM (Apple Partition Map) Partition Header."""
    __slots__ = ('_initialized', 'map_count', 'start_block', 'block_count',
                 'name', 'type_desc', 'data_start', 'data_count', 'status',
                 'boot_start', 'boot_count', 'boot_load', 'boot_load2',
                 'boot_entry', 'boot_entry2', 'boot_cksum', 'processor',
                 'driver_sig')

    FMT = '>HHLLL32s32sLLLLLLLLLL16sL372s'

    MAC_PARTITION_MAGIC = 0x504d

    def __init__(self):
        # type: () -> None
        self._initialized = False

    def parse(self, instr):
        # type: (bytes) -> None
        """
        Parse an APM Partition Header out of existing data.

        Parameters:
         instr - The data to parse.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This APMPartHeader object is already initialized')

        (sig, resv_unused, self.map_count, self.start_block, self.block_count,
         name, type_desc, self.data_start, self.data_count,
         self.status, self.boot_start, self.boot_count, self.boot_load,
         self.boot_load2, self.boot_entry, self.boot_entry2, self.boot_cksum,
         self.processor, self.driver_sig,
         padding_unused) = struct.unpack_from(self.FMT, instr, 0)

        if sig != self.MAC_PARTITION_MAGIC:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid APM signature')

        self.name = name.decode('ascii').rstrip('\x00')
        self.type_desc = type_desc.decode('ascii').rstrip('\x00')

        self._initialized = True

    def new(self, name, type_desc, status):
        # type: (str, str, int) -> None
        """
        Create a new APM Partition Header.

        Parameters:
         name - The name for this partition map.
         type_desc - The type of this partition map.
         status - The status for this partition map.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This APMPartHeader object is already initialized')

        self.map_count = 3
        self.start_block = 0  # this will get set later
        self.block_count = 0  # this will get set later
        self.name = name
        self.type_desc = type_desc
        self.data_start = 0
        self.data_count = 0  # this will get set later
        self.status = status
        self.boot_start = 0
        self.boot_count = 0
        self.boot_load = 0
        self.boot_load2 = 0
        self.boot_entry = 0
        self.boot_entry2 = 0
        self.boot_cksum = 0
        self.processor = b''
        self.driver_sig = 0

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate a string representing this APM Partition Header.

        Parameters:
         None.
        Returns:
         A bytestring representing this APM Partition Header.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This APMPartHeader object is not initialized')

        return struct.pack(self.FMT, self.MAC_PARTITION_MAGIC, 0,
                           self.map_count, self.start_block, self.block_count,
                           self.name.encode('ascii'),
                           self.type_desc.encode('ascii'), self.data_start,
                           self.data_count, self.status, self.boot_start,
                           self.boot_count, self.boot_load, self.boot_load2,
                           self.boot_entry, self.boot_entry2, self.boot_cksum,
                           self.processor, self.driver_sig, b'\x00' * 372)


class GPTPartHeader:
    """A class that represents a GPT Partition Header."""
    __slots__ = ('_initialized', 'part_guid', 'part_type_guid', 'first_lba',
                 'last_lba', 'attributes', 'name')

    FMT = '<16s16sQQ8s72s'

    BASIC_PARTITION = b'\xa2\xa0\xd0\xeb\xe5\xb9\x33\x44\x87\xc0\x68\xb6\xb7\x26\x99\xc7'
    HFS_PARTITION = b'\x00\x53\x46\x48\x00\x00\xaa\x11\xaa\x11\x00\x30\x65\x43\xec\xac'

    def __init__(self):
        # type: () -> None
        self._initialized = False

    def parse(self, instr):
        # type: (bytes) -> int
        """
        Parse a GPT Partition Header out of existing data.

        Parameters:
         instr - The data to parse.
        Returns:
         The number of bytes consumed.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This GPTPartHeader object is already initialized')

        (self.part_type_guid, part_guid, self.first_lba, self.last_lba,
         self.attributes, name) = struct.unpack_from(self.FMT, instr[:struct.calcsize(self.FMT)], 0)

        if self.part_type_guid not in (self.BASIC_PARTITION, self.HFS_PARTITION):
            raise pycdlibexception.PyCdlibInvalidISO('Invalid Partition Type UUID')

        self.name = name.decode('utf-16_le').rstrip('\x00')

        self.part_guid = uuid.UUID(bytes=part_guid)

        self._initialized = True

        return struct.calcsize(self.FMT)

    def new(self, is_basic, name):
        # type: (bool, str) -> None
        """
        Create a new GPT Partition Header.

        Parameters:
         is_basic - Whether this is a basic GPTPartHeader (True), or an HFS
                    one (False).
         name - Bytestring containing the name to store.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This GPTPartHeader object is already initialized')

        if is_basic:
            self.part_type_guid = self.BASIC_PARTITION
        else:
            self.part_type_guid = self.HFS_PARTITION

        self.part_guid = uuid.uuid4()
        self.first_lba = 0
        self.last_lba = 0  # this will be set later
        self.attributes = b'\x00' * 8
        self.name = name

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Generate a string representing this GPT Partition Header.

        Parameters:
         None.
        Returns:
         A bytestring representing this GPT Partition Header.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This GPTPartHeader object is not initialized')

        return struct.pack(self.FMT, self.part_type_guid, self.part_guid.bytes,
                           self.first_lba, self.last_lba, self.attributes,
                           self.name.encode('utf-16_le'))


class GPTHeader:
    """A class that represents a GPT Header."""
    __slots__ = ('_initialized', 'current_lba', 'backup_lba', 'first_usable_lba',
                 'last_usable_lba', 'disk_guid', 'partition_entries_lba',
                 'num_parts', 'size_of_partition_entries')

    FMT = '<8s4s4sLLQQQQ16sQLLL420s'
    GPT_SIG = b'\x45\x46\x49\x20\x50\x41\x52\x54'
    GPT_REV = b'\x00\x00\x01\x00'
    GPT_HEADER_SIZE = b'\x5c\x00\x00\x00'

    def __init__(self):
        # type: () -> None
        self._initialized = False

    def parse(self, instr):
        # type: (bytes) -> int
        """
        Parse a GPT Header out of existing data.

        Parameters:
         instr - The data to parse.
        Returns:
         The number of bytes consumed.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This GPTHeader object is already initialized')

        (sig, revision, header_size, header_crc_unused, resv1_unused,
         self.current_lba, self.backup_lba, self.first_usable_lba,
         self.last_usable_lba, disk_guid, self.partition_entries_lba,
         self.num_parts, self.size_of_partition_entries,
         partition_entries_crc_unused,
         resv2_unused) = struct.unpack_from(self.FMT, instr[:struct.calcsize(self.FMT)], 0)

        if sig != self.GPT_SIG:
            raise pycdlibexception.PyCdlibInvalidISO('Failed to find GPT signature while parsing GPT Header')

        if revision != self.GPT_REV:
            raise pycdlibexception.PyCdlibInvalidISO('Failed to find GPT revision while parsing GPT Header')

        if header_size != self.GPT_HEADER_SIZE:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid GPT Header size while parsing GPT Header')

        self.disk_guid = uuid.UUID(bytes=disk_guid)

        self._initialized = True

        return struct.calcsize(self.FMT)

    def new(self, mac):
        # type: (bool) -> None
        """
        Create a new GPT Header.

        Parameters:
         mac - Whether this GPT has Mac support.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This GPTHeader object is already initialized')

        self.current_lba = 0  # this will be set later
        self.backup_lba = 0  # this will be set laster
        gpt_size = GPT_SIZE
        if mac:
            gpt_size += (APM_PARTS * 4) + 2
        self.first_usable_lba = gpt_size
        self.last_usable_lba = 0  # this will be set later
        self.disk_guid = uuid.uuid4()
        self.partition_entries_lba = 0  # this will be set later
        self.num_parts = 128
        self.size_of_partition_entries = 128

        self._initialized = True

    def set_lbas(self, current, backup):
        # type: (int, int) -> None
        """
        Set the current and backup LBAs for this GPT Header.

        Parameters:
         current - The current LBA to set for this GPT Header.
         backup - The backup LBA to set for this GPT Header.
        Returns:
         Nothing
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This GPTHeader object is not initialized')

        self.current_lba = current
        self.backup_lba = backup

    def set_last_usable_lba(self, iso_size_and_padding):
        # type: (int) -> None
        """
        Set the last usable LBA for this GPT Header.

        Parameters:
         iso_size_and_padding - The size of the ISO with padding.
        Returns:
         Nothing
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This GPTHeader object is not initialized')

        self.last_usable_lba = iso_size_and_padding // 512 - GPT_SIZE

    def record(self, part_entries_crc):
        # type: (int) -> bytes
        """
        Generate a string representing this GPT Header.

        Parameters:
         None.
        Returns:
         A bytestring representing this GPT Header.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This GPTHeader object is not initialized')

        rec = struct.pack(self.FMT, self.GPT_SIG, self.GPT_REV,
                          self.GPT_HEADER_SIZE, 0, 0,
                          self.current_lba, self.backup_lba,
                          self.first_usable_lba, self.last_usable_lba,
                          self.disk_guid.bytes, self.partition_entries_lba,
                          self.num_parts, self.size_of_partition_entries,
                          part_entries_crc, b'\x00' * 420)

        header_crc = crc32(rec[:92])
        header_packed = struct.pack('<L', header_crc)

        ba = bytearray(rec)
        ba[16] = header_packed[0]
        ba[17] = header_packed[1]
        ba[18] = header_packed[2]
        ba[19] = header_packed[3]

        return bytes(ba)


class GPT:
    """
    A class representing one GPT on an ISO.  There are generally two; the
    primary one at the beginning (in the system area), and secondary one at the
    very end.
    """
    __slots__ = ('_initialized', 'is_primary', 'header', 'parts', 'apm_parts')

    def __init__(self, is_primary):
        # type: (bool) -> None
        self.is_primary = is_primary
        self.header = GPTHeader()
        self.parts = []  # type: List[GPTPartHeader]
        self.apm_parts = []  # type: List[APMPartHeader]
        self._initialized = False

    def parse_primary(self, instr, mac):
        # type: (bytes, bool) -> None
        """
        Parse a primary GPT.

        Parameters:
         instr - The string containing the data to parse.
         mac - Whether this GPT contains mac data as well.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This GPT object is already initialized')

        if not self.is_primary:
            raise pycdlibexception.PyCdlibInternalError('Cannot parse primary with a secondary GPT')

        offset = 512
        offset += self.header.parse(instr[offset:])
        if mac:
            # Now go looking for the APMs.  Note that we've seen ISOs in the
            # wild (Fedora 34 Workstation DVD) with Apple partition entries,
            # but no APM parts.  If we see all zeros where we expect the
            # first APM part, we skip the parsing here.
            offset = 2048
            if instr[offset:offset + 2] != b'\x00\x00':
                for i_unused in range(0, APM_PARTS):
                    apm_part = APMPartHeader()
                    apm_part.parse(instr[offset:])
                    self.apm_parts.append(apm_part)
                    offset += 2048

        offset = self.header.partition_entries_lba * 512
        for i_unused in range(0, self.header.num_parts):
            # Some GPT implementations have large numbers of "empty"
            # partition headers (like syslinux).  We could store a slew
            # of these empty partition headers, but that's just a waste
            # of memory.  Instead we peek ahead and once we see one of
            # these, we assume we are done parsing.
            if instr[offset:offset + 2] == b'\x00\x00':
                break
            part = GPTPartHeader()
            offset += part.parse(instr[offset:])
            self.parts.append(part)

        self._initialized = True

    def parse_secondary_header(self, instr):
        # type: (bytes) -> None
        """
        Parse a secondary GPT Header.

        Parameters:
         instr - The string containing the data to parse.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This GPT object is already initialized')

        if self.is_primary:
            raise pycdlibexception.PyCdlibInternalError('Cannot parse secondary header with a primary GPT')

        self.header.parse(instr)

    def parse_secondary_partitions(self, instr):
        # type: (bytes) -> None
        """
        Parse a secondary GPT set of partitions.

        Parameters:
         instr - The string containing the data to parse.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This GPT object is already initialized')

        offset = 0
        for i_unused in range(0, self.header.num_parts):
            # Some GPT implementations have large numbers of "empty"
            # partition headers (like syslinux).  We could store a slew
            # of these empty partition headers, but that's just a waste
            # of memory.  Instead we peek ahead and once we see one of
            # these, we assume we are done parsing.
            if instr[offset:offset + 2] == b'\x00\x00':
                break
            part = GPTPartHeader()
            offset += part.parse(instr[offset:])
            self.parts.append(part)

        self._initialized = True

    def new(self, mac):
        # type: (bool) -> None
        """
        Create a new GPT.

        Parameters:
         mac - Whether this is a GPT for a Macintosh boot.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This GPT object is already initialized')

        self.header.new(mac)
        if self.is_primary:
            hole = 0
            if mac:
                hole = (APM_PARTS * 4) + 2
            self.header.partition_entries_lba = 2 + hole

        part1 = GPTPartHeader()
        part1.new(True, 'ISOHybrid ISO')
        self.parts.append(part1)

        part2 = GPTPartHeader()
        part2.new(True, 'ISOHybrid')
        self.parts.append(part2)

        if mac:
            part3 = GPTPartHeader()
            part3.new(False, 'ISOHybrid')
            self.parts.append(part3)

            if self.is_primary:
                apm = APMPartHeader()
                apm.new('Apple', 'Apple_partition_map', 0x3)
                self.apm_parts.append(apm)

                apm2 = APMPartHeader()
                apm2.new('EFI', 'Apple_HFS', 0x33)
                self.apm_parts.append(apm2)

                apm3 = APMPartHeader()
                apm3.new('EFI', 'Apple_HFS', 0x33)
                self.apm_parts.append(apm3)

        self._initialized = True

    def record(self):
        # type: () -> bytes
        """
        Create a string representing this GPT.

        Parameters:
         None.
        Returns:
         A bytestring representing this GPT.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This GPT object is not initialized')

        # The partitions really go later, but since we need the partition
        # string so we can calculate the CRC for the header, we do it all.
        tmplist = []
        for part in self.parts:
            tmplist.append(part.record())
        part_data = b''.join(tmplist)

        if self.is_primary:
            outlist = [self.header.record(crc32(part_data))]
            if self.apm_parts:
                outlist.append(b'\x00' * 1024)
            for apm_part in self.apm_parts:
                raw = apm_part.record()
                pad = b'\x00' * (2048 - len(raw))
                outlist.extend([raw, pad])
            outlist.append(part_data)
            # Write out all of the "empty" partitions.
            outlist.append(b'\x00' * (self.header.num_parts - len(self.parts)) * 128)
        else:
            outlist = [part_data]
            # Write out all of the "empty" partitions.
            outlist.append(b'\x00' * (self.header.num_parts - len(self.parts)) * 128)
            outlist.append(self.header.record(crc32(part_data)))

        return b''.join(outlist)


class IsoHybrid:
    """
    A class that represents an ISO hybrid; that is, an ISO that can be booted
    via CD or via an alternate boot mechanism (such as USB).
    """
    __slots__ = ('_initialized', 'header', 'mbr', 'rba', 'mbr_id', 'part_entry',
                 'bhead', 'bsect', 'bcyle', 'ptype', 'ehead', 'part_offset',
                 'geometry_heads', 'geometry_sectors', 'efi', 'efi_lba',
                 'efi_count', 'mac', 'mac_lba', 'mac_count', 'primary_gpt',
                 'secondary_gpt')

    FMT = '<400sLLLH'
    ORIG_HEADER = b'\x33\xed' + b'\x90' * 30
    MAC_AFP = b'\x45\x52\x08\x00\x00\x00\x90\x90' + b'\x00' * 24
    EFI_HEADER = b'\x00\xfe\xff\xff\xef\xfe\xff\xff'
    MAC_HEADER = b'\x00\xfe\xff\xff\x00\xfe\xff\xff'

    def __init__(self):
        # type: () -> None
        self.part_entry = -1
        self.efi = False
        self.mac = False
        self.primary_gpt = GPT(True)
        self.secondary_gpt = GPT(False)
        self._initialized = False

    def parse(self, instr):
        # type: (bytes) -> bool
        """
        Parse ISO hybridization info out of an existing ISO.

        Parameters:
         instr - The data for the ISO hybridization.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This IsoHybrid object is already initialized')

        if len(instr) < 512:
            raise pycdlibexception.PyCdlibInternalError('Invalid IsoHybrid MBR passed')

        if instr[0:32] == self.ORIG_HEADER:
            self.header = self.ORIG_HEADER
        elif instr[0:32] == self.MAC_AFP:
            self.header = self.MAC_AFP
        else:
            # If we didn't see anything that we expected, then this is not an
            # IsoHybrid ISO, so just quietly return False
            return False

        (self.mbr, self.rba, unused1, self.mbr_id,
         unused2) = struct.unpack_from(self.FMT, instr[:32 + struct.calcsize(self.FMT)], 32)

        if unused1 != 0:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid IsoHybrid unused1')

        if unused2 != 0:
            raise pycdlibexception.PyCdlibInvalidISO('Invalid IsoHybrid unused2')

        psize = 0
        ecyle = 0
        offset = 32 + struct.calcsize(self.FMT)
        for i in range(1, 5):
            if bytes(bytearray([instr[offset]])) == b'\x80':
                self.part_entry = i
                (const_unused, self.bhead, self.bsect, self.bcyle, self.ptype,
                 self.ehead, esect_unused, ecyle, self.part_offset,
                 psize) = struct.unpack_from('<BBBBBBBBLL', instr[:offset + 16], offset)
            if i == 2 and instr[offset:offset + 8] == self.EFI_HEADER:
                self.efi = True
                (efi_lba, self.efi_count) = struct.unpack_from('<LL', instr[:offset + 16], offset + 8)
                self.efi_lba = efi_lba // 4
            if i == 3 and instr[offset:offset + 8] == self.MAC_HEADER:
                self.mac = True
                (mac_lba, self.mac_count) = struct.unpack_from('<LL', instr[:offset + 16], offset + 8)
                self.mac_lba = mac_lba // 4

            offset += 16

        if self.part_entry < 0:
            raise pycdlibexception.PyCdlibInvalidISO('No valid partition found in IsoHybrid!')

        if instr[510:512] != b'\x55\xaa':
            raise pycdlibexception.PyCdlibInvalidISO('Invalid tail on isohybrid section')

        self.geometry_heads = self.ehead + 1

        self.geometry_sectors = min(psize // ((ecyle + 1) * self.geometry_heads), 63)

        if self.efi:
            self.primary_gpt.parse_primary(instr, self.mac)

        self._initialized = True

        return True

    def parse_secondary_gpt_header(self, instr):
        # type: (bytes) -> None
        """
        Parse the secondary GPT Header.

        Parameters:
         instr -  The data to parse for the GPT Header.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This IsoHybrid object is not initialized')

        self.secondary_gpt.parse_secondary_header(instr)

    def parse_secondary_gpt_partitions(self, instr):
        # type: (bytes) -> None
        """
        Parse the secondary GPT Partitions.

        Parameters:
         instr - The data to parse for the partitions.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This IsoHybrid object is not initialized')

        self.secondary_gpt.parse_secondary_partitions(instr)

    def new(self, efi, mac, part_entry, mbr_id, part_offset,
            geometry_sectors, geometry_heads, part_type):
        # type: (bool, bool, int, Optional[int], int, int, int, int) -> None
        """
        Add ISO hybridization to an ISO.

        Parameters:
         efi - Whether this ISO should be setup for EFI boot.
         mac - Whether this ISO should be made bootable for the Macintosh.
         part_entry - The partition entry for the hybridization.
         mbr_id - The mbr_id to use for the hybridization.
         part_offset - The partition offset to use for the hybridization.
         geometry_sectors - The number of sectors to use for the hybridization.
         geometry_heads - The number of heads to use for the hybridization.
         part_type - The partition type for the hybridization.
        Returns:
         Nothing.
        """
        if self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This IsoHybrid object is already initialized')

        if geometry_sectors < 1 or geometry_sectors > 63:
            raise pycdlibexception.PyCdlibInvalidInput('Geometry sectors can only be between 1 and 63, inclusive')

        if geometry_heads < 1 or geometry_heads > 256:
            raise pycdlibexception.PyCdlibInvalidInput('Geometry heads can only be between 1 and 256, inclusive')

        if mac and part_type != 0:
            raise pycdlibexception.PyCdlibInvalidInput('When generating for Mac, partition type must be 0')

        isohybrid_data_hd0 = b'\x33\xed\xfa\x8e\xd5\xbc\x00\x7c\xfb\xfc\x66\x31\xdb\x66\x31\xc9\x66\x53\x66\x51\x06\x57\x8e\xdd\x8e\xc5\x52\xbe\x00\x7c\xbf\x00\x06\xb9\x00\x01\xf3\xa5\xea\x4b\x06\x00\x00\x52\xb4\x41\xbb\xaa\x55\x31\xc9\x30\xf6\xf9\xcd\x13\x72\x16\x81\xfb\x55\xaa\x75\x10\x83\xe1\x01\x74\x0b\x66\xc7\x06\xf1\x06\xb4\x42\xeb\x15\xeb\x00\x5a\x51\xb4\x08\xcd\x13\x83\xe1\x3f\x5b\x51\x0f\xb6\xc6\x40\x50\xf7\xe1\x53\x52\x50\xbb\x00\x7c\xb9\x04\x00\x66\xa1\xb0\x07\xe8\x44\x00\x0f\x82\x80\x00\x66\x40\x80\xc7\x02\xe2\xf2\x66\x81\x3e\x40\x7c\xfb\xc0\x78\x70\x75\x09\xfa\xbc\xec\x7b\xea\x44\x7c\x00\x00\xe8\x83\x00\x69\x73\x6f\x6c\x69\x6e\x75\x78\x2e\x62\x69\x6e\x20\x6d\x69\x73\x73\x69\x6e\x67\x20\x6f\x72\x20\x63\x6f\x72\x72\x75\x70\x74\x2e\x0d\x0a\x66\x60\x66\x31\xd2\x66\x03\x06\xf8\x7b\x66\x13\x16\xfc\x7b\x66\x52\x66\x50\x06\x53\x6a\x01\x6a\x10\x89\xe6\x66\xf7\x36\xe8\x7b\xc0\xe4\x06\x88\xe1\x88\xc5\x92\xf6\x36\xee\x7b\x88\xc6\x08\xe1\x41\xb8\x01\x02\x8a\x16\xf2\x7b\xcd\x13\x8d\x64\x10\x66\x61\xc3\xe8\x1e\x00\x4f\x70\x65\x72\x61\x74\x69\x6e\x67\x20\x73\x79\x73\x74\x65\x6d\x20\x6c\x6f\x61\x64\x20\x65\x72\x72\x6f\x72\x2e\x0d\x0a\x5e\xac\xb4\x0e\x8a\x3e\x62\x04\xb3\x07\xcd\x10\x3c\x0a\x75\xf1\xcd\x18\xf4\xeb\xfd\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'  # pylint: disable=line-too-long

        self.mbr = isohybrid_data_hd0
        self.rba = 0  # This will be set later
        self.mbr_id = mbr_id
        if self.mbr_id is None:
            self.mbr_id = random.getrandbits(32)

        self.part_entry = part_entry
        self.bhead = (part_offset // geometry_sectors) % geometry_heads
        self.bsect = (part_offset % geometry_sectors) + 1
        self.bcyle = part_offset // (geometry_heads * geometry_sectors)
        self.bsect += (self.bcyle & 0x300) >> 2
        self.bcyle &= 0xff
        self.ptype = part_type
        self.ehead = geometry_heads - 1
        self.part_offset = part_offset
        self.geometry_heads = geometry_heads
        self.geometry_sectors = geometry_sectors

        self.mac = mac
        if self.mac:
            self.header = self.MAC_AFP
            self.mac_lba = 0  # this will be set later
            self.mac_count = 0  # this will be set later
        else:
            self.header = self.ORIG_HEADER

        self.efi = efi
        if self.efi:
            self.efi_lba = 0  # this will be set later
            self.efi_count = 0  # this will be set later
            self.primary_gpt.new(self.mac)
            self.secondary_gpt.new(self.mac)

        self._initialized = True

    def _calc_cc(self, iso_size):
        # type: (int) -> Tuple[int, int]
        """
        Calculate the 'cc' and the 'padding' values for this hybridization.

        Parameters:
         iso_size - The size of the ISO, excluding the hybridization.
        Returns:
         A tuple containing the cc value and the padding.
        """
        cylsize = self.geometry_heads * self.geometry_sectors * 512
        frac = iso_size % cylsize
        padding = 0
        if frac > 0:
            padding = cylsize - frac
        cc = min((iso_size + padding) // cylsize, 1024)

        return (cc, padding)

    def record(self, iso_size):
        # type: (int) -> bytes
        """
        Generate a string containing the ISO hybridization.

        Parameters:
         iso_size - The size of the ISO, excluding the hybridization.
        Returns:
         A string containing the ISO hybridization.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This IsoHybrid object is not initialized')

        outlist = [struct.pack('<32s400sLLLH', self.header, self.mbr, self.rba,
                               0, self.mbr_id, 0)]

        for i in range(1, 5):
            raw = b'\x00' * 16
            if i == self.part_entry:
                cc = self._calc_cc(iso_size)[0]
                esect = self.geometry_sectors + (((cc - 1) & 0x300) >> 2)
                ecyle = (cc - 1) & 0xff
                psize = cc * self.geometry_heads * self.geometry_sectors - self.part_offset
                raw = struct.pack('<BBBBBBBBLL', 0x80, self.bhead, self.bsect,
                                  self.bcyle, self.ptype, self.ehead, esect,
                                  ecyle, self.part_offset, psize)
            if i == 2 and self.efi:
                raw = self.EFI_HEADER
                raw += struct.pack('<LL', self.efi_lba * 4, self.efi_count)
            if i == 3 and self.mac:
                raw = self.MAC_HEADER
                raw += struct.pack('<LL', self.mac_lba * 4, self.mac_count)

            outlist.append(raw)

        outlist.append(b'\x55\xaa')
        if self.efi:
            outlist.append(self.primary_gpt.record())

        return b''.join(outlist)

    def record_padding(self, iso_size):
        # type: (int) -> bytes
        """
        Record padding for the ISO hybridization.

        Parameters:
         iso_size - The size of the ISO, excluding the hybridization.
        Returns:
         A string of zeros the right size to pad the ISO.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This IsoHybrid object is not initialized')

        padlen = self._calc_cc(iso_size)[1]

        return b'\x00' * padlen

    def update_rba(self, current_extent):
        # type: (int) -> None
        """
        Update the current rba for the ISO hybridization.

        Parameters:
         current_extent - The new extent to set the RBA to.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This IsoHybrid object is not initialized')

        self.rba = current_extent * 4  # Plain old ISOLINUX expects LBA * 4 too, apparently

    def update_efi(self, current_extent, sector_count, iso_size):
        # type: (int, int, int) -> None
        """
        Update the current EFI lba for the ISO hybridization.

        Parameters:
         current_extent - The new extent to set the RBA to.
         sector_count - The number of sectors for the EFI entry.
         iso_size - The full size of the ISO.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This IsoHybrid object is not initialized')

        if not self.efi:
            raise pycdlibexception.PyCdlibInternalError('Attempted to set EFI lba on a non-EFI ISO')

        self.efi_lba = current_extent
        self.efi_count = sector_count

        padlen = self._calc_cc(iso_size)[1]
        size_and_padlen = iso_size + padlen
        secondary_lba = (size_and_padlen - 512) // 512

        self.primary_gpt.header.set_lbas(1, secondary_lba)
        self.primary_gpt.header.set_last_usable_lba(size_and_padlen)
        self.primary_gpt.parts[0].last_lba = (iso_size // 512) - 1
        self.primary_gpt.parts[1].first_lba = current_extent * 4
        self.primary_gpt.parts[1].last_lba = (current_extent * 4) + sector_count - 1

        self.secondary_gpt.header.partition_entries_lba = secondary_lba - (128 // 4)
        self.secondary_gpt.header.set_lbas(secondary_lba, 1)
        self.secondary_gpt.header.set_last_usable_lba(size_and_padlen)
        self.secondary_gpt.parts[0].last_lba = (iso_size // 512) - 1
        self.secondary_gpt.parts[1].first_lba = current_extent * 4
        self.secondary_gpt.parts[1].last_lba = (current_extent * 4) + sector_count - 1

    def update_mac(self, current_extent, sector_count):
        # type: (int, int) -> None
        """
        Update the current MAC lba for the ISO hybridization.

        Parameters:
         current_extent - The new extent to set the RBA to.
         sector_count - The number of sectors for the MAC entry.
        Returns:
         Nothing.
        """
        if not self._initialized:
            raise pycdlibexception.PyCdlibInternalError('This IsoHybrid object is not initialized')

        if not self.mac:
            raise pycdlibexception.PyCdlibInternalError('Attempted to set Mac lba on a non-Mac ISO')

        self.mac_lba = current_extent
        self.mac_count = sector_count

        self.primary_gpt.parts[2].first_lba = current_extent * 4
        self.primary_gpt.parts[2].last_lba = (current_extent * 4) + sector_count - 1
