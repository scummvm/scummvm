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

#include "audiostream.h"
#include "mixer.h"
#include "common/engine.h"


template<bool is16Bit, bool isUnsigned>
static inline int16 readSample(const byte *ptr) {
	if (is16Bit) {
		if (isUnsigned)
			return (int16)(READ_BE_UINT16(ptr) ^ 0x8000);
		else
			return (int16)READ_BE_UINT16(ptr);
	} else {
		if (isUnsigned)
			return (int8)(*ptr ^ 0x80) << 8;
		else
			return (int8)*ptr << 8;
	}
}

#pragma mark -
#pragma mark --- LinearMemoryStream ---
#pragma mark -


template<bool stereo, bool is16Bit, bool isUnsigned>
class LinearMemoryStream : public AudioInputStream {
protected:
	const byte *_ptr;
	const byte *_end;

public:
	LinearMemoryStream(const byte *ptr, uint len)
		: _ptr(ptr), _end(ptr+len) {
		if (stereo)	// Stereo requires even sized data
			assert(len % 2 == 0);
	}
	int16 read() {
		assert(_ptr < _end);
		int16 val = readSample<is16Bit, isUnsigned>(_ptr);
		_ptr += (is16Bit ? 2 : 1);
		return val;
	}
	int size() const {
		return (_end - _ptr) / (is16Bit ? 2 : 1);
	}
	bool isStereo() const {
		return stereo;
	}
};


#pragma mark -
#pragma mark --- WrappedMemoryStream ---
#pragma mark -


// Wrapped memory stream, to be used by the ChannelStream class (and possibly others?)
template<bool stereo, bool is16Bit, bool isUnsigned>
class WrappedMemoryStream : public WrappedAudioInputStream {
protected:
	byte *_bufferStart;
	byte *_bufferEnd;
	byte *_pos;
	byte *_end;
	
public:
	WrappedMemoryStream(uint bufferSize);
	~WrappedMemoryStream() { free(_bufferStart); }
	int16 read();
	int size() const;
	bool isStereo() const {
		return stereo;
	}

	void append(const byte *data, uint32 len);
};


template<bool stereo, bool is16Bit, bool isUnsigned>
WrappedMemoryStream<stereo, is16Bit, isUnsigned>::WrappedMemoryStream(uint bufferSize) {
	if (stereo)	// Stereo requires an even sized buffer
		assert(bufferSize % 2 == 0);
	_bufferStart = (byte *)malloc(bufferSize);
	_pos = _end = _bufferStart;
	_bufferEnd = _bufferStart + bufferSize;
}

template<bool stereo, bool is16Bit, bool isUnsigned>
int16 WrappedMemoryStream<stereo, is16Bit, isUnsigned>::read() {
	assert(_pos != _end);
	int16 val = readSample<is16Bit, isUnsigned>(_pos);
	_pos += (is16Bit ? 2 : 1);

	// Wrap around?
	if (_pos >= _bufferEnd)
		_pos = _pos - (_bufferEnd - _bufferStart);

	return val;
}

template<bool stereo, bool is16Bit, bool isUnsigned>
int WrappedMemoryStream<stereo, is16Bit, isUnsigned>::size() const {
	int len = _end - _pos;
	if (len < 0)
		len += (_bufferEnd - _bufferStart);
	return len / (is16Bit ? 2 : 1);
}

template<bool stereo, bool is16Bit, bool isUnsigned>
void WrappedMemoryStream<stereo, is16Bit, isUnsigned>::append(const byte *data, uint32 len) {
	if (_end + len > _bufferEnd) {
		// Wrap-around case
		uint32 size_to_end_of_buffer = _bufferEnd - _end;
		len -= size_to_end_of_buffer;
		if ((_end < _pos) || (_bufferStart + len >= _pos)) {
			debug(2, "WrappedMemoryStream: buffer overflow (A)");
			return;
		}
		memcpy(_end, data, size_to_end_of_buffer);
		memcpy(_bufferStart, data + size_to_end_of_buffer, len);
		_end = _bufferStart + len;
	} else {
		if ((_end < _pos) && (_end + len >= _pos)) {
			debug(2, "WrappedMemoryStream: buffer overflow (B)");
			return;
		}
		memcpy(_end, data, len);
		_end += len;
	}
}


#pragma mark -
#pragma mark --- Input stream factories ---
#pragma mark -


template<bool stereo>
static AudioInputStream *makeLinearInputStream(const byte *ptr, uint32 len, bool isUnsigned, bool is16Bit) {
	if (isUnsigned) {
		if (is16Bit)
			return new LinearMemoryStream<stereo, true, true>(ptr, len);
		else
			return new LinearMemoryStream<stereo, false, true>(ptr, len);
	} else {
		if (is16Bit)
			return new LinearMemoryStream<stereo, true, false>(ptr, len);
		else
			return new LinearMemoryStream<stereo, false, false>(ptr, len);
	}
}


template<bool stereo>
static WrappedAudioInputStream *makeWrappedInputStream(uint32 len, bool isUnsigned, bool is16Bit) {
	if (isUnsigned) {
		if (is16Bit)
			return new WrappedMemoryStream<stereo, true, true>(len);
		else
			return new WrappedMemoryStream<stereo, false, true>(len);
	} else {
		if (is16Bit)
			return new WrappedMemoryStream<stereo, true, false>(len);
		else
			return new WrappedMemoryStream<stereo, false, false>(len);
	}
}


AudioInputStream *makeLinearInputStream(byte _flags, const byte *ptr, uint32 len) {
	if (_flags & SoundMixer::FLAG_STEREO) {
		return makeLinearInputStream<true>(ptr, len, _flags & SoundMixer::FLAG_UNSIGNED, _flags & SoundMixer::FLAG_16BITS);
	} else {
		return makeLinearInputStream<false>(ptr, len, _flags & SoundMixer::FLAG_UNSIGNED, _flags & SoundMixer::FLAG_16BITS);
	}
}

WrappedAudioInputStream *makeWrappedInputStream(byte _flags, uint32 len) {
	if (_flags & SoundMixer::FLAG_STEREO) {
		return makeWrappedInputStream<true>(len, _flags & SoundMixer::FLAG_UNSIGNED, _flags & SoundMixer::FLAG_16BITS);
	} else {
		return makeWrappedInputStream<false>(len, _flags & SoundMixer::FLAG_UNSIGNED, _flags & SoundMixer::FLAG_16BITS);
	}
}
