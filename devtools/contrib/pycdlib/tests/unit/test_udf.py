import os
import sys
import struct
import time

import pytest

prefix = '.'
for i in range(0, 3):
    if os.path.isdir(os.path.join(prefix, 'pycdlib')):
        sys.path.insert(0, prefix)
        break
    else:
        prefix = '../' + prefix

import pycdlib.udf

# BEA
def test_bea_parse_initialized_twice():
    bea = pycdlib.udf.BEAVolumeStructure()
    bea.parse(b'\x00BEA01\x01' + b'\x00'*2041, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        bea.parse(b'\x00BEA01\x01' + b'\x00'*2041, 0)
    assert(str(excinfo.value) == 'BEA Volume Structure already initialized')

def test_bea_parse_bad_structure():
    bea = pycdlib.udf.BEAVolumeStructure()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        bea.parse(b'\x01BEA01\x01' + b'\x00'*2041, 0)
    assert(str(excinfo.value) == 'Invalid structure type')

def test_bea_parse_bad_ident():
    bea = pycdlib.udf.BEAVolumeStructure()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        bea.parse(b'\x00CEA01\x01' + b'\x00'*2041, 0)
    assert(str(excinfo.value) == 'Invalid standard identifier')

def test_bea_parse_bad_version():
    bea = pycdlib.udf.BEAVolumeStructure()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        bea.parse(b'\x00BEA01\x02' + b'\x00'*2041, 0)
    assert(str(excinfo.value) == 'Invalid structure version')

def test_bea_record_not_initialized():
    bea = pycdlib.udf.BEAVolumeStructure()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        bea.record()
    assert(str(excinfo.value) == 'BEA Volume Structure not initialized')

def test_bea_new_initialized_twice():
    bea = pycdlib.udf.BEAVolumeStructure()
    bea.new()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        bea.new()
    assert(str(excinfo.value) == 'BEA Volume Structure already initialized')

def test_bea_extent_location_not_initialized():
    bea = pycdlib.udf.BEAVolumeStructure()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        bea.extent_location()
    assert(str(excinfo.value) == 'UDF BEA Volume Structure not initialized')

def test_bea_set_extent_location_not_initialized():
    bea = pycdlib.udf.BEAVolumeStructure()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        bea.set_extent_location(0)
    assert(str(excinfo.value) == 'This Volume Descriptor is not initialized')

# NSR
def test_nsr_parse_initialized_twice():
    nsr = pycdlib.udf.NSRVolumeStructure()
    nsr.parse(b'\x00NSR02\x01' + b'\x00'*2041, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        nsr.parse(b'\x00NSR02\x01' + b'\x00'*2041, 0)
    assert(str(excinfo.value) == 'UDF NSR Volume Structure already initialized')

def test_nsr_parse_bad_structure():
    nsr = pycdlib.udf.NSRVolumeStructure()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        nsr.parse(b'\x01NSR02\x01' + b'\x00'*2041, 0)
    assert(str(excinfo.value) == 'Invalid structure type')

def test_nsr_parse_bad_ident():
    nsr = pycdlib.udf.NSRVolumeStructure()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        nsr.parse(b'\x00NSR04\x01' + b'\x00'*2041, 0)
    assert(str(excinfo.value) == 'Invalid standard identifier')

def test_nsr_parse_bad_version():
    nsr = pycdlib.udf.NSRVolumeStructure()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        nsr.parse(b'\x00NSR02\x02' + b'\x00'*2041, 0)
    assert(str(excinfo.value) == 'Invalid structure version')

def test_nsr_record_not_initialized():
    nsr = pycdlib.udf.NSRVolumeStructure()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        nsr.record()
    assert(str(excinfo.value) == 'UDF NSR Volume Structure not initialized')

def test_nsr_new_initialized_twice():
    nsr = pycdlib.udf.NSRVolumeStructure()
    nsr.new(2)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        nsr.new(2)
    assert(str(excinfo.value) == 'UDF NSR Volume Structure already initialized')

def test_nsr_new_version_three():
    nsr = pycdlib.udf.NSRVolumeStructure()
    nsr.new(3)
    assert(nsr.standard_ident == b'NSR03')

def test_nsr_new_bad_version():
    nsr = pycdlib.udf.NSRVolumeStructure()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        nsr.new(4)
    assert(str(excinfo.value) == 'Invalid NSR version requested')

def test_nsr_extent_location_not_initialized():
    nsr = pycdlib.udf.NSRVolumeStructure()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        nsr.extent_location()
    assert(str(excinfo.value) == 'UDF NSR Volume Structure not initialized')

def test_nsr_set_extent_location_not_initialized():
    nsr = pycdlib.udf.NSRVolumeStructure()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        nsr.set_extent_location(0)
    assert(str(excinfo.value) == 'This Volume Descriptor is not initialized')

# TEA
def test_tea_parse_initialized_twice():
    tea = pycdlib.udf.TEAVolumeStructure()
    tea.parse(b'\x00TEA01\x01' + b'\x00'*2041, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        tea.parse(b'\x00TEA01\x01' + b'\x00'*2041, 0)
    assert(str(excinfo.value) == 'TEA Volume Structure already initialized')

def test_tea_parse_bad_type():
    tea = pycdlib.udf.TEAVolumeStructure()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        tea.parse(b'\x01TEA01\x01' + b'\x00'*2041, 0)
    assert(str(excinfo.value) == 'Invalid structure type')

def test_tea_parse_bad_ident():
    tea = pycdlib.udf.TEAVolumeStructure()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        tea.parse(b'\x00TEA02\x01' + b'\x00'*2041, 0)
    assert(str(excinfo.value) == 'Invalid standard identifier')

def test_tea_parse_bad_version():
    tea = pycdlib.udf.TEAVolumeStructure()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        tea.parse(b'\x00TEA01\x02' + b'\x00'*2041, 0)
    assert(str(excinfo.value) == 'Invalid structure version')

def test_tea_record_not_initialized():
    tea = pycdlib.udf.TEAVolumeStructure()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        tea.record()
    assert(str(excinfo.value) == 'UDF TEA Volume Structure not initialized')

def test_tea_new_initialized_twice():
    tea = pycdlib.udf.TEAVolumeStructure()
    tea.new()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        tea.new()
    assert(str(excinfo.value) == 'UDF TEA Volume Structure already initialized')

def test_tea_extent_location_not_initialized():
    tea = pycdlib.udf.TEAVolumeStructure()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        tea.extent_location()
    assert(str(excinfo.value) == 'UDF TEA Volume Structure not initialized')

def test_tea_set_extent_location_not_initialized():
    tea = pycdlib.udf.TEAVolumeStructure()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        tea.set_extent_location(0)
    assert(str(excinfo.value) == 'This Volume Descriptor is not initialized')

# Boot Descriptor
def test_boot_descriptor_parse():
    boot = pycdlib.udf.UDFBootDescriptor()
    boot.parse(b'\x00BOOT2\x01\x00' + b'\x00'*32 + b'\x00'*32 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00' + b'\x00'*32 + b'\x00'*1906, 0)
    assert(boot.boot_extent_loc == 0)
    assert(boot.boot_extent_len == 0)
    assert(boot.load_address == 0)
    assert(boot.start_address == 0)
    assert(boot.flags == 0)

def test_boot_descriptor_parse_initialized_twice():
    boot = pycdlib.udf.UDFBootDescriptor()
    boot.parse(b'\x00BOOT2\x01\x00' + b'\x00'*32 + b'\x00'*32 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00' + b'\x00'*32 + b'\x00'*1906, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        boot.parse(b'\x00BOOT2\x01\x00' + b'\x00'*32 + b'\x00'*32 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00' + b'\x00'*32 + b'\x00'*1906, 0)
    assert(str(excinfo.value) == 'UDF Boot Descriptor already initialized')

def test_boot_descriptor_parse_bad_type():
    boot = pycdlib.udf.UDFBootDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        boot.parse(b'\x01BOOT2\x01\x00' + b'\x00'*32 + b'\x00'*32 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00' + b'\x00'*32 + b'\x00'*1906, 0)
    assert(str(excinfo.value) == 'Invalid structure type')

def test_boot_descriptor_parse_bad_ident():
    boot = pycdlib.udf.UDFBootDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        boot.parse(b'\x00BOOT1\x01\x00' + b'\x00'*32 + b'\x00'*32 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00' + b'\x00'*32 + b'\x00'*1906, 0)
    assert(str(excinfo.value) == 'Invalid standard identifier')

def test_boot_descriptor_parse_bad_version():
    boot = pycdlib.udf.UDFBootDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        boot.parse(b'\x00BOOT2\x02\x00' + b'\x00'*32 + b'\x00'*32 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00' + b'\x00'*32 + b'\x00'*1906, 0)
    assert(str(excinfo.value) == 'Invalid structure version')

def test_boot_descriptor_parse_bad_reserved():
    boot = pycdlib.udf.UDFBootDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        boot.parse(b'\x00BOOT2\x01\x01' + b'\x00'*32 + b'\x00'*32 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00' + b'\x00'*32 + b'\x00'*1906, 0)
    assert(str(excinfo.value) == 'Invalid reserved1')

def test_boot_descriptor_parse_bad_flags():
    boot = pycdlib.udf.UDFBootDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        boot.parse(b'\x00BOOT2\x01\x00' + b'\x00'*32 + b'\x00'*32 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x02\x00' + b'\x00'*32 + b'\x00'*1906, 0)
    assert(str(excinfo.value) == 'Invalid flags (must be 0 or 1)')

def test_boot_descriptor_parse_bad_reserved2():
    boot = pycdlib.udf.UDFBootDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        boot.parse(b'\x00BOOT2\x01\x00' + b'\x00'*32 + b'\x00'*32 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00' + b'\x01'*32 + b'\x00'*1906, 0)
    assert(str(excinfo.value) == 'Invalid reserved2')

def test_boot_descriptor_new():
    boot = pycdlib.udf.UDFBootDescriptor()
    boot.new()
    assert(boot.flags == 0)
    assert(boot.boot_extent_loc == 0)
    assert(boot.boot_extent_len == 0)
    assert(boot.load_address == 0)
    assert(boot.start_address == 0)
    assert(boot.flags == 0)
    assert(boot._initialized)

def test_boot_descriptor_new_initialized_twice():
    boot = pycdlib.udf.UDFBootDescriptor()
    boot.new()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        boot.new()
    assert(str(excinfo.value) == 'UDF Boot Descriptor already initialized')

def test_boot_descriptor_record_not_initialized():
    boot = pycdlib.udf.UDFBootDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        boot.record()
    assert(str(excinfo.value) == 'UDF Boot Descriptor not initialized')

def test_boot_descriptor_record():
    boot = pycdlib.udf.UDFBootDescriptor()
    boot.new()
    rec = boot.record()
    assert(rec[0:8] == b'\x00BOOT2\x01\x00')

def test_boot_descriptor_extent_location_not_initialized():
    boot = pycdlib.udf.UDFBootDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        boot.extent_location()
    assert(str(excinfo.value) == 'UDF Boot Descriptor not initialized')

def test_boot_descriptor_extent_location():
    boot = pycdlib.udf.UDFBootDescriptor()
    boot = pycdlib.udf.UDFBootDescriptor()
    boot.parse(b'\x00BOOT2\x01\x00' + b'\x00'*32 + b'\x00'*32 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00' + b'\x00'*32 + b'\x00'*1906, 0)
    assert(boot.extent_location() == 0)

def test_boot_descriptor_set_extent_location_not_initialized():
    boot = pycdlib.udf.UDFBootDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        boot.set_extent_location(0)
    assert(str(excinfo.value) == 'This UDF Boot Descriptor is not initialized')

def test_boot_descriptor_set_extent_location():
    boot = pycdlib.udf.UDFBootDescriptor()
    boot.new()
    boot.set_extent_location(1)
    assert(boot.extent_location() == 1)

# UDFTag
def test_tag_parse_initialized_twice():
    tag = pycdlib.udf.UDFTag()
    tag.parse(b'\x00\x00\x02\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00', 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        tag.parse(b'\x00\x00\x02\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00', 0)
    assert(str(excinfo.value) == 'UDF Tag already initialized')

def test_tag_parse_bad_csum():
    tag = pycdlib.udf.UDFTag()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        tag.parse(b'\x00\x00\x02\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00', 0)
    assert(str(excinfo.value) == 'Tag checksum does not match!')

def test_tag_parse_not_enough_crc_data():
    tag = pycdlib.udf.UDFTag()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        tag.parse(b'\x00\x00\x02\x00\x03\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00', 0)
    assert(str(excinfo.value) == 'Not enough bytes to compute CRC')

def test_tag_parse_bad_crc():
    tag = pycdlib.udf.UDFTag()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        tag.parse(b'\x00\x00\x02\x00\x03\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\xee', 0)
    assert(str(excinfo.value) == 'Tag CRC does not match!')

def test_tag_parse():
    tag = pycdlib.udf.UDFTag()
    tag.parse(b'\x00\x00\x02\x00\x03\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00', 0)
    assert(tag.tag_ident == 0)
    assert(tag.desc_version == 2)
    assert(tag.tag_serial_number == 0)
    assert(tag.desc_crc_length == 1)
    assert(tag.tag_location == 0)
    assert(tag._initialized)

def test_tag_record_not_initialized():
    tag = pycdlib.udf.UDFTag()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        tag.record(b'\x00')
    assert(str(excinfo.value) == 'UDF Descriptor Tag not initialized')

def test_tag_new_initialized_twice():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        tag.new(0, 0)
    assert(str(excinfo.value) == 'UDF Tag already initialized')

def test_tag_eq():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    tag2 = pycdlib.udf.UDFTag()
    tag2.new(0, 0)

    assert(tag == tag2)

def test_tag_eq_not_same_object_type():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    not_a_tag = object()

    assert(tag != not_a_tag)

# Anchor
def test_anchor_parse_initialized_twice():
    anchor = pycdlib.udf.UDFAnchorVolumeStructure()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    anchor.parse(b'\x00'*16 + b'\x00'*8 + b'\x00'*8 + b'\x00'*480, 0, tag)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        anchor.parse(b'\x00'*16 + b'\x00'*8 + b'\x00'*8 + b'\x00'*480, 0, tag)
    assert(str(excinfo.value) == 'Anchor Volume Structure already initialized')

def test_anchor_record_not_initialized():
    anchor = pycdlib.udf.UDFAnchorVolumeStructure()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        anchor.record()
    assert(str(excinfo.value) == 'UDF Anchor Volume Descriptor not initialized')

def test_anchor_extent_location_not_initialized():
    anchor = pycdlib.udf.UDFAnchorVolumeStructure()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        anchor.extent_location()
    assert(str(excinfo.value) == 'UDF Anchor Volume Structure not initialized')

def test_anchor_new_initialized_twice():
    anchor = pycdlib.udf.UDFAnchorVolumeStructure()
    anchor.new()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        anchor.new()
    assert(str(excinfo.value) == 'UDF Anchor Volume Structure already initialized')

def test_anchor_set_extent_location_not_initialized():
    anchor = pycdlib.udf.UDFAnchorVolumeStructure()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        anchor.set_extent_location(0, 0, 0)
    assert(str(excinfo.value) == 'UDF Anchor Volume Structure not initialized')

def test_anchor_eq_not_same_object_type():
    anchor = pycdlib.udf.UDFAnchorVolumeStructure()
    anchor.new()

    not_an_anchor = object()

    assert(anchor != not_an_anchor)

# Volume Descriptor Pointer
def test_vdp_parse_initialized_twice():
    vdp = pycdlib.udf.UDFVolumeDescriptorPointer()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    vdp.parse(b'\x00'*16 + b'\x00\x00\x00x\00' + b'\x00'*8 + b'\x00'*484, 0, tag)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        vdp.parse(b'\x00'*16 + b'\x00\x00\x00x\00' + b'\x00'*8 + b'\x00'*484, 0, b'\x00'*16)
    assert(str(excinfo.value) == 'UDF Volume Descriptor Pointer already initialized')

def test_vdp_parse():
    vdp = pycdlib.udf.UDFVolumeDescriptorPointer()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    vdp.parse(b'\x00'*16 + b'\x00\x00\x00x\00' + b'\x00'*8 + b'\x00'*484, 0, tag)
    assert(vdp.orig_extent_loc == 0)
    assert(vdp.initialized)

def test_vdp_record_not_initialized():
    vdp = pycdlib.udf.UDFVolumeDescriptorPointer()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        vdp.record()
    assert(str(excinfo.value) == 'UDF Volume Descriptor Pointer not initialized')

def test_vdp_record():
    vdp = pycdlib.udf.UDFVolumeDescriptorPointer()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    vdp.parse(b'\x00'*16 + b'\x00\x00\x00x\00' + b'\x00'*8 + b'\x00'*484, 0, tag)
    assert(vdp.record() == (b'\x00\x00\x02\x00\x22\x00\x00\x00\x9c\x93\xf0\x01\x00\x00\x00\x00\x00\x00\x00\x78' + b'\x00'*492))

def test_vdp_extent_location_not_initialized():
    vdp = pycdlib.udf.UDFVolumeDescriptorPointer()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        vdp.extent_location()
    assert(str(excinfo.value) == 'UDF Volume Descriptor Pointer not initialized')

def test_vdp_extent_location_parse():
    vdp = pycdlib.udf.UDFVolumeDescriptorPointer()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    vdp.parse(b'\x00'*16 + b'\x00\x00\x00x\00' + b'\x00'*8 + b'\x00'*484, 0, tag)
    assert(vdp.extent_location() == 0)

def test_vdp_extent_location_new():
    vdp = pycdlib.udf.UDFVolumeDescriptorPointer()
    vdp.new()
    assert(vdp.extent_location() == 0)

def test_vdp_new_initialized_twice():
    vdp = pycdlib.udf.UDFVolumeDescriptorPointer()
    vdp.new()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        vdp.new()
    assert(str(excinfo.value) == 'UDF Volume Descriptor Pointer already initialized')

def test_vdp_new():
    vdp = pycdlib.udf.UDFVolumeDescriptorPointer()
    vdp.new()
    assert(vdp.vol_seqnum == 0)
    assert(vdp.initialized)

def test_vdp_set_extent_location_not_initialized():
    vdp = pycdlib.udf.UDFVolumeDescriptorPointer()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        vdp.set_extent_location(1)
    assert(str(excinfo.value) == 'UDF Volume Descriptor Pointer not initialized')

def test_vdp_set_extent_location():
    vdp = pycdlib.udf.UDFVolumeDescriptorPointer()
    vdp.new()
    vdp.set_extent_location(1)
    assert(vdp.new_extent_loc == 1)

# Timestamp
def test_timestamp_parse_initialized_twice():
    ts = pycdlib.udf.UDFTimestamp()
    ts.parse(b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00')
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ts.parse(b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00')
    assert(str(excinfo.value) == 'UDF Timestamp already initialized')

def test_timestamp_parse_bad_tz():
    ts = pycdlib.udf.UDFTimestamp()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        ts.parse(b'\x00\x06\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00')
    assert(str(excinfo.value) == 'Invalid UDF timezone')

def test_timestamp_parse_bad_year():
    ts = pycdlib.udf.UDFTimestamp()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        ts.parse(b'\x00\x00\x00\x00\x01\x01\x00\x00\x00\x00\x00\x00')
    assert(str(excinfo.value) == 'Invalid UDF year')

def test_timestamp_parse_bad_month():
    ts = pycdlib.udf.UDFTimestamp()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        ts.parse(b'\x00\x00\x01\x00\x00\x01\x00\x00\x00\x00\x00\x00')
    assert(str(excinfo.value) == 'Invalid UDF month')

def test_timestamp_parse_bad_day():
    ts = pycdlib.udf.UDFTimestamp()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        ts.parse(b'\x00\x00\x01\x00\x01\x00\x00\x00\x00\x00\x00\x00')
    assert(str(excinfo.value) == 'Invalid UDF day')

def test_timestamp_parse_bad_hour():
    ts = pycdlib.udf.UDFTimestamp()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        ts.parse(b'\x00\x00\x01\x00\x01\x01\x20\x00\x00\x00\x00\x00')
    assert(str(excinfo.value) == 'Invalid UDF hour')

def test_timestamp_parse_bad_minute():
    ts = pycdlib.udf.UDFTimestamp()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        ts.parse(b'\x00\x00\x01\x00\x01\x01\x00\x40\x00\x00\x00\x00')
    assert(str(excinfo.value) == 'Invalid UDF minute')

def test_timestamp_parse_bad_second():
    ts = pycdlib.udf.UDFTimestamp()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        ts.parse(b'\x00\x00\x01\x00\x01\x01\x00\x00\x40\x00\x00\x00')
    assert(str(excinfo.value) == 'Invalid UDF second')

def test_timestamp_record_not_initialized():
    ts = pycdlib.udf.UDFTimestamp()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ts.record()
    assert(str(excinfo.value) == 'UDF Timestamp not initialized')

def test_timestamp_new_initialized_twice():
    ts = pycdlib.udf.UDFTimestamp()
    ts.new(time.time())
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ts.new(time.time())
    assert(str(excinfo.value) == 'UDF Timestamp already initialized')

def test_timestamp_equal():
    ts = pycdlib.udf.UDFTimestamp()
    ts.new(time.time())

    ts2 = pycdlib.udf.UDFTimestamp()
    ts2.new(time.time())

    assert(ts == ts2)

def test_timestamp_eq_not_same_object_type():
    ts = pycdlib.udf.UDFTimestamp()
    ts.new(time.time())

    not_a_ts = object()

    assert(ts != not_a_ts)

# EntityID
def test_entityid_parse_initialized_twice():
    entity = pycdlib.udf.UDFEntityID()
    entity.parse(b'\x00'*32)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        entity.parse(b'\x00'*32)
    assert(str(excinfo.value) == 'UDF Entity ID already initialized')

def test_entityid_parse_bad_flags():
    entity = pycdlib.udf.UDFEntityID()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        entity.parse(b'\x04' + b'\x00'*31)
    assert(str(excinfo.value) == 'UDF Entity ID flags must be between 0 and 3')

def test_entityid_record_not_initialized():
    entity = pycdlib.udf.UDFEntityID()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        entity.record()
    assert(str(excinfo.value) == 'UDF Entity ID not initialized')

def test_entityid_new_initialized_twice():
    entity = pycdlib.udf.UDFEntityID()
    entity.new(0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        entity.new(0)
    assert(str(excinfo.value) == 'UDF Entity ID already initialized')

def test_entityid_new_bad_flags():
    entity = pycdlib.udf.UDFEntityID()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        entity.new(4)
    assert(str(excinfo.value) == 'UDF Entity ID flags must be between 0 and 3')

def test_entityid_new_bad_identifier():
    entity = pycdlib.udf.UDFEntityID()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        entity.new(0, b'\x00'*25)
    assert(str(excinfo.value) == 'UDF Entity ID identifier must be less than 23 characters')

def test_entityid_new_bad_suffix():
    entity = pycdlib.udf.UDFEntityID()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        entity.new(0, b'', b'\x00'*10)
    assert(str(excinfo.value) == 'UDF Entity ID suffix must be less than 8 characters')

def test_entityid_equals():
    entity = pycdlib.udf.UDFEntityID()
    entity.new(0)

    entity2 = pycdlib.udf.UDFEntityID()
    entity2.new(0)

    assert(entity == entity2)

def test_entityid_eq_not_same_object_type():
    entity = pycdlib.udf.UDFEntityID()
    entity.new(0)

    not_an_entity = object()

    assert(entity != not_an_entity)

# Charspec
def test_charspec_parse_initialized_twice():
    charspec = pycdlib.udf.UDFCharspec()
    charspec.parse(b'\x00'*64)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        charspec.parse(b'\x00'*64)
    assert(str(excinfo.value) == 'UDF Charspec already initialized')

def test_charspec_parse_bad_set_type():
    charspec = pycdlib.udf.UDFCharspec()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        charspec.parse(b'\x09' + b'\x00'*63)
    assert(str(excinfo.value) == 'Invalid charset parsed; only 0-8 supported')

def test_charspec_new_initialized_twice():
    charspec = pycdlib.udf.UDFCharspec()
    charspec.new(0, b'')
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        charspec.new(0, b'')
    assert(str(excinfo.value) == 'UDF Charspec already initialized')

def test_charspec_new_bad_set_type():
    charspec = pycdlib.udf.UDFCharspec()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        charspec.new(9, b'')
    assert(str(excinfo.value) == 'Invalid charset specified; only 0-8 supported')

def test_charspec_new_bad_set_information():
    charspec = pycdlib.udf.UDFCharspec()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        charspec.new(0, b'\x00'*64)
    assert(str(excinfo.value) == 'Invalid charset information; exceeds maximum size of 63')

def test_charspec_record_not_initialized():
    charspec = pycdlib.udf.UDFCharspec()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        charspec.record()
    assert(str(excinfo.value) == 'UDF Charspec not initialized')

def test_charspec_equal():
    charspec = pycdlib.udf.UDFCharspec()
    charspec.new(0, b'\x00'*63)

    charspec2 = pycdlib.udf.UDFCharspec()
    charspec2.new(0, b'\x00'*63)

    assert(charspec == charspec2)

def test_charspec_eq_not_same_object_type():
    charspec = pycdlib.udf.UDFCharspec()
    charspec.new(0, b'\x00'*63)

    not_a_charspec = object()

    assert(charspec != not_a_charspec)

# ExtentAD
def test_extentad_parse_initialized_twice():
    extentad = pycdlib.udf.UDFExtentAD()
    extentad.parse(b'\x00\x00\x00\x00\x00\x00\x00\x00')
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        extentad.parse(b'\x00\x00\x00\x00\x00\x00\x00\x00')
    assert(str(excinfo.value) == 'UDF Extent descriptor already initialized')

def test_extentad_parse_bad_length():
    extentad = pycdlib.udf.UDFExtentAD()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        extentad.parse(b'\x00\x00\x00\xff\x00\x00\x00\x00')
    assert(str(excinfo.value) == 'UDF Extent descriptor length must be less than 0x3fffffff')

def test_extentad_record_not_initialized():
    extentad = pycdlib.udf.UDFExtentAD()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        extentad.record()
    assert(str(excinfo.value) == 'UDF Extent AD not initialized')

def test_extentad_new_initialized_twice():
    extentad = pycdlib.udf.UDFExtentAD()
    extentad.new(0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        extentad.new(0, 0)
    assert(str(excinfo.value) == 'UDF Extent AD already initialized')

def test_extentad_new_bad_length():
    extentad = pycdlib.udf.UDFExtentAD()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        extentad.new(0xff000000, 0)
    assert(str(excinfo.value) == 'UDF Extent AD length must be less than 0x3fffffff')

def test_extentad_equals():
    extentad = pycdlib.udf.UDFExtentAD()
    extentad.new(0, 0)

    extentad2 = pycdlib.udf.UDFExtentAD()
    extentad2.new(0, 0)

    assert(extentad == extentad2)

def test_extentad_eq_not_same_object_type():
    extentad = pycdlib.udf.UDFExtentAD()
    extentad.new(0, 0)

    not_an_extentad = object()

    assert(extentad != not_an_extentad)

# PVD
def test_pvd_parse_initialized_twice():
    pvd = pycdlib.udf.UDFPrimaryVolumeDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    pvd.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*32 + b'\x01\x00\x01\x00\x02\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00' + b'\x00'*128 + b'\x00'*64 + b'\x00'*64 + b'\x00'*8 + b'\x00'*8 + b'\x00'*32 + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00'*32 + b'\x00'*64 + b'\x00\x00\x00\x00\x00\x00' + b'\x00'*22, 0, tag)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        pvd.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*32 + b'\x01\x00\x01\x00\x02\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00' + b'\x00'*128 + b'\x00'*64 + b'\x00'*64 + b'\x00'*8 + b'\x00'*8 + b'\x00'*32 + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00'*32 + b'\x00'*64 + b'\x00\x00\x00\x00\x00\x00' + b'\x00'*22, 0, tag)
    assert(str(excinfo.value) == 'UDF Primary Volume Descriptor already initialized')

def test_pvd_parse_bad_volseqnum():
    pvd = pycdlib.udf.UDFPrimaryVolumeDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        pvd.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*32 + b'\x00\x00\x01\x00\x02\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00' + b'\x00'*128 + b'\x00'*64 + b'\x00'*64 + b'\x00'*8 + b'\x00'*8 + b'\x00'*32 + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00'*32 + b'\x00'*64 + b'\x00\x00\x00\x00\x00\x00' + b'\x00'*22, 0, tag)
    assert(str(excinfo.value) == 'Only DVD Read-Only disks are supported')

def test_pvd_parse_bad_maxvolseqnum():
    pvd = pycdlib.udf.UDFPrimaryVolumeDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        pvd.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*32 + b'\x01\x00\x00\x00\x02\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00' + b'\x00'*128 + b'\x00'*64 + b'\x00'*64 + b'\x00'*8 + b'\x00'*8 + b'\x00'*32 + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00'*32 + b'\x00'*64 + b'\x00\x00\x00\x00\x00\x00' + b'\x00'*22, 0, tag)
    assert(str(excinfo.value) == 'Only DVD Read-Only disks are supported')

def test_pvd_parse_bad_interchangelevel():
    pvd = pycdlib.udf.UDFPrimaryVolumeDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        pvd.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*32 + b'\x01\x00\x01\x00\x01\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00' + b'\x00'*128 + b'\x00'*64 + b'\x00'*64 + b'\x00'*8 + b'\x00'*8 + b'\x00'*32 + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00'*32 + b'\x00'*64 + b'\x00\x00\x00\x00\x00\x00' + b'\x00'*22, 0, tag)
    assert(str(excinfo.value) == 'Unsupported interchange level (only 2 and 3 supported)')

def test_pvd_parse_bad_charsetlist():
    pvd = pycdlib.udf.UDFPrimaryVolumeDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        pvd.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*32 + b'\x01\x00\x01\x00\x02\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00' + b'\x00'*128 + b'\x00'*64 + b'\x00'*64 + b'\x00'*8 + b'\x00'*8 + b'\x00'*32 + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00'*32 + b'\x00'*64 + b'\x00\x00\x00\x00\x00\x00' + b'\x00'*22, 0, tag)
    assert(str(excinfo.value) == 'Only DVD Read-Only disks are supported')

def test_pvd_parse_bad_maxcharsetlist():
    pvd = pycdlib.udf.UDFPrimaryVolumeDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        pvd.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*32 + b'\x01\x00\x01\x00\x02\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*128 + b'\x00'*64 + b'\x00'*64 + b'\x00'*8 + b'\x00'*8 + b'\x00'*32 + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00'*32 + b'\x00'*64 + b'\x00\x00\x00\x00\x00\x00' + b'\x00'*22, 0, tag)
    assert(str(excinfo.value) == 'Only DVD Read-Only disks are supported')

def test_pvd_parse_bad_flags():
    pvd = pycdlib.udf.UDFPrimaryVolumeDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        pvd.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*32 + b'\x01\x00\x01\x00\x02\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00' + b'\x00'*128 + b'\x00'*64 + b'\x00'*64 + b'\x00'*8 + b'\x00'*8 + b'\x00'*32 + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00'*32 + b'\x00'*64 + b'\x00\x00\x00\x00\x02\x00' + b'\x00'*22, 0, tag)
    assert(str(excinfo.value) == 'Invalid UDF flags')

def test_pvd_parse_bad_reserved():
    pvd = pycdlib.udf.UDFPrimaryVolumeDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        pvd.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*32 + b'\x01\x00\x01\x00\x02\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00' + b'\x00'*128 + b'\x00'*64 + b'\x00'*64 + b'\x00'*8 + b'\x00'*8 + b'\x00'*32 + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00'*32 + b'\x00'*64 + b'\x00\x00\x00\x00\x00\x00' + b'\x01' + b'\x00'*21, 0, tag)
    assert(str(excinfo.value) == 'UDF Primary Volume Descriptor reserved data not 0')

def test_pvd_record_not_initialized():
    pvd = pycdlib.udf.UDFPrimaryVolumeDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        pvd.record()
    assert(str(excinfo.value) == 'UDF Primary Volume Descriptor not initialized')

def test_pvd_extent_location_not_initialized():
    pvd = pycdlib.udf.UDFPrimaryVolumeDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        pvd.extent_location()
    assert(str(excinfo.value) == 'UDF Primary Volume Descriptor not initialized')

def test_pvd_new_initialized_twice():
    pvd = pycdlib.udf.UDFPrimaryVolumeDescriptor()
    pvd.new()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        pvd.new()
    assert(str(excinfo.value) == 'UDF Primary Volume Descriptor already initialized')

def test_pvd_set_extent_location_not_initialized():
    pvd = pycdlib.udf.UDFPrimaryVolumeDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        pvd.set_extent_location(0)
    assert(str(excinfo.value) == 'UDF Primary Volume Descriptor not initialized')

def test_pvd_equals():
    pvd = pycdlib.udf.UDFPrimaryVolumeDescriptor()
    pvd.new()

    pvd2 = pycdlib.udf.UDFPrimaryVolumeDescriptor()
    pvd2.new()
    pvd2.vol_set_ident = pvd.vol_set_ident
    pvd2.recording_date = pvd.recording_date

    assert(pvd == pvd2)

def test_pvd_eq_not_same_object_type():
    pvd = pycdlib.udf.UDFPrimaryVolumeDescriptor()
    pvd.new()

    not_a_pvd = object()

    assert(pvd != not_a_pvd)

# Implementation Use Volume Descriptor Implementation Use
def test_impl_use_impl_use_parse_initialized_twice():
    impl = pycdlib.udf.UDFImplementationUseVolumeDescriptorImplementationUse()
    impl.parse(b'\x00'*64 + b'\x00'*128 + b'\x00'*36 + b'\x00'*36 + b'\x00'*36 + b'\x00'*32 + b'\x00'*128)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        impl.parse(b'\x00'*64 + b'\x00'*128 + b'\x00'*36 + b'\x00'*36 + b'\x00'*36 + b'\x00'*32 + b'\x00'*128)
    assert(str(excinfo.value) == 'UDF Implementation Use Volume Descriptor Implementation Use field already initialized')

def test_impl_use_impl_use_record_not_initialized():
    impl = pycdlib.udf.UDFImplementationUseVolumeDescriptorImplementationUse()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        impl.record()
    assert(str(excinfo.value) == 'UDF Implementation Use Volume Descriptor Implementation Use field not initialized')

def test_impl_use_impl_use_new_initialized_twice():
    impl = pycdlib.udf.UDFImplementationUseVolumeDescriptorImplementationUse()
    impl.new()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        impl.new()
    assert(str(excinfo.value) == 'UDF Implementation Use Volume Descriptor Implementation Use field already initialized')

def test_impl_use_impl_use_equals():
    impl = pycdlib.udf.UDFImplementationUseVolumeDescriptorImplementationUse()
    impl.new()

    impl2 = pycdlib.udf.UDFImplementationUseVolumeDescriptorImplementationUse()
    impl2.new()

    assert(impl == impl2)

def test_impl_use_impl_use_eq_not_same_object_type():
    impl = pycdlib.udf.UDFImplementationUseVolumeDescriptorImplementationUse()
    impl.new()

    not_an_impl = object()

    assert(impl != not_an_impl)

# Implementation Use
def test_impl_use_parse_initialized_twice():
    impl = pycdlib.udf.UDFImplementationUseVolumeDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    impl.parse(b'\x00'*16 + b'\x00\x00\x00\x00' + b'\x00*UDF LV Info' + b'\x00'*19 + b'\x00'*460, 0, tag)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        impl.parse(b'\x00'*16 + b'\x00\x00\x00\x00' + b'\x00*UDF LV Info' + b'\x00'*19 + b'\x00'*460, 0, tag)
    assert(str(excinfo.value) == 'UDF Implementation Use Volume Descriptor already initialized')

def test_impl_use_parse_bad_ident():
    impl = pycdlib.udf.UDFImplementationUseVolumeDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        impl.parse(b'\x00'*16 + b'\x00\x00\x00\x00' + b'\x00*MDF LV Info' + b'\x00'*19 + b'\x00'*460, 0, tag)
    assert(str(excinfo.value) == "Implementation Use Identifier not '*UDF LV Info'")

def test_impl_use_record_not_initialized():
    impl = pycdlib.udf.UDFImplementationUseVolumeDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        impl.record()
    assert(str(excinfo.value) == 'UDF Implementation Use Volume Descriptor not initialized')

def test_impl_use_extent_location_not_initialized():
    impl = pycdlib.udf.UDFImplementationUseVolumeDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        impl.extent_location()
    assert(str(excinfo.value) == 'UDF Implementation Use Volume Descriptor not initialized')

def test_impl_use_new_initialized_twice():
    impl = pycdlib.udf.UDFImplementationUseVolumeDescriptor()
    impl.new()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        impl.new()
    assert(str(excinfo.value) == 'UDF Implementation Use Volume Descriptor already initialized')

def test_impl_use_set_extent_location_not_initialized():
    impl = pycdlib.udf.UDFImplementationUseVolumeDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        impl.set_extent_location(0)
    assert(str(excinfo.value) == 'UDF Implementation Use Volume Descriptor not initialized')

def test_impl_use_equals():
    impl = pycdlib.udf.UDFImplementationUseVolumeDescriptor()
    impl.new()

    impl2 = pycdlib.udf.UDFImplementationUseVolumeDescriptor()
    impl2.new()

    assert(impl == impl2)

def test_impl_use_eq_not_same_object_type():
    impl = pycdlib.udf.UDFImplementationUseVolumeDescriptor()
    impl.new()

    not_an_impl = object()

    assert(impl != not_an_impl)

# Partition Header Descriptor
def test_part_header_parse_initialized_twice():
    header = pycdlib.udf.UDFPartitionHeaderDescriptor()
    header.parse(b'\x00'*8 + b'\x00'*8 + b'\x00'*8 + b'\x00'*8 + b'\x00'*8 + b'\x00'*88)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        header.parse(b'\x00'*8 + b'\x00'*8 + b'\x00'*8 + b'\x00'*8 + b'\x00'*8 + b'\x00'*88)
    assert(str(excinfo.value) == 'UDF Partition Header Descriptor already initialized')

def test_part_header_record_not_initialized():
    header = pycdlib.udf.UDFPartitionHeaderDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        header.record()
    assert(str(excinfo.value) == 'UDF Partition Header Descriptor not initialized')

def test_part_header_new_initialized_twice():
    header = pycdlib.udf.UDFPartitionHeaderDescriptor()
    header.new()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        header.new()
    assert(str(excinfo.value) == 'UDF Partition Header Descriptor already initialized')

def test_part_header_equals():
    header = pycdlib.udf.UDFPartitionHeaderDescriptor()
    header.new()

    header2 = pycdlib.udf.UDFPartitionHeaderDescriptor()
    header2.new()

    assert(header == header2)

def test_part_header_eq_not_same_object_type():
    header = pycdlib.udf.UDFPartitionHeaderDescriptor()
    header.new()

    not_a_header = object()

    assert(header != not_a_header)

# Partition Volume
def test_part_parse_initialized_twice():
    part = pycdlib.udf.UDFPartitionVolumeDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    part.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00+NSR02' + b'\x00'*25 + b'\x00'*128 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*32 + b'\x00'*128 + b'\x00'*156, 0, tag)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        part.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00+NSR02' + b'\x00'*25 + b'\x00'*128 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*32 + b'\x00'*128 + b'\x00'*156, 0, tag)
    assert(str(excinfo.value) == 'UDF Partition Volume Descriptor already initialized')

def test_part_parse_bad_flags():
    part = pycdlib.udf.UDFPartitionVolumeDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        part.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x02\x00\x00\x00' + b'\x00+NSR02' + b'\x00'*25 + b'\x00'*128 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*32 + b'\x00'*128 + b'\x00'*156, 0, tag)
    assert(str(excinfo.value) == 'Invalid partition flags')

def test_part_parse_bad_entity():
    part = pycdlib.udf.UDFPartitionVolumeDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        part.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00+NSR04' + b'\x00'*25 + b'\x00'*128 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*32 + b'\x00'*128 + b'\x00'*156, 0, tag)
    assert(str(excinfo.value) == "Partition Contents Identifier not '+FDC01', '+CD001', '+CDW02', '+NSR02', or '+NSR03'")

def test_part_parse_bad_access_type():
    part = pycdlib.udf.UDFPartitionVolumeDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        part.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00+NSR02' + b'\x00'*25 + b'\x00'*128 + b'\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*32 + b'\x00'*128 + b'\x00'*156, 0, tag)
    assert(str(excinfo.value) == 'Invalid UDF partition access type')

def test_part_record_not_initialized():
    part = pycdlib.udf.UDFPartitionVolumeDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        part.record()
    assert(str(excinfo.value) == 'UDF Partition Volume Descriptor not initialized')

def test_part_extent_location_not_initialized():
    part = pycdlib.udf.UDFPartitionVolumeDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        part.extent_location()
    assert(str(excinfo.value) == 'UDF Partition Volume Descriptor not initialized')

def test_part_new_initialized_twice():
    part = pycdlib.udf.UDFPartitionVolumeDescriptor()
    part.new(2)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        part.new(2)
    assert(str(excinfo.value) == 'UDF Partition Volume Descriptor already initialized')

def test_part_new_initialized_version3():
    part = pycdlib.udf.UDFPartitionVolumeDescriptor()
    part.new(3)
    assert(part.part_contents.identifier[:6] == b'+NSR03')

def test_part_new_bad_version():
    part = pycdlib.udf.UDFPartitionVolumeDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        part.new(4)
    assert(str(excinfo.value) == 'Invalid NSR version requested')

def test_part_set_extent_location_not_initialized():
    part = pycdlib.udf.UDFPartitionVolumeDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        part.set_extent_location(0)
    assert(str(excinfo.value) == 'UDF Partition Volume Descriptor not initialized')

def test_part_set_start_location_not_initialized():
    part = pycdlib.udf.UDFPartitionVolumeDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        part.set_start_location(0)
    assert(str(excinfo.value) == 'UDF Partition Volume Descriptor not initialized')

def test_part_equals():
    part = pycdlib.udf.UDFPartitionVolumeDescriptor()
    part.new(3)

    part2 = pycdlib.udf.UDFPartitionVolumeDescriptor()
    part2.new(3)

    assert(part == part2)

def test_part_eq_not_same_object_type():
    part = pycdlib.udf.UDFPartitionVolumeDescriptor()
    part.new(3)

    not_a_part = object()

    assert(part != not_a_part)

# Type 0 Partition Map
def test_type_zero_part_map_parse_initialized_twice():
    partmap = pycdlib.udf.UDFType0PartitionMap()
    partmap.parse(b'\x00\x02')
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        partmap.parse(b'\x00\x02')
    assert(str(excinfo.value) == 'UDF Type 0 Partition Map already initialized')

def test_type_zero_part_map_parse_bad_map_type():
    partmap = pycdlib.udf.UDFType0PartitionMap()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        partmap.parse(b'\x01\x00')
    assert(str(excinfo.value) == 'UDF Type 0 Partition Map type is not 0')

def test_type_zero_part_map_parse_bad_map_length():
    partmap = pycdlib.udf.UDFType0PartitionMap()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        partmap.parse(b'\x00\x01')
    assert(str(excinfo.value) == 'UDF Type 0 Partition Map length does not equal data length')

def test_type_zero_part_map_parse():
    partmap = pycdlib.udf.UDFType0PartitionMap()
    partmap.parse(b'\x00\x02')
    assert(partmap._initialized)

def test_type_zero_part_map_record_not_initialized():
    partmap = pycdlib.udf.UDFType0PartitionMap()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        partmap.record()
    assert(str(excinfo.value) == 'UDF Type 0 Partition Map not initialized')

def test_type_zero_part_map_record():
    partmap = pycdlib.udf.UDFType0PartitionMap()
    partmap.parse(b'\x00\x02')
    assert(partmap.record() == b'\x00\x02')

def test_type_zero_part_map_new_initialized_twice():
    partmap = pycdlib.udf.UDFType0PartitionMap()
    partmap.new()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        partmap.new()
    assert(str(excinfo.value) == 'UDF Type 0 Partition Map already initialized')

def test_type_zero_part_map_new():
    partmap = pycdlib.udf.UDFType0PartitionMap()
    partmap.new()
    assert(partmap._initialized)

# Type 1 Partition Map
def test_type_one_part_map_parse_initialized_twice():
    partmap = pycdlib.udf.UDFType1PartitionMap()
    partmap.parse(b'\x01\x06\x00\x00\x00\x00')
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        partmap.parse(b'\x01\x06\x00\x00\x00\x00')
    assert(str(excinfo.value) == 'UDF Type 1 Partition Map already initialized')

def test_type_one_part_map_parse_bad_map_type():
    partmap = pycdlib.udf.UDFType1PartitionMap()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        partmap.parse(b'\x00\x06\x00\x00\x00\x00')
    assert(str(excinfo.value) == 'UDF Type 1 Partition Map type is not 1')

def test_type_one_part_map_parse_bad_map_length():
    partmap = pycdlib.udf.UDFType1PartitionMap()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        partmap.parse(b'\x01\x05\x00\x00\x00\x00')
    assert(str(excinfo.value) == 'UDF Type 1 Partition Map length is not 6')

def test_type_one_part_map_record_not_initialized():
    partmap = pycdlib.udf.UDFType1PartitionMap()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        partmap.record()
    assert(str(excinfo.value) == 'UDF Type 1 Partition Map not initialized')

def test_type_one_part_map_new_initialized_twice():
    partmap = pycdlib.udf.UDFType1PartitionMap()
    partmap.new()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        partmap.new()
    assert(str(excinfo.value) == 'UDF Type 1 Partition Map already initialized')

# Type 2 Partition Map
def test_type_two_part_map_parse_initialized_twice():
    partmap = pycdlib.udf.UDFType2PartitionMap()
    partmap.parse(b'\x02\x40' + b'\x00'*62)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        partmap.parse(b'\x02\x40' + b'\x00'*62)
    assert(str(excinfo.value) == 'UDF Type 2 Partition Map already initialized')

def test_type_two_part_map_parse_bad_map_type():
    partmap = pycdlib.udf.UDFType2PartitionMap()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        partmap.parse(b'\x00\x40' + b'\x00'*62)
    assert(str(excinfo.value) == 'UDF Type 2 Partition Map type is not 2')

def test_type_two_part_map_parse_bad_map_length():
    partmap = pycdlib.udf.UDFType2PartitionMap()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        partmap.parse(b'\x02\x20' + b'\x00'*62)
    assert(str(excinfo.value) == 'UDF Type 2 Partition Map length is not 64')

def test_type_two_part_map_parse():
    partmap = pycdlib.udf.UDFType2PartitionMap()
    partmap.parse(b'\x02\x40' + b'\x00'*62)
    assert(partmap._initialized)

def test_type_two_part_map_record_not_initialized():
    partmap = pycdlib.udf.UDFType2PartitionMap()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        partmap.record()
    assert(str(excinfo.value) == 'UDF Type 2 Partition Map not initialized')

def test_type_two_part_map_record():
    partmap = pycdlib.udf.UDFType2PartitionMap()
    partmap.parse(b'\x02\x40' + b'\x00'*62)
    assert(partmap.record() == (b'\x02\x40' + b'\x00'*62))

def test_type_two_part_map_new_initialized_twice():
    partmap = pycdlib.udf.UDFType2PartitionMap()
    partmap.new()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        partmap.new()
    assert(str(excinfo.value) == 'UDF Type 2 Partition Map already initialized')

def test_type_two_part_map_new():
    partmap = pycdlib.udf.UDFType2PartitionMap()
    partmap.new()
    assert(partmap._initialized)

# Extended AD
def test_extendedad_parse_initialized_twice():
    ad = pycdlib.udf.UDFExtendedAD()
    ad.parse(b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*6 + b'\x00'*2)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ad.parse(b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*6 + b'\x00'*2)
    assert(str(excinfo.value) == 'UDF Extended Allocation descriptor already initialized')

def test_extendedad_parse():
    ad = pycdlib.udf.UDFExtendedAD()
    ad.parse(b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*6 + b'\x00'*2)
    assert(ad._initialized)
    assert(ad.extent_length == 0)

def test_extendedad_record_not_initialized():
    ad = pycdlib.udf.UDFExtendedAD()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ad.record()
    assert(str(excinfo.value) == 'UDF Extended Allocation Descriptor not initialized')

def test_extendedad_record():
    ad = pycdlib.udf.UDFExtendedAD()
    ad.parse(b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*6 + b'\x00'*2)
    rec = ad.record()
    assert(rec == b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*6 + b'\x00'*2)

def test_extendedad_new_initialized_twice():
    ad = pycdlib.udf.UDFExtendedAD()
    ad.new()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ad.new()
    assert(str(excinfo.value) == 'UDF Extended Allocation Descriptor already initialized')

def test_extendedad_new():
    ad = pycdlib.udf.UDFExtendedAD()
    ad.new()
    assert(ad._initialized)
    assert(ad.extent_length == 0)

# Short AD
def test_shortad_parse_initialized_twice():
    ad = pycdlib.udf.UDFShortAD()
    ad.parse(b'\x00\x00\x00\x00\x00\x00\x00\x00')
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ad.parse(b'\x00\x00\x00\x00\x00\x00\x00\x00')
    assert(str(excinfo.value) == 'UDF Short Allocation descriptor already initialized')

def test_shortad_record_not_initialized():
    ad = pycdlib.udf.UDFShortAD()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ad.record()
    assert(str(excinfo.value) == 'UDF Short AD not initialized')

def test_shortad_new_initialized_twice():
    ad = pycdlib.udf.UDFShortAD()
    ad.new(0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ad.new(0)
    assert(str(excinfo.value) == 'UDF Short AD already initialized')

def test_shortad_new_bad_length():
    ad = pycdlib.udf.UDFShortAD()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ad.new(0x40000000)
    assert(str(excinfo.value) == 'UDF Short AD length must be less than or equal to 0x3fffffff')

def test_shortad_set_extent_location_not_initialized():
    ad = pycdlib.udf.UDFShortAD()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ad.set_extent_location(0, 0)
    assert(str(excinfo.value) == 'UDF Short AD not initialized')

def test_shortad_set_extent_location():
    ad = pycdlib.udf.UDFShortAD()
    ad.new(0)
    ad.set_extent_location(0, 1)
    assert(ad.log_block_num == 1)

def test_shortad_eq_not_same_object_type():
    ad = pycdlib.udf.UDFShortAD()
    ad.new(0)

    not_an_ad = object()

    assert(ad != not_an_ad)

# Long AD
def test_longad_parse_initialized_twice():
    ad = pycdlib.udf.UDFLongAD()
    ad.parse(b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00')
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ad.parse(b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00')
    assert(str(excinfo.value) == 'UDF Long Allocation descriptor already initialized')

def test_longad_record_not_initialized():
    ad = pycdlib.udf.UDFLongAD()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ad.record()
    assert(str(excinfo.value) == 'UDF Long AD not initialized')

def test_longad_new_initialized_twice():
    ad = pycdlib.udf.UDFLongAD()
    ad.new(0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ad.new(0, 0)
    assert(str(excinfo.value) == 'UDF Long AD already initialized')

def test_longad_set_extent_location_not_initialized():
    ad = pycdlib.udf.UDFLongAD()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ad.set_extent_location(0, 0)
    assert(str(excinfo.value) == 'UDF Long AD not initialized')

def test_longad_length():
    ad = pycdlib.udf.UDFLongAD()
    assert(ad.length() == 16)

def test_longad_equals():
    ad = pycdlib.udf.UDFLongAD()
    ad.new(0, 0)

    ad2 = pycdlib.udf.UDFLongAD()
    ad2.new(0, 0)

    assert(ad == ad2)

def test_longad_eq_not_same_object_type():
    ad = pycdlib.udf.UDFLongAD()
    ad.new(0, 0)

    not_an_ad = object()

    assert(ad != not_an_ad)

# Inline AD
def test_inlinead_parse_initialized_twice():
    ad = pycdlib.udf.UDFInlineAD()
    ad.parse(0, 0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ad.parse(0, 0, 0)
    assert(str(excinfo.value) == 'UDF Inline Allocation Descriptor already initialized')

def test_inlinead_record_not_initialized():
    ad = pycdlib.udf.UDFInlineAD()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ad.record()
    assert(str(excinfo.value) == 'UDF Inline AD not initialized')

def test_inlinead_record():
    ad = pycdlib.udf.UDFInlineAD()
    ad.parse(0, 0, 0)
    assert(ad.record() == b'')

def test_inlinead_new_initialized_twice():
    ad = pycdlib.udf.UDFInlineAD()
    ad.new(0, 0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ad.new(0, 0, 0)
    assert(str(excinfo.value) == 'UDF Inline AD already initialized')

def test_inlinead_set_extent_location_not_initialized():
    ad = pycdlib.udf.UDFInlineAD()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ad.set_extent_location(1, 1)
    assert(str(excinfo.value) == 'UDF Inline AD not initialized')

def test_inlinead_set_extent_location():
    ad = pycdlib.udf.UDFInlineAD()
    ad.new(0, 0, 0)
    ad.set_extent_location(1, 1)
    assert(ad.log_block_num == 1)

def test_inlinead_length_not_initialized():
    ad = pycdlib.udf.UDFInlineAD()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ad.length()
    assert(str(excinfo.value) == 'UDF Inline AD not initialized')

def test_inlinead_length():
    ad = pycdlib.udf.UDFInlineAD()
    ad.new(1, 0, 0)
    assert(ad.length() == 1)

# Logical Volume Descriptor
def test_logvoldesc_parse_initialized_twice():
    logvol = pycdlib.udf.UDFLogicalVolumeDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    logvol.parse(b'\x00'*16 + b'\x00\x00\x00\x00' + b'\x00'*64 + b'\x00'*128 + b'\x00\x08\x00\x00' + b'\x00*OSTA UDF Compliant' + b'\x00'*12 + b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*32 + b'\x00'*128 + b'\x00'*8 + b'\x00'*72, 0, tag)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        logvol.parse(b'\x00'*16 + b'\x00\x00\x00\x00' + b'\x00'*64 + b'\x00'*128 + b'\x00\x08\x00\x00' + b'\x00*OSTA UDF Compliant' + b'\x00'*12 + b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*32 + b'\x00'*128 + b'\x00'*8 + b'\x00'*72, 0, tag)
    assert(str(excinfo.value) == 'UDF Logical Volume Descriptor already initialized')

def test_logvoldesc_parse_bad_logical_block_size():
    logvol = pycdlib.udf.UDFLogicalVolumeDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        logvol.parse(b'\x00'*16 + b'\x00\x00\x00\x00' + b'\x00'*64 + b'\x00'*128 + b'\x00\x07\x00\x00' + b'\x00*OSTA UDF Compliant' + b'\x00'*12 + b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*32 + b'\x00'*128 + b'\x00'*8 + b'\x00'*72, 0, tag)
    assert(str(excinfo.value) == 'Volume Descriptor block size is not 2048')

def test_logvoldesc_parse_bad_domain_ident():
    logvol = pycdlib.udf.UDFLogicalVolumeDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        logvol.parse(b'\x00'*16 + b'\x00\x00\x00\x00' + b'\x00'*64 + b'\x00'*128 + b'\x00\x08\x00\x00' + b'\x00$OSTA UDF Compliant' + b'\x00'*12 + b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*32 + b'\x00'*128 + b'\x00'*8 + b'\x00'*72, 0, tag)
    assert(str(excinfo.value) == "Volume Descriptor Identifier not '*OSTA UDF Compliant'")

def test_logvoldesc_parse_bad_map_table_length():
    logvol = pycdlib.udf.UDFLogicalVolumeDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        logvol.parse(b'\x00'*16 + b'\x00\x00\x00\x00' + b'\x00'*64 + b'\x00'*128 + b'\x00\x08\x00\x00' + b'\x00*OSTA UDF Compliant' + b'\x00'*12 + b'\x00'*16 + b'\x00\x00\x00\x10\x00\x00\x00\x00' + b'\x00'*32 + b'\x00'*128 + b'\x00'*8 + b'\x00'*72, 0, tag)
    assert(str(excinfo.value) == 'Map table length greater than size of partition map data; ISO corrupt')

def test_logvoldesc_parse_bad_map_data():
    logvol = pycdlib.udf.UDFLogicalVolumeDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        logvol.parse(b'\x00'*16 + b'\x00\x00\x00\x00' + b'\x00'*64 + b'\x00'*128 + b'\x00\x08\x00\x00' + b'\x00*OSTA UDF Compliant' + b'\x00'*12 + b'\x00'*16 + b'\x00\x00\x00\x00\x01\x00\x00\x00' + b'\x00'*32 + b'\x00'*128 + b'\x00'*8 + b'\x00\x80' + b'\x00'*70, 0, tag)
    assert(str(excinfo.value) == 'Partition map goes beyond end of data, ISO corrupt')

def test_logvoldesc_parse_not_enough_map_data_left():
    logvol = pycdlib.udf.UDFLogicalVolumeDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        logvol.parse(b'\x00'*16 + b'\x00\x00\x00\x00' + b'\x00'*64 + b'\x00'*128 + b'\x00\x08\x00\x00' + b'\x00*OSTA UDF Compliant' + b'\x00'*12 + b'\x00'*16 + b'\x00\x00\x00\x00\x01\x00\x00\x00' + b'\x00'*32 + b'\x00'*128 + b'\x00'*8 + b'\x00\x20' + b'\x00'*70, 0, tag)
    assert(str(excinfo.value) == 'Partition map goes beyond map_table_length left, ISO corrupt')

def test_logvoldesc_parse_map_type_0():
    logvol = pycdlib.udf.UDFLogicalVolumeDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    logvol.parse(b'\x00'*16 + b'\x00\x00\x00\x00' + b'\x00'*64 + b'\x00'*128 + b'\x00\x08\x00\x00' + b'\x00*OSTA UDF Compliant' + b'\x00'*12 + b'\x00'*16 + b'\x02\x00\x00\x00\x01\x00\x00\x00' + b'\x00'*32 + b'\x00'*128 + b'\x00'*8 + b'\x00\x02' + b'\x00'*70, 0, tag)
    assert(len(logvol.partition_maps) == 1)
    assert(isinstance(logvol.partition_maps[0], pycdlib.udf.UDFType0PartitionMap))

def test_logvoldesc_parse_map_type_2():
    logvol = pycdlib.udf.UDFLogicalVolumeDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    logvol.parse(b'\x00'*16 + b'\x00\x00\x00\x00' + b'\x00'*64 + b'\x00'*128 + b'\x00\x08\x00\x00' + b'\x00*OSTA UDF Compliant' + b'\x00'*12 + b'\x00'*16 + b'\x42\x00\x00\x00\x01\x00\x00\x00' + b'\x00'*32 + b'\x00'*128 + b'\x00'*8 + b'\x02\x40' + b'\x00'*70, 0, tag)
    assert(len(logvol.partition_maps) == 1)
    assert(isinstance(logvol.partition_maps[0], pycdlib.udf.UDFType2PartitionMap))

def test_logvoldesc_parse_bad_map_type():
    logvol = pycdlib.udf.UDFLogicalVolumeDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        logvol.parse(b'\x00'*16 + b'\x00\x00\x00\x00' + b'\x00'*64 + b'\x00'*128 + b'\x00\x08\x00\x00' + b'\x00*OSTA UDF Compliant' + b'\x00'*12 + b'\x00'*16 + b'\x42\x00\x00\x00\x01\x00\x00\x00' + b'\x00'*32 + b'\x00'*128 + b'\x00'*8 + b'\x03\x40' + b'\x00'*70, 0, tag)
    assert(str(excinfo.value) == 'Unsupported partition map type')

def test_logvoldesc_record_not_initialized():
    logvol = pycdlib.udf.UDFLogicalVolumeDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        logvol.record()
    assert(str(excinfo.value) == 'UDF Logical Volume Descriptor not initialized')

def test_logvoldesc_extent_location_not_initialized():
    logvol = pycdlib.udf.UDFLogicalVolumeDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        logvol.extent_location()
    assert(str(excinfo.value) == 'UDF Logical Volume Descriptor not initialized')

def test_logvoldesc_new_initialized_twice():
    logvol = pycdlib.udf.UDFLogicalVolumeDescriptor()
    logvol.new()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        logvol.new()
    assert(str(excinfo.value) == 'UDF Logical Volume Descriptor already initialized')

def test_logvoldesc_add_partition_map_not_initialized():
    logvol = pycdlib.udf.UDFLogicalVolumeDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        logvol.add_partition_map(1)
    assert(str(excinfo.value) == 'UDF Logical Volume Descriptor not initialized')

def test_logvoldesc_add_partition_map_type_0():
    logvol = pycdlib.udf.UDFLogicalVolumeDescriptor()
    logvol.new()
    logvol.add_partition_map(0)
    assert(len(logvol.partition_maps) == 1)

def test_logvoldesc_add_partition_map_type_2():
    logvol = pycdlib.udf.UDFLogicalVolumeDescriptor()
    logvol.new()
    logvol.add_partition_map(2)
    assert(len(logvol.partition_maps) == 1)

def test_logvoldesc_add_partition_map_invalid_type():
    logvol = pycdlib.udf.UDFLogicalVolumeDescriptor()
    logvol.new()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        logvol.add_partition_map(3)
    assert(str(excinfo.value) == 'UDF Partition map type must be 0, 1, or 2')

def test_logvoldesc_add_partition_map_too_many_maps():
    logvol = pycdlib.udf.UDFLogicalVolumeDescriptor()
    logvol.new()
    logvol.add_partition_map(2)
    logvol.add_partition_map(2)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        logvol.add_partition_map(2)
    assert(str(excinfo.value) == 'Too many UDF partition maps')

def test_logvoldesc_set_extent_location_not_initialized():
    logvol = pycdlib.udf.UDFLogicalVolumeDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        logvol.set_extent_location(0)
    assert(str(excinfo.value) == 'UDF Logical Volume Descriptor not initialized')

def test_logvoldesc_set_integrity_location_not_initialized():
    logvol = pycdlib.udf.UDFLogicalVolumeDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        logvol.set_integrity_location(0)
    assert(str(excinfo.value) == 'UDF Logical Volume Descriptor not initialized')

def test_logvoldesc_equals():
    logvol = pycdlib.udf.UDFLogicalVolumeDescriptor()
    logvol.new()

    logvol2 = pycdlib.udf.UDFLogicalVolumeDescriptor()
    logvol2.new()

    assert(logvol == logvol2)

def test_logvoldesc_eq_not_same_object_type():
    logvol = pycdlib.udf.UDFLogicalVolumeDescriptor()
    logvol.new()

    not_a_logvol = object()

    assert(logvol != not_a_logvol)

# Unallocated Space
def test_unallocated_parse_initialized_twice():
    un = pycdlib.udf.UDFUnallocatedSpaceDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    un.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*488, 0, tag)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        un.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*488, 0, tag)
    assert(str(excinfo.value) == 'UDF Unallocated Space Descriptor already initialized')

def test_unallocated_parse_too_many_alloc_descs():
    un = pycdlib.udf.UDFUnallocatedSpaceDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        un.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x3e\x00\x00\x00' + b'\x00'*488, 0, tag)
    assert(str(excinfo.value) == 'Too many allocation descriptors')

def test_unallocated_parse_alloc_descs():
    un = pycdlib.udf.UDFUnallocatedSpaceDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    un.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x01\x00\x00\x00' + b'\x00'*488, 0, tag)
    assert(len(un.alloc_descs) == 1)

def test_unallocated_record_not_initialized():
    un = pycdlib.udf.UDFUnallocatedSpaceDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        un.record()
    assert(str(excinfo.value) == 'UDF Unallocated Space Descriptor not initialized')

def test_unallocated_record_alloc_descs():
    un = pycdlib.udf.UDFUnallocatedSpaceDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    un.parse(b'\x00\x00\x02\x00\x05\x00\x00\x00\x52\xc0\xf0\x01\x00\x00\x00\x00' + b'\x00\x00\x00\x00\x01\x00\x00\x00' + b'\x00'*488, 0, tag)
    assert(un.record() == b'\x00\x00\x02\x00\x05\x00\x00\x00\x52\xc0\xf0\x01\x00\x00\x00\x00' + b'\x00\x00\x00\x00\x01\x00\x00\x00' + b'\x00'*488)

def test_unallocated_extent_location_not_initialized():
    un = pycdlib.udf.UDFUnallocatedSpaceDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        un.extent_location()
    assert(str(excinfo.value) == 'UDF Unallocated Space Descriptor not initialized')

def test_unallocated_new_initialized_twice():
    un = pycdlib.udf.UDFUnallocatedSpaceDescriptor()
    un.new()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        un.new()
    assert(str(excinfo.value) == 'UDF Unallocated Space Descriptor already initialized')

def test_unallocated_set_extent_location_not_initialized():
    un = pycdlib.udf.UDFUnallocatedSpaceDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        un.set_extent_location(0)
    assert(str(excinfo.value) == 'UDF Unallocated Space Descriptor not initialized')

def test_unallocated_equals():
    un = pycdlib.udf.UDFUnallocatedSpaceDescriptor()
    un.new()

    un2 = pycdlib.udf.UDFUnallocatedSpaceDescriptor()
    un2.new()

    assert(un == un2)

def test_unallocated_eq_not_same_object_type():
    un = pycdlib.udf.UDFUnallocatedSpaceDescriptor()
    un.new()

    not_a_un = object()

    assert(un != not_a_un)

# Terminating Descriptor
def test_terminating_parse_initialized_twice():
    term = pycdlib.udf.UDFTerminatingDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    term.parse(0, tag)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        term.parse(0, tag)
    assert(str(excinfo.value) == 'UDF Terminating Descriptor already initialized')

def test_terminating_record_not_initialized():
    term = pycdlib.udf.UDFTerminatingDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        term.record()
    assert(str(excinfo.value) == 'UDF Terminating Descriptor not initialized')

def test_terminating_extent_location_not_initialized():
    term = pycdlib.udf.UDFTerminatingDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        term.extent_location()
    assert(str(excinfo.value) == 'UDF Terminating Descriptor not initialized')

def test_terminating_new_initialized_twice():
    term = pycdlib.udf.UDFTerminatingDescriptor()
    term.new()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        term.new()
    assert(str(excinfo.value) == 'UDF Terminating Descriptor already initialized')

def test_terminating_set_extent_location_not_initialized():
    term = pycdlib.udf.UDFTerminatingDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        term.set_extent_location(0)
    assert(str(excinfo.value) == 'UDF Terminating Descriptor not initialized')

# Logical Volume Header
def test_logvol_header_parse_initialized_twice():
    header = pycdlib.udf.UDFLogicalVolumeHeaderDescriptor()
    header.parse(b'\x00'*8 + b'\x00'*24)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        header.parse(b'\x00'*8 + b'\x00'*24)
    assert(str(excinfo.value) == 'UDF Logical Volume Header Descriptor already initialized')

def test_logvol_header_record_not_initialized():
    header = pycdlib.udf.UDFLogicalVolumeHeaderDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        header.record()
    assert(str(excinfo.value) == 'UDF Logical Volume Header Descriptor not initialized')

def test_logvol_header_new_initialized_twice():
    header = pycdlib.udf.UDFLogicalVolumeHeaderDescriptor()
    header.new()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        header.new()
    assert(str(excinfo.value) == 'UDF Logical Volume Header Descriptor already initialized')

# Logical Volume Implementation Use
def test_logvol_impl_parse_initialized_twice():
    impl = pycdlib.udf.UDFLogicalVolumeImplementationUse()
    impl.parse(b'\x00'*32 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00')
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        impl.parse(b'\x00'*32 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00')
    assert(str(excinfo.value) == 'UDF Logical Volume Implementation Use already initialized')

def test_logvol_impl_record_not_initialized():
    impl = pycdlib.udf.UDFLogicalVolumeImplementationUse()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        impl.record()
    assert(str(excinfo.value) == 'UDF Logical Volume Implementation Use not initialized')

def test_logvol_impl_new_initialized_twice():
    impl = pycdlib.udf.UDFLogicalVolumeImplementationUse()
    impl.new()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        impl.new()
    assert(str(excinfo.value) == 'UDF Logical Volume Implementation Use already initialized')

# Logical Volume Integrity
def test_logvol_integrity_parse_initialized_twice():
    integrity = pycdlib.udf.UDFLogicalVolumeIntegrityDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    integrity.parse(b'\x00'*16 + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x00\x00' + b'\x00'*8 + b'\x00'*32 + b'\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*432, 0, tag)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        integrity.parse(b'\x00'*16 + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x00\x00' + b'\x00'*8 + b'\x00'*32 + b'\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*432, 0, tag)
    assert(str(excinfo.value) == 'UDF Logical Volume Integrity Descriptor already initialized')

def test_logvol_integrity_parse_bad_integrity_type():
    integrity = pycdlib.udf.UDFLogicalVolumeIntegrityDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        integrity.parse(b'\x00'*16 + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x02\x00\x00\x00' + b'\x00'*8 + b'\x00'*32 + b'\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*432, 0, tag)
    assert(str(excinfo.value) == 'Logical Volume Integrity Type not 0 or 1')

def test_logvol_integrity_parse_bad_impl_use():
    integrity = pycdlib.udf.UDFLogicalVolumeIntegrityDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        integrity.parse(b'\x00'*16 + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x00\x00' + b'\x00'*8 + b'\x00'*32 + b'\x00\x00\x00\x00\xb1\x01\x00\x00' + b'\x00'*432, 0, tag)
    assert(str(excinfo.value) == 'UDF Logical Volume Integrity specified an implementation use that is too large')

def test_logvol_integrity_record_not_initialized():
    integrity = pycdlib.udf.UDFLogicalVolumeIntegrityDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        integrity.record()
    assert(str(excinfo.value) == 'UDF Logical Volume Integrity Descriptor not initialized')

def test_logvol_integrity_extent_location_not_initialized():
    integrity = pycdlib.udf.UDFLogicalVolumeIntegrityDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        integrity.extent_location()
    assert(str(excinfo.value) == 'UDF Logical Volume Integrity Descriptor not initialized')

def test_logvol_integrity_new_initialized_twice():
    integrity = pycdlib.udf.UDFLogicalVolumeIntegrityDescriptor()
    integrity.new()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        integrity.new()
    assert(str(excinfo.value) == 'UDF Logical Volume Integrity Descriptor already initialized')

def test_logvol_integrity_set_extent_location_not_initialized():
    integrity = pycdlib.udf.UDFLogicalVolumeIntegrityDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        integrity.set_extent_location(0)
    assert(str(excinfo.value) == 'UDF Logical Volume Integrity Descriptor not initialized')

# File Set
def test_file_set_parse_initialized_twice():
    fileset = pycdlib.udf.UDFFileSetDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    fileset.parse(b'\x00'*16 + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x03\x00\x03\x00\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*64 + b'\x00'*128 + b'\x00'*64 + b'\x00'*32 + b'\x00'*32 + b'\x00'*32 + b'\x00'*16 + b'\x00*OSTA UDF Compliant' + b'\x00'*12 + b'\x00'*16 + b'\x00'*16 + b'\x00'*32, 0, tag)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        fileset.parse(b'\x00'*16 + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x03\x00\x03\x00\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*64 + b'\x00'*128 + b'\x00'*64 + b'\x00'*32 + b'\x00'*32 + b'\x00'*32 + b'\x00'*16 + b'\x00*OSTA UDF Compliant' + b'\x00'*12 + b'\x00'*16 + b'\x00'*16 + b'\x00'*32, 0, tag)
    assert(str(excinfo.value) == 'UDF File Set Descriptor already initialized')

def test_file_set_parse_bad_interchange():
    fileset = pycdlib.udf.UDFFileSetDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        fileset.parse(b'\x00'*16 + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x02\x00\x03\x00\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*64 + b'\x00'*128 + b'\x00'*64 + b'\x00'*32 + b'\x00'*32 + b'\x00'*32 + b'\x00'*16 + b'\x00*OSTA UDF Compliant' + b'\x00'*12 + b'\x00'*16 + b'\x00'*16 + b'\x00'*32, 0, tag)
    assert(str(excinfo.value) == 'Only DVD Read-Only disks are supported')

def test_file_set_parse_bad_max_interchange():
    fileset = pycdlib.udf.UDFFileSetDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        fileset.parse(b'\x00'*16 + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x03\x00\x02\x00\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*64 + b'\x00'*128 + b'\x00'*64 + b'\x00'*32 + b'\x00'*32 + b'\x00'*32 + b'\x00'*16 + b'\x00*OSTA UDF Compliant' + b'\x00'*12 + b'\x00'*16 + b'\x00'*16 + b'\x00'*32, 0, tag)
    assert(str(excinfo.value) == 'Only DVD Read-Only disks are supported')

def test_file_set_parse_bad_char_set_list():
    fileset = pycdlib.udf.UDFFileSetDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        fileset.parse(b'\x00'*16 + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x03\x00\x03\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*64 + b'\x00'*128 + b'\x00'*64 + b'\x00'*32 + b'\x00'*32 + b'\x00'*32 + b'\x00'*16 + b'\x00*OSTA UDF Compliant' + b'\x00'*12 + b'\x00'*16 + b'\x00'*16 + b'\x00'*32, 0, tag)
    assert(str(excinfo.value) == 'Only DVD Read-Only disks are supported')

def test_file_set_parse_bad_max_char_set_list():
    fileset = pycdlib.udf.UDFFileSetDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        fileset.parse(b'\x00'*16 + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x03\x00\x03\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*64 + b'\x00'*128 + b'\x00'*64 + b'\x00'*32 + b'\x00'*32 + b'\x00'*32 + b'\x00'*16 + b'\x00*OSTA UDF Compliant' + b'\x00'*12 + b'\x00'*16 + b'\x00'*16 + b'\x00'*32, 0, tag)
    assert(str(excinfo.value) == 'Only DVD Read-Only disks are supported')

def test_file_set_parse_bad_file_set_desc_num():
    fileset = pycdlib.udf.UDFFileSetDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        fileset.parse(b'\x00'*16 + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x03\x00\x03\x00\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00' + b'\x00'*64 + b'\x00'*128 + b'\x00'*64 + b'\x00'*32 + b'\x00'*32 + b'\x00'*32 + b'\x00'*16 + b'\x00*OSTA UDF Compliant' + b'\x00'*12 + b'\x00'*16 + b'\x00'*16 + b'\x00'*32, 0, tag)
    assert(str(excinfo.value) == 'Only DVD Read-Only disks are supported')

def test_file_set_parse_bad_domain_ident():
    fileset = pycdlib.udf.UDFFileSetDescriptor()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        fileset.parse(b'\x00'*16 + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x03\x00\x03\x00\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*64 + b'\x00'*128 + b'\x00'*64 + b'\x00'*32 + b'\x00'*32 + b'\x00'*32 + b'\x00'*16 + b'\x00*MSTA UDF Compliant' + b'\x00'*12 + b'\x00'*16 + b'\x00'*16 + b'\x00'*32, 0, tag)
    assert(str(excinfo.value) == "File Set Descriptor Identifier not '*OSTA UDF Compliant'")

def test_file_set_record_not_initialized():
    fileset = pycdlib.udf.UDFFileSetDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        fileset.record()
    assert(str(excinfo.value) == 'UDF File Set Descriptor not initialized')

def test_file_set_extent_location_not_initialized():
    fileset = pycdlib.udf.UDFFileSetDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        fileset.extent_location()
    assert(str(excinfo.value) == 'UDF File Set Descriptor not initialized')

def test_file_set_new_initialized_twice():
    fileset = pycdlib.udf.UDFFileSetDescriptor()
    fileset.new()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        fileset.new()
    assert(str(excinfo.value) == 'UDF File Set Descriptor already initialized')

def test_file_set_set_extent_location_not_initialized():
    fileset = pycdlib.udf.UDFFileSetDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        fileset.set_extent_location(0)
    assert(str(excinfo.value) == 'UDF File Set Descriptor not initialized')

# LBAddr
def test_lbaddr_parse_initialized_twice():
    lb = pycdlib.udf.UDFLBAddr()
    lb.parse(b'\x00\x00\x00\x00\x00\x00')
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        lb.parse(b'\x00\x00\x00\x00\x00\x00')
    assert(str(excinfo.value) == 'UDF LBAddr already initialized')

def test_lbaddr_record_not_initialized():
    lb = pycdlib.udf.UDFLBAddr()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        lb.record()
    assert(str(excinfo.value) == 'UDF LBAddr not initialized')

def test_lbaddr_new_initialized_twice():
    lb = pycdlib.udf.UDFLBAddr()
    lb.new(0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        lb.new(0)
    assert(str(excinfo.value) == 'UDF LBAddr already initialized')

# ICBTag
def test_icbtag_parse_initialized_twice():
    icb = pycdlib.udf.UDFICBTag()
    icb.parse(b'\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00')
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        icb.parse(b'\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00')
    assert(str(excinfo.value) == 'UDF ICB Tag already initialized')

def test_icbtag_record_not_initialized():
    icb = pycdlib.udf.UDFICBTag()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        icb.record()
    assert(str(excinfo.value) == 'UDF ICB Tag not initialized')

def test_icbtag_new_initialized_twice():
    icb = pycdlib.udf.UDFICBTag()
    icb.new('dir')
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        icb.new('dir')
    assert(str(excinfo.value) == 'UDF ICB Tag already initialized')

def test_icbtag_new_bad_file_type():
    icb = pycdlib.udf.UDFICBTag()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        icb.new('foo')
    assert(str(excinfo.value) == "Invalid file type for ICB; must be one of 'dir', 'file', or 'symlink'")

def test_icbtag_parse_bad_strategy_type():
    icb = pycdlib.udf.UDFICBTag()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        icb.parse(b'\x00\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00')
    assert(str(excinfo.value) == 'UDF ICB Tag invalid strategy type')

def test_icbtag_parse_bad_reserved():
    icb = pycdlib.udf.UDFICBTag()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        icb.parse(b'\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00')
    assert(str(excinfo.value) == 'UDF ICB Tag reserved not 0')

# File Entry
def test_file_entry_parse_initialized_twice():
    entry = pycdlib.udf.UDFFileEntry()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    entry.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x01\x00\x00\x00' + b'\x00'*16 + b'\x00'*32 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00', 0, None, tag)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        entry.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x01\x00\x00\x00' + b'\x00'*16 + b'\x00'*32 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00', 0, None, tag)
    assert(str(excinfo.value) == 'UDF File Entry already initialized')

def test_file_entry_parse_bad_record_format():
    entry = pycdlib.udf.UDFFileEntry()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        entry.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x01\x00\x00\x00' + b'\x00'*16 + b'\x00'*32 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00', 0, None, tag)
    assert(str(excinfo.value) == 'File Entry record format is not 0')

def test_file_entry_parse_bad_record_display_attrs():
    entry = pycdlib.udf.UDFFileEntry()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        entry.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x01\x00\x00\x00' + b'\x00'*16 + b'\x00'*32 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00', 0, None, tag)
    assert(str(excinfo.value) == 'File Entry record display attributes is not 0')

def test_file_entry_parse_bad_record_len():
    entry = pycdlib.udf.UDFFileEntry()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        entry.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x01\x00\x00\x00' + b'\x00'*16 + b'\x00'*32 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00', 0, None, tag)
    assert(str(excinfo.value) == 'File Entry record length is not 0')

def test_file_entry_parse_bad_checkpoint():
    entry = pycdlib.udf.UDFFileEntry()
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        entry.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x00\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x00\x00' + b'\x00'*16 + b'\x00'*32 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00', 0, None, tag)
    assert(str(excinfo.value) == 'Only DVD Read-only disks supported')

def test_file_entry_record_not_initialized():
    entry = pycdlib.udf.UDFFileEntry()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        entry.record()
    assert(str(excinfo.value) == 'UDF File Entry not initialized')

def test_file_entry_extent_location_not_initialized():
    entry = pycdlib.udf.UDFFileEntry()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        entry.extent_location()
    assert(str(excinfo.value) == 'UDF File Entry not initialized')

def test_file_entry_new_initialized_twice():
    entry = pycdlib.udf.UDFFileEntry()
    entry.new(0, 'dir', None, 2048)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        entry.new(0, 'dir', None, 2048)
    assert(str(excinfo.value) == 'UDF File Entry already initialized')

def test_file_entry_new_bad_file_type():
    entry = pycdlib.udf.UDFFileEntry()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        entry.new(0, 'foo', None, 2048)
    assert(str(excinfo.value) == "UDF File Entry file type must be one of 'dir', 'file', or 'symlink'")

def test_file_entry_set_extent_location_not_initialized():
    entry = pycdlib.udf.UDFFileEntry()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        entry.set_extent_location(0, 0)
    assert(str(excinfo.value) == 'UDF File Entry not initialized')

def test_file_entry_add_file_ident_desc_not_initialized():
    entry = pycdlib.udf.UDFFileEntry()
    desc = pycdlib.udf.UDFFileIdentifierDescriptor()
    desc.new(False, False, b'foo', None)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        entry.add_file_ident_desc(desc, 2048)
    assert(str(excinfo.value) == 'UDF File Entry not initialized')

def test_file_entry_add_file_ident_desc_bad_file_type():
    entry = pycdlib.udf.UDFFileEntry()
    entry.new(0, 'file', None, 2048)
    desc = pycdlib.udf.UDFFileIdentifierDescriptor()
    desc.new(False, False, b'foo', None)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        entry.add_file_ident_desc(desc, 2048)
    assert(str(excinfo.value) == 'Can only add a UDF File Identifier to a directory')

def test_file_entry_remove_file_ident_desc_not_initialized():
    entry = pycdlib.udf.UDFFileEntry()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        entry.remove_file_ident_desc_by_name(b'foo', 2048)
    assert(str(excinfo.value) == 'UDF File Entry not initialized')

def test_file_entry_remove_file_ident_desc_file_not_found():
    entry = pycdlib.udf.UDFFileEntry()
    entry.new(0, 'dir', None, 2048)
    desc = pycdlib.udf.UDFFileIdentifierDescriptor()
    desc.new(False, False, b'foo', None)
    entry.add_file_ident_desc(desc, 2048)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        entry.remove_file_ident_desc_by_name(b'bar', 2048)
    assert(str(excinfo.value) == 'Cannot find file to remove')

def test_file_entry_set_data_location_not_initialized():
    entry = pycdlib.udf.UDFFileEntry()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        entry.set_data_location(0, 0)
    assert(str(excinfo.value) == 'UDF File Entry not initialized')

def test_file_entry_get_data_length_not_initialized():
    entry = pycdlib.udf.UDFFileEntry()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        entry.get_data_length()
    assert(str(excinfo.value) == 'UDF File Entry not initialized')

def test_file_entry_set_data_length_not_initialized():
    entry = pycdlib.udf.UDFFileEntry()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        entry.set_data_length(0)
    assert(str(excinfo.value) == 'UDF File Entry not initialized')

def test_file_entry_is_file_not_initialized():
    entry = pycdlib.udf.UDFFileEntry()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        entry.is_file()
    assert(str(excinfo.value) == 'UDF File Entry not initialized')

def test_file_entry_is_symlink_not_initialized():
    entry = pycdlib.udf.UDFFileEntry()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        entry.is_symlink()
    assert(str(excinfo.value) == 'UDF File Entry not initialized')

def test_file_entry_is_dir_not_initialized():
    entry = pycdlib.udf.UDFFileEntry()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        entry.is_dir()
    assert(str(excinfo.value) == 'UDF File Entry not initialized')

def test_file_entry_file_identifier_not_initialized():
    entry = pycdlib.udf.UDFFileEntry()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        entry.file_identifier()
    assert(str(excinfo.value) == 'UDF File Entry not initialized')

def test_file_entry_find_file_ident_desc_not_initialized():
    entry = pycdlib.udf.UDFFileEntry()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        entry.find_file_ident_desc_by_name(b'')
    assert(str(excinfo.value) == 'UDF File Entry not initialized')

def test_file_entry_track_file_ident_desc_not_initialized():
    entry = pycdlib.udf.UDFFileEntry()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        entry.track_file_ident_desc(None)
    assert(str(excinfo.value) == 'UDF File Entry not initialized')

def test_file_entry_is_dot_not_initialized():
    entry = pycdlib.udf.UDFFileEntry()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        entry.is_dot()
    assert(str(excinfo.value) == 'UDF File Entry not initialized')

def test_file_entry_is_dotdot_not_initialized():
    entry = pycdlib.udf.UDFFileEntry()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        entry.is_dotdot()
    assert(str(excinfo.value) == 'UDF File Entry not initialized')

# File Identifier
def test_file_ident_parse_initialized_twice():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    entry = pycdlib.udf.UDFFileEntry()
    entry.new(0, 'dir', None, 2048)

    fi = pycdlib.udf.UDFFileIdentifierDescriptor()
    fi.parse(b'\x00'*16 + b'\x01\x00\x08\x00' + b'\x00'*16 + b'\x00\x00', 0, tag, entry)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        fi.parse(b'\x00'*16 + b'\x01\x00\x08\x00' + b'\x00'*16 + b'\x00\x00', 0, tag, entry)
    assert(str(excinfo.value) == 'UDF File Identifier Descriptor already initialized')

def test_file_ident_parse_bad_file_version():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    entry = pycdlib.udf.UDFFileEntry()
    entry.new(0, 'dir', None, 2048)

    fi = pycdlib.udf.UDFFileIdentifierDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        fi.parse(b'\x00'*16 + b'\x00\x00\x08\x00' + b'\x00'*16 + b'\x00\x00', 0, tag, entry)
    assert(str(excinfo.value) == 'File Identifier Descriptor file version number not 1')

def test_file_ident_parse_bad_encoding():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    entry = pycdlib.udf.UDFFileEntry()
    entry.new(0, 'dir', None, 2048)

    fi = pycdlib.udf.UDFFileIdentifierDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        fi.parse(b'\x00'*16 + b'\x01\x00\x00\x01' + b'\x00'*16 + b'\x00\x00\x00', 0, tag, entry)
    assert(str(excinfo.value) == 'Only UDF File Identifier Descriptor Encodings 8 or 16 are supported')

def test_file_ident_is_dir_not_initialized():
    fi = pycdlib.udf.UDFFileIdentifierDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        fi.is_dir()
    assert(str(excinfo.value) == 'UDF File Identifier Descriptor not initialized')

def test_file_ident_is_parent_not_initialized():
    fi = pycdlib.udf.UDFFileIdentifierDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        fi.is_parent()
    assert(str(excinfo.value) == 'UDF File Identifier Descriptor not initialized')

def test_file_ident_record_not_initialized():
    fi = pycdlib.udf.UDFFileIdentifierDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        fi.record()
    assert(str(excinfo.value) == 'UDF File Identifier Descriptor not initialized')

def test_file_ident_record_bad_encoding():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    entry = pycdlib.udf.UDFFileEntry()
    entry.new(0, 'dir', None, 2048)

    fi = pycdlib.udf.UDFFileIdentifierDescriptor()
    fi.parse(b'\x00'*16 + b'\x01\x00\x08\x01' + b'\x00'*16 + b'\x00\x00\x00', 0, tag, entry)
    fi.encoding = 'bad'
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        fi.record()
    assert(str(excinfo.value) == 'Invalid UDF encoding; this should not happen')

def test_file_ident_extent_location_not_initialized():
    fi = pycdlib.udf.UDFFileIdentifierDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        fi.extent_location()
    assert(str(excinfo.value) == 'UDF File Identifier not initialized')

def test_file_ident_new_initialized_twice():
    fi = pycdlib.udf.UDFFileIdentifierDescriptor()
    fi.new(False, False, b'foo', None)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        fi.new(False, False, b'foo', None)
    assert(str(excinfo.value) == 'UDF File Identifier already initialized')

def test_file_ident_set_extent_location_not_initialized():
    fi = pycdlib.udf.UDFFileIdentifierDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        fi.set_extent_location(0, 0)
    assert(str(excinfo.value) == 'UDF File Identifier not initialized')

def test_file_ident_set_icb_not_initialized():
    fi = pycdlib.udf.UDFFileIdentifierDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        fi.set_icb(0, 0)
    assert(str(excinfo.value) == 'UDF File Identifier not initialized')

def test_file_ident_lt():
    fi = pycdlib.udf.UDFFileIdentifierDescriptor()
    fi.new(False, False, b'foo', None)

    fi2 = pycdlib.udf.UDFFileIdentifierDescriptor()
    fi2.new(False, False, b'hoo', None)

    assert(fi < fi2)

def test_file_ident_lt_both_parent():
    fi = pycdlib.udf.UDFFileIdentifierDescriptor()
    fi.new(False, True, b'foo', None)

    fi2 = pycdlib.udf.UDFFileIdentifierDescriptor()
    fi2.new(False, True, b'hoo', None)

    assert(not(fi < fi2))

def test_file_ident_lt_one_parent():
    fi = pycdlib.udf.UDFFileIdentifierDescriptor()
    fi.new(False, True, b'foo', None)

    fi2 = pycdlib.udf.UDFFileIdentifierDescriptor()
    fi2.new(False, False, b'hoo', None)

    assert(fi < fi2)

def test_file_ident_lt_other_parent():
    fi = pycdlib.udf.UDFFileIdentifierDescriptor()
    fi.new(False, False, b'foo', None)

    fi2 = pycdlib.udf.UDFFileIdentifierDescriptor()
    fi2.new(False, True, b'hoo', None)

    assert(not(fi < fi2))

def test_file_ident_equals():
    fi = pycdlib.udf.UDFFileIdentifierDescriptor()
    fi.new(False, False, b'foo', None)

    fi2 = pycdlib.udf.UDFFileIdentifierDescriptor()
    fi2.new(False, False, b'foo', None)

    assert(fi == fi2)

def test_file_ident_equals_both_parent():
    fi = pycdlib.udf.UDFFileIdentifierDescriptor()
    fi.new(False, True, b'foo', None)

    fi2 = pycdlib.udf.UDFFileIdentifierDescriptor()
    fi2.new(False, True, b'foo', None)

    assert(fi == fi2)

def test_file_ident_equals_both_parent():
    fi = pycdlib.udf.UDFFileIdentifierDescriptor()
    fi.new(False, True, b'foo', None)

    fi2 = pycdlib.udf.UDFFileIdentifierDescriptor()
    fi2.new(False, True, b'foo', None)

    assert(fi == fi2)

def test_file_ident_equals_other_parent():
    fi = pycdlib.udf.UDFFileIdentifierDescriptor()
    fi.new(False, False, b'foo', None)

    fi2 = pycdlib.udf.UDFFileIdentifierDescriptor()
    fi2.new(False, True, b'foo', None)

    assert(fi != fi2)

def test_file_ident_equals_other_parent():
    fi = pycdlib.udf.UDFFileIdentifierDescriptor()
    fi.new(False, True, b'foo', None)

    fi2 = pycdlib.udf.UDFFileIdentifierDescriptor()
    fi2.new(False, False, b'bar', None)

    assert(fi != fi2)

def test_file_ident_eq_not_same_object_type():
    fi = pycdlib.udf.UDFFileIdentifierDescriptor()
    fi.new(False, True, b'foo', None)

    not_an_fi = object()

    assert(fi != not_an_fi)

def test_file_ident_eq_not_other_parent():
    fi = pycdlib.udf.UDFFileIdentifierDescriptor()
    # isdir, isparent, name, parent
    fi.new(False, False, b'foo', None)

    fi2 = pycdlib.udf.UDFFileIdentifierDescriptor()
    fi2.new(False, True, b'bar', None)

    assert(fi != fi2)

# Space Bitmap
def test_space_bitmap_parse_initialized_twice():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    bitmap = pycdlib.udf.UDFSpaceBitmapDescriptor()
    bitmap.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*24, 0, tag)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        bitmap.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*24, 0, tag)
    assert(str(excinfo.value) == 'UDF Space Bitmap Descriptor already initialized')

def test_space_bitmap_parse():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    bitmap = pycdlib.udf.UDFSpaceBitmapDescriptor()
    bitmap.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*24, 0, tag)
    assert(bitmap.num_bits == 0)
    assert(bitmap.num_bytes == 0)

def test_space_bitmap_record_not_initialized():
    bitmap = pycdlib.udf.UDFSpaceBitmapDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        bitmap.record()
    assert(str(excinfo.value) == 'UDF Space Bitmap Descriptor not initialized')

def test_space_bitmap_record():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    bitmap = pycdlib.udf.UDFSpaceBitmapDescriptor()
    bitmap.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*24, 0, tag)

    assert(bitmap.record() == b'\x00\x00\x02\x00\x22\x00\x00\x00\x00\x00\x20' + b'\x00'*37)

def test_space_bitmap_new_initialized_twice():
    bitmap = pycdlib.udf.UDFSpaceBitmapDescriptor()
    bitmap.new()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        bitmap.new()
    assert(str(excinfo.value) == 'UDF Space Bitmap Descriptor already initialized')

def test_space_bitmap_new():
    bitmap = pycdlib.udf.UDFSpaceBitmapDescriptor()
    bitmap.new()
    assert(bitmap.num_bits == 0)
    assert(bitmap.num_bytes == 0)

def test_space_bitmap_extent_location_not_initialized():
    bitmap = pycdlib.udf.UDFSpaceBitmapDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        bitmap.extent_location()
    assert(str(excinfo.value) == 'UDF Space Bitmap Descriptor not initialized')

def test_space_bitmap_extent_location_parse():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    bitmap = pycdlib.udf.UDFSpaceBitmapDescriptor()
    bitmap.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*24, 0, tag)
    assert(bitmap.extent_location() == 0)

def test_space_bitmap_extent_location_new():
    bitmap = pycdlib.udf.UDFSpaceBitmapDescriptor()
    bitmap.new()
    bitmap.set_extent_location(0)
    assert(bitmap.extent_location() == 0)

def test_space_bitmap_set_extent_location_not_initialized():
    bitmap = pycdlib.udf.UDFSpaceBitmapDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        bitmap.set_extent_location(0)
    assert(str(excinfo.value) == 'UDF Space Bitmap Descriptor not initialized')

def test_space_bitmap_set_extent_location():
    bitmap = pycdlib.udf.UDFSpaceBitmapDescriptor()
    bitmap.new()
    bitmap.set_extent_location(1)
    assert(bitmap.extent_location() == 1)

# Allocation Extent
def test_alloc_extent_parse_initialized_twice():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    alloc = pycdlib.udf.UDFAllocationExtentDescriptor()
    alloc.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00', 0, tag)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        alloc.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00', 0, tag)
    assert(str(excinfo.value) == 'UDF Allocation Extent Descriptor already initialized')

def test_alloc_extent_parse():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    alloc = pycdlib.udf.UDFAllocationExtentDescriptor()
    alloc.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00', 0, tag)
    assert(alloc.prev_allocation_extent_loc == 0)
    assert(alloc.len_allocation_descs == 0)

def test_alloc_extent_record_not_initialized():
    alloc = pycdlib.udf.UDFAllocationExtentDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        alloc.record()
    assert(str(excinfo.value) == 'UDF Allocation Extent Descriptor not initialized')

def test_alloc_extent_record():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    alloc = pycdlib.udf.UDFAllocationExtentDescriptor()
    alloc.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00', 0, tag)
    assert(alloc.record() == b'\x00\x00\x02\x00\x0a\x00\x00\x00\x00\x00\x08' + b'\x00'*13)

def test_alloc_extent_new_initialized_twice():
    alloc = pycdlib.udf.UDFAllocationExtentDescriptor()
    alloc.new()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        alloc.new()
    assert(str(excinfo.value) == 'UDF Allocation Extent Descriptor already initialized')

def test_alloc_extent_new():
    alloc = pycdlib.udf.UDFAllocationExtentDescriptor()
    alloc.new()
    assert(alloc.prev_allocation_extent_loc == 0)
    assert(alloc.len_allocation_descs == 0)

def test_alloc_extent_extent_location_not_initialized():
    alloc = pycdlib.udf.UDFAllocationExtentDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        alloc.extent_location()
    assert(str(excinfo.value) == 'UDF Allocation Extent Descriptor not initialized')

def test_alloc_extent_extent_location_parse():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    alloc = pycdlib.udf.UDFAllocationExtentDescriptor()
    alloc.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00', 0, tag)

    assert(alloc.extent_location() == 0)

def test_alloc_extent_extent_location_new():
    alloc = pycdlib.udf.UDFAllocationExtentDescriptor()
    alloc.new()
    alloc.set_extent_location(0)
    assert(alloc.extent_location() == 0)

def test_alloc_extent_set_extent_location_not_initialized():
    alloc = pycdlib.udf.UDFAllocationExtentDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        alloc.set_extent_location(0)
    assert(str(excinfo.value) == 'UDF Allocation Extent Descriptor not initialized')

def test_alloc_extent_set_extent_location():
    alloc = pycdlib.udf.UDFAllocationExtentDescriptor()
    alloc.new()
    alloc.set_extent_location(1)
    assert(alloc.extent_location() == 1)

# Indirect Entry
def test_indirect_parse_initialized_twice():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    indirect = pycdlib.udf.UDFIndirectEntry()
    indirect.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*16, tag)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        indirect.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*16, tag)
    assert(str(excinfo.value) == 'UDF Indirect Entry already initialized')

def test_indirect_parse():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    indirect = pycdlib.udf.UDFIndirectEntry()
    indirect.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*16, tag)
    assert(indirect.desc_tag.tag_ident == 0)

def test_indirect_record_not_initialized():
    indirect = pycdlib.udf.UDFIndirectEntry()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        indirect.record()
    assert(str(excinfo.value) == 'UDF Indirect Entry not initialized')

def test_indirect_record():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    indirect = pycdlib.udf.UDFIndirectEntry()
    indirect.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*16, tag)
    assert(indirect.record() == b'\x00\x00\x02\x00\xd4\x00\x00\x00\x39\x75\x24' + b'\x00'*5 + b'\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*16)

def test_indirect_new_initialized_twice():
    indirect = pycdlib.udf.UDFIndirectEntry()
    indirect.new('dir')
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        indirect.new('dir')
    assert(str(excinfo.value) == 'UDF Indirect Entry already initialized')

def test_indirect_new():
    indirect = pycdlib.udf.UDFIndirectEntry()
    indirect.new('dir')
    assert(indirect.desc_tag.tag_ident == 259)

# Terminating
def test_terminal_entry_parse_initialized_twice():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    term = pycdlib.udf.UDFTerminalEntry()
    term.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00', tag)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        term.parse(b'\x00'*16 + b'\x00'*20, tag)
    assert(str(excinfo.value) == 'UDF Terminal Entry already initialized')

def test_terminal_entry_parse():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    term = pycdlib.udf.UDFTerminalEntry()
    term.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00', tag)
    assert(term.desc_tag.tag_ident == 0)

def test_terminal_entry_record_not_initialized():
    term = pycdlib.udf.UDFTerminalEntry()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        term.record()
    assert(str(excinfo.value) == 'UDF Terminal Entry not initialized')

def test_terminal_entry_record():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    term = pycdlib.udf.UDFTerminalEntry()
    term.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00', tag)
    assert(term.record() == b'\x00\x00\x02\x00\x68\x00\x00\x00\xd2\x80\x14\x00\x00\x00\x00\x00' + b'\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00')

def test_terminal_entry_new_initialized_twice():
    term = pycdlib.udf.UDFTerminalEntry()
    term.new('dir')
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        term.new('dir')
    assert(str(excinfo.value) == 'UDF Terminal Entry already initialized')

def test_terminal_entry_new():
    term = pycdlib.udf.UDFTerminalEntry()
    term.new('dir')
    assert(term.desc_tag.tag_ident == 260)

# Extended Attribute Header
def test_ext_header_parse_initialized_twice():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    ext = pycdlib.udf.UDFExtendedAttributeHeaderDescriptor()
    ext.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00', tag)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ext.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00', tag)
    assert(str(excinfo.value) == 'UDF Extended Attribute Header Descriptor already initialized')

def test_ext_header_parse():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    ext = pycdlib.udf.UDFExtendedAttributeHeaderDescriptor()
    ext.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00', tag)
    assert(ext.desc_tag.tag_ident == 0)

def test_ext_header_record_not_initialized():
    ext = pycdlib.udf.UDFExtendedAttributeHeaderDescriptor()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ext.record()
    assert(str(excinfo.value) == 'UDF Extended Attribute Header Descriptor not initialized')

def test_ext_header_record():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    ext = pycdlib.udf.UDFExtendedAttributeHeaderDescriptor()
    ext.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00', tag)
    assert(ext.record() == b'\x00\x00\x02\x00\x0a\x00\x00\x00\x00\x00\x08\x00\x00\x00\x00\x00' + b'\x00'*8)

def test_ext_header_new_initialized_twice():
    ext = pycdlib.udf.UDFExtendedAttributeHeaderDescriptor()
    ext.new()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ext.new()
    assert(str(excinfo.value) == 'UDF Extended Attribute Header Descriptor already initialized')

def test_ext_header_new():
    ext = pycdlib.udf.UDFExtendedAttributeHeaderDescriptor()
    ext.new()
    assert(ext.desc_tag.tag_ident == 262)

# Unallocated Space
def test_unalloc_space_parse_initialized_twice():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    unalloc = pycdlib.udf.UDFUnallocatedSpaceEntry()
    unalloc.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x00\x00', 0, tag)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        unalloc.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x00\x00', 0, tag)
    assert(str(excinfo.value) == 'UDF Unallocated Space Entry already initialized')

def test_unalloc_space_parse():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    unalloc = pycdlib.udf.UDFUnallocatedSpaceEntry()
    unalloc.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x00\x00', 0, tag)
    assert(unalloc.desc_tag.tag_ident == 0)

def test_unalloc_space_record_not_initialized():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    unalloc = pycdlib.udf.UDFUnallocatedSpaceEntry()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        unalloc.record()
    assert(str(excinfo.value) == 'UDF Unallocated Space Entry not initialized')

def test_unalloc_space_record():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    unalloc = pycdlib.udf.UDFUnallocatedSpaceEntry()
    unalloc.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00', 0, tag)
    assert(unalloc.record() == b'\x00\x00\x02\x00\xa3\x00\x00\x00\xf8\x89 \x00\x00\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00')

def test_unalloc_space_new_initialized_twice():
    unalloc = pycdlib.udf.UDFUnallocatedSpaceEntry()
    unalloc.new('dir')
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        unalloc.new('dir')
    assert(str(excinfo.value) == 'UDF Unallocated Space Entry already initialized')

def test_unalloc_space_new():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    unalloc = pycdlib.udf.UDFUnallocatedSpaceEntry()
    unalloc.new('dir')
    assert(unalloc.desc_tag.tag_ident == 263)

# Partition Integrity
def test_part_integrity_parse_initialized_twice():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    part = pycdlib.udf.UDFPartitionIntegrityEntry()
    part.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00' + b'\x00'*175 + b'\x00'*32 + b'\x00'*256, tag)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        part.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00' + b'\x00'*175 + b'\x00'*32 + b'\x00'*256, tag)
    assert(str(excinfo.value) == 'UDF Partition Integrity Entry already initialized')

def test_part_integrity_parse():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    part = pycdlib.udf.UDFPartitionIntegrityEntry()
    part.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00' + b'\x00'*175 + b'\x00'*32 + b'\x00'*256, tag)
    assert(part.desc_tag.tag_ident == 0)

def test_part_integrity_record_not_initialized():
    part = pycdlib.udf.UDFPartitionIntegrityEntry()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        part.record()
    assert(str(excinfo.value) == 'UDF Partition Integrity Entry not initialized')

def test_part_integrity_record():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    part = pycdlib.udf.UDFPartitionIntegrityEntry()
    part.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00' + b'\x00'*175 + b'\x00'*32 + b'\x00'*256, tag)
    assert(part.record() == b'\x00\x00\x02\x00\x00\x00\x00\x00\xbe\x4f\xf0\x01\x00\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00')

def test_part_integrity_new_initialized_twice():
    part = pycdlib.udf.UDFPartitionIntegrityEntry()
    part.new('dir')
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        part.new('dir')
    assert(str(excinfo.value) == 'UDF Partition Integrity Entry already initialized')

def test_part_integrity_new():
    part = pycdlib.udf.UDFPartitionIntegrityEntry()
    part.new('dir')
    assert(part.desc_tag.tag_ident == 265)

# Extended File
def test_extended_file_parse_initialized_twice():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    ef = pycdlib.udf.UDFExtendedFileEntry()
    ef.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x00\x00' + b'\x00'*4 + b'\x00'*16 + b'\x00'*16 + b'\x00'*32 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00', 0, tag)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ef.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x00\x00' + b'\x00'*4 + b'\x00'*16 + b'\x00'*16 + b'\x00'*32 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00', 0, tag)
    assert(str(excinfo.value) == 'UDF Extended File Entry already initialized')

def test_extended_file_parse():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    ef = pycdlib.udf.UDFExtendedFileEntry()
    ef.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x00\x00' + b'\x00'*4 + b'\x00'*16 + b'\x00'*16 + b'\x00'*32 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00', 0, tag)
    assert(ef.desc_tag.tag_ident == 0)

