// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#include "../stdafx.h"
#include "../debug.h"
#include "mixer.h"
#include "audiostream.h"


// This used to be an inline template function, but
// buggy template function handling in MSVC6 forced
// us to go with the macro approach. So far this is
// the only template function that MSVC6 seemed to
// compile incorrectly. Knock on wood.
#define READSAMPLE(is16Bit, isUnsigned, ptr) \
	((is16Bit ? READ_BE_UINT16(ptr) : (*ptr << 8)) ^ (isUnsigned ? 0x8000 : 0))

template<bool stereo, bool is16Bit, bool isUnsigned>
class LinearMemoryStream : public AudioInputStream {
protected:
	const byte *_ptr;
	const byte *_end;
	const byte *_loopPtr;
	const byte *_loopEnd;

	inline int16 readIntern() {
		//assert(_ptr < _end);
		int16 val = READSAMPLE(is16Bit, isUnsigned, _ptr);
		_ptr += (is16Bit ? 2 : 1);
		if (_loopPtr && eosIntern()) {
			_ptr = _loopPtr;
			_end = _loopEnd;
		}
		return val;
	}
	inline bool eosIntern() const	{ return _ptr >= _end; };
public:
	LinearMemoryStream(const byte *ptr, uint len, uint loopOffset, uint loopLen)
		: _ptr(ptr), _end(ptr+len), _loopPtr(0), _loopEnd(0) {

		// Verify the buffer sizes are sane
		if (is16Bit && stereo)
			assert((len & 3) == 0 && (loopLen & 3) == 0);
		else if (is16Bit || stereo)
			assert((len & 1) == 0 && (loopLen & 1) == 0);

		if (loopLen) {
			_loopPtr = _ptr + loopOffset;
			_loopEnd = _loopPtr + loopLen;
		}
		if (stereo)	// Stereo requires even sized data
			assert(len % 2 == 0);
	}
	int readBuffer(int16 *buffer, const int numSamples);

	int16 read()				{ return readIntern(); }
	bool eos() const			{ return eosIntern(); }
	bool isStereo() const		{ return stereo; }
};

template<bool stereo, bool is16Bit, bool isUnsigned>
int LinearMemoryStream<stereo, is16Bit, isUnsigned>::readBuffer(int16 *buffer, const int numSamples) {
	int samples = 0;
	while (samples < numSamples && !eosIntern()) {
		const int len = MIN(numSamples, samples + (int)(_end - _ptr) / (is16Bit ? 2 : 1));
		while (samples < len) {
			*buffer++ = READSAMPLE(is16Bit, isUnsigned, _ptr);
			_ptr += (is16Bit ? 2 : 1);
			samples++;
		}
		// Loop, if looping was specified
		if (_loopPtr && eosIntern()) {
			_ptr = _loopPtr;
			_end = _loopEnd;
		}
	}
	return samples;
}

// Wrapped memory stream, to be used by the ChannelStream class (and possibly others?)
template<bool stereo, bool is16Bit, bool isUnsigned>
class WrappedMemoryStream : public WrappedAudioInputStream {
protected:
	byte *_bufferStart;
	byte *_bufferEnd;
	byte *_pos;
	byte *_end;

	inline int16 readIntern();
	inline bool eosIntern() const { return _end == _pos; };
public:
	WrappedMemoryStream(uint bufferSize);
	~WrappedMemoryStream()		{ free(_bufferStart); }
	int readBuffer(int16 *buffer, const int numSamples);

	int16 read()				{ return readIntern(); }
	bool eos() const			{ return eosIntern(); }
	bool isStereo() const		{ return stereo; }

	void append(const byte *data, uint32 len);
};


template<bool stereo, bool is16Bit, bool isUnsigned>
WrappedMemoryStream<stereo, is16Bit, isUnsigned>::WrappedMemoryStream(uint bufferSize) {

	// Verify the buffer size is sane
	if (is16Bit && stereo)
		assert((bufferSize & 3) == 0);
	else if (is16Bit || stereo)
		assert((bufferSize & 1) == 0);

	_bufferStart = (byte *)malloc(bufferSize);
	_pos = _end = _bufferStart;
	_bufferEnd = _bufferStart + bufferSize;
}

