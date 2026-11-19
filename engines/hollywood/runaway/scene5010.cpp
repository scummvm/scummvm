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
#include "common/textconsole.h"
#include "hollywood/runaway/scene.h"
#include "hollywood/runaway/scene_data.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{"", 0, 0, 0, {0, 0}, {0, 0}},
	{"Unvisited Area", 0, 0, 0, {1, 301}, {0, 0}},
	{"Thugs' Cabin", 0, 0, 0, {1, 302}, {0, 0}},
	{"Unvisited Area", 0, 0, 0, {1, 303}, {0, 0}},
	{"Unvisited Area", 0, 0, 0, {1, 304}, {0, 0}},
	{"Unvisited Area", 0, 0, 0, {1, 305}, {0, 0}},
	{"Drag Queen Bus", 0, 0, 0, {1, 306}, {0, 0}},
	{"Oil Well", 0, 0, 0, {0, 0}, {0, 0}},
	{"Hangar", 0, 0, 0, {0, 0}, {0, 0}},
	{"Airplane Graveyard", 0, 0, 0, {0, 0}, {0, 0}},
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene5010 = {
	5010, 1024, 600, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {90, 85, 80}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}},
	Common::Point(512, 500),
	nullptr, 0,
	nullptr, 0,
	nullptr, 0
};

void RunawayRoomScene::startDesert() {
	_entering = true;
	resumeDesert();
}

void RunawayRoomScene::resumeDesert() {
	if (!_audio.musicPlaying())
		_audio.playMusic(_room == 5060 ? 21 : 12);
	if (!_entering)
		return;
	_entering = false;
	if (_room == 5140) {
		startScene5140();
		return;
	}
	if (_entryScene == 5021 && !_desert.escapedCabin) {
		_desert.escapedCabin = 1;
		say(0);
		leaveDesertRoom(5140);
		return;
	}
	if (_entryScene == 5062) {
		for (byte variation = 2; variation <= 11; ++variation)
			say(0, variation, false, variation == 2 || variation == 4 || variation == 6 || variation == 11 ? 0 : 1);
		leaveDesertRoom(5130);
		return;
	}
	if (_entryScene == 5171) {
		_position = Common::Point(636, 382);
		_walkPath.reset(_position);
		_facing = 4;
	}
	if (_entryScene == 5081) {
		_position = Common::Point(987, 379);
		_walkPath.reset(_position);
		walkTo(Common::Point(1000, 451), 5, true);
	}
	if (_entryScene == 5101) {
		execute(400);
		return;
	}
	if (_entryScene == 5100)
		walkTo(Common::Point(1053, 403), 3);
	const uint16 rooms[] = {5010, 5020, 5050, 5060, 5070, 5080, 5100, 5110, 5160, 5170};
	for (uint i = 0; i < ARRAYSIZE(rooms); ++i) {
		if (_room != rooms[i] || _desert.visited[i])
			continue;
		_desert.visited[i] = 1;
		if (_room != 5010 && _room != 5020 && _room != 5160) {
			say(0);
			if (_room == 5050 || _room == 5060 || _room == 5100 || _room == 5170)
				say(0, 1);
		}
	}
}

void RunawayRoomScene::leaveDesertRoom(uint16 scene) {
	const RunawayRoom *room = findRunawayRoom(scene);
	if (!room && (scene < 5030 || scene > 5032)) {
		warning("Runaway: scene %u is not implemented yet", scene);
		return;
	}
	if (!_hangarSpeech.subtitle().empty())
		_hangarSpeech.stop();
	_hospitalExit.scene = scene;
	_hospitalExit.position = room ? room->entry : Common::Point(1167, 468);
	_hospitalExit.facing = 3;
}

void RunawayRoomScene::deferDesertAction(uint16 action) {
	_roomAction = action;
	_actionFrames.clear();
}

void RunawayRoomScene::finishDesertAnimation(uint16 action) {
	execute(action + 1000);
}

void RunawayRoomScene::desertAnimationFrame() {
	if (_room == 5060)
		animationFrame5060();
	if (_room == 5100)
		animationFrame5100();
	if (_room == 5080 && _roomAction == 326 && _actionStep == 23)
		_audio.playEffect(72);
}

