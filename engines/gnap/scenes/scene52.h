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

#ifndef GNAP_SCENE52_H
#define GNAP_SCENE52_H

#include "gnap/debugger.h"
#include "gnap/scenes/scenecore.h"

namespace Gnap {

class GnapEngine;

class Scene52: public Scene {
public:
	Scene52(GnapEngine *vm);
	~Scene52() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations() {};
	virtual void updateAnimationsCb() {};

private:
	int _s52_liveAlienRows;
	int _s52_gameScore;
	bool _s52_soundToggle;
	int _s52_arcadeScreenLeft;
	int _s52_arcadeScreenRight;
	int _s52_arcadeScreenBottom;
	int _s52_shipsLeft;
	int _s52_shieldSpriteIds[3];
	int _s52_shieldPosX[3];
	int _s52_shipPosX;
	int _s52_shipCannonPosX, _s52_shipCannonPosY;
	bool _s52_shipCannonFiring;
	bool _s52_shipCannonFired; 
	int _s52_shipCannonWidth, _s52_shipCannonHeight;
	int _s52_shipCannonTopY;
	int _s52_shipMidX, _s52_shipMidY;
	bool _s52_shipFlag;
	bool _s52_aliensInitialized;
	int _s52_alienSpeed, _s52_alienDirection;
	int _s52_alienWidth, _s52_alienHeight;
	int _s52_alienLeftX, _s52_alienTopY;
	int _s52_alienRowDownCtr;
	int _s52_alienRowKind[7];
	int _s52_alienRowAnims[7];
	int _s52_alienRowIds[7];
	int _s52_alienRowXOfs[7];
	int _s52_alienCannonFired[3];
	int _s52_alienCannonPosX[3];
	int _s52_alienCannonPosY[3];
	int _s52_alienCannonSequenceIds[3];
	int _s52_alienCannonIds[3];
	bool _s52_alienWave, _s52_alienSingle;
	int _s52_alienCounter;
	bool _s52_bottomAlienFlag;
	int _s52_aliensCount;
	int _s52_items[8][5];
	int _s52_nextUfoSequenceId, _s52_ufoSequenceId;

	void update();
	void initShipCannon(int bottomY);
	void initAlienCannons();
	void fireShipCannon(int posX);
	void fireAlienCannon();
	int getFreeShipCannon();
	int getFreeAlienCannon();
	void updateShipCannon();
	void updateAlienCannons();
	void initAliens();
	void initAlienRowKind(int rowNum, int alienKind);
	void insertAlienRow(int rowNum);
	void insertAlienRowAliens(int rowNum);
	void updateAlienRow(int rowNum);
	void moveDownAlienRow();
	int updateHitAlien();
	int getHitAlienNum(int rowNum);
	int alienCannonHitShip(int cannonNum);
	int alienCannonHitShield(int cannonNum);
	bool shipCannonHitShield(int cannonNum);
	bool shipCannonHitAlien();
	void shipExplode();
	bool checkAlienRow(int rowNum);
	void updateAlienRowXOfs();
	void initAlienSize();
	void playSound();
	void updateAliens();
	void updateAlien(int rowNum);
	void loseShip();
	void initShields();
	void initAnims();
	void drawScore(int score);
};

} // End of namespace Gnap
#endif // GNAP_SCENE52_H
