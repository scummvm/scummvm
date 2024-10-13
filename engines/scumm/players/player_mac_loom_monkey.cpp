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


#include "scumm/players/player_mac_new.h"
#include "scumm/players/player_mac_intern.h"
#include "scumm/resource.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "common/punycode.h"
#include "common/macresman.h"
#include "common/memstream.h"

namespace Scumm {

#define ASC_DEVICE_RATE		0x56EE8BA3
#define PCM_BUFFER_SIZE		1024

class MacSndLoader{
protected:
	MacSndLoader(bool useInstrTag);
public:
	virtual ~MacSndLoader() {}

	virtual bool init() = 0;
	virtual bool checkResource(const byte *data, uint32 dataSize) const = 0;
	virtual bool blocked(const byte *data, uint32 dataSize) const = 0;
	virtual bool loadSound(const byte *data, uint32 dataSize) = 0;
	virtual void unblock() = 0;
	void setSynthType(byte type) { _synth = type; }

	const MacLowLevelPCMDriver::PCMSound *getInstrData(uint16 chan);
	virtual bool isInstrUsable(uint16 chan) const = 0;
	virtual bool parseNextEvent(uint16 chan, uint16 &duration, uint8 &note, bool &skip, bool &updateInstr) = 0;
	virtual uint16 getChanSetup() const = 0;
	uint16 getTimbre() const { return _timbre; }
	virtual bool isMusic() const = 0;
	bool isLooping() const { return _loop; }
	virtual bool restartSoundAfterLoad() const = 0;

	virtual bool ignoreMachineRating() const = 0;

protected:
	bool loadInstruments(const char *const *tryFileNames, uint16 numTryFileNames, uint16 numInstruments);

	struct Instrument {
	public:
		Instrument(uint32 id, Common::SeekableReadStream *&in, Common::String &&name);
		~Instrument() { _res.reset(); }
		const MacLowLevelPCMDriver::PCMSound *data() const { return &_snd; }
		uint32 id() const { return _id; }

	private:
		uint32 _id;
		Common::SharedPtr<const byte> _res;
		Common::String _name;
		MacLowLevelPCMDriver::PCMSound _snd;
	};

	Common::Array<Common::SharedPtr<Instrument> > _instruments;

	byte _sndRes6;
	byte _isMusic;
	byte _sndRes9;
	byte _sndRes10;
	uint16 _chanSetup;
	uint16 _timbre;
	bool _loop;
	byte _synth;
	const byte *_chanSndData[5];
	uint32 _chanNumEvents[5];
	uint32 _chanCurEvent[5];
	const Common::SharedPtr<Instrument> *_chanInstr[5];
	const Common::SharedPtr<Instrument> *_chanInstr2[5];

	const bool _useInstrTag;
};

class LoomMacSndLoader final : public MacSndLoader {
public:
	LoomMacSndLoader() : MacSndLoader(false) {}
	~LoomMacSndLoader() override {}

	bool init() override;
	bool checkResource(const byte *data, uint32 dataSize) const override;
	bool blocked(const byte *data, uint32 dataSize) const override { return false; }
	bool loadSound(const byte *data, uint32 dataSize) override;
	void unblock() override {}

	bool isInstrUsable(uint16 chan) const override;
	bool parseNextEvent(uint16 chan, uint16 &duration, uint8 &note, bool &skip, bool &updateInstr) override;
	uint16 getChanSetup() const override { return _chanSetup; }
	bool isMusic() const override { return (_chanSetup == 0); }
	bool restartSoundAfterLoad() const override { return true; }
	bool ignoreMachineRating() const override { return false; }
private:
	const Common::SharedPtr<Instrument> *fetchInstrument(uint16 id) const;
};

class MonkeyMacSndLoader final : public MacSndLoader {
public:
	MonkeyMacSndLoader() : MacSndLoader(true), _numInstrumentsMax(17), _blockSfx(false), _transpose(0) {}
	~MonkeyMacSndLoader() override {}

	bool init() override;
	bool checkResource(const byte *data, uint32 dataSize) const override;
	bool blocked(const byte *data, uint32 dataSize) const override;
	bool loadSound(const byte *data, uint32 dataSize) override;
	void unblock() override { _blockSfx = false; }

