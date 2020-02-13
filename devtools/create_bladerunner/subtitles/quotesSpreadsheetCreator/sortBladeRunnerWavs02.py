#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Created by Praetorian (ShadowNate) for Classic Adventures in Greek
# classic.adventures.in.greek@gmail.com
#
# DONE Add code and switch option: to get the blade runner installation directory as input, then find the TLK files and export them with proper naming
# DONE fix proper names for sheets as per latest code changes
#
osLibFound = False
sysLibFound = False
shutilLibFound = False
xlwtLibFound = False
csvLibFound = False

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

if 	(not osLibFound) \
	or (not sysLibFound) \
	or (not shutilLibFound) \
	or (not xlwtLibFound) \
	or (not csvLibFound):
	sys.stdout.write("[Error] Errors were found when trying to import required python libraries\n")
	sys.exit(1)


from os import walk, errno, path

pathToParent = os.path.join(os.path.dirname(os.path.abspath(__file__)), os.pardir)
pathToCommon = os.path.join(pathToParent, "common")
sys.path.append(pathToCommon)

from xlwt import *
from audFileLib import *
from treFileLib import *
from pogoTextResource import *
from devCommentaryText import *
from subtlsVersTextResource import *
from vqasTextResource import *

# encoding=utf8
#reload(sys)
#sys.setdefaultencoding('utf8')

COMPANY_EMAIL = "classic.adventures.in.greek@gmail.com"
APP_VERSION = "1.60"
APP_NAME = "sortBladeRunnerWavs"
APP_WRAPPER_NAME = "quotesSpreadsheetCreator.py"
APP_NAME_SPACED = "Blade Runner Transcript Excel Creator (bare bones)"
APP_SHORT_DESC = "* Create an Excel (.XLS) for transcribing Blade Runner.\n* (Optional) Extract TRx (x is the language code) files as additional sheets in the Excel\n* (Optional) Export speech files from the game's TLK resources to WAV format."

OUTPUT_XLS_FILENAME = 'out'
OUTPUT_XLS_FILENAME_EXT = '.xls'
OUTPUT_XLS_QUOTES_SHEET = 'INGQUO_' # needs suffix x.TRx where x is the language code

SUPPORTED_TLK_INPUT_FILES = [('1.TLK', 'TLK01'), ('2.TLK', 'TLK02'), ('3.TLK', 'TLK03'), ('A.TLK', 'TLK0A'), ('SPCHSFX.TLK', 'TLKSPCHSFX')]
SUPPORTED_MIX_INPUT_FOR_EXTRA_SPEECH_EXPORT_FILES = [('SFX.MIX', 'SFX')]
SUPPORTED_EXPORTED_AUD_FROM_MIX_FILES = [('COLONY.AUD', 'SFX.MIX'), \
										('MA04VO1A.AUD', 'SFX.MIX'), \
										('CT01VO1A.AUD', 'SFX.MIX'), \
										('HC01VO1A.AUD', 'SFX.MIX'), \
										('67_0470R.AUD', 'SFX.MIX'), \
										('67_0480R.AUD', 'SFX.MIX'), \
										('67_0500R.AUD', 'SFX.MIX'), \
										('67_0540R.AUD', 'SFX.MIX'), \
										('67_0560R.AUD', 'SFX.MIX'), \
										('67_0870R.AUD', 'SFX.MIX'), \
										('67_0880R.AUD', 'SFX.MIX'), \
										('67_0900R.AUD', 'SFX.MIX'),\
										('67_0940R.AUD', 'SFX.MIX'), \
										('67_0960R.AUD', 'SFX.MIX'), \
										('67_1070R.AUD', 'SFX.MIX'), \
										('67_1080R.AUD', 'SFX.MIX'), \
										('67_1100R.AUD', 'SFX.MIX'), \
										('67_1140R.AUD', 'SFX.MIX'), \
										('67_1160R.AUD', 'SFX.MIX') \
										]

SUPPORTED_MIX_INPUT_FOR_TRX_EXPORT_FILES = ['STARTUP.MIX']
# 15 TRx files
SUPPORTED_EXPORTED_TRx_FILES = ['CLUES.TR','ACTORS.TR','CRIMES.TR','CLUETYPE.TR','KIA.TR','SPINDEST.TR','VK.TR','OPTIONS.TR','DLGMENU.TR','ENDCRED.TR','HELP.TR','SCORERS.TR','KIACRED.TR','ERRORMSG.TR','AUTOSAVE.TR']
SUPPORTED_PLACEHOLDER_VQA_ENGLISH_FILES = [
	('WSTLGO_', 'Westwood Studios Partnership Intro', '19400 ms, 291 frames'),
	('BRLOGO_', 'Blade Runner Logo', '6000 ms, 90 frames')]
SUPPORTED_PLACEHOLDER_VQA_LOCALIZED_FILES = [
	('INTRO_',  'Act 1 Intro - Prologue', '373267 ms, 5599 frames'),
	('MW_A_',   'Act 2 Intro', '133134 ms, 1997 frames'),
	('MW_B01_', 'Act 3 Intro - Start', '6733.367 ms, 101 frames'),
	('MW_B02_', 'Act 3 Intro - Lucy is Replicant', '16000.080 ms, 240 frames'),
	('MW_B03_', 'Act 3 Intro - Dektora is Replicant', '11000.055 ms, 165 frames'),
	('MW_B04_', 'Act 3 Intro - Lucy and Dektora are Human', '18000.090 ms, 270 frames'),
	('MW_B05_', 'Act 3 Intro - End', '107133.869 ms, 1607 frames'),
	('INTRGT_', 'Interrogation scene - Baker, Holloway, McCoy', '158600 ms, 2379 frames'),
	('MW_C01_', 'Act 4 Intro - Start', '121533.333 ms, 1823 frames'),
	('MW_C02_', 'Act 4 Intro - Clovis with Incept Photo', '22066.667 ms, 331 frames'),
	('MW_C03_', 'Act 4 Intro - Clovis without Incept Photo', '22066.667 ms, 331 frames'),
	('MW_D_',   'Act 5 Intro', '160000 ms, 2400 frames'),
	('END01A_', 'Underground Ending - Lucy Human', '31466.667 ms, 472 frames'),
	('END01B_', 'Underground Ending - Lucy with DNA', '31466.667 ms, 472 frames'),
	('END01C_', 'Underground Ending - Lucy not enough DNA', '31466.667 ms, 472 frames'),
	('END01D_', 'Underground Ending - Dektora Human', '31466.667 ms, 472 frames'),
	('END01E_', 'Underground Ending - Dektora with DNA', '31466.667 ms, 472 frames'),
	('END01F_', 'Underground Ending - Dektora, not enough DNA', '31466.667 ms, 472 frames'),
	('END03_',  'Underground Ending - McCoy alone', '26466.667 ms, 397 frames'),
	('END04A_', 'Moonbus Ending - Start', '7933.333 ms, 119 frames'),
	('END04B_', 'Moonbus Ending - With Lucy', '6533.333 ms, 98 frames'),
	('END04C_', 'Moonbus Ending - With Dektora', '7733.333 ms, 116 frames'),
	('END06_',  'Kipple Ending - With Steele', '36400 ms, 546 frames'),
	('TB_FLY_',  'Fly-through to Tyrell Building', '13533.333 ms, 203 frames')
	]
