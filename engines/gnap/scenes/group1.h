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

#ifndef GNAP_GROUP1_H
#define GNAP_GROUP1_H

#include "gnap/debugger.h"

namespace Gnap {

enum {
	kHS10Platypus		= 0,
	kHS10ExitBar		= 1,
	kHS10ExitBackdoor	= 2,
	kHS10Cook			= 3,
	kHS10Tongs			= 4,
	kHS10Box			= 5,
	kHS10Oven			= 6,
	kHS10WalkArea1		= 7,
	kHS10Device			= 8,
	kHS10WalkArea2		= 9,
	kHS10WalkArea3		= 10,
	kHS10WalkArea4		= 11
};

enum {
	kHS11Platypus		= 0,
	kHS11ExitKitchen	= 1,
	kHS11ExitToilet		= 2,
	kHS11ExitLeft		= 3,
	kHS11GoggleGuy		= 4,
	kHS11HookGuy		= 5,
	kHS11Billard		= 6,
	kHS11WalkArea1		= 7,
	kHS11Device			= 8,
	kHS11WalkArea2		= 9,
	kHS11WalkArea3		= 10,
	kHS11WalkArea4		= 11,
	kHS11WalkArea5		= 12
};

enum {
	kHS12Platypus	= 0,
	kHS12ExitRight	= 1,
	kHS12ToothGuy	= 2,
	kHS12Barkeeper	= 3,
	kHS12BeardGuy	= 4,
	kHS12Jukebox	= 5,
	kHS12WalkArea1	= 6,
	kHS12Device		= 7,
	kHS12WalkArea2	= 8,
	kHS12WalkArea3	= 9,
	kHS12WalkArea4	= 10
};

enum {
	kHS13Platypus	= 0,
	kHS13ExitBar	= 1,
	kHS13WalkArea1	= 2,
	kHS13BackToilet	= 3,
	kHS13FrontToilet= 4,
	kHS13Urinal		= 5,
	kHS13Scribble	= 6,
	kHS13Sink		= 7,
	kHS13WalkArea2	= 8,
	kHS13Device		= 9,
	kHS13WalkArea3	= 10,
	kHS13WalkArea4	= 11,
	kHS13WalkArea5	= 12,
	kHS13WalkArea6	= 13,
	kHS13WalkArea7	= 14,
	kHS13WalkArea8	= 15,
	kHS13WalkArea9	= 16
};

enum {
	kHS14Platypus	= 0,
	kHS14Exit		= 1,
	kHS14Coin		= 2,
	kHS14Toilet		= 3,
	kHS14Device		= 4
};

enum {
	kHS15Platypus	= 0,
	kHS15Exit		= 1,
	kHS15Button1	= 2,
	kHS15Button2	= 3,
	kHS15Button3	= 4,
	kHS15Button4	= 5,
	kHS15Button5	= 6,
	kHS15Button6	= 7,
	kHS15ButtonA	= 8,
	kHS15ButtonB	= 9,
	kHS15ButtonC	= 10,
	kHS15ButtonD	= 11,
	kHS15ButtonE	= 12,
	kHS15ButtonF	= 13,
	kHS15CoinSlot	= 14,
	kHS15PlayButton	= 15,
	kHS15Device		= 16
};

enum {
	kHS17Platypus		= 0,
	kHS17Phone1			= 1,
	kHS17Phone2			= 2,
	kHS17ExitGrubCity	= 3,
	kHS17Device			= 4,
	kHS17ExitToyStore	= 5,
	kHS17Wrench			= 6,
	kHS17WalkArea1		= 7,
	kHS17WalkArea2		= 8,
	kHS17WalkArea3		= 9
};

enum {
	kHS18Platypus			= 0,
	kHS18GarbageCan			= 1,
	kHS18Device				= 2,
	kHS18ExitToyStore		= 3,
	kHS18ExitPhoneBooth		= 4,
	kHS18ExitGrubCity		= 5,
	kHS18HydrantTopValve	= 6,
	kHS18HydrantRightValve	= 7,
	kHS18CowboyHat			= 8,
	kHS18WalkArea1			= 9,
	kHS18WalkArea2			= 10
};

enum {
	kHS19Platypus			= 0,
	kHS19ExitOutsideToyStore= 1,
	kHS19Device				= 2,
	kHS19Picture			= 3,
	kHS19ShopAssistant		= 4,
	kHS19Toy1				= 5,
	kHS19Toy2				= 6,
	kHS19Toy3				= 7,
	kHS19Phone				= 8,
	kHS19Toy4				= 9,
	kHS19Toy5				= 10,
	kHS19Toy6				= 11,
	kHS19Toy7				= 12,
	kHS19WalkArea1			= 13,
	kHS19WalkArea2			= 14,
	kHS19WalkArea3			= 15
};

enum {
	kAS10LeaveScene		= 0,
	kAS10AnnoyCook		= 1,
	kAS10PlatWithBox	= 4
};

enum {
	kAS11LeaveScene					= 0,
	kAS11ShowMagazineToGoggleGuy	= 3,
	kAS11TalkGoggleGuy				= 4,
	kAS11GrabHookGuy				= 6,
	kAS11ShowItemToHookGuy			= 8,
	kAS11TalkHookGuy				= 9,
	kAS11GrabBillardBall			= 11
};

enum {
	kAS12LeaveScene					= 0,
	kAS12QuarterToToothGuyDone		= 1,
	kAS12TalkToothGuy				= 2,
	kAS12GrabToothGuy				= 4,
	kAS12ShowItemToToothGuy			= 5,
	kAS12QuarterWithHoleToToothGuy	= 6,
	kAS12QuarterToToothGuy			= 7,
	kAS12TalkBeardGuy				= 8,
	kAS12LookBeardGuy				= 9,
	kAS12GrabBeardGuy				= 10,
	kAS12ShowItemToBeardGuy			= 11,
	kAS12TalkBarkeeper				= 12,
	kAS12LookBarkeeper				= 13,
	kAS12ShowItemToBarkeeper		= 15,
	kAS12QuarterWithBarkeeper		= 16,
	kAS12PlatWithBarkeeper			= 17,
	kAS12PlatWithToothGuy			= 18,
	kAS12PlatWithBeardGuy			= 19
};

enum {
	kAS13LeaveScene		= 0,
	kAS13BackToilet		= 1,
	kAS13FrontToilet	= 2,
	kAS13LookScribble	= 6,
	kAS13GrabSink		= 7,
	kAS13GrabSinkDone	= 8,
	kAS13Wait			= 12,
	kAS13GrabUrinal		= 13
};

enum {
	kAS17TryGetWrench			= 0,
	kAS17GetWrench2				= 1,
	kAS17GetWrenchDone			= 2,
	kAS17GetWrench1				= 3,
	kAS17PlatUsePhone			= 4,
	kAS17PutCoinIntoPhone		= 5,
	kAS17GetCoinFromPhone		= 6,
	kAS17GetCoinFromPhoneDone	= 7,
	kAS17PutCoinIntoPhoneDone	= 8,
	kAS17GnapUsePhone			= 9,
	kAS17GetWrenchGnapReady		= 10,
	kAS17GnapHangUpPhone		= 11,
	kAS17PlatPhoningAssistant	= 12,
	kAS17PlatHangUpPhone		= 14,
	kAS17LeaveScene				= 15
};

enum {
	kAS18OpenRightValveNoGarbageCanDone		= 0,
	kAS18OpenRightValveNoGarbageCan			= 1,
	kAS18CloseRightValveNoGarbageCan		= 2,
	kAS18OpenTopValveDone					= 3,
	kAS18OpenTopValve						= 4,
	kAS18CloseTopValve						= 5,
	kAS18GrabGarbageCanFromStreet			= 6,
	kAS18GrabCowboyHat						= 7,
	kAS18GrabGarbageCanFromHydrant			= 8,
	kAS18PutGarbageCanOnRunningHydrant		= 9,
	kAS18PutGarbageCanOnRunningHydrant2		= 10,
	kAS18GrabCowboyHatDone					= 11,
	kAS18StandingOnHydrant					= 12,
	kAS18OpenRightValveWithGarbageCan		= 13,
	kAS18OpenRightValveWithGarbageCanDone	= 14,
	kAS18CloseRightValveWithGarbageCan		= 15,
	kAS18PutGarbageCanOnHydrant				= 16,
	kAS18PutGarbageCanOnHydrantDone			= 17,
	kAS18PlatComesHere						= 18,
	kAS18CloseRightValveWithGarbageCanDone	= 19,
	kAS18LeaveScene							= 20
};

enum {
	kAS19UsePhone				= 0,
	kAS19GrabToy				= 1,
	kAS19GrabPicture			= 2,
	kAS19GrabPictureDone		= 3,
	kAS19TalkShopAssistant		= 4,
	kAS19LeaveScene				= 5
};

/*****************************************************************************/

class GnapEngine;
class CutScene;

class Scene10: public Scene {
public:
	Scene10(GnapEngine *vm);
	~Scene10() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override;

private:
	int _nextCookSequenceId;
	int _currCookSequenceId;
};

class Scene11: public Scene {
public:
	Scene11(GnapEngine *vm);
	~Scene11() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {};

private:
	int _billardBallCtr;
	int _nextHookGuySequenceId;
	int _currHookGuySequenceId;
	int _nextGoggleGuySequenceId;
	int _currGoggleGuySequenceId;
};

class Scene12: public Scene {
public:
	Scene12(GnapEngine *vm);
	~Scene12() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {};

private:
	int _nextBeardGuySequenceId;
	int _currBeardGuySequenceId;
	int _nextToothGuySequenceId;
	int _currToothGuySequenceId;
	int _nextBarkeeperSequenceId;
	int _currBarkeeperSequenceId;
};

class Scene13: public Scene {
public:
	Scene13(GnapEngine *vm);
	~Scene13() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {};

private:
	int _backToiletCtr;

