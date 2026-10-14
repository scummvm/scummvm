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

#include "common/endian.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "cruise/cruise.h"
#include "cruise/cruise_main.h"
#include "cruise/sound.h"
#include "cruise/volume.h"

#include "audio/decoders/raw.h"
#include "audio/fmopl.h"
#include "audio/mididrv.h"
#include "audio/mt32gm.h"
#include "common/file.h"
#include "audio/mixer.h"
#include "audio/mods/paula.h"
#include "audio/softsynth/ym2149.h"

namespace Audio {
class Mixer;
}

namespace Cruise {

class PCSoundDriver {
public:
	typedef void (*UpdateCallback)(void *);

	PCSoundDriver() :
		_upCb(nullptr),
		_upRef(nullptr),
		_musicVolume(0),
		_sfxVolume(0) {}
	virtual ~PCSoundDriver() {}

	virtual void setupChannel(int channel, const byte *data, int instrument, int volume) = 0;
	virtual void setChannelFrequency(int channel, int frequency) = 0;
	virtual void stopChannel(int channel) = 0;
	virtual void playSample(const byte *data, int size, int channel, int volume) = 0;
	virtual void stopAll() = 0;
	virtual const char *getInstrumentExtension() const { return ""; }
	virtual void syncSounds();

	void setUpdateCallback(UpdateCallback upCb, void *ref);
	void resetChannel(int channel);
	void findNote(int freq, int *note, int *oct) const;


protected:
	UpdateCallback _upCb;
	void *_upRef;
	uint8 _musicVolume;
	uint8 _sfxVolume;

	static const int _noteTable[];
	static const int _noteTableCount;
};

const int PCSoundDriver::_noteTable[] = {
	0xEEE, 0xE17, 0xD4D, 0xC8C, 0xBD9, 0xB2F, 0xA8E, 0x9F7,
	0x967, 0x8E0, 0x861, 0x7E8, 0x777, 0x70B, 0x6A6, 0x647,
	0x5EC, 0x597, 0x547, 0x4FB, 0x4B3, 0x470, 0x430, 0x3F4,
	0x3BB, 0x385, 0x353, 0x323, 0x2F6, 0x2CB, 0x2A3, 0x27D,
	0x259, 0x238, 0x218, 0x1FA, 0x1DD, 0x1C2, 0x1A9, 0x191,
	0x17B, 0x165, 0x151, 0x13E, 0x12C, 0x11C, 0x10C, 0x0FD,
	0x0EE, 0x0E1, 0x0D4, 0x0C8, 0x0BD, 0x0B2, 0x0A8, 0x09F,
	0x096, 0x08E, 0x086, 0x07E, 0x077, 0x070, 0x06A, 0x064,
	0x05E, 0x059, 0x054, 0x04F, 0x04B, 0x047, 0x043, 0x03F,
	0x03B, 0x038, 0x035, 0x032, 0x02F, 0x02C, 0x02A, 0x027,
	0x025, 0x023, 0x021, 0x01F, 0x01D, 0x01C, 0x01A, 0x019,
	0x017, 0x016, 0x015, 0x013, 0x012, 0x011, 0x010, 0x00F
};

const int PCSoundDriver::_noteTableCount = ARRAYSIZE(_noteTable);

struct AdLibRegisterSoundInstrument {
	uint8 vibrato;
	uint8 attackDecay;
	uint8 sustainRelease;
	uint8 feedbackStrength;
	uint8 keyScaling;
	uint8 outputLevel;
	uint8 freqMod;
};

struct AdLibSoundInstrument {
	byte mode;
	byte channel;
	AdLibRegisterSoundInstrument regMod;
	AdLibRegisterSoundInstrument regCar;
	byte waveSelectMod;
	byte waveSelectCar;
	byte amDepth;
};

struct VolumeEntry {
	int original;
	int adjusted;
};

class AdLibSoundDriver : public PCSoundDriver {
public:
	AdLibSoundDriver(Audio::Mixer *mixer);
	~AdLibSoundDriver() override;

	// PCSoundDriver interface
	void setupChannel(int channel, const byte *data, int instrument, int volume) override;
	void stopChannel(int channel) override;
	void stopAll() override;

	void initCard();
	void onTimer();
	void setupInstrument(const byte *data, int channel);
	void setupInstrument(const AdLibSoundInstrument *ins, int channel);
	void loadRegisterInstrument(const byte *data, AdLibRegisterSoundInstrument *reg);
	virtual void loadInstrument(const byte *data, AdLibSoundInstrument *asi) = 0;
	void syncSounds() override;

	void adjustVolume(int channel, int volume);

protected:
	OPL::OPL *_opl;
	Audio::Mixer *_mixer;

	byte _vibrato;
	VolumeEntry _channelsVolumeTable[5];
	AdLibSoundInstrument _instrumentsTable[5];

	static const int _freqTable[];
	static const int _freqTableCount;
	static const int _operatorsTable[];
	static const int _operatorsTableCount;
	static const int _voiceOperatorsTable[];
	static const int _voiceOperatorsTableCount;
};

const int AdLibSoundDriver::_freqTable[] = {
	0x157, 0x16C, 0x181, 0x198, 0x1B1, 0x1CB,
	0x1E6, 0x203, 0x222, 0x243, 0x266, 0x28A
};

const int AdLibSoundDriver::_freqTableCount = ARRAYSIZE(_freqTable);

const int AdLibSoundDriver::_operatorsTable[] = {
	0, 1, 2, 3, 4, 5, 8, 9, 10, 11, 12, 13,	16, 17, 18, 19, 20, 21
};

const int AdLibSoundDriver::_operatorsTableCount = ARRAYSIZE(_operatorsTable);

const int AdLibSoundDriver::_voiceOperatorsTable[] = {
	0, 3, 1, 4, 2, 5, 6, 9, 7, 10, 8, 11, 12, 15, 16, 16, 14, 14, 17, 17, 13, 13
};

const int AdLibSoundDriver::_voiceOperatorsTableCount = ARRAYSIZE(_voiceOperatorsTable);

class AdLibSoundDriverADL : public AdLibSoundDriver {
public:
	AdLibSoundDriverADL(Audio::Mixer *mixer) : AdLibSoundDriver(mixer) {}
	const char *getInstrumentExtension() const override { return ".ADL"; }
	void loadInstrument(const byte *data, AdLibSoundInstrument *asi) override;
	void setChannelFrequency(int channel, int frequency) override;
	void playSample(const byte *data, int size, int channel, int volume) override;
};

class PCSoundFxPlayer {
private:
	enum {
		NUM_INSTRUMENTS = 15,
		NUM_CHANNELS = 4
	};

	void update();
	void handleEvents();
	void handlePattern(int channel, const byte *patternData);

	char _musicName[33];
	bool _playing;
	bool _songPlayed;
	int _currentPos;
	int _currentOrder;
	int _numOrders;
	int _eventsDelay;
	bool _looping;
	int _fadeOutCounter;
	int _updateTicksCounter;
	int _instrumentsChannelTable[NUM_CHANNELS];
	byte *_sfxData;
	byte *_instrumentsData[NUM_INSTRUMENTS];
	PCSoundDriver *_driver;

public:
	PCSoundFxPlayer(PCSoundDriver *driver);
	~PCSoundFxPlayer();

	bool load(const char *song);
	void play();
	void stop();
	void unload();
	void fadeOut();
	void doSync(Common::Serializer &s);

	static void updateCallback(void *ref);

	bool songLoaded() const { return _sfxData != nullptr; }
	bool songPlayed() const { return _songPlayed; }
	bool playing() const { return _playing; }
	uint8 numOrders() const { assert(_sfxData); return _sfxData[470]; }
	void setNumOrders(uint8 v) { assert(_sfxData); _sfxData[470] = v; }
	void setPattern(int offset, uint8 value) { assert(_sfxData); _sfxData[472 + offset] = value; }
	const char *musicName() { return _musicName; }

	// Note: Original game never actually uses looping variable. Songs are hardcoded to loop
	bool looping() const { return _looping; }
	void setLooping(bool v) { _looping = v; }
};

byte *readBundleSoundFile(const char *name, uint32 *size = nullptr) {
	// Load the correct file
	int fileIdx = findFileInDisks(name);
	if (fileIdx < 0) return nullptr;

	if (size)
		*size = volumePtrToFileDescriptor[fileIdx].extSize;
	int unpackedSize = volumePtrToFileDescriptor[fileIdx].extSize + 2;
	byte *data = (byte *)MemAlloc(unpackedSize);
	assert(data);

	if (volumePtrToFileDescriptor[fileIdx].size + 2 != unpackedSize) {
		uint8 *packedBuffer = (uint8 *)mallocAndZero(volumePtrToFileDescriptor[fileIdx].size + 2);

		loadPackedFileToMem(fileIdx, packedBuffer);

		//uint32 realUnpackedSize = READ_BE_UINT32(packedBuffer + volumePtrToFileDescriptor[fileIdx].size - 4);

		delphineUnpack(data, packedBuffer, volumePtrToFileDescriptor[fileIdx].size);

		MemFree(packedBuffer);
	} else {
		loadPackedFileToMem(fileIdx, data);
	}

	return data;
}



class MT32SoundFxPlayer {
public:
	MT32SoundFxPlayer(MidiDriver_MT32GM *output)
		: _output(output),
		  _module(nullptr),
		  _moduleSize(0),
		  _order(0),
		  _pos(0),
		  _orders(0),
		  _fade(0),
		  _active(false),
		  _paused(false),
		  _songPlayed(false),
		  _looping(false),
		  _elapsed(0),
		  _rowDuration(0) {
		memset(_musicName, 0, sizeof(_musicName));
		memset(_types, 0, sizeof(_types));
		resetChannels();
		_output->property(MidiDriver::PROP_USER_VOLUME_SCALING, 1);
		_output->setSourceType(0, MidiDriver_Multisource::SOURCE_TYPE_MUSIC);
		_output->setSourceType(1, MidiDriver_Multisource::SOURCE_TYPE_SFX);
		// Keep patch output levels constant; controllers provide per-source volume.
		for (uint i = 0; i < 8; ++i) {
			const byte level = 100;
			_output->sysExMT32(&level, 1, 0xc008 + i * 16, true);
		}
		_output->setTimerCallback(this, timerCallback);
	}

	~MT32SoundFxPlayer() {
		_output->setTimerCallback(nullptr, nullptr);
		_output->clearSysExQueue();
		_output->stopAllNotes(true);
		_output->close();
		delete _output;
		MemFree(_module);
	}

