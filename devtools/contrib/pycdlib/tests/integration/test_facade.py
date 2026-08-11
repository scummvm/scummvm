# -*- coding: utf-8 -*-

import io
import os
import sys

import pytest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))

import pycdlib

from test_common import *


def test_facade_iso9660_get_file_from_iso(tmpdir):
    iso = pycdlib.PyCdlib()
    iso.new()

    facade = iso.get_iso9660_facade()

    foostr = b'foo\n'
    facade.add_fp(io.BytesIO(foostr), len(foostr), '/FOO.;1')

    foofile = os.path.join(str(tmpdir), 'foo')
    facade.get_file_from_iso(foofile, '/FOO.;1')

    iso.close()

    with open(foofile, 'r') as infp:
        assert(infp.read() == 'foo\n')

def test_facade_iso9660_get_file_from_iso_fp():
    iso = pycdlib.PyCdlib()
    iso.new()

    facade = iso.get_iso9660_facade()

    foostr = b'foo\n'
    facade.add_fp(io.BytesIO(foostr), len(foostr), '/FOO.;1')

    out = io.BytesIO()
    facade.get_file_from_iso_fp(out, '/FOO.;1')

    assert(out.getvalue() == b'foo\n')

    iso.close()

def test_facade_iso9660_add_fp():
    iso = pycdlib.PyCdlib()
    iso.new()

    facade = iso.get_iso9660_facade()

    foostr = b'foo\n'
    facade.add_fp(io.BytesIO(foostr), len(foostr), '/FOO.;1')

    out = io.BytesIO()
    facade.get_file_from_iso_fp(out, '/FOO.;1')

    assert(out.getvalue() == b'foo\n')

    iso.close()

def test_facade_iso9660_add_fp_with_rr():
    iso = pycdlib.PyCdlib()
    iso.new(rock_ridge='1.09')

    facade = iso.get_iso9660_facade()

    foostr = b'foo\n'
    facade.add_fp(io.BytesIO(foostr), len(foostr), '/FOO.;1')

    out = io.BytesIO()
    facade.get_file_from_iso_fp(out, '/FOO.;1')

    assert(out.getvalue() == b'foo\n')

    iso.close()

def test_facade_iso9660_add_fp_with_rr_bad_name():
    iso = pycdlib.PyCdlib()
    iso.new(rock_ridge='1.09')

    facade = iso.get_iso9660_facade()

    foostr = b'foo\n'
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        facade.add_fp(io.BytesIO(foostr), len(foostr), 'FOO.;1')
    assert(str(excinfo.value) == "iso_path must start with '/'")

    iso.close()

def test_facade_iso9660_add_file(tmpdir):
    iso = pycdlib.PyCdlib()
    iso.new()

    testout = tmpdir.join('writetest.iso')
    with open(str(testout), 'wb') as outfp:
        outfp.write(b'foo\n')

    facade = iso.get_iso9660_facade()

    facade.add_file(str(testout), '/FOO.;1')

    out = io.BytesIO()
    facade.get_file_from_iso_fp(out, '/FOO.;1')

    assert(out.getvalue() == b'foo\n')

    iso.close()

def test_facade_iso9660_add_file_with_rr(tmpdir):
    iso = pycdlib.PyCdlib()
    iso.new(rock_ridge='1.09')

    testout = tmpdir.join('writetest.iso')
    with open(str(testout), 'wb') as outfp:
        outfp.write(b'foo\n')

    facade = iso.get_iso9660_facade()

    facade.add_file(str(testout), '/FOO.;1')

    out = io.BytesIO()
    facade.get_file_from_iso_fp(out, '/FOO.;1')

    assert(out.getvalue() == b'foo\n')

    iso.close()

def test_facade_iso9660_add_directory():
    iso = pycdlib.PyCdlib()
    iso.new()

    facade = iso.get_iso9660_facade()

    facade.add_directory('/DIR1')

    rec = facade.get_record('/DIR1')
    assert(rec.file_identifier() == b'DIR1')

    iso.close()

def test_facade_iso9660_add_directory_with_rr():
    iso = pycdlib.PyCdlib()
    iso.new(rock_ridge='1.09')

    facade = iso.get_iso9660_facade()

    facade.add_directory('/DIR1')

    rec = facade.get_record('/DIR1')
    assert(rec.file_identifier() == b'DIR1')

    iso.close()

