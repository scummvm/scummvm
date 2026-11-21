/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/textconsole.h"
#include "common/util.h"
#include "hollywood/runaway/scene6300.h"
#include "hollywood/runaway/strings.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{1397, 456, 1, {301, 302}, {0, 3}, {1397, 456}, 6}, // 1: Stairs.
	{40, 436, 7, {303, 304}, {0, 1}, {40, 436}, 4}, // 2: Door.
	{389, 381, 7, {303, 305}, {0, 1}, {389, 381}, 4}, // 3: Door.
	{644, 342, 7, {303, 306}, {0, 1}, {644, 342}, 4}, // 4: Door.
	{741, 518, 7, {307, 308}, {0, 0}, {726, 495}}, // 5: Crane controls.
	{0, 0, 0, {309, 310}, {0, 0}, {778, 599}}, // 6: Statue.
	{862, 438, 7, {311, 312}, {0, 0}, {930, 417}}, // 7: Apparatus.
	{862, 438, 7, {313, 314}, {0, 1}, {849, 423}}, // 8: Lever.
	{0, 0, 0, {315, 316}, {0, 0}, {996, 402}}, // 9: Bucket of paint.
	{400, 452, 7, {317, 318}, {0, 0}, {400, 452}}, // 10: Water tank.
	{274, 489, 7, {319, 320}, {1, 1}, {233, 457}}, // 11: Tools.
	{0, 0, 1, {321, 322}, {0, 0}, {788, 341}}, // 12: Bed.
	{687, 525, 5, {323, 324}, {0, 1}, {494, 548}, 3}, // 13: Saturn.
	{211, 507, 7, {331, 332}, {0, 0}, {169, 502}}, // 14: Sculpture.
	{216, 505, 7, {327, 328}, {0, 1}, {179, 475}}, // 15: Container.
	{642, 568, 7, {329, 330}, {0, 1}, {602, 540}} // 16: Blowtorch.
};

const RunawayResponse kResponses[] = {
	{6300, 301, 1, 0},   // Look down the stairs.
	{6300, 303, 2, 0},   // Look through a balcony door toward Sushi's hotel.
	{6300, 315, 11, 0},  // Examine the bucket of paint.
	{6300, 316, 12, 0},  // Leave Saturn's paint alone.
	{6300, 317, 13, 0},  // Examine the water tank.
	{6300, 318, 14, 0},  // Refuse to waste water.
	{6300, 329, 22, 0},  // Examine the blowtorch Saturn left behind.
	{6300, 331, 24, 0},  // Examine Brian's sculpture after the trade.
	{6300, 334, 27, 0},  // Refuse to fill an unsuitable container.
	{6300, 338, 31, 0}   // Keep the wrench for Joshua.
};

const RunawayItemAction kItemActions[] = {
	{6300, 8, 10, 334, false}, {6300, 12, 10, 334, false},
	{6300, 13, 10, 334, false}, {6300, 13, 13, 335},
	{6300, 10, 11, 338, false}, {6300, 21, 10, 334, false},
	{6300, 23, 10, 333}, {6300, 24, 10, 337},
	{6300, 25, 10, 334, false}, {6300, 26, 10, 334, false},
	{6300, 29, 13, 336, -1}, {6300, 30, 13, 339, false},
	{6300, 31, 10, 334, false}
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene6300 = {
	6300, 1620, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {90, 90, 90}, {80, 80, 80}, {70, 70, 70}, {60, 60, 60}},
	Common::Point(1397, 456), kResponses, ARRAYSIZE(kResponses), kItemActions, ARRAYSIZE(kItemActions), nullptr, 0
};