	void showScribble();
};

class Scene14: public Scene {
public:
	Scene14(GnapEngine *vm);
	~Scene14() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {};
};

class Scene15: public Scene {
public:
	Scene15(GnapEngine *vm);
	~Scene15() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {};

private:
	int _nextRecordSequenceId;
	int _currRecordSequenceId;
	int _nextSlotSequenceId;
	int _currSlotSequenceId;
	int _nextUpperButtonSequenceId;
	int _currUpperButtonSequenceId;
	int _nextLowerButtonSequenceId;
	int _currLowerButtonSequenceId;
};

class Scene17: public Scene {
public:
	Scene17(GnapEngine *vm);
	~Scene17() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {};

private:
	bool _canTryGetWrench;
	int _wrenchCtr;
	int _platPhoneCtr;
	int _platTryGetWrenchCtr;
	int _nextPhoneSequenceId;
	int _currPhoneSequenceId;
	int _nextWrenchSequenceId;
	int _currWrenchSequenceId;
	int _nextCarWindowSequenceId;
	int _currCarWindowSequenceId;

	void update();
	void platHangUpPhone();
};

class Scene18: public Scene {
public:
	Scene18(GnapEngine *vm);
	~Scene18() override;

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {};

private:
	Graphics::Surface *_cowboyHatSurface;

	int _platPhoneCtr;
	int _platPhoneIter;
	int _nextPhoneSequenceId;
	int _currPhoneSequenceId;

	void gnapCarryGarbageCanTo(int a5);
	void putDownGarbageCan(int animationIndex);
	void platEndPhoning(bool platFl);
	void closeHydrantValve();
	void waitForGnapAction();
};

class Scene19: public Scene {
public:
	Scene19(GnapEngine *vm);
	~Scene19() override;

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {};

private:
	int _currShopAssistantSequenceId;
	int _nextShopAssistantSequenceId;
	int _toyGrabCtr;
	int _shopAssistantCtr;

	Graphics::Surface *_pictureSurface;
};

} // End of namespace Gnap

#endif // GNAP_GROUP1_H
