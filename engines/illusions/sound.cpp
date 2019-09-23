/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/config-manager.h"
#include "illusions/illusions.h"
#include "illusions/sound.h"
#include "illusions/time.h"
#include "audio/mididrv.h"
#include "audio/midiparser.h"

namespace Illusions {

// MusicPlayer

MusicPlayer::MusicPlayer()
	: _musicId(0), _flags(0) {
	_flags = 1; // TODO?
}

MusicPlayer::~MusicPlayer() {
	stop();
}

void MusicPlayer::play(uint32 musicId, bool looping, int16 volume, int16 pan) {
	debug(1, "MusicPlayer::play(%08X)", musicId);
	if (_flags & 1) {
		stop();
		_musicId = musicId;
		_flags |= 2;
		_flags &= ~4;
		if (looping) {
			_flags |= 8;
		} else {
			_flags &= ~8;
		}
		Common::String filename = Common::String::format("%08x.wav", _musicId);
		Common::File *fd = new Common::File();
		fd->open(filename);
		Audio::AudioStream *audioStream = Audio::makeLoopingAudioStream(Audio::makeWAVStream(fd, DisposeAfterUse::YES), looping ? 0 : 1);
		g_system->getMixer()->playStream(Audio::Mixer::kMusicSoundType, &_soundHandle, audioStream, -1, volume, pan);
	}
}

void MusicPlayer::stop() {
	debug(1, "MusicPlayer::stop()");
	if ((_flags & 1) && (_flags & 2)) {
		if (g_system->getMixer()->isSoundHandleActive(_soundHandle))
			g_system->getMixer()->stopHandle(_soundHandle);
		_flags &= ~2;
		_flags &= ~4;
		_flags &= ~8;
		_musicId = 0;
	}
}

bool MusicPlayer::isPlaying() {
	return (_flags & 1) && (_flags & 2) && g_system->getMixer()->isSoundHandleActive(_soundHandle);
}

// MidiPlayer

MidiPlayer::MidiPlayer()
	: _isIdle(true), _isPlaying(false), _isCurrentlyPlaying(false), _isLooped(false),
	_loopedMusicId(0), _queuedMusicId(0), _loadedMusicId(0),
	_data(0), _dataSize(0) {

	_data = 0;
	_dataSize = 0;
	_isGM = false;

	MidiPlayer::createDriver();

	int ret = _driver->open();
	if (ret == 0) {
		if (_nativeMT32)
			_driver->sendMT32Reset();
		else
			_driver->sendGMReset();

		_driver->setTimerCallback(this, &timerCallback);
	}
}

MidiPlayer::~MidiPlayer() {
	sysMidiStop();
}

bool MidiPlayer::play(uint32 musicId) {
	debug("MidiPlayer::play(%08X)", musicId);
	bool isMusicLooping = true; // TODO Use actual flag

	if (!_isIdle)
		return false;

	if (_isPlaying) {
		if (isMusicLooping) {
			_loopedMusicId = musicId;
		} else {
			_queuedMusicId = musicId;
			_isIdle = false;
		}
		return true;
	}

	if (_isCurrentlyPlaying && _loopedMusicId == musicId)
		return true;

	sysMidiStop();

    _isLooped = isMusicLooping;
    if (_isLooped) {
		_loopedMusicId = musicId;
    } else {
		_isPlaying = true;
	}

	sysMidiPlay(musicId);

	_isCurrentlyPlaying = true;

	return true;
}

void MidiPlayer::stop() {
	sysMidiStop();
	_isIdle = true;
	_isPlaying = false;
	_isCurrentlyPlaying = false;
	_loopedMusicId = 0;
	_queuedMusicId = 0;
}

void MidiPlayer::sysMidiPlay(uint32 musicId) {
	Common::StackLock lock(_mutex);

	Common::String filename = Common::String::format("%08x.mid", musicId);
	debug(0, "MidiPlayer::sysMidiPlay() %s", filename.c_str());

	Common::File fd;
	if (!fd.open(filename)) {
		error("MidiPlayer::sysMidiPlay() Could not open %s", filename.c_str());
	}

	_dataSize = fd.size();
	_data = new byte[_dataSize];
	fd.read(_data, _dataSize);

	_isGM = true;
	_loadedMusicId = musicId;

	MidiParser *parser = MidiParser::createParser_SMF();
	if (parser->loadMusic(_data, _dataSize)) {
		parser->setTrack(0);
		parser->setMidiDriver(this);
		parser->setTimerRate(_driver->getBaseTempo());
		parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);

		_parser = parser;

		syncVolume();

		Audio::MidiPlayer::_isLooping = _isLooped;
		Audio::MidiPlayer::_isPlaying = true;
	}
}