namespace Runaway {

Scene6300::Scene6300(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

const char *Scene6300::actionCaptionVariant(byte id, bool look, byte item) const {
	if (!look && !item) {
		if (id == 8 && _town.catapult) {
			return "catapult";
		}
		if (id == 11 && _town.wrenchPermission) {
			return _town.wrenchPermission == 1 ? "wrench" : "tool";
		}
	}
	return nullptr;
}

bool Scene6300::loadCustomResources() {
	return _catapultAnimation.load(_archive, 6, 48) && _statueAnimation.load(_archive, 7, 2) &&
		_torchAnimation.load(_archive, 8, 2) && _sculptureAnimation.load(_archive, 9, 1) &&
		_kevinAnimation.load(_archive, 13, 112) && loadWorkshopForeground();
}

bool Scene6300::loadWorkshopForeground() {
	return _archive.readVariableChunk(4, _foregroundPatches[0]) && _archive.readVariableChunk(5, _foregroundPatches[1]);
}

bool Scene6300::applyCustomBackgroundPatches() {
	return !_town.sculptureTraded || (applyPatch(10) && applyPatch(11));
}

void Scene6300::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		if (_maps[i] >= 7 && _maps[i] <= 9) {
			_maps[i] = 0;
		}
		byte &id = _maps[256 + i];
		switch (id) {
		case 6: // The statue is no longer hanging from the crane.
			if (_town.statueLowered) {
				id = 0;
			}
			break;
		case 13: // Saturn leaves the workshop with the helmet.
			if (_town.kevinAway) {
				id = 0;
			}
			break;
		case 14: // Brian's sculpture replaces the gasoline by the balcony door.
		case 17:
			id = _town.sculptureTraded ? 14 : id == 17 ? 15 : 0;
			break;
		case 15:
			if (_town.sculptureTraded) {
				id = 0;
			}
			break;
		case 16: // The blowtorch is available after Saturn leaves it behind.
			if (!_town.kevinAway || _town.blowtorchTaken) {
				id = 0;
			}
			break;
		case 18: // Saturn initially obscures the middle balcony door.
			id = _town.kevinAway ? 3 : 13;
			break;
		case 19:
		case 20:
		case 21:
		case 22: // The catapult's target follows its current load.
			if (_town.catapult == 2) {
				id = 6;
			} else if (!_town.catapult) {
				id = id == 22 ? 0 : id == 21 ? 7 : 9;
			} else {
				id = id == 19 || id == 21 ? 7 : 0;
			}
			break;
		default:
			break;
		}
	}
}

void Scene6300::prepareCustomGameplayLoop() {
	_kevinChunk = 13;
	_kevinTick = _now;
}

void Scene6300::resumeAmbientAudio(bool entering) {
	if (!_town.kevinAway) {
		_audio.playMusic(24);
	} else if (!_audio.musicPlaying()) {
		_audio.playMusic(12 + _random.getRandomNumber(4), false);
	}
}

void Scene6300::runCustomEntrySequence() {
	static const Common::Point entries[] = {
		Common::Point(1397, 456), Common::Point(158, 522), Common::Point(492, 425), Common::Point(719, 359)
	};
	_position = entries[_entryScene - 6300];
	_walkPath.reset(_position);
	_facing = _entryScene == 6300 ? 5 : 3;
	if (_entryScene == 6300) {
		walkTo(Common::Point(1369, 459), 5, true);
		if (!_town.kevinMet) {
			deferTownAction(400);
		}
	}
}

void Scene6300::setKevinPose(uint chunk, uint frames, byte frame, byte talkFrame) {
	if (_kevinChunk != chunk && !_kevinAnimation.load(_archive, chunk, frames)) {
		error("Cannot load Runaway Saturn animation %u", chunk);
	}
	_kevinChunk = chunk;
	_kevinFrame = frame;
	_kevinTalkFrame = talkFrame;
	_kevinAnimating = false;
}

void Scene6300::animateKevin(uint16 action, uint chunk, uint frames, uint first, uint last) {
	_kevinAnimating = true;
	beginRoomAnimationRange(action, chunk, frames, first, last, false);
}

void Scene6300::advanceCustomGameplayLoop(uint32 delta) {
	if (_roomAction == 437 && !_actionFrames.empty()) {
		_actionFrames[0] = 33 + (speaker() ? 0 : speechFrame());
	}
	if (_town.kevinAway || _kevinAnimating || _kevinTalking || _kevinChunk != 13 || _now - _kevinTick < 80) {
		return;
	}
	_kevinTick = _now;
	advanceKevinIdle();
}

