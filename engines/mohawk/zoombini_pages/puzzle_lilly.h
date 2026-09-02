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

#ifndef MOHAWK_ZOOMBINI_PAGES_PUZZLE_LILLY_H
#define MOHAWK_ZOOMBINI_PAGES_PUZZLE_LILLY_H

#include "mohawk/zoombini_pages/puzzle_base.h"

namespace Mohawk {

/**
 * Page-owned state for Lilly grid walkers and their associated child runners.
 *
 * Lilly keeps one entry for each runner slot.
 * Toad and crab slots use the traversal fields and visit grid.
 * Child slots use only their owner indices, callback mode, and animation metadata.
 *
 * Created in response to the @ref ZmbFeature::TYPE_GRIDWALKER flag,
 * but in effect only Lilly uses it.
 * Town and Fleens retain their Snoid or generic feature owners.
 * Lilly's swap wand uses only its @ref ZmbFeature state.
 */
class ZmbLillyGridWalker {
public:
	/** Semantic role of the owning Lilly runner slot. */
	enum RunnerKind : byte {
		kRunnerNone = 0,
		/** Zoombini child runner that rides a Toad across the pond. */
		kRunnerChild = 1,
		kRunnerToad = 2,
		kRunnerCrab = 3
	};

	/** Grid coordinate that represents forward progress. */
	enum ProgressAxis : byte {
		kProgressByColumn = 0,
		kProgressByRow = 1
	};

	/** Lilly pad attribute family used as a traversal constraint. */
	enum PadAttrType : byte {
		kPadAttrNone = 0,
		kPadAttrPattern = 1,
		kPadAttrShape = 2,
		kPadAttrColor = 3
	};

	/** Direction encoding stored by the Lilly pathfinder. */
	enum Direction : byte {
		kDirectionUp = 0,
		kDirectionRight = 1,
		kDirectionDown = 2,
		kDirectionLeft = 3,
		kDirectionCount = 4
	};

	/** Select whether a crab starts a fresh search or follows a claimed trail. */
	enum TraversalMode : byte {
		kTraversalFresh = 0,
		kTraversalReuse = 1
	};

	/**
	 * Callback purpose selected before a Lilly runner starts SCRB playback.
	 * @ref ZoombiniPuzzleLilly::onFeatureAnimEvent() dispatches later animation
	 * events according to this value.
	 */
	enum CallbackMode : byte {
		kCallbackNone = 0,
		kCallbackEnter,     // Event 44 queues the far-side toad rotation.
		kCallbackRotate,    // Event 60 queues the toad's return swim.
		kCallbackExit,      // Event 49 completes the toad's return to the bank.
		kCallbackCross,     // Event 54 retires the toad after its crossing.
		kCallbackSnoid,     // Events 1-3 advance the snoid (aka child)'s walk-in, arrival, and departure.
		kCallbackPathBuild, // Events 20 and 26 hand path ownership from child to toad.
		kCallbackReadyMove, // Toad hop events 10-15.
		kCallbackReadyExit, // Toad exit event 30.
		kCallbackCrabMove   // Crab events 70 and 80.
	};

	/** Sentinel used when a runner has not received an animation phase yet. */
	static const int32 kNoAnimationPhase = -1;

	// --- Runner identity ---
	/** Semantic role of this runner slot. */
	RunnerKind runnerKind = kRunnerNone;
	/** Whether event 54 retired this Toad from further crossings. */
	bool toadRetired = false;
	/** Active-pack Zoombini index represented by this child runner, or -1. */
	int16 packSnoidIdx = -1;

	// --- Grid-walker identity ---
	/** Toad bank slot or crab obstacle entry index. */
	int16 entrySlot = 0;
	/** Axis along which this runner makes forward progress. */
	ProgressAxis progressAxis = kProgressByColumn;

	// --- Tattoo and attribute constraint ---
	/** Pad attribute family required by this runner. */
	PadAttrType attrType = kPadAttrNone;
	/** Required value within @ref attrType. */
	byte attrValue = 0;
	/** Toad pattern or crab shell-shape offset. */
	int16 tattooIdx = 0;
	/** Child runner index riding this toad, or -1. */
	int16 childRunnerIdx = -1;
	/** Shape offset used to render this Toad's rider. */
	int16 riderShapeOffset = 0;

	// --- Child-runner binding ---
	/** Shape offset derived from the child Zoombini's hair trait. */
	int16 childShapeOffset = 0;
	/** Associated toad runner index, or -1 for a free child. */
	int16 toadRunnerIdx = -1;
	/** Entry row selecting the child's row-specific SCRB. */
	int16 entryRow = 0;

	// --- Grid traversal state ---
	/** Whether this runner currently occupies a grid cell. */
	bool occupiesGridCell = false;
	/** Current grid column. */
	int16 gridCol = 0;
	/** Current grid row. */
	int16 gridRow = 0;
	/** Previous grid column used to derive movement direction. */
	int16 prevCol = 0;
	/** Previous grid row used to derive movement direction. */
	int16 prevRow = 0;
	/** Frontier column reached by the current path search. */
	int16 frontierCol = 0;
	/** Frontier row reached by the current path search. */
	int16 frontierRow = 0;
	/** Direction of the current grid step. */
	Direction direction = kDirectionRight;
	/** Maximum reachable progress on @ref progressAxis; 11 reaches the far edge. */
	int16 maxReachableProgress = 11;
	/** Number of path steps already consumed. */
	int16 stepCount = 1;
	/** Last movement SCRB selected by the path stepper. */
	int16 scrbKey = 0;
	/** Number of far-side arrivals; two arrivals retire a toad. */
	int16 completedCrossingCount = 0;

