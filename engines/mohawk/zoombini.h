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

#ifndef MOHAWK_ZOOMBINI_H
#define MOHAWK_ZOOMBINI_H

#include "common/queue.h"
#include "common/stack.h"

#include "mohawk/mohawk.h"
#include "mohawk/resource.h"

#include "mohawk/zoombini_debug.h"
#include "mohawk/zoombini_resource.h"
#include "mohawk/zoombini_scripts.h"
#include "mohawk/zoombini_state.h"

namespace Common {

struct Event;
struct KeyState;
class Keymap;

} // End of namespace Common

namespace Mohawk {

class ZoombiniGraphics;
class ZoombiniSound;
class ZoombiniMidiPlayer;
class ZoombiniRandom;
class ZoombiniText;
class ZoombiniGameState;
class ZoombiniPage;
class ZoombiniDialog;
class VideoManager;
class MidiPlayer;
class ZmbShapeOffsetRegs;

/** Runtime-only state used by the original engine's shared debug dispatcher. */
struct ZmbBuiltinDebugRuntimeState {
	bool _stepMode = false;
	bool _stepAdvanceRequested = false;
	int16 _stepRunnerPosition = 0;
	uint16 _pathCycleIndex = 0;
	bool _soundQueueNotifications = false;
	bool _runnerFilterSnoids = false;
	bool _runnerShowIds = false;
	bool _runnerBackdropMode = false;
	bool _showDragCoordinates = false;
};

/**
 * Top-level Zoombini engine coordinator.
 *
 * The engine owns services shared by every page: archive lookup, graphics,
 * sound, MIDI, text, game state, and the random source. It owns the active page
 * and the page queue, while modal dialogs are held in a separate stack and take
 * input and frame updates while they are open.
 *
 * @ref loadNextPage() constructs the concrete page selected by the queue, then
 * runs its setup lifecycle in order: open, background music, background bitmap,
 * feature loading, and fade-in. @ref doFrame() advances background sound queues,
 * dispatches events to the current page or dialog, runs the page frame, flushes
 * the composed screen, and performs the pending page or dialog transition after
 * the fade completes.
 *
 * Pages own page-local features and archives. The engine remains responsible for
 * replacing the active page, clearing page image caches when archive namespaces
 * change, and keeping system resources available across page transitions.
 */
class MohawkEngine_Zoombini : public MohawkEngine {
public:
	/**
	 * Debug channels registered and owned by the Zoombini engine.
	 * Warnings report player-data recovery and non-fatal failures without debug configuration.
	 * Level 1 records infrequent engine, I/O, and persistent-state changes.
	 * Level 2 records major page and puzzle-state transitions.
	 * Level 3 records generated rules and resource selections, while level 4 records individual puzzle actions and callbacks.
	 * Level 5 is reserved for throttled high-frequency traces.
	 */
	enum ZmbDebugFlags {
		kDebugSaveLoad01 = 1,
		kDebugPage02,
		kDebugState03,
	};

	/** Construct the top-level Zoombini engine and defer resource setup to @ref run(). */
	MohawkEngine_Zoombini(OSystem *syst, const MohawkGameDescription *gamedesc);
	/** Release the active page, dialogs, shared services, and loaded archives. */
	~MohawkEngine_Zoombini() override;

	/** Shared random-number source used by pages and engine-wide animation logic. */
	ZoombiniRandom *_rnd = nullptr;
	/** Video decoder and movie service used by the Zoombini presentation pages. */
	VideoManager *_video = nullptr;
	/** Shared sound-effect queue and resource playback service. */
	ZoombiniSound *_sound = nullptr;
	/** MIDI playback service for page music and legacy MIDI resources. */
	ZoombiniMidiPlayer *_midi = nullptr;
	/** Shared indexed-palette graphics and renderer service. */
	ZoombiniGraphics *_gfx = nullptr;
	/** Persistent game state shared by pages, dialogs, and save/load code. */
	ZoombiniGameState *_state = nullptr;
	/** Localized text and font-resource service. */
	ZoombiniText *_text = nullptr;
	/** System archive containing shared game resources. */
	MohawkArchive *_sysMhk = nullptr;
	/** Optional help archive containing on-screen help resources. */
	MohawkArchive *_helpMhk = nullptr;
	/** Shared v1 archive containing the tMID resources used by music and the built-in MIDI test. */
	MohawkArchive *_midiMhk = nullptr;
	/** Whether the currently opened shared MIDI archive uses the Macintosh profile. */
	bool _midiMhkUsesMacProfile = false;

