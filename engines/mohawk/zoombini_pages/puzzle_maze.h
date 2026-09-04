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

#ifndef MOHAWK_ZOOMBINI_PAGES_PUZZLE_MAZE_H
#define MOHAWK_ZOOMBINI_PAGES_PUZZLE_MAZE_H

#include "common/array.h"
#include "mohawk/zoombini_pages/grid_traversal.h"
#include "mohawk/zoombini_pages/puzzle_base.h"

namespace Mohawk {

/**
 * Bubblewonder Abyss puzzle page (@ref ZoombiniPageType::kMaze).
 * Route 4, Puzzle 3
 *
 * Zoombinis traverse a 13x13 device grid in bubbles.
 * White and conditional arrows redirect them.
 * Colored switches rotate linked arrows, while whirlpools and collisions reject the Zoombinis.
 * Sticky cells hold them until released.
 */
class ZoombiniPuzzleMaze : public ZoombiniPuzzle {
public:
	/** Create the Bubblewonder Abyss puzzle page. */
	ZoombiniPuzzleMaze(MohawkEngine_Zoombini *vm);
	/** Release maze cells, runners, overlays, and page resources. */
	~ZoombiniPuzzleMaze() override;
	/** Static puzzle name used by debug answer descriptions. */
	static constexpr const char *kPageName = ZmbXferRouteInfo::kXferPageNameBubblewonderAbyss;
	static constexpr int kRouteNumber = 4;
	static constexpr int kRoutePuzzleIdx = 3;
	/**
	 * Upper bound for one level-4 answer plan search in simulated launches.
	 *
	 * Measured with temporary instrumentation that ran the search on each
	 * level-4 layout and logged the step count and wall time:
	 *
	 * CPU: AMD Ryzen 9 9950X
	 * - REGS 16606 (authored): 349,897 steps, plan found, 146 ms
	 * - REGS 16608 (authored): 349,901 steps, plan found, 233 ms
	 * - REGS 16607 (restored): 3,000,000 steps, budget exhausted, 1,813 ms
	 */
	static constexpr uint32 kDebugMazeSolverMaxSteps = 1500000;
	/**
	 * Upper bound for the generic level-4 answer plan search in simulated launches.
	 *
	 * The generic search runs only when the staged relay search above finds no
	 * plan, which happens for packs the restored layout REGS 16607 does not fit
	 * into the staged relay shape. It branches only on launches that change the
	 * control state, so a solvable pack is normally answered within a few
	 * hundred simulated launches; the bound matters only when the search has to
	 * prove that no perfect run exists.
	 *
	 * Measured on the level-4 practice page with sixteen Zoombinis, timing the
	 * whole answer:
	 *
	 * CPU: AMD Ryzen 9 9950X
	 * - REGS 16607, solvable pack: direct launch order of 17 launches, 852 ms
	 * - REGS 16607, unsolvable pack: search exhausted, 822 ms to 2,599 ms
	 */
	static constexpr uint32 kDebugMazeGenericSolverMaxSteps = 2000000;
	/** Upper bound for the generic search node pool. */
	static constexpr uint32 kDebugMazeGenericSolverMaxNodes = 320000;
	/**
	 * Bounds for one solvability probe made while repairing REGS 16607.
	 *
	 * The probe runs during page setup, so it must answer quickly. A pack that
	 * can be completed is answered within about two thousand simulated
	 * launches, measured across the captured level-4 packs, so this bound
	 * carries roughly a tenfold margin while an unsolvable candidate is
	 * abandoned instead of proven impossible.
	 */
	static const uint32 kMazeRepairProbeMaxSteps = 20000;
	/** Node pool for one solvability probe made while repairing REGS 16607. */
	static const uint32 kMazeRepairProbeMaxNodes = 20000;
	/** Total simulated launches one REGS 16607 repair may spend. */
	static const uint32 kMazeRepairTotalMaxSteps = 300000;
	/** Return the puzzle name used by debug answer descriptions. */
	const char *getPageName() const override { return kPageName; }
	int getRouteNumber() const override { return kRouteNumber; }
	int getRoutePuzzleIdx() const override { return kRoutePuzzleIdx; }

	/** Generate the device grid and initialize the active pack. */
	void open() override;
	/** Select the Bubblewonder Abyss puzzle music. */
	void setBackgroundMusic() override;
	/** Select the Bubblewonder Abyss background bitmap. */
	void setBackgroundBitmap() override;
	/** Initialize puzzle states and departure transfer source. */
	void initStates() override;
	/** Load device cells, arrows, bubbles, and Zoombini features. */
	void loadFeatures() override;
	/** Restart the global Snoid fidget inactivity period after Bubblewonder Abyss loads. */
	bool requiresFidgetActivityResetOnLoad() const override { return true; }
	/** Select the fixed Bubblewonder Abyss F1 help prompt after maze construction. */
	void initHelpPrompt() override;

protected:
	/** Move surviving runners to the accepted exit side for debug Go handling. */
	void debugPrepareForDeparture() override;
	/** Describe the generated maze rule for diagnostics. */
	Common::String debugGetAnswer() const override;
	/** Process cell, collision, and exit animation callbacks. */
	void onFeatureAnimEvent(ZmbFeature *feature, int16 eventCode) override;
	/**
	 * Queue the exact-flag depth reorder after an upper-left or upper-right arrival walk completes.
	 * Lower-left and lower-right arrivals are bare Snoids owned by positional Z-sorting.
	 */
	void onSnoidWalkCompleted(ZmbSnoid *snoid) override;
	/** Update pre-render control state. */
	void onEveryFrame() override;
	/** Process rendered callbacks, traversal queues, and celebrations. */
	void onPostRenderFrame() override;
	/** Return the sound priority used by maze SCRB playback. */
	const ZoombiniPage::ScriptSoundPriorityRanges &getScriptSoundPriorityRanges() const override;
	/** Start a maze drag or handle a page control click. */
	ZmbEventHandleResult onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) override;
	/** Constrain drag movement to the maze's entrance region. */
	void endDrag(const Common::Point &dropPos) override;
	/** Return the maze-specific drag constraint rectangle. */
	const Common::Rect &getDragConstraintRect() const override;

private:
	/** Authored four-way movement directions used by Maze arrows and launchers. */
	enum MazeDirection : int16 {
		/** One column toward the left. */
		kMazeDirection00_West = 0,
		/** One row toward the bottom. */
		kMazeDirection01_South = 1,
		/** One column toward the right. */
		kMazeDirection02_East = 2,
		/** One row toward the top. */
		kMazeDirection03_North = 3
	};

	/** Waiting terrain group and zero-based index into the per-corner state arrays. */
	enum MazeCornerGroup : int16 {
		/** Lower-left starting and return area used at every difficulty. */
		kLowerLeft00 = 0,
		/** Upper-left intermediate area reachable only at level 4. */
		kUpperLeft01 = 1,
		/** Lower-right intermediate area reachable only at level 4. */
		kLowerRight02 = 2,
		/** Upper-right accepted area used at every difficulty. */
		kUpperRight03 = 3
	};

	/** Base shape indices of the seven colored tBMP 5100 cell-shape banks. */
	enum class MazeColorShapeBase : int16 {
		kRed52 = 52,
		kOrange73 = 73,
		kYellow94 = 94,
		kGreen115 = 115,
		kCyan136 = 136,
		kPurple157 = 157,
		kMagenta178 = 178
	};

	/** Authored behavior codes for one Maze layout cell. */
	enum MazeCellType : int16 {
		/** Empty grid location with no device behavior. */
		kMazeCellType00_Empty = 0,
		/** Whirlpool that rejects the incoming Zoombini. */
		kMazeCellType01_Whirlpool = 1,
		/** Conditional arrow that redirects only a matching Zoombini. */
		kMazeCellType02_TraitArrow = 2,
		/** Ordinary arrow that may rotate after a Zoombini passes. */
		kMazeCellType03_FixedArrow = 3,
		/** Colored arrow controlled by a matching switch group. */
		kMazeCellType04_ColoredArrow = 4,
		/** Colored Sticky cell that holds the incoming Zoombini. */
		kMazeCellType05_ColoredSticky = 5,
		/** Colored switch that rotates and releases its matching group. */
		kMazeCellType06_ColoredSwitch = 6,
		/** (NO-OP) Unused cell value with a dedicated visual shape, does not affect routing. */
		kMazeCellType07_Unused = 7,
		/** Boundary exit at the lower-left corner. */
		kMazeCellType20_ExitLowerLeft = 20,
		/** Level-4 boundary exit at the upper-left intermediate corner. */
		kMazeCellType21_ExitUpperLeft = 21,
		/** Level-4 boundary exit at the lower-right intermediate corner. */
		kMazeCellType22_ExitLowerRight = 22,
		/** Successful boundary exit at the upper-right corner. */
		kMazeCellType23_ExitUpperRight = 23,
	};

	/**
	 * Per-Zoombini traversal state for Bubblewonder Abyss.
	 *
	 * Grid cells have a separate lifetime.
	 * This record contains the runner's current logical cell, render position,
	 * rejection/exit flags, and the SCRS table selected from the runner's feet
	 * trait and travel direction.
	 */
	struct MazeRunnerState {
		/** Current grid column. */
		int16 col = 0;
		/** Current grid row. */
		int16 row = 0;
		/** Previous grid column. */
		int16 oldCol = 0;
		/** Previous grid row. */
		int16 oldRow = 0;
		/** Authored movement direction. */
		int16 direction = kMazeDirection00_West;
		/** Waiting terrain group most recently reached through a Maze boundary. */
		MazeCornerGroup cornerGroup = kLowerLeft00;
		/** Render position X in pixels. */
		int16 pixelX = 0;
		/** Render position Y in pixels. */
		int16 pixelY = 0;
		/** Bubble feature owned by this runner. */
		ZmbFeature *bubbleFeature = nullptr;
		/** Collision or exit overlay feature owned by this runner. */
		ZmbFeature *overlayFeature = nullptr;
		/** Launcher seat from which the runner entered. */
		int16 launcherSeatIdx = -1;
		/** Assigned corner waiting position. */
		int16 cornerArrivalPosIdx = -1;
		/** Exit phase: boundary effect, handoff, or feet celebration. */
		int16 exitStage = 0;
		/** Feet trait value used to select the movement table. */
		byte feetTrait = 0;
		/** Whether the runner has been placed on the grid. */
		bool placed = false;
		/** Whether the runner is moving through the grid. */
		bool moving = false;
		/** Whether a sticky cell is holding the runner. */
		bool waiting = false;
		/** Whether rejection SCRS playback is active. */
		bool rejecting = false;
		/** Whether the boundary turn/exit SCRS is active. */
		bool boundaryExit = false;
		/** Whether this runner has collided with another runner. */
		bool collided = false;
		/** Whether the collision companion has handed off to the pop SCRS. */
		bool collisionPopStarted = false;

