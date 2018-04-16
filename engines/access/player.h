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

#ifndef ACCESS_PLAYER_H
#define ACCESS_PLAYER_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "common/serializer.h"
#include "access/asurface.h"
#include "access/data.h"

namespace Access {

enum Direction {
	NONE = 0,
	UP = 1,
	DOWN = 2,
	LEFT = 3,
	RIGHT = 4,
	UPRIGHT = 5,
	DOWNRIGHT = 6,
	UPLEFT = 7,
	DOWNLEFT = 8
};

class AccessEngine;

class Player : public ImageEntry, public Manager {
protected:
	int _leftDelta, _rightDelta;
	int _upDelta, _downDelta;
	int _scrollConst;
	int _sideWalkMin, _sideWalkMax;
	int _upWalkMin, _upWalkMax;
	int _downWalkMin, _downWalkMax;
	int _diagUpWalkMin, _diagUpWalkMax;
	int _diagDownWalkMin, _diagDownWalkMax;
	SpriteResource *_playerSprites1;
	int _scrollEnd;
	int _inactiveYOff;

	void plotCom(int v1);
	void plotCom0();
	void plotCom1();
	void plotCom2();
	void plotCom3();

	void walkUp();
	void walkDown();
	void walkLeft();
	void walkRight();
	void walkUpLeft();
	void walkDownLeft();
	void walkUpRight();
	void walkDownRight();
	void checkScrollUp();
public:
	Direction _playerDirection;
	SpriteResource *_playerSprites;
	// Fields in original Player structure
	byte *_manPal1;
	int *_walkOffRight;
	int *_walkOffLeft;
	int *_walkOffUp;
	int *_walkOffDown;
	Common::Point *_walkOffUR;
	Common::Point *_walkOffDR;
	Common::Point *_walkOffUL;
	Common::Point *_walkOffDL;
	byte _rawTempL;
	int _rawXTemp;
	byte _rawYTempL;
	int _rawYTemp;
	Common::Point _playerOffset;
	int _playerXLow;
	int _playerX;
	int _playerYLow;
	int _playerY;
	int _frame;
	int _xFlag, _yFlag;
	Direction _move;

	// Additional public globals we've added to new Player class
	bool _playerOff;
	bool _playerMove;
	Common::Point _moveTo;
	bool _collideFlag;
	bool _scrollFlag;
	int _scrollThreshold;
	int _scrollAmount;

	// Additional globals that need to be saved
	int _roomNumber;
	Common::Point _rawPlayerLow;
	Common::Point _rawPlayer;
public:
	Player(AccessEngine *vm);
	virtual ~Player();
	static Player *init(AccessEngine *vm);

	virtual void load();

	void loadTexPalette();

	void loadSprites(const Common::String &name);

	void freeSprites();

	void removeSprite1();

	void calcManScale();

	void walk();

	void calcPlayer();

	bool scrollUp(int forcedAmount = -1);
	bool scrollDown(int forcedAmount = -1);
	bool scrollLeft(int forcedAmount = -1);
	bool scrollRight(int forcedAmount = -1);
	void checkScroll();

	void checkMove();

	/**
	* Synchronize savegame data
	*/
	void synchronize(Common::Serializer &s);
};

} // End of namespace Access

#endif /* ACCESS_PLAYER_H */
