#!/usr/bin/env python2.7
# -*- coding: UTF-8 -*-
#
# Created by Praetorian (ShadowNate) for Classic Adventures in Greek
# classic.adventures.in.greek@gmail.com
# Works with Excel version outSpeech-15-06-2018-1856-TranslatingComms-080.xls and above
#
# DONE - Support at least one translation too (ie Greek)
# Print a warning if packing a TRE without the corresponding font(s) -- only a warning though
#
import os, sys

shutilLibFound = False
ctypesLibFound = False
csvLibFound = False
xlrdLibFound = False
reLibFound = False
structLibFound = False

try:
	import shutil 
except ImportError:
	print "Error:: Shutil python library is required to be installed!" 
else:
	shutilLibFound = True

try:
	import ctypes 
except ImportError:
	print "Error:: ctypes python library is required to be installed!" 
else:
	ctypesLibFound = True

try:
	import csv 
except ImportError:
	print "Error:: csv python library is required to be installed!" 
else:
	csvLibFound = True
	
try:
	import xlrd 
except ImportError:
	print "Error:: xlrd python library is required to be installed!" 
else:
	xlrdLibFound = True

try:
	import re 
except ImportError:
	print "Error:: re (Regular expression operations) python library is required to be installed!" 
else:
	reLibFound = True	

try:
	import struct 
except ImportError:
	print "Error:: struct python library is required to be installed!" 
else:
	structLibFound = True	
	
if 	(not shutilLibFound) or (not ctypesLibFound) or (not csvLibFound) or (not xlrdLibFound) or (not reLibFound) or (not structLibFound):
	sys.stdout.write("Error:: Errors were found when trying to import required python libraries\n")
	sys.exit(1)


from os import path
from xlrd import *
# for pack
from struct import *

company_email = "classic.adventures.in.greek@gmail.com"
app_version = "0.70"
app_name = "packBladeRunnerMIXFromPCTLKXLS"
app_wrapper_name = "mixResourceCreator.py"
app_name_spaced = "Blade Runner MIX Resource Creator"
app_short_desc = "Get a TRE file from spoken in-game quotes"
numOfSpokenQuotes = 0

defaultSubtitlesFontName = 'SUBTLS_E.FON'
defaultSubtitlesMixOutputName = u'SUBTITLES.MIX'

origEncoding = 'windows-1252'
defaultTargetEncoding = 'windows-1252'
defaultTargetEncodingUnicode = unicode(defaultTargetEncoding, 'utf-8')
targetEncoding = ''
targetEncodingUnicode = ''

traceModeEnabled = False

# DONE ADD ALL SHEETS NEEDED FROM THE XLS
# all dialogue sheets get the SUBTLS_E.FON for translation to TRE
# - TODO maybe merge this with TAHOMA18.FON eventually
supportedDialogueSheets = ['INGQUO_E.TRE', 'WSTLGO_E.VQA', 'BRLOGO_E.VQA', 'INTRO_E.VQA', 'MW_A_E.VQA', 'MW_B01_E.VQA', 'MW_B02_E.VQA', 'MW_B03_E.VQA', 'MW_B04_E.VQA', 'MW_B05_E.VQA', 'INTRGT_E.VQA', 'MW_D_E.VQA', 'MW_C01_E.VQA', 'MW_C02_E.VQA', 'MW_C03_E.VQA', 'END04A_E.VQA', 'END04B_E.VQA', 'END04C_E.VQA', 'END06_E.VQA', 'END01A_E.VQA', 'END01B_E.VQA', 'END01C_E.VQA', 'END01D_E.VQA', 'END01E_E.VQA', 'END01F_E.VQA', 'END03_E.VQA']
#
# Each TRE sheet gets a specific font to handle their translation to TRE
# TAHOMA means both TAHOMA (their translation should be identical (although in the original they have minor differences but they don't affect anything)
# We use a single naming for TAHOMA here because both TAHOMA18 and TAHOMA24 are used for ENDCRED.TRE
# The TRE files that are identically named to the originals are supposed to override them (needs ScummVM compatible functionality for that)

supportedTranslationSheets = [('OPTIONS.TRE', 'KIA6PT'), ('DLGMENU.TRE', 'KIA6PT'), ('SCORERS.TRE', 'TAHOMA'), ('VK.TRE', 'KIA6PT'), ('CLUES.TRE', 'KIA6PT'), ('CRIMES.TRE', 'KIA6PT'), ('ACTORS.TRE', 'KIA6PT'), ('HELP.TRE', 'KIA6PT'), ('AUTOSAVE.TRE', 'KIA6PT'), ('ERRORMSG.TRE', 'KIA6PT'), ('SPINDEST.TRE', 'KIA6PT'), ('KIA.TRE', 'KIA6PT'),  ('KIACRED.TRE', 'KIA6PT'), ('CLUETYPE.TRE', 'KIA6PT'), ('ENDCRED.TRE', 'TAHOMA'), ('POGO.TRE', 'KIA6PT')]
# The FON files that are identically named to the originals are supposed to override them (needs ScummVM compatible functionality for that)
supportedOtherFilesForMix = [defaultSubtitlesFontName, 'KIA6PT.FON', 'TAHOMA18.FON', 'TAHOMA24.FON'] # , '10PT.FON'] # we don't deal with 10PT.FON since it's not used -- TODO verify this.


tableOfStringIds = []
tableOfStringOffsets = []
tableOfStringEntries = []

# this list is used in order to replace the actual indices of characters with delegate font indices (ASCII indexes of the target code-page) which have been used during the font creation (or exist in in the internal TAHOMA font)
# contains tuples of two values. First value is actual Utf char, the second is a replacement ASCII char
listOfFontNamesToOutOfOrderGlyphs = []
arrangedListOfFontNamesToOutOfOrderGlyphs = []

