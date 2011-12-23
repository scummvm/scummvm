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


typedef void (DreamGenContext::*UseCallback)(void);

// Note: The callback pointer has been placed before the
// ID to keep MSVC happy (otherwise, it throws warnings
// that alignment of a member was sensitive to packing)
struct UseListEntry {
	UseCallback callback;
	const char *id;
};

void DreamGenContext::useRoutine() {

	static const UseListEntry kUseList[] = {
		{ &DreamGenContext::useMon,            "NETW" },
		{ &DreamGenContext::useElevator1,      "ELVA" },
		{ &DreamGenContext::useElevator2,      "ELVB" },
		{ &DreamGenContext::useElevator3,      "ELVC" },
		{ &DreamGenContext::useElevator4,      "ELVE" },
		{ &DreamGenContext::useElevator5,      "ELVF" },
		{ &DreamGenContext::useChurchGate,     "CGAT" },
		{ &DreamGenContext::useStereo,         "REMO" },
		{ &DreamGenContext::useButtonA,        "BUTA" },
		{ &DreamGenContext::useWinch,          "CBOX" },
		{ &DreamGenContext::useLighter,        "LITE" },
		{ &DreamGenContext::usePlate,          "PLAT" },
		{ &DreamGenContext::useControl,        "LIFT" },
		{ &DreamGenContext::useWire,           "WIRE" },
		{ &DreamGenContext::useHandle,         "HNDL" },
		{ &DreamGenContext::useHatch,          "HACH" },
		{ &DreamGenContext::useElvDoor,        "DOOR" },
		{ &DreamGenContext::useCashCard,       "CSHR" },
		{ &DreamGenContext::useGun,            "GUNA" },
		{ &DreamGenContext::useCardReader1,    "CRAA" },
		{ &DreamGenContext::useCardReader2,    "CRBB" },
		{ &DreamGenContext::useCardReader3,    "CRCC" },
		{ &DreamGenContext::sitDownInBar,      "SEAT" },
		{ &DreamGenContext::useMenu,           "MENU" },
		{ &DreamGenContext::useCooker,         "COOK" },
		{ &DreamGenContext::callHotelLift,     "ELCA" },
		{ &DreamGenContext::callEdensLift,     "EDCA" },
		{ &DreamGenContext::callEdensDLift,    "DDCA" },
		{ &DreamGenContext::useAltar,          "ALTR" },
		{ &DreamGenContext::openHotelDoor,     "LOKA" },
		{ &DreamGenContext::openHotelDoor2,    "LOKB" },
		{ &DreamGenContext::openLouis,         "ENTA" },
		{ &DreamGenContext::openRyan,          "ENTB" },
		{ &DreamGenContext::openPoolBoss,      "ENTE" },
		{ &DreamGenContext::openYourNeighbour, "ENTC" },
		{ &DreamGenContext::openEden,          "ENTD" },
		{ &DreamGenContext::openSarters,       "ENTH" },
		{ &DreamGenContext::wearWatch,         "WWAT" },
		{ &DreamGenContext::usePoolReader,     "POOL" },
		{ &DreamGenContext::wearShades,        "WSHD" },
		{ &DreamGenContext::grafittiDoor,      "GRAF" },
		{ &DreamGenContext::trapDoor,          "TRAP" },
		{ &DreamGenContext::edensCDPlayer,     "CDPE" },
		{ &DreamGenContext::openTVDoor,        "DLOK" },
		{ &DreamGenContext::useHole,           "HOLE" },
		{ &DreamGenContext::useDryer,          "DRYR" },
		{ &DreamGenContext::useChurchHole,     "HOLY" },
		{ &DreamGenContext::useWall,           "WALL" },
		{ &DreamGenContext::useDiary,          "BOOK" },
		{ &DreamGenContext::useAxe,            "AXED" },
		{ &DreamGenContext::useShield,         "SHLD" },
		{ &DreamGenContext::useRailing,        "BCNY" },
		{ &DreamGenContext::useCoveredBox,     "LIDC" },
		{ &DreamGenContext::useClearBox,       "LIDU" },
		{ &DreamGenContext::useOpenBox,        "LIDO" },
		{ &DreamGenContext::usePipe,           "PIPE" },
		{ &DreamGenContext::useBalcony,        "BALC" },
		{ &DreamGenContext::useWindow,         "WIND" },
		{ &DreamGenContext::viewFolder,        "PAPR" },
		{ &DreamGenContext::useTrainer,        "UWTA" },
		{ &DreamGenContext::useTrainer,        "UWTB" },
		{ &DreamGenContext::enterSymbol,       "STAT" },
		{ &DreamGenContext::openTomb,          "TLID" },
		{ &DreamGenContext::useSLab,           "SLAB" },
		{ &DreamGenContext::useCart,           "CART" },
		{ &DreamGenContext::useFullCart,       "FCAR" },
		{ &DreamGenContext::sLabDoorA,         "SLBA" },
		{ &DreamGenContext::sLabDoorB,         "SLBB" },
		{ &DreamGenContext::sLabDoorC,         "SLBC" },
		{ &DreamGenContext::sLabDoorD,         "SLBD" },
		{ &DreamGenContext::sLabDoorE,         "SLBE" },
		{ &DreamGenContext::sLabDoorF,         "SLBF" },
		{ &DreamGenContext::usePlinth,         "PLIN" },
		{ &DreamGenContext::useLadder,         "LADD" },
		{ &DreamGenContext::useLadderB,        "LADB" },
		{ &DreamGenContext::chewy,             "GUMA" },
		{ &DreamGenContext::wheelSound,        "SQEE" },
		{ &DreamGenContext::runTap,            "TAPP" },
		{ &DreamGenContext::playGuitar,        "GUIT" },
		{ &DreamGenContext::hotelControl,      "CONT" },
		{ &DreamGenContext::hotelBell,         "BELL" },
	};

	if (data.byte(kReallocation) >= 50) {
		if (data.byte(kPointerpower) == 0)
			return;
		data.byte(kPointerpower) = 0;
	}

	getAnyAd();
	const uint8 *id = es.ptr(bx + 12, 4);

	for (size_t i = 0; i < sizeof(kUseList)/sizeof(UseListEntry); ++i) {
		const UseListEntry &entry = kUseList[i];
		if (('A' + id[0] == entry.id[0]) && ('A' + id[1] == entry.id[1]) && ('A' + id[2] == entry.id[2]) && ('A' + id[3] == entry.id[3])) {
			(this->*entry.callback)();
			return;
		}
	}

	delPointer();
	const uint8 *obText = getObTextStartCPP();
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
	data.byte(kCommandtype) = 255;
}

