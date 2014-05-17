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

#ifndef HOPKINS_COMPUTER_H
#define HOPKINS_COMPUTER_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/rect.h"

namespace Hopkins {

class HopkinsEngine;

enum ComputerEnum { COMPUTER_HOPKINS = 1, COMPUTER_SAMANTHA = 2, COMPUTER_PUBLIC = 3 };

class ComputerManager {
private:
	HopkinsEngine *_vm;

	struct MenuItem {
		int  _lineSize;
		char _line[90];
	};

	struct ScoreItem {
		Common::String _name;
		Common::String _score;
	};

	MenuItem _menuText[50];
	char _inputBuf[200];
	ScoreItem _score[6];
	int _textColor;
	Common::Point _textPosition;
	Common::Point _ballPosition;
	byte *_breakoutSpr;
	int16 *_breakoutLevel;
	int _breakoutBrickNbr;
	int _breakoutScore;
	int _breakoutLives;
	int _breakoutSpeed;
	bool _ballRightFl;
	bool _ballUpFl;
	int _breakoutLevelNbr;
	int _padPositionX;
	int _lowestHiScore;
	int _minBreakoutMoveSpeed;
	int _maxBreakoutMoveSpeed;
	int _lastBreakoutMoveSpeed;

	void loadMenu();
	void restoreFBIRoom();
	void setVideoMode();
	void setTextMode();
	void clearScreen();
	void setTextColor(int col);
	void setTextPosition(int yp, int xp);
	void outText(const Common::String &msg);
	void outText2(const Common::String &msg);
	void readText(int idx);
	void loadHiscore();
	void newLevel();
	void setModeVGA256();
	void displayLives();
	void displayBricks();
	void displayGamesSubMenu();
	int displayHiscores();
	void displayHiscoreLine(const byte *objectData, int x, int y, int curChar);
	void displayMessage(int xp, int yp, int textIdx);
	void displayScore();
	void displayScoreChar(int charPos, int charDisp);
	void getScoreName();
	void playBreakout();
	int moveBall();
	void saveScore();
	void checkBallCollisions();

public:
	ComputerManager(HopkinsEngine *vm);

	void showComputer(ComputerEnum mode);
};

} // End of namespace Hopkins

#endif /* HOPKINS_COMPUTER_H */
