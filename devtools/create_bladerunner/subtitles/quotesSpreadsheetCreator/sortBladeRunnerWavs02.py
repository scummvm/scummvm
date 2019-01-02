#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Created by Praetorian (ShadowNate) for Classic Adventures in Greek
# classic.adventures.in.greek@gmail.com
#
# DONE Add code and switch option: to get the blade runner installation directory as input, then find the TLK files and export them with proper naming
# DONE fix proper names for sheets as per latest code changes
#
import os, sys

shutilLibFound = False
xlwtLibFound = False
csvLibFound = False

try:
	import shutil 
except ImportError:
	print "[Error] Shutil python library is required to be installed!" 
else:
	shutilLibFound = True

try:
	import xlwt 
except ImportError:
	print "[Error] xlwt python library is required to be installed!" 
else:
	xlwtLibFound = True
	
try:
	import csv 
except ImportError:
	print "[Error] csv python library is required to be installed!" 
else:
	csvLibFound = True

if 	(not shutilLibFound) or (not xlwtLibFound) or (not csvLibFound):
	sys.stdout.write("[Error] Errors were found when trying to import required python libraries\n")
	sys.exit(1)

from os import walk, errno, path
from xlwt import *
from audFileLib import *
from treFileLib import *

# encoding=utf8
#reload(sys)
#sys.setdefaultencoding('utf8')

COMPANY_EMAIL = "classic.adventures.in.greek@gmail.com"
APP_VERSION = "0.80"
APP_NAME = "sortBladeRunnerWavs"
APP_WRAPPER_NAME = "quotesSpreadsheetCreator.py"
APP_NAME_SPACED = "Blade Runner Transcript Excel Creator (bare bones)"
APP_SHORT_DESC = "* Create an Excel (.XLS) for transcribing Blade Runner.\n* (Optional) Extract TRx (x is the language code) files as additional sheets in the Excel\n* (Optional) Export speech files from the game's TLK resources to WAV format."

OUTPUT_XLS_FILENAME = 'out'
OUTPUT_XLS_FILENAME_EXT = '.xls'
OUTPUT_XLS_QUOTES_SHEET = 'INGQUO_E.TR'

SUPPORTED_TLK_INPUT_FILES = [('1.TLK', 'TLK01'), ('2.TLK', 'TLK02'), ('3.TLK', 'TLK03'), ('A.TLK', 'TLK0A'), ('SPCHSFX.TLK', 'TLKSPCHSFX')]
SUPPORTED_MIX_INPUT_FILES = ['STARTUP.MIX']
# 15 TRx files
SUPPORTED_EXPORTED_TRx_FILES = ['CLUES.TR','ACTORS.TR','CRIMES.TR','CLUETYPE.TR','KIA.TR','SPINDEST.TR','VK.TR','OPTIONS.TR','DLGMENU.TR','ENDCRED.TR','HELP.TR','SCORERS.TR','KIACRED.TR','ERRORMSG.TR','AUTOSAVE.TR']
SUPPORTED_PLACEHOLDER_VQA_FILES = ['WSTLGO_', 'BRLOGO_', 'INTRO_', 'MW_A_', 'MW_B01_', 'MW_B02_', 'MW_B03_', 'MW_B04_', 'MW_B05_', 'INTRGT_', 'MW_D_', 'MW_C01_', 'MW_C02_', 'MW_C03_', 'END04A_', 'END04B_', 'END04C_', 'END06_', 'END01A_', 'END01B_', 'END01C_', 'END01D_', 'END01E_', 'END01F_', 'END03_']

SUPPORTED_LANGUAGES_DESCRIPTION_CODE_TLIST = [('EN_ANY', 'E', 'English'), ('DE_DEU', 'G', 'German'), ('FR_FRA', 'F', 'French'), ('IT_ITA', 'I', 'Italian'), ('ES_ESP', 'S', 'Spanish'), ('RU_RUS', 'R', 'Russian')]
DEFAULT_LANG_DESC_CODE = SUPPORTED_LANGUAGES_DESCRIPTION_CODE_TLIST[0]

gTraceModeEnabled = False
gActiveLanguageDescriptionCodeTuple = ''

gStringReplacementForRootFolderWithExportedFiles = ""
gNumReplaceStartingCharacters = 0

gWavFiles = []
gWavFilesNoDups = []
gActorPropertyEntries = []				#[0]:id,	[1]:ShortHand Name	[2]:Full Name
gActorPropertyEntriesWasInit = False


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
	#print (strParam +': '  +''.join('{:08X}'.format(hash)))
	return hash

# Fill the gActorPropertyEntries table
def initActorPropertyEntries(thePathToActorNamesTxt):
	global gActorPropertyEntriesWasInit
	global gActorPropertyEntries
	firstLine = True
	gActorPropertyEntriesWasInit = False
#	 print "[Debug] opening actornames"
	if thePathToActorNamesTxt is None or not thePathToActorNamesTxt:

		actorNamesTextFile = u'actornames.txt'
		relPath = u'.'
		thePathToActorNamesTxt = os.path.join(relPath, actorNamesTextFile)
		print "[Warning] Actor names text file %s not found in arguments. Attempting to open local file if it exists" % (thePathToActorNamesTxt)	

	if os.access(thePathToActorNamesTxt, os.F_OK):
		print "[Info] Actor names text file found: {0}.".format(thePathToActorNamesTxt)

		with open(thePathToActorNamesTxt) as tsv:
			if gTraceModeEnabled: 
				print "[Debug] Parsing Override Encoding file info..."
			for line in csv.reader(tsv, dialect="excel-tab"):
				#skip first line header
				if firstLine == True:
					#if gTraceModeEnabled:
					#	print "[Debug] Skipping Header line in Excel sheet"
					firstLine = False
				else:
					gActorPropertyEntries.append(line)
		gActorPropertyEntriesWasInit = True
		tsv.close()
	else:
		## error
		print "[Error] Actor names text file not found: {0}.".format(thePathToActorNamesTxt)
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

