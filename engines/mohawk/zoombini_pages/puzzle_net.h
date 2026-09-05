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

#ifndef MOHAWK_ZOOMBINI_PAGES_PUZZLE_NET_H
#define MOHAWK_ZOOMBINI_PAGES_PUZZLE_NET_H

#include "mohawk/zoombini_pages/puzzle_base.h"

namespace Mohawk {

/**
 * Mudball Wall puzzle page (@ref ZoombiniPageType::kNet).
 * Route 3, Puzzle 3
 *
 */
class ZoombiniPuzzleNet : public ZoombiniPuzzle {
public:
	/** Create the Mudball Wall puzzle page. */
	ZoombiniPuzzleNet(MohawkEngine_Zoombini *vm);
	/** Release columns, slots, and Zoombini features. */
	~ZoombiniPuzzleNet() override;
	/** Static puzzle name used by debug answer descriptions. */
	static constexpr const char *kPageName = ZmbXferRouteInfo::kXferPageNameMudballWall;
	static constexpr int kRouteNumber = 3;
	static constexpr int kRoutePuzzleIdx = 3;
	/** Return the puzzle name used by debug answer descriptions. */
	const char *getPageName() const override { return kPageName; }
	int getRouteNumber() const override { return kRouteNumber; }
	int getRoutePuzzleIdx() const override { return kRoutePuzzleIdx; }

	/** Generate wall-axis rules and initialize the active pack. */
	void open() override;
	/** Select the Mudball Wall puzzle music. */
	void setBackgroundMusic() override;
	/** Select the difficulty-specific Mudball Wall background. */
	void setBackgroundBitmap() override;
	/** Initialize puzzle states and departure transfer source. */
	void initStates() override;
	/** Load entry, column, slot, and feedback features. */
	void loadFeatures() override;
	/** Select the fixed Mudball Wall F1 replay prompt after wall construction. */
	void initHelpPrompt() override;
	/** Return the original Mudball Wall script-sound range priority. */
	const ZoombiniPage::ScriptSoundPriorityRanges &getScriptSoundPriorityRanges() const override;

protected:
	/** Mudball Wall actions accepted by the built-in debug console and keyboard dispatcher. */
	enum class BuiltinDebugAction {
		kInvalid,
		kAutoSort,
		kOverlay
	};
	static constexpr const char *kBuiltinDebugActionAutoSort = "auto-sort";
	static constexpr const char *kBuiltinDebugActionOverlay = "overlay";
	/** Apply the page's color-assist remap to slot visuals. */
	ZoombiniGraphics::PaletteRemapMode getColorAssistPaletteRemap(const ZmbFeature *feature, const ZmbHotspot &hotspot, ZmbResource resource) const override;
	/** Return to the map after serializing the current net state. */
	void onMapButtonActivated() override;
	/** Depart accepted column occupants after the wall is solved. */
	void onGoButtonActivated() override;
	/** Move every active runner to the accepted exit side for debug Go handling. */
	void debugPrepareForDeparture() override;
	/** Describe the generated column rules for diagnostics. */
	Common::String debugGetAnswer() const override;
	/** Describe the Mudball Wall built-in debug actions. */
	Common::String debugGetBuiltinDebugCommandHelp() const override;
	/** Run one Mudball Wall built-in debug action. */
	bool debugDoBuiltinDebugCommand(int argc, const char **argv, Common::String &output) override;
	/** Handle Mudball Wall built-in debug keys. */
	ZmbEventHandleResult onDebugKeyDown(const Common::KeyState &kbd) override;
	/** Parse one console-facing Mudball Wall action name. */
	static BuiltinDebugAction parseBuiltinDebugAction(const Common::String &action);
	/** Run one typed Mudball Wall built-in debug action. */
	bool runBuiltinDebugAction(BuiltinDebugAction action, Common::String &output);
	/** Report submit-based chance usage for diagnostics. */
	ZmbChanceInfo debugGetChances() const override;
	/** Mudball Wall supports debugger chance adjustment. */
	bool debugCanSetChances() const override { return true; }
	/** Set submit-based chance usage for diagnostics. */
	bool debugSetChances(int16 remaining) override;
	/** Process Snoid and slot animation callbacks. */
	void onFeatureAnimEvent(ZmbFeature *feature, int16 eventCode) override;
	/** Advance column assignment and feedback after the current render. */
	void onPostRenderFrame() override;
	/** Start a Zoombini drag or handle a net control click. */
	ZmbEventHandleResult onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) override;
	/** Update entry/slot hover from pointer movement. */
	ZmbEventHandleResult onMouseMove(const Common::Point &absPos, const Common::Point &relPos) override;

private:
	/** Column-origin variants for the two authored sort-animation SCRB families. */
	enum class SortAnimationVariant : int16 {
		/** Center columns use the base sort variants. */
		kCenter00 = 0,
		/** The leftmost column uses the left sort variants. */
		kLeft01 = 1,
		/** The rightmost column uses the right sort variants. */
		kRight02 = 2
	};
	/** Resource IDs owned by the Mudball Wall page. */
	/** Page-local backgrounds, columns, slots, and animations are grouped here. */
	enum PageResourceId : int16 {
		kResBackground5000_Normal = 5000,
		kResBackground5001_Hard = 5001,

