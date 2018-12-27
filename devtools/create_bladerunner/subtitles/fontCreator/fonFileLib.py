#!/usr/bin/env python2.7
# -*- coding: UTF-8 -*-
#
import os, sys, shutil
import struct
from struct import *
import Image

my_module_version = "0.50"
my_module_name = "fonFileLib"


class FonHeader:
	maxEntriesInTableOfDetails = -1			# this is probably always the number of entries in table of details, but it won't work for the corrupted TAHOMA18.FON file
	maxGlyphWidth = -1						# in pixels
	maxGlyphHeight = -1						# in pixels
	graphicSegmentByteSize = -1				# Graphic segment byte size

	def __init__(self):
		return


class fonFile:
	m_header = FonHeader()

	simpleFontFileName = "GENERIC.FON"
	realNumOfCharactersInImageSegment = 0	# this is used for the workaround for the corrupted TAHOME18.FON
	nonEmptyCharacters = 0

	glyphDetailEntriesLst = []	# list of 5-value tuples. Tuple values are (X-offset, Y-offset, Width, Height, Offset in Graphics segment)
	glyphPixelData = None		# buffer of pixel data for glyphs

	def __init__(self):
		del self.glyphDetailEntriesLst[:]
		self.glyphPixelData = None		# buffer of pixel data for glyphs
		self.simpleFontFileName = "GENERIC.FON"
		self.realNumOfCharactersInImageSegment = 0	# this is used for the workaround for the corrupted TAHOME18.FON
		self.nonEmptyCharacters = 0

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
			tmpTuple = struct.unpack_from('I', fonBytesBuff, offsInFonFile)	 # unsigned integer 4 bytes
			self.header().maxEntriesInTableOfDetails = tmpTuple[0]
			offsInFonFile += 4

			if self.simpleFontFileName == 'TAHOMA18.FON': # deal with corrupted original 'TAHOMA18.FON' file
				self.realNumOfCharactersInImageSegment = 176
				print "SPECIAL CASE. WORKAROUND FOR CORRUPTED %s FILE. Only %d characters supported!" % (self.simpleFontFileName, self.realNumOfCharactersInImageSegment)
			else:
				self.realNumOfCharactersInImageSegment = self.header().maxEntriesInTableOfDetails

			tmpTuple = struct.unpack_from('I', fonBytesBuff, offsInFonFile)	 # unsigned integer 4 bytes
			self.header().maxGlyphWidth = tmpTuple[0]
			offsInFonFile += 4

			tmpTuple = struct.unpack_from('I', fonBytesBuff, offsInFonFile)	 # unsigned integer 4 bytes
			self.header().maxGlyphHeight = tmpTuple[0]
			offsInFonFile += 4

			tmpTuple = struct.unpack_from('I', fonBytesBuff, offsInFonFile)	 # unsigned integer 4 bytes
			self.header().graphicSegmentByteSize = tmpTuple[0]
			offsInFonFile += 4

			print "FON Header Info: "
			print "Num of entries: %d\tGlyph max-Width: %d\tGlyph max-Height: %d\tGraphic Segment size %d" % (self.header().maxEntriesInTableOfDetails, self.header().maxGlyphWidth, self.header().maxGlyphHeight, self.header().graphicSegmentByteSize)
			#print "Num of entries: %d\tGlyph max-Width: %d\tGlyph max-Height: %d\tGraphic Segment size %d" % (self.realNumOfCharactersInImageSegment, self.header().maxGlyphWidth, self.header().maxGlyphHeight, self.header().graphicSegmentByteSize)
			#
			# Glyph details table (each entry is 5 unsigned integers == 5*4 = 20 bytes)
			# For most characters, their ASCII value + 1 is the index of their glyph's entry in the details table. The 0 entry of this table is reserved
			#
			#tmpXOffset, tmpYOffset, tmpWidth, tmpHeight, tmpDataOffset
			print "FON glyph details table: "
			for idx in range(0, self.realNumOfCharactersInImageSegment):
				tmpTuple = struct.unpack_from('i', fonBytesBuff, offsInFonFile)	 # unsigned integer 4 bytes
				tmpXOffset = tmpTuple[0]
				offsInFonFile += 4

				tmpTuple = struct.unpack_from('I', fonBytesBuff, offsInFonFile)	 # unsigned integer 4 bytes
				tmpYOffset = tmpTuple[0]
				offsInFonFile += 4

				tmpTuple = struct.unpack_from('I', fonBytesBuff, offsInFonFile)	 # unsigned integer 4 bytes
				tmpWidth = tmpTuple[0]
				offsInFonFile += 4

				tmpTuple = struct.unpack_from('I', fonBytesBuff, offsInFonFile)	 # unsigned integer 4 bytes
				tmpHeight = tmpTuple[0]
				offsInFonFile += 4

				tmpTuple = struct.unpack_from('I', fonBytesBuff, offsInFonFile)	 # unsigned integer 4 bytes
				tmpDataOffset = tmpTuple[0]
				offsInFonFile += 4

				if tmpWidth == 0 or tmpHeight == 0:
					print "Index: %d\t UNUSED *****************************************************************" % (idx)
				else:
					self.nonEmptyCharacters += 1
					print "Index: %d\txOffs: %d\tyOffs: %d\twidth: %d\theight: %d\tdataOffs: %d" % (idx, tmpXOffset, tmpYOffset, tmpWidth, tmpHeight, tmpDataOffset)
					if tmpDataOffset not in localLstOfDataOffsets:
						localLstOfDataOffsets.append(tmpDataOffset)
					else:
						# This never happens in the original files. Offsets are "re-used" but not really because it happens only for empty (height = 0) characters which all seem to point to the next non-empty character
						print "Index: %d\t RE-USING ANOTHER GLYPH *****************************************************************" % (idx)

				self.glyphDetailEntriesLst.append( ( tmpXOffset, tmpYOffset, tmpWidth, tmpHeight, tmpDataOffset) )

			offsInFonFile = (4 * 4) + (self.header().maxEntriesInTableOfDetails * 5 * 4) # we need the total self.header().maxEntriesInTableOfDetails here and not self.realNumOfCharactersInImageSegment
			self.glyphPixelData = fonBytesBuff[offsInFonFile:]
			return True
		except:
			print "Loading failure!"
			raise
			return False

	def outputFonToPNG(self):
		targWidth = 0
		targHeight = 0
		paddingFromTopY = 2
		paddingBetweenGlyphsX = 10


		if len(self.glyphDetailEntriesLst) == 0 or (len(self.glyphDetailEntriesLst) != self.realNumOfCharactersInImageSegment and len(self.glyphDetailEntriesLst) != self.header().maxEntriesInTableOfDetails) :
			print "Error. Fon file load process did not complete correctly. Missing important data in structures. Cannot output image!"
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
#					 print "WEIRD CASE" # NEVER HAPPENS - TRANSPARENCY IS ON/OFF. There's no grades of transparency
				rgbacolour = (0,0,0,0)
				if pixelColor == 0x8000:
					rgbacolour = (0,0,0,0) # alpha: 0.0 fully transparent
				else:
					tmp8bitR1 =	 ( (pixelColor >> 10) ) << 3
					tmp8bitG1 =	 ( (pixelColor & 0x3ff) >> 5 ) << 3
					tmp8bitB1 =	 ( (pixelColor & 0x1f) ) << 3
					rgbacolour = (tmp8bitR1,tmp8bitG1,tmp8bitB1, 255)	# alpha: 1.0 fully opaque
					#rgbacolour = (255,255,255, 255)   # alpha: 1.0 fully opaque

				if currX == glyphWidth:
					currX = 0
					currY += 1

				imTargetGameFont.putpixel(( (drawIdx + 1) * (self.header().maxGlyphWidth + paddingBetweenGlyphsX ) + currX, paddingFromTopY + glyphYoffs + currY), rgbacolour)
				currX += 1

		imTargetGameFont.save(os.path.join('.', self.simpleFontFileName + ".PNG"), "PNG")

	def header(self):
		return self.m_header
