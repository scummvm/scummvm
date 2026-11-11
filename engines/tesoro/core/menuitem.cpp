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

#include "tesoro/core/menuitem.h"
#include "tesoro/tesoro.h"

#include "common/textconsole.h"

#include "tesoro/core/camera.h"
#include "tesoro/core/display.h"

namespace Tesoro {

void MenuItem::init(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now) {
	initAnimated(record, roomSortOffset, levelPlayers, 0);

	const byte first = record.dataByte(0);
	_action = record.dataByte(2);

	if (_anim.bankCount() == 0) {
		warning("MenuItem: object without animation");
		return;
	}

	_anim.selectBank(0, first, first, false, false, now);
	// Collidable, in its room's list and a hitter; not visible until the menu shows it.
	_flags = (_flags & ~kVisible) | kCollidable | kInRoomList | kHitter;
}

void MenuItem::updateAnimation(int32 now) {
	_anim.tickClearMovement(now);
}

void MenuItem::draw(const Camera &camera, Display &display) {
	drawAnimation(camera, display, true);
}

int MenuItem::update() {
	if (_host)
		updateBounds(_host->now());
	return 1;
}

int MenuItem::setVerbAndClick(int verb) {
	switch (_action) {
	case 0:
		_host->fadeOut(4, 100);
		_host->enterNewGameMenu();
		_host->startScript("FANMMENU", "nuevo");
		return 3;
	case 1:
		_host->fadeOut(4, 100);
		_host->openSavedGames();
		return 1;
	case 2:
		_host->startScript("FANMMENU", "facil");
		return 1;
	case 3:
		_host->fadeOut(4, 100);
		_host->startGame();
		return 3;
	case 4:
		_host->startScript("FANMMENU", "dificil");
		return 1;
	default:
		return 1;
	}
}

int MenuItem::rightClick(int verb) {
	return setVerbAndClick(verb);
}

int MenuItem::clearHoverText() {
	if (_host)
		_host->clearHoverText();
	return 1;
}

} // End of namespace Tesoro
