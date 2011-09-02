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

#ifndef NEVERHOOD_MODULE2200_H
#define NEVERHOOD_MODULE2200_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"
#include "neverhood/module1000.h"

namespace Neverhood {

// Module2200

class Module2200 : public Module {
public:
	Module2200(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Module2200();
protected:
	void createScene2201(int which);			
	void createScene2202(int which);			
	void createScene2203(int which);			
	void createScene2204(int which);			
	void createScene2205(int which);			
	void createScene2206(int which);			
	void createScene2207(int which);			
	void createScene2208(int which);			
	void createScene2209(int which);			
	void createScene2210(int which);			
	void createScene2211(int which);			
	void createScene2212(int which);			
	void createScene2213(int which);			
	void createScene2214(int which);			
	void createScene2215(int which);			
	void createScene2216(int which);			
	void createScene2217(int which);			
	void createScene2218(int which);			
	void createScene2219(int which);			
	void createScene2220(int which);			
	void createScene2221(int which);			
	void createScene2222(int which);			
	void createScene2223(int which);			
	void createScene2224(int which);			
	void createScene2225(int which);			
	void createScene2226(int which);			
	void createScene2227(int which);			
	void createScene2228(int which);			
	void createScene2229(int which);			
	void createScene2230(int which);			
	void createScene2231(int which);			
	void createScene2232(int which);			
	void createScene2233(int which);			
	void createScene2234(int which);			
	void createScene2235(int which);			
	void createScene2236(int which);			
	void createScene2237(int which);			
	void createScene2238(int which);			
	void createScene2239(int which);			
	void createScene2240(int which);			
	void createScene2241(int which);			
	void createScene2242(int which);			
	void createScene2243(int which);			
	void createScene2244(int which);			
	void createScene2245(int which);			
	void createScene2246(int which);			
	void createScene2247(int which);			
	void createScene2248(int which);			
	void updateScene2201();			
	void updateScene2202();			
	void updateScene2203();			
	void updateScene2204();			
	void updateScene2205();			
	void updateScene2206();			
	void updateScene2207();			
	void updateScene2208();			
	void updateScene2209();			
	void updateScene2210();			
	void updateScene2211();			
	void updateScene2212();			
	void updateScene2213();			
	void updateScene2214();			
	void updateScene2215();			
	void updateScene2216();			
	void updateScene2217();			
	void updateScene2218();			
	void updateScene2219();			
	void updateScene2220();			
	void updateScene2221();			
	void updateScene2222();			
	void updateScene2223();			
	void updateScene2224();			
	void updateScene2225();			
	void updateScene2226();			
	void updateScene2227();			
	void updateScene2228();			
	void updateScene2229();			
	void updateScene2230();			
	void updateScene2231();			
	void updateScene2232();			
	void updateScene2233();			
	void updateScene2234();			
	void updateScene2235();			
	void updateScene2236();			
	void updateScene2237();			
	void updateScene2238();			
	void updateScene2239();			
	void updateScene2240();			
	void updateScene2241();			
	void updateScene2242();			
	void updateScene2243();			
	void updateScene2244();			
	void updateScene2245();			
	void updateScene2246();			
	void updateScene2247();			
	void updateScene2248();			
};

// Scene2201

static const NPoint kClass444Points[] = {
	{305, 305},
	{321, 305},
	{336, 305},
	{305, 319},
	{321, 319},
	{336, 319},
	{305, 332},
	{321, 332},
	{336, 333}
};

static const uint32 kClass444FileHashes[] = {
	0x88134A44,
	0xAA124340,
	0xB8124602,
	0xA902464C,
	0x890A4244,
	0xA8124642,
	0xB812C204,
	0x381A4A4C
};

class AsScene2201CeilingFan : public AnimatedSprite {
public:
	AsScene2201CeilingFan(NeverhoodEngine *vm);
};

class AsScene2201Door : public AnimatedSprite {
public:
	AsScene2201Door(NeverhoodEngine *vm, Klayman *klayman, Sprite *doorLightSprite, bool flag1);
protected:
	SoundResource _soundResource;
	Klayman *_klayman;
	Sprite *_doorLightSprite;
	bool _doorOpen;
	int _countdown;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void stOpenDoor();
	void stCloseDoor();
};

class Class444 : public StaticSprite {
public:
	Class444(NeverhoodEngine *vm, int pointIndex, int spriteIndex);
};

class Scene2201 : public Scene {
public:
	Scene2201(NeverhoodEngine *vm, Module *parentModule, int which);
	~Scene2201();
protected:
	NRect _rect1;
	NRect _rect2;
	Sprite *_doorLightSprite;
	Sprite *_asDoor;
	Sprite *_ssDoorButton;
	Sprite *_asTape;
	bool _soundFlag;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class SsScene2202PuzzleTile : public StaticSprite {
public:
	SsScene2202PuzzleTile(NeverhoodEngine *vm, Scene *parentScene, int16 tileIndex, int16 value);
protected:
	Scene *_parentScene;	
	int16 _value;
	int16 _tileIndex;
	int16 _newX, _newY;
	int16 _xDelta, _yDelta;
	int16 _xIncr;
	int16 _yIncr;
	int16 _errValue;
	int16 _counter;
	int16 _xFlagPos;
	bool _counterDirection;
	bool _isMoving;
	SoundResource _soundResource1;
	SoundResource _soundResource2;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void suMoveTileX();
	void suMoveTileY();
	void moveTile(int16 newTileIndex);
	void stopMoving();
};

class Scene2202 : public Scene {
public:
	Scene2202(NeverhoodEngine *vm, Module *parentModule, int which);
	~Scene2202();
protected:
	SoundResource _soundResource1;
	SoundResource _soundResource2;
	Sprite *_movingTileSprite;
	Sprite *_doneMovingTileSprite;
	bool _isTileMoving;
	int16 _movingTileIndex;
	int _surfacePriority;
	bool _leaveScene;
	bool _isSolved;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	int16 getFreeTileIndex(int16 index);
	bool testIsSolved();	
};

class Class545 : public AnimatedSprite {
public:
	Class545(NeverhoodEngine *vm, Scene *parentScene, int index, int surfacePriority, int16 x, int16 y);
protected:
	Scene *_parentScene;
	int _index;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene2203Door : public AnimatedSprite {
public:
	AsScene2203Door(NeverhoodEngine *vm, Scene *parentScene, int index);
protected:
	Scene *_parentScene;
	SoundResource _soundResource;
	Sprite *_otherDoor;
	int _index;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void openDoor();
	void closeDoor();
};

class Scene2203 : public Scene {
public:
	Scene2203(NeverhoodEngine *vm, Module *parentModule, int which);
	~Scene2203();
protected:
	Sprite *_asLeftDoor;
	Sprite *_asRightDoor;
	Sprite *_ssSmallLeftDoor;
	Sprite *_ssSmallRightDoor;
	Sprite *_asTape;
	Sprite *_class545;
	NRect _leftDoorClipRect;
	NRect _rightDoorClipRect;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class SsScene2205DoorFrame : public StaticSprite {
public:
	SsScene2205DoorFrame(NeverhoodEngine *vm);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene2205 : public Scene {
public:
	Scene2205(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	Class426 *_ssLightSwitch;
	Sprite *_ssDoorFrame;
	bool _isKlaymanInLight;
	bool _isLightOn;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Class603 : public StaticSprite {
public:
	Class603(NeverhoodEngine *vm, uint32 fileHash);
protected:
	int _index;
	SoundResource _soundResource;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void spriteUpdate481E60();
	void spriteUpdate481E90();
};

class Class604 : public StaticSprite {
public:
	Class604(NeverhoodEngine *vm, uint32 fileHash);
protected:
	int16 _yDelta;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void spriteUpdate482020();
};

class Class607 : public StaticSprite {
public:
	Class607(NeverhoodEngine *vm, Scene *parentScene, int surfacePriority, uint32 fileHash);
protected:
	Scene *_parentScene;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene2206 : public Scene {
public:
	Scene2206(NeverhoodEngine *vm, Module *parentModule, int which);
	~Scene2206();
protected:
	Sprite *_sprite1;
	Sprite *_sprite2;
	Sprite *_sprite3;
	Sprite *_sprite4;
	Sprite *_sprite5;
	Sprite *_class604;
	Sprite *_class607;
	SoundResource _soundResource;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void sub481950();
	void sub4819D0();
	void sub481B00();
};

class AsScene2207Elevator : public AnimatedSprite {
public:
	AsScene2207Elevator(NeverhoodEngine *vm, Scene *parentScene);
	~AsScene2207Elevator();
protected:
	Scene *_parentScene;
	SoundResource _soundResource;
	NPointArray *_pointArray;
	int16 _pointIndex;
	int16 _destPointIndex, _destPointIndexDelta;
	bool _isMoving;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void suSetPosition();
	void moveToY(int16 y);
};

class Class500 : public AnimatedSprite {
public:
	Class500(NeverhoodEngine *vm, Scene *parentScene);
	~Class500();
protected:
	SoundResource _soundResource1;
	SoundResource _soundResource2;
	SoundResource _soundResource3;
	SoundResource _soundResource4;
	bool _flag1;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void sub441D50();
	void sub441D90();
	void sub441DA0();
};

class Class501 : public AnimatedSprite {
public:
	Class501(NeverhoodEngine *vm);
protected:
	bool _flag1;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void sub441FA0();
	void sub441FE0();
	void sub441FF0();
};

class Class597 : public StaticSprite {
public:
	Class597(NeverhoodEngine *vm, uint32 fileHash, int index);
};

class Scene2207 : public Scene {
public:
	Scene2207(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	SoundResource _soundResource1;
	SoundResource _soundResource2;
	Sprite *_asElevator;
	Sprite *_ssMaskPart1;
	Sprite *_ssMaskPart2;
	Sprite *_ssMaskPart3;
	Sprite *_asTape;
	Sprite *_class487;
	Sprite *_class500;
	Sprite *_class501;
	Sprite *_ssButton;
	int _elevatorSurfacePriority;
	bool _flag1;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage2(int messageNum, const MessageParam &param, Entity *sender);
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULE2200_H */
