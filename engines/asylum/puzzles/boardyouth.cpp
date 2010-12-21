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

static const PuzzleBoard::PuzzleData puzzleYouthData = {
	55,
	kGameFlag282,
	431,
	2,
	{{3, false}, {4, false}, {0, false}},
	8,
	{{'E', Common::Point( 64,  55)},
	{'U', Common::Point( 26,  69)},
	{'T', Common::Point(135, 102)},
	{'O', Common::Point( 57, 134)},
	{'H', Common::Point(417, 152)},
	{'T', Common::Point(223, 181)},
	{'H', Common::Point(497, 198)},
	{'Y', Common::Point(435, 231)}},
	true,
	"T H E   Y O U T H "
};

PuzzleBoardYouth::PuzzleBoardYouth(AsylumEngine *engine) : PuzzleBoard(engine, puzzleYouthData) {
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleBoardYouth::mouseLeftDown(const AsylumEvent &evt) {
	error("[PuzzleBoardYouth::mouseLeftDown] Not implemented!");
}

} // End of namespace Asylum
