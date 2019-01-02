#!/usr/bin/env python
# -*- coding: UTF-8 -*-
import sys
if not (sys.version_info[0] == 2 and  sys.version_info[1] == 7):
	sys.stdout.write("[Error] Blade Runner MIX Resource Creator script requires Python 2.7\n")
	sys.exit(1)

import packBladeRunnerMIXFromPCTLKXLS04
if __name__ == "__main__":
	packBladeRunnerMIXFromPCTLKXLS04.main(sys.argv[0:])