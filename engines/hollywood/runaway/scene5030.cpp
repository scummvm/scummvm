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
	uint16 description;
	byte facing;
};

// Approach points and facings from the scene initializer at 0x439e40. The
// hit regions themselves are decoded from E03; these are not bounding boxes.
const SceneHotspot kHotspots[] = {
	{0, 0, 0, 0},
	{101, 507, 511, 7},
	{101, 507, 519, 7},
	{406, 383, 520, 0},
	{1062, 473, 523, 1},
	{942, 407, 528, 0},
	{1215, 542, 530, 7},
	{406, 383, 572, 7},
	{1062, 473, 535, 7},
	{1019, 405, 552, 0},
	{1215, 542, 554, 7},
	{1215, 542, 557, 7},
	{495, 381, 565, 7},
	{873, 409, 567, 1},
	{974, 406, 570, 7},
	{262, 426, 614, 0},
	{262, 426, 576, 7},
	{609, 379, 577, 1},
	{1062, 473, 580, 1},
	{1215, 542, 557, 7}
};

} // End of anonymous namespace

bool RunawayScene5030::load() {
	const Graphics::PixelFormat format(2, 5, 6, 5, 0, 11, 5, 0, 0);
	Common::Array<byte> pixels, packedMask;
	if (!_archive.open(Common::Path("RESOURCE.E03"), 50) ||
			_archive.chunkSize(0) != kRoomWidth * kRoomHeight * 2 ||
			!_archive.readVariableChunk(0, pixels) || !_archive.readVariableChunk(1, packedMask) ||
			!_archive.readVariableChunk(2, _maps) || _maps.size() != 1536 ||
			!_text.load("RESOURCE.003", "Runaway shack", 503, 0, 0x1135))
		return false;
	_background.create(kRoomWidth, kRoomHeight, format);
	_screen.create(kScreenWidth, kScreenHeight, format);
	for (int y = 0; y < kRoomHeight; ++y) {
		uint16 *out = (uint16 *)_background.getBasePtr(0, y);
		for (int x = 0; x < kRoomWidth; ++x)
			out[x] = READ_LE_UINT16(pixels.data() + (y * kRoomWidth + x) * 2);
	}
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
			!_intro.load(_archive, 47, 20) || !_lidActor.load(_archive, 43, 33) ||
			!_lid.load(_archive, 44, 5) || !_unplug.load(_archive, 40, 59) ||
			!loadPatch(5, _unplugPatch) || !loadPatch(7, _openLidPatch) || !loadPatch(9, _closedLidPatch))
		return false;
	const uint pickupChunks[] = {20, 18, 15, 19, 17};
	for (uint i = 0; i < ARRAYSIZE(pickupChunks); ++i) {
		if (!loadPatch(pickupChunks[i], _pickupPatches[i]))
			return false;
	}
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

