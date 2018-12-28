#!/usr/bin/env python2.7
# -*- coding: UTF-8 -*-
#
# Created by Praetorian (ShadowNate) for Classic Adventures in Greek
# classic.adventures.in.greek@gmail.com
#
# TODO update README
# TODO test recreation of TRE file (Especially in Credits which the original has a few special characters (font delegates))
#
# DONE Add code and switch option: to get the blade runner installation directory as input, then find the TLK files and extract them with proper naming
# DONE fix proper names for sheets as per latest code changes
#
import os, sys

shutilLibFound = False
xlwtLibFound = False
csvLibFound = False

try:
	import shutil 
except ImportError:
	print "Error:: Shutil python library is required to be installed!" 
else:
	shutilLibFound = True

try:
	import xlwt 
except ImportError:
	print "Error:: xlwt python library is required to be installed!" 
else:
	xlwtLibFound = True
	
try:
	import csv 
except ImportError:
	print "Error:: csv python library is required to be installed!" 
else:
	csvLibFound = True

if 	(not shutilLibFound) or (not structLibFound) or (not csvLibFound):
	sys.stdout.write("Error:: Errors were found when trying to import required python libraries\n")
	sys.exit(1)

from os import walk, errno, path
from xlwt import *
from audFileLib import *
from treFileLib import *

# encoding=utf8
#reload(sys)
#sys.setdefaultencoding('utf8')

company_email = "classic.adventures.in.greek@gmail.com"
app_version = "0.70"
app_name = "sortBladeRunnerWavs"
app_name_spaced = "Blade Runner Transcript Excel Creator (bare bones)"
app_short_desc = "Create an Excel (.XLS) for transcribing Blade Runner. It can also extract TRE files and export WAV files for game's resources. "
traceModeEnabled = False

stringReplacementForRootFolderWithExtractedFiles = ""
numReplaceStartingCharacters = 0

OUTPUT_XLS_FILENAME = 'out.xls'
OUTPUT_XLS_QUOTES_SHEET = 'INGQUO_E.TRE'

supportedTLKInputFiles = [('1.TLK', 'TLK01'), ('2.TLK', 'TLK02'), ('3.TLK', 'TLK03'), ('A.TLK', 'TLK0A'), ('SPCHSFX.TLK', 'TLKSPCHSFX')]
supportedMIXInputFiles = ['STARTUP.MIX']
# 15 TRE files
supportedExportedTREFiles = ['CLUES.TRE','ACTORS.TRE','CRIMES.TRE','CLUETYPE.TRE','KIA.TRE','SPINDEST.TRE','VK.TRE','OPTIONS.TRE','DLGMENU.TRE','ENDCRED.TRE','HELP.TRE','SCORERS.TRE','KIACRED.TRE','ERRORMSG.TRE','AUTOSAVE.TRE']

wavfiles = []
wavfilesNoDups = []
actorPropertyEntries = []				#[0]:id,	[1]:ShortHand Name	[2]:Full Name
actorPropertyEntriesWasInit = False


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

