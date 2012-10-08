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

static void readCifInfo20(Common::File &f, ResourceManager::CifInfo &info, bool hasOffset = false) {
	info.width = f.readUint16LE();
	info.pitch = f.readUint16LE();
	info.height = f.readUint16LE();
	info.depth = f.readByte();

	info.comp = f.readByte();
	if (hasOffset)
		info.dataOffset = f.readUint32LE();
	info.size = f.readUint32LE();
	info.sizeUnk = f.readUint32LE(); // Unknown
	info.compressedSize = f.readUint32LE();

	info.type = f.readByte();
}

class CifTree {
public:
	virtual ~CifTree() { };
	bool initialize(Common::File &f);
	bool findResource(const Common::String &name, ResourceManager::CifInfo &info) const;
	void listResources(Common::Array<Common::String> &list, uint type) const;
	const Common::String &getName() const { return _filename; }

	static const CifTree *load(const Common::String filename);

protected:
	enum {
		kHashMapSize = 1024
	};

	struct CifInfoChain {
		struct ResourceManager::CifInfo info;
		uint16 next;
	};

	virtual uint readHeader(Common::File &f) = 0;
	virtual void readCifInfo(Common::File &f, CifInfoChain &chain) = 0;

	uint16 _hashMap[kHashMapSize];
	Common::Array<CifInfoChain> _cifInfo;
	Common::String _filename;
};

bool CifTree::initialize(Common::File &f) {
	_filename = f.getName();

	int infoBlockCount = readHeader(f);

	for (int i = 0; i < kHashMapSize; i++)
		_hashMap[i] = f.readUint16LE();

	if (f.eos())
		error("Error reading CifTree '%s'", _filename.c_str());

	for (int i = 0; i < infoBlockCount; i++) {
		CifInfoChain chain;
		memset(&chain, 0, sizeof(CifInfoChain));
		readCifInfo(f, chain);
		_cifInfo.push_back(chain);
	}

	return true;
}

void CifTree::listResources(Common::Array<Common::String> &list, uint type) const {
	for (uint i = 0; i < _cifInfo.size(); i++) {
		if (type == ResourceManager::kResTypeAny || _cifInfo[i].info.type == type)
			list.push_back(_cifInfo[i].info.name);
	}
}

bool CifTree::findResource(const Common::String &name, ResourceManager::CifInfo &info) const {
	Common::String nameUpper = name;
	nameUpper.toUppercase();
	uint hash = 0;

	for (uint i = 0; i < nameUpper.size(); i++)
		hash += nameUpper[i];

	hash &= kHashMapSize - 1;

	uint16 index = _hashMap[hash];
	while (index != 0xffff) {
		if (nameUpper == _cifInfo[index].info.name) {
			info = _cifInfo[index].info;
			return true;
		}
		index = _cifInfo[index].next;
	}

	return false;
}

class CifTree20 : public CifTree {
protected:
	virtual uint readHeader(Common::File &f);
	virtual void readCifInfo(Common::File &f, CifInfoChain &chain);
};

uint CifTree20::readHeader(Common::File &f) {
	uint infoBlockCount = f.readUint16LE();

	if (f.eos())
		error("Failed to read cif info block count from CifTree");

	return infoBlockCount;
}

void CifTree20::readCifInfo(Common::File &f, CifInfoChain &chain) {
	ResourceManager::CifInfo &info = chain.info;

	char name[9];
	f.read(name, 9);
	name[8] = 0;
	info.name = name;

	readCifInfo20(f, info, true);

	chain.next = f.readUint16LE();
	if (f.eos())
		error("Failed to read info block from CifTree");
}

class CifTree21 : public CifTree {
public:
	CifTree21() : _hasLongNames(false), _hasOffsetFirst(false) { };

protected:
	virtual uint readHeader(Common::File &f);
	virtual void readCifInfo(Common::File &f, CifInfoChain &chain);

private:
	void determineSubtype(Common::File &f);
	bool _hasLongNames;
	bool _hasOffsetFirst;
};

uint CifTree21::readHeader(Common::File &f) {
	uint infoBlockCount = f.readUint16LE();

	if (f.eos())
		error("Failed to read cif info block count from CifTree");

	f.readByte(); // Unknown
	f.readByte(); // Unknown

	determineSubtype(f);

	return infoBlockCount;
}

