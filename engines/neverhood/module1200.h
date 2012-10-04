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

#ifndef NEVERHOOD_MODULE1200_H
#define NEVERHOOD_MODULE1200_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"

namespace Neverhood {

// Module1200

class Module1200 : public Module {
public:
	Module1200(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Module1200();
protected:
	void createScene(int sceneNum, int which);
	void updateScene();
};

// Scene1201

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
	AsScene1201TntManRope(NeverhoodEngine *vm, bool flag);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene1201RightDoor : public AnimatedSprite {
public:
	AsScene1201RightDoor(NeverhoodEngine *vm, Sprite *klayman, bool flag);
protected:
	Sprite *_klayman;
	int _countdown;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void stOpenDoor();
	void stCloseDoor();
	void stCloseDoorDone();
};

class AsScene1201KlaymanHead : public AnimatedSprite {
public:
	AsScene1201KlaymanHead(NeverhoodEngine *vm);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene1201TntMan : public AnimatedSprite {
public:
	AsScene1201TntMan(NeverhoodEngine *vm, Scene *parentScene, Sprite *asTntManRope, bool isDown);
	virtual ~AsScene1201TntMan();
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
	~AsScene1201TntManFlame();
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
	AsScene1201Creature(NeverhoodEngine *vm, Scene *parentScene, Sprite *klayman);
protected:
	Scene *_parentScene;
	Sprite *_klayman;
	int _countdown1;
	int _countdown2;
	int _countdown3;
	bool _flag;
	void update();
	uint32 handleMessage40C710(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage40C7B0(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage40C830(int messageNum, const MessageParam &param, Entity *sender);
	void sub40C8E0();
	void sub40C930();
	void sub40C960();
	void sub40C990();
	void sub40C9B0();
	void sub40C9E0();
};

class AsScene1201LeftDoor : public AnimatedSprite {
public:
	AsScene1201LeftDoor(NeverhoodEngine *vm, Sprite *klayman);
protected:
	Sprite *_klayman;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void sub40D590();
};

class SsScene1201Tnt : public StaticSprite {
public:
	SsScene1201Tnt(NeverhoodEngine *vm, uint32 elemIndex, uint32 pointIndex, int16 clipY2);
protected:
	uint32 _elemIndex;
	int16 _field7A;	
};

class Scene1201 : public Scene {
public:
	Scene1201(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Scene1201();
protected:
	Sprite *_asMatch;
	AsScene1201TntMan *_asTntMan;
	Sprite *_asCreature;
	Sprite *_asTntManRope;
	Sprite *_asLeftDoor;
	Sprite *_asRightDoor;
	Sprite *_asTape;
	Sprite *_asKlaymanHead;
	bool _flag;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

// Scene1202

class AsScene1202TntItem : public AnimatedSprite {
public:
	AsScene1202TntItem(NeverhoodEngine *vm, Scene *parentScene, int index);
protected:
	Scene *_parentScene;
	int _index, _index2;
	uint32 handleMessage453FE0(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage454060(int messageNum, const MessageParam &param, Entity *sender);
	void sub4540A0();
	void sub4540D0();
	void sub454100();
	void sub454160();
};

class Scene1202 : public Scene {
public:
	Scene1202(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Scene1202();
protected:
	PaletteResource _paletteResource;
	Sprite *_asTntItems[18];
	int _counter;
	int _index;
	byte _paletteData[1024];
	bool _soundFlag;
	bool _flag;
	void update();
	uint32 handleMessage453C10(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage453D90(int messageNum, const MessageParam &param, Entity *sender);
	bool isSolved();
	void doPaletteEffect();
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULE1200_H */