		/** SCRS IDs for directional walk, alternate walk, idle, special, and feet entries. */
		int16 scrsTable[12];

		void clear() {
			col = row = oldCol = oldRow = 0;
			direction = kMazeDirection00_West;
			cornerGroup = kLowerLeft00;
			pixelX = pixelY = 0;
			bubbleFeature = nullptr;
			overlayFeature = nullptr;
			launcherSeatIdx = cornerArrivalPosIdx = -1;
			exitStage = 0;
			feetTrait = 0;
			placed = moving = waiting = rejecting = boundaryExit = collided = false;
			collisionPopStarted = false;
			memset(scrsTable, 0, sizeof(scrsTable));
		}
	};

	/**
	 * One device cell decoded from a Maze layout 16600-series REGS resource.
	 *
	 * @remark Maze layout is comprised of severals REGS resources.
	 * REGS 16600-series define logical cell behavior.
	 * REGS 16000 supplies screen anchors.
	 * REGS 18000/18001 supply tBMP 5100 shape registration offsets.
	 */
	struct MazeCellState {
		/** Cell behavior code from the authored grid. */
		MazeCellType type = kMazeCellType00_Empty;
		/** Grid row occupied by the cell. */
		int16 row = 0;
		/** Grid column occupied by the cell. */
		int16 col = 0;
		/** Activation wave group assigned to the cell. */
		int16 waveGroup = 0;
		/** Direction availability flags for the cell. */
		bool dirFlags[4] = {false, false, false, false};
		/** Current direction output of the cell. */
		int16 direction = 0;
		/** Whether passing the cell cycles its direction. */
		bool cycleOnPass = false;
		/** Trait category required by the cell. */
		int16 traitCategory = 0;
		/** Trait value required by the cell. */
		int16 traitValue = 0;
		/** Base shape ID used by the cell. */
		int16 shapeBase = 0;
		/** Runner currently held by the cell, or -1. */
		int16 heldRunner = -1;
		/** Frame at which the cell activation began. */
		int16 activationFrame = 0;
		/** Whether the cell's visual activation is active. */
		bool activationActive = false;
		/** Fixed number of frames between visual advances. */
		uint16 visualFrameInterval = 0;
		/** Next frame at which the cell visual may advance. */
		uint32 nextVisualFrame = 0;
	};

	/**
	 * One condition cell whose trait test the restored-layout repair replaced.
	 * The cell keeps its authored trait category; only the value it looks for
	 * moves, so the recorded pair is what the cell asked before the substitution.
	 * @remark Only used for Maze L4 Restored Layout.
	 */
	struct MazeConditionSubstitution {
		/** Index into the maze cell list. */
		int16 cellIdx = -1;
		/** Trait category the cell tested as authored. */
		int16 fromCategory = 0;
		/** Trait value the cell tested as authored. */
		int16 fromValue = 0;
	};

	/** Maze-specific REGS layout selector, separate from puzzle difficulty. */
	enum MazeLayoutLevel : int16 {
		kMazeLayoutLevel1 = 1,
		kMazeLayoutLevel2 = 2,
		kMazeLayoutLevel3 = 3,
		kMazeLayoutLevel4 = 4,
		/** Fixed REGS 16609 layout used by small level-4 packs. */
		kMazeLayoutLevel4SmallPack = 5
	};

	/** Page-local maze cells, arrows, bubbles, exits, and animations. */
	enum PageResourceId : int16 {
		kResBackground5000 = 5000,

		// Terrain bitmaps
		kResBitmapTerrain100 = 100,

		// Shape bitmaps
		kResBitmapShape5100_Creature = 5100,
		kResBitmapShape6000_GoMapButtons = 6000,
		kResBitmapShape7000_Maze = 7000,
		kResBitmapShape8000_Mudball = 8000,
		kResBitmapShape9000_Path = 9000,
		kResBitmapShape10000_Cell = 10000,
		kResBitmapShape12000_Exit = 12000,

		kResRegs16000_Maze = 16000,
		kResRegs16501_Collision = 16501,
		kResRegs16600_MazeLayoutL1Base = 16600,
		kResRegs16601_MazeLayoutL1Alt = 16601,
		kResRegs16602_MazeLayoutL2Base = 16602,
		kResRegs16603_MazeLayoutL2Alt = 16603,
		kResRegs16604_MazeLayoutL3Base = 16604,
		kResRegs16605_MazeLayoutL3Alt = 16605,
		kResRegs16606_MazeLayoutL4Base = 16606,
		kResRegs16607_MazeLayoutL4Unused = 16607, // Not selected by the level-4 variant cycle, which steps by two.
		kResRegs16608_MazeLayoutL4Alt = 16608,
		kResRegs16609_MazeLayoutL4Except = 16609,
		kResRegs17000_Exit = 17000,
		kResRegs18000_Entrance = 18000,

		kResScrb7000_CreatureBase = 7000,
		kResScrb7014_HighlightBase = 7014,
		kResScrb8000_MudballBase = 8000,
		kResScrb8001_Foreground = 8001,
		kResScrb8002_ForegroundAlt = 8002,
		kResScrb8003_ForegroundThird = 8003,
		kResScrb8004_FinalOverlayA = 8004,
		kResScrb8005_FinalForeground = 8005,
		kResScrb8006_Obstacle = 8006,
		kResScrb8007_ObstacleAlt = 8007,
		kResScrb8008_FinalForegroundAlt = 8008,
		kResScrb8009_FinalOverlay = 8009,
		kResScrb8010_FinalForegroundThird = 8010,
		kResScrb8011_Noop = 8011,
		kResScrb9000_PathBase = 9000,
		kResScrb9001_PathShadow = 9001,
		kResScrb9003_PathCreature = 9003,
		kResScrb9005_CreatureBase = 9005,
		kResScrb9006_CreatureSlot = 9006,
		kResScrb9007_CreatureSlotAlt = 9007,
		kResScrb10000_CellBase = 10000,
		kResScrb10003_CollisionBubbleVariant = 10003,
		kResScrb10004_CollisionBubble0 = 10004,
		kResScrb10005_CollisionBubble1 = 10005,
		kResScrb10006_CollisionBubble2 = 10006,
		kResScrb10007_CollisionBubble3 = 10007,
		kResScrb10008_CollisionBubble4 = 10008,
		kResScrb10009_CollisionBubble5 = 10009,
		kResScrb10010_CollisionBubble6 = 10010,
		kResScrb10011_CollisionBubble7 = 10011,
		kResScrb10012_CollisionOverlay0 = 10012,
		kResScrb10015_CollisionOverlay1 = 10015,
		kResScrb10018_CollisionOverlay2 = 10018,
		kResScrb10021_CollisionOverlay3 = 10021,
		kResScrb10024_CollisionOverlay4 = 10024,
		kResScrb10027_CollisionOverlay5 = 10027,
		kResScrb10030_BubbleBase = 10030,
		kResScrb10031_BubbleOverlay = 10031,
		kResScrb10040_ExitBubbleBase = 10040,
		kResScrb10041_BubbleOverlayAlt = 10041,
		kResScrb10042_CellTraversalAlt0 = 10042,
		kResScrb10043_CellTraversalAlt1 = 10043,
		kResScrb10044_CellTraversalAlt2 = 10044,
		kResScrb12000_ExitBase = 12000,
		kResScrb12001_Overlay = 12001,

		kResScrs14000_RejectBase = 14000,
		kResScrs14004_RejectFinal = 14004,
		kResScrs14006_RejectFlight = 14006,
		kResScrs14007_RejectAlt = 14007,
		kResScrs14999_IdleBase = 14999,
		kResScrs15000_NormalBase = 15000,
		kResScrs15004_Special1Base = 15004,
		kResScrs15009_Special2Base = 15009,
		kResScrs15014_Direction0Base = 15014,
		kResScrs15019_Direction1Base = 15019,
		kResScrs15024_Direction2Base = 15024,
		kResScrs15029_Direction3Base = 15029,
		kResScrs15035_CollisionBase0 = 15035,
		kResScrs15040_CollisionBase1 = 15040,
		kResScrs15045_CollisionBase2 = 15045,
		kResScrs15050_CollisionBase3 = 15050,
		kResScrs15055_Direction0AltBase = 15055,
		kResScrs15060_Direction1AltBase = 15060,
		kResScrs15065_Direction2AltBase = 15065,
		kResScrs15070_Direction3AltBase = 15070,
		kResScrs15075_FeetBase = 15075,
		kResScrs15080_TraversalBase = 15080,
		kResScrs15085_TraversalBase = 15085,
		kResScrs15090_CelebrationBase = 15090,

		kResSound5101_ImpactAlternate0 = 5101,
		kResSound5102_ImpactAlternate1 = 5102,
		kResSound5103_ImpactFinal = 5103,

		// Sound ranges used by getScriptSoundPriority().
		kResSoundRange5100_ImpactBase = 5100,
		kResSoundRange5103_ImpactFinal = 5103,
		kResSoundRange5104_ImpactLast = 5104,
		kResSoundRange9000_PathBase = 9000,
		kResSoundRange9001_PathShadow = 9001,
		kResSoundRange10000_CellBase = 10000,
		kResSoundRange10001_CollisionOverlay0 = 10001,
		kResSoundRange10002_CollisionBubbleStart = 10002,
		kResSoundRange10004_CollisionBubble0 = 10004,
		kResSoundRange11000_SpecialBase = 11000,
		kResSoundRange12000_ExitBase = 12000,

		kResMidi30035_MazeBgmBase = 30035
	};

	/** Grid entity lifecycle and collision callbacks. */
	enum GridEntityEventCode : int16 {
		kGridEntityEventCode000_ToggleFacing = 0,
		kGridEntityEventCode020_CollisionReservePhase0 = 20,
		kGridEntityEventCode021_CollisionAdvancePhase0 = 21,
		kGridEntityEventCode030_CollisionReservePhase1 = 30,
		kGridEntityEventCode031_CollisionAdvancePhase1 = 31,
		kGridEntityEventCode040_CollisionReservePhase2 = 40,
		kGridEntityEventCode041_CollisionAdvancePhase2 = 41,
		kGridEntityEventCode050_CollisionReservePhase3 = 50,
		kGridEntityEventCode051_CollisionAdvancePhase3 = 51,
		kGridEntityEventCode061_CollisionAdvancePhase4 = 61,
		kGridEntityEventCode063_RaiseCollisionOverlay = 63,
		kGridEntityEventCode091_FinishBoundaryExit = 91,
		kGridEntityEventCode092_ReleaseRejectedRunner = 92
	};