actorPropertyEntries = []
actorPropertyEntriesWasInit = False

def initOverrideEncoding(pathToConfigureFontsTranslationTxt):
	global targetEncoding

	if pathToConfigureFontsTranslationTxt is None or not pathToConfigureFontsTranslationTxt:
		configureFontsTranslationTextFile = u'configureFontsTranslation.txt'
		relPath = u'.'
		pathToConfigureFontsTranslationTxt = os.path.join(relPath, configureFontsTranslationTextFile)
		print "Warning:: Font Translation Configuration file not found in arguments. Attempting to open local file %s if it exists" % (configureFontsTranslationTextFile)

	configureTranslationFailed = True
	try:
		if os.access(pathToConfigureFontsTranslationTxt, os.F_OK):
			conFontsTranslationFile = open(pathToConfigureFontsTranslationTxt, 'r')
			linesLst = conFontsTranslationFile.readlines()
			conFontsTranslationFile.close()
			if linesLst is None or len(linesLst) == 0:
				configureTranslationFailed = True
			else:
				if traceModeEnabled:
					print "Info:: Font Translation Configuration Info: "
				involvedTokensLst =[]
				for readEncodLine in linesLst:
					tmplineTokens = re.findall("[^\t\n]+",readEncodLine )
					for x in tmplineTokens:
						involvedTokensLst.append(x)

				for tokenNameKeyPair in involvedTokensLst:
					nameKeyTupl = tokenNameKeyPair.split('=', 1)
					try:
						if len(nameKeyTupl) == 2 and nameKeyTupl[0] == 'targetEncoding' and nameKeyTupl[1] is not None and nameKeyTupl[1] != '-' and nameKeyTupl[1] != '':
							targetEncodingUnicode = unicode(nameKeyTupl[1], 'utf-8')
							targetEncoding = unicode.encode("%s" % targetEncodingUnicode, origEncoding)
						elif len(nameKeyTupl) == 2 and nameKeyTupl[0] == 'fontNameAndOutOfOrderGlyphs' and nameKeyTupl[1] is not None and nameKeyTupl[1] != '':
							# split at hash tag first
							tmpListOfOutOfOrderGlyphs = []
							del(tmpListOfOutOfOrderGlyphs[:])
							fontNameAndOOOGlyphsTuple = nameKeyTupl[1].split('#', 1)
							if (len (fontNameAndOOOGlyphsTuple) == 2 and fontNameAndOOOGlyphsTuple[0] != '' and fontNameAndOOOGlyphsTuple[1] is not None and fontNameAndOOOGlyphsTuple[1] != ''):
								tmpFontName = fontNameAndOOOGlyphsTuple[0]
								# split at comma, then split at ':' and store tuples of character
								explicitOutOfOrderGlyphsTokenUnicode = unicode(fontNameAndOOOGlyphsTuple[1], 'utf-8') # unicode(fontNameAndOOOGlyphsTuple[1], 'utf-8')
								#explicitOutOfOrderGlyphsTokenStr =  unicode.encode("%s" % explicitOutOfOrderGlyphsTokenUnicode, targetEncoding)
								#explicitOutOfOrderGlyphsTokenStr =  explicitOutOfOrderGlyphsTokenUnicode.decode(targetEncoding) # unicode.encode("%s" % explicitOutOfOrderGlyphsTokenUnicode, 'utf-8')
								tokensOfOutOfOrderGlyphsStrList = explicitOutOfOrderGlyphsTokenUnicode.split(',')
								for tokenX in tokensOfOutOfOrderGlyphsStrList:
									tokensOfTupleList = tokenX.split(':')
									tmpListOfOutOfOrderGlyphs.append( (unichr(ord(tokensOfTupleList[0])), unichr(ord(tokensOfTupleList[1])))  )

								if tmpFontName not in [x[0] for x in listOfFontNamesToOutOfOrderGlyphs]:
									listOfFontNamesToOutOfOrderGlyphs.append(  ( tmpFontName,  tmpListOfOutOfOrderGlyphs) )

							else:
								configureTranslationFailed = True
								break
					except:
						configureTranslationFailed = True
						raise

				if not (targetEncoding is None or not targetEncoding):
					configureTranslationFailed = False

	except:
		print "Error:: while trying to access file for encoding info: %s" % (pathToConfigureFontsTranslationTxt)
		raise
		configureTranslationFailed = True

	if 	configureTranslationFailed == True:
