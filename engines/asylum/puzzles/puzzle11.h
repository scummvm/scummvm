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

#ifndef ASYLUM_PUZZLE11_H
#define ASYLUM_PUZZLE11_H

#include "asylum/puzzles/puzzle.h"

namespace Asylum {

class AsylumEngine;

class Puzzle11 : public Puzzle {
public:
	Puzzle11(AsylumEngine *engine);
	~Puzzle11();

private:
	uint32 _counter;
	uint32 _frameCounts[5];
	uint32 _frameIndexes[7];
	bool _data_457298;
	bool _data_45AA34;

	//////////////////////////////////////////////////////////////////////////
	// Event Handling
	//////////////////////////////////////////////////////////////////////////
	bool init(const AsylumEvent &evt);
	bool update(const AsylumEvent &evt);
	bool mouseLeftDown(const AsylumEvent &evt);
	bool mouseLeftUp(const AsylumEvent &evt);
	bool mouseRightDown(const AsylumEvent &evt);
};

} // End of namespace Asylum

#endif // ASYLUM_PUZZLE11_H
