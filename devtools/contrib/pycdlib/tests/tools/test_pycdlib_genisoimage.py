import os
import subprocess

import pytest

pycdlib_root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
pycdlib_exe = os.path.join(pycdlib_root, 'tools', 'pycdlib-genisoimage')


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


class ProcessException(Exception):
    def __init__(self, msg):
        Exception.__init__(self, msg)


def run_process(cmdline):
    if not 'LD_LIBRARY_PATH' in os.environ:
        os.environ['LD_LIBRARY_PATH'] = ''

    process = subprocess.Popen(cmdline,
                               env={
                                   'LD_LIBRARY_PATH': os.environ['LD_LIBRARY_PATH'],
                                   'PATH': os.environ['PATH'],
                                   'PYTHONPATH': pycdlib_root,
                               },
                               stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE)

    out, err = process.communicate()

    ret = process.wait()
    if ret != 0:
        raise ProcessException('Process failed: %s\n%s' % (out, err))

    return out, err


@pytest.mark.skipif(find_executable('isosize') is None or
                    find_executable('isovfy') is None,
                    reason='isosize not installed')
def test_pycdlib_genisoimage_nofiles(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('nofiles')
    outfile = str(indir) + '.iso'

    def _do_test(binary):
        run_process([binary, '-v', '-iso-level', '1', '-no-pad', '-o',
                     str(outfile), str(indir)])
        run_process(['isovfy', str(outfile)])
        out, err = run_process(['isosize', str(outfile)])
        size = int(out.strip())
        assert(size == 49152)

    _do_test('genisoimage')
    _do_test(pycdlib_exe)


@pytest.mark.skipif(find_executable('isosize') is None or
                    find_executable('isovfy') is None or
                    find_executable('iso-read') is None,
                    reason='isosize not installed')
def test_pycdlib_genisoimage_onedir(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('onedir')
    outfile = str(indir) + '.iso'
    dir1 = indir.mkdir('dir1')
    with open(os.path.join(str(dir1), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')

    def _do_test(binary):
        run_process([binary, '-v', '-iso-level', '1', '-no-pad', '-o',
                     str(outfile), str(indir)])
        run_process(['isovfy', str(outfile)])
        out, err = run_process(['isosize', str(outfile)])
        genisoimage_size = int(out.strip())
        assert(genisoimage_size == 53248)
        foocheck = os.path.join(str(tmpdir), 'foocheck')
        out, err = run_process(['iso-read', '-i', str(outfile), '-e', 'dir1/foo', '-o', foocheck])
        with open(foocheck, 'rb') as infp:
            assert(infp.read() == b'foo\n')

    _do_test('genisoimage')
    _do_test(pycdlib_exe)


@pytest.mark.skipif(find_executable('isosize') is None or
                    find_executable('isovfy') is None or
                    find_executable('iso-read') is None,
                    reason='isosize not installed')
def test_pycdlib_genisoimage_onefile(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('onedir')
    outfile = str(indir) + '.iso'
    with open(os.path.join(str(indir), 'foo'), 'wb') as outfp:
        outfp.write(b'foo\n')

    def _do_test(binary):
        run_process([binary, '-v', '-iso-level', '1', '-no-pad', '-o',
                     str(outfile), str(indir)])
        run_process(['isovfy', str(outfile)])
        out, err = run_process(['isosize', str(outfile)])
        genisoimage_size = int(out.strip())
        assert(genisoimage_size == 51200)
        foocheck = os.path.join(str(tmpdir), 'foocheck')
        out, err = run_process(['iso-read', '-i', str(outfile), '-e', 'foo', '-o', foocheck])
        with open(foocheck, 'rb') as infp:
            assert(infp.read() == b'foo\n')

    _do_test('genisoimage')
    _do_test(pycdlib_exe)


@pytest.mark.skipif(find_executable('isosize') is None or
                    find_executable('isovfy') is None or
                    find_executable('iso-read') is None,
                    reason='isosize not installed')
def test_pycdlib_genisoimage_file_cmdline(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('onedir')
    outfile = str(indir) + '.iso'
    foofile = os.path.join(str(indir), 'foo')
    with open(foofile, 'wb') as outfp:
        outfp.write(b'foo\n')

    def _do_test(binary):
        run_process([binary, '-v', '-iso-level', '1', '-no-pad', '-o',
                     str(outfile), foofile])
        run_process(['isovfy', str(outfile)])
        out, err = run_process(['isosize', str(outfile)])
        genisoimage_size = int(out.strip())
        assert(genisoimage_size == 51200)
        foocheck = os.path.join(str(tmpdir), 'foocheck')
        out, err = run_process(['iso-read', '-i', str(outfile), '-e', 'foo', '-o', foocheck])
        with open(foocheck, 'rb') as infp:
            assert(infp.read() == b'foo\n')

    _do_test('genisoimage')
    _do_test(pycdlib_exe)


@pytest.mark.skipif(find_executable('isosize') is None or
                    find_executable('isovfy') is None or
                    find_executable('iso-read') is None,
                    reason='isosize not installed')
def test_pycdlib_genisoimage_boot_file_cmdline(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('onedir')
    outfile = str(indir) + '.iso'
    bootfile = os.path.join(str(indir), 'boot')
    with open(bootfile, 'wb') as outfp:
        outfp.write(b'boot\n')

    def _do_test(binary):
        run_process([binary, '-v', '-iso-level', '1', '-no-pad', '-b', 'boot',
                     '-c', 'boot.cat', '-no-emul-boot', '-o', str(outfile),
                     bootfile])
        run_process(['isovfy', str(outfile)])
        out, err = run_process(['isosize', str(outfile)])
        genisoimage_size = int(out.strip())
        assert(genisoimage_size == 55296)
        bootcheck = os.path.join(str(tmpdir), 'bootcheck')
        out, err = run_process(['iso-read', '-i', str(outfile), '-e', 'boot', '-o', bootcheck])
        with open(bootcheck, 'rb') as infp:
            assert(infp.read() == b'boot\n')

    _do_test('genisoimage')
    _do_test(pycdlib_exe)


def test_pycdlib_genisoimage_bootfile_bad(tmpdir):
    # First set things up, and generate the ISO with genisoimage.
    indir = tmpdir.mkdir('onedir')
    outfile = str(indir) + '.iso'

    def _do_test(binary):
        try:
            run_process([binary, '-v', '-iso-level', '1', '-no-pad', '-b', 'boot',
                         '-c', 'boot.cat', '-no-emul-boot', '-o', str(outfile),
                         str(indir)])
        except ProcessException as e:
            err1 = "Uh oh, I cant find the boot image 'boot'" in str(e)
            assert(err1)

    _do_test('genisoimage')
    _do_test(pycdlib_exe)
