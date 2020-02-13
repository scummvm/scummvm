#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
osLibFound = False
sysLibFound = False
shutilLibFound = False
structLibFound = False
imagePilLibFound = False

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
	
try:
	from PIL import Image
except ImportError:
	print "[Error] Image python library (PIL) is required to be installed!"
else:
	imagePilLibFound = True

if 	(not osLibFound) \
	or (not sysLibFound) \
	or (not structLibFound) \
	or (not imagePilLibFound):
	sys.stdout.write("[Error] Errors were found when trying to import required python libraries\n")
	sys.exit(1)

from struct import *

MY_MODULE_VERSION = "0.80"
MY_MODULE_NAME    = "fonFileLib"

class FonHeader(object):
	maxEntriesInTableOfDetails = -1 # this is probably always the number of entries in table of details, but it won't work for the corrupted TAHOMA18.FON file
	maxGlyphWidth = -1              # in pixels
	maxGlyphHeight = -1             # in pixels
	graphicSegmentByteSize = -1     # Graphic segment byte size
	
	def __init__(self):
		return


class fonFile(object):
	m_header = FonHeader()
	
	simpleFontFileName = 'GENERIC.FON'
	realNumOfCharactersInImageSegment = 0 # this is used for the workaround for the corrupted TAHOME18.FON
	nonEmptyCharacters = 0
	
	glyphDetailEntriesLst = [] # list of 5-value tuples. Tuple values are (X-offset, Y-offset, Width, Height, Offset in Graphics segment)
	glyphPixelData = None      # buffer of pixel data for glyphs
	
	m_traceModeEnabled = False
	
	# traceModeEnabled is bool to enable more printed debug messages
	def __init__(self, traceModeEnabled = True):
		del self.glyphDetailEntriesLst[:]
		self.glyphPixelData = None # buffer of pixel data for glyphs
		self.simpleFontFileName = 'GENERIC.FON'
		self.realNumOfCharactersInImageSegment = 0 # this is used for the workaround for the corrupted TAHOME18.FON
		self.nonEmptyCharacters = 0
		self.m_traceModeEnabled = traceModeEnabled
		
		return
		
	def loadFonFile(self, fonBytesBuff, maxLength, fonFileName):
		self.simpleFontFileName =  fonFileName
		
		offsInFonFile = 0
		localLstOfDataOffsets = []
		del localLstOfDataOffsets[:]
		#
		# parse FON file fields for header
		#
		try:
			tmpTuple = struct.unpack_from('I', fonBytesBuff, offsInFonFile) # unsigned integer 4 bytes
			self.header().maxEntriesInTableOfDetails = tmpTuple[0]
			offsInFonFile += 4
			
			if self.simpleFontFileName == 'TAHOMA18.FON': # deal with corrupted original 'TAHOMA18.FON' file
				self.realNumOfCharactersInImageSegment = 176
				if self.m_traceModeEnabled:
					print "[Debug] SPECIAL CASE. WORKAROUND FOR CORRUPTED %s FILE. Only %d characters supported!" % (self.simpleFontFileName, self.realNumOfCharactersInImageSegment)
			else:
				self.realNumOfCharactersInImageSegment = self.header().maxEntriesInTableOfDetails
				
			tmpTuple = struct.unpack_from('I', fonBytesBuff, offsInFonFile)  # unsigned integer 4 bytes
			self.header().maxGlyphWidth = tmpTuple[0]
			offsInFonFile += 4
			
			tmpTuple = struct.unpack_from('I', fonBytesBuff, offsInFonFile)  # unsigned integer 4 bytes
			self.header().maxGlyphHeight = tmpTuple[0]
			offsInFonFile += 4
			
			tmpTuple = struct.unpack_from('I', fonBytesBuff, offsInFonFile)  # unsigned integer 4 bytes
			self.header().graphicSegmentByteSize = tmpTuple[0]
			offsInFonFile += 4
			
			if self.m_traceModeEnabled:
				print "[Debug] Font file (FON) Header Info: "
				print "[Debug] Number of entries: %d, Glyph max-Width: %d, Glyph max-Height: %d, Graphic Segment size: %d" % (self.header().maxEntriesInTableOfDetails, self.header().maxGlyphWidth, self.header().maxGlyphHeight, self.header().graphicSegmentByteSize)
			#
			# Glyph details table (each entry is 5 unsigned integers == 5*4 = 20 bytes)
			# For most characters, their ASCII value + 1 is the index of their glyph's entry in the details table. The 0 entry of this table is reserved
			#
			#tmpXOffset, tmpYOffset, tmpWidth, tmpHeight, tmpDataOffset
			if self.m_traceModeEnabled:
				print "[Debug] Font file (FON) glyph details table: "
			for idx in range(0, self.realNumOfCharactersInImageSegment):
				tmpTuple = struct.unpack_from('i', fonBytesBuff, offsInFonFile)  # unsigned integer 4 bytes
				tmpXOffset = tmpTuple[0]
				offsInFonFile += 4
				
				tmpTuple = struct.unpack_from('I', fonBytesBuff, offsInFonFile)  # unsigned integer 4 bytes
				tmpYOffset = tmpTuple[0]
				offsInFonFile += 4
				
				tmpTuple = struct.unpack_from('I', fonBytesBuff, offsInFonFile)  # unsigned integer 4 bytes
				tmpWidth = tmpTuple[0]
				offsInFonFile += 4
				
				tmpTuple = struct.unpack_from('I', fonBytesBuff, offsInFonFile)  # unsigned integer 4 bytes
				tmpHeight = tmpTuple[0]
				offsInFonFile += 4
				
				tmpTuple = struct.unpack_from('I', fonBytesBuff, offsInFonFile)  # unsigned integer 4 bytes
				tmpDataOffset = tmpTuple[0]
				offsInFonFile += 4
				
				if tmpWidth == 0 or tmpHeight == 0:
					if self.m_traceModeEnabled:
						print "Index: %d\t UNUSED *****************************************************************" % (idx)
				else:
					self.nonEmptyCharacters += 1
					if self.m_traceModeEnabled:
						print "Index: %d\txOffs: %d\tyOffs: %d\twidth: %d\theight: %d\tdataOffs: %d" % (idx, tmpXOffset, tmpYOffset, tmpWidth, tmpHeight, tmpDataOffset)
					if tmpDataOffset not in localLstOfDataOffsets:
						localLstOfDataOffsets.append(tmpDataOffset)
					else:
						# This never happens in the original files. Offsets are "re-used" but not really because it happens only for empty (height = 0) characters which all seem to point to the next non-empty character
						if self.m_traceModeEnabled:
							print "Index: %d\t RE-USING ANOTHER GLYPH *****************************************************************" % (idx)
							
				self.glyphDetailEntriesLst.append( ( tmpXOffset, tmpYOffset, tmpWidth, tmpHeight, tmpDataOffset) )
				
			offsInFonFile = (4 * 4) + (self.header().maxEntriesInTableOfDetails * 5 * 4) # we need the total self.header().maxEntriesInTableOfDetails here and not self.realNumOfCharactersInImageSegment
			self.glyphPixelData = fonBytesBuff[offsInFonFile:]
			return True
		except:
			print "[Error] Loading Font file (FON) %s failed!" % (self.simpleFontFileName)
			raise
		return False
		
	def outputFonToPNG(self):
		print "[Info] Exporting font file (FON) to PNG: %s" % (self.simpleFontFileName + ".PNG")
		
		targWidth = 0
		targHeight = 0
		paddingFromTopY = 2
		paddingBetweenGlyphsX = 10
		
		if len(self.glyphDetailEntriesLst) == 0 or (len(self.glyphDetailEntriesLst) != self.realNumOfCharactersInImageSegment and len(self.glyphDetailEntriesLst) != self.header().maxEntriesInTableOfDetails) :
			print "[Error] Font file (FON) loading process did not complete correctly. Missing important data in structures. Cannot output image!"
			return
			
		# TODO asdf refine this code here. the dimensions calculation is very crude for now
		if self.header().maxGlyphWidth > 0 :
			targWidth = (self.header().maxGlyphWidth + paddingBetweenGlyphsX) * (self.realNumOfCharactersInImageSegment + 1)
		else:
			targWidth = 1080
			
		# TODO asdf refine this code here. the dimensions calculation is very crude for now
		if self.header().maxGlyphHeight > 0 :
			targHeight = self.header().maxGlyphHeight * 2
		else:
			targHeight = 480
			
		imTargetGameFont = Image.new("RGBA",(targWidth, targHeight), (0,0,0,0))
		#print imTargetGameFont.getbands()
		#
		# Now fill in the image segment
		# Fonts in image segment are stored in pixel colors from TOP to Bottom, Left to Right per GLYPH.
		# Each pixel is 16 bit (2 bytes). Highest bit seems to determine transparency (on/off flag).
		# There seem to be 5 bits per RGB channel and the value is the corresponding 8bit value (from the 24 bit pixel color) shifting out (right) the 3 LSBs
		# First font image is the special character (border of top row and left column) - color of font pixels should be "0x7FFF" for filled and "0x8000" for transparent
		drawIdx = 0
		drawIdxDeductAmount = 0
		for idx in range(0, self.realNumOfCharactersInImageSegment):
			# TODO check for size > 0 for self.glyphPixelData
			# TODO mark glyph OUTLINES? (optional by switch)
			(glyphXoffs, glyphYoffs, glyphWidth, glyphHeight, glyphDataOffs) = self.glyphDetailEntriesLst[idx]
			glyphDataOffs = glyphDataOffs * 2
			#print idx, glyphDataOffs
			currX = 0
			currY = 0
			if (glyphWidth == 0 or glyphHeight == 0):
				drawIdxDeductAmount += 1
			drawIdx = idx - drawIdxDeductAmount
			
			for colorIdx in range(0, glyphWidth*glyphHeight):
				tmpTuple = struct.unpack_from('H', self.glyphPixelData, glyphDataOffs)	# unsigned short 2 bytes
				pixelColor = tmpTuple[0]
				glyphDataOffs += 2
				