void CifTree21::readCifInfo(Common::File &f, CifInfoChain &chain) {
	ResourceManager::CifInfo &info = chain.info;
	int nameSize = 8;

	if (_hasLongNames)
		nameSize = 32;

	char name[33];
	f.read(name, nameSize + 1);
	name[nameSize] = 0;
	info.name = name;

	f.skip(2); // Index of this block

	if (_hasOffsetFirst) {
		info.dataOffset = f.readUint32LE();
		chain.next = f.readUint16LE();
	}

	f.skip(32); // TODO

	readCifInfo20(f, info, !_hasOffsetFirst);

	if (!_hasOffsetFirst)
		chain.next = f.readUint16LE();
}

void CifTree21::determineSubtype(Common::File &f) {
	// Perform heuristic for long filenames
	// Assume short file names and read indices 1 and 2
	uint pos = f.pos();

	f.seek(2159);
	uint16 index1 = f.readUint16LE();

	f.seek(68, SEEK_CUR);
	uint16 index2 = f.readUint16LE();

	// If they don't match, this file must have long filenames
	if (index1 != 1 || index2 != 2)
		_hasLongNames = true;

	if (_hasLongNames) {
		// Perform heuristic for offset at the beginning of the block
		// Read offset and next of the first info block
		// If either of these is zero, offset can't be first
		f.seek(2115);
		uint32 offset = f.readUint32LE();
		uint16 next = f.readUint32LE();
		if (offset && next)
			_hasOffsetFirst = true;
	}

	f.seek(pos);
}

const CifTree *CifTree::load(const Common::String filename) {
	Common::File f;
	CifTree *cifTree = 0;

	if (!f.open(filename)) {
		warning("Failed to open CifTree '%s'", filename.c_str());
		return 0;
	}

	char id[20];
	f.read(id, 20);
	id[19] = 0;

	if (f.eos() || Common::String(id) != "CIF TREE WayneSikes") {
		warning("Invalid id string found in CifTree '%s'", filename.c_str());
		f.close();
		return 0;
	}

	// 4 bytes unused
	f.skip(4);

	// Probably some kind of version number
	uint32 ver;
	ver = f.readUint16LE() << 16;
	ver |= f.readUint16LE();

	switch(ver) {
	case 0x00020000:
		cifTree = new CifTree20;
		break;
	case 0x00020001:
		cifTree = new CifTree21;
		break;
	default:
		warning("Unsupported version %d.%d found in CifTree '%s'", ver >> 16, ver & 0xffff, filename.c_str());
	}

	if (cifTree && !cifTree->initialize(f)) {
		warning("Failed to read CifTree '%s'", filename.c_str());
		delete cifTree;
		cifTree = 0;
	}

	f.close();
	return cifTree;
}

ResourceManager::ResourceManager(NancyEngine *vm) : _vm(vm), _cifTree(0) {
}

ResourceManager::~ResourceManager() {
}

bool ResourceManager::loadCifTree(const Common::String filename) {
	// NOTE: It seems likely that multiple CifTrees can be open at the same time
	// For now, we just replace the current CifTree with the new one

	const CifTree *cifTree = CifTree::load(filename);

	if (!cifTree)
		return false;

	// Delete previous tree
	if (_cifTree)
		delete _cifTree;

	_cifTree = cifTree;
	return true;
}

void ResourceManager::initialize() {
	if (!loadCifTree("ciftree.dat"))
		error("Failed to read 'ciftree.dat'");
}

byte *ResourceManager::loadResource(const Common::String name, uint &size) {
	CifInfo info;

	if (!_cifTree->findResource(name, info)) {
		warning("Resource '%s' not found", name.c_str());
		return 0;
	}

	byte *buf = decompress(info, size);
	return buf;
}

bool ResourceManager::loadImage(const Common::String name, Graphics::Surface &surf) {
	CifInfo info;

	if (!_cifTree->findResource(name, info)) {
		warning("Resource '%s' not found", name.c_str());
		return false;
	}

	if (info.type != kResTypeImage) {
		warning("Resource '%s' is not an image", name.c_str());
		return false;
	}

	if (info.depth != 16) {
		warning("Image '%s' has unsupported depth %i", name.c_str(), info.depth);
		return false;
	}

	uint size;
	byte *buf = decompress(info, size);

	if (!buf) {
		warning("Failed to decompress image '%s'", name.c_str());
		return false;
	}

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
		warning("Resource '%s' is not compressed", info.name.c_str());
		return 0;
	}

	if (!cifTree.open(_cifTree->getName())) {
		warning("Failed to open '%s'", _cifTree->getName().c_str());
		return 0;
	}

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
		warning("Failed to decompress resource");
		delete[] output;
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
