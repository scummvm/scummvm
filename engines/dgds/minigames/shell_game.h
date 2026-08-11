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

#ifndef DGDS_MINIGAMES_SHELL_GAME_H
#define DGDS_MINIGAMES_SHELL_GAME_H

#include "dgds/image.h"

namespace Dgds {

/** Native code for the shell game from Heart of China. */
class ShellGame {
public:
	ShellGame();

	void shellGameTick();
	void shellGameEnd();

private:
	void init();
	void drawShellGameStr(int16 count, int16 x, int16 y) const;
	void drawShells() const;
	void swapShells(bool flag);
	void revealPea(bool flag);
	void update();
	bool checkDistract();
	void setupSwap();

	Common::SharedPtr<Image> _shellGameImg;
	uint16 _revealPeaStep;
	uint16 _currentPeaPosition;
	bool _lastPass;
	uint16 _distractStep;
	uint16 _distractDelay;
	uint16 _state13Counter;
	int16 _swapPea1;
	int16 _swapPea2;
	uint16 _lastSwapPea1;
	uint16 _swapStatus;
	int16 _swapMoveDist;
	uint16 _swapMoveStep;
	uint16 _swapCount;
	int16 _reverseDirection;
	bool _clockwise;
};

} // end namespace Dgds

#endif // DGDS_MINIGAMES_SHELL_GAME_H
