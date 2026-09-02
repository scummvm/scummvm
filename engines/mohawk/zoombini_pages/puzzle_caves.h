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

#ifndef MOHAWK_ZOOMBINI_PAGES_PUZZLE_CAVES_H
#define MOHAWK_ZOOMBINI_PAGES_PUZZLE_CAVES_H

#include "mohawk/zoombini_pages/puzzle_base.h"

namespace Mohawk {

/**
 * The Lion's Lair puzzle page (@ref ZoombiniPageType::kCaves).
 * Route 4, Puzzle 1
 *
 * Zoombinis must occupy the correct lion seat based on the rule glyphs shown
 * on the upper wall.
 *
 */
class ZoombiniPuzzleCaves : public ZoombiniPuzzle {
public:
	/** Create the Lion's Lair puzzle page. */
	ZoombiniPuzzleCaves(MohawkEngine_Zoombini *vm);
	/** Release seat runners, glyph features, and page resources. */
	~ZoombiniPuzzleCaves() override;
	/** Static puzzle name used by debug answer descriptions. */
	static constexpr const char *kPageName = ZmbXferRouteInfo::kXferPageNameLionsLair;
	static constexpr int kRouteNumber = 4;
	static constexpr int kRoutePuzzleIdx = 1;
	/** Return the puzzle name used by debug answer descriptions. */
	const char *getPageName() const override { return kPageName; }
	int getRouteNumber() const override { return kRouteNumber; }
	int getRoutePuzzleIdx() const override { return kRoutePuzzleIdx; }

	/** Generate the seat rule and initialize entrance state. */
	void open() override;
	/** Select the Lion's Lair puzzle music. */
	void setBackgroundMusic() override;
	/** Select the Lion's Lair background bitmap. */
	void setBackgroundBitmap() override;
	/** Initialize puzzle states and departure transfer source. */
	void initStates() override;
	/** Load entrances, doors, glyphs, seats, and Zoombini features. */
	void loadFeatures() override;
	/** Select the fixed Lion's Lair F1 prompt retained at every difficulty. */
	void initHelpPrompt() override;
	/** Restart the global Snoid fidget inactivity period after Lion's Lair loads. */
	bool requiresFidgetActivityResetOnLoad() const override { return true; }

	/** Return the original Lion's Lair script-sound range priority. */
	const ZoombiniPage::ScriptSoundPriorityRanges &getScriptSoundPriorityRanges() const override;
	/** Classify the lion's authored growl separately from other Lion's Lair effects. */
	Audio::Mixer::SoundType getFeatureSoundType(const ZmbFeature *feature, ZmbResource soundRes) const override;
	/** Prepare Caves-specific dirty coverage before the 60 TPS render pass. */
	void onAnimFrame() override;
	/** Advance entrance, seat, and rejection animations. */
	void onEveryFrame() override;
	/** Advance the authored Lion's Lair controller after rendering. */
	void onPostRenderFrame() override;
	/** Process door, glyph, and entrance animation callbacks. */
	void onFeatureAnimEvent(ZmbFeature *feature, int16 eventCode) override;