	bool isInstrUsable(uint16 chan) const override;
	bool parseNextEvent(uint16 chan, uint16 &duration, uint8 &note, bool &skip, bool &updateInstr) override;
	uint16 getChanSetup() const override { return _isMusic ? _chanSetup : 7; }
	bool isMusic() const override { return _isMusic; }
	bool restartSoundAfterLoad() const override { return _isMusic && _loop; }
	bool ignoreMachineRating() const override { return true; }
private:
	const Common::SharedPtr<Instrument> *fetchInstrument(const byte *data, uint32 dataSize, uint32 tagOrOffset);
	bool _blockSfx;
	byte _transpose;
	const byte _numInstrumentsMax;
};

MacSndLoader::Instrument::Instrument(uint32 id, Common::SeekableReadStream *&in, Common::String &&name) : _id(id), _name(Common::move(name)) {
	in->seek(2);
	uint16 numTypes = in->readUint16BE();
	in->seek(numTypes * 6 + 4);
	in->seek(in->readUint16BE() * 8 + numTypes * 6 + 10);

	_snd.len = in->readUint32BE();
	_snd.rate = in->readUint32BE();
	_snd.loopst = in->readUint32BE();
	_snd.loopend = in->readUint32BE();
	_snd.enc = in->readByte();
	_snd.baseFreq = in->readByte();

	byte *buff = new byte[_snd.len];
	if (in->read(buff, _snd.len) != _snd.len)
		error("MacSndLoader::Instrument::Instrument(): Data error");
	_snd.data = Common::SharedPtr<const byte>(buff, Common::ArrayDeleter<const byte>());
}

MacSndLoader::MacSndLoader(bool useInstrTag) : _sndRes6(0), _isMusic(0), _sndRes9(0), _sndRes10(0), _chanSetup(0),
	_timbre(0), _useInstrTag(useInstrTag), _synth(0), _loop(false) {
	memset(_chanInstr, 0, sizeof(_chanInstr));
	memset(_chanInstr2, 0, sizeof(_chanInstr2));
	memset(_chanSndData, 0, sizeof(_chanSndData));
	memset(_chanNumEvents, 0, sizeof(_chanNumEvents));
	memset(_chanCurEvent, 0, sizeof(_chanCurEvent));
}

const MacLowLevelPCMDriver::PCMSound *MacSndLoader::getInstrData(uint16 chan) {
	return (chan < ARRAYSIZE(_chanInstr) && _chanInstr[chan]) ? _chanInstr[chan]->get()->data() : nullptr;
}

bool MacSndLoader::loadInstruments(const char *const *tryFileNames, uint16 numTryFileNames, uint16 numInstruments) {
	assert(tryFileNames && numTryFileNames && numInstruments);
	uint32 tag = 0;
	const Common::CodePage tryCodePages[] = {
		Common::kMacRoman,
		Common::kISO8859_1
	};

	Common::MacResManager resMan;
	Common::Path resFile;
	for (int i = 0; resFile.empty() && i < numTryFileNames; ++i) {
		for (int ii = 0; resFile.empty() && ii < ARRAYSIZE(tryCodePages); ++ii) {
			Common::U32String fn(tryFileNames[i], tryCodePages[ii]);
			resFile = Common::Path(fn.encode(Common::kUtf8));
			if (!resMan.exists(resFile) || !resMan.open(resFile) || !resMan.hasResFork()) {
				resMan.close();
				resFile = Common::Path(Common::punycode_encodefilename(fn));
				if (!resMan.exists(resFile) || !resMan.open(resFile) || !resMan.hasResFork()) {
					resMan.close();
					resFile.clear();
				}
			}
		}
	}

	if (resFile.empty()) {
		warning("MacSndLoader::loadInstruments(): Resource fork not found");
		return false;
	}

	Common::MacResIDArray ids = resMan.getResIDArray(MKTAG('s', 'n', 'd', ' '));
	for (uint i = 0; i < ids.size(); ++i) {
		Common::SeekableReadStream *str = resMan.getResource(MKTAG('s', 'n', 'd', ' '), ids[i]);
		if (!str || str->readUint16BE() != 1) {
			static const char *const errStr[2] = {"Failed to load", "Invalid sound resource format for"};
			warning("MacSndLoader::loadInstruments(): %s instrument with id 0x%04x", errStr[str ? 1 : 0], ids[i]);
			delete str;
			return false;
		}
		Common::String nm(resMan.getResName(MKTAG('s', 'n', 'd', ' '), ids[i]));
		memcpy(&tag, nm.c_str(), MIN<uint>(nm.size(), sizeof(tag)));
		uint32 id = _useInstrTag ? FROM_BE_32(tag) : ids[i];
		_instruments.push_back(Common::SharedPtr<Instrument>(new Instrument(id, str, Common::move(nm))));
		delete str;
	}

	if (_instruments.size() != numInstruments)
		warning("MacSndLoader::loadInstruments(): Unexpected number of instruments found (expected: %d, found: %d)", numInstruments, _instruments.size());

	return true;
}

bool LoomMacSndLoader::init() {
	static const char *execNames[] = {
		"Loom",
		"Loom\xaa",
		"Loom PPC",
		"Loom\xaa PPC"
	};

	return loadInstruments(execNames, ARRAYSIZE(execNames), 10);
}

bool LoomMacSndLoader::checkResource(const byte *data, uint32 dataSize) const {
	return (dataSize >= 14 && READ_BE_UINT16(data + 4) == 'so' && !READ_BE_UINT32(data + 10));
}

bool LoomMacSndLoader::loadSound(const byte *data, uint32 dataSize) {
	if (dataSize < 40)
		return false;

	_sndRes6 = READ_BE_UINT16(data + 6) & 0xff;
	_isMusic = READ_BE_UINT16(data + 8) >> 8;
	_sndRes10 = READ_BE_UINT16(data + 10) >> 8;
	_chanSetup = READ_BE_UINT16(data + 16);
	_timbre = READ_BE_UINT16(data + 18);
	for (int i = 0; i < 5; ++i) {
		_chanInstr[i] = fetchInstrument(READ_BE_UINT16(data + 20 + 2 * i));
		_chanSndData[i] = data + READ_BE_UINT16(data + 30 + 2 * i) + 6;
		_chanNumEvents[i] = READ_BE_UINT16(_chanSndData[i] - 2);
		_chanCurEvent[i] = 0;
	}

	return true;
}

bool LoomMacSndLoader::isInstrUsable(uint16 chan) const {
	return (chan < ARRAYSIZE(_chanInstr) && _chanInstr[chan] && _chanInstr[chan]->get()->id() != 0x2D1C);
}

bool LoomMacSndLoader::parseNextEvent(uint16 chan, uint16 &duration, uint8 &note, bool &skip, bool &updateInstr) {
	if (chan >= ARRAYSIZE(_chanSndData) || !_chanSndData[chan] || _chanCurEvent[chan] >= _chanNumEvents[chan])
		return false;

	const byte *s = _chanSndData[chan] + (_chanCurEvent[chan]++) * 3;
	duration = READ_BE_UINT16(s);
	note = s[2] & 0x7f;
	skip = false;
	updateInstr = false;

	if (_synth == 4 && chan != 0 && note == 0)
		note = 1;

	return true;
}

const Common::SharedPtr<MacSndLoader::Instrument> *LoomMacSndLoader::fetchInstrument(uint16 id) const {
	Common::Array<Common::SharedPtr<Instrument> >::const_iterator instr = _instruments.end();
	for (Common::Array<Common::SharedPtr<Instrument> >::const_iterator i = _instruments.begin(); i != _instruments.end(); ++i) {
		if ((*i)->id() == id)
			return i;
		else if ((*i)->id() == 0x2D1C)
			instr = i;
	}
	return (instr != _instruments.end()) ? instr : nullptr;
}

bool MonkeyMacSndLoader::init() {
	static const char *execNames[] = {
		"Monkey Island"
	};

	return loadInstruments(execNames, ARRAYSIZE(execNames), _numInstrumentsMax - 1);
}

bool MonkeyMacSndLoader::checkResource(const byte *data, uint32 dataSize) const {
	return (dataSize >= 14 && (READ_BE_UINT32(data) == MKTAG('M', 'a', 'c', '0') || READ_BE_UINT32(data) == MKTAG('M', 'a', 'c', '1')));
}

bool MonkeyMacSndLoader::blocked(const byte *data, uint32 dataSize) const {
	return (dataSize < 14 || (_blockSfx && !data[13]));
}

bool MonkeyMacSndLoader::loadSound(const byte *data, uint32 dataSize) {
	if (dataSize < 32)
		return false;

	_sndRes9 = data[9];
	_isMusic = data[13];
	_sndRes10 = data[10];
	_chanSetup = data[11];
	_timbre = data[12];

	for (int i = 0; i < 4; ++i) {
		uint32 offs = READ_BE_UINT32(data + 16 + 4 * i);
		_chanCurEvent[i] = 0;
		if (offs) {
			if (dataSize < offs + 12)
				return false;
			_chanInstr[i] = fetchInstrument(data, dataSize, READ_BE_UINT32(data + offs + 8));
			_chanInstr2[i] = nullptr;
			_chanSndData[i] = &data[offs + 12];
			_chanNumEvents[i] = 0;

			for (const byte *s = _chanSndData[i]; s < &data[dataSize - 4]; s += 4) {
				uint32 in = READ_BE_UINT32(s);
				if (in == MKTAG('L', 'o', 'o', 'p') || in == MKTAG('D', 'o', 'n', 'e')) {
					if (i == 1)
						_loop = (in == MKTAG('L', 'o', 'o', 'p'));
					break;
				}
				_chanNumEvents[i]++;
			}
		} else {
			_chanInstr[i] = nullptr;
			_chanSndData[i] = nullptr;
		}
	}

	_blockSfx = (_isMusic && _loop);

	return true;
}

bool MonkeyMacSndLoader::isInstrUsable(uint16 chan) const {
	return (chan < ARRAYSIZE(_chanInstr) && _chanInstr[chan] && _chanInstr[chan]->get()->id() != MKTAG('s', 'i', 'l', 'e'));
}

bool MonkeyMacSndLoader::parseNextEvent(uint16 chan, uint16 &duration, uint8 &note, bool &skip, bool &updateInstr) {
	if (chan >= ARRAYSIZE(_chanSndData) || !_chanSndData[chan] || _chanCurEvent[chan] >= _chanNumEvents[chan])
		return false;

	const byte *s = _chanSndData[chan] + (_chanCurEvent[chan]++) * 4;
	duration = READ_BE_UINT16(s);
	note = s[2];
	skip = false;
	updateInstr = true;

	if (duration == 0 && _chanCurEvent[chan] == _chanNumEvents[chan])
		skip = true;

	if (_synth == 4) {
		if (!skip && note == 0) {
			note = 60;
			_chanInstr2[chan] = _chanInstr[chan];
			_chanInstr[chan] = fetchInstrument(nullptr, 0, MKTAG('s', 'i', 'l', 'e'));
		} else if (_chanInstr2[chan]) {
			_chanInstr[chan] = _chanInstr2[chan];
			_chanInstr2[chan] = nullptr;
		} else {
			updateInstr = false;
		}
		if (note == 1)
			skip = true;
		else if (s[6] == 1)
			duration += READ_BE_UINT16(s + 4);
	} else {
		updateInstr = false;
		if (note == 1)
			note = 0;
		else if (note != 0)
			note += _transpose;
	}

	return true;
}

const Common::SharedPtr<MacSndLoader::Instrument> *MonkeyMacSndLoader::fetchInstrument(const byte *data, uint32 dataSize, uint32 tagOrOffset) {
	Common::Array<Common::SharedPtr<Instrument> >::const_iterator instr = _instruments.end();
	if (tagOrOffset & ~0x7fffff) {
		for (Common::Array<Common::SharedPtr<Instrument> >::const_iterator i = _instruments.begin(); i != _instruments.end(); ++i) {
			if ((*i)->id() == tagOrOffset)
				return i;
			else if ((*i)->id() == MKTAG('s', 'i', 'l', 'e'))
				instr = i;
		}
	} else if (dataSize >= tagOrOffset + 8) {
		Common::SeekableReadStream *str = new Common::MemoryReadStream(&data[tagOrOffset + 8], READ_BE_UINT32(data + tagOrOffset + 4), DisposeAfterUse::NO);
		if (_instruments.size() == _numInstrumentsMax)
			_instruments.pop_back();
		_instruments.push_back(Common::SharedPtr<Instrument>(new Instrument(READ_BE_UINT32(&data[tagOrOffset]), str, Common::String())));
		delete str;
		instr = _instruments.end() - 1;
	}
	return (instr != _instruments.end()) ? instr : nullptr;
}

Common::WeakPtr<LoomMonkeyMacSnd> *LoomMonkeyMacSnd::_inst = nullptr;

LoomMonkeyMacSnd::LoomMonkeyMacSnd(ScummEngine *vm, Audio::Mixer *mixer) : VblTaskClientDriver(), _vm(vm), _mixer(mixer), _curSound(0), _loader(nullptr),
	_macstr(nullptr), _sdrv(nullptr), _vblTskProc(this, &VblTaskClientDriver::vblCallback), _songTimer(0), _songTimerInternal(0),
	_machineRating(0), _selectedQuality(2), _effectiveChanConfig(0), _16bit(false), _idRangeMax(200), _sndChannel(0), _chanUse(0), _defaultChanConfig(0),
	_chanConfigTable(nullptr), _chanPlaying(0), _curChanConfig(0), _curSynthType(0), _curSndType(Audio::Mixer::kPlainSoundType), _mixerThread(false),
	_restartSound(0), _lastSndType(Audio::Mixer::kPlainSoundType), _chanCbProc(this, &MacLowLevelPCMDriver::CallbackClient::sndChannelCallback),
	_curSoundSaveVar(0), _saveVersionChange(vm->_game.id == GID_MONKEY ? 115 : 114), _legacySaveUnits(vm->_game.id == GID_MONKEY ? 3 : 5) {
	assert(_vm);
	assert(_mixer);

	static const byte cfgtable[] = { 0, 0, 0, 1, 4, 5, 1, 5, 6, 6, 8, 9 };
	_chanConfigTable = new byte[sizeof(cfgtable)]();
	memcpy(_chanConfigTable, cfgtable, sizeof(cfgtable));

	if (vm->_game.id == GID_MONKEY)
		_chanConfigTable[10] = 7;

	memset(_musChannels, 0, sizeof(_musChannels));
}

LoomMonkeyMacSnd::~LoomMonkeyMacSnd() {
	_mixer->stopHandle(_soundHandle);
	delete _macstr;
	delete[] _chanConfigTable;

	disposeAllChannels();
	delete _sdrv;
	delete _loader;

	delete _inst;
	_inst = nullptr;
}

Common::SharedPtr<LoomMonkeyMacSnd> LoomMonkeyMacSnd::open(ScummEngine *vm, Audio::Mixer *mixer) {
	Common::SharedPtr<LoomMonkeyMacSnd> scp = nullptr;

	if (_inst == nullptr) {
		scp = Common::SharedPtr<LoomMonkeyMacSnd>(new LoomMonkeyMacSnd(vm, mixer));
		_inst = new Common::WeakPtr<LoomMonkeyMacSnd>(scp);
		// We can start this with the ScummVM mixer output rate instead of the ASC rate. The Mac sample rate converter can handle it (at
		// least for up to 48 KHz, I haven't tried higher settings) and we don't have to do another rate conversion in the ScummVM mixer.
		if ((_inst == nullptr) || (mixer == nullptr) || !(scp->startDevice(mixer->getOutputRate(), mixer->getOutputRate() << 16/*ASC_DEVICE_RATE*/, PCM_BUFFER_SIZE, true, false, true)))
			error("LoomMonkeyMacSnd::open(): Failed to start player");
	}

	return _inst->lock();
}

bool LoomMonkeyMacSnd::startDevice(uint32 outputRate, uint32 pcmDeviceRate, uint32 feedBufferSize, bool enableInterpolation, bool stereo, bool internal16Bit) {
	_macstr = new MacPlayerAudioStream(this, outputRate, stereo, enableInterpolation, internal16Bit);
	if (!_macstr || !_mixer)
		return false;

	if (_vm->_game.id == GID_LOOM)
		_loader = new LoomMacSndLoader();
	else if(_vm->_game.id == GID_MONKEY)
		_loader = new MonkeyMacSndLoader();

	if (!_loader || !_loader->init())
		return false;

	_sdrv = new MacLowLevelPCMDriver(_mixer->mutex(), pcmDeviceRate, internal16Bit);
	if (!_sdrv)
		return false;

	_effectiveChanConfig = 9;
	_16bit = internal16Bit;

	_macstr->initDrivers();
	_macstr->initBuffers(feedBufferSize);
	_macstr->setVblCallback(&_vblTskProc);

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, _macstr, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);