void RunawayRoomScene::updateDesert(uint elapsed, bool inventoryOpen) {
	if (_room == 5140) {
		updateScene5140();
		return;
	}
	updateRoomAnimation();
	updateConversation();
	if (_room == 5080)
		updateScene5080();
	if (_room == 5100)
		updateScene5100(elapsed, inventoryOpen);
	if (_roomAction && _actionFrames.empty() && !speaking() && !_walkPath.active()) {
		const uint16 action = _roomAction;
		_roomAction = 0;
		execute(action);
	}
	if (_hospitalExit.scene && !speaking() && !_roomAction && !_walkPath.active())
		_transition = _hospitalExit;
}

RunawayRoomScene::ActionResult RunawayRoomScene::executeDesert(uint16 action) {
	switch (_room) {
	case 5010: {
		const uint16 destinations[] = {
			5070, // 301: Go to the oil well.
			5020, // 302: Go to the thugs' cabin.
			5100, // 303: Go to the hangar.
			5050, // 304: Go to the airplane graveyard.
			5080, // 305: Go to the railway yard.
			5060  // 306: Go to the bus.
		};
		if (action >= 301 && action <= 306) {
			leaveDesertRoom(destinations[action - 301]);
			return kHandled;
		}
		break;
	}
	case 5020:
		return execute5020(action);
	case 5050:
		return execute5050(action);
	case 5060:
		return execute5060(action);
	case 5070:
		return execute5070(action);
	case 5080:
		return execute5080(action);
	case 5100:
		return execute5100(action);
	case 5110:
		return execute5110(action);
	case 5160:
		return execute5160(action);
	case 5170:
		return execute5170(action);
	default:
		break;
	}
	return kUnhandled;
}

float RunawayRoomScene::desertActorHeight() const {
	switch (_room) {
	case 5020:
		return _position.y * -0.140845075f + 264.225342f;
	case 5050:
		return _position.y * 0.128865972f + 197.809280f;
	case 5060:
		return _position.y * 0.107991360f + 207.213821f;
	case 5070:
		return _position.y * 1.171428561f - 226.085709f;
	case 5080:
		return _position.y * 0.905172408f - 162.241379f;
	case 5100:
		return _position.y * 0.668016195f + 87.591095f;
	case 5110:
		return _position.y * 1.142857194f - 170.571426f;
	case 5160:
		return _position.y * 1.148148179f - 90.370369f;
	case 5170:
		return _position.y * 0.536585391f + 165.243896f;
	default:
		return 455;
	}
}

bool RunawayRoomScene::loadDesertRoom() {
	const RunawayRoom *room = findRunawayRoom(_room);
	_desertHotspots.assign(room->hotspots, room->hotspots + room->hotspotCount);
	struct Animation {
		uint16 room;
		byte chunk;
		uint16 frames;
	};
	static const Animation animations[] = {
		{5010, 9, 48}, {5010, 11, 186}, {5010, 14, 307}, {5010, 10, 700}, {5010, 12, 31}, {5010, 15, 1052},
		{5020, 7, 947}, {5020, 4, 452}, {5020, 5, 667}, {5020, 6, 47}, {5020, 8, 662},
		{5060, 15, 543}, {5060, 16, 411}, {5060, 17, 791}, {5060, 18, 769},
		{5070, 5, 896}, {5070, 6, 945}, {5070, 7, 953}, {5070, 8, 551}, {5070, 4, 48},
		{5080, 18, 769}, {5080, 17, 672}, {5080, 16, 455}, {5080, 20, 631}, {5080, 19, 533}, {5080, 15, 24},
		{5100, 4, 1}, {5100, 7, 27}, {5100, 8, 89},
		{5110, 8, 1}, {5110, 9, 2},
		{5140, 1, 130}, {5140, 2, 33}, {5140, 3, 51}, {5140, 4, 32},
		{5160, 4, 16}, {5160, 5, 24}, {5160, 6, 24}, {5160, 7, 93},
		{5170, 6, 25}, {5170, 5, 24}, {5170, 4, 50}, {5170, 11, 3}
	};
	uint slot = 0;
	for (uint i = 0; i < ARRAYSIZE(animations); ++i) {
		if (animations[i].room == _room && !_desertAnimations[slot++].load(_archive, animations[i].chunk, animations[i].frames))
			return false;
	}
	if (_room == 5050)
		return loadForeground(0, 4) && loadForeground(1, 5);
	if (_room == 5060)
		return loadForeground(0, 8) && _npc.load(_archive, 7, 201) && _machine.load(_archive, 12, 195);
	if (_room == 5080)
		return loadForeground(0, 11) && loadForeground(1, 12) && loadForeground(2, 13) && loadForeground(3, 14) &&
			_npc.load(_archive, 9, 47);
	if (_room == 5100)
		return _npc.load(_archive, 9, 151) && _npcMouth.load(_archive, 12, 126) &&
			_machine.load(_archive, 16, 79);
	return true;
}

