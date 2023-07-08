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

#include "asylum/puzzles/boardyouth.h"

#include "asylum/system/cursor.h"

#include "asylum/asylum.h"

namespace Asylum {

static const PuzzleBoard::PuzzleData puzzleYouthData[] = {
	// English
	{
		55,
		kGameFlag282,
		431,
		2,
		{{3, false}, {4, false}, {0, false}},
		8,
		{
			{'E',   64,  55},
			{'U',   26,  69},
			{'T',  135, 102},
			{'O',   57, 134},
			{'H',  417, 152},
			{'T',  223, 181},
			{'H',  497, 198},
			{'Y',  435, 231},
			{'\0',   0,   0},
			{'\0',   0,   0},
			{'\0',   0,   0}
		},
		true,
		6, 0,
		"T H E   Y O U T H "
	},
	// German
	{
		55,
		kGameFlag282,
		503,
		2,
		{{3, false}, {4, false}, {0, false}},
		9,
		{
			{'G',  25,  50},
			{'E',  60,  66},
			{'D', 471,  82},
			{'N', 340, 114},
			{'J', 102, 146},
			{'U', 311, 162},
			{'R', 261, 194},
			{'E', 390, 210},
			{'D', 470, 226},
			{'\0',  0,   0},
			{'\0',  0,   0}
		},
		true,
		12, 0,
		"J U G E N D   D E R "
	},
	// French
	{
		55,
		kGameFlag282,
		503,
		2,
		{{3, false}, {4, false}, {0, false}},
		11,
		{
			{'E',  64,  50},
			{'S', 514,  50},
			{'T', 192,  98},
			{'J', 297, 114},
			{'E', 595, 130},
			{'S',  62, 146},
			{'U', 376, 146},
			{'N', 281, 162},
			{'E', 482, 178},
			{'E',  66, 194},
			{'S', 133, 210},
		},
		true,
		6, 0,
		"E S T   J E U N E S S E "
	},
	// Hebrew
	{
		55,
		kGameFlag282,
		431,
		2,
		{{3, false}, {4, false}, {0, false}},
		8,
		{
			{'\xF0', 596,  59},
			{'\xE5', 226,  58},
			{'\xE4', 113,  74},
			{'\xE0', 494, 106},
			{'\xF8', 461, 122},
			{'\xF2',  76, 155},
			{'\xFA', 141, 186},
			{'\0',     0,   0},
			{'\0',     0,   0},
			{'\0',     0,   0},
			{'\0',     0,   0}
		},
		true,
		10, 0,
		"\xF8 \xF2 \xE5 \xF0 \xE4   \xFA \xE0 "
	}
};

PuzzleBoardYouth::PuzzleBoardYouth(AsylumEngine *engine) : PuzzleBoard(engine, puzzleYouthData) {
}

void PuzzleBoardYouth::saveLoadWithSerializer(Common::Serializer &s) {
	for (int32 i = 0; i < 9; i++)
		s.syncAsUint32LE(_charUsed[i]);

	s.syncBytes((byte *)&_solvedText, 20);

	s.syncAsUint32LE(_position);
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleBoardYouth::mouseLeftDown(const AsylumEvent &) {
	Common::Point mousePos = getCursor()->position();

	if (mousePos.y <= 350) {
		int32 index = findRect();

		if (index != -1 && _position < strlen(_data.solvedText)) {
			_charUsed[index] = true;
			_selectedSlot = -1;

			_solvedText[_position++] = _data.charMap[index].character;
			_solvedText[_position++] = ' ';

			if (_position == _data.space1Pos) {
				_solvedText[_position++] = ' ';
				_solvedText[_position++] = ' ';
			}

			updateScreen();
		}
	} else if (_vm->isGameFlagNotSet(kGameFlag282)) {
		checkSlots();
	}

	return true;
}

} // End of namespace Asylum
