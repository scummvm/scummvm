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

#include "bladerunner/audio_speech.h"

#include "bladerunner/aud_stream.h"
#include "bladerunner/audio_mixer.h"
#include "bladerunner/bladerunner.h"

#include "common/debug.h"

namespace BladeRunner {

#define BUFFER_SIZE 200000

void AudioSpeech::ended() {
	//Common::StackLock lock(_mutex);
	_isActive = false;
	_channel = -1;
}

void AudioSpeech::mixerChannelEnded(int channel, void *data) {
	AudioSpeech *audioSpeech = (AudioSpeech*)data;
	audioSpeech->ended();
}

AudioSpeech::AudioSpeech(BladeRunnerEngine *vm) : _vm(vm) {
	_volume = 50;
	_isActive = false;
	_data = new byte[BUFFER_SIZE];
	_channel = -1;
}

AudioSpeech::~AudioSpeech() {
	delete[] _data;
}

bool AudioSpeech::playSpeech(const char *name, int pan) {
	// debug("AudioSpeech::playSpeech(\"%s\")", name);
	Common::ScopedPtr<Common::SeekableReadStream> r(_vm->getResourceStream(name));

	if (!r) {
		warning("AudioSpeech::playSpeech: AUD resource \"%s\" not found", name);
		return false;
	}

	if (r->size() > BUFFER_SIZE) {
		warning("AudioSpeech::playSpeech: AUD larger than buffer size (%d > %d)", r->size(), BUFFER_SIZE);
		return false;
	}

	if (isPlaying()) {
		stopSpeech();
	}

	r->read(_data, r->size());
	if (r->err()) {
		warning("AudioSpeech::playSpeech: Error reading resource \"%s\"", name);
		return false;
	}

	AudStream *audioStream = new AudStream(_data);

	// TODO: shorty mode - set rate of sound to 33khz

	_channel = _vm->_audioMixer->play(
		Audio::Mixer::kSpeechSoundType,
		audioStream,
		100,
		false,
		_volume,
		pan,
		mixerChannelEnded,
		this);

	_isActive = true;

	return true;
}

void AudioSpeech::stopSpeech() {
	//Common::StackLock lock(_mutex);
	if (_channel != -1) {
		_vm->_audioMixer->stop(_channel, 0);
	}
}

bool AudioSpeech::isPlaying() {
	if (_channel == -1) {
		return  false;
	}
	return _isActive;
}

} // End of namespace BladeRunner
