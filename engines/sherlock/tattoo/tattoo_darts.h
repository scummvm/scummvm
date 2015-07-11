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

#ifndef SHERLOCK_TATTOO_DARTS_H
#define SHERLOCK_TATTOO_DARTS_H

#include "common/scummsys.h"
#include "sherlock/image_file.h"

namespace Sherlock {

class SherlockEngine;

namespace Tattoo {

enum GameType { GAME_301, GAME_CRICKET, GAME_501 };

class Darts {
private:
	SherlockEngine *_vm;
	GameType _gameType;
	ImageFile *_hand1, *_hand2;
	ImageFile *_dartGraphics;
	ImageFile *_dartsLeft;
	ImageFile *_dartMap;
	ImageFile *_dartBoard;
	Common::Rect _dartInfo;
	int _cricketScore[2][7];
	int _score1, _score2;
	int _roundNum;
	int _level;
	int _compPlay;
	Common::String _opponent;

	/**
	 * Initialize game variables
	 */
	void initDarts();

	/**
	 * Load dartboard graphics
	 */
	void loadDarts();

	/**
	 * Free loaded dart images
	 */
	void closeDarts();
public:
	Darts(SherlockEngine *vm);

	/**
	 * Play the darts game
	 */
	void playDarts(GameType gameType);
};

} // End of namespace Tattoo

} // End of namespace Sherlock

#endif