	// --- Movement and interpolation ---
	/** Earliest absolute frame at which the next grid step may begin. */
	uint32 nextMoveReadyFrame = 0;
	/** Whether path search starts fresh or reuses a claimed trail. */
	TraversalMode traversalMode = kTraversalFresh;
	/** Whether BFS tables must be rebuilt before the next step. */
	bool bfsReinitPending = false;
	/** X coordinate of the current grid-step target. */
	int16 moveTargetX = 0;
	/** Y coordinate of the current grid-step target. */
	int16 moveTargetY = 0;
	/** X position at the start of the current movement segment. */
	int16 moveStartX = 0;
	/** Y position at the start of the current movement segment. */
	int16 moveStartY = 0;
	/** X coordinate of the current scripted bank or retirement target. */
	int16 scriptTargetX = 0;
	/** Y coordinate of the current scripted bank or retirement target. */
	int16 scriptTargetY = 0;

	// --- Event ownership ---
	/** Last animation phase delivered to this runner. */
	int32 lastAnimPhase = kNoAnimationPhase;
	/** Callback interpretation used for the next animation event. */
	CallbackMode callbackMode = kCallbackNone;

	// --- Per-runner traversal map ---
	/** Per-runner visited-cell map used by the Lilly pathfinder. */
	int16 visitGrid[12][13];

	/** Reset identity, traversal, interpolation, and callback state. */
	void clear();
};

/**
 * Titanic Tattooed Toads puzzle page (@ref ZoombiniPageType::kLilly).
 * Route 2, Puzzle 2.
 *
 * The player drags one of 12 tattooed toads onto a matching column-0 lily pad.
 * The next Zoombini in line leaps onto the toad,
 * which then hops rightward across tattoo-matching pads.
 * At difficulty 2 or higher, the toad can be dragged onto the pond to swap pad decorations.
 * At difficulty 3 or higher, crabs descend the columns as obstacles.
 */
class ZoombiniPuzzleLilly : public ZoombiniPuzzle {
public:
	/** Create the Titanic Tattooed Toads puzzle page. */
	ZoombiniPuzzleLilly(MohawkEngine_Zoombini *vm);
	/** Release toads, crabs, child runners, and page resources. */
	~ZoombiniPuzzleLilly() override;
	/** Static puzzle name used by debug answer descriptions. */
	static constexpr const char *kPageName = ZmbXferRouteInfo::kXferPageNameTitanicTattooedToads;
	static constexpr int kRouteNumber = 2;
	static constexpr int kRoutePuzzleIdx = 2;
	/** Return the puzzle name used by debug answer descriptions. */
	const char *getPageName() const override { return kPageName; }
	int getRouteNumber() const override { return kRouteNumber; }
	int getRoutePuzzleIdx() const override { return kRoutePuzzleIdx; }

	/** Generate lily-pad traits and initialize the crossing state. */
	void open() override;
	/** Select the Titanic Tattooed Toads background bitmap. */
	void setBackgroundBitmap() override;
	/** Initialize puzzle states and departure transfer source. */
	void initStates() override;
	/** Load lily pads, toads, crabs, and Zoombini features. */
	void loadFeatures() override;
	/** Select the Lilly F1 prompt after all preceding setup RNG draws. */
	void initHelpPrompt() override;
	/** Restart the global Snoid fidget inactivity period after Titanic Tattooed Toads loads. */
	bool requiresFidgetActivityResetOnLoad() const override { return true; }

	/** Return the original Titanic Tattooed Toads script-sound range priority. */
	const ZoombiniPage::ScriptSoundPriorityRanges &getScriptSoundPriorityRanges() const override;

protected:
	/** Begin departure after the required toads cross the pond. */
	void onGoButtonActivated() override;
	/** Restore hidden pack runners before the shared departure writer runs. */
	void executeDeparture() override;
	/** Describe the generated lily-pad constraints for diagnostics. */
	Common::String debugGetAnswer() const override;
	/** Report the finite swap-wand use budget for diagnostics. */
	ZmbChanceInfo debugGetChances() const override;
	/** Lilly supports debugger adjustment of the swap-wand use budget. */
	bool debugCanSetChances() const override;
	/** Set the remaining swap-wand uses for diagnostics. */
	bool debugSetChances(int16 remaining) override;
	/** Return the finite swap-wand budget details for diagnostics. */
	Common::String debugGetChanceDetails() const override;
	/** Advance render-owned swap, hover, and control state before materialization. */
	void onEveryFrame() override;
	/** Advance grid movement, toad hops, crabs, and child runners after rendering. */
	void onPostRenderFrame() override;
	/** Process per-runner toad, crab, and child animation callbacks. */
	void onFeatureAnimEvent(ZmbFeature *feature, int16 eventCode) override;
	/** Use Lilly's sticky toad and swap-wand drag policy. */
	bool hasStickyMouseDrag() const override;
	/** Complete a sticky toad drop or swap-wand selection. */
	void endStickyMouseDrag(const Common::Point &dropPos) override;
	/** Start a toad/Zoombini drag or handle a page control click. */
	ZmbEventHandleResult onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) override;
	/** Resolve a lily-pad or pond drop and release the pointer state. */
	ZmbEventHandleResult onLButtonUp(const Common::Point &absPos, const Common::Point &relPos) override;
	/** Update lily-pad, pond, and obstacle hover feedback. */
	ZmbEventHandleResult onMouseMove(const Common::Point &absPos, const Common::Point &relPos) override;

private:
	/** Decorative pattern code stored for a lily pad. */
	enum LillyPadPattern : byte {
		/** Flower decoration. */
		kLillyPadPattern00_Flower = 0,
		/** Cross decoration. */
		kLillyPadPattern01_Cross = 1,
		/** Diamond decoration. */
		kLillyPadPattern02_Diamond = 2,
	};

	/** Decorative color code stored for a lily pad. */
	enum LillyPadColor : byte {
		/** Magenta pad decoration. */
		kLillyPadColor00_Magenta = 0,
		/** Red pad decoration. */
		kLillyPadColor01_Red = 1,
		/** Orange pad decoration. */
		kLillyPadColor02_Orange = 2,
		/** Cyan pad decoration. */
		kLillyPadColor03_Cyan = 3,
		/** Beige pad decoration. */
		kLillyPadColor04_Beige = 4,
	};

