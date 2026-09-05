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

#ifndef MOHAWK_ZOOMBINI_PAGES_PUZZLE_SLIDES_H
#define MOHAWK_ZOOMBINI_PAGES_PUZZLE_SLIDES_H

#include "mohawk/zoombini_pages/puzzle_base.h"

namespace Mohawk {

/**
 * Stone Rise puzzle page (@ref ZoombiniPageType::kSlides).
 * Route 2, Puzzle 3
 *
 * Zoombinis must be placed into slots on a hex grid.
 * Matching trait patterns let groups glow, which is a visual indicator of a correct arrangement.
 * Only Zoombinis with correct arrangement can proceed to the next page.
 *
 * Grid Structure:
 * 117 cells (9 wide x 13 tall hex grid), indexed 0-116
 * 9 fields per cell:
 * [0] runnerIdx - feature runner handle
 * [1] state - cell state (500-508)
 * [2] data - zmb runner idx (for 507/508) or trait kind (510-513)
 * [3-8] links - neighbor cells (NW, W, SW, SE, E, NE), -1 = no neighbor
 *
 * Cell States:
 * 500: inert/empty (no cell)
 * 501: walkable path
 * 502: matched (trait confirmed)
 * 504/505: slot base (place zmb here)
 * 506: connector/empty slot
 * 507: occupied by zmb
 * 508: confirmed/locked occupied
 *
 * Trait Kinds (for matching):
 * 510: hair match
 * 511: eyes match
 * 512: nose match
 * 513: feet match
 *
 */
class ZoombiniPuzzleSlides : public ZoombiniPuzzle {
public:
	/** Create the Stone Rise puzzle page. */
	ZoombiniPuzzleSlides(MohawkEngine_Zoombini *vm);
	/** Release grid cells, runners, and page resources. */
	~ZoombiniPuzzleSlides() override;
	/** Static puzzle name used by debug answer descriptions. */
	static constexpr const char *kPageName = ZmbXferRouteInfo::kXferPageNameStoneRise;
	static constexpr int kRouteNumber = 2;
	static constexpr int kRoutePuzzleIdx = 3;
	/** Return the puzzle name used by debug answer descriptions. */
	const char *getPageName() const override { return kPageName; }
	int getRouteNumber() const override { return kRouteNumber; }
	int getRoutePuzzleIdx() const override { return kRoutePuzzleIdx; }

	/** Generate the cell arrangement and initialize the active pack. */
	void open() override;
	/** Select the Stone Rise puzzle music. */
	void setBackgroundMusic() override;
	/** Select the Stone Rise background bitmap. */
	void setBackgroundBitmap() override;
	/** Initialize puzzle states and departure transfer source. */
	void initStates() override;
	/** Load cell, animation, travel, and Zoombini features. */
	void loadFeatures() override;
	/** Select the fixed Stone Rise F1 replay prompt after grid construction. */
	void initHelpPrompt() override;
	/** Advance grid matching, animations, and queue state. */
	void onEveryFrame() override;
	/** Select celebration only after Snoid animation and arrival callbacks have run. */
	void onPostRenderFrame() override;
	/** Process cell and travel animation callbacks. */
	void onFeatureAnimEvent(ZmbFeature *feature, int16 eventCode) override;

	/** Start a grid placement drag or handle a control click. */
	ZmbEventHandleResult onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) override;

protected:
	/** Stone Rise actions accepted by the built-in debug console. */
	enum class BuiltinDebugAction {
		kInvalid,
		kSolve,
		kSolveReset
	};
	static constexpr const char *kBuiltinDebugActionSolve = "solve";
	static constexpr const char *kBuiltinDebugActionSolveReset = "solve-reset";
	/** Prepare all solved runners for the travel sequence. */
	void debugPrepareForDeparture() override;
	/** Run a Stone Rise built-in debug action. */
	bool debugDoBuiltinDebugCommand(int argc, const char **argv, Common::String &output) override;
	/** Describe Stone Rise built-in debug actions. */
	Common::String debugGetBuiltinDebugCommandHelp() const override;
	/** Handle the lowercase s-o-v-l-e debug sequence. */
	ZmbEventHandleResult onDebugKeyDown(const Common::KeyState &kbd) override;
	/** Describe the generated Stone Rise arrangement for diagnostics. */
	Common::String debugGetAnswer() const override;
	/** Stone Rise does not consume a finite mistake budget. */
	ZmbChanceInfo debugGetChances() const override;
	/** Begin travel after the arrangement is accepted. */
	void onGoButtonActivated() override;
	/** Execute the page-specific travel/departure hand-off. */
	void executeDeparture() override;
	/** Return the sound priority used by page SCRB playback. */
	const ZoombiniPage::ScriptSoundPriorityRanges &getScriptSoundPriorityRanges() const override;

private:
	/** Parse one console-facing Stone Rise action name. */
	static BuiltinDebugAction parseBuiltinDebugAction(const Common::String &action);
	/** Run one typed Stone Rise built-in debug action. */
	bool runBuiltinDebugAction(BuiltinDebugAction action, Common::String &output);
	/** Advance the Stone Rise lowercase s-o-v-l-e sequence. */
	void runBuiltinSolveKey(char key);
	/** Place every Zoombini at its generated level-4 answer cell. */
	void placeBuiltinDebugSolution();
	/** Page-local Stone Rise cell, travel, and placement resources. */
	enum PageResourceId : int16 {
		kResBackground5000 = 5000,

