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

#ifndef ASYLUM_PUZZLES_BOARD_H
#define ASYLUM_PUZZLES_BOARD_H

#include "asylum/puzzles/puzzle.h"

#include "asylum/shared.h"

namespace Asylum {

class AsylumEngine;

class PuzzleBoard : public Puzzle {
public:
	struct SoundResource {
		int32 index;
		bool played;
	};

	struct CharMap {
		char character;
		int16 posX, posY;
	};

	struct PuzzleData {
		uint32 backgroundIndex;
		GameFlag gameFlag;
		uint32 maxWidth;
		uint32 soundResourceSize;
		SoundResource soundResources[3];
		uint32 charMapSize;
		CharMap charMap[11];
		bool checkForSpace;
		uint32 space1Pos, space2Pos;
		char solvedText[28];
	};

	PuzzleBoard(AsylumEngine *engine, const PuzzleData *data);

	void reset();

protected:
	PuzzleData _data;
	bool _solved;
	Common::String _text;
	bool _charUsed[20];
	char _solvedText[28]; // KeyHidesTo uses 28 chars, the other puzzles 20
	uint32 _position;
	int32 _rectIndex;
	int32 _selectedSlot;
	ResourceId _soundResourceId;

	//////////////////////////////////////////////////////////////////////////
	// Helpers
	//////////////////////////////////////////////////////////////////////////
	void updateScreen();
	int32 findRect();
	bool stopSound();
	void checkSlots();

private:
	//////////////////////////////////////////////////////////////////////////
	// Event Handling
	//////////////////////////////////////////////////////////////////////////
	bool init(const AsylumEvent &evt);
	bool activate(const AsylumEvent &evt) { return updateScreen(), true; }
	virtual bool mouseRightDown(const AsylumEvent &evt);

	//////////////////////////////////////////////////////////////////////////
	// Helpers
	//////////////////////////////////////////////////////////////////////////
	void drawText();
	void playSound();
	int32 checkMouse();
	void updateCursor();
};

} // End of namespace Asylum

#endif // ASYLUM_PUZZLES_BOARD_H
