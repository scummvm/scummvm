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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ASYLUM_PUZZLES_BOARDKEYHIDESTO_H
#define ASYLUM_PUZZLES_BOARDKEYHIDESTO_H

#include "asylum/puzzles/board.h"

namespace Asylum {

class AsylumEngine;

class PuzzleBoardKeyHidesTo : public PuzzleBoard {
public:
	PuzzleBoardKeyHidesTo(AsylumEngine *engine);

	// Serializable
	virtual void saveLoadWithSerializer(Common::Serializer &s);

private:
	//////////////////////////////////////////////////////////////////////////
	// Event Handling
	//////////////////////////////////////////////////////////////////////////
	bool mouseLeftDown(const AsylumEvent &evt);
};

} // End of namespace Asylum

#endif // ASYLUM_PUZZLES_BOARDKEYHIDESTO_H