	/** Start a drag or handle a seat/door control click. */
	ZmbEventHandleResult onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) override;

protected:
	/** Lion's Lair actions accepted by the built-in debug console and keyboard dispatcher. */
	enum class BuiltinDebugAction {
		kInvalid,
		kDifficulty,
		kReinit,
		kReinitAfterExit,
		kLevel,
		kEntrance
	};
	static constexpr const char *kBuiltinDebugActionDifficulty = "difficulty";
	static constexpr const char *kBuiltinDebugActionReinit = "reinit";
	static constexpr const char *kBuiltinDebugActionReinitAfterExit = "reinit-after-exit";
	static constexpr const char *kBuiltinDebugActionLevel = "level";
	static constexpr const char *kBuiltinDebugActionEntrance = "entrance";
	/** Allow the authored ambient poll on the frame that starts departure. */
	bool runsAmbientDuringDeparture() const override { return true; }
	/** Begin departure after all required seats are resolved. */
	void onGoButtonActivated() override;
	/** Prepare every runner in a solved seat layout for the debug finish motion. */
	void debugPrepareForDeparture() override;
	/** Describe the generated glyph rule for diagnostics. */
	Common::String debugGetAnswer() const override;
	/** Describe the Lion's Lair built-in debug actions. */
	Common::String debugGetBuiltinDebugCommandHelp() const override;
	/** Run one Lion's Lair built-in debug action. */
	bool debugDoBuiltinDebugCommand(int argc, const char **argv, Common::String &output) override;
	/** Replay the F1 sound only on the three supported difficulties. */
	bool debugReplayActiveHelpSound() override;
	/** Handle Lion's Lair built-in debug keys. */
	ZmbEventHandleResult onDebugKeyDown(const Common::KeyState &kbd) override;
	/** Parse one console-facing Lion's Lair action name. */
	static BuiltinDebugAction parseBuiltinDebugAction(const Common::String &action);
	/** Run one typed Lion's Lair built-in debug action. */
	bool runBuiltinDebugAction(BuiltinDebugAction action, int16 argument, Common::String &output);
	/** Report Lion's Lair mistake-budget usage for diagnostics. */
	ZmbChanceInfo debugGetChances() const override;
	/** Lion's Lair supports debugger chance adjustment. */
	bool debugCanSetChances() const override { return true; }
	/** Set Lion's Lair mistake-budget usage for diagnostics. */
	bool debugSetChances(int16 remaining) override;

private:
	/** Page-local Lion's Lair backgrounds, glyphs, doors, and animations. */
	enum PageResourceId : int16 {
		kResBackground5000 = 5000,

		// Terrain bitmaps
		kResBitmapTerrain100 = 100,

		// Shape bitmaps
		kResBitmapShape6000_Entrance = 6000,
		kResBitmapShape7000_Doors = 7000,
		kResBitmapShape8200_GlyphPanel = 8200,
		kResBitmapShape9000_Overlays = 9000,
		kResBitmapShape9025_SeatMask = 9025,
		kResBitmapShape10000_Glyphs = 10000,
		kResBitmapShape11000_Snoid = 11000,

		kResNode1000_WalkNetwork = 1000,

		kResRegs0201_GlyphAdjustments = 201,

		kResScrb6000_EntranceBase = 6000,
		kResScrb6001_EntranceSecond = 6001,
		kResScrb6002_EntranceFinal = 6002,
		kResScrb6003_GlyphPanelBase = 6003,
		kResScrb6004_GlyphPanelLevel2 = 6004,
		kResScrb6005_GlyphPanelLevel3 = 6005,
		kResScrb6006_GlyphPanelLevel4 = 6006,
		kResScrb6012_GlyphPanelOverlay = 6012,
		kResScrb7000_DoorBase = 7000,
		kResScrb7004_DoorGlyphBase = 7004,
		kResScrb7011_DoorGlyphBase = 7011,
		kResScrb7015_DoorGlyphBase = 7015,
		kResScrb8200_GlyphDoorBase = 8200,
		kResScrb9000_OverlayBase = 9000,
		kResScrb9004_GlyphOverlayBase = 9004,
		kResScrb9014_DoorPanelBase = 9014,
		kResScrb9015_GlyphOverlayBase = 9015,
		kResScrb9024_SeatMaskBase = 9024,
		kResScrb9025_SeatMaskBase = 9025,

		kResScrs12000_RejectBase = 12000,
		kResScrs12004_RejectFirstPlayable = 12004,
		kResScrs12012_DoorCloseWalkBack = 12012,
		kResScrs12999_WalkBase = 12999,
		kResScrs13000_NormalBase = 13000,

		kResSound6006_LionGrowl = 6006,

		// Sound ranges used by getScriptSoundPriority().
		kResSoundRange0425_PageRangeBase = 425,
		kResSoundRange0499_PageRangeLast = 499,
		kResSoundRange0600_PageRangeBase = 600,
		kResSoundRange0799_PageRangeLast = 799,
		kResSoundRange6000_Priority = 6000,
		kResSoundRange6001_Priority = 6001,
		kResSoundRange6002_Priority = 6002,
		kResSoundRange6003_Priority = 6003,
		kResSoundRange6004_Priority = 6004,
		kResSoundRange6005_Priority = 6005,
		kResSoundRange6006_Priority = 6006,
		kResSoundRange6007_Priority = 6007,
		kResSoundRange6008_Priority = 6008,
		kResSoundRange8200_PageRangeBase = 8200,
		kResSoundRange12001_PageRangeLast = 12001,

		kResMidi30025_CavesBgmBase = 30025,
	};

