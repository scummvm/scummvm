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

#include "dreamweb/dreamweb.h"

namespace DreamGen {


typedef void (DreamBase::*UseCallback)(void);

// Note: The callback pointer has been placed before the
// ID to keep MSVC happy (otherwise, it throws warnings
// that alignment of a member was sensitive to packing)
struct UseListEntry {
	UseCallback callback;
	const char *id;
};

void DreamBase::useRoutine() {

	static const UseListEntry kUseList[] = {
		{ &DreamBase::useMon,                  "NETW" },
		{ &DreamBase::useElevator1,            "ELVA" },
		{ &DreamBase::useElevator2,            "ELVB" },
		{ &DreamBase::useElevator3,            "ELVC" },
		{ &DreamBase::useElevator4,            "ELVE" },
		{ &DreamBase::useElevator5,            "ELVF" },
		{ &DreamBase::useChurchGate,           "CGAT" },
		{ &DreamBase::useStereo,               "REMO" },
		{ &DreamBase::useButtonA,              "BUTA" },
		{ &DreamBase::useWinch,                "CBOX" },
		{ &DreamBase::useLighter,              "LITE" },
		{ &DreamBase::usePlate,                "PLAT" },
		{ &DreamBase::useControl,              "LIFT" },
		{ &DreamBase::useWire,                 "WIRE" },
		{ &DreamBase::useHandle,               "HNDL" },
		{ &DreamBase::useHatch,                "HACH" },
		{ &DreamBase::useElvDoor,              "DOOR" },
		{ &DreamBase::useCashCard,             "CSHR" },
		{ &DreamBase::useGun,                  "GUNA" },
		{ &DreamBase::useCardReader1,          "CRAA" },
		{ &DreamBase::useCardReader2,          "CRBB" },
		{ &DreamBase::useCardReader3,          "CRCC" },
		{ &DreamBase::sitDownInBar,            "SEAT" },
		{ &DreamBase::useMenu,                 "MENU" },
		{ &DreamBase::useCooker,               "COOK" },
		{ &DreamBase::callHotelLift,           "ELCA" },
		{ &DreamBase::callEdensLift,           "EDCA" },
		{ &DreamBase::callEdensDLift,          "DDCA" },
		{ &DreamBase::useAltar,                "ALTR" },
		{ &DreamBase::openHotelDoor,           "LOKA" },
		{ &DreamBase::openHotelDoor2,          "LOKB" },
		{ &DreamBase::openLouis,               "ENTA" },
		{ &DreamBase::openRyan,                "ENTB" },
		{ &DreamBase::openPoolBoss,            "ENTE" },
		{ &DreamBase::openYourNeighbour,       "ENTC" },
		{ &DreamBase::openEden,                "ENTD" },
		{ &DreamBase::openSarters,             "ENTH" },
		{ &DreamBase::wearWatch,               "WWAT" },
		{ &DreamBase::usePoolReader,           "POOL" },
		{ &DreamBase::wearShades,              "WSHD" },
		{ &DreamBase::grafittiDoor,            "GRAF" },
		{ &DreamBase::trapDoor,                "TRAP" },
		{ &DreamBase::edensCDPlayer,           "CDPE" },
		{ &DreamBase::openTVDoor,              "DLOK" },
		{ &DreamBase::useHole,                 "HOLE" },
		{ &DreamBase::useDryer,                "DRYR" },
		{ &DreamBase::useChurchHole,           "HOLY" },
		{ &DreamBase::useWall,                 "WALL" },
		{ &DreamBase::useDiary,                "BOOK" },
		{ &DreamBase::useAxe,                  "AXED" },
		{ &DreamBase::useShield,               "SHLD" },
		{ &DreamBase::useRailing,              "BCNY" },
		{ &DreamBase::useCoveredBox,           "LIDC" },
		{ &DreamBase::useClearBox,             "LIDU" },
		{ &DreamBase::useOpenBox,              "LIDO" },
		{ &DreamBase::usePipe,                 "PIPE" },
		{ &DreamBase::useBalcony,              "BALC" },
		{ &DreamBase::useWindow,               "WIND" },
		{ &DreamBase::viewFolder,              "PAPR" },
		{ &DreamBase::useTrainer,              "UWTA" },
		{ &DreamBase::useTrainer,              "UWTB" },
		{ &DreamBase::enterSymbol,             "STAT" },
		{ &DreamBase::openTomb,                "TLID" },
		{ &DreamBase::useSlab,                 "SLAB" },
		{ &DreamBase::useCart,                 "CART" },
		{ &DreamBase::useFullCart,             "FCAR" },
		{ &DreamBase::slabDoorA,               "SLBA" },
		{ &DreamBase::slabDoorB,               "SLBB" },
		{ &DreamBase::slabDoorC,               "SLBC" },
		{ &DreamBase::slabDoorD,               "SLBD" },
		{ &DreamBase::slabDoorE,               "SLBE" },
		{ &DreamBase::slabDoorF,               "SLBF" },
		{ &DreamBase::usePlinth,               "PLIN" },
		{ &DreamBase::useLadder,               "LADD" },
		{ &DreamBase::useLadderB,              "LADB" },
		{ &DreamBase::chewy,                   "GUMA" },
		{ &DreamBase::wheelSound,              "SQEE" },
		{ &DreamBase::runTap,                  "TAPP" },
		{ &DreamBase::playGuitar,              "GUIT" },
		{ &DreamBase::hotelControl,            "CONT" },
		{ &DreamBase::hotelBell,               "BELL" },
	};

	if (_realLocation >= 50) {
		if (_pointerPower == 0)
			return;
		_pointerPower = 0;
	}

	uint8 dummy;
	void *obj = getAnyAd(&dummy, &dummy);

	for (size_t i = 0; i < sizeof(kUseList)/sizeof(UseListEntry); ++i) {
		const UseListEntry &entry = kUseList[i];
		if (objectMatches(obj, entry.id)) {
			(this->*entry.callback)();
			return;
		}
	}

	delPointer();
	const uint8 *obText = getObTextStart();
	if (findNextColon(&obText) != 0) {
		if (findNextColon(&obText) != 0) {
			if (*obText != 0) {
				useText(obText);
				hangOnP(400);
				putBackObStuff();
				return;
			}
		}
	}

	createPanel();
	showPanel();
	showMan();
	showExit();
	obIcons();
	printMessage(33, 100, 63, 241, true);
	workToScreenM();
	hangOnP(50);
	putBackObStuff();
	_commandType = 255;
}

void DreamBase::useText(const uint8 *string) {
	createPanel();
	showPanel();
	showMan();
	showExit();
	obIcons();
	printDirect(string, 36, 104, 241, true);
	workToScreenM();
}

void DreamBase::showFirstUse() {
	const uint8 *obText = getObTextStart();
	findNextColon(&obText);
	findNextColon(&obText);
	useText(obText);
	hangOnP(400);
}

void DreamBase::showSecondUse() {
	const uint8 *obText = getObTextStart();
	findNextColon(&obText);
	findNextColon(&obText);
	findNextColon(&obText);
	useText(obText);
	hangOnP(400);
}

void DreamBase::viewFolder() {
	_manIsOffScreen = 1;
	getRidOfAll();
	loadFolder();
	_folderPage = 0;
	showFolder();
	workToScreenM();
	_getBack = 0;
	do {
		if (_quitRequested)
			break;
		delPointer();
		readMouse();
		showPointer();
		vSync();
		dumpPointer();
		dumpTextLine();
		checkFolderCoords();
	} while (_getBack == 0);
	_manIsOffScreen = 0;
	getRidOfTemp();
	getRidOfTemp2();
	getRidOfTemp3();
	getRidOfTempCharset();
	restoreAll();
	redrawMainScrn();
	workToScreenM();
}

void DreamBase::edensCDPlayer() {
	showFirstUse();
	data.word(kWatchingtime) = 18 * 2;
	data.word(kReeltowatch) = 25;
	data.word(kEndwatchreel) = 42;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	_getBack = 1;
}

void DreamBase::hotelBell() {
	playChannel1(12);
	showFirstUse();
	putBackObStuff();
}

void DreamBase::playGuitar() {
	playChannel1(14);
	showFirstUse();
	putBackObStuff();
}

void DreamBase::useElevator1() {
	showFirstUse();
	selectLocation();
	_getBack = 1;
}

void DreamBase::useElevator2() {
	showFirstUse();

	if (data.byte(kLocation) == 23)	// In pool hall
		_newLocation = 31;
	else
		_newLocation = 23;

	data.byte(kCounttoclose) = 20;
	data.byte(kCounttoopen) = 0;
	data.word(kWatchingtime) = 80;
	_getBack = 1;
}

void DreamBase::useElevator3() {
	showFirstUse();
	data.byte(kCounttoclose) = 20;
	_newLocation = 34;
	data.word(kReeltowatch) = 46;
	data.word(kEndwatchreel) = 63;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.word(kWatchingtime) = 80;
	_getBack = 1;
}

void DreamBase::useElevator4() {
	showFirstUse();
	data.word(kReeltowatch) = 0;
	data.word(kEndwatchreel) = 11;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kCounttoclose) = 20;
	data.word(kWatchingtime) = 80;
	_getBack = 1;
	_newLocation = 24;
}

