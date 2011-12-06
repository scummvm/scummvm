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
	uint8 *obText = getObTextStartCPP();
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

void DreamGenContext::useText() {
	useText(es.ptr(si, 0));
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
	uint8 *obText = getObTextStartCPP();
	findNextColon(&obText);
	findNextColon(&obText);
	useText(obText);
	hangOnP(400);
}

void DreamGenContext::showSecondUse() {
	uint8 *obText = getObTextStartCPP();
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
		data.byte(kProgresspoints) = data.byte(kProgresspoints) + 1;
		removeObFromInv();
	}
}

void DreamGenContext::useCoveredBox() {
	data.byte(kProgresspoints) = data.byte(kProgresspoints) + 1;
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
		getAnyAd();
		makeWorn();
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
		getAnyAd();
		makeWorn();
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
	if (data.byte(kWatchmode) == 0xFF) {
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
	data.byte(kProgresspoints) = data.byte(kProgresspoints) + 1;
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
		data.byte(kProgresspoints) = data.byte(kProgresspoints) + 1;
		showFirstUse();
		data.byte(kNewlocation) = 29;
		data.byte(kGetback) = 1;
	}
}

void DreamGenContext::trapDoor() {
	data.byte(kProgresspoints) = data.byte(kProgresspoints) + 1;
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

} /*namespace dreamgen */

