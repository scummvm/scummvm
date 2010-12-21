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
 * $URL$
 * $Id$
 *
 */

#include "asylum/puzzles/hivecontrol.h"

namespace Asylum {

PuzzleHiveControl::PuzzleHiveControl(AsylumEngine *engine) : Puzzle(engine) {
}

PuzzleHiveControl::~PuzzleHiveControl() {
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleHiveControl::init(const AsylumEvent &evt)  {
	error("[PuzzleHiveControl::init] Not implemented!");
}

bool PuzzleHiveControl::update(const AsylumEvent &evt)  {
	error("[PuzzleHiveControl::update] Not implemented!");
}

bool PuzzleHiveControl::mouseLeftDown(const AsylumEvent &evt) {
	error("[PuzzleHiveControl::mouseLeftDown] Not implemented!");
}

bool PuzzleHiveControl::mouseRightDown(const AsylumEvent &evt) {
	error("[PuzzleHiveControl::mouseLeftDown] Not implemented!");
}

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////
void PuzzleHiveControl::updateCursor() {
	error("[PuzzleHiveControl::updateCursor] Not implemented!");
}

int32 PuzzleHiveControl::findRect() {
	error("[PuzzleHiveControl::findRect] Not implemented!");
}

void PuzzleHiveControl::updateScreen() {
	error("[PuzzleHiveControl::updateScreen] Not implemented!");
}

void PuzzleHiveControl::playSound() {
	error("[PuzzleHiveControl::playSound] Not implemented!");
}

} // End of namespace Asylum
