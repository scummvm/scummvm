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

#include "audio/fmopl.h"
#include "audio/mixer.h"
#include "audio/mods/paula.h"

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
	if (vm->getPlatform() == Common::kPlatformAmiga)
		_amigaPlayer = new AmigaSoundFxPlayer(_mixer);
	else {
		_soundDriver = new AdLibSoundDriverADL(_mixer);
		_player = new PCSoundFxPlayer(_soundDriver);
	}
	_genVolume = 0;
}

PCSound::~PCSound() {
	delete _amigaPlayer;
	delete _player;
	delete _soundDriver;
}

void PCSound::loadMusic(const char *name) {
	if (_amigaPlayer) {
		_amigaPlayer->load(name);
		return;
	}
	debugC(5, kCruiseDebugSound, "PCSound::loadMusic('%s')", name);
	_player->load(name);
}

void PCSound::playMusic() {
	if (_amigaPlayer) {
		_amigaPlayer->play();
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
	debugC(5, kCruiseDebugSound, "PCSound::stopMusic()");
	_player->stop();
}

void PCSound::removeMusic() {
	if (_amigaPlayer) {
		_amigaPlayer->unload();
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
	debugC(5, kCruiseDebugSound, "PCSound::fadeOutMusic()");
	_player->fadeOut();
}

void PCSound::playSound(const uint8 *data, int size, int volume) {
	if (_amigaPlayer)
		return; // Amiga sound effects are outside the music player.
	debugC(5, kCruiseDebugSound, "PCSound::playSound() channel %d size %d", 4, size);
	_soundDriver->playSample(data, size, 4, volume);
}

void PCSound::stopSound(int channel) {
	if (_amigaPlayer)
		return; // Amiga sound effects are outside the music player.
	debugC(5, kCruiseDebugSound, "PCSound::stopSound() channel %d", channel);
	_soundDriver->resetChannel(channel);
}

void PCSound::stopChannel(int channel) {
	if (_amigaPlayer) {
		_amigaPlayer->stopChannel(channel);
		return;
	}
	debugC(5, kCruiseDebugSound, "PCSound::stopChannel() channel %d", channel);
	_soundDriver->stopChannel(channel);
}

bool PCSound::isPlaying() const {
	if (_amigaPlayer)
		return _amigaPlayer->playing();
	return _player->playing();
}

bool PCSound::songLoaded() const {
	if (_amigaPlayer)
		return _amigaPlayer->songLoaded();
	return _player->songLoaded();
}

bool PCSound::songPlayed() const {
	if (_amigaPlayer)
		return _amigaPlayer->songPlayed();
	return _player->songPlayed();
}

void PCSound::fadeSong() {
	if (_amigaPlayer) {
		_amigaPlayer->fadeOut();
		return;
	}
	_player->fadeOut();
}

uint8 PCSound::numOrders() const {
	if (_amigaPlayer)
		return _amigaPlayer->numOrders();
	return _player->numOrders();
}

void PCSound::setNumOrders(uint8 v) {
	if (_amigaPlayer) {
		_amigaPlayer->setNumOrders(v);
		return;
	}
	_player->setNumOrders(v);
}

void PCSound::setPattern(int offset, uint8 value) {
	if (_amigaPlayer) {
		_amigaPlayer->setPattern(offset, value);
		return;
	}
	_player->setPattern(offset, value);
}

bool PCSound::musicLooping() const {
	if (_amigaPlayer)
		return _amigaPlayer->looping();
	return _player->looping();
}

void PCSound::musicLoop(bool v) {
	if (_amigaPlayer) {
		_amigaPlayer->setLooping(v);
		return;
	}
	_player->setLooping(v);
}

void PCSound::startNote(int channel, int volume, int freq) {
	if (_amigaPlayer)
		return; // Amiga sound effects are outside the music player.
	warning("TODO: startNote");
//	_soundDriver->setVolume(channel, volume);
	_soundDriver->setChannelFrequency(channel, freq);
}

void PCSound::doSync(Common::Serializer &s) {
	if (_amigaPlayer)
		_amigaPlayer->doSync(s);
	else
		_player->doSync(s);
	s.syncAsSint16LE(_genVolume);
}

const char *PCSound::musicName() {
	if (_amigaPlayer)
		return _amigaPlayer->musicName();
	return _player->musicName();
}

void PCSound::syncSounds() {
	if (_amigaPlayer)
		return; // Music volume/mute is applied by the mixer to the Paula stream.
	_soundDriver->syncSounds();
}

} // End of namespace Cruise
