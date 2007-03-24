/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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

#include "parallaction/defs.h"
#include "parallaction/graphics.h"
#include "parallaction/parallaction.h"
#include "parallaction/disk.h"
#include "parallaction/walk.h"

namespace Parallaction {


Disk::Disk(Parallaction *vm) : _vm(vm) {

}

Disk::~Disk() {

}

void Disk::selectArchive(const char *name) {
	_archive.open(name);
}

void Disk::setLanguage(uint16 language) {

	switch (language) {
	case 0:
		strcpy(_languageDir, "it/");
		break;

	case 1:
		strcpy(_languageDir, "fr/");
		break;

	case 2:
		strcpy(_languageDir, "en/");
		break;

	case 3:
		strcpy(_languageDir, "ge/");
		break;

	default:
		error("unknown language");

	}

	return;
}

#pragma mark -



DosDisk::DosDisk(Parallaction* vm) : Disk(vm) {

}

DosDisk::~DosDisk() {
}


//
// decompress a graphics block
//
uint16 DosDisk::decompressChunk(byte *src, byte *dst, uint16 size) {

	uint16 written = 0;
	uint16 read = 0;
	uint16 len = 0;

	for (; written != size; written += len) {

		len = src[read];
		read++;

		if (len <= 127) {
			// copy run

			len++;
			memcpy(dst+written, src+read, len);
			read += len;

		} else {
			// expand run

			len = 257 - len;
			memset(dst+written, src[read], len);
			read++;

		}

	}

	return read;
}


//
// loads a cnv from an external file
//
void DosDisk::loadExternalCnv(const char *filename, Cnv *cnv) {
//	printf("Gfx::loadExternalCnv(%s)...", filename);

	char path[PATH_LEN];

	sprintf(path, "%s.cnv", filename);

	Common::File stream;

	if (!stream.open(path))
		errorFileNotFound(path);

	cnv->_count = stream.readByte();
	cnv->_width = stream.readByte();
	cnv->_height = stream.readByte();

	cnv->_array = (byte**)malloc(cnv->_count * sizeof(byte*));

	uint16 size = cnv->_width*cnv->_height;
	for (uint16 i = 0; i < cnv->_count; i++) {
		cnv->_array[i] = (byte*)malloc(size);
		stream.read(cnv->_array[i], size);
	}

//	printf("done\n");


	return;
}

void DosDisk::loadExternalStaticCnv(const char *filename, StaticCnv *cnv) {

	char path[PATH_LEN];

	sprintf(path, "%s.cnv", filename);

	Common::File stream;

	if (!stream.open(path))
		errorFileNotFound(path);

	cnv->_width = cnv->_height = 0;

	stream.skip(1);
	cnv->_width = stream.readByte();
	cnv->_height = stream.readByte();

	uint16 size = cnv->_width*cnv->_height;

	cnv->_data0 = (byte*)malloc(size);
	stream.read(cnv->_data0, size);

	return;
}

void DosDisk::loadCnv(const char *filename, Cnv *cnv) {
//	printf("Gfx::loadCnv(%s)\n", filename);

	char path[PATH_LEN];

	strcpy(path, filename);
	if (!_archive.openArchivedFile(path)) {
		sprintf(path, "%s.pp", filename);
		if (!_archive.openArchivedFile(path))
			errorFileNotFound(path);
	}

	cnv->_count = _archive.readByte();
	cnv->_width = _archive.readByte();
	cnv->_height = _archive.readByte();

	uint16 framesize = cnv->_width*cnv->_height;

	cnv->_array = (byte**)malloc(cnv->_count * sizeof(byte*));

	uint32 size = _archive.size() - 3;

	byte *buf = (byte*)malloc(size);
	_archive.read(buf, size);

	byte *s = buf;

	for (uint16 i = 0; i < cnv->_count; i++) {
		cnv->_array[i] = (byte*)malloc(framesize);
		uint16 read = decompressChunk(s, cnv->_array[i], framesize);

//		printf("frame %i decompressed: %i --> %i\n", i, read, framesize);

		s += read;
	}

	free(buf);

	return;
}

Cnv* DosDisk::loadTalk(const char *name) {

	Cnv *cnv = new Cnv;

	const char *ext = strstr(name, ".talk");
	if (ext != NULL) {
		// npc talk
		loadCnv(name, cnv);

	} else {
		// character talk
/*
		if (scumm_stricmp(name, _doughName) &&
			scumm_stricmp(name, _dinoName) &&
			scumm_stricmp(name, _donnaName) &&
			scumm_stricmp(name, _drkiName)) return;
*/
		char v20[PATH_LEN];
		char *v24 = const_cast<char*>(name);
		if (IS_MINI_CHARACTER(v24)) {
			v24+=4;
		}

		if (_engineFlags & kEngineTransformedDonna) {
			sprintf(v20, "%stta", v24);
		} else {
			sprintf(v20, "%stal", v24);
		}

		loadExternalCnv(v20, cnv);

	}

	return cnv;
}

Script* DosDisk::loadLocation(const char *name) {

	char archivefile[PATH_LEN];

	if (IS_MINI_CHARACTER(_vm->_characterName)) {
		sprintf(archivefile, "%s%s", _vm->_characterName+4, _languageDir);
	} else {
		if (IS_DUMMY_CHARACTER(_vm->_characterName)) strcpy(archivefile, _languageDir);
		else {
			sprintf(archivefile, "%s%s", _vm->_characterName, _languageDir);
		}
	}
	strcat(archivefile, name);
	strcat(archivefile, ".loc");

	_languageDir[2] = '\0';
	_archive.open(_languageDir);
	_languageDir[2] = '/';

	if (!_archive.openArchivedFile(archivefile)) {
		sprintf(archivefile, "%s%s.loc", _languageDir, name);
		if (!_archive.openArchivedFile(archivefile))
			error("can't find location file '%s'", name);
	}

	uint32 size = _archive.size();
	char *buf = (char*)malloc(size+1);
	_archive.read(buf, size);
	buf[size] = '\0';

	return new Script(buf, true);

}

Script* DosDisk::loadScript(const char* name) {

	char vC8[PATH_LEN];

	sprintf(vC8, "%s.script", name);

	if (!_archive.openArchivedFile(vC8))
		errorFileNotFound(vC8);

	uint32 size = _archive.size();
	char *buf = (char*)malloc(size+1);
	_archive.read(buf, size);
	buf[size] = '\0';

	return new Script(buf, true);
}

StaticCnv* DosDisk::loadHead(const char* name) {

	char path[PATH_LEN];
/*
	if (scumm_stricmp(name, _doughName) &&
		scumm_stricmp(name, _dinoName) &&
		scumm_stricmp(name, _donnaName) &&
		scumm_stricmp(name, _drkiName)) return;
*/
	if (IS_MINI_CHARACTER(name)) {
		name += 4;
	}

	snprintf(path, 8, "%shead", name);
	path[8] = '\0';

	StaticCnv *cnv = new StaticCnv;
	loadExternalStaticCnv(path, cnv);
	return cnv;

}


StaticCnv* DosDisk::loadPointer() {
	StaticCnv* cnv = new StaticCnv;
	loadExternalStaticCnv("pointer", cnv);
	return cnv;
}

Cnv* DosDisk::loadFont(const char* name) {
	char path[PATH_LEN];

	sprintf(path, "%scnv", name);

	Cnv* cnv = new Cnv;
	loadExternalCnv(path, cnv);
	return cnv;
}

// loads character's icons set

Cnv* DosDisk::loadObjects(const char *name) {

	if (IS_MINI_CHARACTER(name)) {
		name += 4;
	}

	char path[PATH_LEN];
	sprintf(path, "%sobj", name);

	Cnv* cnv = new Cnv;
	loadExternalCnv(path, cnv);
	return cnv;
}


StaticCnv* DosDisk::loadStatic(const char* name) {

	char path[PATH_LEN];

	strcpy(path, name);
	if (!_archive.openArchivedFile(path)) {
		sprintf(path, "%s.pp", name);
		if (!_archive.openArchivedFile(path))
			errorFileNotFound(path);
	}

	StaticCnv* cnv = new StaticCnv;

	_archive.skip(1);
	cnv->_width = _archive.readByte();
	cnv->_height = _archive.readByte();

	uint16 compressedsize = _archive.size() - 3;
	byte *compressed = (byte*)malloc(compressedsize);

	uint16 size = cnv->_width*cnv->_height;
	cnv->_data0 = (byte*)malloc(size);

	_archive.read(compressed, compressedsize);

	decompressChunk(compressed, cnv->_data0, size);
	free(compressed);

	return cnv;
}

Cnv* DosDisk::loadFrames(const char* name) {
	Cnv* cnv = new Cnv;
	loadCnv(name, cnv);
	return cnv;
}

//
//	slides (background images) are stored compressed by scanline in a rle fashion
//
//	the uncompressed data must then be unpacked to get:
//	* color data [bits 0-5]
//	* mask data [bits 6-7] (z buffer)
//	* path data [bit 8] (walkable areas)
//


void DosDisk::unpackBackgroundScanline(byte *src, byte *screen, byte *mask, byte *path) {

	// update mask, path and screen
	for (uint16 i = 0; i < SCREEN_WIDTH; i++) {
		path[i/8] |= ((src[i] & 0x80) >> 7) << (i & 7);
		mask[i/4] |= ((src[i] & 0x60) >> 5) << ((i & 3) << 1);
		screen[i] = src[i] & 0x1F;
	}

	return;
}

void DosDisk::parseDepths(Common::SeekableReadStream &stream) {
	_vm->_gfx->_bgLayers[0] = stream.readByte();
	_vm->_gfx->_bgLayers[1] = stream.readByte();
	_vm->_gfx->_bgLayers[2] = stream.readByte();
	_vm->_gfx->_bgLayers[3] = stream.readByte();
}


void DosDisk::parseBackground(Common::SeekableReadStream &stream) {

	stream.read(_vm->_gfx->_palette, PALETTE_SIZE);

	parseDepths(stream);

	for (uint32 _si = 0; _si < 6; _si++) {
		_vm->_gfx->_palettefx[_si]._timer = stream.readUint16BE();
		_vm->_gfx->_palettefx[_si]._step = stream.readUint16BE();
		_vm->_gfx->_palettefx[_si]._flags = stream.readUint16BE();
		_vm->_gfx->_palettefx[_si]._first = stream.readByte();
		_vm->_gfx->_palettefx[_si]._last = stream.readByte();
	}

#if 0
	uint16 v147;
	for (v147 = 0; v147 < PALETTE_SIZE; v147++) {
		byte _al = _vm->_gfx->_palette[v147];
		_vm->_gfx->_palette[PALETTE_SIZE+v147] = _al / 2;
	}
#endif

}

void DosDisk::loadBackground(const char *filename) {
//	printf("Gfx::loadBackground(%s)\n", filename);

	if (!_archive.openArchivedFile(filename))
		errorFileNotFound(filename);

	parseBackground(_archive);

	byte *bg = (byte*)calloc(1, SCREEN_WIDTH*SCREEN_HEIGHT);
	byte *mask = (byte*)calloc(1, SCREENMASK_WIDTH*SCREEN_HEIGHT);
	byte *path = (byte*)calloc(1, SCREENPATH_WIDTH*SCREEN_HEIGHT);

	byte *v4 = (byte*)malloc(SCREEN_SIZE);
	_archive.read(v4, SCREEN_SIZE);

	byte v144[SCREEN_WIDTH];

	byte *s = v4;
	for (uint16 i = 0; i < SCREEN_HEIGHT; i++) {
		s += decompressChunk(s, v144, SCREEN_WIDTH);
		unpackBackgroundScanline(v144, bg+SCREEN_WIDTH*i, mask+SCREENMASK_WIDTH*i, path+SCREENPATH_WIDTH*i);
	}

	_vm->_gfx->setBackground(bg);
	_vm->_gfx->setMask(mask);
	setPath(path);

	free(v4);

	free(bg);
	free(mask);
	free(path);

	return;
}

//
//	read background path and mask from a file
//
//	mask and path are normally combined (via OR) into the background picture itself
//	read the comment on the top of this file for more
//
void DosDisk::loadMaskAndPath(const char *name) {
	char path[PATH_LEN];
	sprintf(path, "%s.msk", name);

	if (!_archive.openArchivedFile(path))
		errorFileNotFound(name);

	byte *maskBuf = (byte*)calloc(1, SCREENMASK_WIDTH*SCREEN_HEIGHT);
	byte *pathBuf = (byte*)calloc(1, SCREENPATH_WIDTH*SCREEN_HEIGHT);

	parseDepths(_archive);

	_archive.read(pathBuf, SCREENPATH_WIDTH*SCREEN_HEIGHT);
	_archive.read(maskBuf, SCREENMASK_WIDTH*SCREEN_HEIGHT);

	_vm->_gfx->setMask(maskBuf);
	setPath(pathBuf);

	return;
}

void DosDisk::loadSlide(const char *filename) {
	char path[PATH_LEN];
	sprintf(path, "%s.slide", filename);
	loadBackground(path);
}

void DosDisk::loadScenery(const char *name, const char *mask) {
	char path[PATH_LEN];
	sprintf(path, "%s.dyn", name);
	loadBackground(path);

	if (mask != NULL) {
		// load external masks and paths only for certain locations
		loadMaskAndPath(mask);
	}

}

Table* DosDisk::loadTable(const char* name) {
	char path[PATH_LEN];
	sprintf(path, "%s.tab", name);

	Common::File	stream;
	if (!stream.open(path))
		errorFileNotFound(path);

	Table *t = new Table(100);

	fillBuffers(stream);
	while (scumm_stricmp(_tokens[0], "ENDTABLE")) {
		t->addData(_tokens[0]);
		fillBuffers(stream);
	}

	stream.close();

	return t;
}


#pragma mark -



AmigaDisk::AmigaDisk(Parallaction *vm) : Disk(vm) {

}


AmigaDisk::~AmigaDisk() {

}

Script* AmigaDisk::loadLocation(const char *name) {
	return NULL;
}

Script* AmigaDisk::loadScript(const char* name) {
	return NULL;
}

Cnv* AmigaDisk::loadTalk(const char *name) {
	return NULL;
}

Cnv* AmigaDisk::loadObjects(const char *name) {
	return NULL;
}

StaticCnv* AmigaDisk::loadPointer() {
	return NULL;
}

StaticCnv* AmigaDisk::loadHead(const char* name) {
	return NULL;
}

Cnv* AmigaDisk::loadFont(const char* name) {
	return NULL;
}

StaticCnv* AmigaDisk::loadStatic(const char* name) {
	return NULL;
}

Cnv* AmigaDisk::loadFrames(const char* name) {
	return NULL;
}

void AmigaDisk::loadSlide(const char *filename) {
	return;
}

void AmigaDisk::loadScenery(const char* background, const char* mask) {
	return;
}

Table* AmigaDisk::loadTable(const char* name) {

	char path[PATH_LEN];
	sprintf(path, "%s.table", name);

	_archive.openArchivedFile(path);

	Table *t = new Table(100);

	fillBuffers(_archive);
	while (scumm_stricmp(_tokens[0], "ENDTABLE")) {
		t->addData(_tokens[0]);
		fillBuffers(_archive);
	}

	return t;
}


} // namespace Parallaction
