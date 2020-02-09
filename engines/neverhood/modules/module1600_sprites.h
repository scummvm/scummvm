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

#ifndef NEVERHOOD_MODULES_MODULE1600_SPRITES_H
#define NEVERHOOD_MODULES_MODULE1600_SPRITES_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"

namespace Neverhood {

class AsCommonCar : public AnimatedSprite {
public:
	AsCommonCar(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
	~AsCommonCar() override;
	void setPathPoints(NPointArray *pathPoints);
protected:
	Scene *_parentScene;
	NPointArray *_pathPoints;
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

class Tracks : public Common::Array<TrackInfo*> {
public:
	void findTrackPoint(NPoint pt, int &minMatchTrackIndex, int &minMatchDistance,
		DataResource &dataResource);
};

class KmScene1608 : public Klaymen {
public:
	KmScene1608(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULES_MODULE1600_SPRITES_H */