void RunawayScene5030::start(uint32 now) {
	_now = _lastUpdate = now;
	_audio.playMusic(11);
	startAction(kIntro);
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

byte RunawayScene5030::hotspotAt(Common::Point mouse) const {
	if (mouse.x < 0 || mouse.x >= kScreenWidth || mouse.y < kRoomTop || mouse.y >= kRoomTop + kRoomHeight)
		return 0;
	byte id = _maps[256 + _mask[(mouse.y - kRoomTop) * kRoomWidth + mouse.x + _cameraX]];
	// The initial cooler hides the hatch and its later position. These remaps
	// correspond to the native room-state updater at 0x438ce0.
	if (id == 2)
		id = 1;
	if (id == 6 || id == 20)
		id = 8;
	if (id == 21 || (id == 10 && _unplugged))
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
	_walkStarted = _now;
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
	debugC(1, kDebugScene, "Runaway demo: dialogue row %u, voice %u, ends at row %u",
		_speechRow, sample, _speechEnd);
}

void RunawayScene5030::stopSpeech() {
	_audio.stopSpeech();
	_speechRow = _speechEnd = 0;
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
		return _strings.text(id == 3 || id == 4 ? RunawayStrings::kLookThrough : RunawayStrings::kLook) + name;
	RunawayStrings::TextId action = RunawayStrings::kUse;
	if (id == 5 || id == 9 || id == 14 || id == 16 || id == 17)
		action = RunawayStrings::kTake;
	else if (id == 18)
		action = _lidOpen ? RunawayStrings::kClose : RunawayStrings::kOpen;
	else if (id == 8)
		action = _lidOpen ? RunawayStrings::kMove : RunawayStrings::kOpen;
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
	const Common::Point target = id ? Common::Point(kHotspots[id].x, kHotspots[id].y) :
		Common::Point(mouse.x + _cameraX, mouse.y - kRoomTop);
	walkTo(target);
}

void RunawayScene5030::pickUp(byte hotspot, byte item, uint patch) {
	if (!_inventory.add(item))
		return;
	_pickedUp[hotspot] = true;
	drawPatch(_pickupPatches[patch]);
	sayInventory(RunawayInventory::Reply(18, 0));
}

void RunawayScene5030::useItem(byte item, byte hotspot) {
	_inventory.select(0);
	// Dispatch the room's item actions separately from its ordinary verbs.
	// Original rejected actions use the shared, voiced inventory responses.
	if (item == 1 && hotspot == 3) {
		// The large window is never cleaned; cue 33 belongs to the small one.
		sayCue(35, 0);
	} else {
		sayInventory(RunawayInventory::Reply(1, 0));
	}
}

void RunawayScene5030::arrive() {
	const byte id = _pendingHotspot, item = _pendingItem;
	_pendingHotspot = _pendingItem = 0;
	_committedCaption.clear();
	if (!id)
		return;
	_facing = kHotspots[id].facing;
	if (item) {
		useItem(item, id);
	} else if (_pendingVerb == kVerbLook) {
		_examined[id] = true;
		if (id == 18)
			say(_lidOpen ? 581 : 580);
		else if (id == 8)
			say(!_lidOpen ? 535 : _unplugged ? 537 : 536);
		else
			say(kHotspots[id].description);
	} else if (id == 5) {
		pickUp(id, 8, 0);
	} else if (id == 9) {
		pickUp(id, 1, 1);
	} else if (id == 14) {
		pickUp(id, 2, 2);
	} else if (id == 16) {
		pickUp(id, 24, 3);
	} else if (id == 17) {
		pickUp(id, 3, 4);
	} else if (id == 18 || (id == 8 && !_lidOpen)) {
		startAction(_lidOpen ? kCloseLid : kOpenLid);
	} else if (id == 10 && !_unplugged) {
		startAction(kUnplug);
	} else {
		const uint16 responses[] = {0, 512, 519, 521, 526, 528, 532, 572, 546, 552,
			555, 558, 566, 569, 570, 617, 576, 577, 580, 558};
		say(responses[id]);
	}
}

void RunawayScene5030::startAction(Action action) {
	_action = action;
	_actionStarted = _now;
	_step = 0;
	stopSpeech();
	debugC(1, kDebugScene, "Runaway demo: animation %u started", (uint)action);
}

void RunawayScene5030::finishAction() {
	const Action completed = _action;
	_action = kNone;
	if (completed == kIntro) {
		say(500);
	} else if (completed == kOpenLid || completed == kCloseLid) {
		_lidOpen = completed == kOpenLid;
		drawPatch(_lidOpen ? _openLidPatch : _closedLidPatch);
		_facing = 1;
		say(_lidOpen ? (_unplugged ? 537 : 536) : 580);
	} else if (completed == kUnplug) {
		_unplugged = true;
		drawPatch(_unplugPatch);
		_facing = 7;
		say(555);
	}
	debugC(1, kDebugScene, "Runaway demo: animation %u finished, lid=%u unplugged=%u",
		(uint)completed, (uint)_lidOpen, (uint)_unplugged);
}

void RunawayScene5030::update(uint32 now, int scrollDirection, bool inventoryOpen) {
	_now = now;
	const uint elapsed = MIN<uint>(100, now - _lastUpdate);
	const uint32 pausedTime = now - _lastUpdate;
	_lastUpdate = now;
	if (speaking() && !_audio.speechPlaying() && (int32)(now - _subtitleUntil) >= 0)
		skipSpeech();
	if (inventoryOpen) {
		_actionStarted += pausedTime;
		_walkStarted += pausedTime;
		return;
	}
	if (_action != kNone) {
		const uint previousStep = _step;
		_step = (now - _actionStarted) / 80;
		// Frame triggers in 0x437110/0x4373e0 (lid) and 0x435a50 (plug).
		if (_action == kOpenLid || _action == kCloseLid) {
			const uint start = _action == kOpenLid ? 18 : 11;
			if (previousStep < start && _step >= start)
				_audio.playEffect(80, 153);
			if (previousStep < start + 4 && _step >= start + 4)
				_audio.stopEffect();
		} else if (_action == kUnplug && previousStep < 45 && _step >= 45) {
			_audio.playEffect(83);
		}
		const uint length = _action == kIntro ? 20 : _action == kUnplug ? 83 : 33;
		if (_step >= length)
			finishAction();
	} else if (_pathIndex < _path.size()) {
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
		if (_pathIndex == _path.size()) {
			debugC(1, kDebugScene, "Runaway demo: arrived at %d,%d", _position.x, _position.y);
			arrive();
		}
	}
	const bool walking = _pathIndex < _path.size();
	_actor.setFrame(_facing, walking ? 1 + ((now - _walkStarted) / 70) % 18 : 0);
	if (scrollDirection)
		_cameraX += scrollDirection * MAX<int>(1, elapsed / 2);
	else if (walking || (_action != kNone && _action != kIntro))
		_cameraX += CLIP<int>(_position.x - kScreenWidth / 2 - _cameraX, -8, 8);
	setCameraX(_cameraX);
}

void RunawayScene5030::draw() {
	_screen.clear();
	_screen.copyRectToSurface(_background, 0, kRoomTop,
		Common::Rect(_cameraX, 0, _cameraX + kScreenWidth, kRoomHeight));
	uint lidFrame = _lidOpen ? 4 : 0;
	if (_action == kOpenLid)
		lidFrame = CLIP<int>((int)_step - 17, 0, 4);
	else if (_action == kCloseLid)
		lidFrame = CLIP<int>(15 - (int)_step, 0, 4);
	_lid.draw(_screen, lidFrame, _cameraX, kRoomTop);
	if (_action == kIntro) {
		_intro.draw(_screen, _step, _cameraX, kRoomTop);
	} else if (_action == kOpenLid || _action == kCloseLid) {
		_lidActor.draw(_screen, _action == kOpenLid ? _step : 32 - _step, _cameraX, kRoomTop);
	} else if (_action == kUnplug) {
		// Original 0x435a50 sequence: crouch, pull the plug, return to standing.
		const uint frame = _step <= 36 ? MAX<int>(0, (int)_step - 1) : _step <= 47 ? _step + 10 : 83 - _step;
		_unplug.draw(_screen, frame, _cameraX, kRoomTop);
	} else {
		_actor.draw(_screen, _position, _cameraX, kRoomTop, _mask, _maps);
	}
}

} // End of namespace Hollywood