	return true;
}

void LoomMonkeyMacSnd::setMusicVolume(int vol) {
	Common::StackLock lock(_mixer->mutex());
	if (_macstr)
		_macstr->setMasterVolume(Audio::Mixer::kMusicSoundType, vol);
}

void LoomMonkeyMacSnd::setSfxVolume(int vol) {
	Common::StackLock lock(_mixer->mutex());
	if (_macstr)
		_macstr->setMasterVolume(Audio::Mixer::kSFXSoundType, vol);
}

void LoomMonkeyMacSnd::startSound(int id, int jumpToTick) {
	if (_sdrv == nullptr || id < 1 || id >= _idRangeMax) {
		warning("LoomMonkeyMacSnd::startSound(): sound id '%d' out of range (1 - %d)", id, _idRangeMax - 1);
		return;
	}

	Common::StackLock lock(_mixer->mutex());

	uint32 size = _vm->getResourceSize(rtSound, id);
	const byte *ptr = _vm->getResourceAddress(rtSound, id);
	assert(ptr);

	if (!_loader->checkResource(ptr, size)) {
		warning("LoomMonkeyMacSnd::startSound(): Sound resource '%d' cannot be played", id);
		return;
	}

	if (_loader->blocked(ptr, size))
		return;

	stopActiveSound();
	if (_chanUse <= 1)
		disposeAllChannels();

	if (!_defaultChanConfig)
		detectQuality();

	if (!_loader->loadSound(ptr, size)) {
		warning("LoomMonkeyMacSnd::startSound(): Sound resource '%d' cannot be played", id);
		return;
	}

	//if (_sndDisableFlags && _loader->isMusic())
	//	return;

	_effectiveChanConfig = _loader->getChanSetup() ? _loader->getChanSetup() : _defaultChanConfig;
	_curSndType = _loader->isMusic() ? Audio::Mixer::kMusicSoundType : Audio::Mixer::kSFXSoundType;

	if (_lastSndType != _curSndType)
		_curChanConfig = 0;

	_curSound = id;
	_curSoundSaveVar = _loader->restartSoundAfterLoad() ? _curSound : 0;

	setupChannels();
	sendSoundCommands(jumpToTick);

	if (!jumpToTick) {
		_songTimer = 0;
		_songTimerInternal = 0;
	}
}