bool RunawayRoomScene::applyDesertBackground() {
	switch (_room) {
	case 5010:
		return (!_desert.visited[4] || applyPatch(3)) && (!_desert.visited[6] || applyPatch(4)) &&
			(!_desert.visited[2] || applyPatch(5)) && (!_desert.visited[5] || applyPatch(6)) && applyPatch(7);
	case 5020:
		return !_desert.bucketTaken || applyPatch(9);
	case 5050:
		return (!_desert.helmetTaken || applyPatch(6)) && (!_desert.beltTaken || applyPatch(7)) &&
			(!_desert.gun || applyPatch(8));
	case 5060:
		return _desert.tanningOil != 2 || applyPatch(9);
	case 5080:
		return loadRunawayMask(_archive, _background.w, _background.h, _mask, _originalMaps, _desert.antsDistracted ? 4 : 1) &&
			(!_desert.antsDistracted || applyPatch(_desert.dynamite == 1 ? 7 : 8));
	case 5100:
		return applyBackground5100();
	case 5110:
		return _desert.barrel != 2 || applyPatch(13);
	case 5160:
		return (_desert.needle != 2 || applyPatch(8)) && (!_desert.glassesTaken || applyPatch(9)) &&
			(_desert.lipstickShelf >= 2 || applyPatch(10)) &&
			((_desert.busDoor != 1 && _desert.busDoor != 3) || (applyPatch(11) && applyPatch(13))) &&
			(_desert.busDoor != 2 || applyPatch(14));
	case 5170:
		return (_desert.vacuum != 2 || applyPatch(7)) && (!_desert.glassesTaken || applyPatch(8)) &&
			(_desert.lipstickShelf != 2 || applyPatch(9)) && (_desert.lipstickShelf != 3 || applyPatch(10));
	default:
		return true;
	}
}

void RunawayRoomScene::updateDesertMaps() {
	if (_room == 5100) {
		updateMaps5100();
		return;
	}
	for (uint i = 0; i < 256; ++i) {
		byte &id = _maps[256 + i];
		switch (_room) {
		case 5020:
			if (_desert.bucketTaken && id == 7)
				id = 0;
			break;
		case 5050:
			if ((_desert.helmetTaken && id == 2) || (_desert.beltTaken && id == 3))
				id = 0;
			break;
		case 5060:
			if (_desert.tanningOil == 2 && id == 6)
				id = 0;
			break;
		case 5070:
			if (id == 3 || id == 8)
				id = _desert.dynamitePlaced ? 3 : 0;
			break;
		case 5080:
			if (_desert.antsDistracted) {
				if (id == 12)
					id = _desert.dynamite == 1 ? 5 : 0;
				else if (id == 5 && _desert.dynamite == 2)
					id = 3;
			}
			break;
		case 5110:
			if (id == 8)
				id = _desert.bucket == 1 || _desert.bucket == 2 ? 8 : 4;
			break;
		case 5160:
			if (_desert.needle == 2 && id == 9)
				id = 0;
			break;
		case 5170:
			if (id == 5 && (_desert.busDoor == 0 || _desert.busDoor == 2))
				id = 1;
			if ((_desert.glassesTaken && id == 6) || (_desert.vacuum == 2 && id == 7))
				id = 0;
			if (id == 11)
				id = _desert.crank == 2 && _desert.busDoor != 1 ? 4 : 10;
			if (id == 12)
				id = _desert.crank == 2 && _desert.busDoor == 1 ? 4 : 2;
			break;
		default:
			break;
		}
	}
}

