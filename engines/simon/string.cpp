/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"

#include "simon/simon.h"
#include "simon/intern.h"

namespace Simon {

static const byte charWidth[226] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 6, 2, 4, 8, 6,10, 9, 2,
	4, 4, 6, 6, 3, 4, 2, 3, 6, 4,
	6, 6, 7, 6, 6, 6, 6, 6, 2, 3,
	7, 7, 7, 6,11, 8, 7, 8, 8, 7,
	6, 9, 8, 2, 6, 7, 6,10, 8, 9,
	7, 9, 7, 7, 8, 8, 8,12, 8, 8,
	7, 3, 3, 3, 6, 8, 3, 7, 7, 6,
	7, 7, 4, 7, 6, 2, 3, 6, 2,10,
	6, 7, 7, 7, 5, 6, 4, 7, 7,10,
	6, 6, 6, 0, 0, 0, 0, 0, 8, 6,
	7, 7, 7, 7, 7, 6, 7, 7, 7, 4,
	4, 3, 8, 8, 7, 0, 0, 7, 7, 7,
	6, 6, 6, 9, 8, 0, 0, 0, 0, 0,
	7, 3, 7, 6, 6, 8, 0, 6, 0, 0,
	0, 0, 0, 2, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 	0, 0, 0, 0, 0, 7
};

const char *getPixelLength(const char *string, uint16 maxWidth, uint16 &pixels) {
	pixels = 0;

	while (*string != 0) {
		byte chr = *string;
		if ((pixels + charWidth[chr]) > maxWidth)
			break;	
		pixels += charWidth[chr];
		string++;
	}

	return string;
}

bool SimonEngine::printTextOf(uint a, uint x, uint y) {
	const byte *stringPtr;
	uint16 pixels, w;

	if (getGameType() == GType_SIMON2) {
		if (getBitFlag(79)) {
			Subroutine *sub;
			_variableArray[84] = a;
			sub = getSubroutineByID(5003);
			if (sub != NULL)
				startSubroutineEx(sub);
			return true;
		}
	}

	if (a >= _numTextBoxes)
		return false;


	stringPtr = getStringPtrByID(_stringIdArray2[a]);
	if (getGameType() == GType_FF) {
		getPixelLength((const char *)stringPtr, 400, pixels);
		w = pixels + 1;
		x -= w / 2;
		printScreenText(6, 0, (const char *)stringPtr, x, y, w);
	} else {
		showActionString(stringPtr);
	}

	return true;
}

bool SimonEngine::printNameOf(Item *item, uint x, uint y) {
	SubObject *subObject;
	const byte *stringPtr;
	uint16 pixels, w;

	if (item == 0 || item == _dummyItem2 || item == _dummyItem3)
		return false;

	subObject = (SubObject *)findChildOfType(item, 2);
	if (subObject == NULL)
		return false;

	stringPtr = getStringPtrByID(subObject->objectName);
	if (getGameType() == GType_FF) {
		getPixelLength((const char *)stringPtr, 400, pixels);
		w = pixels + 1;
		x -= w / 2;
		printScreenText(6, 0, (const char *)stringPtr, x, y, w);
	} else {
		showActionString(stringPtr);
	}

	return true;
}

void SimonEngine::printInteractText(uint16 num, const char *string) {
	char convertedString[320];
	char *convertedString2 = convertedString;
	const char *string2 = string;
	uint16 height = 15;
	uint16 w = 620;
	uint16 b, pixels, x;

	while (1) {
		string2 = getPixelLength(string, 620, pixels);
		if (*string2 == 0x00) {
			if (w == 620)
				w = pixels;
			strcpy(convertedString2, string);
			break;
		}
		while (*string2 != ' ') {
			byte chr = *string2;
			pixels -= charWidth[chr];
			string2--;
		}
		if (w == 620)
			w = pixels;
		b = string2 - string;
		strncpy(convertedString2, string, b);
		convertedString2 += b;
		*convertedString2++ = '\n';
		height += 15;
		string = string2;
	}

	// ScrollX
	x = _variableArray[251];		
	x += 20;

	if (num == 1)
		_interactY = 385;

	// Returned values for box definition
	_variableArray[51] = x;			
	_variableArray[52] = _interactY;
	_variableArray[53] = w;
	_variableArray[54] = height;

	kill_sprite_simon2(2, num + 6);
	renderString(num, 0, w, height, convertedString);
	loadSprite(4, 2, num + 6, x, _interactY, 12);

	_interactY += height;
}

