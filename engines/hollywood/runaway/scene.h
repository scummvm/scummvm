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

#ifndef HOLLYWOOD_RUNAWAY_SCENE_H
#define HOLLYWOOD_RUNAWAY_SCENE_H

#include "hollywood/resource.h"
#include "hollywood/runaway/audio.h"
#include "hollywood/runaway/graphics.h"
#include "hollywood/runaway/inventory.h"
#include "hollywood/scene_text_store.h"

namespace Hollywood {

class RunawayScene {
public:
	virtual ~RunawayScene() {}
	virtual bool load() = 0;
	virtual void start(uint32 now) = 0;
	virtual void click(Common::Point mouse, RunawayVerb verb, byte item) = 0;
	virtual void update(uint32 now, int scrollDirection, bool inventoryOpen) = 0;
	virtual void draw() = 0;
	virtual void setCameraX(int x) = 0;
	virtual int roomWidth() const = 0;
	virtual Graphics::ManagedSurface &screen() = 0;
	virtual byte hotspotAt(Common::Point mouse) const = 0;
	virtual Common::U32String actionCaption(byte hotspot, RunawayVerb verb, byte item) const = 0;
	virtual const Common::U32String &committedCaption() const = 0;
	virtual const Common::U32String &subtitle() const = 0;
	virtual bool busy() const = 0;
	virtual bool speaking() const = 0;
	virtual byte speechFrame() const { return 0; }
	virtual void skipSpeech() = 0;
	virtual bool playingVideo() const { return false; }
	virtual bool inventoryPresentation() const { return false; }
	virtual void skipVideo() {}
	virtual void sayInventory(const RunawayInventory::Reply &reply) = 0;
	virtual bool escaped() const { return false; }
	virtual bool canSave() const { return false; }
	virtual void captureState(RunawaySaveState &state) const {}
	virtual bool restoreState(const RunawaySaveState &state, uint32 now) { return false; }
	virtual void setOptions(bool antialias, bool fastScrolling) = 0;
};

class RunawayStrings;
struct MuseumHotspot;

// Shared player for the museum demo's numbered scenes.
class RunawayMuseumScene : public RunawayScene {
public:
	RunawayMuseumScene(Audio::Mixer *mixer, RunawayInventory &inventory, const RunawayStrings &strings);
	bool load() override;
	void start(uint32 now) override;
	void click(Common::Point mouse, RunawayVerb verb, byte item) override;
	void update(uint32 now, int scrollDirection, bool inventoryOpen) override;
	void draw() override;
	void setCameraX(int x) override;
	int roomWidth() const override { return _background.w; }
	Graphics::ManagedSurface &screen() override { return _screen; }
	byte hotspotAt(Common::Point mouse) const override;
	Common::U32String actionCaption(byte hotspot, RunawayVerb verb, byte item) const override;
	const Common::U32String &committedCaption() const override { return _caption; }
	const Common::U32String &subtitle() const override { return _subtitle; }
	bool busy() const override { return _pendingHotspot != 0 || _pendingDoorClose || speaking(); }
	bool speaking() const override { return !_subtitle.empty(); }
	void skipSpeech() override;
	void sayInventory(const RunawayInventory::Reply &reply) override;
	void setOptions(bool antialias, bool fastScrolling) override { _antialias = antialias; _fastScrolling = fastScrolling; }

private:
	enum ObjectState { kUnexamined, kIdentified, kTaken };
	enum ActionResult { kUnhandled, kHandled, kBackgroundChanged };

	bool enterRoom(uint16 room, Common::Point position, byte facing);
	bool loadForeground(uint layer, uint chunk);
	void drawForeground(Graphics::ManagedSurface &viewport, uint layer);
	bool rebuildBackground();
	bool applyPatch(uint chunk);
	void updateMaps();
	void walkTo(Common::Point target);
	void arrive();
	void execute(uint16 action);
	void useItem(byte item, byte hotspot);
	void say(uint16 cue, byte variation = 0, bool global = false);
	void queueLine(const Common::U32String &line);
	void showLine();
	void changeFloor(bool upper, Common::Point position);
	void updateCamera();
	float actorHeight() const;
	const MuseumHotspot *hotspot(byte id) const;
	Common::U32String hotspotName(byte id) const;

	byte walkZone() const { return _originalMaps[_mask[_position.y * _background.w + _position.x]]; }

	bool loadScene4030();
	bool applyBackground4030();
	void updateMaps4030();
	Common::U32String hotspotName4030(byte id) const;
	ActionResult execute4030(uint16 action);
	float actorHeight4030() const;
	void drawForeground4030(Graphics::ManagedSurface &viewport);

	bool loadScene4050();
	bool applyBackground4050();
	void updateMaps4050();
	ActionResult execute4050(uint16 action);
	float actorHeight4050() const;
	void drawForeground4050(Graphics::ManagedSurface &viewport);

	bool loadScene4060();
	void updateMaps4060();
	ActionResult execute4060(uint16 action);
	float actorHeight4060() const;
	void setupDepth4060(RunawayActor::RenderParams &params) const;

	float actorHeight4090() const;
	void setupDepth4090(RunawayActor::RenderParams &params) const;

	float actorHeight4100() const;
	void setupDepth4100(RunawayActor::RenderParams &params) const;

	bool loadScene4140();
	void clickScene4140(byte id);
	void drawScene4140(Graphics::ManagedSurface &viewport);

	RunawayAudio _audio;
	RunawayInventory &_inventory;
	const RunawayStrings &_strings;
	ChunkArchive _archive;
	SceneTextStore _text;
	RunawayActor _actor;
	RunawayAnimation _ambient, _overlay, _safe;
	Graphics::ManagedSurface _background, _screen;
	Common::Array<byte> _mask, _originalMaps, _maps;
	Common::Array<Common::Rect> _foreground[5];
	Common::Array<byte> _keyPatches[25];
	Common::Array<Common::Point> _path;
	Common::Array<Common::U32String> _lines;
	Common::U32String _subtitle, _caption;
	Common::Point _position, _camera;
	uint _pathIndex = 0, _lineIndex = 0;
	uint16 _room = 0, _pendingAction = 0;
	byte _facing = 3, _pendingHotspot = 0, _pendingItem = 0;
	float _x = 0, _y = 0;
	uint32 _now = 0, _lastUpdate = 0, _walkElapsed = 0, _subtitleUntil = 0;
	bool _upperFloor = true, _antialias = true, _fastScrolling = true;
	bool _pendingDoorClose = false;
	ObjectState _books = kUnexamined, _recorder = kUnexamined;
	bool _moneyIdentified = false, _safeOpen = false;
	bool _varnishTaken = false, _talcTaken = false, _coffeeTaken = false;
	byte _keyCount = 0, _pressedKey = 0;
	uint32 _keyUntil = 0;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE_H
