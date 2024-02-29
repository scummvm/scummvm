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

namespace Scumm {

#define ASC_DEVICE_RATE		0x56EE8BA3
#define PCM_BUFFER_SIZE		1024

Common::WeakPtr<LoomMacSnd> *LoomMacSnd::_inst = nullptr;

LoomMacSnd::LoomMacSnd(ScummEngine *vm, Audio::Mixer *mixer) : VblTaskClientDriver(),
	_vm(vm), _mixer(mixer), _curSound(0), _macstr(nullptr), _soundUsage(0), _sdrv(nullptr), _songTimerUpdt(this, &VblTaskClientDriver::vblCallback), _soundConfig(this),
	_songTimer(0), _songTimerInternal(0), _machineRating(0), _selectedQuality(2), _effectiveChanConfig(0), _16bit(false), _idRangeMax(200), _sndChannel(0), _chanUse(0),
	_defaultChanConfig(0), _chanConfigTable(nullptr), _chanPlaying(0), _curChanConfig(0), _curSynthType(0), _curSndType(Audio::Mixer::kPlainSoundType),
	_mixerThread(false), _lastSndType(Audio::Mixer::kPlainSoundType), _chanCbProc(this, &MacLowLevelPCMDriver::CallbackClient::sndChannelCallback) {
	assert(_vm);
	assert(_mixer);

	static const byte cfgtable[] = { 0, 0, 0, 1, 4, 5, 1, 5, 6, 6, 8, 9 };
	_chanConfigTable = new byte[sizeof(cfgtable)]();
	memcpy(_chanConfigTable, cfgtable, sizeof(cfgtable));
	_soundUsage = new byte[_idRangeMax]();
	memset(_musChannels, 0, sizeof(_musChannels));
}

LoomMacSnd::~LoomMacSnd() {
	_mixer->stopHandle(_soundHandle);
	delete _macstr;
	delete[] _soundUsage;
	delete[] _chanConfigTable;

	disposeAllChannels();
	delete _sdrv;

	delete _inst;
	_inst = nullptr;
}

Common::SharedPtr<LoomMacSnd> LoomMacSnd::open(ScummEngine *vm, Audio::Mixer *mixer) {
	Common::SharedPtr<LoomMacSnd> scp = nullptr;

	if (_inst == nullptr) {
		scp = Common::SharedPtr<LoomMacSnd>(new LoomMacSnd(vm, mixer));
		_inst = new Common::WeakPtr<LoomMacSnd>(scp);
		// We can start this with the ScummVM mixer output rate instead of the ASC rate. The Mac sample rate converter can handle it (at
		// least for up to 48 KHz, I haven't tried higher settings) and we don't have to do another rate conversion in the ScummVM mixer.
		if ((_inst == nullptr) || (mixer == nullptr) || !(scp->startDevice(mixer->getOutputRate(), mixer->getOutputRate() << 16/*ASC_DEVICE_RATE*/, PCM_BUFFER_SIZE, true, false, true)))
			error("LoomMacSnd::open(): Failed to start player");
	}

	return _inst->lock();
}

bool LoomMacSnd::startDevice(uint32 outputRate, uint32 pcmDeviceRate, uint32 feedBufferSize, bool enableInterpolation, bool stereo, bool internal16Bit) {
	_macstr = new MacPlayerAudioStream(this, outputRate, stereo, enableInterpolation, internal16Bit);
	if (!_macstr || !_mixer)
		return false;

	if (!loadInstruments())
		return false;

	_sdrv = new MacLowLevelPCMDriver(_mixer->mutex(), pcmDeviceRate, internal16Bit);
	if (!_sdrv)
		return false;

	_effectiveChanConfig = 9;
	_16bit = internal16Bit;

	_macstr->initDrivers();
	_macstr->initBuffers(feedBufferSize);
	_macstr->setVblCallback(&_songTimerUpdt);

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, _macstr, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);

	return true;
}

void LoomMacSnd::setMusicVolume(int vol) {
	Common::StackLock lock(_mixer->mutex());
	if (_macstr)
		_macstr->setMasterVolume(Audio::Mixer::kMusicSoundType, vol);
}

void LoomMacSnd::setSfxVolume(int vol) {
	Common::StackLock lock(_mixer->mutex());
	if (_macstr)
		_macstr->setMasterVolume(Audio::Mixer::kSFXSoundType, vol);
}

