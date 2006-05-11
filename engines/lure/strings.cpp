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

#include "lure/strings.h"
#include "lure/disk.h"
#include "lure/room.h"
#include "common/endian.h"

namespace Lure {

StringData *int_strings = NULL;

StringData::StringData() {
	int_strings = this;

	for (uint8 ctr = 0; ctr < MAX_NUM_CHARS; ++ctr) _chars[ctr] = NULL;
	_numChars = 0;
	_names = Disk::getReference().getEntry(NAMES_RESOURCE_ID);
	_strings[0] = Disk::getReference().getEntry(STRINGS_RESOURCE_ID);
	_strings[1] = Disk::getReference().getEntry(STRINGS_2_RESOURCE_ID);
	_strings[2] = Disk::getReference().getEntry(STRINGS_3_RESOURCE_ID);

	// Add in the list of bit sequences, and what characters they represent
	add("00", ' ');
	add("0100", 'e');
	add("0101", 'o');
	add("0110", 't');
	add("01110", 'a');
	add("01111", 'n');
	add("1000", 's');
	add("1001", 'i');
	add("1010", 'r');
	add("10110", 'h');
	add("101110", 'u');
	add("1011110", 'l');
	add("1011111", 'd');
	add("11000", 'y');
	add("110010", 'g');
	add("110011", '\0');
	add("110100", 'w');
	add("110101", 'c');
	add("110110", 'f');
	add("1101110", '.');
	add("1101111", 'm');
	add("111000", 'p');
	add("111001", 'b');
	add("1110100", ',');
	add("1110101", 'k');
	add("1110110", '\'');
	add("11101110", 'I');
	add("11101111", 'v');
	add("1111000", '!');
	add("1111001", '\xb4');
	add("11110100", 'T');
	add("11110101", '\xb5');
	add("11110110", '?');
	add("111101110", '\xb2');
	add("111101111", '\xb3');
	add("11111000", 'W');
	add("111110010", 'H');
	add("111110011", 'A');
	add("111110100", '\xb1');
	add("111110101", 'S');
	add("111110110", 'Y');
	add("1111101110", 'G');
	add("11111011110", 'M');
	add("11111011111", 'N');
	add("111111000", 'O');
	add("1111110010", 'E');
	add("1111110011", 'L');
	add("1111110100", '-');
	add("1111110101", 'R');
	add("1111110110", 'B');
	add("11111101110", 'D');
	add("11111101111", '\xa6');
	add("1111111000", 'C');
	add("11111110010", 'x');
	add("11111110011", 'j');
	add("1111111010", '\xac');
	add("11111110110", '\xa3');
	add("111111101110", 'P');
	add("111111101111", 'U');
	add("11111111000", 'q');
	add("11111111001", '\xad');
	add("111111110100", 'F');
	add("111111110101", '1');
	add("111111110110", '\xaf');
	add("1111111101110", ';');
	add("1111111101111", 'z');
	add("111111111000", '\xa5');
	add("1111111110010", '2');
	add("1111111110011", '\xb0');
	add("111111111010", 'K');
	add("1111111110110", '%');
	add("11111111101110", '\xa2');
	add("11111111101111", '5');
	add("1111111111000", ':');
	add("1111111111001", 'J');
	add("1111111111010", 'V');
	add("11111111110110", '6');
	add("11111111110111", '3');
	add("1111111111100", '\xab');
	add("11111111111010", '\xae');
	add("111111111110110", '0');
	add("111111111110111", '4');
	add("11111111111100", '7');
	add("111111111111010", '9');
	add("111111111111011", '"');
	add("111111111111100", '8');
	add("111111111111101", '\xa7');
	add("1111111111111100", '/');
	add("1111111111111101", 'Q');
	add("11111111111111100", '\xa8');
	add("11111111111111101", '(');
	add("111111111111111100", ')');
	add("111111111111111101", '\x99');
	add("11111111111111111", '\xa9');
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

void StringData::initPosition(uint16 stringId) {
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
	readBit();
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
}

void StringData::getString(uint16 stringId, char *dest, const char *hotspotName, 
							  const char *actionName) {
	char ch;
	char *destPos = dest;
	initPosition(stringId);

	ch = readCharacter();
	while (ch != '\0') {
		if (ch == '%') {
			// Copy over hotspot or action 
			ch = readCharacter();
			const char *p = (ch == '1') ? hotspotName : actionName;
			strcpy(destPos, p);
			destPos += strlen(p);
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