def test_facade_iso9660_rm_file():
    iso = pycdlib.PyCdlib()
    iso.new()

    facade = iso.get_iso9660_facade()

    foostr = b'foo\n'
    facade.add_fp(io.BytesIO(foostr), len(foostr), '/FOO.;1')
    rec = facade.get_record('/')
    assert(len(rec.children) == 3)

    facade.rm_file('/FOO.;1')

    rec = facade.get_record('/')
    assert(len(rec.children) == 2)  # The dot and dotdot records count

    iso.close()

def test_facade_iso9660_rm_directory():
    iso = pycdlib.PyCdlib()
    iso.new()

    facade = iso.get_iso9660_facade()

    facade.add_directory('/DIR1')
    rec = facade.get_record('/')
    assert(len(rec.children) == 3)

    facade.rm_directory('/DIR1')
    rec = facade.get_record('/')
    assert(len(rec.children) == 2)

    iso.close()

def test_facade_iso9660_list_children():
    # Create a new ISO.
    iso = pycdlib.PyCdlib()
    iso.new()

    facade = iso.get_iso9660_facade()

    facade.add_directory('/DIR1')

    bootstr = b'boot\n'
    facade.add_fp(io.BytesIO(bootstr), len(bootstr), '/DIR1/BOOT.;1')

    full_path = None
    for child in facade.list_children('/DIR1'):
        if child.file_identifier() == b'BOOT.;1':
            full_path = iso.full_path_from_dirrecord(child)
            assert(full_path == '/DIR1/BOOT.;1')
            break

    assert(full_path is not None)
    iso.close()

def test_facade_iso9660_get_record():
    iso = pycdlib.PyCdlib()
    iso.new()

    facade = iso.get_iso9660_facade()

    facade.add_directory('/DIR1')

    rec = facade.get_record('/DIR1')
    assert(rec.file_identifier() == b'DIR1')

    iso.close()

def test_facade_iso9660_walk():
    iso = pycdlib.PyCdlib()
    iso.new()

    facade = iso.get_iso9660_facade()

    facade.add_directory('/DIR1')

    index = 0
    for dirname, dirlist, filelist in facade.walk('/'):
        if index == 0:
            assert(dirname == '/')
        else:
            assert(dirname == '/DIR1')
        index += 1

    iso.close()

def test_facade_iso9660_open_file_from_iso():
    iso = pycdlib.PyCdlib()
    iso.new()

    facade = iso.get_iso9660_facade()

    foostr = b'foo\n'
    facade.add_fp(io.BytesIO(foostr), len(foostr), '/FOO.;1')

    with facade.open_file_from_iso('/FOO.;1') as infp:
        assert(infp.read() == b'foo\n')
        assert(infp.tell() == 4)

    iso.close()

def test_facade_joliet_not_joliet_iso():
    iso = pycdlib.PyCdlib()
    iso.new()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        facade = pycdlib.facade.PyCdlibJoliet(iso)
    assert(str(excinfo.value) == 'Can only instantiate a Joliet facade for a Joliet ISO')

def test_facade_joliet_get_file_from_iso(tmpdir):
    iso = pycdlib.PyCdlib()
    iso.new(joliet=3)

    facade = iso.get_joliet_facade()

    foostr = b'foo\n'
    facade.add_fp(io.BytesIO(foostr), len(foostr), '/foo')

    foofile = os.path.join(str(tmpdir), 'foo')
    facade.get_file_from_iso(foofile, '/foo')

    iso.close()

    with open(foofile, 'r') as infp:
        assert(infp.read() == 'foo\n')

def test_facade_joliet_get_file_from_iso_fp():
    iso = pycdlib.PyCdlib()
    iso.new(joliet=3)

    facade = iso.get_joliet_facade()

    foostr = b'foo\n'
    facade.add_fp(io.BytesIO(foostr), len(foostr), '/foo')

    out = io.BytesIO()
    facade.get_file_from_iso_fp(out, '/foo')

    assert(out.getvalue() == b'foo\n')

    iso.close()

def test_facade_joliet_add_fp():
    iso = pycdlib.PyCdlib()
    iso.new(joliet=3)

    facade = iso.get_joliet_facade()

    foostr = b'foo\n'
    facade.add_fp(io.BytesIO(foostr), len(foostr), '/foo')

    out = io.BytesIO()
    facade.get_file_from_iso_fp(out, '/foo')

    assert(out.getvalue() == b'foo\n')

    iso.close()

