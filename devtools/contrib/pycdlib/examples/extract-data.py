# This is a simple program to show how to use PyCdlib to extract data from the
# ISO.

# Import standard python modules.
import sys
try:
    from cStringIO import StringIO as BytesIO
except ImportError:
    from io import BytesIO

# Import pycdlib itself.
import pycdlib

# Check that there are enough command-line arguments.
if len(sys.argv) != 1:
    print('Usage: %s' % (sys.argv[0]))
    sys.exit(1)


# First we'll create a new ISO and write it out (see create-new.py for more
# information about these steps).
iso = pycdlib.PyCdlib()
iso.new()
foostr = b'foo\n'
iso.add_fp(BytesIO(foostr), len(foostr), '/FOO.;1')
out = BytesIO()
iso.write_fp(out)
iso.close()

# Now, let's open up the ISO and extract the contents of the FOO.;1 file.
iso.open_fp(out)
extracted = BytesIO()
# Use the get_file_from_iso_fp() API to extract the named filename into the file
# descriptor.
iso.get_file_from_iso_fp(extracted, iso_path='/FOO.;1')
iso.close()

print(extracted.getvalue().decode('utf-8'))