	bool load(const char *name) {
		unload();
		Common::strlcpy(_musicName, name, sizeof(_musicName));
		uint32 size = 0;
		byte *module = readBundleSoundFile(name, &size);
		if (!module)
			return false;
		if (size < 2400 || !module[470] || module[470] > 128) {
			warning("Invalid Cruise MT-32 module '%s'", name);
			MemFree(module);
			return false;
		}
		for (uint i = 0; i < module[470]; ++i) {
			if (module[472 + i] >= (size - 2400) / 1024) {
				MemFree(module);
				return false;
			}
		}
		byte *instruments[15] = {};
		uint32 instrumentSizes[15] = {};
		bool valid = true;
		for (uint i = 0; i < 15 && valid; ++i) {
			char filename[23];
			memcpy(filename, module + 20 + i * 30, 22);
			filename[22] = 0;
			if (!filename[0])
				continue;
			char *dot = strrchr(filename, '.');
			if (dot)
				*dot = 0;
			Common::strlcat(filename, ".H32", sizeof(filename));
			instruments[i] = readBundleSoundFile(filename, &instrumentSizes[i]);
			valid = validInstrument(instruments[i], instrumentSizes[i]);
		}
		if (!valid) {
			warning("Missing or invalid Cruise MT-32 instrument in '%s'", name);
			for (uint i = 0; i < 15; ++i)
				MemFree(instruments[i]);
			MemFree(module);
			return false;
		}
		Common::StackLock lock(_mutex);
		_module = module;
		_moduleSize = size;
		_orders = module[470];
		_order = _pos = 0;
		for (uint i = 0; i < 4; ++i)
			_instruments[i] = -1;
		_songPlayed = _looping = false;
		// Twelve MFP interrupts per row, with a clock divider of 200.
		const uint divisor = (244 - module[471]) & 255;
		_rowDuration = (uint64)12 * 200 * (divisor ? divisor : 256) * 1000000;
		for (uint i = 0; i < 15; ++i) {
			if (instruments[i]) {
				_types[i] = instruments[i][22];
				if (_types[i] >= 0x80)
					_output->sysExMT32(instruments[i] + 23, 246, 0x20000 + i * 256, true);
				MemFree(instruments[i]);
			} else {
				_types[i] = 0;
			}
			writePatch(i, _types[i]);
		}
		return true;
	}

	void play() {
		Common::StackLock lock(_mutex);
		if (!_module)
			return;
		stopMusicChannels();
		_order = _pos = _fade = 0;
		_elapsed = 0;
		_active = true;
		_songPlayed = false;
		for (uint i = 0; i < 4; ++i)
			_instruments[i] = -1;
	}

	void stop() {
		Common::StackLock lock(_mutex);
		_active = false;
		_fade = 0;
		stopMusicChannels();
	}

	void unload() {
		stop();
		Common::StackLock lock(_mutex);
		MemFree(_module);
		_module = nullptr;
		_moduleSize = 0;
		_orders = 0;
	}

	void fadeOut() {
		Common::StackLock lock(_mutex);
		if (_active) {
			_active = false;
			_fade = 1;
		}
	}

	bool playing() const {
		Common::StackLock lock(_mutex);
		return _active;
	}

	bool songLoaded() const {
		Common::StackLock lock(_mutex);
		return _module != nullptr;
	}

	bool songPlayed() const {
		Common::StackLock lock(_mutex);
		return _songPlayed;
	}

	bool looping() const {
		Common::StackLock lock(_mutex);
		return _looping;
	}

	void setLooping(bool value) {
		Common::StackLock lock(_mutex);
		_looping = value;
	}

	const char *musicName() const { return _musicName; }

	uint8 numOrders() const {
		Common::StackLock lock(_mutex);
		return _orders;
	}

	void setNumOrders(uint8 count) {
		Common::StackLock lock(_mutex);
		if (!_module || !count || count > 128)
			return;
		for (uint i = 0; i < count; ++i) {
			if (_module[472 + i] >= (_moduleSize - 2400) / 1024)
				return;
		}
		_orders = _module[470] = count;
		if (_order >= count)
			_order = 0;
	}

	void setPattern(int offset, uint8 pattern) {
		Common::StackLock lock(_mutex);
		if (_module && offset >= 0 && offset < 128 && pattern < (_moduleSize - 2400) / 1024)
			_module[472 + offset] = pattern;
	}

	void playEffect(const byte *data, int size, int channel, int volume) {
		if (!data || size < 1 || channel < 0 || channel >= 4 || data[0] >= 0xc0 || (data[0] >= 0x80 && size < 247))
			return;
		if (data[0] >= 0x80) {
			for (uint i = 1; i < 247; ++i) {
				if (data[i] >= 0x80)
					return;
			}
		}
		Common::StackLock lock(_mutex);
		stopChannel(channel + 4);
		const int slot = 15 + channel;
		if (data[0] >= 0x80)
			_output->sysExMT32(data + 1, 246, 0x20000 + slot * 256, true, true, 1);
		writePatch(slot, data[0]);
		_programs[channel + 4] = slot;
		_output->send(1, (0xb5 + channel) | (7 << 8) | ((CLIP(volume, 0, 63) * 127 / 63) << 16));
		// H32 effects use a fixed trigger note, as in Cine's MT-32 driver.
		_notes[channel + 4] = 12;
		_pending[channel + 4] = true;
	}

	void stopEffect(int channel) {
		Common::StackLock lock(_mutex);
		if (channel == -1) {
			for (uint i = 4; i < 8; ++i)
				stopChannel(i);
		} else if (channel >= 0 && channel < 4) {
			stopChannel(channel + 4);
		}
	}

	void setEffectPeriod(int channel, int period, int volume) {
		if (channel < 0 || channel >= 4 || period <= 0)
			return;
		Common::StackLock lock(_mutex);
		if (_notes[channel + 4] < 0)
			return;
		const int program = _programs[channel + 4];
		stopChannel(channel + 4);
		_programs[channel + 4] = program;
		_output->send(1, (0xb5 + channel) | (7 << 8) | ((CLIP(volume, 0, 63) * 127 / 63) << 16));
		_notes[channel + 4] = noteForPeriod(period);
		_pending[channel + 4] = true;
	}

	void pause(bool paused) {
		Common::StackLock lock(_mutex);
		if (_paused == paused)
			return;
		_paused = paused;
		_output->setSourceVolume(0, paused ? 0 : 255);
		_output->setSourceVolume(1, paused ? 0 : 255);
		for (uint i = 0; i < 8; ++i) {
			if (paused)
				_output->send(i < 4 ? 0 : 1, (0xb1 + i) | (123 << 8));
			else if (_notes[i] >= 0)
				_pending[i] = true;
		}
	}

	void syncSounds() {
		Common::StackLock lock(_mutex);
		_output->syncSoundSettings();
	}

	void doSync(Common::Serializer &s) {
		char name[33];
		int16 loaded;
		int16 songPlayed;
		int16 looping;
		int16 pos;
		int16 order;
		int16 active;
		{
			Common::StackLock lock(_mutex);
			memcpy(name, _musicName, sizeof(name));
			loaded = _module != nullptr;
			songPlayed = _songPlayed;
			looping = _looping;
			pos = _pos;
			order = _order;
			active = _active;
		}
		s.syncBytes((byte *)name, sizeof(name));
		s.syncAsSint16LE(loaded);
		s.syncAsSint16LE(songPlayed);
		s.syncAsSint16LE(looping);
		s.syncAsSint16LE(pos);
		s.syncAsSint16LE(order);
		s.syncAsSint16LE(active);
		if (!s.isLoading())
			return;
		stopEffect(-1);
		name[32] = 0;
		if (!loaded) {
			unload();
			Common::strlcpy(_musicName, name, sizeof(_musicName));
			Common::StackLock lock(_mutex);
			_songPlayed = songPlayed != 0;
			_looping = looping != 0;
			return;
		}
		if (!load(name))
			return;
		Common::StackLock lock(_mutex);
		_order = order >= 0 && order < _orders ? order : 0;
		_pos = pos >= 0 && pos < 1024 && !(pos % 16) ? pos : 0;
		_looping = looping != 0;
		_songPlayed = songPlayed != 0;
		for (int orderIndex = 0; orderIndex <= _order; ++orderIndex) {
			const int end = orderIndex == _order ? _pos : 1024;
			for (int row = 0; row < end; row += 16)
				playRow(orderIndex, row);
		}
		for (uint i = 0; i < 4; ++i)
			_programs[i] = _instruments[i];
		_active = active != 0;
		_elapsed = 0;
		if (!_active)
			stopMusicChannels();
	}

	static bool validInstrument(const byte *data, uint32 size) {
		if (!data || size < 23 || data[22] >= 0xc0)
			return false;
		if (data[22] < 0x80)
			return true;
		if (size < 269)
			return false;
		for (uint i = 23; i < 269; ++i) {
			if (data[i] >= 0x80)
				return false;
		}
		return true;
	}

private:
	void resetChannels() {
		for (uint i = 0; i < 8; ++i) {
			_notes[i] = -1;
			_programs[i] = -1;
			_pending[i] = false;
		}
		for (uint i = 0; i < 4; ++i)
			_instruments[i] = -1;
	}

	void stopChannel(uint channel) {
		_output->send(channel < 4 ? 0 : 1, (0xb1 + channel) | (123 << 8));
		_notes[channel] = -1;
		_programs[channel] = -1;
		_pending[channel] = false;
	}

	void stopMusicChannels() {
		for (uint i = 0; i < 4; ++i)
			stopChannel(i);
	}

	void writePatch(uint slot, byte type) {
		const byte patch[8] = { (byte)(type < 0x80 ? type >> 6 : 2), (byte)(type < 0x80 ? type & 63 : slot), 24, 50, 12, 3, 1, 0 };
		_output->sysExMT32(patch, sizeof(patch), 0x14000 + slot * 8, true);
	}

	static int noteForPeriod(int period) {
		static const uint16 periods[72] = {
			1076, 1076, 1076, 1076, 1076, 1076, 1076, 1076, 1076, 1076, 1076, 1076,
			1076, 1016, 960, 906, 856, 808, 762, 720, 678, 640, 604, 570,
			538, 508, 480, 453, 428, 404, 381, 360, 339, 320, 302, 285,
			269, 254, 240, 226, 214, 202, 190, 180, 170, 160, 151, 143,
			135, 127, 120, 113, 113, 113, 113, 113, 113, 113, 113, 113,
			113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113,
		};
		int octave;
		if (period < 113)
			octave = 5;
		else if (period <= 135)
			octave = 4;
		else if (period <= 269)
			octave = 3;
		else if (period <= 538)
			octave = 2;
		else if (period <= 1076)
			octave = 1;
		else
			octave = 0;
		int note = 0;
		while (note < 11 && periods[octave * 12 + note] > period)
			++note;
		return (octave + 2) * 12 + note;
	}