SUPPORTED_SPECIAL_POGO_FILE = 'POGO.TR'
SUPPORTED_DIALOGUE_VERSION_SHEET = 'SBTLVERS.TRE'

# v0.85: Russian code (RU_RUS) now corresponds to 'E' suffix instead of 'R' since the unofficial Russian version supported uses the English resources without renaming them, and this is how the ScummVM engine handles that version currently.
SUPPORTED_LANGUAGES_DESCRIPTION_CODE_TLIST = [('EN_ANY', 'E', 'English'), ('DE_DEU', 'G', 'German'), ('FR_FRA', 'F', 'French'), ('IT_ITA', 'I', 'Italian'), ('ES_ESP', 'S', 'Spanish'), ('RU_RUS', 'E', 'Russian')]
DEFAULT_LANG_DESC_CODE = SUPPORTED_LANGUAGES_DESCRIPTION_CODE_TLIST[0]

gTraceModeEnabled = False
gActiveLanguageDescriptionCodeTuple = ''

gStringReplacementForRootFolderWithExportedFiles = ""
gNumReplaceStartingCharacters = 0

gMIXWavFiles = []
gMIXWavFilesNoDups = []
gTLKWavFiles = []
gTLKWavFilesNoDups = []

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
	#print "[Debug] opening actornames"
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

def inputTLKsExport(inputTLKpath, outputWAVpath, pExportWavFilesMode, pExtractDevCommAndExtraSFXMode):
	# try to open all TLK, MIX file entries from SUPPORTED_TLK_INPUT_FILES + SUPPORTED_MIX_INPUT_FOR_EXTRA_SPEECH_EXPORT_FILES
	# then per TLK file
	#	create an output folder in the OUTPUT PATH named TLK## for the 1, 2, 3 TLK and TLKSPCHSFX for the SPCHSFX.TLK
	#	printout:
	#		total entries
	#		total data size
	#		and per entry the
	#				fileID
	#				segment offset
	#				file size
	if not pExportWavFilesMode and not pExtractDevCommAndExtraSFXMode:
		return # nothing to do
	print "[Info] Checking in:\n\t%s\n\tfor TLK or MIX files containing supported speech audio (AUD) to export as WAV to:\n\t%s" % (inputTLKpath, outputWAVpath)
	inputTLKFilesForSpeechFound = []
	inputMIXFilesForSpeechFound = []	# breaking after first for loop yields only the top directory files, which is what we want
	for (dirpath, dirnames, filenames) in walk(inputTLKpath):
		for filename in filenames:
			if pExportWavFilesMode:
				for tlkTuple in (SUPPORTED_TLK_INPUT_FILES):
					if filename.upper() == tlkTuple[0]:
						inputTLKFilesForSpeechFound.append(tlkTuple)
			if pExtractDevCommAndExtraSFXMode:
				for tlkTuple in SUPPORTED_MIX_INPUT_FOR_EXTRA_SPEECH_EXPORT_FILES:
					if filename.upper() == tlkTuple[0]:
						inputMIXFilesForSpeechFound.append(tlkTuple)
		break

	if pExportWavFilesMode:
		if len(inputTLKFilesForSpeechFound) == 0:
			print "[Error] No valid speech audio files (TLK) were found in the specified input path (-ip switch)"
			sys.exit(1)
	if pExtractDevCommAndExtraSFXMode:
		if len(inputMIXFilesForSpeechFound) == 0:
			print "[Error] No valid extras speech audio files (MIX) were found in the specified input path (-ip switch)"
			sys.exit(1)

	# get the supported AUD files from TLK or MIX archives
	for tmpTLKorMIXFileTuple in (inputTLKFilesForSpeechFound + inputMIXFilesForSpeechFound):
		if gTraceModeEnabled:
			print "[Info] Found supported audio file: %s" % ('"' + inputTLKpath + tmpTLKorMIXFileTuple[0] + '"')
		errorFound = False
		inTLKorMIXFile = None
		fileIsMIX = False
		#print "[Trace] Type of file is: %s" % (tmpTLKorMIXFileTuple[0][-3:])
		if tmpTLKorMIXFileTuple[0][-3:].upper() == 'MIX':
			fileIsMIX = True
			print "[Info] Checking MIX file %s for speech audio (-xdevs mode)... " % (tmpTLKorMIXFileTuple[0])
		#
		# Create output folder if not exists at output path
		if gTraceModeEnabled:
			print "[Debug] Ensuring output directory %s" % (os.path.join(outputWAVpath, tmpTLKorMIXFileTuple[1] ))
		ensure_dir(os.path.join(outputWAVpath, tmpTLKorMIXFileTuple[1] ) )
		try:
			inTLKorMIXFile = open(os.path.join(inputTLKpath,tmpTLKorMIXFileTuple[0]), 'rb')
		except:
			errorFound = True
			print "[Error] Unexpected event:", sys.exc_info()[0]
			raise
		if not errorFound:
			tmpBuff = inTLKorMIXFile.read(2)
			# H: unsigned short (2 bytes) followed by I: unsigned int (4 bytes)
			tlkFileEntriesNumTuple = struct.unpack('H', tmpBuff)
			numOfAUDEntriesToExtract = tlkFileEntriesNumTuple[0]
			tmpBuff = inTLKorMIXFile.read(4)
			tlkFileDataSegmentSizeTuple = struct.unpack('I', tmpBuff)
			allTlkFileSize = tlkFileDataSegmentSizeTuple[0]
			inTLKorMIXFile.seek(0, 2) # go to file end
			allActualBytesInMixFile = inTLKorMIXFile.tell()
			inTLKorMIXFile.seek(6, 0) # go to start of table of TLK file entries (right after the 6 bytes header)
			# 2 + 4 = 6 bytes short MIX header
			# 12 bytes per TLK entry in entries table
			# quick size validation
			if gTraceModeEnabled:
				print "[Debug] Entries: %d, Data segment size: %d bytes" % (numOfAUDEntriesToExtract, allTlkFileSize)
			if allActualBytesInMixFile != 2 + 4 + 12 * numOfAUDEntriesToExtract + allTlkFileSize:
				print "[Error] Audio Archive file size mismatch with reported size in header for %s!" % (tmpTLKorMIXFileTuple[0])
			else:
				#
				# 12 bytes per entry
				#		4 bytes: ID
				#		4 bytes: Offset in data segment
				#		4 bytes: Size of data
				#
				for i in range(0, numOfAUDEntriesToExtract):
					inTLKorMIXFile.seek(2 + 4 + 12*i)
					tmpBuff = inTLKorMIXFile.read(4)
					tmpRdTuple = struct.unpack('I', tmpBuff)
					idOfAUDEntry = tmpRdTuple[0]
					tmpBuff = inTLKorMIXFile.read(4)
					tmpRdTuple = struct.unpack('I', tmpBuff)
					offsetOfAUDEntry = tmpRdTuple[0]
					tmpBuff = inTLKorMIXFile.read(4)
					tmpRdTuple = struct.unpack('I', tmpBuff)
					sizeOfAUDEntry = tmpRdTuple[0]

					targetSimpleAudFileName = ''.join('{:08X}'.format(idOfAUDEntry)).upper()+'.AUD'
					foundSupportedAUDFromMIXFile = False
					if fileIsMIX:	# MIX file like SFX.MIX which has some spoken dialogue
						#print 'idOfEntry: %d' % (idOfAUDEntry)
						for suppAUDFromMIXFileName in zip(*SUPPORTED_EXPORTED_AUD_FROM_MIX_FILES)[0]:
							#print 'checked with: %d' % (calculateFoldHash(suppAUDFromMIXFileName))
							if(idOfAUDEntry == calculateFoldHash(suppAUDFromMIXFileName)):
								foundSupportedAUDFromMIXFile = True
								targetSimpleAudFileName = suppAUDFromMIXFileName
								break
						if foundSupportedAUDFromMIXFile:
							if gTraceModeEnabled:
								print "[Debug] Entry: %s, offset: %s, Data segment size: %s bytes" % (''.join('{:08X}'.format(idOfAUDEntry)), ''.join('{:08X}'.format(offsetOfAUDEntry)),''.join('{:08X}'.format(sizeOfAUDEntry)))
								#print "[Debug] Filename: " + targetSimpleAudFileName
					else:			# TLK file
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
					if foundSupportedAUDFromMIXFile or not fileIsMIX:
						inTLKorMIXFile.seek(2 + 4 + 12*numOfAUDEntriesToExtract + offsetOfAUDEntry)
						if(offsetOfAUDEntry + sizeOfAUDEntry > allTlkFileSize):
							print "[Error] audio file (AUD) file size mismatch with reported size in entry header!"
						else:
							targetSimpleWavFileName = targetSimpleAudFileName[:-4] + '.WAV' # remove the .AUD before adding the .WAV
							if not fileIsMIX: # TLK file
								(actorID, actorSName, localQuoteId) = getActorShortNameAndLocalQuoteIdByAUDHashID(idOfAUDEntry)
								targetSimpleWavFileName = actorSName + '_' + str(localQuoteId).zfill(4) + '_' + ''.join('{:08X}'.format(idOfAUDEntry)).upper()+'.WAV'
							
							audFileBuffer = inTLKorMIXFile.read(sizeOfAUDEntry)
							if (len(audFileBuffer) == sizeOfAUDEntry):
							# load audio file (AUD) file
								thisAudFile = audFile(gTraceModeEnabled)
								if (thisAudFile.loadAudFile(audFileBuffer, allTlkFileSize, targetSimpleAudFileName)):
									if gTraceModeEnabled:
										print "[Debug] Audio file (AUD) file %s was loaded successfully!" % (targetSimpleAudFileName)
									#print os.path.join(outputWAVpath, tmpTLKorMIXFileTuple[1], targetSimpleWavFileName)
									# tmpTLKorMIXFileTuple[1] is the subfolder where the AUD -> WAV files for this archive are written
									if not os.path.isfile(os.path.join(outputWAVpath, tmpTLKorMIXFileTuple[1], targetSimpleWavFileName) ):
										thisAudFile.export_as_wav(audFileBuffer, os.path.join(outputWAVpath, tmpTLKorMIXFileTuple[1], targetSimpleWavFileName) )
									else:
										if gTraceModeEnabled:
											print "[Info] Output file %s already exists. Skipping..." % (os.path.join(outputWAVpath, tmpTLKorMIXFileTuple[1], targetSimpleWavFileName))
								else:
									print "[Warning] Failed to load a proper audio file (AUD) %s (to export it to %s)! Size of input is: %d" % (targetSimpleAudFileName, targetSimpleWavFileName,  sizeOfAUDEntry)
							else:
								print "[Error] while reading audio file (AUD) file %s into mem buffer" % (targetSimpleAudFileName)
								#print "[Error] while reading audio file (AUD) file %s into mem buffer" % (''.join('{:08X}'.format(idOfMIXEntry)))
			inTLKorMIXFile.close()


	# SYS EXIT IS HERE ONLY FOR DEBUG PURPOSES OF PARSING TLK FILES - SHOULD BE COMMENTED OUT NORMALLY
	# sys.exit(0)
	return


