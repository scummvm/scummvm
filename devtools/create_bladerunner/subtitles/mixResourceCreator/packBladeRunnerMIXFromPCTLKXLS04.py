#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Created by Praetorian (ShadowNate) for Classic Adventures in Greek
# classic.adventures.in.greek@gmail.com
# Works with Excel version outSpeech-15-06-2018-1856-TranslatingComms-080.xls and above
#
# TODO Print a warning if packing a Text Resource (TRx) without the corresponding font(s) -- only a warning though
#
# DONE - Support at least one translation too (ie Greek)
#
osLibFound = False
sysLibFound = False
shutilLibFound = False
ctypesLibFound = False
csvLibFound = False
xlrdLibFound = False
xlwtLibFound = False
xlutilsLibFound = False
reLibFound = False
structLibFound = False
datetimeLibFound = False

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
	import shutil
except ImportError:
	print "[Error] Shutil python library is required to be installed!"
else:
	shutilLibFound = True

try:
	import ctypes
except ImportError:
	print "[Error] ctypes python library is required to be installed!"
else:
	ctypesLibFound = True

try:
	from datetime import datetime
except ImportError:
	print "[Error] datetime python library is required to be installed!"
else:
	datetimeLibFound = True

try:
	import csv
except ImportError:
	print "[Error] csv python library is required to be installed!"
else:
	csvLibFound = True

try:
	import xlrd
except ImportError:
	print "[Error] xlrd python library is required to be installed!"
else:
	xlrdLibFound = True

try:
	import xlwt
except ImportError:
	print "[Error] xlwt python library is required to be installed!"
else:
	xlwtLibFound = True

try:
	from xlutils.copy import copy
except ImportError:
	print "[Error] xlutils python library is required to be installed!"
else:
	xlutilsLibFound = True

try:
	import re
except ImportError:
	print "[Error] re (Regular expression operations) python library is required to be installed!"
else:
	reLibFound = True

try:
	import struct
except ImportError:
	print "[Error] struct python library is required to be installed!"
else:
	structLibFound = True
	


if 	(not osLibFound) \
	or (not sysLibFound) \
	or (not shutilLibFound) \
	or (not ctypesLibFound) \
	or (not datetimeLibFound) \
	or (not csvLibFound) \
	or (not xlrdLibFound) \
	or (not xlwtLibFound) \
	or (not xlutilsLibFound) \
	or (not reLibFound) \
	or (not structLibFound):
	sys.stdout.write("[Error] Errors were found when trying to import required python libraries\n")
	sys.exit(1)

pathToParent = os.path.join(os.path.dirname(os.path.abspath(__file__)), os.pardir)
pathToCommon = os.path.join(pathToParent, "common")
sys.path.append(pathToCommon)

from os import path
from xlrd import *
# for pack
from struct import *
from subtlsVersTextResource import *

COMPANY_EMAIL = "classic.adventures.in.greek@gmail.com"
APP_VERSION = "1.70"
APP_NAME = "packBladeRunnerMIXFromPCTLKXLS"
APP_WRAPPER_NAME = "mixResourceCreator.py"
APP_NAME_SPACED = "Blade Runner MIX Resource Creator"
APP_SHORT_DESC = "Make a Text Resource file for spoken in-game quotes and pack Text Resources with Fonts into a SUBTITLES.MIX file."

WINDOWS_1252_ENCODING = 'windows-1252'

# TODO- maybe the '_E' part is not needed
SUBTITLES_FONT_NAME_CATEGORY = 'SUBTLS_E'
DEFAULT_SUBTITLES_FONT_NAME = SUBTITLES_FONT_NAME_CATEGORY + '.FON'
DEFAULT_SUBTITLES_MIX_OUTPUT_NAME = u'SUBTITLES.MIX'

SUPPORTED_DIALOGUE_VERSION_SHEET = 'SBTLVERS.TRE'
# all dialogue sheets get the SUBTLS_E.FON for translation to a Text Resource (TRx)
# In-game dialogue sheet
# TODO- maybe the '_E' part is not needed, since we use the suffix (x) of the extension (TRx) to signify the language
SUPPORTED_INGAME_DIALOGUE_SHEETS = ['INGQUO_']
# Video cut-scenes' dialogue sheets - these need the appendix of (x) for the language code, and a suffix of '.VQA'.
# These two first videos seem to be in _E (english) language across translations
SUPPORTED_VIDEO_DIALOGUE_SHEETS_ENGLISH = ['WSTLGO_', 'BRLOGO_']
SUPPORTED_VIDEO_DIALOGUE_SHEETS_LOCALIZED = ['INTRO_', 'MW_A_', 'MW_B01_', 'MW_B02_', 'MW_B03_', 'MW_B04_', 'MW_B05_', 'INTRGT_', 'MW_D_', 'MW_C01_', 'MW_C02_', 'MW_C03_', 'END04A_', 'END04B_', 'END04C_', 'END06_', 'END01A_', 'END01B_', 'END01C_', 'END01D_', 'END01E_', 'END01F_', 'END03_', 'TB_FLY_']
#
# Each Text Resource (TRx) sheet gets a specific font to handle their translation to Text Resource
# TAHOMA means both TAHOMA Fonts (18 and 24)(their translation should be identical (although in the original they have minor differences but they don't affect anything)
# We use a single naming for TAHOMA here because both TAHOMA18 and TAHOMA24 are used for ENDCRED.TRx
# The TRx files that are identically named to the originals are supposed to override them (needs ScummVM compatible functionality for that)
# This is so that fan made translations are supported.
# V:1.00 - Added 'SBTLVERS.TR' here to provide info for the versioning and compilation date of the Subtitles.MIX file
SUPPORTED_TRANSLATION_SHEETS = [('OPTIONS.TR', 'KIA6PT'),
								('DLGMENU.TR', 'KIA6PT'),
								('SCORERS.TR', 'TAHOMA'),
								('VK.TR', 'KIA6PT'),
								('CLUES.TR', 'KIA6PT'),
								('CRIMES.TR', 'KIA6PT'),
								('ACTORS.TR', 'KIA6PT'),
								('HELP.TR', 'KIA6PT'),
								('AUTOSAVE.TR', 'KIA6PT'),
								('ERRORMSG.TR', 'SYSTEM'),
								('SPINDEST.TR', 'KIA6PT'),
								('KIA.TR', 'KIA6PT'),
								('KIACRED.TR', 'KIA6PT'),
								('CLUETYPE.TR', 'KIA6PT'),
								('ENDCRED.TR', 'TAHOMA'),
								('POGO.TR', 'KIA6PT'),
								(SUPPORTED_DIALOGUE_VERSION_SHEET[:-1], SUBTITLES_FONT_NAME_CATEGORY)]
# The FON files that are identically named to the originals are supposed to override them (needs ScummVM compatible functionality for that)
# We don't deal with 10PT.FON since it's not used.
# Also we don't deal with the SYSTEM (external OS font) that ERRORMSG.TRx uses!
# TODO we probably could skip importing ERRORMSG.TRx (to SUBTITLES.MIX) altogether, since translating that has no point! In that case SYSTEM.FON should be removed from here since it's currently of no use otherwise and support for it is not really required.
SUPPORTED_OTHER_FILES_FOR_MIX = [DEFAULT_SUBTITLES_FONT_NAME, 'KIA6PT.FON', 'TAHOMA18.FON', 'TAHOMA24.FON', 'SYSTEM.FON']

# v1.10: Russian code (RU_RUS) now corresponds to 'E' suffix instead of 'R' since the unofficial Russian version supported uses the English resources without renaming them, and this is how the ScummVM engine handles that version currently.
SUPPORTED_LANGUAGES_DESCRIPTION_CODE_TLIST = [('EN_ANY', 'E', 'English'), ('DE_DEU', 'G', 'German'), ('FR_FRA', 'F', 'French'), ('IT_ITA', 'I', 'Italian'), ('ES_ESP', 'S', 'Spanish'), ('RU_RUS', 'E', 'Russian'), ('EFIGS', '#', 'EFIGS')]
DEFAULT_LANG_DESC_CODE = SUPPORTED_LANGUAGES_DESCRIPTION_CODE_TLIST[0]