void Scene6300::advanceKevinIdle() {
	static const byte first[] = {0, 1, 3, 33, 61, 87};
	static const byte last[] = {0, 2, 28, 59, 86, 90};
	if (!_kevinIdle) {
		if (!_random.getRandomNumber(59)) {
			_kevinIdle = 1;
			_kevinFrame = 1;
		}
	} else if (_kevinFrame < last[_kevinIdle]) {
		++_kevinFrame;
		if (_kevinFrame == 9 || _kevinFrame == 41 || _kevinFrame == 65) {
			_audio.playEffect(16, 204);
		} else if (_kevinFrame == 25 || _kevinFrame == 52 || _kevinFrame == 79) {
			_audio.stopEffect(0);
		}
	} else {
		if (_kevinIdle == 5) {
			_kevinIdle = 0;
		} else if (_kevinIdle == 1) {
			_kevinIdle = 2 + _random.getRandomNumber(2);
		} else if (!_random.getRandomNumber(2)) {
			_kevinIdle = 5;
		} else if (_kevinIdle == 4) {
			_kevinIdle = 2 + _random.getRandomBit();
		} else {
			_kevinIdle = _random.getRandomNumber(3) ? 5 - _kevinIdle : 4;
		}
		_kevinFrame = first[_kevinIdle];
	}
}

void Scene6300::drawKevin(Graphics::ManagedSurface &viewport) {
	if (!_town.kevinAway && !(_kevinAnimating && drawingRoomAnimation())) {
		const uint frame = _kevinTalking ? _kevinTalkFrame + (speaker() ? speechFrame() % 6 : 0) : _kevinFrame;
		_kevinAnimation.draw(viewport, frame, _camera.x, 0);
	}
}

void Scene6300::drawCatapult(Graphics::ManagedSurface &viewport) {
	if (_roomAction != 415 || !drawingRoomAnimation()) {
		_catapultAnimation.draw(viewport, !_town.catapult ? 41 : _town.catapult == 2 ? 0 : 47, _camera.x, 0);
	}
}

void Scene6300::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	if (_town.sculptureTraded) {
		_sculptureAnimation.draw(viewport, 0, _camera.x, 0);
	}
	if (_position.y >= 406) {
		drawCatapult(viewport);
	}
	if (_position.y >= 526) {
		_torchAnimation.draw(viewport, _town.kevinAway && !_town.blowtorchTaken ? 0 : 1, _camera.x, 0);
		drawKevin(viewport);
	}
}

void Scene6300::drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) {
	if (_position.y < 406) {
		drawCatapult(viewport);
	}
	if (_position.y < 526) {
		_torchAnimation.draw(viewport, _town.kevinAway && !_town.blowtorchTaken ? 0 : 1, _camera.x, 0);
		drawKevin(viewport);
	}
	if (!_town.statueLowered && !((_roomAction == 448 || _roomAction == 409) && drawingRoomAnimation())) {
		_statueAnimation.draw(viewport, 0, _camera.x, 0);
	}
	if (_roomAction == 422 && drawingRoomAnimation() && _actionStep >= 53) {
		_wrenchAnimation.draw(viewport, MIN<uint>(6, _actionStep - 53), _camera.x, 0);
	}
	drawWorkshopForeground(viewport);
}

bool Scene6300::roomAnimationInForeground() const {
	// Keep the catapult and Saturn at the same depth as their idle frames.
	return _position.y < (_roomAction == 415 ? 406 : 526);
}

void Scene6300::drawWorkshopForeground(Graphics::ManagedSurface &viewport) {
	applyRunawayPatch(_foregroundPatches[0], viewport, 0, 1220 - _camera.x / 3 - _camera.x);
	applyRunawayPatch(_foregroundPatches[1], viewport, 0, 230 - _camera.x / 5 - _camera.x);
}

bool Scene6300::drawCustomComposite(Graphics::ManagedSurface &viewport) {
	if (!_catapultImpact) {
		return false;
	}
	viewport.blitFrom(_impactBackground);
	if (drawingRoomAnimation()) {
		_actionAnimation.draw(viewport, _actionFrames[MIN<uint>(_actionStep, _actionFrames.size() - 1)], 0, 0);
	}
	return true;
}

float Scene6300::actorHeight() const {
	return ((_position.y * 1.28048778f - 297.926819f) * 10 + _position.x * 0.151315793f + 333.947357f) / 11;
}

void Scene6300::setupActorDepth(RunawayActor::RenderParams &params) const {
	static const uint16 depth[] = {0, 450, 508, 522, 424, 358};
	for (uint i = 1; i < ARRAYSIZE(depth); ++i) {
		params.depthY[i] = depth[i];
	}
}

