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
	{0, 31, 25, 0},
};

// New bindings for the original, unused response handlers.
const RunawayItemAction kRestoredItemActions[] = {
	{kHospitalStoreroom, 2, 8, 9, 0}, // Try the car keys in the left file cabinet.
	{kHospitalStoreroom, 2, 9, 9, 0}, // Try the car keys in the right file cabinet.
	{kHospitalStoreroom, 15, 3, 27, 0}, // Return the syringe to its box.
	{kHospitalStoreroom, 17, 7, 27, 0}, // Return the head to the anatomical model.
	{kHospitalStoreroom, 19, 11, 26, 0}, // Return the medical reference book to its box.
	{kHospitalBathroom, 11, 1, 26, 0}, // Return the marker to the wastebasket.
	{kHospitalBathroom, 11, 9, 12, 0}, // Discard the marker in the toilet.
	{kHospitalBathroom, 15, 1, 25, 0}, // Discard the syringe in the wastebasket.
	{kHospitalBathroom, 15, 9, 25, 0}, // Discard the syringe in the toilet.
	{kHospitalBathroom, 19, 1, 12, 0}, // Discard the medical reference book in the wastebasket.
	{kHospitalBathroom, 19, 9, 12, 0}, // Discard the medical reference book in the toilet.
	{kMuseumLaboratory, 6, 7, 25, 0}, // Discard the battery in the wastepaper basket.
	{kMuseumLaboratory, 9, 7, 12, 0}, // Discard the varnish in the wastepaper basket.
	{kMuseumLaboratory, 9, 12, 26, 0}, // Return the varnish to the briefcase.
	{6190, 20, 4, 27, 0}, // Return the ancient stapler to the bank shelves.
};

} // End of anonymous namespace