DEFAULT_TARGET_ENCODING_PER_FONT = [(SUBTITLES_FONT_NAME_CATEGORY, WINDOWS_1252_ENCODING), ('KIA6PT', 'cp437'), ('TAHOMA', 'cp437'), ('SYSTEM', 'latin-1')]
gTargetEncodingPerFont = [] # global var

gTraceModeEnabled = False
gActiveLanguageDescriptionCodeTuple = ''
gNumOfSpokenQuotes = 0

gActorPropertyEntries = []
gActorPropertyEntriesWasInit = False

gTableOfStringIds = []
gTableOfStringOffsets = []
gTableOfStringEntries = []

# This list is used in order to replace the actual indices of characters with delegate font indices (ASCII indexes of the target code-page) which have been used during the font creation (or exist in in the internal TAHOMA font)
# contains tuples of two values. First value is actual UTF char, the second is a replacement ASCII char
gListOfFontNamesToOutOfOrderGlyphs = []
gArrangedListOfFontNamesToOutOfOrderGlyphs = []

#
#
#
def initOverrideEncoding(pathToConfigureFontsTranslationTxt):
	global gTargetEncodingPerFont
	global gListOfFontNamesToOutOfOrderGlyphs
	global gArrangedListOfFontNamesToOutOfOrderGlyphs

	if pathToConfigureFontsTranslationTxt is None or not pathToConfigureFontsTranslationTxt:
		configureFontsTranslationTextFile = u'configureFontsTranslation.txt'
		relPath = u'.'
		pathToConfigureFontsTranslationTxt = os.path.join(relPath, configureFontsTranslationTextFile)
		print "[Warning] Font Translation Configuration file not found in arguments. Attempting to open local file %s if it exists" % (configureFontsTranslationTextFile)

	configureTranslationFailed = False
	try:
		if os.access(pathToConfigureFontsTranslationTxt, os.F_OK):
			print "[Info] Font Translation Configuration file found: {0}".format(pathToConfigureFontsTranslationTxt)
			conFontsTranslationFile = open(pathToConfigureFontsTranslationTxt, 'r')
			linesLst = conFontsTranslationFile.readlines()
			conFontsTranslationFile.close()
			if linesLst is None or len(linesLst) == 0:
				print '[Error] Empty configureFontsTranslation text file!'
				configureTranslationFailed = True
			else:
				if gTraceModeEnabled:
					print "[Debug] Parsing Font Translation Configuration file info..."
				involvedTokensLst =[]
				for readEncodLine in linesLst:
					tmplineTokens = re.findall("[^\t\n]+",readEncodLine )
					for x in tmplineTokens:
						involvedTokensLst.append(x)

				for tokenNameKeyPair in involvedTokensLst:
					nameKeyTupl = tokenNameKeyPair.split('=', 1)
					try:
						if len(nameKeyTupl) == 2 and nameKeyTupl[0] == 'fontNameAndOutOfOrderGlyphs' and nameKeyTupl[1] is not None and nameKeyTupl[1] != '':
							# split at hash tag first
							tmpListOfOutOfOrderGlyphs = []
							del(tmpListOfOutOfOrderGlyphs[:])
							fontCateg_targetEnc_OOOGlyphs_Tuple = nameKeyTupl[1].split('#', 2)
							#print nameKeyTupl[1] + ': %d' % (len(fontCateg_targetEnc_OOOGlyphs_Tuple))
							#print fontCateg_targetEnc_OOOGlyphs_Tuple
							if (len(fontCateg_targetEnc_OOOGlyphs_Tuple) == 3 \
									and fontCateg_targetEnc_OOOGlyphs_Tuple[0] != ''\
									and fontCateg_targetEnc_OOOGlyphs_Tuple[1] is not None \
									and fontCateg_targetEnc_OOOGlyphs_Tuple[1] != ''):
								tmpFontCateg = fontCateg_targetEnc_OOOGlyphs_Tuple[0]
								tmpTargetEncodingForThisFont = fontCateg_targetEnc_OOOGlyphs_Tuple[1]
								if ( tmpFontCateg not in zip(*DEFAULT_TARGET_ENCODING_PER_FONT)[0]):
									print '[Error] Invalid Font name specified in configureFontsTranslation text file!'
									print '        Valid values are: ', ", ".join( zip(*DEFAULT_TARGET_ENCODING_PER_FONT)[0] )
									configureTranslationFailed = True
									break

								elif len(gTargetEncodingPerFont) == 0 \
									or (tmpFontCateg not in zip(*gTargetEncodingPerFont)[0]):
									gTargetEncodingPerFont.append(  ( tmpFontCateg,  tmpTargetEncodingForThisFont) )

								if ( fontCateg_targetEnc_OOOGlyphs_Tuple[2] is not None \
									and fontCateg_targetEnc_OOOGlyphs_Tuple[2] != ''):
									# split at comma, then split at ':' and store tuples of character
									explicitOutOfOrderGlyphsTokenUnicode = unicode(fontCateg_targetEnc_OOOGlyphs_Tuple[2], 'utf-8') # unicode(fontCateg_targetEnc_OOOGlyphs_Tuple[2], 'utf-8')
									#explicitOutOfOrderGlyphsTokenStr =  unicode.encode("%s" % explicitOutOfOrderGlyphsTokenUnicode, gTargetEncoding)
									#explicitOutOfOrderGlyphsTokenStr =  explicitOutOfOrderGlyphsTokenUnicode.decode(gTargetEncoding) # unicode.encode("%s" % explicitOutOfOrderGlyphsTokenUnicode, 'utf-8')
									tokensOfOutOfOrderGlyphsStrList = explicitOutOfOrderGlyphsTokenUnicode.split(',')
									for tokenX in tokensOfOutOfOrderGlyphsStrList:
										tokensOfTupleList = tokenX.split(':')
										if len(tokensOfTupleList) == 2:
											tmpListOfOutOfOrderGlyphs.append( (unichr(ord(tokensOfTupleList[0])), unichr(ord(tokensOfTupleList[1])))  )
										else:
											print '[Error] Bad tuple syntax in configureFontsTranslation text file!'
											configureTranslationFailed = True

									if not configureTranslationFailed \
										and tmpFontCateg not in [x[0] for x in gListOfFontNamesToOutOfOrderGlyphs]:
										gListOfFontNamesToOutOfOrderGlyphs.append(  ( tmpFontCateg,  tmpListOfOutOfOrderGlyphs) )
									elif configureTranslationFailed:
										break
							else:
								print '[Error] Bad line syntax in configureFontsTranslation text file!'
								configureTranslationFailed = True
								break
					except:
						configureTranslationFailed = True
						raise
				#
				# end of for loop over configureFontsTranslation's lines
				#
				if (configureTranslationFailed == False):
					for tmpFontToTargetEncCateg in DEFAULT_TARGET_ENCODING_PER_FONT:
						if (len (gTargetEncodingPerFont) == 0 \
							or  tmpFontToTargetEncCateg[0] not in zip(*gTargetEncodingPerFont)[0]):
							# append the defaults for the mappings not explicitly specified in configureFontsTranslation
							gTargetEncodingPerFont.append(tmpFontToTargetEncCateg)

					if len(gTargetEncodingPerFont) != len(DEFAULT_TARGET_ENCODING_PER_FONT):
						# should never happen
						print '[Error] Failed to populate internal target encoding per font structure!'
						configureTranslationFailed = True
					else:
						if gTraceModeEnabled:
							print '[Debug] My encodings list: ', gTargetEncodingPerFont
						configureTranslationFailed = False

	except:
		print "[Error] while trying to access file for Font Translation Configuration info: %s" % (pathToConfigureFontsTranslationTxt)
		raise
		configureTranslationFailed = True

	if 	configureTranslationFailed == True:
		print "[Error] Could not find proper Font Translation Configuration info in: %s" % (pathToConfigureFontsTranslationTxt)
		sys.exit(1)	# terminate if override Failed (Blade Runner)
	#
	#
	#We no longer assume default out of order list
	#if(len(gListOfFontNamesToOutOfOrderGlyphs) == 0):
	#	tmpFontType = DEFAULT_SUBTITLES_FONT_NAME[:-4] # remove the .FON extensionFromTheName
	#	print "[Info] Empty list for out of order glyphs. Assuming default out of order glyphs and only for the %s font" % (tmpFontType)
	#	tmplistOfOutOfOrderGlyphs = []
	#	tmplistOfOutOfOrderGlyphs.append((u'\xed', u'\u0386')) # spanish i (si)
	#	tmplistOfOutOfOrderGlyphs.append((u'\xf1', u'\xa5')) # spanish n (senor)
	#	tmplistOfOutOfOrderGlyphs.append((u'\xe2', u'\xa6')) # a for (liver) pate
	#	tmplistOfOutOfOrderGlyphs.append((u'\xe9', u'\xa7')) # e for (liver) pate
	#	gListOfFontNamesToOutOfOrderGlyphs.append( (tmpFontType, tmplistOfOutOfOrderGlyphs))
	if gTraceModeEnabled:
		print "[Info] Explicit Out Of Order Glyphs List: " , gListOfFontNamesToOutOfOrderGlyphs
	# arrange list properly:
	# check if the list contains same item as key and value (in different pairs)
	# if such case then the pair with the key should precede the pair with the value matched,
	# to avoid replacing instances of a special character (key) with a delegate (value) that will be later replaced again due to the second pair
	#
	if(len(gListOfFontNamesToOutOfOrderGlyphs) > 0):
		for (itFontName, itOOOGlyphList) in gListOfFontNamesToOutOfOrderGlyphs:
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
			gArrangedListOfFontNamesToOutOfOrderGlyphs.append( ( itFontName, itOOOGlyphList))
	if gTraceModeEnabled:
		print "[Debug] Arranged Glyphs Delegates List: " , gArrangedListOfFontNamesToOutOfOrderGlyphs
	return