void Scene6300::findWalkPath(Common::Point target) {
	walkTown(target);
}

bool Scene6300::adjustCustomWalkTargetToFloorMask(Common::Point &target) const {
	target.x = CLIP<int>(target.x, 158, 1384);
	return false;
}

Common::U32String Scene6300::hotspotName(byte id) const {
	if (id == 7 && _town.catapult) {
		return _strings.hotspotName(_room, 17);
	}
	if (id == 8 && _town.catapult) {
		return _strings.hotspotName(_room, 18);
	}
	if (id == 15 && _town.fuelTradeKnown) {
		return _strings.hotspotName(_room, 19);
	}
	return RunawayRoomScene::hotspotName(id);
}

int8 Scene6300::customizeSceneActionApproach(byte id, bool use, byte item, int8 approach) const {
	if (_room == 6300 && id == 8 && use && !item) {
		return _town.catapult != 1;
	}
	return approach;
}

Common::Point Scene6300::sceneActionLookPosition(byte id) const {
	if (_room == 6300 && id == 6 && _town.statueLowered) {
		return Common::Point(996, 402);
	}
	return RunawayRoomScene::sceneActionLookPosition(id);
}

RunawayRoomScene::ActionResult Scene6300::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 302: // Return downstairs to the saloon.
		leaveTownRoom(6241);
		break;
	case 304: // Go through the nearest balcony door.
		leaveTownRoom(6228);
		break;
	case 305: // Go through the middle balcony door.
		leaveTownRoom(6227);
		break;
	case 306: // Go through the far balcony door.
		leaveTownRoom(6226);
		break;
	case 307: // Examine the crane controls.
		_town.craneKnown = 1;
		say(3);
		break;
	case 308: // Move the statue only after seeing where the catapult's paint landed.
		if (!_town.catapult || _town.bankVisited < 2 || !_town.basementKnown) {
			say(4);
		} else if (!_town.kevinAway) {
			if (_town.craneTried) {
				say(4, 6);
			} else {
				say(4, 1);
				walkTo(Common::Point(741, 518), 7, true);
				beginRoomAnimationRange(407, 25, 16, 0, 9);
			}
		} else if (!_town.craneTried) {
			say(4, 7);
			say(4, 8);
			walkTo(Common::Point(741, 518), 7, true);
			beginRoomAnimationRange(408, 25, 16, 0, 9);
		} else {
			say(4, _town.catapult == 2 ? 11 : 12);
		}
		break;
	case 309: // Examine the hanging or loaded statue.
		say(5, _town.statueLowered ? 2 : _random.getRandomBit());
		break;
	case 310: // Refuse to lift the statue by hand.
		say(6, _town.statueLowered);
		break;
	case 311: // Examine the apparatus before or after identifying the catapult.
		say(7, _town.catapult != 0);
		break;
	case 312: // Leave the catapult aimed at the bank.
		say(8, _town.catapult != 0);
		break;
	case 313: // Examine the catapult release lever.
		say(9, _town.catapult != 0);
		break;
	case 314: // Fire the paint bucket or the statue.
		if (_town.catapult == 1 || _town.catapult == 3) {
			say(10, 7);
		} else {
			say(10, _town.catapult == 2 ? 8 : 0);
			beginRoomAnimation(414, 26, 20);
		}
		break;
	case 319: // Examine Saturn's set of tools.
		say(15, _town.wrenchPermission == 2);
		break;
	case 320: // Ask Saturn for the missing number-ten wrench.
		if (!_town.wrenchNeeded || _town.wrenchFound == 2) {
			say(16);
		} else if (!_town.wrenchPermission) {
			say(16, 1);
		} else if (_town.wrenchThrown) {
			say(16, 2);
		} else {
			say(16, 3);
			deferTownAction(420);
		}
		break;
	case 321: // Examine Saturn's bed.
		say(17, _random.getRandomBit());
		break;
	case 322: // Refuse to sleep in Saturn's bed.
		say(18, _random.getRandomBit());
		break;
	case 323: // Look at Saturn.
		say(19, _random.getRandomBit());
		break;
	case 324: // Talk to Saturn about the town, his inventions, or supplies.
		startKevinConversation(false);
		break;
	case 327: // Identify the container as the gasoline Joshua needs.
		say(20, !_town.fuelNeeded ? 0 : _town.fuelTradeKnown ? 2 : 1);
		break;
	case 328: // Saturn requires a trade for his gasoline.
		say(21, !_town.fuelNeeded ? 0 : _town.fuelTradeKnown ? 2 : 1);
		break;
	case 330: // Take the abandoned blowtorch.
		say(23);
		deferTownAction(430);
		break;
	case 332: // Leave the sculpture with its owner.
		say(25, _town.kevinAway);
		break;
	case 333: // Fill the oil can with water.
	case 337: // Fill the one-liter bottle with water.
		_town.waterRequested = 1;
		if (action == 333 && _inventory.town().oilCanFull) {
			say(26);
		} else if (action == 337 && _inventory.town().bottleContents) {
			say(30, _inventory.town().bottleContents == 1 ? 2 : 3);
		} else if (!_town.kevinAway && !_town.waterPermission) {
			say(26, 1);
		} else {
			say(action == 333 ? 26 : 30, action == 333 ? 2 : 0);
			beginRoomAnimation(action, 19, 27);
		}
		break;
	case 335: // Offer Joshua's helmet as a cure for Saturn's lack of inspiration.
		_kevinTalking = true;
		say(28);
		say(28, 1);
		say(28, 2, false, 1);
		say(28, 3);
		deferTownAction(435);
		break;
	case 336: { // Offer the stone sculpture in exchange for gasoline.
		const byte sculpture = _inventory.town().sculpture;
		if (!_town.fuelTradeKnown) {
			say(29);
		} else if ((!sculpture && _town.roughSculptureRefused) || (sculpture == 1 && _town.polishedSculptureRefused)) {
			say(29, 36);
		} else {
			_facing = 4;
			_kevinTalking = true;
			say(29, !sculpture ? 1 : sculpture == 1 ? 11 : 19);
			say(28);
			say(29, !sculpture ? 2 : sculpture == 1 ? 12 : 20);
			beginRoomAnimationRange(436, 35 + sculpture, 46, 0, 33);
		}
		break;
	}
	case 339: // Keep the sanding block rather than offering it to Saturn.
		say(11, 2, true);
		break;
	case 400: // Introduce Brian to Saturn on the first visit.
		say(0);
		walkTo(Common::Point(687, 525), 5, true);
		deferTownAction(401);
		break;
	case 401: // Saturn introduces himself and asks why Brian is visiting.
		_kevinTalking = true;
		for (uint i = 1; i <= 9; ++i) {
			say(0, i, false, i % 2);
		}
		_town.kevinMet = 1;
		deferTownAction(402);
		break;
	case 402: // Continue the introductory conversation about the Hopi village.
		startKevinConversation(true);
		break;
	case 1407: // Saturn stops Brian from using the crane without permission.
		_kevinTalking = true;
		say(4, 13, false, 1);
		say(4, 2, false, 1);
		say(4, 3);
		say(4, 4, false, 1);
		say(4, 5);
		_town.craneTried = 1;
		beginRoomAnimationRange(406, 25, 16, 9, 0);
		break;
	case 1406: // Saturn resumes work after warning Brian.
		_kevinTalking = false;
		break;
	case 1408: { // Move the suspended statue over the catapult.
		_audio.playEffect(42);
		beginRoomAnimation(448, 20, 85);
		static const byte hand[] = {9};
		setRoomAnimationOverlay(25, 16, hand, ARRAYSIZE(hand));
		break;
	}
	case 1448: { // Lower the statue onto the catapult.
		say(4, 9);
		_audio.playEffect(43);
		beginRoomAnimation(409, 21, 90);
		_actionHoldSpeech = true;
		static const byte hand[] = {9};
		setRoomAnimationOverlay(25, 16, hand, ARRAYSIZE(hand));
		break;
	}
	case 1409: // The statue is ready to launch.
		_town.catapult = 2;
		_town.statueLowered = 1;
		_town.craneTried = 1;
		say(4, 10);
		beginRoomAnimationRange(410, 25, 16, 9, 0);
		return kBackgroundChanged;
	case 1410: // Step away from the crane controls.
		break;
	case 1414: { // The catapult releases its load.
		_audio.playEffect(26);
		Common::Array<byte> frames;
		if (!_town.catapult) {
			for (uint i = 41; i <= 46; ++i) {
				frames.push_back(i);
			}
		}
		for (uint i = _town.catapult ? 0 : 6; i <= 40; ++i) {
			frames.push_back(i);
		}
		beginRoomAnimation(415, 6, 48, frames.data(), frames.size(), false);
		break;
	}
	case 1415: // Follow the projectile to the bank.
		say(10, _town.catapult ? 1 : 10);
		deferTownAction(416);
		break;
	case 416: // Show the paint bucket or statue hitting the bank.
		showCatapultImpact();
		break;
	case 1416: // Return to the workshop after the impact.
		_catapultImpact = false;
		_town.catapult = _town.catapult == 2 ? 3 : 1;
		if (_town.catapult == 3) {
			say(10, 9);
		} else if (!_town.kevinAway) {
			_kevinTalking = true;
			say(10, 2, false, 1);
			say(10, 3);
			say(10, 4, false, 1);
			say(10, 5);
			say(10, 6, false, 1);
			deferTownAction(406);
		}
		return kBackgroundChanged;
	case 406: // End Saturn's reaction to Brian's experiment.
		_kevinTalking = false;
		break;
	case 420: // Call Saturn after discovering the wrench is missing from his tools.
		walkTo(Common::Point(211, 507), 3, true);
		say(16, 4);
		animateKevin(421, 27, 148, 0, 26);
		break;
	case 1421: // Saturn remembers that the wrench is in his pocket.
		setKevinPose(27, 148, 26, 26);
		_kevinTalking = true;
		say(16, 5, false, 1);
		say(16, 6);
		say(16, 7, false, 1);
		deferTownAction(422);
		break;
	case 422: { // Saturn throws the wrench and Brian fails to catch it.
		if (!_wrenchAnimation.load(_archive, 29, 7)) {
			error("Cannot load Runaway flying wrench");
		}
		_kevinAnimating = true;
		byte kevin[77], brian[77];
		for (uint i = 0; i < ARRAYSIZE(kevin); ++i) {
			kevin[i] = MIN<uint>(103, 32 + i);
			brian[i] = i < 53 ? MIN<uint>(15, i) : 16 + i - 53;
		}
		beginRoomAnimation(422, 27, 148, kevin, ARRAYSIZE(kevin));
		setRoomAnimationOverlay(28, 40, brian, ARRAYSIZE(brian));
		break;
	}
	case 1422: // Saturn tells Brian to look for the wrench in the street.
		setKevinPose(27, 148, 103, 103);
		say(16, 8, false, 1);
		say(16, 9);
		animateKevin(423, 27, 148, 109, 147);
		break;
	case 1423: // Leave the wrench in the trough until Brian retrieves it.
		setKevinPose(13, 112, 0, 106);
		_kevinIdle = 0;
		_kevinTalking = false;
		_town.wrenchThrown = 1;
		_town.wrenchPermission = 2;
		return kBackgroundChanged;
	case 430: // Collect the blowtorch.
		_town.blowtorchTaken = 1;
		_inventory.add(15);
		say(23, 1);
		return kBackgroundChanged;
	case 1333: // The oil can is filled with water.
		_inventory.town().oilCanFull = 1;
		_inventory.select(0);
		say(26, 3);
		break;
	case 1337: // The bottle contains exactly one liter of water.
		_inventory.town().bottleContents = 1;
		_inventory.select(0);
		say(30, 1);
		break;
	case 435: { // Brian hands over the helmet and Saturn puts it on.
		byte kevin[114], brian[114];
		for (uint i = 0; i < ARRAYSIZE(kevin); ++i) {
			kevin[i] = i < 13 ? 0 : i - 12;
			brian[i] = !i ? 0 : i < 34 ? i - 1 : i < 42 ? 32 : MIN<uint>(46, i - 9);
		}
		_kevinAnimating = true;
		beginRoomAnimation(435, 30, 107, kevin, ARRAYSIZE(kevin));
		setRoomAnimationOverlay(34, 47, brian, ARRAYSIZE(brian));
		break;
	}
	case 1435: // Saturn begins to feel the helmet's effects.
		setKevinPose(30, 107, 101, 101);
		say(28, 4, false, 1);
		animateKevin(445, 31, 96, 0, 95);
		break;
	case 1445: // Saturn pauses between his two attempts at meditation.
		animateKevin(447, 31, 96, 95, 79);
		break;
	case 1447: // Saturn tries receiving another telepathic message.
		animateKevin(449, 31, 96, 4, 78);
		break;
	case 1449: // Saturn lowers his hands after meditating.
		animateKevin(450, 31, 96, 4, 0);
		break;
	case 1450: // Saturn finds inspiration and decides to look for a new stone.
		setKevinPose(30, 107, 101, 101);
		say(28, 5, false, 1);
		if (!_town.oscarStrengthKnown) {
			say(28, 6);
			say(28, 11);
			say(90, 16, false, 1);
			say(80, 15);
			say(90, 15, false, 1);
			say(28, 12);
			say(90, 7, false, 1);
			_town.oscarStrengthKnown = 1;
		}
		animateKevin(446, 32, 121, 0, 120);
		break;
	case 1446: // Saturn leaves the workshop and Brian keeps the borrowed tools.
		_town.kevinAway = 1;
		_town.craneTried = 0;
		_kevinAnimating = _kevinTalking = false;
		_inventory.remove(13);
		say(28, 7);
		say(28, 8);
		_audio.playMusic(12 + _random.getRandomNumber(4), false);
		return kBackgroundChanged;
	case 1436: { // Saturn judges the sculpture's current finish.
		const byte sculpture = _inventory.town().sculpture;
		const uint first = !sculpture ? 3 : sculpture == 1 ? 13 : 21;
		const uint last = !sculpture ? 9 : sculpture == 1 ? 17 : 25;
		for (uint i = first; i <= last; ++i) {
			say(29, i, false, !(i % 2));
		}
		holdRoomAnimation(437, 35 + sculpture, 46, 33);
		break;
	}
	case 1437: // Brian lowers the sculpture after showing it to Saturn.
		beginRoomAnimationRange(438, 35 + _inventory.town().sculpture, 46, 33, 0);
		break;
	case 1438: // Refuse an unfinished sculpture, or place the decorated one by the door.
		if (_inventory.town().sculpture < 2) {
			if (!_inventory.town().sculpture) {
				_town.roughSculptureRefused = 1;
				say(29, 10);
			} else {
				_town.polishedSculptureRefused = 1;
				say(29, 18);
			}
			_kevinTalking = false;
		} else {
			walkTo(Common::Point(211, 507), 7, true);
			beginRoomAnimation(439, 33, 33);
		}
		break;
	case 1439: // Saturn accepts the sculpture and explains the fuel mixture.
		_town.sculptureTraded = 1;
		_inventory.remove(29);
		for (uint i = 26; i <= 31; ++i) {
			say(29, i, false, !(i % 2));
		}
		say(29, 32);
		deferTownAction(440);
		return kBackgroundChanged;
	case 440: // Collect the concentrate, one-liter bottle, and measuring tubes.
		_inventory.add(12);
		say(29, 33);
		say(29, 34);
		_inventory.add(25);
		_inventory.add(26);
		_inventory.add(24);
		say(29, 35);
		_kevinTalking = false;
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

