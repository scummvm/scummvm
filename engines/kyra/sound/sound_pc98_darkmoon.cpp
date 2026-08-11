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

#ifdef ENABLE_EOB

#include "kyra/sound/sound_intern.h"
#include "kyra/resource/resource.h"
#include "kyra/sound/drivers/capcom98.h"

#include "common/config-manager.h"

namespace Kyra {

SoundPC98_Darkmoon::SoundPC98_Darkmoon(KyraEngine_v1 *vm, MidiDriver::DeviceHandle dev, Audio::Mixer *mixer) : Sound(vm, mixer),
	_vm(vm), _driver(nullptr), _soundData(nullptr), _currentResourceSet(-1), _ready(false), _dev(dev), _drvType(kPC98), _lastTrack(-1) {

	memset(&_resInfo, 0, sizeof(_resInfo));
	_soundData = new uint8[20600];
	memset(_soundData, 0, 20600);
	_fileBuffer = new uint8[10500];
	memset(_fileBuffer, 0, 10500);

	MusicType type = MidiDriver::getMusicType(dev);
	if (type == MT_MT32)
		_drvType = kMidiMT32;
	else if (type == MT_GM)
		_drvType = kMidiGM;
}

SoundPC98_Darkmoon::~SoundPC98_Darkmoon() {
	delete _driver;
	delete[] _soundData;
	delete[] _fileBuffer;
	for (int i = 0; i < 3; i++)
		initAudioResourceInfo(i, nullptr);
}

Sound::kType SoundPC98_Darkmoon::getMusicType() const {
	return _drvType;
}

Sound::kType SoundPC98_Darkmoon::getSfxType() const {
	return kPC98;
}

bool SoundPC98_Darkmoon::init() {
	_driver = new CapcomPC98AudioDriver(_mixer, _dev);
	_ready = (_soundData && _driver && _driver->isUsable());
	return _ready;
}

void SoundPC98_Darkmoon::initAudioResourceInfo(int set, void *info) {
	if (set < kMusicIntro || set > kMusicFinale)
		return;
	delete _resInfo[set];
	_resInfo[set] = info ? new SoundResourceInfo_PC(*(SoundResourceInfo_PC*)info) : nullptr;
}

void SoundPC98_Darkmoon::selectAudioResourceSet(int set) {
	if (set < kMusicIntro || set > kMusicFinale || set == _currentResourceSet || !_ready)
		return;
	if (_resInfo[set])
		_currentResourceSet = set;
}

void SoundPC98_Darkmoon::loadSoundFile(uint file) {
	if (!_ready)
		return;

	if (file < res()->fileListSize)
		loadSoundFile(res()->fileList[file]);
}

void SoundPC98_Darkmoon::loadSoundFile(const Common::Path &name) {
	if (!_ready)
		return;

	haltTrack();
	stopAllSoundEffects();

	Common::Path path(name);
	path.appendInPlace(_drvType == kPC98 ? ".SDO" : ".SDM");
	if (!_ready || _soundFileLoaded == path)
		return;

	Common::SeekableReadStream *in = _vm->resource()->createReadStream(path);
	if (!in)
		error("SoundPC98_Darkmoon::loadSoundFile(): Failed to load sound file '%s'", path.toString().c_str());

	uint16 sz = in->readUint16LE();
	uint8 cmp = in->readByte();
	in->seek(1, SEEK_CUR);
	uint32 outSize = in->readUint32LE();
	if ((cmp == 0 && outSize > 10500) || (cmp != 0 && outSize > 20600))
		error("SoundPC98_Darkmoon::loadSoundFile(): Failed to load sound file '%s'", path.toString().c_str());
	sz -= in->pos();
	in->seek(2, SEEK_CUR);

	memset(_fileBuffer, 0, 10500);
	uint16 readSize = in->read(_fileBuffer, 10500);
	assert(sz == readSize);
	delete in;

	memset(_soundData, 0, 20600);
	if (cmp == 0) {
		memcpy(_soundData, _fileBuffer, outSize);
	} else if (cmp == 3) {
		Screen::decodeFrame3(_fileBuffer, _soundData, outSize, true);
	} else if (cmp == 4) {
		Screen::decodeFrame4(_fileBuffer, _soundData, outSize);
	} else {
		error("SoundPC98_Darkmoon::loadSoundFile(): Failed to load sound file '%s'", path.toString().c_str());
	}

	uint16 instrOffs = READ_LE_UINT16(_soundData);
	if (instrOffs >= 20600)
		error("SoundPC98_Darkmoon::loadSoundFile(): Failed to load sound file '%s'", path.toString().c_str());

	_driver->loadFMInstruments(_soundData + instrOffs);
	_driver->reset();
}

void SoundPC98_Darkmoon::playTrack(uint8 track) {
	if (track == 0 || track == 2)
		_lastTrack = track;
	playSoundEffect(track, 127);
}

void SoundPC98_Darkmoon::haltTrack() {
	if (!_ready)
		return;
	_driver->stopSong();
	_lastTrack = -1;
}

bool SoundPC98_Darkmoon::isPlaying() const {
	return _ready && _driver && _driver->songIsPlaying();
}

void SoundPC98_Darkmoon::playSoundEffect(uint16 track, uint8 vol) {
	if (!_ready)
		return;

	if (track == 0 || track == 2) {
		restartBackgroundMusic();
		return;
	}

	const uint8 *data = getData(track);
	if (!data)
		return;

	if (track < 52 || track > 67) {
		if (_sfxEnabled)
			_driver->startSoundEffect(data, vol);
	} else if (_musicEnabled) {
		_lastTrack = track;
		_driver->startSong(data, vol, false);
	}
}

void SoundPC98_Darkmoon::stopAllSoundEffects() {
	if (_ready)
		_driver->stopSoundEffect();
}

void SoundPC98_Darkmoon::beginFadeOut() {
	_driver->fadeOut();
}

void SoundPC98_Darkmoon::pause(bool paused) {
	if (_ready && paused)
		_driver->allNotesOff();
}

int SoundPC98_Darkmoon::checkTrigger() {
	return _driver ? _driver->checkSoundMarker() : 99;
}

void SoundPC98_Darkmoon::restartBackgroundMusic() {
	if (_lastTrack == -1) {
		haltTrack();
		stopAllSoundEffects();
	} else {
		_lastTrack = -1;
		const uint8 *data = getData(0);
		if (!data)
			return;
		if (_musicEnabled)
			_driver->startSong(data, 127, true);
	}
}

const uint8 *SoundPC98_Darkmoon::getData(uint16 track) const {
	if (!_ready || track >= 120)
		return nullptr;

	uint16 offset = READ_LE_UINT16(&_soundData[(track + 1) << 1]);
	return (offset < 20600) ? &_soundData[offset] : nullptr;
}

void SoundPC98_Darkmoon::updateVolumeSettings() {
	if (!_driver || !_ready)
		return;

	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	_driver->setMusicVolume((mute ? 0 : ConfMan.getInt("music_volume")));
	_driver->setSoundEffectVolume((mute ? 0 : ConfMan.getInt("sfx_volume")));
}

} // End of namespace Kyra

#endif
