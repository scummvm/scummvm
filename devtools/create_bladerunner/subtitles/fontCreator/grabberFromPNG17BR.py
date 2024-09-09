#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-------------------------------------------------------------------------------
# Name:		   grabberFromPNG15BR
# Purpose:	   Parse the character fonts from a PNG file in order to create a
#			   FON file for the Westwood Blade Runner PC game.
#
# Author:	   antoniou
#
# Created:	   16-05-2018
# Copyright:   (c) antoniou 2018
# Licence:
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
# DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
# OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#-------------------------------------------------------------------------------

#
# Let's assume an Input image with only a row of all character glyphs (no double rows...)
# BLADE RUNNER:
# TODO: A way to differentiate between corrupted TAHOMA18 and extended TAHOMA18 (user created)
# TODO: Maybe instead of removing transparency from glyph ROW image (transparency threshold= 0), use a grey gradient for semitransparent pixels for the cases of TAHOMA fonts (and maybe the subtitle ones too)
#           Create the font row in GIMP - add layers for shadows if necessary
#           Merge all layers (maintaining an alpha channel for the background)
#           Select all and float, which should create a floating selection with all the letters
#           Promote that to layer and duplicate it.
#           Choose one of the duplicated layers and COLORIFY it to PITCH BLACK
#               Set the transparency threshold of THAT black layer to 0
#           Put this layer under the other and merge the visible layers.
#           This should get rid of semi-transparent pixels while maintaining the "aliasing" effect. There could be a better way but this should work ok.


# DONE: A way to export game fonts to PNG image (in order to be able to create new expanded fonts keeping glyph consistency!)
#           Also override the corruption in TAHOMA18 while exporting
# TODO: Re-Check the order of fonts in (in-game resource font files) TAHOMA18 (stored corrupted) and TAHOMA24 (in good condition).
# TODO: print a warning for mismatch of number of letters in encoding override (or internal) and detected fonts in ROW IMAGE (especially if we expect a double exclamation mark at the start - and we ignoring one of the two)
# TODO: Don't crash if mismatch in detected letter (fewer) in row image vs the letters in ASCII list of overrideEncoding file
# TODO: A more detailed readme for this tool and how to use it
#
# DONE: enforce overrideEncoding.txt -- this tool should no longer work without one
# DONE: Test Greek subs too
# DONE: Letter fonts should be spaced by TAB when copied into GIMP or other app to create the image row of all character glyphs
# DONE: First character should be repeated in the ROW file (but taken into consideration once) in order to get the pixels for the TAB space between letters (left-start column to left-start column)
# DONE: Use the tab space pixels to calculate the KERNING for each letter (x offset)
# DONE: update the image segment size bytes in the header after having completed populating the image segment
# DONE: allow setting explicit kerning and width addon for cases like i and l characters
# DONE: entrée (boiled dog question) - has an e like goose liver pate --> TESTED
# DONE: Tested ok "si senor" from Peruvian lady / insect dealer too!
# DONE: ability to manually set kerning (x-offset) for fonts by letter like a list in parameters or in overrideEncoding.txt }  i:1,j:-1,l:1	 (no space or white line characters) - POSITIVE OR NEGATIVE VALUES BOTH ADMITTED
# DONE: a value of '-' for this means ignore
# DONE: ability to manually set extra width (additional columns at the end of glyph, with transparent color) for fonts by letter like a list in parameters or in overrideEncoding.txt }	 i:1,j:2,l:1 - POSITIVE VALUES ONLY
# DONE: make space pixels (var spaceWidthInPixels) into an external param?
# DONE: INFO NOTE IT IS NOT POSSIBLE TO have partial transparency
osLibFound = False
sysLibFound = False
shutilLibFound = False
structLibFound = False
imagePilLibFound = False
reLibFound = False

try:
	import os
except ImportError:
	print ("[Error] os python library is required to be installed!")
else:
	osLibFound = True

try:
	import sys
except ImportError:
	print ("[Error] sys python library is required to be installed!")
else:
	sysLibFound = True

try:
	import shutil
except ImportError:
	print ("[Error] Shutil python library is required to be installed!")
else:
	shutilLibFound = True

try:
	import struct
except ImportError:
	print ("[Error] struct python library is required to be installed!")
else:
	structLibFound = True

try:
	from PIL import Image
except ImportError:
	print ("[Error] Image python library (PIL or pillow) is required to be installed!")
else:
	imagePilLibFound = True

try:
	import re
except ImportError:
	print ("[Error] re (Regular expression operations) python library is required to be installed!")
else:
	reLibFound = True

if 	(not osLibFound) \
	or (not sysLibFound) \
	or (not shutilLibFound) \
	or (not structLibFound) \
	or (not imagePilLibFound) \
	or (not reLibFound):
	sys.stdout.write("[Error] Errors were found when trying to import required python libraries\n")
	sys.exit(1)

try:
	import errno
except ImportError:
	from os import errno

pathToParent = os.path.join(os.path.dirname(os.path.abspath(__file__)), os.pardir)
pathToCommon = os.path.join(pathToParent, "common")
sys.path.append(pathToCommon)

from os import walk, path
from struct import *
from fonFileLib import *
from pythonCompat import *

COMPANY_EMAIL = "classic.adventures.in.greek@gmail.com"
APP_VERSION = "1.20"
APP_NAME = "grabberFromPNGHHBR"
APP_WRAPPER_NAME = "fontCreator.py"
APP_NAME_SPACED = "Blade Runner Font Creator/Exporter"
APP_SHORT_DESC = "Create Font Files (.FON) or export existing FON files to PNG images."

SUPPORTED_MIX_INPUT_FILES = ['STARTUP.MIX']
## 4 font files
SUPPORTED_EXPORTED_GAME_FON_FILES = ['10PT.FON', 'TAHOMA18.FON', 'TAHOMA24.FON', 'KIA6PT.FON']

gTraceModeEnabled = False

def calculateFoldHash(strFileName):
	i = 0
	hash = 0
	strParam = strFileName.upper()
	lenFileName = len(strParam);
	while i < lenFileName and i < 12:
		groupSum = 0
		# work in groups of 4 bytes
		for j in range(0, 4):
			# LSB first, so the four letters in the string are re-arranged (first letter goes to lower place)
			groupSum >>= 8;
			if (i < lenFileName):
				groupSum |= (ord(strParam[i]) << 24)
				i += 1
			else: # if	i >= lenFileName  but still haven't completed the four byte loop add 0s
				groupSum |= 0
		hash = ((hash << 1) | ((hash >> 31) & 1)) + groupSum
	hash &= 0xFFFFFFFF	   # mask here!
	#if gTraceModeEnabled:
	#	print ("[Debug] " + (strParam + ": "  +''.join('{:08X}'.format(hash))))
	return hash