void LoomMacSnd::startSound(int id, int jumpToTick) {
	if (_sdrv == nullptr || id < 0 || id >= _idRangeMax) {
		warning("LoomMacSnd::startSound(): sound id '%d' out of range (0 - %d)", id, _idRangeMax - 1);
		return;
	}

	Common::StackLock lock(_mixer->mutex());

	const byte *ptr = _vm->getResourceAddress(rtSound, id);
	assert(ptr);

	if (READ_BE_UINT16(ptr + 4) != 'so' || READ_BE_UINT32(ptr + 10)) {
		warning("LoomMacSnd::startSound(): Sound resource '%d' cannot be played", id);
		return;
	}

	stopActiveSound();
	if (_chanUse <= 1)
		disposeAllChannels();

	if (!_defaultChanConfig)
		detectQuality();

	_soundConfig.load(ptr);
	//if (_sndDisableFlags && _soundConfig.switchable)
	//	return;

	if (_soundConfig.chanSetup) {
		_effectiveChanConfig = _soundConfig.chanSetup;
		_curSndType = Audio::Mixer::kSFXSoundType;
	} else {
		_effectiveChanConfig = _defaultChanConfig;
		_curSndType = Audio::Mixer::kMusicSoundType;
	}

	if (_lastSndType != _curSndType)
		_curChanConfig = 0;

	_curSound = id;
	_soundUsage[id] = 1;

	setupChannels();
	sendSoundCommands(ptr, jumpToTick);

	if (!jumpToTick) {
		_songTimer = 0;
		_songTimerInternal = 0;
	}
}

void LoomMacSnd::stopSound(int id) {
	if (id < 0 || id >= _idRangeMax) {
		warning("LoomMacSnd::stopSound(): sound id '%d' out of range (0 - %d)", id, _idRangeMax - 1);
		return;
	}

	Common::StackLock lock(_mixer->mutex());
	_soundUsage[id] = 0;

	if (id == _curSound)
		stopActiveSound();
}

void LoomMacSnd::stopAllSounds() {
	Common::StackLock lock(_mixer->mutex());
	memset(_soundUsage, 0, _idRangeMax);
	stopActiveSound();
}

int LoomMacSnd::getMusicTimer() {
	Common::StackLock lock(_mixer->mutex());
	return _songTimer;
}

int LoomMacSnd::getSoundStatus(int id) const {
	if (id < 0 || id >= _idRangeMax) {
		warning("LoomMacSnd::getSoundStatus(): sound id '%d' out of range (0 - %d)", id, _idRangeMax - 1);
		return 0;
	}
	Common::StackLock lock(_mixer->mutex());
	return _soundUsage[id];
}