	void playRow(int order, int pos) {
		const byte *row = _module + 2400 + _module[472 + order] * 1024 + pos;
		for (uint channel = 0; channel < 4; ++channel) {
			const byte *event = row + channel * 4;
			const int16 period = READ_BE_INT16(event);
			if (period < 0) {
				stopChannel(channel);
				continue;
			}
			if (period > 0)
				stopChannel(channel);
			const int instrument = (event[2] >> 4) - 1;
			if (instrument >= 0 && (_instruments[channel] != instrument || _fade)) {
				_instruments[channel] = instrument;
				_programs[channel] = instrument;
				const int volume = CLIP<int>(_module[instrument] - _fade, 0, 100);
				_output->send(0, (0xb1 + channel) | (7 << 8) | ((volume * 127 / 100) << 16));
			}
			if (period > 0) {
				_notes[channel] = noteForPeriod(period);
				_pending[channel] = true;
			}
		}
	}

	static void timerCallback(void *ref) { ((MT32SoundFxPlayer *)ref)->onTimer(); }

	void flushPending() {
		for (uint i = 0; i < 8; ++i) {
			if (_programs[i] >= 0) {
				_output->send(i < 4 ? 0 : 1, (0xc1 + i) | (_programs[i] << 8));
				_programs[i] = -1;
			}
			if (_pending[i]) {
				_output->send(i < 4 ? 0 : 1, (0x91 + i) | (_notes[i] << 8) | (127 << 16));
				_pending[i] = false;
			}
		}
	}

	void onTimer() {
		Common::StackLock lock(_mutex);
		if (_paused || !_output->isReady())
			return;
		flushPending();
		if (!_module || (!_active && !_fade))
			return;
		_elapsed += (uint64)_output->getBaseTempo() * 2457600;
		while (_elapsed >= _rowDuration) {
			_elapsed -= _rowDuration;
			if (_fade) {
				_fade += 2;
				if (_fade >= 100) {
					_active = false;
					_fade = 0;
					_songPlayed = true;
					stopMusicChannels();
					return;
				}
			}
			playRow(_order, _pos);
			_pos += 16;
			if (_pos == 1024) {
				_pos = 0;
				if (++_order == _orders) {
					_order = 0;
					_songPlayed = true;
				}
			}
		}
		flushPending();
	}

