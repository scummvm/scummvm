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

// TODO This code is horrible and weird and a lot of stuff needs renaming once a better name is found
// TODO Also the methods should probably rearranged and be grouped together more consistently

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
	void stDoIdleSpinHead();
	void stDoIdleArms();
	void stDoIdleChest();
	void stDoIdleHeadOff();
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
	void stHangOnRing();
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
	void stSitIdleTeleporter();
	void stIdleSitBlink();
	void stIdleSitBlinkSecond();
	void stPickUpNeedle();
	void stPickUpTube();
	void stTurnToUseInTeleporter();
	void stReturnFromUseInTeleporter();
	void stStepOver();
	void stSitInTeleporter();
	void stGetUpFromTeleporter();
	void stMoveObjectSkipTurnFaceObject();
	void evMoveObjectTurnDone();
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

	void stopWalking();
	
	void spriteUpdate41F250();
	void suWalking();
	void suLargeStep();
	void suUpdateDestX();
	void suWalkingOpenDoor();
	void spriteUpdate41F300();
	void suWalkingTestExit();

	uint32 handleMessage41D360(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage41D480(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmWalking(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage41E210(int messageNum, const MessageParam &param, Entity *sender);

	void setKlaymanIdleTable(const KlaymanIdleTableItem *table, int tableCount);
	void setKlaymanIdleTable1();
	void setKlaymanIdleTable2();
	void setKlaymanIdleTable3();
	
	void setSoundFlag(bool value) { _soundFlag = value; }

	void stStandIdleSmall();
	void stWonderAboutAfterSmall();
	void stWonderAboutHalfSmall();
	void stWonderAboutSmall();
	void stWalkToFrontNoStepSmall();
	void stWalkToFront2Small();
	void stWalkToFrontSmall();
	void stTurnToBackHalfSmall();
	void stTurnToBackWalkSmall();
	void stTurnToBackSmall();
	void stPullCord();
	void stReleaseCord();
	uint32 hmPullCord(int messageNum, const MessageParam &param, Entity *sender);
	void stUseTube();
	uint32 hmUseTube(int messageNum, const MessageParam &param, Entity *sender);
	void stWalkingExt();
	void stStartWalkingExt();
	
	void stJumpToGrab();
	void suJumpToGrab();
	uint32 hmJumpToGrab(int messageNum, const MessageParam &param, Entity *sender);
	
	void sub421230();//stGrow??
	uint32 handleMessage41F1D0(int messageNum, const MessageParam &param, Entity *sender);
	
	void stTurnToUseExt();
	void stJumpToGrabFall();
	
	void stJumpToGrabRelease();
	uint32 hmJumpToGrabRelease(int messageNum, const MessageParam &param, Entity *sender);
	
	void stDoIdleTeleporterHands();
	void stIdleTeleporterHands();
	
	void stDoIdleTeleporterHands2();
	void stIdleTeleporterHands2();

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
	int16 _idleCounter, _idleCounterMax;
	int16 _blinkCounter, _blinkCounterMax;
	int16 _countdown1;
	int16 _tapesToInsert;
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
	virtual void xUpdate();
	virtual uint32 xHandleMessage(int messageNum, const MessageParam &param);

	void stIdlePickEar();
	void evIdlePickEarDone();
	uint32 hmIdlePickEar(int messageNum, const MessageParam &param, Entity *sender);

	void stIdleSpinHead();
	uint32 hmIdleSpinHead(int messageNum, const MessageParam &param, Entity *sender);

	void stIdleArms();
	void evIdleArmsDone();
	uint32 hmIdleArms(int messageNum, const MessageParam &param, Entity *sender);
	
	void stIdleChest();
	uint32 hmIdleChest(int messageNum, const MessageParam &param, Entity *sender);
	
	void stIdleHeadOff();
	uint32 hmIdleHeadOff(int messageNum, const MessageParam &param, Entity *sender);

	void startIdleAnimation(uint32 fileHash, AnimationCb callback);
	void upIdleAnimation();

	bool stStartActionFromIdle(AnimationCb callback);
	void gotoNextStateExt();
	void sub41C770();
	void sub41C790();
	
	void upIdle();

	void stIdleBlink();
	void stStandAround();
	
	uint32 hmStartAction(int messageNum, const MessageParam &param, Entity *sender);

	void startWalkToX(int16 x, bool flag);

	uint32 hmSleeping(int messageNum, const MessageParam &param, Entity *sender);
	
	bool stStartAction(AnimationCb callback3);
	
	void stSneak();
	void stWalkingDone();
	uint32 hmSneaking(int messageNum, const MessageParam &param, Entity *sender);
	void startSpecialWalkLeft(int16 x);
	void stStartWalking();
	uint32 hmStartWalking(int messageNum, const MessageParam &param, Entity *sender);

	uint32 hmPickUpGeneric(int messageNum, const MessageParam &param, Entity *sender);
	
	uint32 hmPressButton(int messageNum, const MessageParam &param, Entity *sender);
	
	void startSpecialWalkRight(int16 x);
	void sub41CC40(int16 x1, int16 x2);
	void startWalkToXExt(int16 x);
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
	
	void upSitIdleTeleporter();
	
	uint32 hmInsertDisk(int messageNum, const MessageParam &param, Entity *sender);
	
	void startWalkToXSmall(int16 x);
	void stStartWalkingSmall();
	uint32 hmWalkingSmall(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmWalkFrontBackSmall(int messageNum, const MessageParam &param, Entity *sender);
	
	void walkAlongPathPoints();
	
};

class KmScene1001 : public Klayman {
public:
	KmScene1001(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:	
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	void stPullLever();
	uint32 hmPullLever(int messageNum, const MessageParam &param, Entity *sender);
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
	uint32 hmJumpToRingVenusFlyTrap(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmStandIdleSpecial(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmPressDoorButton(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmMoveVenusFlyTrap(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmFirstMoveVenusFlyTrap(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage449C90(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage449D60(int messageNum, const MessageParam &param, Entity *sender);
	void suFallDown();
	void stJumpToRingVenusFlyTrap();
	void stStandIdleSpecial();
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
	uint32 hmTeleporterAppearDisappear(int messageNum, const MessageParam &param, Entity *sender);
	void stTeleporterAppear();
	void stTeleporterDisappear();
};

class KmScene1201 : public Klayman {
public:
	KmScene1201(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y, Entity *class464);
protected:
	Entity *_class464;
	int _countdown;
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	void upMoveObject();
	uint32 hmMatch(int messageNum, const MessageParam &param, Entity *sender);
	void stFetchMatch();
	void stLightMatch();
	uint32 hmMoveObject(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmTumbleHeadless(int messageNum, const MessageParam &param, Entity *sender);
	void stMoveObject();
	void stMoveObjectSkipTurn();
	void stTumbleHeadless();
	void stCloseEyes();
};

class KmScene1303 : public Klayman {
public:
	KmScene1303(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	uint32 hmPeekWallReturn(int messageNum, const MessageParam &param, Entity *sender);
	void upPeekWallBlink();
	void stPeekWall1();
	void stPeekWall2();
	void stPeekWallBlink();
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
	uint32 hmTeleporterAppearDisappear(int messageNum, const MessageParam &param, Entity *sender);
	void stTeleporterAppear();
	void stTeleporterDisappear();
};

class KmScene1308 : public Klayman {
public:
	KmScene1308(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	bool _flag1;
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	uint32 hmInsertDiskSide(int messageNum, const MessageParam &param, Entity *sender);
	void stInsertDiskSide();
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
	uint32 hmTeleporterAppearDisappear(int messageNum, const MessageParam &param, Entity *sender);
	void suFallSkipJump();
	void stFallSkipJump();
	void stTeleporterAppear();
	void stTeleporterDisappear();
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
	uint32 hmTeleporterAppearDisappear(int messageNum, const MessageParam &param, Entity *sender);
	void stTeleporterAppear();
	void stTeleporterDisappear();
};

class KmScene2101 : public Klayman {
public:
	KmScene2101(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	bool _isSittingInTeleporter;
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	uint32 hmHitByDoor(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmTeleporterAppearDisappear(int messageNum, const MessageParam &param, Entity *sender);
	void stHitByDoor();
	void stTeleporterAppear();
	void stTeleporterDisappear();
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
	void stStartWalkingResume();
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
	void suRidePlatformDown();
	void stRidePlatformDown();
	void stStartWalkingResume();
};

class KmScene2207 : public Klayman {
public:
	KmScene2207(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	void suRidePlatform();
	void stRidePlatform();
	void stInteractLever();
	void stPullLever();
	void stLookLeverDown();
	void stWaitLeverDown();
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

class KmScene2401 : public Klayman {
public:
	KmScene2401(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	bool _canSpit;
	bool _contSpit;
	bool _readyToSpit;
	uint32 _spitPipeIndex;
	uint32 _spitDestPipeIndex;
	uint32 _spitContDestPipeIndex;
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void stTrySpitIntoPipe();
	void spitIntoPipe();
	void stContSpitIntoPipe();
};

class KmScene2402 : public Klayman {
public:
	KmScene2402(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	void sub415840();
};

class KmScene2403 : public Klayman {
public:
	KmScene2403(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
};

class KmScene2406 : public Klayman {
public:
	KmScene2406(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y, NRect *clipRects, int clipRectsCount);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
};

class KmScene2501 : public Klayman {
public:
	KmScene2501(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	bool _isSittingInTeleporter;
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
};

class KmScene2801 : public Klayman {
public:
	KmScene2801(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
};

class KmScene2803 : public Klayman {
public:
	KmScene2803(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y, NRect *clipRects, int clipRectsCount);
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

class KmScene2806 : public Klayman {
public:
	KmScene2806(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y,
		bool flag, NRect *clipRects, uint clipRectsCount);
protected:
	SoundResource _soundRes1;
	SoundResource _soundRes2;
	SoundResource _soundRes3;
	SoundResource _soundRes4;
	SoundResource _soundRes5;
	bool _flag1;
	bool _flag2;
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	uint32 handleMessage40F1F0(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage40F570(int messageNum, const MessageParam &param, Entity *sender);
	void sub40F780();
	void sub40F7C0();
};

class KmScene2809 : public Klayman {
public:
	KmScene2809(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y,
		bool flag, NRect *clipRects, uint clipRectsCount);
protected:
	SoundResource _soundRes1;
	SoundResource _soundRes2;
	SoundResource _soundRes3;
	SoundResource _soundRes4;
	SoundResource _soundRes5;
	bool _flag1;
	bool _flag2;
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	uint32 handleMessage457FC0(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage458340(int messageNum, const MessageParam &param, Entity *sender);
	void sub458550();
	void sub458590();
};

class KmScene2810Small : public Klayman {
public:
	KmScene2810Small(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
};

class KmScene2810 : public Klayman {
public:
	KmScene2810(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y,
		NRect *clipRects, uint clipRectsCount);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
};

class KmScene2812 : public Klayman {
public:
	KmScene2812(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_KLAYMAN_H */