		// Terrain bitmaps
		kResBitmapTerrain100 = 100,

		// Shape bitmaps
		kResBitmapShape6000_Cell = 6000,
		kResBitmapShape7000_CellAnimations = 7000,
		kResBitmapShape8000_Travel = 8000,

		kResNode1000_WalkNetwork = 1000,

		kResScrb7000_CellBase = 7000,
		kResScrb7001_InertCell = 7001,
		kResScrb7002_MatchedCell = 7002,
		kResScrb7004_LayerBase = 7004,
		kResScrb7013_PlacementSlot = 7013,
		kResScrb8000_TravelBase = 8000,
		kResScrb8001_TravelVariant1 = 8001,
		kResScrb8002_TravelVariant2 = 8002,

		kResScrs13000_RejectBase = 13000,
		kResScrs13001_NormalBase = 13001,
		kResScrs14000_TravelBase = 14000,

		// Sound resources.
		kResSound7000_SolvedRiseBase = 7000,
		kResSound7001_SolvedRiseTail = 7001,
		kResSound8500_AcceptedCountDecreaseSmall = 8500,
		kResSound8501_AcceptedCountDecreaseLarge = 8501,
		kResSound8502_AcceptedCellsRearranged = 8502,
		kResSound8504_AcceptedCountIncreaseSmall = 8504,
		kResSound8505_AcceptedCountIncreaseLarge = 8505,

		// Sound ranges used by getScriptSoundPriority().
		kResSoundRange0425_PageBase = 425,
		kResSoundRange0499_PageLast = 499,
		kResSoundRange7000_CellBase = 7000,
		kResSoundRange7001_InertCell = 7001,
		kResSoundRange7002_MatchedCell = 7002,
		kResSoundRange8000_TravelBase = 8000,
		kResSoundRange8500_PlacementFeedbackBase = 8500,
		kResSoundRange8599_PlacementFeedbackLast = 8599,
	};

	/** Queue the entrance sound through the inherited highest-priority system range. */
	void playEntranceSound();
	/** Start the v1.x entrance sound after the page fade has completed. */
	void updateEntranceSound();

	/** Travel SCRS callbacks for the four entrance/exit variants. */
	enum SlidesEventCode : int16 {
		kSlidesEventCode000_ToggleSnoidFacing = 0,
		kSlidesEventCode090_TravelVariant0 = 90,
		kSlidesEventCode091_TravelVariant1 = 91,
		kSlidesEventCode092_TravelVariant2 = 92,
		kSlidesEventCode093_TravelVariant3 = 93
	};

	/** Arm the authored solved-placement departure animation. */
	void beginSolvedDepartureSequence();
	/** Complete the solved-placement departure and hand off to Xfer. */
	void finishSolvedDepartureSequence();
	/** Select the branch anchors that walk out at the two advanced levels. */
	void selectAdvancedDepartureRunners();
	/** Mark a locked cell's Snoid for the next shared departure snapshot. */
	void markCellRunnerForDeparture(int16 cellIdx);

	// =========================================================================
	// Cell Grid Constants
	// =========================================================================

	/** Total number of cells in hex grid (9 wide x 13 tall). */
	static const int16 kNumCells = 117;

	/** Number of fields per cell in the grid. */
	static const int16 kFieldsPerCell = 9;

	/** Cell state constants. */
	enum CellState : int16 {
		kCellInert = 500,     // Empty/unused cell
		kCellPath = 501,      // Walkable path cell
		kCellMatched = 502,   // Trait match confirmed
		kCellSlotBase1 = 504, // Slot base for placing zmb
		kCellSlotBase2 = 505, // Alternate slot base (level 4)
		kCellConnector = 506, // Connector cell
		kCellOccupied = 507,  // Occupied by zmb
		kCellLocked = 508     // Locked/confirmed occupied
	};

	/** Trait kind markers for matching. */
	enum TraitKind : int16 {
		kTraitHair = 510,
		kTraitEyes = 511,
		kTraitNose = 512,
		kTraitFeet = 513
	};

