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

#include "common/util.h"
#include "hollywood/runaway/scene6090.h"
#include "hollywood/runaway/strings.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{910, 599, 3, {301, 302}, {0, 3}, {1023, 599}, 6}, // 1: Oil well area.
	{686, 510, 0, {303, 304}, {0, -1}, {668, 390}, 4}, // 2: Entrance to the house.
	{846, 509, 0, {305, 306}, {0, 0}, {837, 455}}, // 3: Sign.
	{676, 510, 7, {307, 308}, {0, 1}, {517, 463}, 3}, // 4: Big dude.
	{244, 416, 7, {309, 310}, {0, 0}, {216, 394}}, // 5: Ladder.
	{180, 446, 0, {312, 313}, {0, 1}, {180, 425}}, // 6: Vessel.
	{50, 599, 5, {315, 316}, {0, 3}, {0, 599}, 7} // 7: Path.
};

const RunawayResponse kResponses[] = {
	{6090, 301, 13, 0}, // Look toward the abandoned oil well.
	{6090, 303, 2, 0},  // Examine the curtain across the doorway.
	{6090, 305, 4, 0},  // Read Mama Dorita's sign.
	{6090, 309, 7, 0},  // Examine the ladder.
	{6090, 315, 1, 0}   // Look along the deserted path.
};

const RunawayItemAction kItemActions[] = {
	{6090, 27, 4, 314}, // Offer untreated tobacco leaves.
	{6090, 28, 4, 311}  // Give Oscar chewing tobacco.
};



} // End of anonymous namespace

const RunawayRoom kRunawayScene6090 = {
	6090, 1024, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {100, 95, 90}, {80, 75, 70}}, Common::Point(838, 551),
	kResponses, ARRAYSIZE(kResponses), kItemActions, ARRAYSIZE(kItemActions), nullptr, 0
};

namespace Runaway {

Scene6090::Scene6090(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game), _oscar(true) {}

bool Scene6090::loadCustomResources() {
	return _oscarAnimation.load(_archive, 6, 113) && loadForeground(0, 4);
}

bool Scene6090::applyCustomBackgroundPatches() {
	return _town.vessel != 2 || applyPatch(5);
}

void Scene6090::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		if (_maps[256 + i] == 6 && _town.vessel == 2) {
			_maps[256 + i] = 0;
		}
	}
}

void Scene6090::prepareCustomGameplayLoop() {
	_oscar.resume(_now);
}

void Scene6090::runCustomEntrySequence() {
	if (!_town.oscarMet) {
		walkTo(Common::Point(676, 510), 7);
		deferTownAction(400);
	} else {
		walkTo(Common::Point(837, 550), 7);
	}
}

void Scene6090::advanceCustomGameplayLoop(uint32 delta) {
	_oscar.update(_now, _random, _audio);
}

void Scene6090::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	drawOscar(viewport, false);
}

void Scene6090::drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) {
	drawOscar(viewport, true);
}

float Scene6090::actorHeight() const {
	return _position.y < 510 ? _position.y * 0.840707958f - 38.7610626f :
		_position.y * 0.730337083f + 17.5280895f;
}

Common::U32String Scene6090::hotspotName(byte id) const {
	if (id == 4 && _town.oscarTobacco) {
		return _strings.hotspotName(_room, 8);
	}
	return RunawayRoomScene::hotspotName(id);
}

