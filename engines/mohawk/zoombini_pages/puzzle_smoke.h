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

#ifndef MOHAWK_ZOOMBINI_PAGES_PUZZLE_SMOKE_H
#define MOHAWK_ZOOMBINI_PAGES_PUZZLE_SMOKE_H

#include "mohawk/zoombini_pages/puzzle_base.h"

namespace Mohawk {

/**
 * Mirror Machine puzzle page (@ref ZoombiniPageType::kSmoke).
 * Route 4, Puzzle 2
 *
 * Magic mirror checks if the lvalue and rvalue matches.
 * The player must place the correct Zoombini onto the minecart (lvalue), and correct mirror (rvalue) to make the Zoombini passable.
 * In the later difficulty levels, the player must also place the correct shims between the lvalue/rvalue and the magic mirror.
 *
 * The puzzle uses a custom stack-based positioning system instead of @ref ZoombiniInteractive::layoutStaticAndWalkIn().
 */
class ZoombiniPuzzleSmoke : public ZoombiniPuzzle {
private:
	/** Stores puzzle-specific state for each Smoke runner. */
	struct ZmbSmokeRunnerState {
		/** Shape-bank and frame variants used by Smoke's custom runner renderer. */
		enum class Orientation : byte {
			/** Body bank 1 with frame 65. */
			kBaseOneFramed00 = 0,
			/** Body bank 2 with frame 64. */
			kBaseTwoFramed01 = 1,
			/** Body bank 3 with frame 66. */
			kBaseThreeFramed02 = 2,
			/** Body bank 3 without a frame. */
			kBaseThree03 = 3,
			/** Body bank 2 without a frame. */
			kBaseTwo04 = 4,
			/** Body bank 1 without a frame. */
			kBaseOne05 = 5,
			/** Alternate body-bank-1 frame variant. */
			kBaseOneFramedAlternate06 = 6,
			/** Body bank 2 with the grid frame. */
			kBaseTwoFramedGrid07 = 7,
			/** Body bank 3 with the exit frame. */
			kBaseThreeFramedExit08 = 8
		};
		/** Coverage update modes used by Smoke's custom runner renderer. */
		enum class DirtyMode : byte {
			/** Do not request a redraw for a hidden custom runner. */
			kSuppressRedraw02 = 2,
			/** Recompose the custom runner and its old coverage. */
			kRecompose04 = 4,
			/** Recompose coverage while the runner follows a drag. */
			kDragging05 = 5
		};
		/** One-based trait column selected for the runner's cycle or blink. */
		enum class CycleTrait : byte {
			/** No trait column is selected. */
			kNone00 = 0,
			/** Hair column. */
			kHair01 = 1,
			/** Eyes column. */
			kEyes02 = 2,
			/** Nose column. */
			kNose03 = 3,
			/** Feet column. */
			kFeet04 = 4
		};
		/** Cached canonical traits displayed by this runner. */
		ZmbTrait traits;
		/** Current mirror-facing orientation. */
		Orientation orientation = Orientation::kBaseOneFramed00;
		/** Dirty-rectangle mode used by the custom runner renderer. */
		DirtyMode dirtyMode = DirtyMode::kRecompose04;
		/** Trait currently being cycled or blinked by the runner display. */
		CycleTrait cycleTrait = CycleTrait::kNone00;
		/** Return the one-based cycle trait index as an integer. */
		int16 getCycleTraitIndex() const { return static_cast<int16>(cycleTrait); }
		/** Nonzero value that advances through the cycling trait values. */
		byte cyclingValue = 0;
		/** Frozen trait value restored on alternating blink phases. */
		byte blinkValue = 0;
		/** Whether the runner renderer suppresses all shape output. */
		bool suppressShapes = false;
		/** Next frame at which the runner may render. */
		uint32 nextRenderFrame = 0;
		/** Next frame at which the runner cycles or blinks its displayed trait. */
		uint32 nextTraitUpdateFrame = 0;
	};

public:
	/** Create the Mirror Machine puzzle page. */
	ZoombiniPuzzleSmoke(MohawkEngine_Zoombini *vm);
	/** Release mirror, runner, and page resources. */
	~ZoombiniPuzzleSmoke() override;
	/** Static puzzle name used by debug answer descriptions. */
	static constexpr const char *kPageName = ZmbXferRouteInfo::kXferPageNameMirrorMachine;
	static constexpr int kRouteNumber = 4;
	static constexpr int kRoutePuzzleIdx = 2;
	/** Return the puzzle name used by debug answer descriptions. */
	const char *getPageName() const override { return kPageName; }
	int getRouteNumber() const override { return kRouteNumber; }
	int getRoutePuzzleIdx() const override { return kRoutePuzzleIdx; }

