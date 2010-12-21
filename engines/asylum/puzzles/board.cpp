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

#include "asylum/puzzles/board.h"

#include "asylum/resources/worldstats.h"

#include "asylum/system/screen.h"
#include "asylum/system/sound.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"

namespace Asylum {

PuzzleBoard::PuzzleBoard(AsylumEngine *engine) : Puzzle(engine) {
	_backgroundIndex = -1;
}

PuzzleBoard::PuzzleBoard(AsylumEngine *engine, int32 backgroundIndex) : Puzzle(engine) {
	_backgroundIndex = backgroundIndex;
}

void PuzzleBoard::reset() {
	memset(&_charUsed, 0, sizeof(_charUsed));
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleBoard::activate(const AsylumEvent &evt)  {
	if (_backgroundIndex == -1)
		error("[PuzzleBoard::activate] Invalid background index!");

	getScreen()->clearGraphicsInQueue();

	getScreen()->draw(getWorld()->graphicResourceIds[_backgroundIndex]);
	drawText();

	getScreen()->drawGraphicsInQueue();
	getScreen()->copyBackBufferToScreen();

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////
bool PuzzleBoard::stopSound() {
	ResourceId soundResourceId = getPuzzleData()->soundResourceId;

	if (soundResourceId && getSound()->isPlaying(soundResourceId)) {
		getSound()->stopAll(soundResourceId);

		return true;
	}

	return false;
}

} // End of namespace Asylum
