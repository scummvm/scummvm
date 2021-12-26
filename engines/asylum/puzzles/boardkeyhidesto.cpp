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

#include "asylum/puzzles/boardkeyhidesto.h"

#include "asylum/system/cursor.h"

#include "asylum/asylum.h"

namespace Asylum {

static const PuzzleBoard::PuzzleData puzzleKeyHidesToData[] = {
	// English
	{
		56,
		kGameFlag283,
		503,
		3,
		{{5, false}, {6, false}, {7, false}},
		10,
		{
			{'I',   30,  53},
			{'E',  212,  71},
			{'D',   31, 103},
			{'H',  447, 134},
			{'S',  240, 151},
			{'E',   95, 167},
			{'O',  372, 182},
			{'K',  210, 215},
			{'Y',  440, 247},
			{'T',  479, 262},
			{'\0',   0,   0},
		},
		true,
		6, 18,
		"K E Y   H I D E S   T O "
	},
	// German
	{
		56,
		kGameFlag283,
		503,
		3,
		{{5, false}, {6, false}, {7, false}},
		9,
		{
			{'U',  52,  50},
			{'R',  28,  66},
			{'Z', 254,  66},
			{'E',  40,  82},
			{'W', 209, 130},
			{'G',  61, 146},
			{'T',  33, 162},
			{'I', 527, 162},
			{'S', 233, 210},
			{'\0',  0,   0},
			{'\0',  0,   0}
		},
		true,
		6, 14,
		"W E G   I S T   Z U R "
	},
	// French
	{
		56,
		kGameFlag283,
		431,
		3,
		{{5, false}, {6, false}, {7, false}},
		6,
		{
			{'L',  357,   66},
			{'A',  128,   98},
			{'C',  367,  114},
			{'L',  237,  130},
			{'E',   25,  162},
			{'F',   47,  194},
			{'\0',   0,    0},
			{'\0',   0,    0},
			{'\0',   0,    0},
			{'\0',   0,    0},
			{'\0',   0,    0}
		},
		true,
		4, 0,
		"L A   C L E F "
	}
};

PuzzleBoardKeyHidesTo::PuzzleBoardKeyHidesTo(AsylumEngine *engine) : PuzzleBoard(engine, puzzleKeyHidesToData) {
}

void PuzzleBoardKeyHidesTo::saveLoadWithSerializer(Common::Serializer &s) {
	for (int32 i = 0; i < 12; i++)
		s.syncAsUint32LE(_charUsed[i]);

	s.syncBytes((byte *)&_solvedText, 28);

	s.syncAsUint32LE(_position);
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleBoardKeyHidesTo::mouseLeftDown(const AsylumEvent &) {
	Common::Point mousePos = getCursor()->position();

	if (mousePos.y <= 350) {
		int32 index = findRect();

		if (index != -1 && _position < strlen(_data.solvedText)) {
			_charUsed[index] = true;
			_selectedSlot = -1;

			_solvedText[_position++] = _data.charMap[index].character;
			_solvedText[_position++] = ' ';

			if (_position == _data.space1Pos || _position == _data.space2Pos) {
				_solvedText[_position++] = ' ';
				_solvedText[_position++] = ' ';
			}

			updateScreen();
		}
	} else if (_vm->isGameFlagNotSet(kGameFlag283)) {
		checkSlots();
	}

	return true;
}

} // End of namespace Asylum
