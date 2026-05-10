import io
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

import pycdlib.utils
import pycdlib.pycdlibexception

def test_swab_32bit():
    assert(pycdlib.utils.swab_32bit(0x89) == 0x89000000)

def test_swab_32bit_bad_input():
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        pycdlib.utils.swab_32bit(-1)
    assert(str(excinfo.value) == 'Invalid integer passed to swab; must be unsigned 32-bits!')

def test_swab_16bit():
    assert(pycdlib.utils.swab_16bit(0x55aa) == 0xaa55)

def test_swab_16bit_bad_input():
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInternalError) as excinfo:
        pycdlib.utils.swab_16bit(-1)
    assert(str(excinfo.value) == 'Invalid integer passed to swab; must be unsigned 16-bits!')

def test_ceiling_div():
    assert(pycdlib.utils.ceiling_div(0, 2048) == 0)

def test_ceiling_div2():
    assert(pycdlib.utils.ceiling_div(2048, 2048) == 1)

def test_ceiling_div3():
    assert(pycdlib.utils.ceiling_div(2049, 2048) == 2)

def test_ceiling_div_nan():
    with pytest.raises(ZeroDivisionError) as exc_info:
        pycdlib.utils.ceiling_div(2048, 0)

def test_copy_data():
    infp = io.BytesIO()
    outfp = io.BytesIO()

    infp.write(b'\x00'*1)
    infp.seek(0)

    pycdlib.utils.copy_data(1, 8192, infp, outfp)

    assert(outfp.getvalue() == b'\x00')

def test_copy_data_short():
    infp = io.BytesIO()
    outfp = io.BytesIO()

    infp.write(b'\x00'*10)
    infp.seek(0)

    pycdlib.utils.copy_data(100, 8192, infp, outfp)

    assert(outfp.getvalue() == b'\x00'*10)

def test_encode_space_pad_too_short():
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as exc_info:
        pycdlib.utils.encode_space_pad(b'hello', 4, 'ascii')
    assert(str(exc_info.value) == 'Input string too long!')

def test_encode_space_pad_no_pad():
    assert(pycdlib.utils.encode_space_pad(b'hello', 5, 'ascii') == b'hello')

def test_encode_space_pad_one():
    assert(pycdlib.utils.encode_space_pad(b'hello', 6, 'ascii') == b'hello ')

def test_encode_space_pad_extra():
    assert(pycdlib.utils.encode_space_pad(b'hello', 11, 'utf-16_be') == b'\x00h\x00e\x00l\x00l\x00o\x00')

def test_normpath_double_slashes_beginning():
    assert(pycdlib.utils.normpath('//') == b'/')

def test_normpath_double_slashes_middle():
    assert(pycdlib.utils.normpath('/foo//bar') == b'/foo/bar')

def test_normpath_with_dotdot():
    assert(pycdlib.utils.normpath('/foo/bar/../baz') == b'/foo/baz')

def test_normpath_with_dotdot_after_slash():
    assert(pycdlib.utils.normpath('/../foo') == b'/foo')

def test_normpath_empty():
    assert(pycdlib.utils.normpath('') == b'.')

def save_and_set_tz(newtz):
    if 'TZ' in os.environ:
        oldtz = os.environ['TZ']
    else:
        oldtz = None

    os.environ['TZ'] = newtz
    time.tzset()

    return oldtz

def restore_tz(oldtz):
    if oldtz is not None:
        os.environ['TZ'] = oldtz
    else:
        del os.environ['TZ']
    time.tzset()

def test_gmtoffset_from_tm():
    oldtz = save_and_set_tz('US/Eastern')
    now = 1546914300
    # US/Eastern is GMT-5 hours, which is 5*60 = 300 minutes behind.  Since
    # the gmtoffset in ISO 9660 is in 15 minute intervals, this is 300/15 = 20
    # behind.
    assert(pycdlib.utils.gmtoffset_from_tm(now, time.localtime(now)) == -20)
    restore_tz(oldtz)

