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

#include "common/endian.h"
#include "common/str-enc.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "hollywood/runaway/navigation.h"
#include "hollywood/runaway/saveload.h"
#include "hollywood/runaway/scene.h"
#include "hollywood/runaway/scene_data.h"
#include "hollywood/runaway/strings.h"

namespace Hollywood {

namespace {

const RunawayRoom *const kRooms[] = {
	&kRunawayScene2020, &kRunawayScene2060, &kRunawayScene2080,
	&kRunawayScene4030, &kRunawayScene4050, &kRunawayScene4060, &kRunawayScene4070, &kRunawayScene4090, &kRunawayScene4100, &kRunawayScene4140,
	&kRunawayScene5010, &kRunawayScene5020, &kRunawayScene5050, &kRunawayScene5060, &kRunawayScene5070,
	&kRunawayScene5080, &kRunawayScene5100, &kRunawayScene5110, &kRunawayScene5130, &kRunawayScene5140,
	&kRunawayScene5160, &kRunawayScene5170,
	&kRunawayScene6014, &kRunawayScene6080, &kRunawayScene6090, &kRunawayScene6130,
	&kRunawayScene6180, &kRunawayScene6190, &kRunawayScene6200, &kRunawayScene6210,
	&kRunawayScene6220, &kRunawayScene6240, &kRunawayScene6260, &kRunawayScene6263,
	&kRunawayScene6270, &kRunawayScene6290,
	&kRunawayScene6350, &kRunawayScene6370
};

const RunawayResponse kGlobalResponses[] = {
	{0, 3, 2, 0},
	{0, 4, 3, 0},
	{0, 5, 4, 0},
	{0, 6, 5, 0},
	{0, 8, 7, 0},
	{0, 9, 8, 0},
	{0, 10, 9, 0},
	{0, 11, 10, 0},
	{0, 13, 12, 0},
	{0, 14, 13, 0},
	{0, 16, 15, 0},
	{0, 17, 1, 2},
	{0, 18, 16, 0},
	{0, 20, 11, 0},
	{0, 22, 19, 0},
	{0, 23, 19, 1},
	{0, 29, 23, 0},
	{0, 30, 24, 0},
};

const RunawayRoom *findRoom(int number) {
	if (number >= 2021 && number <= 2023)
		number = kHospitalBedroom;
	if (number == 4010 || (number >= 4061 && number <= 4065))
		number = kMuseumHall;
	else if (number >= 4071 && number <= 4072)
		number = kMuseumWilly;
	else if (number == 4101)
		number = kMuseumSecurity;
	else if (number >= 4141 && number <= 4142)
		number = kMuseumKeypad;
	else if (number == 5021 || number == 5022 || number == 5061 || number == 5062 ||
			number == 5081 || number == 5101 || number == 5171)
		number -= number % 10;
	else if (number >= 6221 && number <= 6225)
		number = 6220;
	else if (number == 6015) {
		number = 6014;
	} else if (number == 6081 || number == 6082) {
		number = 6080;
	} else if (number == 6131) {
		number = 6130;
	} else if (number == 6181) {
		number = 6180;
	} else if (number == 6241) {
		number = 6240;
	} else if (number == 6201) {
		number = 6200;
	} else if (number == 6261 || number == 6262 || number == 6265) {
		number = 6260;
	} else if (number == 6264) {
		number = 6263;
	} else if (number == 6291) {
		number = 6290;
	}
	for (uint i = 0; i < ARRAYSIZE(kRooms); ++i) {
		if (kRooms[i]->number == number)
			return kRooms[i];
	}
	return nullptr;
}

} // End of anonymous namespace

const RunawayRoom *findRunawayRoom(uint16 number) {
	return findRoom(number);
}

RunawayRoomScene::RunawayRoomScene(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	_audio(audio), _sequence(audio), _inventory(inventory), _strings(strings), _game(game), _random("runaway"),
	_speech(audio, _random, game == kRunawayFullGame ? RunawaySpeech::kRecordedLipSync : RunawaySpeech::kNoLipSync),
	_hangarSpeech(audio, _random, RunawaySpeech::kNoLipSync) {}

const Common::U32String &RunawayRoomScene::subtitle() const {
	if (_sequence.active())
		return _sequence.subtitle();
	return speaking() ? _speech.subtitle() : _hangarSpeech.subtitle();
}

bool RunawayRoomScene::load(uint16 entryScene) {
	const RunawayRoom *room = findRoom(entryScene);
	if (!room)
		return false;
	_entryScene = entryScene;
	const Graphics::PixelFormat format(2, 5, 6, 5, 0, 11, 5, 0, 0);
	_screen.create(kRunawayScreenWidth, kRunawayScreenHeight, format);
	return enterRoom(room->number, room->entry, room->number == kHospitalBedroom ? 7 : 3);
}

void RunawayRoomScene::start(uint32 now) {
	_now = _lastUpdate = now;
	_idleHead.reset(now);
	if (townChapter()) {
		_inventory.startChapter(4);
		_entering = true;
		resumeTown();
		return;
	}
	if (desertChapter()) {
		startDesert();
		return;
	}
	if (_room == kHospitalBedroom) {
		start2020();
		resume();
		return;
	}
	if (museumChapter()) {
		startMuseum();
		return;
	}
	const byte initialItems[] = {1, 2, 3, 4, 7};
	for (uint i = 0; i < ARRAYSIZE(initialItems); ++i)
		_inventory.add(initialItems[i]);
	_inventory.takeNewItem();
	resume();
}

void RunawayRoomScene::resume() {
	if (townChapter()) {
		resumeTown();
		return;
	}
	if (desertChapter()) {
		resumeDesert();
		return;
	}
	if (museumChapter()) {
		resumeMuseum();
		return;
	}
	if (_game == kRunawayFullGame) {
		if (_sequence.active())
			return;
		if (!_audio.musicPlaying())
			_audio.playMusic(11, false);
		if (_room == kHospitalBedroom)
			_audio.playEffect(30, 3, 1, true);
		if (_entering) {
			if (_room == kHospitalBathroom) {
				_audio.playEffect(48);
				walkTo(Common::Point(227, 546), 1, true);
			} else {
				if (_inventory.water() == 1)
					_inventory.setWater(2);
				if (_room == kHospitalStoreroom || _entryScene == 2022) {
					static const byte frames[] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
					beginRoomAnimation(_entryScene, _room == kHospitalStoreroom ? 10 : 23, 13, frames, ARRAYSIZE(frames));
				} else if (_entryScene == 2023) {
					startHospitalFilm(kAmbushFilm);
				} else {
					_audio.playEffect(48);
				}
			}
			_entering = false;
		}
		return;
	}
	// The original demo loops M04 cue 11 throughout gameplay (0x4175c0).
	if (!_audio.musicPlaying())
		_audio.playMusic(11);
}

void RunawayRoomScene::captureState(RunawaySaveState &state) const {
	state.scene = _entryScene;
	state.x = _position.x;
	state.y = _position.y;
	state.camera = _camera.x;
	state.cameraY = _camera.y;
	state.facing = _facing;
	state.upperFloor = _upperFloor;
	state.books = _books;
	state.recorder = _recorder;
	state.moneyIdentified = _moneyIdentified;
	state.safeOpen = _safeOpen;
	state.varnishTaken = _varnishTaken;
	state.talcTaken = _talcTaken;
	state.coffeeTaken = _coffeeTaken;
	state.keyCount = _keyCount;
	state.hospital = _hospital;
	state.museum = _museum;
	state.desert = _desert;
	state.town = _town;
	state.randomSeed = _random.getSeed();
}

bool RunawayRoomScene::restoreState(const RunawaySaveState &state, uint32 now, bool entering) {
	if (!state.valid() || state.game != _game || state.scene != _entryScene)
		return false;
	_hospital = state.hospital;
	_museum = state.museum;
	_desert = state.desert;
	_town = state.town;
	_entering = entering;
	_upperFloor = state.upperFloor;
	_books = (ObjectState)state.books;
	_recorder = (ObjectState)state.recorder;
	_moneyIdentified = state.moneyIdentified;
	_safeOpen = state.safeOpen;
	_varnishTaken = state.varnishTaken;
	_talcTaken = state.talcTaken;
	_coffeeTaken = state.coffeeTaken;
	if (!rebuildBackground() || !_actor.setFrame(state.facing, 0))
		return false;
	updateMaps();
	_position = Common::Point(state.x, state.y);
	_walkPath.reset(_position);
	_facing = state.facing;
	_keyCount = entering ? 0 : state.keyCount;
	_now = _lastUpdate = now;
	_random.setSeed(state.randomSeed);
	_idleHead.reset(now);
	if (entering)
		updateCamera();
	else
		_camera = Common::Point(state.camera, state.cameraY);
	return true;
}

bool RunawayRoomScene::enterRoom(uint16 number, Common::Point position, byte facing) {
	const RunawayRoom *room = findRoom(number);
	if (!room || position.x < 0 || position.x >= room->width || position.y < 0 || position.y >= room->height)
		return false;
	const Common::Path path(Common::String::format("RESOURCE.%c%02u", 'A' + number / 1000 - 1,
		number == 6014 ? 3 : (number % 1000) / 10));
	if (!_archive.open(path, 40) ||
			!_text.load("RESOURCE.003", "Runaway room", number == kMuseumWilly ? 406 : number == 6014 ? 608 : number / 10, 0, 0x1135))
		return false;
	if (number == 5140 || number == 6014) {
		// These cutscenes contain a background and sprite tracks, without a mask.
		_mask.resize(room->width * room->height, 0);
		_originalMaps.resize(1536, 0);
	} else if (!loadRunawayMask(_archive, room->width, room->height, _mask, _originalMaps, number == 6263 ? 21 : 1)) {
		return false;
	}
	if (_game == kRunawayFullGame ? !_actor.loadChapter(number < 4000 ? 1 : number < 5000 ? 2 : number < 6000 ? 3 : 4,
			number != kMuseumHall && number != 6190 && number != 6200 && number != 6240 && number != 6260 && number != 6263 &&
			number != 6350 && number != 6370) :
			!_actor.load(number == kMuseumHall ? RunawayActor::kMuseumHall : RunawayActor::kMuseumInterior))
		return false;
	_room = number;
	_position = position;
	_walkPath.reset(position);
	_facing = facing;
	_walkElapsed = 0;
	_pendingHotspot = _pendingItem = 0;
	_pendingAction = 0;
	_lines.clear();
	_lineIndex = 0;
	_speech.reset();
	_hangarSpeech.reset();
	_caption.clear();
	_ambient = RunawayAnimation();
	_overlay = RunawayAnimation();
	for (uint i = 0; i < ARRAYSIZE(_foreground); ++i)
		_foreground[i].clear();
	for (uint i = 0; i < ARRAYSIZE(_keyPatches); ++i)
		_keyPatches[i].clear();
	for (uint i = 0; i < ARRAYSIZE(_saloonPatches); ++i) {
		_saloonPatches[i].clear();
	}
	if (!loadCustomResources() || !rebuildBackground()) {
		return false;
	}
	_keyCount = _pressedKey = 0;
	updateMaps();
	updateCamera();
	return _actor.setFrame(_facing, 0);
}

bool RunawayRoomScene::loadCustomResources() {
	bool loaded = true;
	if (desertChapter())
		loaded = loadDesertRoom();
	switch (_room) {
	case kHospitalBedroom:
		loaded = loadScene2020();
		break;
	case kHospitalBathroom:
		loaded = loadScene2080();
		break;
	case kMuseumOffice:
		loaded = loadScene4030();
		break;
	case kMuseumLaboratory:
		loaded = loadScene4050();
		break;
	case kMuseumHall:
		loaded = loadScene4060();
		break;
	case kMuseumWilly:
		loaded = loadScene4070();
		break;
	case kMuseumSecurity:
		loaded = loadScene4100();
		break;
	case kMuseumKeypad:
		loaded = loadScene4140();
		break;
	case 6080:
		loaded = _ambient.load(_archive, 6, 2) && loadForeground(0, 4) && loadForeground(1, 7) &&
			(_entryScene != 6082 || _npc.load(_archive, 9, 135));
		break;
	case 6090:
		loaded = _npc.load(_archive, 6, 113) && loadForeground(0, 4);
		break;
	case 6130:
		loaded = _npc.load(_archive, 6, 106);
		break;
	case 6180:
		loaded = _ambient.load(_archive, 8, 71) && loadForeground(0, 4) && loadForeground(1, 5) &&
			_archive.readVariableChunk(6, _saloonPatches[0]) && _archive.readVariableChunk(7, _saloonPatches[1]);
		break;
	case 6200:
		loaded = _ambient.load(_archive, 6, 26);
		break;
	case 6220:
		loaded = _npc.load(_archive, 7, 170) && _npcMouth.load(_archive, 8, 45) && _machine.load(_archive, 9, 60);
		break;
	case 6240:
		loaded = _ambient.load(_archive, 4, 36);
		for (uint i = 0; i < ARRAYSIZE(_saloonPatches) && loaded; ++i) {
			loaded = _archive.readVariableChunk(6 + i, _saloonPatches[i]);
		}
		break;
	case 6260:
		loaded = loadForeground(0, 6);
		break;
	case 6263:
		loaded = loadScene6263();
		break;
	case 6370:
		loaded = _npc.load(_archive, 4, 88) && _ambient.load(_archive, 5, 20);
		break;
	default:
		break;
	}
	return loaded;
}

bool RunawayRoomScene::loadForeground(uint layer, uint chunk, bool pixels) {
	assert(layer < ARRAYSIZE(_foreground));
	const RunawayRoom *room = findRoom(_room);
	// Foreground masks restore (x, y, length) spans from the current background.
	Common::Array<byte> spans;
	if (!_archive.readVariableChunk(chunk, spans) || spans.size() < 2)
		return false;
	uint offset = 2;
	for (uint i = 0; i < READ_LE_UINT16(spans.data()); ++i) {
		if (offset + 6 > spans.size())
			return false;
		const uint x = READ_LE_UINT16(spans.data() + offset);
		const uint y = READ_LE_UINT16(spans.data() + offset + 2);
		const uint width = READ_LE_UINT16(spans.data() + offset + 4);
		if (!width || x + width > room->width || y >= room->height)
			return false;
		_foreground[layer].push_back(Common::Rect(x, y, x + width, y + 1));
		offset += 6 + (pixels ? width * 2 : 0);
	}
	return offset == spans.size();
}

bool RunawayRoomScene::applyPatch(uint chunk) {
	Common::Array<byte> data;
	return _archive.readVariableChunk(chunk, data) && applyRunawayPatch(data, _background);
}

bool RunawayRoomScene::rebuildBackground() {
	return loadBackground() && applyCustomBackgroundPatches();
}

bool RunawayRoomScene::loadBackground() {
	const RunawayRoom *room = findRoom(_room);
	return loadRunawaySurface(_archive, _room == 6263 ? 20 : 0, room->width, room->height, _background);
}

bool RunawayRoomScene::applyCustomBackgroundPatches() {
	if (desertChapter())
		return applyDesertBackground();
	switch (_room) {
	case kHospitalBedroom:
		return applyBackground2020();
	case kHospitalStoreroom:
		return applyBackground2060();
	case kHospitalBathroom:
		return applyBackground2080();
	case kMuseumOffice:
		return applyBackground4030();
	case kMuseumLaboratory:
		return applyBackground4050();
	case kMuseumExhibition:
		return applyBackground4090();
	case kMuseumSecurity:
		return applyBackground4100();
	case 6080:
		return _town.oilCan != 2 || applyPatch(5);
	case 6090:
		return _town.vessel != 2 || applyPatch(5);
	case 6130:
		return (_town.stone != 2 || applyPatch(4)) && (_town.vessel != 2 || applyPatch(5));
	case 6190:
		return applyBackground6190();
	case 6200:
		return !_town.firewoodTaken || applyPatch(4);
	case 6240:
		return !_town.flowerPotTaken || applyPatch(9);
	case 6260:
		return applyBackground6260();
	case 6350:
		return !_town.arrived || (applyPatch(5) && applyPatch(7));
	default:
		return true;
	}
}

void RunawayRoomScene::updateMaps() {
	_maps = _originalMaps;
	applyCustomHotspotState();
}

void RunawayRoomScene::applyCustomHotspotState() {
	if (_room == 6220) {
		for (uint i = 0; i < 256; ++i) {
			// The balcony shares this mask but is reached through Kevin's room.
			if (_maps[i] < 2 || _maps[i] > 4)
				_maps[i] = 0;
		}
		return;
	}
	if (_room == 6350) {
		for (uint i = 0; i < 256; ++i) {
			// The route to the Hopi village has not been discovered yet.
			if (_maps[256 + i] == 5)
				_maps[256 + i] = 0;
		}
		return;
	}
	if (desertChapter()) {
		updateDesertMaps();
		return;
	}
	switch (_room) {
	case 6080:
		updateMaps6080();
		break;
	case 6180:
		for (uint i = 0; i < 256; ++i) {
			if (_maps[i] == 3) {
				_maps[i] = 0;
			}
			if (_maps[256 + i] == 4 && !_town.saloonDoorKnown) {
				_maps[256 + i] = 0;
			}
		}
		break;
	case 6240:
		for (uint i = 0; i < 256; ++i) {
			if (_maps[i] >= 6 && _maps[i] <= 8) {
				_maps[i] = 0;
			}
			if (_maps[256 + i] == 3 && _town.flowerPotTaken) {
				_maps[256 + i] = 0;
			}
		}
		break;
	case 6090:
	case 6130:
		for (uint i = 0; i < 256; ++i) {
			if ((_room == 6090 && _maps[256 + i] == 6 && _town.vessel == 2) ||
					(_room == 6130 && _maps[256 + i] == 4 && _town.stone == 2)) {
				_maps[256 + i] = 0;
			}
		}
		break;
	case 6190:
		updateMaps6190();
		break;
	case 6200:
		updateMaps6200();
		break;
	case 6260:
		updateMaps6260();
		break;
	case kHospitalBedroom:
		updateMaps2020();
		break;
	case kHospitalStoreroom:
		updateMaps2060();
		break;
	case kHospitalBathroom:
		updateMaps2080();
		break;
	case kMuseumOffice:
		updateMaps4030();
		break;
	case kMuseumLaboratory:
		updateMaps4050();
		break;
	case kMuseumHall:
		updateMaps4060();
		break;
	case kMuseumSecurity:
		updateMaps4100();
		break;
	default:
		break;
	}
}

const RunawayHotspot *RunawayRoomScene::hotspot(byte id) const {
	if (desertChapter())
		return id && id < _desertHotspots.size() ? &_desertHotspots[id] : nullptr;
	const RunawayRoom *room = findRoom(_room);
	return room && id && id < room->hotspotCount ? &room->hotspots[id] : nullptr;
}

byte RunawayRoomScene::hotspotAt(Common::Point mouse) const {
	if (mouse.x < 0 || mouse.x >= kRunawayScreenWidth || mouse.y < kRunawayViewportTop ||
			mouse.y >= kRunawayViewportTop + kRunawayViewportHeight)
		return 0;
	const int x = mouse.x + _camera.x, y = mouse.y - kRunawayViewportTop + _camera.y;
	if (x < 0 || x >= _background.w || y < 0 || y >= _background.h)
		return 0;
	const byte id = _maps[256 + _mask[y * _background.w + x]];
	return (_room == kMuseumKeypad && id <= 12) || hotspot(id) ? id : 0;
}

Common::U32String RunawayRoomScene::hotspotName(byte id) const {
	if (desertChapter())
		return desertHotspotName(id);
	if (_room == 6350 && id == 2 && _town.arrived)
		return Common::U32String("Douglasville");
	if (_room == 6350 && id == 3 && _town.houseVisited) {
		return Common::U32String("Mama Dorita's house");
	}
	if (_room == 6080 && id == 2 && _town.oilCan) {
		return Common::U32String("Oil can");
	}
	if (_room == 6090 && id == 4 && _town.oscarTobacco) {
		return Common::U32String("Oscar");
	}
	if (_room == 6240) {
		if (id == 5 && _town.gardeningShed) {
			return Common::U32String("Gardening shed");
		}
		if (id == 7 && _town.carnivorousPlantSeen) {
			return Common::U32String("Meat-eating plant");
		}
	}
	if (_room == 6220 && id == 8 && _town.strapSeen == 1) {
		return Common::U32String("Strap");
	}
	if (_room == 6263) {
		if (id == 3 && _town.steamLever) {
			return Common::U32String("Steam outlet lever");
		}
		if (id == 4 && _town.pressureWheel) {
			return Common::U32String("Pressure wheel");
		}
	}
	const RunawayHotspot *spot = hotspot(id);
	if (!spot)
		return Common::U32String();
	if (_room == kHospitalBathroom && id == 4 && _hospital.alcohol == 2)
		return Common::U32String("Bottle of alcohol");
	if (_room == kHospitalBedroom) {
		if (id == 9 && _hospital.chartReplaced)
			return Common::U32String("Fake medical chart");
		if (id == 10 && _hospital.chartMoved)
			return Common::U32String("Gina's medical chart");
	}
	if (_room == kHospitalStoreroom) {
		if (id == 3 && _hospital.syringesSeen)
			return Common::U32String("Box of syringes");
		if (id == 5 && _hospital.book)
			return Common::U32String("Vademecum");
		if (id == 6 && _hospital.blankChart)
			return Common::U32String("Medical chart");
	}
	return _room == kMuseumOffice ? hotspotName4030(id) : Common::U32String(spot->name);
}

Common::U32String RunawayRoomScene::actionCaption(byte id, RunawayVerb verb, byte item) const {
	if (_room == kMuseumKeypad)
		return Common::U32String();
	const Common::U32String name = hotspotName(id);
	if (item)
		return _strings.text(RunawayStrings::kUse) + _inventory.name(item) +
			(id ? _strings.text(RunawayStrings::kWith) + name : Common::U32String());
	if (!id)
		return Common::U32String();
	return _strings.text(verb == kVerbLook ? RunawayStrings::kLook : RunawayStrings::kUse) + name;
}

void RunawayRoomScene::walkTo(Common::Point target, byte facing, bool scripted, bool reserved) {
	if (scripted) {
		_walkPath.setTarget(target);
	} else if (reserved) {
		_walkPath.findPath(RunawayWalkMap(_background.w, _background.h, _mask, _originalMaps), target);
	} else {
		findWalkPath(target);
	}
	_arrivalFacing = facing;
	_walkElapsed = _lastFootstep = 0;
	if (!_walkPath.active())
		arrive();
}

void RunawayRoomScene::findWalkPath(Common::Point target) {
	if (_room == 6080 || _room == 6180 || _room == 6240 || _room == 6260) {
		walkTown(target);
	} else {
		_walkPath.findPath(RunawayWalkMap(_background.w, _background.h, _mask, _maps), target);
	}
}

void RunawayRoomScene::arrive() {
	if (_arrivalFacing != 0xff) {
		_facing = _arrivalFacing;
		_arrivalFacing = 0xff;
	}
	const byte id = _pendingHotspot;
	_pendingHotspot = 0;
	if (id) {
		_facing = hotspot(id)->facing;
		if (museumChapter() && _room == kMuseumLaboratory && id == 3 && _museum.susan == 1)
			_facing = 1;
		if (_pendingItem)
			useItem(_pendingItem, id);
		else
			execute(_pendingAction);
	}
	if (!speaking())
		_caption.clear();
}

void RunawayRoomScene::click(Common::Point mouse, RunawayVerb verb, byte item) {
	if (busy())
		return;
	const byte id = hotspotAt(mouse);
	if (_room == kMuseumKeypad) {
		clickScene4140(id);
		return;
	}
	_caption = actionCaption(id, verb, item);
	const RunawayHotspot *spot = hotspot(id);
	_pendingItem = item;
	if (spot) {
		const uint index = verb == kVerbLook && !item ? 0 : 1;
		_pendingHotspot = id;
		_pendingAction = spot->action[index];
		bool approach = spot->approach[index] != 0;
		if (_room == 6210 && !index) {
			if ((id == 5 && _town.bucketSeen) || (id == 8 && _town.bottleSeen)) {
				approach = false;
			}
		}
		if (_room == 6220 && id == 8 && index && _town.strapSeen == 1) {
			approach = true;
		}
		if (_room == 6260 && id == 5 && !index && _town.locomotiveKey == 2) {
			approach = false;
		}
		if (_room == 6130 && id == 4 && !index && _town.stone) {
			approach = false;
		}
		if (_room == kHospitalBathroom && id == 4 && !index && _hospital.alcohol == 2)
			approach = false;
		if (item) {
			approach = _game != kRunawayFullGame;
			const RunawayRoom *room = findRoom(_room);
			for (uint i = 0; i < room->itemActionCount; ++i) {
				const RunawayItemAction &action = room->itemActions[i];
				if (action.hotspot == id && (!action.item || action.item == item)) {
					approach = action.approach;
					break;
				}
			}
			if (_room == 6220 && id == 8 && _town.strapSeen != 1) {
				approach = false;
			}
		}
		if (approach) {
			Common::Point position(spot->x, spot->y);
			if (museumChapter() && _room == kMuseumLaboratory && id == 3 && _museum.susan == 1)
				position = Common::Point(912, 584);
			if (museumChapter() && _room == kMuseumSecurity && item == 5 && (id == 3 || id == 4))
				position = Common::Point(204, 448);
			walkTo(position);
		}
		else {
			_walkPath.clear();
			arrive();
		}
	} else {
		if (_room != 5010 && _room != 6350)
			walkTo(Common::Point(mouse.x + _camera.x, mouse.y - kRunawayViewportTop + _camera.y));
	}
}

void RunawayRoomScene::showLine() {
	const SpeechLine &line = _lines[_lineIndex];
	_speech.start(line.text, line.sample, line.global, _now);
	if (_game == kRunawayFullGame)
		_idleHead.reset(_now);
}

void RunawayRoomScene::say(uint16 cue, byte variation, bool global, byte speaker) {
	if (!_hangarSpeech.subtitle().empty())
		_hangarSpeech.stop();
	SceneSpeechCue speech;
	if (global)
		_text.getStaticSpeechCue(cue, variation, speech.textRecordId, speech.continuationCount, speech.voiceSampleId);
	else
		speech = _text.stageCue(cue, variation);
	if (!speech.valid())
		return;
	for (uint i = 0; i < MAX<uint>(1, speech.continuationCount); ++i) {
		SpeechLine line;
		line.text = Common::U32String(_text.largeTextRecord(speech.textRecordId + i), Common::kDos850);
		line.sample = _game == kRunawayFullGame && speech.voiceSampleId ? speech.voiceSampleId + i : 0;
		line.global = global;
		line.speaker = speaker;
		if (!line.text.empty())
			_lines.push_back(line);
	}
	if (!speaking() && _lineIndex < _lines.size())
		showLine();
}

void RunawayRoomScene::sayInventory(const RunawayInventory::Reply &reply) {
	_walkPath.clear();
	_pendingHotspot = 0;
	_caption.clear();
	if (townChapter() && reply.cue == 214) {
		say(214, _town.oilCanFull, true);
		return;
	}
	if (townChapter() && reply.cue == 227 && reply.variation == 1) {
		say(227, 1, true);
		_audio.playEffect(30);
		say(227, 2, true);
		return;
	}
	if (_game == kRunawayFullGame && !desertChapter() && !townChapter() &&
			(museumChapter() ? sayMuseumInventory(reply) : sayHospitalInventory(reply)))
		return;
	say(reply.cue, reply.variation, true);
}

void RunawayRoomScene::skipSpeech() {
	if (_sequence.active()) {
		_sequence.skip();
		return;
	}
	if (!speaking())
		return;
	_speech.stop();
	if (++_lineIndex < _lines.size()) {
		showLine();
	} else {
		_lines.clear();
		_lineIndex = 0;
		_caption.clear();
	}
}

void RunawayRoomScene::useItem(byte item, byte id) {
	const RunawayRoom *room = findRoom(_room);
	for (uint i = 0; i < room->itemActionCount; ++i) {
		const RunawayItemAction &action = room->itemActions[i];
		if (action.hotspot == id && (!action.item || action.item == item)) {
			execute(action.action);
			return;
		}
	}
	say(1, 0, true);
}

RunawayRoomScene::ActionResult RunawayRoomScene::dispatchCustomSceneAction(uint16 action) {
	ActionResult result = desertChapter() ? executeDesert(action) : kUnhandled;
	switch (_room) {
	case kHospitalBedroom:
		result = execute2020(action);
		break;
	case kHospitalStoreroom:
		result = execute2060(action);
		break;
	case kHospitalBathroom:
		result = execute2080(action);
		break;
	case kMuseumOffice:
		result = execute4030(action);
		break;
	case kMuseumLaboratory:
		result = execute4050(action);
		break;
	case kMuseumHall:
		result = execute4060(action);
		break;
	case kMuseumExhibition:
		result = execute4090(action);
		break;
	case kMuseumSecurity:
		result = execute4100(action);
		break;
	case 6014:
		result = execute6014(action);
		break;
	case 6080:
		result = execute6080(action);
		break;
	case 6090:
		result = execute6090(action);
		break;
	case 6130:
		result = execute6130(action);
		break;
	case 6180:
		result = execute6180(action);
		break;
	case 6190:
		result = execute6190(action);
		break;
	case 6200:
		result = execute6200(action);
		break;
	case 6210:
		result = execute6210(action);
		break;
	case 6220:
		result = execute6220(action);
		break;
	case 6240:
		result = execute6240(action);
		break;
	case 6260:
		result = execute6260(action);
		break;
	case 6263:
		result = execute6263(action);
		break;
	case 6350:
		result = execute6350(action);
		break;
	case 6370:
		result = execute6370(action);
		break;
	default:
		break;
	}
	return result;
}

void RunawayRoomScene::execute(uint16 action) {
	const ActionResult result = dispatchCustomSceneAction(action);
	if (result == kBackgroundChanged) {
		if (!rebuildBackground())
			error("Cannot update Runaway room %u", _room);
		updateMaps();
	}
	if (result != kUnhandled)
		return;
	if (action == 15 || action == 19) {
		say(action == 15 ? 14 : 17, _random.getRandomNumber(action == 15 ? 2 : 1), true);
		return;
	}

	const RunawayRoom *room = findRoom(_room);
	for (uint i = 0; i < room->exitCount; ++i) {
		const RunawayExit &exit = room->exits[i];
		if (exit.action != action)
			continue;
		const Common::Point entry(exit.x, exit.y);
		if (exit.destination == _room) {
			changeFloor(exit.upperFloor, entry);
		} else {
			_upperFloor = exit.upperFloor;
			_transition.scene = exit.destination;
			_transition.position = entry;
			_transition.door = exit.door;
		}
		return;
	}
	for (uint i = 0; i < room->responseCount; ++i) {
		const RunawayResponse &response = room->responses[i];
		if (response.action == action) {
			say(response.cue, response.variation);
			return;
		}
	}
	for (uint i = 0; i < ARRAYSIZE(kGlobalResponses); ++i) {
		const RunawayResponse &response = kGlobalResponses[i];
		if (response.action == action) {
			say(response.cue, response.variation, true);
			return;
		}
	}
	warning("Runaway: room %u action %u is not implemented yet", _room, action);
}

void RunawayRoomScene::setCameraX(int x) {
	_camera.x = CLIP<int>(x, _room == kHospitalBedroom ? 156 : 0, MAX<int>(0, _background.w - kRunawayScreenWidth));
}

void RunawayRoomScene::updateCamera() {
	setCameraX(_position.x - kRunawayScreenWidth / 2);
	_camera.y = sceneViewportYOffset();
}

int RunawayRoomScene::sceneViewportYOffset() const {
	return _room == kMuseumHall && _upperFloor ? 0 :
		CLIP<int>(_position.y - 400, _room == kMuseumHall ? 200 : 0, _background.h - kRunawayViewportHeight);
}

float RunawayRoomScene::actorHeight() const {
	if (desertChapter())
		return desertActorHeight();
	switch (_room) {
	case kHospitalBedroom:
		return actorHeight2020();
	case kHospitalStoreroom:
		return _position.y * 3.55555558f - 1550.33337f;
	case kHospitalBathroom:
		return _position.y * 0.266272187f + 295.502960f;
	case kMuseumOffice:
		return actorHeight4030();
	case kMuseumLaboratory:
		return actorHeight4050();
	case kMuseumHall:
		return actorHeight4060();
	case kMuseumExhibition:
		return actorHeight4090();
	case kMuseumSecurity:
		return actorHeight4100();
	case 6080:
		return _position.y * 1.12355208f - 218.007721f;
	case 6090:
		return _position.y < 510 ? _position.y * 0.840707958f - 38.7610626f :
			_position.y * 0.730337083f + 17.5280895f;
	case 6130:
		return _position.y * 1.027668f - 160.57312f;
	case 6180:
		return _position.y * 1.3565892f - 249.069763f;
	case 6240:
		return _position.y * 1.00515461f - 83.762886f;
	case 6190:
		return (_position.y * 0.426829278f + 16.4634151f + _position.x * 0.0980392173f + 41.7647057f) / 2;
	case 6200:
		return _position.y > 501 ? _position.y * 0.618556678f + 84.4845352f :
			_position.y * 1.38297868f - 299.255310f;
	case 6210:
		return _position.y * 0.413793117f + 229.482758f;
	case 6220:
		return _position.y * 0.194835678f - 11.706573f;
	case 6260:
		return _position.y * 0.95f - 175;
	case 6263:
		return _position.y * 1.04838705f - 175.645157f;
	case 6370:
		return _position.y * 0.634920657f - 221.190475f;
	default:
		return 455;
	}
}

void RunawayRoomScene::update(uint32 now, int scrollDirection, bool inventoryOpen) {
	_now = now;
	const uint elapsed = MIN<uint>(100, now - _lastUpdate);
	_lastUpdate = now;
	if (_sequence.active()) {
		_sequence.update(now);
		if (!_sequence.active()) {
			if (townChapter())
				finishTownFilm();
			else if (museumChapter())
				finishMuseumFilm();
			else if (_room == 5140)
				updateScene5140();
			else if (_entryScene == 5022)
				_chapterComplete = true;
			else if (!desertChapter())
				finishHospitalFilm();
		}
		return;
	}
	if (_pressedKey && (int32)(now - _keyUntil) >= 0)
		_pressedKey = 0;
	if (speaking() && _speech.finished(now))
		skipSpeech();
	if (_room == kHospitalBedroom)
		update2020();
	else if (_room == kHospitalBathroom)
		update2080();
	if (townChapter()) {
		updateTown(elapsed);
		if (_sequence.active() || _transition.scene)
			return;
	} else if (desertChapter()) {
		updateDesert(elapsed, inventoryOpen);
		if (_sequence.active() || _transition.scene)
			return;
	} else if (museumChapter()) {
		updateMuseum(elapsed, inventoryOpen);
		if (_sequence.active() || _transition.scene)
			return;
	} else if (_game == kRunawayFullGame) {
		if (_room != kHospitalBathroom)
			updateRoomAnimation();
		if (_sequence.active() || _transition.scene)
			return;
		updateHospitalAudio();
		if (_hospitalExit.scene && !_audio.effectPlaying()) {
			_transition = _hospitalExit;
			return;
		}
	}
	if (_room == kMuseumWilly) {
		_actor.setFrame(_facing, 0, speaking() && !speaker() ? speechFrame() : -1);
		return;
	}
	if (inventoryOpen || _room == kMuseumKeypad || _room == 5010 || _room == 5140 || _room == 6350)
		return;
	if (_walkPath.active() && !speaking()) {
		_walkPath.advance(elapsed * 0.18f * actorHeight() / 455, _position, _facing);
		_walkElapsed += elapsed;
		if (_game == kRunawayFullGame && (!_lastFootstep || _walkElapsed - _lastFootstep >= 360)) {
			const byte surface = _maps[1280 + _mask[_position.y * _background.w + _position.x]];
			const byte volume = CLIP<int>((int)(actorHeight() * 255 / 455), 0, 255);
			if (surface >= 1 && surface <= 7)
				_audio.playFootstep(surface, _random.getRandomBit(), volume);
			_lastFootstep = _walkElapsed;
		}
		if (!_walkPath.active())
			arrive();
	}
	const bool walking = _walkPath.active();
	int headFrame = -1;
	if (_game == kRunawayFullGame) {
		const bool idle = !walking && !busy();
		_idleHead.update(now, idle, _speech.frameDuration(), _random);
		if (!walking)
			headFrame = speaking() && !speaker() ? speechFrame() : idle ? _idleHead.frame() : -1;
	}
	_actor.setFrame(_facing, walking ? 1 + (_walkElapsed / 60) % 12 : 0, headFrame);
	if (scrollDirection)
		setCameraX(_camera.x + scrollDirection * MAX<int>(1, elapsed / (_fastScrolling ? 2 : 4)));
	else if (walking)
		setCameraX(_camera.x + CLIP<int>(_position.x - kRunawayScreenWidth / 2 - _camera.x,
			-MIN<int>(_fastScrolling ? 8 : 4, elapsed), MIN<int>(_fastScrolling ? 8 : 4, elapsed)));
	if (_room == kMuseumHall && !_upperFloor && (!_roomAction || _roomAction < 461 || _roomAction > 466))
		_camera.y = CLIP<int>(_position.y - 400, 200, 600);
}

void RunawayRoomScene::draw() {
	if (_sequence.active()) {
		_sequence.draw(_screen);
		return;
	}
	_screen.clear();
	_screen.copyRectToSurface(_background, 0, kRunawayViewportTop,
		Common::Rect(_camera.x, _camera.y, _camera.x + kRunawayScreenWidth, _camera.y + kRunawayViewportHeight));
	Graphics::ManagedSurface viewport(_screen, Common::Rect(0, kRunawayViewportTop, kRunawayScreenWidth, kRunawayViewportTop + kRunawayViewportHeight));
	if (_room == 5140) {
		drawScene5140(viewport);
		return;
	}
	if (_room == 6014) {
		drawScene6014(viewport);
		return;
	}
	if (_room == 6350)
		return;
	if (_room == 5010) {
		drawDesertRoom(viewport, false);
		return;
	}
	if (_room == kMuseumKeypad) {
		drawScene4140(viewport);
		return;
	}
	drawCustomBackgroundComposite(viewport);
	RunawayActor::RenderParams params;
	// Earlier rooms use six depth layers; Douglasville also uses layers 7..9.
	for (uint i = 0; i < 7; ++i)
		params.depthY[i] = 0;
	if (desertChapter())
		setupDesertDepth(params);
	switch (_room) {
	case kMuseumHall:
		setupDepth4060(params);
		break;
	case kMuseumExhibition:
		setupDepth4090(params);
		break;
	case kMuseumSecurity:
		setupDepth4100(params);
		break;
	case 6130:
		params.depthY[1] = 480;
		params.depthY[2] = 599;
		break;
	case 6190:
		params.depthY[1] = 599;
		params.depthY[2] = 388;
		break;
	case 6200:
	case 6210:
	case 6263:
		params.depthY[1] = 599;
		break;
	case 6220:
		params.depthY[1] = 263;
		params.depthY[2] = 195;
		params.depthY[3] = 490;
		params.depthY[4] = 396;
		params.depthY[5] = 227;
		params.depthY[6] = 247;
		params.depthY[7] = 599;
		params.depthY[8] = 322;
		params.depthY[9] = 599;
		break;
	case 6240:
		params.depthY[1] = 599;
		params.depthY[2] = 599;
		params.depthY[3] = 351;
		params.depthY[4] = 351;
		break;
	case 6370:
		params.depthY[1] = 800;
		params.depthY[2] = 646;
		params.depthY[3] = 400;
		params.depthY[4] = 800;
		params.depthY[5] = 645;
		params.depthY[6] = 684;
		break;
	default:
		break;
	}
	params.position = _position;
	params.camera = _camera;
	params.roomWidth = _background.w;
	params.height = actorHeight();
	params.antialias = _antialias;
	const byte light = MIN<byte>(9, _maps[768 + _mask[_position.y * _background.w + _position.x]]);
	memcpy(params.light, findRoom(_room)->light[light], sizeof(params.light));
	const bool roomAnimation = drawingRoomAnimation();
	if ((!roomAnimation || !_actionHidesActor) && _townEncounter.phase == TownEncounter::kInactive &&
			(_room != kHospitalBathroom || (_roomAction != 302 && _roomAction != 307)))
		_actor.draw(_screen, params, _mask, _maps);
	if (roomAnimation) {
		_actionAnimation.draw(viewport, _actionFrames[MIN<uint>(_actionStep, _actionFrames.size() - 1)], _camera.x, -_camera.y);
		if (!_secondaryFrames.empty())
			_secondaryAnimation.draw(viewport, _secondaryFrames[MIN<uint>(_actionStep, _secondaryFrames.size() - 1)], _camera.x, -_camera.y);
	}
	drawCustomForegroundComposite(viewport);
}

void RunawayRoomScene::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	if (_room == 6080) {
		drawScene6080(viewport, false);
	} else if (_room == 6090) {
		drawScene6090(viewport, false);
	} else if (_room == 6130) {
		_npc.draw(viewport, oscarFrame(), _camera.x, -_camera.y);
	} else if (_room == 6180) {
		drawScene6180(viewport, false);
	} else if (_room == 6200) {
		drawScene6200(viewport);
	} else if (_room == 6220) {
		drawScene6220(viewport);
	} else if (_room == 6263) {
		drawScene6263(viewport);
	} else if (_room == 6370) {
		drawScene6370(viewport);
	} else if (desertChapter()) {
		drawDesertRoom(viewport, false);
	} else if (_room == kMuseumWilly) {
		drawScene4070(viewport);
	} else if (_room == kHospitalBedroom) {
		drawScene2020(viewport, false);
	} else if (_room == kHospitalBathroom) {
		drawScene2080(viewport, false);
	} else if (_room == kMuseumSecurity && museumChapter()) {
		drawScene4100(viewport);
	} else {
		const bool movingSusan = museumChapter() && _room == kMuseumLaboratory && drawingRoomAnimation() &&
			(_roomAction == 450 || _roomAction == 451 || _roomAction == 452 || _roomAction == 331 || _roomAction == 454);
		if (!movingSusan && (!museumChapter() || _room != kMuseumHall || !_museum.willyInLab))
			_ambient.draw(viewport, museumChapter() ? _ambientFrame : 0, _camera.x, -_camera.y);
		_overlay.draw(viewport, 0, _camera.x, -_camera.y);
	}
}

void RunawayRoomScene::drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) {
	if (desertChapter())
		drawDesertRoom(viewport, true);
	else if (_room == kHospitalBedroom)
		drawScene2020(viewport, true);
	else if (_room == kHospitalBathroom)
		drawScene2080(viewport, true);
	else if (_room == kMuseumOffice)
		drawForeground4030(viewport);
	else if (_room == kMuseumLaboratory)
		drawForeground4050(viewport);
	else if (_room == 6080) {
		drawScene6080(viewport, true);
	} else if (_room == 6090) {
		drawScene6090(viewport, true);
	} else if (_room == 6180) {
		drawScene6180(viewport, true);
	} else if (_room == 6240) {
		drawForeground6240(viewport);
	} else if (_room == 6260) {
		drawForeground(viewport, 0);
	}
}

