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

import pycdlib.dr

# XA
def test_xa_parse_initialized_twice():
    xa = pycdlib.dr.XARecord()
    xa.parse(b'\x00\x00\x00\x00\x00\x00\x58\x41\x00\x00\x00\x00\x00\x00', 1)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        xa.parse(b'\x00\x00\x00\x00\x00\x00\x58\x41\x00\x00\x00\x00\x00\x00', 1)
    assert(str(excinfo.value) == 'This XARecord is already initialized')

def test_xa_parse_bad_reserved():
    xa = pycdlib.dr.XARecord()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        xa.parse(b'\x00\x00\x00\x00\x00\x00\x58\x41\x00\x00\x00\x00\x00\x01', 1)
    assert(str(excinfo.value) == 'Unused fields should be 0')

def test_xa_parse_padding():
    xa = pycdlib.dr.XARecord()
    # Test out the case when there is a bit of padding at the front of the
    # XA Record, as there are for Windows 98 SE ISOs.
    xa.parse(b'\x00\x00\x00\x00' + b'\x00\x00\x00\x00\x00\x00\x58\x41\x00\x00\x00\x00\x00\x00', 4)
    assert(xa._initialized)
    assert(xa._pad_size == 4)

def test_xa_new_initialized_twice():
    xa = pycdlib.dr.XARecord()
    xa.new()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        xa.new()
    assert(str(excinfo.value) == 'This XARecord is already initialized')

def test_xa_record_not_initialized():
    xa = pycdlib.dr.XARecord()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        xa.record()
    assert(str(excinfo.value) == 'This XARecord is not initialized')