void DreamBase::useElevator5() {
	placeSetObject(4);
	removeSetObject(0);
	_newLocation = 20;
	data.word(kWatchingtime) = 80;
	data.byte(kLiftflag) = 1;
	data.byte(kCounttoclose) = 8;
	_getBack = 1;
}

void DreamBase::useHatch() {
	showFirstUse();
	_newLocation = 40;
	_getBack = 1;
}

void DreamBase::wheelSound() {
	playChannel1(17);
	showFirstUse();
	putBackObStuff();
}

void DreamBase::callHotelLift() {
	playChannel1(12);
	showFirstUse();
	data.byte(kCounttoopen) = 8;
	_getBack = 1;
	_destination = 5;
	_finalDest = 5;
	autoSetWalk();
	turnPathOn(4);
}

void DreamBase::useShield() {
	if (_realLocation != 20 || data.byte(kCombatcount) == 0) {
		// Not in Sart room
		showFirstUse();
		putBackObStuff();
	} else {
		data.byte(kLastweapon) = 3;
		showSecondUse();
		_getBack = 1;
		data.byte(kProgresspoints)++;
		removeObFromInv();
	}
}

void DreamBase::useCoveredBox() {
	data.byte(kProgresspoints)++;
	showFirstUse();
	data.word(kWatchingtime) = 50;
	data.word(kReeltowatch) = 41;
	data.word(kEndwatchreel) = 66;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	_getBack = 1;
}

void DreamBase::useRailing() {
	showFirstUse();
	data.word(kWatchingtime) = 80;
	data.word(kReeltowatch) = 0;
	data.word(kEndwatchreel) = 30;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	_getBack = 1;
	data.byte(kMandead) = 4;
}

void DreamBase::wearWatch() {
	if (data.byte(kWatchon) == 1) {
		// Already wearing watch
		showSecondUse();
		putBackObStuff();
	} else {
		showFirstUse();
		data.byte(kWatchon) = 1;
		_getBack = 1;
		uint8 dummy;
		makeWorn((DynObject *)getAnyAd(&dummy, &dummy));
	}
}

void DreamBase::wearShades() {
	if (data.byte(kShadeson) == 1) {
		// Already wearing shades
		showSecondUse();
		putBackObStuff();
	} else {
		data.byte(kShadeson) = 1;
		showFirstUse();
		_getBack = 1;
		uint8 dummy;
		makeWorn((DynObject *)getAnyAd(&dummy, &dummy));
	}
}

