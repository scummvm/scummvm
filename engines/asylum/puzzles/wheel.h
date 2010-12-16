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

#ifndef ASYLUM_WHEEL_H
#define ASYLUM_WHEEL_H

#include "asylum/puzzles/puzzle.h"

namespace Asylum {

class AsylumEngine;

class PuzzleWheel : public Puzzle {
public:
	PuzzleWheel(AsylumEngine *engine);
	~PuzzleWheel();

	void reset();

private:
	int32 _currentRect;
	uint32 _resourceIndex;
	uint32 _resourceIndex9;
	uint32 _resourceIndex10;
	uint32 _frameIndex30;
	int32 _frameIndexes[8];
	int32 _frameCounts[12];

	bool _showResource9;
	bool _flag1;
	bool _flag2;
	bool _flag3;

	//////////////////////////////////////////////////////////////////////////
	// Event Handling
	//////////////////////////////////////////////////////////////////////////
	bool init(const AsylumEvent &evt);
	bool update(const AsylumEvent &evt);
	bool mouseLeftDown(const AsylumEvent &evt);
	bool mouseRightDown(const AsylumEvent &evt);

	//////////////////////////////////////////////////////////////////////////
	// Helpers
	//////////////////////////////////////////////////////////////////////////
	void updateCursor(const AsylumEvent &evt);
	int32 findRect(Common::Point mousePos);
	void updateIndex();
	void checkFlags();
	void playSound();
	void playSoundReset();
};

} // End of namespace Asylum

#endif // ASYLUM_WHEEL_H
