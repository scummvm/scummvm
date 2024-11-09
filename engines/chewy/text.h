/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef CHEWY_TEXT_H
#define CHEWY_TEXT_H

#include "common/list.h"
#include "common/serializer.h"
#include "chewy/atds.h"
#include "chewy/chewy.h"
#include "chewy/defines.h"
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
	kADSTextMax = 500, //   0 - 499
	kATSTextMax = 100, // 500 - 599
	kAADTextMax = 100, // 600 - 699
	kINVTextMax = 140, // 700 - 849
	kUSETextMax =  60  // 850 - 899
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
	int16 _speechId;
	Common::String _text;
};

typedef Common::List<TextEntry> TextEntryList;

#define ROOM_ATS_MAX 1000

class Text : public Resource {
public:
	Text();
	~Text();

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
	*
	* A chunk can contain multiple subchunks with
	* the same entry IDs per subchunk:
	* 0 - name, 1 - look, 2 - use, 3 - walk, 4 - talk
	*/
	TextEntry *getText(uint chunk, uint entry, int type, int subEntry = -1);
	Common::StringArray getTextArray(uint chunk, uint entry, int type, int subEntry = -1);
	Common::String getTextEntry(uint chunk, uint entry, int type, int subEntry = -1);
	int16 getLastSpeechId() { return _lastSpeechId; }

	const char *strPos(const char *txtAdr, int16 pos);

	void syncHotspotStrings(Common::Serializer &s);
	void syncInventoryStrings(Common::Serializer &s);
	void syncInventoryUseStrings(Common::Serializer &s);

	bool getControlBit(int16 txtNr, int16 bitIdx);
	void setControlBit(int16 txtNr, int16 bitIdx);
	void delControlBit(int16 txtNr, int16 bitIdx);

	void setSubtextNum(uint16 entry, int8 txtMode, uint8 strNr, uint8 bufferType);

private:
	int16 _lastSpeechId = -1;
	uint8 _hotspotStrings[ROOM_ATS_MAX * 3] = { 0 };
	uint8 _inventoryStrings[MAX_MOV_OBJ * 3] = { 0 };
	uint8 _inventoryUseStrings[INV_USE_ATS_MAX * 3] = { 0 };

	uint8 *getBuffer(uint8 type);
	uint8 getSubtextNum(uint16 entry, int8 txtMode, uint8 bufferType);
};

} // namespace Chewy

#endif
