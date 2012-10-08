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
	NPoint _againDestPt;
	int _againDestPtFlag;
	int _steps;
	int _stepError;
	int _idleCounter;
	int _idleCounterMax;
	int _lastDistance;
	int _field100;
	int _againDestPointFlag;
	int _flag10E;
	int _moreY;
	int _flag10F;
	int _flag113;
	int _flag114;
	int _flag11A;
	int _newDeltaXType;
	int _field11E;
	int _againDestPointIndex;
	int _value112;
	int _anotherY;
	int16 _someX, _someY;
	NPoint pathPoint(uint index) { return (*_pathPoints)[index]; }
	void update();
	void update45C790();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage45CC30(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage45CCA0(int messageNum, const MessageParam &param, Entity *sender);
	void sub45CD00();
	void sub45CDC0();
	void sub45CE10();
	void sub45CF80();
	void sub45CFB0();
	void sub45CFE0();
	void sub45D040();
	void sub45D050();
	void sub45D0A0();
	void sub45D0E0();
	void sub45D100();
	void sub45D180();
	void moveToNextPoint();
	void sub45D350();
	void sub45D390();
	void moveToPrevPoint();
	void sub45D580();
	void sub45D5D0();
	void sub45D620();
	void suMoveToNextPoint();
	void suMoveToPrevPoint();
	void sub45E0A0();
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

class AsScene1608IdleCarLower : public AnimatedSprite {
public:
	AsScene1608IdleCarLower(NeverhoodEngine *vm, int16 x, int16 y);
};

class AsScene1608IdleCarFull : public AnimatedSprite {
public:
	AsScene1608IdleCarFull(NeverhoodEngine *vm, int16 x, int16 y);
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