void MidiPlayer::sysMidiStop() {
	Audio::MidiPlayer::stop();
	delete[] _data;
	_data = 0;
	_dataSize = 0;
	_loadedMusicId = 0;
}

void MidiPlayer::send(uint32 b) {
	if ((b & 0xF0) == 0xC0 && !_isGM && !_nativeMT32) {
		b = (b & 0xFFFF00FF) | MidiDriver::_mt32ToGm[(b >> 8) & 0xFF] << 8;
	}

	Audio::MidiPlayer::send(b);
}

void MidiPlayer::sendToChannel(byte channel, uint32 b) {
	if (!_channelsTable[channel]) {
		_channelsTable[channel] = (channel == 9) ? _driver->getPercussionChannel() : _driver->allocateChannel();
		// If a new channel is allocated during the playback, make sure
		// its volume is correctly initialized.
		if (_channelsTable[channel])
			_channelsTable[channel]->volume(_channelsVolume[channel] * _masterVolume / 255);
	}

	if (_channelsTable[channel])
		_channelsTable[channel]->send(b);
}

void MidiPlayer::endOfTrack() {
	uint32 nextMusicId = _queuedMusicId;
	if (nextMusicId == 0)
		nextMusicId = _loopedMusicId;

	if (_isLooped && _loadedMusicId == nextMusicId) {
		Audio::MidiPlayer::endOfTrack();
		return;
	}

	sysMidiStop();
	_queuedMusicId = 0;
	_isIdle = true;
	play(nextMusicId);
}

// VoicePlayer

VoicePlayer::VoicePlayer() : _wasPlaying(false), _isPaused(false) {
}

VoicePlayer::~VoicePlayer() {
	stop();
}

bool VoicePlayer::cue(const char *voiceName) {
	debug(1, "VoicePlayer::cue(%s)", voiceName);
	_voiceName = voiceName;
	_voiceStatus = 2;
	if (!isEnabled()) {
		_voiceStatus = 3;
		return false;
	}
	return true;
}

void VoicePlayer::stopCueing() {
	_voiceStatus = 3;
}

void VoicePlayer::start(int16 volume, int16 pan) {
	Common::String filename = Common::String::format("%s.wav", _voiceName.c_str());
	Common::File *fd = new Common::File();
	fd->open(filename);
	Audio::AudioStream *audioStream = Audio::makeWAVStream(fd, DisposeAfterUse::YES);
	g_system->getMixer()->playStream(Audio::Mixer::kSpeechSoundType, &_soundHandle, audioStream, -1, volume, pan);
	_voiceStatus = 4;
}

void VoicePlayer::stop() {
	if (g_system->getMixer()->isSoundHandleActive(_soundHandle))
		g_system->getMixer()->stopHandle(_soundHandle);
	_voiceStatus = 1;
	_voiceName.clear();
}

void VoicePlayer::pause() {
	if (!_isPaused) {
		_isPaused = true;
		_wasPlaying = isPlaying();
		g_system->getMixer()->pauseHandle(_soundHandle, true);
	}
}

void VoicePlayer::unpause() {
	if (_isPaused) {
		_isPaused = false;
		if (_wasPlaying)
			g_system->getMixer()->pauseHandle(_soundHandle, false);
	}
}

bool VoicePlayer::isPlaying() {
	return g_system->getMixer()->isSoundHandleActive(_soundHandle);
}

bool VoicePlayer::isEnabled() {
	// TODO
	return true;
}

bool VoicePlayer::isCued() {
	return _voiceStatus == 2;
}

// Sound

