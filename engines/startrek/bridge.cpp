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
	//sub_1312C();	// TODO

	// TODO
	//initStarfieldSprite();
	//initStarfieldSprite();
	//initStarfieldSprite();
	//initStarfieldSprite();
	//initStarfieldSprite();
}

void StarTrekEngine::loadBridgeActors() {
	loadActorAnim(0, "bstndki", 0, 0, 4);	// Kirk
	loadActorAnim(1, "bstndsp", 0, 0, 0);	// Spock
	loadActorAnim(6, "bstnduh", 0, 0, 0);	// Uhura
	loadActorAnim(5, "bstndch", 0, 0, 0);	// Chekov
	loadActorAnim(4, "bstndsu", 0, 0, 0);	// Sulu
	loadActorAnim(7, "bstndsc", 0, 0, 0);	// Scotty
	loadActorAnim(2, "xstndmc", 0, 0, 0);	// McCoy
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
			// TODO
			break;

		case TREKEVENT_MOUSEMOVE:
			// TODO
			break;

		case TREKEVENT_RBUTTONDOWN:
			// TODO
			break;

		case TREKEVENT_KEYDOWN:
			switch (event.kbd.keycode) {
			case Common::KEYCODE_w:
				// Toggle weapons on/off
				// TODO
				break;
			case Common::KEYCODE_s:
				// Toggle shields on/off
				// TODO
				break;
			case Common::KEYCODE_a:
				// Target analysis
				// TODO
				break;
			case Common::KEYCODE_d:
				// Damage control
				// TODO
				break;
			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
			case Common::KEYCODE_F1:
				// Fire phasers
				// TODO
				break;
			case Common::KEYCODE_SPACE:
				// Fire photon torpedoes
				// TODO
				break;
			case Common::KEYCODE_v:
				// Toggle between the normal bridge view and full screen view
				// TODO
				break;
			case Common::KEYCODE_TAB:
				// Toggle between movement/fire mode and crew selection mode
				// TODO
				break;
			case Common::KEYCODE_COMMA:
				// Reduce the main view screen magnification
				// TODO
				break;
			case Common::KEYCODE_PERIOD:
				// Enlarge the main view screen magnification
				// TODO
				break;
			case Common::KEYCODE_o:
				// Enter/exit orbit
				// TODO
				break;
			case Common::KEYCODE_KP5:
				// Center controls
				// TODO
				break;
			case Common::KEYCODE_n:
				// Main star navigational map
				// TODO
				break;
			case Common::KEYCODE_k:
				// Kirk's options
				// TODO
				break;
			case Common::KEYCODE_t:
				// Ask Mr. Spock for advice
				// TODO
				break;
			case Common::KEYCODE_c:
				// Spock's library computer
				handleBridgeComputer();
				break;
			case Common::KEYCODE_h:
				// Uhura's communication icon
				// TODO
				break;
			case Common::KEYCODE_p:
				// Pause game
				// TODO
				break;

			case Common::KEYCODE_e:
				if (event.kbd.flags && Common::KBD_CTRL) {
					_sound->toggleSfx();
				} else {
					// Emergency power
					// TODO
				}
				break;

			case Common::KEYCODE_m:
				if (event.kbd.flags && Common::KBD_CTRL) {
					_sound->toggleMusic();
				}
				break;

			case Common::KEYCODE_q:
				if (event.kbd.flags && Common::KBD_CTRL) {
					showQuitGamePrompt(20, 20);
				}
				break;
			case Common::KEYCODE_TILDE:
				// Ship speed: reverse
				// TODO
				break;
			case Common::KEYCODE_1:
				// Ship speed: stop
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