	/** Launcher SCRB callbacks use numeric event codes from the resource scripts. */
	enum LauncherEventCode : int16 {
		kLauncherEventCode050_LoadCreatureCompanion = 50,
		kLauncherEventCode061_StartDirectTraversal = 61,
		kLauncherEventCode062_StartNormalTraversal = 62,
		kLauncherEventCode064_QueueTraversalColumnRelink = 64,
		kLauncherEventCode065_StartInitialTraversal = 65,
		kLauncherEventCode066_FinishLauncher = 66,
		kLauncherEventCode071_StartTableTraversal = 71,
		kLauncherEventCode072_StartDirectTraversal = 72,
		kLauncherEventCode074_QueueTraversalColumnRelinkAlt = 74,
		kLauncherEventCode075_StartInitialTraversalAlt = 75,
		kLauncherEventCode076_FinishLauncherAlt = 76,
		kLauncherEventCode081_StartTableTraversalAlt = 81,
		kLauncherEventCode082_StartDirectTraversalAlt = 82,
		kLauncherEventCode084_QueueTraversalColumnRelinkAlt2 = 84,
		kLauncherEventCode085_StartInitialTraversalAlt = 85,
		kLauncherEventCode086_FinishLauncherAlt2 = 86
	};

	/** Terminal outcome of a read-only launcher-route simulation. */
	enum class LaunchSimulationOutcome : int16 {
		/** The route used an invalid direction, left the grid, or failed to advance. */
		kInvalidTraversalMinus04 = -4,
		/** The route did not terminate within the simulation step limit. */
		kStepLimitExceededMinus03 = -3,
		/** The route stopped at a colored Sticky cell. */
		kHeldAtColoredStickyMinus02 = -2,
		/** The route entered a whirlpool and was rejected. */
		kRejectedByWhirlpoolMinus01 = -1,
		/** The route returned through the lower-left boundary. */
		kExitLowerLeft00 = 0,
		/** The route returned through the level-4 upper-left boundary. */
		kExitUpperLeft01 = 1,
		/** The route returned through the level-4 lower-right boundary. */
		kExitLowerRight02 = 2,
		/** The route reached the accepted upper-right completion boundary. */
		kExitUpperRight03 = 3
	};

	/** Simulate one legal launcher choice without changing the live Maze state. */
	LaunchSimulationOutcome simulateNextLaunch(const ZmbSnoid *snoid, int16 seatIdx, Common::Array<int16> *switchCells = nullptr) const;

	// --- Initialization ---
	/** Select the level-specific 16600-series Maze layout REGS resource. */
	void loadRegsConfigByLevel();
	/** Read the launcher header and cell records from the selected layout REGS resource. */
	void loadAndParseRegsData();
	/** Create the authored creature launcher features. */
	void createCreatureFeatures();
	/** Create remaining grid, bubble, and overlay features. */
	void createRemainingCreatureFeatures();

	/**
	 * Move the drop-target helper of every seat whose launcher is @ref kResScrb9003_PathCreature
	 * to just behind that launcher, so the launcher's full-coverage dormant badge cannot erase it.
	 */
	void relinkSolidLauncherDropTargetHelpers();
	/** Load the Maze REGS tables for cell anchors, offsets, and collision overlays. */
	void loadRegsCoordinateTables();
	/** Generate the complete grid during page setup. */
	void initGridAndSelectPaths();
	/** Place base nodes on empty grid from static tables. */
	void generateBaseNodes();
	/** Initialize the SCRS animation table for a runner based on feet trait. */
	void initRunnerAnimTable(int16 runnerIdx);
	/** Initialize all active Zoombini animation tables. */
	void initAllRunnerAnimTables();

	// Path selection ---

	/** Clear the partial and complete trait records used during path selection. */
	void clearTraitSelectionTables();
	/** Collect Zoombini traits into @ref ZoombiniPuzzleMaze::_pathCandidateTraits. */
	void collectZmbTraits();
	/**
	 * Remove candidates matching @p traitSlotIdx, rebuild @ref ZoombiniPuzzleMaze::_pathTraitMatchCounts,
	 * and return the remaining candidate count.
	 */
	int16 removeMatchingPathCandidatesAndRecount(int16 traitSlotIdx);
	/** Rebuild the working candidate records with only Zoombinis matching @p traitSlotIdx. */
	int16 collectMatchingPathCandidates(int16 traitSlotIdx);
	/** Init connection table from free slots. */
	void initConnectionTable();
	/** Rebuild reachability list. */
	int16 rebuildReachabilityList();
	/** Fallback: all slots reachable. */
	void initAllSlotsReachable();
	/** Find best slot in score range. */
	int16 findBestTraitSlotInRange(int16 minScore, int16 maxScore);
	/** Find highest-scored slot of same category. */
	int16 findHighestScoredSlotInRange(int16 excludeSlot, int16 minScore, int16 maxScore);
	/** Find highest-scored slot excluding one. */
	int16 findHighestScoredSlot(int16 excludeSlot);
	/** Get score for slot. */
	int16 getTraitMatchCount(int16 traitSlotIdx) const;
	/** Count path slots with a nonzero trait-match count. */
	int16 countScoredPathSlots();
	/** Build the sparse list of path slots with zero current matches. */
	void initFreePathSlotList();
	/** Count the nonzero entries in the sparse free-path-slot list. */
	int16 countFreePathSlots();
	/** Find best next slot with uniqueness. */
	int16 findBestNextSlot(int16 searchIdx);
	/** Commit best slot. */
	int16 commitBestTraitSlot(int16 maxThreshold, int16 minThreshold);
	/** Find and commit next slot. */
	int16 findAndCommitNextSlot(int16 slotIdx, int16 direction);
	/** Find and commit new trait slot. */
	int16 findAndCommitNewTraitSlot(int16 maxThreshold, int16 minThreshold);
	// Path selection algorithms (dispatched by difficulty)
	/** Build the level 1 path assignment. */
	void buildZmbAssignmentAlt2();
	/** Build the level 2 variant 0 path assignment. */
	void buildZmbAssignmentAlt();
	/** Select path slots for level 2 variant 1 and level 3 variant 0. */
	void selectPathSlots2();
	/** Select path slots for level 3 variant 1 and low-count level 4. */
	void selectPathSlots();
	/** Build the level 4 path assignment list. */
	void buildZmbAssignmentList();
	/** Select a random reachable path slot. */
	int16 randomReachableSlot();
	/** Select a random slot from the sparse free-path-slot list. */
	int16 randomFreePathSlot();

	// --- Grid device initialization/rendering ---
	/** Initialize the page-owned state decoded from all layout cell records. */
	void initGridCellStates();
	/** Decode and register one layout cell state. */
	void registerGridCellState();
	/** Render the 13x13 maze cell layer. */
	ZmbRenderResult renderGridCells(ZmbFeature *feature);
	/** Adjust ordinary bubble hotspots to the current grid cell position. */
	void adjustBubbleHotspotPosition(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);
	/** Adjust the exit bubble hotspot to the current grid cell position. */
	void adjustExitBubbleHotspotPosition(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);
	/** Draw one cell shape at its grid position. */
	void drawCellShape(int16 shapeId, const Common::Point &cellPos);
	/** Return the mutable Maze cell state at @p cellIdx. */
	MazeCellState *getCellState(int16 cellIdx);
	/** Return the read-only Maze cell state at @p cellIdx. */
	const MazeCellState *getCellState(int16 cellIdx) const;
	/** Convert row and column coordinates to the flat cell index. */
	int16 getCellStateIndex(int16 row, int16 col) const;

	// Process the queues from @ref ZoombiniPuzzleMaze::onPostRenderFrame().

	/** Process all Maze queues in their authored frame order. */
	void processQueues();
	/** Start SCRB setup for queued launcher seats. */
	void processLauncherSetupQueue();
	/** Advance queued runner movement. */
	void processMovementQueue();
	/** Raise queued collision companions before the page overlay anchor. */
	void processCollisionOverlayRelinkQueue();
	/** Relink queued traversal Snoids before their launch-column depth anchors. */
	void processTraversalColumnRelinkQueue();
	/** Start queued feet-specific celebrations for accepted runners. */
	void processAcceptedCelebrationQueue();
	/** Apply level-4 upper-left and shared upper-right arrival depth reorders before exit completion work. */
	void processArrivalDepthReorderRequests();
	/** Complete queued rejection and boundary-exit sequences. */
	void processExitCompletionQueue();
	/** Dispatch runners whose movement SCRS reached the next cell. */
	void processCellArrivalQueue();
	/** Assign paired runners to collision-crossing SCRBs. */
	void processCollisionPairQueue();

	// --- Queue helpers ---

	/** Load the authored launcher SCRB for a queued seat. */
	void setupLauncherScrb(int16 launcherSeatIdx);
	/** Start the initial traversal script for a runner. */
	void startInitialTraversal(int16 runnerIdx, ZmbScrsCompletionMode completionMode);

	/** Move runner one cell on grid. */
	void moveZmbOnGrid(int16 runnerIdx);

	/** Dispatch based on cell type after row change. */
	void handleRowChange(MazeCellType cellType, int16 runnerIdx);

	/** Process runner arrival at exit. */
	void handleArrival(MazeCornerGroup cornerGroup, int16 runnerIdx);
	/** Seed the per-direction waiting-position counters to their page-setup values. */
	void resetArrivalPosCounters();
	/** Sort an exact Snoid flag group by its current visual depth and link it after column zero. */
	void reorderExactFlagSnoidGroupByDepth(uint32 exactFlags);

	/** Assign crossing SCRB pairs, which handles collision between two snoids. */
	void assignCrossRunnerScrbs(int16 runner1Idx, int16 runner2Idx);

	/**
	 * Handle an animation event raised by a collision companion overlay.
	 * @return true when @p feature is a companion overlay and the event was consumed.
	 */
	bool handleCollisionOverlayEvent(ZmbFeature *feature, int16 eventCode);

	// --- Cell type routing (row-change handler) ---

	/** Turn node arrival. */
	void zmbArriveAtNode(MazeCellType cellType, int16 runnerIdx);

	/** Straight node arrival (alt). */
	void zmbArriveAtNodeAlt(int16 cellIdx, int16 runnerIdx);

	/** Intersection movement step. */
	void moveRunnerStep(int16 cellIdx, int16 runnerIdx);

	/** Trait matching movement step. */
	void moveRunnerStepAlt(int16 cellIdx, int16 runnerIdx);

	/** Setup collision tracking at hitchhiker. */
	void zmbSetupCollisionTracking(int16 cellIdx, int16 runnerIdx);

	/** Activate a wave-group switch and update its linked cell states. */
	void activateWaveGroupSwitch(int16 cellIdx);
	/** Finish a runner's exit stage and update arrival counts. */
	void finishRunnerExit(int16 runnerIdx);
	/** Release bubble, overlay, and traversal visual ownership. */
	void releaseRunnerTraversalVisuals(int16 runnerIdx);
	/** Remove a placed runner from its cell and page maps. */
	void removePlacedRunner(int16 runnerIdx);
	/** Activate a runner bubble at a position. */
	void activateRunnerBubble(int16 runnerIdx, int16 scrbId, const Common::Point &pos);
	void activateRunnerBubble(int16 runnerIdx, int16 scrbId, const Common::Point &pos,
							  uint32 flags);