def test_extended_file_record_not_initialized():
    ef = pycdlib.udf.UDFExtendedFileEntry()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ef.record()
    assert(str(excinfo.value) == 'UDF Extended File Entry not initialized')

def test_extended_file_record():
    tag = pycdlib.udf.UDFTag()
    tag.new(0, 0)

    ef = pycdlib.udf.UDFExtendedFileEntry()
    ef.parse(b'\x00'*16 + b'\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00' + b'\x00\x00\x00\x00' + b'\x00'*4 + b'\x00'*16 + b'\x00'*16 + b'\x00'*32 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00' + b'\x00\x00\x00\x00\x00\x00\x00\x00', 0, tag)
    assert(ef.record() == b'\x00\x00\x02\x00\xf1\x00\x00\x00\x52\xcd\xd0\x00\x00\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00')

def test_extended_file_new_initialized_twice():
    ef = pycdlib.udf.UDFExtendedFileEntry()
    ef.new('dir', 0, 2048)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ef.new('dir', 0, 2048)
    assert(str(excinfo.value) == 'UDF Extended File Entry already initialized')

def test_extended_file_new_dir():
    ef = pycdlib.udf.UDFExtendedFileEntry()
    ef.new('dir', 0, 2048)
    assert(ef.desc_tag.tag_ident == 266)

def test_extended_file_new_file():
    ef = pycdlib.udf.UDFExtendedFileEntry()
    ef.new('file', 5, 2048)
    assert(ef.desc_tag.tag_ident == 266)

# parse_allocation_descriptors
def test_parse_allocation_descriptors_long():
    alloc_descs = pycdlib.udf._parse_allocation_descriptors(1, b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00', 16, 0, 0)
    assert(len(alloc_descs) == 1)
    assert(isinstance(alloc_descs[0], pycdlib.udf.UDFLongAD))

def test_parse_allocation_descriptors_extended():
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        alloc_descs = pycdlib.udf._parse_allocation_descriptors(2, b'', 0, 0, 0)
    assert(str(excinfo.value) == 'UDF Allocation Descriptor of type 2 (Extended) not yet supported')

def test_parse_allocation_descriptors_inline():
    alloc_descs = pycdlib.udf._parse_allocation_descriptors(3, b'', 0, 0, 0)
    assert(len(alloc_descs) == 1)
    assert(isinstance(alloc_descs[0], pycdlib.udf.UDFInlineAD))

def test_parse_allocation_descriptors_invalid():
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        alloc_descs = pycdlib.udf._parse_allocation_descriptors(4, b'', 0, 0, 0)
    assert(str(excinfo.value) == 'UDF Allocation Descriptor type invalid')

# UDFDescriptorSequence
def test_descriptor_sequence_append_to_list():
    seq = pycdlib.udf.UDFDescriptorSequence()

    pvd1 = pycdlib.udf.UDFPrimaryVolumeDescriptor()
    pvd1.new()
    seq.append_to_list('pvds', pvd1)

    pvd2 = pycdlib.udf.UDFPrimaryVolumeDescriptor()
    pvd2.new()
    pvd2.desc_num = 1
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        seq.append_to_list('pvds', pvd2)
    assert(str(excinfo.value) == 'Descriptors with same sequence number do not have the same contents')
