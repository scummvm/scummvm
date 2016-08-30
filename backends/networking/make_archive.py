#!/usr/bin/env python
# encoding: utf-8
import sys
import re
import os
import zipfile

ARCHIVE_FILE_EXTENSIONS = ('.html', '.css', '.js', '.ico', '.png')

def buildArchive(archiveName):
	if not os.path.isdir(archiveName):
		print ("Invalid archive name: " + archiveName)
		return

	zf = zipfile.ZipFile(archiveName + ".zip", 'w')

	print ("Building '" + archiveName + "' archive:")
	os.chdir(archiveName)

	directories = ['.', './icons']
	for d in directories:
		filenames = os.listdir(d)
		filenames.sort()
		for filename in filenames:
			if os.path.isfile(d + '/' + filename) and filename.endswith(ARCHIVE_FILE_EXTENSIONS):
				zf.write(d + '/' + filename, d + '/' + filename)
				print ("    Adding file: " + d + '/' + filename)

	os.chdir('../')

	zf.close()

def main():
	buildArchive("wwwroot")

if __name__ == "__main__":
	sys.exit(main())
