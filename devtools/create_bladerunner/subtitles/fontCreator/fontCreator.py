#!/usr/bin/env python
# -*- coding: UTF-8 -*-
sysLibFound = False
try:
	import sys
except ImportError:
	print ("[Error] sys python library is required to be installed!")
else:
	sysLibFound = True

if (not sysLibFound):
	sys.stdout.write("[Error] Errors were found when trying to import required python libraries\n")
	sys.exit(1)

if not (sys.version_info[0] == 2 and  sys.version_info[1] == 7):
	sys.stdout.write("[Error] Blade Runner Font Creator script requires Python 2.7\n")
	sys.exit(1)

import grabberFromPNG17BR
if __name__ == "__main__":
	grabberFromPNG17BR.main(sys.argv[0:])