# DR
def test_dr_parse_initialized_twice():
    pvd = pycdlib.headervd.pvd_factory(b'', b'', 0, 0, 0, b'', b'', b'', b'', b'', b'', b'', 0.0, b'', False)
    dr = pycdlib.dr.DirectoryRecord()
    dr.parse(pvd, b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*7 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00', None)
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        dr.parse(pvd, b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*7 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00', None)
    assert(str(excinfo.value) == 'Directory Record already initialized')

def test_dr_bad_record_length():
    dr = pycdlib.dr.DirectoryRecord()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        dr.parse(None, b'\x00' * 256, None)
    assert(str(excinfo.value) == 'Directory record longer than 255 bytes!')

def test_dr_bad_extent_location():
    dr = pycdlib.dr.DirectoryRecord()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        dr.parse(None, b'\x00\x00\x01\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*7 + b'\x00\x00\x00\x00\x00\x00\x00\x00\x00', None)
    assert(str(excinfo.value) == 'Little-endian (1) and big-endian (2) extent location disagree')

def test_dr_bad_seqnum():
    dr = pycdlib.dr.DirectoryRecord()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        dr.parse(None, b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*7 + b'\x00\x00\x00\x01\x00\x00\x02\x00\x00', None)
    assert(str(excinfo.value) == 'Little-endian and big-endian seqnum disagree')

def test_dr_bad_rr_parent():
    pvd = pycdlib.headervd.pvd_factory(b'', b'', 0, 0, 0, b'', b'', b'', b'', b'', b'', b'', 0.0, b'', False)
    root_dr = pycdlib.dr.DirectoryRecord()
    root_dr.parse(pvd, b'\x22\x00\x17\x00\x00\x00\x00\x00\x00\x17\x00\x08\x00\x00\x00\x00\x08\x00\x78\x09\x0d\x0d\x07\x15\xf0\x02\x00\x00\x01\x00\x00\x01\x01\x00', None)

    dotdot_dr = pycdlib.dr.DirectoryRecord()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        dotdot_dr.parse(pvd, b'\x66\x00\x17\x00\x00\x00\x00\x00\x00\x17\x00\x08\x00\x00\x00\x00\x08\x00\x78\x09\x0d\x0d\x07\x15\xf0\x02\x00\x00\x01\x00\x00\x01\x01\x01\x52\x52\x05\x01\x81\x50\x58\x24\x01\x6d\x41\x00\x00\x00\x00\x41\x6d\x02\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x54\x46\x1a\x01\x0e\x78\x09\x0d\x0d\x07\x15\xf0\x78\x09\x0d\x0d\x07\x26\xf0\x78\x09\x0d\x0d\x07\x15\xf0\x00', root_dr)
    assert(str(excinfo.value) == 'Parent has no dot child')

def test_dr_rr_dot_no_rr():
    pvd = pycdlib.headervd.pvd_factory(b'', b'', 0, 0, 0, b'', b'', b'', b'', b'', b'', b'', 0.0, b'', False)
    root_dr = pycdlib.dr.DirectoryRecord()
    root_dr.parse(pvd, b'\x22\x00\x17\x00\x00\x00\x00\x00\x00\x17\x00\x08\x00\x00\x00\x00\x08\x00\x78\x09\x0d\x0d\x07\x15\xf0\x02\x00\x00\x01\x00\x00\x01\x01\x00', None)

    dot_dr = pycdlib.dr.DirectoryRecord()
    dot_dr.parse(pvd, b'\x22\x00\x17\x00\x00\x00\x00\x00\x00\x17\x00\x08\x00\x00\x00\x00\x08\x00x\x09\x0d\x0d\x07\x15\xf0\x02\x00\x00\x01\x00\x00\x01\x01\x00', root_dr)
    root_dr.track_child(dot_dr, 2048)

    dotdot_dr = pycdlib.dr.DirectoryRecord()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        dotdot_dr.parse(pvd, b'\x66\x00\x17\x00\x00\x00\x00\x00\x00\x17\x00\x08\x00\x00\x00\x00\x08\x00\x78\x09\x0d\x0d\x07\x15\xf0\x02\x00\x00\x01\x00\x00\x01\x01\x01\x52\x52\x05\x01\x81\x50\x58\x24\x01\x6d\x41\x00\x00\x00\x00\x41\x6d\x02\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x54\x46\x1a\x01\x0e\x78\x09\x0d\x0d\x07\x15\xf0\x78\x09\x0d\x0d\x07\x26\xf0\x78\x09\x0d\x0d\x07\x15\xf0\x00', root_dr)
    assert(str(excinfo.value) == 'Dot child does not have Rock Ridge; ISO is corrupt')

def test_dr_rr_dir_no_rr():
    pvd = pycdlib.headervd.pvd_factory(b'', b'', 0, 0, 0, b'', b'', b'', b'', b'', b'', b'', 0.0, b'', False)
    root_dr = pycdlib.dr.DirectoryRecord()
    root_dr.parse(pvd, b'\x22\x00\x17\x00\x00\x00\x00\x00\x00\x17\x00\x08\x00\x00\x00\x00\x08\x00\x78\x09\x0d\x0d\x07\x15\xf0\x02\x00\x00\x01\x00\x00\x01\x01\x00', None)

    dir1_dr = pycdlib.dr.DirectoryRecord()
    dir1_dr.parse(pvd, b'\x26\x00\x18\x00\x00\x00\x00\x00\x00\x18\x00\x08\x00\x00\x00\x00\x08\x00x\x09\x0d\x0d\x18\x06\xf0\x02\x00\x00\x01\x00\x00\x01\x04DIR1\x00', root_dr)

    foo_dr = pycdlib.dr.DirectoryRecord()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        foo_dr.parse(pvd, b'\x74\x00\x1a\x00\x00\x00\x00\x00\x00\x1a\x04\x00\x00\x00\x00\x00\x00\x04\x78\x09\x0d\x0d\x18\x06\xf0\x00\x00\x00\x01\x00\x00\x01\x06FOO.;1\x00RR\x05\x01\x89NM\x08\x01\x00fooPX$\x01$\x81\x00\x00\x00\x00\x81$\x01\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00TF\x1a\x01\x0ex\t\r\r\x18\x06\xf0x\x09\x0d\x0d\x18\x0a\xf0x\x09\x0d\x0d\x18\x06\xf0\x00', dir1_dr)
    assert(str(excinfo.value) == 'Parent does not have Rock Ridge; ISO is corrupt')

def test_dr_rr_new_on_root():
    root_dr = pycdlib.dr.DirectoryRecord()
    root_dr.new_root(None, 1, 2048, time.time())

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        root_dr._rr_new('1.09', b'', b'', False, False, False, 0)
    assert(str(excinfo.value) == 'Invalid call to create new Rock Ridge on root directory')

def test_dr_new_dir_no_parent_rr():
    pvd = pycdlib.headervd.pvd_factory(b'', b'', 0, 0, 0, b'', b'', b'', b'', b'', b'', b'', 0.0, b'', False)
    root_dr = pycdlib.dr.DirectoryRecord()
    root_dr.new_root(pvd, 1, 2048, time.time())

    dir1_dr = pycdlib.dr.DirectoryRecord()
    dir1_dr.new_dir(pvd, b'DIR1', root_dr, 1, '', b'', 2048, False, False, False,
                    0, time.time())

    dir2_dr = pycdlib.dr.DirectoryRecord()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        dir2_dr.new_dir(pvd, b'DIR2', dir1_dr, 1, '1.09', b'', 2048, False,
                        False, False, 0, time.time())
    assert(str(excinfo.value) == 'Parent of the entry did not have Rock Ridge, ISO is corrupt')

def test_dr_rr_new_on_root():
    pvd = pycdlib.headervd.pvd_factory(b'', b'', 0, 0, 0, b'', b'', b'', b'', b'', b'', b'', 0.0, b'', False)
    root_dr = pycdlib.dr.DirectoryRecord()
    root_dr.new_root(pvd, 1, 2048, time.time())

    dir1_dr = pycdlib.dr.DirectoryRecord()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        dir1_dr.new_dir(pvd, b'DIR1', root_dr, 1, '1.09', b'', 2048, False,
                        False, False, 0, time.time())
    assert(str(excinfo.value) == 'Expected at least 2 children of the root directory record, saw 0')

def test_dr_new_symlink_already_initialized():
    pvd = pycdlib.headervd.pvd_factory(b'', b'', 0, 0, 0, b'', b'', b'', b'', b'', b'', b'', 0.0, b'', False)
    dr = pycdlib.dr.DirectoryRecord()
    dr.new_root(pvd, 1, 2048, time.time())

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        dr.new_symlink(pvd, b'', None, b'', 1, '', b'', False, time.time())
    assert(str(excinfo.value) == 'Directory Record already initialized')

def test_dr_new_file_already_initialized():
    pvd = pycdlib.headervd.pvd_factory(b'', b'', 0, 0, 0, b'', b'', b'', b'', b'', b'', b'', 0.0, b'', False)
    dr = pycdlib.dr.DirectoryRecord()
    dr.new_root(pvd, 1, 2048, time.time())

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        dr.new_file(pvd, 0, b'', None, 1, '', b'', False, 0, time.time())
    assert(str(excinfo.value) == 'Directory Record already initialized')

def test_dr_new_root_already_initialized():
    pvd = pycdlib.headervd.pvd_factory(b'', b'', 0, 0, 0, b'', b'', b'', b'', b'', b'', b'', 0.0, b'', False)
    dr = pycdlib.dr.DirectoryRecord()
    dr.new_root(pvd, 1, 2048, time.time())

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        dr.new_root(pvd, 1, 2048, time.time())
    assert(str(excinfo.value) == 'Directory Record already initialized')

def test_dr_new_dot_already_initialized():
    pvd = pycdlib.headervd.pvd_factory(b'', b'', 0, 0, 0, b'', b'', b'', b'', b'', b'', b'', 0.0, b'', False)
    dr = pycdlib.dr.DirectoryRecord()
    dr.new_root(pvd, 1, 2048, time.time())

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        dr.new_dot(pvd, None, 1, '', 2048, False, 0, time.time())
    assert(str(excinfo.value) == 'Directory Record already initialized')

def test_dr_new_dotdot_already_initialized():
    pvd = pycdlib.headervd.pvd_factory(b'', b'', 0, 0, 0, b'', b'', b'', b'', b'', b'', b'', 0.0, b'', False)
    dr = pycdlib.dr.DirectoryRecord()
    dr.new_root(pvd, 1, 2048, time.time())

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        dr.new_dotdot(pvd, None, 1, '', 2048, False, False, 0, time.time())
    assert(str(excinfo.value) == 'Directory Record already initialized')

def test_dr_new_dir_already_initialized():
    pvd = pycdlib.headervd.pvd_factory(b'', b'', 0, 0, 0, b'', b'', b'', b'', b'', b'', b'', 0.0, b'', False)
    dr = pycdlib.dr.DirectoryRecord()
    dr.new_root(pvd, 1, 2048, time.time())

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        dr.new_dir(pvd, b'', None, 1, '', b'', 2048, False, False, False, 0,
                   time.time())
    assert(str(excinfo.value) == 'Directory Record already initialized')

def test_dr_change_existence_not_initialized():
    dr = pycdlib.dr.DirectoryRecord()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        dr.change_existence(False)
    assert(str(excinfo.value) == 'Directory Record not initialized')

def test_dr_add_child_not_dir():
    pvd = pycdlib.headervd.pvd_factory(b'', b'', 0, 0, 0, b'', b'', b'', b'', b'', b'', b'', 0.0, b'', False)
    dr = pycdlib.dr.DirectoryRecord()
    dr.new_file(pvd, 0, b'', None, 1, '', b'', False, 0, time.time())

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        dr.add_child(pvd, 2048)
    assert(str(excinfo.value) == 'Trying to add a child to a record that is not a directory')

def test_dr_add_child_not_initialized():
    dr = pycdlib.dr.DirectoryRecord()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        dr.add_child(None, 2048)
    assert(str(excinfo.value) == 'Directory Record not initialized')

def test_dr_track_child_not_initialized():
    dr = pycdlib.dr.DirectoryRecord()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        dr.track_child(None, 2048)
    assert(str(excinfo.value) == 'Directory Record not initialized')

def test_dr_remove_child_not_initialized():
    dr = pycdlib.dr.DirectoryRecord()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        dr.remove_child(None, 0, 2048)
    assert(str(excinfo.value) == 'Directory Record not initialized')

def test_dr_remove_child_negative_index():
    pvd = pycdlib.headervd.pvd_factory(b'', b'', 0, 0, 0, b'', b'', b'', b'', b'', b'', b'', 0.0, b'', False)
    dr = pycdlib.dr.DirectoryRecord()
    dr.new_file(pvd, 0, b'', None, 1, '', b'', False, 0, time.time())

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        dr.remove_child(pvd, -1, 2048)
    assert(str(excinfo.value) == 'Invalid child index to remove')

def test_dr_is_dir_not_initialized():
    dr = pycdlib.dr.DirectoryRecord()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        dr.is_dir()
    assert(str(excinfo.value) == 'Directory Record not initialized')

def test_dr_is_file_not_initialized():
    dr = pycdlib.dr.DirectoryRecord()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        dr.is_file()
    assert(str(excinfo.value) == 'Directory Record not initialized')

def test_dr_is_symlink_not_initialized():
    dr = pycdlib.dr.DirectoryRecord()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        dr.is_symlink()
    assert(str(excinfo.value) == 'Directory Record not initialized')

def test_dr_is_dot_not_initialized():
    dr = pycdlib.dr.DirectoryRecord()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        dr.is_dot()
    assert(str(excinfo.value) == 'Directory Record not initialized')

def test_dr_is_dotdot_not_initialized():
    dr = pycdlib.dr.DirectoryRecord()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        dr.is_dotdot()
    assert(str(excinfo.value) == 'Directory Record not initialized')

def test_dr_directory_record_length_not_initialized():
    dr = pycdlib.dr.DirectoryRecord()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        dr.directory_record_length()
    assert(str(excinfo.value) == 'Directory Record not initialized')

def test_dr_directory_record_length():
    pvd = pycdlib.headervd.pvd_factory(b'', b'', 0, 0, 0, b'', b'', b'', b'', b'', b'', b'', 0.0, b'', False)
    dr = pycdlib.dr.DirectoryRecord()
    dr.new_file(pvd, 0, b'', None, 1, '', b'', False, 0, time.time())
    assert(dr.directory_record_length() == 34)

def test_dr_directory_extent_location_not_initialized():
    dr = pycdlib.dr.DirectoryRecord()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        dr.extent_location()
    assert(str(excinfo.value) == 'Directory Record not initialized')

def test_dr_directory_file_identifier_not_initialized():
    dr = pycdlib.dr.DirectoryRecord()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        dr.file_identifier()
    assert(str(excinfo.value) == 'Directory Record not initialized')

def test_dr_record_not_initialized():
    dr = pycdlib.dr.DirectoryRecord()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        dr.record()
    assert(str(excinfo.value) == 'Directory Record not initialized')

def test_dr_is_associated_file_not_initialized():
    dr = pycdlib.dr.DirectoryRecord()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        dr.is_associated_file()
    assert(str(excinfo.value) == 'Directory Record not initialized')

def test_dr_set_ptr_not_initialized():
    dr = pycdlib.dr.DirectoryRecord()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        dr.set_ptr(None)
    assert(str(excinfo.value) == 'Directory Record not initialized')

def test_dr_set_data_location_not_initialized():
    dr = pycdlib.dr.DirectoryRecord()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        dr.set_data_location(0, 0)
    assert(str(excinfo.value) == 'Directory Record not initialized')

def test_dr_get_data_length_not_initialized():
    dr = pycdlib.dr.DirectoryRecord()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        dr.get_data_length()
    assert(str(excinfo.value) == 'Directory Record not initialized')

def test_dr_set_data_length_not_initialized():
    dr = pycdlib.dr.DirectoryRecord()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        dr.set_data_length(0)
    assert(str(excinfo.value) == 'Directory Record not initialized')

def test_dr_xattr_with_record_bit():
    pvd = pycdlib.headervd.pvd_factory(b'', b'', 0, 0, 0, b'', b'', b'', b'', b'', b'', b'', 0.0, b'', False)
    dr = pycdlib.dr.DirectoryRecord()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        dr.parse(pvd, b'\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*7 + b'\x08\x00\x00\x00\x00\x00\x00\x00\x00', None)
    assert(str(excinfo.value) == 'Record Bit not allowed with Extended Attributes')

def test_dr_xattr_with_protection_bit():
    pvd = pycdlib.headervd.pvd_factory(b'', b'', 0, 0, 0, b'', b'', b'', b'', b'', b'', b'', 0.0, b'', False)
    dr = pycdlib.dr.DirectoryRecord()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        dr.parse(pvd, b'\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*7 + b'\x10\x00\x00\x00\x00\x00\x00\x00\x00', None)
    assert(str(excinfo.value) == 'Protection Bit not allowed with Extended Attributes')

def test_dr_file_too_big():
    pvd = pycdlib.headervd.pvd_factory(b'', b'', 0, 0, 0, b'', b'', b'', b'', b'', b'', b'', 0.0, b'', False)
    dr = pycdlib.dr.DirectoryRecord()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        dr.new_file(pvd, 2**32, b'', None, 1, '', b'', False, 0, 0)
    assert(str(excinfo.value) == 'Maximum supported file length is 2^32-1')