	/** Generate mirror rules and initialize the active pack. */
	void open() override;
	/** Select the Mirror Machine puzzle music. */
	void setBackgroundMusic() override;
	/** Select the Mirror Machine background bitmap. */
	void setBackgroundBitmap() override;
	/** Initialize page-specific state before loading features. */
	void initStates() override;
	/** Load mirror, crystal, runner, and feedback features. */
	void loadFeatures() override;
	/** Restart the global Snoid fidget inactivity period after Mirror Machine loads. */
	bool requiresFidgetActivityResetOnLoad() const override { return true; }
	/** Select one authored Mirror Machine F1 help prompt after the fidget reset. */
	void initHelpPrompt() override;
	/** Advance custom crystal timers before rendering. */
	void onEveryFrame() override;
	/** Process rendered animation events, comparison state, and idle behavior. */
	void onPostRenderFrame() override;
	/** Process mirror, runner, and answer animation callbacks. */
	void onFeatureAnimEvent(ZmbFeature *feature, int16 eventCode) override;

	/** Start a mirror-slot drag or handle a control click. */
	ZmbEventHandleResult onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) override;
	/** Resolve a mirror-slot drop and release the pointer state. */
	ZmbEventHandleResult onLButtonUp(const Common::Point &absPos, const Common::Point &relPos) override;
	/** Update mirror-slot hover and active drag feedback. */
	ZmbEventHandleResult onMouseMove(const Common::Point &absPos, const Common::Point &relPos) override;

protected:
	/** Mirror Machine actions accepted by the built-in debug console and keyboard dispatcher. */
	enum class BuiltinDebugAction {
		kInvalid,
		kLevel,
		kCheatOn,
		kCheatOff
	};
	static constexpr const char *kBuiltinDebugActionLevel = "level";
	static constexpr const char *kBuiltinDebugActionCheatOn = "cheat-on";
	static constexpr const char *kBuiltinDebugActionCheatOff = "cheat-off";
	/** Custom runner families created by the Smoke stack builder. */
	enum class RunnerType : int16 {
		kCliff01 = 1,
		kLevel02 = 2,
		kGrid03 = 3,
		kExit04 = 4,
		kBottom05 = 5
	};
	/** Result of comparing the two active Smoke order lines. */
	enum class CompareResult : int16 {
		kMatch00 = 0,
		kMismatch02 = 2
	};
	/** Level-4 transition phase encoded by the authored transition positions. */
	enum class TransitionPhase : int16 {
		/** The first pair is still using the initial target. */
		kInitialPair03 = 3,
		/** The second pair member is moving through the transition. */
		kSecondPair02 = 2,
		/** The answer display is ready for the next pair. */
		kAnswerReady01 = 1
	};
	/** Remap only custom runner nose shapes for Color Assist. */
	ZoombiniGraphics::PaletteRemapMode getColorAssistPaletteRemap(const ZmbFeature *feature, const ZmbHotspot &hotspot, ZmbResource resource) const override;
	/** Resolve the custom stack placement after a drag ends. */
	void endDrag(const Common::Point &dropPos) override;
	/** Use the custom stack drag policy rather than ordinary Snoid dragging. */
	bool hasStickyMouseDrag() const override;
	/** Finish a sticky drag and update the runner stack. */
	void endStickyMouseDrag(const Common::Point &dropPos) override;
	/** Begin the Mirror Machine departure sequence. */
	void onGoButtonActivated() override;
	/** Wait for the authored cart controller to complete the departure state. */
	void updateDepartureState() override;
	/** Keep the authored cart controller active until its transition event fires. */
	bool runsControllerDuringDeparture() const override { return true; }
	/** Return the sound priority used by page SCRB playback. */
	const ZoombiniPage::ScriptSoundPriorityRanges &getScriptSoundPriorityRanges() const override;
	/** Describe the generated mirror comparison for diagnostics. */
	Common::String debugGetAnswer() const override;
	/** Describe the Mirror Machine built-in debug actions. */
	Common::String debugGetBuiltinDebugCommandHelp() const override;
	/** Run one Mirror Machine built-in debug action. */
	bool debugDoBuiltinDebugCommand(int argc, const char **argv, Common::String &output) override;
	/** Handle Mirror Machine built-in debug keys. */
	ZmbEventHandleResult onDebugKeyDown(const Common::KeyState &kbd) override;

private:
	/** Parse one console-facing Mirror Machine action name. */
	static BuiltinDebugAction parseBuiltinDebugAction(const Common::String &action);
	/** Run one typed Mirror Machine built-in debug action. */
	bool runBuiltinDebugAction(BuiltinDebugAction action, Common::String &output);
	/** Apply one authored display-trait row to a diagnostic trait record. */
	void applyDebugDisplayTraitRow(ZmbTrait &traits, int16 row) const;
	/** Apply one filter runner's trait effects to a diagnostic trait record. */
	static void applyDebugFilterTraits(ZmbTrait &traits, const ZmbSmokeRunnerState &filter);
	/** Advance the six-filter diagnostic permutation. */
	static bool advanceDebugFilterPermutation(int16 values[6]);
	/** Page-local Mirror Machine crystal, runner, and animation resources. */
	enum PageResourceId : int16 {
		kResBackground5000 = 5000,

		// Terrain bitmaps
		kResBitmapTerrain100 = 100,

		// Shape bitmaps
		kResBitmapShape6000_GoMapButtons = 6000,
		kResBitmapShape10000_Crystal = 10000,
		kResBitmapShape11000_Smoke = 11000,

