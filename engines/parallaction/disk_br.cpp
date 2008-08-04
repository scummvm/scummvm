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



void DosDisk_br::errorFileNotFound(const FilesystemNode &dir, const Common::String &filename) {
	error("File '%s' not found in directory '%s'", filename.c_str(), dir.getDisplayName().c_str());
}

Common::String DosDisk_br::selectArchive(const Common::String& name) {
	debugC(5, kDebugDisk, "DosDisk_br::selectArchive");

	Common::String oldPath;
	if (_partDir.exists()) {
		oldPath = _partDir.getDisplayName();
	}

	_partDir = _baseDir.getChild(name);

	_aniDir = _partDir.getChild("ani");
	_bkgDir = _partDir.getChild("bkg");
	_mscDir = _partDir.getChild("msc");
	_mskDir = _partDir.getChild("msk");
	_pthDir = _partDir.getChild("pth");
	_rasDir = _partDir.getChild("ras");
	_scrDir = _partDir.getChild("scripts");
	_sfxDir = _partDir.getChild("sfx");
	_talDir = _partDir.getChild("tal");

	return oldPath;
}

void DosDisk_br::setLanguage(uint16 language) {
	debugC(5, kDebugDisk, "DosDisk_br::setLanguage");
	assert(language < 4);
	_language = language;
}

DosDisk_br::DosDisk_br(Parallaction* vm) : _vm(vm), _baseDir(ConfMan.get("path")) {
}

DosDisk_br::~DosDisk_br() {
}

GfxObj* DosDisk_br::loadTalk(const char *name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadTalk(%s)", name);

	Common::String path(name);
	FilesystemNode node = _talDir.getChild(path);
	if (!node.exists()) {
		path += ".tal";
		node = _talDir.getChild(path);
		if (!node.exists())
			errorFileNotFound(_talDir, path);
	}

	Common::File stream;
	stream.open(node);

	// talk position is set to (0,0), because talks are always displayed at
	// absolute coordinates, set in the dialogue manager. The original used
	// to null out coordinates every time they were needed. We do it better!
	Sprites *spr = createSprites(stream);
	for (int i = 0; i < spr->getNum(); i++) {
		spr->_sprites[i].x = 0;
		spr->_sprites[i].y = 0;
	}
	return new GfxObj(0, spr, name);
}

Script* DosDisk_br::loadLocation(const char *name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadLocation");

	Common::String langs[4] = { "it", "fr", "en", "ge" };
	FilesystemNode locDir = _partDir.getChild(langs[_language]);

	Common::String path(name);
	path += ".slf";
	FilesystemNode node = locDir.getChild(path);
	if (!node.exists()) {
		path = Common::String(name) + ".loc";
		node = locDir.getChild(path);
		if (!node.exists()) {
			errorFileNotFound(locDir, path);
		}
	}

	Common::File *stream = new Common::File;
	stream->open(node);
	return new Script(stream, true);
}

Script* DosDisk_br::loadScript(const char* name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadScript");

	Common::String path(name);
	path += ".scr";
	FilesystemNode node = _scrDir.getChild(path);
	if (!node.exists()) {
		errorFileNotFound(_scrDir, path);
	}

	Common::File *stream = new Common::File;
	stream->open(node);
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

	Common::String path(name);
	path += ".ras";
	FilesystemNode node = _baseDir.getChild(path);
	if (!node.exists()) {
		errorFileNotFound(_baseDir, path);
	}

	Common::File stream;
	stream.open(node);

	Graphics::Surface *surf = new Graphics::Surface;
	loadBitmap(stream, *surf, 0);
	return new SurfaceToFrames(surf);
}


Font* DosDisk_br::loadFont(const char* name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadFont");

	Common::String path(name);
	path += ".fnt";
	FilesystemNode node = _baseDir.getChild(path);
	if (!node.exists()) {
		errorFileNotFound(_baseDir, path);
	}

	Common::File stream;
	stream.open(node);
	return createFont(name, stream);
}


GfxObj* DosDisk_br::loadObjects(const char *name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadObjects");

	Common::String path(name);
	FilesystemNode node = _partDir.getChild(path);
	if (!node.exists()) {
		errorFileNotFound(_partDir, path);
	}

	Common::File stream;
	stream.open(node);

	return createInventoryObjects(stream);
}

