/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef __lure_surface_h__
#define __lure_surface_h__

#include "common/stdafx.h"
#include "common/str.h"
#include "lure/disk.h"
#include "lure/luredefs.h"

using namespace Common;

namespace Lure {

class Surface {
private:
	MemoryBlock *_data;	
	uint16 _width, _height;
public:
	Surface(MemoryBlock *src, uint16 width, uint16 height);
	Surface(uint16 width, uint16 height);		
	~Surface();
	
	static void initialise();
	static void deinitialise();

	uint16 width() { return _width; }
	uint16 height() { return _height; }
	MemoryBlock &data() { return *_data; }

	void loadScreen(uint16 resourceId);
	void writeChar(uint16 x, uint16 y, uint8 ascii, bool transparent, uint8 colour);
	void writeString(uint16 x, uint16 y, Common::String line, bool transparent, 
		uint8 colour = DIALOG_TEXT_COLOUR, bool varLength = true);
	void transparentCopyTo(Surface *dest);
	void copyTo(Surface *dest);
	void copyTo(Surface *dest, uint16 x, uint16 y);
	void copyTo(Surface *dest, const Rect &srcBounds, uint16 destX, uint16 destY,
		int transparentColour = -1);
	void copyFrom(MemoryBlock *src) { _data->copyFrom(src); }
	void copyFrom(MemoryBlock *src, uint32 destOffset);
	void empty() { _data->empty(); }
	void fillRect(const Rect &r, uint8 colour);
	void createDialog(bool blackFlag = false);
	void copyToScreen(uint16 x, uint16 y);
	void centerOnScreen();

	static uint16 textWidth(const char *s, int numChars = 0);
	static Surface *newDialog(uint16 width, uint8 numLines, char **lines, bool varLength = true, uint8 colour = DIALOG_TEXT_COLOUR);
	static Surface *newDialog(uint16 width, const char *lines, uint8 colour = DIALOG_TEXT_COLOUR);
	static Surface *getScreen(uint16 resourceId);
};

class Dialog {
public:
	static void show(const char *text);
	static void show(uint16 stringId);
	static void showMessage(uint16 messageId, uint16 characterId);
};

} // End of namespace Lure

#endif
