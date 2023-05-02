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

#include "tetraedge/game/puzzle_disjoncteur.h"

namespace Tetraedge {

PuzzleDisjoncteur::PuzzleDisjoncteur() : _state(0) {
}

void PuzzleDisjoncteur::wakeUp() {
	error("TODO: Implement PuzzleDisjoncteur::wakeUp");
}

void PuzzleDisjoncteur::sleep() {
	error("TODO: Implement PuzzleDisjoncteur::sleep");
}

void PuzzleDisjoncteur::addState(uint32 flags) {
	if (!(_state & flags))
		_state += flags;
}

void PuzzleDisjoncteur::removeState(uint32 flags) {
	if (_state & flags)
		_state -= flags;
}

void PuzzleDisjoncteur::setDraggedSprite(TeSpriteLayout *sprite) {
	error("TODO: Implement PuzzleDisjoncteur::setDraggedSprite");
}

void PuzzleDisjoncteur::setDraggedSpriteBack() {
	error("TODO: Implement PuzzleDisjoncteur::setDraggedSpriteBack");
}

bool PuzzleDisjoncteur::onExitButton() {
	sleep();
	return false;
}

bool PuzzleDisjoncteur::onLevierAnimFinished() {
	error("TODO: Implement PuzzleDisjoncteur::onLevierAnimFinished");
}

bool PuzzleDisjoncteur::onMouseDown(const Common::Point &pt) {
	error("TODO: Implement PuzzleDisjoncteur::onMouseDown");
}

bool PuzzleDisjoncteur::onMouseMove(const Common::Point &pt) {
	error("TODO: Implement PuzzleDisjoncteur::onMouseMove");
}

bool PuzzleDisjoncteur::onMouseUp(const Common::Point &pt) {
	error("TODO: Implement PuzzleDisjoncteur::onMouseUp");
}

bool PuzzleDisjoncteur::onWinTimer() {
	sleep();
	return false;
}


} // end namespace Tetraedge
