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

#include "macventure/container.h"

namespace MacVenture {

Container::Container(Common::String filename) {
	_filename = filename;

	if (!_file.open(_filename)) {
		error("CONTAINER: Could not open %s", _filename.c_str());
	}

	_res = _file.readStream(_file.size());
	_header = _res->readUint32BE();
	_simplified = false;

	for (uint i = 0; i < 16; ++i) {
		_huff.push_back(0);
	}

	for (uint i = 0; i < 16; ++i) {
		_lens.push_back(0);
	}

	if (!(_header & 0x80000000)) {
		// Is simplified container
		_simplified = true;
		int dataLen = _res->size() - sizeof(_header);
		_lenObjs = _header;
		_numObjs = dataLen / _lenObjs;
	} else {
		_header &= 0x7fffffff;
		_res->seek(_header, SEEK_SET);
		_numObjs = _res->readUint16BE();

		for (uint i = 0; i < 15; ++i) {
			_huff[i] = _res->readUint16BE();
		}

		for (uint i = 0; i < 16; ++i) {
			_lens[i] = _res->readByte();
		}

		// Read groups
		uint numGroups = _numObjs / 64;
		if ((_numObjs % 64) > 0) {
			numGroups++;
		}

		for (uint i = 0; i < numGroups; ++i) {
			ItemGroup group;

			// Place myself in the correct position to read group
			_res->seek(_header + (i * 6) + 0x30, SEEK_SET);
			byte b1, b2, b3;
			b1 = _res->readByte();
			b2 = _res->readByte();
			b3 = _res->readByte();
			group.bitOffset = (b1 << 16) + (b2 << 8) + (b3 << 0);

			b1 = _res->readByte();
			b2 = _res->readByte();
			b3 = _res->readByte();
			group.offset = (b1 << 16) + (b2 << 8) + (b3 << 0);

			// Place the bit reader in the correct position
			// group.bitOffset indicates the offset from the start of the subHeader
			_res->seek(_header + (group.bitOffset >> 3), SEEK_SET);
			uint32 bits = group.bitOffset & 7;

			for (uint j = 0; j < 64; ++j) {
				uint32 length = 0;
				uint32 mask = 0;
				mask = _res->readUint32BE();
				mask >>= (16 - bits);
				mask &= 0xFFFF;
				debugC(4, kMVDebugContainer, "Load mask of object &%d:%d is %x", i, j, mask);
				_res->seek(-4, SEEK_CUR);
				// Look in the Huffman table
				int x = 0;
				for (x = 0; x < 16; x++) {
					if (_huff[x] > mask) {
						break;
					}
				}

				// I will opt to copy the code from webventure,
				// But according to the docs, this call should suffice:
				// length = bitStream.getBits(_lens[x]);
				// The problem is that _lens[] usually contains values larger
				// Than 32, so we have to read them with the method below

				//This code below, taken from the implementation, seems to give the same results.

				uint32 bitSize = _lens[x];
				bits += bitSize & 0xF;
				if (bits & 0x10) {
					bits &= 0xF;
					_res->seek(2, SEEK_CUR);
				}
				bitSize >>= 4;
				if (bitSize) {
					length = _res->readUint32BE();
					_res->seek(-4, SEEK_CUR);
					bitSize--;
					if (bitSize == 0) {
						length = 0;
					} else {
						length >>= (32 - bitSize) - bits;
					}
					length &= (1 << bitSize) - 1;
					length |= 1 << bitSize;
					bits += bitSize;
					if (bits & 0x10) {
						bits &= 0xF;
						_res->seek(2, SEEK_CUR);
					}
				}
				group.lengths[j] = length;
				debugC(4, kMVDebugContainer, "Load legth of object %d:%d is %d", i, j, length);
			}
			_groups.push_back(group);
		}
	}
}

Container::~Container() {

	if (_file.isOpen())
		_file.close();

	if (_res)
		delete _res;
}

uint32 Container::getItemByteSize(uint32 id) {
	if (_simplified) {
		return _lenObjs;
	} else {
		uint32 groupID = (id >> 6);
		uint32 objectIndex = id & 0x3f; // Index within the group
		return _groups[groupID].lengths[objectIndex];
	}
}

Common::SeekableReadStream *Container::getItem(uint32 id) {
	if (_simplified) {
		_res->seek((id * _lenObjs) + sizeof(_header), SEEK_SET);
	} else {
		uint32 groupID = (id >> 6);
		uint32 objectIndex = id & 0x3f; // Index within the group

		uint32 offset = 0;
		for (uint i = 0; i < objectIndex; i++) {
			offset += _groups[groupID].lengths[i];
		}
		_res->seek(_groups[groupID].offset + offset + sizeof(_header), SEEK_SET);
	}

	// HACK Should Limit the size of the stream returned
	Common::SeekableReadStream *res = _res->readStream(_res->size() - _res->pos() + 1);
	return res;
}


} // End of namespace MacVenture