void RunawayRoomScene::drawForeground(Graphics::ManagedSurface &viewport, uint layer) {
	assert(layer < ARRAYSIZE(_foreground));
	for (uint span = 0; span < _foreground[layer].size(); ++span) {
		const Common::Rect &rect = _foreground[layer][span];
		viewport.blitFrom(_background, rect, Common::Point(rect.left - _camera.x, rect.top - _camera.y));
	}
}

void RunawayRoomScene::beginRoomAnimation(uint16 action, uint chunk, uint frames, const byte *order, uint count, bool hideActor, uint frameDuration) {
	assert(frameDuration);
	if (!_actionAnimation.load(_archive, chunk, frames))
		error("Cannot load Runaway room %u animation %u", _room, chunk);
	_actionFrames.clear();
	_secondaryFrames.clear();
	_actionHoldSpeech = false;
	_actionLoopSpeech = false;
	for (uint i = 0; i < (order ? count : frames); ++i)
		_actionFrames.push_back(order ? order[i] : i);
	_roomAction = action;
	_actionHidesActor = hideActor;
	_actionStage = 0;
	_actionStep = 0;
	_actionFrameDuration = frameDuration;
	_actionStarted = _now;
}

void RunawayRoomScene::beginRoomAnimationRange(uint16 action, uint chunk, uint frames, uint first, uint last, bool hideActor, uint frameDuration) {
	Common::Array<byte> order;
	assert(first < frames && last < frames && frames <= 256);
	for (int frame = first;; frame += first <= last ? 1 : -1) {
		order.push_back(frame);
		if (frame == (int)last)
			break;
	}
	beginRoomAnimation(action, chunk, frames, order.data(), order.size(), hideActor, frameDuration);
}