def getActorShortNameAndLocalQuoteIdByAUDHashID(audHashId):
	actorId = 0
	actorShortName = ''
	actorLocalQuoteId = 0
	if not gActorPropertyEntriesWasInit:
		print "[Error] actor properties were not initialized!"
		return (actorId, actorShortName, actorLocalQuoteId)

	for actorEntryTmp in gActorPropertyEntries:
		if( (audHashId - (int(actorEntryTmp[0]) * 10000) ) >= 0) and ((audHashId - (int(actorEntryTmp[0]) * 10000)) < 10000):
			actorId = int(actorEntryTmp[0])
			actorShortName = actorEntryTmp[1]
			actorLocalQuoteId = audHashId - (actorId * 10000)
			return (actorId, actorShortName, actorLocalQuoteId)
	return (actorId, actorShortName, actorLocalQuoteId)

# Aux. Ensure existence of output directory
def ensure_dir(directory):
	try:
		os.makedirs(directory)
	except OSError as e:
		if e.errno != errno.EEXIST:
			raise

#
# Reading in the INPUT TLK files and checking all the AUD file properties
#
def inputTLKsExport(inputTLKpath, outputWAVpath):
	# try to open all TLK file entries from SUPPORTED_TLK_INPUT_FILES
	# then per TLK file
	#	create an output folder in the OUTPUT PATH named TLK## for the 1, 2, 3 TLK and TLKSPCHSFX for the SPCHSFX.TLK
	#	printout:
	#		total entries
	#		total data size
	#		and per entry the
	#				fileID
	#				segment offset
	#				file size
	print "[Info] Checking in %s for TLK files to export to %s" % (inputTLKpath, outputWAVpath)
	inputTLKFilesFound = []
	# breaking after first for loop yields only the top directory files, which is what we want
	for (dirpath, dirnames, filenames) in walk(inputTLKpath):
		for filename in filenames:
			for tlkTuple in SUPPORTED_TLK_INPUT_FILES:
				if filename.upper() == tlkTuple[0]:
					inputTLKFilesFound.append(tlkTuple)
		break
	for tmpTLKfileTuple in inputTLKFilesFound:
		if gTraceModeEnabled:
			print "[Info] Found TLK: %s" % ('"' + inputTLKpath + tmpTLKfileTuple[0] + '"')
		errorFound = False
		inTLKFile = None
		#
		# Create output folder if not exists at output path
		if gTraceModeEnabled:
			print "Ensuring output directory %s" % (os.path.join(outputWAVpath, tmpTLKfileTuple[1] ))
		ensure_dir(os.path.join(outputWAVpath, tmpTLKfileTuple[1] ) )
		try:
			inTLKFile = open(os.path.join(inputTLKpath,tmpTLKfileTuple[0]), 'rb')
		except:
			errorFound = True
			print "[Error] Unexpected event:", sys.exc_info()[0]
			raise
		if not errorFound:
			tmpBuff = inTLKFile.read(2)
			# H: unsigned short (2 bytes) followed by I: unsigned int (4 bytes)
			tlkFileEntriesNumTuple = struct.unpack('H', tmpBuff)
			numOfTREEntriesToExtract = tlkFileEntriesNumTuple[0]
			tmpBuff = inTLKFile.read(4)
			tlkFileDataSegmentSizeTuple = struct.unpack('I', tmpBuff)
			allTlkFileSize = tlkFileDataSegmentSizeTuple[0]
			inTLKFile.seek(0, 2) # go to file end
			allActualBytesInMixFile = inTLKFile.tell()
			inTLKFile.seek(6, 0) # go to start of table of TLK file entries (right after the 6 bytes header)
			# 2 + 4 = 6 bytes short MIX header
			# 12 bytes per TLK entry in entries table
			# quick size validation
			if gTraceModeEnabled:
				print "[Debug] Entries: %d, Data segment size: %d bytes" % (numOfTREEntriesToExtract, allTlkFileSize)
			if allActualBytesInMixFile != 2 + 4 + 12 * numOfTREEntriesToExtract + allTlkFileSize:
				print "[Error] TLK file size mismatch with reported size in header for %s!" % (tmpTLKfileTuple[0])
			else:
				#
				# 12 bytes per entry
				#		4 bytes: ID
				#		4 bytes: Offset in data segment
				#		4 bytes: Size of data
				#
				for i in range(0, numOfTREEntriesToExtract):
					inTLKFile.seek(2 + 4 + 12*i)
					tmpBuff = inTLKFile.read(4)
					tmpRdTuple = struct.unpack('I', tmpBuff)
					idOfAUDEntry = tmpRdTuple[0]
					tmpBuff = inTLKFile.read(4)
					tmpRdTuple = struct.unpack('I', tmpBuff)
					offsetOfAUDEntry = tmpRdTuple[0]
					tmpBuff = inTLKFile.read(4)
					tmpRdTuple = struct.unpack('I', tmpBuff)
					sizeOfAUDEntry = tmpRdTuple[0]
					if gTraceModeEnabled:
						print "[Debug] Entry: %s, offset: %s, Data segment size: %s bytes" % (''.join('{:08X}'.format(idOfAUDEntry)), ''.join('{:08X}'.format(offsetOfAUDEntry)),''.join('{:08X}'.format(sizeOfAUDEntry)))
					#
					# put file in AUD object
					# do we need AUD decode?
					# create WAV from AUD
					# write WAV to appropriate output folder
					# Figure out proper naming for file
					# then:
					#	AudFile aud;
					#	aud.loadAudFile(fs); (fs is file stream)
					#	aud.export_as_wav(fs, offset, offset + int(sizeof(AudHeader)) + aud.header().size_in, target);
					#
					#
					inTLKFile.seek(2 + 4 + 12*numOfTREEntriesToExtract + offsetOfAUDEntry)
					if(offsetOfAUDEntry + sizeOfAUDEntry > allTlkFileSize):
						print "[Error] audio file (AUD) file size mismatch with reported size in entry header!"
					else:
						targetSimpleAudFileName = ''.join('{:08X}'.format(idOfAUDEntry)).upper()+'.AUD'	
						audFileBuffer = inTLKFile.read(sizeOfAUDEntry)
						if (len(audFileBuffer) == sizeOfAUDEntry):
						# load audio file (AUD) file
							thisAudFile = audFile(gTraceModeEnabled)
							if (thisAudFile.loadAudFile(audFileBuffer, allTlkFileSize, targetSimpleAudFileName)):
								if gTraceModeEnabled:
									print "[Debug] Audio file (AUD) file %s was loaded successfully!" % (targetSimpleAudFileName)
								# find
								# print "[Debug] Emulating Wav write to appropriate folder..."
								(actorID, actorSName, localQuoteId) = getActorShortNameAndLocalQuoteIdByAUDHashID(idOfAUDEntry)
								targetSimpleWavFileName = actorSName + '_' + str(localQuoteId).zfill(4) + '_' + ''.join('{:08X}'.format(idOfAUDEntry)).upper()+'.WAV'
								#print os.path.join(outputWAVpath, tmpTLKfileTuple[1], targetSimpleWavFileName)
								if not os.path.isfile(os.path.join(outputWAVpath, tmpTLKfileTuple[1], targetSimpleWavFileName) ):
									thisAudFile.export_as_wav(audFileBuffer, os.path.join(outputWAVpath, tmpTLKfileTuple[1], targetSimpleWavFileName) )
								else:
									if gTraceModeEnabled:
										print "[Info] Output file %s already exists. Skipping..." % (os.path.join(outputWAVpath, tmpTLKfileTuple[1], targetSimpleWavFileName))
							else:
								print "[Error] while loading audio file (AUD) %s!" % (targetSimpleAudFileName)
						else:
							print "[Error] while reading audio file (AUD) file %s into mem buffer" % (targetSimpleAudFileName)
							#print "[Error] while reading audio file (AUD) file %s into mem buffer" % (''.join('{:08X}'.format(idOfMIXEntry)))
			inTLKFile.close()


	# SYS EXIT IS HERE ONLY FOR DEBUG PURPOSES OF PARSING TLK FILES - SHOULD BE COMMENTED OUT NORMALLY
	# sys.exit(0)
	return