	/**
	 * Immutable waiting-area rectangles are instance members because Common::Rect
	 * requires runtime construction and ScummVM prohibits global C++ constructors.
	 * The eleven rectangles use zero-based array indices 0-10 and together form one stepped hit region.
	 */
	const Common::Rect _waitingAreaRects[11] = {
		Common::Rect(0, 0, 195, 130),
		Common::Rect(0, 128, 175, 147),
		Common::Rect(0, 146, 155, 165),
		Common::Rect(0, 164, 135, 191),
		Common::Rect(0, 190, 120, 214),
		Common::Rect(0, 213, 100, 236),
		Common::Rect(0, 235, 87, 250),
		Common::Rect(0, 249, 67, 269),
		Common::Rect(0, 268, 40, 289),
		Common::Rect(0, 288, 27, 357),
		Common::Rect(0, 356, 36, 394),
	};

	/** Door SCRB callbacks that start, finish, or chain the entrance animation. */
	enum FloorDoorEventCode : int16 {
		kFloorDoorEventCode001_StartRejectEntrance = 1,
		kFloorDoorEventCode002_StartCorrectEntrance = 2,
		kFloorDoorEventCode004_FinishDoorTransition = 4,
		kFloorDoorEventCode005_ArmPageTransition = 5,
		kFloorDoorEventCode010_StartFloorBounce = 10,
		kFloorDoorEventCode020_CheckEntranceCompletion = 20,
		kFloorDoorEventCode021_ForceEntranceCompletion = 21
	};

	/** Glyph-panel SCRB callbacks that advance the panel state. */
	enum GlyphEventCode : int16 {
		kGlyphEventCode010_StartAnimation = 10,
		kGlyphEventCode020_CheckCompletion = 20,
		kGlyphEventCode021_ForceCompletion = 21
	};

	/**
	 * Initialize the mistake limit and ledge-controller SCRB for the current difficulty.
	 * Also resets the level-1 clue-attention sequence.
	 */
	void initDifficultyParams();
	/** Reset the current difficulty controller and its ledge runner. */
	void debugReinitDifficulty();
	/** Return every loaded Zoombini to the waiting area and reset the difficulty controller. */
	void debugReinitAfterExit();
	/**
	 * Rebuild the generated rule and seat every loaded Zoombini for one primary trait kind.
	 * @param traitKindIndex Zero-based @ref ZmbTrait index in the range 0-3.
	 */
	void debugProcessEntrance(int16 traitKindIndex);

	/**
	 * Set up the sorting-rule glyph patterns.
	 * Selects the rule traits, counts the active pack, assigns rule values to seats, and selects visible clues.
	 */
	void setupEntranceGlyphs();

	/** Select the rule trait kinds and shuffle their five trait values. */
	void initEntranceTraitPattern();

	/** Count primary rule buckets and, for levels 3/4, primary-secondary bucket pairs. */
	void countGlyphDistribution();

	/** Assign the counted primary and secondary rule values to the usable seats. */
	void buildGlyphTimingTable();

	/** Select the visible rule clues and map their trait values to glyph shape IDs. */
	void distributeEntranceTraits();

	/**
	 * Find which lion seat matches a Zoombini's traits.
	 * The dropped seat wins when valid; otherwise randomly select an empty match.
	 * @param droppedSeatNumber Preferred one-based seat number in the range 1-20.
	 * @return Matching one-based seat number in the range 1-20.
	 * Seat 1 is the fallback sentinel when the generated rule has no empty match.
	 */
	int16 findMatchingSeatNumber(const ZmbTrait &traits, int16 droppedSeatNumber);

	/**
	 * Process a correct lion-seat placement.
	 * @param seatNumber One-based matching seat number in the range 1-20.
	 * @param queueFinalWalkIn Queue the final Snoid when it has not already been dropped at its seat.
	 */
	void handleCorrectPlacement(ZmbSnoid *snoid, int16 seatNumber, bool queueFinalWalkIn = false);

	/**
	 * Scans seats 20 through 1, skips empty entries, and animates up to 3 actual occupants toward the requested point.
	 * @p staggerFrames controls the delay between occupants.
	 * @p destPos is the target position for the staggered walk.
	 * Locks UI drag during the celebration sequence.
	 */
	void triggerSuccessAnim(int16 staggerFrames, Common::Point destPos);

