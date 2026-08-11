# -*- coding: utf-8 -*-

import subprocess
import os
import sys
import struct

import pytest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))

import pycdlib

from test_common import *

def do_a_test(tmpdir, outfile, check_func):
    testout = tmpdir.join('writetest.iso')

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()
    iso.open(str(outfile))
    check_func(iso, os.stat(str(outfile)).st_size)

    iso.write(str(testout))

    iso.close()

    # Now round-trip through write.
    iso2 = pycdlib.PyCdlib()
    iso2.open(str(testout))
    check_func(iso2, os.stat(str(outfile)).st_size)
    iso2.close()

def test_parse_invalid_file(tmpdir):
    iso = pycdlib.PyCdlib()
    with pytest.raises(TypeError):
        iso.open(None)

def test_parse_nofiles(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('nofiles')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_nofiles)

def test_parse_onefile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('onefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_onefile)

def test_parse_onedir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('onedir')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_onedir)

def test_parse_twofiles(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('twofile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    with open(os.path.join(str(indir), 'bar'), 'wb') as outfp:
        outfp.write(b'bar\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_twofiles)

def test_parse_twodirs(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('onefileonedir')
    outfile = str(indir)+'.iso'
    indir.mkdir('bb')
    indir.mkdir('aa')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_twodirs)

def test_parse_onefileonedir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('onefileonedir')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_onefileonedir)

def test_parse_onefile_onedirwithfile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('onefileonedirwithfile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    dir1 = indir.mkdir('dir1')
    with open(os.path.join(str(dir1), 'bar'), 'wb') as outfp:
        outfp.write(b'bar\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_onefile_onedirwithfile)

def test_parse_tendirs(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('tendirs')
    outfile = str(indir)+'.iso'
    numdirs = 10
    for i in range(1, 1+numdirs):
        indir.mkdir('dir%d' % i)
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_tendirs)

def test_parse_dirs_overflow_ptr_extent(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('manydirs')
    outfile = str(indir)+'.iso'
    numdirs = 295
    for i in range(1, 1+numdirs):
        indir.mkdir('dir%d' % i)
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_dirs_overflow_ptr_extent)

def test_parse_dirs_just_short_ptr_extent(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('manydirs')
    outfile = str(indir)+'.iso'
    numdirs = 293
    for i in range(1, 1+numdirs):
        indir.mkdir('dir%d' % i)
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_dirs_just_short_ptr_extent)

def test_parse_twoextentfile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('bigfile')
    outfile = str(indir)+'.iso'
    outstr = b''
    for j in range(0, 8):
        for i in range(0, 256):
            outstr += struct.pack('=B', i)
    outstr += struct.pack('=B', 0)
    with open(os.path.join(str(indir), 'bigfile'), 'wb') as outfp:
        outfp.write(outstr)
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    testout = tmpdir.join('writetest.iso')

    do_a_test(tmpdir, outfile, check_twoextentfile)

def test_parse_twoleveldeepdir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('twoleveldeep')
    outfile = str(indir)+'.iso'
    dir1 = indir.mkdir('dir1')
    dir1.mkdir('subdir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_twoleveldeepdir)

def test_parse_twoleveldeepfile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('twoleveldeepfile')
    outfile = str(indir)+'.iso'
    dir1 = indir.mkdir('dir1')
    subdir1 = dir1.mkdir('subdir1')
    with open(os.path.join(str(subdir1), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_twoleveldeepfile)

def test_parse_joliet_nofiles(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('joliet')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_joliet_nofiles)

def test_parse_joliet_onedir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('joliet')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_joliet_onedir)

def test_parse_joliet_onefile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolietfile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_joliet_onefile)

def test_parse_joliet_onefileonedir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolietfile')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1')
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_joliet_onefileonedir)

def test_parse_eltorito_nofiles(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('eltoritonofiles')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_eltorito_nofiles)

def test_parse_eltorito_twofile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('eltoritotwofile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    with open(os.path.join(str(indir), 'aa'), 'wb') as outfp:
        outfp.write(b'aa\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_eltorito_twofile)

def test_parse_rr_nofiles(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rrnofiles')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_rr_nofiles)

def test_parse_rr_onefile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rronefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_rr_onefile)

def test_parse_rr_twofile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rrtwofile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    with open(os.path.join(str(indir), 'bar'), 'wb') as outfp:
        outfp.write(b'bar\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_rr_twofile)

def test_parse_rr_onefileonedir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rronefileonedir')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_rr_onefileonedir)

def test_parse_rr_onefileonedirwithfile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rronefileonedirwithfile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    dir1 = indir.mkdir('dir1')
    with open(os.path.join(str(dir1), 'bar'), 'wb') as outfp:
        outfp.write(b'bar\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_rr_onefileonedirwithfile)

def test_parse_rr_symlink(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rrsymlink')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    pwd = os.getcwd()
    os.chdir(str(indir))
    os.symlink('foo', 'sym')
    os.chdir(pwd)
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_rr_symlink)

def test_parse_rr_symlink2(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rrsymlink2')
    outfile = str(indir)+'.iso'
    dir1 = indir.mkdir('dir1')
    with open(os.path.join(str(dir1), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    pwd = os.getcwd()
    os.chdir(str(indir))
    os.symlink('dir1/foo', 'sym')
    os.chdir(pwd)
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_rr_symlink2)

def test_parse_rr_symlink_dot(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rrsymlinkdot')
    outfile = str(indir)+'.iso'
    pwd = os.getcwd()
    os.chdir(str(indir))
    os.symlink('.', 'sym')
    os.chdir(pwd)
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_rr_symlink_dot)

def test_parse_rr_symlink_dotdot(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rrsymlinkdotdot')
    outfile = str(indir)+'.iso'
    pwd = os.getcwd()
    os.chdir(str(indir))
    os.symlink('..', 'sym')
    os.chdir(pwd)
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_rr_symlink_dotdot)

def test_parse_rr_symlink_broken(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rrsymlinkbroken')
    outfile = str(indir)+'.iso'
    pwd = os.getcwd()
    os.chdir(str(indir))
    os.symlink('foo', 'sym')
    os.chdir(pwd)
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_rr_symlink_broken)

def test_parse_alternating_subdir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('alternating')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'bb'), 'wb') as outfp:
        outfp.write(b'bb\n')
    cc = indir.mkdir('cc')
    aa = indir.mkdir('aa')
    with open(os.path.join(str(indir), 'dd'), 'wb') as outfp:
        outfp.write(b'dd\n')
    with open(os.path.join(str(cc), 'sub2'), 'wb') as outfp:
        outfp.write(b'sub2\n')
    with open(os.path.join(str(aa), 'sub1'), 'wb') as outfp:
        outfp.write(b'sub1\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_alternating_subdir)

def test_parse_rr_verylongname(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rrverylongname')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'a'*RR_MAX_FILENAME_LENGTH), 'wb') as outfp:
        outfp.write(b'aa\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_rr_verylongname)

def test_parse_rr_verylongname_joliet(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rrverylongname')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'a'*RR_MAX_FILENAME_LENGTH), 'wb') as outfp:
        outfp.write(b'aa\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-rational-rock', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_rr_verylongname_joliet)

def test_parse_rr_manylongname(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rrmanylongname')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'a'*RR_MAX_FILENAME_LENGTH), 'wb') as outfp:
        outfp.write(b'aa\n')
    with open(os.path.join(str(indir), 'b'*RR_MAX_FILENAME_LENGTH), 'wb') as outfp:
        outfp.write(b'bb\n')
    with open(os.path.join(str(indir), 'c'*RR_MAX_FILENAME_LENGTH), 'wb') as outfp:
        outfp.write(b'cc\n')
    with open(os.path.join(str(indir), 'd'*RR_MAX_FILENAME_LENGTH), 'wb') as outfp:
        outfp.write(b'dd\n')
    with open(os.path.join(str(indir), 'e'*RR_MAX_FILENAME_LENGTH), 'wb') as outfp:
        outfp.write(b'ee\n')
    with open(os.path.join(str(indir), 'f'*RR_MAX_FILENAME_LENGTH), 'wb') as outfp:
        outfp.write(b'ff\n')
    with open(os.path.join(str(indir), 'g'*RR_MAX_FILENAME_LENGTH), 'wb') as outfp:
        outfp.write(b'gg\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_rr_manylongname)

def test_parse_rr_manylongname2(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rrmanylongname2')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'a'*RR_MAX_FILENAME_LENGTH), 'wb') as outfp:
        outfp.write(b'aa\n')
    with open(os.path.join(str(indir), 'b'*RR_MAX_FILENAME_LENGTH), 'wb') as outfp:
        outfp.write(b'bb\n')
    with open(os.path.join(str(indir), 'c'*RR_MAX_FILENAME_LENGTH), 'wb') as outfp:
        outfp.write(b'cc\n')
    with open(os.path.join(str(indir), 'd'*RR_MAX_FILENAME_LENGTH), 'wb') as outfp:
        outfp.write(b'dd\n')
    with open(os.path.join(str(indir), 'e'*RR_MAX_FILENAME_LENGTH), 'wb') as outfp:
        outfp.write(b'ee\n')
    with open(os.path.join(str(indir), 'f'*RR_MAX_FILENAME_LENGTH), 'wb') as outfp:
        outfp.write(b'ff\n')
    with open(os.path.join(str(indir), 'g'*RR_MAX_FILENAME_LENGTH), 'wb') as outfp:
        outfp.write(b'gg\n')
    with open(os.path.join(str(indir), 'h'*RR_MAX_FILENAME_LENGTH), 'wb') as outfp:
        outfp.write(b'hh\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_rr_manylongname2)

def test_parse_joliet_and_rr_nofiles(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolietandrrnofiles')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-J', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_joliet_and_rr_nofiles)

def test_parse_joliet_and_rr_onefile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolietandrronefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-J', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_joliet_and_rr_onefile)

def test_parse_joliet_and_rr_onedir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolietandrronedir')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-J', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_joliet_and_rr_onedir)

def test_parse_rr_and_eltorito_nofiles(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rrandeltoritonofiles')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_rr_and_eltorito_nofiles)

def test_parse_rr_and_eltorito_onefile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rrandeltoritoonefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_rr_and_eltorito_onefile)

def test_parse_rr_and_eltorito_onedir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rrandeltoritoonedir')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_rr_and_eltorito_onedir)

def test_parse_joliet_and_eltorito_nofiles(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolietandeltoritonofiles')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-J', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_joliet_and_eltorito_nofiles)

def test_parse_joliet_and_eltorito_onefile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolietandeltoritoonefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-J', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_joliet_and_eltorito_onefile)

def test_parse_joliet_and_eltorito_onedir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolietandeltoritoonedir')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-J', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_joliet_and_eltorito_onedir)

@pytest.mark.skipif(find_executable('isohybrid') is None,
                    reason='syslinux not installed')
def test_parse_isohybrid(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('isohybrid')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'isolinux.bin'), 'wb') as outfp:
        outfp.seek(0x40)
        outfp.write(b'\xfb\xc0\x78\x70')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'isolinux.bin', '-no-emul-boot',
                     '-boot-load-size', '4',
                     '-o', str(outfile), str(indir)])
    subprocess.call(['isohybrid', '-v', str(outfile)])

    do_a_test(tmpdir, outfile, check_isohybrid)

@pytest.mark.skipif(find_executable('isohybrid') is None,
                    reason='syslinux not installed')
def test_parse_isohybrid_uefi(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('isohybriduefi')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'isolinux.bin'), 'wb') as outfp:
        outfp.seek(0x40)
        outfp.write(b'\xfb\xc0\x78\x70')
    with open(os.path.join(str(indir), 'efiboot.img'), 'wb') as outfp:
        outfp.write(b'a')
    retcode = subprocess.call(['genisoimage', '-v', '-v', '-no-pad',
                               '-c', 'boot.cat', '-b', 'isolinux.bin',
                               '-no-emul-boot', '-boot-load-size', '4',
                               '-boot-info-table', '-eltorito-alt-boot',
                               '-efi-boot', 'efiboot.img', '-no-emul-boot',
                               '-o', str(outfile), str(indir)])
    if retcode != 0:
        pytest.skip("This version of genisoimage doesn't support -efi-boot")

    subprocess.call(['isohybrid', '-u', '-v', str(outfile)])

    do_a_test(tmpdir, outfile, check_isohybrid_uefi)

@pytest.mark.skipif(find_executable('isohybrid') is None,
                    reason='syslinux not installed')
def test_parse_isohybrid_mac_uefi(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('isohybridmacuefi')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'isolinux.bin'), 'wb') as outfp:
        outfp.seek(0x40)
        outfp.write(b'\xfb\xc0\x78\x70')
    with open(os.path.join(str(indir), 'efiboot.img'), 'wb') as outfp:
        outfp.write(b'a')
    with open(os.path.join(str(indir), 'macboot.img'), 'wb') as outfp:
        outfp.write(b'b')
    retcode = subprocess.call(['genisoimage', '-v', '-v', '-no-pad',
                               '-c', 'boot.cat', '-b', 'isolinux.bin',
                               '-no-emul-boot', '-boot-load-size', '4',
                               '-boot-info-table', '-eltorito-alt-boot',
                               '-efi-boot', 'efiboot.img', '-no-emul-boot',
                               '-eltorito-alt-boot', '-efi-boot', 'macboot.img',
                               '-no-emul-boot',
                               '-o', str(outfile), str(indir)])
    if retcode != 0:
        pytest.skip("This version of genisoimage doesn't support -efi-boot")

    subprocess.call(['isohybrid', '-u', '-m', '-v', str(outfile)])

    do_a_test(tmpdir, outfile, check_isohybrid_mac_uefi)

def test_parse_joliet_rr_and_eltorito_nofiles(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolietrrandeltoritonofiles')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-J', '-rational-rock', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_joliet_rr_and_eltorito_nofiles)

def test_parse_joliet_rr_and_eltorito_onefile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolietrrandeltoritoonefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-J', '-rational-rock', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_joliet_rr_and_eltorito_onefile)

def test_parse_joliet_rr_and_eltorito_onedir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolietrrandeltoritoonedir')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-J', '-rational-rock', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_joliet_rr_and_eltorito_onedir)

def test_parse_rr_deep_dir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rrdeep')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1').mkdir('dir2').mkdir('dir3').mkdir('dir4').mkdir('dir5').mkdir('dir6').mkdir('dir7').mkdir('dir8')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_rr_deep_dir)

def test_parse_rr_deep(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rrdeep')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1').mkdir('dir2').mkdir('dir3').mkdir('dir4').mkdir('dir5').mkdir('dir6').mkdir('dir7').mkdir('dir8')
    with open(os.path.join(str(indir), 'dir1', 'dir2', 'dir3', 'dir4', 'dir5', 'dir6', 'dir7', 'dir8', 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_rr_deep)

def test_parse_rr_deep2(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rrdeep')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1').mkdir('dir2').mkdir('dir3').mkdir('dir4').mkdir('dir5').mkdir('dir6').mkdir('dir7').mkdir('dir8').mkdir('dir9')
    with open(os.path.join(str(indir), 'dir1', 'dir2', 'dir3', 'dir4', 'dir5', 'dir6', 'dir7', 'dir8', 'dir9', 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_rr_deep2)

def test_parse_xa_nofiles(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('xa')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-xa', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_xa_nofiles)

def test_parse_xa_onefile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('xa')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-xa', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_xa_onefile)

def test_parse_xa_onedir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('xa')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-xa', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_xa_onedir)

def test_parse_sevendeepdirs(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('sevendeepdirs')
    outfile = str(indir)+'.iso'
    numdirs = 7
    x = indir
    for i in range(1, 1+numdirs):
        x = x.mkdir('dir%d' % i)
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_sevendeepdirs)

def test_parse_xa_joliet_nofiles(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('xajoliet')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-xa', '-J', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_xa_joliet_nofiles)

def test_parse_xa_joliet_onefile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('xajolietonefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-xa', '-J', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_xa_joliet_onefile)

def test_parse_xa_joliet_onedir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('xajolietonefile')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-xa', '-J', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_xa_joliet_onedir)

def test_parse_iso_level4_nofiles(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('isolevel4nofiles')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_isolevel4_nofiles)

def test_parse_iso_level4_onefile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('isolevel4onefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_isolevel4_onefile)

def test_parse_iso_level4_onedir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('isolevel4onedir')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_isolevel4_onedir)

def test_parse_iso_level4_eltorito(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('isolevel4eltorito')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_isolevel4_eltorito)

def test_parse_everything(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('everything')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1').mkdir('dir2').mkdir('dir3').mkdir('dir4').mkdir('dir5').mkdir('dir6').mkdir('dir7').mkdir('dir8')
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    with open(os.path.join(str(indir), 'dir1', 'dir2', 'dir3', 'dir4', 'dir5', 'dir6', 'dir7', 'dir8', 'bar'), 'wb') as outfp:
        outfp.write(b'bar\n')
    pwd = os.getcwd()
    os.chdir(str(indir))
    os.symlink('foo', 'sym')
    os.chdir(pwd)
    os.link(os.path.join(str(indir), 'foo'), os.path.join(str(indir), 'dir1', 'foo'))
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-J', '-rational-rock', '-xa', '-boot-info-table',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_everything)

def test_parse_rr_xa_nofiles(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('xarrnofiles')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-xa', '-rational-rock', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_rr_xa_nofiles)

def test_parse_rr_xa_onefile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('xarronefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-xa', '-rational-rock', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_rr_xa_onefile)

def test_parse_rr_xa_onedir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('xarronefile')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-xa', '-rational-rock', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_rr_xa_onedir)

def test_parse_rr_joliet_symlink(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rrsymlinkbroken')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    pwd = os.getcwd()
    os.chdir(str(indir))
    os.symlink('foo', 'sym')
    os.chdir(pwd)
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-J', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_rr_joliet_symlink)

def test_parse_rr_joliet_deep(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rrjolietdeep')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1').mkdir('dir2').mkdir('dir3').mkdir('dir4').mkdir('dir5').mkdir('dir6').mkdir('dir7').mkdir('dir8')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-J', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_rr_joliet_deep)

def test_parse_eltorito_multi_boot(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('multiboot')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    with open(os.path.join(str(indir), 'boot2'), 'wb') as outfp:
        outfp.write(b'boot2\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-b', 'boot', '-c', 'boot.cat', '-no-emul-boot',
                     '-eltorito-alt-boot', '-b', 'boot2', '-no-emul-boot',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_eltorito_multi_boot)

def test_parse_eltorito_boot_table(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('boottable')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-b', 'boot', '-c', 'boot.cat', '-no-emul-boot',
                     '-boot-info-table', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_eltorito_boot_info_table)

def test_parse_eltorito_boot_table_large(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('boottable')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot'*20)
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-b', 'boot', '-c', 'boot.cat', '-no-emul-boot',
                     '-boot-info-table', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_eltorito_boot_info_table_large)

def test_parse_hard_link(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('boottable')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    dir1 = indir.mkdir('dir1')
    os.link(os.path.join(str(indir), 'foo'), os.path.join(str(indir), str(dir1), 'foo'))
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_hard_link)

def test_parse_open_twice(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'f\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-o', str(outfile), str(indir)])

    iso = pycdlib.PyCdlib()
    iso.open(str(outfile))

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'This object already has an ISO; either close it or create a new object')

    iso.close()

def test_parse_get_and_write_fp_not_initialized(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'f\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-o', str(outfile), str(indir)])

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        iso.get_and_write_fp('/FOO.;1', open(os.path.join(str(tmpdir), 'bar'), 'w'))
    assert(str(excinfo.value) == 'This object is not initialized; call either open() or new() to create an ISO')

def test_parse_get_and_write_not_initialized(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'f\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-o', str(outfile), str(indir)])

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        iso.get_and_write('/FOO.;1', 'foo')
    assert(str(excinfo.value) == 'This object is not initialized; call either open() or new() to create an ISO')

def test_parse_write_not_initialized(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'f\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-o', str(outfile), str(indir)])

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        iso.write('out.iso')
    assert(str(excinfo.value) == 'This object is not initialized; call either open() or new() to create an ISO')

def test_parse_write_with_progress(tmpdir):
    test_parse_write_with_progress.num_progress_calls = 0
    test_parse_write_with_progress.done = 0
    def _progress(done, total):
        assert(total == 73728)
        test_parse_write_with_progress.num_progress_calls += 1
        test_parse_write_with_progress.done = done

    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'f\n')
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-rational-rock', '-J', '-o', str(outfile), str(indir)])

    iso = pycdlib.PyCdlib()
    iso.open(str(outfile))
    iso.write(str(tmpdir.join('writetest.iso')), progress_cb=_progress)

    assert(test_parse_write_with_progress.num_progress_calls == 16)
    assert(test_parse_write_with_progress.done == 73728)

    iso.close()

def test_parse_write_with_progress_three_arg(tmpdir):
    def _progress(done, total, opaque):
        assert(total == 73728)
        opaque['num_calls'] += 1
        opaque['done'] = done

    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'f\n')
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-rational-rock', '-J', '-o', str(outfile), str(indir)])

    iso = pycdlib.PyCdlib()
    iso.open(str(outfile))
    collect = {'num_calls': 0, 'done': 0}
    iso.write(str(tmpdir.join('writetest.iso')), progress_cb=_progress, progress_opaque=collect)

    assert(collect['num_calls'] == 16)
    assert(collect['done'] == 73728)

    iso.close()

def test_parse_get_entry(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('twofile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()
    iso.open(str(outfile))

    fooentry = iso.get_entry('/FOO.;1')
    assert(len(fooentry.children) == 0)
    assert(fooentry.isdir == False)
    assert(fooentry.is_root == False)
    assert(fooentry.file_ident == b'FOO.;1')
    assert(fooentry.dr_len == 40)
    assert(fooentry.extent_location() == 24)
    assert(fooentry.file_flags == 0)
    assert(fooentry.get_data_length() == 4)

    iso.close()

def test_parse_get_entry_not_initialized(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('twofile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        fooentry = iso.get_entry('/FOO.;1')
    assert(str(excinfo.value) == 'This object is not initialized; call either open() or new() to create an ISO')

def test_parse_list_dir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('twofile')
    outfile = str(indir)+'.iso'
    dir1 = indir.mkdir('dir1')
    with open(os.path.join(str(dir1), 'bar'), 'wb') as outfp:
        outfp.write(b'bar\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()
    iso.open(str(outfile))

    for children in iso.list_dir('/DIR1'):
        pass

    iso.close()

def test_parse_list_dir_not_initialized(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('twofile')
    outfile = str(indir)+'.iso'
    dir1 = indir.mkdir('dir1')
    with open(os.path.join(str(dir1), 'bar'), 'wb') as outfp:
        outfp.write(b'bar\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        for children in iso.list_dir('/DIR1'):
            pass
    assert(str(excinfo.value) == 'This object is not initialized; call either open() or new() to create an ISO')

def test_parse_list_dir_not_dir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('twofile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        for children in iso.list_dir('/FOO.;1'):
            pass
    assert(str(excinfo.value) == 'Record is not a directory!')

    iso.close()

def test_parse_get_and_write(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'f\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-o', str(outfile), str(indir)])

    iso = pycdlib.PyCdlib()
    iso.open(str(outfile))

    foofile = os.path.join(str(indir), 'foo')
    iso.get_and_write('/foo', foofile)

    iso.close()

    with open(foofile, 'r') as infp:
        assert(infp.read() == 'f\n')

def test_parse_open_fp_twice(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-o', str(outfile), str(indir)])

    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        with open(str(outfile), 'rb') as infp:
            iso.open_fp(infp)
    assert(str(excinfo.value) == 'This object already has an ISO; either close it or create a new object')

    iso.close()

def test_parse_open_fp_seek(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-o', str(outfile), str(indir)])

    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    with iso.open_file_from_iso(iso_path='/foo') as infp:
        assert(infp.tell() == 0)
        assert(infp.readall() == b'foo\n')
        assert(infp.seek(2) == 2)
        assert(infp.tell() == 2)
        assert(infp.readall() == b'o\n')
        assert(infp.seek(-2, whence=2) == 2)
        assert(infp.tell() == 2)
        assert(infp.readall() == b'o\n')

    iso.close()

def test_parse_open_invalid_vd(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and perturb the first
    # byte.  This should be enough to make an invalid ISO.
    with open(str(outfile), 'r+b') as fp:
        fp.seek(16*2048)
        fp.write(b'\xF4')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'Valid ISO9660 filesystems must have at least one PVD')

def test_parse_same_dirname_different_parent(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('samedirnamedifferentparent')
    outfile = str(indir)+'.iso'
    dir1 = indir.mkdir('dir1')
    dir2 = indir.mkdir('dir2')
    boot1 = dir1.mkdir('boot')
    boot2 = dir2.mkdir('boot')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-rational-rock',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_same_dirname_different_parent)

def test_parse_joliet_iso_level_4(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolietisolevel4')
    outfile = str(indir)+'.iso'
    dir1 = indir.mkdir('dir1')
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-J', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_joliet_isolevel4)

def test_parse_eltorito_nofiles_hide(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('eltoritonofiles')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-hide', 'boot.cat',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_eltorito_nofiles_hide)

def test_parse_eltorito_nofiles_hide_joliet(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('eltoritonofiles')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-J', '-hide', 'boot.cat', '-hide-joliet', 'boot.cat',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_joliet_and_eltorito_nofiles_hide)

def test_parse_eltorito_nofiles_hide_joliet_only(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('eltoritonofiles')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-J', '-hide-joliet', 'boot.cat',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_joliet_and_eltorito_nofiles_hide_only)

def test_parse_eltorito_nofiles_hide_iso_only_joliet(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('eltoritonofiles')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-J', '-hide', 'boot.cat',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_joliet_and_eltorito_nofiles_hide_iso_only)

def test_parse_hard_link_reshuffle(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('boottable')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    os.link(os.path.join(str(indir), 'foo'), os.path.join(str(indir), 'bar'))
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_hard_link_reshuffle)

def test_parse_open_invalid_pvd_ident(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and perturb the first
    # byte.  This should be enough to make an invalid ISO.
    with open(str(outfile), 'r+b') as fp:
        fp.seek((16*2048)+5)
        fp.write(b'\x02')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'Valid ISO9660 filesystems must have at least one PVD')

def test_parse_open_invalid_pvd_version(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and perturb the first
    # byte.  This should be enough to make an invalid ISO.
    with open(str(outfile), 'r+b') as fp:
        fp.seek((16*2048)+6)
        fp.write(b'\x02')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'Invalid volume descriptor version 2')

def test_parse_open_invalid_pvd_unused1(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and perturb the first
    # byte.  This should be enough to make an invalid ISO.
    with open(str(outfile), 'r+b') as fp:
        fp.seek((16*2048)+7)
        fp.write(b'\x02')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'PVD flags field is not zero')

def test_parse_open_invalid_pvd_unused2(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and perturb the first
    # byte.  This should be enough to make an invalid ISO.
    with open(str(outfile), 'r+b') as fp:
        fp.seek((16*2048)+72)
        fp.write(b'\x02')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'data in 2nd unused field not zero')

def test_parse_open_invalid_pvd_unused4(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and perturb the first
    # byte.  This should be enough to make an invalid ISO.
    with open(str(outfile), 'r+b') as fp:
        fp.seek((16*2048)+882)
        fp.write(b'\x02')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'data in 2nd unused field not zero')

def test_parse_open_invalid_pvd_unused5(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and perturb the last
    # byte of the PVD.  According to Ecma-119 this is invalid, but we have
    # seen ISOs in the wild where there is something other than 0 here, so
    # we allow it.
    with open(str(outfile), 'r+b') as fp:
        fp.seek((17*2048)-1)
        fp.write(b'\x02')

    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))
    iso.close()

def test_parse_invalid_pvd_space_size_le_be_mismatch(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and perturb the first
    # byte.  This should be enough to make an invalid ISO.
    with open(str(outfile), 'r+b') as fp:
        fp.seek((16*2048)+84)
        fp.write(b'\x00\x00\x00\x00')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'Little-endian and big-endian space size disagree')

def test_parse_invalid_pvd_set_size_le_be_mismatch(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and perturb the first
    # byte.  This should be enough to make an invalid ISO.
    with open(str(outfile), 'r+b') as fp:
        fp.seek((16*2048)+122)
        fp.write(b'\x00\x44')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'Little-endian and big-endian set size disagree')

def test_parse_invalid_pvd_seqnum_le_be_mismatch(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and perturb the first
    # byte.  This should be enough to make an invalid ISO.
    with open(str(outfile), 'r+b') as fp:
        fp.seek((16*2048)+126)
        fp.write(b'\x00\x44')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'Little-endian and big-endian seqnum disagree')

def test_parse_invalid_pvd_lb_le_be_mismatch(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and perturb the first
    # byte.  This should be enough to make an invalid ISO.
    with open(str(outfile), 'r+b') as fp:
        fp.seek((16*2048)+130)
        fp.write(b'\x00\x01')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'Little-endian and big-endian logical block size disagree')

def test_parse_invalid_pvd_ptr_size_le_be_mismatch(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and perturb the first
    # byte.  This should be enough to make an invalid ISO.
    with open(str(outfile), 'r+b') as fp:
        fp.seek((16*2048)+136)
        fp.write(b'\x00\x01\x00\x00')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'Little-endian and big-endian path table size disagree')

def test_parse_open_invalid_vdst_ident(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and perturb the first
    # byte.  This should be enough to make an invalid ISO.
    with open(str(outfile), 'r+b') as fp:
        fp.seek((17*2048)+5)
        fp.write(b'\x02')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'Valid ISO9660 filesystems must have at least one Volume Descriptor Set Terminator')

def test_parse_open_invalid_vdst_version(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and perturb the first
    # byte.  This should be enough to make an invalid ISO.
    with open(str(outfile), 'r+b') as fp:
        fp.seek((17*2048)+6)
        fp.write(b'\x02')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'Invalid VDST version')

def test_parse_invalid_br_ident(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('eltoritonofiles')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and perturb the first
    # byte.  This should be enough to make an invalid ISO.
    with open(str(outfile), 'r+b') as fp:
        fp.seek((17*2048)+5)
        fp.write(b'\x02')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    # The error is a bit odd, but correct; since we can't identify the record,
    # we have to assume it is the end of volume descriptors (there is no other
    # definitive marker).  We don't find out that this was a problem until we
    # see that we didn't parse the VDST.
    assert(str(excinfo.value) == 'Valid ISO9660 filesystems must have at least one Volume Descriptor Set Terminator')

def test_parse_invalid_br_version(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('eltoritonofiles')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and perturb the first
    # byte.  This should be enough to make an invalid ISO.
    with open(str(outfile), 'r+b') as fp:
        fp.seek((17*2048)+6)
        fp.write(b'\x02')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'Invalid boot record version')

def test_parse_open_invalid_svd_ident(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and perturb the first
    # byte.  This should be enough to make an invalid ISO.
    with open(str(outfile), 'r+b') as fp:
        fp.seek((17*2048)+5)
        fp.write(b'\x02')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    # The error is a bit odd, but correct; since we can't identify the record,
    # we have to assume it is the end of volume descriptors (there is no other
    # definitive marker).  We don't find out that this was a problem until we
    # see that we didn't parse the VDST.
    assert(str(excinfo.value) == 'Valid ISO9660 filesystems must have at least one Volume Descriptor Set Terminator')

def test_parse_open_invalid_svd_version(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and perturb the first
    # byte.  This should be enough to make an invalid ISO.
    with open(str(outfile), 'r+b') as fp:
        fp.seek((17*2048)+6)
        fp.write(b'\x03')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'Invalid volume descriptor version 3')

def test_parse_open_invalid_svd_unused1(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and perturb the first
    # byte.  This should be enough to make an invalid ISO.
    with open(str(outfile), 'r+b') as fp:
        fp.seek((17*2048)+72)
        fp.write(b'\x02')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'data in 2nd unused field not zero')

def test_parse_open_invalid_svd_file_structure_version(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and perturb the first
    # byte.  This should be enough to make an invalid ISO.
    with open(str(outfile), 'r+b') as fp:
        fp.seek((17*2048)+881)
        fp.write(b'\x03')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'File structure version expected to be 1')

def test_parse_open_invalid_svd_unused2(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and perturb the first
    # byte.  This should be enough to make an invalid ISO.
    with open(str(outfile), 'r+b') as fp:
        fp.seek((17*2048)+882)
        fp.write(b'\x02')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'data in 2nd unused field not zero')

def test_parse_invalid_svd_space_size_le_be_mismatch(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and perturb the first
    # byte.  This should be enough to make an invalid ISO.
    with open(str(outfile), 'r+b') as fp:
        fp.seek((17*2048)+84)
        fp.write(b'\x00\x00\x00\x00')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'Little-endian and big-endian space size disagree')

def test_parse_invalid_svd_set_size_le_be_mismatch(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and perturb the first
    # byte.  This should be enough to make an invalid ISO.
    with open(str(outfile), 'r+b') as fp:
        fp.seek((17*2048)+122)
        fp.write(b'\x00\x44')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'Little-endian and big-endian set size disagree')

def test_parse_invalid_svd_seqnum_le_be_mismatch(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and perturb the first
    # byte.  This should be enough to make an invalid ISO.
    with open(str(outfile), 'r+b') as fp:
        fp.seek((17*2048)+126)
        fp.write(b'\x00\x44')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'Little-endian and big-endian seqnum disagree')

def test_parse_invalid_svd_lb_le_be_mismatch(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and perturb the first
    # byte.  This should be enough to make an invalid ISO.
    with open(str(outfile), 'r+b') as fp:
        fp.seek((17*2048)+130)
        fp.write(b'\x00\x01')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'Little-endian and big-endian logical block size disagree')

def test_parse_invalid_svd_ptr_size_le_be_mismatch(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and perturb the first
    # byte.  This should be enough to make an invalid ISO.
    with open(str(outfile), 'r+b') as fp:
        fp.seek((17*2048)+136)
        fp.write(b'\x00\x01\x00\x00')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'Little-endian and big-endian path table size disagree')

def test_parse_iso_too_small(tmpdir):
    indir = tmpdir.mkdir('isotoosmall')
    outfile = str(indir)+'.iso'
    with open(outfile, 'wb') as outfp:
        outfp.write(b'\x00'*16*2048)

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'Failed to read entire volume descriptor')

def test_parse_rr_deeper_dir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rrdeep')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1').mkdir('dir2').mkdir('dir3').mkdir('dir4').mkdir('dir5').mkdir('dir6').mkdir('dir7').mkdir('dir8')
    indir.mkdir('a1').mkdir('a2').mkdir('a3').mkdir('a4').mkdir('a5').mkdir('a6').mkdir('a7').mkdir('a8')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_rr_deeper_dir)

def test_parse_eltorito_boot_table_odd(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('boottable')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boo'*27)
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-b', 'boot', '-c', 'boot.cat', '-no-emul-boot',
                     '-boot-info-table', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_eltorito_boot_info_table_large_odd)

def test_parse_joliet_large_directory(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolietlargedirectory')
    outfile = str(indir)+'.iso'
    for i in range(1, 50):
        indir.mkdir('dir' + str(i))
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_joliet_large_directory)

def test_parse_zero_byte_file(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('zerobytefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        pass
    with open(os.path.join(str(indir), 'bar'), 'wb') as outfp:
        outfp.write(b'bar\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_zero_byte_file)

def test_parse_dirrecord_too_short(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('tooshort')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    with open(outfile, 'a+b') as editfp:
        os.ftruncate(editfp.fileno(), 47104)

    iso = pycdlib.PyCdlib()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'Invalid directory record')

def test_parse_eltorito_hide_boot(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('eltoritohideboot')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-hide', 'boot',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_eltorito_hide_boot)

def test_parse_get_entry_joliet(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('getentryjoliet')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()
    iso.open(str(outfile))

    fooentry = iso.get_entry('/foo', joliet=True)
    assert(len(fooentry.children) == 0)
    assert(fooentry.isdir == False)
    assert(fooentry.is_root == False)
    assert(fooentry.file_ident == 'foo'.encode('utf-16_be'))
    assert(fooentry.dr_len == 40)
    assert(fooentry.extent_location() == 30)
    assert(fooentry.file_flags == 0)
    assert(fooentry.get_data_length() == 4)

    iso.close()

def test_parse_dirrecord_nonzero_pad(tmpdir):
    indir = tmpdir.mkdir('dirrecordnonzeropad')
    outfile = str(indir) + '.iso'

    for d in range(0, 53):
        indir.mkdir('dir%d' % d)
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    with open(outfile, 'r+b') as changefp:
        changefp.seek(24*2048 - 1)
        changefp.write(b'\xff')

    iso = pycdlib.PyCdlib()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'Invalid padding on ISO')

def test_parse_open_invalid_eltorito_header_id(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and perturb the El Torito
    # header ID (extent 25).  This should be enough to make an invalid ISO.
    with open(str(outfile), 'r+b') as fp:
        fp.seek(25*2048 + 0)
        fp.write(b'\xF4')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'El Torito Validation entry header ID not 1')

def test_parse_open_invalid_eltorito_platform_id(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and perturb the El Torito
    # header ID (extent 25).  This should be enough to make an invalid ISO.
    with open(str(outfile), 'r+b') as fp:
        fp.seek(25*2048 + 1)
        fp.write(b'\xF4')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'El Torito Validation entry platform ID not valid')

def test_parse_open_invalid_eltorito_first_key_byte(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and perturb the El Torito
    # header ID (extent 25).  This should be enough to make an invalid ISO.
    with open(str(outfile), 'r+b') as fp:
        fp.seek(25*2048 + 0x1e)
        fp.write(b'\xF4')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'El Torito Validation entry first keybyte not 0x55')

def test_parse_open_invalid_eltorito_second_key_byte(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and perturb the El Torito
    # header ID (extent 25).  This should be enough to make an invalid ISO.
    with open(str(outfile), 'r+b') as fp:
        fp.seek(25*2048 + 0x1f)
        fp.write(b'\xF4')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'El Torito Validation entry second keybyte not 0xaa')

def test_parse_open_invalid_eltorito_csum(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and perturb the El Torito
    # header ID (extent 25).  This should be enough to make an invalid ISO.
    with open(str(outfile), 'r+b') as fp:
        fp.seek(25*2048 + 0x1c)
        fp.write(b'\x00')
        fp.write(b'\x00')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'El Torito Validation entry checksum not correct')

def test_parse_hidden_file(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('onefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'aaaaaaaa'), 'wb') as outfp:
        outfp.write(b'aa\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-hidden', 'aaaaaaaa', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_hidden_file)

def test_parse_hidden_dir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('onedir')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-hidden', 'dir1', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_hidden_dir)

def test_parse_eltorito_bad_boot_indicator(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('eltoritonofiles')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-o', str(outfile), str(indir)])

    # Now that we have the ISO, perturb the initial entry
    with open(str(outfile), 'r+b') as fp:
        fp.seek(25*2048+32)
        fp.write(b'\xF4')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'Invalid El Torito initial entry boot indicator')

def test_parse_eltorito_bad_boot_media(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('eltoritonofiles')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-o', str(outfile), str(indir)])

    # Now that we have the ISO, perturb the initial entry
    with open(str(outfile), 'r+b') as fp:
        fp.seek(25*2048+33)
        fp.write(b'\xF4')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'Invalid El Torito boot media type')

def test_parse_eltorito_bad_unused(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('eltoritonofiles')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-o', str(outfile), str(indir)])

    # Now that we have the ISO, perturb the initial entry
    with open(str(outfile), 'r+b') as fp:
        fp.seek(25*2048+37)
        fp.write(b'\xF4')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'El Torito unused field must be 0')

def test_parse_eltorito_hd_emul(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('eltoritonofiles')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'\x00'*446 + b'\x00\x01\x01\x00\x02\x01\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*16 + b'\x00'*16 + b'\x00'*16 + b'\x55' + b'\xaa')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-hard-disk-boot',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_eltorito_hd_emul)

def test_parse_eltorito_hd_emul_not_bootable(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('eltoritonofiles')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'\x00'*446 + b'\x00\x01\x01\x00\x02\x01\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00' + b'\x00'*16 + b'\x00'*16 + b'\x00'*16 + b'\x55' + b'\xaa')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-hard-disk-boot', '-no-boot',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_eltorito_hd_emul_not_bootable)

def test_parse_eltorito_floppy12(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('eltoritonofiles')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'\x00'*(2400*512))
    # If you don't pass -hard-disk-boot or -no-emul-boot to genisoimage,
    # it assumes floppy.
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_eltorito_floppy12)

def test_parse_eltorito_floppy144(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('eltoritonofiles')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'\x00'*(2880*512))
    # If you don't pass -hard-disk-boot or -no-emul-boot to genisoimage,
    # it assumes floppy.
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_eltorito_floppy144)

def test_parse_eltorito_floppy288(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('eltoritonofiles')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'\x00'*(5760*512))
    # If you don't pass -hard-disk-boot or -no-emul-boot to genisoimage,
    # it assumes floppy.
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_eltorito_floppy288)

def test_parse_ptr_le_and_be_disagree(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('eltoritonofiles')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and perturb the first
    # byte of the Big Endian PTR.  This should make open_fp() fail.
    with open(str(outfile), 'r+b') as fp:
        fp.seek(21*2048)
        fp.write(b'\xF4')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'Little-endian and big-endian path table records do not agree')

def test_parse_joliet_ptr_le_and_be_disagree(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('eltoritonofiles')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-iso-level', '1', '-no-pad', '-J',
                     '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and perturb the first
    # byte of the Joliet Big Endian PTR.  This should make open_fp() fail.
    with open(str(outfile), 'r+b') as fp:
        fp.seek(24*2048)
        fp.write(b'\xF4')

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidISO) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'Joliet little-endian and big-endian path table records do not agree')

def test_parse_add_file_with_semicolon(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('onefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'FOO;1'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-relaxed-filenames', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_onefile_with_semicolon)

def test_parse_bad_eltorito_ident(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('eltoritonofiles')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-o', str(outfile), str(indir)])

    with open(str(outfile), 'r+') as outfp:
        # Change the EL TORITO SPECIFICATION string to be something else
        outfp.seek(17*2048+7)
        outfp.write('Z')

    do_a_test(tmpdir, outfile, check_bad_eltorito_ident)

def test_parse_duplicate_rrmoved_name(tmpdir):
    iso = pycdlib.PyCdlib()
    iso.new(rock_ridge='1.09')

    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('eltoritonofiles')
    outfile = str(indir)+'.iso'
    fdir = indir.mkdir('A').mkdir('B').mkdir('C').mkdir('D').mkdir('E').mkdir('F')
    fdir.mkdir('G').mkdir('1')
    fdir.mkdir('H').mkdir('1')
    with open(os.path.join(str(indir), 'A', 'B', 'C', 'D', 'E', 'F', 'G', '1', 'first'), 'wb') as outfp:
        outfp.write(b'first\n')
    with open(os.path.join(str(indir), 'A', 'B', 'C', 'D', 'E', 'F', 'H', '1', 'second'), 'wb') as outfp:
        outfp.write(b'second\n')

    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_rr_two_dirs_same_level)

def test_parse_eltorito_rr_verylongname(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rrverylongname')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'a'*RR_MAX_FILENAME_LENGTH, '-b', 'boot', '-no-emul-boot',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_eltorito_rr_verylongname)

@pytest.mark.skipif(find_executable('isohybrid') is None,
                    reason='syslinux not installed')
def test_parse_isohybrid_file_before(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('isohybrid')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'isolinux.bin'), 'wb') as outfp:
        outfp.seek(0x40)
        outfp.write(b'\xfb\xc0\x78\x70')
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'isolinux.bin', '-no-emul-boot',
                     '-boot-load-size', '4',
                     '-o', str(outfile), str(indir)])
    subprocess.call(['isohybrid', '-v', str(outfile)])

    do_a_test(tmpdir, outfile, check_isohybrid_file_before)

def test_parse_eltorito_rr_joliet_verylongname(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rrjolietverylongname')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'a'*RR_MAX_FILENAME_LENGTH, '-b', 'boot', '-no-emul-boot',
                     '-rational-rock', '-J', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_eltorito_rr_joliet_verylongname)

def test_parse_joliet_dirs_overflow_ptr_extent(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolietmanydirs')
    outfile = str(indir)+'.iso'
    numdirs = 216
    for i in range(1, 1+numdirs):
        indir.mkdir('dir%d' % i)
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_joliet_dirs_overflow_ptr_extent)

def test_parse_joliet_dirs_just_short_ptr_extent(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolietjustshortdirs')
    outfile = str(indir)+'.iso'
    numdirs = 215
    for i in range(1, 1+numdirs):
        indir.mkdir('dir%d' % i)
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_joliet_dirs_just_short_ptr_extent)

def test_parse_joliet_dirs_add_ptr_extent(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolietjustshortdirs')
    outfile = str(indir)+'.iso'
    numdirs = 295
    for i in range(1, 1+numdirs):
        indir.mkdir('dir%d' % i)
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_joliet_dirs_add_ptr_extent)

def test_parse_joliet_dirs_rm_ptr_extent(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolietjustshortdirs')
    outfile = str(indir)+'.iso'
    numdirs = 293
    for i in range(1, 1+numdirs):
        indir.mkdir('dir%d' % i)
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_joliet_dirs_rm_ptr_extent)

def test_parse_long_directory_name(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('longdirectoryname')
    outfile = str(indir)+'.iso'
    indir.mkdir('directory1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '3', '-no-pad',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_long_directory_name)

def test_parse_long_file_name(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('longdirectoryname')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foobarbaz1'), 'wb') as outfp:
        outfp.write(b'foobarbaz1\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '3', '-no-pad',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_long_file_name)

def test_parse_overflow_root_dir_record(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('overflowrootdirrecord')
    outfile = str(indir)+'.iso'
    for letter in ('a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'j', 'k', 'l', 'm', 'n', 'o'):
        with open(os.path.join(str(indir), letter*20), 'wb') as outfp:
            outfp.write(b'\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-J', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_overflow_root_dir_record)

def test_parse_duplicate_deep_dir(tmpdir):
    indir = tmpdir.mkdir('duplicatedeepdir')
    outfile = str(indir)+'.iso'

    get = indir.mkdir('books').mkdir('lkhg').mkdir('HyperNews').mkdir('get')
    get.mkdir('fs').mkdir('fs').mkdir('1')
    khg = get.mkdir('khg')
    khg.mkdir('1')
    khg.mkdir('117').mkdir('1').mkdir('1').mkdir('1').mkdir('1')
    khg.mkdir('35').mkdir('1').mkdir('1')

    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-J', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_duplicate_deep_dir)

def test_parse_no_joliet_name(tmpdir):
    indir = tmpdir.mkdir('nojolietname')
    outfile = str(indir)+'.iso'

    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')

    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-hide-joliet', 'foo', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_onefile_joliet_no_file)

def test_parse_joliet_isolevel4_nofiles(tmpdir):
    indir = tmpdir.mkdir('jolietisolevel4nofiles')
    outfile = str(indir)+'.iso'

    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-J', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_joliet_isolevel4_nofiles)

def test_parse_deep_rr_symlink(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('sevendeepdirs')
    outfile = str(indir)+'.iso'
    dir7 = indir.mkdir('dir1').mkdir('dir2').mkdir('dir3').mkdir('dir4').mkdir('dir5').mkdir('dir6').mkdir('dir7')
    pwd = os.getcwd()
    os.chdir(str(dir7))
    os.symlink('/usr/share/foo', 'sym')
    os.chdir(pwd)
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_deep_rr_symlink)

def test_parse_rr_deep_weird_layout(tmpdir):
    indir = tmpdir.mkdir('rrdeepweird')
    outfile = str(indir) + '.iso'
    absimp = indir.mkdir('astroid').mkdir('astroid').mkdir('tests').mkdir('testdata').mkdir('python3').mkdir('data').mkdir('absimp')
    sidepackage = absimp.mkdir('sidepackage')
    with open(os.path.join(str(absimp), 'string.py'), 'wb') as outfp:
        outfp.write(b'from __future__ import absolute_import, print_functino\nimport string\nprint(string)\n')
    with open(os.path.join(str(sidepackage), '__init__.py'), 'wb') as outfp:
        outfp.write(b'"""a side package with nothing in it\n"""\n')

    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_rr_deep_weird_layout)

def test_parse_rr_long_dir_name(tmpdir):
    indir = tmpdir.mkdir('rrlongdirname')
    outfile = str(indir) + '.iso'
    indir.mkdir('a'*RR_MAX_FILENAME_LENGTH)
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_rr_long_dir_name)

def test_parse_rr_hidden_relocated(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rrdeep')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1').mkdir('dir2').mkdir('dir3').mkdir('dir4').mkdir('dir5').mkdir('dir6').mkdir('dir7').mkdir('dir8').mkdir('dir9')
    with open(os.path.join(str(indir), 'dir1', 'dir2', 'dir3', 'dir4', 'dir5', 'dir6', 'dir7', 'dir8', 'dir9', 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-hide-rr-moved', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_rr_relocated_hidden)

def test_parse_open_fp_not_binary(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rrdeep')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        with open(str(outfile), 'r') as infp:
            iso.open_fp(infp)
    assert(str(excinfo.value) == "The file to open must be in binary mode (add 'b' to the open flags)")

def test_parse_open_too_small_pvd(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('toosmallpvd')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and shrink the volume
    # size to be too small.  PyCdlib should fix it at open time.
    with open(str(outfile), 'r+b') as fp:
        fp.seek(16*2048+0x50)
        fp.write(b'\x16\x00\x00\x00\x00\x00\x00\x16')

    do_a_test(tmpdir, outfile, check_onefile)

def test_parse_open_too_small_joliet(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('toosmalljoliet')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-no-pad', '-J',
                     '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and shrink the volume
    # size to be too small.  PyCdlib should fix it at open time.
    with open(str(outfile), 'r+b') as fp:
        fp.seek(16*2048+0x50)
        fp.write(b'\x16\x00\x00\x00\x00\x00\x00\x16')

    do_a_test(tmpdir, outfile, check_joliet_onefile)

def test_parse_open_too_small_isolevel4(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('toosmallisolevel4')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-no-pad', '-iso-level', '4',
                     '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and shrink the volume
    # size to be too small.  PyCdlib should fix it at open time.
    with open(str(outfile), 'r+b') as fp:
        fp.seek(16*2048+0x50)
        fp.write(b'\x16\x00\x00\x00\x00\x00\x00\x16')

    do_a_test(tmpdir, outfile, check_isolevel4_onefile)

def test_parse_open_larger_than_iso(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('largerthaniso')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-no-pad', '-iso-level', '1',
                     '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and make the file too
    # large.  PyCdlib should fix it at open time.
    with open(str(outfile), 'r+b') as fp:
        fp.seek(23*2048+0x4e)
        fp.write(b'\x01\x08\x00\x00\x00\x00\x08\x01')

    do_a_test(tmpdir, outfile, check_onefile_toolong)

def test_parse_pvd_zero_datetime(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('zerodatetimeiso')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-no-pad', '-iso-level', '1',
                     '-o', str(outfile), str(indir)])

    with open(str(outfile), 'r+b') as fp:
        fp.seek(16*2048 + 813)
        fp.write(b'\x00'*17)

    do_a_test(tmpdir, outfile, check_pvd_zero_datetime)

def test_parse_pvd_zero_digit_datetime(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('zerodatetimeiso')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-no-pad', '-iso-level', '1',
                     '-o', str(outfile), str(indir)])

    with open(str(outfile), 'r+b') as fp:
        fp.seek(16*2048 + 813)
        fp.write(b'0'*17)

    do_a_test(tmpdir, outfile, check_pvd_zero_datetime)

def test_parse_pvd_zero_digit_datetime_zero_tz(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('zerodatetimeiso')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-no-pad', '-iso-level', '1',
                     '-o', str(outfile), str(indir)])

    with open(str(outfile), 'r+b') as fp:
        fp.seek(16*2048 + 813)
        fp.write(b'0'*16)
        fp.write(b'\x00')

    do_a_test(tmpdir, outfile, check_pvd_zero_datetime)

def test_parse_pvd_invalid_year(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('zerodatetimeiso')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-no-pad', '-iso-level', '1',
                     '-o', str(outfile), str(indir)])

    with open(str(outfile), 'r+b') as fp:
        fp.seek(16*2048 + 813)
        fp.write(b'0'*4)

    do_a_test(tmpdir, outfile, check_pvd_zero_datetime)

def test_parse_bad_root_dir_ident(tmpdir):
    indir = tmpdir.mkdir('badrootdirident')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-no-pad', '-iso-level', '1',
                     '-o', str(outfile), str(indir)])

    with open(str(outfile), 'r+b') as fp:
        fp.seek(16*2048 + 156 + 33)
        fp.write(b'\x01')

    do_a_test(tmpdir, outfile, check_nofiles)

def test_parse_bad_file_structure_version(tmpdir):
    indir = tmpdir.mkdir('badfilestructureversion')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-no-pad', '-iso-level', '1',
                     '-o', str(outfile), str(indir)])

    with open(str(outfile), 'r+b') as fp:
        fp.seek(16*2048 + 881)
        fp.write(b'\x02')

    do_a_test(tmpdir, outfile, check_nofiles)

def test_parse_get_file_from_iso_not_initialized(tmpdir):
    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        iso.get_file_from_iso('junk')
    assert(str(excinfo.value) == 'This object is not initialized; call either open() or new() to create an ISO')

def test_parse_get_file_from_iso(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('onefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    iso = pycdlib.PyCdlib()
    iso.open(str(outfile))

    foofile = os.path.join(str(indir), 'foo')
    iso.get_file_from_iso(foofile, iso_path='/FOO.;1')

    iso.close()

    with open(foofile, 'r') as infp:
        assert(infp.read() == 'foo\n')

def test_parse_joliet_encoded_system_identifier(tmpdir):
    indir = tmpdir.mkdir('jolietsysident')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'user-data'), 'wb') as outfp:
        outfp.write(b'''\
#cloud-config
password: password
chpasswd: { expire: False }
ssh_pwauth: True
''')

    with open(os.path.join(str(indir), 'meta-data'), 'wb') as outfp:
        outfp.write(b'''\
local-hostname: cloudimg
''')

    subprocess.call(['genisoimage', '-v', '-v', '-no-pad', '-iso-level', '4',
                     '-J', '-rational-rock', '-sysid', 'LINUX', '-volid', 'cidata',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_joliet_ident_encoding)

def test_parse_joliet_hidden_iso_file(tmpdir):
    indir = tmpdir.mkdir('joliethiddeniso')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')

    subprocess.call(['genisoimage', '-v', '-v', '-no-pad', '-iso-level', '1',
                     '-J', '-hide', 'foo',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_joliet_hidden_iso_file)

def test_parse_udf_nofiles(tmpdir):
    indir = tmpdir.mkdir('udfnofiles')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-no-pad', '-iso-level', '1',
                     '-udf', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_udf_nofiles)

def test_parse_udf_onedir(tmpdir):
    indir = tmpdir.mkdir('udfonedir')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-no-pad', '-iso-level', '1',
                     '-udf', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_udf_onedir)

def test_parse_udf_twodirs(tmpdir):
    indir = tmpdir.mkdir('udftwodirs')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1')
    indir.mkdir('dir2')
    subprocess.call(['genisoimage', '-v', '-v', '-no-pad', '-iso-level', '1',
                     '-udf', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_udf_twodirs)

def test_parse_udf_subdir(tmpdir):
    indir = tmpdir.mkdir('udfsubdir')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1').mkdir('subdir1')
    subprocess.call(['genisoimage', '-v', '-v', '-no-pad', '-iso-level', '1',
                     '-udf', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_udf_subdir)

def test_parse_udf_subdir_odd(tmpdir):
    indir = tmpdir.mkdir('udfsubdir')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1').mkdir('subdi1')
    subprocess.call(['genisoimage', '-v', '-v', '-no-pad', '-iso-level', '1',
                     '-udf', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_udf_subdir_odd)

def test_parse_udf_onefile(tmpdir):
    indir = tmpdir.mkdir('udfonefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')

    subprocess.call(['genisoimage', '-v', '-v', '-no-pad', '-iso-level', '1',
                     '-udf', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_udf_onefile)

def test_parse_udf_onefileonedir(tmpdir):
    indir = tmpdir.mkdir('udfonefileonedir')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1')
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')

    subprocess.call(['genisoimage', '-v', '-v', '-no-pad', '-iso-level', '1',
                     '-udf', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_udf_onefileonedir)

def test_parse_udf_dir_spillover(tmpdir):
    indir = tmpdir.mkdir('udfdirspillover')
    outfile = str(indir)+'.iso'
    for i in range(ord('a'), ord('v')):
        dirname = chr(i) * 64
        indir.mkdir(dirname)

    subprocess.call(['genisoimage', '-v', '-v', '-no-pad', '-iso-level', '1',
                     '-udf', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_udf_dir_spillover)

def test_parse_udf_dir_oneshort(tmpdir):
    indir = tmpdir.mkdir('udfdironeshort')
    outfile = str(indir)+'.iso'
    for i in range(ord('a'), ord('u')):
        dirname = chr(i) * 64
        indir.mkdir(dirname)

    subprocess.call(['genisoimage', '-v', '-v', '-no-pad', '-iso-level', '1',
                     '-udf', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_udf_dir_oneshort)

def test_parse_udf_iso_hidden(tmpdir):
    indir = tmpdir.mkdir('udfisohidden')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')

    subprocess.call(['genisoimage', '-v', '-v', '-no-pad', '-iso-level', '1',
                     '-udf', '-hide', 'foo', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_udf_iso_hidden)

@pytest.mark.slow
def test_parse_udf_very_large(tmpdir):
    indir = tmpdir.mkdir('udfverylarge')
    outfile = str(indir)+'.iso'
    largefile = os.path.join(str(indir), 'foo')
    with open(largefile, 'wb') as outfp:
        outfp.truncate(1073739776+1)

    subprocess.call(['genisoimage', '-v', '-v', '-no-pad', '-iso-level', '1',
                     '-udf', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_udf_very_large)

def test_parse_joliet_udf_nofiles(tmpdir):
    indir = tmpdir.mkdir('jolietudfnofiles')
    outfile = str(indir)+'.iso'

    subprocess.call(['genisoimage', '-v', '-v', '-no-pad', '-iso-level', '1',
                     '-J', '-udf', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_joliet_udf_nofiles)

def test_parse_udf_dir_exactly2048(tmpdir):
    indir = tmpdir.mkdir('udfdirspillover')
    outfile = str(indir)+'.iso'
    indir.mkdir('a' * 248)
    indir.mkdir('b' * 248)
    indir.mkdir('c' * 248)
    indir.mkdir('d' * 248)
    indir.mkdir('e' * 248)
    indir.mkdir('f' * 248)
    indir.mkdir('g' * 240)

    subprocess.call(['genisoimage', '-v', '-v', '-no-pad', '-iso-level', '1',
                     '-udf', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_udf_dir_exactly2048)

def test_parse_udf_overflow_dir_extent(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('udfoverflow')
    outfile = str(indir)+'.iso'
    numdirs = 46
    for i in range(1, 1+numdirs):
        indir.mkdir('dir%d' % i)
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-udf', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_udf_overflow_dir_extent)

def test_parse_multi_hard_link(tmpdir):
    indir = tmpdir.mkdir('jolietudfnofiles')
    outfile = str(indir)+'.iso'

    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    pwd = os.getcwd()
    os.chdir(str(indir))
    os.link('foo', 'bar')
    os.link('bar', 'baz')
    os.chdir(pwd)

    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-cache-inodes', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_multi_hard_link)

def test_parse_udf_joliet_onefile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolietfile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-udf', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_udf_joliet_onefile)

def test_parse_zero_byte_hard_link(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('boottable')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        pass
    os.link(os.path.join(str(indir), 'foo'), os.path.join(str(indir), 'bar'))
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_zero_byte_hard_link)

def test_parse_unicode_name(tmpdir):
    indir = tmpdir.mkdir('unicode')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'föo'), 'wb') as outfp:
        outfp.write(b'foo\n')

    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_unicode_name)

def test_parse_unicode_name_isolevel4(tmpdir):
    indir = tmpdir.mkdir('unicodeisolevel4')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'föo'), 'wb') as outfp:
        outfp.write(b'foo\n')

    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_unicode_name_isolevel4)

def test_parse_unicode_name_joliet(tmpdir):
    indir = tmpdir.mkdir('unicodejoliet')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'föo'), 'wb') as outfp:
        outfp.write(b'foo\n')

    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-J',
                     '-no-pad', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_unicode_name_joliet)

def test_parse_unicode_name_udf(tmpdir):
    indir = tmpdir.mkdir('unicodeudf')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'föo'), 'wb') as outfp:
        outfp.write(b'foo\n')

    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-udf',
                     '-no-pad', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_unicode_name_udf)

def test_parse_unicode_name_two_byte(tmpdir):
    indir = tmpdir.mkdir('unicode')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'fᴔo'), 'wb') as outfp:
        outfp.write(b'foo\n')

    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_unicode_name_two_byte)

def test_parse_unicode_name_two_byte(tmpdir):
    indir = tmpdir.mkdir('unicode')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'fᴔo'), 'wb') as outfp:
        outfp.write(b'foo\n')

    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_unicode_name_two_byte)

def test_parse_unicode_name_two_byte_isolevel4(tmpdir):
    indir = tmpdir.mkdir('unicodeisolevel4')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'fᴔo'), 'wb') as outfp:
        outfp.write(b'foo\n')

    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_unicode_name_two_byte_isolevel4)

def test_parse_unicode_name_two_byte_joliet(tmpdir):
    indir = tmpdir.mkdir('unicodetwobytejoliet')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'fᴔo'), 'wb') as outfp:
        outfp.write(b'foo\n')

    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-J',
                     '-no-pad', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_unicode_name_two_byte_joliet)

def test_parse_unicode_name_two_byte_udf(tmpdir):
    indir = tmpdir.mkdir('unicodeudftwobyte')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'fᴔo'), 'wb') as outfp:
        outfp.write(b'foo\n')

    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-udf',
                     '-no-pad', '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_unicode_name_two_byte_udf)

def test_parse_eltorito_get_bootcat(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('eltoritogetbootcat')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_eltorito_get_bootcat)

def test_parse_eltorito_uefi(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('eltoritonofiles')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    retcode = subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1',
                               '-no-pad', '-c', 'boot.cat', '-efi-boot', 'boot',
                               '-no-emul-boot', '-o', str(outfile), str(indir)])
    if retcode != 0:
        pytest.skip("This version of genisoimage doesn't support -efi-boot")

    do_a_test(tmpdir, outfile, check_eltorito_uefi)

def test_parse_isolevel4_deep_directory(tmpdir):
    indir = tmpdir.mkdir('isolevel4deep')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1').mkdir('dir2').mkdir('dir3').mkdir('dir4').mkdir('dir5').mkdir('dir6').mkdir('dir7')
    with open(os.path.join(str(indir), 'dir1', 'dir2', 'dir3', 'dir4', 'dir5', 'dir6', 'dir7', 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-o', str(outfile), str(indir)])

    do_a_test(tmpdir, outfile, check_isolevel4_deep_directory)

def test_parse_udf_open_invalid_file_set_terminator(caplog, tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('udfinvalidfilesetterminator')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-udf', '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, we open it up and set the file set
    # terminator sector to something invalid.
    with open(str(outfile), 'r+b') as fp:
        fp.seek(0x81000)
        fp.write(b'\x00'*512)

    do_a_test(tmpdir, outfile, check_udf_onefile)

def test_parse_and_walk_rr_moved_iso_path(tmpdir):
    indir = tmpdir.mkdir('walkrrmovedisopath')
    outfile = str(indir) + '.iso'
    indir.mkdir('usr').mkdir('lib').mkdir('debug').mkdir('usr').mkdir('lib').mkdir('clang').mkdir('13.0.0').mkdir('lib').mkdir('freebsd')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '3', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    iso = pycdlib.PyCdlib()
    iso.open(str(outfile))

    expected_dirnames = ['/', '/RR_MOVED', '/RR_MOVED/LIB', '/RR_MOVED/LIB/FREEBSD', '/USR', '/USR/LIB', '/USR/LIB/DEBUG', '/USR/LIB/DEBUG/USR', '/USR/LIB/DEBUG/USR/LIB', '/USR/LIB/DEBUG/USR/LIB/CLANG', '/USR/LIB/DEBUG/USR/LIB/CLANG/13_0.0']

    # Ensure that we don't see duplicates of any directory names
    seen_dirnames = []
    for dirname, dirlist, filelist in iso.walk(iso_path='/'):
        assert(dirname not in seen_dirnames)
        seen_dirnames.append(dirname)

    assert(expected_dirnames == seen_dirnames)
    iso.close()

def test_parse_full_path_from_dirrecord_rr_moved(tmpdir):
    indir = tmpdir.mkdir('fullpathfromdirrecordrrmoved')
    outfile = str(indir) + '.iso'
    indir.mkdir('usr').mkdir('lib').mkdir('debug').mkdir('usr').mkdir('lib').mkdir('clang').mkdir('13.0.0').mkdir('lib').mkdir('freebsd')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '3', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    iso = pycdlib.PyCdlib()
    iso.open(str(outfile))

    rec = iso.get_record(rr_path='/usr/lib/debug/usr/lib/clang/13.0.0/lib')

    path = iso.full_path_from_dirrecord(rec, True)

    assert(path == '/usr/lib/debug/usr/lib/clang/13.0.0/lib')

    iso.close()

def test_parse_walk_rr_moved(tmpdir):
    indir = tmpdir.mkdir('walkrrmoved')
    outfile = str(indir) + '.iso'
    indir.mkdir('usr').mkdir('lib').mkdir('debug').mkdir('usr').mkdir('lib').mkdir('clang').mkdir('13.0.0').mkdir('lib').mkdir('freebsd')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '3', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    iso = pycdlib.PyCdlib()
    iso.open(str(outfile))

    expected_dirnames = ['/', '/rr_moved', '/usr', '/usr/lib', '/usr/lib/debug', '/usr/lib/debug/usr', '/usr/lib/debug/usr/lib', '/usr/lib/debug/usr/lib/clang', '/usr/lib/debug/usr/lib/clang/13.0.0', '/usr/lib/debug/usr/lib/clang/13.0.0/lib', '/usr/lib/debug/usr/lib/clang/13.0.0/lib/freebsd']

    # Ensure that we don't see duplicates of any directory names
    seen_dirnames = []
    for dirname, dirlist, filelist in iso.walk(rr_path='/'):
        assert(dirname not in seen_dirnames)
        seen_dirnames.append(dirname)

    assert(expected_dirnames == seen_dirnames)
    iso.close()

def test_parse_invalid_vdst(tmpdir):
    indir = tmpdir.mkdir('invalidvdst')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now that we've made a valid ISO, open it up and perturb the VDST version
    # field to 0.  We should allow this.
    with open(str(outfile), 'r+b') as fp:
        fp.seek(17*2048 + 6)
        fp.write(b'\x00')

    do_a_test(tmpdir, outfile, check_onefile)

def test_parse_walk_shiftjis(tmpdir):
    # The filename below is Shift-JIS encoded, and in Japanese is: 検索ブラウザ.exe
    # (according to Google translate, 'search browser.exe')
    # 16 bytes
    shiftjis_filename = b'\x8c\x9f\x8d\xf5\x83\x75\x83\x89\x83\x45\x83\x55\x2e\x65\x78\x65'

    indir = tmpdir.mkdir('shiftjis')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foofoofoofoo.exe'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '3', '-no-pad',
                     '-o', str(outfile), str(indir)])

    with open(str(outfile), 'r+b') as fp:
        fp.seek(23*2048 + 101)
        fp.write(shiftjis_filename)

    iso = pycdlib.PyCdlib()
    iso.open(str(outfile))

    expected_filenames = [shiftjis_filename.decode('shiftjis') + ';1']

    # Ensure that we don't see duplicates of any directory names
    seen_filenames = []
    for dirname, dirlist, filelist in iso.walk(iso_path='/', encoding='shiftjis'):
        seen_filenames.extend(filelist)

    assert(expected_filenames == seen_filenames)
    iso.close()

def test_parse_walk_vietnamese(tmpdir):
    # The filename below is UTF8 encoded, and in Vietnamese is: Yêu cầu ưu đãi
    # (according to Google translate, 'request offer')
    # 20 bytes
    vietnamese_filename = b'\x59\xc3\xaa\x75\x20\x63\xe1\xba\xa7\x75\x20\xc6\xb0\x75\x20\xc4\x91\xc3\xa3\x69'

    indir = tmpdir.mkdir('vietnamese')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foodfoodfoodfood.foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '3', '-no-pad',
                     '-o', str(outfile), str(indir)])

    with open(str(outfile), 'r+b') as fp:
        fp.seek(23*2048 + 101)
        fp.write(vietnamese_filename)

    iso = pycdlib.PyCdlib()
    iso.open(str(outfile))

    expected_filenames = [vietnamese_filename.decode('utf8') + ';1']

    # Ensure that we don't see duplicates of any directory names
    seen_filenames = []
    for dirname, dirlist, filelist in iso.walk(iso_path='/'):
        seen_filenames.extend(filelist)

    assert(expected_filenames == seen_filenames)
    iso.close()

def test_parse_one_extent_path_tables(tmpdir):
    indir = tmpdir.mkdir('onefileonextentpathtables')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # genisoimage always creates ISOs with two extent path tables.  For this
    # test, we need to cut out the empty second extent from the path tables.
    # Read the whole thing into memory, and then cut out those bits.
    with open(str(outfile), 'rb') as infp:
        contents = infp.read()

    shrunk = bytearray(contents[:0xa000] + contents[0xa800:0xb000] + contents[0xb800:])

    # The things we need to shrink by two extents to make a valid ISO are the
    # PVD space size, the root dir record location, the path table root dir
    # location, the dot file record, the dotdot file record, and the FOO file
    # record.
    # We also need to move the PTR BE location up one extent.

    for offset in (0x8050, 0x8057, 0x809e, 0x80a5, 0x9802, 0xa005, 0xa802, 0xa809, 0xa824, 0xa82b, 0xa846, 0xa84d):
        shrunk[offset] = shrunk[offset] - 2

    # Move the PTR BE up one extent
    shrunk[0x8097] = 0x14

    with open(str(outfile), 'wb') as outfp:
        outfp.write(shrunk)

    do_a_test(tmpdir, outfile, check_onefile_one_extent_path_tables)