	/**
	 * Adjacency bit flags for the six hex-grid directions.
	 *
	 * The low six bits are laid out as follows; bits 15 through 6 are unused:
	 * @code
	 * bit:  5  4  3  2  1  0
	 * dir: NE  E SE SW  W NW
	 * @endcode
	 * Each of the 117 cells in the 9-by-13 grid owns one uint16 mask in @ref _cellAdjacencyMasks.
	 */
	enum AdjBit : uint16 {
		kAdjNorthWest = 0x01,
		kAdjWest = 0x02,
		kAdjSouthWest = 0x04,
		kAdjSouthEast = 0x08,
		kAdjEast = 0x10,
		kAdjNorthEast = 0x20
	};

	// =========================================================================
	// Initialization Functions
	// =========================================================================

	/**
	 * Initialize the cell grid based on difficulty level.
	 * Sets up cell states, slot positions, and link configurations.
	 */
	void initGridByDifficulty();

	/**
	 * Build the hex adjacency table.
	 * This path computes neighbor cell indices and adjacency bit flags for all 117 cells.
	 */
	void buildHexAdjacencyTable();

	/**
	 * Generate trait pairings for matching.
	 * This path creates trait pairs based on placed Zoombinis.
	 */
	void generateTraitPairings();

	/**
	 * Snapshot each loaded Zoombini's four canonical traits.
	 */
	void snapshotZmbTraits();

	// =========================================================================
	// Input Handling
	// =========================================================================

	/** End a drag operation and process the drop. */
	void endDrag(const Common::Point &dropPos) override;

	/**
	 * Return the drag constraint rect for this puzzle.
	 * Constrain dragging to the left-bank area.
	 */
	const Common::Rect &getDragConstraintRect() const override;

	// =========================================================================
	// Grid Cell Functions
	// =========================================================================

	/**
	 * Find the placement cell selected by the DRAW_ON_REG slot system.
	 * @param pos Dragged Snoid root position to test.
	 * @return Cell index (0-116), or -1 if no cell at position.
	 */
	int16 findCellAtPosition(const Common::Point &pos) const;
	/** Return the placement-slot index that owns a grid cell, or -1. */
	int16 findSlotIndexForCell(int16 cellIdx) const;

	/**
	 * Check if a cell is a valid drop target for a Zoombini.
	 * @param cellIdx Cell index to check.
	 * @return true if the cell accepts a Zoombini.
	 */
	bool isCellValidDropTarget(int16 cellIdx) const;

	/**
	 * Assign a Zoombini to a slot cell.
	 * Updates cell state and runner references.
	 * @param snoid The Zoombini to place.
	 * @param cellIdx Target cell index.
	 */
	void assignZmbToSlot(ZmbSnoid *snoid, int16 cellIdx);
	/** Reserve the next available placement slot associated with a base cell. */
	int16 assignZmbToSlot(int16 slotBaseCell);

	/** Orphan the accepted suffix after removing a Snoid from a level 1/2 chain. */
	void orphanLinearChainFromCell(int16 sourceCellIdx);

	/** Move a Zoombini to a cell position. */
	void moveZmbToCell(ZmbSnoid *snoid, int16 cellIdx);
	/** Decode a scripted move record and move its Zoombini to the target cell. */
	int16 moveZmbToCell(int16 moveData);

	/** Clear a cell to empty state. */
	void clearCellToEmpty(int16 cellIdx);

	/** Reset a cell to inert state and clear all links. */
	void resetCellToEmpty(int16 cellIdx);

	/** Clear link bits from a cell. */
	void clearCellLinkBits(uint16 bitMask, int16 linkField, int16 cellIdx);

	/** Update neighbor flags after cell state change. */
	void updateNeighborFlags();
	/** Set a cell's match trait when its neighboring cells agree. */
	void maybeSetMatchTrait(int16 destCell, int16 cellIdx, int16 otherCellIdx);
	/** Write the state and auxiliary data fields of one grid cell. */
	void setCellStateData(int16 cellIdx, int16 state, int16 data);
	/** Reset every grid cell, slot, and runner association. */
	void clearBoard();
	/** Rebuild the occupied-slot list from the current grid states. */
	void rebuildOccupiedSlots();
	/** Store a neighbor link only when its direction is present in the adjacency mask. */
	void setLinkIfValid(uint16 adjMask, int16 base, uint16 bit, int16 fieldOffset, int16 neighborCell);
	/** Return a directional neighbor cell index, or -1 when no link exists. */
	int16 getCellLink(int16 cellIdx, int16 dir) const;
	/** Return the current state field for a grid cell. */
	int16 getCellState(int16 cellIdx) const;

	// =========================================================================
	// Chain Building and Matching
	// =========================================================================

	/** Build the Slides chain-link sequence. */
	void buildChainSequence();

	/**
	 * Validate a chain and mark matched cells.
	 * @return Number of matches found.
	 */
	int16 validateChainAndMarkMatched(int16 startCellIdx);

	/** Find a runner by matching trait. */
	int16 findRunnerByMatchingTrait(int16 runnerIdx);

	/** Sort Zoombinis by overlap count for optimal pairing. */
	void sortZmbsByOverlapCount();