		kResBitmapShape6000_Snoid = 6000,
		kResBitmapShape7000_Entry = 7000,
		kResBitmapShape8000_Column = 8000,
		kResBitmapShape9000_Slot = 9000,
		kResBitmapShape10000_Feedback = 10000,

		kResRegs7000_Entry = 7000,
		kResRegs9000_Slot = 9000,

		kResScrb7000_ExitBase = 7000,
		kResScrb7018_TraitBase = 7018,
		kResScrb7019_TraitBase = 7019,
		kResScrb7020_SortBase = 7020,
		kResScrb7023_SlotDisplayEasy = 7023,
		kResScrb7024_SlotDisplayHard = 7024,
		kResScrb7025_TraitBase = 7025,
		kResScrb7026_TraitBase = 7026,
		kResScrb7027_TraitBase = 7027,
		kResScrb7028_SortDirectBase = 7028,
		kResScrb7031_ExitBase = 7031,
		kResScrb8000_ColumnBase = 8000,
		kResScrb8005_EntryBase = 8005,
		kResScrb9000_SlotBase = 9000,
		kResScrb9151_LabelEasy = 9151,
		kResScrb9153_LabelHard = 9153,
		kResScrb10000_FeedbackBase = 10000,
		kResScrb10001_FeedbackReject = 10001,
		kResScrb10002_TraitColumn0Base = 10002,
		kResScrb10007_TraitColumn1Base = 10007,
		kResScrb10012_TraitColumn2Base = 10012,
		kResScrb10017_FeedbackPartial = 10017,
		kResScrb10018_FeedbackComplete = 10018,
		/** v2.0US only */
		kResScrb10019_TraitColumn0HoverBase = 10019,
		/** v2.0US only */
		kResScrb10024_TraitColumn1HoverBase = 10024,
		/** v2.0US only */
		kResScrb10029_TraitColumn2HoverBase = 10029,
		/** v2.0US only */
		kResScrb10034_SubmitHover = 10034,

		kResScrs13000_NormalBase = 13000,
		kResScrs13001_NormalWalkBase = 13001,
		kResScrs13016_NormalSeatBase = 13016,
		kResScrs13031_NormalLaunchBase = 13031,
		kResScrs13046_NormalIdleBase = 13046,
		kResScrs14000_EntryBase = 14000,

		// Sound ranges used by getScriptSoundPriority().
		kResSoundRange0300_PageRangeBase = 300,
		kResSoundRange0324_PageRangeLast = 324,
		kResSoundRange0425_PageRangeBase = 425,
		kResSoundRange0499_PageRangeLast = 499,
		kResSoundRange7000_PageRangeBase = 7000,
		kResSoundRange7999_PageRangeLast = 7999,
		kResSoundRange8000_PagePriorityBase = 8000,
		kResSoundRange8002_PagePriorityLast = 8002,
		kResSoundRange9000_PageRangeBase = 9000,
		kResSoundRange10000_PagePriorityBase = 10000,
		kResSoundRange10099_PagePriorityLast = 10099,
		kResSoundRange10999_PageRangeLast = 10999,
	};

	/** Normal exit-runner interval in 60 Hz frames. */
	static constexpr uint32 kMudTankNormalFrameInterval = 6;
	/** Accelerated interval used only by debugger chance adjustments. */
	static constexpr uint32 kMudTankDebugFrameInterval = 1;
	/** Convert remaining launch budget to the displayed mud-tank fill level. */
	static int16 getMudTankTargetLevel(int16 remainingShotAllowance);