def inputMIXExtractTREs(inputMIXpath, excelOutBook = None):
	print "[Info] Checking in %s for MIX files to extract Text Resources (TR%ss) from..." % (inputMIXpath, gActiveLanguageDescriptionCodeTuple[1])
	inputMIXFilesFound = []
	# breaking after first for loop yields only the top directory files, which is what we want
	for (dirpath, dirnames, filenames) in walk(inputMIXpath):
		for filename in filenames:
			for mixFileName in SUPPORTED_MIX_INPUT_FILES:
				if filename.upper() == mixFileName:
					inputMIXFilesFound.append(mixFileName)
		break
	for tmpMIXfileName in inputMIXFilesFound:
		if gTraceModeEnabled:
			print "[Info] Found MIX file: %s" % ('"' + tmpMIXfileName + '"')
		errorFound = False
		inMIXFile = None
		#
		try:
			inMIXFile = open(os.path.join(inputMIXpath,tmpMIXfileName), 'rb')
		except:
			errorFound = True
			print "[Error] Unexpected event:", sys.exc_info()[0]
			raise
		if not errorFound:
			totalTREs = 0
			tmpBuff = inMIXFile.read(2)
			# H: unsigned short (2 bytes) followed by I: unsigned int (4 bytes)
			mixFileEntriesNumTuple = struct.unpack('H', tmpBuff)
			numOfTREEntriesToExtract = mixFileEntriesNumTuple[0]
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
				print "[Debug] Entries: %d, Data segment size: %d bytes" % (numOfTREEntriesToExtract, allMixFileSize)
			if allActualBytesInMixFile != 2 + 4 + 12 * numOfTREEntriesToExtract + allMixFileSize:
				print "[Error] MIX file size mismatch with reported size in header for %s!" % (tmpMIXfileName)
			else:
				#
				# 12 bytes per entry
				#		4 bytes: ID
				#		4 bytes: Offset in data segment
				#		4 bytes: Size of data
				#
				for i in range(0, numOfTREEntriesToExtract):
					foundTREFile = False
					currTreFileName = 'UNKNOWN.TR%s' % (gActiveLanguageDescriptionCodeTuple[1])
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

					for suppTREFileName in SUPPORTED_EXPORTED_TRx_FILES:
						suppTREFileName = suppTREFileName + gActiveLanguageDescriptionCodeTuple[1]
						if(idOfMIXEntry == calculateFoldHash(suppTREFileName)):
							foundTREFile = True
							currTreFileName = suppTREFileName
							break

					if (foundTREFile == True):
						if gTraceModeEnabled:
							print "[Debug] Entry Name: %s, Entry ID: %s, offset: %s, Data segment size: %s bytes" % (currTreFileName, ''.join('{:08X}'.format(idOfMIXEntry)), ''.join('{:08X}'.format(offsetOfMIXEntry)),''.join('{:08X}'.format(sizeOfMIXEntry)))
						#
						# IF TRE FILE:
						# put file in TRE object
						#
						#
						inMIXFile.seek(2 + 4 + 12*numOfTREEntriesToExtract + offsetOfMIXEntry)
						if(offsetOfMIXEntry + sizeOfMIXEntry > allMixFileSize):
							print "[Error] TR%s file size mismatch with reported size in entry header!" % (gActiveLanguageDescriptionCodeTuple[1])
						else:
							treFileBuffer = inMIXFile.read(sizeOfMIXEntry)
							if (len(treFileBuffer) == sizeOfMIXEntry):
							# load TRE file
								thisTreFile = treFile(gTraceModeEnabled)
								if (thisTreFile.loadTreFile(treFileBuffer, allMixFileSize, currTreFileName)):
									if gTraceModeEnabled:
										print "[Debug] TR%s file %s was loaded successfully!" % (gActiveLanguageDescriptionCodeTuple[1], currTreFileName)
									if excelOutBook != None:
										sh = excelOutBook.add_sheet(currTreFileName)
										n = 0 # keeps track of rows
										col1_name = 'Text Resource File: %s' % (currTreFileName)
										sh.write(n, 0, col1_name)
										# Second Row
										n = 1
										col1_name = 'TextId'
										col2_name = 'Text'
										sh.write(n, 0, col1_name)
										sh.write(n, 1, col2_name)
										n+=1
										for m, e1 in enumerate(thisTreFile.stringEntriesLst, n):
											sh.write(m, 0, e1[0])
											objStr = e1[1]
											#print type (objUTF8SafeStr) # the type is STR here
											# python strings are immutable (can't replace characters) but we have an issue with certain special characters in the ORIGINAL TRE (kiacred and endcred)
											# (they are out of their order from their proper order in windows-1252)
											# so we need to create a new string.
											# TODO: The following check could be streamlined if it could use semi-auto constructed glyph mappings per font per Text Resource file.
											# TODO: ^^ This could help support Russian versions -- but one the other hand maybe it's a bit over-engineering (?)											
											objUTF8SafeStr = ""
											for i in range(0, len(objStr)):
												if ( currTreFileName[:-1] == 'ERRORMSG.TR'): 
													# ERRORMSG.TRx uses different font than the others and there are conflicts, so we can't put it in the same checks with the other TRxs
													#
													# ENG version has no ERRORMSG exceptions
													# GER
													if (objStr[i] == '\xe4'):   # DEU - ERRORMSG.TRG
														objUTF8SafeStr += 'ä'
													elif (objStr[i] == '\xf6'): # DEU - ERRORMSG.TRG
														objUTF8SafeStr += 'ö'
													elif (objStr[i] == '\xfc'): # DEU - ERRORMSG.TRG
														objUTF8SafeStr += 'ü'
													# FRA
													elif (objStr[i] == '\x82'): # FRA - ERRORMSG.TRF
														objUTF8SafeStr += 'é'	 # this is identical to the KIA6PT mapping
													elif (objStr[i] == '\x85'): # FRA - ERRORMSG.TRF (also used in ITA ERRORMSG.TRI - same glyph)
														objUTF8SafeStr += 'à'	 # this is identical to the KIA6PT mapping
													elif (objStr[i] == '\x8a'): # FRA - ERRORMSG.TRF (also used in ITA ERRORMSG.TRI - same glyph)
														objUTF8SafeStr += 'è'    # this is identical to the KIA6PT mapping
													elif (objStr[i] == '\xe0'): # FRA - ERRORMSG.TRF
														objUTF8SafeStr += 'à'
													elif (objStr[i] == '\xe8'): # FRA - ERRORMSG.TRF
														objUTF8SafeStr += 'è'
													elif (objStr[i] == '\xe9'): # FRA - ERRORMSG.TRF (also used in ESP ERRORMSG.TRS - same glyph)
														objUTF8SafeStr += 'é'
													elif (objStr[i] == '\xea'): # FRA - ERRORMSG.TRF
														objUTF8SafeStr += 'ê'
													# ITA
													#elif (objStr[i] == '\x85'): # ITA - ERRORMSG.TRI [commented out: already covered in FRA check above]
													#	objUTF8SafeStr += 'à'	 # this is identical to the KIA6PT mapping
													#elif (objStr[i] == '\x8a'): # ITA - ERRORMSG.TRI [commented out: already covered in FRA check above]
													#	objUTF8SafeStr += 'è'    # this is identical to the KIA6PT mapping													
													elif (objStr[i] == '\x97'): # ITA - ERRORMSG.TRI
														objUTF8SafeStr += 'ù'    # this is identical to the KIA6PT mapping													
													# ESP
													elif (objStr[i] == '\xa2'): # ESP - ERRORMSG.TRS
														objUTF8SafeStr += 'ó' 	# this is identical to the KIA6PT mapping
													elif (objStr[i] == '\xe1'): # ESP - ERRORMSG.TRS
														objUTF8SafeStr += 'á'
													#elif (objStr[i] == '\xe9'): # ESP - ERRORMSG.TRS [commented out: already covered in FRA check above]
													#	objUTF8SafeStr += 'é'
													elif (objStr[i] == '\xed'): # ESP - ERRORMSG.TRS
														objUTF8SafeStr += 'í'
													elif (objStr[i] == '\xf3'): # ESP - ERRORMSG.TRS
														objUTF8SafeStr += 'ó'
													else:
														objUTF8SafeStr += objStr[i]
												else:
												# all the other TRx use the KIA6PT.FON
												# There could be variances of the KIA6PT.FON per Blade Runner version
												# TODO: For now, we assume that there aren't significant variances that warrant a more elaborate approach
													if (objStr[i] == '\x81'):	# EN, DEU, FRA, ITA, ESP
														objUTF8SafeStr += 'ü'
													elif (objStr[i] == '\x82'): # EN, DEU, FRA, ITA, ESP
														objUTF8SafeStr += 'é'
													## Extras (DEU):
													elif (objStr[i] == '\x84'): # DEU 
														objUTF8SafeStr += 'ä'	#
													elif (objStr[i] == '\x8e'): # DEU 
														objUTF8SafeStr += 'Ä'	#
													elif (objStr[i] == '\x94'): # DEU 
														objUTF8SafeStr += 'ö'	#
													elif (objStr[i] == '\x9a'): # DEU 
														objUTF8SafeStr += 'Ü'	#
													elif (objStr[i] == '\xe1'): # DEU (ENDCRED.TRG)
														objUTF8SafeStr += 'ß'	#
													## Extras (FRA):
													elif (objStr[i] == '\x85'): # FRA, ITA
														objUTF8SafeStr += 'à'	# re-used in ITA, same glyph
													elif (objStr[i] == '\x87'): # FRA
														objUTF8SafeStr += 'ç'
													elif (objStr[i] == '\x88'): # FRA
														objUTF8SafeStr += 'ê'
													elif (objStr[i] == '\x8a'): # FRA, ITA
														objUTF8SafeStr += 'è'	# re-used in ITA, same glyph
													elif (objStr[i] == '\x8b'): # FRA
														objUTF8SafeStr += 'ï'
													elif (objStr[i] == '\x8c'): # FRA
														objUTF8SafeStr += 'î'
													elif (objStr[i] == '\x93'): # FRA
														objUTF8SafeStr += 'ô'
													elif (objStr[i] == '\x96'): # FRA
														objUTF8SafeStr += 'û'
													elif (objStr[i] == '\x97'): # FRA, ITA
														objUTF8SafeStr += 'ù' # re-used in ITA, same glyph
													## Extras (ITA):
													#elif (objStr[i] == '\x85'): # ITA [commented out: already covered in FRA check above]
													#	objUTF8SafeStr += 'à'
													#elif (objStr[i] == '\x8a'): # ITA [commented out: already covered in FRA check above]
													#	objUTF8SafeStr += 'è'
													elif (objStr[i] == '\x8d'): # ITA
														objUTF8SafeStr += 'ì'
													#elif (objStr[i] == '\x97'): # ITA [commented out: already covered in FRA check above]
													#	objUTF8SafeStr += 'ù'
													# Extras (ESP):
													elif (objStr[i] == '\xa0'): # ESP
														objUTF8SafeStr += 'á'
													elif (objStr[i] == '\xa1'): # ESP
														objUTF8SafeStr += 'í'
													elif (objStr[i] == '\xa2'): # ESP
														objUTF8SafeStr += 'ó'
													elif (objStr[i] == '\xa3'): # ESP
														objUTF8SafeStr += 'ú'
													elif (objStr[i] == '\xa4'): # ESP
														objUTF8SafeStr += 'ñ'
													elif (objStr[i] == '\xa5'): # ESP
														objUTF8SafeStr += 'Ñ'
													elif (objStr[i] == '\xa8'): # ESP
														objUTF8SafeStr += '¿'
													elif (objStr[i] == '\xad'): # ESP
														objUTF8SafeStr += '¡'
													elif (objStr[i] == '\x90'): # ESP
														objUTF8SafeStr += 'É'
													else:
														objUTF8SafeStr += objStr[i]
											#objUTF8Safe = objUTF8Safe.replace('\x81',u'u') #'ü' # this does not work
											#objUTF8Safe = objUTF8Safe.replace('\x82',u'e') #'é' # this does not work
											try:
												objUTF8Unicode = unicode(objUTF8SafeStr, 'utf-8')
											except Exception as e:
												print '[Error] Failed to create unicode string: ' + str(e)
												objUTF8Unicode = unicode("???", 'utf-8')
											sh.write(m, 1, objUTF8Unicode)


									#for tupleIdString in thisTreFile.stringEntriesLst:
									#	#print "[Debug] Id: %d\t Text: %s" % (tupleIdString[0], tupleIdString[1])
									#	pass
									totalTREs =   totalTREs + 1
								else:
									print "[Error] while loading TR%s %s file!" % (gActiveLanguageDescriptionCodeTuple[1], currTreFileName)
							else:
								print "[Error] while reading TR%s file %s into mem buffer" % (gActiveLanguageDescriptionCodeTuple[1], currTreFileName)
								#print "[Error] while reading TR%s file %s into mem buffer" % (gActiveLanguageDescriptionCodeTuple[1], ''.join('{:08X}'.format(idOfMIXEntry)))
			inMIXFile.close()
			print "[Info] Total TR%ss processed: %d " % (gActiveLanguageDescriptionCodeTuple[1], totalTREs)
	return


