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

import pycdlib.inode
import pycdlib.pycdlibexception

def test_inode_new_initialized_twice():
    ino = pycdlib.inode.Inode()
    ino.new(0, '', False, 0)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ino.new(0, '', False, 0)
    assert(str(excinfo.value) == 'Inode is already initialized')

def test_inode_parse_initialized_twice():
    ino = pycdlib.inode.Inode()
    ino.parse(0, 0, None, 0)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ino.parse(0, 0, None, 0)
    assert(str(excinfo.value) == 'Inode is already initialized')

def test_inode_extent_location_not_initialized():
    ino = pycdlib.inode.Inode()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ino.extent_location()
    assert(str(excinfo.value) == 'Inode is not initialized')

def test_inode_set_extent_location_not_initialized():
    ino = pycdlib.inode.Inode()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ino.set_extent_location(0)
    assert(str(excinfo.value) == 'Inode is not initialized')

def test_inode_get_data_length_not_initialized():
    ino = pycdlib.inode.Inode()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ino.get_data_length()
    assert(str(excinfo.value) == 'Inode is not initialized')

def test_inode_add_boot_info_table_not_initialized():
    ino = pycdlib.inode.Inode()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ino.add_boot_info_table(None)
    assert(str(excinfo.value) == 'Inode is not initialized')

def test_inode_update_fp_not_initialized():
    ino = pycdlib.inode.Inode()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        ino.update_fp(None, 0)
    assert(str(excinfo.value) == 'Inode is not initialized')
