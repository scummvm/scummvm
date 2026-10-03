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

#ifndef MOHAWK_ZOOMBINI_PAGES_SHELTER_TOWN_H
#define MOHAWK_ZOOMBINI_PAGES_SHELTER_TOWN_H

#include "mohawk/zoombini_pages/shelter_base.h"

namespace Mohawk {

/**
 * Town shelter page at the end of the journey.
 *
 * Town transfers the active pack into persistent town storage, displays
 * population-gated overlays and memorial cards, and animates celebration
 * walkers. Its horizontal scrolling background is saved as page state so a
 * later visit restores both the entity positions and the parallax layer
 * frames.
 */
class ZoombiniShelterTown : public ZoombiniShelter {
public:
	/** Create the Town page. */
	ZoombiniShelterTown(MohawkEngine_Zoombini *vm);
	/** Release town walkers, memorial card state, and features. */
	~ZoombiniShelterTown() override;

	/** Load persistent town state and prepare the current population view. */
	void open() override;
	/** Select Town background music, including route-dependent variants. */
	void setBackgroundMusic() override;
	/** Select the Town background bitmap. */
	void setBackgroundBitmap() override;
	/** Transfer the arriving pack and derive the persistent Town population state. */
	void initStates() override;
	/** Load town overlays, inhabitants, memorials, and controls. */
	void loadFeatures() override;
	/** Restart the global Snoid fidget inactivity period after Town loads. */
	bool requiresFidgetActivityResetOnLoad() const override { return true; }
	/** Return the original Town script-sound range priority. */
	const ZoombiniPage::ScriptSoundPriorityRanges &getScriptSoundPriorityRanges() const override;

	/** Handle Town-specific state controls exposed through the page debugger command. */
	bool debugDoPageCommand(int argc, const char **argv, Common::String &output) override;
	/** Describe Town-specific debugger subcommands for the top-level page help. */
	Common::String debugGetPageCommandHelp() const override;

	/** Set a temporary population value for diagnostic previewing. */
	void debugSetPopulationPreview(int16 population);
	/** Clear the diagnostic population override. */
	void debugClearPopulationPreview();
	/** Set the number of pending diagnostic fireworks. */
	void debugSetPendingFireworks(int16 count);
	/** Start the diagnostic fireworks cycle. */
	void debugStartFireworksCycle();
	/** Stop the diagnostic fireworks cycle. */
	void debugStopFireworksCycle();

protected:
	/** Run a Town built-in debug action. */
	bool debugDoBuiltinDebugCommand(int argc, const char **argv, Common::String &output) override;
	/** Describe Town built-in debug actions. */
	Common::String debugGetBuiltinDebugCommandHelp() const override;
	/** Handle the Town built-in debug keys. */
	ZmbEventHandleResult onDebugKeyDown(const Common::KeyState &kbd) override;
	/** Forget the cached Town hover cursor before modal input takes ownership. */
	void clearHoverCursor() override;
	/** Handle Town keyboard shortcuts and modal-card dismissal. */
	ZmbEventHandleResult onKeyDown(const Common::KeyState &kbd, bool kbdRepeat) override;
	/** Advance Town walkers and Town-owned audio after the render pass. */
	void onPostRenderFrame() override;
	/** Update Town scroll-button and memorial-card hover state. */
	ZmbEventHandleResult onMouseMove(const Common::Point &absPos, const Common::Point &relPos) override;
	/** Dispatch a Town click to scrolling, memorial cards, or walkers. */
	ZmbEventHandleResult onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) override;
	/** Save Town scroll and memorial state before leaving for the map. */
	void saveStateBeforeMapTransition() override;

	// Memorial card overlay system ---

	/**
	 * Shows the memorial card for the given slot.
	 * Loads the SCRB with the five-row text layout for the level, route, practice label, difficulty, and date.
	 * Calls @ref ZoombiniShelterTown::setBackgroundRunnerRenderEnabled().
	 * This hides the background runners while the modal is active.
	 */
	void showMemorialCard(int16 slotIdx);

	/** Dismiss the memorial card overlay. */
	void hideMemorialCard();

	/** Resolve a town walker drag release. */
	void endDrag(const Common::Point &dropPos) override;

	/** Hit-test against memorial statue hotspots (16 card slots). */
	int16 hitTestMemorialHotspots(const Common::Point &pos) const;
	/** Return whether @p pos is inside a Town control rectangle. */
	bool isTownButtonRect(const Common::Point &pos) const;
	/** Return whether @p pos is inside the Town panorama input viewport. */
	bool isTownScrollViewportPoint(const Common::Point &pos) const;

