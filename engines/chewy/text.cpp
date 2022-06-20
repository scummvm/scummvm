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
#include "chewy/atds.h"
#include "chewy/defines.h"

namespace Chewy {

Text::Text() : Resource("atds.tap") {
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

} // namespace Chewy