	/**
	 * Process a wrong lion-seat placement by redirecting the Snoid to its matching seat.
	 * @param droppedSeatNumber One-based dropped seat number in the range 1-20.
	 * @param matchingSeatNumber One-based matching seat number in the range 1-20.
	 */
	void handleWrongPlacement(ZmbSnoid *snoid, int16 droppedSeatNumber, int16 matchingSeatNumber);

	/** End drag and evaluate drop target. */
	void endDrag(const Common::Point &dropPos) override;

	/** Return whether a point lies in the Caves waiting area. */
	bool isInWaitingArea(const Common::Point &pos) const;

	/**
	 * Custom render callback for the virtual glyph renderer feature.
	 * Draw the visible sorting-rule glyphs on the upper wall.
	 */
	ZmbRenderResult renderEntranceGlyphs(ZmbFeature *feature);

	/**
	 * Play a seat-transfer SCRS on the active transfer Snoid.
	 * @param isReject Whether the script hides the Snoid on completion.
	 * @param scrsResId Page SCRS resource ID to play.
	 */
	void playEntranceScript(bool isReject, int16 scrsResId);

	/**
	 * Load one difficulty-relative feedback SCRB onto the ledge controller.
	 * @param feedbackStep Zero-based completed-mistake step in the range 0 through @ref ZoombiniPuzzleCaves::_mistakeLimit.
	 * @param replaceActive Whether to replace active ledge feedback.
	 */
	void loadGlyphPanelFrame(int16 feedbackStep, bool replaceActive = false);

	/**
	 * Set up one phase of a wrong-seat redirect or floor-return sequence.
	 *
	 * @param transferPhase Zero-based phase code in the range 0-2:
	 * 0 opens the selected entrance for the drop target.
	 * 1 opens the matching entrance after a redirect.
	 * 2 starts the floor-return script after an out-of-zone drop.
	 */
	void setupDoorAnimation(int16 transferPhase);

	/**
	 * Register the no-op Z-order anchor for one seat.
	 * The anchor carries SCRB 6000 with a zero frame interval and draws nothing.
	 * It exists only to hold a stable list position between the seat's entrance overlay and its
	 * DRAW_ON_REG door runner, so a seated Zoombini can be linked at that exact depth.
	 *
	 * @param seatNumber One-based anchor number in the range 5-21.
	 * Number 21 is the trailing Z-order sentinel after seat 20.
	 */
	void registerSeatZOrderAnchor(int16 seatNumber);

	/** Pre-render gate for a seat Z-order anchor. Always skips animation. */
	bool seatAnchor_preRender(ZmbFeature *feature);
	/** Render callback for a seat Z-order anchor. Always draws nothing. */
	ZmbRenderResult seatAnchor_render(ZmbFeature *feature);

	/** Place a seated Zoombini at the fixed Z-order for one-based seat 1-20. */
	void linkSnoidToSeatZOrder(ZmbSnoid *snoid, int16 seatNumber);

	/**
	 * Get the persistent overlay feature for a one-based seat number.
	 *
	 * @param seatNumber One-based seat number in the range 1-20.
	 * @return Overlay feature for seats 5-20, or nullptr for seats 1-4 and out-of-range values.
	 */
	ZmbFeature *getEntranceOverlayFeature(int16 seatNumber) const;

	/**
	 * Handle events from seat-transfer and floor-return animations.
	 * Events 1/2 start SCRS playback, 4 finalizes a redirect, 5 arms its matching-seat phase,
	 * 10 starts the floor return, and 20/21 release or retain the drag gate.
	 */
	void handleEntranceDoorEvent(ZmbFeature *feature, int16 eventCode);

	/**
	 * Handle events from the ledge controller and final lion animation.
	 * Events: 10(phase change), 20(completion check), 21(force completion)
	 */
	void handleGlyphPanelEvent(ZmbFeature *feature, int16 eventCode);

	/**
	 * Lion's Lair keeps separate pack and authored-seat index domains.
	 * The current page materializes N pack Snoids, where 1 <= N <= 16, and assigns zero-based waiting-position indices 0 through N - 1.
	 * The background and resource layout has twenty one-based seats numbered 1-20.
	 * Only the trailing N seats, numbered 21 - N through 20, are usable; a full group therefore uses seats 5-20.
	 * A twenty-entry table is consequently not evidence that the current page accepts twenty active Snoids.
	 */