#
# Fill the gActorPropertyEntries table
def initActorPropertyEntries(thePathToActorNamesTxt):
	global gActorPropertyEntriesWasInit
	global gActorPropertyEntries
	firstLine = True
	#if gTraceModeEnabled:
	#	print "[Debug] opening actornames"
	if thePathToActorNamesTxt is None or not thePathToActorNamesTxt:

		actorNamesTextFile = u'actornames.txt'
		relPath = u'.'
		thePathToActorNamesTxt = os.path.join(relPath, actorNamesTextFile)
		print "[Warning] Actor names text file %s not found in arguments. Attempting to open local file if it exists" % (thePathToActorNamesTxt)

	if os.access(thePathToActorNamesTxt, os.F_OK):
		print "[Info] Actor names text file found: {0}".format(thePathToActorNamesTxt)

		with open(thePathToActorNamesTxt) as tsv:
			for line in csv.reader(tsv, dialect="excel-tab"):
				#skip first line header
				if firstLine == True:
					#if gTraceModeEnabled:
					#	print "[Debug] skipping Header line in Excel sheet"
					firstLine = False
				else:
					gActorPropertyEntries.append(line)
		gActorPropertyEntriesWasInit = True
		tsv.close()
	else:
		## error
		print "[Error] Actor names text file not found: {0}".format(thePathToActorNamesTxt)
		sys.exit(1)	# terminate if finding actor names file failed (Blade Runner)

def getActorShortNameById(lookupActorId):
	if not gActorPropertyEntriesWasInit:
		return ''
	else:
		for actorEntryTmp in gActorPropertyEntries:
			if int(actorEntryTmp[0]) == int(lookupActorId):
				return actorEntryTmp[1]
	return ''


def getActorFullNameById(lookupActorId):
	if not gActorPropertyEntriesWasInit:
		return ''
	else:
		for actorEntryTmp in gActorPropertyEntries:
			if int(actorEntryTmp[0]) == int(lookupActorId):
				return actorEntryTmp[2]
	return ''

def getActorIdByShortName(lookupActorShortName):
	if not gActorPropertyEntriesWasInit:
		return ''
	else:
		for actorEntryTmp in gActorPropertyEntries:
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
	if gTraceModeEnabled:
		print '[Debug] ', (strParam +': '  +''.join('{:08X}'.format(hash)))
	return hash

#
# aux - sort by first object in list of tuples
def getSortMixFilesKey(item):
	keyTmp = item[0] & 0xFFFFFFFF

	signedKeyTmp = ctypes.c_int32(keyTmp).value
	return signedKeyTmp

def getSupportedInGameQuotesSheetsList():
	supportedInGameQuotesSheetsList = []
	for tmpActiveLanguageDescriptionCodeTuple in SUPPORTED_LANGUAGES_DESCRIPTION_CODE_TLIST:
		if (gActiveLanguageDescriptionCodeTuple[1] != '#' and tmpActiveLanguageDescriptionCodeTuple[1] == gActiveLanguageDescriptionCodeTuple[1]) \
			or (gActiveLanguageDescriptionCodeTuple[1] == '#' and tmpActiveLanguageDescriptionCodeTuple[1] != '#' and tmpActiveLanguageDescriptionCodeTuple[0] != 'RU_RUS'):
			supportedInGameQuotesSheetsList += [(x + '%s.TR%s' % (tmpActiveLanguageDescriptionCodeTuple[1], tmpActiveLanguageDescriptionCodeTuple[1])) for x in SUPPORTED_INGAME_DIALOGUE_SHEETS]
	return supportedInGameQuotesSheetsList

def getSupportedSubtitleSheetsList():
	mergedListOfSupportedSubtitleSheets = []
	mergedListOfSupportedSubtitleSheets += getSupportedInGameQuotesSheetsList()
	mergedListOfSupportedSubtitleSheets += [(x + 'E.VQA') for x in SUPPORTED_VIDEO_DIALOGUE_SHEETS_ENGLISH]
	for tmpActiveLanguageDescriptionCodeTuple in SUPPORTED_LANGUAGES_DESCRIPTION_CODE_TLIST:
		if (gActiveLanguageDescriptionCodeTuple[1] != '#' and tmpActiveLanguageDescriptionCodeTuple[1] == gActiveLanguageDescriptionCodeTuple[1]) \
			or (gActiveLanguageDescriptionCodeTuple[1] == '#' and tmpActiveLanguageDescriptionCodeTuple[1] != '#' and tmpActiveLanguageDescriptionCodeTuple[0] != 'RU_RUS'):
			mergedListOfSupportedSubtitleSheets += [(x + '%s.VQA' % (tmpActiveLanguageDescriptionCodeTuple[1])) for x in SUPPORTED_VIDEO_DIALOGUE_SHEETS_LOCALIZED]
	return mergedListOfSupportedSubtitleSheets

def getSupportedTranslatedTrxFilenamesList():
	listOfSupportedTranslatedTrxFilenames = []
	for tmpActiveLanguageDescriptionCodeTuple in SUPPORTED_LANGUAGES_DESCRIPTION_CODE_TLIST:
		if (gActiveLanguageDescriptionCodeTuple[1] != '#' and tmpActiveLanguageDescriptionCodeTuple[1] == gActiveLanguageDescriptionCodeTuple[1]) \
			or (gActiveLanguageDescriptionCodeTuple[1] == '#' and tmpActiveLanguageDescriptionCodeTuple[1] != '#' and tmpActiveLanguageDescriptionCodeTuple[0] != 'RU_RUS'):
			for translatedTRxFileName in [ (x[0] + '%s' % (tmpActiveLanguageDescriptionCodeTuple[1])) for x in SUPPORTED_TRANSLATION_SHEETS] :
				if translatedTRxFileName[:-1] != SUPPORTED_DIALOGUE_VERSION_SHEET[:-1] \
					or (gActiveLanguageDescriptionCodeTuple[1] == '#' and tmpActiveLanguageDescriptionCodeTuple[1] == 'E') \
					or (gActiveLanguageDescriptionCodeTuple[1] != '#'):
					if translatedTRxFileName[:-1] == SUPPORTED_DIALOGUE_VERSION_SHEET[:-1]:
						listOfSupportedTranslatedTrxFilenames.append(SUPPORTED_DIALOGUE_VERSION_SHEET)
					else:
						listOfSupportedTranslatedTrxFilenames.append(translatedTRxFileName)
	return listOfSupportedTranslatedTrxFilenames
