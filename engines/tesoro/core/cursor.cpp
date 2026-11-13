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

#include "common/textconsole.h"

#include "tesoro/core/cursor.h"
#include "tesoro/core/camera.h"
#include "tesoro/core/display.h"
#include "tesoro/core/level.h"
#include "tesoro/core/room.h"
#include "tesoro/world.h"

namespace Tesoro {

void Cursor::init() {
	initPlain();
	_sortValue = 0;
	_flags |= kCollidable;
}

void Cursor::initMain(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now) {
	SpriteObject::init(record, roomSortOffset, levelPlayers, now);

	_sortValue = 0;
	_flags |= kCollidable;

	_companion = new Cursor();
	_companion->init();
	_companion->_owner = this;
}

void Cursor::updateAnimation(int32 now) {
	if (AnimPlayer *player = currentPlayer())
		player->advance(now);
}

void Cursor::onAttach() {
	// Only the steps' hit rectangles of the main pointer change: they all become points.
	if (_companion && _anim.bankCount() != 0) {
		AnimBank *bank = _anim.player(0).bank();
		for (uint i = 0; i < bank->stepCount(); ++i)
			bank->clearStepHitRect(i);
	}

	SpriteObject::onAttach();
}

void Cursor::draw(const Camera &camera, Display &display) {
	AnimPlayer &player = _anim.current();
	const AnimStep &step = player.bank()->step(player.current());
	if (step.hasImage()) {
		const int32 x = (int32)((uint32)player.movementX() + (uint32)(int32)step.imageX + (uint32)_x - (uint32)camera.scaledX() + 0x140);
		const int32 y = (int32)((uint32)(camera.scaledY() - step.imageY) - (uint32)player.movementY() - (uint32)_y + 0xf0);
		display.queueImageScaled(x, y, (uint16)_sortValue, player.bank()->image(step.imageIndex), 0x10000, 0x10000, 0);
	}

	// The name of what is under the pointer, centered above it.
	if (_label.text()[0]) {
		const int32 centreX = (int32)((uint32)_x - (uint32)camera.scaledX() + 0x140);
		display.queueText((const byte *)_label.text(), _label.colour(), centreX, camera.scaledY() - _y + 0xcb, 200, 0, true, 0);
	}
}

int Cursor::update() {
	// The companion has no step of its own: the main pointer moves it.
	if (!_companion)
		return 0;

	if (!_mouse || !_level) {
		warning("Cursor: no mouse or level to follow");
		return 0;
	}

	// What was touched is kept until the next frame's step, so it can be looked at in between.
	_touched.clear();
	if (_companion)
		_companion->_touched.clear();

	moveTo(_mouse->x + _level->camera().x(), _level->camera().y() - _mouse->y, true);

	if (_playerLevel && _playerLevel->roomCount() != 0) {
		const Camera &playerCamera = _playerLevel->camera();
		_companion->_level = _playerLevel;
		_companion->_room = &_playerLevel->room(0);
		_companion->moveTo(_mouse->x + playerCamera.x(), playerCamera.y() - _mouse->y, true);
	}
	return 1;
}

int Cursor::update2() {
	if (!_companion || !_mouse)
		return 0;

	const byte left = _mouse->buttonState(PointerState::kLeft);
	// takes the current animation step as the verb for the click
	const int verb = _anim.current().current();

	int result = processCandidates(left, verb);
	clearTouched();
	if (result != 0) {
		_companion->clearTouched();
		return result;
	}

	result = _companion->processCandidates(left, verb);
	_companion->clearTouched();
	if (result == 2 || result == 3)
		return result;

	// A click on bare ground with the walking pointer sends the player there.
	if ((left & 8) && result == 0 && verb == 0)
		_host->walkJimToPointer(_mouse->x, _mouse->y);

	return 1;
}

int Cursor::onTouched(Object *other, int extra) {
	_touched.push_back(other);
	return 1;
}

void Cursor::setActive(bool active) {
	Object::setActive(active);
	if (active)
		return;

	for (uint i = 0; i < _hovered.size(); ++i)
		_hovered[i]->clearHoverText();
	_hovered.clear();
	_label.set(nullptr, 0);
}

int Cursor::processCandidates(byte buttons, int verb) {
	// What was hovered last time becomes the list to compare with.
	ObjectList previous = _hovered;
	_hovered.clear();

	int result = 0;
	for (uint t = 0; t < _touched.size() && result == 0; ++t) {
		Object *object = _touched[t];

		bool hoveredBefore = false;
		for (uint i = 0; i < previous.size(); ++i) {
			if (previous[i]->nameKey() == object->nameKey()) {
				hoveredBefore = true;
				break;
			}
		}
		// goes through all touched objects (detected by collision)
		// until it finds one that returns != 0
		if (!hoveredBefore) {
			// if it wasnt hovered in the previous frame, just displays the text
			if (object->acceptsPointer()) {
				_hovered.push_back(object);
				result = object->showHoverText();
			}
		} else { // if it was hovered before, it accepts clicks
			if (buttons & 8)
				result = object->setVerbAndClick(verb);
			else if (buttons & 0x10)
				result = object->rightClick(verb);
			else
				result = object->showHoverText();

			if (result == 3)
				return 3;

			_hovered.push_back(object);
			removeFromList(previous, object);
		}
	}

	// Whatever was hovered last frame and not seen again loses its label.
	for (uint i = 0; i < previous.size(); ++i)
		previous[i]->clearHoverText();

	return result;
}

} // End of namespace Tesoro