	/**
	 * Twenty authored waiting-area positions indexed from 0 through 19.
	 * The current active group consumes entries 0 through N - 1 for 1 <= N <= 16, so entries 16-19 remain unused.
	 */
	static constexpr Common::Point kWaitingSnoidPositions[20]{
		Common::Point(180, 110),
		Common::Point(160, 136),
		Common::Point(130, 167),
		Common::Point(106, 193),
		Common::Point(86, 232),
		Common::Point(140, 100),
		Common::Point(120, 126),
		Common::Point(100, 157),
		Common::Point(76, 183),
		Common::Point(46, 222),
		Common::Point(100, 90),
		Common::Point(80, 116),
		Common::Point(60, 147),
		Common::Point(36, 173),
		Common::Point(60, 80),
		Common::Point(40, 106),
		Common::Point(20, 137),
		Common::Point(10, 167),
		Common::Point(20, 90),
		Common::Point(20, 116),
	};

	/**
	 * Fixed positions for one-based lion seats 1-20.
	 * Seat number S uses zero-based array entry S - 1 and DRAW_ON_REG slot S - 1.
	 */
	static constexpr Common::Point kSeatEntrancePositions[20]{
		Common::Point(254, 140),
		Common::Point(296, 148),
		Common::Point(340, 146),
		Common::Point(373, 163),
		Common::Point(364, 187),
		Common::Point(337, 212),
		Common::Point(316, 234),
		Common::Point(301, 263),
		Common::Point(314, 292),
		Common::Point(346, 311),
		Common::Point(388, 316),
		Common::Point(429, 301),
		Common::Point(458, 281),
		Common::Point(482, 261),
		Common::Point(521, 247),
		Common::Point(556, 263),
		Common::Point(567, 290),
		Common::Point(543, 314),
		Common::Point(529, 342),
		Common::Point(554, 359),
	};

	/**
	 * Final facing direction for accepted Snoids at one-based seats 1-20.
	 * Seat number S uses zero-based array entry S - 1.
	 */
	static constexpr bool kSeatFacingLeft[20]{
		false,
		false,
		false,
		false,
		true,
		true,
		true,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		true,
		true,
		true,
		false,
	};

	/** Screen X centers for one-based rule-clue slots 1-10; zero-based array index 0 is unused. */
	static constexpr int16 kRuleGlyphScreenX[11]{
		0,
		326,
		348,
		375,
		397,
		423,
		324,
		347,
		373,
		395,
		422,
	};

	/** Screen Y positions for one-based rule-clue slots 1-10; zero-based array index 0 is unused. */
	static constexpr int16 kRuleGlyphScreenY[11]{
		0,
		36,
		39,
		42,
		44,
		46,
		77,
		80,
		83,
		86,
		90,
	};

	// Glyph system state ---

	/** Wrong-seat transfers allowed before the ledge response completes (4-7 by difficulty). */
	int16 _mistakeLimit = 4;
	/** Difficulty-selected SCRB base for ledge feedback frames (6006 through 6003). */
	int16 _ledgeControllerScrbBaseId = kResScrb6006_GlyphPanelLevel4;
	/** Visibility flags for one-based rule-clue slots 1-10; zero-based array index 0 is unused. */
	byte _ruleGlyphVisibility[11] = {};
	/** One-based tBMP 10000 shape numbers for one-based clue slots 1-10; zero-based array index 0 is unused. */
	byte _ruleGlyphShapeIds[11] = {};
	/** One-based clue slot hidden by the level-1 attention sequence: 0 means none and 6 is the terminal sentinel. */
	int16 _glyphBlinkHiddenSlot = 0;
	/** REGS 201 vertical adjustments for one-based rule-clue slots 1-10; zero-based array index 0 is unused. */
	int16 _ruleGlyphYAdjustments[11] = {};

	// Extended glyph system state ---

