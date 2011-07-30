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
 */

/*
 * This code is based on original Soltys source code
 * Copyright (c) 1994-1995 Janus B. Wisniewski and L.K. Avalon
 */

#ifndef __CGE_WAV__
#define __CGE_WAV__

#include "cge/general.h"

namespace CGE {

#define WAVE_FORMAT_PCM     0x0001
#define IBM_FORMAT_MULAW    0x0101
#define IBM_FORMAT_ALAW     0x0102
#define IBM_FORMAT_ADPCM    0x0103

typedef char FourCC[4];             // Four-character code

class ChunkId { // Chunk type identifier
	union {
		FourCC _text;
		uint32 _id;
	};
protected:
	static XFile *ckFile;
public:
	ChunkId(FourCC text) {
		memcpy(_text, text, sizeof(_text));
	}
	ChunkId(uint32 d) {
		_id = d;
	}
	ChunkId(XFile *xf) {
		(ckFile = xf)->read(_text, sizeof(_text));
	}
	bool operator !=(ChunkId &X) {
		return _id != X._id;
	}
	bool operator ==(ChunkId &X) {
		return _id == X._id;
	}
	const char *name();
};


class CkHea : public ChunkId {
protected:
	uint32 _ckSize;        // Chunk size field (size of ckData)
public:
	CkHea(XFile *xf) : ChunkId(xf) {
		XRead(xf, &_ckSize);
	}
	CkHea(char id[]) : ChunkId(id), _ckSize(0) { }
	void skip();
	uint32 size() {
		return _ckSize;
	}
};


class FmtCk : public CkHea {
	struct Wav {
		uint16 _wFormatTag;         // Format category
		uint16 _wChannels;          // Number of channels
		uint32 _dwSamplesPerSec;    // Sampling rate
		uint32 _dwAvgBytesPerSec;   // For buffer estimation
		uint16 _wBlockAlign;        // Data block size
	} _wav;

	union {
		struct {
			uint16 _wBitsPerSample;      // Sample size
		} _pcm;
	};
public:
	FmtCk(CkHea &hea);
	inline  uint16 channels() {
		return _wav._wChannels;
	}
	inline uint32 smplRate() {
		return _wav._dwSamplesPerSec;
	}
	inline uint32 byteRate() {
		return _wav._dwAvgBytesPerSec;
	}
	inline uint16 blckSize() {
		return _wav._wBlockAlign;
	}
	inline uint16 smplSize() {
		return _pcm._wBitsPerSample;
	}
};


class DataCk : public CkHea {
	bool _ef;
	uint8 *_buf;
public:
	DataCk(CkHea &hea);
	DataCk(int first, int last);
	~DataCk();
	inline uint8 *addr() {
		return _buf;
	}
};


extern ChunkId _riff;
extern ChunkId _wave;
extern ChunkId _fmt;
extern ChunkId _data;


DataCk *loadWave(XFile *file);

} // End of namespace CGE

#endif