void LoomMonkeyMacSnd::stopSound(int id) {
	if (id < 1 || id >= _idRangeMax) {
		warning("LoomMonkeyMacSnd::stopSound(): sound id '%d' out of range (1 - %d)", id, _idRangeMax - 1);
		return;
	}

	Common::StackLock lock(_mixer->mutex());

	if (id == _curSound)
		stopActiveSound();
}

void LoomMonkeyMacSnd::stopAllSounds() {
	Common::StackLock lock(_mixer->mutex());
	stopActiveSound();
}

int LoomMonkeyMacSnd::getMusicTimer() {
	Common::StackLock lock(_mixer->mutex());
	return _songTimer;
}

int LoomMonkeyMacSnd::getSoundStatus(int id) const {
	if (id < 1 || id >= _idRangeMax) {
		warning("LoomMonkeyMacSnd::getSoundStatus(): sound id '%d' out of range (1 - %d)", id, _idRangeMax - 1);
		return 0;
	}
	Common::StackLock lock(_mixer->mutex());
	return (_curSound == id) ? 1 : 0;
}

void LoomMonkeyMacSnd::setQuality(int qual) {
	assert(qual >= MacSound::kQualityAuto && qual <= MacSound::kQualityHighest);
	Common::StackLock lock(_mixer->mutex());

	if (qual > MacSound::kQualityAuto) {
		qual--;
		if (!_loader->ignoreMachineRating())
			_machineRating = (qual / 3) + 1;
		_selectedQuality = qual % 3;
		qual = _chanConfigTable[_machineRating * 3 + _selectedQuality];
		if (qual && qual == _defaultChanConfig)
			return;
	}

	int csnd = _curSound;
	int32 timeStamp = csnd ? _songTimer * 1000 + ((_songTimerInternal * 1000) / 30) : 0;
	stopActiveSound();
	
	detectQuality();
	if (csnd)
		startSound(csnd, timeStamp);
}

