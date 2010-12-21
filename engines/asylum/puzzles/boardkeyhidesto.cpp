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

static const PuzzleBoard::PuzzleData puzzleKeyHidesToData = {
	56,
	kGameFlag283,
	503,
	3,
	{{5, false}, {6, false}, {7, false}},
	10,
	{{'I', Common::Point( 30,  53)},
	{'E', Common::Point(212,  71)},
	{'D', Common::Point( 31, 103)},
	{'H', Common::Point(447, 134)},
	{'S', Common::Point(240, 151)},
	{'E', Common::Point( 95, 167)},
	{'O', Common::Point(372, 182)},
	{'K', Common::Point(210, 215)},
	{'Y', Common::Point(440, 247)},
	{'T', Common::Point(479, 262)}},
	true
};

PuzzleBoardKeyHidesTo::PuzzleBoardKeyHidesTo(AsylumEngine *engine) : PuzzleBoard(engine, puzzleKeyHidesToData) {
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleBoardKeyHidesTo::mouseLeftDown(const AsylumEvent &evt) {
	error("[PuzzleBoardKeyHidesTo::mouseLeftDown] Not implemented!");
}

} // End of namespace Asylum
