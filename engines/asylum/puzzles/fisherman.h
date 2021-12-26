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

#ifndef ASYLUM_PUZZLES_FISHERMAN_H
#define ASYLUM_PUZZLES_FISHERMAN_H

#include "asylum/puzzles/puzzle.h"

namespace Asylum {

class AsylumEngine;

class PuzzleFisherman : public Puzzle {
public:
	PuzzleFisherman(AsylumEngine *engine);
	~PuzzleFisherman();

	// Serializable
	virtual void saveLoadWithSerializer(Common::Serializer &s);

private:
	bool _state[6];
	int32 _counter;

	int32 _pauseTimer;
	bool  _allowClick;
	bool  _resetPressed;

	//////////////////////////////////////////////////////////////////////////
	// Event Handling
	//////////////////////////////////////////////////////////////////////////
	bool init(const AsylumEvent &evt);
	void updateScreen();
	bool key(const AsylumEvent &evt) { return keyExit(evt); }
	bool mouseLeftDown(const AsylumEvent &evt);
	bool mouseRightDown(const AsylumEvent &evt);

	//////////////////////////////////////////////////////////////////////////
	// Helpers
	//////////////////////////////////////////////////////////////////////////
	void updateCursor();
	void setFlags(uint32 index);
};

} // End of namespace Asylum

#endif // ASYLUM_PUZZLES_FISHERMAN_H