#
# Creating the OUTPUT XLS file with one sheet named as the @param sheet with entries based on the list1 (wav files, without duplicates)
#
def outputXLS(filename, sheet, list1, parseTREResourcesAlso = False, mixInputFolderPath = ''):
	global gStringReplacementForRootFolderWithExportedFiles
	global gNumReplaceStartingCharacters
	book = xlwt.Workbook()
	sh = book.add_sheet(sheet)
# First Row
	n = 0      # keeps track of rows
#	 variables = [x, y, z]
#	 x_desc = 'Display'
#	 y_desc = 'Dominance'
#	 z_desc = 'Test'
#	 desc = [x_desc, y_desc, z_desc]
#
#
#	 #You may need to group the variables together
#	 #for n, (v_desc, v) in enumerate(zip(desc, variables)):
#	 for n, (v_desc, v) in enumerate(zip(desc, variables)):
#		 sh.write(n, 0, v_desc)
#		 sh.write(n, 1, v)
	col1_name = 'BladeRunnerTLK In-Game dialogue / voiceover quotes'
	sh.write(n, 0, col1_name)
# Second Row
	n = 1
	col1_name = 'Filename'
	col2_name = 'Quote'
	col3_name = 'By Actor'
	col4_name = 'Notes'
	col5_name = 'To Actor'
	col6_name = 'Resource'
	col7_name = 'ShortHandFileName'

	sh.write(n, 0, col1_name)
	sh.write(n, 1, col2_name)
	sh.write(n, 2, col3_name)
	sh.write(n, 3, col4_name)
	sh.write(n, 4, col5_name)
	sh.write(n, 5, col6_name)
	sh.write(n, 6, col7_name)

	n+=1

	for m, e1 in enumerate(list1, n):
		twoTokensOfRelDirnameAndFilenameXLS = e1.split('&', 2)
		if len(twoTokensOfRelDirnameAndFilenameXLS) == 3:
			fourTokensOfFilename = twoTokensOfRelDirnameAndFilenameXLS[0].split('#', 3)
			if len(fourTokensOfFilename) == 4:
				# fix rogue _ chars in 3rd token of filename (split at '_')
				tmpAUDFileName = fourTokensOfFilename[0] + '-' + fourTokensOfFilename[1] + '.AUD'
				#ActorId-QuoteId.AUD
				sh.write(m, 0, tmpAUDFileName)
				twoTokensOfQuote = fourTokensOfFilename[2].split('-', 1)
				if len(twoTokensOfQuote) == 2:
					#Quote
					sh.write(m, 1, twoTokensOfQuote[1])
				else:
					#Quote
					sh.write(m, 1, fourTokensOfFilename[2])
					#Notes
					sh.write(m, 3, 'TODO')
				#byActor
				sh.write(m, 2, fourTokensOfFilename[3])
				#ResourceFolder
				sh.write(m, 5, twoTokensOfRelDirnameAndFilenameXLS[1])
				#ShortHandFileName
				tmpActorShortHand = getActorShortNameById(fourTokensOfFilename[0])
				shortHandFileName = tmpActorShortHand + '_' + fourTokensOfFilename[1] + '_' + fourTokensOfFilename[2] + '.WAV'
				# real path of filename
				realPathOfFileNameToLink = twoTokensOfRelDirnameAndFilenameXLS[2]
				# checks if not empty
				if gStringReplacementForRootFolderWithExportedFiles and gNumReplaceStartingCharacters > 0:
					realPathOfFileNameToLink = realPathOfFileNameToLink.replace(realPathOfFileNameToLink[:gNumReplaceStartingCharacters], gStringReplacementForRootFolderWithExportedFiles)

				#works in Linux + Libreoffice
				# also works in Windows + LibreOffice (run from msys) -- tried something like:
				#	python sortBladeRunnerWavs.py -p /g/WORKSPACE/BladeRunnerWorkspace/br-mixer-master/data/WAV -m "G:/WORKSPACE/BladeRunnerWorkspace/br-mixer-master/data/WAV"
				#TODO put real full path for each file as FILE URL, and real (or approximate shorthand file name as alias)
				hyperlinkAudioFormula = 'HYPERLINK("file://%s","%s")' % (realPathOfFileNameToLink, shortHandFileName)
				sh.write(m, 6, Formula(hyperlinkAudioFormula))
			else:
				sh.write(m, 0, e1)
				#Notes
				sh.write(m, 3, 'error')
		else:
			sh.write(m, 0, e1)
			#Notes
			sh.write(m, 3, 'error')


		# works for filenames where some rogue greek characters exist
		#sh.write(m, 0, str.decode("%s" % e1, 'utf-8'))

