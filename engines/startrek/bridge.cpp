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
	kBridgeSpockTalk = 0,
	kBridgeUnk1 = 1,
	kBridgeUnk2 = 2,
	kBridgeUnk3 = 3,
	kBridgeKirkCaptainsLog = 16,
	kBridgeKirkTransporter = 17,
	kBridgeKirkOptions = 18,
	kBridgeSpockNothingToReport = 32,
	kBridgeSpockConsultComputer = 33,
	kBridgeScottyDamageControl = 48,
	kBridgeScottyEmergencyPower = 49,
	kBridgeUhuraCommunications = 64,
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

void StarTrekEngine::initBridge(bool b) {
	_gfx->loadPalette("bridge");
	_sound->loadMusicFile("bridge");

	initStarfieldPosition();
	// TODO: starfield

	loadBridge();
}

void StarTrekEngine::loadBridge() {
	initStarfield(72, 30, 247, 102, 0);
	// TODO
	//initStarfieldSprite();

	_gfx->setBackgroundImage("bridge");
	_gfx->loadPri("bridge");
	_gfx->copyBackgroundScreen();
	_system->updateScreen();

	loadBridgeActors();
	_missionName = _missionToLoad;
	_resource->setTxtFileName(_missionName);
	_sound->loadMusicFile("bridge");
	_sound->playMidiMusicTracks(0, -1);
	//loadBanFile("T0BAN");
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
	}
}

void StarTrekEngine::handleBridgeEvents() {
	TrekEvent event;

	if (popNextEvent(&event)) {
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
				// TODO
				break;
			case Common::KEYCODE_k:	// Kirk's options
				handleBridgeMenu(kBridgeKirkOptions);
				break;
			case Common::KEYCODE_t:	// Ask Mr. Spock for advice
				// TODO
				handleBridgeMenu(kBridgeSpockNothingToReport);
				break;
			case Common::KEYCODE_c:	// Spock's library computer
				handleBridgeMenu(kBridgeSpockConsultComputer);
				break;
			case Common::KEYCODE_h:	// Uhura's communication icon
				handleBridgeMenu(kBridgeUhuraCommunications);
				break;
			case Common::KEYCODE_p:	// Pause game
				// TODO
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
	const char *kirkHeader = "KIRK";
	const char *spockHeader = "MR. SPOCK";
	const char *scottyHeader = "MR. SCOTT";
	const char *uhuraHeader = "LIEUTENANT UHURA";
	const char *suluHeader = "MR. SULU";
	const char *chekovHeader = "MR. CHEKOV";
	const char *shieldsUpText = "#BRID\\B_332#Captain, the shields are up.";
	const char *notInOrbitText = "#BRID\\B_350#We're not in orbit, Captain.";
	const char *transporterText = "#BRID\\C_060#Spock, come with me. Mr Scott, you have the conn.";
	const char *nothingToReportText = "#BRID\\B_155#Nothing to report, Captain.";
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
	case kBridgeSpockTalk:
		// TODO
		break;
	case kBridgeUnk1:
		// TODO
		break;
	case kBridgeUnk2:
		// TODO
		break;
	case kBridgeUnk3:
		// TODO
		break;
	case kBridgeKirkCaptainsLog: // Kirk, captain's log
		captainsLog();
		break;
	case kBridgeKirkTransporter: // Kirk, transporter
		if (_enterpriseState.shields) {
			showTextbox(suluHeader, shieldsUpText, 122, 116, 161, 0);
		} else {
			if (!_enterpriseState.inOrbit) {
				showTextbox(suluHeader, notInOrbitText, 122, 116, 161, 0);
			} else {
				// TODO: Check if the destination is correct
				// TODO: Check for veng or feather missions and show extra text
				showTextbox(kirkHeader, transporterText, 160, 130, 176, 0);
				runGameMode(GAMEMODE_BEAMDOWN, false);
			}
		}
		break;
	case kBridgeKirkOptions: // Kirk, options
		showOptionsMenu(65, 60);
		break;
	case kBridgeSpockNothingToReport: // Spock, nothing to report
		showTextbox(spockHeader, nothingToReportText, 294, 106, 44, 0);
		break;
	case kBridgeSpockConsultComputer: // Spock, consult computer
		handleBridgeComputer();
		break;
	case kBridgeScottyDamageControl: // Scotty, damage control
		showBridgeMenu("repair", 30, 30);
		break;
	case kBridgeScottyEmergencyPower: // Scotty, emergency power
		// TODO: check for emergency power
		showTextbox(scottyHeader, noEmergencyPowerText, 64, 100, 161, 0);
		break;
	case kBridgeUhuraCommunications: // Uhura, communications
		// TODO: text
		showTextbox(uhuraHeader, _resource->getLoadedText(16), 298, 150, 161, 0);
		break;
	case kBridgeSuluOrbit: // Sulu, orbit
		if (_enterpriseState.underAttack) {
			showTextbox(suluHeader, underAttackText, 122, 116, 176, 0);
		} else {
			// TODO: Check if mission is over
			//showTextbox(suluHeader, missionNotOverText, 122, 116, 44, 0);

			if (_enterpriseState.inOrbit) {
				showTextbox(suluHeader, leaveOrbitText, 122, 116, 176, 0);
			} else {
				// TODO: Finish this
				_enterpriseState.inOrbit = true;
			}
		}
		break;
	case kBridgeSuluShields: // Sulu, shields
		_enterpriseState.shields = !_enterpriseState.shields;
		showTextbox(suluHeader, _enterpriseState.shields ? raiseShieldsText : lowerShieldsText, 122, 116, 176, 0);
		break;
	case kBridgeChekovNavigation: // Chekov, navigation
		// TODO
		break;
	case kBridgeChekovWeapons: // Chekov, weapons
		_enterpriseState.weapons = !_enterpriseState.weapons;
		showTextbox(chekovHeader, _enterpriseState.weapons ? armWeaponsText : disarmWeaponsText, 196, 116, 176, 0);
		// TODO: weapons
		break;
	case kBridgeChekovRepairShields:
	case kBridgeChekovRepairPhasers:
	case kBridgeChekovRepairPhotonTorpedoes:
	case kBridgeChekovRepairSensors:
	case kBridgeChekovRepairBridge:
	case kBridgeChekovRepairHull:
	case kBridgeChekovRepairWarpDrives:
		// TODO: Repair ship part (-1, 4, 2, 6, 1, 0, 7)
		showTextbox(scottyHeader, ayeSirText, 64, 100, 161, 0);
		break;
	case kBridgeSuluTargetAnalysis: // Chekov, target analysis
		_enterpriseState.targetAnalysis = !_enterpriseState.targetAnalysis;
		showTextbox(suluHeader, _enterpriseState.targetAnalysis ? targetAnalysisOnText : targetAnalysisOffText, 122, 116, 176, 0);
		// TODO: Target analysis
		break;
	default:
		break;
	}
}

void StarTrekEngine::captainsLog() {
	// TODO: Show points for recently completed missions
	const char *captainsLogHeader = "Captain's Log";
	const char *noRecentMissions = "#BRID\\C_007#No recent missions have been completed.";
	showTextbox(captainsLogHeader, noRecentMissions, 160, 130, 176, 0);
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