RunawayRoomScene::RunawayRoomScene(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	_audio(audio), _sequence(audio, game == kRunawayMuseumDemo ? RunawaySpeech::kRandomLipSync : RunawaySpeech::kRecordedLipSync),
	_inventory(inventory), _strings(strings), _game(game), _random("runaway"),
	_speech(audio, _random, game == kRunawayMuseumDemo ? RunawaySpeech::kRandomLipSync : RunawaySpeech::kRecordedLipSync) {}

const Common::U32String &RunawayRoomScene::subtitle() const {
	if (_sequence.active())
		return _sequence.subtitle();
	return _speech.subtitle();
}

bool RunawayRoomScene::load(uint16 entryScene) {
	const RunawayRoom *room = findRunawayRoom(entryScene);
	if (!room)
		return false;
	_entryScene = entryScene;
	const Graphics::PixelFormat format(2, 5, 6, 5, 0, 11, 5, 0, 0);
	_screen.create(kRunawayScreenWidth, kRunawayScreenHeight, format);
	return enterRoom(room->number, room->entry, initialActorFacing());
}

void RunawayRoomScene::start(uint32 now) {
	_now = _lastUpdate = now;
	_idleHead.reset(now);
	_idleBody.reset(now);
	if (townChapter() || hopiChapter() || finaleChapter()) {
		_inventory.startChapter(finaleChapter() ? 6 : hopiChapter() ? 5 : 4);
		_entering = true;
		resume();
		return;
	}
	if (desertChapter()) {
		startDesert();
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
	if (hopiChapter() || finaleChapter()) {
		prepareCustomGameplayLoop();
		resumeAmbientAudio(_entering);
		if (_entering) {
			_entering = false;
			runCustomEntrySequence();
		}
		return;
	}
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
		resumeHospital();
		return;
	}
	// The original demo loops M04 cue 11 throughout gameplay (0x4175c0).
	if (!_audio.musicPlaying())
		_audio.playMusic(11);
}

void RunawayRoomScene::resumeAmbientAudio(bool entering) {
	// Close-ups and conversations retain the surrounding scene's music.
	const bool keepMusic = _room == 4140 || _room == 5130 || _room == 5140 || _room == 6270 || _room == 6390;
	if (keepMusic) {
		return;
	}
	if (!_audio.musicPlaying() || (entering && _audio.musicLooping())) {
		const uint16 cue = museumChapter() ? 13 : desertChapter() ? 12 :
			townChapter() ? 12 + _random.getRandomNumber(4) : 11;
		_audio.playMusic(cue, _game != kRunawayFullGame);
	}
}

void RunawayRoomScene::updateAmbientMusic() {
	if (museumChapter()) {
		if (_room == 4100) {
			if (!_audio.musicPlaying()) {
				_audio.playMusic(11);
			}
		} else if (_room != 4090 && _room != 4140) {
			_audio.updateBackgroundMusic(_now, _random, 14, 3);
		}
	} else if (desertChapter()) {
		if (_room == 5160 || _room == 5170) {
			if (!_audio.musicPlaying()) {
				_audio.playMusic(16);
				_audio.setMusicVolume(_room == 5170 ? 70 : 100);
			}
		} else if (_game == kRunawayFullGame && _room != 5060 && _room != 5130 && _room != 5140) {
			// The cabin demos omit the outdoor background tracks.
			_audio.updateBackgroundMusic(_now, _random, 13, 2);
		}
	} else if (townChapter()) {
		switch (_room) {
		case 6200:
		case 6210:
			_audio.updateBackgroundMusic(_now, _random, 18, 1, 75);
			break;
		case 6250:
			_audio.updateBackgroundMusic(_now, _random, 22, 1);
			break;
		case 6290:
			_audio.updateBackgroundMusic(_now, _random, _town.joshuaGone ? 12 : 19, _town.joshuaGone ? 5 : 1);
			break;
		case 6300:
			if (_town.kevinAway) {
				updateOutdoorMusic(5);
			}
			break;
		case 6270:
		case 6280:
		case 6370:
		case 6390:
			break;
		default:
			updateOutdoorMusic(5);
			break;
		}
	}
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
	state.hopi = _hopi;
	state.finale = _finale;
	state.randomSeed = _random.getSeed();
}

bool RunawayRoomScene::restoreState(const RunawaySaveState &state, uint32 now, bool entering) {
	if (!state.valid() || state.game != _game || state.scene != _entryScene)
		return false;
	_hospital = state.hospital;
	_museum = state.museum;
	_desert = state.desert;
	_town = state.town;
	_hopi = state.hopi;
	_finale = state.finale;
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
	_cameraFollow.reset();
	_facing = state.facing;
	_keyCount = entering ? 0 : state.keyCount;
	_now = _lastUpdate = now;
	_random.setSeed(state.randomSeed);
	_idleHead.reset(now);
	_idleBody.reset(now);
	if (entering)
		updateCamera();
	else
		_camera = Common::Point(state.camera, state.cameraY);
	return true;
}

bool RunawayRoomScene::enterRoom(uint16 number, Common::Point position, byte facing) {
	const RunawayRoom *room = findRunawayRoom(number);
	if (!room || position.x < 0 || position.x >= room->width || position.y < 0 || position.y >= room->height)
		return false;
	_roomData = room;
	_room = number;
	const Common::Path path(Common::String::format("RESOURCE.%c%02u", 'A' + number / 1000 - 1, sceneResourceNumber()));
	if (!_archive.open(path, 40) ||
			!_text.load("RESOURCE.003", "Runaway room", sceneTextNumber(), 0, 0x1135))
		return false;
	_walkRoutes.clear();
	if (!loadWalkMap()) {
		return false;
	}
	if (_game != kRunawayMuseumDemo ? !_actor.loadChapter(number < 4000 ? 1 : number < 5000 ? 2 : number < 6000 ? 3 : 4,
			_game == kRunawayFullGame && usesAlternateActor()) :
			!_actor.load(usesAlternateActor() ? RunawayActor::kMuseumInterior : RunawayActor::kMuseumHall))
		return false;
	_position = position;
	_walkPath.reset(position);
	_facing = facing;
	_idleHead.reset(_now);
	_idleBody.reset(_now);
	_pendingHotspot = _pendingItem = 0;
	_pendingAction = 0;
	_lines.clear();
	_lineIndex = 0;
	_speech.reset();
	_caption.clear();
	for (uint i = 0; i < ARRAYSIZE(_foreground); ++i)
		_foreground[i].clear();
	if (!loadCustomResources() || !rebuildBackground()) {
		return false;
	}
	_keyCount = 0;
	updateMaps();
	updateCamera();
	return _actor.setFrame(_facing, 0);
}

bool RunawayRoomScene::loadWalkMap() {
	const RunawayRoom *room = findRunawayRoom(_room);
	return loadRunawayWalkMap(_archive, room->width, room->height, _mask, _originalMaps, _walkRoutes);
}

bool RunawayRoomScene::loadForeground(uint layer, uint chunk, bool pixels) {
	assert(layer < ARRAYSIZE(_foreground));
	const RunawayRoom *room = findRunawayRoom(_room);
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
	const RunawayRoom *room = findRunawayRoom(_room);
	return loadRunawaySurface(_archive, 0, room->width, room->height, _background);
}

void RunawayRoomScene::updateMaps() {
	_maps = _originalMaps;
	applyCustomHotspotState();
}

const RunawayHotspot *RunawayRoomScene::hotspot(byte id) const {
	return _roomData && id && id < _roomData->hotspotCount ? &_roomData->hotspots[id] : nullptr;
}

byte RunawayRoomScene::actionCursor(byte id) const {
	const RunawayHotspot *target = hotspot(id);
	return target ? target->actionCursor : 0;
}

bool RunawayRoomScene::preferLook(byte id) const {
	const RunawayHotspot *target = hotspot(id);
	return target && target->preferLook;
}

bool RunawayRoomScene::isWalkHotspot(byte id) const {
	const RunawayHotspot *target = hotspot(id);
	return target && target->walkOnly;
}

byte RunawayRoomScene::hotspotAt(Common::Point mouse) const {
	if (mouse.x < 0 || mouse.x >= kRunawayScreenWidth || mouse.y < kRunawayViewportTop ||
			mouse.y >= kRunawayViewportTop + kRunawayViewportHeight)
		return 0;
	const int x = mouse.x + _camera.x, y = mouse.y - kRunawayViewportTop + _camera.y;
	if (x < 0 || x >= _background.w || y < 0 || y >= _background.h)
		return 0;
	const byte id = _maps[256 + _mask[y * _background.w + x]];
	return isValidHotspot(id) ? id : 0;
}

Common::U32String RunawayRoomScene::hotspotName(byte id) const {
	if (!hotspot(id)) {
		return Common::U32String();
	}
	return _strings.hotspotName(_room, id);
}

Common::U32String RunawayRoomScene::actionCaption(byte id, RunawayVerb verb, byte item) const {
	const Common::U32String name = hotspotName(id);
	if (_game == kRunawayFullGame || _game == kRunawayMuseumDemo) {
		const bool look = verb == kVerbLook && !isWalkHotspot(id);
		const Common::U32String pattern = _strings.sceneActionCaption(_room, id, item, look, actionCaptionVariant(id, look, item));
		if (!pattern.empty()) {
			return RunawayStrings::formatActionCaption(pattern, _inventory.name(item), name);
		}
	}
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
		_walkPath.findPath(RunawayWalkMap(_background.w, _background.h, _mask, _originalMaps, &_walkRoutes), target);
	} else {
		findWalkPath(target);
	}
	_arrivalFacing = facing;
	Common::Array<float> scales;
	const Common::Point position = _position;
	scales.push_back(actorWalkScale());
	// Sample each waypoint with the room's perspective calculation.
	for (uint i = 0; i < _walkPath.points().size(); ++i) {
		_position = _walkPath.points()[i];
		scales.push_back(actorWalkScale());
	}
	_position = position;
	_walkPath.start(_facing, facing, scales, _game == kRunawayMuseumDemo);
	if (!_walkPath.active())
		arrive();
}