	// --- Click handling ---

	/** Map the shared empty drop target at a point to its authored launcher seat. */
	int16 findSeatAtPoint(const Common::Point &pos) const;

	/** Find an idle snoid at the given point for drag start. */
	ZmbSnoid *findIdleSnoidAtPoint(const Common::Point &pos) const;

	/** Drop a zoombini onto a grid seat and start moving. */
	void handleGridDrop(int16 seatIdx, ZmbSnoid *snoid);

	// --- SCRB animation dispatch ---

	/** SCRB animation event handler. */
	void processScrbAnimEvent(ZmbFeature *feature, int16 eventCode);

	/** Collision tracking callback. */
	void handleTraversalGridEvent(int16 eventCode, int16 runnerIdx);

	// --- Celebration / idle ---
	/** Select and start idle celebration animations for eligible Snoids. */
	void processIdleAnimations();

	// --- Helpers ---

	/**
	 * Translate the maze's authored direction code to a canonical grid direction.
	 *
	 * The @ref ZoombiniPuzzleMaze::MazeDirection encoding is 0=West, 1=South, 2=East, and 3=North. It differs from
	 * @ref ZmbGridDirection, so every cell advance goes through this translation.
	 */
	static bool decodeMazeGridDirection(int16 direction, ZmbGridDirection &gridDirection);

	/** Get trait value by category (1=hair, 2=eye, 3=nose, 4=feet). */
	static byte getTraitByCategory(const ZmbTrait &trait, int16 category);

	/** Get MazeRunnerState for a runner index. */
	MazeRunnerState *getRunnerState(int16 runnerIdx);

	/** Find runner index by snoid ID. */
	int16 findRunnerBySnoidId(uint16 snoidId) const;

	/** Find runner index by feature ID. */
	int16 findRunnerByFeatureId(uint16 featureId) const;

	/** Find seat index by creature obstacle/shadow feature identity. */
	int16 findSeatByFeature(const ZmbFeature *feature) const;

	// =================================================================
	// Static data tables
	// =================================================================

	/** Pedestal positions for zoombini lineup. */
	static constexpr Common::Point kSnoidPositions[20]{
		Common::Point(287, 394),
		Common::Point(260, 426),
		Common::Point(224, 447),
		Common::Point(188, 441),
		Common::Point(157, 455),
		Common::Point(263, 384),
		Common::Point(219, 397),
		Common::Point(184, 388),
		Common::Point(155, 402),
		Common::Point(121, 417),
		Common::Point(226, 354),
		Common::Point(189, 349),
		Common::Point(156, 354),
		Common::Point(131, 375),
		Common::Point(85, 394),
		Common::Point(164, 311),
		Common::Point(125, 324),
		Common::Point(79, 352),
		Common::Point(29, 318),
		Common::Point(15, 285),
	};

	/** Record whether each creature slot from 0 through 13 casts a shadow. */
	static constexpr int16 kCreatureHasShadow[14]{
		0,
		0,
		0,
		1,
		1,
		1,
		1,
		1,
		1,
		0,
		0,
		0,
		1,
		1,
	};

	/** Creature type ID per slot: 0=base, 1=type1, 2=type2. */
	static constexpr int16 kCreatureTypeId[14]{
		0,
		0,
		0,
		0,
		0,
		0,
		1,
		1,
		1,
		2,
		2,
		2,
		2,
		2,
	};

	/**
	 * Creature launcher SCRB table indexed by slot.
	 * A creature shadow, when present, uses the next SCRB ID after the slot's base ID.
	 */
	static constexpr int16 kCreatureScrbTable[14]{
		kResScrb9000_PathBase,
		kResScrb9000_PathBase,
		kResScrb9000_PathBase,
		kResScrb9001_PathShadow,
		kResScrb9001_PathShadow,
		kResScrb9001_PathShadow,
		kResScrb9001_PathShadow,
		kResScrb9001_PathShadow,
		kResScrb9001_PathShadow,
		kResScrb9000_PathBase,
		kResScrb9000_PathBase,
		kResScrb9000_PathBase,
		kResScrb9003_PathCreature,
		kResScrb9003_PathCreature,
	};

	/**
	 * Pixel position per seat (x, y).
	 * Entry positions are stored as 14 packed x/y pairs.
	 */
	static constexpr Common::Point kSeatPositions[14]{
		Common::Point(101, 283),
		Common::Point(148, 282),
		Common::Point(188, 280),
		Common::Point(195, 275),
		Common::Point(203, 297),
		Common::Point(210, 316),
		Common::Point(95, 65),
		Common::Point(100, 81),
		Common::Point(104, 96),
		Common::Point(622, 271),
		Common::Point(576, 288),
		Common::Point(545, 287),
		Common::Point(543, 286),
		Common::Point(554, 308),
	};

	/**
	 * Pixel position per launcher SCRB.
	 * These match the absolute first-hotspot positions of SCRB 7000..7013.
	 * They are not Snoid snap points.
	 */
	static constexpr Common::Point kLauncherPositions[14]{
		Common::Point(91, 295),
		Common::Point(138, 294),
		Common::Point(178, 292),
		Common::Point(185, 287),
		Common::Point(193, 309),
		Common::Point(201, 328),
		Common::Point(85, 77),
		Common::Point(90, 93),
		Common::Point(94, 108),
		Common::Point(612, 283),
		Common::Point(576, 300),
		Common::Point(535, 299),
		Common::Point(533, 298),
		Common::Point(544, 320),
	};

	/** Grid coordinates per seat (row, col). */
	static constexpr Common::Point kSeatGridCoords[14]{
		Common::Point(0, 9),
		Common::Point(1, 9),
		Common::Point(2, 9),
		Common::Point(4, 10),
		Common::Point(4, 11),
		Common::Point(4, 12),
		Common::Point(3, 0),
		Common::Point(3, 1),
		Common::Point(3, 2),
		Common::Point(12, 10),
		Common::Point(11, 10),
		Common::Point(10, 10),
		Common::Point(8, 11),
		Common::Point(8, 12),
	};

	/** Facing-left selector per seat. The two lower-right seats face left. */
	static constexpr bool kSeatFacingLeft[14]{
		false,
		false,
		false,
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
	};

	/** Authored entry direction per seat. */
	static constexpr MazeDirection kSeatMoveDirection[14]{
		kMazeDirection00_West,
		kMazeDirection00_West,
		kMazeDirection00_West,
		kMazeDirection01_South,
		kMazeDirection01_South,
		kMazeDirection01_South,
		kMazeDirection01_South,
		kMazeDirection01_South,
		kMazeDirection01_South,
		kMazeDirection00_West,
		kMazeDirection00_West,
		kMazeDirection00_West,
		kMazeDirection03_North,
		kMazeDirection03_North,
	};

	/** Common-image indices used by Maze seat launchers. */
	enum SeatCommonImageIndex : byte {
		/** Front-facing common pose. */
		kSeatCommonImageIndex00_Front = 0,
		/** Launcher-specific common pose. */
		kSeatCommonImageIndex02_LauncherVariant = 2
	};

	/** Common-image index to materialize for each seat launcher. */
	static constexpr SeatCommonImageIndex kSeatCommonImageIndex[14]{
		kSeatCommonImageIndex00_Front,
		kSeatCommonImageIndex00_Front,
		kSeatCommonImageIndex00_Front,
		kSeatCommonImageIndex02_LauncherVariant,
		kSeatCommonImageIndex02_LauncherVariant,
		kSeatCommonImageIndex02_LauncherVariant,
		kSeatCommonImageIndex02_LauncherVariant,
		kSeatCommonImageIndex02_LauncherVariant,
		kSeatCommonImageIndex02_LauncherVariant,
		kSeatCommonImageIndex00_Front,
		kSeatCommonImageIndex00_Front,
		kSeatCommonImageIndex00_Front,
		kSeatCommonImageIndex02_LauncherVariant,
		kSeatCommonImageIndex02_LauncherVariant,
	};

	/** Base node cell types (18 entries covering each boundary exit). */
	static constexpr MazeCellType kBaseNodeTypes[18]{
		kMazeCellType20_ExitLowerLeft,
		kMazeCellType20_ExitLowerLeft,
		kMazeCellType20_ExitLowerLeft,
		kMazeCellType20_ExitLowerLeft,
		kMazeCellType20_ExitLowerLeft,
		kMazeCellType21_ExitUpperLeft,
		kMazeCellType21_ExitUpperLeft,
		kMazeCellType21_ExitUpperLeft,
		kMazeCellType21_ExitUpperLeft,
		kMazeCellType22_ExitLowerRight,
		kMazeCellType22_ExitLowerRight,
		kMazeCellType22_ExitLowerRight,
		kMazeCellType22_ExitLowerRight,
		kMazeCellType23_ExitUpperRight,
		kMazeCellType23_ExitUpperRight,
		kMazeCellType23_ExitUpperRight,
		kMazeCellType23_ExitUpperRight,
		kMazeCellType23_ExitUpperRight,
	};

	/** Base node coordinates (row, col) -- 18 pairs. */
	static constexpr Common::Point kBaseNodeCoords[18]{
		Common::Point(0, 10),
		Common::Point(1, 10),
		Common::Point(2, 10),
		Common::Point(2, 11),
		Common::Point(2, 12),
		Common::Point(0, 2),
		Common::Point(1, 2),
		Common::Point(1, 1),
		Common::Point(1, 0),
		Common::Point(10, 12),
		Common::Point(10, 11),
		Common::Point(11, 11),
		Common::Point(12, 10),
		Common::Point(10, 0),
		Common::Point(10, 1),
		Common::Point(10, 2),
		Common::Point(11, 2),
		Common::Point(12, 3),
	};

	/** Trait offset table: {0, 5, 10, 15} for Hair/Eyes/Nose/Feet. */
	static constexpr int16 kTraitOffsets[4]{
		0,
		5,
		10,
		15,
	};

