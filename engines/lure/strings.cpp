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

#include "lure/strings.h"
#include "lure/disk.h"
#include "lure/lure.h"
#include "lure/res.h"
#include "lure/room.h"
#include "common/endian.h"

namespace Lure {

StringData *int_strings = NULL;

StringData::StringData() {
	int_strings = this;
	Disk &disk = Disk::getReference();

	for (uint8 ctr = 0; ctr < MAX_NUM_CHARS; ++ctr) _chars[ctr] = NULL;
	_numChars = 0;
	_names = Disk::getReference().getEntry(NAMES_RESOURCE_ID);
	_strings[0] = disk.getEntry(STRINGS_RESOURCE_ID);
	_strings[1] = disk.getEntry(STRINGS_2_RESOURCE_ID);
	_strings[2] = disk.getEntry(STRINGS_3_RESOURCE_ID);

	// Add in the list of bit sequences, and what characters they represent
	MemoryBlock *decoderList = disk.getEntry(STRING_DECODER_RESOURCE_ID);

	const char *p = (const char *) decoderList->data();
	while ((byte)*p != 0xff) {
		char ascii = *p++;
		add(p, ascii);
		p += strlen(p) + 1;
	}

	delete decoderList;
}

StringData::~StringData() {
	int_strings = NULL;

	for (uint8 ctr = 0; ctr < MAX_NUM_CHARS; ++ctr)
		if (_chars[ctr]) delete _chars[ctr];
		else break;

	delete _names;
	delete _strings[0];
	delete _strings[1];
	delete _strings[2];
}

StringData &StringData::getReference() {
	return *int_strings;
}

void StringData::add(const char *sequence, char ascii) {
	uint32 value = 0;

	for (uint8 index = 0; index < strlen(sequence); ++index) {
		if (sequence[index] == '1') 
			value |= (1 << index);
		else if (sequence[index] != '0') 
			error("Invalid character in string bit-stream sequence");
	}

	if (_numChars == MAX_NUM_CHARS) 
		error("Max characters too lower in string decoder");
	_chars[_numChars++] = new CharacterEntry(strlen(sequence), value, ascii);
}

byte StringData::readBit() {
	byte result = ((*_srcPos & _bitMask) != 0) ? 1 : 0;
	_bitMask >>= 1;
	if (_bitMask == 0) {
		_bitMask = 0x80;
		++_srcPos;
	}

	return result;
}

bool StringData::initPosition(uint16 stringId) {
	uint16 roomNumber = Room::getReference().roomNumber();
	byte *stringTable;
	
	if ((roomNumber >= 0x2A) && (stringId >= STRING_ID_RANGE) && (stringId < STRING_ID_UPPER)) 
		stringId = 0x76;
	if ((roomNumber < 0x2A) && (stringId >= STRING_ID_UPPER)) 
		stringId = 0x76;

	if (stringId < STRING_ID_RANGE)
		stringTable = _strings[0]->data();
	else if (stringId < STRING_ID_RANGE*2) {
		stringId -= STRING_ID_RANGE;
		stringTable = _strings[1]->data();
	} else {
		stringId -= STRING_ID_RANGE * 2;
		stringTable = _strings[2]->data();
	}

	_srcPos = stringTable + 4;
	
	uint32 total = 0;
	int numLoops = stringId >> 5;
	for (int ctr = 0; ctr < numLoops; ++ctr) {
		total += READ_LE_UINT16(_srcPos);
		_srcPos += sizeof(uint16);
	}

	numLoops = stringId & 0x1f;
	if (numLoops!= 0) {
		byte *tempPtr = stringTable + (stringId & 0xffe0) + READ_LE_UINT16(stringTable);
		
		for (int ctr = 0; ctr < numLoops; ++ctr) {
			byte v = *tempPtr++;
			if ((v & 0x80) == 0) {
				total += v;
			} else {
				total += (v & 0x7f) << 3;
			}
		}
	}

	_bitMask = 0x80;

	if ((total & 3) != 0)
		_bitMask >>= (total & 3) * 2;

	_srcPos = stringTable + (total >> 2) + READ_LE_UINT16(stringTable + 2);

	// Final positioning to start of string
	for (;;) {
		if (readBit() == 0) break;
		_srcPos += 2;
	}
	return readBit() != 0;
}

// readCharatcer
// Reads the next character from the input bit stream

char StringData::readCharacter() {
	uint32 searchValue = 0;

	// Loop through an increasing number of bits

	for (uint8 numBits = 1; numBits <= 18; ++numBits) {
		searchValue |= readBit() << (numBits - 1);

		// Scan through list for a match
		for (int index = 0; _chars[index] != NULL; ++index) {
			if ((_chars[index]->_numBits == numBits) && 
				(_chars[index]->_sequence == searchValue))
				return _chars[index]->_ascii;
		}
	}

	error("Unknown bit sequence encountered when decoding string");

	return 0;
}

void StringData::getString(uint16 stringId, char *dest, const char *hotspotName, 
		const char *characterName, int hotspotArticle, int characterArticle) {
	StringList &stringList = Resources::getReference().stringList();
	char ch;
	strcpy(dest, "");
	char *destPos = dest;
	stringId &= 0x1fff;      // Strip off any article identifier
	if (stringId == 0) return;

	bool includeArticles = initPosition(stringId);

	ch = readCharacter();
	while (ch != '\0') {
		if (ch == '%') {
			// Copy over hotspot or action 
			ch = readCharacter();
			const char *p = (ch == '1') ? hotspotName : characterName;
			int article = !includeArticles ? 3 : ((ch == 1) ? hotspotArticle : characterArticle);

			if (p != NULL) {
				if (article > 0)
					strcpy(destPos, stringList.getString(S_ARTICLE_LIST + article - 1));
				strcat(destPos, p);
				destPos += strlen(destPos);
			}
		} else if ((uint8) ch >= 0xa0) {
			const char *p = getName((uint8) ch - 0xa0);
			strcpy(destPos, p);
			destPos += strlen(p);
		} else {
			*destPos++ = ch;
		}

		ch = readCharacter();
	}

	*destPos = '\0';
}

// getName
// Returns the name or fragment of word at the specified index in the names resource

char *StringData::getName(uint8 nameIndex) {
	uint16 numNames = *((uint16 *) _names->data()) / 2;
	if (nameIndex >= numNames) 
		error("Invalid name index was passed to getCharacterName");

	uint16 nameStart = *((uint16 *) (_names->data() + (nameIndex * 2)));
	return (char *) (_names->data() + nameStart);
}

} // namespace Lure
