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

CrimsonCrownGame::CrimsonCrownGame() : ComprehendGameV1(),
		_diskNum(1), _newDiskNum(1) {
	setupDisk(1);
}

void CrimsonCrownGame::setupDisk(uint diskNum) {
	assert(diskNum == 1 || diskNum == 2);

	_gameDataFile = Common::String::format("cc%u.gda", diskNum);

	_stringFiles.clear();
	_stringFiles.push_back(Common::String::format("ma.ms%u", diskNum).c_str());

	_locationGraphicFiles.clear();
	_locationGraphicFiles.push_back(Common::String::format("ra.ms%u", diskNum));
	_locationGraphicFiles.push_back(Common::String::format("rb.ms%u", diskNum));
	if (diskNum == 1)
		_locationGraphicFiles.push_back("RC.ms1");

	_itemGraphicFiles.clear();
	_itemGraphicFiles.push_back(Common::String::format("oa.ms%u", diskNum));
	_itemGraphicFiles.push_back(Common::String::format("ob.ms%u", diskNum));

	if (diskNum == 1)
		_gameStrings = &CC1_STRINGS;
	else
		_gameStrings = nullptr;

	_titleGraphicFile = "cctitle.ms1";
	_diskNum = diskNum;
}

void CrimsonCrownGame::beforeGame() {
	// Draw the title
	g_comprehend->drawPicture(TITLE_IMAGE);
	g_comprehend->readChar();
}

void CrimsonCrownGame::synchronizeSave(Common::Serializer &s) {
	if (s.isSaving()) {
		s.syncAsByte(_diskNum);
	} else {
		// Get the disk the save is for. The beforeTurn call allows
		// for the currently loaded disk to be switched if necessary
		s.syncAsByte(_newDiskNum);
		beforeTurn();
	}

	ComprehendGame::synchronizeSave(s);
}

void CrimsonCrownGame::handleSpecialOpcode() {
	switch (_specialOpcode) {
	case 1:
		// Crystyal ball cutscene
		if (_diskNum == 1) {
			crystalBallCutscene();
		} else {
			throneCutscene();
		}
		break;

	case 3:
		// Game over - failure
		game_restart();
		break;

	case 5:
		if (_diskNum == 1) {
			// Finished disk 1
			g_comprehend->readChar();
			g_comprehend->drawLocationPicture(41);
			console_println(_strings2[26].c_str());
			g_comprehend->readChar();

			_newDiskNum = 2;
			move_to(21);
			console_println(_strings[407].c_str());

		} else {
			// Won the game
			g_comprehend->drawLocationPicture(29, false);
			g_comprehend->drawItemPicture(20);
			console_println(stringLookup(0x21c).c_str());
			console_println(stringLookup(0x21d).c_str());

			g_comprehend->readChar();
			g_comprehend->quitGame();
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

void CrimsonCrownGame::throneCutscene() {
	// Show the screen
	update();
	console_println(stringLookup(0x20A).c_str());

	// Handle what happens in climatic showdown
	eval_function(14, nullptr);
}

void CrimsonCrownGame::beforePrompt() {
	// Clear the Sabrina/Erik action flags
	_flags[0xa] = 0;
	_flags[0xb] = 0;
}

void CrimsonCrownGame::beforeTurn() {
	if (_newDiskNum != _diskNum) {
		setupDisk(_newDiskNum);
		loadGame();
		move_to(_currentRoom);
	}

	ComprehendGameV1::beforeTurn();
}

bool CrimsonCrownGame::handle_restart() {
	if (_diskNum != 1) {
		setupDisk(1);
		loadGame();
	}

	return ComprehendGame::handle_restart();
}

} // namespace Comprehend
} // namespace Glk
