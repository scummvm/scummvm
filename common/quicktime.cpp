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

//
// Heavily based on ffmpeg code.
//
// Copyright (c) 2001 Fabrice Bellard.
// First version by Francois Revol revol@free.fr
// Seek function by Gael Chardon gael.dev@4now.net
//

#include "common/debug.h"
#include "common/endian.h"
#include "common/macresman.h"
#include "common/memstream.h"
#include "common/quicktime.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "common/zlib.h"

namespace Common {

////////////////////////////////////////////
// QuickTimeParser
////////////////////////////////////////////

QuickTimeParser::QuickTimeParser() {
	_beginOffset = 0;
	_numStreams = 0;
	_fd = 0;
	_scaleFactorX = 1;
	_scaleFactorY = 1;
	_resFork = new Common::MacResManager();
	_disposeFileHandle = DisposeAfterUse::YES;

	initParseTable();
}

QuickTimeParser::~QuickTimeParser() {
	close();
	delete _resFork;
}

bool QuickTimeParser::parseFile(const Common::String &filename) {
	if (!_resFork->open(filename) || !_resFork->hasDataFork())
		return false;

	_foundMOOV = false;
	_numStreams = 0;
	_disposeFileHandle = DisposeAfterUse::YES;

	MOVatom atom = { 0, 0, 0xffffffff };

	if (_resFork->hasResFork()) {
		// Search for a 'moov' resource
		Common::MacResIDArray idArray = _resFork->getResIDArray(MKTAG('m', 'o', 'o', 'v'));

		if (!idArray.empty())
			_fd = _resFork->getResource(MKTAG('m', 'o', 'o', 'v'), idArray[0]);

		if (_fd) {
			atom.size = _fd->size();
			if (readDefault(atom) < 0 || !_foundMOOV)
				return false;
		}
		delete _fd;

		atom.type = 0;
		atom.offset = 0;
		atom.size = 0xffffffff;
	}

	_fd = _resFork->getDataFork();

	if (readDefault(atom) < 0 || !_foundMOOV)
		return false;

	init();
	return true;
}

bool QuickTimeParser::parseStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeFileHandle) {
	_fd = stream;
	_foundMOOV = false;
	_numStreams = 0;
	_disposeFileHandle = disposeFileHandle;

	MOVatom atom = { 0, 0, 0xffffffff };

	if (readDefault(atom) < 0 || !_foundMOOV) {
		close();
		return false;
	}

	init();
	return true;
}

void QuickTimeParser::init() {
	// Remove unknown/unhandled streams
	for (uint32 i = 0; i < _numStreams;) {
		if (_streams[i]->codec_type == CODEC_TYPE_MOV_OTHER) {
			delete _streams[i];
			for (uint32 j = i + 1; j < _numStreams; j++)
				_streams[j - 1] = _streams[j];
			_numStreams--;
		} else
			i++;
	}

	// Adjust time scale
	for (uint32 i = 0; i < _numStreams; i++)
		if (!_streams[i]->time_scale)
			_streams[i]->time_scale = _timeScale;
}

