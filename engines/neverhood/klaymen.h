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

#ifndef NEVERHOOD_KLAYMEN_H
#define NEVERHOOD_KLAYMEN_H

#include "neverhood/neverhood.h"
#include "neverhood/sprite.h"
#include "neverhood/graphics.h"
#include "neverhood/resource.h"

namespace Neverhood {

// TODO This code is horrible and weird and a lot of stuff needs renaming once a better name is found
// TODO Also the methods should probably rearranged and be grouped together more consistently

class Scene;

const uint32 kKlaymenSpeedUpHash = 0x004A2148;

#include "common/pack-start.h"	// START STRUCT PACKING

struct KlaymenIdleTableItem {
	int weight;
	uint idleAnimation;
};

#include "common/pack-end.h"	// END STRUCT PACKING

enum {
	kIdlePickEar,
	kIdleSpinHead,
	kIdleArms,
	kIdleChest,
	kIdleHeadOff,
	kIdleTeleporterHands,
	kIdleTeleporterHands2,
	kIdleWonderAbout
};

class Klaymen : public AnimatedSprite {
public:
	Klaymen(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y, NRectArray *clipRects = NULL);

	void update();

	void startIdleAnimation(uint32 fileHash, AnimationCb callback);
	void upIdleAnimation();

	// Idle animations - start
	void stIdlePickEar();
	void evIdlePickEarDone();
	void stIdleSpinHead();
	void stIdleArms();
	void evIdleArmsDone();
	void stIdleChest();
	void stIdleHeadOff();
	void stIdleWonderAbout();
	void stIdleTeleporterHands();
	void stIdleTeleporterHands2();