class grabberFromPNG:
	targetEncoding = 'windows-1253'

	overrideEncodingPath = ""
	originalFontName = ''

	BR_GameID = 3
	BR_Desc = 'Blade Runner'
	BR_CodeName = 'BLADERUNNER'
	BR_DefaultFontFileName = 'SUBTLS_E.FON'

	defaultSpaceWidthInPixelsConst = 0x0007 #0x0008	 #0x0006
	spaceWidthInPixels = defaultSpaceWidthInPixelsConst
	specialGlyphMode = True
	autoTabCalculation = True

	reconstructEntireFont = False # TODO: TRUE!!!
	minSpaceBetweenLettersInRowLeftToLeft = 0
	minSpaceBetweenLettersInColumnTopToTop = 0
	kerningForFirstDummyFontLetter = 0
	yOffsetForAllGlyphsExceptFirstSpecialGamma = 0
	#deductKerningPixels = 0

	inputFonMixPath = ""
	targetFONFilename = BR_DefaultFontFileName
	#origFontFilename =""
	origFontPropertiesTxt = ""
	#imageOriginalPNG =""
	imageRowFilePNG =""
	copyFontFileName =""
	copyFontPropertiesTxt = ""
	copyPNGFileName =""

	lettersFound = 0
	listOfBaselines = []
	listOfWidths = []
	listOfHeights = [] # new for Blade Runner support
	listOfLetterBoxes = []
	properListOfLetterBoxes = []
	startColOfPrevFontLetter = 0 # new for Blade Runner support
	tabSpaceWidth = 0
	startOfAllLettersIncludingTheExtraDoubleAndWithKern = 0
	maxAsciiValueInEncoding	 = 0

	listOfXOffsets = [] # new for Blade Runner support
	listOfYOffsets = [] # new for Blade Runner support

	listOfExplicitKerning = []
	listOfWidthIncrements = []
	listOfOutOfOrderGlyphs = []

	targetLangOrderAndListOfForeignLettersAsciiValues = None

	def initOverrideEncoding(self):
		##
		## FOR INIT PURPOSES!!!!
		##
		overrideFailed = True
		targetLangOrderAndListOfForeignLettersStrUnicode = None
		targetLangOrderAndListOfForeignLettersStr = None
		# Read from an override file if it exists. Filename should be overrideEncoding.txt (overrideEncodingTextFile)
		if not self.overrideEncodingPath :
			overrideEncodingTextFile = u'overrideEncoding.txt'
			self.overrideEncodingPath = os.path.join(u'.', overrideEncodingTextFile)
			print ("[Warning] Font Creation Override Encoding file not found in arguments. Attempting to open local file %s, if it exists" % (overrideEncodingTextFile))

		if os.access(self.overrideEncodingPath, os.F_OK):
			print ("[Info] Override encoding file found: {0}.".format(self.overrideEncodingPath))
			overEncodFile = openWithUTF8Encoding(self.overrideEncodingPath, 'r')
			linesLst = overEncodFile.readlines()
			overEncodFile.close()
			if linesLst is None or len(linesLst) == 0:
				overrideFailed = True
			else:
				if gTraceModeEnabled:
					print ("[Debug] Parsing Override Encoding file info...")
				involvedTokensLst =[]
				del involvedTokensLst[:] # unneeded
				for readEncodLine in linesLst:
					tmplineTokens = re.findall("[^\t\n]+",readEncodLine )
					for x in tmplineTokens:
						involvedTokensLst.append(x)
					#if gTraceModeEnabled:
					#	print ("[Debug] ", involvedTokensLst)
					#break #only read first line
				for tokenNameKeyPair in involvedTokensLst:
					nameKeyTupl = tokenNameKeyPair.split('=', 1)
					try:
						if len(nameKeyTupl) == 2 and nameKeyTupl[0] == 'targetEncoding' and nameKeyTupl[1] is not None and nameKeyTupl[1] != '-' and nameKeyTupl[1] != '':
							self.targetEncoding = nameKeyTupl[1]
						elif len(nameKeyTupl) == 2 and nameKeyTupl[0] == 'asciiCharList' and nameKeyTupl[1] is not None and nameKeyTupl[1] != '':
							targetLangOrderAndListOfForeignLettersStrUnicode = makeUnicode(nameKeyTupl[1])
						elif len(nameKeyTupl) == 2 and nameKeyTupl[0] == 'explicitKerningList' and nameKeyTupl[1] is not None and nameKeyTupl[1] != '-' and nameKeyTupl[1] != '':
							# split at comma, then split at ':' and store tuples of character and explicit kerning
							explicitKerningTokenUnicode = makeUnicode(nameKeyTupl[1])
							explicitKerningTokenStr = unicodeEncode("%s" % explicitKerningTokenUnicode, self.targetEncoding)
							tokensOfExplicitKerningTokenStrList = encodedSplit(explicitKerningTokenStr, ',')
							for tokenX in tokensOfExplicitKerningTokenStrList:
								tokensOfTupleList = encodedSplit(tokenX, ':')
								self.listOfExplicitKerning.append((ord(tokensOfTupleList[0]), int(tokensOfTupleList[1])) )
						elif len(nameKeyTupl) == 2 and nameKeyTupl[0] == 'explicitWidthIncrement' and nameKeyTupl[1] is not None and nameKeyTupl[1] != '-' and nameKeyTupl[1] != '':
							# split at comma, then split at ':' and store tuples of character and explicit additional width - POSITIVE VALUES ONLY
							explicitWidthIncrementTokenUnicode = makeUnicode(nameKeyTupl[1])
							explicitWidthIncrementTokenStr = unicodeEncode("%s" % explicitWidthIncrementTokenUnicode, self.targetEncoding)
							tokensOfWidthIncrementStrList = encodedSplit(explicitWidthIncrementTokenStr, ',')
							for tokenX in tokensOfWidthIncrementStrList:
								tokensOfTupleList = encodedSplit(tokenX, ':')
								self.listOfWidthIncrements.append((ord(tokensOfTupleList[0]), int(tokensOfTupleList[1])))
						elif len(nameKeyTupl) == 2 and nameKeyTupl[0] == 'specialOutOfOrderGlyphsUTF8ToAsciiTargetEncoding' and nameKeyTupl[1] is not None and nameKeyTupl[1] != '-' and nameKeyTupl[1] != '':
							# split at comma, then split at ':' and store tuples of character
							explicitOutOfOrderGlyphsTokenUnicode = makeUnicode(nameKeyTupl[1]) # unicode(nameKeyTupl[1], 'utf-8')
							#explicitOutOfOrderGlyphsTokenStr =  unicodeEncode("%s" % explicitOutOfOrderGlyphsTokenUnicode, self.targetEncoding)
							#explicitOutOfOrderGlyphsTokenStr =  explicitOutOfOrderGlyphsTokenUnicode.decode(self.targetEncoding) # unicodeEncode("%s" % explicitOutOfOrderGlyphsTokenUnicode, 'utf-8')
							tokensOfOutOfOrderGlyphsStrList = explicitOutOfOrderGlyphsTokenUnicode.split(',')
							for tokenX in tokensOfOutOfOrderGlyphsStrList:
								tokensOfTupleList = tokenX.split(':')
								self.listOfOutOfOrderGlyphs.append( (getUnicodeSym(tokensOfTupleList[0]), getUnicodeSym(tokensOfTupleList[1])) )
						elif len(nameKeyTupl) == 2 and nameKeyTupl[0] == 'originalFontName' and nameKeyTupl[1] is not None and nameKeyTupl[1] != '-' and nameKeyTupl[1] != '':
							self.originalFontName =  nameKeyTupl[1]
					except:
						overrideFailed = True
						raise
				if not (self.targetEncoding is None or not self.targetEncoding or targetLangOrderAndListOfForeignLettersStrUnicode is None or len(targetLangOrderAndListOfForeignLettersStrUnicode) == 0 or self.originalFontName is None or not self.originalFontName):
					overrideFailed = False

				if gTraceModeEnabled:
					print ("[Debug] Target Encoding: ", self.targetEncoding)
					#print ("[Debug] Lang Order: ", targetLangOrderAndListOfForeignLettersStrUnicode)
					print ("[Debug] Explicit Kern List: ", self.listOfExplicitKerning)
					print ("[Debug] Explicit Width Increment List: ", self.listOfWidthIncrements)
					print ("[Debug] Original Font Name: ", self.originalFontName)

			if(len(self.listOfOutOfOrderGlyphs) == 0 and self.specialGlyphMode == True):
				# Just keep those that are needed
				#if self.originalFontName == 'SUBTLS_E':
				#	self.listOfOutOfOrderGlyphs.append((u'\xed', u'\u0386')) # spanish i (si)
				#	self.listOfOutOfOrderGlyphs.append((u'\xf1', u'\xa5'))   # spanish n (senor)
				#	self.listOfOutOfOrderGlyphs.append((u'\xe2', u'\xa6'))   # a for (liver) pate
				#	self.listOfOutOfOrderGlyphs.append((u'\xe9', u'\xa7'))   # e for (liver) pate
				#elif self.originalFontName == 'TAHOMA': # treat TAHOMA18 and TAHOMA24 similarly here
				if self.originalFontName == 'TAHOMA': # treat TAHOMA18 and TAHOMA24 similarly here
					self.listOfOutOfOrderGlyphs.append((u'\xe9', u'\u0192')) 	# french e punctuated
					self.listOfOutOfOrderGlyphs.append((u'\xfc', u'\u2013')) 	# u umlaut
			if gTraceModeEnabled:
				print ("[Debug] Explicit Out Of Order Glyphs List: " , self.listOfOutOfOrderGlyphs)
		else:
			## error
			print ("[Error] Override encoding file not found: {0}.".format(self.overrideEncodingPath))
			pass

		if overrideFailed:
			## error
			print ("[Error] Override encoding file parsing has failed: Error Code 1.")
			sys.exit(1)	# terminate if override Failed (Blade Runner)

		try:
			targetLangOrderAndListOfForeignLettersStr = unicodeEncode("%s" % targetLangOrderAndListOfForeignLettersStrUnicode, self.targetEncoding)
		except:
			## error
			print ("[Error] Override encoding file parsing has failed: Error Code 2.")
			sys.exit(1)	# terminate if override Failed (Blade Runner)

		targetLangOrderAndListOfForeignLetters = list(targetLangOrderAndListOfForeignLettersStr)
		if gTraceModeEnabled:
			print ("[Debug] ", targetLangOrderAndListOfForeignLetters, len(targetLangOrderAndListOfForeignLetters))	   # new
		self.targetLangOrderAndListOfForeignLettersAsciiValues = makeToBytes(targetLangOrderAndListOfForeignLetters)
		if gTraceModeEnabled:
			print ("[Debug] ", self.targetLangOrderAndListOfForeignLettersAsciiValues, len(self.targetLangOrderAndListOfForeignLettersAsciiValues))
		self.maxAsciiValueInEncoding = max(self.targetLangOrderAndListOfForeignLettersAsciiValues)
	#if gTraceModeEnabled:
	#	for charAsciiValue in targetLangOrderAndListOfForeignLetters:
	#		print ("[Debug] Ord of chars: %d" % ord(charAsciiValue))
