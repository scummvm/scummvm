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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef HOPKINS_COMPUTER_H
#define HOPKINS_COMPUTER_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "common/str.h"

namespace Hopkins {

class HopkinsEngine;

static char _englishText[] = 
    "% ****** FBI COMPUTER NUMBER 4985 ****** J.HOPKINS COMPUTER ******\n"
    "% ****** FBI COMPUTER NUMBER 4998 ****** S.COLLINS COMPUTER ******\n"
	"% ****** FBI COMPUTER NUMBER 4997 ****** ACCES FREE COMPUTER ******\n"
	"% PASSWORD IS: ALLFREE\n% ENTER CURRENT PASSWORD\n"
	"% ****** ACCES DENIED ******\n"
	"% 1) *** GAME ***\n"
	"% 0) QUIT COMPUTER\n"
	"% 2) STRANGE CADAVER\n"
	"% 3) STRANGE CADAVER\n"
	"% 4) SENATOR FERGUSSON\n"
	"% 5) DOG KILLER\n"
	"% 2) SCIENTIST KIDNAPPED.\n"
	"% 3) SCIENTIST KIDNAPPED (next).\n"
	"% 4) SCIENTIST KIDNAPPED (next).\n"
	"% 5) SCIENTIST KIDNAPPED (next).\n"
	"% 6) SCIENTIST KIDNAPPED (next).\n"
	"%% fin\n";
static char _frenchText[] =  
    "% ****** FBI COMPUTER NUMBER 4985 ****** J.HOPKINS COMPUTER ******\n"
	"% ****** FBI COMPUTER NUMBER 4998 ****** S.COLLINS COMPUTER ******\n"
	"% ****** FBI COMPUTER NUMBER 4997 ****** ACCES FREE COMPUTER ******\n"
	"% PASSWORD IS: ALLFREE\n"
	"% ENTER CURRENT PASSWORD\n"
	"% ****** ACCES DENIED ******\n"
	"% 1) *** CASSE BRIQUE ***\n"
	"% 0) QUITTER L'ORDINATEUR\n"
	"% 2) CADAVRE SANS TETE\n"
	"% 3) CADAVRE SANS TETE\n"
	"% 4) AGRESSION DU SENATEUR\n"
	"% 5) LES CHIENS TUEURS\n"
	"% 2) DISPARITIONS DE CHERCHEURS.\n"
	"% 3) DISPARITIONS (suite).\n"
	"% 4) DISPARITIONS (suite).\n"
	"% 5) DISPARITIONS (suite).\n"
	"% 6) DISPARITIONS (suite).\n"
	"%% fin\n";

struct MenuItem {
	bool _actvFl;
	int  _lineSize;
	char _line[90];
};

struct ScoreItem {
	Common::String _name;
	Common::String _score;
};

enum ComputerEnum { COMPUTER_HOPKINS = 1, COMPUTER_SAMANTHAS = 2, COMPUTER_PUBLIC = 3 };

class ComputerManager {
private:
	HopkinsEngine *_vm;
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
	int _breakoutHiscore;
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
	void displayScore();
	int  displayHiscores();
	void displayHiscoreLine(byte *objectData, int x, int y, int curChar);
	void playBreakout();
	void saveScore();
	int moveBall();
	void checkBallCollisions();
	void getScoreName();

	void TXT4(int xp, int yp, int textIdx);

	void IMPSCORE(int charPos, int charDisp);

public:
	ComputerManager();
	void setParent(HopkinsEngine *vm);
	void showComputer(ComputerEnum mode);
};

} // End of namespace Hopkins

#endif /* HOPKINS_COMPUTER_H */