def getActorShortNameAndLocalQuoteIdByAUDHashID(audHashId):
	actorId = 0
	actorShortName = ''
	actorLocalQuoteId = 0
	if not actorPropertyEntriesWasInit:
		print "Error:: actor properties were not initialized!"
		return (actorId, actorShortName, actorLocalQuoteId)

	for actorEntryTmp in actorPropertyEntries:
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
def inputTLKsExtract(inputTLKpath, outputWAVpath):
	# try to open all TLK file entries from supportedTLKInputFiles
	# then per TLK file
	#	create an output folder in the OUTPUT PATH named TLK## for the 1, 2, 3 TLK and TLKSPCHSFX for the SPCHSFX.TLK
	#	printout:
	#		total entries
	#		total data size
	#		and per entry the
	#				fileID
	#				segment offset
	#				file size
	print "Info:: Checking in %s for TLK files to extract to %s" % (inputTLKpath, outputWAVpath)
	inputTLKFilesFound = []
	# breaking after first for loop yields only the top directory files, which is what we want
	for (dirpath, dirnames, filenames) in walk(inputTLKpath):
		for filename in filenames:
			for tlkTuple in supportedTLKInputFiles:
				if filename.upper() == tlkTuple[0]:
					inputTLKFilesFound.append(tlkTuple)
		break
	for tmpTLKfileTuple in inputTLKFilesFound:
		if traceModeEnabled:
			print "Info:: Found TLK: %s" % ('"' + inputTLKpath + tmpTLKfileTuple[0] + '"')
		errorFound = False
		inTLKFile = None
		#
		# Create output folder if not exists at output path
		if traceModeEnabled:
			print "Ensuring output directory %s" % (os.path.join(outputWAVpath, tmpTLKfileTuple[1] ))
		ensure_dir(os.path.join(outputWAVpath, tmpTLKfileTuple[1] ) )
		try:
			inTLKFile = open(os.path.join(inputTLKpath,tmpTLKfileTuple[0]), 'rb')
		except:
			errorFound = True
			print "Error:: Unexpected error:", sys.exc_info()[0]
			raise
		if not errorFound:
			tmpBuff = inTLKFile.read(2)
			# H: unsigned short (2 bytes) followed by I: unsigned int (4 bytes)
			tlkFileEntriesNumTuple = struct.unpack('H', tmpBuff)
			numOfEntriesToExtract = tlkFileEntriesNumTuple[0]
			tmpBuff = inTLKFile.read(4)
			tlkFileDataSegmentSizeTuple = struct.unpack('I', tmpBuff)
			allTlkFileSize = tlkFileDataSegmentSizeTuple[0]
			inTLKFile.seek(0, 2) # go to file end
			allActualBytesInMixFile = inTLKFile.tell()
			inTLKFile.seek(6, 0) # go to start of table of TLK file entries (right after the 6 bytes header)
			# 2 + 4 = 6 bytes short MIX header
			# 12 bytes per TLK entry in entries table
			# quick size validation
			if traceModeEnabled:
				print "Entries: %d, data segment %d bytes" % (numOfEntriesToExtract, allTlkFileSize)
			if allActualBytesInMixFile != 2 + 4 + 12 * numOfEntriesToExtract + allTlkFileSize:
				print "Error:: TLK file size mismatch with reported size in header for %s!" % (tmpTLKfileTuple[0])
			else:
				#
				# 12 bytes per entry
				#		4 bytes: ID
				#		4 bytes: Offset in data segment
				#		4 bytes: Size of data
				#
				for i in range(0, numOfEntriesToExtract):
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
					if traceModeEnabled:
						print "Entry: %s, offset %s, data segment %s bytes" % (''.join('{:08X}'.format(idOfAUDEntry)), ''.join('{:08X}'.format(offsetOfAUDEntry)),''.join('{:08X}'.format(sizeOfAUDEntry)))
					#
					# put file in AUD object
					# do we need AUD decode?
					# create WAV from AUD
					# write WAV to appropriate output folder
					# Figure out proper naming for file
					# then:
					#	AudFile aud;
					#	aud.loadAudFile(fs); (fs is file stream)
					#	aud.extract_as_wav(fs, offset, offset + int(sizeof(AudHeader)) + aud.header().size_in, target);
					#
					#
					inTLKFile.seek(2 + 4 + 12*numOfEntriesToExtract + offsetOfAUDEntry)
					if(offsetOfAUDEntry + sizeOfAUDEntry > allTlkFileSize):
						print "Error:: AUD file size mismatch with reported size in entry header!"
					else:
						audFileBuffer = inTLKFile.read(sizeOfAUDEntry)
						if (len(audFileBuffer) == sizeOfAUDEntry):
						# load Aud file
							thisAudFile = audFile()
							if (thisAudFile.loadAudFile(audFileBuffer, allTlkFileSize)):
								# print "AUD file load successful!"
								# find
								# print "Emulating Wav write to appropriate folder..."
								(actorID, actorSName, localQuoteId) = getActorShortNameAndLocalQuoteIdByAUDHashID(idOfAUDEntry)
								targetSimpleFileName = actorSName + '_' + str(localQuoteId).zfill(4) + '_' + ''.join('{:08X}'.format(idOfAUDEntry)).upper()+'.WAV'
								#print os.path.join(outputWAVpath, tmpTLKfileTuple[1], targetSimpleFileName)
								if not os.path.isfile(os.path.join(outputWAVpath, tmpTLKfileTuple[1], targetSimpleFileName) ):
									thisAudFile.extract_as_wav(audFileBuffer, os.path.join(outputWAVpath, tmpTLKfileTuple[1], targetSimpleFileName) )
								else:
									if traceModeEnabled:
										print "Info:: Output file %s already exists. Skipping..." % (os.path.join(outputWAVpath, tmpTLKfileTuple[1], targetSimpleFileName))
							else:
								print "Error:: while LOADING aud file!"
						else:
							print "Error:: while reading AUD file %s into mem buffer" % (''.join('{:08X}'.format(idOfAUDEntry)))
			inTLKFile.close()


	# SYS EXIT IS HERE ONLY FOR DEBUG PURPOSES OF PARSING TLK FILES - SHOULD BE COMMENTED OUT NORMALLY
	# sys.exit(0)
	return