	/** Prepare population-gated overlay shapes for rendering. */
	void overlay_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);
	/** Prepare memorial-marker shapes and their unlocked hotspot set. */
	void memorialMarkers_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);
	/** Render the active memorial card over the Town scene. */
	void memorialCard_onPostRender(ZmbFeature *feature);
	/** Select per-frame Map and Help button shapes. */
	bool controls_preRender(ZmbFeature *feature);
	/** Render the combined Map and Help control runner. */
	ZmbRenderResult controls_render(ZmbFeature *feature);
	/** Dispatch actions from the combined control runner. */
	void controls_postRender(ZmbFeature *feature);
	/** Handle presses on the combined control runner. */
	ZmbEventHandleResult controls_lButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);

	/**
	 * Transfer active pack Zoombinis into town stored chunk entries.
	 * Each Zoombini is stored at the index returned by @ref ZmbRosterEntry::snoidId().
	 */
	void transferActivePackToTownStorage();

private:
	/** Town actions accepted by the built-in debug console and keyboard dispatcher. */
	enum class BuiltinDebugAction {
		kInvalid,
		kCheatText,
		kFireworksLast,
		kFireworksNext,
		kMemorialPrevious,
		kMemorialNext,
		kMemorialFill,
		kMemorialClear,
		kThresholdUp,
		kThresholdDown
	};
	static constexpr const char *kBuiltinDebugActionCheatText = "cheat-text";
	static constexpr const char *kBuiltinDebugActionFireworksLast = "fireworks-last";
	static constexpr const char *kBuiltinDebugActionFireworksNext = "fireworks-next";
	static constexpr const char *kBuiltinDebugActionMemorialPrevious = "memorial-prev";
	static constexpr const char *kBuiltinDebugActionMemorialNext = "memorial-next";
	static constexpr const char *kBuiltinDebugActionMemorialFill = "memorial-fill";
	static constexpr const char *kBuiltinDebugActionMemorialClear = "memorial-clear";
	static constexpr const char *kBuiltinDebugActionThresholdUp = "threshold-up";
	static constexpr const char *kBuiltinDebugActionThresholdDown = "threshold-down";
	/** Parse one console-facing Town action name. */
	static BuiltinDebugAction parseBuiltinDebugAction(const Common::String &action);
	/** Run one typed Town keyboard-debug branch. */
	bool runBuiltinDebugAction(BuiltinDebugAction action, Common::String &output);
	/** Draw and report the current Town memorial Cheat Text selection. */
	void showBuiltinCheatTextSelection(Common::String &output);
	/**
	 * Immutable Town control rectangles are instance members because Common::Rect
	 * requires runtime construction and ScummVM prohibits global C++ constructors.
	 */
	const Common::Rect _mapButtonRect = Common::Rect(600, 403, 639, 440);
	const Common::Rect _helpButtonRect = Common::Rect(600, 441, 639, 478);

	/** Convert the stored Town population to the authored density range. */
	static int16 calculatePopulationDensity(int16 population);
	/** Rebuild population-gated Town overlays after a count change. */
	void refreshPopulationOverlay();
	/** Return whether any celebration walker is still active. */
	bool hasActiveCelebrationWalkers() const;
	/** Advance the debugger-only accelerated fireworks cycle. */
	void advanceDebugFireworksCycle();

