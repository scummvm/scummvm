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

#ifndef GNAP_ARCADE_H
#define GNAP_ARCADE_H

#include "gnap/debugger.h"

namespace Gnap {

class GnapEngine;
class CutScene;

struct Scene49Obstacle {
	int _currSequenceId;
	int _closerSequenceId;
	int _passedSequenceId;
	int _splashSequenceId;
	int _collisionSequenceId;
	int _prevId;
	int _currId;
	int _laneNum;
};

struct ObstacleDef {
	int _sequenceId;
	int _ticks;
};

class Scene49: public Scene {
public:
	Scene49(GnapEngine *vm);
	~Scene49() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {};

private:
	int _scoreBarPos;
	int _scoreLevel;
	bool _scoreBarFlash;
	int _obstacleIndex;
	Scene49Obstacle _obstacles[5];
	int _truckSequenceId;
	int _truckId;
	int _truckLaneNum;

	void checkObstacles();
	void updateObstacle(int id);
	void increaseScore(int amount);
	void decreaseScore(int amount);
	void refreshScoreBar();
	void clearObstacle(int index);
};

/*****************************************************************************/

class Scene50: public Scene {
public:
	Scene50(GnapEngine *vm);
	~Scene50() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {};

private:
	bool _fightDone;
	int _timesPlayed;
	int _timesPlayedModifier;
	int _attackCounter;
	int _roundNum;
	int _timeRemaining;
	int _leftTongueRoundsWon;
	int _rightTongueRoundsWon;
	int _leftTongueEnergyBarPos;
	int _rightTongueEnergyBarPos;
	int _leftTongueSequenceId;
	int _leftTongueId;
	int _leftTongueNextSequenceId;
	int _leftTongueNextId;
	int _leftTongueNextIdCtr;
	int _rightTongueSequenceId;
	int _rightTongueId;
	int _rightTongueNextSequenceId;
	int _rightTongueNextId;
	int _rightTongueNextIdCtr;
	int _leftTongueEnergy;
	int _rightTongueEnergy;

	bool tongueWinsRound(int tongueNum);
	void playWinAnim(int tongueNum, bool fightOver);
	void delayTicks();
	void initRound();
	bool updateCountdown();
	void drawCountdown(int value);
	void playTonguesIdle();
	void playRoundAnim(int roundNum);
	bool updateEnergyBars(int newLeftBarPos, int newRightBarPos);
	void waitForAnim(int animationIndex);
	int checkInput();
	int getRightTongueAction();
	int getRightTongueActionTicks();
	int getLeftTongueNextId();
	int getRightTongueNextId();
	void playWinBadgeAnim(int tongueNum);
};

/*****************************************************************************/

struct Scene51Item {
	int _currSequenceId;
	int _droppedSequenceId;
	int _x, _y;
	int _collisionX;
	bool _canCatch;
	bool _isCollision;
	int _x2;
	int _id;
};

class Scene51: public Scene {
public:
	Scene51(GnapEngine *vm);
	~Scene51() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override {};
	void updateAnimationsCb() override {};

private:
	bool _dropLoseCash;

	int _cashAmount;
	int _digits[4];
	int _digitSequenceIds[4];
	int _guySequenceId;
	int _guyNextSequenceId;
	int _itemsCaughtCtr;
	int _dropSpeedTicks;
	int _nextDropItemKind;
	int _itemInsertX;
	int _itemInsertDirection;
	int _platypusSequenceId;
	int _platypusNextSequenceId;
	int _platypusJumpSequenceId;
	int _itemsCtr;
	int _itemsCtr1;
	int _itemsCtr2;

	Scene51Item _items[6];

	void clearItem(Scene51Item *item);
	void dropNextItem();
	void updateItemAnimations();
	int checkCollision(int sequenceId);
	void updateItemAnimation(Scene51Item *item, int index);
	void removeCollidedItems();
	int itemIsCaught(Scene51Item *item);
	bool isJumpingRight(int sequenceId);
	bool isJumpingLeft(int sequenceId);
	bool isJumping(int sequenceId);
	void waitForAnim(int animationIndex);
	int getPosRight(int sequenceId);
	int getPosLeft(int sequenceId);
	void playIntroAnim();
	void updateGuyAnimation();
	int incCashAmount(int sequenceId);
	void winMinigame();
	void playCashAppearAnim();
	void updateCash(int amount);
	void drawDigit(int digit, int position);
	void initCashDisplay();
};

/*****************************************************************************/

class Scene52: public Scene {
public:
	Scene52(GnapEngine *vm);
	~Scene52() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override {};
	void updateAnimationsCb() override {};

private:
	int _liveAlienRows;
	int _gameScore;
	bool _soundToggle;
	int _arcadeScreenBottom;
	int _shipsLeft;
	int _shieldSpriteIds[3];
	int _shieldPosX[3];
	int _shipPosX;
	int _shipCannonPosX, _shipCannonPosY;
	bool _shipCannonFiring;
	bool _shipCannonFired;
	int _shipCannonWidth, _shipCannonHeight;
	int _shipCannonTopY;
	int _shipMidX, _shipMidY;
	bool _shipFlag;
	bool _aliensInitialized;
	int _alienSpeed, _alienDirection;
	int _alienWidth, _alienHeight;
	int _alienLeftX, _alienTopY;
	int _alienRowDownCtr;
	int _alienRowKind[8];
	int _alienRowAnims[8];
	int _alienRowIds[8];
	int _alienRowXOfs[8];
	int _alienCannonFired[3];
	int _alienCannonPosX[3];
	int _alienCannonPosY[3];
	int _alienCannonSequenceIds[3];
	int _alienCannonIds[3];
	bool _alienWave, _alienSingle;
	int _alienCounter;
	bool _bottomAlienFlag;
	int _aliensCount;
	int _items[8][5];
	int _nextUfoSequenceId, _ufoSequenceId;

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

#endif // GNAP_ARCADE_H