#
def outputMIX():
	# output file should be DEFAULT_SUBTITLES_MIX_OUTPUT_NAME
	print "[Info] Writing to output MIX file: %s..." % (DEFAULT_SUBTITLES_MIX_OUTPUT_NAME)

	errorFound = False
	outMIXFile = None
	try:
		relPath = u'.'
		outputMixPath = os.path.join(relPath, DEFAULT_SUBTITLES_MIX_OUTPUT_NAME)
		outMIXFile = open(outputMixPath, 'wb')
	except Exception as e:
		errorFound = True
		print '[Error] Unable to write to output MIX file. ' + str(e)
	if not errorFound:
		# Write header
		# 2 bytes: number of entries (NumFiles)
		# TODO 4 bytes: size of data segment
		# 12 * NumFiles bytes: Entry descriptors table
		#			4 bytes: ID (hash)
		#			4 bytes: Byte offset in Data Segment
		#			4 bytes: Byte length of entry data
		# *Data Segment* - contains the file data. Offset from Entry Descriptors does not include header segment byte length.
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
		#		 -- So we sort hash but we first translate the unsigned key to signed with ctypes
		# 2) the offsets are not necessarily sorted, meaning that the first entry in the index table won't necessarily have the 0x00000000 offset
		i = 0
		mixFileEntries = []
		totalFilesDataSize = 0
		currOffsetForDataSegment = 0 # we start after header and table of index entries, from 0, (but this means that when reading the offset we need to add 6 + numOfFiles * 12). This does not concern us though.

		mergedListOfSupportedSubtitleSheets = getSupportedSubtitleSheetsList()

		if gTraceModeEnabled:
			print "[Trace] mergedListOfSupportedSubtitleSheets="
			print mergedListOfSupportedSubtitleSheets

		for sheetDialogueName in mergedListOfSupportedSubtitleSheets:
			sheetDialogueNameTRx = sheetDialogueName
			if sheetDialogueNameTRx[-4:-1] != '.TR':
				tmpActiveLanguageTRxCode = sheetDialogueNameTRx[-5]
				sheetDialogueNameTRx = sheetDialogueName[:-4] + ('.TR%s' %(tmpActiveLanguageTRxCode))
			if os.path.isfile('./' + sheetDialogueNameTRx):
				entryID = calculateFoldHash(sheetDialogueNameTRx)
				mixEntryfileSizeBytes = os.path.getsize('./' + sheetDialogueNameTRx)
				mixFileEntries.append((entryID, sheetDialogueNameTRx, mixEntryfileSizeBytes))
				totalFilesDataSize += mixEntryfileSizeBytes

		supportedTranslatedTrxFilenamesList = getSupportedTranslatedTrxFilenamesList()
		for translatedTRxFileName in supportedTranslatedTrxFilenamesList:
			if os.path.isfile('./' + translatedTRxFileName):
				entryID = calculateFoldHash(translatedTRxFileName)
				mixEntryfileSizeBytes = os.path.getsize('./' + translatedTRxFileName)
				mixFileEntries.append((entryID, translatedTRxFileName, mixEntryfileSizeBytes))
				totalFilesDataSize += mixEntryfileSizeBytes

		for otherFileName in SUPPORTED_OTHER_FILES_FOR_MIX:
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

		if gTraceModeEnabled:
			print ("[Debug] Sorted Entries based on EntryId.")
		for mixFileEntry in mixFileEntries:
			if gTraceModeEnabled:
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
				print ("[Error] Error while reading in ENTRY file")
				break

		outMIXFile.close()
		print "[Info] Total Resource files packed in %s: %d" % (DEFAULT_SUBTITLES_MIX_OUTPUT_NAME, numOfFiles)
		print "[Info] Done."
	return
#
# END FOR MIX FILE
#
#

	# TODO some ' (single quote characters) appear as \u2019 and others appear normally as '. Probably due to Excel auto-converting them.
	# DONE manually I've replaced all weird \u2019 single quotes with '''
	# the Spanish n is \xf1 -> we put it at ASCII value: \xA5 -- font index 0xA6 ?
	# the Spanish i is \xed -> we put it at ASCII value: \xA2 -- font index 0xA3 ?
	# pâté
	#	 a actual ASCII value is 0xE2 in codepage 1252 -- put it in ASCII value 0xA6 (165) -- font index 0xA7
	#	 e actual ASCII value is 0xE9 in codepage 1252 -- put it in ASCII value 0xA7 (166) -- font index 0xA8
	# TODO what are other characters are special?
	# TODO transition to ASCII chars to store in TRE file?
	# DONE manually I've replaced all one-char '...' with three dots
	# TODO actors TRE has 0x49 entries, (73 names), but table of ids has 73 entries BUT the offset table (first offset is calced + 0x04, so from end of the first 4 count bytes) has 74 entries. The last entry indexes the end of file (!)
	# TODO all strings are NULL terminated in the TRE file!