Common::U32String RunawayRoomScene::desertHotspotName(byte id) const {
	const RunawayHotspot *spot = hotspot(id);
	if (!spot)
		return Common::U32String();
	if (_room == 5010) {
		if (id == 1 && _desert.visited[4])
			return Common::U32String("Oil well");
		if (id == 3 && _desert.visited[6])
			return Common::U32String("Hangar");
		if (id == 4 && _desert.visited[2])
			return Common::U32String("Airplane graveyard");
		if (id == 5 && _desert.visited[5])
			return Common::U32String("Railway yard");
	}
	if (_room == 5050 && id == 3 && _desert.beltSeen)
		return _inventory.name(14);
	if (_room == 5060 && id == 6 && _desert.tanningOil)
		return _inventory.name(12);
	if (_room == 5060 && id == 7 && _desert.busPosterSeen)
		return Common::U32String("Lawn chair");
	if (_room == 5080 && id == 3 && _desert.antsDistracted)
		return Common::U32String("Sawdust");
	if (_room == 5100 && id == 3 && _desert.peanutButter)
		return _inventory.name(19);
	if (_room == 5100 && id == 8 && _desert.footrestSeen)
		return _inventory.name(21);
	if (_room == 5100 && id == 12)
		return Common::U32String(_desert.beerSeen ? "Beer can" : "Can");
	if (_room == 5110 && id == 1)
		return Common::U32String("Door");
	if (_room == 5110 && id == 2)
		return Common::U32String("Door");
	if (_room == 5110 && id == 5 && _desert.barrel)
		return Common::U32String("Barrel of gunpowder");
	if (_room == 5170 && id == 3 && _desert.lipstickShelf && _desert.lipstickShelf < 3)
		return _inventory.name(11);
	if (_room == 5170 && id == 4 && _desert.crank == 2)
		return Common::U32String("Makeshift crank");
	if (_room == 5170 && id == 7 && _desert.vacuum)
		return _inventory.name(22);
	if (_room == 5170 && id == 8 && _desert.necklacesSeen)
		return Common::U32String("Necklaces");
	if (_room == 5170 && id == 9 && _desert.ringsSeen)
		return Common::U32String("Rings and earrings");
	return Common::U32String(spot->name);
}

void RunawayRoomScene::drawDesertRoom(Graphics::ManagedSurface &viewport, bool foreground) {
	if (_room == 5100) {
		drawScene5100(viewport, foreground);
		return;
	}
	if (foreground) {
		if (_room == 5050) {
			if (_position.x < 314)
				drawForeground(viewport, 0);
			if (_position.x > 195 && _position.y < 465)
				drawForeground(viewport, 1);
		} else if (_room == 5060 && _position.x < 670) {
			drawForeground(viewport, 0);
		} else if (_room == 5080) {
			drawForeground(viewport, 0);
			drawForeground(viewport, 1);
			if (walkZone() == 15 || walkZone() == 16)
				drawForeground(viewport, 2);
			if (walkZone() == 15)
				drawForeground(viewport, 3);
		}
		return;
	}
	for (uint i = 0; i < ARRAYSIZE(_desertAnimations); ++i) {
		const RunawayAnimation &animation = _desertAnimations[i];
		if (!animation.frameCount())
			continue;
		uint frame = (_now / 80) % animation.frameCount();
		if (_room == 5110) {
			if (!i && (_desert.bucket == 0 || _desert.bucket == 3))
				continue;
			frame = i ? (_desert.trainDoorOpen != 0) : 0;
		} else if (_room == 5170) {
			if (i == 0)
				frame = _desert.busDoor == 0 || _desert.busDoor == 2 ? 24 : 0;
			if (i == 2)
				frame = 49;
			if (i == 3)
				frame = _desert.crank == 2 ? (_desert.busDoor == 1 ? 1 : 2) : 0;
		} else if (_room == 5160) {
			if (i == 3)
				frame = 92;
		}
		animation.draw(viewport, frame, _camera.x, -_camera.y);
	}
	if (_room == 5060)
		drawScene5060(viewport);
	else if (_room == 5080)
		drawScene5080(viewport);
}

void RunawayRoomScene::setupDesertDepth(RunawayActor::RenderParams &params) const {
	switch (_room) {
	case 5100:
		params.depthY[1] = 599;
		params.depthY[2] = 303;
		params.depthY[3] = 446;
		params.depthY[4] = 699;
		break;
	case 5110:
		params.depthY[1] = 599;
		params.depthY[2] = 408;
		break;
	case 5160:
		params.depthY[1] = 599;
		break;
	case 5170:
		params.depthY[4] = 599;
		break;
	default:
		break;
	}
}

} // End of namespace Hollywood