#				 if pixelColor > 0x8000:
#					 print "[Debug] WEIRD CASE" # NEVER HAPPENS - TRANSPARENCY IS ON/OFF. There's no grades of transparency
				rgbacolour = (0,0,0,0)
				if pixelColor == 0x8000:
					rgbacolour = (0,0,0,0) # alpha: 0.0 fully transparent
				else:
					tmp8bitR1 =	 ( (pixelColor >> 10) ) << 3
					tmp8bitG1 =	 ( (pixelColor & 0x3ff) >> 5 ) << 3
					tmp8bitB1 =	 ( (pixelColor & 0x1f) ) << 3
					rgbacolour = (tmp8bitR1,tmp8bitG1,tmp8bitB1, 255) # alpha: 1.0 fully opaque
					#rgbacolour = (255,255,255, 255)   # alpha: 1.0 fully opaque
					
				if currX == glyphWidth:
					currX = 0
					currY += 1
					
				imTargetGameFont.putpixel(( (drawIdx + 1) * (self.header().maxGlyphWidth + paddingBetweenGlyphsX ) + currX, paddingFromTopY + glyphYoffs + currY), rgbacolour)
				currX += 1
		try:
			imTargetGameFont.save(os.path.join('.', self.simpleFontFileName + ".PNG"), "PNG")
		except Exception as e:
			print '[Error] Unable to write to output PNG file. ' + str(e)
			
	def header(self):
		return self.m_header
