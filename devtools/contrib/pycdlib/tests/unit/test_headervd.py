import os
import sys
import struct

import pytest

prefix = '.'
for i in range(0, 3):
    if os.path.isdir(os.path.join(prefix, 'pycdlib')):
        sys.path.insert(0, prefix)
        break
    else:
        prefix = '../' + prefix

import pycdlib.headervd

# PrimaryOrSupplementaryVolumeDescriptor
def test_pvd_parse_initialized_twice():
    pvd = pycdlib.headervd.PrimaryOrSupplementaryVD(1)
    pvd.parse(b'\x01CD001\x01' + b'\x00'*2041, 16)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        pvd.parse(b'\x01CD001\x01' + b'\x00'*2041, 16)
    assert(str(excinfo.value) == 'This Primary Volume Descriptor is already initialized')

def test_pvd_parse_invalid_vd_type():
    pvd = pycdlib.headervd.PrimaryOrSupplementaryVD(1)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        pvd.parse(b'\x00'*2048, 16)
    assert(str(excinfo.value) == 'Invalid volume descriptor')

def test_pvd_parse_invalid_identifier():
    pvd = pycdlib.headervd.PrimaryOrSupplementaryVD(1)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        pvd.parse(b'\x01CD002' + b'\x00'*2042, 16)
    assert(str(excinfo.value) == 'invalid CD isoIdentification')

def test_pvd_new_initialized_twice():
    pvd = pycdlib.headervd.PrimaryOrSupplementaryVD(1)
    pvd.new(0, b'', b'', 0, 0, 0, b'', b'', b'', b'', b'', b'', b'', 0.0, b'', False, 1, b'')

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        pvd.new(0, b'', b'', 0, 0, 0, b'', b'', b'', b'', b'', b'', b'', 0.0, b'', False, 1, b'')
    assert(str(excinfo.value) == 'This Primary Volume Descriptor is already initialized')

def test_pvd_new_pvd_invalid_flags():
    pvd = pycdlib.headervd.PrimaryOrSupplementaryVD(1)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        pvd.new(1, b'', b'', 0, 0, 0, b'', b'', b'', b'', b'', b'', b'', 0.0, b'', False, 1, b'')
    assert(str(excinfo.value) == 'Non-zero flags not allowed for a PVD')

def test_pvd_new_pvd_invalid_escape_sequence():
    pvd = pycdlib.headervd.PrimaryOrSupplementaryVD(1)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        pvd.new(0, b'', b'', 0, 0, 0, b'', b'', b'', b'', b'', b'', b'', 0.0, b'', False, 1, b'\x00')
    assert(str(excinfo.value) == 'Non-empty escape sequence not allowed for a PVD')

def test_pvd_new_pvd_invalid_version():
    pvd = pycdlib.headervd.PrimaryOrSupplementaryVD(1)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        pvd.new(0, b'', b'', 0, 0, 0, b'', b'', b'', b'', b'', b'', b'', 0.0, b'', False, 2, b'')
    assert(str(excinfo.value) == 'Only version 1 supported for a PVD')

def test_pvd_new_svd_invalid_version():
    svd = pycdlib.headervd.PrimaryOrSupplementaryVD(2)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        svd.new(0, b'', b'', 0, 0, 0, b'', b'', b'', b'', b'', b'', b'', 0.0, b'', False, 3, b'')
    assert(str(excinfo.value) == 'Only version 1 and version 2 supported for a Supplementary Volume Descriptor')

def test_pvd_copy_initialized_twice():
    svd = pycdlib.headervd.PrimaryOrSupplementaryVD(2)
    svd.new(0, b'', b'', 0, 0, 0, b'', b'', b'', b'', b'', b'', b'', 0.0, b'', False, 2, b'')

    svd2 = pycdlib.headervd.PrimaryOrSupplementaryVD(2)
    svd2.copy(svd)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        svd2.copy(svd)
    assert(str(excinfo.value) == 'This Volume Descriptor is already initialized')

