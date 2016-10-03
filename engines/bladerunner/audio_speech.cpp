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
#include "bladerunner/bladerunner.h"

#include "common/debug.h"

namespace BladeRunner {

#define BUFFER_SIZE 200000

AudioSpeech::AudioSpeech(BladeRunnerEngine *vm) : _vm(vm) {
	_volume = 50;
	_isMaybeActive = false;
	_data = new byte[BUFFER_SIZE];
}

AudioSpeech::~AudioSpeech() {
	delete[] _data;
}

bool AudioSpeech::playSpeech(const char *name, int balance) {
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

	_vm->_mixer->playStream(
		Audio::Mixer::kPlainSoundType,
		&_soundHandle,
		audioStream,
		-1,
		_volume * 255 / 100,
		balance);

	_isMaybeActive = true;

	return true;
}

void AudioSpeech::stopSpeech() {
	_vm->_mixer->stopHandle(_soundHandle);
}

bool AudioSpeech::isPlaying() {
	if (!_isMaybeActive)
		return false;

	return _isMaybeActive = _vm->_mixer->isSoundHandleActive(_soundHandle);
}

} // End of namespace BladeRunner