	/** Snoid and routed SCRB callbacks use the same event values. */
	enum NetEventCode : int16 {
		kNetEventCode000_ToggleFacing = 0,
		kNetEventCode002_SpawnSnoidSlot = 2,
		kNetEventCode004_StartColumnTravel = 4,
		kNetEventCode020_StartColumnPositioning = 20,
		kNetEventCode030_StartColumnExit = 30
	};

	/** Register the column, entry, and slot feature runners. */
	void registerColumnRunners();
	/**
	 * Start one tracked mud-tank animation toward @p destinationLevel.
	 * @param accelerated Whether to use the debugger-only accelerated interval.
	 */
	void startMudTankVisualAnimation(int16 scrbId, int16 destinationLevel, bool accelerated);
	/** Commit a tracked mud-tank animation after its final frame. */
	void finishMudTankVisualAnimation();
	/** Freeze the mud-tank feature on the normal settled frame for the displayed level. */
	void settleMudTankVisualFrame();
	/** Append one chance-adjustment step when the mud-tank feature is available. */
	void updateChanceVisualAnimation();

	// --- Core puzzle logic ---

	/** Generate axis-rule grids and target assignments. */
	void generateTraitRules();
	/** Partition the materialized active pack into target launch batches. */
	void computeTargetLaunchBatches();
	/** Find the wall cell selected by the current control values. */
	int16 findTargetSlotForSelection();
	/** Apply one selector value or submit the current selection. */
	void updateSelectorValue(int16 selectorValue, int16 selectorGroup);
	/** Assign the next Snoid to an available staging column. */
	void assignNextSnoidToColumn();
	/** Settle the current fired-shot feature at a wall cell and commit its result. */
	void settleShotAtTargetSlot(int16 slotIndex);
	/** Start the fired-shot feature toward a wall cell. */
	void startShotAtTargetSlot(int16 slotIndex);
	/** Release selector input after both trait and accepted-launch owners have completed. */
	void releaseSelectorGateAfterTraitAnimation();

	// --- Animation event dispatch ---
	// @ref ZoombiniPuzzleNet::onFeatureAnimEvent() routes Snoid and SCRB events to the handlers below.
	// ASCII-event traversal belongs to the Maze page and is not used here.

	/** Zoombini snoid animation events. */
	void processSnoidAnimEvent(ZmbFeature *feature, int16 eventCode);
	/** Events from SCRB features. */
	void processZmbScrbAnimEvent(ZmbFeature *feature, int16 eventCode);
	/** Flip snoid facing for NET callback event 0. */
	void flipEventFacing(ZmbFeature *feature);
	/** Start a NET NORMAL SCRS, optionally ending at the requested anchor. */
	bool startVisibleNormalScrs(ZmbSnoid *snoid, int16 scrsId, const Common::Point *endPos = nullptr);
	/** Restore the event-4 slot/column/Snoid runner chain. */
	void linkActiveSnoidAboveColumns(ZmbSnoid *snoid);
	/** Insert a newly accepted Snoid before the previous one. */
	void linkAcceptedSnoid(ZmbSnoid *snoid);

	// --- Render callbacks ---

	/** Prepare trait-selector slot hotspots. */
	bool traitSlots_preRender(ZmbFeature *feature);
	/** Render the trait-selector slots. */
	ZmbRenderResult traitSlots_render(ZmbFeature *feature);
	/** v2.0US only */
	/** Prepare TLC v2 selector-hover hotspots. */
	bool selectorHover_preRender(ZmbFeature *feature);
	/** v2.0US only */
	/** Render TLC v2 selector-hover visuals. */
	ZmbRenderResult selectorHover_render(ZmbFeature *feature);
	/** v2.0US only */
	/** Register TLC v2 selector-hover feature runners. */
	void registerV2SelectorHoverRunners();
	/** v2.0US only */
	/** Update TLC v2 selector-hover runner state from pointer input. */
	void updateV2SelectorHoverRunners();
	/** v2.0US only */
	/** Return whether a TLC v2 selector-hover runner should be visible. */
	bool isV2SelectorHoverVisible(const ZmbFeature *feature) const;

	/** Remap hotspot frames by current and previous selector values. */
	void remapHotspotFramesByTrait(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);

