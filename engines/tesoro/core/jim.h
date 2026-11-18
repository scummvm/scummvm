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

#ifndef TESORO_CORE_JIM_H
#define TESORO_CORE_JIM_H

#include "common/array.h"
#include "tesoro/core/object.h"
#include "tesoro/core/speaker.h"
#include "tesoro/core/speechtable.h"
#include "tesoro/core/walkarea.h"

namespace Tesoro {

class World;

/** Jim, the player's character: an object with nine animations (four speaking, four walking, the standing frames). Facing is 1 up, 2 down, 3 left, 4 right. */
class Jim : public Object, private SpeakingObject {
public:
	enum { kBanks = 9 };
	// The order of the banks, as the files are loaded.
	enum Bank { kSpeakLeft, kSpeakRight, kStanding, kWalkUp, kWalkDown, kWalkLeft, kWalkRight, kSpeakDown, kSpeakUp };

	explicit Jim(World &world) : _world(world), _speaker(*this) {}

	void init(const Common::Array<AnimPlayer> &levelPlayers, const SpeechTable::Line *name);

	/** The narration, in Jim's voice: his mouth stays shut and the subtitle is centred on the screen. */
	void sayNarrationLine(const SpeechTable::Line &line) { _speaker.say(line, -(int32)_scriptColour, 0); }
	/** A verb answer: Jim's mouth moves and the subtitle goes above his head. */
	void sayAnswer(const SpeechTable::Line &line) { _speaker.say(line, _answerColour, 0); }

	Speaker &speaker() { return _speaker; }

	/** Walks to (x, y). direct: straight there, not round the waypoints. target is told on arrival. */
	int walkTo(int32 x, int32 y, WalkTarget *target, bool direct);

	void setFacing(int facing);

	bool walking() const { return _walking; }
	void stopWalking() { _walking = false; }
	bool direct() const { return _direct; }
	int facing() const override { return _facing; }
	WalkTarget *target() const { return _target; }
	int32 destX() const { return _destX; }
	int32 destY() const { return _destY; }

	/** Moves Jim into a room of a level and puts him at (x, y), the camera following him. */
	void enterLevel(Level *level, Room *room, int32 x, int32 y);

	/** Takes Jim out of his level, for when the level is thrown away. */
	void leaveLevel();

	int32 scale() const { return _scale; }

	int update() override;

	bool acceptsPointer() override { return true; }
	int showHoverText() override;
	int clearHoverText() override;

	void updateAnimation(int32 now) override { _anim.tickClearMovement(now); }
	void updateBounds(int32 now) override;
	void draw(const Camera &camera, Display &display) override;
	bool isHitter(const Object *other) const override { return other->kindId() == 7; }

	/** Jim touched the level's walk area edge: a straight walk is replaced by a route round the waypoints. */
	int onTouched(Object *other, int extra) override;

	int32 anchorX() const { return _anchorX; }
	int32 anchorY() const { return _anchorY; }
	uint16 textWidth() const { return _textWidth; }

	const SpeechTable::Line *name() const { return _name; }

private:
	void stopAtCurrentPosition() { _previousX = _x; _previousY = _y; }
	bool findPath(int32 destX, int32 destY, WalkTarget *target);
	bool startWalkAnimation(int facing);
	void advanceAnimation(int16 &moveX, int16 &moveY);
	void tickAnimation();
	void showFacing(int facing);

	// SpeakingObject
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
	void stashAnimation() override { _anim.stash(); }
	void setPose(uint32 pose) override;
	void queueFrame() override;
	void setFrameRange(uint16 first, int32 last) override;
	uint32 frameCount() override { return _anim.current().bank()->stepCount(); }
	bool animationHeld() override { return _anim.isStashed(); }
	void setAnimationHeld(bool held) override { _anim.setStashed(held); }
	bool pending() override { return _walking; }
	void setPending(bool pending) override { if (!pending) _walking = false; }
	int objectUpdate() override { return update(); }
	int interactiveUpdate() override;
	void interactiveDraw() override;
	void drawSubtitle(const SpeechTable::Line &line) override;

	void placeSpeech(uint pose);
	void drawScaled(int32 x, int32 y, uint16 depth, int imageIndex);

	World &_world;
	Speaker _speaker;

	bool _walking = false;
	bool _direct = false;
	WalkTarget *_target = nullptr;
	int32 _fixedX = 0, _fixedY = 0;	// the position in 24.8 fixed point
	int32 _moveX = 0, _moveY = 0;	// the animation's movement already consumed
	int32 _destX = 0, _destY = 0;
	int32 _scale = 0x100;	// 0x100 is unscaled
	int _facing = 3;

	const SpeechTable::Line *_name = nullptr;
	uint16 _scriptColour = 0x1f;
	uint16 _answerColour = 0xffc0;
	uint16 _textWidth = 300;
	int32 _anchorX = 0, _anchorY = 0;
	byte _poses[4] = { kSpeakLeft, kSpeakRight, kSpeakDown, kSpeakUp };
	const Camera *_camera = nullptr;
	Display *_display = nullptr;
};

} // End of namespace Tesoro

#endif