	/**
	 * Registration-point offsets for system snoid shapes (tBMP 3000 in ZOOMBINI.MHK).
	 * Loaded once at startup from REGS 100+101 in ZOOMBINI.MHK.
	 * @ref ZoombiniPage::blitShapes() uses them to anchor each body-part sprite to the Snoid's base position.
	 */
	ZmbShapeOffsetRegs *_snoidShapeOffsetRegs = nullptr;

	/**
	 * Registration-point offsets for small snoid shapes (tBMP 3200 / 0xC80 in ZOOMBINI.MHK).
	 * Loaded once at startup from REGS 3200+3201 in ZOOMBINI.MHK.
	 * Used for the XFER FromIsle scene which uses small-scale shapes.
	 */
	ZmbShapeOffsetRegs *_smallSnoidShapeOffsetRegs = nullptr;

	/**
	 * Registration-point offsets for SCRS-script-rendered snoid shapes
	 * (tBMP 3100 / 0xC1C in ZOOMBINI.MHK).
	 * Loaded once at startup from REGS 102+103 in ZOOMBINI.MHK.
	 * Used by Snoids in @ref kSnoidAnimState009_ScriptNormal.
	 * This table pairs with shape archive tBMP 3100 instead of the tBMP 3000 idle and layered-animation pool.
	 * Ferry reject-flight SCRS 1900-1906 therefore draw body parts from the correct sprite pool.
	 */
	ZmbShapeOffsetRegs *_snoidScriptShapeRegs = nullptr;

	/** Nominal animation tick rate used by page state machines. */
	static constexpr uint32 kAnimateFrameRate = 60;
	/** Millisecond base used to convert elapsed time into animation ticks. */
	static constexpr uint32 kAnimationClockTimeBaseMs = 1000;
	/**
	 * The original engine uses a 17 ms integer tick to avoid floating-point math on 486SX systems.
	 * ScummVM defaults to the intended exact 60 FPS interval and exposes the integer interval as an option.
	 */
	static constexpr uint32 kOriginalAnimateFrameTimeMs = 17;
	/**
	 * Maximum presentation-loop rate, currently matched to the authored animation clock.
	 * A future high-refresh presentation mode may separate this from @ref kAnimateFrameRate.
	 */
	static constexpr uint32 kTargetFrameRate = kAnimateFrameRate;
	/** Duration of one renderer update at @ref kTargetFrameRate. */
	static constexpr double kTargetFrameTimeMs = 1000.0 / kTargetFrameRate;
	/**
	 * Double-click time threshold in frame count.
	 * Default double-click time is 500 ms, following Windows default.
	 * - https://learn.microsoft.com/en-us/windows/win32/controls/ttm-setdelaytime
	 * - https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getdoubleclicktime
	 *
	 * In Zoombinis, double-click support is not mandatory.
	 * However, it is used in the LoadDialog to quickly select a save slot.
	 *
	 * TODO: Make it configurable via ScummVM config?
	 */
	/** Maximum time between clicks treated as a double-click. */
	static constexpr uint32 kDoubleClickFrameTimeMs = 500;
	/** Animation ticks corresponding to @ref kDoubleClickFrameTimeMs. */
	static constexpr uint32 kDoubleClickFrameRate = kAnimateFrameRate / (1000.0 / kDoubleClickFrameTimeMs);
	/** Period of the text-entry caret blink. */
	static constexpr uint32 kTextCursorBlinkFrameTimeMs = 500;
	/** Animation ticks corresponding to @ref kTextCursorBlinkFrameTimeMs. */
	static constexpr uint32 kTextCursorBlinkFrameRate = kAnimateFrameRate / (1000.0 / kTextCursorBlinkFrameTimeMs);
	/** Animation ticks between authored page-loading cursor poses. */
	static constexpr uint32 kMouseCursorEyeFrameTicks = 12;
	/** v2.x animation-clock delay before restoring the default cursor after a page load. */
	static constexpr uint32 kTlcV2BusyCursorStopDelay = 3;