void DreamBase::useChurchHole() {
	showFirstUse();
	_getBack = 1;
	data.word(kWatchingtime) = 28;
	data.word(kReeltowatch) = 13;
	data.word(kEndwatchreel) = 26;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
}

void DreamBase::sitDownInBar() {
	if (data.byte(kWatchmode) != 0xFF) {
		// Sat down
		showSecondUse();
		putBackObStuff();
	} else {
		showFirstUse();
		data.word(kWatchingtime) = 50;
		data.word(kReeltowatch) = 55;
		data.word(kEndwatchreel) = 71;
		data.word(kReeltohold) = 73;
		data.word(kEndofholdreel) = 83;
		data.byte(kWatchspeed) = 1;
		data.byte(kSpeedcount) = 1;
		_getBack = 1;
	}
}

void DreamBase::useDryer() {
	playChannel1(12);
	showFirstUse();
	_getBack = 1;
}

void DreamBase::useBalcony() {
	showFirstUse();
	turnPathOn(6);
	turnPathOff(0);
	turnPathOff(1);
	turnPathOff(2);
	turnPathOff(3);
	turnPathOff(4);
	turnPathOff(5);
	data.byte(kProgresspoints)++;
	_mansPath = 6;
	_destination = 6;
	_finalDest = 6;
	findXYFromPath();
	switchRyanOff();
	_resetManXY = 1;
	data.word(kWatchingtime) = 30 * 2;
	data.word(kReeltowatch) = 183;
	data.word(kEndwatchreel) = 212;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	_getBack = 1;
}

void DreamBase::useWindow() {
	if (_mansPath != 6) {
		// Not on balcony
		showSecondUse();
		putBackObStuff();
	} else {
		data.byte(kProgresspoints)++;
		showFirstUse();
		_newLocation = 29;
		_getBack = 1;
	}
}

void DreamBase::trapDoor() {
	data.byte(kProgresspoints)++;
	showFirstUse();
	switchRyanOff();
	data.word(kWatchingtime) = 20 * 2;
	data.word(kReeltowatch) = 181;
	data.word(kEndwatchreel) = 197;
	_newLocation = 26;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	_getBack = 1;
}

void DreamBase::callEdensLift() {
	showFirstUse();
	data.byte(kCounttoopen) = 8;
	_getBack = 1;
	turnPathOn(2);
}

void DreamBase::callEdensDLift() {
	if (data.byte(kLiftflag) == 1) {
		// Eden's D here
		showSecondUse();
		putBackObStuff();
	} else {
		showFirstUse();
		data.byte(kCounttoopen) = 8;
		_getBack = 1;
		turnPathOn(2);
	}
}

void DreamBase::openYourNeighbour() {
	enterCode(255, 255, 255, 255);
	_getBack = 1;
}

void DreamBase::openRyan() {
	enterCode(5, 1, 0, 6);
	_getBack = 1;
}

void DreamBase::openPoolBoss() {
	enterCode(5, 2, 2, 2);
	_getBack = 1;
}

void DreamBase::openEden() {
	enterCode(2, 8, 6, 5);
	_getBack = 1;
}

void DreamBase::openSarters() {
	enterCode(7, 8, 3, 3);
	_getBack = 1;
}

void DreamBase::openLouis() {
	enterCode(5, 2, 3, 8);
	_getBack = 1;
}


void DreamBase::useWall() {
	showFirstUse();

	if (_mansPath != 3) {
		data.word(kWatchingtime) = 30*2;
		data.word(kReeltowatch) = 2;
		data.word(kEndwatchreel) = 31;
		data.byte(kWatchspeed) = 1;
		data.byte(kSpeedcount) = 1;
		_getBack = 1;
		turnPathOn(3);
		turnPathOn(4);
		turnPathOff(0);
		turnPathOff(1);
		turnPathOff(2);
		turnPathOff(5);
		_mansPath = 3;
		_finalDest = 3;
		findXYFromPath();
		_resetManXY = 1;
		switchRyanOff();
	} else {
		// Go back over
		data.word(kWatchingtime) = 30 * 2;
		data.word(kReeltowatch) = 34;
		data.word(kEndwatchreel) = 60;
		data.byte(kWatchspeed) = 1;
		data.byte(kSpeedcount) = 1;
		_getBack = 1;
		turnPathOff(3);
		turnPathOff(4);
		turnPathOn(0);
		turnPathOn(1);
		turnPathOn(2);
		turnPathOn(5);
		_mansPath = 5;
		_finalDest = 5;
		findXYFromPath();
		_resetManXY = 1;
		switchRyanOff();
	}
}

void DreamBase::useLadder() {
	showFirstUse();
	_mapX = _mapX - 11;
	findRoomInLoc();
	_facing = 6;
	_turnToFace = 6;
	_mansPath = 0;
	_destination = 0;
	_finalDest = 0;
	findXYFromPath();
	_resetManXY = 1;
	_getBack = 1;
}

void DreamBase::useLadderB() {
	showFirstUse();
	_mapX = _mapX + 11;
	findRoomInLoc();
	_facing = 2;
	_turnToFace = 2;
	_mansPath = 1;
	_destination = 1;
	_finalDest = 1;
	findXYFromPath();
	_resetManXY = 1;
	_getBack = 1;
}

void DreamBase::slabDoorA() {
	showFirstUse();
	_getBack = 1;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.word(kReeltowatch) = 13;
	if (data.byte(kDreamnumber) != 3) {
		// Wrong
		data.word(kWatchingtime) = 40;
		data.word(kEndwatchreel) = 34;
		data.byte(kWatchspeed) = 1;
		data.byte(kSpeedcount) = 1;
	} else {
		data.byte(kProgresspoints)++;
		data.word(kWatchingtime) = 60;
		data.word(kEndwatchreel) = 42;
		_newLocation = 47;
	}
}