void QuickTimeParser::initParseTable() {
	static const ParseTable p[] = {
		{ &QuickTimeParser::readDefault, MKTAG('d', 'i', 'n', 'f') },
		{ &QuickTimeParser::readLeaf,    MKTAG('d', 'r', 'e', 'f') },
		{ &QuickTimeParser::readDefault, MKTAG('e', 'd', 't', 's') },
		{ &QuickTimeParser::readELST,    MKTAG('e', 'l', 's', 't') },
		{ &QuickTimeParser::readHDLR,    MKTAG('h', 'd', 'l', 'r') },
		{ &QuickTimeParser::readDefault, MKTAG('m', 'd', 'a', 't') },
		{ &QuickTimeParser::readMDHD,    MKTAG('m', 'd', 'h', 'd') },
		{ &QuickTimeParser::readDefault, MKTAG('m', 'd', 'i', 'a') },
		{ &QuickTimeParser::readDefault, MKTAG('m', 'i', 'n', 'f') },
		{ &QuickTimeParser::readMOOV,    MKTAG('m', 'o', 'o', 'v') },
		{ &QuickTimeParser::readMVHD,    MKTAG('m', 'v', 'h', 'd') },
		{ &QuickTimeParser::readLeaf,    MKTAG('s', 'm', 'h', 'd') },
		{ &QuickTimeParser::readDefault, MKTAG('s', 't', 'b', 'l') },
		{ &QuickTimeParser::readSTCO,    MKTAG('s', 't', 'c', 'o') },
		{ &QuickTimeParser::readSTSC,    MKTAG('s', 't', 's', 'c') },
		{ &QuickTimeParser::readSTSD,    MKTAG('s', 't', 's', 'd') },
		{ &QuickTimeParser::readSTSS,    MKTAG('s', 't', 's', 's') },
		{ &QuickTimeParser::readSTSZ,    MKTAG('s', 't', 's', 'z') },
		{ &QuickTimeParser::readSTTS,    MKTAG('s', 't', 't', 's') },
		{ &QuickTimeParser::readTKHD,    MKTAG('t', 'k', 'h', 'd') },
		{ &QuickTimeParser::readTRAK,    MKTAG('t', 'r', 'a', 'k') },
		{ &QuickTimeParser::readLeaf,    MKTAG('u', 'd', 't', 'a') },
		{ &QuickTimeParser::readLeaf,    MKTAG('v', 'm', 'h', 'd') },
		{ &QuickTimeParser::readCMOV,    MKTAG('c', 'm', 'o', 'v') },
		{ &QuickTimeParser::readWAVE,    MKTAG('w', 'a', 'v', 'e') },
		{ &QuickTimeParser::readESDS,    MKTAG('e', 's', 'd', 's') },
		{ 0, 0 }
	};

	_parseTable = p;
}

int QuickTimeParser::readDefault(MOVatom atom) {
	uint32 total_size = 0;
	MOVatom a;
	int err = 0;

	a.offset = atom.offset;

	while(((total_size + 8) < atom.size) && !_fd->eos() && _fd->pos() < _fd->size() && !err) {
		a.size = atom.size;
		a.type = 0;

		if (atom.size >= 8) {
			a.size = _fd->readUint32BE();
			a.type = _fd->readUint32BE();

			// Some QuickTime videos with resource forks have mdat chunks
			// that are of size 0. Adjust it so it's the correct size.
			if (a.type == MKTAG('m', 'd', 'a', 't') && a.size == 0)
				a.size = _fd->size();
		}

		total_size += 8;
		a.offset += 8;
		debug(4, "type: %08x  %.4s  sz: %x %x %x", a.type, tag2str(a.type), a.size, atom.size, total_size);

		if (a.size == 1) { // 64 bit extended size
			warning("64 bit extended size is not supported in QuickTime");
			return -1;
		}

		if (a.size == 0) {
			a.size = atom.size - total_size;
			if (a.size <= 8)
				break;
		}

		uint32 i = 0;

		for (; _parseTable[i].type != 0 && _parseTable[i].type != a.type; i++)
			; // Empty

		if (a.size < 8)
			break;

		a.size -= 8;

		if (_parseTable[i].type == 0) { // skip leaf atoms data
			debug(0, ">>> Skipped [%s]", tag2str(a.type));

			_fd->seek(a.size, SEEK_CUR);
		} else {
			uint32 start_pos = _fd->pos();
			err = (this->*_parseTable[i].func)(a);

			uint32 left = a.size - _fd->pos() + start_pos;

			if (left > 0) // skip garbage at atom end
				_fd->seek(left, SEEK_CUR);
		}

		a.offset += a.size;
		total_size += a.size;
	}

	if (!err && total_size < atom.size)
		_fd->seek(atom.size - total_size, SEEK_SET);

	return err;
}

int QuickTimeParser::readLeaf(MOVatom atom) {
	if (atom.size > 1)
		_fd->seek(atom.size, SEEK_SET);

	return 0;
}

int QuickTimeParser::readMOOV(MOVatom atom) {
	if (readDefault(atom) < 0)
		return -1;

	// We parsed the 'moov' atom, so we don't need anything else
	_foundMOOV = true;
	return 1;
}