	MidiDriver_MT32GM *_output;
	mutable Common::Mutex _mutex;
	byte *_module;
	uint32 _moduleSize;
	byte _types[15];
	char _musicName[33];
	int _notes[8];
	int _programs[8];
	bool _pending[8];
	int _instruments[4];
	int _order;
	int _pos;
	uint8 _orders;
	int _fade;
	bool _active;
	bool _paused;
	bool _songPlayed;
	bool _looping;
	uint64 _elapsed;
	uint64 _rowDuration;
};

// Cruise's Amiga modules are SoundFX songs with external signed PCM .AMI files.
// Keep a persistent stream: the scripts can stop, reload and restart music.
class AmigaSoundFxPlayer : public Audio::Paula {
public:
	AmigaSoundFxPlayer(Audio::Mixer *mixer)
		: Audio::Paula(true, mixer->getOutputRate(), 0, Audio::Paula::kFilterModeDefault, 2),
		  _mixer(mixer),
		  _module(nullptr),
		  _moduleSize(0),
		  _order(0),
		  _pos(0),
		  _orders(0),
		  _fade(0),
		  _active(false),
		  _songPlayed(false),
		  _looping(false),
		  _rowNumerator(0),
		  _sampleRemainder(0) {
		memset(_musicName, 0, sizeof(_musicName));
		memset(_instruments, 0, sizeof(_instruments));
		memset(_volumes, 0, sizeof(_volumes));
		setChannelPanning(0, PANNING_LEFT);
		setChannelPanning(3, PANNING_LEFT);
		setChannelPanning(1, PANNING_RIGHT);
		setChannelPanning(2, PANNING_RIGHT);
		_end = false;
		_mixer->playStream(Audio::Mixer::kMusicSoundType, &_handle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
	}

	~AmigaSoundFxPlayer() override {
		_mixer->stopHandle(_handle);
		unload();
	}

	bool load(const char *name) {
		unload();
		Common::strlcpy(_musicName, name, sizeof(_musicName));
		uint32 size = 0;
		byte *module = readBundleSoundFile(name, &size);
		if (!module)
			return false;
		if (size < 600 || memcmp(module, "SONG", 4) || !READ_BE_UINT16(module + 4) || module[470] == 0 || module[470] > 128) {
			warning("Invalid Cruise Amiga module '%s'", name);
			MemFree(module);
			return false;
		}
		const uint patterns = (size - 600) / 1024;
		for (uint i = 0; i < module[470]; ++i) {
			if (module[472 + i] >= patterns) {
				MemFree(module);
				return false;
			}
		}
		Instrument instruments[15] = {};
		bool valid = true;
		for (uint i = 0; i < 15 && valid; ++i) {
			const byte *header = module + 20 + i * 30;
			Instrument &instrumentData = instruments[i];
			instrumentData.length = READ_BE_UINT16(header + 22) * 2;
			instrumentData.volume = MIN<byte>(header[25], 64);
			instrumentData.repeatPos = READ_BE_UINT16(header + 26) * 2;
			instrumentData.repeatLength = READ_BE_UINT16(header + 28) * 2;
			char filename[23];
			memcpy(filename, header, 22);
			filename[22] = 0;
			if (!filename[0])
				continue; // Unused slots can contain stale length fields.
			uint32 sampleSize = 0;
			instrumentData.data = (int8 *)readBundleSoundFile(filename, &sampleSize);
			valid = instrumentData.data && instrumentData.length <= sampleSize && (instrumentData.repeatLength <= 2 || instrumentData.repeatPos + instrumentData.repeatLength <= sampleSize);
		}
		if (!valid) {
			warning("Missing or truncated instrument in Cruise Amiga module '%s'", name);
			for (uint i = 0; i < 15; ++i)
				MemFree(instruments[i].data);
			MemFree(module);
			return false;
		}
		Common::StackLock lock(_mutex);
		_module = module;
		_moduleSize = size;
		memcpy(_instruments, instruments, sizeof(instruments));
		_orders = module[470];
		_order = _pos = _fade = 0;
		_songPlayed = false;
		_looping = false;
		// Same global CIA period conversion as the validated Amiga executable.
		// Pitch remains at the PAL Paula clock; only the song timer is scaled.
		const uint32 originalTicks = READ_BE_UINT16(module + 4) + 1;
		const bool originalTempo = ConfMan.hasKey("cruise_amiga_original_tempo") && ConfMan.getBool("cruise_amiga_original_tempo");
		const uint32 ticks = originalTempo ? originalTicks : (uint32)(((uint64)originalTicks * 57031 + 32768) / 65536);
		_rowNumerator = (uint64)ticks * 6 * getRate();
		return true;
	}

	void play() {
		Common::StackLock lock(_mutex);
		if (!_module)
			return;
		clearVoices();
		memset(_volumes, 0, sizeof(_volumes));
		_order = _pos = _fade = 0;
		_sampleRemainder = 0;
		_active = true;
		_songPlayed = false;
		startPlay();
		scheduleRow();
	}

	void stop() {
		unload();
	}

	void unload() {
		Common::StackLock lock(_mutex);
		unloadUnlocked();
	}

	void fadeOut() {
		Common::StackLock lock(_mutex);
		if (_active) {
			_fade = 1;
			_active = false;
		}
	}

	bool playing() const {
		Common::StackLock lock(_mutex);
		return _active;
	}

	bool songLoaded() const {
		Common::StackLock lock(_mutex);
		return _module != nullptr;
	}

	bool songPlayed() const {
		Common::StackLock lock(_mutex);
		return _songPlayed;
	}

	bool looping() const {
		Common::StackLock lock(_mutex);
		return _looping;
	}

	void setLooping(bool value) {
		Common::StackLock lock(_mutex);
		_looping = value;
	}

	const char *musicName() const {
		return _musicName;
	}

	uint8 numOrders() const {
		Common::StackLock lock(_mutex);
		return _orders;
	}

	void setNumOrders(uint8 count) {
		Common::StackLock lock(_mutex);
		if (!_module || !count || count > 128)
			return;
		for (uint i = 0; i < count; ++i)
			if (_module[472 + i] >= (_moduleSize - 600) / 1024)
				return;
		_orders = _module[470] = count;
		if (_order >= count)
			_order = 0;
	}

	void setPattern(int offset, uint8 pattern) {
		Common::StackLock lock(_mutex);
		if (_module && offset >= 0 && offset < 128 && pattern < (_moduleSize - 600) / 1024)
			_module[472 + offset] = pattern;
	}

	void stopChannel(int channel) {
		Common::StackLock lock(_mutex);
		if (channel >= 0 && channel < 4)
			clearVoice(channel);
	}

	void doSync(Common::Serializer &s) {
		// Preserve the existing Cruise sound save layout (no version change).
		char name[33];
		int16 loaded;
		int16 songPlayed;
		int16 looping;
		int16 pos;
		int16 order;
		int16 active;
		{
			Common::StackLock lock(_mutex);
			memcpy(name, _musicName, sizeof(name));
			loaded = _module != nullptr;
			songPlayed = _songPlayed;
			looping = _looping;
			pos = _pos;
			order = _order;
			active = _active;
		}
		s.syncBytes((byte *)name, sizeof(name));
		s.syncAsSint16LE(loaded);
		s.syncAsSint16LE(songPlayed);
		s.syncAsSint16LE(looping);
		s.syncAsSint16LE(pos);
		s.syncAsSint16LE(order);
		s.syncAsSint16LE(active);
		if (!s.isLoading())
			return;
		name[32] = 0;
		if (!loaded) {
			unload();
			Common::StackLock lock(_mutex);
			memcpy(_musicName, name, sizeof(name));
			_songPlayed = songPlayed != 0;
			_looping = looping != 0;
			return;
		}
		if (!load(name)) {
			unload();
			return;
		}
		Common::StackLock lock(_mutex);
		_looping = looping != 0;
		_songPlayed = songPlayed != 0;
		_order = (order >= 0 && order < _orders) ? order : 0;
		_pos = (pos >= 0 && pos < 1024 && pos % 16 == 0) ? pos : 0;
		// Recreate the last instrument/period on each voice. The legacy save
		// format has no sample cursor: active notes restart on restoration.
		for (int orderIndex = 0; orderIndex <= _order; ++orderIndex) {
			const int end = orderIndex == _order ? _pos : 1024;
			for (int row = 0; row < end; row += 16)
				playRow(orderIndex, row);
		}
		_active = active != 0;
		if (_active) {
			_sampleRemainder = 0;
			startPlay();
			scheduleRow();
		} else {
			clearVoices();
		}
	}

private:
	struct Instrument {
		int8 *data;
		uint32 length;
		uint32 repeatPos;
		uint32 repeatLength;
		byte volume;
	};

	Audio::Mixer *_mixer;
	Audio::SoundHandle _handle;
	byte *_module;
	uint32 _moduleSize;
	Instrument _instruments[15];
	byte _volumes[4];
	char _musicName[33];
	int _order;
	int _pos;
	int _orders;
	int _fade;
	bool _active;
	bool _songPlayed;
	bool _looping;
	uint64 _rowNumerator;
	uint64 _sampleRemainder;

	void unloadUnlocked() {
		stopPlay();
		clearVoices();
		_active = false;
		_fade = 0;
		_songPlayed = true;
		_orders = _order = _pos = 0;
		MemFree(_module);
		_module = nullptr;
		_moduleSize = 0;
		for (uint i = 0; i < 15; ++i) {
			MemFree(_instruments[i].data);
			_instruments[i].data = nullptr;
		}
	}

	void scheduleRow() {
		_sampleRemainder += _rowNumerator;
		const uint samples = MAX<uint>(1, _sampleRemainder / kPalCiaClock);
		_sampleRemainder %= kPalCiaClock;
		// Paula invokes interrupt() before mixing its next block. Preserve the
		// fractional sample remainder instead of truncating every CIA tick.
		setInterruptFreq(samples + 1);
		setSingleInterrupt(samples);
	}

	void playRow(int order, int pos) {
		const byte *row = _module + 600 + _module[472 + order] * 1024 + pos;
		for (uint channel = 0; channel < 4; ++channel, row += 4) {
			const uint16 period = READ_BE_UINT16(row);
			const uint16 command = READ_BE_UINT16(row + 2);
			if (period == 0xFFFD)
				continue; // PIC: no new note.
			if (period == 0xFFFE) {
				clearVoice(channel);
				continue;
			}
			const uint instrument = command >> 12;
			if (instrument) {
				const Instrument &instrumentData = _instruments[instrument - 1];
				if (instrumentData.data && instrumentData.length > 2) {
					const bool loop = instrumentData.repeatLength > 2;
					setChannelData(channel, instrumentData.data, loop ? instrumentData.data + instrumentData.repeatPos : nullptr, instrumentData.length, loop ? instrumentData.repeatLength : 0);
				} else {
					clearVoice(channel);
				}
				int volume = instrumentData.volume;
				const int effect = (command >> 8) & 15;
				if (effect == 5)
					volume += command & 255;
				else if (effect == 6)
					volume -= command & 255;
				_volumes[channel] = CLIP(volume, 0, 64);
			}
			if (period > 0 && period < 0x8000)
				setChannelPeriod(channel, period);
			setChannelVolume(channel, MAX<int>(0, _volumes[channel] - _fade));
		}
	}

	void interrupt() override {
		if (!_module || (!_active && !_fade))
			return;
		playRow(_order, _pos);
		if (_fade) {
			_fade += 2;
			if (_fade >= 64) {
				unloadUnlocked();
				return;
			}
		}
		_pos += 16;
		if (_pos == 1024) {
			_pos = 0;
			if (++_order >= _orders)
				_order = 0; // Native Cruise loops; SongLoop is only a stored flag.
		}
		scheduleRow();
	}
};

// The ST player uses three PSG voices and instruments embedded at offset 600.
class AtariSoundFxPlayer : public Audio::AudioStream {
public:
	AtariSoundFxPlayer(Audio::Mixer *mixer)
		: _mixer(mixer),
		  _module(nullptr),
		  _moduleSize(0),
		  _order(0),
		  _pos(0),
		  _orders(0),
		  _tick(0),
		  _fade(0),
		  _volume(15),
		  _active(false),
		  _songPlayed(false),
		  _looping(false),
		  _tickSamples(0),
		  _envelopeSamples(0),
		  _tickRemainder(0),
		  _envelopeRemainder(0) {
		memset(_musicName, 0, sizeof(_musicName));
		memset(_voices, 0, sizeof(_voices));
		_psg.init();
		clearVoices();
		_mixer->playStream(Audio::Mixer::kMusicSoundType, &_handle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
	}

	~AtariSoundFxPlayer() override {
		_mixer->stopHandle(_handle);
		unload();
	}

	bool load(const char *name) {
		unload();
		Common::strlcpy(_musicName, name, sizeof(_musicName));
		uint32 size = 0;
		byte *module = readBundleSoundFile(name, &size);
		if (!module)
			return false;
		if (size < 2400 || !module[470] || module[470] > 128) {
			warning("Invalid Cruise Atari module '%s'", name);
			MemFree(module);
			return false;
		}
		for (uint i = 0; i < module[470]; ++i) {
			if (module[472 + i] >= (size - 2400) / 1024) {
				MemFree(module);
				return false;
			}
		}
		Common::StackLock lock(_mutex);
		_module = module;
		_moduleSize = size;
		_orders = module[470];
		_order = _pos = 0;
		_songPlayed = _looping = false;
		return true;
	}

	void play() {
		Common::StackLock lock(_mutex);
		if (!_module)
			return;
		clearVoices();
		_order = _pos = _tick = _fade = 0;
		_volume = 15;
		_active = true;
		_songPlayed = false;
		resetTimers();
	}

	void stop() {
		Common::StackLock lock(_mutex);
		_active = false;
		_fade = 0;
		clearVoices();
	}

	void unload() {
		stop();
		Common::StackLock lock(_mutex);
		MemFree(_module);
		_module = nullptr;
		_moduleSize = 0;
		_orders = 0;
	}

	void fadeOut() {
		Common::StackLock lock(_mutex);
		if (_active) {
			_active = false;
			_fade = 1;
		}
	}

	bool playing() const {
		Common::StackLock lock(_mutex);
		return _active;
	}

	bool songLoaded() const {
		Common::StackLock lock(_mutex);
		return _module != nullptr;
	}

	bool songPlayed() const {
		Common::StackLock lock(_mutex);
		return _songPlayed;
	}

	bool looping() const {
		Common::StackLock lock(_mutex);
		return _looping;
	}

	bool busy() const {
		Common::StackLock lock(_mutex);
		return _active || _fade;
	}

	void setLooping(bool value) {
		Common::StackLock lock(_mutex);
		_looping = value;
	}

	const char *musicName() const { return _musicName; }

	uint8 numOrders() const {
		Common::StackLock lock(_mutex);
		return _orders;
	}

	void setNumOrders(uint8 count) {
		Common::StackLock lock(_mutex);
		if (!_module || !count || count > 128)
			return;
		for (uint i = 0; i < count; ++i) {
			if (_module[472 + i] >= (_moduleSize - 2400) / 1024)
				return;
		}
		_orders = _module[470] = count;
		if (_order >= count)
			_order = 0;
	}

	void setPattern(int offset, uint8 pattern) {
		Common::StackLock lock(_mutex);
		if (_module && offset >= 0 && offset < 128 && pattern < (_moduleSize - 2400) / 1024)
			_module[472 + offset] = pattern;
	}

	int readBuffer(int16 *buffer, const int numSamples) override {
		Common::StackLock lock(_mutex);
		if (!_module || (!_active && !_fade)) {
			memset(buffer, 0, numSamples * sizeof(int16));
			return numSamples;
		}
		int remaining = numSamples;
		while (remaining) {
			const uint32 count = MIN<uint32>(remaining, MIN(_tickSamples, _envelopeSamples));
			_psg.render(buffer, count);
			buffer += count;
			remaining -= count;
			_tickSamples -= count;
			_envelopeSamples -= count;
			if (!_envelopeSamples) {
				// The native Timer C updates voices in descending channel order.
				for (int channel = 2; channel >= 0; --channel)
					updateVoice(channel);
				_envelopeSamples = schedule(getRate(), 200, _envelopeRemainder);
			}
			if (!_tickSamples) {
				updateTick();
				_tickSamples = schedule(tickNumerator(), 2457600, _tickRemainder);
			}
			if (!_active && !_fade) {
				memset(buffer, 0, remaining * sizeof(int16));
				break;
			}
		}
		return numSamples;
	}

	bool isStereo() const override { return false; }
	bool endOfData() const override { return false; }
	int getRate() const override { return _mixer->getOutputRate(); }

	void doSync(Common::Serializer &s) {
		char name[33];
		int16 loaded;
		int16 songPlayed;
		int16 looping;
		int16 pos;
		int16 order;
		int16 active;
		{
			Common::StackLock lock(_mutex);
			memcpy(name, _musicName, sizeof(name));
			loaded = _module != nullptr;
			songPlayed = _songPlayed;
			looping = _looping;
			pos = _pos;
			order = _order;
			active = _active;
		}
		s.syncBytes((byte *)name, sizeof(name));
		s.syncAsSint16LE(loaded);
		s.syncAsSint16LE(songPlayed);
		s.syncAsSint16LE(looping);
		s.syncAsSint16LE(pos);
		s.syncAsSint16LE(order);
		s.syncAsSint16LE(active);
		if (!s.isLoading())
			return;
		name[32] = 0;
		if (!loaded) {
			unload();
			Common::strlcpy(_musicName, name, sizeof(_musicName));
			Common::StackLock lock(_mutex);
			_songPlayed = songPlayed != 0;
			_looping = looping != 0;
			return;
		}
		if (!load(name))
			return;
		Common::StackLock lock(_mutex);
		_order = order >= 0 && order < _orders ? order : 0;
		_pos = pos >= 0 && pos < 1024 && !(pos % 16) ? pos : 0;
		_songPlayed = songPlayed != 0;
		_looping = looping != 0;
		// The existing save layout has no PSG envelopes or timer phase.
		for (int orderIndex = 0; orderIndex <= _order; ++orderIndex) {
			const int end = orderIndex == _order ? _pos : 1024;
			for (int row = 0; row < end; row += 16)
				playRow(orderIndex, row);
		}
		_active = active != 0;
		if (_active)
			resetTimers();
		else
			clearVoices();
	}

private:
	class PSG : public Audio::YM2149Emu {
	public:
		void render(int16 *buffer, int count) { generateSamples(buffer, count); }
	};

	struct Envelope {
		uint16 phase;
		int32 attack;
		int32 attackLimit;
		int32 decay;
		int32 sustain;
		int32 release;
		int32 value;
	};

	struct Modulation {
		int32 limit;
		int32 step;
		int32 positiveStep;
		int32 negativeLimit;
		int32 negativeStep;
		uint16 delay;
		int32 value;
	};

	struct Voice {
		uint16 duration;
		int16 tone;
		int16 period;
		int16 noise;
		uint16 volume;
		int instrument;
		Envelope volumeEnvelope;
		Envelope pitchEnvelope;
		Envelope noiseEnvelope;
		Modulation volumeModulation;
		Modulation pitchModulation;
		Modulation noiseModulation;
	};

	void clearVoices() {
		memset(_voices, 0, sizeof(_voices));
		for (uint i = 0; i < 3; ++i) {
			_voices[i].instrument = -1;
			_psg.writeReg(8 + i, 0);
		}
		_mixerRegister = 0x3f;
		_psg.writeReg(7, _mixerRegister);
	}

	uint64 tickNumerator() const {
		// The native byte subtraction wraps; a zero reload counts 256 ticks.
		const uint divisor = (250 - _module[471]) & 255;
		return (uint64)getRate() * 200 * (divisor ? divisor : 256);
	}

	static uint32 schedule(uint64 numerator, uint32 denominator, uint64 &remainder) {
		remainder += numerator;
		const uint32 samples = remainder / denominator;
		remainder %= denominator;
		return samples;
	}

	void resetTimers() {
		_tick = _fade = 0;
		_volume = 15;
		_tickRemainder = _envelopeRemainder = 0;
		_tickSamples = schedule(tickNumerator(), 2457600, _tickRemainder);
		_envelopeSamples = schedule(getRate(), 200, _envelopeRemainder);
	}

	void updateTick() {
		if (_fade && ++_fade == 33) {
			_fade = 1;
			if (!--_volume) {
				_fade = 0;
				clearVoices();
				return;
			}
		}
		if (++_tick == 12) {
			_tick = 0;
			playRow(_order, _pos);
			_pos += 16;
			if (_pos == 1024) {
				_pos = 0;
				if (++_order == _orders) {
					_order = 0;
					_songPlayed = true;
				}
			}
		}
	}

	static int32 add(int32 a, int32 b) { return (int32)((uint32)a + (uint32)b); }

	static void readEnvelope(Envelope &envelope, const byte *data, bool volume) {
		envelope.phase = READ_BE_UINT16(data);
		envelope.attack = READ_BE_INT32(data + 2);
		envelope.attackLimit = volume ? 0xf0000 : READ_BE_INT32(data + 6);
		envelope.decay = READ_BE_INT32(data + (volume ? 6 : 10));
		envelope.sustain = READ_BE_INT32(data + (volume ? 10 : 14));
		envelope.release = READ_BE_INT32(data + (volume ? 14 : 18));
		envelope.value = 0;
	}

	static void readModulation(Modulation &modulation, const byte *data, bool pitch) {
		modulation.limit = READ_BE_INT32(data);
		modulation.step = READ_BE_INT32(data + 4);
		modulation.positiveStep = pitch ? READ_BE_INT32(data + 8) : 0;
		modulation.negativeLimit = pitch ? READ_BE_INT32(data + 12) : 0;
		modulation.negativeStep = pitch ? READ_BE_INT32(data + 16) : 0;
		modulation.delay = READ_BE_UINT16(data + (pitch ? 20 : 8));
		modulation.value = 0;
	}

	void playRow(int order, int pos) {
		const byte *row = _module + 2400 + _module[472 + order] * 1024 + pos;
		for (uint channel = 0; channel < 3; ++channel) {
			Voice &voice = _voices[channel];
			const byte *event = row + channel * 4;
			const int instrument = event[2] >> 4;
			if (instrument)
				voice.instrument = instrument - 1;
			const int16 period = READ_BE_INT16(event);
			if (!period || voice.instrument < 0)
				continue;
			const byte *data = _module + 600 + voice.instrument * 120;
			_psg.writeReg(channel + 8, 0);
			voice.period = period;
			voice.duration = READ_BE_UINT16(data);
			if (!voice.duration)
				continue;
			voice.tone = READ_BE_INT16(data + 2);
			voice.noise = READ_BE_INT16(data + 4);
			voice.volume = _volume;
			readEnvelope(voice.volumeEnvelope, data + 8, true);
			readModulation(voice.volumeModulation, data + 26, false);
			readEnvelope(voice.pitchEnvelope, data + 36, false);
			readModulation(voice.pitchModulation, data + 58, true);
			readEnvelope(voice.noiseEnvelope, data + 80, false);
			readModulation(voice.noiseModulation, data + 102, false);
			_mixerRegister &= ~((1 << channel) | (8 << channel));
			if (voice.tone >= 0) {
				voice.tone = period;
				_psg.writeReg(channel * 2, voice.tone);
				if (voice.tone >= 0)
					_psg.writeReg(channel * 2 + 1, voice.tone >> 8);
			} else {
				_mixerRegister |= 1 << channel;
				voice.pitchEnvelope.phase = 0;
				voice.pitchModulation.limit = 0;
			}
			if (voice.noise >= 0)
				_psg.writeReg(6, voice.noise);
			else {
				_mixerRegister |= 8 << channel;
				voice.noiseEnvelope.phase = 0;
				voice.noiseModulation.limit = 0;
			}
			_psg.writeReg(7, _mixerRegister);
			if (!voice.volumeEnvelope.phase) {
				voice.volumeEnvelope.value = 0xf0000;
				_psg.writeReg(channel + 8, voice.volume);
			}
		}
	}

	static void updateEnvelope(Envelope &envelope, bool volume) {
		int32 step, limit;
		switch (envelope.phase) {
		case 1:
			step = envelope.attack;
			limit = envelope.attackLimit;
			break;
		case 2:
			step = envelope.decay;
			limit = envelope.sustain;
			break;
		case 4:
			step = envelope.release;
			limit = 0;
			break;
		default:
			return;
		}
		envelope.value = add(envelope.value, step);
		const bool reached = volume ? (envelope.phase == 1 ? envelope.value >= limit : envelope.value <= limit) : (step >= 0 ? envelope.value >= limit : envelope.value <= limit);
		if (reached) {
			envelope.value = limit;
			if (envelope.phase != 4)
				++envelope.phase;
			else if (volume)
				envelope.phase = 0;
		}
	}

	static void updateModulation(Modulation &modulation, bool pitch) {
		if (!modulation.limit)
			return;
		if (modulation.delay) {
			--modulation.delay;
			return;
		}
		const int32 previous = modulation.value;
		modulation.value = add(previous, modulation.step);
		if (pitch) {
			const int32 limit = modulation.step >= 0 ? modulation.limit : modulation.negativeLimit;
			if (modulation.step >= 0 ? (uint32)modulation.value < (uint32)previous : (uint32)modulation.value >= (uint32)previous)
				modulation.step = modulation.step >= 0 ? modulation.positiveStep : modulation.negativeStep;
			if (modulation.step >= 0 ? modulation.value < limit : modulation.value > limit)
				return;
			modulation.value = limit;
		} else {
			if (modulation.value < modulation.limit && modulation.value > -(int64)modulation.limit)
				return;
			modulation.value = modulation.value >= modulation.limit ? modulation.limit : (int32)-(int64)modulation.limit;
		}
		modulation.step = (int32)(0U - (uint32)modulation.step);
	}

	void updateVoice(uint channel) {
		Voice &voice = _voices[channel];
		if (!voice.duration)
			return;
		const uint16 previousPhase = voice.volumeEnvelope.phase;
		updateEnvelope(voice.volumeEnvelope, true);
		if (previousPhase == 4 && !voice.volumeEnvelope.phase)
			voice.duration = 1;
		updateModulation(voice.volumeModulation, false);
		if (voice.volumeEnvelope.phase || (voice.volumeModulation.limit >> 16)) {
			static const uint16 volumeScale[16] = { 0, 18, 35, 52, 69, 86, 103, 120, 137, 154, 171, 188, 205, 222, 237, 256 };
			const int32 value = add(voice.volumeEnvelope.value, voice.volumeModulation.value);
			const int volume = value < 0 ? 0 : MIN<int>(15, ((int16)(value >> 8) * volumeScale[voice.volume]) >> 16);
			_psg.writeReg(channel + 8, volume);
		}
		updateEnvelope(voice.pitchEnvelope, false);
		updateModulation(voice.pitchModulation, true);
		if (voice.pitchEnvelope.phase || (voice.pitchModulation.limit >> 16)) {
			const int16 amount = add(voice.pitchEnvelope.value, voice.pitchModulation.value) >> 16;
			const uint32 scaled = (uint32)((int32)amount * voice.tone) << 4;
			int delta = (int16)(scaled >> 16);
			if (scaled & 0x8000)
				++delta;
			const int tone = CLIP<int>((int16)(voice.tone + delta), 0, 4095);
			_psg.writeReg(channel * 2, tone);
			_psg.writeReg(channel * 2 + 1, tone >> 8);
		}
		updateEnvelope(voice.noiseEnvelope, false);
		updateModulation(voice.noiseModulation, false);
		if (voice.noiseEnvelope.phase || (voice.noiseModulation.limit >> 16)) {
			const int noise = (int16)((add(voice.noiseEnvelope.value, voice.noiseModulation.value) >> 16) + voice.noise);
			_psg.writeReg(6, CLIP(noise, 0, 31));
		}
		if (voice.period < 0 && !--voice.duration) {
			if (!voice.volumeEnvelope.phase) {
				_psg.writeReg(channel + 8, 0);
				return;
			}
			voice.duration = 0xffff;
			voice.volumeEnvelope.phase = 4;
			if (voice.pitchEnvelope.phase) {
				voice.pitchEnvelope.phase = 4;
				if ((int32)((uint32)voice.pitchEnvelope.release ^ (uint32)voice.pitchEnvelope.value) >= 0)
					voice.pitchEnvelope.release = (int32)(0U - (uint32)voice.pitchEnvelope.release);
			}
			if (voice.noiseEnvelope.phase) {
				voice.noiseEnvelope.phase = 4;
				if ((int32)((uint32)voice.noiseEnvelope.release ^ (uint32)voice.noiseEnvelope.value) >= 0)
					voice.noiseEnvelope.release = (int32)(0U - (uint32)voice.noiseEnvelope.release);
			}
		}

	}

	Audio::Mixer *_mixer;
	Audio::SoundHandle _handle;
	PSG _psg;
	mutable Common::Mutex _mutex;
	byte *_module;
	uint32 _moduleSize;
	Voice _voices[3];
	char _musicName[33];
	uint8 _mixerRegister;
	int _order;
	int _pos;
	uint8 _orders;
	uint _tick;
	uint _fade;
	uint _volume;
	bool _active;
	bool _songPlayed;
	bool _looping;
	uint32 _tickSamples;
	uint32 _envelopeSamples;
	uint64 _tickRemainder;
	uint64 _envelopeRemainder;
};

class AtariSoundEffects {
public:
	AtariSoundEffects(Audio::Mixer *mixer)
		: _mixer(mixer) {
	}