void DreamBase::slabDoorB() {
	if (data.byte(kDreamnumber) != 1) {
		// Wrong
		showFirstUse();
		_getBack = 1;
		data.byte(kWatchspeed) = 1;
		data.byte(kSpeedcount) = 1;
		data.word(kReeltowatch) = 44;
		data.word(kWatchingtime) = 40;
		data.word(kEndwatchreel) = 63;
		data.byte(kWatchspeed) = 1;
		data.byte(kSpeedcount) = 1;
	} else {
		if (!isRyanHolding("SHLD")) {
			// No crystal
			showPuzText(44, 200);
			putBackObStuff();
		} else {
			// Got crystal
			showFirstUse();
			data.byte(kProgresspoints)++;
			_getBack = 1;
			data.byte(kWatchspeed) = 1;
			data.byte(kSpeedcount) = 1;
			data.word(kReeltowatch) = 44;
			data.word(kWatchingtime) = 60;
			data.word(kEndwatchreel) = 71;
			_newLocation = 47;
		}
	}
}

void DreamBase::slabDoorC() {
	showFirstUse();
	_getBack = 1;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.word(kReeltowatch) = 108;
	if (data.byte(kDreamnumber) != 4) {
		// Wrong
		data.word(kWatchingtime) = 40;
		data.word(kEndwatchreel) = 127;
		data.byte(kWatchspeed) = 1;
		data.byte(kSpeedcount) = 1;
	} else {
		data.byte(kProgresspoints)++;
		data.word(kWatchingtime) = 60;
		data.word(kEndwatchreel) = 135;
		_newLocation = 47;
	}
}

void DreamBase::slabDoorD() {
	showFirstUse();
	_getBack = 1;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.word(kReeltowatch) = 75;
	if (data.byte(kDreamnumber) != 0) {
		// Wrong
		data.word(kWatchingtime) = 40;
		data.word(kEndwatchreel) = 94;
		data.byte(kWatchspeed) = 1;
		data.byte(kSpeedcount) = 1;
	} else {
		data.byte(kProgresspoints)++;
		data.word(kWatchingtime) = 60;
		data.word(kEndwatchreel) = 102;
		_newLocation = 47;
	}
}

void DreamBase::slabDoorE() {
	showFirstUse();
	_getBack = 1;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.word(kReeltowatch) = 141;
	if (data.byte(kDreamnumber) != 5) {
		// Wrong
		data.word(kWatchingtime) = 40;
		data.word(kEndwatchreel) = 160;
		data.byte(kWatchspeed) = 1;
		data.byte(kSpeedcount) = 1;
	} else {
		data.byte(kProgresspoints)++;
		data.word(kWatchingtime) = 60;
		data.word(kEndwatchreel) = 168;
		_newLocation = 47;
	}
}

void DreamBase::slabDoorF() {
	showFirstUse();
	_getBack = 1;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.word(kReeltowatch) = 171;
	if (data.byte(kDreamnumber) != 2) {
		// Wrong
		data.word(kWatchingtime) = 40;
		data.word(kEndwatchreel) = 189;
		data.byte(kWatchspeed) = 1;
		data.byte(kSpeedcount) = 1;
	} else {
		data.byte(kProgresspoints)++;
		data.word(kWatchingtime) = 60;
		data.word(kEndwatchreel) = 197;
		_newLocation = 47;
	}
}

bool DreamBase::defaultUseHandler(const char *id) {
	if (_withObject == 255) {
		withWhat();
		return true;	// event handled
	}

	if (!compare(_withObject, _withType, id)) {
		// Wrong item
		showPuzText(14, 300);
		putBackObStuff();
		return true;	// event handled
	}

	return false;	// continue with the original event
}

void DreamBase::useChurchGate() {
	if (defaultUseHandler("CUTT"))
		return;

	// Cut gate
	showFirstUse();
	data.word(kWatchingtime) = 64 * 2;
	data.word(kReeltowatch) = 4;
	data.word(kEndwatchreel) = 70;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	_getBack = 1;
	data.byte(kProgresspoints)++;
	turnPathOn(3);
	if (data.byte(kAidedead) != 0)
		turnPathOn(2);	// Open church
}