def test_facade_joliet_add_file(tmpdir):
    iso = pycdlib.PyCdlib()
    iso.new(joliet=3)

    testout = tmpdir.join('writetest.iso')
    with open(str(testout), 'wb') as outfp:
        outfp.write(b'foo\n')

    facade = iso.get_joliet_facade()

    facade.add_file(str(testout), '/foo')

    out = io.BytesIO()
    facade.get_file_from_iso_fp(out, '/foo')

    assert(out.getvalue() == b'foo\n')

    iso.close()

def test_facade_joliet_add_directory():
    iso = pycdlib.PyCdlib()
    iso.new(joliet=3)

    facade = iso.get_joliet_facade()

    facade.add_directory('/dir1')

    rec = facade.get_record('/dir1')
    assert(rec.file_identifier() == bytes('dir1'.encode('utf-16_be')))

    iso.close()

def test_facade_joliet_rm_file():
    iso = pycdlib.PyCdlib()
    iso.new(joliet=3)

    facade = iso.get_joliet_facade()

    foostr = b'foo\n'
    facade.add_fp(io.BytesIO(foostr), len(foostr), '/foo')
    rec = facade.get_record('/')
    assert(len(rec.children) == 3)

    facade.rm_file('/foo')

    rec = facade.get_record('/')
    assert(len(rec.children) == 2)  # The dot and dotdot records count

    iso.close()

def test_facade_joliet_rm_directory():
    iso = pycdlib.PyCdlib()
    iso.new(joliet=3)

    facade = iso.get_joliet_facade()

    facade.add_directory('/dir1')
    rec = facade.get_record('/')
    assert(len(rec.children) == 3)

    facade.rm_directory('/dir1')
    rec = facade.get_record('/')
    assert(len(rec.children) == 2)

    iso.close()

def test_facade_joliet_list_children():
    # Create a new ISO.
    iso = pycdlib.PyCdlib()
    iso.new(joliet=3)

    facade = iso.get_joliet_facade()

    facade.add_directory('/dir1')

    bootstr = b'boot\n'
    facade.add_fp(io.BytesIO(bootstr), len(bootstr), '/dir1/boot')

    full_path = None
    for child in facade.list_children('/dir1'):
        if child.file_identifier() == bytes('boot'.encode('utf-16_be')):
            full_path = iso.full_path_from_dirrecord(child)
            assert(full_path == '/dir1/boot')
            break

    assert(full_path is not None)
    iso.close()

def test_facade_joliet_get_record():
    iso = pycdlib.PyCdlib()
    iso.new(joliet=3)

    facade = iso.get_joliet_facade()

    facade.add_directory('/dir1')

    rec = facade.get_record('/dir1')
    assert(rec.file_identifier() == bytes('dir1'.encode('utf-16_be')))

    iso.close()

def test_facade_joliet_walk():
    iso = pycdlib.PyCdlib()
    iso.new(joliet=3)

    facade = iso.get_joliet_facade()

    facade.add_directory('/dir1')

    index = 0
    for dirname, dirlist, filelist in facade.walk('/'):
        if index == 0:
            assert(dirname == '/')
        else:
            assert(dirname == '/dir1')
        index += 1

    iso.close()

def test_facade_joliet_open_file_from_iso():
    iso = pycdlib.PyCdlib()
    iso.new(joliet=3)

    facade = iso.get_joliet_facade()

    foostr = b'foo\n'
    facade.add_fp(io.BytesIO(foostr), len(foostr), '/foo')

    with facade.open_file_from_iso('/foo') as infp:
        assert(infp.read() == b'foo\n')
        assert(infp.tell() == 4)

    iso.close()

def test_facade_rock_ridge_not_rock_ridge_iso():
    iso = pycdlib.PyCdlib()
    iso.new()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        facade = pycdlib.facade.PyCdlibRockRidge(iso)
    assert(str(excinfo.value) == 'Can only instantiate a Rock Ridge facade for a Rock Ridge ISO')

