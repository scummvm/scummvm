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

namespace Parallaction {

//
// decompress a graphics block
//
uint16 decompressChunk(byte *src, byte *dst, uint16 size) {

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
void loadExternalCnv(const char *filename, Cnv *cnv) {
//	printf("Graphics::loadExternalCnv(%s)...", filename);

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

	stream.close();

//	printf("done\n");


	return;
}

void loadExternalStaticCnv(const char *filename, StaticCnv *cnv) {

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

	stream.close();

	return;
}

void loadCnv(const char *filename, Cnv *cnv) {
//	printf("Graphics::loadCnv(%s)\n", filename);

	char path[PATH_LEN];

	strcpy(path, filename);
	if (!_vm->_archive.openArchivedFile(path)) {
		sprintf(path, "%s.pp", filename);
		if (!_vm->_archive.openArchivedFile(path))
			errorFileNotFound(path);
	}

	cnv->_count = _vm->_archive.readByte();
	cnv->_width = _vm->_archive.readByte();
	cnv->_height = _vm->_archive.readByte();

	uint16 framesize = cnv->_width*cnv->_height;

	cnv->_array = (byte**)malloc(cnv->_count * sizeof(byte*));

	uint32 size = _vm->_archive.size() - 3;

	byte *buf = (byte*)malloc(size);
	_vm->_archive.read(buf, size);

	byte *s = buf;

	for (uint16 i = 0; i < cnv->_count; i++) {
		cnv->_array[i] = (byte*)malloc(framesize);
		uint16 read = decompressChunk(s, cnv->_array[i], framesize);

//		printf("frame %i decompressed: %i --> %i\n", i, read, framesize);

		s += read;
	}

	_vm->_archive.closeArchivedFile();

	free(buf);

	return;
}

void loadTalk(const char *name, Cnv *cnv) {

	char* ext = strstr(name, ".talk");
	if (ext != NULL) {
		// npc talk
		loadCnv(name, cnv);

	} else {
		// character talk

		char v20[PATH_LEN];
		char *v24 = const_cast<char*>(name);
		if (!scumm_strnicmp(v24, "mini", 4)) {
			v24+=4;
		}

		if (_engineFlags & kEngineMiniDonna) {
			sprintf(v20, "%stta", v24);
		} else {
			sprintf(v20, "%stal", v24);
		}

		loadExternalCnv(v20, cnv);

	}

}

void loadLocation(const char *name, char* script) {

	char archivefile[PATH_LEN];

	if (_vm->_characterName[0] == 'm') {
		sprintf(archivefile, "%s%s", _vm->_characterName+4, _vm->_languageDir);
	} else {
		if (_vm->_characterName[0] == 'D') strcpy(archivefile, _vm->_languageDir);
		else {
			sprintf(archivefile, "%s%s", _vm->_characterName, _vm->_languageDir);
		}
	}
	strcat(archivefile, name);
	strcat(archivefile, ".loc");

	_vm->_archive.close();

	_vm->_languageDir[2] = '\0';
	_vm->_archive.open(_vm->_languageDir);
	_vm->_languageDir[2] = '/';

	if (!_vm->_archive.openArchivedFile(archivefile)) {
		sprintf(archivefile, "%s%s.loc", _vm->_languageDir, name);
		if (!_vm->_archive.openArchivedFile(archivefile))
			error("can't find location file '%s'", name);
	}

	uint32 count = _vm->_archive.size();
	_vm->_archive.read(script, count);
	_vm->_archive.closeArchivedFile();
	_vm->_archive.close();

}

void loadScript(const char* name, char *script) {

	char vC8[PATH_LEN];

	sprintf(vC8, "%s.script", name);

	if (!_vm->_archive.openArchivedFile(vC8))
		errorFileNotFound(vC8);

	uint32 size = _vm->_archive.size();

	_vm->_archive.read(script, size);
	script[size] = '\0';

	_vm->_archive.closeArchivedFile();

}

void loadHead(const char* name, StaticCnv* cnv) {

	char path[PATH_LEN];

	if (!scumm_strnicmp(name, "mini", 4)) {
		name += 4;
	}

	snprintf(path, 8, "%shead", name);
	path[8] = '\0';

	loadExternalStaticCnv(path, cnv);

}


void loadPointer(StaticCnv* cnv) {
	loadExternalStaticCnv("pointer", cnv);
}

void loadFont(const char* name, Cnv* cnv) {
	char path[PATH_LEN];

	sprintf(path, "%scnv", name);
	loadExternalCnv(path, cnv);
}

// loads character's icons set

void loadObjects(const char *name, Cnv* cnv) {

	if (!scumm_strnicmp("mini", name, 4)) {
		name += 4;
	}

	char path[PATH_LEN];
	sprintf(path, "%sobj", name);

	loadExternalCnv(path, cnv);

	return;
}


void loadStatic(const char* name, StaticCnv* cnv) {

	char path[PATH_LEN];

	strcpy(path, name);
	if (!_vm->_archive.openArchivedFile(path)) {
		sprintf(path, "%s.pp", name);
		if (!_vm->_archive.openArchivedFile(path))
			errorFileNotFound(path);
	}

	_vm->_archive.skip(1);
	cnv->_width = _vm->_archive.readByte();
	cnv->_height = _vm->_archive.readByte();

	uint16 compressedsize = _vm->_archive.size() - 3;
	byte *compressed = (byte*)malloc(compressedsize);

	uint16 size = cnv->_width*cnv->_height;
	cnv->_data0 = (byte*)malloc(size);

	_vm->_archive.read(compressed, compressedsize);
	_vm->_archive.closeArchivedFile();

	decompressChunk(compressed, cnv->_data0, size);
	free(compressed);

	return;
}

void loadFrames(const char* name, Cnv* cnv) {

	loadCnv(name, cnv);

	return;
}

//
//	slides (background images) are stored compressed by scanline in a rle fashion
//
//	the uncompressed data must then be unpacked to get:
//	* color data [bits 0-5]
//	* mask data [bits 6-7] (z buffer)
//	* path data [bit 8] (walkable areas)
//


void unpackBackgroundScanline(byte *src, byte *screen, byte *mask, byte *path) {

	// update mask, path and screen
	for (uint16 i = 0; i < SCREEN_WIDTH; i++) {
		path[i/8] |= ((src[i] & 0x80) >> 7) << (i & 7);
		mask[i/4] |= ((src[i] & 0x60) >> 5) << ((i & 3) << 1);
		screen[i] = src[i] & 0x1F;
	}

	return;
}

void loadBackground(const char *filename) {
//	printf("Graphics::loadBackground(%s)\n", filename);

	if (!_vm->_archive.openArchivedFile(filename))
		errorFileNotFound(filename);

	_vm->_graphics->parseBackground(_vm->_archive);

	byte *bg = (byte*)calloc(1, SCREEN_WIDTH*SCREEN_HEIGHT);
	byte *mask = (byte*)calloc(1, SCREENMASK_WIDTH*SCREEN_HEIGHT);
	byte *path = (byte*)calloc(1, SCREENPATH_WIDTH*SCREEN_HEIGHT);

	byte *v4 = (byte*)malloc(SCREEN_SIZE);
	_vm->_archive.read(v4, SCREEN_SIZE);

	byte v144[SCREEN_WIDTH];

	byte *s = v4;
	for (uint16 i = 0; i < SCREEN_HEIGHT; i++) {
		s += decompressChunk(s, v144, SCREEN_WIDTH);
		unpackBackgroundScanline(v144, bg+SCREEN_WIDTH*i, mask+SCREENMASK_WIDTH*i, path+SCREENPATH_WIDTH*i);
	}

	_vm->_graphics->setBackground(bg);
	_vm->_graphics->setMask(mask);
	_vm->_graphics->setPath(path);

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
void loadMaskAndPath(const char *name) {
	char path[PATH_LEN];
	sprintf(path, "%s.msk", name);

	if (!_vm->_archive.openArchivedFile(path))
		errorFileNotFound(name);

	byte *maskBuf = (byte*)calloc(1, SCREENMASK_WIDTH*SCREEN_HEIGHT);
	byte *pathBuf = (byte*)calloc(1, SCREENPATH_WIDTH*SCREEN_HEIGHT);

	_vm->_graphics->parseDepths(_vm->_archive);

	_vm->_archive.read(pathBuf, SCREENPATH_WIDTH*SCREEN_HEIGHT);
	_vm->_archive.read(maskBuf, SCREENMASK_WIDTH*SCREEN_HEIGHT);

	_vm->_graphics->setMask(maskBuf);
	_vm->_graphics->setPath(pathBuf);

	return;
}

void loadSlide(const char *filename) {
	char path[PATH_LEN];
	sprintf(path, "%s.slide", filename);
	loadBackground(path);
}

void loadScenery(const char *filename) {
	char path[PATH_LEN];
	sprintf(path, "%s.dyn", filename);
	loadBackground(path);
}

void loadBackground(const char* background, const char* mask) {

	loadScenery(background);

	if (scumm_stricmp(background, mask)) {
		// load external masks and paths only for certain locations
		loadMaskAndPath(mask);
	}

}


} // namespace Parallaction
