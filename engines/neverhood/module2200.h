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
#include "neverhood/graphics.h"

namespace Neverhood {

// Module2200

class Module2200 : public Module {
public:
	Module2200(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Module2200();
protected:
	void createScene(int sceneNum, int which);
	void updateScene();
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
	AsScene2203Door(NeverhoodEngine *vm, Scene *parentScene, uint index);
protected:
	Scene *_parentScene;
	SoundResource _soundResource;
	Sprite *_otherDoor;
	uint _index;
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

class AsScene2207Lever : public AnimatedSprite {
public:
	AsScene2207Lever(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y, int doDeltaX);
protected:
	Scene *_parentScene;
	SoundResource _soundResource;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void stLeverDown();
	void stLeverDownEvent();
	void stLeverUp();
	void stLeverUpEvent();
};

class AsScene2207WallRobotAnimation : public AnimatedSprite {
public:
	AsScene2207WallRobotAnimation(NeverhoodEngine *vm, Scene *parentScene);
	~AsScene2207WallRobotAnimation();
protected:
	SoundResource _soundResource1;
	SoundResource _soundResource2;
	SoundResource _soundResource3;
	SoundResource _soundResource4;
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
	Sprite *_asLever;
	Sprite *_asWallRobotAnimation;
	Sprite *_asWallCannonAnimation;
	Sprite *_ssButton;
	int _elevatorSurfacePriority;
	bool _klaymanAtElevator;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage2(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene2208 : public Scene {
public:
	Scene2208(NeverhoodEngine *vm, Module *parentModule, int which);
	~Scene2208();
protected:
	FontSurface *_fontSurface;
	BaseSurface *_backgroundSurface;
	BaseSurface *_topBackgroundSurface;
	BaseSurface *_bottomBackgroundSurface;
	TextResource _textResource;
	int16 _backgroundScrollY;
	int16 _newRowIndex;
	int16 _currRowIndex;
	int16 _rowScrollY;
	int16 _maxRowIndex;
	int16 _visibleRowsCount;
	Common::Array<const char*> _strings; // TODO: Move to TextResource
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void createFontSurface();
	void drawRow(int16 rowIndex);
};

class Scene2242 : public Scene {
public:
	Scene2242(NeverhoodEngine *vm, Module *parentModule, int which);
	~Scene2242();
protected:
	Sprite *_asTape;
	bool _isKlaymanInLight;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void readClickedColumn();
};

class HallOfRecordsScene : public Scene {
public:
	HallOfRecordsScene(NeverhoodEngine *vm, Module *parentModule, int which, uint32 sceneInfo140Id);
	~HallOfRecordsScene();
protected:
	SceneInfo140 *_sceneInfo140;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void readClickedColumn();
};

class Scene2247 : public Scene {
public:
	Scene2247(NeverhoodEngine *vm, Module *parentModule, int which);
	~Scene2247();
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void readClickedColumn();
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULE2200_H */