def test_pvd_record_not_initialized():
    svd = pycdlib.headervd.PrimaryOrSupplementaryVD(2)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        svd.record()
    assert(str(excinfo.value) == 'This Volume Descriptor is not initialized')

def test_pvd_track_rr_ce_entry_not_initialized():
    pvd = pycdlib.headervd.PrimaryOrSupplementaryVD(2)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        pvd.track_rr_ce_entry(0, 0, 0)
    assert(str(excinfo.value) == 'This Primary Volume Descriptor is not initialized')

def test_pvd_add_rr_ce_entry_not_initialized():
    pvd = pycdlib.headervd.PrimaryOrSupplementaryVD(2)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        pvd.add_rr_ce_entry(0)
    assert(str(excinfo.value) == 'This Primary Volume Descriptor is not initialized')

def test_pvd_clear_rr_ce_entries_not_initialized():
    pvd = pycdlib.headervd.PrimaryOrSupplementaryVD(2)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        pvd.clear_rr_ce_entries()
    assert(str(excinfo.value) == 'This Primary Volume Descriptor is not initialized')

def test_pvd_path_table_size_not_initialized():
    pvd = pycdlib.headervd.PrimaryOrSupplementaryVD(2)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        pvd.path_table_size()
    assert(str(excinfo.value) == 'This Volume Descriptor is not initialized')

def test_pvd_add_to_space_size_not_initialized():
    pvd = pycdlib.headervd.PrimaryOrSupplementaryVD(2)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        pvd.add_to_space_size(0)
    assert(str(excinfo.value) == 'This Volume Descriptor is not initialized')

def test_pvd_remove_from_space_size_not_initialized():
    pvd = pycdlib.headervd.PrimaryOrSupplementaryVD(2)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        pvd.remove_from_space_size(0)
    assert(str(excinfo.value) == 'This Volume Descriptor is not initialized')

def test_pvd_root_directory_record_not_initialized():
    pvd = pycdlib.headervd.PrimaryOrSupplementaryVD(2)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        pvd.root_directory_record()
    assert(str(excinfo.value) == 'This Volume Descriptor is not initialized')

def test_pvd_logical_block_size_not_initialized():
    pvd = pycdlib.headervd.PrimaryOrSupplementaryVD(2)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        pvd.logical_block_size()
    assert(str(excinfo.value) == 'This Volume Descriptor is not initialized')

def test_pvd_add_to_ptr_size_not_initialized():
    pvd = pycdlib.headervd.PrimaryOrSupplementaryVD(2)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        pvd.add_to_ptr_size(0)
    assert(str(excinfo.value) == 'This Volume Descriptor is not initialized')

def test_pvd_remove_from_ptr_size_not_initialized():
    pvd = pycdlib.headervd.PrimaryOrSupplementaryVD(2)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        pvd.remove_from_ptr_size(0)
    assert(str(excinfo.value) == 'This Volume Descriptor is not initialized')

def test_pvd_sequence_number_not_initialized():
    pvd = pycdlib.headervd.PrimaryOrSupplementaryVD(2)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        pvd.sequence_number()
    assert(str(excinfo.value) == 'This Volume Descriptor is not initialized')

def test_pvd_copy_sizes_not_initialized():
    pvd = pycdlib.headervd.PrimaryOrSupplementaryVD(2)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        pvd.copy_sizes(None)
    assert(str(excinfo.value) == 'This Volume Descriptor is not initialized')

def test_pvd_extent_location_not_initialized():
    pvd = pycdlib.headervd.PrimaryOrSupplementaryVD(2)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        pvd.extent_location()
    assert(str(excinfo.value) == 'This Volume Descriptor is not initialized')

