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
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

#include "macventure/text.h"

namespace MacVenture {
TextAsset::TextAsset(ObjID objid, Container *container, bool isOld, const HuffmanLists *huffman) {
	_id = objid;
	_container = container;
	_huffman = huffman;

	if (isOld) {
		decodeOld();
	}
	else {
		decodeHuffman();
	}	
}

void TextAsset::decodeOld() {
	Common::SeekableReadStream *res = _container->getItem(_id);
	uint16 strLen = res->readUint16BE();
	Common::BitStream32BELSB stream(res);
	char* str = new char[strLen + 1];
	bool lowercase = false;
	char c;
	for (uint16 i = 0; i < strLen; i++) {
		char val = stream.getBits(5);
		if (val == 0x0) { // Space
			c = ' ';
		}
		else if (val >= 0x1 && val <= 0x1A) {
			if (lowercase) { // Ascii a-z
				c = val + 0x60;
			}
			else { // Ascii A-Z
				c = val + 0x40;
			}
			lowercase = true;
		}
		else if (val == 0x1B) {
			if (lowercase) {
				c = '.';
			}
			else {
				c = ',';
			}
			lowercase = true;
		}
		else if (val == 0x1C) {
			if (lowercase) {
				c = 0x27; // Ascii '
			}
			else {
				c = 0x22; // Ascii '"'
			}
			lowercase = true;
		}
		else if (val == 0x1D) { // Composite
			warning("Composite strings not implemented");
			stream.getBits(16);
			lowercase = true;
		}
		else if (val == 0x1E) {
			c = stream.getBits(8);
			lowercase = true;
		}
		else if (val == 0x1F) {
			lowercase = !lowercase;
		}
		else {
			warning("Unrecognized char in old text %d, pos %d", _id, i);
		}
		str[i] = c;
	}

	str[strLen] = '\0';
	debug(7, "Decoded %d string (old): %s", _id, str);
	_decoded = Common::String(str);
}

void TextAsset::decodeHuffman() {
	_decoded = Common::String("Huffman string");
	Common::SeekableReadStream *res = _container->getItem(_id);
	Common::BitStream32BEMSB stream(res);
	uint16 strLen = 0;
	if (stream.getBit()) {
		strLen = stream.getBits(15);
	}
	else {
		strLen = stream.getBits(7);
	}

	char* str = new char[strLen + 1];
	uint32 mask = 0;
	uint32 symbol = 0;
	char c;
	for (uint16 i = 0; i < strLen; i++) {
		mask = stream.peekBits(16); // The mask is OK, so it means that I don't know how to use the huffman
		uint32 entry;
		// Find the length index
		for (entry = 0; entry < _huffman->getNumEntries(); entry++) {
			if (mask < _huffman->getMask(entry)) break;
		}			

		stream.skip(_huffman->getLength(entry));

		symbol = _huffman->getSymbol(entry);

		if (symbol == 1) { // 7-bit ascii
			c = stream.getBits(7);
		}
		else if (symbol == 2) { // Composite
			warning("Composite huffman strings not implemented");
		}
		else { // Plain ascii
			c = symbol & 0xFF;
		}

		str[i] = c;
	}

	str[strLen] = '\0';
	debug(7, "Decoded %d'th string (new): %s", _id, str);
	_decoded = Common::String(str);
}
} // End of namespace MacVenture