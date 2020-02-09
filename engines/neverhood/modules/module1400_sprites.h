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

#ifndef NEVERHOOD_MODULES_MODULE1400_SPRITES_H
#define NEVERHOOD_MODULES_MODULE1400_SPRITES_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"

namespace Neverhood {

class AsScene1401Pipe : public AnimatedSprite {
public:
	AsScene1401Pipe(NeverhoodEngine *vm);
	~AsScene1401Pipe() override;
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
	~AsCommonProjector() override;
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

class Scene1405;

class AsScene1405Tile : public AnimatedSprite {
public:
	AsScene1405Tile(NeverhoodEngine *vm, Scene1405 *parentScene, uint32 tileIndex);
	void show();
	void hide(bool playClickSound);
protected:
	Scene1405 *_parentScene;
	bool _isShowing;
	uint32 _tileIndex;
	int _countdown;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class KmScene1401 : public Klaymen {
public:
	KmScene1401(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

class KmScene1402 : public Klaymen {
public:
	KmScene1402(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

class KmScene1403 : public Klaymen {
public:
	KmScene1403(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

class KmScene1404 : public Klaymen {
public:
	KmScene1404(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULES_MODULE1400_SPRITES_H */