	/** Place a matching Zoombini in a cell. */
	int16 placeMatchingZmbInCell(int16 matchCellIdx, int16 outSlot);

	/** Pick the first matching trait between two occupied cells. */
	int16 pickRandomMatchingTrait(int16 cellIdx, int16 otherCellIdx) const;

	/** Activate a link in the chain sequence. */
	void activateChainLink(int16 linkIdx);

	/** Confirm endpoint matches in a chain. */
	void confirmEndpointMatches();

	/** Check the first matching trait between two sorted Zoombini candidates. */
	bool checkFirstTraitMatch(int16 leftSortedIdx, int16 rightSortedIdx);

	/** Evaluate trait match and advance chain. */
	void evalTraitMatchAndAdvance(int16 leadCellIdx, int16 middleCellIdx, int16 tailCellIdx);

	/** Evaluate neighbor states for chain propagation. */
	void evalNeighborStates(int16 cellIdx);

	/** Propagate match through the chain. */
	void propagateMatchChain(int16 chainIdx);

	/** Ensure a grid cell has a draw-on-reg feature backing its SCRB transitions. */
	void ensureCellFeature(int16 cellIdx);
	/** Register the shared overlay that draws SCRB 7000 cell-link shapes above the cell surfaces. */
	void loadCellLinkOverlay();
	/** Rebuild the shared cell-link overlay after its owning cell state changes. */
	bool cellLinkOverlay_preRender(ZmbFeature *feature);
	/** Request cell-link overlay rematerialization while retaining its old dirty coverage. */
	void requestCellLinkOverlayRefresh();
	/** Return SCRB 7000 relationship shapes to their cell runners before the solved rise. */
	void handoffCellLinksToGridRunners();

	/** Preserve the generated solved placement before initialization hides it. */
	void snapshotDebugSolution();
	/** Build a debug solution by satisfying the generated slot constraints. */
	bool buildDebugSolutionFromConstraints();
	/** Fill any remaining debug solution positions with unused Snoids. */
	bool completeDebugSolutionWithUnusedSnoids();
	bool solveDebugSlotAssignments(int16 slotIdx, const int16 *requiredTraits,
								   bool *usedSnoids, int16 *assignedSnoids);
	/** Return whether two debug candidates share the requested trait. */
	bool debugSnoidsMatchTrait(int16 leftSnoidIdx, int16 rightSnoidIdx, int16 traitKind) const;

	/** Register the per-slot SCRB 7013 DRAW_ON_REG helpers. */
	void loadPlacementSlotFeatures();

	/** Restore the six row-layer links after cell SCRB loads. */
	void relinkGridLayers();

	/** Restore the initial Slides Snoid runner order used before the first click. */
	void relinkSnoidsBehindForeground();

	/** Restore the Snoid flags saved before the initial foreground-tree relink. */
	void restoreInitialSnoidFlags();

	/** Find the first backward-side chain link (fields 3, 4, 5). */
	int16 getBackwardChainLink(int16 cellIdx) const;

	/** Find the first forward-side chain link (fields 8, 7, 6). */
	int16 getForwardChainLink(int16 cellIdx) const;

	/** Check whether a cell is currently in one of the supplied states. */
	bool cellStateIs(int16 cellIdx, int16 stateA, int16 stateB = -1, int16 stateC = -1) const;

	/** Compare two occupied cells on a single trait kind. */
	bool cellsMatchTrait(int16 leftCellIdx, int16 rightCellIdx, int16 traitKind) const;

	/** Set the cell state and reload its visual SCRB when a feature exists. */
	bool setCellStateAndReload(int16 cellIdx, int16 state, int16 scrbId = kResScrb7000_CellBase);

	// =========================================================================
	// Animation and Travel
	// =========================================================================

	/** Reset accepted cell states and rebuild the valid chain from current placements. */
	void resetAnimStates();

	/** Start the invalid-cell spark/ejection sequence at the grid cell currently under the dragged Snoid. */
	bool beginZmbTravel(ZmbSnoid *snoid);

	/** Dispatch SCRB 8000-8002 and SCRS 14000-14003 travel-chain events. */
	void handleZmbTravelEvent(int16 eventCode);

	/**
	 * Return the Snoid anchor for a grid cell.
	 * The cell runner is registered at the table point; Snoids stand 24 pixels right and 5 pixels above that point.
	 */
	Common::Point getCellSnoidPosition(int16 cellIdx) const;

	/**
	 * Count cells in the matched or locked state and update the sum of their cell indices.
	 * This lightweight checksum detects some placement changes when the accepted-cell count is unchanged.
	 * It is not a complete accepted-cell set comparison and can have collisions.
	 */
	int16 countAcceptedCellsAndUpdateChecksum();

	/** Play placement feedback based on the accepted-cell count change. */
	void playPlacementFeedbackSFX();

	/** Load SCRB data onto a runner feature. */
	void loadRunnerSCRB(uint16 runnerId, int16 scrbId);

