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


namespace Audio {
	class AudioStream;

	AudioStream *make8SVXStream(Common::ReadStream &input);
}

namespace Parallaction {


//  HACK: Several archives ('de', 'en', 'fr' and 'disk0') in the multi-lingual
//  Amiga version of Nippon Safes, and one archive ('fr') in the Amiga Demo of
//  Nippon Safes used different internal offsets than all the other archives.
//
//  When an archive is opened in the Amiga demo, its size is checked against
//  SIZEOF_SMALL_ARCHIVE to detect when the smaller archive is used.
//
//  When an archive is opened in Amiga multi-lingual version, the header is
//  checked again NDOS to detect when a smaller archive is used.
//
#define SIZEOF_SMALL_ARCHIVE		12778

#define ARCHIVE_FILENAMES_OFS		0x16

#define NORMAL_ARCHIVE_FILES_NUM	384
#define SMALL_ARCHIVE_FILES_NUM		180

#define NORMAL_ARCHIVE_SIZES_OFS	0x3016
#define SMALL_ARCHIVE_SIZES_OFS		0x1696

#define NORMAL_ARCHIVE_DATA_OFS		0x4000
#define SMALL_ARCHIVE_DATA_OFS		0x1966

Archive::Archive() {
	resetArchivedFile();
}

void Archive::open(const char *file) {
	debugC(1, kDebugDisk, "Archive::open(%s)", file);

	if (_archive.isOpen())
		close();

	if (!_archive.open(file))
		error("archive '%s' not found", file);

	_archiveName = file;

	bool isSmallArchive = false;
	if (_vm->getPlatform() == Common::kPlatformAmiga) {
		if (_vm->getFeatures() & GF_DEMO) {
			isSmallArchive = _archive.size() == SIZEOF_SMALL_ARCHIVE;
		} else if (_vm->getFeatures() & GF_LANG_MULT) {
			isSmallArchive = (_archive.readUint32BE() != MKID_BE('NDOS'));
		}
	}

	_numFiles = (isSmallArchive) ? SMALL_ARCHIVE_FILES_NUM : NORMAL_ARCHIVE_FILES_NUM;

	_archive.seek(ARCHIVE_FILENAMES_OFS);
	_archive.read(_archiveDir, _numFiles*32);

	_archive.seek((isSmallArchive) ? SMALL_ARCHIVE_SIZES_OFS : NORMAL_ARCHIVE_SIZES_OFS);

	uint32 dataOffset = (isSmallArchive) ? SMALL_ARCHIVE_DATA_OFS : NORMAL_ARCHIVE_DATA_OFS;
	for (uint16 i = 0; i < _numFiles; i++) {
		_archiveOffsets[i] = dataOffset;
		_archiveLenghts[i] = _archive.readUint32BE();
		dataOffset += _archiveLenghts[i];
	}

	return;
}


void Archive::close() {
	if (!_archive.isOpen()) return;

	resetArchivedFile();

	_archive.close();
	_archiveName.clear();
}

Common::String Archive::name() const {
	return _archiveName;
}

bool Archive::openArchivedFile(const char *filename) {
	debugC(3, kDebugDisk, "Archive::openArchivedFile(%s)", filename);

	resetArchivedFile();

	debugC(3, kDebugDisk, "Archive::openArchivedFile(%s)", filename);

	if (!_archive.isOpen())
		error("Archive::openArchivedFile: the archive is not open");

	uint16 i = 0;
	for ( ; i < _numFiles; i++) {
		if (!scumm_stricmp(_archiveDir[i], filename)) break;
	}
	if (i == _numFiles) return false;

	debugC(9, kDebugDisk, "Archive::openArchivedFile: '%s' found in slot %i", filename, i);

	_file = true;

	_fileOffset = _archiveOffsets[i];
	_fileCursor = _archiveOffsets[i];
	_fileEndOffset = _archiveOffsets[i] + _archiveLenghts[i];

	_archive.seek(_fileOffset);

	return true;
}

void Archive::resetArchivedFile() {
	_file = false;
	_fileCursor = 0;
	_fileOffset = 0;
	_fileEndOffset = 0;
}

void Archive::closeArchivedFile() {
	resetArchivedFile();
}


int32 Archive::size() const {
	return (_file == true ? _fileEndOffset - _fileOffset : 0);
}

int32 Archive::pos() const {
	return (_file == true ? _fileCursor - _fileOffset : 0 );
}

bool Archive::eos() const {
	return (_file == true ? _fileCursor == _fileEndOffset : true ); // FIXME (eos definition change)
}

bool Archive::seek(int32 offs, int whence) {
	assert(_file == true && _fileCursor <= _fileEndOffset);

	switch (whence) {
	case SEEK_CUR:
		_fileCursor += offs;
		break;
	case SEEK_SET:
		_fileCursor = _fileOffset + offs;
		break;
	case SEEK_END:
		_fileCursor = _fileEndOffset - offs;
		break;
	}
	assert(_fileCursor <= _fileEndOffset && _fileCursor >= _fileOffset);

	return _archive.seek(_fileCursor, SEEK_SET);
}

uint32 Archive::read(void *dataPtr, uint32 dataSize) {
//	printf("read(%i, %i)\n", file->_cursor, file->_endOffset);
	if (_file == false)
		error("Archive::read: no archived file is currently open");

	if (_fileCursor >= _fileEndOffset)
		error("can't read beyond end of archived file");

	if (_fileEndOffset - _fileCursor < dataSize)
		dataSize = _fileEndOffset - _fileCursor;

	int32 readBytes = _archive.read(dataPtr, dataSize);
	_fileCursor += readBytes;

	return readBytes;
}





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

