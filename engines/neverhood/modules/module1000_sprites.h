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

#ifndef NEVERHOOD_MODULES_MODULE1000_SPRITES_H
#define NEVERHOOD_MODULES_MODULE1000_SPRITES_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"

namespace Neverhood {

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

class AsScene1004TrashCan : public AnimatedSprite {
public:
	AsScene1004TrashCan(NeverhoodEngine *vm);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class KmScene1001 : public Klaymen {
public:
	KmScene1001(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
protected:
	void stWakeUp();
	void stSleeping();
	void stPullHammerLever();
	uint32 hmSleeping(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmPullHammerLever(int messageNum, const MessageParam &param, Entity *sender);

	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

class KmScene1002 : public Klaymen {
public:
	KmScene1002(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
protected:
	void stJumpToRing1();
	void stJumpToRing2();
	void stJumpToRing3();
	void stJumpToRing4();
	void setupJumpToRing();
	void stHangOnRing();
	void stHoldRing3();
	void stDropFromRing();
	void stJumpToRingVenusFlyTrap();
	void stJumpAndFall();
	void stMoveVenusFlyTrap();
	void stContinueMovingVenusFlyTrap();
	void evMoveVenusFlyTrapDone();
	void stPressDoorButton();
	void stHitByBoxingGlove();
	void evHitByBoxingGloveDone();

	uint32 hmJumpToRing(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmJumpToRing3(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmHoldRing3(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmJumpToRingVenusFlyTrap(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmJumpAndFall(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmMoveVenusFlyTrap(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmFirstMoveVenusFlyTrap(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmPressDoorButton(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmHitByBoxingGlove(int messageNum, const MessageParam &param, Entity *sender);

	void xUpdate() override;
	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

class KmScene1004 : public Klaymen {
public:
	KmScene1004(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
protected:
	void stReadNote();
	uint32 hmReadNote(int messageNum, const MessageParam &param, Entity *sender);
	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULES_MODULE1000_SPRITES_H */
