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

#include "stdafx.h"
#include "common/file.h"
#include "common/util.h"
#include "sound/audiostream.h"
#include "sound/mixer.h"


// This used to be an inline template function, but
// buggy template function handling in MSVC6 forced
// us to go with the macro approach. So far this is
// the only template function that MSVC6 seemed to
// compile incorrectly. Knock on wood.
#define READSAMPLE(is16Bit, isUnsigned, ptr) \
	((is16Bit ? READ_BE_UINT16(ptr) : (*ptr << 8)) ^ (isUnsigned ? 0x8000 : 0))

#define READ_ENDIAN_SAMPLE(is16Bit, isUnsigned, ptr, isLE) \
	((is16Bit ? (isLE ? READ_LE_UINT16(ptr) : READ_BE_UINT16(ptr)) : (*ptr << 8)) ^ (isUnsigned ? 0x8000 : 0))

#pragma mark -
#pragma mark --- LinearMemoryStream ---
#pragma mark -


/**
 * A simple raw audio stream, purely memory based. It operates on a single
 * block of data, which is passed to it upon creation. 
 * Optionally supports looping the sound.
 *
 * Design note: This code tries to be as optimiized as possible (without
 * resorting to assembly, that is). To this end, it is written as a template
 * class. This way the compiler can actually create optimized code for each
 * special code. This results in a total of 12 versions of the code being
 * generated.
 */
template<bool stereo, bool is16Bit, bool isUnsigned, bool isLE>
class LinearMemoryStream : public AudioInputStream {
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

	int16 read() {
		//assert(_ptr < _end);
		int16 val = READ_ENDIAN_SAMPLE(is16Bit, isUnsigned, _ptr, isLE);
		_ptr += (is16Bit ? 2 : 1);
		if (_loopPtr && eosIntern()) {
			_ptr = _loopPtr;
			_end = _loopEnd;
		}
		return val;
	}
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


#pragma mark -
#pragma mark --- WrappedMemoryStream ---
#pragma mark -


/**
 * Wrapped memory stream.
 */
template<bool stereo, bool is16Bit, bool isUnsigned>
class WrappedMemoryStream : public WrappedAudioInputStream {
protected:
	byte *_bufferStart;
	byte *_bufferEnd;
	byte *_pos;
	byte *_end;
	bool _finalized;
	const int _rate;

	inline bool eosIntern() const { return _end == _pos; };
public:
	WrappedMemoryStream(int rate, uint bufferSize);
	~WrappedMemoryStream()		{ free(_bufferStart); }
	int readBuffer(int16 *buffer, const int numSamples);

	int16 read();
	bool isStereo() const		{ return stereo; }
	bool endOfStream() const	{ return _finalized && eosIntern(); }
	bool endOfData() const		{ return eosIntern(); }

	int getRate() const			{ return _rate; }

