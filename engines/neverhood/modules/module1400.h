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

#ifndef NEVERHOOD_MODULES_MODULE1400_H
#define NEVERHOOD_MODULES_MODULE1400_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"
#include "neverhood/modules/module1200.h"

namespace Neverhood {

class Module1400 : public Module {
public:
	Module1400(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Module1400();
protected:
	int _sceneNum;
	void createScene(int sceneNum, int which);
	void updateScene();
};

// Scene1401

class AsScene1401Pipe : public AnimatedSprite {
public:
	AsScene1401Pipe(NeverhoodEngine *vm);
	virtual ~AsScene1401Pipe();
protected:
	int _countdown1;
	int _countdown2;
	void update();
	void upSuckInProjector();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmSuckInProjector(int messageNum, const MessageParam &param, Entity *sender);
	void stStartSucking();
	void stDoneSucking();
	void stSuckInProjector();
};

class AsScene1401Mouse : public AnimatedSprite {
public:
	AsScene1401Mouse(NeverhoodEngine *vm);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void suSuckedIn();
	void stSuckedIn();
};

class AsScene1401Cheese : public AnimatedSprite {
public:
	AsScene1401Cheese(NeverhoodEngine *vm);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void suSuckedIn();
	void stSuckedIn();
};

class AsScene1401BackDoor : public AnimatedSprite {
public:
	AsScene1401BackDoor(NeverhoodEngine *vm, Sprite *klaymen, bool isOpen);
protected:
	Sprite *_klaymen;
	int _countdown;
	bool _isOpen;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void stOpenDoor();
	void stCloseDoor();
	void stCloseDoorDone();
};

struct AsCommonProjectorItem {
	NPoint point;
	int8 maxSlotCount;
	int8 lockSlotIndex;
	int8 index1;
	int8 leftBorderLeaves;
	int8 rightBorderLeaves;
};

class AsCommonProjector : public AnimatedSprite {
public:
	AsCommonProjector(NeverhoodEngine *vm, Scene *parentScene, Sprite *klaymen, Sprite *asPipe);
	virtual ~AsCommonProjector();
protected:
	Scene *_parentScene;
	Sprite *_klaymen;
	Sprite *_asPipe;
	const AsCommonProjectorItem *_asProjectorItem;
	int16 _beforeMoveX;
	bool _lockedInSlot;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmLockedInSlot(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmAnimation(int messageNum, const MessageParam &param, Entity *sender);
	void suMoving();
	void moveProjector();
	void stSuckedIn();
	void stIdle();
	void stMoving();
	void stStartLockedInSlot();
	void stStayLockedInSlot();
	void stStartProjecting();
	void stLockedInSlot();
	void stStopProjecting();
	void stTurnToFront();
	void stStartSuckedIn();
};

class Scene1401 : public Scene {
public:
	Scene1401(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	bool _projectorBorderFlag;
	Sprite *_ssFloorButton;
	AsCommonProjector *_asProjector;
	Sprite *_asPipe;
	Sprite *_asMouse;
	Sprite *_asCheese;
	Sprite *_asBackDoor;
	Sprite *_sprite1;
	Sprite *_sprite2;
	Sprite *_sprite3;
	Sprite *_ssButton;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

// Scene1402

class SsScene1402BridgePart : public StaticSprite {
public:
	SsScene1402BridgePart(NeverhoodEngine *vm, uint32 fileHash, int surfacePriority);
};

class AsScene1402PuzzleBox : public AnimatedSprite {
public:
	AsScene1402PuzzleBox(NeverhoodEngine *vm, Scene *parentScene, int status);
protected:
	Scene *_parentScene;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void stMoveUpDone();
	void stMoveDownDone();
	void stMoveDownSolvedDone();
};

class Scene1402 : public Scene {
public:
	Scene1402(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	Sprite *_ssBridgePart1;
	Sprite *_ssBridgePart2;
	Sprite *_ssBridgePart3;
	Sprite *_asPuzzleBox;
	AsCommonProjector *_asProjector;
	bool _isShaking;
	void upShaking();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void startShaking();
	void stopShaking();
};

// Scene1407

class AsScene1407Mouse : public AnimatedSprite {
public:
	AsScene1407Mouse(NeverhoodEngine *vm, Scene *parentScene);
protected:
	Scene *_parentScene;
	int16 _walkDestX;
	int16 _currSectionIndex;
	int16 _nextHoleIndex;
	int _countdown;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void suWalkTo();
	void upGoThroughHole();
	void stIdleLookAtGoodHole();
	void stWalkToDest();
	void stWalkToHole();
	void stGoThroughHole();
	void stArriveAtHole();
};

class Scene1407 : public Scene {
public:
	Scene1407(NeverhoodEngine *vm, Module *parentModule);
protected:
	Sprite *_asMouse;
	Sprite *_ssResetButton;
	int _puzzleSolvedCountdown;
	int _resetButtonCountdown;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

// Scene1403

class Scene1403 : public Scene {
public:
	Scene1403(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	Sprite *_sprite1;
	Sprite *_sprite2;
	Sprite *_sprite3;
	AsScene1201Tape *_asTape1;
	AsScene1201Tape *_asTape2;
	AsCommonProjector *_asProjector;
	bool _isProjecting;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

// Scene1404

class Scene1404 : public Scene {
public:
	Scene1404(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Scene1404();
protected:
	Sprite *_sprite1;
	Sprite *_asTape;
	AsCommonProjector *_asProjector;
	Sprite *_asKey;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

// Scene1405

class Scene1405;

class AsScene1405Tile : public AnimatedSprite {
public:
	AsScene1405Tile(NeverhoodEngine *vm, Scene1405 *parentScene, uint32 tileIndex);
	void show();
	void hide();
protected:
	Scene1405 *_parentScene;
	bool _isShowing;
	uint32 _tileIndex;
	int _countdown;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene1405 : public Scene {
public:
	Scene1405(NeverhoodEngine *vm, Module *parentModule);
	int getCountdown() const { return _countdown; }
protected:
	bool _selectFirstTile;
	int _firstTileIndex;
	int _secondTileIndex;
	int _tilesLeft;
	int _countdown;
	AsScene1405Tile *_tiles[48];
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULES_MODULE1400_H */