int QuickTimeParser::readCMOV(MOVatom atom) {
#ifdef USE_ZLIB
	// Read in the dcom atom
	_fd->readUint32BE();
	if (_fd->readUint32BE() != MKTAG('d', 'c', 'o', 'm'))
		return -1;
	if (_fd->readUint32BE() != MKTAG('z', 'l', 'i', 'b')) {
		warning("Unknown cmov compression type");
		return -1;
	}

	// Read in the cmvd atom
	uint32 compressedSize = _fd->readUint32BE() - 12;
	if (_fd->readUint32BE() != MKTAG('c', 'm', 'v', 'd'))
		return -1;
	uint32 uncompressedSize = _fd->readUint32BE();

	// Read in data
	byte *compressedData = (byte *)malloc(compressedSize);
	_fd->read(compressedData, compressedSize);

	// Create uncompressed stream
	byte *uncompressedData = (byte *)malloc(uncompressedSize);

	// Uncompress the data
	unsigned long dstLen = uncompressedSize;
	if (!Common::uncompress(uncompressedData, &dstLen, compressedData, compressedSize)) {
		warning ("Could not uncompress cmov chunk");
		free(compressedData);
		free(uncompressedData);
		return -1;
	}

	// Load data into a new MemoryReadStream and assign _fd to be that
	Common::SeekableReadStream *oldStream = _fd;
	_fd = new Common::MemoryReadStream(uncompressedData, uncompressedSize, DisposeAfterUse::YES);

	// Read the contents of the uncompressed data
	MOVatom a = { MKTAG('m', 'o', 'o', 'v'), 0, uncompressedSize };
	int err = readDefault(a);

	// Assign the file handle back to the original handle
	free(compressedData);
	delete _fd;
	_fd = oldStream;

	return err;
#else
	warning ("zlib not found, cannot read QuickTime cmov atom");
	return -1;
#endif
}

int QuickTimeParser::readMVHD(MOVatom atom) {
	byte version = _fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	if (version == 1) {
		warning("QuickTime version 1");
		_fd->readUint32BE(); _fd->readUint32BE();
		_fd->readUint32BE(); _fd->readUint32BE();
	} else {
		_fd->readUint32BE(); // creation time
		_fd->readUint32BE(); // modification time
	}

	_timeScale = _fd->readUint32BE(); // time scale
	debug(0, "time scale = %i\n", _timeScale);

	// duration
	_duration = (version == 1) ? (_fd->readUint32BE(), _fd->readUint32BE()) : _fd->readUint32BE();
	_fd->readUint32BE(); // preferred scale

	_fd->readUint16BE(); // preferred volume

	_fd->seek(10, SEEK_CUR); // reserved

	// We only need two values from the movie display matrix. Most of the values are just
	// skipped. xMod and yMod are 16:16 fixed point numbers, the last part of the 3x3 matrix
	// is 2:30.
	uint32 xMod = _fd->readUint32BE();
	_fd->skip(12);
	uint32 yMod = _fd->readUint32BE();
	_fd->skip(16);

	_scaleFactorX = Common::Rational(0x10000, xMod);
	_scaleFactorY = Common::Rational(0x10000, yMod);

	_scaleFactorX.debugPrint(1, "readMVHD(): scaleFactorX =");
	_scaleFactorY.debugPrint(1, "readMVHD(): scaleFactorY =");

	_fd->readUint32BE(); // preview time
	_fd->readUint32BE(); // preview duration
	_fd->readUint32BE(); // poster time
	_fd->readUint32BE(); // selection time
	_fd->readUint32BE(); // selection duration
	_fd->readUint32BE(); // current time
	_fd->readUint32BE(); // next track ID

	return 0;
}

int QuickTimeParser::readTRAK(MOVatom atom) {
	MOVStreamContext *sc = new MOVStreamContext();

	if (!sc)
		return -1;

	sc->codec_type = CODEC_TYPE_MOV_OTHER;
	sc->start_time = 0; // XXX: check
	_streams[_numStreams++] = sc;

	return readDefault(atom);
}