void DreamBase::useGun() {

	if (_objectType != kExObjectType) {
		// gun is not taken
		showSecondUse();
		putBackObStuff();

	} else if (_realLocation == 22) {
		// in pool room
		showPuzText(34, 300);
		data.byte(kLastweapon) = 1;
		data.byte(kCombatcount) = 39;
		_getBack = 1;
		data.byte(kProgresspoints)++;

	} else if (_realLocation == 25) {
		// helicopter
		showPuzText(34, 300);
		data.byte(kLastweapon) = 1;
		data.byte(kCombatcount) = 19;
		_getBack = 1;
		data.byte(kDreamnumber) = 2;
		data.byte(kRoomafterdream) = 38;
		data.byte(kSartaindead) = 1;
		data.byte(kProgresspoints)++;

	} else if (_realLocation == 27) {
		// in rock room
		showPuzText(46, 300);
		_pointerMode = 2;
		data.byte(kRockstardead) = 1;
		data.byte(kLastweapon) = 1;
		data.byte(kNewsitem) = 1;
		_getBack = 1;
		data.byte(kRoomafterdream) = 32;
		data.byte(kDreamnumber) = 0;
		data.byte(kProgresspoints)++;

	} else if (_realLocation == 8 && _mapX == 22 && _mapY == 40
	    && !isSetObOnMap(92) && _mansPath != 9) {
		// by studio
		_destination = 9;
		_finalDest = 9;
		autoSetWalk();
		data.byte(kLastweapon) = 1;
		_getBack = 1;
		data.byte(kProgresspoints)++;

	} else if (_realLocation == 6 && _mapX == 11 && _mapY == 20
	    && isSetObOnMap(5)) {
		// sarters
		_destination = 1;
		_finalDest = 1;
		autoSetWalk();
		removeSetObject(5);
		placeSetObject(6);
		turnAnyPathOn(1, _roomNum - 1);
		data.byte(kLiftflag) = 1;
		data.word(kWatchingtime) = 40*2;
		data.word(kReeltowatch) = 4;
		data.word(kEndwatchreel) = 43;
		data.byte(kWatchspeed) = 1;
		data.byte(kSpeedcount) = 1;
		_getBack = 1;
		data.byte(kProgresspoints)++;

	} else if (_realLocation == 29) {
		// aide
		_getBack = 1;
		resetLocation(13);
		setLocation(12);
		_destPos = 12;
		_destination = 2;
		_finalDest = 2;
		autoSetWalk();
		data.word(kWatchingtime) = 164*2;
		data.word(kReeltowatch) = 3;
		data.word(kEndwatchreel) = 164;
		data.byte(kWatchspeed) = 1;
		data.byte(kSpeedcount) = 1;
		data.byte(kAidedead) = 1;
		data.byte(kDreamnumber) = 3;
		data.byte(kRoomafterdream) = 33;
		data.byte(kProgresspoints)++;

	} else if (_realLocation == 23 && _mapX == 0 && _mapY == 50) {
		// with boss
		if (_mansPath != 5) {
			_destination = 5;
			_finalDest = 5;
			autoSetWalk();
		}
		data.byte(kLastweapon) = 1;
		_getBack = 1;

	} else if (_realLocation == 8 && _mapX == 11 && _mapY == 10) {
		// tv soldier
		if (_mansPath != 2) {
			_destination = 2;
			_finalDest = 2;
			autoSetWalk();
		}
		data.byte(kLastweapon) = 1;
		_getBack = 1;

	} else {
		showFirstUse();
		putBackObStuff();
	}
}

void DreamBase::useFullCart() {
	data.byte(kProgresspoints)++;
	turnAnyPathOn(2, _roomNum + 6);
	_mansPath = 4;
	_facing = 4;
	_turnToFace = 4;
	_finalDest = 4;
	findXYFromPath();
	_resetManXY = 1;
	showFirstUse();
	data.word(kWatchingtime) = 72 * 2;
	data.word(kReeltowatch) = 58;
	data.word(kEndwatchreel) = 142;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	_getBack = 1;
}

void DreamBase::useClearBox() {
	if (defaultUseHandler("RAIL"))
		return;

	// Open box
	data.byte(kProgresspoints)++;
	showFirstUse();
	data.word(kWatchingtime) = 80;
	data.word(kReeltowatch) = 67;
	data.word(kEndwatchreel) = 105;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	_getBack = 1;
}

void DreamBase::openTVDoor() {
	if (defaultUseHandler("ULOK"))
		return;

	// Key on TV
	showFirstUse();
	data.byte(kLockstatus) = 0;
	_getBack = 1;
}

void DreamBase::usePlate() {
	if (_withObject == 255) {
		withWhat();
		return;
	}

	if (compare(_withObject, _withType, "SCRW")) {
		// Unscrew plate
		playChannel1(20);
		showFirstUse();
		placeSetObject(28);
		placeSetObject(24);
		removeSetObject(25);
		placeFreeObject(0);
		data.byte(kProgresspoints)++;
		_getBack = 1;
	} else if (compare(_withObject, _withType, "KNFE")) {
		// Tried knife
		showPuzText(54, 300);
		putBackObStuff();
	} else {
		// Wrong item
		showPuzText(14, 300);
		putBackObStuff();
	}
}

void DreamBase::usePlinth() {
	if (_withObject == 255) {
		withWhat();
		return;
	}

	if (!compare(_withObject, _withType, "DKEY")) {
		// Wrong key
		showFirstUse();
		putBackObStuff();
	} else {
		data.byte(kProgresspoints)++;
		showSecondUse();
		data.word(kWatchingtime) = 220;
		data.word(kReeltowatch) = 0;
		data.word(kEndwatchreel) = 104;
		data.byte(kWatchspeed) = 1;
		data.byte(kSpeedcount) = 1;
		_getBack = 1;
		_newLocation = data.byte(kRoomafterdream);
	}
}

void DreamBase::useElvDoor() {
	if (defaultUseHandler("AXED"))
		return;

	// Axe on door
	showPuzText(15, 300);
	data.byte(kProgresspoints)++;
	data.word(kWatchingtime) = 46 * 2;
	data.word(kReeltowatch) = 31;
	data.word(kEndwatchreel) = 77;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	_getBack = 1;
}

void DreamBase::useObject() {
	_withObject = 255;

	if (_commandType != 229) {
		_commandType = 229;
		commandWithOb(51, _objectType, _command);
	}

	if (_mouseButton == _oldButton)
		return;	// nouse

	if (_mouseButton & 1)
		useRoutine();
}

void DreamBase::useWinch() {
	uint16 contentIndex = checkInside(40, 1);
	if (contentIndex == kNumexobjects || !compare(contentIndex, kExObjectType, "FUSE")) {
		// No winch
		showFirstUse();
		putBackObStuff();
		return;
	}

	data.word(kWatchingtime) = 217 * 2;
	data.word(kReeltowatch) = 0;
	data.word(kEndwatchreel) = 217;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	_destPos = 1;
	_newLocation = 45;
	data.byte(kDreamnumber) = 1;
	data.byte(kRoomafterdream) = 44;
	data.byte(kGeneraldead) = 1;
	data.byte(kNewsitem) = 2;
	_getBack = 1;
	data.byte(kProgresspoints)++;
}

void DreamBase::useCart() {
	if (defaultUseHandler("ROCK"))
		return;

	DynObject *exObject = getExAd(_withObject);
	exObject->mapad[0] = 0;
	removeSetObject(_command);
	placeSetObject(_command + 1);
	data.byte(kProgresspoints)++;
	playChannel1(17);
	showFirstUse();
	_getBack = 1;
}

