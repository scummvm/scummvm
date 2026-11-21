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

#include "common/random.h"
#include "common/ustr.h"
#include "hollywood/resource.h"
#include "hollywood/runaway/animation.h"
#include "hollywood/runaway/audio.h"
#include "hollywood/runaway/graphics.h"
#include "hollywood/runaway/inventory.h"
#include "hollywood/runaway/presentation.h"
#include "hollywood/runaway/scene.h"
#include "hollywood/runaway/video.h"
#include "hollywood/scene_text_store.h"

namespace Hollywood {
namespace Runaway {

// The demos and retail chapter three share the shack, its room resources, actor,
// interactions and audio; RunawayEngine owns the backend and event loop.
class Scene5030 : public RunawayScene {
public:
	enum {
		kScreenWidth = kRunawayScreenWidth, kScreenHeight = kRunawayScreenHeight,
		kRoomWidth = 1376, kRoomHeight = kRunawayViewportHeight, kRoomTop = kRunawayViewportTop
	};

	Scene5030(Audio::Mixer *mixer, RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings,
			RunawayGameType game = kRunawayCabinDemo) :
		_audio(audio), _inventory(inventory), _strings(strings), _game(game), _interrogation(mixer), _random("runawayShack"), _actor(),
		_speech(audio, _random, game == kRunawayFullGame ? RunawaySpeech::kRecordedLipSync : RunawaySpeech::kTextLipSync),
		_walkPath(Common::Point(623, 447)) {}
	bool load(uint16 entryScene) override;
	void start(uint32 now) override;
	void resume() override;
	void click(Common::Point mouse, RunawayVerb verb, byte item) override;
	void update(uint32 now, int scrollDirection, bool inventoryOpen) override;
	void draw() override;
	void setCameraX(int x) override;
	int cameraX() const override { return _cameraX; }
	int roomWidth() const override { return kRoomWidth; }
	Graphics::ManagedSurface &screen() override { return _screen; }
	byte hotspotAt(Common::Point mouse) const override;
	Common::U32String actionCaption(byte hotspot, RunawayVerb verb, byte item) const override;
	const Common::U32String &committedCaption() const override { return _committedCaption; }
	const Common::U32String &subtitle() const override { return _action == kPeekConversation ? _interrogation.subtitle() : _speech.subtitle(); }
	bool busy() const override { return _action != kNone || _transition.scene; }
	bool speaking() const override { return _action == kPeekConversation ? _interrogation.speaking() : _speechRow != 0; }
	byte speechFrame() const override { return _speech.frame(_now); }
	void skipSpeech() override;
	bool playingVideo() const override { return _action == kIntroVideo || _action == kCleanWindowVideo; }
	bool inventoryPresentation() const override { return _action == kIntroInventory; }
	void skipVideo() override;
	void cancelWalk();
	void sayInventory(const RunawayInventory::Reply &reply) override;
	bool escaped() const override { return _escaped; }
	bool canSave() const override { return !busy() && !speaking() && !_escaped && !_walkPath.active(); }
	void captureState(RunawaySaveState &state) const override;
	bool restoreState(const RunawaySaveState &state, uint32 now, bool entering = false) override;
	void setOptions(bool antialias, bool fastScrolling) override { _antialias = antialias; _fastScrolling = fastScrolling; }

private:
	Common::U32String hotspotName(byte id) const override;
	enum Action {
		kNone, kIntroVideo, kIntro, kIntroLookAround, kIntroInventory,
		kIntroListen, kIntroWalkToDoor, kIntroExamineDoor,
		kOpenLid, kCloseLid, kUnplug,
		kCleanWindowSpeech, kCleanWindowClimb, kCleanWindowVideo,
		kWaitSpeech, kWalkingAction, kPickup, kShirt,
		kMeltIce, kMeltWait, kInspectIce, kFinishMelting,
		kTryMoveCooler, kMoveCooler, kRevealHatch, kFinishMoving, kHatchFound,
		kDrainCooler, kFinishDraining, kPryHatch, kOpenHatch, kEscape,
		kPeekDoor, kPeekConversation, kReturnFromPeek,
		kHeatHelmet, kPrepareStirringHelmet, kStirHelmet, kEnterHatch
	};
	enum IceState { kFrozen, kMelted, kDrained };
	enum HatchState { kHidden, kLocked, kUnlocked, kOpen };
	bool loadRoomGraphics();
	void walkTo(Common::Point target);
	void arrive(bool faceHotspot = true);
	void say(uint16 row);
	void sayCue(uint16 cue, byte variation);
	void pickUp(byte hotspot, byte item, uint patch);
	void useItem(byte item, byte hotspot);
	void examine(byte hotspot);
	void collect(byte hotspot);
	void waitForSpeech(uint16 cue, byte variation, Action next);
	void walkForAction(Common::Point target, byte facing, Action next);
	void setPosition(Common::Point position, byte facing);
	void updateAction();
	void showSpeechRow();
	void stopSpeech();
	void playWalkFootstep();
	void startAction(Action action);
	void finishAction();
	bool loadPatch(uint chunk, Common::Array<byte> &patch);
	void applyPatch(uint chunk);
	void drawPatch(const Common::Array<byte> &patch);

	int _cameraX = 111;
	ChunkArchive _archive;
	SceneTextStore _text;
	Graphics::ManagedSurface _background, _screen;
	Common::Array<byte> _mask, _maps, _unplugPatch, _openLidPatch, _closedLidPatch;
	Common::Array<byte> _pickupPatches[5];
	Common::Array<uint16> _voiceSamples, _cueEnds;
	RunawayAudio &_audio;
	RunawayInventory &_inventory;
	const RunawayStrings &_strings;
	const RunawayGameType _game;
	RunawayInterrogation _interrogation;
	Common::RandomSource _random;
	bool _peeked = false;
	bool _entering = false;
	bool _antialias = true, _fastScrolling = true;
	RunawayActor _actor;
	RunawaySpeech _speech;
	RunawayIdleHead _idleHead;
	RunawayAnimation _intro, _lidActor, _lid, _unplug, _climb, _actionAnimation, _water, _hatch;
	RunawayVideo _video;
	Common::Point _position = Common::Point(623, 447);
	RunawayWalkPath _walkPath;
	byte _facing = 3, _pendingHotspot = 0;
	byte _pendingItem = 0;
	RunawayVerb _pendingVerb = kVerbWalk;
	Action _action = kNone;
	Action _afterSpeech = kNone, _afterWalk = kNone;
	byte _actionFacing = 0, _pickupHotspot = 0;
	bool _lidOpen = false, _unplugged = false;
	bool _windowClean = false;
	bool _coolerMoved = false, _coolerInspected = false, _escaped = false;
	IceState _ice = kFrozen;
	HatchState _hatchState = kHidden;
	bool _pickedUp[20] = {};
	bool _examined[20] = {};
	bool _globalSpeech = false;
	uint32 _now = 0, _lastUpdate = 0, _actionStarted = 0;
	uint32 _walkElapsed = 0, _lastFootstep = 0;
	uint _step = 0;
	uint16 _speechRow = 0, _speechEnd = 0;
	uint16 _speechSample = 0;
	RunawayInventory::Reply _nextInventoryReply = RunawayInventory::Reply(0);
	Common::U32String _committedCaption;
};

} // End of namespace Runaway
} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE5030_H
