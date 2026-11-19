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

#include <math.h>

namespace Hollywood {

namespace {

const RunawayRoom *const kRooms[] = {
	&kRunawayScene2020, &kRunawayScene2060, &kRunawayScene2080,
	&kRunawayScene4030, &kRunawayScene4050, &kRunawayScene4060, &kRunawayScene4090, &kRunawayScene4100, &kRunawayScene4140
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
	_audio(audio), _sequence(audio), _inventory(inventory), _strings(strings), _game(game), _random("runaway") {}

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
	if (_room == kHospitalBedroom) {
		start2020();
		resume();
		return;
	}
	const byte initialItems[] = {1, 2, 3, 4, 7};
	for (uint i = 0; i < ARRAYSIZE(initialItems); ++i)
		_inventory.add(initialItems[i]);
	_inventory.takeNewItem();
	resume();
}

void RunawayRoomScene::resume() {
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
					beginHospitalAnimation(_entryScene, _room == kHospitalStoreroom ? 10 : 23, 13, frames, ARRAYSIZE(frames));
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
	state.randomSeed = _random.getSeed();
}

bool RunawayRoomScene::restoreState(const RunawaySaveState &state, uint32 now, bool entering) {
	if (!state.valid() || state.game != _game || state.scene != _entryScene)
		return false;
	_hospital = state.hospital;
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
	_x = state.x;
	_y = state.y;
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
	const Common::Path path(Common::String::format("RESOURCE.%c%02u", 'A' + number / 1000 - 1, (number % 1000) / 10));
	if (!_archive.open(path, 40) ||
			!loadRunawayMask(_archive, room->width, room->height, _mask, _originalMaps) ||
			!_text.load("RESOURCE.003", "Runaway room", number / 10, 0, 0x1135))
		return false;
	if (_game == kRunawayFullGame ? !_actor.loadChapter(1, true) :
			!_actor.load(number == kMuseumHall ? RunawayActor::kMuseumHall : RunawayActor::kMuseumInterior))
		return false;
	_room = number;
	_position = position;
	_x = position.x;
	_y = position.y;
	_facing = facing;
	_path.clear();
	_pathIndex = _walkElapsed = 0;
	_pendingHotspot = _pendingItem = 0;
	_pendingAction = 0;
	_lines.clear();
	_lineIndex = 0;
	_subtitle.clear();
	_caption.clear();
	_ambient = RunawayAnimation();
	_overlay = RunawayAnimation();
	for (uint i = 0; i < ARRAYSIZE(_foreground); ++i)
		_foreground[i].clear();
	for (uint i = 0; i < ARRAYSIZE(_keyPatches); ++i)
		_keyPatches[i].clear();
	bool loaded = true;
	switch (number) {
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
	case kMuseumKeypad:
		loaded = loadScene4140();
		break;
	default:
		break;
	}
	if (!loaded || !rebuildBackground())
		return false;
	_keyCount = _pressedKey = 0;
	updateMaps();
	updateCamera();
	return _actor.setFrame(_facing, 0);
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
	const RunawayRoom *room = findRoom(_room);
	if (!loadRunawaySurface(_archive, 0, room->width, room->height, _background))
		return false;
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
	default:
		return true;
	}
}

void RunawayRoomScene::updateMaps() {
	_maps = _originalMaps;
	switch (_room) {
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
	default:
		break;
	}
}

const RunawayHotspot *RunawayRoomScene::hotspot(byte id) const {
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

void RunawayRoomScene::walkTo(Common::Point target, byte facing, bool scripted) {
	if (scripted) {
		_path.clear();
		_path.push_back(target);
	} else {
		RunawayWalkMap(_background.w, _background.h, _mask, _maps).findPath(_position, target, _path);
	}
	_arrivalFacing = facing;
	_pathIndex = _walkElapsed = _lastFootstep = 0;
	if (_path.empty())
		arrive();
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
		}
		if (approach)
			walkTo(Common::Point(spot->x, spot->y));
		else {
			_path.clear();
			_pathIndex = 0;
			arrive();
		}
	} else {
		walkTo(Common::Point(mouse.x + _camera.x, mouse.y - kRunawayViewportTop + _camera.y));
	}
}

void RunawayRoomScene::showLine() {
	const SpeechLine &line = _lines[_lineIndex];
	_subtitle = line.text;
	const uint32 duration = line.sample ? _audio.playSpeech(line.sample, line.global) : 0;
	_subtitleUntil = _now + (duration ? MAX<uint>(1000, duration + 150) : MAX<uint>(3000, _subtitle.size() * 55));
	if (_game == kRunawayFullGame) {
		_idleHead.reset(_now);
		_lipSync.start(_subtitle, _now, duration, _random, line.sample);
	}
}

void RunawayRoomScene::say(uint16 cue, byte variation, bool global) {
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
		if (!line.text.empty())
			_lines.push_back(line);
	}
	if (!speaking() && _lineIndex < _lines.size())
		showLine();
}

