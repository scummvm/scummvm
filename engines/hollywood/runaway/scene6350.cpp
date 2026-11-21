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

#include "common/file.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "hollywood/runaway/scene6350.h"
#include "hollywood/runaway/strings.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{0, 0, 0, {1, 301}, {0, 0}, {0, 0}, 7}, // 1: Entrance to the mine.
	{0, 0, 0, {1, 302}, {0, 0}, {0, 0}, 5}, // 2: Unknown place.
	{0, 0, 0, {1, 303}, {0, 0}, {0, 0}, 6}, // 3: Unknown place.
	{0, 0, 0, {1, 304}, {0, 0}, {0, 0}, 5}, // 4: Unknown place.
	{0, 0, 0, {1, 305}, {0, 0}, {0, 0}, 4}, // 5: Unknown place.
	{0, 0, 0, {1, 306}, {0, 0}, {0, 0}, 7} // 6: Locomotive.
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene6350 = {
	6350, 1024, 600, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}},
	Common::Point(512, 500), nullptr, 0, nullptr, 0, nullptr, 0
};

namespace Runaway {

Scene6350::Scene6350(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

bool Scene6350::usesAlternateActor() const {
	return false;
}

bool Scene6350::applyCustomBackgroundPatches() {
	return !_town.arrived || (applyPatch(5) && applyPatch(7));
}

void Scene6350::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		// The route to the Hopi village has not been discovered yet.
		if (_maps[256 + i] == 5) {
			_maps[256 + i] = 0;
		}
	}
}

void Scene6350::resume() {
	if (_entering && !_town.arrived) {
		_entering = false;
		startTownFilm(1);
		return;
	}
	RunawayRoomScene::resume();
}

void Scene6350::finishCustomFilm() {
	if (_townFilm == 1) {
		startTownFilm(2);
		return;
	}
	_townFilm = 0;
	_town.arrived = 1;
	if (!rebuildBackground()) {
		error("Cannot update Runaway chapter 4 map");
	}
	_audio.playMusic(11);
	say(0);
	leaveTownRoom(6220);
}

bool Scene6350::drawCustomComposite(Graphics::ManagedSurface &viewport) {
	return true;
}

bool Scene6350::shouldUpdateActor() const {
	return false;
}

Common::U32String Scene6350::hotspotName(byte id) const {
	if (id == 2 && _town.arrived) {
		return _strings.hotspotName(_room, 7);
	}
	if (id == 3 && _town.houseVisited) {
		return _strings.hotspotName(_room, 8);
	}
	if (id == 4 && (_town.craterSeen || _town.craterVisited)) {
		return _strings.hotspotName(_room, 9);
	}
	return RunawayRoomScene::hotspotName(id);
}

bool Scene6350::shouldPlayGameplayClickPath() const {
	return false;
}

RunawayRoomScene::ActionResult Scene6350::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 1: // Looking at the overview does not start a speech.
		break;
	case 301: // Go to the mine entrance.
		leaveTownRoom(6080);
		break;
	case 302: // Go to Douglasville.
		leaveTownRoom(6220);
		break;
	case 303: // Go to the house.
		leaveTownRoom(6130);
		break;
	case 304: // Go to the crater.
		_town.craterSeen = 1;
		leaveTownRoom(6270);
		break;
	case 306: // Go to the locomotive.
		leaveTownRoom(6265);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