def translateQuoteToAsciiProper(cellObj, pSheetName):
	if cellObj.ctype == xlrd.XL_CELL_NUMBER:
		return '%.2f' % cellObj.value
	newQuoteReplaceSpecials =  cellObj.value.encode("utf-8")
	#if gTraceModeEnabled:
	#	print ('[Debug] Encoded to unicode: %s' % (newQuoteReplaceSpecials))
	newQuoteReplaceSpecials = newQuoteReplaceSpecials.decode("utf-8")

	pertinentListOfOutOfOrderGlyphs = []

	mergedListOfSupportedSubtitleSheets = getSupportedSubtitleSheetsList()
	supportedTranslatedTrxFilenamesList = getSupportedTranslatedTrxFilenamesList()
	localTargetEncoding = ''

	#if gTraceModeEnabled:
	#	print '[Debug] ', pSheetName
	#	print '[Debug] ', mergedListOfSupportedSubtitleSheets
	#	print '[Debug] ', DEFAULT_SUBTITLES_FONT_NAME[:-4]
	#	print [x[0] for x in gListOfFontNamesToOutOfOrderGlyphs]
	if pSheetName in mergedListOfSupportedSubtitleSheets:

		for (tmpFontName, tmpTargetEnc) in gTargetEncodingPerFont:
			if tmpFontName == DEFAULT_SUBTITLES_FONT_NAME[:-4]:
				localTargetEncoding = tmpTargetEnc
				break

		if(len(gListOfFontNamesToOutOfOrderGlyphs) > 0):
			for (tmpFontName, tmpOOOList) in gListOfFontNamesToOutOfOrderGlyphs:
				if tmpFontName == DEFAULT_SUBTITLES_FONT_NAME[:-4]:
					pertinentListOfOutOfOrderGlyphs = tmpOOOList
					break
	elif pSheetName in supportedTranslatedTrxFilenamesList:
		pertinentFontType = ''
        #[treAndFontTypeTuple for treAndFontTypeTuple in SUPPORTED_TRANSLATION_SHEETS if treAndFontTypeTuple[0] == pSheetName]
		for (tmpSheetName, tmpFontType) in SUPPORTED_TRANSLATION_SHEETS:
			if tmpSheetName == pSheetName[:-1]:
				pertinentFontType = tmpFontType
				break

		for (tmpFontName, tmpTargetEnc) in gTargetEncodingPerFont:
			if tmpFontName == pertinentFontType:
				localTargetEncoding = tmpTargetEnc
				break

		if(len(gListOfFontNamesToOutOfOrderGlyphs) > 0):
			for (tmpFontName, tmpOOOList) in gListOfFontNamesToOutOfOrderGlyphs:
				if tmpFontName == pertinentFontType:
					pertinentListOfOutOfOrderGlyphs = tmpOOOList
					break

	#newQuoteReplaceSpecials = newQuoteReplaceSpecials.replace(u"\u0386", u"\u00A3")
	for repTuple in pertinentListOfOutOfOrderGlyphs:
		newQuoteReplaceSpecials = newQuoteReplaceSpecials.replace(repTuple[0], repTuple[1])
	# WORKAROUND, we re-replace the Spanish i delegate again here!
	#newQuoteReplaceSpecials = newQuoteReplaceSpecials.replace(u'\xa2', u'\u0386')   # this is needed for spanish i because in utf-8 it's actually the u'\u0386' that's assigned to A tonomeno which is the delegate.
	#newQuoteReplaceSpecials = newQuoteReplaceSpecials.replace(u"\u0386", u"\u00A3")
	##newQuoteReplaceSpecials = newQuoteReplaceSpecials.replace(u"\u0386", u"\u00A3")	 # greek alpha tonomeno -- TODO which character is this in the excel (utf value) ???
	#newQuoteReplaceSpecials = newQuoteReplaceSpecials.replace(u"\u00ed", u"\u00A2")	 # spanish i
	#newQuoteReplaceSpecials = newQuoteReplaceSpecials.replace(u"\u00f1", u"\u00A5")	 # spanish n
	##newQuoteReplaceSpecials = newQuoteReplaceSpecials.replace(u"\u00A4", u"\u00A5")  # spanish n
	#newQuoteReplaceSpecials = newQuoteReplaceSpecials.replace(u"\u00e2", u"\u00A6")	 # a from pate -- todo this is not confirmed in-game font (but it is in our external font as of	 yet)
	#newQuoteReplaceSpecials = newQuoteReplaceSpecials.replace(u"\u00e9", u"\u00A7")	 # e from pate -- todo this is not confirmed in-game font (but it is in our external font as of	 yet)
	## other replacements.
	newQuoteReplaceSpecials = newQuoteReplaceSpecials.replace(u"\u2019", u"\u0027")	 # right single quote
	newQuoteReplaceSpecials = newQuoteReplaceSpecials.replace(u"\u2018", u"\u0027")	 # left single quote
	newQuoteReplaceSpecials = newQuoteReplaceSpecials.replace(u"\u2026", u"\u002e\u002e\u002e")	 # three dots together (changes length)
	newQuoteReplaceSpecials = newQuoteReplaceSpecials.replace(u"\u201D", u"\u0022")	 # right double quote
	newQuoteReplaceSpecials = newQuoteReplaceSpecials.replace(u"\u201C", u"\u0022")	 # left double quote
	# TODO? replace new line ???	with another char (maybe |)?

	#newQuoteReplaceSpecialsUnicode = unicode(newQuoteReplaceSpecials, 'utf-8')
	#newQuoteReplaceSpecialsStr = unicode.encode("%s" % newQuoteReplaceSpecials, localTargetEncoding)
	#if gTraceModeEnabled:
	#	print '[Debug] ', type(newQuoteReplaceSpecials)                 # type is unicode
	#	print '[Debug] ', type(newQuoteReplaceSpecials.encode('utf-8')) # type is str
	#	print '[Debug] ', localTargetEncoding
	#	print '[Debug] ', newQuoteReplaceSpecials
	#newQuoteReplaceSpecialsDec = newQuoteReplaceSpecials.decode(localTargetEncoding)
	newQuoteReplaceSpecialsRetStr = ''
	try:
		newQuoteReplaceSpecialsRetStr = newQuoteReplaceSpecials.encode(localTargetEncoding)
	except Exception as e:
		print "[Error] Could not encode text in " + localTargetEncoding + "::" + str(e)
		newQuoteReplaceSpecialsRetStr = "??????????"
	#try:
	#	newQuoteReplaceSpecialsRetStr = newQuoteReplaceSpecials.encode(localTargetEncoding)
	#except:
	#	print "==============================================================================="
	#	print "==============================================================================="
	#	print "ERROR:"
	#	print newQuoteReplaceSpecials
	#	print newQuoteReplaceSpecials.encode(localTargetEncoding, errors='xmlcharrefreplace')
	#	print "==============================================================================="
	#	print "==============================================================================="
	#	newQuoteReplaceSpecialsRetStr = newQuoteReplaceSpecials.encode(localTargetEncoding, errors='xmlcharrefreplace')
	return newQuoteReplaceSpecialsRetStr
	#return newQuoteReplaceSpecialsEnStr

# aux function to validate an int cell value
def parseIntCellValue(cell_obj_value, row_idx, col_idx, xl_sheet_name, xl_sheet_nrows, xl_sheet_ncols):
	retCandValue = 0
	try:
		retCandValue =	int(cell_obj_value)
	except Exception as e:
		print "\n[Error] Invalid cell value at row: %d, column: %d in sheet: %s (rows: %d, columns: %d)" %(row_idx+1, col_idx+1, xl_sheet_name, xl_sheet_nrows, xl_sheet_ncols)
		print "\tPlease check for redundant empty rows or invalid cell values.\n"
		sys.exit()
	return retCandValue