def appendVQAPlaceHolderSheets(excelOutBook = None):
	if excelOutBook != None:
		print "[Info] Appending placeholder sheets for supported video cutscenes (VQA)..."
		currVQAFileNameLclzd = None
		for currVQAFileNameDesc in SUPPORTED_PLACEHOLDER_VQA_ENGLISH_FILES + SUPPORTED_PLACEHOLDER_VQA_LOCALIZED_FILES:
			if currVQAFileNameDesc in SUPPORTED_PLACEHOLDER_VQA_ENGLISH_FILES:
				currVQAFileNameLclzd = currVQAFileNameDesc[0] + 'E.VQA'
			elif currVQAFileNameDesc in SUPPORTED_PLACEHOLDER_VQA_LOCALIZED_FILES:
				currVQAFileNameLclzd = currVQAFileNameDesc[0] + ('%s' % (gActiveLanguageDescriptionCodeTuple[1])) + '.VQA'
			if 	currVQAFileNameLclzd is not None:
				if gTraceModeEnabled:
					print "[Debug] Creating placeholder sheet %s: %s!" % (currVQAFileNameLclzd, currVQAFileNameDesc[1])
				sh = excelOutBook.add_sheet(currVQAFileNameLclzd)
				# First Row
				n = 0 # keeps track of rows
				col_names = ['%s' % (currVQAFileNameLclzd), currVQAFileNameDesc[2], currVQAFileNameDesc[1], '22050 audio sampling rate', 'at 15 fps']
				colIdx = 0
				for colNameIt in col_names:
					sh.write(n, colIdx, colNameIt)
					colIdx+=1

				# Second Row
				n = 1
#				col_names = ['Start (YT)', 'End (YT)', 'Subtitle', 'By Actor', 'StartTime', 'Time Diff-SF', 'TimeDiff-SF(ms)', 'TimeDiff-EF', 'TimeDiff-EF(ms)', 'Frame Start', 'Frame End', 'Notes']
				col_names = ['Frame Start', 'Frame End', 'Subtitle', 'Time Start', 'Time End', 'By Actor', 'Notes']
				colIdx = 0
				for colNameIt in col_names:
					sh.write(n, colIdx, colNameIt)
					colIdx+=1
				# Fill in with placeholder content
				n+=1
				vqaTRInstance = vqasTextResource(gTraceModeEnabled)
				
				objUTF8Unicode = None
				for m, e1 in enumerate(vqaTRInstance.getVqaEntriesList(currVQAFileNameDesc[0]), n):
					sh.write(m, 0, e1[0]) # start frame
					sh.write(m, 1, e1[1]) # end frame
					for i1 in range(2,6):
						objStr = e1[i1]
						try:
							objUTF8Unicode = unicode(objStr, 'utf-8')
						except Exception as e:
							print '[Error] Failed to create unicode string: ' + str(e)
							objUTF8Unicode = unicode("???", 'utf-8')
						sh.write(m, i1, objUTF8Unicode)
	return

