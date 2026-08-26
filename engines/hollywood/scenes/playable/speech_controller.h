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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SPEECH_CONTROLLER_H
#define HOLLYWOOD_SCENES_PLAYABLE_SPEECH_CONTROLLER_H

#include "common/random.h"
#include "common/types.h"

#include "hollywood/music.h"
#include "hollywood/scenes/playable/speech_overlay.h"

namespace Hollywood {

// Owns speech playback, subtitles, and actor speech-animation state.
class SpeechController {
public:
	SpeechController();

	void initialize(byte secondaryTextColor, byte primaryTextColor);
	void resetRuntimeState(byte invalidPrimaryGroup, byte defaultPrimaryFrame);
	void clearSecondaryOverlay();
	void clearPrimaryOverlay();
	void clearAllOverlays();
	void prepareSecondaryActorSpeech();
	void advanceSecondaryActorSpeechAnimation(uint32 delta, Common::RandomSource &random,
		uint32 frameMillis, byte frameCount);
	void advanceSecondaryActorSpeechFrame(Common::RandomSource &random, byte frameCount);
	byte advancePrimaryLeftSpeechFrame(Common::RandomSource &random);
	byte advancePrimaryDialogueSpeechFrame(Common::RandomSource &random, byte baseFrame, byte frameCount);
	void startPrimaryLeftSpeech();
	void stopPrimaryLeftSpeech();
	void startPrimaryDialogueSpeech(byte animationGroup, byte baseFrame);
	void stopPrimaryDialogueSpeech(byte invalidPrimaryGroup, byte defaultPrimaryFrame);

	SpeechPlayer player;
	SpeechOverlay secondaryOverlay;
	SpeechOverlay primaryOverlay;
	byte primaryLeftSpeechLastFrame;
	byte primaryDialogueSpeechLastFrame;
	byte primaryDialogueSpeechGroup;
	bool primaryLeftSpeechActive;
	bool primaryDialogueSpeechActive;
	uint32 secondaryActorTimerAccumulator;
	uint32 primaryLeftSpeechTimerAccumulator;
	uint32 primaryDialogueSpeechTimerAccumulator;
	byte secondaryActorFrame;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SPEECH_CONTROLLER_H