def test_facade_rock_ridge_get_file_from_iso(tmpdir):
    iso = pycdlib.PyCdlib()
    iso.new(rock_ridge='1.09')

    facade = iso.get_rock_ridge_facade()

    foostr = b'foo\n'
    facade.add_fp(io.BytesIO(foostr), len(foostr), '/foo', 0o040555)

    foofile = os.path.join(str(tmpdir), 'foo')
    facade.get_file_from_iso(foofile, '/foo')

    iso.close()

    with open(foofile, 'r') as infp:
        assert(infp.read() == 'foo\n')

def test_facade_rock_ridge_get_file_from_iso_fp():
    iso = pycdlib.PyCdlib()
    iso.new(rock_ridge='1.09')

    facade = iso.get_rock_ridge_facade()

    foostr = b'foo\n'
    facade.add_fp(io.BytesIO(foostr), len(foostr), '/foo', 0o040555)

    out = io.BytesIO()
    facade.get_file_from_iso_fp(out, '/foo')

    assert(out.getvalue() == b'foo\n')

    iso.close()

def test_facade_rock_ridge_add_fp():
    iso = pycdlib.PyCdlib()
    iso.new(rock_ridge='1.09')

    facade = iso.get_rock_ridge_facade()

    foostr = b'foo\n'
    facade.add_fp(io.BytesIO(foostr), len(foostr), '/foo', 0o040555)

    out = io.BytesIO()
    facade.get_file_from_iso_fp(out, '/foo')

    assert(out.getvalue() == b'foo\n')

    iso.close()

def test_facade_rock_ridge_add_fp_bad_filename():
    iso = pycdlib.PyCdlib()
    iso.new(rock_ridge='1.09')

    facade = iso.get_rock_ridge_facade()

    foostr = b'foo\n'
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        facade.add_fp(io.BytesIO(foostr), len(foostr), 'foo', 0o040555)
    assert(str(excinfo.value) == "rr_path must start with '/'")

    iso.close()

def test_facade_rock_ridge_add_file(tmpdir):
    iso = pycdlib.PyCdlib()
    iso.new(rock_ridge='1.09')

    testout = tmpdir.join('writetest.iso')
    with open(str(testout), 'wb') as outfp:
        outfp.write(b'foo\n')

    facade = iso.get_rock_ridge_facade()

    facade.add_file(str(testout), '/foo', 0o040555)

    out = io.BytesIO()
    facade.get_file_from_iso_fp(out, '/foo')

    assert(out.getvalue() == b'foo\n')

    iso.close()

def test_facade_rock_ridge_add_directory():
    iso = pycdlib.PyCdlib()
    iso.new(rock_ridge='1.09')

    facade = iso.get_rock_ridge_facade()

    facade.add_directory('/dir1', 0o040555)

    rec = facade.get_record('/dir1')
    assert(rec.file_identifier() == b'DIR1')

    iso.close()

def test_facade_rock_ridge_rm_file():
    iso = pycdlib.PyCdlib()
    iso.new(rock_ridge='1.09')

    facade = iso.get_rock_ridge_facade()

    foostr = b'foo\n'
    facade.add_fp(io.BytesIO(foostr), len(foostr), '/foo', 0o040555)
    rec = facade.get_record('/')
    assert(len(rec.children) == 3)

    facade.rm_file('/foo')

    rec = facade.get_record('/')
    assert(len(rec.children) == 2)  # The dot and dotdot records count

    iso.close()

def test_facade_rock_ridge_rm_file_bad_filename():
    iso = pycdlib.PyCdlib()
    iso.new(rock_ridge='1.09')

    facade = iso.get_rock_ridge_facade()

    foostr = b'foo\n'
    facade.add_fp(io.BytesIO(foostr), len(foostr), '/foo', 0o040555)
    rec = facade.get_record('/')
    assert(len(rec.children) == 3)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        facade.rm_file('foo')
    assert(str(excinfo.value) == "rr_path must start with '/'")

    iso.close()

def test_facade_rock_ridge_rm_file_root():
    iso = pycdlib.PyCdlib()
    iso.new(rock_ridge='1.09')

    facade = iso.get_rock_ridge_facade()

    foostr = b'foo\n'
    facade.add_fp(io.BytesIO(foostr), len(foostr), '/foo', 0o040555)
    rec = facade.get_record('/')
    assert(len(rec.children) == 3)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        facade.rm_file('/')
    assert(str(excinfo.value) == 'Cannot remove a directory with rm_file (try rm_directory instead)')

    iso.close()

