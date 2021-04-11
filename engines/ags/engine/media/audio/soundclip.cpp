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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ags/engine/media/audio/soundclip.h"
#include "ags/ags.h"

namespace AGS3 {

SOUNDCLIP::SOUNDCLIP() : _panning(12./8), _panningAsPercentage(0),
		_sourceClip(nullptr), _sourceClipType(0), _speed(1000), _priority(50),
		_xSource(-1), _ySource(-1), _maximumPossibleDistanceAway(0), _muted(false),
		_volAsPercentage(0), _vol(0), _volModifier(0), _repeat(false), _directionalVolModifier(0) {
}

/*------------------------------------------------------------------*/

SoundClipWaveBase::SoundClipWaveBase(Audio::AudioStream *stream, int volume, bool repeat) :
		SOUNDCLIP(), _state(SoundClipInitial), _stream(stream) {
	_mixer = ::AGS::g_vm->_mixer;
	_repeat = repeat;
	_vol = volume;

	if (repeat) {
		Audio::SeekableAudioStream *str = dynamic_cast<Audio::SeekableAudioStream *>(stream);
		if (str)
			_stream = new Audio::LoopingAudioStream(str, 0);
	}
}

void SoundClipWaveBase::destroy() {
	_mixer->stopHandle(_soundHandle);
	delete _stream;
	_stream = nullptr;
}

void SoundClipWaveBase::poll() {
	bool playing = is_playing();
	if (playing)
		_state = SoundClipPlaying;
	else if (_state == SoundClipPlaying)
		_state = SoundClipStopped;
}

int SoundClipWaveBase::play() {
	_mixer->playStream(_soundType, &_soundHandle, _stream,
		-1, 255, 0, DisposeAfterUse::NO);
	return 1;
}

int SoundClipWaveBase::play_from(int position) {
	if (position == 0) {
		play();
		return 1;
	} else {
		// TODO: Implement playing from arbitrary positions. This is
		// used when restoring savegames to resume the music at the
		// point the savegame was made. For now, since ScummVM doesn't
		// have seek for audio streams, we'll restart from the beginning
		play();
		return 1;
	}
}

void SoundClipWaveBase::pause() {
	_mixer->pauseHandle(_soundHandle, false);
	_state = SoundClipPaused;
}

void SoundClipWaveBase::resume() {
	_mixer->pauseHandle(_soundHandle, false);
	_state = SoundClipPlaying;
	poll();
}

bool SoundClipWaveBase::is_playing() const {
	return _mixer->isSoundHandleActive(_soundHandle);
}

void SoundClipWaveBase::seek(int offset) {
	warning("TODO: SoundClipWaveBase::seek");
}

int SoundClipWaveBase::get_pos() {
	return _mixer->getSoundElapsedTime(_soundHandle) / 1000;
}

int SoundClipWaveBase::get_pos_ms() {
	return _mixer->getSoundElapsedTime(_soundHandle);
}

int SoundClipWaveBase::get_length_ms() {
	warning("TODO: SoundClipWaveBase::get_length_ms");
	return 0;
}

void SoundClipWaveBase::set_volume(int volume) {
	_mixer->setChannelVolume(_soundHandle, volume);
}

void SoundClipWaveBase::set_panning(int newPanning) {
	_mixer->setChannelBalance(_soundHandle, newPanning);
}

void SoundClipWaveBase::set_speed(int new_speed) {
	warning("TODO: SoundClipWaveBase::set_speed");
	_speed = new_speed;
}

void SoundClipWaveBase::adjust_volume() {
	// TODO: See if this method is needed
}

} // namespace AGS3