def inputXLS(pathtoInputExcelFilename):
	global gNumOfSpokenQuotes
	global gTableOfStringIds
	global gTableOfStringOffsets
	global gTableOfStringEntries
	# Open the workbook
	xl_workbook = None

	pathToExcelFilenameNoExt, excelFileExtension = os.path.splitext(pathtoInputExcelFilename)
	# Check if no extension or invalid extension
	if excelFileExtension is None \
			and not (excelFileExtension.lower() == "xls" or excelFileExtension.lower() == "xlsx"):
		print "[Error] Bad file extension found for the Excel input file %s. Supported extensions are '.xls' and 'xlsx'" % (excelFileExtension, pathtoInputExcelFilename)
		sys.exit(1) # Terminate if the input Excel had unsupported extension (or none)

	try:
		xl_workbook = xlrd.open_workbook(pathtoInputExcelFilename, encoding_override="utf-8")
		if xl_workbook is not None:
			print "[Info] Opened Excel input file: %s" % (pathtoInputExcelFilename)
	except Exception as e:
		print '[Error] Could not open the Excel input file: '  + str(e)

	if xl_workbook is None:
		print '[Error] Giving up. Could not open the specified Excel input file.'
		sys.exit(1) # Terminate if the input Excel was not found
	# List sheet names, and pull a sheet by name
	#
	#sheet_names = xl_workbook.sheet_names()
	#if gTraceModeEnabled:
	#	print('[Debug] Sheet Names', sheet_names)
	#
	#xl_sheet = xl_workbook.sheet_by_name(sheet_names[0])

	# Or grab the first sheet by index
	#  (sheets are zero-indexed)
	# First sheet is the in-game quotes
	#
	#xl_sheet = xl_workbook.sheet_by_index(0)
	#
	#
	supportedInGameQuotesSheetsList = getSupportedInGameQuotesSheetsList()
	mergedListOfSupportedSubtitleSheets = getSupportedSubtitleSheetsList()
	supportedTranslatedTrxFilenamesList = getSupportedTranslatedTrxFilenamesList()

	mergedListOfSupportedSubtitleSheetsAndTranslatedTREs = mergedListOfSupportedSubtitleSheets + supportedTranslatedTrxFilenamesList

	# Check for a version info sheet and create one if it does not exist
	xl_sheet = None
	try:
		if gTraceModeEnabled:
			print '[Debug] Checking for existence of sheet: ' + SUPPORTED_DIALOGUE_VERSION_SHEET
		xl_sheet = xl_workbook.sheet_by_name(SUPPORTED_DIALOGUE_VERSION_SHEET)
	except Exception as e:
		if gTraceModeEnabled:
			print '[Debug] Could not open requested sheet: ' + SUPPORTED_DIALOGUE_VERSION_SHEET + ' in Excel::' + str(e)

	if xl_sheet is None:
		# we didn't find a sheet for version info, so we should auto-create a Sheet for it
		if gTraceModeEnabled:
			print '[Debug] Sheet: %s was not found. Creating a temporary sheet for version info...' % (SUPPORTED_DIALOGUE_VERSION_SHEET)
		sbtlVersTRInstance = sbtlVersTextResource(gTraceModeEnabled)
		bookCopy = copy(xl_workbook) 
		xl_sheet = bookCopy.add_sheet(SUPPORTED_DIALOGUE_VERSION_SHEET)
		n = 0
		col1_name = 'Subtitles Version Info'
		xl_sheet.write(n, 0, col1_name)
		# Second Row
		n = 1
		col1_name = 'ID'
		col2_name = 'Value'
		col3_name = 'Notes'
		xl_sheet.write(n, 0, col1_name)
		xl_sheet.write(n, 1, col2_name)
		xl_sheet.write(n, 2, col3_name)
		n += 1
		objUTF8Unicode = None
		for m, e1 in enumerate(sbtlVersTRInstance.getSbtlVersEntriesList(), n):
			xl_sheet.write(m, 0, e1[0])
			for i1 in range(1,3):
				objStr = e1[i1]
				try:
					# We assume utf-8 charset (since we get the text from a python script)
					objUTF8Unicode = unicode(objStr, 'utf-8')
				except Exception as e:
					print '[Error] Failed to create unicode string: ' + str(e)
					objUTF8Unicode = unicode("???", 'utf-8')
				xl_sheet.write(m, i1, objUTF8Unicode)
		try:
			bookCopy.save(pathtoInputExcelFilename)
		except Exception as e:
			print "[Error] Giving up: Could not save to output Excel file:: " + str(e)
			sys.exit(1) # Terminate if we couldn't write to output Excel file

		if gTraceModeEnabled:
			print '[Debug] Sheet: %s was created successfully.' % (SUPPORTED_DIALOGUE_VERSION_SHEET)
		inputXLS(pathtoInputExcelFilename)
		return
	# end of check for a version info sheet

	if gTraceModeEnabled:
		print '[Debug] mergedListOfSupportedSubtitleSheetsAndTranslatedTREs: ', mergedListOfSupportedSubtitleSheetsAndTranslatedTREs
	for sheetDialogueName in mergedListOfSupportedSubtitleSheetsAndTranslatedTREs:
		xl_sheet = None
		try:
			xl_sheet = xl_workbook.sheet_by_name(sheetDialogueName)
		except Exception as e:
			if gTraceModeEnabled:
				print '[Debug] Could not open requested sheet: ' + sheetDialogueName + ' in Excel::' + str(e)

		if xl_sheet is None:
			if (not gTraceModeEnabled) and (sheetDialogueName not in supportedTranslatedTrxFilenamesList):
				print '[Warning] %s sheet was not found in input Excel file.' % (sheetDialogueName)

		if xl_sheet is not None:
			if gTraceModeEnabled:
				print ('[Debug] Sheet name: %s' % xl_sheet.name)
			gNumOfSpokenQuotes = xl_sheet.nrows - 2 # all rows minus the first TWO rows with headers
			if gTraceModeEnabled:
				print ('[Debug] Number of spoken quotes: %d' % gNumOfSpokenQuotes)
			# stats for debug
			extremeQuotesList = []
			longestLength = 0
			predefinedLengthThreshold = 145
			quoteNumAboveThreshold = 0
			# end of stats for debug


			absStartOfIndexTable = 4
			absStartOfOffsetTable = absStartOfIndexTable + (gNumOfSpokenQuotes * 4)		# = 4 + 0x1577 * 4 = 4 + 0x55DC = 0x55E0
			absStartOfStringTable = absStartOfOffsetTable + ((gNumOfSpokenQuotes + 1) * 4) # =	 0x55E0 + (0x1578 * 4) = 0xABC0
			curStrStartOffset = absStartOfStringTable - 4
			newQuoteReplaceSpecialsAscii = ''
			tmpQuoteID = 0
			#switchFlagShowQuote = False # for debugging
			tmpStartFrame = 0			# for VQA sheets
			tmpEndFrame = 0				# for VQA sheets
			mode = 0					# init to unknown
			# Decide vqaSheetFormatVersion
			# 0: old version - Col order: "Start (YT)", "End (YT)", "Subtitle", "By Actor", "StartTime", "Time Diff-SF", "TimeDiff-SF(ms)", "TimeDiff-EF", "TimeDiff-EF(ms)", "Frame Start", "Frame End", "Notes"
			# 1: new version - Col order: "Frame Start", "Frame End", "Subtitle", "Time Start", "Time End", "By Actor", "Notes"
			vqaSheetFormatVersion = 0

			if xl_sheet.name in supportedInGameQuotesSheetsList:
				if gTraceModeEnabled:
					print '[Debug] IN GAME QUOTES'
				mode = 1 #in-game quote
			elif xl_sheet.name in mergedListOfSupportedSubtitleSheets:
				if gTraceModeEnabled:
					print '[Debug] VQA SCENE DIALOGUE'
				mode = 2 #VQA
				# check if the VQA sheets are of the old format or the new format
				cell_obj = xl_sheet.cell(1, 0)
				if cell_obj is not None and cell_obj.value.lower() == 'frame start':
					vqaSheetFormatVersion = 1
			elif xl_sheet.name in supportedTranslatedTrxFilenamesList:
				if gTraceModeEnabled:
					print '[Debug] TRANSLATED TEXT RESOURCE'
				mode = 3 # Translated TRE
			#
			del gTableOfStringIds[:]
			del gTableOfStringEntries[:]
			del gTableOfStringOffsets[:]
			for row_idx in range(2, xl_sheet.nrows):
				#if gTraceModeEnabled:
				#	print "[Debug] Line %d" % (row_idx)
				for col_idx in range(0, xl_sheet.ncols):
					cell_obj = xl_sheet.cell(row_idx, col_idx)
					#
					# FOR IN-GAME QUOTES -- Iterate through columns starting from col 0. We need cols: 0, 2
					#
					if mode == 1:
						#if gTraceModeEnabled:
						#	print ('[Debug] Column: [%s] cell_obj: [%s]' % (col_idx, cell_obj))
						if(col_idx == 0):
							#switchFlagShowQuote = False
							twoTokensfirstColSplitAtDotXLS = cell_obj.value.split('.', 1)
							if len(twoTokensfirstColSplitAtDotXLS) == 2:
								twoTokensfirstColSplitAtDashXLS = twoTokensfirstColSplitAtDotXLS[0].split('-', 1)
								if len(twoTokensfirstColSplitAtDashXLS) == 2:
									tmpActorPart = parseIntCellValue(twoTokensfirstColSplitAtDashXLS[0], row_idx, col_idx, xl_sheet.name, xl_sheet.nrows, xl_sheet.ncols)
									tmpSubQuotePart = parseIntCellValue(twoTokensfirstColSplitAtDashXLS[1], row_idx, col_idx, xl_sheet.name, xl_sheet.nrows, xl_sheet.ncols)
									tmpQuoteID = tmpActorPart * 10000 + tmpSubQuotePart
									#if gTraceModeEnabled:
									#	print ('[Debug] Row_idx: %d. Tag: %s, QuoteId: [%d]' % (row_idx, twoTokensfirstColSplitAtDotXLS[0], tmpQuoteID))
									gTableOfStringIds.append(tmpQuoteID)
									#if(tmpQuoteID == 160110 or tmpQuoteID == 160010):
									#	switchFlagShowQuote = True

						elif(col_idx == 1) :
							#if switchFlagShowQuote == True:
								#if gTraceModeEnabled:
								#	print ('[Debug] length: %d: %s' % (len(cell_obj.value), cell_obj.value))
								#	print ('[Debug] object: %s' % (cell_obj))
								##	newQuoteReplaceSpecials =	 cell_obj.value.decode("utf-8") # unicode(cell_obj.value, 'windows-1252')
								##	print ('[Debug] decoded to unicode: %s ' % (newQuoteReplaceSpecials)) # error with char xf1
							newQuoteReplaceSpecialsAscii = translateQuoteToAsciiProper(cell_obj, xl_sheet.name)
							#if switchFlagShowQuote == True:
								#if gTraceModeEnabled:
								#	print ('[Debug] length: %d: %s' % (len(newQuoteReplaceSpecialsAscii), newQuoteReplaceSpecialsAscii))
							#if gTraceModeEnabled:
							#	print ':'.join(x.encode('hex') for x in newQuoteReplaceSpecialsAscii)	 # seems to work.  new chars are non-printable but exist in string

							gTableOfStringEntries.append(newQuoteReplaceSpecialsAscii)
							gTableOfStringOffsets.append(curStrStartOffset)
							curStrStartOffset += (len(newQuoteReplaceSpecialsAscii) + 1)
							if ( longestLength < len(newQuoteReplaceSpecialsAscii)):
								longestLength = len(newQuoteReplaceSpecialsAscii)
							if ( predefinedLengthThreshold < len(newQuoteReplaceSpecialsAscii)):
								extremeQuotesList.append((tmpQuoteID, newQuoteReplaceSpecialsAscii))
								quoteNumAboveThreshold += 1
								#print ('[Debug] row_idx: %d. tag %s: quoteId [%d], length: %d: %s' % (row_idx, twoTokensfirstColSplitAtDotXLS[0], tmpQuoteID, len(newQuoteReplaceSpecialsAscii), newQuoteReplaceSpecialsAscii))
					#
					# FOR VQAs -- Iterate through columns
					# Earlier versions (up to Jun 23): We need columns: 2, 9, 10
					# New version (post Jun 23): We need columns: 1, 2, 3
					#
					elif mode == 2:
						if (col_idx == 2): # subtitle text
							newQuoteReplaceSpecialsAscii = translateQuoteToAsciiProper(cell_obj, xl_sheet.name)
							#print ('[Debug] length: %d: %s' % (len(newQuoteReplaceSpecialsAscii), newQuoteReplaceSpecialsAscii))
							#print ':'.join(x.encode('hex') for x in newQuoteReplaceSpecialsAscii)	# seems to work.  new chars are non-printable but exist in string
							# don't append to gTableOfStringEntries yet
						elif (vqaSheetFormatVersion == 1 and col_idx == 0) or (vqaSheetFormatVersion == 0 and col_idx == 9):
							# startFrame
							#print "[Debug] cell: %s" % (cell_obj.value)
							tmpStartFrame = parseIntCellValue(cell_obj.value, row_idx, col_idx, xl_sheet.name, xl_sheet.nrows, xl_sheet.ncols)
						elif (vqaSheetFormatVersion == 1 and col_idx == 1) or (vqaSheetFormatVersion == 0 and col_idx == 10):
							# endFrame
							tmpEndFrame = parseIntCellValue(cell_obj.value, row_idx, col_idx, xl_sheet.name, xl_sheet.nrows, xl_sheet.ncols)
						if (vqaSheetFormatVersion == 1 and col_idx == 2) or (vqaSheetFormatVersion == 0 and col_idx == 10):
							# do the final processing when you reached the final meaningful column
							tmpQuoteID = tmpStartFrame | (tmpEndFrame << 16) # top 16 bits are end frame (up to 65536 frames which is enough) and low 16 bits are startFrame
							gTableOfStringIds.append(tmpQuoteID)
							gTableOfStringEntries.append(newQuoteReplaceSpecialsAscii)
							gTableOfStringOffsets.append(curStrStartOffset)
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
						if gTraceModeEnabled:
							print ('[Debug] Column: [%s] cell_obj: [%s]' % (col_idx, cell_obj))
						if(col_idx == 0):
							tmpQuoteID = parseIntCellValue(cell_obj.value, row_idx, col_idx, xl_sheet.name, xl_sheet.nrows, xl_sheet.ncols)
							gTableOfStringIds.append(tmpQuoteID)
						elif(col_idx == 1) :
							#if switchFlagShowQuote == True:
							#	 print ('[Debug] length: %d: %s' % (len(cell_obj.value), cell_obj.value))
							#	 print ('[Debug] object: %s' % (cell_obj))
							#	 #newQuoteReplaceSpecials =	 cell_obj.value.decode("utf-8") # unicode(cell_obj.value, 'windows-1252')
							#	 #print ('[Debug] decoded to unicode: %s ' % (newQuoteReplaceSpecials)) # error with char xf1
							newQuoteReplaceSpecialsAscii = translateQuoteToAsciiProper(cell_obj, xl_sheet.name)
							if xl_sheet.name == SUPPORTED_DIALOGUE_VERSION_SHEET:
								if tmpQuoteID == 2:
								#generate date timestamp
									now = datetime.now()
									newQuoteReplaceSpecialsAscii = now.strftime("%H:%M:%S %d/%m/%Y")
								elif tmpQuoteID == 3:
									newQuoteReplaceSpecialsAscii = gActiveLanguageDescriptionCodeTuple[2]
							#if switchFlagShowQuote == True:
							#	 print ('[Debug] length: %d: %s' % (len(newQuoteReplaceSpecialsAscii), newQuoteReplaceSpecialsAscii))
							#print ':'.join(x.encode('hex') for x in newQuoteReplaceSpecialsAscii)	 # seems to work.  new chars are non-printable but exist in string

							gTableOfStringEntries.append(newQuoteReplaceSpecialsAscii)
							gTableOfStringOffsets.append(curStrStartOffset)
							curStrStartOffset += (len(newQuoteReplaceSpecialsAscii) + 1)
							if ( longestLength < len(newQuoteReplaceSpecialsAscii)):
								longestLength = len(newQuoteReplaceSpecialsAscii)
							if ( predefinedLengthThreshold < len(newQuoteReplaceSpecialsAscii)):
								extremeQuotesList.append((tmpQuoteID, newQuoteReplaceSpecialsAscii))
								quoteNumAboveThreshold += 1
								#print ('[Debug] row_idx: %d. tag %s: quoteId [%d], length: %d: %s' % (row_idx, twoTokensfirstColSplitAtDotXLS[0], tmpQuoteID, len(newQuoteReplaceSpecialsAscii), newQuoteReplaceSpecialsAscii))

			gTableOfStringOffsets.append(curStrStartOffset) # the final extra offset entry
			if gTraceModeEnabled:
				print '[Debug] Longest Length: %d, Quotes above threshold (%d): %d' % (longestLength, predefinedLengthThreshold, quoteNumAboveThreshold)
				for extremQuotTuple in extremeQuotesList:
					print "[Debug] Id: %d, Q: %s" % (extremQuotTuple[0], extremQuotTuple[1])
			#
			# WRITE TO TRE FILE
			#
			errorFound = False
			outTRxFile = None
			outTRxFileName = sheetDialogueName
			# this check is basically for sheets that end in .VQA, which should now end in .TRx
			# ie. INTRO_E.VQA in EFIGS mode will create INTRO_E.TRE
			# in FIGS mode we can't create all TRx for each one of the .VQA sheets, because
			# that would be redundant rather than practical and also
			# pairs (MW_B01_E.TRI, MW_C01_E.TRE), (MW_B02_E.TRI, MW_C02_E.TRE), (MW_B03_E.TRI, MW_C03_E.TRE) have the same hash, which is a problem for their indexing in the MIX file
			if outTRxFileName[-4:-1] != '.TR':
				tmpActiveLanguageTRxCode = outTRxFileName[-5]
				outTRxFileName = sheetDialogueName[:-4] + ('.TR%s' %(tmpActiveLanguageTRxCode))
			try:
				outTRxFile = open("./" + outTRxFileName, 'wb')
			except Exception as e:
				errorFound = True
				print ('[Error] Unable to write to output file %s:: ' %(outTRxFileName)) + str(e)
			if not errorFound:
				numOfSpokenQuotesToWrite = pack('I', gNumOfSpokenQuotes)	# unsigned integer 4 bytes
				outTRxFile.write(numOfSpokenQuotesToWrite)
				# write string IDs table
				for idxe in range(0, len(gTableOfStringIds)):
					idOfStringToWrite = pack('I', gTableOfStringIds[idxe])  # unsigned integer 4 bytes
					outTRxFile.write(idOfStringToWrite)
				# write string offsets table
				for idxe in range(0, len(gTableOfStringOffsets)):
					offsetOfStringToWrite = pack('I', gTableOfStringOffsets[idxe])  # unsigned integer 4 bytes
					outTRxFile.write(offsetOfStringToWrite)
				#write strings with null terminator
				for idxe in range(0, len(gTableOfStringEntries)):
					outTRxFile.write(gTableOfStringEntries[idxe])
					outTRxFile.write('\0')
				outTRxFile.close()
	return

