# This is a example program to show how to use PyCdlib to open up a 'raw'
# Windows device.  That is, assuming that the real CD drive on a computer is
# 'D:', this example will open up a disc inserted into that drive and print
# all of the file names at the root of the ISO.

# Import pycdlib itself.
import pycdlib

# Create a new PyCdlib object.
iso = pycdlib.PyCdlib()

# Open up the raw Windows CD drive.
iso.open(r'\\.\D:')

# Now iterate through each of the files on the root of the ISO, printing out
# their names.
for child in iso.list_children(iso_path='/'):
    print(child.file_identifier())

# Close the ISO object.  After this call, the PyCdlib object has forgotten
# everything about the previous ISO, and can be re-used.
iso.close()