def test_gmtoffset_from_tm_day_rollover():
    # Setup the timezone to Tokyo
    oldtz = save_and_set_tz('Asia/Tokyo')

    # Asia/Tokyo is GMT+9 hours, which is 9*60 = 540 minutes ahead.  Since the
    # gmtoffset in ISO 9660 is in 15 minute intervals, this is 540/15 = 36
    # ahead.

    # This tm is carefully chosen so that the day of the week is the next day
    # in the Tokyo region.
    now = 1550417871
    assert(pycdlib.utils.gmtoffset_from_tm(now, time.localtime(now)) == 36)
    restore_tz(oldtz)

def test_gmtoffset_from_tm_2023_rollover():
    oldtz = save_and_set_tz('Australia/Sydney')

    # Australia/Sydney is GMT+11 hours, which is 11*60 = 660 minutes ahead.
    # Since the gmtoffset in ISO 9660 is in 15 minute intervals, this is
    # 660/15 = 44 ahead.

    now = 1672530300
    assert(pycdlib.utils.gmtoffset_from_tm(now, time.localtime(now)) == 44)

    restore_tz(oldtz)

def test_zero_pad():
    fp = io.BytesIO()
    pycdlib.utils.zero_pad(fp, 5, 10)
    assert(fp.getvalue() == b'\x00'*5)

def test_zero_pad_no_pad():
    fp = io.BytesIO()
    pycdlib.utils.zero_pad(fp, 5, 5)
    assert(fp.getvalue() == b'')

def test_zero_pad_negative_pad():
    fp = io.BytesIO()
    pycdlib.utils.zero_pad(fp, 5, 4)
    assert(fp.getvalue() == b'\x00'*3)

def test_starts_with_slash():
    assert(pycdlib.utils.starts_with_slash(b'/'))

def test_starts_with_slash_no_slash():
    assert(not pycdlib.utils.starts_with_slash(b'foo/bar'))

def test_starts_with_slash_double_slash():
    assert(pycdlib.utils.starts_with_slash(b'//foo/bar'))

def test_split_path():
    assert(pycdlib.utils.split_path(b'/foo/bar') == [b'foo', b'bar'])

def test_split_path_no_leading_slash():
    with pytest.raises(pycdlib.pycdlibexception.PyCdlibInvalidInput) as exc_info:
        pycdlib.utils.split_path(b'foo/bar')
    assert(str(exc_info.value) == 'Must be a path starting with /')

def test_split_path_single():
    assert(pycdlib.utils.split_path(b'/foo') == [b'foo'])

def test_split_path_slash_only():
    assert(pycdlib.utils.split_path(b'/') == [b''])

def test_split_path_trailing_slash():
    assert(pycdlib.utils.split_path(b'/foo/') == [b'foo', b''])

def test_file_object_supports_binary_bytesio():
    fp = io.BytesIO()
    assert(pycdlib.utils.file_object_supports_binary(fp))

def test_truncate_basename_isolevel4():
    assert(pycdlib.utils.truncate_basename('foo', 4, False) == 'foo')

def test_truncate_basename_isolevel3():
    assert(pycdlib.utils.truncate_basename('foo', 3, False) == 'FOO')

def test_mangle_file_for_iso9660_isolevel4_no_ext():
    assert(pycdlib.utils.mangle_file_for_iso9660('foo', 4) == ('foo', ''))

def test_mangle_file_for_iso9660_isolevel4_with_ext():
    assert(pycdlib.utils.mangle_file_for_iso9660('foo.txt', 4) == ('foo', 'txt'))

def test_mangle_file_for_iso9660_isolevel3_with_empty_ext():
    assert(pycdlib.utils.mangle_file_for_iso9660('foo.', 3) == ('FOO_', ';1'))

def test_file_object_supports_binary_real_file(tmpdir):
    testout = tmpdir.join('foo')
    with open(str(testout), 'wb') as outfp:
        assert(pycdlib.utils.file_object_supports_binary(outfp))

def test_file_object_does_not_support_binary_real_file(tmpdir):
    testout = tmpdir.join('foo')
    with open(str(testout), 'w') as outfp:
        assert(not pycdlib.utils.file_object_supports_binary(outfp))
