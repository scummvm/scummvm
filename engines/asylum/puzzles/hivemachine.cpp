/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "asylum/puzzles/hivemachine.h"

#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"

namespace Asylum {

PuzzleHiveMachine::PuzzleHiveMachine(AsylumEngine *engine) : Puzzle(engine) {
	_rectIndex = 0;
}

PuzzleHiveMachine::~PuzzleHiveMachine() {
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleHiveMachine::init(const AsylumEvent &evt)  {
	error("[PuzzleHiveMachine::init] Not implemented!");

	getScreen()->setPalette(getWorld()->graphicResourceIds[9]);
	getScreen()->setGammaLevel(getWorld()->graphicResourceIds[9], 0);
	_rectIndex = -2;
	updateCursor();

	return true;
}

bool PuzzleHiveMachine::update(const AsylumEvent &evt)  {
	error("[PuzzleHiveMachine::update] Not implemented!");
}

bool PuzzleHiveMachine::mouseLeftDown(const AsylumEvent &evt) {
	error("[PuzzleHiveMachine::mouseLeftDown] Not implemented!");
}

bool PuzzleHiveMachine::mouseRightDown(const AsylumEvent &evt) {
	getScreen()->clear();
	_vm->switchEventHandler(getScene());

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////
void PuzzleHiveMachine::updateCursor() {
	int32 index = findRect();

	if (_rectIndex == index)
		return;

	_rectIndex = index;
	if (index == -1)
		getCursor()->set(getWorld()->graphicResourceIds[12], -1, kCursorAnimationNone);
	else
		getCursor()->set(getWorld()->graphicResourceIds[12], -1);
}

int32 PuzzleHiveMachine::findRect() {
	error("[PuzzleHiveMachine::findRect] Not implemented!");
}

void PuzzleHiveMachine::updateScreen() {
	error("[PuzzleHiveMachine::updateScreen] Not implemented!");
}

void PuzzleHiveMachine::playSound() {
	error("[PuzzleHiveMachine::playSound] Not implemented!");
}

} // End of namespace Asylum