void Scene6300::showCatapultImpact() {
	if (_impactBackground.empty() && !loadRunawaySurface(_archive, 22, 1024, 600, _impactBackground)) {
		error("Cannot load Runaway catapult background");
	}
	_catapultImpact = true;
	beginRoomAnimation(416, _town.catapult == 2 ? 24 : 23, _town.catapult == 2 ? 134 : 59);
}

void Scene6300::handleAnimationFrameHook() {
	static const AnimationFootstep leavingSteps[] = {
		{50, 560}, {57, 610}, {63, 660}, {70, 710}, {77, 760}, {84, 810},
		{91, 860}, {98, 910}, {106, 960}, {113, 1010}, {120, 1060}
	};
	if (_roomAction == 446) {
		playAnimationFootsteps(_actionFrames[_actionStep], 3, leavingSteps, ARRAYSIZE(leavingSteps));
		if (_actionFrames[_actionStep] == 38) {
			_audio.playEffectAt(33, Common::Point(607, 200), 255, 1);
		}
	}
	if (_roomAction == 445 || _roomAction == 449) {
		if (_actionFrames[_actionStep] == 14) {
			say(28, _roomAction == 445 ? 9 : 10, false, 1);
			_actionHoldSpeech = _actionLoopSpeech = true;
		} else if (_actionStep == _actionFrames.size() - 1) {
			_actionLoopSpeech = false;
		}
	} else if (_roomAction == 422 && _actionStep == 52) {
		_audio.playEffect(31);
	} else if (_roomAction == 416 && _actionStep == (_town.catapult == 2 ? 63 : 58)) {
		_audio.playEffect(_town.catapult == 2 ? 44 : 28);
	} else if ((_roomAction == 333 || _roomAction == 337) && _actionStep == 25) {
		_audio.playEffect(_roomAction == 333 ? 18 : 19, 230);
	}
}

