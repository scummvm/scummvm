#!/usr/bin/env python
# encoding: utf-8
import sys
import re

def main():
	theme_file = open(sys.argv[1], "r")
	def_file = open("default.inc", "w")
	
	try:
		output = "\""
		for line in theme_file:
			output += (line.rstrip("\n\r ").lstrip() + " ")
		
		output = re.sub("\/\*(.*?)\*\/", "", output)
		def_file.write(output + "\"\n")
	finally:
		theme_file.close()
		def_file.close()

if __name__ == "__main__":
	sys.exit(main())
