#!/usr/bin/env python
# -*- coding: UTF-8 -*-
import sys
if not (sys.version_info[0] == 2 and  sys.version_info[1] == 7):
	sys.stdout.write("Error:: Blade Runner Font Creator script requires Python 2.7\n")
	sys.exit(1)

import grabberFromPNG17BR
if __name__ == "__main__":
	grabberFromPNG17BR.main(sys.argv[0:])