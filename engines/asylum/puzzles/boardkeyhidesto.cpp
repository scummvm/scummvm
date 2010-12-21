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

#include "asylum/puzzles/boardkeyhidesto.h"

namespace Asylum {

PuzzleBoardKeyHidesTo::PuzzleBoardKeyHidesTo(AsylumEngine *engine) : Puzzle(engine) {
}

PuzzleBoardKeyHidesTo::~PuzzleBoardKeyHidesTo() {
}

void PuzzleBoardKeyHidesTo::reset() {
	memset(&_charUsed, 0, sizeof(_charUsed));
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleBoardKeyHidesTo::init(const AsylumEvent &evt)  {
	error("[PuzzleBoardKeyHidesTo::init] Not implemented!");
}

bool PuzzleBoardKeyHidesTo::activate(const AsylumEvent &evt)  {
	error("[PuzzleBoardKeyHidesTo::activate] Not implemented!");
}

bool PuzzleBoardKeyHidesTo::update(const AsylumEvent &evt)  {
	error("[PuzzleBoardKeyHidesTo::update] Not implemented!");
}

bool PuzzleBoardKeyHidesTo::mouseLeftDown(const AsylumEvent &evt) {
	error("[PuzzleBoardKeyHidesTo::mouseLeftDown] Not implemented!");
}

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////
void PuzzleBoardKeyHidesTo::updateCursor() {
	error("[PuzzleBoardKeyHidesTo::updateCursor] Not implemented!");
}

void PuzzleBoardKeyHidesTo::drawText() {
	error("[PuzzleBoardKeyHidesTo::drawText] Not implemented!");
}

int32 PuzzleBoardKeyHidesTo::findRect() {
	error("[PuzzleBoardKeyHidesTo::findRect] Not implemented!");
}

int32 PuzzleBoardKeyHidesTo::checkMouse() {
	error("[PuzzleBoardKeyHidesTo::checkMouse] Not implemented!");
}

void PuzzleBoardKeyHidesTo::playSound() {
	error("[PuzzleBoardKeyHidesTo::playSound] Not implemented!");
}


} // End of namespace Asylum