void RunawayRoomScene::findWalkPath(Common::Point target) {
	findGameplayWalkPath(target);
}

void RunawayRoomScene::arrive() {
	_arrivalFacing = 0xff;
	const byte id = _pendingHotspot;
	_pendingHotspot = 0;
	if (id) {
		if (_pendingItem)
			useItem(_pendingItem, id);
		else
			execute(_pendingAction);
	}
	if (!speaking())
		_caption.clear();
}

bool RunawayRoomScene::canSkipExit(byte id) const {
	const RunawayHotspot *spot = hotspot(id);
	if (_game != kRunawayFullGame || !spot || !spot->skipExit)
		return false;
	// These exits acquire the native double-click flag as their puzzles progress.
	switch (_room) {
	case 2020:
		return id != 8 || _hospital.window;
	case 4030:
		return !_safeOpen;
	case 4060:
		return id != 7 || !_inventory.museum().restoredCrucifix;
	case 5020:
		return id != 6 || _desert.hatchSeen;
	case 5080:
		return id == 1 || _desert.trainEntered;
	case 6180:
		return id != 2 || _town.saloonDoorKnown;
	case 6190:
		return id != 2 || _town.basementVisited;
	case 6200:
		return id != 2 || _town.cellOpen;
	case 6260:
		return id != 2 || _town.cabVisited;
	case 6263:
		return _town.cabExited;
	case 8090:
		if (id == 2)
			return _finale.oscarAway && (!_finale.recordingPlan || !_finale.recordings || _finale.recordings == 2);
		return id != 6 || _finale.oscarAway;
	case 8200:
		return id != 2 || !_finale.oscarAway;
	default:
		return true;
	}
}

void RunawayRoomScene::waitForExitClick(uint32 now) {
	_idleHead.pause(now - _lastUpdate);
	_idleBody.pause(now - _lastUpdate);
	_now = _lastUpdate = now;
}

void RunawayRoomScene::skipExit(byte id) {
	if (busy() || !canSkipExit(id))
		return;
	_walkPath.clear();
	_arrivalFacing = 0xff;
	_pendingHotspot = _pendingItem = 0;
	_idleHead.reset(_now);
	_idleBody.reset(_now);
	_actor.setFrame(_facing, 0);
	_caption = actionCaption(id, kVerbExit, 0);
	_skippingExit = true;
	execute(hotspot(id)->action[1]);
	_skippingExit = false;
	if (!speaking())
		_caption.clear();
}

void RunawayRoomScene::click(Common::Point mouse, RunawayVerb verb, byte item) {
	if (busy())
		return;
	_idleBody.interact(_now);
	const byte id = hotspotAt(mouse);
	_caption = actionCaption(id, verb, item);
	const RunawayHotspot *spot = hotspot(id);
	_pendingItem = item;
	if (spot) {
		const uint index = verb == kVerbLook && !item && !spot->walkOnly ? 0 : 1;
		_pendingHotspot = id;
		_pendingAction = spot->action[index];
		int8 approach = spot->approach[index];
		if (item) {
			approach = _game == kRunawayCabinDemo;
			const RunawayRoom *room = findRunawayRoom(_room);
			for (uint i = 0; i < room->itemActionCount; ++i) {
				const RunawayItemAction &action = room->itemActions[i];
				if (action.hotspot == id && (!action.item || action.item == item)) {
					approach = action.approach;
					break;
				}
			}
			const RunawayItemAction *restored = restoredItemAction(item, id);
			if (restored)
				approach = restored->approach;
		}
		approach = customizeSceneActionApproach(id, index != 0, item, approach);
		if (approach > 0) {
			const Common::Point position(spot->x, spot->y);
			const byte facing = approach > 1 ? 0xff : customizeSceneActionFacing(id, spot->facing);
			walkTo(customizeSceneActionPosition(id, item, position), facing);
		} else {
			const byte facing = approach < 0 ? 0xff : runawayFacingTo(_position, sceneActionLookPosition(id));
			if (shouldUpdateActor() && shouldDrawActor()) {
				walkTo(_position, facing, true);
			} else {
				// Close-ups and overviews have no walking actor.
				_walkPath.clear();
				if (facing != 0xff)
					_facing = facing;
				arrive();
			}
		}
	} else {
		if (shouldPlayGameplayClickPath())
			walkTo(Common::Point(mouse.x + _camera.x, mouse.y - kRunawayViewportTop + _camera.y));
	}
}

