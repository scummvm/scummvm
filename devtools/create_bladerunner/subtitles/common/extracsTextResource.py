#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#

my_module_version = "1.60"
my_module_name = "extracTextResource"

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
	(12, "jakejakejake1234", ""),
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
			print "%s\t%s" % (idTre, textTre)
		return

	def getExtracEntriesList(self):
		return EXTRAC_TEXT_RESOURCE_TUPLE_LIST

if __name__ == '__main__':
	#	 main()
	print "[Debug] Running %s as main module" % (my_module_name)
	traceModeEnabled = False
	excrTRInstance = extracTextResource(traceModeEnabled)
	excrTRInstance.printExtracTemplate()

else:
	#debug
	#print "[Debug] Running	 %s imported from another module" % (my_module_name)
	pass
