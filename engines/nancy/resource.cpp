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
#include "common/stream.h"
#include "common/memstream.h"
#include "graphics/surface.h"
#include "nancy/resource.h"

namespace Nancy {

static void readCifInfo20(Common::File &f, ResourceManager::CifInfo &info, uint32 *dataOffset = 0) {
	info.width = f.readUint16LE();
	info.pitch = f.readUint16LE();
	info.height = f.readUint16LE();
	info.depth = f.readByte();

	info.comp = f.readByte();
	if (dataOffset)
		*dataOffset = f.readUint32LE();
	info.size = f.readUint32LE();
	f.skip(4); // A 2nd size for obsolete Cif type 1
	info.compressedSize = f.readUint32LE();

	info.type = f.readByte();
}

class Decompressor {
public:
	// Decompresses data from input until the end of the stream
	// The output stream must have the right size for the decompressed data
	bool decompress(Common::ReadStream &input, Common::MemoryWriteStream &output);

private:
	enum {
		kBufSize = 4096,
		kBufStart = 4078
	};

	void init(Common::ReadStream &input, Common::WriteStream &output);
	bool readByte(byte &b);
	bool writeByte(byte b);

	byte _buf[kBufSize];
	uint _bufpos;
	bool _err;
	byte _val;
	Common::ReadStream *_input;
	Common::WriteStream *_output;
};

void Decompressor::init(Common::ReadStream &input, Common::WriteStream &output) {
	memset(_buf, ' ', kBufSize);
	_bufpos = kBufStart;
	_err = false;
	_val = 0;
	_input = &input;
	_output = &output;
}

bool Decompressor::readByte(byte &b) {
	b = _input->readByte();

	if (_input->eos())
		return false;

	if (_input->err())
		error("Read error encountered during decompression");

	b -= _val++;
	return true;
}

bool Decompressor::writeByte(byte b) {
	_output->writeByte(b);
	_buf[_bufpos++] = b;
	_bufpos &= kBufSize - 1;
	return true;
}

bool Decompressor::decompress(Common::ReadStream &input, Common::MemoryWriteStream &output) {
	init(input, output);
	uint16 bits = 0;

	while(1) {
		byte b;

		bits >>= 1;

		// The highest 8 bits are used to keep track of how many bits are left to process
		if (!(bits & 0x100)) {
			// Out of bits
			if (!readByte(b))
				break;
			bits = 0xff00 | b;
		}

		if (bits & 1) {
			// Literal byte
			if (!readByte(b))
				break;
			writeByte(b);
		} else {
			// Copy from buffer
			byte b2;
			if (!readByte(b) || !readByte(b2))
				break;

			uint16 offset = b | ((b2 & 0xf0) << 4);
			uint16 len = (b2 & 0xf) + 3;

			for (uint i = 0; i < len; i++)
				writeByte(_buf[(offset + i) & (kBufSize - 1)]);
		}
	}

	if (output.err() || output.pos() != output.size()) {
		warning("Failed to decompress resource");
		return false;
	}

	return true;
}

class CifFile {
public:
	CifFile(const Common::String &name, Common::File *f) : _name(name), _f(f) { };
	virtual ~CifFile();

	bool initialize();
	byte *getCifData(ResourceManager::CifInfo &info, uint *size = 0) const;
	void getCifInfo(ResourceManager::CifInfo &info) const;

	static const CifFile *load(const Common::String &name);

protected:
	virtual void readCifInfo(Common::File &f) = 0;

