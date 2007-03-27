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
#include "graphics/ilbm.h"

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

	byte pal[96];
	stream.read(pal, PALETTE_SIZE);
	_vm->_gfx->setPalette(pal);

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


/* the decoder presented here is taken from pplib by Stuart Caie. The
 * following statement comes from the original source.
 *
 * pplib 1.0: a simple PowerPacker decompression and decryption library
 * placed in the Public Domain on 2003-09-18 by Stuart Caie.
 */

#define PP_READ_BITS(nbits, var) do {                            \
  bit_cnt = (nbits); (var) = 0;                                  \
  while (bits_left < bit_cnt) {                                  \
    if (buf < src) return 0;                                     \
    bit_buffer |= *--buf << bits_left;                           \
    bits_left += 8;                                              \
  }                                                              \
  bits_left -= bit_cnt;                                          \
  while (bit_cnt--) {                                            \
    (var) = ((var) << 1) | (bit_buffer & 1);                     \
    bit_buffer >>= 1;                                            \
  }                                                              \
} while (0)

#define PP_BYTE_OUT(byte) do {                                   \
  if (out <= dest) return 0;                                     \
  *--out = (byte); written++;                                    \
} while (0)


class DecrunchStream : public Common::SeekableReadStream {

	SeekableReadStream *_stream;
	bool				_dispose;

private:
	int ppDecrunchBuffer(byte *src, byte *dest, uint32 src_len, uint32 dest_len) {

		byte *buf, *out, *dest_end, *off_lens, bits_left = 0, bit_cnt;
		uint32 bit_buffer = 0, x, todo, offbits, offset, written = 0;

		if (src == NULL || dest == NULL) return 0;

		/* set up input and output pointers */
		off_lens = src; src = &src[4];
		buf = &src[src_len];

		out = dest_end = &dest[dest_len];

		/* skip the first few bits */
		PP_READ_BITS(src[src_len + 3], x);

		/* while there are input bits left */
		while (written < dest_len) {
			PP_READ_BITS(1, x);
			if (x == 0) {
				  /* bit==0: literal, then match. bit==1: just match */
				  todo = 1; do { PP_READ_BITS(2, x); todo += x; } while (x == 3);
				  while (todo--) { PP_READ_BITS(8, x); PP_BYTE_OUT(x); }

				  /* should we end decoding on a literal, break out of the main loop */
				  if (written == dest_len) break;
			}

			/* match: read 2 bits for initial offset bitlength / match length */
			PP_READ_BITS(2, x);
			offbits = off_lens[x];
			todo = x+2;
			if (x == 3) {
				PP_READ_BITS(1, x);
				if (x == 0) offbits = 7;
				PP_READ_BITS(offbits, offset);
				do { PP_READ_BITS(3, x); todo += x; } while (x == 7);
			}
			else {
				PP_READ_BITS(offbits, offset);
			}
			if (&out[offset] >= dest_end) return 0; /* match_overflow */
			while (todo--) { x = out[offset]; PP_BYTE_OUT(x); }
		}

		/* all output bytes written without error */
		return 1;
	}

	uint16 getCrunchType(uint32 signature) {

		byte eff;

		switch (signature) {
		case 0x50503230: /* PP20 */
			eff = 4;
			break;
		case 0x50504C53: /* PPLS */
			error("PPLS crunched files are not supported");
			eff = 8;
			break;
		case 0x50583230: /* PX20 */
			error("PX20 crunched files are not supported");
			eff = 6;
			break;
		default:
			eff = 0;

		}

		return eff;
	}

public:
	DecrunchStream(Common::SeekableReadStream &stream) {

		_dispose = false;

		uint32 signature = stream.readUint32BE();
		if (getCrunchType(signature) == 0) {
			stream.seek(0, SEEK_SET);
			_stream = &stream;
			return;
		}

		stream.seek(4, SEEK_END);
		uint32 decrlen = stream.readUint32BE() >> 8;
		byte *dest = (byte*)malloc(decrlen);

		uint32 crlen = stream.size() - 4;
		byte *src = (byte*)malloc(crlen);
		stream.seek(4, SEEK_SET);
		stream.read(src, crlen);

		ppDecrunchBuffer(src, dest, crlen-8, decrlen);

		free(src);
		_stream = new Common::MemoryReadStream(dest, decrlen, true);
		_dispose = true;
	}