#
# Aux function to validate input language description
#
def getLanguageDescCodeTuple(candidateLangDescriptionStr):
	if (candidateLangDescriptionStr is None or not candidateLangDescriptionStr ):
		resultTuple = DEFAULT_LANG_DESC_CODE
	else:
		tmpMatchTuplesList = [ (x,y,z) for (x,y,z) in SUPPORTED_LANGUAGES_DESCRIPTION_CODE_TLIST if  x ==  candidateLangDescriptionStr]
		if tmpMatchTuplesList is not None and len(tmpMatchTuplesList) > 0:
			resultTuple = tmpMatchTuplesList[0]
		else:
			resultTuple = None
	return resultTuple

def printInfoMessageForLanguageSelectionSyntax():
	tmpCSVSupportedLangDescValues = ", ".join( zip(*SUPPORTED_LANGUAGES_DESCRIPTION_CODE_TLIST)[0] )
	print "Valid values for language selection are: %s" % (tmpCSVSupportedLangDescValues)
	print "Default value is: %s (%s)" % (DEFAULT_LANG_DESC_CODE[0], DEFAULT_LANG_DESC_CODE[2])
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
	global gTraceModeEnabled
	global gActiveLanguageDescriptionCodeTuple

	gTraceModeEnabled = False
	gActiveLanguageDescriptionCodeTuple = DEFAULT_LANG_DESC_CODE

	pathToQuoteExcelFile = ""
	pathToActorNamesTxt = ""
	pathToConfigureFontsTranslationTxt = ""

	candidateLangDescriptionTxt = ""

	invalidSyntax = False
	print "Running %s (%s)..." % (APP_NAME_SPACED, APP_VERSION)
	#print "Len of sysargv = %s" % (len(argsCL))
	if len(argsCL) == 2:
		if(argsCL[1] == '--help'or argsCL[1] == '-h'):
			print "%s %s supports Westwood's Blade Runner PC Game (1997)." % (APP_NAME_SPACED, APP_VERSION)
			print APP_SHORT_DESC
			print "Created by Praetorian of the classic adventures in Greek team."
			print "Always keep backups!"
			print "--------------------"
			print "Preparatory steps:"
			print "1. Copy the transcript Excel file (eg. BladeRunnerPCTLK.xlsx, latest version, downloaded from Google Sheets) in some folder on your PC."
			print "--------------------"
			print "%s takes 1 mandatory argument:" % (APP_WRAPPER_NAME)
			print "Valid syntax: "
			print "%s -x path_to_BladeRunnerPCTLK_xlsx [-ian path_to_actornames_txt] [-cft path_to_configureFontsTranslation_txt] [-ld gameInputLanguageDescription] [--trace]" % (APP_WRAPPER_NAME)
			print "-x is followed by the path to the excel file with the subtitle quotes."
			print "-ian is followed by the path to actornames.txt, if it's not in the current working directory."
			print "-cft is followed by the path to configureFontsTranslation.txt, if it's not in the current working directory."
			print "-ld is followed by the language description of the target game version that you will install the subtitles to."
			printInfoMessageForLanguageSelectionSyntax()
			print "The --trace switch enables more debug messages being printed during execution."
			print "--------------------"
			print "If the app finishes successfully, it creates a %sx file and a few other Text Resource (TRx) files " % (SUPPORTED_INGAME_DIALOGUE_SHEETS[0])
			print "for each VQAs sheet in the input Excel file respectively. Additionally, a %s file containing all " % (DEFAULT_SUBTITLES_MIX_OUTPUT_NAME)
			print "of the resources in the Excel file and a few extra (subtitle font, (optional) edited fonts) is created as well."
			print "All output files are written in the current working directory."
			print "--------------------"
			print "Thank you for using this app."
			print "Please provide any feedback to: %s " % (COMPANY_EMAIL)
			sys.exit()
		elif(argsCL[1] == '--version' or argsCL[1] == '-v'):
			print "%s %s supports Westwood's Blade Runner PC Game (1997)." % (APP_NAME_SPACED, APP_VERSION)
			print "Please provide any feedback to: %s " % (COMPANY_EMAIL)
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
				elif (argsCL[i] == '-ld'):
					candidateLangDescriptionTxt = argsCL[i+1]
			elif sys.argv[i] == '--trace':
				print "[Info] Trace mode enabled (more debug messages)."
				gTraceModeEnabled = True
			elif argsCL[i][:1] == '-':
				invalidSyntax = True
				break

		if not pathToQuoteExcelFile:
			invalidSyntax = True

		gActiveLanguageDescriptionCodeTuple = getLanguageDescCodeTuple(candidateLangDescriptionTxt)
		if (not invalidSyntax) and gActiveLanguageDescriptionCodeTuple is None:
			print "[Error] Invalid language code was specified"
			printInfoMessageForLanguageSelectionSyntax()
			invalidSyntax = True

		if not invalidSyntax:
			print "[Info] Game Language Selected: %s (%s)" % (gActiveLanguageDescriptionCodeTuple[0], gActiveLanguageDescriptionCodeTuple[2])
			#
			# Early check for external subtitles font file, since we no longer support internal font
			relPath = u'.'
			thePathToExternalFontFileFon = os.path.join(relPath, DEFAULT_SUBTITLES_FONT_NAME)
			if not os.path.isfile(thePathToExternalFontFileFon):
				print "[Error] Font file %s for subtitles was not found!" % (thePathToExternalFontFileFon)
				sys.exit(1)
			else:
				print "[Info] Supported font file for subtitles found: {0}".format(thePathToExternalFontFileFon)

			# parse any overrideEncoding file if exists:
			initOverrideEncoding(pathToConfigureFontsTranslationTxt)

			# parse the EXCEL File
			# parse Actors files:
			initActorPropertyEntries(pathToActorNamesTxt)
			#if gTraceModeEnabled:
			#	for actorEntryTmp in gActorPropertyEntries:
			#		print "[Debug] Found actor: %s %s %s" % (actorEntryTmp[0], actorEntryTmp[1], actorEntryTmp[2])
			inputXLS(pathToQuoteExcelFile)
			outputMIX()

	else:
		invalidSyntax = True

	if invalidSyntax == True:
		print "[Error] Invalid syntax\n Try: \n %s --help for more info \n %s --version for version info " % (APP_WRAPPER_NAME, APP_WRAPPER_NAME)
		print "Valid syntax: "
		print "%s -x path_to_BladeRunnerPCTLK_xlsx [-ian path_to_actornames_txt] [-cft path_to_configureFontsTranslation_txt] [-ld gameInputLanguageDescription] [--trace]" % (APP_WRAPPER_NAME)
		print "\nDetected arguments:"
		tmpi = 0
		for tmpArg in argsCL:
			if tmpi==0: #skip first argument
				tmpi+=1
				continue
			print "Argument: %s" % (tmpArg)
			tmpi+=1

# Total unique quotes seems to be 5495!
# TODO rename files in folders to conform to the underscore '_' and '-' format (a few don't -- let's have them all conforming!)
# #########################
#
if __name__ == "__main__":
	main(sys.argv[0:])
else:
	## debug
	#print '[Debug] %s was imported from another module' % (APP_WRAPPER_NAME)
	pass
