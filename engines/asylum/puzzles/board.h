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
 */

#ifndef ASYLUM_BOARD_H
#define ASYLUM_BOARD_H

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
		Common::Point position;
	};

	struct PuzzleData {
		uint32 backgroundIndex;
		GameFlag gameFlag;
		uint32 maxWidth;
		uint32 soundResourceSize;
		SoundResource soundResources[3];
		uint32 charMapSize;
		CharMap charMap[10];
		bool checkForSpace;
		char solvedText[28];
	};

	PuzzleBoard(AsylumEngine *engine);
	PuzzleBoard(AsylumEngine *engine, PuzzleData data);

	void reset();

protected:
	bool _solved;
	char _text[800];
	bool _charUsed[20];
	char _solvedText[28]; // KeyHidesTo uses 28 chars, the other puzzles 20
	uint32 _position;
	int32 _rectIndex;
	int32 _selectedSlot;
	ResourceId _soundResourceId;

	//////////////////////////////////////////////////////////////////////////
	// Helpers
	//////////////////////////////////////////////////////////////////////////
	bool updateScreen();
	int32 findRect();
	bool stopSound();
	void checkSlots();

private:
	PuzzleData _data;

	//////////////////////////////////////////////////////////////////////////
	// Event Handling
	//////////////////////////////////////////////////////////////////////////
	bool init(const AsylumEvent &evt);
	bool activate(const AsylumEvent &evt) { return updateScreen(); }
	bool update(const AsylumEvent &evt);
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

#endif // ASYLUM_BOARD_H
