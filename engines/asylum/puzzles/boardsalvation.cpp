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

#include "asylum/puzzles/boardsalvation.h"

#include "asylum/system/cursor.h"
#include "asylum/system/screen.h"
#include "asylum/system/sound.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"

namespace Asylum {

static const PuzzleBoard::PuzzleData puzzleSalvationData = {
	31,
	kGameFlag281,
	431,
	3,
	{{0, false}, {1, false}, {2, false}},
	9,
	{{'I', Common::Point( 61,  53)},
	{'S', Common::Point(322,  53)},
	{'A', Common::Point(529,  86)},
	{'L', Common::Point(256, 117)},
	{'V', Common::Point(251, 151)},
	{'A', Common::Point( 66, 199)},
	{'T', Common::Point(436, 229)},
	{'O', Common::Point(172, 262)},
	{'N', Common::Point(393, 296)}},
	false,
	"S A L V A T I O N "
};

static const uint32 puzzleSalvationSoundResourceIndex[11] = {5, 6, 7, 10, 11, 28, 29, 30, 31, 32, 36};

PuzzleBoardSalvation::PuzzleBoardSalvation(AsylumEngine *engine) : PuzzleBoard(engine, puzzleSalvationData) {
}

void PuzzleBoardSalvation::saveLoadWithSerializer(Common::Serializer &s) {
	for (int32 i = 0; i < 9; i++)
		s.syncAsUint32LE(_charUsed[i]);

	s.syncBytes((byte *)&_solvedText, 20);

	s.syncAsUint32LE(_position);
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleBoardSalvation::mouseLeftDown(const AsylumEvent &) {
	Common::Point mousePos = getCursor()->position();

	if (mousePos.y <= 350) {
		int32 index = findRect();

		if (index != -1 && _position < 18) {
			_charUsed[index] = true;
			_selectedSlot = -1;

			_solvedText[_position++] = puzzleSalvationData.charMap[index].character;
			_solvedText[_position++] = ' ';

			updateScreen();
		}
	} else if (_vm->isGameFlagNotSet(kGameFlag281)) {
		checkSlots();
	}

	return true;
}

bool PuzzleBoardSalvation::mouseRightDown(const AsylumEvent &) {
	if (!stopSound()) {
		checkANALText();
		getScreen()->clear();
		_vm->switchEventHandler(getScene());
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////
void PuzzleBoardSalvation::checkANALText() {
	if (!strcmp(_solvedText, "A N A L "))
		getSound()->playSound(MAKE_RESOURCE(kResourcePackSpeech, puzzleSalvationSoundResourceIndex[rnd(11)]), false, Config.voiceVolume);
}

} // End of namespace Asylum
