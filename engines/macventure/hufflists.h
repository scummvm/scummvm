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

#ifndef MACVENTURE_HUFFLIST_H
#define MACVENTURE_HUFFLIST_H

namespace MacVenture {

// The engine uses a <= comparison instead of ==, so I can't use Common::Huffman
class HuffmanLists {
public:
	HuffmanLists() {
		_numEntries = 0;
	}
	HuffmanLists(uint32 num, uint32 *lens, uint32 *masks, uint32 *symbols) {
		_numEntries = num;
		_lens = Common::Array<uint32>(lens, num);
		_masks = Common::Array<uint32>(masks, num);
		_symbols = Common::Array<uint32>(symbols, num);
	}
	~HuffmanLists() {}


	uint32 getNumEntries() const { return _numEntries; }
	uint32 getLength(uint32 index) const { return _lens[index]; }
	uint32 getMask(uint32 index) const { return _masks[index]; }
	uint32 getSymbol(uint32 index) const { return _symbols[index]; }

private:
	uint32 _numEntries;
	Common::Array<uint32> _lens;
	Common::Array<uint32> _masks;
	Common::Array<uint32> _symbols;
};

} // End of namespace MacVenture

#endif
