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

#ifndef SHERLOCK_SCALPEL_DARTS_H
#define SHERLOCK_SCALPEL_DARTS_H

#include "sherlock/image_file.h"

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

	/**
	 * Load the graphics needed for the dart game
	 */
	void loadDarts();

	/**
	 * Initializes the variables needed for the dart game
	 */
	void initDarts();

	/**
	 * Frees the images used by the dart game
	 */
	void closeDarts();

	/**
	 * Show the names of the people playing, Holmes and his opponent
	 */
	void showNames(int playerNum);

	/**
	 * Show the player score and game status
	 */
	void showStatus(int playerNum);

	/**
	 * Throws a single dart.
	 * @param dartNum	Dart number
	 * @param computer	0 = Player, 1 = 1st player computer, 2 = 2nd player computer
	 * @returns			Score for what dart hit
	 */
	int throwDart(int dartNum, int computer);

	/**
	 * Draw a dart moving towards the board
	 */
	void drawDartThrow(const Common::Point &pt);

	/**
	 * Erases the power bars
	 */
	void erasePowerBars();

	/**
	 * Show a gradually incrementing incrementing power that bar. If goToPower is provided, it will
	 * increment to that power level ignoring all keyboard input (ie. for computer throws).
	 * Otherwise, it will increment until either a key/mouse button is pressed, or it reaches the end
	 */
	int doPowerBar(const Common::Point &pt, byte color, int goToPower, bool isVertical);

	/**
	 * Returns true if a mouse button or key is pressed.
	 */
	int dartHit();

	/**
	 * Return the score of the given location on the dart-board
	 */
	int dartScore(const Common::Point &pt);

	/**
	 * Calculates where a computer player is trying to throw their dart, and choose the actual
	 * point that was hit with some margin of error
	 */
	Common::Point getComputerDartDest(int playerNum);

	/**
	 * Returns the center position for the area of the dartboard with a given number
	 */
	bool findNumberOnBoard(int aim, Common::Point &pt);
public:
	Darts(ScalpelEngine *vm);

	/**
	 * Main method for playing darts game
	 */
	void playDarts();
};

} // End of namespace Scalpel

} // End of namespace Sherlock

#endif