#		targetEncoding = defaultTargetEncoding
		print "Error: Could not find proper override encoding info in: %s" % (pathToConfigureFontsTranslationTxt)
		sys.exit()	# terminate if override Failed (Blade Runner)
	#
	# TODO ASDF fix this!!!
	#
	if(len(listOfFontNamesToOutOfOrderGlyphs) == 0):
		tmpFontType = defaultSubtitlesFontName[:-4] # remove the .FON extensionFromTheName
		print "Info:: Empty list for out of order glyphs. Assuming default out of order glyphs and only for the %s font" % (tmpFontType)
		tmplistOfOutOfOrderGlyphs = []
		tmplistOfOutOfOrderGlyphs.append((u'\xed', u'\u0386')) # spanish i (si)
		tmplistOfOutOfOrderGlyphs.append((u'\xf1', u'\xa5')) # spanish n (senor)
		tmplistOfOutOfOrderGlyphs.append((u'\xe2', u'\xa6')) # a for (liver) pate
		tmplistOfOutOfOrderGlyphs.append((u'\xe9', u'\xa7')) # e for (liver) pate
		listOfFontNamesToOutOfOrderGlyphs.append( (tmpFontType, tmplistOfOutOfOrderGlyphs))
	if traceModeEnabled:
		print "Info:: Explicit Out Of Order Glyphs List: " , listOfFontNamesToOutOfOrderGlyphs
	# arrange list properly:
	# check if the list contains same item as key and value (in different pairs)
	# if such case then the pair with the key should preceed the pair with the value matched,
	# to avoid replacing instances of a special character (key) with a delegate (value) that will be later replaced again due to the second pair
	#
	for (itFontName, itOOOGlyphList) in listOfFontNamesToOutOfOrderGlyphs:
		while (True):
			foundMatchingPairs = False
			for glyphDelegItA in itOOOGlyphList:
				for glyphDelegItB in itOOOGlyphList:
					if (glyphDelegItA[1] == glyphDelegItB[0] and  itOOOGlyphList.index(glyphDelegItA) < itOOOGlyphList.index(glyphDelegItB)):
						# swap
						itamA, itamB = itOOOGlyphList.index(glyphDelegItA), itOOOGlyphList.index(glyphDelegItB)
						itOOOGlyphList[itamB], itOOOGlyphList[itamA] = itOOOGlyphList[itamA], itOOOGlyphList[itamB]
						foundMatchingPairs = True
						break
				if (foundMatchingPairs == True):
					break
			if(foundMatchingPairs == False):
				break # the whole while loop
        arrangedListOfFontNamesToOutOfOrderGlyphs.append( ( itFontName, itOOOGlyphList))
	if traceModeEnabled:
		print "Info:: Arranged Glyphs Delegates List: " , arrangedListOfFontNamesToOutOfOrderGlyphs
	return

#
# Fill the actorPropertyEntries table
def initActorPropertyEntries(thePathToActorNamesTxt):
	global actorPropertyEntriesWasInit
	global actorPropertyEntries
	firstLine = True
#	 print "opening actornames"
	if thePathToActorNamesTxt is None or not thePathToActorNamesTxt:

		actorNamesTextFile = u'actornames.txt'
		relPath = u'.'
		thePathToActorNamesTxt = os.path.join(relPath, actorNamesTextFile)
		print "Warning:: Actor names text file %s not found in arguments. Attempting to open local file if it exists" % (thePathToActorNamesTxt)
	with open(thePathToActorNamesTxt) as tsv:
		for line in csv.reader(tsv, dialect="excel-tab"):
			#skip first line header
			if firstLine == True:
#				 print "skippingHeader"
				firstLine = False
			else:
				actorPropertyEntries.append(line)
	actorPropertyEntriesWasInit = True
	tsv.close()

def getActorShortNameById(lookupActorId):
	global actorPropertyEntriesWasInit
	global actorPropertyEntries
	if not actorPropertyEntriesWasInit:
		return ''
	else:
		for actorEntryTmp in actorPropertyEntries:
			if int(actorEntryTmp[0]) == int(lookupActorId):
				return actorEntryTmp[1]
	return ''


def getActorFullNameById(lookupActorId):
	global actorPropertyEntriesWasInit
	global actorPropertyEntries
	if not actorPropertyEntriesWasInit:
		return ''
	else:
		for actorEntryTmp in actorPropertyEntries:
			if int(actorEntryTmp[0]) == int(lookupActorId):
				return actorEntryTmp[2]
	return ''

def getActorIdByShortName(lookupActorShortName):
	global actorPropertyEntriesWasInit
	global actorPropertyEntries
	if not actorPropertyEntriesWasInit:
		return ''
	else:
		for actorEntryTmp in actorPropertyEntries:
			if actorEntryTmp[1] == lookupActorShortName:
				return actorEntryTmp[0].zfill(2)
	return ''

#
#
# FOR MIX FILE
#

# strFileName should be the full file name (including extension)
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
	if traceModeEnabled:
		print (strParam +': '  +''.join('{:08X}'.format(hash)))
	return hash

#
# aux - sort by first object in list of tuples
def getSortMixFilesKey(item):
	keyTmp = item[0] & 0xFFFFFFFF

	signedKeyTmp = ctypes.c_long(keyTmp).value
	return signedKeyTmp