void DreamGenContext::useText(const uint8 *string) {
	createPanel();
	showPanel();
	showMan();
	showExit();
	obIcons();
	printDirect(string, 36, 104, 241, true);
	workToScreenM();
}

void DreamGenContext::showFirstUse() {
	const uint8 *obText = getObTextStartCPP();
	findNextColon(&obText);
	findNextColon(&obText);
	useText(obText);
	hangOnP(400);
}

void DreamGenContext::showSecondUse() {
	const uint8 *obText = getObTextStartCPP();
	findNextColon(&obText);
	findNextColon(&obText);
	findNextColon(&obText);
	useText(obText);
	hangOnP(400);
}

void DreamGenContext::viewFolder() {
	data.byte(kManisoffscreen) = 1;
	getRidOfAll();
	loadFolder();
	data.byte(kFolderpage) = 0;
	showFolder();
	workToScreenM();
	data.byte(kGetback) = 0;
	do {
		if (quitRequested())
			break;
		delPointer();
		readMouse();
		showPointer();
		vSync();
		dumpPointer();
		dumpTextLine();
		checkFolderCoords();
	} while (data.byte(kGetback) == 0);
	data.byte(kManisoffscreen) = 0;
	getRidOfTemp();
	getRidOfTemp2();
	getRidOfTemp3();
	getRidOfTempCharset();
	restoreAll();
	redrawMainScrn();
	workToScreenM();
}

void DreamGenContext::edensCDPlayer() {
	showFirstUse();
	data.word(kWatchingtime) = 18 * 2;
	data.word(kReeltowatch) = 25;
	data.word(kEndwatchreel) = 42;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kGetback) = 1;
}

void DreamGenContext::hotelBell() {
	playChannel1(12);
	showFirstUse();
	putBackObStuff();
}

void DreamGenContext::playGuitar() {
	playChannel1(14);
	showFirstUse();
	putBackObStuff();
}

void DreamGenContext::useElevator1() {
	showFirstUse();
	selectLocation();
	data.byte(kGetback) = 1;
}

void DreamGenContext::useElevator2() {
	showFirstUse();

	if (data.byte(kLocation) == 23)	// In pool hall
		data.byte(kNewlocation) = 31;
	else
		data.byte(kNewlocation) = 23;

	data.byte(kCounttoclose) = 20;
	data.byte(kCounttoopen) = 0;
	data.word(kWatchingtime) = 80;
	data.byte(kGetback) = 1;
}

void DreamGenContext::useElevator3() {
	showFirstUse();
	data.byte(kCounttoclose) = 20;
	data.byte(kNewlocation) = 34;
	data.word(kReeltowatch) = 46;
	data.word(kEndwatchreel) = 63;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.word(kWatchingtime) = 80;
	data.byte(kGetback) = 1;
}

void DreamGenContext::useElevator4() {
	showFirstUse();
	data.word(kReeltowatch) = 0;
	data.word(kEndwatchreel) = 11;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kCounttoclose) = 20;
	data.word(kWatchingtime) = 80;
	data.byte(kGetback) = 1;
	data.byte(kNewlocation) = 24;
}

void DreamGenContext::useElevator5() {
	placeSetObject(4);
	removeSetObject(0);
	data.byte(kNewlocation) = 20;
	data.word(kWatchingtime) = 80;
	data.byte(kLiftflag) = 1;
	data.byte(kCounttoclose) = 8;
	data.byte(kGetback) = 1;
}

void DreamGenContext::useHatch() {
	showFirstUse();
	data.byte(kNewlocation) = 40;
	data.byte(kGetback) = 1;
}

