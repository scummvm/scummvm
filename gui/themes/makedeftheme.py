#!/usr/bin/env python
# encoding: utf-8
import sys
import re

def main():
	theme_file = open(sys.argv[1], "r")
	def_file = open("default.inc", "w")
	comm = re.compile("\/\*(.*?)\*\/", re.DOTALL)
	
	try:
		output = ""
		for line in theme_file:
			output +=  line.rstrip("\r\n\t ").lstrip() + " \n"
		
		output = re.sub(comm, "", output).replace("\t", " ").replace("  ", " ").splitlines()
		
		for line in output:
			if line and not line.isspace():
				def_file.write("\"" + line + "\"\n") 
	finally:
		theme_file.close()
		def_file.close()

if __name__ == "__main__":
	sys.exit(main())
