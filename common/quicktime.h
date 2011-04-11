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

#ifndef COMMON_QUICKTIME_H
#define COMMON_QUICKTIME_H

#include "common/array.h"
#include "common/scummsys.h"
#include "common/stream.h"
#include "common/rational.h"

namespace Common {
	class MacResManager;

/**
 * Parser for QuickTime/MPEG-4 files.
 *
 * File parser used in engines:
 *  - groovie
 *  - mohawk
 *  - sci
 */
class QuickTimeParser {
public:
	QuickTimeParser();
	virtual ~QuickTimeParser();

	/**
	 * Load a QuickTime file
	 * @param filename	the filename to load
	 */
	bool parseFile(const Common::String &filename);

	/**
	 * Load a QuickTime file from a SeekableReadStream
	 * @param stream	the stream to load
	 * @param disposeFileHandle whether to delete the stream after use
	 */
	bool parseStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeFileHandle = DisposeAfterUse::YES);

	/**
	 * Close a QuickTime file
	 */
	void close();

	/**
	 * Set the beginning offset of the video so we can modify the offsets in the stco
	 * atom of videos inside the Mohawk archives
	 * @param the beginning offset of the video
	 */
	void setChunkBeginOffset(uint32 offset) { _beginOffset = offset; }

	bool isOpen() const { return _fd != 0; }

protected:
	// This is the file handle from which data is read from. It can be the actual file handle or a decompressed stream.
	Common::SeekableReadStream *_fd;

	DisposeAfterUse::Flag _disposeFileHandle;

	struct MOVatom {
		uint32 type;
		uint32 offset;
		uint32 size;
	};

	struct ParseTable {
		int (QuickTimeParser::*func)(MOVatom atom);
		uint32 type;
	};

	struct MOVstts {
		int count;
		int duration;
	};

	struct MOVstsc {
		uint32 first;
		uint32 count;
		uint32 id;
	};

	struct EditListEntry {
		uint32 trackDuration;
		int32 mediaTime;
		Common::Rational mediaRate;
	};

	struct SampleDesc {
		SampleDesc();
		virtual ~SampleDesc() {}

		uint32 codecTag;
		uint16 bitsPerSample;
	};

	enum CodecType {
		CODEC_TYPE_MOV_OTHER,
		CODEC_TYPE_VIDEO,
		CODEC_TYPE_AUDIO
	};

	struct MOVStreamContext {
		MOVStreamContext();
		~MOVStreamContext();

		uint32 chunk_count;
		uint32 *chunk_offsets;
		int stts_count;
		MOVstts *stts_data;
		uint32 sample_to_chunk_sz;
		MOVstsc *sample_to_chunk;
		uint32 sample_size;
		uint32 sample_count;
		uint32 *sample_sizes;
		uint32 keyframe_count;
		uint32 *keyframes;
		int32 time_scale;

		uint16 width;
		uint16 height;
		CodecType codec_type;

		Common::Array<SampleDesc *> sampleDescs;

		uint32 editCount;
		EditListEntry *editList;

		Common::SeekableReadStream *extradata;

		uint32 nb_frames;
		uint32 duration;
		uint32 start_time;
		Common::Rational scaleFactorX;
		Common::Rational scaleFactorY;

		byte objectTypeMP4;
	};

	virtual SampleDesc *readSampleDesc(MOVStreamContext *st, uint32 format) = 0;

	const ParseTable *_parseTable;
	bool _foundMOOV;
	uint32 _timeScale;
	uint32 _duration;
	uint32 _numStreams;
	Common::Rational _scaleFactorX;
	Common::Rational _scaleFactorY;
	MOVStreamContext *_streams[20];
	uint32 _beginOffset;
	Common::MacResManager *_resFork;

	void initParseTable();
	void init();

	int readDefault(MOVatom atom);
	int readLeaf(MOVatom atom);
	int readELST(MOVatom atom);
	int readHDLR(MOVatom atom);
	int readMDHD(MOVatom atom);
	int readMOOV(MOVatom atom);
	int readMVHD(MOVatom atom);
	int readTKHD(MOVatom atom);
	int readTRAK(MOVatom atom);
	int readSTCO(MOVatom atom);
	int readSTSC(MOVatom atom);
	int readSTSD(MOVatom atom);
	int readSTSS(MOVatom atom);
	int readSTSZ(MOVatom atom);
	int readSTTS(MOVatom atom);
	int readCMOV(MOVatom atom);
	int readWAVE(MOVatom atom);
	int readESDS(MOVatom atom);
};

} // End of namespace Common

#endif