def inputMIXExtractTREs(inputMIXpath, excelOutBook = None):
	print "Info:: Checking in %s for MIX files to extract TRE's from" % (inputMIXpath)
	inputMIXFilesFound = []
	# breaking after first for loop yields only the top directory files, which is what we want
	for (dirpath, dirnames, filenames) in walk(inputMIXpath):
		for filename in filenames:
			for mixFileName in supportedMIXInputFiles:
				if filename.upper() == mixFileName:
					inputMIXFilesFound.append(mixFileName)
		break
	for tmpMIXfileName in inputMIXFilesFound:
		if traceModeEnabled:
			print "Found MIX: %s" % ('"' + inputMIXpath + tmpMIXfileName + '"')
		errorFound = False
		inMIXFile = None
		#
		try:
			inMIXFile = open(os.path.join(inputMIXpath,tmpMIXfileName), 'rb')
		except:
			errorFound = True
			print "Error:: Unexpected error:", sys.exc_info()[0]
			raise
		if not errorFound:
			totalTREs = 0
			tmpBuff = inMIXFile.read(2)
			# H: unsigned short (2 bytes) followed by I: unsigned int (4 bytes)
			mixFileEntriesNumTuple = struct.unpack('H', tmpBuff)
			numOfEntriesToExtract = mixFileEntriesNumTuple[0]
			tmpBuff = inMIXFile.read(4)
			mixFileDataSegmentSizeTuple = struct.unpack('I', tmpBuff)
			allMixFileSize = mixFileDataSegmentSizeTuple[0]
			inMIXFile.seek(0, 2) # go to file end
			allActualBytesInMixFile = inMIXFile.tell()
			inMIXFile.seek(6, 0) # go to start of table of MIX file entries (right after the 6 bytes header)
			# 2 + 4 = 6 bytes short MIX header
			# 12 bytes per MIX entry in entries table
			# quick size validation
			if traceModeEnabled:
				print "Entries: %d, data segment %d bytes" % (numOfEntriesToExtract, allMixFileSize)
			if allActualBytesInMixFile != 2 + 4 + 12 * numOfEntriesToExtract + allMixFileSize:
				print "Error:: MIX file size mismatch with reported size in header for %s!" % (tmpMIXfileName)
			else:
				#
				# 12 bytes per entry
				#		4 bytes: ID
				#		4 bytes: Offset in data segment
				#		4 bytes: Size of data
				#
				for i in range(0, numOfEntriesToExtract):
					foundTREFile = False
					currTreFileName = 'UNKNOWN.TRE'
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

					for suppTREFileName in supportedExportedTREFiles:
						if(idOfMIXEntry == calculateFoldHash(suppTREFileName)):
							foundTREFile = True
							currTreFileName = suppTREFileName
							break

					if (foundTREFile == True):
						if traceModeEnabled:
							print "Entry Name: %s, Entry ID: %s, offset %s, data segment %s bytes" % (currTreFileName, ''.join('{:08X}'.format(idOfMIXEntry)), ''.join('{:08X}'.format(offsetOfMIXEntry)),''.join('{:08X}'.format(sizeOfMIXEntry)))
						#
						# IF TRE FILE:
						# put file in TRE object
						#
						#
						inMIXFile.seek(2 + 4 + 12*numOfEntriesToExtract + offsetOfMIXEntry)
						if(offsetOfMIXEntry + sizeOfMIXEntry > allMixFileSize):
							print "Error:: TRE file size mismatch with reported size in entry header!"
						else:
							treFileBuffer = inMIXFile.read(sizeOfMIXEntry)
							if (len(treFileBuffer) == sizeOfMIXEntry):
							# load TRE file
								thisTreFile = treFile()
								if (thisTreFile.loadTreFile(treFileBuffer, allMixFileSize)):
									if traceModeEnabled:
										print "Info:: TRE file loaded"
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
											# (they are out of their order from their proper order in windwos-1252)
											# so we need to create a new string.
											objUTF8SafeStr = ""
											for i in range(0, len(objStr)):
												if (objStr[i] == '\x81'):
													objUTF8SafeStr += 'ü'
												elif (objStr[i] == '\x82'):
													objUTF8SafeStr += 'é'
												else:
													objUTF8SafeStr += objStr[i]
											#objUTF8Safe = objUTF8Safe.replace('\x81',u'u') #'ü' # this does not work
											#objUTF8Safe = objUTF8Safe.replace('\x82',u'e') #'é' # this does not work
											objUTF8Unicode = unicode(objUTF8SafeStr, 'utf-8')
											sh.write(m, 1, objUTF8Unicode)


									#for tupleIdString in thisTreFile.stringEntriesLst:
									#	#print "Id: %d\t Text: %s" % (tupleIdString[0], tupleIdString[1])
									#	pass
									totalTREs =   totalTREs + 1
								else:
									print "Error:: while LOADING TRE file!"
							else:
								print "Error:: while reading TRE file %s into mem buffer" % (''.join('{:08X}'.format(idOfMIXEntry)))
			inMIXFile.close()
			print "Info:: Total TREs: %d " % (totalTREs)
	return