void RunawayRoomScene::showLine() {
	while (_lineIndex < _lines.size() && _lines[_lineIndex].pickup && !_lines[_lineIndex].animatePickup) {
		_inventory.add(_lines[_lineIndex].pickup, false);
		++_lineIndex;
	}
	if (_lineIndex == _lines.size()) {
		_lines.clear();
		_lineIndex = 0;
		_caption.clear();
		return;
	}
	const SpeechLine &line = _lines[_lineIndex];
	if (line.pickup)
		_inventory.beginPickup(line.pickup, line.pickupEffect);
	else
		_speech.start(line.text, line.sample, line.global, _now, line.color);
	_idleHead.reset(_now);
	_idleBody.reset(_now);
}

void RunawayRoomScene::pickUp(byte item, uint16 cue, byte variation, bool global, byte before, uint16 effect) {
	const uint firstLine = _lines.size();
	if (before != 0xff)
		say(cue, before, global);
	if (!_inventory.contains(item)) {
		SpeechLine line;
		line.inventory = true;
		line.pickup = item;
		line.pickupEffect = effect;
		_lines.push_back(line);
		if (_lineIndex == _lines.size() - 1)
			showLine();
	}
	if (cue != 0xffff && variation != 0xff)
		say(cue, variation, global);
	for (uint i = firstLine; i < _lines.size(); ++i)
		_lines[i].inventory = true;
}

void RunawayRoomScene::addInventoryItem(byte item) {
	SpeechLine line;
	line.pickup = item;
	line.animatePickup = false;
	_lines.push_back(line);
	if (_lineIndex == _lines.size() - 1)
		showLine();
}

void RunawayRoomScene::say(uint16 cue, byte variation, bool global, byte speaker, byte color) {
	SceneSpeechCue speech;
	if (global)
		_text.getStaticSpeechCue(cue, variation, speech.textRecordId, speech.continuationCount, speech.voiceSampleId);
	else
		speech = _text.stageCue(cue, variation);
	const uint firstLine = _lines.size();
	say(_text, speech, global, speaker, color);
	if (_game == kRunawayMuseumDemo && global)
		setMuseumInventorySpeechPoses(cue, variation, firstLine);
	else if (global) {
		for (uint i = firstLine; i < _lines.size(); ++i)
			_lines[i].inventoryPose = RunawayInventory::speechPose(cue, variation);
	}
}

void RunawayRoomScene::say(const SceneTextStore &text, const SceneSpeechCue &speech, bool global, byte speaker, byte color) {
	assert(speaker < ARRAYSIZE(_speechColors));
	stopAmbientSpeech();
	if (!speech.valid())
		return;
	for (uint i = 0; i < MAX<uint>(1, speech.continuationCount); ++i) {
		SpeechLine line;
		line.text = Common::U32String(text.largeTextRecord(speech.textRecordId + i), Common::kDos850);
		line.sample = _game != kRunawayMuseumDemo && speech.voiceSampleId ? speech.voiceSampleId + i : 0;
		line.global = global;
		line.speaker = speaker;
		line.color = color ? color : _speechColors[speaker];
		if (!line.text.empty())
			_lines.push_back(line);
	}
	if (!speaking() && !_inventory.pickingUp() && _lineIndex < _lines.size())
		showLine();
}

void RunawayRoomScene::sayInventory(const RunawayInventory::Reply &reply) {
	_walkPath.clear();
	_pendingHotspot = 0;
	_caption.clear();
	if (reply.action) {
		executeItemAction(reply.action);
		return;
	}
	if (reply.stage) {
		SceneTextStore text;
		if (text.loadStage("RESOURCE.003", "Runaway inventory", reply.stage)) {
			say(text, text.stageCue(reply.cue, reply.variation), false);
		}
		return;
	}
	if (townChapter() && sayTownInventory(reply)) {
		return;
	}
	if (hopiChapter() && sayHopiInventory(reply)) {
		return;
	}
	if (finaleChapter() && sayFinaleInventory(reply)) {
		return;
	}
	if (museumChapter() && sayMuseumInventory(reply))
		return;
	if (_game == kRunawayFullGame && _room / 1000 == 2 && sayHospitalInventory(reply))
		return;
	for (RunawayInventory::Reply line = reply; line.cue; line = _inventory.nextReply(line)) {
		say(line.cue, line.variation, true);
	}
}

