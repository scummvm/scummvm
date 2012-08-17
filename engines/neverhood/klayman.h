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

#ifndef NEVERHOOD_KLAYMAN_H
#define NEVERHOOD_KLAYMAN_H

#include "neverhood/neverhood.h"
#include "neverhood/sprite.h"
#include "neverhood/graphics.h"
#include "neverhood/resource.h"

namespace Neverhood {

// TODO: This code is horrible and weird and a lot of stuff needs renaming once a better name is found

class Klayman;

const uint32 kKlaymanSpeedUpHash = 0x004A2148;

#include "common/pack-start.h"	// START STRUCT PACKING

struct KlaymanIdleTableItem {
	int value;
	void (Klayman::*callback)();
};

#include "common/pack-end.h"	// END STRUCT PACKING

class Klayman : public AnimatedSprite {
public:
	Klayman(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y, int surfacePriority = 1000, int objectPriority = 1000, NRectArray *clipRects = NULL);

	void update();

	void stDoIdlePickEar();
	void sub41FDA0();
	void sub41FDF0();
	void stDoIdleChest();
	void sub41FEB0();
	void stTryStandIdle();
	void stWakeUp();
	void stSleeping();
	void stPickUpGeneric();
	void stTurnPressButton();
	void stStampFloorButton();
	void stPressButtonSide();
	void stLargeStep();
	void stWonderAboutHalf();
	void stWonderAboutAfter();
	void stTurnToUseHalf();
	void stTurnAwayFromUse();
	void stWonderAbout();
	void stPeekWall();
	void stJumpToRing1();
	void setupJumpToRing();
	void sub420340();
	void stJumpToRing2();
	void stJumpToRing3();
	void stHoldRing();
	void stReleaseRing();
	void stJumpToRing4();
	void stContinueClimbLadderUp();
	void stStartClimbLadderDown();
	void stClimbLadderHalf();
	void stStartClimbLadderUp();
	void stWalkToFrontNoStep();
	void stWalkToFront();
	void stTurnToFront();
	void stTurnToBack();
	void stLandOnFeet();
	void stTurnToBackToUse();
	void stClayDoorOpen();
	void stTurnToUse();
	void stReturnFromUse();
	void stWalkingOpenDoor();
	void sub421350();
	void stIdleSitBlink();
	void stIdleSitBlinkSecond();
	void stPickUpNeedle();
	void sub41FFF0();
	void stTurnToUseInTeleporter();
	void stReturnFromUseInTeleporter();
	void stStepOver();
	void stSitInTeleporter();
	void stGetUpFromTeleporter();
	void stMoveObjectSkipTurnFaceObject();
	void sub420660();
	void stMoveObjectSkipTurn();
	void stMoveObjectFaceObject();
	void stUseLever();
	void stPullLeverDown();
	void stHoldLeverDown();
	void stUseLeverRelease();
	void stReleaseLever();
	void stLetGoOfLever();
	void cbLeverReleasedEvent();
	void stInsertDisk();
	void stWalking();
	void stUpdateWalking();
	void stStartWalkingDone();

	void sub41CE70();
	
	void spriteUpdate41F250();
	void suWalking();
	void suLargeStep();
	void spriteUpdate41F230();
	void suWalkingOpenDoor();
	void spriteUpdate41F300();
	void suWalkingTestExit();

