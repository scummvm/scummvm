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

#include "common/str.h"

#include "mohawk/riven.h"
#include "mohawk/riven_stack.h"
#include "mohawk/riven_graphics.h"

namespace Mohawk {

// The Riven variable system is complex. The scripts of each stack give a number, but the number has to be matched
// to a variable name defined in NAME resource 4.

static const char *variableNames[] = {
	// aspit
	"aAtrusBook",
	"aAtrusPage",
	"aCathBook",
	"aCathPage",
	"aCathState",
	"aDoIt",
	"aDomeCombo",
	"aGehn",
	"aInventory",
	"aOva",
	"aPower",
	"aRaw",
	"aTemp",
	"aTrap",
	"aTrapBook",
	"aUserVolume",
	"aZip",

	// bspit
	"bBackLock",
	"bBait",
	"bBigBridge",
	"bBirds",
	"bBlrArm",
	"bBlrDoor",
	"bBlrGrt",
	"bBlrSw",
	"bBlrValve",
	"bBlrWtr",
	"bBook",
	"bBrLever",
	"bCaveDoor",
	"bcombo",
	"bCPipeGr",
	"bCraterGg",
	"bDome",
	"bDrwr",
	"bFans",
	"bFMDoor",
	"bIdVlv",
	"bLab",
	"bLabBackDr",
	"bLabBook",
	"bLabEye",
	"bLabFrontDr",
	"bLabPage",
	"bLever",
	"bFrontLock",
	"bHeat",
	"bMagCar",
	"bPipDr",
	"bPrs",
	"bStove",
	"bTrap",
	"bValve",
	"bVise",
	"bYtram",
	"bYtramTime",
	"bYtrap",
	"bYtrapped",

	// gspit
	"gBook",
	"gCathTime",
	"gCathState",
	"gCombo",
	"gDome",
	"gEmagCar",
	"gImageCurr",
	"gimagemax",
	"gImageRot",
	"gLkBtns",
	"gLkBridge",
	"gLkElev",
	"gLView",
	"gLViewMPos",
	"gLViewPos",
	"gNmagRot",
	"gNmagCar",
	"gPinUp",
	"gPinPos",
	"gPinsMPos",
	"gRView",
	"gRViewMPos",
	"gRViewPos",
	"gScribe",
	"gScribeTime",
	"gSubElev",
	"gSubDr",
	"gUpMoov",
	"gWhark",
	"gWharkTime",

	// jspit
	"jWMagCar",
	"jBeetle",
	"jBeetlePool",
	"jBook",
	"jBridge1",
	"jBridge2",
	"jBridge3",
	"jBridge4",
	"jBridge5",
	"jCCB",
	"jCombo",
	"jCrg",
	"jDome",
	"jDrain",
	"jGallows",
	"jGate",
	"jGirl",
	"jIconCorrectOrder",
	"jIconOrder",
	"jIcons",
	"jLadder",
	"jLeftPos",
	"jPeek",
	"jPlayBeetle",
	"jPRebel",
	"jPrisonDr",
	"jPrisonSecDr",
	"jrBook",
	"jRightPos",
	"jSouthPathDr",
	"jSchoolDr",
	"jSub",
	"jSubDir",
	"jSubHatch",
	"jSubSw",
	"jSunners",
	"jSunnerTime",
	"jThroneDr",
	"jTunnelDr",
	"jTunnelLamps",
	"jVillagePeople",
	"jWarning",
	"jWharkPos",
	"jWharkRam",
	"jWMouth",
	"jWMagCar",
	"jYMagCar",

	// ospit
	"oambient",
	"oButton",
	"ocage",
	"oDeskBook",
	"oGehnPage",
	"oMusicPlayer",
	"oStandDrawer",
	"oStove",

	// pspit
	"pBook",
	"pCage",
	"pCathCheck",
	"pCathState",
	"pCathTime",
	"pCombo",
	"pCorrectOrder",
	"pdome",
	"pElevCombo",
	"pLeftPos",
	"pRightPos",
	"pTemp",
	"pWharkPos",

	// rspit
	"rRebel",
	"rRebelView",
	"rRichard",
	"rVillageTime",

	// tspit
	"tBars",
	"tBeetle",
	"tBlue",
	"tBook",
	"tBookValve",
	"tCage",
	"tCombo",
	"tCorrectOrder",
	"tCoverCombo",
	"tDL",
	"tDome",
	"tDomeElev",
	"tDomeElevBtn",
	"tGateBrHandle",
	"tGateBridge",
	"tGateState",
	"tGreen",
	"tGRIDoor",
	"tGRODoor",
	"tGRMDoor",
	"tGuard",
	"tImageDoor",
	"tMagCar",
	"tOrange",
	"tRed",
	"tSecDoor",
	"tSubBridge",
	"tTeleCover",
	"tTeleHandle",
	"tTelePin",
	"tTelescope",
	"tTeleValve",
	"tTemple",
	"tTempleDoor",
	"tTunnelDoor",
	"tViewer",
	"tViolet",
	"tWaBrValve",
	"tWaffle",
	"tYellow",

	// Miscellaneous
	"elevbtn1",
	"elevbtn2",
	"elevbtn3",
	"domeCheck",
	"transitionsEnabled",
	"transitionMode",
	"waterEnabled",
	"RivenAmbients",
	"stackVarsInitialized",
	"DoingSetupScreens",
	"all_book",
	"playerHasBook",
	"returnStackID",
	"returnCardID",
	"NewPos",
	"theMarble",
	"CurrentStackID",
	"CurrentCardID"
};

uint32 &MohawkEngine_Riven::getStackVar(uint32 index) {
	Common::String name = getStack()->getName(kVariableNames, index);

	if (!_vars.contains(name))
		error("Could not find variable '%s' (stack variable %d)", name.c_str(), index);

	return _vars[name];
}

void MohawkEngine_Riven::initVars() {
	// Most variables just start at 0, it's simpler to do this
	for (uint32 i = 0; i < ARRAYSIZE(variableNames); i++)
		_vars[variableNames[i]] = 0;

	// Initialize the rest of the variables to their proper state
	_vars["ttelescope"] = 5;
	_vars["tgatestate"] = 1;
	_vars["jbridge1"] = 1;
	_vars["jbridge4"] = 1;
	_vars["jgallows"] = 1;
	_vars["jiconcorrectorder"] = 12068577;
	_vars["jcrg"] = 1;
	_vars["jwharkpos"] = 1;
	_vars["bblrvalve"] = 1;
	_vars["bblrwtr"] = 1;
	_vars["bfans"] = 1;
	_vars["bytrap"] = 2;
	_vars["aatrusbook"] = 1;
	_vars["acathbook"] = 1;
	_vars["bheat"] = 1;
	_vars["blabpage"] = 1;
	_vars["bidvlv"] = 1;
	_vars["bvise"] = 1;
	_vars["waterenabled"] = 1;
	_vars["ogehnpage"] = 1;
	_vars["bblrsw"] = 1;
	_vars["ocage"] = 1;
	_vars["jbeetle"] = 1;
	_vars["tdl"] = 1;
	_vars["bmagcar"] = 1;
	_vars["gnmagcar"] = 1;
	_vars["gemagcar"] = 1;
	_vars["gimagecurr"] = 1;
	_vars["gimagemax"] = 1;
	_vars["gimagerot"] = 1;
	_vars["glkbridge"] = 1;
	_vars["grviewpos"] = 2;
	_vars["gpinpos"] = 1;
	_vars["grviewmpos"] = 1617;
	_vars["omusicplayer"] = 1;
	_vars["tdomeelev"] = 1;
	_vars["transitionmode"] = kRivenTransitionModeFastest;

	// Randomize the telescope combination
	uint32 &teleCombo = _vars["tcorrectorder"];
	for (byte i = 0; i < 5; i++) {
		teleCombo *= 10;
		teleCombo += _rnd->getRandomNumberRng(1, 5); // 5 buttons
	}

	// Randomize the prison combination
	uint32 &prisonCombo = _vars["pcorrectorder"];
	for (byte i = 0; i < 5; i++) {
		prisonCombo *= 10;
		prisonCombo += _rnd->getRandomNumberRng(1, 3); // 3 buttons/sounds
	}

	// Randomize the dome combination -- each bit represents a slider position,
	// the highest bit (1 << 24) represents 1, (1 << 23) represents 2, etc.
	uint32 &domeCombo = _vars["adomecombo"];
	for (byte bitsSet = 0; bitsSet < 5;) {
		uint32 randomBit = 1 << (24 - _rnd->getRandomNumber(24));

		// Don't overwrite a bit we already set, and throw out the bottom five bits being set
		if (domeCombo & randomBit || (domeCombo | randomBit) == 31)
			continue;

		domeCombo |= randomBit;
		bitsSet++;
	}
}

} // End of namespace Mohawk
