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

SpeechController::SpeechController() :
		primaryLeftSpeechLastFrame(0),
		primaryDialogueSpeechLastFrame(0),
		primaryDialogueSpeechGroup(0),
		primaryLeftSpeechActive(false),
		primaryDialogueSpeechActive(false),
		secondaryActorTimerAccumulator(0),
		primaryLeftSpeechTimerAccumulator(0),
		primaryDialogueSpeechTimerAccumulator(0),
		secondaryActorFrame(0) {
	initialize(0, 0);
}

void SpeechController::initialize(byte secondaryTextColor, byte primaryTextColor) {
	secondaryOverlay.visible = false;
	secondaryOverlay.colorIndex = secondaryTextColor;
	secondaryOverlay.centerX = 0;
	secondaryOverlay.topY = 0;
	secondaryOverlay.lines.clear();
	primaryOverlay.visible = false;
	primaryOverlay.colorIndex = primaryTextColor;
	primaryOverlay.centerX = 0;
	primaryOverlay.topY = 0;
	primaryOverlay.lines.clear();
}

void SpeechController::resetRuntimeState(byte invalidPrimaryGroup, byte defaultPrimaryFrame) {
	primaryLeftSpeechLastFrame = 0;
	primaryDialogueSpeechLastFrame = defaultPrimaryFrame;
	primaryDialogueSpeechGroup = invalidPrimaryGroup;
	primaryLeftSpeechActive = false;
	primaryDialogueSpeechActive = false;
	secondaryActorTimerAccumulator = 0;
	primaryLeftSpeechTimerAccumulator = 0;
	primaryDialogueSpeechTimerAccumulator = 0;
	secondaryActorFrame = 0;
	clearAllOverlays();
}

void SpeechController::clearSecondaryOverlay() {
	secondaryOverlay.visible = false;
	secondaryOverlay.lines.clear();
}

void SpeechController::clearPrimaryOverlay() {
	primaryOverlay.visible = false;
	primaryOverlay.lines.clear();
}

void SpeechController::clearAllOverlays() {
	clearSecondaryOverlay();
	clearPrimaryOverlay();
}

void SpeechController::prepareSecondaryActorSpeech() {
	secondaryActorFrame = 0;
	secondaryActorTimerAccumulator = 0;
}

void SpeechController::advanceSecondaryActorSpeechAnimation(uint32 delta, Common::RandomSource &random,
		uint32 frameMillis, byte frameCount) {
	if (!secondaryOverlay.visible) {
		prepareSecondaryActorSpeech();
		return;
	}

	secondaryActorTimerAccumulator += delta;
	while (secondaryActorTimerAccumulator >= frameMillis) {
		secondaryActorTimerAccumulator -= frameMillis;
		advanceSecondaryActorSpeechFrame(random, frameCount);
	}
}

void SpeechController::advanceSecondaryActorSpeechFrame(Common::RandomSource &random, byte frameCount) {
	if (frameCount == 0)
		return;

	byte nextFrame = secondaryActorFrame;
	for (uint attempt = 0; attempt < 8 && nextFrame == secondaryActorFrame; ++attempt)
		nextFrame = (byte)random.getRandomNumber(frameCount - 1);

	if (nextFrame == secondaryActorFrame)
		nextFrame = (byte)((secondaryActorFrame + 1) % frameCount);

	secondaryActorFrame = nextFrame;
}

byte SpeechController::advancePrimaryLeftSpeechFrame(Common::RandomSource &random) {
	byte nextFrame = primaryLeftSpeechLastFrame;
	for (uint attempt = 0; attempt < 8 && nextFrame == primaryLeftSpeechLastFrame; ++attempt)
		nextFrame = (byte)random.getRandomNumber(3);

	if (nextFrame == primaryLeftSpeechLastFrame)
		nextFrame = (byte)((primaryLeftSpeechLastFrame + 1) % 4);

	primaryLeftSpeechLastFrame = nextFrame;
	return nextFrame;
}

byte SpeechController::advancePrimaryDialogueSpeechFrame(Common::RandomSource &random, byte baseFrame,
		byte frameCount) {
	if (frameCount == 0)
		return baseFrame;

	const byte lastFrame = baseFrame + frameCount - 1;
	byte nextFrame = primaryDialogueSpeechLastFrame;
	for (uint attempt = 0; attempt < 8 && nextFrame == primaryDialogueSpeechLastFrame; ++attempt)
		nextFrame = (byte)(baseFrame + random.getRandomNumber(frameCount - 1));

	if (nextFrame == primaryDialogueSpeechLastFrame)
		nextFrame = nextFrame >= lastFrame ? baseFrame : (byte)(nextFrame + 1);

	primaryDialogueSpeechLastFrame = nextFrame;
	return nextFrame;
}

void SpeechController::startPrimaryLeftSpeech() {
	primaryLeftSpeechActive = true;
}

void SpeechController::stopPrimaryLeftSpeech() {
	primaryLeftSpeechActive = false;
	primaryLeftSpeechTimerAccumulator = 0;
}

void SpeechController::startPrimaryDialogueSpeech(byte animationGroup, byte baseFrame) {
	primaryDialogueSpeechActive = true;
	primaryDialogueSpeechGroup = animationGroup;
	primaryDialogueSpeechLastFrame = baseFrame;
	primaryDialogueSpeechTimerAccumulator = 0;
}

void SpeechController::stopPrimaryDialogueSpeech(byte invalidPrimaryGroup, byte defaultPrimaryFrame) {
	primaryDialogueSpeechActive = false;
	primaryDialogueSpeechGroup = invalidPrimaryGroup;
	primaryDialogueSpeechTimerAccumulator = 0;
	primaryDialogueSpeechLastFrame = defaultPrimaryFrame;
}

} // End of namespace Hollywood