	ResourceManager::CifInfo _cifInfo;
	Common::String _name;
	Common::File *_f;
	uint32 _dataOffset;
};

CifFile::~CifFile() {
	delete _f;
}

bool CifFile::initialize() {
	readCifInfo(*_f);

	if (_f->eos() || _f->err()) {
		warning("Error reading from CifFile '%s'", _name.c_str());
		return false;
	}

	return true;
}

byte *CifFile::getCifData(ResourceManager::CifInfo &info, uint *size) const {
	uint dataSize = (_cifInfo.comp == 2 ? _cifInfo.compressedSize : _cifInfo.size);
	byte *buf = new byte[dataSize];

	if (_f->read(buf, dataSize) < dataSize) {
		warning("Failed to read CifFile '%s'", _name.c_str());
		delete[] buf;
		return 0;
	}

	if (size)
		*size = dataSize;
	info = _cifInfo;
	return buf;
}

void CifFile::getCifInfo(ResourceManager::CifInfo &info) const {
	info = _cifInfo;
}

class CifFile20 : public CifFile {
public:
	CifFile20(const Common::String &name, Common::File *f) : CifFile(name, f) { }
protected:
	virtual void readCifInfo(Common::File &f);
};

void CifFile20::readCifInfo(Common::File &f) {
	readCifInfo20(f, _cifInfo);
}

class CifFile21 : public CifFile {
public:
	CifFile21(const Common::String &name, Common::File *f) : CifFile(name, f) { }
protected:
	virtual void readCifInfo(Common::File &f);
};

void CifFile21::readCifInfo(Common::File &f) {
	f.skip(32);
	readCifInfo20(f, _cifInfo);
}

const CifFile *CifFile::load(const Common::String &name) {
	Common::File *f = new Common::File;
	CifFile *cifFile = 0;

	if (!f->open(name + ".cif")) {
		delete f;
		return 0;
	}

	char id[20];
	f->read(id, 20);
	id[19] = 0;

	if (f->eos() || Common::String(id) != "CIF FILE WayneSikes") {
		warning("Invalid id string found in CifFile '%s'", name.c_str());
		delete f;
		return 0;
	}

	// 4 bytes unused
	f->skip(4);

	// Probably some kind of version number
	uint32 ver;
	ver = f->readUint16LE() << 16;
	ver |= f->readUint16LE();

	switch(ver) {
	case 0x00020000:
		cifFile = new CifFile20(name, f);
		break;
	case 0x00020001:
		cifFile = new CifFile21(name, f);
		break;
	default:
		warning("Unsupported version %d.%d found in CifFile '%s'", ver >> 16, ver & 0xffff, name.c_str());
	}

	if (!cifFile || !cifFile->initialize()) {
		warning("Failed to read CifFile '%s'", name.c_str());
		delete cifFile;
		delete f;
		return 0;
	}

	return cifFile;
}

class CifTree {
public:
	CifTree(const Common::String &name, const Common::String &ext);
	virtual ~CifTree() { };
	bool initialize();
	void list(Common::Array<Common::String> &nameList, uint type) const;
	byte *getCifData(const Common::String &name, ResourceManager::CifInfo &info, uint *size = 0) const;
	bool getCifInfo(const Common::String &name, ResourceManager::CifInfo &info, uint32 *dataOffset = 0) const;
	const Common::String &getName() const { return _name; }

	static const CifTree *load(const Common::String &name, const Common::String &ext);

protected:
	enum {
		kHashMapSize = 1024
	};

	struct CifInfoChain {
		struct ResourceManager::CifInfo info;
		uint32 dataOffset;
		uint16 next;
	};

	virtual uint readHeader(Common::File &f) = 0;
	virtual void readCifInfo(Common::File &f, CifInfoChain &chain) = 0;

	uint16 _hashMap[kHashMapSize];
	Common::Array<CifInfoChain> _cifInfo;
	Common::String _name;
	Common::String _filename;
};

CifTree::CifTree(const Common::String &name, const Common::String &ext) : _name(name) {
	_filename = name + '.' + ext;
}

bool CifTree::initialize() {
	Common::File f;

	if (!f.open(_filename) || !f.seek(28))
		error("Failed to open CifTree '%s'", _name.c_str());

	int infoBlockCount = readHeader(f);

	for (int i = 0; i < kHashMapSize; i++)
		_hashMap[i] = f.readUint16LE();

	if (f.eos())
		error("Error reading CifTree '%s'", _name.c_str());

	for (int i = 0; i < infoBlockCount; i++) {
		CifInfoChain chain;
		memset(&chain, 0, sizeof(CifInfoChain));
		readCifInfo(f, chain);
		_cifInfo.push_back(chain);
	}

	f.close();
	return true;
}

void CifTree::list(Common::Array<Common::String> &nameList, uint type) const {
	for (uint i = 0; i < _cifInfo.size(); i++) {
		if (type == ResourceManager::kResTypeAny || _cifInfo[i].info.type == type)
			nameList.push_back(_cifInfo[i].info.name);
	}
}

bool CifTree::getCifInfo(const Common::String &name, ResourceManager::CifInfo &info, uint32 *dataOffset) const {
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
			if (dataOffset)
				*dataOffset = _cifInfo[index].dataOffset;
			return true;
		}
		index = _cifInfo[index].next;
	}

	warning("Couldn't find '%s' in CifTree '%s'", name.c_str(), _name.c_str());
	return false;
}

