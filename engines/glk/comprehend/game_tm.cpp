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

#include "glk/comprehend/comprehend.h"
#include "glk/comprehend/game_tm.h"
#include "glk/comprehend/pics.h"
#include "common/md5.h"

namespace Glk {
namespace Comprehend {

TalismanGame::TalismanGame() : ComprehendGameV2() {
	_gameDataFile = "G0";

	_locationGraphicFiles.push_back("RA");
	_locationGraphicFiles.push_back("RB");
	_locationGraphicFiles.push_back("RC");
	_locationGraphicFiles.push_back("RD");
	_locationGraphicFiles.push_back("RE");
	_locationGraphicFiles.push_back("RF");
	_locationGraphicFiles.push_back("RG");
	_itemGraphicFiles.push_back("OA");
	_itemGraphicFiles.push_back("OB");
	_itemGraphicFiles.push_back("OE");
	_itemGraphicFiles.push_back("OF");

	_titleGraphicFile = "t0";
}

#define STRINGS_SEGMENT 0x16490
#define BANKS_COUNT 15
#define STRINGS_PER_BANK 64

void TalismanGame::loadStrings() {
	uint16 bankOffsets[BANKS_COUNT];
	uint16 stringOffsets[STRINGS_PER_BANK + 1];

	Common::File f;
	if (!f.open("novel.exe"))
		error("novel.exe is a required file");

	Common::String md5 = Common::computeStreamMD5AsString(f, 1024);
	if (md5 != "0e7f002971acdb055f439020363512ce")
		error("Unrecognised novel.exe encountered");

	f.seek(STRINGS_SEGMENT);
	for (int bank = 0; bank < BANKS_COUNT; ++bank)
		bankOffsets[bank] = f.readUint16LE();

	// Iterate through the banks loading the strings
	for (int bank = 0; bank < BANKS_COUNT; ++bank) {
		f.seek(STRINGS_SEGMENT + bankOffsets[bank]);
		for (int strNum = 0; strNum <= STRINGS_PER_BANK; ++strNum)
			stringOffsets[strNum] = f.readUint16LE();

		for (int strNum = 0; strNum < STRINGS_PER_BANK; ++strNum) {
			f.seek(STRINGS_SEGMENT + bankOffsets[bank] + stringOffsets[strNum]);
			FileBuffer fb(&f, stringOffsets[strNum + 1] - stringOffsets[strNum]);
			_strings.push_back(parseString(&fb));
		}
	}
}

void TalismanGame::beforeGame() {
	loadStrings();

	// Draw the title
	g_comprehend->drawPicture(TITLE_IMAGE);

	// Print game information
	console_println("Story by Bruce X.Hoffman. Graphics by Ray Redlich and Brian Poff");
	console_println("Project managed and IBM version by Jeffrey A. Jay. "
		"Copyright 1987 POLARWARE Inc.");
	g_comprehend->readChar();

	g_comprehend->glk_window_clear(g_comprehend->_bottomWindow);
}

} // namespace Comprehend
} // namespace Glk
