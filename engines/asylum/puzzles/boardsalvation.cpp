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

#include "asylum/puzzles/boardsalvation.h"

namespace Asylum {

static const struct {
	char character;
	Common::Point position;
} puzzleSalvationCharmap[9] = {
	{'I', Common::Point( 61,  53)},
	{'S', Common::Point(322,  53)},
	{'A', Common::Point(529,  86)},
	{'L', Common::Point(256, 117)},
	{'V', Common::Point(251, 151)},
	{'A', Common::Point( 66, 199)},
	{'T', Common::Point(436, 229)},
	{'O', Common::Point(172, 262)},
	{'N', Common::Point(393, 296)}
};

PuzzleBoardSalvation::PuzzleBoardSalvation(AsylumEngine *engine) : Puzzle(engine) {
}

PuzzleBoardSalvation::~PuzzleBoardSalvation() {
}

void PuzzleBoardSalvation::reset() {
	memset(&_charUsed, 0, sizeof(_charUsed));
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleBoardSalvation::init(const AsylumEvent &evt)  {
	error("[PuzzleBoardSalvation::init] Not implemented!");
}

bool PuzzleBoardSalvation::activate(const AsylumEvent &evt)  {
	error("[PuzzleBoardSalvation::activate] Not implemented!");
}

bool PuzzleBoardSalvation::update(const AsylumEvent &evt)  {
	error("[PuzzleBoardSalvation::update] Not implemented!");
}

bool PuzzleBoardSalvation::mouseLeftDown(const AsylumEvent &evt) {
	error("[PuzzleBoardSalvation::mouseLeftDown] Not implemented!");
}

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////
void PuzzleBoardSalvation::updateCursor() {
	error("[PuzzleBoardSalvation::updateCursor] Not implemented!");
}

void PuzzleBoardSalvation::drawText() {
	error("[PuzzleBoardSalvation::drawText] Not implemented!");
}

int32 PuzzleBoardSalvation::findRect() {
	error("[PuzzleBoardSalvation::findRect] Not implemented!");
}

int32 PuzzleBoardSalvation::checkMouse() {
	error("[PuzzleBoardSalvation::checkMouse] Not implemented!");
}

void PuzzleBoardSalvation::playSound() {
	error("[PuzzleBoardSalvation::playSound] Not implemented!");
}

} // End of namespace Asylum
