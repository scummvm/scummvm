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
#include "hollywood/scenes/speech_overlay.h"

namespace Hollywood {

/**
 * Owns speech playback, subtitle overlays, and actor speech-animation state.
 *
 * PlayableScene supplies elapsed time and draws the result; this helper owns
 * and mutates the playback state but never presents a frame.
 */
class SpeechController {
public:
	SpeechController(Common::Language language, bool speechEnabled = true);

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

	SpeechPlayer _player;
	SpeechOverlay _secondaryOverlay;
	SpeechOverlay _primaryOverlay;
	byte _primaryLeftSpeechLastFrame;
	byte _primaryDialogueSpeechLastFrame;
	byte _primaryDialogueSpeechGroup;
	bool _primaryLeftSpeechActive;
	bool _primaryDialogueSpeechActive;
	uint32 _secondaryActorTimerAccumulator;
	uint32 _primaryLeftSpeechTimerAccumulator;
	uint32 _primaryDialogueSpeechTimerAccumulator;
	byte _secondaryActorFrame;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SPEECH_CONTROLLER_H