#
def outputMIX():
	# output file should be defaultSubtitlesMixOutputName
	# checking with known hashes to verify calculateFoldHash
	#calculateFoldHash('AR01-MIN.SET')
	#calculateFoldHash('AR02-MIN.SET')
	#calculateFoldHash('CLOVDIES.AUD')
	#calculateFoldHash('INTRO.VQA')

	errorFound = False
	outMIXFile = None
	try:
		relPath = u'.'
		outputMixPath = os.path.join(relPath, defaultSubtitlesMixOutputName)
		outMIXFile = open(outputMixPath, 'wb')
	except:
		errorFound = True
	if not errorFound:
		# Write header
		# 2 bytes: number of entries (NumFiles)
		# TODO 4 bytes: size of data segment
		# 12 * NumFiles bytes: Entry descriptors table
		#			4 bytes: ID (hash)
		#			4 bytes: Byte offset in Data Segment
		#			4 bytes: Byte length of entry data
		# TODO *Data Segment* - contains the file data. Offset from Entry Descriptors does not include header segment byte length.
		#												Note that the offsets are relative to the start of the body so to find the
		#												actual offset in the MIX you have to add the size of the header which is
		#												(6 + (12 * NumFiles))

		#
		# ID column should in ascending order in MIX FILES (the engine uses binary sort to search for files)
		# so order the files based on ID hash
		# Create a list of 3-item tuples, first item is id, second item is filename
		# Then sort the list
		# Then write to entry table
		#
		# Also filenames should be 8 characters at most and 4 more for extension to conform with specs
		#	^^ this is done manually by making sure the filenames in the sheets of the excel as compliant
		# Based on observations from STARTUP.MIX:
		# 1) the hash ids can overflow and so lower numbers seem to appear down in the index table entries list
		#		 -- So we sort hash but we first tranlste the unsigned key to signed with ctypes
		# 2) the offsets are not necessarily sorted, meaning that the first entry in the index table won't necessarily have the 0x00000000 offset
		i = 0
		mixFileEntries = []
		totalFilesDataSize = 0
		currOffsetForDataSegment = 0 # we start after header and table of index entries, from 0, (but this means that when reading the offset we need to add 6 + numOfFiles * 12). This does not concern us though.
		for sheetDialogueName in supportedDialogueSheets:
			sheetDialogueNameTRE =	sheetDialogueName[:-4] + '.TRE'
			if os.path.isfile('./' + sheetDialogueNameTRE):
				entryID = calculateFoldHash(sheetDialogueNameTRE)
				mixEntryfileSizeBytes = os.path.getsize('./' + sheetDialogueNameTRE)
				mixFileEntries.append((entryID, sheetDialogueNameTRE, mixEntryfileSizeBytes))
				totalFilesDataSize += mixEntryfileSizeBytes

		for translatedTREFileName in [ x[0] for x in supportedTranslationSheets] :
			if os.path.isfile('./' + translatedTREFileName):
				entryID = calculateFoldHash(translatedTREFileName)
				mixEntryfileSizeBytes = os.path.getsize('./' + translatedTREFileName)
				mixFileEntries.append((entryID, translatedTREFileName, mixEntryfileSizeBytes))
				totalFilesDataSize += mixEntryfileSizeBytes

		for otherFileName in supportedOtherFilesForMix:
			if os.path.isfile('./' + otherFileName):
				entryID = calculateFoldHash(otherFileName)
				mixEntryfileSizeBytes = os.path.getsize('./' + otherFileName)
				mixFileEntries.append((entryID, otherFileName, mixEntryfileSizeBytes))
				totalFilesDataSize += mixEntryfileSizeBytes
		mixFileEntries.sort(key=getSortMixFilesKey)
		#
		# We write num of files here. After we verified they exist
		#
		numOfFiles = len(mixFileEntries)
		numOfFilesToWrite = pack('h',numOfFiles)  # short 2 bytes
		outMIXFile.write(numOfFilesToWrite)

		# This is just the data segment (after the entries index table). Adds up all the file sizes here
		totalFilesDataSizeToWrite = pack('I',totalFilesDataSize)  # unsigned integer 4 bytes
		outMIXFile.write(totalFilesDataSizeToWrite)

		if traceModeEnabled:
			print ("Debug:: Sorted Entries based on EntryId")
		for mixFileEntry in mixFileEntries:
			if traceModeEnabled:
				print (''.join('{:08X}'.format(mixFileEntry[0])) + ': ' + mixFileEntry[1] + ' : ' + ''.join('{:08X}'.format(mixFileEntry[2])))
			entryID = mixFileEntry[0] & 0xFFFFFFFF
			entryIDToWrite = pack('I',entryID)	# unsigned integer 4 bytes
			outMIXFile.write(entryIDToWrite)
			entryOffset = currOffsetForDataSegment		# offsets have base after header and table of index entries
			entryOffsetToWrite = pack('I',entryOffset)	# unsigned integer 4 bytes
			outMIXFile.write(entryOffsetToWrite)
			entryByteLength =  mixFileEntry[2]	# File size
			entryByteLengthToWrite = pack('I',entryByteLength)	# unsigned integer 4 bytes
			outMIXFile.write(entryByteLengthToWrite)
			currOffsetForDataSegment += entryByteLength
		# Add data segments here
		errorReadingFound = False
		for mixFileEntry in mixFileEntries:
			try:
				inEntryMIXFile = open("./"+ mixFileEntry[1], 'rb')
			except:
				errorReadingFound = True
			if not errorReadingFound:
				outMIXFile.write(inEntryMIXFile.read())
				inEntryMIXFile.close()
			else:
				print ("Error while reading in ENTRY file")
				break

		outMIXFile.close()
		print "Info:: Total Resource files packed in %s: %d" % (defaultSubtitlesMixOutputName, numOfFiles)
	return
#
# END FOR MIX FILE
#
#

#def inputXLS(filename)
	#TODO extra pass once the quotes have been updated for weird unicode characters
	#TODO some ' quotes appear as \u2019 and others appear normally as '. what's that about?
	#DONE manually I've replaced all weird \u2019 single quotes with '''
	#the spanish n is \xf1 -> we put it at ascii value: \xA5 -- font index 0xA6 ?
	#the spanish i is \xed -> we put it at ascii value: \xA2 -- font index 0xA3 ?
	#pâté
	#	 a actual ascii value is 0xE2 in codepage 1252 -- put it in ascii value 0xA6 (165) -- font index 0xA7
	#	 e actual ascii value is 0xE9 in codepage 1252 -- put it in ascii value 0xA7 (166) -- font index 0xA8
	#TODO what are other characters are special?
	#TODO transition to ASCII chars to store in TRE file?
	#DONE manually I've replaced all one-char '...' with three dots
	# TODO actors TRE has 0x49 entries, (73 names), but table of ids has 73 entries BUT the offset table (first offset is calced + 0x04, so from end of the first 4 count bytes) has 74 entries. The last entry indexes the end of file (!)
	# TODO all strings are NULL terminated in the TRE file!