	/** Outline shape code stored for a lily pad. */
	enum LillyPadShape : byte {
		/** One-cut outline. */
		kLillyPadShape00_OneCut = 0,
		/** Two-cut outline. */
		kLillyPadShape01_TwoCut = 1,
		/** Three-pointed outline. */
		kLillyPadShape02_ThreePointed = 2,
		/** Four-pointed outline. */
		kLillyPadShape03_FourPointed = 3,
	};

	/** Page-local lily pad, toad, crab, and pond animation resources. */
	enum PageResourceId : int16 {
		kResBackground5000 = 5000,

		kResBitmapShape7000_LilyPads = 7000,
		kResBitmapShape10000_FeatureOverlay = 10000,
		kResBitmapShape11000_Water = 11000,
		kResBitmapShape13000_Detail = 13000,
		kResBitmapShape14000_FinalOverlay = 14000,

		kResNode1000_WalkNetwork = 1000,

		kResRegs0100_Main = 100,
		kResRegs0200_LilyPads = 200,
		kResRegs10000_FeatureOverlay = 10000,

		kResScrb10000_FeatureBase = 10000,
		kResScrb10001_StepDirection0 = 10001,
		kResScrb10002_StepDirection1 = 10002,
		kResScrb10003_StepDirection2 = 10003,
		kResScrb10004_StepDirection3 = 10004,
		kResScrb10004_StepLast = 10004,
		kResScrb10019_RowBase = 10019,
		kResScrb10031_Exit = 10031,
		kResScrb10043_EntryBase = 10043,
		kResScrb10057_Move = 10057,
		kResScrb10058_MoveAlternate = 10058,
		kResScrb10059_MoveFinal = 10059,
		kResScrb10060_SwapBase = 10060,
		kResScrb10061_SwapAlternateBase = 10061,
		kResScrb10067_Crab = 10067,
		kResScrb10069_Departure = 10069,
		kResScrb10071_CrabDirection0 = 10071,
		kResScrb10073_CrabDirection2 = 10073,
		kResScrb10075_CrabDirection3 = 10075,
		kResScrb10077_CrabDirection1 = 10077,
		kResScrb10078_ToadBase = 10078,
		kResScrb10089_ChildBase = 10089,
		kResScrb10109_ChildEntryBase = 10109,
		kResScrb10129_GridRowBase = 10129,
		kResScrb10141_ExitBase = 10141,
		/** v2.0US only */
		kResScrb10170_ToadHoverBase = 10170,
		kResScrb11000_ToadIntro = 11000,
		kResScrb13000_DetailBase = 13000,
		kResScrb14000_FinalBase = 14000,

		// Sound resources.
		kResSound12000_SwapBase = 12000,

		// Sound ranges used by getScriptSoundPriority().
		kResSoundRange10000_SwapBase = 10000,
		kResSoundRange10002_Priority = 10002,
		kResSoundRange10003_Priority = 10003,
		kResSoundRange10004_Priority = 10004,
		kResSoundRange10005_PriorityBase = 10005,
		kResSoundRange10008_PriorityLast = 10008,
		kResSoundRange10009_Priority = 10009,
		kResSoundRange10010_Priority = 10010,
		kResSoundRange10011_Priority = 10011,
		kResSoundRange11000_SystemRangeBase = 11000,
		kResSoundRange11001_SystemRangeLast = 11001,
		kResSoundRange12000_SwapBase = 12000,
		kResSoundRange12004_SwapRangeLast = 12004,
	};

	/** Common base event codes shared by the page's Snoid, Toad, and Crab animation callbacks. */
	enum EntityBaseEventCode : int16 {
		kEntityBaseEventCode000_ToggleFacing = 0,
		kEntityBaseEventCode020_StartPathBuild = 20,
		kEntityBaseEventCode026_FinishChildLeap = 26,
		kEntityBaseEventCode030_ArriveAtFarNode = 30,
		kEntityBaseEventCode044_FinishEntryInterpolation = 44,
		kEntityBaseEventCode049_RestoreExitedToad = 49,
		kEntityBaseEventCode054_FinishCrossing = 54,
		kEntityBaseEventCode060_FinishRotation = 60,
		kEntityBaseEventCode070_FinishCrabCellMove = 70,
		kEntityBaseEventCode080_FinishCrabExit = 80
	};

	/** Snoid movement callbacks for the Zoombini runners. */
	enum SnoidMoveEventCode : int16 {
		kSnoidMoveEventCode001_FinishWalkIn = 1,
		kSnoidMoveEventCode002_QueueArrival = 2,
		kSnoidMoveEventCode003_QueueDeparture = 3
	};

	/** Toad return and swap-script callbacks. */
	enum ToadReturnEventCode : int16 {
		kToadReturnEventCode003_StartFinalChildWalkIn = 3,
		kToadReturnEventCode004_FinishToadIntro = 4,
		kToadReturnEventCode005_SelectNextSwapPair = 5
	};

	/** Toad hop callbacks. */
	enum ToadHopEventCode : int16 {
		kToadHopEventCode010_SnapToDestination = 10,
		kToadHopEventCode011_AdvanceCell = 11,
		kToadHopEventCode012_ClearPreviousCell = 12,
		kToadHopEventCode013_MidpointFrame = 13,
		kToadHopEventCode014_MidpointFrameAlternate = 14,
		kToadHopEventCode015_QueueNextStep = 15
	};

	// --- Runner index space ---
	/** First runner slot reserved for toads. */
	static const int16 kToadBase = 20;
	/** Number of toad runner slots. */
	static const int16 kToadCount = 12;
	/** First runner slot reserved for crabs. */
	static const int16 kCrabBase = 32;
	/** Maximum number of crab runner slots. */
	static const int16 kMaxCrabs = 20;
	/** Total runner slots including Zoombini children, toads, and crabs. */
	static const int kMaxRunners = 52;

