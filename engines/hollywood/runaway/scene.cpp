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

const MuseumRoom *const kRooms[] = {
	&kRunawayScene4030, &kRunawayScene4050, &kRunawayScene4060, &kRunawayScene4090, &kRunawayScene4100, &kRunawayScene4140
};

const MuseumResponse kGlobalResponses[] = {
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

const MuseumRoom *findRoom(int number) {
	for (uint i = 0; i < ARRAYSIZE(kRooms); ++i) {
		if (kRooms[i]->number == number)
			return kRooms[i];
	}
	return nullptr;
}

} // End of anonymous namespace

const MuseumRoom *findRunawayMuseumRoom(uint16 number) {
	return findRoom(number);
}

RunawayMuseumScene::RunawayMuseumScene(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings) :
	_audio(audio), _inventory(inventory), _strings(strings) {}

bool RunawayMuseumScene::load(uint16 entryScene) {
	const MuseumRoom *room = findRoom(entryScene);
	if (!room)
		return false;
	_entryScene = entryScene;
	const Graphics::PixelFormat format(2, 5, 6, 5, 0, 11, 5, 0, 0);
	_screen.create(kRunawayScreenWidth, kRunawayScreenHeight, format);
	return enterRoom(room->number, room->entry, 3);
}

void RunawayMuseumScene::start(uint32 now) {
	_now = _lastUpdate = now;
	const byte initialItems[] = {1, 2, 3, 4, 7};
	for (uint i = 0; i < ARRAYSIZE(initialItems); ++i)
		_inventory.add(initialItems[i]);
	_inventory.takeNewItem();
	resume();
}

void RunawayMuseumScene::resume() {
	// The original demo loops M04 cue 11 throughout gameplay (0x4175c0).
	if (!_audio.musicPlaying())
		_audio.playMusic(11);
}

void RunawayMuseumScene::captureState(RunawaySaveState &state) const {
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
}

bool RunawayMuseumScene::restoreState(const RunawaySaveState &state, uint32 now, bool entering) {
	if (!state.valid() || state.game != kRunawayMuseumDemo || state.scene != _entryScene)
		return false;
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
	if (entering)
		updateCamera();
	else
		_camera = Common::Point(state.camera, state.cameraY);
	return true;
}