	uint32 handleMessage41D360(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage41D480(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage41EB70(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage41E210(int messageNum, const MessageParam &param, Entity *sender);

	void setKlaymanIdleTable(const KlaymanIdleTableItem *table, int tableCount);
	void setKlaymanIdleTable1();
	void setKlaymanIdleTable2();
	void setKlaymanIdleTable3();
	
	void setSoundFlag(bool value) { _soundFlag = value; }

	void sub421640();
	void sub421740();
	void sub421780();
	void sub421700();
	void sub421840();
	void sub421800();
	void sub4217C0();
	void sub421900();
	void sub4218C0();
	void sub421880();

protected:
	Entity *_parentScene;
	Sprite *_attachedSprite;
	int _ladderStatus;
	bool _isWalking;
	bool _isSneaking;
	bool _isLargeStep;
	bool _isMoveObjectRequested;
	bool _acceptInput;
	int16 _destX, _destY;
	int16 _counter, _counterMax;
	int16 _counter3, _counter3Max;
	int16 _counter1;
	int16 _counter2;
	bool _flagF6;
	bool _isLeverDown;
	bool _isWalkingOpenDoorNotified;
	int _status2;
	bool _flagFA;
	SoundResource _soundResource1;
	SoundResource _soundResource2;
	int _status3;
	const KlaymanIdleTableItem *_idleTable;
	int _idleTableCount;
	int _idleTableMaxValue;
	NPointArray *_pathPoints;
	bool _soundFlag;
	int _resourceHandle;
	virtual void xUpdate();
	virtual uint32 xHandleMessage(int messageNum, const MessageParam &param);

	void stIdlePickEar();
	void evIdlePickEarDone();
	uint32 hmIdlePickEar(int messageNum, const MessageParam &param, Entity *sender);

	void sub41FDB0();
	uint32 handleMessage41E980(int messageNum, const MessageParam &param, Entity *sender);

	void sub41FE00();
	void sub41FE50();
	uint32 handleMessage41E9E0(int messageNum, const MessageParam &param, Entity *sender);
	
	void stIdleChest();
	uint32 hmIdleChest(int messageNum, const MessageParam &param, Entity *sender);
	
	void sub41FEC0();
	uint32 handleMessage41EFE0(int messageNum, const MessageParam &param, Entity *sender);

	void startIdleAnimation(uint32 fileHash, AnimationCb callback);
	void upIdleAnimation();

	bool stStartActionFromIdle(AnimationCb callback);
	void gotoNextStateExt();
	void sub41C770();
	void sub41C790();
	
	void update41D0F0();

	void stStand();
	void stStandAround();
	
	uint32 hmStartAction(int messageNum, const MessageParam &param, Entity *sender);

	void startWalkToX(int16 x, bool flag);

	uint32 hmSleeping(int messageNum, const MessageParam &param, Entity *sender);
	
	bool stStartAction(AnimationCb callback3);
	
	void stSneak();
	void stWalkingDone();
	uint32 hmWalking(int messageNum, const MessageParam &param, Entity *sender);
	void startSpecialWalkLeft(int16 x);
	void stStartWalking();
	uint32 hmStartWalking(int messageNum, const MessageParam &param, Entity *sender);

	uint32 hmPickUpGeneric(int messageNum, const MessageParam &param, Entity *sender);
	
	uint32 hmPressButton(int messageNum, const MessageParam &param, Entity *sender);
	
	void startSpecialWalkRight(int16 x);
	void sub41CC40(int16 x1, int16 x2);
	void sub41CAC0(int16 x);
	void sub41CCE0(int16 x);
	void stLargeStepDone();
	
	uint32 hmLargeStep(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmTurnToUse(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmPeekWall(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmJumpToRing(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmJumpToRing3(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmHoldRing(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage41DFD0(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmClimbLadderUpDown(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmWalkToFront(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmLandOnFeet(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmTurnToBackToUse(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage41D640(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmSitInTeleporter(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmMoveObjectTurn(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmClayDoorOpen(int messageNum, const MessageParam &param, Entity *sender);
	
	void update41D1C0();
	
	uint32 hmInsertDisk(int messageNum, const MessageParam &param, Entity *sender);
	
	void sub41CDE0(int16 x);
	void sub421680();
	uint32 handleMessage41ED70(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage41EE00(int messageNum, const MessageParam &param, Entity *sender);
	
	void walkAlongPathPoints();
	
};

class KmScene1001 : public Klayman {
public:
	KmScene1001(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:	
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	void sub44FA50();
	uint32 handleMessage44FA00(int messageNum, const MessageParam &param, Entity *sender);
};

class KmScene1002 : public Klayman {
public:
	KmScene1002(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y, Sprite *class599, Sprite *ssLadderArch);
protected:
	Sprite *_class599;
	Sprite *_ssLadderArch;
	Sprite *_otherSprite;
	int _status;
	void xUpdate();	
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	void update4497D0();
	uint32 handleMessage449800(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage4498E0(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmPressDoorButton(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmMoveVenusFlyTrap(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmFirstMoveVenusFlyTrap(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage449C90(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage449D60(int messageNum, const MessageParam &param, Entity *sender);
	void suFallDown();
	void sub449E20();
	void sub449E90();
	void sub449EF0();
	void sub449F70();
	void stSpitOutFall();
	void sub44A0D0();
	void sub44A150();
	void sub44A230();
	void stJumpAndFall();
	void stDropFromRing();
	void stPressDoorButton();
	void stHitByBoxingGlove();
	void stHitByBoxingGloveDone();
	void stMoveVenusFlyTrap();
	void stContinueMovingVenusFlyTrap();
	void stMoveVenusFlyTrapDone();
};

class KmScene1004 : public Klayman {
public:
	KmScene1004(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	uint32 hmReadNote(int messageNum, const MessageParam &param, Entity *sender);
	void stReadNote();
};

class KmScene1109 : public Klayman {
public:
	KmScene1109(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	bool _isSittingInTeleporter;
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	uint32 handleMessage461EA0(int messageNum, const MessageParam &param, Entity *sender);
	void sub461F30();
	void sub461F70();
};

class KmScene1201 : public Klayman {
public:
	KmScene1201(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y, Entity *class464);
protected:
	Entity *_class464;
	int _countdown;
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	void update40DBE0();
	uint32 hmMatch(int messageNum, const MessageParam &param, Entity *sender);
	void stFetchMatch();
	void stLightMatch();
	uint32 hmMoveObject(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmTumbleHeadless(int messageNum, const MessageParam &param, Entity *sender);
	void stMoveObject();
	void stMoveObjectSkipTurn();
	void stTumbleHeadless();
	void sub40E040();
};

class KmScene1303 : public Klayman {
public:
	KmScene1303(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	uint32 hmPeekWallReturn(int messageNum, const MessageParam &param, Entity *sender);
	void update4161A0();
	void stPeekWall1();
	void stPeekWall2();
	void stPeekWall3();
	void stPeekWallReturn();
};

class KmScene1304 : public Klayman {
public:
	KmScene1304(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
};

class KmScene1305 : public Klayman {
public:
	KmScene1305(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	void stCrashDown();
	void cbCrashDownEvent();
};

class KmScene1306 : public Klayman {
public:
	KmScene1306(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	bool _isSittingInTeleporter;
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	uint32 handleMessage417CB0(int messageNum, const MessageParam &param, Entity *sender);
	void sub417D40();
	void sub417D80();
};

class KmScene1308 : public Klayman {
public:
	KmScene1308(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	bool _flag1;
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void sub456150();
};

class KmScene1401 : public Klayman {
public:
	KmScene1401(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
};

class KmScene1402 : public Klayman {
public:
	KmScene1402(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
};

class KmScene1403 : public Klayman {
public:
	KmScene1403(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
};

class KmScene1404 : public Klayman {
public:
	KmScene1404(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
};

class KmScene1608 : public Klayman {
public:
	KmScene1608(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	bool _isSittingInTeleporter;
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
};

class KmScene1705 : public Klayman {
public:
	KmScene1705(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	bool _isSittingInTeleporter;
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	uint32 handleMessage4689A0(int messageNum, const MessageParam &param, Entity *sender);
	void spriteUpdate468A30();
	void stFallSkipJump();
	void sub468AD0();
	void sub468B10();
};

class KmScene1901 : public Klayman {
public:
	KmScene1901(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
};

class KmScene2001 : public Klayman {
public:
	KmScene2001(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	bool _isSittingInTeleporter;
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	uint32 handleMessage4401A0(int messageNum, const MessageParam &param, Entity *sender);
	void sub440230();
	void stDoTeleport();
};

class KmScene2101 : public Klayman {
public:
	KmScene2101(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	bool _isSittingInTeleporter;
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	uint32 handleMessage486160(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage486230(int messageNum, const MessageParam &param, Entity *sender);
	void sub4862C0();
	void sub486320();
	void sub486360();
};

class KmScene2201 : public Klayman {
public:
	KmScene2201(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y, NRect *clipRects, int clipRectsCount);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
};

class KmScene2203 : public Klayman {
public:
	KmScene2203(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
};

class KmScene2205 : public Klayman {
public:
	KmScene2205(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
	void sub423980();
protected:
	void xUpdate();
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
};

class KmScene2206 : public Klayman {
public:
	KmScene2206(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
	~KmScene2206();
protected:
	int16 _yDelta;
	void xUpdate();
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	void spriteUpdate482450();
	void sub482490();
	void sub482530();
};

class KmScene2207 : public Klayman {
public:
	KmScene2207(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	void spriteUpdate442430();
	void sub442460();
	void sub4424B0();
	void sub442520();
	void sub442560();
	void sub4425A0();
};

class KmScene2242 : public Klayman {
public:
	KmScene2242(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	void xUpdate();
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	void sub444D20();
};

class KmHallOfRecords : public Klayman {
public:
	KmHallOfRecords(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	void xUpdate();
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	void sub43B130();
};

class KmScene2247 : public Klayman {
public:
	KmScene2247(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	void xUpdate();
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	void sub453520();
};

class KmScene2801 : public Klayman {
public:
	KmScene2801(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
};

class KmScene2803b : public Klayman {
public:
	KmScene2803b(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	SoundResource _soundResource;
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	uint32 handleMessage460600(int messageNum, const MessageParam &param, Entity *sender);
	void sub460670();
};

class KmScene2805 : public Klayman {
public:
	KmScene2805(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	bool _isSittingInTeleporter;
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	uint32 handleMessage404800(int messageNum, const MessageParam &param, Entity *sender);
	void sub404890();
	void sub4048D0();
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_KLAYMAN_H */