def translateQuoteToAsciiProper(cellObj, pSheetName):
	newQuoteReplaceSpecials =  cellObj.value.encode("utf-8")
	#print ('Encoded to unicode: %s ' % (newQuoteReplaceSpecials))
	newQuoteReplaceSpecials = newQuoteReplaceSpecials.decode("utf-8")

	pertinentListOfOutOfOrderGlyphs = []
	#print pSheetName
	#print supportedDialogueSheets
	#print defaultSubtitlesFontName[:-4]
	#print [x[0] for x in listOfFontNamesToOutOfOrderGlyphs]
	if pSheetName in supportedDialogueSheets and defaultSubtitlesFontName[:-4] in [x[0] for x in listOfFontNamesToOutOfOrderGlyphs]:
		for (tmpFontName, tmpOOOList) in listOfFontNamesToOutOfOrderGlyphs:
			if tmpFontName == defaultSubtitlesFontName[:-4]:
				pertinentListOfOutOfOrderGlyphs = tmpOOOList
				break
	elif pSheetName in [x[0] for x in supportedTranslationSheets]:
		pertinentFontType = ''
        #[treAndFontTypeTuple for treAndFontTypeTuple in supportedTranslationSheets if treAndFontTypeTuple[0] == pSheetName]
		for (tmpSheetName, tmpFontType) in supportedTranslationSheets:
			if tmpSheetName == pSheetName:
				pertinentFontType = tmpFontType
				break
		for (tmpFontName, tmpOOOList) in listOfFontNamesToOutOfOrderGlyphs:
			if tmpFontName ==  pertinentFontType:
				pertinentListOfOutOfOrderGlyphs = tmpOOOList
				break

	#newQuoteReplaceSpecials = newQuoteReplaceSpecials.replace(u"\u0386", u"\u00A3")
	for repTuple in pertinentListOfOutOfOrderGlyphs:
		newQuoteReplaceSpecials = newQuoteReplaceSpecials.replace(repTuple[0], repTuple[1])
	# WORKAROUND, we re-replace the spanish i delegate again here!
#	newQuoteReplaceSpecials = newQuoteReplaceSpecials.replace(u'\xa2', u'\u0386')   # this is needed for spanish i because in utf-8 it's actually the u'\u0386' that's assigned to A tonomeno which is the delegate.
#	newQuoteReplaceSpecials = newQuoteReplaceSpecials.replace(u"\u0386", u"\u00A3")
#	#newQuoteReplaceSpecials = newQuoteReplaceSpecials.replace(u"\u0386", u"\u00A3")	 # greek alpha tonomeno -- TODO which character is this in the excel (utf value) ???
#	newQuoteReplaceSpecials = newQuoteReplaceSpecials.replace(u"\u00ed", u"\u00A2")	 # spanish i
#	newQuoteReplaceSpecials = newQuoteReplaceSpecials.replace(u"\u00f1", u"\u00A5")	 # spanish n
#	#newQuoteReplaceSpecials = newQuoteReplaceSpecials.replace(u"\u00A4", u"\u00A5")  # spanish n
#	newQuoteReplaceSpecials = newQuoteReplaceSpecials.replace(u"\u00e2", u"\u00A6")	 # a from pate -- todo this is not confirmed in-game font (but it is in our external font as of	 yet)
#	newQuoteReplaceSpecials = newQuoteReplaceSpecials.replace(u"\u00e9", u"\u00A7")	 # e from pate -- todo this is not confirmed in-game font (but it is in our external font as of	 yet)
	# other replacements.
	newQuoteReplaceSpecials = newQuoteReplaceSpecials.replace(u"\u2019", u"\u0027")	 # right single quote
	newQuoteReplaceSpecials = newQuoteReplaceSpecials.replace(u"\u2018", u"\u0027")	 # left single quote
	newQuoteReplaceSpecials = newQuoteReplaceSpecials.replace(u"\u2026", u"\u002e\u002e\u002e")	 # three dots together (changes length)
	newQuoteReplaceSpecials = newQuoteReplaceSpecials.replace(u"\u201D", u"\u0022")	 # right double quote
	newQuoteReplaceSpecials = newQuoteReplaceSpecials.replace(u"\u201C", u"\u0022")	 # left double quote
	# TODO? replace new line ???	with another char (maybe |)?

	#newQuoteReplaceSpecialsUnicode = unicode(newQuoteReplaceSpecials, 'utf-8')
	#newQuoteReplaceSpecialsStr = unicode.encode("%s" % newQuoteReplaceSpecials, targetEncoding)

	#print type(newQuoteReplaceSpecials)                 # type is unicode
	#print type(newQuoteReplaceSpecials.encode('utf-8')) # type is str
#	print targetEncoding
#	print newQuoteReplaceSpecials
#	newQuoteReplaceSpecialsDec = newQuoteReplaceSpecials.decode(targetEncoding)
	newQuoteReplaceSpecialsRetStr = ''
	newQuoteReplaceSpecialsRetStr = newQuoteReplaceSpecials.encode(targetEncoding)
#	try:
#		newQuoteReplaceSpecialsRetStr = newQuoteReplaceSpecials.encode(targetEncoding)
#	except:
#		print "==============================================================================="
#		print "==============================================================================="
#		print "ERROR:"
#		print newQuoteReplaceSpecials
#		print newQuoteReplaceSpecials.encode(targetEncoding, errors='xmlcharrefreplace')
#		print "==============================================================================="
#		print "==============================================================================="
#		newQuoteReplaceSpecialsRetStr = newQuoteReplaceSpecials.encode(targetEncoding, errors='xmlcharrefreplace')
	return newQuoteReplaceSpecialsRetStr
#	return newQuoteReplaceSpecialsEnStr


