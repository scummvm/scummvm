/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef AUDIOSTREAM_H
#define AUDIOSTREAM_H

#include "mixer.h"

// TODO:
// * maybe make readIntern return 16.16 or 24.8 fixed point values
//   since MAD (and maybe OggVorbis?) gives us those -> higher quality.
//   The rate converters should be able to deal with those just fine, too.
// * possibly add MADInputStream and VorbisInputStream

/**
 * Generic input stream for the resampling code.
 */
class AudioInputStream {
protected:
	virtual int16 readIntern() = 0;
	virtual void advance() = 0;
public:
	int16 read() { assert(size() > 0); int16 val = readIntern(); advance(); return val; }
	int16 peek() { assert(size() > 0); return readIntern(); }
	virtual int size() = 0;
	bool eof() { return size() <= 0; }
};

class ZeroInputStream : public AudioInputStream {
protected:
	uint _len;
	int16 readIntern() { return 0; }
	void advance() { _len--; }
public:
	ZeroInputStream(uint len) : _len(len) { }
	virtual int size() { return _len; }
};

class MemoryAudioInputStream : public AudioInputStream {
protected:
	const byte *_ptr;
	const byte *_end;
public:
	MemoryAudioInputStream(const byte *ptr, uint len) : _ptr(ptr), _end(ptr+len) { }
};


template<int channels>
class Input8bitSignedStream : public MemoryAudioInputStream {
protected:
	int16 readIntern() { int8 v = (int8)*_ptr; return v << 8; }
	void advance() { _ptr += channels; }
public:
	Input8bitSignedStream(const byte *ptr, int len) : MemoryAudioInputStream(ptr, len) { }
	virtual int size() { return (_end - _ptr) / channels; }
};

template<int channels>
class Input8bitUnsignedStream : public MemoryAudioInputStream {
protected:
	int16 readIntern() { int8 v = (int8)(*_ptr ^ 0x80); return v << 8; }
	void advance() { _ptr += channels; }
public:
	Input8bitUnsignedStream(const byte *ptr, int len) : MemoryAudioInputStream(ptr, len) { }
	virtual int size() { return (_end - _ptr) / channels; }
};

template<int channels>
class Input16bitSignedStream : public MemoryAudioInputStream {
protected:
	int16 readIntern() { return (int16)READ_BE_UINT16(_ptr); }
	void advance() { _ptr += 2*channels; }
public:
	Input16bitSignedStream(const byte *ptr, int len) : MemoryAudioInputStream(ptr, len) { }
	virtual int size() { return (_end - _ptr) / (2 * channels); }
};

template<int channels>
class Input16bitUnsignedStream : public MemoryAudioInputStream {
protected:
	int16 readIntern() { return (int16)(READ_BE_UINT16(_ptr) ^ 0x8000); }
	void advance() { _ptr += 2*channels; }
public:
	Input16bitUnsignedStream(const byte *ptr, int len) : MemoryAudioInputStream(ptr, len) { }
	virtual int size() { return (_end - _ptr) / (2 * channels); }
};


template<int channels>
static AudioInputStream *makeInputStream(const byte *ptr, uint32 len, bool isUnsigned, bool is16Bit) {
	if (isUnsigned) {
		if (is16Bit)
			return new Input16bitUnsignedStream<channels>(ptr, len);
		else
			return new Input8bitUnsignedStream<channels>(ptr, len);
	} else {
		if (is16Bit)
			return new Input16bitSignedStream<channels>(ptr, len);
		else
			return new Input8bitSignedStream<channels>(ptr, len);
	}
}

static inline AudioInputStream *makeInputStream(byte _flags, const byte *ptr, uint32 len) {
	if (_flags & SoundMixer::FLAG_STEREO)
		return makeInputStream<2>(ptr, len, _flags & SoundMixer::FLAG_UNSIGNED, _flags & SoundMixer::FLAG_16BITS);
	else
		return makeInputStream<1>(ptr, len, _flags & SoundMixer::FLAG_UNSIGNED, _flags & SoundMixer::FLAG_16BITS);
}

#endif