void RunawayRoomScene::sayInventory(const RunawayInventory::Reply &reply) {
	_path.clear();
	_pathIndex = 0;
	_pendingHotspot = 0;
	_caption.clear();
	if (_game == kRunawayFullGame && sayHospitalInventory(reply))
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
	_audio.stopSpeech();
	_lipSync.stop();
	if (++_lineIndex < _lines.size()) {
		showLine();
	} else {
		_lines.clear();
		_lineIndex = 0;
		_subtitle.clear();
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

void RunawayRoomScene::execute(uint16 action) {
	ActionResult result = kUnhandled;
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
	default:
		break;
	}
	if (result == kBackgroundChanged) {
		if (!rebuildBackground())
			error("Cannot update Runaway room %u", _room);
		updateMaps();
	}
	if (result != kUnhandled)
		return;

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
	_camera.y = _room == kMuseumHall && _upperFloor ? 0 :
		CLIP<int>(_position.y - 400, _room == kMuseumHall ? 200 : 0, _background.h - kRunawayViewportHeight);
}

float RunawayRoomScene::actorHeight() const {
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
		if (!_sequence.active())
			finishHospitalFilm();
		return;
	}
	if (_pressedKey && (int32)(now - _keyUntil) >= 0)
		_pressedKey = 0;
	if (speaking() && (int32)(now - _subtitleUntil) >= 0)
		skipSpeech();
	if (_room == kHospitalBedroom)
		update2020();
	else if (_room == kHospitalBathroom)
		update2080();
	if (_game == kRunawayFullGame) {
		if (_room != kHospitalBathroom)
			updateHospitalAnimation();
		if (_sequence.active() || _transition.scene)
			return;
		updateHospitalAudio();
		if (_hospitalExit.scene && !_audio.effectPlaying()) {
			_transition = _hospitalExit;
			return;
		}
	}
	if (inventoryOpen || _room == kMuseumKeypad)
		return;
	if (_pathIndex < _path.size() && !speaking()) {
		float travel = elapsed * 0.18f * actorHeight() / 455;
		while (travel > 0 && _pathIndex < _path.size()) {
			const Common::Point target = _path[_pathIndex];
			const float dx = target.x - _x, dy = target.y - _y;
			const float distance = sqrtf(dx * dx + dy * dy);
			if (distance > 0) {
				if (ABS(dx) > ABS(dy) * 2)
					_facing = dx > 0 ? 2 : 6;
				else if (ABS(dy) > ABS(dx) * 2)
					_facing = dy > 0 ? 4 : 0;
				else
					_facing = dy > 0 ? (dx > 0 ? 3 : 5) : (dx > 0 ? 1 : 7);
			}
			if (distance <= travel) {
				_x = target.x;
				_y = target.y;
				travel -= distance;
				++_pathIndex;
			} else {
				_x += dx * travel / distance;
				_y += dy * travel / distance;
				travel = 0;
			}
		}
		_position = Common::Point((int)(_x + 0.5f), (int)(_y + 0.5f));
		_walkElapsed += elapsed;
		if (_game == kRunawayFullGame && (!_lastFootstep || _walkElapsed - _lastFootstep >= 360)) {
			const byte surface = _maps[1280 + _mask[_position.y * _background.w + _position.x]];
			const byte volume = CLIP<int>((int)(actorHeight() * 255 / 455), 0, 255);
			if (surface >= 1 && surface <= 7)
				_audio.playFootstep(surface, _random.getRandomBit(), volume);
			_lastFootstep = _walkElapsed;
		}
		if (_pathIndex == _path.size())
			arrive();
	}
	const bool walking = _pathIndex < _path.size();
	int headFrame = -1;
	if (_game == kRunawayFullGame) {
		const bool idle = !walking && !busy();
		_idleHead.update(now, idle, _lipSync.frameDuration(), _random);
		if (!walking)
			headFrame = speaking() ? speechFrame() : idle ? _idleHead.frame() : -1;
	}
	_actor.setFrame(_facing, walking ? 1 + (_walkElapsed / 60) % 12 : 0, headFrame);
	if (scrollDirection)
		setCameraX(_camera.x + scrollDirection * MAX<int>(1, elapsed / (_fastScrolling ? 2 : 4)));
	else if (walking)
		setCameraX(_camera.x + CLIP<int>(_position.x - kRunawayScreenWidth / 2 - _camera.x,
			-MIN<int>(_fastScrolling ? 8 : 4, elapsed), MIN<int>(_fastScrolling ? 8 : 4, elapsed)));
	if (_room == kMuseumHall && !_upperFloor)
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
	if (_room == kMuseumKeypad) {
		drawScene4140(viewport);
		return;
	}
	if (_room == kHospitalBedroom) {
		drawScene2020(viewport, false);
	} else if (_room == kHospitalBathroom) {
		drawScene2080(viewport, false);
	} else {
		_ambient.draw(viewport, 0, _camera.x, -_camera.y);
		_overlay.draw(viewport, 0, _camera.x, -_camera.y);
	}
	RunawayActor::RenderParams params;
	memset(params.depthY, 0, sizeof(params.depthY));
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
	const bool roomAnimation = !_actionFrames.empty() && _roomAction && !speaking() && _pathIndex == _path.size();
	if (!roomAnimation && (_room != kHospitalBathroom || (_roomAction != 302 && _roomAction != 307)))
		_actor.draw(_screen, params, _mask, _maps);
	if (roomAnimation)
		_actionAnimation.draw(viewport, _actionFrames[MIN<uint>(_actionStep, _actionFrames.size() - 1)], _camera.x, -_camera.y);
	if (_room == kHospitalBedroom)
		drawScene2020(viewport, true);
	else if (_room == kHospitalBathroom)
		drawScene2080(viewport, true);
	else if (_room == kMuseumOffice)
		drawForeground4030(viewport);
	else if (_room == kMuseumLaboratory)
		drawForeground4050(viewport);
}

void RunawayRoomScene::drawForeground(Graphics::ManagedSurface &viewport, uint layer) {
	assert(layer < ARRAYSIZE(_foreground));
	for (uint span = 0; span < _foreground[layer].size(); ++span) {
		const Common::Rect &rect = _foreground[layer][span];
		viewport.blitFrom(_background, rect, Common::Point(rect.left - _camera.x, rect.top - _camera.y));
	}
}

} // End of namespace Hollywood