	/** Advance shared services, dispatch input, render the active page, and commit transitions. */
	void doFrame();
	/** Delay for a bounded interval while continuing normal engine frame processing. */
	void delayRunningFrames(uint32 ms);

	/** Open and return the system MHK archive selected by the detected release. */
	MohawkArchive *loadSystemArchive();
	/** Open and return the optional help MHK archive selected by the detected release. */
	MohawkArchive *loadHelpArchive();
	/** Lazily open and return the shared v1 MIDI archive selected by the music profile option. */
	MohawkArchive *loadMidiArchive();
	/** Add detected Zoombini resource roots to the search path in priority order. */
	void initSearchPaths();
	/** Load next interactive/transition page from the page queue. */
	void loadNextPage();

	/** Add a page archive to the current page-resource lookup chain. */
	void addPageArchive(Archive *archive);
	/** Remove one page archive from the current page-resource lookup chain. */
	void removePageArchive(Archive *archive);
	/** Remove all page archives before loading a different page namespace. */
	void clearPageArchives();

	/** Return the page currently owned by the engine, if any. */
	ZoombiniPage *getActivePage() const { return _activePage; }
	/** Return the page receiving gameplay events after accounting for dialogs. */
	ZoombiniPage *getCurrentPage() const;
	/** Queue a page transition and unwind every modal dialog before @ref doFrame() completes it. */
	void setNextPage(ZoombiniPageType type);

	/**
	 * Source SI page set by each puzzle or area before transitioning to @ref ZoombiniPageType::kXfer.
	 * Used by @ref ZoombiniTransitionXfer to determine which route to display.
	 */
	ZmbSrcPageKind _xferSrcPage = ZmbSrcPageKind::kMinus1;

	/**
	 * Debug-only go xfer handoff. Preserve the current active pack when the
	 * transition closes, including in practice mode.
	 */
	bool _debugPreserveActivePackOnXferClose = false;
	/** Shared runtime state for hardcoded and console-invoked built-in debug actions. */
	ZmbBuiltinDebugRuntimeState _builtinDebug;

	/**
	 * Level 1 Bridge split-count exclusion consumed by Tunnels.
	 *
	 * An active exclusion carries the number of Zoombinis that match Bridge's
	 * selected rule. Tunnels Level 1 may exclude candidates with that same
	 * count when another nonzero split is available.
	 */
	struct BridgeTunnelsLevel1Exclusion {
		/** Return whether a Bridge Level 1 split count is available to Tunnels. */
		bool isActive() const { return _active; }
		/** Return the active Bridge Level 1 match count. */
		int16 getSplitCount() const { return _splitCount; }
		/** Clear the Bridge Level 1 handoff. */
		void clear() {
			_active = false;
			_splitCount = 0;
		}
		/** Record one Bridge Level 1 match count for the following Tunnels rule. */
		void setSplitCount(int16 splitCount) {
			_active = true;
			_splitCount = splitCount;
		}

	private:
		bool _active = false;
		int16 _splitCount = 0;
	};

	/** Bridge Level 1 handoff used while selecting the following Tunnels rule. */
	BridgeTunnelsLevel1Exclusion _bridgeTunnelsLevel1Exclusion;

	/**
	 * Caves uses a process-lifetime one-shot when choosing its first primary rule trait.
	 * This path consumes the normal random value, overrides it with trait index 2,
	 * and then clears this flag permanently.
	 */
	bool _cavesFirstRuleTraitPending = true;

	/**
	 * Global fidget interval threshold.
	 * Snoids trigger fidget only when their per-snoid idle counter exceeds this value.
	 * Default 64. Set to 0 to disable fidgets, including during XFER.
	 * Halved (min 1) when the game has been idle for 3600 ticks.
	 * Reset to 64 on user activity and authored presentation boundaries via @ref MohawkEngine_Zoombini::resetFidgetActivity().
	 */
	uint16 _fidgetThreshold = 64;

	/**
	 * Frame counter at the last user activity or authored presentation boundary.
	 * Actor inactivity checks keep measuring from this boundary when the fidget threshold is halved.
	 */
	uint32 _lastActivityFrame = 0;

	/** Frame counter at the last activity boundary or fidget-threshold halving. */
	uint32 _lastFidgetIntervalFrame = 0;

