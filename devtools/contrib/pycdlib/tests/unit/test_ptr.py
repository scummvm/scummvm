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

import pycdlib.path_table_record

def test_path_table_record_parse_initialized_twice():
    ptr = pycdlib.path_table_record.PathTableRecord()
    ptr.parse(b'\x00\x00\x00\x00\x00\x00\x00\x00')

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ptr.parse(b'\x00\x00\x00\x00\x00\x00\x00\x00')
    assert(str(excinfo.value) == 'Path Table Record already initialized')

def test_path_table_record_record_little_endian_not_initialized():
    ptr = pycdlib.path_table_record.PathTableRecord()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ptr.record_little_endian()
    assert(str(excinfo.value) == 'Path Table Record not initialized')

def test_path_table_record_record_big_endian_not_initialized():
    ptr = pycdlib.path_table_record.PathTableRecord()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ptr.record_big_endian()
    assert(str(excinfo.value) == 'Path Table Record not initialized')

def test_path_table_record_new_root_initialized_twice():
    ptr = pycdlib.path_table_record.PathTableRecord()
    ptr.new_root()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ptr.new_root()
    assert(str(excinfo.value) == 'Path Table Record already initialized')

def test_path_table_record_new_dir_initialized_twice():
    ptr = pycdlib.path_table_record.PathTableRecord()
    ptr.new_dir(b'foo')

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ptr.new_dir(b'foo')
    assert(str(excinfo.value) == 'Path Table Record already initialized')

def test_path_table_record_update_extent_location_not_initialized():
    ptr = pycdlib.path_table_record.PathTableRecord()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ptr.update_extent_location(0)
    assert(str(excinfo.value) == 'Path Table Record not initialized')

def test_path_table_record_update_parent_directory_number_not_initialized():
    ptr = pycdlib.path_table_record.PathTableRecord()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ptr.update_parent_directory_number(0)
    assert(str(excinfo.value) == 'Path Table Record not initialized')

def test_path_table_record_equal_to_be_not_initialized():
    ptr = pycdlib.path_table_record.PathTableRecord()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ptr.equal_to_be(None)
    assert(str(excinfo.value) == 'Path Table Record not initialized')
