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

#include "common/stdafx.h"

#include "graphics/iff.h"
#include "parallaction/parallaction.h"


namespace Audio {
	AudioStream *make8SVXStream(Common::ReadStream &input);
}

namespace Parallaction {

/*
	This stream class is just a wrapper around Archive, so
	deallocation is not a problem. In fact, this class doesn't
	delete its input (Archive) stream.
*/
class DummyArchiveStream : public Common::SeekableReadStream {

	Archive *_input;

public:
	DummyArchiveStream(Archive &input) : _input(&input) {

	}

	~DummyArchiveStream() {
		// this class exists to provide this empty destructor
	}

	bool eos() const {
		return _input->eos();
	}

	uint32 read(void* data, uint32 dataSize) {
		return _input->read(data, dataSize);
	}

	uint32 pos() const {
		return _input->pos();
	}

	uint32 size() const {
		return _input->size();
	}

	void seek(int32 offset, int whence) {
		_input->seek(offset, whence);
	}

};



Disk::Disk(Parallaction *vm) : _vm(vm) {

}

Disk::~Disk() {

}

void Disk::errorFileNotFound(const char *s) {
	error("File '%s' not found", s);
}


void Disk::selectArchive(const char *name) {
	_resArchive.open(name);
}

void Disk::setLanguage(uint16 language) {
	debugC(1, kDebugDisk, "setLanguage(%i)", language);

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

	_languageDir[2] = '\0';
	_locArchive.open(_languageDir);
	_languageDir[2] = '/';

	return;
}

#pragma mark -



DosDisk::DosDisk(Parallaction* vm) : Disk(vm) {

}

DosDisk::~DosDisk() {
}


//
// loads a cnv from an external file
//
Cnv* DosDisk::loadExternalCnv(const char *filename) {
//	printf("Gfx::loadExternalCnv(%s)...", filename);

	char path[PATH_LEN];

	sprintf(path, "%s.cnv", filename);

	Common::File stream;

	if (!stream.open(path))
		errorFileNotFound(path);

	uint16 numFrames = stream.readByte();
	uint16 width = stream.readByte();
	uint16 height = stream.readByte();

	uint32 decsize = numFrames * width * height;
	byte *data = (byte*)malloc(decsize);
	stream.read(data, decsize);

	return new Cnv(numFrames, width, height, data);
}

StaticCnv *DosDisk::loadExternalStaticCnv(const char *filename) {

	char path[PATH_LEN];

	sprintf(path, "%s.cnv", filename);

	Common::File stream;

	if (!stream.open(path))
		errorFileNotFound(path);

	StaticCnv *cnv = new StaticCnv;

	stream.skip(1);
	cnv->_width = stream.readByte();
	cnv->_height = stream.readByte();

	uint16 size = cnv->_width*cnv->_height;

	cnv->_data0 = (byte*)malloc(size);
	stream.read(cnv->_data0, size);

	return cnv;
}

Cnv* DosDisk::loadCnv(const char *filename) {
//	printf("Gfx::loadCnv(%s)\n", filename);

	char path[PATH_LEN];

	strcpy(path, filename);
	if (!_resArchive.openArchivedFile(path)) {
		sprintf(path, "%s.pp", filename);
		if (!_resArchive.openArchivedFile(path))
			errorFileNotFound(path);
	}

	uint16 numFrames = _resArchive.readByte();
	uint16 width = _resArchive.readByte();
	uint16 height = _resArchive.readByte();

	uint32 decsize = numFrames * width * height;
	byte *data = (byte*)malloc(decsize);

	Graphics::PackBitsReadStream decoder(_resArchive);
	decoder.read(data, decsize);

	return new Cnv(numFrames, width, height, data);
}

Cnv* DosDisk::loadTalk(const char *name) {

	const char *ext = strstr(name, ".talk");
	if (ext != NULL) {
		// npc talk
		return loadCnv(name);

	}

	// character talk
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

	return loadExternalCnv(v20);
}

Script* DosDisk::loadLocation(const char *name) {

	char archivefile[PATH_LEN];

	if (IS_MINI_CHARACTER(_vm->_characterName)) {
		sprintf(archivefile, "%s%s", _vm->_characterName+4, _languageDir);
	} else {
		if (IS_DUMMY_CHARACTER(_vm->_characterName)) {
			strcpy(archivefile, _languageDir);
		} else {
			sprintf(archivefile, "%s%s", _vm->_characterName, _languageDir);
		}
	}

	strcat(archivefile, name);
	strcat(archivefile, ".loc");

	debugC(3, kDebugDisk, "DosDisk::loadLocation(%s): trying '%s'", name, archivefile);

	if (!_locArchive.openArchivedFile(archivefile)) {
		sprintf(archivefile, "%s%s.loc", _languageDir, name);
		debugC(3, kDebugDisk, "DosDisk::loadLocation(%s): trying '%s'", name, archivefile);

		if (!_locArchive.openArchivedFile(archivefile))
			errorFileNotFound(name);
	}

	return new Script(new DummyArchiveStream(_locArchive), true);
}

Script* DosDisk::loadScript(const char* name) {

	char vC8[PATH_LEN];

	sprintf(vC8, "%s.script", name);

	if (!_resArchive.openArchivedFile(vC8))
		errorFileNotFound(vC8);

	return new Script(new DummyArchiveStream(_resArchive), true);
}

StaticCnv* DosDisk::loadHead(const char* name) {

	char path[PATH_LEN];

	if (IS_MINI_CHARACTER(name)) {
		name += 4;
	}

	sprintf(path, "%shead", name);
	path[8] = '\0';

	return loadExternalStaticCnv(path);
}


StaticCnv* DosDisk::loadPointer() {
	return loadExternalStaticCnv("pointer");
}


Font* DosDisk::loadFont(const char* name) {
	char path[PATH_LEN];
	sprintf(path, "%scnv", name);
	return createFont(name, loadExternalCnv(path));
}


Cnv* DosDisk::loadObjects(const char *name) {

	if (IS_MINI_CHARACTER(name)) {
		name += 4;
	}

	char path[PATH_LEN];
	sprintf(path, "%sobj", name);
	return loadExternalCnv(path);
}


StaticCnv* DosDisk::loadStatic(const char* name) {

	char path[PATH_LEN];

	strcpy(path, name);
	if (!_resArchive.openArchivedFile(path)) {
		sprintf(path, "%s.pp", name);
		if (!_resArchive.openArchivedFile(path))
			errorFileNotFound(path);
	}

	StaticCnv* cnv = new StaticCnv;

	_resArchive.skip(1);
	cnv->_width = _resArchive.readByte();
	cnv->_height = _resArchive.readByte();

	uint16 size = cnv->_width*cnv->_height;
	cnv->_data0 = (byte*)malloc(size);

	Graphics::PackBitsReadStream decoder(_resArchive);
	decoder.read(cnv->_data0, size);

	return cnv;
}

Cnv* DosDisk::loadFrames(const char* name) {
	return loadCnv(name);
}

//
//	slides (background images) are stored compressed by scanline in a rle fashion
//
//	the uncompressed data must then be unpacked to get:
//	* color data [bits 0-5]
//	* mask data [bits 6-7] (z buffer)
//	* path data [bit 8] (walkable areas)
//
void DosDisk::unpackBackground(Common::ReadStream *stream, byte *screen, byte *mask, byte *path) {

	byte b;
	uint32 i = 0;

	while (!stream->eos()) {
		b = stream->readByte();

		path[i/8] |= ((b & 0x80) >> 7) << (i & 7);
		mask[i/4] |= ((b & 0x60) >> 5) << ((i & 3) << 1);
		screen[i] = b & 0x1F;
		i++;
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

	stream.read(_vm->_gfx->_palette, BASE_PALETTE_SIZE);
	_vm->_gfx->setPalette(_vm->_gfx->_palette);

	parseDepths(stream);

	for (uint32 _si = 0; _si < 6; _si++) {
		_vm->_gfx->_palettefx[_si]._timer = stream.readUint16BE();
		_vm->_gfx->_palettefx[_si]._step = stream.readUint16BE();
		_vm->_gfx->_palettefx[_si]._flags = stream.readUint16BE();
		_vm->_gfx->_palettefx[_si]._first = stream.readByte();
		_vm->_gfx->_palettefx[_si]._last = stream.readByte();
	}

}

void DosDisk::loadBackground(const char *filename) {

	if (!_resArchive.openArchivedFile(filename))
		errorFileNotFound(filename);

	parseBackground(_resArchive);

	byte *bg = (byte*)calloc(1, SCREEN_WIDTH*SCREEN_HEIGHT);
	byte *mask = (byte*)calloc(1, SCREENMASK_WIDTH*SCREEN_HEIGHT);
	byte *path = (byte*)calloc(1, SCREENPATH_WIDTH*SCREEN_HEIGHT);


	Graphics::PackBitsReadStream stream(_resArchive);
	unpackBackground(&stream, bg, mask, path);

	_vm->_gfx->setBackground(bg);
	_vm->_gfx->setMask(mask);
	_vm->setPath(path);

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

	if (!_resArchive.openArchivedFile(path))
		errorFileNotFound(name);

	byte *maskBuf = (byte*)calloc(1, SCREENMASK_WIDTH*SCREEN_HEIGHT);
	byte *pathBuf = (byte*)calloc(1, SCREENPATH_WIDTH*SCREEN_HEIGHT);

	parseDepths(_resArchive);

	_resArchive.read(pathBuf, SCREENPATH_WIDTH*SCREEN_HEIGHT);
	_resArchive.read(maskBuf, SCREENMASK_WIDTH*SCREEN_HEIGHT);

	_vm->_gfx->setMask(maskBuf);
	_vm->setPath(pathBuf);

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

Common::ReadStream* DosDisk::loadMusic(const char* name) {
	char path[PATH_LEN];
	sprintf(path, "%s.mid", name);

	Common::File *stream = new Common::File;
	if (!stream->open(path))
		errorFileNotFound(path);

	return stream;
}


Common::ReadStream* DosDisk::loadSound(const char* name) {
	return NULL;
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


class PowerPackerStream : public Common::SeekableReadStream {

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
	PowerPackerStream(Common::SeekableReadStream &stream) {

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

	~PowerPackerStream() {
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

#define NUM_PLANES		5

// FIXME: no mask is loaded
void AmigaDisk::unpackBitmap(byte *dst, byte *src, uint16 numFrames, uint16 planeSize) {

	byte s0, s1, s2, s3, s4, mask, t0, t1, t2, t3, t4;

	for (uint32 i = 0; i < numFrames; i++) {
		for (uint32 j = 0; j < planeSize; j++) {
			s0 = src[j];
			s1 = src[j+planeSize];
			s2 = src[j+planeSize*2];
			s3 = src[j+planeSize*3];
			s4 = src[j+planeSize*4];

			for (uint32 k = 0; k < 8; k++) {
				mask = 1 << (7 - k);
				t0 = (s0 & mask ? 1 << 0 : 0);
				t1 = (s1 & mask ? 1 << 1 : 0);
				t2 = (s2 & mask ? 1 << 2 : 0);
				t3 = (s3 & mask ? 1 << 3 : 0);
				t4 = (s4 & mask ? 1 << 4 : 0);
				*dst++ = t0 | t1 | t2 | t3 | t4;
			}

		}

		src += planeSize * NUM_PLANES;
	}
}

StaticCnv* AmigaDisk::makeStaticCnv(Common::SeekableReadStream &stream) {

	stream.skip(1);
	uint16 width = stream.readByte();
	uint16 height = stream.readByte();

	assert((width & 7) == 0);

	byte bytesPerPlane = width / 8;

	uint32 rawsize = bytesPerPlane * NUM_PLANES * height;
	byte *buf = (byte*)malloc(rawsize);
	stream.read(buf, rawsize);

	uint32 decsize = width * height;
	byte *data = (byte*)calloc(decsize, 1);

	unpackBitmap(data, buf, 1, height * bytesPerPlane);

	free(buf);

	StaticCnv *cnv = new StaticCnv();
	cnv->_width = width;
	cnv->_height = height;
	cnv->_data0 = data;
	cnv->_data1 = NULL;

	return cnv;
}

Cnv* AmigaDisk::makeCnv(Common::SeekableReadStream &stream) {

	uint16 numFrames = stream.readByte();
	uint16 width = stream.readByte();
	uint16 height = stream.readByte();

	assert((width & 7) == 0);

	byte bytesPerPlane = width / 8;

	uint32 rawsize = numFrames * bytesPerPlane * NUM_PLANES * height;
	byte *buf = (byte*)malloc(rawsize);
	stream.read(buf, rawsize);

	uint32 decsize = numFrames * width * height;
	byte *data = (byte*)calloc(decsize, 1);

	unpackBitmap(data, buf, numFrames, height * bytesPerPlane);

	free(buf);

	return new Cnv(numFrames, width, height, data);
}
#undef NUM_PLANES

Script* AmigaDisk::loadLocation(const char *name) {
	debugC(1, kDebugDisk, "AmigaDisk()::loadLocation '%s'", name);

	char path[PATH_LEN];
	if (IS_MINI_CHARACTER(_vm->_characterName)) {
		sprintf(path, "%s%s%s.loc.pp", _vm->_characterName+4, _languageDir, name);
	} else
		sprintf(path, "%s%s%s.loc.pp", _vm->_characterName, _languageDir, name);

	if (!_locArchive.openArchivedFile(path)) {
		sprintf(path, "%s%s.loc.pp", _languageDir, name);
		if (!_locArchive.openArchivedFile(path)) {
			errorFileNotFound(name);
		}
	}

	debugC(3, kDebugDisk, "location file found: %s", path);

	return new Script(new PowerPackerStream(_locArchive), true);
}

Script* AmigaDisk::loadScript(const char* name) {
	debugC(1, kDebugDisk, "AmigaDisk::loadScript '%s'", name);

	char vC8[PATH_LEN];

	sprintf(vC8, "%s.script", name);

	if (!_resArchive.openArchivedFile(vC8))
		errorFileNotFound(vC8);

	return new Script(new DummyArchiveStream(_resArchive), true);
}

StaticCnv* AmigaDisk::loadPointer() {
	debugC(1, kDebugDisk, "AmigaDisk::loadPointer");

	Common::File stream;
	if (!stream.open("pointer"))
		errorFileNotFound("pointer");

	return makeStaticCnv(stream);
}

StaticCnv* AmigaDisk::loadStatic(const char* name) {
	debugC(1, kDebugDisk, "AmigaDisk::loadStatic '%s'", name);

	Common::SeekableReadStream *s = openArchivedFile(name, true);
	StaticCnv *cnv = makeStaticCnv(*s);

	delete s;

	return cnv;
}

Common::SeekableReadStream *AmigaDisk::openArchivedFile(const char* name, bool errorOnFileNotFound) {
	debugC(3, kDebugDisk, "AmigaDisk::openArchivedFile(%s)", name);

	if (_resArchive.openArchivedFile(name)) {
		return new DummyArchiveStream(_resArchive);
	}

	char path[PATH_LEN];

	sprintf(path, "%s.pp", name);
	if (_resArchive.openArchivedFile(path)) {
		return new PowerPackerStream(_resArchive);
	}

	sprintf(path, "%s.dd", name);
	if (_resArchive.openArchivedFile(path)) {
		return new PowerPackerStream(_resArchive);
	}

	if (errorOnFileNotFound)
		errorFileNotFound(name);

	return NULL;
}

// FIXME: mask values are not computed correctly for level 1 and 2
void buildMask(byte* buf) {

	byte mask1[16] = { 0, 0x80, 0x20, 0xA0, 8, 0x88, 0x28, 0xA8, 2, 0x82, 0x22, 0xA2, 0xA, 0x8A, 0x2A, 0xAA };
	byte mask0[16] = { 0, 0x40, 0x10, 0x50, 4, 0x44, 0x14, 0x54, 1, 0x41, 0x11, 0x51, 0x5, 0x45, 0x15, 0x55 };

	byte plane0[40];
	byte plane1[40];

	for (uint32 i = 0; i < 200; i++) {

		memcpy(plane0, buf, 40);
		memcpy(plane1, buf+40, 40);

		for (uint32 j = 0; j < 40; j++) {
			*buf++ = mask0[(plane0[j] & 0xF0) >> 4] | mask1[(plane1[j] & 0xF0) >> 4];
			*buf++ = mask0[plane0[j] & 0xF] | mask1[plane1[j] & 0xF];
		}

	}
}

class BackgroundDecoder : public Graphics::ILBMDecoder {

	PaletteFxRange *_range;
	uint32			_i;

protected:
	void readCRNG(Common::IFFChunk &chunk) {
		_range[_i]._timer = chunk.readUint16BE();
		_range[_i]._step = chunk.readUint16BE();
		_range[_i]._flags = chunk.readUint16BE();
		_range[_i]._first = chunk.readByte();
		_range[_i]._last = chunk.readByte();

		_i++;
	}

public:
	BackgroundDecoder(Common::ReadStream &input, Graphics::Surface &surface, byte *&colors, PaletteFxRange *range) :
		Graphics::ILBMDecoder(input, surface, colors), _range(range), _i(0) {
	}

	void decode() {
		Common::IFFChunk *chunk;
		while ((chunk = nextChunk()) != 0) {
			switch (chunk->id) {
			case ID_BMHD:
				readBMHD(*chunk);
				break;

			case ID_CMAP:
				readCMAP(*chunk);
				break;

			case ID_BODY:
				readBODY(*chunk);
				break;

			case ID_CRNG:
				readCRNG(*chunk);
				break;
			}
		}
	}

	uint32	getNumRanges() {
		return _i;
	}
};


void AmigaDisk::loadBackground(const char *name) {

	Common::SeekableReadStream *s = openArchivedFile(name, true);

	Graphics::Surface surf;
	byte *pal;
	BackgroundDecoder decoder(*s, surf, pal, _vm->_gfx->_palettefx);
	decoder.decode();

	for (uint32 i = 0; i < BASE_PALETTE_COLORS * 3; i++)
		_vm->_gfx->_palette[i] = pal[i] >> 2;
	free(pal);
	_vm->_gfx->setPalette(_vm->_gfx->_palette);
	_vm->_gfx->setBackground(static_cast<byte*>(surf.pixels));
	surf.free();
	delete s;

	return;

}

void AmigaDisk::loadMask(const char *name) {

	char path[PATH_LEN];
	sprintf(path, "%s.mask", name);

	Common::SeekableReadStream *s = openArchivedFile(path, false);
	if (s == NULL)
		return;	// no errors if missing mask files: not every location has one

	s->seek(0x30, SEEK_SET);

	byte r, g, b;
	for (uint i = 0; i < 4; i++) {
		r = s->readByte();
		g = s->readByte();
		b = s->readByte();

		_vm->_gfx->_bgLayers[i] = (((r << 4) & 0xF00) | (g & 0xF0) | (b >> 4)) & 0xFF;

//		printf("rgb = (%x, %x, %x) -> %x\n", r, g, b, _vm->_gfx->_bgLayers[i]);
	}


	s->seek(0x126, SEEK_SET);	// HACK: skipping IFF/ILBM header should be done by analysis, not magic
	Graphics::PackBitsReadStream stream(*s);

	byte *buf = (byte*)malloc(SCREENMASK_WIDTH*SCREEN_HEIGHT);
	stream.read(buf, SCREENMASK_WIDTH*SCREEN_HEIGHT);
	buildMask(buf);
	_vm->_gfx->setMask(buf);
	free(buf);
	delete s;

	return;
}

void AmigaDisk::loadPath(const char *name) {

	char path[PATH_LEN];
	sprintf(path, "%s.path", name);

	Common::SeekableReadStream *s = openArchivedFile(path, false);
	if (s == NULL)
		return;	// no errors if missing path files: not every location has one


	s->seek(0x120, SEEK_SET);	// HACK: skipping IFF/ILBM header should be done by analysis, not magic

	Graphics::PackBitsReadStream stream(*s);
	byte *buf = (byte*)malloc(SCREENPATH_WIDTH*SCREEN_HEIGHT);
	stream.read(buf, SCREENPATH_WIDTH*SCREEN_HEIGHT);
	_vm->setPath(buf);
	free(buf);
	delete s;

	return;
}

void AmigaDisk::loadScenery(const char* background, const char* mask) {
	debugC(1, kDebugDisk, "AmigaDisk::loadScenery '%s', '%s'", background, mask);

	char path[PATH_LEN];
	sprintf(path, "%s.bkgnd", background);

	loadBackground(path);
	loadMask(background);
	loadPath(background);

	return;
}


AmigaFullDisk::AmigaFullDisk(Parallaction *vm) : AmigaDisk(vm) {
}


void AmigaFullDisk::loadSlide(const char *name) {
	debugC(1, kDebugDisk, "AmigaDisk::loadSlide '%s'", name);

	char path[PATH_LEN];
	sprintf(path, "slides/%s", name);
	Common::SeekableReadStream *s = openArchivedFile(path, false);
	if (s)
		loadBackground(path);
	else
		loadBackground(name);

	return;
}

Cnv* AmigaFullDisk::loadFrames(const char* name) {
	debugC(1, kDebugDisk, "AmigaDisk::loadFrames '%s'", name);

	Common::SeekableReadStream *s;

	char path[PATH_LEN];
	sprintf(path, "anims/%s", name);

	s = openArchivedFile(path, false);
	if (!s)
		s = openArchivedFile(name, true);

	Cnv *cnv = makeCnv(*s);
	delete s;

	return cnv;
}



StaticCnv* AmigaFullDisk::loadHead(const char* name) {
	debugC(1, kDebugDisk, "AmigaDisk::loadHead '%s'", name);

	char path[PATH_LEN];
	sprintf(path, "%s.head", name);

	Common::SeekableReadStream *s = openArchivedFile(path, true);
	StaticCnv *cnv = makeStaticCnv(*s);

	delete s;

	return cnv;
}


Cnv* AmigaFullDisk::loadObjects(const char *name) {
	debugC(1, kDebugDisk, "AmigaDisk::loadObjects");

	char path[PATH_LEN];
	sprintf(path, "objs/%s.objs", name);
	Common::SeekableReadStream *s = openArchivedFile(path, true);

	Cnv *cnv = makeCnv(*s);
	delete s;

	return cnv;
}


Cnv* AmigaFullDisk::loadTalk(const char *name) {
	debugC(1, kDebugDisk, "AmigaDisk::loadTalk '%s'", name);

	Common::SeekableReadStream *s;

	char path[PATH_LEN];
	sprintf(path, "talk/%s.talk", name);
	s = openArchivedFile(path, false);
	if (s == NULL) {
		s = openArchivedFile(name, true);
	}

	Cnv *cnv = makeCnv(*s);
	delete s;

	return cnv;
}

Table* AmigaFullDisk::loadTable(const char* name) {
	debugC(1, kDebugDisk, "AmigaDisk::loadTable '%s'", name);

	char path[PATH_LEN];
	sprintf(path, "%s.table", name);

	bool dispose = false;

	Common::SeekableReadStream *stream;

	if (!scumm_stricmp(name, "global")) {
		Common::File *s = new Common::File;
		if (!s->open(path))
			errorFileNotFound(path);

		dispose = true;
		stream = s;
	} else {
		sprintf(path, "objs/%s.table", name);
		if (!_resArchive.openArchivedFile(path))
			errorFileNotFound(path);

		stream = &_resArchive;
	}

	Table *t = new Table(100);

	fillBuffers(*stream);
	while (scumm_stricmp(_tokens[0], "ENDTABLE")) {
		t->addData(_tokens[0]);
		fillBuffers(*stream);
	}

	if (dispose)
		delete stream;

	return t;
}

Font* AmigaFullDisk::loadFont(const char* name) {
	debugC(1, kDebugDisk, "AmigaFullDisk::loadFont '%s'", name);

	char path[PATH_LEN];
	sprintf(path, "%sfont", name);

	if (_vm->getFeatures() & GF_LANG_MULT) {
		if (!_resArchive.openArchivedFile(path))
			errorFileNotFound(path);

		return createFont(name, _resArchive);
	} else {
		// Italian version has separate font files?
		Common::File stream;
		if (!stream.open(path))
			errorFileNotFound(path);

		return createFont(name, stream);
	}
}


Common::ReadStream* AmigaDisk::loadMusic(const char* name) {
	return openArchivedFile(name);
}

Common::ReadStream* AmigaDisk::loadSound(const char* name) {
	char path[PATH_LEN];
	sprintf(path, "%s.snd", name);

	openArchivedFile(path);

	return new DummyArchiveStream(_resArchive);
}

AmigaDemoDisk::AmigaDemoDisk(Parallaction *vm) : AmigaDisk(vm) {
}

Cnv* AmigaDemoDisk::loadTalk(const char *name) {
	debugC(1, kDebugDisk, "AmigaDisk::loadTalk '%s'", name);

	Common::SeekableReadStream *s;

	char path[PATH_LEN];
	sprintf(path, "%s.talk", name);
	s = openArchivedFile(path, false);
	if (s == NULL) {
		s = openArchivedFile(name, true);
	}

	Cnv *cnv = makeCnv(*s);
	delete s;

	return cnv;
}

Cnv* AmigaDemoDisk::loadObjects(const char *name) {
	debugC(1, kDebugDisk, "AmigaDisk::loadObjects");

	char path[PATH_LEN];
	sprintf(path, "%s.objs", name);
	Common::SeekableReadStream *s = openArchivedFile(path, true);

	Cnv *cnv = makeCnv(*s);
	delete s;

	return cnv;
}


Cnv* AmigaDemoDisk::loadFrames(const char* name) {
	debugC(1, kDebugDisk, "AmigaDisk::loadFrames '%s'", name);

	if (IS_MINI_CHARACTER(name))
		return NULL;

	Common::SeekableReadStream *s = openArchivedFile(name, true);
	Cnv *cnv = makeCnv(*s);
	delete s;

	return cnv;
}

void AmigaDemoDisk::loadSlide(const char *name) {
	debugC(1, kDebugDisk, "AmigaDisk::loadSlide '%s'", name);
	loadBackground(name);
	return;
}

StaticCnv* AmigaDemoDisk::loadHead(const char* name) {
	debugC(1, kDebugDisk, "AmigaDisk::loadHead '%s'", name);

	char path[PATH_LEN];
	sprintf(path, "%s.head", name);

	Common::SeekableReadStream *s = openArchivedFile(path, true);
	StaticCnv *cnv = makeStaticCnv(*s);

	delete s;

	return cnv;
}

Table* AmigaDemoDisk::loadTable(const char* name) {
	debugC(1, kDebugDisk, "AmigaDisk::loadTable '%s'", name);

	char path[PATH_LEN];
	sprintf(path, "%s.table", name);

	bool dispose = false;

	Common::SeekableReadStream *stream;

	if (!scumm_stricmp(name, "global")) {
		Common::File *s = new Common::File;
		if (!s->open(path))
			errorFileNotFound(path);

		dispose = true;
		stream = s;
	} else {
		if (!_resArchive.openArchivedFile(path))
			errorFileNotFound(path);

		stream = &_resArchive;
	}

	Table *t = new Table(100);

	fillBuffers(*stream);
	while (scumm_stricmp(_tokens[0], "ENDTABLE")) {
		t->addData(_tokens[0]);
		fillBuffers(*stream);
	}

	if (dispose)
		delete stream;

	return t;
}

Font* AmigaDemoDisk::loadFont(const char* name) {
	debugC(1, kDebugDisk, "AmigaDisk::loadFont '%s'", name);

	char path[PATH_LEN];
	sprintf(path, "%sfont", name);

	if (!_resArchive.openArchivedFile(path))
		errorFileNotFound(path);

	return createFont(name, _resArchive);
}

} // namespace Parallaction