void RunawayRoomScene::skipSpeech() {
	if (_sequence.active()) {
		_sequence.skipSpeech();
		return;
	}
	if (!speaking())
		return;
	_speech.stop();
	advanceLine();
}

void RunawayRoomScene::advanceLine() {
	if (++_lineIndex < _lines.size()) {
		showLine();
	} else {
		_lines.clear();
		_lineIndex = 0;
		_caption.clear();
	}
}

const RunawayItemAction *RunawayRoomScene::restoredItemAction(byte item, byte id) const {
	if (_game != kRunawayFullGame || !_restoredContent)
		return nullptr;
	for (uint i = 0; i < ARRAYSIZE(kRestoredItemActions); ++i) {
		const RunawayItemAction &action = kRestoredItemActions[i];
		if (action.room == _room && action.item == item && action.hotspot == id)
			return &action;
	}
	return nullptr;
}

void RunawayRoomScene::useItem(byte item, byte id) {
	const RunawayItemAction *restored = restoredItemAction(item, id);
	if (restored) {
		executeItemAction(restored->action);
		return;
	}
	const RunawayRoom *room = findRunawayRoom(_room);
	for (uint i = 0; i < room->itemActionCount; ++i) {
		const RunawayItemAction &action = room->itemActions[i];
		if (action.hotspot == id && (!action.item || action.item == item)) {
			executeItemAction(action.action);
			return;
		}
	}
	if (_game == kRunawayFullGame) {
		executeItemAction(defaultItemAction(item, id));
	} else if (_game == kRunawayMuseumDemo) {
		executeItemAction(1);
	} else {
		say(1, 0, true);
	}
}

void RunawayRoomScene::executeItemAction(uint16 action) {
	if ((_game == kRunawayFullGame && (action == 1 || action == 5)) ||
			(_game == kRunawayMuseumDemo && action == 1)) {
		const RunawayInventory::Reply reply = RunawayInventory::randomReply(_random, _game == kRunawayMuseumDemo);
		say(reply.cue, reply.variation, true);
		if (reply.nextVariation != 0xff)
			say(reply.cue, reply.nextVariation, true);
		return;
	}
	if (_game == kRunawayMuseumDemo && !action)
		return;
	if (_game == kRunawayFullGame) {
		switch (action) {
		case 2: // A variation of "That won't work."
			say(1, _random.getRandomNumber(2), true);
			return;
		case 7: // "No can do."
			say(6, _random.getRandomBit(), true);
			return;
		case 12: // Keep the item (masculine).
			say(11, _random.getRandomNumber(2), true);
			return;
		case 24: // Think before rejecting the combination.
			say(20, 0, true);
			say(20, 1 + _random.getRandomNumber(2), true);
			return;
		case 25: { // Keep the item (feminine).
			const byte variation = _random.getRandomNumber(2);
			say(11, variation ? variation + 2 : 0, true);
			return;
		}
		case 26: // Refuse to put the item back (masculine).
		case 27: // Refuse to put the item back (feminine).
			say(21, (action == 27 ? 2 : 0) + _random.getRandomBit(), true);
			return;
		case 29: // Refuse to burn the object.
			say(_random.getRandomNumber(4) ? 23 : 14, 0, true);
			return;
		case 32: // "I don't get it."
			say(26, 0, true);
			return;
		default:
			break;
		}
	}
	execute(action);
}

void RunawayRoomScene::beginHeadMovement(uint16 nextAction, RunawayHeadMovement::Direction direction, uint16 hold) {
	if (_game != kRunawayFullGame) {
		if (nextAction)
			execute(nextAction, true);
		return;
	}
	_idleHead.reset(_now);
	_idleBody.reset(_now);
	_headMovement.start(direction, hold, _now);
	_afterHeadMovement = nextAction;
	_actor.setFrame(_facing, 0, _headMovement.frame());
}

void RunawayRoomScene::returnHeadToCenter(uint16 nextAction) {
	_headMovement.returnToCenter(_now);
	_afterHeadMovement = nextAction;
}