##
## END OF INIT CODE
##

#
# TODO: warning: assumes that there is a margin on top and bellow the letters
# (especially on top; if a letter starts from the first pixel row,
# it might not detect it!) <---to fix
#
	def __init__(self, pselectedEncoding=None):
		self.minSpaceBetweenLettersInRowLeftToLeft = 0
		self.minSpaceBetweenLettersInColumnTopToTop = 0
		self.kerningForFirstDummyFontLetter = 0
		self.yOffsetForAllGlyphsExceptFirstSpecialGamma = 0
		self.spaceWidthInPixels = self.defaultSpaceWidthInPixelsConst
		#self.deductKerningPixels = 0
		self.reconstructEntireFont = False # TODO : True?
		#self.origFontFilename=porigFontFilename
		self.inputFonMixPath = ""
		self.targetFONFilename = self.BR_DefaultFontFileName
		self.copyFontFileName = ""
		self.copyPNGFileName=""
		#self.imageOriginalPNG=pimageOriginalPNG
		self.imageRowFilePNG = ""
		self.baselineOffset = 0

		self.lettersFound = 0
		self.origFontPropertiesTxt = ""
		self.copyFontPropertiesTxt = ""
		self.cleanup() # for good practice (this should not be here, but it's here too due to quick/sloppy :) coding (TODO: fix it)
		#
		if pselectedEncoding == None:
			pselectedEncoding = self.targetEncoding

		self.selectedGameID = self.BR_GameID
		self.activeEncoding = pselectedEncoding
		self.lettersInOriginalFontFile = 0 #initialization

		return


	def cleanup(self):
		self.lettersFound = 0
		self.startColOfPrevFontLetter = 0
		self.tabSpaceWidth = 0
		self.startOfAllLettersIncludingTheExtraDoubleAndWithKern = 0
		del self.listOfBaselines[:]
		del self.listOfWidths[:]
		del self.listOfHeights[:]
		del self.listOfLetterBoxes[:]
		del self.properListOfLetterBoxes[:]
		del self.listOfXOffsets[:] # new for Blade Runner support
		del self.listOfYOffsets[:] # new for Blade Runner support
		#del self.listOfExplicitKerning[:] # don't clean these up
		#del self.listOfWidthIncrements[:] # don't clean these up

		self.origFontPropertiesTxt = ""
		self.copyFontPropertiesTxt = ""
		return

##
## SETTERS
##
	def setInputPathForFonMix(self, pInputMixPath):
		self.inputFonMixPath = pInputMixPath
		return

	def setImageRowFilePNG(self, pimageRowFilePNG):
		self.imageRowFilePNG = pimageRowFilePNG
		return

	def setTargetFONFilename(self, pTargetFONFilename):
		self.targetFONFilename = pTargetFONFilename
		return

	def setMinSpaceBetweenLettersInRowLeftToLeft(self, pminSpaceBetweenLettersInRowLeftToLeft):
		self.minSpaceBetweenLettersInRowLeftToLeft = pminSpaceBetweenLettersInRowLeftToLeft
		return
	def setMinSpaceBetweenLettersInColumnTopToTop(self, pminSpaceBetweenLettersInColumnTopToTop):
		self.minSpaceBetweenLettersInColumnTopToTop = pminSpaceBetweenLettersInColumnTopToTop
		return

	def setKerningForFirstDummyFontLetter(self, pKerningForFirstDummyFontLetter):
		self.kerningForFirstDummyFontLetter = pKerningForFirstDummyFontLetter
		return

	def setYOffsetForAllGlyphsExceptFirstSpecialGamma(self, pYOffsetForAllGlyphsExceptFirstSpecialGamma):
		self.yOffsetForAllGlyphsExceptFirstSpecialGamma = pYOffsetForAllGlyphsExceptFirstSpecialGamma
		return

	#def setDeductKerningPixels(self, pDeductKerningPixels):
	#	self.deductKerningPixels = pDeductKerningPixels
	#	return

	def setSpaceWidthInPixels(self, pSpaceWidthInPixels):
		self.spaceWidthInPixels = pSpaceWidthInPixels
		return

	def setSpecialGlyphMode(self, pSpecialGlyphMode):
		self.specialGlyphMode = pSpecialGlyphMode
		return


	def setAutoTabCalculation(self, pAutoTabCalculation):
		self.autoTabCalculation = pAutoTabCalculation
		return

	def setOverrideEncodingPath(self, pOverrideEncodingPath):
		self.overrideEncodingPath = pOverrideEncodingPath
		return

##
## END OF SETTERS
##



	def parseImage(self,  loadedImag, imwidth, imheight, trimTopPixels=0, trimBottomPixels = 0, firstDoubleLetterIgnore = False):
		""" parsing input image and detect one character font per run, and deleting the detected character font after calculating its specs (this is done in-memory; we are not writing back to the file)
		"""
		prevColStartForLetter = 0
		prevRowStartForLetter = 0
		startCol = 0
		startRow = 0
		endCol = 0
		endRow = 0
		for x in range(0, imwidth):		 # for each column
			if startCol != 0:
				break
			for y in range(0, imheight):	 # we search all rows (for each column)
				if type(loadedImag[x, y]) is not tuple or len(loadedImag[x, y]) < 4:
					#print type(loadedImag[x, y]), len(loadedImag[x, y])
					print ("[Error] Bad format for pixel type -- probable cause: unsupported PNG optimization")
					return -2

				r1,g1,b1,a1 = loadedImag[x, y]
				if a1 != 0:	 # if pixel not completely transparent -- this is not necessarily the *top* left pixel of a font letter though! -- the startRow is still to be determined.
					#if gTraceModeEnabled:
					#	print ("[Debug] ", loadedImag[x, y])
					if prevColStartForLetter == 0:
						prevColStartForLetter = x
						prevRowStartForLetter = y
						startCol = x
						#if gTraceModeEnabled:
						#	print ("[Debug] Letter found.")
						#	print ("[Debug] Start col: %d" % startCol)
						##	print ("[Debug] hypothetical row: %d" % y)
						# starting from the first row of the row-image (to do optimize), we parse by rows to find the top point (row coordinate) of the character font
						#for y2 in range(0, y+1):
						tmpSum = y + self.minSpaceBetweenLettersInColumnTopToTop
						scanToRow = imheight		  # - explicitly set this to the whole image height -- assumes only one row of character fonts
						if tmpSum < imheight:	   # TODO: WAS scanToRow < imheight but this doesn't get executed anymore due to explicitly setting scanToRow to imheight (assuming only one row of character fonts)
													 # DONE: NEW changed check to if tmpSum < imheight which makes more sense
							scanToRow = tmpSum
						for y2 in range(0, scanToRow):		   # a loop to find the startRow  - Check by row (starting from the top of the image and the left column of the letter)
							if startRow != 0:
								break
							tmpSum = startCol + self.minSpaceBetweenLettersInRowLeftToLeft
							scanToCol = imwidth
							if tmpSum < imwidth:
								scanToCol = tmpSum
							#if gTraceModeEnabled:
							#	print ("[Debug] ", (startCol, scanToCol))
							for x2 in range(startCol, scanToCol): # check all columns (for each row)
								#if gTraceModeEnabled:
								#	print ("[Debug] ", loadedImag[x2, y2])
								r2,g2,b2,a2 = loadedImag[x2, y2]
								if a2 != 0 and startRow == 0:
									startRow = y2 + trimTopPixels
									#if gTraceModeEnabled:
									#	print ("[Debug] Start row: %d" % startRow)
									break
		if startCol > 0 and startRow > 0:		   # WARNING: TODO NOTE: SO NEVER HAVE AN INPUT IMAGE WHERE THE FONT CHARACTERS ARE TOUCHING THE TOP OF THE IMAGE WITH NO EMPTY SPACE WHATSOEVER
			tmpSum = startRow + self.minSpaceBetweenLettersInColumnTopToTop
			scanToRow = imheight
			if tmpSum < imheight:
			   scanToRow = tmpSum
			tmpSum = startCol + self.minSpaceBetweenLettersInRowLeftToLeft
			scanToCol = imwidth
			if tmpSum < imwidth:
			   scanToCol = tmpSum
			for y in range(startRow, scanToRow):		# now check per row (we go through all theoretical rows, no breaks)-- we want to find the bottom row
				for x in range(startCol, scanToCol):	# check the columns for each row
					r1,g1,b1,a1 = loadedImag[x, y]
					if a1 != 0:
						endRow = y
			if endRow > 0:
				endRow = endRow - trimBottomPixels
			#if gTraceModeEnabled:
			#	print ("[Debug] End row: %d" % endRow)

		if startCol > 0 and startRow > 0 and endRow > 0:
			tmpSum = startCol + self.minSpaceBetweenLettersInRowLeftToLeft
			scanToCol = imwidth
			if tmpSum < imwidth:
			   scanToCol = tmpSum
			for x in range(startCol, scanToCol):	# now check per column (we go through all theoretical columns, no breaks) -- we want to find the bottom column
				for y in range(startRow, endRow+1): # check the rows for each column
					r1,g1,b1,a1 = loadedImag[x, y]
					#print	loadedImag[x, y]
					if a1 != 0:
						endCol = x
			#if gTraceModeEnabled:
			#	print ("[Debug] End col: %d" % endCol)
		if startCol > 0 and startRow > 0 and endRow > 0 and endCol > 0:
			# append deducted baseline
			#
			if firstDoubleLetterIgnore == True:
				self.startOfAllLettersIncludingTheExtraDoubleAndWithKern = startCol - self.kerningForFirstDummyFontLetter
			else:							  # firstDoubleLetterIgnore == False
				if self.autoTabCalculation == True:
					if self.tabSpaceWidth == 0:
						#if gTraceModeEnabled:
						#	print ("[Debug] start startPre: ", startCol, self.startColOfPrevFontLetter)
						self.tabSpaceWidth = startCol - self.startColOfPrevFontLetter
						if gTraceModeEnabled:
							print ("[Debug] Tab Space Width detected: %d " % (self.tabSpaceWidth))
					# new if -- don't use else here, to include the case of when we first detected the tab space width
					if self.tabSpaceWidth > 0:
						self.listOfXOffsets.append(startCol - (self.startOfAllLettersIncludingTheExtraDoubleAndWithKern + (self.lettersFound + 1) * self.tabSpaceWidth) ) #	 + self.deductKerningPixels )
						#if gTraceModeEnabled:
						#	print ("[Debug] xOffSet: ", startCol - (self.startOfAllLettersIncludingTheExtraDoubleAndWithKern + (self.lettersFound + 1) * self.tabSpaceWidth))
				else:
					self.listOfXOffsets.append(0)


				self.listOfBaselines.append(endRow)
				self.listOfWidths.append(endCol-startCol + 1) # includes the last col (TODO this was without the +1 for MI:SE translator -- possible bug? did we compensate?)
				self.listOfHeights.append(endRow - startRow + 1) # +1 includes the last row
				self.listOfLetterBoxes.append((startCol, startRow, endCol, endRow))

			self.startColOfPrevFontLetter = startCol	  #update for next pass
			#delete the letter - even in the case of ignoring the first double letter
			for x in range(startCol, endCol+1):
				for y in range(startRow - trimTopPixels, endRow+1 + trimBottomPixels):
				   loadedImag[x, y] = 0, 0, 0, 0
			return 0
		else:
			return -1