void DreamGenContext::wheelSound() {
	playChannel1(17);
	showFirstUse();
	putBackObStuff();
}

void DreamGenContext::callHotelLift() {
	playChannel1(12);
	showFirstUse();
	data.byte(kCounttoopen) = 8;
	data.byte(kGetback) = 1;
	data.byte(kDestination) = 5;
	data.byte(kFinaldest) = 5;
	autoSetWalk();
	turnPathOn(4);
}

void DreamGenContext::useShield() {
	if (data.byte(kReallocation) != 20 || data.byte(kCombatcount) == 0) {
		// Not in Sart room
		showFirstUse();
		putBackObStuff();
	} else {
		data.byte(kLastweapon) = 3;
		showSecondUse();
		data.byte(kGetback) = 1;
		data.byte(kProgresspoints)++;
		removeObFromInv();
	}
}

void DreamGenContext::useCoveredBox() {
	data.byte(kProgresspoints)++;
	showFirstUse();
	data.word(kWatchingtime) = 50;
	data.word(kReeltowatch) = 41;
	data.word(kEndwatchreel) = 66;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kGetback) = 1;
}

void DreamGenContext::useRailing() {
	showFirstUse();
	data.word(kWatchingtime) = 80;
	data.word(kReeltowatch) = 0;
	data.word(kEndwatchreel) = 30;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kGetback) = 1;
	data.byte(kMandead) = 4;
}

void DreamGenContext::wearWatch() {
	if (data.byte(kWatchon) == 1) {
		// Already wearing watch
		showSecondUse();
		putBackObStuff();
	} else {
		showFirstUse();
		data.byte(kWatchon) = 1;
		data.byte(kGetback) = 1;
		uint8 dummy;
		makeWorn((DynObject *)getAnyAd(&dummy, &dummy));
	}
}

void DreamGenContext::wearShades() {
	if (data.byte(kShadeson) == 1) {
		// Already wearing shades
		showSecondUse();
		putBackObStuff();
	} else {
		data.byte(kShadeson) = 1;
		showFirstUse();
		data.byte(kGetback) = 1;
		uint8 dummy;
		makeWorn((DynObject *)getAnyAd(&dummy, &dummy));
	}
}

void DreamGenContext::useChurchHole() {
	showFirstUse();
	data.byte(kGetback) = 1;
	data.word(kWatchingtime) = 28;
	data.word(kReeltowatch) = 13;
	data.word(kEndwatchreel) = 26;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
}

void DreamGenContext::sitDownInBar() {
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
		data.byte(kGetback) = 1;
	}
}

void DreamGenContext::useDryer() {
	playChannel1(12);
	showFirstUse();
	data.byte(kGetback) = 1;
}

void DreamGenContext::useBalcony() {
	showFirstUse();
	turnPathOn(6);
	turnPathOff(0);
	turnPathOff(1);
	turnPathOff(2);
	turnPathOff(3);
	turnPathOff(4);
	turnPathOff(5);
	data.byte(kProgresspoints)++;
	data.byte(kManspath) = 6;
	data.byte(kDestination) = 6;
	data.byte(kFinaldest) = 6;
	findXYFromPath();
	switchRyanOff();
	data.byte(kResetmanxy) = 1;
	data.word(kWatchingtime) = 30 * 2;
	data.word(kReeltowatch) = 183;
	data.word(kEndwatchreel) = 212;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kGetback) = 1;
}

void DreamGenContext::useWindow() {
	if (data.byte(kManspath) != 6) {
		// Not on balcony
		showSecondUse();
		putBackObStuff();
	} else {
		data.byte(kProgresspoints)++;
		showFirstUse();
		data.byte(kNewlocation) = 29;
		data.byte(kGetback) = 1;
	}
}

void DreamGenContext::trapDoor() {
	data.byte(kProgresspoints)++;
	showFirstUse();
	switchRyanOff();
	data.word(kWatchingtime) = 20 * 2;
	data.word(kReeltowatch) = 181;
	data.word(kEndwatchreel) = 197;
	data.byte(kNewlocation) = 26;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kGetback) = 1;
}

void DreamGenContext::callEdensLift() {
	showFirstUse();
	data.byte(kCounttoopen) = 8;
	data.byte(kGetback) = 1;
	turnPathOn(2);
}

void DreamGenContext::callEdensDLift() {
	if (data.byte(kLiftflag) == 1) {
		// Eden's D here
		showSecondUse();
		putBackObStuff();
	} else {
		showFirstUse();
		data.byte(kCounttoopen) = 8;
		data.byte(kGetback) = 1;
		turnPathOn(2);
	}
}

void DreamGenContext::openYourNeighbour() {
	enterCode(255, 255, 255, 255);
	data.byte(kGetback) = 1;
}

void DreamGenContext::openRyan() {
	enterCode(5, 1, 0, 6);
	data.byte(kGetback) = 1;
}

void DreamGenContext::openPoolBoss() {
	enterCode(5, 2, 2, 2);
	data.byte(kGetback) = 1;
}

void DreamGenContext::openEden() {
	enterCode(2, 8, 6, 5);
	data.byte(kGetback) = 1;
}