	/**
	 * Arrival positions per direction: 4 directions x 20 positions.
	 * Coordinates are stored as packed x/y pairs.
	 */
	static constexpr Common::Point kArrivalPositions[80]{
		// Direction 0 - left/center exit
		Common::Point(287, 394),
		Common::Point(260, 426),
		Common::Point(224, 447),
		Common::Point(188, 441),
		Common::Point(157, 455),
		Common::Point(263, 384),
		Common::Point(219, 397),
		Common::Point(184, 388),
		Common::Point(155, 402),
		Common::Point(121, 417),
		Common::Point(226, 354),
		Common::Point(189, 349),
		Common::Point(156, 354),
		Common::Point(131, 375),
		Common::Point(85, 394),
		Common::Point(164, 311),
		Common::Point(125, 324),
		Common::Point(79, 352),
		Common::Point(29, 318),
		Common::Point(15, 285),
		// Direction 1 - top exit
		Common::Point(4, 86),
		Common::Point(29, 70),
		Common::Point(50, 68),
		Common::Point(70, 68),
		Common::Point(94, 63),
		Common::Point(7, 102),
		Common::Point(30, 86),
		Common::Point(55, 79),
		Common::Point(72, 79),
		Common::Point(8, 116),
		Common::Point(33, 100),
		Common::Point(56, 93),
		Common::Point(73, 93),
		Common::Point(9, 131),
		Common::Point(34, 115),
		Common::Point(99, 108),
		Common::Point(74, 108),
		Common::Point(57, 108),
		Common::Point(97, 79),
		Common::Point(98, 93),
		// Direction 2 - right exit
		Common::Point(633, 311),
		Common::Point(613, 311),
		Common::Point(590, 336),
		Common::Point(571, 340),
		Common::Point(551, 345),
		Common::Point(632, 297),
		Common::Point(612, 297),
		Common::Point(589, 321),
		Common::Point(570, 325),
		Common::Point(550, 330),
		Common::Point(629, 281),
		Common::Point(609, 281),
		Common::Point(588, 307),
		Common::Point(569, 311),
		Common::Point(549, 316),
		Common::Point(589, 291),
		Common::Point(566, 296),
		Common::Point(546, 300),
		Common::Point(634, 336),
		Common::Point(614, 336),
		// Direction 3 - right-top exit (completion direction)
		Common::Point(621, 18),
		Common::Point(624, 40),
		Common::Point(624, 64),
		Common::Point(625, 84),
		Common::Point(594, 20),
		Common::Point(598, 40),
		Common::Point(593, 60),
		Common::Point(594, 75),
		Common::Point(594, 90),
		Common::Point(556, 32),
		Common::Point(560, 50),
		Common::Point(555, 72),
		Common::Point(563, 94),
		Common::Point(511, 42),
		Common::Point(515, 60),
		Common::Point(521, 80),
		Common::Point(529, 100),
		Common::Point(476, 67),
		Common::Point(484, 89),
		Common::Point(594, 104),
	};

	/** Per-corner drag bounds. */
	/**
	 * Immutable drag bounds are instance members because Common::Rect requires
	 * runtime construction and ScummVM prohibits global C++ constructors.
	 */
	const Common::Rect _dragConstraintRects[4] = {
		Common::Rect(0, 250, 300, 479),
		Common::Rect(0, 0, 200, 175),
		Common::Rect(450, 230, 639, 400),
		Common::Rect(400, 0, 639, 200),
	};

	/** Creature companion SCRB table indexed by creature type (0/1/2). */
	static constexpr int16 kCreatureTypeScrbTable[3]{
		kResScrb9005_CreatureBase,
		kResScrb9006_CreatureSlot,
		kResScrb9007_CreatureSlotAlt,
	};

	/** Path selection threshold table. */
	static constexpr int16 kPathSelectThresholds[20]{
		0,
		1,
		1,
		2,
		2,
		3,
		3,
		4,
		4,
		5,
		5,
		6,
		6,
		7,
		7,
		8,
		8,
		0,
		1,
		0,
	};

	/** One-based trait categories used to group path slots. */
	enum PathSlotTraitCategory : byte {
		/** Reserved path slot with no trait category. */
		kPathSlotTraitCategory00_None = 0,
		/** Hair trait path slots. */
		kPathSlotTraitCategory01_Hair = 1,
		/** Eye trait path slots. */
		kPathSlotTraitCategory02_Eyes = 2,
		/** Nose trait path slots. */
		kPathSlotTraitCategory03_Nose = 3,
		/** Feet trait path slots. */
		kPathSlotTraitCategory04_Feet = 4
	};

	/** Trait category table indexed by path slot. */
	static constexpr PathSlotTraitCategory kSlotTraitCategories[21]{
		kPathSlotTraitCategory00_None,
		kPathSlotTraitCategory01_Hair,
		kPathSlotTraitCategory01_Hair,
		kPathSlotTraitCategory01_Hair,
		kPathSlotTraitCategory01_Hair,
		kPathSlotTraitCategory01_Hair,
		kPathSlotTraitCategory02_Eyes,
		kPathSlotTraitCategory02_Eyes,
		kPathSlotTraitCategory02_Eyes,
		kPathSlotTraitCategory02_Eyes,
		kPathSlotTraitCategory02_Eyes,
		kPathSlotTraitCategory03_Nose,
		kPathSlotTraitCategory03_Nose,
		kPathSlotTraitCategory03_Nose,
		kPathSlotTraitCategory03_Nose,
		kPathSlotTraitCategory03_Nose,
		kPathSlotTraitCategory04_Feet,
		kPathSlotTraitCategory04_Feet,
		kPathSlotTraitCategory04_Feet,
		kPathSlotTraitCategory04_Feet,
		kPathSlotTraitCategory04_Feet,
	};

	/** Convert a survivor score into the number of path candidates to examine. */
	static constexpr int16 kScoreToLoopCount[17]{
		1,
		1,
		1,
		1,
		4,
		4,
		4,
		7,
		7,
		7,
		10,
		10,
		10,
		13,
		13,
		13,
		16,
	};

	/** Provide the base shape index for each path-color group before shuffling. */
	static constexpr int16 kStaticPathPool[11]{
		0,
		31,
		52,
		73,
		94,
		115,
		136,
		157,
		178,
		0,
		0,
	};

	/** Select the SCRS variant and table offset associated with each seat. */
	static constexpr int16 kSeatFlagValue[14]{
		0,
		0,
		0,
		1,
		1,
		1,
		1,
		1,
		1,
		0,
		0,
		0,
		2,
		2,
	};

	/**
	 * Trait slot mapping: maps slot index (0-20) to trait category offset.
	 * @ref ZoombiniPuzzleMaze::kTraitSlotKind maps each slot to @ref ZmbTrait::TraitKind.
	 * The mapping covers the path slot indices from 0 through 20.
	 */
	static constexpr ZmbTrait::TraitKind kTraitSlotKind[21]{
		ZmbTrait::kTraitHair,
		ZmbTrait::kTraitHair,
		ZmbTrait::kTraitHair,
		ZmbTrait::kTraitHair,
		ZmbTrait::kTraitHair,
		ZmbTrait::kTraitHair, // 0-5
		ZmbTrait::kTraitEyes,
		ZmbTrait::kTraitEyes,
		ZmbTrait::kTraitEyes,
		ZmbTrait::kTraitEyes,
		ZmbTrait::kTraitEyes, // 6-10
		ZmbTrait::kTraitNose,
		ZmbTrait::kTraitNose,
		ZmbTrait::kTraitNose,
		ZmbTrait::kTraitNose,
		ZmbTrait::kTraitNose, // 11-15
		ZmbTrait::kTraitFeet,
		ZmbTrait::kTraitFeet,
		ZmbTrait::kTraitFeet,
		ZmbTrait::kTraitFeet,
		ZmbTrait::kTraitFeet, // 16-20
	};

	/**
	 * Trait slot mapping: maps slot index (0-20) to trait value (0-5).
	 * The mapping covers the path slot indices from 0 through 20.
	 */
	static constexpr int16 kTraitSlotValue[21]{
		0,
		1,
		2,
		3,
		4,
		5, // 0-5
		1,
		2,
		3,
		4,
		5, // 6-10
		1,
		2,
		3,
		4,
		5, // 11-15
		1,
		2,
		3,
		4,
		5, // 16-20
	};

	// =================================================================
	// Debug answer plan search helpers (static; no instance state)
	// =================================================================

	// Grid and pack limits shared by the solver-only plan types below.
	static const int kDebugMazeRows = 13;
	static const int kDebugMazeCols = 13;
	static const int kDebugMazeMaxCells = 64;
	static const int kDebugMazeMaxSnoids = 20;
	static const int kDebugMazeSeatCount = 14;
	static const int kDebugMazeMaxStagingKeys = 4;

	/** Cells whose arrow direction can change while the puzzle is played. */
	static const int kDebugMazeMaxControlCells = 16;
	/** Distinct combinations those cells can hold. */
	static const int kDebugMazeMaxControlStates = 48;
	/** Sticky cells the generic search tracks. */
	static const int kDebugMazeMaxStickyCells = 8;
	/** Control launches one generic plan may contain. */
	static const int kDebugMazeMaxControlSteps = 40;

	// Solver-only plan types used by the private answer-plan search helpers
	// below. They hold no instance state, so they live in this class instead
	// of at namespace scope.

	/** Read-only snapshot of one maze layout and its Zoombini pack that the answer-plan builders search against. */
	struct SolverPlanContext {
		MazeCellType cellTypes[kDebugMazeRows][kDebugMazeCols];
		int16 cellIndices[kDebugMazeRows][kDebugMazeCols];
		int16 traitCategories[kDebugMazeRows][kDebugMazeCols];
		int16 traitValues[kDebugMazeRows][kDebugMazeCols];
		bool directionFlags[kDebugMazeRows][kDebugMazeCols][4];
		int16 initialDirections[kDebugMazeRows][kDebugMazeCols];
		MazeCellState cells[kDebugMazeMaxCells];
		int16 cellCount;
		const ZmbSnoid *snoids[kDebugMazeMaxSnoids];
		int16 snoidCount;
		bool seatActive[kDebugMazeSeatCount];
		int16 seatCorner[kDebugMazeSeatCount];
		int16 seatRow[kDebugMazeSeatCount];
		int16 seatCol[kDebugMazeSeatCount];
		int16 seatDirection[kDebugMazeSeatCount];
	};

	/** Position of one Zoombini inside a simulated plan state. */
	struct SolverPlanSnoidState {
		int16 corner;
		int16 row;
		int16 col;
		int16 direction;
		int16 heldCellIdx;
		bool accepted;
	};

	/** Mutable maze state advanced while simulating an answer-plan launch sequence. */
	struct SolverPlanState {
		int16 directions[kDebugMazeRows][kDebugMazeCols];
		int16 stickyHeldSnoids[kDebugMazeMaxCells];
		SolverPlanSnoidState snoids[kDebugMazeMaxSnoids];
	};

	/** Outcome of one Zoombini passing through a single trait-condition cell during a simulated launch. */
	struct SolverTraitDecision {
		int16 cellIdx;
		bool matched;
	};

	/** Recorded effect of launching one Zoombini from one launcher in a plan, used to replay or report the step. */
	struct SolverPlanAction {
		int16 snoidIdx;
		int16 launcherSeatIdx;
		int16 rootOutcome;
		Common::Array<int16> switchGroups;
		Common::Array<int16> pushedSnoids;
		Common::Array<int16> releasedSnoids;
		Common::Array<SolverTraitDecision> traitDecisions;

		void clear() {
			snoidIdx = -1;
			launcherSeatIdx = -1;
			rootOutcome = -4;
			switchGroups.clear();
			pushedSnoids.clear();
			releasedSnoids.clear();
			traitDecisions.clear();
		}
	};