Sound::Sound(uint32 soundEffectId, uint32 soundGroupId, bool looping)
	: _stream(0), _soundEffectId(soundEffectId), _soundGroupId(soundGroupId), _looping(looping) {
	load();
}

Sound::~Sound() {
	unload();
}

void Sound::load() {
	Common::String filename = Common::String::format("%08x/%08x.wav", _soundGroupId, _soundEffectId);
	Common::File *fd = new Common::File();
	if (!fd->open(filename)) {
		delete fd;
		error("SoundMan::loadSound() Could not load %s", filename.c_str());
	}
	_stream = Audio::makeWAVStream(fd, DisposeAfterUse::YES);
}

void Sound::unload() {
	debug(1, "Sound::unload() %08X", _soundEffectId);
	stop();
	delete _stream;
	_stream = 0;
}

void Sound::play(int16 volume, int16 pan) {
	stop();
	_stream->rewind();
	Audio::AudioStream *audioStream = new Audio::LoopingAudioStream(_stream, _looping ? 0 : 1, DisposeAfterUse::NO);
	g_system->getMixer()->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, audioStream,
		-1, volume, pan, DisposeAfterUse::YES);
}

void Sound::stop() {
	if (isPlaying())
		g_system->getMixer()->stopHandle(_soundHandle);
}

bool Sound::isPlaying() {
	return g_system->getMixer()->isSoundHandleActive(_soundHandle);
}

bool Sound::isLooping() {
	return _looping;
}

// SoundMan

SoundMan::SoundMan(IllusionsEngine *vm)
	: _vm(vm), _musicNotifyThreadId(0) {
	_musicPlayer = new MusicPlayer();
	_midiPlayer = new MidiPlayer();
	_voicePlayer = new VoicePlayer();
}

SoundMan::~SoundMan() {
	delete _musicPlayer;
	delete _midiPlayer;
	delete _voicePlayer;
	unloadSounds(0);
}

void SoundMan::update() {
	updateMidi();
	if (_musicNotifyThreadId && !_musicPlayer->isPlaying())
		_vm->notifyThreadId(_musicNotifyThreadId);
}

void SoundMan::playMusic(uint32 musicId, int16 type, int16 volume, int16 pan, uint32 notifyThreadId) {
	_vm->notifyThreadId(_musicNotifyThreadId);
	_musicPlayer->play(musicId, type == 2, volume, pan);
	_musicNotifyThreadId = notifyThreadId;
}

void SoundMan::stopMusic() {
	_musicPlayer->stop();
}

void SoundMan::playMidiMusic(uint32 musicId) {
	if (!_midiPlayer->play(musicId)) {
		_midiMusicQueue.push_back(musicId);
	}
}

void SoundMan::stopMidiMusic() {
	_midiPlayer->stop();
}

void SoundMan::fadeMidiMusic(int16 finalVolume, int16 duration, uint32 notifyThreadId) {
	_midiMusicFader._active = true;
	_midiMusicFader._notifyThreadId = notifyThreadId;
	_midiMusicFader._startVolume = _midiMusicFader._currVolume;
	_midiMusicFader._finalVolume = finalVolume;
	_midiMusicFader._startTime = getCurrentTime();
	_midiMusicFader._duration = duration;
}

void SoundMan::clearMidiMusicQueue() {
	_midiMusicQueue.clear();
}

bool SoundMan::cueVoice(const char *voiceName) {
	return _voicePlayer->cue(voiceName);
}

void SoundMan::stopCueingVoice() {
	_voicePlayer->stopCueing();
}

void SoundMan::startVoice(int16 volume, int16 pan) {
	_voicePlayer->start(calcAdjustedVolume("speech_volume", (uint8)volume), pan);
}

void SoundMan::stopVoice() {
	_voicePlayer->stop();
}

void SoundMan::pauseVoice() {
	_voicePlayer->pause();
}

void SoundMan::unpauseVoice() {
	_voicePlayer->unpause();
}

bool SoundMan::isVoicePlaying() {
	return _voicePlayer->isPlaying();
}

bool SoundMan::isVoiceEnabled() {
	return _voicePlayer->isEnabled();
}