	uint32 hmIdlePickEar(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmIdleSpinHead(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmIdleArms(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmIdleChest(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmIdleHeadOff(int messageNum, const MessageParam &param, Entity *sender);
	// Idle animations - end

	void stTryStandIdle();
	void stStandAround();
	void upStandIdle();
	void stIdleBlink();

	bool stStartAction(AnimationCb callback3);
	bool stStartActionFromIdle(AnimationCb callback);
	uint32 hmStartAction(int messageNum, const MessageParam &param, Entity *sender);

	void stSneak();
	uint32 hmSneaking(int messageNum, const MessageParam &param, Entity *sender);
	void suSneaking();
	void evSneakingDone();

	void stStartWalking();
	void stStartWalkingExt();
	void stWalkingOpenDoor();
	void suWalkingOpenDoor();
	void stStepOver();
	uint32 hmStartWalking(int messageNum, const MessageParam &param, Entity *sender);
	void evStartWalkingDone();

	void stWalkingFirst();
	void stWalkingFirstExt();
	void stStartWalkingResume();
	void stUpdateWalkingFirst();
	uint32 hmWalking(int messageNum, const MessageParam &param, Entity *sender);
	void suWalkingFirst();

	void stWalkToFrontNoStep();
	void stWalkToFront();
	void stTurnToFront();
	void stTurnToBack();
	uint32 hmWalkToFront(int messageNum, const MessageParam &param, Entity *sender);

	void stTurnToBackToUse();
	uint32 hmTurnToBackToUse(int messageNum, const MessageParam &param, Entity *sender);

	void stPickUpGeneric();
	void stPickUpNeedle();
	uint32 hmPickUpObject(int messageNum, const MessageParam &param, Entity *sender);

	void stPickUpTube();
	uint32 hmPickUpTube(int messageNum, const MessageParam &param, Entity *sender);

	void stTurnToUse();
	void stTurnToUseHalf();
	void stTurnAwayFromUse();
	void stReturnFromUse();
	void stTurnToUseExt();
	uint32 hmTurnToUse(int messageNum, const MessageParam &param, Entity *sender);

	void stLargeStep();
	uint32 hmLargeStep(int messageNum, const MessageParam &param, Entity *sender);
	void suLargeStep();
	void evLargeStepDone();

	void stInsertKey();
	uint32 hmInsertKey(int messageNum, const MessageParam &param, Entity *sender);

	void stPeekWall();
	uint32 hmPeekWall(int messageNum, const MessageParam &param, Entity *sender);

	void stPeekWallReturn();
	uint32 hmPeekWallReturn(int messageNum, const MessageParam &param, Entity *sender);

	void stPeekWallBlink();
	void upPeekWallBlink();

	void stPeekWall1();
	void stPeekWall2();

	void stGrow();
	uint32 hmGrow(int messageNum, const MessageParam &param, Entity *sender);

	void stDrinkPotion();
	uint32 hmDrinkPotion(int messageNum, const MessageParam &param, Entity *sender);

	void stPullCord();
	void stReleaseCord();
	uint32 hmPullReleaseCord(int messageNum, const MessageParam &param, Entity *sender);

	void stUseTube();
	uint32 hmUseTube(int messageNum, const MessageParam &param, Entity *sender);

	void stUseLever();
	void stUseLeverRelease();
	void stReleaseLever();
	void stInteractLever();
	uint32 hmLever(int messageNum, const MessageParam &param, Entity *sender);

	void stLetGoOfLever();
	void evLeverReleasedEvent();

	void stPressButton();
	void stPressFloorButton();
	void stPressButtonSide();
	uint32 hmPressButton(int messageNum, const MessageParam &param, Entity *sender);

	void stWonderAbout();
	void stWonderAboutHalf();
	void stWonderAboutAfter();

	void stStandWonderAbout();

	void stStartClimbLadderUp();
	void stStartClimbLadderDown();
	uint32 hmClimbLadderUpDown(int messageNum, const MessageParam &param, Entity *sender);

	void stContinueClimbLadderUp();

	void stClimbLadderHalf();
	uint32 hmClimbLadderHalf(int messageNum, const MessageParam &param, Entity *sender);

	void stReleaseRing();

	void stLandOnFeet();
	uint32 hmLandOnFeet(int messageNum, const MessageParam &param, Entity *sender);

	void stPullLeverDown();
	void stHoldLeverDown();

	void stInsertDisk();
	uint32 hmInsertDisk(int messageNum, const MessageParam &param, Entity *sender);

	void stMoveObjectSkipTurnFaceObject();
	void stMoveObjectSkipTurn();
	void stMoveObjectFaceObject();
	uint32 hmMoveObjectTurn(int messageNum, const MessageParam &param, Entity *sender);
	void evMoveObjectTurnDone();

	void stJumpToGrab();
	void stJumpToGrabFall();
	uint32 hmJumpToGrab(int messageNum, const MessageParam &param, Entity *sender);
	void suJumpToGrab();

	void stJumpToGrabRelease();
	uint32 hmJumpToGrabRelease(int messageNum, const MessageParam &param, Entity *sender);

	void stSitInTeleporter();
	uint32 hmSitInTeleporter(int messageNum, const MessageParam &param, Entity *sender);

	void stSitIdleTeleporter();
	void upSitIdleTeleporter();
	void stSitIdleTeleporterBlink();
	void stSitIdleTeleporterBlinkSecond();

	void stTurnToUseInTeleporter();
	void stReturnFromUseInTeleporter();
	void stGetUpFromTeleporter();

	void teleporterAppear(uint32 fileHash);
	void teleporterDisappear(uint32 fileHash);
	uint32 hmTeleporterAppearDisappear(int messageNum, const MessageParam &param, Entity *sender);

	void stFallSkipJump();
	void suFallSkipJump();

	void stMoveObject();
	void stContinueMoveObject();
	uint32 hmMoveObject(int messageNum, const MessageParam &param, Entity *sender);
	void upMoveObject();

	void stStandIdleSmall();
	void stWonderAboutSmall();
	void stWonderAboutHalfSmall();
	void stWonderAboutAfterSmall();

	void stWalkToFrontNoStepSmall();
	void stWalkToFrontSmall();
	void stWalkToFront2Small();
	void stTurnToBackHalfSmall();
	void stTurnToBackWalkSmall();
	void stTurnToBackSmall();
	uint32 hmWalkFrontBackSmall(int messageNum, const MessageParam &param, Entity *sender);

	void stFinishGrow();
	uint32 hmFinishGrow(int messageNum, const MessageParam &param, Entity *sender);

	void stStandIdleSpecial();
	uint32 hmStandIdleSpecial(int messageNum, const MessageParam &param, Entity *sender);

	void stSpitOutFall0();
	void stSpitOutFall2();
	void suFallDown();
	void upSpitOutFall();

	void stFalling();
	void stFallTouchdown();

	void stPeekInside();
	void stPeekInsideReturn();
	void stPeekInsideBlink();
	void upPeekInsideBlink();

	////////////////////////////////////////////////////////////////////////////

	void stopWalking();

	void suAction();
	void suUpdateDestX();
	void suWalkingTestExit();

	uint32 hmLowLevel(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmLowLevelAnimation(int messageNum, const MessageParam &param, Entity *sender);

	void setKlaymenIdleTable(const KlaymenIdleTableItem *table, uint tableCount);
	void setKlaymenIdleTable1();
	void setKlaymenIdleTable2();
	void setKlaymenIdleTable3();

	void setSoundFlag(bool value) { _soundFlag = value; }

	void stRidePlatform();
	void suRidePlatform();
	void stPullLever();
	void stLookLeverDown();
	void stWaitLeverDown();

protected:
	Scene *_parentScene;
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
	int16 _tapesToInsert, _keysToInsert;
	bool _doYHitIncr;
	bool _isLeverDown;
	bool _isWalkingOpenDoorNotified;
	int _busyStatus;
	bool _actionStatusChanged;
	int _actionStatus;

	const KlaymenIdleTableItem *_idleTable;
	uint _idleTableCount;
	int _idleTableTotalWeight;
	int _idleTableNum;

	NPointArray *_pathPoints;
	bool _soundFlag;

	int16 _spitOutCountdown;

	bool _isSittingInTeleporter;

	bool _potionFlag1;
	bool _potionFlag2;

	int16 _platformDeltaY;

	Sprite *_otherSprite;

	int16 _walkResumeFrameIncr;

	int _moveObjectCountdown;

	virtual void xUpdate();
	virtual uint32 xHandleMessage(int messageNum, const MessageParam &param);

	void startWalkToX(int16 x, bool walkExt);
	void startWalkToXExt(int16 x);
	void startWalkToXSmall(int16 x);
	void startSpecialWalkLeft(int16 x);
	void startSpecialWalkRight(int16 x);
	void startWalkToXDistance(int16 destX, int16 distance);
	void startWalkToAttachedSpriteXDistance(int16 distance);

	void gotoNextStateExt();
	void beginAction();
	void endAction();

	void stStartWalkingSmall();
	uint32 hmWalkingSmall(int messageNum, const MessageParam &param, Entity *sender);

	void enterIdleAnimation(uint idleAnimation);
	void walkAlongPathPoints();
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_KLAYMEN_H */