void DreamGenContext::openSarters() {
	enterCode(7, 8, 3, 3);
	data.byte(kGetback) = 1;
}

void DreamGenContext::openLouis() {
	enterCode(5, 2, 3, 8);
	data.byte(kGetback) = 1;
}


void DreamGenContext::useWall() {
	showFirstUse();

	if (data.byte(kManspath) != 3) {
		data.word(kWatchingtime) = 30*2;
		data.word(kReeltowatch) = 2;
		data.word(kEndwatchreel) = 31;
		data.byte(kWatchspeed) = 1;
		data.byte(kSpeedcount) = 1;
		data.byte(kGetback) = 1;
		turnPathOn(3);
		turnPathOn(4);
		turnPathOff(0);
		turnPathOff(1);
		turnPathOff(2);
		turnPathOff(5);
		data.byte(kManspath) = 3;
		data.byte(kFinaldest) = 3;
		findXYFromPath();
		data.byte(kResetmanxy) = 1;
		switchRyanOff();
	} else {
		// Go back over
		data.word(kWatchingtime) = 30 * 2;
		data.word(kReeltowatch) = 34;
		data.word(kEndwatchreel) = 60;
		data.byte(kWatchspeed) = 1;
		data.byte(kSpeedcount) = 1;
		data.byte(kGetback) = 1;
		turnPathOff(3);
		turnPathOff(4);
		turnPathOn(0);
		turnPathOn(1);
		turnPathOn(2);
		turnPathOn(5);
		data.byte(kManspath) = 5;
		data.byte(kFinaldest) = 5;
		findXYFromPath();
		data.byte(kResetmanxy) = 1;
		switchRyanOff();
	}
}

void DreamGenContext::useLadder() {
	showFirstUse();
	data.byte(kMapx) = data.byte(kMapx) - 11;
	findRoomInLoc();
	data.byte(kFacing) = 6;
	data.byte(kTurntoface) = 6;
	data.byte(kManspath) = 0;
	data.byte(kDestination) = 0;
	data.byte(kFinaldest) = 0;
	findXYFromPath();
	data.byte(kResetmanxy) = 1;
	data.byte(kGetback) = 1;
}

void DreamGenContext::useLadderB() {
	showFirstUse();
	data.byte(kMapx) = data.byte(kMapx) + 11;
	findRoomInLoc();
	data.byte(kFacing) = 2;
	data.byte(kTurntoface) = 2;
	data.byte(kManspath) = 1;
	data.byte(kDestination) = 1;
	data.byte(kFinaldest) = 1;
	findXYFromPath();
	data.byte(kResetmanxy) = 1;
	data.byte(kGetback) = 1;
}

void DreamGenContext::sLabDoorA() {
	showFirstUse();
	data.byte(kGetback) = 1;
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
		data.byte(kNewlocation) = 47;
	}
}

void DreamGenContext::sLabDoorB() {
	if (data.byte(kDreamnumber) != 1) {
		// Wrong
		showFirstUse();
		data.byte(kGetback) = 1;
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
			data.byte(kGetback) = 1;
			data.byte(kWatchspeed) = 1;
			data.byte(kSpeedcount) = 1;
			data.word(kReeltowatch) = 44;
			data.word(kWatchingtime) = 60;
			data.word(kEndwatchreel) = 71;
			data.byte(kNewlocation) = 47;
		}
	}
}

void DreamGenContext::sLabDoorC() {
	showFirstUse();
	data.byte(kGetback) = 1;
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
		data.byte(kNewlocation) = 47;
	}
}

void DreamGenContext::sLabDoorD() {
	showFirstUse();
	data.byte(kGetback) = 1;
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
		data.byte(kNewlocation) = 47;
	}
}

void DreamGenContext::sLabDoorE() {
	showFirstUse();
	data.byte(kGetback) = 1;
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
		data.byte(kNewlocation) = 47;
	}
}

void DreamGenContext::sLabDoorF() {
	showFirstUse();
	data.byte(kGetback) = 1;
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
		data.byte(kNewlocation) = 47;
	}
}

bool DreamGenContext::defaultUseHandler(const char *id) {
	if (data.byte(kWithobject) == 255) {
		withWhat();
		return true;	// event handled
	}

	if (!compare(data.byte(kWithobject), data.byte(kWithtype), id)) {
		// Wrong item
		showPuzText(14, 300);
		putBackObStuff();
		return true;	// event handled
	}

	return false;	// continue with the original event
}

void DreamGenContext::useChurchGate() {
	if (defaultUseHandler("CUTT"))
		return;

	// Cut gate
	showFirstUse();
	data.word(kWatchingtime) = 64 * 2;
	data.word(kReeltowatch) = 4;
	data.word(kEndwatchreel) = 70;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kGetback) = 1;
	data.byte(kProgresspoints)++;
	turnPathOn(3);
	if (data.byte(kAidedead) != 0)
		turnPathOn(2);	// Open church
}