	// --- Initialization ---
	/** Load active-pack Zoombinis and bind their child slots. */
	void loadZoombinisFromPack();
	/** Select difficulty-dependent obstacle and traversal parameters. */
	void setDifficultyParams();
	/** Build pad attributes and the initial traversal grid. */
	void initGridWithAttributes();
	/** Create child visual runners for the active-pack Zoombinis. */
	void createZoombiniChildRunners();
	/** Create and initialize the toad runner slots. */
	void createToadRunners();
	/** Load registration-point tables used by grid and runner rendering. */
	void loadRegsCoordinateTables();
	/** Generate the difficulty-specific pad and obstacle patterns. */
	void generateChallengePatterns();
	/** Record generation-owned routes and their post-challenge direct state. */
	void recordGeneratedAnswerEntryRows();

	// --- Grid transform helpers ---
	/** Load one authored grid registration table. */
	void loadGridPatternRegs(int gridIdx, int16 resId);
	/** Rotate an authored grid pattern in place. */
	void rotateGrid(int rotType, int16 grid[12][12]);
	/** Flip an authored grid pattern in place. */
	void flipGrid(int flipType, int16 grid[12][12]);

	// Process the queues from @ref ZoombiniPuzzleLilly::onPostRenderFrame().
	/** Start queued toad-entry animations. */
	void processEnterQueue();
	/** Start queued toad-exit animations. */
	void processExitQueue();
	/** Retire a toad whose exit animation completed. */
	void processCompletedExitRunner();
	/** Start queued far-side toad rotations. */
	void processRotateQueue();
	/** Start queued crossing animations. */
	void processCrossQueue();
	/** Retire a toad whose crossing animation completed. */
	void processCompletedCrossRunner();
	/** Start queued Zoombini departure animations. */
	void processDepartQueue();
	/** Start queued Zoombini arrival animations. */
	void processArriveQueue();
	/** Advance the active grid movement phase. */
	void processMovePhase();
	/** Return whether a runner has satisfied the prerequisites for movement. */
	bool isMoveRunnerReady(int16 runnerIdx) const;
	/** Release runners whose animation and ownership work has completed. */
	void processFreedRunners();
	/** Advance the pond swap-cell blink and swap animation. */
	void processCellSwapAnimation();

	// --- Toad drag & drop ---
	/** Begin dragging a toad from the bank. */
	void beginToadDrag(int16 toadRunnerIdx, const Common::Point &absPos);
	/** Update the dragged toad and entry-row highlight. */
	void updateToadDrag(const Common::Point &absPos);
	/** Resolve a toad drop on the bank or pond. */
	void endToadDrag(const Common::Point &absPos);
	/** Place a toad at an entry row and queue its entry animation. */
	void placeToad(int16 toadRunnerIdx, int16 entryRow);
	/** Cancel a toad drag and restore its bank position. */
	void cancelToadDrag(int16 toadRunnerIdx);

	// --- Toad drag & swap selection ---
	/** Begin dragging the difficulty 2+ swap wand. */
	void beginToadSwapDrag(const Common::Point &absPos);
	/** Apply a swap-wand click to the pond or selected cell. */
	void handleToadPondClick(const Common::Point &absPos);
	/** Cancel swap-wand dragging and restore its visual state. */
	void cancelToadSwapDrag();
	/** Update TLC v2 swap-wand hover feedback. */
	void updateV2SwapWandHover();
	/** Return whether a point hits the idle swap wand. */
	bool isSwapWandAtPoint(const Common::Point &pos) const;
	/** Return whether a point lies inside the interactive pond area. */
	bool isInsidePondArea(const Common::Point &pos) const;

	/** Update the entry-cell highlight while dragging a toad. */
	void updateEntryHighlight(const Common::Point &absPos, int16 toadRunnerIdx);
	/** Clear the currently highlighted entry row. */
	void clearEntryHighlight();

	// --- Pathfinding ---

	/**
	 * Advance a toad one step on the grid.
	 * @return SCRB 10001-10004 for the hop, 10031 for exit, 0 for no move.
	 */
	int16 advancePathOnGrid(int16 runnerIdx);

	/** Dijkstra-style shortest path fill on the 12x12 grid. */
	void computeShortestPath(int16 targetProgress, int16 runnerIdx);

	/** Greedy BFS traversal of the cost matrix. */
	void traversePathBFS(int16 targetProgress, int16 runnerIdx);

	/** Initialize full BFS path state for a runner. */
	void initRunnerBFSPath(int16 runnerIdx);

	// --- Callbacks (event handlers) ---

	/** Child runner events. */
	void handleChildEvent(int16 eventCode, int16 runnerIdx);

	/** Crab events 70/80. */
	void handleCrabArriveOrDepart(int16 eventCode, int16 runnerIdx);

	/** Toad hop events 10-15. */
	void handleMoveFinalizeStep(int16 stepIdx, int16 runnerIdx);

	/** Toad arrival at the far side (event 30). */
	void handleArriveAtNode(int16 runnerIdx);

	/** Toad return and swap-script events 3/4/5. */
	void handleScriptEvent(int16 eventId, ZmbFeature *eventFeature);

	// --- Custom render callbacks ---

	/** Render the 144 lily pads (two shape layers per cell). */
	ZmbRenderResult renderGridSprites(ZmbFeature *feature);

	/** No-op row anchor runners used only. */
	bool padAnchorPreRender(ZmbFeature *feature);
	/** Render the row-anchor runner's visible pad geometry. */
	ZmbRenderResult renderPadAnchor(ZmbFeature *feature);

	/** Render the blinking highlight on the hovered entry cell. */
	ZmbRenderResult renderCursorIndicator(ZmbFeature *feature);

	/** Render the blinking swap-cell overlays. */
	ZmbRenderResult renderCellAnimA(ZmbFeature *feature);
	/** Render the second blinking swap-cell animation layer. */
	ZmbRenderResult renderCellAnimB(ZmbFeature *feature);

	/** Draw one cell's two layers with a blink phase. */
	void renderCellBlink(int16 blinkPhase, int16 col, int16 row);