#
# Creating the OUTPUT XLS file with one sheet named as the @param sheet with entries based on the list1 (wav files, without duplicates)
#
def outputXLS(filename, sheet, list1, parseTREResourcesAlso = False, mixInputFolderPath = ''):
	global stringReplacementForRootFolderWithExtractedFiles
	global numReplaceStartingCharacters
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
				if stringReplacementForRootFolderWithExtractedFiles and numReplaceStartingCharacters > 0:
					realPathOfFileNameToLink = realPathOfFileNameToLink.replace(realPathOfFileNameToLink[:numReplaceStartingCharacters], stringReplacementForRootFolderWithExtractedFiles)

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

	book.save(filename)

#
#
#
# ########################
# main
def main(argsCL):
	# TODO parse arguments using argparse? https://docs.python.org/3/library/argparse.html#module-argparse
	global traceModeEnabled
	traceModeEnabled = False

	pathToActorNamesTxt = ""

	TMProotFolderWithExtractedFiles = ""
	TMProotFolderWithInputTLKFiles = ""

	extractWavFilesMode = False
	extractTreFilesMode = False

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
			print "%s takes has one mandatory argument, the folder of the extracted WAV files:" % (app_name)
			print "Valid syntax: %s -op folderpath_for_extracted_wav_Files [-ip folderpath_for_TLK_Files] [-ian path_to_actornames_txt] [-m stringPathToReplaceFolderpathInExcelLinks] [-xwav] [-xtre] [--trace]" % (app_name)
			print "The -op switch has an argument that is the path for extracted WAV files folder. The -op switch is REQUIRED always."
			print "The -ip switch has an argument that is the path for the input (TLK or MIX) files folder (can be the same as the Blade Runner installation folder)."
			print "The -m switch has an argument that is a replacement string for the path to the folder of extracted WAV files which will be used as a prefix for the links in the output XLS file."
			print "The -xwav switch enables the WAV audio extract mode from the TLK files. It requires an INPUT path to be set with the -ip switch."
			print "The -xtre switch enables the TRE parsing mode from the original MIX files. It requires an INPUT path to be set with the -ip switch."
			print "The --trace switch enables more debug messages being printed during execution."
			print "If the app finishes successfully a sortedWavs.xls file will be created in the same folder with the app."
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
				if (argsCL[i] == '-op'):
					TMProotFolderWithExtractedFiles = argsCL[i+1]
					numReplaceStartingCharacters = len(TMProotFolderWithExtractedFiles)
				elif (argsCL[i] == '-ip'):
					TMProotFolderWithInputTLKFiles = argsCL[i+1]
				elif (argsCL[i] == '-m'):
					stringReplacementForRootFolderWithExtractedFiles = argsCL[i+1]
				elif (argsCL[i] == '-ian'):
					pathToActorNamesTxt = argsCL[i+1]
			elif (argsCL[i] == '-xwav'):
				print "Extract WAVs from TLK files mode enabled."
				extractWavFilesMode = True
			elif (argsCL[i] == '-xtre'):
				print "Extract TRE mode enabled."
				extractTreFilesMode = True
			elif argsCL[i] == '--trace':
				print "Info:: Trace mode enabled (more debug messages)."
				traceModeEnabled = True
		if not TMProotFolderWithExtractedFiles: # this argument is mandatory
			invalidSyntax = True

		if (extractWavFilesMode == True or extractTreFilesMode == True) and (TMProotFolderWithInputTLKFiles == ''):
			invalidSyntax = True

		if not invalidSyntax:

			# parse Actors files:
			initActorPropertyEntries(pathToActorNamesTxt)