	/**
	 * Fidget voice cadence state retained modulo 32.
	 * Incremented only when an enabled Snoid fidget selects a voice.
	 * The archive layer loads the selected SND resource on demand; this counter
	 * does not perform an explicit preload.
	 */
	uint16 _fidgetVoiceCadenceCounter = 0;

	/**
	 * Global post-arrival turn-around state.
	 * @ref MohawkEngine_Zoombini::setArrivalTurnDirection() maps movement turnDir -1, 0,
	 * or 1 to the corresponding @ref SnoidAnimState.
	 * Arrival and path completion enter this state instead of idle.
	 * @ref kSnoidAnimState001_TurnLeft and @ref kSnoidAnimState002_TurnRight briefly flip the facing turnDir
	 * before settling to idle.
	 */
	SnoidAnimState _arrivalTurnState = kSnoidAnimState000_Idle;

	/**
	 * Global counter of snoids currently walking in.
	 * Incremented when @ref kSnoidAnimState010_ArrivalMotion fires,
	 * decremented when a snoid completes its path (state 112 arrival).
	 * Interactive pages use it to prevent concurrent drag during walk-in animations.
	 */
	int16 _walkersInProgress = 0;

	/**
	 * Set @ref MohawkEngine_Zoombini::_arrivalTurnState from a movement turnDir value.
	 * A value of @ref ArrivalTurnDirection::kLeft selects state 1 and settles facing left,
	 * @ref ArrivalTurnDirection::kNone selects @ref kSnoidAnimState000_Idle,
	 * and @ref ArrivalTurnDirection::kRight selects state 2 and settles facing right.
	 */
	void setArrivalTurnDirection(ArrivalTurnDirection turnDir);

	/**
	 * Reset the fidget threshold to 64 and restart the idle timer on user activity.
	 * Activity includes input events, puzzle initialization, and dialog closing.
	 */
	void resetFidgetActivity();
	/** Return the engine-frame count since the last fidget activity reset, or zero while a dialog is open. */
	uint32 getActivityIdleFrames() const;

	/** Return whether any modal dialog is currently open. */
	bool hasDialogOpened() const;
	/** Return whether the active modal dialog is a save/load dialog. */
	bool hasSaveLoadDialogOpened() const;
	/** Return whether opening a save/load dialog is valid in the current state. */
	bool canOpenSaveLoadDialog() const;
	/** Open the engine options dialog. */
	void openOptionsDialog();
	/** Begin the deferred quit sequence, including any required save prompt. */
	void requestQuit();
	/** Defer a game-owned Save or Load dialog until the ScummVM main menu closes. */
	void requestMainMenuSaveLoadDialog(bool loadDialog);
	/** Open the save dialog and return its result. */
	ZoombiniDialogResult openSaveDialog();
	/** Open the load dialog and return its result. */
	ZoombiniDialogResult openLoadDialog(bool newGameMode = false);
	/** Ask whether to save a generated game, then open Load regardless of the Save result. */
	ZoombiniDialogResult openLoadDialogWithSavePrompt();
	/** Open a localized message box selected by its message type. */
	ZoombiniDialogResult openMsgBoxDialog(ZoombiniMsgBoxType type);
	/** Open a message box containing an explicit localized message. */
	ZoombiniDialogResult openMsgBoxDialog(const Common::U32String &message);
	/** Open a confirmation message box containing an explicit message. */
	ZoombiniDialogResult openConfirmMsgBoxDialog(const Common::U32String &message);
	/** Defer a debugger-requested confirmation message box until after the debugger closes. */
	void requestMsgBoxDialog(uint32 textKey);
	/** Open the credits dialog. */
	void openCreditsDialog();
	/** Open help for the specified page type when the release provides it. */
	void openHelpDialog(ZoombiniPageType forPage);
	/** Record Picker's one-shot version-update Help request. */
	void requestPickerUpdateHelp();
	/** Consume the one-shot Picker update Help request. */
	bool consumePickerUpdateHelpRequest();
	/** Return whether the current release provides on-screen help resources. */
	bool supportsOnScreenHelp() const;
	/** Open the debug dialog for a parsed debug command. */
	void openDebugDialog(const ZoombiniDebugCommand &cmd);
	/** Close the topmost modal dialog, if one is open. */
	void closeActiveDialog();