bool RunawayRoomScene::beginActionHeadMovement(uint16 action) {
	if (_game != kRunawayFullGame)
		return false;
	// Descriptions that call the retail 0x436d40 helper unconditionally.
	static const struct {
		uint16 room, action;
	} down[] = {
		{2020, 304}, {2020, 306}, {2020, 315}, {2020, 317}, // Glass, pills, charts.
		{2060, 306}, // Syringes.
		{4030, 302}, {4030, 306}, {4030, 308}, {4030, 309}, // Books and safe contents.
		{4050, 312}, {4050, 322}, {4050, 337}, // Wastebasket, briefcase, brush.
		{4060, 328}, {4100, 313}, // Plaque and strange key.
		{5060, 309}, {5080, 303}, // Trunk and wooden box.
		{5100, 315}, {5100, 323}, {5110, 303}, {5110, 311}, // Barrels, bucket, compartment, screw.
		{5110, 419}, // Peanuts behind the compartment door.
		{5160, 309}, {5160, 313}, {5170, 313}, // Grating, needle and thread, under the bed.
		{6180, 413}, {6260, 309}, {6270, 401}, // Buried wrench, locomotive key, crater.
		{7010, 309}, {7010, 318}, {7010, 320}, // Cliff edge, beam, mining tool.
		{8100, 427}, // Opening to the séance room.
		{8260, 309}, {8370, 321}, {8400, 308}, {8410, 305} // Badge, register, papers, trunk.
	};
	for (uint i = 0; i < ARRAYSIZE(down); ++i) {
		if (_room == down[i].room && action == down[i].action) {
			beginHeadMovement(action, RunawayHeadMovement::kDown);
			return true;
		}
	}
	RunawayHeadMovement::Direction direction = RunawayHeadMovement::kDown;
	uint16 hold = 2;
	bool move = false;
	switch (_room) {
	case 4030:
		move = action == 304;
		hold = RunawayHeadMovement::kHold;
		break;
	case 4060:
		move = action == 313 && _museum.basketSeen && !_coffeeTaken;
		break;
	case 5050:
		move = action == 401;
		direction = RunawayHeadMovement::kRight;
		break;
	case 5070:
		move = (action == 315 || action == 316) && !_desert.oilSeen;
		break;
	case 5100:
		move = action == 500 || action == 578;
		direction = RunawayHeadMovement::kRight;
		break;
	case 5110:
		move = action == 312 && !_desert.screwTaken && !_desert.screwSeen;
		break;
	case 5160:
		move = action == 314 && !_desert.needle;
		break;
	case 5170:
		move = action == 314 && !_desert.vacuum;
		break;
	case 6130:
		move = (action == 307 || action == 308) && !_town.stone;
		break;
	case 6210:
	case 8210:
		move = ((action == 307 || action == 408) && !_town.bucketSeen) || (action == 313 && !_town.bottleSeen);
		break;
	case 6260:
		move = action == 310 && _town.locomotiveKey == 1;
		break;
	case 6280:
		move = action == 301;
		direction = RunawayHeadMovement::kUp;
		break;
	case 6300:
	case 8300:
		move = (action == 307 && !_town.craneKnown) ||
			(_room == 6300 && (action == 309 || action == 310) && !_town.statueLowered);
		direction = RunawayHeadMovement::kUp;
		break;
	case 7010:
		move = action == 321 && _hopi.timber == 1;
		break;
	case 7050:
		if (action == 301 || action == 401) {
			move = true;
			direction = RunawayHeadMovement::kUp;
			hold = RunawayHeadMovement::kHold;
		} else {
			move = (action == 303 || action == 304) && _hopi.tomahawk == kUnexamined;
		}
		break;
	case 8260:
		move = action == 310 && _finale.locomotiveBadge == 1;
		break;
	default:
		break;
	}
	if (move)
		beginHeadMovement(action, direction, hold);
	return move;
}

