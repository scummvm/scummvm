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

#ifndef SHERLOCK_DARTS_H
#define SHERLOCK_DARTS_H

#include "sherlock/resources.h"

namespace Sherlock {

namespace Scalpel {

class ScalpelEngine;

class Darts {
private:
	ScalpelEngine *_vm;
	ImageFile *_dartImages;
	int _dartScore1, _dartScore2;
	int _roundNumber;
	int _level;
	int _computerPlayer;
	Common::String _opponent;
	bool _playerDartMode;
	int _roundScore;
	bool _oldDartButtons;

	void loadDarts();
	void initDarts();
	void closeDarts();

	void showNames(int playerNum);
	void showStatus(int playerNum);

	int throwDart(int dartNum, int computer);
	void drawDartThrow(const Common::Point &pt);

	void erasePowerBars();
	int doPowerBar(const Common::Point &pt, byte color, int goToPower, bool isVertical);

	bool dartHit();
	int dartScore(const Common::Point &pt);

	Common::Point getComputerDartDest(int playerNum);

	bool findNumberOnBoard(int aim, Common::Point &pt);

	void setFlagsForDarts(int flagNum);
public:
	Darts(ScalpelEngine *vm);

	void playDarts();
};

} // End of namespace Scalpel

} // End of namespace Sherlock

#endif