#	 for m, e2 in enumerate(list2, n+1):
#		 sh.write(m, 1, e2)

	if parseTREResourcesAlso == True and mixInputFolderPath != '':
		inputMIXExtractTREs(mixInputFolderPath, book)
		# TODO add sheets
		# TODO handle special string characters (to UTF-8)
	try:
		book.save(filename)
		print "[Info] Done."
	except Exception as e:
		print "[Error] Could not save the output Excel file (maybe it's open?). " + str(e)

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
def main(argsCL):
	# TODO parse arguments using argparse? https://docs.python.org/3/library/argparse.html#module-argparse
	global gTraceModeEnabled
	global gStringReplacementForRootFolderWithExportedFiles
	global gNumReplaceStartingCharacters
	global gActiveLanguageDescriptionCodeTuple 
	global gWavFiles
	global gWavFilesNoDups
	
	gTraceModeEnabled = False
	gActiveLanguageDescriptionCodeTuple = DEFAULT_LANG_DESC_CODE

	
	pathToActorNamesTxt = ""
	candidateLangDescriptionTxt = ""

	TMProotFolderWithExportedFiles = ""
	TMProotFolderWithInputTLKFiles = ""

	exportWavFilesMode = False
	extractTreFilesMode = False

	invalidSyntax = False
	print "Running %s (%s)..." % (APP_NAME_SPACED, APP_VERSION)
