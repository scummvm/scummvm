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

template<bool stereo, int sampleSize>
class LinearMemoryStream : public AudioInputStream {
protected:
	const byte *_ptr;
	const byte *_end;
	void advance() { _ptr += sampleSize; }
public:
	LinearMemoryStream(const byte *ptr, uint len) : _ptr(ptr), _end(ptr+len) { }
	virtual int size() const { return (_end - _ptr) / sampleSize; }
	virtual bool isStereo() const { return stereo; }
};


#pragma mark -


template<bool stereo, int sampleSize>
WrappedMemoryStream<stereo, sampleSize>::WrappedMemoryStream(const byte *buffer, uint bufferSize)
	: _bufferStart(buffer), _bufferEnd(buffer+bufferSize) {
	if (stereo)	// Stereo requires an even sized buffer
		assert(bufferSize % 2 == 0);
}

template<bool stereo, int sampleSize>
void WrappedMemoryStream<stereo, sampleSize>::advance() {
	_pos += sampleSize;
	// Wrap around?
	if (_pos >= _bufferEnd)
		_pos = _pos - (_bufferEnd - _bufferStart);
}

template<bool stereo, int sampleSize>
int WrappedMemoryStream<stereo, sampleSize>::size() const {
	int len = _end - _pos;
	if (len < 0)
		len += (_bufferEnd - _bufferStart);
	return len / sampleSize;
}

template<bool stereo, int sampleSize>
void WrappedMemoryStream<stereo, sampleSize>::append(const byte *data, uint32 len) {
	if (_end + len > _bufferEnd) {
		// Wrap-around case
		uint32 size_to_end_of_buffer = _bufferEnd - _end;
		len -= size_to_end_of_buffer;
		if ((_end < _pos) || (_bufferStart + len >= _pos)) {
			debug(2, "WrappedMemoryStream: buffer overflow (A)");
			return;
		}
		memcpy(_end, (byte*)data, size_to_end_of_buffer);
		memcpy(_bufferStart, (byte *)data + size_to_end_of_buffer, len);
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


template<bool stereo, class T = class LinearMemoryStream<stereo, 1> >
class Input8bitSignedStream : public T {
protected:
	int16 readIntern() { int8 v = (int8)*_ptr; return v << 8; }
public:
	Input8bitSignedStream(const byte *ptr, int len) : T(ptr, len) { }
};

template<bool stereo, class T = class LinearMemoryStream<stereo, 1> >
class Input8bitUnsignedStream : public T {
protected:
	int16 readIntern() { int8 v = (int8)(*_ptr ^ 0x80); return v << 8; }
public:
	Input8bitUnsignedStream(const byte *ptr, int len) : T(ptr, len) { }
};

template<bool stereo, class T = class LinearMemoryStream<stereo, 2> >
class Input16bitSignedStream : public T {
protected:
	int16 readIntern() { return (int16)READ_BE_UINT16(_ptr); }
public:
	Input16bitSignedStream(const byte *ptr, int len) : T(ptr, len) { }
};

template<bool stereo, class T = class LinearMemoryStream<stereo, 2> >
class Input16bitUnsignedStream : public T {
protected:
	int16 readIntern() { return (int16)(READ_BE_UINT16(_ptr) ^ 0x8000); }
public:
	Input16bitUnsignedStream(const byte *ptr, int len) : T(ptr, len) { }
};


template<bool stereo>
static AudioInputStream *makeInputStream(const byte *ptr, uint32 len, bool isUnsigned, bool is16Bit) {
	if (isUnsigned) {
		if (is16Bit)
			return new Input16bitUnsignedStream<stereo>(ptr, len);
		else
			return new Input8bitUnsignedStream<stereo>(ptr, len);
	} else {
		if (is16Bit)
			return new Input16bitSignedStream<stereo>(ptr, len);
		else
			return new Input8bitSignedStream<stereo>(ptr, len);
	}
}


AudioInputStream *makeInputStream(byte _flags, const byte *ptr, uint32 len) {
	if (_flags & SoundMixer::FLAG_STEREO)
		return makeInputStream<true>(ptr, len, _flags & SoundMixer::FLAG_UNSIGNED, _flags & SoundMixer::FLAG_16BITS);
	else
		return makeInputStream<false>(ptr, len, _flags & SoundMixer::FLAG_UNSIGNED, _flags & SoundMixer::FLAG_16BITS);
}