bool SoundMan::isVoiceCued() {
	return _voicePlayer->isCued();
}

void SoundMan::loadSound(uint32 soundEffectId, uint32 soundGroupId, bool looping) {
	Sound *sound = new Sound(soundEffectId, soundGroupId, looping);
	_sounds.push_front(sound);
}

void SoundMan::playSound(uint32 soundEffectId, int16 volume, int16 pan) {
	Sound *sound = getSound(soundEffectId);
	if (sound)
		sound->play(calcAdjustedVolume("sfx_volume", (uint8)volume), pan);
}

void SoundMan::stopSound(uint32 soundEffectId) {
	Sound *sound = getSound(soundEffectId);
	if (sound)
		sound->stop();
}

void SoundMan::stopLoopingSounds() {
	for (SoundListIterator it = _sounds.begin(); it != _sounds.end(); ++it) {
		Sound *sound = *it;
		if (sound->isPlaying() && sound->isLooping()) {
			sound->stop();
		}
	}
}

void SoundMan::unloadSounds(uint32 soundGroupId) {
	SoundListIterator it = _sounds.begin();
	while (it != _sounds.end()) {
		Sound *sound = *it;
		if (soundGroupId == 0 || sound->_soundGroupId == soundGroupId) {
			delete sound;
			it = _sounds.erase(it);
		} else
			++it;
	}
}

Sound *SoundMan::getSound(uint32 soundEffectId) {
	for (SoundListIterator it = _sounds.begin(); it != _sounds.end(); ++it) {
		if ((*it)->_soundEffectId == soundEffectId)
			return *it;
	}
	return 0;
}

void SoundMan::updateMidi() {
	if (_midiPlayer->isIdle() & !_midiMusicQueue.empty()) {
		uint32 musicId = _midiMusicQueue.front();
		_midiMusicQueue.remove_at(0);
		_midiPlayer->play(musicId);
	}
	updateMidiMusicFader();
}

void SoundMan::updateMidiMusicFader() {
	if (_midiMusicFader._active) {
		int16 currTime = getCurrentTime();
		if (currTime - _midiMusicFader._startTime > _midiMusicFader._duration) {
			_midiMusicFader._active = false;
			currTime = _midiMusicFader._startTime + _midiMusicFader._duration;
			if (_midiMusicFader._notifyThreadId) {
				_vm->notifyThreadId(_midiMusicFader._notifyThreadId);
				_midiMusicFader._notifyThreadId = 0;
			}
		}
		const int16 elapsedTime = currTime - _midiMusicFader._startTime;
		const int16 volumeDelta = _midiMusicFader._finalVolume - _midiMusicFader._startVolume;
		const int masterMusicVolume = _vm->_mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType);
		_midiMusicFader._currVolume = _midiMusicFader._startVolume + (elapsedTime * volumeDelta / _midiMusicFader._duration);
		_midiPlayer->setVolume(_midiMusicFader._currVolume * masterMusicVolume / 255);
	}
}

void SoundMan::setMusicVolume(uint16 volume) {
	ConfMan.setInt("music_volume", volume);
	_midiPlayer->syncVolume();
	ConfMan.flushToDisk();
}

void SoundMan::setSfxVolume(uint16 volume) {
	ConfMan.setInt("sfx_volume", volume);
	ConfMan.flushToDisk();
}

void SoundMan::setSpeechVolume(uint16 volume) {
	ConfMan.setInt("speech_volume", volume);
	ConfMan.flushToDisk();
}

uint16 SoundMan::calcAdjustedVolume(const Common::String &volumeConfigKey, uint16 volume) {
	uint16 masterVolume = (uint16)ConfMan.getInt(volumeConfigKey);
	return (uint16)(((float)masterVolume/256) * (float)volume);
}

uint16 SoundMan::getMusicVolume() {
	return (uint16)ConfMan.getInt("music_volume");
}

uint16 SoundMan::getSfxVolume() {
	return (uint16)ConfMan.getInt("sfx_volume");
}

uint16 SoundMan::getSpeechVolume() {
	return (uint16)ConfMan.getInt("speech_volume");
}
} // End of namespace Illusions