def appendPOGOTextSheet(excelOutBook = None):
	if excelOutBook != None:
		print "[Info] Appending POGO sheet..."
		currTreFileName = '%s%s' % (SUPPORTED_SPECIAL_POGO_FILE, gActiveLanguageDescriptionCodeTuple[1]) # POGO
		if gTraceModeEnabled:
			print "[Debug] TR%s file %s was loaded successfully!" % (gActiveLanguageDescriptionCodeTuple[1], currTreFileName) # POGO
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
		pogoTRInstance = pogoTextResource(gTraceModeEnabled)
		
		objUTF8Unicode = None
		for m, e1 in enumerate(pogoTRInstance.getPogoEntriesList(), n):
			sh.write(m, 0, e1[0])
			objStr = e1[1]
			try:
				# We assume utf-8 charset for POGO (since we get the text from a python script)
				objUTF8Unicode = unicode(objStr, 'utf-8')
			except Exception as e:
				print '[Error] Failed to create unicode string: ' + str(e)
				objUTF8Unicode = unicode("???", 'utf-8')
			sh.write(m, 1, objUTF8Unicode)

# aux: populate a row in extra audio sheets
def auxPopulateExtraSpeechAudioRow(sh = None, n = 0, pFilenameStr = '', pTextStr = '', pListOfExtraWavsForDevSheet = None):
	if sh is None \
		or not pFilenameStr \
		or pListOfExtraWavsForDevSheet is None \
		or len(pListOfExtraWavsForDevSheet) == 0:
		return False
		
	foundMatch = False
	for dcAUDFilename, dcArchiveMIXFile in SUPPORTED_EXPORTED_AUD_FROM_MIX_FILES:
		if dcAUDFilename == pFilenameStr:
			sh.write(n, 0, pFilenameStr)				# Filename
			# We assume utf-8 charset for Audio Commentary and extra text entries (since we get the text from a python script)
			try:
				objUTF8Unicode = unicode(pTextStr, 'utf-8')
			except Exception as e:
				print '[Error] Failed to create unicode string: ' + str(e)
				objUTF8Unicode = unicode("???", 'utf-8')
			sh.write(n, 1, objUTF8Unicode)				# Quote
			sh.write(n, 2, 'N/A')						# By Actor
			sh.write(n, 3, '')							# Notes
			sh.write(n, 4, 'N/A')						# To Actor
			smArchiveMIXFile = ''
			for smArchiveMIXFile, smArchiveMIXShort in SUPPORTED_MIX_INPUT_FOR_EXTRA_SPEECH_EXPORT_FILES:
				if smArchiveMIXFile == dcArchiveMIXFile:
					sh.write(n, 5, smArchiveMIXShort)	# Resource - proper
					break
			
			for extraWavFDS in pListOfExtraWavsForDevSheet:
				twoTokensOfRelDirnameAndFilenameXLS = extraWavFDS.split('&', 2)
				if len(twoTokensOfRelDirnameAndFilenameXLS) == 3:
					tokensOfFilename = twoTokensOfRelDirnameAndFilenameXLS[0].split('#', 3)
					#ShortHandFileName
					if (tokensOfFilename[0][:-4]+'.AUD') in zip(*SUPPORTED_EXPORTED_AUD_FROM_MIX_FILES)[0] \
						and (tokensOfFilename[0][:-4]+'.AUD') == dcAUDFilename:
						shortHandFileName = tokensOfFilename[0]
						# real path of filename
						realPathOfFileNameToLink = twoTokensOfRelDirnameAndFilenameXLS[2]
						# checks if not empty
						if gStringReplacementForRootFolderWithExportedFiles and gNumReplaceStartingCharacters > 0:
							realPathOfFileNameToLink = realPathOfFileNameToLink.replace(realPathOfFileNameToLink[:gNumReplaceStartingCharacters], gStringReplacementForRootFolderWithExportedFiles)
						# Libreoffice seems to only work with forward slashes (v6.2.8.2)
						# Also works with MS Excel (tested with Office 2007)
						realPathOfFileNameToLink = realPathOfFileNameToLink.replace('\\','/');
						hyperlinkAudioFormula = 'HYPERLINK("file:///%s","%s")' % (realPathOfFileNameToLink, shortHandFileName)
						sh.write(n, 6, Formula(hyperlinkAudioFormula))
						break
			foundMatch = True
			break
	return foundMatch
	
def appendDevCommentarySheet(excelOutBook = None, listOfExtraWavsForDevSheet = None ):
		
	if excelOutBook != None \
		and listOfExtraWavsForDevSheet != None \
		and len(listOfExtraWavsForDevSheet) > 0:
		
		objUTF8Unicode = None
		print "[Info] Appending Developer Commentary sheet..."
		
		sh = excelOutBook.add_sheet("DEV AUDIO")
		devCommentaryTextInstance = devCommentaryText(gTraceModeEnabled)
		#
		#
		# First Sheet
		n = 0 # keeps track of rows
		col_names = ["DEVELOPERS' AUDIO COMMENTARY"]
		colIdx = 0
		for colNameIt in col_names:
			sh.write(n, colIdx, colNameIt)
			colIdx+=1
		n+= 1
		
		col_names = ['Filename', 'Quote', 'By Actor', 'Notes', 'To Actor', 'Resource', 'ShortHandFileName']
		colIdx = 0
		for colNameIt in col_names:
			sh.write(n, colIdx, colNameIt)
			colIdx+=1
		n+= 1
		# populate audio commentary rows
		for pFilenameStr, pTextStr in devCommentaryTextInstance.getAudioCommentaryTextEntriesList():
			if auxPopulateExtraSpeechAudioRow(sh, n, pFilenameStr, pTextStr, listOfExtraWavsForDevSheet) == True:
				n+=1 # change row
		#
		#
		# Second Sheet (EXTRA SPEECH)
		sh = excelOutBook.add_sheet("SPEECH (SFX)")
		n = 0 # keeps track of rows
		col_names = ['EXTRA SPEECH (SFX)']
		colIdx = 0
		for colNameIt in col_names:
			sh.write(n, colIdx, colNameIt)
			colIdx+=1
		n+= 1
		
		col_names = ['Filename', 'Quote', 'By Actor', 'Notes', 'To Actor', 'Resource', 'ShortHandFileName']
		colIdx = 0
		for colNameIt in col_names:
			sh.write(n, colIdx, colNameIt)
			colIdx+=1
		n+= 1
		
		# populate extra speech audio rows
		for pFilenameStr, pTextStr in devCommentaryTextInstance.getExtraSpeechAudioEntriesList():
			if auxPopulateExtraSpeechAudioRow(sh, n, pFilenameStr, pTextStr, listOfExtraWavsForDevSheet) == True:
				n+=1 # change row
		#
		#
		# Third Sheet
		sh = excelOutBook.add_sheet("DEV I_SEZ")
		n = 0 # keeps track of rows
		col_names = ['I_SEZ QUOTES']
		colIdx = 0
		for colNameIt in col_names:
			sh.write(n, colIdx, colNameIt)
			colIdx+=1
		n+= 1

		col_names = ['AssignedID', 'Quote', 'By Actor', 'Notes', 'To Actor', 'Resource', 'ShortHandFileName']
		colIdx = 0
		for colNameIt in col_names:
			sh.write(n, colIdx, colNameIt)
			colIdx+=1
		n+= 1	
		
		for idStr, textStr in devCommentaryTextInstance.getISEZTextEntriesList():
			sh.write(n, 0, idStr)
			# We assume utf-8 charset for I_SEZ Commentary (since we get the text from a python script)
			try:
				objUTF8Unicode = unicode(textStr, 'utf-8')
			except Exception as e:
				print '[Error] Failed to create unicode string: ' + str(e)
				objUTF8Unicode = unicode("???", 'utf-8')
			sh.write(n, 1, objUTF8Unicode)
			sh.write(n, 2, 'N/A')	# by actor
			sh.write(n, 3, '')	# notes
			sh.write(n, 4, 'N/A')	# to actor
			sh.write(n, 5, 'N/A')	# Resources
			sh.write(n, 6, 'N/A')	# ShortHandFilename and URL
			n+=1
	return

