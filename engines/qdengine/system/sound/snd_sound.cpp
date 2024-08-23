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

#include "audio/audiostream.h"
#include "common/debug.h"

#include "qdengine/qdengine.h"
#include "qdengine/system/sound/snd_sound.h"
#include "qdengine/system/sound/wav_sound.h"


namespace QDEngine {

sndSound::~sndSound() {
	release_sound_buffer();
}

bool sndSound::create_sound_buffer() {
	if (!sound())
		return false;

	return true;
}

bool sndSound::release_sound_buffer() {
	if (!is_stopped())
		stop();

	return true;
}

bool sndSound::play() {
	debugC(5, kDebugSound, "sndSound::play(). this: %p",  (void *)this);

	_flags &= ~SOUND_FLAG_PAUSED;

	_sound->_audioStream->rewind();

	if (_flags & SOUND_FLAG_LOOPING) {
		Audio::AudioStream *audio = new Audio::LoopingAudioStream(_sound->_audioStream, 0, DisposeAfterUse::NO);
		g_system->getMixer()->playStream(Audio::Mixer::kSFXSoundType, &_audHandle, audio, -1, Audio::Mixer::kMaxChannelVolume, 0,  DisposeAfterUse::NO);
	} else {
		g_system->getMixer()->playStream(Audio::Mixer::kSFXSoundType, &_audHandle, _sound->_audioStream, -1, Audio::Mixer::kMaxChannelVolume, 0,  DisposeAfterUse::NO);
	}

	return true;
}

bool sndSound::stop() {
	debugC(5, kDebugSound, "sndSound::stop(). this: %p",  (void *)this);
	g_system->getMixer()->stopHandle(_audHandle);

	if (_sound && _sound->_audioStream)
		_sound->_audioStream->seek(0);

	return true;
}

void sndSound::pause() {
	debugC(5, kDebugSound, "sndSound::pause(). this: %p",  (void *)this);

	_flags |= SOUND_FLAG_PAUSED;
	g_system->getMixer()->pauseHandle(_audHandle, true);
}

void sndSound::resume() {
	debugC(5, kDebugSound, "sndSound::resume(). this: %p",  (void *)this);

	g_system->getMixer()->pauseHandle(_audHandle, false);
}

sndSound::status_t sndSound::status() const {
	if (is_paused())
		return sndSound::SOUND_PAUSED;

	if (g_system->getMixer()->isSoundHandleActive(_audHandle))
		return SOUND_PLAYING;

	return SOUND_STOPPED;
}

bool sndSound::is_stopped() const {
	switch (status()) {
	case SOUND_PLAYING:
	case SOUND_PAUSED:
		return false;
	default:
		return true;
	}
}

bool sndSound::set_volume(int vol) {
	g_system->getMixer()->setChannelVolume(_audHandle, vol);
	return true;
}

bool sndSound::change_frequency(float coeff) {
	if (coeff != 1.0)
		warning("STUB: sndSound::change_frequency(%f) '%s'", coeff, transCyrillic(sound()->_fname.toString()));
	return true;
}

} // namespace QDEngine