RunawayRoomScene::ActionResult Scene6090::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 302: // Return to the wide view of the house.
		leaveTownRoom(6131);
		break;
	case 304: // Oscar will not allow Brian into Mama Dorita's house yet.
		_facing = 4;
		if (_town.oscarTobacco == 1) {
			say(3, _random.getRandomBit());
		} else {
			say(11, 0, true);
		}
		break;
	case 306: // Refuse to remove the sign.
		if (_town.oscarTobacco == 1) {
			say(5);
		} else {
			say(14, _random.getRandomNumber(2), true);
		}
		break;
	case 307: { // Look at Oscar.
		const byte variation = _random.getRandomNumber(2);
		if (variation == 2) {
			_facing = 4;
		}
		say(6, variation);
		break;
	}
	case 308: // Talk to Oscar.
		startOscarConversation();
		break;
	case 310: // Refuse to sneak into the house via the ladder.
		if (_town.oscarTobacco == 1) {
			say(8);
		} else {
			say(14, _random.getRandomNumber(2), true);
		}
		break;
	case 311: // Offer the prepared chewing tobacco.
		_oscar.setTalking(true);
		_audio.stopEffect(1);
		say(9);
		say(9, 1, false, 1);
		say(9, 2);
		deferTownAction(411);
		break;
	case 411: { // Oscar and Brian exchange the tobacco.
		byte oscar[35], brian[35];
		for (uint i = 0; i < ARRAYSIZE(oscar); ++i) {
			oscar[i] = i < 6 || i == 34 ? 0 : i - 5;
			brian[i] = i ? MIN<uint>(23, i - 1) : 0;
		}
		beginRoomAnimation(411, 7, 29, oscar, ARRAYSIZE(oscar));
		setRoomAnimationOverlay(8, 24, brian, ARRAYSIZE(brian));
		break;
	}
	case 1411: // Complete the tobacco exchange with Oscar.
		_inventory.remove(28);
		_inventory.select(0);
		_town.oscarTobacco = 2;
		say(9, 3, false, 1);
		say(9, 4);
		say(9, 5, false, 1);
		deferTownAction(412);
		break;
	case 412: // Step back after giving Oscar the tobacco.
		_oscar.setTalking(false);
		walkTo(Common::Point(718, 543), 4);
		say(9, 6);
		break;
	case 312: // Examine the clay vessel.
		say(10);
		_town.vessel = 1;
		break;
	case 313: // Take the vessel while Oscar is occupied.
		if (!_town.vessel) {
			dispatchCustomSceneAction(312);
		}
		if (_town.oscarTobacco == 1) {
			say(11);
		} else {
			say(18, 0, true);
		}
		deferTownAction(413);
		break;
	case 413: // Collect the clay vessel.
		_town.vessel = 2;
		_inventory.add(31);
		say(11, 1);
		return kBackgroundChanged;
	case 314: // Oscar chews tobacco; he does not smoke the untreated leaves.
		_oscar.setTalking(true);
		_audio.stopEffect(1);
		say(12);
		say(12, 1, false, 1);
		_town.rawTobaccoRefused = 1;
		deferTownAction(401);
		break;
	case 316: // Return directly to the overview.
		leaveTownRoom(6350);
		break;
	case 400: // First meeting at Mama Dorita's door.
		_oscar.setTalking(true);
		_audio.stopEffect(1);
		for (uint i = 0; i <= 14; ++i) {
			say(0, i, false, (i % 2) == 0);
		}
		_town.oscarMet = 1;
		deferTownAction(401);
		break;
	case 401: // Oscar returns to his idle animation.
		_oscar.startIdle();
		break;
	case 402: // Oscar has agreed to move the mine's rock.
		leaveTownRoom(6082);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

uint16 Scene6090::dialoguePlayerCue() const {
	return 81;
}

uint16 Scene6090::dialogueResponseCue() const {
	return 91;
}

void Scene6090::handleCustomDialogueChoice(byte effect) {
	if (effect == 3) { // Remember that Oscar has already refused the favour.
		_town.oscarHelpRefused = 1;
	} else if (effect == 5) { // A gift of chewing tobacco earns his help.
		say(81, 7);
		_afterConversation = 402;
	}
}

void Scene6090::drawOscar(Graphics::ManagedSurface &viewport, bool foreground) {
	if ((_position.y < 460) == foreground && !drawingRoomAnimation()) {
		_oscarAnimation.draw(viewport, _oscar.frame(speaker(), speechFrame()), _camera.x, -_camera.y);
	}
	if (foreground) {
		drawForeground(viewport, 0);
	}
}

void Scene6090::startOscarConversation() {
	_conversation = 6;
	_dialogueDepth = _dialogueNode = _dialogueIntro = 0;
	_dialogueEnding = false;
	_choices.clear();
	_choiceIndices.clear();
	memset(_dialogue, 0, sizeof(_dialogue));
	_afterConversation = 401;
	_oscar.setTalking(true);
	_audio.stopEffect(1);
	say(81, _town.oscarTobacco == 1 ? 0 : 8);
	say(91, 0, false, 1);

	// Original page 81: the favour becomes available after Kevin names Oscar.
	static const DialogueChoice topics[] = {
		{1, 0, 3, 1, 1, 1, 255}, {1, 0, 3, 2, 2, 1, 255},
		{1, 0, 3, 3, 3, 1, 255}, {0, 0, 1, 5, 5, 2, 255},
		{1, 0, 0, 4, 4, 1, 0}
	};
	memcpy(_dialogue[0], topics, sizeof(topics));
	_dialogue[0][1].enabled = _town.oscarTobacco < 2;
	_dialogue[0][3].enabled = _town.oscarStrengthKnown && !_town.rockMoved;
	_dialogue[10][0] = {1, 0, 2, 6, (byte)(_town.oscarHelpRefused ? 7 : 6), 3, 255};
	if (_town.oscarTobacco == 2) {
		_dialogue[10][0].transition = 0;
		_dialogue[10][0].response = 8;
		_dialogue[10][0].disable = 5;
	}
}

} // End of namespace Runaway
} // End of namespace Hollywood
