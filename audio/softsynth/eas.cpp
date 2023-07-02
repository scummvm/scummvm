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

#include "common/scummsys.h"

#if defined(USE_SONIVOX)

#include <sonivox/eas.h>
#include <sonivox/eas_reverb.h>

#include "common/debug.h"
#include "common/endian.h"
#include "common/textconsole.h"
#include "common/error.h"
#include "common/file.h"
#include "common/config-manager.h"
#include "common/system.h"
#include "audio/audiostream.h"
#include "audio/mpu401.h"
#include "audio/musicplugin.h"
#include "audio/mixer.h"

//#define EAS_DUMPSTREAM "/sdcard/eas.dump"

// NOTE:
// EAS's render function *only* accepts one mix buffer size. it's defined at
// compile time of the library and can be retrieved via EASLibConfig.bufSize
// (seen: 128 bytes).
// to avoid local intermediate buffers, this implementation insists on a fixed
// buffer size of the calling rate converter, which in return must be a
// multiple of EAS's. that may change if there're hickups because slower
// devices can't render fast enough

// from rate_arm.cpp
#define INTERMEDIATE_BUFFER_SIZE 512

// EAS does many 1 byte reads, avoid seeking by caching values
struct EAS_FileHandle {
	Common::SeekableReadStream *stream;
	int64 size;
	int64 pos;
};

static int EAS_DLS_read(void *handle, void *buf, int offset, int size) {
	EAS_FileHandle *fh = (EAS_FileHandle *)handle;
	if (fh->pos != offset) {
		if (!fh->stream->seek(offset)) {
			fh->pos = -1;
			return 0;
		}
		fh->pos = offset;
	}
	int ret = fh->stream->read(buf, size);
	fh->pos += ret;

	return ret;
}

static int EAS_DLS_size(void *handle) {
	EAS_FileHandle *fh = (EAS_FileHandle *)handle;
	if (fh->size < 0) {
		fh->size = fh->stream->size();
	}
	return fh->size;
}

class MidiDriver_EAS : public MidiDriver_MPU401, Audio::AudioStream {
public:
	MidiDriver_EAS();
	virtual ~MidiDriver_EAS();

	// MidiDriver
	int open() override;
	bool isOpen() const override { return _EASHandle; }

	void close() override;
	void send(uint32 b) override;
	void sysEx(const byte *msg, uint16 length) override;
	void setTimerCallback(void *timerParam,
								Common::TimerManager::TimerProc timerProc) override;
	uint32 getBaseTempo() override;

	// AudioStream
	int readBuffer(int16 *buffer, const int numSamples) override;
	bool isStereo() const override;
	int getRate() const override;
	bool endOfData() const override;

private:
	const S_EAS_LIB_CONFIG *_config;
	EAS_DATA_HANDLE _EASHandle;
	EAS_HANDLE _midiStream;

	Common::TimerManager::TimerProc _timerProc;
	void *_timerParam;
	uint32 _baseTempo;
	uint _rounds;
	Audio::SoundHandle _soundHandle;

	Common::DumpFile _dump;
};

MidiDriver_EAS::MidiDriver_EAS() :
	MidiDriver_MPU401(),
	_config(0),
	_EASHandle(0),
	_midiStream(0),
	_timerProc(0),
	_timerParam(0),
	_baseTempo(0),
	_rounds(0),
	_soundHandle(),
	_dump() {
}

MidiDriver_EAS::~MidiDriver_EAS() {
}

int MidiDriver_EAS::open() {
	if (isOpen())
		return MERR_ALREADY_OPEN;

	_config = EAS_Config();
	if (!_config) {
		close();
		warning("error retrieving EAS library configuration");
		return -1;
	}

	if (_config->numChannels > 2) {
		close();
		warning("unsupported number of EAS channels: %d", (int32)_config->numChannels);
		return -1;
	}

	// see note at top of this file
	if (INTERMEDIATE_BUFFER_SIZE % (_config->mixBufferSize * _config->numChannels)) {
		close();
		warning("unsupported EAS buffer size: %d", (int32)_config->mixBufferSize);
		return -1;
	}

	EAS_RESULT res = EAS_Init(&_EASHandle);
	if (res) {
		close();
		warning("error initializing the EAS library: %d", (int32)res);
		return -1;
	}

	res = EAS_SetParameter(_EASHandle, EAS_MODULE_REVERB, EAS_PARAM_REVERB_PRESET, EAS_PARAM_REVERB_CHAMBER);
	if (res)
		warning("error setting reverb preset: %d", (int32)res);

	res = EAS_SetParameter(_EASHandle, EAS_MODULE_REVERB, EAS_PARAM_REVERB_BYPASS, 0);
	if (res)
		warning("error disabling reverb bypass: %d", (int32)res);

	// 90 is EAS's default, max is 100
	// so the option slider will only work from 0.1 to 1.1
	res = EAS_SetVolume(_EASHandle, 0, ConfMan.getInt("midi_gain") - 10);
	if (res)
		warning("error setting EAS master volume: %d", (int32)res);

	if (ConfMan.hasKey("soundfont")) {
		const Common::String dls = ConfMan.get("soundfont");

		debug("loading DLS file '%s'", dls.c_str());
		Common::FSNode fsnode(dls);
		Common::SeekableReadStream *stream = fsnode.createReadStream();

		if (stream) {
			EAS_FileHandle h;
			h.stream = stream;
			h.size = -1;
			h.pos = -1;

			EAS_FILE f;
			memset(&f, 0, sizeof(EAS_FILE));
			f.handle = &h;
			f.readAt = EAS_DLS_read;
			f.size = EAS_DLS_size;

			res = EAS_LoadDLSCollection(_EASHandle, 0, &f);
			if (res)
				warning("error loading DLS file '%s': %d", dls.c_str(), (int32)res);
			else
				debug("DLS file loaded");

			delete stream;
		} else {
			warning("error loading DLS file '%s': can't be opened", dls.c_str());
		}
	}

	res = EAS_OpenMIDIStream(_EASHandle, &_midiStream, 0);
	if (res) {
		close();
		warning("error opening EAS MIDI stream: %d", (int32)res);
		return -1;
	}

	// set the timer frequency to match a single buffer size
	_baseTempo = (1000000 * _config->mixBufferSize) / _config->sampleRate;

	// number of buffer fills per readBuffer()
	_rounds = INTERMEDIATE_BUFFER_SIZE / (_config->mixBufferSize * _config->numChannels);

	debug("EAS initialized (voices:%d channels:%d rate:%d buffer:%d) "
			"tempo:%u rounds:%u", (int32)_config->maxVoices, (int32)_config->numChannels,
			(int32)_config->sampleRate, (int32)_config->mixBufferSize, _baseTempo, _rounds);

#ifdef EAS_DUMPSTREAM
	if (!_dump.open(EAS_DUMPSTREAM))
		warning("error opening EAS dump file");
#endif

	g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType,
	                                 &_soundHandle, this, -1,
	                                 Audio::Mixer::kMaxChannelVolume, 0,
	                                 DisposeAfterUse::NO, true);

	return 0;
}

