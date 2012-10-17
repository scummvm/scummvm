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

#ifndef NEVERHOOD_MODULE1600_H
#define NEVERHOOD_MODULE1600_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"
#include "neverhood/module3000.h"

namespace Neverhood {

// Module1600

class Module1600 : public Module {
public:
	Module1600(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Module1600();
protected:
	void createScene(int sceneNum, int which);
	void updateScene();
};

class AsCommonCar : public AnimatedSprite {
public:
	AsCommonCar(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
	~AsCommonCar();
	void setPathPoints(NPointArray *pathPoints);
protected:
	Scene *_parentScene;
	NPointArray *_pathPoints;
	NRectArray *_rectList;
	int _newMoveDirection;
	int _currMoveDirection;
	int _exitDirection;
	int _currPointIndex;
	bool _hasAgainDestPoint;
	NPoint _againDestPoint;
	bool _hasAgainDestPointIndex;
	int _againDestPointIndex;
	bool _inMainArea;
	bool _isBraking;
	bool _isBusy;
	bool _isIdle;
	bool _isMoving;
	bool _rectFlag;
	int _idleCounter;
	int _idleCounterMax;
	int _steps;
	int _stepError;
	int _lastDistance;
	int _yMoveTotalSteps;
	int _ySteps;
	int _newDeltaXType;
	int _soundCounter;
	int _turnMoveStatus;
	int16 _destX, _destY;
	NPoint pathPoint(uint index) { return (*_pathPoints)[index]; }
	void update();
	void upIdle();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmAnimation(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmLeaveCar(int messageNum, const MessageParam &param, Entity *sender);
	void stCarAtHome();
	void updateTurnMovement();
	void updateMovement();
	void stEnterCar();
	void stLeaveCar();
	void stLeanForwardIdle();
	void evIdleDone();
	void stIdleBlink();
	void stHandleRect();
	void evHandleRectDone();
	void stUpdateMoveDirection();
	void stTurnCar();
	void moveToNextPoint();
	void stBrakeMoveToNextPoint();
	void stTurnCarMoveToNextPoint();
	void moveToPrevPoint();
	void stBrakeMoveToPrevPoint();
	void stTurnCarMoveToPrevPoint();
	void evTurnCarDone();
	void suMoveToNextPoint();
	void suMoveToPrevPoint();
	void updateSound();
};

class AsScene1608Door : public AnimatedSprite {
public:
	AsScene1608Door(NeverhoodEngine *vm, Scene *parentScene);
protected:
	Scene *_parentScene;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void stOpenDoor();
	void stOpenDoorDone();
	void stCloseDoor();
	void stCloseDoorDone();
};

class AsCommonIdleCarLower : public AnimatedSprite {
public:
	AsCommonIdleCarLower(NeverhoodEngine *vm, int16 x, int16 y);
};

class AsCommonIdleCarFull : public AnimatedSprite {
public:
	AsCommonIdleCarFull(NeverhoodEngine *vm, int16 x, int16 y);
};

class AsCommonCarConnector : public AnimatedSprite {
public:
	AsCommonCarConnector(NeverhoodEngine *vm, AsCommonCar *asCar);
protected:
	AsCommonCar *_asCar;
	void update();
};

class Scene1608 : public Scene {
public:
	Scene1608(NeverhoodEngine *vm, Module *parentModule, int which);
	~Scene1608();
protected:
	AsCommonCar *_asCar;
	Sprite *_asKey;
	Sprite *_asDoor;
	Sprite *_asIdleCarLower;
	Sprite *_asIdleCarFull;
	Sprite *_sprite1;
	Sprite *_sprite2;
	Sprite *_sprite3;
	Sprite *_asTape;
	Klayman *_kmScene1608;
	NRect _rect1;
	NRect _rect2;
	NRect _rect3;
	int _carStatus;
	bool _carClipFlag;
	bool _klaymanInCar;
	int _countdown1;
	NPointArray *_roomPathPoints;
	void upLowerFloor();
	void upUpperFloor();
	void upCarAtHome();
	void upGettingOutOfCar();
	void upRidingCar();
	uint32 hmLowerFloor(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmUpperFloor(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmRidingCar(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmCarAtHome(int messageNum, const MessageParam &param, Entity *sender);
	void updateKlaymanCliprect();
};

class Scene1609 : public Scene {
public:
	Scene1609(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	Sprite *_ssButton;
	AsScene3011Symbol *_asSymbols[12];
	int _currentSymbolIndex;
	int _noisySymbolIndex;
	int _symbolPosition;
	int _countdown1;
	bool _changeCurrentSymbol;
	bool _isSolved;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	bool testVars();
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULE1600_H */
