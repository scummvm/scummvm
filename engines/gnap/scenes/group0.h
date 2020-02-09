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

#ifndef GNAP_GROUP0_H
#define GNAP_GROUP0_H

#include "gnap/debugger.h"

namespace Gnap {

enum {
	kHS01Platypus	= 0,
	kHS01ExitTruck	= 1,
	kHS01Mud		= 2,
	kHS01Pigs		= 3,
	kHS01Spaceship	= 4,
	kHS01Device		= 5,
	kHS01WalkArea1	= 6,
	kHS01WalkArea2	= 7,
	kHS01WalkArea3	= 8,
	kHS01WalkArea4	= 9,
	kHS01WalkArea5	= 10,
	kHS01WalkArea6	= 11,
	kHS01WalkArea7	= 12,
	kHS01WalkArea8	= 13
};

enum {
	kHS02Platypus	= 0,
	kHS02Chicken	= 1,
	kHS02Truck1		= 2,
	kHS02Truck2		= 3,
	kHS02TruckGrill	= 4,
	kHS02Device		= 5,
	kHS02ExitHouse	= 6,
	kHS02ExitBarn	= 7,
	kHS02ExitCreek	= 8,
	kHS02ExitPigpen	= 9,
	kHS02WalkArea1	= 10,
	kHS02WalkArea2	= 11,
	kHS02WalkArea3	= 12,
	kHS02WalkArea4	= 13
};

enum {
	kHS03Platypus			= 0,
	kHS03Grass				= 1,
	kHS03ExitTruck			= 2,
	kHS03Creek				= 3,
	kHS03TrappedPlatypus	= 4,
	kHS03Device				= 5,
	kHS03WalkAreas1			= 6,
	kHS03WalkAreas2			= 7,
	kHS03PlatypusWalkArea	= 8,
	kHS03WalkAreas3			= 9
};

enum {
	kHS04Platypus	= 0,
	kHS04Twig		= 1,
	kHS04Dog		= 2,
	kHS04Axe		= 3,
	kHS04Door		= 4,
	kHS04ExitTruck	= 5,
	kHS04Device		= 6,
	kHS04Window		= 7,
	kHS04ExitBarn	= 8,
	kHS04WalkArea1	= 9,
	kHS04WalkArea2	= 10
};

enum {
	kHS05Platypus	= 0,
	kHS05Haystack	= 1,
	kHS05Padlock	= 2,
	kHS05Ladder		= 3,
	kHS05ExitHouse	= 4,
	kHS05Chicken	= 5,
	kHS05Device		= 6,
	kHS05WalkArea1	= 7,
	kHS05WalkArea2	= 8,
	kHS05WalkArea3	= 9
};

enum {
	kHS06Platypus			= 0,
	kHS06Gas				= 1,
	kHS06Ladder				= 2,
	kHS06Horse				= 3,
	kHS06ExitOutsideBarn	= 4,
	kHS06Device				= 5,
	kHS06WalkArea1			= 6,
	kHS06WalkArea2			= 7,
	kHS06WalkArea3			= 8,
	kHS06WalkArea4			= 9,
	kHS06WalkArea5			= 10
};

enum {
	kHS07Platypus	= 0,
	kHS07ExitHouse	= 1,
	kHS07Dice		= 2,
	kHS07Device		= 3,
	kHS07WalkArea1	= 4,
	kHS07WalkArea2	= 5,
	kHS07WalkArea3	= 6
};

enum {
	kH08SPlatypus	= 0,
	kHS08ExitBackdoor	= 1,
	kHS08ExitCrash	= 2,
	kHS08Man		= 3,
	kHS08Door		= 4,
	kHS08Meat		= 5,
	kHS08Bone		= 6,
	kHS08Toy		= 7,
	kHS08WalkArea1	= 8,
	kHS08Device		= 9,
	kHS08WalkArea2	= 10
};

enum {
	kHS09Platypus		= 0,
	kHS09ExitKitchen	= 1,
	kHS09ExitHouse		= 2,
	kHS09Trash			= 3,
	kHS09Device			= 4,
	kHS09WalkArea1		= 5,
	kHS09WalkArea2		= 6,
	kHS09WalkArea3		= 7
};

enum {
	kAS01LookSpaceship		= 1,
	kAS01LookSpaceshipDone	= 2,
	kAS01LeaveScene			= 3,
	kAS01TakeMud			= 5,
	kAS01LookPigs			= 6,
	kAS01UsePigs			= 7
};

enum {
	kAS02UseTruckNoKeys			= 0,
	kAS02UseGasWithTruck		= 1,
	kAS02UseTruckGas			= 2,
	kAS02UseTruckNoGas			= 3,
	kAS02GrabTruckGrill			= 5,
	kAS02LeaveScene				= 6,
	kAS02TalkChicken			= 7,
	kAS02GrabChicken			= 8,
	kAS02GrabChickenDone		= 9,
	kAS02UseTruckNoKeysDone		= 11,
	kAS02UseGasWithTruckDone	= 12,
	kAS02UseTwigWithChicken		= 16
};

enum {
	kAS03LeaveScene			= 0,
	kAS03FreePlatypus		= 1,
	kAS03HypnotizePlat		= 2,
	kAS03HypnotizeScaredPlat= 3,
	kAS03FreePlatypusDone	= 4,
	kAS03GrabPlatypus		= 5,
	kAS03GrabCreek			= 6,
	kAS03GrabCreekDone		= 7,
	kAS03GrabScaredPlatypus	= 8
};

enum {
	kAS04OpenDoor			= 1,
	kAS04GetKeyFirst		= 2,
	kAS04GetKeyAnother		= 3,
	kAS04LeaveScene			= 4,
	kAS04GetKeyFirstDone	= 6,
	kAS04GetKeyFirst2		= 7,
	kAS04GetKeyAnother2		= 8,
	kAS04GetKeyAnotherDone	= 9,
	kAS04OpenDoorDone		= 10,
	kAS04GrabDog			= 12,
	kAS04GrabAxe			= 13
};

enum {
	kAS05PlatSearchHaystack	= 0,
	kAS05TryPickPadlock		= 1,
	kAS05PickPadlock		= 2,
	kAS05TalkChicken		= 3,
	kAS05GrabChicken		= 4,
	kAS05GrabLadder			= 5,
	kAS05EnterBarn			= 6,
	kAS05UseTwigWithChicken	= 11,
	kAS05LeaveScene			= 12
};

enum {
	kAS06TryToGetGas			= 0,
	kAS06TryToClimbLadder		= 1,
	kAS06TryToClimbLadderDone	= 2,
	kAS06TalkToHorse			= 3,
	kAS06UseTwigOnHorse			= 4,
	kAS06LeaveScene				= 5
};

enum {
	kAS07Wait		= 0,
	kAS07LeaveScene	= 1
};

enum {
	kAS08LeaveScene		= 0,
	kAS08TalkMan		= 1,
	kAS08LookMan		= 2,
	kAS08LookDog		= 3,
	kAS08GrabDog		= 4,
	kAS08TalkDog		= 5,
	kAS08PlatWithMan	= 6,
	kAS08PlatWithDog	= 7
};

enum {
	kAS09LeaveScene			= 0,
	kAS09SearchTrash		= 1,
	kAS09SearchTrashDone	= 2
};

/*****************************************************************************/

class GnapEngine;
class CutScene;

class Scene01: public Scene {
public:
	Scene01(GnapEngine *vm);
	~Scene01() override;

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {}

private:
	int _pigsIdCtr;
	int _smokeIdCtr;
	Graphics::Surface *_spaceshipSurface;
};

class Scene02: public Scene {
public:
	Scene02(GnapEngine *vm);
	~Scene02() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {}

private:
	int _truckGrillCtr;
	int _nextChickenSequenceId;
	int _currChickenSequenceId;
	int _gnapTruckSequenceId;
};

class Scene03: public Scene {
public:
	Scene03(GnapEngine *vm);
	~Scene03() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {}

private:
	bool _platypusHypnotized;
	bool _platypusScared;
	int _nextPlatSequenceId;
	int _nextFrogSequenceId;
	int _currFrogSequenceId;
};

class Scene04: public Scene {
public:
	Scene04(GnapEngine *vm);
	~Scene04() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {}

private:
	bool _triedWindow;
	int _dogIdCtr;
	int _nextDogSequenceId;
	int _currDogSequenceId;
};

class Scene05: public Scene {
public:
	Scene05(GnapEngine *vm);
	~Scene05() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {}

private:
	int _nextChickenSequenceId;
	int _currChickenSequenceId;
};

class Scene06: public Scene {
public:
	Scene06(GnapEngine *vm);
	~Scene06() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {}

private:
	bool _horseTurnedBack;
	int _nextPlatSequenceId;
	int _nextHorseSequenceId;
	int _currHorseSequenceId;
};

class Scene07: public Scene {
public:
	Scene07(GnapEngine *vm);
	~Scene07() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {}
};

class Scene08: public Scene {
public:
	Scene08(GnapEngine *vm);
	~Scene08() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override;

private:
	int _nextDogSequenceId;
	int _currDogSequenceId;
	int _nextManSequenceId;
	int _currManSequenceId;
};

class Scene09: public Scene {
public:
	Scene09(GnapEngine *vm);
	~Scene09() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {}
};

} // End of namespace Gnap

#endif // GNAP_GROUP0_H