void genSlidePath(char *path, const char* name) {
	sprintf(path, "%s.bmp", name);
}

GfxObj* DosDisk_br::loadStatic(const char* name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadStatic");

	Common::String path(name);
	FilesystemNode node = _rasDir.getChild(path);
	if (!node.exists()) {
		errorFileNotFound(_rasDir, path);
	}

	Common::File stream;
	stream.open(node);

	Graphics::Surface *surf = new Graphics::Surface;
	loadBitmap(stream, *surf, 0);
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

	Common::String path(name);
	FilesystemNode node = _aniDir.getChild(path);
	if (!node.exists()) {
		path += ".ani";
		node = _aniDir.getChild(path);
		if (!node.exists()) {
			errorFileNotFound(_aniDir, path);
		}
	}

	Common::File stream;
	stream.open(node);
	return createSprites(stream);
}

// Slides in Nippon Safes are basically screen-sized pictures with valid
// palette data used for menu and for location switches. Big Red Adventure
// doesn't need slides in that sense, but it still has some special
// graphics resources with palette data, so those will be named slides.
//
void DosDisk_br::loadSlide(BackgroundInfo& info, const char *name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadSlide");

	Common::String path(name);
	path += ".bmp";
	FilesystemNode node = _baseDir.getChild(path);
	if (!node.exists()) {
		errorFileNotFound(_baseDir, path);
	}

	Common::File stream;
	stream.open(node);

	byte rgb[768];

	loadBitmap(stream, info.bg, rgb);
	info.width = info.bg.w;
	info.height = info.bg.h;

	for (uint i = 0; i < 256; i++) {
		info.palette.setEntry(i, rgb[i] >> 2, rgb[i+256] >> 2, rgb[i+512] >> 2);
	}

	return;
}

void DosDisk_br::loadScenery(BackgroundInfo& info, const char *name, const char *mask, const char* path) {
	debugC(5, kDebugDisk, "DosDisk_br::loadScenery");

	Common::String filepath;
	FilesystemNode node;
	Common::File stream;

	if (name) {
		filepath = Common::String(name) + ".bkg";
		node = _bkgDir.getChild(filepath);
		if (!node.exists()) {
			errorFileNotFound(_bkgDir, filepath);
		}
		stream.open(node);

		byte rgb[768];

		loadBitmap(stream, info.bg, rgb);
		info.width = info.bg.w;
		info.height = info.bg.h;

		for (uint i = 0; i < 256; i++) {
			info.palette.setEntry(i, rgb[i] >> 2, rgb[i+256] >> 2, rgb[i+512] >> 2);
		}

		stream.close();
	}

	if (mask) {
		filepath = Common::String(mask) + ".msk";
		node = _mskDir.getChild(filepath);
		if (!node.exists()) {
			errorFileNotFound(_mskDir, filepath);
		}
		stream.open(node);

		// NOTE: info.width and info.height are only valid if the background graphics
		// have already been loaded
		info.mask.create(info.width, info.height);
		info.mask.bigEndian = false;
		stream.read(info.mask.data, info.mask.size);
		stream.close();
	}

	if (path) {
		filepath = Common::String(path) + ".pth";
		node = _pthDir.getChild(filepath);
		if (!node.exists()) {
			errorFileNotFound(_pthDir, filepath);
		}
		stream.open(node);

		// NOTE: info.width and info.height are only valid if the background graphics
		// have already been loaded
		info.path.create(info.width, info.height);
		stream.read(info.path.data, info.path.size);
		stream.close();
	}

	return;
}

