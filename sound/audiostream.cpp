/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2005 The ScummVM project
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

#include "common/stdafx.h"
#include "common/file.h"
#include "common/util.h"
#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "sound/mp3.h"
#include "sound/vorbis.h"
#include "sound/flac.h"

struct StreamFileFormat {
	/** Decodername */
	const char* decoderName;
	const char* fileExtension;
	/**
	 * Pointer to a function which tries to open a file of type StreamFormat.
	 * Return NULL in case of an error (invalid/nonexisting file).
	 */
	AudioStream* (*openStreamFile)(Common::File *file, uint32 size);
};

static const StreamFileFormat STREAM_FILEFORMATS[] = {
	/* decoderName,		fileExt, openStreamFuntion */
#ifdef USE_FLAC
	{ "Flac",			"flac", makeFlacStream },
	{ "Flac",			"fla",  makeFlacStream },
#endif
#ifdef USE_VORBIS
	{ "Ogg Vorbis",		"ogg",  makeVorbisStream },
#endif
#ifdef USE_MAD
	{ "Mpeg Layer 3",	"mp3",  makeMP3Stream },
#endif

	{ NULL, NULL, NULL } // Terminator
};

AudioStream* AudioStream::openStreamFile(const char *filename)
{
	char buffer[1024];
	const uint len = strlen(filename);
	assert(len+6 < sizeof(buffer)); // we need a bigger buffer if wrong

	memcpy(buffer, filename, len);
	buffer[len] = '.';
	char *ext = &buffer[len+1];

	AudioStream* stream = NULL;
	Common::File *fileHandle = new Common::File();

	for (int i = 0; i < ARRAYSIZE(STREAM_FILEFORMATS)-1 && stream == NULL; ++i) {
		strcpy(ext, STREAM_FILEFORMATS[i].fileExtension);
		fileHandle->open(buffer);
		if (fileHandle->isOpen())
			stream = STREAM_FILEFORMATS[i].openStreamFile(fileHandle, fileHandle->size());
	}

	// Do not reference the file anymore. If the stream didn't incRef the file,
	// the object will be deleted (and the file be closed).
	fileHandle->decRef();

	if (stream == NULL) {
		debug(1, "AudioStream: Could not open compressed AudioFile %s", filename);
	}

	return stream;
}

#pragma mark -
#pragma mark --- LinearMemoryStream ---
#pragma mark -


/**
 * A simple raw audio stream, purely memory based. It operates on a single
 * block of data, which is passed to it upon creation.
 * Optionally supports looping the sound.
 *
 * Design note: This code tries to be as optimized as possible (without
 * resorting to assembly, that is). To this end, it is written as a template
 * class. This way the compiler can actually create optimized code for each
 * special code. This results in a total of 12 versions of the code being
 * generated.
 */
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


#pragma mark -
#pragma mark --- Input stream factory ---
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

AudioStream *makeLinearInputStream(int rate, byte flags, const byte *ptr, uint32 len, uint loopOffset, uint loopLen) {
	const bool isStereo   = (flags & Audio::Mixer::FLAG_STEREO) != 0;
	const bool is16Bit    = (flags & Audio::Mixer::FLAG_16BITS) != 0;
	const bool isUnsigned = (flags & Audio::Mixer::FLAG_UNSIGNED) != 0;
	const bool isLE       = (flags & Audio::Mixer::FLAG_LITTLE_ENDIAN) != 0;
	const bool autoFree   = (flags & Audio::Mixer::FLAG_AUTOFREE) != 0;

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