	/** Change acitve resourceKind, and return last active resourceKind. */
	/** Select the archive namespace used by legacy numeric resource lookups. */
	ZmbResource::ArchiveKind setActiveResourceKind(ZmbResource::ArchiveKind kind);
	/**
	 * Temporarily select one exact resource for a legacy ID-only lookup.
	 *
	 * This preserves explicit @ref ZmbResource ownership while base audio APIs
	 * still request only the numeric resource ID.
	 */
	ZmbResource setActiveResource(ZmbResource resource);
	/** Resolve a legacy bare SND ID using the original archive convention. */
	ZmbResource resolveLegacySoundResource(int16 soundId) const;
	/** Open a resource by numeric ID using the active legacy namespace. */
	Common::SeekableReadStream *getResource(uint32 tag, uint16 id) override;
	/** Open a resource by its explicit archive kind and numeric ID. */
	Common::SeekableReadStream *getResource(uint32 tag, ZmbResource res);
	/** Return whether an explicit resource exists in the selected archive namespace. */
	bool hasResource(uint32 tag, ZmbResource res);
	/** Return all IDs for a tag in one archive namespace. */
	Common::Array<int16> getResourceIDList(ZmbResource::ArchiveKind kind, uint32 tag) const;
	/**
	 * Return an immutable system SCRB decode retained for the engine lifetime.
	 * Successful decodes are cached lazily; failed decodes are not cached.
	 */
	const ZmbScriptDecoder::DecodedScrb *getSystemDecodedScrb(int16 resourceId);
	/**
	 * Return an immutable system SCRS decode retained for the engine lifetime.
	 * Successful decodes are cached lazily; failed decodes are not cached.
	 */
	const ZmbScriptDecoder::DecodedScrs *getSystemDecodedScrs(int16 resourceId);
	/** Return the number of archives registered for one namespace. */
	uint getArchiveCount(ZmbResource::ArchiveKind kind) const;
	/** Return an archive by namespace and zero-based archive index. */
	Archive *getArchive(ZmbResource::ArchiveKind kind, uint archiveIdx) const;

	/** Return whether the detected release belongs to the European v1 family. */
	bool isVersionFamilyEuV1() const;
	/** Return whether the detected release belongs to the US v1 family. */
	bool isVersionFamilyUsV1() const;
	/** Return whether the detected release belongs to the TLC v2 family. */
	bool isVersionFamilyTlcV2() const;
	/** Return whether the detected release belongs to either v1 family. */
	bool isVersionFamilyV1() const;
	/** Return whether the detected release stores per-route perfect counters. */
	bool hasRoutePerfectCounterState() const;
	/** Return the compact display name for the detected release family. */
	const char *getVersionFamilyName() const;
	/** Return whether the detected game is the limited demo release. */
	bool isDemo() const;
	/** Consume the one-shot demo startup-logo reveal request. */
	bool consumeDemoStartupLogoReveal();

	/** Return the detected game language. */
	Common::Language getLanguage() const override;
	/** Return the root directory used for Zoombini archive discovery. */
	Common::String getArchiveRoot() const { return _mhkArchiveRoot; }
	/** Return whether a ScummVM engine feature is supported. */
	bool hasFeature(EngineFeature f) const override;
	/** Synchronize mixer and MIDI settings with ScummVM volumes and Zoombini audio gates. */
	void syncSoundSettings() override;
	/** Apply engine settings that affect the active Zoombini services. */
	void applyGameSettings() override;
	/** Build the engine keymaps for the selected target. */
	static Common::Array<Common::Keymap *> initKeymaps(const char *target);
	/** Return whether the selected palette-brightening option is active. */
	bool useBrightenPalette() const { return _brightenPalette; }
	/** Return whether enhanced keyboard shortcuts are enabled. */
	bool useEnhancedKbdShortcuts() const;
	/** Set the serialized built-in debug mode and apply its shared mutation feedback. */
	void setBuiltinDebugMode(bool enabled);
	/** Return whether the serialized built-in debug flag is active. */
	bool isBuiltinDebugMode() const { return _state && _state->getDebugEnabled(); }
	/** Return whether the runtime-only built-in MIDI test is active. */
	bool isBuiltinMidiTestMode() const { return _builtinMidiTestMode; }
	/** Set the page-independent runtime MIDI test mode and show its built-in notification. */
	void setBuiltinMidiTestMode(bool enabled);
	/** Return the currently selected resource ID in the built-in MIDI-test table. */
	uint16 getBuiltinMidiTestResource() const;
	/** Run the current or next built-in MIDI-test selector and show its original notification. */
	uint16 playBuiltinMidiTestResource(bool advance);
	/** Apply the shared Space branch after page and feature key handlers. */
	bool handleBuiltinCheatSpace(const Common::KeyState &kbd);
	/** Show one original long NotiBox message when no modal dialog owns input. */
	void showBuiltinNotiBox(const Common::U32String &text, bool persistent);
	/** Return whether color-blind presentation adjustments are enabled. */
	bool isColorBlindModeEnabled() const { return _colorBlindMode; }
	/** Latch the initial per-level Maze layout selectors once for this engine session. */
	void initializeMazeLayoutVariants();
	/** Convert elapsed milliseconds to the engine's monotonic animation frame. */
	uint32 getAnimationFrameCounter(uint32 timeMs) const;