byte *CifTree::getCifData(const Common::String &name, ResourceManager::CifInfo &info, uint *size) const {
	uint32 dataOffset;

	if (!getCifInfo(name, info, &dataOffset))
		return 0;

	Common::File f;

	if (!f.open(_filename)) {
		warning("Failed to open CifTree '%s'", _name.c_str());
		return 0;
	}

	uint dataSize = (info.comp == 2 ? info.compressedSize : info.size);
	byte *buf = new byte[dataSize];

	if (!f.seek(dataOffset) || f.read(buf, dataSize) < dataSize) {
		warning("Failed to read data for '%s' from CifTree '%s'", name.c_str(), _name.c_str());
		delete[] buf;
		f.close();
		return 0;
	}

	f.close();
	if (size)
		*size = dataSize;
	return buf;
}

class CifTree20 : public CifTree {
public:
	CifTree20(const Common::String &name, const Common::String &ext) : CifTree(name, ext) { }
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

	f.skip(2); // Index of this block

	readCifInfo20(f, info, &chain.dataOffset);

	chain.next = f.readUint16LE();
	if (f.eos())
		error("Failed to read info block from CifTree");
}

class CifTree21 : public CifTree {
public:
	CifTree21(const Common::String &name, const Common::String &ext) : CifTree(name, ext), _hasLongNames(false), _hasOffsetFirst(false) { };

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
		chain.dataOffset = f.readUint32LE();
		chain.next = f.readUint16LE();
	}

	f.skip(32); // TODO

	readCifInfo20(f, info, (_hasOffsetFirst ? 0 : &chain.dataOffset));

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

const CifTree *CifTree::load(const Common::String &name, const Common::String &ext) {
	Common::File f;
	CifTree *cifTree = 0;

	if (!f.open(name + '.' + ext)) {
		warning("Failed to open CifTree '%s'", name.c_str());
		return 0;
	}

	char id[20];
	f.read(id, 20);
	id[19] = 0;

	if (f.eos() || Common::String(id) != "CIF TREE WayneSikes") {
		warning("Invalid id string found in CifTree '%s'", name.c_str());
		f.close();
		return 0;
	}

	// 4 bytes unused
	f.skip(4);

	// Probably some kind of version number
	uint32 ver;
	ver = f.readUint16LE() << 16;
	ver |= f.readUint16LE();

	f.close();

	switch(ver) {
	case 0x00020000:
		cifTree = new CifTree20(name, ext);
		break;
	case 0x00020001:
		cifTree = new CifTree21(name, ext);
		break;
	default:
		warning("Unsupported version %d.%d found in CifTree '%s'", ver >> 16, ver & 0xffff, name.c_str());
	}

	if (cifTree && !cifTree->initialize()) {
		warning("Failed to read CifTree '%s'", name.c_str());
		delete cifTree;
		cifTree = 0;
	}

	return cifTree;
}

ResourceManager::ResourceManager(NancyEngine *vm) : _vm(vm) {
}

ResourceManager::~ResourceManager() {
	for (uint i = 0; i < _cifTrees.size(); i++)
		delete _cifTrees[i];
}

bool ResourceManager::loadCifTree(const Common::String &name, const Common::String &ext) {
	// NOTE: It seems likely that multiple CifTrees can be open at the same time
	// For now, we just replace the current CifTree with the new one

	const CifTree *cifTree = CifTree::load(name, ext);

	if (!cifTree)
		return false;

	_cifTrees.push_back(cifTree);
	return true;
}