void DreamGenContext::useGun() {

	if (data.byte(kObjecttype) != kExObjectType) {
		// gun is not taken
		showSecondUse();
		putBackObStuff();

	} else if (data.byte(kReallocation) == 22) {
		// in pool room
		showPuzText(34, 300);
		data.byte(kLastweapon) = 1;
		data.byte(kCombatcount) = 39;
		data.byte(kGetback) = 1;
		data.byte(kProgresspoints)++;

	} else if (data.byte(kReallocation) == 25) {
		// helicopter
		showPuzText(34, 300);
		data.byte(kLastweapon) = 1;
		data.byte(kCombatcount) = 19;
		data.byte(kGetback) = 1;
		data.byte(kDreamnumber) = 2;
		data.byte(kRoomafterdream) = 38;
		data.byte(kSartaindead) = 1;
		data.byte(kProgresspoints)++;

	} else if (data.byte(kReallocation) == 27) {
		// in rock room
		showPuzText(46, 300);
		data.byte(kPointermode) = 2;
		data.byte(kRockstardead) = 1;
		data.byte(kLastweapon) = 1;
		data.byte(kNewsitem) = 1;
		data.byte(kGetback) = 1;
		data.byte(kRoomafterdream) = 32;
		data.byte(kDreamnumber) = 0;
		data.byte(kProgresspoints)++;

	} else if (data.byte(kReallocation) == 8 && data.byte(kMapx) == 22 && data.byte(kMapy) == 40
	    && !isSetObOnMap(92) && data.byte(kManspath) != 9) {
		// by studio
		data.byte(kDestination) = 9;
		data.byte(kFinaldest) = 9;
		autoSetWalk();
		data.byte(kLastweapon) = 1;
		data.byte(kGetback) = 1;
		data.byte(kProgresspoints)++;

	} else if (data.byte(kReallocation) == 6 && data.byte(kMapx) == 11 && data.byte(kMapy) == 20
	    && isSetObOnMap(5)) {
		// sarters
		data.byte(kDestination) = 1;
		data.byte(kFinaldest) = 1;
		autoSetWalk();
		removeSetObject(5);
		placeSetObject(6);
		turnAnyPathOn(1, data.byte(kRoomnum) - 1);
		data.byte(kLiftflag) = 1;
		data.word(kWatchingtime) = 40*2;
		data.word(kReeltowatch) = 4;
		data.word(kEndwatchreel) = 43;
		data.byte(kWatchspeed) = 1;
		data.byte(kSpeedcount) = 1;
		data.byte(kGetback) = 1;
		data.byte(kProgresspoints)++;

	} else if (data.byte(kReallocation) == 29) {
		// aide
		data.byte(kGetback) = 1;
		resetLocation(13);
		setLocation(12);
		data.byte(kDestpos) = 12;
		data.byte(kDestination) = 2;
		data.byte(kFinaldest) = 2;
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

	} else if (data.byte(kReallocation) == 23 && data.byte(kMapx) == 0 && data.byte(kMapy) == 50) {
		// with boss
		if (data.byte(kManspath) != 5) {
			data.byte(kDestination) = 5;
			data.byte(kFinaldest) = 5;
			autoSetWalk();
		}
		data.byte(kLastweapon) = 1;
		data.byte(kGetback) = 1;

	} else if (data.byte(kReallocation) == 8 && data.byte(kMapx) == 11 && data.byte(kMapy) == 10) {
		// tv soldier
		if (data.byte(kManspath) != 2) {
			data.byte(kDestination) = 2;
			data.byte(kFinaldest) = 2;
			autoSetWalk();
		}
		data.byte(kLastweapon) = 1;
		data.byte(kGetback) = 1;

	} else {
		showFirstUse();
		putBackObStuff();
	}
}

void DreamGenContext::useFullCart() {
	data.byte(kProgresspoints)++;
	turnAnyPathOn(2, data.byte(kRoomnum) + 6);
	data.byte(kManspath) = 4;
	data.byte(kFacing) = 4;
	data.byte(kTurntoface) = 4;
	data.byte(kFinaldest) = 4;
	findXYFromPath();
	data.byte(kResetmanxy) = 1;
	showFirstUse();
	data.word(kWatchingtime) = 72 * 2;
	data.word(kReeltowatch) = 58;
	data.word(kEndwatchreel) = 142;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kGetback) = 1;
}

void DreamGenContext::useClearBox() {
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
	data.byte(kGetback) = 1;
}

void DreamGenContext::openTVDoor() {
	if (defaultUseHandler("ULOK"))
		return;

	// Key on TV
	showFirstUse();
	data.byte(kLockstatus) = 0;
	data.byte(kGetback) = 1;
}

void DreamGenContext::usePlate() {
	if (data.byte(kWithobject) == 255) {
		withWhat();
		return;
	}

	if (compare(data.byte(kWithobject), data.byte(kWithtype), "SCRW")) {
		// Unscrew plate
		playChannel1(20);
		showFirstUse();
		placeSetObject(28);
		placeSetObject(24);
		removeSetObject(25);
		placeFreeObject(0);
		data.byte(kProgresspoints)++;
		data.byte(kGetback) = 1;
	} else if (compare(data.byte(kWithobject), data.byte(kWithtype), "KNFE")) {
		// Tried knife
		showPuzText(54, 300);
		putBackObStuff();
	} else {
		// Wrong item
		showPuzText(14, 300);
		putBackObStuff();
	}
}

