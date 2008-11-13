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

class STXBinaryFile:
	class InvalidRGBColor(Exception):
		pass
		
	class InvalidResolution(Exception):
		pass
		
	BLOCK_HEADERS = {
		"palette" 	: 0x0100000A,
		"bitmaps" 	: 0x0100000B,
		"fonts"		: 0x0100000C,
		"defaults"	: 0x0100000D,
		"cursor"	: 0x0100000E,
		"drawdata"	: 0x0100000F
	}
	
	DRAWSTEP_FORMAT_DEF = [
		"stroke", 		"shadow", 	
		"bevel", 		"factor", 
		"fg_color", 	"bg_color",	
		"fill",			"gradient_start", 
		"gradient_end", "bevel_color", 
		"gradient_factor", 
	]
	
	DRAWSTEP_FORMAT = [
		"func",		"radius", 	"width", 	"height", 
		"xpos", 	"ypos", 	"file",		"orientation",
	] + DRAWSTEP_FORMAT_DEF
	
	def __init__(self, themeName, autoLoad = True, verbose = False):
		self._themeName = themeName
		self._stxFiles = []
		self._verbose = verbose
		
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
		
	def __parseDrawStep(self, drawstepDom):
		pass
		
		
	def __parseDrawStepToBin(self, stepDict, isDefault):
		DRAWSTEP_BININFO = {
			"stroke" : "B",		"shadow" 	: "B", 		"bevel" 	: "B", 
			"factor" : "B", 	"fg_color" 	: "BBB", 	"bg_color" 	: "BBB",
			"fill" 	 : "B", 	"func" 		: "B", 		"radius" 	: "i", 
			"width"  : "i", 	"height" 	: "i",		"xpos" 		: "i", 
			"ypos" 	 : "i", 	"orientation" : "B", 	"file" 		: "B%ds",
			
			"gradient_start" 	: "BBB",	"gradient_end" 		: "BBB", 
			"bevel_color" 		: "BBB",	"gradient_factor" 	: "B", 
		}
		
		packLayout = ""
		packData = []
		attributes = DRAWSTEP_FORMAT_DEF if isDefault else DRAWSTEP_FORMAT
		
		for attr in attributes:
			layout = DRAWSTEP_BININFO[attr]
			data = stepDict[attr]
			
			if layout == "B%ds":
				packLayout += layout % (len(data) + 1)
				packData.append(len(data))
				packData.append(data)
				
			else:
				packLayout += stepDict[attr]
				
				if isinstance(data, tuple):
					for d in data:
						packData.append(d)
				else:
					packData.append(data)
					
		stepBin = struct.pack(packLayout, *tuple(packData))
		self.debugBinary(stepBin)
		return stepBin
			
		
	def __parseResolutionToBin(self, resString):
		"""
			/b bII bII bII/
			b => number of resolution sections
			bII => exclude byte, X resolution, Y resolution
		"""
		
		if resString == "":
			return struct.pack("bbII", 1, 0, 0, 0)
			
		resolutions = resString.split(", ")
		packFormat = "b" + "bII" * len(resolutions)
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
		
		buff = struct.pack(packFormat, *tuple(packData))
		self.debug("Pack format: %s => %s" % (packFormat, str(packData)))
		self.debugBinary(buff)
		return buff
		
	def __parseRGBToBin(self, color):
		"""
			/BBB/
			B => red color byte
			B => green color byte
			B => blue color byte
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
				
		rgb = struct.pack("BBB", *tuple(rgb))
		
		self.debugBinary(rgb)
		return rgb
		
	def __parseColors(self, paletteDom):
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
			
			color = font.getAttribute("color")
			try:
				color = self.__parseRGBToBin(color)
			except self.InvalidRGBColor:
				if color not in self._colors:
					raise self.InvalidRGBColor
				color = self._colors[color]
				
			filename = font.getAttribute("file")
#			if filename != "default" and not os.path.isfile(os.path.join(self._themeName, filename)):
#				raise IOError
			
			resolution = self.__parseResolutionToBin(font.getAttribute("resolution"))
			self.debug("FONT: %s @ %s" % (ident, filename))
			self._fonts.append((ident, filename, color, resolution))
		
		
	def __parseLayout(self, layoutDom):
		self.debug("GLOBAL SECTION: LAYOUT INFO.")
		
	def __parseRender(self, renderDom):
		self.debug("GLOBAL SECTION: RENDER INFO.")
		
		paletteDom = renderDom.getElementsByTagName("palette")[0]
		bitmapsDom = renderDom.getElementsByTagName("bitmaps")[0]
		fontsDom = renderDom.getElementsByTagName("fonts")[0]
		defaultsDom = renderDom.getElementsByTagName("defaults")[0]
		
		self.__parseColors(paletteDom)
		self.__parseBitmaps(bitmapsDom)
		self.__parseFonts(fontsDom)


if __name__ == '__main__':
	bin = STXBinaryFile('scummmodern', True, True)
	bin.parse()
	