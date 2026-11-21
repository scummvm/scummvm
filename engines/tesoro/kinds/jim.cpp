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

#include "tesoro/kinds/jim.h"

#include "tesoro/kinds/cameraframe.h"
#include "tesoro/core/display.h"
#include "tesoro/core/level.h"
#include "tesoro/core/namekey.h"
#include "tesoro/world.h"

namespace Tesoro {

static const int kLevelPriority = 20;
static const int kWalkPriority = 5;
static const int kMapLevel = 8;
static const int kScreenMiddleX = 320;
static const int kScreenMiddleY = 240;
static const int kSubtitleGap = 50;	// above the top of his hit rectangle

int Jim::walkTo(int32 x, int32 y, WalkTarget *target, bool direct) {
	_world.removeFromMainList(this);
	if (_world.hasWalkArea()) {
		_world.insertInMainListFront(this, kWalkPriority);
		_walking = true;
		_fixedX = _x * 256;
		_fixedY = _y * 256;
		_destX = x;
		_destY = y;
		_direct = direct;
		_target = target;

		const int32 dx = _destX - _x;
		const int32 dy = _destY - _y;
		if (ABS(dy) < ABS(dx))
			_facing = (dx >= 0) + 3;
		else
			_facing = (dy < 0) + 1;

		if (startWalkAnimation(_facing))
			_moveX = _moveY = 0;
		return 1;
	}
	if (target)
		return target->arrived();
	return 0;
}

int Jim::onTouched(Object *other, int) {
	const bool otherHasWalkArea = _world.hasWalkAreaOf(other);
	if (!_walking)
		return 1;
	stopAtCurrentPosition();
	removeFromMainList();
	if (!_direct && otherHasWalkArea) {
		if (!_world.findPath(_destX, _destY, _target)) {
			setFacing(_facing);
			if (_target) {
				_target->arrived();
				return 1;
			}
		}
	} else {
		_direct = false;
		setFacing(_facing);
	}
	return 1;
}

void Jim::setFacing(int facing) {
	_walking = false;
	_facing = facing;
	showFacing(facing);
}

int Jim::update() {
	if (!_walking) {
		_scale = _world.hasWalkArea() ? _world.scaleAt(_x, _y) : 0x100;
		return _speaker.update();
	}

	const int32 dx = _destX - (_fixedX >> 8);
	const int32 dy = _destY - (_fixedY >> 8);
	// Squares are summed as the original's 32-bit integers do, wrapping when a walk has run far off.
	const int32 distance = (int32)((uint32)dx * (uint32)dx + (uint32)dy * (uint32)dy);

	int16 moveX, moveY;
	advanceAnimation(moveX, moveY);
	int32 stepX = (moveX - _moveX) * _scale;
	_moveX = moveX;
	int32 stepY = (moveY - _moveY) * _scale;
	_moveY = moveY;
	tickAnimation();

	// Along the line to the destination: the axis the animation does not move on follows from the other.
	if (_facing < 3) {
		if (dy != 0)
			stepX += (stepY * dx) / dy;
	} else if (dx != 0) {
		stepY += (stepX * dy) / dx;
	}
	_fixedX += stepX;
	_fixedY += stepY;

	if (distance == 0) {
		_x = _destX;
		_y = _destY;
		_world.removeFromMainList(this);
		if (!_target) {
			setFacing(_facing);
		} else {
			const int result = _target->arrived();
			if (result == 2 || result == 3)
				return result;
		}
	} else {
		int32 x, y;
		const int32 stepLength = (int32)((uint32)(stepX >> 8) * (uint32)(stepX >> 8) + (uint32)(stepY >> 8) * (uint32)(stepY >> 8));
		if (distance < stepLength) {
			x = _destX;
			y = _destY;
			_fixedX = x * 256;
			_fixedY = y * 256;
		} else {
			x = _fixedX >> 8;
			y = _fixedY >> 8;
		}
		Object::moveTo(x, y, false);
	}

	_scale = _world.hasWalkArea() ? _world.scaleAt(_x, _y) : 0x100;
	return 1;
}

void Jim::init(const Common::Array<AnimPlayer> &levelPlayers, const SpeechTable::Line *name) {
	_flags = kActive | kVisible | kCollidable | kHasAnimation | kHitter;
	_sortValue = 35;
	_nameKey = nameKey64("JIM");
	_x = 350;
	_y = 20;
	_level = nullptr;
	_room = nullptr;
	_name = name;

	byte indices[kBanks];
	for (uint i = 0; i < kBanks; ++i)
		indices[i] = (byte)i;
	_anim.init(levelPlayers, indices, kBanks);

	// Every bank starts on its first frame; the four walking banks get one point each.
	for (uint i = 0; i < kBanks; ++i)
		_anim.player(i).setRange(0, 0, false, false, _world.now());
	for (uint i = kWalkUp; i <= kWalkRight; ++i)
		_anim.player(i).bank()->makeSinglePoints();

	_facing = 3;
	showFacing(3);
}

void Jim::enterLevel(Level *level, Room *room, int32 x, int32 y) {
	if (_level) {
		_level->movedObjects().clear();
		removeFromList(_level->objects(), this);
		_room->removeObject(this);
		_world.removeFromMainList(_level);
	}

	_level = level;
	_room = room;
	level->objects().push_back(this);
	level->movedObjects().clear();
	room->insertSorted(this);

	const int32 now = _world.now();
	level->camera().moveTo(_x, _y, 0, now);
	level->camera().setTarget(this);
	_world.insertInMainList(level, kLevelPriority);
	level->reloadDeferredAnims(_world.archiveStream());

	// The brightness comes from the level's kind 1 object.
	const CameraFrame *frame = level->cameraFrame();
	for (uint i = 0; i < _anim.bankCount(); ++i) {
		AnimBank *bank = _anim.player(i).bank();
		if (!bank)
			continue;
		bank->loadDeferredImages(_world.archiveStream());
		if (frame)
			bank->scaleBrightness(frame->count());
	}

	_x = _previousX = x;
	_y = _previousY = y;
	_flags = (_flags & ~kHitter) | (level->index() != kMapLevel ? kHitter : 0);

	level->cameraMoveTo(x, y, 0, now);
	update();
}

bool Jim::startWalkAnimation(int facing) {
	const uint bank = kStanding + facing;
	if (_anim.bankIndex() != bank || !(_anim.current().flags() & AnimPlayer::kPlaying)) {
		_anim.switchPlayingTo(bank);
		_anim.current().setRange(1, -1, true, false, _world.now());
		return true;
	}
	return false;
}

void Jim::advanceAnimation(int16 &moveX, int16 &moveY) {
	AnimPlayer &player = _anim.current();
	player.advance(_world.now());
	moveX = player.movementX();
	moveY = player.movementY();
}

void Jim::placeSpeech(uint pose) {
	const int32 cx = _x + kScreenMiddleX;
	AnimPlayer &player = _anim.current();
	const AnimStep &seq = player.bank()->step(player.current());
	if (!seq.hasImage())
		_anchorX = seq.hitW / 2 + cx + seq.hitX;
	else
		_anchorX = seq.imageX + cx + (READ_LE_UINT16(player.bank()->image(seq.imageIndex).data() + 8) >> 1);

	_anim.switchTo(_poses[pose]);
	_anim.current().setRange(1, 1, false, false, _world.now());
}

void Jim::prepareToSpeak(uint32) {
	// Jim speaks in the pose of the way he faces, whatever the line asks for.
	switch (_facing) {
	case 3:
		placeSpeech(0);
		break;
	case 4:
		placeSpeech(1);
		break;
	case 2:
		placeSpeech(2);
		break;
	case 1:
		placeSpeech(3);
		break;
	default:
		break;
	}
	AnimPlayer &player = _anim.current();
	const AnimStep &seq = player.bank()->step(player.current());
	_anchorY = kScreenMiddleY - kSubtitleGap - _y - ((seq.hitY * _scale) >> 8);
}

void Jim::drawScaled(int32 x, int32 y, uint16 depth, int imageIndex) {
	const int32 scale = _scale;
	_display->queueImageScaled(x, y, depth, _anim.current().bank()->image(imageIndex), scale << 8, scale << 8, 0);
}

void Jim::queueFrame() {
	AnimPlayer &player = _anim.current();
	player.advance(_world.now());
	// The speaking banks keep the body in their first step, the mouth frames are what plays.
	const AnimStep &seq = player.bank()->step(0);
	if (!seq.hasImage())
		return;
	const int32 scale = _scale;
	const int32 x = (((seq.imageX * scale) >> 8) + _x) - _camera->scaledX() + kScreenMiddleX + player.movementX();
	const int32 y = (((_camera->scaledY() - ((seq.imageY * scale) >> 8)) - _y) - player.movementY()) + kScreenMiddleY;
	// drawn 1 deeper than the main character to appear above him
	drawScaled(x, y, (uint16)(_sortValue + 1), seq.imageIndex);
}

void Jim::leaveLevel() {
	if (!_level)
		return;
	_level->movedObjects().clear();
	removeFromList(_level->objects(), this);
	_room->removeObject(this);
	_level = nullptr;
	_room = nullptr;
}

void Jim::showFacing(int facing) {
	_anim.selectBank(kStanding, facing - 1, facing - 1, false, false, _world.now());
}

void Jim::updateBounds(int32 now) {
	Object::updateBounds(now);
	const int32 scale = _scale;
	_hitRect.x1 = (_hitRect.x1 * scale) >> 8;
	_hitRect.y1 = (_hitRect.y1 * scale) >> 8;
	_hitRect.x2 = (_hitRect.x2 * scale) >> 8;
	_hitRect.y2 = (_hitRect.y2 * scale) >> 8;
}

int Jim::showHoverText() {
	if (_name) {
		_world.setHoverText(_name->str());
	} else {
		char name[13];
		nameFromKey(_nameKey, name);
		_world.setHoverText(name);
	}
	return 1;
}

void Jim::interactiveDraw() {
	AnimPlayer &player = _anim.current();
	const AnimStep &seq = player.bank()->step(player.current());
	if (!seq.hasImage())
		return;
	const int32 scale = _scale;
	const int32 x = ((seq.imageX * scale) >> 8) - _camera->scaledX() + 320 + _x;
	const int32 y = ((_camera->scaledY() - ((seq.imageY * scale) >> 8)) - _y) + 240;
	drawScaled(x, y, (uint16)_sortValue, seq.imageIndex);
}

void Jim::draw(const Camera &camera, Display &display) {
	_camera = &camera;
	_display = &display;
	_speaker.drawJim();
}

void Jim::saveState(Common::WriteStream &stream) const {
	stream.writeSint32LE(_x);
	stream.writeSint32LE(_y);
	stream.writeSint32LE(_facing);
	stream.writeByte(_anim.selectedBank());
	stream.writeUint32LE(_world.progressBytes());
	stream.writeUint32LE(_flags);
}

void Jim::loadState(Common::ReadStream &stream) {
	_walking = false;
	_world.removeFromMainList(this);
	_x = stream.readSint32LE();
	_y = stream.readSint32LE();
	_facing = stream.readSint32LE();
	_anim.setSelectedBank(stream.readByte());
	_world.setProgressBytes(stream.readUint32LE());
	_flags = stream.readUint32LE();
}

int Jim::useItem(int item) { return 0; }
int Jim::setVerbAndClick(int verb) { return _world.jimClicked(verb); }
int Jim::doubleClick(int verb) { return _world.jimClicked(verb); }
bool Jim::findPath(int32 destX, int32 destY, WalkTarget *target) { return _world.findPath(destX, destY, target); }
int32 Jim::now() { return _world.now(); }
uint32 Jim::randomNumber(uint32 lo, uint32 hi) { return _world.randomNumber(lo, hi); }
uint32 Jim::playVoice(uint32 archiveOffset) { return _world.playVoice(archiveOffset); }
bool Jim::voiceActive(uint32 id) { return _world.voiceActive(id); }
void Jim::stopVoice(uint32 id) { _world.stopVoice(id); }
bool Jim::skipPressed() { return _world.skipPressed(); }
bool Jim::subtitlesOn() { return _world.subtitlesOn(); }
void Jim::insertInMainList(int priority) { _world.insertInMainList(this, priority); }
void Jim::removeFromMainList() { _world.removeFromMainList(this); }
void Jim::pauseAnimation() { _anim.current().pause(_world.now()); }
void Jim::setFrameRange(uint16 first, int32 last) { _anim.current().setRange(first, last, false, false, _world.now()); }
int Jim::interactiveUpdate() { return _world.interactiveUpdate(*this); }
void Jim::drawSubtitle(const SpeechTable::Line &line) { _world.drawSubtitle(*this, line); }
void Jim::tickAnimation() { _anim.tickClearMovement(_world.now()); }
int Jim::clearHoverText() {
	_world.setHoverText(nullptr);
	return 1;
}
} // End of namespace Tesoro