def inputMIXExtractTREs(inputMIXpath, excelOutBook = None):
	print "[Info] Checking in %s for MIX files to extract Text Resources (TR%ss) from..." % (inputMIXpath, gActiveLanguageDescriptionCodeTuple[1])
	inputMIXFilesFound = []
	objUTF8Unicode = None
	# breaking after first for loop yields only the top directory files, which is what we want
	for (dirpath, dirnames, filenames) in walk(inputMIXpath):
		for filename in filenames:
			for mixFileName in SUPPORTED_MIX_INPUT_FOR_TRX_EXPORT_FILES:
				if filename.upper() == mixFileName:
					inputMIXFilesFound.append(mixFileName)
		break

	if len(inputMIXFilesFound) == 0:
		print "[Error] No supported game archive resource files (MIX) were found in the specified input path (-ip switch)"
		sys.exit(1)

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
											if ( currTreFileName[:-1] == 'ERRORMSG.TR'):
												# ERRORMSG.TRx uses different font than the others 
												# and there are conflicts (with the other in-game fonts)
												# Supposedly these messages would be printed by an OS font
												# They seem to mostly use the Latin-1 codepage, although there are a few 
												# characters that don't encode right (but they do with a IBM437 like the rest of the TRx
												# Probably, these are forgotten (buggy) characters from a previous plan (print error messages with in-game font?)
												# So we treat them as bugs and fix them. 
												# Further proof that they are buggy, there are language versions that have two codes for the same character
												# in ERRORMSG TRx. 
												# Eg. in Spanish version they use \xa2 (correct with IBM437) and \xf3 (correct with Latin-1) for the 'ó' character
												#  or in French version 'è' uses both \x8a and \xe8
												objEncodeSafeStr = ""
												for i in range(0, len(objStr)):
													# ENG version has no buggy character exceptions
													# GER version has no buggy character exceptions
													# FRA version
													if (objStr[i] == '\x82'): # FRA - ERRORMSG.TRF
														objEncodeSafeStr += '\xe9'	# fixing buggy character 'é'
													elif (objStr[i] == '\x85'): # FRA - ERRORMSG.TRF (also used in ITA ERRORMSG.TRI - same glyph)
														objEncodeSafeStr += '\xe0'	 # fixing buggy character 'à'
													elif (objStr[i] == '\x8a'): # FRA - ERRORMSG.TRF (also used in ITA ERRORMSG.TRI - same glyph)
														objEncodeSafeStr += '\xe8'    # fixing buggy character 'è'
													# ITA version
													#elif (objStr[i] == '\x85'): # ITA - ERRORMSG.TRI [commented out: already covered in FRA check above]
													#	objEncodeSafeStr += '\xe0'	 # buggy - fixed above
													#elif (objStr[i] == '\x8a'): # ITA - ERRORMSG.TRI [commented out: already covered in FRA check above]
													#	objEncodeSafeStr += '\xe8'    # buggy - fixed above
													elif (objStr[i] == '\x97'): # ITA - ERRORMSG.TRI
														objEncodeSafeStr += '\xf9'    # fixing buggy character 'ù'
													# ESP version
													elif (objStr[i] == '\xa2'): # ESP - ERRORMSG.TRS
														objEncodeSafeStr += '\xf3' 	# fixing buggy character 'ó'
													else:
														objEncodeSafeStr += objStr[i]
													#
													try:
														objUTF8Unicode = unicode(objEncodeSafeStr, 'latin-1')
													except Exception as e:
														print '[Error] Failed to create unicode string: ' + str(e)
														objUTF8Unicode = unicode("???", 'latin-1')
											else:
											# all the other official game TRx use the KIA6PT.FON
											# There could be variances of the KIA6PT.FON per Blade Runner version
											# TODO: For now, we assume that there aren't significant variances that warrant a more elaborate approach
											# TODO: Tested for EN, DEU, FRA, ITA, ESP. Pending testing for Russian version.
												try:
													objUTF8Unicode = unicode(objStr, 'cp437')
												except Exception as e:
													print '[Error] Failed to create unicode string: ' + str(e)
													objUTF8Unicode = unicode("???", 'cp437')
											sh.write(m, 1, objUTF8Unicode)
									#for tupleIdString in thisTreFile.stringEntriesLst:
									#	#print "[Debug] Id: %d\t Text: %s" % (tupleIdString[0], tupleIdString[1])
									#	pass
									totalTREs =  totalTREs + 1
								else:
									print "[Error] while loading TR%s %s file!" % (gActiveLanguageDescriptionCodeTuple[1], currTreFileName)
							else:
								print "[Error] while reading TR%s file %s into mem buffer" % (gActiveLanguageDescriptionCodeTuple[1], currTreFileName)
								#print "[Error] while reading TR%s file %s into mem buffer" % (gActiveLanguageDescriptionCodeTuple[1], ''.join('{:08X}'.format(idOfMIXEntry)))
			inMIXFile.close()
			print "[Info] Total TR%ss processed: %d " % (gActiveLanguageDescriptionCodeTuple[1], totalTREs)
	return