		kResRegs10000_Smoke = 10000,

		kResScrb11000_FeatureBase = 11000,
		kResScrb11001_DrawOnReg = 11001,
		kResScrb11002_Well = 11002,
		kResScrb11003_Answer = 11003,
		kResScrb11004_WellAlternate = 11004,
		kResScrb11005_AnswerAlternate = 11005,
		kResScrb11006_CliffLeft = 11006,
		kResScrb11007_CliffRight = 11007,
		kResScrb11008_Background = 11008,
		kResScrb11009_BackgroundOverlay = 11009,
		kResScrb11011_Level4Overlay = 11011,
		kResScrb11012_Level4Transition = 11012,
		kResScrb11013_Overlay = 11013,
		kResScrb11015_MainStartup = 11015,
		kResScrb11016_SecondStartup = 11016,
		kResScrb11017_MainResult = 11017,
		kResScrb11018_CompareA = 11018,
		kResScrb11019_CompareB = 11019,
		kResScrb11024_MainLevel1 = 11024,
		kResScrb11025_MainLevel1 = 11025,
		kResScrb11026_MainLevel1 = 11026,
		kResScrb11027_MainLevel1 = 11027,
		kResScrb11028_MainLevel3 = 11028,
		kResScrb11029_MainLevel3 = 11029,
		kResScrb11030_MainLevel3 = 11030,
		kResScrb11031_MainLevel3 = 11031,
		kResScrb11032_StackLevel1 = 11032,
		kResScrb11033_StackLevel3 = 11033,
		kResScrb11034_StackLevel4 = 11034,
		kResScrb11035_TravelLevel3 = 11035,
		kResScrb11036_RejectionBase = 11036,
		kResScrb11052_ColumnLast = 11052,
		kResScrb11071_ColumnFirst = 11071,
		kResScrb11072_DepartureBase = 11072,
		kResScrb11076_Level12Extra = 11076,
		kResScrb11077_HoldingArea = 11077,

		kResScrs11999_NormalBase = 11999,
		kResScrs12000_RejectBase = 12000,
		kResScrs12004_RejectLevel2 = 12004,
		kResScrs12009_RejectLevel3 = 12009,
		kResScrs12014_RejectLevel4 = 12014,
		kResScrs12020_RejectionBase = 12020,
		kResScrs12038_PickupLevel34 = 12038,
		kResScrs12039_DropLevel34 = 12039,
		kResScrs12040_WalkLevel34 = 12040,
		kResScrs12041_DepartureRiderBase = 12041,
		kResScrs12045_FeetIdleBase = 12045,

		// Sound ranges used by getScriptSoundPriority().
		kResSoundRange0125_PageBase = 125,
		kResSoundRange0149_PageLast = 149,
		kResSoundRange0450_PageBase = 450,
		kResSoundRange0474_PageLast = 474,
		kResSoundRange11000_Startup = 11000,
		kResSoundRange11001_Startup = 11001,
		kResSoundRange11002_Startup = 11002,
		kResSoundRange11003_Startup = 11003,
		kResSoundRange11004_Startup = 11004,
		kResSoundRange11005_Startup = 11005,
		kResSoundRange11006_Startup = 11006,
		kResSoundRange11007_Startup = 11007,
		kResSoundRange11008_Startup = 11008,
		kResSoundRange11009_Startup = 11009,
		kResSoundRange11010_Startup = 11010,
		kResSoundRange11011_Startup = 11011,
		kResSoundRange11012_Startup = 11012,
		kResSoundRange11013_Startup = 11013,
		kResSoundRange11014_Startup = 11014,
		kResSoundRange11015_Startup = 11015,
		kResSoundRange11016_Startup = 11016,
		kResSoundRange11017_Startup = 11017,

		kResMidi30030_SmokeBgmBase = 30030,
	};

	/** Level 4 callbacks that chain the mirrored runner display into the next phase. */
	enum L4MirrorChainEventCode : int16 {
		kL4MirrorChainEventCode017_AdvanceRunner = 17,
		kL4MirrorChainEventCode018_RefreshAnswer = 18,
		kL4MirrorChainEventCode019_ReloadLevel = 19
	};

	/** Callbacks that move Zoombinis through the mirrored lane and departure display. */
	enum MirrorLaneEventCode : int16 {
		kMirrorLaneEventCode030_PickupZoombini = 30,
		kMirrorLaneEventCode031_ReloadSmokeStack = 31,
		kMirrorLaneEventCode035_LoadNextZoombini = 35,
		kMirrorLaneEventCode036_AssignFirstTraits = 36,
		kMirrorLaneEventCode037_StartZoombiniWalk = 37,
		kMirrorLaneEventCode038_FinishLevel4Compare = 38,
		kMirrorLaneEventCode050_PlaceZoombini = 50,
		kMirrorLaneEventCode051_LinkRunners = 51,
		kMirrorLaneEventCode060_FinishDeparture = 60,
		kMirrorLaneEventCode251_SetBodyArrangementOne = 251
	};

