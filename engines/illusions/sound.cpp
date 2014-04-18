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

#include "illusions/illusions.h"
#include "illusions/sound.h"

namespace Illusions {

// MusicPlayer

MusicPlayer::MusicPlayer(Audio::Mixer *mixer)
	: _mixer(mixer), _musicId(0), _flags(0) {
	_flags = 1; // TODO?
}

MusicPlayer::~MusicPlayer() {
	stop();
}

void MusicPlayer::play(uint32 musicId, bool looping, int16 volume, int16 pan) {
	debug("MusicPlayer::play(%08X)", musicId);
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
		_mixer->playStream(Audio::Mixer::kMusicSoundType, &_soundHandle, audioStream, -1, volume, pan);
	}
}

void MusicPlayer::stop() {
	debug("MusicPlayer::stop()");
	if ((_flags & 1) && (_flags & 2)) {
		if (_mixer->isSoundHandleActive(_soundHandle))
			_mixer->stopHandle(_soundHandle);
		_flags &= ~2;
		_flags &= ~4;
		_flags &= ~8;
		_musicId = 0;
	}
}

bool MusicPlayer::isPlaying() {
	return (_flags & 1) && (_flags & 2) && _mixer->isSoundHandleActive(_soundHandle);
}

// VoicePlayer

VoicePlayer::VoicePlayer(Audio::Mixer *mixer)
	: _mixer(mixer) {
}

VoicePlayer::~VoicePlayer() {
	stop();
}

bool VoicePlayer::cue(const char *voiceName) {
debug("VoicePlayer::cue(%s)", voiceName);
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
	_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_soundHandle, audioStream, -1, volume, pan);
	_voiceStatus = 4;
}

void VoicePlayer::stop() {
	if (_mixer->isSoundHandleActive(_soundHandle))
		_mixer->stopHandle(_soundHandle);
	_voiceStatus = 1;
	_voiceName.clear();
}

bool VoicePlayer::isPlaying() {
	return _mixer->isSoundHandleActive(_soundHandle);
}

bool VoicePlayer::isEnabled() {
	// TODO
	return true;
}

bool VoicePlayer::isCued() {
	return _voiceStatus == 2;
}

// SoundMan

SoundMan::SoundMan(IllusionsEngine *vm)
	: _vm(vm), _musicNotifyThreadId(0) {
	_musicPlayer = new MusicPlayer(_vm->_mixer);
	_voicePlayer = new VoicePlayer(_vm->_mixer);
}

SoundMan::~SoundMan() {
	delete _musicPlayer;
	delete _voicePlayer;
}

void SoundMan::update() {
	// TODO voc_testCued();
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

bool SoundMan::cueVoice(const char *voiceName) {
	return _voicePlayer->cue(voiceName);
}

void SoundMan::stopCueingVoice() {
	_voicePlayer->stopCueing();
}

void SoundMan::startVoice(int16 volume, int16 pan) {
	_voicePlayer->start(volume, pan);
}

void SoundMan::stopVoice() {
	_voicePlayer->stop();
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

} // End of namespace Illusions