void DreamGenContext::usePlinth() {
	if (data.byte(kWithobject) == 255) {
		withWhat();
		return;
	}

	if (!compare(data.byte(kWithobject), data.byte(kWithtype), "DKEY")) {
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
		data.byte(kGetback) = 1;
		data.byte(kNewlocation) = data.byte(kRoomafterdream);
	}
}

void DreamGenContext::useElvDoor() {
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
	data.byte(kGetback) = 1;
}

void DreamGenContext::useObject() {
	data.byte(kWithobject) = 255;

	if (data.byte(kCommandtype) != 229) {
		data.byte(kCommandtype) = 229;
		commandWithOb(51, data.byte(kObjecttype), data.byte(kCommand));
	}

	if (data.word(kMousebutton) == data.word(kOldbutton))
		return;	// nouse

	if (data.word(kMousebutton) & 1)
		useRoutine();
}

void DreamGenContext::useWinch() {
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
	data.byte(kDestpos) = 1;
	data.byte(kNewlocation) = 45;
	data.byte(kDreamnumber) = 1;
	data.byte(kRoomafterdream) = 44;
	data.byte(kGeneraldead) = 1;
	data.byte(kNewsitem) = 2;
	data.byte(kGetback) = 1;
	data.byte(kProgresspoints)++;
}

void DreamGenContext::useCart() {
	if (defaultUseHandler("ROCK"))
		return;

	DynObject *exObject = getExAd(data.byte(kWithobject));
	exObject->mapad[0] = 0;
	removeSetObject(data.byte(kCommand));
	placeSetObject(data.byte(kCommand) + 1);
	data.byte(kProgresspoints)++;
	playChannel1(17);
	showFirstUse();
	data.byte(kGetback) = 1;
}

void DreamGenContext::useTrainer() {
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

void DreamGenContext::chewy() {
	showFirstUse();
	// TODO: Use the C++ version of getAnyAd()
	getAnyAd();
	es.byte(bx + 2) = 255;
	data.byte(kGetback) = 1;
}

void DreamGenContext::useHole() {
	if (defaultUseHandler("HNDA"))
		return;

	showFirstUse();
	removeSetObject(86);
	DynObject *exObject = getExAd(data.byte(kWithobject));
	exObject->mapad[0] = 255;
	data.byte(kCanmovealtar) = 1;
	data.byte(kGetback) = 1;
}

void DreamGenContext::openHotelDoor() {
	if (defaultUseHandler("KEYA"))
		return;

	playChannel1(16);
	showFirstUse();
	data.byte(kLockstatus) = 0;
	data.byte(kGetback) = 1;
}

void DreamGenContext::openHotelDoor2() {
	if (defaultUseHandler("KEYA"))
		return;

	playChannel1(16);
	showFirstUse();
	putBackObStuff();
}

void DreamGenContext::grafittiDoor() {
	if (defaultUseHandler("APEN"))
		return;

	showFirstUse();
	putBackObStuff();
}

void DreamGenContext::usePoolReader() {
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
		data.byte(kGetback) = 1;
	}
}

void DreamGenContext::useCardReader1() {
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
		data.byte(kGetback) = 1;
	}
}

void DreamGenContext::useCardReader2() {
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
		data.byte(kGetback) = 1;
	}
}

void DreamGenContext::useCardReader3() {
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
		data.byte(kGetback) = 1;
	}
}

void DreamGenContext::useLighter() {
	if (data.byte(kWithobject) == 255) {
		withWhat();
		return;
	}

	if (!compare(data.byte(kWithobject), data.byte(kWithtype), "SMKE")) {
		showFirstUse();
		putBackObStuff();
	} else {
		showPuzText(9, 300);
		DynObject *withObj = getExAd(data.byte(kWithobject));
		withObj->mapad[0] = 255;
		data.byte(kGetback) = 1;
	}
}

void DreamGenContext::useWire() {
	if (data.byte(kWithobject) == 255) {
		withWhat();
		return;
	}

	if (compare(data.byte(kWithobject), data.byte(kWithtype), "KNFE")) {
		removeSetObject(51);
		placeSetObject(52);
		showPuzText(11, 300);
		data.byte(kProgresspoints)++;
		data.byte(kGetback) = 1;
		return;
	}

	if (compare(data.byte(kWithobject), data.byte(kWithtype), "AXED")) {
		showPuzText(16, 300);
		putBackObStuff();
		return;
	}

	showPuzText(14, 300);
	putBackObStuff();
}

void DreamGenContext::openTomb() {
	data.byte(kProgresspoints)++;
	showFirstUse();
	data.word(kWatchingtime) = 35 * 2;
	data.word(kReeltowatch) = 1;
	data.word(kEndwatchreel) = 33;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kGetback) = 1;
}

void DreamGenContext::hotelControl() {
	if (data.byte(kReallocation) != 21 || data.byte(kMapx) != 33)
		showSecondUse();	// Not right control
	else
		showFirstUse();

	putBackObStuff();
}