bool RunawayMuseumScene::enterRoom(uint16 number, Common::Point position, byte facing) {
	const MuseumRoom *room = findRoom(number);
	if (!room || position.x < 0 || position.x >= room->width || position.y < 0 || position.y >= room->height)
		return false;
	const Common::Path path(Common::String::format("RESOURCE.D%02u", number / 10 - 400));
	if (!_archive.open(path, 40) ||
			!loadRunawayMask(_archive, room->width, room->height, _mask, _originalMaps) ||
			!_text.load("RESOURCE.003", "Runaway museum", number / 10, 0, 0x1135) ||
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

bool RunawayMuseumScene::loadForeground(uint layer, uint chunk) {
	assert(layer < ARRAYSIZE(_foreground));
	const MuseumRoom *room = findRoom(_room);
	// Foreground masks restore (x, y, length) spans from the current background.
	Common::Array<byte> spans;
	if (!_archive.readVariableChunk(chunk, spans) || spans.size() < 2 ||
			spans.size() != 2U + READ_LE_UINT16(spans.data()) * 6U)
		return false;
	for (uint offset = 2; offset < spans.size(); offset += 6) {
		const uint x = READ_LE_UINT16(spans.data() + offset);
		const uint y = READ_LE_UINT16(spans.data() + offset + 2);
		const uint width = READ_LE_UINT16(spans.data() + offset + 4);
		if (!width || x + width > room->width || y >= room->height)
			return false;
		_foreground[layer].push_back(Common::Rect(x, y, x + width, y + 1));
	}
	return true;
}

bool RunawayMuseumScene::applyPatch(uint chunk) {
	Common::Array<byte> data;
	return _archive.readVariableChunk(chunk, data) && applyRunawayPatch(data, _background);
}

bool RunawayMuseumScene::rebuildBackground() {
	const MuseumRoom *room = findRoom(_room);
	if (!loadRunawaySurface(_archive, 0, room->width, room->height, _background))
		return false;
	switch (_room) {
	case kMuseumOffice:
		return applyBackground4030();
	case kMuseumLaboratory:
		return applyBackground4050();
	default:
		return true;
	}
}

void RunawayMuseumScene::updateMaps() {
	_maps = _originalMaps;
	switch (_room) {
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

const MuseumHotspot *RunawayMuseumScene::hotspot(byte id) const {
	const MuseumRoom *room = findRoom(_room);
	return room && id && id < room->hotspotCount ? &room->hotspots[id] : nullptr;
}

byte RunawayMuseumScene::hotspotAt(Common::Point mouse) const {
	if (mouse.x < 0 || mouse.x >= kRunawayScreenWidth || mouse.y < kRunawayViewportTop ||
			mouse.y >= kRunawayViewportTop + kRunawayViewportHeight)
		return 0;
	const int x = mouse.x + _camera.x, y = mouse.y - kRunawayViewportTop + _camera.y;
	if (x < 0 || x >= _background.w || y < 0 || y >= _background.h)
		return 0;
	const byte id = _maps[256 + _mask[y * _background.w + x]];
	return (_room == kMuseumKeypad && id <= 12) || hotspot(id) ? id : 0;
}

Common::U32String RunawayMuseumScene::hotspotName(byte id) const {
	const MuseumHotspot *spot = hotspot(id);
	if (!spot)
		return Common::U32String();
	return _room == kMuseumOffice ? hotspotName4030(id) : Common::U32String(spot->name);
}

Common::U32String RunawayMuseumScene::actionCaption(byte id, RunawayVerb verb, byte item) const {
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

void RunawayMuseumScene::walkTo(Common::Point target) {
	RunawayWalkMap(_background.w, _background.h, _mask, _maps).findPath(_position, target, _path);
	_pathIndex = _walkElapsed = 0;
	if (_path.empty())
		arrive();
}

void RunawayMuseumScene::arrive() {
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

void RunawayMuseumScene::click(Common::Point mouse, RunawayVerb verb, byte item) {
	if (busy())
		return;
	const byte id = hotspotAt(mouse);
	if (_room == kMuseumKeypad) {
		clickScene4140(id);
		return;
	}
	_caption = actionCaption(id, verb, item);
	const MuseumHotspot *spot = hotspot(id);
	_pendingItem = item;
	if (spot) {
		const uint index = verb == kVerbLook && !item ? 0 : 1;
		_pendingHotspot = id;
		_pendingAction = spot->action[index];
		if (spot->approach[index] || item)
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

void RunawayMuseumScene::queueLine(const Common::U32String &line) {
	if (line.empty())
		return;
	_lines.push_back(line);
	if (!speaking())
		showLine();
}

void RunawayMuseumScene::showLine() {
	_subtitle = _lines[_lineIndex];
	_subtitleUntil = _now + MAX<uint>(3000, _subtitle.size() * 55);
}

void RunawayMuseumScene::say(uint16 cue, byte variation, bool global) {
	SceneSpeechCue speech;
	if (global)
		_text.getStaticSpeechCue(cue, variation, speech.textRecordId, speech.continuationCount, speech.voiceSampleId);
	else
		speech = _text.stageCue(cue, variation);
	if (!speech.valid())
		return;
	for (uint i = 0; i < MAX<uint>(1, speech.continuationCount); ++i)
		queueLine(Common::U32String(_text.largeTextRecord(speech.textRecordId + i), Common::kDos850));
}

void RunawayMuseumScene::sayInventory(const RunawayInventory::Reply &reply) {
	_path.clear();
	_pathIndex = 0;
	_pendingHotspot = 0;
	_caption.clear();
	say(reply.cue, reply.variation, true);
}

void RunawayMuseumScene::skipSpeech() {
	if (!speaking())
		return;
	if (++_lineIndex < _lines.size()) {
		showLine();
	} else {
		_lines.clear();
		_lineIndex = 0;
		_subtitle.clear();
		_caption.clear();
	}
}

void RunawayMuseumScene::useItem(byte item, byte id) {
	const MuseumRoom *room = findRoom(_room);
	for (uint i = 0; i < room->itemActionCount; ++i) {
		const MuseumItemAction &action = room->itemActions[i];
		if (action.hotspot == id && (!action.item || action.item == item)) {
			execute(action.action);
			return;
		}
	}
	say(1, 0, true);
}

void RunawayMuseumScene::execute(uint16 action) {
	ActionResult result = kUnhandled;
	switch (_room) {
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
			error("Cannot update museum room %u", _room);
		updateMaps();
	}
	if (result != kUnhandled)
		return;

	const MuseumRoom *room = findRoom(_room);
	for (uint i = 0; i < room->exitCount; ++i) {
		const MuseumExit &exit = room->exits[i];
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
		const MuseumResponse &response = room->responses[i];
		if (response.action == action) {
			say(response.cue, response.variation);
			return;
		}
	}
	for (uint i = 0; i < ARRAYSIZE(kGlobalResponses); ++i) {
		const MuseumResponse &response = kGlobalResponses[i];
		if (response.action == action) {
			say(response.cue, response.variation, true);
			return;
		}
	}
	warning("Runaway museum: room %u action %u is not implemented yet", _room, action);
}

void RunawayMuseumScene::setCameraX(int x) {
	_camera.x = CLIP<int>(x, 0, MAX<int>(0, _background.w - kRunawayScreenWidth));
}

void RunawayMuseumScene::updateCamera() {
	setCameraX(_position.x - kRunawayScreenWidth / 2);
	_camera.y = _room == kMuseumHall && _upperFloor ? 0 :
		CLIP<int>(_position.y - 400, _room == kMuseumHall ? 200 : 0, _background.h - kRunawayViewportHeight);
}

float RunawayMuseumScene::actorHeight() const {
	switch (_room) {
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

void RunawayMuseumScene::update(uint32 now, int scrollDirection, bool inventoryOpen) {
	_now = now;
	const uint elapsed = MIN<uint>(100, now - _lastUpdate);
	_lastUpdate = now;
	if (_pressedKey && (int32)(now - _keyUntil) >= 0)
		_pressedKey = 0;
	if (speaking() && (int32)(now - _subtitleUntil) >= 0)
		skipSpeech();
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
		if (_pathIndex == _path.size())
			arrive();
	}
	const bool walking = _pathIndex < _path.size();
	_actor.setFrame(_facing, walking ? 1 + (_walkElapsed / 60) % 12 : 0);
	if (scrollDirection)
		setCameraX(_camera.x + scrollDirection * MAX<int>(1, elapsed / (_fastScrolling ? 2 : 4)));
	else if (walking)
		setCameraX(_camera.x + CLIP<int>(_position.x - kRunawayScreenWidth / 2 - _camera.x,
			-MIN<int>(_fastScrolling ? 8 : 4, elapsed), MIN<int>(_fastScrolling ? 8 : 4, elapsed)));
	if (_room == kMuseumHall && !_upperFloor)
		_camera.y = CLIP<int>(_position.y - 400, 200, 600);
}

void RunawayMuseumScene::draw() {
	_screen.clear();
	_screen.copyRectToSurface(_background, 0, kRunawayViewportTop,
		Common::Rect(_camera.x, _camera.y, _camera.x + kRunawayScreenWidth, _camera.y + kRunawayViewportHeight));
	Graphics::ManagedSurface viewport(_screen, Common::Rect(0, kRunawayViewportTop, kRunawayScreenWidth, kRunawayViewportTop + kRunawayViewportHeight));
	if (_room == kMuseumKeypad) {
		drawScene4140(viewport);
		return;
	}
	_ambient.draw(viewport, 0, _camera.x, -_camera.y);
	_overlay.draw(viewport, 0, _camera.x, -_camera.y);
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
	_actor.draw(_screen, params, _mask, _maps);
	if (_room == kMuseumOffice)
		drawForeground4030(viewport);
	else if (_room == kMuseumLaboratory)
		drawForeground4050(viewport);
}

void RunawayMuseumScene::drawForeground(Graphics::ManagedSurface &viewport, uint layer) {
	assert(layer < ARRAYSIZE(_foreground));
	for (uint span = 0; span < _foreground[layer].size(); ++span) {
		const Common::Rect &rect = _foreground[layer][span];
		viewport.blitFrom(_background, rect, Common::Point(rect.left - _camera.x, rect.top - _camera.y));
	}
}

} // End of namespace Hollywood