#
#
#
if __name__ == '__main__':
	# main()
	errorFound = False
	# By default assumes a file of name SUBTLS_E.FON in same directory
	# otherwise tries to use the first command line argument as input file
	# 'TAHOMA24.FON'   # USED IN CREDIT END-TITLES and SCORERS BOARD AT POLICE STATION
	# 'TAHOMA18.FON'   # USED IN CREDIT END-TITLES
	# '10PT.FON'       # BLADE RUNNER UNUSED FONT -  Probably font for reporting system errors
	# 'KIA6PT.FON'     # BLADE RUNNER MAIN FONT
	# 'SUBTLS_E.FON'   # OUR EXTRA FONT USED FOR SUBTITLES
	inFONFile = None
	inFONFileName =  'SUBTLS_E.FON'    # Subtitles font custom
	
	if len(sys.argv[1:])  > 0 \
		and os.path.isfile(os.path.join('.', sys.argv[1])) \
		and len(sys.argv[1]) >= 5 \
		and sys.argv[1][-3:].upper() == 'FON':
		inFONFileName = sys.argv[1]
		print "[Info] Attempting to use %s as input FON file..." % (inFONFileName)
	elif os.path.isfile(os.path.join('.', inFONFileName)):
		print "[Info] Using default %s as input FON file..." % (inFONFileName)
	else:
		print "[Error] No valid input file argument was specified and default input file %s is missing." % (inFONFileName)
		errorFound = True
	
	if not errorFound:
		try:
			print "[Info] Opening %s" % (inFONFileName)
			inFONFile = open(os.path.join('.',inFONFileName), 'rb')
		except:
			errorFound = True
			print "[Error] Unexpected event:", sys.exc_info()[0]
			raise
		if not errorFound:
			allOfFonFileInBuffer = inFONFile.read()
			fonFileInstance = fonFile(True)
			if fonFileInstance.m_traceModeEnabled:
				print "[Debug] Running %s (%s) as main module" % (MY_MODULE_NAME, MY_MODULE_VERSION)
			if (fonFileInstance.loadFonFile(allOfFonFileInBuffer, len(allOfFonFileInBuffer), inFONFileName)):
				print "[Info] Font file (FON) was loaded successfully!"
				fonFileInstance.outputFonToPNG()
			else:
				print "[Error] Error while loading Font file (FON)!"
			inFONFile.close()
else:
	#debug
	#print "[Debug] Running	 %s imported from another module" % (MY_MODULE_NAME)
	pass