	void append(const byte *data, uint32 len);
	void finish()				{ _finalized = true; }
};


template<bool stereo, bool is16Bit, bool isUnsigned>
WrappedMemoryStream<stereo, is16Bit, isUnsigned>::WrappedMemoryStream(int rate, uint bufferSize)
 : _finalized(false), _rate(rate) {

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
inline int16 WrappedMemoryStream<stereo, is16Bit, isUnsigned>::read() {
	assert(!eosIntern());

	// Wrap around?
	if (_pos >= _bufferEnd)
		_pos = _pos - (_bufferEnd - _bufferStart);

	int16 val = READSAMPLE(is16Bit, isUnsigned, _pos);
	_pos += (is16Bit ? 2 : 1);

	return val;
}

template<bool stereo, bool is16Bit, bool isUnsigned>
int WrappedMemoryStream<stereo, is16Bit, isUnsigned>::readBuffer(int16 *buffer, const int numSamples) {
	int samples = 0;
	while (samples < numSamples && !eosIntern()) {
		// Wrap around?
		if (_pos >= _bufferEnd)
			_pos = _pos - (_bufferEnd - _bufferStart);

		const byte *endMarker = (_pos > _end) ? _bufferEnd : _end;
		const int len = MIN(numSamples, samples + (int)(endMarker - _pos) / (is16Bit ? 2 : 1));
		while (samples < len) {
			*buffer++ = READSAMPLE(is16Bit, isUnsigned, _pos);
			_pos += (is16Bit ? 2 : 1);
			samples++;
		}
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
	
	// Verify that the stream has not yet been finalized (by a call to finish())
	assert(!_finalized);

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


#pragma mark -
#pragma mark --- Procedural stream ---
#pragma mark -


#if 0
// Work in progress!!! Not yet usable/finished/working/anything :-)

class ProcInputStream : public AudioInputStream {
public:
	typedef void InputProc (void *refCon, int16 *data, uint len);

private:
	const int _rate;
	const bool _isStereo;
	InputProc *_proc;
	void *_refCon;
	int16 _buffer[2048];
	const int16 *_pos;
	int _len;

	void refill() {
		// Fill the buffer
		(_proc)(_refCon, _buffer, 2048);
		_pos = _buffer;
		_len = 2048;
	}

public:
	ProcInputStream(int rate, bool stereo, InputProc *proc, void *refCon)
		: _rate(rate), _isStereo(stereo), _proc(proc), _refCon(refCon), _len(0) { }
	int readBuffer(int16 *buffer, const int numSamples) {
		int remSamples = numSamples;
		while (remSamples > 0) {
			if (_len == 0)
				refill();
			// Copy data to the output
			int samples = MIN(_len, remSamples);
			memcpy(buffer, _pos, samples * sizeof(int16));
			_pos += samples;
			_len -= samples;
			buffer += samples;
			remSamples -= samples;
		}
		return numSamples;
	}
	int16 read() {
		if (_len == 0)
			refill();
		_len--;
		return *_pos++;
	}
	bool isStereo() const { return _isStereo; }
	bool endOfData() const { return false; }
	
	int getRate() const { return _rate; }
};
#endif


#pragma mark -
#pragma mark --- Input stream factories ---
#pragma mark -

/* In the following, we use preprocessor / macro tricks to simplify the code
 * which instantiates the input streams. We used to use template functions for
 * this, but MSVC6 / EVC 3-4 (used for WinCE builds) are extremely buggy when it
 * comes to this feature of C++... so as a compromise we use macros to cut down
 * on the (source) code duplication a bit.
 * So while normally macro tricks are said to make maintenance harder, in this
 * particular case it should actually help it :-)
 */

#define MAKE_LINEAR(STEREO, UNSIGNED) \
		if (is16Bit) { \
			if (isLE) \
				return new LinearMemoryStream<STEREO, true, UNSIGNED, true>(rate, ptr, len, loopOffset, loopLen, autoFree); \
			else  \
				return new LinearMemoryStream<STEREO, true, UNSIGNED, false>(rate, ptr, len, loopOffset, loopLen, autoFree); \
		} else \
			return new LinearMemoryStream<STEREO, false, UNSIGNED, false>(rate, ptr, len, loopOffset, loopLen, autoFree)

AudioInputStream *makeLinearInputStream(int rate, byte _flags, const byte *ptr, uint32 len, uint loopOffset, uint loopLen) {
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
		if (is16Bit) \
			return new WrappedMemoryStream<STEREO, true, UNSIGNED>(rate, len); \
		else \
			return new WrappedMemoryStream<STEREO, false, UNSIGNED>(rate, len)

WrappedAudioInputStream *makeWrappedInputStream(int rate, byte _flags, uint32 len) {
	const bool isStereo = (_flags & SoundMixer::FLAG_STEREO) != 0;
	const bool is16Bit = (_flags & SoundMixer::FLAG_16BITS) != 0;
	const bool isUnsigned = (_flags & SoundMixer::FLAG_UNSIGNED) != 0;
	
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
