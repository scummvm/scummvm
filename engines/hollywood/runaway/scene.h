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

#include "hollywood/hotspots.h"
#include "hollywood/resource.h"
#include "hollywood/runaway/animation.h"
#include "hollywood/runaway/audio.h"
#include "hollywood/runaway/graphics.h"
#include "hollywood/runaway/inventory.h"
#include "hollywood/runaway/navigation.h"
#include "hollywood/runaway/presentation.h"
#include "hollywood/runaway/saveload.h"
#include "hollywood/runaway/scene_data.h"
#include "hollywood/runaway/sequence.h"
#include "hollywood/runaway/speech.h"
#include "hollywood/scene_text_store.h"

namespace Hollywood {

class RunawayScene : public RunawayPresentation, public HotspotSource {
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
	virtual int cameraX() const { return 0; }
	virtual int roomWidth() const = 0;
	virtual Graphics::ManagedSurface &screen() = 0;
	byte hotspotAt(Common::Point mouse) const override = 0;
	virtual bool hasVerbControls() const { return true; }
	// Cursor tile 3 is talk, 4..7 are exit arrows; zero keeps the usual verbs.
	virtual byte actionCursor(byte hotspot) const { return 0; }
	virtual bool preferLook(byte hotspot) const { return false; }
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
	Common::U32String hotspotName(byte id) const override { return Common::U32String(); }
	void skipPresentation(uint32 now) override final { skipVideo(); }
	void skipPresentationSpeech(uint32 now) override final { skipSpeech(); }

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
	int cameraX() const override { return _camera.x; }
	int roomWidth() const override { return _background.w; }
	Graphics::ManagedSurface &screen() override { return _screen; }
	byte hotspotAt(Common::Point mouse) const override;
	byte actionCursor(byte id) const override;
	bool preferLook(byte id) const override;
	Common::U32String actionCaption(byte hotspot, RunawayVerb verb, byte item) const override;
	const Common::U32String &committedCaption() const override { return _caption; }
	const Common::U32String &subtitle() const override;
	bool busy() const override { return _sequence.active() || _pendingHotspot != 0 || _transition.scene || _hospitalExit.scene || _arrivalFacing != 0xff || _roomAction || _conversation || speaking(); }
	bool speaking() const override { return _sequence.active() ? !_sequence.subtitle().empty() : !_speech.subtitle().empty(); }
	byte speechFrame() const override { return _sequence.active() ? _sequence.speechFrame() : _speech.frame(_now); }
	bool playingVideo() const override { return _sequence.active(); }
	void skipVideo() override { _sequence.skip(); }
	bool escaped() const override { return _chapterComplete; }
	void skipSpeech() override;
	void sayInventory(const RunawayInventory::Reply &reply) override;
	const Common::Array<Common::U32String> &dialogueChoices() const override { return _choices; }
	void chooseDialogue(uint choice) override;
	// The hangar contest's animation progress is transient, like a room action.
	bool canSave() const override { return !busy() && !_walkPath.active() && _desert.guard != 1; }
	void captureState(RunawaySaveState &state) const override;
	bool restoreState(const RunawaySaveState &state, uint32 now, bool entering = false) override;
	void setOptions(bool antialias, bool fastScrolling) override { _antialias = antialias; _fastScrolling = fastScrolling; }

protected:
	enum ObjectState { kUnexamined, kIdentified, kTaken };
	enum ActionResult { kUnhandled, kHandled, kBackgroundChanged };
	struct AnimationFootstep {
		constexpr AnimationFootstep(byte frameNumber, int16 position, byte loudness = 255) :
			frame(frameNumber), x(position), volume(loudness) {}
		byte frame;
		int16 x;
		byte volume;
	};

	virtual bool loadCustomResources() { return true; }
	virtual bool loadBackground();
	virtual bool applyCustomBackgroundPatches() { return true; }
	virtual void applyCustomHotspotState() {}
	virtual void prepareCustomGameplayLoop() {}
	virtual void runCustomEntrySequence() {}
	virtual void advanceCustomGameplayLoop(uint32 delta) {}
	virtual void drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {}
	virtual void drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) {}
	virtual ActionResult dispatchCustomSceneAction(uint16 action) { return kUnhandled; }
	virtual byte defaultItemAction(byte item, byte hotspot) const { return 1; }
	// Return true when the scene handles the reply after Brian's dialogue choice.
	virtual bool handleCustomDialogueReply() { return false; }
	virtual void handleCustomDialogueChoice(byte effect) {}
	virtual void runCustomDialogueIntro() {}
	virtual void finishCustomConversation() {}
	virtual void resumeAmbientAudio(bool entering);
	virtual void stopAmbientSpeech() {}
	virtual void runExitSideEffectsAfterLoop() {}
	virtual void updateActor(uint elapsed, int scrollDirection, bool inventoryOpen);
	virtual void findWalkPath(Common::Point target);
	virtual int sceneViewportYOffset() const;