void DreamBase::useTrainer() {
	uint8 dummy;
	DynObject *object = (DynObject *)getAnyAd(&dummy, &dummy);
	if (object->mapad[0] != 4) {
		notHeldError();
	} else {
		data.byte(kProgresspoints)++;
		makeWorn(object);
		showSecondUse();
		putBackObStuff();
	}
}

void DreamBase::chewy() {
	// Chewing a gum
	showFirstUse();
	uint8 dummy;
	DynObject *object = (DynObject *)getAnyAd(&dummy, &dummy);
	object->mapad[0] = 255;
	_getBack = 1;
}

void DreamBase::useHole() {
	if (defaultUseHandler("HNDA"))
		return;

	showFirstUse();
	removeSetObject(86);
	DynObject *exObject = getExAd(_withObject);
	exObject->mapad[0] = 255;
	data.byte(kCanmovealtar) = 1;
	_getBack = 1;
}

void DreamBase::openHotelDoor() {
	if (defaultUseHandler("KEYA"))
		return;

	playChannel1(16);
	showFirstUse();
	data.byte(kLockstatus) = 0;
	_getBack = 1;
}

void DreamBase::openHotelDoor2() {
	if (defaultUseHandler("KEYA"))
		return;

	playChannel1(16);
	showFirstUse();
	putBackObStuff();
}

void DreamBase::grafittiDoor() {
	if (defaultUseHandler("APEN"))
		return;

	showFirstUse();
	putBackObStuff();
}

void DreamBase::usePoolReader() {
	if (defaultUseHandler("MEMB"))
		return;

	if (data.byte(kTalkedtoattendant) != 1) {
		// Can't open pool
		showSecondUse();
		putBackObStuff();
	} else {
		playChannel1(17);
		showFirstUse();
		data.byte(kCounttoopen) = 6;
		_getBack = 1;
	}
}

void DreamBase::useCardReader1() {
	if (defaultUseHandler("CSHR"))
		return;

	if (data.byte(kTalkedtosparky) == 0) {
		// Not yet
		showFirstUse();
		putBackObStuff();
	} else if (data.word(kCard1money) != 0) {
		// No cash
		showPuzText(17, 300);
		putBackObStuff();
	} else {
		// Get cash
		playChannel1(16);
		showPuzText(18, 300);
		data.byte(kProgresspoints)++;
		data.word(kCard1money) = 12432;
		_getBack = 1;
	}
}

void DreamBase::useCardReader2() {
	if (defaultUseHandler("CSHR"))
		return;

	if (data.byte(kTalkedtoboss) == 0) {
		// Haven't talked to boss
		showFirstUse();
		putBackObStuff();
	} else if (data.byte(kCard1money) == 0) {
		// No cash
		showPuzText(20, 300);
		putBackObStuff();
	} else if (data.byte(kGunpassflag) == 2) {
		// Already got new
		showPuzText(22, 300);
		putBackObStuff();
	} else {
		playChannel1(18);
		showPuzText(19, 300);
		placeSetObject(94);
		data.byte(kGunpassflag) = 1;
		data.word(kCard1money) -= 2000;
		data.byte(kProgresspoints)++;
		_getBack = 1;
	}
}

void DreamBase::useCardReader3() {
	if (defaultUseHandler("CSHR"))
		return;

	if (data.byte(kTalkedtorecep) == 0) {
		// Haven't talked to receptionist
		showFirstUse();
		putBackObStuff();
	} else if (data.byte(kCardpassflag) != 0) {
		// Already used it
		showPuzText(26, 300);
		putBackObStuff();
	} else {
		playChannel1(16);
		showPuzText(25, 300);
		data.byte(kProgresspoints)++;
		data.word(kCard1money) -= 8300;
		data.byte(kCardpassflag) = 1;
		_getBack = 1;
	}
}

void DreamBase::useLighter() {
	if (_withObject == 255) {
		withWhat();
		return;
	}

	if (!compare(_withObject, _withType, "SMKE")) {
		showFirstUse();
		putBackObStuff();
	} else {
		showPuzText(9, 300);
		DynObject *withObj = getExAd(_withObject);
		withObj->mapad[0] = 255;
		_getBack = 1;
	}
}

void DreamBase::useWire() {
	if (_withObject == 255) {
		withWhat();
		return;
	}

	if (compare(_withObject, _withType, "KNFE")) {
		removeSetObject(51);
		placeSetObject(52);
		showPuzText(11, 300);
		data.byte(kProgresspoints)++;
		_getBack = 1;
		return;
	}

	if (compare(_withObject, _withType, "AXED")) {
		showPuzText(16, 300);
		putBackObStuff();
		return;
	}

	showPuzText(14, 300);
	putBackObStuff();
}

void DreamBase::openTomb() {
	data.byte(kProgresspoints)++;
	showFirstUse();
	data.word(kWatchingtime) = 35 * 2;
	data.word(kReeltowatch) = 1;
	data.word(kEndwatchreel) = 33;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	_getBack = 1;
}

void DreamBase::hotelControl() {
	if (_realLocation != 21 || _mapX != 33)
		showSecondUse();	// Not right control
	else
		showFirstUse();

	putBackObStuff();
}

void DreamBase::useCooker() {
	if (checkInside(_command, _objectType) == kNumexobjects)
		showFirstUse();
	else
		showSecondUse();	// Food inside
		
	putBackObStuff();
}

void DreamBase::placeFreeObject(uint8 index) {
	findOrMake(index, 0, 1);
	getFreeAd(index)->mapad[0] = 0;
}

void DreamBase::removeFreeObject(uint8 index) {
	getFreeAd(index)->mapad[0] = 0xFF;
}

