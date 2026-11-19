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
#include "hollywood/runaway/scene.h"
#include "hollywood/runaway/scene_data.h"

namespace Hollywood {
namespace {

const byte kPressButtonFrames[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
};

const byte kFreezeBatteryFrames[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
	19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
	39, 40, 41, 42, 43, 44, 45, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46,
	46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55,
	56, 57, 58, 59, 60, 61, 62, 63, 64, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17,
	16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
};

const RunawayHotspot kHotspots[] = {
	{"", 0, 0, 0, {0, 0}, {0, 0}},
	{"Door", 141, 468, 7, {301, 302}, {0, 1}},
	{"First aid kit", 196, 451, 1, {303, 304}, {0, 0}},
	{"Glass display case", 310, 457, 1, {305, 306}, {1, 1}},
	{"Button", 204, 448, 1, {307, 308}, {1, 1}},
	{"Mayan mask", 310, 457, 1, {309, 310}, {1, 1}},
	{"Scanner", 906, 470, 0, {311, 312}, {0, 0}},
	{"Tank", 1352, 431, 1, {315, 316}, {0, 0}},
	{"Golden object", 111, 446, 1, {313, 314}, {1, 1}},
	{"Device", 539, 439, 1, {317, 318}, {0, 0}},
	{"Ladle", 1329, 409, 1, {328, 329}, {0, 1}},
};

const RunawayResponse kResponses[] = {
	{kMuseumSecurity, 301, 1, 0},
	{kMuseumSecurity, 303, 2, 0},
	{kMuseumSecurity, 304, 3, 0},
	{kMuseumSecurity, 307, 6, 0},
	{kMuseumSecurity, 312, 11, 0},
	{kMuseumSecurity, 316, 15, 0},
	{kMuseumSecurity, 318, 17, 0},
	{kMuseumSecurity, 319, 18, 0},
	{kMuseumSecurity, 322, 21, 0},
	{kMuseumSecurity, 323, 11, 0},
	{kMuseumSecurity, 324, 22, 0},
	{kMuseumSecurity, 326, 24, 0},
	{kMuseumSecurity, 327, 25, 0},
	{kMuseumSecurity, 328, 26, 0},
};

const RunawayItemAction kItemActions[] = {
	{kMuseumSecurity, 5, 3, 321},
	{kMuseumSecurity, 11, 3, 320},
	{kMuseumSecurity, 0, 3, 319},
	{kMuseumSecurity, 5, 4, 321},
	{kMuseumSecurity, 7, 6, 323},
	{kMuseumSecurity, 0, 6, 322},
	{kMuseumSecurity, 6, 7, 330},
	{kMuseumSecurity, 18, 7, 325},
	{kMuseumSecurity, 0, 7, 324},
	{kMuseumSecurity, 7, 9, 327},
	{kMuseumSecurity, 0, 9, 326},
};

const RunawayExit kExits[] = {
	{kMuseumSecurity, 302, kMuseumHall, 1118, 309, true, false}
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene4100 = {
	kMuseumSecurity, 1536, 600, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {40, 40, 60}, {30, 30, 50}, {20, 20, 40}},
	Common::Point(141, 468),
	kResponses, ARRAYSIZE(kResponses),
	kItemActions, ARRAYSIZE(kItemActions),
	kExits, ARRAYSIZE(kExits)
};

bool RunawayRoomScene::loadScene4100() {
	return !museumChapter() || (_ambient.load(_archive, 4, 121) && _overlay.load(_archive, 5, 121) &&
		_npc.load(_archive, 12, 42) && _machine.load(_archive, 20, 45) &&
		_computer.load(_archive, 9, 49) && _rejectionLight.load(_archive, 11, 7));
}

bool RunawayRoomScene::applyBackground4100() {
	if (!museumChapter())
		return true;
	if (!applyPatch((_museum.chamberOpen ? 15 : 13) + (_museum.maskTaken ? 1 : 0)))
		return false;
	if (_museum.keyTaken && !applyPatch(6))
		return false;
	if (_museum.ladleTaken && !applyPatch(8))
		return false;
	return true;
}

void RunawayRoomScene::updateMaps4100() {
	if (!museumChapter())
		return;
	for (uint i = 0; i < 256; ++i) {
		byte &id = _maps[256 + i];
		if (id == 4 && (!_museum.buttonSeen || _museum.chamberOpen))
			id = 3;
		else if (id == 5 && (!_museum.cabinetSeen || _museum.maskTaken))
			id = 3;
		else if ((id == 8 && _museum.keyTaken) || (id == 10 && _museum.ladleTaken))
			id = 0;
		if (_maps[i] == 4 || _maps[i] == 5)
			_maps[i] = 0;
	}
}

void RunawayRoomScene::drawScene4100(Graphics::ManagedSurface &viewport) {
	const uint tick = (_now / 80 + 46) % 217;
	const uint frame = tick <= 106 ? 14 + tick : 216 - tick;
	_ambient.draw(viewport, frame, _camera.x, 0);
	_overlay.draw(viewport, frame, _camera.x, 0);
	static const byte durations[][45] = {
		{3, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12,
		 13, 13, 14, 14, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23},
		{3, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9,
		 10, 10, 10, 11, 11, 11, 12, 12, 12, 13, 13, 13, 14, 14, 14, 15, 15, 15, 16, 16, 16}
	};
	for (uint machine = 0; machine < 2; ++machine) {
		uint position = _machinePosition[machine], index = 0;
		const uint count = machine ? 45 : 42;
		while (index + 1 < count && position >= durations[machine][index])
			position -= durations[machine][index++];
		(machine ? _machine : _npc).draw(viewport, index, _camera.x, 0);
	}
	if (_cliveVisible)
		_npcMouth.draw(viewport, 17 + (speaker() ? speechFrame() : 0), _camera.x, 0);
	_computer.draw(viewport, _computerFrame, _camera.x, 0);
	if (_computerReject)
		_rejectionLight.draw(viewport, 6, _camera.x, 0);
}

void RunawayRoomScene::closeMuseumChamber() {
	_museum.chamberTime = 0;
	beginRoomAnimationRange(430, _museum.maskTaken ? 33 : 32, 36, 35, 0, false);
	_audio.playEffect(33, 191);
}

RunawayRoomScene::ActionResult RunawayRoomScene::execute4100(uint16 action) {
	if (!museumChapter())
		return kUnhandled;
	switch (action) {
	case 302:
		leaveMuseumRoom(4062, Common::Point(1118, 309), 3);
		return kHandled;
	case 305:
		say(4, !_museum.cabinetSeen ? 0 : _museum.maskTaken ? 2 : 1);
		_museum.cabinetSeen = 1;
		return kBackgroundChanged;
	case 306:
		say(5, _museum.buttonSeen);
		_museum.buttonSeen = 1;
		return kBackgroundChanged;
	case 308:
		beginRoomAnimation(308, 29, 12, kPressButtonFrames, ARRAYSIZE(kPressButtonFrames));
		return kHandled;
	case 309:
		say(8, _museum.chamberOpen != 0);
		return kHandled;
	case 310:
		if (!_museum.chamberOpen)
			say(9);
		else {
			say(9, 1);
			_museum.maskTaken = 1;
			_inventory.add(11);
			say(9, 2);
			deferMuseumAction(432);
		}
		return kBackgroundChanged;
	case 311:
		say(10, _museum.scannerSeen);
		_museum.scannerSeen = 1;
		return kHandled;
	case 313:
		say(12, _museum.keySeen);
		_museum.keySeen = 1;
		return kHandled;
	case 314:
		if (!_museum.keySeen)
			say(12);
		_museum.keySeen = _museum.keyTaken = 1;
		_inventory.add(17);
		say(13);
		say(13, 1);
		say(13, 2);
		return kBackgroundChanged;
	case 315:
		say(14, _museum.nitrogenSeen);
		_museum.nitrogenSeen = 1;
		return kHandled;
	case 317:
		say(16, _museum.microscopeSeen);
		_museum.microscopeSeen = 1;
		return kHandled;
	case 320:
		say(19, _museum.chamberOpen != 0);
		return kHandled;
	case 321:
		if (!_museum.voiceLockSeen)
			say(2, 0, true);
		else if (!_inventory.museum().voiceRecorded)
			say(20);
		else if (_museum.chamberOpen)
			say(1, 2, true);
		else if (_inventory.museum().recorderBattery == 1 || _inventory.museum().recorderBattery == 2)
			say(20, _inventory.museum().recorderBattery == 1 ? 2 : 1);
		else {
			const byte battery = _inventory.museum().recorderBattery;
			say(20, battery == 0 ? 3 : battery == 3 ? 7 : 9);
			walkTo(Common::Point(204, 448), 1, true);
			beginRoomAnimation(340, 29, 12, kPressButtonFrames, ARRAYSIZE(kPressButtonFrames));
		}
		return kHandled;
	case 325:
		if (!_inventory.museum().batteryInLadle)
			say(23);
		else {
			say(23, 1);
			beginRoomAnimation(325, 23, 69, kFreezeBatteryFrames, ARRAYSIZE(kFreezeBatteryFrames));
			byte tank[ARRAYSIZE(kFreezeBatteryFrames)];
			for (uint i = 0; i < ARRAYSIZE(tank); ++i)
				tank[i] = MIN<uint>(44, i > 17 ? i - 17 : 0);
			setRoomAnimationOverlay(25, 45, tank, ARRAYSIZE(tank));
		}
		return kHandled;
	case 329:
		_museum.ladleTaken = 1;
		_inventory.add(18);
		say(113, 0, true);
		return kBackgroundChanged;
	case 330:
		say(27, _inventory.museum().frozenBattery);
		return kHandled;
	default: return kUnhandled;
	}
}

void RunawayRoomScene::finish4100(uint16 action) {
	switch (action) {
	case 340:
		say(90, 0, false, 1);
		deferMuseumAction(341);
		break;
	case 341:
		beginRoomAnimationRange(342, 27, 21, 0, 20);
		break;
	case 342:
		say(91, _inventory.museum().recorderBattery ? 1 : 0, false, 2);
		holdRoomAnimation(343, 27, 21, 20);
		break;
	case 343:
		beginRoomAnimationRange(344, 27, 21, 20, 0);
		break;
	case 344:
		_computerReject = !_inventory.museum().recorderBattery;
		say(90, _computerReject ? 1 : 2, false, 1);
		deferMuseumAction(321);
		break;
	case 321:
		_computerReject = false;
		if (!_inventory.museum().recorderBattery) {
			say(20, 4);
			say(20, 5);
			_inventory.museum().recorderBattery = 2;
			_inventory.add(6);
			say(20, 6);
		} else {
			beginRoomAnimation(345, 10, 7, nullptr, 0, false);
		}
		break;
	case 345:
		beginRoomAnimation(431, _museum.maskTaken ? 33 : 32, 36, nullptr, 0, false);
		_audio.playEffect(33, 191);
		break;
	case 325:
		_inventory.museum().batteryInLadle = 0;
		_inventory.museum().frozenBattery = 1;
		_inventory.add(6);
		say(23, 2);
		say(23, 3);
		break;
	case 308:
		startConversation(3);
		break;
	case 409:
		say(28, 1);
		beginRoomAnimation(410, 26, 46);
		break;
	case 410:
		startMuseumFilm(kScannerFilm);
		break;
	case 411:
		startMuseumFilm(kBetrayalFilm);
		break;
	case 412:
		if (!_museum.labVisited) {
			_museum.labVisited = 1;
			say(0);
		}
		break;
	case 413:
		say(28);
		deferMuseumAction(414);
		break;
	case 414:
		beginRoomAnimationRange(415, 24, 28, 0, 4);
		break;
	case 415: {
		_camera.x = 0;
		beginRoomAnimationRange(416, 22, 60, 0, 16);
		const byte brian[] = {4};
		setRoomAnimationOverlay(24, 28, brian, ARRAYSIZE(brian));
		break;
	}
	case 416:
		if (!_npcMouth.load(_archive, 22, 60))
			error("Cannot load Runaway Clive animation");
		_cliveVisible = true;
		say(92, 0, false, 1);
		holdRoomAnimation(417, 24, 28, 4);
		break;
	case 417:
		beginRoomAnimationRange(418, 24, 28, 5, 9);
		break;
	case 418:
		say(92, 1, false, 1);
		holdRoomAnimation(419, 24, 28, 9);
		break;
	case 419: {
		_cliveVisible = false;
		beginRoomAnimationRange(420, 22, 60, 44, 59);
		const byte brian[] = {9};
		setRoomAnimationOverlay(24, 28, brian, ARRAYSIZE(brian));
		break;
	}
	case 420:
		beginRoomAnimationRange(411, 24, 28, 10, 27);
		break;
	case 432:
		closeMuseumChamber();
		break;
	case 430:
		_museum.chamberOpen = 0;
		_museum.chamberTime = 0;
		refreshMuseum();
		break;
	case 431:
		if (_inventory.museum().recorderBattery == 3)
			say(20, 8);
		_inventory.museum().recorderBattery = 4;
		_museum.buttonSeen = _museum.chamberOpen = 1;
		_museum.chamberTime = 30000;
		refreshMuseum();
		break;
	}
}

float RunawayRoomScene::actorHeight4100() const {
	const int x = _position.x, y = _position.y;
	if (x > 1260)
		return MIN(y, 545) * 1.2807018f - 242.982452f;
	if (x > 645)
		return MIN(y, 545) * 0.73333335f + 55.333332f;
	return MIN(y, 545) * 0.9381443f - 56.288658f;
}

void RunawayRoomScene::setupDepth4100(RunawayActor::RenderParams &params) const {
	params.depthY[1] = 599;
	params.depthY[2] = 477;
}

} // End of namespace Hollywood
