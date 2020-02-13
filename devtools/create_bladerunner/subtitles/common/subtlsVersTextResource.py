#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#

my_module_version = "1.60"
my_module_name = "subtlsVersTextResource"

# Template for SBTLVERS.TRE sheet's values ((row 2 and below)
SBTLVERS_TEXT_RESOURCE_TUPLE_LIST = [
	(0, "ScummVM Team", "Credits"),
	(1, "6", "Version (an incremental number)"),
	(2, "##:##:## ##/##/####", "Placeholder – Date of compilation (HH:mm:ss dd/mm/yyyy)"),
	(3, "EFIGS", "Placeholder – Language mode")
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
			print "%s\t%s" % (idTre, textTre)
		return
		
	def getSbtlVersEntriesList(self):
		return SBTLVERS_TEXT_RESOURCE_TUPLE_LIST

if __name__ == '__main__':
	#	 main()
	print "[Debug] Running %s as main module" % (my_module_name)
	traceModeEnabled = False
	sbtlVersTRInstance = sbtlVersTextResource(traceModeEnabled)
	sbtlVersTRInstance.printSbtlVersTemplate()
	
else:
	#debug
	#print "[Debug] Running	 %s imported from another module" % (my_module_name)
	pass
	