	/** Complete level-4 answer plan, holding the ordered key launches and their staged actions. */
	struct SolverLevel4Plan {
		bool valid;
		int16 primaryGroup;
		int16 finalStickyGroup;
		int16 rescueGroup;
		int16 highwayGroup;
		int16 stagingStickyGroup;
		int16 stagingCorner;
		int16 stagingKeyCount;
		int16 stagingKeys[kDebugMazeMaxStagingKeys];
		int16 stagingKey;
		int16 primaryKey;
		int16 pusherKey;
		int16 rescueKey;
		int16 finalKey;
		int16 highwaySeat;
		int16 highwaySnoidCount;
		bool rescueReleasesStagingKey;
		SolverPlanAction stageStagingActions[kDebugMazeMaxStagingKeys];
		SolverPlanAction stageHighwaySwitchActions[kDebugMazeMaxStagingKeys];
		SolverPlanAction stageStaging;
		SolverPlanAction stagePrimary;
		SolverPlanAction stageHighwaySwitch;
		SolverPlanAction stageRescue;
		SolverPlanAction stagePush;
		SolverPlanAction stageFinal;

		SolverLevel4Plan()
			: valid(false), primaryGroup(0), finalStickyGroup(0), rescueGroup(0), highwayGroup(0),
			  stagingStickyGroup(0), stagingCorner(0), stagingKeyCount(0), stagingKey(-1), primaryKey(-1), pusherKey(-1),
			  rescueKey(-1), finalKey(-1), highwaySeat(-1), highwaySnoidCount(0),
			  rescueReleasesStagingKey(false) {
			for (int16 keyIdx = 0; keyIdx < kDebugMazeMaxStagingKeys; keyIdx++)
				stagingKeys[keyIdx] = -1;
		}
	};

	/**
	 * One candidate assignment of colored groups to the level-4 relay roles.
	 *
	 * The staged-relay search must choose all four roles together because each
	 * group must be distinct and have the device behavior required by its role.
	 */
	struct SolverLevel4GroupSelection {
		/** Group activated by the primary key before the staging keys enter the highway. */
		int16 primaryGroup;
		/** Sticky group used to hold the primary key and later the pusher key. */
		int16 finalStickyGroup;
		/** Group whose switch releases or moves the staging relay. */
		int16 rescueGroup;
		/** Group activated while the staging keys traverse the highway. */
		int16 highwayGroup;
	};

	/**
	 * Candidate relay after its staging keys reached the highway phase.
	 *
	 * The state, keys, and recorded actions form one input to the search for
	 * the rescue, push, and final launches.
	 */
	struct SolverLevel4StagedRelay {
		/** Simulated state after every staging key has made its highway launch. */
		SolverPlanState postHighwayState;
		/** Number of staging keys and actions stored in the fixed-size arrays. */
		int16 stagingKeyCount;
		/** Selected keys that first moved to the common staging corner. */
		int16 stagingKeys[kDebugMazeMaxStagingKeys];
		/** Actions that transferred the staging keys to the common corner. */
		SolverPlanAction stagingActions[kDebugMazeMaxStagingKeys];
		/** Actions that sent the staging keys through the highway switch. */
		SolverPlanAction highwayActions[kDebugMazeMaxStagingKeys];
		/** Common corner occupied before the staging keys enter the highway. */
		int16 stagingCorner;
		/** Group of the sticky cell holding a staging key after the highway phase. */
		int16 stagingStickyGroup;
		/** Key and action that activated the primary switch before the highway phase. */
		int16 primaryKey;
		SolverPlanAction primaryAction;

		SolverLevel4StagedRelay() : stagingKeyCount(0), stagingCorner(-1), stagingStickyGroup(0), primaryKey(-1) {
			for (int16 keyIdx = 0; keyIdx < kDebugMazeMaxStagingKeys; keyIdx++)
				stagingKeys[keyIdx] = -1;
		}
	};

	/** Answer plan for maze levels one through three, including the optional two-switch relay phase. */
	struct SolverLevel123Plan {
		bool valid;
		bool hasSwitchPhase;
		int16 priorityCategory;
		int16 priorityValue;
		bool priorityMatched;
		bool precursorInPriorityGroup;
		int16 priorityCount;
		int16 remainderCount;
		int16 precursorKey;
		int16 victim;
		int16 finalKey;
		int16 precursorGroup;
		int16 finalGroup;
		int16 victimActionIdx;
		int16 finalActionIdx;
		Common::Array<SolverPlanAction> actions;

		SolverLevel123Plan()
			: valid(false), hasSwitchPhase(false), priorityCategory(0), priorityValue(0),
			  priorityMatched(false), precursorInPriorityGroup(false), priorityCount(0),
			  remainderCount(0), precursorKey(-1),
			  victim(-1), finalKey(-1), precursorGroup(0), finalGroup(0),
			  victimActionIdx(-1), finalActionIdx(-1) {
		}
	};

	/** One Zoombini mid-trace while simulating a launch; direction uses the authored encoding. */
	struct SolverMovingSnoid {
		int16 snoidIdx;
		int16 row;
		int16 col;
		/**
		 * 0=West
		 * 1=South
		 * 2=East
		 * 3=North
		 */
		int16 direction;
	};

	/** Terminal of one traced route. Sticky cells add their slot to the sticky base. */
	enum SolverRouteTerminal {
		kDebugMazeRouteGoal = -3,
		kDebugMazeRouteRejected = -2,
		kDebugMazeRouteStickyBase = 8
	};

	/** Where one Zoombini stands in a generic search node. */
	enum SolverSnoidSpot {
		kDebugMazeSpotAccepted = -3,
		kDebugMazeSpotHeld = -1
	};

	/** One traced route, precomputed per Zoombini class, launcher and control state. */
	struct SolverRouteResult {
		int16 terminal;
		int16 arriveDirection;
		int16 nextControlState;
		uint16 pressedGroups;
	};

	/** One full arrow-direction grid, passed by reference through the route tracer. */
	struct SolverDirectionGrid {
		int16 directions[kDebugMazeRows][kDebugMazeCols];
	};

	/**
	 * Read-only tables the generic search consults instead of resimulating.
	 * Zoombinis that answer every trait cell identically share one class, so the
	 * tables are indexed by class rather than by pack slot.
	 */
	struct SolverTables {
		int16 controlCellIdx[kDebugMazeMaxControlCells];
		int16 controlRows[kDebugMazeMaxControlCells];
		int16 controlCols[kDebugMazeMaxControlCells];
		int16 controlCellCount;
		int16 stickyCells[kDebugMazeMaxStickyCells];
		int16 stickyRows[kDebugMazeMaxStickyCells];
		int16 stickyCols[kDebugMazeMaxStickyCells];
		int16 stickyGroups[kDebugMazeMaxStickyCells];
		int16 stickyCount;
		int16 controlStates[kDebugMazeMaxControlStates][kDebugMazeMaxControlCells];
		int16 controlStateCount;
		int16 classOf[kDebugMazeMaxSnoids];
		int16 classRepresentative[kDebugMazeMaxSnoids];
		int16 classCount;
		int16 snoidCount;
		int16 seats[kDebugMazeSeatCount];
		int16 seatCorner[kDebugMazeSeatCount];
		int16 seatCount;
		int16 baseDirections[kDebugMazeRows][kDebugMazeCols];
		SolverRouteResult launchRoutes[kDebugMazeMaxSnoids][kDebugMazeSeatCount][kDebugMazeMaxControlStates];
		SolverRouteResult resumeRoutes[kDebugMazeMaxSnoids][kDebugMazeMaxStickyCells][4][kDebugMazeMaxControlStates];
	};

	/**
	 * One generic search node.
	 * @p covered marks Zoombinis that already had a side-effect-free route to the
	 * goal, so they no longer need a decision of their own.
	 */
	struct SolverSearchNode {
		int32 parent;
		uint32 covered;
		int8 controlState;
		int8 keySnoid;
		int8 keySeat;
		int8 depth;
		int8 spot[kDebugMazeMaxSnoids];
		int8 stickyHolder[kDebugMazeMaxStickyCells];
		int8 stickyDirection[kDebugMazeMaxStickyCells];
	};

	/** Working sets of the relaxed reachability test that prunes dead branches. */
	struct SolverReachSet {
		bool stateReached[kDebugMazeMaxControlStates];
		int16 stateList[kDebugMazeMaxControlStates];
		int16 stateCount;
		bool available[kDebugMazeMaxSnoids][3];
		bool occupant[kDebugMazeMaxStickyCells][kDebugMazeMaxSnoids][4];
		bool exitDirection[kDebugMazeMaxStickyCells][4];
	};

	/** One launch of a generic plan. */
	struct SolverGenericStep {
		int16 snoidIdx;
		int16 launcherSeatIdx;
		bool controlStep;
		int16 outcome;
		int16 stickyCellIdx;
		Common::Array<int16> switchGroups;
		Common::Array<int16> pushedSnoids;
		Common::Array<int16> releasedSnoids;

		SolverGenericStep()
			: snoidIdx(-1), launcherSeatIdx(-1), controlStep(false), outcome(-4), stickyCellIdx(-1) {
		}
	};

	/** One selected control launch in the generic solver's parent chain. */
	struct SolverKeyLaunch {
		/** Zoombini selected to perform the control launch. */
		int16 snoidIdx;
		/** Launcher selected for the control launch. */
		int16 launcherSeatIdx;
	};

	/**
	 * Ordered launch list that delivers every Zoombini of the pack.
	 * @p postSpots records where the launched Zoombini stands after each step, so
	 * the progress report can tell which steps the player has already made.
	 */
	struct SolverGenericPlan {
		bool valid;
		int16 controlStepCount;
		Common::Array<SolverGenericStep> steps;
		Common::Array<int16> postSpots;

		SolverGenericPlan() : valid(false), controlStepCount(0) {
		}
	};

	/** Search caps for one layout-agnostic answer-plan search. */
	struct SolverGenericSearchLimits {
		/** Maximum expanded control launches before the search stops. */
		uint32 maxSteps;
		/** Maximum retained search nodes before the search stops. */
		uint32 maxNodes;
	};

	/** Status published by one layout-agnostic answer-plan search. */
	struct SolverGenericSearchResult {
		/** Whether the search explored every reachable node without finding a plan. */
		bool searchExhausted;
		/** Whether a configured step or node cap stopped the search. */
		bool limitReached;
		/** Number of candidate control launches examined by the search. */
		uint32 stepsUsed;

		SolverGenericSearchResult() : searchExhausted(false), limitReached(false), stepsUsed(0) {
		}
	};

	/** Step budget for one bounded answer-plan search; limit 0 disables the cap. */
	struct SolverBudget {
		uint32 limit = 0;
		uint32 steps = 0;
	};

