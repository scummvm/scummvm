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

SOUNDCLIP::SOUNDCLIP() : _state(SoundClipInitial), _panning(128), _panningAsPercentage(0),
		_sourceClip(nullptr), _sourceClipType(0), _speed(1000), _priority(50),
		_xSource(-1), _ySource(-1), _maximumPossibleDistanceAway(0),
		_volAsPercentage(100), _vol(0), _volModifier(0), _repeat(false) {
	_mixer = ::AGS::g_vm->_mixer;
}

void SOUNDCLIP::poll() {
	bool playing = is_playing();
	if (playing)
		_state = SoundClipPlaying;
	else if (_state == SoundClipPlaying)
		_state = SoundClipStopped;
}

SOUNDCLIP::~SOUNDCLIP() {
	destroy();
}

void SOUNDCLIP::set_speed(int new_speed) {
	warning("TODO: SOUNDCLIP::set_speed");
	_speed = new_speed;
}

void SOUNDCLIP::adjust_volume() {
	// TODO: See if this method is needed
}

int SOUNDCLIP::play_from(int position) {
	// TODO: Implement playing from arbitrary positions
	if (position == 0) {
		play();
		return 1;
	} else {
		return 0;
	}
}


/*------------------------------------------------------------------*/

SoundClipWaveBase::SoundClipWaveBase(Audio::AudioStream *stream, int volume, bool repeat) :
		SOUNDCLIP(), _stream(stream) {
	_mixer = ::AGS::g_vm->_mixer;
	_repeat = repeat;
	_vol = volume;

	if (repeat) {
		Audio::SeekableAudioStream *str = dynamic_cast<Audio::SeekableAudioStream *>(stream);
		assert(str);
		_stream = new Audio::LoopingAudioStream(str, 0);
	}
}

void SoundClipWaveBase::destroy() {
	stop();
	delete _stream;
	_stream = nullptr;
}

int SoundClipWaveBase::play() {
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, _stream,
		-1, 255, 0, DisposeAfterUse::NO);
	return 1;
}

void SoundClipWaveBase::stop() {
	_mixer->stopHandle(_soundHandle);
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

int SoundClipWaveBase::get_volume() const {
	return _mixer->getChannelVolume(_soundHandle);
}

void SoundClipWaveBase::set_volume(int volume) {
	_mixer->setChannelVolume(_soundHandle, volume);
}

void SoundClipWaveBase::set_panning(int newPanning) {
	_mixer->setChannelBalance(_soundHandle, newPanning);
}

} // namespace AGS3
