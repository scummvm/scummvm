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

#ifndef NEVERHOOD_MODULES_MODULE1000_H
#define NEVERHOOD_MODULES_MODULE1000_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"

namespace Neverhood {

// Module1000

class Module1000 : public Module {
public:
	Module1000(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Module1000();
protected:
	int _sceneNum;
	uint32 _musicFileHash;
	void createScene(int sceneNum, int which);
	void updateScene();
};

// Scene1001

class AsScene1001Door : public AnimatedSprite {
public:
	AsScene1001Door(NeverhoodEngine *vm);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void hammerHitsDoor();
	void stShowIdleDoor();
	void stBustedDoorMove();
	void stBustedDoorGone();
};

class AsScene1001Hammer : public AnimatedSprite {
public:
	AsScene1001Hammer(NeverhoodEngine *vm, Sprite *asDoor);
protected:
	Sprite *_asDoor;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene1001Window : public AnimatedSprite {
public:
	AsScene1001Window(NeverhoodEngine *vm);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene1001Lever : public AnimatedSprite {
public:
	AsScene1001Lever(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y, int deltaXType);
protected:
	Scene *_parentScene;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class SsCommonButtonSprite : public StaticSprite {
public:
	SsCommonButtonSprite(NeverhoodEngine *vm, Scene *parentScene, uint32 fileHash, int surfacePriority, uint32 soundFileHash);
protected:
	Scene *_parentScene;
	uint32 _soundFileHash;
	int16 _countdown;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene1001 : public Scene {
public:
	Scene1001(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Scene1001();
protected:
	Sprite *_asHammer;
	Sprite *_asDoor;
	Sprite *_asWindow;
	Sprite *_asLever;
	Sprite *_ssButton;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

// TODO: Move this to some common file since it's used several times

class StaticScene : public Scene {
public:
	StaticScene(NeverhoodEngine *vm, Module *parentModule, uint32 backgroundFileHash, uint32 cursorFileHash);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

// Scene1002

class AsScene1002Ring : public AnimatedSprite {
public:
	AsScene1002Ring(NeverhoodEngine *vm, Scene *parentScene, bool isSpecial, int16 x, int16 y, int16 clipY1, bool isRingLow);
protected:
	Scene *_parentScene;
	bool _isSpecial;
	void update();
	uint32 hmRingIdle(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmRingPulled1(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmRingPulled2(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmRingHangingLow(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmRingReleased(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene1002Door : public StaticSprite {
public:
	AsScene1002Door(NeverhoodEngine *vm, NRect &clipRect);
protected:
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void suOpenDoor();
	void suCloseDoor();
};

class AsScene1002BoxingGloveHitEffect : public AnimatedSprite {
public:
	AsScene1002BoxingGloveHitEffect(NeverhoodEngine *vm);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene1002DoorSpy : public AnimatedSprite {
public:
	AsScene1002DoorSpy(NeverhoodEngine *vm, NRect &clipRect, Scene *parentScene, Sprite *asDoor, Sprite *asScene1002BoxingGloveHitEffect);
protected:
	Scene *_parentScene;
	Sprite *_asDoor;
	Sprite *_asBoxingGloveHitEffect;
	NRect _clipRect;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmDoorSpyAnimation(int messageNum, const MessageParam &param, Entity *sender);
	void suDoorSpy();
	void stDoorSpyIdle();
	void stDoorSpyBoxingGlove();
};

class SsCommonPressButton : public StaticSprite {
public:
	SsCommonPressButton(NeverhoodEngine *vm, Scene *parentScene, uint32 fileHash1, uint32 fileHash2, int surfacePriority, uint32 soundFileHash);
	void setFileHashes(uint32 fileHash1, uint32 fileHash2);
protected:
	Scene *_parentScene;
	uint32 _soundFileHash;
	uint32 _fileHashes[2];
	int _status;
	int _countdown;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene1002VenusFlyTrap : public AnimatedSprite {
public:
	AsScene1002VenusFlyTrap(NeverhoodEngine *vm, Scene *parentScene, Sprite *klaymen, bool isSecond);
protected:
	Scene *_parentScene;
	Sprite *_klaymen;
	int _countdown;
	bool _isSecond;
	void update();
	void upIdle();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmAnimationSimple(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmAnimationExt(int messageNum, const MessageParam &param, Entity *sender);
	void stWalkBack();
	void stWalk();
	void stRelease();
	void stGrabRing();
	void stRingGrabbed();
	void stKlaymenInside();
	void stIdle();
	void stKlaymenInsideMoving();
	void stSpitOutKlaymen();
	void swallowKlaymen();
};

class AsScene1002OutsideDoorBackground : public AnimatedSprite {
public:
	AsScene1002OutsideDoorBackground(NeverhoodEngine *vm);
protected:
	int _countdown;
	bool _isDoorClosed;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmAnimation(int messageNum, const MessageParam &param, Entity *sender);
	void stOpenDoor();
	void stCloseDoor();
	void stDoorClosed();
};

class AsScene1002KlaymenLadderHands : public AnimatedSprite {
public:
	AsScene1002KlaymenLadderHands(NeverhoodEngine *vm, Klaymen *klaymen);
protected:
	Klaymen *_klaymen;
	void update();
};

class AsScene1002KlaymenPeekHand : public AnimatedSprite {
public:
	AsScene1002KlaymenPeekHand(NeverhoodEngine *vm, Scene *parentScene, Klaymen *klaymen);
protected:
	Scene *_parentScene;
	Klaymen *_klaymen;
	bool _isClipRectSaved;
	NRect _savedClipRect;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene1002 : public Scene {
public:
	Scene1002(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Scene1002();
protected:
	Sprite *_asRing1;
	Sprite *_asRing2;
	Sprite *_asRing3;
	Sprite *_asRing4;
	Sprite *_asRing5;
	Sprite *_asDoor;
	Sprite *_asDoorSpy;
	Sprite *_asVenusFlyTrap;
	Sprite *_ssLadderArch;
	Sprite *_ssLadderArchPart1;
	Sprite *_ssLadderArchPart2;
	Sprite *_ssLadderArchPart3;
	Sprite *_ssCeiling;
	Sprite *_asKlaymenLadderHands;
	Sprite *_asKlaymenPeekHand;
	Sprite *_asOutsideDoorBackground;
	Sprite *_ssPressButton;
	bool _isKlaymenFloor;
	bool _isClimbingLadder;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

// Scene1004

class AsScene1004TrashCan : public AnimatedSprite {
public:
	AsScene1004TrashCan(NeverhoodEngine *vm);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene1004 : public Scene {
public:
	Scene1004(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	Sprite *_asKlaymenLadderHands;
	Sprite *_asTrashCan;
	int _paletteAreaStatus;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void updatePaletteArea();
};

// Scene1005

class Scene1005 : public Scene {
public:
	Scene1005(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void drawTextToBackground();
	uint32 getTextIndex();
	uint32 getTextIndex1();
	uint32 getTextIndex2();
	uint32 getTextIndex3();
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULES_MODULE1000_H */