	// =========================================================================
	// Slot Management
	// =========================================================================

	/** Relink Snoids for solved-rise depth ordering. */
	void unlockInteractiveSlots();

	/** Place next Zoombini in a cell. */
	int16 placeNextZmbInCell(int16 cellIdx);

	/** Check if there's a pending Zoombini to place. */
	bool hasPendingZmb() const;

	/** Scan and reset active cells. */
	void scanAndResetActiveCells();

	/** Find a matching Zoombini for a cell. */
	int16 findMatchingZmbForCell(int16 matchCellIdx, int16 outResult);

	/** Reassign dead slots. */
	void reassignDeadSlots();

	/** Pick next cell for chain linking. */
	void pickNextCellForLink(int16 cellIdx, int16 nextCell, int16 direction);

	/** Mark matched runners as done. */
	void markMatchedRunnersDone();

	// =========================================================================
	// Victory Checking
	// =========================================================================

	/** Check if victory condition is met. */
	void checkVictoryCondition();
	/** Recompute Go enabled state from whether any active slot contains a locked Snoid. */
	void refreshGoButtonEnabledState();

	/** Resolve the grid cell that owns a draw-on-reg slot feature. */
	int16 findCellIdxForFeature(const ZmbFeature *feature) const;

	/** Select the proper pre-render callback for a slot SCRB. */
	void setCellFeaturePreRenderHook(ZmbFeature *feature, int16 scrbId);

	/** Reload the slot SCRB that matches the current cell state. */
	void syncCellFeatureScript(int16 cellIdx);

	/** Map Slides hotspot opcodes onto adjacency mask bits. */
	static uint16 getAdjMaskForCommand(int16 cmd);

	// =========================================================================
	// Filter/Callback Functions
	// =========================================================================

	/** Filter hotspot script for rendering. */
	void filterHotspotScript(ZmbFeature *feature, ZmbHotspotGroup *hsGroup,
							 Common::Array<ZmbHotspot> &hotspots);

	/** Filter command by flags. */
	void filterCommandByFlags(ZmbFeature *feature, ZmbHotspotGroup *hsGroup,
							  Common::Array<ZmbHotspot> &hotspots);

	/** Process command queue. */
	void processCommandQueue(ZmbFeature *feature, ZmbHotspotGroup *hsGroup,
							 Common::Array<ZmbHotspot> &hotspots);

	// =========================================================================
	// Static Data Tables
	// =========================================================================

	/** Pedestal positions for 16 Zoombinis. */
	static constexpr Common::Point kSnoidPositions[16]{
		Common::Point(482, 127),
		Common::Point(428, 128),
		Common::Point(375, 129),
		Common::Point(318, 127),
		Common::Point(272, 129),
		Common::Point(226, 128),
		Common::Point(184, 127),
		Common::Point(140, 129),
		Common::Point(87, 128),
		Common::Point(110, 170),
		Common::Point(122, 246),
		Common::Point(84, 212),
		Common::Point(140, 327),
		Common::Point(77, 293),
		Common::Point(40, 157),
		Common::Point(44, 232),
	};

