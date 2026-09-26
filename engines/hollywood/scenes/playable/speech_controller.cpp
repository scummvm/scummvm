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

#include "hollywood/scenes/playable/speech_controller.h"

namespace Hollywood {

SpeechController::SpeechController(Common::Language language, bool speechEnabled) :
		_player(language, speechEnabled),
		_primaryLeftSpeechLastFrame(0),
		_primaryDialogueSpeechLastFrame(0),
		_primaryDialogueSpeechGroup(0),
		_primaryLeftSpeechActive(false),
		_primaryDialogueSpeechActive(false),
		_secondaryActorTimerAccumulator(0),
		_primaryLeftSpeechTimerAccumulator(0),
		_primaryDialogueSpeechTimerAccumulator(0),
		_secondaryActorFrame(0) {
	initialize(0, 0);
}

void SpeechController::initialize(byte secondaryTextColor, byte primaryTextColor) {
	_secondaryOverlay.visible = false;
	_secondaryOverlay.colorIndex = secondaryTextColor;
	_secondaryOverlay.centerX = 0;
	_secondaryOverlay.topY = 0;
	_secondaryOverlay.lines.clear();
	_primaryOverlay.visible = false;
	_primaryOverlay.colorIndex = primaryTextColor;
	_primaryOverlay.centerX = 0;
	_primaryOverlay.topY = 0;
	_primaryOverlay.lines.clear();
}

void SpeechController::resetRuntimeState(byte invalidPrimaryGroup, byte defaultPrimaryFrame) {
	_primaryLeftSpeechLastFrame = 0;
	_primaryDialogueSpeechLastFrame = defaultPrimaryFrame;
	_primaryDialogueSpeechGroup = invalidPrimaryGroup;
	_primaryLeftSpeechActive = false;
	_primaryDialogueSpeechActive = false;
	_secondaryActorTimerAccumulator = 0;
	_primaryLeftSpeechTimerAccumulator = 0;
	_primaryDialogueSpeechTimerAccumulator = 0;
	_secondaryActorFrame = 0;
	clearAllOverlays();
}

void SpeechController::clearSecondaryOverlay() {
	_secondaryOverlay.visible = false;
	_secondaryOverlay.lines.clear();
}

void SpeechController::clearPrimaryOverlay() {
	_primaryOverlay.visible = false;
	_primaryOverlay.lines.clear();
}

void SpeechController::clearAllOverlays() {
	clearSecondaryOverlay();
	clearPrimaryOverlay();
}

void SpeechController::prepareSecondaryActorSpeech() {
	_secondaryActorFrame = 0;
	_secondaryActorTimerAccumulator = 0;
}

void SpeechController::advanceSecondaryActorSpeechAnimation(uint32 delta, Common::RandomSource &random,
		uint32 frameMillis, byte frameCount) {
	if (!_secondaryOverlay.visible) {
		prepareSecondaryActorSpeech();
		return;
	}

	_secondaryActorTimerAccumulator += delta;
	while (_secondaryActorTimerAccumulator >= frameMillis) {
		_secondaryActorTimerAccumulator -= frameMillis;
		advanceSecondaryActorSpeechFrame(random, frameCount);
	}
}

void SpeechController::advanceSecondaryActorSpeechFrame(Common::RandomSource &random, byte frameCount) {
	if (frameCount == 0)
		return;

	byte nextFrame = _secondaryActorFrame;
	for (uint attempt = 0; attempt < 8 && nextFrame == _secondaryActorFrame; ++attempt)
		nextFrame = (byte)random.getRandomNumber(frameCount - 1);

	if (nextFrame == _secondaryActorFrame)
		nextFrame = (byte)((_secondaryActorFrame + 1) % frameCount);

	_secondaryActorFrame = nextFrame;
}

byte SpeechController::advancePrimaryLeftSpeechFrame(Common::RandomSource &random) {
	byte nextFrame = _primaryLeftSpeechLastFrame;
	for (uint attempt = 0; attempt < 8 && nextFrame == _primaryLeftSpeechLastFrame; ++attempt)
		nextFrame = (byte)random.getRandomNumber(3);

	if (nextFrame == _primaryLeftSpeechLastFrame)
		nextFrame = (byte)((_primaryLeftSpeechLastFrame + 1) % 4);

	_primaryLeftSpeechLastFrame = nextFrame;
	return nextFrame;
}

byte SpeechController::advancePrimaryDialogueSpeechFrame(Common::RandomSource &random, byte baseFrame,
		byte frameCount) {
	if (frameCount == 0)
		return baseFrame;

	const byte lastFrame = baseFrame + frameCount - 1;
	byte nextFrame = _primaryDialogueSpeechLastFrame;
	for (uint attempt = 0; attempt < 8 && nextFrame == _primaryDialogueSpeechLastFrame; ++attempt)
		nextFrame = (byte)(baseFrame + random.getRandomNumber(frameCount - 1));

	if (nextFrame == _primaryDialogueSpeechLastFrame)
		nextFrame = nextFrame >= lastFrame ? baseFrame : (byte)(nextFrame + 1);

	_primaryDialogueSpeechLastFrame = nextFrame;
	return nextFrame;
}

void SpeechController::startPrimaryLeftSpeech() {
	_primaryLeftSpeechActive = true;
}

void SpeechController::stopPrimaryLeftSpeech() {
	_primaryLeftSpeechActive = false;
	_primaryLeftSpeechTimerAccumulator = 0;
}

void SpeechController::startPrimaryDialogueSpeech(byte animationGroup, byte baseFrame) {
	_primaryDialogueSpeechActive = true;
	_primaryDialogueSpeechGroup = animationGroup;
	_primaryDialogueSpeechLastFrame = baseFrame;
	_primaryDialogueSpeechTimerAccumulator = 0;
}

void SpeechController::stopPrimaryDialogueSpeech(byte invalidPrimaryGroup, byte defaultPrimaryFrame) {
	_primaryDialogueSpeechActive = false;
	_primaryDialogueSpeechGroup = invalidPrimaryGroup;
	_primaryDialogueSpeechTimerAccumulator = 0;
	_primaryDialogueSpeechLastFrame = defaultPrimaryFrame;
}

} // End of namespace Hollywood