#			 for actorEntryTmp in actorPropertyEntries:
#				  print "Found actor: %s %s %s" % (actorEntryTmp[0], actorEntryTmp[1], actorEntryTmp[2])
			#
			# Checking for the optional case of parsing the input TLK files to extract to WAV
			#
			if TMProotFolderWithInputTLKFiles != '':
				if (extractWavFilesMode == True):
					inputTLKsExtract(TMProotFolderWithInputTLKFiles, TMProotFolderWithExtractedFiles)
				#if (extractTreFilesMode == True):
				#	inputMIXExtractTREs(TMProotFolderWithInputTLKFiles)
			#
			# Parsing the extracted WAV files
			#
			print "Parsing the extracted WAV audio files. Please wait (it could take a while)..."
			for (dirpath, dirnames, filenames) in walk(TMProotFolderWithExtractedFiles):
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
				 wavfiles.extend(filenames)
#				 break
			for fileIdx, filenameTmp in enumerate(wavfiles):
				twoTokensOfFilenameAndRelDirname = filenameTmp.split('&', 1)
				if len(twoTokensOfFilenameAndRelDirname) != 2:
					print "ERROR in filename and rel dirname split: %s" % (filenameTmp)
					sys.exit(0)
				twoTokensOfFilenameForExt = twoTokensOfFilenameAndRelDirname[0].split('.', 1)
				if len(twoTokensOfFilenameForExt) == 2:
					if twoTokensOfFilenameForExt[1] != 'WAV' and  twoTokensOfFilenameForExt[1] != 'wav':
						print "ERROR in proper extension (not WAV): %s" % (twoTokensOfFilenameAndRelDirname[0])
						sys.exit(0)
				else:
					print "ERROR in extension split: %s" % (twoTokensOfFilenameAndRelDirname[0])
					sys.exit(0)
				#remove WAV extension here
#				 filenameTmp =	twoTokensOfFilenameAndRelDirname[0] + '&' + twoTokensOfFilenameForExt[0]
#				 print "Found %s" % (filenameTmp)

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
							print "ERROR in actorIdMatch match: %s %s" % (tmpActorId, twoTokensOfFilenameForExt[0])
							sys.exit(0)
					else:
					#fatal error if something cannot convert to spot it immediately
						print "ERROR in shorthand match: %s %s" % (threeTokensOfFilename[0], twoTokensOfFilenameForExt[0])
						sys.exit(0)