	virtual uint sceneResourceNumber() const { return (_room % 1000) / 10; }
	virtual uint sceneTextNumber() const { return finaleChapter() ? (_room - 2000) / 10 : _room / 10; }
	virtual bool loadWalkMap();
	virtual bool usesAlternateActor() const { return true; }
	virtual byte initialActorFacing() const { return 3; }
	virtual void setupActorDepth(RunawayActor::RenderParams &params) const {}
	virtual bool drawCustomComposite(Graphics::ManagedSurface &viewport) { return false; }
	virtual bool shouldDrawActor() const { return true; }
	virtual bool roomAnimationInForeground() const { return false; }
	virtual bool shouldUpdateActor() const { return true; }
	virtual bool shouldPlayGameplayClickPath() const { return true; }
	virtual int8 customizeSceneActionApproach(byte id, bool use, byte item, int8 approach) const { return approach; }
	// Accept the adjusted target without the shared floor-mask adjustment.
	virtual bool adjustCustomWalkTargetToFloorMask(Common::Point &target) const { return false; }
	// Return true when a scene sequence owns this frame's action updates.
	virtual bool updateCustomSequence() { return false; }
	virtual void finishCustomAnimation(uint16 action) {}
	virtual void handleAnimationFrameHook() {}
	virtual void finishCustomFilm() {}
	virtual uint16 dialoguePlayerCue() const { return 80; }
	virtual uint16 dialogueResponseCue() const { return 90; }
	virtual uint16 dialogueCue(byte choice, bool response) const { return response ? dialogueResponseCue() : dialoguePlayerCue(); }

	bool enterRoom(uint16 room, Common::Point position, byte facing);
	bool loadForeground(uint layer, uint chunk, bool pixels = false);
	void drawForeground(Graphics::ManagedSurface &viewport, uint layer);
	void drawRoomAnimation(Graphics::ManagedSurface &viewport);
	void drawAmbientAnimations(Graphics::ManagedSurface &viewport, const RunawayAnimation *animations, uint count);
	bool rebuildBackground();
	bool applyPatch(uint chunk);
	void updateMaps();
	void walkTo(Common::Point target, byte facing = 0xff, bool scripted = false, bool reserved = false);
	void arrive();
	void execute(uint16 action);
	void executeItemAction(uint16 action);
	void useItem(byte item, byte hotspot);
	void say(uint16 cue, byte variation = 0, bool global = false, byte speaker = 0);
	void showLine();
	void changeFloor(bool upper, Common::Point position);
	void updateCamera();
	void beginRoomAnimation(uint16 action, uint chunk, uint frames, const byte *order = nullptr, uint count = 0, bool hideActor = true, uint frameDuration = 80);
	void beginActorAnimation(uint16 action, uint bank, uint frames, uint count = 0, uint frameDuration = 60);
	void beginAnimation(uint16 action, uint frames, const byte *order, uint count, bool hideActor, uint frameDuration);
	void beginRoomAnimationRange(uint16 action, uint chunk, uint frames, uint first, uint last, bool hideActor = true, uint frameDuration = 80);
	void holdRoomAnimation(uint16 action, uint chunk, uint frames, uint frame, bool hideActor = true);
	void loopRoomAnimationDuringSpeech(uint16 action, uint chunk, uint frames, uint first, uint last, bool hideActor = true);
	void setRoomAnimationOverlay(uint chunk, uint frames, const byte *order, uint count);
	bool drawingRoomAnimation() const { return !_actionUsesActor && _roomAction && !_actionFrames.empty() && (_actionHoldSpeech || !speaking()) && !_walkPath.active(); }
	void updateRoomAnimation();
	void playAnimationFootsteps(uint frame, byte surface, const AnimationFootstep *steps, uint count);
	virtual float actorHeight() const { return 455; }
	virtual float actorWalkSpeed() const { return 0.18f * actorHeight() / 455; }
	virtual const RunawayHotspot *hotspot(byte id) const;
	Common::U32String hotspotName(byte id) const override;
	virtual const char *actionCaptionVariant(byte id, bool look, byte item) const { return nullptr; }
	virtual bool isValidHotspot(byte id) const { return hotspot(id) != nullptr; }
	virtual Common::Point customizeSceneActionPosition(byte id, byte item, Common::Point position) const { return position; }
	virtual byte customizeSceneActionFacing(byte id, byte facing) const { return facing; }
	virtual Common::Point sceneActionLookPosition(byte id) const { return hotspot(id)->lookPosition; }

	byte walkZone() const { return _originalMaps[_mask[_position.y * _background.w + _position.x]]; }

	bool hospitalChapter() const { return _game == kRunawayFullGame && _room / 1000 == 2; }
	void resumeHospital();
	void updateHospital(uint elapsed);
	void updateHospitalAudio();
	bool sayHospitalInventory(const RunawayInventory::Reply &reply);
	void leaveHospitalRoom(uint16 scene, Common::Point position, byte facing);