def test_pvd_set_extent_location_not_initialized():
    pvd = pycdlib.headervd.PrimaryOrSupplementaryVD(2)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        pvd.set_extent_location(0)
    assert(str(excinfo.value) == 'This Volume Descriptor is not initialized')

def test_pvd_is_pvd_not_initialized():
    pvd = pycdlib.headervd.PrimaryOrSupplementaryVD(2)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        pvd.is_pvd()
    assert(str(excinfo.value) == 'This Volume Descriptor is not initialized')

# FileOrTextIdentifer
def test_fot_parse_initialized_twice():
    fot = pycdlib.headervd.FileOrTextIdentifier()
    fot.parse(b'a'*128)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        fot.parse(b'a'*128)
    assert(str(excinfo.value) == 'This File or Text identifier is already initialized')

def test_fot_new_initialized_twice():
    fot = pycdlib.headervd.FileOrTextIdentifier()
    fot.new(b'a'*128)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        fot.new(b'a'*128)
    assert(str(excinfo.value) == 'This File or Text identifier is already initialized')

def test_fot_new_bad_length():
    fot = pycdlib.headervd.FileOrTextIdentifier()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        fot.new(b'a'*127)
    assert(str(excinfo.value) == 'Length of text must be 128')

def test_fot_record_not_initialized():
    fot = pycdlib.headervd.FileOrTextIdentifier()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        fot.record()
    assert(str(excinfo.value) == 'This File or Text identifier is not initialized')

# Volume Descriptor Set Terminator
def test_vdst_parse_initialized_twice():
    vdst = pycdlib.headervd.VolumeDescriptorSetTerminator()
    vdst.parse(b'\xffCD001\x01' + b'\x00'*2041, 0)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        vdst.parse(b'\xffCD001\x01' + b'\x00'*2041, 0)
    assert(str(excinfo.value) == 'Volume Descriptor Set Terminator already initialized')

def test_vdst_parse_bad_descriptor_type():
    vdst = pycdlib.headervd.VolumeDescriptorSetTerminator()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        vdst.parse(b'\xfeCD001\x01' + b'\x00'*2041, 0)
    assert(str(excinfo.value) == 'Invalid VDST descriptor type')

def test_vdst_parse_bad_identifier():
    vdst = pycdlib.headervd.VolumeDescriptorSetTerminator()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        vdst.parse(b'\xffCD002\x01' + b'\x00'*2041, 0)
    assert(str(excinfo.value) == 'Invalid VDST identifier')

def test_vdst_new_initialized_twice():
    vdst = pycdlib.headervd.VolumeDescriptorSetTerminator()
    vdst.new()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        vdst.new()
    assert(str(excinfo.value) == 'Volume Descriptor Set Terminator already initialized')

def test_vdst_record_not_initialized():
    vdst = pycdlib.headervd.VolumeDescriptorSetTerminator()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        vdst.record()
    assert(str(excinfo.value) == 'Volume Descriptor Set Terminator not initialized')

def test_vdst_extent_location_not_initialized():
    vdst = pycdlib.headervd.VolumeDescriptorSetTerminator()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        vdst.extent_location()
    assert(str(excinfo.value) == 'Volume Descriptor Set Terminator not initialized')

def test_vdst_set_extent_location_not_initialized():
    vdst = pycdlib.headervd.VolumeDescriptorSetTerminator()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        vdst.set_extent_location(0)
    assert(str(excinfo.value) == 'This Volume Descriptor is not initialized')

# Boot Record
def test_br_parse_initialized_twice():
    br = pycdlib.headervd.BootRecord()
    br.parse(b'\x00CD001\x01' + b'\x00'*32 + b'\x00'*32 + b'\x00'*1977, 0)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        br.parse(b'\x00CD001\x01' + b'\x00'*32 + b'\x00'*32 + b'\x00'*1977, 0)
    assert(str(excinfo.value) == 'Boot Record already initialized')