	~DecrunchStream() {
		if (_dispose) delete _stream;
	}

	uint32 size() const {
		return _stream->size();
	}

	uint32 pos() const {
		return _stream->pos();
	}

	bool eos() const {
		return _stream->eos();
	}

	void seek(int32 offs, int whence = SEEK_SET) {
		_stream->seek(offs, whence);
	}

	uint32 read(void *dataPtr, uint32 dataSize) {
		return _stream->read(dataPtr, dataSize);
	}
};



AmigaDisk::AmigaDisk(Parallaction *vm) : Disk(vm) {

}


AmigaDisk::~AmigaDisk() {

}

Script* AmigaDisk::loadLocation(const char *name) {
	debugC(1, kDebugDisk, "AmigaDisk::loadLocation '%s'", name);
	return NULL;
}

Script* AmigaDisk::loadScript(const char* name) {
	debugC(1, kDebugDisk, "AmigaDisk::loadScript '%s'", name);
	return NULL;
}

Cnv* AmigaDisk::loadTalk(const char *name) {
	debugC(1, kDebugDisk, "AmigaDisk::loadTalk '%s'", name);
	return NULL;
}

Cnv* AmigaDisk::loadObjects(const char *name) {
	debugC(1, kDebugDisk, "AmigaDisk::loadObjects '%s'", name);
	return NULL;
}

StaticCnv* AmigaDisk::loadPointer() {
	debugC(1, kDebugDisk, "AmigaDisk::loadPointer");
	return NULL;
}

StaticCnv* AmigaDisk::loadHead(const char* name) {
	debugC(1, kDebugDisk, "AmigaDisk::loadHead '%s'", name);
	return NULL;
}

Cnv* AmigaDisk::loadFont(const char* name) {
	debugC(1, kDebugDisk, "AmigaDisk::loadFont '%s'", name);
	return NULL;
}

StaticCnv* AmigaDisk::loadStatic(const char* name) {
	debugC(1, kDebugDisk, "AmigaDisk::loadStatic '%s'", name);
	return NULL;
}

Cnv* AmigaDisk::loadFrames(const char* name) {
	debugC(1, kDebugDisk, "AmigaDisk::loadFrames '%s'", name);
	return NULL;
}

void AmigaDisk::loadSlide(const char *name) {
	debugC(1, kDebugDisk, "AmigaDisk::loadSlide '%s'", name);

	char path[PATH_LEN];
	sprintf(path, "%s.pp", name);

	if (!_archive.openArchivedFile(path))
		error("can't open archived file %s", path);

	DecrunchStream stream(_archive);

	Graphics::Surface surf;
	byte *pal;

	// CRNG headers may be safely ignored for slides
	Graphics::ILBMDecoder decoder(stream);
	decoder.decode(surf, pal);

	for (uint32 i = 0; i < 96; i++)
		pal[i] >>= 2;


	_vm->_gfx->setPalette(pal);
	free(pal);

	_vm->_gfx->setBackground(static_cast<byte*>(surf.pixels));

	surf.free();

	return;
}

void AmigaDisk::loadScenery(const char* background, const char* mask) {
	debugC(1, kDebugDisk, "AmigaDisk::loadScenery '%s', '%s'", background, mask);
	return;
}

Table* AmigaDisk::loadTable(const char* name) {
	printf("AmigaDisk::loadTable\n");

	char path[PATH_LEN];
	sprintf(path, "%s.table", name);

	Common::SeekableReadStream *stream;

	if (!scumm_stricmp(name, "global")) {
		Common::File *s = new Common::File;
		if (!s->open(path))
			error("can't open %s", path);

		stream = s;
	} else {
		if (!_archive.openArchivedFile(path))
			error("can't open archived file %s", path);

//		DecrunchStream *s = new DecrunchStream(_archive);
		stream = &_archive;
	}

	Table *t = new Table(100);

	fillBuffers(*stream);
	while (scumm_stricmp(_tokens[0], "ENDTABLE")) {
		t->addData(_tokens[0]);
		fillBuffers(*stream);
	}

	delete stream;

	return t;
}


} // namespace Parallaction