#
# Creating the OUTPUT XLS file with one sheet named as the @param sheet with entries based on the 
#	listTlkWavs (wav files from TLK files, without duplicates)
#	and /or
#	listDevsWavs (extra (supported) wav files practicaclly from the SFX.MIX file), without duplicates)
#
def outputXLS(filename, sheet, listTlkWavs, listDevsWavs, parseTREResourcesAlso = False, addDevCommAndExtraSFXSheetEnabled = False, addPOGOTextSheetEnabled = False, mixInputFolderPath = ''):
	global gStringReplacementForRootFolderWithExportedFiles
	global gNumReplaceStartingCharacters

	book = xlwt.Workbook()
	# first add version info sheet
	sbtlVersTRInstance = sbtlVersTextResource(gTraceModeEnabled)
	vrsn_sheet = book.add_sheet(SUPPORTED_DIALOGUE_VERSION_SHEET)
	n = 0
	col1_name = 'Subtitles Version Info'
	vrsn_sheet.write(n, 0, col1_name)
	# Second Row
	n = 1
	col1_name = 'ID'
	col2_name = 'Value'
	col3_name = 'Notes'
	vrsn_sheet.write(n, 0, col1_name)
	vrsn_sheet.write(n, 1, col2_name)
	vrsn_sheet.write(n, 2, col3_name)
	n += 1
	objUTF8Unicode = None
	for m, e1 in enumerate(sbtlVersTRInstance.getSbtlVersEntriesList(), n):
		vrsn_sheet.write(m, 0, e1[0])
		tmpQuoteID = int(e1[0])
		for i1 in range(1,3):
			objStr = e1[i1]
			try:
				# We assume utf-8 charset (since we get the text from a python script)
				# populate row with quote ID == 3 and column B with the description of language used in the command's execution (or assumed)
				if tmpQuoteID == 3 and i1 == 1:
					objUTF8Unicode = unicode(gActiveLanguageDescriptionCodeTuple[2], 'utf-8')
				else:
					objUTF8Unicode = unicode(objStr, 'utf-8')
			except Exception as e:
				print '[Error] Failed to create unicode string: ' + str(e)
				objUTF8Unicode = unicode("???", 'utf-8')
			vrsn_sheet.write(m, i1, objUTF8Unicode)
	#
	# Add ingame quotes sheet
	#
	sh = book.add_sheet(sheet)
	# First Row
	n = 0      # keeps track of rows
	#variables = [x, y, z]
	#x_desc = 'Display'
	#y_desc = 'Dominance'
	#z_desc = 'Test'
	#desc = [x_desc, y_desc, z_desc]
	#
	#	
	# You may need to group the variables together
	#for n, (v_desc, v) in enumerate(zip(desc, variables)):
		#sh.write(n, 0, v_desc)
		#sh.write(n, 1, v)
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

	for m, e1 in enumerate(listTlkWavs, n):
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
				#	python sortBladeRunnerWavs.py -op /g/WORKSPACE/BladeRunnerWorkspace/br-mixer-master/data/WAV -m "G:/WORKSPACE/BladeRunnerWorkspace/br-mixer-master/data/WAV"
				# put real full path for each file as FILE URL, and real (or approximate shorthand file name as alias)
				#
				# Libreoffice seems to only work with forward slashes (v6.2.8.2)
				# Also works with MS Excel (tested with Office 2007)
				realPathOfFileNameToLink = realPathOfFileNameToLink.replace('\\','/');
				hyperlinkAudioFormula = 'HYPERLINK("file:///%s","%s")' % (realPathOfFileNameToLink, shortHandFileName)
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

	#for m, e2 in enumerate(list2, n+1):
	#	sh.write(m, 1, e2)
	appendVQAPlaceHolderSheets(book)

	if parseTREResourcesAlso == True and mixInputFolderPath != '':
		inputMIXExtractTREs(mixInputFolderPath, book)

	if addPOGOTextSheetEnabled == True:
		appendPOGOTextSheet(book)

	if addDevCommAndExtraSFXSheetEnabled == True:
		appendDevCommentarySheet(book, listDevsWavs)

	try:
		book.save(filename)
		print "[Info] Done."
	except Exception as e:
		print "[Error] Could not save the output Excel file:: " + str(e)

