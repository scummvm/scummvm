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
#include "hollywood/runaway/scene6250.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{516, 314, 1, {301, 302}, {0, 0}, {564, 286}}, // 1: Statue.
	{516, 314, 1, {305, 306}, {0, 1}, {564, 286}, 5}, // 2: Hole.
	{462, 312, 7, {303, 304}, {0, 1}, {0, 0}}, // 3: Safe.
	{0, 0, 4, {307, 307}, {0, 0}, {512, 599}}, // 4: Rubble.
	{462, 312, 7, {308, 309}, {0, 1}, {512, 599}} // 5: Combination dial.
};

const RunawayResponse kResponses[] = {
	{6250, 301, 1, 0}, // Examine the statue that broke through the bank floor.
	{6250, 302, 2, 0}, // Refuse to move the statue by hand.
	{6250, 303, 3, 0}, // Examine the antique safe.
	{6250, 305, 5, 0}, // Look up at the damaged floor.
	{6250, 307, 6, 0}, // Refuse to venture into the unstable rubble.
	{6250, 308, 7, 0}  // Examine the combination dial.
};

const RunawayItemAction kItemActions[] = {
	{6250, 7, 3, 310}, {6250, 7, 5, 310}
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene6250 = {
	6250, 1024, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {40, 40, 40}, {70, 70, 70}}, Common::Point(516, 314),
	kResponses, ARRAYSIZE(kResponses), kItemActions, ARRAYSIZE(kItemActions), nullptr, 0
};

namespace Runaway {

Scene6250::Scene6250(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

bool Scene6250::loadCustomResources() {
	return _safeAnimation.load(_archive, 11, 145) && loadForeground(0, 4);
}

void Scene6250::resumeAmbientAudio(bool entering) {
	_audio.playMusic(21);
}

void Scene6250::runCustomEntrySequence() {
	if (_entryScene == 6251 || _entryScene == 6252) {
		_position = Common::Point(462, 312);
		_walkPath.reset(_position);
		_facing = 7;
		const bool stethoscope = _entryScene == 6252;
		beginRoomAnimationRange(401, stethoscope ? 9 : 8, stethoscope ? 36 : 24, stethoscope ? 35 : 23, 0);
	} else {
		_facing = 5;
		beginRoomAnimation(400, 6, 79);
	}
}

void Scene6250::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	const uint frame = _safeAnimating && drawingRoomAnimation() ?
		_actionFrames[MIN<uint>(_actionStep, _actionFrames.size() - 1)] : _safeFrame;
	_safeAnimation.draw(viewport, frame, 0, 0);
}

void Scene6250::drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) {
	drawForeground(viewport, 0);
}

void Scene6250::advanceCustomGameplayLoop(uint32 delta) {
	if (_safeAnimating && drawingRoomAnimation()) {
		_safeFrame = _actionFrames[MIN<uint>(_actionStep, _actionFrames.size() - 1)];
	}
	if (_roomAction == 405 && !_actionFrames.empty()) {
		_actionFrames[0] = 44 + speechFrame();
		_safeFrame = _actionFrames[0];
	}
}

void Scene6250::animateSafe(uint16 action, uint first, uint last) {
	_safeAnimating = true;
	beginRoomAnimationRange(action, 10, 112, first, last);
}

void Scene6250::findWalkPath(Common::Point target) {
	walkTown(target);
}

bool Scene6250::adjustCustomWalkTargetToFloorMask(Common::Point &target) const {
	target.x = CLIP<int>(target.x, 462, 516);
	return false;
}

RunawayRoomScene::ActionResult Scene6250::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 304: // Try the handle of the locked safe.
		beginRoomAnimation(404, 7, 33);
		break;
	case 1404: // The handle will not turn without the combination.
		say(4);
		break;
	case 306: // Climb back through the hole to the bank.
		beginRoomAnimation(406, 5, 61);
		break;
	case 1406: // Return upstairs after climbing out of the basement.
		leaveTownRoom(6191);
		break;
	case 309: // Lean toward the dial to try the combination.
		beginRoomAnimation(409, 8, 24);
		break;
	case 1409: // Open the dial close-up without the stethoscope.
		leaveTownRoom(6390);
		break;
	case 310: // Listen to the safe's tumblers with the stethoscope.
		if (_town.safeOpened) {
			say(9, 11);
		} else {
			if (!_town.stethoscopeTried) {
				_town.stethoscopeTried = 1;
				say(9);
			}
			_inventory.select(0);
			beginRoomAnimation(410, 9, 36);
		}
		break;
	case 1410: // Open the dial close-up with amplified tumbler sounds.
		leaveTownRoom(6391);
		break;
	case 1400: // Brian reaches the bank basement for the first time.
		if (!_town.basementVisited) {
			_town.basementVisited = 1;
			say(0);
		}
		break;
	case 1401: // After entering the combination, open the safe or report failure.
		if (_town.safeProgress == 3) {
			animateSafe(402, 0, 44);
		} else if (_entryScene == 6252) {
			say(9, 7);
		} else if (!_town.safeOpened) {
			say(8, 4);
		}
		break;
	case 1402: // Search the open safe for the map of the mine.
		_safeAnimating = false;
		_safeFrame = 44;
		say(_town.safeOpened ? 8 : 9, _town.safeOpened ? 5 : 8);
		holdRoomAnimation(405, 10, 112, 44);
		break;
	case 1405: // Reach into the safe to retrieve the map.
		if (!_town.safeOpened) {
			animateSafe(403, 56, 64);
		} else {
			animateSafe(407, 65, 111);
		}
		break;
	case 1403: // Keep the map and read the route to the Hopi village.
		_safeAnimating = false;
		_town.safeOpened = 1;
		_inventory.add(17);
		_audio.playMusicEffect(39);
		say(9, 9);
		holdRoomAnimation(408, 10, 112, 64);
		break;
	case 1408: { // Show the map and read the route to the Hopi village.
		typedef RunawaySequence S;
		static const S::Command commands[] = {
			{S::kImage, 146}, {S::kSpeech, 9, 10, 10}, {S::kWaitInput}, {S::kEnd}
		};
		_sequence.start(commands, _text, _now, 'F');
		break;
	}
	case 1407: // Reset the dial after closing the safe.
		_safeAnimating = false;
		_safeFrame = 0;
		_town.safeProgress = 0;
		_town.safeDial = 69;
		say(9, 12);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

void Scene6250::finishCustomFilm() {
	animateSafe(407, 65, 111);
}

void Scene6250::handleAnimationFrameHook() {
	if (_roomAction == 400 && _actionStep == 7) {
		_audio.playEffect(45, 153);
	} else if (_roomAction == 402 && _actionStep == 24) {
		_audio.playEffect(63);
	} else if (_roomAction == 407 && _actionStep == 23) {
		_audio.playEffect(66);
	}
}

} // End of namespace Runaway
} // End of namespace Hollywood