void Scene6300::finishCustomConversation() {
	_kevinTalking = false;
}

void Scene6300::startKevinConversation(bool introduction) {
	startConversation(8);
	_kevinTalking = true;
	if (introduction) {
		say(80, 2);
		say(90, 2, false, 1);
		_town.hopiDiscussed = 1;
		_dialogueDepth = 1;
	} else {
		say(80);
		say(90, 0, false, 1);
	}
	static const DialogueChoice topics[][8] = {
		{{1, 0, 3, 1, 1, 1, 255}, {0, 0, 1, 2, 2, 2, 255},
		 {1, 0, 1, 3, 3, 1, 255}, {1, 0, 3, 4, 4, 1, 255},
		 {0, 1, 1, 5, 5, 1, 255}, {0, 0, 3, 6, 6, 1, 255}, {1, 0, 0, 7, 7, 0, 0}},
		{{1, 0, 3, 8, 8, 1, 255}, {1, 0, 1, 9, 9, 1, 255},
		 {1, 0, 3, 10, 10, 1, 255}, {1, 0, 2, 11, 11, 0, 0}},
		{{1, 1, 3, 12, 12, 10, 255}, {1, 1, 3, 13, 13, 3, 255},
		 {1, 1, 1, 14, 14, 4, 255}, {0, 1, 3, 15, 15, 11, 255}, {1, 0, 2, 16, 16, 0, 0}},
		{{1, 0, 3, 17, 17, 1, 255}, {1, 0, 3, 18, 18, 1, 255},
		 {1, 0, 3, 19, 19, 1, 255}, {1, 0, 4, 20, 20, 0, 0}},
		{{1, 0, 4, 21, 21, 0, 255}}
	};
	static const byte pages[] = {0, 10, 11, 20, 21};
	for (uint i = 0; i < ARRAYSIZE(pages); ++i) {
		memcpy(_dialogue[pages[i]], topics[i], sizeof(topics[i]));
	}
	_dialogue[0][1].enabled = !_town.hopiDiscussed;
	_dialogue[0][2].enabled = _town.hopiDiscussed;
	_dialogue[0][5].enabled = _town.catapult != 0;
	updateKevinTopics();
}

