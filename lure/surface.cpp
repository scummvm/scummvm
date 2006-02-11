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

#include "lure/surface.h"
#include "lure/decode.h"
#include "lure/system.h"
#include "lure/events.h"
#include "lure/screen.h"
#include "lure/room.h"
#include "lure/strings.h"

namespace Lure {

// These variables hold resources commonly used by the Surfaces, and must be initialised and freed
// by the static Surface methods initialise and deinitailse

static MemoryBlock *int_font = NULL;
static MemoryBlock *int_dialog_frame = NULL;
static uint8 fontSize[NUM_CHARS_IN_FONT];

void Surface::initialise() {
	int_font = Disk::getReference().getEntry(FONT_RESOURCE_ID);
	int_dialog_frame = Disk::getReference().getEntry(DIALOG_RESOURCE_ID);

	// Calculate the size of each font character
	for (int ctr = 0; ctr < NUM_CHARS_IN_FONT; ++ctr) {
		byte *pChar = int_font->data() + (ctr * 8);
		fontSize[ctr] = 0;

		for (int yp = 0; yp < FONT_HEIGHT; ++yp) 
		{
			byte v = *pChar++;

			for (int xp = 0; xp < FONT_WIDTH; ++xp) {
				if ((v & 0x80) && (xp > fontSize[ctr])) 
					fontSize[ctr] = xp;
				v = (v << 1) & 0xff;
			}
		}

		// If character is empty, like for a space, give a default size
		if (fontSize[ctr] == 0) fontSize[ctr] = 2;
	}
}

void Surface::deinitialise() {
	delete int_font;
	delete int_dialog_frame;
}

/*--------------------------------------------------------------------------*/

Surface::Surface(MemoryBlock *src, uint16 wdth, uint16 hght): _data(src), 
		_width(wdth), _height(hght) {
	if ((uint32) (wdth * hght) != src->size())
		error("Surface dimensions do not match size of passed data");
}

Surface::Surface(uint16 wdth, uint16 hght): _data(Memory::allocate(wdth*hght)),
	_width(wdth), _height(hght) {
}

Surface::~Surface() {
	delete _data;
}

void Surface::loadScreen(uint16 resourceId) {
	MemoryBlock *rawData = Disk::getReference().getEntry(resourceId);
	PictureDecoder decoder;
	MemoryBlock *tmpScreen = decoder.decode(rawData, FULL_SCREEN_HEIGHT * FULL_SCREEN_WIDTH);
	delete rawData;
	empty();
	copyFrom(tmpScreen, MENUBAR_Y_SIZE * FULL_SCREEN_WIDTH);
	
	delete tmpScreen;
}

void Surface::writeChar(uint16 x, uint16 y, uint8 ascii, bool transparent, uint8 colour) {
	byte *const addr = _data->data() + (y * _width) + x;

	if ((ascii < 32) || (ascii >= 32 + NUM_CHARS_IN_FONT))
		error("Invalid ascii character passed for display '%d'", ascii);
	
	uint8 v;
	byte *pFont = int_font->data() + ((ascii - 32) * 8);
	byte *pDest;
	uint8 charWidth = 0;

	for (int y1 = 0; y1 < 8; ++y1) {
		v = *pFont++;
		pDest = addr + (y1 * _width);

		for (int x1 = 0; x1 < 8; ++x1, ++pDest) {
			if (v & 0x80) {
				*pDest = colour;
				if (x1+1 > charWidth) charWidth = x1 + 1;
			}
			else if (!transparent) *pDest = 0;
			v = (v << 1) & 0xff;
		}
	}
}

void Surface::writeString(uint16 x, uint16 y, Common::String line, bool transparent, 
						  uint8 colour, bool varLength) {
	const char *sPtr = line.c_str();

	while (*sPtr) {
		writeChar(x, y, (uint8) *sPtr, transparent, colour);

		// Move to after the character in preparation for the next character
		if (!varLength) x += FONT_WIDTH;
		else x += fontSize[*sPtr - ' '] + 2;

		++sPtr;		// Move to next character
	}
}

void Surface::transparentCopyTo(Surface *dest) {
	if (dest->width() != _width) 
		error("Incompatible surface sizes for transparent copy");

	byte *pSrc = _data->data();
	byte *pDest = dest->data().data();
	uint16 numBytes = MIN(_height,dest->height()) * FULL_SCREEN_WIDTH;

	while (numBytes-- > 0) {
		if (*pSrc) *pDest = *pSrc;
		
		++pSrc;
		++pDest;
	}
}

void Surface::copyTo(Surface *dest)
{
	copyTo(dest, 0, 0);
}

void Surface::copyTo(Surface *dest, uint16 x, uint16 y)
{
	if ((x == 0) && (dest->width() == _width)) {
		// Use fast data transfer
		uint32 dataSize = dest->data().size() - (y * _width);
		if (dataSize > _data->size()) dataSize = _data->size();
		dest->data().copyFrom(_data, 0, y * _width, dataSize);
	} else {
		// Use slower transfer
		Rect rect;
		rect.left = 0; rect.top = 0;
		rect.right = _width-1; rect.bottom = _height-1;
		copyTo(dest, rect, x, y);
	}
}

void Surface::copyTo(Surface *dest, const Rect &srcBounds, 
					 uint16 destX, uint16 destY, int transparentColour) {
	for (uint16 y=0; y<=(srcBounds.bottom-srcBounds.top); ++y) {
		const uint32 srcPos = (srcBounds.top + y) * _width + srcBounds.left;
		const uint32 destPos = (destY+y) * dest->width() + destX;
		
		uint16 numBytes = srcBounds.right-srcBounds.left+1;
		if (transparentColour == -1) {
			// No trnnsparent colour, so copy all the bytes of the line
			dest->data().copyFrom(_data, srcPos, destPos, numBytes);
		} else {
			byte *pSrc = _data->data() + srcPos;
			byte *pDest = dest->data().data() + destPos;

			while (numBytes-- > 0) {
				if (*pSrc != (uint8) transparentColour)
					*pDest = *pSrc;
				++pSrc;
				++pDest;
			}
		}
	}
}

void Surface::copyFrom(MemoryBlock *src, uint32 destOffset) {
	uint32 size = _data->size() - destOffset;
	if (src->size() > size) size = src->size();
	_data->copyFrom(src, 0, destOffset, size);
}

// fillRect
// Fills a rectangular area with a colour

void Surface::fillRect(const Rect &r, uint8 colour) {
	for (int yp = r.top; yp <= r.bottom; ++yp) {
		byte *const addr = _data->data() + (yp * _width) + r.left;
		memset(addr, colour, r.width());
	}
}

// createDialog
// Forms a dialog encompassing the entire surface

void copyLine(byte *pSrc, byte *pDest, uint16 leftSide, uint16 center, uint16 rightSide) {
	// Left area
	memcpy(pDest, pSrc, leftSide); 
	pSrc += leftSide; pDest += leftSide; 
	// Center area
	memset(pDest, *pSrc, center);
	++pSrc; pDest += center; 
	// Right side
	memcpy(pDest, pSrc, rightSide); 
	pSrc += rightSide; pDest += rightSide; 
}

void Surface::createDialog(bool blackFlag) {
	if ((_width < 20) || (_height < 20)) return;

	byte *pSrc = int_dialog_frame->data();
	byte *pDest = _data->data();
	uint16 xCenter = _width - DIALOG_EDGE_SIZE * 2;
	uint16 yCenter = _height - DIALOG_EDGE_SIZE * 2;

	// Dialog top
	for (int y = 0; y < 9; ++y) {
		copyLine(pSrc, pDest, DIALOG_EDGE_SIZE - 2, xCenter + 2, DIALOG_EDGE_SIZE);
		pSrc += (DIALOG_EDGE_SIZE - 2) + 1 + DIALOG_EDGE_SIZE;
		pDest += _width;
	}

	// Dialog sides - note that the same source data gets used for all side lines
	for (int y = 0; y < yCenter; ++y) {
		copyLine(pSrc, pDest, DIALOG_EDGE_SIZE, xCenter, DIALOG_EDGE_SIZE);
		pDest += _width;
	}
	pSrc += DIALOG_EDGE_SIZE * 2 + 1;

	// Dialog bottom
	for (int y = 0; y < 9; ++y) {
		copyLine(pSrc, pDest, DIALOG_EDGE_SIZE, xCenter + 1, DIALOG_EDGE_SIZE - 1);
		pSrc += DIALOG_EDGE_SIZE + 1 + (DIALOG_EDGE_SIZE - 1);
		pDest += _width;
	}

	// Final processing - if black flag set, clear dialog inside area
	if (blackFlag) {
		Rect r = Rect(DIALOG_EDGE_SIZE, DIALOG_EDGE_SIZE, 
			_width - DIALOG_EDGE_SIZE, _height-DIALOG_EDGE_SIZE);
		fillRect(r, 0);
	}
}

void Surface::copyToScreen(uint16 x, uint16 y) {
	OSystem &system = System::getReference();
	system.copyRectToScreen(_data->data(), _width, x, y, _width, _height);
	system.updateScreen();
}

void Surface::centerOnScreen() {
	OSystem &system = System::getReference();

	system.copyRectToScreen(_data->data(), _width, 
		(FULL_SCREEN_WIDTH - _width) / 2, (FULL_SCREEN_HEIGHT - _height) / 2,
		_width, _height);
	system.updateScreen();
}

uint16 Surface::textWidth(const char *s, int numChars) {
	uint16 result = 0;
	if (numChars == 0) numChars = strlen(s);

	while (numChars-- > 0) result += fontSize[*s++ - ' '] + 2;
	return result;
}

Surface *Surface::newDialog(uint16 width, uint8 numLines, char **lines, bool varLength, uint8 colour) {
	Surface *s = new Surface(width, (DIALOG_EDGE_SIZE + 3) * 2 + 
		numLines * (FONT_HEIGHT - 1));
	s->createDialog();

	for (uint8 ctr = 0; ctr < numLines; ++ctr)
		s->writeString(DIALOG_EDGE_SIZE + 3, DIALOG_EDGE_SIZE + 3 + 
			(ctr * (FONT_HEIGHT - 1)), lines[ctr], true, colour, varLength);
	return s;
}

Surface *Surface::newDialog(uint16 width, const char *line, uint8 colour) {
	uint8 numLines = 1;
	uint16 lineWidth = 0;
	char *s, *lineCopy;
	bool newLine;

	s = lineCopy = strdup(line);

	// Scan through the text and insert NULLs to break the line into allowable widths

	while (*s != '\0') {
		char *wordStart = s;
		while (*wordStart == ' ') ++wordStart;
		char *wordEnd = strchr(wordStart, ' ');
		char *wordEnd2 = strchr(wordStart, '\n');
		if ((!wordEnd) || ((wordEnd2) && (wordEnd2 < wordEnd))) {
			wordEnd = wordEnd2;
			newLine = (wordEnd2 != NULL);
		} else {
			newLine = false;
		}

		if (wordEnd) --wordEnd; // move back one to end of word
		else wordEnd = strchr(s, '\0') - 1;

		uint16 wordSize = textWidth(s, (int) (wordEnd - s + 1));

		if (lineWidth + wordSize > width - (DIALOG_EDGE_SIZE + 3) * 2) {
			// Break word onto next line
			*(wordStart - 1) = '\0';
			++numLines;
			lineWidth = textWidth(wordStart, (int) (wordEnd - wordStart + 1));
		} else if (newLine) {
			// Break on newline
			++numLines;
			++wordEnd;
			*wordEnd = '\0';
			lineWidth = 0;
		} else {
			// Add word's length to total for line
			lineWidth += wordSize;
		}

		s = wordEnd+1;
	}

	// Set up a list for the start of each line 
	char **lines = (char **) Memory::alloc(sizeof(char *) * numLines);
	lines[0] = lineCopy;
	for (int ctr = 1; ctr < numLines; ++ctr) 
		lines[ctr] = strchr(lines[ctr-1], 0) + 1;

	// Create the dialog 
	Surface *result = newDialog(width, numLines, lines, true, colour);

	// Deallocate used resources
	free(lines);
	free(lineCopy);

	return result;
}

Surface *Surface::getScreen(uint16 resourceId) {
	MemoryBlock *block = Disk::getReference().getEntry(resourceId);
	PictureDecoder d;
	MemoryBlock *decodedData = d.decode(block);
	delete block;
	return new Surface(decodedData, FULL_SCREEN_WIDTH, decodedData->size() / FULL_SCREEN_WIDTH);
}

/*--------------------------------------------------------------------------*/

void Dialog::show(const char *text) {
	Screen &screen = Screen::getReference();
	Mouse &mouse = Mouse::getReference();
	mouse.cursorOff();

	Surface *s = Surface::newDialog(INFO_DIALOG_WIDTH, text);
	s->copyToScreen(INFO_DIALOG_X, INFO_DIALOG_Y);

	// Wait for a keypress or mouse button
	Events::getReference().waitForPress();

	screen.update();
	mouse.cursorOn();
}

void Dialog::show(uint16 stringId) {
	char buffer[MAX_DESC_SIZE];
	Room &r = Room::getReference();
	StringData &sl = StringData::getReference();

	Action action = r.getCurrentAction();

	const char *actionName = (action == NONE) ? NULL : actionList[action];
	char hotspotName[MAX_HOTSPOT_NAME_SIZE];
	if (r.hotspotId() == 0) 
		strcpy(hotspotName, "");
	else
		sl.getString(r.hotspot().nameId, hotspotName, NULL, NULL);

	sl.getString(stringId, buffer, hotspotName, actionName);
	show(buffer);
}

void Dialog::showMessage(uint16 messageId, uint16 characterId) {
	MemoryBlock *data = Resources::getReference().messagesData();
    uint16 *v = (uint16 *) data->data();
	uint16 v2, idVal;
	messageId &= 0x7fff;

	// Skip through header to find table for given character
	while (READ_LE_UINT16(v) != characterId) v += 2;

	// Scan through secondary list
	++v;
	v = (uint16 *) (data->data() + READ_LE_UINT16(v));
	v2 = 0;
	while ((idVal = READ_LE_UINT16(v)) != 0xffff) {
		++v;
		if (READ_LE_UINT16(v) == messageId) break;
		++v;
	}
	// default response if a specific response not found
	if (idVal == 0xffff) idVal = 0x8c4; 
	
	if (idVal == 0x76) {
		/* 
		call    sub_154                 ; (64E7)
		mov     ax,word ptr ds:[5813h]  ; (273F:5813=1BA3h)
		mov     [bx+ANIM_SEGMENT],ax
		mov     ax,word ptr ds:[5817h]  ; (273F:5817=0ED8Eh)
		mov     [bx+ANIM_FRAME],ax
		retn
*/
	} else if (idVal == 0x120) {
		/* 
		call    sub_154                 ; (64E7)
		mov     ax,word ptr ds:[5813h]  ; (273F:5813=1BA3h)
		mov     [bx+ANIM_SEGMENT],ax
		mov     ax,word ptr ds:[5817h]  ; (273F:5817=0ED8Eh)
		shl     ax,1
		mov     [bx+ANIM_FRAME],ax
*/
	} else if (idVal >= 0x8000) {
		// Handle string display
		idVal &= 0x7fff;
		Dialog::show(idVal);
		
	} else if (idVal != 0) {
		/* still to be decoded */

	}
}

} // end of namespace Lure