	~AtariSoundEffects() { stop(); }

	void play(const byte *data, int size, bool loop) {
		if (!data || size <= 0)
			return;
		stop();
		byte *pcm = (byte *)malloc(size * 4);
		if (!pcm)
			return;
		// Native three-channel PSG DAC levels, normalized to signed PCM.
		static const int16 levels[16] = { 32767, 29038, 24851, 19536, 16259, 11718, 7106, 2329, -2069, -6414, -10775, -15473, -19632, -24013, -28338, -32767 };
		for (int i = 0; i < size; ++i) {
			WRITE_LE_UINT16(pcm + i * 4, levels[data[i] >> 4]);
			WRITE_LE_UINT16(pcm + i * 4 + 2, levels[data[i] & 15]);
		}
		Audio::SeekableAudioStream *stream = Audio::makeRawStream(pcm, size * 4, 6400, Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN, DisposeAfterUse::YES);
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &_handle, Audio::makeLoopingAudioStream(stream, loop ? 0 : 1));
	}

	void stop() { _mixer->stopHandle(_handle); }

private:
	Audio::Mixer *_mixer;
	Audio::SoundHandle _handle;
};

// Sound effects use their own mixer channels, as in the Cine Amiga driver.
// Copy sample data: scripts may free or replace a resource during playback.
class AmigaSoundEffects {
public:
	AmigaSoundEffects(Audio::Mixer *mixer)
		: _mixer(mixer) {
	}