void RunawayRoomScene::execute(uint16 action, bool headMoved) {
	if (!headMoved && beginActionHeadMovement(action))
		return;
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

	const RunawayRoom *room = findRunawayRoom(_room);
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

void RunawayRoomScene::changeFloor(bool upper, Common::Point position) {
	_upperFloor = upper;
	_position = position;
	_walkPath.reset(position);
	_facing = 3;
	updateMaps();
	updateCamera();
}

void RunawayRoomScene::setCameraX(int x) {
	_camera.x = CLIP<int>(x, 0, MAX<int>(0, _background.w - kRunawayScreenWidth));
}

void RunawayRoomScene::updateCamera() {
	_cameraFollow.reset();
	setCameraX(_position.x - kRunawayScreenWidth / 2);
	_camera.y = sceneViewportYOffset();
}

void RunawayRoomScene::followCamera(Common::Point camera) {
	setCameraX(camera.x);
	setCameraY(camera.y);
}

int RunawayRoomScene::sceneViewportYOffset() const {
	return CLIP<int>(_position.y - (int)actorHeight() / 2 - kRunawayViewportHeight / 2,
		0, _background.h - kRunawayViewportHeight);
}

void RunawayRoomScene::update(uint32 now, int scrollDirection, bool inventoryOpen) {
	_now = now;
	if (inventoryOpen || _inventoryOpen) {
		_idleHead.pause(now - _lastUpdate);
		_idleBody.pause(now - _lastUpdate);
		_headMovement.pause(now - _lastUpdate);
	}
	_inventoryOpen = inventoryOpen;
	const uint elapsed = MIN<uint>(100, now - _lastUpdate);
	_lastUpdate = now;
	if (_sequence.active()) {
		_idleHead.reset(now);
		_idleBody.reset(now);
		_sequence.update(now);
		if (!_sequence.active()) {
			if (_inventorySequence) {
				_inventorySequence = false;
				resumeAmbientAudio(false);
			} else {
				finishCustomFilm();
			}
		}
		return;
	}
	if (speaking() && _speech.finished(now))
		skipSpeech();
	if (_lineIndex < _lines.size() && _lines[_lineIndex].pickup && !_inventory.pickingUp())
		advanceLine();
	updateAmbientMusic();
	if (presentingPickup()) {
		_actionStarted = now;
		return;
	}
	if (_inventory.handling()) {
		const RunawayInventory::Reply line = _inventory.updateHandling(now, _audio, speaking());
		if (line.closeup || line.stage) {
			sayInventory(line);
		} else if (line.cue) {
			say(line.cue, line.variation, true);
			if (line.cue == 68 && line.variation == 3)
				_hospital.medicineRead = 1;
		}
		return;
	}
	_headMovement.update(now, _random);
	if (_headMovement.blocking()) {
		advanceCustomGameplayLoop(elapsed);
		updateActor(elapsed, scrollDirection, inventoryOpen);
		return;
	}
	if (_afterHeadMovement) {
		const uint16 action = _afterHeadMovement;
		_afterHeadMovement = 0;
		execute(action, true);
	}
	if (townChapter() || hopiChapter() || finaleChapter()) {
		updateGameplay(elapsed);
		if (_sequence.active() || _transition.scene)
			return;
	} else if (desertChapter()) {
		updateDesert(elapsed);
		if (_sequence.active() || _transition.scene)
			return;
	} else if (museumChapter()) {
		updateMuseum(elapsed);
		if (_sequence.active() || _transition.scene)
			return;
	} else if (_game == kRunawayFullGame) {
		updateHospital(elapsed);
		if (_sequence.active() || _transition.scene) {
			return;
		}
	}
	updateActor(elapsed, scrollDirection, inventoryOpen);
}

void RunawayRoomScene::updateGameplay(uint32 delta) {
	advanceCustomGameplayLoop(delta);
	if (updateCustomSequence()) {
		return;
	}
	updateRoomAnimation();
	if (_sequence.active() || _transition.scene || presentingPickup()) {
		return;
	}
	updateConversation();
	if (_roomAction && _actionFrames.empty() && !speaking() && !_walkPath.active()) {
		const uint16 action = _roomAction;
		_roomAction = 0;
		execute(action);
	}
	if (_hospitalExit.scene && !speaking() && !presentingPickup() && !_roomAction && !_walkPath.active()) {
		_transition = _hospitalExit;
	}
}

void RunawayRoomScene::findGameplayWalkPath(Common::Point target) {
	if (!_pendingHotspot) {
		// Clicks stay on the ground; hotspot approaches may use the reserved zones.
		target.x = CLIP<int>(target.x, 0, _background.w - 1);
		target.y = CLIP<int>(target.y, 0, _background.h - 1);
		if (!adjustCustomWalkTargetToFloorMask(target) &&
				!RunawayWalkMap(_background.w, _background.h, _mask, _maps).projectToFloor(target)) {
			_walkPath.clear();
			return;
		}
	}
	_walkPath.findPath(RunawayWalkMap(_background.w, _background.h, _mask, _originalMaps,
		_walkRoutes.empty() ? nullptr : &_walkRoutes), target);
}

void RunawayRoomScene::updateOutdoorMusic(byte cueCount) {
	_audio.updateBackgroundMusic(_now, _random, 12, cueCount);
}

void RunawayRoomScene::deferAction(uint16 action) {
	_roomAction = action;
	_actionFrames.clear();
}

void RunawayRoomScene::leaveRoom(uint16 scene) {
	const RunawayRoom *room = findRunawayRoom(scene);
	if (!room) {
		warning("Runaway: scene %u is not implemented yet", scene);
		return;
	}
	_hospitalExit.scene = scene;
	_hospitalExit.position = room->entry;
	_hospitalExit.facing = 3;
}

void RunawayRoomScene::updateActor(uint elapsed, int scrollDirection, bool inventoryOpen) {
	if (inventoryOpen || presentingPickup())
		return;
	if (!shouldUpdateActor()) {
		_idleHead.reset(_now);
		_idleBody.reset(_now);
		return;
	}
	if (_idleBody.active() && _walkPath.active() && !speaking() && !_headMovement.active()) {
		_idleBody.interact(_now);
		_idleHead.reset(_now);
		if (_idleBody.update(_now, true, false, _facing, _actor, _random))
			return;
	}
	uint cameraElapsed = elapsed;
	if (_walkPath.active() && !speaking() && !_headMovement.active()) {
		_cameraFollow.reset();
		uint footsteps = 0;
		while (cameraElapsed && _walkPath.active()) {
			const uint step = MIN(cameraElapsed, _walkPath.timeToNextFrame());
			const Common::Point previous = _position;
			footsteps += _walkPath.advance(step, _position, _facing);
			cameraElapsed -= step;
			if (!scrollDirection && _position != previous && shouldFollowCamera())
				followCamera(RunawayCamera::followWalk(_camera, previous, _position, (int)actorHeight(), _fastScrolling));
		}
		if (_game != kRunawayMuseumDemo && footsteps) {
			const bool inside = _position.x >= 0 && _position.x < _background.w && _position.y >= 0 && _position.y < _background.h;
			const byte surface = inside ? _maps[1280 + _mask[_position.y * _background.w + _position.x]] : 0;
			const byte volume = CLIP<int>((int)(actorHeight() * 100 / 455), 0, 100);
			if (surface >= 1 && surface <= 7) {
				for (uint i = 0; i < footsteps; ++i)
					_audio.playFootstep(surface, _random.getRandomBit(), volume, _position.x);
			}
		}
		if (!_walkPath.active()) {
			arrive();
		}
	}
	// Dialogue can pause a queued walk, including one scheduled on arrival.
	const bool walking = _walkPath.active() && !speaking() && !_headMovement.active();
	const bool idle = !walking && !busy() && shouldDrawActor();
	_idleHead.update(_now, idle && !_idleBody.active(), _random, _game == kRunawayMuseumDemo);
	const bool bodyIdle = _idleBody.update(_now, idle && _game != kRunawayMuseumDemo,
		_idleHead.centered(), _facing, _actor, _random);
	const int headFrame = _headMovement.active() ? _headMovement.frame() : walking ? -1 :
		speaking() && !speaker() ? speechFrame() : idle ? _idleHead.frame() : -1;
	if (!_headMovement.active() && _actionUsesActor && _roomAction && !_actionFrames.empty() && !walking && !speaking()) {
		_actor.setActionFrame(_actionFrames[_actionStep]);
	} else if (!bodyIdle) {
		_actor.setFrame(_facing, walking ? _walkPath.frame() : 0, headFrame);
	}
	if (scrollDirection) {
		_cameraFollow.reset();
		setCameraX(_camera.x + scrollDirection * MAX<int>(1, elapsed / (_fastScrolling ? 2 : 4)));
	} else if (!_fastScrolling && !walking && shouldFollowCamera()) {
		followCamera(_cameraFollow.followIdle(_camera, _position, (int)actorHeight(), cameraElapsed));
	} else {
		_cameraFollow.reset();
	}
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
	if (drawCustomComposite(viewport)) {
		return;
	}
	drawCustomBackgroundComposite(viewport);
	RunawayActor::RenderParams params;
	// Earlier rooms use six depth layers; Douglasville also uses layers 7..9.
	for (uint i = 0; i < 7; ++i)
		params.depthY[i] = 0;
	setupActorDepth(params);
	params.position = _position;
	params.camera = _camera;
	params.roomWidth = _background.w;
	params.height = actorHeight();
	params.antialias = _antialias;
	const byte light = MIN<byte>(9, findRunawayLight(_position, _background.w, _mask, _maps));
	memcpy(params.light, findRunawayRoom(_room)->light[light], sizeof(params.light));
	const bool roomAnimation = drawingRoomAnimation();
	const bool foregroundAnimation = roomAnimation && roomAnimationInForeground();
	// NPC animations normally remain behind Brian, just like their idle poses.
	if (roomAnimation && !foregroundAnimation) {
		drawRoomAnimation(viewport);
	}
	if ((!roomAnimation || !_actionHidesActor) && shouldDrawActor()) {
		_actor.draw(_screen, params, _mask, _maps);
	}
	if (foregroundAnimation) {
		drawRoomAnimation(viewport);
	}
	drawCustomForegroundComposite(viewport);
}

void RunawayRoomScene::drawRoomAnimation(Graphics::ManagedSurface &viewport) {
	_actionAnimation.draw(viewport, _actionFrames[MIN<uint>(_actionStep, _actionFrames.size() - 1)], _camera.x, -_camera.y);
	if (!_secondaryFrames.empty()) {
		_secondaryAnimation.draw(viewport, _secondaryFrames[MIN<uint>(_actionStep, _secondaryFrames.size() - 1)], _camera.x, -_camera.y);
	}
}

void RunawayRoomScene::drawAmbientAnimations(Graphics::ManagedSurface &viewport, const RunawayAnimation *animations, uint count) {
	for (uint i = 0; i < count; ++i) {
		const RunawayAnimation &animation = animations[i];
		if (animation.frameCount()) {
			animation.draw(viewport, (_now / 80) % animation.frameCount(), _camera.x, -_camera.y);
		}
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
	if (!_actionAnimation.load(_archive, chunk, frames))
		error("Cannot load Runaway room %u animation %u", _room, chunk);
	beginAnimation(action, frames, order, count, hideActor, frameDuration);
}

void RunawayRoomScene::beginActorAnimation(uint16 action, uint bank, uint frames, uint count, uint frameDuration) {
	assert(count <= frames);
	if (!_actor.loadAction(bank, frames)) {
		error("Cannot load Runaway actor animation %u", bank);
	}
	beginAnimation(action, count ? count : frames, nullptr, 0, false, frameDuration);
	_actionUsesActor = true;
	_actor.setActionFrame(0);
}

void RunawayRoomScene::beginAnimation(uint16 action, uint frames, const byte *order, uint count, bool hideActor, uint frameDuration) {
	assert(frameDuration);
	_actionFrames.clear();
	_secondaryFrames.clear();
	_actionUsesActor = false;
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

void RunawayRoomScene::playAnimationFootsteps(uint frame, byte surface, const AnimationFootstep *steps, uint count) {
	for (uint i = 0; i < count; ++i) {
		if (frame == steps[i].frame) {
			_audio.playFootstep(surface, _random.getRandomBit(), steps[i].volume, steps[i].x);
		}
	}
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
			if (townChapter() || desertChapter() || hopiChapter() || finaleChapter()) {
				execute(action + 1000);
			} else {
				finishCustomAnimation(action);
			}
			return;
		}
		handleAnimationFrameHook();
	}
}

} // End of namespace Hollywood