protected:
	/**
	 * Horizontally shifts town entity positions by one 320px column with wrap across the 1920px world.
	 * @p phaseIdx 1 scrolls right (entities move left); @p phaseIdx 0 scrolls left.
	 */
	void shiftRunnersForScroll(int16 phaseIdx);
	/** Shift one Town entity while wrapping it through the 1920-pixel world. */
	void shiftTownEntityForScroll(ZmbFeature *feature, int16 phaseIdx);

	/**
	 * Select the parallax background-layer frame indices for one Town scroll column.
	 * Initial setup performs this immediately before registering the clock runner.
	 */
	void advanceLayerFrameState(uint16 scrollCol);
	/** Request a one-column scroll toward the left. */
	void scrollTownLeft();
	/** Request a one-column scroll toward the right. */
	void scrollTownRight();

	/** Page-local Town backgrounds, overlays, memorials, and walkers. */
	enum PageResourceId : int16 {
		kResBackground1200 = 1200,

		kResBitmapShape1000_Overlay = 1000,
		kResBitmapShape1100 = 1100,
		kResBitmapShape2000_Cursors = 2000,
		kResBitmapShape4000_Inhabitant = 4000,
		kResBitmapShape6000_ClockHands = 6000,
		kResBitmapShape8000_SubFeature = 8000,

		kResRegs2000 = 2000,
		kResRegs6000_ClockHands = 6000,

		kResScrb1000_Overlay = 1000,
		kResScrb1001_Overlay = 1001,
		kResScrb1002_Overlay = 1002,
		kResScrb1003_Overlay = 1003,
		kResScrb1004_MemorialCard = 1004,
		kResScrb1005_MemorialCard = 1005,
		kResScrb1006_MemorialCard = 1006,
		kResScrb1007_MemorialCard = 1007,
		kResScrb4000_SubFeature = 4000,
		kResScrb4999_Reject = 4999,
		kResScrb5000_Normal = 5000,
		kResScrb6000_ClockHands = 6000,
		kResScrb8000_SubFeatureBase = 8000,
		kResScrb8007_SubFeatureGroup0Last = 8007,
		kResScrb8009_SubFeatureGroup1Last = 8009,
		kResScrb8017_SubFeatureGroup2Last = 8017,
		kResScrb8021_SubFeatureLast = 8021,
		kResScrb8022_NormalBase = 8022,
		kResScrb8029_NormalGroup0Last = 8029,
		kResScrb8031_NormalGroup1Last = 8031,
		kResScrb8039_NormalGroup2Last = 8039,
		kResScrb8043_NormalLast = 8043,

		kResScrs4999_NormalBase = 4999,

		// Sound resources.
		kResSound3000_BGM = 3000,
		kResSound3001_BGM = 3001,
		kResSound3002_BGM = 3002,
		kResSound3003_BGM = 3003,

		// Sound ranges used by getScriptSoundPriority().
		kResSoundRange3000_BGM = 3000,
		kResSoundRange3003_BGM = 3003,
	};

	// -----------------------------------------------------------------------
	// Shape indices within TOWN cursor/button resources
	// -----------------------------------------------------------------------
	enum ShapeId : uint16 {
		// Hover cursor shapes.
		kShape2000_ArrowLeft_01 = 1,
		kShape2000_ArrowRight_02 = 2,
		kShape2000_Magnifier_03 = 3,

		// Exit gate scroll buttons.
		kShape1100_ExitGateLeftNormal_05 = 5,
		kShape1100_ExitGateLeftPressed_06 = 6,
		kShape1000_ExitGateRightNormal_24 = 24,
		kShape1000_ExitGateRightPressed_25 = 25,
	};

	/** Whether all 625 Zoombinis have been stored in town. */
	bool _allZoombinisInTown = false;

	/** Number of active pack Zoombinis that were moved to town storage on entry. */
	int16 _activePackCount = 0;
	/** First empty Town storage slot after the arriving pack is transferred. */
	int16 _firstEmptyStorageSlot = 0;

	/**
	 * Calculated town population density, controls how full the town appears.
	 * Range: 25 ~ 80
	 */
	int16 _townPopDensity = 0;

	/**
	 * Temporary Town population used only by the debugger preview command.
	 * It never updates the save state or the actual Town arrival count.
	 */
	bool _debugPopulationPreviewActive = false;

	/** Exact sound resource to play on Town entry. */
	ZmbResource _entrySoundRes;

	/** Whether to play the entry sound immediately. */
	bool _playEntrySoundImmediately = false;

	/**
	 * Number of pending celebration walkers ("fireworks") to spawn.
	 * Set from development-level thresholds in @ref ZoombiniShelterTown::loadFeatures().
	 * Decremented each time a walker is spawned.
	 */
	int16 _developAnimTimer = 0;

	/**
	 * Debug-only accelerated cycle: queue the full-Town entry amount,
	 * then the ambient-completion refill amount after it drains.
	 * This is page-local and disappears when Town is destroyed.
	 */
	bool _debugFireworksCyclePendingRefill = false;
	/** Last count selected by the F fireworks debug branch. */
	int16 _builtinFireworksCount = 0;
	/** One-based memorial debug-card data selector; zero disables the override. */
	int16 _builtinCheatTextIndex = 0;
	/** Route/level pair advanced by each period-key memorial insertion. */
	int16 _builtinMemorialRouteCounter = 0;

	/**
	 * Pointers to the four overlay SCRB features, saved for sub-feature linking.
	 * Index 0: SCRB 1000, 1: SCRB 1002, 2: SCRB 1003, 3: SCRB 1001
	 */
	ZmbFeature *_overlayFeatures[4] = {nullptr, nullptr, nullptr, nullptr};
	/** Immutable Map hotspot templates for the combined control runner. */
	Common::Array<ZmbHotspot> _controlGoMapBaseHotspots;
	/** Per-frame Map hotspots for the combined control runner. */
	Common::Array<ZmbHotspot> _controlGoMapHotspots;
	/** Immutable Help hotspot templates for the combined control runner. */
	Common::Array<ZmbHotspot> _controlHelpBaseHotspots;
	/** Per-frame Help hotspots for the combined control runner. */
	Common::Array<ZmbHotspot> _controlHelpHotspots;

	/**
	 * Number of town inhabitant Zoombinis rendered as static background decorations.
	 * Determined by (storedTownCount - 20) / 37, clamped to [0, 16].
	 */
	uint16 _inhabitantCount = 0;

	/** Walking Zoombini snoid count. Up to 20 can be spawned from stored chunk data. */
	uint16 _walkingZmbCount = 0;

	/**
	 * Snoid IDs for walking Zoombinis. Up to 20 entries.
	 * These are the snoid feature IDs registered by @ref ZoombiniPage::loadSnoidFromPack(),
	 * with the mutable runtime category changed from TYPE_SNOID to TYPE_GRIDWALKER
	 * after creation.
	 * The objects remain @ref ZmbSnoid instances and never gain
	 * @ref ZmbGridWalker traversal state.
	 * Snoid IDs use range 20000+ to avoid collision with inhabitant IDs (0-15).
	 */
	uint16 _walkingZmbSnoidIds[20] = {};

	/** Town inhabitant position data (16 x,y coordinate pairs). */
	static constexpr Common::Point kInhabitantPositions[16]{
		Common::Point(467, 265),
		Common::Point(349, 225),
		Common::Point(777, 291),
		Common::Point(828, 284),
		Common::Point(44, 330),
		Common::Point(283, 152),
		Common::Point(195, 211),
		Common::Point(607, 201),
		Common::Point(1182, 287),
		Common::Point(1299, 228),
		Common::Point(1422, 269),
		Common::Point(1807, 316),
		Common::Point(1048, 309),
		Common::Point(709, 228),
		Common::Point(1740, 284),
		Common::Point(1532, 172),
	};

	/**
	 * Town-inhabitant SCRB table for the 16 authored positions.
	 * Entries cycle through SCRB 4000-4007 twice and remain paired by slot with
	 * the positions selected by the random slot-selection pass.
	 * These are SCRB resources, not SCRS, loaded through @ref ZoombiniPage::loadSnoidFromScrb().
	 */
	static constexpr int16 kInhabitantScrbTable[16]{
		4000,
		4001,
		4002,
		4003,
		4004,
		4005,
		4006,
		4007,
		4000,
		4001,
		4002,
		4003,
		4004,
		4005,
		4006,
		4007,
	};

	static constexpr int16 kScrollableCursorAreaWidth = 80;

	/**
	 * Clock-tower hands feature (SCRB 6000).
	 * It registers with the Snoid storage selector, then changes its mutable
	 * runtime category to GRIDWALKER without gaining grid traversal state.
	 * The final category makes Town scrolling and memorial modal visibility
	 * updates include it, while LOOP_ANIM selects its render bucket.
	 * @ref ZoombiniShelterTown::clockHands_preRenderShape() renders the time-dependent shapes.
	 */
	ZmbFeature *_clockHandsFeature = nullptr;

	/**
	 * Memorial card overlay state.
	 * @ref ZoombiniShelterTown::_memorialCardActive blocks other clicks while the card is displayed.
	 * @ref ZoombiniShelterTown::_memorialCardSlotIdx identifies which memorial slot is shown.
	 * @ref ZoombiniShelterTown::_memorialHotspots stores the 16 statue hit-test rectangles.
	 */
	bool _memorialCardActive = false;
	/** Town storage slot currently shown by the memorial-card overlay. */
	int16 _memorialCardSlotIdx = -1;
	/** Feature that renders the active memorial-card overlay. */
	ZmbFeature *_memorialCardFeature = nullptr;
	/** Number of valid entries in @ref _memorialHotspots. */
	uint16 _memorialHotspotCount = 0;
	/** Authored Town statue rectangles used for memorial-card hit testing. */
	Common::Rect _memorialHotspots[16] = {};
	/** Mapping from visible statue index to stored memorial-card slot. */
	int16 _memorialSlotMapping[16] = {};
	/** Cursor shape selected for the current Town hover target. */
	uint16 _hoverCursorShapeIdx = ZmbHotspot::kShapeNone;

	/** Hide or restore Town background runners while the memorial card is modal. */
	void setBackgroundRunnerRenderEnabled(bool renderEnabled);

	/** Run the clock-tower hands timer and materialize one due SCRB frame. */
	bool clockHands_preRender(ZmbFeature *feature);

	/**
	 * Pre-render shape callback for the clock-tower hands (SCRB 6000).
	 * Filters visibility, advances an armed spin, and selects shapes from system time.
	 */
	void clockHands_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);

	/**
	 * Updates the clock-hand steps from the system clock.
	 * Samples immediately on Town setup, then every 1800 frames (debounced).
	 */
	void updateClockHandTime();

	/** Whether the visible memorial-marker group contains the unlocked clock tower. */
	bool _clockTowerHandsEnabled = false;
	/** Clock hour-hand step. System time uses 0-4; a spin traverses 0-11. */
	byte _clockHourStep = 0;
	/** Clock minute-hand step (0-11). */
	byte _clockMinuteStep = 0;
	/** Captured hour-hand step at which the active spin ends. */
	byte _clockSpinTargetHourStep = 0;
	/** Captured minute-hand step at which the active spin ends. */
	byte _clockSpinTargetMinuteStep = 0;
	/** Negative arms a spin, positive counts complete 12-by-12 revolutions. */
	int16 _clockSpinCycles = 0;
	/** Frame counter of the last system-time update. */
	uint32 _clockLastTimeUpdateFrame = 0;

	// Celebration walker state ---

	/**
	 * Active celebration walker features. Up to 3 concurrent walkers.
	 * Created from SCRB 8000-8043 with PLAY_ONCE animation.
	 */
	ZmbFeature *_celebWalkerFeatures[3] = {nullptr, nullptr, nullptr};

	/** Number of celebration walkers pending removal (animation completed). */
	int16 _nPendingWalkerRemovals = 0;

	/** Spawn one celebration walker when @ref ZoombiniShelterTown::_developAnimTimer is active and a slot is free. */
	void spawnCelebrationWalker();

	/** Remove completed celebration walkers whose animation has ended. */
	void cleanupFinishedWalkers();

	// Ambient sound cycling state ---

	/**
	 * Exact current ambient resource.
	 * Cycles between page music SND 3000-3002 and system voice SND 20089-20093.
	 */
	ZmbResource _ambientSoundResource;

	/**
	 * Whether the current ambient sound has finished playing.
	 * When set, triggers the delay timer. Cleared after delay starts.
	 */
	bool _ambientSoundDone = false;

	/**
	 * First-play flag: when set, next sound selection picks from voice pool.
	 * When clear, next selection advances music track.
	 * Toggled each cycle to alternate voice/music.
	 */
	bool _ambientSoundFirstPlay = false;

	/** Frame counter when the last ambient sound finished. */
	uint32 _ambientSoundLastTime = 0;

	/** Random delay duration before the next ambient sound (150-300 frames). */
	uint32 _ambientSoundDelay = 0;

	/**
	 * Non-repeat random pool state for ambient voice selection.
	 * Pool of 5 entries: [20089, 20090, 20091, 20092, 20093].
	 */
	uint32 _ambientVoicePoolState = 0;

	/** Ambient voice pool: 5 voice tracks for Town ambient cycling. */
	static constexpr int16 kAmbientVoicePool[5]{
		kSysResSound20089_Ambient,
		kSysResSound20090_Ambient,
		kSysResSound20091_Ambient,
		kSysResSound20092_Ambient,
		kSysResSound20093_Ambient,
	};

	/** Queue Town PCM music or Narrator speech without leaving its shared priority domain. */
	void queueTownSound(ZmbResource resource);

	/** Compute route-based music sound ID from maze page flag. */
	int16 computeRouteMusicId() const;

	// Idle animation state ---
	// Town uses walkers and development state to choose idle work; this is not
	// interchangeable with puzzle-page idle animation state.

	/** Idle animation budget: number of celebrations remaining. */
	int16 _idleAnimBudget = 0;
	/** Frame counter of last idle anim trigger. */
	uint32 _idleAnimLastFrame = 0;
	/** Idle anim interval in frames. */
	uint32 _idleAnimInterval = 0;
	/** Non-repeat random pool state for idle anim selection. */
	uint32 _idleAnimPoolState = 0;
};

} // End of namespace Mohawk

#endif