int QuickTimeParser::readTKHD(MOVatom atom) {
	MOVStreamContext *st = _streams[_numStreams - 1];
	byte version = _fd->readByte();

	_fd->readByte(); _fd->readByte();
	_fd->readByte(); // flags
	//
	//MOV_TRACK_ENABLED 0x0001
	//MOV_TRACK_IN_MOVIE 0x0002
	//MOV_TRACK_IN_PREVIEW 0x0004
	//MOV_TRACK_IN_POSTER 0x0008
	//

	if (version == 1) {
		_fd->readUint32BE(); _fd->readUint32BE();
		_fd->readUint32BE(); _fd->readUint32BE();
	} else {
		_fd->readUint32BE(); // creation time
		_fd->readUint32BE(); // modification time
	}

	/* st->id = */_fd->readUint32BE(); // track id (NOT 0 !)
	_fd->readUint32BE(); // reserved
	//st->start_time = 0; // check
	(version == 1) ? (_fd->readUint32BE(), _fd->readUint32BE()) : _fd->readUint32BE(); // highlevel (considering edits) duration in movie timebase
	_fd->readUint32BE(); // reserved
	_fd->readUint32BE(); // reserved

	_fd->readUint16BE(); // layer
	_fd->readUint16BE(); // alternate group
	_fd->readUint16BE(); // volume
	_fd->readUint16BE(); // reserved

	// We only need the two values from the displacement matrix for a track.
	// See readMVHD() for more information.
	uint32 xMod = _fd->readUint32BE();
	_fd->skip(12);
	uint32 yMod = _fd->readUint32BE();
	_fd->skip(16);

	st->scaleFactorX = Common::Rational(0x10000, xMod);
	st->scaleFactorY = Common::Rational(0x10000, yMod);

	st->scaleFactorX.debugPrint(1, "readTKHD(): scaleFactorX =");
	st->scaleFactorY.debugPrint(1, "readTKHD(): scaleFactorY =");

	// these are fixed-point, 16:16
	// uint32 tkWidth = _fd->readUint32BE() >> 16; // track width
	// uint32 tkHeight = _fd->readUint32BE() >> 16; // track height

	return 0;
}

// edit list atom
int QuickTimeParser::readELST(MOVatom atom) {
	MOVStreamContext *st = _streams[_numStreams - 1];

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	st->editCount = _fd->readUint32BE();
	st->editList = new EditListEntry[st->editCount];

	debug(2, "Track %d edit list count: %d", _numStreams - 1, st->editCount);

	for (uint32 i = 0; i < st->editCount; i++){
		st->editList[i].trackDuration = _fd->readUint32BE();
		st->editList[i].mediaTime = _fd->readSint32BE();
		st->editList[i].mediaRate = Common::Rational(_fd->readUint32BE(), 0x10000);
		debugN(3, "\tDuration = %d, Media Time = %d, ", st->editList[i].trackDuration, st->editList[i].mediaTime);
		st->editList[i].mediaRate.debugPrint(3, "Media Rate =");
	}

	if (st->editCount != 1)
		warning("Multiple edit list entries. Things may go awry");

	return 0;
}

int QuickTimeParser::readHDLR(MOVatom atom) {
	MOVStreamContext *st = _streams[_numStreams - 1];

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	// component type
	uint32 ctype = _fd->readUint32BE();
	uint32 type = _fd->readUint32BE(); // component subtype

	debug(0, "ctype= %s (0x%08lx)", tag2str(ctype), (long)ctype);
	debug(0, "stype= %s", tag2str(type));

	if (ctype == MKTAG('m', 'h', 'l', 'r')) // MOV
		debug(0, "MOV detected");
	else if (ctype == 0)
		debug(0, "MPEG-4 detected");

	if (type == MKTAG('v', 'i', 'd', 'e'))
		st->codec_type = CODEC_TYPE_VIDEO;
	else if (type == MKTAG('s', 'o', 'u', 'n'))
		st->codec_type = CODEC_TYPE_AUDIO;

	_fd->readUint32BE(); // component manufacture
	_fd->readUint32BE(); // component flags
	_fd->readUint32BE(); // component flags mask

	if (atom.size <= 24)
		return 0; // nothing left to read

	// .mov: PASCAL string
	byte len = _fd->readByte();
	_fd->seek(len, SEEK_CUR);

	_fd->seek(atom.size - (_fd->pos() - atom.offset), SEEK_CUR);

	return 0;
}

int QuickTimeParser::readMDHD(MOVatom atom) {
	MOVStreamContext *st = _streams[_numStreams - 1];
	byte version = _fd->readByte();

	if (version > 1)
		return 1; // unsupported

	_fd->readByte(); _fd->readByte();
	_fd->readByte(); // flags

	if (version == 1) {
		_fd->readUint32BE(); _fd->readUint32BE();
		_fd->readUint32BE(); _fd->readUint32BE();
	} else {
		_fd->readUint32BE(); // creation time
		_fd->readUint32BE(); // modification time
	}

	st->time_scale = _fd->readUint32BE();
	st->duration = (version == 1) ? (_fd->readUint32BE(), _fd->readUint32BE()) : _fd->readUint32BE(); // duration

	_fd->readUint16BE(); // language
	_fd->readUint16BE(); // quality

	return 0;
}

