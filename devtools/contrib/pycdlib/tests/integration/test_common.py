# -*- coding: utf-8 -*-

import io
import os
import sys
import struct

import pytest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))

import pycdlib

# Technically, Rock Ridge doesn't impose a length limitation on NM (alternate
# name) or SL (symlinks).  However, in practice, the Linux kernel (at least
# ext4) doesn't support any names longer than 255, and the ISO driver doesn't
# support any names longer than 248.  Thus we stick to 248 for our tests.
RR_MAX_FILENAME_LENGTH = 248

def find_executable(executable):
    paths = os.environ['PATH'].split(os.pathsep)

    if os.path.isfile(executable):
        return executable
    else:
        for p in paths:
            f = os.path.join(p, executable)
            if os.path.isfile(f):
                return f
    return None

################################ INTERNAL HELPERS #############################

def internal_check_pvd(pvd, extent, size, ptbl_size, ptbl_location_le, ptbl_location_be):
    # The length of the system identifer should always be 32.
    assert(len(pvd.system_identifier) == 32)
    # The length of the volume identifer should always be 32.
    assert(len(pvd.volume_identifier) == 32)
    # The amount of space the ISO takes depends on the files and directories
    # on the ISO.
    assert(pvd.space_size == size)
    # The set size should always be one for these tests.
    assert(pvd.set_size == 1)
    # genisoimage only supports setting the sequence number to 1
    assert(pvd.seqnum == 1)
    # genisoimage always produces ISOs with 2048-byte sized logical blocks.
    assert(pvd.log_block_size == 2048)
    # The path table size depends on how many directories there are on the ISO.
    assert(pvd.path_tbl_size == ptbl_size)
    # The little endian version of the path table should start at the location
    # passed in (this changes based on how many volume descriptors there are,
    # e.g. Joliet).
    assert(pvd.path_table_location_le == ptbl_location_le)
    # The optional path table location should always be zero.
    assert(pvd.optional_path_table_location_le == 0)
    # The big endian version of the path table changes depending on how many
    # directories there are on the ISO.
    assert(pvd.path_table_location_be == ptbl_location_be)
    # The optional path table location should always be zero.
    assert(pvd.optional_path_table_location_be == 0)
    # The length of the volume set identifer should always be 128.
    assert(len(pvd.volume_set_identifier) == 128)
    # The volume set identifier is always blank here.
    assert(pvd.volume_set_identifier == b' '*128)
    # The publisher identifier text should be blank.
    assert(pvd.publisher_identifier.text == b' '*128)
    # The preparer identifier text should be blank.
    assert(pvd.preparer_identifier.text == b' '*128)
    # The copyright file identifier should be blank.
    assert(pvd.copyright_file_identifier == b' '*37)
    # The abstract file identifier should be blank.
    assert(pvd.abstract_file_identifier == b' '*37)
    # The bibliographic file identifier should be blank.
    assert(pvd.bibliographic_file_identifier == b' '*37)
    # The primary volume descriptor should always have a file structure version
    # of 1.
    assert(pvd.file_structure_version == 1)
    # The length of the application use string should always be 512.
    assert(len(pvd.application_use) == 512)
    # The PVD should be where we want it.
    assert(pvd.extent_location() == extent)

def internal_check_enhanced_vd(en_vd, size, ptbl_size, ptbl_location_le,
                               ptbl_location_be):
    assert(en_vd.version == 2)
    assert(en_vd.flags == 0)
    # The length of the system identifer should always be 32.
    assert(len(en_vd.system_identifier) == 32)
    # The length of the volume identifer should always be 32.
    assert(len(en_vd.volume_identifier) == 32)
    # The amount of space the ISO takes depends on the files and directories
    # on the ISO.
    assert(en_vd.space_size == size)
    assert(en_vd.escape_sequences == b'\x00'*32)
    assert(en_vd.set_size == 1)
    assert(en_vd.seqnum == 1)
    assert(en_vd.log_block_size == 2048)
    assert(en_vd.path_tbl_size == ptbl_size)
    # The little endian version of the path table should start at the location
    # passed in (this changes based on how many volume descriptors there are,
    # e.g. Joliet).
    assert(en_vd.path_table_location_le == ptbl_location_le)
    # The optional path table location should always be zero.
    assert(en_vd.optional_path_table_location_le == 0)
    # The big endian version of the path table changes depending on how many
    # directories there are on the ISO.
    assert(en_vd.path_table_location_be == ptbl_location_be)
    # The optional path table location should always be zero.
    assert(en_vd.optional_path_table_location_be == 0)
    # The length of the volume set identifer should always be 128.
    assert(len(en_vd.volume_set_identifier) == 128)
    # The volume set identifier is always blank here.
    assert(en_vd.volume_set_identifier == b' '*128)
    # The publisher identifier text should be blank.
    assert(en_vd.publisher_identifier.text == b' '*128)
    # The preparer identifier text should be blank.
    assert(en_vd.preparer_identifier.text == b' '*128)
    # The copyright file identifier should be blank.
    assert(en_vd.copyright_file_identifier == b' '*37)
    # The abstract file identifier should be blank.
    assert(en_vd.abstract_file_identifier == b' '*37)
    # The bibliographic file identifier should be blank.
    assert(en_vd.bibliographic_file_identifier == b' '*37)
    # The primary volume descriptor should always have a file structure version
    # of 1.
    assert(en_vd.file_structure_version == 2)

def internal_check_eltorito(iso, boot_catalog_extent, load_rba, media_type,
                            system_type, bootable, platform_id):
    # Now check the Eltorito Boot Record.

    # We support only one boot record for now.
    assert(len(iso.brs) == 1)
    eltorito = iso.brs[0]
    # The boot_system_identifier for El Torito should always be a space-padded
    # version of "EL TORITO SPECIFICATION".
    assert(eltorito.boot_system_identifier == b'EL TORITO SPECIFICATION'.ljust(32, b'\x00'))
    # The boot identifier should always be 32 zeros.
    assert(eltorito.boot_identifier == b'\x00'*32)
    # The boot_system_use field should always contain the boot catalog extent
    # encoded as a string.
    assert(eltorito.boot_system_use[:4] == struct.pack('<L', boot_catalog_extent))
    # The boot catalog validation entry should have a platform id of 0.
    assert(iso.eltorito_boot_catalog.validation_entry.platform_id == platform_id)
    # The boot catalog validation entry should have an id string of all zeros.
    assert(iso.eltorito_boot_catalog.validation_entry.id_string == b'\x00'*24)
    if platform_id == 0xef:
        # UEFI
        assert(iso.eltorito_boot_catalog.validation_entry.checksum == 0x66aa)
    else:
        # The boot catalog validation entry should have a checksum of 0x55aa.
        assert(iso.eltorito_boot_catalog.validation_entry.checksum == 0x55aa)

    # The boot catalog initial entry should have a boot indicator of 0x88.
    if bootable:
        assert(iso.eltorito_boot_catalog.initial_entry.boot_indicator == 0x88)
    else:
        assert(iso.eltorito_boot_catalog.initial_entry.boot_indicator == 0)
    # The boot catalog initial entry should have a boot media type of 0.
    assert(iso.eltorito_boot_catalog.initial_entry.boot_media_type == media_type)
    # The boot catalog initial entry should have a load segment of 0.
    assert(iso.eltorito_boot_catalog.initial_entry.load_segment == 0)
    # The boot catalog initial entry should have a system type of 0.
    assert(iso.eltorito_boot_catalog.initial_entry.system_type == system_type)
    # The boot catalog initial entry should have a sector count of 4.
    if media_type == 0:
        sector_count = 4
    else:
        sector_count = 1
    assert(iso.eltorito_boot_catalog.initial_entry.sector_count == sector_count)
    # The boot catalog initial entry should have the correct load rba.
    if load_rba is not None:
        assert(iso.eltorito_boot_catalog.initial_entry.load_rba == load_rba)
    # The El Torito boot record should always be at extent 17.
    assert(eltorito.extent_location() == 17)

def internal_check_jolietvd(svd, space_size, path_tbl_size, path_tbl_loc_le,
                            path_tbl_loc_be):
    # The supplementary volume descriptor should always have a version of 1.
    assert(svd.version == 1 or svd.version == 2)
    # The supplementary volume descriptor should always have flags of 0.
    assert(svd.flags == 0)
    # The supplementary volume descriptor system identifier length should always
    # be 32.
    assert(len(svd.system_identifier) == 32)
    # The supplementary volume descriptor volume identifer length should always
    # be 32.
    assert(len(svd.volume_identifier) == 32)
    # The amount of space the ISO takes depends on the files and directories
    # on the ISO.
    assert(svd.space_size == space_size)
    # The supplementary volume descriptor in these tests only supports the one
    # Joliet sequence of '%\E'.
    assert(svd.escape_sequences == b'%/@'+b'\x00'*29 or
           svd.escape_sequences == b'%/C'+b'\x00'*29 or
           svd.escape_sequences == b'%/E'+b'\x00'*29)
    # The supplementary volume descriptor should always have a set size of 1.
    assert(svd.set_size == 1)
    # The supplementary volume descriptor should always have a sequence number of 1.
    assert(svd.seqnum == 1)
    # The supplementary volume descriptor should always have a logical block size
    # of 2048.
    assert(svd.log_block_size == 2048)
    # The path table size depends on how many directories there are on the ISO.
    assert(svd.path_tbl_size == path_tbl_size)
    # The little endian version of the path table moves depending on what else is
    # on the ISO.
    assert(svd.path_table_location_le == path_tbl_loc_le)
    # The optional path table location should be 0.
    assert(svd.optional_path_table_location_le == 0)
    # The big endian version of the path table changes depending on how many
    # directories there are on the ISO.
    assert(svd.path_table_location_be == path_tbl_loc_be)
    # The length of the volume set identifer should always be 128.
    assert(svd.volume_set_identifier == b'\x00 '*64)
    # The publisher identifier text should be blank.
    assert(svd.publisher_identifier.text == b'\x00 '*64)
    # The preparer identifier text should be blank.
    assert(svd.preparer_identifier.text == b'\x00 '*64)
    # The copyright file identifier should be blank.
    assert(svd.copyright_file_identifier == b'\x00 '*18+b'\x00')
    # The abstract file identifier should be blank.
    assert(svd.abstract_file_identifier == b'\x00 '*18+b'\x00')
    # The bibliographic file identifier should be blank.
    assert(svd.bibliographic_file_identifier == b'\x00 '*18+b'\x00')
    # The supplementary volume descriptor should always have a file structure version
    # of 1.
    assert(svd.file_structure_version == 1)
    # The length of the application use string should always be 512.
    assert(len(svd.application_use) == 512)

def internal_check_terminator(terminators, extent):
    # There should only ever be one terminator (though the standard seems to
    # allow for multiple, I'm not sure how or why that would work).
    assert(len(terminators) == 1)
    terminator = terminators[0]

    assert(terminator.extent_location() == extent)

def internal_check_root_dir_record(root_dir_record, num_children, data_length,
                                   extent_location, rr, rr_nlinks, xa, rr_onetwelve):
    # The root_dir_record directory record length should be exactly 34.
    assert(root_dir_record.dr_len == 34)
    # We don't support xattrs at the moment, so it should always be 0.
    assert(root_dir_record.xattr_len == 0)
    # Make sure the root directory record starts at the extent we expect.
    assert(root_dir_record.extent_location() == extent_location)

    # We don't check the extent_location_le or extent_location_be, since I
    # don't really understand the algorithm by which genisoimage generates them.

    # The length of the root directory record depends on the number of entries
    # there are at the top level.
    assert(root_dir_record.get_data_length() == data_length)

    # We skip checking the date since it changes all of the time.

    # The file flags for the root dir record should always be 0x2 (DIRECTORY bit).
    assert(root_dir_record.file_flags == 2)
    # The file unit size should always be zero.
    assert(root_dir_record.file_unit_size == 0)
    # The interleave gap size should always be zero.
    assert(root_dir_record.interleave_gap_size == 0)
    # The sequence number should always be one.
    assert(root_dir_record.seqnum == 1)
    # The len_fi should always be one.
    assert(root_dir_record.len_fi == 1)

    # Everything after here is derived data.

    # The root directory should be the, erm, root.
    assert(root_dir_record.is_root == True)
    # The root directory record should also be a directory.
    assert(root_dir_record.isdir == True)
    # The root directory record should have a name of the byte 0.
    assert(root_dir_record.file_ident == b'\x00')
    assert(root_dir_record.parent == None)
    assert(root_dir_record.rock_ridge == None)
    # The number of children the root directory record has depends on the number
    # of files+directories there are at the top level.
    assert(len(root_dir_record.children) == num_children)

    # Now check the 'dot' directory record.
    internal_check_dot_dir_record(root_dir_record.children[0], rr=rr, rr_nlinks=rr_nlinks, first_dot=True, xa=xa, datalen=data_length, rr_onetwelve=rr_onetwelve)

    # Now check the 'dotdot' directory record.
    internal_check_dotdot_dir_record(root_dir_record.children[1], rr=rr, rr_nlinks=rr_nlinks, xa=xa, rr_onetwelve=rr_onetwelve)

def internal_check_dot_dir_record(dot_record, rr, rr_nlinks, first_dot, xa, datalen, rr_onetwelve):
    # The file identifier for the 'dot' directory entry should be the byte 0.
    assert(dot_record.file_ident == b'\x00')
    # The 'dot' directory entry should be a directory.
    assert(dot_record.isdir == True)
    # The 'dot' directory record length should be exactly 34 with no extensions.
    if rr:
        if first_dot:
            if rr_onetwelve:
                expected_dr_len = 140
            else:
                expected_dr_len = 136
        else:
            # All other Rock Ridge 'dot' entries are 102 bytes long.
            expected_dr_len = 102
    else:
        expected_dr_len = 34

    if xa:
        expected_dr_len += 14

    assert(dot_record.data_length == datalen)
    assert(dot_record.dr_len == expected_dr_len)
    # The 'dot' directory record is not the root.
    assert(dot_record.is_root == False)
    # The 'dot' directory record should have no children.
    assert(len(dot_record.children) == 0)
    assert(dot_record.file_flags == 2)

    if rr:
        assert(dot_record.rock_ridge._initialized == True)
        if first_dot:
            assert(dot_record.rock_ridge.dr_entries.sp_record != None)
            if xa:
                assert(dot_record.rock_ridge.dr_entries.sp_record.bytes_to_skip == 14)
            else:
                assert(dot_record.rock_ridge.dr_entries.sp_record.bytes_to_skip == 0)
        else:
            assert(dot_record.rock_ridge.dr_entries.sp_record == None)
        if not rr_onetwelve:
            assert(dot_record.rock_ridge.dr_entries.rr_record != None)
            assert(dot_record.rock_ridge.dr_entries.rr_record.rr_flags == 0x81)
        if first_dot:
            assert(dot_record.rock_ridge.dr_entries.ce_record != None)
            assert(dot_record.rock_ridge.ce_entries.sp_record == None)
            assert(dot_record.rock_ridge.ce_entries.rr_record == None)
            assert(dot_record.rock_ridge.ce_entries.ce_record == None)
            assert(dot_record.rock_ridge.ce_entries.px_record == None)
            assert(dot_record.rock_ridge.ce_entries.er_record != None)
            if not rr_onetwelve:
                assert(dot_record.rock_ridge.ce_entries.er_record.ext_id == b'RRIP_1991A')
                assert(dot_record.rock_ridge.ce_entries.er_record.ext_des == b'THE ROCK RIDGE INTERCHANGE PROTOCOL PROVIDES SUPPORT FOR POSIX FILE SYSTEM SEMANTICS')
                assert(dot_record.rock_ridge.ce_entries.er_record.ext_src == b'PLEASE CONTACT DISC PUBLISHER FOR SPECIFICATION SOURCE.  SEE PUBLISHER IDENTIFIER IN PRIMARY VOLUME DESCRIPTOR FOR CONTACT INFORMATION.')
            else:
                assert(dot_record.rock_ridge.ce_entries.er_record.ext_id == b'IEEE_P1282')
                assert(dot_record.rock_ridge.ce_entries.er_record.ext_des == b'THE IEEE P1282 PROTOCOL PROVIDES SUPPORT FOR POSIX FILE SYSTEM SEMANTICS')
                assert(dot_record.rock_ridge.ce_entries.er_record.ext_src == b'PLEASE CONTACT THE IEEE STANDARDS DEPARTMENT, PISCATAWAY, NJ, USA FOR THE P1282 SPECIFICATION')
            assert(dot_record.rock_ridge.ce_entries.es_records == [])
            assert(dot_record.rock_ridge.ce_entries.pn_record == None)
            assert(dot_record.rock_ridge.ce_entries.sl_records == [])
            assert(dot_record.rock_ridge.ce_entries.nm_records == [])
            assert(dot_record.rock_ridge.ce_entries.cl_record == None)
            assert(dot_record.rock_ridge.ce_entries.pl_record == None)
            assert(dot_record.rock_ridge.ce_entries.tf_record == None)
            assert(dot_record.rock_ridge.ce_entries.sf_record == None)
            assert(dot_record.rock_ridge.ce_entries.re_record == None)
        else:
            assert(dot_record.rock_ridge.dr_entries.ce_record == None)
        assert(dot_record.rock_ridge.dr_entries.px_record != None)
        assert(dot_record.rock_ridge.dr_entries.px_record.posix_file_mode == 0o040555)
        assert(dot_record.rock_ridge.dr_entries.px_record.posix_file_links == rr_nlinks)
        assert(dot_record.rock_ridge.dr_entries.px_record.posix_user_id == 0)
        assert(dot_record.rock_ridge.dr_entries.px_record.posix_group_id == 0)
        assert(dot_record.rock_ridge.dr_entries.px_record.posix_serial_number == 0)
        assert(dot_record.rock_ridge.dr_entries.er_record == None)
        assert(dot_record.rock_ridge.dr_entries.es_records == [])
        assert(dot_record.rock_ridge.dr_entries.pn_record == None)
        assert(dot_record.rock_ridge.dr_entries.sl_records == [])
        assert(dot_record.rock_ridge.dr_entries.nm_records == [])
        assert(dot_record.rock_ridge.dr_entries.cl_record == None)
        assert(dot_record.rock_ridge.dr_entries.pl_record == None)
        assert(dot_record.rock_ridge.dr_entries.tf_record != None)
        assert(dot_record.rock_ridge.dr_entries.tf_record.creation_time == None)
        assert(type(dot_record.rock_ridge.dr_entries.tf_record.access_time) == pycdlib.dates.DirectoryRecordDate)
        assert(type(dot_record.rock_ridge.dr_entries.tf_record.modification_time) == pycdlib.dates.DirectoryRecordDate)
        assert(type(dot_record.rock_ridge.dr_entries.tf_record.attribute_change_time) == pycdlib.dates.DirectoryRecordDate)
        assert(dot_record.rock_ridge.dr_entries.tf_record.backup_time == None)
        assert(dot_record.rock_ridge.dr_entries.tf_record.expiration_time == None)
        assert(dot_record.rock_ridge.dr_entries.tf_record.effective_time == None)
        assert(dot_record.rock_ridge.dr_entries.sf_record == None)
        assert(dot_record.rock_ridge.dr_entries.re_record == None)

def internal_check_dotdot_dir_record(dotdot_record, rr, rr_nlinks, xa, rr_onetwelve):
    # The file identifier for the 'dotdot' directory entry should be the byte 1.
    assert(dotdot_record.file_ident == b'\x01')
    # The 'dotdot' directory entry should be a directory.
    assert(dotdot_record.isdir == True)
    # The 'dotdot' directory record length should be exactly 34 with no extensions.
    if rr:
        if rr_onetwelve:
            expected_dr_len = 104
        else:
            expected_dr_len = 102
    else:
        expected_dr_len = 34

    if xa:
        expected_dr_len += 14

    assert(dotdot_record.dr_len == expected_dr_len)
    # The 'dotdot' directory record is not the root.
    assert(dotdot_record.is_root == False)
    # The 'dotdot' directory record should have no children.
    assert(len(dotdot_record.children) == 0)
    assert(dotdot_record.file_flags == 2)

    if rr:
        assert(dotdot_record.rock_ridge._initialized == True)
        assert(dotdot_record.rock_ridge.dr_entries.sp_record == None)
        if not rr_onetwelve:
            assert(dotdot_record.rock_ridge.dr_entries.rr_record != None)
            assert(dotdot_record.rock_ridge.dr_entries.rr_record.rr_flags == 0x81)
        assert(dotdot_record.rock_ridge.dr_entries.ce_record == None)
        assert(dotdot_record.rock_ridge.dr_entries.px_record != None)
        assert(dotdot_record.rock_ridge.dr_entries.px_record.posix_file_mode == 0o040555)
        assert(dotdot_record.rock_ridge.dr_entries.px_record.posix_file_links == rr_nlinks)
        assert(dotdot_record.rock_ridge.dr_entries.px_record.posix_user_id == 0)
        assert(dotdot_record.rock_ridge.dr_entries.px_record.posix_group_id == 0)
        assert(dotdot_record.rock_ridge.dr_entries.px_record.posix_serial_number == 0)
        assert(dotdot_record.rock_ridge.dr_entries.er_record == None)
        assert(dotdot_record.rock_ridge.dr_entries.es_records == [])
        assert(dotdot_record.rock_ridge.dr_entries.pn_record == None)
        assert(dotdot_record.rock_ridge.dr_entries.sl_records == [])
        assert(dotdot_record.rock_ridge.dr_entries.nm_records == [])
        assert(dotdot_record.rock_ridge.dr_entries.cl_record == None)
        assert(dotdot_record.rock_ridge.dr_entries.pl_record == None)
        assert(dotdot_record.rock_ridge.dr_entries.tf_record != None)
        assert(dotdot_record.rock_ridge.dr_entries.tf_record.creation_time == None)
        assert(type(dotdot_record.rock_ridge.dr_entries.tf_record.access_time) == pycdlib.dates.DirectoryRecordDate)
        assert(type(dotdot_record.rock_ridge.dr_entries.tf_record.modification_time) == pycdlib.dates.DirectoryRecordDate)
        assert(type(dotdot_record.rock_ridge.dr_entries.tf_record.attribute_change_time) == pycdlib.dates.DirectoryRecordDate)
        assert(dotdot_record.rock_ridge.dr_entries.tf_record.backup_time == None)
        assert(dotdot_record.rock_ridge.dr_entries.tf_record.expiration_time == None)
        assert(dotdot_record.rock_ridge.dr_entries.tf_record.effective_time == None)
        assert(dotdot_record.rock_ridge.dr_entries.sf_record == None)
        assert(dotdot_record.rock_ridge.dr_entries.re_record == None)

def internal_check_file_contents(iso, path, contents, which):
    fout = io.BytesIO()
    if which == 'iso_path':
        iso.get_file_from_iso_fp(fout, iso_path=path)
    elif which == 'rr_path':
        iso.get_file_from_iso_fp(fout, rr_path=path)
    elif which == 'joliet_path':
        iso.get_file_from_iso_fp(fout, joliet_path=path)
    elif which == 'udf_path':
        iso.get_file_from_iso_fp(fout, udf_path=path)
    else:
        assert('' == 'Invalid Test parameter')
    assert(fout.getvalue() == contents)

def internal_check_ptr(ptr, name, len_di, loc, parent):
    assert(ptr.len_di == len_di)
    assert(ptr.xattr_length == 0)
    if loc is not None:
        assert(ptr.extent_location == loc)
    if parent > 0:
        assert(ptr.parent_directory_num == parent)
    assert(ptr.directory_identifier == name)

def internal_check_empty_directory(dirrecord, name, dr_len, extent, rr, hidden):
    internal_check_dir_record(dirrecord, num_children=2, name=name, dr_len=dr_len, extent_location=extent, rr=rr, rr_name=b'dir1', rr_links=2, xa=False, hidden=hidden, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dirrecord.children[1], rr=rr, rr_nlinks=3, xa=False, rr_onetwelve=False)

def internal_check_file(dirrecord, name, dr_len, loc, datalen, hidden, multi_extent=False):
    assert(len(dirrecord.children) == 0)
    assert(dirrecord.isdir == False)
    assert(dirrecord.is_root == False)
    assert(dirrecord.file_ident == name)
    if dr_len is not None:
        assert(dirrecord.dr_len == dr_len)
    if loc is not None:
        assert(dirrecord.extent_location() == loc)
    if hidden:
        assert(dirrecord.file_flags == 1)
    elif multi_extent:
        assert(dirrecord.file_flags == 128)
    else:
        assert(dirrecord.file_flags == 0)
    assert(dirrecord.get_data_length() == datalen)

def internal_generate_inorder_names(numdirs):
    tmp = []
    for i in range(1, 1+numdirs):
        tmp.append(b'DIR' + bytes(str(i).encode('ascii')))
    names = sorted(tmp)
    names.insert(0, None)
    names.insert(0, None)
    return names

def internal_generate_joliet_inorder_names(numdirs):
    tmp = []
    for i in range(1, 1+numdirs):
        name = 'dir' + str(i)
        tmp.append(bytes(name.encode('utf-16_be')))
    names = sorted(tmp)
    names.insert(0, None)
    names.insert(0, None)
    return names

def internal_generate_udf_inorder_names(numdirs):
    tmp = []
    for i in range(1, 1+numdirs):
        tmp.append(b'dir' + bytes(str(i).encode('ascii')))
    names = sorted(tmp)
    names.insert(0, None)
    return names

def internal_check_dir_record(dir_record, num_children, name, dr_len,
                              extent_location, rr, rr_name, rr_links, xa, hidden,
                              is_cl_record, datalen, relocated):
    # The directory should have the number of children passed in.
    assert(len(dir_record.children) == num_children)
    # The directory should be a directory.
    if is_cl_record:
        assert(dir_record.isdir == False)
    else:
        assert(dir_record.isdir == True)
    # The directory should not be the root.
    assert(dir_record.is_root == False)
    # The directory should have an ISO9660 mangled name the same as passed in.
    assert(dir_record.file_ident == name)
    # The directory record should have a dr_len as passed in.
    if dr_len is not None:
        assert(dir_record.dr_len == dr_len)
    # The 'dir1' directory record should be at the extent passed in.
    if extent_location is not None:
        assert(dir_record.extent_location() == extent_location)
    if is_cl_record:
        assert(dir_record.file_flags == 0)
    else:
        if hidden:
            assert(dir_record.file_flags == 3)
        else:
            assert(dir_record.file_flags == 2)

    if rr:
        assert(dir_record.rock_ridge.dr_entries.sp_record == None)
        assert(dir_record.rock_ridge.dr_entries.rr_record != None)
        if is_cl_record:
            assert(dir_record.rock_ridge.dr_entries.rr_record.rr_flags == 0x99)
        elif relocated:
            assert(dir_record.rock_ridge.dr_entries.rr_record.rr_flags == 0xC9)
        else:
            assert(dir_record.rock_ridge.dr_entries.rr_record.rr_flags == 0x89)

        px_record = None
        if dir_record.rock_ridge.dr_entries.px_record is not None:
            px_record = dir_record.rock_ridge.dr_entries.px_record
        elif dir_record.rock_ridge.ce_entries.px_record is not None:
            px_record = dir_record.rock_ridge.ce_entries.px_record
        assert(px_record is not None)
        assert(px_record.posix_file_mode == 0o040555)
        assert(px_record.posix_file_links == rr_links)
        assert(px_record.posix_user_id == 0)
        assert(px_record.posix_group_id == 0)
        assert(px_record.posix_serial_number == 0)
        assert(dir_record.rock_ridge.dr_entries.er_record == None)
        assert(dir_record.rock_ridge.dr_entries.es_records == [])
        assert(dir_record.rock_ridge.dr_entries.pn_record == None)
        assert(dir_record.rock_ridge.dr_entries.sl_records == [])
        assert(len(dir_record.rock_ridge.dr_entries.nm_records) > 0)
        assert(dir_record.rock_ridge.name() == rr_name)
        if is_cl_record:
            assert(dir_record.rock_ridge.dr_entries.cl_record != None)
        else:
            assert(dir_record.rock_ridge.dr_entries.cl_record == None)
        assert(dir_record.rock_ridge.dr_entries.pl_record == None)
        if dir_record.rock_ridge.dr_entries.tf_record is not None:
            tf_record = dir_record.rock_ridge.dr_entries.tf_record
        elif dir_record.rock_ridge.ce_entries.tf_record is not None:
            tf_record = dir_record.rock_ridge.ce_entries.tf_record
        assert(tf_record is not None)
        assert(tf_record.creation_time == None)
        assert(type(tf_record.access_time) == pycdlib.dates.DirectoryRecordDate)
        assert(type(tf_record.modification_time) == pycdlib.dates.DirectoryRecordDate)
        assert(type(tf_record.attribute_change_time) == pycdlib.dates.DirectoryRecordDate)
        assert(tf_record.backup_time == None)
        assert(tf_record.expiration_time == None)
        assert(tf_record.effective_time == None)
        assert(dir_record.rock_ridge.dr_entries.sf_record == None)
        if relocated:
            assert(dir_record.rock_ridge.dr_entries.re_record != None)
        else:
            assert(dir_record.rock_ridge.dr_entries.re_record == None)

    # The 'dir1' directory record should have a valid 'dot' record.
    if num_children > 0:
        internal_check_dot_dir_record(dir_record.children[0], rr=rr, rr_nlinks=rr_links, first_dot=False, xa=xa, datalen=datalen, rr_onetwelve=False)