	/**
	 * Mutable state shared by every recursive call in one accepted-only search.
	 *
	 * The current @ref SolverPlanState remains a separate parameter because each
	 * recursive branch supplies its own candidate state. The requirements,
	 * optional final launch, accumulated actions, and visited set remain common
	 * to all branches.
	 */
	struct SolverAcceptedPlanSearch {
		/** Whether each pack slot must reach the accepted exit before completion. */
		bool requiredSnoids[kDebugMazeMaxSnoids];
		/** Optional final key launched after all required Snoids are accepted. */
		int16 finalSnoid;
		/** Switch group that the optional final key must press. */
		int16 finalSwitchGroup;
		/** Accepted launch sequence accumulated along the current recursive branch. */
		Common::Array<SolverPlanAction> actions;
		/** Simulated state that reached the requested accepted-plan outcome. */
		SolverPlanState resultState;
		/** Hashes of states already explored by this search. */
		Common::HashMap<uint64, bool> visited;
		/** Number of states inserted into @ref visited. */
		int32 visitedStateCount;

		SolverAcceptedPlanSearch() : finalSnoid(-1), finalSwitchGroup(0), visitedStateCount(0) {
			memset(requiredSnoids, 0, sizeof(requiredSnoids));
		}
	};

	/**
	 * Publishes a search budget's step count on every exit path.
	 * The plan builders return from many places, so the count is copied out here
	 * instead of at each return.
	 */
	struct SolverStepCounter {
		const SolverBudget &budget;
		uint32 &target;

		SolverStepCounter(const SolverBudget &searchBudget, uint32 &stepsUsed)
			: budget(searchBudget), target(stepsUsed) {
			target = 0;
		}

		~SolverStepCounter() {
			target = budget.steps;
		}
	};

	static bool debugMazeApplyPlanLaunch(const SolverPlanContext &context,
										 SolverPlanState &state, int16 snoidIdx, int16 seatIdx,
										 SolverPlanAction &action, SolverBudget &budget);
	static bool debugTransferMazeStagingKey(const SolverPlanContext &context,
											SolverPlanState &state, int16 snoidIdx,
											int16 requiredCorner, SolverPlanAction &action,
											SolverBudget &budget);
	static bool debugCompleteExpandedMazeLevel4Plan(
		const SolverPlanContext &context, const SolverLevel4StagedRelay &stagedRelay,
		SolverLevel4Plan &plan, SolverBudget &budget);
	static bool debugBuildMazeLevel4PlanForGroups(const SolverPlanContext &context,
												  const SolverLevel4GroupSelection &groupSelection,
												  SolverLevel4Plan &plan,
												  SolverBudget &budget);
	static bool debugBuildMazeLevel4Plan(const SolverPlanContext &context,
										 SolverLevel4Plan &plan, bool &searchExhausted,
										 uint32 &stepsUsed);
	static bool debugFindMazeAcceptedPlan(const SolverPlanContext &context, const SolverPlanState &state,
										  SolverAcceptedPlanSearch &search, SolverBudget &budget);
	static bool debugBuildMazeSimplePlan(const SolverPlanContext &context,
										 SolverLevel123Plan &plan,
										 SolverBudget &budget);
	static bool debugBuildMazeSwitchPlan(const SolverPlanContext &context,
										 SolverLevel123Plan &plan,
										 SolverBudget &budget);
	static bool debugBuildMazeLevel123Plan(const SolverPlanContext &context,
										   SolverLevel123Plan &plan,
										   SolverBudget &budget);

	// Generic (layout-agnostic) answer-plan search.
	static int16 debugMazeStickySlot(const SolverTables &tables, int16 cellIdx);
	static void debugMazeApplyControlState(const SolverTables &tables, int16 stateIdx,
										   SolverDirectionGrid &grid);
	static int16 debugMazeControlStateIndex(const SolverTables &tables,
											const SolverDirectionGrid &grid);
	static void debugMazeTraceRoute(const SolverPlanContext &context,
									const SolverTables &tables, const SolverMovingSnoid &start,
									SolverDirectionGrid &grid, SolverRouteResult &route);
	static bool debugMazeBuildSolverTables(const SolverPlanContext &context,
										   SolverTables &tables);
	static void debugMazeSolverNodeToPlanState(const SolverPlanContext &context,
											   const SolverTables &tables,
											   const SolverSearchNode &node,
											   SolverPlanState &state);
	static bool debugMazeSolverNodeFromPlanState(const SolverTables &tables,
												 const SolverPlanState &state,
												 SolverSearchNode &node);
	static int16 debugMazeSolverPlainSeat(const SolverTables &tables, int16 classIdx,
										  int16 corner, int16 controlState);
	static void debugMazeSolverUpdateCover(const SolverTables &tables,
										   SolverSearchNode &node);
	static int16 debugMazeSolverUncovered(const SolverTables &tables,
										  const SolverSearchNode &node);
	static uint64 debugMazeSolverNodeKey(const SolverTables &tables,
										 const SolverSearchNode &node);
	static bool debugMazeSolverAddState(SolverReachSet &reach, int16 stateIdx);
	static void debugMazeSolverExpandReach(const SolverTables &tables,
										   SolverReachSet &reach);
	static bool debugMazeSolverGoalReachable(const SolverTables &tables,
											 const SolverSearchNode &node);
	static bool debugMazeGenericPlanFromKeys(const SolverPlanContext &context,
											 const SolverTables &tables,
											 const Common::Array<SolverKeyLaunch> &keyLaunches,
											 SolverGenericPlan &plan);
	static bool debugBuildMazeGenericPlan(const SolverPlanContext &context,
										  SolverGenericPlan &plan, const SolverGenericSearchLimits &limits,
										  SolverGenericSearchResult &result);
	/** Fill the search context from the current grid, launchers, and pack. */
	void debugBuildMazePlanContext(SolverPlanContext &context) const;
	/** Whether the current layout and pack still have a complete solution. */
	bool isMazeLayoutSolvable(uint32 &stepsUsed) const;
	/** Retune one condition cell's trait test, updating router, search, and shape. */
	void setConditionCellTrait(int16 cellIdx, int16 category, int16 value);
	/**
	 * Retune one restored-layout condition cell when the arriving pack cannot
	 * finish REGS 16607 as authored. Returns whether a cell was changed.
	 */
	bool repairRestoredLayoutConditionCell();
	/** Describe one substitution in the wording the answer and the log share. */
	Common::String describeConditionSubstitution(const MazeConditionSubstitution &substitution) const;
	/** Tell the player which condition cells the layout repair substituted. */
	void logConditionSubstitutions(uint32 stepsUsed) const;
	int16 debugMazeLiveSnoidSpot(const SolverPlanContext &context, int16 snoidIdx) const;
	Common::String debugMazeGenericStepText(const SolverPlanContext &context, const SolverGenericStep &step, bool colorBlindMode) const;

	static const char *debugMazeDirectionName(int16 direction);
	static const char *debugMazeColorName(MazeColorShapeBase shapeBase);
	static const char *debugMazeOutcomeName(LaunchSimulationOutcome outcome);
	static const char *debugMazePlanCornerName(int16 corner);
	static int16 debugMazeNextDirection(int16 direction, const bool *dirFlags);
	static int16 debugMazeInitialDirection(const Common::Array<int16> &regsData, int16 cellIdx, int16 fallback);
	static bool debugMazeContainsCell(const Common::Array<int16> &cells, int16 cellIdx);
	static bool debugMazeIsPreferredKeyOutcome(LaunchSimulationOutcome outcome);
	static bool debugMazeArrayContains(const Common::Array<int16> &values, int16 value);
	static byte debugMazeTraitValue(const ZmbTrait &trait, int16 category);
	static void debugMazeInitializePlanState(const SolverPlanContext &context, SolverPlanState &state);
	static bool debugMazePlanSnoidAccepted(const SolverPlanState &state, int16 snoidIdx);
	static bool debugMazePlanSnoidHeldInGroup(const SolverPlanContext &context, const SolverPlanState &state, int16 snoidIdx, int16 waveGroup);
	static bool debugMazePlanAllAccepted(const SolverPlanContext &context, const SolverPlanState &state);
	static bool debugMazePlanDistinctKeys(int16 stagingKey, int16 primaryKey, int16 rescueKey, int16 pusherKey, int16 finalKey);
	static bool debugMazePlanIsStagingKey(const int16 *stagingKeys, int16 stagingKeyCount, int16 snoidIdx);
	static bool debugMazePlanSameTraits(const SolverPlanContext &context, int16 firstSnoid, int16 secondSnoid);
	static bool debugMazePlanMatchesTrait(const SolverPlanContext &context, int16 snoidIdx, int16 category, int16 value, bool matched);
	static uint64 debugMazePlanStateHash(const SolverPlanContext &context, const SolverPlanState &state);
	static bool debugMazePlanActionPressesOnly(const SolverPlanAction &action, int16 switchGroup);
	static int16 debugMazePlanGroupSwitchCell(const SolverPlanContext &context, int16 waveGroup);
	static Common::String debugMazeLauncherLabel(const Common::Point (&launcherPositions)[14], int16 seatIdx, bool includeRegion);
	static Common::String debugMazePlanRouteFilters(const SolverPlanContext &context, const SolverPlanAction &action);
	static Common::String debugMazePlanTraitCondition(int16 category, int16 value, bool matched);
	static Common::String debugMazePlanStagingLabel(const SolverLevel4Plan &plan, int16 stagingKey);
	static Common::String debugMazePlanStagingLaunchSequence(const SolverLevel4Plan &plan, bool highwayPhase, const Common::Point (&launcherPositions)[14]);
	static const char *debugMazePlanGroupColor(const SolverPlanContext &context, int16 waveGroup, bool colorBlindMode);

	// =================================================================
	// Member variables
	// =================================================================

	// --- Layout and variant ---

	/** Maze-only layout selector; this is not the shared puzzle difficulty. */
	MazeLayoutLevel _layoutLevel = kMazeLayoutLevel1;
	/** REGS resource selected for the active layout. */
	int16 _mazeLayoutRegsId = kResRegs16600_MazeLayoutL1Base;
	/** Selected per-level REGS variant index, or zero for the fixed layout. */
	int16 _levelVariantIdx = 0;
	/** Condition cells substituted to make the restored layout completable. */
	Common::Array<MazeConditionSubstitution> _conditionSubstitutions;

	// --- REGS data ---
	/** Number of validated ten-word cell records in the selected layout REGS. */
	int16 _layoutCellRecordCount = 0;
	/** One-based launcher seat identifiers from layout header words 1 through 9. */
	int16 _launcherSeatIds[10] = {};
	/** Header and ten-word cell records from the selected 16600-series layout resource. */
	Common::Array<int16> _layoutRegsWords;
	/** Values from REGS 16501 used to select collision overlay bases. */
	Common::Array<int16> _collisionOverlayRegs;

