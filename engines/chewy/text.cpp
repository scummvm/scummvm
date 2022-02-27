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

namespace Chewy {

Text::Text() : Resource("atds.tap") {
}

Text::~Text() {
}

TextEntryList *Text::getDialog(uint dialogNum, uint entryNum) {
	if (dialogNum >= kADSTextMax)
		error("getDialog(): Invalid entry number requested, %d (max %d)", dialogNum, kADSTextMax - 1);

	TextEntryList *l = new TextEntryList();

	byte *data = getChunkData(dialogNum);
	byte *ptr = data;

	ptr += 2;  // entry number
	ptr += 2;  // number of persons
	ptr += 2;  // automove count
	ptr += 2;  // cursor number
	ptr += 13; // misc data

	for (uint i = 0; i <= entryNum; i++) {
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

			if (i == entryNum)
				l->push_back(curDialog);

		} while (*(ptr + 1) != kEndEntry);

		ptr += 2; // kEndText, kEndEntry

		if (*ptr == kEndBlock) // not found
			break;
	}

	delete[] data;

	return l;
}

TextEntry *Text::getText(uint dialogNum, uint entryNum) {
	if (dialogNum < kADSTextMax)
		error("getText(): Invalid entry number requested, %d (min %d)", dialogNum, kADSTextMax);

	TextEntry *d = new TextEntry();
	bool isText = (dialogNum >= kADSTextMax && dialogNum < kADSTextMax + kATSTextMax);
	bool isAutoDialog = (dialogNum >= kADSTextMax + kATSTextMax && dialogNum < kADSTextMax + kATSTextMax + kAADTextMax);
	//bool isInvText = (dialogNum >= kADSTextMax + kATSTextMax + kAADTextMax && dialogNum < kADSTextMax + kATSTextMax + kAADTextMax + kINVTextMax);

	byte *data = getChunkData(dialogNum);
	byte *ptr = data;

	if (isAutoDialog)
		ptr += 3;

	for (uint i = 0; i <= entryNum; i++) {
		ptr += 13;
		d->_speechId = READ_LE_UINT16(ptr) - VOICE_OFFSET;
		ptr += 2;

		do {
			if (i == entryNum)
				d->_text += *ptr++;
			else
				ptr++;

			if (*ptr == 0 && *(ptr + 1) != kEndText) {
				// TODO: Split lines
				*ptr = ' ';
			}
		} while (*ptr);

		if (*(ptr + 1) != kEndText || *(ptr + 2) != kEndChunk) {
			warning("Invalid text resource - %d, %d", dialogNum, entryNum);

			delete[] data;
			delete d;

			return nullptr;
		}

		if (!isText)
			ptr += 3; // 0, kEndText, kEndChunk
		if (isAutoDialog)
			ptr += 3;

		if (i == entryNum) {
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


void Text::crypt(char *txt, uint32 size) {
	uint8 *sp = (uint8 *)txt;
	for (uint32 i = 0; i < size; i++) {
		*sp = -(*sp);
		++sp;
	}
}

char *Text::strPos(char *txtAdr, int16 pos) {
	char *ptr = txtAdr;
	for (int16 i = 0; i < pos;) {
		if (*ptr == 0)
			++i;
		++ptr;
	}

	return ptr;
}

} // namespace Chewy