void Scene6300::updateKevinTopics() {
	_dialogue[11][0].enabled = _town.waterRequested && !_town.waterPermission;
	_dialogue[11][1].enabled = _town.wrenchNeeded && !_town.wrenchPermission;
	_dialogue[11][2].enabled = _town.fuelNeeded && !_town.fuelTradeKnown;
	_dialogue[11][3].enabled = _town.mineVisited && !_town.oscarStrengthKnown;
	_dialogue[0][4].enabled = _dialogue[11][0].enabled || _dialogue[11][1].enabled ||
		_dialogue[11][2].enabled || _dialogue[11][3].enabled;
}

void Scene6300::handleCustomDialogueChoice(byte effect) {
	switch (effect) {
	case 2: // Saturn has heard of the Hopi village.
		_town.hopiDiscussed = 1;
		break;
	case 3: // Saturn permits Brian to borrow a wrench.
		_town.wrenchPermission = 1;
		break;
	case 4: // Learn that Saturn wants a work of art in exchange for gasoline.
		_town.fuelTradeKnown = 1;
		break;
	case 10: // Saturn permits Brian to take water.
		_town.waterPermission = 1;
		break;
	case 11: // Learn how Oscar helps Saturn move heavy stones.
		_town.oscarStrengthKnown = 1;
		break;
	default:
		break;
	}
	updateKevinTopics();
}

} // End of namespace Runaway
} // End of namespace Hollywood