	/** Low callbacks emitted by the mirrored runner match/comparison SCRB. */
	enum MirrorMatchEventCode : int16 {
		kMirrorMatchEventCode000_ToggleSnoidFacing = 0,
		kMirrorMatchEventCode001_ReloadMainRunner = 1,
		kMirrorMatchEventCode002_StartMatchCompare = 2,
		kMirrorMatchEventCode003_ResetCurrentLevel = 3,
		kMirrorMatchEventCode004_StartNextCompare = 4,
		kMirrorMatchEventCode010_PlayCompareReject = 10,
		kMirrorMatchEventCode011_PlayCompareRejectAlternate = 11,
		kMirrorMatchEventCode013_PlayCompareRejectThird = 13,
		kMirrorMatchEventCode014_PlayCompareRejectFourth = 14,
		kMirrorMatchEventCode016_AdvanceDisplayPair = 16,
		kMirrorMatchEventCode017_RejectCurrentZoombini = 17
	};

	/** Load the active pack into Smoke's custom runner stacks. */
	void loadZoombinisFromPack();

	// === Static coordinate tables ===

	/** Zoombini pedestal positions. */
	static constexpr Common::Point kSnoidPositions[20]{
		Common::Point(214, 128),
		Common::Point(175, 126),
		Common::Point(135, 127),
		Common::Point(94, 126),
		Common::Point(53, 128),
		Common::Point(237, 176),
		Common::Point(196, 177),
		Common::Point(150, 178),
		Common::Point(110, 176),
		Common::Point(69, 178),
		Common::Point(234, 36),
		Common::Point(195, 37),
		Common::Point(155, 36),
		Common::Point(114, 35),
		Common::Point(73, 38),
		Common::Point(237, 79),
		Common::Point(196, 78),
		Common::Point(150, 80),
		Common::Point(110, 78),
		Common::Point(69, 79),
	};
	/** Origin used by the custom DRAW_ON_REG runner. */
	static constexpr Common::Point kDrawOnRegPosition = Common::Point(43, 258);
	static constexpr Common::Point kCliffRunnerPositions[8]{
		Common::Point(459, 26),
		Common::Point(535, 25),
		Common::Point(429, 80),
		Common::Point(500, 84),
		Common::Point(619, 76),
		Common::Point(423, 168),
		Common::Point(525, 167),
		Common::Point(605, 163),
	};
	/**
	 * Positions for grid runners 0-7.
	 * The cliff drop uses @ref ZoombiniPuzzleSmoke::kCliffDropSnapPosition.
	 */
	static constexpr Common::Point kGridRunnerPositions[8]{
		Common::Point(441, 66),
		Common::Point(531, 70),
		Common::Point(605, 67),
		Common::Point(421, 160),
		Common::Point(483, 153),
		Common::Point(612, 153),
		Common::Point(548, 255),
		Common::Point(616, 253),
	};
	static constexpr Common::Point kLevel2RunnerPositions[4]{
		Common::Point(187, 255),
		Common::Point(247, 255),
		Common::Point(424, 255),
		Common::Point(484, 255),
	};
	static constexpr Common::Point kExitRunnerPositions[2]{
		Common::Point(124, 255),
		Common::Point(548, 255),
	};
	/**
	 * L4 transition positions are indexed after decrementing transition phase
	 * from 3 to 2 and then from 2 to 1.
	 * Index 3 restores the grid-8 target home.
	 */
	static constexpr Common::Point kLevel4TransitionPositions[4]{
		Common::Point(124, 255),
		Common::Point(548, 255),
		Common::Point(580, 258),
		Common::Point(616, 253),
	};
	static constexpr Common::Point kBottomRunnerPositions[2]{
		Common::Point(317, 254),
		Common::Point(354, 254),
	};
	/** Off-screen hide position. */
	static constexpr Common::Point kHidePosition = Common::Point(-8, 258);
	/** Rejection position. */
	static constexpr Common::Point kRejectPosition = Common::Point(530, 384);
	/** Cliff drop snap position. */
	static constexpr Common::Point kCliffDropSnapPosition = Common::Point(548, 255);
	/**
	 * Immutable hit rectangles are instance members because Common::Rect requires
	 * runtime construction and ScummVM prohibits global C++ constructors.
	 */
	/** Level 1-2 cliff drop zone. */
	const Common::Rect _cliffDropRect = Common::Rect(525, 211, 582, 300);
	/** Minecart drop zone. */
	const Common::Rect _cartDropRect = Common::Rect(10, 245, 60, 290);
	/** Pedestal drop zone. */
	const Common::Rect _pedestalDropRect = Common::Rect(0, 31, 262, 244);
	/** Lever click zone. */
	const Common::Rect _leverClickRect = Common::Rect(9, 300, 75, 364);
	/** Pack-area row Y anchors. */
	static constexpr int16 kPedestalRowY[5]{
		38,
		81,
		126,
		176,
		221,
	};
	const Common::Rect _dragRectsA[9] = {
		// L3-4 group A (3 slots x 3 rects)
		Common::Rect(185, 230, 245, 293),
		Common::Rect(0, 0, 0, 0),
		Common::Rect(0, 0, 0, 0),
		Common::Rect(137, 230, 197, 293),
		Common::Rect(236, 230, 296, 293),
		Common::Rect(0, 0, 0, 0),
		Common::Rect(124, 230, 184, 293),
		Common::Rect(197, 230, 257, 293),
		Common::Rect(265, 230, 325, 293),
	};
	const Common::Rect _dragRectsB[9] = {
		// L3-4 group B (3 slots x 3 rects)
		Common::Rect(426, 230, 486, 293),
		Common::Rect(0, 0, 0, 0),
		Common::Rect(0, 0, 0, 0),
		Common::Rect(372, 230, 432, 293),
		Common::Rect(474, 230, 534, 293),
		Common::Rect(0, 0, 0, 0),
		Common::Rect(350, 230, 400, 293),
		Common::Rect(419, 230, 469, 293),
		Common::Rect(485, 230, 535, 293),
	};
	/** Layouts selected by the current left-side count. */
	static constexpr Common::Point kFilterPositionsA[9]{
		Common::Point(211, 255),
		Common::Point(0, 0),
		Common::Point(0, 0),
		Common::Point(187, 255),
		Common::Point(247, 255),
		Common::Point(0, 0),
		Common::Point(164, 255),
		Common::Point(211, 255),
		Common::Point(258, 255),
	};
	/** Layouts selected by the current right-side count. */
	static constexpr Common::Point kFilterPositionsB[9]{
		Common::Point(457, 255),
		Common::Point(0, 0),
		Common::Point(0, 0),
		Common::Point(424, 255),
		Common::Point(484, 255),
		Common::Point(0, 0),
		Common::Point(409, 255),
		Common::Point(457, 255),
		Common::Point(505, 255),
	};
	/** Display pair normal A (x=317). */
	static constexpr Common::Point kDisplayPairNormalA[13]{
		Common::Point(317, 263),
		Common::Point(317, 248),
		Common::Point(317, 236),
		Common::Point(317, 210),
		Common::Point(317, 201),
		Common::Point(317, 192),
		Common::Point(317, 201),
		Common::Point(317, 210),
		Common::Point(317, 236),
		Common::Point(317, 248),
		Common::Point(317, 263),
		Common::Point(317, 254),
		Common::Point(317, 254),
	};
	/** Display pair normal B (x=354). */
	static constexpr Common::Point kDisplayPairNormalB[13]{
		Common::Point(354, 263),
		Common::Point(354, 248),
		Common::Point(354, 236),
		Common::Point(354, 210),
		Common::Point(354, 201),
		Common::Point(354, 192),
		Common::Point(354, 201),
		Common::Point(354, 210),
		Common::Point(354, 236),
		Common::Point(354, 248),
		Common::Point(354, 263),
		Common::Point(354, 254),
		Common::Point(354, 254),
	};
	/** Display pair swapped A (x=317). */
	static constexpr Common::Point kDisplayPairSwappedA[17]{
		Common::Point(317, 257),
		Common::Point(317, 261),
		Common::Point(317, 264),
		Common::Point(317, 267),
		Common::Point(317, 270),
		Common::Point(317, 274),
		Common::Point(317, 277),
		Common::Point(317, 280),
		Common::Point(317, 277),
		Common::Point(317, 270),
		Common::Point(317, 267),
		Common::Point(317, 264),
		Common::Point(317, 261),
		Common::Point(317, 257),
		Common::Point(317, 254),
		Common::Point(317, 254),
		Common::Point(317, 254),
	};
	/** Display pair swapped B (x=354). */
	static constexpr Common::Point kDisplayPairSwappedB[17]{
		Common::Point(354, 257),
		Common::Point(354, 261),
		Common::Point(354, 264),
		Common::Point(354, 267),
		Common::Point(354, 270),
		Common::Point(354, 274),
		Common::Point(354, 277),
		Common::Point(354, 280),
		Common::Point(354, 277),
		Common::Point(354, 270),
		Common::Point(354, 267),
		Common::Point(354, 264),
		Common::Point(354, 261),
		Common::Point(354, 257),
		Common::Point(354, 254),
		Common::Point(354, 254),
		Common::Point(354, 254),
	};

