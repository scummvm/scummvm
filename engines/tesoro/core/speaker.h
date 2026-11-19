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

#ifndef TESORO_CORE_SPEAKER_H
#define TESORO_CORE_SPEAKER_H

#include "common/scummsys.h"
#include "tesoro/core/speechtable.h"

namespace Tesoro {

/** What a speaking object asks of the rest of the game. */
class SpeakingObject {
public:
	virtual ~SpeakingObject() {}

	virtual int32 now() = 0;

	virtual uint32 randomNumber(uint32 lo, uint32 hi) = 0;


	virtual uint32 playVoice(uint32 archiveOffset) = 0;
	virtual bool voiceActive(uint32 id) = 0;
	virtual void stopVoice(uint32 id) = 0;

	virtual bool skipPressed() = 0;
	virtual bool subtitlesOn() = 0;

	virtual void insertInMainList(int priority) = 0;
	virtual void removeFromMainList() = 0;

	virtual void pauseAnimation() = 0;
	virtual void stashAnimation() = 0;

	virtual void prepareToSpeak(uint32 pose) = 0;
	virtual void queueFrame() = 0;
	virtual void setFrameRange(uint16 first, int32 last) = 0;
	virtual uint32 frameCount() = 0;

	/** The two flags a speaker shares with the rest of the object: its animation is held for the speaking pose, and a byte that is set while a walk is on (Jim) or by the walk stub (characters). */
	virtual bool animationHeld() = 0;
	virtual void setAnimationHeld(bool held) = 0;
	virtual bool pending() = 0;
	virtual void setPending(bool pending) = 0;

	virtual int objectUpdate() = 0;

	/** What the object does when it is not speaking. */
	virtual int interactiveUpdate() = 0;
	virtual void interactiveDraw() = 0;

	virtual void drawSubtitle(const SpeechTable::Line &line) = 0;
};

/**
 * The speaking part of a character: the kind 5 objects of the levels and Jim.
 */
class Speaker {
public:
	explicit Speaker(SpeakingObject &object) : _object(object) {}

	void say(const SpeechTable::Line &line, int32 colour, uint32 pose);

	int update();

	/** The line is cleared. */
	void forgetLine() { _line = nullptr; }

	/** Ends the line at once. */
	void clearSpeech();

	/** The frame of a character that talks: its mouth moves, about every tenth of a second. */
	void talkAnimate(bool talking);

	void drawCharacter();
	void drawJim();

	bool speaking() const { return _speaking; }
	const SpeechTable::Line *line() const { return _line; }
	int32 colour() const { return _colour; }

private:
	SpeakingObject &_object;
	const SpeechTable::Line *_line = nullptr;
	uint32 _voiceId = 0;
	int32 _colour = 0;
	bool _speaking = false;
	int32 _talkTime = 0;
};

} // End of namespace Tesoro

#endif
