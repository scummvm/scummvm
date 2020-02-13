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

#ifndef GNAP_GROUP2_H
#define GNAP_GROUP2_H

#include "gnap/debugger.h"

namespace Gnap {

enum {
	kHS20Platypus				= 0,
	kHS20GroceryStoreHat		= 1,
	kHS20ExitParkingLot			= 2,
	kHS20StonerGuy				= 3,
	kHS20GroceryStoreGuy		= 4,
	kHS20Device					= 5,
	kHS20ExitInsideGrubCity		= 6,
	kHS20ExitOutsideCircusWorld	= 7,
	kHS20ExitOutsideToyStore	= 8,
	kHS20ExitPhone				= 9,
	kHS20WalkArea1				= 10,
	kHS20WalkArea2				= 11
};

enum {
	kHS21Platypus				= 0,
	kHS21Banana					= 1,
	kHS21OldLady				= 2,
	kHS21Device					= 3,
	kHS21ExitOutsideGrubCity	= 4,
	kHS21WalkArea1				= 5,
	kHS21WalkArea2				= 6
};

enum {
	kHS22Platypus				= 0,
	kHS22ExitOutsideGrubCity	= 1,
	kHS22ExitBackGrubCity		= 2,
	kHS22Cashier				= 3,
	kHS22Device					= 4,
	kHS22WalkArea1				= 5,
	kHS22WalkArea2				= 6
};

enum {
	kHS23Platypus			= 0,
	kHS23ExitFrontGrubCity	= 1,
	kHS23Device				= 2,
	kHS23Cereals			= 3,
	kHS23WalkArea1			= 4,
	kHS23WalkArea2			= 5
};

enum {
	kHS24Platypus				= 0,
	kHS24ExitCircusWorld		= 1,
	kHS24ExitOutsideGrubCity	= 2,
	kHS24Device					= 3,
	kHS24WalkArea1				= 4,
	kHS24WalkArea2				= 5,
	kHS24WalkArea3				= 6
};

enum {
	kHS25Platypus				= 0,
	kHS25TicketVendor			= 1,
	kHS25ExitOutsideCircusWorld	= 2,
	kHS25ExitInsideCircusWorld	= 3,
	kHS25Device					= 4,
	kHS25Posters1				= 5,
	kHS25Posters2				= 6,
	kHS25Posters3				= 7,
	kHS25Posters4				= 8,
	kHS25WalkArea1				= 9,
	kHS25WalkArea2				= 10
};

enum {
	kHS26Platypus				= 0,
	kHS26ExitOutsideCircusWorld	= 1,
	kHS26ExitOutsideClown		= 2,
	kHS26ExitArcade				= 3,
	kHS26ExitElephant			= 4,
	kHS26ExitBeerStand			= 5,
	kHS26Device					= 6,
	kHS26WalkArea1				= 7,
	kHS26WalkArea2				= 8
};

enum {
	kHS27Platypus		= 0,
	kHS27Janitor		= 1,
	kHS27Device			= 2,
	kHS27Bucket			= 3,
	kHS27ExitCircus		= 4,
	kHS27ExitArcade		= 5,
	kHS27ExitBeerStand	= 6,
	kHS27ExitClown		= 7,
	kHS27WalkArea1		= 8
};

enum {
	kHS28Platypus			= 0,
	kHS28Horn				= 1,
	kHS28Clown				= 2,
	kHS28ExitOutsideClown	= 3,
	kHS28EmptyBucket		= 4,
	kHS28Device				= 5,
	kHS28WalkArea1			= 6,
	kHS28WalkArea2			= 7
};

enum {
	kHS29Platypus			= 0,
	kHS29Monkey				= 1,
	kHS29Device				= 2,
	kHS29ExitCircus			= 3,
	kHS29ExitOutsideClown	= 4,
	kHS29Arcade				= 5,
	kHS29WalkArea1			= 6
};

enum {
	kAS20LeaveScene					= 0,
	kAS20TalkStonerGuyNoJoint		= 2,
	kAS20TalkStonerGuyHasJoint		= 3,
	kAS20GrabJoint					= 4,
	kAS20ActionDone					= 5,
	kAS20TalkGroceryStoreGuy		= 6,
	kAS20GrabGroceryStoreGuy		= 9,
	kAS20GrabGroceryStoreHat		= 10,
	kAS20SwitchGroceryStoreHat		= 11,
	kAS20SwitchGroceryStoreHatDone	= 12,
	kAS20GrabJointDone				= 13
};

enum {
	kAS21TalkOldLady			= 0,
	kAS21GrabBanana				= 1,
	kAS21GrabBananaDone			= 2,
	kAS21GrabOldLady			= 3,
	kAS21UseHatWithOldLady		= 4,
	kAS21UseHatWithOldLadyDone	= 5,
	kAS21LeaveScene				= 6
};

enum {
	kAS22LeaveScene		= 0,
	kAS22TalkCashier	= 1
};

enum {
	kAS23LookCereals		= 0,
	kAS23GrabCereals		= 1,
	kAS23GrabCerealsDone	= 2,
	kAS23LeaveScene			= 3
};

enum {
	kAS24LeaveScene	= 0
};

enum {
	kAS25TalkTicketVendor				= 0,
	kAS25EnterCircusWihoutTicket		= 1,
	kAS25ShowTicketToVendor				= 2,
	kAS25ShowTicketToVendorDone			= 3,
	kAS25EnterCircusWihoutTicketDone	= 4,
	kAS25LeaveScene						= 5
};

enum {
	kAS26LeaveScene	= 0
};

enum {
	kAS27TalkJanitor			= 0,
	kAS27GrabBucket				= 1,
	kAS27GrabBucketDone			= 2,
	kAS27ShowPictureToJanitor	= 3,
	kAS27TryEnterClownTent		= 4,
	kAS27TryEnterClownTentDone	= 5,
	kAS27EnterClownTent			= 6,
	kAS27LeaveScene				= 7
};

enum {
	kAS28UseBeerBucketWithClown	= 0,
	kAS28UsePillBucketWithClown	= 1,
	kAS28GrabHornFails			= 2,
	kAS28GrabEmptyBucket		= 3,
	kAS28GrabHornSuccess		= 4,
	kAS28GrabHornSuccessDone	= 5,
	kAS28GrabEmptyBucketDone	= 6,
	kAS28GrabHornFailsDone		= 7,
	kAS28TalkClown				= 8,
	kAS28GnapWaiting			= 9,
	kAS28LeaveScene				= 10
};

enum {
	kAS29UseBananaWithMonkey	= 0,
	kAS29LeaveScene				= 2
};

class GnapEngine;
class CutScene;

class Scene20: public Scene {
public:
	Scene20(GnapEngine *vm);
	~Scene20() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override;

private:
	int _currStonerGuySequenceId;
	int _nextStonerGuySequenceId;
	int _currGroceryStoreGuySequenceId;
	int _nextGroceryStoreGuySequenceId;
	int _stonerGuyCtr;
	int _groceryStoreGuyCtr;
	bool _stonerGuyShowingJoint;