void LoomMonkeyMacSnd::saveLoadWithSerializer(Common::Serializer &ser) {
	if (ser.isLoading() && ser.getVersion() < VER(94)) {
		_curSound = _curSoundSaveVar = 0;
		return;
	}

	if (ser.isLoading()) {
		if (ser.getVersion() < VER(_saveVersionChange)) {
			// Skip over old driver savedata, since it is not needed here.
			ser.skip(4);
			// We need only this
			ser.syncAsSint16LE(_curSound);
			_curSoundSaveVar = CLIP<int>(_curSound, 0, _idRangeMax - 1);
			// Skip the rest
			ser.skip(_legacySaveUnits * (ser.getVersion() >= VER(94) && ser.getVersion() <= VER(103) ? 24 : 20));
		} else if (ser.getVersion() <= VER(114)) {
			_curSoundSaveVar = 0;
			byte tmp = 0;
			for (int i = 0; !_curSoundSaveVar && i < 200; ++i) {
				ser.syncAsByte(tmp);
				if (tmp)
					_curSoundSaveVar = i;
			}
		}
	}
	ser.syncAsSint16LE(_curSoundSaveVar, VER(115));
}

void LoomMonkeyMacSnd::restoreAfterLoad() {
	int sound = _curSoundSaveVar;
	stopActiveSound();
	if (sound)
		startSound(sound);
}

