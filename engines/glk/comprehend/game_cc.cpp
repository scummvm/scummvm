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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/comprehend/game_cc.h"
#include "glk/comprehend/comprehend.h"
#include "glk/comprehend/pics.h"

namespace Glk {
namespace Comprehend {

static const GameStrings CC1_STRINGS = {0x9};

CrimsonCrownGame::CrimsonCrownGame() : ComprehendGame(), _diskNum(1) {
	setupDisk(1);
}

void CrimsonCrownGame::setupDisk(uint diskNum) {
	assert(diskNum == 1 || diskNum == 2);

	_gameDataFile = Common::String::format("cc%u.gda", diskNum);
	_stringFiles.push_back(StringFile(Common::String::format("ma.ms%u", diskNum), 0x89));
	_locationGraphicFiles.push_back(Common::String::format("ra.ms%u", diskNum));
	_locationGraphicFiles.push_back(Common::String::format("rb.ms%u", diskNum));
	if (diskNum == 1)
		_locationGraphicFiles.push_back("RC.ms1");
	_itemGraphicFiles.push_back(Common::String::format("oa.ms%u", diskNum));
	_itemGraphicFiles.push_back(Common::String::format("ob.ms%u", diskNum));

	if (diskNum == 1)
		_gameStrings = &CC1_STRINGS;
	else
		_gameStrings = nullptr;

	_titleGraphicFile = "cctitle.ms1";
}

void CrimsonCrownGame::beforeGame() {
	// Draw the title
	g_comprehend->drawPicture(TITLE_IMAGE);
	g_comprehend->readChar();
}

void CrimsonCrownGame::synchronizeSave(Common::Serializer &s) {
	ComprehendGame::synchronizeSave(s);
	s.syncAsByte(_diskNum);
}

void CrimsonCrownGame::handleSpecialOpcode(uint8 operand) {
	switch (operand) {
	case 1:
		// Crystyal ball cutscene
		assert(_diskNum == 1);
		crystalBallCutscene();
		break;

	case 3:
		// Game over - failure
		setupDisk(1);
		game_restart();
		break;

	case 5:
		if (_diskNum == 1) {
			// Finished disk 1
			error("[Completed disk 1 - handle switch to disk 2]");
		} else {
			// Won the game.
			// FIXME: The merchant ship should arrives, etc.
			game_restart();
		}
		break;

	case 6:
		game_save();
		break;

	case 7:
		game_restore();
		break;

	default:
		break;
	}
}

void CrimsonCrownGame::crystalBallCutscene() {
	g_comprehend->showGraphics();

	for (int screenNum = 38; screenNum <= 40; ++screenNum) {
		g_comprehend->drawLocationPicture(screenNum);
		g_comprehend->readChar();
		if (g_comprehend->shouldQuit())
			return;
	}
} 

void CrimsonCrownGame::beforePrompt() {
	// Clear the Sabrina/Erik action flags
	_flags[0xa] = 0;
	_flags[0xb] = 0;
}

} // namespace Comprehend
} // namespace Glk
