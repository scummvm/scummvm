#!/usr/bin/env python
# encoding: utf-8
import sys
import re
import os
import zipfile
import argparse

def buildArchive(archiveName, dest_file="fonts.dat"):
    if not os.path.isdir(archiveName):
        print("Invalid archive name: " + archiveName)
        return

    zf = zipfile.ZipFile(dest_file, 'w', zipfile.ZIP_DEFLATED)

    print("Building 'fonts' archive:")

    filenames = os.listdir(archiveName)
    filenames.sort()
    for filename in filenames:
        if os.path.isfile(archiveName + '/' + filename):
            zf.write(archiveName + '/' + filename, filename)
            print("    Adding file: " + './' + filename)

    zf.close()

def main():
    parser = argparse.ArgumentParser(description='Generates fonts archive')
    parser.add_argument('path', nargs='?', help='path where the fonts source data can be found')
    parser.add_argument('destination_file', nargs='?', help='path where the destination file can be found')
    args = parser.parse_args()

    if args.path != None:
        if not os.path.isdir(args.path):
            print ("Directory '" + args.path + "' does not exist!")
            return
        else:
            archiveName = args.path

    if args.destination_file != None:
        buildArchive(archiveName, args.destination_file)
    else:
        buildArchive(archiveName)

if __name__ == "__main__":
    sys.exit(main())
