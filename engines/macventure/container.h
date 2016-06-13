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

#ifndef MACVENTURE_CONTAINER_H
#define MACVENTURE_CONTAINER_H

#include "macventure/macventure.h"

#include "common/file.h"

namespace MacVenture {


struct ItemGroup {
	uint32 bitOffset; //It's really uint24
	uint32 offset; //It's really uint24
	uint32 lengths[64];
};

typedef uint32 ContainerHeader;

template <class T>
class Container {	

public:
	Container(char *filename) {
		if (!_file.open(filename))
			error("Could not open %s", filename);

		_res = _file.readStream(_file.size());
		_header = _res->readUint32BE();
		
		if (!(_header & 0x80000000)) { 
			// Is simplified container
			int dataLen = _res->size() - sizeof(_header);
			_lenObjs = _header;
			_numObjs = dataLen / _lenObjs;
		}
		else {
			ContainerHeader subHead = _header & 0x7fffffff;
			_res->seek(subHead, SEEK_SET);
			_numObjs = _res->readUint16BE();

			for (int i = 0; i < 15; ++i)
				_huff[i] = _res->readUint16BE();

			for (int i = 0; i < 16; ++i)
				_lens[i] = _res->readByte();

			ItemGroup group;
			for (int i = 0; i < _numObjs; ++i) {	
				uint32 bits;
				if ((i & 0x37) == 0) { // It's the start of a group					
					// Place myself in the correct position to read group
					_res->seek(subHead + (i >> 6) * 6 + 0x30, SEEK_SET);
					_res->read(&bits, 3);
					bits >>= 4;
					_res->read(&group.offset, 3); // Read 3 bytes
					group.offset >>= 4;
					bits &= 7;
					group.bitOffset = bits;
					_res->seek(subHead + (bits >> 3), SEEK_SET);
				}	

				// Workaround to implement peek
				// Read the value in 32 bits
				uint32 v = (_res->readUint32BE() >> (16 - bits)) & 0xffff;
				// Go back
				_res->seek(-4, SEEK_CUR); 

				// Look in the Huffman table
				int x;
				for (x = 0; x<16; x++)
					if (_huff[x] > v) break;

				// Bits that we need to read from the length table
				uint8 bitsToRead = _lens[x];

				bits += (bitsToRead & 0xf);
				if (bits & 0x10) {
					bits &= 0xf;
					_res->seek(2, SEEK_CUR);
				}

				// We already have in bits the first 4 bits (97)
				bitsToRead = bitsToRead >> 4;

				// The actual length of the object
				uint32 len = 0;
				if (bitsToRead) {
					// Peek 4 bytes
					len = _res->readUint32BE();
					_res->seek(-4, SEEK_CUR);

					bitsToRead--;

					if (bitsToRead == 0) len = 0;
					else len >>= (32 - bitsToRead) - bits;

					len &= (1 << bitsToRead) - 1;
					len |= 1 << bitsToRead;

					if (bits & 0x10) {
						bits &= 0xf;
						_res->seek(2, SEEK_CUR);
					}

				}

				group.lengths[(i & 0x3f)] = len;

				if ((i & 0x37) == 0) {
					_groups.push_back(group);
				}
			}			 
		}
	}

	~Container() {

		if (_file.isOpen())
			_file.close();

		if (_res)
			delete _res;
	}

public:
	T getItem(uint32 id) {
		T item;
		if (!(_header & 0x80000000)) {
			_res->seek((id * _lenObjs) + sizeof(_header), SEEK_SET);
			_res->read(&item, _lenObjs);
		} else {
			ContainerHeader subHead = _header & 0x7fffffff;
			uint32 groupID = (id >> 6);
			uint32 objectIndex = id & 0x3f; // Index within the group

			_res->seek(subHead + (groupID * 6), SEEK_SET);

			uint32 offset = 0;
			for (int i = 0; i < objectIndex; i++) {
				offset += _groups[groupID].lengths[i];
			}

			_res->seek(offset, SEEK_CUR);

			_res->read(&item, _groups[groupID].lengths[objectIndex]);
		}
		return item;
	}

protected:
	
	uint _lenObjs;
	uint _numObjs;

	ContainerHeader _header;
	
	uint16 _huff[15];   // huffman masks
	uint8  _lens[16];   // huffman lengths
	Common::Array<ItemGroup> _groups;

	Common::File _file;
	Common::SeekableReadStream *_res;

};

/*
template <typedef T>
class PersistentContainer : public Container {
public:
	PersistentContainer(Common::String filename) :
		Container(filename) {
		// Load 
	}

private:

};*/

} // End of namespace MacVenture

#endif
