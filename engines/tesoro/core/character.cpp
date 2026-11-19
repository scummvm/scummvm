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

#include "tesoro/core/character.h"
#include "tesoro/world.h"

#include "common/textconsole.h"
#include "tesoro/core/animbank.h"
#include "tesoro/core/camera.h"
#include "tesoro/core/room.h"
#include "tesoro/core/display.h"

namespace Tesoro {

static uint32 tag(const char *three) {
	return (byte)three[0] | ((byte)three[1] << 8) | ((byte)three[2] << 16);
}

void Character::init(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now, World &host, uint16 stateSize) {
	_world = &host;
	// The text colour and width come first, the clickable object's data follows.
	InteractiveObject::init(record, roomSortOffset, levelPlayers, now, host, stateSize, 4);
	_textColour = record.dataByte(0) | (record.dataByte(1) << 8);
	_textWidth = record.dataByte(2) | (record.dataByte(3) << 8);

	_faceBank = 0;
	_poseCount = 0;
	for (uint i = 0; i < _anim.bankCount(); ++i) {
		const uint32 name = _anim.player(i).bank()->headerDword(0) & 0xffffff;
		if (name == tag("MIR")) {
			_faceBank = i;
		} else if (name == tag("HAB")) {
			if (_poseCount < kMaxPoses)
				_poses[_poseCount++] = i;
			else
				warning("Tesoro: a character with more than %d speaking animations", kMaxPoses);
		}
	}
	_walkAsked = false;
	_flags |= kVisible;
}

void Character::face(int facing) {
	_anim.selectBank(_faceBank, facing - 1, facing - 1, false, false, _world->now());
}

void Character::prepareToSpeak(uint32 pose) {
	const int32 cx = _x + 320;
	const int32 cy = 240 - _y;
	AnimPlayer &player = _anim.current();
	const AnimStep &seq = player.bank()->step(player.current());
	if (!seq.hasImage())
		_anchorX = seq.hitW / 2 + cx + seq.hitX;
	else
		_anchorX = seq.imageX + cx + (READ_LE_UINT16(player.bank()->image(seq.imageIndex).data() + 8) >> 1);
	_anchorY = (cy - seq.hitY) - 50;

	if (_poseCount == 0)
		return;
	if (pose >= kMaxPoses) {
		warning("Tesoro: a character asked for speaking animation %u", pose);
		return;
	}
	_anim.switchTo(_poses[pose]);
	_anim.current().setRange(1, 1, false, false, _world->now());
}

void Character::draw(const Camera &camera, Display &display) {
	_camera = &camera;
	_display = &display;
	_speaker.drawCharacter();
}

void Character::queueFrame() {
	AnimPlayer &player = _anim.current();
	const AnimStep &seq = player.bank()->step(0);
	if (!seq.hasImage())
		return;
	const int32 x = (int32)((uint32)(_x + player.movementX() - _camera->scaledX()) + 320 + (uint32)(int32)seq.imageX);
	const int32 y = (int32)((uint32)(_camera->scaledY() - _y - seq.imageY - player.movementY()) + 240);
	_display->queueImage(x, y, (uint16)(_sortValue + 1), player.bank()->image(seq.imageIndex), 0);
}

void Character::drawSubtitle(const SpeechTable::Line &line) {
	const byte *text = (const byte *)line.str();
	const int32 colour = _speaker.colour();
	if (colour < 1) {
		_world->queueText(text, (uint16)-(int16)colour, 320, 240, 640);
		return;
	}
	const Camera &camera = _world->updateCamera(_room->cameraDivisor());
	_world->queueText(text, (uint16)colour, _anchorX - camera.scaledX(), _anchorY + camera.scaledY(), _textWidth);
}

int32 Character::SpeakingAdapter::now() { return _ch._world->now(); }
uint32 Character::SpeakingAdapter::randomNumber(uint32 lo, uint32 hi) { return _ch._world->randomNumber(lo, hi); }
uint32 Character::SpeakingAdapter::playVoice(uint32 archiveOffset) { return _ch._world->playVoice(archiveOffset); }
bool Character::SpeakingAdapter::voiceActive(uint32 id) { return _ch._world->voiceActive(id); }
void Character::SpeakingAdapter::stopVoice(uint32 id) { _ch._world->stopVoice(id); }
bool Character::SpeakingAdapter::skipPressed() { return _ch._world->skipPressed(); }
bool Character::SpeakingAdapter::subtitlesOn() { return _ch._world->subtitlesOn(); }
void Character::SpeakingAdapter::insertInMainList(int priority) { _ch._world->insertInMainList(&_ch, priority); }
void Character::SpeakingAdapter::removeFromMainList() {
	_ch._world->removeFromMainList(&_ch);
	_ch.speechCleared();
}
void Character::SpeakingAdapter::pauseAnimation() { _ch._anim.current().pause(_ch._world->now()); }
void Character::SpeakingAdapter::stashAnimation() { _ch._anim.stash(); }
void Character::SpeakingAdapter::prepareToSpeak(uint32 pose) { _ch.prepareToSpeak(pose); }
void Character::SpeakingAdapter::queueFrame() { _ch.queueFrame(); }
void Character::SpeakingAdapter::setFrameRange(uint16 first, int32 last) { _ch._anim.current().setRange(first, last, false, false, _ch._world->now()); }
uint32 Character::SpeakingAdapter::frameCount() { return _ch._anim.current().bank()->stepCount(); }
bool Character::SpeakingAdapter::animationHeld() { return _ch._anim.isStashed(); }
void Character::SpeakingAdapter::setAnimationHeld(bool held) { _ch._anim.setStashed(held); }
bool Character::SpeakingAdapter::pending() { return _ch._walkAsked; }
void Character::SpeakingAdapter::setPending(bool pending) { _ch._walkAsked = pending; }
int Character::SpeakingAdapter::objectUpdate() { return _ch.update(); }
int Character::SpeakingAdapter::interactiveUpdate() { return _ch.InteractiveObject::update(); }
void Character::SpeakingAdapter::interactiveDraw() { _ch.InteractiveObject::draw(*_ch._camera, *_ch._display); }
void Character::SpeakingAdapter::drawSubtitle(const SpeechTable::Line &line) { _ch.drawSubtitle(line); }

} // End of namespace Tesoro