	/**
	 * Cell center positions (117 cells).
	 *
	 * Cell IDs are row-major in this table, but each row is stored from the screen's right edge to its left edge.
	 * The table index is row * 9 + tableColumn.
	 * In a displayed row, cell row * 9 + 8 is leftmost and cell row * 9 is rightmost.
	 *
	 * Hex-grid layout (cell IDs, top-to-bottom and screen left-to-right).
	 * Odd rows are shifted left by half a cell in this diagram.
	 * The indentation shows the alternating row relationship.
	 * Use the center points below for exact screen positions.
	 * @code
	 * row 00:     8   7   6   5   4   3   2   1   0
	 * row 01:  17  16  15  14  13  12  11  10   9
	 * row 02:    26  25  24  23  22  21  20  19  18
	 * row 03:  35  34  33  32  31  30  29  28  27
	 * row 04:    44  43  42  41  40  39  38  37  36
	 * row 05:  53  52  51  50  49  48  47  46  45
	 * row 06:    62  61  60  59  58  57  56  55  54
	 * row 07:  71  70  69  68  67  66  65  64  63
	 * row 08:    80  79  78  77  76  75  74  73  72
	 * row 09:  89  88  87  86  85  84  83  82  81
	 * row 10:    98  97  96  95  94  93  92  91  90
	 * row 11: 107 106 105 104 103 102 101 100  99
	 * row 12:   116 115 114 113 112 111 110 109 108
	 * @endcode
	 *
	 * The grid has 13 rows and 9 columns.
	 * X decreases left-to-right by about 42 pixels and Y increases top-to-bottom by about 18 pixels.
	 * Odd rows shift left by about 16 pixels for the hex-grid offset.
	 */
	static constexpr Common::Point kCellPositions[117]{
		// Row 0 (cells 0-8)
		Common::Point(477, 152),
		Common::Point(435, 152),
		Common::Point(393, 152),
		Common::Point(351, 152),
		Common::Point(309, 152),
		Common::Point(267, 152),
		Common::Point(225, 152),
		Common::Point(183, 152),
		Common::Point(141, 152),
		// Row 1 (cells 9-17)
		Common::Point(461, 170),
		Common::Point(419, 170),
		Common::Point(377, 170),
		Common::Point(335, 170),
		Common::Point(293, 170),
		Common::Point(251, 170),
		Common::Point(209, 170),
		Common::Point(167, 170),
		Common::Point(125, 170),
		// Row 2 (cells 18-26)
		Common::Point(487, 188),
		Common::Point(445, 188),
		Common::Point(403, 188),
		Common::Point(361, 188),
		Common::Point(319, 188),
		Common::Point(277, 188),
		Common::Point(235, 188),
		Common::Point(193, 188),
		Common::Point(151, 188),
		// Row 3 (cells 27-35)
		Common::Point(471, 206),
		Common::Point(429, 206),
		Common::Point(387, 206),
		Common::Point(345, 206),
		Common::Point(303, 206),
		Common::Point(261, 206),
		Common::Point(219, 206),
		Common::Point(177, 206),
		Common::Point(135, 206),
		// Row 4 (cells 36-44)
		Common::Point(497, 224),
		Common::Point(455, 224),
		Common::Point(413, 224),
		Common::Point(371, 224),
		Common::Point(329, 224),
		Common::Point(287, 224),
		Common::Point(245, 224),
		Common::Point(203, 224),
		Common::Point(161, 224),
		// Row 5 (cells 45-53)
		Common::Point(481, 242),
		Common::Point(439, 242),
		Common::Point(397, 242),
		Common::Point(355, 242),
		Common::Point(313, 242),
		Common::Point(271, 242),
		Common::Point(229, 242),
		Common::Point(187, 242),
		Common::Point(145, 242),
		// Row 6 (cells 54-62)
		Common::Point(507, 260),
		Common::Point(465, 260),
		Common::Point(423, 260),
		Common::Point(381, 260),
		Common::Point(339, 260),
		Common::Point(297, 260),
		Common::Point(255, 260),
		Common::Point(213, 260),
		Common::Point(171, 260),
		// Row 7 (cells 63-71)
		Common::Point(491, 278),
		Common::Point(449, 278),
		Common::Point(407, 278),
		Common::Point(365, 278),
		Common::Point(323, 278),
		Common::Point(281, 278),
		Common::Point(239, 278),
		Common::Point(197, 278),
		Common::Point(155, 278),
		// Row 8 (cells 72-80)
		Common::Point(517, 296),
		Common::Point(475, 296),
		Common::Point(433, 296),
		Common::Point(391, 296),
		Common::Point(349, 296),
		Common::Point(307, 296),
		Common::Point(265, 296),
		Common::Point(223, 296),
		Common::Point(181, 296),
		// Row 9 (cells 81-89)
		Common::Point(501, 314),
		Common::Point(459, 314),
		Common::Point(417, 314),
		Common::Point(375, 314),
		Common::Point(333, 314),
		Common::Point(291, 314),
		Common::Point(249, 314),
		Common::Point(207, 314),
		Common::Point(165, 314),
		// Row 10 (cells 90-98)
		Common::Point(527, 332),
		Common::Point(485, 332),
		Common::Point(443, 332),
		Common::Point(401, 332),
		Common::Point(359, 332),
		Common::Point(317, 332),
		Common::Point(275, 332),
		Common::Point(233, 332),
		Common::Point(191, 332),
		// Row 11 (cells 99-107)
		Common::Point(511, 350),
		Common::Point(469, 350),
		Common::Point(427, 350),
		Common::Point(385, 350),
		Common::Point(343, 350),
		Common::Point(301, 350),
		Common::Point(259, 350),
		Common::Point(217, 350),
		Common::Point(175, 350),
		// Row 12 (cells 108-116)
		Common::Point(537, 368),
		Common::Point(495, 368),
		Common::Point(453, 368),
		Common::Point(411, 368),
		Common::Point(369, 368),
		Common::Point(327, 368),
		Common::Point(285, 368),
		Common::Point(243, 368),
		Common::Point(201, 368),
	};

	/** Primary slot cell indices (26 cells), evenly spaced across the grid. */
	static constexpr int16 kSlotCellIndices[26]{
		2,
		4,
		6,
		19,
		21,
		23,
		25,
		38,
		40,
		42,
		44,
		55,
		57,
		59,
		61,
		74,
		76,
		78,
		80,
		91,
		93,
		95,
		97,
		110,
		112,
		114,
	};