#
#
#
if __name__ == '__main__':
	#	 main()
	print "Running %s as main module" % (my_module_name)
	# assumes a file of name TAHOMA24.FON in same directory
	inFONFile = None
	#inFONFileName =  'TAHOMA24.FON'		# USED IN CREDIT END-TITLES and SCORERS BOARD AT POLICE STATION
	#inFONFileName =	 'TAHOMA18.FON'		   # USED IN CREDIT END-TITLES
	#inFONFileName =  '10PT.FON'			# BLADE RUNNER UNUSED FONT?
	#inFONFileName =  'KIA6PT.FON'			# BLADE RUNNER MAIN FONT
	inFONFileName =  'SUBTLS_E.FON'			# Subtitles font custom

	errorFound = False
	try:
		print "Opening %s" % (inFONFileName)
		inFONFile = open(os.path.join('.',inFONFileName), 'rb')
	except:
		errorFound = True
		print "Unexpected error:", sys.exc_info()[0]
		raise
	if not errorFound:
		allOfFonFileInBuffer = inFONFile.read()
		fonFileInstance = fonFile()
		if (fonFileInstance.loadFonFile(allOfFonFileInBuffer, len(allOfFonFileInBuffer), inFONFileName)):
			print "FON file loaded successfully!"
			fonFileInstance.outputFonToPNG()
		else:
			print "Error while loading FON file!"
		inFONFile.close()
else:
	#debug
	#print "Running	 %s imported from another module" % (my_module_name)
	pass