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

/*
 * Based on
 * WebVenture (c) 2010, Sean Kasun
 * https://github.com/mrkite/webventure, http://seancode.com/webventure/
 *
 * Used with explicit permission from the author
 */

#include "macventure/text.h"

namespace MacVenture {
TextAsset::TextAsset(MacVentureEngine *engine, ObjID objid, ObjID source, ObjID target, Container *container, bool isOld, const HuffmanLists *huffman) {
	_id = objid;
	_sourceObj = source;
	_targetObj = target;
	_container = container;
	_huffman = huffman;
	_isOld = isOld;
	_engine = engine;

	if (_isOld) {
		decodeOld();
	} else {
		decodeHuffman();
	}
}

void TextAsset::decodeOld() {
	Common::SeekableReadStream *res = _container->getItem(_id);
	uint16 strLen = res->readUint16BE();
	Common::BitStream32BELSB stream(res, DisposeAfterUse::YES);
	char *str = new char[strLen + 1];
	bool lowercase = false;
	char c = ' ';
	for (uint16 i = 0; i < strLen; i++) {
		char val = stream.getBits(5);
		if (val == 0x0) { // Space
			c = ' ';
		} else if (val >= 0x1 && val <= 0x1A) {
			if (lowercase) { // Ascii a-z
				c = val + 0x60;
			} else { // Ascii A-Z
				c = val + 0x40;
			}
			lowercase = true;
		} else if (val == 0x1B) {
			if (lowercase) {
				c = '.';
			} else {
				c = ',';
			}
			lowercase = true;
		} else if (val == 0x1C) {
			if (lowercase) {
				c = '\'';
			} else {
				c = '"';
			}
			lowercase = true;
		} else if (val == 0x1D) { // Composite
			ObjID subval = stream.getBits(16);
			Common::String child;
			if (subval & 0x8000) {
				// Composite object id
				subval ^= 0xFFFF;
				child = getNoun(subval);
			} else {
				// Just another id
				// HACK, see below in getNoun()
				child = *TextAsset(_engine, subval, _sourceObj, _targetObj, _container, _isOld, _huffman).decode();
			}
			if (child.size() > 0) {
				c = '?'; // HACK Will fix later, should append
			}
			lowercase = true;
		} else if (val == 0x1E) {
			c = stream.getBits(8);
			lowercase = true;
		} else if (val == 0x1F) {
			lowercase = !lowercase;
		} else {
			warning("Unrecognized char in old text %d, pos %d", _id, i);
		}
		str[i] = c;
	}

	str[strLen] = '\0';
	debugC(3, kMVDebugText, "Decoded string [%d] (old encoding): %s", _id, str);
	_decoded = Common::String(str);
}

void TextAsset::decodeHuffman() {
	_decoded = Common::String("");
	Common::SeekableReadStream *res = _container->getItem(_id);
	Common::BitStream8MSB stream(res, DisposeAfterUse::YES);
	uint16 strLen = 0;
	if (stream.getBit()) {
		strLen = stream.getBits(15);
	} else {
		strLen = stream.getBits(7);
	}
	uint32 mask = 0;
	uint32 symbol = 0;
	char c;
	for (uint16 i = 0; i < strLen; i++) {
		mask = stream.peekBits(16);

		uint32 entry;
		// Find the length index
		for (entry = 0; entry < _huffman->getNumEntries(); entry++) {
			if (mask < _huffman->getMask(entry)) {
				break;
			}
		}

		stream.skip(_huffman->getLength(entry));

		symbol = _huffman->getSymbol(entry);

		if (symbol == 1) { // 7-bit ascii
			c = stream.getBits(7);
			_decoded += c;
		} else if (symbol == 2) { // Composite
			if (stream.getBit()) { // TextID
				ObjID embedId = stream.getBits(15);
				uint pos = stream.pos(); // HACK, part 1
				TextAsset embedded(_engine, embedId, _sourceObj, _targetObj, _container, _isOld, _huffman);
				stream.rewind();// HACK, part 2
				stream.skip(pos);

				_decoded.replace(_decoded.end(), _decoded.end(), *embedded.decode());

				// Another HACK, to get around that EOS char I insert at the end
				_decoded.replace(_decoded.end() - 1, _decoded.end(), "");
			} else { //Composite obj string
				ObjID embedId = stream.getBits(8);
				uint pos = stream.pos(); // HACK, part 1

				_decoded.replace(_decoded.end(), _decoded.end(), getNoun(embedId));
				stream.rewind();// HACK, part 2
				stream.skip(pos);

				// Another HACK, to get around that EOS char I insert at the end
				_decoded.replace(_decoded.end() - 1, _decoded.end(), "");
			}
		} else { // Plain ascii
			c = symbol & 0xFF;
			_decoded.replace(_decoded.end(), _decoded.end(), Common::String(c));
		}
	}
	_decoded += '\0';
	debugC(3, kMVDebugText, "Decoded string [%d] (new encoding): %s", _id, _decoded.c_str());
}
Common::String TextAsset::getNoun(ObjID subval) {
	ObjID obj;
	Common::String name;
	if (subval & 8) {
		obj = _targetObj;
	} else {
		obj = _sourceObj;
	}
	if ((subval & 3) == 1) {
		uint idx = _engine->getPrefixNdx(obj);
		idx = ((idx >> 4) & 3) + 1;
		name = _engine->getNoun(idx);
	} else {
		// HACK, there should be a pool of assets or something like in the GUI
		name = *TextAsset(_engine, obj, _sourceObj, _targetObj, _container, _isOld, _huffman).decode();
		switch (subval & 3) {
		case 2:
			name = _engine->getPrefixString(0, obj) + name;
			break;
		case 3:
			name = _engine->getPrefixString(2, obj) + name;
			break;
		default:
			break;
		}
	}
	if (name.size() && (subval & 4)) {
		Common::String tmp = name;
		name.toUppercase();
		name.replace(1, name.size() - 1, tmp, 1, tmp.size() - 1);
	}

	return name;
}

} // End of namespace MacVenture
