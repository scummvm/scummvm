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
#include "audio/decoders/raw.h"
#include "audio/decoders/voc.h"
#include "darkseed/sound.h"
#include "darkseed/darkseed.h"

namespace Darkseed {

Sound::Sound(Audio::Mixer *mixer) : _mixer(mixer) {
	_didSpeech.resize(978);
	resetSpeech();
}

void Sound::playTosSpeech(int tosIdx) {
	if (!g_engine->isCdVersion() || _didSpeech[tosIdx] == 1) {
		return;
	}
	Common::String filename = Common::String::format("%d.voc", tosIdx + 1);
	Common::Path path = Common::Path("speech").join(filename);
	Common::File f;
	if (!f.open(path)) {
		return;
	}
	Common::SeekableReadStream *srcStream = f.readStream((uint32)f.size());
	Audio::SeekableAudioStream *stream = Audio::makeVOCStream(srcStream,
															  Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
	_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_speechHandle, stream);
	_didSpeech[tosIdx] = 1;
}

bool Sound::isPlayingSpeech() {
	return _mixer->isSoundHandleActive(_speechHandle);
}

void Sound::waitForSpeech() {
	while (isPlayingSpeech()) {
		// TODO poll events / wait a bit here.
	}
}

void Sound::resetSpeech() {
	for (int i = 0; i < (int)_didSpeech.size(); i++) {
		_didSpeech[i] = 0;
	}
}

Common::Error Sound::sync(Common::Serializer &s) {
	s.syncArray(_didSpeech.data(), _didSpeech.size(), Common::Serializer::Byte);
	return Common::kNoError;
}

} // End of namespace Darkseed
