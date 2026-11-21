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
#include "hollywood/runaway/scene4090.h"
#include "hollywood/runaway/strings.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{1100, 204, 1, {302, 301}, {0, 3}, {1023, 251}, 5}, // 1: Exhibition hall.
	{826, 442, 3, {303, 304}, {1, 0}, {838, 513}}, // 2: Statue.
	{572, 420, 1, {305, 306}, {1, 0}, {657, 380}}, // 3: Statue.
	{572, 420, 1, {307, 308}, {1, 1}, {657, 380}}, // 4: Mayan objects.
	{316, 382, 7, {309, 310}, {0, 0}, {232, 353}}, // 5: Mayan arch.
	{532, 238, 0, {311, 312}, {0, 0}, {510, 208}}, // 6: Mayan arch.
	{280, 491, 7, {313, 314}, {1, 0}, {197, 415}}, // 7: Mayan object.
	{302, 426, 7, {315, 316}, {1, 0}, {232, 353}}, // 8: Mayan object.
	{316, 382, 7, {317, 318}, {1, 0}, {255, 313}}, // 9: Mayan object.
};

const RunawayResponse kResponses[] = {
	{kMuseumExhibition, 302, 1, 0}, // Look at the exhibition hall exit.
	{kMuseumExhibition, 303, 2, 0}, // Look at the right statue.
	{kMuseumExhibition, 304, 3, 0}, // Take the right statue.
	{kMuseumExhibition, 305, 4, 0}, // Look at the left statue.
	{kMuseumExhibition, 306, 3, 0}, // Take the left statue.
	{kMuseumExhibition, 307, 5, 0}, // Look at the Mayan offerings.
	{kMuseumExhibition, 309, 7, 0}, // Look at the near Mayan arch.
	{kMuseumExhibition, 310, 8, 0}, // Use the near Mayan arch.
	{kMuseumExhibition, 311, 9, 0}, // Look at the far Mayan arch.
	{kMuseumExhibition, 312, 8, 0}, // Use the far Mayan arch.
	{kMuseumExhibition, 313, 10, 0}, // Look at the first Mayan object.
	{kMuseumExhibition, 314, 11, 0}, // Take the first Mayan object.
	{kMuseumExhibition, 315, 12, 0}, // Look at the second Mayan object.
	{kMuseumExhibition, 316, 11, 0}, // Take the second Mayan object.
	{kMuseumExhibition, 317, 13, 0}, // Look at the third Mayan object.
	{kMuseumExhibition, 318, 11, 0}, // Take the third Mayan object.
};

const RunawayExit kExits[] = {
	{kMuseumExhibition, 301, kMuseumHall, 1400, 600, false, false}, // Return to the exhibition hall.
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene4090 = {
	kMuseumExhibition, 1124, 600, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {70, 70, 90}, {60, 60, 80}, {50, 50, 70}},
	Common::Point(881, 392),
	kResponses, ARRAYSIZE(kResponses),
	nullptr, 0,
	kExits, ARRAYSIZE(kExits)
};

namespace Runaway {

Scene4090::Scene4090(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

void Scene4090::setCameraX(int x) {
	// The extra 100 columns are for the exit path, beyond the fixed view.
	_camera.x = 0;
}

bool Scene4090::applyCustomBackgroundPatches() {
	return !museumChapter() || !_museum.coffeeBeansTaken || applyPatch(4);
}

void Scene4090::runCustomEntrySequence() {
	if (!_museum.exhibitionVisited) {
		_museum.exhibitionVisited = 1;
		say(0);
	}
}

RunawayRoomScene::ActionResult Scene4090::dispatchCustomSceneAction(uint16 action) {
	if (!museumChapter()) {
		return kUnhandled;
	}
	switch (action) {
	case 307: // Look at the Mayan offerings.
	case 308: // Take the bowl of coffee beans.
		if (_museum.coffeeBeansTaken) {
			say(action == 307 ? 5 : 6, action == 307 ? 2 : 3);
		} else if (_museum.susan == 1 && _museum.coffeeSeen) {
			say(6, 1);
			_inventory.add(14);
			_museum.coffeeBeansTaken = 1;
			say(6, 2);
			return kBackgroundChanged;
		} else {
			say(action == 307 ? 5 : 6, action == 307 ? _museum.mayanObjectsSeen : 0);
			if (action == 307) {
				_museum.mayanObjectsSeen = 1;
			}
		}
		return kHandled;
	case 313: // Look at the first Mayan object.
	case 315: // Look at the second Mayan object.
	case 317: { // Look at the third Mayan object.
		const uint object = (action - 313) / 2;
		say(object == 0 ? 10 : object == 1 ? 12 : 13, _museum.exhibitSeen[object]);
		_museum.exhibitSeen[object] = 1;
		return kHandled;
	}
	default:
		return kUnhandled;
	}
}

float Scene4090::actorHeight() const {
	const int y = _position.y;
	return MIN(y, 390) * 0.2f + 122;
}

Common::U32String Scene4090::hotspotName(byte id) const {
	if (id == 4 && _museum.mayanObjectsSeen) {
		return _strings.text(RunawayStrings::kMayanBowlsAndJars);
	}
	if (id >= 7 && id <= 9 && _museum.exhibitSeen[id - 7]) {
		static const RunawayStrings::TextId kIdentifiedNames[] = {
			RunawayStrings::kExhibitBucket, RunawayStrings::kTeapot, RunawayStrings::kExhibitBowl
		};
		return _strings.text(kIdentifiedNames[id - 7]);
	}
	return RunawayRoomScene::hotspotName(id);
}

void Scene4090::setupActorDepth(RunawayActor::RenderParams &params) const {
	params.depthY[1] = 599;
	params.depthY[2] = 374;
}

} // End of namespace Runaway
} // End of namespace Hollywood
