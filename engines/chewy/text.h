/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef CHEWY_TEXT_H
#define CHEWY_TEXT_H

#include "common/list.h"
#include "chewy/chewy.h"
#include "chewy/resource.h"

namespace Chewy {

/**
 * Game texts are contained in txt/atds.tap, and contain the following (in that order):
 * ADS (Adventure Dialog System) - dialogs, 500 entries max
 * ATS (Adventure Text System) - text descriptions, 100 entries max
 * AAD (Adventure Auto Dialog System) - automatic dialogs, 100 entries max
 * INV - inventory text descriptions, 100 entries max
 * USE - use action texts, 100 entries max
 */
enum MaxTextTypes {
	kADSTextMax = 500,
	kATSTextMax = 100,
	kAADTextMax = 100,
	kINVTextMax = 100,
	kUSETextMax = 100
};

/**
 * Markers for text entries
 */
enum TextEntryMarkers {
	kEndRow = 0x00,
	kEndBlock = 0x0b,
	kEndEntry = 0x0c,
	kEndText = 0x0d,
	kEndChunk = 0x0e
	// There's also 0x0f, block end, which we don't use
};

#define VOICE_OFFSET 20

struct Dialog {
	uint16 speechId;
	Common::String text;
};

typedef Common::List<Dialog> DialogList;


class Text : public Resource {
public:
	Text();
	virtual ~Text();

	DialogList *getDialog(uint dialogNum, uint entryNum);
	// TODO: getText()
	// TODO: getAutoDialog()
	// TODO: getInvDesc()
	// TODO: getUseText()
};

class ErrorMessage : public Resource {
public:
	ErrorMessage(Common::String filename) : Resource(filename) {}
	virtual ~ErrorMessage() {}

	Common::String getErrorMessage(uint num);
};

class Font {
public:
	Font(Common::String filename);
	virtual ~Font();

	::Graphics::Surface *getLine(Common::String text);

private:
	uint16 _count, _first, _last, _width, _height;

	::Graphics::Surface _fontSurface;
};

} // End of namespace Chewy

#endif
