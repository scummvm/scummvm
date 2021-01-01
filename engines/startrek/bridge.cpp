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

#include "startrek/iwfile.h"
#include "startrek/resource.h"
#include "startrek/room.h"
#include "startrek/startrek.h"

namespace StarTrek {

#define ACTOR_STANDING_SPOCK "xstndsp"
#define ACTOR_STANDING_MCCOY "xstndmc"
#define ACTOR_SITTING_SPOCK "bstndsp"

struct BridgeActorAndMenu {
	int id;
	const char *anim;
	const char *menu;
	int16 x;
	int16 y;
};

BridgeActorAndMenu bridgeActorsAndMenus[] = {
	{ 0, "bstndki", "command",  60,  100 },	// Kirk
	{ 1, "bstndsp", "comp",    200,   30 },	// Spock
	{ 4, "bstndsu", "helm",     60,   70 },	// Sulu
	{ 5, "bstndch", "nav",      60,   70 },	// Chekov
	{ 6, "bstnduh", "comm",     30,   30 },	// Uhura
	{ 7, "bstndsc", "eng",      30,   30 },	// Scotty
};

enum BridgeMenuEvent {
	kBridgeNone = -1,
	kBridgeKirkCommand = 0,
	kBridgeUnk1 = 1,
	kBridgeUnk2 = 2,
	kBridgeStarfieldFullScreen = 3,
	kBridgeKirkCaptainsLog = 16,
	kBridgeKirkTransporter = 17,
	kBridgeKirkOptions = 18,
	kBridgeSpock = 32,
	kBridgeSpockComputer = 33,
	kBridgeScottyDamageControl = 48,
	kBridgeScottyEmergencyPower = 49,
	kBridgeUhura = 64,
	kBridgeSuluOrbit = 80,
	kBridgeSuluShields = 81,
	kBridgeChekovNavigation = 96,
	kBridgeChekovWeapons = 97,
	kBridgeChekovRepairShields = 112,
	kBridgeChekovRepairPhasers = 113,
	kBridgeChekovRepairPhotonTorpedoes = 114,
	kBridgeChekovRepairSensors = 115,
	kBridgeChekovRepairBridge = 116,
	kBridgeChekovRepairHull = 117,
	kBridgeChekovRepairWarpDrives = 118,
	kBridgeSuluTargetAnalysis = 119
};

enum BridgeSequence {
	kSeqNone = -1,
	// -- Chapter 1 ----
	kSeqStartMissionDemon = 0,
	kSeqEndMockBattle = 1,
	kSeqShowDebriefDemon = 2,
	kSeqArrivedAtPolluxV = 3,
	kSeqEndMissionDemon = 4,
	// -- Chapter 2 ----
	kSeqStartMissionTug = 5,
	kSeqStartElasiPirateBattle = 6,
	kSeqArrivedAtBetaMyamid = 7,
	kSeqApproachedTheMasada = 8,
	kSeqEndMissionTug = 9,
	// -- Chapter 3 ----
	kSeqStartMissionLove = 10,
	kSeqAfterLoveDebrief = 11,
	kSeqStartRomulanBattle = 12,
	kSeqAfterRomulanBattle = 13,
	kSeqArrivedAtArk7 = 14,
	kSeqEndMissionLove = 15,
	// -- Chapter 4 ----
	kSeqStartMissionMudd = 16,
	kSeqFightElasiShipsAndContactMudd = 17,
	kSeqEndMissionMudd = 18,
	// -- Chapter 5 ----
	kSeqStartMissionFeather = 19,
	kSeqUnk20 = 20,
	kSeqUnk21 = 21,
	kSeqStartMissionTrial = 22,
	kSeqUnk23 = 23,
	kSeqUnk24 = 24,
	kSeqUnk25 = 25,
	kSeqUnk26 = 26,
	kSeqUnk27 = 27,
	// -- Chapter 6 ----
	kSeqStartMissionVeng = 28,
	kSeqUnk29 = 29,
	// -- Chapter 7 ----
	kSeqStartMissionSins = 30,
	kSeqUnk31 = 31
};

enum Planet {
	kPlanetNone = -1,
	kPlanetCenturius = 0,
	kPlanetCameronsStar = 1,
	kPlanetArk7 = 2,      // Chapter 3: Love's Labor Jeopardized (love)
	kPlanetHarlequin = 3, // Chapter 4: Another Fine Mess (mudd)
	kPlanetHarrapa = 4,
	kPlanetElasiPrime = 5,
	kPlanetDigifal = 6, // Chapter 5A: The Feathered Serpent (feather)
	kPlanetStrahkeer = 7,
	kPlanetHrakkour = 8, // Chapter 5B: The Feathered Serpent (trial)
	kPlanetTriRhoNautica = 9,
	kPlanetShivaOmicron = 10,
	kPlanetAlphaProxima = 11, // Chapter 6: The Old Devil Moon (sins)
	kPlanetOmegaMaelstrom = 12,
	kPlanetArgosIV = 13,
	kPlanetBetaMyamid = 14, // Chapter 2: Hijacked (tug)
	kPlanetSirius = 15,
	kPlanetSigmaZhukova = 16,
	kPlanetCastor = 17,
	kPlanetPollux = 18, // Chapter 1: Demon world (demon)
	kPlanetChristgen = 19
};

enum BridgeTalkers {
	kBridgeTalkerNone = -1,
	kBridgeTalkerKirk = 0,
	kBridgeTalkerSpock,
	kBridgeTalkerSulu,
	kBridgeTalkerChekov,
	kBridgeTalkerUhura,
	kBridgeTalkerScotty,
	kBridgeTalkerMcCoy,
	kBridgeTalkerCaptainsLog,
	kBridgeTalkerElasiCaptain,
	kBridgeTalkerElasiCereth,
	kBridgeTalkerAutobeacon,
	kBridgeTalkerVoice,
	kBridgeTalkerHarryMudd
};

enum RandomEncounterType {
	kRandomEncounterNone = 0,
	kRandomEncounterKlingon = 1,
	kRandomEncounterRomulan = 2,
	kRandomEncounterElasi = 3
};

void StarTrekEngine::initBridge(bool b) {
	_gfx->loadPalette("bridge");
	_sound->loadMusicFile("bridge");

	initStarfieldPosition();
	// TODO: starfield

	loadBridge();
}

void StarTrekEngine::loadBridge() {
	// TODO: Check why the +1 is needed here
	initStarfield(72 + 1, 30, 247 + 1, 102, 0);

	_gfx->setBackgroundImage("bridge");
	_gfx->loadPri("bridge");
	_gfx->copyBackgroundScreen();
	_system->updateScreen();

	loadBridgeActors();

	//sub_1312C();	// TODO

	// TODO
	//initStarfieldSprite();
	//initStarfieldSprite();
	//initStarfieldSprite();
	//initStarfieldSprite();
	//initStarfieldSprite();
}

void StarTrekEngine::loadBridgeActors() {
	for (int i = 0; i < ARRAYSIZE(bridgeActorsAndMenus); ++i) {
		BridgeActorAndMenu a = bridgeActorsAndMenus[i];
		loadActorAnim(a.id, a.anim, 0, 0, 1.0);
	}
}

// TODO: 2 params, change Enterprise state
void StarTrekEngine::setBridgeMouseCursor() {
	_gfx->setMouseBitmap("pushbtn" /*_mouseControllingShip ? "entcur" : "cursor"*/);
}

void StarTrekEngine::showTextboxBridge(int talker, int textId) {
	showTextboxBridge(talker, _resource->getLoadedText(textId));
}

void StarTrekEngine::showTextboxBridge(int talker, Common::String text) {
	switch (talker) {
	case kBridgeTalkerKirk:
		showTextbox("Captain Kirk", text, 160, 130, 176, 0);
		break;
	case kBridgeTalkerSpock:
		showTextbox("Mr. Spock", text, 294, 106, 44, 0);
		break;
	case kBridgeTalkerSulu:
		showTextbox("Mr. Sulu", text, 122, 116, 176, 0);
		break;
	case kBridgeTalkerChekov:
		showTextbox("Mr. Chekov", text, 196, 116, 176, 0);
		break;
	case kBridgeTalkerUhura:
		showTextbox("Lieutenant Uhura", text, 298, 150, 161, 0);
		break;
	case kBridgeTalkerScotty:
		showTextbox("Mr. Scott", text, 64, 100, 161, 0);
		break;
	case kBridgeTalkerMcCoy:
		showTextbox("Dr. McCoy", text, 160, 100, 44, 0);
		break;
	case kBridgeTalkerCaptainsLog:
		showTextbox("Captain's Log", text, 160, 130, 176, 0);
		break;
	case kBridgeTalkerElasiCaptain:
		showTextbox("Elasi Captain", text, 160, 190, 44, 0);
		break;
	case kBridgeTalkerElasiCereth:
		showTextbox("Elasi Cereth", text, 160, 190, 44, 0);
		break;
	case kBridgeTalkerAutobeacon:
		showTextbox("Autobeacon", text, 160, 130, 161, 0);
		break;
	case kBridgeTalkerVoice:
		showTextbox("Voice", text, 160, 130, 44, 0);
		break;
	case kBridgeTalkerHarryMudd:
		showTextbox("Harry Mudd", text, 160, 190, 44, 0);
		break;
	}
}

void StarTrekEngine::playBridgeSequence(int sequenceId) {
	const char *lowerShieldsText = "#BRID\\B_336#Lowering shields and disarming weapons.";

	switch (sequenceId) {
	case kSeqStartMissionDemon: // Chapter 1: Demon world (demon)
		_targetPlanet = kPlanetPollux;	// We set it earlier for uniformity
		_missionName = _missionToLoad = "DEMON";
		_resource->setTxtFileName(_missionName);
		_sound->loadMusicFile("bridgew");
		showMissionStartEnterpriseFlyby("DEM0\\FLYBY", "demon");
		showTextboxBridge(kBridgeTalkerCaptainsLog, 0);
		showTextboxBridge(kBridgeTalkerUhura, 1);
		showTextboxBridge(kBridgeTalkerSpock, 2);
		// TODO: sub_2FF19("enterpri") // random number generation
		// TODO: changeBridgeMode 1
		_sound->playMidiMusicTracks(2, -1);
		_sound->playSoundEffectIndex(kSfxRedAlert);

		// HACK: Play the end of the mock battle sequence, until the 3D code is implemented
		_bridgeSequenceToLoad = kSeqEndMockBattle;
		break;
	case kSeqEndMockBattle:
		if (true) {	// TODO: Check for Enterprise damage
			// Mock battle won
			_sound->playMidiMusicTracks(3, -1);
			showTextboxBridge(kBridgeTalkerSpock, 4);
			showTextboxBridge(kBridgeTalkerUhura, 5);
			// TODO: sub_2f4c3()
			// TODO: sub_321f9()
			showTextboxBridge(kBridgeTalkerSulu, lowerShieldsText);
			// TODO: Check variable
		} else {
			// Mock battle lost
			// TODO
		}

		_bridgeSequenceToLoad = kSeqShowDebriefDemon;
		break;
	case kSeqShowDebriefDemon:
		setBridgeMouseCursor();	// 0, 0
		// TODO: Delete 3D object
		showBridgeScreenTalkerWithMessage(12, "Admiral", "woman");
		break;
	case kSeqArrivedAtPolluxV:
		showTextboxBridge(kBridgeTalkerSpock, 15); // We have arrived at Pollux V
		_hailedTarget = false;
		// In the original, this is actually a handler for Spock, Uhura and Chekov.
		// We moved their actions in separate functions instead.
		break;
	case kSeqEndMissionDemon:
		_resource->setTxtFileName("DEMON");
		loadActorAnim(1, ACTOR_STANDING_SPOCK, 0, 0, 1.0);
		loadActorAnim(2, ACTOR_STANDING_MCCOY, 0, 0, 1.0);
		_sound->playSoundEffectIndex(kSfxHailing);
		showTextboxBridge(kBridgeTalkerUhura, 23); // Message from Starfleet
		showTextboxBridge(kBridgeTalkerKirk, 24);
		showMissionPerformance(_awayMission.demon.missionScore * 100 / 32, 29, 0);
		showTextboxBridge(kBridgeTalkerMcCoy, 25);
		showTextboxBridge(kBridgeTalkerKirk, 26);
		showTextboxBridge(kBridgeTalkerSpock, 27);
		showTextboxBridge(kBridgeTalkerMcCoy, 28);
		loadActorAnim(1, ACTOR_SITTING_SPOCK, 0, 0, 1.0);
		removeActorFromScreen(2);
		_bridgeSequenceToLoad = kSeqStartMissionTug;
		break;
	case kSeqStartMissionTug: // Chapter 2: Hijacked (tug)
		_targetPlanet = kPlanetBetaMyamid;
		_missionName = _missionToLoad = "TUG";
		_resource->setTxtFileName(_missionName);
		_sound->loadMusicFile("bridge");
		showMissionStartEnterpriseFlyby("TUG0\\FLYBY", "hijacked");
		_sound->playMidiMusicTracks(0, -1);
		_sound->playSoundEffectIndex(kSfxHailing);
		showTextboxBridge(kBridgeTalkerUhura, 0);
		showTextboxBridge(kBridgeTalkerKirk, 1);
		showBridgeScreenTalkerWithMessage(2, "Admiral", "woman");
		showTextboxBridge(kBridgeTalkerChekov, 3);
		break;
	case kSeqStartElasiPirateBattle:
		showTextboxBridge(kBridgeTalkerSpock, 6);
		_sound->playSoundEffectIndex(kSfxHailing);
		showTextboxBridge(kBridgeTalkerUhura, 7);
		showTextboxBridge(kBridgeTalkerKirk, 8);
		showBridgeScreenTalkerWithMessage(9, "Elasi Captain", "pira", false);
		showTextboxBridge(kBridgeTalkerKirk, 10);
		showTextboxBridge(kBridgeTalkerElasiCaptain, 11);
		removeActorFromScreen(_currentScreenTalker);
		initStarfieldSprite(&_starfieldSprite, new StubBitmap(0, 0), _starfieldRect);
		_sound->playSoundEffectIndex(kSfxRedAlert);
		_enterpriseState.underAttack = true;
		_sound->playMidiMusicTracks(2, -1);
		startBattle("orion"); // Elasi Pirate battle
		_enterpriseState.underAttack = false;
		_sound->playMidiMusicTracks(3, -1);
		showTextboxBridge(kBridgeTalkerChekov, 14);	// Captain they are fleeing!
		showTextboxBridge(kBridgeTalkerSpock, 15);	// The enemy ship's initial intercept course...
		_bridgeSequenceToLoad = kSeqArrivedAtBetaMyamid;
		break;
	case kSeqArrivedAtBetaMyamid:
		// In the original, this is actually a handler for Spock, Uhura and Chekov.
		// We moved their actions in separate functions instead.
		break;
	case kSeqApproachedTheMasada:
		// TODO: approach the Masada 3D animation
		showTextboxBridge(kBridgeTalkerSpock, 20); // There she is, Captain. She does not appear to be seriously damaged
		_hailedTarget = false;
		_beamDownAllowed = false;
		break;
	case kSeqEndMissionTug:
		_resource->setTxtFileName("TUG");
		loadActorAnim(1, ACTOR_STANDING_SPOCK, 0, 0, 1.0); // Standing Spock
		loadActorAnim(2, ACTOR_STANDING_MCCOY, 0, 0, 1.0); // Standing McCoy
		if (_missionEndFlag == 0) {
			// Good ending
			_sound->playSoundEffectIndex(kSfxHailing);
			showTextboxBridge(kBridgeTalkerUhura, 29);
			showTextboxBridge(kBridgeTalkerKirk, 30);
			showMissionPerformance(_awayMission.tug.missionScore * 100 / 32, 31, 1);
			showTextboxBridge(kBridgeTalkerMcCoy, 32); // Trying to hold a Federation starship captive. Can you believe it?
			showTextboxBridge(kBridgeTalkerSpock, 33); // Since we just witnessed that very event, Doctor, I'm surprised you ask.
			showTextboxBridge(kBridgeTalkerMcCoy, 34);
			showTextboxBridge(kBridgeTalkerKirk, 35);
			showTextboxBridge(kBridgeTalkerSpock, 36);
			showTextboxBridge(kBridgeTalkerKirk, 37);
		} else if (_missionEndFlag == 1) {
			// Bad ending 1 (0 score): the Masada got destroyed, everyone died
			_sound->playSoundEffectIndex(kSfxHailing);
			showTextboxBridge(kBridgeTalkerUhura, 39);
			showTextboxBridge(kBridgeTalkerKirk, 40);
			showMissionPerformance(0, 41, 1);
		} else if (_missionEndFlag == 2) {
			// Bad ending 2 (0 score): bad handling of the situation, some prisoners died
			_sound->playSoundEffectIndex(kSfxHailing);
			showTextboxBridge(kBridgeTalkerUhura, 42);
			showMissionPerformance(0, 43, 1);
		}
		loadActorAnim(1, ACTOR_SITTING_SPOCK, 0, 0, 1.0);
		removeActorFromScreen(2);
		_bridgeSequenceToLoad = kSeqStartMissionLove;
		break;
	case kSeqStartMissionLove:	// Chapter 3: Love's Labor Jeopardized (love)
		_targetPlanet = kPlanetArk7;	// We set it earlier for uniformity
		_missionName = _missionToLoad = "LOVE";
		_resource->setTxtFileName(_missionName);
		_sound->loadMusicFile("bridge");
		showMissionStartEnterpriseFlyby("LOV0\\FLYBY", "tlove");
		_sound->playSoundEffectIndex(kSfxHailing);
		showTextboxBridge(kBridgeTalkerUhura, 0);
		showTextboxBridge(kBridgeTalkerKirk, 1);
		showBridgeScreenTalkerWithMessage(2, "Admiral", "woman");
		_mouseControllingShip = true;
		break;
	case kSeqAfterLoveDebrief:
		// In the original, this is actually a handler for Spock, Uhura and Chekov.
		// We moved their actions in separate functions instead.
		break;
	case kSeqStartRomulanBattle:
		showTextboxBridge(kBridgeTalkerSpock, "#BRID\\B_160#Romulan ship decloaking, Captain.");
		_sound->playSoundEffectIndex(kSfxHailing);
		showTextboxBridge(kBridgeTalkerUhura, 6);	// Message from the Romulan ship, Sir
		showBridgeScreenTalkerWithMessage(9, "Romulan Captain", "romula");
		showTextboxBridge(kBridgeTalkerSpock, 10);
		_sound->playMidiMusicTracks(2, -1);
		_enterpriseState.underAttack = true;
		// HACK: Play the end of the Romulan battle sequence, until the 3D code is implemented
		_bridgeSequenceToLoad = kSeqAfterRomulanBattle;
		break;
	case kSeqAfterRomulanBattle:
		_enterpriseState.underAttack = false;
		showTextboxBridge(kBridgeTalkerSpock, 11);	// They activated a self-destruct device...
		showTextboxBridge(kBridgeTalkerSulu, 12);	// Resuming course to ARK7, Captain
		orbitPlanetSequence(4);
		_bridgeSequenceToLoad = kSeqArrivedAtArk7;
		break;
	case kSeqArrivedAtArk7:
		_hailedTarget = false;
		_sound->playSoundEffectIndex(kSfxHailing);
		showTextboxBridge(kBridgeTalkerUhura, 17);
		showTextboxBridge(kBridgeTalkerAutobeacon, 16);
		break;
	case kSeqEndMissionLove:
		_resource->setTxtFileName("LOVE");
		loadActorAnim(1, ACTOR_STANDING_SPOCK, 0, 0, 1.0); // Standing Spock
		loadActorAnim(2, ACTOR_STANDING_MCCOY, 0, 0, 1.0); // Standing McCoy
		_sound->playSoundEffectIndex(kSfxHailing);
		showTextboxBridge(kBridgeTalkerUhura, 18);
		showTextboxBridge(kBridgeTalkerKirk, 19);
		showMissionPerformance(_awayMission.love.missionScore * 100 / 32, 26, 2);
		showTextboxBridge(kBridgeTalkerKirk, 20);
		showTextboxBridge(kBridgeTalkerMcCoy, 21);
		showTextboxBridge(kBridgeTalkerKirk, 22);
		showTextboxBridge(kBridgeTalkerMcCoy, 23);
		showTextboxBridge(kBridgeTalkerScotty, 24);
		showTextboxBridge(kBridgeTalkerKirk, 25);
		loadActorAnim(1, ACTOR_SITTING_SPOCK, 0, 0, 1.0);
		removeActorFromScreen(2);
		_bridgeSequenceToLoad = kSeqStartMissionMudd;
		break;
	case kSeqStartMissionMudd: // Chapter 4: Another Fine Mess (mudd)
		_targetPlanet = kPlanetHarlequin;	// We set it earlier for uniformity
		_missionName = _missionToLoad = "MUDD";
		_resource->setTxtFileName(_missionName);
		_sound->loadMusicFile("bridge");
		// TODO: sub_321F9()
		showMissionStartEnterpriseFlyby("MUD0\\FLYBY", "another");
		_sound->playSoundEffectIndex(kSfxHailing);
		showTextboxBridge(kBridgeTalkerUhura, 0);
		showTextboxBridge(kBridgeTalkerKirk, 1);
		showBridgeScreenTalkerWithMessage(2, "Admiral", "woman");
		// TODO: changeBridgeMode 1
		// Spock's planet arrival quote has been moved to kSeqFightElasiShipsAndContactMudd
		break;
	case kSeqFightElasiShipsAndContactMudd:
		showTextboxBridge(kBridgeTalkerSpock, 5);	// We have arrived at Harlequin
		//_beamDownAllowed = false;	// TODO: unused, since the player jumps straight into a battle?
		showTextboxBridge(kBridgeTalkerSpock, 6);
		_sound->playSoundEffectIndex(kSfxHailing);
		showTextboxBridge(kBridgeTalkerUhura, 7);
		showBridgeScreenTalkerWithMessage(14, "Elasi Pirate", "pira");
		_enterpriseState.underAttack = true;
		_sound->playMidiMusicTracks(2, -1);
		startBattle("orion");	// Pirate Ship 1
		startBattle("orion");	// Pirate Ship 2
		_enterpriseState.underAttack = false;
		showTextboxBridge(kBridgeTalkerSpock, 8);	// The ion trail of the scout ship leads to the Harrapan system, Captain
		showTextboxBridge(kBridgeTalkerUhura, 9);
		showTextboxBridge(kBridgeTalkerKirk, 10);
		showTextboxBridge(kBridgeTalkerVoice, 11);
		showTextboxBridge(kBridgeTalkerKirk, 12);
		showTextboxBridge(kBridgeTalkerUhura, 13);
		showBridgeScreenTalkerWithMessage(15, "Harry Mudd", "mudd", false);
		showTextboxBridge(kBridgeTalkerKirk, 16);
		showTextboxBridge(kBridgeTalkerHarryMudd, 17);
		removeActorFromScreen(_currentScreenTalker);
		initStarfieldSprite(&_starfieldSprite, new StubBitmap(0, 0), _starfieldRect);
		showTextboxBridge(kBridgeTalkerSpock, 18);
		break;
	case kSeqEndMissionMudd:
		_resource->setTxtFileName("MUDD");
		loadActorAnim(1, ACTOR_STANDING_SPOCK, 0, 0, 1.0); // Standing Spock
		loadActorAnim(2, ACTOR_STANDING_MCCOY, 0, 0, 1.0); // Standing McCoy
		showMissionPerformance(_awayMission.mudd.missionScore * 100 / 32, 35, 3);
		if (_missionEndFlag != 0) {
			// Obtained alien contraption
			showTextboxBridge(kBridgeTalkerScotty, 36);
			showTextboxBridge(kBridgeTalkerKirk, 37);
		} else {
			// Did not obtain alien contraption
			showTextboxBridge(kBridgeTalkerKirk, 38);
		}
		showTextboxBridge(kBridgeTalkerUhura, 41);
		showTextboxBridge(kBridgeTalkerKirk, 42);
		showTextboxBridge(kBridgeTalkerUhura, 43);
		showTextboxBridge(kBridgeTalkerKirk, 44);
		loadActorAnim(1, ACTOR_SITTING_SPOCK, 0, 0, 1.0);
		removeActorFromScreen(2);
		_bridgeSequenceToLoad = kSeqStartMissionFeather;
		break;
	case kSeqStartMissionFeather:
		_targetPlanet = kPlanetDigifal; // We set it earlier for uniformity
		_missionName = _missionToLoad = "FEATHER";
		_resource->setTxtFileName(_missionName);
		_sound->loadMusicFile("bridgeb");
		// TODO: sub_321F9()
		showMissionStartEnterpriseFlyby("FEA0\\FLYBY", "feather");
		showTextboxBridge(kBridgeTalkerKirk, 5);	// Captain's log, Stardate 5097.3. Starfleet reports major military activity...
		// TODO
		break;
	case kSeqStartMissionTrial:
		_currentPlanet = _targetPlanet = kPlanetHrakkour;
		_missionName = _missionToLoad = "TRIAL";
		_resource->setTxtFileName(_missionName);
		orbitPlanet();
		showTextboxBridge(kBridgeTalkerKirk, 0);	// Captain's Log. We have come to the ruined Klingon planet of Hrakkour...
		// TODO
		break;
	case kSeqStartMissionSins:
		_targetPlanet = kPlanetAlphaProxima; // We set it earlier for uniformity
		_missionName = _missionToLoad = "SINS";
		_resource->setTxtFileName(_missionName);
		_sound->loadMusicFile("bridge");
		showMissionStartEnterpriseFlyby("SIN0\\FLYBY", "devilmon");
		_sound->playSoundEffectIndex(kSfxHailing);
		showTextboxBridge(kBridgeTalkerUhura, 0);
		showTextboxBridge(kBridgeTalkerKirk, 1);
		showBridgeScreenTalkerWithMessage(2, "Admiral", "woman");
		showTextboxBridge(kBridgeTalkerSpock, 3);
		showTextboxBridge(kBridgeTalkerKirk, 4);
		showTextboxBridge(kBridgeTalkerSpock, 5);
		showTextboxBridge(kBridgeTalkerChekov, 6);
		showTextboxBridge(kBridgeTalkerSpock, 7);
		showTextboxBridge(kBridgeTalkerKirk, 8);
		showTextboxBridge(kBridgeTalkerSpock, 9);
		// TODO
		break;
	case kSeqStartMissionVeng:
		// TODO
		break;
	// TODO: The rest
	default:
		break;
	}
}

struct CrewTextsForChapter {
	int targetPlanet;
	int talker;
	int underAttackTextId;
	int notReachedPlanetTextId;
	int notInOrbitTextId;
	int inOrbitTextId;
};

CrewTextsForChapter crewTexts[] = {
    // -- Chapter 1 ----
	{ kPlanetPollux,       kBridgeTalkerSpock, 10, 20, 21, 22 },
	{ kPlanetPollux,       kBridgeTalkerUhura,  9, 16, 19, 19 },
    // -- Chapter 2 ----
	{ kPlanetBetaMyamid,   kBridgeTalkerSpock, 13,  5, 17, 27 },
	{ kPlanetBetaMyamid,   kBridgeTalkerUhura, 12,  4, 16, -1 },
    // -- Chapter 3 ----
	{ kPlanetArk7,         kBridgeTalkerSpock,  8,  4, -1, 13 },
	{ kPlanetArk7,         kBridgeTalkerUhura,  7,  5, -1, -1 },
    // -- Chapter 4 ----
	{ kPlanetHarlequin,    kBridgeTalkerSpock, 21,  4, 22, -1 },
	{ kPlanetHarlequin,    kBridgeTalkerUhura, 19,  3, 20, -1 },
    // -- Chapter 6 ----
	{ kPlanetAlphaProxima, kBridgeTalkerSpock, -1, 11, -1, -1 },
	{ kPlanetAlphaProxima, kBridgeTalkerUhura, -1, 10, -1, -1 },
	// TODO: The rest
	{ kPlanetNone,       kBridgeTalkerNone,   0,  0,  0,  0 }
};

void StarTrekEngine::bridgeCrewAction(int crewId) {
	CrewTextsForChapter *curCrewTexts = crewTexts;
	int textId = -1;
	const char *underAttackText = "#BRID\\B_348#Unable to comply, Captain.  We're under attack.";
	const char *missionNotOverText = "#BRID\\B_338#May I respectfully remind the Captain that we haven't accomplished our mission, Sir.";
	const char *leaveOrbitText = "#BRID\\B_349#We must first leave orbit, Sir.";
	const char *nothingToReportText = "#BRID\\B_155 #Nothing to report, Captain.";
	const char *hailingFrequenciesOpenText = "#BRID\\BRIDU016#Hailing frequencies open.";
	const char *shieldsUpText = "#BRID\\B_332#Captain, the shields are up.";
	const char *notInOrbitText = "#BRID\\B_350#We're not in orbit, Captain.";
	const char *wrongDestinationText = "#BRID\\B_346#This isn't our destination.";
	const char *transporterText = "#BRID\\C_060#Spock, come with me. Mr Scott, you have the conn.";
	const char *transporterTextFeather = "#BRID\\C_006#Assemble a landing party. Unless we find this so-called criminal, we're going to war.";

	switch (crewId) {
	case kBridgeTalkerSpock:
	case kBridgeTalkerUhura:
		while (curCrewTexts->targetPlanet != kPlanetNone) {
			if (_targetPlanet == curCrewTexts->targetPlanet && curCrewTexts->talker == crewId) {
				if (_enterpriseState.underAttack) {
					textId = curCrewTexts->underAttackTextId;
				} else if (_currentPlanet != _targetPlanet) {
					textId = curCrewTexts->notReachedPlanetTextId;
				} else if (!_enterpriseState.inOrbit) {
					textId = curCrewTexts->notInOrbitTextId;
				} else {
					textId = curCrewTexts->inOrbitTextId;
				}
				break;
			}

			curCrewTexts++;
		}

		// Uhura's hailing sequences
		if (crewId == kBridgeTalkerUhura && _currentPlanet == _targetPlanet) {
			if (!_hailedTarget) {
				contactTargetAction();
				_hailedTarget = true;
				return;
			} else if (_currentPlanet == kPlanetBetaMyamid && _hailedTarget) {
				hailTheMasada();
				return;
			}
		}
		break;
	case kBridgeTalkerChekov:
		if (_enterpriseState.underAttack) {
			showTextboxBridge(kBridgeTalkerSulu, underAttackText);
		} else if (_currentPlanet == _targetPlanet) {
			showTextboxBridge(kBridgeTalkerSulu, missionNotOverText);
		} else if (_enterpriseState.inOrbit) {
			showTextboxBridge(kBridgeTalkerSulu, leaveOrbitText);
		} else {
			showStarMap();
			if (_currentPlanet != _targetPlanet) {
				wrongDestinationRandomEncounter();
				// TODO: Redraw sprites
			}
		}
		break;
	case kBridgeTalkerSulu:
		if (_enterpriseState.underAttack) {
			showTextboxBridge(kBridgeTalkerSulu, underAttackText);
		} else if (_currentPlanet == _targetPlanet && _enterpriseState.inOrbit) {
			showTextboxBridge(kBridgeTalkerSulu, missionNotOverText);
		} else {
			if (_targetPlanet == kPlanetBetaMyamid) {
				orbitPlanetSequence(6);
				_bridgeSequenceToLoad = kSeqApproachedTheMasada;
			} else if (_targetPlanet == kPlanetHarlequin) {
				orbitPlanetSequence(5);
			}

			orbitPlanet();
		}
		break;
	case kBridgeTalkerKirk:
		if (_enterpriseState.shields) {
			showTextboxBridge(kBridgeTalkerSulu, shieldsUpText);
		} else if (!_enterpriseState.inOrbit) {
			showTextboxBridge(kBridgeTalkerSulu, notInOrbitText);
		} else if (_currentPlanet != _targetPlanet) {
			showTextboxBridge(kBridgeTalkerSulu, wrongDestinationText);
		} else {
			if (_targetPlanet == kPlanetBetaMyamid && !_beamDownAllowed) {
				// Chapter 2, disallow beaming down until the correct code is set
				showTextboxBridge(kBridgeTalkerSulu, 28); // Captain, the Masada's shields are still up
			} else {
				if (_missionToLoad != "FEATHER")
					showTextboxBridge(kBridgeTalkerKirk, transporterText);
				else
					showTextboxBridge(kBridgeTalkerKirk, transporterTextFeather);
				_gameMode = GAMEMODE_BEAMDOWN;
			}
		}
		break;
	}

	if (textId >= 0) {
		showTextboxBridge(crewId, textId);
	} else if (crewId == kBridgeTalkerSpock) {
		showTextboxBridge(crewId, nothingToReportText);
	} else if (crewId == kBridgeTalkerUhura) {
		showTextboxBridge(crewId, hailingFrequenciesOpenText);
	}
}

void StarTrekEngine::contactTargetAction() {
	switch (_targetPlanet) {
	case kPlanetPollux:	// Chapter 1: Demon world (demon)
		_sound->playSoundEffectIndex(kSfxHailing);
		showTextboxBridge(kBridgeTalkerUhura, 17);
		showBridgeScreenTalkerWithMessage(18, "Priest", "prst");
		break;
	case kPlanetBetaMyamid: // Chapter 2: Hijacked (tug)
		if (!_hailedTarget) {
			showTextboxBridge(kBridgeTalkerUhura, 21);
			showBridgeScreenTalkerWithMessage(65, "Elasi Captain", "pira", false);
			negotiateWithElasiCereth();
			removeActorFromScreen(_currentScreenTalker);
			initStarfieldSprite(&_starfieldSprite, new StubBitmap(0, 0), _starfieldRect);
		} else {
			hailTheMasada();
		}	
		break;
	case kPlanetArk7:	// Chapter 3: Love's Labor Jeopardized (love)
		_sound->playSoundEffectIndex(kSfxHailing);
		showTextboxBridge(kBridgeTalkerUhura, 14);
		showBridgeScreenTalkerWithMessage(15, "Centurion Preax", "romula");
		break;
		// TODO: The rest
	}
}

void StarTrekEngine::negotiateWithElasiCereth() {
	// TODO: Read these from tug.txt
	const char *options1[] = {
	    "Captain Kirk",
		"#BRID\\C_074#This is Captain James T. Kirk of the U.S.S. Enterprise. You are illegally in possession of Starfleet property.",
		"#BRID\\C_072#This is Captain James T. Kirk of the U.S.S. Enterprise. Listen, Elasi, hand over the ship and hostages now or things are going to get very nasty.",
		"#BRID\\C_073#This is Captain James T. Kirk of the U.S.S. Enterprise. Mr. Elasi, it appears that you have found something that Starfleet lost.",
		""
	};

	const char *options2[] = {
	    "Captain Kirk",
		"#BRID\\C_026#Freeing the prisoners you requested will take time. We have to track down their current locations, and Starfleet will have to grant permission to move them here. I will get that started - you should tender a show of good faith on your part. Will you beam over so we can discuss negotiations face to face?",
		"#BRID\\C_091#We can work together on this, Elasi. How about beaming some of the hostages over as a show of good faith?",
		""
	};

	const char *options3[] = {
	    "Captain Kirk",
		"#BRID\\C_077#Very well, we'll do it your way. I will put together the data packet.  Kirk out.",
		"#BRID\\C_102#Your fellow Elasi have been tried in Federation courts of law, and found guilty of their crimes. We'll listen to your complaints if you discuss them reasonably.",
	    "#BRID\\C_014#Forget it. Elasi Cereth, you'll be the next one languishing on a Federation penal planet unless you drop your shields and surrender to me immediately.",
		""
	};

	int choice = showText(&StarTrekEngine::readTextFromArrayWithChoices, (uintptr)options1, 160, 130, 176, true, false, false);
	if (choice == 0) {
		showTextboxBridge(kBridgeTalkerElasiCereth, 44);	// Ah, the infamous captain Kirk...
		showTextboxBridge(kBridgeTalkerUhura, 45);
	} else if (choice == 1) {
		showTextboxBridge(kBridgeTalkerElasiCereth, 46);	// Well, Kirk, someone is about to pay the price for your insolence
		showTextboxBridge(kBridgeTalkerSpock, 60);
		_deadMasadaPrisoners++;
	} else if (choice == 2) {
		showTextboxBridge(kBridgeTalkerElasiCereth, 47);
		showTextboxBridge(kBridgeTalkerElasiCereth, 44); // Ah, the infamous captain Kirk...
		showTextboxBridge(kBridgeTalkerUhura, 45);
	}

	showText(&StarTrekEngine::readTextFromArrayWithChoices, (uintptr)options2, 160, 130, 176, true, false, false);
	// The choice here makes no difference
	showTextboxBridge(kBridgeTalkerElasiCereth, 51);
	showTextboxBridge(kBridgeTalkerScotty, 52);
	showTextboxBridge(kBridgeTalkerSulu, 53);
	showTextboxBridge(kBridgeTalkerElasiCereth, 54);

	choice = showText(&StarTrekEngine::readTextFromArrayWithChoices, (uintptr)options3, 160, 130, 176, true, false, false);
	if (choice == 0) {
		// End of dialog
	} else if (choice == 1) {
		showTextboxBridge(kBridgeTalkerElasiCereth, 69);
	} else if (choice == 2) {
		showTextboxBridge(kBridgeTalkerElasiCereth, 70);
		showTextboxBridge(kBridgeTalkerSpock, 60);
		_deadMasadaPrisoners++;
		showTextboxBridge(kBridgeTalkerElasiCereth, 71);
		// The original checks for 4 dead prisoners here, but that is impossible at this point
	}

	// There are some more checks for dialog choices here, but all seem to be dead code
}

void StarTrekEngine::hailTheMasada() {
	// TODO: Read these from tug.txt
	const char *options[] = {
		"Captain Kirk",
	    "Hail the Masada.",
	    "Send prefix code.",
	    ""
	};

	int choice = showText(&StarTrekEngine::readTextFromArrayWithChoices, (uintptr)options, 160, 130, 176, true, false, false);
	if (choice == 0) {
		showTextboxBridge(kBridgeTalkerUhura, 21);
		showBridgeScreenTalkerWithMessage(22, "Elasi Cereth", "pira", false);
		showTextboxBridge(kBridgeTalkerKirk, 23);
		removeActorFromScreen(_currentScreenTalker);
		initStarfieldSprite(&_starfieldSprite, new StubBitmap(0, 0), _starfieldRect);
		showTextboxBridge(kBridgeTalkerSpock, 60);
		_deadMasadaPrisoners++;
		if (_deadMasadaPrisoners > 4) {
			_missionEndFlag = 2;
			_bridgeSequenceToLoad = kSeqEndMissionTug;
		}
	} else {
		Common::String code = showCodeInputBox();
		if (code == "293391-197736-3829") {
			showTextboxBridge(kBridgeTalkerUhura, 25);
			_beamDownAllowed = true;
		} else {
			showTextboxBridge(kBridgeTalkerUhura, 26);
		}
	}
}

Common::String StarTrekEngine::getSpeechSampleForNumber(int number) {
	const char *speechTemplate = ",BRID\\B_%03d";
	Common::String result;

	if (number <= 19) {
		// Speech samples 228 ("zero") - 247 ("nineteen")
		result = Common::String::format(speechTemplate, number + 228);
	} else if (number >= 20 && number <= 99) {
		// Speech samples 248 ("twenty") - 255 ("ninety")
		result = Common::String::format(speechTemplate, number / 10 - 2 + 248);
		if (number % 10 > 0)
			result += Common::String::format(speechTemplate, number % 10 + 228);
	} else if (number == 100) {
		result = Common::String::format(speechTemplate, 256);
	}

	return result;
}

// TODO: one more parameter
void StarTrekEngine::showMissionPerformance(int score, int missionScoreTextId, int missionId) {
	Common::String performanceDescription;
	int midiTrack = 0;
	int commendationPoints = 0;

	if (score >= 0 && score <= 50) {
		performanceDescription = "#BRID\\B_199#I'll be frank, Kirk. Starfleet expects more of you than that. Try to do better on your next assignment.";
		commendationPoints = 0;
		midiTrack = 13;
	} else if (score >= 60 && score <= 70) {
		performanceDescription = "#BRID\\B_197#A satisfactory performance, Captain, but there's still room for improvement.";
		commendationPoints = 1;
		midiTrack = 13;
	} else if (score >= 71 && score <= 85) {
		performanceDescription = "#BRID\\B_214#Well done, Captain. Keep up the good work.";
		commendationPoints = 2;
		midiTrack = 11;
	} else if (score >= 86 && score <= 99) {
		performanceDescription = "#BRID\\B_414#The top brass at Starfleet are impressed. Outstanding work, Jim.";
		commendationPoints = 3;
		midiTrack = 12;
	} else if (score == 100) {
		performanceDescription = "#BRID\\B_195#A perfect mission, Jim! You are a model for all Starfleet!";
		commendationPoints = 4;
		midiTrack = 14;
	}

	_sound->playMidiMusicTracks(midiTrack, -1);

	Common::String speechIdPerformance = getSpeechSampleForNumber(score);
	Common::String speechIdCommendationPoints = getSpeechSampleForNumber(commendationPoints);

	Common::String missionPerformanceText = Common::String::format(
		_resource->getLoadedText(missionScoreTextId).c_str(),
		speechIdPerformance.c_str(),
		speechIdCommendationPoints.c_str(),
		score,
		commendationPoints
	);

	Common::String texts[] = {
		missionPerformanceText,
		performanceDescription,
		""
	};

	showBridgeScreenTalkerWithMessages(texts, "Admiral", "woman");

	_lastMissionId = missionId;
	_missionPoints[missionId] = commendationPoints;
}

void StarTrekEngine::showBridgeScreenTalkerWithMessage(int textId, Common::String talkerHeader, Common::String talkerId, bool removeTalker) {
	Common::String text = _resource->getLoadedText(textId);
	Common::String texts[] = {
		text,
		""
	};
	showBridgeScreenTalkerWithMessages(texts, talkerHeader, talkerId, removeTalker);
}

void StarTrekEngine::showBridgeScreenTalkerWithMessages(Common::String texts[], Common::String talkerHeader, Common::String talkerId, bool removeTalker) {
	if (talkerId == "romula" || talkerId == "pira" || talkerId == "klg1" || talkerId == "klg2" || talkerId == "maddoc")
		_sound->playMidiMusicTracks(15, -1);
	else if (talkerId == "mudd")
		_sound->playMidiMusicTracks(17, -1);

	initStarfieldSprite(&_starfieldSprite, new Bitmap(_resource->loadBitmapFile(talkerId)), _starfieldRect);
	_starfieldSprite.drawMode = 0;
	_currentScreenTalker = loadActorAnim(-1, talkerId, 72 + _starfieldSprite.bitmap->xoffset - 2, 30 + _starfieldSprite.bitmap->yoffset, 1.0);
	int i = 0;
	Common::String text = texts[i];
	while (text != "") {
		showTextbox(talkerHeader, text, 160, 190, 44, 0);
		text = texts[++i];
	}

	if (removeTalker) {
		removeActorFromScreen(_currentScreenTalker);
		initStarfieldSprite(&_starfieldSprite, new StubBitmap(0, 0), _starfieldRect);
	}
}

void StarTrekEngine::bridgeLeftClick() {
	Sprite *sprite = _gfx->getSpriteAt(_gfx->getMousePos());
	int clickedActor = -1;

	if (sprite == nullptr)
		return;

	for (int i = 0; i < NUM_ACTORS; i++) {
		Actor *actor = &_actorList[i];
		if (sprite == &actor->sprite) {
			clickedActor = i;
			break;
		}
	}

	if (clickedActor == -1)
		return;

	for (int i = 0; i < ARRAYSIZE(bridgeActorsAndMenus); ++i) {
		BridgeActorAndMenu a = bridgeActorsAndMenus[i];
		if (a.id == clickedActor) {
			showBridgeMenu(a.menu, a.x, a.y);
			break;
		}
	}
}

void StarTrekEngine::cleanupBridge() {
	// TODO
	// if (!v_starfieldInitialized)
	// sub_12691()
	// else
	// clearScreenAndDelSeveralSprites()
	// v_mouseControllingShip = 0
	// v_keyboardControlsMouse = 1
}

void StarTrekEngine::runBridge() {
	while (_gameMode == GAMEMODE_BRIDGE && !_resetGameMode) {
		handleBridgeEvents();

		// TODO: pseudo-3D related functionality

		if (_bridgeSequenceToLoad != kSeqNone) {
			int bridgeSequence = _bridgeSequenceToLoad;
			_bridgeSequenceToLoad = kSeqNone;
			playBridgeSequence(bridgeSequence);
		}
	}
}

void StarTrekEngine::handleBridgeEvents() {
	TrekEvent event;

	if (popNextEvent(&event)) {
		if (_gameIsPaused) {
			if (event.type == TREKEVENT_LBUTTONDOWN || event.type == TREKEVENT_KEYDOWN) {
				_gameIsPaused = false;
			}
			return;
		}

		switch (event.type) {
		case TREKEVENT_TICK:
			updateActorAnimations();

			//updateMouseBitmap();
			renderBanBelowSprites();
			_gfx->drawAllSprites(false);
			renderBanAboveSprites();
			_gfx->updateScreen();

			_sound->checkLoopMusic();
			_frameIndex++;
			_roomFrameCounter++;
			addAction(ACTION_TICK, _roomFrameCounter & 0xff, (_roomFrameCounter >> 8) & 0xff, 0);
			if (_roomFrameCounter >= 2)
				_gfx->incPaletteFadeLevel();
			break;

		case TREKEVENT_LBUTTONDOWN:
			bridgeLeftClick();
			break;

		case TREKEVENT_MOUSEMOVE:
			// TODO
			break;

		case TREKEVENT_RBUTTONDOWN:
			// TODO
			break;

		case TREKEVENT_KEYDOWN:
			switch (event.kbd.keycode) {
			case Common::KEYCODE_w:	// Toggle weapons on/off
				handleBridgeMenu(kBridgeChekovWeapons);
				break;
			case Common::KEYCODE_s:	// Toggle shields on/off
				handleBridgeMenu(kBridgeSuluShields);
				break;
			case Common::KEYCODE_a:	// Target analysis
				handleBridgeMenu(kBridgeSuluTargetAnalysis);
				break;
			case Common::KEYCODE_d:	// Damage control
				handleBridgeMenu(kBridgeScottyDamageControl);
				break;
			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
			case Common::KEYCODE_F1:
				// Fire phasers
				// TODO
				break;
			case Common::KEYCODE_SPACE:	// Fire photon torpedoes
				// TODO
				break;
			case Common::KEYCODE_v:	// Toggle between the normal bridge view and full screen view
				// TODO
				break;
			case Common::KEYCODE_TAB:	// Toggle between movement/fire mode and crew selection mode
				// TODO
				break;
			case Common::KEYCODE_COMMA:	// Reduce the main view screen magnification
				// TODO
				break;
			case Common::KEYCODE_PERIOD:	// Enlarge the main view screen magnification
				// TODO
				break;
			case Common::KEYCODE_o:	// Enter/exit orbit
				handleBridgeMenu(kBridgeSuluOrbit);
				break;
			case Common::KEYCODE_KP5:	// Center controls
				// TODO
				break;
			case Common::KEYCODE_n:	// Main star navigational map
				handleBridgeMenu(kBridgeChekovNavigation);
				break;
			case Common::KEYCODE_k:	// Kirk's options
				handleBridgeMenu(kBridgeKirkOptions);
				break;
			case Common::KEYCODE_t:	// Ask Mr. Spock for advice
				handleBridgeMenu(kBridgeSpock);
				break;
			case Common::KEYCODE_c:	// Spock's library computer
				handleBridgeMenu(kBridgeSpockComputer);
				break;
			case Common::KEYCODE_h:	// Uhura's communication icon
				handleBridgeMenu(kBridgeUhura);
				break;
			case Common::KEYCODE_p:	// Pause game
				_gameIsPaused = true;
				break;
			case Common::KEYCODE_e:	// Toggle SFX / emergency power
				if (event.kbd.flags & Common::KBD_CTRL)
					_sound->toggleSfx();
				else
					handleBridgeMenu(kBridgeScottyEmergencyPower);
				break;
			case Common::KEYCODE_m:
				if (event.kbd.flags & Common::KBD_CTRL)
					_sound->toggleMusic();
				break;
			case Common::KEYCODE_q:
				if (event.kbd.flags & Common::KBD_CTRL)
					showQuitGamePrompt(20, 20);
				break;
			case Common::KEYCODE_TILDE:	// Ship speed: reverse
				// TODO
				break;
			case Common::KEYCODE_1:	// Ship speed: stop
				// TODO
				break;
			case Common::KEYCODE_2:
			case Common::KEYCODE_3:
			case Common::KEYCODE_4:
			case Common::KEYCODE_5:
			case Common::KEYCODE_6:
			case Common::KEYCODE_7:
			case Common::KEYCODE_8:
			case Common::KEYCODE_9:
			case Common::KEYCODE_0:
				// Ship speed: toggle slow -> fast
				// TODO
				break;
			default:
				break;
			}
			break;

		default:
			break;
		}
	}
}

void StarTrekEngine::handleBridgeMenu(int menuEvent) {
	const char *raiseShieldsText = "#BRID\\B_340#Raising shields.";
	const char *lowerShieldsText = "#BRID\\B_337#Lowering shields, Captain.";
	const char *armWeaponsText = "#BRID\\B_351#Arming weapons.";
	const char *disarmWeaponsText = "#BRID\\B_354#Disarming weapons.";
	const char *emergencyPowerText = "#BRID\\BRID_S32#I don't know how long she can take it, Captain.";
	const char *noEmergencyPowerText = "#BRID\\BRID_S41#She can't take it, Captain.";
	const char *targetAnalysisOnText = "#BRID\\B_344#Target Analysis On.";
	const char *targetAnalysisOffText = "#BRID\\B_343#Target Analysis Off.";
	const char *ayeSirText = "#BRID\\BRID_S22#Aye Sir.";

	switch (menuEvent) {
	case kBridgeKirkCommand:
		showBridgeMenu("command", 60, 100);
		break;
	case kBridgeUnk1:
		// TODO
		break;
	case kBridgeUnk2:
		// TODO
		break;
	case kBridgeStarfieldFullScreen:
		// TODO
		break;
	case kBridgeKirkCaptainsLog: // Kirk, captain's log
		captainsLog();
		break;
	case kBridgeKirkTransporter: // Kirk, transporter
		bridgeCrewAction(kBridgeTalkerKirk);
		break;
	case kBridgeKirkOptions: // Kirk, options
		showOptionsMenu(65, 60);
		break;
	case kBridgeSpock: // Spock, consult
		bridgeCrewAction(kBridgeTalkerSpock);
		break;
	case kBridgeSpockComputer: // Spock, consult computer
		handleBridgeComputer();
		break;
	case kBridgeScottyDamageControl: // Scotty, damage control
		showBridgeMenu("repair", 30, 30);
		break;
	case kBridgeScottyEmergencyPower: // Scotty, emergency power
		// TODO: check for emergency power
		showTextboxBridge(kBridgeTalkerScotty, noEmergencyPowerText);
		break;
	case kBridgeUhura: // Uhura, communications
		bridgeCrewAction(kBridgeTalkerUhura);
		break;
	case kBridgeSuluOrbit: // Sulu, orbit
		bridgeCrewAction(kBridgeTalkerSulu);
		break;
	case kBridgeSuluShields: // Sulu, shields
		_enterpriseState.shields = !_enterpriseState.shields;
		showTextboxBridge(kBridgeTalkerSulu, _enterpriseState.shields ? raiseShieldsText : lowerShieldsText);
		break;
	case kBridgeChekovNavigation: // Chekov, navigation
		bridgeCrewAction(kBridgeTalkerChekov);
		break;
	case kBridgeChekovWeapons: // Chekov, weapons
		_enterpriseState.weapons = !_enterpriseState.weapons;
		showTextboxBridge(kBridgeTalkerChekov, _enterpriseState.weapons ? armWeaponsText : disarmWeaponsText);
		setBridgeMouseCursor();
		break;
	case kBridgeChekovRepairShields:
	case kBridgeChekovRepairPhasers:
	case kBridgeChekovRepairPhotonTorpedoes:
	case kBridgeChekovRepairSensors:
	case kBridgeChekovRepairBridge:
	case kBridgeChekovRepairHull:
	case kBridgeChekovRepairWarpDrives:
		// TODO: Repair ship part (-1, 4, 2, 6, 1, 0, 7)
		showTextboxBridge(kBridgeTalkerScotty, ayeSirText);
		break;
	case kBridgeSuluTargetAnalysis: // Chekov, target analysis
		_enterpriseState.targetAnalysis = !_enterpriseState.targetAnalysis;
		showTextboxBridge(kBridgeTalkerSulu, _enterpriseState.targetAnalysis ? targetAnalysisOnText : targetAnalysisOffText);
		// TODO: Target analysis
		break;
	default:
		break;
	}
}

void StarTrekEngine::showStarMap() {
	// HACK: Move to the target planet and play its associated sequence, until the starmap code is implemented
	_currentPlanet = _targetPlanet;

	if (_targetPlanet == kPlanetPollux)
		_bridgeSequenceToLoad = kSeqArrivedAtPolluxV;
	else if (_targetPlanet == kPlanetBetaMyamid)
		_bridgeSequenceToLoad = kSeqStartElasiPirateBattle;
	else if (_targetPlanet == kPlanetArk7)
		_bridgeSequenceToLoad = kSeqStartRomulanBattle;
	else if (_targetPlanet == kPlanetHarlequin)
		_bridgeSequenceToLoad = kSeqFightElasiShipsAndContactMudd;
	// TODO
}

void StarTrekEngine::showMissionStartEnterpriseFlyby(Common::String sequence, Common::String name) {
	// TODO
}

void StarTrekEngine::startBattle(Common::String enemyShip) {
	// TODO
}

void StarTrekEngine::wrongDestinationRandomEncounter() {
	_randomEncounterType = _randomSource.getRandomNumberRng(1, 3);

	switch (_randomEncounterType) {
	case kRandomEncounterKlingon:
		_resource->setTxtFileName("klingon");
		break;
	case kRandomEncounterRomulan:
		_resource->setTxtFileName("romulan");
		break;
	case kRandomEncounterElasi:
		_resource->setTxtFileName("elasi");
		break;
	default:
		break;
	}

	// TODO: The rest

	_enterpriseState.underAttack = true;
	_sound->loadMusicFile("bridge");	// TODO: check for bridgeb
}

// Used when approaching Beta Myamid / Masada in chapter 2, and Ark 7 in chapter 3
void StarTrekEngine::orbitPlanetSequence(int sequenceId) {
	// TODO
}

void StarTrekEngine::orbitPlanet() {
	const char *enteringOrbitText = "#BRID\\B_333#Entering standard orbit.";
	const char *leavingOrbitText = "#BRID\\B_335#Leaving orbit.";

	// TODO: Show starfield
	// TODO: Warp to planet, if needed
	// TODO: Update sprites

	_enterpriseState.inOrbit = !_enterpriseState.inOrbit;
	showTextboxBridge(kBridgeTalkerSulu, _enterpriseState.inOrbit ? enteringOrbitText : leavingOrbitText);
}

void StarTrekEngine::captainsLog() {
	const char *missionNames[] = {
		"Demon World",
		"Hijacked",
		"Love's Labor Jeopardized",
		"Another Fine Mess",
		"Feathered Serpent",
		"That Old Devil Moon",
		"Vengeance"
	};

	int totalPoints = 0;
	for (int i = 0; i < 7; i++)
		totalPoints += _missionPoints[i];

	Common::String captainsLogMessage = "";

	if (_lastMissionId > -1) {
		captainsLogMessage = Common::String::format(
			"#BRID\\C_007#We have earned %d point",
			totalPoints
		);

		if (totalPoints > 1)
			captainsLogMessage += "s";

		captainsLogMessage += " for the following recently completed missions:\n";

		for (int i = 6; i >= 0; i--) {
			captainsLogMessage += missionNames[i] + Common::String::format(":   %d point", _missionPoints[i]);
			if (_missionPoints[i] > 1)
				captainsLogMessage += "s";
			captainsLogMessage + ".\n\n";
		}
	} else {
		captainsLogMessage = "#BRID\\C_007#No recent missions have been completed.";
	}

	showTextboxBridge(kBridgeTalkerCaptainsLog, captainsLogMessage);
}

void StarTrekEngine::loadBridgeComputerTopics() {
	Common::MemoryReadStreamEndian *indexFile = _resource->loadFile("db.key");
	Common::String line;
	while (!indexFile->eos() && !indexFile->err()) {
		line = indexFile->readLine();
		if (line == "$")
			break;
		ComputerTopic topic;
		topic.fileName = line.substr(0, 7);
		topic.topic = line.substr(8);
		_computerTopics.push_back(topic);
	}
	delete indexFile;
}

void StarTrekEngine::handleBridgeComputer() {
	Common::String line;
	Common::String input = showComputerInputBox();
	if (input.size() < 3)
		return;

	char topics[10][8];
	int topicIndex = 0;

	memset(topics, 0, ARRAYSIZE(topics) * 8);

	for (Common::List<ComputerTopic>::const_iterator i = _computerTopics.begin(), end = _computerTopics.end(); i != end; ++i) {
		if (i->topic.hasPrefixIgnoreCase(input)) {
			memcpy(topics[topicIndex++], i->fileName.c_str(), i->fileName.size());
			if (topicIndex >= ARRAYSIZE(topics))
				break;
		}
	}

	showText(&StarTrekEngine::readTextFromFoundComputerTopics, (uintptr)topics, 20, 20, TEXTCOLOR_YELLOW, true, false, true);
}

} // End of namespace StarTrek
