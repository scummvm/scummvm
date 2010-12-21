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

#include "asylum/puzzles/boardyouth.h"

namespace Asylum {

static const struct {
	char character;
	Common::Point position;
} puzzleYouthCharmap[8] = {
	{'E', Common::Point( 64,  55)},
	{'U', Common::Point( 26,  69)},
	{'T', Common::Point(135, 102)},
	{'O', Common::Point( 57, 134)},
	{'H', Common::Point(417, 152)},
	{'T', Common::Point(223, 181)},
	{'H', Common::Point(497, 198)},
	{'Y', Common::Point(435, 231)}
};

PuzzleBoardYouth::PuzzleBoardYouth(AsylumEngine *engine) : Puzzle(engine) {
}

PuzzleBoardYouth::~PuzzleBoardYouth() {
}

void PuzzleBoardYouth::reset() {
	memset(&_charUsed, 0, sizeof(_charUsed));
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleBoardYouth::init(const AsylumEvent &evt)  {
	error("[PuzzleBoardYouth::init] Not implemented!");
}

bool PuzzleBoardYouth::activate(const AsylumEvent &evt)  {
	error("[PuzzleBoardYouth::activate] Not implemented!");
}

bool PuzzleBoardYouth::update(const AsylumEvent &evt)  {
	error("[PuzzleBoardYouth::update] Not implemented!");
}

bool PuzzleBoardYouth::mouseLeftDown(const AsylumEvent &evt) {
	error("[PuzzleBoardYouth::mouseLeftDown] Not implemented!");
}

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////
void PuzzleBoardYouth::updateCursor() {
	error("[PuzzleBoardYouth::updateCursor] Not implemented!");
}

void PuzzleBoardYouth::drawText() {
	error("[PuzzleBoardYouth::drawText] Not implemented!");
}

int32 PuzzleBoardYouth::findRect() {
	error("[PuzzleBoardYouth::findRect] Not implemented!");
}

int32 PuzzleBoardYouth::checkMouse() {
	error("[PuzzleBoardYouth::checkMouse] Not implemented!");
}

void PuzzleBoardYouth::playSound() {
	error("[PuzzleBoardYouth::playSound] Not implemented!");
}

} // End of namespace Asylum