	~AmigaSoundEffects() {
		stop(-1);
	}

	void play(const byte *data, int size, int volume, int channel, int period, bool loop) {
		if (!data || size <= 0 || channel < 0 || channel >= 4 || period <= 0)
			return;

		stop(channel);
		const uint32 rate = Audio::Paula::kPalPaulaClock / period;
		if (!rate || rate >= 131072)
			return;

		byte *sample = (byte *)malloc(size);
		if (!sample) {
			warning("Unable to allocate Cruise Amiga sound effect");
			return;
		}
		memcpy(sample, data, size);

		Audio::SeekableAudioStream *stream = Audio::makeRawStream(sample, size, rate, 0, DisposeAfterUse::YES);
		static const int8 channelBalance[4] = { -64, 64, 64, -64 };
		const byte mixerVolume = CLIP(volume, 0, 63) * Audio::Mixer::kMaxChannelVolume / 63;
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &_handles[channel], Audio::makeLoopingAudioStream(stream, loop ? 0 : 1), -1, mixerVolume, channelBalance[channel]);
	}

	void stop(int channel) {
		if (channel == -1) {
			for (uint i = 0; i < 4; ++i)
				_mixer->stopHandle(_handles[i]);
		} else if (channel >= 0 && channel < 4) {
			_mixer->stopHandle(_handles[channel]);
		}
	}

