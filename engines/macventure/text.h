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

#ifndef MACVENTURE_TEXT_H
#define MACVENTURE_TEXT_H

#include "macventure/macventure.h"
#include "macventure/hufflists.h"

namespace MacVenture {
typedef uint32 ObjID;
class MacVentureEngine;

class TextAsset {
public:
	TextAsset(MacVentureEngine *engine, ObjID objid, ObjID source, ObjID target, Container *container, bool isOld, const HuffmanLists *huffman);
	~TextAsset() {}

	const Common::String *decode() {
		return &_decoded;
	}

private:
	void decodeOld();
	void decodeHuffman();

	Common::String getNoun(ObjID id);

private:
	MacVentureEngine *_engine;

	Container *_container;
	ObjID _id;
	ObjID _targetObj;
	ObjID _sourceObj;
	const HuffmanLists *_huffman;
	bool _isOld;

	Common::String _decoded;

};

} // End of namespace MacVenture

#endif