def internal_check_joliet_root_dir_record(jroot_dir_record, num_children,
                                          data_length, extent_location):
    # The jroot_dir_record directory record length should be exactly 34.
    assert(jroot_dir_record.dr_len == 34)
    # We don't support xattrs at the moment, so it should always be 0.
    assert(jroot_dir_record.xattr_len == 0)
    # Make sure the root directory record starts at the extent we expect.
    assert(jroot_dir_record.extent_location() == extent_location)

    # We don't check the extent_location_le or extent_location_be, since I
    # don't really understand the algorithm by which genisoimage generates them.

    # The length of the root directory record depends on the number of entries
    # there are at the top level.
    assert(jroot_dir_record.get_data_length() == data_length)

    # We skip checking the date since it changes all of the time.

    # The file flags for the root dir record should always be 0x2 (DIRECTORY bit).
    assert(jroot_dir_record.file_flags == 2)
    # The file unit size should always be zero.
    assert(jroot_dir_record.file_unit_size == 0)
    # The interleave gap size should always be zero.
    assert(jroot_dir_record.interleave_gap_size == 0)
    # The sequence number should always be one.
    assert(jroot_dir_record.seqnum == 1)
    # The len_fi should always be one.
    assert(jroot_dir_record.len_fi == 1)

    # Everything after here is derived data.

    # The root directory should be the, erm, root.
    assert(jroot_dir_record.is_root == True)
    # The root directory record should also be a directory.
    assert(jroot_dir_record.isdir == True)
    # The root directory record should have a name of the byte 0.
    assert(jroot_dir_record.file_ident == b'\x00')
    assert(jroot_dir_record.parent == None)
    assert(jroot_dir_record.rock_ridge == None)
    # The number of children the root directory record has depends on the number
    # of files+directories there are at the top level.
    assert(len(jroot_dir_record.children) == num_children)

    # Now check the 'dot' directory record.
    internal_check_dot_dir_record(jroot_dir_record.children[0], rr=False, rr_nlinks=0, first_dot=False, xa=False, datalen=2048, rr_onetwelve=False)

    # Now check the 'dotdot' directory record.
    internal_check_dotdot_dir_record(jroot_dir_record.children[1], rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

def internal_check_rr_longname(iso, dir_record, extent, letter):
    bytes_iso_name = letter.upper()*8 + b'.;1'
    str_iso_path = '/' + bytes_iso_name.decode('ascii')
    str_rr_path = '/' + letter.decode('ascii')*RR_MAX_FILENAME_LENGTH
    internal_check_file(dir_record, name=bytes_iso_name, dr_len=None, loc=extent, datalen=3, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path=str_iso_path, contents=letter*2+b'\n', which='iso_path')
    # Now check rock ridge extensions.
    assert(dir_record.rock_ridge.dr_entries.sp_record == None)
    assert(dir_record.rock_ridge.dr_entries.rr_record != None)
    assert(dir_record.rock_ridge.dr_entries.rr_record.rr_flags == 0x89)
    assert(dir_record.rock_ridge.dr_entries.ce_record != None)
    assert(dir_record.rock_ridge.ce_entries.sp_record == None)
    assert(dir_record.rock_ridge.ce_entries.rr_record == None)
    assert(dir_record.rock_ridge.ce_entries.ce_record == None)
    assert(dir_record.rock_ridge.ce_entries.px_record != None)
    assert(dir_record.rock_ridge.ce_entries.px_record.posix_file_mode == 0o0100444)
    assert(dir_record.rock_ridge.ce_entries.px_record.posix_file_links == 1)
    assert(dir_record.rock_ridge.ce_entries.px_record.posix_user_id == 0)
    assert(dir_record.rock_ridge.ce_entries.px_record.posix_group_id == 0)
    assert(dir_record.rock_ridge.ce_entries.px_record.posix_serial_number == 0)
    assert(dir_record.rock_ridge.ce_entries.er_record == None)
    assert(dir_record.rock_ridge.ce_entries.es_records == [])
    assert(dir_record.rock_ridge.ce_entries.pn_record == None)
    assert(dir_record.rock_ridge.ce_entries.sl_records == [])
    assert(len(dir_record.rock_ridge.ce_entries.nm_records) > 0)
    assert(dir_record.rock_ridge.ce_entries.nm_records[0].posix_name_flags == 0)
    assert(dir_record.rock_ridge.ce_entries.cl_record == None)
    assert(dir_record.rock_ridge.ce_entries.pl_record == None)
    assert(dir_record.rock_ridge.ce_entries.tf_record != None)
    assert(type(dir_record.rock_ridge.ce_entries.tf_record.access_time) == pycdlib.dates.DirectoryRecordDate)
    assert(type(dir_record.rock_ridge.ce_entries.tf_record.modification_time) == pycdlib.dates.DirectoryRecordDate)
    assert(type(dir_record.rock_ridge.ce_entries.tf_record.attribute_change_time) == pycdlib.dates.DirectoryRecordDate)
    assert(dir_record.rock_ridge.ce_entries.sf_record == None)
    assert(dir_record.rock_ridge.ce_entries.re_record == None)
    assert(dir_record.rock_ridge.dr_entries.px_record == None)
    assert(dir_record.rock_ridge.dr_entries.er_record == None)
    assert(dir_record.rock_ridge.dr_entries.es_records == [])
    assert(dir_record.rock_ridge.dr_entries.pn_record == None)
    assert(dir_record.rock_ridge.dr_entries.sl_records == [])
    assert(len(dir_record.rock_ridge.dr_entries.nm_records) > 0)
    assert(dir_record.rock_ridge.dr_entries.nm_records[0].posix_name_flags == 1)
    assert(dir_record.rock_ridge.name() == letter*RR_MAX_FILENAME_LENGTH)
    assert(dir_record.rock_ridge.dr_entries.cl_record == None)
    assert(dir_record.rock_ridge.dr_entries.pl_record == None)
    assert(dir_record.rock_ridge.dr_entries.tf_record == None)
    assert(dir_record.rock_ridge.dr_entries.sf_record == None)
    assert(dir_record.rock_ridge.dr_entries.re_record == None)
    internal_check_file_contents(iso, path=str_rr_path, contents=letter*2+b'\n', which='rr_path')

def internal_check_rr_file(dir_record, name):
    assert(dir_record.rock_ridge._initialized == True)
    assert(dir_record.rock_ridge.dr_entries.sp_record == None)
    assert(dir_record.rock_ridge.dr_entries.rr_record != None)
    assert(dir_record.rock_ridge.dr_entries.rr_record.rr_flags == 0x89)
    assert(dir_record.rock_ridge.dr_entries.ce_record == None)
    assert(dir_record.rock_ridge.dr_entries.px_record != None)
    assert(dir_record.rock_ridge.dr_entries.px_record.posix_file_mode == 0o0100444)
    assert(dir_record.rock_ridge.dr_entries.px_record.posix_file_links == 1)
    assert(dir_record.rock_ridge.dr_entries.px_record.posix_user_id == 0)
    assert(dir_record.rock_ridge.dr_entries.px_record.posix_group_id == 0)
    assert(dir_record.rock_ridge.dr_entries.px_record.posix_serial_number == 0)
    assert(dir_record.rock_ridge.dr_entries.er_record == None)
    assert(dir_record.rock_ridge.dr_entries.es_records == [])
    assert(dir_record.rock_ridge.dr_entries.pn_record == None)
    assert(dir_record.rock_ridge.dr_entries.sl_records == [])
    assert(len(dir_record.rock_ridge.dr_entries.nm_records) > 0)
    assert(dir_record.rock_ridge.dr_entries.nm_records[0].posix_name_flags == 0)
    assert(dir_record.rock_ridge.dr_entries.nm_records[0].posix_name == name)
    assert(dir_record.rock_ridge.dr_entries.cl_record == None)
    assert(dir_record.rock_ridge.dr_entries.pl_record == None)
    assert(dir_record.rock_ridge.dr_entries.tf_record != None)
    assert(dir_record.rock_ridge.dr_entries.tf_record.creation_time == None)
    assert(type(dir_record.rock_ridge.dr_entries.tf_record.access_time) == pycdlib.dates.DirectoryRecordDate)
    assert(type(dir_record.rock_ridge.dr_entries.tf_record.modification_time) == pycdlib.dates.DirectoryRecordDate)
    assert(type(dir_record.rock_ridge.dr_entries.tf_record.attribute_change_time) == pycdlib.dates.DirectoryRecordDate)
    assert(dir_record.rock_ridge.dr_entries.tf_record.backup_time == None)
    assert(dir_record.rock_ridge.dr_entries.tf_record.expiration_time == None)
    assert(dir_record.rock_ridge.dr_entries.tf_record.effective_time == None)
    assert(dir_record.rock_ridge.dr_entries.sf_record == None)
    assert(dir_record.rock_ridge.dr_entries.re_record == None)

def internal_check_rr_symlink(dir_record, name, dr_len, comps):
    # The 'sym' file should not have any children.
    assert(len(dir_record.children) == 0)
    # The 'sym' file should not be a directory.
    assert(dir_record.isdir == False)
    # The 'sym' file should not be the root.
    assert(dir_record.is_root == False)
    # The 'sym' file should have an ISO9660 mangled name of 'SYM.;1'.
    assert(dir_record.file_ident == name)
    # The 'sym' directory record should have a length of 126.
    assert(dir_record.dr_len == dr_len)
    assert(dir_record.file_flags == 0)
    # Now check rock ridge extensions.
    assert(dir_record.rock_ridge._initialized == True)
    assert(dir_record.rock_ridge.dr_entries.sp_record == None)
    assert(dir_record.rock_ridge.dr_entries.rr_record != None)
    assert(dir_record.rock_ridge.dr_entries.rr_record.rr_flags == 0x8d)
    assert(dir_record.rock_ridge.dr_entries.px_record != None)
    assert(dir_record.rock_ridge.dr_entries.px_record.posix_file_mode == 0o0120555)
    assert(dir_record.rock_ridge.dr_entries.px_record.posix_file_links == 1)
    assert(dir_record.rock_ridge.dr_entries.px_record.posix_user_id == 0)
    assert(dir_record.rock_ridge.dr_entries.px_record.posix_group_id == 0)
    assert(dir_record.rock_ridge.dr_entries.px_record.posix_serial_number == 0)
    assert(dir_record.rock_ridge.dr_entries.er_record == None)
    assert(dir_record.rock_ridge.dr_entries.es_records == [])
    assert(dir_record.rock_ridge.dr_entries.pn_record == None)
    assert(dir_record.rock_ridge.is_symlink() == True)
    split = dir_record.rock_ridge.symlink_path().split(b'/')
    assert(len(split) == len(comps))
    for index,comp in enumerate(comps):
        assert(comps[index] == split[index])
    assert(len(dir_record.rock_ridge.dr_entries.nm_records) > 0)
    assert(dir_record.rock_ridge.dr_entries.nm_records[0].posix_name_flags == 0)
    assert(dir_record.rock_ridge.dr_entries.nm_records[0].posix_name == b'sym')
    assert(dir_record.rock_ridge.dr_entries.cl_record == None)
    assert(dir_record.rock_ridge.dr_entries.pl_record == None)
    tf_record = None
    if dir_record.rock_ridge.dr_entries.tf_record is not None:
        tf_record = dir_record.rock_ridge.dr_entries.tf_record
    elif dir_record.rock_ridge.ce_entries.tf_record is not None:
        tf_record = dir_record.rock_ridge.ce_entries.tf_record
    assert(tf_record != None)
    assert(tf_record.creation_time == None)
    assert(type(tf_record.access_time) == pycdlib.dates.DirectoryRecordDate)
    assert(type(tf_record.modification_time) == pycdlib.dates.DirectoryRecordDate)
    assert(type(tf_record.attribute_change_time) == pycdlib.dates.DirectoryRecordDate)
    assert(tf_record.backup_time == None)
    assert(tf_record.expiration_time == None)
    assert(tf_record.effective_time == None)
    assert(dir_record.rock_ridge.dr_entries.sf_record == None)
    assert(dir_record.rock_ridge.dr_entries.re_record == None)

def internal_check_udf_tag(tag, ident, location):
    assert(tag.tag_ident == ident)
    assert(tag.desc_version == 2)
    assert(tag.tag_serial_number == 0)
    if location is not None:
        assert(tag.tag_location == location)

def internal_check_udf_anchor(anchor, location):
    assert(anchor.extent_location() == location)
    internal_check_udf_tag(anchor.desc_tag, ident=2, location=location)
    assert(anchor.main_vd.extent_length == 32768)
    assert(anchor.main_vd.extent_location == 32)
    assert(anchor.reserve_vd.extent_length == 32768)
    assert(anchor.reserve_vd.extent_location == 48)

def internal_check_udf_entity(entity, flags, ident, suffix):
    assert(entity.flags == flags)
    if ident is not None:
        full = ident + b'\x00' * (23 - len(ident))
        assert(entity.identifier == full)
    full = suffix + b'\x00' * (8 - len(suffix))
    assert(entity.suffix == full)

def internal_check_udf_pvd(pvd, location):
    assert(pvd.extent_location() == location)
    internal_check_udf_tag(pvd.desc_tag, ident=1, location=location)
    assert(pvd.vol_desc_seqnum == 0)
    assert(pvd.desc_num == 0)
    assert(pvd.vol_ident == b'\x08CDROM\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x06')
    assert(pvd.desc_char_set.set_type == 0)
    assert(pvd.desc_char_set.set_information == b'OSTA Compressed Unicode\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00')
    assert(pvd.explanatory_char_set.set_type == 0)
    assert(pvd.explanatory_char_set.set_information == b'OSTA Compressed Unicode\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00')
    assert(pvd.vol_abstract.extent_length == 0)
    assert(pvd.vol_abstract.extent_location == 0)
    assert(pvd.vol_copyright.extent_length == 0)
    assert(pvd.vol_copyright.extent_location == 0)
    internal_check_udf_entity(pvd.app_ident, 0, b'\x00', b'')
    internal_check_udf_entity(pvd.impl_ident, 0, None, b'')
    assert(pvd.implementation_use == b'\x00' * 64)
    assert(pvd.predecessor_vol_desc_location == 0)

def internal_check_udf_impl_use(impl_use, location):
    assert(impl_use.extent_location() == location)
    internal_check_udf_tag(impl_use.desc_tag, ident=4, location=location)
    assert(impl_use.vol_desc_seqnum == 1)
    assert(impl_use.impl_ident.identifier == b'*UDF LV Info\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00')
    assert(impl_use.impl_use.char_set.set_type == 0)
    assert(impl_use.impl_use.char_set.set_information == b'OSTA Compressed Unicode' + b'\x00' * 40)
    assert(impl_use.impl_use.log_vol_ident == b'\x08CDROM' + b'\x00' * 121 + b'\x06')
    assert(impl_use.impl_use.lv_info1 == b'\x00' * 36)
    assert(impl_use.impl_use.lv_info2 == b'\x00' * 36)
    assert(impl_use.impl_use.lv_info3 == b'\x00' * 36)
    internal_check_udf_entity(impl_use.impl_ident, 0, b'*UDF LV Info', b'\x02\x01')
    assert(impl_use.impl_use.impl_use == b'\x00' * 128)

def internal_check_udf_partition(partition, location, length):
    assert(partition.extent_location() == location)
    internal_check_udf_tag(partition.desc_tag, ident=5, location=location)
    assert(partition.vol_desc_seqnum == 2)
    assert(partition.part_flags == 1)
    assert(partition.part_num == 0)
    assert(partition.part_contents.flags == 2)
    internal_check_udf_entity(partition.part_contents, 2, b'+NSR02', b'')
    assert(partition.access_type == 1)
    assert(partition.part_start_location == 257)
    assert(partition.part_length == length)
    internal_check_udf_entity(partition.impl_ident, 0, None, b'')
    assert(partition.implementation_use == b'\x00' * 128)

def internal_check_udf_longad(longad, size, blocknum, abs_blocknum):
    assert(longad.extent_length == size)
    if blocknum is not None:
        assert(longad.log_block_num == blocknum)
    assert(longad.part_ref_num == 0)
    if abs_blocknum is not None:
        assert(longad.impl_use == b'\x00\x00' + struct.pack('<L', abs_blocknum))

def internal_check_udf_logical_volume(lv, location):
    assert(lv.extent_location() == location)
    internal_check_udf_tag(lv.desc_tag, ident=6, location=location)
    assert(lv.vol_desc_seqnum == 3)
    assert(lv.desc_char_set.set_type == 0)
    assert(lv.desc_char_set.set_information == b'OSTA Compressed Unicode\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00')
    assert(lv.logical_vol_ident == b'\x08CDROM\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x06')
    internal_check_udf_entity(lv.domain_ident, 0, b'*OSTA UDF Compliant', b'\x02\x01\x03')
    internal_check_udf_longad(lv.logical_volume_contents_use, size=4096, blocknum=0, abs_blocknum=0)
    internal_check_udf_entity(lv.impl_ident, 0, None, b'')
    assert(lv.implementation_use == b'\x00' * 128)
    assert(lv.integrity_sequence.extent_length == 4096)
    assert(lv.integrity_sequence.extent_location == 64)

def internal_check_udf_unallocated_space(unallocated_space, location):
    assert(unallocated_space.extent_location() == location)
    internal_check_udf_tag(unallocated_space.desc_tag, ident=7, location=location)
    assert(unallocated_space.vol_desc_seqnum == 4)

def internal_check_udf_terminator(terminator, location, tagloc):
    assert(terminator.extent_location() == location)
    internal_check_udf_tag(terminator.desc_tag, ident=8, location=tagloc)

def internal_check_udf_headers(iso, bea_extent, end_anchor_extent, part_length, unique_id, num_dirs, num_files):
    assert(len(iso.udf_beas) == 1)
    assert(iso.udf_beas[0].extent_location() == bea_extent)
    assert(iso.udf_nsr is not None)
    assert(iso.udf_nsr.extent_location() == (bea_extent + 1))
    assert(len(iso.udf_teas) == 1)
    assert(iso.udf_teas[0].extent_location() == (bea_extent + 2))

    assert(len(iso.udf_anchors) == 2)
    internal_check_udf_anchor(iso.udf_anchors[0], location=256)
    internal_check_udf_anchor(iso.udf_anchors[1], location=end_anchor_extent)

    internal_check_udf_pvd(iso.udf_main_descs.pvds[0], location=32)

    internal_check_udf_impl_use(iso.udf_main_descs.impl_use[0], location=33)

    internal_check_udf_partition(iso.udf_main_descs.partitions[0], location=34, length=part_length)

    internal_check_udf_logical_volume(iso.udf_main_descs.logical_volumes[0], location=35)

    internal_check_udf_unallocated_space(iso.udf_main_descs.unallocated_space[0], location=36)

    internal_check_udf_terminator(iso.udf_main_descs.terminator, location=37, tagloc=37)

    internal_check_udf_pvd(iso.udf_reserve_descs.pvds[0], location=48)

    internal_check_udf_impl_use(iso.udf_reserve_descs.impl_use[0], location=49)

    internal_check_udf_partition(iso.udf_reserve_descs.partitions[0], location=50, length=part_length)

    internal_check_udf_logical_volume(iso.udf_reserve_descs.logical_volumes[0], location=51)

    internal_check_udf_unallocated_space(iso.udf_reserve_descs.unallocated_space[0], location=52)

    internal_check_udf_terminator(iso.udf_reserve_descs.terminator, location=53, tagloc=53)

    assert(iso.udf_logical_volume_integrity.extent_location() == 64)
    internal_check_udf_tag(iso.udf_logical_volume_integrity.desc_tag, ident=9, location=64)
    assert(iso.udf_logical_volume_integrity.logical_volume_contents_use.unique_id == unique_id)
    assert(iso.udf_logical_volume_integrity.length_impl_use == 46)
    assert(iso.udf_logical_volume_integrity.free_space_tables[0] == 0)
    assert(iso.udf_logical_volume_integrity.size_tables[0] == part_length)
    internal_check_udf_entity(iso.udf_logical_volume_integrity.logical_volume_impl_use.impl_id, 0, None, b'')
    assert(iso.udf_logical_volume_integrity.logical_volume_impl_use.num_files == num_files)
    assert(iso.udf_logical_volume_integrity.logical_volume_impl_use.num_dirs == num_dirs)
    assert(iso.udf_logical_volume_integrity.logical_volume_impl_use.min_udf_read_revision == 258)
    assert(iso.udf_logical_volume_integrity.logical_volume_impl_use.min_udf_write_revision == 258)
    assert(iso.udf_logical_volume_integrity.logical_volume_impl_use.max_udf_write_revision == 258)

    internal_check_udf_terminator(iso.udf_logical_volume_integrity_terminator, location=65, tagloc=65)

    internal_check_udf_tag(iso.udf_file_set.desc_tag, ident=256, location=0)
    internal_check_udf_entity(iso.udf_file_set.domain_ident, 0, b'*OSTA UDF Compliant', b'\x02\x01\x03')
    internal_check_udf_longad(iso.udf_file_set.root_dir_icb, size=2048, blocknum=2, abs_blocknum=0)

    internal_check_udf_terminator(iso.udf_file_set_terminator, location=258, tagloc=1)

def internal_check_udf_file_entry(file_entry, location, tag_location, num_links,
                                  info_len, num_blocks_recorded, num_fi_descs,
                                  file_type, num_alloc_descs):
    if location is not None:
        assert(file_entry.extent_location() == location)
    internal_check_udf_tag(file_entry.desc_tag, ident=261, location=tag_location)
    assert(file_entry.icb_tag.prior_num_direct_entries == 0)
    assert(file_entry.icb_tag.strategy_type == 4)
    assert(file_entry.icb_tag.strategy_param == 0)
    assert(file_entry.icb_tag.max_num_entries == 1)
    if file_type == 'dir':
        assert(file_entry.icb_tag.file_type == 4)
    elif file_type == 'symlink':
        assert(file_entry.icb_tag.file_type == 12)
    else:
        assert(file_entry.icb_tag.file_type == 5)
    assert(file_entry.icb_tag.parent_icb.logical_block_num == 0)
    assert(file_entry.icb_tag.parent_icb.part_ref_num == 0)
    assert(file_entry.icb_tag.flags == 560)
    assert(file_entry.uid == 4294967295)
    assert(file_entry.gid == 4294967295)
    if file_type == 'dir':
        assert(file_entry.perms == 5285)
    else:
        assert(file_entry.perms == 4228)
    assert(file_entry.file_link_count == num_links)
    assert(file_entry.info_len == info_len)
    assert(file_entry.log_block_recorded == num_blocks_recorded)
    internal_check_udf_longad(file_entry.extended_attr_icb, size=0, blocknum=0, abs_blocknum=0)
    internal_check_udf_entity(file_entry.impl_ident, 0, None, b'')
    assert(file_entry.extended_attrs == b'')
    assert(len(file_entry.alloc_descs) == num_alloc_descs)
    assert(len(file_entry.fi_descs) == num_fi_descs)

def internal_check_udf_file_ident_desc(fi_desc, extent, tag_location,
                                       characteristics, blocknum, abs_blocknum,
                                       name, isparent, isdir):
    if extent is not None:
        assert(fi_desc.extent_location() == extent)
    internal_check_udf_tag(fi_desc.desc_tag, ident=257, location=tag_location)
    assert(fi_desc.file_characteristics == characteristics)
    namelen = len(name)
    if namelen > 0:
        namelen += 1
    assert(fi_desc.len_fi == namelen)
    internal_check_udf_longad(fi_desc.icb, size=2048, blocknum=blocknum, abs_blocknum=abs_blocknum)
    assert(fi_desc.len_impl_use == 0)
    assert(fi_desc.impl_use == b'')
    assert(fi_desc.fi == name)
    if isparent:
        assert(fi_desc.file_entry is None)
    else:
        assert(fi_desc.file_entry is not None)
    assert(fi_desc.isdir == isdir)
    assert(fi_desc.isparent == isparent)

def internal_check_boot_info_table(bi_table, vd_extent, inode_extent, orig_len, csum):
    assert(bi_table is not None)
    assert(bi_table.vd.extent_location() == vd_extent)
    assert(bi_table.inode.extent_location() == inode_extent)
    assert(bi_table.orig_len == orig_len)
    assert(bi_table.csum == csum)

######################## EXTERNAL CHECKERS #####################################
def check_nofiles(iso, filesize):
    assert(filesize == 49152)

    internal_check_pvd(iso.pvd, extent=16, size=24, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=2, data_length=2048, extent_location=23, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    assert(not iso.has_rock_ridge())
    assert(not iso.has_joliet())
    assert(not iso.has_udf())

    # Check to make sure accessing a missing file results in an exception.
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibException):
        iso.get_file_from_iso_fp(io.BytesIO(), iso_path='/FOO.;1')

def check_onefile(iso, filesize):
    assert(filesize == 51200)

    internal_check_pvd(iso.pvd, extent=16, size=25, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=23, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'FOO.;1', dr_len=40, loc=24, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

def check_onedir(iso, filesize):
    assert(filesize == 51200)

    internal_check_pvd(iso.pvd, extent=16, size=25, ptbl_size=22, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=23, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    dir1_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(dir1_record.ptr, name=b'DIR1', len_di=4, loc=24, parent=1)
    internal_check_empty_directory(dir1_record, name=b'DIR1', dr_len=38, extent=24, rr=False, hidden=False)

def check_twofiles(iso, filesize):
    assert(filesize == 53248)

    internal_check_pvd(iso.pvd, extent=16, size=26, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=23, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BAR.;1', dr_len=40, loc=24, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BAR.;1', contents=b'bar\n', which='iso_path')

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'FOO.;1', dr_len=40, loc=25, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

def check_twodirs(iso, filesize):
    assert(filesize == 53248)

    internal_check_pvd(iso.pvd, extent=16, size=26, ptbl_size=30, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=23, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    aa_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(aa_record.ptr, name=b'AA', len_di=2, loc=None, parent=1)
    internal_check_empty_directory(aa_record, name=b'AA', dr_len=36, extent=None, rr=False, hidden=False)

    bb_record = iso.pvd.root_dir_record.children[3]
    internal_check_ptr(bb_record.ptr, name=b'BB', len_di=2, loc=None, parent=1)
    internal_check_empty_directory(bb_record, name=b'BB', dr_len=36, extent=None, rr=False, hidden=False)

def check_onefileonedir(iso, filesize):
    assert(filesize == 53248)

    internal_check_pvd(iso.pvd, extent=16, size=26, ptbl_size=22, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=23, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    dir1_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(dir1_record.ptr, name=b'DIR1', len_di=4, loc=24, parent=1)
    internal_check_empty_directory(dir1_record, name=b'DIR1', dr_len=38, extent=24, rr=False, hidden=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'FOO.;1', dr_len=40, loc=25, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

def check_onefile_onedirwithfile(iso, filesize):
    assert(filesize == 55296)

    internal_check_pvd(iso.pvd, extent=16, size=27, ptbl_size=22, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=23, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    dir1_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(dir1_record.ptr, name=b'DIR1', len_di=4, loc=24, parent=1)
    internal_check_dir_record(dir1_record, num_children=3, name=b'DIR1', dr_len=38, extent_location=24, rr=False, rr_name=None, rr_links=0, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dir1_record.children[1], rr=False, rr_nlinks=3, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'FOO.;1', dr_len=40, loc=25, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

    internal_check_file(dir1_record.children[2], name=b'BAR.;1', dr_len=40, loc=26, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/DIR1/BAR.;1', contents=b'bar\n', which='iso_path')

def check_twoextentfile(iso, filesize):
    assert(filesize == 53248)

    internal_check_pvd(iso.pvd, extent=16, size=26, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=23, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    # Now check the file at the root.  It should have a name of BIGFILE.;1, it
    # should have a directory record length of 44, it should start at extent 24,
    # and its contents should be the bytes 0x0-0xff, repeating 8 times plus one.
    outstr = b''
    for j in range(0, 8):
        for i in range(0, 256):
            outstr += struct.pack('=B', i)
    outstr += struct.pack('=B', 0)
    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BIGFILE.;1', dr_len=44, loc=24, datalen=2049, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BIGFILE.;1', contents=outstr, which='iso_path')

def check_twoleveldeepdir(iso, filesize):
    assert(filesize == 53248)

    internal_check_pvd(iso.pvd, extent=16, size=26, ptbl_size=38, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=23, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    dir1_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(dir1_record.ptr, name=b'DIR1', len_di=4, loc=24, parent=1)
    internal_check_dir_record(dir1_record, num_children=3, name=b'DIR1', dr_len=38, extent_location=24, rr=False, rr_name=None, rr_links=0, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dir1_record.children[1], rr=False, rr_nlinks=3, xa=False, rr_onetwelve=False)

    subdir1_record = dir1_record.children[2]
    internal_check_ptr(subdir1_record.ptr, name=b'SUBDIR1', len_di=7, loc=25, parent=2)
    internal_check_empty_directory(subdir1_record, name=b'SUBDIR1', dr_len=40, extent=25, rr=False, hidden=False)

def check_tendirs(iso, filesize):
    assert(filesize == 69632)

    internal_check_pvd(iso.pvd, extent=16, size=34, ptbl_size=132, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=12, data_length=2048, extent_location=23, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)
    # The rest of the path table records will be checked by the loop below.

    names = internal_generate_inorder_names(10)
    for index in range(2, 2+10):
        dir_record = iso.pvd.root_dir_record.children[index]
        # We skip checking the path table record extent locations because
        # genisoimage seems to have a bug assigning the extent locations, and
        # seems to assign them in reverse order.
        internal_check_ptr(dir_record.ptr, name=names[index], len_di=len(names[index]), loc=None, parent=1)

        internal_check_empty_directory(dir_record, name=names[index], dr_len=38, extent=None, rr=False, hidden=False)

def check_dirs_overflow_ptr_extent(iso, filesize):
    assert(filesize == 671744)

    internal_check_pvd(iso.pvd, extent=16, size=328, ptbl_size=4122, ptbl_location_le=19, ptbl_location_be=23)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=297, data_length=12288, extent_location=27, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=27, parent=1)
    # The rest of the path table records will be checked by the loop below.

    names = internal_generate_inorder_names(295)
    for index in range(2, 2+295):
        dir_record = iso.pvd.root_dir_record.children[index]
        # We skip checking the path table record extent locations because
        # genisoimage seems to have a bug assigning the extent locations, and
        # seems to assign them in reverse order.
        internal_check_ptr(dir_record.ptr, name=names[index], len_di=len(names[index]), loc=None, parent=1)

        internal_check_empty_directory(dir_record, name=names[index], dr_len=33 + len(names[index]) + (1 - (len(names[index]) % 2)), extent=None, rr=False, hidden=False)

def check_dirs_just_short_ptr_extent(iso, filesize):
    assert(filesize == 659456)

    internal_check_pvd(iso.pvd, extent=16, size=322, ptbl_size=4094, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=295, data_length=12288, extent_location=23, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)
    # The rest of the path table records will be checked by the loop below.

    names = internal_generate_inorder_names(293)
    for index in range(2, 2+293):
        dir_record = iso.pvd.root_dir_record.children[index]
        # We skip checking the path table record extent locations because
        # genisoimage seems to have a bug assigning the extent locations, and
        # seems to assign them in reverse order.
        internal_check_ptr(dir_record.ptr, name=names[index], len_di=len(names[index]), loc=None, parent=1)

        internal_check_empty_directory(dir_record, name=names[index], dr_len=33 + len(names[index]) + (1 - (len(names[index]) % 2)), extent=None, rr=False, hidden=False)

def check_twoleveldeepfile(iso, filesize):
    assert(filesize == 55296)

    internal_check_pvd(iso.pvd, extent=16, size=27, ptbl_size=38, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=23, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    dir1_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(dir1_record.ptr, name=b'DIR1', len_di=4, loc=24, parent=1)
    internal_check_dir_record(dir1_record, num_children=3, name=b'DIR1', dr_len=38, extent_location=24, rr=False, rr_name=None, rr_links=0, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dir1_record.children[1], rr=False, rr_nlinks=3, xa=False, rr_onetwelve=False)

    subdir1_record = dir1_record.children[2]
    internal_check_ptr(subdir1_record.ptr, name=b'SUBDIR1', len_di=7, loc=25, parent=2)
    internal_check_dir_record(subdir1_record, num_children=3, name=b'SUBDIR1', dr_len=40, extent_location=25, rr=False, rr_name=None, rr_links=0, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(subdir1_record.children[1], rr=False, rr_nlinks=3, xa=False, rr_onetwelve=False)

    internal_check_file(subdir1_record.children[2], name=b'FOO.;1', dr_len=40, loc=26, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/DIR1/SUBDIR1/FOO.;1', contents=b'foo\n', which='iso_path')

def check_joliet_nofiles(iso, filesize):
    assert(filesize == 61440)

    internal_check_pvd(iso.pvd, extent=16, size=30, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_jolietvd(iso.svds[0], space_size=30, path_tbl_size=10, path_tbl_loc_le=24, path_tbl_loc_be=26)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=28, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=29, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=2, data_length=2048, extent_location=28, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=2, data_length=2048, extent_location=29)

    assert(not iso.has_rock_ridge())
    assert(iso.has_joliet())
    assert(not iso.has_udf())

def check_joliet_onedir(iso, filesize):
    assert(filesize == 65536)

    internal_check_pvd(iso.pvd, extent=16, size=32, ptbl_size=22, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_jolietvd(iso.svds[0], space_size=32, path_tbl_size=26, path_tbl_loc_le=24, path_tbl_loc_be=26)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=28, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=30, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=28, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    dir1_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(dir1_record.ptr, name=b'DIR1', len_di=4, loc=29, parent=1)
    internal_check_empty_directory(dir1_record, name=b'DIR1', dr_len=38, extent=29, rr=False, hidden=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=3, data_length=2048, extent_location=30)

    joliet_dir1_record = iso.joliet_vd.root_dir_record.children[2]
    internal_check_ptr(joliet_dir1_record.ptr, name='dir1'.encode('utf-16_be'), len_di=8, loc=31, parent=1)
    internal_check_empty_directory(joliet_dir1_record, name='dir1'.encode('utf-16_be'), dr_len=42, extent=31, rr=False, hidden=False)

def check_joliet_onefile(iso, filesize):
    assert(filesize == 63488)

    internal_check_pvd(iso.pvd, extent=16, size=31, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_jolietvd(iso.svds[0], space_size=31, path_tbl_size=10, path_tbl_loc_le=24, path_tbl_loc_be=26)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=28, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=29, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=28, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=3, data_length=2048, extent_location=29)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'FOO.;1', dr_len=40, loc=30, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

    internal_check_file(iso.joliet_vd.root_dir_record.children[2], name='foo'.encode('utf-16_be'), dr_len=40, loc=30, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/foo', contents=b'foo\n', which='joliet_path')

def check_joliet_onefileonedir(iso, filesize):
    assert(filesize == 67584)

    internal_check_pvd(iso.pvd, extent=16, size=33, ptbl_size=22, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_jolietvd(iso.svds[0], space_size=33, path_tbl_size=26, path_tbl_loc_le=24, path_tbl_loc_be=26)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=28, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=30, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=28, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    dir1_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(dir1_record.ptr, name=b'DIR1', len_di=4, loc=29, parent=1)
    internal_check_empty_directory(dir1_record, name=b'DIR1', dr_len=38, extent=29, rr=False, hidden=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=4, data_length=2048, extent_location=30)

    joliet_dir1_record = iso.joliet_vd.root_dir_record.children[2]
    internal_check_ptr(joliet_dir1_record.ptr, name='dir1'.encode('utf-16_be'), len_di=8, loc=31, parent=1)
    internal_check_empty_directory(joliet_dir1_record, name='dir1'.encode('utf-16_be'), dr_len=42, extent=31, rr=False, hidden=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'FOO.;1', dr_len=40, loc=32, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

    internal_check_file(iso.joliet_vd.root_dir_record.children[3], name='foo'.encode('utf-16_be'), dr_len=40, loc=32, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/foo', contents=b'foo\n', which='joliet_path')

def check_eltorito_nofiles(iso, filesize):
    assert(filesize == 55296)

    internal_check_pvd(iso.pvd, extent=16, size=27, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_eltorito(iso, boot_catalog_extent=25, load_rba=26, media_type=0, system_type=0, bootable=True, platform_id=0)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=24, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=24, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'BOOT.CAT;1', dr_len=44, loc=25, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BOOT.;1', dr_len=40, loc=26, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BOOT.;1', contents=b'boot\n', which='iso_path')

def check_eltorito_twofile(iso, filesize):
    assert(filesize == 57344)

    internal_check_pvd(iso.pvd, extent=16, size=28, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_eltorito(iso, boot_catalog_extent=25, load_rba=26, media_type=0, system_type=0, bootable=True, platform_id=0)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=24, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=5, data_length=2048, extent_location=24, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[4], name=b'BOOT.CAT;1', dr_len=44, loc=25, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'BOOT.;1', dr_len=40, loc=26, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BOOT.;1', contents=b'boot\n', which='iso_path')

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'AA.;1', dr_len=38, loc=27, datalen=3, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/AA.;1', contents=b'aa\n', which='iso_path')

def check_rr_nofiles(iso, filesize):
    assert(filesize == 51200)

    internal_check_pvd(iso.pvd, extent=16, size=25, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=2, data_length=2048, extent_location=23, rr=True, rr_nlinks=2, xa=False, rr_onetwelve=False)

    assert(iso.has_rock_ridge())
    assert(not iso.has_joliet())
    assert(not iso.has_udf())

def check_rr_onefile(iso, filesize):
    assert(filesize == 53248)

    internal_check_pvd(iso.pvd, extent=16, size=26, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=23, rr=True, rr_nlinks=2, xa=False, rr_onetwelve=False)

    foo_dir_record = iso.pvd.root_dir_record.children[2]
    internal_check_file(foo_dir_record, name=b'FOO.;1', dr_len=116, loc=25, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

    internal_check_rr_file(foo_dir_record, name=b'foo')
    internal_check_file_contents(iso, path='/foo', contents=b'foo\n', which='rr_path')

def check_rr_twofile(iso, filesize):
    assert(filesize == 55296)

    internal_check_pvd(iso.pvd, extent=16, size=27, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=23, rr=True, rr_nlinks=2, xa=False, rr_onetwelve=False)

    bar_dir_record = iso.pvd.root_dir_record.children[2]
    internal_check_file(bar_dir_record, name=b'BAR.;1', dr_len=116, loc=25, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BAR.;1', contents=b'bar\n', which='iso_path')

    internal_check_rr_file(bar_dir_record, name=b'bar')
    internal_check_file_contents(iso, path='/bar', contents=b'bar\n', which='rr_path')

    foo_dir_record = iso.pvd.root_dir_record.children[3]
    internal_check_file(foo_dir_record, name=b'FOO.;1', dr_len=116, loc=26, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

    internal_check_rr_file(foo_dir_record, name=b'foo')
    internal_check_file_contents(iso, path='/foo', contents=b'foo\n', which='rr_path')

def check_rr_onefileonedir(iso, filesize):
    assert(filesize == 55296)

    internal_check_pvd(iso.pvd, extent=16, size=27, ptbl_size=22, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=23, rr=True, rr_nlinks=3, xa=False, rr_onetwelve=False)

    dir1_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(dir1_record.ptr, name=b'DIR1', len_di=4, loc=24, parent=1)
    internal_check_empty_directory(dir1_record, name=b'DIR1', dr_len=114, extent=24, rr=True, hidden=False)

    foo_dir_record = iso.pvd.root_dir_record.children[3]
    internal_check_file(foo_dir_record, name=b'FOO.;1', dr_len=116, loc=26, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

    internal_check_rr_file(foo_dir_record, name=b'foo')
    internal_check_file_contents(iso, path='/foo', contents=b'foo\n', which='rr_path')

def check_rr_onefileonedirwithfile(iso, filesize):
    assert(filesize == 57344)

    internal_check_pvd(iso.pvd, extent=16, size=28, ptbl_size=22, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=23, rr=True, rr_nlinks=3, xa=False, rr_onetwelve=False)

    dir1_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(dir1_record.ptr, name=b'DIR1', len_di=4, loc=24, parent=1)
    internal_check_dir_record(dir1_record, num_children=3, name=b'DIR1', dr_len=114, extent_location=24, rr=True, rr_name=b'dir1', rr_links=2, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dir1_record.children[1], rr=True, rr_nlinks=3, xa=False, rr_onetwelve=False)

    foo_dir_record = iso.pvd.root_dir_record.children[3]
    internal_check_file(foo_dir_record, name=b'FOO.;1', dr_len=116, loc=26, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

    internal_check_rr_file(foo_dir_record, name=b'foo')
    internal_check_file_contents(iso, path='/foo', contents=b'foo\n', which='rr_path')

    bar_dir_record = dir1_record.children[2]
    internal_check_file(bar_dir_record, name=b'BAR.;1', dr_len=116, loc=27, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/DIR1/BAR.;1', contents=b'bar\n', which='iso_path')

    internal_check_rr_file(bar_dir_record, name=b'bar')
    internal_check_file_contents(iso, path='/dir1/bar', contents=b'bar\n', which='rr_path')

def check_rr_symlink(iso, filesize):
    assert(filesize == 53248)

    internal_check_pvd(iso.pvd, extent=16, size=26, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=23, rr=True, rr_nlinks=2, xa=False, rr_onetwelve=False)

    foo_dir_record = iso.pvd.root_dir_record.children[2]
    internal_check_file(foo_dir_record, name=b'FOO.;1', dr_len=116, loc=25, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

    internal_check_rr_file(foo_dir_record, name=b'foo')
    internal_check_file_contents(iso, path='/foo', contents=b'foo\n', which='rr_path')

    # Now check the rock ridge symlink.  It should have a directory record
    # length of 126, and the symlink components should be 'foo'.
    sym_dir_record = iso.pvd.root_dir_record.children[3]
    internal_check_rr_symlink(sym_dir_record, name=b'SYM.;1', dr_len=126, comps=[b'foo'])

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibException):
        internal_check_file_contents(iso, path='/sym', contents=b'foo\n', which='iso_path')

def check_rr_symlink2(iso, filesize):
    assert(filesize == 55296)

    internal_check_pvd(iso.pvd, extent=16, size=27, ptbl_size=22, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=23, rr=True, rr_nlinks=3, xa=False, rr_onetwelve=False)

    dir1_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(dir1_record.ptr, name=b'DIR1', len_di=4, loc=24, parent=1)
    internal_check_dir_record(dir1_record, num_children=3, name=b'DIR1', dr_len=114, extent_location=24, rr=True, rr_name=b'dir1', rr_links=2, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dir1_record.children[1], rr=True, rr_nlinks=3, xa=False, rr_onetwelve=False)

    foo_dir_record = dir1_record.children[2]
    internal_check_file(foo_dir_record, name=b'FOO.;1', dr_len=116, loc=26, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/DIR1/FOO.;1', contents=b'foo\n', which='iso_path')

    internal_check_rr_file(foo_dir_record, name=b'foo')
    internal_check_file_contents(iso, path='/dir1/foo', contents=b'foo\n', which='rr_path')

    # Now check the rock ridge symlink.  It should have a directory record
    # length of 132, and the symlink components should be 'dir1' and 'foo'.
    sym_dir_record = iso.pvd.root_dir_record.children[3]
    internal_check_rr_symlink(sym_dir_record, name=b'SYM.;1', dr_len=132, comps=[b'dir1', b'foo'])

def check_rr_symlink_dot(iso, filesize):
    assert(filesize == 51200)

    internal_check_pvd(iso.pvd, extent=16, size=25, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=23, rr=True, rr_nlinks=2, xa=False, rr_onetwelve=False)

    # Now check the rock ridge symlink.  It should have a directory record
    # length of 132, and the symlink components should be 'dir1' and 'foo'.
    sym_dir_record = iso.pvd.root_dir_record.children[2]
    internal_check_rr_symlink(sym_dir_record, name=b'SYM.;1', dr_len=122, comps=[b'.'])

def check_rr_symlink_dotdot(iso, filesize):
    assert(filesize == 51200)

    internal_check_pvd(iso.pvd, extent=16, size=25, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=23, rr=True, rr_nlinks=2, xa=False, rr_onetwelve=False)

    # Now check the rock ridge symlink.  It should have a directory record
    # length of 132, and the symlink components should be 'dir1' and 'foo'.
    sym_dir_record = iso.pvd.root_dir_record.children[2]
    internal_check_rr_symlink(sym_dir_record, name=b'SYM.;1', dr_len=122, comps=[b'..'])

def check_rr_symlink_broken(iso, filesize):
    assert(filesize == 51200)

    internal_check_pvd(iso.pvd, extent=16, size=25, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=23, rr=True, rr_nlinks=2, xa=False, rr_onetwelve=False)

    # Now check the rock ridge symlink.  It should have a directory record
    # length of 132, and the symlink components should be 'dir1' and 'foo'.
    sym_dir_record = iso.pvd.root_dir_record.children[2]
    internal_check_rr_symlink(sym_dir_record, name=b'SYM.;1', dr_len=126, comps=[b'foo'])

def check_alternating_subdir(iso, filesize):
    assert(filesize == 61440)

    internal_check_pvd(iso.pvd, extent=16, size=30, ptbl_size=30, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=6, data_length=2048, extent_location=23, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    aa_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(aa_record.ptr, name=b'AA', len_di=2, loc=None, parent=1)
    internal_check_dir_record(aa_record, num_children=3, name=b'AA', dr_len=36, extent_location=None, rr=False, rr_name=None, rr_links=0, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(aa_record.children[1], rr=False, rr_nlinks=3, xa=False, rr_onetwelve=False)

    bb_dir_record = iso.pvd.root_dir_record.children[3]
    internal_check_file(bb_dir_record, name=b'BB.;1', dr_len=38, loc=26, datalen=3, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BB.;1', contents=b'bb\n', which='iso_path')

    cc_record = iso.pvd.root_dir_record.children[4]
    internal_check_ptr(cc_record.ptr, name=b'CC', len_di=2, loc=None, parent=1)
    internal_check_dir_record(cc_record, num_children=3, name=b'CC', dr_len=36, extent_location=None, rr=False, rr_name=None, rr_links=0, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(cc_record.children[1], rr=False, rr_nlinks=3, xa=False, rr_onetwelve=False)

    dd_dir_record = iso.pvd.root_dir_record.children[5]
    internal_check_file(dd_dir_record, name=b'DD.;1', dr_len=38, loc=27, datalen=3, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/DD.;1', contents=b'dd\n', which='iso_path')

    sub1_dir_record = aa_record.children[2]
    internal_check_file(sub1_dir_record, name=b'SUB1.;1', dr_len=40, loc=None, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/AA/SUB1.;1', contents=b'sub1\n', which='iso_path')

    sub2_dir_record = cc_record.children[2]
    internal_check_file(sub2_dir_record, name=b'SUB2.;1', dr_len=40, loc=None, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/CC/SUB2.;1', contents=b'sub2\n', which='iso_path')

def check_rr_verylongname(iso, filesize):
    assert(filesize == 55296)

    internal_check_pvd(iso.pvd, extent=16, size=27, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=23, rr=True, rr_nlinks=2, xa=False, rr_onetwelve=False)

    internal_check_rr_longname(iso, dir_record=iso.pvd.root_dir_record.children[2], extent=26, letter=b'a')

def check_rr_verylongname_joliet(iso, filesize):
    assert(filesize == 67584)

    internal_check_pvd(iso.pvd, extent=16, size=33, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_jolietvd(iso.svds[0], space_size=33, path_tbl_size=10, path_tbl_loc_le=24, path_tbl_loc_be=26)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=28, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=30, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=28, rr=True, rr_nlinks=2, xa=False, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=3, data_length=2048, extent_location=30)

    internal_check_rr_longname(iso, dir_record=iso.pvd.root_dir_record.children[2], extent=32, letter=b'a')

    internal_check_file(iso.joliet_vd.root_dir_record.children[2], name=('a'*64).encode('utf-16_be'), dr_len=162, loc=32, datalen=3, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/'+'a'*64, contents=b'aa\n', which='joliet_path')

def check_rr_manylongname(iso, filesize):
    assert(filesize == 67584)

    internal_check_pvd(iso.pvd, extent=16, size=33, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=9, data_length=2048, extent_location=23, rr=True, rr_nlinks=2, xa=False, rr_onetwelve=False)

    aa_dir_record = iso.pvd.root_dir_record.children[2]
    internal_check_rr_longname(iso, dir_record=aa_dir_record, extent=26, letter=b'a')

    bb_dir_record = iso.pvd.root_dir_record.children[3]
    internal_check_rr_longname(iso, dir_record=bb_dir_record, extent=27, letter=b'b')

    cc_dir_record = iso.pvd.root_dir_record.children[4]
    internal_check_rr_longname(iso, dir_record=cc_dir_record, extent=28, letter=b'c')

    dd_dir_record = iso.pvd.root_dir_record.children[5]
    internal_check_rr_longname(iso, dir_record=dd_dir_record, extent=29, letter=b'd')

    ee_dir_record = iso.pvd.root_dir_record.children[6]
    internal_check_rr_longname(iso, dir_record=ee_dir_record, extent=30, letter=b'e')

    ff_dir_record = iso.pvd.root_dir_record.children[7]
    internal_check_rr_longname(iso, dir_record=ff_dir_record, extent=31, letter=b'f')

    gg_dir_record = iso.pvd.root_dir_record.children[8]
    internal_check_rr_longname(iso, dir_record=gg_dir_record, extent=32, letter=b'g')

def check_rr_manylongname2(iso, filesize):
    assert(filesize == 71680)

    internal_check_pvd(iso.pvd, extent=16, size=35, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=10, data_length=4096, extent_location=23, rr=True, rr_nlinks=2, xa=False, rr_onetwelve=False)

    aa_dir_record = iso.pvd.root_dir_record.children[2]
    internal_check_rr_longname(iso, dir_record=aa_dir_record, extent=27, letter=b'a')

    bb_dir_record = iso.pvd.root_dir_record.children[3]
    internal_check_rr_longname(iso, dir_record=bb_dir_record, extent=28, letter=b'b')

    cc_dir_record = iso.pvd.root_dir_record.children[4]
    internal_check_rr_longname(iso, dir_record=cc_dir_record, extent=29, letter=b'c')

    dd_dir_record = iso.pvd.root_dir_record.children[5]
    internal_check_rr_longname(iso, dir_record=dd_dir_record, extent=30, letter=b'd')

    ee_dir_record = iso.pvd.root_dir_record.children[6]
    internal_check_rr_longname(iso, dir_record=ee_dir_record, extent=31, letter=b'e')

    ff_dir_record = iso.pvd.root_dir_record.children[7]
    internal_check_rr_longname(iso, dir_record=ff_dir_record, extent=32, letter=b'f')

    gg_dir_record = iso.pvd.root_dir_record.children[8]
    internal_check_rr_longname(iso, dir_record=gg_dir_record, extent=33, letter=b'g')

    hh_dir_record = iso.pvd.root_dir_record.children[9]
    internal_check_rr_longname(iso, dir_record=hh_dir_record, extent=34, letter=b'h')

def check_rr_verylongnameandsymlink(iso, filesize):
    assert(filesize == 55296)

    internal_check_pvd(iso.pvd, extent=16, size=27, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=23, rr=True, rr_nlinks=2, xa=False, rr_onetwelve=False)

    internal_check_rr_longname(iso, dir_record=iso.pvd.root_dir_record.children[2], extent=26, letter=b'a')

def check_joliet_and_rr_nofiles(iso, filesize):
    assert(filesize == 63488)

    internal_check_pvd(iso.pvd, extent=16, size=31, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_jolietvd(iso.svds[0], space_size=31, path_tbl_size=10, path_tbl_loc_le=24, path_tbl_loc_be=26)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=28, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=29, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=2, data_length=2048, extent_location=28, rr=True, rr_nlinks=2, xa=False, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=2, data_length=2048, extent_location=29)

def check_joliet_and_rr_onefile(iso, filesize):
    assert(filesize == 65536)

    internal_check_pvd(iso.pvd, extent=16, size=32, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_jolietvd(iso.svds[0], space_size=32, path_tbl_size=10, path_tbl_loc_le=24, path_tbl_loc_be=26)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=28, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=29, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=28, rr=True, rr_nlinks=2, xa=False, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=3, data_length=2048, extent_location=29)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'FOO.;1', dr_len=116, loc=31, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

    internal_check_file(iso.joliet_vd.root_dir_record.children[2], name='foo'.encode('utf-16_be'), dr_len=40, loc=31, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/foo', contents=b'foo\n', which='joliet_path')

def check_joliet_and_rr_onedir(iso, filesize):
    assert(filesize == 67584)

    internal_check_pvd(iso.pvd, extent=16, size=33, ptbl_size=22, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_jolietvd(iso.svds[0], space_size=33, path_tbl_size=26, path_tbl_loc_le=24, path_tbl_loc_be=26)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=28, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=30, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=28, rr=True, rr_nlinks=3, xa=False, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=3, data_length=2048, extent_location=30)

    dir1_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(dir1_record.ptr, name=b'DIR1', len_di=4, loc=29, parent=1)
    internal_check_dir_record(dir1_record, num_children=2, name=b'DIR1', dr_len=114, extent_location=29, rr=True, rr_name=b'dir1', rr_links=2, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dir1_record.children[1], rr=True, rr_nlinks=3, xa=False, rr_onetwelve=False)

    joliet_dir1_record = iso.joliet_vd.root_dir_record.children[2]
    internal_check_ptr(joliet_dir1_record.ptr, name='dir1'.encode('utf-16_be'), len_di=8, loc=31, parent=1)
    internal_check_dir_record(joliet_dir1_record, num_children=2, name='dir1'.encode('utf-16_be'), dr_len=42, extent_location=31, rr=False, rr_name=None, rr_links=0, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(joliet_dir1_record.children[1], rr=False, rr_nlinks=3, xa=False, rr_onetwelve=False)

def check_rr_and_eltorito_nofiles(iso, filesize):
    assert(filesize == 57344)

    internal_check_pvd(iso.pvd, extent=16, size=28, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_eltorito(iso, boot_catalog_extent=26, load_rba=27, media_type=0, system_type=0, bootable=True, platform_id=0)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=24, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=24, rr=True, rr_nlinks=2, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'BOOT.CAT;1', dr_len=124, loc=26, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BOOT.;1', dr_len=116, loc=27, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BOOT.;1', contents=b'boot\n', which='iso_path')

def check_rr_and_eltorito_onefile(iso, filesize):
    assert(filesize == 59392)

    internal_check_pvd(iso.pvd, extent=16, size=29, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_eltorito(iso, boot_catalog_extent=26, load_rba=27, media_type=0, system_type=0, bootable=True, platform_id=0)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=24, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=5, data_length=2048, extent_location=24, rr=True, rr_nlinks=2, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'BOOT.CAT;1', dr_len=124, loc=26, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BOOT.;1', dr_len=116, loc=27, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BOOT.;1', contents=b'boot\n', which='iso_path')

    internal_check_file(iso.pvd.root_dir_record.children[4], name=b'FOO.;1', dr_len=116, loc=28, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

def check_rr_and_eltorito_onedir(iso, filesize):
    assert(filesize == 59392)

    internal_check_pvd(iso.pvd, extent=16, size=29, ptbl_size=22, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_eltorito(iso, boot_catalog_extent=27, load_rba=28, media_type=0, system_type=0, bootable=True, platform_id=0)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=24, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=5, data_length=2048, extent_location=24, rr=True, rr_nlinks=3, xa=False, rr_onetwelve=False)

    dir1_record = iso.pvd.root_dir_record.children[4]
    internal_check_ptr(dir1_record.ptr, name=b'DIR1', len_di=4, loc=25, parent=1)
    internal_check_dir_record(dir1_record, num_children=2, name=b'DIR1', dr_len=114, extent_location=25, rr=True, rr_name=b'dir1', rr_links=2, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dir1_record.children[1], rr=True, rr_nlinks=3, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'BOOT.CAT;1', dr_len=124, loc=27, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BOOT.;1', dr_len=116, loc=28, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BOOT.;1', contents=b'boot\n', which='iso_path')

def check_joliet_and_eltorito_nofiles(iso, filesize):
    assert(filesize == 67584)

    internal_check_pvd(iso.pvd, extent=16, size=33, ptbl_size=10, ptbl_location_le=21, ptbl_location_be=23)

    internal_check_jolietvd(iso.svds[0], space_size=33, path_tbl_size=10, path_tbl_loc_le=25, path_tbl_loc_be=27)

    internal_check_eltorito(iso, boot_catalog_extent=31, load_rba=32, media_type=0, system_type=0, bootable=True, platform_id=0)

    internal_check_terminator(iso.vdsts, extent=19)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=29, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=30, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=29, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=4, data_length=2048, extent_location=30)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'BOOT.CAT;1', dr_len=44, loc=31, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BOOT.;1', dr_len=40, loc=32, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BOOT.;1', contents=b'boot\n', which='iso_path')

    internal_check_file(iso.joliet_vd.root_dir_record.children[3], name='boot.cat'.encode('utf-16_be'), dr_len=50, loc=31, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.joliet_vd.root_dir_record.children[2], name='boot'.encode('utf-16_be'), dr_len=42, loc=32, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/boot', contents=b'boot\n', which='joliet_path')

def check_isohybrid(iso, filesize):
    assert(filesize == 1048576)

    internal_check_pvd(iso.pvd, extent=16, size=27, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_eltorito(iso, boot_catalog_extent=25, load_rba=26, media_type=0, system_type=0, bootable=True, platform_id=0)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=24, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=24, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BOOT.CAT;1', dr_len=44, loc=25, datalen=2048, hidden=False, multi_extent=False)

    # Now check out the isohybrid stuff.
    assert(iso.isohybrid_mbr.geometry_heads == 64)
    assert(iso.isohybrid_mbr.geometry_sectors == 32)
    assert(iso.isohybrid_mbr.bhead == 0)
    assert(iso.isohybrid_mbr.bsect == 1)
    assert(iso.isohybrid_mbr.bcyle == 0)
    assert(iso.isohybrid_mbr.ptype == 23)
    assert(iso.isohybrid_mbr.ehead == 63)
    assert(iso.isohybrid_mbr.part_offset == 0)
    assert(not iso.isohybrid_mbr.efi)
    assert(not iso.isohybrid_mbr.mac)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'ISOLINUX.BIN;1', dr_len=48, loc=26, datalen=68, hidden=False, multi_extent=False)

def check_isohybrid_uefi(iso, filesize):
    assert(filesize == 1048576)

    internal_check_pvd(iso.pvd, extent=16, size=28, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_eltorito(iso, boot_catalog_extent=25, load_rba=None, media_type=0, system_type=0, bootable=True, platform_id=0)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=24, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=5, data_length=2048, extent_location=24, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BOOT.CAT;1', dr_len=44, loc=25, datalen=2048, hidden=False, multi_extent=False)

    # Now check out the isohybrid stuff.
    assert(iso.isohybrid_mbr.geometry_heads == 64)
    assert(iso.isohybrid_mbr.geometry_sectors == 32)
    assert(iso.isohybrid_mbr.bhead == 0)
    assert(iso.isohybrid_mbr.bsect == 1)
    assert(iso.isohybrid_mbr.bcyle == 0)
    assert(iso.isohybrid_mbr.ptype == 0)
    assert(iso.isohybrid_mbr.ehead == 63)
    assert(iso.isohybrid_mbr.part_offset == 0)
    assert(iso.isohybrid_mbr.efi)
    assert(iso.isohybrid_mbr.efi_lba == 26)
    assert(iso.isohybrid_mbr.efi_count == 4)
    assert(iso.isohybrid_mbr.primary_gpt.is_primary)
    assert(iso.isohybrid_mbr.primary_gpt.header.current_lba == 1)
    assert(iso.isohybrid_mbr.primary_gpt.header.backup_lba == 2047)
    assert(iso.isohybrid_mbr.primary_gpt.header.first_usable_lba == 34)
    assert(iso.isohybrid_mbr.primary_gpt.header.last_usable_lba == 2014)
    assert(iso.isohybrid_mbr.primary_gpt.header.partition_entries_lba == 2)
    assert(iso.isohybrid_mbr.primary_gpt.header.num_parts == 128)
    assert(iso.isohybrid_mbr.primary_gpt.header.size_of_partition_entries == 128)
    assert(len(iso.isohybrid_mbr.primary_gpt.parts) == 2)
    assert(iso.isohybrid_mbr.primary_gpt.parts[0].first_lba == 0)
    assert(iso.isohybrid_mbr.primary_gpt.parts[0].last_lba == 111)
    assert(iso.isohybrid_mbr.primary_gpt.parts[0].attributes == b'\x00'*8)
    assert(iso.isohybrid_mbr.primary_gpt.parts[0].name == 'ISOHybrid ISO')
    assert(iso.isohybrid_mbr.primary_gpt.parts[1].first_lba == 104)
    assert(iso.isohybrid_mbr.primary_gpt.parts[1].last_lba == 107)
    assert(iso.isohybrid_mbr.primary_gpt.parts[1].attributes == b'\x00'*8)
    assert(iso.isohybrid_mbr.primary_gpt.parts[1].name == 'ISOHybrid')
    assert(not iso.isohybrid_mbr.mac)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'EFIBOOT.IMG;1', dr_len=46, loc=None, datalen=1, hidden=False, multi_extent=False)

def check_isohybrid_mac_uefi(iso, filesize):
    assert(filesize == 1048576)

    internal_check_pvd(iso.pvd, extent=16, size=29, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_eltorito(iso, boot_catalog_extent=25, load_rba=None, media_type=0, system_type=0, bootable=True, platform_id=0)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=24, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=6, data_length=2048, extent_location=24, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BOOT.CAT;1', dr_len=44, loc=25, datalen=2048, hidden=False, multi_extent=False)

    # Now check out the isohybrid stuff.
    assert(iso.isohybrid_mbr.geometry_heads == 64)
    assert(iso.isohybrid_mbr.geometry_sectors == 32)
    assert(iso.isohybrid_mbr.bhead == 0)
    assert(iso.isohybrid_mbr.bsect == 1)
    assert(iso.isohybrid_mbr.bcyle == 0)
    assert(iso.isohybrid_mbr.ptype == 0)
    assert(iso.isohybrid_mbr.ehead == 63)
    assert(iso.isohybrid_mbr.part_offset == 0)
    assert(iso.isohybrid_mbr.efi)
    assert(iso.isohybrid_mbr.efi_lba == 26)
    assert(iso.isohybrid_mbr.efi_count == 4)
    assert(iso.isohybrid_mbr.primary_gpt.is_primary)
    assert(iso.isohybrid_mbr.primary_gpt.header.current_lba == 1)
    assert(iso.isohybrid_mbr.primary_gpt.header.backup_lba == 2047)
    assert(iso.isohybrid_mbr.primary_gpt.header.first_usable_lba == 48)
    assert(iso.isohybrid_mbr.primary_gpt.header.last_usable_lba == 2014)
    assert(iso.isohybrid_mbr.primary_gpt.header.partition_entries_lba == 16)
    assert(iso.isohybrid_mbr.primary_gpt.header.num_parts == 128)
    assert(iso.isohybrid_mbr.primary_gpt.header.size_of_partition_entries == 128)
    assert(len(iso.isohybrid_mbr.primary_gpt.parts) == 3)
    assert(iso.isohybrid_mbr.primary_gpt.parts[0].first_lba == 0)
    assert(iso.isohybrid_mbr.primary_gpt.parts[0].last_lba == 115)
    assert(iso.isohybrid_mbr.primary_gpt.parts[0].attributes == b'\x00'*8)
    assert(iso.isohybrid_mbr.primary_gpt.parts[0].name == 'ISOHybrid ISO')
    assert(iso.isohybrid_mbr.primary_gpt.parts[1].first_lba == 104)
    assert(iso.isohybrid_mbr.primary_gpt.parts[1].last_lba == 107)
    assert(iso.isohybrid_mbr.primary_gpt.parts[1].attributes == b'\x00'*8)
    assert(iso.isohybrid_mbr.primary_gpt.parts[1].name == 'ISOHybrid')
    assert(iso.isohybrid_mbr.primary_gpt.parts[2].first_lba == 112)
    assert(iso.isohybrid_mbr.primary_gpt.parts[2].last_lba == 115)
    assert(iso.isohybrid_mbr.primary_gpt.parts[2].attributes == b'\x00'*8)
    assert(iso.isohybrid_mbr.primary_gpt.parts[2].name == 'ISOHybrid')
    assert(iso.isohybrid_mbr.mac)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'EFIBOOT.IMG;1', dr_len=46, loc=None, datalen=1, hidden=False, multi_extent=False)

def check_joliet_and_eltorito_onefile(iso, filesize):
    assert(filesize == 69632)

    internal_check_pvd(iso.pvd, extent=16, size=34, ptbl_size=10, ptbl_location_le=21, ptbl_location_be=23)

    internal_check_jolietvd(iso.svds[0], space_size=34, path_tbl_size=10, path_tbl_loc_le=25, path_tbl_loc_be=27)

    internal_check_eltorito(iso, boot_catalog_extent=31, load_rba=32, media_type=0, system_type=0, bootable=True, platform_id=0)

    internal_check_terminator(iso.vdsts, extent=19)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=29, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=30, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=5, data_length=2048, extent_location=29, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=5, data_length=2048, extent_location=30)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'BOOT.CAT;1', dr_len=44, loc=31, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BOOT.;1', dr_len=40, loc=32, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BOOT.;1', contents=b'boot\n', which='iso_path')

    internal_check_file(iso.pvd.root_dir_record.children[4], name=b'FOO.;1', dr_len=40, loc=33, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

    internal_check_file(iso.joliet_vd.root_dir_record.children[2], name='boot'.encode('utf-16_be'), dr_len=42, loc=32, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/boot', contents=b'boot\n', which='joliet_path')

    internal_check_file(iso.joliet_vd.root_dir_record.children[4], name='foo'.encode('utf-16_be'), dr_len=40, loc=33, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/foo', contents=b'foo\n', which='joliet_path')

def check_joliet_and_eltorito_onedir(iso, filesize):
    assert(filesize == 71680)

    internal_check_pvd(iso.pvd, extent=16, size=35, ptbl_size=22, ptbl_location_le=21, ptbl_location_be=23)

    internal_check_jolietvd(iso.svds[0], space_size=35, path_tbl_size=26, path_tbl_loc_le=25, path_tbl_loc_be=27)

    internal_check_eltorito(iso, boot_catalog_extent=33, load_rba=34, media_type=0, system_type=0, bootable=True, platform_id=0)

    internal_check_terminator(iso.vdsts, extent=19)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=29, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=31, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=5, data_length=2048, extent_location=29, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=5, data_length=2048, extent_location=31)

    dir1_record = iso.pvd.root_dir_record.children[4]
    internal_check_ptr(dir1_record.ptr, name=b'DIR1', len_di=4, loc=30, parent=1)
    internal_check_dir_record(dir1_record, num_children=2, name=b'DIR1', dr_len=38, extent_location=30, rr=False, rr_name=None, rr_links=0, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dir1_record.children[1], rr=False, rr_nlinks=3, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'BOOT.CAT;1', dr_len=44, loc=33, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BOOT.;1', dr_len=40, loc=34, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BOOT.;1', contents=b'boot\n', which='iso_path')

    joliet_dir1_record = iso.joliet_vd.root_dir_record.children[4]
    internal_check_ptr(joliet_dir1_record.ptr, name='dir1'.encode('utf-16_be'), len_di=8, loc=32, parent=1)
    internal_check_dir_record(joliet_dir1_record, num_children=2, name='dir1'.encode('utf-16_be'), dr_len=42, extent_location=32, rr=False, rr_name=None, rr_links=0, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(joliet_dir1_record.children[1], rr=False, rr_nlinks=3, xa=False, rr_onetwelve=False)

    internal_check_file(iso.joliet_vd.root_dir_record.children[3], name='boot.cat'.encode('utf-16_be'), dr_len=50, loc=33, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.joliet_vd.root_dir_record.children[2], name='boot'.encode('utf-16_be'), dr_len=42, loc=34, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/boot', contents=b'boot\n', which='joliet_path')

def check_joliet_rr_and_eltorito_nofiles(iso, filesize):
    assert(filesize == 69632)

    internal_check_pvd(iso.pvd, extent=16, size=34, ptbl_size=10, ptbl_location_le=21, ptbl_location_be=23)

    internal_check_jolietvd(iso.svds[0], space_size=34, path_tbl_size=10, path_tbl_loc_le=25, path_tbl_loc_be=27)

    internal_check_eltorito(iso, boot_catalog_extent=32, load_rba=33, media_type=0, system_type=0, bootable=True, platform_id=0)

    internal_check_terminator(iso.vdsts, extent=19)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=29, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=30, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=29, rr=True, rr_nlinks=2, xa=False, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=4, data_length=2048, extent_location=30)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'BOOT.CAT;1', dr_len=124, loc=32, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BOOT.;1', dr_len=116, loc=33, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BOOT.;1', contents=b'boot\n', which='iso_path')

    internal_check_file(iso.joliet_vd.root_dir_record.children[3], name='boot.cat'.encode('utf-16_be'), dr_len=50, loc=32, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.joliet_vd.root_dir_record.children[2], name='boot'.encode('utf-16_be'), dr_len=42, loc=33, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/boot', contents=b'boot\n', which='joliet_path')

def check_joliet_rr_and_eltorito_onefile(iso, filesize):
    assert(filesize == 71680)

    internal_check_pvd(iso.pvd, extent=16, size=35, ptbl_size=10, ptbl_location_le=21, ptbl_location_be=23)

    internal_check_jolietvd(iso.svds[0], space_size=35, path_tbl_size=10, path_tbl_loc_le=25, path_tbl_loc_be=27)

    internal_check_eltorito(iso, boot_catalog_extent=32, load_rba=33, media_type=0, system_type=0, bootable=True, platform_id=0)

    internal_check_terminator(iso.vdsts, extent=19)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=29, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=30, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=5, data_length=2048, extent_location=29, rr=True, rr_nlinks=2, xa=False, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=5, data_length=2048, extent_location=30)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'BOOT.CAT;1', dr_len=124, loc=32, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BOOT.;1', dr_len=116, loc=33, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BOOT.;1', contents=b'boot\n', which='iso_path')

    internal_check_file(iso.pvd.root_dir_record.children[4], name=b'FOO.;1', dr_len=116, loc=34, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

    internal_check_file(iso.joliet_vd.root_dir_record.children[3], name='boot.cat'.encode('utf-16_be'), dr_len=50, loc=32, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.joliet_vd.root_dir_record.children[2], name='boot'.encode('utf-16_be'), dr_len=42, loc=33, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/boot', contents=b'boot\n', which='joliet_path')

    internal_check_file(iso.joliet_vd.root_dir_record.children[4], name='foo'.encode('utf-16_be'), dr_len=40, loc=34, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/foo', contents=b'foo\n', which='joliet_path')

def check_joliet_rr_and_eltorito_onedir(iso, filesize):
    assert(filesize == 73728)

    internal_check_pvd(iso.pvd, extent=16, size=36, ptbl_size=22, ptbl_location_le=21, ptbl_location_be=23)

    internal_check_eltorito(iso, boot_catalog_extent=34, load_rba=35, media_type=0, system_type=0, bootable=True, platform_id=0)

    internal_check_jolietvd(iso.svds[0], space_size=36, path_tbl_size=26, path_tbl_loc_le=25, path_tbl_loc_be=27)

    internal_check_terminator(iso.vdsts, extent=19)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=29, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=31, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=5, data_length=2048, extent_location=29, rr=True, rr_nlinks=3, xa=False, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=5, data_length=2048, extent_location=31)

    dir1_record = iso.pvd.root_dir_record.children[4]
    internal_check_ptr(dir1_record.ptr, name=b'DIR1', len_di=4, loc=30, parent=1)
    internal_check_dir_record(dir1_record, num_children=2, name=b'DIR1', dr_len=114, extent_location=30, rr=True, rr_name=b'dir1', rr_links=2, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dir1_record.children[1], rr=True, rr_nlinks=3, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'BOOT.CAT;1', dr_len=124, loc=34, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BOOT.;1', dr_len=116, loc=35, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BOOT.;1', contents=b'boot\n', which='iso_path')

    joliet_dir1_record = iso.joliet_vd.root_dir_record.children[4]
    internal_check_ptr(joliet_dir1_record.ptr, name='dir1'.encode('utf-16_be'), len_di=8, loc=32, parent=1)
    internal_check_dir_record(joliet_dir1_record, num_children=2, name='dir1'.encode('utf-16_be'), dr_len=42, extent_location=32, rr=False, rr_name=None, rr_links=0, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(joliet_dir1_record.children[1], rr=False, rr_nlinks=3, xa=False, rr_onetwelve=False)

    internal_check_file(iso.joliet_vd.root_dir_record.children[3], name='boot.cat'.encode('utf-16_be'), dr_len=50, loc=34, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.joliet_vd.root_dir_record.children[2], name='boot'.encode('utf-16_be'), dr_len=42, loc=35, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/boot', contents=b'boot\n', which='joliet_path')

def check_rr_deep_dir(iso, filesize):
    assert(filesize == 69632)

    internal_check_pvd(iso.pvd, extent=16, size=34, ptbl_size=122, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    dir1_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(dir1_record.ptr, name=b'DIR1', len_di=4, loc=None, parent=1)
    rr_moved_record = iso.pvd.root_dir_record.children[3]
    internal_check_ptr(rr_moved_record.ptr, name=b'RR_MOVED', len_di=8, loc=None, parent=1)
    dir2_record = dir1_record.children[2]
    internal_check_ptr(dir2_record.ptr, name=b'DIR2', len_di=4, loc=None, parent=2)
    dir8_record = rr_moved_record.children[2]
    internal_check_ptr(dir8_record.ptr, name=b'DIR8', len_di=4, loc=None, parent=3)
    dir3_record = dir2_record.children[2]
    internal_check_ptr(dir3_record.ptr, name=b'DIR3', len_di=4, loc=None, parent=4)
    dir4_record = dir3_record.children[2]
    internal_check_ptr(dir4_record.ptr, name=b'DIR4', len_di=4, loc=None, parent=6)
    dir5_record = dir4_record.children[2]
    internal_check_ptr(dir5_record.ptr, name=b'DIR5', len_di=4, loc=None, parent=7)
    dir6_record = dir5_record.children[2]
    internal_check_ptr(dir6_record.ptr, name=b'DIR6', len_di=4, loc=None, parent=8)
    dir7_record = dir6_record.children[2]
    internal_check_ptr(dir7_record.ptr, name=b'DIR7', len_di=4, loc=None, parent=9)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=23, rr=True, rr_nlinks=4, xa=False, rr_onetwelve=False)

def check_rr_deep(iso, filesize):
    assert(filesize == 71680)

    internal_check_pvd(iso.pvd, extent=16, size=35, ptbl_size=122, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=23, rr=True, rr_nlinks=4, xa=False, rr_onetwelve=False)

    internal_check_file_contents(iso, path='/dir1/dir2/dir3/dir4/dir5/dir6/dir7/dir8/foo', contents=b'foo\n', which='rr_path')

def check_rr_deep2(iso, filesize):
    assert(filesize == 73728)

    internal_check_pvd(iso.pvd, extent=16, size=36, ptbl_size=134, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=23, rr=True, rr_nlinks=4, xa=False, rr_onetwelve=False)

    internal_check_file_contents(iso, path='/dir1/dir2/dir3/dir4/dir5/dir6/dir7/dir8/dir9/foo', contents=b'foo\n', which='rr_path')

def check_xa_nofiles(iso, filesize):
    assert(filesize == 49152)

    internal_check_pvd(iso.pvd, extent=16, size=24, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=21)

    assert(iso.pvd.application_use[141:149] == b'CD-XA001')

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=2, data_length=2048, extent_location=23, rr=False, rr_nlinks=0, xa=True, rr_onetwelve=False)

def check_xa_onefile(iso, filesize):
    assert(filesize == 51200)

    internal_check_pvd(iso.pvd, extent=16, size=25, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=21)

    assert(iso.pvd.application_use[141:149] == b'CD-XA001')

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=23, rr=False, rr_nlinks=0, xa=True, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'FOO.;1', dr_len=54, loc=24, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

def check_xa_onedir(iso, filesize):
    assert(filesize == 51200)

    internal_check_pvd(iso.pvd, extent=16, size=25, ptbl_size=22, ptbl_location_le=19, ptbl_location_be=21)

    assert(iso.pvd.application_use[141:149] == b'CD-XA001')

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=23, rr=False, rr_nlinks=0, xa=True, rr_onetwelve=False)

    dir1_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(dir1_record.ptr, name=b'DIR1', len_di=4, loc=24, parent=1)
    internal_check_dir_record(dir1_record, num_children=2, name=b'DIR1', dr_len=52, extent_location=24, rr=False, rr_name=None, rr_links=0, xa=True, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dir1_record.children[1], rr=False, rr_nlinks=3, xa=True, rr_onetwelve=False)

def check_sevendeepdirs(iso, filesize):
    assert(filesize == 65536)

    internal_check_pvd(iso.pvd, extent=16, size=32, ptbl_size=94, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=23, rr=True, rr_nlinks=3, xa=False, rr_onetwelve=False)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    dir1_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(dir1_record.ptr, name=b'DIR1', len_di=4, loc=24, parent=1)
    internal_check_dir_record(dir1_record, num_children=3, name=b'DIR1', dr_len=114, extent_location=24, rr=True, rr_name=b'dir1', rr_links=3, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dir1_record.children[1], rr=True, rr_nlinks=3, xa=False, rr_onetwelve=False)

    dir2_record = dir1_record.children[2]
    internal_check_ptr(dir2_record.ptr, name=b'DIR2', len_di=4, loc=25, parent=2)
    internal_check_dir_record(dir2_record, num_children=3, name=b'DIR2', dr_len=114, extent_location=25, rr=True, rr_name=b'dir2', rr_links=3, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dir2_record.children[1], rr=True, rr_nlinks=3, xa=False, rr_onetwelve=False)

    dir3_record = dir2_record.children[2]
    internal_check_ptr(dir3_record.ptr, name=b'DIR3', len_di=4, loc=26, parent=3)
    internal_check_dir_record(dir3_record, num_children=3, name=b'DIR3', dr_len=114, extent_location=26, rr=True, rr_name=b'dir3', rr_links=3, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dir3_record.children[1], rr=True, rr_nlinks=3, xa=False, rr_onetwelve=False)

    dir4_record = dir3_record.children[2]
    internal_check_ptr(dir4_record.ptr, name=b'DIR4', len_di=4, loc=27, parent=4)
    internal_check_dir_record(dir4_record, num_children=3, name=b'DIR4', dr_len=114, extent_location=27, rr=True, rr_name=b'dir4', rr_links=3, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dir4_record.children[1], rr=True, rr_nlinks=3, xa=False, rr_onetwelve=False)

    dir5_record = dir4_record.children[2]
    internal_check_ptr(dir5_record.ptr, name=b'DIR5', len_di=4, loc=28, parent=5)
    internal_check_dir_record(dir5_record, num_children=3, name=b'DIR5', dr_len=114, extent_location=28, rr=True, rr_name=b'dir5', rr_links=3, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dir5_record.children[1], rr=True, rr_nlinks=3, xa=False, rr_onetwelve=False)

    dir6_record = dir5_record.children[2]
    internal_check_ptr(dir6_record.ptr, name=b'DIR6', len_di=4, loc=29, parent=6)
    internal_check_dir_record(dir6_record, num_children=3, name=b'DIR6', dr_len=114, extent_location=29, rr=True, rr_name=b'dir6', rr_links=3, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dir6_record.children[1], rr=True, rr_nlinks=3, xa=False, rr_onetwelve=False)

    dir7_record = dir6_record.children[2]
    internal_check_ptr(dir7_record.ptr, name=b'DIR7', len_di=4, loc=30, parent=7)
    internal_check_dir_record(dir7_record, num_children=2, name=b'DIR7', dr_len=114, extent_location=30, rr=True, rr_name=b'dir7', rr_links=2, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dir7_record.children[1], rr=True, rr_nlinks=3, xa=False, rr_onetwelve=False)

def check_xa_joliet_nofiles(iso, filesize):
    assert(filesize == 61440)

    internal_check_pvd(iso.pvd, extent=16, size=30, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    assert(iso.pvd.application_use[141:149] == b'CD-XA001')

    internal_check_jolietvd(iso.svds[0], space_size=30, path_tbl_size=10, path_tbl_loc_le=24, path_tbl_loc_be=26)

    assert(iso.joliet_vd.application_use[141:149] == b'CD-XA001')

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=28, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=29, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=2, data_length=2048, extent_location=28, rr=False, rr_nlinks=0, xa=True, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=2, data_length=2048, extent_location=29)

def check_xa_joliet_onefile(iso, filesize):
    assert(filesize == 63488)

    internal_check_pvd(iso.pvd, extent=16, size=31, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    assert(iso.pvd.application_use[141:149] == b'CD-XA001')

    internal_check_jolietvd(iso.svds[0], space_size=31, path_tbl_size=10, path_tbl_loc_le=24, path_tbl_loc_be=26)

    assert(iso.joliet_vd.application_use[141:149] == b'CD-XA001')

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=28, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=29, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=28, rr=False, rr_nlinks=0, xa=True, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=3, data_length=2048, extent_location=29)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'FOO.;1', dr_len=54, loc=30, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

    internal_check_file(iso.joliet_vd.root_dir_record.children[2], name='foo'.encode('utf-16_be'), dr_len=40, loc=30, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/foo', contents=b'foo\n', which='joliet_path')

def check_xa_joliet_onedir(iso, filesize):
    assert(filesize == 65536)

    internal_check_pvd(iso.pvd, extent=16, size=32, ptbl_size=22, ptbl_location_le=20, ptbl_location_be=22)

    assert(iso.pvd.application_use[141:149] == b'CD-XA001')

    internal_check_jolietvd(iso.svds[0], space_size=32, path_tbl_size=26, path_tbl_loc_le=24, path_tbl_loc_be=26)

    assert(iso.joliet_vd.application_use[141:149] == b'CD-XA001')

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=28, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=30, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=28, rr=False, rr_nlinks=0, xa=True, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=3, data_length=2048, extent_location=30)

    dir1_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(dir1_record.ptr, name=b'DIR1', len_di=4, loc=29, parent=1)
    internal_check_dir_record(dir1_record, num_children=2, name=b'DIR1', dr_len=52, extent_location=29, rr=False, rr_name=None, rr_links=0, xa=True, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(iso.pvd.root_dir_record.children[2].children[1], rr=False, rr_nlinks=3, xa=True, rr_onetwelve=True)

    joliet_dir1_record = iso.joliet_vd.root_dir_record.children[2]
    internal_check_ptr(joliet_dir1_record.ptr, name='dir1'.encode('utf-16_be'), len_di=8, loc=31, parent=1)
    internal_check_dir_record(joliet_dir1_record, num_children=2, name='dir1'.encode('utf-16_be'), dr_len=42, extent_location=31, rr=False, rr_name=None, rr_links=0, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(joliet_dir1_record.children[1], rr=False, rr_nlinks=3, xa=False, rr_onetwelve=False)

def check_isolevel4_nofiles(iso, filesize):
    assert(filesize == 51200)

    internal_check_pvd(iso.pvd, extent=16, size=25, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_enhanced_vd(iso.enhanced_vd, size=25, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=24, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=2, data_length=2048, extent_location=24, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

def check_isolevel4_onefile(iso, filesize):
    assert(filesize == 53248)

    internal_check_pvd(iso.pvd, extent=16, size=26, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_enhanced_vd(iso.enhanced_vd, size=26, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=24, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=24, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'foo', dr_len=36, loc=25, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/foo', contents=b'foo\n', which='iso_path')

def check_isolevel4_onedir(iso, filesize):
    assert(filesize == 53248)

    internal_check_pvd(iso.pvd, extent=16, size=26, ptbl_size=22, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_enhanced_vd(iso.enhanced_vd, size=26, ptbl_size=22, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=24, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=24, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    dir1_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(dir1_record.ptr, name=b'dir1', len_di=4, loc=25, parent=1)
    internal_check_dir_record(dir1_record, num_children=2, name=b'dir1', dr_len=38, extent_location=25, rr=False, rr_name=None, rr_links=0, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dir1_record.children[1], rr=False, rr_nlinks=3, xa=False, rr_onetwelve=False)

def check_isolevel4_eltorito(iso, filesize):
    assert(filesize == 57344)

    internal_check_pvd(iso.pvd, extent=16, size=28, ptbl_size=10, ptbl_location_le=21, ptbl_location_be=23)

    internal_check_eltorito(iso, boot_catalog_extent=26, load_rba=27, media_type=0, system_type=0, bootable=True, platform_id=0)

    internal_check_enhanced_vd(iso.enhanced_vd, size=28, ptbl_size=10, ptbl_location_le=21, ptbl_location_be=23)

    internal_check_terminator(iso.vdsts, extent=19)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=25, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=25, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'boot.cat', dr_len=42, loc=26, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'boot', dr_len=38, loc=27, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/boot', contents=b'boot\n', which='iso_path')

def check_everything(iso, filesize):
    assert(filesize == 108544)

    internal_check_pvd(iso.pvd, extent=16, size=53, ptbl_size=106, ptbl_location_le=22, ptbl_location_be=24)

    internal_check_eltorito(iso, boot_catalog_extent=49, load_rba=50, media_type=0, system_type=0, bootable=True, platform_id=0)

    internal_check_enhanced_vd(iso.enhanced_vd, size=53, ptbl_size=106, ptbl_location_le=22, ptbl_location_be=24)

    assert(iso.pvd.application_use[141:149] == b'CD-XA001')

    internal_check_jolietvd(iso.svds[1], space_size=53, path_tbl_size=138, path_tbl_loc_le=26, path_tbl_loc_be=28)

    assert(iso.joliet_vd.application_use[141:149] == b'CD-XA001')

    internal_check_terminator(iso.vdsts, extent=20)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=30, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=39, parent=1)
    joliet_dir1_record = iso.joliet_vd.root_dir_record.children[4]
    internal_check_ptr(joliet_dir1_record.ptr, name='dir1'.encode('utf-16_be'), len_di=8, loc=40, parent=1)
    joliet_dir2_record = joliet_dir1_record.children[2]
    internal_check_ptr(joliet_dir2_record.ptr, name='dir2'.encode('utf-16_be'), len_di=8, loc=41, parent=2)
    joliet_dir3_record = joliet_dir2_record.children[2]
    internal_check_ptr(joliet_dir3_record.ptr, name='dir3'.encode('utf-16_be'), len_di=8, loc=42, parent=3)
    joliet_dir4_record = joliet_dir3_record.children[2]
    internal_check_ptr(joliet_dir4_record.ptr, name='dir4'.encode('utf-16_be'), len_di=8, loc=43, parent=4)
    joliet_dir5_record = joliet_dir4_record.children[2]
    internal_check_ptr(joliet_dir5_record.ptr, name='dir5'.encode('utf-16_be'), len_di=8, loc=44, parent=5)
    joliet_dir6_record = joliet_dir5_record.children[2]
    internal_check_ptr(joliet_dir6_record.ptr, name='dir6'.encode('utf-16_be'), len_di=8, loc=45, parent=6)
    joliet_dir7_record = joliet_dir6_record.children[2]
    internal_check_ptr(joliet_dir7_record.ptr, name='dir7'.encode('utf-16_be'), len_di=8, loc=46, parent=7)
    joliet_dir8_record = joliet_dir7_record.children[2]
    internal_check_ptr(joliet_dir8_record.ptr, name='dir8'.encode('utf-16_be'), len_di=8, loc=47, parent=8)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=7, data_length=2048, extent_location=30, rr=True, rr_nlinks=3, xa=True, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=7, data_length=2048, extent_location=39)

    boot_rec = iso.pvd.root_dir_record.children[2]
    internal_check_file(boot_rec, name=b'boot', dr_len=128, loc=50, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/boot', contents=b'boot\n', which='iso_path')

    internal_check_boot_info_table(boot_rec.inode.boot_info_table, vd_extent=16, inode_extent=50, orig_len=5, csum=0)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'boot.cat', dr_len=136, loc=49, datalen=2048, hidden=False, multi_extent=False)

    dir1_record = iso.pvd.root_dir_record.children[4]
    internal_check_ptr(dir1_record.ptr, name=b'dir1', len_di=4, loc=31, parent=1)
    internal_check_dir_record(dir1_record, num_children=4, name=b'dir1', dr_len=128, extent_location=31, rr=True, rr_name=b'dir1', rr_links=3, xa=True, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dir1_record.children[1], rr=True, rr_nlinks=3, xa=True, rr_onetwelve=False)

    internal_check_file(dir1_record.children[3], name=b'foo', dr_len=126, loc=51, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/dir1/foo', contents=b'foo\n', which='iso_path')

    internal_check_file(iso.pvd.root_dir_record.children[5], name=b'foo', dr_len=126, loc=51, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/foo', contents=b'foo\n', which='iso_path')

    # Now check the rock ridge symlink.  It should have a directory record
    # length of 132, and the symlink components should be 'dir1' and 'foo'.
    sym_dir_record = iso.pvd.root_dir_record.children[6]
    internal_check_rr_symlink(sym_dir_record, name=b'sym', dr_len=136, comps=[b'foo'])

    dir2_record = dir1_record.children[2]
    internal_check_ptr(dir2_record.ptr, name=b'dir2', len_di=4, loc=32, parent=2)
    internal_check_dir_record(dir2_record, num_children=3, name=b'dir2', dr_len=128, extent_location=32, rr=True, rr_name=b'dir2', rr_links=3, xa=True, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dir2_record.children[1], rr=True, rr_nlinks=3, xa=True, rr_onetwelve=False)

    dir3_record = dir2_record.children[2]
    internal_check_ptr(dir3_record.ptr, name=b'dir3', len_di=4, loc=33, parent=3)
    internal_check_dir_record(dir3_record, num_children=3, name=b'dir3', dr_len=128, extent_location=33, rr=True, rr_name=b'dir3', rr_links=3, xa=True, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dir3_record.children[1], rr=True, rr_nlinks=3, xa=True, rr_onetwelve=False)

    dir4_record = dir3_record.children[2]
    internal_check_ptr(dir4_record.ptr, name=b'dir4', len_di=4, loc=34, parent=4)
    internal_check_dir_record(dir4_record, num_children=3, name=b'dir4', dr_len=128, extent_location=34, rr=True, rr_name=b'dir4', rr_links=3, xa=True, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dir4_record.children[1], rr=True, rr_nlinks=3, xa=True, rr_onetwelve=False)

    dir5_record = dir4_record.children[2]
    internal_check_ptr(dir5_record.ptr, name=b'dir5', len_di=4, loc=35, parent=5)
    internal_check_dir_record(dir5_record, num_children=3, name=b'dir5', dr_len=128, extent_location=35, rr=True, rr_name=b'dir5', rr_links=3, xa=True, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dir5_record.children[1], rr=True, rr_nlinks=3, xa=True, rr_onetwelve=False)

    dir6_record = dir5_record.children[2]
    internal_check_ptr(dir6_record.ptr, name=b'dir6', len_di=4, loc=36, parent=6)
    internal_check_dir_record(dir6_record, num_children=3, name=b'dir6', dr_len=128, extent_location=36, rr=True, rr_name=b'dir6', rr_links=3, xa=True, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dir6_record.children[1], rr=True, rr_nlinks=3, xa=True, rr_onetwelve=False)

    dir7_record = dir6_record.children[2]
    internal_check_ptr(dir7_record.ptr, name=b'dir7', len_di=4, loc=37, parent=7)
    internal_check_dir_record(dir7_record, num_children=3, name=b'dir7', dr_len=128, extent_location=37, rr=True, rr_name=b'dir7', rr_links=3, xa=True, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dir7_record.children[1], rr=True, rr_nlinks=3, xa=True, rr_onetwelve=False)

    dir8_record = dir7_record.children[2]
    internal_check_ptr(dir8_record.ptr, name=b'dir8', len_di=4, loc=38, parent=8)
    internal_check_dir_record(dir8_record, num_children=3, name=b'dir8', dr_len=128, extent_location=38, rr=True, rr_name=b'dir8', rr_links=2, xa=True, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dir8_record.children[1], rr=True, rr_nlinks=3, xa=True, rr_onetwelve=False)

    internal_check_file(dir8_record.children[2], name=b'bar', dr_len=126, loc=52, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/dir1/dir2/dir3/dir4/dir5/dir6/dir7/dir8/bar', contents=b'bar\n', which='iso_path')

def check_rr_xa_nofiles(iso, filesize):
    assert(filesize == 51200)

    internal_check_pvd(iso.pvd, extent=16, size=25, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=21)

    assert(iso.pvd.application_use[141:149] == b'CD-XA001')

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=2, data_length=2048, extent_location=23, rr=True, rr_nlinks=2, xa=True, rr_onetwelve=False)

def check_rr_xa_onefile(iso, filesize):
    assert(filesize == 53248)

    internal_check_pvd(iso.pvd, extent=16, size=26, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=21)

    assert(iso.pvd.application_use[141:149] == b'CD-XA001')

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=23, rr=True, rr_nlinks=2, xa=True, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'FOO.;1', dr_len=130, loc=25, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')
    internal_check_file_contents(iso, path='/foo', contents=b'foo\n', which='rr_path')

def check_rr_xa_onedir(iso, filesize):
    assert(filesize == 53248)

    internal_check_pvd(iso.pvd, extent=16, size=26, ptbl_size=22, ptbl_location_le=19, ptbl_location_be=21)

    assert(iso.pvd.application_use[141:149] == b'CD-XA001')

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=23, rr=True, rr_nlinks=3, xa=True, rr_onetwelve=False)

    dir1_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(dir1_record.ptr, name=b'DIR1', len_di=4, loc=24, parent=1)
    internal_check_dir_record(dir1_record, num_children=2, name=b'DIR1', dr_len=128, extent_location=24, rr=True, rr_name=b'dir1', rr_links=2, xa=True, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dir1_record.children[1], rr=True, rr_nlinks=3, xa=True, rr_onetwelve=False)

def check_rr_joliet_symlink(iso, filesize):
    assert(filesize == 65536)

    internal_check_pvd(iso.pvd, extent=16, size=32, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_jolietvd(iso.svds[0], space_size=32, path_tbl_size=10, path_tbl_loc_le=24, path_tbl_loc_be=26)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=28, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=29, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=28, rr=True, rr_nlinks=2, xa=False, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=4, data_length=2048, extent_location=29)

    foo_dir_record = iso.pvd.root_dir_record.children[2]
    internal_check_file(foo_dir_record, name=b'FOO.;1', dr_len=116, loc=31, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

    internal_check_rr_file(foo_dir_record, name=b'foo')
    internal_check_file_contents(iso, path='/foo', contents=b'foo\n', which='rr_path')

    # Now check the rock ridge symlink.  It should have a directory record
    # length of 126, and the symlink components should be 'foo'.
    sym_dir_record = iso.pvd.root_dir_record.children[3]
    internal_check_rr_symlink(sym_dir_record, name=b'SYM.;1', dr_len=126, comps=[b'foo'])

    internal_check_file(iso.joliet_vd.root_dir_record.children[2], name='foo'.encode('utf-16_be'), dr_len=40, loc=31, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/foo', contents=b'foo\n', which='joliet_path')

    assert(iso.has_rock_ridge())
    assert(iso.has_joliet())
    assert(not iso.has_udf())

def check_rr_joliet_deep(iso, filesize):
    assert(filesize == 98304)

    internal_check_pvd(iso.pvd, extent=16, size=48, ptbl_size=122, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=28, parent=1)
    dir1_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(dir1_record.ptr, name=b'DIR1', len_di=4, loc=None, parent=1)
    rr_moved_record = iso.pvd.root_dir_record.children[3]
    internal_check_ptr(rr_moved_record.ptr, name=b'RR_MOVED', len_di=8, loc=None, parent=1)
    dir2_record = dir1_record.children[2]
    internal_check_ptr(dir2_record.ptr, name=b'DIR2', len_di=4, loc=None, parent=2)
    dir8_record = rr_moved_record.children[2]
    internal_check_ptr(dir8_record.ptr, name=b'DIR8', len_di=4, loc=None, parent=3)
    dir3_record = dir2_record.children[2]
    internal_check_ptr(dir3_record.ptr, name=b'DIR3', len_di=4, loc=None, parent=4)
    dir4_record = dir3_record.children[2]
    internal_check_ptr(dir4_record.ptr, name=b'DIR4', len_di=4, loc=None, parent=6)
    dir5_record = dir4_record.children[2]
    internal_check_ptr(dir5_record.ptr, name=b'DIR5', len_di=4, loc=None, parent=7)
    dir6_record = dir5_record.children[2]
    internal_check_ptr(dir6_record.ptr, name=b'DIR6', len_di=4, loc=None, parent=8)
    dir7_record = dir6_record.children[2]
    internal_check_ptr(dir7_record.ptr, name=b'DIR7', len_di=4, loc=None, parent=9)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=None, parent=1)
    joliet_dir1_record = iso.joliet_vd.root_dir_record.children[2]
    internal_check_ptr(joliet_dir1_record.ptr, name='dir1'.encode('utf-16_be'), len_di=8, loc=None, parent=1)
    joliet_dir2_record = joliet_dir1_record.children[2]
    internal_check_ptr(joliet_dir2_record.ptr, name='dir2'.encode('utf-16_be'), len_di=8, loc=None, parent=2)
    joliet_dir3_record = joliet_dir2_record.children[2]
    internal_check_ptr(joliet_dir3_record.ptr, name='dir3'.encode('utf-16_be'), len_di=8, loc=None, parent=3)
    joliet_dir4_record = joliet_dir3_record.children[2]
    internal_check_ptr(joliet_dir4_record.ptr, name='dir4'.encode('utf-16_be'), len_di=8, loc=None, parent=4)
    joliet_dir5_record = joliet_dir4_record.children[2]
    internal_check_ptr(joliet_dir5_record.ptr, name='dir5'.encode('utf-16_be'), len_di=8, loc=None, parent=5)
    joliet_dir6_record = joliet_dir5_record.children[2]
    internal_check_ptr(joliet_dir6_record.ptr, name='dir6'.encode('utf-16_be'), len_di=8, loc=None, parent=6)
    joliet_dir7_record = joliet_dir6_record.children[2]
    internal_check_ptr(joliet_dir7_record.ptr, name='dir7'.encode('utf-16_be'), len_di=8, loc=None, parent=7)
    joliet_dir8_record = joliet_dir7_record.children[2]
    internal_check_ptr(joliet_dir8_record.ptr, name='dir8'.encode('utf-16_be'), len_di=8, loc=None, parent=8)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=28, rr=True, rr_nlinks=4, xa=False, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=3, data_length=2048, extent_location=38)

def check_eltorito_multi_boot(iso, filesize):
    assert(filesize == 59392)

    internal_check_pvd(iso.pvd, extent=16, size=29, ptbl_size=10, ptbl_location_le=21, ptbl_location_be=23)

    internal_check_eltorito(iso, boot_catalog_extent=26, load_rba=27, media_type=0, system_type=0, bootable=True, platform_id=0)

    assert(len(iso.eltorito_boot_catalog.sections) == 1)
    sec = iso.eltorito_boot_catalog.sections[0]
    assert(sec.header_indicator == 0x91)
    assert(sec.platform_id == 0)
    assert(sec.num_section_entries == 1)
    assert(sec.id_string == b'\x00'*28)
    assert(len(sec.section_entries) == 1)
    entry = sec.section_entries[0]
    assert(entry.boot_indicator == 0x88)
    assert(entry.boot_media_type == 0x0)
    assert(entry.load_segment == 0x0)
    assert(entry.system_type == 0)
    assert(entry.sector_count == 4)
    assert(entry.load_rba == 28)

    internal_check_terminator(iso.vdsts, extent=19)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=25, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=5, data_length=2048, extent_location=25, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'boot.cat', dr_len=42, loc=26, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'boot', dr_len=38, loc=27, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/boot', contents=b'boot\n', which='iso_path')

    internal_check_file(iso.pvd.root_dir_record.children[4], name=b'boot2', dr_len=38, loc=28, datalen=6, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/boot2', contents=b'boot2\n', which='iso_path')

def check_eltorito_multi_boot_hard_link(iso, filesize):
    assert(filesize == 59392)

    internal_check_pvd(iso.pvd, extent=16, size=29, ptbl_size=10, ptbl_location_le=21, ptbl_location_be=23)

    internal_check_eltorito(iso, boot_catalog_extent=26, load_rba=27, media_type=0, system_type=0, bootable=True, platform_id=0)

    assert(len(iso.eltorito_boot_catalog.sections) == 1)
    sec = iso.eltorito_boot_catalog.sections[0]
    assert(sec.header_indicator == 0x91)
    assert(sec.platform_id == 0)
    assert(sec.num_section_entries == 1)
    assert(sec.id_string == b'\x00'*28)
    assert(len(sec.section_entries) == 1)
    entry = sec.section_entries[0]
    assert(entry.boot_indicator == 0x88)
    assert(entry.boot_media_type == 0x0)
    assert(entry.load_segment == 0x0)
    assert(entry.system_type == 0)
    assert(entry.sector_count == 4)
    assert(entry.load_rba == 28)

    internal_check_terminator(iso.vdsts, extent=19)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=25, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=6, data_length=2048, extent_location=25, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'boot.cat', dr_len=42, loc=26, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'boot', dr_len=38, loc=27, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/boot', contents=b'boot\n', which='iso_path')

    internal_check_file(iso.pvd.root_dir_record.children[4], name=b'boot2', dr_len=38, loc=28, datalen=6, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/boot2', contents=b'boot2\n', which='iso_path')

    internal_check_file(iso.pvd.root_dir_record.children[5], name=b'bootlink', dr_len=42, loc=28, datalen=6, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/bootlink', contents=b'boot2\n', which='iso_path')

def check_eltorito_boot_info_table(iso, filesize):
    assert(filesize == 57344)

    internal_check_pvd(iso.pvd, extent=16, size=28, ptbl_size=10, ptbl_location_le=21, ptbl_location_be=23)

    internal_check_eltorito(iso, boot_catalog_extent=26, load_rba=27, media_type=0, system_type=0, bootable=True, platform_id=0)

    internal_check_terminator(iso.vdsts, extent=19)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=25, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=25, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'boot.cat', dr_len=42, loc=26, datalen=2048, hidden=False, multi_extent=False)

    boot_rec = iso.pvd.root_dir_record.children[2]
    internal_check_file(boot_rec, name=b'boot', dr_len=38, loc=27, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/boot', contents=b'boot\n', which='iso_path')

    internal_check_boot_info_table(boot_rec.inode.boot_info_table, vd_extent=16, inode_extent=27, orig_len=5, csum=0)

def check_eltorito_boot_info_table_large(iso, filesize):
    assert(filesize == 57344)

    internal_check_pvd(iso.pvd, extent=16, size=28, ptbl_size=10, ptbl_location_le=21, ptbl_location_be=23)

    internal_check_eltorito(iso, boot_catalog_extent=26, load_rba=27, media_type=0, system_type=0, bootable=True, platform_id=0)

    internal_check_terminator(iso.vdsts, extent=19)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=25, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=25, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'boot.cat', dr_len=42, loc=26, datalen=2048, hidden=False, multi_extent=False)

    boot_rec = iso.pvd.root_dir_record.children[2]
    internal_check_file(boot_rec, name=b'boot', dr_len=38, loc=27, datalen=80, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/boot', contents=b'bootboot\x10\x00\x00\x00\x1b\x00\x00\x00P\x00\x00\x00\x88\xbd\xbd\xd1\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00bootbootbootboot', which='iso_path')

    internal_check_boot_info_table(boot_rec.inode.boot_info_table, vd_extent=16, inode_extent=27, orig_len=80, csum=0xd1bdbd88)

def check_hard_link(iso, filesize):
    assert(filesize == 53248)

    internal_check_pvd(iso.pvd, extent=16, size=26, ptbl_size=22, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=23, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'FOO.;1', dr_len=40, loc=25, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

    dir1_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(dir1_record.ptr, name=b'DIR1', len_di=4, loc=24, parent=1)
    internal_check_dir_record(dir1_record, num_children=3, name=b'DIR1', dr_len=38, extent_location=24, rr=False, rr_name=b'', rr_links=0, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dir1_record.children[1], rr=False, rr_nlinks=3, xa=False, rr_onetwelve=False)

    internal_check_file(dir1_record.children[2], name=b'FOO.;1', dr_len=40, loc=25, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/DIR1/FOO.;1', contents=b'foo\n', which='iso_path')

def check_same_dirname_different_parent(iso, filesize):
    assert(filesize == 79872)

    internal_check_pvd(iso.pvd, extent=16, size=39, ptbl_size=58, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_jolietvd(iso.svds[0], space_size=39, path_tbl_size=74, path_tbl_loc_le=24, path_tbl_loc_be=26)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=28, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=33, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=28, rr=True, rr_nlinks=4, xa=False, rr_onetwelve=False)

    dir1_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(dir1_record.ptr, name=b'DIR1', len_di=4, loc=None, parent=1)
    internal_check_dir_record(dir1_record, num_children=3, name=b'DIR1', dr_len=114, extent_location=None, rr=True, rr_name=b'dir1', rr_links=3, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dir1_record.children[1], rr=True, rr_nlinks=4, xa=False, rr_onetwelve=False)

    dir2_record = iso.pvd.root_dir_record.children[3]
    internal_check_ptr(dir2_record.ptr, name=b'DIR2', len_di=4, loc=None, parent=1)
    internal_check_dir_record(dir2_record, num_children=3, name=b'DIR2', dr_len=114, extent_location=None, rr=True, rr_name=b'dir2', rr_links=3, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dir2_record.children[1], rr=True, rr_nlinks=4, xa=False, rr_onetwelve=False)

    boot1_record = dir1_record.children[2]
    internal_check_ptr(boot1_record.ptr, name=b'BOOT', len_di=4, loc=None, parent=2)
    internal_check_dir_record(boot1_record, num_children=2, name=b'BOOT', dr_len=114, extent_location=None, rr=True, rr_name=b'boot', rr_links=2, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(boot1_record.children[1], rr=True, rr_nlinks=3, xa=False, rr_onetwelve=False)

    boot2_record = dir2_record.children[2]
    internal_check_ptr(boot2_record.ptr, name=b'BOOT', len_di=4, loc=None, parent=3)
    internal_check_dir_record(boot2_record, num_children=2, name=b'BOOT', dr_len=114, extent_location=None, rr=True, rr_name=b'boot', rr_links=2, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(boot2_record.children[1], rr=True, rr_nlinks=3, xa=False, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=4, data_length=2048, extent_location=33)

    dir1_joliet_record = iso.joliet_vd.root_dir_record.children[2]
    internal_check_ptr(dir1_joliet_record.ptr, name='dir1'.encode('utf-16_be'), len_di=8, loc=None, parent=1)
    internal_check_dir_record(dir1_joliet_record, num_children=3, name='dir1'.encode('utf-16_be'), dr_len=42, extent_location=None, rr=False, rr_name=None, rr_links=0, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dir1_joliet_record.children[1], rr=False, rr_nlinks=3, xa=False, rr_onetwelve=False)

    dir2_joliet_record = iso.joliet_vd.root_dir_record.children[3]
    internal_check_ptr(dir2_joliet_record.ptr, name='dir2'.encode('utf-16_be'), len_di=8, loc=None, parent=1)
    internal_check_dir_record(dir2_joliet_record, num_children=3, name='dir2'.encode('utf-16_be'), dr_len=42, extent_location=None, rr=False, rr_name=None, rr_links=0, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dir2_joliet_record.children[1], rr=False, rr_nlinks=3, xa=False, rr_onetwelve=False)

    boot1_joliet_record = dir1_joliet_record.children[2]
    internal_check_ptr(boot1_joliet_record.ptr, name='boot'.encode('utf-16_be'), len_di=8, loc=None, parent=2)
    internal_check_dir_record(boot1_joliet_record, num_children=2, name='boot'.encode('utf-16_be'), dr_len=42, extent_location=None, rr=False, rr_name=None, rr_links=0, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(boot1_joliet_record.children[1], rr=False, rr_nlinks=3, xa=False, rr_onetwelve=False)

    boot2_joliet_record = dir2_joliet_record.children[2]
    internal_check_ptr(boot2_joliet_record.ptr, name='boot'.encode('utf-16_be'), len_di=8, loc=None, parent=3)
    internal_check_dir_record(boot2_joliet_record, num_children=2, name='boot'.encode('utf-16_be'), dr_len=42, extent_location=None, rr=False, rr_name=None, rr_links=0, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(boot2_joliet_record.children[1], rr=False, rr_nlinks=3, xa=False, rr_onetwelve=False)

def check_joliet_isolevel4(iso, filesize):
    assert(filesize == 69632)

    internal_check_pvd(iso.pvd, extent=16, size=34, ptbl_size=22, ptbl_location_le=21, ptbl_location_be=23)

    internal_check_enhanced_vd(iso.enhanced_vd, size=34, ptbl_size=22, ptbl_location_le=21, ptbl_location_be=23)

    internal_check_jolietvd(iso.joliet_vd, space_size=34, path_tbl_size=26, path_tbl_loc_le=25, path_tbl_loc_be=27)

    internal_check_terminator(iso.vdsts, extent=19)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=29, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=31, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=29, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    dir1_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(dir1_record.ptr, name=b'dir1', len_di=4, loc=30, parent=1)
    internal_check_empty_directory(dir1_record, name=b'dir1', dr_len=38, extent=30, rr=False, hidden=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=4, data_length=2048, extent_location=31)

    joliet_dir1_record = iso.joliet_vd.root_dir_record.children[2]
    internal_check_ptr(joliet_dir1_record.ptr, name='dir1'.encode('utf-16_be'), len_di=8, loc=32, parent=1)
    internal_check_empty_directory(joliet_dir1_record, name='dir1'.encode('utf-16_be'), dr_len=42, extent=32, rr=False, hidden=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'foo', dr_len=36, loc=33, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/foo', contents=b'foo\n', which='iso_path')

    internal_check_file(iso.joliet_vd.root_dir_record.children[3], name='foo'.encode('utf-16_be'), dr_len=40, loc=33, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/foo', contents=b'foo\n', which='iso_path')

def check_eltorito_nofiles_hide(iso, filesize):
    assert(filesize == 55296)

    internal_check_pvd(iso.pvd, extent=16, size=27, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_eltorito(iso, boot_catalog_extent=25, load_rba=26, media_type=0, system_type=0, bootable=True, platform_id=0)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=24, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=24, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BOOT.;1', dr_len=40, loc=26, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BOOT.;1', contents=b'boot\n', which='iso_path')

def check_joliet_and_eltorito_nofiles_hide(iso, filesize):
    assert(filesize == 67584)

    internal_check_pvd(iso.pvd, extent=16, size=33, ptbl_size=10, ptbl_location_le=21, ptbl_location_be=23)

    internal_check_jolietvd(iso.svds[0], space_size=33, path_tbl_size=10, path_tbl_loc_le=25, path_tbl_loc_be=27)

    internal_check_eltorito(iso, boot_catalog_extent=31, load_rba=32, media_type=0, system_type=0, bootable=True, platform_id=0)

    internal_check_terminator(iso.vdsts, extent=19)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=29, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=30, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=29, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=3, data_length=2048, extent_location=30)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BOOT.;1', dr_len=40, loc=32, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BOOT.;1', contents=b'boot\n', which='iso_path')

    internal_check_file(iso.joliet_vd.root_dir_record.children[2], name='boot'.encode('utf-16_be'), dr_len=42, loc=32, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/boot', contents=b'boot\n', which='joliet_path')

def check_joliet_and_eltorito_nofiles_hide_only(iso, filesize):
    assert(filesize == 67584)

    internal_check_pvd(iso.pvd, extent=16, size=33, ptbl_size=10, ptbl_location_le=21, ptbl_location_be=23)

    internal_check_jolietvd(iso.svds[0], space_size=33, path_tbl_size=10, path_tbl_loc_le=25, path_tbl_loc_be=27)

    internal_check_eltorito(iso, boot_catalog_extent=31, load_rba=32, media_type=0, system_type=0, bootable=True, platform_id=0)

    internal_check_terminator(iso.vdsts, extent=19)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=29, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=30, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=29, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=3, data_length=2048, extent_location=30)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BOOT.;1', dr_len=40, loc=32, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BOOT.;1', contents=b'boot\n', which='iso_path')

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'BOOT.CAT;1', dr_len=44, loc=31, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.joliet_vd.root_dir_record.children[2], name='boot'.encode('utf-16_be'), dr_len=42, loc=32, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/boot', contents=b'boot\n', which='joliet_path')

def check_joliet_and_eltorito_nofiles_hide_iso_only(iso, filesize):
    assert(filesize == 67584)

    internal_check_pvd(iso.pvd, extent=16, size=33, ptbl_size=10, ptbl_location_le=21, ptbl_location_be=23)

    internal_check_jolietvd(iso.svds[0], space_size=33, path_tbl_size=10, path_tbl_loc_le=25, path_tbl_loc_be=27)

    internal_check_eltorito(iso, boot_catalog_extent=31, load_rba=32, media_type=0, system_type=0, bootable=True, platform_id=0)

    internal_check_terminator(iso.vdsts, extent=19)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=29, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=30, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=29, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=4, data_length=2048, extent_location=30)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BOOT.;1', dr_len=40, loc=32, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BOOT.;1', contents=b'boot\n', which='iso_path')

    internal_check_file(iso.joliet_vd.root_dir_record.children[3], name='boot.cat'.encode('utf-16_be'), dr_len=50, loc=None, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.joliet_vd.root_dir_record.children[2], name='boot'.encode('utf-16_be'), dr_len=42, loc=32, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/boot', contents=b'boot\n', which='joliet_path')

def check_hard_link_reshuffle(iso, filesize):
    assert(filesize == 51200)

    internal_check_pvd(iso.pvd, extent=16, size=25, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=23, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'FOO.;1', dr_len=40, loc=24, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BAR.;1', dr_len=40, loc=24, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BAR.;1', contents=b'foo\n', which='iso_path')

def check_rr_deeper_dir(iso, filesize):
    assert(filesize == 86016)

    internal_check_pvd(iso.pvd, extent=16, size=42, ptbl_size=202, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)
    a1_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(a1_record.ptr, name=b'A1', len_di=2, loc=None, parent=1)
    dir1_record = iso.pvd.root_dir_record.children[3]
    internal_check_ptr(dir1_record.ptr, name=b'DIR1', len_di=4, loc=None, parent=1)
    rr_moved_record = iso.pvd.root_dir_record.children[4]
    internal_check_ptr(rr_moved_record.ptr, name=b'RR_MOVED', len_di=8, loc=None, parent=1)
    a2_record = a1_record.children[2]
    internal_check_ptr(a2_record.ptr, name=b'A2', len_di=2, loc=None, parent=2)
    dir2_record = dir1_record.children[2]
    internal_check_ptr(dir2_record.ptr, name=b'DIR2', len_di=4, loc=None, parent=3)
    a8_record = rr_moved_record.children[2]
    internal_check_ptr(a8_record.ptr, name=b'A8', len_di=2, loc=None, parent=4)
    dir8_record = rr_moved_record.children[3]
    internal_check_ptr(dir8_record.ptr, name=b'DIR8', len_di=4, loc=None, parent=4)
    a3_record = a2_record.children[2]
    internal_check_ptr(a3_record.ptr, name=b'A3', len_di=2, loc=None, parent=5)
    dir3_record = dir2_record.children[2]
    internal_check_ptr(dir3_record.ptr, name=b'DIR3', len_di=4, loc=None, parent=6)
    a4_record = a3_record.children[2]
    internal_check_ptr(a4_record.ptr, name=b'A4', len_di=2, loc=None, parent=9)
    dir4_record = dir3_record.children[2]
    internal_check_ptr(dir4_record.ptr, name=b'DIR4', len_di=4, loc=None, parent=10)
    a5_record = a4_record.children[2]
    internal_check_ptr(a5_record.ptr, name=b'A5', len_di=2, loc=None, parent=11)
    dir5_record = dir4_record.children[2]
    internal_check_ptr(dir5_record.ptr, name=b'DIR5', len_di=4, loc=None, parent=12)
    a6_record = a5_record.children[2]
    internal_check_ptr(a6_record.ptr, name=b'A6', len_di=2, loc=None, parent=13)
    dir6_record = dir5_record.children[2]
    internal_check_ptr(dir6_record.ptr, name=b'DIR6', len_di=4, loc=None, parent=14)
    a7_record = a6_record.children[2]
    internal_check_ptr(a7_record.ptr, name=b'A7', len_di=2, loc=None, parent=15)
    dir7_record = dir6_record.children[2]
    internal_check_ptr(dir7_record.ptr, name=b'DIR7', len_di=4, loc=None, parent=16)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=5, data_length=2048, extent_location=23, rr=True, rr_nlinks=5, xa=False, rr_onetwelve=False)

def check_eltorito_boot_info_table_large_odd(iso, filesize):
    assert(filesize == 57344)

    internal_check_pvd(iso.pvd, extent=16, size=28, ptbl_size=10, ptbl_location_le=21, ptbl_location_be=23)

    internal_check_eltorito(iso, boot_catalog_extent=26, load_rba=27, media_type=0, system_type=0, bootable=True, platform_id=0)

    internal_check_terminator(iso.vdsts, extent=19)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=25, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=25, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'boot.cat', dr_len=42, loc=26, datalen=2048, hidden=False, multi_extent=False)

    boot_rec = iso.pvd.root_dir_record.children[2]
    internal_check_file(boot_rec, name=b'boot', dr_len=38, loc=27, datalen=81, hidden=False, multi_extent=False)

    internal_check_file_contents(iso, path='/boot', contents=b'booboobo\x10\x00\x00\x00\x1b\x00\x00\x00\x51\x00\x00\x00\x1e\xb1\xa3\xb0\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00ooboobooboobooboo', which='iso_path')

    internal_check_boot_info_table(boot_rec.inode.boot_info_table, vd_extent=16, inode_extent=27, orig_len=81, csum=0xb0a3b11e)

def check_joliet_large_directory(iso, filesize):
    assert(filesize == 264192)

    internal_check_pvd(iso.pvd, extent=16, size=129, ptbl_size=678, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_jolietvd(iso.svds[0], space_size=129, path_tbl_size=874, path_tbl_loc_le=24, path_tbl_loc_be=26)

    internal_check_terminator(iso.vdsts, extent=18)

    # FIXME: this test should probably be more comprehensive

def check_zero_byte_file(iso, filesize):
    assert(filesize == 51200)

    internal_check_pvd(iso.pvd, extent=16, size=25, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=23, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'FOO.;1', dr_len=40, loc=None, datalen=0, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'', which='iso_path')

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BAR.;1', dr_len=40, loc=24, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BAR.;1', contents=b'bar\n', which='iso_path')

def check_eltorito_hide_boot(iso, filesize):
    assert(filesize == 55296)

    internal_check_pvd(iso.pvd, extent=16, size=27, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_eltorito(iso, boot_catalog_extent=25, load_rba=26, media_type=0, system_type=0, bootable=True, platform_id=0)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=24, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=24, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BOOT.CAT;1', dr_len=44, loc=25, datalen=2048, hidden=False, multi_extent=False)

    # Here, the initial entry is hidden, so we check it out by manually looking
    # for it in the raw output.  To do that in the current framework, we need
    # to re-write the iso into a string, then search the string.
    initial_entry_offset = iso.eltorito_boot_catalog.initial_entry.get_rba()

    # Re-render the output into a string.
    myout = io.BytesIO()
    iso.write_fp(myout)

    # Now seek within the string to the right location.
    myout.seek(initial_entry_offset * 2048)

    val = myout.read(5)
    assert(val == b'boot\n')

def check_modify_in_place_spillover(iso, filesize):
    assert(filesize == 151552)

    internal_check_pvd(iso.pvd, extent=16, size=74, ptbl_size=22, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=23, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    dir1_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(dir1_record.ptr, name=b'DIR1', len_di=4, loc=24, parent=1)
    internal_check_dir_record(dir1_record, num_children=50, name=b'DIR1', dr_len=38, extent_location=24, rr=False, rr_name=None, rr_links=0, xa=False, hidden=False, is_cl_record=False, datalen=4096, relocated=False)

def check_duplicate_pvd(iso, filesize):
    assert(filesize == 53248)

    internal_check_pvd(iso.pvd, extent=16, size=26, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_pvd(iso.pvds[1], extent=17, size=26, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=24, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=24, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

def check_eltorito_multi_multi_boot(iso, filesize):
    assert(filesize == 61440)

    internal_check_pvd(iso.pvd, extent=16, size=30, ptbl_size=10, ptbl_location_le=21, ptbl_location_be=23)

    internal_check_eltorito(iso, boot_catalog_extent=26, load_rba=27, media_type=0, system_type=0, bootable=True, platform_id=0)

    assert(len(iso.eltorito_boot_catalog.sections) == 2)
    sec = iso.eltorito_boot_catalog.sections[0]
    assert(sec.header_indicator == 0x90)
    assert(sec.platform_id == 0)
    assert(sec.num_section_entries == 1)
    assert(sec.id_string == b'\x00'*28)
    assert(len(sec.section_entries) == 1)
    entry = sec.section_entries[0]
    assert(entry.boot_indicator == 0x88)
    assert(entry.boot_media_type == 0x0)
    assert(entry.load_segment == 0x0)
    assert(entry.system_type == 0)
    assert(entry.sector_count == 4)
    assert(entry.load_rba == 28)

    sec = iso.eltorito_boot_catalog.sections[1]
    assert(sec.header_indicator == 0x91)
    assert(sec.platform_id == 0)
    assert(sec.num_section_entries == 1)
    assert(sec.id_string == b'\x00'*28)
    assert(len(sec.section_entries) == 1)
    entry = sec.section_entries[0]
    assert(entry.boot_indicator == 0x88)
    assert(entry.boot_media_type == 0x0)
    assert(entry.load_segment == 0x0)
    assert(entry.system_type == 0)
    assert(entry.sector_count == 4)
    assert(entry.load_rba == 29)

    internal_check_terminator(iso.vdsts, extent=19)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=25, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=6, data_length=2048, extent_location=25, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'boot.cat', dr_len=42, loc=26, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'boot', dr_len=38, loc=27, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/boot', contents=b'boot\n', which='iso_path')

    internal_check_file(iso.pvd.root_dir_record.children[4], name=b'boot2', dr_len=38, loc=28, datalen=6, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/boot2', contents=b'boot2\n', which='iso_path')

def check_hidden_file(iso, filesize):
    assert(filesize == 51200)

    internal_check_pvd(iso.pvd, extent=16, size=25, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=23, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'AAAAAAAA.;1', dr_len=44, loc=24, datalen=3, hidden=True, multi_extent=False)
    internal_check_file_contents(iso, path='/AAAAAAAA.;1', contents=b'aa\n', which='iso_path')

def check_hidden_dir(iso, filesize):
    assert(filesize == 51200)

    internal_check_pvd(iso.pvd, extent=16, size=25, ptbl_size=22, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=23, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    dir1_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(dir1_record.ptr, name=b'DIR1', len_di=4, loc=24, parent=1)
    internal_check_empty_directory(dir1_record, name=b'DIR1', dr_len=38, extent=24, rr=False, hidden=True)

def check_eltorito_hd_emul(iso, filesize):
    assert(filesize == 55296)

    internal_check_pvd(iso.pvd, extent=16, size=27, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_eltorito(iso, boot_catalog_extent=25, load_rba=26, media_type=4, system_type=2, bootable=True, platform_id=0)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=24, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=24, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'BOOT.CAT;1', dr_len=44, loc=25, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BOOT.;1', dr_len=40, loc=26, datalen=512, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BOOT.;1', contents=b'\x00'*446 + b'\x00\x01\x01\x00\x02\x01\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*16 + b'\x00'*16 + b'\x00'*16 + b'\x55' + b'\xaa', which='iso_path')

def check_eltorito_hd_emul_bad_sec(iso, filesize):
    assert(filesize == 55296)

    internal_check_pvd(iso.pvd, extent=16, size=27, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_eltorito(iso, boot_catalog_extent=25, load_rba=26, media_type=4, system_type=2, bootable=True, platform_id=0)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=24, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=24, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'BOOT.CAT;1', dr_len=44, loc=25, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BOOT.;1', dr_len=40, loc=26, datalen=512, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BOOT.;1', contents=b'\x00'*446 + b'\x00\x00\x00\x00\x02\x01\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*16 + b'\x00'*16 + b'\x00'*16 + b'\x55' + b'\xaa', which='iso_path')

def check_eltorito_hd_emul_invalid_geometry(iso, filesize):
    assert(filesize == 55296)

    internal_check_pvd(iso.pvd, extent=16, size=27, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_eltorito(iso, boot_catalog_extent=25, load_rba=26, media_type=4, system_type=2, bootable=True, platform_id=0)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=24, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=24, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'BOOT.CAT;1', dr_len=44, loc=25, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BOOT.;1', dr_len=40, loc=26, datalen=512, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BOOT.;1', contents=b'\x00'*446 + b'\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*16 + b'\x00'*16 + b'\x00'*16 + b'\x55' + b'\xaa', which='iso_path')

def check_eltorito_hd_emul_not_bootable(iso, filesize):
    assert(filesize == 55296)

    internal_check_pvd(iso.pvd, extent=16, size=27, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_eltorito(iso, boot_catalog_extent=25, load_rba=26, media_type=4, system_type=2, bootable=False, platform_id=0)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=24, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=24, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'BOOT.CAT;1', dr_len=44, loc=25, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BOOT.;1', dr_len=40, loc=26, datalen=512, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BOOT.;1', contents=b'\x00'*446 + b'\x00\x01\x01\x00\x02\x01\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*16 + b'\x00'*16 + b'\x00'*16 + b'\x55' + b'\xaa', which='iso_path')

def check_eltorito_floppy12(iso, filesize):
    assert(filesize == 1282048)

    internal_check_pvd(iso.pvd, extent=16, size=626, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_eltorito(iso, boot_catalog_extent=25, load_rba=26, media_type=1, system_type=0, bootable=True, platform_id=0)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=24, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=24, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'BOOT.CAT;1', dr_len=44, loc=25, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BOOT.;1', dr_len=40, loc=26, datalen=1228800, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BOOT.;1', contents=b'\x00'*(2400*512), which='iso_path')

def check_eltorito_floppy144(iso, filesize):
    assert(filesize == 1527808)

    internal_check_pvd(iso.pvd, extent=16, size=746, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_eltorito(iso, boot_catalog_extent=25, load_rba=26, media_type=2, system_type=0, bootable=True, platform_id=0)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=24, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=24, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'BOOT.CAT;1', dr_len=44, loc=25, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BOOT.;1', dr_len=40, loc=26, datalen=1474560, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BOOT.;1', contents=b'\x00'*(2880*512), which='iso_path')

def check_eltorito_floppy288(iso, filesize):
    assert(filesize == 3002368)

    internal_check_pvd(iso.pvd, extent=16, size=1466, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_eltorito(iso, boot_catalog_extent=25, load_rba=26, media_type=3, system_type=0, bootable=True, platform_id=0)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=24, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=24, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'BOOT.CAT;1', dr_len=44, loc=25, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BOOT.;1', dr_len=40, loc=26, datalen=2949120, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BOOT.;1', contents=b'\x00'*(5760*512), which='iso_path')

def check_eltorito_multi_hidden(iso, filesize):
    assert(filesize == 59392)

    internal_check_pvd(iso.pvd, extent=16, size=29, ptbl_size=10, ptbl_location_le=21, ptbl_location_be=23)

    internal_check_eltorito(iso, boot_catalog_extent=26, load_rba=28, media_type=0, system_type=0, bootable=True, platform_id=0)

    assert(len(iso.eltorito_boot_catalog.sections) == 1)
    sec = iso.eltorito_boot_catalog.sections[0]
    assert(sec.header_indicator == 0x91)
    assert(sec.platform_id == 0)
    assert(sec.num_section_entries == 1)
    assert(sec.id_string == b'\x00'*28)
    assert(len(sec.section_entries) == 1)
    entry = sec.section_entries[0]
    assert(entry.boot_indicator == 0x88)
    assert(entry.boot_media_type == 0x0)
    assert(entry.load_segment == 0x0)
    assert(entry.system_type == 0)
    assert(entry.sector_count == 4)
    assert(entry.load_rba == 27)

    internal_check_terminator(iso.vdsts, extent=19)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=25, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=25, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'boot.cat', dr_len=42, loc=26, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'boot', dr_len=38, loc=28, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/boot', contents=b'boot\n', which='iso_path')

def check_onefile_with_semicolon(iso, filesize):
    assert(filesize == 51200)

    internal_check_pvd(iso.pvd, extent=16, size=25, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=23, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'FOO;1.;1', dr_len=42, loc=24, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO;1.;1', contents=b'foo\n', which='iso_path')

def check_bad_eltorito_ident(iso, filesize):
    assert(filesize == 55296)

    internal_check_pvd(iso.pvd, extent=16, size=27, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    # Because this is a bad eltorito ident, we expect the len(brs) to be > 0,
    # but no eltorito catalog available
    assert(len(iso.brs) == 1)
    assert(iso.eltorito_boot_catalog is None)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=24, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=24, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BOOT.;1', dr_len=40, loc=26, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BOOT.;1', contents=b'boot\n', which='iso_path')

def check_rr_two_dirs_same_level(iso, filesize):
    assert(filesize == 77824)

    # For two relocated directories at the same level with the same name,
    # genisoimage seems to pad the second entry in the PTR with three zeros (000), the
    # third one with 001, etc.  pycdlib does not do this, so the sizes do not match.
    # Hence, for now, we disable this check.
    #internal_check_pvd(iso.pvd, extent=16, size=38, ptbl_size=128, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=23, rr=True, rr_nlinks=4, xa=False, rr_onetwelve=False)

    a_dir_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(a_dir_record.ptr, name=b'A', len_di=1, loc=None, parent=1)
    internal_check_dir_record(a_dir_record, num_children=3, name=b'A', dr_len=108, extent_location=None, rr=True, rr_name=b'A', rr_links=3, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(a_dir_record.children[1], rr=True, rr_nlinks=4, xa=False, rr_onetwelve=False)

    b_dir_record = a_dir_record.children[2]
    internal_check_dir_record(b_dir_record, num_children=3, name=b'B', dr_len=108, extent_location=None, rr=True, rr_name=b'B', rr_links=3, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(b_dir_record.children[1], rr=True, rr_nlinks=3, xa=False, rr_onetwelve=False)

    c_dir_record = b_dir_record.children[2]
    internal_check_dir_record(c_dir_record, num_children=3, name=b'C', dr_len=108, extent_location=29, rr=True, rr_name=b'C', rr_links=3, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(c_dir_record.children[1], rr=True, rr_nlinks=3, xa=False, rr_onetwelve=False)

    d_dir_record = c_dir_record.children[2]
    internal_check_dir_record(d_dir_record, num_children=3, name=b'D', dr_len=108, extent_location=30, rr=True, rr_name=b'D', rr_links=3, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(d_dir_record.children[1], rr=True, rr_nlinks=3, xa=False, rr_onetwelve=False)

    e_dir_record = d_dir_record.children[2]
    internal_check_dir_record(e_dir_record, num_children=3, name=b'E', dr_len=108, extent_location=31, rr=True, rr_name=b'E', rr_links=3, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(e_dir_record.children[1], rr=True, rr_nlinks=3, xa=False, rr_onetwelve=False)

    f_dir_record = e_dir_record.children[2]
    internal_check_dir_record(f_dir_record, num_children=4, name=b'F', dr_len=108, extent_location=32, rr=True, rr_name=b'F', rr_links=4, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(f_dir_record.children[1], rr=True, rr_nlinks=3, xa=False, rr_onetwelve=False)

    g_dir_record = f_dir_record.children[2]
    internal_check_dir_record(g_dir_record, num_children=3, name=b'G', dr_len=108, extent_location=None, rr=True, rr_name=b'G', rr_links=3, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(g_dir_record.children[1], rr=True, rr_nlinks=4, xa=False, rr_onetwelve=False)

    # This is the first of the two relocated entries.
    one_dir_record = g_dir_record.children[2]
    internal_check_dir_record(one_dir_record, num_children=0, name=b'1', dr_len=120, extent_location=None, rr=True, rr_name=b'1', rr_links=2, xa=False, hidden=False, is_cl_record=True, datalen=2048, relocated=False)

    h_dir_record = f_dir_record.children[3]
    internal_check_dir_record(h_dir_record, num_children=3, name=b'H', dr_len=108, extent_location=None, rr=True, rr_name=b'H', rr_links=3, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(g_dir_record.children[1], rr=True, rr_nlinks=4, xa=False, rr_onetwelve=False)

    # This is the second of the two relocated entries.
    one_dir_record = h_dir_record.children[2]
    internal_check_dir_record(one_dir_record, num_children=0, name=b'1', dr_len=120, extent_location=None, rr=True, rr_name=b'1', rr_links=2, xa=False, hidden=False, is_cl_record=True, datalen=2048, relocated=False)

    # Now check the foo file.  It should have a name of FOO.;1, it should
    # have a directory record length of 116, it should start at extent 26, and
    # its contents should be 'foo\n'.
    internal_check_file_contents(iso, path='/A/B/C/D/E/F/G/1/FIRST.;1', contents=b'first\n', which='iso_path')
    internal_check_file_contents(iso, path='/A/B/C/D/E/F/H/1/SECOND.;1', contents=b'second\n', which='iso_path')

def check_eltorito_rr_verylongname(iso, filesize):
    assert(filesize == 59392)

    internal_check_pvd(iso.pvd, extent=16, size=29, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_eltorito(iso, boot_catalog_extent=27, load_rba=28, media_type=0, system_type=0, bootable=True, platform_id=0)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=24, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=24, rr=True, rr_nlinks=2, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'AAAAAAAA.;1', dr_len=None, loc=27, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'BOOT.;1', dr_len=116, loc=28, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BOOT.;1', contents=b'boot\n', which='iso_path')

def check_isohybrid_file_before(iso, filesize):
    assert(filesize == 1048576)

    internal_check_pvd(iso.pvd, extent=16, size=28, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_eltorito(iso, boot_catalog_extent=25, load_rba=26, media_type=0, system_type=0, bootable=True, platform_id=0)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=24, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=5, data_length=2048, extent_location=24, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BOOT.CAT;1', dr_len=44, loc=25, datalen=2048, hidden=False, multi_extent=False)

    # Now check out the isohybrid stuff.
    assert(iso.isohybrid_mbr.geometry_heads == 64)
    assert(iso.isohybrid_mbr.geometry_sectors == 32)
    assert(iso.isohybrid_mbr.rba != 0)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'FOO.;1', dr_len=40, loc=27, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

    internal_check_file(iso.pvd.root_dir_record.children[4], name=b'ISOLINUX.BIN;1', dr_len=48, loc=26, datalen=68, hidden=False, multi_extent=False)

def check_eltorito_rr_joliet_verylongname(iso, filesize):
    assert(filesize == 71680)

    internal_check_pvd(iso.pvd, extent=16, size=35, ptbl_size=10, ptbl_location_le=21, ptbl_location_be=23)

    internal_check_eltorito(iso, boot_catalog_extent=33, load_rba=34, media_type=0, system_type=0, bootable=True, platform_id=0)

    internal_check_jolietvd(iso.svds[0], space_size=35, path_tbl_size=10, path_tbl_loc_le=25, path_tbl_loc_be=27)

    internal_check_terminator(iso.vdsts, extent=19)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=29, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=31, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=29, rr=True, rr_nlinks=2, xa=False, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=4, data_length=2048, extent_location=31)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'AAAAAAAA.;1', dr_len=None, loc=33, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'BOOT.;1', dr_len=116, loc=34, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BOOT.;1', contents=b'boot\n', which='iso_path')

    joliet_name = 'a'*64
    internal_check_file(iso.joliet_vd.root_dir_record.children[2], name=joliet_name.encode('utf-16_be'), dr_len=162, loc=33, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.joliet_vd.root_dir_record.children[3], name='boot'.encode('utf-16_be'), dr_len=42, loc=34, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/boot', contents=b'boot\n', which='joliet_path')

def check_joliet_dirs_overflow_ptr_extent(iso, filesize):
    assert(filesize == 970752)

    internal_check_pvd(iso.pvd, extent=16, size=474, ptbl_size=3016, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_jolietvd(iso.svds[0], space_size=474, path_tbl_size=4114, path_tbl_loc_le=24, path_tbl_loc_be=28)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=216+2, data_length=10240, extent_location=32, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=32, parent=1)
    # The rest of the path table records will be checked by the loop below.

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=253, parent=1)

    names = internal_generate_joliet_inorder_names(216)
    for index in range(2, 2+216):
        joliet_dir_record = iso.joliet_vd.root_dir_record.children[index]
        # We skip checking the path table record extent locations because
        # genisoimage seems to have a bug assigning the extent locations, and
        # seems to assign them in reverse order.
        internal_check_ptr(joliet_dir_record.ptr, name=names[index], len_di=len(names[index]), loc=None, parent=1)

def check_joliet_dirs_just_short_ptr_extent(iso, filesize):
    assert(filesize == 958464)

    internal_check_pvd(iso.pvd, extent=16, size=468, ptbl_size=3002, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_jolietvd(iso.svds[0], space_size=468, path_tbl_size=4094, path_tbl_loc_le=24, path_tbl_loc_be=26)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=215+2, data_length=10240, extent_location=28, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=28, parent=1)
    # The rest of the path table records will be checked by the loop below.

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=248, parent=1)

    names = internal_generate_joliet_inorder_names(215)
    for index in range(2, 2+215):
        joliet_dir_record = iso.joliet_vd.root_dir_record.children[index]
        # We skip checking the path table record extent locations because
        # genisoimage seems to have a bug assigning the extent locations, and
        # seems to assign them in reverse order.
        internal_check_ptr(joliet_dir_record.ptr, name=names[index], len_di=len(names[index]), loc=None, parent=1)

def check_joliet_dirs_add_ptr_extent(iso, filesize):
    assert(filesize == 1308672)

    internal_check_pvd(iso.pvd, extent=16, size=639, ptbl_size=4122, ptbl_location_le=20, ptbl_location_be=24)

    internal_check_jolietvd(iso.svds[0], space_size=639, path_tbl_size=5694, path_tbl_loc_le=28, path_tbl_loc_be=32)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=295+2, data_length=12288, extent_location=36, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=36, parent=1)
    # The rest of the path table records will be checked by the loop below.

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=337, parent=1)

    names = internal_generate_joliet_inorder_names(295)
    for index in range(2, 2+295):
        joliet_dir_record = iso.joliet_vd.root_dir_record.children[index]
        # We skip checking the path table record extent locations because
        # genisoimage seems to have a bug assigning the extent locations, and
        # seems to assign them in reverse order.
        internal_check_ptr(joliet_dir_record.ptr, name=names[index], len_di=len(names[index]), loc=None, parent=1)

def check_joliet_dirs_rm_ptr_extent(iso, filesize):
    assert(filesize == 1292288)

    internal_check_pvd(iso.pvd, extent=16, size=631, ptbl_size=4094, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_jolietvd(iso.svds[0], space_size=631, path_tbl_size=5654, path_tbl_loc_le=24, path_tbl_loc_be=28)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=293+2, data_length=12288, extent_location=32, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=32, parent=1)
    # The rest of the path table records will be checked by the loop below.

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=331, parent=1)

    names = internal_generate_joliet_inorder_names(293)
    for index in range(2, 2+293):
        joliet_dir_record = iso.joliet_vd.root_dir_record.children[index]
        # We skip checking the path table record extent locations because
        # genisoimage seems to have a bug assigning the extent locations, and
        # seems to assign them in reverse order.
        internal_check_ptr(joliet_dir_record.ptr, name=names[index], len_di=len(names[index]), loc=None, parent=1)

def check_long_directory_name(iso, filesize):
    assert(filesize == 51200)

    internal_check_pvd(iso.pvd, extent=16, size=25, ptbl_size=28, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=23, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    directory1_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(directory1_record.ptr, name=b'DIRECTORY1', len_di=10, loc=24, parent=1)
    internal_check_empty_directory(directory1_record, name=b'DIRECTORY1', dr_len=44, extent=24, rr=False, hidden=False)

def check_long_file_name(iso, filesize):
    assert(filesize == 51200)

    internal_check_pvd(iso.pvd, extent=16, size=25, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=23, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'FOOBARBAZ1.;1', dr_len=46, loc=24, datalen=11, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOOBARBAZ1.;1', contents=b'foobarbaz1\n', which='iso_path')

def check_overflow_root_dir_record(iso, filesize):
    assert(filesize == 94208)

    internal_check_pvd(iso.pvd, extent=16, size=46, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=28, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=16, data_length=4096, extent_location=28, rr=True, rr_nlinks=2, xa=False, rr_onetwelve=False)

    internal_check_dot_dir_record(iso.pvd.root_dir_record.children[0], rr=True, rr_nlinks=2, first_dot=True, xa=False, datalen=4096, rr_onetwelve=False)

def check_overflow_correct_extents(iso, filesize):
    assert(filesize == 102400)

    internal_check_pvd(iso.pvd, extent=16, size=50, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=28, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=18, data_length=6144, extent_location=28, rr=True, rr_nlinks=2, xa=False, rr_onetwelve=False)

    internal_check_dot_dir_record(iso.pvd.root_dir_record.children[0], rr=True, rr_nlinks=2, first_dot=True, xa=False, datalen=6144, rr_onetwelve=False)

def check_duplicate_deep_dir(iso, filesize):
    assert(filesize == 135168)

    internal_check_pvd(iso.pvd, extent=16, size=66, ptbl_size=216, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=28, parent=1)
    books_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(books_record.ptr, name=b'BOOKS', len_di=5, loc=None, parent=1)
    rr_moved_record = iso.pvd.root_dir_record.children[3]
    internal_check_ptr(rr_moved_record.ptr, name=b'RR_MOVED', len_di=8, loc=None, parent=1)
    lkhg_record = books_record.children[2]
    internal_check_ptr(lkhg_record.ptr, name=b'LKHG', len_di=4, loc=None, parent=2)
    first_one_record = rr_moved_record.children[2]
    internal_check_ptr(first_one_record.ptr, name=b'1', len_di=1, loc=None, parent=3)
    one_thousand_record = rr_moved_record.children[3]
    internal_check_ptr(one_thousand_record.ptr, name=b'1000', len_di=4, loc=None, parent=3)
    hypernew_record = lkhg_record.children[2]
    internal_check_ptr(hypernew_record.ptr, name=b'HYPERNEW', len_di=8, loc=None, parent=4)
    get_record = hypernew_record.children[2]
    internal_check_ptr(get_record.ptr, name=b'GET', len_di=3, loc=None, parent=7)
    first_fs_record = get_record.children[2]
    internal_check_ptr(first_fs_record.ptr, name=b'FS', len_di=2, loc=None, parent=9)
    khg_record = get_record.children[3]
    internal_check_ptr(khg_record.ptr, name=b'KHG', len_di=3, loc=None, parent=9)
    second_fs_record = first_fs_record.children[2]
    internal_check_ptr(second_fs_record.ptr, name=b'FS', len_di=2, loc=None, parent=11)
    fourth_one_record = khg_record.children[2]
    internal_check_ptr(fourth_one_record.ptr, name=b'1', len_di=1, loc=None, parent=12)
    one_one_seven_record = khg_record.children[3]
    internal_check_ptr(one_one_seven_record.ptr, name=b'117', len_di=3, loc=None, parent=12)
    thirty_five_record = khg_record.children[4]
    internal_check_ptr(thirty_five_record.ptr, name=b'35', len_di=2, loc=None, parent=12)
    fifth_one_record = second_fs_record.children[2]
    internal_check_ptr(fifth_one_record.ptr, name=b'1', len_di=1, loc=None, parent=13)
    sixth_one_record = one_one_seven_record.children[2]
    internal_check_ptr(sixth_one_record.ptr, name=b'1', len_di=1, loc=None, parent=15)
    seventh_one_record = thirty_five_record.children[2]
    internal_check_ptr(seventh_one_record.ptr, name=b'1', len_di=1, loc=None, parent=16)

    # This is the second of the two relocated entries.
    rr_moved_dir_record = iso.pvd.root_dir_record.children[3]
    internal_check_dir_record(rr_moved_dir_record, num_children=4, name=b'RR_MOVED', dr_len=122, extent_location=None, rr=True, rr_name=b'rr_moved', rr_links=4, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)

    # In theory we should check the dir_records underneath rr_moved here.
    # Unfortunately, which directory gets renamed to 1000 is unstable,
    # and thus we don't know which record it is.  We skip the check for now,
    # although we could go grubbing through the children to try and find it.

def check_onefile_joliet_no_file(iso, filesize):
    assert(filesize == 63488)

    internal_check_pvd(iso.pvd, extent=16, size=31, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_jolietvd(iso.svds[0], space_size=31, path_tbl_size=10, path_tbl_loc_le=24, path_tbl_loc_be=26)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=28, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=29, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=28, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=2, data_length=2048, extent_location=29)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'FOO.;1', dr_len=40, loc=30, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

def check_joliet_isolevel4_nofiles(iso, filesize):
    assert(filesize == 63488)

    internal_check_pvd(iso.pvd, extent=16, size=31, ptbl_size=10, ptbl_location_le=21, ptbl_location_be=23)

    internal_check_jolietvd(iso.joliet_vd, space_size=31, path_tbl_size=10, path_tbl_loc_le=25, path_tbl_loc_be=27)

    internal_check_enhanced_vd(iso.enhanced_vd, size=31, ptbl_size=10, ptbl_location_le=21, ptbl_location_be=23)

    internal_check_terminator(iso.vdsts, extent=19)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=29, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=30, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=2, data_length=2048, extent_location=29, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=2, data_length=2048, extent_location=30)

def check_rr_absolute_symlink(iso, filesize):
    assert(filesize == 51200)

    internal_check_pvd(iso.pvd, extent=16, size=25, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=23, rr=True, rr_nlinks=2, xa=False, rr_onetwelve=False)

    # Now check the rock ridge symlink.  It should have a directory record
    # length of 126, and the symlink components should be 'foo'.
    sym_dir_record = iso.pvd.root_dir_record.children[2]
    internal_check_rr_symlink(sym_dir_record, name=b'SYM.;1', dr_len=140, comps=[b'', b'usr', b'local', b'foo'])

def check_deep_rr_symlink(iso, filesize):
    assert(filesize == 65536)

    internal_check_pvd(iso.pvd, extent=16, size=32, ptbl_size=94, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=23, rr=True, rr_nlinks=3, xa=False, rr_onetwelve=False)

    dir1_record = iso.pvd.root_dir_record.children[2]
    dir2_record = dir1_record.children[2]
    dir3_record = dir2_record.children[2]
    dir4_record = dir3_record.children[2]
    dir5_record = dir4_record.children[2]
    dir6_record = dir5_record.children[2]
    dir7_record = dir6_record.children[2]

    # Now check the rock ridge symlink.  It should have a directory record
    # length of 126, and the symlink components should be 'foo'.
    sym_dir_record = dir7_record.children[2]
    internal_check_rr_symlink(sym_dir_record, name=b'SYM.;1', dr_len=140, comps=[b'', b'usr', b'share', b'foo'])

def check_rr_deep_weird_layout(iso, filesize):
    assert(filesize == 73728)

    internal_check_pvd(iso.pvd, extent=16, size=36, ptbl_size=146, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=23, rr=True, rr_nlinks=4, xa=False, rr_onetwelve=False)

    astroid_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(astroid_record.ptr, name=b'ASTROID', len_di=7, loc=None, parent=1)
    internal_check_dir_record(astroid_record, num_children=3, name=b'ASTROID', dr_len=120, extent_location=None, rr=True, rr_name=b'astroid', rr_links=3, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)

    rr_moved_record = iso.pvd.root_dir_record.children[3]
    internal_check_ptr(rr_moved_record.ptr, name=b'RR_MOVED', len_di=8, loc=None, parent=1)

    astroid2_record = astroid_record.children[2]
    internal_check_ptr(astroid2_record.ptr, name=b'ASTROID', len_di=7, loc=None, parent=2)

    sidepack_record = rr_moved_record.children[2]
    internal_check_ptr(sidepack_record.ptr, name=b'SIDEPACK', len_di=8, loc=None, parent=3)

    tests_record = astroid2_record.children[2]
    internal_check_ptr(tests_record.ptr, name=b'TESTS', len_di=5, loc=28, parent=4)

    testdata_record = tests_record.children[2]
    internal_check_ptr(testdata_record.ptr, name=b'TESTDATA', len_di=8, loc=29, parent=6)

    python3_record = testdata_record.children[2]
    internal_check_ptr(python3_record.ptr, name=b'PYTHON3', len_di=7, loc=30, parent=7)

    data_record = python3_record.children[2]
    internal_check_ptr(data_record.ptr, name=b'DATA', len_di=4, loc=31, parent=8)

    absimp_record = data_record.children[2]
    internal_check_ptr(absimp_record.ptr, name=b'ABSIMP', len_di=6, loc=32, parent=9)

    internal_check_file_contents(iso, path='/ASTROID/ASTROID/TESTS/TESTDATA/PYTHON3/DATA/ABSIMP/STRING.PY;1', contents=b'from __future__ import absolute_import, print_functino\nimport string\nprint(string)\n', which='iso_path')

    internal_check_file_contents(iso, path='/ASTROID/ASTROID/TESTS/TESTDATA/PYTHON3/DATA/ABSIMP/SIDEPACK/__INIT__.PY;1', contents=b'"""a side package with nothing in it\n"""\n', which='iso_path')

def check_rr_long_dir_name(iso, filesize):
    assert(filesize == 55296)

    internal_check_pvd(iso.pvd, extent=16, size=27, ptbl_size=26, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=23, rr=True, rr_nlinks=3, xa=False, rr_onetwelve=False)

    aa_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(aa_record.ptr, name=b'AAAAAAAA', len_di=8, loc=None, parent=1)
    internal_check_dir_record(aa_record, num_children=2, name=b'AAAAAAAA', dr_len=None, extent_location=None, rr=True, rr_name=b'a'*RR_MAX_FILENAME_LENGTH, rr_links=2, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)

def check_rr_out_of_order_ce(iso, filesize):
    assert(filesize == 55296)

    internal_check_pvd(iso.pvd, extent=16, size=27, ptbl_size=26, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=23, rr=True, rr_nlinks=3, xa=False, rr_onetwelve=False)

    aa_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(aa_record.ptr, name=b'AAAAAAAA', len_di=8, loc=None, parent=1)
    internal_check_dir_record(aa_record, num_children=2, name=b'AAAAAAAA', dr_len=None, extent_location=None, rr=True, rr_name=b'a'*RR_MAX_FILENAME_LENGTH, rr_links=2, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)

    # Now check the rock ridge symlink.  It should have a directory record
    # length of 126, and the symlink components should be 'foo'.
    sym_dir_record = iso.pvd.root_dir_record.children[3]
    internal_check_rr_symlink(sym_dir_record, name=b'SYM.;1', dr_len=254, comps=[b'a'*RR_MAX_FILENAME_LENGTH, b'b'*RR_MAX_FILENAME_LENGTH, b'c'*RR_MAX_FILENAME_LENGTH, b'd'*RR_MAX_FILENAME_LENGTH, b'e'*RR_MAX_FILENAME_LENGTH])

def check_rr_ce_removal(iso, filesize):
    assert(filesize == 61440)

    internal_check_pvd(iso.pvd, extent=16, size=30, ptbl_size=74, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    ee_record = iso.pvd.root_dir_record.children[5]
    internal_check_ptr(ee_record.ptr, name=b'EEEEEEEE', len_di=8, loc=None, parent=1)
    internal_check_dir_record(ee_record, num_children=2, name=b'EEEEEEEE', dr_len=None, extent_location=None, rr=True, rr_name=b'e'*RR_MAX_FILENAME_LENGTH, rr_links=2, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)

def check_rr_relocated_hidden(iso, filesize):
    assert(filesize == 73728)

    internal_check_pvd(iso.pvd, extent=16, size=36, ptbl_size=134, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    dir1_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(dir1_record.ptr, name=b'DIR1', len_di=4, loc=None, parent=1)
    rr_moved_record = iso.pvd.root_dir_record.children[3]
    internal_check_ptr(rr_moved_record.ptr, name=b'_RR_MOVE', len_di=8, loc=None, parent=1)
    dir2_record = dir1_record.children[2]
    internal_check_ptr(dir2_record.ptr, name=b'DIR2', len_di=4, loc=None, parent=2)
    dir8_record = rr_moved_record.children[2]
    internal_check_ptr(dir8_record.ptr, name=b'DIR8', len_di=4, loc=None, parent=3)
    dir3_record = dir2_record.children[2]
    internal_check_ptr(dir3_record.ptr, name=b'DIR3', len_di=4, loc=None, parent=4)
    dir9_record = dir8_record.children[2]
    internal_check_ptr(dir9_record.ptr, name=b'DIR9', len_di=4, loc=None, parent=5)
    dir4_record = dir3_record.children[2]
    internal_check_ptr(dir4_record.ptr, name=b'DIR4', len_di=4, loc=None, parent=6)
    dir5_record = dir4_record.children[2]
    internal_check_ptr(dir5_record.ptr, name=b'DIR5', len_di=4, loc=None, parent=8)
    dir6_record = dir5_record.children[2]
    internal_check_ptr(dir6_record.ptr, name=b'DIR6', len_di=4, loc=None, parent=9)
    dir7_record = dir6_record.children[2]
    internal_check_ptr(dir7_record.ptr, name=b'DIR7', len_di=4, loc=None, parent=10)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=23, rr=True, rr_nlinks=4, xa=False, rr_onetwelve=False)

    internal_check_file_contents(iso, path='/dir1/dir2/dir3/dir4/dir5/dir6/dir7/dir8/dir9/foo', contents=b'foo\n', which='rr_path')

def check_duplicate_pvd_joliet(iso, filesize):
    assert(filesize == 65536)

    internal_check_pvd(iso.pvd, extent=16, size=32, ptbl_size=10, ptbl_location_le=21, ptbl_location_be=23)

    internal_check_pvd(iso.pvds[1], extent=17, size=32, ptbl_size=10, ptbl_location_le=21, ptbl_location_be=23)

    internal_check_jolietvd(iso.svds[0], space_size=32, path_tbl_size=10, path_tbl_loc_le=25, path_tbl_loc_be=27)

    internal_check_terminator(iso.vdsts, extent=19)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=29, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=29, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=2, data_length=2048, extent_location=30)

    internal_check_root_dir_record(iso.pvds[1].root_dir_record, num_children=3, data_length=2048, extent_location=29, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'FOO.;1', dr_len=40, loc=31, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

    internal_check_file(iso.pvds[1].root_dir_record.children[2], name=b'FOO.;1', dr_len=40, loc=31, datalen=4, hidden=False, multi_extent=False)

def check_onefile_toolong(iso, filesize):
    assert(filesize == 51200)

    internal_check_pvd(iso.pvd, extent=16, size=25, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=23, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'FOO.;1', dr_len=40, loc=24, datalen=2048, hidden=False, multi_extent=False)

def check_pvd_zero_datetime(iso, filesize):
    assert(filesize == 49152)

    internal_check_pvd(iso.pvd, extent=16, size=24, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=2, data_length=2048, extent_location=23, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    assert(iso.pvd.volume_creation_date.year == 0)
    assert(iso.pvd.volume_creation_date.month == 0)
    assert(iso.pvd.volume_creation_date.dayofmonth == 0)
    assert(iso.pvd.volume_creation_date.hour == 0)
    assert(iso.pvd.volume_creation_date.minute == 0)
    assert(iso.pvd.volume_creation_date.second == 0)
    assert(iso.pvd.volume_creation_date.hundredthsofsecond == 0)
    assert(iso.pvd.volume_creation_date.gmtoffset == 0)

def check_joliet_different_names(iso, filesize):
    assert(filesize == 67584)

    internal_check_pvd(iso.pvd, extent=16, size=33, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_jolietvd(iso.svds[0], space_size=33, path_tbl_size=10, path_tbl_loc_le=24, path_tbl_loc_be=26)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=28, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=29, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=28, rr=True, rr_nlinks=2, xa=False, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=4, data_length=2048, extent_location=29)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'FOO.;1', dr_len=116, loc=31, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'FOOJ.;1', dr_len=116, loc=32, datalen=10, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOOJ.;1', contents=b'foojoliet\n', which='iso_path')

def check_hidden_joliet_file(iso, size):
    assert(size == 63488)

    internal_check_pvd(iso.pvd, extent=16, size=31, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_jolietvd(iso.svds[0], space_size=31, path_tbl_size=10, path_tbl_loc_le=24, path_tbl_loc_be=26)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=28, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=29, parent=1)

def check_hidden_joliet_dir(iso, size):
    assert(size == 65536)

    internal_check_pvd(iso.pvd, extent=16, size=32, ptbl_size=22, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_jolietvd(iso.svds[0], space_size=32, path_tbl_size=26, path_tbl_loc_le=24, path_tbl_loc_be=26)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=28, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=30, parent=1)
    dir1_record = iso.joliet_vd.root_dir_record.children[2]
    internal_check_ptr(dir1_record.ptr, name='dir1'.encode('utf-16_be'), len_di=8, loc=31, parent=1)

def check_rr_onefileonedir_hidden(iso, filesize):
    assert(filesize == 55296)

    internal_check_pvd(iso.pvd, extent=16, size=27, ptbl_size=22, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=23, rr=True, rr_nlinks=3, xa=False, rr_onetwelve=False)

    dir1_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(dir1_record.ptr, name=b'DIR1', len_di=4, loc=24, parent=1)

    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

    foo_dir_record = iso.pvd.root_dir_record.children[3]
    internal_check_rr_file(foo_dir_record, name=b'foo')
    internal_check_file_contents(iso, path='/foo', contents=b'foo\n', which='rr_path')

    internal_check_file(foo_dir_record, name=b'FOO.;1', dr_len=116, loc=26, datalen=4, hidden=True, multi_extent=False)
    internal_check_empty_directory(dir1_record, name=b'DIR1', dr_len=114, extent=24, rr=True, hidden=True)

def check_rr_onefile_onetwelve(iso, size):
    assert(size == 53248)

    internal_check_pvd(iso.pvd, extent=16, size=26, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=23, rr=True, rr_nlinks=2, xa=False, rr_onetwelve=True)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'FOO.;1', dr_len=118, loc=25, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')
    internal_check_file_contents(iso, path='/foo', contents=b'foo\n', which='rr_path')

def check_joliet_ident_encoding(iso, filesize):
    assert(filesize == 69632)

    internal_check_pvd(iso.pvd, extent=16, size=34, ptbl_size=10, ptbl_location_le=21, ptbl_location_be=23)

    internal_check_enhanced_vd(iso.enhanced_vd, size=34, ptbl_size=10, ptbl_location_le=21, ptbl_location_be=23)

    internal_check_jolietvd(iso.joliet_vd, space_size=34, path_tbl_size=10, path_tbl_loc_le=25, path_tbl_loc_be=27)
    assert(iso.joliet_vd.volume_identifier == 'cidata'.ljust(16, ' ').encode('utf-16_be'))
    assert(iso.joliet_vd.system_identifier == 'LINUX'.ljust(16, ' ').encode('utf-16_be'))

    internal_check_terminator(iso.vdsts, extent=19)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=29, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=30, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=29, rr=True, rr_nlinks=2, xa=False, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=4, data_length=2048, extent_location=30)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'meta-data', dr_len=124, loc=32, datalen=25, hidden=False, multi_extent=False)
    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'user-data', dr_len=124, loc=33, datalen=78, hidden=False, multi_extent=False)

    internal_check_file(iso.joliet_vd.root_dir_record.children[2], name='meta-data'.encode('utf-16_be'), dr_len=52, loc=32, datalen=25, hidden=False, multi_extent=False)
    internal_check_file(iso.joliet_vd.root_dir_record.children[3], name='user-data'.encode('utf-16_be'), dr_len=52, loc=33, datalen=78, hidden=False, multi_extent=False)

def check_duplicate_pvd_isolevel4(iso, filesize):
    assert(filesize == 55296)

    internal_check_pvd(iso.pvd, extent=16, size=27, ptbl_size=10, ptbl_location_le=21, ptbl_location_be=23)

    internal_check_pvd(iso.pvds[1], extent=17, size=27, ptbl_size=10, ptbl_location_le=21, ptbl_location_be=23)

    internal_check_enhanced_vd(iso.enhanced_vd, size=27, ptbl_size=10, ptbl_location_le=21, ptbl_location_be=23)

    internal_check_terminator(iso.vdsts, extent=19)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=25, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=25, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

def check_joliet_hidden_iso_file(iso, filesize):
    assert(filesize == 63488)

    internal_check_pvd(iso.pvd, extent=16, size=31, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_jolietvd(iso.svds[0], space_size=31, path_tbl_size=10, path_tbl_loc_le=24, path_tbl_loc_be=26)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=28, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=29, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=2, data_length=2048, extent_location=28, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=3, data_length=2048, extent_location=29)

    internal_check_file(iso.joliet_vd.root_dir_record.children[2], name='foo'.encode('utf-16_be'), dr_len=40, loc=30, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/foo', contents=b'foo\n', which='joliet_path')

def check_eltorito_bootlink(iso, filesize):
    assert(filesize == 55296)

    internal_check_pvd(iso.pvd, extent=16, size=27, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_eltorito(iso, boot_catalog_extent=25, load_rba=26, media_type=0, system_type=0, bootable=True, platform_id=0)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=24, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=24, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BOOT.CAT;1', dr_len=44, loc=25, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'BOOTLINK.;1', dr_len=44, loc=26, datalen=5, hidden=False, multi_extent=False)

    # Here, the initial entry is hidden, so we check it out by manually looking
    # for it in the raw output.  To do that in the current framework, we need
    # to re-write the iso into a string, then search the string.
    initial_entry_offset = iso.eltorito_boot_catalog.initial_entry.get_rba()

    # Re-render the output into a string.
    myout = io.BytesIO()
    iso.write_fp(myout)

    # Now seek within the string to the right location.
    myout.seek(initial_entry_offset * 2048)

    val = myout.read(5)
    assert(val == b'boot\n')

def check_udf_nofiles(iso, filesize):
    assert(filesize == 546816)

    internal_check_pvd(iso.pvd, extent=16, size=267, ptbl_size=10, ptbl_location_le=261, ptbl_location_be=263)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=265, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=2, data_length=2048, extent_location=265, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_udf_headers(iso, bea_extent=18, end_anchor_extent=266, part_length=9, unique_id=261, num_dirs=1, num_files=0)

    internal_check_udf_file_entry(iso.udf_root, location=259, tag_location=2, num_links=1, info_len=40, num_blocks_recorded=1, num_fi_descs=1, file_type='dir', num_alloc_descs=1)

    internal_check_udf_file_ident_desc(iso.udf_root.fi_descs[0], extent=260, tag_location=3, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

def check_udf_onedir(iso, filesize):
    assert(filesize == 552960)

    internal_check_pvd(iso.pvd, extent=16, size=270, ptbl_size=22, ptbl_location_le=263, ptbl_location_be=265)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=267, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=267, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    dir1_record = iso.pvd.root_dir_record.children[2]
    internal_check_empty_directory(dir1_record, name=b'DIR1', dr_len=38, extent=268, rr=False, hidden=False)

    internal_check_udf_headers(iso, bea_extent=18, end_anchor_extent=269, part_length=12, unique_id=263, num_dirs=2, num_files=0)

    internal_check_udf_file_entry(iso.udf_root, location=259, tag_location=2, num_links=2, info_len=84, num_blocks_recorded=1, num_fi_descs=2, file_type='dir', num_alloc_descs=1)
    internal_check_udf_file_ident_desc(iso.udf_root.fi_descs[0], extent=260, tag_location=3, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    dir1_file_ident = iso.udf_root.fi_descs[1]
    internal_check_udf_file_ident_desc(dir1_file_ident, extent=260, tag_location=3, characteristics=2, blocknum=4, abs_blocknum=261, name=b'dir1', isparent=False, isdir=True)

    dir1_file_entry = dir1_file_ident.file_entry
    internal_check_udf_file_entry(dir1_file_entry, location=261, tag_location=4, num_links=1, info_len=40, num_blocks_recorded=1, num_fi_descs=1, file_type='dir', num_alloc_descs=1)
    internal_check_udf_file_ident_desc(dir1_file_entry.fi_descs[0], extent=262, tag_location=5, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

def check_udf_twodirs(iso, filesize):
    assert(filesize == 559104)

    internal_check_pvd(iso.pvd, extent=16, size=273, ptbl_size=34, ptbl_location_le=265, ptbl_location_be=267)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=269, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=269, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_udf_headers(iso, bea_extent=18, end_anchor_extent=272, part_length=15, unique_id=265, num_dirs=3, num_files=0)

    internal_check_udf_file_entry(iso.udf_root, location=259, tag_location=2, num_links=3, info_len=128, num_blocks_recorded=1, num_fi_descs=3, file_type='dir', num_alloc_descs=1)
    internal_check_udf_file_ident_desc(iso.udf_root.fi_descs[0], extent=260, tag_location=3, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    dir1_file_ident = iso.udf_root.fi_descs[1]
    internal_check_udf_file_ident_desc(dir1_file_ident, extent=260, tag_location=3, characteristics=2, blocknum=None, abs_blocknum=None, name=b'dir1', isparent=False, isdir=True)

    dir1_file_entry = dir1_file_ident.file_entry
    internal_check_udf_file_entry(dir1_file_entry, location=None, tag_location=None, num_links=1, info_len=40, num_blocks_recorded=1, num_fi_descs=1, file_type='dir', num_alloc_descs=1)
    internal_check_udf_file_ident_desc(dir1_file_entry.fi_descs[0], extent=None, tag_location=None, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    dir2_file_ident = iso.udf_root.fi_descs[2]
    internal_check_udf_file_ident_desc(dir2_file_ident, extent=260, tag_location=3, characteristics=2, blocknum=None, abs_blocknum=None, name=b'dir2', isparent=False, isdir=True)

    dir2_file_entry = dir2_file_ident.file_entry
    internal_check_udf_file_entry(dir2_file_entry, location=None, tag_location=None, num_links=1, info_len=40, num_blocks_recorded=1, num_fi_descs=1, file_type='dir', num_alloc_descs=1)
    internal_check_udf_file_ident_desc(dir2_file_entry.fi_descs[0], extent=None, tag_location=None, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

def check_udf_subdir(iso, filesize):
    assert(filesize == 559104)

    internal_check_pvd(iso.pvd, extent=16, size=273, ptbl_size=38, ptbl_location_le=265, ptbl_location_be=267)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=269, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=269, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_udf_headers(iso, bea_extent=18, end_anchor_extent=272, part_length=15, unique_id=265, num_dirs=3, num_files=0)

    internal_check_udf_file_entry(iso.udf_root, location=259, tag_location=2, num_links=2, info_len=84, num_blocks_recorded=1, num_fi_descs=2, file_type='dir', num_alloc_descs=1)
    internal_check_udf_file_ident_desc(iso.udf_root.fi_descs[0], extent=260, tag_location=3, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    dir1_file_ident = iso.udf_root.fi_descs[1]
    internal_check_udf_file_ident_desc(dir1_file_ident, extent=260, tag_location=3, characteristics=2, blocknum=4, abs_blocknum=261, name=b'dir1', isparent=False, isdir=True)

    dir1_file_entry = dir1_file_ident.file_entry
    internal_check_udf_file_entry(dir1_file_entry, location=261, tag_location=4, num_links=2, info_len=88, num_blocks_recorded=1, num_fi_descs=2, file_type='dir', num_alloc_descs=1)
    internal_check_udf_file_ident_desc(dir1_file_entry.fi_descs[0], extent=262, tag_location=5, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    subdir1_file_ident = dir1_file_entry.fi_descs[1]
    internal_check_udf_file_ident_desc(subdir1_file_ident, extent=262, tag_location=5, characteristics=2, blocknum=6, abs_blocknum=263, name=b'subdir1', isparent=False, isdir=True)

    subdir1_file_entry = subdir1_file_ident.file_entry
    internal_check_udf_file_entry(subdir1_file_entry, location=263, tag_location=6, num_links=1, info_len=40, num_blocks_recorded=1, num_fi_descs=1, file_type='dir', num_alloc_descs=1)
    internal_check_udf_file_ident_desc(subdir1_file_entry.fi_descs[0], extent=264, tag_location=7, characteristics=10, blocknum=None, abs_blocknum=None, name=b'', isparent=True, isdir=True)

def check_udf_subdir_odd(iso, filesize):
    assert(filesize == 559104)

    internal_check_pvd(iso.pvd, extent=16, size=273, ptbl_size=36, ptbl_location_le=265, ptbl_location_be=267)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=269, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=269, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_udf_headers(iso, bea_extent=18, end_anchor_extent=272, part_length=15, unique_id=265, num_dirs=3, num_files=0)

    internal_check_udf_file_entry(iso.udf_root, location=259, tag_location=2, num_links=2, info_len=84, num_blocks_recorded=1, num_fi_descs=2, file_type='dir', num_alloc_descs=1)
    internal_check_udf_file_ident_desc(iso.udf_root.fi_descs[0], extent=260, tag_location=3, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    dir1_file_ident = iso.udf_root.fi_descs[1]
    internal_check_udf_file_ident_desc(dir1_file_ident, extent=260, tag_location=3, characteristics=2, blocknum=4, abs_blocknum=261, name=b'dir1', isparent=False, isdir=True)

    dir1_file_entry = dir1_file_ident.file_entry
    internal_check_udf_file_entry(dir1_file_entry, location=261, tag_location=4, num_links=2, info_len=88, num_blocks_recorded=1, num_fi_descs=2, file_type='dir', num_alloc_descs=1)
    internal_check_udf_file_ident_desc(dir1_file_entry.fi_descs[0], extent=262, tag_location=5, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    subdi1_file_ident = dir1_file_entry.fi_descs[1]
    internal_check_udf_file_ident_desc(subdi1_file_ident, extent=262, tag_location=5, characteristics=2, blocknum=6, abs_blocknum=263, name=b'subdi1', isparent=False, isdir=True)

    subdi1_file_entry = subdi1_file_ident.file_entry
    internal_check_udf_file_entry(subdi1_file_entry, location=263, tag_location=6, num_links=1, info_len=40, num_blocks_recorded=1, num_fi_descs=1, file_type='dir', num_alloc_descs=1)
    internal_check_udf_file_ident_desc(subdi1_file_entry.fi_descs[0], extent=264, tag_location=7, characteristics=10, blocknum=None, abs_blocknum=None, name=b'', isparent=True, isdir=True)

def check_udf_onefile(iso, filesize):
    assert(filesize == 550912)

    internal_check_pvd(iso.pvd, extent=16, size=269, ptbl_size=10, ptbl_location_le=262, ptbl_location_be=264)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=266, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=266, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'FOO.;1', dr_len=40, loc=267, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

    internal_check_udf_headers(iso, bea_extent=18, end_anchor_extent=268, part_length=11, unique_id=262, num_dirs=1, num_files=1)

    internal_check_udf_file_entry(iso.udf_root, location=259, tag_location=2, num_links=1, info_len=84, num_blocks_recorded=1, num_fi_descs=2, file_type='dir', num_alloc_descs=1)

    internal_check_udf_file_ident_desc(iso.udf_root.fi_descs[0], extent=260, tag_location=3, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    foo_file_ident = iso.udf_root.fi_descs[1]
    internal_check_udf_file_ident_desc(foo_file_ident, extent=260, tag_location=3, characteristics=0, blocknum=4, abs_blocknum=261, name=b'foo', isparent=False, isdir=False)

    foo_file_entry = foo_file_ident.file_entry
    internal_check_udf_file_entry(foo_file_entry, location=261, tag_location=4, num_links=1, info_len=4, num_blocks_recorded=1, num_fi_descs=0, file_type='file', num_alloc_descs=1)

    internal_check_file_contents(iso, path='/foo', contents=b'foo\n', which='udf_path')

def check_udf_onefileonedir(iso, filesize):
    assert(filesize == 557056)

    internal_check_pvd(iso.pvd, extent=16, size=272, ptbl_size=22, ptbl_location_le=264, ptbl_location_be=266)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=268, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=268, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    dir1_record = iso.pvd.root_dir_record.children[2]
    internal_check_empty_directory(dir1_record, name=b'DIR1', dr_len=38, extent=269, rr=False, hidden=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'FOO.;1', dr_len=40, loc=270, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

    internal_check_udf_headers(iso, bea_extent=18, end_anchor_extent=271, part_length=14, unique_id=264, num_dirs=2, num_files=1)

    internal_check_udf_file_entry(iso.udf_root, location=259, tag_location=2, num_links=2, info_len=128, num_blocks_recorded=1, num_fi_descs=3, file_type='dir', num_alloc_descs=1)

    internal_check_udf_file_ident_desc(iso.udf_root.fi_descs[0], extent=260, tag_location=3, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    foo_file_ident = iso.udf_root.fi_descs[2]
    internal_check_udf_file_ident_desc(foo_file_ident, extent=260, tag_location=3, characteristics=0, blocknum=6, abs_blocknum=263, name=b'foo', isparent=False, isdir=False)

    foo_file_entry = foo_file_ident.file_entry
    internal_check_udf_file_entry(foo_file_entry, location=263, tag_location=6, num_links=1, info_len=4, num_blocks_recorded=1, num_fi_descs=0, file_type='file', num_alloc_descs=1)

    internal_check_file_contents(iso, path='/foo', contents=b'foo\n', which='udf_path')

    dir1_file_ident = iso.udf_root.fi_descs[1]
    internal_check_udf_file_ident_desc(dir1_file_ident, extent=260, tag_location=3, characteristics=2, blocknum=4, abs_blocknum=261, name=b'dir1', isparent=False, isdir=True)

    dir1_file_entry = dir1_file_ident.file_entry
    internal_check_udf_file_entry(dir1_file_entry, location=261, tag_location=4, num_links=1, info_len=40, num_blocks_recorded=1, num_fi_descs=1, file_type='dir', num_alloc_descs=1)
    internal_check_udf_file_ident_desc(dir1_file_entry.fi_descs[0], extent=262, tag_location=5, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

def check_udf_dir_spillover(iso, filesize):
    assert(filesize == 677888)

    internal_check_pvd(iso.pvd, extent=16, size=331, ptbl_size=346, ptbl_location_le=304, ptbl_location_be=306)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=308, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=23, data_length=2048, extent_location=308, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    letter = ord('A')
    for child in iso.pvd.root_dir_record.children[2:]:
        namestr = chr(letter) * 8
        namestr = bytes(namestr.encode('utf-8'))

        # We skip checking the path table record extent locations because
        # genisoimage seems to have a bug assigning the extent locations, and
        # seems to assign them in reverse order.
        internal_check_ptr(child.ptr, name=namestr, len_di=len(namestr), loc=None, parent=1)

        internal_check_empty_directory(child, name=namestr, dr_len=42, extent=None, rr=False, hidden=False)

        letter += 1

    # Make sure we saw everything we expected.
    assert(chr(letter) == 'V')

    internal_check_udf_headers(iso, bea_extent=18, end_anchor_extent=330, part_length=73, unique_id=304, num_dirs=22, num_files=0)

    internal_check_udf_file_entry(iso.udf_root, location=259, tag_location=2, num_links=22, info_len=2224, num_blocks_recorded=2, num_fi_descs=22, file_type='dir', num_alloc_descs=1)
    internal_check_udf_file_ident_desc(iso.udf_root.fi_descs[0], extent=260, tag_location=3, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    letter = ord('a')
    for file_ident in iso.udf_root.fi_descs[1:]:
        namestr = chr(letter) * 64
        internal_check_udf_file_ident_desc(file_ident, extent=None, tag_location=None, characteristics=2, blocknum=None, abs_blocknum=None, name=bytes(namestr.encode('utf-8')), isparent=False, isdir=True)
        letter += 1

        file_entry = file_ident.file_entry
        internal_check_udf_file_entry(file_entry, location=None, tag_location=None, num_links=1, info_len=40, num_blocks_recorded=1, num_fi_descs=1, file_type='dir', num_alloc_descs=1)
        internal_check_udf_file_ident_desc(file_entry.fi_descs[0], extent=None, tag_location=None, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    # Make sure we saw everything we expected.
    assert(chr(letter) == 'v')

def check_udf_dir_oneshort(iso, filesize):
    assert(filesize == 671744)

    internal_check_pvd(iso.pvd, extent=16, size=328, ptbl_size=330, ptbl_location_le=302, ptbl_location_be=304)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=306, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=22, data_length=2048, extent_location=306, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    letter = ord('A')
    for child in iso.pvd.root_dir_record.children[2:]:
        namestr = chr(letter) * 8
        namestr = bytes(namestr.encode('utf-8'))

        # We skip checking the path table record extent locations because
        # genisoimage seems to have a bug assigning the extent locations, and
        # seems to assign them in reverse order.
        internal_check_ptr(child.ptr, name=namestr, len_di=len(namestr), loc=None, parent=1)

        internal_check_empty_directory(child, name=namestr, dr_len=42, extent=None, rr=False, hidden=False)

        letter += 1

    # Make sure we saw everything we expected.
    assert(chr(letter) == 'U')

    internal_check_udf_headers(iso, bea_extent=18, end_anchor_extent=327, part_length=70, unique_id=302, num_dirs=21, num_files=0)

    internal_check_udf_file_entry(iso.udf_root, location=259, tag_location=2, num_links=21, info_len=2120, num_blocks_recorded=2, num_fi_descs=21, file_type='dir', num_alloc_descs=1)
    internal_check_udf_file_ident_desc(iso.udf_root.fi_descs[0], extent=260, tag_location=3, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    letter = ord('a')
    for file_ident in iso.udf_root.fi_descs[1:]:
        namestr = chr(letter) * 64
        internal_check_udf_file_ident_desc(file_ident, extent=None, tag_location=None, characteristics=2, blocknum=None, abs_blocknum=None, name=bytes(namestr.encode('utf-8')), isparent=False, isdir=True)
        letter += 1

        file_entry = file_ident.file_entry
        internal_check_udf_file_entry(file_entry, location=None, tag_location=None, num_links=1, info_len=40, num_blocks_recorded=1, num_fi_descs=1, file_type='dir', num_alloc_descs=1)
        internal_check_udf_file_ident_desc(file_entry.fi_descs[0], extent=None, tag_location=None, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    # Make sure we saw everything we expected.
    assert(chr(letter) == 'u')

def check_udf_iso_hidden(iso, filesize):
    assert(filesize == 550912)

    internal_check_pvd(iso.pvd, extent=16, size=269, ptbl_size=10, ptbl_location_le=262, ptbl_location_be=264)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=266, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=2, data_length=2048, extent_location=266, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_udf_headers(iso, bea_extent=18, end_anchor_extent=268, part_length=11, unique_id=262, num_dirs=1, num_files=1)

    internal_check_udf_file_entry(iso.udf_root, location=259, tag_location=2, num_links=1, info_len=84, num_blocks_recorded=1, num_fi_descs=2, file_type='dir', num_alloc_descs=1)

    internal_check_udf_file_ident_desc(iso.udf_root.fi_descs[0], extent=260, tag_location=3, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    foo_file_ident = iso.udf_root.fi_descs[1]
    internal_check_udf_file_ident_desc(foo_file_ident, extent=260, tag_location=3, characteristics=0, blocknum=4, abs_blocknum=261, name=b'foo', isparent=False, isdir=False)

    foo_file_entry = foo_file_ident.file_entry
    internal_check_udf_file_entry(foo_file_entry, location=261, tag_location=4, num_links=1, info_len=4, num_blocks_recorded=1, num_fi_descs=0, file_type='file', num_alloc_descs=1)

    internal_check_file_contents(iso, path='/foo', contents=b'foo\n', which='udf_path')

def check_udf_hidden(iso, filesize):
    assert(filesize == 548864)

    internal_check_pvd(iso.pvd, extent=16, size=268, ptbl_size=10, ptbl_location_le=261, ptbl_location_be=263)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=265, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=265, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_udf_headers(iso, bea_extent=18, end_anchor_extent=267, part_length=10, unique_id=261, num_dirs=1, num_files=0)

    internal_check_udf_file_entry(iso.udf_root, location=259, tag_location=2, num_links=1, info_len=40, num_blocks_recorded=1, num_fi_descs=1, file_type='dir', num_alloc_descs=1)

    internal_check_udf_file_ident_desc(iso.udf_root.fi_descs[0], extent=260, tag_location=3, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'FOO.;1', dr_len=40, loc=266, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

def check_very_largefile(iso, filesize):
    assert(filesize == 5368758272)

    internal_check_pvd(iso.pvd, extent=16, size=2621464, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=23, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BIGFILE.;1', dr_len=44, loc=24, datalen=4294965248, hidden=False, multi_extent=True)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'BIGFILE.;1', dr_len=44, loc=2097175, datalen=1073743872, hidden=False, multi_extent=False)

def check_udf_very_large(iso, filesize):
    assert(filesize == 1074290688)

    internal_check_pvd(iso.pvd, extent=16, size=524556, ptbl_size=10, ptbl_location_le=262, ptbl_location_be=264)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=266, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=266, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'FOO.;1', dr_len=40, loc=267, datalen=1073739777, hidden=False, multi_extent=False)

    internal_check_udf_headers(iso, bea_extent=18, end_anchor_extent=524555, part_length=524298, unique_id=262, num_dirs=1, num_files=1)

    internal_check_udf_file_entry(iso.udf_root, location=259, tag_location=2, num_links=1, info_len=84, num_blocks_recorded=1, num_fi_descs=2, file_type='dir', num_alloc_descs=1)

    internal_check_udf_file_ident_desc(iso.udf_root.fi_descs[0], extent=260, tag_location=3, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    foo_file_ident = iso.udf_root.fi_descs[1]
    internal_check_udf_file_ident_desc(foo_file_ident, extent=260, tag_location=3, characteristics=0, blocknum=4, abs_blocknum=261, name=b'foo', isparent=False, isdir=False)

    foo_file_entry = foo_file_ident.file_entry
    internal_check_udf_file_entry(foo_file_entry, location=261, tag_location=4, num_links=1, info_len=1073739777, num_blocks_recorded=524288, num_fi_descs=0, file_type='file', num_alloc_descs=2)

def check_joliet_udf_nofiles(iso, filesize):
    assert(filesize == 557056)

    internal_check_pvd(iso.pvd, extent=16, size=272, ptbl_size=10, ptbl_location_le=261, ptbl_location_be=263)

    internal_check_jolietvd(iso.svds[0], space_size=272, path_tbl_size=10, path_tbl_loc_le=265, path_tbl_loc_be=267)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=269, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=270, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=2, data_length=2048, extent_location=269, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=2, data_length=2048, extent_location=270)

    internal_check_udf_headers(iso, bea_extent=19, end_anchor_extent=271, part_length=14, unique_id=261, num_dirs=1, num_files=0)

    internal_check_udf_file_entry(iso.udf_root, location=259, tag_location=2, num_links=1, info_len=40, num_blocks_recorded=1, num_fi_descs=1, file_type='dir', num_alloc_descs=1)

    internal_check_udf_file_ident_desc(iso.udf_root.fi_descs[0], extent=260, tag_location=3, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    assert(not iso.has_rock_ridge())
    assert(iso.has_joliet())
    assert(iso.has_udf())

def check_udf_dir_exactly2048(iso, filesize):
    assert(filesize == 589824)

    internal_check_pvd(iso.pvd, extent=16, size=288, ptbl_size=122, ptbl_location_le=275, ptbl_location_be=277)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=279, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=9, data_length=2048, extent_location=279, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    letter = ord('A')
    for child in iso.pvd.root_dir_record.children[2:]:
        namestr = chr(letter) * 8
        namestr = bytes(namestr.encode('utf-8'))

        internal_check_ptr(child.ptr, name=namestr, len_di=8, loc=None, parent=1)
        internal_check_empty_directory(child, name=namestr, dr_len=42, extent=None, rr=False, hidden=False)

        letter += 1

    assert(chr(letter) == 'H')

    internal_check_udf_headers(iso, bea_extent=18, end_anchor_extent=287, part_length=30, unique_id=275, num_dirs=8, num_files=0)

    internal_check_udf_file_entry(iso.udf_root, location=259, tag_location=2, num_links=8, info_len=2048, num_blocks_recorded=1, num_fi_descs=8, file_type='dir', num_alloc_descs=1)
    internal_check_udf_file_ident_desc(iso.udf_root.fi_descs[0], extent=260, tag_location=3, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    letter = ord('a')
    for file_ident in iso.udf_root.fi_descs[1:-1]:
        namestr = chr(letter) * 248
        internal_check_udf_file_ident_desc(file_ident, extent=None, tag_location=None, characteristics=2, blocknum=None, abs_blocknum=None, name=bytes(namestr.encode('utf-8')), isparent=False, isdir=True)

        file_entry = file_ident.file_entry
        internal_check_udf_file_entry(file_entry, location=None, tag_location=None, num_links=1, info_len=40, num_blocks_recorded=1, num_fi_descs=1, file_type='dir', num_alloc_descs=1)
        internal_check_udf_file_ident_desc(file_entry.fi_descs[0], extent=None, tag_location=None, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

        letter += 1

    assert(chr(letter) == 'g')

    namestr = chr(letter) * 240
    file_ident = iso.udf_root.fi_descs[-1]

    internal_check_udf_file_ident_desc(file_ident, extent=None, tag_location=None, characteristics=2, blocknum=None, abs_blocknum=None, name=bytes(namestr.encode('utf-8')), isparent=False, isdir=True)

    file_entry = file_ident.file_entry
    internal_check_udf_file_entry(file_entry, location=None, tag_location=None, num_links=1, info_len=40, num_blocks_recorded=1, num_fi_descs=1, file_type='dir', num_alloc_descs=1)
    internal_check_udf_file_ident_desc(file_entry.fi_descs[0], extent=None, tag_location=None, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    letter += 1

    # Make sure we saw everything we expected.
    assert(chr(letter) == 'h')

def check_udf_symlink(iso, filesize):
    assert(filesize == 555008)

    internal_check_pvd(iso.pvd, extent=16, size=271, ptbl_size=10, ptbl_location_le=263, ptbl_location_be=265)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=267, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=267, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BAR.;1', dr_len=40, loc=None, datalen=0, hidden=False, multi_extent=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'FOO.;1', dr_len=40, loc=268, datalen=4, hidden=False, multi_extent=False)

    internal_check_udf_headers(iso, bea_extent=18, end_anchor_extent=270, part_length=13, unique_id=263, num_dirs=1, num_files=2)

    internal_check_udf_file_entry(iso.udf_root, location=259, tag_location=2, num_links=1, info_len=128, num_blocks_recorded=1, num_fi_descs=3, file_type='dir', num_alloc_descs=1)

    internal_check_udf_file_ident_desc(iso.udf_root.fi_descs[0], extent=260, tag_location=3, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    bar_file_ident = iso.udf_root.fi_descs[2]
    internal_check_udf_file_ident_desc(bar_file_ident, extent=260, tag_location=3, characteristics=0, blocknum=5, abs_blocknum=262, name=b'bar', isparent=False, isdir=False)

    bar_file_entry = bar_file_ident.file_entry
    internal_check_udf_file_entry(bar_file_entry, location=262, tag_location=5, num_links=1, info_len=8, num_blocks_recorded=1, num_fi_descs=0, file_type='symlink', num_alloc_descs=1)

def check_udf_symlink_in_dir(iso, filesize):
    assert(filesize == 561152)

    internal_check_pvd(iso.pvd, extent=16, size=274, ptbl_size=22, ptbl_location_le=265, ptbl_location_be=267)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=269, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=269, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BAR.;1', dr_len=40, loc=None, datalen=0, hidden=False, multi_extent=False)

    dir1_record = iso.pvd.root_dir_record.children[3]
    internal_check_ptr(dir1_record.ptr, name=b'DIR1', len_di=4, loc=270, parent=1)
    internal_check_dir_record(dir1_record, num_children=3, name=b'DIR1', dr_len=38, extent_location=270, rr=False, rr_name=None, rr_links=0, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    internal_check_file(dir1_record.children[2], name=b'FOO.;1', dr_len=40, loc=271, datalen=4, hidden=False, multi_extent=False)

    internal_check_udf_headers(iso, bea_extent=18, end_anchor_extent=273, part_length=16, unique_id=265, num_dirs=2, num_files=2)

    internal_check_udf_file_entry(iso.udf_root, location=259, tag_location=2, num_links=2, info_len=128, num_blocks_recorded=1, num_fi_descs=3, file_type='dir', num_alloc_descs=1)

    internal_check_udf_file_ident_desc(iso.udf_root.fi_descs[0], extent=260, tag_location=3, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    bar_file_ident = iso.udf_root.fi_descs[2]
    internal_check_udf_file_ident_desc(bar_file_ident, extent=260, tag_location=3, characteristics=0, blocknum=6, abs_blocknum=263, name=b'bar', isparent=False, isdir=False)

    bar_file_entry = bar_file_ident.file_entry
    internal_check_udf_file_entry(bar_file_entry, location=263, tag_location=6, num_links=1, info_len=17, num_blocks_recorded=1, num_fi_descs=0, file_type='symlink', num_alloc_descs=1)

def check_udf_symlink_abs_path(iso, filesize):
    assert(filesize == 550912)

    internal_check_pvd(iso.pvd, extent=16, size=269, ptbl_size=10, ptbl_location_le=262, ptbl_location_be=264)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=266, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=266, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BAR.;1', dr_len=40, loc=None, datalen=0, hidden=False, multi_extent=False)

    internal_check_udf_headers(iso, bea_extent=18, end_anchor_extent=268, part_length=11, unique_id=262, num_dirs=1, num_files=1)

    internal_check_udf_file_entry(iso.udf_root, location=259, tag_location=2, num_links=1, info_len=84, num_blocks_recorded=1, num_fi_descs=2, file_type='dir', num_alloc_descs=1)

    internal_check_udf_file_ident_desc(iso.udf_root.fi_descs[0], extent=260, tag_location=3, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    bar_file_ident = iso.udf_root.fi_descs[1]
    internal_check_udf_file_ident_desc(bar_file_ident, extent=260, tag_location=3, characteristics=0, blocknum=4, abs_blocknum=261, name=b'bar', isparent=False, isdir=False)

    bar_file_entry = bar_file_ident.file_entry
    internal_check_udf_file_entry(bar_file_entry, location=261, tag_location=4, num_links=1, info_len=27, num_blocks_recorded=1, num_fi_descs=0, file_type='symlink', num_alloc_descs=1)

def check_udf_rr_symlink(iso, filesize):
    assert(filesize == 557056)

    internal_check_pvd(iso.pvd, extent=16, size=272, ptbl_size=10, ptbl_location_le=263, ptbl_location_be=265)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=267, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=267, rr=True, rr_nlinks=2, xa=False, rr_onetwelve=False)

    foo_dir_record = iso.pvd.root_dir_record.children[2]
    internal_check_file(foo_dir_record, name=b'FOO.;1', dr_len=116, loc=269, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

    internal_check_rr_file(foo_dir_record, name=b'foo')
    internal_check_file_contents(iso, path='/foo', contents=b'foo\n', which='rr_path')

    # Now check the rock ridge symlink.  It should have a directory record
    # length of 126, and the symlink components should be 'foo'.
    sym_dir_record = iso.pvd.root_dir_record.children[3]
    internal_check_rr_symlink(sym_dir_record, name=b'SYM.;1', dr_len=126, comps=[b'foo'])

    internal_check_udf_headers(iso, bea_extent=18, end_anchor_extent=271, part_length=14, unique_id=263, num_dirs=1, num_files=2)

    internal_check_udf_file_entry(iso.udf_root, location=259, tag_location=2, num_links=1, info_len=128, num_blocks_recorded=1, num_fi_descs=3, file_type='dir', num_alloc_descs=1)

    internal_check_udf_file_ident_desc(iso.udf_root.fi_descs[0], extent=260, tag_location=3, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    foo_file_ident = iso.udf_root.fi_descs[1]
    internal_check_udf_file_ident_desc(foo_file_ident, extent=260, tag_location=3, characteristics=0, blocknum=4, abs_blocknum=261, name=b'foo', isparent=False, isdir=False)

    foo_file_entry = foo_file_ident.file_entry
    internal_check_udf_file_entry(foo_file_entry, location=261, tag_location=4, num_links=1, info_len=4, num_blocks_recorded=1, num_fi_descs=0, file_type='file', num_alloc_descs=1)

    sym_file_ident = iso.udf_root.fi_descs[2]
    internal_check_udf_file_ident_desc(sym_file_ident, extent=260, tag_location=3, characteristics=0, blocknum=5, abs_blocknum=262, name=b'sym', isparent=False, isdir=False)

    sym_file_entry = sym_file_ident.file_entry
    internal_check_udf_file_entry(sym_file_entry, location=262, tag_location=5, num_links=1, info_len=8, num_blocks_recorded=1, num_fi_descs=0, file_type='symlink', num_alloc_descs=1)

    assert(iso.has_rock_ridge())
    assert(not iso.has_joliet())
    assert(iso.has_udf())

def check_udf_overflow_dir_extent(iso, filesize):
    assert(filesize == 831488)

    internal_check_pvd(iso.pvd, extent=16, size=406, ptbl_size=636, ptbl_location_le=354, ptbl_location_be=356)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=358, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=48, data_length=2048, extent_location=358, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    names = internal_generate_inorder_names(46)
    for index in range(2, 2+46):
        dir_record = iso.pvd.root_dir_record.children[index]
        # We skip checking the path table record extent locations because
        # genisoimage seems to have a bug assigning the extent locations, and
        # seems to assign them in reverse order.
        internal_check_ptr(dir_record.ptr, name=names[index], len_di=len(names[index]), loc=None, parent=1)

        internal_check_empty_directory(dir_record, name=names[index], dr_len=38, extent=None, rr=False, hidden=False)

    internal_check_udf_headers(iso, bea_extent=18, end_anchor_extent=405, part_length=148, unique_id=354, num_dirs=47, num_files=0)

    names = internal_generate_udf_inorder_names(46)
    for index in range(1, 1+46):
        file_ident = iso.udf_root.fi_descs[index]
        internal_check_udf_file_ident_desc(file_ident, extent=None, tag_location=None, characteristics=2, blocknum=None, abs_blocknum=None, name=names[index], isparent=False, isdir=True)

        file_entry = file_ident.file_entry
        internal_check_udf_file_entry(file_entry, location=None, tag_location=None, num_links=1, info_len=40, num_blocks_recorded=1, num_fi_descs=1, file_type='dir', num_alloc_descs=1)
        internal_check_udf_file_ident_desc(file_entry.fi_descs[0], extent=None, tag_location=None, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

def check_udf_hardlink(iso, filesize):
    assert(filesize == 550912)

    internal_check_pvd(iso.pvd, extent=16, size=269, ptbl_size=10, ptbl_location_le=262, ptbl_location_be=264)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=266, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=266, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    foo_dir_record = iso.pvd.root_dir_record.children[2]
    internal_check_file(foo_dir_record, name=b'FOO.;1', dr_len=40, loc=267, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

    internal_check_udf_headers(iso, bea_extent=18, end_anchor_extent=268, part_length=11, unique_id=262, num_dirs=1, num_files=2)

    internal_check_udf_file_entry(iso.udf_root, location=259, tag_location=2, num_links=1, info_len=128, num_blocks_recorded=1, num_fi_descs=3, file_type='dir', num_alloc_descs=1)

    internal_check_udf_file_ident_desc(iso.udf_root.fi_descs[0], extent=260, tag_location=3, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    bar_file_ident = iso.udf_root.fi_descs[2]
    internal_check_udf_file_ident_desc(bar_file_ident, extent=260, tag_location=3, characteristics=0, blocknum=4, abs_blocknum=261, name=b'bar', isparent=False, isdir=False)

    foo_file_ident = iso.udf_root.fi_descs[1]
    internal_check_udf_file_ident_desc(foo_file_ident, extent=260, tag_location=3, characteristics=0, blocknum=4, abs_blocknum=261, name=b'foo', isparent=False, isdir=False)

    bar_file_entry = bar_file_ident.file_entry
    internal_check_udf_file_entry(bar_file_entry, location=261, tag_location=4, num_links=1, info_len=4, num_blocks_recorded=1, num_fi_descs=0, file_type='file', num_alloc_descs=1)
    internal_check_file_contents(iso, path='/bar', contents=b'foo\n', which='udf_path')

    foo_file_entry = foo_file_ident.file_entry
    internal_check_udf_file_entry(foo_file_entry, location=261, tag_location=4, num_links=1, info_len=4, num_blocks_recorded=1, num_fi_descs=0, file_type='file', num_alloc_descs=1)
    internal_check_file_contents(iso, path='/foo', contents=b'foo\n', which='udf_path')

def check_multi_hard_link(iso, filesize):
    assert(filesize == 51200)

    internal_check_pvd(iso.pvd, extent=16, size=25, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=5, data_length=2048, extent_location=23, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BAR.;1', dr_len=40, loc=24, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BAR.;1', contents=b'foo\n', which='iso_path')

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'BAZ.;1', dr_len=40, loc=24, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BAZ.;1', contents=b'foo\n', which='iso_path')

    internal_check_file(iso.pvd.root_dir_record.children[4], name=b'FOO.;1', dr_len=40, loc=24, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

def check_joliet_with_version(iso, filesize):
    assert(filesize == 63488)

    internal_check_pvd(iso.pvd, extent=16, size=31, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_jolietvd(iso.svds[0], space_size=31, path_tbl_size=10, path_tbl_loc_le=24, path_tbl_loc_be=26)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=28, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=29, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=28, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=3, data_length=2048, extent_location=29)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'FOO.;1', dr_len=40, loc=30, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

    internal_check_file(iso.joliet_vd.root_dir_record.children[2], name='foo.;1'.encode('utf-16_be'), dr_len=46, loc=30, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/foo.;1', contents=b'foo\n', which='joliet_path')

def check_udf_joliet_onefile(iso, filesize):
    assert(filesize == 561152)

    internal_check_pvd(iso.pvd, extent=16, size=274, ptbl_size=10, ptbl_location_le=262, ptbl_location_be=264)

    internal_check_jolietvd(iso.svds[0], space_size=274, path_tbl_size=10, path_tbl_loc_le=266, path_tbl_loc_be=268)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=270, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=271, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=270, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=3, data_length=2048, extent_location=271)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'FOO.;1', dr_len=40, loc=272, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

    internal_check_file(iso.joliet_vd.root_dir_record.children[2], name='foo'.encode('utf-16_be'), dr_len=40, loc=272, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/foo', contents=b'foo\n', which='joliet_path')

    internal_check_udf_headers(iso, bea_extent=19, end_anchor_extent=273, part_length=16, unique_id=262, num_dirs=1, num_files=1)

    internal_check_udf_file_entry(iso.udf_root, location=259, tag_location=2, num_links=1, info_len=84, num_blocks_recorded=1, num_fi_descs=2, file_type='dir', num_alloc_descs=1)

    internal_check_udf_file_ident_desc(iso.udf_root.fi_descs[0], extent=260, tag_location=3, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    foo_file_ident = iso.udf_root.fi_descs[1]
    internal_check_udf_file_ident_desc(foo_file_ident, extent=260, tag_location=3, characteristics=0, blocknum=4, abs_blocknum=261, name=b'foo', isparent=False, isdir=False)

    foo_file_entry = foo_file_ident.file_entry
    internal_check_udf_file_entry(foo_file_entry, location=261, tag_location=4, num_links=1, info_len=4, num_blocks_recorded=1, num_fi_descs=0, file_type='file', num_alloc_descs=1)

    internal_check_file_contents(iso, path='/foo', contents=b'foo\n', which='udf_path')

def check_joliet_and_eltorito_joliet_only(iso, filesize):
    assert(filesize == 67584)

    internal_check_pvd(iso.pvd, extent=16, size=33, ptbl_size=10, ptbl_location_le=21, ptbl_location_be=23)

    internal_check_jolietvd(iso.svds[0], space_size=33, path_tbl_size=10, path_tbl_loc_le=25, path_tbl_loc_be=27)

    internal_check_eltorito(iso, boot_catalog_extent=31, load_rba=32, media_type=0, system_type=0, bootable=True, platform_id=0)

    internal_check_terminator(iso.vdsts, extent=19)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=29, parent=1)

    internal_check_ptr(iso.joliet_vd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=30, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=29, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=4, data_length=2048, extent_location=30)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BOOT.;1', dr_len=40, loc=32, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BOOT.;1', contents=b'boot\n', which='iso_path')

    internal_check_file(iso.joliet_vd.root_dir_record.children[3], name='boot.cat'.encode('utf-16_be'), dr_len=50, loc=31, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.joliet_vd.root_dir_record.children[2], name='boot'.encode('utf-16_be'), dr_len=42, loc=32, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/boot', contents=b'boot\n', which='joliet_path')

def check_udf_and_eltorito_udf_only(iso, filesize):
    assert(filesize == 555008)

    internal_check_pvd(iso.pvd, extent=16, size=271, ptbl_size=10, ptbl_location_le=263, ptbl_location_be=265)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=267, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=267, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BOOT.;1', dr_len=40, loc=269, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BOOT.;1', contents=b'boot\n', which='iso_path')

    internal_check_udf_headers(iso, bea_extent=19, end_anchor_extent=270, part_length=13, unique_id=263, num_dirs=1, num_files=2)

    internal_check_udf_file_entry(iso.udf_root, location=259, tag_location=2, num_links=1, info_len=132, num_blocks_recorded=1, num_fi_descs=3, file_type='dir', num_alloc_descs=1)

    internal_check_udf_file_ident_desc(iso.udf_root.fi_descs[0], extent=260, tag_location=3, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    boot_file_ident = iso.udf_root.fi_descs[1]
    internal_check_udf_file_ident_desc(boot_file_ident, extent=260, tag_location=3, characteristics=0, blocknum=4, abs_blocknum=261, name=b'boot', isparent=False, isdir=False)

    boot_file_entry = boot_file_ident.file_entry
    internal_check_udf_file_entry(boot_file_entry, location=261, tag_location=4, num_links=1, info_len=5, num_blocks_recorded=1, num_fi_descs=0, file_type='file', num_alloc_descs=1)

    internal_check_file_contents(iso, path='/boot', contents=b'boot\n', which='udf_path')

    bootcat_file_ident = iso.udf_root.fi_descs[2]
    internal_check_udf_file_ident_desc(bootcat_file_ident, extent=260, tag_location=3, characteristics=0, blocknum=5, abs_blocknum=262, name=b'boot.cat', isparent=False, isdir=False)

    bootcat_file_entry = bootcat_file_ident.file_entry
    internal_check_udf_file_entry(bootcat_file_entry, location=262, tag_location=5, num_links=1, info_len=2048, num_blocks_recorded=1, num_fi_descs=0, file_type='file', num_alloc_descs=1)

def check_udf_onefile_multi_links(iso, filesize):
    assert(filesize == 550912)

    internal_check_pvd(iso.pvd, extent=16, size=269, ptbl_size=10, ptbl_location_le=262, ptbl_location_be=264)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=266, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=266, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'FOO.;1', dr_len=40, loc=267, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

    internal_check_udf_headers(iso, bea_extent=18, end_anchor_extent=268, part_length=11, unique_id=262, num_dirs=1, num_files=2)

    internal_check_udf_file_entry(iso.udf_root, location=259, tag_location=2, num_links=1, info_len=128, num_blocks_recorded=1, num_fi_descs=3, file_type='dir', num_alloc_descs=1)

    internal_check_udf_file_ident_desc(iso.udf_root.fi_descs[0], extent=260, tag_location=3, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    baz_file_ident = iso.udf_root.fi_descs[2]
    internal_check_udf_file_ident_desc(baz_file_ident, extent=260, tag_location=3, characteristics=0, blocknum=4, abs_blocknum=261, name=b'baz', isparent=False, isdir=False)

    baz_file_entry = baz_file_ident.file_entry
    internal_check_udf_file_entry(baz_file_entry, location=261, tag_location=4, num_links=1, info_len=4, num_blocks_recorded=1, num_fi_descs=0, file_type='file', num_alloc_descs=1)

    internal_check_file_contents(iso, path='/baz', contents=b'foo\n', which='udf_path')

    foo_file_ident = iso.udf_root.fi_descs[1]
    internal_check_udf_file_ident_desc(foo_file_ident, extent=260, tag_location=3, characteristics=0, blocknum=4, abs_blocknum=261, name=b'foo', isparent=False, isdir=False)

    foo_file_entry = foo_file_ident.file_entry
    internal_check_udf_file_entry(foo_file_entry, location=261, tag_location=4, num_links=1, info_len=4, num_blocks_recorded=1, num_fi_descs=0, file_type='file', num_alloc_descs=1)

    internal_check_file_contents(iso, path='/foo', contents=b'foo\n', which='udf_path')

def check_udf_dotdot_symlink(iso, filesize):
    assert(filesize == 561152)

    internal_check_pvd(iso.pvd, extent=16, size=274, ptbl_size=22, ptbl_location_le=265, ptbl_location_be=267)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=269, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=269, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    dir1_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(dir1_record.ptr, name=b'DIR1', len_di=4, loc=270, parent=1)
    internal_check_dir_record(dir1_record, num_children=3, name=b'DIR1', dr_len=38, extent_location=270, rr=False, rr_name=None, rr_links=0, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'FOO.;1', dr_len=40, loc=None, datalen=4, hidden=False, multi_extent=False)

    sym_record = dir1_record.children[2]
    internal_check_file(sym_record, name=b'SYM.;1', dr_len=40, loc=None, datalen=0, hidden=False, multi_extent=False)

    internal_check_udf_headers(iso, bea_extent=18, end_anchor_extent=273, part_length=16, unique_id=265, num_dirs=2, num_files=2)

    internal_check_udf_file_entry(iso.udf_root, location=259, tag_location=2, num_links=2, info_len=128, num_blocks_recorded=1, num_fi_descs=3, file_type='dir', num_alloc_descs=1)

    # parent
    internal_check_udf_file_ident_desc(iso.udf_root.fi_descs[0], extent=260, tag_location=3, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    dir1_file_ident = iso.udf_root.fi_descs[1]
    internal_check_udf_file_ident_desc(dir1_file_ident, extent=260, tag_location=3, characteristics=2, blocknum=4, abs_blocknum=261, name=b'dir1', isparent=False, isdir=True)

    dir1_file_entry = dir1_file_ident.file_entry
    internal_check_udf_file_entry(dir1_file_entry, location=261, tag_location=4, num_links=1, info_len=84, num_blocks_recorded=1, num_fi_descs=2, file_type='dir', num_alloc_descs=1)

    foo_file_ident = iso.udf_root.fi_descs[2]
    internal_check_udf_file_ident_desc(foo_file_ident, extent=260, tag_location=3, characteristics=0, blocknum=6, abs_blocknum=263, name=b'foo', isparent=False, isdir=False)

    foo_file_entry = foo_file_ident.file_entry
    internal_check_udf_file_entry(foo_file_entry, location=263, tag_location=6, num_links=1, info_len=4, num_blocks_recorded=1, num_fi_descs=0, file_type='file', num_alloc_descs=1)

    internal_check_file_contents(iso, path='/foo', contents=b'foo\n', which='udf_path')

    sym_file_ident = dir1_file_entry.fi_descs[1]
    internal_check_udf_file_ident_desc(sym_file_ident, extent=262, tag_location=5, characteristics=0, blocknum=7, abs_blocknum=264, name=b'sym', isparent=False, isdir=False)

    sym_file_entry = sym_file_ident.file_entry
    internal_check_udf_file_entry(sym_file_entry, location=264, tag_location=7, num_links=1, info_len=12, num_blocks_recorded=1, num_fi_descs=0, file_type='symlink', num_alloc_descs=1)

def check_udf_dot_symlink(iso, filesize):
    assert(filesize == 555008)

    internal_check_pvd(iso.pvd, extent=16, size=271, ptbl_size=10, ptbl_location_le=263, ptbl_location_be=265)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=267, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=267, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'FOO.;1', dr_len=40, loc=None, datalen=4, hidden=False, multi_extent=False)

    sym_record = iso.pvd.root_dir_record.children[3]
    internal_check_file(sym_record, name=b'SYM.;1', dr_len=40, loc=None, datalen=0, hidden=False, multi_extent=False)

    internal_check_udf_headers(iso, bea_extent=18, end_anchor_extent=270, part_length=13, unique_id=263, num_dirs=1, num_files=2)

    internal_check_udf_file_entry(iso.udf_root, location=259, tag_location=2, num_links=1, info_len=128, num_blocks_recorded=1, num_fi_descs=3, file_type='dir', num_alloc_descs=1)

    internal_check_udf_file_ident_desc(iso.udf_root.fi_descs[0], extent=260, tag_location=3, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    foo_file_ident = iso.udf_root.fi_descs[1]
    internal_check_udf_file_ident_desc(foo_file_ident, extent=260, tag_location=3, characteristics=0, blocknum=4, abs_blocknum=261, name=b'foo', isparent=False, isdir=False)

    foo_file_entry = foo_file_ident.file_entry
    internal_check_udf_file_entry(foo_file_entry, location=261, tag_location=4, num_links=1, info_len=4, num_blocks_recorded=1, num_fi_descs=0, file_type='file', num_alloc_descs=1)

    internal_check_file_contents(iso, path='/foo', contents=b'foo\n', which='udf_path')

    sym_file_ident = iso.udf_root.fi_descs[2]
    internal_check_udf_file_ident_desc(sym_file_ident, extent=260, tag_location=3, characteristics=0, blocknum=5, abs_blocknum=262, name=b'sym', isparent=False, isdir=False)

    sym_file_entry = sym_file_ident.file_entry
    internal_check_udf_file_entry(sym_file_entry, location=262, tag_location=5, num_links=1, info_len=12, num_blocks_recorded=1, num_fi_descs=0, file_type='symlink', num_alloc_descs=1)

def check_udf_zero_byte_file(iso, filesize):
    assert(filesize == 552960)

    internal_check_pvd(iso.pvd, extent=16, size=270, ptbl_size=10, ptbl_location_le=263, ptbl_location_be=265)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=267, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=267, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BAR.;1', dr_len=40, loc=268, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BAR.;1', contents=b'bar\n', which='iso_path')

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'FOO.;1', dr_len=40, loc=None, datalen=0, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'', which='iso_path')

    internal_check_udf_headers(iso, bea_extent=18, end_anchor_extent=269, part_length=12, unique_id=263, num_dirs=1, num_files=2)

    internal_check_udf_file_entry(iso.udf_root, location=259, tag_location=2, num_links=1, info_len=128, num_blocks_recorded=1, num_fi_descs=3, file_type='dir', num_alloc_descs=1)

    internal_check_udf_file_ident_desc(iso.udf_root.fi_descs[0], extent=260, tag_location=3, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    bar_file_ident = iso.udf_root.fi_descs[2]
    internal_check_udf_file_ident_desc(bar_file_ident, extent=260, tag_location=3, characteristics=0, blocknum=5, abs_blocknum=262, name=b'bar', isparent=False, isdir=False)

    bar_file_entry = bar_file_ident.file_entry
    internal_check_udf_file_entry(bar_file_entry, location=262, tag_location=5, num_links=1, info_len=4, num_blocks_recorded=1, num_fi_descs=0, file_type='file', num_alloc_descs=1)

    internal_check_file_contents(iso, path='/bar', contents=b'bar\n', which='udf_path')

    foo_file_ident = iso.udf_root.fi_descs[1]
    internal_check_udf_file_ident_desc(foo_file_ident, extent=260, tag_location=3, characteristics=0, blocknum=4, abs_blocknum=261, name=b'foo', isparent=False, isdir=False)

    foo_file_entry = foo_file_ident.file_entry
    internal_check_udf_file_entry(foo_file_entry, location=261, tag_location=4, num_links=1, info_len=0, num_blocks_recorded=0, num_fi_descs=0, file_type='file', num_alloc_descs=0)

    internal_check_file_contents(iso, path='/foo', contents=b'', which='udf_path')

def check_udf_onefile_onedirwithfile(iso, filesize):
    assert(filesize == 561152)

    internal_check_pvd(iso.pvd, extent=16, size=274, ptbl_size=22, ptbl_location_le=265, ptbl_location_be=267)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=269, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=269, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    dir1_record = iso.pvd.root_dir_record.children[2]
    internal_check_dir_record(dir1_record, num_children=3, name=b'DIR1', dr_len=38, extent_location=270, rr=False, rr_name=b'dir1', rr_links=2, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    # The directory record should have a valid 'dotdot' record.
    internal_check_dotdot_dir_record(dir1_record.children[1], rr=False, rr_nlinks=3, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'FOO.;1', dr_len=40, loc=271, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

    internal_check_file(dir1_record.children[2], name=b'BAR.;1', dr_len=40, loc=272, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/DIR1/BAR.;1', contents=b'bar\n', which='iso_path')

    internal_check_udf_headers(iso, bea_extent=18, end_anchor_extent=273, part_length=16, unique_id=265, num_dirs=2, num_files=2)

    internal_check_udf_file_entry(iso.udf_root, location=259, tag_location=2, num_links=2, info_len=128, num_blocks_recorded=1, num_fi_descs=3, file_type='dir', num_alloc_descs=1)

    internal_check_udf_file_ident_desc(iso.udf_root.fi_descs[0], extent=260, tag_location=3, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    dir1_file_ident = iso.udf_root.fi_descs[1]
    internal_check_udf_file_ident_desc(dir1_file_ident, extent=260, tag_location=3, characteristics=2, blocknum=4, abs_blocknum=261, name=b'dir1', isparent=False, isdir=True)

    dir1_file_entry = dir1_file_ident.file_entry
    internal_check_udf_file_entry(dir1_file_entry, location=261, tag_location=4, num_links=1, info_len=84, num_blocks_recorded=1, num_fi_descs=2, file_type='dir', num_alloc_descs=1)
    internal_check_udf_file_ident_desc(dir1_file_entry.fi_descs[0], extent=262, tag_location=5, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    foo_file_ident = iso.udf_root.fi_descs[2]
    internal_check_udf_file_ident_desc(foo_file_ident, extent=260, tag_location=3, characteristics=0, blocknum=6, abs_blocknum=263, name=b'foo', isparent=False, isdir=False)

    foo_file_entry = foo_file_ident.file_entry
    internal_check_udf_file_entry(foo_file_entry, location=263, tag_location=6, num_links=1, info_len=4, num_blocks_recorded=1, num_fi_descs=0, file_type='file', num_alloc_descs=1)

    internal_check_file_contents(iso, path='/foo', contents=b'foo\n', which='udf_path')

    bar_file_ident = dir1_file_entry.fi_descs[1]
    internal_check_udf_file_ident_desc(bar_file_ident, extent=262, tag_location=5, characteristics=0, blocknum=7, abs_blocknum=264, name=b'bar', isparent=False, isdir=False)

    bar_file_entry = bar_file_ident.file_entry
    internal_check_udf_file_entry(bar_file_entry, location=264, tag_location=7, num_links=1, info_len=4, num_blocks_recorded=1, num_fi_descs=0, file_type='file', num_alloc_descs=1)

    internal_check_file_contents(iso, path='/dir1/bar', contents=b'bar\n', which='udf_path')

def check_zero_byte_hard_link(iso, filesize):
    assert(filesize == 49152)

    internal_check_pvd(iso.pvd, extent=16, size=24, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=23, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BAR.;1', dr_len=40, loc=None, datalen=0, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BAR.;1', contents=b'', which='iso_path')

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'FOO.;1', dr_len=40, loc=None, datalen=0, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'', which='iso_path')

def check_udf_zero_byte_hard_link(iso, filesize):
    assert(filesize == 548864)

    internal_check_pvd(iso.pvd, extent=16, size=268, ptbl_size=10, ptbl_location_le=262, ptbl_location_be=264)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=266, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=266, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'FOO.;1', dr_len=40, loc=None, datalen=0, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'', which='iso_path')

    internal_check_udf_headers(iso, bea_extent=18, end_anchor_extent=267, part_length=10, unique_id=262, num_dirs=1, num_files=2)

    internal_check_udf_file_entry(iso.udf_root, location=259, tag_location=2, num_links=1, info_len=128, num_blocks_recorded=1, num_fi_descs=3, file_type='dir', num_alloc_descs=1)

    internal_check_udf_file_ident_desc(iso.udf_root.fi_descs[0], extent=260, tag_location=3, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    bar_file_ident = iso.udf_root.fi_descs[2]
    internal_check_udf_file_ident_desc(bar_file_ident, extent=260, tag_location=3, characteristics=0, blocknum=4, abs_blocknum=261, name=b'bar', isparent=False, isdir=False)

    bar_file_entry = bar_file_ident.file_entry
    internal_check_udf_file_entry(bar_file_entry, location=261, tag_location=4, num_links=1, info_len=0, num_blocks_recorded=0, num_fi_descs=0, file_type='file', num_alloc_descs=0)

    internal_check_file_contents(iso, path='/bar', contents=b'', which='udf_path')

def check_unicode_name(iso, filesize):
    assert(filesize == 51200)

    internal_check_pvd(iso.pvd, extent=16, size=25, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=23, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'F__O.;1', dr_len=40, loc=24, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/F__O.;1', contents=b'foo\n', which='iso_path')

def check_unicode_name_isolevel4(iso, filesize):
    assert(filesize == 53248)

    internal_check_pvd(iso.pvd, extent=16, size=26, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_enhanced_vd(iso.enhanced_vd, size=26, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=24, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=24, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'f\xc3\xb6o', dr_len=38, loc=25, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/föo', contents=b'foo\n', which='iso_path')

def check_unicode_name_joliet(iso, filesize):
    assert(filesize == 63488)

    internal_check_pvd(iso.pvd, extent=16, size=31, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_jolietvd(iso.svds[0], space_size=31, path_tbl_size=10, path_tbl_loc_le=24, path_tbl_loc_be=26)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=28, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=28, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=3, data_length=2048, extent_location=29)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'F__O.;1', dr_len=40, loc=30, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/F__O.;1', contents=b'foo\n', which='iso_path')

    internal_check_file(iso.joliet_vd.root_dir_record.children[2], name=b'\x00f\x00\xf6\x00o', dr_len=40, loc=30, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/föo', contents=b'foo\n', which='joliet_path')

def check_unicode_name_udf(iso, filesize):
    assert(filesize == 550912)

    internal_check_pvd(iso.pvd, extent=16, size=269, ptbl_size=10, ptbl_location_le=262, ptbl_location_be=264)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=266, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=266, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'F__O.;1', dr_len=40, loc=267, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/F__O.;1', contents=b'foo\n', which='iso_path')

    internal_check_udf_headers(iso, bea_extent=18, end_anchor_extent=268, part_length=11, unique_id=262, num_dirs=1, num_files=1)

    internal_check_udf_file_entry(iso.udf_root, location=259, tag_location=2, num_links=1, info_len=84, num_blocks_recorded=1, num_fi_descs=2, file_type='dir', num_alloc_descs=1)

    internal_check_udf_file_ident_desc(iso.udf_root.fi_descs[0], extent=260, tag_location=3, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    foo_file_ident = iso.udf_root.fi_descs[1]
    internal_check_udf_file_ident_desc(foo_file_ident, extent=260, tag_location=3, characteristics=0, blocknum=4, abs_blocknum=261, name=b'f\xf6o', isparent=False, isdir=False)

    foo_file_entry = foo_file_ident.file_entry
    internal_check_udf_file_entry(foo_file_entry, location=261, tag_location=4, num_links=1, info_len=4, num_blocks_recorded=1, num_fi_descs=0, file_type='file', num_alloc_descs=1)

    internal_check_file_contents(iso, path='/föo', contents=b'foo\n', which='udf_path')

def check_unicode_name_two_byte(iso, filesize):
    assert(filesize == 51200)

    internal_check_pvd(iso.pvd, extent=16, size=25, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=21)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=23, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=23, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'F___O.;1', dr_len=42, loc=24, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/F___O.;1', contents=b'foo\n', which='iso_path')

def check_unicode_name_two_byte_isolevel4(iso, filesize):
    assert(filesize == 53248)

    internal_check_pvd(iso.pvd, extent=16, size=26, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_enhanced_vd(iso.enhanced_vd, size=26, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=24, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=24, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'f\xe1\xb4\x94o', dr_len=38, loc=25, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/fᴔo', contents=b'foo\n', which='iso_path')

def check_unicode_name_two_byte_joliet(iso, filesize):
    assert(filesize == 63488)

    internal_check_pvd(iso.pvd, extent=16, size=31, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_jolietvd(iso.svds[0], space_size=31, path_tbl_size=10, path_tbl_loc_le=24, path_tbl_loc_be=26)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=28, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=28, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_joliet_root_dir_record(iso.joliet_vd.root_dir_record, num_children=3, data_length=2048, extent_location=29)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'F___O.;1', dr_len=42, loc=30, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/F___O.;1', contents=b'foo\n', which='iso_path')

    internal_check_file(iso.joliet_vd.root_dir_record.children[2], name=b'\x00f\x1d\x14\x00o', dr_len=40, loc=30, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/fᴔo', contents=b'foo\n', which='joliet_path')

def check_unicode_name_two_byte_udf(iso, filesize):
    assert(filesize == 550912)

    internal_check_pvd(iso.pvd, extent=16, size=269, ptbl_size=10, ptbl_location_le=262, ptbl_location_be=264)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=266, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=266, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'F___O.;1', dr_len=42, loc=267, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/F___O.;1', contents=b'foo\n', which='iso_path')

    internal_check_udf_headers(iso, bea_extent=18, end_anchor_extent=268, part_length=11, unique_id=262, num_dirs=1, num_files=1)

    internal_check_udf_file_entry(iso.udf_root, location=259, tag_location=2, num_links=1, info_len=88, num_blocks_recorded=1, num_fi_descs=2, file_type='dir', num_alloc_descs=1)

    internal_check_udf_file_ident_desc(iso.udf_root.fi_descs[0], extent=260, tag_location=3, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    foo_file_ident = iso.udf_root.fi_descs[1]
    internal_check_udf_file_ident_desc(foo_file_ident, extent=260, tag_location=3, characteristics=0, blocknum=4, abs_blocknum=261, name=b'\x00f\x1d\x14\x00o', isparent=False, isdir=False)

    foo_file_entry = foo_file_ident.file_entry
    internal_check_udf_file_entry(foo_file_entry, location=261, tag_location=4, num_links=1, info_len=4, num_blocks_recorded=1, num_fi_descs=0, file_type='file', num_alloc_descs=1)

def check_udf_unicode_symlink(iso, filesize):
    assert(filesize == 555008)

    internal_check_pvd(iso.pvd, extent=16, size=271, ptbl_size=10, ptbl_location_le=263, ptbl_location_be=265)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=267, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=267, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BAR.;1', dr_len=40, loc=None, datalen=0, hidden=False, multi_extent=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'F___O.;1', dr_len=42, loc=None, datalen=4, hidden=False, multi_extent=False)

    internal_check_udf_headers(iso, bea_extent=18, end_anchor_extent=270, part_length=13, unique_id=263, num_dirs=1, num_files=2)

    internal_check_udf_file_entry(iso.udf_root, location=259, tag_location=2, num_links=1, info_len=132, num_blocks_recorded=1, num_fi_descs=3, file_type='dir', num_alloc_descs=1)

    internal_check_udf_file_ident_desc(iso.udf_root.fi_descs[0], extent=260, tag_location=3, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    foo_file_ident = iso.udf_root.fi_descs[1]
    internal_check_udf_file_ident_desc(foo_file_ident, extent=260, tag_location=3, characteristics=0, blocknum=4, abs_blocknum=261, name=b'\x00f\x1d\x14\x00o', isparent=False, isdir=False)

    foo_file_entry = foo_file_ident.file_entry
    internal_check_udf_file_entry(foo_file_entry, location=261, tag_location=4, num_links=1, info_len=4, num_blocks_recorded=1, num_fi_descs=0, file_type='file', num_alloc_descs=1)

    internal_check_file_contents(iso, path='/fᴔo', contents=b'foo\n', which='udf_path')

    bar_file_ident = iso.udf_root.fi_descs[2]
    internal_check_udf_file_ident_desc(bar_file_ident, extent=260, tag_location=3, characteristics=0, blocknum=5, abs_blocknum=262, name=b'bar', isparent=False, isdir=False)

    bar_file_entry = bar_file_ident.file_entry
    internal_check_udf_file_entry(bar_file_entry, location=262, tag_location=5, num_links=1, info_len=11, num_blocks_recorded=1, num_fi_descs=0, file_type='symlink', num_alloc_descs=1)

def check_udf_zeroed_file_entry(iso, filesize):
    assert(filesize == 550912)

    internal_check_pvd(iso.pvd, extent=16, size=269, ptbl_size=10, ptbl_location_le=262, ptbl_location_be=264)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=266, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=266, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'FOO.;1', dr_len=40, loc=267, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')

    internal_check_udf_headers(iso, bea_extent=18, end_anchor_extent=268, part_length=11, unique_id=262, num_dirs=1, num_files=1)

    internal_check_udf_file_entry(iso.udf_root, location=259, tag_location=2, num_links=1, info_len=84, num_blocks_recorded=1, num_fi_descs=2, file_type='dir', num_alloc_descs=1)

    internal_check_udf_file_ident_desc(iso.udf_root.fi_descs[0], extent=260, tag_location=3, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    foo_file_ident = iso.udf_root.fi_descs[1]

    # Essentially equivalent to calling internal_check_udf_file_ident_desc,
    # but ensures that the file_entry is None
    assert(foo_file_ident.extent_location() == 260)
    internal_check_udf_tag(foo_file_ident.desc_tag, ident=257, location=3)
    assert(foo_file_ident.file_characteristics == 0)
    assert(foo_file_ident.len_fi == 4)
    internal_check_udf_longad(foo_file_ident.icb, size=2048, blocknum=4, abs_blocknum=261)
    assert(foo_file_ident.len_impl_use == 0)
    assert(foo_file_ident.impl_use == b'')
    assert(foo_file_ident.fi == b'foo')
    assert(foo_file_ident.file_entry is None)
    assert(foo_file_ident.isdir == False)
    assert(foo_file_ident.isparent == False)

def check_udf_unicode(iso, filesize):
    assert(filesize == 571392)

    # Check ISO headers
    internal_check_pvd(iso.pvd, extent=16, size=279, ptbl_size=48, ptbl_location_le=270, ptbl_location_be=272)

    internal_check_terminator(iso.vdsts, extent=17)

    # Check PTR
    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=274, parent=1)

    # Check ISO files/directories
    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=274, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'TEST.TXT;1', dr_len=44, loc=0, datalen=0, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/TEST.TXT;1', contents=b'', which='iso_path')

    top_dirrecord = iso.pvd.root_dir_record.children[3]
    internal_check_dir_record(top_dirrecord, num_children=4, name=b'__', dr_len=36, extent_location=275, rr=False, rr_name=b'', rr_links=0, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    internal_check_dotdot_dir_record(top_dirrecord.children[1], rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    port_dirrecord = top_dirrecord.children[2]
    internal_check_dir_record(port_dirrecord, num_children=3, name=b'PORT', dr_len=38, extent_location=276, rr=False, rr_name=b'', rr_links=0, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    internal_check_dotdot_dir_record(port_dirrecord.children[1], rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    cyrillic_dirrecord = top_dirrecord.children[3]
    internal_check_dir_record(cyrillic_dirrecord, num_children=3, name=b'________', dr_len=42, extent_location=277, rr=False, rr_name=b'', rr_links=0, xa=False, hidden=False, is_cl_record=False, datalen=2048, relocated=False)
    internal_check_dotdot_dir_record(cyrillic_dirrecord.children[1], rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(port_dirrecord.children[2], name=b'________.TXT;1', dr_len=48, loc=0, datalen=0, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/__/PORT/________.TXT;1', contents=b'', which='iso_path')

    internal_check_file(cyrillic_dirrecord.children[2], name=b'________.TXT;1', dr_len=48, loc=0, datalen=0, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/__/________/________.TXT;1', contents=b'', which='iso_path')

    # Check UDF headers
    internal_check_udf_headers(iso, bea_extent=18, end_anchor_extent=278, part_length=21, unique_id=270, num_dirs=4, num_files=3)

    internal_check_udf_file_entry(iso.udf_root, location=259, tag_location=2, num_links=2, info_len=132, num_blocks_recorded=1, num_fi_descs=3, file_type='dir', num_alloc_descs=1)

    internal_check_udf_file_ident_desc(iso.udf_root.fi_descs[0], extent=260, tag_location=3, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    test_file_ident = iso.udf_root.fi_descs[1]
    internal_check_udf_file_ident_desc(test_file_ident, extent=260, tag_location=3, characteristics=0, blocknum=10, abs_blocknum=267, name=b'test.txt', isparent=False, isdir=False)

    test_file_entry = test_file_ident.file_entry
    internal_check_udf_file_entry(test_file_entry, location=267, tag_location=10, num_links=1, info_len=0, num_blocks_recorded=0, num_fi_descs=0, file_type='file', num_alloc_descs=0)

    p3_file_ident = iso.udf_root.fi_descs[2]
    internal_check_udf_file_ident_desc(p3_file_ident, extent=260, tag_location=3, characteristics=2, blocknum=4, abs_blocknum=261, name=b'\x04 \x04-', isparent=False, isdir=True)
    p3_file_entry = p3_file_ident.file_entry
    internal_check_udf_file_entry(p3_file_entry, location=261, tag_location=4, num_links=3, info_len=148, num_blocks_recorded=1, num_fi_descs=3, file_type='dir', num_alloc_descs=1)
    internal_check_udf_file_ident_desc(p3_file_entry.fi_descs[0], extent=262, tag_location=5, characteristics=10, blocknum=2, abs_blocknum=0, name=b'', isparent=True, isdir=True)

    port_file_ident = p3_file_entry.fi_descs[1]
    internal_check_udf_file_ident_desc(port_file_ident, extent=262, tag_location=5, characteristics=2, blocknum=6, abs_blocknum=263, name=b'Port', isparent=False, isdir=True)
    port_file_entry = port_file_ident.file_entry
    internal_check_udf_file_entry(port_file_entry, location=263, tag_location=6, num_links=1, info_len=120, num_blocks_recorded=1, num_fi_descs=2, file_type='dir', num_alloc_descs=1)
    internal_check_udf_file_ident_desc(port_file_entry.fi_descs[0], extent=264, tag_location=7, characteristics=10, blocknum=None, abs_blocknum=None, name=b'', isparent=True, isdir=True)

    py_file_ident = p3_file_entry.fi_descs[2]
    internal_check_udf_file_ident_desc(py_file_ident, extent=262, tag_location=5, characteristics=2, blocknum=8, abs_blocknum=265, name=b'\x04 \x04C\x04:\x04>\x042\x04>\x044\x04A\x04B\x042\x040', isparent=False, isdir=True)
    py_file_entry = py_file_ident.file_entry
    internal_check_udf_file_entry(py_file_entry, location=265, tag_location=8, num_links=1, info_len=116, num_blocks_recorded=1, num_fi_descs=2, file_type='dir', num_alloc_descs=1)
    internal_check_udf_file_ident_desc(py_file_entry.fi_descs[0], extent=266, tag_location=9, characteristics=10, blocknum=None, abs_blocknum=None, name=b'', isparent=True, isdir=True)

    bn_file_ident = port_file_entry.fi_descs[1]
    internal_check_udf_file_ident_desc(bn_file_ident, extent=264, tag_location=7, characteristics=0, blocknum=11, abs_blocknum=268, name=b'\x042\x048\x04@\x04B\x04C\x040\x04;\x04L\x04=\x04K\x049\x00 \x04?\x04>\x04@\x04B\x00.\x00t\x00x\x00t', isparent=False, isdir=False)
    bn_file_entry = bn_file_ident.file_entry
    internal_check_udf_file_entry(bn_file_entry, location=268, tag_location=11, num_links=1, info_len=0, num_blocks_recorded=0, num_fi_descs=0, file_type='file', num_alloc_descs=0)

    pyk_file_ident = py_file_entry.fi_descs[1]
    internal_check_udf_file_ident_desc(pyk_file_ident, extent=266, tag_location=9, characteristics=0, blocknum=12, abs_blocknum=269, name=b'\x04 \x04C\x04:\x04>\x042\x04>\x044\x04A\x04B\x042\x04>\x00 \x04?\x04>\x00.\x00t\x00x\x00t', isparent=False, isdir=False)
    pyk_file_entry = pyk_file_ident.file_entry
    internal_check_udf_file_entry(pyk_file_entry, location=269, tag_location=12, num_links=1, info_len=0, num_blocks_recorded=0, num_fi_descs=0, file_type='file', num_alloc_descs=0)

def check_eltorito_get_bootcat(iso, filesize):
    assert(filesize == 55296)

    internal_check_pvd(iso.pvd, extent=16, size=27, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_eltorito(iso, boot_catalog_extent=25, load_rba=26, media_type=0, system_type=0, bootable=True, platform_id=0)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=24, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=24, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'BOOT.CAT;1', dr_len=44, loc=25, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BOOT.;1', dr_len=40, loc=26, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BOOT.;1', contents=b'boot\n', which='iso_path')

    internal_check_file_contents(iso, path='/BOOT.CAT;1', contents=b'\x01'+b'\x00'*27+b'\xaa\x55\x55\xaa\x88'+b'\x00'*5+b'\x04\x00\x1a'+b'\x00'*2007, which='iso_path')

def check_eltorito_uefi(iso, filesize):
    assert(filesize == 55296)

    internal_check_pvd(iso.pvd, extent=16, size=27, ptbl_size=10, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_eltorito(iso, boot_catalog_extent=25, load_rba=26, media_type=0, system_type=0, bootable=True, platform_id=0xef)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=24, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=4, data_length=2048, extent_location=24, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[3], name=b'BOOT.CAT;1', dr_len=44, loc=25, datalen=2048, hidden=False, multi_extent=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'BOOT.;1', dr_len=40, loc=26, datalen=5, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/BOOT.;1', contents=b'boot\n', which='iso_path')

def check_isolevel4_deep_directory(iso, filesize):
    assert(filesize == 67584)

    internal_check_pvd(iso.pvd, extent=16, size=33, ptbl_size=94, ptbl_location_le=20, ptbl_location_be=22)

    internal_check_terminator(iso.vdsts, extent=18)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=24, parent=1)

    assert(len(iso.pvd.root_dir_record.children) == 3)

    dir1_record = iso.pvd.root_dir_record.children[2]
    internal_check_ptr(dir1_record.ptr, name=b'dir1', len_di=4, loc=None, parent=1)

    dir2_record = dir1_record.children[2]
    internal_check_ptr(dir2_record.ptr, name=b'dir2', len_di=4, loc=None, parent=2)

    dir3_record = dir2_record.children[2]
    internal_check_ptr(dir3_record.ptr, name=b'dir3', len_di=4, loc=None, parent=3)

    dir4_record = dir3_record.children[2]
    internal_check_ptr(dir4_record.ptr, name=b'dir4', len_di=4, loc=None, parent=4)

    dir5_record = dir4_record.children[2]
    internal_check_ptr(dir5_record.ptr, name=b'dir5', len_di=4, loc=None, parent=5)

    dir6_record = dir5_record.children[2]
    internal_check_ptr(dir6_record.ptr, name=b'dir6', len_di=4, loc=None, parent=6)

    dir7_record = dir6_record.children[2]
    internal_check_ptr(dir7_record.ptr, name=b'dir7', len_di=4, loc=None, parent=7)

    internal_check_file_contents(iso, path='/dir1/dir2/dir3/dir4/dir5/dir6/dir7/foo', contents=b'foo\n', which='iso_path')

def check_onefile_one_extent_path_tables(iso, filesize):
    assert(filesize == 47104)

    internal_check_pvd(iso.pvd, extent=16, size=23, ptbl_size=10, ptbl_location_le=19, ptbl_location_be=20)

    internal_check_terminator(iso.vdsts, extent=17)

    internal_check_ptr(iso.pvd.root_dir_record.ptr, name=b'\x00', len_di=1, loc=21, parent=1)

    internal_check_root_dir_record(iso.pvd.root_dir_record, num_children=3, data_length=2048, extent_location=21, rr=False, rr_nlinks=0, xa=False, rr_onetwelve=False)

    internal_check_file(iso.pvd.root_dir_record.children[2], name=b'FOO.;1', dr_len=40, loc=22, datalen=4, hidden=False, multi_extent=False)
    internal_check_file_contents(iso, path='/FOO.;1', contents=b'foo\n', which='iso_path')
