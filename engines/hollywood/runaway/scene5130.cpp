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
#include "hollywood/runaway/scene5130.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
};

enum Speaker { kBrian, kLula, kMariola, kCarla };
const Speaker kSpeakers[] = {
	kBrian, kLula, kMariola, kBrian, kMariola, // Explain Mariola's part in the rescue.
	kBrian, kLula, kBrian, kLula, kBrian, kLula, // Distribute the transmitters and brief Lula.
	kBrian, kBrian, kCarla, kBrian, kCarla, kLula, kCarla, // Get Carla's attention.
	kBrian, kMariola, kBrian // Send everyone to their positions.
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene5130 = {
	5130, 1024, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{80, 75, 70}, {80, 75, 70}, {80, 75, 70}, {80, 75, 70}, {80, 75, 70},
	 {80, 75, 70}, {80, 75, 70}, {80, 75, 70}, {80, 75, 70}, {80, 75, 70}},
	Common::Point(205, 599),
	nullptr, 0,
	nullptr, 0,
	nullptr, 0
};

namespace Runaway {

Scene5130::Scene5130(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

bool Scene5130::loadCustomResources() {
	return _groupAnimation.load(_archive, 4, 71) &&
		_carlaAnimation.load(_archive, 5, 33);
}

float Scene5130::actorHeight() const {
	return 360;
}

void Scene5130::runCustomEntrySequence() {
	_rescuePlan = RescuePlan();
	_rescuePlan.tick = _now;
	_facing = 1;
	_roomAction = 5130;
}

void Scene5130::updateRescuePlan() {
	RescuePlan &plan = _rescuePlan;
	const Speaker next = plan.nextLine <= ARRAYSIZE(kSpeakers) ? kSpeakers[plan.nextLine - 1] : kBrian;
	if (_now - plan.tick >= 80) {
		plan.tick = _now;
		if (plan.groupFrame < 58) {
			if (plan.groupFrame != 32 && plan.groupFrame != 57) {
				++plan.groupFrame;
			} else if (!speaking() && (next == kLula || next == kMariola)) {
				plan.groupFrame = next == kLula ? 64 : 58;
			} else {
				plan.groupFrame = 33;
			}
		}
		if (plan.carlaFrame < 15) {
			++plan.carlaFrame;
		} else if (plan.carlaFrame == 15) {
			plan.carlaFrame = !speaking() && next == kCarla ? 16 : 0;
		} else if (plan.carlaFrame < 20) {
			++plan.carlaFrame;
		}
		plan.blink = !_random.getRandomNumber(14);
	}
	if (speaking()) {
		return;
	}
	if (plan.nextLine > ARRAYSIZE(kSpeakers)) {
		_roomAction = 0;
		leaveDesertRoom(5022);
		_transition = _hospitalExit;
		return;
	}
	if ((plan.nextLine == 7 || plan.nextLine == 20) && plan.groupFrame < 58) {
		return;
	}
	if (plan.nextLine == 14 && plan.carlaFrame < 20) {
		return;
	}
	if (plan.nextLine == 13) {
		if (!plan.pauseUntil) {
			plan.pauseUntil = _now + 3000;
		}
		if ((int32)(_now - plan.pauseUntil) < 0) {
			return;
		}
	}
	if (next == kLula || next == kMariola) {
		plan.groupFrame = next == kLula ? 64 : 58;
	}
	switch (plan.nextLine) {
	case 6: // Distribute the transmitters.
	case 19: // Reassure everyone about Carla's flying.
		plan.groupFrame = 33;
		break;
	case 12: // Turn to Carla.
	case 21: // Finish the briefing.
		plan.groupFrame = 0;
		break;
	default:
		break;
	}
	say(0, plan.nextLine++, false, next);
}

void Scene5130::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	const RescuePlan &plan = _rescuePlan;
	uint carla = plan.carlaFrame;
	if (carla == 20) {
		carla += speaking() && speaker() == kCarla ? speechFrame() : plan.blink ? 6 : 0;
	}
	uint group = plan.groupFrame;
	if (speaking() && (speaker() == kLula || speaker() == kMariola)) {
		group = (speaker() == kLula ? 64 : 58) + speechFrame() % 6;
	}
	_carlaAnimation.draw(viewport, carla, 0, 0);
	_groupAnimation.draw(viewport, group, 0, 0);
}

bool Scene5130::updateCustomSequence() {
	updateRescuePlan();
	return true;
}

} // End of namespace Runaway
} // End of namespace Hollywood
