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

#include "common/system.h"
#include "chewy/resource.h"
#include "chewy/text.h"

#include "types.h"
#include "chewy/atds.h"
#include "chewy/defines.h"

namespace Chewy {

Text::Text() : Resource("atds.tap") {
	memset(_hotspotStrings, 0, sizeof(_hotspotStrings));

	Common::File f;

	if (!f.open(ROOM_ATS_STEUER))
		error("Error reading file: %s", ROOM_ATS_STEUER);
	for (int16 i = 0; i < ROOM_ATS_MAX; i++)
		_hotspotStrings[i * MAX_ATS_STATUS] = f.readByte();

	f.close();

	if (!f.open(INV_ATS_STEUER))
		error("Error reading file: %s", INV_ATS_STEUER);
	for (int16 i = 0; i < MAX_MOV_OBJ; i++)
		_inventoryStrings[i * MAX_ATS_STATUS] = f.readByte();

	f.close();

	// WORKAROUND: For English version, taxi hotspot in
	// room 45 (Big City) isn't turned on by default
	_hotspotStrings[295] = ATS_ACTION_BIT;
}

Text::~Text() {
}

TextEntryList *Text::getDialog(uint chunk, uint entry) {
	if (chunk >= kADSTextMax)
		error("getDialog(): Invalid entry number requested, %d (max %d)", chunk, kADSTextMax - 1);

	TextEntryList *l = new TextEntryList();

	byte *data = getChunkData(chunk);
	byte *ptr = data;

	ptr += 2;  // entry number
	ptr += 2;  // number of persons
	ptr += 2;  // automove count
	ptr += 2;  // cursor number
	ptr += 13; // misc data

	for (uint i = 0; i <= entry; i++) {
		do {
			TextEntry curDialog;
			ptr++; // current entry
			ptr += 2;
			curDialog._speechId = READ_LE_UINT16(ptr) - VOICE_OFFSET;
			ptr += 2;

			do {
				curDialog._text += *ptr++;

				if (*ptr == 0 && *(ptr + 1) != kEndText) {
					// TODO: Split lines
					*ptr = ' ';
				}
			} while (*ptr != kEndText);

			if (i == entry)
				l->push_back(curDialog);

		} while (*(ptr + 1) != kEndEntry);

		ptr += 2; // kEndText, kEndEntry

		if (*ptr == kEndBlock) // not found
			break;
	}

	delete[] data;

	return l;
}

TextEntry *Text::getText(uint chunk, uint entry, int type, int subEntry) {
	bool isText = false;
	bool isAutoDialog = false;
	bool isInvDesc = false;

	//int subEntryNew = -1;

	//if (subEntry >= 0)
	//	subEntryNew = getTextId(entry, subEntry, type);

	switch (type) {
	case AAD_DATA:
		chunk += kADSTextMax + kATSTextMax;
		isAutoDialog = true;
		break;
	case ATS_DATA:
		chunk += kADSTextMax;
		isText = true;
		break;
	case DIALOG_CLOSEUP_DATA:
		// No change - chunk starts from 0
		break;
	case INV_USE_DATA:
	case INV_USE_DEF:
		chunk += kADSTextMax + kATSTextMax + kAADTextMax + kINVTextMax;
		isInvDesc = true;
		isText = true;
		break;
	case INV_ATS_DATA:
		chunk += kADSTextMax + kATSTextMax + kAADTextMax;
		isInvDesc = true;
		break;
	}

	if (chunk < kADSTextMax)
		error("getText(): Invalid chunk number requested, %d (min %d)", chunk, kADSTextMax);

	TextEntry *d = new TextEntry();

	byte *data = getChunkData(chunk);
	byte *ptr = data;
	uint entryId = 0;
	uint16 headerBytes, txtNum;
	int curSubEntry = -1;

	//Common::hexdump(data, _chunkList[chunk].size);

	if (isAutoDialog)
		ptr += 3;

	while (true) {
		ptr += 3;
		headerBytes = READ_LE_UINT16(ptr);
		ptr += 2;

		if (headerBytes == 0xFEF2) {
			// Start of subchunk
			curSubEntry = *ptr;
			ptr++;
			headerBytes = READ_LE_UINT16(ptr);
			ptr += 2;
		}

		if (headerBytes != 0xFEF0)
			break;

		txtNum = !isInvDesc ? READ_LE_UINT16(ptr) : entryId++;
		ptr += 2;
		ptr += 6;
		d->_speechId = READ_LE_UINT16(ptr) - VOICE_OFFSET;
		ptr += 2;

		do {
			if (txtNum == entry && curSubEntry == subEntry)
				d->_text += *ptr++;
			else
				ptr++;

			if (*ptr == 0 && *(ptr + 1) != kEndText) {
				*ptr = '|';
			}
		} while (*ptr);

		// FIXME: Skip other embedded strings for now
		if (*(ptr + 1) == kEndText && *(ptr + 2) == 0xf1 && *(ptr + 3) == 0xfe) {
			ptr += 5;
			while (!(!*ptr && *(ptr + 1) == kEndText && *(ptr + 2) == kEndChunk)) {
				ptr++;
			}
		}

		if (*(ptr + 1) != kEndText || *(ptr + 2) != kEndChunk) {
			warning("Invalid text resource - %d, %d", chunk, entry);

			delete[] data;
			delete d;

			return nullptr;
		}

		if (!isText)
			ptr += 3; // 0, kEndText, kEndChunk
		if (isAutoDialog)
			ptr += 3;

		if (txtNum == entry && curSubEntry == subEntry) {
			// Found
			delete[] data;
			return d;
		}
	}

	// Not found
	delete[] data;
	delete d;

	return nullptr;
}

Common::StringArray Text::getTextArray(uint chunk, uint entry, int type, int subEntry) {
	TextEntry *textData = getText(chunk, entry, type, subEntry);
	Common::StringArray res;
	Common::String txt = textData ? textData->_text : "";
	char *text = new char[txt.size() + 1];
	Common::strlcpy(text, txt.c_str(), txt.size() + 1);
	char *line = strtok(text, "|");

	while (line) {
		res.push_back(line);
		line = strtok(nullptr, "|");
	}

	_lastSpeechId = textData ? textData->_speechId : -1;

	delete[] text;
	delete textData;

	return res;
}

Common::String Text::getTextEntry(uint chunk, uint entry, int type, int subEntry) {
	Common::StringArray res = getTextArray(chunk, entry, type, subEntry);
	return res.size() > 0 ? res[0] : "";
}

const char *Text::strPos(const char *txtAdr, int16 pos) {
	const char *ptr = txtAdr;
	for (int16 i = 0; i < pos;) {
		if (*ptr == 0 || *ptr == '|')
			++i;
		++ptr;
	}

	return ptr;
}

void Text::syncHotspotStrings(Common::Serializer &s) {
	for (size_t i = 0; i < sizeof(_hotspotStrings); ++i)
		s.syncAsByte(_hotspotStrings[i]);
}

void Text::syncInventoryStrings(Common::Serializer &s) {
	for (size_t i = 0; i < sizeof(_inventoryStrings); ++i)
		s.syncAsByte(_inventoryStrings[i]);
}

void Text::syncInventoryUseStrings(Common::Serializer &s) {
	for (size_t i = 0; i < sizeof(_inventoryUseStrings); ++i)
		s.syncAsByte(_inventoryUseStrings[i]);
}

bool Text::getControlBit(int16 txtNr, int16 bitIdx) {
	return (_hotspotStrings[txtNr * MAX_ATS_STATUS] & bitIdx) != 0;
}

void Text::setControlBit(int16 txtNr, int16 bitIdx) {
	_hotspotStrings[txtNr * MAX_ATS_STATUS] |= bitIdx;
}

void Text::delControlBit(int16 txtNr, int16 bitIdx) {
	_hotspotStrings[txtNr * MAX_ATS_STATUS] &= ~bitIdx;
}

/*uint8 Text::getTextStatus(uint8 status, int16 subEntry, int16 strNr) {
	const int16 hotspotActionStr = (subEntry + 1) % 2;
	uint8 lo_hi[2];
	lo_hi[0] = status &= 15;
	lo_hi[1] = status >> 4;
	lo_hi[hotspotActionStr] = strNr;
	status = 0;
	lo_hi[1] <<= 4;
	status |= lo_hi[0];
	status |= lo_hi[1];

	return status;
}*/

uint8 Text::updateTextStatus(int16 entry, int16 subEntry, int16 strNr, int16 type) {
	byte *buffer;

	switch (type) {
	case ATS_DATA:
		buffer = _hotspotStrings;
		break;
	case INV_USE_DATA:
		buffer = _inventoryUseStrings;
		break;
	case INV_ATS_DATA:
		buffer = _inventoryStrings;
		break;
	default:
		error("setTextId called for type %d", type);
	}

	const uint8 status = buffer[(entry * MAX_ATS_STATUS) + (subEntry + 1) / 2];
	if (strNr >= 0) {
		// TODO: This is buggy
		//buffer[(entry * MAX_ATS_STATUS) + (subEntry + 1) / 2] = strNr; // getTextStatus(status, subEntry, strNr);
		return strNr;
	}

	return status;
}

uint8 Text::getTextId(uint entry, uint subEntry, int type) {
	uint8 status = updateTextStatus(entry, subEntry, -1, type);

	const int16 hotspotActionStr = (subEntry + 1) % 2;
	uint8 lo_hi[2];
	lo_hi[0] = status &= 15;
	lo_hi[1] = status >> 4;
	return lo_hi[hotspotActionStr];
}

void Text::setTextId(int16 entry, int16 subEntry, int16 strNr, int16 type) {
	updateTextStatus(entry, subEntry, strNr, type);
}

} // namespace Chewy