template<bool stereo, bool is16Bit, bool isUnsigned>
inline int16 WrappedMemoryStream<stereo, is16Bit, isUnsigned>::readIntern() {
	//assert(_pos != _end);
	int16 val = READSAMPLE(is16Bit, isUnsigned, _pos);
	_pos += (is16Bit ? 2 : 1);

	// Wrap around?
	if (_pos >= _bufferEnd)
		_pos = _pos - (_bufferEnd - _bufferStart);

	return val;
}

template<bool stereo, bool is16Bit, bool isUnsigned>
int WrappedMemoryStream<stereo, is16Bit, isUnsigned>::readBuffer(int16 *buffer, const int numSamples) {
	int samples = 0;
	while (samples < numSamples && !eosIntern()) {
		const byte *endMarker = (_pos > _end) ? _bufferEnd : _end;
		const int len = MIN(numSamples, samples + (int)(endMarker - _pos) / (is16Bit ? 2 : 1));
		while (samples < len) {
			*buffer++ = READSAMPLE(is16Bit, isUnsigned, _pos);
			_pos += (is16Bit ? 2 : 1);
			samples++;
		}
		// Wrap around?
		if (_pos >= _bufferEnd)
			_pos = _pos - (_bufferEnd - _bufferStart);
	}
	return samples;
}

template<bool stereo, bool is16Bit, bool isUnsigned>
void WrappedMemoryStream<stereo, is16Bit, isUnsigned>::append(const byte *data, uint32 len) {

	// Verify the buffer size is sane
	if (is16Bit && stereo)
		assert((len & 3) == 0);
	else if (is16Bit || stereo)
		assert((len & 1) == 0);

	if (_end + len > _bufferEnd) {
		// Wrap-around case
		uint32 size_to_end_of_buffer = _bufferEnd - _end;
		len -= size_to_end_of_buffer;
		if ((_end < _pos) || (_bufferStart + len >= _pos)) {
			warning("WrappedMemoryStream: buffer overflow (A)");
			return;
		}
		memcpy(_end, data, size_to_end_of_buffer);
		memcpy(_bufferStart, data + size_to_end_of_buffer, len);
		_end = _bufferStart + len;
	} else {
		if ((_end < _pos) && (_end + len >= _pos)) {
			warning("WrappedMemoryStream: buffer overflow (B)");
			return;
		}
		memcpy(_end, data, len);
		_end += len;
	}
}

template<bool stereo>
static AudioInputStream *makeLinearInputStream(const byte *ptr, uint32 len, bool is16Bit, bool isUnsigned, uint loopOffset, uint loopLen) {
	if (isUnsigned) {
		if (is16Bit)
			return new LinearMemoryStream<stereo, true, true>(ptr, len, loopOffset, loopLen);
		else
			return new LinearMemoryStream<stereo, false, true>(ptr, len, loopOffset, loopLen);
	} else {
		if (is16Bit)
			return new LinearMemoryStream<stereo, true, false>(ptr, len, loopOffset, loopLen);
		else
			return new LinearMemoryStream<stereo, false, false>(ptr, len, loopOffset, loopLen);
	}
}

template<bool stereo>
static WrappedAudioInputStream *makeWrappedInputStream(uint32 len, bool is16Bit, bool isUnsigned) {
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

AudioInputStream *makeLinearInputStream(byte _flags, const byte *ptr, uint32 len, uint loopOffset, uint loopLen) {
	const bool is16Bit = (_flags & SoundMixer::FLAG_16BITS) != 0;
	const bool isUnsigned = (_flags & SoundMixer::FLAG_UNSIGNED) != 0;
	if (_flags & SoundMixer::FLAG_STEREO) {
		return makeLinearInputStream<true>(ptr, len, is16Bit, isUnsigned, loopOffset, loopLen);
	} else {
		return makeLinearInputStream<false>(ptr, len, is16Bit, isUnsigned, loopOffset, loopLen);
	}
}

WrappedAudioInputStream *makeWrappedInputStream(byte _flags, uint32 len) {
	const bool is16Bit = (_flags & SoundMixer::FLAG_16BITS) != 0;
	const bool isUnsigned = (_flags & SoundMixer::FLAG_UNSIGNED) != 0;
	if (_flags & SoundMixer::FLAG_STEREO) {
		return makeWrappedInputStream<true>(len, is16Bit, isUnsigned);
	} else {
		return makeWrappedInputStream<false>(len, is16Bit, isUnsigned);
	}
}
