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

#include "ags/engine/media/audio/sound_clip.h"
#include "ags/ags.h"

namespace AGS3 {

SOUNDCLIP::SOUNDCLIP() : _panning(12. / 8), _panningAsPercentage(0),
	_sourceClipID(-1), _sourceClipType(0), _speed(1000), _priority(50),
	_xSource(-1), _ySource(-1), _maximumPossibleDistanceAway(0), _muted(false),
	_vol100(0), _vol255(0), _volModifier(0), _repeat(false), _directionalVolModifier(0) {
}

void SOUNDCLIP::set_volume100(int volume) {
	_vol100 = volume;
	_vol255 = (volume * 255) / 100;
	adjust_volume();
}

// Sets the current volume property in units of 255
void SOUNDCLIP::set_volume255(int volume) {
	_vol255 = volume;
	_vol100 = (_vol255 * 100) / 255;
	adjust_volume();
}

void SOUNDCLIP::set_volume_direct(int vol_percent, int vol_absolute) {
	_vol255 = vol_absolute;
	_vol100 = vol_percent;
	adjust_volume();
}

void SOUNDCLIP::set_mute(bool mute) {
	_muted = mute;
	adjust_volume();
}

void SOUNDCLIP::apply_volume_modifier(int mod) {
	_volModifier = mod;
	adjust_volume();
}

void SOUNDCLIP::apply_directional_modifier(int mod) {
	_directionalVolModifier = mod;
	adjust_volume();
}

bool SOUNDCLIP::update() {
	if (!is_ready())
		return false;

	if (_paramsChanged) {
		auto vol_f = static_cast<float>(get_final_volume()) / 255.0f;
		if (vol_f < 0.0f) {
			vol_f = 0.0f;
		}
		if (vol_f > 1.0f) {
			vol_f = 1.0f;
		}

		auto speed_f = static_cast<float>(_speed) / 1000.0f;
		if (speed_f <= 0.0) {
			speed_f = 1.0f;
		}

		// Sets the pan position, ranging from -100 (left) to +100 (right)
		auto panning_f = (static_cast<float>(_panning) / 100.0f);
		if (panning_f < -1.0f) {
			panning_f = -1.0f;
		}
		if (panning_f > 1.0f) {
			panning_f = 1.0f;
		}

		//audio_core_slot_configure(slot_, vol_f, speed_f, panning_f);
		_paramsChanged = false;
	}
/*
	float pos_f, posms_f;
	PlaybackState core_state = audio_core_slot_get_play_state(slot_, pos_f, posms_f);
	pos = static_cast<int>(pos_f);
	posMs = static_cast<int>(posms_f);
	if (state == core_state || core_state == PlayStateError || core_state == PlayStateFinished) {
		state = core_state;
		return is_ready();
	}

	switch (state) {
	case PlaybackState::PlayStatePlaying:
		state = audio_core_slot_play(slot_);
		break;
	}
*/
	return is_ready();
}

/*------------------------------------------------------------------*/

SoundClipWaveBase::SoundClipWaveBase(Audio::AudioStream *stream, bool repeat) :
	SOUNDCLIP(), _state(SoundClipInitial), _stream(stream) {
	_mixer = ::AGS::g_vm->_mixer;
	_repeat = repeat;
	_vol255 = 255;

	if (repeat) {
		Audio::SeekableAudioStream *str = dynamic_cast<Audio::SeekableAudioStream *>(stream);
		if (str)
			_stream = new Audio::LoopingAudioStream(str, 0);
	}
}

SoundClipWaveBase::~SoundClipWaveBase() {
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
	if (_soundType != Audio::Mixer::kPlainSoundType) {
		_mixer->playStream(_soundType, &_soundHandle, _stream,
			-1, _vol255, 0, DisposeAfterUse::NO);
	} else {
		_waitingToPlay = true;
	}

	return 1;
}

void SoundClipWaveBase::setType(Audio::Mixer::SoundType type) {
	assert(type != Audio::Mixer::kPlainSoundType);
	_soundType = type;

	if (_waitingToPlay) {
		_waitingToPlay = false;

		play();
	}
}

int SoundClipWaveBase::play_from(int position) {
	if (position != 0)
		seek(position);

	play();
	return 1;
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

bool SoundClipWaveBase::is_playing() {
	return _mixer->isSoundHandleActive(_soundHandle) || is_paused();
}

bool SoundClipWaveBase::is_paused() {
	return _state == SoundClipPaused;
}

void SoundClipWaveBase::seek(int offset) {
	Audio::SeekableAudioStream *stream =
		dynamic_cast<Audio::SeekableAudioStream *>(_stream);

	if (stream) {
		stream->seek(Audio::Timestamp(offset));
	} else {
		warning("Audio stream did not support seeking");
	}
}

int SoundClipWaveBase::get_pos() {
	return _mixer->getSoundElapsedTime(_soundHandle);
}

int SoundClipWaveBase::get_pos_ms() {
	return _mixer->getSoundElapsedTime(_soundHandle);
}

int SoundClipWaveBase::get_length_ms() {
	Audio::SeekableAudioStream *stream =
		dynamic_cast<Audio::SeekableAudioStream *>(_stream);

	if (stream) {
		return stream->getLength().msecs();
	} else {
		warning("Unable to determine audio stream length");
		return 0;
	}
}

void SoundClipWaveBase::set_panning(int newPanning) {
	_mixer->setChannelBalance(_soundHandle, newPanning);
}

void SoundClipWaveBase::set_speed(int new_speed) {
	warning("TODO: SoundClipWaveBase::set_speed");
	_speed = new_speed;
}

void SoundClipWaveBase::adjust_volume() {
	_mixer->setChannelVolume(_soundHandle, _vol255);
}

} // namespace AGS3
