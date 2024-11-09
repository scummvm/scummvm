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
	uint origChunk = chunk;

	//debug("getText %d, %d, %d, %d", chunk, entry, type, subEntry);

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
	uint8 altSubString = !isInvDesc ?
		getSubtextNum(entry, subEntry, type) :
		getSubtextNum(origChunk, 0, type);

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

			if (*ptr == 0 && *(ptr + 1) != kEndText)
				*ptr = '|';
		} while (*ptr);

		// Pick the appropriate substring
		// Each substring starts with 0xf1 0xfe and ends with 0x0 0x0d
		if (*(ptr + 1) == kEndText && *(ptr + 2) == 0xf1 && *(ptr + 3) == 0xfe) {
			ptr += 6;
			uint curSubString = 0;
			bool endOfSubString = (*ptr == 0 && *(ptr + 1) == kEndText);
			bool endOfChunk = (endOfSubString && *(ptr + 2) == kEndChunk);

			if (txtNum == entry && curSubEntry == subEntry && curSubString < altSubString) {
				d->_text = "";
				d->_speechId++;
				curSubString++;

				while (!endOfChunk && curSubString <= altSubString) {
					d->_text += *ptr++;

					endOfSubString = (*ptr == 0 && *(ptr + 1) == kEndText);
					endOfChunk = (endOfSubString && *(ptr + 2) == kEndChunk);

					if (*ptr == 0 && *(ptr + 1) != kEndText)
						*ptr = '|';

					if (endOfSubString) {
						if (curSubString < altSubString) {
							d->_text = "";
							d->_speechId++;
							curSubString++;
							ptr += 6;
						} else {
							break;
						}
					}
				}
			}

			endOfSubString = (*ptr == 0 && *(ptr + 1) == kEndText);
			endOfChunk = (endOfSubString && *(ptr + 2) == kEndChunk);

			// Keep going until the chunk ends
			while (!endOfChunk) {
				ptr++;
				endOfSubString = (*ptr == 0 && *(ptr + 1) == kEndText);
				endOfChunk = (endOfSubString && *(ptr + 2) == kEndChunk);
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

		d->_text.trim();

		if (txtNum == entry && curSubEntry == subEntry && d->_text.size() > 1) {
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

uint8 *Text::getBuffer(uint8 type) {
	switch (type) {
	case ATS_DATA:
		return _hotspotStrings;
	case INV_USE_DATA:
		return _inventoryUseStrings;
	case INV_ATS_DATA:
		return _inventoryStrings;
	default:
		error("getBuffer called for type %d", type);
	}
}

uint8 getNibble(uint8 curValue, int8 subEntry) {
	if ((subEntry + 1) % 2 == 0)
		return curValue & 0x0F;
	else
		return (curValue >> 4) & 0x0F;
}

uint8 setNibble(uint8 curValue, int8 subEntry, uint8 newValue) {
	if ((subEntry + 1) % 2 == 0)
		return (curValue & 0xF0) | (newValue & 0x0F);
	else
		return (newValue << 4) | (curValue & 0x0F);
}

uint8 Text::getSubtextNum(uint16 entry, int8 txtMode, uint8 bufferType) {
	if (bufferType != ATS_DATA && bufferType != INV_USE_DATA && bufferType != INV_ATS_DATA)
		return 0;

	const int8 mode = txtMode == -1 ? 0 : txtMode;
	const uint8 *buffer = getBuffer(bufferType);
	const uint8 curValue = buffer[(entry * MAX_ATS_STATUS) + (mode + 1) / 2];
	return getNibble(curValue, mode);
}

void Text::setSubtextNum(uint16 entry, int8 txtMode, uint8 strNr, uint8 bufferType) {
	if (bufferType != ATS_DATA && bufferType != INV_USE_DATA && bufferType != INV_ATS_DATA)
		return;

	const int8 mode = txtMode == -1 ? 0 : txtMode;
	uint8 *buffer = getBuffer(bufferType);
	const uint8 curValue = buffer[(entry * MAX_ATS_STATUS) + (mode + 1) / 2];
	buffer[(entry * MAX_ATS_STATUS) + (mode + 1) / 2] = setNibble(curValue, mode, (uint8)strNr);
}

} // namespace Chewy
