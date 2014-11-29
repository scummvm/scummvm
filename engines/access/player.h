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

#ifndef ACCESS_PLAYER_H
#define ACCESS_PLAYER_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "common/serializer.h"
#include "access/asurface.h"
#include "access/data.h"

namespace Access {

#define PLAYER_DATA_COUNT 8

enum Direction { NONE = 0, UP = 1, DOWN = 2, LEFT = 3, RIGHT = 4,
	UPRIGHT = 5, DOWNRIGHT = 6, UPLEFT = 7, DOWNLEFT = 8 };

class AccessEngine;

class Player: public ImageEntry, public Manager {
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
	byte *_manPal1;
	int _scrollEnd;
	int _inactiveYOff;

	void plotCom(int v1);
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
	bool scrollUp();
	bool scrollDown();
	bool scrollLeft();
	bool scrollRight();
public:
	Direction _playerDirection;
	SpriteResource *_playerSprites;
	// Fields in original Player structure
	byte *_monData;
	int _walkOffRight[PLAYER_DATA_COUNT];
	int _walkOffLeft[PLAYER_DATA_COUNT];
	int _walkOffUp[PLAYER_DATA_COUNT];
	int _walkOffDown[PLAYER_DATA_COUNT];
	Common::Point _walkOffUR[PLAYER_DATA_COUNT];
	Common::Point _walkOffDR[PLAYER_DATA_COUNT];
	Common::Point _walkOffUL[PLAYER_DATA_COUNT];
	Common::Point _walkOffDL[PLAYER_DATA_COUNT];
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

	void loadSprites(const Common::String &name);

	void freeSprites();

	void removeSprite1();

	void calcManScale();

	void walk();

	void calcPlayer();

	void checkScroll();

	void checkMove();

	/**
	* Synchronize savegame data
	*/
	void synchronize(Common::Serializer &s);
};

} // End of namespace Access

#endif /* ACCESS_PLAYER_H */
