# This is program to show how to use PyCdlib to create a new ISO that is
# bootable.

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
# This file will be used as the boot file on the bootable ISO.
bootstr = b'boot\n'
iso.add_fp(BytesIO(bootstr), len(bootstr), '/BOOT.;1')

# Once the bootable file is on the ISO, we need to link it to the boot catalog
# by calling add_eltorito.
iso.add_eltorito('/BOOT.;1', bootcatfile='/BOOT.CAT;1')

# Write out the ISO to the file called 'eltorito.iso'.  This will fully master
# the ISO, making it bootable.
iso.write('eltorito.iso')

# Close the ISO object.  After this call, the PyCdlib object has forgotten
# everything about the previous ISO, and can be re-used.
iso.close()
