#!/usr/bin/python
# Amiga AGI game palette extractor.
# Extracts palette from an Amiga AGI game's executable file.
# Initial version written during summer of 2007 by Buddha^.
# Unoptimized.
import struct, sys, os.path

# Constants
maxComponentValue     = 0x0F
colorsPerPalette      = 16
componentsPerColor    = 3
bytesPerComponent     = 2
bytesPerColor         = componentsPerColor    * bytesPerComponent
paletteComponentCount = colorsPerPalette      * componentsPerColor
paletteByteCount      = paletteComponentCount * bytesPerComponent
black                 = tuple([0 for x in range(componentsPerColor)])
white                 = tuple([maxComponentValue for x in range(componentsPerColor)])
componentPrintFormat  = "0x%1X"

def is12BitColor(color):
	"""Is color 12-bit (i.e. 4 bits per color component)?"""
	return color == filter(lambda component: 0 <= component <= maxComponentValue, color)

def printColor(color, tabulate = True, printLastComma = True, newLine = True):
	"""Prints color with optional start tabulation, comma in the end and a newline"""
	result = ""
	if tabulate:
		result = result + "\t"
	for component in color[:-1]:
		result = result + ((componentPrintFormat + ", ") % component)
	result = result + (componentPrintFormat % color[-1])
	if printLastComma:
		result = result + ","
	if newLine:
		print result
	else:
		print result,

def printPalette(palette, arrayName):
	"""Prints out palette as a C-style array"""
	print "/** A 16-color, 12-bit RGB palette from an Amiga AGI game. */"
	print "static const unsigned char " + arrayName + "[] = {"
	for color in palette[:-1]:
		printColor(color)
	printColor(palette[-1], printLastComma = False)
	print("};")

def isAmigaPalette(palette):
	"""Test if the given palette is an Amiga-style palette"""
	# Palette must be of correct size
	if len(palette) != colorsPerPalette:
		return False
	
	# First palette color must be black, last color white
	if palette[0] != black or palette[-1] != white:
		return False
	
	# All colors must be 12-bit (i.e. 4 bits per color component)
	for color in palette:
		if not is12BitColor(color):
			return False
	
	# All colors must be unique
	if len(set(palette)) != colorsPerPalette:
		return False
	
	return True

# The main program starts here
if len(sys.argv) <= 2 or sys.argv[1] == "-h" or sys.argv[1] == "--help":
	quit("Usage: " + os.path.basename(sys.argv[0]) + " FILE ARRAYNAME\n" \
		"  Searches FILE for an Amiga AGI game palette\n" \
		"  and prints out any found candidates as C-array style arrays with the name ARRAYNAME.");

# Open file and read it into memory
file = open(sys.argv[1], 'rb')
data = file.read()
foundPalettes = []
palette = [black for x in range(colorsPerPalette)]
# Search through the whole file
for searchPosition in range(len(data) - paletteByteCount + 1):
	# Parse possible palette from byte data
	for colorNum in range(colorsPerPalette):
		paletteStart = searchPosition + colorNum * bytesPerColor
		paletteEnd   = paletteStart + bytesPerColor
		# Parse color components as unsigned 16-bit big endian integers
		color = struct.unpack('>' + 'H' * componentsPerColor, data[paletteStart:paletteEnd]);
		palette[colorNum] = color;

	# Save good candidates to a list
	if isAmigaPalette(palette):
		foundPalettes.append(tuple(palette));

# Close file
file.close()

# Remove duplicates from found palettes list
uniqueFoundPalettes = set(foundPalettes)

# Print out the found unique palettes
if len(uniqueFoundPalettes) > 0:
	for palette in uniqueFoundPalettes:
		printPalette(palette, sys.argv[2])
else:
	print "Didn't find any Amiga AGI game palettes in file " + sys.argv[1]
	print "Heuristic isn't perfect though, so caveat emptor"
