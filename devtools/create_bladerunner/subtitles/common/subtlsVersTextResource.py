#!/usr/bin/env python
# -*- coding: utf-8 -*-
#

MY_MODULE_VERSION = "1.70"
MY_MODULE_NAME = "subtlsVersTextResource"

# Template for SBTLVERS.TRE sheet's values ((row 2 and below)
SBTLVERS_TEXT_RESOURCE_TUPLE_LIST = [
	(0, "ScummVM Team", "Credits"),
	(1, "9", "Version (an incremental number)"),
	(2, "##:##:## ##/##/####", "Placeholder – Date of compilation (HH:mm:ss dd/mm/yyyy)"),
	(3, "EFIGS", "Placeholder – Language mode"),
	(4, "Copyright (C) 2019-2022  ScummVM team", "Copyright"),
	(5, "", "Font type"),
	(6, "", "Font name"),
	(7, "", "License"),
	(8, "", "License link")
 ]
#
#
#
class sbtlVersTextResource:
	m_traceModeEnabled = True
	# traceModeEnabled is bool to enable more printed debug messages
	def __init__(self, traceModeEnabled = True):
		self.m_traceModeEnabled = traceModeEnabled
		return

	def printSbtlVersTemplate(self):
		for (idTre, textTre) in SBTLVERS_TEXT_RESOURCE_TUPLE_LIST:
			print ("%s\t%s" % (idTre, textTre))
		return

	def getSbtlVersEntriesList(self):
		return SBTLVERS_TEXT_RESOURCE_TUPLE_LIST

if __name__ == '__main__':
	#	 main()
	print ("[Debug] Running %s (%s) as main module" % (MY_MODULE_NAME, MY_MODULE_VERSION))
	traceModeEnabled = False
	sbtlVersTRInstance = sbtlVersTextResource(traceModeEnabled)
	sbtlVersTRInstance.printSbtlVersTemplate()

else:
	#debug
	#print ("[Debug] Running %s (%s) imported from another module" % (MY_MODULE_NAME, MY_MODULE_VERSION))
	pass
