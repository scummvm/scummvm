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

#ifndef NEVERHOOD_MODULES_MODULE2200_SPRITES_H
#define NEVERHOOD_MODULES_MODULE2200_SPRITES_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"
#include "neverhood/graphics.h"

namespace Neverhood {

static const NPoint kSsScene2201PuzzleCubePoints[] = {
	{305, 305},	{321, 305},	{336, 305},	{305, 319},
	{321, 319},	{336, 319},	{305, 332},	{321, 332},
	{336, 333}
};

static const uint32 kSsScene2201PuzzleCubeFileHashes[] = {
	0x88134A44,	0xAA124340,	0xB8124602,	0xA902464C,
	0x890A4244,	0xA8124642,	0xB812C204,	0x381A4A4C
};

class AsScene2201CeilingFan : public AnimatedSprite {
public:
	AsScene2201CeilingFan(NeverhoodEngine *vm);
};

class AsScene2201Door : public AnimatedSprite {
public:
	AsScene2201Door(NeverhoodEngine *vm, Klaymen *klaymen, Sprite *ssDoorLight, bool isOpen);
protected:
	Klaymen *_klaymen;
	Sprite *_ssDoorLight;
	bool _isOpen;
	int _countdown;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void stOpenDoor();
	void stCloseDoor();
};

class SsScene2201PuzzleCube : public StaticSprite {
public:
	SsScene2201PuzzleCube(NeverhoodEngine *vm, uint32 positionIndex, uint32 cubeIndex);
};

class SsScene2202PuzzleCube : public StaticSprite {
public:
	SsScene2202PuzzleCube(NeverhoodEngine *vm, Scene *parentScene, int16 cubePosition, int16 cubeSymbol);
protected:
	Scene *_parentScene;
	int16 _cubeSymbol;
	int16 _cubePosition;
	int16 _newX, _newY;
	int16 _xDelta, _yDelta;
	int16 _xIncr;
	int16 _yIncr;
	int16 _errValue;
	int16 _counter;
	int16 _xFlagPos;
	bool _counterDirection;
	bool _isMoving;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void suMoveCubeX();
	void suMoveCubeY();
	void moveCube(int16 newCubePosition);
	void stopMoving();
};

class AsCommonKey : public AnimatedSprite {
public:
	AsCommonKey(NeverhoodEngine *vm, Scene *parentScene, int keyIndex, int surfacePriority, int16 x, int16 y);
protected:
	Scene *_parentScene;
	int _keyIndex;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene2203Door : public AnimatedSprite {
public:
	AsScene2203Door(NeverhoodEngine *vm, Scene *parentScene, uint doorIndex);
protected:
	Scene *_parentScene;
	Sprite *_otherDoor;
	uint _doorIndex;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void openDoor();
	void closeDoor();
};

class SsScene2205DoorFrame : public StaticSprite {
public:
	SsScene2205DoorFrame(NeverhoodEngine *vm);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene2206DoorSpikes : public StaticSprite {
public:
	AsScene2206DoorSpikes(NeverhoodEngine *vm, uint32 fileHash);
protected:
	int _deltaIndex;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void suOpen();
	void suClose();
};

class AsScene2206Platform : public StaticSprite {
public:
	AsScene2206Platform(NeverhoodEngine *vm, uint32 fileHash);
protected:
	int16 _yDelta;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void suMoveDown();
};

class SsScene2206TestTube : public StaticSprite {
public:
	SsScene2206TestTube(NeverhoodEngine *vm, Scene *parentScene, int surfacePriority, uint32 fileHash);
protected:
	Scene *_parentScene;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene2207Elevator : public AnimatedSprite {
public:
	AsScene2207Elevator(NeverhoodEngine *vm, Scene *parentScene);
	~AsScene2207Elevator() override;
protected:
	Scene *_parentScene;
	NPointArray *_pointArray;
	int16 _pointIndex;
	int16 _destPointIndex, _destPointIndexDelta;
	bool _isMoving;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void suSetPosition();
	void moveToY(int16 y);
};

class AsScene2207Lever : public AnimatedSprite {
public:
	AsScene2207Lever(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y, int doDeltaX);
protected:
	Scene *_parentScene;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void stLeverDown();
	void stLeverDownEvent();
	void stLeverUp();
	void stLeverUpEvent();
};

class AsScene2207WallRobotAnimation : public AnimatedSprite {
public:
	AsScene2207WallRobotAnimation(NeverhoodEngine *vm, Scene *parentScene);
	~AsScene2207WallRobotAnimation() override;
protected:
	bool _idle;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void stStartAnimation();
	void stStopAnimation();
	void cbStopAnimation();
};

class AsScene2207WallCannonAnimation : public AnimatedSprite {
public:
	AsScene2207WallCannonAnimation(NeverhoodEngine *vm);
protected:
	bool _idle;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void stStartAnimation();
	void stStopAnimation();
	void cbStopAnimation();
};

class SsScene2207Symbol : public StaticSprite {
public:
	SsScene2207Symbol(NeverhoodEngine *vm, uint32 fileHash, int index);
};

class KmScene2201 : public Klaymen {
public:
	KmScene2201(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y, NRect *clipRects, int clipRectsCount);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

class KmScene2203 : public Klaymen {
public:
	KmScene2203(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
protected:
	void stClayDoorOpen();
	uint32 hmClayDoorOpen(int messageNum, const MessageParam &param, Entity *sender);

	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

class KmScene2205 : public Klaymen {
public:
	KmScene2205(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
protected:
	void xUpdate() override;
	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

class KmScene2206 : public Klaymen {
public:
	KmScene2206(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
	~KmScene2206() override;
protected:
	void stRidePlatformDown();
	void suRidePlatformDown();

	void xUpdate() override;
	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

class KmScene2207 : public Klaymen {
public:
	KmScene2207(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

class KmScene2242 : public Klaymen {
public:
	KmScene2242(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
protected:
	void xUpdate() override;
	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

class KmHallOfRecords : public Klaymen {
public:
	KmHallOfRecords(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
protected:
	void xUpdate() override;
	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

class KmScene2247 : public Klaymen {
public:
	KmScene2247(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
protected:
	void xUpdate() override;
	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULES_MODULE2200_SPRITES_H */