	void setPeriod(int channel, int period, int volume) {
		if (channel < 0 || channel >= 4 || period <= 0)
			return;

		const uint32 rate = Audio::Paula::kPalPaulaClock / period;
		if (!rate || rate >= 131072)
			return;

		_mixer->setChannelRate(_handles[channel], rate);
		_mixer->setChannelVolume(_handles[channel], CLIP(volume, 0, 63) * Audio::Mixer::kMaxChannelVolume / 63);
	}

private:
	Audio::Mixer *_mixer;
	Audio::SoundHandle _handles[4];
};

void PCSoundDriver::setUpdateCallback(UpdateCallback upCb, void *ref) {
	_upCb = upCb;
	_upRef = ref;
}

void PCSoundDriver::findNote(int freq, int *note, int *oct) const {
	*note = _noteTableCount - 1;
	for (int i = 0; i < _noteTableCount; ++i) {
		if (_noteTable[i] <= freq) {
			*note = i;
			break;
		}
	}

	*oct = *note / 12;
	*note %= 12;
}

void PCSoundDriver::resetChannel(int channel) {
	stopChannel(channel);
	stopAll();
}

void PCSoundDriver::syncSounds() {
	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	bool music_mute = mute;
	bool sfx_mute = mute;

	if (!mute) {
		music_mute = ConfMan.getBool("music_mute");
		sfx_mute = ConfMan.getBool("sfx_mute");
	}

	// Get the new music and sfx volumes
	_musicVolume = music_mute ? 0 : MIN(255, ConfMan.getInt("music_volume"));
	_sfxVolume = sfx_mute ? 0 : MIN(255, ConfMan.getInt("sfx_volume"));
}

AdLibSoundDriver::AdLibSoundDriver(Audio::Mixer *mixer)
	: _mixer(mixer) {
	_opl = OPL::Config::create();
	if (!_opl || !_opl->init())
		error("Failed to create OPL");

	for (int i = 0; i < 5; ++i) {
		_channelsVolumeTable[i].original = 0;
		_channelsVolumeTable[i].adjusted = 0;
	}
	memset(_instrumentsTable, 0, sizeof(_instrumentsTable));
	initCard();

	_musicVolume = ConfMan.getBool("music_mute") ? 0 : MIN(255, ConfMan.getInt("music_volume"));
	_sfxVolume = ConfMan.getBool("sfx_mute") ? 0 : MIN(255, ConfMan.getInt("sfx_volume"));

	//_opl->start(new Common::Functor0Mem<void, AdLibSoundDriver>(this, &AdLibSoundDriver::onTimer), 50);
	_opl->start(new Common::Functor0Mem<void, AdLibSoundDriver>(this, &AdLibSoundDriver::onTimer), 98);
}

AdLibSoundDriver::~AdLibSoundDriver() {
	delete _opl;
}

void AdLibSoundDriver::syncSounds() {
	PCSoundDriver::syncSounds();

	// Force all instruments to reload on the next playing point
	for (int i = 0; i < 5; ++i) {
		adjustVolume(i, _channelsVolumeTable[i].original);
		AdLibSoundInstrument *ins = &_instrumentsTable[i];
		setupInstrument(ins, i);
	}
}

void AdLibSoundDriver::adjustVolume(int channel, int volume) {
	_channelsVolumeTable[channel].original = volume;

	volume = CLIP(volume, 0, 80);
	volume += volume / 4;
	// The higher possible value for volume is 100

	int volAdjust = (channel == 4) ? _sfxVolume : _musicVolume;
	volume = (volume * volAdjust) / 128;

	if (volume > 127)
		volume = 127;

	_channelsVolumeTable[channel].adjusted = volume;
}

void AdLibSoundDriver::setupChannel(int channel, const byte *data, int instrument, int volume) {
	assert(channel < 5);
	if (data) {
		adjustVolume(channel, volume);
		setupInstrument(data, channel);
	}
}

void AdLibSoundDriver::stopChannel(int channel) {
	assert(channel < 5);
	AdLibSoundInstrument *ins = &_instrumentsTable[channel];
	if (ins->mode != 0 && ins->channel == 6) {
		channel = 6;
	}
	if (ins->mode == 0 || channel == 6) {
		_opl->writeReg(0xB0 | channel, 0);
	}
	if (ins->mode != 0) {
		_vibrato &= ~(1 << (10 - ins->channel));
		_opl->writeReg(0xBD, _vibrato);
	}
}

void AdLibSoundDriver::stopAll() {
	for (int i = 0; i < 18; ++i)
		_opl->writeReg(0x40 | _operatorsTable[i], 63);

	for (int i = 0; i < 9; ++i)
		_opl->writeReg(0xB0 | i, 0);

	_opl->writeReg(0xBD, 0);
}

void AdLibSoundDriver::initCard() {
	_vibrato = 0x20;
	_opl->writeReg(0xBD, _vibrato);
	_opl->writeReg(0x08, 0x40);

	static const int oplRegs[] = { 0x40, 0x60, 0x80, 0x20, 0xE0 };

	for (int i = 0; i < 9; ++i) {
		_opl->writeReg(0xB0 | i, 0);
	}
	for (int i = 0; i < 9; ++i) {
		_opl->writeReg(0xC0 | i, 0);
	}

	for (int j = 0; j < 5; j++) {
		for (int i = 0; i < 18; ++i) {
			_opl->writeReg(oplRegs[j] | _operatorsTable[i], 0);
		}
	}

	_opl->writeReg(1, 0x20);
	_opl->writeReg(1, 0);
}

void AdLibSoundDriver::onTimer() {
	if (_upCb) {
		(*_upCb)(_upRef);
	}
}

void AdLibSoundDriver::setupInstrument(const byte *data, int channel) {
	assert(channel < 5);
	AdLibSoundInstrument *ins = &_instrumentsTable[channel];
	loadInstrument(data, ins);

	setupInstrument(ins, channel);
}

void AdLibSoundDriver::setupInstrument(const AdLibSoundInstrument *ins, int channel) {
	int mod, car, tmp;
	const AdLibRegisterSoundInstrument *reg;

	if (ins->mode != 0)  {
		mod = _operatorsTable[_voiceOperatorsTable[2 * ins->channel + 0]];
		car = _operatorsTable[_voiceOperatorsTable[2 * ins->channel + 1]];
	} else {
		mod = _operatorsTable[_voiceOperatorsTable[2 * channel + 0]];
		car = _operatorsTable[_voiceOperatorsTable[2 * channel + 1]];
	}

	if (ins->mode == 0 || ins->channel == 6) {
		reg = &ins->regMod;
		_opl->writeReg(0x20 | mod, reg->vibrato);
		if (reg->freqMod) {
			tmp = reg->outputLevel & 0x3F;
		} else {
			tmp = (63 - (reg->outputLevel & 0x3F)) * _channelsVolumeTable[channel].adjusted;
			tmp = 63 - (2 * tmp + 127) / (2 * 127);
		}
		_opl->writeReg(0x40 | mod, tmp | (reg->keyScaling << 6));
		_opl->writeReg(0x60 | mod, reg->attackDecay);
		_opl->writeReg(0x80 | mod, reg->sustainRelease);
		if (ins->mode != 0) {
			_opl->writeReg(0xC0 | ins->channel, reg->feedbackStrength);
		} else {
			_opl->writeReg(0xC0 | channel, reg->feedbackStrength);
		}
		_opl->writeReg(0xE0 | mod, ins->waveSelectMod);
	}

	reg = &ins->regCar;
	_opl->writeReg(0x20 | car, reg->vibrato);
	tmp = (63 - (reg->outputLevel & 0x3F)) * _channelsVolumeTable[channel].adjusted;
	tmp = 63 - (2 * tmp + 127) / (2 * 127);
	_opl->writeReg(0x40 | car, tmp | (reg->keyScaling << 6));
	_opl->writeReg(0x60 | car, reg->attackDecay);
	_opl->writeReg(0x80 | car, reg->sustainRelease);
	_opl->writeReg(0xE0 | car, ins->waveSelectCar);
}

void AdLibSoundDriver::loadRegisterInstrument(const byte *data, AdLibRegisterSoundInstrument *reg) {
	reg->vibrato = 0;
	if (READ_LE_UINT16(data + 18)) { // amplitude vibrato
		reg->vibrato |= 0x80;
	}
	if (READ_LE_UINT16(data + 20)) { // frequency vibrato
		reg->vibrato |= 0x40;
	}
	if (READ_LE_UINT16(data + 10)) { // sustaining sound
		reg->vibrato |= 0x20;
	}
	if (READ_LE_UINT16(data + 22)) { // envelope scaling
		reg->vibrato |= 0x10;
	}
	reg->vibrato |= READ_LE_UINT16(data + 2) & 0xF; // frequency multiplier

	reg->attackDecay = READ_LE_UINT16(data + 6) << 4; // attack rate
	reg->attackDecay |= READ_LE_UINT16(data + 12) & 0xF; // decay rate

	reg->sustainRelease = READ_LE_UINT16(data + 8) << 4; // sustain level
	reg->sustainRelease |= READ_LE_UINT16(data + 14) & 0xF; // release rate

	reg->feedbackStrength = READ_LE_UINT16(data + 4) << 1; // feedback
	if (READ_LE_UINT16(data + 24) == 0) { // frequency modulation
		reg->feedbackStrength |= 1;
	}

	reg->keyScaling = READ_LE_UINT16(data);
	reg->outputLevel = READ_LE_UINT16(data + 16);
	reg->freqMod = READ_LE_UINT16(data + 24);
}

void AdLibSoundDriverADL::loadInstrument(const byte *data, AdLibSoundInstrument *asi) {
	asi->mode = *data++;
	asi->channel = *data++;
	asi->waveSelectMod = *data++ & 3;
	asi->waveSelectCar = *data++ & 3;
	asi->amDepth = *data++;
	++data;
	loadRegisterInstrument(data, &asi->regMod); data += 26;
	loadRegisterInstrument(data, &asi->regCar); data += 26;
}

void AdLibSoundDriverADL::setChannelFrequency(int channel, int frequency) {
	assert(channel < 5);
	AdLibSoundInstrument *ins = &_instrumentsTable[channel];
	if (ins->mode != 0) {
		channel = ins->channel;
		if (channel == 9) {
			channel = 8;
		} else if (channel == 10) {
			channel = 7;
		}
	}
	int freq, note, oct;
	findNote(frequency, &note, &oct);

	note += oct * 12;
	if (ins->amDepth) {
		note = ins->amDepth;
	}
	if (note < 0) {
		note = 0;
	}

	freq = _freqTable[note % 12];
	_opl->writeReg(0xA0 | channel, freq);
	freq = ((note / 12) << 2) | ((freq & 0x300) >> 8);
	if (ins->mode == 0) {
		freq |= 0x20;
	}
	_opl->writeReg(0xB0 | channel, freq);
	if (ins->mode != 0) {
		_vibrato |= 1 << (10 - channel);
		_opl->writeReg(0xBD, _vibrato);
	}
}

void AdLibSoundDriverADL::playSample(const byte *data, int size, int channel, int volume) {
	assert(channel < 5);
	adjustVolume(channel, 127);

	setupInstrument(data, channel);
	AdLibSoundInstrument *ins = &_instrumentsTable[channel];
	if (ins->mode != 0 && ins->channel == 6) {
		_opl->writeReg(0xB0 | channel, 0);
	}
	if (ins->mode != 0) {
		_vibrato &= ~(1 << (10 - ins->channel));
		_opl->writeReg(0xBD, _vibrato);
	}
	if (ins->mode != 0) {
		channel = ins->channel;
		if (channel == 9) {
			channel = 8;
		} else if (channel == 10) {
			channel = 7;
		}
	}
	uint16 note = 48;
	if (ins->amDepth) {
		note = ins->amDepth;
	}
	int freq = _freqTable[note % 12];
	_opl->writeReg(0xA0 | channel, freq);
	freq = ((note / 12) << 2) | ((freq & 0x300) >> 8);
	if (ins->mode == 0) {
		freq |= 0x20;
	}
	_opl->writeReg(0xB0 | channel, freq);
	if (ins->mode != 0) {
		_vibrato |= 1 << (10 - channel);
		_opl->writeReg(0xBD, _vibrato);
	}
}

PCSoundFxPlayer::PCSoundFxPlayer(PCSoundDriver *driver)
	: _playing(false), _songPlayed(false), _driver(driver) {
	memset(_instrumentsData, 0, sizeof(_instrumentsData));
	_sfxData = nullptr;
	_fadeOutCounter = 0;
	_driver->setUpdateCallback(updateCallback, this);

	_currentPos = 0;
	_currentOrder = 0;
	_numOrders = 0;
	_eventsDelay = 0;
	_looping = false;
	_updateTicksCounter = 0;
}

PCSoundFxPlayer::~PCSoundFxPlayer() {
	_driver->setUpdateCallback(nullptr, nullptr);
	stop();
}

bool PCSoundFxPlayer::load(const char *song) {
	debug(9, "PCSoundFxPlayer::load('%s')", song);

	/* stop (w/ fade out) the previous song */
	while (_fadeOutCounter != 0 && _fadeOutCounter < 100) {
		g_system->delayMillis(50);
	}
	_fadeOutCounter = 0;

	if (_playing) {
		stop();
	}

	Common::strlcpy(_musicName, song, sizeof(_musicName));
	_songPlayed = false;
	_looping = false;
	_sfxData = readBundleSoundFile(song);
	if (!_sfxData) {
		warning("Unable to load soundfx module '%s'", song);
		return 0;
	}

	for (int i = 0; i < NUM_INSTRUMENTS; ++i) {
		_instrumentsData[i] = nullptr;

		char instrument[64];
		memset(instrument, 0, 64); // Clear the data first
		memcpy(instrument, _sfxData + 20 + i * 30, 12);
		instrument[63] = '\0';

		if (strlen(instrument) != 0) {
			char *dot = strrchr(instrument, '.');
			if (dot) {
				*dot = '\0';
			}
			Common::strlcat(instrument, _driver->getInstrumentExtension(), sizeof(instrument));
			_instrumentsData[i] = readBundleSoundFile(instrument);
			if (!_instrumentsData[i]) {
				warning("Unable to load soundfx instrument '%s'", instrument);
			}
		}
	}
	return 1;
}

void PCSoundFxPlayer::play() {
	debug(9, "PCSoundFxPlayer::play()");
	if (_sfxData) {
		for (int i = 0; i < NUM_CHANNELS; ++i) {
			_instrumentsChannelTable[i] = -1;
		}
		_currentPos = 0;
		_currentOrder = 0;
		_numOrders = _sfxData[470];
		_eventsDelay = (244 - _sfxData[471]) * 100 / 1060;
		_updateTicksCounter = 0;
		_playing = true;
	}
}

void PCSoundFxPlayer::stop() {
	if (_playing || _fadeOutCounter != 0) {
		_fadeOutCounter = 0;
		_playing = false;
		for (int i = 0; i < NUM_CHANNELS; ++i) {
			_driver->stopChannel(i);
		}
		_driver->stopAll();
	}
	unload();
}

void PCSoundFxPlayer::fadeOut() {
	if (_playing) {
		_fadeOutCounter = 1;
		_playing = false;
	}
}

void PCSoundFxPlayer::updateCallback(void *ref) {
	((PCSoundFxPlayer *)ref)->update();
}

void PCSoundFxPlayer::update() {
	if (_playing || (_fadeOutCounter != 0 && _fadeOutCounter < 100)) {
		++_updateTicksCounter;
		if (_updateTicksCounter > _eventsDelay) {
			handleEvents();
			_updateTicksCounter = 0;
		}
	}
}

void PCSoundFxPlayer::handleEvents() {
	const byte *patternData = _sfxData + 600 + 1800;
	const byte *orderTable = _sfxData + 472;
	uint16 patternNum = orderTable[_currentOrder] * 1024;

	for (int i = 0; i < 4; ++i) {
		handlePattern(i, patternData + patternNum + _currentPos);
		patternData += 4;
	}

	if (_fadeOutCounter != 0 && _fadeOutCounter < 100) {
		_fadeOutCounter += 2;
	}
	if (_fadeOutCounter >= 100) {
		stop();
		return;
	}

	_currentPos += 16;
	if (_currentPos >= 1024) {
		_currentPos = 0;
		++_currentOrder;
		if (_currentOrder == _numOrders) {
			_currentOrder = 0;
		}
	}
	debug(7, "_currentOrder=%d/%d _currentPos=%d", _currentOrder, _numOrders, _currentPos);
}

void PCSoundFxPlayer::handlePattern(int channel, const byte *patternData) {
	int instrument = patternData[2] >> 4;
	if (instrument != 0) {
		--instrument;
		if (_instrumentsChannelTable[channel] != instrument || _fadeOutCounter != 0) {
			_instrumentsChannelTable[channel] = instrument;
			const int volume = _sfxData[instrument] - _fadeOutCounter;
			_driver->setupChannel(channel, _instrumentsData[instrument], instrument, volume);
		}
	}
	int16 freq = (int16)READ_BE_UINT16(patternData);
	if (freq > 0) {
		_driver->stopChannel(channel);
		_driver->setChannelFrequency(channel, freq);
	}
}

void PCSoundFxPlayer::unload() {
	for (int i = 0; i < NUM_INSTRUMENTS; ++i) {
		MemFree(_instrumentsData[i]);
		_instrumentsData[i] = nullptr;
	}
	MemFree(_sfxData);
	_sfxData = nullptr;
	_songPlayed = true;
}

void PCSoundFxPlayer::doSync(Common::Serializer &s) {
	s.syncBytes((byte *)_musicName, 33);
	uint16 v = (uint16)songLoaded();
	s.syncAsSint16LE(v);

	if (s.isLoading() && v) {
		load(_musicName);

		for (int i = 0; i < NUM_CHANNELS; ++i) {
			_instrumentsChannelTable[i] = -1;
		}

		_numOrders = _sfxData[470];
		_eventsDelay = (244 - _sfxData[471]) * 100 / 1060;
		_updateTicksCounter = 0;
	}

	s.syncAsSint16LE(_songPlayed);
	s.syncAsSint16LE(_looping);
	s.syncAsSint16LE(_currentPos);
	s.syncAsSint16LE(_currentOrder);
	s.syncAsSint16LE(_playing);
}

PCSound::PCSound(Audio::Mixer *mixer, CruiseEngine *vm) {
	_vm = vm;
	_mixer = mixer;
	_soundDriver = nullptr;
	_player = nullptr;
	_amigaPlayer = nullptr;
	_amigaEffects = nullptr;
	_atariPlayer = nullptr;
	_atariEffects = nullptr;
	_mt32Player = nullptr;
	_genVolume = 0;
	if (vm->getPlatform() == Common::kPlatformDOS || vm->getPlatform() == Common::kPlatformAtariST) {
		const uint32 flags = MDT_MIDI | MDT_PREFER_MT32 | (vm->getPlatform() == Common::kPlatformDOS ? MDT_ADLIB : 0);
		const MidiDriver::DeviceHandle device = MidiDriver::detectDevice(flags);
		const MusicType type = MidiDriver::getMusicType(device);
		if (type == MT_MT32 || ((type == MT_GM || type == MT_GS) && ConfMan.getBool("native_mt32"))) {
			MidiDriver_MT32GM *output = new MidiDriver_MT32GM(MT_MT32);
			if (output->open(MidiDriver::createMidi(device), true) == 0)
				_mt32Player = new MT32SoundFxPlayer(output);
			else {
				warning("Unable to open Cruise MT-32 output; using native audio");
				delete output;
			}
		}
	}
	if (_mt32Player)
		return;
	if (vm->getPlatform() == Common::kPlatformAmiga) {
		_amigaPlayer = new AmigaSoundFxPlayer(_mixer);
		_amigaEffects = new AmigaSoundEffects(_mixer);
	} else if (vm->getPlatform() == Common::kPlatformAtariST) {
		_atariPlayer = new AtariSoundFxPlayer(_mixer);
		_atariEffects = new AtariSoundEffects(_mixer);
	} else {
		_soundDriver = new AdLibSoundDriverADL(_mixer);
		_player = new PCSoundFxPlayer(_soundDriver);
	}
}

PCSound::~PCSound() {
	delete _mt32Player;
	delete _atariEffects;
	delete _atariPlayer;
	delete _amigaEffects;
	delete _amigaPlayer;
	delete _player;
	delete _soundDriver;
}

void PCSound::loadMusic(const char *name) {
	if (_amigaPlayer) {
		_amigaPlayer->load(name);
		return;
	}
	if (_atariPlayer) {
		_atariPlayer->load(name);
		return;
	}
	if (_mt32Player) {
		_mt32Player->load(name);
		return;
	}
	debugC(5, kCruiseDebugSound, "PCSound::loadMusic('%s')", name);
	_player->load(name);
}

void PCSound::playMusic() {
	if (_atariEffects)
		_atariEffects->stop();
	if (_amigaPlayer) {
		_amigaPlayer->play();
		return;
	}
	if (_atariPlayer) {
		_atariPlayer->play();
		return;
	}
	if (_mt32Player) {
		_mt32Player->play();
		return;
	}
	debugC(5, kCruiseDebugSound, "PCSound::playMusic()");
	_player->play();
}

void PCSound::stopMusic() {
	if (_amigaPlayer) {
		_amigaPlayer->stop();
		return;
	}
	if (_atariPlayer) {
		_atariPlayer->stop();
		return;
	}
	if (_mt32Player) {
		_mt32Player->stop();
		return;
	}
	debugC(5, kCruiseDebugSound, "PCSound::stopMusic()");
	_player->stop();
}

void PCSound::removeMusic() {
	if (_amigaPlayer) {
		_amigaPlayer->unload();
		return;
	}
	if (_atariPlayer) {
		_atariPlayer->unload();
		return;
	}
	if (_mt32Player) {
		_mt32Player->unload();
		return;
	}
	debugC(5, kCruiseDebugSound, "PCSound::removeMusic()");
	_player->unload();
}

void PCSound::fadeOutMusic() {
	if (_amigaPlayer) {
		_amigaPlayer->fadeOut();
		return;
	}
	if (_atariPlayer) {
		_atariPlayer->fadeOut();
		return;
	}
	if (_mt32Player) {
		_mt32Player->fadeOut();
		return;
	}
	debugC(5, kCruiseDebugSound, "PCSound::fadeOutMusic()");
	_player->fadeOut();
}

void PCSound::playSound(const uint8 *data, int size, int volume, int channel, int period, bool loop) {
	if (_mt32Player) {
		_mt32Player->playEffect(data, size, channel, volume);
		return;
	}
	if (_atariEffects) {
		if (!_atariPlayer->busy())
			_atariEffects->play(data, size, loop);
		return;
	}
	if (_amigaEffects) {
		_amigaEffects->play(data, size, volume, channel, period, loop);
		return;
	}
	debugC(5, kCruiseDebugSound, "PCSound::playSound() channel %d size %d", 4, size);
	_soundDriver->playSample(data, size, 4, volume);
}

void PCSound::stopSound(int channel) {
	if (_mt32Player) {
		_mt32Player->stopEffect(channel);
		return;
	}
	if (_atariEffects) {
		_atariEffects->stop();
		return;
	}
	if (_amigaEffects) {
		_amigaEffects->stop(channel);
		return;
	}
	debugC(5, kCruiseDebugSound, "PCSound::stopSound() channel %d", channel);
	_soundDriver->resetChannel(channel);
}

void PCSound::stopChannel(int channel) {
	if (_mt32Player) {
		_mt32Player->stopEffect(channel);
		return;
	}
	if (_atariEffects) {
		_atariEffects->stop();
		return;
	}
	if (_amigaEffects) {
		_amigaEffects->stop(channel);
		return;
	}
	debugC(5, kCruiseDebugSound, "PCSound::stopChannel() channel %d", channel);
	_soundDriver->stopChannel(channel);
}

bool PCSound::isPlaying() const {
	if (_amigaPlayer)
		return _amigaPlayer->playing();
	if (_atariPlayer)
		return _atariPlayer->playing();
	if (_mt32Player)
		return _mt32Player->playing();
	return _player->playing();
}

bool PCSound::songLoaded() const {
	if (_amigaPlayer)
		return _amigaPlayer->songLoaded();
	if (_atariPlayer)
		return _atariPlayer->songLoaded();
	if (_mt32Player)
		return _mt32Player->songLoaded();
	return _player->songLoaded();
}

bool PCSound::songPlayed() const {
	if (_amigaPlayer)
		return _amigaPlayer->songPlayed();
	if (_atariPlayer)
		return _atariPlayer->songPlayed();
	if (_mt32Player)
		return _mt32Player->songPlayed();
	return _player->songPlayed();
}

void PCSound::fadeSong() {
	if (_amigaPlayer) {
		_amigaPlayer->fadeOut();
		return;
	}
	if (_atariPlayer) {
		_atariPlayer->fadeOut();
		return;
	}
	if (_mt32Player) {
		_mt32Player->fadeOut();
		return;
	}
	_player->fadeOut();
}

uint8 PCSound::numOrders() const {
	if (_amigaPlayer)
		return _amigaPlayer->numOrders();
	if (_atariPlayer)
		return _atariPlayer->numOrders();
	if (_mt32Player)
		return _mt32Player->numOrders();
	return _player->numOrders();
}

void PCSound::setNumOrders(uint8 v) {
	if (_amigaPlayer) {
		_amigaPlayer->setNumOrders(v);
		return;
	}
	if (_atariPlayer) {
		_atariPlayer->setNumOrders(v);
		return;
	}
	if (_mt32Player) {
		_mt32Player->setNumOrders(v);
		return;
	}
	_player->setNumOrders(v);
}

void PCSound::setPattern(int offset, uint8 value) {
	if (_amigaPlayer) {
		_amigaPlayer->setPattern(offset, value);
		return;
	}
	if (_atariPlayer) {
		_atariPlayer->setPattern(offset, value);
		return;
	}
	if (_mt32Player) {
		_mt32Player->setPattern(offset, value);
		return;
	}
	_player->setPattern(offset, value);
}

bool PCSound::musicLooping() const {
	if (_amigaPlayer)
		return _amigaPlayer->looping();
	if (_atariPlayer)
		return _atariPlayer->looping();
	if (_mt32Player)
		return _mt32Player->looping();
	return _player->looping();
}

void PCSound::musicLoop(bool v) {
	if (_amigaPlayer) {
		_amigaPlayer->setLooping(v);
		return;
	}
	if (_atariPlayer) {
		_atariPlayer->setLooping(v);
		return;
	}
	if (_mt32Player) {
		_mt32Player->setLooping(v);
		return;
	}
	_player->setLooping(v);
}

void PCSound::startNote(int channel, int volume, int freq) {
	if (_mt32Player) {
		_mt32Player->setEffectPeriod(channel, freq, volume);
		return;
	}
	if (_atariEffects)
		return; // The ST PSG sample interrupt runs at a fixed 6400 Hz.
	if (_amigaEffects) {
		_amigaEffects->setPeriod(channel, freq, volume);
		return;
	}
	warning("TODO: startNote");
//	_soundDriver->setVolume(channel, volume);
	_soundDriver->setChannelFrequency(channel, freq);
}

void PCSound::doSync(Common::Serializer &s) {
	if (s.isLoading() && _amigaEffects)
		_amigaEffects->stop(-1);
	if (s.isLoading() && _atariEffects)
		_atariEffects->stop();
	if (_amigaPlayer)
		_amigaPlayer->doSync(s);
	else if (_atariPlayer)
		_atariPlayer->doSync(s);
	else if (_mt32Player)
		_mt32Player->doSync(s);
	else
		_player->doSync(s);
	s.syncAsSint16LE(_genVolume);
}

const char *PCSound::musicName() {
	if (_amigaPlayer)
		return _amigaPlayer->musicName();
	if (_atariPlayer)
		return _atariPlayer->musicName();
	if (_mt32Player)
		return _mt32Player->musicName();
	return _player->musicName();
}

void PCSound::syncSounds() {
	if (_mt32Player) {
		_mt32Player->syncSounds();
		return;
	}
	if (_amigaPlayer || _atariPlayer)
		return; // Native music volume and mute are applied by the mixer.
	_soundDriver->syncSounds();
}

void PCSound::pause(bool paused) {
	if (_mt32Player)
		_mt32Player->pause(paused);
}

} // End of namespace Cruise