#	 print "Len of sysargv = %s" % (len(argsCL))
	if len(argsCL) == 2:
		if(argsCL[1] == '--help'or argsCL[1] == '-h'):
			print "%s %s supports Westwood's Blade Runner PC Game (1997)." % (APP_NAME_SPACED, APP_VERSION)
			print APP_SHORT_DESC
			print "Created by Praetorian of the classic adventures in Greek team."
			print "Always keep backups!"
			print "--------------------"
			print "%s takes has one mandatory argument, ie. the folder of the exported WAV files:" % (APP_WRAPPER_NAME)
			print "Valid syntax: %s -op folderpath_for_exported_wav_Files [-ip folderpath_for_TLK_Files] [-ian path_to_actornames_txt] [-m stringPathToReplaceFolderpathInExcelLinks] [-ld gameInputLanguageDescription] [-xwav] [-xtre] [--trace]" % (APP_WRAPPER_NAME)
			print "The -op switch has an argument that is the path for exported WAV files folder. The -op switch is REQUIRED always."
			print "The -ip switch has an argument that is the path for the input (TLK or MIX) files folder (can be the same as the Blade Runner installation folder)."
			print "The -ian switch is followed by the path to actornames.txt, if it's not in the current working directory."
			print "The -m switch has an argument that is a replacement string for the path to the folder of exported WAV files which will be used as a prefix for the links in the output XLS file."
			print "The -ld switch has an argument that is the language description of the original game files that you use as input."
			printInfoMessageForLanguageSelectionSyntax()
			print "The -xwav switch enables the WAV audio export mode from the TLK files. It requires an INPUT path to be set with the -ip switch."
			print "The -xtre switch enables the TRx parsing mode from the original MIX files. It requires an INPUT path to be set with the -ip switch."
			print "The --trace switch enables more debug messages being printed during execution."
			print "--------------------"
			print "If the app finishes successfully, a file named %s-(language)%s will be created in the current working folder." % (OUTPUT_XLS_FILENAME, OUTPUT_XLS_FILENAME_EXT)
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
				if (argsCL[i] == '-op'):
					TMProotFolderWithExportedFiles = argsCL[i+1]
					gNumReplaceStartingCharacters = len(TMProotFolderWithExportedFiles)
				elif (argsCL[i] == '-ip'):
					TMProotFolderWithInputTLKFiles = argsCL[i+1]
				elif (argsCL[i] == '-m'):
					gStringReplacementForRootFolderWithExportedFiles = argsCL[i+1]
				elif (argsCL[i] == '-ian'):
					pathToActorNamesTxt = argsCL[i+1]
				elif (argsCL[i] == '-ld'):
					candidateLangDescriptionTxt = argsCL[i+1]
			elif (argsCL[i] == '-xwav'):
				print "[Info] Export WAVs from TLK files mode enabled."
				exportWavFilesMode = True
			elif (argsCL[i] == '-xtre'):
				print "[Info] Extract Text Resources (TRx) mode enabled."
				extractTreFilesMode = True
			elif argsCL[i] == '--trace':
				print "[Info] Trace mode enabled (more debug messages)."
				gTraceModeEnabled = True
		if not TMProotFolderWithExportedFiles: # this argument is mandatory
			invalidSyntax = True

		if (not invalidSyntax) and (exportWavFilesMode == True or extractTreFilesMode == True) and (TMProotFolderWithInputTLKFiles == ''):
			invalidSyntax = True
		
		gActiveLanguageDescriptionCodeTuple = getLanguageDescCodeTuple(candidateLangDescriptionTxt)
		if (not invalidSyntax) and gActiveLanguageDescriptionCodeTuple is None:	
			print "[Error] Invalid language code was specified"
			printInfoMessageForLanguageSelectionSyntax()
			invalidSyntax = True
			
		if not invalidSyntax:
			print "[Info] Game Language Selected: %s (%s)" % (gActiveLanguageDescriptionCodeTuple[0], gActiveLanguageDescriptionCodeTuple[2])
			# parse Actors files:
			initActorPropertyEntries(pathToActorNamesTxt)
