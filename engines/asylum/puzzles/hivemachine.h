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

#ifndef ASYLUM_PUZZLES_HIVEMACHINE_H
#define ASYLUM_PUZZLES_HIVEMACHINE_H

#include "common/array.h"

#include "asylum/puzzles/puzzle.h"

namespace Asylum {

class AsylumEngine;

class PuzzleHiveMachine : public Puzzle {
public:
	PuzzleHiveMachine(AsylumEngine *engine);
	~PuzzleHiveMachine();

private:
	enum MusicalNote {
		kMusicalNoteNone = -1,
		kMusicalNoteF,
		kMusicalNoteD,
		kMusicalNoteE,
		kMusicalNoteA,
		kMusicalNoteB
	};

	uint32 _counterRed, _counterGreen, _counterKey;
	int32 _rectIndex;
	uint32 _frameIndex, _frameIndex1;
	Common::Array<MusicalNote> _melody;
	MusicalNote _soundingNote;
	uint32 _notesNumber;
	bool _ok;

	//////////////////////////////////////////////////////////////////////////
	// Event Handling
	//////////////////////////////////////////////////////////////////////////
	bool init(const AsylumEvent &evt);
	bool mouseLeftDown(const AsylumEvent &evt);
	bool mouseRightDown(const AsylumEvent &evt);

	//////////////////////////////////////////////////////////////////////////
	// Helpers
	//////////////////////////////////////////////////////////////////////////
	void updateCursor();
	int32 findRect();
	void updateScreen();
	void playSound();
};

} // End of namespace Asylum

#endif // ASYLUM_PUZZLES_HIVEMACHINE_H
