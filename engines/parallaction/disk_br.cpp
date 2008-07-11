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



void DosDisk_br::errorFileNotFound(const char *s) {
	error("File '%s' not found", s);
}

Common::String DosDisk_br::selectArchive(const Common::String& name) {
	debugC(5, kDebugDisk, "DosDisk_br::selectArchive");

	Common::String oldPath(_partPath);
	strcpy(_partPath, name.c_str());

	return oldPath;
}

void DosDisk_br::setLanguage(uint16 language) {
	debugC(5, kDebugDisk, "DosDisk_br::setLanguage");

	switch (language) {
	case 0:
		strcpy(_languageDir, "it");
		break;

	case 1:
		strcpy(_languageDir, "fr");
		break;

	case 2:
		strcpy(_languageDir, "en");
		break;

	case 3:
		strcpy(_languageDir, "ge");
		break;

	default:
		error("unknown language");

	}

	return;
}

DosDisk_br::DosDisk_br(Parallaction* vm) : _vm(vm) {

}

DosDisk_br::~DosDisk_br() {
}

GfxObj* DosDisk_br::loadTalk(const char *name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadTalk(%s)", name);

	Common::File stream;

	char path[PATH_LEN];
	sprintf(path, "%s/tal/%s", _partPath, name);
	if (!stream.open(path)) {
		sprintf(path, "%s/tal/%s.tal", _partPath, name);
		if (!stream.open(path))
			errorFileNotFound(path);
	}

	return new GfxObj(0, createSprites(stream), name);
}

Script* DosDisk_br::loadLocation(const char *name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadLocation");

	Common::File *stream = new Common::File;

	char path[PATH_LEN];
	sprintf(path, "%s/%s/%s.slf", _partPath, _languageDir, name);
	if (!stream->open(path)) {
		sprintf(path, "%s/%s/%s.loc", _partPath, _languageDir, name);
		if (!stream->open(path))
			errorFileNotFound(path);
	}

	return new Script(stream, true);
}

Script* DosDisk_br::loadScript(const char* name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadScript");

	Common::File *stream = new Common::File;

	char path[PATH_LEN];
	sprintf(path, "%s/scripts/%s.scr", _partPath, name);
	if (!stream->open(path))
		errorFileNotFound(path);

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

	char path[PATH_LEN];
	sprintf(path, "%s.ras", name);

	Common::File stream;
	if (!stream.open(path))
		errorFileNotFound(path);

	Graphics::Surface *surf = new Graphics::Surface;
	loadBitmap(stream, *surf, 0);
	return new SurfaceToFrames(surf);
}


Font* DosDisk_br::loadFont(const char* name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadFont");

	char path[PATH_LEN];
	sprintf(path, "%s.fnt", name);

	Common::File stream;
	if (!stream.open(path))
		errorFileNotFound(path);

	return createFont(name, stream);
}


GfxObj* DosDisk_br::loadObjects(const char *name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadObjects");
	return 0;
}

void genSlidePath(char *path, const char* name) {
	sprintf(path, "%s.bmp", name);
}

GfxObj* DosDisk_br::loadStatic(const char* name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadStatic");

	char path[PATH_LEN];
	sprintf(path, "%s/ras/%s", _partPath, name);
	Common::File stream;
	if (!stream.open(path)) {
		errorFileNotFound(path);
	}

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

	char path[PATH_LEN];
	sprintf(path, "%s/ani/%s", _partPath, name);

	Common::File stream;
	if (!stream.open(path)) {
		sprintf(path, "%s/ani/%s.ani", _partPath, name);
		if (!stream.open(path)) {
			errorFileNotFound(path);
		}
	}

	return createSprites(stream);
}