#			 for actorEntryTmp in gActorPropertyEntries:
#				  print "[Debug] Found actor: %s %s %s" % (actorEntryTmp[0], actorEntryTmp[1], actorEntryTmp[2])
			#
			# Checking for the optional case of parsing the input TLK files to export to WAV
			#
			if TMProotFolderWithInputTLKFiles != '':
				if (exportWavFilesMode == True):
					inputTLKsExport(TMProotFolderWithInputTLKFiles, TMProotFolderWithExportedFiles)
				#if (extractTreFilesMode == True):
				#	inputMIXExtractTREs(TMProotFolderWithInputTLKFiles)
			#
			# Parsing the exported WAV files
			#
			print "[Info] Parsing the exported WAV audio files. Please wait (this could take a while)..."
			for (dirpath, dirnames, filenames) in walk(TMProotFolderWithExportedFiles):
				 for nameIdx, nameTmp in enumerate(filenames):
					  relDirName = ''
# os.path.split would Split the pathname path into a pair, (head, tail) where tail is the last pathname component and head is everything leading up to that. The tail part will never contain a slash
					  pathTokens = dirpath.split(os.path.sep)
					  for pTokenTmp in pathTokens:
						   if pTokenTmp.find("TLK") != -1:
								relDirName = pTokenTmp
#					   print os.path.dirname(dirpath)
#					   print os.path.abspath(os.path.join(os.path.join(dirpath, nameTmp), os.pardir))
					  filenames[nameIdx] = filenames[nameIdx] +'&' + relDirName + '&' + os.path.join(dirpath, nameTmp)
				 gWavFiles.extend(filenames)
#				 break
			for fileIdx, filenameTmp in enumerate(gWavFiles):
				twoTokensOfFilenameAndRelDirname = filenameTmp.split('&', 1)
				if len(twoTokensOfFilenameAndRelDirname) != 2:
					print "[Error] in filename and rel dirname split: %s" % (filenameTmp)
					sys.exit(0)
				twoTokensOfFilenameForExt = twoTokensOfFilenameAndRelDirname[0].split('.', 1)
				if len(twoTokensOfFilenameForExt) == 2:
					if twoTokensOfFilenameForExt[1] != 'WAV' and  twoTokensOfFilenameForExt[1] != 'wav':
						print "[Error] in proper extension (not WAV): %s" % (twoTokensOfFilenameAndRelDirname[0])
						sys.exit(0)
				else:
					print "[Error] in extension split: %s" % (twoTokensOfFilenameAndRelDirname[0])
					sys.exit(0)
				#remove WAV extension here
#				 filenameTmp =	twoTokensOfFilenameAndRelDirname[0] + '&' + twoTokensOfFilenameForExt[0]
#				 print "[Debug] Found %s" % (filenameTmp)

				threeTokensOfFilename = twoTokensOfFilenameForExt[0].split('_', 2)
				if len(threeTokensOfFilename) == 3:
					# fix rogue _ chars in 3rd token of filename (split at '_')
					threeTokensOfFilename[2] = threeTokensOfFilename[2].replace("_", "-")
					# Replace first token
					# replace actor name shorthand with ActorID in first part
					tmpActorId = getActorIdByShortName(threeTokensOfFilename[0])
					tmpActorFullName = ''
					if(tmpActorId != '' and tmpActorId is not None):
						tmpActorFullName = getActorFullNameById(tmpActorId)
						if(tmpActorFullName != '' and tmpActorFullName is not None):
							threeTokensOfFilename[0] = tmpActorId.zfill(2)
							threeTokensOfFilename.append(tmpActorFullName)
						else:
					#fatal error if something cannot convert to spot it immediately
							print "[Error] in actorIdMatch match: %s %s" % (tmpActorId, twoTokensOfFilenameForExt[0])
							sys.exit(0)
					else:
					#fatal error if something cannot convert to spot it immediately
						print "[Error] in shorthand match: %s %s" % (threeTokensOfFilename[0], twoTokensOfFilenameForExt[0])
						sys.exit(0)
#
#
#					  foundMatchForActorShortHand = False
#					  for actorEntryTmp in gActorPropertyEntries:
#						   if actorEntryTmp[1] == threeTokensOfFilename[0]:
#								 threeTokensOfFilename[0] = actorEntryTmp[0].zfill(2)
#								 threeTokensOfFilename.append(actorEntryTmp[2])
#								 foundMatchForActorShortHand = True
#								 break
					 # end of replace actor name shorthand
					twoTokensOfFilenameForExt[0] = '#'.join(threeTokensOfFilename)
					filenameTmp =  twoTokensOfFilenameForExt[0] + '&' + twoTokensOfFilenameAndRelDirname[1]
					gWavFiles[fileIdx] = filenameTmp
				else:
					print "[Error] in splitting tokens on _: %s" % (filenameTmp)
					sys.exit(0)
			#sort in-place
			#
			#
			gWavFiles.sort()
