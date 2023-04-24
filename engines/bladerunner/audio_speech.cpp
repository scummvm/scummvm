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

#include "bladerunner/audio_speech.h"

#include "bladerunner/actor.h"
#include "bladerunner/aud_stream.h"
#include "bladerunner/audio_mixer.h"
#include "bladerunner/audio_player.h"
#include "bladerunner/bladerunner.h"

#include "common/str.h"

namespace BladeRunner {

// Note: Speech samples here should be from A.TLK file
const int kSpeechSamplesNumber = 23;
const int AudioSpeech::kSpeechSamples[kSpeechSamplesNumber] = { 65, 355, 490, 465, 480, 485, 505, 760, 7655, 7770, 7740, 8170, 2705, 7200, 6460, 5560, 4870, 4555, 3880, 3525, 3595, 3250, 3070 };

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
	// _speechVolumeFactorOriginalEngine here sets a percentage to be applied on the voice cues' volume
	// before sending them to the audio player.
	// This is how the original engine set the volume via the in-game KIA volume slider controls.
	// Setting _speechVolumeFactorOriginalEngine to 100, for the purposes ScummVM engine, renders it indifferent,
	// so sound volume can be controlled by ScummVM's Global Main Menu / ConfMan/ syncSoundSettings().
	_speechVolumeFactorOriginalEngine = BLADERUNNER_ORIGINAL_SETTINGS ? 50 : 100;
	_isActive = false;
	_data = new byte[kBufferSize];
	_channel = -1;
}

AudioSpeech::~AudioSpeech() {
	stopSpeech();
	while (isPlaying()) {
		// wait for the mixer to finish
	}

	delete[] _data;
}

// pan should be in [-100, 100]
bool AudioSpeech::playSpeech(const Common::String &name, int pan) {
	if (isPlaying()) {
		stopSpeech();
	}

	// Audio cache is not usable as hash function is producing collision for speech lines.
	// It was not used in the original game either

	Common::ScopedPtr<Common::SeekableReadStream> r(_vm->getResourceStream(_vm->_enhancedEdition ? ("audio/" + name) : name));

	if (!r) {
		warning("AudioSpeech::playSpeech: AUD resource \"%s\" not found", name.c_str());
		return false;
	}

	if (r->size() > kBufferSize) {
		warning("AudioSpeech::playSpeech: AUD larger than buffer size (%d > %d)", (int)r->size(), kBufferSize);
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

	AudStream *audioStream = new AudStream(_data, _vm->_shortyMode ? 33000 : -1);

	// Speech plays here with priority 100 (max)
	// Using directly _speechVolumeFactorOriginalEngine as the volume for audioMixer::play(),
	// which for the ScummVM engine is 100, so speech volume will be only determined
	// by the ScummVM volume for the speech sound type.
	_channel = _vm->_audioMixer->play(Audio::Mixer::kSpeechSoundType,
	                                  audioStream,
	                                  100,
	                                  false,
	                                  _speechVolumeFactorOriginalEngine,
	                                  pan,
	                                  mixerChannelEnded,
	                                  this,
	                                  audioStream->getLength());

	_isActive = true;

	return true;
}

void AudioSpeech::stopSpeech() {
	//Common::StackLock lock(_mutex);
	if (_channel != -1) {
		_vm->_audioMixer->stop(_channel, 0u);
	}
}

bool AudioSpeech::isPlaying() const {
	if (_channel == -1) {
		return false;
	}
	return _isActive;
}

// volume should be in [0, 100]
// priority should be in [0, 100]
// pan is calculated based on actor's position with Actor::soundPan()
bool AudioSpeech::playSpeechLine(int actorId, int sentenceId, int volume, int a4, int priority) {
	int pan = _vm->_actors[actorId]->soundPan();
	Common::String name = Common::String::format("%02d-%04d%s.AUD", actorId, sentenceId, _vm->_languageCode.c_str());
	return _vm->_audioPlayer->playAud(name, (volume * _speechVolumeFactorOriginalEngine) / 100, pan, pan, priority, kAudioPlayerOverrideVolume, Audio::Mixer::kSpeechSoundType);
}

#if BLADERUNNER_ORIGINAL_SETTINGS
// We no longer set the _speechVolumeFactorOriginalEngine via a public method.
// For the ScummVM Engine's purposes it is set in AudioSpeech::AudioSpeech() constructor and keeps its value constant.
void AudioSpeech::setVolume(int volume) {
	_speechVolumeFactorOriginalEngine = volume;
}

int AudioSpeech::getVolume() const {
	return _speechVolumeFactorOriginalEngine;
}
#endif // BLADERUNNER_ORIGINAL_SETTINGS

void AudioSpeech::playSample() {
#if BLADERUNNER_ORIGINAL_BUGS
	_vm->_playerActor->speechPlay(kSpeechSamples[_vm->_rnd.getRandomNumber(kSpeechSamplesNumber-1)], true);
#else
	if (_vm->openArchive("A.TLK")) {
		// load sample speech even when in initial KIA screen (upon launch - but before loading a game)
		_vm->_playerActor->speechPlay(kSpeechSamples[_vm->_rnd.getRandomNumber(kSpeechSamplesNumber-1)], true);
	}
#endif // BLADERUNNER_ORIGINAL_BUGS
}

} // End of namespace BladeRunner