void LoomMonkeyMacSnd::enable() {
	restoreAfterLoad();
}

void LoomMonkeyMacSnd::disable() {
	int sound = _curSoundSaveVar;
	stopActiveSound();
	_curSoundSaveVar = sound;
}

void LoomMonkeyMacSnd::vblCallback() {
	if (_songTimerInternal++ == 29) {
		_songTimerInternal = 0;
		++_songTimer;
	}

	if (_restartSound) {
		startSound(_restartSound);
		_restartSound = 0;
	}
}

void LoomMonkeyMacSnd::generateData(int8 *dst, uint32 len, Audio::Mixer::SoundType type, bool expectStereo) const {
	assert(dst);
	memset(dst, 0, len);
	_sdrv->feed(dst, len, type, expectStereo);
}

const MacSoundDriver::Status &LoomMonkeyMacSnd::getDriverStatus(uint8, Audio::Mixer::SoundType sndType) const {
	return _sdrv->getStatus(sndType);
}

void LoomMonkeyMacSnd::sndChannelCallback(uint16 arg1, const void*) {
	// The original Loom player stops the track immediately when the first channel invoked its end-of-track
	// callback. This cuts of the playback early, in an unpleasant way. The Monkey Island player stops the
	// playback not before all channels have finished. We do the same here for both games.
	_chanPlaying &= ~arg1;
	if (_chanPlaying)
		return;

	if (_loader->isLooping())
		_restartSound = _curSound;

	stopActiveSound();
}

