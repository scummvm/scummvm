// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
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

#include "stdafx.h"
#include "debug.h"
#include "driver.h"

#include "mixer/mixer.h"
#include "mixer/audiostream.h"

#define READSAMPLE(is16Bit, isUnsigned, ptr) \
	((is16Bit ? READ_BE_UINT16(ptr) : (*ptr << 8)) ^ (isUnsigned ? 0x8000 : 0))

#define READ_ENDIAN_SAMPLE(is16Bit, isUnsigned, ptr, isLE) \
	((is16Bit ? (isLE ? READ_LE_UINT16(ptr) : READ_BE_UINT16(ptr)) : (*ptr << 8)) ^ (isUnsigned ? 0x8000 : 0))

template<bool stereo, bool is16Bit, bool isUnsigned, bool isLE>
class LinearMemoryStream : public AudioStream {
protected:
	const byte *_ptr;
	const byte *_end;
	const byte *_loopPtr;
	const byte *_loopEnd;
	const int _rate;
	const byte *_origPtr;

	inline bool eosIntern() const	{ return _ptr >= _end; };
public:
	LinearMemoryStream(int rate, const byte *ptr, uint len, uint loopOffset, uint loopLen, bool autoFreeMemory)
		: _ptr(ptr), _end(ptr+len), _loopPtr(0), _loopEnd(0), _rate(rate) {

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
		
		_origPtr = autoFreeMemory ? ptr : 0;
	}
	~LinearMemoryStream() {
		free(const_cast<byte *>(_origPtr));
	}
	int readBuffer(int16 *buffer, const int numSamples);

	bool isStereo() const		{ return stereo; }
	bool endOfData() const		{ return eosIntern(); }

	int getRate() const			{ return _rate; }
};

