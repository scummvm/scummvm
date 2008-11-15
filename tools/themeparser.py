#!/usr/bin/env python
# encoding: utf-8

"""
 " ScummVM - Graphic Adventure Engine
 "
 " ScummVM is the legal property of its developers, whose names
 " are too numerous to list here. Please refer to the COPYRIGHT
 " file distributed with this source distribution.
 "
 " This program is free software; you can redistribute it and/or
 " modify it under the terms of the GNU General Public License
 " as published by the Free Software Foundation; either version 2
 " of the License, or (at your option) any later version.
 "
 " This program is distributed in the hope that it will be useful,
 " but WITHOUT ANY WARRANTY; without even the implied warranty of
 " MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 " GNU General Public License for more details.
 "
 " You should have received a copy of the GNU General Public License
 " along with this program; if not, write to the Free Software
 " Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 "
 " $URL$
 " $Id$
"""

from __future__ import with_statement
import os
import xml.dom.minidom as DOM
import struct
from UserDict import UserDict

def pbin(data):
	return str(map(lambda c: hex(ord(c)), data))
#	return " ".join(["%0.2X" % ord(c) for c in data])

class STXBinaryFile:
	class InvalidRGBColor(Exception):
		pass
		
	class InvalidResolution(Exception):
		pass
		
	class DrawStepData:
		def __init__(self, isDefault, packFormat, function):
			self.isDefault = isDefault
			self.packFormat = packFormat
			self.parse = function
		
	BYTEORDER = '>' # big-endian format by default, platform independent
		
	TRUTH_VALUES = {"" : 0, "true" : 1, "false" : 0, "True" : 1, "False" : 0}
		
	BLOCK_HEADERS = {
		"palette" 	: 0x0100000A,
		"bitmaps" 	: 0x0100000B,
		"fonts"		: 0x0100000C,
		"defaults"	: 0x0100000D,
		"cursor"	: 0x0100000E,
		"drawdata"	: 0x0100000F
	}
	
	DS_triangleOrientations = {"top" : 0x1, "bottom" : 0x2, "left" : 0x3, "right" : 0x4}
	DS_fillModes = {"none" : 0x0, "foreground" : 0x1, "background" : 0x2, "gradient" : 0x3}
	DS_vectorAlign = {"left" : 0x1, "right" : 0x2, "bottom" : 0x3, "top" : 0x4, "center" : 0x5}
   
	DS_functions = {
		"void" : 0x0, "circle" : 0x1, "square" : 0x2, "roundedsq" : 0x3, "bevelsq" : 0x4, 
   		"line" : 0x5, "triangle" : 0x6, "fill" : 0x7, "tab" : 0x8, "bitmap" : 0x9, "cross" : 0xA
	}
	
	DS_list = [
		"func", "fill", "stroke", "gradient_factor",
		"width", "height", "xpos", "ypos",
		"radius", "bevel", "shadow", "orientation",
		"fg_color", "bg_color", "gradient_start", "gradient_end", "bevel_color",
		"file"		
	]
	
	def __init__(self, themeName, autoLoad = True, verbose = False):
		self._themeName = themeName
		self._stxFiles = []
		self._verbose = verbose
		
		self.DS_data = {
		#	attribute name						isDefault	pack	parse function
			"func"				: self.DrawStepData(False,	"B",	lambda f: self.DS_functions[f]),
			"fill"				: self.DrawStepData(True, 	"B",	lambda f: self.DS_fillModes[f]),
			"stroke" 			: self.DrawStepData(True,	"B",	int),
			"gradient_factor" 	: self.DrawStepData(True, 	"B",	int),
			"width"				: self.DrawStepData(False, 	"i", 	lambda w: -1 if w == 'height' else 0 if w == 'auto' else int(w)),
			"height"			: self.DrawStepData(False, 	"i",	lambda h: -1 if h == 'width' else 0 if h == 'auto' else int(h)),
			"xpos"				: self.DrawStepData(False,	"i",	lambda pos: self.DS_vectorAlign[pos] if pos in self.DS_vectorAlign else int(pos)),
			"ypos"				: self.DrawStepData(False,	"i", 	lambda pos: self.DS_vectorAlign[pos] if pos in self.DS_vectorAlign else int(pos)),
			"radius"			: self.DrawStepData(False,	"B",	lambda r: 0xFF if r == 'auto' else int(r)),
			"bevel" 			: self.DrawStepData(True, 	"B", 	int),
			"shadow" 			: self.DrawStepData(True, 	"B", 	int),
			"orientation"		: self.DrawStepData(False,	"B", 	lambda o: self.DS_triangleOrientations[o]),
			"fg_color"			: self.DrawStepData(True, 	"4s",	self.__parseColor),
			"bg_color"			: self.DrawStepData(True, 	"4s",	self.__parseColor),
			"gradient_start"	: self.DrawStepData(True, 	"4s",	self.__parseColor),
			"gradient_end"		: self.DrawStepData(True, 	"4s",	self.__parseColor),
			"bevel_color"		: self.DrawStepData(True, 	"4s",	self.__parseColor),
			"file"				: self.DrawStepData(False,	"%ds",	str),
		}
		
		if autoLoad:
			if not os.path.isdir(themeName) or not os.path.isfile(os.path.join(themeName, "THEMERC")):
				raise IOError
				
			for filename in os.listdir(themeName):
				filename = os.path.join(themeName, filename)
				if os.path.isfile(filename) and filename.endswith('.stx'):
					self._stxFiles.append(filename)
					
	def debug(self, text):
		if self._verbose: print text
		
	def debugBinary(self, data):
		if self._verbose:
			print "BINARY OUTPUT (%d bytes):" % len(data), " ".join(["%0.2X" % ord(c) for c in data])
			
	def addSTXFile(self, filename):
		if not os.path.isfile(filename):
			raise IOError
		else:
			self._stxFiles.append(filename) 
		
	def parse(self):
		if not self._stxFiles:
			self.debug("No files have been loaded for parsing on the theme.")
			raise IOError
		
		for f in self._stxFiles:
			self.debug("Parsing %s." % f)
			with open(f) as stxFile:
				self.__parseFile(stxFile)
	
	def __parseFile(self, xmlFile):
		stxDom = DOM.parse(xmlFile)
		
		for layout in stxDom.getElementsByTagName("layout_info"):
			self.__parseLayout(layout)

		for render in stxDom.getElementsByTagName("render_info"):
			self.__parseRender(render)
		
		stxDom.unlink()
		
	def __parseDrawStep(self, drawstepDom, localDefaults = {}):
		
		dstable = {}
		
		if drawstepDom.tagName == "defaults":
			isGlobal = drawstepDom.parentNode.tagName == "render_info"
			
			for ds in self.DS_list:
				if self.DS_data[ds].isDefault and drawstepDom.hasAttribute(ds):
					dstable[ds] = self.DS_data[ds].parse(drawstepDom.getAttribute(ds))
					
				elif isGlobal:
					dstable[ds] = None
		
		else:
			for ds in self.DS_data:
				if drawstepDom.hasAttribute(ds):
					dstable[ds] = self.DS_data[ds].parse(drawstepDom.getAttribute(ds))
				elif self.DS_data[ds].isDefault:
					dstable[ds] = localDefaults[ds] if ds in localDefaults else self._globalDefaults[ds]
				else:
					dstable[ds] = None
		
		return dstable
		
		
	def __parseDrawStepToBin(self, stepDict):
		"""
			/BBBBiiiiBBBB4s4s4s4s4ss/ == 32 + 4 * 32 + 32 + 5 * 32 = 352 / 4 bits = 88 bytes + var string
			function 		(byte)
			fill 			(byte)
			stroke			(byte)
			gradient_factor	(byte)
			width			(int32)
			height			(int32)
			xpos			(int32)
			ypos			(int32)
			radius			(byte)
			bevel			(byte)
			shadow			(byte)
			orientation 	(byte)
			fg_color		(4 byte)
			bg_color		(4 byte)
			gradient_start 	(4 byte)
			gradient_end	(4 byte)
			bevel_color		(4 byte)
			file			(byte string, null terminated)
		"""
		
		packLayout = ""
		packData = []
		
		for ds in self.DS_list:
			layout = self.DS_data[ds].packFormat
			data = stepDict[ds]
			
			if layout == "%ds":
				if data:
					packLayout += layout % (len(data) + 1)
					packData.append(data)
				else:
					packLayout +=  "B"
					packData.append(0)
			elif not data:
				size = struct.calcsize(layout)
				packLayout += "B" * size
				
				for d in xrange(size):
					packData.append(0)
			else:
				packLayout += layout
				packData.append(data)
			
					
		stepBin = struct.pack(self.BYTEORDER + packLayout, *tuple(packData))