void LoomMonkeyMacSnd::sendSoundCommands(int timeStamp) {
	if (!_defaultChanConfig || !_curSound)
		return;

	uint16 duration = 0;
	byte note = 0;
	bool skip = false;
	bool updateInstr = false;
	_chanPlaying = 0;

	if (_chanUse == 1 && _sndChannel) {
		while (_loader->parseNextEvent(0, duration, note, skip, updateInstr)) {
			if (timeStamp > 0) {
				int ts = timeStamp;
				timeStamp = MAX<int>(0, timeStamp - duration);
				duration -= ts;
			}
			if (timeStamp)
				continue;

			if (updateInstr)
				_sdrv->loadInstrument(_sndChannel, MacLowLevelPCMDriver::kEnqueue, _loader->getInstrData(0));

			if (!skip) {
				_sdrv->playNote(_sndChannel, MacLowLevelPCMDriver::kEnqueue, note, duration);
				if (note == 0) // Workaround for tempo glitch in original driver
					_sdrv->wait(_sndChannel, MacLowLevelPCMDriver::kEnqueue, duration);
			}
		}
		_sdrv->quiet(_sndChannel, MacLowLevelPCMDriver::kEnqueue);
		_sdrv->callback(_sndChannel, MacLowLevelPCMDriver::kEnqueue, 1, nullptr);
		_chanPlaying |= 1;

	} else if (_chanUse == 4) {
		int tmstmp[4];
		uint8 busy = 0;

		for (int i = 0; i < 4; ++i) {
			tmstmp[i] = timeStamp;
			if (_musChannels[i])
				busy |= (1 << i);
		}

		while (busy) {
			for (int i = 0; i < 4; ++i) {
				if (!(busy & (1 << i)) || !_loader->parseNextEvent(i + 1, duration, note, skip, updateInstr)) {
					busy &= ~(1 << i);
					continue;
				}
				if (tmstmp[i] > 0) {
					int ts = tmstmp[i];
					tmstmp[i] = MAX<int>(0, tmstmp[i] - duration);
					duration -= ts;
				}

				if (tmstmp[i])
					continue;

				if (updateInstr)
					_sdrv->loadInstrument(_musChannels[i], MacLowLevelPCMDriver::kEnqueue, _loader->getInstrData(i + 1));

				if (!skip) {
					_sdrv->playNote(_musChannels[i], MacLowLevelPCMDriver::kEnqueue, note, duration);
					// Workaround for tempo glitch in original driver. For the sampled synth in 4 channel mode, there is
					// some sort of fix in the original (see parseNextEvent()), but that really does not work well for the other cases.
					if (note == 0 && _curSynthType != 4)
						_sdrv->wait(_musChannels[i], MacLowLevelPCMDriver::kEnqueue, duration);
				}
				_chanPlaying |= (1 << i);
			}
		}

		
		for (int i = 0; i < 4; ++i) {
			if (_chanPlaying & (1 << i)) {
				_sdrv->quiet(_musChannels[i], MacLowLevelPCMDriver::kEnqueue);
				_sdrv->callback(_musChannels[i], MacLowLevelPCMDriver::kEnqueue, 1 << i, nullptr);
			}
		}
	}
}

void LoomMonkeyMacSnd::stopActiveSound() {
	if (_sndChannel) {
		_sdrv->quiet(_sndChannel, MacLowLevelPCMDriver::kImmediate);
		_sdrv->flush(_sndChannel, MacLowLevelPCMDriver::kImmediate);
	}

	for (int i = 0; i < ARRAYSIZE(_musChannels); ++i) {
		if (_musChannels[i]) {
			_sdrv->quiet(_musChannels[i], MacLowLevelPCMDriver::kImmediate);
			_sdrv->flush(_musChannels[i], MacLowLevelPCMDriver::kImmediate);
		}
	}

	_chanPlaying = 0;
	_curSound = 0;
	_curSoundSaveVar = 0;
	_loader->unblock();
}