def test_br_parse_bad_descriptor_type():
    br = pycdlib.headervd.BootRecord()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        br.parse(b'\x01CD001\x01' + b'\x00'*32 + b'\x00'*32 + b'\x00'*1977, 0)
    assert(str(excinfo.value) == 'Invalid boot record descriptor type')

def test_br_parse_bad_identifier():
    br = pycdlib.headervd.BootRecord()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        br.parse(b'\x00CD002\x01' + b'\x00'*32 + b'\x00'*32 + b'\x00'*1977, 0)
    assert(str(excinfo.value) == 'Invalid boot record identifier')

def test_br_new_initialized_twice():
    br = pycdlib.headervd.BootRecord()
    br.new(b'foo')

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        br.new(b'foo')
    assert(str(excinfo.value) == 'Boot Record already initialized')

def test_br_record_not_initialized():
    br = pycdlib.headervd.BootRecord()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        br.record()
    assert(str(excinfo.value) == 'Boot Record not initialized')

def test_br_update_boot_system_use_not_initialized():
    br = pycdlib.headervd.BootRecord()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        br.update_boot_system_use(b'\x00'*1977)
    assert(str(excinfo.value) == 'Boot Record not initialized')

def test_br_update_boot_system_use_invalid_length():
    br = pycdlib.headervd.BootRecord()
    br.new(b'foo')

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        br.update_boot_system_use(b'\x00'*1976)
    assert(str(excinfo.value) == 'Boot system use field must be 1977 bytes')

def test_br_extent_location_not_initialized():
    br = pycdlib.headervd.BootRecord()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        br.extent_location()
    assert(str(excinfo.value) == 'Boot Record not initialized')

def test_br_set_extent_location_not_initialized():
    br = pycdlib.headervd.BootRecord()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        br.set_extent_location(0)
    assert(str(excinfo.value) == 'This Volume Descriptor is not initialized')

# Version Volume Descriptor
def test_version_parse_initialized_twice():
    version = pycdlib.headervd.VersionVolumeDescriptor()
    version.parse(b'\x00'*2048, 0)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        version.parse(b'\x00'*2048, 0)
    assert(str(excinfo.value) == 'This Version Volume Descriptor is already initialized')

def test_version_parse_not_version():
    version = pycdlib.headervd.VersionVolumeDescriptor()
    assert(not(version.parse(b'\x00'*2047, 0)))

def test_version_new_initialized_twice():
    version = pycdlib.headervd.VersionVolumeDescriptor()
    version.new(2048)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        version.new(2048)
    assert(str(excinfo.value) == 'This Version Volume Descriptor is already initialized')

def test_version_record_not_initialized():
    version = pycdlib.headervd.VersionVolumeDescriptor()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        version.record()
    assert(str(excinfo.value) == 'This Version Volume Descriptor is not initialized')

def test_version_extent_location_not_initialized():
    version = pycdlib.headervd.VersionVolumeDescriptor()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        version.extent_location()
    assert(str(excinfo.value) == 'This Version Volume Descriptor is not initialized')

def test_version_set_extent_location_not_initialized():
    version = pycdlib.headervd.VersionVolumeDescriptor()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        version.set_extent_location(0)
    assert(str(excinfo.value) == 'This Volume Descriptor is not initialized')

def test_pvd_remove_from_ptr_size_never_happen():
    pvd = pycdlib.headervd.PrimaryOrSupplementaryVD(1)
    pvd.parse(b'\x01CD001\x01' + b'\x00'*2041, 16)

    pvd.path_tbl_size = 4097
    pvd.path_table_num_extents = 2
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        pvd.remove_from_ptr_size(0)
    assert(str(excinfo.value) == 'Extent number should never grow when removing PTR')

def test_file_or_text_ident_compare_other_object():
    file_or_text_ident = pycdlib.headervd.FileOrTextIdentifier()

    file_or_text_ident.new('a'*128)

    assert(file_or_text_ident != 'foo')
