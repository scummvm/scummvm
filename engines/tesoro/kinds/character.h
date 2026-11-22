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

#ifndef TESORO_KINDS_CHARACTER_H
#define TESORO_KINDS_CHARACTER_H

#include "tesoro/core/camera.h"
#include "tesoro/core/speaker.h"
#include "tesoro/kinds/interactive.h"

namespace Tesoro {

class World;

class Character;

/**
 * A person of a level (kind 5): a clickable object that also speaks.
 * Facing is 1 up, 2 down, 3 left, 4 right.
 * */
class Character : public InteractiveObject {
public:
	enum { kMaxPoses = 4 };

	void init(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now, World &world, uint16 stateSize);

	void say(const SpeechTable::Line &line, int32 colour, uint32 pose) { _speaker.say(line, colour, pose); }

	void clearSpeech() { _speaker.clearSpeech(); }

	void face(int facing);
	/** Puts the character in the speaking pose and works out where its subtitle goes. */
	virtual void prepareToSpeak(uint32 pose);
	/** A walk order only marks that it was asked. */
	void startWalk() { _walkAsked = true; }

	int update() override { return _speaker.update(); }
	virtual void speechCleared() {}
	void draw(const Camera &camera, Display &display) override;

	Speaker &speaker() { return _speaker; }
	uint16 textColour() const { return _textColour; }
	uint16 textWidth() const { return _textWidth; }
	int32 anchorX() const { return _anchorX; }
	int32 anchorY() const { return _anchorY; }

private:
	/** What the speaking rules ask of the character. A separate class, so its methods do not clash with InteractiveObject's. */
	class SpeakingAdapter : public SpeakingObject {
	public:
		explicit SpeakingAdapter(Character &character) : _ch(character) {}
		int32 now() override;
		uint32 randomNumber(uint32 lo, uint32 hi) override;
		uint32 playVoice(uint32 archiveOffset) override;
		bool voiceActive(uint32 id) override;
		void stopVoice(uint32 id) override;
		bool skipPressed() override;
		bool subtitlesOn() override;
		void insertInMainList(int priority) override;
		void removeFromMainList() override;
		void pauseAnimation() override;
		void stashAnimation() override;
		void prepareToSpeak(uint32 pose) override;
		void queueFrame() override;
		void setFrameRange(uint16 first, int32 last) override;
		uint32 frameCount() override;
		bool animationHeld() override;
		void setAnimationHeld(bool held) override;
		bool pending() override;
		void setPending(bool pending) override;
		int objectUpdate() override;
		int interactiveUpdate() override;
		void interactiveDraw() override;
		void drawSubtitle(const SpeechTable::Line &line) override;

	private:
		Character &_ch;
	};

	/** The current frame of the speaking pose goes into the picture, unscaled. */
	void queueFrame();
	/** The line's text: Jim's go in the middle of the screen, a character's above its head. */
	void drawSubtitle(const SpeechTable::Line &line);

	SpeakingAdapter _adapter{*this};
	Speaker _speaker{_adapter};
	uint16 _textColour = 0, _textWidth = 0;
	int _faceBank = 0;
	byte _poses[kMaxPoses] = {};
	uint _poseCount = 0;
	bool _walkAsked = false;
	int32 _anchorX = 0, _anchorY = 0;
	const Camera *_camera = nullptr;
	Display *_display = nullptr;
};

} // End of namespace Tesoro

#endif
