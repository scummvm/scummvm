#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
osLibFound = False
sysLibFound = False
shutilLibFound = False
structLibFound = False

try:
	import os
except ImportError:
	print "[Error] os python library is required to be installed!"
else:
	osLibFound = True
	
try:
	import sys
except ImportError:
	print "[Error] sys python library is required to be installed!"
else:
	sysLibFound = True

try:
	import struct
except ImportError:
	print "[Error] struct python library is required to be installed!"
else:
	structLibFound = True

if 	(not osLibFound) \
	or (not sysLibFound) \
	or (not structLibFound):
	sys.stdout.write("[Error] Errors were found when trying to import required python libraries\n")
	sys.exit(1)
	
from struct import *

MY_MODULE_VERSION = "0.50"
MY_MODULE_NAME = "treFileLib"


class TreHeader(object):
	numOfTextResources = -1
	def __init__(self):
		return


class treFile(object):
	m_header = TreHeader()
	simpleTextResourceFileName = 'GENERIC.TRE'
	stringEntriesLst = []  # list of two-value tuples. First value is ID, second value is String content
	stringOffsets = []
	m_traceModeEnabled = False
	
	# traceModeEnabled is bool to enable more printed debug messages
	def __init__(self, traceModeEnabled = True):
		del self.stringEntriesLst[:]
		del self.stringOffsets[:]
		self.simpleTextResourceFileName = 'GENERIC.TRE'
		self.m_traceModeEnabled = traceModeEnabled
		return
		
	def loadTreFile(self, treBytesBuff, maxLength, treFileName):
		self.simpleTextResourceFileName = treFileName
		offsInTreFile = 0
		#
		# parse TRE file fields for header
		#
		try:
			tmpTuple = struct.unpack_from('I', treBytesBuff, offsInTreFile)  # unsigned integer 4 bytes
			self.header().numOfTextResources = tmpTuple[0]
			offsInTreFile += 4
			#
			# string IDs table (each entry is unsigned integer 4 bytes)
			#
			if self.m_traceModeEnabled:
				print "[Info] Total texts in Text Resource file: %d" % (self.header().numOfTextResources)
			for idx in range(0, self.header().numOfTextResources):
				tmpTuple = struct.unpack_from('I', treBytesBuff, offsInTreFile)  # unsigned integer 4 bytes
				self.stringEntriesLst.append( (tmpTuple[0], '') )
				offsInTreFile += 4
				
			# string offsets table (each entry is unsigned integer 4 bytes)
			for idx in range(0, self.header().numOfTextResources):
				tmpTuple = struct.unpack_from('I', treBytesBuff, offsInTreFile)  # unsigned integer 4 bytes
				self.stringOffsets.append( tmpTuple[0] )
				offsInTreFile += 4
			#
			# strings (all entries are null terminated)
			#  TODO +++
			absStartOfIndexTable = 4
			#absStartOfOffsetTable = absStartOfIndexTable + (self.header().numOfTextResources * 4)
			#absStartOfStringTable = absStartOfOffsetTable + ((self.header().numOfTextResources+1) * 4)
			
			#print "[Debug] buffer type: " , type(treBytesBuff) # it is str
			
			for idx in range(0, self.header().numOfTextResources):
				currOffset = self.stringOffsets[idx] + absStartOfIndexTable
				# the buffer (treBytesBuff) where we read the TRE file into, is "str" type but contains multiple null terminated strings
				# the solution here (to not get out of index errors when reading the null terminator points) is
				# to split the substring starting at the indicated offset each time, at the null character, and get the first string token.
				# This works ok.
				#
				allTextsFound = treBytesBuff[currOffset:].split('\x00')
				### check "problematic" character cases:
				##if self.m_traceModeEnabled:
				##	if  currOffset == 5982 or currOffset == 6050 or currOffset == 2827  or currOffset == 2880:
				##		print "[Debug] Offs: %d\tFound String: %s" % ( currOffset,''.join(allTextsFound[0]) )
				(theId, stringOfIdx) = self.stringEntriesLst[idx]
				self.stringEntriesLst[idx] = (theId, ''.join(allTextsFound[0]))
				if self.m_traceModeEnabled:
					print "[Trace] ID: %d\tFound String: %s" % ( theId,''.join(allTextsFound[0]) )
			return True
  		except:
			print "[Error] Loading Text Resource %s failed!" % (self.simpleTextResourceFileName)
			return False
			
	def header(self):
		return self.m_header
#
#
#
if __name__ == '__main__':
	#	 main()
	errorFound = False
	# By default assumes a file of name ACTORS.TRE in same directory
	# otherwise tries to use the first command line argument as input file
	inTREFile = None
	inTREFileName =  'ACTORS.TRE'
	
	if len(sys.argv[1:])  > 0 \
		and os.path.isfile(os.path.join('.', sys.argv[1])) \
		and len(sys.argv[1]) >= 5 \
		and sys.argv[1][-3:].upper() == 'TRE':
		inTREFileName = sys.argv[1]
		print "[Info] Attempting to use %s as input TRE file..." % (inTREFileName)
	elif os.path.isfile(os.path.join('.', inTREFileName)):
		print "[Info] Using default %s as input TRE file..." % (inTREFileName)
	else:
		print "[Error] No valid input file argument was specified and default input file %s is missing." % (inTREFileName)
		errorFound = True
	
	if not errorFound:
		try:
			print "[Info] Opening %s" % (inTREFileName)
			inTREFile = open(os.path.join('.',inTREFileName), 'rb')
		except:
			errorFound = True
			print "[Error] Unexpected event: ", sys.exc_info()[0]
			raise
		if not errorFound:
			allOfTreFileInBuffer = inTREFile.read()
			treFileInstance = treFile(True)
			if treFileInstance.m_traceModeEnabled:
				print "[Debug] Running %s (%s) as main module" % (MY_MODULE_NAME, MY_MODULE_VERSION)
			if treFileInstance.loadTreFile(allOfTreFileInBuffer, len(allOfTreFileInBuffer), inTREFileName):
				print "[Info] Text Resource file loaded successfully!"
			else:
				print "[Error] Error while loading Text Resource file!"
			inTREFile.close()
else:
	#debug
	#print "[Debug] Running %s (%s) imported from another module" % (MY_MODULE_NAME, MY_MODULE_VERSION)
	pass
