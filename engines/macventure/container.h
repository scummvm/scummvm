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

/*
 * Based on
 * WebVenture (c) 2010, Sean Kasun
 * https://github.com/mrkite/webventure, http://seancode.com/webventure/
 *
 * Used with explicit permission from the author
 */

#ifndef MACVENTURE_CONTAINER_H
#define MACVENTURE_CONTAINER_H

#include "macventure/macventure.h"

#include "common/file.h"
#include "common/fs.h"

namespace MacVenture {

struct ItemGroup {
	uint32 bitOffset; //It's really uint24
	uint32 offset; //It's really uint24
	uint32 lengths[64];
};

typedef uint32 ContainerHeader;

class Container {

public:
	Container(const Common::Path &filename);
	~Container();

public:
	/**
	* Must be called before retrieving an object.
	*/
	uint32 getItemByteSize(uint32 id);

	/**
	* getItemByteSize should be called before this one
	*/
	Common::SeekableReadStream *getItem(uint32 id);

protected:

	bool _simplified;

	uint _lenObjs; // In the case of simple container, length of an object
	uint _numObjs;

	ContainerHeader _header;

	Common::Array<uint16> _huff; // huffman masks
	Common::Array<uint8> _lens; // huffman lengths
	Common::Array<ItemGroup> _groups;

	Common::Path _filename;
	Common::File _file;
	Common::SeekableReadStream *_res;

};


} // End of namespace MacVenture

#endif
