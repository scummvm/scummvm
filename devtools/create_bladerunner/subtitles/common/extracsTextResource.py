#!/usr/bin/env python
# -*- coding: utf-8 -*-
#

MY_MODULE_VERSION = "1.70"
MY_MODULE_NAME = "extracTextResource"

# Template for EXTRA.TRE sheet's values ((row 2 and below)
EXTRAC_TEXT_RESOURCE_TUPLE_LIST = [
	(0, "Thomas Fach-Pedersen", ""),
	(1, "Peter Kohaut", ""),
	(2, "Eugene Sandulenko", ""),
	(3, "Thanasis Antoniou", ""),
	(4, "Xavier Mohedano", ""),
	(5, "Víctor González Fraile", ""),
	(6, "Sergio Carmona", ""),
	(7, "Mark Benninghofen", ""),
	(8, "Frank Klepacki", ""),
	(9, "ScummVM", ""),
	(10, "Westwood Studios", ""),
	(11, "ix", ""),
	(12, "Jarrod O'Rafferty", ""),
	(13, "Sean Leong (JakeSteven1980)", ""),
	(14, ":...:", "REQUIRED")
 ]
#
#
#
class extracTextResource:
	m_traceModeEnabled = True
	# traceModeEnabled is bool to enable more printed debug messages
	def __init__(self, traceModeEnabled = True):
		self.m_traceModeEnabled = traceModeEnabled
		return

	def printExtracTemplate(self):
		for (idTre, textTre) in EXTRAC_TEXT_RESOURCE_TUPLE_LIST:
			print ("%s\t%s" % (idTre, textTre))
		return

	def getExtracEntriesList(self):
		return EXTRAC_TEXT_RESOURCE_TUPLE_LIST

if __name__ == '__main__':
	#	 main()
	print ("[Debug] Running %s (%s) as main module" % (MY_MODULE_NAME, MY_MODULE_VERSION))
	traceModeEnabled = False
	excrTRInstance = extracTextResource(traceModeEnabled)
	excrTRInstance.printExtracTemplate()

else:
	#debug
	#print ("[Debug] Running %s (%s) imported from another module" % (MY_MODULE_NAME, MY_MODULE_VERSION))
	pass