#
#
#
	def generateModFiles(self, customBaselineOffs):
		""" Generate font (FON) files (work on copies, not the originals). Return values: 0 no errors, -1 output font file has alrady new letters, -2 no fonts found in png (TODO: more error cases)
		"""
		#
		# When a customBaselineOffs is set, we should expand the space for the letter (otherwise it will be overflown in the next line or truncated (outside the png)
		# We can't expand the space for the letter downwards, because the engine will (com)press the new height to fit its expected and it will look bad.
		# NO THAT WON'T WORK--> The font will remain in the wrong place: Should we	CUT from the top and hope that we don't trunctate!? Keeping the resulting height equal to the expected one?
		# MAYBE: Do not alter the baseline of the original file, but the detected (popular) one of the line file!!!
		retVal = 0
		totalFontLetters = 0
		importedNumOfLetters = 0
		errMsg = ""
		errorFound = False
		im = None
		pix = None
		pixReloaded = None
		#
		# CONSTANTS
		#
		origGameFontDiakenoHeight = 0
		interLetterSpacingInPNG = 4
		origGameFontSizeEqBaseLine = 0
		# offset for start of PNG index table
		#firstTableLineOffset = self.PNG_TABLE_STARTLINE_OFFSET
		lettersInOriginalFontFile = self.lettersInOriginalFontFile
		#
		# detection of origGameFontSizeEqBaseLine
		#
		#origGameFontSizeEqBaseLine = self.findDetectedBaseline() ## NEW BR REMOVED
		self.cleanup() # necessary after detection of baseline, because it fills up some of the	 lists used in the following!

		##self.origFontPropertiesTxt = self.getImagePropertiesInfo(True) # "%dx%dx%s" % (im.size[0],im.size[1], im.mode) # NEW REMOVED
		#
		#if gTraceModeEnabled:
		#	print ("[Debug] WEEEE::: ", self.imageOriginalPNG, im.format, "%dx%d" % im.size, im.mode)
		#	print ("[Debug] BASELINE DETECTED: %d " % origGameFontSizeEqBaseLine)

		#
		# OPEN THE IMAGE WITH THE ROW OF CHARACTER FONTS TO BE IMPORTED
		#
		if os.access(self.imageRowFilePNG, os.F_OK) :
			try:
				im = Image.open(self.imageRowFilePNG)
			except:
				errMsg = "[Error] No letters were found in input png - IO exception!"
				print (errMsg)
				retVal = -2
				errorFound = True
		else:
			errMsg = "[Error] No letters were found in input png - IO fault!"
			print (errMsg)
			retVal = -2
			errorFound = True
		if not errorFound:
			#debug
			if gTraceModeEnabled:
				print ("[Debug] ", self.imageRowFilePNG, im.format, "%dx%d" % im.size, im.mode)
			w1, h1 = im.size
			trimTopPixels = 0
			trimBottomPixels = 0
			italicsMode = False	  # will be set to true only if the prefix of the row file is itcrp_ or it_ in order to activate some extra settings for kerning and letter width!
			# TODO the note about special handling of row PNG files with it_ or itcrp_ prefix, should be moved to the documentation
			# TODO the special settings for handling italic native letters should be in the settings(?)
			filepathSplitTbl = os.path.split(self.imageRowFilePNG)
			sFilenameOnlyImageRowFilePNG = filepathSplitTbl[1]
			print ("[Info] Parsing input Font glyphs image %s. Please wait..." % (sFilenameOnlyImageRowFilePNG))

			if sFilenameOnlyImageRowFilePNG.startswith("itcrp_") or sFilenameOnlyImageRowFilePNG.startswith("it_"):
				italicsMode = True

			if sFilenameOnlyImageRowFilePNG.startswith("itcrp_"):
				trimTopPixels = 1
				trimBottomPixels = 1
				print ("[Info] Mode Enabled: Trim upper line by %d pixels and bottom line by %d pixels." % (trimTopPixels, trimBottomPixels))
			pix = im.load()
			# pix argument is mutable (will be changed in the parseImage body)
			if self.parseImage(pix, w1, h1, trimTopPixels, trimBottomPixels, True) == 0:	#first run, just get the start column, ignore the letter - don't store it . We need this for the tab space width calculation and eventually the kerning calc of the letters
				# after the first call, we got an update on self.startColOfPrevFontLetter using the dummy double firstg letter font
				while self.parseImage(pix, w1, h1, trimTopPixels, trimBottomPixels) == 0:
					self.lettersFound = self.lettersFound + 1 # == 0 means one character font was detected so +1 to the counter
			#debug
			#if gTraceModeEnabled:
			#	print ("[Debug] ", self.listOfBaselines)
			print ("[Info] Font Glyphs Detected (not including the first double): %d" % (self.lettersFound))
			if (self.lettersFound ) > 0 :
				#if gTraceModeEnabled:
				#	print ([Debug] widths: ", self.listOfWidths[0:])
				#	print ("[Debug] Plain x offsets:")
				#	print (list(zip(self.targetLangOrderAndListOfForeignLettersAsciiValues[1:], self.listOfXOffsets)))
				# normalize x offsets
				# minXoffset = min(self.listOfXOffsets)
				#if(minXoffset < 0):
				#	 addNormalizer = minXoffset * (-1)
				#	 self.listOfXOffsets = [ x + addNormalizer	for x in self.listOfXOffsets]
				#if gTraceModeEnabled:
				#	print ("[Debug] Normalized x offsets: ")
				#	print (self.listOfXOffsets)
				# calculate y offsets
				(listOfStartCols, listOfStartRows, listOfEndCols, listOfEndRows) = list(zip(* self.listOfLetterBoxes))
				minTopRow = min(listOfStartRows)
				self.listOfYOffsets = [ x - minTopRow for x in listOfStartRows]
				if (self.yOffsetForAllGlyphsExceptFirstSpecialGamma != 0):
					self.listOfYOffsets =  [ x + self.yOffsetForAllGlyphsExceptFirstSpecialGamma for x in self.listOfYOffsets]
				#if gTraceModeEnabled:
				#	print ("[Debug] Y offsets: ")
				#	print (self.listOfYOffsets)
				#
				#
				# Actually explicit Width setting could affect this, so calculate a new list here with final widths and get the max on that list!
				#
				#
				listOfCalcWidths = []
				kIncIndx = 1
				for tmpWidth in self.listOfWidths:
					explicitWidthIncrementVal = 0
					if len(self.listOfWidthIncrements ) > 0:
						tmpOrd = self.targetLangOrderAndListOfForeignLettersAsciiValues[kIncIndx]
						keysOfWidthIncrements, valuesOfWidthIncrements = (list(zip(*self.listOfWidthIncrements)))
						if tmpOrd in keysOfWidthIncrements:
							#if gTraceModeEnabled:
							#	print ("[Debug] Explicit width increment for %d: %d" % (tmpOrd, valuesOfWidthIncrements[keysOfWidthIncrements.index(tmpOrd)]))
							explicitWidthIncrementVal = valuesOfWidthIncrements[keysOfWidthIncrements.index(tmpOrd)]
							listOfCalcWidths.append(tmpWidth + explicitWidthIncrementVal )
					if explicitWidthIncrementVal == 0:
						listOfCalcWidths.append(tmpWidth)
					kIncIndx = kIncIndx + 1
				#maxFontWidth =	 max(self.listOfWidths)
				maxFontWidth =	max(listOfCalcWidths)
				maxFontHeight =	 max(self.listOfHeights)
				if gTraceModeEnabled:
					print ("[Debug] Max Width: %d, Max Height: %d (not necessarily for the same character glyph)." % (maxFontWidth, maxFontHeight))
				#	print ("Index\tAsciiOrd\tX Offs\tY Offs\tWidth\tHeight")
				#	print (list(zip(range(1, len(self.listOfXOffsets)), self.targetLangOrderAndListOfForeignLettersAsciiValues[1:], self.listOfXOffsets, self.listOfYOffsets, listOfCalcWidths, self.listOfHeights)))
				numberOfEntriesInFontTable = self.maxAsciiValueInEncoding + 1 + 1  # 0x0100 # This is actually the max ascii value + plus one (1) to get the font index value + plus another one (1) to get the count (since we have zero based indices)
				# TODO ??? could be more than this if we need to keep other characters (not in our codeset) and expand the ascii table and offset the new characters
				if self.maxAsciiValueInEncoding > 254:
					print ("[Error] Max ascii value is too large. Should be less than or equal to 254 (max)")
					errorFound = True
					retVal = -3
				if not errorFound:
					targetFontFile = None
					try:
						targetFontFile = open(self.targetFONFilename, 'wb')
					except Exception as e:
						print ("[Error] Failed to create target font (FON) file: " + self.targetFONFilename + "::" + str(e))
						errorFound = True
					if not errorFound:
						print ("[Info] Creating target font (FON) file: " + self.targetFONFilename)
					# reopen the image with our Fonts because we deleted the letters in the in-mem copy
						im = None
						if os.access(self.imageRowFilePNG, os.F_OK) :
							try:
								im = Image.open(self.imageRowFilePNG)
							except:
								errorFound = True
						else:
							errorFound = True
						if not errorFound:
							pixReloaded = None
							pixReloaded = im.load()

							# first 4 bytes are the max ascii char value supported (it's basically the number of entries in the character index table)
							# next 4 bytes are max font char width	(pixels)
							# next 4 bytes are max font char height (pixels)
							# next 4 bytes give the size of the graphic segment for the font characters (this size is in word units, so it needs *2 to get the byte size)
							#				this size should be updated at the end (after filling the file with all font image data)
							#
							# pack 'I' unsigned int
							if gTraceModeEnabled:
								print ("[Debug] Number Of Entries In Font Table: ", (self.maxAsciiValueInEncoding + 1 + 1))
							numberOfEntriesInFontTableInFile = pack('I', numberOfEntriesInFontTable )
							targetFontFile.write(numberOfEntriesInFontTableInFile)
							maxFontWidthPixelsToWrite = pack('I', maxFontWidth)
							targetFontFile.write(maxFontWidthPixelsToWrite)
							maxFontHeightPixelsToWrite = pack('I', maxFontHeight)
							targetFontFile.write(maxFontHeightPixelsToWrite)
							fontImagesSegmentSize = pack('I', 0x0000) #	 - to be updated at the end!
							targetFontFile.write(fontImagesSegmentSize)

							startOfImageSegmentAbs = 0x10 + 20 * numberOfEntriesInFontTable # header is 0x10 bytes. Then table of  20 bytes *	numberOfEntriesInFontTable and then the data.
							lastImageSegmentOffset = 0
							#targetFontFile.close() # don't close here
							#
							# Fonts index table - properties and offset in image segment
							# TODO - REVISE WHEN FINISHED WITH COMPLETE TRANSCRIPT for special glyphs
							# So far additional required characters (not included in the standard ASCII (127 chars) are:
							# the spanish i (put it in ASCII value 0xA2 (162), font index 0xA3)? todo verify -- actual ASCII value in codepage 1252 is 0xED (237)
							# the spanish n (put it in ASCII value 0xA5 (165), font index 0xA6)? todo verify -- actual ASCII value in codepage 1252 is 0xF1 (241)
							# DONE we also need special fonts for liver	 pâté
							#											a actual ASCII value is 0xE2 (226) in codepage 1252 -- put it in ASCII value 0xA6 (166) -- font index 0xA7
							#											e actual ASCII value is 0xE9 (233) in codepage 1252 -- put it in ASCII value 0xA7 (167) -- font index 0xA8
							# In the row png font glyphs, the letter glyphs (images) should be the actual character fonts (spanish n, i etc)
							#				but in the overrideEncoding.txt we need the corresponding ASCII characters for the particular codepage of the text (eg here the greek windows-1253)
							#
							# NOTE! WARNING: We need to add the corresponding ASCII characters for our codepage (eg for Windows 1253 the characters with value 0xA2 and 0xA5 which are not the spanish characters but will act as delegates for them)
							# the greek Ά (alpha tonoumeno) character has ASCII value 0xA2 (162) (in codeset Windows 1253) so conflict with spanish i in in-game Tahoma -- put it in 0xA3 (163) (font index 0xA4)
							# We should fill all unused characters with "space" placeholder. Probably all of them should also point to the same area (second block) of the image segment too.
							# First block of the image area (font index = 0) is reserved and should be the "border" gamma-like character.
							#
							# Kerning of the first letter font is '1' for Tahoma18 (when shadowed from every side (the left side shadow reduces the kerning), otherwise it would be 2) -- TODO for now this should be a launch parameter
							# Y offset should be calculated from the top row of the heighest font
							#kIncIndx = 0
							## aux list because the order of the fonts in our overrideEncoding may not match the real order in the ascii table
							#listOfWriteableEntries = [ (0,0,0,0,0) for i in range(0, numberOfEntriesInFontTable)] # create a list of placeholders for the number of entries we need to write
							#print (" *************** DBG **************")
							#print (listOfWriteableEntries)
							#print (" *************** DBG **************")
							del self.properListOfLetterBoxes[:]
							for i in range(0, numberOfEntriesInFontTable):	# blocks of 20 bytes
								# 20 byte block
								# 4 bytes x offset (from what ref point? is this for kerning ) - CAN THIS BE NEGATIVE?
								# 4 bytes y offset (from what ref point? is this for the baseline?) - CAN THIS BE NEGATIVE?
								# 4 bytes char width
								# 4 bytes char height
								# 4 bytes offset in image segment (units in words (2 bytes))

								# TODO add all standard ascii characters in the ROW IMAGE before the additional required spanish and then GREEK alphabet characters --
								#											-- greek Ά should be at its proper place (between spanish i and spanish n).
								# TODO check possible support issues for ώ greek character
								if i == 0:
									# the first entry is a special font character of max width and max height with a horizontal line across the top-most row and a vertical line across the left-most column
									tmpXOffsetToWrite = pack('I', 0x0000)
									targetFontFile.write(tmpXOffsetToWrite)
									tmpYOffsetToWrite = pack('I', 0x0000)
									targetFontFile.write(tmpYOffsetToWrite)
									tmpWidthToWrite = pack('I', maxFontWidth)
									targetFontFile.write(tmpWidthToWrite)
									tmpHeightToWrite = pack('I', maxFontHeight)
									targetFontFile.write(tmpHeightToWrite)
									tmpDataOffsetToWrite = pack('I', 0x0000) # start of image segment means 0 offset
									targetFontFile.write(tmpDataOffsetToWrite)
									# TODO maybe conform more with game's format: Eg Tahoma24.fon (native game resource) does not always point to the second character font offset for dummy entries, but to the latest offset and only additionally sets the x-offset property (all others are 0) - eg look for 0x74c9 offsets (byte sequence 0xc9 0x74)
									dummyCharFontImageConstOffset = maxFontWidth * maxFontHeight; # const. actual offset in bytes is twice that. This counts in words (2-bytes)	 - This points to the first valid entry but with properties that make it translate as a space or dummy(?)
									lastImageSegmentOffset = maxFontWidth * maxFontHeight; # actual offset in bytes is twice that. This counts in words (2-bytes)
									#listOfWriteableEntries[0] = (tmpXOffsetToWrite, tmpYOffsetToWrite, tmpWidthToWrite, tmpHeightToWrite, tmpDataOffsetToWrite)
								else:
									if (i-1) in self.targetLangOrderAndListOfForeignLettersAsciiValues:
										# then this is an actual entry
										# i-1 is the order of an ascii character, that should be placed in the next slot in the output FON file
										# but this ascii character in the input overrideEncoding could be not it the same i spot -- because of the correspondance to an out-of-order PNG row file
										kIncIndxLst = [item for item in enumerate(self.targetLangOrderAndListOfForeignLettersAsciiValues[1:], 0) if item[1] == (i-1) ]
										kIncIndx = kIncIndxLst[0][0]
										#kIncIndx = self.targetLangOrderAndListOfForeignLettersAsciiValues.index(i-1)
										#if gTraceModeEnabled:
										#	print ("[Debug] ", kIncIndxLst)
										#	print ("[Debug] ", kIncIndx, i-1)
										#	print ("[Debug] ", i, ": actual entry index of ascii char", (i-1)," width:", self.listOfWidths[kIncIndx])
										#	print ("[Debug] Self explicit kerning list: ", self.listOfExplicitKerning)
										if len(self.listOfExplicitKerning ) > 0:
											keysOfExplicitKerning, valuesOfExplicitKerning = (list(zip(*self.listOfExplicitKerning)))
											if (i - 1) in keysOfExplicitKerning:
												# found explicit kerning for this
												#if gTraceModeEnabled:
												#	print ("[Debug] Explicit kerning for %d " % (i-1))
												self.listOfXOffsets[kIncIndx] = valuesOfExplicitKerning[keysOfExplicitKerning.index(i-1)] # explicit X offset

										tmpXOffsetToWrite = pack('i', self.listOfXOffsets[kIncIndx]) # x offset - from left			# TODO check if ok. Changed to signed int since it can be negative sometimes!
										targetFontFile.write(tmpXOffsetToWrite)
										tmpYOffsetToWrite = pack('I', self.listOfYOffsets[kIncIndx])   # y offset from topmost row
										targetFontFile.write(tmpYOffsetToWrite)

										if len(self.listOfWidthIncrements ) > 0:
											keysOfWidthIncrements, valuesOfWidthIncrements = (list(zip(*self.listOfWidthIncrements)))
											if (i - 1) in keysOfWidthIncrements:
												#if gTraceModeEnabled:
												#	print ("[Debug] Explicit width increment for %d " % (i-1))
												foundExplicitWidthIncrement = True
												self.listOfWidths[kIncIndx] = self.listOfWidths[kIncIndx] + valuesOfWidthIncrements[keysOfWidthIncrements.index(i-1)]

										tmpWidthToWrite = pack('I', self.listOfWidths[kIncIndx] )
										targetFontFile.write(tmpWidthToWrite)
										tmpHeightToWrite = pack('I', self.listOfHeights[kIncIndx])
										targetFontFile.write(tmpHeightToWrite)
										tmpDataOffsetToWrite = pack('I', lastImageSegmentOffset) #
										targetFontFile.write(tmpDataOffsetToWrite)
										lastImageSegmentOffset = lastImageSegmentOffset +  self.listOfWidths[kIncIndx]  * self.listOfHeights[kIncIndx]
										#kIncIndx = kIncIndx + 1 # increases only for valid characters
										#
										# populate self.properListOfLetterBoxes here
										#
										self.properListOfLetterBoxes.append(self.listOfLetterBoxes[kIncIndx])
									else:
										#
										#if gTraceModeEnabled:
										#	print ("[Debug] ", i, ": phony entry")
										# TODO in-game resource fonts don't point all to the first entry as dummy but to the last valid entry encountered
										tmpXOffsetToWrite = pack('I', 0x0000)	# 0 x offset
										targetFontFile.write(tmpXOffsetToWrite)
										tmpYOffsetToWrite = pack('I', 0x0000)	# 0 y offset
										targetFontFile.write(tmpYOffsetToWrite)
										tmpWidthToWrite = pack('I', self.spaceWidthInPixels) # font width set for some pixels of space
										targetFontFile.write(tmpWidthToWrite)
										tmpHeightToWrite = pack('I', 0x0000)
										targetFontFile.write(tmpHeightToWrite)
										tmpDataOffsetToWrite = pack('I', dummyCharFontImageConstOffset) #
										targetFontFile.write(tmpDataOffsetToWrite)
							# end of for loop over all possible ascii values contained in the fon file
							# print the corrected properties per glyph font:
							if gTraceModeEnabled:
								print ("***** FINAL (Explicit kerning and width accounted) *****\nIndex\tAsciiOrd\tX Offs\tY Offs\tWidth\tHeight")
								tmpListOfTuplesToPrintDbg = list(zip(range(1, len(self.listOfXOffsets)), self.targetLangOrderAndListOfForeignLettersAsciiValues[1:], self.listOfXOffsets, self.listOfYOffsets, listOfCalcWidths, self.listOfHeights))
								for itemDbg in tmpListOfTuplesToPrintDbg:
									print ("%4d\t%8d\t%6d\t%6d\t%6d\t%6d" % (itemDbg[0], itemDbg[1], itemDbg[2], itemDbg[3], itemDbg[4], itemDbg[5]))
								#print ("\n")
							#
							#
							# Now fill in the image segment
							# Fonts are written from TOP to Bottom, Left to Right. Each pixel is 16 bit (2 bytes). Highest bit seems to determine transparency (on/off flag).
							#
							# There seem to be 5 bits per RGB channel and the value is the corresponding 8bit value (from the 24 bit pixel color) shifting out (right) the 3 LSBs
							# NOTE: Since we can't have transparency at channel level(?), it's best to have the input PNG not have transparent colored pixels (in Gimp merge the font layers, foreground and shadow and then from Layer settings set transparency threshold to 0 for that layer)- keep the background transparent!
							#
							# First font image is the special character (border of top row and left column) - color of font pixels should be "0x7FFF" for filled and "0x8000" for transparent
							#
							#
							# Then follow up with the image parts for each letter!
							#
							#
							#
							# START of First special character image segment
							#
							for i in range(0, maxFontWidth * maxFontHeight):
								if (i < maxFontWidth or i % maxFontWidth == 0):
									tmpPixelColorRGB555ToWrite = pack('H', 0x7FFF) #unsigned short - 2 bytes
									targetFontFile.write(tmpPixelColorRGB555ToWrite)
								else:
									tmpPixelColorRGB555ToWrite = pack('H', 0x8000)
									targetFontFile.write(tmpPixelColorRGB555ToWrite) # unsigned short - 2 bytes
							#
							# END of First special character image segment
							#
							#
							# Start rest of the font characters image segments
							#
							#

							#
							# If we have a character with explicit width increment (y) we should add columns of transparent colored pixels at the end (so since this is done by row, we should add y number of transparent pixels at the end of each row)
							kIncIndx = 1 # start after the first glyph (which is DOUBLE)
							for (c_startCol, c_startRow, c_endCol, c_endRow) in self.properListOfLetterBoxes[0:]:
								#if gTraceModeEnabled:
								#	print ("[Debug] ", (c_startCol, c_startRow, c_endCol, c_endRow),' for letter ', self.targetLangOrderAndListOfForeignLettersAsciiValues[kIncIndx])
								explicitWidthIncrementVal = 0
								if len(self.listOfWidthIncrements ) > 0:
									tmpOrd = self.targetLangOrderAndListOfForeignLettersAsciiValues[kIncIndx]
									keysOfWidthIncrements, valuesOfWidthIncrements = (list(zip(*self.listOfWidthIncrements)))
									if tmpOrd in keysOfWidthIncrements:
										#if gTraceModeEnabled:
										#	print ("[Debug] Explicit width increment for %d: %d" % (tmpOrd, valuesOfWidthIncrements[keysOfWidthIncrements.index(tmpOrd)]))
										explicitWidthIncrementVal = valuesOfWidthIncrements[keysOfWidthIncrements.index(tmpOrd)]


								for tmpRowCur in range(c_startRow, c_endRow + 1):
									for tmpColCur in range(c_startCol, c_endCol +1):
										#if gTraceModeEnabled:
										#	print ("[Debug] ", (tmpRowCur, tmpColCur))
										r1,g1,b1,a1 = pixReloaded[tmpColCur, tmpRowCur] # Index col first, row second for image pixel array. TODO asdf this pix has been modified. All pixels would be transparent? - load image again?
										if (a1 == 0):
											#if gTraceModeEnabled:
											#	print ("[Debug] With alpha 8bit: ", (r1, g1, b1, a1))
											#make completely transparent - write 0x8000
											tmpPixelColorRGB555ToWrite = pack('H', 0x8000)
											targetFontFile.write(tmpPixelColorRGB555ToWrite) # unsigned short - 2 bytes
										else:	  # alpha should be 255 here really.
											#if gTraceModeEnabled:
											#	print ("[Debug] 8bit:", (r1, g1, b1))
											tmp5bitR1 = (r1 >> 3) & 0x1f
											tmp5bitG1 = (g1 >> 3) & 0x1f
											tmp5bitB1 = (b1 >> 3) & 0x1f
											#if gTraceModeEnabled:
											#	print ("[Debug] 5bit: ", (tmp5bitR1, tmp5bitG1, tmp5bitB1))
											tmpPixelColorConvertedToRGB555 = (tmp5bitR1 << 10) | (tmp5bitG1 << 5) | (tmp5bitB1)
											#if gTraceModeEnabled:
											#	print ("[Debug] 16bit: ", tmpPixelColorConvertedToRGB555)
											tmpPixelColorRGB555ToWrite = pack('H', tmpPixelColorConvertedToRGB555)
											targetFontFile.write(tmpPixelColorRGB555ToWrite) # unsigned short - 2 bytes
										if (tmpColCur == c_endCol and explicitWidthIncrementVal > 0):
											for tmpExtraColCur in range (0, explicitWidthIncrementVal):
												#make completely transparent - write 0x8000
												tmpPixelColorRGB555ToWrite = pack('H', 0x8000)
												targetFontFile.write(tmpPixelColorRGB555ToWrite) # unsigned short - 2 bytes
								kIncIndx = kIncIndx + 1 # finally increase the kIncIndx for next glyph

							#
							# End rest of the font characters image segments
							#
							targetFontFile.close()
							#
							# Re -open and write the image segment
							#
							targetFontFile = None
							try:
								targetFontFile = open(self.targetFONFilename, 'r+b')
							except:
								errorFound = True
							if not errorFound:
								targetFontFile.seek(0x0C)	  # position to write imageSegmentSize
								tmpImageSegmentToWrite = pack('I', lastImageSegmentOffset)
								targetFontFile.write(tmpImageSegmentToWrite)
								targetFontFile.close()

			else: ## if (self.lettersFound ) <= 0
				errMsg = "[Error] No letters were found in input png!"
				print (errMsg)
				retVal = -2
		if retVal == 0:
			print ("[Info] Done.")
		else:
			print ("[Error] Errors were found during execution.")
		return (retVal, errMsg, origGameFontSizeEqBaseLine, totalFontLetters, importedNumOfLetters)

	def exportFonFilesFromMix(self):
		""" Generate PNG files out of FON files stores in a MIX resource
		"""
		print ("[Info] Checking in %s for MIX files to export font files (FON) from..." % (self.inputFonMixPath))
		inputMIXFilesFound = []
		# breaking after first for loop yields only the top directory files, which is what we want
		for (dirpath, dirnames, filenames) in walk(self.inputFonMixPath):
			for filename in filenames:
				for mixFileName in SUPPORTED_MIX_INPUT_FILES:
					if filename.upper() == mixFileName:
						inputMIXFilesFound.append(mixFileName)
			break

		if len(inputMIXFilesFound) == 0:
			print ("[Error] No supported game archive resource files (MIX) were found in the specified input path (-ip switch)")
			sys.exit(1)

		for tmpMIXfileName in inputMIXFilesFound:
			print ("[Info] Found MIX file: %s" % ('"' + tmpMIXfileName + '"'))
			errorFound = False
			inMIXFile = None
			#
			try:
				inMIXFile = open(os.path.join(self.inputFonMixPath, tmpMIXfileName), 'rb')
			except:
				errorFound = True
				print ("[Error] Unexpected event:", sys.exc_info()[0])
				raise
			if not errorFound:
				totalFONs = 0
				tmpBuff = inMIXFile.read(2)
				# H: unsigned short (2 bytes) followed by I: unsigned int (4 bytes)
				mixFileEntriesNumTuple = struct.unpack('H', tmpBuff)
				numOfEntriesToExport = mixFileEntriesNumTuple[0]
				tmpBuff = inMIXFile.read(4)
				mixFileDataSegmentSizeTuple = struct.unpack('I', tmpBuff)
				allMixFileSize = mixFileDataSegmentSizeTuple[0]
				inMIXFile.seek(0, 2) # go to file end
				allActualBytesInMixFile = inMIXFile.tell()
				inMIXFile.seek(6, 0) # go to start of table of MIX file entries (right after the 6 bytes header)
				# 2 + 4 = 6 bytes short MIX header
				# 12 bytes per MIX entry in entries table
				# quick size validation
				if gTraceModeEnabled:
					print ("[Debug] MIX file entries: %d, Data segment: %d bytes" % (numOfEntriesToExport, allMixFileSize))
				if allActualBytesInMixFile != 2 + 4 + 12 * numOfEntriesToExport + allMixFileSize:
					print ("[Error] MIX file size mismatch with reported size in header for %s!" % (tmpMIXfileName))
				else:
					#
					# 12 bytes per entry
					#		4 bytes: ID
					#		4 bytes: Offset in data segment
					#		4 bytes: Size of data
					#
					if gTraceModeEnabled:
						print ("[Debug] Checking for supported font file (FON) entries in MIX file...")

					for i in range(0, numOfEntriesToExport):
						foundFONFile = False
						currFonFileName = 'UNKNOWN.FON'
						inMIXFile.seek(2 + 4 + 12*i)
						tmpBuff = inMIXFile.read(4)
						tmpRdTuple = struct.unpack('I', tmpBuff)
						idOfMIXEntry = tmpRdTuple[0]
						tmpBuff = inMIXFile.read(4)
						tmpRdTuple = struct.unpack('I', tmpBuff)
						offsetOfMIXEntry = tmpRdTuple[0]
						tmpBuff = inMIXFile.read(4)
						tmpRdTuple = struct.unpack('I', tmpBuff)
						sizeOfMIXEntry = tmpRdTuple[0]

						for suppFONFileName in SUPPORTED_EXPORTED_GAME_FON_FILES:
							if(idOfMIXEntry == calculateFoldHash(suppFONFileName)):
								foundFONFile = True
								currFonFileName = suppFONFileName
								break

						if (foundFONFile == True):
							if gTraceModeEnabled:
								print ("\n[Debug] Entry Name: %s, Entry ID: %s, Offset: %s, Data segment: %s bytes" % (currFonFileName, ''.join('{:08X}'.format(idOfMIXEntry)), ''.join('{:08X}'.format(offsetOfMIXEntry)),''.join('{:08X}'.format(sizeOfMIXEntry))))
							#
							# IF FON FILE:
							# put file in FON object
							#
							#
							inMIXFile.seek(2 + 4 + 12*numOfEntriesToExport + offsetOfMIXEntry)
							if(offsetOfMIXEntry + sizeOfMIXEntry > allMixFileSize):
								print ("[Error] Font file (FON) size mismatch with reported size in entry header!")
							else:
								fonFileBuffer = inMIXFile.read(sizeOfMIXEntry)
								if (len(fonFileBuffer) == sizeOfMIXEntry):
								# load FON file
									thisFonFile = fonFile(gTraceModeEnabled)
									if (thisFonFile.loadFonFile(fonFileBuffer, allMixFileSize, currFonFileName)):
										if gTraceModeEnabled:
											print ("[Debug] Font file (FON) %s was loaded successfully!" % (currFonFileName))
										thisFonFile.outputFonToPNG()
										totalFONs = totalFONs + 1
									else:
										print ("[Error] Error while loading font file (FON) %s!" % (currFonFileName))
								else:
									print ("[Error] Error while reading font file (FON) %s into mem buffer" % (''.join('{:08X}'.format(idOfMIXEntry))))

				inMIXFile.close()
				print ("[Info] Total font files (FONs) processed: %d " % (totalFONs))
				print ("[Info] Done.")
		return