void LoomMacSnd::setQuality(int qual) {
	assert(qual >= MacSound::kQualityAuto && qual <= MacSound::kQualityHighest);
	Common::StackLock lock(_mixer->mutex());

	if (qual > MacSound::kQualityAuto) {
		qual--;
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

void LoomMacSnd::saveLoadWithSerializer(Common::Serializer &ser) {
	if (ser.isLoading() && ser.getVersion() < VER(94)) {
		memset(_soundUsage, 0, _idRangeMax);
		_curSound = 0;
		return;
	}

	if (ser.isLoading() && ser.getVersion() < VER(114)) {
		memset(_soundUsage, 0, _idRangeMax);
		// Skip over old driver savedata, since it is not needed here.
		ser.skip(4);
		// We need only this
		ser.syncAsSint16LE(_curSound);
		_curSound = CLIP<int>(_curSound, 0, _idRangeMax - 1);
		if (_curSound > 0)
			_soundUsage[_curSound] = 1;
		// Skip the rest
		ser.skip(ser.getVersion() >= VER(94) && ser.getVersion() <= VER(103) ? 120 : 100);
	} else {
		ser.syncBytes(_soundUsage, _idRangeMax, VER(114));
	}
}

void LoomMacSnd::restoreAfterLoad() {
	for (int i = 1; i < _idRangeMax; ++i) {
		if (_soundUsage[i]) {
			_soundUsage[i] = 0;
			startSound(i);
		}
	}
}

void LoomMacSnd::vblCallback() {
	if (_songTimerInternal++ == 29) {
		_songTimerInternal = 0;
		++_songTimer;
	}
}

void LoomMacSnd::generateData(int8 *dst, uint32 len, Audio::Mixer::SoundType type, bool expectStereo) const {
	assert(dst);
	memset(dst, 0, len);
	_sdrv->feed(dst, len, type, expectStereo);
}

const MacSoundDriver::Status &LoomMacSnd::getDriverStatus(uint8, Audio::Mixer::SoundType sndType) const {
	return _sdrv->getStatus(sndType);
}

void LoomMacSnd::sndChannelCallback(uint16 arg1, const void*) {
	// We do this a little smarter than the original player which would stop the track immediately when
	// the first channel invoked its end-of-track callback. This would cut of the playback early, in an
	// unpleasant way. Instead, we stop the playback not before all channels have finished.
	_chanPlaying &= ~arg1;
	if (_chanPlaying)
		return;
	stopActiveSound();
}

void LoomMacSnd::sendSoundCommands(const byte *data, int timeStamp) {
	if (!_defaultChanConfig || !_curSound)
		return;

	if (_chanUse == 1 && _sndChannel) {
		const byte *s = data + READ_BE_UINT16(data + 30) + 6;
		uint16 len = READ_BE_UINT16(s - 2);
		while (len--) {
			uint16 p1 = READ_BE_UINT16(s);
			s += 2;
			uint8 note = *s++;

			if (timeStamp > 0) {
				int ts = timeStamp;
				timeStamp = MAX<int>(0, timeStamp - p1);
				p1 -= ts;
			}
			if (timeStamp)
				continue;

			_sdrv->playNote(_sndChannel, MacLowLevelPCMDriver::kEnqueue, note & 0x7f, p1);
			if (note == 0) // Workaround for tempo glitch in original driver
				_sdrv->wait(_sndChannel, MacLowLevelPCMDriver::kEnqueue, p1);
		}
		_sdrv->quiet(_sndChannel, MacLowLevelPCMDriver::kEnqueue);
		_sdrv->callback(_sndChannel, MacLowLevelPCMDriver::kEnqueue, 1, nullptr);
		_chanPlaying |= 1;

	} else if (_chanUse == 4) {
		const byte *src[4];
		uint16 len[4];
		int tmstmp[4];

		for (int i = 0; i < 4; ++i) {
			src[i] = nullptr;
			len[i] = 0;
			tmstmp[i] = timeStamp;
			if (!_musChannels[i])
				continue;
			src[i] = data + READ_BE_UINT16(data + 32 + 2 * i) + 6;
			len[i] = READ_BE_UINT16(src[i] - 2);
		}

		for (bool loop = true; loop; ) {
			loop = false;
			for (int i = 0; i < 4; ++i) {
				if (!src[i] || !len[i])
					continue;
				uint16 p1 = READ_BE_UINT16(src[i]);
				src[i] += 2;
				byte note = *src[i]++;
				if (tmstmp[i] > 0) {
					int ts = tmstmp[i];
					tmstmp[i] = MAX<int>(0, tmstmp[i] - p1);
					p1 -= ts;
				}
				loop |= static_cast<bool>(--len[i]);

				if (tmstmp[i])
					continue;

				_sdrv->playNote(_musChannels[i], MacLowLevelPCMDriver::kEnqueue, (_curSynthType == 4 && note == 0) ? 1 : note & 0x7f, p1);
				// Workaround for tempo glitch in original driver. For the sampled synth in 4 channel mode, there is
				// some sort of fix in the original (see above), but that really does not work well for the other cases.
				if (note == 0 && _curSynthType != 4)
					_sdrv->wait(_musChannels[i], MacLowLevelPCMDriver::kEnqueue, p1);
			}
		}

		_chanPlaying = 0;
		for (int i = 0; i < 4; ++i) {
			if (src[i]) {
				_sdrv->quiet(_musChannels[i], MacLowLevelPCMDriver::kEnqueue);
				_sdrv->callback(_musChannels[i], MacLowLevelPCMDriver::kEnqueue, 1 << i, nullptr);
				_chanPlaying |= (1 << i);
			}
		}
	}
}

void LoomMacSnd::stopActiveSound() {
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

	if (_curSound) {
		_soundUsage[_curSound] = 0;
		_curSound = 0;
	}
	_chanPlaying = 0;
}

void LoomMacSnd::setupChannels() {
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
					if ((_soundConfig.instruments[i + 1] && _soundConfig.instruments[i + 1]->get()->id() == 0x2D1C) || _musChannels[i])
						continue;
					_musChannels[i] = _sdrv->createChannel(_curSndType, MacLowLevelPCMDriver::kWaveTableSynth, attrib[_curChanConfig] + i, &_chanCbProc);
				}
			}
			break;
		case 4:
			if (_chanUse == 1 && !_sndChannel) {
				_sndChannel = _sdrv->createChannel(_curSndType, MacLowLevelPCMDriver::kSampledSynth, attrib[_curChanConfig], &_chanCbProc);
			} else if (_chanUse == 4) {
				for (int i = 0; i < ARRAYSIZE(_musChannels); ++i) {
					if ((_soundConfig.instruments[i + 1] && _soundConfig.instruments[i + 1]->get()->id() == 0x2D1C) || _musChannels[i])
						continue;
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
			_sdrv->setTimbre(_sndChannel, MacLowLevelPCMDriver::kImmediate, _soundConfig.timbre);
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
			if (_sndChannel && _soundConfig.instruments[0])
				_sdrv->loadInstrument(_sndChannel, MacLowLevelPCMDriver::kImmediate, _soundConfig.instruments[0]->get()->data());
		} else {
			for (int i = 0; i < ARRAYSIZE(_musChannels); ++i) {
				if (_musChannels[i] && _soundConfig.instruments[i + 1])
					_sdrv->loadInstrument(_musChannels[i], MacLowLevelPCMDriver::kImmediate, _soundConfig.instruments[i + 1]->get()->data());
			}
		}
		break;
	default:
		break;
	}
}