	int32 pos() const {
		return _input->pos();
	}

	int32 size() const {
		return _input->size();
	}

	bool seek(int32 offset, int whence) {
		return _input->seek(offset, whence);
	}

};



Disk_ns::Disk_ns(Parallaction *vm) : _vm(vm) {

}

Disk_ns::~Disk_ns() {

}

void Disk_ns::errorFileNotFound(const char *s) {
	error("File '%s' not found", s);
}

Common::SeekableReadStream *Disk_ns::openFile(const char *filename) {
	Common::File *stream = new Common::File;
	if (!stream->open(filename))
		errorFileNotFound(filename);
	return stream;
}


Common::String Disk_ns::selectArchive(const Common::String& name) {
	Common::String oldName = _resArchive.name();
	_resArchive.open(name.c_str());
	return oldName;
}

void Disk_ns::setLanguage(uint16 language) {
	debugC(1, kDebugDisk, "setLanguage(%i)", language);
	assert(language < 4);
	const char *languages[] = { "it", "fr", "en", "ge" };
	sprintf(_languageDir, "%s/", languages[language]);
	_locArchive.open(languages[language]);
}

#pragma mark -



DosDisk_ns::DosDisk_ns(Parallaction* vm) : Disk_ns(vm) {

}

DosDisk_ns::~DosDisk_ns() {
}

//
// loads a cnv from an external file
//
Cnv* DosDisk_ns::loadExternalCnv(const char *filename) {

	char path[PATH_LEN];

	sprintf(path, "%s.cnv", filename);

	Common::SeekableReadStream *stream = openFile(path);

	uint16 numFrames = stream->readByte();
	uint16 width = stream->readByte();
	uint16 height = stream->readByte();
	uint32 decsize = numFrames * width * height;
	byte *data = (byte*)malloc(decsize);
	stream->read(data, decsize);
	Cnv *cnv = new Cnv(numFrames, width, height, data);

	delete stream;

	return cnv;
}