int QuickTimeParser::readSTSD(MOVatom atom) {
	MOVStreamContext *st = _streams[_numStreams - 1];

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	uint32 entryCount = _fd->readUint32BE();
	st->sampleDescs.resize(entryCount);

	for (uint32 i = 0; i < entryCount; i++) { // Parsing Sample description table
		MOVatom a = { 0, 0, 0 };
		uint32 start_pos = _fd->pos();
		int size = _fd->readUint32BE(); // size
		uint32 format = _fd->readUint32BE(); // data format

		_fd->readUint32BE(); // reserved
		_fd->readUint16BE(); // reserved
		_fd->readUint16BE(); // index

		st->sampleDescs[i] = readSampleDesc(st, format);

		debug(0, "size=%d 4CC= %s codec_type=%d", size, tag2str(format), st->codec_type);

		if (!st->sampleDescs[i]) {
			// other codec type, just skip (rtp, mp4s, tmcd ...)
			_fd->seek(size - (_fd->pos() - start_pos), SEEK_CUR);
		}

		// this will read extra atoms at the end (wave, alac, damr, avcC, SMI ...)
		a.size = size - (_fd->pos() - start_pos);
		if (a.size > 8)
			readDefault(a);
		else if (a.size > 0)
			_fd->seek(a.size, SEEK_CUR);
	}

	return 0;
}

int QuickTimeParser::readSTSC(MOVatom atom) {
	MOVStreamContext *st = _streams[_numStreams - 1];

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	st->sample_to_chunk_sz = _fd->readUint32BE();

	debug(0, "track[%i].stsc.entries = %i", _numStreams - 1, st->sample_to_chunk_sz);

	st->sample_to_chunk = new MOVstsc[st->sample_to_chunk_sz];

	if (!st->sample_to_chunk)
		return -1;

	for (uint32 i = 0; i < st->sample_to_chunk_sz; i++) {
		st->sample_to_chunk[i].first = _fd->readUint32BE() - 1;
		st->sample_to_chunk[i].count = _fd->readUint32BE();
		st->sample_to_chunk[i].id = _fd->readUint32BE();
		//warning("Sample to Chunk[%d]: First = %d, Count = %d", i, st->sample_to_chunk[i].first, st->sample_to_chunk[i].count);
	}

	return 0;
}

int QuickTimeParser::readSTSS(MOVatom atom) {
	MOVStreamContext *st = _streams[_numStreams - 1];

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	st->keyframe_count = _fd->readUint32BE();

	debug(0, "keyframe_count = %d", st->keyframe_count);

	st->keyframes = new uint32[st->keyframe_count];

	if (!st->keyframes)
		return -1;

	for (uint32 i = 0; i < st->keyframe_count; i++) {
		st->keyframes[i] = _fd->readUint32BE() - 1; // Adjust here, the frames are based on 1
		debug(6, "keyframes[%d] = %d", i, st->keyframes[i]);

	}
	return 0;
}

int QuickTimeParser::readSTSZ(MOVatom atom) {
	MOVStreamContext *st = _streams[_numStreams - 1];

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	st->sample_size = _fd->readUint32BE();
	st->sample_count = _fd->readUint32BE();

	debug(5, "sample_size = %d sample_count = %d", st->sample_size, st->sample_count);

	if (st->sample_size)
		return 0; // there isn't any table following

	st->sample_sizes = new uint32[st->sample_count];

	if (!st->sample_sizes)
		return -1;

	for(uint32 i = 0; i < st->sample_count; i++) {
		st->sample_sizes[i] = _fd->readUint32BE();
		debug(6, "sample_sizes[%d] = %d", i, st->sample_sizes[i]);
	}

	return 0;
}

int QuickTimeParser::readSTTS(MOVatom atom) {
	MOVStreamContext *st = _streams[_numStreams - 1];
	uint32 totalSampleCount = 0;

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	st->stts_count = _fd->readUint32BE();
	st->stts_data = new MOVstts[st->stts_count];

	debug(0, "track[%d].stts.entries = %d", _numStreams - 1, st->stts_count);

	for (int32 i = 0; i < st->stts_count; i++) {
		st->stts_data[i].count = _fd->readUint32BE();
		st->stts_data[i].duration = _fd->readUint32BE();

		debug(1, "\tCount = %d, Duration = %d", st->stts_data[i].count, st->stts_data[i].duration);

		totalSampleCount += st->stts_data[i].count;
	}

	st->nb_frames = totalSampleCount;
	return 0;
}

