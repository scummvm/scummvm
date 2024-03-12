#!/usr/bin/env python
# -*- coding: utf-8 -*-
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

import sortBladeRunnerWavs02
if __name__ == "__main__":
	sortBladeRunnerWavs02.main(sys.argv[0:])