	/** Interior/link cell indices (43 cells). */
	static constexpr int16 kLinkCellIndices[43]{
		10,
		11,
		12,
		13,
		14,
		15,
		28,
		29,
		30,
		31,
		32,
		33,
		34,
		46,
		47,
		48,
		49,
		50,
		51,
		52,
		56,
		58,
		60,
		64,
		65,
		66,
		67,
		68,
		69,
		70,
		82,
		83,
		84,
		85,
		86,
		87,
		88,
		100,
		101,
		102,
		103,
		104,
		105,
	};

	/** Even-row link cells (20 cells). */
	static constexpr int16 kEvenRowLinkCells[20]{
		10,
		12,
		14,
		29,
		31,
		33,
		46,
		48,
		50,
		52,
		65,
		67,
		69,
		82,
		84,
		86,
		88,
		101,
		103,
		105,
	};

	/** Odd-row link cells (20 cells). */
	static constexpr int16 kOddRowLinkCells[20]{
		11,
		13,
		15,
		28,
		30,
		32,
		34,
		47,
		49,
		51,
		64,
		66,
		68,
		70,
		83,
		85,
		87,
		100,
		102,
		104,
	};

	/** Pair start offsets (16 entries). */
	static constexpr int16 kPairStartOffsets[16]{
		0,
		54,
		45,
		36,
		27,
		18,
		9,
		0,
		18,
		18,
		9,
		9,
		0,
		0,
		0,
		0,
	};

	/** Pair spacing array (16 entries). */
	static constexpr int16 kPairSpacingArray[16]{
		0,
		0,
		18,
		18,
		18,
		18,
		18,
		18,
		9,
		9,
		9,
		9,
		9,
		9,
		10,
		12,
	};

	/** Left-arm link cells (18 cells). */
	static constexpr int16 kLeftArmLinkCells[18]{
		10,
		12,
		14,
		28,
		30,
		32,
		46,
		48,
		50,
		64,
		66,
		68,
		82,
		84,
		86,
		100,
		102,
		104,
	};

	/** Right-arm + diagonal link cells (18 cells). */
	static constexpr int16 kRightArmLinkCells[18]{
		11,
		29,
		47,
		65,
		83,
		101,
		13,
		31,
		49,
		67,
		85,
		103,
		24,
		60,
		96,
		19,
		55,
		91,
	};

	/** Left endpoint cells (3 cells). */
	static constexpr int16 kLeftEndpointCells[3]{
		18,
		54,
		90,
	};

	/** Right endpoint cells (3 cells). */
	static constexpr int16 kRightEndpointCells[3]{
		24,
		60,
		96,
	};

	/** Inner link pair cells (12 cells). */
	static constexpr int16 kInnerLinkPairs[12]{
		11,
		13,
		29,
		31,
		47,
		49,
		65,
		67,
		83,
		85,
		101,
		103,
	};

	/**
	 * Drag constraint rectangle. It is instance-owned because Common::Rect
	 * requires runtime construction and ScummVM prohibits global C++ constructors.
	 */
	const Common::Rect _dragConstraint = Common::Rect(0, 110, 540, 400);