	// === SCRB feature runners ===

	/** Feature for the page overlay animation. */
	ZmbFeature *_overlayAnimFeature = nullptr;
	/** Extra level 1-2 feature. */
	ZmbFeature *_level12ExtraFeature = nullptr;
	/** Left cliff feature. */
	ZmbFeature *_cliffLeftFeature = nullptr;
	/** Right cliff feature. */
	ZmbFeature *_cliffRightFeature = nullptr;
	/** Main page animation feature. */
	ZmbFeature *_mainAnimFeature = nullptr;
	/** First Smoke stack feature. */
	ZmbFeature *_smokeStackAFeature = nullptr;
	/** Second Smoke stack feature. */
	ZmbFeature *_smokeStackBFeature = nullptr;
	/** Secondary page animation feature. */
	ZmbFeature *_secondAnimFeature = nullptr;
	/** First compare display feature. */
	ZmbFeature *_compareAFeature = nullptr;
	/** Second compare display feature. */
	ZmbFeature *_compareBFeature = nullptr;
	/** Rejection animation feature. */
	ZmbFeature *_rejectionFeature = nullptr;
	/** Background controller feature. */
	ZmbFeature *_backgroundFeature = nullptr;
	/** Answer display feature. */
	ZmbFeature *_answerZoneFeature = nullptr;
	/** Holding-area feature for unassigned runners. */
	ZmbFeature *_holdingAreaFeature = nullptr;
	// === Gameplay methods ===

