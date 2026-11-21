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

#include "common/debug.h"
#include "common/endian.h"
#include "common/str-enc.h"
#include "common/util.h"

#include "hollywood/debug.h"
#include "hollywood/runaway/navigation.h"
#include "hollywood/runaway/scene5030.h"
#include "hollywood/runaway/saveload.h"
#include "hollywood/runaway/strings.h"

namespace Hollywood {
namespace Runaway {
namespace {

struct SceneHotspot {
	int x, y;
	byte facing;
	bool approach[2];
	Common::Point lookPosition;
};

// Approach points and facings from demo initializer 0x439e40, with approach
// flags and look targets from retail 0x479ea0. The hit regions are decoded from E03.
const SceneHotspot kHotspots[] = {
	{0, 0, 0, {false, false}, {0, 0}},
	{101, 507, 7, {true, false}, {42, 451}},
	{101, 507, 7, {true, true}, {42, 451}},
	{406, 383, 0, {false, false}, {406, 360}},
	{1062, 473, 1, {false, false}, {1212, 459}},
	{942, 407, 0, {true, true}, {942, 385}},
	{1215, 542, 7, {true, true}, {1169, 479}},
	{406, 383, 7, {true, false}, {371, 336}},
	{1062, 473, 7, {false, false}, {1100, 450}},
	{1019, 405, 0, {true, true}, {1019, 375}},
	{1215, 542, 7, {true, true}, {1271, 477}},
	{1215, 542, 7, {true, true}, {1100, 427}},
	{495, 381, 7, {false, false}, {545, 362}},
	{873, 409, 1, {false, false}, {844, 385}},
	{974, 406, 7, {false, true}, {974, 380}},
	{262, 426, 0, {true, false}, {253, 399}},
	{262, 426, 7, {false, true}, {208, 400}},
	{609, 379, 1, {true, true}, {652, 360}},
	{1062, 473, 1, {false, true}, {1100, 450}},
	{1215, 542, 7, {false, false}, {0, 0}}
};

} // End of anonymous namespace

bool Scene5030::load(uint16 entryScene) {
	if (entryScene != 5030 && !(_game == kRunawayFullGame && entryScene >= 5031 && entryScene <= 5032))
		return false;
	_entryScene = entryScene;
	const Graphics::PixelFormat format(2, 5, 6, 5, 0, 11, 5, 0, 0);
	if (!_archive.open(Common::Path("RESOURCE.E03"), 50) ||
			!loadRunawayMask(_archive, kRoomWidth, kRoomHeight, _mask, _maps) ||
			!_text.load("RESOURCE.003", "Runaway shack", 503, 0, 0x1135))
		return false;
	_background.create(kRoomWidth, kRoomHeight, format);
	_screen.create(kScreenWidth, kScreenHeight, format);
	if (!_actor.load() ||
			!_intro.load(_archive, 47, 20) || !_climb.load(_archive, 42, 18) ||
			!_hatch.load(_archive, 14, 2) || !loadRoomGraphics() ||
			!_video.load("DATAVE03.001"))
		return false;
	const uint rows = _text._stageLargeRows.size() / 321;
	_voiceSamples.resize(rows, 0);
	_cueEnds.resize(rows, 0);
	// The shared five-byte cue descriptors map each line (including all
	// continuations) to its actual global voice ID in DATAACA3.
	for (uint cueOffset = 0; cueOffset + 5 <= _text._stageBlock.size(); cueOffset += 5) {
		const byte *cue = _text._stageBlock.data() + cueOffset;
		const uint row = READ_LE_UINT16(cue), count = MAX<uint>(1, cue[2]);
		const uint sample = READ_LE_UINT16(cue + 3);
		if (row < 500 || row + count > 500 + rows || !sample || sample + count > 6000)
			continue;
		for (uint i = 0; i < count; ++i) {
			_voiceSamples[row - 500 + i] = sample + i;
			_cueEnds[row - 500 + i] = row + count - 1;
		}
	}
	debugC(1, kDebugScene, "Runaway demo: scene 5030, 152 walking cels, room animations and hotspot mask loaded");
	return true;
}

bool Scene5030::loadRoomGraphics() {
	Common::Array<byte> pixels, overlay, patch;
	if (!_archive.readVariableChunk(_windowClean ? 4 : 0, pixels) ||
			pixels.size() != kRoomWidth * kRoomHeight * 2 ||
			!_lidActor.load(_archive, _windowClean ? 45 : 43, 33) ||
			!_lid.load(_archive, _windowClean ? 46 : 44, 5) ||
			!_unplug.load(_archive, _windowClean ? 39 : 40, 59) ||
			!loadPatch(_windowClean ? 6 : 5, _unplugPatch) ||
			!loadPatch(_windowClean ? 8 : 7, _openLidPatch) ||
			!loadPatch(_windowClean ? 10 : 9, _closedLidPatch) ||
			!loadPatch(_windowClean ? 27 : 26, overlay))
		return false;
	for (int y = 0; y < kRoomHeight; ++y) {
		uint16 *out = (uint16 *)_background.getBasePtr(0, y);
		for (int x = 0; x < kRoomWidth; ++x)
			out[x] = READ_LE_UINT16(pixels.data() + (y * kRoomWidth + x) * 2);
	}
	// 0x438ce0 rebuilds the room after cleaning. The sunny base already
	// omits the cloth and cleaner; other collected objects still need patches.
	const uint pickupChunks[2][5] = {{20, 18, 15, 19, 17}, {21, 0, 16, 19, 0}};
	const byte hotspots[] = {5, 9, 14, 16, 17};
	for (uint i = 0; i < ARRAYSIZE(hotspots); ++i) {
		_pickupPatches[i].clear();
		uint chunk = pickupChunks[_windowClean][i];
		if (chunk && !loadPatch(chunk, _pickupPatches[i]))
			return false;
		if (_pickedUp[hotspots[i]])
			drawPatch(_pickupPatches[i]);
	}
	if (_coolerMoved) {
		if (!loadPatch(_hatchState == kLocked ? 12 : 13, patch))
			return false;
		drawPatch(patch);
		if (_hatchState == kOpen) {
			if (!loadPatch(24, patch))
				return false;
			drawPatch(patch);
		}
	} else {
		drawPatch(_lidOpen ? _openLidPatch : _closedLidPatch);
		if (_unplugged)
			drawPatch(_unplugPatch);
		if (_ice == kDrained) {
			if (!loadPatch(11, patch))
				return false;
			drawPatch(patch);
		}
	}
	drawPatch(overlay);
	return true;
}

void Scene5030::start(uint32 now) {
	_now = _lastUpdate = now;
	_entering = true;
	resume();
	if (_entryScene == 5030)
		startAction(kIntroVideo);
}

void Scene5030::resume() {
	if (!_audio.musicPlaying())
		_audio.playMusic(11);
	if (_entering) {
		_entering = false;
		if (_entryScene == 5031) {
			setPosition(Common::Point(1215, 542), 3);
			setCameraX(_position.x - kScreenWidth / 2);
			startAction(kEnterHatch);
		}
	}
}

void Scene5030::captureState(RunawaySaveState &state) const {
	state.scene = _entryScene;
	state.x = _position.x;
	state.y = _position.y;
	state.camera = _cameraX;
	state.facing = _facing;
	state.lidOpen = _lidOpen;
	state.unplugged = _unplugged;
	state.windowClean = _windowClean;
	state.coolerMoved = _coolerMoved;
	state.coolerInspected = _coolerInspected;
	state.peeked = _peeked;
	state.ice = _ice;
	state.hatch = _hatchState;
	for (uint i = 0; i < ARRAYSIZE(_pickedUp); ++i) {
		state.pickedUp[i] = _pickedUp[i];
		state.examined[i] = _examined[i];
	}
	state.randomSeed = _random.getSeed();
}

bool Scene5030::restoreState(const RunawaySaveState &state, uint32 now, bool entering) {
	if (!state.valid() || state.game != _game || state.scene != _entryScene)
		return false;
	_lidOpen = state.lidOpen;
	_unplugged = state.unplugged;
	_windowClean = state.windowClean;
	_coolerMoved = state.coolerMoved;
	_coolerInspected = state.coolerInspected;
	_peeked = state.peeked;
	_ice = (IceState)state.ice;
	_hatchState = (HatchState)state.hatch;
	for (uint i = 0; i < ARRAYSIZE(_pickedUp); ++i) {
		_pickedUp[i] = state.pickedUp[i];
		_examined[i] = state.examined[i];
	}
	if (!loadRoomGraphics())
		return false;
	_interrogation.stop();
	_video.close();
	_random.setSeed(state.randomSeed);
	_now = _lastUpdate = _actionStarted = now;
	_entering = entering;
	_walkElapsed = _lastFootstep = _step = 0;
	_action = _afterSpeech = _afterWalk = kNone;
	_pendingHotspot = _pendingItem = _pickupHotspot = 0;
	_pendingVerb = kVerbWalk;
	_walkPath.clear();
	_committedCaption.clear();
	_escaped = false;
	setPosition(Common::Point(state.x, state.y), state.facing);
	setCameraX(entering ? state.x - kScreenWidth / 2 : state.camera);
	_idleHead.reset(now);
	return _actor.setFrame(_facing, 0, 0);
}

void Scene5030::setCameraX(int x) {
	_cameraX = CLIP<int>(x, 0, kRoomWidth - kScreenWidth);
}

bool Scene5030::loadPatch(uint chunk, Common::Array<byte> &patch) {
	if (!_archive.readVariableChunk(chunk, patch) || patch.size() < 2)
		return false;
	uint cursor = 2;
	for (uint i = 0; i < READ_LE_UINT16(patch.data()); ++i) {
		if (patch.size() - cursor < 6)
			return false;
		const byte *run = patch.data() + cursor;
		const uint x = READ_LE_UINT16(run), y = READ_LE_UINT16(run + 2), length = READ_LE_UINT16(run + 4);
		cursor += 6;
		if (!length || x + length > kRoomWidth || y >= kRoomHeight || length * 2 > patch.size() - cursor)
			return false;
		cursor += length * 2;
	}
	return cursor == patch.size();
}

void Scene5030::drawPatch(const Common::Array<byte> &patch) {
	if (patch.empty())
		return;
	const byte *run = patch.data() + 2;
	for (uint i = 0; i < READ_LE_UINT16(patch.data()); ++i) {
		const uint x = READ_LE_UINT16(run), y = READ_LE_UINT16(run + 2), length = READ_LE_UINT16(run + 4);
		uint16 *out = (uint16 *)_background.getBasePtr(x, y);
		run += 6;
		for (uint pixel = 0; pixel < length; ++pixel)
			*out++ = READ_LE_UINT16(run + pixel * 2);
		run += length * 2;
	}
}

void Scene5030::applyPatch(uint chunk) {
	Common::Array<byte> patch;
	if (!loadPatch(chunk, patch))
		error("Cannot load Runaway room patch %u", chunk);
	drawPatch(patch);
}

byte Scene5030::hotspotAt(Common::Point mouse) const {
	if (mouse.x < 0 || mouse.x >= kScreenWidth || mouse.y < kRoomTop || mouse.y >= kRoomTop + kRoomHeight)
		return 0;
	byte id = _maps[256 + _mask[(mouse.y - kRoomTop) * kRoomWidth + mouse.x + _cameraX]];
	// Resolve the original mask through the current puzzle state. In particular,
	// the drain and hatch overlap, and the moved cooler covers its old lid target.
	if (id == 2 && !_examined[1])
		id = 1;
	if (_coolerMoved) {
		if (id == 20 || id == 21 || id == 18)
			id = 8;
		else if (id == 8 || id == 19)
			id = 0;
		else if (id == 11)
			id = 6;
	} else {
		if (id == 6 || id == 20)
			id = 8;
		else if (id == 21)
			id = 0;
		else if (id == 19)
			id = 11;
	}
	if (id == 10 && _unplugged)
		id = 0;
	return id < ARRAYSIZE(kHotspots) && !_pickedUp[id] ? id : 0;
}

void Scene5030::walkTo(Common::Point target) {
	_walkPath.findPath(RunawayWalkMap(kRoomWidth, kRoomHeight, _mask, _maps), target);
	_walkElapsed = _lastFootstep = 0;
	if (!_walkPath.active())
		arrive();
}

void Scene5030::say(uint16 row) {
	stopSpeech();
	if (row < 500 || row - 500U >= _voiceSamples.size())
		return;
	_globalSpeech = false;
	_speechRow = row;
	_speechEnd = MAX<uint16>(row, _cueEnds[row - 500]);
	showSpeechRow();
}

void Scene5030::showSpeechRow() {
	const uint16 sample = _globalSpeech ? _speechSample : _voiceSamples[_speechRow - 500];
	_speech.start(Common::U32String(_text.largeTextRecord(_speechRow), Common::kDos850), sample, _globalSpeech, _now);
	_idleHead.reset(_now);
	debugC(1, kDebugScene, "Runaway demo: dialogue row %u, voice %u, ends at row %u",
		_speechRow, sample, _speechEnd);
}

void Scene5030::stopSpeech() {
	_speech.stop();
	_speechRow = _speechEnd = 0;
	_nextInventoryReply = RunawayInventory::Reply(0);
}

Common::U32String Scene5030::hotspotName(byte id) const {
	Common::U32String name = _strings.shackHotspotName(id);
	if (id < ARRAYSIZE(kHotspots) && _examined[id]) {
		if (id == 5 || id == 9 || id == 17) {
			name = _inventory.name(id == 5 ? 8 : id == 9 ? 1 : 3);
		} else if (id == 7) {
			name = _strings.text(RunawayStrings::kShirt);
		}
	}
	return name;
}

Common::U32String Scene5030::actionCaption(byte id, RunawayVerb verb, byte item) const {
	const Common::U32String name = hotspotName(id);
	if (_game == kRunawayFullGame && id) {
		const char *variant = nullptr;
		if (item == 2 && id == 6 && _hatchState >= kUnlocked) {
			variant = "unlocked";
		} else if (!item && verb != kVerbLook) {
			if ((id == 6 && _hatchState == kOpen) || (id == 18 && _lidOpen)) {
				variant = "open";
			} else if (id == 11 && _ice == kDrained) {
				variant = "closed";
			}
		}
		const Common::U32String pattern = _strings.sceneActionCaption(5030, id, item, verb == kVerbLook, variant);
		if (!pattern.empty()) {
			return RunawayStrings::formatActionCaption(pattern, _inventory.name(item), name);
		}
	}
	if (item) {
		Common::U32String caption = _strings.text(RunawayStrings::kUse) + _inventory.name(item);
		if (id)
			caption += _strings.text(RunawayStrings::kWith) + name;
		return caption;
	}
	if (!id)
		return Common::U32String();
	if (verb == kVerbLook)
		return _strings.text(id == 2 || id == 3 || id == 4 ? RunawayStrings::kLookThrough : RunawayStrings::kLook) + name;
	if (id == 6 && _hatchState == kOpen)
		return _strings.text(RunawayStrings::kLeaveHatch);
	RunawayStrings::TextId action = RunawayStrings::kUse;
	if (id == 5 || id == 9 || id == 14 || id == 16 || id == 17)
		action = RunawayStrings::kTake;
	else if (id == 18)
		action = _lidOpen ? RunawayStrings::kClose : RunawayStrings::kOpen;
	else if (id == 8)
		action = RunawayStrings::kMove;
	else if (id == 11)
		action = _ice == kDrained ? RunawayStrings::kClose : RunawayStrings::kOpen;
	else if (id == 10)
		return _strings.text(RunawayStrings::kUnplugCooler);
	else if (id == 12)
		action = RunawayStrings::kUsePhone;
	else if (id == 13)
		action = RunawayStrings::kTakeFromShelf;
	else if (id == 1 || id == 3 || id == 4 || id == 6)
		action = RunawayStrings::kOpen;
	return _strings.text(action) + name;
}

void Scene5030::sayInventory(const RunawayInventory::Reply &reply) {
	const RunawayInventory::Reply speech = reply.action == 1 || reply.action == 5 ? RunawayInventory::randomReply(_random) : reply;
	stopSpeech();
	uint16 row, sample;
	byte count;
	if (!_text.getStaticSpeechCue(speech.cue, speech.variation, row, count, sample))
		return;
	if (!row || row + MAX<uint>(1, count) > _text._inventoryOwnerLargeRows.size() / 321 ||
			sample + MAX<uint>(1, count) > 6000)
		return;
	_globalSpeech = true;
	_speechRow = row;
	_speechEnd = row + MAX<uint>(1, count) - 1;
	_speechSample = sample;
	if (speech.nextVariation != 0xff) {
		_nextInventoryReply = RunawayInventory::Reply(speech.cue, speech.nextVariation);
	}
	showSpeechRow();
}

void Scene5030::sayCue(uint16 cue, byte variation) {
	const SceneSpeechCue speech = _text.stageCue(cue, variation);
	if (speech.valid())
		say(speech.textRecordId);
}

void Scene5030::skipSpeech() {
	if (_action == kPeekConversation) {
		_interrogation.skipSpeech();
		return;
	}
	if (!_speechRow)
		return;
	_speech.stop();
	if (_speechRow < _speechEnd) {
		++_speechRow;
		if (_globalSpeech)
			++_speechSample;
		showSpeechRow();
	} else if (_nextInventoryReply.cue) {
		const RunawayInventory::Reply next = _nextInventoryReply;
		sayInventory(next);
	} else {
		stopSpeech();
	}
}

void Scene5030::cancelWalk() {
	_walkPath.clear();
	_pendingHotspot = _pendingItem = 0;
	_committedCaption.clear();
}

void Scene5030::playWalkFootstep() {
	// The sixth E03 lookup table selects the walking surface (native
	// 0x4282e2). Zero is silent; the shack uses surface 3. Volume follows
	// the native actor-height / 455 ratio, so distant steps are quieter.
	const byte surface = _maps[1280 + _mask[_position.y * kRoomWidth + _position.x]];
	const uint volume = CLIP<int>((int)(RunawayActor::perspectiveHeight(_position) * 100 / 455), 0, 100);
	if (surface >= 1 && surface <= 7) {
		_audio.playFootstep(surface, _random.getRandomBit(), volume * 255 / 100, _position.x);
	}
	_lastFootstep = _walkElapsed;
}

void Scene5030::click(Common::Point mouse, RunawayVerb verb, byte item) {
	if (_action != kNone || speaking() || mouse.y < kRoomTop || mouse.y >= kRoomTop + kRoomHeight)
		return;
	const byte id = hotspotAt(mouse);
	debugC(1, kDebugScene, "Runaway demo: verb %u item %u at %d,%d, hotspot %u",
		(uint)verb, item, mouse.x + _cameraX, mouse.y - kRoomTop, id);
	_pendingHotspot = id;
	_pendingItem = item;
	_pendingVerb = verb;
	_committedCaption = actionCaption(id, verb, item);
	if (_game == kRunawayFullGame && id) {
		bool approach = kHotspots[id].approach[verb == kVerbLook ? 0 : 1];
		if (item) {
			approach = (item == 1 && id == 4) || (item == 2 && id == 6) || (item == 6 && id == 8);
		} else if (verb == kVerbLook && (id == 5 || id == 7 || id == 9 || id == 15 || id == 17)) {
			approach = !_examined[id];
		}
		if (!approach) {
			const Common::Point target = id == 8 && _coolerMoved ? Common::Point(1128, 406) : kHotspots[id].lookPosition;
			_facing = runawayFacingTo(_position, target);
			_walkPath.clear();
			arrive(false);
			return;
		}
	}
	const Common::Point target = id == 8 && _coolerMoved ? Common::Point(1054, 448) :
		id ? Common::Point(kHotspots[id].x, kHotspots[id].y) :
		Common::Point(mouse.x + _cameraX, mouse.y - kRoomTop);
	walkTo(target);
}

void Scene5030::pickUp(byte hotspot, byte item, uint patch) {
	if (!_inventory.add(item))
		return;
	_pickedUp[hotspot] = true;
	drawPatch(_pickupPatches[patch]);
	if (hotspot == 14 || hotspot == 17)
		sayCue(hotspot == 14 ? 26 : 31, 0);
	else
		sayInventory(RunawayInventory::Reply(18, _random.getRandomNumber(2)));
}

void Scene5030::useItem(byte item, byte hotspot) {
	_inventory.select(0);
	if (!_inventory.contains(item))
		return;
	// Dispatch the room's item actions separately from its ordinary verbs.
	// Original rejected actions use the shared, voiced inventory responses.
	if (item == 1 && hotspot == 3) {
		// 339: Use the chamois with the large window.
		sayCue(35, 0);
	} else if (item == 1 && hotspot == 4) {
		// 337: Clean the small window with the damp chamois.
		if (_windowClean) {
			sayCue(33, 3);
		} else if (!_inventory.clothState()) {
			sayCue(33, 0);
		} else {
			// Load before committing the interaction, so missing/corrupt movie
			// data cannot leave Brian halfway through climbing onto the cooler.
			if (!_video.load("DATAVE01.001"))
				error("Cannot load Runaway cleaning movie DATAVE01.001");
			startAction(kCleanWindowSpeech);
			sayCue(33, 1);
		}
	} else if (item == 3 && hotspot == 4) {
		// 343: Use the cleaner with the small window.
		sayCue(39, 0);
	} else if (item == 2 && hotspot == 4) {
		// 342: Use the crowbar with the small window.
		sayCue(38, 0);
	} else if (item == 2 && hotspot == 6) {
		// 338: Pry the padlock off the trap door.
		if (_hatchState == kLocked)
			waitForSpeech(34, 0, kPryHatch);
		else
			sayCue(34, _hatchState == kUnlocked ? 2 : 3);
	} else if (_game == kRunawayFullGame && item == 4 && hotspot == 8) {
		// 340: Use butter with the freezer.
		sayCue(36, 0);
	} else if (_game == kRunawayFullGame && item == 6 && hotspot == 8) {
		// 341: Heat the helmet's contents on the sunlit freezer lid.
		switch (_inventory.desert().helmet) {
		case 0:
		case 4:
			sayCue(37, 0);
			break;
		case 1:
			sayCue(37, 1);
			break;
		case 2:
			waitForSpeech(37, 2, kHeatHelmet);
			break;
		case 3:
			sayCue(37, 4);
			break;
		default:
			break;
		}
	} else {
		sayInventory(_game == kRunawayFullGame ? RunawayInventory::Reply::fromAction(1) : RunawayInventory::Reply());
	}
}

void Scene5030::examine(byte id) {
	const bool examined = _examined[id];
	_examined[id] = true;
	switch (id) {
	case 1: // Look at the door.
		sayCue(1, _peeked ? 1 : 0);
		break;
	case 2: // Look through the crack in the door.
		startAction(kPeekDoor);
		break;
	case 3: // Look through the large window.
		sayCue(5, 0);
		break;
	case 4: // Look through the small window.
		sayCue(7, _windowClean ? 1 : 0);
		break;
	case 5: // Look at the wooden peg.
		sayCue(9, examined ? 1 : 0);
		break;
	case 6: // Look at the trap door.
		sayCue(_hatchState == kOpen ? 12 : 10, _hatchState == kUnlocked ? 1 : 0);
		break;
	case 7: // Look at Brian's torn shirt.
		sayCue(27, examined ? 1 : 0);
		break;
	case 8: // Look at the freezer and its contents.
		_coolerInspected |= _lidOpen;
		sayCue(13, _coolerMoved ? 0 : !_lidOpen ? 1 : _ice == kDrained ? 6 :
			_ice == kMelted ? 5 : _windowClean ? (_unplugged ? 5 : 4) : (_unplugged ? 3 : 2));
		break;
	case 9: // Look at the chamois.
		sayCue(15, examined ? 1 : 0);
		break;
	case 10: // Look at the power plug.
		sayCue(16, 0);
		break;
	case 11: // Look at the freezer drain.
		sayCue(18, 0);
		break;
	case 12: // Look at the telephone.
		sayCue(21, 0);
		break;
	case 13: // Look at the shelf.
		sayCue(23, 0);
		break;
	case 14: // Look at the crowbar.
		sayCue(25, 0);
		break;
	case 15: // Look at Brian's broken glasses.
		sayCue(40, 0);
		break;
	case 16: // Look at the bellows.
		sayCue(29, 0);
		break;
	case 17: // Look at the cleaner.
		sayCue(30, examined ? 1 : 0);
		break;
	case 18: // Look at the freezer lid.
		sayCue(32, _lidOpen ? 1 : 0);
		break;
	default:
		break;
	}
}

void Scene5030::collect(byte hotspot) {
	_pickupHotspot = hotspot;
	if (!_examined[hotspot] && (hotspot == 5 || hotspot == 9 || hotspot == 17)) {
		_examined[hotspot] = true;
		waitForSpeech(hotspot == 5 ? 9 : hotspot == 9 ? 15 : 30, 0, kPickup);
	} else {
		startAction(kPickup);
	}
}

void Scene5030::waitForSpeech(uint16 cue, byte variation, Action next) {
	startAction(kWaitSpeech);
	_afterSpeech = next;
	sayCue(cue, variation);
}

void Scene5030::walkForAction(Common::Point target, byte facing, Action next) {
	startAction(kWalkingAction);
	_afterWalk = next;
	_actionFacing = facing;
	walkTo(target);
}

void Scene5030::setPosition(Common::Point position, byte facing) {
	_position = position;
	_walkPath.reset(position);
	_facing = facing;
}

void Scene5030::arrive(bool faceHotspot) {
	if (_action == kWalkingAction) {
		_facing = _actionFacing;
		startAction(_afterWalk);
		return;
	}
	const byte id = _pendingHotspot, item = _pendingItem;
	_pendingHotspot = _pendingItem = 0;
	_committedCaption.clear();
	if (!id)
		return;
	if (faceHotspot) {
		_facing = kHotspots[id].facing;
	}
	if (item) {
		useItem(item, id);
	} else if (_pendingVerb == kVerbLook) {
		examine(id);
	} else if (id == 5 || id == 9 || id == 14 || id == 16 || id == 17) {
		collect(id);
	} else if (id == 18) {
		if (_ice != kFrozen)
			sayCue(32, 2);
		else
			startAction(_lidOpen ? kCloseLid : kOpenLid);
	} else if (id == 8) {
		if (_coolerMoved)
			sayCue(14, 0);
		else
			walkForAction(Common::Point(1222, 538), 7, _ice == kDrained ? kRevealHatch : kTryMoveCooler);
	} else if (id == 10 && !_unplugged) {
		if (_coolerInspected)
			startAction(kUnplug);
		else
			sayInventory(RunawayInventory::Reply(25, 0));
	} else if (id == 11) {
		if (_ice == kDrained)
			sayCue(19, 3);
		else if (_ice == kMelted)
			waitForSpeech(19, 1, kDrainCooler);
		else if (_coolerInspected)
			sayCue(19, 0);
		else
			sayInventory(RunawayInventory::Reply(25, 0));
	} else if (id == 6) {
		if (_hatchState == kLocked)
			sayCue(11, 0);
		else if (_hatchState == kUnlocked)
			startAction(kOpenHatch);
		else if (_hatchState == kOpen)
			waitForSpeech(42, 0, kEscape);
	} else if (id == 7 && !_examined[7]) {
		_examined[7] = true;
		waitForSpeech(27, 0, kShirt);
	} else {
		const uint16 cues[] = {0, 2, 4, 6, 8, 0, 11, 28, 14, 0, 17, 19, 22, 24, 26, 41};
		if (id < ARRAYSIZE(cues))
			sayCue(cues[id], 0);
	}
}

void Scene5030::startAction(Action action) {
	_action = action;
	_actionStarted = _now;
	_step = 0;
	stopSpeech();
	uint chunk = 0, frames = 0;
	switch (action) {
	case kIntroLookAround:
		waitForSpeech(0, 1, kIntroInventory);
		break;
	case kIntroInventory:
		setPosition(_position, 4);
		sayCue(0, 2);
		break;
	case kIntroListen:
		waitForSpeech(0, 3, kIntroWalkToDoor);
		break;
	case kIntroWalkToDoor:
		walkForAction(Common::Point(101, 507), 7, kIntroExamineDoor);
		break;
	case kIntroExamineDoor:
		_examined[1] = true;
		waitForSpeech(1, 0, kPeekDoor);
		break;
	case kPickup:
		_action = kNone;
		switch (_pickupHotspot) {
		case 5: // Take the wooden peg.
			pickUp(5, 8, 0);
			break;
		case 9: // Take the chamois.
			pickUp(9, 1, 1);
			break;
		case 14: // Take the crowbar.
			pickUp(14, 2, 2);
			break;
		case 16: // Take the bellows.
			pickUp(16, 24, 3);
			break;
		case 17: // Take the cleaner.
			pickUp(17, 3, 4);
			break;
		default:
			break;
		}
		break;
	case kShirt:
		_action = kNone;
		sayCue(28, 0);
		break;
	case kMeltIce:
		waitForSpeech(20, 0, kMeltWait);
		break;
	case kInspectIce:
		walkForAction(Common::Point(1062, 473), 1, kFinishMelting);
		break;
	case kFinishMelting:
		_ice = kMelted;
		_action = kNone;
		sayCue(13, 5);
		break;
	case kTryMoveCooler:
		chunk = _windowClean ? 37 : 38;
		frames = 26;
		break;
	case kRevealHatch:
		waitForSpeech(14, 4, kMoveCooler);
		break;
	case kMoveCooler:
		chunk = 41;
		frames = 83;
		applyPatch(23);
		break;
	case kFinishMoving:
		walkForAction(Common::Point(1215, 542), 7, kHatchFound);
		break;
	case kHatchFound:
		_action = kNone;
		sayCue(14, 6);
		break;
	case kDrainCooler:
		chunk = 39;
		frames = 59;
		if (!_water.load(_archive, 33, 97))
			error("Cannot load Runaway draining animation");
		break;
	case kFinishDraining:
		_action = kNone;
		sayCue(19, 2);
		break;
	case kPryHatch:
		chunk = 32;
		frames = 43;
		break;
	case kOpenHatch:
		chunk = 36;
		frames = 47;
		applyPatch(24);
		break;
	case kEscape:
		chunk = 35;
		frames = 37;
		break;
	case kEnterHatch: // Climb back into the cabin through the trap door.
		chunk = 34;
		frames = 32;
		break;
	case kPeekDoor:
		chunk = 22;
		frames = 19;
		break;
	case kReturnFromPeek:
		chunk = 22;
		frames = 19;
		break;
	case kHeatHelmet:
		chunk = 31;
		frames = 69;
		break;
	case kPrepareStirringHelmet:
		_facing = 4;
		waitForSpeech(37, 6, kStirHelmet);
		break;
	case kStirHelmet:
		if (!loadRoomGraphics())
			error("Cannot rebuild Runaway room before stirring the helmet");
		chunk = 31;
		frames = 69;
		break;
	default:
		break;
	}
	if (frames && !_actionAnimation.load(_archive, chunk, frames))
		error("Cannot load Runaway action animation %u", chunk);
	debugC(1, kDebugScene, "Runaway demo: animation %u started", (uint)action);
}

void Scene5030::finishAction() {
	const Action completed = _action;
	_action = kNone;
	if (completed == kIntroVideo) {
		_video.close();
		startAction(kIntro);
	} else if (completed == kIntro) {
		// 0x4377a0: wake up, lament the theft in the inventory close-up,
		// then walk to the door and listen before giving control to the player.
		waitForSpeech(0, 0, kIntroLookAround);
	} else if (completed == kOpenLid || completed == kCloseLid) {
		_lidOpen = completed == kOpenLid;
		drawPatch(_lidOpen ? _openLidPatch : _closedLidPatch);
		_facing = 1;
		if (_lidOpen && !_coolerInspected) {
			_coolerInspected = true;
			sayCue(13, 2);
		}
	} else if (completed == kUnplug) {
		_unplugged = true;
		drawPatch(_unplugPatch);
		_facing = 7;
		sayCue(17, 0);
	} else if (completed == kCleanWindowVideo) {
		_video.close();
		_audio.stopEffect();
		_windowClean = true;
		_inventory.soilCloth();
		if (!loadRoomGraphics())
			error("Cannot load Runaway sunlit room resources");
		_facing = 1;
		sayCue(33, 2);
	} else if (completed == kMeltWait) {
		waitForSpeech(20, 1, kInspectIce);
	} else if (completed == kTryMoveCooler) {
		sayCue(14, _ice == kMelted ? 3 : 2);
	} else if (completed == kMoveCooler) {
		_coolerMoved = true;
		_hatchState = kLocked;
		if (!loadRoomGraphics())
			error("Cannot rebuild Runaway room after moving the cooler");
		setPosition(Common::Point(1156, 468), 3);
		waitForSpeech(14, 5, kFinishMoving);
	} else if (completed == kDrainCooler) {
		_ice = kDrained;
		applyPatch(11);
		walkForAction(Common::Point(1062, 473), 1, kFinishDraining);
	} else if (completed == kPryHatch || completed == kOpenHatch) {
		_hatchState = completed == kPryHatch ? kUnlocked : kOpen;
		if (!loadRoomGraphics())
			error("Cannot rebuild Runaway hatch graphics");
		if (completed == kPryHatch)
			sayCue(34, 1);
	} else if (completed == kEscape) {
		if (_game == kRunawayFullGame) {
			_transition.scene = 5021;
			_transition.position = Common::Point(618, 386);
			_transition.facing = 3;
		} else {
			_escaped = true;
		}
		_audio.stopMusic();
		_audio.stopEffect();
	} else if (completed == kPeekDoor) {
		if (!_interrogation.load())
			error("Cannot load Runaway interrogation scene");
		startAction(kPeekConversation);
		_interrogation.start(_now);
	} else if (completed == kReturnFromPeek) {
		_facing = 4;
		sayCue(_peeked ? 3 : 1, _peeked ? _random.getRandomNumber(3) : 1);
		_peeked = true;
	} else if (completed == kHeatHelmet) {
		applyPatch(25);
		waitForSpeech(37, 5, kPrepareStirringHelmet);
	} else if (completed == kStirHelmet) {
		_audio.stopEffect();
		_inventory.desert().helmet = 3;
		sayCue(37, 3);
	}
	debugC(1, kDebugScene, "Runaway demo: animation %u finished, lid=%u unplugged=%u",
		(uint)completed, (uint)_lidOpen, (uint)_unplugged);
}

void Scene5030::skipVideo() {
	if (playingVideo())
		finishAction();
}

void Scene5030::updateAction() {
	const uint previousStep = _step;
	const uint32 elapsed = _now - _actionStarted;
	if (_action == kWaitSpeech) {
		if (!speaking())
			startAction(_afterSpeech);
	} else if (_action == kIntroInventory) {
		if (!speaking())
			startAction(kIntroListen);
	} else if (_action == kPeekConversation) {
		_interrogation.update(_now);
		if (_interrogation.finished())
			startAction(kReturnFromPeek);
	} else if (_action == kCleanWindowSpeech) {
		if (!speaking())
			startAction(kCleanWindowClimb);
	} else if (_action == kCleanWindowClimb) {
		_step = elapsed / 60;
		if (_step >= 18)
			startAction(kCleanWindowVideo);
	} else if (playingVideo()) {
		_step = (uint64)elapsed * 24 / 1000;
		// Both demo movies run at 24 fps. Dispatch callbacks on crossing a
		// frame, including delayed updates, so each effect plays exactly once.
		if (_action == kIntroVideo && previousStep < 353 && _step >= 353) {
			_audio.playFootstep(3, _random.getRandomBit(), 255, 512);
		}
		if (_action == kCleanWindowVideo && previousStep < 56 && _step >= 56)
			_audio.playEffect(22);
		if (_step >= _video.frameCount()) {
			finishAction();
		} else if (!_video.decodeToFrame(_step)) {
			error("Cannot decode Runaway movie frame %u", _step);
		}
	} else if (_action == kMeltWait) {
		if (elapsed >= 3000)
			finishAction();
	} else if (_action == kDrainCooler) {
		// Crouch at 60 ms per cel, hold while the water runs at 80 ms,
		// then stand up. The actor and water tracks have separate native clocks.
		_step = elapsed < 2700 ? elapsed / 60 : elapsed < 10380 ? 45 : 45 + (elapsed - 10380) / 60;
		if (previousStep < 41 && _step >= 41)
			_audio.playEffect(28);
		if (previousStep < 44 && _step >= 44) {
			_audio.playEffect(16, 204, 1);
			applyPatch(11);
		}
		if (_step >= 81)
			finishAction();
	} else if (_action != kNone && _action != kWalkingAction) {
		_step = elapsed / 60;
		uint length = 0;
		switch (_action) {
		case kIntro:
			length = 19;
			if (previousStep < 5 && _step >= 5) {
				_audio.playFootstep(3, _random.getRandomBit(), 179, 512);
			}
			break;
		case kOpenLid:
		case kCloseLid: {
			length = 33;
			const uint start = _action == kOpenLid ? 18 : 11;
			if (previousStep < start && _step >= start) {
				_audio.playEffectAt(80, Common::Point(1238, 200), 153);
			}
			if (previousStep < start + 4 && _step >= start + 4)
				_audio.stopEffect();
			break;
		}
		case kUnplug:
			length = 83;
			if (previousStep < 45 && _step >= 45) {
				_audio.playEffectAt(83, Common::Point(1273, 200));
			}
			break;
		case kTryMoveCooler:
			length = 74;
			if (previousStep < 13 && _step >= 13)
				sayCue(14, 1);
			break;
		case kMoveCooler:
			length = 83;
			if (previousStep < 27 && _step >= 27)
				_audio.playEffect(29);
			break;
		case kPryHatch:
			length = 57;
			if (previousStep < 30 && _step >= 30)
				_audio.playEffect(30);
			if (previousStep < 31 && _step >= 31)
				applyPatch(13);
			break;
		case kOpenHatch:
			length = 47;
			if (previousStep < 25 && _step >= 25) {
				_audio.playEffectAt(17, Common::Point(1184, 200));
			}
			break;
		case kEscape:
			length = 37;
			break;
		case kEnterHatch:
			length = 32;
			if (previousStep < 24 && _step >= 24) {
				_audio.playFootstep(3, _random.getRandomBit(), 230, 1238);
			}
			break;
		case kPeekDoor:
		case kReturnFromPeek:
			length = 20;
			break;
		case kHeatHelmet:
			length = 28;
			if (previousStep < 22 && _step >= 22)
				_audio.playEffect(70);
			break;
		case kStirHelmet:
			length = 90;
			if (previousStep < 5 && _step >= 5)
				_audio.playEffect(71, 191);
			if (previousStep < 81 && _step >= 81)
				_audio.stopEffect();
			break;
		default:
			break;
		}
		if (length && _step >= length)
			finishAction();
	}
}

void Scene5030::update(uint32 now, int scrollDirection, bool inventoryOpen) {
	_now = now;
	const uint elapsed = MIN<uint>(100, now - _lastUpdate);
	const uint32 pausedTime = now - _lastUpdate;
	_lastUpdate = now;
	if (_speechRow && !_audio.speechPlaying() && _speech.finished(now))
		skipSpeech();
	if (inventoryOpen && !inventoryPresentation()) {
		_actionStarted += pausedTime;
		_idleHead.pause(pausedTime);
		return;
	}
	updateAction();
	if ((_action == kNone || _action == kWalkingAction) && _walkPath.active()) {
		const uint previousWalkFrame = _walkElapsed / 60;
		if (_walkPath.advance(elapsed * 0.18f, _position, _facing)) {
			_walkElapsed += elapsed;
			// Native walking repeats cels 1..12 at 60 ms, with contacts at
			// 6 and 12. Cels 13..18 belong to stopping/turning, not the gait.
			for (uint frame = previousWalkFrame + 1; frame <= _walkElapsed / 60; ++frame) {
				if ((1 + frame % 12) % 6 == 0)
					playWalkFootstep();
			}
			// The final standing cel (0) is also a contact. Suppress a
			// duplicate when it immediately follows another footfall.
			if (!_walkPath.active() && (!_lastFootstep || _walkElapsed - _lastFootstep > 60))
				playWalkFootstep();
		}
		if (!_walkPath.active()) {
			debugC(1, kDebugScene, "Runaway demo: arrived at %d,%d", _position.x, _position.y);
			arrive();
		}
	}
	if (_action == kNone && !speaking() && !_walkPath.active() &&
			_windowClean && _unplugged && _lidOpen && _ice == kFrozen)
		walkForAction(Common::Point(900, 473), 2, kMeltIce);
	const bool walking = _walkPath.active();
	const bool idle = !walking && !_speechRow && _action == kNone;
	_idleHead.update(now, idle, _speech.frameDuration(), _random);
	const int headFrame = walking ? -1 : _speechRow ? speechFrame() : idle ? _idleHead.frame() : -1;
	_actor.setFrame(_facing, walking ? 1 + (_walkElapsed / 60) % 12 : 0, headFrame);
	if (scrollDirection)
		_cameraX += scrollDirection * MAX<int>(1, elapsed / (_fastScrolling ? 2 : 4));
	else if (walking || (_action != kNone && _action != kIntro))
		_cameraX += CLIP<int>(_position.x - kScreenWidth / 2 - _cameraX,
			-MIN<int>(_fastScrolling ? 8 : 4, elapsed), MIN<int>(_fastScrolling ? 8 : 4, elapsed));
	setCameraX(_cameraX);
}

void Scene5030::draw() {
	_screen.clear();
	if (_action == kPeekConversation) {
		_interrogation.draw(_screen);
		return;
	}
	if (playingVideo()) {
		_video.draw(_screen, Common::Rect(0, kRoomTop, kScreenWidth, kRoomTop + kRoomHeight));
		return;
	}
	_screen.copyRectToSurface(_background, 0, kRoomTop,
		Common::Rect(_cameraX, 0, _cameraX + kScreenWidth, kRoomHeight));
	uint lidFrame = _lidOpen ? 4 : 0;
	if (_action == kOpenLid)
		lidFrame = CLIP<int>((int)_step - 17, 0, 4);
	else if (_action == kCloseLid)
		lidFrame = CLIP<int>(15 - (int)_step, 0, 4);
	if (!_coolerMoved && _action != kMoveCooler)
		_lid.draw(_screen, lidFrame, _cameraX, kRoomTop);
	if (_hatchState >= kUnlocked && _action != kOpenHatch)
		_hatch.draw(_screen, _hatchState == kOpen ? 1 : 0, _cameraX, kRoomTop);
	if (_action == kIntro) {
		_intro.draw(_screen, MAX<int>(0, (int)_step - 1), _cameraX, kRoomTop);
	} else if (_action == kOpenLid || _action == kCloseLid) {
		_lidActor.draw(_screen, _action == kOpenLid ? _step : 32 - _step, _cameraX, kRoomTop);
	} else if (_action == kUnplug) {
		// Original 0x435a50 sequence: crouch, pull the plug, return to standing.
		const uint frame = _step <= 36 ? MAX<int>(0, (int)_step - 1) : _step <= 47 ? _step + 10 : 83 - _step;
		_unplug.draw(_screen, frame, _cameraX, kRoomTop);
	} else if (_action == kCleanWindowClimb) {
		_climb.draw(_screen, MAX<int>(0, (int)_step - 1), _cameraX, kRoomTop);
	} else if (_action == kTryMoveCooler) {
		_actionAnimation.draw(_screen, _step < 25 ? _step : _step < 50 ? 25 : 74 - _step, _cameraX, kRoomTop);
	} else if (_action == kMoveCooler) {
		_actionAnimation.draw(_screen, _step, _cameraX, kRoomTop);
	} else if (_action == kDrainCooler) {
		const uint frame = _step <= 47 ? (_step ? _step - 1 : 0) : 82 - _step;
		_actionAnimation.draw(_screen, frame, _cameraX, kRoomTop);
		const uint elapsed = _now - _actionStarted;
		if (elapsed >= 2700)
			_water.draw(_screen, MIN<uint>(96, (elapsed - 2700) / 80), _cameraX, kRoomTop);
	} else if (_action == kPryHatch) {
		const uint frame = _step <= 43 ? (_step ? _step - 1 : 0) : 56 - _step;
		_actionAnimation.draw(_screen, frame, _cameraX, kRoomTop);
	} else if (_action == kOpenHatch || _action == kEscape) {
		_actionAnimation.draw(_screen, MAX<uint>(1, _step), _cameraX, kRoomTop);
	} else if (_action == kEnterHatch) {
		_actionAnimation.draw(_screen, _step ? _step - 1 : 0, _cameraX, kRoomTop);
	} else if (_action == kPeekDoor || _action == kReturnFromPeek) {
		const uint frame = _action == kPeekDoor ? MAX<int>(0, (int)_step - 1) : 18 - MAX<int>(0, (int)_step - 1);
		_actionAnimation.draw(_screen, frame, _cameraX, kRoomTop);
	} else if (_action == kHeatHelmet || _action == kStirHelmet) {
		const uint step = _action == kHeatHelmet ? _step : _step + 29;
		const uint frame = step < 54 ? MAX<int>(0, (int)step - 1) : step < 103 ? 46 + (step - 54) % 7 : step - 50;
		_actionAnimation.draw(_screen, frame, _cameraX, kRoomTop);
	} else {
		_actor.draw(_screen, _position, _cameraX, kRoomTop, _mask, _maps, _windowClean, _antialias);
	}
	if (_action == kMeltWait) {
		const uint elapsed = _now - _actionStarted;
		const uint brightness = elapsed < 500 ? 500 - elapsed : elapsed < 2500 ? 0 : MIN<uint>(500, elapsed - 2500);
		for (int y = kRoomTop; y < kRoomTop + kRoomHeight; ++y) {
			uint16 *pixels = (uint16 *)_screen.getBasePtr(0, y);
			for (int x = 0; x < kScreenWidth; ++x) {
				const uint16 p = pixels[x];
				pixels[x] = (((p >> 11) * brightness / 500) << 11) |
					((((p >> 5) & 63) * brightness / 500) << 5) | ((p & 31) * brightness / 500);
			}
		}
	}
}

} // End of namespace Runaway
} // End of namespace Hollywood
