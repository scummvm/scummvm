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
	kADSTextMax = 500,	//   0 - 499
	kATSTextMax = 100,	// 500 - 599
	kAADTextMax = 100,	// 600 - 699
	kINVTextMax = 100,	// 700 - 799
	kUSETextMax = 100	// 800 - 899
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

struct TextEntry {
	int16 speechId;
	Common::String text;
};

typedef Common::List<TextEntry> TextEntryList;


class Text : public Resource {
public:
	Text();
	~Text() override;

	/**
	 * Gets a list of lines for a specific dialog entry
	 */
	TextEntryList *getDialog(uint dialogNum, uint entryNum);

	/**
	* Gets a line of text of the following types:
	* - text (ATS) - 500 - 599
	* - auto dialog (AAD) - 600 - 699
	* - inventory text (INV) - 700 - 799
	* - use text (USE) - 800 - 899
	*/
	TextEntry *getText(uint dialogNum, uint entryNum);
};

class ErrorMessage : public Resource {
public:
	ErrorMessage(Common::String filename) : Resource(filename) {}
	~ErrorMessage() override {}

	Common::String getErrorMessage(uint num);
};

class Font {
public:
	Font(Common::String filename);
	virtual ~Font();

	::Graphics::Surface *getLine(const Common::String &text);

private:
	uint16 _count, _first, _last, _width, _height;

	::Graphics::Surface _fontSurface;
};

} // End of namespace Chewy

#endif
