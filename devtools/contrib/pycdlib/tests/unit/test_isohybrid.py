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

import pycdlib.isohybrid

# APMPartHeader
def test_apm_part_header_parse_initialized_twice():
    apm_part = pycdlib.isohybrid.APMPartHeader()
    apm_part.parse(b'\x50\x4d\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*32 + b'\x00'*32 + b'\x00\x00\x00\x00'*10 + b'\x00'*16 + b'\x00\x00\x00\x00' + b'\x00'*372)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        apm_part.parse(b'\x50\x4d\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*32 + b'\x00'*32 + b'\x00\x00\x00\x00'*10 + b'\x00'*16 + b'\x00\x00\x00\x00' + b'\x00'*372)
    assert(str(excinfo.value) == 'This APMPartHeader object is already initialized')

def test_apm_part_header_parse_not_magic():
    apm_part = pycdlib.isohybrid.APMPartHeader()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        apm_part.parse(b'\x40\x4d\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*32 + b'\x00'*32 + b'\x00\x00\x00\x00'*10 + b'\x00'*16 + b'\x00\x00\x00\x00' + b'\x00'*372)
    assert(str(excinfo.value) == 'Invalid APM signature')

def test_apm_part_header_new_initialized_twice():
    apm_part = pycdlib.isohybrid.APMPartHeader()
    apm_part.new('foo', 'type', 0)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        apm_part.new('foo', 'type', 0)
    assert(str(excinfo.value) == 'This APMPartHeader object is already initialized')

def test_apm_part_header_record_not_initialized():
    apm_part = pycdlib.isohybrid.APMPartHeader()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        apm_part.record()
    assert(str(excinfo.value) == 'This APMPartHeader object is not initialized')

# GPTPartHeader
def test_gpt_part_header_parse_initialized_twice():
    gpt_part = pycdlib.isohybrid.GPTPartHeader()
    gpt_part.parse(b'\xa2\xa0\xd0\xeb\xe5\xb9\x33\x44\x87\xc0\x68\xb6\xb7\x26\x99\xc7' + b'\x00'*16 + b'\x00'*16 + b'\x00'*8 + b'\x00'*72)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        gpt_part.parse(b'\xa2\xa0\xd0\xeb\xe5\xb9\x33\x44\x87\xc0\x68\xb6\xb7\x26\x99\xc7' + b'\x00'*16 + b'\x00'*16 + b'\x00'*8 + b'\x00'*72)
    assert(str(excinfo.value) == 'This GPTPartHeader object is already initialized')

def test_gpt_part_header_parse_invalid_guid():
    gpt_part = pycdlib.isohybrid.GPTPartHeader()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        gpt_part.parse(b'\x00'*16 + b'\x00'*16 + b'\x00'*16 + b'\x00'*8 + b'\x00'*72)
    assert(str(excinfo.value) == 'Invalid Partition Type UUID')

def test_gpt_part_header_new_initialized_twice():
    gpt_part = pycdlib.isohybrid.GPTPartHeader()
    gpt_part.new(True, 'foo')

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        gpt_part.new(True, 'foo')
    assert(str(excinfo.value) == 'This GPTPartHeader object is already initialized')

def test_gpt_part_header_record_not_initialized():
    gpt_part = pycdlib.isohybrid.GPTPartHeader()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        gpt_part.record()
    assert(str(excinfo.value) == 'This GPTPartHeader object is not initialized')

# GPTHeader
def test_gpt_header_parse_initialized_twice():
    gpt_header = pycdlib.isohybrid.GPTHeader()
    gpt_header.parse(b'\x45\x46\x49\x20\x50\x41\x52\x54' + b'\x00\x00\x01\x00' + b'\x5c\x00\x00\x00' + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*420)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        gpt_header.parse(b'\x45\x46\x49\x20\x50\x41\x52\x54' + b'\x00\x00\x01\x00' + b'\x5c\x00\x00\x00' + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*420)
    assert(str(excinfo.value) == 'This GPTHeader object is already initialized')

def test_gpt_header_parse_bad_gpt_sig():
    gpt_header = pycdlib.isohybrid.GPTHeader()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        gpt_header.parse(b'\x44\x46\x49\x20\x50\x41\x52\x54' + b'\x00\x00\x01\x00' + b'\x5c\x00\x00\x00' + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*420)
    assert(str(excinfo.value) == 'Failed to find GPT signature while parsing GPT Header')

def test_gpt_header_parse_bad_gpt_rev():
    gpt_header = pycdlib.isohybrid.GPTHeader()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        gpt_header.parse(b'\x45\x46\x49\x20\x50\x41\x52\x54' + b'\x01\x00\x01\x00' + b'\x5c\x00\x00\x00' + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*420)
    assert(str(excinfo.value) == 'Failed to find GPT revision while parsing GPT Header')

def test_gpt_header_parse_bad_header_size():
    gpt_header = pycdlib.isohybrid.GPTHeader()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        gpt_header.parse(b'\x45\x46\x49\x20\x50\x41\x52\x54' + b'\x00\x00\x01\x00' + b'\x5b\x00\x00\x00' + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*420)
    assert(str(excinfo.value) == 'Invalid GPT Header size while parsing GPT Header')

def test_gpt_header_new_initialized_twice():
    gpt_header = pycdlib.isohybrid.GPTHeader()
    gpt_header.new(False)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        gpt_header.new(False)
    assert(str(excinfo.value) == 'This GPTHeader object is already initialized')

def test_gpt_header_set_lbas_not_initialized():
    gpt_header = pycdlib.isohybrid.GPTHeader()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        gpt_header.set_lbas(0, 0)
    assert(str(excinfo.value) == 'This GPTHeader object is not initialized')

def test_gpt_header_set_last_usable_lba_not_initialized():
    gpt_header = pycdlib.isohybrid.GPTHeader()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        gpt_header.set_last_usable_lba(0)
    assert(str(excinfo.value) == 'This GPTHeader object is not initialized')

def test_gpt_header_record_not_initialized():
    gpt_header = pycdlib.isohybrid.GPTHeader()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        gpt_header.record(0)
    assert(str(excinfo.value) == 'This GPTHeader object is not initialized')

# GPT
def test_gpt_parse_primary_initialized_twice():
    gpt = pycdlib.isohybrid.GPT(True)
    gpt.parse_primary(b'\x00'*512 + b'\x45\x46\x49\x20\x50\x41\x52\x54' + b'\x00\x00\x01\x00' + b'\x5c\x00\x00\x00' + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*420, False)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        gpt.parse_primary(b'\x00'*512 + b'\x45\x46\x49\x20\x50\x41\x52\x54' + b'\x00\x00\x01\x00' + b'\x5c\x00\x00\x00' + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*420, False)
    assert(str(excinfo.value) == 'This GPT object is already initialized')

def test_gpt_parse_primary_with_secondary_GPT():
    gpt = pycdlib.isohybrid.GPT(False)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        gpt.parse_primary(b'\x00'*512 + b'\x45\x46\x49\x20\x50\x41\x52\x54' + b'\x00\x00\x01\x00' + b'\x5c\x00\x00\x00' + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*420, False)
    assert(str(excinfo.value) == 'Cannot parse primary with a secondary GPT')

def test_gpt_parse_secondary_header_initialized_twice():
    gpt = pycdlib.isohybrid.GPT(False)
    gpt.parse_secondary_header(b'\x45\x46\x49\x20\x50\x41\x52\x54' + b'\x00\x00\x01\x00' + b'\x5c\x00\x00\x00' + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*420)
    gpt.parse_secondary_partitions(b'')

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        gpt.parse_secondary_header(b'\x45\x46\x49\x20\x50\x41\x52\x54' + b'\x00\x00\x01\x00' + b'\x5c\x00\x00\x00' + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*420)
    assert(str(excinfo.value) == 'This GPT object is already initialized')

def test_gpt_parse_secondary_header_with_primary_gpt():
    gpt = pycdlib.isohybrid.GPT(True)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        gpt.parse_secondary_header(b'\x45\x46\x49\x20\x50\x41\x52\x54' + b'\x00\x00\x01\x00' + b'\x5c\x00\x00\x00' + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*420)
    assert(str(excinfo.value) == 'Cannot parse secondary header with a primary GPT')

def test_gpt_parse_secondary_header_twice():
    gpt = pycdlib.isohybrid.GPT(False)
    gpt.parse_secondary_header(b'\x45\x46\x49\x20\x50\x41\x52\x54' + b'\x00\x00\x01\x00' + b'\x5c\x00\x00\x00' + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*16 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*420)
    gpt.parse_secondary_partitions(b'')

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        gpt.parse_secondary_partitions(b'')
    assert(str(excinfo.value) == 'This GPT object is already initialized')

def test_gpt_new_initialized_twice():
    gpt = pycdlib.isohybrid.GPT(True)
    gpt.new(False)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        gpt.new(False)
    assert(str(excinfo.value) == 'This GPT object is already initialized')

def test_gpt_record_not_initialized():
    gpt = pycdlib.isohybrid.GPT(True)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        gpt.record()
    assert(str(excinfo.value) == 'This GPT object is not initialized')

# IsoHybrid
def test_isohybrid_parse_initialized_twice():
    isohybrid = pycdlib.isohybrid.IsoHybrid()
    isohybrid.parse(b'\x33\xed' + b'\x90' * 30 + b'\x00'*400 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x80' + b'\x00'*63 + b'\x55\xaa')

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        isohybrid.parse(b'\x33\xed' + b'\x90' * 30 + b'\x00'*400 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x80' + b'\x00'*63 + b'\x55\xaa')
    assert(str(excinfo.value) == 'This IsoHybrid object is already initialized')

def test_isohybrid_parse_invalid_size():
    isohybrid = pycdlib.isohybrid.IsoHybrid()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        isohybrid.parse(b'')
    assert(str(excinfo.value) == 'Invalid IsoHybrid MBR passed')

def test_isohybrid_parse_invalid_unused1():
    isohybrid = pycdlib.isohybrid.IsoHybrid()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        isohybrid.parse(b'\x33\xed' + b'\x90' * 30 + b'\x00'*400 + b'\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x80' + b'\x00'*63 + b'\x55\xaa')
    assert(str(excinfo.value) == 'Invalid IsoHybrid unused1')

def test_isohybrid_parse_invalid_unused2():
    isohybrid = pycdlib.isohybrid.IsoHybrid()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        isohybrid.parse(b'\x33\xed' + b'\x90' * 30 + b'\x00'*400 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00' + b'\x80' + b'\x00'*63 + b'\x55\xaa')
    assert(str(excinfo.value) == 'Invalid IsoHybrid unused2')

def test_isohybrid_parse_no_part_entry():
    isohybrid = pycdlib.isohybrid.IsoHybrid()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        isohybrid.parse(b'\x33\xed' + b'\x90' * 30 + b'\x00'*400 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00' + b'\x00'*63 + b'\x55\xaa')
    assert(str(excinfo.value) == 'No valid partition found in IsoHybrid!')

def test_isohybrid_parse_bad_tail():
    isohybrid = pycdlib.isohybrid.IsoHybrid()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        isohybrid.parse(b'\x33\xed' + b'\x90' * 30 + b'\x00'*400 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x80' + b'\x00'*63 + b'\xaa\xaa')
    assert(str(excinfo.value) == 'Invalid tail on isohybrid section')

def test_isohybrid_parse_secondary_gpt_header_not_initialized():
    isohybrid = pycdlib.isohybrid.IsoHybrid()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        isohybrid.parse_secondary_gpt_header(b'')
    assert(str(excinfo.value) == 'This IsoHybrid object is not initialized')

def test_isohybrid_parse_secondary_gpt_partitions_not_initialized():
    isohybrid = pycdlib.isohybrid.IsoHybrid()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        isohybrid.parse_secondary_gpt_partitions(b'')
    assert(str(excinfo.value) == 'This IsoHybrid object is not initialized')

def test_isohybrid_parse_clamp_geometry_sectors():
    isohybrid = pycdlib.isohybrid.IsoHybrid()

    isohybrid.parse(b'\x33\xed' + b'\x90' * 30 + b'\x00'*400 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x80' + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x40\x00\x00\x00' + b'\x00'*47 + b'\x55\xaa')
    assert(isohybrid.geometry_sectors == 63)

def test_isohybrid_new_initialized_twice():
    isohybrid = pycdlib.isohybrid.IsoHybrid()
    isohybrid.new(False, False, 0, 0, 0, 1, 1, 0)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        isohybrid.new(False, False, 0, 0, 0, 1, 1, 0)
    assert(str(excinfo.value) == 'This IsoHybrid object is already initialized')

def test_isohybrid_new_bad_sectors():
    isohybrid = pycdlib.isohybrid.IsoHybrid()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        isohybrid.new(False, False, 0, 0, 0, 0, 1, 0)
    assert(str(excinfo.value) == 'Geometry sectors can only be between 1 and 63, inclusive')

def test_isohybrid_new_bad_heads():
    isohybrid = pycdlib.isohybrid.IsoHybrid()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        isohybrid.new(False, False, 0, 0, 0, 1, 0, 0)
    assert(str(excinfo.value) == 'Geometry heads can only be between 1 and 256, inclusive')

def test_isohybrid_new_bad_mac_part_type():
    isohybrid = pycdlib.isohybrid.IsoHybrid()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        isohybrid.new(False, True, 0, 0, 0, 1, 1, 1)
    assert(str(excinfo.value) == 'When generating for Mac, partition type must be 0')

def test_isohybrid_record_not_initialized():
    isohybrid = pycdlib.isohybrid.IsoHybrid()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        isohybrid.record(1)
    assert(str(excinfo.value) == 'This IsoHybrid object is not initialized')

def test_isohybrid_record_padding_not_initialized():
    isohybrid = pycdlib.isohybrid.IsoHybrid()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        isohybrid.record_padding(1)
    assert(str(excinfo.value) == 'This IsoHybrid object is not initialized')

def test_isohybrid_update_rba_not_initialized():
    isohybrid = pycdlib.isohybrid.IsoHybrid()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        isohybrid.update_rba(1)
    assert(str(excinfo.value) == 'This IsoHybrid object is not initialized')

def test_isohybrid_update_efi_not_initialized():
    isohybrid = pycdlib.isohybrid.IsoHybrid()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        isohybrid.update_efi(1, 1, 1)
    assert(str(excinfo.value) == 'This IsoHybrid object is not initialized')

def test_isohybrid_update_efi_not_efi():
    isohybrid = pycdlib.isohybrid.IsoHybrid()
    isohybrid.new(False, False, 0, 0, 0, 1, 1, 0)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        isohybrid.update_efi(1, 1, 1)
    assert(str(excinfo.value) == 'Attempted to set EFI lba on a non-EFI ISO')

def test_isohybrid_update_mac_not_initialized():
    isohybrid = pycdlib.isohybrid.IsoHybrid()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        isohybrid.update_mac(1, 1)
    assert(str(excinfo.value) == 'This IsoHybrid object is not initialized')

def test_isohybrid_update_mac_not_mac():
    isohybrid = pycdlib.isohybrid.IsoHybrid()
    isohybrid.new(False, False, 0, 0, 0, 1, 1, 0)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        isohybrid.update_mac(1, 1)
    assert(str(excinfo.value) == 'Attempted to set Mac lba on a non-Mac ISO')