def test_facade_rock_ridge_rm_directory():
    iso = pycdlib.PyCdlib()
    iso.new(rock_ridge='1.09')

    facade = iso.get_rock_ridge_facade()

    facade.add_directory('/dir1', 0o040555)
    rec = facade.get_record('/')
    assert(len(rec.children) == 3)

    facade.rm_directory('/dir1')
    rec = facade.get_record('/')
    assert(len(rec.children) == 2)

    iso.close()

def test_facade_rock_ridge_rm_directory_root():
    iso = pycdlib.PyCdlib()
    iso.new(rock_ridge='1.09')

    facade = iso.get_rock_ridge_facade()

    foostr = b'foo\n'
    facade.add_fp(io.BytesIO(foostr), len(foostr), '/foo', 0o040555)
    rec = facade.get_record('/')
    assert(len(rec.children) == 3)

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        facade.rm_directory('/')
    assert(str(excinfo.value) == 'Cannot remove base directory')

    iso.close()

def test_facade_rock_ridge_add_symlink():
    iso = pycdlib.PyCdlib()
    iso.new(rock_ridge='1.09')

    facade = iso.get_rock_ridge_facade()

    foostr = b'foo\n'
    facade.add_fp(io.BytesIO(foostr), len(foostr), '/foo', 0o040555)

    facade.add_symlink('/sym', 'foo')

    rec = facade.get_record('/')
    assert(len(rec.children) == 4)

    iso.close()

def test_facade_rock_ridge_list_children():
    # Create a new ISO.
    iso = pycdlib.PyCdlib()
    iso.new(rock_ridge='1.09')

    facade = iso.get_rock_ridge_facade()

    facade.add_directory('/dir1', 0o040555)

    bootstr = b'boot\n'
    facade.add_fp(io.BytesIO(bootstr), len(bootstr), '/dir1/boot', 0o040555)

    full_path = None
    for child in facade.list_children('/dir1'):
        if child.file_identifier() == b'BOOT.;1':
            full_path = iso.full_path_from_dirrecord(child)
            assert(full_path == '/DIR1/BOOT.;1')
            break

    assert(full_path is not None)
    iso.close()

def test_facade_rock_ridge_get_record():
    iso = pycdlib.PyCdlib()
    iso.new(rock_ridge='1.09')

    facade = iso.get_rock_ridge_facade()

    facade.add_directory('/dir1', 0o040555)

    rec = facade.get_record('/dir1')
    assert(rec.file_identifier() == b'DIR1')

    iso.close()

def test_facade_rock_ridge_walk():
    iso = pycdlib.PyCdlib()
    iso.new(rock_ridge='1.09')

    facade = iso.get_rock_ridge_facade()

    facade.add_directory('/dir1', 0o040555)

    index = 0
    for dirname, dirlist, filelist in facade.walk('/'):
        if index == 0:
            assert(dirname == '/')
        else:
            assert(dirname == '/dir1')
        index += 1

    iso.close()

def test_facade_rock_ridge_open_file_from_iso():
    iso = pycdlib.PyCdlib()
    iso.new(rock_ridge='1.09')

    facade = iso.get_rock_ridge_facade()

    foostr = b'foo\n'
    facade.add_fp(io.BytesIO(foostr), len(foostr), '/foo', 0o040555)

    with facade.open_file_from_iso('/foo') as infp:
        assert(infp.read() == b'foo\n')
        assert(infp.tell() == 4)

    iso.close()

def test_facade_udf_not_udf_iso():
    iso = pycdlib.PyCdlib()
    iso.new()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        facade = pycdlib.facade.PyCdlibUDF(iso)
    assert(str(excinfo.value) == 'Can only instantiate a UDF facade for a UDF ISO')

def test_facade_udf_get_file_from_iso(tmpdir):
    iso = pycdlib.PyCdlib()
    iso.new(udf='2.60')

    facade = iso.get_udf_facade()

    foostr = b'foo\n'
    facade.add_fp(io.BytesIO(foostr), len(foostr), '/foo')

    foofile = os.path.join(str(tmpdir), 'foo')
    facade.get_file_from_iso(foofile, '/foo')

    iso.close()

    with open(foofile, 'r') as infp:
        assert(infp.read() == 'foo\n')