#
#
#					  foundMatchForActorShortHand = False
#					  for actorEntryTmp in actorPropertyEntries:
#						   if actorEntryTmp[1] == threeTokensOfFilename[0]:
#								 threeTokensOfFilename[0] = actorEntryTmp[0].zfill(2)
#								 threeTokensOfFilename.append(actorEntryTmp[2])
#								 foundMatchForActorShortHand = True
#								 break
					 # end of replace actor name shorthand
					twoTokensOfFilenameForExt[0] = '#'.join(threeTokensOfFilename)
					filenameTmp =  twoTokensOfFilenameForExt[0] + '&' + twoTokensOfFilenameAndRelDirname[1]
					wavfiles[fileIdx] = filenameTmp
				else:
					print "ERROR in spliting tokens on _: %s" % (filenameTmp)
					sys.exit(0)
			#sort in-place
			#
			#
			wavfiles.sort()
#			  #
#			  #
#			  #	 Code for renaming non conforming filenames - just to be consistent in file naming
#			  # TO BE RUN ONCE FOR CONFORMANCE. No NEED TO Re-RUN
#			  # If its run though, comment this section and re-run the tool to get proper links in Excel file
#			  #
#			  for filenameSrcTmp in wavfiles:
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
			for filenameSrcTmp in wavfiles:
				duplicateFound = False
#				  print "Converted %s" % (filenameSrcTmp)
			 # Weed out duplicates by copying to another table (quick and dirty)
				twoTokensOfRelDirnameAndFilenameSrc = filenameSrcTmp.split('&', 2)
				tmpRelDirNameSrc = twoTokensOfRelDirnameAndFilenameSrc[1]
				threeTokensOfQuoteFilenameSrc = twoTokensOfRelDirnameAndFilenameSrc[0].split('#', 2)
				#concatenate actorID and quoteID for search key
				keyForDuplicateSearchSrc =	threeTokensOfQuoteFilenameSrc[0] + threeTokensOfQuoteFilenameSrc[1]
				for fileTargIdx, filenameTargTmp in enumerate(wavfilesNoDups):
					twoTokensOfRelDirnameAndFilenameTarg = filenameTargTmp.split('&', 2)
					tmpRelDirNameTarg = twoTokensOfRelDirnameAndFilenameTarg[1]
					threeTokensOfQuoteFilenameTarg = twoTokensOfRelDirnameAndFilenameTarg[0].split('#', 2)
					#concatenate actorID and quoteID for search key
					keyForDuplicateSearchTarg =	 threeTokensOfQuoteFilenameTarg[0] + threeTokensOfQuoteFilenameTarg[1]
					if(keyForDuplicateSearchSrc == keyForDuplicateSearchTarg):
						#print "Found duplicate %s" % (filenameSrcTmp)
						duplicateFound = True
						wavfilesNoDups[fileTargIdx] = twoTokensOfRelDirnameAndFilenameTarg[0] + '&' +  tmpRelDirNameSrc + ',' + tmpRelDirNameTarg + '&' + twoTokensOfRelDirnameAndFilenameTarg[2]
						break
				if(duplicateFound == False):
					wavfilesNoDups.append(filenameSrcTmp)
#			 for filenameSrcTmp in wavfilesNoDups:
#				 print "Unique %s" % (filenameSrcTmp)

			print "Creating output excel %s file..." % (OUTPUT_XLS_FILENAME)
			outputXLS(OUTPUT_XLS_FILENAME, OUTPUT_XLS_QUOTES_SHEET, wavfilesNoDups, extractTreFilesMode, TMProotFolderWithInputTLKFiles)
	else:
		invalidSyntax = True

	if invalidSyntax == True:
		print "Invalid syntax\n Try: \n %s --help for more info \n %s --version for version info " % (app_name, app_name)
		print "Valid syntax: %s -op folderpath_for_extracted_wav_Files [-ip folderpath_for_TLK_Files] [-ian path_to_actornames_txt] [-m stringPathToReplaceFolderpathInExcelLinks] [-xwav] [-xtre] [--trace]" % (app_name)
		tmpi = 0
		for tmpArg in argsCL:
			if tmpi==0: #skip first argument
				tmpi+=1
				continue
			print "\nArgument: %s" % (tmpArg)
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
	#print 'Debug:: %s was imported from another module' % (app_name)
	pass
