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
#include "hollywood/runaway/animation.h"
#include "hollywood/runaway/audio.h"
#include "hollywood/runaway/graphics.h"
#include "hollywood/runaway/inventory.h"
#include "hollywood/runaway/navigation.h"
#include "hollywood/runaway/saveload.h"
#include "hollywood/runaway/scene_data.h"
#include "hollywood/runaway/sequence.h"
#include "hollywood/runaway/speech.h"
#include "hollywood/scene_text_store.h"

namespace Hollywood {

class RunawayScene {
public:
	struct Transition {
		uint16 scene = 0; // Preserve entry substates, e.g. 2020..2023, until dispatch.
		Common::Point position;
		byte facing = 3;
		bool door = false;
	};
	virtual ~RunawayScene() {}
	virtual bool load(uint16 entryScene) = 0;
	virtual void start(uint32 now) = 0;
	virtual void resume() = 0;
	const Transition &transition() const { return _transition; }
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
	virtual const Common::Array<Common::U32String> &dialogueChoices() const { return _noChoices; }
	virtual void chooseDialogue(uint choice) {}
	virtual bool playingVideo() const { return false; }
	virtual bool inventoryPresentation() const { return false; }
	virtual void skipVideo() {}
	virtual void sayInventory(const RunawayInventory::Reply &reply) = 0;
	virtual bool escaped() const { return false; }
	virtual bool canSave() const { return false; }
	virtual void captureState(RunawaySaveState &state) const {}
	// Prepare a replacement scene without changing shared inventory or audio.
	virtual bool restoreState(const RunawaySaveState &state, uint32 now, bool entering = false) { return false; }
	virtual void setOptions(bool antialias, bool fastScrolling) = 0;

protected:
	Common::Array<Common::U32String> _noChoices;
	uint16 _entryScene = 0;
	Transition _transition;
};

class RunawayStrings;
struct RunawayHotspot;

// Shared walking, rendering and interaction for the numbered rooms.
class RunawayRoomScene : public RunawayScene {
public:
	RunawayRoomScene(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game);
	bool load(uint16 entryScene) override;
	void start(uint32 now) override;
	void resume() override;
	void click(Common::Point mouse, RunawayVerb verb, byte item) override;
	void update(uint32 now, int scrollDirection, bool inventoryOpen) override;
	void draw() override;
	void setCameraX(int x) override;
	int roomWidth() const override { return _background.w; }
	Graphics::ManagedSurface &screen() override { return _screen; }
	byte hotspotAt(Common::Point mouse) const override;
	Common::U32String actionCaption(byte hotspot, RunawayVerb verb, byte item) const override;
	const Common::U32String &committedCaption() const override { return _caption; }
	const Common::U32String &subtitle() const override { return _sequence.active() ? _sequence.subtitle() : _speech.subtitle(); }
	bool busy() const override { return _sequence.active() || _pendingHotspot != 0 || _transition.scene || _hospitalExit.scene || _arrivalFacing != 0xff || _hospitalEntryStep || _roomAction || _conversation || speaking(); }
	bool speaking() const override { return !subtitle().empty(); }
	byte speechFrame() const override { return _sequence.active() ? _sequence.speechFrame() : _speech.frame(_now); }
	bool playingVideo() const override { return _sequence.active(); }
	void skipVideo() override { _sequence.skip(); }
	bool escaped() const override { return _chapterComplete; }
	void skipSpeech() override;
	void sayInventory(const RunawayInventory::Reply &reply) override;
	const Common::Array<Common::U32String> &dialogueChoices() const override { return _choices; }
	void chooseDialogue(uint choice) override;
	bool canSave() const override { return !busy() && !_walkPath.active() && !_pressedKey; }
	void captureState(RunawaySaveState &state) const override;
	bool restoreState(const RunawaySaveState &state, uint32 now, bool entering = false) override;
	void setOptions(bool antialias, bool fastScrolling) override { _antialias = antialias; _fastScrolling = fastScrolling; }

private:
	enum ObjectState { kUnexamined, kIdentified, kTaken };
	enum ActionResult { kUnhandled, kHandled, kBackgroundChanged };