Table* DosDisk_br::loadTable(const char* name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadTable");

	Common::String path(name);
	path += ".tab";
	FilesystemNode node = _partDir.getChild(path);
	if (!node.exists()) {
		errorFileNotFound(_partDir, path);
	}

	Common::File stream;
	stream.open(node);
	Table *t = createTableFromStream(100, stream);
	stream.close();

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






DosDemo_br::DosDemo_br(Parallaction *vm) : DosDisk_br(vm) {

}


DosDemo_br::~DosDemo_br() {

}

Common::String DosDemo_br::selectArchive(const Common::String& name) {
	debugC(5, kDebugDisk, "DosDemo_br::selectArchive");

	Common::String oldPath;
	if (_partDir.exists()) {
		oldPath = _partDir.getDisplayName();
	}

	_partDir = _baseDir;

	_aniDir = _partDir.getChild("ani");
	_bkgDir = _partDir.getChild("bkg");
	_mscDir = _partDir.getChild("msc");
	_mskDir = _partDir.getChild("msk");
	_pthDir = _partDir.getChild("pth");
	_rasDir = _partDir.getChild("ras");
	_scrDir = _partDir.getChild("scripts");
	_sfxDir = _partDir.getChild("sfx");
	_talDir = _partDir.getChild("tal");

	return oldPath;
}






AmigaDisk_br::AmigaDisk_br(Parallaction *vm) : DosDisk_br(vm) {
	_fntDir = _baseDir.getChild("fonts");

	_baseBkgDir = _baseDir.getChild("backs");

	FilesystemNode commonDir = _baseDir.getChild("common");
	_commonAniDir = commonDir.getChild("anims");
	_commonBkgDir = commonDir.getChild("backs");
	_commonMscDir = commonDir.getChild("msc");
	_commonMskDir = commonDir.getChild("msk");
	_commonPthDir = commonDir.getChild("pth");
	_commonTalDir = commonDir.getChild("talks");
}


AmigaDisk_br::~AmigaDisk_br() {

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

	Common::String filepath;
	FilesystemNode node;
	Common::File stream;

	if (name) {
		filepath = Common::String(name) + ".bkg";
		node = _bkgDir.getChild(filepath);
		if (!node.exists()) {
			filepath = Common::String(name) + ".bkg";
			node = _commonBkgDir.getChild(filepath);
			if (!node.exists()) {
				errorFileNotFound(_bkgDir, filepath);
			}
		}
		stream.open(node);
		loadBackground(info, stream);
		stream.close();
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
	if (path && _pthDir.exists()) {
		filepath = Common::String(path) + ".pth";
		node = _pthDir.getChild(filepath);
		if (!node.exists()) {
			filepath = Common::String(path) + ".pth";
			node = _commonPthDir.getChild(filepath);
			if (!node.exists()) {
				errorFileNotFound(_pthDir, filepath);
			}
		}
		stream.open(node);
		// NOTE: info.width and info.height are only valid if the background graphics
		// have already been loaded
		info.path.create(info.width, info.height);
		stream.read(info.path.data, info.path.size);
		stream.close();
	}

	return;
}

void AmigaDisk_br::loadSlide(BackgroundInfo& info, const char *name) {
	debugC(1, kDebugDisk, "AmigaDisk_br::loadSlide '%s'", name);

	Common::String path(name);
	path += ".bkg";
	FilesystemNode node = _baseBkgDir.getChild(path);
	if (!node.exists()) {
		errorFileNotFound(_baseBkgDir, path);
	}
	Common::File stream;
	stream.open(node);
	loadBackground(info, stream);
	return;
}

GfxObj* AmigaDisk_br::loadStatic(const char* name) {
	debugC(1, kDebugDisk, "AmigaDisk_br::loadStatic '%s'", name);

	Common::String path(name);
	FilesystemNode node = _rasDir.getChild(path);
	if (!node.exists()) {
		errorFileNotFound(_rasDir, path);
	}
	Common::File stream;
	stream.open(node);

	byte *pal = 0;
	Graphics::Surface* surf = new Graphics::Surface;

	Graphics::ILBMDecoder decoder(stream, *surf, pal);
	decoder.decode();

	free(pal);

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

	Common::String path(name);
	FilesystemNode node = _aniDir.getChild(path);
	if (!node.exists()) {
		path += ".ani";
		node = _aniDir.getChild(path);
		if (!node.exists()) {
			path = Common::String(name);
			node = _commonAniDir.getChild(path);
			if (!node.exists()) {
				path += ".ani";
				node = _commonAniDir.getChild(path);
				if (!node.exists()) {
					errorFileNotFound(_aniDir, path);
				}
			}
		}
	}

	Common::File stream;
	stream.open(node);
	return createSprites(stream);
}

GfxObj* AmigaDisk_br::loadTalk(const char *name) {
	debugC(1, kDebugDisk, "AmigaDisk_br::loadTalk '%s'", name);

	Common::String path(name);
	FilesystemNode node = _talDir.getChild(path);
	if (!node.exists()) {
		path += ".tal";
		node = _talDir.getChild(path);
		if (!node.exists()) {
			path = Common::String(name);
			node = _commonTalDir.getChild(path);
			if (!node.exists()) {
				path += ".tal";
				node = _commonTalDir.getChild(path);
				if (!node.exists()) {
					errorFileNotFound(_talDir, path);
				}
			}
		}
	}

	Common::File stream;
	stream.open(node);
	return new GfxObj(0, createSprites(stream));
}

Font* AmigaDisk_br::loadFont(const char* name) {
	debugC(1, kDebugDisk, "AmigaFullDisk::loadFont '%s'", name);

	Common::String path(name);
	path += ".font";
	FilesystemNode node = _fntDir.getChild(path);
	if (!node.exists()) {
		errorFileNotFound(_fntDir, path);
	}

	Common::String fontDir;
	Common::String fontFile;
	byte ch;

	Common::File stream;
	stream.open(node);
	stream.seek(4, SEEK_SET);
	while ((ch = stream.readByte()) != 0x2F) fontDir += ch;
	while ((ch = stream.readByte()) != 0) fontFile += ch;
	stream.close();

	node = _fntDir.getChild(fontDir);
	if (!node.exists()) {
		errorFileNotFound(_fntDir, fontDir);
	}
	node = node.getChild(fontFile);
	if (!node.exists()) {
		errorFileNotFound(node, fontFile);
	}

	stream.open(node);
	return createFont(name, stream);
}

Common::SeekableReadStream* AmigaDisk_br::loadMusic(const char* name) {
	debugC(5, kDebugDisk, "AmigaDisk_br::loadMusic");

	Common::String path(name);
	FilesystemNode node = _mscDir.getChild(path);
	if (!node.exists()) {
		// TODO (Kirben): error out when music file is not found?
		return 0;
	}

	Common::File *stream = new Common::File;
	stream->open(node);
	return stream;
}


Common::ReadStream* AmigaDisk_br::loadSound(const char* name) {
	debugC(5, kDebugDisk, "AmigaDisk_br::loadSound");

	Common::String path(name);
	FilesystemNode node = _sfxDir.getChild(path);
	if (!node.exists()) {
		errorFileNotFound(_sfxDir, path);
	}

	Common::File *stream = new Common::File;
	stream->open(node);
	return stream;
}

GfxObj* AmigaDisk_br::loadObjects(const char *name) {
	debugC(5, kDebugDisk, "AmigaDisk_br::loadObjects");

	Common::String path(name);
	FilesystemNode node = _partDir.getChild(path);
	if (!node.exists()) {
		errorFileNotFound(_partDir, path);
	}

	Common::File stream;
	stream.open(node);

	byte *pal = 0;
	Graphics::Surface* surf = new Graphics::Surface;

	Graphics::ILBMDecoder decoder(stream, *surf, pal);
	decoder.decode();

	free(pal);

	return new GfxObj(0, new SurfaceToFrames(surf));
}

Common::String AmigaDisk_br::selectArchive(const Common::String& name) {
	debugC(5, kDebugDisk, "AmigaDisk_br::selectArchive");

	Common::String oldPath;
	if (_partDir.exists()) {
		oldPath = _partDir.getDisplayName();
	}

	_partDir = _baseDir.getChild(name);

	_aniDir = _partDir.getChild("anims");
	_bkgDir = _partDir.getChild("backs");
	_mscDir = _partDir.getChild("msc");
	_mskDir = _partDir.getChild("msk");
	_pthDir = _partDir.getChild("pth");
	_rasDir = _partDir.getChild("ras");
	_scrDir = _partDir.getChild("scripts");
	_sfxDir = _partDir.getChild("sfx");
	_talDir = _partDir.getChild("talks");

	return oldPath;
}

} // namespace Parallaction