void SimonEngine::printScreenText(uint vgaSpriteId, uint color, const char *string, int16 x, int16 y, int16 width) {
	char convertedString[320];
	char *convertedString2 = convertedString;
	const char *string2 = string;
	int16 height, talkDelay;
	int stringLength = strlen(string);
	int padding, lettersPerRow, lettersPerRowJustified;
	const int textHeight = (getGameType() == GType_FF) ? 15: 10;

	height = textHeight;
	lettersPerRow = width / 6;
	lettersPerRowJustified = stringLength / (stringLength / lettersPerRow + 1) + 1;

	talkDelay = (stringLength + 3) / 3;
	if ((getGameType() == GType_SIMON1) && (getFeatures() & GF_TALKIE)) {
		if (_variableArray[141] == 0)
			_variableArray[141] = 9;
		_variableArray[85] = _variableArray[141] * talkDelay;
	} else {
		if (_variableArray[86] == 0)
			talkDelay /= 2;
		if (_variableArray[86] == 2)
			talkDelay *= 2;
		_variableArray[85] = talkDelay * 5;
	}

	assert(stringLength > 0);

	if (getGameType() == GType_FF) {
		uint16 b, pixels, spaces;
		uint16 curWdth = width;

		while (1) {
			string2 = getPixelLength(string, curWdth, pixels);
			if (*string2 == 0) {
				spaces = (width - pixels) / 12;
				if (spaces != 0)
					spaces--;
				while (spaces) {
	    				*convertedString2++=' ';
	    				spaces--;
				}
				strcpy(convertedString2, string);
				break;
			}
			while (*string2 != ' ') {
				byte chr = *string2;
				pixels -= charWidth[chr];
				string2--;
			}
			spaces = (width - pixels) / 12;
			if (spaces != 0)
				spaces--;
			while (spaces) {
	    			*convertedString2++ = ' ';
		    		spaces--;
			}
			b = string2 - string;
			strncpy(convertedString2, string, b);
			convertedString2 += b;
			*convertedString2++ = '\n';
			height += textHeight;
			y -= textHeight;
			if (y < 2)
			    y = 2;
			curWdth = pixels;
			string = string2;
		}
	} else {
		while (stringLength > 0) {
			int pos = 0;
			if (stringLength > lettersPerRow) {
				int removeLastWord = 0;
				if (lettersPerRow > lettersPerRowJustified) {
					pos = lettersPerRowJustified;
					while (string[pos] != ' ')
						pos++;
					if (pos > lettersPerRow)
						removeLastWord = 1;
				}
				if (lettersPerRow <= lettersPerRowJustified || removeLastWord) {
					pos = lettersPerRow;
					while (string[pos] != ' ' && pos > 0)
						pos--;
				}
				height += textHeight;
				y -= textHeight;
			} else
				pos = stringLength;
			padding = (lettersPerRow - pos) % 2 ?
				(lettersPerRow - pos) / 2 + 1 : (lettersPerRow - pos) / 2;
			while (padding--)
				*convertedString2++ = ' ';
			stringLength -= pos;
			while (pos--)
				*convertedString2++ = *string++;
			*convertedString2++ = '\n';
			string++; // skip space
			stringLength--; // skip space
		}
		*(convertedString2 - 1) = '\0';
	}

	if (getGameType() == GType_SIMON1)
		kill_sprite_simon1(vgaSpriteId + 199);
	else
		kill_sprite_simon2(2, vgaSpriteId);

	if (getGameType() == GType_FF) {
		renderString(1, color, width, height, convertedString);
	} else {
		color = color * 3 + 192;
		if (getPlatform() == Common::kPlatformAmiga)
			renderStringAmiga(vgaSpriteId, color, width, height, convertedString);
		else
			renderString(vgaSpriteId, color, width, height, convertedString);
	}

	int b = 4;
	if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
		if (!getBitFlag(133))
			b = 3;

		x /= 8;
		if (y < 2)
			y = 2;
	}

	if (getGameType() == GType_SIMON1)
		loadSprite(b, 2, vgaSpriteId + 199, x, y, 12);
	else
		loadSprite(b, 2, vgaSpriteId, x, y, 12);
}

} // End of namespace Simon
