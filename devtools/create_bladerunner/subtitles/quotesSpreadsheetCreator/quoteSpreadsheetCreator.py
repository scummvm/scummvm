#!/usr/bin/env python2.7
# -*- coding: UTF-8 -*-
import sys
if not (sys.version_info[0] == 2 and  sys.version_info[1] == 7):
	sys.stdout.write("Error:: Blade Runner Quotes Spreadsheet Creator script requires Python 2.7\n")
	sys.exit(1)

import sortBladeRunnerWavs02
if __name__ == "__main__":
	sortBladeRunnerWavs02.main(sys.argv[0:])