	/** State of the deferred quit prompt and finalization sequence. */
	enum QuitEventState {
		kQuitEventNone = 0,
		kQuitEventPrompting,
		kQuitEventRunning,
		kQuitEventDone,
	};
	/** Return whether the engine has reached the final quit state. */
	bool mustQuit() const;

protected:
	/** Run startup, the main frame loop, and orderly engine shutdown. */
	Common::Error run() override;

private:
	/** Decode a practice boot parameter into a page type and difficulty level. */
	static bool parsePracticeBootParam(int32 bootParam, ZoombiniPageType &pageType, uint16 &level);
	/** Add one existing filesystem node and its bounded descendants to the search path. */
	static void addSearchDirectoryIfPresent(const Common::FSNode &node, int priority = 0, int depth = 1);
	/** Return whether a direct child directory has the requested name. */
	static bool hasSubDirectoryMatching(const Common::FSNode &root, const Common::String &name);
	/** Return whether the root contains the language directories used by European releases. */
	static bool hasEuropeLanguageDirectories(const Common::FSNode &root);
	/** Detect and register the root of an extracted Zoombini ISO layout. */
	static bool tryAddZoombiniIsoRootSearchPath(const Common::FSNode &root);

	/** Dispatch all queued events to the active page or its modal dialog. */
	void processEvents(ZoombiniPage *page);
	/** Dispatch one event while preserving modal-dialog ownership rules. */
	void processEvent(ZoombiniPage *page, const Common::Event &event);
	/** Update the platform IME composition state exposed to text-entry dialogs. */
	void updateImeCompositionState();
	/** Start the quit transition for a page that has received a quit request. */
	void beginQuitEvent(ZoombiniPage *page);
	/** Advance the pending quit state machine after the current frame. */
	void processPendingQuitRequest();
	/** Process a save/load request deferred by the ScummVM main menu. */
	void processPendingMainMenuSaveLoadDialog();
	/** Return whether quitting should offer a save for the current game state. */
	bool shouldAskSaveBeforeQuit() const;
	/** Open the save-before-quit prompt and record the selected result. */
	void askSaveBeforeQuit();
	/** Run one modal dialog until it returns a result. */
	ZoombiniDialogResult loadModalDialog(ZoombiniDialog *page);

	/** Language selected by the detected game description. */
	Common::Language _language = Common::UNK_LANG;
	/** Root path used while searching for the game's MHK archives. */
	Common::String _mhkArchiveRoot;

