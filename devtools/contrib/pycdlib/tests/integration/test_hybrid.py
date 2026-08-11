import io
import os
import subprocess
import sys
import shutil

import pytest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))

import pycdlib

from test_common import *

def do_a_test(iso, check_func):
    out = io.BytesIO()
    iso.write_fp(out)

    check_func(iso, len(out.getvalue()))

    iso2 = pycdlib.PyCdlib()
    iso2.open_fp(out)
    check_func(iso2, len(out.getvalue()))
    iso2.close()

def test_hybrid_nofiles(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('nofiles')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.rm_file('/FOO.;1')

    do_a_test(iso, check_nofiles)

    iso.close()

def test_hybrid_onefile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('onefile')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    foostr = b'foo\n'
    iso.add_fp(io.BytesIO(foostr), len(foostr), '/FOO.;1')

    do_a_test(iso, check_onefile)

    iso.close()

def test_hybrid_onedir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('onefile')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.add_directory('/DIR1')

    do_a_test(iso, check_onedir)

    iso.close()

def test_hybrid_twofiles(tmpdir):
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

    barstr = b'bar\n'
    iso.add_fp(io.BytesIO(barstr), len(barstr), '/BAR.;1')

    do_a_test(iso, check_twofiles)

    iso.close()

def test_hybrid_twofiles2(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('twofile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'bar'), 'wb') as outfp:
        outfp.write(b'bar\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    foostr = b'foo\n'
    iso.add_fp(io.BytesIO(foostr), len(foostr), '/FOO.;1')

    do_a_test(iso, check_twofiles)

    iso.close()

def test_hybrid_twofiles3(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('twofile')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    foostr = b'foo\n'
    iso.add_fp(io.BytesIO(foostr), len(foostr), '/FOO.;1')

    barstr = b'bar\n'
    iso.add_fp(io.BytesIO(barstr), len(barstr), '/BAR.;1')

    do_a_test(iso, check_twofiles)

    iso.close()

def test_hybrid_twofiles4(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('twofile')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    barstr = b'bar\n'
    iso.add_fp(io.BytesIO(barstr), len(barstr), '/BAR.;1')

    foostr = b'foo\n'
    iso.add_fp(io.BytesIO(foostr), len(foostr), '/FOO.;1')

    do_a_test(iso, check_twofiles)

    iso.close()

def test_hybrid_twodirs(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('twodir')
    outfile = str(indir)+'.iso'
    indir.mkdir('aa')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.add_directory('/BB')

    do_a_test(iso, check_twodirs)

    iso.close()

def test_hybrid_twodirs2(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('twodir')
    outfile = str(indir)+'.iso'
    indir.mkdir('bb')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.add_directory('/AA')

    do_a_test(iso, check_twodirs)

    iso.close()

def test_hybrid_twodirs3(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('twodir')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.add_directory('/AA')
    iso.add_directory('/BB')

    do_a_test(iso, check_twodirs)

    iso.close()

def test_hybrid_twodirs4(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('twodir')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.add_directory('/BB')
    iso.add_directory('/AA')

    do_a_test(iso, check_twodirs)

    iso.close()

def test_hybrid_rmfile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('twofile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    with open(os.path.join(str(indir), 'bar'), 'wb') as outfp:
        outfp.write(b'bar\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.rm_file('/BAR.;1')

    do_a_test(iso, check_onefile)

    iso.close()

def test_hybrid_rmdir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rmdir')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.rm_directory('/DIR1')

    do_a_test(iso, check_onefile)

    iso.close()

def test_hybrid_onefileonedir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rmdir')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.add_directory('/DIR1')

    do_a_test(iso, check_onefileonedir)

    iso.close()

def test_hybrid_onefileonedir2(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rmdir')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    foostr = b'foo\n'
    iso.add_fp(io.BytesIO(foostr), len(foostr), '/FOO.;1')

    do_a_test(iso, check_onefileonedir)

    iso.close()

def test_hybrid_onefileonedir3(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rmdir')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.add_directory('/DIR1')

    foostr = b'foo\n'
    iso.add_fp(io.BytesIO(foostr), len(foostr), '/FOO.;1')

    do_a_test(iso, check_onefileonedir)

    iso.close()

def test_hybrid_onefileonedir4(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rmdir')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    foostr = b'foo\n'
    iso.add_fp(io.BytesIO(foostr), len(foostr), '/FOO.;1')

    iso.add_directory('/DIR1')

    do_a_test(iso, check_onefileonedir)

    iso.close()

def test_hybrid_onefile_onedirwithfile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rmdir')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.add_directory('/DIR1')

    barstr = b'bar\n'
    iso.add_fp(io.BytesIO(barstr), len(barstr), '/DIR1/BAR.;1')

    do_a_test(iso, check_onefile_onedirwithfile)

    iso.close()

def test_hybrid_onefile_onedirwithfile2(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rmdir')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    foostr = b'foo\n'
    iso.add_fp(io.BytesIO(foostr), len(foostr), '/FOO.;1')

    barstr = b'bar\n'
    iso.add_fp(io.BytesIO(barstr), len(barstr), '/DIR1/BAR.;1')

    do_a_test(iso, check_onefile_onedirwithfile)

    iso.close()

def test_hybrid_onefile_onedirwithfile3(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rmdir')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    foostr = b'foo\n'
    iso.add_fp(io.BytesIO(foostr), len(foostr), '/FOO.;1')

    iso.add_directory('/DIR1')

    barstr = b'bar\n'
    iso.add_fp(io.BytesIO(barstr), len(barstr), '/DIR1/BAR.;1')

    do_a_test(iso, check_onefile_onedirwithfile)

    iso.close()

def test_hybrid_onefile_onedirwithfile4(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rmdir')
    outfile = str(indir)+'.iso'
    dir1 = indir.mkdir('dir1')
    with open(os.path.join(str(dir1), 'bar'), 'wb') as outfp:
        outfp.write(b'bar\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    foostr = b'foo\n'
    iso.add_fp(io.BytesIO(foostr), len(foostr), '/FOO.;1')

    do_a_test(iso, check_onefile_onedirwithfile)

    iso.close()

def test_hybrid_twoextentfile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rmdir')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    outstr = b''
    for j in range(0, 8):
        for i in range(0, 256):
            outstr += struct.pack('=B', i)
    outstr += struct.pack('=B', 0)

    iso.add_fp(io.BytesIO(outstr), len(outstr), '/BIGFILE.;1')

    do_a_test(iso, check_twoextentfile)

    iso.close()

def test_hybrid_ptr_extent(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('manydirs')
    outfile = str(indir)+'.iso'
    numdirs = 293
    for i in range(1, 1+numdirs):
        indir.mkdir('dir%d' % i)
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.add_directory('/DIR294')
    iso.add_directory('/DIR295')

    do_a_test(iso, check_dirs_overflow_ptr_extent)

    iso.close()

def test_hybrid_ptr_extent2(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('manydirs')
    outfile = str(indir)+'.iso'
    numdirs = 295
    for i in range(1, 1+numdirs):
        indir.mkdir('dir%d' % i)
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.rm_directory('/DIR294')
    iso.rm_directory('/DIR295')

    do_a_test(iso, check_dirs_just_short_ptr_extent)

    iso.close()

def test_hybrid_remove_many(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('manydirs')
    outfile = str(indir)+'.iso'
    numdirs = 295
    for i in range(1, 1+numdirs):
        indir.mkdir('dir%d' % i)
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    # Now remove all but one of the entries.
    for i in range(2, 1+numdirs):
        iso.rm_directory('/DIR' + str(i))

    do_a_test(iso, check_onedir)

    iso.close()

def test_hybrid_twoleveldeepdir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('twoleveldeep')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.add_directory('/DIR1/SUBDIR1')

    do_a_test(iso, check_twoleveldeepdir)

    iso.close()

def test_hybrid_twoleveldeepdir2(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('twoleveldeep')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.add_directory('/DIR1')

    iso.add_directory('/DIR1/SUBDIR1')

    do_a_test(iso, check_twoleveldeepdir)

    iso.close()

def test_hybrid_rmsubdir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('twoleveldeep')
    outfile = str(indir)+'.iso'
    dir1 = indir.mkdir('dir1')
    dir1.mkdir('subdir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.rm_directory('/DIR1/SUBDIR1')

    do_a_test(iso, check_onedir)

    iso.close()

def test_hybrid_removeall(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('twoleveldeep')
    outfile = str(indir)+'.iso'
    dir1 = indir.mkdir('dir1')
    dir1.mkdir('subdir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.rm_directory('/DIR1/SUBDIR1')
    iso.rm_directory('/DIR1')

    do_a_test(iso, check_nofiles)

    iso.close()

def test_hybrid_add_new_file_to_subdir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('twoleveldeep')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    foostr = b'foo\n'
    iso.add_fp(io.BytesIO(foostr), len(foostr), '/FOO.;1')

    barstr = b'bar\n'
    iso.add_fp(io.BytesIO(barstr), len(barstr), '/DIR1/BAR.;1')

    do_a_test(iso, check_onefile_onedirwithfile)

    iso.close()

def test_hybrid_eltorito_add(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('eltoritohybrid')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    # Now add the eltorito stuff
    bootstr = b'boot\n'
    iso.add_fp(io.BytesIO(bootstr), len(bootstr), '/BOOT.;1')
    iso.add_eltorito('/BOOT.;1', '/BOOT.CAT;1')

    do_a_test(iso, check_eltorito_nofiles)

    iso.close()

def test_hybrid_eltorito_remove(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('eltoritonofiles')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.rm_eltorito()
    iso.rm_file('/BOOT.;1')

    do_a_test(iso, check_nofiles)

    iso.close()

def test_hybrid_eltorito_add_bootcat(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('eltoritotwofile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    with open(os.path.join(str(indir), 'aa'), 'wb') as outfp:
        outfp.write(b'aa\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.add_eltorito('/BOOT.;1', '/BOOT.CAT;1')

    do_a_test(iso, check_eltorito_twofile)

    iso.close()

def test_hybrid_rr_nofiles(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rrnofiles')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.rm_file('/FOO.;1', rr_name='foo')

    do_a_test(iso, check_rr_nofiles)

    iso.close()

def test_hybrid_rr_onefile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rronefile')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    foostr = b'foo\n'
    iso.add_fp(io.BytesIO(foostr), len(foostr), '/FOO.;1', rr_name='foo')

    do_a_test(iso, check_rr_onefile)

    iso.close()

def test_hybrid_rr_rmfile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rrrmfile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    with open(os.path.join(str(indir), 'baz'), 'wb') as outfp:
        outfp.write(b'baz\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.rm_file('/BAZ.;1', rr_name='baz')

    do_a_test(iso, check_rr_onefile)

    iso.close()

def test_hybrid_rr_onefileonedir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rronefileonedir')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.add_directory('/DIR1', rr_name='dir1')

    do_a_test(iso, check_rr_onefileonedir)

    iso.close()

def test_hybrid_rr_onefileonedirwithfile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rronefileonedirwithfile')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    foostr = b'foo\n'
    iso.add_fp(io.BytesIO(foostr), len(foostr), '/FOO.;1', rr_name='foo')

    barstr = b'bar\n'
    iso.add_fp(io.BytesIO(barstr), len(barstr), '/DIR1/BAR.;1', rr_name='bar')

    do_a_test(iso, check_rr_onefileonedirwithfile)

    iso.close()

def test_hybrid_rr_and_joliet_nofiles(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rrjolietnofiles')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-J', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.rm_file('/FOO.;1', rr_name='foo', joliet_path='/foo')

    do_a_test(iso, check_joliet_and_rr_nofiles)

    iso.close()

def test_hybrid_rr_and_joliet_onefile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rrjolietonefile')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-J', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    foostr = b'foo\n'
    iso.add_fp(io.BytesIO(foostr), len(foostr), '/FOO.;1', rr_name='foo', joliet_path='/foo')

    do_a_test(iso, check_joliet_and_rr_onefile)

    iso.close()

def test_hybrid_rr_and_eltorito_nofiles(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rreltoritonofiles')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.add_eltorito('/BOOT.;1', '/BOOT.CAT;1')

    do_a_test(iso, check_rr_and_eltorito_nofiles)

    iso.close()

def test_hybrid_rr_and_eltorito_nofiles2(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rreltoritonofiles2')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    bootstr = b'boot\n'
    iso.add_fp(io.BytesIO(bootstr), len(bootstr), '/BOOT.;1', rr_name='boot')
    iso.add_eltorito('/BOOT.;1', '/BOOT.CAT;1')

    do_a_test(iso, check_rr_and_eltorito_nofiles)

    iso.close()

def test_hybrid_rr_and_eltorito_onefile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rreltoritoonefile')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    bootstr = b'boot\n'
    iso.add_fp(io.BytesIO(bootstr), len(bootstr), '/BOOT.;1', rr_name='boot')
    iso.add_eltorito('/BOOT.;1', '/BOOT.CAT;1')

    foostr = b'foo\n'
    iso.add_fp(io.BytesIO(foostr), len(foostr), '/FOO.;1', rr_name='foo')

    do_a_test(iso, check_rr_and_eltorito_onefile)

    iso.close()

def test_hybrid_rr_and_eltorito_onefile2(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rreltoritoonefile2')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    bootstr = b'boot\n'
    iso.add_fp(io.BytesIO(bootstr), len(bootstr), '/BOOT.;1', rr_name='boot')
    iso.add_eltorito('/BOOT.;1', '/BOOT.CAT;1')

    do_a_test(iso, check_rr_and_eltorito_onefile)

    iso.close()

def test_hybrid_rr_and_eltorito_onefile3(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rreltoritoonefile3')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    foostr = b'foo\n'
    iso.add_fp(io.BytesIO(foostr), len(foostr), '/FOO.;1', rr_name='foo')

    do_a_test(iso, check_rr_and_eltorito_onefile)

    iso.close()

def test_hybrid_rr_and_eltorito_onedir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rreltoritoonedir')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    bootstr = b'boot\n'
    iso.add_fp(io.BytesIO(bootstr), len(bootstr), '/BOOT.;1', rr_name='boot')
    iso.add_eltorito('/BOOT.;1', '/BOOT.CAT;1')

    iso.add_directory('/DIR1', rr_name='dir1')

    do_a_test(iso, check_rr_and_eltorito_onedir)

    iso.close()

def test_hybrid_rr_and_eltorito_onedir2(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rreltoritoonedir2')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.add_directory('/DIR1', rr_name='dir1')

    bootstr = b'boot\n'
    iso.add_fp(io.BytesIO(bootstr), len(bootstr), '/BOOT.;1', rr_name='boot')
    iso.add_eltorito('/BOOT.;1', '/BOOT.CAT;1')

    do_a_test(iso, check_rr_and_eltorito_onedir)

    iso.close()

def test_hybrid_rr_and_eltorito_onedir3(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rreltoritoonedir3')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    bootstr = b'boot\n'
    iso.add_fp(io.BytesIO(bootstr), len(bootstr), '/BOOT.;1', rr_name='boot')
    iso.add_eltorito('/BOOT.;1', '/BOOT.CAT;1')

    do_a_test(iso, check_rr_and_eltorito_onedir)

    iso.close()

def test_hybrid_rr_and_eltorito_onedir4(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rreltoritoonedir4')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.add_directory('/DIR1', rr_name='dir1')

    do_a_test(iso, check_rr_and_eltorito_onedir)

    iso.close()

def test_hybrid_rr_and_eltorito_rmdir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rreltoritormdir')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.rm_directory('/DIR1', rr_name='dir1')

    do_a_test(iso, check_rr_and_eltorito_nofiles)

    iso.close()

def test_hybrid_rr_and_eltorito_rmdir2(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rreltoritormdir2')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    dir1 = indir.mkdir('dir1')
    dir1.mkdir('subdir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.rm_directory('/DIR1/SUBDIR1', rr_name='subdir1')

    do_a_test(iso, check_rr_and_eltorito_onedir)

    iso.close()

def test_hybrid_joliet_and_eltorito_remove(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolieteltoritoremove')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-J', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.rm_eltorito()

    iso.rm_file('/BOOT.;1', joliet_path='/boot')

    do_a_test(iso, check_joliet_nofiles)

    iso.close()

def test_hybrid_joliet_and_eltorito_onefile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolieteltoritoonefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-J', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    foostr = b'foo\n'
    iso.add_fp(io.BytesIO(foostr), len(foostr), '/FOO.;1', joliet_path='/foo')

    do_a_test(iso, check_joliet_and_eltorito_onefile)

    iso.close()

def test_hybrid_joliet_and_eltorito_onefile2(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolieteltoritoonefile2')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    bootstr = b'boot\n'
    iso.add_fp(io.BytesIO(bootstr), len(bootstr), '/BOOT.;1', joliet_path='/boot')

    foostr = b'foo\n'
    iso.add_fp(io.BytesIO(foostr), len(foostr), '/FOO.;1', joliet_path='/foo')

    iso.add_eltorito('/BOOT.;1', '/BOOT.CAT;1')

    do_a_test(iso, check_joliet_and_eltorito_onefile)

    iso.close()

def test_hybrid_joliet_and_eltorito_onefile3(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolieteltoritoonefile3')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    bootstr = b'boot\n'
    iso.add_fp(io.BytesIO(bootstr), len(bootstr), '/BOOT.;1', joliet_path='/boot')

    iso.add_eltorito('/BOOT.;1', '/BOOT.CAT;1')

    do_a_test(iso, check_joliet_and_eltorito_onefile)

    iso.close()

def test_hybrid_joliet_and_eltorito_onedir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolieteltoritoonedir')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    bootstr = b'boot\n'
    iso.add_fp(io.BytesIO(bootstr), len(bootstr), '/BOOT.;1', joliet_path='/boot')

    iso.add_eltorito('/BOOT.;1', '/BOOT.CAT;1')

    do_a_test(iso, check_joliet_and_eltorito_onedir)

    iso.close()

def test_hybrid_joliet_and_eltorito_onedir2(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolieteltoritoonedir2')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-J', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.add_directory('/DIR1', joliet_path='/dir1')

    do_a_test(iso, check_joliet_and_eltorito_onedir)

    iso.close()

def test_hybrid_isohybrid(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('isohybrid')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    # Add Eltorito
    isolinuxstr = b'\x00'*0x40 + b'\xfb\xc0\x78\x70'
    iso.add_fp(io.BytesIO(isolinuxstr), len(isolinuxstr), '/ISOLINUX.BIN;1')
    iso.add_eltorito('/ISOLINUX.BIN;1', '/BOOT.CAT;1', boot_load_size=4)
    # Now add the syslinux
    iso.add_isohybrid()

    do_a_test(iso, check_isohybrid)

    iso.close()

def test_hybrid_isohybrid2(tmpdir):
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

    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    # Now add the syslinux
    iso.add_isohybrid()

    do_a_test(iso, check_isohybrid)

    iso.close()

@pytest.mark.skipif(find_executable('isohybrid') is None,
                    reason='syslinux not installed')
def test_hybrid_isohybrid3(tmpdir):
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

    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.rm_isohybrid()

    iso.rm_eltorito()
    iso.rm_file('/ISOLINUX.BIN;1')

    do_a_test(iso, check_nofiles)

    iso.close()

def test_hybrid_joliet_rr_and_eltorito_nofiles(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolietrreltoritonofiles')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-J', '-rational-rock', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.rm_file('/FOO.;1', joliet_path='/foo', rr_name='foo')

    do_a_test(iso, check_joliet_rr_and_eltorito_nofiles)

    iso.close()

def test_hybrid_joliet_rr_and_eltorito_nofiles2(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolietrreltoritonofiles2')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-rational-rock', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    bootstr = b'boot\n'
    iso.add_fp(io.BytesIO(bootstr), len(bootstr), '/BOOT.;1', rr_name='boot', joliet_path='/boot')

    iso.add_eltorito('/BOOT.;1', '/BOOT.CAT;1')

    do_a_test(iso, check_joliet_rr_and_eltorito_nofiles)

    iso.close()

def test_hybrid_joliet_rr_and_eltorito_onefile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolietrreltoritoonefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-J', '-rational-rock', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    foostr = b'foo\n'
    iso.add_fp(io.BytesIO(foostr), len(foostr), '/FOO.;1', rr_name='foo', joliet_path='/foo')

    do_a_test(iso, check_joliet_rr_and_eltorito_onefile)

    iso.close()

def test_hybrid_joliet_rr_and_eltorito_onefile2(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolietrreltoritoonefile2')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-rational-rock', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    bootstr = b'boot\n'
    iso.add_fp(io.BytesIO(bootstr), len(bootstr), '/BOOT.;1', rr_name='boot', joliet_path='/boot')

    iso.add_eltorito('/BOOT.;1', '/BOOT.CAT;1')

    do_a_test(iso, check_joliet_rr_and_eltorito_onefile)

    iso.close()

def test_hybrid_joliet_rr_and_eltorito_onefile3(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolietrreltoritoonefile3')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-rational-rock', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    foostr = b'foo\n'
    iso.add_fp(io.BytesIO(foostr), len(foostr), '/FOO.;1', rr_name='foo', joliet_path='/foo')

    bootstr = b'boot\n'
    iso.add_fp(io.BytesIO(bootstr), len(bootstr), '/BOOT.;1', rr_name='boot', joliet_path='/boot')

    iso.add_eltorito('/BOOT.;1', '/BOOT.CAT;1')

    do_a_test(iso, check_joliet_rr_and_eltorito_onefile)

    iso.close()

def test_hybrid_joliet_rr_and_eltorito_onedir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolietrreltoritoonedir')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-rational-rock', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    bootstr = b'boot\n'
    iso.add_fp(io.BytesIO(bootstr), len(bootstr), '/BOOT.;1', rr_name='boot', joliet_path='/boot')

    iso.add_eltorito('/BOOT.;1', '/BOOT.CAT;1')

    do_a_test(iso, check_joliet_rr_and_eltorito_onedir)

    iso.close()

def test_hybrid_joliet_rr_and_eltorito_onedir2(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolietrreltoritoonedir2')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-rational-rock', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    bootstr = b'boot\n'
    iso.add_fp(io.BytesIO(bootstr), len(bootstr), '/BOOT.;1', rr_name='boot', joliet_path='/boot')

    iso.add_eltorito('/BOOT.;1', '/BOOT.CAT;1')

    iso.add_directory('/DIR1', rr_name='dir1', joliet_path='/dir1')

    do_a_test(iso, check_joliet_rr_and_eltorito_onedir)

    iso.close()

def test_hybrid_joliet_rr_and_eltorito_onedir3(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolietrreltoritoonedir2')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-J', '-rational-rock', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.add_directory('/DIR1', rr_name='dir1', joliet_path='/dir1')

    do_a_test(iso, check_joliet_rr_and_eltorito_onedir)

    iso.close()

def test_hybrid_rr_rmfile2(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rrrmfile2')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.rm_file('/FOO.;1', rr_name='foo')

    do_a_test(iso, check_rr_nofiles)

    iso.close()

def test_hybrid_rr_rmdir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rrrmfile2')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.rm_directory('/DIR1', rr_name='dir1')

    do_a_test(iso, check_rr_nofiles)

    iso.close()

def test_hybrid_xa_nofiles(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('xarmfile2')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-xa', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.rm_directory('/DIR1')

    do_a_test(iso, check_xa_nofiles)

    iso.close()

def test_hybrid_xa_nofiles2(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('xarmfile2')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-xa', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.rm_file('/FOO.;1')

    do_a_test(iso, check_xa_nofiles)

    iso.close()

def test_hybrid_xa_onefile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('xarmfile2')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-xa', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    foostr = b'foo\n'
    iso.add_fp(io.BytesIO(foostr), len(foostr), '/FOO.;1')

    do_a_test(iso, check_xa_onefile)

    iso.close()

def test_hybrid_xa_onedir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('xarmfile2')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-xa', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.add_directory('/DIR1')

    do_a_test(iso, check_xa_onedir)

    iso.close()

def test_hybrid_sevendeepdirs(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('sevendeepdirs')
    outfile = str(indir)+'.iso'
    numdirs = 8
    x = indir
    for i in range(1, 1+numdirs):
        x = x.mkdir('dir%d' % i)
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.rm_directory('/DIR1/DIR2/DIR3/DIR4/DIR5/DIR6/DIR7/DIR8', rr_name='dir8')

    do_a_test(iso, check_sevendeepdirs)

    iso.close()

def test_hybrid_xa_joliet_onedir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('xarmfile2')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-xa', '-J', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.add_directory('/DIR1', joliet_path='/dir1')

    do_a_test(iso, check_xa_joliet_onedir)

    iso.close()

def test_hybrid_xa_joliet_onefile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('xarmfile2')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-xa', '-J', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    foostr = b'foo\n'
    iso.add_fp(io.BytesIO(foostr), len(foostr), '/FOO.;1', joliet_path='/foo')

    do_a_test(iso, check_xa_joliet_onefile)

    iso.close()

def test_hybrid_isolevel4_onefile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('xarmfile2')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    with open(os.path.join(str(indir), 'bar'), 'wb') as outfp:
        outfp.write(b'bar\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.rm_file('/bar')

    do_a_test(iso, check_isolevel4_onefile)

    iso.close()

def test_hybrid_isolevel4_onefile2(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('xarmfile2')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.rm_directory('/dir1')

    do_a_test(iso, check_isolevel4_onefile)

    iso.close()

def test_hybrid_isolevel4_eltorito(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('isolevel4eltorito')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    bootstr = b'boot\n'
    iso.add_fp(io.BytesIO(bootstr), len(bootstr), '/boot')

    iso.add_eltorito('/boot', '/boot.cat')

    do_a_test(iso, check_isolevel4_eltorito)

    iso.close()

def test_hybrid_isolevel4_eltorito2(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('isolevel4eltorito')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.rm_eltorito()
    iso.rm_file('/boot')

    do_a_test(iso, check_isolevel4_nofiles)

    iso.close()

def test_hybrid_eltorito_multi_boot(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('eltoritonofiles')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    boot2str = b'boot2\n'
    iso.add_fp(io.BytesIO(boot2str), len(boot2str), '/boot2')
    iso.add_eltorito('/boot2', '/boot.cat')

    do_a_test(iso, check_eltorito_multi_boot)

    iso.close()

def test_hybrid_eltorito_multi_boot_boot_info(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('eltoritonofiles')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    boot2str = b'boot2\n'
    iso.add_fp(io.BytesIO(boot2str), len(boot2str), '/boot2')
    iso.add_eltorito('/boot2', '/boot.cat', boot_info_table=True)

    do_a_test(iso, check_eltorito_multi_boot)

    iso.close()

def test_hybrid_eltorito_multi_boot_hard_link(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('eltoritonofiles')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    boot2str = b'boot2\n'
    iso.add_fp(io.BytesIO(boot2str), len(boot2str), '/boot2')
    iso.add_hard_link(iso_new_path='/bootlink', iso_old_path='/boot2')
    iso.add_eltorito('/boot2', '/boot.cat')

    do_a_test(iso, check_eltorito_multi_boot_hard_link)

    iso.close()

def open_and_check(outfile, checkfunc):
    iso = pycdlib.PyCdlib()
    iso.open(str(outfile))
    checkfunc(iso, os.stat(str(outfile)).st_size)
    iso.close()

def test_hybrid_modify_in_place_onefile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceonefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'f\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and modify it.
    iso = pycdlib.PyCdlib()
    iso.open(str(outfile), 'r+b')
    foostr = b'foo\n'
    iso.modify_file_in_place(io.BytesIO(foostr), len(foostr), '/FOO.;1')
    iso.close()

    # Now re-open it and check things out.
    open_and_check(outfile, check_onefile)

def test_hybrid_joliet_modify_in_place_onefile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolietmodifyinplaceonefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'f\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and modify it.
    iso = pycdlib.PyCdlib()
    iso.open(str(outfile), 'r+b')
    foostr = b'foo\n'
    iso.modify_file_in_place(io.BytesIO(foostr), len(foostr), '/FOO.;1', joliet_path='/foo')
    iso.close()

    # Now re-open it and check things out.
    open_and_check(outfile, check_joliet_onefile)

def test_hybrid_modify_in_place_iso_level4_onefile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'f\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()
    iso.open(str(outfile), 'r+b')
    foostr = b'foo\n'
    iso.modify_file_in_place(io.BytesIO(foostr), len(foostr), '/foo')
    iso.close()

    # Now open up the ISO with pycdlib and modify it.
    open_and_check(outfile, check_isolevel4_onefile)

def test_hybrid_modify_in_place_udf(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceoneudf')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'f\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-udf', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()
    iso.open(str(outfile), 'r+b')
    foostr = b'foo\n'
    iso.modify_file_in_place(io.BytesIO(foostr), len(foostr), '/FOO.;1')
    iso.close()

    # Now open up the ISO with pycdlib and modify it.
    open_and_check(outfile, check_udf_onefile)

def test_hybrid_modify_in_place_udf_shrink(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceoneudfshrink')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foobarbaz\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-udf', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()
    iso.open(str(outfile), 'r+b')
    foostr = b'foo\n'
    iso.modify_file_in_place(io.BytesIO(foostr), len(foostr), '/FOO.;1')
    iso.close()

    # Now open up the ISO with pycdlib and modify it.
    open_and_check(outfile, check_udf_onefile)

def test_hybrid_try_to_use_new_on_open_file(tmpdir):
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
        iso.new()
    assert(str(excinfo.value) == 'This object already has an ISO; either close it or create a new object')

    iso.close()

def test_hybrid_try_to_use_open_on_new_file(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'f\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-o', str(outfile), str(indir)])

    iso = pycdlib.PyCdlib()
    iso.new()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        iso.open(str(outfile))
    assert(str(excinfo.value) == 'This object already has an ISO; either close it or create a new object')

    iso.close()

def test_hybrid_modify_in_place_not_initialized(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceonefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'f\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    foostr = b'foo\n'
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        iso.modify_file_in_place(io.BytesIO(foostr), len(foostr), '/FOO.;1', rr_name='foo', joliet_path='/foo')
    assert(str(excinfo.value) == 'This object is not initialized; call either open() or new() to create an ISO')

def test_hybrid_modify_in_place_read_only(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceonefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'f\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    with open(str(outfile), 'rb') as fp:
        iso.open_fp(fp)

        foostr = b'foo\n'
        with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
            iso.modify_file_in_place(io.BytesIO(foostr), len(foostr), '/FOO.;1', rr_name='foo', joliet_path='/foo')
        assert(str(excinfo.value) == 'To modify a file in place, the original ISO must have been opened in a write mode (r+, w, or a)')

        iso.close()

def test_hybrid_add_isohybrid_file_wrong_size(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceonefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    with open(os.path.join(str(indir), 'file.bin'), 'wb') as outfp:
        outfp.write(b'file')

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        iso.add_isohybrid(os.path.join(str(indir), 'file.bin'))
    assert(str(excinfo.value) == 'Invalid signature on boot file for iso hybrid')

    iso.close()

def test_hybrid_add_isohybrid_no_eltorito(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceonefile')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        iso.add_isohybrid('/usr/share/syslinux/isohdpfx.bin')
    assert(str(excinfo.value) == 'The ISO must have an El Torito Boot Record to add isohybrid support')

    iso.close()

def test_hybrid_eltorito_remove_not_initialized(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('eltoritonofiles')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        iso.rm_eltorito()
    assert(str(excinfo.value) == 'This object is not initialized; call either open() or new() to create an ISO')

def test_hybrid_eltorito_remove_not_present(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('eltoritonofiles')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        iso.rm_eltorito()
    assert(str(excinfo.value) == 'This ISO does not have an El Torito Boot Record')

    iso.close()

def test_hybrid_rmdir_not_initialized(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rmdir')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        iso.rm_directory('/DIR1')
    assert(str(excinfo.value) == 'This object is not initialized; call either open() or new() to create an ISO')

def test_hybrid_rmdir_slash(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rmdir')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        iso.rm_directory('/')
    assert(str(excinfo.value) == 'Cannot remove base directory')

    iso.close()

def test_hybrid_rmdir_not_dir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rmdir')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        iso.rm_directory('/FOO.;1')
    assert(str(excinfo.value) == 'Cannot remove a file with rm_directory (try rm_file instead)')

    iso.close()

def test_hybrid_rmdir_not_empty(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rmdir')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    dir1 = indir.mkdir('dir1')
    with open(os.path.join(str(dir1), 'bar'), 'wb') as outfp:
        outfp.write(b'bar\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        iso.rm_directory('/DIR1')
    assert(str(excinfo.value) == 'Directory must be empty to use rm_directory')

    iso.close()

def test_hybrid_rmfile_not_initialized(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolieteltoritoremove')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        iso.rm_file('/BOOT.;1')
    assert(str(excinfo.value) == 'This object is not initialized; call either open() or new() to create an ISO')

def test_hybrid_rmfile_bad_filename(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolieteltoritoremove')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        iso.rm_file('BOOT.;1')
    assert(str(excinfo.value) == 'Must be a path starting with /')

    iso.close()

def test_hybrid_rmfile_not_file(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rmdir')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        iso.rm_file('/DIR1')
    assert(str(excinfo.value) == 'Cannot remove a directory with rm_file (try rm_directory instead)')

    iso.close()

def test_hybrid_add_directory_not_initialized(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('onefile')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        iso.add_directory('/DIR1')
    assert(str(excinfo.value) == 'This object is not initialized; call either open() or new() to create an ISO')

def test_hybrid_addfile_not_initialized(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rmdir')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    foostr = b'foo\n'
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        iso.add_fp(io.BytesIO(foostr), len(foostr), '/FOO.;1')
    assert(str(excinfo.value) == 'This object is not initialized; call either open() or new() to create an ISO')

def test_hybrid_modify_in_place_bad_path(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'f\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile), 'r+b')

    foostr = b'foo\n'
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        iso.modify_file_in_place(io.BytesIO(foostr), len(foostr), 'foo', rr_name='foo', joliet_path='/foo')
    assert(str(excinfo.value) == 'Must be a path starting with /')

    iso.close()

def test_hybrid_modify_in_place_grow_file(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'f\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile), 'r+b')

    foostr = b'f'*2049
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        iso.modify_file_in_place(io.BytesIO(foostr), len(foostr), '/foo')
    assert(str(excinfo.value) == 'When modifying a file in-place, the number of extents for a file cannot change!')

    iso.close()

def test_hybrid_modify_in_place_modify_dir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceisolevel4onefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'f\n')
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile), 'r+b')

    foostr = b'foo\n'
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        iso.modify_file_in_place(io.BytesIO(foostr), len(foostr), '/dir1')
    assert(str(excinfo.value) == 'Cannot modify a directory with modify_file_in_place')

    iso.close()

def test_hybrid_joliet_isolevel4(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolietisolevel4')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-J', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.add_directory('/dir1', joliet_path='/dir1')

    do_a_test(iso, check_joliet_isolevel4)

    iso.close()

def test_hybrid_joliet_isolevel4_2(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolietisolevel4')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-J', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    foostr = b'foo\n'
    iso.add_fp(io.BytesIO(foostr), len(foostr), '/foo', joliet_path='/foo')

    do_a_test(iso, check_joliet_isolevel4)

    iso.close()

def test_hybrid_joliet_isolevel4_3(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolietisolevel4')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '4', '-no-pad',
                     '-J', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    foostr = b'foo\n'
    iso.add_fp(io.BytesIO(foostr), len(foostr), '/foo', joliet_path='/foo')

    iso.add_directory('/dir1', joliet_path='/dir1')

    do_a_test(iso, check_joliet_isolevel4)

    iso.close()

def test_hybrid_eltorito_remove_with_dir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('eltoritonofiles')
    outfile = str(indir)+'.iso'
    indir.mkdir('a')
    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.rm_eltorito()
    iso.rm_file('/BOOT.;1')
    iso.rm_directory('/A')

    do_a_test(iso, check_nofiles)

    iso.close()

def test_hybrid_modify_in_place_dirrecord_spillover(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceonefile')
    outfile = str(indir)+'.iso'
    dir1 = indir.mkdir('dir1')
    for i in range(1, 49):
        fname = os.path.join(str(dir1), 'foo%.2d' % (i))
        with open(fname, 'wb') as outfp:
            outfp.write(b'f\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()
    iso.open(str(outfile), 'r+b')
    foostr = b'foo\n'
    iso.modify_file_in_place(io.BytesIO(foostr), len(foostr), '/DIR1/FOO48.;1')
    iso.close()

    # Now open up the ISO with pycdlib and modify it.
    open_and_check(outfile, check_modify_in_place_spillover)

def test_hybrid_modify_in_place_dirrecord_spillover2(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('modifyinplaceonefile')
    outfile = str(indir)+'.iso'
    dir1 = indir.mkdir('dir1')
    for i in range(1, 49):
        fname = os.path.join(str(dir1), 'foo%.2d' % (i))
        with open(fname, 'wb') as outfp:
            outfp.write(b'f\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()
    iso.open(str(outfile), 'r+b')
    foostr = b'foo\n'
    iso.modify_file_in_place(io.BytesIO(foostr), len(foostr), '/DIR1/FOO40.;1')
    iso.close()

    # Now open up the ISO with pycdlib and modify it.
    open_and_check(outfile, check_modify_in_place_spillover)

def test_hybrid_shuffle_deep(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rrdeepreshuffle')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1').mkdir('dir2').mkdir('dir3').mkdir('dir4').mkdir('dir5').mkdir('dir6').mkdir('dir7').mkdir('dir8')
    with open(os.path.join(str(indir), 'dir1', 'dir2', 'dir3', 'dir4', 'dir5', 'dir6', 'dir7', 'dir8', 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    # Before making changes, save off the extent location of DIR1
    dir1 = iso.pvd.root_dir_record.children[2]
    assert(dir1.file_identifier() == b'DIR1')

    rr_moved = iso.pvd.root_dir_record.children[3]
    assert(rr_moved.file_identifier() == b'RR_MOVED')

    dir8_rr = rr_moved.children[2]
    assert(dir8_rr.file_identifier() == b'DIR8')
    assert(dir8_rr.rock_ridge.dr_entries.rr_record is not None)
    orig_pl = dir8_rr.children[1].rock_ridge.dr_entries.pl_record.parent_log_block_num

    dir2 = dir1.children[2]
    assert(dir2.file_identifier() == b'DIR2')

    dir3 = dir2.children[2]
    assert(dir3.file_identifier() == b'DIR3')

    dir4 = dir3.children[2]
    assert(dir4.file_identifier() == b'DIR4')

    dir5 = dir4.children[2]
    assert(dir5.file_identifier() == b'DIR5')

    dir6 = dir5.children[2]
    assert(dir6.file_identifier() == b'DIR6')

    dir7 = dir6.children[2]
    assert(dir7.file_identifier() == b'DIR7')

    dir8 = dir7.children[2]
    assert(dir8.file_identifier() == b'DIR8')

    assert(dir8.rock_ridge.dr_entries.cl_record is not None)
    orig_cl = dir8.rock_ridge.dr_entries.cl_record.child_log_block_num

    iso.add_directory('/A', rr_name='a')

    iso.force_consistency()

    new_cl = dir8.rock_ridge.dr_entries.cl_record.child_log_block_num
    assert(orig_cl != new_cl)

    new_pl = dir8_rr.children[1].rock_ridge.dr_entries.pl_record.parent_log_block_num
    assert(orig_pl != new_pl)

    iso.close()

def test_hybrid_hidden_file(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('onefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'aaaaaaaa'), 'wb') as outfp:
        outfp.write(b'aa\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))
    iso.set_hidden('/AAAAAAAA.;1')

    do_a_test(iso, check_hidden_file)

    iso.close()

def test_hybrid_hidden_dir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('onefile')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.set_hidden('/DIR1')

    do_a_test(iso, check_hidden_dir)

    iso.close()

def test_hybrid_clear_hidden_file(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('onefile')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-hidden', 'foo', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))
    iso.clear_hidden('/FOO.;1')

    do_a_test(iso, check_onefile)

    iso.close()

def test_hybrid_clear_hidden_dir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('onefile')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-hidden', 'dir1', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.clear_hidden('/DIR1')

    do_a_test(iso, check_onedir)

    iso.close()

@pytest.mark.skipif(find_executable('isohybrid') is None,
                    reason='syslinux not installed')
def test_hybrid_isohybrid_file_before(tmpdir):
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

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    foostr = b'foo\n'
    iso.add_fp(io.BytesIO(foostr), len(foostr), '/FOO.;1')

    do_a_test(iso, check_isohybrid_file_before)

    iso.close()

def test_hybrid_joliet_dirs_ptr_extent(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolietmanydirs')
    outfile = str(indir)+'.iso'
    numdirs = 214
    for i in range(1, 1+numdirs):
        indir.mkdir('dir%d' % i)
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.add_directory('/DIR215', joliet_path='/dir215')
    iso.add_directory('/DIR216', joliet_path='/dir216')

    do_a_test(iso, check_joliet_dirs_overflow_ptr_extent)

    iso.close()

def test_hybrid_joliet_dirs_ptr_extent2(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('jolietmanydirs')
    outfile = str(indir)+'.iso'
    numdirs = 216
    for i in range(1, 1+numdirs):
        indir.mkdir('dir%d' % i)
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.rm_directory('/DIR216', joliet_path='/dir216')

    do_a_test(iso, check_joliet_dirs_just_short_ptr_extent)

    iso.close()

def test_hybrid_joliet_dirs_add_ptr_extent(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('dirsaddptrextent')
    outfile = str(indir)+'.iso'
    numdirs = 293
    for i in range(1, 1+numdirs):
        indir.mkdir('dir%d' % i)
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.add_directory('/DIR294', joliet_path='/dir294')
    iso.add_directory('/DIR295', joliet_path='/dir295')

    do_a_test(iso, check_joliet_dirs_add_ptr_extent)

    iso.close()

def test_hybrid_joliet_dirs_rm_ptr_extent(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('dirsrmptrextent')
    outfile = str(indir)+'.iso'
    numdirs = 295
    for i in range(1, 1+numdirs):
        indir.mkdir('dir%d' % i)
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.rm_directory('/DIR294', joliet_path='/dir294')
    iso.rm_directory('/DIR295', joliet_path='/dir295')

    do_a_test(iso, check_joliet_dirs_rm_ptr_extent)

    iso.close()

def test_hybrid_joliet_rm_large_directory(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('dirsrmptrextent')
    outfile = str(indir)+'.iso'
    numdirs = 50
    for i in range(1, 1+numdirs):
        indir.mkdir('dir%d' % i)
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-J', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    for i in range(1, 1+numdirs):
        iso.rm_directory('/DIR%d' % i, joliet_path='/dir%d' % i)

    do_a_test(iso, check_joliet_nofiles)

    iso.close()

def test_hybrid_set_relocated_name_not_initialized(tmpdir):
    iso = pycdlib.PyCdlib()

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        iso.set_relocated_name('RR_MOVED', 'rr_moved')
    assert(str(excinfo.value) == 'This object is not initialized; call either open() or new() to create an ISO')

def test_hybrid_set_relocated_not_rockridge(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('setrelocatednotrr')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        iso.set_relocated_name('RR_MOVED', 'rr_moved')
    assert(str(excinfo.value) == 'Can only set the relocated name on a Rock Ridge ISO')

    iso.close()

def test_hybrid_set_relocated_change_name(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('setrelocatednotrr')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.set_relocated_name('RR_MOVED', 'rr_moved')

    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        iso.set_relocated_name('XX_MOVED', 'xx_moved')
    assert(str(excinfo.value) == 'Changing the existing rr_moved name is not allowed')

    iso.close()

def test_hybrid_set_relocated_same_name(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('setrelocatednotrr')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.set_relocated_name('RR_MOVED', 'rr_moved')

    iso.set_relocated_name('RR_MOVED', 'rr_moved')

    iso.close()

def test_hybrid_rr_hidden_relocated(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('setrelocatednotrr')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.set_relocated_name('_RR_MOVE', '.rr_moved')

    iso.add_directory('/DIR1', rr_name='dir1')
    iso.add_directory('/DIR1/DIR2', rr_name='dir2')
    iso.add_directory('/DIR1/DIR2/DIR3', rr_name='dir3')
    iso.add_directory('/DIR1/DIR2/DIR3/DIR4', rr_name='dir4')
    iso.add_directory('/DIR1/DIR2/DIR3/DIR4/DIR5', rr_name='dir5')
    iso.add_directory('/DIR1/DIR2/DIR3/DIR4/DIR5/DIR6', rr_name='dir6')
    iso.add_directory('/DIR1/DIR2/DIR3/DIR4/DIR5/DIR6/DIR7', rr_name='dir7')
    iso.add_directory('/DIR1/DIR2/DIR3/DIR4/DIR5/DIR6/DIR7/DIR8', rr_name='dir8')
    iso.add_directory('/DIR1/DIR2/DIR3/DIR4/DIR5/DIR6/DIR7/DIR8/DIR9', rr_name='dir9')

    foostr = b'foo\n'
    iso.add_fp(io.BytesIO(foostr), len(foostr), '/DIR1/DIR2/DIR3/DIR4/DIR5/DIR6/DIR7/DIR8/DIR9/FOO.;1', 'foo')

    do_a_test(iso, check_rr_relocated_hidden)

    iso.close()

def test_hybrid_rr_relocated_list_dir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('rrdeeplistdir')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1').mkdir('dir2').mkdir('dir3').mkdir('dir4').mkdir('dir5').mkdir('dir6').mkdir('dir7').mkdir('dir8').mkdir('dir9')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-rational-rock', '-o', str(outfile), str(indir)])

    iso = pycdlib.PyCdlib()
    iso.open(str(outfile))

    for index, child in enumerate(iso.list_dir('/dir1/dir2/dir3/dir4/dir5/dir6/dir7')):
        if index == 0:
            assert(child.is_dot())
        elif index == 1:
            assert(child.is_dotdot())
        elif index == 2:
            assert(child.file_identifier() == b'DIR8')
            assert(child.rock_ridge.name() == b'dir8')
            for index, child in enumerate(iso.list_dir('/dir1/dir2/dir3/dir4/dir5/dir6/dir7/dir8')):
                if index == 0:
                    assert(child.is_dot())
                elif index == 1:
                    assert(child.is_dotdot())
                    assert(child.rock_ridge.parent_link_record_exists())

        else:
            assert(False)

def test_hybrid_rm_hard_link(tmpdir):
    indir = tmpdir.mkdir('rmhardlink')
    outfile = str(indir)+'.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-o', str(outfile), str(indir)])
    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.rm_hard_link(iso_path='/FOO.;1')

    do_a_test(iso, check_nofiles)

    iso.close()

def test_hybrid_udf_onedir(tmpdir):
    indir = tmpdir.mkdir('udfnofiles')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-no-pad', '-iso-level', '1',
                     '-udf', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.add_directory('/DIR1', udf_path='/dir1')

    do_a_test(iso, check_udf_onedir)

    iso.close()

def test_hybrid_udf_twodirs(tmpdir):
    indir = tmpdir.mkdir('udfnofiles')
    outfile = str(indir)+'.iso'
    subprocess.call(['genisoimage', '-v', '-v', '-no-pad', '-iso-level', '1',
                     '-udf', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.add_directory('/DIR1', udf_path='/dir1')
    iso.add_directory('/DIR2', udf_path='/dir2')

    do_a_test(iso, check_udf_twodirs)

    iso.close()

def test_hybrid_udf_twodirs2(tmpdir):
    indir = tmpdir.mkdir('udfnofiles')
    outfile = str(indir)+'.iso'
    indir.mkdir('dir1')
    subprocess.call(['genisoimage', '-v', '-v', '-no-pad', '-iso-level', '1',
                     '-udf', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.add_directory('/DIR2', udf_path='/dir2')

    do_a_test(iso, check_udf_twodirs)

    iso.close()

def test_hybrid_udf_dir_oneshort(tmpdir):
    indir = tmpdir.mkdir('udfdironeshort')
    outfile = str(indir)+'.iso'
    for i in range(ord('a'), ord('v')):
        dirname = chr(i) * 64
        indir.mkdir(dirname)

    subprocess.call(['genisoimage', '-v', '-v', '-no-pad', '-iso-level', '1',
                     '-udf', '-o', str(outfile), str(indir)])

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.rm_directory('/UUUUUUUU', udf_path='/'+'u'*64)

    do_a_test(iso, check_udf_dir_oneshort)

    iso.close()

def test_hybrid_udf_zero_udf_file_entry(tmpdir):
    indir = tmpdir.mkdir('udfzerofileentry')
    outfile = str(indir)+'.iso'

    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')

    subprocess.call(['genisoimage', '-v', '-v', '-no-pad', '-iso-level', '3',
                     '-udf', '-o', str(outfile), str(indir)])

    # Now open up the ISO and zero out the UDF File Entry
    with open(str(outfile), 'r+b') as fp:
        fp.seek(261*2048)
        fp.write(b'\x00'*2048)

    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    do_a_test(iso, check_udf_zeroed_file_entry)

    iso.close()

def test_hybrid_udf_rm_zero_udf_file_entry(tmpdir):
    indir = tmpdir.mkdir('udfzerofileentry')
    outfile = str(indir)+'.iso'

    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')

    subprocess.call(['genisoimage', '-v', '-v', '-no-pad', '-iso-level', '3',
                     '-udf', '-o', str(outfile), str(indir)])

    # Now open up the ISO and zero out the UDF File Entry
    with open(str(outfile), 'r+b') as fp:
        fp.seek(261*2048)
        fp.write(b'\x00'*2048)

    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.rm_file(iso_path='/FOO.;1')
    iso.rm_file(udf_path='/foo')

    do_a_test(iso, check_udf_nofiles)

    iso.close()

def test_hybrid_udf_rm_hard_link_zero_udf_file_entry(tmpdir):
    indir = tmpdir.mkdir('udfzerofileentry')
    outfile = str(indir)+'.iso'

    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')

    subprocess.call(['genisoimage', '-v', '-v', '-no-pad', '-iso-level', '3',
                     '-udf', '-o', str(outfile), str(indir)])

    # Now open up the ISO and zero out the UDF File Entry
    with open(str(outfile), 'r+b') as fp:
        fp.seek(261*2048)
        fp.write(b'\x00'*2048)

    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    iso.rm_hard_link(iso_path='/FOO.;1')
    iso.rm_hard_link(udf_path='/foo')

    do_a_test(iso, check_udf_nofiles)

    iso.close()

def test_hybrid_udf_get_from_iso_zero_udf_file_entry(tmpdir):
    indir = tmpdir.mkdir('udfzerofileentry')
    outfile = str(indir)+'.iso'

    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')

    subprocess.call(['genisoimage', '-v', '-v', '-no-pad', '-iso-level', '3',
                     '-udf', '-o', str(outfile), str(indir)])

    # Now open up the ISO and zero out the UDF File Entry
    with open(str(outfile), 'r+b') as fp:
        fp.seek(261*2048)
        fp.write(b'\x00'*2048)

    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    out = io.BytesIO()
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as excinfo:
        iso.get_file_from_iso_fp(out, udf_path='/foo')
    assert(str(excinfo.value) == 'Cannot get the contents of an empty UDF File Entry')

    iso.close()

def test_hybrid_boot_record_retain_system_use(tmpdir):
    indir = tmpdir.mkdir('bootrecordretainsystemuse')
    outfile = str(indir)+'.iso'

    with open(os.path.join(str(indir), 'boot'), 'wb') as outfp:
        outfp.write(b'boot\n')
    subprocess.call(['genisoimage', '-v', '-v', '-iso-level', '1', '-no-pad',
                     '-c', 'boot.cat', '-b', 'boot', '-no-emul-boot',
                     '-o', str(outfile), str(indir)])

    # Now modify the boot record and add some stuff into system use
    system_use_offset = 17*2048 + 0x4b
    with open(str(outfile), 'r+b') as fp:
        fp.seek(system_use_offset)
        fp.write(b'hello')

    # Now open up the ISO with pycdlib and check some things out.
    iso = pycdlib.PyCdlib()

    iso.open(str(outfile))

    foostr = b'foo\n'
    iso.add_fp(io.BytesIO(foostr), len(foostr), '/FOO.;1')

    out = io.BytesIO()
    iso.write_fp(out)

    out.seek(system_use_offset)
    assert(out.read(5) == b'hello')

    iso.close()

# FIXME: write tests for 'empty' UDF File Entries (like on the Win2k8 ISO).
