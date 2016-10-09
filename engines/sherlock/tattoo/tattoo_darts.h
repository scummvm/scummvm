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
	int _roundScore;
	int _level;
	int _compPlay;
	Common::String _opponent;
	int _spacing;
	bool _oldDartButtons;
	int _handX;
	Common::Point _handSize;
	bool _escapePressed;

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

	/**
	 * Show the player names
	 */
	void showNames(int playerNum);

	/**
	 * Show the current scores
	 */
	void showStatus(int playerNum);

	/**
	 * Erases the power bars
	 */
	void erasePowerBars();

	/**
	 * Returns true if a mouse button or key is pressed
	 */
	bool dartHit();

	/**
	 * Shows a power bar and increments it until a key or mouse button is pressed. If the bar
	 * reaches the end, it will also end. The reached power bar number is returned.
	 * @param pt			Bar position
	 * @param color			draw color
	 * @param goToPower		If provided, input is ignored, and the bar is increased up to the specified level
	 * @param orientation	0=Horizontal, 1=Vertical
	 */
	int doPowerBar(const Common::Point &pt, byte color, int goToPower, int orientation);

	/**
	 * This is similar to doPowerBar, except it draws the player's hand moving across the
	 * bottom of the screen to indicate the positioning of the darts
	 */
	int drawHand(int goToPower, int computer);

	/**
	 * Converts a passed co-ordinates from screen co-ordinates to an offset within the dartboard
	 */
	Common::Point convertFromScreenToScoreCoords(const Common::Point &pt) const;

	/**
	 * Return the score a dart at the given position will get
	 */
	int dartScore(const Common::Point &pt);

	/**
	 * Draw a dart travelling to the board
	 */
	void drawDartThrow(const Common::Point &dartPos, int computer);

	/**
	 * Looks for the passed number on the dartboard. If it finds it, it will return
	 * the co-ordinates of the center of the number
	 */
	int findNumberOnBoard(int aim, Common::Point &pt);

	/**
	 * Calculates a position for the comptuer wants to throw, and then calculates where they
	 * actually did throw. The computer will not always hit what it's aiming it.
	 */
	void getComputerNumber(int playerNum, Common::Point &targetPos);

	/**
	 * Throw one dart. If computer is 1 or 2, the computer will throw the dart, and user input
	 * will be ignored.
	 * @param computer		1=1st computer player, 2=2nd computer player
	 */
	int throwDart(int dartNum, int computer);

	/**
	 * This will update the number of hits for the target score, as well as updating the
	 * score if it's closed
	 */
	void doCricketScoreHits(int player, int scoreIndex, int numHits);

	/**
	 * Updates the score based upon what the dart hit
	 */
	void updateCricketScore(int player, int dartVal, int multiplier);

	/**
	 * Draw the darts left
	 */
	void drawDartsLeft(int dartNum, int computer);
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
