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
	kSeqEndMockBattle,
	kSeqShowDebriefDemon,
	kSeqArrivedAtPolluxV,
	kSeqEndMissionDemon,
	// -- Chapter 2 ----
	kSeqStartMissionTug,
	kSeqStartElasiPirateBattle,
	kSeqArrivedAtBetaMyamid,
	kSeqApproachedTheMasada,
	kSeqEndMissionTug,
	// -- Chapter 3 ----
	kSeqStartMissionLove,
	kSeqUnk11,
	kSeqUnk12,
	kSeqUnk13,
	kSeqUnk14,
	kSeqUnk15,
	// -- Chapter 4 ----
	kSeqStartMissionMudd,
	kSeqUnk17,
	kSeqUnk18,
	// -- Chapter 5 ----
	kSeqStartMissionFeather,
	kSeqUnk20,
	kSeqUnk21,
	kSeqStartMissionTrial,
	kSeqUnk23,
	kSeqUnk24,
	kSeqUnk25,
	kSeqUnk26,
	kSeqUnk27,
	// -- Chapter 6 ----
	kSeqStartMissionVeng,
	kSeqUnk29,
	// -- Chapter 7 ----
	kSeqStartMissionSins,
	kSeqUnk31
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
	kBridgeTalkerCaptainsLog
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
	initStarfield(72 + 1, 30 + 1, 247 + 1, 102 + 1, 0);

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
	_gfx->setMouseBitmap("pushbtn"/*_mouseControllingShip ? "entcur" : "cursor"*/);
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
		_sound->playSoundEffectIndex(40);

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
		// In the original, this is actually a handler for Spock, Uhura and Chekov.
		// We moved their actions in separate functions instead.
		if (_currentPlanet == _targetPlanet) {
			showTextboxBridge(kBridgeTalkerSpock, 15); // We have arrived at Pollux V
		}
		break;
	case kSeqEndMissionDemon:
		_resource->setTxtFileName("DEMON");
		loadActorAnim(1, "xstndsp", 0, 0, 1.0);	// Standing Spock
		loadActorAnim(2, "xstndmc", 0, 0, 1.0);	// Standing McCoy
		_sound->playSoundEffectIndex(34);
		showTextboxBridge(kBridgeTalkerUhura, 23); // Message from Starfleet
		showTextboxBridge(kBridgeTalkerKirk, 24);
		showMissionPerformance(_awayMission.demon.missionScore * 100 / 32, 29);
		showTextboxBridge(kBridgeTalkerMcCoy, 25);
		showTextboxBridge(kBridgeTalkerKirk, 26);
		showTextboxBridge(kBridgeTalkerSpock, 27);
		showTextboxBridge(kBridgeTalkerMcCoy, 28);
		loadActorAnim(1, "bstndsp", 0, 0, 1.0); // Sitting Spock
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
		_sound->playSoundEffectIndex(34);
		showTextboxBridge(kBridgeTalkerUhura, 0);
		showTextboxBridge(kBridgeTalkerKirk, 1);
		showBridgeScreenTalkerWithMessage(2, "Admiral", "woman");
		showTextboxBridge(kBridgeTalkerChekov, 3);
		break;
	case kSeqStartElasiPirateBattle:
		showTextboxBridge(kBridgeTalkerSpock, 6);
		_sound->playSoundEffectIndex(34);
		showTextboxBridge(kBridgeTalkerUhura, 7);
		showTextboxBridge(kBridgeTalkerKirk, 8);
		showBridgeScreenTalkerWithMessage(9, "Elasi Captain", "pira", false);
		showTextboxBridge(kBridgeTalkerKirk, 10);
		showTextbox("Elasi Captain", _resource->getLoadedText(11), 160, 190, 44, 0);
		removeActorFromScreen(_currentScreenTalker);
		initStarfieldSprite(&_starfieldSprite, new StubBitmap(0, 0), _starfieldRect);
		_sound->playSoundEffectIndex(40);
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
		break;
	case kSeqApproachedTheMasada:
		showTextboxBridge(kBridgeTalkerSpock, 20);
		// TODO: showText2
		showTextboxBridge(kBridgeTalkerUhura, 21);
		showBridgeScreenTalkerWithMessage(22, "Elasi Cereth", "pira", false);
		showTextboxBridge(kBridgeTalkerKirk, 23);
		removeActorFromScreen(_currentScreenTalker);
		initStarfieldSprite(&_starfieldSprite, new StubBitmap(0, 0), _starfieldRect);
		break;
	case kSeqEndMissionTug:
		_resource->setTxtFileName("TUG");
		loadActorAnim(1, "xstndsp", 0, 0, 1.0); // Standing Spock
		loadActorAnim(2, "xstndmc", 0, 0, 1.0); // Standing McCoy
		// TODO: 3 possible outcomes depending on mission actions
		_sound->playSoundEffectIndex(34);
		showTextboxBridge(kBridgeTalkerUhura, 29); // Message from Starfleet
		showTextboxBridge(kBridgeTalkerKirk, 30);
		showMissionPerformance(_awayMission.tug.missionScore * 100 / 32, 31);
		showTextboxBridge(kBridgeTalkerMcCoy, 32);
		showTextboxBridge(kBridgeTalkerSpock, 33);
		showTextboxBridge(kBridgeTalkerMcCoy, 34);
		showTextboxBridge(kBridgeTalkerKirk, 35);
		showTextboxBridge(kBridgeTalkerSpock, 36);
		showTextboxBridge(kBridgeTalkerKirk, 37);
		loadActorAnim(1, "bstndsp", 0, 0, 1.0); // Sitting Spock
		removeActorFromScreen(2);
		_bridgeSequenceToLoad = kSeqStartMissionLove;
		break;
	case kSeqStartMissionLove:	// Chapter 3: Love's Labor Jeopardized (love)
		_targetPlanet = kPlanetArk7;
		// TODO
		break;
	case kSeqStartMissionMudd: // Chapter 4: Another Fine Mess (mudd)
		_targetPlanet = kPlanetHarlequin;
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
	{ kPlanetPollux,     kBridgeTalkerSpock, 10, 20, 21, 22 },
	{ kPlanetPollux,     kBridgeTalkerUhura,  9, 16, 19, 19},
	{ kPlanetBetaMyamid, kBridgeTalkerSpock, 13,  5, 17, 27 },
	{ kPlanetBetaMyamid, kBridgeTalkerUhura, 12,  4, 16, -1 },
	// TODO: The rest
	{ kPlanetNone,       kBridgeTalkerNone,   0,  0,  0,  0 }
};