	// =========================================================================
	// Runtime State
	// =========================================================================
	/**
	 * Generated placement-slot cell state.
	 * It is 504 except for the Level-4 variant that selects 505.
	 */
	int16 _slotCellState = 504;
	/**
	 * Shape-bank offset for SCRB 7002 relationship links during the solved rise.
	 * The script encodes its north-west, west, and north-east links as shape IDs 4, 8, and 24.
	 * @ref ZoombiniPuzzleSlides::processCommandQueue() removes links absent from the cell adjacency mask, then adds this offset to each surviving ID.
	 * This selects shapes 52/56/72 for slot state 504 or shapes 28/32/48 for slot state 505.
	 */
	int16 _linkShapeVariantOffset = 48;
	/**
	 * The hex cell grid.
	 * 117 cells x 9 fields = 1053 int16 values.
	 */
	int16 _cellGrid[kNumCells * kFieldsPerCell];
	/** Six-direction adjacency mask for each cell. */
	uint16 _cellAdjacencyMasks[kNumCells];
	/** Mapping from each active placement-slot index to its grid-cell index. */
	int16 _slotCellIndices[26];
	/** Number of valid entries in @ref _slotCellIndices. */
	int16 _slotCount = 0;
	/** Runner IDs of the loaded Snoids in trait-array order. */
	int16 _snoidIds[16];
	/** Indices into @ref _snoidIds, sorted by trait-overlap count. */
	int16 _sortedSnoidIndices[16];
	/** Canonical traits in the same order as @ref _snoidIds. */
	ZmbTrait _snoidTraits[16];
	/** Pairing state per Snoid: zero unused, one used, or 99 for an unpaired sentinel. */
	int16 _pairingUsageStates[16];
	/** Generated link type per pair/group: 501 or one of trait codes 510-513. */
	int16 _pairLinkTypes[16];
	/** Number of Level-1 pairs or Level-2 three-Snoid groups generated. */
	int16 _pairGroupCount = 0;
	/** Zero-based trait index selected by the current match search. */
	int16 _currentMatchedTraitIndex = 0;
	/** Grid-cell indices in the current occupied-cell propagation work list. */
	int16 _activeCellIndices[26];
	/** Snoid runner IDs corresponding to @ref _activeCellIndices. */
	int16 _activeCellSnoidIds[26];
	/** Number of valid entries in the active-cell work lists. */
	int16 _activeCellCount = 0;
	/**
	 * Feature pointers for cells (optional, for quick lookup).
	 * These are tracked explicitly rather than through runner indices.
	 */
	ZmbFeature *_cellFeatures[kNumCells];
	/** Shared runner that draws SCRB 7000 direction shapes after every cell surface. */
	ZmbFeature *_cellLinkOverlayFeature = nullptr;
	/** True when the shared cell-link hotspot list must be rebuilt. */
	bool _cellLinkOverlayDirty = false;
	/** True while the flat-grid overlay owns the SCRB 7000 relationship shapes. */
	bool _cellLinkOverlayActive = true;
	/** SCRB 7013 DRAW_ON_REG helper for each valid placement slot. */
	ZmbFeature *_slotFeatures[26];
	/** Shared DRAW_ON_REG index belonging to each Slides placement helper. */
	int16 _slotDrawOnRegIndices[26];
	/** Row-layer SCRB features used to preserve grid depth order. */
	ZmbFeature *_rowLayerFeatures[9];
	/** Cell indexes of the generated solved placement for @c puzzle finish. */
	int16 _debugSolutionCellIndices[16];
	/** Runner IDs corresponding to the generated solved placement. */
	int16 _debugSolutionRunnerIds[16];
	/** Number of valid entries in the retained debug solved placement. */
	int16 _debugSolutionCount = 0;
	/** Progress through the lowercase s-o-v-l-e solve sequence. */
	int16 _builtinDebugSolveState = 0;
	/** Runner ID of the Snoid owned by the invalid-drop travel sequence. */
	uint16 _invalidDropSnoidId = 0;
	/** Invalid-drop spark controller feature. */
	ZmbFeature *_invalidDropEffectFeature = nullptr;
	/** Whether an invalid-drop travel animation is active. */
	bool _invalidDropTravelActive = false;
	/** Whether Go is enabled because at least one active slot contains a locked Snoid. */
	bool _goButtonEnabled = false;
	/** Whether the Level-4 victory palette rotation is active. */
	bool _victoryPaletteActive = false;
	/** Frame of the most recent Level-4 victory palette rotation. */
	uint32 _lastVictoryPaletteFrame = 0;
	/** Whether the player may rearrange Snoids on the board or start the solved Go action. */
	bool _boardInputEnabled = true;
	/** First solved-rise cell feature that owns the SCRB completion and sound boundary. */
	ZmbFeature *_solvedRiseTimingFeature = nullptr;
	/** Frame at which page SND 7001 takes over from page SND 7000. */
	uint32 _solvedRiseTailStartFrame = 0;
	/** Whether the solved-rise SCRB owner has queued page SND 7001. */
	bool _solvedRiseTailQueued = false;
	/** True until the entrance sound has been submitted after the page fade. */
	bool _entranceSoundPending = false;
	/** True after at least one v1.x fade frame has reached the page. */
	bool _entranceFadeObserved = false;
	/** Common Snoid flags saved before the initial foreground-tree relink. */
	uint32 _preRelinkSnoidFlags = 0;
	/** Whether the first Slides click still needs to restore @ref _preRelinkSnoidFlags. */
	bool _preRelinkSnoidFlagsPending = false;

	// -------------------------------------------------------------------------
	// Celebration state
	// -------------------------------------------------------------------------

	/**
	 * Per-visit latch set when the single celebration opportunity is consumed.
	 * The opportunity remains consumed when its frame deadline has not elapsed
	 * or no eligible runner starts a celebration.
	 * It is reset only when a new Slides page is initialized.
	 */
	bool _celebrationVisitLatched = false;
	/**
	 * Number of successful celebration starts needed before the session cycle resets.
	 * This equals the loaded Zoombini count for the current page visit.
	 * Slides owns the full-match trigger and completion rules; do not share
	 * this state with another puzzle's celebration scheduler.
	 */
	int16 _celebrationCycleTarget = 0;
	/** Number of full-pack match triggers waiting on the per-visit celebration scheduler. */
	int16 _fullMatchTriggerCount = 0;
	/** Number of accepted cells immediately before the current drag. */
	int16 _acceptedCellCountBeforeDrag = 0;
	/** Lightweight checksum formed by summing indices of matched or locked cells. */
	int16 _acceptedCellIndexChecksum = 0;
	/** Accepted-cell index checksum captured immediately before the current drag. */
	int16 _acceptedCellIndexChecksumBeforeDrag = 0;
};

} // End of namespace Mohawk

#endif