void RunawayRoomScene::holdRoomAnimation(uint16 action, uint chunk, uint frames, uint frame, bool hideActor) {
	beginRoomAnimationRange(action, chunk, frames, frame, frame, hideActor);
	_actionHoldSpeech = true;
}

void RunawayRoomScene::loopRoomAnimationDuringSpeech(uint16 action, uint chunk, uint frames, uint first, uint last, bool hideActor) {
	beginRoomAnimationRange(action, chunk, frames, first, last, hideActor);
	_actionHoldSpeech = true;
	_actionLoopSpeech = true;
}

void RunawayRoomScene::setRoomAnimationOverlay(uint chunk, uint frames, const byte *order, uint count) {
	if (!_secondaryAnimation.load(_archive, chunk, frames))
		error("Cannot load Runaway room %u overlay %u", _room, chunk);
	_secondaryFrames.assign(order, order + count);
}

void RunawayRoomScene::updateRoomAnimation() {
	if (!_roomAction || _actionFrames.empty())
		return;
	if ((speaking() && !_actionLoopSpeech) || _walkPath.active()) {
		_actionStarted = _now;
		return;
	}
	while (_now - _actionStarted >= _actionFrameDuration) {
		_actionStarted += _actionFrameDuration;
		if (++_actionStep == _actionFrames.size()) {
			if (_actionLoopSpeech && speaking()) {
				_actionStep = 0;
				continue;
			}
			const uint16 action = _roomAction;
			_roomAction = 0;
			_actionFrames.clear();
			if (townChapter())
				execute(action + 1000);
			else if (desertChapter())
				finishDesertAnimation(action);
			else if (museumChapter())
				finishMuseumAnimation(action);
			else
				finishHospitalAnimation(action);
			return;
		}
		if (desertChapter()) {
			desertAnimationFrame();
			continue;
		}
		if (museumChapter()) {
			museumAnimationFrame();
			continue;
		}
		if (townChapter()) {
			townAnimationFrame();
			continue;
		}
		if ((_roomAction == 335 && _actionStep == 15) || (_roomAction == 339 && _actionStep == 47))
			_audio.playEffect(46);
		if (_roomAction == 339 && _actionStep == 16)
			_audio.playEffect(45);
		if (_roomAction == 339 && _actionStep == 1 && !applyPatch(18))
			error("Cannot clear Runaway hospital chart");
		if ((_roomAction == 332 && (_actionStep == 21 || _actionStep == 35 || _actionStep == 41)) ||
				(_room == kHospitalStoreroom && _roomAction == 302 && (_actionStep == 23 || _actionStep == 36)))
			_audio.playFootstep(1, _random.getRandomBit());
	}
}

} // End of namespace Hollywood
