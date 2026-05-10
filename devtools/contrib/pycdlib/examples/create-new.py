# This is a simple program to show how to use PyCdlib to create a new
# ISO, with one file and one directory on it.

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

# Create a new PyCdlib object.
iso = pycdlib.PyCdlib()

# Create a new ISO, accepting all of the defaults.
iso.new()

# Add a new file to the ISO, with the contents coming from the file object.
# Note that the file object must remain open for the lifetime of the PyCdlib
# object, as the PyCdlib object uses it for internal operations.  Also note that
# the filename passed here is the filename the data will get assigned on the
# final ISO; it must begin with a forward slash, and according to ISO9660 must
# have a '.', and a semicolon followed by a number.  PyCdlib will raise a
# PyCdlibException if any of the rules for an ISO9660 filename are violated.
foostr = b'foo\n'
iso.add_fp(BytesIO(foostr), len(foostr), '/FOO.;1')

# Add a new directory to the ISO.  Like the filename above, ISO9660 directory
# names must conform to certain standards, and PyCdlib will raise a
# PyCdlibException if those standards are not met.
iso.add_directory('/DIR1')

# Write out the ISO to the file called 'new.iso'.  This will fully master the
# ISO, creating a file that can be burned onto a CD.
iso.write('new.iso')

# Close the ISO object.  After this call, the PyCdlib object has forgotten
# everything about the previous ISO, and can be re-used.
iso.close()
