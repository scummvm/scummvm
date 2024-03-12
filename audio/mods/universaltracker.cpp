/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// Disable symbol overrides for FILE and fseek as those are used in the
// mikmod headers.
#define FORBIDDEN_SYMBOL_EXCEPTION_FILE

// On Windows, unlink and setjmp/longjmp may also be triggered.
#if defined(WIN32)
#define FORBIDDEN_SYMBOL_EXCEPTION_chdir
#define FORBIDDEN_SYMBOL_EXCEPTION_getcwd
#define FORBIDDEN_SYMBOL_EXCEPTION_mkdir
#define FORBIDDEN_SYMBOL_EXCEPTION_unlink
#define FORBIDDEN_SYMBOL_EXCEPTION_setjmp
#define FORBIDDEN_SYMBOL_EXCEPTION_longjmp
#endif

#include "common/inttypes.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

#ifdef USE_OPENMPT

#include <libopenmpt/libopenmpt.h>

#elif defined(USE_MIKMOD)

#include <mikmod.h>

#endif

#ifdef USE_OPENMPT

static size_t memoryReaderRead(void *stream, void *dst, size_t bytes) {
	Common::SeekableReadStream *reader = (Common::SeekableReadStream *)stream;

	if (!reader) {
		return 0;
	}

	uint32 receivedBytes = reader->read(dst, bytes);

	if (receivedBytes < bytes) {
		return 0;
	}

	return receivedBytes;
}

static int memoryReaderSeek(void *stream, int64_t offset, int whence) {
	Common::SeekableReadStream *reader = (Common::SeekableReadStream *)stream;

	if (!reader) {
		return -1;
	}

	bool ret = reader->seek(offset, whence);

	if (ret)
		return 0;

	return -1;
}

static int64_t memoryReaderTell(void *stream) {
	Common::SeekableReadStream *reader = (Common::SeekableReadStream *)stream;

	if (reader) {
		return reader->pos();
	}

	return -1;
}

#elif defined(USE_MIKMOD)

typedef struct MikMemoryReader {
	MREADER core;
	Common::SeekableReadStream *stream;
} MikMemoryReader;

static BOOL memoryReaderEof(MREADER *reader);
static BOOL memoryReaderRead(MREADER *reader, void *ptr, size_t size);
static int  memoryReaderGet(MREADER *reader);
static int  memoryReaderSeek(MREADER *reader, long offset, int whence);
static long memoryReaderTell(MREADER *reader);

MREADER *createMikMemoryReader(Common::SeekableReadStream *stream) {
	MikMemoryReader *reader = (MikMemoryReader *)calloc(1, sizeof(MikMemoryReader));

	if (reader) {
		reader->core.Eof = &memoryReaderEof;
		reader->core.Read = &memoryReaderRead;
		reader->core.Get = &memoryReaderGet;
		reader->core.Seek = &memoryReaderSeek;
		reader->core.Tell = &memoryReaderTell;
		reader->stream = stream;
	}

	return (MREADER *)reader;
}

static BOOL memoryReaderEof(MREADER *reader) {
	MikMemoryReader *mr = (MikMemoryReader *)reader;

	if (!mr)
		return 1;

	if (mr->stream && mr->stream->eos() == true)
		return 1;

	return 0;
}

static BOOL memoryReaderRead(MREADER *reader, void *ptr, size_t size) {
	MikMemoryReader *mr;
	mr = (MikMemoryReader *)reader;

	if (!mr || !mr->stream)
		return 0;

	uint32 receivedBytes = mr->stream->read(ptr, size);

	if (receivedBytes < size)
		return 0; // not enough remaining bytes (or error)

	return 1;
}

static int memoryReaderGet(MREADER *reader) {
	MikMemoryReader *mr;
	mr = (MikMemoryReader *)reader;

	if (!mr->stream)
		return -1;

	return mr->stream->readByte();
}

static int memoryReaderSeek(MREADER *reader, long offset, int whence) {
	MikMemoryReader *mr;
	mr = (MikMemoryReader *)reader;

	if (!reader || !mr->stream)
		return -1;

	return mr->stream->seek(offset, whence);
}

static long memoryReaderTell(MREADER *reader) {
	if (reader)
		return ((MikMemoryReader *)reader)->stream->pos();

	return 0;
}
// End memory wrappper

#endif // USE_MIKMOD

#ifdef USE_OPENMPT

namespace Audio {
class UniversalTrackerMod : public RewindableAudioStream {
public:
	UniversalTrackerMod(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, int rate);
	~UniversalTrackerMod();

	// ImpulseTrackerMod functions
	bool isLoaded() {
		return _mpt_load_successful;
	}

	int readBuffer(int16 *buffer, const int numSamples) override {
		openmpt_module_read_interleaved_stereo(_mod, getRate(), numSamples / 2, buffer);

		return numSamples;
	}

	bool isStereo() const override {
		return true;
	}

	int getRate() const override {
		return _sampleRate;
	}

	bool endOfData() const override {
		return _stream->eos();
	}

	bool endOfStream() const override {
		return endOfData();
	}

