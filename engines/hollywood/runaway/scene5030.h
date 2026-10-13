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

#ifndef HOLLYWOOD_RUNAWAY_SCENE5030_H
#define HOLLYWOOD_RUNAWAY_SCENE5030_H

#include "common/ustr.h"
#include "hollywood/resource.h"
#include "hollywood/runaway/audio.h"
#include "hollywood/runaway/graphics.h"
#include "hollywood/runaway/inventory.h"
#include "hollywood/scene_text_store.h"

namespace Hollywood {

// The demo starts in the shack. This scene owns its room resources, actor,
// interactions and audio; RunawayEngine owns the backend and event loop.
class RunawayScene5030 {
public:
	enum {
		kScreenWidth = kRunawayScreenWidth, kScreenHeight = kRunawayScreenHeight,
		kRoomWidth = 1376, kRoomHeight = kRunawayViewportHeight, kRoomTop = kRunawayViewportTop
	};

	RunawayScene5030(Audio::Mixer *mixer, RunawayInventory &inventory) : _audio(mixer), _inventory(inventory) {}
	bool load();
	void start(uint32 now);
	void click(Common::Point mouse, RunawayVerb verb, byte item);
	void update(uint32 now, int scrollDirection, bool inventoryOpen);
	void draw();
	void setCameraX(int x);
	Graphics::ManagedSurface &screen() { return _screen; }
	byte hotspotAt(Common::Point mouse) const;
	Common::U32String actionCaption(byte hotspot, RunawayVerb verb, byte item) const;
	const Common::U32String &committedCaption() const { return _committedCaption; }
	const Common::U32String &subtitle() const { return _subtitle; }
	bool busy() const { return _action != kNone; }
	bool speaking() const { return _speechRow != 0; }
	void skipSpeech();
	void cancelWalk();
	void sayInventory(const RunawayInventory::Reply &reply);

private:
	enum Action { kNone, kIntro, kOpenLid, kCloseLid, kUnplug };
	bool walkable(int x, int y) const;
	bool clearPath(Common::Point start, Common::Point end) const;
	void walkTo(Common::Point target);
	void arrive();
	void say(uint16 row);
	void sayCue(uint16 cue, byte variation);
	void pickUp(byte hotspot, byte item, uint patch);
	void useItem(byte item, byte hotspot);
	void showSpeechRow();
	void stopSpeech();
	void startAction(Action action);
	void finishAction();
	bool loadPatch(uint chunk, Common::Array<byte> &patch);
	void drawPatch(const Common::Array<byte> &patch);

	int _cameraX = 111;
	ChunkArchive _archive;
	SceneTextStore _text;
	Graphics::ManagedSurface _background, _screen;
	Common::Array<byte> _mask, _maps, _unplugPatch, _openLidPatch, _closedLidPatch;
	Common::Array<byte> _pickupPatches[5];
	Common::Array<uint16> _voiceSamples, _cueEnds;
	RunawayAudio _audio;
	RunawayInventory &_inventory;
	RunawayActor _actor;
	RunawayAnimation _intro, _lidActor, _lid, _unplug;
	Common::Point _position = Common::Point(623, 447);
	Common::Array<Common::Point> _path;
	uint _pathIndex = 0;
	float _x = 623, _y = 447;
	byte _facing = 3, _pendingHotspot = 0;
	byte _pendingItem = 0;
	RunawayVerb _pendingVerb = kVerbWalk;
	Action _action = kNone;
	bool _lidOpen = false, _unplugged = false;
	bool _pickedUp[20] = {};
	bool _examined[20] = {};
	bool _globalSpeech = false;
	uint32 _now = 0, _lastUpdate = 0, _actionStarted = 0, _walkStarted = 0, _subtitleUntil = 0;
	uint _step = 0;
	uint16 _speechRow = 0, _speechEnd = 0;
	uint16 _speechSample = 0;
	Common::U32String _subtitle;
	Common::U32String _committedCaption;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE5030_H
