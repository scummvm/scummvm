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

#ifndef NEVERHOOD_MODULES_MODULE1200_SPRITES_H
#define NEVERHOOD_MODULES_MODULE1200_SPRITES_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"

namespace Neverhood {

// Used for both the scene sprites and the scene itself (for clipping)
static const NPoint kScene1201PointArray[] = {
	{218, 193}, {410, 225}, {368, 277},
	{194, 227}, {366, 174}, {458, 224},
	{242, 228}, {512, 228}, {458, 277},
	{217, 233}, {458, 173}, {410, 276},
	{203, 280}, {371, 226}, {508, 279},
	{230, 273}, {410, 171}, {493, 174}
};

class AsScene1201Tape : public AnimatedSprite {
public:
	AsScene1201Tape(NeverhoodEngine *vm, Scene *parentScene, uint32 nameHash, int surfacePriority, int16 x, int16 y, uint32 fileHash);
protected:
	Scene *_parentScene;
	uint32 _nameHash;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene1201TntManRope : public AnimatedSprite {
public:
	AsScene1201TntManRope(NeverhoodEngine *vm, bool isDummyHanging);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene1201RightDoor : public AnimatedSprite {
public:
	AsScene1201RightDoor(NeverhoodEngine *vm, Sprite *klaymen, bool isOpen);
protected:
	Sprite *_klaymen;
	int _countdown;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void stOpenDoor();
	void stCloseDoor();
	void stCloseDoorDone();
};

class AsScene1201KlaymenHead : public AnimatedSprite {
public:
	AsScene1201KlaymenHead(NeverhoodEngine *vm);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene1201TntMan : public AnimatedSprite {
public:
	AsScene1201TntMan(NeverhoodEngine *vm, Scene *parentScene, Sprite *asTntManRope, bool isDown);
	~AsScene1201TntMan() override;
protected:
	Scene *_parentScene;
	Sprite *_asTntManRope;
	Sprite *_sprite;
	bool _isMoving;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmComingDown(int messageNum, const MessageParam &param, Entity *sender);
	void suMoving();
	void stStanding();
	void stComingDown();
	void stMoving();
};

class AsScene1201TntManFlame : public AnimatedSprite {
public:
	AsScene1201TntManFlame(NeverhoodEngine *vm, Sprite *asTntMan);
	~AsScene1201TntManFlame() override;
protected:
	Sprite *_asTntMan;
	void update();
	void suUpdate();
};

class AsScene1201Match : public AnimatedSprite {
public:
	AsScene1201Match(NeverhoodEngine *vm, Scene *parentScene);
protected:
	Scene *_parentScene;
	int _countdown;
	int _status;
	void update();
	uint32 hmOnDoorFrameAboutToMove(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmOnDoorFrameMoving(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmIdle(int messageNum, const MessageParam &param, Entity *sender);
	void stOnDoorFrameMoving();
	void stFallingFromDoorFrame();
	void stOnDoorFrameAboutToMove();
	void stIdleOnDoorFrame();
	void stIdleOnFloor();
};

class AsScene1201Creature : public AnimatedSprite {
public:
	AsScene1201Creature(NeverhoodEngine *vm, Scene *parentScene, Sprite *klaymen);
protected:
	Scene *_parentScene;
	Sprite *_klaymen;
	int _countdown;
	bool _klaymenTooClose;
	void update();
	uint32 hmWaiting(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmPincerSnap(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmPincerSnapKlaymen(int messageNum, const MessageParam &param, Entity *sender);
	void stWaiting();
	void stPincerSnap();
	void stStartReachForTntDummy();
	void stReachForTntDummy();
	void stPincerSnapKlaymen();
};

class AsScene1201LeftDoor : public AnimatedSprite {
public:
	AsScene1201LeftDoor(NeverhoodEngine *vm, Sprite *klaymen);
protected:
	Sprite *_klaymen;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void stCloseDoor();
};

class SsScene1201Tnt : public StaticSprite {
public:
	SsScene1201Tnt(NeverhoodEngine *vm, uint32 elemIndex, uint32 pointIndex, int16 clipY2);
};

class AsScene1202TntItem : public AnimatedSprite {
public:
	AsScene1202TntItem(NeverhoodEngine *vm, Scene *parentScene, int index);
protected:
	Scene *_parentScene;
	int _itemIndex, _newPosition;
	uint32 hmShowIdle(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmChangePosition(int messageNum, const MessageParam &param, Entity *sender);
	void stShowIdle();
	void stChangePositionFadeOut();
	void stChangePositionFadeIn();
	void stChangePositionDone();
};

class KmScene1201 : public Klaymen {
public:
	KmScene1201(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
protected:
	void stCloseEyes();
	void stTumbleHeadless();
	void stFetchMatch();
	void stLightMatch();

	uint32 hmTumbleHeadless(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmMatch(int messageNum, const MessageParam &param, Entity *sender);

	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULES_MODULE1200_SPRITES_H */