	void stopSounds();
};

class Scene21: public Scene {
public:
	Scene21(GnapEngine *vm);
	~Scene21() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {};

private:
	int _currOldLadySequenceId;
	int _nextOldLadySequenceId;
};

class Scene22: public Scene {
public:
	Scene22(GnapEngine *vm);
	~Scene22() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {};

private:
	int _currCashierSequenceId;
	int _nextCashierSequenceId;
	bool _caughtBefore;
	int _cashierCtr;
};

class Scene23: public Scene {
public:
	Scene23(GnapEngine *vm);
	~Scene23() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {};

private:
	int _currStoreClerkSequenceId;
	int _nextStoreClerkSequenceId;
};

class Scene24: public Scene {
public:
	Scene24(GnapEngine *vm);
	~Scene24() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {};

private:
	int _currWomanSequenceId;
	int _nextWomanSequenceId;
	int _boySequenceId;
	int _girlSequenceId;
};

class Scene25: public Scene {
public:
	Scene25(GnapEngine *vm);
	~Scene25() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {};

private:
	int _currTicketVendorSequenceId;
	int _nextTicketVendorSequenceId;

	void playAnims(int index);
};

class Scene26: public Scene {
public:
	Scene26(GnapEngine *vm);
	~Scene26() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {};

private:
	int _currKidSequenceId;
	int _nextKidSequenceId;
};

class Scene27: public Scene {
public:
	Scene27(GnapEngine *vm);
	~Scene27() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {};

private:
	int _nextJanitorSequenceId;
	int _currJanitorSequenceId;
};

class Scene28: public Scene {
public:
	Scene28(GnapEngine *vm);
	~Scene28() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {};

private:
	int _currClownSequenceId;
	int _nextClownSequenceId;
	int _clownTalkCtr;
};

class Scene29: public Scene {
public:
	Scene29(GnapEngine *vm);
	~Scene29() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {};

private:
	int _currMonkeySequenceId;
	int _nextMonkeySequenceId;
	int _currManSequenceId;
	int _nextManSequenceId;
};

} // End of namespace Gnap

#endif // GNAP_GROUP1_H