#		self.debugBinary(stepBin)
		return stepBin
			
		
	def __parseResolutionToBin(self, resString):
		"""
			/i xxxbII xxxbII xxxbII/ == 32 bits + x * 32 bits
			number of resolution sections (int32)
			padding (byte)
			exclude resolution (byte)
			resolution X (int32)
			resolution Y (int32)
		"""
		
		if resString == "":
			return struct.pack(self.BYTEORDER + "ixxxbII", 1, 0, 0, 0)
			
		resolutions = resString.split(", ")
		packFormat = "i" + "xxxbII" * len(resolutions)
		packData = [len(resolutions)]
			
		for res in resolutions:
			exclude = 0
			if res[0] == '-':
				exclude = 1
				res = res[1:]
			
			try:
				x, y = res.split('x')
				x = 0 if x == 'X' else int(x)
				y = 0 if y == 'Y' else int(y)
			except ValueError:
				raise InvalidResolution
			
			packData.append(exclude)
			packData.append(x)
			packData.append(y)
		
		buff = struct.pack(self.BYTEORDER + packFormat, *tuple(packData))
		self.debug("Pack format: %s => %s" % (packFormat, str(packData)))
#		self.debugBinary(buff)
		return buff
		
	def __parseRGBToBin(self, color):
		"""
			/xBBB/ == 32 bits
			padding (byte)
			red color (byte)
			green color (byte)
			blue color (byte)
		"""
		
		try:
			rgb = tuple(map(int, color.split(", ")))
		except ValueError:
			raise self.InvalidRGBColor
		
		if len(rgb) != 3:
			raise self.InvalidRGBColor
			
		for c in rgb:
			if c < 0 or c > 255:
				raise self.InvalidRGBColor
				
		rgb = struct.pack(self.BYTEORDER + "xBBB", *tuple(rgb))
		