int QuickTimeParser::readSTCO(MOVatom atom) {
	MOVStreamContext *st = _streams[_numStreams - 1];

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	st->chunk_count = _fd->readUint32BE();
	st->chunk_offsets = new uint32[st->chunk_count];

	if (!st->chunk_offsets)
		return -1;

	for (uint32 i = 0; i < st->chunk_count; i++) {
		// WORKAROUND/HACK: The offsets in Riven videos (ones inside the Mohawk archives themselves)
		// have offsets relative to the archive and not the video. This is quite nasty. We subtract
		// the initial offset of the stream to get the correct value inside of the stream.
		st->chunk_offsets[i] = _fd->readUint32BE() - _beginOffset;
	}

	return 0;
}

int QuickTimeParser::readWAVE(MOVatom atom) {
	if (_numStreams < 1)
		return 0;

	MOVStreamContext *st = _streams[_numStreams - 1];

	if (atom.size > (1 << 30))
		return -1;

	if (st->sampleDescs[0]->codecTag == MKTAG('Q', 'D', 'M', '2')) // Read extradata for QDM2
		st->extradata = _fd->readStream(atom.size - 8);
	else if (atom.size > 8)
		return readDefault(atom);
	else
		_fd->skip(atom.size);

	return 0;
}

enum {
	kMP4IODescTag          = 2,
	kMP4ESDescTag          = 3,
	kMP4DecConfigDescTag   = 4,
	kMP4DecSpecificDescTag = 5
};

static int readMP4DescLength(Common::SeekableReadStream *stream) {
	int length = 0;
	int count = 4;

	while (count--) {
		byte c = stream->readByte();
		length = (length << 7) | (c & 0x7f);

		if (!(c & 0x80))
			break;
	}

	return length;
}

static void readMP4Desc(Common::SeekableReadStream *stream, byte &tag, int &length) {
	tag = stream->readByte();
	length = readMP4DescLength(stream);
}

int QuickTimeParser::readESDS(MOVatom atom) {
	if (_numStreams < 1)
		return 0;

	MOVStreamContext *st = _streams[_numStreams - 1];

	_fd->readUint32BE(); // version + flags

	byte tag;
	int length;
	
	readMP4Desc(_fd, tag, length);
	_fd->readUint16BE(); // id
	if (tag == kMP4ESDescTag)
		_fd->readByte(); // priority

	// Check if we've got the Config MPEG-4 header
	readMP4Desc(_fd, tag, length);
	if (tag != kMP4DecConfigDescTag) 
		return 0;

	st->objectTypeMP4 = _fd->readByte();
	_fd->readByte();                      // stream type
	_fd->readUint16BE(); _fd->readByte(); // buffer size
	_fd->readUint32BE();                  // max bitrate
	_fd->readUint32BE();                  // avg bitrate

	// Check if we've got the Specific MPEG-4 header
	readMP4Desc(_fd, tag, length);
	if (tag != kMP4DecSpecificDescTag)
		return 0;

	st->extradata = _fd->readStream(length);

	debug(0, "MPEG-4 object type = %02x", st->objectTypeMP4);
	return 0;
}

void QuickTimeParser::close() {
	for (uint32 i = 0; i < _numStreams; i++)
		delete _streams[i];

	_numStreams = 0;

	if (_disposeFileHandle == DisposeAfterUse::YES)
		delete _fd;

	_fd = 0;
}

QuickTimeParser::SampleDesc::SampleDesc() {
	codecTag = 0;
	bitsPerSample = 0;
}

QuickTimeParser::MOVStreamContext::MOVStreamContext() {
	chunk_count = 0;
	chunk_offsets = 0;
	stts_count = 0;
	stts_data = 0;
	sample_to_chunk_sz = 0;
	sample_to_chunk = 0;
	sample_size = 0;
	sample_count = 0;
	sample_sizes = 0;
	keyframe_count = 0;
	keyframes = 0;
	time_scale = 0;
	width = 0;
	height = 0;
	codec_type = CODEC_TYPE_MOV_OTHER;
	editCount = 0;
	editList = 0;
	extradata = 0;
	nb_frames = 0;
	duration = 0;
	start_time = 0;
	objectTypeMP4 = 0;
}

QuickTimeParser::MOVStreamContext::~MOVStreamContext() {
	delete[] chunk_offsets;
	delete[] stts_data;
	delete[] sample_to_chunk;
	delete[] sample_sizes;
	delete[] keyframes;
	delete[] editList;
	delete extradata;

	for (uint32 i = 0; i < sampleDescs.size(); i++)
		delete sampleDescs[i];
}

} // End of namespace Video