#
#
# ########################
# main
def main(argsCL):
	# TODO parse arguments using argparse? https://docs.python.org/3/library/argparse.html#module-argparse
	global gTraceModeEnabled
	gTraceModeEnabled = False
	invalidSyntax = False
	exportFonMode = False

	TMPSpecialGlyphMode = True
	TMPAutoTabCalculation = True

	TMPOverrideEncodingFilePath = ""
	TMPinputPathForMixFiles = ""
	TMPimageRowFilePNG = ""
	TMPTargetFONfilename = ""
	TMPminSpaceBetweenLettersInRowLeftToLeft = 0
	TMPminSpaceBetweenLettersInColumnTopToTop = 0
	TMPkerningForFirstDummyFontLetter = 0
	TMPYOffsToApplyToAllGlyphsExceptFirstSpecialGamma = 0
	TMPSpaceWidthInPixels = 10
	#TMPdeductKerningPixels = 0
	TMPcustomBaseLineOffset = 0
	print ("Running %s (%s)..." % (APP_NAME_SPACED, APP_VERSION))
	#print ("Len of sysargv = %s" % (len(argsCL)))
	if len(argsCL) == 2:
		if(argsCL[1] == '--help'or argsCL[1] == '-h'):
			print ("%s %s supports Westwood's Blade Runner PC Game (1997)." % (APP_NAME_SPACED, APP_VERSION))
			print (APP_SHORT_DESC)
			print ("Created by Praetorian of the classic adventures in Greek team.")
			print ("Always keep backups!")
			print ("--------------------")
			print ("Preparatory steps:")
			print ("1. Put overrideEncoding.txt file in the same folder with this tool. (Recommended, but not obligatory step)")
			print ("--------------------")
			print ("Valid syntax A - export game fonts to PNG images:")
			print ("%s -ip folderpath_for_MIX_Files [--trace]\n" % (APP_WRAPPER_NAME))
			print ("Valid syntax B - create subtitle font:")
			print ("%s -im image_Row_PNG_Filename -om output_FON_filename -pxLL minSpaceBetweenLettersInRowLeftToLeft -pxTT minSpaceBetweenLettersInColumnTopToTop -pxKn kerningForFirstDummyFontLetter -pxWS whiteSpaceWidthInPixels [--noSpecialGlyphs] [--noAutoTabCalculation] [--trace]\n" % (APP_WRAPPER_NAME))    # deductKerningPixels"
			print ("The -ip switch has an argument that is the path for the input (MIX) files folder (can be the same as the Blade Runner installation folder).")
			print ("The -oe switch has an argument that is the input overrideEncoding file to use for the particular font creation.")
			print ("The -im switch has an argument that is the input PNG image with a row of the font glyphs spaced apart.")
			print ("The -om switch has an argument that is the output FON filename.")
			print ("The -pxLL switch has an integer argument that specifies the minimum number of pixels between the left side of a glyph and the left side of the next glyph to its right in the line-row PNG.")
			print ("The -pxTT switch has an integer argument that specifies the minimum number of pixels between the top side of a glyph and the top side of the glyph below (if there's a second row) in the row PNG. If there is only one row, this argument should still be set (as if there was another row) to define where the parser should stop checking for the limits of a glyph vertically.")
			print ("The -pxKn switch has an integer argument that sets kerning for the first dummy font glyph.")
			print ("The -pxYo switch has an integer argument that sets an offset to be added to all detected y offsets for the glyphs (except the special first one).")
			print ("The -pxWS switch has an integer argument that sets the white space width in pixels for this particular font.")
			print ("The --noSpecialGlyphs switch removes consideration for special glyphs that exist out of their proper ascii order.")
			print ("The --noAutoTabCalculation switch removes the detection of tab spacing between letters (use this switch if you didn't create the PNG row file using a tab spaced list of glyphs).")
			print ("The --trace switch enables more debug messages being printed during execution.")
			print ("--------------------")
			print ("Thank you for using this app.")
			print ("Please provide any feedback to: %s " % (COMPANY_EMAIL))
			sys.exit()
		elif(argsCL[1] == '--version' or argsCL[1] == '-v'):
			print ("%s %s supports Westwood's Blade Runner PC Game (1997)." % (APP_NAME_SPACED, APP_VERSION))
			print ("Please provide any feedback to: %s " % (COMPANY_EMAIL))
			sys.exit()
		else:
			invalidSyntax = True
	elif len(argsCL) > 2:
		for i in range(1, len(argsCL)):
			if( i < (len(argsCL) - 1) and argsCL[i][:1] == '-' and argsCL[i+1][:1] != '-'):
				if (argsCL[i] == '-ip'):
					TMPinputPathForMixFiles = argsCL[i+1]
					exportFonMode = True
					print ("[Info] Original font file (FON) exporting mode enabled.")
				elif (argsCL[i] == '-oe'):
					TMPOverrideEncodingFilePath = argsCL[i+1]
				elif (argsCL[i] == '-im'):
					TMPimageRowFilePNG = argsCL[i+1]
				elif (argsCL[i] == '-om'):
					TMPTargetFONfilename = argsCL[i+1]
				elif (argsCL[i] == '-pxLL'):
					TMPminSpaceBetweenLettersInRowLeftToLeft = int(argsCL[i+1])
				elif (argsCL[i] == '-pxTT'):
					TMPminSpaceBetweenLettersInColumnTopToTop = int(argsCL[i+1])
				elif (argsCL[i] == '-pxKn'):
					TMPkerningForFirstDummyFontLetter = int(argsCL[i+1])
				elif (argsCL[i] == '-pxYo'):
					TMPYOffsToApplyToAllGlyphsExceptFirstSpecialGamma = int(argsCL[i+1])
				elif (argsCL[i] == '-pxWS'):
					TMPSpaceWidthInPixels = int(argsCL[i+1])
			elif argsCL[i] == '--noSpecialGlyphs':
				print ("[Info] No special out-of-order glyphs mode enabled.")
				TMPSpecialGlyphMode = False
			elif argsCL[i] == '--noAutoTabCalculation':
				print ("[Info] No automatic tab calculation between glyphs.")
				TMPAutoTabCalculation = False
			elif sys.argv[i] == '--trace':
				print ("[Info] Trace mode enabled (more debug messages).")
				gTraceModeEnabled = True
			elif argsCL[i][:1] == '-':
				invalidSyntax = True
				break

		if (exportFonMode == False) and (not TMPTargetFONfilename or not TMPimageRowFilePNG or TMPminSpaceBetweenLettersInRowLeftToLeft <= 0 or TMPminSpaceBetweenLettersInColumnTopToTop <= 0 or TMPkerningForFirstDummyFontLetter <= 0 or TMPSpaceWidthInPixels <= 0)  : # this argument is mandatory
			invalidSyntax = True

		if (exportFonMode == True) and ( (not TMPinputPathForMixFiles) ): # not needed for exporting mode --  or  not TMPOverrideEncodingFilePath ):
			invalidSyntax = True
	else:
		invalidSyntax = True


	if (invalidSyntax == False):
		#myGrabInstance = grabberFromPNG('windows-1253') #, grabberFromPNG.BR_GameID)
		myGrabInstance = grabberFromPNG()
		myGrabInstance.setInputPathForFonMix(TMPinputPathForMixFiles)
		myGrabInstance.setImageRowFilePNG(TMPimageRowFilePNG)
		myGrabInstance.setTargetFONFilename(TMPTargetFONfilename)
		myGrabInstance.setMinSpaceBetweenLettersInRowLeftToLeft(TMPminSpaceBetweenLettersInRowLeftToLeft)
		myGrabInstance.setMinSpaceBetweenLettersInColumnTopToTop(TMPminSpaceBetweenLettersInColumnTopToTop)
		myGrabInstance.setKerningForFirstDummyFontLetter(TMPkerningForFirstDummyFontLetter)
		myGrabInstance.setYOffsetForAllGlyphsExceptFirstSpecialGamma(TMPYOffsToApplyToAllGlyphsExceptFirstSpecialGamma)
		myGrabInstance.setSpaceWidthInPixels(TMPSpaceWidthInPixels)
		myGrabInstance.setSpecialGlyphMode(TMPSpecialGlyphMode)
		myGrabInstance.setAutoTabCalculation(TMPAutoTabCalculation)
		myGrabInstance.setOverrideEncodingPath(TMPOverrideEncodingFilePath)
		#myGrabInstance.setDeductKerningPixels(TMPdeductKerningPixels)
		if exportFonMode:
			myGrabInstance.exportFonFilesFromMix()
		else:
			myGrabInstance.initOverrideEncoding()
			myGrabInstance.generateModFiles(TMPcustomBaseLineOffset)
	else:
		invalidSyntax = True

	if invalidSyntax == True:
		print ("[Error] Invalid syntax\n Try: \n %s --help for more info \n %s --version for version info " % (APP_WRAPPER_NAME, APP_WRAPPER_NAME))
		print ("Valid syntax A - export game fonts to PNG images:")
		print ("%s -ip folderpath_for_MIX_Files [--trace]\n" % (APP_WRAPPER_NAME))
		print ("Valid syntax B - create subtitle font:")
		print ("%s -im image_Row_PNG_Filename -om output_FON_filename -pxLL minSpaceBetweenLettersInRowLeftToLeft -pxTT minSpaceBetweenLettersInColumnTopToTop -pxKn kerningForFirstDummyFontLetter -pxWS whiteSpaceWidthInPixels [--noSpecialGlyphs] [--noAutoTabCalculation] [--trace]\n" % (APP_WRAPPER_NAME))    # deductKerningPixels"
		print ("\nDetected arguments:")
		tmpi = 0
		for tmpArg in argsCL:
			if tmpi==0: #skip first argument
				tmpi+=1
				continue
			print ("Argument: %s" % (tmpArg))
			tmpi+=1


#
# #########################
#
if __name__ == '__main__':
	main(sys.argv[0:])
else:
	## debug
	#print ("[Debug] %s was imported from another module" % (APP_NAME))
	pass
