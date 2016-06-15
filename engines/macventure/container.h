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
#include "common/bitstream.h"

namespace MacVenture {


struct ItemGroup {
	uint32 bitOffset; //It's really uint24
	uint32 offset; //It's really uint24
	uint32 lengths[64];
};

typedef uint32 ContainerHeader;

class Container {	

public:
	Container(char *filename) {
		if (!_file.open(filename))
			error("Could not open %s", filename);

		_res = _file.readStream(_file.size());
		_header = _res->readUint32BE();
		_simplified = false;
		
		if (!(_header & 0x80000000)) { 
			// Is simplified container
			_simplified = true;
			int dataLen = _res->size() - sizeof(_header);
			_lenObjs = _header;
			_numObjs = dataLen / _lenObjs;
		}
		else {
			_header &= 0x7fffffff;
			_res->seek(_header, SEEK_SET);
			_numObjs = _res->readUint16BE();

			for (int i = 0; i < 15; ++i)
				_huff[i] = _res->readUint16BE();

			for (int i = 0; i < 16; ++i)
				_lens[i] = _res->readByte();

			// Read groups
			uint numGroups = _numObjs / 64;
			if ((_numObjs % 64) > 0)
				numGroups++;

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
					//debug("reading mask from address %x", _res->pos());
					uint32 mask = _res->readUint32BE();
					mask >>= (16 - bits);
					mask &= 0xFFFF;
					debug(11, "Load mask of object &%d:%d is %x", i, j, mask);
					_res->seek(-4, SEEK_CUR);
					// Look in the Huffman table
					int x = 0;
					for (x = 0; x < 16; x++) {
						if (_huff[x] > mask) break;
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
						bits &= 0xf;
						_res->seek(2, SEEK_CUR);
					}
					bitSize >>= 4;
					if (bitSize) {
						length = _res->readUint32BE();
						_res->seek(-4, SEEK_CUR);
						bitSize--;
						if (bitSize == 0) length = 0;
						else length >>= (32 - bitSize) - bits;
						length &= (1 << bitSize) - 1;
						length |= 1 << bitSize;
						bits += bitSize;						
					}		

					group.lengths[j] = length;
					debug(11, "Load legth of object %d:%d is %d", i, j, length);
				}
				
				_groups.push_back(group);				
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
	/**
	* Must be called before retrieving an object.
	*/
	uint32 getItemByteSize(uint32 id) {
		if (_simplified) {
			return _lenObjs;
		} else {
			uint32 groupID = (id >> 6);
			uint32 objectIndex = id & 0x3f; // Index within the group
			return _groups[groupID].lengths[objectIndex];
		}
	}

	/**
	* Assumes storage is initialized.
	*/
	void getItem(uint32 id, void* storage) {		
		if (_simplified) {
			_res->seek((id * _lenObjs) + sizeof(_header), SEEK_SET);
			_res->read(storage, _lenObjs);
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

			_res->read(storage, _groups[groupID].lengths[objectIndex]);
		}
	}
	
protected:

	bool _simplified;
	
	uint _lenObjs; // In the case of simple container, lenght of an object
	uint _numObjs;

	ContainerHeader _header;
	
	uint16 _huff[15];   // huffman masks
	uint8  _lens[16];   // huffman lengths
	Common::Array<ItemGroup> _groups;

	Common::File _file;
	Common::SeekableReadStream *_res;

	// To be moved
	//byte _remainderOffset;

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
