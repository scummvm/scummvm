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

#include "immortal/immortal.h"

namespace Immortal {

bool ImmortalEngine::trapKeys() {
	// This weirdly named routine just checks if you want to restart the game. On the NES it pulls up a dialog with a yes/no,
	// But on the Apple IIGS it's just the R key
	getInput();
	if (_pressedAction == kActionRestart) {
		gameOver();
		return true;
	} else if (_pressedAction == kActionSound) {
		//toggleSound();
	}
	return false;
}

// There's no way this routine needs to still be here. In fact I'm not sure it needed to be in the game anyway?
int ImmortalEngine::getLevel() {
	return _level;
}

void ImmortalEngine::logicInit() {
	_titlesShown = 0;
	_time = 0;
	_promoting = 0;
	_restart = 1;
	//level_initAtStartOfGameOnly
	_lastCertLen = 0;
}

void ImmortalEngine::logic() {
	_time += 1;
	// if time overflows the counter, inc the high byte? What the heck...

}

void ImmortalEngine::restartLogic() {
}

int ImmortalEngine::logicFreeze() {
	// Very silly way of checking if the level is over and/or the game is over
	int g = _gameOverFlag | _levelOver;
	return (g ^ 1) >> 1;
}

void ImmortalEngine::gameOverDisplay() {
	_themePaused |= 2;
	//text_print(kGameOverString)
}

void ImmortalEngine::gameOver() {
	_gameOverFlag = 1;
}

void ImmortalEngine::levelOver() {
	_levelOver = 1;
}

} // namespace Immortal



