	/** Build the per-difficulty Smoke runner stacks. */
	void buildRunnerStacks();

	/** Spawn @p count runners of one Smoke stack type. */
	void spawnStackRunners(int16 count, RunnerType runnerType);

	/** Materializes the custom crystal-filter runner from its four trait bytes and orientation. */
	void smokeRunner_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup,
									Common::Array<ZmbHotspot> &hotspots);
	/** Render a custom Smoke runner from its cached traits. */
	ZmbRenderResult smokeRunner_render(ZmbFeature *feature);
	/** Advance one custom Smoke runner's cycle and render deadlines. */
	void tickSmokeRunner(ZmbFeature *feature, ZmbSmokeRunnerState &state, uint32 currentFrame);
	/** Advance every active custom Smoke runner before a render pass. */
	void tickSmokeRunners();
	/** Move a custom Smoke runner and select its dirty-rectangle mode. */
	void setSmokeRunnerPoint(ZmbFeature *feature, const Common::Point &point,
							 ZmbSmokeRunnerState::DirtyMode dirtyMode);

	/** Mutate @ref ZoombiniPuzzleSmoke::_questionZmbCount and @ref ZoombiniPuzzleSmoke::_questionTraits. */
	void selectQuestionZmb();

	/** Copy the selected display pair into the compare buffer. */
	int16 copyPairToCompareBuffer();

	/** Assign level-specific traits to one Smoke runner state. */
	void assignLevel2RunnerTraits(int16 runnerIdx, ZmbSmokeRunnerState &state);

	/** Generate one trait-grid row for a Smoke runner. */
	void generateTraitGrid(int16 rowIndex, ZmbSmokeRunnerState &state);

	/** Copy zmb traits to runner */
	void assignZmbTraitsFromSrc(int16 dstIdx, ZmbFeature *source);

	/** Read runner traits to display trait table */
	void cacheZmbTraits(int16 dstIdx, ZmbFeature *source);

	/** Read grid runner traits to display trait table */
	void loadZmbTraitsToCache();

	/** Advance display runner trait cycling */
	void cycleZmbTraitDisplay();

	/** Cache answer runner traits */
	void cacheAnswerRunnerTraits();

	/** Advance the answer runner through its authored frame sequence. */
	void advanceAnswerRunnerFrames();

	/** Clear runner traits by index */
	void clearZmbTraits(int16 runnerIdx);

	/** Clear one custom runner slot and its cached state. */
	void clearRunnerSlot(int16 slotIdx);

	/** Clear all custom runner slots before a level reset. */
	void clearAllRunnerSlots();

	/** Clear the two central reflection runners. */
	void clearReflectionRunners();

	/** Initialize the two runner features used for comparison. */
	void initMatchCompareRunners();

	/** Compare the active left and right order lines. */
	CompareResult compareTwoOrderLines();

	/** Reinit cliff runners for L1-2 */
	void initLowLevelQuestionRunners(int16 count);

	/** Reassign all level2 runners */
	void initLevel2RunnersAllTraits();

	/** Reinit grid runners for L3-4 */
	void initLevel3RunnersAllTraits();

	/** Start the next authored pair-comparison animation sequence. */
	void startNextCompareSequence();

	/** Reset current answer state and rebuild the selected level. */
	void resetAndReinitLevel();

	/** L4 transition phases */
	void handleFrameTransition(int16 eventCode);

	/** Play the rejection path for the currently compared Zoombini. */
	void playRejectedAnimation();

	/** Load a page SCRB on the answer runner. */
	void loadScrbOnAnswerRunner(int16 scrbId);

	/** Load a dispatch SCRB and optionally schedule its first render. */
	void loadScrbOnDispatchRunner(int16 scrbId, bool scheduleRender);

	/** Load a SCRB on the well controller runner. */
	void loadScrbOnWellRunner(int16 scrbId);

	/** Load the level-specific score animation SCRBs. */
	void loadScoreScrbs();

	/** Load score-display SCRBs for the active release. */
	void loadScoreDisplayScrbs();

	/** Load the level 1-2 timer controller SCRB. */
	void loadTimerScrb();

	/** Central animation callback */
	void processAnimDispatchEvent(ZmbFeature *feature, int16 eventCode);

	/** Custom drag mechanic (event-driven state machine) */
	int16 evaluateRunnerDrop(ZmbFeature *runner, const Common::Point &dropPos);
	/** Set custom-runner drag ownership and synchronize cursor visibility. */
	void setRunnerDragActive(bool active);

	/** Play SCRS on a Snoid through the page archive. */
	void playZmbScript(ZmbScrsCompletionMode completionMode, ZmbFeature *dispatchFeature, int16 scrsId, ZmbSnoid *snoid);
	/** Correct the arrival pose of level 1-2 walk-in Snoids. */
	void updateLevel12WalkInArrivalPose();

	/** Unload L1-2 timer SCRB */
	void unloadTimerScrb();

	/** Find a SmokeRunnerState for a given runner feature. */
	ZmbSmokeRunnerState *findRunnerState(ZmbFeature *feature);

	// === Core gameplay state ===

	/** Index of the Zoombini currently being processed in the page queue. */
	int16 _currentQueueIdx = 0;
	// Counts Smoke cliff placements; the same field name on other puzzles has a
	// different transition contract and must remain page-owned.
	/** Number of Snoids placed into Smoke's active positions. */
	int16 _placedZmbCount = 0;
	/** Number of Snoids currently loaded on the cliff. */
	int16 _loadedOnCliffCount = 0;
	/** Answer display state. */
	enum class AnswerState : int16 {
		/** Answer display is hidden. */
		kHidden00 = 0,
		/** Answer display is prepared for the next question. */
		kQuestion01 = 1,
		/** Answer display is idle or showing the current answer. */
		kIdle02 = 2
	};
	/** Current answer display state. */
	AnswerState _answerState = AnswerState::kIdle02;
	/** Whether the answer display is visible. */
	bool _bShowAnswer = false;
	/** Result of the current compare pair. */
	CompareResult _compareResult = CompareResult::kMatch00;
	/** Whether the current compare pair has swapped sides. */
	bool _bCompareSwapped = false;
	/** Current level 4 transition phase. */
	TransitionPhase _transitionPhase = TransitionPhase::kInitialPair03;
	/** Whether the first trait assignment is pending. */
	bool _bFirstTraitAssign = true;
	/** Position-table index for the moving reflection pair. */
	int16 _displayPairPositionIdx = 0;
	/** One-based selected candidate-panel index, or the level 3-4 ready sentinel. */
	int16 _matchRunnerIndex = 0;
	/** Toggle selecting Smoke stack A or B. */
	bool _bRunnerToggle = false;
	/** Whether Go is enabled after the first accepted Zoombini reaches the cliff. */
	bool _goButtonEnabled = false;
	/** Whether later level 3/4 filter regeneration preserves generated filter home-position order. */
	bool _bPreserveFilterHomePositionOrder = false;
	/** Zoombini currently participating in the cart, comparison, or rejection sequence. */
	ZmbSnoid *_activeZmb = nullptr;
	/** Whether the player may use the comparison lever or rearrange Zoombinis and comparison runners. */
	bool _comparisonInputEnabled = true;
	/** Zoombinis entering during the level 1/2 walk-in sequence. */
	ZmbSnoid *_level12WalkInSnoids[2] = {};
	/** Whether the level 1/2 walk-in pose is waiting for its animation event. */
	bool _level12WalkInPosePending[2] = {};

	// --- Non-Snoid runner drag state ---
	/** Cliff or grid runner currently being dragged. */
	ZmbFeature *_draggedRunner = nullptr;
	/** Runner position before the current non-Snoid drag began. */
	Common::Point _dragRunnerOrigPos;
	/** Mouse-to-runner offset preserved during a non-Snoid drag. */
	Common::Point _dragRunnerOffset;
	/** Last non-warped mouse position during a non-Snoid drag. */
	Common::Point _dragRunnerLastMousePos;
	/** Runner frame interval saved while a non-Snoid drag is active. */
	uint32 _dragRunnerSavedInterval = 0;
	/** One-based cliff runner selected when a drag is resumed. */
	int16 _dragRunnerMatchIdx = 0;
	/** Whether a non-Snoid runner drag is in progress. */
	bool _bRunnerDragActive = false;

	// --- Event flags (set by anim callback, consumed by frame handler) ---

	/** Event flag requesting placement of the current Zoombini. */
	bool _bPlaceActiveZmb = false;
	/** Event flag requesting the rejection runners to be relinked. */
	bool _bRelinkRejectionRunners = false;
	/** Event flag requesting the overlay and answer display to reload. */
	bool _bReloadOverlayAndAnswer = false;
	/** Event flag requesting a complete level reset. */
	bool _bResetLevel = false;
	/** Event flag requesting the accepted-cart departure sequence. */
	bool _bStartDeparture = false;
	/** Event flag requesting the main runner to reload. */
	bool _bReloadMainRunner = false;

	// --- Question/Answer traits ---

	/** Primary and secondary trait records displayed for the current question. */
	ZmbTrait _questionTraits[2];
	/** Zoombini selected to seed the current level 1-2 question. */
	uint16 _questionSnoidId = 0;
	/** Cliff crystal selected to match the current level 1-2 question. */
	int16 _questionCrystalIdx = -1;
	/** Remaining selectable Zoombinis at levels 1-2, or current pair size at levels 3-4. */
	int16 _questionZmbCount = 0;

	// === Trait display system ===

	/**
	 * Eight canonical trait records used for trait cycling.
	 * Row 0: source Zoombini, rows 1-3: ordered left filters,
	 * rows 4-6: ordered right filters, row 7: target Zoombini.
	 */
	ZmbTrait _displayTraits[8];

	/** Seen trait history records used while generating filters. */
	ZmbTrait _seenTraitsA;
	ZmbTrait _seenTraitsB;

	/** Home-position permutation for the six movable level 3-4 filters. */
	int16 _filterHomePermutation[8] = {};

	// === SCRB resource IDs (per-difficulty) ===

	/** Four level-specific main animation SCRB IDs. */
	int16 _scrbAnimIdArr[4] = {};
	/** Two Zoombini animation base SCRB IDs. */
	int16 _scrbZmbAnimIdArr[2] = {};
	/** Smoke stack A SCRB ID. */
	int16 _scrbSmokeStackResA = 0;
	/** Smoke stack B SCRB ID. */
	int16 _scrbSmokeStackResB = 0;
	/** Overlay SCRB ID. */
	int16 _scrbOverlayResId = 0;
	/** Transition SCRB ID. */
	int16 _scrbTransitionResId = 0;
	/** Travel SCRB ID. */
	int16 _scrbTravelResId = 0;
	/** Pickup SCRB ID. */
	int16 _scrbPickupResId = 0;
	/** Drop SCRB ID. */
	int16 _scrbDropResId = 0;
	/** Walk SCRB ID. */
	int16 _scrbWalkResId = 0;

	// === Runner arrays ===

	/** Cart-column runners created for placed Zoombinis and reused during departure. */
	ZmbFeature *_departureColumnRunners[20] = {};

	/** Accepted Zoombinis aligned with the placed cart-column runners. */
	ZmbSnoid *_acceptedZmbs[20] = {};

	/** Zoombini queue waiting for Smoke's next authored phase. */
	uint16 _zmbQueue[21] = {};

	/** Level 1-2 candidate-panel runner features. */
	ZmbFeature *_cliffRunners[20] = {};
	/** Number of active cliff runners. */
	int16 _cliffRunnerCount = 0;
	/** Cached state for level 1 cliff runners. */
	ZmbSmokeRunnerState _cliffRunnerStates[20];

	/** Level 2 mirrored-lane runner features. */
	ZmbFeature *_level2Runners[6] = {};
	/** Number of active level 2 runners. */
	int16 _level2RunnerCount = 0;
	/** Cached state for level 2 runners. */
	ZmbSmokeRunnerState _level2RunnerStates[6];

	/** Level 3 grid runner features. */
	ZmbFeature *_gridRunners[9] = {};
	/** Number of active grid runners; runtime setup starts at one. */
	int16 _gridRunnerCount = 0;
	/** Cached state for level 3 grid runners. */
	ZmbSmokeRunnerState _gridRunnerStates[9];

	/** Level 4 exit runner features. */
	ZmbFeature *_exitRunners[4] = {};
	/** Number of active exit runners. */
	int16 _exitRunnerCount = 0;
	/** Cached state for level 4 exit runners. */
	ZmbSmokeRunnerState _exitRunnerStates[4];

	/** Central reflection runner features. */
	ZmbFeature *_reflectionRunners[2] = {};
	/** Number of active central reflection runners. */
	int16 _reflectionRunnerCount = 0;
	/** Cached state for central reflection runners. */
	ZmbSmokeRunnerState _reflectionRunnerStates[2];

	/** Number of filters placed on the left side. */
	int16 _leftFilterCount = 0;
	/** Number of filters placed on the right side. */
	int16 _rightFilterCount = 0;

	/** Custom runner displaying the level 3/4 comparison target. */
	ZmbFeature *_targetDisplayRunner = nullptr;
	/** Custom runner displaying the level 4 pair member in transition. */
	ZmbFeature *_sourceDisplayRunner = nullptr;

	// === Display runners (L3-4 only) ===

	/** Filter runner assigned to each of the six ordered level 3/4 slots. */
	ZmbFeature *_filterSlotRunners[6] = {};

	// === Trait grid (L3-4) ===

	/** Primary trait values for the nine-row level 3/4 comparison grid. */
	ZmbTrait _primaryGridTraits[9];
	/** Secondary trait values for the nine-row level 3/4 comparison grid. */
	ZmbTrait _secondaryGridTraits[9];
	/** Per-row matching trait values for the level 3/4 comparison grid; zero denotes no match in that trait column. */
	ZmbTrait _gridMatchTraits[9];

	// === Idle animation state ===
	// Eligibility, runner selection, and completion are Smoke-owned; only the
	// fact that this state contains a timer resembles other pages.

	/** Frame at which the completion-celebration idle driver last ran. */
	uint32 _lastCelebrationIdleFrame = 0;
	/** Finite progress counter for the completion-celebration idle sequence. */
	int16 _celebrationIdleProgress = 0;
	/** Whether the completion-celebration idle sequence is active. */
	bool _bCelebrationIdleActive = false;
	/** Non-repeating random pool state for the completion-celebration idle sequence. */
	uint32 _celebrationIdlePoolState = 0;

	// === Misc state ===

	/** Whether SFX was enabled before the muted setup render. */
	bool _savedSfxEnabled = false;
};

} // End of namespace Mohawk

#endif