	/** Number of active rule dimensions: one at levels 1/2 and two at levels 3/4. */
	int16 _ruleTraitCount = 1;
	/** Number of shuffled values in each rule-trait row. */
	int16 _ruleValueCount = 5;
	/** Trait kind used by the primary seat rule. */
	ZmbTrait::TraitKind _primaryRuleTraitKind = ZmbTrait::kTraitHair;
	/** Trait kind used by the secondary seat rule at levels 3/4. */
	ZmbTrait::TraitKind _secondaryRuleTraitKind = ZmbTrait::kTraitHair;
	/** Two zero-based rows of five shuffled trait values: indices 0-4 are primary and 5-9 are secondary. */
	int16 _ruleTraitValues[10] = {};
	/** Six-by-six counts at zero-based indices 0-35, addressed as `6 * secondaryValue + primaryValue` for trait values 0-5. */
	int16 _ruleBucketCounts[36] = {};
	/** Primary rule value assigned to one-based seats 1-20; zero-based array index 0 is unused. */
	int16 _seatPrimaryRuleValues[21] = {};
	/** Secondary rule value assigned to one-based seats 1-20; zero-based array index 0 is unused. */
	int16 _seatSecondaryRuleValues[21] = {};

	/**
	 * Occupant at each one-based seat 1-20, or nullptr for an empty seat; zero-based array index 0 is unused.
	 * @ref ZoombiniPuzzleCaves::findMatchingSeatNumber() skips occupied seats.
	 * @ref ZoombiniPuzzleCaves::triggerSuccessAnim() walks the placed Snoids out.
	 */
	ZmbSnoid *_seatOccupants[21] = {};
	/** First usable one-based seat, computed as 21 - N and therefore in the range 5-20 for the current 1-16 Snoid group. */
	int16 _firstUsableSeatNumber = 1;
	/** Number of leading zero-based DRAW_ON_REG slots covered by SCRB 9025-9028, in the range 0-4. */
	int16 _maskedLeadingSeatCount = 0;

	/** Lion SCRB 6000, 6001, and 6002 runners at zero-based indices 0-2. */
	ZmbFeature *_lionEntranceFeatures[3] = {};
	/** SCRB 9025-9028 overlay that covers unused leading background seats. */
	ZmbFeature *_leadingSeatMaskFeature = nullptr;
	/** SCRB 7000-7019 DRAW_ON_REG runners for one-based seats 1-20, stored at zero-based index seatNumber - 1. */
	ZmbFeature *_seatDoorFeatures[20] = {};
	/** Persistent SCRB 9011-9014 overlays for one-based seats 12-15, stored at zero-based indices 0-3. */
	ZmbFeature *_middleSeatOverlayFeatures[4] = {};
	/** Persistent SCRB 9004-9010 overlays for one-based seats 5-11, stored at zero-based indices 0-6. */
	ZmbFeature *_upperSeatOverlayFeatures[7] = {};
	/** Persistent SCRB 9015-9019 overlays for one-based seats 16-20, stored at zero-based indices 0-4. */
	ZmbFeature *_lowerSeatOverlayFeatures[5] = {};
	/**
	 * No-op Z-order anchors addressed directly by one-based number; array index 0 is unused.
	 * Seats 5-20 receive one anchor each, registered immediately after that seat's persistent
	 * overlay and immediately before its DRAW_ON_REG door runner.
	 * Index 21 holds the trailing sentinel after seat 20.
	 */
	ZmbFeature *_seatZOrderAnchors[22] = {};
	/** Difficulty-selected REGION_TRACK runner that controls ledge feedback. */
	ZmbFeature *_ledgeControllerFeature = nullptr;
	/**
	 * Virtual glyph renderer feature.
	 * Renders all entrance glyphs through @ref ZoombiniPuzzleCaves::renderEntranceGlyphs().
	 */
	ZmbFeature *_ruleGlyphRendererFeature = nullptr;

	// Entrance callback state ---

