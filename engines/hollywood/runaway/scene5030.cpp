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
#include "hollywood/runaway/scene5030.h"
#include "hollywood/runaway/strings.h"

namespace Hollywood {
namespace {

struct SceneHotspot {
	int x, y;
	byte facing;
};

// Approach points and facings from the scene initializer at 0x439e40. The
// hit regions themselves are decoded from E03; these are not bounding boxes.
const SceneHotspot kHotspots[] = {
	{0, 0, 0},
	{101, 507, 7},
	{101, 507, 7},
	{406, 383, 0},
	{1062, 473, 1},
	{942, 407, 0},
	{1215, 542, 7},
	{406, 383, 7},
	{1062, 473, 7},
	{1019, 405, 0},
	{1215, 542, 7},
	{1215, 542, 7},
	{495, 381, 7},
	{873, 409, 1},
	{974, 406, 7},
	{262, 426, 0},
	{262, 426, 7},
	{609, 379, 1},
	{1062, 473, 1},
	{1215, 542, 7}
};

} // End of anonymous namespace

bool RunawayScene5030::load() {
	const Graphics::PixelFormat format(2, 5, 6, 5, 0, 11, 5, 0, 0);
	Common::Array<byte> packedMask;
	if (!_archive.open(Common::Path("RESOURCE.E03"), 50) ||
			!_archive.readVariableChunk(1, packedMask) ||
			!_archive.readVariableChunk(2, _maps) || _maps.size() != 1536 ||
			!_text.load("RESOURCE.003", "Runaway shack", 503, 0, 0x1135))
		return false;
	_background.create(kRoomWidth, kRoomHeight, format);
	_screen.create(kScreenWidth, kScreenHeight, format);
	// 0x42b380 expands (byte value, uint16 length) runs into one room pixel
	// lookup. Tables 0..3 map those values to walking, hotspots, depth, lighting.
	_mask.resize(kRoomWidth * kRoomHeight);
	uint offset = 0;
	if (packedMask.size() % 3)
		return false;
	for (uint i = 0; i < packedMask.size(); i += 3) {
		const uint length = READ_LE_UINT16(packedMask.data() + i + 1);
		if (!length || length > _mask.size() - offset)
			return false;
		memset(_mask.data() + offset, packedMask[i], length);
		offset += length;
	}
	if (offset != _mask.size() || !_actor.load() ||
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

bool RunawayScene5030::loadRoomGraphics() {
	Common::Array<byte> pixels, overlay;
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
		applyPatch(_hatchState == kLocked ? 12 : 13);
		if (_hatchState == kOpen)
			applyPatch(24);
	} else {
		drawPatch(_lidOpen ? _openLidPatch : _closedLidPatch);
		if (_unplugged)
			drawPatch(_unplugPatch);
		if (_ice == kDrained)
			applyPatch(11);
	}
	drawPatch(overlay);
	return true;
}

void RunawayScene5030::start(uint32 now) {
	_now = _lastUpdate = now;
	_audio.playMusic(11);
	startAction(kIntroVideo);
}

void RunawayScene5030::setCameraX(int x) {
	_cameraX = CLIP<int>(x, 0, kRoomWidth - kScreenWidth);
}

bool RunawayScene5030::loadPatch(uint chunk, Common::Array<byte> &patch) {
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

void RunawayScene5030::drawPatch(const Common::Array<byte> &patch) {
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

void RunawayScene5030::applyPatch(uint chunk) {
	Common::Array<byte> patch;
	if (!loadPatch(chunk, patch))
		error("Cannot load Runaway room patch %u", chunk);
	drawPatch(patch);
}

byte RunawayScene5030::hotspotAt(Common::Point mouse) const {
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

bool RunawayScene5030::walkable(int x, int y) const {
	return x >= 0 && x < kRoomWidth && y >= 0 && y < kRoomHeight && _maps[_mask[y * kRoomWidth + x]] != 0;
}

bool RunawayScene5030::clearPath(Common::Point start, Common::Point end) const {
	const int count = MAX(ABS(end.x - start.x), ABS(end.y - start.y));
	for (int i = 0; i <= count; ++i) {
		const int x = start.x + (end.x - start.x) * i / MAX(1, count);
		const int y = start.y + (end.y - start.y) * i / MAX(1, count);
		if (!walkable(x, y))
			return false;
	}
	return true;
}

void RunawayScene5030::walkTo(Common::Point target) {
	_path.clear();
	_pathIndex = 0;
	// Search only the connected floor. Using pixel resolution preserves the
	// one-pixel-wide approach to the plug, which a coarse navigation grid loses.
	Common::Array<int32> parent;
	Common::Array<int> queue;
	parent.resize(_mask.size());
	for (uint i = 0; i < parent.size(); ++i)
		parent[i] = -1;
	const int start = _position.y * kRoomWidth + _position.x;
	parent[start] = start;
	queue.push_back(start);
	int closest = start, bestDistance = 0x7fffffff;
	const int dx[] = {1, -1, 0, 0, 1, 1, -1, -1};
	const int dy[] = {0, 0, 1, -1, 1, -1, 1, -1};
	for (uint head = 0; head < queue.size(); ++head) {
		const int pixel = queue[head], x = pixel % kRoomWidth, y = pixel / kRoomWidth;
		const int distance = (x - target.x) * (x - target.x) + (y - target.y) * (y - target.y);
		if (distance < bestDistance) {
			closest = pixel;
			bestDistance = distance;
			if (!distance)
				break;
		}
		for (uint direction = 0; direction < 8; ++direction) {
			const int nx = x + dx[direction], ny = y + dy[direction], next = ny * kRoomWidth + nx;
			if (walkable(nx, ny) && parent[next] == -1) {
				parent[next] = pixel;
				queue.push_back(next);
			}
		}
	}
	Common::Array<Common::Point> reverse;
	for (int pixel = closest; pixel != start; pixel = parent[pixel])
		reverse.push_back(Common::Point(pixel % kRoomWidth, pixel / kRoomWidth));
	Common::Point previous = _position;
	for (int i = (int)reverse.size() - 1; i >= 0;) {
		int next = i;
		while (next > 0 && clearPath(previous, reverse[next - 1]))
			--next;
		previous = reverse[next];
		_path.push_back(previous);
		i = next - 1;
	}
	_walkElapsed = _lastFootstep = 0;
	debugC(1, kDebugScene, "Runaway demo: walk to %d,%d, reachable %d,%d, %u waypoints",
		target.x, target.y, closest % kRoomWidth, closest / kRoomWidth, _path.size());
	if (_path.empty())
		arrive();
}

void RunawayScene5030::say(uint16 row) {
	stopSpeech();
	if (row < 500 || row - 500 >= _voiceSamples.size())
		return;
	_globalSpeech = false;
	_speechRow = row;
	_speechEnd = MAX<uint16>(row, _cueEnds[row - 500]);
	showSpeechRow();
}

void RunawayScene5030::showSpeechRow() {
	_subtitle = Common::U32String(_text.largeTextRecord(_speechRow), Common::kDos850);
	const uint16 sample = _globalSpeech ? _speechSample : _voiceSamples[_speechRow - 500];
	const uint32 duration = sample ? _audio.playSpeech(sample, _globalSpeech) : 0;
	_subtitleUntil = _now + (duration ? MAX<uint>(1000, duration + 150) : MAX<uint>(3000, _subtitle.size() * 55));
	_idleHead.reset(_now);
	_lipSync.start(_subtitle, _now, duration, _random);
	debugC(1, kDebugScene, "Runaway demo: dialogue row %u, voice %u, ends at row %u",
		_speechRow, sample, _speechEnd);
}

void RunawayScene5030::stopSpeech() {
	_audio.stopSpeech();
	_speechRow = _speechEnd = 0;
	_lipSync.stop();
	_subtitle.clear();
}

Common::U32String RunawayScene5030::actionCaption(byte id, RunawayVerb verb, byte item) const {
	Common::U32String name = _strings.shackHotspotName(id);
	if (id < ARRAYSIZE(kHotspots) && _examined[id]) {
		if (id == 5 || id == 9 || id == 17)
			name = _inventory.name(id == 5 ? 8 : id == 9 ? 1 : 3);
		else if (id == 7)
			name = _strings.text(RunawayStrings::kShirt);
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

void RunawayScene5030::sayInventory(const RunawayInventory::Reply &reply) {
	stopSpeech();
	uint16 row, sample;
	byte count;
	if (!_text.getStaticSpeechCue(reply.cue, reply.variation, row, count, sample))
		return;
	if (!row || row + MAX<uint>(1, count) > _text._inventoryOwnerLargeRows.size() / 321 ||
			sample + MAX<uint>(1, count) > 6000)
		return;
	_globalSpeech = true;
	_speechRow = row;
	_speechEnd = row + MAX<uint>(1, count) - 1;
	_speechSample = sample;
	showSpeechRow();
}

void RunawayScene5030::sayCue(uint16 cue, byte variation) {
	const SceneSpeechCue speech = _text.stageCue(cue, variation);
	if (speech.valid())
		say(speech.textRecordId);
}

void RunawayScene5030::skipSpeech() {
	if (_action == kPeekConversation) {
		_interrogation.skipSpeech();
		return;
	}
	if (!_speechRow)
		return;
	_audio.stopSpeech();
	if (_speechRow < _speechEnd) {
		++_speechRow;
		if (_globalSpeech)
			++_speechSample;
		showSpeechRow();
	} else {
		stopSpeech();
	}
}

void RunawayScene5030::cancelWalk() {
	_path.clear();
	_pathIndex = 0;
	_pendingHotspot = _pendingItem = 0;
	_committedCaption.clear();
}

void RunawayScene5030::playWalkFootstep() {
	// The sixth E03 lookup table selects the walking surface (native
	// 0x4282e2). Zero is silent; the shack uses surface 3. Volume follows
	// the native actor-height / 455 ratio, so distant steps are quieter.
	const byte surface = _maps[1280 + _mask[_position.y * kRoomWidth + _position.x]];
	const uint volume = CLIP<int>((int)(RunawayActor::perspectiveHeight(_position) * 100 / 455), 0, 100);
	if (surface >= 1 && surface <= 7)
		_audio.playFootstep(surface, _random.getRandomBit(), volume * 255 / 100);
	_lastFootstep = _walkElapsed;
}

void RunawayScene5030::click(Common::Point mouse, RunawayVerb verb, byte item) {
	if (_action != kNone || speaking() || mouse.y < kRoomTop || mouse.y >= kRoomTop + kRoomHeight)
		return;
	const byte id = hotspotAt(mouse);
	debugC(1, kDebugScene, "Runaway demo: verb %u item %u at %d,%d, hotspot %u",
		(uint)verb, item, mouse.x + _cameraX, mouse.y - kRoomTop, id);
	_pendingHotspot = id;
	_pendingItem = item;
	_pendingVerb = verb;
	_committedCaption = actionCaption(id, verb, item);
	const Common::Point target = id == 8 && _coolerMoved ? Common::Point(1054, 448) :
		id ? Common::Point(kHotspots[id].x, kHotspots[id].y) :
		Common::Point(mouse.x + _cameraX, mouse.y - kRoomTop);
	walkTo(target);
}

void RunawayScene5030::pickUp(byte hotspot, byte item, uint patch) {
	if (!_inventory.add(item))
		return;
	_pickedUp[hotspot] = true;
	drawPatch(_pickupPatches[patch]);
	if (hotspot == 14 || hotspot == 17)
		sayCue(hotspot == 14 ? 26 : 31, 0);
	else
		sayInventory(RunawayInventory::Reply(18, _random.getRandomNumber(2)));
}

void RunawayScene5030::useItem(byte item, byte hotspot) {
	_inventory.select(0);
	if (!_inventory.contains(item))
		return;
	// Dispatch the room's item actions separately from its ordinary verbs.
	// Original rejected actions use the shared, voiced inventory responses.
	if (item == 1 && hotspot == 3) {
		// The large window is never cleaned; cue 33 belongs to the small one.
		sayCue(35, 0);
	} else if (item == 1 && hotspot == 4) {
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
		sayCue(39, 0);
	} else if (item == 2 && hotspot == 4) {
		sayCue(38, 0);
	} else if (item == 2 && hotspot == 6) {
		if (_hatchState == kLocked)
			waitForSpeech(34, 0, kPryHatch);
		else
			sayCue(34, _hatchState == kUnlocked ? 2 : 3);
	} else {
		sayInventory(RunawayInventory::Reply(1, 0));
	}
}

void RunawayScene5030::examine(byte id) {
	const bool examined = _examined[id];
	_examined[id] = true;
	switch (id) {
	case 1: sayCue(1, _peeked ? 1 : 0); break;
	case 2: startAction(kPeekDoor); break;
	case 3: sayCue(5, 0); break;
	case 4: sayCue(7, _windowClean ? 1 : 0); break;
	case 5: sayCue(9, examined ? 1 : 0); break;
	case 6: sayCue(_hatchState == kOpen ? 12 : 10, _hatchState == kUnlocked ? 1 : 0); break;
	case 7: sayCue(27, examined ? 1 : 0); break;
	case 8:
		_coolerInspected |= _lidOpen;
		sayCue(13, _coolerMoved ? 0 : !_lidOpen ? 1 : _ice == kDrained ? 6 :
			_ice == kMelted ? 5 : _windowClean ? (_unplugged ? 5 : 4) : (_unplugged ? 3 : 2));
		break;
	case 9: sayCue(15, examined ? 1 : 0); break;
	case 10: sayCue(16, 0); break;
	case 11: sayCue(18, 0); break;
	case 12: sayCue(21, 0); break;
	case 13: sayCue(23, 0); break;
	case 14: sayCue(25, 0); break;
	case 15: sayCue(40, 0); break;
	case 16: sayCue(29, 0); break;
	case 17: sayCue(30, examined ? 1 : 0); break;
	case 18: sayCue(32, _lidOpen ? 1 : 0); break;
	default: break;
	}
}

void RunawayScene5030::collect(byte hotspot) {
	_pickupHotspot = hotspot;
	if (!_examined[hotspot] && (hotspot == 5 || hotspot == 9 || hotspot == 17)) {
		_examined[hotspot] = true;
		waitForSpeech(hotspot == 5 ? 9 : hotspot == 9 ? 15 : 30, 0, kPickup);
	} else {
		startAction(kPickup);
	}
}

void RunawayScene5030::waitForSpeech(uint16 cue, byte variation, Action next) {
	startAction(kWaitSpeech);
	_afterSpeech = next;
	sayCue(cue, variation);
}

void RunawayScene5030::walkForAction(Common::Point target, byte facing, Action next) {
	startAction(kWalkingAction);
	_afterWalk = next;
	_actionFacing = facing;
	walkTo(target);
}

void RunawayScene5030::setPosition(Common::Point position, byte facing) {
	_position = position;
	_x = position.x;
	_y = position.y;
	_facing = facing;
	_path.clear();
	_pathIndex = 0;
}

void RunawayScene5030::arrive() {
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
	_facing = kHotspots[id].facing;
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

void RunawayScene5030::startAction(Action action) {
	_action = action;
	_actionStarted = _now;
	_step = 0;
	stopSpeech();
	uint chunk = 0, frames = 0;
	switch (action) {
	case kIntroLookAround: waitForSpeech(0, 1, kIntroInventory); break;
	case kIntroInventory:
		setPosition(_position, 4);
		sayCue(0, 2);
		break;
	case kIntroListen: waitForSpeech(0, 3, kIntroWalkToDoor); break;
	case kIntroWalkToDoor: walkForAction(Common::Point(101, 507), 7, kIntroExamineDoor); break;
	case kIntroExamineDoor:
		_examined[1] = true;
		waitForSpeech(1, 0, kPeekDoor);
		break;
	case kPickup:
		_action = kNone;
		switch (_pickupHotspot) {
		case 5: pickUp(5, 8, 0); break;
		case 9: pickUp(9, 1, 1); break;
		case 14: pickUp(14, 2, 2); break;
		case 16: pickUp(16, 24, 3); break;
		case 17: pickUp(17, 3, 4); break;
		default: break;
		}
		break;
	case kShirt: _action = kNone; sayCue(28, 0); break;
	case kMeltIce: waitForSpeech(20, 0, kMeltWait); break;
	case kInspectIce: walkForAction(Common::Point(1062, 473), 1, kFinishMelting); break;
	case kFinishMelting: _ice = kMelted; _action = kNone; sayCue(13, 5); break;
	case kTryMoveCooler: chunk = _windowClean ? 37 : 38; frames = 26; break;
	case kRevealHatch: waitForSpeech(14, 4, kMoveCooler); break;
	case kMoveCooler: chunk = 41; frames = 83; applyPatch(23); break;
	case kFinishMoving: walkForAction(Common::Point(1215, 542), 7, kHatchFound); break;
	case kHatchFound: _action = kNone; sayCue(14, 6); break;
	case kDrainCooler:
		chunk = 39;
		frames = 59;
		if (!_water.load(_archive, 33, 97))
			error("Cannot load Runaway draining animation");
		break;
	case kFinishDraining: _action = kNone; sayCue(19, 2); break;
	case kPryHatch: chunk = 32; frames = 43; break;
	case kOpenHatch: chunk = 36; frames = 47; applyPatch(24); break;
	case kEscape: chunk = 35; frames = 37; break;
	case kPeekDoor: chunk = 22; frames = 19; break;
	case kReturnFromPeek: chunk = 22; frames = 19; break;
	default: break;
	}
	if (frames && !_actionAnimation.load(_archive, chunk, frames))
		error("Cannot load Runaway action animation %u", chunk);
	debugC(1, kDebugScene, "Runaway demo: animation %u started", (uint)action);
}

void RunawayScene5030::finishAction() {
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
		_escaped = true;
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
	}
	debugC(1, kDebugScene, "Runaway demo: animation %u finished, lid=%u unplugged=%u",
		(uint)completed, (uint)_lidOpen, (uint)_unplugged);
}

void RunawayScene5030::skipVideo() {
	if (playingVideo())
		finishAction();
}

void RunawayScene5030::updateAction() {
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
		if (_action == kIntroVideo && previousStep < 353 && _step >= 353)
			_audio.playFootstep(3, _random.getRandomBit());
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
	} else if (_action != kNone && _action != kWalkingAction && _action != kPeekConversation) {
		_step = elapsed / 60;
		uint length = 0;
		switch (_action) {
		case kIntro:
			length = 19;
			if (previousStep < 5 && _step >= 5)
				_audio.playFootstep(3, _random.getRandomBit(), 179);
			break;
		case kOpenLid:
		case kCloseLid: {
			length = 33;
			const uint start = _action == kOpenLid ? 18 : 11;
			if (previousStep < start && _step >= start)
				_audio.playEffect(80, 153);
			if (previousStep < start + 4 && _step >= start + 4)
				_audio.stopEffect();
			break;
		}
		case kUnplug:
			length = 83;
			if (previousStep < 45 && _step >= 45)
				_audio.playEffect(83);
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
			if (previousStep < 25 && _step >= 25)
				_audio.playEffect(17);
			break;
		case kEscape: length = 37; break;
		case kPeekDoor:
		case kReturnFromPeek: length = 20; break;
		default: break;
		}
		if (length && _step >= length)
			finishAction();
	}
}

void RunawayScene5030::update(uint32 now, int scrollDirection, bool inventoryOpen) {
	_now = now;
	const uint elapsed = MIN<uint>(100, now - _lastUpdate);
	const uint32 pausedTime = now - _lastUpdate;
	_lastUpdate = now;
	if (_speechRow && !_audio.speechPlaying() && (int32)(now - _subtitleUntil) >= 0)
		skipSpeech();
	if (inventoryOpen && !inventoryPresentation()) {
		_actionStarted += pausedTime;
		_idleHead.pause(pausedTime);
		return;
	}
	updateAction();
	if ((_action == kNone || _action == kWalkingAction) && _pathIndex < _path.size()) {
		const uint previousWalkFrame = _walkElapsed / 60;
		const float previousX = _x, previousY = _y;
		float travel = elapsed * 0.18f;
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
		if (_x != previousX || _y != previousY) {
			_walkElapsed += elapsed;
			// Native walking repeats cels 1..12 at 60 ms, with contacts at
			// 6 and 12. Cels 13..18 belong to stopping/turning, not the gait.
			for (uint frame = previousWalkFrame + 1; frame <= _walkElapsed / 60; ++frame) {
				if ((1 + frame % 12) % 6 == 0)
					playWalkFootstep();
			}
			// The final standing cel (0) is also a contact. Suppress a
			// duplicate when it immediately follows another footfall.
			if (_pathIndex == _path.size() && (!_lastFootstep || _walkElapsed - _lastFootstep > 60))
				playWalkFootstep();
		}
		if (_pathIndex == _path.size()) {
			debugC(1, kDebugScene, "Runaway demo: arrived at %d,%d", _position.x, _position.y);
			arrive();
		}
	}
	if (_action == kNone && !speaking() && _pathIndex == _path.size() &&
			_windowClean && _unplugged && _lidOpen && _ice == kFrozen)
		walkForAction(Common::Point(900, 473), 2, kMeltIce);
	const bool walking = _pathIndex < _path.size();
	const bool idle = !walking && !_speechRow && _action == kNone;
	_idleHead.update(now, idle, _lipSync.frameDuration(), _random);
	const int headFrame = walking ? -1 : _speechRow ? speechFrame() : idle ? _idleHead.frame() : -1;
	_actor.setFrame(_facing, walking ? 1 + (_walkElapsed / 60) % 12 : 0, headFrame);
	if (scrollDirection)
		_cameraX += scrollDirection * MAX<int>(1, elapsed / 2);
	else if (walking || (_action != kNone && _action != kIntro))
		_cameraX += CLIP<int>(_position.x - kScreenWidth / 2 - _cameraX, -MIN<int>(8, elapsed), MIN<int>(8, elapsed));
	setCameraX(_cameraX);
}

void RunawayScene5030::draw() {
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
	} else if (_action == kPeekDoor || _action == kReturnFromPeek) {
		const uint frame = _action == kPeekDoor ? MAX<int>(0, (int)_step - 1) : 18 - MAX<int>(0, (int)_step - 1);
		_actionAnimation.draw(_screen, frame, _cameraX, kRoomTop);
	} else {
		_actor.draw(_screen, _position, _cameraX, kRoomTop, _mask, _maps, _windowClean);
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

} // End of namespace Hollywood