def inputXLS(filename):
	global numOfSpokenQuotes
	global tableOfStringIds
	global tableOfStringOffsets
	global tableOfStringEntries
	# Open the workbook
	xl_workbook = xlrd.open_workbook(filename, encoding_override="utf-8")


	# List sheet names, and pull a sheet by name
	#
	# sheet_names = xl_workbook.sheet_names()
	#print('Sheet Names', sheet_names)
	#
	#xl_sheet = xl_workbook.sheet_by_name(sheet_names[0])

	# Or grab the first sheet by index
	#  (sheets are zero-indexed)
	# First sheet is the in-game quotes
	#
	# xl_sheet = xl_workbook.sheet_by_index(0)
	#
	#
	mergedListOfSubtitleSheetsAndTranslatedTREs = supportedDialogueSheets + [ x[0] for x in supportedTranslationSheets ]

	for sheetDialogueName in mergedListOfSubtitleSheetsAndTranslatedTREs:
		xl_sheet = xl_workbook.sheet_by_name(sheetDialogueName)
		if(xl_sheet is not None):
			if traceModeEnabled:
				print ('Sheet name: %s' % xl_sheet.name)
			numOfSpokenQuotes = xl_sheet.nrows - 2 # all rows minus the first TWO rows with headers
			if traceModeEnabled:
				print ('Debug:: num of spoken quotes: %d' % numOfSpokenQuotes)
			# stats for debug
			extremeQuotesList = []
			longestLength = 0
			predefinedLengthThreshold = 145
			quoteNumAboveThreshold = 0
			# end of stats for debug


			absStartOfIndexTable = 4
			absStartOfOffsetTable = absStartOfIndexTable + (numOfSpokenQuotes * 4)		# = 4 + 0x1577 * 4 = 4 + 0x55DC = 0x55E0
			absStartOfStringTable = absStartOfOffsetTable + ((numOfSpokenQuotes+1) * 4) # =	 0x55E0 + (0x1578 * 4) = 0xABC0
			curStrStartOffset = absStartOfStringTable - 4
			newQuoteReplaceSpecialsAscii = ''
			tmpQuoteID = 0
			#switchFlagShowQuote = False # for debugging
			tmpStartFrame = 0			# for VQA sheets
			tmpEndFrame = 0				# for VQA sheets
			mode = 0					# init to unknown
			if xl_sheet.name == supportedDialogueSheets[0]:
				if traceModeEnabled:
					print 'IN GAME QUOTES'
				mode = 1 #in-game quote
			elif xl_sheet.name in supportedDialogueSheets:
				if traceModeEnabled:
					print 'VQA SCENE DIALOGUE'
				mode = 2 #VQA
			elif xl_sheet.name in [ x[0] for x in supportedTranslationSheets ]:
				if traceModeEnabled:
					print 'TRANSLATED TRE'
				mode = 3 # Translated TRE
			#
			del tableOfStringIds[:]
			del tableOfStringEntries[:]
			del tableOfStringOffsets[:]
			for row_idx in range(2, xl_sheet.nrows):
				#print "Line %d" % (row_idx)
				for col_idx in range(0, xl_sheet.ncols):
					cell_obj = xl_sheet.cell(row_idx, col_idx)
					#
					# FOR IN-GAME QUOTES -- Iterate through columns starting from col 0. We need cols: 0, 2
					#
					if mode == 1:
					   #print ('Column: [%s] cell_obj: [%s]' % (col_idx, cell_obj))
						if(col_idx == 0):
							#switchFlagShowQuote = False
							twoTokensfirstColSplitAtDotXLS = cell_obj.value.split('.', 1)
							if len(twoTokensfirstColSplitAtDotXLS) == 2:
								twoTokensfirstColSplitAtDashXLS = twoTokensfirstColSplitAtDotXLS[0].split('-', 1)
								if len(twoTokensfirstColSplitAtDashXLS) == 2:
									tmpQuoteID = int( twoTokensfirstColSplitAtDashXLS[0]) * 10000 + int(twoTokensfirstColSplitAtDashXLS[1])
									#print ('row_idx %d. tag %s = quoteId [%d]' % (row_idx, twoTokensfirstColSplitAtDotXLS[0], tmpQuoteID))
									tableOfStringIds.append(tmpQuoteID)
									#if(tmpQuoteID == 160110 or tmpQuoteID == 160010):
									#	 switchFlagShowQuote = True

						elif(col_idx == 1) :
							#if switchFlagShowQuote == True:
							#	 print ('length: %d: %s' % (len(cell_obj.value), cell_obj.value))
							#	 print ('object: %s' % (cell_obj))
							#	 #newQuoteReplaceSpecials =	 cell_obj.value.decode("utf-8") # unicode(cell_obj.value, 'windows-1252')
							#	 #print ('decoded to unicode: %s ' % (newQuoteReplaceSpecials)) # error with char xf1
							newQuoteReplaceSpecialsAscii = translateQuoteToAsciiProper(cell_obj, xl_sheet.name)
							#if switchFlagShowQuote == True:
							#	 print ('length: %d: %s' % (len(newQuoteReplaceSpecialsAscii), newQuoteReplaceSpecialsAscii))
							#print ':'.join(x.encode('hex') for x in newQuoteReplaceSpecialsAscii)	 # seems to work.  new chars are non-printable but exist in string

							tableOfStringEntries.append(newQuoteReplaceSpecialsAscii)
							tableOfStringOffsets.append(curStrStartOffset)
							curStrStartOffset += (len(newQuoteReplaceSpecialsAscii) + 1)
							if ( longestLength < len(newQuoteReplaceSpecialsAscii)):
								longestLength = len(newQuoteReplaceSpecialsAscii)
							if ( predefinedLengthThreshold < len(newQuoteReplaceSpecialsAscii)):
								extremeQuotesList.append((tmpQuoteID, newQuoteReplaceSpecialsAscii))
								quoteNumAboveThreshold += 1
								#print ('row_idx %d. tag %s = quoteId [%d], length: %d: %s' % (row_idx, twoTokensfirstColSplitAtDotXLS[0], tmpQuoteID, len(newQuoteReplaceSpecialsAscii), newQuoteReplaceSpecialsAscii))
					#
					# FOR VQAs -- Iterate through columns starting from col 2. We need cols: 2, 9, 10
					#
					elif mode == 2:
						if(col_idx == 2): # subtitle text
							newQuoteReplaceSpecialsAscii = translateQuoteToAsciiProper(cell_obj, xl_sheet.name)
							#print ('length: %d: %s' % (len(newQuoteReplaceSpecialsAscii), newQuoteReplaceSpecialsAscii))
							#print ':'.join(x.encode('hex') for x in newQuoteReplaceSpecialsAscii)	# seems to work.  new chars are non-printable but exist in string
							# don't append to tableOfStringEntries yet
						elif(col_idx == 9): # startFrame
							#print "cell: %s" % (cell_obj.value)
							tmpStartFrame =	 int(cell_obj.value)
						elif(col_idx == 10): # endFrame
							tmpEndFrame = int(cell_obj.value)
							tmpQuoteID = tmpStartFrame | (tmpEndFrame << 16) # top 16 bits are end frame (up to 65536 frames which is enough) and low 16 bits are startFrame

							tableOfStringIds.append(tmpQuoteID)
							tableOfStringEntries.append(newQuoteReplaceSpecialsAscii)
							tableOfStringOffsets.append(curStrStartOffset)
							curStrStartOffset += (len(newQuoteReplaceSpecialsAscii) + 1)
							if ( longestLength < len(newQuoteReplaceSpecialsAscii)):
								longestLength = len(newQuoteReplaceSpecialsAscii)
							if ( predefinedLengthThreshold < len(newQuoteReplaceSpecialsAscii)):
								extremeQuotesList.append((tmpQuoteID, newQuoteReplaceSpecialsAscii))
								quoteNumAboveThreshold += 1
					#
					# For translated TRE sheets the id is already in first column, the text is in the next one
					#
					elif mode == 3:
					   #print ('Column: [%s] cell_obj: [%s]' % (col_idx, cell_obj))
						if(col_idx == 0):
							tmpQuoteID = int(cell_obj.value)
							tableOfStringIds.append(tmpQuoteID)
						elif(col_idx == 1) :
							#if switchFlagShowQuote == True:
							#	 print ('length: %d: %s' % (len(cell_obj.value), cell_obj.value))
							#	 print ('object: %s' % (cell_obj))
							#	 #newQuoteReplaceSpecials =	 cell_obj.value.decode("utf-8") # unicode(cell_obj.value, 'windows-1252')
							#	 #print ('decoded to unicode: %s ' % (newQuoteReplaceSpecials)) # error with char xf1
							newQuoteReplaceSpecialsAscii = translateQuoteToAsciiProper(cell_obj, xl_sheet.name)
							#if switchFlagShowQuote == True:
							#	 print ('length: %d: %s' % (len(newQuoteReplaceSpecialsAscii), newQuoteReplaceSpecialsAscii))
							#print ':'.join(x.encode('hex') for x in newQuoteReplaceSpecialsAscii)	 # seems to work.  new chars are non-printable but exist in string

							tableOfStringEntries.append(newQuoteReplaceSpecialsAscii)
							tableOfStringOffsets.append(curStrStartOffset)
							curStrStartOffset += (len(newQuoteReplaceSpecialsAscii) + 1)
							if ( longestLength < len(newQuoteReplaceSpecialsAscii)):
								longestLength = len(newQuoteReplaceSpecialsAscii)
							if ( predefinedLengthThreshold < len(newQuoteReplaceSpecialsAscii)):
								extremeQuotesList.append((tmpQuoteID, newQuoteReplaceSpecialsAscii))
								quoteNumAboveThreshold += 1
								#print ('row_idx %d. tag %s = quoteId [%d], length: %d: %s' % (row_idx, twoTokensfirstColSplitAtDotXLS[0], tmpQuoteID, len(newQuoteReplaceSpecialsAscii), newQuoteReplaceSpecialsAscii))

			tableOfStringOffsets.append(curStrStartOffset) # the final extra offset entry
			if traceModeEnabled:
				print 'Debug:: Longest Length = %d, quotes above threshold (%d): %d' % (longestLength, predefinedLengthThreshold, quoteNumAboveThreshold)
				for extremQuotTuple in extremeQuotesList:
					print "Id: %d, Q: %s" % (extremQuotTuple[0], extremQuotTuple[1])
			#
			# WRITE TO TRE FILE
			#
			errorFound = False
			outTREFile = None
			outTREFileName = sheetDialogueName[:-4]
			try:
				outTREFile = open("./" + outTREFileName + ".TRE", 'wb')
			except:
				errorFound = True
			if not errorFound:
				numOfSpokenQuotesToWrite = pack('I',numOfSpokenQuotes)	# unsigned integer 4 bytes
				outTREFile.write(numOfSpokenQuotesToWrite)
				# write string IDs table
				for idxe in range(0,len(tableOfStringIds)):
					idOfStringToWrite = pack('I',tableOfStringIds[idxe])  # unsigned integer 4 bytes
					outTREFile.write(idOfStringToWrite)
				# write string offsets table
				for idxe in range(0,len(tableOfStringOffsets)):
					offsetOfStringToWrite = pack('I',tableOfStringOffsets[idxe])  # unsigned integer 4 bytes
					outTREFile.write(offsetOfStringToWrite)
				#write strings with null terminator
				for idxe in range(0,len(tableOfStringEntries)):
					outTREFile.write(tableOfStringEntries[idxe])
					outTREFile.write('\0')
				outTREFile.close()
	return
