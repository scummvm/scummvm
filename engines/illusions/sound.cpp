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
}

void MusicPlayer::play(uint32 musicId, bool looping, int16 volume, int16 pan) {
	debug("MusicPlayer::play(%08X)", musicId);
	if (_flags & 1) {
		stop();
		_musicId = musicId;
		Common::String filename = Common::String::format("%08x.wav", _musicId);
		_flags |= 2;
		_flags &= ~4;
		if (looping) {
			_flags |= 8;
		} else {
			_flags &= ~8;
		}
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

// SoundMan

SoundMan::SoundMan(IllusionsEngine *vm)
	: _vm(vm), _musicNotifyThreadId(0) {
	_musicPlayer = new MusicPlayer(_vm->_mixer);
}

SoundMan::~SoundMan() {
	delete _musicPlayer;
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

} // End of namespace Illusions
