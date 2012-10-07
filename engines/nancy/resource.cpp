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

#include "common/file.h"
#include "common/textconsole.h"
#include "common/debug.h"
#include "common/memstream.h"
#include "graphics/surface.h"
#include "nancy/resource.h"

namespace Nancy {

class CifTree {
public:
	virtual ~CifTree() { };	
	bool findResource(const Common::String name, ResourceManager::CifInfo &info);
	void listResources(Common::Array<Common::String> &list, uint type);

protected:
	enum {
		kHashMapSize = 1024
	};

	struct CifInfoChain {
		struct ResourceManager::CifInfo info;
		uint16 next;
	};

	uint16 _hashMap[kHashMapSize];
	Common::Array<CifInfoChain> _cifInfo;
};

void CifTree::listResources(Common::Array<Common::String> &list, uint type) {
	for (uint i = 0; i < _cifInfo.size(); i++) {
		if (type == ResourceManager::kResTypeAny || _cifInfo[i].info.type == type)
			list.push_back(_cifInfo[i].info.name);
	}
}

bool CifTree::findResource(const Common::String name, ResourceManager::CifInfo &info) {
	uint hash = 0;

	for (uint i = 0; i < name.size(); i++)
		hash += name[i];

	hash &= kHashMapSize - 1;

	uint16 index = _hashMap[hash];
	while (index != 0xffff) {
		if (name == _cifInfo[index].info.name) {
			info = _cifInfo[index].info;
			return true;
		}
		index = _cifInfo[index].next;
	}

	return false;
}

class CifTree20 : public CifTree {
public:
	CifTree20(Common::File &f);
	virtual ~CifTree20() { };
};

CifTree20::CifTree20(Common::File &f) {
	// Number of info blocks
	int infoBlockCount = f.readUint16LE();

	for (int i = 0; i < kHashMapSize; i++)
		_hashMap[i] = f.readUint16LE();

	if (f.eos())
		error("Failed to read hash map from ciftree.dat");

	for (int i = 0; i < infoBlockCount; i++) {
		CifInfoChain chain;
		ResourceManager::CifInfo &info = chain.info;

		char name[9];
		f.read(name, 9);
		name[8] = 0;
		info.name = name;

		f.skip(2); // Index of this block
		info.width = f.readUint16LE();
		info.pitch = f.readUint16LE();
		info.height = f.readUint16LE();
		info.depth = f.readByte();

		info.comp = f.readByte();
		info.dataOffset = f.readUint32LE();
		info.size = f.readUint32LE();
		info.sizeUnk = f.readUint32LE(); // Unknown
		info.compressedSize = f.readUint32LE();

		info.type = f.readByte();

		chain.next = f.readUint16LE();
		if (f.eos())
			error("Failed to read info blocks from ciftree.dat");

		_cifInfo.push_back(chain);
	}
}

ResourceManager::ResourceManager(NancyEngine *vm) : _vm(vm) {
}

ResourceManager::~ResourceManager() {
}

void ResourceManager::initialize() {
	Common::File cifTree;

	if (!cifTree.open("ciftree.dat"))
		error("Failed to open ciftree.dat");

	char id[20];
	cifTree.read(id, 20);
	id[19] = 0;

	if (cifTree.eos() || Common::String(id) != "CIF TREE WayneSikes")
		error("Invalid id string found in ciftree.dat");

	// 4 bytes unused
	cifTree.skip(4);

	// Probably some kind of version number
	uint32 ver;
	ver = cifTree.readUint16LE() << 16;
	ver |= cifTree.readUint16LE();

	switch(ver) {
	case 0x00020000:
		_cifTree = new CifTree20(cifTree);
		break;
	default:
		error("CifTree version %d.%d not supported", ver >> 16, ver & 0xffff);
	}

	cifTree.close();
}

byte *ResourceManager::loadResource(const Common::String name, uint &size) {
	CifInfo info;

	if (!_cifTree->findResource(name, info)) {
		debug("Resource '%s' not found", name.c_str());
		return 0;
	}

	byte *buf = decompress(info, size);
	return buf;
}

bool ResourceManager::loadImage(const Common::String name, Graphics::Surface &surf) {
	CifInfo info;

	if (!_cifTree->findResource(name, info)) {
		debug("Resource '%s' not found", name.c_str());
		return 0;
	}

	if (info.type != kResTypeImage) {
		debug("Resource '%s' is not an image", name.c_str());
		return 0;
	}

	if (info.depth != 16) {
		debug("Image '%s' has unsupported depth %i", name.c_str(), info.depth);
		return 0;
	}

	uint size;
	byte *buf = decompress(info, size);

	Graphics::PixelFormat format(2, 5, 5, 5, 0, 10, 5, 0, 0);
	surf.w = info.width;
	surf.h = info.height;
	surf.pitch = info.pitch;
	surf.setPixels(buf);
	surf.format = format;
	return true;
}

byte *ResourceManager::decompress(const CifInfo &info, uint &size) {
	// TODO: clean this up...
	Common::File cifTree;
	uint read = 0, written = 0;

	if (info.comp != kResCompression) {
		debug("Resource '%s' is not compressed", info.name.c_str());
		return 0;
	}

	if (!cifTree.open("ciftree.dat"))
		error("Failed to open ciftree.dat");

	cifTree.seek(info.dataOffset);

	const int bufSize = 4096;
	const int bufStart = 4078;
	byte *buf = new byte[bufSize];
	byte *output = new byte[info.size];
	uint bufpos = bufStart;

	memset(buf, ' ', bufStart);
	uint16 bits = 0;
	byte val = 0;

	while(1) {
		bits >>= 1;

		// The highest 8 bits are used to keep track of how many bits are left to process
		if (!(bits & 0x100)) {
			// Out of bits
			if (cifTree.eos() || read == info.compressedSize)
				break;
			bits = 0xff00 | ((cifTree.readByte() - val++) & 0xff);
			++read;
		}

		if (bits & 1) {
			// Literal byte
			if (cifTree.eos() || read == info.compressedSize)
				break;
			byte b = cifTree.readByte() - val++;
			++read;
			output[written++] = b;
			if (written == info.size)
				break;
			buf[bufpos++] = b;
			bufpos &= bufSize - 1;
		} else {
			// Copy from buffer
			if (cifTree.eos() || read == info.compressedSize)
				break;
			byte b1 = cifTree.readByte() - val++;
			++read;

			if (cifTree.eos() || read == info.compressedSize)
				break;
			byte b2 = cifTree.readByte() - val++;
			++read;

			uint16 offset = b1 | ((b2 & 0xf0) << 4);
			uint16 len = (b2 & 0xf) + 3;
	
			for (uint i = 0; i < len; i++) {
				byte t = buf[(offset + i) & (bufSize - 1)];
				output[written++] = t;
				if (written == info.size)
					break;
				buf[bufpos++] = t;
				bufpos &=  bufSize - 1;
			}
		}
	}

	delete[] buf;

	if (read != info.compressedSize || written != info.size) {
		debug("Failed to decompress resource");
		return 0;
	}

	size = written;
	return output;
}

void ResourceManager::listResources(Common::Array<Common::String> &list, uint type) {
	_cifTree->listResources(list, type);
}

Common::String ResourceManager::getResourceDesc(const Common::String name) {
	CifInfo info;

	if (!_cifTree->findResource(name, info))
		return Common::String::format("Resource '%s' not found\n", name.c_str());

	Common::String desc;
	desc = Common::String::format("Name: %s\n", info.name.c_str());
	desc += Common::String::format("Type: %i\n", info.type);
	desc += Common::String::format("Compression: %i\n", info.comp);
	desc += Common::String::format("Data offset: %i\n", info.dataOffset);
	desc += Common::String::format("Size: %i\n", info.size);
	desc += Common::String::format("Size (unknown): %i\n", info.sizeUnk);
	desc += Common::String::format("Compressed size: %i\n", info.compressedSize);
	desc += Common::String::format("Width: %i\n", info.width);
	desc += Common::String::format("Pitch: %i\n", info.pitch);
	desc += Common::String::format("Height: %i\n", info.height);
	desc += Common::String::format("Bit depth: %i\n", info.depth);
	return desc;
}

} // End of namespace Nancy