	/** Applies the toad pattern and rider overlays, movement interpolation, and rotation offsets before rendering. */
	void toadPreRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);

	/** Child pre-render shape hook: per-zoombini sprite variant offset. */
	void childPreRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);

	/** Crab pre-render shape hook: shell pattern overlay + 8-phase movement. */
	void crabPreRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);

	// --- Helpers ---

	/** Insert a grid entity immediately before its row anchor. */
	void linkRunnerBeforePad(int16 runnerIdx, int16 row);

	/**
	 * Rebase a feature's position onto its current hotspot-0 position.
	 * The next SCRB then continues from the animation's visible end,
	 * including the REGS correction.
	 */
	void snapPosLocToAnim(ZmbFeature *feature);

	/** Load a swap-wand SCRB while preserving its raw hotspot cursor anchor. */
	void loadSwapWandScrb(int16 scrbId);

	/** Place the swap wand's raw hotspot at its home position. */
	void setToadHomePosition();

	/** Count toads with a complete BFS path and play the match/mismatch sound. */
	void countMatchesAndPlaySound();

	/** Activate a cell anim runner on a grid cell. */
	void setRunnerClickRect(int16 col, int16 row, ZmbFeature *feature);

	/**
	 * Keep the swap wand's hit area on the shapes it actually draws.
	 * The original recomputes clickRect from the positioned shapes each pre-render; this follows
	 * the positioned bounds once the wand is engaged.
	 */
	void syncSwapWandClickRect();

	/** Deactivate a cell anim runner. */
	void initCellRunnerPosition(int16 col, int16 row, ZmbFeature *feature);

	/** Swap two cells' attribute bytes and update affected runner paths. */
	void swapCellsAndUpdateRunners(int16 colA, int16 rowA, int16 colB, int16 rowB);

	/** Handle a swap-cell selection click (states 4/5). */
	bool trySelectSwapCell(const Common::Point &absPos);

	/** Spawn a crab obstacle (difficulty >= 3). */
	void spawnCrab();

	/** Crab forward advance (reuse mode). */
	uint16 advanceCrabForwardStep(int16 runnerIdx);

	/** Crab BFS advance (fresh mode). */
	uint16 advanceCrabPathStepAlt(int16 runnerIdx);

	// --- BFS layer arrays (crab pathfinding) ---

	/** Initialize the BFS arrays for one attribute value layer. */
	void initBFSGrid(int16 attrValue, ZmbLillyGridWalker::PadAttrType attributeType);

	/** Expand one BFS cell in 4 directions. */
	void bfsExpandCell(int16 col, int16 row1, int16 attrValue, ZmbLillyGridWalker::PadAttrType attributeType);

	/** Return one Lilly pad attribute from the requested family and zero-based cell. */
	byte getGridAttrByType(ZmbLillyGridWalker::PadAttrType attributeType, int16 row0, int16 col) const;
	/** Return whether one tattoo can reach the far side from an entry row. */
	bool isAnswerEntryRowCrossable(ZmbLillyGridWalker::PadAttrType attributeType, byte attrValue, int16 entryRow) const;

	/** Highlight rect for the cursor indicator */
	Common::Rect cursorHighlightRect(int16 row, int16 col) const;

	// =================================================================
	// Static data tables
	// =================================================================

	/** Y offset per column for cell positions. */
	static constexpr int16 kColYOffset[13]{
		2,
		2,
		4,
		4,
		6,
		6,
		8,
		8,
		10,
		10,
		12,
		12,
		0,
	};

	/** Preset swap pair column coordinates (word-indexed). */
	static constexpr int16 kSwapPairCol[20]{
		4,
		0,
		3,
		0,
		8,
		0,
		10,
		0,
		0,
		0,
		4,
		0,
		6,
		0,
		3,
		0,
		5,
		0,
		0,
		0,
	};
	/** Preset swap pair row coordinates. */
	static constexpr int16 kSwapPairRow[20]{
		4,
		0,
		6,
		0,
		3,
		0,
		5,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		2,
		2,
		4,
		4,
		6,
	};

	/** Zoombini count -> required grid row count. */
	static constexpr int16 kZmbToRowCount[21]{
		1,
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
		9,
		9,
		10,
		10,
	};

	/** Base used to combine the first and third attributes into a decoration shape. */
	static constexpr byte kCombinedAttrBase[5]{
		5,
		8,
		11,
		14,
		17,
	};

	/** Row/column validity for pattern placement (0=invalid). */
	static constexpr int16 kRowColValidity[13]{
		0,
		0,
		2,
		3,
		4,
		5,
		6,
		7,
		8,
		9,
		10,
		11,
		0,
	};

	/** Attribute family for each tattoo pattern entry. */
	static constexpr ZmbLillyGridWalker::PadAttrType kPatternAttrType[13]{
		ZmbLillyGridWalker::kPadAttrPattern,
		ZmbLillyGridWalker::kPadAttrPattern,
		ZmbLillyGridWalker::kPadAttrPattern,
		ZmbLillyGridWalker::kPadAttrShape,
		ZmbLillyGridWalker::kPadAttrShape,
		ZmbLillyGridWalker::kPadAttrShape,
		ZmbLillyGridWalker::kPadAttrShape,
		ZmbLillyGridWalker::kPadAttrColor,
		ZmbLillyGridWalker::kPadAttrColor,
		ZmbLillyGridWalker::kPadAttrColor,
		ZmbLillyGridWalker::kPadAttrColor,
		ZmbLillyGridWalker::kPadAttrColor,
		ZmbLillyGridWalker::kPadAttrNone,
	};
	/** Pattern attr value pool. */
	static constexpr int16 kPatternAttrValue[13]{
		0,
		1,
		2,
		0,
		1,
		2,
		3,
		0,
		1,
		2,
		3,
		4,
		0,
	};
	/** Pattern extra index pool. */
	static constexpr int16 kPatternAttrExtra[13]{
		0,
		1,
		2,
		3,
		4,
		5,
		6,
		7,
		8,
		9,
		10,
		11,
		0,
	};

	/**
	 * BFS layer offset by attribute family.
	 * The obstacle-type offset selects an extra pattern index for each attribute value.
	 */
	static constexpr int16 kObstacleBFSOffset[5]{
		0,
		0,
		3,
		7,
		0,
	};

	/** Toad bank positions (12 slots). */
	static constexpr Common::Point kEntryPositions[12]{
		{66, 118},
		{60, 147},
		{46, 177},
		{58, 205},
		{44, 232},
		{52, 262},
		{39, 289},
		{18, 313},
		{47, 327},
		{17, 345},
		{43, 363},
		{53, 393},
	};

	/** Initial Zoombini staging positions (20 slots). */
	static constexpr Common::Point kInitialPositions[20]{
		{101, 27},
		{100, 42},
		{95, 55},
		{88, 69},
		{78, 80},
		{88, 24},
		{85, 39},
		{80, 54},
		{72, 67},
		{62, 81},
		{74, 25},
		{70, 40},
		{64, 53},
		{56, 67},
		{46, 79},
		{59, 25},
		{55, 39},
		{49, 53},
		{41, 65},
		{44, 31},
	};

	/**
	 * Immutable drop rectangles are instance members because Common::Rect requires
	 * runtime construction and ScummVM prohibits global C++ constructors.
	 */
	/** Drop rectangles for the twelve toad-entry cells. */
	const Common::Rect _toadEntryCellRects[12] = {
		Common::Rect(134, 65, 168, 93),
		Common::Rect(131, 94, 165, 122),
		Common::Rect(129, 122, 163, 150),
		Common::Rect(127, 152, 161, 180),
		Common::Rect(124, 181, 158, 209),
		Common::Rect(117, 210, 151, 238),
		Common::Rect(113, 239, 147, 267),
		Common::Rect(109, 268, 143, 296),
		Common::Rect(100, 297, 134, 331),
		Common::Rect(93, 326, 127, 354),
		Common::Rect(91, 355, 125, 383),
		Common::Rect(88, 384, 120, 532),
	};

	// Pond area rect; bank triangle boxes
	/** Interactive pond rectangle used by the swap wand. */
	const Common::Rect _swapPondRect = Common::Rect(83, 62, 556, 430);
	/** Left bank exclusion box for toad dragging. */
	const Common::Rect _leftBankExclusionRect = Common::Rect(83, 62, 130, 337);
	/** Right bank exclusion box for toad dragging. */
	const Common::Rect _rightBankExclusionRect = Common::Rect(509, 155, 556, 430);
	/** Home position of the swap wand. */
	static constexpr Common::Point kToadHomePosition = Common::Point(38, 415);
	/** Offset from a pad cell to the swap-wand pick point. */
	static constexpr Common::Point kToadCellPickOffset = Common::Point(27, 22);
	/** Stable home hit area shared by the normal and TLC v2 hover wand shapes. */
	const Common::Rect _normalSwapWandHomeRect = Common::Rect(38, 415, 69, 447);

	/** Cursor highlight base shape per attr2 (pad shape). */
	static constexpr int16 kCursorShapeBase[4]{
		20,
		22,
		24,
		26,
	};
	/** Cursor blink cycle. */
	static constexpr int16 kCursorBlinkFrames[4]{
		0,
		0,
		1,
		1,
	};
	/** Swap-cell blink base shape per attr2. */
	static constexpr int16 kCellBlinkShapeBase[4]{
		28,
		30,
		32,
		34,
	};

	// =================================================================
	// Member variables
	// =================================================================

	// --- Core puzzle state ---
	/** Whether Go is enabled after the first successful Toad arrival. */
	bool _goButtonEnabled = false;

	// --- Difficulty parameters ---
	/** Number of generated crab-entry slots cycled by the spawner. */
	int16 _crabEntryCycleLength = 0;

	// --- Zoombini counts ---
	// Counts Lilly crossings; the same field name on other pages has a different
	// progression contract and must remain page-owned.
	/** Number of pack Zoombinis assigned a child runner and Toad crossing. */
	int16 _assignedSnoidCount = 0;
	/** Number of walk-in animations that must finish before toad clicks resume. */
	int16 _pendingChildWalkInCount = 0;

	// --- Grid state ---
	/** Authored grid pattern family, or zero for the no-obstacle layout. */
	int16 _obstacleGridFamily = 0;

	/** Authored grid pattern tables selected during setup. */
	int16 _authoredPathGrids[3][12][12];
	/** Occupancy marker for each pond cell. */
	byte _cellOccupancy[12][13];
	/** Reservation marker used while a runner claims an exit cell. */
	byte _exitCellReservations[12][13];
	/** Pattern attribute for each pad cell. */
	byte _padPattern[12][13];
	/** Pad-shape attribute for each pad cell. */
	byte _padShape[12][13];
	/** Color attribute for each pad cell. */
	byte _padColor[12][13];
	/** Decoration shape derived from each pad's pattern and color. */
	byte _padDecorationShape[12][13];
	/** Hit rectangles for pond cells. */
	Common::Rect _padCellRects[12][13];
	/** Screen positions of pond cells. */
	Common::Point _padCellPositions[12][13];

	// --- Challenge pattern state ---
	/** Attribute family required by each generated challenge slot. */
	ZmbLillyGridWalker::PadAttrType _challengeAttrTypes[13] = {};
	/** Required value for each generated challenge slot. */
	int16 _challengeAttrValues[13] = {};
	/** Tattoo index associated with each generated challenge slot. */
	int16 _challengeTattooIndices[13] = {};
	/** Number of times each generated challenge slot has been placed. */
	int16 _challengePlacementCounts[13] = {};
	/** Challenge markers; zero marks a slot selected for this puzzle. */
	int16 _challengeEnabledMarkers[13] = {};
	/** Randomized slot order used while selecting the active challenge set. */
	int16 _challengeSlotShuffle[13] = {};
	/** Generation-owned crossing-entry rows indexed by tattoo deck index. */
	uint16 _generatedAnswerEntryRowMasks[kToadCount] = {};
	/** Generated routes that remain directly crossable after challenge placement. */
	uint16 _generatedDirectEntryRowMasks[kToadCount] = {};

	// --- Crab entry point table (difficulty 3/4) ---
	/** Entry columns available to newly spawned crabs. */
	int16 _crabEntryColumns[16] = {};
	/** Attribute families assigned to crab entry points. */
	ZmbLillyGridWalker::PadAttrType _crabEntryAttrTypes[16] = {};
	/** Attribute values assigned to crab entry points. */
	int16 _crabEntryAttrValues[16] = {};
	/** Number of valid crab entry points. */
	int16 _crabEntryCount = 0;
	/** Pad attribute family followed by this puzzle's crabs. */
	ZmbLillyGridWalker::PadAttrType _crabPathAttrType = ZmbLillyGridWalker::kPadAttrNone;

	/** Claimed crab-trail state used when later crabs reuse an existing path. */
	int8 _crabTrailState[12][13] = {};

	// --- BFS layer arrays ---
	/** Maximum number of attribute-specific BFS layers. */
	static const int kMaxBFSLayers = 5;
	/** Number of cells and directions represented by one BFS layer. */
	static const int kBFSEntriesPerLayer = 507;
	/** Total entries reserved for all BFS layers. */
	static const int kMaxBFSEntries = kBFSEntriesPerLayer * kMaxBFSLayers;

	/** Row/layer visit value propagated through each BFS entry. */
	int16 _bfsVisitState[kMaxBFSEntries] = {};
	/** Direction used to reach each BFS layer entry. */
	int16 _bfsDirection[kMaxBFSEntries] = {};
	/** Distance from the layer's BFS origin. */
	int16 _bfsDistance[kMaxBFSEntries] = {};

	/** Maximum number of cells in the active BFS queue. */
	static const int kBFSQueueMax = 144;
	/** Queued BFS columns. */
	int16 _bfsQueueCol[kBFSQueueMax] = {};
	/** Queued BFS rows. */
	int16 _bfsQueueRow[kBFSQueueMax] = {};
	/** Head index of the active BFS queue. */
	int16 _bfsQueueHead = 0;
	/** Tail index of the active BFS queue. */
	int16 _bfsQueueTail = 0;

	// --- REGS coordinate tables ---
	/** Row X coordinates loaded from the main REGS table. */
	Common::Array<int16> _gridRowBaseX;
	/** Row Y coordinates loaded from the main REGS table. */
	Common::Array<int16> _gridRowBaseY;
	/** Pad-shape X offsets loaded from the pad REGS table. */
	Common::Array<int16> _padShapeOffsetX;
	/** Pad-shape Y offsets loaded from the pad REGS table. */
	Common::Array<int16> _padShapeOffsetY;

	// --- Runners ---
	/**
	 * Grid-walker semantic state indexed by Lilly runner slot.
	 * Child visual slots use only the ScummVM binding fields documented by
	 * @ref ZmbLillyGridWalker; Toad and crab slots also consume the traversal state.
	 * Maze has a separate grid and queue state machine; matching field names do
	 * not make these arrays interchangeable.
	 */
	ZmbLillyGridWalker _runnerStates[kMaxRunners];
	/** Visual feature runner for each Lilly semantic runner slot. */
	ZmbFeature *_runnerFeatures[kMaxRunners] = {};
	/** Active-pack Snoids in their original pack order. */
	Common::Array<ZmbSnoid *> _packSnoids;

	/** Runner slots currently occupied by crabs. */
	int16 _activeCrabRunnerIndices[kMaxCrabs] = {};
	/** Number of active crab slots. */
	int16 _activeCrabCount = 0;
	/** Next crab entry table index to test. */
	int16 _nextCrabEntryIdx = 0;
	/** Frame at which the next crab may spawn. */
	uint32 _nextCrabSpawnFrame = 0;

	// --- Freed runners pending cleanup ---
	/** Runner slots awaiting feature unlink and cleanup. */
	int16 _runnerCleanupQueue[kMaxRunners];
	/** Number of entries in @ref _runnerCleanupQueue. */
	int16 _runnerCleanupQueueSize = 0;

	// --- Animation queues ---
	/** Capacity of the serialized runner-event queues. */
	static const int kMaxQueueSize = 21;
	/** Capacity of movement-phase runner queues. */
	static const int kMaxMoveQueueSize = 100;

	/** Toad runners waiting to turn in on the far bank. */
	int16 _farBankTurnInQueue[kMaxQueueSize];
	/** Number of queued far-bank turn-in runners. */
	int16 _farBankTurnInQueueSize = 0;
	/** Toad runners waiting to return to the near bank. */
	int16 _bankReturnQueue[kMaxQueueSize];
	/** Number of queued bank-return runners. */
	int16 _bankReturnQueueSize = 0;
	/** Toad runners waiting to play their retirement crossing. */
	int16 _retirementQueue[kMaxQueueSize];
	/** Number of queued retirement runners. */
	int16 _retirementQueueSize = 0;
	/** Toad runners waiting to rotate on the far bank. */
	int16 _farBankRotationQueue[kMaxQueueSize];
	/** Number of queued rotation runners. */
	int16 _farBankRotationQueueSize = 0;
	/** Toad runners waiting for their child rider to mount. */
	int16 _childMountQueue[kMaxQueueSize];
	/** Number of queued child-mount runners. */
	int16 _childMountQueueSize = 0;
	/** Child runners waiting to depart from the far bank. */
	int16 _childDepartureQueue[kMaxQueueSize];
	/** Number of queued child departures. */
	int16 _childDepartureQueueSize = 0;
	/** Toad movement-ready queue. */
	int16 _toadStepReadyQueue[kMaxMoveQueueSize];
	/** Number of entries in the ready queue. */
	int16 _toadStepReadyQueueSize = 0;
	// Lilly queue entries use Lilly runner slots and Lilly traversal phases.
	// Do not centralize this queue with Maze movement scheduling.
	/** Lilly path-step queue. */
	int16 _toadPathStepQueue[kMaxMoveQueueSize];
	/** Number of entries in the movement queue. */
	int16 _toadPathStepQueueSize = 0;
	/** Deferred ready events waiting for the current frame phase. */
	int16 _pendingToadStepReadyQueue[kMaxMoveQueueSize];
	/** Number of deferred ready events. */
	int16 _pendingToadStepReadyQueueSize = 0;
	/** Deferred movement events waiting for the current frame phase. */
	int16 _pendingToadPathStepQueue[kMaxMoveQueueSize];
	/** Number of deferred movement events. */
	int16 _pendingToadPathStepQueueSize = 0;
	/** Toad runners waiting for path initialization. */
	int16 _toadPathInitQueue[kMaxQueueSize];
	/** Number of queued path initializations. */
	int16 _toadPathInitQueueSize = 0;

	// --- Serialized runner tracking ---
	/** Toad currently turning in on the far bank. */
	int16 _activeFarBankTurnInRunner = -1;
	/** Toad currently returning to the near bank. */
	int16 _activeBankReturnRunner = -1;
	/** Toad currently playing its retirement crossing. */
	int16 _activeRetirementRunner = -1;
	/** Runner whose exit animation completed most recently. */
	int16 _completedBankReturnRunner = -1;
	/** Runner whose crossing animation completed most recently. */
	int16 _completedRetirementRunner = -1;
	/** Number of successful Toad arrivals used to unlock Go. */
	int16 _successfulToadArrivalCount = 0;
	/** Number of child Zoombinis that completed their far-bank departure. */
	int16 _completedSnoidCount = 0;

	// --- Move phase state ---
	/** Whether the next movement phase processes Toad paths before crabs. */
	bool _toadPathPhasePending = false;

	// --- Toad drag state ---
	/** Toad runner currently being dragged, or -1. */
	int16 _draggedToadRunnerIdx = -1;
	/** Hovered entry row; the entry column is always zero. */
	int16 _highlightedEntryRow = -1;
	/** Current frame in the entry-cell cursor blink cycle. */
	int16 _cursorBlinkFrame = 0;
	/** Next frame at which the entry-cell cursor blink advances. */
	uint32 _cursorBlinkTimer = 0;

	// --- Toad / swap state ---
	/** Whether the swap wand is currently dragged. */
	bool _swapWandDragging = false;
	/** Whether a Toad or swap-wand drag currently owns the hidden cursor. */
	bool _pageDragCursorHidden = false;
	/** Pointer position used for swap-wand hover feedback. */
	Common::Point _swapWandHoverPos = Common::Point(-1, -1);
	/** Home hit rectangle used by the swap wand. */
	Common::Rect _swapWandHomeClickRect;
	/**
	 * Whether the player may pick up a bank Toad or the swap wand.
	 * The same gate pauses Toad and Crab movement while a pad selection is pending.
	 */
	bool _toadPickupInputEnabled = true;
	/** Ordered states for selecting and swapping two Lilly-pad cells. */
	enum class CellSelectionState : int16 {
		/** The swap wand is inactive. */
		kInactive00 = 0,
		/** Await the first cell. */
		kSelectFirst04 = 4,
		/** Await the second cell. */
		kSelectSecond05 = 5,
		/** The selected pair is running its swap animation. */
		kAnimating06 = 6
	};
	/** Swap selection state: off, pick A, pick B, or animate. */
	CellSelectionState _swapSelectionState = CellSelectionState::kInactive00;
	/** First selected swap cell column. */
	int16 _firstSwapCellCol = 0;
	/** First selected swap cell row. */
	int16 _firstSwapCellRow = 0;
	/** Second selected swap cell column. */
	int16 _secondSwapCellCol = 0;
	/** Second selected swap cell row. */
	int16 _secondSwapCellRow = 0;
	/** Swap-wand visual and use stage, from 0 through 6. */
	int16 _swapWandStage = 0;
	/** Successful swaps required to advance one wand stage. */
	int16 _swapsPerWandStage = 0;
	/** Successful swaps accumulated toward the next wand stage. */
	int16 _swapsTowardNextStage = 0;
	/** Coordinate cursor into the authored intro swap sequence. */
	int16 _introPresetSwapCellIdx = 0;
	/** Sound index selected for the current swap result. */
	int16 _nextSwapSoundIdx = 0;
	/** Number of blink intervals required before swap selection resets. */
	int16 _swapBlinkIntervalLimit = 3;
	/** Number of completed blink intervals for the selected pair. */
	int16 _swapBlinkIntervalCount = 0;
	/** Blink phase of the first selected cell. */
	byte _firstSwapBlinkFrame = 0;
	/** Blink phase of the second selected cell. */
	byte _secondSwapBlinkFrame = 0;
	/** Next update frame for the first cell blink. */
	uint32 _nextFirstSwapBlinkFrame = 0;
	/** Next update frame for the second cell blink. */
	uint32 _nextSecondSwapBlinkFrame = 0;
	/** Whether the current cell swap has been applied. */
	bool _selectedCellsSwapped = false;

	// --- Features ---
	/** Feature that renders the entry-cell cursor. */
	ZmbFeature *_entryHighlightFeature = nullptr;
	/** First swap-cell animation feature. */
	ZmbFeature *_firstSwapCellFeature = nullptr;
	/** Second swap-cell animation feature. */
	ZmbFeature *_secondSwapCellFeature = nullptr;
	/** Row-anchor features used by lily-pad rendering. */
	ZmbFeature *_padRowAnchorFeatures[12] = {};
	/** One-shot Toad intro feature used at levels 2 through 4. */
	ZmbFeature *_toadIntroFeature = nullptr;
	/** Interactive swap-wand feature. */
	ZmbFeature *_swapWandFeature = nullptr;
};

} // End of namespace Mohawk

#endif
