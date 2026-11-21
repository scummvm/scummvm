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
#include "hollywood/runaway/scene8263.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{252, 492, 0, {301, 302}, {0, 1}, {233, 165}, 5}, // 1: Window.
	{223, 483, 1, {303, 304}, {0, 0}, {300, 443}}, // 2: Boiler.
	{287, 505, 1, {305, 306}, {0, 1}, {340, 448}}, // 3: Steam outlet lever.
	{287, 505, 1, {307, 308}, {0, 1}, {340, 448}}, // 4: Pressure wheel.
	{287, 505, 1, {311, 312}, {0, 0}, {340, 448}}, // 5: Gauges.
	{0, 0, 0, {314, 315}, {0, 0}, {265, 445}}, // 6: Lever.
	{0, 0, 1, {316, 317}, {0, 0}, {340, 448}} // 7: Lever.
};

const RunawayResponse kResponses[] = {
	{8263, 301, 21, 0}, // Examine the window.
	{8263, 303, 23, 2}, // The boiler is still burning.
	{8263, 304, 24, 1}, // Leave the red-hot boiler alone.
	{8263, 305, 25, 2}, // Examine the familiar steam lever.
	{8263, 307, 27, 1}, // Identify the pressure wheel.
	{8263, 308, 28, 3}, // Do not raise the pressure any further.
	{8263, 310, 30, 1}, // The boiler already has enough fuel.
	{8263, 311, 31, 0}, // Examine the pressure and water gauges.
	{8263, 312, 32, 0}, // Leave the gauges alone.
	{8263, 314, 34, 0}, // Identify the driving lever.
	{8263, 315, 35, 1}, // Refuse to drive the derailed locomotive.
	{8263, 317, 37, 0}  // Refuse to drain the tank.
};

const RunawayItemAction kItemActions[] = {
	{8263, 0, 2, 310, false} // The lit boiler needs no more fuel.
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene8263 = {
	8263, 1024, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {60, 60, 60}}, Common::Point(252, 492),
	kResponses, ARRAYSIZE(kResponses), kItemActions, ARRAYSIZE(kItemActions), nullptr, 0
};

namespace Runaway {

Scene8263::Scene8263(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	Scene6263(audio, inventory, strings, game) {}

void Scene8263::prepareCustomGameplayLoop() {
	// The chapter starts with a full tank and the boiler at high pressure.
	_boilerFrame = 41;
	_boilerTick = _now;
	_locomotiveGaugeFrames[0] = _random.getRandomNumber(30);
	_locomotiveGaugeFrames[3] = _random.getRandomNumber(30);
	_locomotiveGaugeFrames[1] = _locomotiveGaugeFrames[2] = 30;
}

void Scene8263::runCustomEntrySequence() {
	if (_entryScene == 8264) {
		animateSteamLever(401, true);
	} else {
		beginRoomAnimation(400, 24, 24, nullptr, 0, true, 60);
	}
}

void Scene8263::resumeAmbientAudio(bool entering) {
	resumeFinaleOutdoorAudio(entering, 36, 51);
	_audio.playEffect(33, 178, 1, true);
}

void Scene8263::advanceCustomGameplayLoop(uint32 delta) {
	updateFinaleOutdoorAudio();
	if (advanceBoiler()) {
		for (uint i = 0; i < 4; i += 3) {
			const uint step = i ? 2 : 1;
			byte &frame = _locomotiveGaugeFrames[i];
			if (_random.getRandomBit()) {
				if (frame <= 30 - step) {
					frame += step;
				}
			} else if (frame > 2 * step - 1) {
				frame -= step;
			}
		}
		_locomotiveGaugeFrames[1] = _locomotiveGaugeFrames[1] == 30 && !_random.getRandomNumber(9) ? 28 : 30;
		_locomotiveGaugeFrames[2] = _locomotiveGaugeFrames[2] == 30 && !_random.getRandomNumber(14) ? 28 : 30;
	}
}

void Scene8263::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	drawBoiler(viewport, !drawingRoomAnimation() || (_roomAction != 306 && _roomAction != 401));
}

Common::U32String Scene8263::hotspotName(byte id) const {
	return RunawayRoomScene::hotspotName(id);
}

RunawayRoomScene::ActionResult Scene8263::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 302: // Climb out through the window.
		beginRoomAnimation(302, 25, 29, nullptr, 0, true, 60);
		break;
	case 1302: // Finish climbing out in the exterior view.
		leaveRoom(8261);
		break;
	case 306: // Release steam to dislodge the sheriff's badge.
		animateSteamLever(306, false, !_finale.locomotiveBadge);
		break;
	case 1306: // Show the steam burst only while the badge is still in the smokestack.
		if (!_finale.locomotiveBadge) {
			leaveRoom(8262);
		} else {
			say(26, 3);
		}
		break;
	case 316: // Identify the drainage lever, retaining the earlier visit's discovery.
		say(36, _town.drainSeen);
		_town.drainSeen = 1;
		break;
	case 1400: // Finish climbing into the cab.
		_position = Common::Point(252, 492);
		_walkPath.reset(_position);
		_facing = 4;
		break;
	case 1401: // Release the lever after watching the steam burst outside.
		_position = Common::Point(287, 505);
		_walkPath.reset(_position);
		_facing = 1;
		say(26, 2);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

void Scene8263::handleAnimationFrameHook() {
	if (_roomAction == 400 && _actionStep == 13) {
		_audio.playEffect(21, 204);
	} else if (_roomAction == 302 && _actionStep == 10) {
		_audio.playEffect(73, 204);
	} else if ((_roomAction == 306 && (_actionStep == 16 || _actionStep == 43)) ||
			(_roomAction == 401 && _actionStep == 3)) {
		_audio.playEffect(29);
	}
}

} // End of namespace Runaway
} // End of namespace Hollywood