Frames* DosDisk_ns::loadCnv(const char *filename) {

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

GfxObj* DosDisk_ns::loadTalk(const char *name) {

	const char *ext = strstr(name, ".talk");
	if (ext != NULL) {
		// npc talk
		return new GfxObj(0, loadCnv(name), name);

	}

	char v20[30];
	if (_engineFlags & kEngineTransformedDonna) {
		sprintf(v20, "%stta", name);
	} else {
		sprintf(v20, "%stal", name);
	}

	return new GfxObj(0, loadExternalCnv(v20), name);
}

Script* DosDisk_ns::loadLocation(const char *name) {

	char archivefile[PATH_LEN];
	sprintf(archivefile, "%s%s%s.loc", _vm->_char.getBaseName(), _languageDir, name);

	debugC(3, kDebugDisk, "DosDisk_ns::loadLocation(%s): trying '%s'", name, archivefile);

	if (!_locArchive.openArchivedFile(archivefile)) {
		sprintf(archivefile, "%s%s.loc", _languageDir, name);
		debugC(3, kDebugDisk, "DosDisk_ns::loadLocation(%s): trying '%s'", name, archivefile);

		if (!_locArchive.openArchivedFile(archivefile))
			errorFileNotFound(name);
	}

	return new Script(new DummyArchiveStream(_locArchive), true);
}

Script* DosDisk_ns::loadScript(const char* name) {

	char vC8[PATH_LEN];

	sprintf(vC8, "%s.script", name);

	if (!_resArchive.openArchivedFile(vC8))
		errorFileNotFound(vC8);

	return new Script(new DummyArchiveStream(_resArchive), true);
}

GfxObj* DosDisk_ns::loadHead(const char* name) {
	char path[PATH_LEN];
	sprintf(path, "%shead", name);
	path[8] = '\0';
	return new GfxObj(0, loadExternalCnv(path));
}


Frames* DosDisk_ns::loadPointer(const char *name) {
	return loadExternalCnv(name);
}


Font* DosDisk_ns::loadFont(const char* name) {
	char path[PATH_LEN];
	sprintf(path, "%scnv", name);
	return createFont(name, loadExternalCnv(path));
}


GfxObj* DosDisk_ns::loadObjects(const char *name) {
	char path[PATH_LEN];
	sprintf(path, "%sobj", name);
	return new GfxObj(0, loadExternalCnv(path), name);
}


GfxObj* DosDisk_ns::loadStatic(const char* name) {
	return new GfxObj(0, loadCnv(name), name);
}

Frames* DosDisk_ns::loadFrames(const char* name) {
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
void DosDisk_ns::unpackBackground(Common::ReadStream *stream, byte *screen, byte *mask, byte *path) {

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

void DosDisk_ns::parseDepths(BackgroundInfo &info, Common::SeekableReadStream &stream) {
	info.layers[0] = stream.readByte();
	info.layers[1] = stream.readByte();
	info.layers[2] = stream.readByte();
	info.layers[3] = stream.readByte();
}


void DosDisk_ns::parseBackground(BackgroundInfo& info, Common::SeekableReadStream &stream) {

	byte tmp[3];

	for (uint i = 0; i < 32; i++) {
		tmp[0] = stream.readByte();
		tmp[1] = stream.readByte();
		tmp[2] = stream.readByte();
		info.palette.setEntry(i, tmp[0], tmp[1], tmp[2]);
	}

	parseDepths(info, stream);

	PaletteFxRange range;
	for (uint32 _si = 0; _si < 6; _si++) {
		range._timer = stream.readUint16BE();
		range._step = stream.readUint16BE();
		range._flags = stream.readUint16BE();
		range._first = stream.readByte();
		range._last = stream.readByte();

		info.setPaletteRange(_si, range);
	}

}

void DosDisk_ns::loadBackground(BackgroundInfo& info, const char *filename) {

	if (!_resArchive.openArchivedFile(filename))
		errorFileNotFound(filename);

	info.width = _vm->_screenWidth;	// 320
	info.height = _vm->_screenHeight;	// 200

	parseBackground(info, _resArchive);

	info.bg.create(info.width, info.height, 1);
	info.mask.create(info.width, info.height);
	info.mask.bigEndian = true;
	info.path.create(info.width, info.height);

	Graphics::PackBitsReadStream stream(_resArchive);
	unpackBackground(&stream, (byte*)info.bg.pixels, info.mask.data, info.path.data);

	return;
}

//
//	read background path and mask from a file
//
//	mask and path are normally combined (via OR) into the background picture itself
//	read the comment on the top of this file for more
//
void DosDisk_ns::loadMaskAndPath(BackgroundInfo& info, const char *name) {
	char path[PATH_LEN];
	sprintf(path, "%s.msk", name);

	if (!_resArchive.openArchivedFile(path))
		errorFileNotFound(name);

	parseDepths(info, _resArchive);

	info.path.create(info.width, info.height);
	_resArchive.read(info.path.data, info.path.size);

	info.mask.create(info.width, info.height);
	info.mask.bigEndian = true;
	_resArchive.read(info.mask.data, info.mask.size);

	return;
}

void DosDisk_ns::loadSlide(BackgroundInfo& info, const char *filename) {
	char path[PATH_LEN];
	sprintf(path, "%s.slide", filename);
	loadBackground(info, path);

	return;
}

void DosDisk_ns::loadScenery(BackgroundInfo& info, const char *name, const char *mask, const char* path) {
	char filename[PATH_LEN];
	sprintf(filename, "%s.dyn", name);

	loadBackground(info, filename);

	if (mask != NULL) {
		// load external masks and paths only for certain locations
		loadMaskAndPath(info, mask);
	}

	return;
}

Table* DosDisk_ns::loadTable(const char* name) {
	char path[PATH_LEN];
	sprintf(path, "%s.tab", name);
	Common::SeekableReadStream *stream = openFile(path);
	Table *t = createTableFromStream(100, *stream);
	delete stream;
	return t;
}

Common::SeekableReadStream* DosDisk_ns::loadMusic(const char* name) {
	char path[PATH_LEN];
	sprintf(path, "%s.mid", name);
	return openFile(path);
}


Common::ReadStream* DosDisk_ns::loadSound(const char* name) {
	return NULL;
}






#pragma mark -


/* the decoder presented here is taken from pplib by Stuart Caie. The
 * following statement comes from the original source.
 *
 * pplib 1.0: a simple PowerPacker decompression and decryption library
 * placed in the Public Domain on 2003-09-18 by Stuart Caie.
 */

#define PP_READ_BITS(nbits, var) do {				\
	bit_cnt = (nbits); (var) = 0;				\
	while (bits_left < bit_cnt) {				\
		if (buf < src) return 0;			\
		bit_buffer |= *--buf << bits_left;		\
		bits_left += 8;					\
	}							\
	bits_left -= bit_cnt;					\
	while (bit_cnt--) {					\
		(var) = ((var) << 1) | (bit_buffer & 1);	\
		bit_buffer >>= 1;				\
	}							\
} while (0)

#define PP_BYTE_OUT(byte) do {					\
	if (out <= dest) return 0;				\
	*--out = (byte); written++;				\
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

	int32 size() const {
		return _stream->size();
	}

	int32 pos() const {
		return _stream->pos();
	}

	bool eos() const {
		return _stream->eos();
	}

	bool seek(int32 offs, int whence = SEEK_SET) {
		return _stream->seek(offs, whence);
	}

	uint32 read(void *dataPtr, uint32 dataSize) {
		return _stream->read(dataPtr, dataSize);
	}
};





AmigaDisk_ns::AmigaDisk_ns(Parallaction *vm) : Disk_ns(vm) {

}


AmigaDisk_ns::~AmigaDisk_ns() {

}

#define NUM_PLANES		5

/*
	unpackFrame transforms images from 5-bitplanes format to
	8-bit color-index mode
*/
void AmigaDisk_ns::unpackFrame(byte *dst, byte *src, uint16 planeSize) {

	byte s0, s1, s2, s3, s4, mask, t0, t1, t2, t3, t4;

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

}

/*
	patchFrame applies DLTA data (dlta) to specified buffer (dst)
*/
void AmigaDisk_ns::patchFrame(byte *dst, byte *dlta, uint16 bytesPerPlane, uint16 height) {

	uint32 *dataIndex = (uint32*)dlta;
	uint32 *ofslenIndex = (uint32*)dlta + 8;

	uint16 *base = (uint16*)dlta;
	uint16 wordsPerLine = bytesPerPlane >> 1;

	for (uint j = 0; j < NUM_PLANES; j++) {
		uint16 *dst16 = (uint16*)(dst + j * bytesPerPlane * height);

		uint16 *data = base + READ_BE_UINT32(dataIndex);
		dataIndex++;
		uint16 *ofslen = base + READ_BE_UINT32(ofslenIndex);
		ofslenIndex++;

		while (*ofslen != 0xFFFF) {

			uint16 ofs = READ_BE_UINT16(ofslen);
			ofslen++;
			uint16 size = READ_BE_UINT16(ofslen);
			ofslen++;

			while (size > 0) {
				dst16[ofs] ^= *data++;
				ofs += wordsPerLine;
				size--;
			}

		}

	}

}

// FIXME: no mask is loaded
void AmigaDisk_ns::unpackBitmap(byte *dst, byte *src, uint16 numFrames, uint16 bytesPerPlane, uint16 height) {

	byte *baseFrame = src;
	byte *tempBuffer = 0;

	uint16 planeSize = bytesPerPlane * height;

	for (uint32 i = 0; i < numFrames; i++) {
		if (READ_BE_UINT32(src) == MKID_BE('DLTA')) {

			uint size = READ_BE_UINT32(src + 4);

			if (tempBuffer == 0)
				tempBuffer = (byte*)malloc(planeSize * NUM_PLANES);

			memcpy(tempBuffer, baseFrame, planeSize * NUM_PLANES);

			patchFrame(tempBuffer, src + 8, bytesPerPlane, height);
			unpackFrame(dst, tempBuffer, planeSize);

			src += (size + 8);
			dst += planeSize * 8;
		} else {
			unpackFrame(dst, src, planeSize);
			src += planeSize * NUM_PLANES;
			dst += planeSize * 8;
		}
	}

	free(tempBuffer);

}

Frames* AmigaDisk_ns::makeStaticCnv(Common::SeekableReadStream &stream) {

	stream.skip(1);
	uint16 width = stream.readByte();
	uint16 height = stream.readByte();

	assert((width & 7) == 0);

	byte bytesPerPlane = width / 8;

	uint32 rawsize = bytesPerPlane * NUM_PLANES * height;
	byte *buf = (byte*)malloc(rawsize);
	stream.read(buf, rawsize);

	Graphics::Surface *cnv = new Graphics::Surface;
	cnv->create(width, height, 1);

	unpackBitmap((byte*)cnv->pixels, buf, 1, bytesPerPlane, height);

	free(buf);

	return new SurfaceToFrames(cnv);
}

Cnv* AmigaDisk_ns::makeCnv(Common::SeekableReadStream &stream) {

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

	unpackBitmap(data, buf, numFrames, bytesPerPlane, height);

	free(buf);

	return new Cnv(numFrames, width, height, data);
}
#undef NUM_PLANES

Script* AmigaDisk_ns::loadLocation(const char *name) {
	debugC(1, kDebugDisk, "AmigaDisk_ns()::loadLocation '%s'", name);

	char path[PATH_LEN];
	sprintf(path, "%s%s%s.loc.pp", _vm->_char.getBaseName(), _languageDir, name);

	if (!_locArchive.openArchivedFile(path)) {
		sprintf(path, "%s%s.loc.pp", _languageDir, name);
		if (!_locArchive.openArchivedFile(path)) {
			errorFileNotFound(name);
		}
	}

	debugC(3, kDebugDisk, "location file found: %s", path);

	return new Script(new PowerPackerStream(_locArchive), true);
}

Script* AmigaDisk_ns::loadScript(const char* name) {
	debugC(1, kDebugDisk, "AmigaDisk_ns::loadScript '%s'", name);

	char vC8[PATH_LEN];

	sprintf(vC8, "%s.script", name);

	if (!_resArchive.openArchivedFile(vC8))
		errorFileNotFound(vC8);

	return new Script(new DummyArchiveStream(_resArchive), true);
}

Frames* AmigaDisk_ns::loadPointer(const char* name) {
	debugC(1, kDebugDisk, "AmigaDisk_ns::loadPointer");

	Common::SeekableReadStream *stream = openFile(name);
	Frames *frames = makeStaticCnv(*stream);
	delete stream;

	return frames;
}

GfxObj* AmigaDisk_ns::loadStatic(const char* name) {
	debugC(1, kDebugDisk, "AmigaDisk_ns::loadStatic '%s'", name);

	Common::SeekableReadStream *s = openArchivedFile(name, true);
	Frames *cnv = makeStaticCnv(*s);

	delete s;

	return new GfxObj(0, cnv, name);
}

Common::SeekableReadStream *AmigaDisk_ns::openArchivedFile(const char* name, bool errorOnFileNotFound) {
	debugC(3, kDebugDisk, "AmigaDisk_ns::openArchivedFile(%s)", name);

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

/*
	FIXME: mask values are not computed correctly for level 1 and 2

	NOTE: this routine is only able to build masks for Nippon Safes, since mask widths are hardcoded
	into the main loop.
*/
void buildMask(byte* buf) {

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


void AmigaDisk_ns::loadBackground(BackgroundInfo& info, const char *name) {

	Common::SeekableReadStream *s = openArchivedFile(name, true);

	byte *pal;
	PaletteFxRange ranges[6];

	BackgroundDecoder decoder(*s, info.bg, pal, ranges);
	decoder.decode();

	uint i;

	info.width = info.bg.w;
	info.height = info.bg.h;

	byte *p = pal;
	for (i = 0; i < 32; i++) {
		byte r = *p >> 2;
		p++;
		byte g = *p >> 2;
		p++;
		byte b = *p >> 2;
		p++;
		info.palette.setEntry(i, r, g, b);
	}

	free(pal);

	for (i = 0; i < 6; i++) {
		info.setPaletteRange(i, ranges[i]);
	}

	delete s;

	return;

}

void AmigaDisk_ns::loadMask(BackgroundInfo& info, const char *name) {
	debugC(5, kDebugDisk, "AmigaDisk_ns::loadMask(%s)", name);

	char path[PATH_LEN];
	sprintf(path, "%s.mask", name);

	Common::SeekableReadStream *s = openArchivedFile(path, false);
	if (s == NULL) {
		debugC(5, kDebugDisk, "Mask file not found");
		return;	// no errors if missing mask files: not every location has one
	}

	s->seek(0x30, SEEK_SET);

	byte r, g, b;
	for (uint i = 0; i < 4; i++) {
		r = s->readByte();
		g = s->readByte();
		b = s->readByte();

		info.layers[i] = (((r << 4) & 0xF00) | (g & 0xF0) | (b >> 4)) & 0xFF;
	}

	s->seek(0x126, SEEK_SET);	// HACK: skipping IFF/ILBM header should be done by analysis, not magic
	Graphics::PackBitsReadStream stream(*s);

	info.mask.create(info.width, info.height);
	stream.read(info.mask.data, info.mask.size);
	buildMask(info.mask.data);

	delete s;

	return;
}

void AmigaDisk_ns::loadPath(BackgroundInfo& info, const char *name) {

	char path[PATH_LEN];
	sprintf(path, "%s.path", name);

	Common::SeekableReadStream *s = openArchivedFile(path, false);
	if (s == NULL)
		return;	// no errors if missing path files: not every location has one


	s->seek(0x120, SEEK_SET);	// HACK: skipping IFF/ILBM header should be done by analysis, not magic

	Graphics::PackBitsReadStream stream(*s);

	info.path.create(info.width, info.height);
	stream.read(info.path.data, info.path.size);

	delete s;

	return;
}

void AmigaDisk_ns::loadScenery(BackgroundInfo& info, const char* background, const char* mask, const char* path) {
	debugC(1, kDebugDisk, "AmigaDisk_ns::loadScenery '%s', '%s'", background, mask);

	char filename[PATH_LEN];
	sprintf(filename, "%s.bkgnd", background);

	loadBackground(info, filename);

	if (mask == NULL) {
		loadMask(info, background);
		loadPath(info, background);
	} else {
		loadMask(info, mask);
		loadPath(info, mask);
	}

	return;
}

void AmigaDisk_ns::loadSlide(BackgroundInfo& info, const char *name) {
	debugC(1, kDebugDisk, "AmigaDisk_ns::loadSlide '%s'", name);
	loadBackground(info, name);
	return;
}

Frames* AmigaDisk_ns::loadFrames(const char* name) {
	debugC(1, kDebugDisk, "AmigaDisk_ns::loadFrames '%s'", name);

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

GfxObj* AmigaDisk_ns::loadHead(const char* name) {
	debugC(1, kDebugDisk, "AmigaDisk_ns::loadHead '%s'", name);

	char path[PATH_LEN];
	sprintf(path, "%s.head", name);

	Common::SeekableReadStream *s = openArchivedFile(path, true);
	Frames *cnv = makeStaticCnv(*s);

	delete s;

	return new GfxObj(0, cnv, name);
}


GfxObj* AmigaDisk_ns::loadObjects(const char *name) {
	debugC(1, kDebugDisk, "AmigaDisk_ns::loadObjects");

	char path[PATH_LEN];
	if (_vm->getFeatures() & GF_DEMO)
		sprintf(path, "%s.objs", name);
	else
		sprintf(path, "objs/%s.objs", name);

	Common::SeekableReadStream *s = openArchivedFile(path, true);

	Cnv *cnv = makeCnv(*s);
	delete s;

	return new GfxObj(0, cnv, name);
}


GfxObj* AmigaDisk_ns::loadTalk(const char *name) {
	debugC(1, kDebugDisk, "AmigaDisk_ns::loadTalk '%s'", name);

	Common::SeekableReadStream *s;

	char path[PATH_LEN];
	if (_vm->getFeatures() & GF_DEMO)
		sprintf(path, "%s.talk", name);
	else
		sprintf(path, "talk/%s.talk", name);

	s = openArchivedFile(path, false);
	if (s == NULL) {
		s = openArchivedFile(name, true);
	}

	Cnv *cnv = makeCnv(*s);
	delete s;

	return new GfxObj(0, cnv, name);
}

Table* AmigaDisk_ns::loadTable(const char* name) {
	debugC(1, kDebugDisk, "AmigaDisk_ns::loadTable '%s'", name);

	char path[PATH_LEN];
	sprintf(path, "%s.table", name);

	bool dispose = false;

	Common::SeekableReadStream *stream;

	if (!scumm_stricmp(name, "global")) {
		Common::SeekableReadStream *s = openFile(path);
		dispose = true;
		stream = s;
	} else {
		if (!(_vm->getFeatures() & GF_DEMO))
			sprintf(path, "objs/%s.table", name);
		if (!_resArchive.openArchivedFile(path))
			errorFileNotFound(path);

		stream = &_resArchive;
	}

	Table *t = createTableFromStream(100, *stream);

	if (dispose)
		delete stream;

	return t;
}

Font* AmigaDisk_ns::loadFont(const char* name) {
	debugC(1, kDebugDisk, "AmigaFullDisk::loadFont '%s'", name);

	char path[PATH_LEN];
	sprintf(path, "%sfont", name);

	Font *font = 0;

	if (_vm->getFeatures() & GF_LANG_IT) {
		// Italian version has separate font files
		Common::SeekableReadStream *stream = openFile(path);
		font = createFont(name, *stream);
		delete stream;
	} else {
		if (!_resArchive.openArchivedFile(path))
			errorFileNotFound(path);
		font = createFont(name, _resArchive);
	}

	return font;
}


Common::SeekableReadStream* AmigaDisk_ns::loadMusic(const char* name) {
	return openArchivedFile(name);
}

Common::ReadStream* AmigaDisk_ns::loadSound(const char* name) {
	char path[PATH_LEN];
	sprintf(path, "%s.snd", name);

	return openArchivedFile(path);
}

} // namespace Parallaction