	bool enterRoom(uint16 room, Common::Point position, byte facing);
	bool loadForeground(uint layer, uint chunk, bool pixels = false);
	void drawForeground(Graphics::ManagedSurface &viewport, uint layer);
	bool rebuildBackground();
	bool applyPatch(uint chunk);
	void updateMaps();
	void walkTo(Common::Point target, byte facing = 0xff, bool scripted = false);
	void arrive();
	void execute(uint16 action);
	void useItem(byte item, byte hotspot);
	void say(uint16 cue, byte variation = 0, bool global = false, byte speaker = 0);
	void showLine();
	void changeFloor(bool upper, Common::Point position);
	void updateCamera();
	void leaveHospitalRoom(uint16 scene, Common::Point position, byte facing);
	void updateHospitalAudio();
	enum HospitalFilm { kGinaStoryFilm, kLedgeFilm, kDecoyFilm, kAmbushFilm, kEscapeFilm };
	void startHospitalFilm(HospitalFilm film);
	void finishHospitalFilm();
	void adaptGogFilm(const RunawaySequence::Command *commands);
	void beginRoomAnimation(uint16 action, uint chunk, uint frames, const byte *order = nullptr, uint count = 0, bool hideActor = true);
	void beginRoomAnimationRange(uint16 action, uint chunk, uint frames, uint first, uint last, bool hideActor = true);
	void holdRoomAnimation(uint16 action, uint chunk, uint frames, uint frame, bool hideActor = true);
	void setRoomAnimationOverlay(uint chunk, uint frames, const byte *order, uint count);
	bool drawingRoomAnimation() const { return _roomAction && !_actionFrames.empty() && (_actionHoldSpeech || !speaking()) && !_walkPath.active(); }
	void updateRoomAnimation();
	void finishHospitalAnimation(uint16 action);
	float actorHeight() const;
	const RunawayHotspot *hotspot(byte id) const;
	Common::U32String hotspotName(byte id) const;

	byte walkZone() const { return _originalMaps[_mask[_position.y * _background.w + _position.x]]; }

	bool loadScene2020();
	bool applyBackground2020();
	void start2020();
	void update2020();
	void updateMaps2020();
	bool sayHospitalInventory(const RunawayInventory::Reply &reply);
	ActionResult execute2020(uint16 action);
	float actorHeight2020() const;
	void drawScene2020(Graphics::ManagedSurface &viewport, bool foreground);
	bool applyBackground2060();
	void updateMaps2060();
	ActionResult execute2060(uint16 action);
	bool loadScene2080();
	bool applyBackground2080();
	void updateMaps2080();
	void update2080();
	ActionResult execute2080(uint16 action);
	void drawScene2080(Graphics::ManagedSurface &viewport, bool foreground);

	bool museumChapter() const { return _game == kRunawayFullGame && _room / 1000 == 4; }
	void startMuseum();
	void resumeMuseum();
	void updateMuseum(uint elapsed, bool inventoryOpen);
	void finishMuseumAnimation(uint16 action);
	void museumAnimationFrame();
	void finish4050(uint16 action);
	void finish4060(uint16 action);
	void finish4070(uint16 action);
	void finish4100(uint16 action);
	bool sayMuseumInventory(const RunawayInventory::Reply &reply);
	void deferMuseumAction(uint16 action);
	void refreshMuseum();
	void leaveMuseumRoom(uint16 scene, Common::Point position, byte facing = 3, bool door = false);
	enum MuseumFilm { kArrivalFilm, kCliveFilm, kDrillingFilm, kLaserFilm, kRubyFilm,
		kMobstersFilm, kPhoneFilm, kScannerFilm, kBetrayalFilm, kMuseumExitFilm };
	void startMuseumFilm(MuseumFilm film);
	void finishMuseumFilm();
	void startConversation(byte character);
	void updateConversation();
	void buildDialogueChoices();
	void finishConversation();
	byte speaker() const { return speaking() && _lineIndex < _lines.size() ? _lines[_lineIndex].speaker : 0; }

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

	bool loadScene4070();
	void start4070();
	void drawScene4070(Graphics::ManagedSurface &viewport);
	ActionResult execute4090(uint16 action);
	bool applyBackground4090();
	float actorHeight4090() const;
	void setupDepth4090(RunawayActor::RenderParams &params) const;

	bool loadScene4100();
	bool applyBackground4100();
	void updateMaps4100();
	ActionResult execute4100(uint16 action);
	void drawScene4100(Graphics::ManagedSurface &viewport);
	void closeMuseumChamber();
	float actorHeight4100() const;
	void setupDepth4100(RunawayActor::RenderParams &params) const;

	void start4140();
	bool loadScene4140();
	void clickScene4140(byte id);
	void drawScene4140(Graphics::ManagedSurface &viewport);