	bool museumChapter() const { return _game == kRunawayFullGame && _room / 1000 == 4; }
	void startMuseum();
	void resumeMuseum();
	void updateMuseum(uint elapsed);
	bool sayMuseumInventory(const RunawayInventory::Reply &reply);
	void deferMuseumAction(uint16 action);
	void refreshMuseum();
	void leaveMuseumRoom(uint16 scene, Common::Point position, byte facing = 3, bool door = false);
	enum MuseumFilm { kArrivalFilm, kCliveFilm, kDrillingFilm, kLaserFilm, kRubyFilm,
		kMobstersFilm, kPhoneFilm, kScannerFilm, kBetrayalFilm, kMuseumExitFilm };
	void startMuseumFilm(MuseumFilm film);
	void startConversation(byte character);
	void updateConversation();
	void buildDialogueChoices();
	void finishConversation();
	byte speaker() const { return speaking() && _lineIndex < _lines.size() ? _lines[_lineIndex].speaker : 0; }

	bool desertChapter() const { return _game == kRunawayFullGame && _room / 1000 == 5; }
	void startDesert();
	void resumeDesert();
	void updateDesert(uint elapsed);
	void visitDesertRoom(byte index, byte lines);
	void leaveDesertRoom(uint16 scene);
	void deferDesertAction(uint16 action);

	bool townChapter() const { return _game == kRunawayFullGame && _room / 1000 == 6; }
	void resumeTown();
	bool sayTownInventory(const RunawayInventory::Reply &reply);
	void leaveTownRoom(uint16 scene);
	void deferTownAction(uint16 action);
	void walkTown(Common::Point target);

	bool hopiChapter() const { return _game == kRunawayFullGame && _room / 1000 == 7; }
	bool sayHopiInventory(const RunawayInventory::Reply &reply);
	bool finaleChapter() const { return _game == kRunawayFullGame && _room / 1000 == 8; }
	bool sayFinaleInventory(const RunawayInventory::Reply &reply);
	void resumeFinaleOutdoorAudio(bool entering, uint16 windCue, byte windVolume);
	void updateFinaleOutdoorAudio(byte wildlifeVolume = 0);
	void resumeFinaleJailAudio(bool entering);
	void updateFinaleJailAudio();
	void updateOutdoorMusic(byte cueCount = 3);
	void updateGameplay(uint32 delta);
	void findGameplayWalkPath(Common::Point target);
	void deferAction(uint16 action);
	void leaveRoom(uint16 scene);

	RunawayAudio &_audio;
	RunawaySequence _sequence;
	bool _inventorySequence = false;
	RunawayInventory &_inventory;
	const RunawayStrings &_strings;
	const RunawayGameType _game;
	Common::RandomSource _random;
	RunawaySpeech _speech;
	RunawayIdleHead _idleHead;
	ChunkArchive _archive;
	SceneTextStore _text;
	RunawayActor _actor;
	RunawayAnimation _actionAnimation, _secondaryAnimation;
	Common::Array<byte> _actionFrames, _secondaryFrames;
	Graphics::ManagedSurface _background, _screen;
	Common::Array<byte> _mask, _originalMaps, _maps;
	Common::Array<Common::Rect> _foreground[5];
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
	const RunawayRoom *_roomData = nullptr;
	uint16 _room = 0, _pendingAction = 0;
	byte _facing = 3, _pendingHotspot = 0, _pendingItem = 0;
	byte _arrivalFacing = 0xff;
	uint32 _now = 0, _lastUpdate = 0, _walkElapsed = 0;
	bool _upperFloor = true, _antialias = true, _fastScrolling = true;
	bool _inventoryOpen = false;
	ObjectState _books = kUnexamined, _recorder = kUnexamined;
	bool _moneyIdentified = false, _safeOpen = false;
	bool _varnishTaken = false, _talcTaken = false, _coffeeTaken = false;
	byte _keyCount = 0;
	uint32 _lastFootstep = 0;
	RunawaySaveState::Hospital _hospital;
	RunawaySaveState::Museum _museum;
	RunawaySaveState::Desert _desert;
	RunawaySaveState::Town _town;
	RunawaySaveState::Hopi _hopi;
	RunawaySaveState::Finale _finale;
	MuseumFilm _museumFilm = kArrivalFilm;
	SceneTextStore _filmText;
	Common::Array<RunawaySequence::Command> _filmCommands;
	struct DialogueChoice {
		byte enabled, node, transition, player, response, disable, selectable;
	};
	DialogueChoice _dialogue[22][8] = {};
	Common::Array<Common::U32String> _choices;
	Common::Array<byte> _choiceIndices;
	byte _conversation = 0, _dialogueDepth = 0, _dialogueNode = 0, _dialogueIntro = 0;
	bool _dialogueEnding = false;
	uint16 _afterConversation = 0;
	byte _musicCue = 11;
	uint32 _nextMusic = 0;
	uint32 _finaleAmbientTick = 0;
	byte _finaleLastAmbient = 0;
	bool _entering = false;
	uint16 _roomAction = 0;
	bool _actionHidesActor = true;
	bool _actionUsesActor = false;
	bool _actionHoldSpeech = false;
	bool _actionLoopSpeech = false;
	byte _actionStage = 0;
	uint _actionStep = 0;
	uint _actionFrameDuration = 80;
	uint32 _actionStarted = 0;
	Transition _hospitalExit;
	bool _chapterComplete = false;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE_H