void StarTrekEngine::bridgeCrewAction(int crewId) {
	CrewTextsForChapter *curCrewTexts = crewTexts;
	int textId = -1;

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
	if (crewId == kBridgeTalkerUhura && _currentPlanet == _targetPlanet && !_hailedTarget) {
		contactTargetAction();
		_hailedTarget = true;
		return;
	}

	if (textId >= 0)
		showTextboxBridge(crewId, textId);
}

void StarTrekEngine::contactTargetAction() {
	switch (_targetPlanet) {
	case kPlanetPollux:	// Chapter 1: Demon world (demon)
		_sound->playSoundEffectIndex(34);
		showTextboxBridge(kBridgeTalkerUhura, 17);
		showBridgeScreenTalkerWithMessage(18, "Priest", "prst");
		break;
	case kPlanetBetaMyamid: // Chapter 2: Hijacked (tug)
		break;
	// TODO: The rest
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

// TODO: two more parameters
void StarTrekEngine::showMissionPerformance(int score, int missionScoreTextId) {
	Common::String performanceDescription;
	int midiTrack = 0;

	if (score >= 0 && score <= 50) {
		performanceDescription = "#BRID\\B_199#I'll be frank, Kirk. Starfleet expects more of you than that. Try to do better on your next assignment.";
		midiTrack = 13;
	} else if (score >= 60 && score <= 70) {
		performanceDescription = "#BRID\\B_197#A satisfactory performance, Captain, but there's still room for improvement.";
		midiTrack = 13;
	} else if (score >= 71 && score <= 85) {
		performanceDescription = "#BRID\\B_214#Well done, Captain. Keep up the good work.";
		midiTrack = 11;
	} else if (score >= 86 && score <= 99) {
		performanceDescription = "#BRID\\B_414#The top brass at Starfleet are impressed. Outstanding work, Jim.";
		midiTrack = 12;
	} else if (score == 100) {
		performanceDescription = "#BRID\\B_195#A perfect mission, Jim! You are a model for all Starfleet!";
		midiTrack = 14;
	}

	_sound->playMidiMusicTracks(midiTrack, -1);

	int commendationPoints = 0;	// TODO
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
	// TODO: Check why we need the coord adjustments below
	_currentScreenTalker = loadActorAnim(-1, talkerId, 72 - 2, 30 + 1, 1.0);
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
				if (event.kbd.flags && Common::KBD_CTRL)
					_sound->toggleSfx();
				else
					handleBridgeMenu(kBridgeScottyEmergencyPower);
				break;
			case Common::KEYCODE_m:
				if (event.kbd.flags && Common::KBD_CTRL)
					_sound->toggleMusic();
				break;
			case Common::KEYCODE_q:
				if (event.kbd.flags && Common::KBD_CTRL)
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
	// TODO: Move these
	const char *shieldsUpText = "#BRID\\B_332#Captain, the shields are up.";
	const char *notInOrbitText = "#BRID\\B_350#We're not in orbit, Captain.";
	const char *wrongDestinationText = "#BRID\\B_346#This isn't our destination.";
	const char *transporterText = "#BRID\\C_060#Spock, come with me. Mr Scott, you have the conn.";
	const char *transporterTextFeather = "#BRID\\C_006#Assemble a landing party. Unless we find this so-called criminal, we're going to war.";
	const char *raiseShieldsText = "#BRID\\B_340#Raising shields.";
	const char *lowerShieldsText = "#BRID\\B_337#Lowering shields, Captain.";
	const char *armWeaponsText = "#BRID\\B_351#Arming weapons.";
	const char *disarmWeaponsText = "#BRID\\B_354#Disarming weapons.";
	const char *emergencyPowerText = "#BRID\\BRID_S32#I don't know how long she can take it, Captain.";
	const char *noEmergencyPowerText = "#BRID\\BRID_S41#She can't take it, Captain.";
	const char *underAttackText = "#BRID\\B_348#Unable to comply, Captain.  We're under attack.";
	const char *missionNotOverText = "#BRID\\B_338#May I respectfully remind the Captain that we haven't accomplished our mission, Sir.";
	const char *leaveOrbitText = "#BRID\\B_349#We must first leave orbit, Sir.";
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
		if (_enterpriseState.shields) {
			showTextboxBridge(kBridgeTalkerSulu, shieldsUpText);
		} else if (!_enterpriseState.inOrbit) {
			showTextboxBridge(kBridgeTalkerSulu, notInOrbitText);
		} else if (_currentPlanet != _targetPlanet) {
			showTextboxBridge(kBridgeTalkerSulu, wrongDestinationText);
		} else {
			if (_missionToLoad != "FEATHER")
				showTextboxBridge(kBridgeTalkerKirk, transporterText);
			else
				showTextboxBridge(kBridgeTalkerKirk, transporterTextFeather);
			runGameMode(GAMEMODE_BEAMDOWN, false);
		}
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
		if (_enterpriseState.underAttack) {
			showTextboxBridge(kBridgeTalkerSulu, underAttackText);
		//} else if (false) {
		//	// TODO: Check if mission is over
		//	showTextboxBridge(kBridgeTalkerSulu, missionNotOverText);
		} else {
			orbitPlanet();
		}
		break;
	case kBridgeSuluShields: // Sulu, shields
		_enterpriseState.shields = !_enterpriseState.shields;
		showTextboxBridge(kBridgeTalkerSulu, _enterpriseState.shields ? raiseShieldsText : lowerShieldsText);
		break;
	case kBridgeChekovNavigation: // Chekov, navigation
		if (_enterpriseState.underAttack) {
			showTextboxBridge(kBridgeTalkerSulu, underAttackText);
		//} else if (false) {
		//	// TODO: Check if mission is over
		//	showTextboxBridge(kBridgeTalkerSulu, missionNotOverText);
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

	// TODO
}

void StarTrekEngine::showMissionStartEnterpriseFlyby(Common::String sequence, Common::String name) {
	// TODO
}

void StarTrekEngine::startBattle(Common::String enemyShip) {
	// TODO
}

void StarTrekEngine::wrongDestinationRandomEncounter() {
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
	// TODO: Show points for recently completed missions
	const char *noRecentMissions = "#BRID\\C_007#No recent missions have been completed.";
	showTextboxBridge(kBridgeTalkerCaptainsLog, noRecentMissions);
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