const CifTree *ResourceManager::findCifTree(const Common::String &name) const {
	for (uint i = 0; i < _cifTrees.size(); i++)
		if (_cifTrees[i]->getName().equalsIgnoreCase(name))
			return _cifTrees[i];

	warning("CifTree '%s' not loaded", name.c_str());
	return 0;
}

void ResourceManager::initialize() {
	if (!loadCifTree("ciftree", "dat"))
		error("Failed to read 'ciftree.dat'");
}

bool ResourceManager::getCifInfo(const Common::String &treeName, const Common::String &name, CifInfo &info) {
	const CifFile *cifFile = CifFile::load(name);

	if (cifFile) {
		cifFile->getCifInfo(info);
		delete cifFile;
	}

	const CifTree *cifTree = findCifTree(treeName);

	if (!cifTree)
		return 0;

	return cifTree->getCifInfo(name, info);
}

byte *ResourceManager::getCifData(const Common::String &treeName, const Common::String &name, CifInfo &info, uint *size) {
	const CifFile *cifFile = CifFile::load(name);
	byte *buf;

	if (cifFile) {
		buf = cifFile->getCifData(info, size);
		delete cifFile;
	} else {
		const CifTree *cifTree = findCifTree(treeName);
		if (!cifTree)
			return 0;

		buf = cifTree->getCifData(name, info, size);
	}

	if (buf && info.comp == kResCompression) {
		Decompressor dec;
		Common::MemoryReadStream input(buf, info.compressedSize);
		byte *raw = new byte[info.size];
		Common::MemoryWriteStream output(raw, info.size);
		if (!dec.decompress(input, output)) {
			warning("Failed to decompress '%s'", name.c_str());
			delete[] buf;
			delete[] raw;
			return 0;
		}
		delete[] buf;
		if (size)
			*size = output.size();
		return raw;
	}

	return buf;
}

byte *ResourceManager::loadCif(const Common::String &treeName, const Common::String &name, uint &size) {
	CifInfo info;
	return getCifData(treeName, name, info, &size);
}

byte *ResourceManager::loadData(const Common::String &treeName, const Common::String &name, uint &size) {
	CifInfo info;

	byte *buf = getCifData(treeName, name, info, &size);

	if (!buf)
		return 0;

	if (info.type != kResTypeData) {
		warning("Resource '%s' is not data", name.c_str());
		delete[] buf;
		return 0;
	}

	return buf;
}

bool ResourceManager::loadImage(const Common::String &treeName, const Common::String &name, Graphics::Surface &surf) {
	CifInfo info;

	byte *buf = getCifData(treeName, name, info);

	if (!buf)
		return false;

	if (info.type != kResTypeImage) {
		warning("Resource '%s' is not an image", name.c_str());
		delete[] buf;
		return false;
	}

	if (info.depth != 16) {
		warning("Image '%s' has unsupported depth %i", name.c_str(), info.depth);
		delete[] buf;
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

void ResourceManager::list(const Common::String &treeName, Common::Array<Common::String> &nameList, uint type) {
	const CifTree *cifTree = findCifTree(treeName);

	if (!cifTree)
		return;

	cifTree->list(nameList, type);
}

Common::String ResourceManager::getCifDescription(const Common::String &treeName, const Common::String &name) {
	CifInfo info;
	const CifTree *cifTree = findCifTree(treeName);

	if (!cifTree)
		return Common::String::format("Failed to open CifTree '%s'\n", treeName.c_str());

	if (!cifTree->getCifInfo(name, info))
		return Common::String::format("Couldn't find '%s' in CifTree '%s'\n", name.c_str(), treeName.c_str());

	Common::String desc;
	desc = Common::String::format("Name: %s\n", info.name.c_str());
	desc += Common::String::format("Type: %i\n", info.type);
	desc += Common::String::format("Compression: %i\n", info.comp);
	desc += Common::String::format("Size: %i\n", info.size);
	desc += Common::String::format("Compressed size: %i\n", info.compressedSize);
	desc += Common::String::format("Width: %i\n", info.width);
	desc += Common::String::format("Pitch: %i\n", info.pitch);
	desc += Common::String::format("Height: %i\n", info.height);
	desc += Common::String::format("Bit depth: %i\n", info.depth);
	return desc;
}

} // End of namespace Nancy