void LoomMonkeyMacSnd::setupChannels() {
	static const byte synthType[] =	{ 0x00,	0x01, 0x02, 0x04, 0x04, 0x04, 0x02, 0x04, 0x04, 0x04 };
	static const byte numChan[] =	{ 0x00,	0x01, 0x01, 0x01, 0x01, 0x01, 0x04, 0x04, 0x04, 0x04 };
	static const byte attrib[] =	{ 0x00, 0x00, 0x04, 0xAC, 0xA4, 0xA0, 0x04, 0xAC, 0xA4, 0xA0 };

	if (!_defaultChanConfig)
		return;

	if (_curChanConfig != _effectiveChanConfig) {
		disposeAllChannels();
		_curChanConfig = _effectiveChanConfig;
		_curSynthType = synthType[_curChanConfig];
		_chanUse = numChan[_curChanConfig];
		_lastSndType = _curSndType;
		_loader->setSynthType(_curSynthType);

		switch (_curSynthType) {
		case 1:
			if (_chanUse == 1 && !_sndChannel) {
				_sndChannel = _sdrv->createChannel(_curSndType, MacLowLevelPCMDriver::kSquareWaveSynth, attrib[_curChanConfig], &_chanCbProc);
			}
			break;
		case 2:
			if (_chanUse == 1 && !_sndChannel) {
				_sndChannel = _sdrv->createChannel(_curSndType, MacLowLevelPCMDriver::kWaveTableSynth, attrib[_curChanConfig], &_chanCbProc);
			} else if (_chanUse == 4) {
				for (int i = 0; i < ARRAYSIZE(_musChannels); ++i) {
					if (!_musChannels[i] && _loader->isInstrUsable(i + 1))
						_musChannels[i] = _sdrv->createChannel(_curSndType, MacLowLevelPCMDriver::kWaveTableSynth, attrib[_curChanConfig] + i, &_chanCbProc);
				}
			}
			break;
		case 4:
			if (_chanUse == 1 && !_sndChannel) {
				_sndChannel = _sdrv->createChannel(_curSndType, MacLowLevelPCMDriver::kSampledSynth, attrib[_curChanConfig], &_chanCbProc);
			} else if (_chanUse == 4) {
				for (int i = 0; i < ARRAYSIZE(_musChannels); ++i) {
					if (!_musChannels[i] && _loader->isInstrUsable(i + 1))
						_musChannels[i] = _sdrv->createChannel(_curSndType, MacLowLevelPCMDriver::kSampledSynth, attrib[_curChanConfig], &_chanCbProc);
				}
			}
			break;
		default:
			break;
		}
	}

	switch (_curSynthType) {
	case 1:
		if (_sndChannel)
			_sdrv->setTimbre(_sndChannel, MacLowLevelPCMDriver::kImmediate, _loader->getTimbre());
		break;
	case 2:
		if (_chanUse == 1) {
			if (_sndChannel)
				_sdrv->loadWaveTable(_sndChannel, MacLowLevelPCMDriver::kImmediate, _fourToneSynthWaveForm, _fourToneSynthWaveFormSize);
		} else {
			for (int i = 0; i < ARRAYSIZE(_musChannels); ++i) {
				if (_musChannels[i])
					_sdrv->loadWaveTable(_musChannels[i], MacLowLevelPCMDriver::kImmediate, _fourToneSynthWaveForm, _fourToneSynthWaveFormSize);
			}
		}
		break;
	case 4:
		if (_chanUse == 1) {
			if (_sndChannel && _loader->getInstrData(0))
				_sdrv->loadInstrument(_sndChannel, MacLowLevelPCMDriver::kImmediate, _loader->getInstrData(0));
		} else {
			for (int i = 0; i < ARRAYSIZE(_musChannels); ++i) {
				if (_musChannels[i] && _loader->getInstrData(i + 1))
					_sdrv->loadInstrument(_musChannels[i], MacLowLevelPCMDriver::kImmediate, _loader->getInstrData(i + 1));
			}
		}
		break;
	default:
		break;
	}
}

void LoomMonkeyMacSnd::disposeAllChannels() {
	if (_sndChannel)
		_sdrv->disposeChannel(_sndChannel);
	_sndChannel = 0;

	for (int i = 0; i < ARRAYSIZE(_musChannels); ++i) {
		if (_musChannels[i])
			_sdrv->disposeChannel(_musChannels[i]);
		_musChannels[i] = 0;
	}

	_curChanConfig = 0;
}

void LoomMonkeyMacSnd::detectQuality() {
	if (_machineRating == 0) {
		if (!_loader->ignoreMachineRating()) {
			if (isSoundCardType10())
				_machineRating = 2;
			/*else if (0)
				_machineRating = 1;*/
		}
		_machineRating ^= 3;
	}

	_defaultChanConfig = _effectiveChanConfig = _chanConfigTable[_machineRating * 3 + _selectedQuality];
	_curChanConfig = 0;
	disposeAllChannels();
	setupChannels();
	_chanConfigTable[_machineRating * 3 + _selectedQuality] = _defaultChanConfig;
}

bool LoomMonkeyMacSnd::isSoundCardType10() const {
	return _mixerThread ? (_machineRating == 1) : (_vm->VAR_SOUNDCARD != 0xff && _vm->VAR(_vm->VAR_SOUNDCARD) == 10);
}

#undef ASC_DEVICE_RATE
#undef PCM_BUFFER_SIZE

} // End of namespace Scumm