// Slides in Nippon Safes are basically screen-sized pictures with valid
// palette data used for menu and for location switches. Big Red Adventure
// doesn't need slides in that sense, but it still has some special
// graphics resources with palette data, so those will be named slides.
//
void DosDisk_br::loadSlide(BackgroundInfo& info, const char *name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadSlide");

	char path[PATH_LEN];
	genSlidePath(path, name);

	Common::File stream;
	if (!stream.open(path))
		errorFileNotFound(path);

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

	char filename[PATH_LEN];
	Common::File stream;

	if (name) {
		sprintf(filename, "%s/bkg/%s.bkg", _partPath, name);
		if (!stream.open(filename))
			errorFileNotFound(filename);

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
		sprintf(filename, "%s/msk/%s.msk", _partPath, mask);
		if (!stream.open(filename))
			errorFileNotFound(filename);

		// NOTE: info.width and info.height are only valid if the background graphics
		// have already been loaded
		info.mask.create(info.width, info.height);
		info.mask.bigEndian = false;
		stream.read(info.mask.data, info.mask.size);
		stream.close();
	}

	if (path) {
		sprintf(filename, "%s/pth/%s.pth", _partPath, path);
		if (!stream.open(filename))
			errorFileNotFound(filename);

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

	char path[PATH_LEN];
	sprintf(path, "%s/%s.tab", _partPath, name);

	Common::File	stream;
	if (!stream.open(path))
		errorFileNotFound(path);

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






AmigaDisk_br::AmigaDisk_br(Parallaction *vm) : DosDisk_br(vm) {

}


AmigaDisk_br::~AmigaDisk_br() {

}


/*
	FIXME: mask values are not computed correctly for level 1 and 2

	NOTE: this routine is only able to build masks for Nippon Safes, since mask widths are hardcoded
	into the main loop.
*/
void buildMask2(byte* buf) {

	byte mask1[16] = { 0, 0x80, 0x20, 0xA0, 8, 0x88, 0x28, 0xA8, 2, 0x82, 0x22, 0xA2, 0xA, 0x8A, 0x2A, 0xAA };
	byte mask0[16] = { 0, 0x40, 0x10, 0x50, 4, 0x44, 0x14, 0x54, 1, 0x41, 0x11, 0x51, 0x5, 0x45, 0x15, 0x55 };

	byte plane0[40];
	byte plane1[40];

	for (int32 i = 0; i < _vm->_screenHeight; i++) {

		memcpy(plane0, buf, 40);
		memcpy(plane1, buf+40, 40);

		for (uint32 j = 0; j < 40; j++) {
			*buf++ = mask0[(plane0[j] & 0xF0) >> 4] | mask1[(plane1[j] & 0xF0) >> 4];
			*buf++ = mask0[plane0[j] & 0xF] | mask1[plane1[j] & 0xF];
		}

	}
}

void AmigaDisk_br::loadBackground(BackgroundInfo& info, const char *name) {

	char path[PATH_LEN];
	sprintf(path, "%s", name);

	Common::File s;

	if (!s.open(path))
		errorFileNotFound(path);

	byte *pal;

	Graphics::ILBMDecoder decoder(s, info.bg, pal);
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

void AmigaDisk_br::loadMask(BackgroundInfo& info, const char *name) {
	debugC(5, kDebugDisk, "AmigaDisk_br::loadMask(%s)", name);

	Common::File s;

	if (!s.open(name))
		return;

	s.seek(0x30, SEEK_SET);

	byte r, g, b;
	for (uint i = 0; i < 4; i++) {
		r = s.readByte();
		g = s.readByte();
		b = s.readByte();

		info.layers[i] = (((r << 4) & 0xF00) | (g & 0xF0) | (b >> 4)) & 0xFF;
	}

	s.seek(0x126, SEEK_SET);	// HACK: skipping IFF/ILBM header should be done by analysis, not magic
	Graphics::PackBitsReadStream stream(s);

	info.mask.create(info.width, info.height);
	stream.read(info.mask.data, info.mask.size);
	buildMask2(info.mask.data);

	return;
}

void AmigaDisk_br::loadScenery(BackgroundInfo& info, const char* name, const char* mask, const char* path) {
	debugC(1, kDebugDisk, "AmigaDisk_br::loadScenery '%s', '%s' '%s'", name, mask, path);

	char filename[PATH_LEN];
	Common::File stream;

	if (name) {
		sprintf(filename, "%s/backs/%s.bkg", _partPath, name);

		loadBackground(info, filename);
	}

	if (mask) {
		sprintf(filename, "%s/msk/%s.msk", _partPath, name);

		loadMask(info, filename);
	}

	if (path) {
		sprintf(filename, "%s/pth/%s.pth", _partPath, path);
		if (!stream.open(filename))
			errorFileNotFound(filename);

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

	char path[PATH_LEN];
	sprintf(path, "backs/%s.bkg", name);

	loadBackground(info, path);
	return;
}

GfxObj* AmigaDisk_br::loadStatic(const char* name) {
	debugC(1, kDebugDisk, "AmigaDisk_br::loadStatic '%s'", name);

	char path[PATH_LEN];
	sprintf(path, "%s/ras/%s", _partPath, name);
	Common::File stream;
	if (!stream.open(path)) {
		errorFileNotFound(path);
	}

	byte *pal = 0;
	Graphics::Surface* surf = new Graphics::Surface;

	Graphics::ILBMDecoder decoder(stream, *surf, pal);
	decoder.decode();

	free(pal);

	return new GfxObj(0, new SurfaceToFrames(surf));
}

Sprites* AmigaDisk_br::createSprites(const char *path) {

	Common::File	stream;
	if (!stream.open(path)) {
		errorFileNotFound(path);
	}

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

	char path[PATH_LEN];
	sprintf(path, "%s/anims/%s", _partPath, name);

	return createSprites(path);
}

GfxObj* AmigaDisk_br::loadTalk(const char *name) {
	debugC(1, kDebugDisk, "AmigaDisk_br::loadTalk '%s'", name);

	char path[PATH_LEN];
	sprintf(path, "%s/talks/%s.tal", _partPath, name);

	return new GfxObj(0, createSprites(path));
}

Font* AmigaDisk_br::loadFont(const char* name) {
	debugC(1, kDebugDisk, "AmigaFullDisk::loadFont '%s'", name);

	char path[PATH_LEN];
	sprintf(path, "%s", name);

	Common::File stream;
	if (!stream.open(path))
		errorFileNotFound(path);

	return createFont(name, stream);
}

} // namespace Parallaction
