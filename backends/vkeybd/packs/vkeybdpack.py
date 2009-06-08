#!/usr/bin/env python
# encoding: utf-8
import sys
import re
import os
import zipfile

PACK_FILE_EXTENSIONS = ('.xml', '.bmp')

def buildPack(packName):
	if not os.path.isdir(packName):
		print ("Invalid pack name: " + packName)
		return
	
	zf = zipfile.ZipFile(packName + ".zip", 'w')
	
	print ("Building '" + packName + "' pack:")
	os.chdir(packName)

	for filename in os.listdir('.'):
		if os.path.isfile(filename) and not filename[0] == '.' and filename.endswith(PACK_FILE_EXTENSIONS):
			zf.write(filename, './' + filename)
			print ("    Adding file: " + filename)
			
	os.chdir('../')
	
	zf.close()

def buildAllPacks():
	for f in os.listdir('.'):
		if os.path.isdir(os.path.join('.', f)) and not f[0] == '.':
			buildPack(f)
			
def parseSTX(pack_file, def_file):
	comm = re.compile("<!--(.*?)-->", re.DOTALL)
	head = re.compile("<\?(.*?)\?>")

	output = ""
	for line in pack_file:
		output +=  line.rstrip("\r\n\t ").lstrip() + " \n"
	
	output = re.sub(comm, "", output)
	output = re.sub(head, "", output)
	output = output.replace("\t", " ").replace("  ", " ").replace("\"", "'")
	output = output.replace(" = ", "=").replace(", ", ",")
		
	for line in output.splitlines():
		if line and not line.isspace():
			def_file.write("\"" + line + "\"\n")
	
def printUsage():
	print ("===============================================")
	print ("ScummVM Virtual Keyboard Pack Generation Script")
	print ("===============================================")
	print ("Usage:")
	print ("vkeybdpack.py makeall")
	print ("    Builds all the available pack.\n")
	print ("vkeybdpack.py make [packname]")
	print ("    Builds the pack called 'packname'.\n")

def main():
		
	if len(sys.argv) == 2 and sys.argv[1] == "makeall":
		buildAllPacks()
		
	elif len(sys.argv) == 3 and sys.argv[1] == "make":
		buildPack(sys.argv[2])
		
	else:
		printUsage()

if __name__ == "__main__":
	sys.exit(main())
