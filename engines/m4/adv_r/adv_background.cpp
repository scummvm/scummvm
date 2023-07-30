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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "m4/adv_r/adv_background.h"
#include "m4/adv_r/adv_file.h"
#include "m4/vars.h"

namespace M4 {

void adv_freeCodes() {
	if (_G(screenCodeBuff)) {
		delete _G(screenCodeBuff);
		_G(screenCodeBuff) = nullptr;
	}
}

void adv_freeBackground() {
	if (_G(game_bgBuff)) {
		delete _G(game_bgBuff);
		_G(game_bgBuff) = nullptr;
	}
}

bool adv_restoreBackground() {
	RGB8 myPalette[256];
	SysFile sysFile(_G(currBackgroundFN));

	if (load_background(&sysFile, &_G(game_bgBuff), myPalette)) {
		sysFile.close();
		return true;
	} else {
		return false;
	}
}

bool adv_restoreCodes() {
	SysFile sysFile(_G(currCodeFN));

	_G(screenCodeBuff) = load_codes(&sysFile);
	if (_G(screenCodeBuff)) {
		sysFile.close();
		return true;
	} else {
		return false;
	}

	return true;
}

} // End of namespace M4
