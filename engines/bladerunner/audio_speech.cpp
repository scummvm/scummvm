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

#include "bladerunner/actor.h"
#include "bladerunner/aud_stream.h"
#include "bladerunner/audio_mixer.h"
#include "bladerunner/audio_player.h"
#include "bladerunner/bladerunner.h"

#include "common/debug.h"
#include "common/str.h"

namespace BladeRunner {

const int AudioSpeech::kSpeechSamples[] = { 65, 355, 490, 465, 480, 485, 505, 760, 7655, 7770, 7740, 8170, 2705, 7200, 6460, 5560, 4870, 4555, 3880, 3525, 3595, 3250, 3070 };

void AudioSpeech::ended() {
	//Common::StackLock lock(_mutex);
	_isActive = false;
	_channel = -1;
}

void AudioSpeech::mixerChannelEnded(int channel, void *data) {
	AudioSpeech *audioSpeech = (AudioSpeech *)data;
	audioSpeech->ended();
}

AudioSpeech::AudioSpeech(BladeRunnerEngine *vm) {
	_vm = vm;
	_speechVolume = 50;
	_isActive = false;
	_data = new byte[kBufferSize];
	_channel = -1;
}

AudioSpeech::~AudioSpeech() {
	delete[] _data;
}

bool AudioSpeech::playSpeech(const Common::String &name, int pan) {
	// debug("AudioSpeech::playSpeech(\"%s\")", name);
	Common::ScopedPtr<Common::SeekableReadStream> r(_vm->getResourceStream(name));

	if (!r) {
		warning("AudioSpeech::playSpeech: AUD resource \"%s\" not found", name.c_str());
		return false;
	}

	if (r->size() > kBufferSize) {
		warning("AudioSpeech::playSpeech: AUD larger than buffer size (%d > %d)", r->size(), kBufferSize);
		return false;
	}

	if (isPlaying()) {
		stopSpeech();
	}

	r->read(_data, r->size());
	if (r->err()) {
		warning("AudioSpeech::playSpeech: Error reading resource \"%s\"", name.c_str());
		return false;
	}

	AudStream *audioStream = new AudStream(_data);

	// TODO: shorty mode - set rate of sound to 33khz

	_channel = _vm->_audioMixer->play(
		Audio::Mixer::kSpeechSoundType,
		audioStream,
		100,
		false,
		_speechVolume,
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

bool AudioSpeech::isPlaying() const {
	if (_channel == -1) {
		return  false;
	}
	return _isActive;
}

bool AudioSpeech::playSpeechLine(int actorId, int sentenceId, int volume, int a4, int priority) {
	int balance = _vm->_actors[actorId]->soundBalance();
	Common::String name = Common::String::format("%02d-%04d%s.AUD", actorId, sentenceId, _vm->_languageCode.c_str());
	return _vm->_audioPlayer->playAud(name, _speechVolume * volume / 100, balance, balance, priority, kAudioPlayerOverrideVolume);
}

void AudioSpeech::setVolume(int volume) {
	_speechVolume = volume;
}

int AudioSpeech::getVolume() const {
	return _speechVolume;
}

void AudioSpeech::playSample() {
	_vm->_playerActor->speechPlay(kSpeechSamples[_vm->_rnd.getRandomNumber(22)], true);
}

} // End of namespace BladeRunner