	/** Base of the wrong-seat SCRS pair: reject 12004 followed by redirect 12005. */
	int16 _seatTransferScrsBaseId = kResScrs12004_RejectFirstPlayable;
	/** Base of the four-SCRB transfer group for each one-based seat. */
	int16 _seatTransferScrbBaseId = kResScrb8200_GlyphDoorBase;
	/** Number of completed wrong-seat transfers. */
	int16 _mistakeCount = 0;
	/** Transient one-shot SCRB 82xx overlay for the active transfer phase. */
	ZmbFeature *_activeTransferOverlayFeature = nullptr;
	/** Persistent SCRB 90xx overlay for the dropped seat. */
	ZmbFeature *_droppedSeatOverlayFeature = nullptr;
	/** Persistent SCRB 90xx overlay for the matching seat. */
	ZmbFeature *_matchingSeatOverlayFeature = nullptr;
	/** Event-5 latch that starts the matching-seat transfer phase on the next post-render tick. */
	bool _matchingSeatTransferPending = false;
	/** Whether the player may start dragging another Snoid from the waiting area. */
	bool _snoidDragEnabled = true;
	/** Event-4 latch that restores persistent overlays and final facing on the next post-render tick. */
	bool _seatTransferFinalizePending = false;
	/** Ordered departure states driven by the Go action and glyph-panel callbacks. */
	enum class DeparturePhase : byte {
		/** No departure animation is active. */
		kIdle00 = 0,
		/** The Go action loaded the entrance-final SCRB. */
		kEntranceFinal01 = 1,
		/** Glyph event 10 armed the success animation. */
		kSuccessPending02 = 2,
		/** The success animation began normal puzzle departure. */
		kDeparting03 = 3
	};
	/** Current phase of the Go-driven lion and departure sequence. */
	DeparturePhase _departurePhase = DeparturePhase::kIdle00;
	/** Snoid owned by the active seat-transfer or floor-return sequence. */
	ZmbSnoid *_activeTransferSnoid = nullptr;
	/** One-based seat number where the Snoid was dropped, in the range 1-20; zero means none. */
	int16 _droppedSeatNumber = 0;
	/** One-based seat number that matches the Snoid's rule traits, in the range 1-20; zero means none. */
	int16 _matchingSeatNumber = 0;
	/**
	 * Wrong-placement latch that starts the dropped-seat transfer phase on the next post-render tick.
	 */
	bool _wrongSeatTransferPending = false;
	/** Whether the post-completion N-1 random walk-in driver is running. */
	bool _completionWalkInsActive = false;
	/** Go-button latch that reloads and raises lion SCRB 6002 on the next post-render tick. */
	bool _goLionSequencePending = false;
	/** Whether Go is enabled after the first accepted placement. */
	bool _goButtonEnabled = false;
	/**
	 * Number of Zoombinis placed (correct or redirected).
	 * This is Caves progression state, not a generic placed-Snoid counter.
	 * The count ranges from 0 through N, where the current page has 1 <= N <= 16.
	 */
	int16 _acceptedSnoidCount = 0;
	/**
	 * LIFO stack of Snoids queued for walk-in animation after correct placement.
	 * @ref ZoombiniPuzzleCaves::onPostRenderFrame() drains the entire stack each tick,
	 * allowing clustered placements to start their walk-in animations together.
	 */
	struct WalkInEntry {
		/** Snoid scheduled for the grouped walk-in. */
		ZmbSnoid *snoid;
		/** SCRS resource used by the walk-in. */
		int16 scrsId;
		/** Target position for the walk-in. */
		Common::Point targetPos;
	};
	/** Twenty-entry storage for accepted walk-ins; the current page uses zero-based entries 0 through N - 1 for N <= 16. */
	WalkInEntry _acceptedWalkStack[20] = {};
	/** Number of valid LIFO entries in @ref _acceptedWalkStack, in the range 0 through N for N <= 16. */
	int16 _acceptedWalkStackSize = 0;

	/** Number of post-completion random walk-in scripts successfully started, in the range 0 through N - 1. */
	int16 _completionWalkInStartedCount = 0;
	/**
	 * Number of random walk-ins to schedule after multi-Snoid completion.
	 * The final correct placement skips its individual queue entry and starts this N-1 group instead.
	 * The current 1-16 Snoid page therefore stores a value in the range 0-15.
	 */
	int16 _completionWalkInTargetCount = 0;
	/** Frame when the most recent post-completion random walk-in was started. */
	uint32 _completionWalkLastStartFrame = 0;
	/** Non-repeat random-pool state for the post-completion Snoid picker. */
	uint32 _completionWalkPoolState = 0;

	/** Optional target supplied to the active seat-transfer or floor-return SCRS. */
	Common::Point _entranceScriptTarget;
	/** Whether @ref _entranceScriptTarget contains a valid script anchor. */
	bool _hasEntranceScriptTarget = false;
	/** Pickup position targeted by SCRS 12013 after an out-of-zone drop. */
	Common::Point _floorReturnPosition;

	/** Last one-based clue slot selected by the level-1 attention sequence, from 0 through terminal sentinel 6. */
	int16 _glyphBlinkSequenceIndex = 0;

	/** Frame when the level-1 clue-attention sequence next toggles visibility. */
	uint32 _nextGlyphBlinkFrame = 0;
};

} // End of namespace Mohawk

#endif