	bool desertChapter() const { return _game == kRunawayFullGame && _room / 1000 == 5; }
	void startDesert();
	void resumeDesert();
	void updateDesert(uint elapsed, bool inventoryOpen);
	bool loadDesertRoom();
	bool applyDesertBackground();
	void updateDesertMaps();
	void drawDesertRoom(Graphics::ManagedSurface &viewport, bool foreground);
	void drawScene5060(Graphics::ManagedSurface &viewport);
	void drawScene5080(Graphics::ManagedSurface &viewport);
	void updateScene5080();
	void drawScene5100(Graphics::ManagedSurface &viewport, bool foreground);
	void updateScene5100();
	bool applyBackground5100();
	void updateMaps5100();
	void walkHangar(Common::Point target, byte facing);
	void setupDesertDepth(RunawayActor::RenderParams &params) const;
	float desertActorHeight() const;
	Common::U32String desertHotspotName(byte id) const;
	void leaveDesertRoom(uint16 scene);
	void deferDesertAction(uint16 action);
	void finishDesertAnimation(uint16 action);
	void desertAnimationFrame();
	ActionResult executeDesert(uint16 action);
	ActionResult execute5020(uint16 action);
	ActionResult execute5050(uint16 action);
	ActionResult execute5060(uint16 action);
	ActionResult execute5070(uint16 action);
	ActionResult execute5080(uint16 action);
	ActionResult execute5100(uint16 action);
	ActionResult execute5110(uint16 action);
	ActionResult execute5160(uint16 action);
	void startCarlaConversation();
	ActionResult execute5170(uint16 action);

	RunawayAudio &_audio;
	RunawaySequence _sequence;
	RunawayInventory &_inventory;
	const RunawayStrings &_strings;
	const RunawayGameType _game;
	Common::RandomSource _random;
	RunawaySpeech _speech;
	RunawayIdleHead _idleHead;
	ChunkArchive _archive;
	SceneTextStore _text;
	RunawayActor _actor;
	RunawayAnimation _ambient, _overlay, _safe;
	RunawayAnimation _actionAnimation, _secondaryAnimation;
	Common::Array<byte> _actionFrames, _secondaryFrames;
	Graphics::ManagedSurface _background, _screen;
	Common::Array<byte> _mask, _originalMaps, _maps;
	Common::Array<Common::Rect> _foreground[5];
	Common::Array<byte> _keyPatches[25];
	RunawayWalkPath _walkPath;
	struct SpeechLine {
		Common::U32String text;
		uint16 sample;
		bool global;
		byte speaker;
	};
	Common::Array<SpeechLine> _lines;
	Common::U32String _caption;
	Common::Point _position, _camera;
	uint _lineIndex = 0;
	uint16 _room = 0, _pendingAction = 0;
	byte _facing = 3, _pendingHotspot = 0, _pendingItem = 0;
	byte _arrivalFacing = 0xff;
	uint32 _now = 0, _lastUpdate = 0, _walkElapsed = 0;
	bool _upperFloor = true, _antialias = true, _fastScrolling = true;
	ObjectState _books = kUnexamined, _recorder = kUnexamined;
	bool _moneyIdentified = false, _safeOpen = false;
	bool _varnishTaken = false, _talcTaken = false, _coffeeTaken = false;
	byte _keyCount = 0, _pressedKey = 0, _keyResult = 0;
	uint32 _keyUntil = 0;
	uint32 _lastFootstep = 0;
	RunawaySaveState::Hospital _hospital;
	RunawaySaveState::Museum _museum;
	RunawaySaveState::Desert _desert;
	Common::Array<RunawayHotspot> _desertHotspots;
	RunawayAnimation _desertAnimations[6];
	RunawayAnimation _npc, _npcMouth, _machine;
	RunawayAnimation _computer, _rejectionLight;
	uint32 _nextLaser = 0;
	byte _willyPose = 54, _computerFrame = 48;
	bool _cliveVisible = false, _computerReject = false;
	uint16 _machinePosition[2] = {0, 0};
	MuseumFilm _museumFilm = kArrivalFilm;
	SceneTextStore _filmText;
	Common::Array<RunawaySequence::Command> _filmCommands;
	struct DialogueChoice {
		byte enabled, node, transition, player, response, disable, selectable;
	};
	DialogueChoice _dialogue[21][8] = {};
	Common::Array<Common::U32String> _choices;
	Common::Array<byte> _choiceIndices;
	byte _conversation = 0, _dialogueDepth = 0, _dialogueNode = 0, _dialogueIntro = 0;
	bool _dialogueEnding = false;
	uint16 _afterConversation = 0;
	Common::Array<byte> _monitorPatches[2];
	byte _hospitalEntryStep = 0, _ambientFrame = 0, _musicCue = 11;
	int _ambientDirection = 1;
	uint32 _ambientTick = 0, _nextMusic = 0;
	bool _entering = false;
	uint16 _roomAction = 0;
	bool _actionHidesActor = true;
	bool _actionHoldSpeech = false;
	byte _actionStage = 0;
	uint _actionStep = 0;
	uint32 _actionStarted = 0;
	Transition _hospitalExit;
	HospitalFilm _hospitalFilm = kGinaStoryFilm;
	bool _chapterComplete = false;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE_H