void DreamGenContext::useCooker() {
	if (checkInside(data.byte(kCommand), data.byte(kObjecttype)) == kNumexobjects)
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

void DreamGenContext::useDiary() {
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
	workToScreenCPP();
	delPointer();
	data.byte(kGetback) = 0;

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
	} while (!data.byte(kGetback) && !quitRequested());


	getRidOfTemp();
	getRidOfTempText();
	getRidOfTempCharset();
	restoreReels();
	data.byte(kManisoffscreen) = 0;
	redrawMainScrn();
	workToScreenM();
}

void DreamGenContext::useControl() {
	if (data.byte(kWithobject) == 255) {
		withWhat();
		return;
	}

	if (compare(data.byte(kWithobject), data.byte(kWithtype), "KEYA")) {	// Right key
		playChannel1(16);
		if (data.byte(kLocation) == 21) {	// Going down
			showPuzText(3, 300);
			data.byte(kNewlocation) = 30;
		} else {
			showPuzText(0, 300);
			data.byte(kNewlocation) = 21;
		}

		data.byte(kCounttoclose) = 8;
		data.byte(kCounttoopen) = 0;
		data.word(kWatchingtime) = 80;
		data.byte(kGetback) = 1;
		return;
	}

	if (data.byte(kReallocation) == 21) {
		if (compare(data.byte(kWithobject), data.byte(kWithtype), "KNFE")) {
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
			data.byte(kGetback) = 1;
		} else if (compare(data.byte(kWithobject), data.byte(kWithtype), "AXED")) {
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

void DreamGenContext::useSLab() {
	if (data.byte(kWithobject) == 255) {
		withWhat();
		return;
	}

	if (!compare(data.byte(kWithobject), data.byte(kWithtype), "JEWL")) {
		showPuzText(14, 300);
		putBackObStuff();
		return;
	}

	DynObject *exObject = getExAd(data.byte(kWithobject));
	exObject->mapad[0] = 0;

	removeSetObject(data.byte(kCommand));
	placeSetObject(data.byte(kCommand) + 1);
	if (data.byte(kCommand) + 1 == 54) {
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
	data.byte(kGetback) = 1;
}

void DreamGenContext::usePipe() {
	if (data.byte(kWithobject) == 255) {
		withWhat();
		return;
	}

	if (compare(data.byte(kWithobject), data.byte(kWithtype), "CUPE")) {
		// Fill cup
		showPuzText(36, 300);
		putBackObStuff();
		DynObject *exObject = getExAd(data.byte(kWithobject));
		exObject->id[3] = 'F'-'A';	// CUPE (empty cup) -> CUPF (full cup)
		return;
	} else if (compare(data.byte(kWithobject), data.byte(kWithtype), "CUPF")) {
		// Already full
		showPuzText(35, 300);
		putBackObStuff();
	} else {
		showPuzText(14, 300);
		putBackObStuff();
	}
}

void DreamGenContext::useOpenBox() {
	if (data.byte(kWithobject) == 255) {
		withWhat();
		return;
	}

	if (compare(data.byte(kWithobject), data.byte(kWithtype), "CUPF")) {
		// Destroy open box
		data.byte(kProgresspoints)++;
		showPuzText(37, 300);
		DynObject *exObject = getExAd(data.byte(kWithobject));
		exObject->id[3] = 'E'-'A';	// CUPF (full cup) -> CUPE (empty cup)
		data.word(kWatchingtime) = 140;
		data.word(kReeltowatch) = 105;
		data.word(kEndwatchreel) = 181;
		data.byte(kWatchspeed) = 1;
		data.byte(kSpeedcount) = 1;
		turnPathOn(4);
		data.byte(kGetback) = 1;
		return;
	}

	if (compare(data.byte(kWithobject), data.byte(kWithtype), "CUPE")) {
		// Open box wrong
		showPuzText(38, 300);
		putBackObStuff();
		return;
	}

	showFirstUse();
}

void DreamGenContext::runTap() {
	if (data.byte(kWithobject) == 255) {
		withWhat();
		return;
	}

	if (compare(data.byte(kWithobject), data.byte(kWithtype), "CUPE")) {
		// Fill cup from tap
		DynObject *exObject = getExAd(data.byte(kWithobject));
		exObject->id[3] = 'F'-'A';	// CUPE (empty cup) -> CUPF (full cup)
		playChannel1(8);
		showPuzText(57, 300);
		putBackObStuff();
		return;
	}

	if (compare(data.byte(kWithobject), data.byte(kWithtype), "CUPF")) {
		// Cup from tap full
		showPuzText(58, 300);
		putBackObStuff();
		return;
	}

	showPuzText(56, 300);
	putBackObStuff();
}

void DreamGenContext::useAxe() {
	if (data.byte(kReallocation) != 22) {
		// Not in pool
		showFirstUse();
		return;
	}

	if (data.byte(kMapy) == 10) {
		// Axe on door
		showPuzText(15, 300);
		data.byte(kProgresspoints)++;
		data.word(kWatchingtime) = 46*2;
		data.word(kReeltowatch) = 31;
		data.word(kEndwatchreel) = 77;
		data.byte(kWatchspeed) = 1;
		data.byte(kSpeedcount) = 1;
		data.byte(kGetback) = 1;
		return;
	}

	showSecondUse();
	data.byte(kProgresspoints)++;
	data.byte(kLastweapon) = 2;
	data.byte(kGetback) = 1;
	removeObFromInv();
}

void DreamGenContext::useKey() {
	switch(data.byte(kLocation)) {
	case 5:
	case 30:
		if (data.byte(kMapx) == 22 && data.byte(kMapy) == 10) {
			showPuzText(0, 300);
			data.byte(kCounttoclose) = 100;
			data.byte(kGetback) = 1;
		} else {
			// Wrong room
			showPuzText(2, 200);
			putBackObStuff();
		}
		break;
	case 21:
		if (data.byte(kMapx) == 11 && data.byte(kMapy) == 10) {
			showPuzText(3, 300);
			data.byte(kNewlocation) = 30;
			al = 2;
			fadeScreenDown();
			showFirstUse();
			putBackObStuff();
		} else {
			// Wrong room
			showPuzText(2, 200);
			putBackObStuff();
		}
	default:
		showPuzText(1, 200);
		putBackObStuff();
		break;
	}
}

void DreamGenContext::useHandle() {
	SetObject *object = getSetAd(findSetObject("CUTW"));
	if (object->mapad[0] == 255) {
		// Wire not cut
		showPuzText(12, 300);
	} else {
		// Wire has been cut
		showPuzText(13, 300);
		data.byte(kNewlocation) = 22;
	}

	data.byte(kGetback) = 1;
}

void DreamGenContext::useAltar() {
	if (findExObject("CNDA") == 114 || findExObject("CNDB") == 114) {
		// Things on altar
		showFirstUse();
		data.byte(kGetback) = 1;
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
		data.byte(kGetback) = 1;
	} else {
		showPuzText(23, 300);
		data.byte(kGetback) = 1;
	}
}

void DreamBase::withWhat() {
	uint8 commandLine[64] = "OBJECT NAME ONE                         ";

	createPanel();
	showPanel();
	showMan();
	showExit();

	copyName(data.byte(kObjecttype), data.byte(kCommand), commandLine);
	printMessage2(100, 21, 63, 200, false, 2);
	uint16 x = data.word(kLastxpos) + 5;
	printDirect(commandLine, x, 21, 220, false);
	printMessage2(data.word(kLastxpos) + 5, 21, 63, 200, false, 3);

	fillRyan();
	data.byte(kCommandtype) = 255;
	readMouse();
	showPointer();
	workToScreenCPP();
	delPointer();
	data.byte(kInvopen) = 2;
}

void DreamGenContext::notHeldError() {
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

void DreamGenContext::useCashCard() {
	getRidOfReels();
	loadKeypad();
	createPanel();
	showPanel();
	showExit();
	showMan();
	uint16 y = (!data.byte(kForeignrelease)) ? 120 : 120 - 3;
	showFrame(tempGraphics(), 114, y, 39, 0);
	const uint8 *obText = getObTextStartCPP();
	findNextColon(&obText);
	findNextColon(&obText);
	y = 98;
	printDirect(&obText, 36, &y, 36, 36 & 1);
	char amountStr[10];
	sprintf(amountStr, "%04d", data.word(kCard1money) / 10);
	data.word(kCharshift) = 91 * 2 + 75;
	printDirect((const uint8 *)amountStr, 160, 155, 240, 240 & 1);
	sprintf(amountStr, "%02d", (data.word(kCard1money) % 10) * 10);
	data.word(kCharshift) = 91 * 2 + 85;
	printDirect((const uint8 *)amountStr, 187, 155, 240, 240 & 1);
	data.word(kCharshift) = 0;
	workToScreenM();
	hangOnP(400);
	getRidOfTemp();
	restoreReels();
	putBackObStuff();
}

void DreamGenContext::useStereo() {
	if (data.byte(kLocation) != 0) {
		showPuzText(4, 400);
		putBackObStuff();
	} else if (data.byte(kMapx) != 11) {
		showPuzText(5, 400);
		putBackObStuff();
	} else if (checkInside(findSetObject("CDPL"), 1) == kNumexobjects) {
		// No CD inside
		showPuzText(6, 400);
		putBackObStuff();
		// TODO: Use the C++ version of getAnyAd()
		getAnyAd();
		es.byte(bx + 10) = 255;
	} else {
		// CD inside
		getAnyAd();
		es.byte(bx + 10) ^= 1;
		if (es.byte(bx + 10) != 255) {
			// Stereo off
			showPuzText(7, 400);
		} else {
			// Stereo on
			showPuzText(8, 400);
		}

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
	uint16 offset = kTextstart + getSegment(data.word(kPuzzletext)).word(command * 2);
	const uint8 *string = getSegment(data.word(kPuzzletext)).ptr(offset, 0);
	printDirect(string, 36, 104, 241, 241 & 1);
	workToScreenM();
	hangOnP(count);
}

} // End of namespace DreamGen
