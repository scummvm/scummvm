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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/memstream.h"

#include "image/bmp.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/decompress.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/util.h"

namespace Nancy {

static void readCifInfo20(Common::File &f, ResourceManager::CifInfo &info, uint32 *dataOffset = nullptr) {
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

class CifFile {
public:
	CifFile(const Common::String &name, Common::File *f) : _name(name), _f(f), _dataOffset(0) { };
	virtual ~CifFile();

	bool initialize();
	byte *getCifData(ResourceManager::CifInfo &info, uint *size = nullptr) const;
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

	_cifInfo.name = _name;

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
		return nullptr;
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
	void readCifInfo(Common::File &f) override;
};

void CifFile20::readCifInfo(Common::File &f) {
	readCifInfo20(f, _cifInfo);
}

class CifFile21 : public CifFile {
public:
	CifFile21(const Common::String &name, Common::File *f) : CifFile(name, f) { }
protected:
	void readCifInfo(Common::File &f) override;
};

void CifFile21::readCifInfo(Common::File &f) {
	readRect(f, _cifInfo.src);
	readRect(f, _cifInfo.dest);
	readCifInfo20(f, _cifInfo);
}

const CifFile *CifFile::load(const Common::String &name) {
	Common::File *f = new Common::File;
	CifFile *cifFile = nullptr;

	if (!f->open(name + ".cif")) {
		delete f;
		return nullptr;
	}

	char id[20];
	f->read(id, 20);
	id[19] = 0;

	if (f->eos() || Common::String(id) != "CIF FILE WayneSikes") {
		warning("Invalid id string found in CifFile '%s'", name.c_str());
		delete f;
		return nullptr;
	}

	// 4 bytes unused
	f->skip(4);

	// Probably some kind of version number
	uint32 ver;
	ver = f->readUint16LE() << 16;
	ver |= f->readUint16LE();

	switch (ver) {
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
		return nullptr;
	}

	return cifFile;
}

class CifTree {
public:
	CifTree(const Common::String &name, const Common::String &ext);
	virtual ~CifTree() { };
	virtual uint32 getVersion() const = 0;

	bool initialize();
	void list(Common::Array<Common::String> &nameList, uint type) const;
	byte *getCifData(const Common::String &name, ResourceManager::CifInfo &info, uint *size = nullptr) const;
	bool getCifInfo(const Common::String &name, ResourceManager::CifInfo &info, uint32 *dataOffset = nullptr) const;
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

	_cifInfo.reserve(infoBlockCount);
	for (int i = 0; i < infoBlockCount; i++) {
		CifInfoChain chain;
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
		return nullptr;

	Common::File f;

	if (!f.open(_filename)) {
		warning("Failed to open CifTree '%s'", _name.c_str());
		return nullptr;
	}

	uint dataSize = (info.comp == 2 ? info.compressedSize : info.size);
	byte *buf = new byte[dataSize];

	if (!f.seek(dataOffset) || f.read(buf, dataSize) < dataSize) {
		warning("Failed to read data for '%s' from CifTree '%s'", name.c_str(), _name.c_str());
		delete[] buf;
		f.close();
		return nullptr;
	}

	f.close();
	if (size)
		*size = dataSize;
	return buf;
}

byte *ResourceManager::getCifData(const Common::String &treeName, const Common::String &name, CifInfo &info, uint *size) const {
	const CifFile *cifFile = CifFile::load(name);
	byte *buf;

	if (cifFile) {
		buf = cifFile->getCifData(info, size);
		delete cifFile;
	} else {
		const CifTree *cifTree = findCifTree(treeName);
		if (!cifTree)
			return nullptr;

		buf = cifTree->getCifData(name, info, size);
	}

	if (buf && info.comp == kResCompression) {
		Common::MemoryReadStream input(buf, info.compressedSize);
		byte *raw = new byte[info.size];
		Common::MemoryWriteStream output(raw, info.size);
		if (!_dec->decompress(input, output)) {
			warning("Failed to decompress '%s'", name.c_str());
			delete[] buf;
			delete[] raw;
			return nullptr;
		}
		delete[] buf;
		if (size)
			*size = output.size();
		return raw;
	}

	return buf;
}

class CifTree20 : public CifTree {
public:
	CifTree20(const Common::String &name, const Common::String &ext) : CifTree(name, ext) { }
protected:
	uint readHeader(Common::File &f) override;
	void readCifInfo(Common::File &f, CifInfoChain &chain) override;
	uint32 getVersion() const override { return 0x00020000; }
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

class CifTree21 : public CifTree20 {
public:
	CifTree21(const Common::String &name, const Common::String &ext) : CifTree20(name, ext), _hasLongNames(false), _hasOffsetFirst(false) { };

protected:
	uint readHeader(Common::File &f) override;
	void readCifInfo(Common::File &f, CifInfoChain &chain) override;
	uint32 getVersion() const override { return 0x00020001; }

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

	readRect(f, info.src);
	readRect(f, info.dest);

	readCifInfo20(f, info, (_hasOffsetFirst ? nullptr : &chain.dataOffset));

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
	CifTree *cifTree = nullptr;

	if (!f.open(name + '.' + ext)) {
		warning("Failed to open CifTree '%s'", name.c_str());
		return nullptr;
	}

	char id[20];
	f.read(id, 20);
	id[19] = 0;

	if (f.eos() || Common::String(id) != "CIF TREE WayneSikes") {
		warning("Invalid id string found in CifTree '%s'", name.c_str());
		f.close();
		return nullptr;
	}

	// 4 bytes unused
	f.skip(4);

	// Probably some kind of version number
	uint32 ver;
	ver = f.readUint16LE() << 16;
	ver |= f.readUint16LE();

	f.close();

	switch (ver) {
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
		cifTree = nullptr;
	}

	return cifTree;
}

class CifExporter {
public:
	virtual ~CifExporter() { };
	bool dump(const byte *data, uint32 size, const ResourceManager::CifInfo &info) const;

	static const CifExporter *create(uint32 version);

protected:
	virtual void writeCifInfo(Common::DumpFile &f, const ResourceManager::CifInfo &info) const = 0;
	virtual uint32 getVersion() const = 0;
	virtual void writeHeader(Common::DumpFile &f) const;
};

void CifExporter::writeHeader(Common::DumpFile &f) const {
	f.writeString("CIF FILE WayneSikes");
	f.writeByte(0);
	f.writeUint32LE(0);
	uint32 version = getVersion();
	f.writeUint16LE(version >> 16);
	f.writeUint16LE(version);
}

bool CifExporter::dump(const byte *data, uint32 size, const ResourceManager::CifInfo &info) const {
	Common::DumpFile f;
	if (!f.open(info.name + ".cif")) {
		warning("Failed to open export file '%s.cif'", info.name.c_str());
		return false;
	}

	writeHeader(f);
	writeCifInfo(f, info);
	f.write(data, size);

	if (f.err()) {
		warning("Error writing to export file '%s.cif'", info.name.c_str());
		f.close();
		return false;
	}

	f.close();
	return true;
}

class CifExporter20 : public CifExporter {
protected:
	void writeCifInfo(Common::DumpFile &f, const ResourceManager::CifInfo &info) const override;
	uint32 getVersion() const override { return 0x00020000; }
};

void CifExporter20::writeCifInfo(Common::DumpFile &f, const ResourceManager::CifInfo &info) const {
	f.writeUint16LE(info.width);
	f.writeUint16LE(info.pitch);
	f.writeUint16LE(info.height);
	f.writeByte(info.depth);

	f.writeByte(1);
	f.writeUint32LE(info.size);
	f.writeUint32LE(0);
	f.writeUint32LE(0);

	f.writeByte(info.type);
}

class CifExporter21 : public CifExporter20 {
protected:
	void writeCifInfo(Common::DumpFile &f, const ResourceManager::CifInfo &info) const override;
	uint32 getVersion() const override { return 0x00020001; }
};

void CifExporter21::writeCifInfo(Common::DumpFile &f, const ResourceManager::CifInfo &info) const {
	for (uint i = 0; i < 32; i++)
		f.writeByte(0); // TODO

	CifExporter20::writeCifInfo(f, info);
}

const CifExporter *CifExporter::create(uint32 version) {
	const CifExporter *exp;

	switch (version) {
	case 0x00020000:
		exp = new CifExporter20;
		break;
	case 0x00020001:
		exp = new CifExporter21;
		break;
	default:
		warning("Version %d.%d not supported by CifExporter", version >> 16, version & 0xffff);
		return nullptr;
	}

	return exp;
}

ResourceManager::ResourceManager() {
	_dec = new Decompressor;
}

ResourceManager::~ResourceManager() {
	for (uint i = 0; i < _cifTrees.size(); i++)
		delete _cifTrees[i];
	delete _dec;
}

bool ResourceManager::loadCifTree(const Common::String &name, const Common::String &ext) {
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
	return nullptr;
}

void ResourceManager::initialize() {
	if (g_nancy->getGameType() != kGameTypeVampire) {
		loadCifTree("ciftree", "dat");
	}
}

bool ResourceManager::getCifInfo(const Common::String &name, CifInfo &info) const {
	for (const auto &tree : _cifTrees) {
		if (getCifInfo(tree->getName(), name, info)) {
			return true;
		}
	}

	return false;
}

bool ResourceManager::getCifInfo(const Common::String &treeName, const Common::String &name, CifInfo &info) const {
	const CifFile *cifFile = CifFile::load(name);

	if (cifFile) {
		cifFile->getCifInfo(info);
		delete cifFile;
		return true;
	}

	const CifTree *cifTree = findCifTree(treeName);

	if (!cifTree)
		return false;

	return cifTree->getCifInfo(name, info);
}

byte *ResourceManager::getCifData(const Common::String &name, CifInfo &info, uint *size) const {
	// Try to open name.cif
	const CifFile *cifFile = CifFile::load(name);
	byte *buf = nullptr;

	// Look for cif inside cif tree
	if (cifFile) {
		buf = cifFile->getCifData(info, size);
		delete cifFile;
	} else {
		for (auto &tree : _cifTrees) {
			buf = tree->getCifData(name, info, size);
			if (buf) {
				break;
			}
		}
	}

	if (buf && info.comp == kResCompression) {
		Common::MemoryReadStream input(buf, info.compressedSize);
		byte *raw = new byte[info.size];
		Common::MemoryWriteStream output(raw, info.size);
		if (!_dec->decompress(input, output)) {
			warning("Failed to decompress '%s'", name.c_str());
			delete[] buf;
			delete[] raw;
			return nullptr;
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

bool ResourceManager::exportCif(const Common::String &treeName, const Common::String &name) {
	CifInfo info;
	uint size;
	byte *buf = getCifData(name, info, &size);

	if (!buf)
		return false;

	// Find out what CIF version this game uses
	uint32 version = 0;
	if (_cifTrees.size() > 0)
		version = _cifTrees[0]->getVersion();

	bool retval = false;
	const CifExporter *exp = CifExporter::create(version);
	if (exp) {
		retval = exp->dump(buf, size, info);
		delete exp;
	}
	return retval;
}

byte *ResourceManager::loadData(const Common::String &name, uint &size) {
	CifInfo info;
	byte *buf = getCifData(name, info, &size);

	if (!buf) {
		// Data was not found inside a cif tree or a cif file, try to open an .iff file
		// This is used by The Vampire Diaries
		Common::File f;
		if (f.open(name.hasSuffixIgnoreCase(".iff") ? name : name + ".iff")) {
			size = f.size();
			buf = new byte[size];
			f.read(buf, size);
		} else {
			return nullptr;
		}
	} else if (info.type != kResTypeScript) {
		warning("Resource '%s' is not a script", name.c_str());
		delete[] buf;
		return nullptr;
	}

	return buf;
}

bool ResourceManager::loadImage(const Common::String &name, Graphics::Surface &surf, const Common::String treeName, Common::Rect *outSrc, Common::Rect *outDest) {
	CifInfo info;
	surf.free();

	byte *buf = nullptr;

	if (treeName.size()) {
		buf = getCifData(treeName, name, info);
	} else {
		buf = getCifData(name, info);
	}

	if (!buf && treeName.size() > 0) {
		// Couldn't find image in a cif tree, try to open a .bmp file
		// This is used by The Vampire Diaries
		Common::File f;
		if (f.open(name + ".bmp")) {
			Image::BitmapDecoder dec;
			if (dec.loadStream(f)) {
				surf.copyFrom(*dec.getSurface());
				return true;
			} else {
				return false;
			}
		} else {
			return false;
		}
	} else {
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
	}

	if (outSrc) {
		*outSrc = info.src;
	}

	if (outDest) {
		*outDest = info.dest;
	}

	surf.w = info.width;
	surf.h = info.height;
	surf.pitch = info.pitch;
	surf.setPixels(buf);
	surf.format = g_nancy->_graphicsManager->getInputPixelFormat();
	return true;
}

bool ResourceManager::loadImage(const Common::String &name, Graphics::ManagedSurface &surf, const Common::String treeName, Common::Rect *outSrc, Common::Rect *outDest) {
	CifInfo info;
	bool loadedFromBitmapFile = false;
	surf.free();

	byte *buf = nullptr;

	if (treeName.size()) {
		buf = getCifData(treeName, name, info);
	} else {
		buf = getCifData(name, info);
	}

	if (!buf && treeName.size() > 0) {
		// Couldn't find image in a cif tree, try to open a .bmp file
		// This is used by The Vampire Diaries
		Common::File f;
		loadedFromBitmapFile = f.open(name + ".bmp");
		if (loadedFromBitmapFile) {
			Image::BitmapDecoder dec;
			if (dec.loadStream(f)) {
				GraphicsManager::copyToManaged(*dec.getSurface(), surf);
				surf.setPalette(dec.getPalette(), dec.getPaletteStartIndex(), MIN<uint>(256, dec.getPaletteColorCount())); // LOGO.BMP reports 257 colors
				return true;
			} else {
				return false;
			}
		} else {
			return false;
		}
	} else {
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

		if (outSrc) {
			*outSrc = info.src;
		}

		if (outDest) {
			*outDest = info.dest;
		}

		GraphicsManager::copyToManaged(buf, surf, info.width, info.height, g_nancy->_graphicsManager->getInputPixelFormat());
		return true;
	}
}

void ResourceManager::list(const Common::String &treeName, Common::Array<Common::String> &nameList, uint type) const {
	const CifTree *cifTree = findCifTree(treeName);

	if (!cifTree) {
		Common::ArchiveMemberList list;
		if (type == ResourceManager::kResTypeAny || type == ResourceManager::kResTypeImage) {
			SearchMan.listMatchingMembers(list, Common::Path("*.bmp"));
		}

		if (type == ResourceManager::kResTypeAny || type == ResourceManager::kResTypeScript) {
			SearchMan.listMatchingMembers(list, Common::Path("*.iff"));
		}

		for (auto &i : list) {
			nameList.push_back(i.get()->getDisplayName());
		}
	} else {
		cifTree->list(nameList, type);
	}

	Common::sort(nameList.begin(), nameList.end());
}

Common::String ResourceManager::getCifDescription(const Common::String &treeName, const Common::String &name) const {
	CifInfo info;
	if (!getCifInfo(treeName, name, info))
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
