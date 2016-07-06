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

	filenames = os.listdir('.')
	filenames.sort()
	for filename in filenames:
		if os.path.isfile(filename) and filename.endswith(ARCHIVE_FILE_EXTENSIONS):
			zf.write(filename, './' + filename)
			print ("    Adding file: " + filename)

	os.chdir('../')

	zf.close()

def main():
	buildArchive("wwwroot")

if __name__ == "__main__":
	sys.exit(main())