void DreamBase::useDiary() {
	getRidOfReels();
	loadIntoTemp("DREAMWEB.G14");
	loadTempText("DREAMWEB.T51");
	loadTempCharset("DREAMWEB.C02");
	createPanel();
	showIcon();
	showDiary();
	underTextLine();
	showDiaryPage();
	readMouse();
	showPointer();
	workToScreen();
	delPointer();
	_getBack = 0;

	RectWithCallback<DreamBase> diaryList[] = {
		{ kDiaryx+94,kDiaryx+110,kDiaryy+97,kDiaryy+113,&DreamBase::diaryKeyN },
		{ kDiaryx+151,kDiaryx+167,kDiaryy+71,kDiaryy+87,&DreamBase::diaryKeyP },
		{ kDiaryx+176,kDiaryx+192,kDiaryy+108,kDiaryy+124,&DreamBase::quitKey },
		{ 0,320,0,200,&DreamBase::blank },
		{ 0xFFFF,0,0,0,0 }
	};
	
	do {
		delPointer();
		readMouse();
		showDiaryKeys();
		showPointer();
		vSync();
		dumpPointer();
		dumpDiaryKeys();
		dumpTextLine();
		checkCoords(diaryList);
	} while (!_getBack && !_quitRequested);


	getRidOfTemp();
	getRidOfTempText();
	getRidOfTempCharset();
	restoreReels();
	_manIsOffScreen = 0;
	redrawMainScrn();
	workToScreenM();
}

void DreamBase::useControl() {
	if (_withObject == 255) {
		withWhat();
		return;
	}

	if (compare(_withObject, _withType, "KEYA")) {	// Right key
		playChannel1(16);
		if (data.byte(kLocation) == 21) {	// Going down
			showPuzText(3, 300);
			_newLocation = 30;
		} else {
			showPuzText(0, 300);
			_newLocation = 21;
		}

		data.byte(kCounttoclose) = 8;
		data.byte(kCounttoopen) = 0;
		data.word(kWatchingtime) = 80;
		_getBack = 1;
		return;
	}

	if (_realLocation == 21) {
		if (compare(_withObject, _withType, "KNFE")) {
			// Jimmy controls
			placeSetObject(50);
			placeSetObject(51);
			placeSetObject(26);
			placeSetObject(30);
			removeSetObject(16);
			removeSetObject(17);
			playChannel1(14);
			showPuzText(10, 300);
			data.byte(kProgresspoints)++;
			_getBack = 1;
		} else if (compare(_withObject, _withType, "AXED")) {
			// Axe on controls
			showPuzText(16, 300);
			data.byte(kProgresspoints)++;
			putBackObStuff();
		} else {
			// Balls
			showFirstUse();
			putBackObStuff();
		}
	} else {
		// Balls
		showFirstUse();
		putBackObStuff();
	}
}

void DreamBase::useSlab() {
	if (_withObject == 255) {
		withWhat();
		return;
	}

	if (!compare(_withObject, _withType, "JEWL")) {
		showPuzText(14, 300);
		putBackObStuff();
		return;
	}

	DynObject *exObject = getExAd(_withObject);
	exObject->mapad[0] = 0;

	removeSetObject(_command);
	placeSetObject(_command + 1);
	if (_command + 1 == 54) {
		// Last slab
		turnPathOn(0);
		data.word(kWatchingtime) = 22;
		data.word(kReeltowatch) = 35;
		data.word(kEndwatchreel) = 48;
		data.byte(kWatchspeed) = 1;
		data.byte(kSpeedcount) = 1;
	}

	data.byte(kProgresspoints)++;
	showFirstUse();
	_getBack = 1;
}

void DreamBase::usePipe() {
	if (_withObject == 255) {
		withWhat();
		return;
	}

	if (compare(_withObject, _withType, "CUPE")) {
		// Fill cup
		showPuzText(36, 300);
		putBackObStuff();
		DynObject *exObject = getExAd(_withObject);
		exObject->id[3] = 'F'-'A';	// CUPE (empty cup) -> CUPF (full cup)
		return;
	} else if (compare(_withObject, _withType, "CUPF")) {
		// Already full
		showPuzText(35, 300);
		putBackObStuff();
	} else {
		showPuzText(14, 300);
		putBackObStuff();
	}
}

void DreamBase::useOpenBox() {
	if (_withObject == 255) {
		withWhat();
		return;
	}

	if (compare(_withObject, _withType, "CUPF")) {
		// Destroy open box
		data.byte(kProgresspoints)++;
		showPuzText(37, 300);
		DynObject *exObject = getExAd(_withObject);
		exObject->id[3] = 'E'-'A';	// CUPF (full cup) -> CUPE (empty cup)
		data.word(kWatchingtime) = 140;
		data.word(kReeltowatch) = 105;
		data.word(kEndwatchreel) = 181;
		data.byte(kWatchspeed) = 1;
		data.byte(kSpeedcount) = 1;
		turnPathOn(4);
		_getBack = 1;
		return;
	}

	if (compare(_withObject, _withType, "CUPE")) {
		// Open box wrong
		showPuzText(38, 300);
		putBackObStuff();
		return;
	}

	showFirstUse();
}

void DreamBase::runTap() {
	if (_withObject == 255) {
		withWhat();
		return;
	}

	if (compare(_withObject, _withType, "CUPE")) {
		// Fill cup from tap
		DynObject *exObject = getExAd(_withObject);
		exObject->id[3] = 'F'-'A';	// CUPE (empty cup) -> CUPF (full cup)
		playChannel1(8);
		showPuzText(57, 300);
		putBackObStuff();
		return;
	}

	if (compare(_withObject, _withType, "CUPF")) {
		// Cup from tap full
		showPuzText(58, 300);
		putBackObStuff();
		return;
	}

	showPuzText(56, 300);
	putBackObStuff();
}