def test_facade_udf_get_file_from_iso_fp():
    iso = pycdlib.PyCdlib()
    iso.new(udf='2.60')

    facade = iso.get_udf_facade()

    foostr = b'foo\n'
    facade.add_fp(io.BytesIO(foostr), len(foostr), '/foo')

    out = io.BytesIO()
    facade.get_file_from_iso_fp(out, '/foo')

    assert(out.getvalue() == b'foo\n')

    iso.close()

def test_facade_udf_add_fp():
    iso = pycdlib.PyCdlib()
    iso.new(udf='2.60')

    facade = iso.get_udf_facade()

    foostr = b'foo\n'
    facade.add_fp(io.BytesIO(foostr), len(foostr), '/foo')

    out = io.BytesIO()
    facade.get_file_from_iso_fp(out, '/foo')

    assert(out.getvalue() == b'foo\n')

    iso.close()

def test_facade_udf_add_file(tmpdir):
    iso = pycdlib.PyCdlib()
    iso.new(udf='2.60')

    testout = tmpdir.join('writetest.iso')
    with open(str(testout), 'wb') as outfp:
        outfp.write(b'foo\n')

    facade = iso.get_udf_facade()

    facade.add_file(str(testout), '/foo')

    out = io.BytesIO()
    facade.get_file_from_iso_fp(out, '/foo')

    assert(out.getvalue() == b'foo\n')

    iso.close()

def test_facade_udf_add_directory():
    iso = pycdlib.PyCdlib()
    iso.new(udf='2.60')

    facade = iso.get_udf_facade()

    facade.add_directory('/dir1')

    rec = facade.get_record('/dir1')
    assert(rec.file_identifier() == b'dir1')

    iso.close()

def test_facade_udf_rm_file():
    iso = pycdlib.PyCdlib()
    iso.new(udf='2.60')

    facade = iso.get_udf_facade()

    foostr = b'foo\n'
    facade.add_fp(io.BytesIO(foostr), len(foostr), '/foo')
    rec = facade.get_record('/foo')
    assert(rec.is_file())

    facade.rm_file('/foo')

    iso.close()

def test_facade_udf_rm_directory():
    iso = pycdlib.PyCdlib()
    iso.new(udf='2.60')

    facade = iso.get_udf_facade()

    facade.add_directory('/dir1')
    rec = facade.get_record('/dir1')
    assert(rec.is_dir())

    facade.rm_directory('/dir1')

    iso.close()

def test_facade_udf_add_symlink():
    iso = pycdlib.PyCdlib()
    iso.new(udf='2.60')

    facade = iso.get_udf_facade()

    foostr = b'foo\n'
    facade.add_fp(io.BytesIO(foostr), len(foostr), '/foo')

    facade.add_symlink('/sym', 'foo')

    rec = facade.get_record('/sym')
    assert(rec.is_symlink())

    iso.close()

def test_facade_udf_list_children():
    # Create a new ISO.
    iso = pycdlib.PyCdlib()
    iso.new(udf='2.60')

    facade = iso.get_udf_facade()

    facade.add_directory('/dir1')

    bootstr = b'boot\n'
    facade.add_fp(io.BytesIO(bootstr), len(bootstr), '/dir1/boot')

    full_path = None
    for child in facade.list_children('/dir1'):
        if child is not None:
            if child.file_identifier() == b'boot':
                break
    else:
        assert(False)

    iso.close()

def test_facade_udf_get_record():
    iso = pycdlib.PyCdlib()
    iso.new(udf='2.60')

    facade = iso.get_udf_facade()

    facade.add_directory('/dir1')

    rec = facade.get_record('/dir1')
    assert(rec.file_identifier() == b'dir1')

    iso.close()

def test_facade_udf_walk():
    iso = pycdlib.PyCdlib()
    iso.new(udf='2.60')

    facade = iso.get_udf_facade()

    facade.add_directory('/dir1')

    index = 0
    for dirname, dirlist, filelist in facade.walk('/'):
        if index == 0:
            assert(dirname == '/')
        else:
            assert(dirname == '/dir1')
        index += 1

    iso.close()

def test_facade_udf_open_file_from_iso():
    iso = pycdlib.PyCdlib()
    iso.new(udf='2.60')

    facade = iso.get_udf_facade()

    foostr = b'foo\n'
    facade.add_fp(io.BytesIO(foostr), len(foostr), '/foo')

    with facade.open_file_from_iso('/foo') as infp:
        assert(infp.read() == b'foo\n')
        assert(infp.tell() == 4)

    iso.close()
