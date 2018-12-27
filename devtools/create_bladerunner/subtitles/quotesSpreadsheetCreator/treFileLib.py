#!/usr/bin/env python2.7
# -*- coding: UTF-8 -*-
#
import os, sys, shutil
import struct
from struct import *

my_module_version = "0.50"
my_module_name = "treFileLib"


class TreHeader:
	numOfTextResources = -1
	def __init__(self):
		return


class treFile:
	m_header = TreHeader()
	stringEntriesLst = []  # list of two-value tuples. First value is ID, second value is String content
	stringOffsets = []
	def __init__(self):
		del self.stringEntriesLst[:]
		del self.stringOffsets[:]
		return

	def loadTreFile(self, treBytesBuff, maxLength):
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
			print "Total texts in TRE: %d" % (self.header().numOfTextResources)
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
			#  TODO +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			absStartOfIndexTable = 4
			#absStartOfOffsetTable = absStartOfIndexTable + (self.header().numOfTextResources * 4)
			#absStartOfStringTable = absStartOfOffsetTable + ((self.header().numOfTextResources+1) * 4)

			#print "buffer type " , type(treBytesBuff) # it is str

			for idx in range(0, self.header().numOfTextResources):
				currOffset = self.stringOffsets[idx] + absStartOfIndexTable
				# the buffer (treBytesBuff) where we read the TRE file into, is "str" type but contains multiple null terminated strings
				# the solution here (to not get out of index errors when reading the null terminator points) is
				# to split the substring starting at the indicated offset each time, at the null character, and get the first string token.
				# This works ok.
				#
				allTextsFound = treBytesBuff[currOffset:].split('\x00')
				# check "problematic" character cases:
				if  currOffset == 5982 or currOffset == 6050 or currOffset == 2827  or currOffset == 2880:
				 	print "Offs: %d\tFound String: %s" % ( currOffset,''.join(allTextsFound[0]) )
					 #print "Offs: %d\tFound String: %s" % ( currOffset,''.join(allTextsFound[0]) )
				(theId, stringOfIdx) = self.stringEntriesLst[idx]
				self.stringEntriesLst[idx] = (theId, ''.join(allTextsFound[0]))
				#print "ID: %d\tFound String: %s" % ( theId,''.join(allTextsFound[0]) )
			return True
  		except:
			print "Loading failure!"
			return False

	def header(self):
		return self.m_header
#
#
#
if __name__ == '__main__':
	#	 main()
	print "Running %s as main module" % (my_module_name)
	# assumes a file of name ACTORS.TRE in same directory
	inTREFile = None
	errorFound = False
	try:
		inTREFile = open(os.path.join('.','ACTORS.TRE'), 'rb')
	except:
		errorFound = True
		print "Unexpected error:", sys.exc_info()[0]
		raise
	if not errorFound:
		allOfTreFileInBuffer = inTREFile.read()
		treFileInstance = treFile()
		if (treFileInstance.loadTreFile(allOfTreFileInBuffer, len(allOfTreFileInBuffer))):
			print "TRE file loaded successfully!"
		else:
			print "Error while loading TRE file!"
		inTREFile.close()
else:
	#debug
	#print "Running	 %s imported from another module" % (my_module_name)
	pass