void DreamBase::useAxe() {
	if (_realLocation != 22) {
		// Not in pool
		showFirstUse();
		return;
	}

	if (_mapY == 10) {
		// Axe on door
		showPuzText(15, 300);
		data.byte(kProgresspoints)++;
		data.word(kWatchingtime) = 46*2;
		data.word(kReeltowatch) = 31;
		data.word(kEndwatchreel) = 77;
		data.byte(kWatchspeed) = 1;
		data.byte(kSpeedcount) = 1;
		_getBack = 1;
		return;
	}

	showSecondUse();
	data.byte(kProgresspoints)++;
	data.byte(kLastweapon) = 2;
	_getBack = 1;
	removeObFromInv();
}

void DreamBase::useHandle() {
	SetObject *object = getSetAd(findSetObject("CUTW"));
	if (object->mapad[0] == 255) {
		// Wire not cut
		showPuzText(12, 300);
	} else {
		// Wire has been cut
		showPuzText(13, 300);
		_newLocation = 22;
	}

	_getBack = 1;
}

void DreamBase::useAltar() {
	if (findExObject("CNDA") == 114 || findExObject("CNDB") == 114) {
		// Things on altar
		showFirstUse();
		_getBack = 1;
		return;
	}

	if (data.byte(kCanmovealtar) == 1) {
		// Move altar
		data.byte(kProgresspoints)++;
		showSecondUse();
		data.word(kWatchingtime) = 160;
		data.word(kReeltowatch) = 81;
		data.word(kEndwatchreel) = 174;
		data.byte(kWatchspeed) = 1;
		data.byte(kSpeedcount) = 1;
		DreamBase::setupTimedUse(47, 32, 98, 52, 76);
		_getBack = 1;
	} else {
		showPuzText(23, 300);
		_getBack = 1;
	}
}

void DreamBase::withWhat() {
	uint8 commandLine[64] = "OBJECT NAME ONE                         ";

	createPanel();
	showPanel();
	showMan();
	showExit();

	copyName(_objectType, _command, commandLine);
	printMessage2(100, 21, 63, 200, false, 2);
	uint16 x = _lastXPos + 5;
	printDirect(commandLine, x, 21, 220, false);
	printMessage2(_lastXPos + 5, 21, 63, 200, false, 3);

	fillRyan();
	_commandType = 255;
	readMouse();
	showPointer();
	workToScreen();
	delPointer();
	_invOpen = 2;
}

void DreamBase::notHeldError() {
	createPanel();
	showPanel();
	showMan();
	showExit();
	obIcons();
	printMessage2(64, 100, 63, 200 + 1, true, 1);
	workToScreenM();
	hangOnP(50);
	putBackObStuff();
}

void DreamBase::useCashCard() {
	getRidOfReels();
	loadKeypad();
	createPanel();
	showPanel();
	showExit();
	showMan();
	uint16 y = (!_foreignRelease) ? 120 : 120 - 3;
	showFrame(_tempGraphics, 114, y, 39, 0);
	const uint8 *obText = getObTextStart();
	findNextColon(&obText);
	findNextColon(&obText);
	y = 98;
	printDirect(&obText, 36, &y, 36, 36 & 1);
	char amountStr[10];
	sprintf(amountStr, "%04d", data.word(kCard1money) / 10);
	_charShift = 91 * 2 + 75;
	printDirect((const uint8 *)amountStr, 160, 155, 240, 240 & 1);
	sprintf(amountStr, "%02d", (data.word(kCard1money) % 10) * 10);
	_charShift = 91 * 2 + 85;
	printDirect((const uint8 *)amountStr, 187, 155, 240, 240 & 1);
	_charShift = 0;
	workToScreenM();
	hangOnP(400);
	getRidOfTemp();
	restoreReels();
	putBackObStuff();
}

void DreamBase::useStereo() {
	// Handles the stereo in Ryan's apartment (accessible from the remote on
	// the couch)

	if (data.byte(kLocation) != 0) {
		showPuzText(4, 400);
		putBackObStuff();
	} else if (_mapX != 11) {
		showPuzText(5, 400);
		putBackObStuff();
	} else if (checkInside(findSetObject("CDPL"), 1) == kNumexobjects) {
		// No CD inside
		showPuzText(6, 400);
		putBackObStuff();
		uint8 dummy;
		DynObject *object = (DynObject *)getAnyAd(&dummy, &dummy);
		object->turnedOn = 255;
	} else {
		// CD inside
		uint8 dummy;
		DynObject *object = (DynObject *)getAnyAd(&dummy, &dummy);
		object->turnedOn ^= 1;
		if (object->turnedOn != 255)
			showPuzText(7, 400);	// Stereo off
		else
			showPuzText(8, 400);	// Stereo on

		putBackObStuff();
	}
}

uint16 DreamBase::checkInside(uint16 command, uint16 type) {
	for (uint16 index = 0; index < kNumexobjects; index++) {
		DynObject *object = getExAd(index);
		if (object->mapad[1] == command && object->mapad[0] == type)
			return index;
	}

	return kNumexobjects;
}

void DreamBase::showPuzText(uint16 command, uint16 count) {
	createPanel();
	showPanel();
	showMan();
	showExit();
	obIcons();
	const uint8 *string = (const uint8 *)_puzzleText.getString(command);
	printDirect(string, 36, 104, 241, 241 & 1);
	workToScreenM();
	hangOnP(count);
}

void DreamBase::useButtonA() {
	if (!isSetObOnMap(95)) {
		showFirstUse();
		turnAnyPathOn(0, _roomNum - 1);
		removeSetObject(9);
		placeSetObject(95);
		data.word(kWatchingtime) = 15 * 2;
		data.word(kReeltowatch) = 71;
		data.word(kEndwatchreel) = 85;
		data.byte(kWatchspeed) = 1;
		data.byte(kSpeedcount) = 1;
		_getBack = 1;
		data.byte(kProgresspoints)++;
	} else {
		// Done this bit
		showSecondUse();
		putBackObStuff();
	}
}


} // End of namespace DreamGen