#			  #
#			  #
#			  #	 Code for renaming non conforming filenames - just to be consistent in file naming
#			  # TO BE RUN ONCE FOR CONFORMANCE. No NEED TO Re-RUN
#			  # If its run though, comment this section and re-run the tool to get proper links in Excel file
#			  #
#			  for filenameSrcTmp in gWavFiles:
#				  # get real full path from last token when split at &
#				  # create target full path from the parentdir of last token and the current state of first 3 tokens when splitting at '#'
#				  # report mismatch
#				  # print (BUT DON'T PROCEED AT THIS POINT) what you would rename to what.
#				  threeTokensOfFilenameAndRelDirname = filenameSrcTmp.split('&', 2)
#				  currentSrcFullPath = threeTokensOfFilenameAndRelDirname[2]
#				  fourTokensOfTargetFileName = threeTokensOfFilenameAndRelDirname[0].split('#', 3)
#				  tmpActorShortHand = getActorShortNameById(fourTokensOfTargetFileName[0])
#				  targetFileName = tmpActorShortHand + '_' + fourTokensOfTargetFileName[1] + '_' + fourTokensOfTargetFileName[2] + '.WAV'
#				  # os.path.split would Split the pathname path into a pair, (head, tail) where tail is the last pathname component and head is everything leading up to that. The tail part will never contain a slash
#				  (srcParentDir, srcTail) = os.path.split(currentSrcFullPath)
#				  targetFullPath = os.path.join(srcParentDir, targetFileName)
#				  # os.rename(src, dst)
#				  if(currentSrcFullPath != targetFullPath):
#					 print currentSrcFullPath
#					 print targetFullPath
#					 os.rename(currentSrcFullPath, targetFullPath)
			#
			# END OF: Code for renaming non conforming filenames - just to be consistent in file naming
			#
			#
			for filenameSrcTmp in gWavFiles:
				duplicateFound = False
#				  print "[Debug]Converted %s" % (filenameSrcTmp)
			 # Weed out duplicates by copying to another table (quick and dirty)
				twoTokensOfRelDirnameAndFilenameSrc = filenameSrcTmp.split('&', 2)
				tmpRelDirNameSrc = twoTokensOfRelDirnameAndFilenameSrc[1]
				threeTokensOfQuoteFilenameSrc = twoTokensOfRelDirnameAndFilenameSrc[0].split('#', 2)
				#concatenate actorID and quoteID for search key
				keyForDuplicateSearchSrc =	threeTokensOfQuoteFilenameSrc[0] + threeTokensOfQuoteFilenameSrc[1]
				for fileTargIdx, filenameTargTmp in enumerate(gWavFilesNoDups):
					twoTokensOfRelDirnameAndFilenameTarg = filenameTargTmp.split('&', 2)
					tmpRelDirNameTarg = twoTokensOfRelDirnameAndFilenameTarg[1]
					threeTokensOfQuoteFilenameTarg = twoTokensOfRelDirnameAndFilenameTarg[0].split('#', 2)
					#concatenate actorID and quoteID for search key
					keyForDuplicateSearchTarg =	 threeTokensOfQuoteFilenameTarg[0] + threeTokensOfQuoteFilenameTarg[1]
					if(keyForDuplicateSearchSrc == keyForDuplicateSearchTarg):
						#print "[Debug] Found duplicate %s" % (filenameSrcTmp)
						duplicateFound = True
						gWavFilesNoDups[fileTargIdx] = twoTokensOfRelDirnameAndFilenameTarg[0] + '&' +  tmpRelDirNameSrc + ',' + tmpRelDirNameTarg + '&' + twoTokensOfRelDirnameAndFilenameTarg[2]
						break
				if(duplicateFound == False):
					gWavFilesNoDups.append(filenameSrcTmp)
#			 for filenameSrcTmp in gWavFilesNoDups:
#				 print "[Debug] Unique %s" % (filenameSrcTmp)
			constructedOutputFilename = "%s-%s%s" % (OUTPUT_XLS_FILENAME, gActiveLanguageDescriptionCodeTuple[2], OUTPUT_XLS_FILENAME_EXT)
			print "[Info] Creating output excel %s file..." % (constructedOutputFilename)
			outputXLS(constructedOutputFilename, OUTPUT_XLS_QUOTES_SHEET + gActiveLanguageDescriptionCodeTuple[1], gWavFilesNoDups, extractTreFilesMode, TMProotFolderWithInputTLKFiles)
	else:
		invalidSyntax = True

	if invalidSyntax == True:
		print "[Error] Invalid syntax!\n Try: \n %s --help for more info \n %s --version for version info " % (APP_WRAPPER_NAME, APP_WRAPPER_NAME)
		print "Valid syntax: %s -op folderpath_for_exported_wav_Files [-ip folderpath_for_TLK_Files] [-ian path_to_actornames_txt] [-m stringPathToReplaceFolderpathInExcelLinks] [-ld gameInputLanguageDescription] [-xwav] [-xtre] [--trace]" % (APP_WRAPPER_NAME)
		print "\nDetected arguments:"
		tmpi = 0
		for tmpArg in argsCL:
			if tmpi==0: #skip first argument
				tmpi+=1
				continue
			print "Argument: %s" % (tmpArg)
			tmpi+=1

# 00_0000 -- DealsInInsects					dupl TLK01, TLK0A
# 00_0510 -- ThinkingOfChangingJobs-Leon	dupl TLK02, TLK03
# 00-8520 -- WhatDoYouKnow					dupl TLK01, TLK0A

# Total unique quotes seems to be 5495!
# TODO rename files in folders to conform to the underscore '_' and '-' format (a few don't -- let's have them all conforming!)
#
# #########################
#
if __name__ == '__main__':
	main(sys.argv[0:])
else:
	## debug
	#print '[Debug] %s was imported from another module' % (APP_WRAPPER_NAME)
	pass