	/** Currently active page owned by the engine. */
	ZoombiniPage *_activePage = nullptr;
	/** Archive namespace used by legacy numeric resource calls. */
	ZmbResource::ArchiveKind _activeResourceKind = ZmbResource::kPage;
	/** Exact resource temporarily selected for legacy resource consumers. */
	ZmbResource _activeResource;
	/** Stable engine-lifetime decoded system SCRB resources keyed by their raw 16-bit ID. */
	Common::StableMap<int16, ZmbScriptDecoder::DecodedScrb> _systemDecodedScrbCache;
	/** Stable engine-lifetime decoded system SCRS resources keyed by their raw 16-bit ID. */
	Common::StableMap<int16, ZmbScriptDecoder::DecodedScrs> _systemDecodedScrsCache;
	/** Whether indexed palettes receive the optional brightness adjustment. */
	bool _brightenPalette = true;
	/** Whether the enhanced engine keyboard shortcuts are enabled. */
	bool _enhancedKbdShortcuts = true;
	/** Whether the engine uses the original integer 60 FPS timing interval. */
	bool _useAccurate60FPS = true;
	/** Whether color-blind palette and UI adjustments are enabled. */
	bool _colorBlindMode = false;
	/** Millisecond epoch used to make animation time monotonic across frame calls. */
	uint32 _animationClockEpochTimeMs = 0;
	/** Animation-frame epoch paired with @ref _animationClockEpochTimeMs. */
	uint32 _animationClockEpochFrame = 0;
	/** v2.x animation frame after which the post-load busy cursor returns to default. */
	uint32 _pageLoadingCursorStopFrame = 0;
	/** Whether the demo's one-time startup logo reveal is still pending. */
	bool _demoStartupLogoRevealPending = true;
	/** Whether Picker Help must consume STRL 2900 for the version-update topic. */
	bool _pickerUpdateHelpPending = false;
	/** Whether Ctrl+A armed collection of one built-in cheat phrase. */
	bool _builtinCheatCaptureArmed = false;
	/** Bounded printable phrase collected after Ctrl+A. */
	Common::String _builtinCheatCapture;
	/** v1.x page tMID resource IDs cycled by the built-in MIDI test. */
	static constexpr uint16 kBuiltinMidiTestResourceIds[18]{
		30000,
		30001,
		30020,
		30021,
		30022,
		30023,
		30025,
		30026,
		30027,
		30028,
		30030,
		30031,
		30032,
		30033,
		30035,
		30036,
		30037,
		30038,
	};
	/** TLC v2 Picker-page SND accepted by the built-in MIDI test. */
	static constexpr uint16 kBuiltinTlcMidiTestSoundResourceId = 30001;
	/** Runtime-only MIDI test mode selected by the built-in phrase. */
	bool _builtinMidiTestMode = false;
	/** Zero-based MIDI-test table entry selected by Shift+Space. */
	uint16 _builtinMidiTestResourceIndex = 0;

	/** Page types waiting to be constructed by @ref loadNextPage(). */
	Common::Queue<ZoombiniPageType> _pageQueue;
	/** Modal dialogs waiting above the active page, from oldest to newest. */
	Common::Stack<ZoombiniDialog *> _dialogPageStack;
	/** Result returned by the most recently closed modal dialog. */
	ZoombiniDialogResult _lastDialogResult = ZoombiniDialogResult::kNone;
	/** Events deferred while a page fade is queued or running. */
	Common::Queue<Common::Event> _deferredEventQueue;
	/** Current state of the quit event transition. */
	QuitEventState _quitEventState = kQuitEventNone;
	/** Requests deferred until options and save dialogs have closed. */
	enum PendingQuitRequest {
		kPendingQuitNone = 0,
		kPendingQuitCloseOptions,
		kPendingQuitWaitForOptionsClose,
		kPendingQuitFinalizing,
	};
	/** Pending quit request owned by the engine-level quit state machine. */
	PendingQuitRequest _pendingQuitRequest = kPendingQuitNone;
	/** Requests deferred while returning to the ScummVM main menu. */
	enum PendingMainMenuSaveLoadDialog {
		kPendingMainMenuSaveLoadNone = 0,
		kPendingMainMenuSaveDialog,
		kPendingMainMenuLoadDialog
	};
	/** Pending save/load dialog requested by the main menu callback. */
	PendingMainMenuSaveLoadDialog _pendingMainMenuSaveLoadDialog = kPendingMainMenuSaveLoadNone;
	/** Localized key queued by the debugger message-box command. */
	uint32 _pendingTextMsgBoxKey = 0;

	/** Observe one key before page dispatch for Ctrl+A cheat phrase collection. */
	void observeBuiltinCheatKey(const Common::KeyState &kbd);
};

} // End of namespace Mohawk

#endif // MOHAWK_ZOOMBINI_H