#
#
#
# ########################
# main
# 00_0000 -- DealsInInsects					dupl TLK01, TLK0A
# 00_0510 -- ThinkingOfChangingJobs-Leon	dupl TLK02, TLK03
# 00-8520 -- WhatDoYouKnow					dupl TLK01, TLK0A
def main(argsCL):
	# TODO parse arguments using argparse? https://docs.python.org/3/library/argparse.html#module-argparse
	global traceModeEnabled
	traceModeEnabled = False
	pathToQuoteExcelFile = ""
	pathToActorNamesTxt = ""
	pathToConfigureFontsTranslationTxt = ""

	invalidSyntax = False
	print "Running %s (%s)..." % (app_name_spaced, app_version)
#	 print "Len of sysargv = %s" % (len(argsCL))
	if len(argsCL) == 2:
		if(argsCL[1] == '--help'or argsCL[1] == '-h'):
			print "%s %s supports Blade Runner (English version, CD edition)." % (app_name_spaced, app_version)
			print app_short_desc
			print "Created by Praetorian of the classic adventures in Greek team."
			print "Always keep backups!"
			print "--------------------"
			print "Preparatory steps:"
			print "1. Copy the BladeRunnerPCTLK.xls file (latest version, downloaded from Google Sheets) in some folder on your PC."
			print "--------------------"
			print "%s takes 1 mandatory argument:" % (app_wrapper_name)
			print "Valid syntax: "
			print "%s -x path_to_BladeRunnerPCTLK_xls [-ian path_to_actornames_txt] [-cft path_to_configureFontsTranslation_txt] [--trace]" % (app_wrapper_name)
			print "-x is followed by the path to the excel file with the subtitle quotes."
			print "-ian is followed by the path to actornames.txt, if it's not in the current working directory."
			print "-cft is followed by the path to configureFontsTranslation.txt, if it's not in the current working directory."
			print "The --trace switch enables more debug messages being printed during execution."
			print "If the app finishes successfully a " + supportedDialogueSheets[0] + " and a few other .TRE files for the VQAs "
			print "in the Excel file as well as a %s file containing all of them will be created in the same folder with the app." % (defaultSubtitlesMixOutputName)
			print "--------------------"
			print "Thank you for using this app."
			print "Please provide any feedback to: %s " % (company_email)
			sys.exit()
		elif(argsCL[1] == '--version' or argsCL[1] == '-v'):
			print "%s %s supports Blade Runner (English version, CD edition)." % (app_name_spaced, app_version)
			print "Please provide any feedback to: %s " % (company_email)
			sys.exit()
		else:
			invalidSyntax = True
	elif len(argsCL) > 2:
		for i in range(1, len(argsCL)):
			if( i < (len(argsCL) - 1) and argsCL[i][:1] == '-' and argsCL[i+1][:1] != '-'):
				if (argsCL[i] == '-x'):
					pathToQuoteExcelFile = argsCL[i+1]
				elif (argsCL[i] == '-ian'):
					pathToActorNamesTxt = argsCL[i+1]
				elif (argsCL[i] == '-cft'):
					pathToConfigureFontsTranslationTxt = argsCL[i+1]
			elif sys.argv[i] == '--trace':
				print "Info:: Trace mode enabled (more debug messages)."
				traceModeEnabled = True

		if not pathToQuoteExcelFile:
			invalidSyntax = True

		if not invalidSyntax:
			# parse any overrideEncoding file if exists:
			initOverrideEncoding(pathToConfigureFontsTranslationTxt)


			# parse the EXCEL File
			# parse Actors files:
			initActorPropertyEntries(pathToActorNamesTxt)