	// RewindableAudioStream API
	bool rewind() override {
		openmpt_module_set_position_seconds(_mod, 0.0);
		return true;
	}

private:
	DisposeAfterUse::Flag _dispose;
	bool _mpt_load_successful = false;
	Common::SeekableReadStream *_stream;
	openmpt_module *_mod = nullptr;
	int _sampleRate;
};

UniversalTrackerMod::UniversalTrackerMod(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, int rate) {
	if (!stream) {
		warning("UniversalTrackerMod::UniversalTrackerMod(): Input file/stream is invalid.");
		return;
	}

	int mod_err;
	const char *mod_err_str = NULL;

	_stream = stream;
	_dispose = disposeAfterUse;
	_sampleRate = rate;

	openmpt_stream_callbacks stream_callbacks;
	stream_callbacks.read = &memoryReaderRead;
	stream_callbacks.seek = &memoryReaderSeek;
	stream_callbacks.tell = &memoryReaderTell;

	_mod = openmpt_module_create2(stream_callbacks, _stream, NULL, NULL, NULL, NULL, &mod_err, &mod_err_str, NULL);

	if (!_mod) {
		mod_err_str = openmpt_error_string(mod_err);
		warning("UniversalTrackerMod::UniversalTrackerMod(): Parsing mod error: %s ", mod_err_str);
		openmpt_free_string(mod_err_str);
		return;
	}

	_mpt_load_successful = true;
}

UniversalTrackerMod::~UniversalTrackerMod() {
	if (_mod)
		openmpt_module_destroy(_mod);

	if (_dispose == DisposeAfterUse::Flag::YES)
		delete _stream;
}

} // End of namespace Audio

#endif // #ifdef USE_OPENMPT

#ifdef USE_MIKMOD

namespace Audio {
class UniversalTrackerMod : public RewindableAudioStream {
public:
	UniversalTrackerMod(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, int rate);
	~UniversalTrackerMod();

	// ImpulseTrackerMod functions
	bool isLoaded() {
		return _mikmod_load_successful;
	}

	// AudioStream API
	int readBuffer(int16 *buffer, const int numSamples) override {
		// Multiplied by 2 as VC_WriteBytes function expects 8 byte integer arrays, whereas buffer needs 16 ones.
		VC_WriteBytes((SBYTE *)buffer, numSamples * 2);

		return numSamples;
	}

	bool isStereo() const override {
		return true;
	}
	int getRate() const override {
		return _sampleRate;
	}
	bool endOfData() const override {
		return !Player_Active();
	}
	bool endOfStream() const override {
		return endOfData();
	}

	// RewindableAudioStream API
	bool rewind() override {
		Player_SetPosition(0);
		return true;
	}

private:
	DisposeAfterUse::Flag _dispose;
	bool _mikmod_load_successful = false;
	Common::SeekableReadStream *_stream;
	MREADER *_reader = nullptr;
	MODULE *_mod = nullptr;
	int _sampleRate;
};

UniversalTrackerMod::UniversalTrackerMod(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, int rate) {
	if (!stream) {
		warning("UniversalTrackerMod::UniversalTrackerMod(): Input file/stream is invalid.");
		return;
	}

	_sampleRate = rate;

	MikMod_InitThreads();
	MikMod_RegisterDriver(&drv_nos);

	// Set flags
	md_mode |= DMODE_SOFT_MUSIC | DMODE_NOISEREDUCTION;
	md_mixfreq = getRate();

	if (MikMod_Init("")) {
		warning("UniversalTrackerMod::UniversalTrackerMod(): Could not initialize sound, reason: %s",
		        MikMod_strerror(MikMod_errno));
		return;
	}

	// Loading only impulse tracker loader!
	MikMod_RegisterLoader(&load_it);

	_stream = stream;
	_dispose = disposeAfterUse;

	// Load mod using custom loader class!
	_reader = createMikMemoryReader(_stream);
	_mod = Player_LoadGeneric(_reader, 64, 0);
	if (!_mod) {
		warning("UniversalTrackerMod::UniversalTrackerMod(): Parsing mod error: %s", MikMod_strerror(MikMod_errno));
		return;
	}

	// Start mikmod playing, ie fill VC_Driver buffer with data
	Player_Start(_mod);
	_mikmod_load_successful = true;
}

UniversalTrackerMod::~UniversalTrackerMod() {
	Player_Stop();

	if (_mod)
		Player_Free(_mod);

	if (_reader)
		free(_reader);

	if (_dispose == DisposeAfterUse::Flag::YES)
		delete _stream;

	MikMod_Exit();
}


} // End of namespace Audio

#endif // #ifdef USE_MIKMOD

namespace Audio {

RewindableAudioStream *makeUniversalTrackerStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, int rate) {

#if !defined(USE_OPENMPT) && !defined(USE_MIKMOD)
	warning("Modplayer Support not compiled in");
	return nullptr;
#else

	UniversalTrackerMod *impulseTrackerMod = new UniversalTrackerMod(stream, disposeAfterUse, rate);

	if (!impulseTrackerMod->isLoaded()) {
		delete impulseTrackerMod;
		return nullptr;
	}

	return impulseTrackerMod;

#endif
}

}