#		self.debugBinary(rgb)
		return rgb
		
	def __parseColor(self, color):
		try:
			color = self.__parseRGBToBin(color)
		except self.InvalidRGBColor:
			if color not in self._colors:
				raise self.InvalidRGBColor
			color = self._colors[color]
			
		return color
		
		
	def __parsePalette(self, paletteDom):
		self._colors = {}		
		
		for color in paletteDom.getElementsByTagName("color"):
			color_name = color.getAttribute('name')
			color_rgb = self.__parseRGBToBin(color.getAttribute('rgb'))
			
			self._colors[color_name] = color_rgb
			self.debug("COLOR: %s" % (color_name))
			
	
	def __parseBitmaps(self, bitmapsDom):
		self._bitmaps = []
		
		for bitmap in bitmapsDom.getElementsByTagName("bitmap"):
			bmpName = bitmap.getAttribute("filename")
			resolution = self.__parseResolutionToBin(bitmap.getAttribute("resolution"))
			
			self._bitmaps.append((bmpName, resolution))
			self.debug("BITMAP: %s" % bmpName)
			
	def __parseFonts(self, fontsDom):
		self._fonts = []
		
		for font in fontsDom.getElementsByTagName("font"):
			ident = font.getAttribute("id")
			color = self.__parseColor(font.getAttribute("color"))
			filename = font.getAttribute("file")
			
			resolution = self.__parseResolutionToBin(font.getAttribute("resolution"))
			self.debug("FONT: %s @ %s" % (ident, filename))
			self._fonts.append((ident, filename, color, resolution))
			
	def __parseTextToBin(self, textDom):
		return ""
			
	def __parseDrawData(self, ddDom):
		"""
			/IsIBBHss/
				Section Header (uint32)
				Resolution (byte array, word-aligned)
				DrawData id hash (uint32)
				Cached (byte)
				has text section? (byte)
				number of DD sections (uint16)
				** text segment (byte array)
				drawstep segments (byte array)
		"""
				
		
		localDefaults = ddDom.getElementsByTagName("defaults")
		localDefaults = localDefaults[0] if localDefaults else {}
		
		stepList = []
		
		for ds in ddDom.getElementsByTagName("drawstep"):
			dstable = self.__parseDrawStep(ds, localDefaults)
			dsbinary = self.__parseDrawStepToBin(dstable)
			
			stepList.append(dsbinary)

		stepByteArray = "".join(stepList)
		
		resolution = self.__parseResolutionToBin(ddDom.getAttribute("resolution"))
		
		text = ddDom.getElementsByTagName("text")
		text = self.__parseTextToBin(text[0]) if text else ""
		
		id_hash = str.__hash__(str(ddDom.getAttribute("id")))
		cached = self.TRUTH_VALUES[ddDom.getAttribute("cached")]
			
		ddBinary = struct.pack(self.BYTEORDER + \
			"I%dsiBBH%ds%ds" % (len(resolution), len(text), len(stepByteArray)),
			
			self.BLOCK_HEADERS['drawdata'], # Section Header (uint32)
			resolution,						# Resolution (byte array, word-aligned)
			id_hash,						# DrawData id hash (uint32)
			cached,							# Cached (byte)
			0x1 if text else 0x0,			# has text section? (byte)
			len(stepList),					# number of DD sections (uint16)
			text,							# ** text segment (byte array)
			stepByteArray					# drawstep segments (byte array)
		)
		
		self.debug("DRAW DATA %s (%X): \n" % (ddDom.getAttribute("id"), id_hash) + pbin(ddBinary) + "\n\n")
		return ddBinary
		
		
	def __parseLayout(self, layoutDom):
		self.debug("GLOBAL SECTION: LAYOUT INFO.")
		
	def __parseRender(self, renderDom):
		self.debug("GLOBAL SECTION: RENDER INFO.")
		
		paletteDom = renderDom.getElementsByTagName("palette")[0]
		bitmapsDom = renderDom.getElementsByTagName("bitmaps")[0]
		fontsDom = renderDom.getElementsByTagName("fonts")[0]
		defaultsDom = renderDom.getElementsByTagName("defaults")[0]
		
		self.__parsePalette(paletteDom)
		self.__parseBitmaps(bitmapsDom)
		self.__parseFonts(fontsDom)
		
		self._globalDefaults = self.__parseDrawStep(defaultsDom)
		
		for dd in renderDom.getElementsByTagName("drawdata"):
			self.__parseDrawData(dd)


if __name__ == '__main__':
	bin = STXBinaryFile('../gui/themes/scummmodern', True, True)
	bin.parse()
	