#
# Aux function to weed out duplicates from wav file lists 
# valid listTypes: 'TLKWAVS', 'EXTRAMIXWAVS'
def wavDupsRemove(listOfWavFiles, listType):
	if listType != 'TLKWAVS' and listType != 'EXTRAMIXWAVS':
		print '[Error] Invalid listtype %s specified to remove duplicate wav files' % (listType)
		sys.exit(1)
	
	listOfWavFilesNoDups = []
	for filenameSrcTmp in listOfWavFiles:
		duplicateFound = False
		#print "[Debug]Converted %s" % (filenameSrcTmp)
		# Weed out duplicates by copying to another table (quick and dirty)
		twoTokensOfRelDirnameAndFilenameSrc = filenameSrcTmp.split('&', 2)
		tmpRelDirNameSrc = twoTokensOfRelDirnameAndFilenameSrc[1] # eg. TLK1
		keyForDuplicateSearchSrc = None
		keyForDuplicateSearchTarg = None
		
		if (listType == 'EXTRAMIXWAVS'):
			keyForDuplicateSearchSrc = twoTokensOfRelDirnameAndFilenameSrc[0]		
		else:
			threeTokensOfQuoteFilenameSrc = twoTokensOfRelDirnameAndFilenameSrc[0].split('#', 2)
			#concatenate actorID and quoteID for search key
			keyForDuplicateSearchSrc = threeTokensOfQuoteFilenameSrc[0] + threeTokensOfQuoteFilenameSrc[1]
		
		for fileTargIdx, filenameTargTmp in enumerate(listOfWavFilesNoDups):
			twoTokensOfRelDirnameAndFilenameTarg = filenameTargTmp.split('&', 2)
			tmpRelDirNameTarg = twoTokensOfRelDirnameAndFilenameTarg[1]

			if (listType == 'EXTRAMIXWAVS'):
				keyForDuplicateSearchTarg = twoTokensOfRelDirnameAndFilenameTarg[0]			
			else:
				threeTokensOfQuoteFilenameTarg = twoTokensOfRelDirnameAndFilenameTarg[0].split('#', 2)
				#concatenate actorID and quoteID for search key
				keyForDuplicateSearchTarg =	 threeTokensOfQuoteFilenameTarg[0] + threeTokensOfQuoteFilenameTarg[1]
			if(keyForDuplicateSearchSrc == keyForDuplicateSearchTarg):
				#print "[Debug] Found duplicate %s" % (filenameSrcTmp)
				duplicateFound = True
				listOfWavFilesNoDups[fileTargIdx] = twoTokensOfRelDirnameAndFilenameTarg[0] + '&' +  tmpRelDirNameSrc + ',' + tmpRelDirNameTarg + '&' + twoTokensOfRelDirnameAndFilenameTarg[2]
				break
		if(duplicateFound == False):
			listOfWavFilesNoDups.append(filenameSrcTmp)
	#for filenameSrcTmp in listOfWavFilesNoDups:
	#	print "[Debug] Unique %s" % (filenameSrcTmp)
	return listOfWavFilesNoDups

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
	global gMIXWavFiles
	global gMIXWavFilesNoDups
	global gTLKWavFiles
	global gTLKWavFilesNoDups
	
	gTraceModeEnabled = False
	gActiveLanguageDescriptionCodeTuple = DEFAULT_LANG_DESC_CODE


	pathToActorNamesTxt = ""
	candidateLangDescriptionTxt = ""

	TMProotFolderWithExportedFiles = ""
	TMProotFolderWithInputTLKFiles = ""

	exportWavFilesMode = False
	extractTreFilesMode = False
	extractDevCommAndExtraSFXMode = False
	extractPOGOTextMode = False

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
			print "%s takes has one mandatory argument, ie. the folder of the exported WAV files:" % (APP_WRAPPER_NAME)
			print "Valid syntax: %s -op folderpath_for_exported_wav_Files [-ip folderpath_for_TLK_Files] [-ian path_to_actornames_txt] [-m stringPathToReplaceFolderpathInExcelLinks] [-ld gameInputLanguageDescription] [-xwav] [-xtre] [-xdevs] [-xpogo] [--trace]" % (APP_WRAPPER_NAME)
			print "The -op switch has an argument that is the path for exported WAV files folder. The -op switch is REQUIRED always."
			print "The -ip switch has an argument that is the path for the input (TLK or MIX) files folder (can be the same as the Blade Runner installation folder)."
			print "The -ian switch is followed by the path to actornames.txt, if it's not in the current working directory."
			print "The -m switch has an argument that is a replacement string for the path to the folder of exported WAV files which will be used as a prefix for the links in the output XLS file."
			print "The -ld switch has an argument that is the language description of the original game files that you use as input."
			printInfoMessageForLanguageSelectionSyntax()
			print "The -xwav switch enables the WAV audio export mode from the TLK files. It requires an INPUT path to be set with the -ip switch."
			print "The -xtre switch enables the TRx parsing mode from the original MIX files. It requires an INPUT path to be set with the -ip switch."
			print "The -xdevs switch will add a sheet for Developer Commentary text and some additional voice-overs from SFX.MIX."
			print "The -xpogo switch will add a sheet for the POGO text."
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
				print "[Info] Export WAVs from TLK files mode enabled (only missing files will be exported)."
				exportWavFilesMode = True
			elif (argsCL[i] == '-xtre'):
				print "[Info] Extract Text Resources (TRx) mode enabled."
				extractTreFilesMode = True
			elif (argsCL[i] == '-xdevs'):
				print "[Info] Additional Sheet for developer commentary and extra SFX mode enabled."
				extractDevCommAndExtraSFXMode = True
			elif (argsCL[i] == '-xpogo'):
				print "[Info] Additional Sheet for POGO text mode enabled."
				extractPOGOTextMode = True
			elif argsCL[i] == '--trace':
				print "[Info] Trace mode enabled (more debug messages)."
				gTraceModeEnabled = True
			elif argsCL[i][:1] == '-':
				invalidSyntax = True
				break

		if (not exportWavFilesMode):
			print "[Info] Export WAVs from TLK files mode disabled."
		if (not extractTreFilesMode):
			print "[Info] Extract Text Resources (TRx) mode disabled."
		if (not extractDevCommAndExtraSFXMode):
			print "[Info] Additional Sheet for developer commentary and extra SFX mode disabled."
		if (not extractPOGOTextMode):
			print "[Info] Additional Sheet for POGO text mode disabled."

		if not TMProotFolderWithExportedFiles: # this argument is mandatory
			print "[Error] The output path for exported files (-op switch) argument is mandatory!"
			invalidSyntax = True

		if (not invalidSyntax) \
			and (exportWavFilesMode == True or extractDevCommAndExtraSFXMode == True or extractTreFilesMode == True) \
			and (TMProotFolderWithInputTLKFiles == ''):
			print "[Error] No game input path (-ip switch) specified, while any of the export audio to WAV mode (-xwav), the export extras (-xdevs) or the extract Text Resources mode (-xtre) is enabled."
			invalidSyntax = True

		if (not invalidSyntax) \
			and (exportWavFilesMode == False and extractDevCommAndExtraSFXMode == False and extractTreFilesMode == False) \
			and (TMProotFolderWithInputTLKFiles != ''):
			print "[Warning] Specified game input path (-ip switch) will be ignored, since the export audio to WAV mode (-xwav), the export extras (-xdevs) and the extract Text Resources mode (-xtre) are disabled."
			# not invalid syntax though

		gActiveLanguageDescriptionCodeTuple = getLanguageDescCodeTuple(candidateLangDescriptionTxt)
		if (not invalidSyntax) and gActiveLanguageDescriptionCodeTuple is None:
			print "[Error] Invalid language code was specified"
			printInfoMessageForLanguageSelectionSyntax()
			invalidSyntax = True

		if not invalidSyntax:
			print "[Info] Game Language Selected: %s (%s)" % (gActiveLanguageDescriptionCodeTuple[0], gActiveLanguageDescriptionCodeTuple[2])
			# parse Actors files:
			initActorPropertyEntries(pathToActorNamesTxt)
			#for actorEntryTmp in gActorPropertyEntries:
			#	print "[Debug] Found actor: %s %s %s" % (actorEntryTmp[0], actorEntryTmp[1], actorEntryTmp[2])
			#
			# Early checks for invalid cases
			# 1. if TMProotFolderWithInputTLKFiles is not valid and -xtre or -xwav -> error!
			# 		use os.path.isdir?
			#
			# 2. if not -wav and -op path is invalid then empty INGQUOT sheet -> error
			# 3 [We check for this further bellow, before call for outputXLS]. if not -wav and -op path is empty (has no WAVs) then empty INGQUOT sheet -> Treat as an error case!
			if ((exportWavFilesMode == True or extractDevCommAndExtraSFXMode == True or extractTreFilesMode == True) and (not os.path.isdir(TMProotFolderWithInputTLKFiles))):
				print "[Error] Invalid game input path (-ip switch) was specified, while the export audio to WAV mode (-xwav), export extras (-xdevs) or the extract Text Resources mode (-xtre) is enabled."
				sys.exit(1)
			if ((exportWavFilesMode == False and extractDevCommAndExtraSFXMode == False) and (not os.path.isdir(TMProotFolderWithExportedFiles))):
				print "[Error] Invalid output path for exported files (-op switch) was specified, while the export audio to WAV mode (-xwav) and export extras (-xdevs) are disabled (if enabled, it would create the path)."
				sys.exit(1)
			#
			# Checking for the optional case of parsing the input TLK files to export to WAV
			#
			if TMProotFolderWithInputTLKFiles != '':
				if (exportWavFilesMode == True or extractDevCommAndExtraSFXMode == True):
					inputTLKsExport(TMProotFolderWithInputTLKFiles, TMProotFolderWithExportedFiles, exportWavFilesMode, extractDevCommAndExtraSFXMode)
				#if (extractTreFilesMode == True):
				#	inputMIXExtractTREs(TMProotFolderWithInputTLKFiles)
			#
			# Parsing the exported WAV files
			# NO: depending on the mode don't parse the irrellevant WAVs?
			print "[Info] Parsing the (exported) WAV audio files in the -op path.\n\tPlease wait (this could take a while)..."
			# The broad search here is intentional
			for (dirpath, dirnames, filenames) in walk(TMProotFolderWithExportedFiles):
				for nameIdx, nameTmp in enumerate(filenames):
					if (len(nameTmp) > 4) \
						and nameTmp.upper()[-4:] == '.WAV' \
						and len(nameTmp.split('.', 1)) == 2: # radical heal (but practical) - don't allow cases like .AUD.WAV or any intermediate dots in the filename!
						relDirName = ''				
						# os.path.split would Split the pathname path into a pair, 
						# (head, tail) where tail is the last pathname component and head is everything leading up to that. The tail part will never contain a slash
						pathTokens = dirpath.split(os.path.sep)
						idxOfLastTLKToken = -1
						idxOfLastSFXToken = -1
						for tokenIdx, pTokenTmp in enumerate(pathTokens):
							posOfTLKWithinRightmostToken = pTokenTmp.find("TLK")
							posOfSFXWithinRightmostToken = pTokenTmp.find("SFX")
							
							if posOfTLKWithinRightmostToken != -1:
								idxOfLastTLKToken = tokenIdx
							if posOfSFXWithinRightmostToken != -1:
								idxOfLastSFXToken = tokenIdx
								
							if (posOfTLKWithinRightmostToken != -1 \
								or posOfSFXWithinRightmostToken != -1):
								relDirName = pTokenTmp	# don't break, we are interested in right-most matching token in the path
						#if (nameTmp.upper()[:-4] + '.AUD') in zip(*SUPPORTED_EXPORTED_AUD_FROM_MIX_FILES)[0]:
						#	print '[Trace] 1 file: %s' % (nameTmp.upper())
						#	print 'TLKss %d vs SFXss:%d ' % (idxOfLastTLKToken, idxOfLastSFXToken)

						if (idxOfLastSFXToken > idxOfLastTLKToken \
							and (nameTmp.upper()[:-4] + '.AUD') in zip(*SUPPORTED_EXPORTED_AUD_FROM_MIX_FILES)[0]):
							gMIXWavFiles.append(filenames[nameIdx] +'&' + relDirName + '&' + os.path.join(dirpath, nameTmp))
						else:	
							gTLKWavFiles.append(filenames[nameIdx] +'&' + relDirName + '&' + os.path.join(dirpath, nameTmp))
				#break
			for fileIdx, filenameTmp in enumerate(gTLKWavFiles):
				twoTokensOfFilenameAndRelDirname = filenameTmp.split('&', 1)
				if len(twoTokensOfFilenameAndRelDirname) != 2:
					print "[Error] While attempting filename and relative dirname split: %s" % (filenameTmp)
					sys.exit(0)
				#
				# Remove WAV extension here
				# AUD file from TLK archive
				threeTokensOfFilename = twoTokensOfFilenameAndRelDirname[0][:-4].split('_', 2)
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
							print "[Error] in actorIdMatch match: %s %s" % (tmpActorId, twoTokensOfFilenameAndRelDirname[0][:-4])
							sys.exit(0)
					else:
					#fatal error if something cannot convert to spot it immediately
						print "[Error] in shorthand match: %s %s" % (threeTokensOfFilename[0], twoTokensOfFilenameAndRelDirname[0][:-4])
						sys.exit(0)
					#
					#
					#foundMatchForActorShortHand = False
					#for actorEntryTmp in gActorPropertyEntries:
					#	if actorEntryTmp[1] == threeTokensOfFilename[0]:
					#		threeTokensOfFilename[0] = actorEntryTmp[0].zfill(2)
					#		threeTokensOfFilename.append(actorEntryTmp[2])
					#		foundMatchForActorShortHand = True
					#		break
					# end of replace actor name shorthand
					tmpHealedSimpleFilename = '#'.join(threeTokensOfFilename)
					filenameTmp =  tmpHealedSimpleFilename + '&' + twoTokensOfFilenameAndRelDirname[1]
					gTLKWavFiles[fileIdx] = filenameTmp
				else:
					print "[Error] in splitting tokens on _: %s" % (filenameTmp)
					sys.exit(0)
			# 
			# Now handle the extra speech files (-xdevs mode)
			for fileIdx, filenameTmp in enumerate(gMIXWavFiles): # 
				# if it's a AUD file from a MIX archive, we treat it differently here 
				# (we will put it in another sheet)
				twoTokensOfFilenameAndRelDirname = filenameTmp.split('&', 1)
				if len(twoTokensOfFilenameAndRelDirname) != 2:
					print "[Error] While attempting filename and relative dirname split on extra speech file: %s" % (filenameTmp)
					sys.exit(0)

				if (gTraceModeEnabled):
					print "[Debug] No healing action needed on: %s" % (twoTokensOfFilenameAndRelDirname[0])

			#sort in-place
			#
			#
			gTLKWavFiles.sort()
			gMIXWavFiles.sort()
			#
			#
			# Remove duplicates from TLK AUD speech files (-xwav mode) 
			#
			gTLKWavFilesNoDups = wavDupsRemove(gTLKWavFiles, 'TLKWAVS')
			if len(gTLKWavFilesNoDups) == 0:
				print "[Error] No supported speech audio files (WAV) were found in the output folder path (-op switch)."
				sys.exit(1)
			#
			# Remove duplicates from extra speech files (-xdevs mode)
			#
			gMIXWavFilesNoDups = wavDupsRemove(gMIXWavFiles, 'EXTRAMIXWAVS')
			if extractDevCommAndExtraSFXMode and len(gMIXWavFilesNoDups) == 0:
				print "[Error] No supported extra speech audio files (WAV) were found in the output folder path (-op switch)."
				sys.exit(1)
				
			constructedOutputFilename = "%s-%s%s" % (OUTPUT_XLS_FILENAME, gActiveLanguageDescriptionCodeTuple[2], OUTPUT_XLS_FILENAME_EXT)
			print "[Info] Creating output excel %s file..." % (constructedOutputFilename)
			outputXLS(constructedOutputFilename, OUTPUT_XLS_QUOTES_SHEET + gActiveLanguageDescriptionCodeTuple[1] + '.TR' + gActiveLanguageDescriptionCodeTuple[1], gTLKWavFilesNoDups, gMIXWavFilesNoDups, extractTreFilesMode, extractDevCommAndExtraSFXMode, extractPOGOTextMode, TMProotFolderWithInputTLKFiles)
	else:
		invalidSyntax = True

	if invalidSyntax == True:
		print "[Error] Invalid syntax!\n Try: \n %s --help for more info \n %s --version for version info " % (APP_WRAPPER_NAME, APP_WRAPPER_NAME)
		print "Valid syntax: %s -op folderpath_for_exported_wav_Files [-ip folderpath_for_TLK_Files] [-ian path_to_actornames_txt] [-m stringPathToReplaceFolderpathInExcelLinks] [-ld gameInputLanguageDescription] [-xwav] [-xtre] [-xdevs] [-xpogo] [--trace]" % (APP_WRAPPER_NAME)
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