template<bool stereo, bool is16Bit, bool isUnsigned, bool isLE>
int LinearMemoryStream<stereo, is16Bit, isUnsigned, isLE>::readBuffer(int16 *buffer, const int numSamples) {
	int samples = 0;
	while (samples < numSamples && !eosIntern()) {
		const int len = MIN(numSamples, samples + (int)(_end - _ptr) / (is16Bit ? 2 : 1));
		while (samples < len) {
			*buffer++ = READ_ENDIAN_SAMPLE(is16Bit, isUnsigned, _ptr, isLE);
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

/**
 * Wrapped memory stream.
 */
template<bool stereo, bool is16Bit, bool isUnsigned, bool isLE>
class AppendableMemoryStream : public AppendableAudioStream {
protected:
	MutexRef _mutex;

	byte *_bufferStart;
	byte *_bufferEnd;
	byte *_pos;
	byte *_end;
	bool _finalized;
	const int _rate;

	inline bool eosIntern() const { return _end == _pos; };
public:
	AppendableMemoryStream(int rate, uint bufferSize);
	~AppendableMemoryStream();
	int readBuffer(int16 *buffer, const int numSamples);

	bool isStereo() const		{ return stereo; }
	bool endOfStream() const	{ return _finalized && eosIntern(); }
	bool endOfData() const		{ return eosIntern(); }

	int getRate() const			{ return _rate; }

	void append(const byte *data, uint32 len);
	void finish()				{ _finalized = true; }
};

template<bool stereo, bool is16Bit, bool isUnsigned, bool isLE>
AppendableMemoryStream<stereo, is16Bit, isUnsigned, isLE>::AppendableMemoryStream(int rate, uint bufferSize)
	: _finalized(false), _rate(rate) {

	// Verify the buffer size is sane
	if (is16Bit && stereo)
		assert((bufferSize & 3) == 0);
	else if (is16Bit || stereo)
		assert((bufferSize & 1) == 0);

	_bufferStart = (byte *)malloc(bufferSize);
	_pos = _end = _bufferStart;
	_bufferEnd = _bufferStart + bufferSize;

	_mutex = g_driver->createMutex();
}

template<bool stereo, bool is16Bit, bool isUnsigned, bool isLE>
AppendableMemoryStream<stereo, is16Bit, isUnsigned, isLE>::~AppendableMemoryStream() {
	free(_bufferStart);
	g_driver->deleteMutex(_mutex);
}

template<bool stereo, bool is16Bit, bool isUnsigned, bool isLE>
int AppendableMemoryStream<stereo, is16Bit, isUnsigned, isLE>::readBuffer(int16 *buffer, const int numSamples) {
	StackLock lock(_mutex);

	int samples = 0;
	while (samples < numSamples && !eosIntern()) {
		// Wrap around?
		if (_pos >= _bufferEnd)
			_pos = _pos - (_bufferEnd - _bufferStart);

		const byte *endMarker = (_pos > _end) ? _bufferEnd : _end;
		const int len = MIN(numSamples, samples + (int)(endMarker - _pos) / (is16Bit ? 2 : 1));
		while (samples < len) {
			*buffer++ = READ_ENDIAN_SAMPLE(is16Bit, isUnsigned, _pos, isLE);
			_pos += (is16Bit ? 2 : 1);
			samples++;
		}
	}

	return samples;
}

template<bool stereo, bool is16Bit, bool isUnsigned, bool isLE>
void AppendableMemoryStream<stereo, is16Bit, isUnsigned, isLE>::append(const byte *data, uint32 len) {
	StackLock lock(_mutex);

	// Verify the buffer size is sane
	if (is16Bit && stereo)
		assert((len & 3) == 0);
	else if (is16Bit || stereo)
		assert((len & 1) == 0);
	
	// Verify that the stream has not yet been finalized (by a call to finish())
	assert(!_finalized);

	if (_end + len > _bufferEnd) {
		// Wrap-around case
		uint32 size_to_end_of_buffer = _bufferEnd - _end;
		len -= size_to_end_of_buffer;
		if ((_end < _pos) || (_bufferStart + len >= _pos)) {
			warning("AppendableMemoryStream: buffer overflow (A)");
			return;
		}
		memcpy(_end, data, size_to_end_of_buffer);
		memcpy(_bufferStart, data + size_to_end_of_buffer, len);
		_end = _bufferStart + len;
	} else {
		if ((_end < _pos) && (_end + len >= _pos)) {
			warning("AppendableMemoryStream: buffer overflow (B)");
			return;
		}
		memcpy(_end, data, len);
		_end += len;
	}
}

#define MAKE_LINEAR(STEREO, UNSIGNED) \
		if (is16Bit) { \
			if (isLE) \
				return new LinearMemoryStream<STEREO, true, UNSIGNED, true>(rate, ptr, len, loopOffset, loopLen, autoFree); \
			else  \
				return new LinearMemoryStream<STEREO, true, UNSIGNED, false>(rate, ptr, len, loopOffset, loopLen, autoFree); \
		} else \
			return new LinearMemoryStream<STEREO, false, UNSIGNED, false>(rate, ptr, len, loopOffset, loopLen, autoFree)

AudioStream *makeLinearInputStream(int rate, byte _flags, const byte *ptr, uint32 len, uint loopOffset, uint loopLen) {
	const bool isStereo   = (_flags & SoundMixer::FLAG_STEREO) != 0;
	const bool is16Bit    = (_flags & SoundMixer::FLAG_16BITS) != 0;
	const bool isUnsigned = (_flags & SoundMixer::FLAG_UNSIGNED) != 0;
	const bool isLE       = (_flags & SoundMixer::FLAG_LITTLE_ENDIAN) != 0;
	const bool autoFree   = (_flags & SoundMixer::FLAG_AUTOFREE) != 0;
	
	if (isStereo) {
		if (isUnsigned) {
			MAKE_LINEAR(true, true);
		} else {
			MAKE_LINEAR(true, false);
		}
	} else {
		if (isUnsigned) {
			MAKE_LINEAR(false, true);
		} else {
			MAKE_LINEAR(false, false);
		}
	}
}

#define MAKE_WRAPPED(STEREO, UNSIGNED) \
		if (is16Bit) { \
			if (isLE) \
				return new AppendableMemoryStream<STEREO, true, UNSIGNED, true>(rate, len); \
			else  \
				return new AppendableMemoryStream<STEREO, true, UNSIGNED, false>(rate, len); \
		} else \
			return new AppendableMemoryStream<STEREO, false, UNSIGNED, false>(rate, len)

AppendableAudioStream *makeAppendableAudioStream(int rate, byte _flags, uint32 len) {
	const bool isStereo = (_flags & SoundMixer::FLAG_STEREO) != 0;
	const bool is16Bit = (_flags & SoundMixer::FLAG_16BITS) != 0;
	const bool isUnsigned = (_flags & SoundMixer::FLAG_UNSIGNED) != 0;
	const bool isLE       = (_flags & SoundMixer::FLAG_LITTLE_ENDIAN) != 0;
	
	if (isStereo) {
		if (isUnsigned) {
			MAKE_WRAPPED(true, true);
		} else {
			MAKE_WRAPPED(true, false);
		}
	} else {
		if (isUnsigned) {
			MAKE_WRAPPED(false, true);
		} else {
			MAKE_WRAPPED(false, false);
		}
	}
}