void Scene6350::startTownFilm(byte film) {
	typedef RunawaySequence S;
	_townFilm = film;
	const bool gog = Common::File::exists("DATAVC00.000");
	_townFilmCommands.clear();
	// GOG moved the animated portraits out of the main films, changing the
	// frame numbers after each conversation. Keep both releases' cue points.
	if (film == 1) {
		const S::Command commands[] = {
			{S::kOpen, 1},
			{S::kMusic, 26, 178, 1},
			{S::kEffect, 67, 178},
			{S::kPlay, 0, 688},
			{S::kEffect, 68, 178, 0, 1},
			{S::kTalk, 1, 0}, // The helicopter lands.
			{S::kTalk, 1, 1},
			{S::kTalk, 1, 2},
			{S::kTalk, 1, 3},
			{S::kTalk, 1, 4},
			{S::kTalk, 1, 5},
			{S::kTalk, 1, 6},
			{S::kTalk, 1, 7},
			{S::kTalk, 1, 8},
			{S::kSpeech, 1, 9, 9},
			{S::kEffect, 69, 178},
			{S::kPlay, 689, 951},
			{S::kWaitSpeech},
			{S::kTalk, 1, 10, 3}, // Brian and Gina plan their next move.
			{S::kTalk, 1, 11, 1},
			{S::kTalk, 1, 12, 3},
			{S::kTalk, 1, 13, 1},
			{S::kTalk, 1, 14, 3},
			{S::kTalk, 1, 15, 1},
			{S::kTalk, 1, 16, 3},
			{S::kMusic, 28, 255},
			{S::kEffect, 70, 178},
			{S::kPlay, (uint16)(gog ? 952 : 975), 975},
			{S::kSpeech, 1, 24, 24},
			{S::kPlay, 976, (uint16)(gog ? 1875 : 1899)},
			{S::kSpeech, 1, 17, 17},
			{S::kPlay, (uint16)(gog ? 1876 : 1900), (uint16)(gog ? 2108 : 2132)},
			{S::kSpeech, 1, 18, 19}, // Gina falls through the ground.
			{S::kPlay, (uint16)(gog ? 2109 : 2133), (uint16)(gog ? 2111 : 2135)},
			{S::kMusic, 29, 255},
			{S::kPlay, (uint16)(gog ? 2112 : 2136), (uint16)(gog ? 2150 : 2174)},
			{S::kSpeech, 1, 20, 20},
			{S::kPlay, (uint16)(gog ? 2151 : 2175), (uint16)(gog ? 2248 : 2272)},
			{S::kWaitSpeech},
			{S::kTalk, 1, 21},
			{S::kDelay, 500},
			{S::kTalk, 1, 22},
			{S::kDelay, 500},
			{S::kTalk, 1, 23},
			{S::kEnd}
		};
		for (uint i = 0; i < ARRAYSIZE(commands); ++i)
			_townFilmCommands.push_back(commands[i]);
	} else {
		const S::Command commands[] = {
			{S::kOpen, 2},
			{S::kMusic, 25, 255},
			{S::kEffect, 71, 230},
			{S::kPlay, 0, 9},
			{S::kSpeech, 2, 0, 0},
			{S::kPlay, 10, 1943},
			{S::kMusic, 27, 255, 1},
			{S::kSpeech, 2, 1, 1},
			{S::kPlay, 1944, 2014},
			{S::kSpeech, 2, 2, 2},
			{S::kPlay, 2015, 2094},
			{S::kWaitSpeech},
			{S::kTalk, 2, 3, 1}, // Find Gina below.
			{S::kEffect, 72, 230},
			{S::kPlay, (uint16)(gog ? 2095 : 2108), (uint16)(gog ? 2109 : 2121)},
			{S::kTalk, 2, 4, 3},
			{S::kTalk, 2, 5, 3, 1},
			{S::kTalk, 2, 6, 3},
			{S::kPlay, (uint16)(gog ? 2110 : 2135), (uint16)(gog ? 2133 : 2158)},
			{S::kTalk, 2, 7, 7},
			{S::kStill, (uint16)(gog ? 2134 : 2158)},
			{S::kTalk, 2, 8, 3},
			{S::kTalk, 2, 9, 3, 1},
			{S::kStill, (uint16)(gog ? 2135 : 2158)},
			{S::kTalk, 2, 10, 5},
			{S::kTalk, 2, 11, 7},
			{S::kStill, (uint16)(gog ? 2136 : 2158)},
			{S::kTalk, 2, 12, 3},
			{S::kEffect, 73, 255},
			{S::kPlay, (uint16)(gog ? 2137 : 2240), (uint16)(gog ? 2166 : 2269)},
			{S::kSpeech, 2, 13, 13},
			{S::kPlay, (uint16)(gog ? 2167 : 2270), (uint16)(gog ? 2252 : 2354)},
			{S::kWaitSpeech},
			{S::kStill, (uint16)(gog ? 2253 : 2354)},
			{S::kTalk, 2, 14, 7}, // Joshua explains the Hopi key.
			{S::kTalk, 2, 15, 5},
			{S::kTalk, 2, 16, 7},
			{S::kTalk, 2, 17, 5},
			{S::kTalk, 2, 18, 7},
			{S::kTalk, 2, 19, 5},
			{S::kTalk, 2, 20, 7},
			{S::kEffect, 74, 230},
			{S::kPlay, (uint16)(gog ? 2254 : 2322), (uint16)(gog ? 2340 : 2409)},
			{S::kSpeech, 2, 21, 21},
			{S::kPlay, (uint16)(gog ? 2341 : 2410), (uint16)(gog ? 2488 : 2555)},
			{S::kWaitSpeech},
			{S::kTalk, 2, 22, 9},
			{S::kEffect, 75, 230},
			{S::kPlay, (uint16)(gog ? 2489 : 2567), (uint16)(gog ? 2555 : 2635)},
			{S::kEnd}
		};
		for (uint i = 0; i < ARRAYSIZE(commands); ++i)
			_townFilmCommands.push_back(commands[i]);
	}
	_sequence.start(_townFilmCommands.data(), _text, _now, 'F');
}

} // End of namespace Runaway
} // End of namespace Hollywood