	// --- REGS coordinate tables ---
	/** X registration offsets from REGS 18000 for tBMP 5100 cell shapes. */
	Common::Array<int16> _shapeRegsX;
	/** Y registration offsets from REGS 18001 for tBMP 5100 cell shapes. */
	Common::Array<int16> _shapeRegsY;
	/** X registration values from REGS 17000 for bubble shapes. */
	Common::Array<int16> _bubbleShapeRegsX;
	/** Y registration values from REGS 17001 for bubble shapes. */
	Common::Array<int16> _bubbleShapeRegsY;

	// --- Grid state (13x13) ---
	/** Number of rows in the Maze grid. */
	static const int kGridRows = 13;
	/** Number of columns in the Maze grid. */
	static const int kGridCols = 13;

	/** Cell behavior codes for the authored grid. */
	MazeCellType _cellTypes[kGridRows][kGridCols];
	/** Index into @ref _cellStates for each authored cell, or -1. */
	int16 _cellStateIdx[kGridRows][kGridCols];
	/** Per-cell trait category, or zero for no trait constraint. */
	int16 _cellTraitKind[kGridRows][kGridCols];
	/** Per-cell trait value, or zero for no trait constraint. */
	int16 _cellTraitValue[kGridRows][kGridCols];
	/** Per-node direction availability flags. */
	bool _nodeDirFlags[kGridRows][kGridCols][4];
	/** Current direction output of each node. */
	int16 _nodeDirection[kGridRows][kGridCols];
	/** Whether a node cycles its direction when passed. */
	bool _nodeCycleFlag[kGridRows][kGridCols];
	/** Screen anchor of each grid cell, decoded from REGS 16000. */
	Common::Point _gridCellPos[kGridRows][kGridCols];

	/** Maximum number of decoded layout cell states. */
	static const int kMaxCellStates = 64;
	/** Page-owned state decoded from the layout cell records. */
	MazeCellState _cellStates[kMaxCellStates];
	/** Number of valid entries in @ref _cellStates. */
	int16 _cellStateCount = 0;
	/** Base shape selected for each activation wave group. */
	int16 _waveGroupShapeBase[9] = {};

	// Collision tracking grid.
	/** Number of collision occupants per cell. */
	int16 _collisionReservationCount[kGridRows][kGridCols];
	/** Runner index associated with each collision cell. */
	int16 _collisionReservationRunnerIdx[kGridRows][kGridCols];

	// --- Per-runner state ---
	/** Maximum number of active Maze runners. */
	static const int kMaxRunners = 20;
	/** Logical traversal state for each active runner. */
	MazeRunnerState _runnerStates[kMaxRunners];
	/** Snoid ID paired with each runner state. */
	uint16 _runnerSnoidIds[kMaxRunners];
	/** Number of active runner states. */
	int16 _runnerCount = 0;

	// Each authored cell belongs to one of eight activation wave groups.
	/** Number of activation waves in the authored maze. */
	static const int kMaxWaveGroups = 8;
	/** Cell-state indices grouped by activation wave. */
	int16 _waveGroupCellIndices[kMaxWaveGroups][kMaxCellStates];
	/** Number of cell-state indices in each activation wave. */
	int16 _waveGroupCellCounts[kMaxWaveGroups];

	// --- Path selection state ---
	/** Working trait record for each of up to twenty Zoombinis. */
	ZmbTrait _pathCandidateTraits[20];
	/** Match score for each candidate path slot. */
	int16 _pathTraitMatchCounts[21];
	/** Connection table used while selecting path slots. */
	int16 _connectionTable[21];
	/** Sparse free-slot list: entry @c i is @c i when the slot has zero matches, otherwise zero. */
	int16 _freePathSlotList[21];
	/** Slots currently reachable from the generated graph. */
	int16 _reachablePathSlots[21];
	/** Output path slot selected for each Zoombini. */
	int16 _selectedPathSlots[20];
	/** Number of selected path slots and next output write index. */
	int16 _selectedPathSlotCount = 0;
	/** Next selected path slot to assign to a trait-arrow cell. */
	int16 _nextPathSlotAssignmentIdx = 0;
	/** Number of reachable path slots. */
	int16 _reachablePathSlotCount = 0;
	/** Number of nonzero entries in @ref _freePathSlotList. */
	int16 _freePathSlotCount = 0;
	/** Number of trait assignments committed to the path. */
	int16 _committedTraitCount = 0;
	/** Partial trait records used as uniqueness masks; empty fields are ignored. */
	ZmbTrait _uniqueCheckTraits[20];
	/** Complete trait records committed to selected paths. */
	ZmbTrait _committedTraits[20];
	/** Shuffled path candidate pool. */
	int16 _shuffledPathPool[11];
	/** Selected layout variant reused by variant-sensitive path generation. */
	int16 _pathLayoutVariantIdx = 0;

	// --- Grid runner init ---
	/** Read cursor for grid REGS initialization. */
	int16 _layoutCellReadIdx = 0;

	// Queue capacities for grid setup, movement, and crossing work.
	/** Capacity of ordinary Maze work queues. */
	static const int kMaxQueueSize = 21;
	/** Capacity of paired-runner collision queues. */
	static const int kMaxCrossQueueSize = 42;

	/** Launcher seat indices waiting for SCRB setup. */
	int16 _launcherSetupQueue[kMaxQueueSize];
	/** Number of queued launcher seats. */
	int16 _launcherSetupQueueSize = 0;

	// The remaining Maze queues store Maze runner indices.
	/** Runner indices waiting for a movement step. */
	int16 _movementQueue[kMaxQueueSize];
	/** Number of queued movement steps. */
	int16 _movementQueueSize = 0;

	/** Runner indices whose collision companion must be raised before the page overlay. */
	int16 _collisionOverlayRelinkQueue[kMaxQueueSize];
	/** Number of queued collision-companion relinks. */
	int16 _collisionOverlayRelinkQueueSize = 0;

	/** Runner indices queued by launcher events 64, 74, and 84 for launch-column depth relinking. */
	int16 _traversalColumnRelinkQueue[kMaxQueueSize];
	/** Number of queued traversal-column relinks. */
	int16 _traversalColumnRelinkQueueSize = 0;

	/** Accepted runner indices waiting for feet-specific celebration SCRS playback. */
	int16 _acceptedCelebrationQueue[kMaxQueueSize];
	/** Number of queued accepted-runner celebrations. */
	int16 _acceptedCelebrationQueueSize = 0;

	/** Exact flags shared by level-4 upper-left waiting Snoids. */
	static constexpr uint32 kUpperLeftWaitingSnoidFlags = ZmbFeature::FLAG_00000001_TYPE_SNOID | ZmbFeature::FLAG_00008000_LOOP_ANIM;
	/** Exact flags shared by upper-right waiting and traversing Snoids. */
	static constexpr uint32 kUpperRightWaitingSnoidFlags = ZmbFeature::FLAG_00000001_TYPE_SNOID | ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_04000000_OVERLAY;
	/** Whether a completed level-4 upper-left walk requires a depth reorder on the next Maze queue pass. */
	bool _upperLeftArrivalDepthReorderPending = false;
	/** Whether a completed upper-right walk requires a depth reorder on the next Maze queue pass. */
	bool _upperRightArrivalDepthReorderPending = false;

	/** Runner indices waiting for rejection or boundary-exit completion. */
	int16 _exitCompletionQueue[kMaxQueueSize];
	/** Number of queued exit completions. */
	int16 _exitCompletionQueueSize = 0;
	/** Runner indices waiting for next-cell dispatch. */
	int16 _cellArrivalQueue[kMaxQueueSize];
	/** Number of queued cell arrivals. */
	int16 _cellArrivalQueueSize = 0;

	/** Paired runner indices waiting for collision SCRB assignment. */
	int16 _collisionPairQueue[kMaxCrossQueueSize];
	/** Number of queued collision pairs. */
	int16 _collisionPairQueueSize = 0;

	// Arrival position counters (per direction 0-3).
	/**
	 * First lower-left waiting position handed out to an arriving Zoombini.
	 * Slots below this index belong to the initial pack lineup.
	 */
	static const int16 kLowerLeftArrivalPosStart = 16;
	/** Next corner waiting position for each arrival direction. */
	int16 _arrivalPosCounter[4] = {kLowerLeftArrivalPosStart, 0, 0, 0};

	// Placed runner tracking.
	/** Runner indices currently placed on the grid. */
	int16 _placedRunnerIndices[10] = {};
	/** Number of placed runners. */
	int16 _placedRunnerCount = 0;

	// Launcher assignment.
	/** One-based seat marker for each occupied launcher, or zero when empty. */
	int16 _launcherOccupancyMarkers[14] = {};
	/** Maze runner index paired with each launcher seat, or -1. */
	int16 _launcherRunnerIndices[14] = {};

	// --- Feature runners ---
	/** Page overlay anchor before which collision companion overlays are raised. */
	ZmbFeature *_collisionOverlayAnchorFeature = nullptr;
	/** Shared visual companions indexed by authored launcher type. */
	ZmbFeature *_launcherCompanionFeatures[3] = {};
	/** DRAW_ON_REG drop-target helpers indexed by launcher seat. */
	ZmbFeature *_launcherDropTargetFeatures[14] = {};
	/** Primary launcher features indexed by seat. */
	ZmbFeature *_launcherFeatures[14] = {};
	/** Optional launcher shadow features indexed by seat. */
	ZmbFeature *_launcherShadowFeatures[14] = {};
	/** Callback-free overlay anchors retained for authored column depth links. */
	ZmbFeature *_columnDepthAnchors[12] = {};

	// --- Puzzle phase ---
	/** Whether the cell grid has been initialized. */
	bool _gridInitialized = false;
	/** Whether the Maze frame loop may consume its runner and animation queues. */
	bool _queueProcessingEnabled = false;
	/** Whether all runner SCRS tables have been initialized. */
	bool _animTablesInitialized = false;
	/** Whether each launcher seat is present in the selected layout header. */
	bool _launcherSeatEnabled[14] = {};

	// --- Animation event state ---
	/** Alternates the two collision impact sounds. */
	int16 _soundAlternator = 0;

	// Celebration state ---
	/** Whether a celebration batch should be selected. */
	bool _celebrationTrigger = false;
	// Maze owns the eligibility scan and completion target; selection history survives page visits.
	/** Number of celebration animations already played. */
	int16 _celebrationsPlayed = 0;
	/** Number of Snoids targeted by the current celebration batch. */
	int16 _celebrationTarget = 0;

	// --- Zoombini count ---
	/** Number of accepted runners whose exit sequence has completed. */
	int16 _acceptedExitCompletionCount = 0;

	/** Whether Go is enabled after the first accepted upper-right boundary entry. */
	bool _goButtonEnabled = false;

	/** Number of runners that entered accepted boundary cell type 23. */
	int16 _acceptedBoundaryEntryCount = 0;
};

} // End of namespace Mohawk

#endif