void LoomMacSnd::disposeAllChannels() {
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

void LoomMacSnd::detectQuality() {
	if (_machineRating == 0) {
		if (isSoundCardType10())
			_machineRating = 1;
		/*else if (0)
			_machineRating = 2;*/
		else
			_machineRating = 3;
	}

	_defaultChanConfig = _effectiveChanConfig = _chanConfigTable[_machineRating * 3 + _selectedQuality];
	_curChanConfig = 0;
	disposeAllChannels();
	setupChannels();
	_chanConfigTable[_machineRating * 3 + _selectedQuality] = _defaultChanConfig;
}

bool LoomMacSnd::isSoundCardType10() const {
	return _mixerThread ? (_machineRating == 1) : (_vm->VAR_SOUNDCARD != 0xff && _vm->VAR(_vm->VAR_SOUNDCARD) == 10);
}

LoomMacSnd::Instrument::Instrument(uint16 id, Common::SeekableReadStream *&in, Common::String &&name) : _id(id), _name(name) {
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
	in->read(buff, _snd.len);
	_snd.data = Common::SharedPtr<const byte> (buff, Common::ArrayDeleter<const byte>());
}

bool LoomMacSnd::loadInstruments() {
	static const char *tryNames[] = {
		"Loom",
		"Loom\xaa",
		"Loom PPC",
		"Loom\xaa PPC"
	};

	const Common::CodePage tryCodePages[] = {
		Common::kMacRoman,
		Common::kISO8859_1
	};

	Common::MacResManager resMan;
	Common::Path resFile;
	for (int i = 0; resFile.empty() && i < ARRAYSIZE(tryNames); ++i) {
		for (int ii = 0; resFile.empty() && ii < ARRAYSIZE(tryCodePages); ++ii) {
			Common::U32String fn(tryNames[i], tryCodePages[ii]);
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
		warning("LoomMacSnd::loadInstruments(): Loom resource fork not found");
		return false;
	}

	Common::MacResIDArray ids = resMan.getResIDArray(MKTAG('s', 'n', 'd', ' '));
	for (uint i = 0; i < ids.size(); ++i) {
		Common::SeekableReadStream *str = resMan.getResource(MKTAG('s', 'n', 'd', ' '), ids[i]);
		if (!str || str->readUint16BE() != 1) {
			static const char *const errStr[2] = { "Failed to load", "Invalid sound resource format for" };
			warning("LoomMacSnd::loadInstruments(): %s instrument with id 0x%04x", errStr[str ? 1 : 0], ids[i]);
			delete str;
			return false;
		}
		_instruments.push_back(Common::SharedPtr<Instrument>(new Instrument(ids[i], str, resMan.getResName(MKTAG('s', 'n', 'd', ' '), ids[i]))));
		delete str;
	}

	return true;
}

const Common::SharedPtr<LoomMacSnd::Instrument> *LoomMacSnd::findInstrument(uint16 id) const {
	Common::Array<Common::SharedPtr<Instrument> >::const_iterator replacement = _instruments.end();
	for (Common::Array<Common::SharedPtr<Instrument> >::const_iterator i = _instruments.begin(); i != _instruments.end(); ++i) {
		if ((*i)->id() == id)
			return i;
		else if ((*i)->id() == 0x2D1C)
			replacement = i;
	}
	return (replacement != _instruments.end()) ? replacement : nullptr;
}

void LoomMacSnd::SoundConfig::load(const byte *data) {
	sndRes6 = READ_BE_UINT16(data + 6) & 0xff;
	switchable = READ_BE_UINT16(data + 8) >> 8;
	sndRes10 = READ_BE_UINT16(data + 10) >> 8;
	chanSetup = READ_BE_UINT16(data + 16);
	timbre = READ_BE_UINT16(data + 18);
	for (int i = 0; i < 5; ++i)
		instruments[i] = player->findInstrument(READ_BE_UINT16(data + 20 + 2 * i));
}

#undef ASC_DEVICE_RATE
#undef PCM_BUFFER_SIZE

} // End of namespace Scumm