void MidiDriver_EAS::close() {
	MidiDriver_MPU401::close();

	if (!_EASHandle)
		return;

	g_system->getMixer()->stopHandle(_soundHandle);

#ifdef EAS_DUMPSTREAM
	if (_dump.isOpen())
		_dump.close();
#endif

	// not pretty, but better than a mutex
	g_system->delayMillis((_baseTempo * _rounds) / 1000);

	if (_midiStream) {
		EAS_RESULT res = EAS_CloseMIDIStream(_EASHandle, _midiStream);
		if (res)
			warning("error closing EAS MIDI stream: %d", (int32)res);

		_midiStream = 0;
	}

	EAS_RESULT res = EAS_Shutdown(_EASHandle);
	if (res)
		warning("error shutting down the EAS library: %d", (int32)res);

	_EASHandle = 0;
}

void MidiDriver_EAS::send(uint32 b) {
	byte buf[4];

	if (!isOpen())
		return;

	WRITE_LE_UINT32(buf, b);

	int32 len = 3;
	if ((buf[0] >> 4) == 0xC || (buf[0] >> 4) == 0xD)
		len = 2;

	int32 res = EAS_WriteMIDIStream(_EASHandle, _midiStream, buf, len);
	if (res)
		warning("error writing to EAS MIDI stream: %d", (int32)res);
}

void MidiDriver_EAS::sysEx(const byte *msg, uint16 length) {
	byte buf[266];

	if (!isOpen())
		return;

	assert(length + 2 <= ARRAYSIZE(buf));

	buf[0] = 0xF0;
	memcpy(buf + 1, msg, length);
	buf[length + 1] = 0xF7;

	EAS_RESULT res = EAS_WriteMIDIStream(_EASHandle, _midiStream, buf, length + 2);
	if (res)
		warning("error writing to EAS MIDI stream: %d", (int32)res);
}

void MidiDriver_EAS::setTimerCallback(void *timerParam,
								Common::TimerManager::TimerProc timerProc) {
	_timerParam = timerParam;
	_timerProc = timerProc;
}

uint32 MidiDriver_EAS::getBaseTempo() {
	return _baseTempo;
}

int MidiDriver_EAS::readBuffer(int16 *buffer, const int numSamples) {
	// see note at top of this file
	assert(numSamples == INTERMEDIATE_BUFFER_SIZE);

	if (!isOpen())
		return -1;

	EAS_RESULT res;
	EAS_I32 c;

	for (uint i = 0; i < _rounds; ++i) {
		// pull in MIDI events for exactly one buffer size
		if (_timerProc)
			(*_timerProc)(_timerParam);

		// if there are no MIDI events, this just renders silence
		res = EAS_Render(_EASHandle, buffer, _config->mixBufferSize, &c);
		if (res) {
			warning("error rendering EAS samples: %d", (int32)res);
			return -1;
		}

#ifdef EAS_DUMPSTREAM
		if (_dump.isOpen())
			_dump.write(buffer, c * _config->numChannels * 2);
#endif

		buffer += c * _config->numChannels;
	}

	return numSamples;
}

bool MidiDriver_EAS::isStereo() const {
	return _config->numChannels == 2;
}

int MidiDriver_EAS::getRate() const {
	return _config->sampleRate;
}

bool MidiDriver_EAS::endOfData() const {
	return false;
}

class EASMusicPlugin : public MusicPluginObject {
public:
	EASMusicPlugin();
	virtual ~EASMusicPlugin();

	const char *getName() const;
	const char *getId() const;
	MusicDevices getDevices() const;
	Common::Error createInstance(MidiDriver **mididriver,
									MidiDriver::DeviceHandle = 0) const;
};

EASMusicPlugin::EASMusicPlugin() {
}

EASMusicPlugin::~EASMusicPlugin() {
}

const char *EASMusicPlugin::getName() const {
	return "Embedded Audio Synthesis";
}

const char *EASMusicPlugin::getId() const {
	return "eas";
}

MusicDevices EASMusicPlugin::getDevices() const {
	MusicDevices devices;
	devices.push_back(MusicDevice(this, "", MT_GM));

	return devices;
}

Common::Error EASMusicPlugin::createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle) const {
	*mididriver = new MidiDriver_EAS();

	return Common::kNoError;
}

REGISTER_PLUGIN_STATIC(EAS, PLUGIN_TYPE_MUSIC, EASMusicPlugin);

#endif
