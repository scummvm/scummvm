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

#include "tesoro/core/speaker.h"

namespace Tesoro {

static const int kSpeakerPriority = 6;
static const int32 kMouthInterval = 100;

void Speaker::say(const SpeechTable::Line &line, int32 colour, uint32 pose) {
	_colour = colour;
	_line = &line;
	_speaking = true;
	if (!_object.animationHeld()) {
		_object.setAnimationHeld(true);
		_object.pauseAnimation();
		_object.stashAnimation();
	}
	_voiceId = line.wavOffset ? _object.playVoice(line.wavOffset) : 0;
	_talkTime = _object.now();
	_object.prepareToSpeak(pose);
	_object.insertInMainList(kSpeakerPriority);
	_object.objectUpdate();
}

int Speaker::update() {
	if (!_speaking) {
		if (!_object.pending())
			return _object.interactiveUpdate();
		_object.setPending(false);
		return 0;
	}

	bool done = false;
	if (!_object.skipPressed()) {
		if (_voiceId)
			done = !_object.voiceActive(_voiceId);
	} else {
		done = true;
		if (_voiceId && _object.voiceActive(_voiceId))
			_object.stopVoice(_voiceId);
	}

	if (_line && !done) {
		if (_object.subtitlesOn())
			_object.drawSubtitle(*_line);
		return 1;
	}
	clearSpeech();
	return 3;
}

void Speaker::clearSpeech() {
	_line = nullptr;
	_speaking = false;
	_object.removeFromMainList();
}

void Speaker::talkAnimate(bool talking) {
	_object.queueFrame();
	if (!talking) {
		_object.setFrameRange(1, 1);
		return;
	}
	const int32 t = _object.now();
	if ((uint32)(t - _talkTime) > (uint32)kMouthInterval) {
		const uint32 frame = _object.randomNumber(1, (uint16)(_object.frameCount() - 1));
		_object.setFrameRange((uint16)frame, (int32)frame);
		_talkTime = t;
	}
}

void Speaker::drawCharacter() {
	if (_speaking) {
		talkAnimate(true);
	} else if (_object.animationHeld() && !_object.pending()) {
		talkAnimate(false);
	}
	_object.interactiveDraw();
}

void Speaker::drawJim() {
	if (!_speaking) {
		if (_object.animationHeld() && !_object.pending())
			talkAnimate(false);
	} else {
		talkAnimate(_colour > 0);
	}
	_object.interactiveDraw();
}

} // End of namespace Tesoro