#			 for actorEntryTmp in actorPropertyEntries:
#				  print "Found actor: %s %s %s" % (actorEntryTmp[0], actorEntryTmp[1], actorEntryTmp[2])
			inputXLS(pathToQuoteExcelFile)
			outputMIX()

	else:
		invalidSyntax = True

	if invalidSyntax == True:
		print "Invalid syntax\n Try: \n %s --help for more info \n %s --version for version info " % (app_wrapper_name, app_wrapper_name)
		print "Valid syntax: "
		print "%s -x path_to_BladeRunnerPCTLK_xls [-ian path_to_actornames_txt] [-cft path_to_configureFontsTranslation_txt] [--trace]" % (app_wrapper_name)
		print "-x is followed by the path to the excel file with the subtitle quotes."
		print "-ian is followed by the path to actornames.txt, if it's not in the current working directory."
		print "-cft is followed by the path to configureFontsTranslation.txt, if it's not in the current working directory."
		print "The --trace switch enabled more debug messages being printed during execution."
		print "If the app finishes successfully a " + supportedDialogueSheets[0] + " and a few other .TRE files for the VQAs "
		print "in the Excel file as well as a %s file containing all of them will be created in the same folder with the app." % (defaultSubtitlesMixOutputName)
		tmpi = 0
		for tmpArg in argsCL:
			if tmpi==0: #skip first argument
				tmpi+=1
				continue
			print "\nArgument: %s" % (tmpArg)
			tmpi+=1

# Total unique quotes seems to be 5495!
# TODO rename files in folders to conform to the underscore '_' and '-' format (a few don't -- let's have them all conforming!)
# #########################
#
if __name__ == "__main__":
	main(sys.argv[0:])
else:
	## debug
	#print 'Debug:: %s was imported from another module' % (app_wrapper_name)
	pass
