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
 * $URL$
 * $Id$
 *
 */

#include "graphics/iff.h"

#include "common/config-manager.h"
#include "parallaction/parallaction.h"


namespace Parallaction {


struct Sprite {
	uint16	size;
	uint16	x;
	uint16	y;
	uint16	w;
	uint16	h;

	byte *packedData;

	Sprite() : size(0), x(0), y(0), w(0), h(0), packedData(0) {
	}

	~Sprite() {
		free(packedData);
	}
};

struct Sprites : public Frames {
	uint16		_num;
	Sprite*		_sprites;

	Sprites(uint num) {
		_num = num;
		_sprites = new Sprite[_num];
	}

	~Sprites() {
		delete[] _sprites;
	}

	uint16 getNum() {
		return _num;
	}

	byte* getData(uint16 index) {
		assert(index < _num);
		return _sprites[index].packedData;
	}

	void getRect(uint16 index, Common::Rect &r) {
		assert(index < _num);
		r.setWidth(_sprites[index].w);
		r.setHeight(_sprites[index].h);
		r.moveTo(_sprites[index].x, _sprites[index].y);
	}
	uint	getRawSize(uint16 index) {
		assert(index < _num);
		return _sprites[index].size;
	}
	uint	getSize(uint16 index) {
		assert(index < _num);
		return _sprites[index].w * _sprites[index].h;
	}


};

Common::SeekableReadStream *Disk_br::openFile_internal(bool errorOnNotFound, const Common::String &name, const Common::String &ext) {
	assert(!name.empty());
	debugC(5, kDebugDisk, "Disk_br::openFile(%s, %s)", name.c_str(), ext.c_str());

	Common::String lookup(name);

	if (!ext.empty() && !name.hasSuffix(ext.c_str())) {
		// make sure we are using the specified extension
		debugC(9, kDebugDisk, "Disk_br::openFile: appending explicit extension (%s) to (%s)", ext.c_str(), name.c_str());
		lookup = name + ext;
	}

	Common::SeekableReadStream *stream = _sset.openFile(lookup);
	if (!stream && errorOnNotFound) {
		errorFileNotFound(lookup);
	}
	return stream;
}

Common::SeekableReadStream *Disk_br::openFile(const Common::String &name, const Common::String &ext) {
	return openFile_internal(true, name, ext);
}

Common::SeekableReadStream *Disk_br::tryOpenFile(const Common::String &name, const Common::String &ext) {
	return openFile_internal(false, name, ext);
}


void Disk_br::errorFileNotFound(const Common::String &filename) {
	error("File '%s' not found", filename.c_str());
}

Common::String DosDisk_br::selectArchive(const Common::String& name) {
	debugC(5, kDebugDisk, "DosDisk_br::selectArchive");

	Common::String oldPath = _currentPart;
	_currentPart = name;

	debugC(5, kDebugDisk, "DosDisk_br::selectArchive: adding part directory to search set");
	_sset.remove("part");
	_sset.add("part", _baseDir->getSubDirectory(name, 3), 10);

	return oldPath;
}

void DosDisk_br::setLanguage(uint16 language) {
	debugC(5, kDebugDisk, "DosDisk_br::setLanguage");
	assert(language < 4);
	_language = language;
}

DosDisk_br::DosDisk_br(Parallaction* vm) : Disk_br(vm) {
}


void DosDisk_br::init() {
	// TODO: clarify whether the engine or OSystem should add the base game directory to the search manager.
	// Right now, I am keeping an internal search set to do the job.
	_baseDir = new Common::FSDirectory(ConfMan.get("path"));
	_sset.add("base", _baseDir, 5, false);
}


GfxObj* DosDisk_br::loadTalk(const char *name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadTalk(%s)", name);

	Common::SeekableReadStream *stream = openFile("tal/" + Common::String(name), ".tal");

	// talk position is set to (0,0), because talks are always displayed at
	// absolute coordinates, set in the dialogue manager. The original used
	// to null out coordinates every time they were needed. We do it better!
	Sprites *spr = createSprites(*stream);
	for (int i = 0; i < spr->getNum(); i++) {
		spr->_sprites[i].x = 0;
		spr->_sprites[i].y = 0;
	}

	delete stream;
	return new GfxObj(0, spr, name);
}

Script* DosDisk_br::loadLocation(const char *name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadLocation");

	static const Common::String langs[4] = { "it/", "fr/", "en/", "ge/" };

	Common::String fullName(name);
	if (!fullName.hasSuffix(".slf")) {
		fullName += ".loc";
	}

	Common::SeekableReadStream *stream = openFile(langs[_language] + fullName);
	return new Script(stream, true);
}

Script* DosDisk_br::loadScript(const char* name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadScript");
	Common::SeekableReadStream *stream = openFile("scripts/" + Common::String(name), ".scr");
	return new Script(stream, true);
}

//	there are no Head resources in Big Red Adventure
GfxObj* DosDisk_br::loadHead(const char* name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadHead");
	return 0;
}

void DosDisk_br::loadBitmap(Common::SeekableReadStream &stream, Graphics::Surface &surf, byte *palette) {
	stream.skip(4);
	uint width = stream.readUint32BE();
	if (width & 1) width++;
	uint height = stream.readUint32BE();
	stream.skip(20);

	if (palette) {
		stream.read(palette, 768);
	} else {
		stream.skip(768);
	}

	surf.create(width, height, 1);
	stream.read(surf.pixels, width * height);
}

Frames* DosDisk_br::loadPointer(const char *name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadPointer");
	Common::SeekableReadStream *stream = openFile(Common::String(name), ".ras");
	Graphics::Surface *surf = new Graphics::Surface;
	loadBitmap(*stream, *surf, 0);
	delete stream;
	return new SurfaceToFrames(surf);
}


Font* DosDisk_br::loadFont(const char* name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadFont");
	Common::SeekableReadStream *stream = openFile(name, ".fnt");
	Font *font = createFont(name, *stream);
	delete stream;
	return font;
}


GfxObj* DosDisk_br::loadObjects(const char *name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadObjects");
	Common::SeekableReadStream *stream = openFile(name);
	GfxObj *obj = createInventoryObjects(*stream);
	delete stream;
	return obj;
}

void genSlidePath(char *path, const char* name) {
	sprintf(path, "%s.bmp", name);
}

GfxObj* DosDisk_br::loadStatic(const char* name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadStatic");
	Common::SeekableReadStream *stream = openFile("ras/" + Common::String(name), ".ras");
	Graphics::Surface *surf = new Graphics::Surface;
	loadBitmap(*stream, *surf, 0);
	delete stream;
	return new GfxObj(0, new SurfaceToFrames(surf), name);
}

Sprites* DosDisk_br::createSprites(Common::ReadStream &stream) {

	uint16 num = stream.readUint16LE();

	Sprites *sprites = new Sprites(num);

	for (uint i = 0; i < num; i++) {
		Sprite *spr = &sprites->_sprites[i];
		spr->size = stream.readUint16LE();
		spr->x = stream.readUint16LE();
		spr->y = stream.readUint16LE();
		spr->w = stream.readUint16LE();
		spr->h = stream.readUint16LE();

		spr->packedData = (byte*)malloc(spr->size);
		stream.read(spr->packedData, spr->size);
	}

	return sprites;
}

Frames* DosDisk_br::loadFrames(const char* name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadFrames");
	Common::SeekableReadStream *stream = openFile("ani/" + Common::String(name), ".ani");
	Frames *frames = createSprites(*stream);
	delete stream;
	return frames;
}

// Slides in Nippon Safes are basically screen-sized pictures with valid
// palette data used for menu and for location switches. Big Red Adventure
// doesn't need slides in that sense, but it still has some special
// graphics resources with palette data, so those will be named slides.
//
void DosDisk_br::loadSlide(BackgroundInfo& info, const char *name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadSlide");

	Common::SeekableReadStream *stream = openFile(name, ".bmp");

	byte rgb[768];

	loadBitmap(*stream, info.bg, rgb);
	info.width = info.bg.w;
	info.height = info.bg.h;

	delete stream;

	for (uint i = 0; i < 256; i++) {
		info.palette.setEntry(i, rgb[i] >> 2, rgb[i+256] >> 2, rgb[i+512] >> 2);
	}
}

void DosDisk_br::loadMask(const char *name, MaskBuffer &buffer) {
	if (!name) {
		return;
	}

	Common::SeekableReadStream *stream = openFile("msk/" + Common::String(name), ".msk");

	// NOTE: info.width and info.height are only valid if the background graphics
	// have already been loaded
	buffer.bigEndian = false;
	stream->read(buffer.data, buffer.size);
	delete stream;
}

void DosDisk_br::loadScenery(BackgroundInfo& info, const char *name, const char *mask, const char* path) {
	debugC(5, kDebugDisk, "DosDisk_br::loadScenery");

	Common::SeekableReadStream *stream;

	if (name) {
		stream = openFile("bkg/" + Common::String(name), ".bkg");
		byte rgb[768];

		loadBitmap(*stream, info.bg, rgb);
		info.width = info.bg.w;
		info.height = info.bg.h;

		for (uint i = 0; i < 256; i++) {
			info.palette.setEntry(i, rgb[i] >> 2, rgb[i+256] >> 2, rgb[i+512] >> 2);
		}

		delete stream;
	}

	if (mask) {
		info.mask.create(info.width, info.height);
		loadMask(mask, info.mask);
	}

	if (path) {
		stream = openFile("pth/" + Common::String(path), ".pth");

		// NOTE: info.width and info.height are only valid if the background graphics
		// have already been loaded
		info.path.create(info.width, info.height);
		stream->read(info.path.data, info.path.size);
		delete stream;
	}

	return;
}

Table* DosDisk_br::loadTable(const char* name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadTable");
	Common::SeekableReadStream *stream = openFile(name, ".tab");
	Table *t = createTableFromStream(100, *stream);
	delete stream;
	return t;
}

Common::SeekableReadStream* DosDisk_br::loadMusic(const char* name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadMusic");
	return 0;
}


Common::ReadStream* DosDisk_br::loadSound(const char* name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadSound");
	return 0;
}




DosDemoDisk_br::DosDemoDisk_br(Parallaction *vm) : DosDisk_br(vm) {

}

void DosDemoDisk_br::init() {
	// TODO: clarify whether the engine or OSystem should add the base game directory to the search manager.
	// Right now, I am keeping an internal search set to do the job.
	_baseDir = new Common::FSDirectory(ConfMan.get("path"), 2);
	_sset.add("base", _baseDir, 5, false);
}


Common::String DosDemoDisk_br::selectArchive(const Common::String& name) {
	debugC(5, kDebugDisk, "DosDemoDisk_br::selectArchive");
	Common::String oldPath = _currentPart;
	_currentPart = name;
	return oldPath;
}


AmigaDisk_br::AmigaDisk_br(Parallaction *vm) : DosDisk_br(vm) {
}

void AmigaDisk_br::init() {
	_baseDir = new Common::FSDirectory(ConfMan.get("path"));
	_sset.add("base", _baseDir, 5, false);

	const Common::String subDirNames[3] = { "fonts", "backs", "common" };
	const Common::String subDirPrefixes[3] = { "fonts", "backs", Common::String::emptyString };
	for (int i = 0; i < 3; i++)
		_sset.add(subDirNames[i], _baseDir->getSubDirectory(subDirPrefixes[i], subDirNames[i], 2), 6);
}

void AmigaDisk_br::loadBackground(BackgroundInfo& info, Common::SeekableReadStream &stream) {

	byte *pal;

	Graphics::ILBMDecoder decoder(stream, info.bg, pal);
	decoder.decode();

	uint i;

	info.width = info.bg.w;
	info.height = info.bg.h;

	byte *p = pal;
	for (i = 0; i < 16; i++) {
		byte r = *p >> 2;
		p++;
		byte g = *p >> 2;
		p++;
		byte b = *p >> 2;
		p++;
		info.palette.setEntry(i, r, g, b);
	}

	free(pal);

	return;
}


void AmigaDisk_br::loadScenery(BackgroundInfo& info, const char* name, const char* mask, const char* path) {
	debugC(1, kDebugDisk, "AmigaDisk_br::loadScenery '%s', '%s' '%s'", name, mask, path);

	Common::SeekableReadStream *stream;

	if (name) {
		stream = openFile("backs/" + Common::String(name), ".bkg");
		loadBackground(info, *stream);
		delete stream;
	}
#if 0
	if (mask && _mskDir.exists()) {
		filepath = Common::String(mask) + ".msk";
		node = _mskDir.getChild(filepath);
		if (!node.exists()) {
			filepath = Common::String(mask) + ".msk";
			node = _commonMskDir.getChild(filepath);
		}

		if (node.exists()) {
			stream.open(node);
			stream.seek(0x30, SEEK_SET);
			Graphics::PackBitsReadStream unpackedStream(stream);
			info.mask.create(info.width, info.height);
			unpackedStream.read(info.mask.data, info.mask.size);
			// TODO: there is another step to do after decompression...
			loadMask(info, stream);
			stream.close();
		}
	}
#endif
	if (path) {
		stream = tryOpenFile("pth/" + Common::String(path), ".pth");
		if (stream) {
			// NOTE: info.width and info.height are only valid if the background graphics
			// have already been loaded
			info.path.create(info.width, info.height);
			stream->read(info.path.data, info.path.size);
			delete stream;
		} else {
			debugC(1, kDebugDisk, "AmigaDisk_br::loadScenery: (%s) not found", path);
		}
	}

	return;
}

void AmigaDisk_br::loadSlide(BackgroundInfo& info, const char *name) {
	debugC(1, kDebugDisk, "AmigaDisk_br::loadSlide '%s'", name);

	Common::SeekableReadStream *stream = openFile("backs/" + Common::String(name), ".bkg");
	loadBackground(info, *stream);
	delete stream;
}

GfxObj* AmigaDisk_br::loadStatic(const char* name) {
	debugC(1, kDebugDisk, "AmigaDisk_br::loadStatic '%s'", name);

	Common::SeekableReadStream *stream = openFile("ras/" + Common::String(name), ".ras");

	byte *pal = 0;
	Graphics::Surface* surf = new Graphics::Surface;

	Graphics::ILBMDecoder decoder(*stream, *surf, pal);
	decoder.decode();

	free(pal);
	delete stream;

	return new GfxObj(0, new SurfaceToFrames(surf));
}

Sprites* AmigaDisk_br::createSprites(Common::ReadStream &stream) {
	uint16 num = stream.readUint16BE();

	Sprites *sprites = new Sprites(num);

	for (uint i = 0; i < num; i++) {
		Sprite *spr = &sprites->_sprites[i];
		spr->size = stream.readUint16BE();
		spr->x = stream.readUint16BE();
		spr->y = stream.readUint16BE();
		spr->w = stream.readUint16BE();
		spr->h = stream.readUint16BE();

		// TODO: Convert image format
		spr->packedData = (byte*)malloc(spr->size);
		stream.read(spr->packedData, spr->size);
	}

	return sprites;
}

Frames* AmigaDisk_br::loadFrames(const char* name) {
	debugC(1, kDebugDisk, "AmigaDisk_br::loadFrames '%s'", name);
	Common::SeekableReadStream *stream = openFile("anims/" + Common::String(name), ".ani");
	Frames *frames = createSprites(*stream);
	delete stream;
	return frames;
}

GfxObj* AmigaDisk_br::loadTalk(const char *name) {
	debugC(1, kDebugDisk, "AmigaDisk_br::loadTalk '%s'", name);
	Common::SeekableReadStream *stream = openFile("talks/" + Common::String(name), ".tal");
	GfxObj* obj = new GfxObj(0, createSprites(*stream));
	delete stream;
	return obj;
}

Font* AmigaDisk_br::loadFont(const char* name) {
	debugC(1, kDebugDisk, "AmigaFullDisk::loadFont '%s'", name);

	Common::SeekableReadStream *stream = openFile("fonts/" + Common::String(name), ".font");

	Common::String fontDir;
	Common::String fontFile;
	byte ch;

	stream->seek(4, SEEK_SET);
	while ((ch = stream->readByte()) != 0x2F) fontDir += ch;
	while ((ch = stream->readByte()) != 0) fontFile += ch;
	delete stream;

	stream = openFile("fonts/" + fontDir + "/" + fontFile);
	Font *font = createFont(name, *stream);

	delete stream;
	return font;
}

Common::SeekableReadStream* AmigaDisk_br::loadMusic(const char* name) {
	debugC(5, kDebugDisk, "AmigaDisk_br::loadMusic");
	return openFile("msc/" + Common::String(name), ".msc");
}


Common::ReadStream* AmigaDisk_br::loadSound(const char* name) {
	debugC(5, kDebugDisk, "AmigaDisk_br::loadSound");
	return openFile("sfx/" + Common::String(name), ".sfx");
}

GfxObj* AmigaDisk_br::loadObjects(const char *name) {
	debugC(5, kDebugDisk, "AmigaDisk_br::loadObjects");

	Common::SeekableReadStream *stream = openFile(name);

	byte *pal = 0;
	Graphics::Surface* surf = new Graphics::Surface;

	Graphics::ILBMDecoder decoder(*stream, *surf, pal);
	decoder.decode();

	delete stream;
	free(pal);

	return new GfxObj(0, new SurfaceToFrames(surf));
}

Common::String AmigaDisk_br::selectArchive(const Common::String& name) {
	debugC(5, kDebugDisk, "AmigaDisk_br::selectArchive");

	Common::String oldPath = _currentPart;
	_currentPart = name;

	debugC(5, kDebugDisk, "AmigaDisk_br::selectArchive: adding part directory to search set");
	_sset.remove("part");
	_sset.add("part", _baseDir->getSubDirectory(name, 3), 10);

	return oldPath;
}


Disk_br::Disk_br(Parallaction *vm) : _vm(vm), _baseDir(0) {

}

Disk_br::~Disk_br() {
	delete _baseDir;
	_sset.clear();
}



} // namespace Parallaction