	/** Add the target's launch-count indicator to a wall-cell feature. */
	void slotPreRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);

	// --- Static data tables ---

	static constexpr Common::Point kSnoidPositions[16]{
		Common::Point(233, 392),
		Common::Point(209, 378),
		Common::Point(196, 390),
		Common::Point(185, 365),
		Common::Point(167, 380),
		Common::Point(160, 408),
		Common::Point(135, 397),
		Common::Point(121, 407),
		Common::Point(115, 368),
		Common::Point(114, 342),
		Common::Point(99, 375),
		Common::Point(97, 394),
		Common::Point(95, 346),
		Common::Point(91, 411),
		Common::Point(79, 355),
		Common::Point(62, 404),
	};

	/** Low-difficulty slot positions (5x5 = 25 entries). */
	static constexpr Common::Point kSlotPositionsLow[25]{
		Common::Point(102, 117),
		Common::Point(204, 106),
		Common::Point(306, 94),
		Common::Point(409, 79),
		Common::Point(507, 69),
		Common::Point(102, 157),
		Common::Point(204, 143),
		Common::Point(306, 129),
		Common::Point(407, 115),
		Common::Point(507, 104),
		Common::Point(102, 195),
		Common::Point(204, 180),
		Common::Point(306, 166),
		Common::Point(407, 151),
		Common::Point(507, 140),
		Common::Point(102, 232),
		Common::Point(204, 217),
		Common::Point(306, 205),
		Common::Point(407, 191),
		Common::Point(507, 178),
		Common::Point(102, 272),
		Common::Point(204, 257),
		Common::Point(306, 245),
		Common::Point(407, 229),
		Common::Point(507, 214),
	};

	/** High-difficulty slot positions (5x25 = 125 entries). */
	static constexpr Common::Point kSlotPositionsHigh[125]{
		// Plane 0 (slots 0-24)
		Common::Point(74, 121),
		Common::Point(94, 119),
		Common::Point(114, 116),
		Common::Point(135, 113),
		Common::Point(156, 111),
		Common::Point(177, 109),
		Common::Point(197, 107),
		Common::Point(217, 104),
		Common::Point(237, 102),
		Common::Point(257, 99),
		Common::Point(278, 98),
		Common::Point(298, 95),
		Common::Point(319, 93),
		Common::Point(340, 89),
		Common::Point(360, 87),
		Common::Point(380, 84),
		Common::Point(398, 82),
		Common::Point(418, 79),
		Common::Point(438, 77),
		Common::Point(458, 74),
		Common::Point(481, 72),
		Common::Point(499, 70),
		Common::Point(519, 67),
		Common::Point(538, 65),
		Common::Point(559, 63),
		// Plane 1 (slots 25-49)
		Common::Point(74, 160),
		Common::Point(93, 158),
		Common::Point(113, 155),
		Common::Point(133, 152),
		Common::Point(154, 149),
		Common::Point(177, 147),
		Common::Point(197, 145),
		Common::Point(217, 143),
		Common::Point(237, 140),
		Common::Point(257, 139),
		Common::Point(279, 135),
		Common::Point(299, 132),
		Common::Point(319, 130),
		Common::Point(341, 127),
		Common::Point(362, 124),
		Common::Point(382, 121),
		Common::Point(400, 118),
		Common::Point(420, 115),
		Common::Point(439, 112),
		Common::Point(459, 110),
		Common::Point(480, 109),
		Common::Point(499, 107),
		Common::Point(518, 105),
		Common::Point(538, 103),
		Common::Point(558, 101),
		// Plane 2 (slots 50-74)
		Common::Point(72, 200),
		Common::Point(93, 198),
		Common::Point(113, 195),
		Common::Point(134, 192),
		Common::Point(156, 190),
		Common::Point(177, 188),
		Common::Point(197, 185),
		Common::Point(216, 181),
		Common::Point(236, 178),
		Common::Point(256, 176),
		Common::Point(279, 173),
		Common::Point(299, 170),
		Common::Point(319, 168),
		Common::Point(338, 164),
		Common::Point(358, 162),
		Common::Point(380, 158),
		Common::Point(399, 156),
		Common::Point(419, 153),
		Common::Point(438, 150),
		Common::Point(458, 148),
		Common::Point(479, 147),
		Common::Point(500, 144),
		Common::Point(519, 141),
		Common::Point(538, 138),
		Common::Point(557, 136),
		// Plane 3 (slots 75-99)
		Common::Point(74, 239),
		Common::Point(94, 236),
		Common::Point(114, 233),
		Common::Point(135, 230),
		Common::Point(156, 227),
		Common::Point(178, 224),
		Common::Point(198, 221),
		Common::Point(219, 218),
		Common::Point(238, 216),
		Common::Point(256, 214),
		Common::Point(278, 212),
		Common::Point(298, 209),
		Common::Point(319, 206),
		Common::Point(338, 203),
		Common::Point(359, 200),
		Common::Point(380, 196),
		Common::Point(399, 194),
		Common::Point(419, 191),
		Common::Point(438, 189),
		Common::Point(459, 186),
		Common::Point(479, 184),
		Common::Point(499, 181),
		Common::Point(518, 178),
		Common::Point(538, 177),
		Common::Point(556, 175),
		// Plane 4 (slots 100-124)
		Common::Point(75, 278),
		Common::Point(95, 276),
		Common::Point(115, 273),
		Common::Point(135, 269),
		Common::Point(156, 267),
		Common::Point(177, 263),
		Common::Point(197, 261),
		Common::Point(217, 258),
		Common::Point(237, 255),
		Common::Point(257, 253),
		Common::Point(280, 250),
		Common::Point(300, 248),
		Common::Point(319, 245),
		Common::Point(339, 241),
		Common::Point(359, 238),
		Common::Point(380, 235),
		Common::Point(400, 232),
		Common::Point(419, 229),
		Common::Point(439, 226),
		Common::Point(457, 223),
		Common::Point(480, 219),
		Common::Point(499, 216),
		Common::Point(519, 214),
		Common::Point(538, 211),
		Common::Point(556, 209),
	};

	/** Exit positions (16 packed x,y pairs). */
	static constexpr Common::Point kExitPositions[16]{
		Common::Point(16, 58),
		Common::Point(17, 45),
		Common::Point(15, 33),
		Common::Point(16, 19),
		Common::Point(47, 59),
		Common::Point(51, 48),
		Common::Point(46, 30),
		Common::Point(48, 20),
		Common::Point(77, 63),
		Common::Point(74, 47),
		Common::Point(76, 32),
		Common::Point(77, 18),
		Common::Point(146, 67),
		Common::Point(143, 58),
		Common::Point(141, 40),
		Common::Point(147, 32),
	};

	/** Entry start positions (Snoid event 4). */
	static constexpr Common::Point kEntryStartPositions[3]{
		Common::Point(203, 42),
		Common::Point(242, 35),
		Common::Point(283, 28),
	};

	/** Entry exit positions (Snoid event 30). */
	static constexpr Common::Point kEntryExitPositions[3]{
		Common::Point(220, 41),
		Common::Point(259, 34),
		Common::Point(300, 27),
	};
	/** Column offset remapping table 1. */
	static constexpr int16 kColOffsets1[5]{
		2,
		3,
		0,
		1,
		4,
	};
	/** Column offset remapping table 2. */
	static constexpr int16 kColOffsets2[5]{
		4,
		0,
		2,
		1,
		3,
	};

	/**
	 * Natural-language names for each selector family's five button values.
	 *
	 * Indexed as [selector family][button value 0-4]. The SubColor family shares the
	 * Color palette, so both rows carry the same color names. Used by @ref debugGetAnswer().
	 */
	static constexpr const char *kMudballSelectorValueNames[3][5]{
		{"Yellow", "Blue", "Green", "Red", "Purple"},
		{"Rectangle", "Triangle", "Star", "Circle", "Diamond"},
		{"Yellow", "Blue", "Green", "Red", "Purple"},
	};

	/**
	 * Fixed click rectangles for buttons 4-19.
	 *
	 * Index mapping (matching hotspot IDs):
	 * [0] = submit button (hotspot 4)
	 * [1-5] = column 0 values 0-4 (hotspots 5-9, diff>=2 only)
	 * [6-10] = column 1 values 0-4 (hotspots 10-14)
	 * [11-15] = column 2 values 0-4 (hotspots 15-19)
	 */
	/**
	 * Immutable button rectangles are instance members because Common::Rect
	 * requires runtime construction and ScummVM prohibits global C++ constructors.
	 */
	const Common::Rect _buttonClickRects[16] = {
		Common::Rect(450, 275, 587, 362),
		Common::Rect(446, 378, 475, 407),
		Common::Rect(476, 375, 498, 402),
		Common::Rect(499, 373, 524, 399),
		Common::Rect(525, 367, 549, 394),
		Common::Rect(550, 363, 573, 390),
		Common::Rect(446, 408, 475, 433),
		Common::Rect(476, 403, 501, 428),
		Common::Rect(499, 400, 526, 424),
		Common::Rect(525, 395, 550, 421),
		Common::Rect(550, 391, 577, 415),
		Common::Rect(450, 434, 478, 459),
		Common::Rect(479, 429, 501, 455),
		Common::Rect(502, 425, 527, 451),
		Common::Rect(528, 422, 553, 446),
		Common::Rect(554, 416, 577, 441),
	};

	// --- Puzzle configuration ---

	/** Number of wall cells: 25 on L1/L2 or 125 on L3/L4. */
	int16 _totalSlotCount = 25;

	// --- Rule grids (puzzle solution tables) ---

	/** Generated selector value for the row axis at each wall cell. */
	int16 _rowAxisRuleValues[125] = {};
	/** Generated selector value for the broad-column axis at each wall cell. */
	int16 _columnAxisRuleValues[125] = {};
	/** Generated selector value for the subcolumn axis at each high-level wall cell. */
	int16 _subcolumnAxisRuleValues[125] = {};
	/** Zoombinis launched by each unfired target; zero marks a miss and -1 a fired cell. */
	int16 _targetLaunchCounts[125] = {};
	/** Immutable generated launch counts retained for @ref debugGetAnswer(). */
	int16 _initialTargetLaunchCounts[125] = {};

	/** Level-2/4 cascading rotation step applied to one axis grid, or zero when unused. */
	int16 _axisRuleRotationStep = 0;

	/** Per-target launch batches whose sum covers the materialized active pack. */
	int16 _targetLaunchBatchSizes[12] = {};
	/** Number of generated nonzero targets for the materialized active pack. */
	int16 _targetCount = 2;

	// --- Axis-selector mapping ---

	/** Selector-family index associated with the row axis during generation. */
	int16 _rowAxisSelectorIdx = 0;
	/** Selector-family index associated with the broad-column axis during generation. */
	int16 _columnAxisSelectorIdx = 0;
	/** Selector-family index associated with the third axis during high-level generation. */
	int16 _subcolumnAxisSelectorIdx = 0;
	/** Authoritative high-level mapping from the three selectors to wall axes. */
	int16 _axisSelectorPermutationIdx = 0;

	// --- Player selector values ---

	/** Live values selected on the three machine controls. */
	int16 _selectedSelectorValues[3] = {0, 0, 0};
	/** Values preceding the latest selector change, used by transition rendering. */
	int16 _previousSelectorValues[3] = {-1, -1, -1};

	/** TLC v2 hovered value for each selector, or -1 when none. */
	int16 _hoveredSelectorValues[3] = {-1, -1, -1};

	/** Whether the TLC v2 submit-hover overlay is visible. */
	bool _submitHoverVisible = false;

	/** Last mouse position used to update TLC v2 selector hover. */
	Common::Point _selectorHoverMousePos = Common::Point(-1, -1);

	// --- Feature runners ---

	/** Wall-column route features indexed by horizontal column. */
	ZmbFeature *_columnRouteFeatures[5] = {};
	/** Column-entry transition feature. */
	ZmbFeature *_columnEntryFeature = nullptr;
	/** Introductory wall-column label feature. */
	ZmbFeature *_columnLabelFeature = nullptr;
	/** Shared mudball and selector-transition animation feature. */
	ZmbFeature *_selectorAnimFeature = nullptr;
	/** Submit and narrator-feedback feature. */
	ZmbFeature *_feedbackFeature = nullptr;
	/** Visible selector-value features indexed by machine control. */
	ZmbFeature *_selectorValueFeatures[3] = {};

	/** v2.0US only */
	/** TLC v2 selector-hover feature runners. */
	ZmbFeature *_selectorHoverFeatures[3] = {};

	/** v2.0US only */
	/** TLC v2 submit-hover feature runner. */
	ZmbFeature *_submitHoverFeature = nullptr;
	/** Mud-tank fill and drain feature. */
	ZmbFeature *_mudTankFeature = nullptr;

	// --- Slot display tracking ---

	/** Static target-indicator feature for each wall cell. */
	ZmbFeature *_targetSlotFeatures[125] = {};
	/** Fired-shot features in submission order, including a terminal rejected shot. */
	ZmbFeature *_shotHistoryFeatures[25] = {};
	/** Settled wall position recorded for each fired-shot feature. */
	Common::Point _shotHistoryPositions[25] = {};
	/** Selector values captured when each shot was fired. */
	int16 _shotSelectorValues[25][3] = {};
	/** Previous selector values captured for each shot's transition frames. */
	int16 _shotPreviousSelectorValues[25][3] = {};

	/** Last occupied entry in the shot-history arrays, or -1 before the first shot. */
	int16 _lastShotFeatureIdx = -1;

	// --- Animation state machine ---

	/** Whether the initial mud-tank fill sequence is active. */
	bool _mudTankIntroActive = false;
	/** Current step in the initial mud-tank fill sequence. */
	int16 _mudTankIntroStep = 0;

	/** Remaining lossless shots; the first decrement below zero rejects a shot. */
	int16 _remainingShotAllowance = 0;
	/** Initial lossless-shot allowance, also used as the mud-tank intro length. */
	int16 _initialShotAllowance = 0;
	/** Mudballs currently shown in the tank after the last completed animation. */
	int16 _mudTankVisualLevel = 0;
	/** Mud-tank level reached when the active tracked animation completes. */
	int16 _mudTankAnimEndLevel = 0;
	/** Settled tank level requested by the latest debugger chance command. */
	int16 _mudTankAdjustmentTargetLevel = 0;
	/** Whether the mud-tank feature owns a tracked level transition. */
	bool _mudTankAnimActive = false;
	/** Whether a debugger target still needs its final settled-frame rematerialization. */
	bool _mudTankAdjustmentPending = false;
	/** Whether the mud-tank feature holds a debugger-installed static frame. */
	bool _mudTankSettledFrameActive = false;
	/** Whether the current initial-fill SCRB step is awaiting completion. */
	bool _mudTankIntroStepActive = false;
	/** Whether the introductory column-label SCRB is awaiting completion. */
	bool _columnLabelIntroRunning = false;
	/** Whether the opening selector reveal has finished and the player may use puzzle controls. */
	bool _puzzleInputEnabled = false;

	/** Whether the sort animation is running. */
	bool _sortAnimRunning = false;
	/** Nonzero token count requesting selector setup after tank movement. */
	int16 _selectorSetupPendingCount = 0;
	/** Whether the selector-reveal sequence must start after tank movement. */
	bool _selectorRevealStartPending = false;
	/** Whether selector-reveal SCRB 7018 is awaiting completion. */
	bool _selectorRevealScrb7018Running = false;
	/** Whether selector-reveal SCRB 7025 is awaiting completion. */
	bool _selectorRevealScrb7025Running = false;
	/** Whether selector-reveal SCRB 7026 is awaiting completion. */
	bool _selectorRevealScrb7026Running = false;
	/** Whether high-level selector-reveal SCRB 7027 is awaiting completion. */
	bool _selectorRevealScrb7027Running = false;
	/** Nonzero completion count requesting the next accepted column launch. */
	int16 _columnOpenPendingCount = 0;
	/** Whether a column-open animation is running. */
	bool _columnOpenAnimRunning = false;
	/** Wall-column index whose opening SCRB is awaiting completion. */
	int16 _openingColumnIdx = 0;

	/** Whether a wall-column entry SCRB is awaiting completion. */
	bool _columnEntryAnimRunning = false;

	// --- Column/walk tracking ---

	/** Snoid IDs currently assigned to column slots. */
	uint16 _columnSlotSnoidIds[3] = {};
	/** Snoid IDs currently running the initial column-travel SCRS. */
	uint16 _travelingColumnSnoidIds[3] = {};
	/** Column currently used by staging, entry, or launch processing. */
	int16 _activeColumnIdx = 0;
	/** Active-pack index walking to the staging point, or -1 when none. */
	int16 _pendingZmbIndex = -1;
	/** Next active-pack index to assign. */
	int16 _nextZmbToAssign = 0;
	/** Snoid ID currently running the accepted column-launch sequence. */
	uint16 _launchingSnoidId = 0;
	/** Snoid ID most recently linked into the accepted exit-side chain. */
	uint16 _lastAcceptedSnoidId = 0;

	// --- Bounce animation ---

	/** Current X position of the slot bounce. */
	int16 _bounceX = 0;
	/** Current Y position of the slot bounce. */
	int16 _bounceY = 0;
	/** X delta applied by the slot bounce. */
	int16 _bounceDeltaX = 0;
	/** Y delta applied by the slot bounce. */
	int16 _bounceDeltaY = 0;
	/** Remaining frames in the slot bounce. */
	int16 _bounceCounter = 0;

	// --- Scoring and progress ---

	/** Zoombinis still to launch from the target hit by the current shot. */
	int16 _remainingLaunchesForShot = 0;
	/** Zoombinis accepted by hit targets, including launches still animating. */
	int16 _acceptedZmbCount = 0;

	// Net rejection accounting is round-local and must not share Smoke's reset rules.
	/** Number of shots fired after the lossless-shot allowance was exhausted. */
	int16 _rejectedShotCount = 0;
	/** Nonzero submission-chain activity count used by input and timeout gates. */
	int16 _submissionActivityCount = 0;
	/**
	 * Whether the player may change the selected mudball recipe.
	 * Firing also requires no active submission chain or rejection state.
	 */
	bool _selectorInputEnabled = false;
	/** Whether the current accepted shot still owns a Snoid launch SCRS. */
	bool _acceptedLaunchGatePending = false;
	/** Whether trait completion is waiting for the accepted launch SCRS. */
	bool _selectorGateReleasePending = false;
	/** Wall-cell index selected by the submitted selector values, or -1 when none. */
	int16 _selectedTargetSlotIdx = -1;

	// --- Flags ---

	/** Whether the initial selector-reveal sequence still needs its one-shot feedback path. */
	bool _initialSelectorSetup = false;
	/** Whether the current shot hit a wall cell with no target. */
	bool _shotMissedTarget = false;
	/** SCRB-table offset equal to 16 minus the tracked mud-tank level. */
	int16 _mudTankScrbOffset = 0;
	/** Whether the shared selector renderer positions the fired-shot hotspots. */
	bool _positionShotHotspots = false;
	/** Accepted launch completions awaiting final exit-side placement. */
	int16 _acceptedExitCompletionCount = 0;
	/** Whether no unassigned active-pack Snoid remains for a column slot. */
	bool _packAssignmentExhausted = false;
	/** Number of queued active-pack assignments waiting for an empty column slot. */
	int16 _queuedColumnAssignmentCount = 0;
	/** Readiness tokens allowing queued assignments to enter column slots. */
	int16 _columnAssignmentReadyCount = 0;
	/** Whether the TLC v2 submit-hover outline may be shown. */
	bool _submitHoverEnabled = false;
	/** Whether at least one accepted Snoid has reached the exit side and enabled Go. */
	bool _goButtonEnabled = false;

	// --- Exit/sort tracking ---

	/** Next accepted exit-side position assigned to a launched Snoid. */
	int16 _acceptedExitPositionIdx = 0;
	/** Current sort animation variant. */
	SortAnimationVariant _sortAnimType = SortAnimationVariant::kCenter00;
	/** Whether accepted-launch completion must play partial-success narration. */
	bool _partialSuccessNarrationPending = false;

	// --- Timing ---

	/** Frame of the latest submission-chain activity. */
	uint32 _lastSubmitFrame = 0;

	// --- Dirty flags for rendering ---

	/** Go-button enable state used during the most recent custom HUD render. */
	bool _renderedGoButtonEnabled = false;
	/** One-frame invalidation latch for the custom column-label rectangle. */
	bool _columnLabelInvalidatedThisFrame = false;

	// --- Idle animation state ---
	// Net owns the trigger and round completion contract; selection history survives page visits.
	// Do not promote these fields into generic puzzle idle state.

	/** Whether a Net idle-animation batch should be selected. */
	bool _idleAnimTrigger = false;
	/** Number of idle animations already played. */
	int16 _idleAnimCount = 0;
	/** Target number of idle animations in the current batch. */
	int16 _idleAnimMax = 0;
	/** Whether the current Net round completed. */
	bool _roundCompleted = false;
};

} // End of namespace Mohawk

#endif
