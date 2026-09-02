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

#include "common/hashmap.h"

#include "gui/debugger.h"
#include "mohawk/zoombini.h"
#include "mohawk/zoombini_graphics.h"
#include "mohawk/zoombini_metaengine.h"
#include "mohawk/zoombini_pages/grid_traversal.h"
#include "mohawk/zoombini_pages/puzzle_maze.h"
#include "mohawk/zoombini_random.h"
#include "mohawk/zoombini_resource.h"
#include "mohawk/zoombini_sound.h"
#include "mohawk/zoombini_state.h"

namespace Mohawk {

// =================================================================
// Static data tables
// =================================================================
constexpr Common::Point ZoombiniPuzzleMaze::kSnoidPositions[20];
constexpr int16 ZoombiniPuzzleMaze::kCreatureHasShadow[14];
constexpr int16 ZoombiniPuzzleMaze::kCreatureTypeId[14];
constexpr int16 ZoombiniPuzzleMaze::kCreatureScrbTable[14];
constexpr Common::Point ZoombiniPuzzleMaze::kSeatPositions[14];
constexpr Common::Point ZoombiniPuzzleMaze::kLauncherPositions[14];
constexpr Common::Point ZoombiniPuzzleMaze::kSeatGridCoords[14];
constexpr bool ZoombiniPuzzleMaze::kSeatFacingLeft[14];
constexpr ZoombiniPuzzleMaze::MazeDirection ZoombiniPuzzleMaze::kSeatMoveDirection[14];
constexpr ZoombiniPuzzleMaze::SeatCommonImageIndex ZoombiniPuzzleMaze::kSeatCommonImageIndex[14];
constexpr ZoombiniPuzzleMaze::MazeCellType ZoombiniPuzzleMaze::kBaseNodeTypes[18];
constexpr Common::Point ZoombiniPuzzleMaze::kBaseNodeCoords[18];
constexpr int16 ZoombiniPuzzleMaze::kTraitOffsets[4];
constexpr Common::Point ZoombiniPuzzleMaze::kArrivalPositions[80];
constexpr int16 ZoombiniPuzzleMaze::kCreatureTypeScrbTable[3];
constexpr int16 ZoombiniPuzzleMaze::kPathSelectThresholds[20];
constexpr ZoombiniPuzzleMaze::PathSlotTraitCategory ZoombiniPuzzleMaze::kSlotTraitCategories[21];
constexpr int16 ZoombiniPuzzleMaze::kScoreToLoopCount[17];
constexpr int16 ZoombiniPuzzleMaze::kStaticPathPool[11];
constexpr int16 ZoombiniPuzzleMaze::kSeatFlagValue[14];
constexpr ZmbTrait::TraitKind ZoombiniPuzzleMaze::kTraitSlotKind[21];
constexpr int16 ZoombiniPuzzleMaze::kTraitSlotValue[21];

// =================================================================
// Construction / Lifecycle
// =================================================================
ZoombiniPuzzleMaze::ZoombiniPuzzleMaze(MohawkEngine_Zoombini *vm) : ZoombiniPuzzle(vm, ZoombiniPageType::kMaze, ZmbSrcPageKind::kMaze_16) {
	memset(_cellTypes, 0, sizeof(_cellTypes));
	memset(_cellStateIdx, 0xFF, sizeof(_cellStateIdx));
	memset(_cellTraitKind, 0, sizeof(_cellTraitKind));
	memset(_cellTraitValue, 0, sizeof(_cellTraitValue));
	memset(_nodeDirFlags, 0, sizeof(_nodeDirFlags));
	memset(_nodeDirection, 0, sizeof(_nodeDirection));
	memset(_nodeCycleFlag, 0, sizeof(_nodeCycleFlag));
	memset(_gridCellPos, 0, sizeof(_gridCellPos));
	memset(_collisionReservationCount, 0, sizeof(_collisionReservationCount));
	memset(_collisionReservationRunnerIdx, 0xFF, sizeof(_collisionReservationRunnerIdx));
	memset(_runnerSnoidIds, 0, sizeof(_runnerSnoidIds));
	memset(_launcherSetupQueue, 0, sizeof(_launcherSetupQueue));
	memset(_movementQueue, 0, sizeof(_movementQueue));
	memset(_collisionOverlayRelinkQueue, 0, sizeof(_collisionOverlayRelinkQueue));
	memset(_traversalColumnRelinkQueue, 0, sizeof(_traversalColumnRelinkQueue));
	memset(_acceptedCelebrationQueue, 0, sizeof(_acceptedCelebrationQueue));
	memset(_exitCompletionQueue, 0, sizeof(_exitCompletionQueue));
	memset(_cellArrivalQueue, 0, sizeof(_cellArrivalQueue));
	memset(_collisionPairQueue, 0, sizeof(_collisionPairQueue));
	resetArrivalPosCounters();
	memset(_pathTraitMatchCounts, 0, sizeof(_pathTraitMatchCounts));
	memset(_connectionTable, 0, sizeof(_connectionTable));
	memset(_freePathSlotList, 0, sizeof(_freePathSlotList));
	memset(_reachablePathSlots, 0, sizeof(_reachablePathSlots));
	memset(_selectedPathSlots, 0, sizeof(_selectedPathSlots));
	clearTraitSelectionTables();
	memset(_shuffledPathPool, 0, sizeof(_shuffledPathPool));
	memset(_placedRunnerIndices, 0, sizeof(_placedRunnerIndices));
	memset(_launcherOccupancyMarkers, 0, sizeof(_launcherOccupancyMarkers));
	memset(_launcherRunnerIndices, 0xFF, sizeof(_launcherRunnerIndices));
	memset(_launcherSeatEnabled, 0, sizeof(_launcherSeatEnabled));
	memset(_waveGroupShapeBase, 0, sizeof(_waveGroupShapeBase));
	for (int i = 0; i < kMaxRunners; i++)
		_runnerStates[i].clear();
}

ZoombiniPuzzleMaze::~ZoombiniPuzzleMaze() {
}

const ZoombiniPage::ScriptSoundPriorityRanges &ZoombiniPuzzleMaze::getScriptSoundPriorityRanges() const {
	// Original shared page dispatch automatically prepends SND 996-997 at priority 32.
	static const ZoombiniPage::ScriptSoundPriorityRanges kRanges = {
		{kSysResSoundRange0996_DepartSFX, kSysResSoundRange0997_ArriveSFX},
		// Maze then registers the same range again in its authored list.
		{kSysResSoundRange0996_DepartSFX, kSysResSoundRange0997_ArriveSFX},
		{kSysResSoundRange20000_Base, kSysResSoundRange29999_Last},
		{kResSoundRange11000_SpecialBase, kResSoundRange11000_SpecialBase},
		{kResSoundRange5104_ImpactLast, kResSoundRange5104_ImpactLast},
		{kResSoundRange10000_CellBase, kResSoundRange10000_CellBase},
		{kResSoundRange10002_CollisionBubbleStart, kResSoundRange10004_CollisionBubble0},
		{kResSoundRange9000_PathBase, kResSoundRange9001_PathShadow},
		{kResSoundRange5100_ImpactBase, kResSoundRange5103_ImpactFinal},
		{kResSoundRange12000_ExitBase, kResSoundRange12000_ExitBase},
		{kResSoundRange10001_CollisionOverlay0, kResSoundRange10001_CollisionOverlay0}};
	return kRanges;
}

void ZoombiniPuzzleMaze::open() {
	// MIDI BGM (tMID 30035-30038) - Broderbund v1.x only.
	if (!_vm->isVersionFamilyTlcV2())
		openMidiArchive();
	openArchive(ZMB_MHK_MAZE2);
}

void ZoombiniPuzzleMaze::setBackgroundMusic() {
	// Select the MIDI track from the active route difficulty.
	if (!_vm->isVersionFamilyTlcV2()) {
		const int16 routeLevel = _difficultyLevel - kPuzzleLevel1;
		_vm->_midi->playZmbMidi(ZmbResource(ZmbResource::kPage, static_cast<int16>(kResMidi30035_MazeBgmBase + routeLevel)));
	}
}

void ZoombiniPuzzleMaze::setBackgroundBitmap() {
	_vm->_gfx->setPalette(kResBackground5000);
	_vm->_gfx->drawBackground(kResBackground5000);
}

void ZoombiniPuzzleMaze::initStates() {
	_layoutLevel = static_cast<MazeLayoutLevel>(_difficultyLevel);

	_celebrationTrigger = false;
	_celebrationsPlayed = 0;
	_celebrationTarget = 0;
	_celebrationPoolState = 0;
	_celebrationLastFrame = 0;
	_upperLeftArrivalDepthReorderPending = false;
	_upperRightArrivalDepthReorderPending = false;

	// Go starts disabled until the first accepted upper-right boundary entry.
	_goButtonEnabled = false;
	_acceptedBoundaryEntryCount = 0;
}

void ZoombiniPuzzleMaze::loadFeatures() {
	// Load terrain barrier bitmap (tBMP 100)
	loadTerrainBitmap(kResBitmapTerrain100);

	// Preload shape images
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape5100_Creature));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape7000_Maze));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape8000_Mudball));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape9000_Path));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape10000_Cell));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape12000_Exit));

	// Load main features: 28 SCRBs at 7000
	ZmbFeature *mainFeature = createPuzzleMainFeatureHead();

	{
		ZmbFeature *parent = mainFeature;
		for (uint16 i = 0; i < 14; i++)
			parent = loadSubFeature(parent, ZmbResource(ZmbResource::kPage, kResBitmapShape8000_Mudball), kResScrb8000_MudballBase + i);
	}

	{
		ZmbFeature *parent = mainFeature;
		for (uint16 i = 0; i < 8; i++)
			parent = loadSubFeature(parent, ZmbResource(ZmbResource::kPage, kResBitmapShape9000_Path), kResScrb9000_PathBase + i);
	}

	{
		ZmbFeature *parent = mainFeature;
		for (uint16 i = 0; i < 44; i++)
			parent = loadSubFeature(parent, ZmbResource(ZmbResource::kPage, kResBitmapShape10000_Cell), kResScrb10000_CellBase + i);
	}

	{
		ZmbFeature *parent = mainFeature;
		for (uint16 i = 0; i < 2; i++)
			parent = loadSubFeature(parent, ZmbResource(ZmbResource::kPage, kResBitmapShape12000_Exit), kResScrb12000_ExitBase + i);
	}

	loadRegsCoordinateTables();

	// Load reject pool: 8 at SCRS 14000
	registerScrsGroup(kResScrs14000_RejectBase, 8);

	// Load normal pool: 96 at SCRS 15000
	registerScrsGroup(kResScrs15000_NormalBase, 96);

	// Overlay anim feature
	_collisionOverlayAnchorFeature = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape12000_Exit), kResScrb12001_Overlay, 7,
		ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE);

	loadZoombinisFromPack(kSnoidPositions, ARRAYSIZE(kSnoidPositions));
	_celebrationTarget = _pageLoadedZmbCount - 1;

	// At @ref kPuzzleLevel4, fewer than five Zoombinis use the fixed REGS 16609 layout.
	if (_difficultyLevel == kPuzzleLevel4 && _pageLoadedZmbCount < 5)
		_layoutLevel = kMazeLayoutLevel4SmallPack;

	loadRegsConfigByLevel();
	loadAndParseRegsData();
	createCreatureFeatures();

	// The shared type-0 companion precedes the grid-cell layer.
	_launcherCompanionFeatures[0] = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape9000_Path), kResScrb9005_CreatureBase, 7,
		ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_04000000_OVERLAY);

	if (0 < _pageLoadedZmbCount)
		initGridAndSelectPaths();

	// The v1.11KR Win16 original reverses two grid-cell feature flags.
	// It sets LOOP_ANIM and omits DEFER_RENDER (0x05980000), while the intended setup clears LOOP_ANIM and sets DEFER_RENDER. (0x04980000)
	// That mistake disables animation of whirl/sticky cells.
	// All other versions and binaries use the intended setup.

	// Port renderer strategy for the REGS grid cells.
	//
	// The original allocates one callback-only runner per grid cell.
	// Each of those runners carries SCRB id zero, its own custom pre-render and post-render
	// callbacks, a per-cell frame interval drawn randomly from 20 to 25, and the flag set
	// LOOP_ANIM, DEFER_ANIM, PLAY_ONCE, and OVERLAY.
	// Every cell runner is then relinked immediately after the last active type-1 launcher,
	// or after the SCRB 12001 overlay runner when no such launcher exists.
	//
	// This port keeps shape composition and independent cell state in a single callback-only
	// render runner instead, with page-owned cell state and direct tBMP composition.
	// @ref ZoombiniPuzzleMaze::renderGridCells() reproduces the per-cell behavior the original
	// obtained from separate runners: it advances each cell on its own 20-to-25 frame interval,
	// keeps per-cell direction and activation state, and composes the multi-layer cells in
	// original layer order.
	// LOOP_ANIM places this runner in the loop-animation bucket, which is where the original
	// cell runners also land, so the resulting depth relative to launchers, overlays, and
	// Snoids is unchanged.
	// SCRB id zero carries no frames, so no SCRB playback starts; the render callback owns all drawing.
	// DEFER_ANIM and PLAY_ONCE are deliberately not carried over: on a single shared runner they
	// would gate the callback that drives every cell.
	{
		ZmbFeature::EventHooks hooks;
		hooks.setRenderFunc(static_cast<ZmbFeature::OnRenderFunc>(&ZoombiniPuzzleMaze::renderGridCells));
		loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape5100_Creature), 0, 0,
						ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_04000000_OVERLAY,
						hooks);
	}

	// Reserve eleven callback-free overlay layers.
	for (int i = 0; i < 11; i++) {
		_columnDepthAnchors[i] = loadScrbFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape8000_Mudball), kResScrb8011_Noop, 0,
			ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00008000_LOOP_ANIM);
	}

	createRemainingCreatureFeatures();

	// Final SCRB 8011
	loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape8000_Mudball), kResScrb8011_Noop, 0,
					ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00008000_LOOP_ANIM);

	// Load the two final foreground overlays.
	loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape8000_Mudball), kResScrb8004_FinalOverlayA, 0,
					ZmbFeature::FLAG_04000000_OVERLAY);

	loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape8000_Mudball), kResScrb8000_MudballBase, 0,
					ZmbFeature::FLAG_04000000_OVERLAY);

	// Reserve the final callback-free overlay layer.
	_columnDepthAnchors[11] = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape8000_Mudball), kResScrb8011_Noop, 0,
		ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00008000_LOOP_ANIM);

	layoutStaticAndWalkIn(0, false);
	// Materialize the feature list and advance each independent cell visual
	// before drawing the Snoid walk-in delays.
	renderFeatures();
	assignStaggeredWalkDelays(30, 45);
	// Maze starts with no accepted Snoids.
	// The flag is set only for boundary cell type 23.
	schedulePackSnoids(false, false);

	configureStandardPuzzleControlRects();
	loadStandardPuzzleControlFeatures(kResBitmapShape6000_GoMapButtons);
	queueScriptSoundForNextRenderPass(ZmbResource(ZmbResource::kSystem, kSysResSound0997_ArriveSFX));
}

void ZoombiniPuzzleMaze::initHelpPrompt() {
	_activeHelpSoundId = ZmbResource(ZmbResource::kSystem, kSysResSound20068_Help);
}

void ZoombiniPuzzleMaze::debugPrepareForDeparture() {
	if (isDragging())
		finishSnoidDrag();

	// Stop any in-progress traversal. A runner that has already fallen into a
	// whirlpool is render-disabled and is intentionally not restored or saved as
	// an accepted runner.
	_launcherSetupQueueSize = 0;
	_movementQueueSize = 0;
	_collisionOverlayRelinkQueueSize = 0;
	_traversalColumnRelinkQueueSize = 0;
	_acceptedCelebrationQueueSize = 0;
	_exitCompletionQueueSize = 0;
	_cellArrivalQueueSize = 0;
	_collisionPairQueueSize = 0;
	_queueProcessingEnabled = false;
	_celebrationTrigger = false;
	_celebrationsPlayed = 0;
	_upperLeftArrivalDepthReorderPending = false;
	_upperRightArrivalDepthReorderPending = false;
	_placedRunnerCount = 0;
	// Unlike the other debug-finish paths, Maze must preserve the rejected
	// runners as non-occupied entries so the shared save routine can route them
	// away instead of reviving them.
	_debugFinishAllAccepted = false;
	_acceptedBoundaryEntryCount = 0;
	_acceptedExitCompletionCount = 0;
	resetArrivalPosCounters();

	int16 exitIndex = 0;
	for (int16 runnerIdx = 0; runnerIdx < _runnerCount; runnerIdx++) {
		ZmbSnoid *snoid = getSnoid(_runnerSnoidIds[runnerIdx]);
		const bool survives = snoid && snoid->isPackSnoid() && snoid->isRenderActivated();

		releaseRunnerTraversalVisuals(runnerIdx);
		_runnerStates[runnerIdx].clear();

		if (!survives)
			continue;

		const int16 arrivalIndex = CLIP<int16>(exitIndex, 0, 19);
		const Common::Point exitPos = kArrivalPositions[60 + arrivalIndex];
		addExternalDirtyRect(snoid->getClickRect());
		snoid->setPointLoc(exitPos);
		snoid->setAnimTargetPos(exitPos);
		snoid->setFlags(ZmbFeature::FLAG_00000001_TYPE_SNOID | ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_04000000_OVERLAY);
		snoid->setCommonImageIndex(0);
		snoid->setAnimState(kSnoidAnimState000_Idle);
		snoid->setupIdleHotspots();
		snoid->activateRender();
		snoid->_packIsOccupied = true;
		snoid->setNeedsRedraw(true);
		prepareSnoidVisualCoverage(snoid, true);
		addExternalDirtyRect(snoid->getClickRect());

		_runnerStates[runnerIdx].cornerGroup = kUpperRight03;
		_runnerStates[runnerIdx].cornerArrivalPosIdx = arrivalIndex;
		_runnerStates[runnerIdx].feetTrait = snoid->_trait._feet;
		exitIndex += 1;
	}

	_acceptedBoundaryEntryCount = exitIndex;
	_goButtonEnabled = 0 < exitIndex;
	setGoButtonsEnabled(_goButtonEnabled);
}

const char *ZoombiniPuzzleMaze::debugMazeDirectionName(int16 direction) {
	static constexpr const char *kDirectionNames[] = {"West", "South", "East", "North"};
	return (0 <= direction && direction < 4) ? kDirectionNames[direction] : "unknown direction";
}

const char *ZoombiniPuzzleMaze::debugMazeColorName(MazeColorShapeBase shapeBase) {
	switch (shapeBase) {
	case MazeColorShapeBase::kRed52:
		return "Red";
	case MazeColorShapeBase::kOrange73:
		return "Orange";
	case MazeColorShapeBase::kYellow94:
		return "Yellow";
	case MazeColorShapeBase::kGreen115:
		return "Green";
	case MazeColorShapeBase::kCyan136:
		return "Cyan";
	case MazeColorShapeBase::kPurple157:
		return "Purple";
	case MazeColorShapeBase::kMagenta178:
		return "Magenta";
	default:
		return "Unknown-color";
	}
}

const char *ZoombiniPuzzleMaze::debugMazeOutcomeName(LaunchSimulationOutcome outcome) {
	switch (outcome) {
	case LaunchSimulationOutcome::kExitLowerLeft00:
		return "then returns through the left/center exit";
	case LaunchSimulationOutcome::kExitUpperLeft01:
		return "then returns through the top exit";
	case LaunchSimulationOutcome::kExitLowerRight02:
		return "then returns through the right exit";
	case LaunchSimulationOutcome::kExitUpperRight03:
		return "then reaches the completion exit";
	case LaunchSimulationOutcome::kRejectedByWhirlpoolMinus01:
		return "then is rejected by a trap";
	case LaunchSimulationOutcome::kHeldAtColoredStickyMinus02:
		return "then stops at a sticky cell";
	case LaunchSimulationOutcome::kStepLimitExceededMinus03:
		return "then remains in a loop";
	case LaunchSimulationOutcome::kInvalidTraversalMinus04:
		return "then leaves the valid grid";
	}
	return "then leaves the valid grid";
}

int16 ZoombiniPuzzleMaze::debugMazeNextDirection(int16 direction, const bool dirFlags[4]) {
	for (int16 attempts = 0; attempts < 4; attempts++) {
		direction += 1;
		if (kMazeDirection03_North < direction)
			direction = kMazeDirection00_West;
		if (dirFlags[direction])
			break;
	}
	return direction;
}

int16 ZoombiniPuzzleMaze::debugMazeInitialDirection(const Common::Array<int16> &regsData, int16 cellIdx, int16 fallback) {
	const int32 directionOffset = 10 + 10 * cellIdx + 8;
	if (cellIdx < 0 || static_cast<int32>(regsData.size()) <= directionOffset)
		return fallback;
	return regsData[directionOffset];
}

bool ZoombiniPuzzleMaze::debugMazeContainsCell(const Common::Array<int16> &cells, int16 cellIdx) {
	for (uint idx = 0; idx < cells.size(); idx++) {
		if (cells[idx] == cellIdx)
			return true;
	}
	return false;
}

bool ZoombiniPuzzleMaze::debugMazeIsPreferredKeyOutcome(LaunchSimulationOutcome outcome) {
	return outcome == LaunchSimulationOutcome::kHeldAtColoredStickyMinus02 ||
		   (LaunchSimulationOutcome::kExitLowerLeft00 <= outcome && outcome <= LaunchSimulationOutcome::kExitUpperRight03);
}

bool ZoombiniPuzzleMaze::debugMazeArrayContains(const Common::Array<int16> &values, int16 value) {
	for (uint idx = 0; idx < values.size(); idx++) {
		if (values[idx] == value)
			return true;
	}
	return false;
}

byte ZoombiniPuzzleMaze::debugMazeTraitValue(const ZmbTrait &trait, int16 category) {
	if (category < 1 || 4 < category)
		return 0;
	return trait.getTraitValue(ZmbTrait::traitKindFromIndex(category - 1));
}

void ZoombiniPuzzleMaze::debugMazeInitializePlanState(const ZoombiniPuzzleMaze::SolverPlanContext &context, ZoombiniPuzzleMaze::SolverPlanState &state) {
	memcpy(state.directions, context.initialDirections, sizeof(state.directions));
	for (int16 cellIdx = 0; cellIdx < ZoombiniPuzzleMaze::kDebugMazeMaxCells; cellIdx++)
		state.stickyHeldSnoids[cellIdx] = -1;
	for (int16 snoidIdx = 0; snoidIdx < ZoombiniPuzzleMaze::kDebugMazeMaxSnoids; snoidIdx++) {
		state.snoids[snoidIdx].corner = 0;
		state.snoids[snoidIdx].row = 0;
		state.snoids[snoidIdx].col = 0;
		state.snoids[snoidIdx].direction = kMazeDirection00_West;
		state.snoids[snoidIdx].heldCellIdx = -1;
		state.snoids[snoidIdx].accepted = false;
	}
}

bool ZoombiniPuzzleMaze::debugMazeApplyPlanLaunch(const ZoombiniPuzzleMaze::SolverPlanContext &context,
												  ZoombiniPuzzleMaze::SolverPlanState &state, int16 snoidIdx, int16 seatIdx,
												  ZoombiniPuzzleMaze::SolverPlanAction &action, ZoombiniPuzzleMaze::SolverBudget &budget) {
	action.clear();
	// Count every simulated launch so the answer can report the search cost.
	// A zero limit only disables the cap, not the count.
	if (budget.limit && budget.limit <= budget.steps)
		return false;
	budget.steps += 1;
	if (snoidIdx < 0 || context.snoidCount <= snoidIdx || !context.snoids[snoidIdx] ||
		seatIdx < 0 || ZoombiniPuzzleMaze::kDebugMazeSeatCount <= seatIdx || !context.seatActive[seatIdx])
		return false;
	ZoombiniPuzzleMaze::SolverPlanSnoidState &rootState = state.snoids[snoidIdx];
	if (rootState.accepted || 0 <= rootState.heldCellIdx || rootState.corner != context.seatCorner[seatIdx])
		return false;

	action.snoidIdx = snoidIdx;
	action.launcherSeatIdx = seatIdx;
	ZoombiniPuzzleMaze::SolverMovingSnoid movingQueue[ZoombiniPuzzleMaze::kDebugMazeMaxSnoids * 2];
	int16 movingCount = 1;
	movingQueue[0].snoidIdx = snoidIdx;
	movingQueue[0].row = context.seatRow[seatIdx];
	movingQueue[0].col = context.seatCol[seatIdx];
	movingQueue[0].direction = context.seatDirection[seatIdx];
	// Row-entry launch scripts finish on the launcher cell, so their first
	// row-change dispatch must process that cell before moving beyond it.
	if (movingQueue[0].direction == kMazeDirection01_South)
		movingQueue[0].row -= 1;
	else if (movingQueue[0].direction == kMazeDirection03_North)
		movingQueue[0].row += 1;
	rootState.heldCellIdx = -1;

	while (0 < movingCount) {
		movingCount -= 1;
		ZoombiniPuzzleMaze::SolverMovingSnoid moving = movingQueue[movingCount];
		bool routeEnded = false;
		for (int16 step = 0; step < 256 && !routeEnded; step++) {
			ZmbGridDirection gridDirection;
			if (!decodeMazeGridDirection(moving.direction, gridDirection))
				return false;
			const ZmbGridStep gridStep = ZmbGridTraversal::computeStep(
				ZmbGridCell(moving.row, moving.col), gridDirection,
				ZmbGridBounds(0, ZoombiniPuzzleMaze::kDebugMazeRows - 1, 0, ZoombiniPuzzleMaze::kDebugMazeCols - 1));
			if (!gridStep.inBounds)
				return false;
			moving.row = gridStep.cell.row;
			moving.col = gridStep.cell.col;
			const MazeCellType cellType = context.cellTypes[moving.row][moving.col];
			const int16 cellIdx = context.cellIndices[moving.row][moving.col];
			switch (cellType) {
			case kMazeCellType01_Whirlpool:
				state.snoids[moving.snoidIdx].corner = -2;
				state.snoids[moving.snoidIdx].heldCellIdx = -1;
				routeEnded = true;
				break;
			case kMazeCellType02_TraitArrow: {
				const int16 category = context.traitCategories[moving.row][moving.col];
				const int16 value = context.traitValues[moving.row][moving.col];
				const bool matched = debugMazeTraitValue(context.snoids[moving.snoidIdx]->_trait, category) == value;
				ZoombiniPuzzleMaze::SolverTraitDecision decision;
				decision.cellIdx = cellIdx;
				decision.matched = matched;
				action.traitDecisions.push_back(decision);
				if (matched)
					moving.direction = state.directions[moving.row][moving.col];
				break;
			}
			case kMazeCellType03_FixedArrow:
			case kMazeCellType04_ColoredArrow:
				moving.direction = state.directions[moving.row][moving.col];
				if (0 <= cellIdx && context.cells[cellIdx].cycleOnPass)
					state.directions[moving.row][moving.col] = debugMazeNextDirection(state.directions[moving.row][moving.col], context.directionFlags[moving.row][moving.col]);
				break;
			case kMazeCellType05_ColoredSticky: {
				if (cellIdx < 0)
					return false;
				const int16 captive = state.stickyHeldSnoids[cellIdx];
				if (0 <= captive) {
					state.stickyHeldSnoids[cellIdx] = -1;
					state.snoids[captive].heldCellIdx = -1;
					state.snoids[captive].direction = moving.direction;
					if (static_cast<int16>(ARRAYSIZE(movingQueue)) <= movingCount)
						return false;
					movingQueue[movingCount].snoidIdx = captive;
					movingQueue[movingCount].row = moving.row;
					movingQueue[movingCount].col = moving.col;
					movingQueue[movingCount].direction = moving.direction;
					movingCount += 1;
					action.pushedSnoids.push_back(captive);
				}
				state.stickyHeldSnoids[cellIdx] = moving.snoidIdx;
				ZoombiniPuzzleMaze::SolverPlanSnoidState &held = state.snoids[moving.snoidIdx];
				held.corner = -1;
				held.row = moving.row;
				held.col = moving.col;
				held.direction = moving.direction;
				held.heldCellIdx = cellIdx;
				routeEnded = true;
				break;
			}
			case kMazeCellType06_ColoredSwitch: {
				if (cellIdx < 0)
					return false;
				const int16 waveGroup = context.cells[cellIdx].waveGroup;
				if (!debugMazeArrayContains(action.switchGroups, waveGroup))
					action.switchGroups.push_back(waveGroup);
				for (int16 linkedIdx = 0; linkedIdx < context.cellCount; linkedIdx++) {
					const MazeCellState &linked = context.cells[linkedIdx];
					if (linked.waveGroup != waveGroup)
						continue;
					if (linked.type == kMazeCellType04_ColoredArrow) {
						state.directions[linked.row][linked.col] = debugMazeNextDirection(state.directions[linked.row][linked.col], linked.dirFlags);
					} else if (linked.type == kMazeCellType05_ColoredSticky && 0 <= state.stickyHeldSnoids[linkedIdx]) {
						const int16 released = state.stickyHeldSnoids[linkedIdx];
						state.stickyHeldSnoids[linkedIdx] = -1;
						ZoombiniPuzzleMaze::SolverPlanSnoidState &releasedState = state.snoids[released];
						releasedState.heldCellIdx = -1;
						if (static_cast<int16>(ARRAYSIZE(movingQueue)) <= movingCount)
							return false;
						movingQueue[movingCount].snoidIdx = released;
						movingQueue[movingCount].row = releasedState.row;
						movingQueue[movingCount].col = releasedState.col;
						movingQueue[movingCount].direction = releasedState.direction;
						movingCount += 1;
						action.releasedSnoids.push_back(released);
					}
				}
				break;
			}
			case kMazeCellType20_ExitLowerLeft:
			case kMazeCellType21_ExitUpperLeft:
			case kMazeCellType22_ExitLowerRight:
			case kMazeCellType23_ExitUpperRight: {
				ZoombiniPuzzleMaze::SolverPlanSnoidState &arrived = state.snoids[moving.snoidIdx];
				arrived.corner = static_cast<int16>(cellType) - static_cast<int16>(kMazeCellType20_ExitLowerLeft);
				arrived.row = moving.row;
				arrived.col = moving.col;
				arrived.direction = moving.direction;
				arrived.heldCellIdx = -1;
				if (cellType == kMazeCellType23_ExitUpperRight)
					arrived.accepted = true;
				routeEnded = true;
				break;
			}
			default:
				break;
			}
		}
		if (!routeEnded)
			return false;
	}

	const ZoombiniPuzzleMaze::SolverPlanSnoidState &result = state.snoids[snoidIdx];
	if (result.accepted)
		action.rootOutcome = 3;
	else if (0 <= result.heldCellIdx)
		action.rootOutcome = -2;
	else if (0 <= result.corner && result.corner <= 2)
		action.rootOutcome = result.corner;
	else
		action.rootOutcome = -1;
	return true;
}

bool ZoombiniPuzzleMaze::debugMazePlanSnoidAccepted(const ZoombiniPuzzleMaze::SolverPlanState &state, int16 snoidIdx) {
	return 0 <= snoidIdx && snoidIdx < ZoombiniPuzzleMaze::kDebugMazeMaxSnoids && state.snoids[snoidIdx].accepted;
}

bool ZoombiniPuzzleMaze::debugMazePlanSnoidHeldInGroup(const ZoombiniPuzzleMaze::SolverPlanContext &context, const ZoombiniPuzzleMaze::SolverPlanState &state, int16 snoidIdx, int16 waveGroup) {
	if (snoidIdx < 0 || context.snoidCount <= snoidIdx)
		return false;
	const int16 cellIdx = state.snoids[snoidIdx].heldCellIdx;
	return 0 <= cellIdx && cellIdx < context.cellCount && context.cells[cellIdx].waveGroup == waveGroup;
}

bool ZoombiniPuzzleMaze::debugMazePlanAllAccepted(const ZoombiniPuzzleMaze::SolverPlanContext &context, const ZoombiniPuzzleMaze::SolverPlanState &state) {
	for (int16 snoidIdx = 0; snoidIdx < context.snoidCount; snoidIdx++) {
		if (!state.snoids[snoidIdx].accepted)
			return false;
	}
	return true;
}

bool ZoombiniPuzzleMaze::debugMazePlanDistinctKeys(int16 stagingKey, int16 primaryKey, int16 rescueKey, int16 pusherKey, int16 finalKey) {
	return stagingKey != primaryKey && stagingKey != rescueKey && stagingKey != pusherKey &&
		   stagingKey != finalKey && primaryKey != rescueKey && primaryKey != pusherKey &&
		   primaryKey != finalKey && rescueKey != pusherKey && rescueKey != finalKey &&
		   pusherKey != finalKey;
}

bool ZoombiniPuzzleMaze::debugMazePlanIsStagingKey(const int16 stagingKeys[ZoombiniPuzzleMaze::kDebugMazeMaxStagingKeys], int16 stagingKeyCount, int16 snoidIdx) {
	for (int16 keyIdx = 0; keyIdx < stagingKeyCount; keyIdx++) {
		if (stagingKeys[keyIdx] == snoidIdx)
			return true;
	}
	return false;
}

bool ZoombiniPuzzleMaze::debugTransferMazeStagingKey(const ZoombiniPuzzleMaze::SolverPlanContext &context,
													 ZoombiniPuzzleMaze::SolverPlanState &state, int16 snoidIdx,
													 int16 requiredCorner, ZoombiniPuzzleMaze::SolverPlanAction &action,
													 ZoombiniPuzzleMaze::SolverBudget &budget) {
	for (int16 seatIdx = 0; seatIdx < ZoombiniPuzzleMaze::kDebugMazeSeatCount; seatIdx++) {
		if (!context.seatActive[seatIdx] || context.seatCorner[seatIdx] != 0)
			continue;
		ZoombiniPuzzleMaze::SolverPlanState candidateState = state;
		ZoombiniPuzzleMaze::SolverPlanAction candidateAction;
		if (!debugMazeApplyPlanLaunch(context, candidateState, snoidIdx, seatIdx, candidateAction, budget) ||
			!candidateAction.switchGroups.empty() || candidateAction.rootOutcome < 1 ||
			2 < candidateAction.rootOutcome)
			continue;
		const int16 stagingCorner = candidateState.snoids[snoidIdx].corner;
		if (0 <= requiredCorner && stagingCorner != requiredCorner)
			continue;
		state = candidateState;
		action = candidateAction;
		return true;
	}
	return false;
}

bool ZoombiniPuzzleMaze::debugCompleteExpandedMazeLevel4Plan(
	const ZoombiniPuzzleMaze::SolverPlanContext &context,
	const ZoombiniPuzzleMaze::SolverLevel4StagedRelay &stagedRelay,
	ZoombiniPuzzleMaze::SolverLevel4Plan &plan, ZoombiniPuzzleMaze::SolverBudget &budget) {
	const ZoombiniPuzzleMaze::SolverPlanState &highwayState = stagedRelay.postHighwayState;
	const int16 *stagingKeys = stagedRelay.stagingKeys;
	const ZoombiniPuzzleMaze::SolverPlanAction *stagingActions = stagedRelay.stagingActions;
	const ZoombiniPuzzleMaze::SolverPlanAction *highwayActions = stagedRelay.highwayActions;
	const int16 stagingKeyCount = stagedRelay.stagingKeyCount;
	const int16 stagingCorner = stagedRelay.stagingCorner;
	const int16 stagingStickyGroup = stagedRelay.stagingStickyGroup;
	const int16 primaryKey = stagedRelay.primaryKey;
	const ZoombiniPuzzleMaze::SolverPlanAction &primaryAction = stagedRelay.primaryAction;
	int16 heldStagingKey = -1;
	for (int16 keyIdx = 0; keyIdx < stagingKeyCount; keyIdx++) {
		const int16 stagingKey = stagingKeys[keyIdx];
		if (debugMazePlanSnoidAccepted(highwayState, stagingKey))
			continue;
		if (0 <= heldStagingKey || highwayState.snoids[stagingKey].heldCellIdx < 0)
			return false;
		heldStagingKey = stagingKey;
	}
	if (heldStagingKey < 0)
		return false;

	for (int16 rescueKey = 0; rescueKey < context.snoidCount; rescueKey++) {
		if (rescueKey == primaryKey ||
			debugMazePlanIsStagingKey(stagingKeys, stagingKeyCount, rescueKey))
			continue;
		for (int16 pusherKey = 0; pusherKey < context.snoidCount; pusherKey++) {
			if (pusherKey == primaryKey || pusherKey == rescueKey ||
				debugMazePlanIsStagingKey(stagingKeys, stagingKeyCount, pusherKey))
				continue;
			for (int16 finalKey = 0; finalKey < context.snoidCount; finalKey++) {
				if (finalKey == primaryKey || finalKey == rescueKey || finalKey == pusherKey ||
					debugMazePlanIsStagingKey(stagingKeys, stagingKeyCount, finalKey))
					continue;
				for (int16 highwaySeat = 0; highwaySeat < ZoombiniPuzzleMaze::kDebugMazeSeatCount; highwaySeat++) {
					if (!context.seatActive[highwaySeat] || context.seatCorner[highwaySeat] != 0)
						continue;
					ZoombiniPuzzleMaze::SolverPlanState batchState = highwayState;
					int16 highwayCount = 0;
					bool batchValid = true;
					for (int16 snoidIdx = 0; snoidIdx < context.snoidCount; snoidIdx++) {
						if (debugMazePlanIsStagingKey(stagingKeys, stagingKeyCount, snoidIdx) ||
							snoidIdx == primaryKey || snoidIdx == rescueKey || snoidIdx == pusherKey ||
							snoidIdx == finalKey || batchState.snoids[snoidIdx].accepted ||
							0 <= batchState.snoids[snoidIdx].heldCellIdx)
							continue;
						ZoombiniPuzzleMaze::SolverPlanAction batchAction;
						if (!debugMazeApplyPlanLaunch(context, batchState, snoidIdx, highwaySeat, batchAction, budget) ||
							!batchState.snoids[snoidIdx].accepted || !batchAction.switchGroups.empty()) {
							batchValid = false;
							break;
						}
						highwayCount += 1;
					}
					if (!batchValid)
						continue;

					for (int16 rescueSeat = 0; rescueSeat < ZoombiniPuzzleMaze::kDebugMazeSeatCount; rescueSeat++) {
						if (!context.seatActive[rescueSeat] || context.seatCorner[rescueSeat] != 0)
							continue;
						ZoombiniPuzzleMaze::SolverPlanState rescueState = batchState;
						ZoombiniPuzzleMaze::SolverPlanAction rescueAction;
						if (!debugMazeApplyPlanLaunch(context, rescueState, rescueKey, rescueSeat, rescueAction, budget) ||
							!debugMazeArrayContains(rescueAction.switchGroups, plan.rescueGroup) ||
							!debugMazePlanSnoidAccepted(rescueState, rescueKey))
							continue;
						bool allStagingKeysReleased = true;
						bool stagingRelayValid = true;
						for (int16 keyIdx = 0; keyIdx < stagingKeyCount; keyIdx++) {
							const int16 stagingKey = stagingKeys[keyIdx];
							if (debugMazePlanSnoidAccepted(rescueState, stagingKey))
								continue;
							allStagingKeysReleased = false;
							if (!debugMazePlanSnoidHeldInGroup(
									context, rescueState, stagingKey, plan.finalStickyGroup)) {
								stagingRelayValid = false;
								break;
							}
						}
						if (!stagingRelayValid)
							continue;

						for (int16 pusherSeat = 0; pusherSeat < ZoombiniPuzzleMaze::kDebugMazeSeatCount; pusherSeat++) {
							if (!context.seatActive[pusherSeat] || context.seatCorner[pusherSeat] != 0)
								continue;
							ZoombiniPuzzleMaze::SolverPlanState pushState = rescueState;
							ZoombiniPuzzleMaze::SolverPlanAction pushAction;
							if (!debugMazeApplyPlanLaunch(context, pushState, pusherKey, pusherSeat, pushAction, budget) ||
								!debugMazeArrayContains(pushAction.pushedSnoids, primaryKey) ||
								!debugMazePlanSnoidHeldInGroup(
									context, pushState, pusherKey, plan.finalStickyGroup) ||
								!debugMazePlanSnoidAccepted(pushState, primaryKey))
								continue;

							for (int16 finalSeat = 0; finalSeat < ZoombiniPuzzleMaze::kDebugMazeSeatCount; finalSeat++) {
								if (!context.seatActive[finalSeat] || context.seatCorner[finalSeat] != 0)
									continue;
								ZoombiniPuzzleMaze::SolverPlanState finalState = pushState;
								ZoombiniPuzzleMaze::SolverPlanAction finalAction;
								if (!debugMazeApplyPlanLaunch(context, finalState, finalKey, finalSeat, finalAction, budget) ||
									!debugMazeArrayContains(finalAction.switchGroups, plan.finalStickyGroup) ||
									!debugMazeArrayContains(finalAction.releasedSnoids, pusherKey) ||
									!debugMazePlanSnoidAccepted(finalState, pusherKey) ||
									!debugMazePlanSnoidAccepted(finalState, finalKey) ||
									!debugMazePlanAllAccepted(context, finalState))
									continue;

								plan.valid = true;
								plan.stagingCorner = stagingCorner;
								plan.stagingKeyCount = stagingKeyCount;
								plan.stagingKey = heldStagingKey;
								for (int16 keyIdx = 0; keyIdx < stagingKeyCount; keyIdx++) {
									plan.stagingKeys[keyIdx] = stagingKeys[keyIdx];
									plan.stageStagingActions[keyIdx] = stagingActions[keyIdx];
									plan.stageHighwaySwitchActions[keyIdx] = highwayActions[keyIdx];
								}
								plan.primaryKey = primaryKey;
								plan.pusherKey = pusherKey;
								plan.rescueKey = rescueKey;
								plan.finalKey = finalKey;
								plan.highwaySeat = highwaySeat;
								plan.highwaySnoidCount = highwayCount;
								plan.stagingStickyGroup = stagingStickyGroup;
								plan.rescueReleasesStagingKey = allStagingKeysReleased;
								plan.stageStaging = stagingActions[0];
								plan.stagePrimary = primaryAction;
								plan.stageHighwaySwitch = highwayActions[stagingKeyCount - 1];
								plan.stageRescue = rescueAction;
								plan.stagePush = pushAction;
								plan.stageFinal = finalAction;
								return true;
							}
						}
					}
				}
			}
		}
	}
	return false;
}

bool ZoombiniPuzzleMaze::debugBuildMazeLevel4PlanForGroups(const ZoombiniPuzzleMaze::SolverPlanContext &context,
														   const ZoombiniPuzzleMaze::SolverLevel4GroupSelection &groupSelection,
														   ZoombiniPuzzleMaze::SolverLevel4Plan &plan,
														   ZoombiniPuzzleMaze::SolverBudget &budget) {
	// Build one complete relay from the initial authored control state. Prefer
	// the minimal five-key form, then reserve four staging keys when N=1 is needed.
	plan.primaryGroup = groupSelection.primaryGroup;
	plan.finalStickyGroup = groupSelection.finalStickyGroup;
	plan.rescueGroup = groupSelection.rescueGroup;
	plan.highwayGroup = groupSelection.highwayGroup;

	ZoombiniPuzzleMaze::SolverPlanState initialState;
	debugMazeInitializePlanState(context, initialState);
	for (int16 stagingKey = 0; stagingKey < context.snoidCount; stagingKey++) {
		for (int16 stagingSeat = 0; stagingSeat < ZoombiniPuzzleMaze::kDebugMazeSeatCount; stagingSeat++) {
			if (!context.seatActive[stagingSeat] || context.seatCorner[stagingSeat] != 0)
				continue;
			ZoombiniPuzzleMaze::SolverPlanState stagedState = initialState;
			ZoombiniPuzzleMaze::SolverPlanAction stagingAction;
			if (!debugMazeApplyPlanLaunch(context, stagedState, stagingKey, stagingSeat, stagingAction, budget) ||
				!stagingAction.switchGroups.empty() || stagingAction.rootOutcome < 1 || 2 < stagingAction.rootOutcome)
				continue;
			const int16 stagingCorner = stagedState.snoids[stagingKey].corner;
			bool hasStagingLauncher = false;
			for (int16 seatIdx = 0; seatIdx < ZoombiniPuzzleMaze::kDebugMazeSeatCount; seatIdx++) {
				if (context.seatActive[seatIdx] && context.seatCorner[seatIdx] == stagingCorner) {
					hasStagingLauncher = true;
					break;
				}
			}
			if (!hasStagingLauncher)
				continue;

			for (int16 primaryKey = 0; primaryKey < context.snoidCount; primaryKey++) {
				if (primaryKey == stagingKey)
					continue;
				for (int16 primarySeat = 0; primarySeat < ZoombiniPuzzleMaze::kDebugMazeSeatCount; primarySeat++) {
					if (!context.seatActive[primarySeat] || context.seatCorner[primarySeat] != 0)
						continue;
					ZoombiniPuzzleMaze::SolverPlanState primaryState = stagedState;
					ZoombiniPuzzleMaze::SolverPlanAction primaryAction;
					if (!debugMazeApplyPlanLaunch(context, primaryState, primaryKey, primarySeat, primaryAction, budget) ||
						!debugMazeArrayContains(primaryAction.switchGroups, plan.primaryGroup) ||
						!debugMazePlanSnoidHeldInGroup(context, primaryState, primaryKey, plan.finalStickyGroup))
						continue;

					for (int16 stagingExitSeat = 0; stagingExitSeat < ZoombiniPuzzleMaze::kDebugMazeSeatCount; stagingExitSeat++) {
						if (!context.seatActive[stagingExitSeat] ||
							context.seatCorner[stagingExitSeat] != stagingCorner)
							continue;
						ZoombiniPuzzleMaze::SolverPlanState highwayState = primaryState;
						ZoombiniPuzzleMaze::SolverPlanAction highwaySwitchAction;
						if (!debugMazeApplyPlanLaunch(context, highwayState, stagingKey, stagingExitSeat,
													  highwaySwitchAction, budget))
							continue;
						if (!debugMazeArrayContains(highwaySwitchAction.switchGroups, plan.highwayGroup))
							continue;
						if (highwayState.snoids[stagingKey].heldCellIdx < 0)
							continue;
						const int16 stagingStickyGroup = context.cells[highwayState.snoids[stagingKey].heldCellIdx].waveGroup;

						for (int16 rescueKey = 0; rescueKey < context.snoidCount; rescueKey++) {
							for (int16 pusherKey = 0; pusherKey < context.snoidCount; pusherKey++) {
								for (int16 finalKey = 0; finalKey < context.snoidCount; finalKey++) {
									if (!debugMazePlanDistinctKeys(stagingKey, primaryKey, rescueKey, pusherKey, finalKey))
										continue;
									for (int16 highwaySeat = 0; highwaySeat < ZoombiniPuzzleMaze::kDebugMazeSeatCount; highwaySeat++) {
										if (!context.seatActive[highwaySeat] || context.seatCorner[highwaySeat] != 0)
											continue;
										ZoombiniPuzzleMaze::SolverPlanState batchState = highwayState;
										int16 highwayCount = 0;
										bool batchValid = true;
										for (int16 snoidIdx = 0; snoidIdx < context.snoidCount; snoidIdx++) {
											if (snoidIdx == stagingKey || snoidIdx == primaryKey || snoidIdx == rescueKey ||
												snoidIdx == pusherKey || snoidIdx == finalKey || batchState.snoids[snoidIdx].accepted ||
												0 <= batchState.snoids[snoidIdx].heldCellIdx)
												continue;
											ZoombiniPuzzleMaze::SolverPlanAction batchAction;
											if (!debugMazeApplyPlanLaunch(context, batchState, snoidIdx, highwaySeat, batchAction, budget) ||
												!batchState.snoids[snoidIdx].accepted || !batchAction.switchGroups.empty()) {
												batchValid = false;
												break;
											}
											highwayCount += 1;
										}
										if (!batchValid)
											continue;

										for (int16 rescueSeat = 0; rescueSeat < ZoombiniPuzzleMaze::kDebugMazeSeatCount; rescueSeat++) {
											if (!context.seatActive[rescueSeat] || context.seatCorner[rescueSeat] != 0)
												continue;
											ZoombiniPuzzleMaze::SolverPlanState rescueState = batchState;
											ZoombiniPuzzleMaze::SolverPlanAction rescueAction;
											if (!debugMazeApplyPlanLaunch(context, rescueState, rescueKey, rescueSeat, rescueAction, budget) ||
												!debugMazeArrayContains(rescueAction.switchGroups, plan.rescueGroup) ||
												!debugMazePlanSnoidAccepted(rescueState, rescueKey))
												continue;
											const bool stagingKeyReleased = debugMazePlanSnoidAccepted(rescueState, stagingKey);
											const bool stagingKeyWaitsForFinal = debugMazePlanSnoidHeldInGroup(
												context, rescueState, stagingKey, plan.finalStickyGroup);
											if (!stagingKeyReleased && !stagingKeyWaitsForFinal)
												continue;

											for (int16 pusherSeat = 0; pusherSeat < ZoombiniPuzzleMaze::kDebugMazeSeatCount; pusherSeat++) {
												if (!context.seatActive[pusherSeat] || context.seatCorner[pusherSeat] != 0)
													continue;
												ZoombiniPuzzleMaze::SolverPlanState pushState = rescueState;
												ZoombiniPuzzleMaze::SolverPlanAction pushAction;
												if (!debugMazeApplyPlanLaunch(context, pushState, pusherKey, pusherSeat, pushAction, budget) ||
													!debugMazeArrayContains(pushAction.pushedSnoids, primaryKey) ||
													!debugMazePlanSnoidHeldInGroup(context, pushState, pusherKey, plan.finalStickyGroup) ||
													!debugMazePlanSnoidAccepted(pushState, primaryKey))
													continue;

												for (int16 finalSeat = 0; finalSeat < ZoombiniPuzzleMaze::kDebugMazeSeatCount; finalSeat++) {
													if (!context.seatActive[finalSeat] || context.seatCorner[finalSeat] != 0)
														continue;
													ZoombiniPuzzleMaze::SolverPlanState finalState = pushState;
													ZoombiniPuzzleMaze::SolverPlanAction finalAction;
													if (!debugMazeApplyPlanLaunch(context, finalState, finalKey, finalSeat, finalAction, budget) ||
														!debugMazeArrayContains(finalAction.switchGroups, plan.finalStickyGroup) ||
														!debugMazeArrayContains(finalAction.releasedSnoids, pusherKey) ||
														!debugMazePlanSnoidAccepted(finalState, pusherKey) ||
														!debugMazePlanSnoidAccepted(finalState, finalKey) ||
														!debugMazePlanAllAccepted(context, finalState))
														continue;

													plan.valid = true;
													plan.stagingCorner = stagingCorner;
													plan.stagingKeyCount = 1;
													plan.stagingKeys[0] = stagingKey;
													plan.stagingKey = stagingKey;
													plan.primaryKey = primaryKey;
													plan.pusherKey = pusherKey;
													plan.rescueKey = rescueKey;
													plan.finalKey = finalKey;
													plan.highwaySeat = highwaySeat;
													plan.highwaySnoidCount = highwayCount;
													plan.stagingStickyGroup = stagingStickyGroup;
													plan.rescueReleasesStagingKey = stagingKeyReleased;
													plan.stageStagingActions[0] = stagingAction;
													plan.stageHighwaySwitchActions[0] = highwaySwitchAction;
													plan.stageStaging = stagingAction;
													plan.stagePrimary = primaryAction;
													plan.stageHighwaySwitch = highwaySwitchAction;
													plan.stageRescue = rescueAction;
													plan.stagePush = pushAction;
													plan.stageFinal = finalAction;
													return true;
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	// Some generated packs need four staging keys (3N+1 with N=1) so every
	// highway-incompatible Snoid can be reserved for the relay instead.
	static constexpr int16 kStagingPermutations[24][ZoombiniPuzzleMaze::kDebugMazeMaxStagingKeys] = {
		{0, 1, 2, 3},
		{0, 1, 3, 2},
		{0, 2, 1, 3},
		{0, 2, 3, 1},
		{0, 3, 1, 2},
		{0, 3, 2, 1},
		{1, 0, 2, 3},
		{1, 0, 3, 2},
		{1, 2, 0, 3},
		{1, 2, 3, 0},
		{1, 3, 0, 2},
		{1, 3, 2, 0},
		{2, 0, 1, 3},
		{2, 0, 3, 1},
		{2, 1, 0, 3},
		{2, 1, 3, 0},
		{2, 3, 0, 1},
		{2, 3, 1, 0},
		{3, 0, 1, 2},
		{3, 0, 2, 1},
		{3, 1, 0, 2},
		{3, 1, 2, 0},
		{3, 2, 0, 1},
		{3, 2, 1, 0},
	};
	for (int16 key0 = 0; key0 < context.snoidCount; key0++) {
		for (int16 key1 = key0 + 1; key1 < context.snoidCount; key1++) {
			for (int16 key2 = key1 + 1; key2 < context.snoidCount; key2++) {
				for (int16 key3 = key2 + 1; key3 < context.snoidCount; key3++) {
					ZoombiniPuzzleMaze::SolverLevel4StagedRelay stagedRelay;
					stagedRelay.stagingKeyCount = ZoombiniPuzzleMaze::kDebugMazeMaxStagingKeys;
					stagedRelay.stagingKeys[0] = key0;
					stagedRelay.stagingKeys[1] = key1;
					stagedRelay.stagingKeys[2] = key2;
					stagedRelay.stagingKeys[3] = key3;
					ZoombiniPuzzleMaze::SolverPlanState stagedState = initialState;
					bool stagingValid = true;
					for (int16 keyIdx = 0; keyIdx < ZoombiniPuzzleMaze::kDebugMazeMaxStagingKeys; keyIdx++) {
						if (!debugTransferMazeStagingKey(
								context, stagedState, stagedRelay.stagingKeys[keyIdx], stagedRelay.stagingCorner,
								stagedRelay.stagingActions[keyIdx], budget)) {
							stagingValid = false;
							break;
						}
						if (stagedRelay.stagingCorner < 0)
							stagedRelay.stagingCorner = stagedState.snoids[stagedRelay.stagingKeys[keyIdx]].corner;
					}
					if (!stagingValid)
						continue;
					bool hasStagingLauncher = false;
					for (int16 seatIdx = 0; seatIdx < ZoombiniPuzzleMaze::kDebugMazeSeatCount; seatIdx++) {
						if (context.seatActive[seatIdx] && context.seatCorner[seatIdx] == stagedRelay.stagingCorner) {
							hasStagingLauncher = true;
							break;
						}
					}
					if (!hasStagingLauncher)
						continue;

					for (int16 primaryKey = 0; primaryKey < context.snoidCount; primaryKey++) {
						if (debugMazePlanIsStagingKey(stagedRelay.stagingKeys, stagedRelay.stagingKeyCount, primaryKey))
							continue;
						for (int16 primarySeat = 0; primarySeat < ZoombiniPuzzleMaze::kDebugMazeSeatCount; primarySeat++) {
							if (!context.seatActive[primarySeat] || context.seatCorner[primarySeat] != 0)
								continue;
							ZoombiniPuzzleMaze::SolverPlanState primaryState = stagedState;
							ZoombiniPuzzleMaze::SolverPlanAction primaryAction;
							if (!debugMazeApplyPlanLaunch(
									context, primaryState, primaryKey, primarySeat, primaryAction, budget) ||
								!debugMazeArrayContains(primaryAction.switchGroups, plan.primaryGroup) ||
								!debugMazePlanSnoidHeldInGroup(
									context, primaryState, primaryKey, plan.finalStickyGroup))
								continue;
							stagedRelay.primaryKey = primaryKey;
							stagedRelay.primaryAction = primaryAction;

							for (int16 stagingExitSeat = 0;
								 stagingExitSeat < ZoombiniPuzzleMaze::kDebugMazeSeatCount; stagingExitSeat++) {
								if (!context.seatActive[stagingExitSeat] ||
									context.seatCorner[stagingExitSeat] != stagedRelay.stagingCorner)
									continue;
								for (int16 permutationIdx = 0; permutationIdx < 24; permutationIdx++) {
									stagedRelay.postHighwayState = primaryState;
									bool highwayValid = true;
									bool highwaySwitchPressed = false;
									for (int16 actionIdx = 0; actionIdx < ZoombiniPuzzleMaze::kDebugMazeMaxStagingKeys; actionIdx++) {
										const int16 stagingKey = stagedRelay.stagingKeys[kStagingPermutations[permutationIdx][actionIdx]];
										ZoombiniPuzzleMaze::SolverPlanAction &action = stagedRelay.highwayActions[actionIdx];
										if (!debugMazeApplyPlanLaunch(
												context, stagedRelay.postHighwayState, stagingKey, stagingExitSeat, action, budget) ||
											action.rootOutcome == -1) {
											highwayValid = false;
											break;
										}
										for (uint switchIdx = 0; switchIdx < action.switchGroups.size(); switchIdx++) {
											if (action.switchGroups[switchIdx] != plan.highwayGroup) {
												highwayValid = false;
												break;
											}
											highwaySwitchPressed = true;
										}
										if (!highwayValid)
											break;
									}
									if (!highwayValid || !highwaySwitchPressed)
										continue;
									stagedRelay.stagingStickyGroup = 0;
									for (int16 keyIdx = 0; keyIdx < ZoombiniPuzzleMaze::kDebugMazeMaxStagingKeys; keyIdx++) {
										const int16 heldCellIdx = stagedRelay.postHighwayState.snoids[stagedRelay.stagingKeys[keyIdx]].heldCellIdx;
										if (0 <= heldCellIdx) {
											stagedRelay.stagingStickyGroup = context.cells[heldCellIdx].waveGroup;
											break;
										}
									}
									if (stagedRelay.stagingStickyGroup == 0)
										continue;
									if (debugCompleteExpandedMazeLevel4Plan(
											context, stagedRelay, plan, budget))
										return true;
								}
							}
						}
					}
				}
			}
		}
	}
	return false;
}

bool ZoombiniPuzzleMaze::debugBuildMazeLevel4Plan(const ZoombiniPuzzleMaze::SolverPlanContext &context,
												  ZoombiniPuzzleMaze::SolverLevel4Plan &plan, bool &searchExhausted,
												  uint32 &stepsUsed) {
	ZoombiniPuzzleMaze::SolverBudget budget;
	budget.steps = 0;
	budget.limit = kDebugMazeSolverMaxSteps;
	SolverStepCounter stepCounter(budget, stepsUsed);
	int16 switchCellsByGroup[9];
	int16 branchCounts[9];
	int16 stickyCounts[9];
	for (int16 group = 0; group < 9; group++) {
		switchCellsByGroup[group] = -1;
		branchCounts[group] = 0;
		stickyCounts[group] = 0;
	}
	for (int16 cellIdx = 0; cellIdx < context.cellCount; cellIdx++) {
		const MazeCellState &cell = context.cells[cellIdx];
		if (cell.waveGroup < 2 || 8 < cell.waveGroup)
			continue;
		if (cell.type == kMazeCellType06_ColoredSwitch)
			switchCellsByGroup[cell.waveGroup] = cellIdx;
		else if (cell.type == kMazeCellType04_ColoredArrow)
			branchCounts[cell.waveGroup] += 1;
		else if (cell.type == kMazeCellType05_ColoredSticky)
			stickyCounts[cell.waveGroup] += 1;
	}

	for (int16 primaryGroup = 2; primaryGroup < 9; primaryGroup++) {
		if (switchCellsByGroup[primaryGroup] < 0 || branchCounts[primaryGroup] < 2)
			continue;
		for (int16 finalStickyGroup = 2; finalStickyGroup < 9; finalStickyGroup++) {
			if (switchCellsByGroup[finalStickyGroup] < 0 || stickyCounts[finalStickyGroup] < 2)
				continue;
			for (int16 rescuePass = 0; rescuePass < 2; rescuePass++) {
				for (int16 rescueGroup = 2; rescueGroup < 9; rescueGroup++) {
					if (switchCellsByGroup[rescueGroup] < 0 || branchCounts[rescueGroup] < 1 ||
						rescueGroup == primaryGroup || rescueGroup == finalStickyGroup)
						continue;
					const bool rescueHasSticky = 0 < stickyCounts[rescueGroup];
					if ((rescuePass == 0) != rescueHasSticky)
						continue;
					for (int16 highwayGroup = 2; highwayGroup < 9; highwayGroup++) {
						if (switchCellsByGroup[highwayGroup] < 0 || branchCounts[highwayGroup] < 1 ||
							highwayGroup == primaryGroup || highwayGroup == finalStickyGroup ||
							highwayGroup == rescueGroup)
							continue;
						ZoombiniPuzzleMaze::SolverLevel4GroupSelection groupSelection;
						groupSelection.primaryGroup = primaryGroup;
						groupSelection.finalStickyGroup = finalStickyGroup;
						groupSelection.rescueGroup = rescueGroup;
						groupSelection.highwayGroup = highwayGroup;
						ZoombiniPuzzleMaze::SolverLevel4Plan candidate;
						if (debugBuildMazeLevel4PlanForGroups(context, groupSelection, candidate, budget)) {
							plan = candidate;
							budget.limit = 0;
							return true;
						}
						if (kDebugMazeSolverMaxSteps <= budget.steps) {
							budget.limit = 0;
							searchExhausted = true;
							return false;
						}
					}
				}
			}
		}
	}
	budget.limit = 0;
	searchExhausted = kDebugMazeSolverMaxSteps <= budget.steps;
	return false;
}

bool ZoombiniPuzzleMaze::debugMazePlanSameTraits(const ZoombiniPuzzleMaze::SolverPlanContext &context,
												 int16 firstSnoid, int16 secondSnoid) {
	if (firstSnoid < 0 || context.snoidCount <= firstSnoid || secondSnoid < 0 ||
		context.snoidCount <= secondSnoid || !context.snoids[firstSnoid] ||
		!context.snoids[secondSnoid])
		return false;
	for (int16 category = 1; category <= 4; category++) {
		if (debugMazeTraitValue(context.snoids[firstSnoid]->_trait, category) !=
			debugMazeTraitValue(context.snoids[secondSnoid]->_trait, category))
			return false;
	}
	return true;
}

bool ZoombiniPuzzleMaze::debugMazePlanMatchesTrait(const ZoombiniPuzzleMaze::SolverPlanContext &context, int16 snoidIdx,
												   int16 category, int16 value, bool matched) {
	if (snoidIdx < 0 || context.snoidCount <= snoidIdx || !context.snoids[snoidIdx])
		return false;
	return (debugMazeTraitValue(context.snoids[snoidIdx]->_trait, category) == value) == matched;
}

uint64 ZoombiniPuzzleMaze::debugMazePlanStateHash(const ZoombiniPuzzleMaze::SolverPlanContext &context,
												  const ZoombiniPuzzleMaze::SolverPlanState &state) {
	uint64 hash = 1469598103934665603ULL;
	for (int16 row = 0; row < ZoombiniPuzzleMaze::kDebugMazeRows; row++) {
		for (int16 col = 0; col < ZoombiniPuzzleMaze::kDebugMazeCols; col++) {
			hash ^= static_cast<uint64>(state.directions[row][col] + 5);
			hash *= 1099511628211ULL;
		}
	}
	for (int16 snoidIdx = 0; snoidIdx < context.snoidCount; snoidIdx++) {
		const ZoombiniPuzzleMaze::SolverPlanSnoidState &snoidState = state.snoids[snoidIdx];
		hash ^= static_cast<uint64>(snoidState.corner + 5);
		hash *= 1099511628211ULL;
		hash ^= static_cast<uint64>(snoidState.heldCellIdx + 2);
		hash *= 1099511628211ULL;
		hash ^= snoidState.accepted ? 1 : 0;
		hash *= 1099511628211ULL;
	}
	return hash;
}

bool ZoombiniPuzzleMaze::debugMazePlanActionPressesOnly(const ZoombiniPuzzleMaze::SolverPlanAction &action,
														int16 switchGroup) {
	return action.switchGroups.size() == 1 && action.switchGroups[0] == switchGroup;
}

bool ZoombiniPuzzleMaze::debugFindMazeAcceptedPlan(const ZoombiniPuzzleMaze::SolverPlanContext &context,
												   const ZoombiniPuzzleMaze::SolverPlanState &state,
												   ZoombiniPuzzleMaze::SolverAcceptedPlanSearch &search,
												   ZoombiniPuzzleMaze::SolverBudget &budget) {
	static constexpr int32 kMaxVisitedStates = 200000;
	if (kMaxVisitedStates <= search.visitedStateCount)
		return false;

	bool allRequiredAccepted = true;
	for (int16 snoidIdx = 0; snoidIdx < context.snoidCount; snoidIdx++) {
		if (search.requiredSnoids[snoidIdx] && !state.snoids[snoidIdx].accepted) {
			allRequiredAccepted = false;
			break;
		}
	}
	if (allRequiredAccepted) {
		if (search.finalSnoid < 0) {
			search.resultState = state;
			return true;
		}
		for (int16 seatIdx = 0; seatIdx < ZoombiniPuzzleMaze::kDebugMazeSeatCount; seatIdx++) {
			if (!context.seatActive[seatIdx] ||
				context.seatCorner[seatIdx] != state.snoids[search.finalSnoid].corner)
				continue;
			ZoombiniPuzzleMaze::SolverPlanState candidateState = state;
			ZoombiniPuzzleMaze::SolverPlanAction candidateAction;
			if (!debugMazeApplyPlanLaunch(
					context, candidateState, search.finalSnoid, seatIdx, candidateAction, budget) ||
				!debugMazePlanSnoidAccepted(candidateState, search.finalSnoid) ||
				!debugMazePlanActionPressesOnly(candidateAction, search.finalSwitchGroup))
				continue;
			search.actions.push_back(candidateAction);
			search.resultState = candidateState;
			return true;
		}
		return false;
	}

	const uint64 stateHash = debugMazePlanStateHash(context, state);
	if (search.visited.contains(stateHash))
		return false;
	search.visited[stateHash] = true;
	search.visitedStateCount += 1;

	for (int16 snoidIdx = 0; snoidIdx < context.snoidCount; snoidIdx++) {
		if (!search.requiredSnoids[snoidIdx] || state.snoids[snoidIdx].accepted ||
			0 <= state.snoids[snoidIdx].heldCellIdx || state.snoids[snoidIdx].corner < 0)
			continue;
		bool duplicateTraits = false;
		for (int16 earlierIdx = 0; earlierIdx < snoidIdx; earlierIdx++) {
			if (search.requiredSnoids[earlierIdx] && !state.snoids[earlierIdx].accepted &&
				state.snoids[earlierIdx].corner == state.snoids[snoidIdx].corner &&
				debugMazePlanSameTraits(context, earlierIdx, snoidIdx)) {
				duplicateTraits = true;
				break;
			}
		}
		if (duplicateTraits)
			continue;

		for (int16 seatIdx = 0; seatIdx < ZoombiniPuzzleMaze::kDebugMazeSeatCount; seatIdx++) {
			if (!context.seatActive[seatIdx] ||
				context.seatCorner[seatIdx] != state.snoids[snoidIdx].corner)
				continue;
			ZoombiniPuzzleMaze::SolverPlanState candidateState = state;
			ZoombiniPuzzleMaze::SolverPlanAction candidateAction;
			if (!debugMazeApplyPlanLaunch(
					context, candidateState, snoidIdx, seatIdx, candidateAction, budget) ||
				!candidateAction.switchGroups.empty() ||
				!debugMazePlanSnoidAccepted(candidateState, snoidIdx))
				continue;
			search.actions.push_back(candidateAction);
			if (debugFindMazeAcceptedPlan(context, candidateState, search, budget))
				return true;
			search.actions.pop_back();
		}
	}
	return false;
}

bool ZoombiniPuzzleMaze::debugBuildMazeSimplePlan(const ZoombiniPuzzleMaze::SolverPlanContext &context,
												  ZoombiniPuzzleMaze::SolverLevel123Plan &plan,
												  ZoombiniPuzzleMaze::SolverBudget &budget) {
	ZoombiniPuzzleMaze::SolverPlanState initialState;
	debugMazeInitializePlanState(context, initialState);
	ZoombiniPuzzleMaze::SolverAcceptedPlanSearch search;
	for (int16 snoidIdx = 0; snoidIdx < context.snoidCount; snoidIdx++)
		search.requiredSnoids[snoidIdx] = true;
	if (!debugFindMazeAcceptedPlan(context, initialState, search, budget) ||
		!debugMazePlanAllAccepted(context, search.resultState))
		return false;
	plan.valid = true;
	plan.hasSwitchPhase = false;
	plan.remainderCount = context.snoidCount;
	plan.actions = search.actions;
	return true;
}

bool ZoombiniPuzzleMaze::debugBuildMazeSwitchPlan(const ZoombiniPuzzleMaze::SolverPlanContext &context,
												  ZoombiniPuzzleMaze::SolverLevel123Plan &plan,
												  ZoombiniPuzzleMaze::SolverBudget &budget) {
	int16 switchGroups[2] = {0, 0};
	int16 switchGroupCount = 0;
	for (int16 cellIdx = 0; cellIdx < context.cellCount; cellIdx++) {
		const MazeCellState &switchCell = context.cells[cellIdx];
		if (switchCell.type != kMazeCellType06_ColoredSwitch || switchCell.waveGroup < 2)
			continue;
		bool hasLinkedBranch = false;
		for (int16 linkedIdx = 0; linkedIdx < context.cellCount; linkedIdx++) {
			if (context.cells[linkedIdx].waveGroup == switchCell.waveGroup &&
				context.cells[linkedIdx].type == kMazeCellType04_ColoredArrow) {
				hasLinkedBranch = true;
				break;
			}
		}
		if (!hasLinkedBranch)
			continue;
		bool duplicateGroup = false;
		for (int16 groupIdx = 0; groupIdx < switchGroupCount; groupIdx++) {
			if (switchGroups[groupIdx] == switchCell.waveGroup) {
				duplicateGroup = true;
				break;
			}
		}
		if (duplicateGroup)
			continue;
		if (2 <= switchGroupCount)
			return false;
		switchGroups[switchGroupCount] = switchCell.waveGroup;
		switchGroupCount += 1;
	}
	if (switchGroupCount != 2)
		return false;

	ZoombiniPuzzleMaze::SolverPlanState initialState;
	debugMazeInitializePlanState(context, initialState);
	for (int16 traitCellIdx = 0; traitCellIdx < context.cellCount; traitCellIdx++) {
		const MazeCellState &traitCell = context.cells[traitCellIdx];
		if (traitCell.type != kMazeCellType02_TraitArrow || traitCell.traitCategory < 1 || 4 < traitCell.traitCategory)
			continue;
		bool duplicateCondition = false;
		for (int16 earlierIdx = 0; earlierIdx < traitCellIdx; earlierIdx++) {
			const MazeCellState &earlier = context.cells[earlierIdx];
			if (earlier.type == kMazeCellType02_TraitArrow && earlier.traitCategory == traitCell.traitCategory &&
				earlier.traitValue == traitCell.traitValue) {
				duplicateCondition = true;
				break;
			}
		}
		if (duplicateCondition)
			continue;

		for (int16 matchPass = 0; matchPass < 2; matchPass++) {
			const bool priorityMatched = matchPass == 0;
			int16 priorityCount = 0;
			for (int16 snoidIdx = 0; snoidIdx < context.snoidCount; snoidIdx++) {
				if (debugMazePlanMatchesTrait(
						context, snoidIdx, traitCell.traitCategory,
						traitCell.traitValue, priorityMatched))
					priorityCount += 1;
			}
			if (priorityCount < 2 || context.snoidCount <= priorityCount)
				continue;

			for (int16 precursorKey = 0; precursorKey < context.snoidCount; precursorKey++) {
				const bool precursorInPriorityGroup = debugMazePlanMatchesTrait(
					context, precursorKey, traitCell.traitCategory,
					traitCell.traitValue, priorityMatched);
				if (precursorInPriorityGroup && priorityCount < 3)
					continue;
				for (int16 precursorSeat = 0; precursorSeat < ZoombiniPuzzleMaze::kDebugMazeSeatCount; precursorSeat++) {
					if (!context.seatActive[precursorSeat] || context.seatCorner[precursorSeat] != 0)
						continue;
					ZoombiniPuzzleMaze::SolverPlanState precursorState = initialState;
					ZoombiniPuzzleMaze::SolverPlanAction precursorAction;
					if (!debugMazeApplyPlanLaunch(
							context, precursorState, precursorKey, precursorSeat, precursorAction, budget) ||
						!debugMazePlanSnoidAccepted(precursorState, precursorKey) ||
						precursorAction.switchGroups.size() != 1)
						continue;
					const int16 precursorGroup = precursorAction.switchGroups[0];
					int16 finalGroup = 0;
					if (switchGroups[0] == precursorGroup)
						finalGroup = switchGroups[1];
					else if (switchGroups[1] == precursorGroup)
						finalGroup = switchGroups[0];
					if (finalGroup == 0)
						continue;

					for (int16 finalKey = 0; finalKey < context.snoidCount; finalKey++) {
						if (finalKey == precursorKey ||
							!debugMazePlanMatchesTrait(
								context, finalKey, traitCell.traitCategory,
								traitCell.traitValue, priorityMatched))
							continue;
						for (int16 victim = 0; victim < context.snoidCount; victim++) {
							if (victim == precursorKey || victim == finalKey ||
								!debugMazePlanMatchesTrait(
									context, victim, traitCell.traitCategory,
									traitCell.traitValue, priorityMatched))
								continue;
							for (int16 victimSeat = 0; victimSeat < ZoombiniPuzzleMaze::kDebugMazeSeatCount; victimSeat++) {
								if (!context.seatActive[victimSeat] ||
									context.seatCorner[victimSeat] != precursorState.snoids[victim].corner)
									continue;
								ZoombiniPuzzleMaze::SolverPlanState unprotectedState = initialState;
								ZoombiniPuzzleMaze::SolverPlanAction unprotectedAction;
								if (!debugMazeApplyPlanLaunch(
										context, unprotectedState, victim, victimSeat,
										unprotectedAction, budget) ||
									unprotectedAction.rootOutcome != -1)
									continue;
								ZoombiniPuzzleMaze::SolverPlanState victimState = precursorState;
								ZoombiniPuzzleMaze::SolverPlanAction victimAction;
								if (!debugMazeApplyPlanLaunch(
										context, victimState, victim, victimSeat, victimAction, budget) ||
									!debugMazePlanSnoidAccepted(victimState, victim) ||
									!victimAction.switchGroups.empty())
									continue;

								ZoombiniPuzzleMaze::SolverAcceptedPlanSearch prioritySearch;
								for (int16 snoidIdx = 0; snoidIdx < context.snoidCount; snoidIdx++) {
									prioritySearch.requiredSnoids[snoidIdx] = snoidIdx != precursorKey &&
																			  snoidIdx != victim && snoidIdx != finalKey &&
																			  debugMazePlanMatchesTrait(
																				  context, snoidIdx, traitCell.traitCategory,
																				  traitCell.traitValue, priorityMatched);
								}
								prioritySearch.finalSnoid = finalKey;
								prioritySearch.finalSwitchGroup = finalGroup;
								prioritySearch.actions.push_back(precursorAction);
								prioritySearch.actions.push_back(victimAction);
								if (!debugFindMazeAcceptedPlan(context, victimState, prioritySearch, budget))
									continue;
								const int16 finalActionIdx = static_cast<int16>(prioritySearch.actions.size()) - 1;

								ZoombiniPuzzleMaze::SolverAcceptedPlanSearch remainderSearch;
								for (int16 snoidIdx = 0; snoidIdx < context.snoidCount; snoidIdx++) {
									remainderSearch.requiredSnoids[snoidIdx] = !debugMazePlanMatchesTrait(
										context, snoidIdx, traitCell.traitCategory,
										traitCell.traitValue, priorityMatched);
								}
								remainderSearch.actions = prioritySearch.actions;
								if (!debugFindMazeAcceptedPlan(context, prioritySearch.resultState, remainderSearch, budget) ||
									!debugMazePlanAllAccepted(context, remainderSearch.resultState))
									continue;

								plan.valid = true;
								plan.hasSwitchPhase = true;
								plan.priorityCategory = traitCell.traitCategory;
								plan.priorityValue = traitCell.traitValue;
								plan.priorityMatched = priorityMatched;
								plan.precursorInPriorityGroup = precursorInPriorityGroup;
								plan.priorityCount = priorityCount;
								plan.remainderCount = context.snoidCount - priorityCount - (precursorInPriorityGroup ? 0 : 1);
								plan.precursorKey = precursorKey;
								plan.victim = victim;
								plan.finalKey = finalKey;
								plan.precursorGroup = precursorGroup;
								plan.finalGroup = finalGroup;
								plan.victimActionIdx = 1;
								plan.finalActionIdx = finalActionIdx;
								plan.actions = remainderSearch.actions;
								return true;
							}
						}
					}
				}
			}
		}
	}
	return false;
}

bool ZoombiniPuzzleMaze::debugBuildMazeLevel123Plan(const ZoombiniPuzzleMaze::SolverPlanContext &context,
													ZoombiniPuzzleMaze::SolverLevel123Plan &plan,
													ZoombiniPuzzleMaze::SolverBudget &budget) {
	bool hasBranchSwitch = false;
	for (int16 cellIdx = 0; cellIdx < context.cellCount && !hasBranchSwitch; cellIdx++) {
		if (context.cells[cellIdx].type != kMazeCellType06_ColoredSwitch)
			continue;
		for (int16 linkedIdx = 0; linkedIdx < context.cellCount; linkedIdx++) {
			if (context.cells[linkedIdx].waveGroup == context.cells[cellIdx].waveGroup &&
				context.cells[linkedIdx].type == kMazeCellType04_ColoredArrow) {
				hasBranchSwitch = true;
				break;
			}
		}
	}
	if (hasBranchSwitch)
		return debugBuildMazeSwitchPlan(context, plan, budget);
	return debugBuildMazeSimplePlan(context, plan, budget);
}

// =================================================================
// Generic answer-plan search
// =================================================================
//
// The staged level-4 search above matches one authored relay shape. A layout
// whose colored groups are wired differently needs a search that reads the
// layout instead of assuming its shape.
//
// The search branches only on control launches: launches that press a switch,
// stop on a sticky cell, push or release another Zoombini, or move the
// launched Zoombini to another corner. A launch that simply walks to the goal
// changes nothing else, so it can be made at any later moment; the search only
// records that the Zoombini became deliverable and schedules those launches
// while assembling the printed plan.

int16 ZoombiniPuzzleMaze::debugMazeStickySlot(const ZoombiniPuzzleMaze::SolverTables &tables, int16 cellIdx) {
	for (int16 slot = 0; slot < tables.stickyCount; slot++) {
		if (tables.stickyCells[slot] == cellIdx)
			return slot;
	}
	return -1;
}

void ZoombiniPuzzleMaze::debugMazeApplyControlState(const ZoombiniPuzzleMaze::SolverTables &tables,
													int16 stateIdx, ZoombiniPuzzleMaze::SolverDirectionGrid &grid) {
	memcpy(grid.directions, tables.baseDirections, sizeof(grid.directions));
	if (stateIdx < 0 || tables.controlStateCount <= stateIdx)
		return;
	for (int16 cellIdx = 0; cellIdx < tables.controlCellCount; cellIdx++)
		grid.directions[tables.controlRows[cellIdx]][tables.controlCols[cellIdx]] =
			tables.controlStates[stateIdx][cellIdx];
}

int16 ZoombiniPuzzleMaze::debugMazeControlStateIndex(const ZoombiniPuzzleMaze::SolverTables &tables,
													 const ZoombiniPuzzleMaze::SolverDirectionGrid &grid) {
	for (int16 stateIdx = 0; stateIdx < tables.controlStateCount; stateIdx++) {
		bool same = true;
		for (int16 cellIdx = 0; cellIdx < tables.controlCellCount; cellIdx++) {
			if (tables.controlStates[stateIdx][cellIdx] !=
				grid.directions[tables.controlRows[cellIdx]][tables.controlCols[cellIdx]]) {
				same = false;
				break;
			}
		}
		if (same)
			return stateIdx;
	}
	return -1;
}

void ZoombiniPuzzleMaze::debugMazeTraceRoute(const ZoombiniPuzzleMaze::SolverPlanContext &context,
											 const ZoombiniPuzzleMaze::SolverTables &tables,
											 const ZoombiniPuzzleMaze::SolverMovingSnoid &start,
											 ZoombiniPuzzleMaze::SolverDirectionGrid &grid,
											 ZoombiniPuzzleMaze::SolverRouteResult &route) {
	int16 row = start.row;
	int16 col = start.col;
	int16 direction = start.direction;
	route.arriveDirection = direction;
	route.pressedGroups = 0;
	for (int16 step = 0; step < 256; step++) {
		ZmbGridDirection gridDirection;
		if (!decodeMazeGridDirection(direction, gridDirection)) {
			route.terminal = ZoombiniPuzzleMaze::kDebugMazeRouteRejected;
			return;
		}
		const ZmbGridStep gridStep = ZmbGridTraversal::computeStep(
			ZmbGridCell(row, col), gridDirection,
			ZmbGridBounds(0, ZoombiniPuzzleMaze::kDebugMazeRows - 1, 0, ZoombiniPuzzleMaze::kDebugMazeCols - 1));
		if (!gridStep.inBounds) {
			route.terminal = ZoombiniPuzzleMaze::kDebugMazeRouteRejected;
			return;
		}
		row = gridStep.cell.row;
		col = gridStep.cell.col;
		const MazeCellType cellType = context.cellTypes[row][col];
		const int16 cellIdx = context.cellIndices[row][col];
		switch (cellType) {
		case kMazeCellType01_Whirlpool:
			route.arriveDirection = direction;
			route.terminal = ZoombiniPuzzleMaze::kDebugMazeRouteRejected;
			return;
		case kMazeCellType02_TraitArrow:
			if (debugMazeTraitValue(context.snoids[start.snoidIdx]->_trait,
									context.traitCategories[row][col]) ==
				context.traitValues[row][col])
				direction = grid.directions[row][col];
			break;
		case kMazeCellType03_FixedArrow:
		case kMazeCellType04_ColoredArrow:
			direction = grid.directions[row][col];
			if (0 <= cellIdx && context.cells[cellIdx].cycleOnPass)
				grid.directions[row][col] = debugMazeNextDirection(grid.directions[row][col], context.directionFlags[row][col]);
			break;
		case kMazeCellType05_ColoredSticky: {
			const int16 slot = debugMazeStickySlot(tables, cellIdx);
			if (slot < 0) {
				route.terminal = ZoombiniPuzzleMaze::kDebugMazeRouteRejected;
				return;
			}
			route.arriveDirection = direction;
			route.terminal = static_cast<int16>(ZoombiniPuzzleMaze::kDebugMazeRouteStickyBase + slot);
			return;
		}
		case kMazeCellType06_ColoredSwitch: {
			if (cellIdx < 0) {
				route.terminal = ZoombiniPuzzleMaze::kDebugMazeRouteRejected;
				return;
			}
			const int16 waveGroup = context.cells[cellIdx].waveGroup;
			if (1 <= waveGroup && waveGroup <= kMaxWaveGroups)
				route.pressedGroups |= static_cast<uint16>(1 << waveGroup);
			for (int16 linkedIdx = 0; linkedIdx < context.cellCount; linkedIdx++) {
				const MazeCellState &linked = context.cells[linkedIdx];
				if (linked.waveGroup == waveGroup && linked.type == kMazeCellType04_ColoredArrow)
					grid.directions[linked.row][linked.col] = debugMazeNextDirection(grid.directions[linked.row][linked.col], linked.dirFlags);
			}
			break;
		}
		case kMazeCellType20_ExitLowerLeft:
		case kMazeCellType21_ExitUpperLeft:
		case kMazeCellType22_ExitLowerRight:
		case kMazeCellType23_ExitUpperRight:
			route.arriveDirection = direction;
			if (cellType == kMazeCellType23_ExitUpperRight)
				route.terminal = static_cast<int16>(ZoombiniPuzzleMaze::kDebugMazeRouteGoal);
			else
				route.terminal = static_cast<int16>(cellType - kMazeCellType20_ExitLowerLeft);
			return;
		default:
			break;
		}
	}
	route.terminal = ZoombiniPuzzleMaze::kDebugMazeRouteRejected;
}

bool ZoombiniPuzzleMaze::debugMazeBuildSolverTables(const ZoombiniPuzzleMaze::SolverPlanContext &context,
													ZoombiniPuzzleMaze::SolverTables &tables) {
	tables.controlCellCount = 0;
	tables.stickyCount = 0;
	tables.controlStateCount = 0;
	tables.classCount = 0;
	tables.seatCount = 0;
	tables.snoidCount = context.snoidCount;
	if (context.snoidCount <= 0 || ZoombiniPuzzleMaze::kDebugMazeMaxSnoids < context.snoidCount)
		return false;
	for (int16 snoidIdx = 0; snoidIdx < context.snoidCount; snoidIdx++) {
		if (!context.snoids[snoidIdx])
			return false;
	}
	memcpy(tables.baseDirections, context.initialDirections, sizeof(tables.baseDirections));
	for (int16 seatIdx = 0; seatIdx < ZoombiniPuzzleMaze::kDebugMazeSeatCount; seatIdx++) {
		tables.seatCorner[seatIdx] = context.seatCorner[seatIdx];
		if (!context.seatActive[seatIdx])
			continue;
		tables.seats[tables.seatCount] = seatIdx;
		tables.seatCount += 1;
	}
	if (tables.seatCount == 0)
		return false;

	// Sticky cells, plus every arrow a switch or a pass-through cycle can turn.
	for (int16 cellIdx = 0; cellIdx < context.cellCount; cellIdx++) {
		const MazeCellState &cell = context.cells[cellIdx];
		if (cell.type == kMazeCellType05_ColoredSticky) {
			if (ZoombiniPuzzleMaze::kDebugMazeMaxStickyCells <= tables.stickyCount)
				return false;
			tables.stickyCells[tables.stickyCount] = cellIdx;
			tables.stickyRows[tables.stickyCount] = cell.row;
			tables.stickyCols[tables.stickyCount] = cell.col;
			tables.stickyGroups[tables.stickyCount] = cell.waveGroup;
			tables.stickyCount += 1;
			continue;
		}
		if (cell.type != kMazeCellType03_FixedArrow && cell.type != kMazeCellType04_ColoredArrow)
			continue;
		bool switchDriven = false;
		if (cell.type == kMazeCellType04_ColoredArrow) {
			for (int16 otherIdx = 0; otherIdx < context.cellCount; otherIdx++) {
				if (context.cells[otherIdx].type == kMazeCellType06_ColoredSwitch &&
					context.cells[otherIdx].waveGroup == cell.waveGroup) {
					switchDriven = true;
					break;
				}
			}
		}
		if (!switchDriven && !cell.cycleOnPass)
			continue;
		if (ZoombiniPuzzleMaze::kDebugMazeMaxControlCells <= tables.controlCellCount)
			return false;
		tables.controlCellIdx[tables.controlCellCount] = cellIdx;
		tables.controlRows[tables.controlCellCount] = cell.row;
		tables.controlCols[tables.controlCellCount] = cell.col;
		tables.controlCellCount += 1;
	}

	// Every way the arrow set can change: one switch group at a time, and one
	// pass over a self-cycling arrow at a time.
	int16 transitionCells[ZoombiniPuzzleMaze::kDebugMazeMaxControlCells + kMaxWaveGroups][ZoombiniPuzzleMaze::kDebugMazeMaxControlCells];
	int16 transitionSizes[ZoombiniPuzzleMaze::kDebugMazeMaxControlCells + kMaxWaveGroups];
	int16 transitionCount = 0;
	for (int16 waveGroup = 1; waveGroup <= kMaxWaveGroups; waveGroup++) {
		bool hasSwitch = false;
		for (int16 cellIdx = 0; cellIdx < context.cellCount; cellIdx++) {
			if (context.cells[cellIdx].type == kMazeCellType06_ColoredSwitch &&
				context.cells[cellIdx].waveGroup == waveGroup) {
				hasSwitch = true;
				break;
			}
		}
		if (!hasSwitch)
			continue;
		int16 size = 0;
		for (int16 slot = 0; slot < tables.controlCellCount; slot++) {
			const MazeCellState &cell = context.cells[tables.controlCellIdx[slot]];
			if (cell.type == kMazeCellType04_ColoredArrow && cell.waveGroup == waveGroup) {
				transitionCells[transitionCount][size] = slot;
				size += 1;
			}
		}
		if (size == 0)
			continue;
		transitionSizes[transitionCount] = size;
		transitionCount += 1;
	}
	for (int16 slot = 0; slot < tables.controlCellCount; slot++) {
		if (!context.cells[tables.controlCellIdx[slot]].cycleOnPass)
			continue;
		transitionCells[transitionCount][0] = slot;
		transitionSizes[transitionCount] = 1;
		transitionCount += 1;
	}

	tables.controlStateCount = 1;
	for (int16 slot = 0; slot < tables.controlCellCount; slot++)
		tables.controlStates[0][slot] = tables.baseDirections[tables.controlRows[slot]][tables.controlCols[slot]];
	for (int16 stateIdx = 0; stateIdx < tables.controlStateCount; stateIdx++) {
		for (int16 transitionIdx = 0; transitionIdx < transitionCount; transitionIdx++) {
			int16 candidate[ZoombiniPuzzleMaze::kDebugMazeMaxControlCells];
			for (int16 slot = 0; slot < tables.controlCellCount; slot++)
				candidate[slot] = tables.controlStates[stateIdx][slot];
			for (int16 entry = 0; entry < transitionSizes[transitionIdx]; entry++) {
				const int16 slot = transitionCells[transitionIdx][entry];
				candidate[slot] = debugMazeNextDirection(candidate[slot], context.cells[tables.controlCellIdx[slot]].dirFlags);
			}
			bool known = false;
			for (int16 otherIdx = 0; otherIdx < tables.controlStateCount && !known; otherIdx++) {
				bool same = true;
				for (int16 slot = 0; slot < tables.controlCellCount; slot++) {
					if (tables.controlStates[otherIdx][slot] != candidate[slot]) {
						same = false;
						break;
					}
				}
				known = same;
			}
			if (known)
				continue;
			if (ZoombiniPuzzleMaze::kDebugMazeMaxControlStates <= tables.controlStateCount)
				return false;
			for (int16 slot = 0; slot < tables.controlCellCount; slot++)
				tables.controlStates[tables.controlStateCount][slot] = candidate[slot];
			tables.controlStateCount += 1;
		}
	}

	// Zoombinis that answer every trait cell the same way behave identically.
	for (int16 snoidIdx = 0; snoidIdx < context.snoidCount; snoidIdx++) {
		int16 matchedClass = -1;
		for (int16 otherIdx = 0; otherIdx < snoidIdx && matchedClass < 0; otherIdx++) {
			bool same = true;
			for (int16 cellIdx = 0; cellIdx < context.cellCount && same; cellIdx++) {
				const MazeCellState &cell = context.cells[cellIdx];
				if (cell.type != kMazeCellType02_TraitArrow)
					continue;
				const bool firstMatches = debugMazeTraitValue(context.snoids[snoidIdx]->_trait, cell.traitCategory) == cell.traitValue;
				const bool secondMatches = debugMazeTraitValue(context.snoids[otherIdx]->_trait, cell.traitCategory) == cell.traitValue;
				if (firstMatches != secondMatches)
					same = false;
			}
			if (same)
				matchedClass = tables.classOf[otherIdx];
		}
		if (matchedClass < 0) {
			matchedClass = tables.classCount;
			tables.classRepresentative[matchedClass] = snoidIdx;
			tables.classCount += 1;
		}
		tables.classOf[snoidIdx] = matchedClass;
	}

	for (int16 classIdx = 0; classIdx < tables.classCount; classIdx++) {
		const int16 repIdx = tables.classRepresentative[classIdx];
		for (int16 seatSlot = 0; seatSlot < tables.seatCount; seatSlot++) {
			const int16 seatIdx = tables.seats[seatSlot];
			for (int16 stateIdx = 0; stateIdx < tables.controlStateCount; stateIdx++) {
				ZoombiniPuzzleMaze::SolverDirectionGrid grid;
				debugMazeApplyControlState(tables, stateIdx, grid);
				ZoombiniPuzzleMaze::SolverMovingSnoid routeStart;
				routeStart.snoidIdx = repIdx;
				routeStart.row = context.seatRow[seatIdx];
				routeStart.col = context.seatCol[seatIdx];
				routeStart.direction = context.seatDirection[seatIdx];
				// Row-entry launch scripts finish on the launcher cell, so start
				// one cell behind and let the tracer dispatch that cell first.
				if (routeStart.direction == kMazeDirection01_South)
					routeStart.row -= 1;
				else if (routeStart.direction == kMazeDirection03_North)
					routeStart.row += 1;
				ZoombiniPuzzleMaze::SolverRouteResult &route = tables.launchRoutes[classIdx][seatIdx][stateIdx];
				debugMazeTraceRoute(context, tables, routeStart, grid, route);
				const int16 nextState = debugMazeControlStateIndex(tables, grid);
				if (nextState < 0)
					return false;
				route.nextControlState = nextState;
			}
		}
		for (int16 slot = 0; slot < tables.stickyCount; slot++) {
			for (int16 direction = 0; direction < 4; direction++) {
				for (int16 stateIdx = 0; stateIdx < tables.controlStateCount; stateIdx++) {
					ZoombiniPuzzleMaze::SolverDirectionGrid grid;
					debugMazeApplyControlState(tables, stateIdx, grid);
					ZoombiniPuzzleMaze::SolverMovingSnoid routeStart;
					routeStart.snoidIdx = repIdx;
					routeStart.row = tables.stickyRows[slot];
					routeStart.col = tables.stickyCols[slot];
					routeStart.direction = direction;
					ZoombiniPuzzleMaze::SolverRouteResult &route = tables.resumeRoutes[classIdx][slot][direction][stateIdx];
					debugMazeTraceRoute(context, tables, routeStart, grid, route);
					const int16 nextState = debugMazeControlStateIndex(tables, grid);
					if (nextState < 0)
						return false;
					route.nextControlState = nextState;
				}
			}
		}
	}
	return true;
}

void ZoombiniPuzzleMaze::debugMazeSolverNodeToPlanState(const ZoombiniPuzzleMaze::SolverPlanContext &context,
														const ZoombiniPuzzleMaze::SolverTables &tables,
														const ZoombiniPuzzleMaze::SolverSearchNode &node,
														ZoombiniPuzzleMaze::SolverPlanState &state) {
	ZoombiniPuzzleMaze::SolverDirectionGrid grid;
	debugMazeApplyControlState(tables, node.controlState, grid);
	memcpy(state.directions, grid.directions, sizeof(state.directions));
	for (int16 cellIdx = 0; cellIdx < ZoombiniPuzzleMaze::kDebugMazeMaxCells; cellIdx++)
		state.stickyHeldSnoids[cellIdx] = -1;
	for (int16 snoidIdx = 0; snoidIdx < ZoombiniPuzzleMaze::kDebugMazeMaxSnoids; snoidIdx++) {
		ZoombiniPuzzleMaze::SolverPlanSnoidState &snoidState = state.snoids[snoidIdx];
		snoidState.corner = 0;
		snoidState.row = 0;
		snoidState.col = 0;
		snoidState.direction = kMazeDirection00_West;
		snoidState.heldCellIdx = -1;
		snoidState.accepted = false;
	}
	for (int16 snoidIdx = 0; snoidIdx < context.snoidCount; snoidIdx++) {
		ZoombiniPuzzleMaze::SolverPlanSnoidState &snoidState = state.snoids[snoidIdx];
		const int8 spot = node.spot[snoidIdx];
		if (spot == ZoombiniPuzzleMaze::kDebugMazeSpotAccepted) {
			snoidState.accepted = true;
			snoidState.corner = 3;
		} else if (spot == ZoombiniPuzzleMaze::kDebugMazeSpotHeld) {
			snoidState.corner = -1;
		} else {
			snoidState.corner = spot;
		}
	}
	for (int16 slot = 0; slot < tables.stickyCount; slot++) {
		const int8 holder = node.stickyHolder[slot];
		if (holder < 0)
			continue;
		state.stickyHeldSnoids[tables.stickyCells[slot]] = holder;
		ZoombiniPuzzleMaze::SolverPlanSnoidState &snoidState = state.snoids[holder];
		snoidState.heldCellIdx = tables.stickyCells[slot];
		snoidState.row = tables.stickyRows[slot];
		snoidState.col = tables.stickyCols[slot];
		snoidState.direction = node.stickyDirection[slot];
		snoidState.corner = -1;
	}
}

bool ZoombiniPuzzleMaze::debugMazeSolverNodeFromPlanState(const ZoombiniPuzzleMaze::SolverTables &tables,
														  const ZoombiniPuzzleMaze::SolverPlanState &state,
														  ZoombiniPuzzleMaze::SolverSearchNode &node) {
	ZoombiniPuzzleMaze::SolverDirectionGrid grid;
	memcpy(grid.directions, state.directions, sizeof(grid.directions));
	const int16 stateIdx = debugMazeControlStateIndex(tables, grid);
	if (stateIdx < 0)
		return false;
	node.controlState = static_cast<int8>(stateIdx);
	for (int16 slot = 0; slot < ZoombiniPuzzleMaze::kDebugMazeMaxStickyCells; slot++) {
		node.stickyHolder[slot] = -1;
		node.stickyDirection[slot] = 0;
	}
	for (int16 slot = 0; slot < tables.stickyCount; slot++) {
		const int16 held = state.stickyHeldSnoids[tables.stickyCells[slot]];
		if (held < 0)
			continue;
		node.stickyHolder[slot] = static_cast<int8>(held);
		node.stickyDirection[slot] = static_cast<int8>(state.snoids[held].direction);
	}
	for (int16 snoidIdx = 0; snoidIdx < tables.snoidCount; snoidIdx++) {
		const ZoombiniPuzzleMaze::SolverPlanSnoidState &snoidState = state.snoids[snoidIdx];
		if (snoidState.accepted)
			node.spot[snoidIdx] = ZoombiniPuzzleMaze::kDebugMazeSpotAccepted;
		else if (0 <= snoidState.heldCellIdx)
			node.spot[snoidIdx] = ZoombiniPuzzleMaze::kDebugMazeSpotHeld;
		else if (0 <= snoidState.corner && snoidState.corner <= 2)
			node.spot[snoidIdx] = static_cast<int8>(snoidState.corner);
		else
			return false; // A Zoombini was rejected, so this branch cannot win.
	}
	return true;
}

int16 ZoombiniPuzzleMaze::debugMazeSolverPlainSeat(const ZoombiniPuzzleMaze::SolverTables &tables,
												   int16 classIdx, int16 corner, int16 controlState) {
	for (int16 seatSlot = 0; seatSlot < tables.seatCount; seatSlot++) {
		const int16 seatIdx = tables.seats[seatSlot];
		if (tables.seatCorner[seatIdx] != corner)
			continue;
		const ZoombiniPuzzleMaze::SolverRouteResult &route = tables.launchRoutes[classIdx][seatIdx][controlState];
		if (route.terminal == ZoombiniPuzzleMaze::kDebugMazeRouteGoal && route.pressedGroups == 0)
			return seatIdx;
	}
	return -1;
}

void ZoombiniPuzzleMaze::debugMazeSolverUpdateCover(const ZoombiniPuzzleMaze::SolverTables &tables,
													ZoombiniPuzzleMaze::SolverSearchNode &node) {
	for (int16 snoidIdx = 0; snoidIdx < tables.snoidCount; snoidIdx++) {
		if (node.covered & (1u << snoidIdx))
			continue;
		const int8 spot = node.spot[snoidIdx];
		if (spot < 0 || 2 < spot)
			continue;
		if (0 <= debugMazeSolverPlainSeat(tables, tables.classOf[snoidIdx], spot, node.controlState))
			node.covered |= (1u << snoidIdx);
	}
}

int16 ZoombiniPuzzleMaze::debugMazeSolverUncovered(const ZoombiniPuzzleMaze::SolverTables &tables,
												   const ZoombiniPuzzleMaze::SolverSearchNode &node) {
	int16 count = 0;
	for (int16 snoidIdx = 0; snoidIdx < tables.snoidCount; snoidIdx++) {
		if (node.spot[snoidIdx] == ZoombiniPuzzleMaze::kDebugMazeSpotAccepted)
			continue;
		if (node.covered & (1u << snoidIdx))
			continue;
		count += 1;
	}
	return count;
}

uint64 ZoombiniPuzzleMaze::debugMazeSolverNodeKey(const ZoombiniPuzzleMaze::SolverTables &tables,
												  const ZoombiniPuzzleMaze::SolverSearchNode &node) {
	// Zoombinis of one class are interchangeable, so the key counts them by
	// class, place and cover state instead of by pack slot.
	int16 counts[ZoombiniPuzzleMaze::kDebugMazeMaxSnoids][8];
	memset(counts, 0, sizeof(counts));
	for (int16 snoidIdx = 0; snoidIdx < tables.snoidCount; snoidIdx++) {
		const int8 spot = node.spot[snoidIdx];
		if (spot == ZoombiniPuzzleMaze::kDebugMazeSpotHeld)
			continue; // Encoded through the sticky slots below.
		const bool covered = (node.covered & (1u << snoidIdx)) != 0;
		int16 bucket;
		if (spot == ZoombiniPuzzleMaze::kDebugMazeSpotAccepted)
			bucket = 6;
		else {
			bucket = static_cast<int16>(spot) * 2;
			if (covered)
				bucket += 1;
		}
		counts[tables.classOf[snoidIdx]][bucket] += 1;
	}
	uint64 hash = 1469598103934665603ULL;
	hash ^= static_cast<uint64>(node.controlState + 1);
	hash *= 1099511628211ULL;
	for (int16 classIdx = 0; classIdx < tables.classCount; classIdx++) {
		for (int16 bucket = 0; bucket < 8; bucket++) {
			hash ^= static_cast<uint64>(counts[classIdx][bucket] + 1);
			hash *= 1099511628211ULL;
		}
	}
	for (int16 slot = 0; slot < tables.stickyCount; slot++) {
		const int8 holder = node.stickyHolder[slot];
		const int16 holderClass = (0 <= holder) ? tables.classOf[holder] : -1;
		hash ^= static_cast<uint64>(holderClass + 2);
		hash *= 1099511628211ULL;
		hash ^= static_cast<uint64>(node.stickyDirection[slot] + 1);
		hash *= 1099511628211ULL;
	}
	return hash;
}

bool ZoombiniPuzzleMaze::debugMazeSolverAddState(ZoombiniPuzzleMaze::SolverReachSet &reach, int16 stateIdx) {
	if (stateIdx < 0 || ZoombiniPuzzleMaze::kDebugMazeMaxControlStates <= stateIdx || reach.stateReached[stateIdx])
		return false;
	reach.stateReached[stateIdx] = true;
	reach.stateList[reach.stateCount] = stateIdx;
	reach.stateCount += 1;
	return true;
}

void ZoombiniPuzzleMaze::debugMazeSolverExpandReach(const ZoombiniPuzzleMaze::SolverTables &tables,
													ZoombiniPuzzleMaze::SolverReachSet &reach) {
	// Relaxed closure: assume every Zoombini that can stand somewhere may be
	// launched from there any number of times. The result is a superset of the
	// control states and corners the puzzle can still reach.
	bool changed = true;
	while (changed) {
		changed = false;
		for (int16 stateSlot = 0; stateSlot < reach.stateCount; stateSlot++) {
			const int16 stateIdx = reach.stateList[stateSlot];
			for (int16 classIdx = 0; classIdx < tables.classCount; classIdx++) {
				for (int16 corner = 0; corner < 3; corner++) {
					if (!reach.available[classIdx][corner])
						continue;
					for (int16 seatSlot = 0; seatSlot < tables.seatCount; seatSlot++) {
						const int16 seatIdx = tables.seats[seatSlot];
						if (tables.seatCorner[seatIdx] != corner)
							continue;
						const ZoombiniPuzzleMaze::SolverRouteResult &route = tables.launchRoutes[classIdx][seatIdx][stateIdx];
						if (route.terminal == ZoombiniPuzzleMaze::kDebugMazeRouteRejected)
							continue;
						changed |= debugMazeSolverAddState(reach, route.nextControlState);
						if (0 <= route.terminal && route.terminal <= 2 &&
							!reach.available[classIdx][route.terminal]) {
							reach.available[classIdx][route.terminal] = true;
							changed = true;
						}
						if (ZoombiniPuzzleMaze::kDebugMazeRouteStickyBase <= route.terminal) {
							const int16 slot = route.terminal - ZoombiniPuzzleMaze::kDebugMazeRouteStickyBase;
							if (!reach.occupant[slot][classIdx][route.arriveDirection]) {
								reach.occupant[slot][classIdx][route.arriveDirection] = true;
								changed = true;
							}
							// Whoever waited there leaves along the pusher's heading.
							if (!reach.exitDirection[slot][route.arriveDirection]) {
								reach.exitDirection[slot][route.arriveDirection] = true;
								changed = true;
							}
						}
						for (int16 slot = 0; slot < tables.stickyCount; slot++) {
							const int16 waveGroup = tables.stickyGroups[slot];
							if (waveGroup < 1 || kMaxWaveGroups < waveGroup)
								continue;
							if ((route.pressedGroups & (1 << waveGroup)) == 0)
								continue;
							for (int16 occClass = 0; occClass < tables.classCount; occClass++) {
								for (int16 dir = 0; dir < 4; dir++) {
									if (!reach.occupant[slot][occClass][dir] ||
										reach.exitDirection[slot][dir])
										continue;
									reach.exitDirection[slot][dir] = true;
									changed = true;
								}
							}
						}
					}
				}
			}
			for (int16 slot = 0; slot < tables.stickyCount; slot++) {
				for (int16 occClass = 0; occClass < tables.classCount; occClass++) {
					bool present = false;
					for (int16 dir = 0; dir < 4 && !present; dir++)
						present = reach.occupant[slot][occClass][dir];
					if (!present)
						continue;
					for (int16 exitDir = 0; exitDir < 4; exitDir++) {
						if (!reach.exitDirection[slot][exitDir])
							continue;
						const ZoombiniPuzzleMaze::SolverRouteResult &route = tables.resumeRoutes[occClass][slot][exitDir][stateIdx];
						if (route.terminal == ZoombiniPuzzleMaze::kDebugMazeRouteRejected)
							continue;
						changed |= debugMazeSolverAddState(reach, route.nextControlState);
						if (0 <= route.terminal && route.terminal <= 2 &&
							!reach.available[occClass][route.terminal]) {
							reach.available[occClass][route.terminal] = true;
							changed = true;
						}
						if (ZoombiniPuzzleMaze::kDebugMazeRouteStickyBase <= route.terminal) {
							const int16 target = route.terminal - ZoombiniPuzzleMaze::kDebugMazeRouteStickyBase;
							if (!reach.occupant[target][occClass][route.arriveDirection]) {
								reach.occupant[target][occClass][route.arriveDirection] = true;
								changed = true;
							}
							if (!reach.exitDirection[target][route.arriveDirection]) {
								reach.exitDirection[target][route.arriveDirection] = true;
								changed = true;
							}
						}
					}
				}
			}
		}
	}
}

bool ZoombiniPuzzleMaze::debugMazeSolverGoalReachable(const ZoombiniPuzzleMaze::SolverTables &tables,
													  const ZoombiniPuzzleMaze::SolverSearchNode &node) {
	ZoombiniPuzzleMaze::SolverReachSet reach;
	memset(&reach, 0, sizeof(reach));
	debugMazeSolverAddState(reach, node.controlState);
	for (int16 snoidIdx = 0; snoidIdx < tables.snoidCount; snoidIdx++) {
		const int8 spot = node.spot[snoidIdx];
		if (0 <= spot && spot <= 2)
			reach.available[tables.classOf[snoidIdx]][spot] = true;
	}
	for (int16 slot = 0; slot < tables.stickyCount; slot++) {
		const int8 holder = node.stickyHolder[slot];
		if (0 <= holder)
			reach.occupant[slot][tables.classOf[holder]][node.stickyDirection[slot]] = true;
	}
	debugMazeSolverExpandReach(tables, reach);

	for (int16 snoidIdx = 0; snoidIdx < tables.snoidCount; snoidIdx++) {
		if (node.spot[snoidIdx] == ZoombiniPuzzleMaze::kDebugMazeSpotAccepted || (node.covered & (1u << snoidIdx)))
			continue;
		const int16 classIdx = tables.classOf[snoidIdx];
		bool reachable = false;
		if (node.spot[snoidIdx] == ZoombiniPuzzleMaze::kDebugMazeSpotHeld) {
			int16 startSlot = -1;
			for (int16 slot = 0; slot < tables.stickyCount && startSlot < 0; slot++) {
				if (node.stickyHolder[slot] == snoidIdx)
					startSlot = slot;
			}
			if (startSlot < 0)
				return false;
			bool seen[ZoombiniPuzzleMaze::kDebugMazeMaxStickyCells][4];
			memset(seen, 0, sizeof(seen));
			int16 frontierSlot[ZoombiniPuzzleMaze::kDebugMazeMaxStickyCells * 4];
			int16 frontierDir[ZoombiniPuzzleMaze::kDebugMazeMaxStickyCells * 4];
			int16 frontierCount = 0;
			for (int16 dir = 0; dir < 4; dir++) {
				if (!reach.exitDirection[startSlot][dir])
					continue;
				seen[startSlot][dir] = true;
				frontierSlot[frontierCount] = startSlot;
				frontierDir[frontierCount] = dir;
				frontierCount += 1;
			}
			while (0 < frontierCount && !reachable) {
				frontierCount -= 1;
				const int16 slot = frontierSlot[frontierCount];
				const int16 dir = frontierDir[frontierCount];
				for (int16 stateSlot = 0; stateSlot < reach.stateCount && !reachable; stateSlot++) {
					const int16 stateIdx = reach.stateList[stateSlot];
					const ZoombiniPuzzleMaze::SolverRouteResult &route = tables.resumeRoutes[classIdx][slot][dir][stateIdx];
					if (route.terminal == ZoombiniPuzzleMaze::kDebugMazeRouteGoal) {
						reachable = true;
						break;
					}
					if (0 <= route.terminal && route.terminal <= 2) {
						for (int16 seatSlot = 0; seatSlot < tables.seatCount; seatSlot++) {
							const int16 seatIdx = tables.seats[seatSlot];
							if (tables.seatCorner[seatIdx] != route.terminal)
								continue;
							if (tables.launchRoutes[classIdx][seatIdx][stateIdx].terminal ==
								ZoombiniPuzzleMaze::kDebugMazeRouteGoal) {
								reachable = true;
								break;
							}
						}
					}
					if (ZoombiniPuzzleMaze::kDebugMazeRouteStickyBase <= route.terminal) {
						const int16 target = route.terminal - ZoombiniPuzzleMaze::kDebugMazeRouteStickyBase;
						for (int16 nextDir = 0; nextDir < 4; nextDir++) {
							if (!reach.exitDirection[target][nextDir] || seen[target][nextDir])
								continue;
							seen[target][nextDir] = true;
							frontierSlot[frontierCount] = target;
							frontierDir[frontierCount] = nextDir;
							frontierCount += 1;
						}
					}
				}
			}
		} else {
			bool seenCorner[3] = {false, false, false};
			int16 frontier[3];
			int16 frontierCount = 0;
			seenCorner[node.spot[snoidIdx]] = true;
			frontier[0] = node.spot[snoidIdx];
			frontierCount = 1;
			while (0 < frontierCount && !reachable) {
				frontierCount -= 1;
				const int16 corner = frontier[frontierCount];
				for (int16 stateSlot = 0; stateSlot < reach.stateCount && !reachable; stateSlot++) {
					const int16 stateIdx = reach.stateList[stateSlot];
					for (int16 seatSlot = 0; seatSlot < tables.seatCount; seatSlot++) {
						const int16 seatIdx = tables.seats[seatSlot];
						if (tables.seatCorner[seatIdx] != corner)
							continue;
						const ZoombiniPuzzleMaze::SolverRouteResult &route = tables.launchRoutes[classIdx][seatIdx][stateIdx];
						if (route.terminal == ZoombiniPuzzleMaze::kDebugMazeRouteGoal) {
							reachable = true;
							break;
						}
						if (ZoombiniPuzzleMaze::kDebugMazeRouteStickyBase <= route.terminal) {
							const int16 target = route.terminal - ZoombiniPuzzleMaze::kDebugMazeRouteStickyBase;
							for (int16 dir = 0; dir < 4; dir++) {
								if (reach.exitDirection[target][dir]) {
									reachable = true;
									break;
								}
							}
							if (reachable)
								break;
						}
						if (0 <= route.terminal && route.terminal <= 2 && !seenCorner[route.terminal]) {
							seenCorner[route.terminal] = true;
							frontier[frontierCount] = route.terminal;
							frontierCount += 1;
						}
					}
				}
			}
		}
		if (!reachable)
			return false;
	}
	return true;
}

bool ZoombiniPuzzleMaze::debugMazeGenericPlanFromKeys(const ZoombiniPuzzleMaze::SolverPlanContext &context,
													  const ZoombiniPuzzleMaze::SolverTables &tables,
													  const Common::Array<ZoombiniPuzzleMaze::SolverKeyLaunch> &keyLaunches,
													  ZoombiniPuzzleMaze::SolverGenericPlan &plan) {
	// Replay the control launches once to learn where each remaining Zoombini
	// first gains a side-effect-free route, then emit those launches in place.
	const int16 keyCount = static_cast<int16>(keyLaunches.size());
	int16 coverPosition[ZoombiniPuzzleMaze::kDebugMazeMaxSnoids];
	int16 coverSeat[ZoombiniPuzzleMaze::kDebugMazeMaxSnoids];
	for (int16 snoidIdx = 0; snoidIdx < ZoombiniPuzzleMaze::kDebugMazeMaxSnoids; snoidIdx++) {
		coverPosition[snoidIdx] = -1;
		coverSeat[snoidIdx] = -1;
	}

	ZoombiniPuzzleMaze::SolverBudget budget;
	ZoombiniPuzzleMaze::SolverPlanState state;
	debugMazeInitializePlanState(context, state);
	ZoombiniPuzzleMaze::SolverSearchNode node;
	memset(&node, 0, sizeof(node));
	node.parent = -1;
	if (!debugMazeSolverNodeFromPlanState(tables, state, node))
		return false;
	for (int16 position = 0; position <= keyCount; position++) {
		for (int16 snoidIdx = 0; snoidIdx < tables.snoidCount; snoidIdx++) {
			if (0 <= coverPosition[snoidIdx] || node.spot[snoidIdx] < 0 || 2 < node.spot[snoidIdx])
				continue;
			const int16 seatIdx = debugMazeSolverPlainSeat(tables, tables.classOf[snoidIdx], node.spot[snoidIdx], node.controlState);
			if (seatIdx < 0)
				continue;
			coverPosition[snoidIdx] = position;
			coverSeat[snoidIdx] = seatIdx;
		}
		if (keyCount <= position)
			break;
		const ZoombiniPuzzleMaze::SolverKeyLaunch &keyLaunch = keyLaunches[position];
		ZoombiniPuzzleMaze::SolverPlanAction action;
		if (!debugMazeApplyPlanLaunch(context, state, keyLaunch.snoidIdx, keyLaunch.launcherSeatIdx, action, budget))
			return false;
		ZoombiniPuzzleMaze::SolverSearchNode nextNode = node;
		if (!debugMazeSolverNodeFromPlanState(tables, state, nextNode))
			return false;
		// A Zoombini that moved, or that a control launch just consumed, must
		// find its plain route again after this launch.
		for (int16 snoidIdx = 0; snoidIdx < tables.snoidCount; snoidIdx++) {
			if (nextNode.spot[snoidIdx] != node.spot[snoidIdx] || snoidIdx == keyLaunch.snoidIdx)
				coverPosition[snoidIdx] = -1;
		}
		node = nextNode;
	}

	// Assemble and verify the launch order on a fresh state.
	plan.steps.clear();
	plan.postSpots.clear();
	plan.controlStepCount = keyCount;
	ZoombiniPuzzleMaze::SolverPlanState replay;
	debugMazeInitializePlanState(context, replay);
	ZoombiniPuzzleMaze::SolverBudget replayBudget;
	for (int16 position = 0; position <= keyCount; position++) {
		for (int16 snoidIdx = 0; snoidIdx < tables.snoidCount; snoidIdx++) {
			if (coverPosition[snoidIdx] != position || replay.snoids[snoidIdx].accepted)
				continue;
			ZoombiniPuzzleMaze::SolverGenericStep step;
			step.snoidIdx = snoidIdx;
			step.launcherSeatIdx = coverSeat[snoidIdx];
			step.controlStep = false;
			ZoombiniPuzzleMaze::SolverPlanAction action;
			if (!debugMazeApplyPlanLaunch(context, replay, snoidIdx, coverSeat[snoidIdx], action,
										  replayBudget))
				return false;
			if (!replay.snoids[snoidIdx].accepted || !action.switchGroups.empty() ||
				!action.pushedSnoids.empty() || !action.releasedSnoids.empty())
				return false;
			step.outcome = action.rootOutcome;
			plan.steps.push_back(step);
			plan.postSpots.push_back(ZoombiniPuzzleMaze::kDebugMazeSpotAccepted);
		}
		if (keyCount <= position)
			break;
		const ZoombiniPuzzleMaze::SolverKeyLaunch &keyLaunch = keyLaunches[position];
		ZoombiniPuzzleMaze::SolverGenericStep step;
		step.snoidIdx = keyLaunch.snoidIdx;
		step.launcherSeatIdx = keyLaunch.launcherSeatIdx;
		step.controlStep = true;
		ZoombiniPuzzleMaze::SolverPlanAction action;
		if (!debugMazeApplyPlanLaunch(context, replay, step.snoidIdx, step.launcherSeatIdx, action,
									  replayBudget))
			return false;
		step.outcome = action.rootOutcome;
		step.switchGroups = action.switchGroups;
		step.pushedSnoids = action.pushedSnoids;
		step.releasedSnoids = action.releasedSnoids;
		step.stickyCellIdx = replay.snoids[step.snoidIdx].heldCellIdx;
		plan.steps.push_back(step);
		const ZoombiniPuzzleMaze::SolverPlanSnoidState &launched = replay.snoids[step.snoidIdx];
		if (launched.accepted)
			plan.postSpots.push_back(ZoombiniPuzzleMaze::kDebugMazeSpotAccepted);
		else if (0 <= launched.heldCellIdx)
			plan.postSpots.push_back(ZoombiniPuzzleMaze::kDebugMazeSpotHeld);
		else
			plan.postSpots.push_back(launched.corner);
	}
	if (!debugMazePlanAllAccepted(context, replay))
		return false;
	plan.valid = true;
	return true;
}

bool ZoombiniPuzzleMaze::debugBuildMazeGenericPlan(const ZoombiniPuzzleMaze::SolverPlanContext &context,
												   ZoombiniPuzzleMaze::SolverGenericPlan &plan,
												   const ZoombiniPuzzleMaze::SolverGenericSearchLimits &limits,
												   ZoombiniPuzzleMaze::SolverGenericSearchResult &result) {
	result.searchExhausted = false;
	result.limitReached = false;
	result.stepsUsed = 0;
	ZoombiniPuzzleMaze::SolverTables *tables = new ZoombiniPuzzleMaze::SolverTables();
	if (!tables)
		return false;
	if (!debugMazeBuildSolverTables(context, *tables)) {
		delete tables;
		return false;
	}

	Common::Array<ZoombiniPuzzleMaze::SolverSearchNode> nodes;
	Common::HashMap<uint64, bool> visited;
	Common::Array<uint32> buckets[ZoombiniPuzzleMaze::kDebugMazeMaxSnoids + 1];
	uint32 bucketHead[ZoombiniPuzzleMaze::kDebugMazeMaxSnoids + 1];
	for (int16 bucketIdx = 0; bucketIdx <= ZoombiniPuzzleMaze::kDebugMazeMaxSnoids; bucketIdx++)
		bucketHead[bucketIdx] = 0;

	ZoombiniPuzzleMaze::SolverPlanState rootState;
	debugMazeInitializePlanState(context, rootState);
	ZoombiniPuzzleMaze::SolverSearchNode root;
	memset(&root, 0, sizeof(root));
	root.parent = -1;
	if (!debugMazeSolverNodeFromPlanState(*tables, rootState, root)) {
		delete tables;
		return false;
	}
	root.keySnoid = -1;
	root.keySeat = -1;
	debugMazeSolverUpdateCover(*tables, root);
	visited[debugMazeSolverNodeKey(*tables, root)] = true;
	nodes.push_back(root);
	buckets[debugMazeSolverUncovered(*tables, root)].push_back(0);

	uint32 steps = 0;
	int32 goalNode = -1;
	while (goalNode < 0 && !result.limitReached) {
		int32 currentIdx = -1;
		for (int16 bucketIdx = 0; bucketIdx <= ZoombiniPuzzleMaze::kDebugMazeMaxSnoids; bucketIdx++) {
			if (bucketHead[bucketIdx] < buckets[bucketIdx].size()) {
				currentIdx = static_cast<int32>(buckets[bucketIdx][bucketHead[bucketIdx]]);
				bucketHead[bucketIdx] += 1;
				break;
			}
		}
		if (currentIdx < 0) {
			result.searchExhausted = true;
			break;
		}
		const ZoombiniPuzzleMaze::SolverSearchNode current = nodes[currentIdx];
		if (debugMazeSolverUncovered(*tables, current) == 0) {
			goalNode = currentIdx;
			break;
		}
		if (ZoombiniPuzzleMaze::kDebugMazeMaxControlSteps <= current.depth)
			continue;

		bool tried[ZoombiniPuzzleMaze::kDebugMazeMaxSnoids][3][ZoombiniPuzzleMaze::kDebugMazeSeatCount];
		memset(tried, 0, sizeof(tried));
		for (int16 snoidIdx = 0; snoidIdx < tables->snoidCount && !result.limitReached; snoidIdx++) {
			const int8 spot = current.spot[snoidIdx];
			if (spot < 0 || 2 < spot)
				continue;
			const int16 classIdx = tables->classOf[snoidIdx];
			for (int16 seatSlot = 0; seatSlot < tables->seatCount; seatSlot++) {
				const int16 seatIdx = tables->seats[seatSlot];
				if (tables->seatCorner[seatIdx] != spot || tried[classIdx][spot][seatIdx])
					continue;
				tried[classIdx][spot][seatIdx] = true;
				const ZoombiniPuzzleMaze::SolverRouteResult &route = tables->launchRoutes[classIdx][seatIdx][current.controlState];
				if (route.terminal == ZoombiniPuzzleMaze::kDebugMazeRouteRejected)
					continue;
				if (route.terminal == ZoombiniPuzzleMaze::kDebugMazeRouteGoal && route.pressedGroups == 0)
					continue; // A plain delivery is scheduled later instead.

				steps += 1;
				result.stepsUsed = steps;
				if (limits.maxSteps <= steps) {
					result.limitReached = true;
					break;
				}
				ZoombiniPuzzleMaze::SolverPlanState childState;
				debugMazeSolverNodeToPlanState(context, *tables, current, childState);
				ZoombiniPuzzleMaze::SolverPlanAction action;
				ZoombiniPuzzleMaze::SolverBudget budget;
				if (!debugMazeApplyPlanLaunch(context, childState, snoidIdx, seatIdx, action, budget))
					continue;
				ZoombiniPuzzleMaze::SolverSearchNode child = current;
				if (!debugMazeSolverNodeFromPlanState(*tables, childState, child))
					continue;
				child.parent = currentIdx;
				child.keySnoid = static_cast<int8>(snoidIdx);
				child.keySeat = static_cast<int8>(seatIdx);
				child.depth = static_cast<int8>(current.depth + 1);
				for (int16 otherIdx = 0; otherIdx < tables->snoidCount; otherIdx++) {
					if (child.spot[otherIdx] != current.spot[otherIdx] || otherIdx == snoidIdx)
						child.covered &= ~(1u << otherIdx);
				}
				debugMazeSolverUpdateCover(*tables, child);
				const uint64 key = debugMazeSolverNodeKey(*tables, child);
				if (visited.contains(key))
					continue;
				visited[key] = true;
				if (!debugMazeSolverGoalReachable(*tables, child))
					continue;
				if (limits.maxNodes <= nodes.size()) {
					result.limitReached = true;
					break;
				}
				nodes.push_back(child);
				buckets[debugMazeSolverUncovered(*tables, child)].push_back(static_cast<uint32>(nodes.size() - 1));
			}
		}
	}

	bool built = false;
	if (0 <= goalNode) {
		Common::Array<ZoombiniPuzzleMaze::SolverKeyLaunch> keyLaunches;
		for (int32 walkIdx = goalNode; 0 <= walkIdx; walkIdx = nodes[walkIdx].parent) {
			if (nodes[walkIdx].parent < 0)
				break;
			ZoombiniPuzzleMaze::SolverKeyLaunch keyLaunch;
			keyLaunch.snoidIdx = nodes[walkIdx].keySnoid;
			keyLaunch.launcherSeatIdx = nodes[walkIdx].keySeat;
			keyLaunches.push_back(keyLaunch);
		}
		for (uint frontIdx = 0; frontIdx < keyLaunches.size() / 2; frontIdx++) {
			SWAP(keyLaunches[frontIdx], keyLaunches[keyLaunches.size() - 1 - frontIdx]);
		}
		built = debugMazeGenericPlanFromKeys(context, *tables, keyLaunches, plan);
		if (!built) {
			// The replay rejected the assembled order, so the search proved nothing.
			result.searchExhausted = false;
			result.limitReached = false;
		}
	}
	delete tables;
	return built;
}

bool ZoombiniPuzzleMaze::isMazeLayoutSolvable(uint32 &stepsUsed) const {
	ZoombiniPuzzleMaze::SolverPlanContext context = {};
	debugBuildMazePlanContext(context);
	ZoombiniPuzzleMaze::SolverGenericPlan plan;
	const ZoombiniPuzzleMaze::SolverGenericSearchLimits limits = {
		kMazeRepairProbeMaxSteps,
		kMazeRepairProbeMaxNodes};
	ZoombiniPuzzleMaze::SolverGenericSearchResult result;
	const bool solvable = debugBuildMazeGenericPlan(context, plan, limits, result);
	stepsUsed = result.stepsUsed;
	return solvable;
}

void ZoombiniPuzzleMaze::setConditionCellTrait(int16 cellIdx, int16 category, int16 value) {
	if (cellIdx < 0 || _cellStateCount <= cellIdx)
		return;
	MazeCellState &cell = _cellStates[cellIdx];
	cell.traitCategory = category;
	cell.traitValue = value;
	// The runtime router reads the cell, the answer search reads the grid tables,
	// and the cell renderer picks the drawn condition shape from the same pair.
	_cellTraitKind[cell.row][cell.col] = category;
	_cellTraitValue[cell.row][cell.col] = value;
}

bool ZoombiniPuzzleMaze::repairRestoredLayoutConditionCell() {
	// The restored level-4 layout REGS 16607 is content the shipped game never
	// selects, so its pack generator was never tuned against it. Its device
	// chain is strict: the only route to the goal opens through the group-4
	// switch, whose presser is stranded on a sticky cell that kills it on a
	// plain release, and the only rescue is a push that needs the linked branch
	// turned and turned back. A pack that lacks the Zoombinis those switches
	// need therefore cannot be completed at all, whatever the player does.
	//
	// The pack arrives from the previous page and must not be touched, so the
	// repair changes one condition cell instead: it keeps the trait category
	// the cell tests and only moves the value it looks for, which is the
	// smallest edit that can hand the device the keys it is missing.
	if (_layoutLevel != kMazeLayoutLevel4 || _mazeLayoutRegsId != kResRegs16607_MazeLayoutL4Unused)
		return false;

	uint32 probeSteps = 0;
	if (isMazeLayoutSolvable(probeSteps))
		return false;

	uint32 spentSteps = probeSteps;
	for (int16 sameCategoryPass = 1; 0 <= sameCategoryPass; sameCategoryPass -= 1) {
		for (int16 cellIdx = 0; cellIdx < _cellStateCount; cellIdx++) {
			MazeCellState &cell = _cellStates[cellIdx];
			if (cell.type != kMazeCellType02_TraitArrow || cell.traitCategory < 1 || 4 < cell.traitCategory)
				continue;
			const int16 authoredCategory = cell.traitCategory;
			const int16 authoredValue = cell.traitValue;
			for (int16 slot = 1; slot <= 20; slot++) {
				const int16 category = static_cast<int16>(kTraitSlotKind[slot]) + 1;
				const int16 value = kTraitSlotValue[slot];
				if (category == authoredCategory && value == authoredValue)
					continue;
				if ((category == authoredCategory) != (sameCategoryPass == 1))
					continue;
				if (kMazeRepairTotalMaxSteps <= spentSteps) {
					warning("maze: REGS %d condition repair gave up after %u simulated launches", _mazeLayoutRegsId, static_cast<uint>(spentSteps));
					return false;
				}
				setConditionCellTrait(cellIdx, category, value);
				uint32 candidateSteps = 0;
				if (isMazeLayoutSolvable(candidateSteps)) {
					MazeConditionSubstitution substitution;
					substitution.cellIdx = cellIdx;
					substitution.fromCategory = authoredCategory;
					substitution.fromValue = authoredValue;
					_conditionSubstitutions.push_back(substitution);
					logConditionSubstitutions(spentSteps + candidateSteps);
					return true;
				}
				spentSteps += candidateSteps;
			}
			setConditionCellTrait(cellIdx, authoredCategory, authoredValue);
		}
	}
	warning("maze: REGS %d has no single-condition repair for this pack; at least one Zoombini cannot reach the goal", _mazeLayoutRegsId);
	return false;
}

Common::String ZoombiniPuzzleMaze::describeConditionSubstitution(
	const MazeConditionSubstitution &substitution) const {
	if (substitution.cellIdx < 0 || _cellStateCount <= substitution.cellIdx)
		return "an unknown condition cell";
	const MazeCellState &cell = _cellStates[substitution.cellIdx];
	const ZmbTrait::TraitKind cellKind = ZmbTrait::traitKindFromIndex(cell.traitCategory - 1);
	const ZmbTrait::TraitKind sourceKind = ZmbTrait::traitKindFromIndex(substitution.fromCategory - 1);
	return Common::String::format(
		"cell (%d, %d) now tests %s %s instead of %s %s",
		cell.row + 1, cell.col + 1,
		ZmbTrait::debugTraitKindName(cellKind),
		ZmbTrait::debugTraitValueName(cellKind, cell.traitValue),
		ZmbTrait::debugTraitKindName(sourceKind),
		ZmbTrait::debugTraitValueName(sourceKind, substitution.fromValue));
}

void ZoombiniPuzzleMaze::logConditionSubstitutions(uint32 stepsUsed) const {
	if (_conditionSubstitutions.empty())
		return;

	warning("maze: REGS %d cannot deliver the arriving pack of %d Zoombinis as authored", _mazeLayoutRegsId, _pageLoadedZmbCount);
	for (uint substitutionIdx = 0; substitutionIdx < _conditionSubstitutions.size(); substitutionIdx++)
		warning("maze: substituted condition %s", describeConditionSubstitution(_conditionSubstitutions[substitutionIdx]).c_str());
	warning("maze: all %d Zoombinis can reach the goal after %s (%u simulated launches)",
			_pageLoadedZmbCount,
			_conditionSubstitutions.size() == 1 ? "that substitution" : "those substitutions",
			static_cast<uint>(stepsUsed));
}

int16 ZoombiniPuzzleMaze::debugMazePlanGroupSwitchCell(const ZoombiniPuzzleMaze::SolverPlanContext &context, int16 waveGroup) {
	for (int16 cellIdx = 0; cellIdx < context.cellCount; cellIdx++) {
		if (context.cells[cellIdx].type == kMazeCellType06_ColoredSwitch && context.cells[cellIdx].waveGroup == waveGroup)
			return cellIdx;
	}
	return -1;
}

const char *ZoombiniPuzzleMaze::debugMazePlanGroupColor(const ZoombiniPuzzleMaze::SolverPlanContext &context,
														int16 waveGroup, bool colorBlindMode) {
	const int16 cellIdx = debugMazePlanGroupSwitchCell(context, waveGroup);
	if (cellIdx < 0)
		return "Unknown-color";
	const MazeColorShapeBase shapeBase = static_cast<MazeColorShapeBase>(context.cells[cellIdx].shapeBase);
	if (colorBlindMode && shapeBase == MazeColorShapeBase::kPurple157)
		return "Dark blue";
	return debugMazeColorName(shapeBase);
}

const char *ZoombiniPuzzleMaze::debugMazePlanCornerName(int16 corner) {
	static constexpr const char *kCornerNames[] = {
		"lower-left", "upper-left", "lower-right", "upper-right"};
	return (0 <= corner && corner < 4) ? kCornerNames[corner] : "unknown";
}

int16 ZoombiniPuzzleMaze::debugMazeLiveSnoidSpot(const ZoombiniPuzzleMaze::SolverPlanContext &context,
												 int16 snoidIdx) const {
	if (snoidIdx < 0 || context.snoidCount <= snoidIdx || !context.snoids[snoidIdx])
		return -4;
	const ZmbSnoid *snoid = context.snoids[snoidIdx];
	if (snoid->_packIsOccupied)
		return ZoombiniPuzzleMaze::kDebugMazeSpotAccepted;
	const int16 runnerIdx = findRunnerBySnoidId(snoid->getId());
	if (runnerIdx < 0)
		return 0;
	for (int16 cellIdx = 0; cellIdx < _cellStateCount; cellIdx++) {
		if (_cellStates[cellIdx].heldRunner == runnerIdx)
			return ZoombiniPuzzleMaze::kDebugMazeSpotHeld;
	}
	const MazeCornerGroup cornerGroup = _runnerStates[runnerIdx].cornerGroup;
	switch (cornerGroup) {
	case kLowerLeft00:
	case kUpperLeft01:
	case kLowerRight02:
		return cornerGroup;
	case kUpperRight03:
		return -4;
	}
	return -4;
}

Common::String ZoombiniPuzzleMaze::debugMazeGenericStepText(const ZoombiniPuzzleMaze::SolverPlanContext &context,
															const ZoombiniPuzzleMaze::SolverGenericStep &step,
															bool colorBlindMode) const {
	Common::String text;
	for (uint groupIdx = 0; groupIdx < step.switchGroups.size(); groupIdx++) {
		text += Common::String::format(" It presses the %s switch.", debugMazePlanGroupColor(context, step.switchGroups[groupIdx], colorBlindMode));
	}
	for (uint pushIdx = 0; pushIdx < step.pushedSnoids.size(); pushIdx++) {
		const int16 pushed = step.pushedSnoids[pushIdx];
		if (pushed < 0 || context.snoidCount <= pushed || !context.snoids[pushed])
			continue;
		text += Common::String::format(" It pushes %s onward.", context.snoids[pushed]->toStr(true).c_str());
	}
	for (uint releaseIdx = 0; releaseIdx < step.releasedSnoids.size(); releaseIdx++) {
		const int16 released = step.releasedSnoids[releaseIdx];
		if (released < 0 || context.snoidCount <= released || !context.snoids[released])
			continue;
		text += Common::String::format(" It releases %s.", context.snoids[released]->toStr(true).c_str());
	}
	if (step.outcome == 3) {
		text += " It reaches the completion exit.";
	} else if (step.outcome == -2) {
		if (0 <= step.stickyCellIdx && step.stickyCellIdx < context.cellCount)
			text += Common::String::format(" It waits at sticky cell (%d, %d).", context.cells[step.stickyCellIdx].row + 1, context.cells[step.stickyCellIdx].col + 1);
		else
			text += " It waits at a sticky cell.";
	} else if (0 <= step.outcome && step.outcome <= 2) {
		text += Common::String::format(" It returns to the %s corner.", debugMazePlanCornerName(step.outcome));
	}
	return text;
}

Common::String ZoombiniPuzzleMaze::debugMazeLauncherLabel(const Common::Point (&launcherPositions)[14], int16 seatIdx,
														  bool includeRegion) {
	if (seatIdx < 0 || 14 <= seatIdx)
		return "launcher at an unknown position";

	const Common::Point pos = launcherPositions[seatIdx];
	if (!includeRegion)
		return Common::String::format("launcher at (%d, %d)", pos.x, pos.y);

	const char *vertical = pos.y < 240 ? "upper" : "lower";
	const char *horizontal = pos.x < 320 ? "left" : "right";
	return Common::String::format("%s-%s launcher at (%d, %d)", vertical, horizontal, pos.x, pos.y);
}

Common::String ZoombiniPuzzleMaze::debugMazePlanRouteFilters(const ZoombiniPuzzleMaze::SolverPlanContext &context,
															 const ZoombiniPuzzleMaze::SolverPlanAction &action) {
	Common::String result;
	Common::Array<Common::String> conditions;
	for (uint decisionIdx = 0; decisionIdx < action.traitDecisions.size(); decisionIdx++) {
		const ZoombiniPuzzleMaze::SolverTraitDecision &decision = action.traitDecisions[decisionIdx];
		if (decision.cellIdx < 0 || context.cellCount <= decision.cellIdx)
			continue;
		const MazeCellState &cell = context.cells[decision.cellIdx];
		if (cell.traitCategory < 1 || 4 < cell.traitCategory)
			continue;
		const ZmbTrait::TraitKind kind = ZmbTrait::traitKindFromIndex(cell.traitCategory - 1);
		const Common::String condition = Common::String::format(
			"%s %s %s", ZmbTrait::debugTraitKindName(kind), decision.matched ? "=" : "!=",
			ZmbTrait::debugTraitValueName(kind, cell.traitValue));
		bool duplicate = false;
		for (uint conditionIdx = 0; conditionIdx < conditions.size(); conditionIdx++) {
			if (conditions[conditionIdx] == condition) {
				duplicate = true;
				break;
			}
		}
		if (!duplicate)
			conditions.push_back(condition);
	}
	for (uint conditionIdx = 0; conditionIdx < conditions.size(); conditionIdx++) {
		if (!result.empty())
			result += ", ";
		result += conditions[conditionIdx];
	}
	return result;
}

Common::String ZoombiniPuzzleMaze::debugMazePlanTraitCondition(int16 category, int16 value, bool matched) {
	if (category < 1 || 4 < category)
		return "unknown trait";
	const ZmbTrait::TraitKind kind = ZmbTrait::traitKindFromIndex(category - 1);
	const char *matchOperator;
	if (matched)
		matchOperator = "=";
	else
		matchOperator = "!=";
	return Common::String::format("%s %s %s", ZmbTrait::debugTraitKindName(kind), matchOperator, ZmbTrait::debugTraitValueName(kind, value));
}

Common::String ZoombiniPuzzleMaze::debugMazePlanStagingLabel(const ZoombiniPuzzleMaze::SolverLevel4Plan &plan,
															 int16 stagingKey) {
	for (int16 keyIdx = 0; keyIdx < plan.stagingKeyCount; keyIdx++) {
		if (plan.stagingKeys[keyIdx] == stagingKey) {
			if (plan.stagingKeyCount == 1)
				return "1";
			return Common::String::format("1%c", 'A' + keyIdx);
		}
	}
	return "1?";
}

Common::String ZoombiniPuzzleMaze::debugMazePlanStagingLaunchSequence(const ZoombiniPuzzleMaze::SolverLevel4Plan &plan,
																	  bool highwayPhase,
																	  const Common::Point (&launcherPositions)[14]) {
	Common::String result;
	for (int16 actionIdx = 0; actionIdx < plan.stagingKeyCount; actionIdx++) {
		const ZoombiniPuzzleMaze::SolverPlanAction *action;
		if (highwayPhase)
			action = &plan.stageHighwaySwitchActions[actionIdx];
		else
			action = &plan.stageStagingActions[actionIdx];
		if (!result.empty())
			result += ", ";
		result += Common::String::format("key %s via %s",
										 debugMazePlanStagingLabel(plan, action->snoidIdx).c_str(),
										 debugMazeLauncherLabel(launcherPositions, action->launcherSeatIdx, true).c_str());
	}
	return result;
}

ZoombiniPuzzleMaze::LaunchSimulationOutcome ZoombiniPuzzleMaze::simulateNextLaunch(const ZmbSnoid *snoid, int16 seatIdx,
																				   Common::Array<int16> *switchCells) const {
	if (switchCells)
		switchCells->clear();

	int16 directions[kGridRows][kGridCols];
	memcpy(directions, _nodeDirection, sizeof(directions));
	int16 row = kSeatGridCoords[seatIdx].x;
	int16 col = kSeatGridCoords[seatIdx].y;
	int16 direction = kSeatMoveDirection[seatIdx];
	// Directions 1 and 3 use an in-place launch script. Begin one cell behind
	// so the common step loop dispatches the launcher cell first.
	if (direction == kMazeDirection01_South)
		row -= 1;
	else if (direction == kMazeDirection03_North)
		row += 1;

	for (int16 step = 0; step < 256; step++) {
		const int16 oldRow = row;
		const int16 oldCol = col;
		ZmbGridDirection gridDirection;
		if (!decodeMazeGridDirection(direction, gridDirection))
			return LaunchSimulationOutcome::kInvalidTraversalMinus04;

		const ZmbGridStep gridStep = ZmbGridTraversal::computeStep(ZmbGridCell(row, col), gridDirection, ZmbGridBounds(0, kGridRows - 1, 0, kGridCols - 1));
		if (!gridStep.inBounds)
			return LaunchSimulationOutcome::kInvalidTraversalMinus04;
		row = gridStep.cell.row;
		col = gridStep.cell.col;
		if (row == oldRow && col == oldCol)
			return LaunchSimulationOutcome::kInvalidTraversalMinus04;

		const MazeCellType cellType = _cellTypes[row][col];
		const int16 cellIdx = getCellStateIndex(row, col);
		const MazeCellState *cell = getCellState(cellIdx);
		switch (cellType) {
		case kMazeCellType01_Whirlpool:
			return LaunchSimulationOutcome::kRejectedByWhirlpoolMinus01;
		case kMazeCellType02_TraitArrow:
			if (getTraitByCategory(snoid->_trait, _cellTraitKind[row][col]) == _cellTraitValue[row][col])
				direction = directions[row][col];
			break;
		case kMazeCellType03_FixedArrow:
		case kMazeCellType04_ColoredArrow:
			direction = directions[row][col];
			if (_nodeCycleFlag[row][col]) {
				for (int16 attempts = 0; attempts < 4; attempts++) {
					directions[row][col] += 1;
					if (3 < directions[row][col])
						directions[row][col] = 0;
					if (_nodeDirFlags[row][col][directions[row][col]])
						break;
				}
			}
			break;
		case kMazeCellType05_ColoredSticky:
			return LaunchSimulationOutcome::kHeldAtColoredStickyMinus02;
		case kMazeCellType06_ColoredSwitch:
			if (switchCells && 0 <= cellIdx) {
				bool alreadyRecorded = false;
				for (uint switchIdx = 0; switchIdx < switchCells->size(); switchIdx++) {
					if ((*switchCells)[switchIdx] == cellIdx) {
						alreadyRecorded = true;
						break;
					}
				}
				if (!alreadyRecorded)
					switchCells->push_back(cellIdx);
			}
			if (cell && 1 < cell->waveGroup && cell->waveGroup <= kMaxWaveGroups) {
				const int16 groupIdx = cell->waveGroup - 1;
				for (int16 colIdx = _waveGroupCellCounts[groupIdx] - 1; 0 <= colIdx; colIdx--) {
					const MazeCellState &linked = _cellStates[_waveGroupCellIndices[groupIdx][colIdx]];
					if (linked.type != kMazeCellType04_ColoredArrow)
						continue;
					int16 &linkedDirection = directions[linked.row][linked.col];
					for (int16 attempts = 0; attempts < 4; attempts++) {
						linkedDirection += 1;
						if (3 < linkedDirection)
							linkedDirection = 0;
						if (linked.dirFlags[linkedDirection])
							break;
					}
				}
			}
			break;
		case kMazeCellType20_ExitLowerLeft:
			return LaunchSimulationOutcome::kExitLowerLeft00;
		case kMazeCellType21_ExitUpperLeft:
			return LaunchSimulationOutcome::kExitUpperLeft01;
		case kMazeCellType22_ExitLowerRight:
			return LaunchSimulationOutcome::kExitLowerRight02;
		case kMazeCellType23_ExitUpperRight:
			return LaunchSimulationOutcome::kExitUpperRight03;
		default:
			break;
		}
	}
	return LaunchSimulationOutcome::kStepLimitExceededMinus03;
}

void ZoombiniPuzzleMaze::debugBuildMazePlanContext(ZoombiniPuzzleMaze::SolverPlanContext &context) const {
	if (_cellStateCount < ZoombiniPuzzleMaze::kDebugMazeMaxCells)
		context.cellCount = _cellStateCount;
	else
		context.cellCount = ZoombiniPuzzleMaze::kDebugMazeMaxCells;
	if (_pageLoadedZmbCount < ZoombiniPuzzleMaze::kDebugMazeMaxSnoids)
		context.snoidCount = _pageLoadedZmbCount;
	else
		context.snoidCount = ZoombiniPuzzleMaze::kDebugMazeMaxSnoids;
	for (int16 rowIdx = 0; rowIdx < ZoombiniPuzzleMaze::kDebugMazeRows; rowIdx++) {
		for (int16 colIdx = 0; colIdx < ZoombiniPuzzleMaze::kDebugMazeCols; colIdx++) {
			context.cellTypes[rowIdx][colIdx] = _cellTypes[rowIdx][colIdx];
			context.cellIndices[rowIdx][colIdx] = -1;
			context.traitCategories[rowIdx][colIdx] = _cellTraitKind[rowIdx][colIdx];
			context.traitValues[rowIdx][colIdx] = _cellTraitValue[rowIdx][colIdx];
			context.initialDirections[rowIdx][colIdx] = _nodeDirection[rowIdx][colIdx];
			for (int16 direction = 0; direction < 4; direction++)
				context.directionFlags[rowIdx][colIdx][direction] = _nodeDirFlags[rowIdx][colIdx][direction];
		}
	}
	for (int16 cellIdx = 0; cellIdx < context.cellCount; cellIdx++) {
		context.cells[cellIdx] = _cellStates[cellIdx];
		const MazeCellState &cell = _cellStates[cellIdx];
		if (0 <= cell.row && cell.row < ZoombiniPuzzleMaze::kDebugMazeRows && 0 <= cell.col && cell.col < ZoombiniPuzzleMaze::kDebugMazeCols) {
			context.cellIndices[cell.row][cell.col] = cellIdx;
			context.initialDirections[cell.row][cell.col] =
				debugMazeInitialDirection(_layoutRegsWords, cellIdx, _nodeDirection[cell.row][cell.col]);
		}
	}
	for (int16 snoidIdx = 0; snoidIdx < context.snoidCount; snoidIdx++)
		context.snoids[snoidIdx] = getSnoid(10000 + snoidIdx);
	for (int16 seatIdx = 0; seatIdx < ZoombiniPuzzleMaze::kDebugMazeSeatCount; seatIdx++) {
		context.seatActive[seatIdx] = _launcherSeatEnabled[seatIdx];
		if (seatIdx < 6)
			context.seatCorner[seatIdx] = 0;
		else if (seatIdx < 9)
			context.seatCorner[seatIdx] = 1;
		else
			context.seatCorner[seatIdx] = 2;
		context.seatRow[seatIdx] = kSeatGridCoords[seatIdx].x;
		context.seatCol[seatIdx] = kSeatGridCoords[seatIdx].y;
		context.seatDirection[seatIdx] = kSeatMoveDirection[seatIdx];
	}
}

Common::String ZoombiniPuzzleMaze::debugGetAnswer() const {
	Common::String s = getDebugBanner();
	s += "\n";
	const bool includeLauncherRegion = _layoutLevel == kMazeLayoutLevel4;
	const char *layoutDescription = "Unknown Layout";
	// Levels 1-3 use two variants: zero is Base and one is Alt.
	if (_layoutLevel <= kMazeLayoutLevel3) {
		if (_levelVariantIdx == 0)
			layoutDescription = "Base";
		else if (_levelVariantIdx == 1)
			layoutDescription = "Alt";
	} else if (_layoutLevel == kMazeLayoutLevel4) {
		// Level 4 uses zero for Base, two for Alt, and one for the restored layout.
		if (_levelVariantIdx == 0)
			layoutDescription = "Base";
		else if (_levelVariantIdx == 1)
			layoutDescription = "Restored";
		else if (_levelVariantIdx == 2)
			layoutDescription = "Alt";
	} else if (_layoutLevel == kMazeLayoutLevel4SmallPack) {
		layoutDescription = "Exception for small-pack";
	}
	s += Common::String::format("  Active layout: %s (REGS %d).\n", layoutDescription, _mazeLayoutRegsId);
	if (!_conditionSubstitutions.empty()) {
		const char *conditionCountSuffix;
		const char *conditionVerb;
		if (_conditionSubstitutions.size() == 1) {
			conditionCountSuffix = "";
			conditionVerb = "was";
		} else {
			conditionCountSuffix = "s";
			conditionVerb = "were";
		}
		s += Common::String::format(
			"  The arriving pack could not finish this layout as authored, so %u condition cell%s\n"
			"  %s substituted:\n",
			_conditionSubstitutions.size(), conditionCountSuffix, conditionVerb);
		for (uint substitutionIdx = 0; substitutionIdx < _conditionSubstitutions.size(); substitutionIdx++)
			s += Common::String::format("    %s.\n", describeConditionSubstitution(_conditionSubstitutions[substitutionIdx]).c_str());
	}

	ZoombiniPuzzleMaze::SolverPlanContext planContext = {};
	debugBuildMazePlanContext(planContext);
	ZoombiniPuzzleMaze::SolverLevel4Plan level4Plan;
	bool level4PlanSearchExhausted = false;
	uint32 level4Steps = 0;
	ZoombiniPuzzleMaze::SolverGenericPlan genericPlan;
	const ZoombiniPuzzleMaze::SolverGenericSearchLimits genericSearchLimits = {
		kDebugMazeGenericSolverMaxSteps,
		kDebugMazeGenericSolverMaxNodes};
	ZoombiniPuzzleMaze::SolverGenericSearchResult genericSearch;
	bool genericSearchRan = false;
	if (_layoutLevel == kMazeLayoutLevel4) {
		// The search may run up to kDebugMazeSolverMaxSteps simulated launches,
		// so announce the cap before the console prints the assembled answer.
		GUI::Debugger *debugger = _vm->getDebugger();
		if (debugger)
			debugger->debugPrintf("Finding... (max %u simulated launches)\n",
								  static_cast<uint>(kDebugMazeSolverMaxSteps));
		debugBuildMazeLevel4Plan(planContext, level4Plan, level4PlanSearchExhausted, level4Steps);
		if (!level4Plan.valid) {
			// A layout the staged relay shape does not describe, such as the
			// restored REGS 16607, still has an answer; search for it directly.
			if (debugger)
				debugger->debugPrintf("Finding a direct launch order... (max %u simulated launches)\n",
									  static_cast<uint>(kDebugMazeGenericSolverMaxSteps));
			genericSearchRan = true;
			debugBuildMazeGenericPlan(planContext, genericPlan, genericSearchLimits, genericSearch);
		}
		s += Common::String::format("  Staged relay search: %u steps used (max %u).\n", static_cast<uint>(level4Steps), static_cast<uint>(kDebugMazeSolverMaxSteps));
		if (genericSearchRan)
			s += Common::String::format("  Direct launch-order search: %u steps used (max %u).\n", static_cast<uint>(genericSearch.stepsUsed), static_cast<uint>(kDebugMazeGenericSolverMaxSteps));
	}
	ZoombiniPuzzleMaze::SolverLevel123Plan level123Plan;
	if (_layoutLevel <= kMazeLayoutLevel3) {
		ZoombiniPuzzleMaze::SolverBudget budget123;
		debugBuildMazeLevel123Plan(planContext, level123Plan, budget123);
		s += Common::String::format("  Routing search: %u steps used (uncapped).\n", static_cast<uint>(budget123.steps));
	}

	struct DebugLaunchPrediction {
		const ZmbSnoid *snoid;
		int16 launcherSeatIdx;
		LaunchSimulationOutcome outcome;
		Common::Array<int16> switchCells;
	};
	Common::Array<DebugLaunchPrediction> predictions;
	int16 loadedSnoidCount = 0;
	int16 acceptedSnoidCount = 0;
	int16 traversingSnoidCount = 0;

	// Dry-run each currently legal launch on a private copy of the arrow state.
	for (int16 snoidIdx = 0; snoidIdx < _pageLoadedZmbCount; snoidIdx++) {
		ZmbSnoid *snoid = getSnoid(10000 + snoidIdx);
		if (!snoid)
			continue;
		loadedSnoidCount += 1;
		const int16 runnerIdx = findRunnerBySnoidId(snoid->getId());
		const bool isPlaced = 0 <= runnerIdx && _runnerStates[runnerIdx].placed;
		bool isHeld = false;
		if (isPlaced) {
			for (int16 cellIdx = 0; cellIdx < _cellStateCount; cellIdx++) {
				if (_cellStates[cellIdx].heldRunner == runnerIdx) {
					isHeld = true;
					break;
				}
			}
		}
		const MazeCornerGroup cornerGroup = (0 <= runnerIdx) ? _runnerStates[runnerIdx].cornerGroup : kLowerLeft00;
		if (snoid->_packIsOccupied) {
			acceptedSnoidCount += 1;
			continue;
		}
		if (isPlaced) {
			if (!isHeld)
				traversingSnoidCount += 1;
			continue;
		}
		for (int16 seatIdx = 0; seatIdx < 14; seatIdx++) {
			if (!_launcherSeatEnabled[seatIdx] || _launcherOccupancyMarkers[seatIdx] ||
				!_dragConstraintRects[cornerGroup].contains(kSeatPositions[seatIdx]))
				continue;
			DebugLaunchPrediction prediction;
			prediction.snoid = snoid;
			prediction.launcherSeatIdx = seatIdx;
			prediction.outcome = simulateNextLaunch(snoid, seatIdx, &prediction.switchCells);
			predictions.push_back(prediction);
		}
	}

	int16 directCompletionSnoidCount = 0;
	for (int16 snoidIdx = 0; snoidIdx < _pageLoadedZmbCount; snoidIdx++) {
		const ZmbSnoid *snoid = getSnoid(10000 + snoidIdx);
		if (!snoid)
			continue;
		bool hasDirectCompletion = false;
		for (uint predictionIdx = 0; predictionIdx < predictions.size(); predictionIdx++) {
			const DebugLaunchPrediction &prediction = predictions[predictionIdx];
			if (prediction.snoid == snoid && prediction.outcome == LaunchSimulationOutcome::kExitUpperRight03) {
				hasDirectCompletion = true;
				break;
			}
		}
		if (hasDirectCompletion)
			directCompletionSnoidCount += 1;
	}

	struct DebugSwitchRequirement {
		int16 switchCellIdx;
		bool hasLinkedBranch;
		bool linkedBranchChanged;
		bool hasOccupiedSticky;
		bool neededNow;
		int16 reachableRouteCount;
		int16 preferredRouteCount;
	};
	Common::Array<DebugSwitchRequirement> switchRequirements;
	int16 branchSwitchCount = 0;
	int16 changedBranchSwitchCount = 0;
	int16 occupiedStickyCellCount = 0;
	for (int16 cellIdx = 0; cellIdx < _cellStateCount; cellIdx++) {
		const MazeCellState &switchCell = _cellStates[cellIdx];
		if (switchCell.type != kMazeCellType06_ColoredSwitch || switchCell.waveGroup <= 1 ||
			kMaxWaveGroups < switchCell.waveGroup)
			continue;

		DebugSwitchRequirement requirement;
		requirement.switchCellIdx = cellIdx;
		requirement.hasLinkedBranch = false;
		requirement.linkedBranchChanged = false;
		requirement.hasOccupiedSticky = false;
		requirement.neededNow = false;
		requirement.reachableRouteCount = 0;
		requirement.preferredRouteCount = 0;

		const int16 groupIdx = switchCell.waveGroup - 1;
		for (int16 groupCellIdx = 0; groupCellIdx < _waveGroupCellCounts[groupIdx]; groupCellIdx++) {
			const int16 linkedCellIdx = _waveGroupCellIndices[groupIdx][groupCellIdx];
			const MazeCellState &linked = _cellStates[linkedCellIdx];
			if (linked.type == kMazeCellType04_ColoredArrow) {
				requirement.hasLinkedBranch = true;
				const int16 currentDirection = _nodeDirection[linked.row][linked.col];
				const int16 initialDirection = debugMazeInitialDirection(_layoutRegsWords, linkedCellIdx, currentDirection);
				if (currentDirection != initialDirection)
					requirement.linkedBranchChanged = true;
			} else if (linked.type == kMazeCellType05_ColoredSticky && 0 <= linked.heldRunner) {
				requirement.hasOccupiedSticky = true;
				occupiedStickyCellCount += 1;
			}
		}

		if (requirement.hasLinkedBranch) {
			branchSwitchCount += 1;
			if (requirement.linkedBranchChanged)
				changedBranchSwitchCount += 1;
		}

		for (uint predictionIdx = 0; predictionIdx < predictions.size(); predictionIdx++) {
			const DebugLaunchPrediction &prediction = predictions[predictionIdx];
			if (!debugMazeContainsCell(prediction.switchCells, cellIdx))
				continue;
			requirement.reachableRouteCount += 1;
			if (debugMazeIsPreferredKeyOutcome(prediction.outcome))
				requirement.preferredRouteCount += 1;
		}
		switchRequirements.push_back(requirement);
	}

	for (uint requirementIdx = 0; requirementIdx < switchRequirements.size(); requirementIdx++) {
		DebugSwitchRequirement &requirement = switchRequirements[requirementIdx];
		if (_layoutLevel == kMazeLayoutLevel4 && 0 < occupiedStickyCellCount)
			requirement.neededNow = requirement.hasOccupiedSticky;
		else
			requirement.neededNow = requirement.hasLinkedBranch && !requirement.linkedBranchChanged;
	}

	s += Common::String::format("  Layout selector %d solution scenario:\n", static_cast<int>(_layoutLevel));
	switch (_layoutLevel) {
	case kMazeLayoutLevel1:
	case kMazeLayoutLevel2:
	case kMazeLayoutLevel3:
		if (level123Plan.valid && level123Plan.hasSwitchPhase) {
			const bool colorBlindMode = _vm->isColorBlindModeEnabled();
			const char *precursorColor = debugMazePlanGroupColor(planContext, level123Plan.precursorGroup, colorBlindMode);
			const char *finalColor = debugMazePlanGroupColor(planContext, level123Plan.finalGroup, colorBlindMode);
			const Common::String priorityCondition = debugMazePlanTraitCondition(level123Plan.priorityCategory, level123Plan.priorityValue,
																				 level123Plan.priorityMatched);
			const ZoombiniPuzzleMaze::SolverPlanAction &precursorAction = level123Plan.actions[0];
			const ZoombiniPuzzleMaze::SolverPlanAction &victimAction = level123Plan.actions[level123Plan.victimActionIdx];
			const ZoombiniPuzzleMaze::SolverPlanAction &finalAction = level123Plan.actions[level123Plan.finalActionIdx];
			if (level123Plan.precursorInPriorityGroup) {
				s += Common::String::format(
					"    1. Separate the %d Zoombinis with %s as the priority group. Reserve key 1, the protected Zoombini, and key 2 listed below; no non-priority Zoombini should be sent yet.\n",
					level123Plan.priorityCount, priorityCondition.c_str());
			} else {
				s += Common::String::format(
					"    1. Separate the %d Zoombinis with %s as the priority group and reserve the separate precursor key 1. Also mark the protected Zoombini and key 2 listed below; no other Zoombini should be sent yet.\n",
					level123Plan.priorityCount, priorityCondition.c_str());
			}
			s += Common::String::format(
				"    2. Send key 1 first through the %s. It presses the %s precursor switch and opens the route that otherwise rejects the protected Zoombini.\n",
				debugMazeLauncherLabel(kLauncherPositions, precursorAction.launcherSeatIdx, includeLauncherRegion).c_str(), precursorColor);
			int16 deliveredPriorityCount;
			if (level123Plan.precursorInPriorityGroup)
				deliveredPriorityCount = level123Plan.priorityCount - 2;
			else
				deliveredPriorityCount = level123Plan.priorityCount - 1;
			const char *deliveredPrioritySuffix;
			if (deliveredPriorityCount == 1)
				deliveredPrioritySuffix = "";
			else
				deliveredPrioritySuffix = "s";
			s += Common::String::format(
				"    3. Send the protected Zoombini through the %s, then send every other waiting member of the %s group except key 2. This phase delivers %d group member%s after key 1 without changing another colored switch.\n",
				debugMazeLauncherLabel(kLauncherPositions, victimAction.launcherSeatIdx, includeLauncherRegion).c_str(), priorityCondition.c_str(),
				deliveredPriorityCount, deliveredPrioritySuffix);
			s += Common::String::format(
				"    4. Send key 2 last among that group through the %s. It presses the %s closing switch and changes the branch so the non-priority group can use the completion route.\n",
				debugMazeLauncherLabel(kLauncherPositions, finalAction.launcherSeatIdx, includeLauncherRegion).c_str(), finalColor);
			s += Common::String::format(
				"    5. Send all %d remaining non-priority Zoombinis. Follow the same trait-routing cycle as the simpler layouts: use the currently compatible route, let the white arrow advance, and repeat until all are accepted.\n",
				level123Plan.remainderCount);
		} else if (level123Plan.valid) {
			const ZoombiniPuzzleMaze::SolverPlanAction &firstAction = level123Plan.actions[0];
			const Common::String firstFilters = debugMazePlanRouteFilters(planContext, firstAction);
			if (!firstFilters.empty()) {
				s += Common::String::format(
					"    1. Start the verified routing cycle through the %s with a waiting Zoombini that follows this route: %s.\n",
					debugMazeLauncherLabel(kLauncherPositions, firstAction.launcherSeatIdx, includeLauncherRegion).c_str(), firstFilters.c_str());
			} else {
				s += Common::String::format(
					"    1. Start the verified routing cycle through the %s with any Zoombini accepted by the currently open route.\n",
					debugMazeLauncherLabel(kLauncherPositions, firstAction.launcherSeatIdx, includeLauncherRegion).c_str());
			}
			s += Common::String::format(
				"    2. Each successful passage advances the white arrow to the next trait route. At each state send a compatible waiting group member; the verified cycle accepts all %d Zoombinis without rejection.\n",
				level123Plan.remainderCount);
		} else if (_layoutLevel == kMazeLayoutLevel1) {
			s += "    Read the trait branches and alternate compatible groups with the white arrow.\n";
		} else if (switchRequirements.empty()) {
			s += "    Sort the trait groups through the open route and cycle the white arrow between groups.\n";
		} else {
			s += "    The full priority-group switch plan could not be derived from this layout and pack.\n";
		}
		break;
	case kMazeLayoutLevel4:
		if (level4Plan.valid) {
			const bool colorBlindMode = _vm->isColorBlindModeEnabled();
			const char *primaryColor = debugMazePlanGroupColor(planContext, level4Plan.primaryGroup, colorBlindMode);
			const char *stickyColor = debugMazePlanGroupColor(planContext, level4Plan.finalStickyGroup, colorBlindMode);
			const char *rescueColor = debugMazePlanGroupColor(planContext, level4Plan.rescueGroup, colorBlindMode);
			const char *highwayColor = debugMazePlanGroupColor(planContext, level4Plan.highwayGroup, colorBlindMode);
			const char *stagingStickyColor = debugMazePlanGroupColor(planContext, level4Plan.stagingStickyGroup, colorBlindMode);
			const int16 totalKeyCount = level4Plan.stagingKeyCount + 4;
			const int16 stagingN = (level4Plan.stagingKeyCount - 1) / 3;
			const char *stagingKeySuffix;
			if (level4Plan.stagingKeyCount == 1)
				stagingKeySuffix = "";
			else
				stagingKeySuffix = "s";
			const Common::String stagingSequence = debugMazePlanStagingLaunchSequence(level4Plan, false, kLauncherPositions);
			const Common::String highwaySequence = debugMazePlanStagingLaunchSequence(level4Plan, true, kLauncherPositions);
			s += Common::String::format(
				"    1. Select and reserve all %d keys listed below. The key-1 staging group uses 3N+1 = %d Zoombini%s (N=%d).\n",
				totalKeyCount, level4Plan.stagingKeyCount,
				stagingKeySuffix, stagingN);
			s += Common::String::format(
				"    2. Before touching the %s switch, send the staging group to the %s corner in this order: %s. They must wait there because the onward colored branch is still locked; the primary switch will also close this transfer route.\n",
				primaryColor, debugMazePlanCornerName(level4Plan.stagingCorner),
				stagingSequence.c_str());
			s += Common::String::format(
				"    3. Send key 2 through the %s. It presses the %s primary switch and waits in a %s sticky cell.\n",
				debugMazeLauncherLabel(kLauncherPositions, level4Plan.stagePrimary.launcherSeatIdx, includeLauncherRegion).c_str(), primaryColor, stickyColor);
			s += Common::String::format(
				"    4. Relaunch the staging group from the %s corner in this order: %s. Together they press the %s highway switch, leave the linked 3-cycle branch in the start-to-goal state, and park one staging key in a %s sticky cell.\n",
				debugMazePlanCornerName(level4Plan.stagingCorner), highwaySequence.c_str(),
				highwayColor, stagingStickyColor);
			s += Common::String::format(
				"    5. Keep keys 3-5 aside and send all other %d non-key Zoombinis through the %s while the %s highway from the start to the goal is open.\n",
				level4Plan.highwaySnoidCount,
				debugMazeLauncherLabel(kLauncherPositions, level4Plan.highwaySeat, includeLauncherRegion).c_str(), highwayColor);
			if (level4Plan.rescueReleasesStagingKey) {
				s += Common::String::format(
					"    6. Send key 4 through the %s to press the %s rescue switch. The parked staging key is released to the goal, and the linked branch opens the route to key 2.\n",
					debugMazeLauncherLabel(kLauncherPositions, level4Plan.stageRescue.launcherSeatIdx, includeLauncherRegion).c_str(), rescueColor);
			} else {
				s += Common::String::format(
					"    6. Send key 4 through the %s to press the %s rescue switch. Its linked branch opens key 2's safe escape route; the remaining staging key stays parked for the final release.\n",
					debugMazeLauncherLabel(kLauncherPositions, level4Plan.stageRescue.launcherSeatIdx, includeLauncherRegion).c_str(), rescueColor);
			}
			s += Common::String::format(
				"    7. Send key 3 through the %s into key 2's %s sticky cell. Key 2 is pushed safely to the goal; key 3 takes its place with the safe release direction.\n",
				debugMazeLauncherLabel(kLauncherPositions, level4Plan.stagePush.launcherSeatIdx, includeLauncherRegion).c_str(), stickyColor);
			if (level4Plan.rescueReleasesStagingKey) {
				s += Common::String::format(
					"    8. Send key 5 through the %s to press the %s final-release switch. Key 3 is released to the goal.\n",
					debugMazeLauncherLabel(kLauncherPositions, level4Plan.stageFinal.launcherSeatIdx, includeLauncherRegion).c_str(), stickyColor);
			} else {
				s += Common::String::format(
					"    8. Send key 5 through the %s to press the %s final-release switch. The parked staging key and key 3 are released to the goal together.\n",
					debugMazeLauncherLabel(kLauncherPositions, level4Plan.stageFinal.launcherSeatIdx, includeLauncherRegion).c_str(), stickyColor);
			}
		} else if (genericPlan.valid) {
			const bool colorBlindMode = _vm->isColorBlindModeEnabled();
			s += Common::String::format(
				"    This layout does not fit the staged relay shape, so the answer is a direct launch order.\n"
				"    %d launches in total; %d of them change the control state.\n",
				static_cast<int>(genericPlan.steps.size()), genericPlan.controlStepCount);
			int16 stepNumber = 0;
			for (uint stepIdx = 0; stepIdx < genericPlan.steps.size(); stepIdx++) {
				const ZoombiniPuzzleMaze::SolverGenericStep &step = genericPlan.steps[stepIdx];
				const Common::String launcher =
					debugMazeLauncherLabel(kLauncherPositions, step.launcherSeatIdx, includeLauncherRegion);
				stepNumber += 1;
				if (step.controlStep) {
					s += Common::String::format(
						"    %2d. Send %s through the %s.%s\n", stepNumber,
						planContext.snoids[step.snoidIdx]->toStr(true).c_str(), launcher.c_str(),
						debugMazeGenericStepText(planContext, step, colorBlindMode).c_str());
					continue;
				}
				// Deliveries that share a launcher change nothing, so group them.
				uint runEnd = stepIdx;
				while (runEnd < genericPlan.steps.size() && !genericPlan.steps[runEnd].controlStep &&
					   genericPlan.steps[runEnd].launcherSeatIdx == step.launcherSeatIdx)
					runEnd += 1;
				if (runEnd - stepIdx == 1) {
					s += Common::String::format(
						"    %2d. Send %s through the %s. It reaches the completion exit.\n", stepNumber,
						planContext.snoids[step.snoidIdx]->toStr(true).c_str(), launcher.c_str());
				} else {
					s += Common::String::format(
						"    %2d. Send these %u Zoombinis through the %s in any order; each reaches the completion exit:\n",
						stepNumber, runEnd - stepIdx, launcher.c_str());
					for (uint runIdx = stepIdx; runIdx < runEnd; runIdx++)
						s += Common::String::format("          %s\n", planContext.snoids[genericPlan.steps[runIdx].snoidIdx]->toStr(true).c_str());
				}
				stepIdx = runEnd - 1;
			}
		} else if (genericSearch.searchExhausted) {
			s += "    No launch order delivers this pack on this layout. The search covered every\n";
			s += "    reachable control state, so at least one Zoombini has to be given up here.\n";
			s += "    Inspect the colored branches, sticky cells, and launcher routes below.\n";
		} else if (genericSearch.limitReached) {
			s += Common::String::format(
				"    Neither the staged relay plan nor a direct launch order was found. The direct\n"
				"    search stopped at its own bound of %u simulated launches or %u search nodes,\n"
				"    so this pack is undecided rather than proven impossible.\n",
				static_cast<uint>(kDebugMazeGenericSolverMaxSteps),
				static_cast<uint>(kDebugMazeGenericSolverMaxNodes));
			s += "    Inspect the colored branches, sticky cells, and launcher routes below.\n";
		} else if (level4PlanSearchExhausted) {
			s += Common::String::format(
				"    The level-4 relay plan search stopped without finding a plan after %u simulated launches.\n",
				static_cast<uint>(kDebugMazeSolverMaxSteps));
			s += "    Inspect the colored branches, sticky cells, and launcher routes below.\n";
		} else {
			s += "    The full level-4 relay plan could not be derived from this layout and pack.\n";
			s += "    Inspect the colored branches, sticky cells, and launcher routes below.\n";
		}
		break;
	default:
		s += "    Route every remaining Zoombini to the right-top completion exit.\n";
		break;
	}

	const int16 remainingSnoidCount = loadedSnoidCount - acceptedSnoidCount;
	s += "  Current progress:\n";
	s += Common::String::format("    Accepted: %d/%d; remaining: %d.\n", acceptedSnoidCount, loadedSnoidCount, remainingSnoidCount);
	if (0 < traversingSnoidCount)
		s += Common::String::format("    %d Zoombini%s currently traversing; wait for the result before acting on this snapshot.\n",
									traversingSnoidCount, traversingSnoidCount == 1 ? " is" : "s are");
	s += Common::String::format("    %d waiting Zoombini%s can reach completion in the current control state.\n",
								directCompletionSnoidCount, directCompletionSnoidCount == 1 ? "" : "s");
	if (0 < branchSwitchCount)
		s += Common::String::format("    Colored branch switches changed from their starting state: %d/%d.\n", changedBranchSwitchCount, branchSwitchCount);
	if (0 < occupiedStickyCellCount)
		s += Common::String::format("    Occupied sticky cells awaiting release: %d.\n", occupiedStickyCellCount);

	if (remainingSnoidCount == 0) {
		s += "    Current stage: complete -- every Zoombini has reached the completion exit.\n";
	} else if (_layoutLevel <= kMazeLayoutLevel3 && level123Plan.valid) {
		if (level123Plan.hasSwitchPhase) {
			auto actualAccepted = [&](int16 snoidIdx) -> bool {
				return 0 <= snoidIdx && snoidIdx < planContext.snoidCount &&
					   planContext.snoids[snoidIdx] && planContext.snoids[snoidIdx]->_packIsOccupied;
			};
			int16 acceptedPriorityCount = 0;
			for (int16 snoidIdx = 0; snoidIdx < planContext.snoidCount; snoidIdx++) {
				if (debugMazePlanMatchesTrait(
						planContext, snoidIdx, level123Plan.priorityCategory,
						level123Plan.priorityValue, level123Plan.priorityMatched) &&
					actualAccepted(snoidIdx))
					acceptedPriorityCount += 1;
			}
			if (!actualAccepted(level123Plan.precursorKey)) {
				s += "    Current stage: 2/5 precursor unlock -- send key 1 before the protected Zoombini.\n";
			} else if (!actualAccepted(level123Plan.finalKey) &&
					   acceptedPriorityCount < level123Plan.priorityCount - 1) {
				s += Common::String::format(
					"    Current stage: 3/5 priority delivery -- send %d remaining priority-group member%s before key 2.\n",
					level123Plan.priorityCount - 1 - acceptedPriorityCount,
					level123Plan.priorityCount - 1 - acceptedPriorityCount == 1 ? "" : "s");
			} else if (!actualAccepted(level123Plan.finalKey)) {
				s += "    Current stage: 4/5 closing switch -- every other priority member is safe; send key 2 now.\n";
			} else {
				s += Common::String::format(
					"    Current stage: 5/5 cyclic delivery -- send the %d remaining non-priority Zoombinis through the opened route.\n",
					remainingSnoidCount);
			}
		} else {
			const ZoombiniPuzzleMaze::SolverPlanAction *nextAction = nullptr;
			for (uint actionIdx = 0; actionIdx < level123Plan.actions.size(); actionIdx++) {
				const ZoombiniPuzzleMaze::SolverPlanAction &action = level123Plan.actions[actionIdx];
				if (0 <= action.snoidIdx && action.snoidIdx < planContext.snoidCount &&
					planContext.snoids[action.snoidIdx] &&
					!planContext.snoids[action.snoidIdx]->_packIsOccupied) {
					nextAction = &action;
					break;
				}
			}
			if (nextAction) {
				const Common::String nextFilters = debugMazePlanRouteFilters(planContext, *nextAction);
				if (!nextFilters.empty()) {
					s += Common::String::format(
						"    Current stage: cyclic routing -- next use the %s with a waiting Zoombini following %s; %d remain.\n",
						debugMazeLauncherLabel(kLauncherPositions, nextAction->launcherSeatIdx, includeLauncherRegion).c_str(),
						nextFilters.c_str(), remainingSnoidCount);
				} else {
					s += Common::String::format(
						"    Current stage: cyclic routing -- next use the %s with a Zoombini accepted by the open route; %d remain.\n",
						debugMazeLauncherLabel(kLauncherPositions, nextAction->launcherSeatIdx, includeLauncherRegion).c_str(),
						remainingSnoidCount);
				}
			} else {
				s += "    Current stage: cyclic routing -- follow the currently open trait route to completion.\n";
			}
		}
	} else if (_layoutLevel == kMazeLayoutLevel4) {
		if (level4Plan.valid) {
			auto actualHeldGroup = [&](int16 snoidIdx) -> int16 {
				if (snoidIdx < 0 || planContext.snoidCount <= snoidIdx || !planContext.snoids[snoidIdx])
					return 0;
				const int16 runnerIdx = findRunnerBySnoidId(planContext.snoids[snoidIdx]->getId());
				if (runnerIdx < 0)
					return 0;
				for (int16 cellIdx = 0; cellIdx < _cellStateCount; cellIdx++) {
					if (_cellStates[cellIdx].heldRunner == runnerIdx)
						return _cellStates[cellIdx].waveGroup;
				}
				return 0;
			};
			auto actualCorner = [&](int16 snoidIdx) -> int16 {
				if (snoidIdx < 0 || planContext.snoidCount <= snoidIdx || !planContext.snoids[snoidIdx])
					return 0;
				const int16 runnerIdx = findRunnerBySnoidId(planContext.snoids[snoidIdx]->getId());
				return (0 <= runnerIdx) ? _runnerStates[runnerIdx].cornerGroup : 0;
			};
			auto actualAccepted = [&](int16 snoidIdx) -> bool {
				return 0 <= snoidIdx && snoidIdx < planContext.snoidCount &&
					   planContext.snoids[snoidIdx] && planContext.snoids[snoidIdx]->_packIsOccupied;
			};
			int16 stagingAtCorner = 0;
			int16 stagingRelayComplete = 0;
			for (int16 keyIdx = 0; keyIdx < level4Plan.stagingKeyCount; keyIdx++) {
				const int16 stagingKey = level4Plan.stagingKeys[keyIdx];
				if (actualCorner(stagingKey) == level4Plan.stagingCorner)
					stagingAtCorner += 1;
				if (actualAccepted(stagingKey) ||
					actualHeldGroup(stagingKey) == level4Plan.stagingStickyGroup)
					stagingRelayComplete += 1;
			}
			int16 nonKeyRemaining = 0;
			for (int16 snoidIdx = 0; snoidIdx < planContext.snoidCount; snoidIdx++) {
				if (debugMazePlanIsStagingKey(
						level4Plan.stagingKeys, level4Plan.stagingKeyCount, snoidIdx) ||
					snoidIdx == level4Plan.primaryKey ||
					snoidIdx == level4Plan.pusherKey || snoidIdx == level4Plan.rescueKey ||
					snoidIdx == level4Plan.finalKey)
					continue;
				if (!actualAccepted(snoidIdx))
					nonKeyRemaining += 1;
			}
			if (remainingSnoidCount == 0) {
				s += "    Current stage: complete -- the planned relay and highway delivery are finished.\n";
			} else if (actualHeldGroup(level4Plan.pusherKey) == level4Plan.finalStickyGroup) {
				if (level4Plan.rescueReleasesStagingKey)
					s += "    Current stage: 8/8 final release -- send key 5 to release key 3.\n";
				else
					s += "    Current stage: 8/8 final release -- send key 5 to release the parked staging key and key 3 together.\n";
			} else if (actualAccepted(level4Plan.rescueKey) &&
					   actualHeldGroup(level4Plan.primaryKey) == level4Plan.finalStickyGroup) {
				s += "    Current stage: 7/8 sticky push -- send key 3 to push key 2 safely onward.\n";
			} else if (stagingRelayComplete == level4Plan.stagingKeyCount) {
				if (0 < nonKeyRemaining)
					s += Common::String::format("    Current stage: 5/8 highway delivery -- send %d remaining non-key Zoombinis before the rescue relay.\n", nonKeyRemaining);
				else if (level4Plan.rescueReleasesStagingKey)
					s += "    Current stage: 6/8 rescue switch -- send key 4 to release the parked staging key and open key 2's route.\n";
				else
					s += "    Current stage: 6/8 rescue switch -- send key 4 to open key 2's route; the remaining staging key stays parked.\n";
			} else if (actualHeldGroup(level4Plan.primaryKey) == level4Plan.finalStickyGroup) {
				s += Common::String::format(
					"    Current stage: 4/8 highway unlock -- relay %d remaining staging key%s from the staging corner.\n",
					level4Plan.stagingKeyCount - stagingRelayComplete,
					level4Plan.stagingKeyCount - stagingRelayComplete == 1 ? "" : "s");
			} else if (stagingAtCorner == level4Plan.stagingKeyCount) {
				s += "    Current stage: 3/8 primary switch -- send key 2 only after all staging transfers are done.\n";
			} else {
				s += Common::String::format(
					"    Current stage: 2/8 staging transfer -- send %d remaining staging key%s before the primary switch locks that route.\n",
					level4Plan.stagingKeyCount - stagingAtCorner,
					level4Plan.stagingKeyCount - stagingAtCorner == 1 ? "" : "s");
			}
		} else if (genericPlan.valid) {
			// A launch is done once its Zoombini stands where the plan expects.
			int16 nextStep = -1;
			for (uint stepIdx = 0; stepIdx < genericPlan.steps.size() && nextStep < 0; stepIdx++) {
				if (debugMazeLiveSnoidSpot(planContext, genericPlan.steps[stepIdx].snoidIdx) != genericPlan.postSpots[stepIdx])
					nextStep = static_cast<int16>(stepIdx);
			}
			if (nextStep < 0) {
				s += "    Current stage: complete -- every planned launch has been made.\n";
			} else {
				const ZoombiniPuzzleMaze::SolverGenericStep &step = genericPlan.steps[nextStep];
				s += Common::String::format(
					"    Current stage: launch order step %d of %d -- next send %s through the %s.\n",
					nextStep + 1, static_cast<int>(genericPlan.steps.size()),
					planContext.snoids[step.snoidIdx]->toStr(true).c_str(),
					debugMazeLauncherLabel(kLauncherPositions, step.launcherSeatIdx, includeLauncherRegion).c_str());
			}
		} else if (0 < occupiedStickyCellCount) {
			s += "    Current stage: sticky relay -- release the held Zoombini with the matching switch key.\n";
		} else if (changedBranchSwitchCount == 0) {
			s += "    Current stage: primary unlock -- operate the first reachable colored branch switch.\n";
		} else if (changedBranchSwitchCount < branchSwitchCount) {
			s += "    Current stage: switch chain -- operate the next exposed colored branch switch.\n";
		} else {
			s += "    Current stage: final routing -- send the remaining groups to completion.\n";
		}
	} else if (changedBranchSwitchCount < branchSwitchCount) {
		s += "    Current stage: colored-switch setup -- operate the still-unset switches with their keys.\n";
	} else {
		s += "    Current stage: final routing -- all colored branches have changed; finish the remaining groups.\n";
	}
	const bool hasCompletePlan =
		(_layoutLevel == kMazeLayoutLevel4 && (level4Plan.valid || genericPlan.valid)) ||
		(_layoutLevel <= kMazeLayoutLevel3 && level123Plan.valid);
	if (!hasCompletePlan)
		s += "    Rerun after every launch, switch press, sticky release, or arrow cycle.\n";

	bool anyCurrentKey = false;
	for (uint requirementIdx = 0; requirementIdx < switchRequirements.size(); requirementIdx++) {
		const DebugSwitchRequirement &requirement = switchRequirements[requirementIdx];
		if (requirement.neededNow && 0 < requirement.reachableRouteCount) {
			anyCurrentKey = true;
			break;
		}
	}

	if (_layoutLevel == kMazeLayoutLevel4 && level4Plan.valid) {
		const bool colorBlindMode = _vm->isColorBlindModeEnabled();
		const char *primaryColor = debugMazePlanGroupColor(planContext, level4Plan.primaryGroup, colorBlindMode);
		const char *stickyColor = debugMazePlanGroupColor(planContext, level4Plan.finalStickyGroup, colorBlindMode);
		const char *rescueColor = debugMazePlanGroupColor(planContext, level4Plan.rescueGroup, colorBlindMode);
		const char *highwayColor = debugMazePlanGroupColor(planContext, level4Plan.highwayGroup, colorBlindMode);
		const char *stagingStickyColor = debugMazePlanGroupColor(planContext, level4Plan.stagingStickyGroup, colorBlindMode);
		s += "  Complete key roster for the full solution:\n";
		for (int16 keyIdx = 0; keyIdx < level4Plan.stagingKeyCount; keyIdx++) {
			const int16 stagingKey = level4Plan.stagingKeys[keyIdx];
			const ZoombiniPuzzleMaze::SolverPlanAction &transferAction = level4Plan.stageStagingActions[keyIdx];
			const ZoombiniPuzzleMaze::SolverPlanAction *highwayAction = nullptr;
			for (int16 actionIdx = 0; actionIdx < level4Plan.stagingKeyCount; actionIdx++) {
				if (level4Plan.stageHighwaySwitchActions[actionIdx].snoidIdx == stagingKey) {
					highwayAction = &level4Plan.stageHighwaySwitchActions[actionIdx];
					break;
				}
			}
			if (!highwayAction)
				continue;
			const Common::String keyLabel = debugMazePlanStagingLabel(level4Plan, stagingKey);
			s += Common::String::format("    %s. %s\n", keyLabel.c_str(), planContext.snoids[stagingKey]->toStr(true).c_str());
			if (stagingKey == level4Plan.stagingKey) {
				s += Common::String::format(
					"       Staging/highway key: %s -> %s corner, then %s -> %s relay -> %s sticky.\n",
					debugMazeLauncherLabel(kLauncherPositions, transferAction.launcherSeatIdx, includeLauncherRegion).c_str(),
					debugMazePlanCornerName(level4Plan.stagingCorner),
					debugMazeLauncherLabel(kLauncherPositions, highwayAction->launcherSeatIdx, includeLauncherRegion).c_str(),
					highwayColor, stagingStickyColor);
			} else {
				s += Common::String::format(
					"       Staging/highway key: %s -> %s corner, then %s -> %s relay -> goal.\n",
					debugMazeLauncherLabel(kLauncherPositions, transferAction.launcherSeatIdx, includeLauncherRegion).c_str(),
					debugMazePlanCornerName(level4Plan.stagingCorner),
					debugMazeLauncherLabel(kLauncherPositions, highwayAction->launcherSeatIdx, includeLauncherRegion).c_str(), highwayColor);
			}
			const Common::String transferFilters = debugMazePlanRouteFilters(planContext, transferAction);
			const Common::String highwayFilters = debugMazePlanRouteFilters(planContext, *highwayAction);
			if (!transferFilters.empty())
				s += Common::String::format("       Required transfer traits: %s.\n", transferFilters.c_str());
			if (!highwayFilters.empty())
				s += Common::String::format("       Required highway-relay traits: %s.\n", highwayFilters.c_str());
		}
		s += Common::String::format("    2. %s\n", planContext.snoids[level4Plan.primaryKey]->toStr(true).c_str());
		s += Common::String::format(
			"       Primary-switch key: %s -> %s switch -> %s sticky; later pushed out by key 3.\n",
			debugMazeLauncherLabel(kLauncherPositions, level4Plan.stagePrimary.launcherSeatIdx, includeLauncherRegion).c_str(),
			primaryColor, stickyColor);
		s += Common::String::format("    3. %s\n", planContext.snoids[level4Plan.pusherKey]->toStr(true).c_str());
		s += Common::String::format(
			"       Sticky-push key: %s pushes key 2 out of the %s sticky and waits there in the safe release direction.\n",
			debugMazeLauncherLabel(kLauncherPositions, level4Plan.stagePush.launcherSeatIdx, includeLauncherRegion).c_str(), stickyColor);
		s += Common::String::format("    4. %s\n", planContext.snoids[level4Plan.rescueKey]->toStr(true).c_str());
		if (level4Plan.rescueReleasesStagingKey) {
			s += Common::String::format(
				"       Rescue-switch key: %s -> %s switch; releases the parked staging key and opens key 2's escape branch.\n",
				debugMazeLauncherLabel(kLauncherPositions, level4Plan.stageRescue.launcherSeatIdx, includeLauncherRegion).c_str(), rescueColor);
		} else {
			s += Common::String::format(
				"       Rescue-switch key: %s -> %s switch; opens key 2's safe escape branch.\n",
				debugMazeLauncherLabel(kLauncherPositions, level4Plan.stageRescue.launcherSeatIdx, includeLauncherRegion).c_str(), rescueColor);
		}
		s += Common::String::format("    5. %s\n", planContext.snoids[level4Plan.finalKey]->toStr(true).c_str());
		s += Common::String::format(
			"       Final-release key: %s -> %s switch; releases %s after every other Zoombini is safe.\n",
			debugMazeLauncherLabel(kLauncherPositions, level4Plan.stageFinal.launcherSeatIdx, includeLauncherRegion).c_str(), stickyColor,
			level4Plan.rescueReleasesStagingKey ? "key 3" : "the parked staging key and key 3");
	} else if (_layoutLevel <= kMazeLayoutLevel3 && level123Plan.valid) {
		if (level123Plan.hasSwitchPhase) {
			const bool colorBlindMode = _vm->isColorBlindModeEnabled();
			const char *precursorColor = debugMazePlanGroupColor(planContext, level123Plan.precursorGroup, colorBlindMode);
			const char *finalColor = debugMazePlanGroupColor(planContext, level123Plan.finalGroup, colorBlindMode);
			const Common::String priorityCondition = debugMazePlanTraitCondition(level123Plan.priorityCategory, level123Plan.priorityValue,
																				 level123Plan.priorityMatched);
			const ZoombiniPuzzleMaze::SolverPlanAction &precursorAction = level123Plan.actions[0];
			const ZoombiniPuzzleMaze::SolverPlanAction &victimAction = level123Plan.actions[level123Plan.victimActionIdx];
			const ZoombiniPuzzleMaze::SolverPlanAction &finalAction = level123Plan.actions[level123Plan.finalActionIdx];
			s += "  Complete critical roster for the full solution:\n";
			s += Common::String::format("    1. %s\n", planContext.snoids[level123Plan.precursorKey]->toStr(true).c_str());
			s += Common::String::format(
				"       Precursor-switch key: %s -> %s switch. Send first to make the protected Zoombini's route safe.\n",
				debugMazeLauncherLabel(kLauncherPositions, precursorAction.launcherSeatIdx, includeLauncherRegion).c_str(), precursorColor);
			s += Common::String::format("    V. %s\n", planContext.snoids[level123Plan.victim]->toStr(true).c_str());
			s += Common::String::format(
				"       Protected Zoombini: %s after key 1 and before key 2. In the starting switch state this same route is rejected.\n",
				debugMazeLauncherLabel(kLauncherPositions, victimAction.launcherSeatIdx, includeLauncherRegion).c_str());
			s += Common::String::format("    2. %s\n", planContext.snoids[level123Plan.finalKey]->toStr(true).c_str());
			s += Common::String::format(
				"       Priority-group closing key: %s -> %s switch. Send last among all Zoombinis with %s to open the route for everyone else.\n",
				debugMazeLauncherLabel(kLauncherPositions, finalAction.launcherSeatIdx, includeLauncherRegion).c_str(),
				finalColor, priorityCondition.c_str());
			const Common::String precursorFilters = debugMazePlanRouteFilters(planContext, precursorAction);
			const Common::String victimFilters = debugMazePlanRouteFilters(planContext, victimAction);
			const Common::String finalFilters = debugMazePlanRouteFilters(planContext, finalAction);
			if (!precursorFilters.empty())
				s += Common::String::format("       Key 1 route traits: %s.\n", precursorFilters.c_str());
			if (!victimFilters.empty())
				s += Common::String::format("       Protected Zoombini route traits: %s.\n", victimFilters.c_str());
			if (!finalFilters.empty())
				s += Common::String::format("       Key 2 route traits: %s.\n", finalFilters.c_str());
		} else {
			s += "  Complete key roster for the full solution:\n";
			s += "    (none; this layout is solved entirely by the trait/white-arrow routing cycle)\n";
		}
	} else {
		s += "  Key Zoombinis for the current stage:\n";
		if (anyCurrentKey) {
			for (uint requirementIdx = 0; requirementIdx < switchRequirements.size(); requirementIdx++) {
				const DebugSwitchRequirement &requirement = switchRequirements[requirementIdx];
				if (!requirement.neededNow || requirement.reachableRouteCount <= 0)
					continue;
				const MazeCellState &switchCell = _cellStates[requirement.switchCellIdx];
				const MazeColorShapeBase colorShapeBase = static_cast<MazeColorShapeBase>(switchCell.shapeBase);
				const char *colorName;
				if (_vm->isColorBlindModeEnabled() && colorShapeBase == MazeColorShapeBase::kPurple157)
					colorName = "Dark blue";
				else
					colorName = debugMazeColorName(colorShapeBase);
				s += Common::String::format("    %s switch key -- switch cell (%d, %d):\n", colorName, switchCell.row + 1, switchCell.col + 1);
				s += "      Purpose:\n";
				const int16 groupIdx = switchCell.waveGroup - 1;
				for (int16 groupCellIdx = 0; groupCellIdx < _waveGroupCellCounts[groupIdx]; groupCellIdx++) {
					const MazeCellState &linked = _cellStates[_waveGroupCellIndices[groupIdx][groupCellIdx]];
					if (linked.type == kMazeCellType04_ColoredArrow) {
						const int16 currentDirection = _nodeDirection[linked.row][linked.col];
						const int16 nextDirection = debugMazeNextDirection(currentDirection, linked.dirFlags);
						s += Common::String::format("        - rotate branch cell (%d, %d) from %s to %s\n",
													linked.row + 1, linked.col + 1,
													debugMazeDirectionName(currentDirection), debugMazeDirectionName(nextDirection));
					} else if (linked.type == kMazeCellType05_ColoredSticky && 0 <= linked.heldRunner) {
						s += Common::String::format("        - release the Zoombini held at sticky cell (%d, %d)\n", linked.row + 1, linked.col + 1);
					}
				}
				s += "      Key choices:\n";
				const bool preferNonRejectingRoute = 0 < requirement.preferredRouteCount;
				for (uint predictionIdx = 0; predictionIdx < predictions.size(); predictionIdx++) {
					const DebugLaunchPrediction &prediction = predictions[predictionIdx];
					if (!debugMazeContainsCell(prediction.switchCells, requirement.switchCellIdx))
						continue;
					if (preferNonRejectingRoute && !debugMazeIsPreferredKeyOutcome(prediction.outcome))
						continue;
					s += Common::String::format("        %s -- %s; %s\n",
												prediction.snoid->toStr(true).c_str(),
												debugMazeLauncherLabel(kLauncherPositions, prediction.launcherSeatIdx, includeLauncherRegion).c_str(),
												debugMazeOutcomeName(prediction.outcome));
				}
			}
		} else if (switchRequirements.empty()) {
			s += "    (none required in this level's routing scenario)\n";
		} else if (changedBranchSwitchCount == branchSwitchCount && occupiedStickyCellCount == 0) {
			s += "    (none required now; the colored-switch setup is complete)\n";
		} else {
			s += "    (none reachable now; use the open route or cycling arrow to expose the next key)\n";
		}
	}

	return s;
}

void ZoombiniPuzzleMaze::loadRegsConfigByLevel() {
	// Direct practice boot can load Maze before Rod Map initializes the session selectors.
	_vm->initializeMazeLayoutVariants();
	ZoombiniGameState::MazeLayoutVariantState &variantState = _vm->_state->getMazeLayoutVariantState();

	// ID_REGS identifies a family of page resources, not one universal payload format.
	// For Maze, the selected 16600-series resource is the logical layout table.
	// Each difficulty has its own Base/Alt resource pair; L2 does not reuse the L1 or L3 pair.
	// L2 REGS 16603 is denser than 16602 and can therefore look closer to an L3 layout.
	// Select each level's layout variant with its persistent per-level counter.
	// The counter advances after grid generation,
	// so repeated plays cycle deterministically through the available layouts instead of using random selection.
	switch (_layoutLevel) {
	case kMazeLayoutLevel1:
		_levelVariantIdx = variantState._level1;
		_mazeLayoutRegsId = static_cast<int16>(kResRegs16600_MazeLayoutL1Base + _levelVariantIdx);
		break;
	case kMazeLayoutLevel2:
		_levelVariantIdx = variantState._level2;
		_mazeLayoutRegsId = static_cast<int16>(kResRegs16602_MazeLayoutL2Base + _levelVariantIdx);
		break;
	case kMazeLayoutLevel3:
		_levelVariantIdx = variantState._level3;
		_mazeLayoutRegsId = static_cast<int16>(kResRegs16604_MazeLayoutL3Base + _levelVariantIdx);
		break;
	case kMazeLayoutLevel4:
		_levelVariantIdx = variantState._level4;
		_mazeLayoutRegsId = static_cast<int16>(kResRegs16606_MazeLayoutL4Base + _levelVariantIdx);
		break;
	default:
		// Fixed REGS 16609.
		_mazeLayoutRegsId = kResRegs16609_MazeLayoutL4Except;
		_levelVariantIdx = 0;
		break;
	}
	_pathLayoutVariantIdx = _levelVariantIdx;
	debugC(2, MohawkEngine_Zoombini::kDebugPage02, "maze: layout level %d, variant %d, REGS %d", static_cast<int>(_layoutLevel), _levelVariantIdx, _mazeLayoutRegsId);

}

void ZoombiniPuzzleMaze::loadAndParseRegsData() {
	// The selected 16600-series REGS resource is a Maze layout table.
	// Its first ten words contain the launcher header, followed by ten-word cell records.
	// Other Maze REGS resources have different schemas and are read by loadRegsCoordinateTables().
	_layoutRegsWords.clear();
	_layoutCellRecordCount = 0;
	memset(_launcherSeatIds, 0, sizeof(_launcherSeatIds));
	memset(_launcherSeatEnabled, 0, sizeof(_launcherSeatEnabled));

	Common::SeekableReadStream *stream = _vm->getResource(ID_REGS, ZmbResource(ZmbResource::kPage, _mazeLayoutRegsId));
	static constexpr uint32 kHeaderWordCount = 10;
	static constexpr uint32 kRecordWordCount = 10;
	static constexpr uint32 kMinDataSize = kHeaderWordCount * sizeof(int16);
	static constexpr uint32 kMaxDataSize = (kHeaderWordCount + kMaxCellStates * kRecordWordCount) * sizeof(int16);
	const uint32 dataSize = static_cast<uint32>(stream->size());
	if (!ZmbResource::hasSize(stream, kMinDataSize, kMaxDataSize) ||
		(dataSize - kMinDataSize) % (kRecordWordCount * sizeof(int16)) != 0) {
		delete stream;
		error("maze: required REGS %d has an invalid layout-table size", _mazeLayoutRegsId);
		return;
	}

	const uint32 wordCount = dataSize / sizeof(int16);
	Common::Array<int16> parsedData;
	parsedData.resize(wordCount);
	for (uint32 i = 0; i < wordCount; i++)
		parsedData[i] = stream->readSint16BE();
	delete stream;

	const int16 recordCount = static_cast<int16>((wordCount - kHeaderWordCount) / kRecordWordCount);
	if (parsedData[0] != recordCount) {
		error("maze: invalid REGS %d record count: declared %d, parsed %d", _mazeLayoutRegsId, parsedData[0], recordCount);
		return;
	}
	for (int col = 1; col <= 9; col++) {
		if (parsedData[col] < 0 || 14 < parsedData[col]) {
			error("maze: invalid REGS %d launcher slot %d", _mazeLayoutRegsId, parsedData[col]);
			return;
		}
	}

	const auto isValidCellType = [](int16 value) -> bool {
		return (0 <= value && value <= 7) || (20 <= value && value <= 23);
	};
	for (int16 recordIdx = 0; recordIdx < recordCount; recordIdx++) {
		const uint32 baseOffset = kHeaderWordCount + recordIdx * kRecordWordCount;
		const int16 cellType = parsedData[baseOffset];
		const int16 row = parsedData[baseOffset + 1];
		const int16 col = parsedData[baseOffset + 2];
		const int16 waveGroup = parsedData[baseOffset + 3];
		const int16 direction = parsedData[baseOffset + 8];
		if (!isValidCellType(cellType) || row < 0 || kGridRows <= row || col < 0 || kGridCols <= col ||
			waveGroup < 0 || kMaxWaveGroups < waveGroup || direction < kMazeDirection00_West || kMazeDirection03_North < direction) {
			error("maze: invalid REGS %d cell record at index %d", _mazeLayoutRegsId, recordIdx);
			return;
		}
		for (uint32 flagOffset = 4; flagOffset <= 7; flagOffset++) {
			if (parsedData[baseOffset + flagOffset] < 0 || 1 < parsedData[baseOffset + flagOffset]) {
				error("maze: invalid REGS %d direction flag at record %d", _mazeLayoutRegsId, recordIdx);
				return;
			}
		}
		if (parsedData[baseOffset + 9] < 0 || 1 < parsedData[baseOffset + 9]) {
			error("maze: invalid REGS %d cycle flag at record %d", _mazeLayoutRegsId, recordIdx);
			return;
		}
	}

	_layoutRegsWords = parsedData;
	_layoutCellRecordCount = recordCount;
	for (int col = 1; col <= 9; col++)
		_launcherSeatIds[col] = parsedData[col];

	for (int col = 1; col <= 9; col++) {
		int16 seatIdx = _launcherSeatIds[col] - 1;
		if (0 <= seatIdx && seatIdx < 14)
			_launcherSeatEnabled[seatIdx] = true;
	}
	debugC(3, MohawkEngine_Zoombini::kDebugPage02, "maze: loaded REGS %d with %d cells and launcher seats [%d %d %d %d %d %d %d %d %d]",
		   _mazeLayoutRegsId, _layoutCellRecordCount, _launcherSeatIds[1], _launcherSeatIds[2], _launcherSeatIds[3],
		   _launcherSeatIds[4], _launcherSeatIds[5], _launcherSeatIds[6], _launcherSeatIds[7], _launcherSeatIds[8], _launcherSeatIds[9]);

}

void ZoombiniPuzzleMaze::createCreatureFeatures() {
	// One type-1 companion is registered for every active type-1 launcher.
	// The lookup retains the last runner.
	for (int col = 1; col <= 9; col++) {
		int16 slot = _launcherSeatIds[col];
		if (slot == 0)
			continue;
		int16 slotIdx = slot - 1;
		if (slotIdx < 0 || 14 <= slotIdx)
			continue;
		int16 typeId = kCreatureTypeId[slotIdx];
		if (typeId == 1) {
			_launcherCompanionFeatures[1] = loadScrbFeature(
				ZmbResource(ZmbResource::kPage, kResBitmapShape9000_Path), kResScrb9006_CreatureSlot, 7,
				ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE |
					ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_04000000_OVERLAY);
		}
	}

	// These remain visible while their separate draw-on-register highlight is inactive.
	for (int col = 1; col <= 9; col++) {
		int16 slot = _launcherSeatIds[col];
		if (slot == 0)
			continue;
		int16 slotIdx = slot - 1;
		if (slotIdx < 0 || 14 <= slotIdx)
			continue;
		loadScrbFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape7000_Maze),
			kResScrb7000_CreatureBase + slotIdx, 6,
			ZmbFeature::FLAG_00008000_LOOP_ANIM |
				ZmbFeature::FLAG_00100000_PLAY_ONCE |
				ZmbFeature::FLAG_04000000_OVERLAY);
	}

	// SCRB 7014..7027 render only while a Snoid occupies the slot.
	for (int col = 1; col <= 9; col++) {
		int16 slot = _launcherSeatIds[col];
		if (slot == 0)
			continue;
		int16 slotIdx = slot - 1;
		if (slotIdx < 0 || 14 <= slotIdx)
			continue;
		_launcherDropTargetFeatures[slotIdx] = loadScrbFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape7000_Maze), kResScrb7014_HighlightBase + slotIdx, 7, kSeatPositions[slotIdx],
			ZmbFeature::FLAG_00002000_DRAW_ON_REG | ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM |
				ZmbFeature::FLAG_01000000_DEFER_RENDER | ZmbFeature::FLAG_04000000_OVERLAY);
	}

	// Type-1 launchers are registered in a separate pass.
	// SCRB 8005..8007 are foreground pieces.
	// Their exact list position makes the launcher cover the Snoid during the bubble sequence.
	ZmbFeature *type1Anchor = nullptr;
	for (int col = 1; col <= 9; col++) {
		int16 slot = _launcherSeatIds[col];
		if (slot == 0)
			continue;
		int16 slotIdx = slot - 1;
		if (slotIdx < 0 || 14 <= slotIdx)
			continue;
		if (kCreatureTypeId[slotIdx] != 1)
			continue;

		int16 scrbId = kCreatureScrbTable[slotIdx];
		bool hasShadow = (kCreatureHasShadow[slotIdx] != 0);
		_launcherFeatures[slotIdx] = loadScrbFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape9000_Path), scrbId, 7, kLauncherPositions[slotIdx],
			ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE |
				ZmbFeature::FLAG_00800000_POS_DELTA | ZmbFeature::FLAG_04000000_OVERLAY);
		type1Anchor = _launcherFeatures[slotIdx];

		if (slotIdx == 6) {
			loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape8000_Mudball), kResScrb8006_Obstacle, 0,
							ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_04000000_OVERLAY);
		} else if (slotIdx == 7) {
			loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape8000_Mudball), kResScrb8007_ObstacleAlt, 0,
							ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_04000000_OVERLAY);
		}

		if (hasShadow) {
			_launcherShadowFeatures[slotIdx] = loadScrbFeature(
				ZmbResource(ZmbResource::kPage, kResBitmapShape9000_Path), scrbId + 1, 7, kLauncherPositions[slotIdx],
				ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE |
					ZmbFeature::FLAG_00800000_POS_DELTA | ZmbFeature::FLAG_01000000_DEFER_RENDER | ZmbFeature::FLAG_04000000_OVERLAY);
		}
	}

	if (type1Anchor) {
		ZmbFeature *foreground = loadScrbFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape8000_Mudball), kResScrb8005_FinalForeground, 0,
			ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_04000000_OVERLAY);
		manualLinkAfter(foreground, type1Anchor);
	} else {
		loadScrbFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape8000_Mudball), kResScrb8010_FinalForegroundThird, 0,
			ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_04000000_OVERLAY);
	}
}

void ZoombiniPuzzleMaze::createRemainingCreatureFeatures() {
	// Type-2 companions follow the first eleven column-parent runners.
	// As with type 1, the lookup retains the last one.
	for (int col = 1; col <= 9; col++) {
		int16 slot = _launcherSeatIds[col];
		if (slot == 0)
			continue;
		int16 slotIdx = slot - 1;
		if (slotIdx < 0 || 14 <= slotIdx)
			continue;
		int16 typeId = kCreatureTypeId[slotIdx];
		if (typeId == 2) {
			_launcherCompanionFeatures[2] = loadScrbFeature(
				ZmbResource(ZmbResource::kPage, kResBitmapShape9000_Path), kResScrb9007_CreatureSlotAlt, 7,
				ZmbFeature::FLAG_00080000_DEFER_ANIM |
					ZmbFeature::FLAG_00100000_PLAY_ONCE);
		}
	}

	// All non-type-1 launchers and their 8000 foreground pieces form the second registration pass.
	ZmbFeature *lowerAnchor = nullptr;
	ZmbFeature *rightAnchor = nullptr;
	for (int col = 1; col <= 9; col++) {
		int16 slot = _launcherSeatIds[col];
		if (slot == 0)
			continue;
		int16 slotIdx = slot - 1;
		if (slotIdx < 0 || 14 <= slotIdx)
			continue;
		if (kCreatureTypeId[slotIdx] == 1)
			continue;

		const int16 scrbId = kCreatureScrbTable[slotIdx];
		_launcherFeatures[slotIdx] = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape9000_Path), scrbId, 7, kLauncherPositions[slotIdx],
													 ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE |
														 ZmbFeature::FLAG_00800000_POS_DELTA | ZmbFeature::FLAG_04000000_OVERLAY);

		if (slotIdx == 3) {
			loadScrbFeature(
				ZmbResource(ZmbResource::kPage, kResBitmapShape8000_Mudball), kResScrb8002_ForegroundAlt, 0,
				ZmbFeature::FLAG_00008000_LOOP_ANIM |
					ZmbFeature::FLAG_04000000_OVERLAY);
			lowerAnchor = _launcherFeatures[slotIdx];
		} else if (slotIdx == 4) {
			loadScrbFeature(
				ZmbResource(ZmbResource::kPage, kResBitmapShape8000_Mudball), kResScrb8003_ForegroundThird, 0,
				ZmbFeature::FLAG_00008000_LOOP_ANIM |
					ZmbFeature::FLAG_04000000_OVERLAY);
			lowerAnchor = _launcherFeatures[slotIdx];
		} else if (slotIdx == 5) {
			lowerAnchor = _launcherFeatures[slotIdx];
		} else if (slotIdx == 12) {
			// SCRB 8009 is the only 8000-group foreground registered without LOOP_ANIM.
			// It therefore joins the overlay bucket rather than the loop-animation bucket.
			loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape8000_Mudball), kResScrb8009_FinalOverlay, 0, ZmbFeature::FLAG_04000000_OVERLAY);
			rightAnchor = _launcherFeatures[slotIdx];
		} else if (slotIdx == 13) {
			rightAnchor = _launcherFeatures[slotIdx];
		}

		// The non-type-1 pass registers its shadow without LOOP_ANIM, unlike the type-1 pass.
		// The shadow therefore joins the overlay bucket and keeps its list position behind its launcher.
		if (kCreatureHasShadow[slotIdx]) {
			_launcherShadowFeatures[slotIdx] = loadScrbFeature(
				ZmbResource(ZmbResource::kPage, kResBitmapShape9000_Path), scrbId + 1, 7, kLauncherPositions[slotIdx],
				ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_00800000_POS_DELTA |
					ZmbFeature::FLAG_01000000_DEFER_RENDER | ZmbFeature::FLAG_04000000_OVERLAY);
		}
	}

	if (lowerAnchor) {
		ZmbFeature *foreground = loadScrbFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape8000_Mudball), kResScrb8001_Foreground, 0,
			ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_04000000_OVERLAY);
		manualLinkAfter(foreground, lowerAnchor);
	}
	if (rightAnchor) {
		ZmbFeature *foreground = loadScrbFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape8000_Mudball), kResScrb8008_FinalForegroundAlt, 0,
			ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_04000000_OVERLAY);
		manualLinkAfter(foreground, rightAnchor);
	}

	relinkSolidLauncherDropTargetHelpers();
}

void ZoombiniPuzzleMaze::relinkSolidLauncherDropTargetHelpers() {
	// Every launcher in @ref ZoombiniPuzzleMaze::_launcherFeatures registers after its
	// seat's drop-target helper in @ref ZoombiniPuzzleMaze::_launcherDropTargetFeatures, so the
	// launcher's dormant frame repaints over that helper on every pass.
	// For @ref kResScrb9000_PathBase and @ref kResScrb9001_PathShadow that is harmless: their
	// dormant badge is a thin rim, and the helper's denser lit fill still shows through it.
	// @ref kResScrb9003_PathCreature is the exception. Its dormant badge repeats the seat marker
	// at full coverage and strictly contains the helper's lit frames, so the helper is erased
	// before it reaches the screen and the seat reads as a dead drop target.
	//
	// This relink is an intentional difference from the original, which has none.
	// @ref kResScrb9003_PathCreature belongs only to the two lower-right seats, and no layout the
	// original selects activates either of them, so the original never reaches this combination
	// and there is no authentic behavior to preserve.
	// The helper moves to just behind its own launcher and stays in front of the 8000-group
	// foreground, which leaves every other painter at its established depth.
	for (int slotIdx = 0; slotIdx < 14; slotIdx++) {
		if (kCreatureScrbTable[slotIdx] != kResScrb9003_PathCreature)
			continue;
		ZmbFeature *helper = _launcherDropTargetFeatures[slotIdx];
		ZmbFeature *launcher = _launcherFeatures[slotIdx];
		if (helper && launcher)
			manualLinkAfter(helper, launcher);
	}
}

void ZoombiniPuzzleMaze::loadRegsCoordinateTables() {
	// Maze uses ID_REGS for several page-specific tables with different schemas.
	// The selected 16600-series layout table is decoded by loadAndParseRegsData().
	// The resources below provide screen anchors, registration values, and collision lookup data.
	// REGS 16000 is not an X/Y registration pair: it is 169 packed POINTS.
	memset(_gridCellPos, 0, sizeof(_gridCellPos));
	Common::SeekableReadStream *gridStream = _vm->getResource(ID_REGS, ZmbResource(ZmbResource::kPage, kResRegs16000_Maze));
	static constexpr uint32 kGridPointCount = kGridRows * kGridCols;
	static constexpr uint32 kGridPointTableSize = kGridPointCount * 2 * sizeof(int16);
	if (ZmbResource::hasSize(gridStream, kGridPointTableSize, kGridPointTableSize)) {
		Common::Point parsedGridCellPos[kGridRows][kGridCols];
		for (int row = 0; row < kGridRows; row++) {
			for (int col = 0; col < kGridCols; col++) {
				const int16 x = gridStream->readSint16BE();
				const int16 y = gridStream->readSint16BE();
				parsedGridCellPos[row][col] = Common::Point(x, y);
			}
		}
		memcpy(_gridCellPos, parsedGridCellPos, sizeof(parsedGridCellPos));
	} else {
		delete gridStream;
		error("maze: malformed required REGS %d coordinate table", kResRegs16000_Maze);
		return;
	}
	delete gridStream;

	const auto readRegsTable = [&](int16 resourceId, uint32 expectedCount, Common::Array<int16> &values) -> bool {
		values.clear();
		Common::SeekableReadStream *stream = _vm->getResource(ID_REGS, ZmbResource(ZmbResource::kPage, resourceId));
		const uint32 expectedSize = expectedCount * sizeof(int16);
		if (!ZmbResource::hasSize(stream, expectedSize, expectedSize)) {
			delete stream;
			error("maze: malformed required REGS %d table", resourceId);
			return false;
		}

		values.resize(expectedCount);
		for (uint32 valueIdx = 0; valueIdx < expectedCount; valueIdx++)
			values[valueIdx] = stream->readSint16BE();
		delete stream;
		return true;
	};

	// REGS 16501 selects the direction-dependent companion SCRB used when two
	// bubbles collide.
	// Index this table with a 12-word row stride.
	readRegsTable(kResRegs16501_Collision, kGridRows * kGridCols, _collisionOverlayRegs);

	// REGS 18000/18001 are the X/Y registration offsets for tBMP 5100.
	static constexpr uint32 kShapeRegistrationCount = 199;
	const bool hasShapeX = readRegsTable(kResRegs18000_Entrance, kShapeRegistrationCount, _shapeRegsX);
	const bool hasShapeY = readRegsTable(kResRegs18000_Entrance + 1, kShapeRegistrationCount, _shapeRegsY);
	if (!hasShapeX || !hasShapeY) {
		_shapeRegsX.clear();
		_shapeRegsY.clear();
	}

	// REGS 17000/17001 register the bubble SCRB's first shape around the Snoid anchor.
	static constexpr uint32 kBubbleRegistrationCount = 67;
	const bool hasBubbleX = readRegsTable(kResRegs17000_Exit, kBubbleRegistrationCount, _bubbleShapeRegsX);
	const bool hasBubbleY = readRegsTable(kResRegs17000_Exit + 1, kBubbleRegistrationCount, _bubbleShapeRegsY);
	if (!hasBubbleX || !hasBubbleY) {
		_bubbleShapeRegsX.clear();
		_bubbleShapeRegsY.clear();
	}
}

// =================================================================
// Grid initialization
// Master Dispatcher
// =================================================================

void ZoombiniPuzzleMaze::clearTraitSelectionTables() {
	for (int16 recordIdx = 0; recordIdx < 20; recordIdx++) {
		_uniqueCheckTraits[recordIdx] = ZmbTrait();
		_committedTraits[recordIdx] = ZmbTrait();
	}
}

void ZoombiniPuzzleMaze::initGridAndSelectPaths() {
	ZoombiniGameState::MazeLayoutVariantState &variantState = _vm->_state->getMazeLayoutVariantState();

	if (_gridInitialized)
		return;
	_gridInitialized = true;

	// Clear grid state
	memset(_cellTypes, 0, sizeof(_cellTypes));
	memset(_cellStateIdx, 0xFF, sizeof(_cellStateIdx));
	memset(_cellTraitKind, 0, sizeof(_cellTraitKind));
	memset(_cellTraitValue, 0, sizeof(_cellTraitValue));
	memset(_nodeDirFlags, 0, sizeof(_nodeDirFlags));
	memset(_nodeDirection, 0, sizeof(_nodeDirection));
	memset(_nodeCycleFlag, 0, sizeof(_nodeCycleFlag));
	memset(_collisionReservationCount, 0, sizeof(_collisionReservationCount));
	memset(_collisionReservationRunnerIdx, 0xFF, sizeof(_collisionReservationRunnerIdx));

	// Place 18 base nodes at fixed grid positions
	generateBaseNodes();

	// Copy and shuffle the seven colored shape banks. Group 1 always uses 31;
	// groups 2-8 use a permutation of 52..178.
	for (int i = 0; i < 11; i++)
		_shuffledPathPool[i] = kStaticPathPool[i];
	memset(_waveGroupShapeBase, 0, sizeof(_waveGroupShapeBase));
	_waveGroupShapeBase[1] = kStaticPathPool[1];

	int16 lastAvailablePathPoolIdx = 8;
	for (int16 group = 2; group < 9; group++) {
		const int16 randIdx = _vm->_rnd->getRandomNumber(2, lastAvailablePathPoolIdx);
		_waveGroupShapeBase[group] = _shuffledPathPool[randIdx];
		for (int16 pathPoolIdx = randIdx; pathPoolIdx < lastAvailablePathPoolIdx + 1; pathPoolIdx++)
			_shuffledPathPool[pathPoolIdx] = _shuffledPathPool[pathPoolIdx + 1];
		lastAvailablePathPoolIdx -= 1;
	}

	if (_vm->isColorBlindModeEnabled()) {
		// Color assist keeps the active groups in red/orange/yellow/cyan/purple families.
		// Green and magenta are omitted from the active combination, while the
		// purple bank is remapped to the Zoombini dark-blue family during drawing.
		// Derive this remap from the shuffled banks without consuming another draw.
		static constexpr int16 kColorAssistPathPool[5] = {52, 73, 94, 136, 157};
		bool activeGroups[9] = {};
		for (int16 runner = 0; runner < _layoutCellRecordCount; runner++) {
			int16 baseOff = 10 + 10 * runner;
			if (static_cast<int16>(_layoutRegsWords.size()) <= baseOff + 3)
				break;
			int16 waveGroup = _layoutRegsWords[baseOff + 3];
			if (1 < waveGroup && waveGroup <= 8)
				activeGroups[waveGroup] = true;
		}

		int16 availablePathPool[5] = {};
		for (int16 i = 0; i < 5; i++)
			availablePathPool[i] = kColorAssistPathPool[i];
		int16 availablePathCount = 5;
		for (int16 group = 2; group < 9; group++) {
			if (activeGroups[group] && 0 < availablePathCount) {
				const int16 originalColorIdx = (_waveGroupShapeBase[group] - 52) / 21;
				const int16 colorIdx = originalColorIdx % availablePathCount;
				_waveGroupShapeBase[group] = availablePathPool[colorIdx];
				for (int16 pathPoolIdx = colorIdx; pathPoolIdx + 1 < availablePathCount; pathPoolIdx++)
					availablePathPool[pathPoolIdx] = availablePathPool[pathPoolIdx + 1];
				availablePathCount -= 1;
			} else {
				_waveGroupShapeBase[group] = kColorAssistPathPool[(group - 2) % 5];
			}
		}
	}

	// Initialize path selection state
	_selectedPathSlotCount = 0;
	_nextPathSlotAssignmentIdx = 0;
	_reachablePathSlotCount = 0;
	_freePathSlotCount = 0;
	_committedTraitCount = 0;
	memset(_selectedPathSlots, 0, sizeof(_selectedPathSlots));
	for (int16 snoidIdx = 0; snoidIdx < 20; snoidIdx++)
		_pathCandidateTraits[snoidIdx] = ZmbTrait();
	memset(_pathTraitMatchCounts, 0, sizeof(_pathTraitMatchCounts));
	clearTraitSelectionTables();

	// Dispatch path selection by layout level.
	switch (_layoutLevel) {
	case kMazeLayoutLevel1:
		buildZmbAssignmentAlt2();
		variantState._level1 += 1;
		if (1 < variantState._level1)
			variantState._level1 = 0;
		break;
	case kMazeLayoutLevel2:
		if (variantState._level2)
			selectPathSlots2();
		else
			buildZmbAssignmentAlt();
		variantState._level2 += 1;
		if (1 < variantState._level2)
			variantState._level2 = 0;
		break;
	case kMazeLayoutLevel3:
		if (variantState._level3)
			selectPathSlots();
		else
			selectPathSlots2();
		variantState._level3 += 1;
		if (1 < variantState._level3)
			variantState._level3 = 0;
		break;
	case kMazeLayoutLevel4:
		buildZmbAssignmentList();
		// The original engine advances by two, so the unused REGS 16607
		// layout is never selected. The compatibility option advances
		// through every variant to restore it to the rotation. An entry
		// through the restored REGS 16607 layout, whether reached through
		// the option or forced through the state console key, continues
		// to the next layout REGS 16608 instead of falling back into the
		// two-layout rotation.
		if (ConfMan.getBool(MohawkMetaEngine_Zoombini::kOptionMazeRestoreUnusedL4Layout) || variantState._level4 == 1)
			variantState._level4 += 1;
		else
			variantState._level4 += 2;
		if (2 < variantState._level4)
			variantState._level4 = 0;
		break;
	default: // This path should not be reached.

		selectPathSlots();
		break;
	}

	// Initialize page-owned grid cell states from the layout REGS.
	initGridCellStates();

	// The restored level-4 layout can be handed a pack it cannot deliver.
	// Retune one condition cell rather than leave the crossing unwinnable.
	repairRestoredLayoutConditionCell();

	_queueProcessingEnabled = true;
	debugC(2, MohawkEngine_Zoombini::kDebugPage02, "maze: initialized grid for layout level %d with %d path slots", static_cast<int>(_layoutLevel), _selectedPathSlotCount);

}

void ZoombiniPuzzleMaze::generateBaseNodes() {
	for (int i = 0; i < 18; i++) {
		int16 row = kBaseNodeCoords[i].x;
		int16 col = kBaseNodeCoords[i].y;
		if (0 <= row && row < kGridRows && 0 <= col && col < kGridCols) {
			_cellTypes[row][col] = kBaseNodeTypes[i];
		}
	}
}

// =================================================================
// SCRS animation table initialization
// =================================================================

void ZoombiniPuzzleMaze::initRunnerAnimTable(int16 runnerIdx) {
	// Initialize the SCRS animation lookup table on a runner based on feet trait.
	if (runnerIdx < 0 || kMaxRunners <= runnerIdx)
		return;

	MazeRunnerState &rs = _runnerStates[runnerIdx];
	int16 feet = rs.feetTrait;

	// Direction walking SCRS (4 directions)
	rs.scrsTable[0] = static_cast<int16>(feet + kResScrs15014_Direction0Base); // dir 0: decCol walk
	rs.scrsTable[1] = static_cast<int16>(feet + kResScrs15019_Direction1Base); // dir 1: incRow walk
	rs.scrsTable[2] = static_cast<int16>(feet + kResScrs15024_Direction2Base); // dir 2: incCol walk
	rs.scrsTable[3] = static_cast<int16>(feet + kResScrs15029_Direction3Base); // dir 3: decRow walk

	// Direction alt SCRS (4 directions)
	rs.scrsTable[4] = static_cast<int16>(feet + kResScrs15055_Direction0AltBase); // dir 0 alt
	rs.scrsTable[5] = static_cast<int16>(feet + kResScrs15060_Direction1AltBase); // dir 1 alt
	rs.scrsTable[6] = static_cast<int16>(feet + kResScrs15065_Direction2AltBase); // dir 2 alt
	rs.scrsTable[7] = static_cast<int16>(feet + kResScrs15070_Direction3AltBase); // dir 3 alt

	// Special SCRS
	rs.scrsTable[8] = static_cast<int16>(feet + kResScrs14999_IdleBase);      // idle
	rs.scrsTable[9] = static_cast<int16>(feet + kResScrs15004_Special1Base);  // special 1
	rs.scrsTable[10] = static_cast<int16>(feet + kResScrs15009_Special2Base); // special 2

	rs.scrsTable[11] = static_cast<uint16>(feet - 1); // feet index (0-based)
}

void ZoombiniPuzzleMaze::initAllRunnerAnimTables() {
	if (_animTablesInitialized)
		return;
	_animTablesInitialized = true;

	for (int16 i = 0; i < _runnerCount; i++) {
		initRunnerAnimTable(i);
	}
}

// =================================================================
// Path selection helpers
// =================================================================

void ZoombiniPuzzleMaze::collectZmbTraits() {
	// Clear the working trait records.
	for (int16 snoidIdx = 0; snoidIdx < _pageLoadedZmbCount; snoidIdx++)
		_pathCandidateTraits[snoidIdx] = ZmbTrait();

	// Collect traits from all loaded Zoombinis.
	for (int16 i = 0; i < _pageLoadedZmbCount; i++) {
		uint16 snoidId = 10000 + i;
		ZmbSnoid *snoid = getSnoid(snoidId);
		if (!snoid)
			continue;
		_pathCandidateTraits[i] = snoid->_trait;
	}
}

int16 ZoombiniPuzzleMaze::removeMatchingPathCandidatesAndRecount(int16 traitSlotIdx) {
	int16 remainingCandidateCount = 0;

	// Clear the per-slot match counts.
	for (int16 i = 0; i < 21; i++)
		_pathTraitMatchCounts[i] = 0;

	for (int16 snoidIdx = 0; snoidIdx < _pageLoadedZmbCount; snoidIdx++) {
		int16 candidateRetained = 1;
		// Phase 1: Check whether the Zoombini matches the requested trait slot, then remove it.
		for (int16 traitIdx = 0; traitIdx < 4; traitIdx++) {
			if (_pathCandidateTraits[snoidIdx][traitIdx] &&
				kTraitOffsets[traitIdx] + _pathCandidateTraits[snoidIdx][traitIdx] == traitSlotIdx && traitSlotIdx) {
				candidateRetained = 0;
				_pathCandidateTraits[snoidIdx] = ZmbTrait();
				break;
			}
		}
		// Phase 2: Tally the remaining candidate's traits into the per-slot counts.
		if (candidateRetained) {
			for (int16 traitIdx = 0; traitIdx < 4; traitIdx++) {
				if (_pathCandidateTraits[snoidIdx][traitIdx]) {
					int16 slot = kTraitOffsets[traitIdx] + _pathCandidateTraits[snoidIdx][traitIdx];
					_pathTraitMatchCounts[slot] += 1;
				} else {
					candidateRetained = 0;
					break;
				}
			}
		}
		if (candidateRetained)
			remainingCandidateCount += 1;
	}
	return remainingCandidateCount;
}

int16 ZoombiniPuzzleMaze::collectMatchingPathCandidates(int16 traitSlotIdx) {
	// Clear the working trait records.
	for (int16 snoidIdx = 0; snoidIdx < _pageLoadedZmbCount; snoidIdx++)
		_pathCandidateTraits[snoidIdx] = ZmbTrait();

	int16 matchCount = 0;

	// Re-collect traits and filter them by the requested trait.
	for (int16 snoidIdx = 0; snoidIdx < _pageLoadedZmbCount; snoidIdx++) {
		uint16 snoidId = 10000 + snoidIdx;
		ZmbSnoid *snoid = getSnoid(snoidId);
		if (!snoid)
			continue;

		const ZmbTrait &traits = snoid->_trait;

		bool found = false;
		for (int16 traitIdx = 0; traitIdx < 4; traitIdx++) {
			if (kTraitOffsets[traitIdx] + traits[traitIdx] == traitSlotIdx)
				found = true;
		}
		if (found) {
			_pathCandidateTraits[snoidIdx] = traits;
			matchCount += 1;
		}
	}
	return matchCount;
}

void ZoombiniPuzzleMaze::initConnectionTable() {
	memset(_connectionTable, 0, sizeof(_connectionTable));
	for (int16 i = 0; i < 21; i++) {
		if (!_pathTraitMatchCounts[i])
			_connectionTable[i] = i;
	}
}

int16 ZoombiniPuzzleMaze::rebuildReachabilityList() {
	memset(_reachablePathSlots, 0, sizeof(_reachablePathSlots));
	_reachablePathSlotCount = 0;

	for (int16 i = 0; i < 21; i++) {
		if (_connectionTable[i]) {
			_reachablePathSlotCount += 1;
			_reachablePathSlots[_reachablePathSlotCount] = _connectionTable[i];
		}
	}
	return _reachablePathSlotCount;
}

void ZoombiniPuzzleMaze::initAllSlotsReachable() {
	for (int16 i = 0; i < 21; i++)
		_reachablePathSlots[i] = i;
	_reachablePathSlotCount = 20;
}

int16 ZoombiniPuzzleMaze::findBestTraitSlotInRange(int16 minScore, int16 maxScore) {
	int16 bestSlot = 0;
	int16 bestScore = 0;
	for (int16 slot = 1; slot < 21; slot++) {
		if (minScore <= _pathTraitMatchCounts[slot] && _pathTraitMatchCounts[slot] <= maxScore && bestScore < _pathTraitMatchCounts[slot]) {
			bestScore = _pathTraitMatchCounts[slot];
			bestSlot = slot;
		}
	}
	return bestSlot;
}

int16 ZoombiniPuzzleMaze::findHighestScoredSlotInRange(int16 excludeSlot, int16 minScore, int16 maxScore) {
	// NOTE: searches 1..19 (not 1..20!) and requires same category
	int16 bestSlot = 0;
	int16 bestScore = 0;
	for (int16 slot = 1; slot < 20; slot++) {
		if (kSlotTraitCategories[slot] == kSlotTraitCategories[excludeSlot] && minScore <= _pathTraitMatchCounts[slot] && _pathTraitMatchCounts[slot] <= maxScore && bestScore < _pathTraitMatchCounts[slot]) {
			bestScore = _pathTraitMatchCounts[slot];
			bestSlot = slot;
		}
	}
	return bestSlot;
}

int16 ZoombiniPuzzleMaze::findHighestScoredSlot(int16 excludeSlot) {
	int16 bestSlot = 0;
	int16 bestScore = 0;
	for (int16 slot = 1; slot < 21; slot++) {
		if (bestScore < _pathTraitMatchCounts[slot] && slot != excludeSlot) {
			bestScore = _pathTraitMatchCounts[slot];
			bestSlot = slot;
		}
	}
	return bestSlot;
}

int16 ZoombiniPuzzleMaze::getTraitMatchCount(int16 traitSlotIdx) const {
	if (traitSlotIdx < 0 || 21 <= traitSlotIdx)
		return 0;
	return _pathTraitMatchCounts[traitSlotIdx];
}

int16 ZoombiniPuzzleMaze::countScoredPathSlots() {
	int16 count = 0;
	for (int16 slot = 1; slot < 21; slot++) {
		if (_pathTraitMatchCounts[slot])
			count += 1;
	}
	return count;
}

void ZoombiniPuzzleMaze::initFreePathSlotList() {
	memset(_freePathSlotList, 0, sizeof(_freePathSlotList));
	for (int16 i = 0; i < 21; i++) {
		if (!_pathTraitMatchCounts[i])
			_freePathSlotList[i] = i;
	}
}

int16 ZoombiniPuzzleMaze::countFreePathSlots() {
	_freePathSlotCount = 0;

	for (int16 i = 0; i < 21; i++) {
		if (_freePathSlotList[i])
			_freePathSlotCount += 1;
	}
	return _freePathSlotCount;
}

int16 ZoombiniPuzzleMaze::findBestNextSlot(int16 searchIdx) {
	int16 bestSlot = 0;
	int16 bestZmb = -1;
	int16 minScore = 21;

	for (int16 iterSlot = 1; iterSlot < 21; iterSlot++) {
		if (_pathTraitMatchCounts[iterSlot] <= 0 || minScore < _pathTraitMatchCounts[iterSlot] || searchIdx == iterSlot)
			continue;

		for (int16 zmbIdx = 0; zmbIdx < _pageLoadedZmbCount; zmbIdx++) {
			bool foundMatch = false;
			for (int16 traitJ = 0; traitJ < 4; traitJ++) {
				if (_pathCandidateTraits[zmbIdx][traitJ] <= 0)
					continue;
				if (kTraitOffsets[traitJ] + _pathCandidateTraits[zmbIdx][traitJ] != iterSlot)
					continue;

				// Check uniqueness against committed entries
				int16 isUnique = 1;
				for (int16 committedIdx = 0; committedIdx < 20 && _committedTraitCount < 4; committedIdx++) {
					if (0 < _uniqueCheckTraits[committedIdx][traitJ] && _uniqueCheckTraits[committedIdx][traitJ] == _pathCandidateTraits[zmbIdx][traitJ])
						isUnique = 0;
				}
				if (isUnique) {
					bestSlot = iterSlot;
					bestZmb = zmbIdx;
					minScore = _pathTraitMatchCounts[iterSlot];
					foundMatch = true;
				}
				break; // break traitJ loop
			}
			if (foundMatch)
				break; // break zmbIdx loop
		}
	}

	if (bestSlot) {
		// Clear scores
		memset(_pathTraitMatchCounts, 0, sizeof(_pathTraitMatchCounts));

		// Record winning zmb's traits
		if (_committedTraitCount < 20) {
			_uniqueCheckTraits[_committedTraitCount] = _pathCandidateTraits[bestZmb];
			_committedTraitCount += 1;
		}

		// Remove matching zmbs and rebuild scores
		for (int16 matchIdx = 0; matchIdx < _pageLoadedZmbCount; matchIdx++) {
			for (int16 traitIdx = 0; traitIdx < 4; traitIdx++) {
				if (_pathCandidateTraits[matchIdx][traitIdx] &&
					kTraitOffsets[traitIdx] + _pathCandidateTraits[matchIdx][traitIdx] == bestSlot) {
					_pathCandidateTraits[matchIdx] = ZmbTrait();
					break;
				}
			}
			if (0 < _pathCandidateTraits[matchIdx][0]) {
				for (int16 traitIdx = 0; traitIdx < 4; traitIdx++) {
					int16 traitSlotIdx = kTraitOffsets[traitIdx] + _pathCandidateTraits[matchIdx][traitIdx];
					_pathTraitMatchCounts[traitSlotIdx] += 1;
				}
			}
		}
	}
	return bestSlot;
}

int16 ZoombiniPuzzleMaze::commitBestTraitSlot(int16 maxThreshold, int16 minThreshold) {
	int16 bestSlot = 0;
	int16 bestScore = 0;

	for (int16 iterSlot = 1; iterSlot < 21; iterSlot++) {
		if (_pathTraitMatchCounts[iterSlot] <= bestScore || _pathTraitMatchCounts[iterSlot] < minThreshold || maxThreshold < _pathTraitMatchCounts[iterSlot])
			continue;

		for (int16 zmbIdx = 0; zmbIdx < _pageLoadedZmbCount; zmbIdx++) {
			bool foundMatch = false;
			for (int16 traitJ = 0; traitJ < 4; traitJ++) {
				if (_pathCandidateTraits[zmbIdx][traitJ] <= 0)
					continue;
				if (kTraitOffsets[traitJ] + _pathCandidateTraits[zmbIdx][traitJ] != iterSlot)
					continue;

				int16 isUnique = 1;
				for (int16 committedIdx = 0; committedIdx < 20; committedIdx++) {
					if (0 < _uniqueCheckTraits[committedIdx][traitJ] && _uniqueCheckTraits[committedIdx][traitJ] == _pathCandidateTraits[zmbIdx][traitJ])
						isUnique = 0;
				}
				if (isUnique) {
					bestSlot = iterSlot;
					bestScore = _pathTraitMatchCounts[iterSlot];
					foundMatch = true;
				}
				break;
			}
			if (foundMatch)
				break;
		}
	}

	if (bestSlot) {
		memset(_pathTraitMatchCounts, 0, sizeof(_pathTraitMatchCounts));

		for (int16 matchIdx = 0; matchIdx < _pageLoadedZmbCount; matchIdx++) {
			for (int16 traitIdx = 0; traitIdx < 4; traitIdx++) {
				if (_pathCandidateTraits[matchIdx][traitIdx] &&
					kTraitOffsets[traitIdx] + _pathCandidateTraits[matchIdx][traitIdx] == bestSlot) {
					// Record in BOTH arrays
					if (_committedTraitCount < 20) {
						_committedTraits[_committedTraitCount] = _pathCandidateTraits[matchIdx];
						_uniqueCheckTraits[_committedTraitCount] = _pathCandidateTraits[matchIdx];
						_committedTraitCount += 1;
					}
					// Clear zmb
					_pathCandidateTraits[matchIdx] = ZmbTrait();
					break;
				}
			}
			// Rebuild scores
			if (0 < _pathCandidateTraits[matchIdx][0]) {
				for (int16 traitIdx = 0; traitIdx < 4; traitIdx++) {
					int16 traitSlotIdx = kTraitOffsets[traitIdx] + _pathCandidateTraits[matchIdx][traitIdx];
					_pathTraitMatchCounts[traitSlotIdx] += 1;
				}
			}
		}
	}
	return bestSlot;
}

int16 ZoombiniPuzzleMaze::findAndCommitNextSlot(int16 slotIdx, int16 direction) {
	int16 bestSlot = 0;
	int16 bestZmb = -1;
	int16 bestTrait = 0;
	int16 minScore = 21;

	for (int16 iterSlot = 1; iterSlot < 21; iterSlot++) {
		if (_pathTraitMatchCounts[iterSlot] <= 0 || minScore < _pathTraitMatchCounts[iterSlot] || direction == iterSlot)
			continue;

		for (int16 zmbIdx = 0; zmbIdx < _pageLoadedZmbCount; zmbIdx++) {
			bool foundMatch = false;
			for (int16 traitJ = 0; traitJ < 4; traitJ++) {
				if (_pathCandidateTraits[zmbIdx][traitJ] <= 0)
					continue;
				if (kTraitOffsets[traitJ] + _pathCandidateTraits[zmbIdx][traitJ] != iterSlot)
					continue;

				// Check ALL 4 columns for uniqueness
				int16 isUnique = 1;
				for (int16 committedIdx = 0; committedIdx < 20 && _committedTraitCount < 3; committedIdx++) {
					if (0 < _uniqueCheckTraits[committedIdx][0] && _uniqueCheckTraits[committedIdx][0] == _pathCandidateTraits[zmbIdx][0])
						isUnique = 0;
					else if (0 < _uniqueCheckTraits[committedIdx][1] && _uniqueCheckTraits[committedIdx][1] == _pathCandidateTraits[zmbIdx][1])
						isUnique = 0;
					else if (0 < _uniqueCheckTraits[committedIdx][2] && _uniqueCheckTraits[committedIdx][2] == _pathCandidateTraits[zmbIdx][2])
						isUnique = 0;
					else if (0 < _uniqueCheckTraits[committedIdx][3] && _uniqueCheckTraits[committedIdx][3] == _pathCandidateTraits[zmbIdx][3])
						isUnique = 0;
				}
				if (isUnique) {
					bestSlot = iterSlot;
					bestZmb = zmbIdx;
					bestTrait = traitJ;
					minScore = _pathTraitMatchCounts[iterSlot];
					foundMatch = true;
				}
				break;
			}
			if (foundMatch)
				break;
		}
	}

	if (bestSlot) {
		if (_committedTraitCount < 4) {
			if (slotIdx) {
				// Mode 1: store ALL 4 traits
				_committedTraits[_committedTraitCount] = _pathCandidateTraits[bestZmb];
				_uniqueCheckTraits[_committedTraitCount] = _pathCandidateTraits[bestZmb];
			} else {
				// Mode 0: store only matching trait column
				_uniqueCheckTraits[_committedTraitCount] = ZmbTrait();
				_uniqueCheckTraits[_committedTraitCount][bestTrait] = _pathCandidateTraits[bestZmb][bestTrait];
				_committedTraits[_committedTraitCount] = _pathCandidateTraits[bestZmb];
			}
			_committedTraitCount += 1;
		}

		// Clear zmb and rebuild scores
		_pathCandidateTraits[bestZmb] = ZmbTrait();
		memset(_pathTraitMatchCounts, 0, sizeof(_pathTraitMatchCounts));
		for (int16 matchIdx = 0; matchIdx < _pageLoadedZmbCount; matchIdx++) {
			if (_pathCandidateTraits[matchIdx][0]) {
				for (int16 traitIdx = 0; traitIdx < 4; traitIdx++) {
					int16 traitSlotIdx = kTraitOffsets[traitIdx] + _pathCandidateTraits[matchIdx][traitIdx];
					_pathTraitMatchCounts[traitSlotIdx] += 1;
				}
			}
		}
	}
	return bestSlot;
}

int16 ZoombiniPuzzleMaze::findAndCommitNewTraitSlot(int16 maxThreshold, int16 minThreshold) {
	int16 bestSlot = 0;
	int16 bestScore = 0;

	for (int16 iterSlot = 1; iterSlot < 21; iterSlot++) {
		if (_pathTraitMatchCounts[iterSlot] <= bestScore || _pathTraitMatchCounts[iterSlot] < minThreshold || maxThreshold < _pathTraitMatchCounts[iterSlot])
			continue;

		for (int16 zmbIdx = 0; zmbIdx < _pageLoadedZmbCount; zmbIdx++) {
			bool foundMatch = false;
			for (int16 traitJ = 0; traitJ < 4; traitJ++) {
				if (_pathCandidateTraits[zmbIdx][traitJ] <= 0)
					continue;
				if (kTraitOffsets[traitJ] + _pathCandidateTraits[zmbIdx][traitJ] != iterSlot)
					continue;

				// Check all four trait columns against @ref ZoombiniPuzzleMaze::_committedTraits.
				int16 isUnique = 1;
				for (int16 committedIdx = 0; committedIdx < 20; committedIdx++) {
					if (0 < _committedTraits[committedIdx][0] && _committedTraits[committedIdx][0] == _pathCandidateTraits[zmbIdx][0])
						isUnique = 0;
					else if (0 < _committedTraits[committedIdx][1] && _committedTraits[committedIdx][1] == _pathCandidateTraits[zmbIdx][1])
						isUnique = 0;
					else if (0 < _committedTraits[committedIdx][2] && _committedTraits[committedIdx][2] == _pathCandidateTraits[zmbIdx][2])
						isUnique = 0;
					else if (0 < _committedTraits[committedIdx][3] && _committedTraits[committedIdx][3] == _pathCandidateTraits[zmbIdx][3])
						isUnique = 0;
				}
				if (isUnique) {
					bestSlot = iterSlot;
					bestScore = _pathTraitMatchCounts[iterSlot];
					foundMatch = true;
				}
				break;
			}
			if (foundMatch)
				break;
		}
	}

	if (bestSlot) {
		memset(_pathTraitMatchCounts, 0, sizeof(_pathTraitMatchCounts));

		for (int16 matchIdx = 0; matchIdx < _pageLoadedZmbCount; matchIdx++) {
			for (int16 traitIdx = 0; traitIdx < 4; traitIdx++) {
				if (_pathCandidateTraits[matchIdx][traitIdx] &&
					kTraitOffsets[traitIdx] + _pathCandidateTraits[matchIdx][traitIdx] == bestSlot) {
					// Record the selected traits only in @ref ZoombiniPuzzleMaze::_committedTraits.
					if (_committedTraitCount < 20) {
						_committedTraits[_committedTraitCount] = _pathCandidateTraits[matchIdx];
						_committedTraitCount += 1;
					}
					_pathCandidateTraits[matchIdx] = ZmbTrait();
					break;
				}
			}
			if (0 < _pathCandidateTraits[matchIdx][0]) {
				for (int16 traitIdx = 0; traitIdx < 4; traitIdx++) {
					int16 traitSlotIdx = kTraitOffsets[traitIdx] + _pathCandidateTraits[matchIdx][traitIdx];
					_pathTraitMatchCounts[traitSlotIdx] += 1;
				}
			}
		}
	}
	return bestSlot;
}

// =================================================================
// Path selection algorithms
// =================================================================

int16 ZoombiniPuzzleMaze::randomReachableSlot() {
	if (_reachablePathSlotCount <= 0)
		return 0;
	return _reachablePathSlots[_vm->_rnd->getRandomNumber(1, _reachablePathSlotCount)];
}

int16 ZoombiniPuzzleMaze::randomFreePathSlot() {
	// Select entry zero when no free slot is available; otherwise choose among the nonzero sparse entries.
	if (_freePathSlotCount <= 0)
		return _freePathSlotList[0];
	return _freePathSlotList[_vm->_rnd->getRandomNumber(1, _freePathSlotCount)];
}

void ZoombiniPuzzleMaze::buildZmbAssignmentAlt2() {
	_selectedPathSlotCount = 0;
	collectZmbTraits();
	int16 survivorCount = removeMatchingPathCandidatesAndRecount(0);
	initConnectionTable();
	if (!rebuildReachabilityList())
		initAllSlotsReachable();

	int16 firstSlot = 0;
	if (survivorCount < 3) {
		firstSlot = findBestTraitSlotInRange(1, 2);
	} else {
		firstSlot = findBestTraitSlotInRange(2, 5);
		if (!firstSlot)
			firstSlot = findBestTraitSlotInRange(6, 9);
		if (!firstSlot)
			firstSlot = findBestTraitSlotInRange(10, 16);
		if (!firstSlot)
			firstSlot = findBestTraitSlotInRange(1, 16);
	}
	if (!firstSlot)
		firstSlot = randomReachableSlot();

	_selectedPathSlots[_selectedPathSlotCount] = firstSlot;
	_selectedPathSlotCount += 1;
	_selectedPathSlots[_selectedPathSlotCount] = firstSlot;
	_selectedPathSlotCount += 1;
	if (_pathLayoutVariantIdx == 2) {
		_selectedPathSlots[_selectedPathSlotCount] = firstSlot;
		_selectedPathSlotCount += 1;
	}

	int16 survivorAfterFirst = removeMatchingPathCandidatesAndRecount(firstSlot);
	int16 firstFinalScore = 0;
	int16 secondFinalScore = 0;
	int16 firstFinalSlot = 0;
	int16 secondFinalSlot = 0;
	if (countScoredPathSlots() <= 4) {
		firstFinalSlot = randomReachableSlot();
		secondFinalSlot = randomReachableSlot();
	} else {
		int16 threshold = kPathSelectThresholds[survivorAfterFirst];
		firstFinalSlot = findBestTraitSlotInRange(1, threshold);
		if (firstFinalSlot)
			firstFinalScore = getTraitMatchCount(firstFinalSlot);
		else
			firstFinalSlot = randomReachableSlot();
		removeMatchingPathCandidatesAndRecount(firstFinalSlot);
		secondFinalSlot = findHighestScoredSlotInRange(firstFinalSlot, 1, threshold);
		if (secondFinalSlot)
			secondFinalScore = getTraitMatchCount(secondFinalSlot);
		else
			secondFinalSlot = randomReachableSlot();
	}
	if (secondFinalScore < firstFinalScore)
		SWAP(firstFinalSlot, secondFinalSlot);
	_selectedPathSlots[_selectedPathSlotCount] = firstFinalSlot;
	_selectedPathSlotCount += 1;
	_selectedPathSlots[_selectedPathSlotCount] = secondFinalSlot;
	_selectedPathSlotCount += 1;
}

void ZoombiniPuzzleMaze::buildZmbAssignmentAlt() {
	_selectedPathSlotCount = 0;
	collectZmbTraits();
	int16 survivorCount = removeMatchingPathCandidatesAndRecount(0);
	initConnectionTable();
	if (!rebuildReachabilityList())
		initAllSlotsReachable();

	int16 firstSlot = 0;
	if (survivorCount < 3) {
		firstSlot = findBestTraitSlotInRange(1, 2);
	} else {
		firstSlot = findBestTraitSlotInRange(2, 5);
		if (!firstSlot)
			firstSlot = findBestTraitSlotInRange(6, 9);
		if (!firstSlot)
			firstSlot = findBestTraitSlotInRange(10, 16);
		if (!firstSlot)
			firstSlot = findBestTraitSlotInRange(1, 16);
	}
	if (!firstSlot)
		firstSlot = randomReachableSlot();

	_selectedPathSlots[_selectedPathSlotCount] = firstSlot;
	_selectedPathSlotCount += 1;
	_selectedPathSlots[_selectedPathSlotCount] = firstSlot;
	_selectedPathSlotCount += 1;
	_selectedPathSlots[_selectedPathSlotCount] = firstSlot;
	_selectedPathSlotCount += 1;

	int16 survivorAfterFirst = removeMatchingPathCandidatesAndRecount(firstSlot);
	int16 firstFinalScore = 0;
	int16 secondFinalScore = 0;
	int16 firstFinalSlot = 0;
	int16 secondFinalSlot = 0;
	if (countScoredPathSlots() <= 4) {
		firstFinalSlot = findHighestScoredSlot(0);
		if (firstFinalSlot)
			firstFinalScore = getTraitMatchCount(firstFinalSlot);
		else
			firstFinalSlot = randomReachableSlot();
		secondFinalSlot = randomReachableSlot();
	} else {
		int16 threshold = kPathSelectThresholds[survivorAfterFirst];
		firstFinalSlot = findBestTraitSlotInRange(1, threshold);
		if (firstFinalSlot)
			firstFinalScore = getTraitMatchCount(firstFinalSlot);
		else
			firstFinalSlot = randomReachableSlot();
		removeMatchingPathCandidatesAndRecount(firstFinalSlot);
		secondFinalSlot = findHighestScoredSlotInRange(firstFinalSlot, 1, threshold);
		if (secondFinalSlot)
			secondFinalScore = getTraitMatchCount(secondFinalSlot);
		else
			secondFinalSlot = randomReachableSlot();
	}
	if (secondFinalScore < firstFinalScore)
		SWAP(firstFinalSlot, secondFinalSlot);
	_selectedPathSlots[_selectedPathSlotCount] = firstFinalSlot;
	_selectedPathSlotCount += 1;
	_selectedPathSlots[_selectedPathSlotCount] = secondFinalSlot;
	_selectedPathSlotCount += 1;
}

void ZoombiniPuzzleMaze::selectPathSlots2() {
	_selectedPathSlotCount = 0;
	_committedTraitCount = 0;
	clearTraitSelectionTables();

	collectZmbTraits();
	int16 survivorCount = removeMatchingPathCandidatesAndRecount(0);
	initConnectionTable();
	if (!rebuildReachabilityList())
		initAllSlotsReachable();

	int16 firstSlot = 0;
	if (survivorCount < 3) {
		firstSlot = findBestTraitSlotInRange(1, 2);
	} else {
		firstSlot = findBestTraitSlotInRange(2, 5);
		if (!firstSlot)
			firstSlot = findBestTraitSlotInRange(6, 9);
		if (!firstSlot)
			firstSlot = findBestTraitSlotInRange(10, 16);
		if (!firstSlot)
			firstSlot = findBestTraitSlotInRange(1, 16);
	}
	if (!firstSlot)
		firstSlot = randomReachableSlot();

	_selectedPathSlots[0] = firstSlot;
	_selectedPathSlots[1] = firstSlot;
	collectMatchingPathCandidates(firstSlot);
	_selectedPathSlotCount = 2;
	removeMatchingPathCandidatesAndRecount(0);
	initFreePathSlotList();
	countFreePathSlots();

	for (int16 phase = 0; phase < 3; phase++) {
		int16 midSlot = findBestNextSlot(firstSlot);
		if (!midSlot)
			midSlot = randomFreePathSlot();
		if (!midSlot)
			midSlot = randomReachableSlot();
		_selectedPathSlots[_selectedPathSlotCount] = midSlot;
		_selectedPathSlotCount += 1;
		if (_difficultyLevel == kPuzzleLevel2 && phase == 0) {
			_selectedPathSlots[_selectedPathSlotCount] = midSlot;
			_selectedPathSlotCount += 1;
		}
	}

	collectZmbTraits();
	int16 survivorAfterFirst = removeMatchingPathCandidatesAndRecount(firstSlot);
	if (countScoredPathSlots() <= 4) {
		if (_pathLayoutVariantIdx == 1) {
			int16 firstFinalSlot = findHighestScoredSlot(0);
			if (!firstFinalSlot)
				firstFinalSlot = randomReachableSlot();
			_selectedPathSlots[_selectedPathSlotCount] = firstFinalSlot;
			_selectedPathSlotCount += 1;
			_selectedPathSlots[_selectedPathSlotCount] = randomReachableSlot();
			_selectedPathSlotCount += 1;
		} else {
			int16 firstFinalSlot = randomReachableSlot();
			_selectedPathSlots[_selectedPathSlotCount] = firstFinalSlot;
			_selectedPathSlotCount += 1;
			int16 secondFinalSlot = findHighestScoredSlot(firstFinalSlot);
			if (!secondFinalSlot)
				secondFinalSlot = randomReachableSlot();
			_selectedPathSlots[_selectedPathSlotCount] = secondFinalSlot;
			_selectedPathSlotCount += 1;
		}
	} else {
		int16 threshold = kPathSelectThresholds[survivorAfterFirst];
		int16 firstFinalSlot = findBestTraitSlotInRange(1, threshold);
		int16 firstFinalScore = 0;
		if (firstFinalSlot)
			firstFinalScore = getTraitMatchCount(firstFinalSlot);
		else
			firstFinalSlot = randomReachableSlot();
		removeMatchingPathCandidatesAndRecount(firstFinalSlot);
		int16 secondFinalSlot = findHighestScoredSlotInRange(firstFinalSlot, 1, threshold);
		int16 secondFinalScore = 0;
		if (secondFinalSlot)
			secondFinalScore = getTraitMatchCount(secondFinalSlot);
		else
			secondFinalSlot = randomReachableSlot();
		if (secondFinalScore < firstFinalScore)
			SWAP(firstFinalSlot, secondFinalSlot);
		_selectedPathSlots[_selectedPathSlotCount] = firstFinalSlot;
		_selectedPathSlotCount += 1;
		_selectedPathSlots[_selectedPathSlotCount] = secondFinalSlot;
		_selectedPathSlotCount += 1;
	}
}

void ZoombiniPuzzleMaze::selectPathSlots() {
	_selectedPathSlotCount = 0;
	_committedTraitCount = 0;
	clearTraitSelectionTables();

	collectZmbTraits();
	int16 survivorCount = removeMatchingPathCandidatesAndRecount(0);
	initConnectionTable();
	if (!rebuildReachabilityList())
		initAllSlotsReachable();

	int16 bestNextSlot = findBestNextSlot(_selectedPathSlots[0]);
	int16 firstSlot = 0;
	if (survivorCount < 2) {
		firstSlot = commitBestTraitSlot(1, 1);
	} else {
		firstSlot = commitBestTraitSlot(4, 2);
		if (!firstSlot)
			firstSlot = commitBestTraitSlot(8, 5);
		if (!firstSlot)
			firstSlot = commitBestTraitSlot(12, 9);
		if (!firstSlot)
			firstSlot = commitBestTraitSlot(16, 1);
	}
	if (!firstSlot)
		firstSlot = randomReachableSlot();

	_selectedPathSlots[0] = firstSlot;
	_selectedPathSlots[1] = firstSlot;
	collectMatchingPathCandidates(firstSlot);
	_selectedPathSlotCount = 2;
	removeMatchingPathCandidatesAndRecount(0);
	initFreePathSlotList();
	countFreePathSlots();

	_selectedPathSlots[_selectedPathSlotCount] = bestNextSlot;
	_selectedPathSlotCount += 1;
	_selectedPathSlots[_selectedPathSlotCount] = bestNextSlot;
	_selectedPathSlotCount += 1;

	for (int16 phase = 0; phase < 2; phase++) {
		int16 midSlot = findBestNextSlot(firstSlot);
		if (!midSlot)
			midSlot = randomFreePathSlot();
		if (!midSlot)
			midSlot = randomReachableSlot();
		_selectedPathSlots[_selectedPathSlotCount] = midSlot;
		_selectedPathSlotCount += 1;
	}

	collectZmbTraits();
	removeMatchingPathCandidatesAndRecount(firstSlot);
	int16 survivorAfterBest = removeMatchingPathCandidatesAndRecount(bestNextSlot);
	initFreePathSlotList();
	countFreePathSlots();

	if (countScoredPathSlots() <= 4) {
		int16 firstFinalSlot = randomFreePathSlot();
		_selectedPathSlots[_selectedPathSlotCount] = firstFinalSlot;
		_selectedPathSlotCount += 1;
		int16 secondFinalSlot = findHighestScoredSlot(firstFinalSlot);
		if (!secondFinalSlot)
			secondFinalSlot = randomFreePathSlot();
		_selectedPathSlots[_selectedPathSlotCount] = secondFinalSlot;
		_selectedPathSlotCount += 1;
	} else {
		int16 threshold = kPathSelectThresholds[survivorAfterBest];
		int16 firstFinalSlot = findBestTraitSlotInRange(1, threshold);
		int16 firstFinalScore = 0;
		if (firstFinalSlot)
			firstFinalScore = getTraitMatchCount(firstFinalSlot);
		else {
			firstFinalSlot = randomFreePathSlot();
			if (!firstFinalSlot)
				firstFinalSlot = randomReachableSlot();
		}
		removeMatchingPathCandidatesAndRecount(firstFinalSlot);
		int16 secondFinalSlot = findHighestScoredSlotInRange(firstFinalSlot, 1, threshold);
		int16 secondFinalScore = 0;
		if (secondFinalSlot)
			secondFinalScore = getTraitMatchCount(secondFinalSlot);
		else {
			secondFinalSlot = randomFreePathSlot();
			if (!secondFinalSlot)
				secondFinalSlot = randomReachableSlot();
		}
		if (secondFinalScore < firstFinalScore)
			SWAP(firstFinalSlot, secondFinalSlot);
		_selectedPathSlots[_selectedPathSlotCount] = firstFinalSlot;
		_selectedPathSlotCount += 1;
		_selectedPathSlots[_selectedPathSlotCount] = secondFinalSlot;
		_selectedPathSlotCount += 1;
	}
}

void ZoombiniPuzzleMaze::buildZmbAssignmentList() {
	_selectedPathSlotCount = 0;
	_committedTraitCount = 0;
	clearTraitSelectionTables();

	collectZmbTraits();
	removeMatchingPathCandidatesAndRecount(0);
	initConnectionTable();
	if (!rebuildReachabilityList())
		initAllSlotsReachable();

	for (int16 phase = 0; phase < 3; phase++) {
		int16 slot = findAndCommitNextSlot(0, 0);
		if (!slot)
			slot = randomReachableSlot();
		_selectedPathSlots[_selectedPathSlotCount] = slot;
		_selectedPathSlotCount += 1;
	}

	int16 reservedSlot = findAndCommitNextSlot(0, 0);
	if (!reservedSlot)
		reservedSlot = randomReachableSlot();

	removeMatchingPathCandidatesAndRecount(_selectedPathSlots[0]);
	removeMatchingPathCandidatesAndRecount(_selectedPathSlots[1]);
	int16 survivorTarget = removeMatchingPathCandidatesAndRecount(_selectedPathSlots[2]);
	int16 targetScore = kScoreToLoopCount[survivorTarget];
	int16 progress = 0;
	int16 remaining = 0;
	for (; progress < targetScore; progress++) {
		if (!progress) {
			int16 slot = commitBestTraitSlot(1, 1);
			if (!slot)
				slot = _selectedPathSlots[_vm->_rnd->getRandomNumber(0, 2)];
			int16 matchCount = getTraitMatchCount(slot);
			removeMatchingPathCandidatesAndRecount(slot);
			_selectedPathSlots[_selectedPathSlotCount] = slot;
			_selectedPathSlotCount += 1;
			progress = matchCount;
			remaining = targetScore - matchCount;
		} else if (remaining) {
			int16 slot = commitBestTraitSlot(remaining, 1);
			if (!slot)
				slot = randomReachableSlot();
			int16 matchCount = getTraitMatchCount(slot);
			removeMatchingPathCandidatesAndRecount(slot);
			progress += matchCount;
			remaining = targetScore - progress;
		}
	}

	_selectedPathSlots[_selectedPathSlotCount] = reservedSlot;
	_selectedPathSlotCount += 1;

	collectZmbTraits();
	removeMatchingPathCandidatesAndRecount(0);
	initConnectionTable();
	bool hadReachableSlots = rebuildReachabilityList() != 0;
	if (!hadReachableSlots)
		initAllSlotsReachable();
	removeMatchingPathCandidatesAndRecount(reservedSlot);

	for (int16 phase = 0; phase < 2; phase++) {
		int16 slot = findAndCommitNewTraitSlot(3, 1);
		if (!slot)
			slot = hadReachableSlots ? randomReachableSlot() : _selectedPathSlots[0];
		removeMatchingPathCandidatesAndRecount(slot);
		_selectedPathSlots[_selectedPathSlotCount] = slot;
		_selectedPathSlotCount += 1;
	}

	_selectedPathSlots[_selectedPathSlotCount] = _vm->_rnd->getRandomNumber(1, 20);
	_selectedPathSlotCount += 1;
	_selectedPathSlots[_selectedPathSlotCount] = _vm->_rnd->getRandomNumber(1, 20);
	_selectedPathSlotCount += 1;
}

// =================================================================
// Grid runner initialization
// =================================================================

void ZoombiniPuzzleMaze::initGridCellStates() {
	_layoutCellReadIdx = 0;
	_cellStateCount = 0;
	memset(_waveGroupCellCounts, 0, sizeof(_waveGroupCellCounts));
	for (int16 i = 0; i < _layoutCellRecordCount && i < kMaxCellStates; i++) {
		registerGridCellState();
	}
}

void ZoombiniPuzzleMaze::registerGridCellState() {
	const uint16 visualFrameInterval = static_cast<uint16>(_vm->_rnd->getRandomNumber(20, 25));

	// Read one cell record from the selected 16600-series Maze layout resource.
	// Read ten words from REGS data per cell.
	// REGS layout per cell:
	//   [0]=cellType, [1]=row, [2]=col, [3]=waveGroup,
	//   [4]=dirFlag0, [5]=dirFlag1, [6]=dirFlag2, [7]=dirFlag3,
	//   [8]=direction, [9]=cycleFlag.
	if (static_cast<int16>(_layoutRegsWords.size()) / 10 <= _layoutCellReadIdx)
		return;

	int16 baseOff = 10 + 10 * _layoutCellReadIdx; // Skip 10-word header
	if (static_cast<int16>(_layoutRegsWords.size()) < baseOff + 10)
		return;

	const MazeCellType cellType = static_cast<MazeCellType>(_layoutRegsWords[baseOff + 0]);
	int16 row = _layoutRegsWords[baseOff + 1];
	int16 col = _layoutRegsWords[baseOff + 2];
	int16 waveGroup = _layoutRegsWords[baseOff + 3];
	const bool dirFlag0 = _layoutRegsWords[baseOff + 4] != 0;
	const bool dirFlag1 = _layoutRegsWords[baseOff + 5] != 0;
	const bool dirFlag2 = _layoutRegsWords[baseOff + 6] != 0;
	const bool dirFlag3 = _layoutRegsWords[baseOff + 7] != 0;
	int16 direction = _layoutRegsWords[baseOff + 8];
	const bool cycleFlag = _layoutRegsWords[baseOff + 9] != 0;

	_layoutCellReadIdx += 1;

	if (row < 0 || kGridRows <= row || col < 0 || kGridCols <= col)
		return;

	// Store in grid arrays
	_cellTypes[row][col] = cellType;
	if (kMaxCellStates <= _cellStateCount)
		return;

	int16 cellIdx = _cellStateCount;
	_cellStateCount += 1;
	_cellStateIdx[row][col] = cellIdx;

	// Store per-node direction data.
	_nodeDirFlags[row][col][0] = dirFlag0;
	_nodeDirFlags[row][col][1] = dirFlag1;
	_nodeDirFlags[row][col][2] = dirFlag2;
	_nodeDirFlags[row][col][3] = dirFlag3;
	_nodeDirection[row][col] = direction;
	_nodeCycleFlag[row][col] = cycleFlag;

	// Trait arrows consume their condition from the selected path slots.
	if (cellType == kMazeCellType02_TraitArrow) {
		if (_nextPathSlotAssignmentIdx < _selectedPathSlotCount) {
			int16 slotIdx = _selectedPathSlots[_nextPathSlotAssignmentIdx];
			if (0 <= slotIdx && slotIdx <= 20) {
				_cellTraitKind[row][col] = static_cast<int16>(kTraitSlotKind[slotIdx]) + 1;
				_cellTraitValue[row][col] = kTraitSlotValue[slotIdx]; // 1-5
			}
			_nextPathSlotAssignmentIdx += 1;
		}
	}

	MazeCellState &cell = _cellStates[cellIdx];
	cell = MazeCellState();
	cell.type = cellType;
	cell.row = row;
	cell.col = col;
	cell.waveGroup = (1 <= waveGroup && waveGroup <= 8) ? waveGroup : 1;
	cell.dirFlags[0] = dirFlag0;
	cell.dirFlags[1] = dirFlag1;
	cell.dirFlags[2] = dirFlag2;
	cell.dirFlags[3] = dirFlag3;
	cell.direction = direction;
	cell.cycleOnPass = cycleFlag;
	cell.traitCategory = _cellTraitKind[row][col];
	cell.traitValue = _cellTraitValue[row][col];
	cell.shapeBase = _waveGroupShapeBase[cell.waveGroup];
	cell.visualFrameInterval = visualFrameInterval;
	cell.nextVisualFrame = 0;

	int16 groupIdx = cell.waveGroup - 1;
	if (_waveGroupCellCounts[groupIdx] < kMaxCellStates) {
		_waveGroupCellIndices[groupIdx][_waveGroupCellCounts[groupIdx]] = cellIdx;
		_waveGroupCellCounts[groupIdx] += 1;
	}

}

ZoombiniPuzzleMaze::MazeCellState *ZoombiniPuzzleMaze::getCellState(int16 cellIdx) {
	if (cellIdx < 0 || _cellStateCount <= cellIdx)
		return nullptr;
	return &_cellStates[cellIdx];
}

const ZoombiniPuzzleMaze::MazeCellState *ZoombiniPuzzleMaze::getCellState(int16 cellIdx) const {
	if (cellIdx < 0 || _cellStateCount <= cellIdx)
		return nullptr;
	return &_cellStates[cellIdx];
}

int16 ZoombiniPuzzleMaze::getCellStateIndex(int16 row, int16 col) const {
	if (row < 0 || kGridRows <= row || col < 0 || kGridCols <= col)
		return -1;
	return _cellStateIdx[row][col];
}

void ZoombiniPuzzleMaze::drawCellShape(int16 shapeId, const Common::Point &cellPos) {
	if (shapeId <= 0 || static_cast<int16>(_shapeRegsX.size()) <= shapeId ||
		static_cast<int16>(_shapeRegsY.size()) <= shapeId)
		return;

	// tBMP 5100 sub-images 020-024 are the five nose-condition indicators.
	// drawShape() uses 1-based shape IDs, so these are shapes 21-25 here.
	const bool isNoseConditionShape = 21 <= shapeId && shapeId <= 25;
	ZoombiniGraphics::PaletteRemapMode remap;
	if (_vm->isColorBlindModeEnabled()) {
		if (isNoseConditionShape)
			remap = ZoombiniGraphics::kPaletteRemapNoseNet;
		else
			remap = ZoombiniGraphics::kPaletteRemapMazePurple;
	} else {
		remap = ZoombiniGraphics::kPaletteRemapNone;
	}
	// cellPos comes from REGS 16000; these shape registrations come from
	// REGS 18000/18001 and align the selected tBMP 5100 sub-image to that anchor.
	_vm->_gfx->drawShape(ZoombiniGraphics::kShapeScreen,
						 ZmbResource(ZmbResource::kPage, kResBitmapShape5100_Creature), static_cast<uint16>(shapeId),
						 Common::Point(cellPos.x - _shapeRegsX[shapeId], cellPos.y - _shapeRegsY[shapeId]), false, remap);
}

ZmbRenderResult ZoombiniPuzzleMaze::renderGridCells(ZmbFeature *feature) {
	if (!feature->isRenderActivated())
		return ZmbRenderResult::kSkipped;

	uint32 now = getCurrentFrameCounter();
	for (int16 cellIdx = 0; cellIdx < _cellStateCount; cellIdx++) {
		MazeCellState &cell = _cellStates[cellIdx];
		if (cell.nextVisualFrame <= now) {
			if (cell.type == kMazeCellType01_Whirlpool) {
				cell.direction += 1;
				if (3 < cell.direction)
					cell.direction = 0;
			} else if (cell.type == kMazeCellType05_ColoredSticky) {
				cell.direction += 1;
				if (5 < cell.direction)
					cell.direction = 0;
			}
			if (cell.activationActive) {
				cell.activationFrame += 1;
				if (3 < cell.activationFrame) {
					cell.activationFrame = 0;
					cell.activationActive = false;
				}
			}
			cell.nextVisualFrame = now + cell.visualFrameInterval;
		}

		const Common::Point &pos = _gridCellPos[cell.row][cell.col];
		switch (cell.type) {
		case kMazeCellType01_Whirlpool:
			drawCellShape(cell.direction + 2, pos);
			break;
		case kMazeCellType02_TraitArrow:
		case kMazeCellType03_FixedArrow:
		case kMazeCellType04_ColoredArrow:
			drawCellShape(cell.shapeBase, pos);
			for (int16 dir = 0; dir < 4; dir++) {
				if (cell.dirFlags[dir])
					drawCellShape(cell.shapeBase + dir + 1, pos);
			}
			drawCellShape(cell.shapeBase + cell.direction + 5, pos);
			if (cell.traitCategory)
				drawCellShape(cell.traitValue + 5 * cell.traitCategory + 5, pos);
			break;
		case kMazeCellType05_ColoredSticky:
			drawCellShape(cell.shapeBase + cell.direction + 9, pos);
			break;
		case kMazeCellType06_ColoredSwitch:
			drawCellShape(cell.shapeBase + 19, pos);
			if (cell.activationActive)
				drawCellShape(cell.shapeBase + cell.activationFrame + 15, pos);
			break;
		case kMazeCellType07_Unused:
			drawCellShape(cell.shapeBase + 20, pos);
			if (cell.activationActive)
				drawCellShape(cell.shapeBase + cell.activationFrame + 15, pos);
			break;
		default:
			break;
		}
	}

	return ZmbRenderResult::kRendered;
}

void ZoombiniPuzzleMaze::adjustBubbleHotspotPosition(ZmbFeature *feature,
													 ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)feature;
	(void)hsGroup;

	if (hotspots.empty()) {
		error("maze: malformed required bubble SCRB hotspot table");
		return;
	}

	ZmbHotspot &hotspot = hotspots[0];
	const int16 shapeIdx = hotspot._shapeIdx;
	if (shapeIdx < 0 || static_cast<int16>(_bubbleShapeRegsX.size()) <= shapeIdx ||
		static_cast<int16>(_bubbleShapeRegsY.size()) <= shapeIdx) {
		error("maze: bubble SCRB references invalid shape %d", shapeIdx);
		return;
	}

	hotspot._x -= _bubbleShapeRegsX[shapeIdx];
	hotspot._y += 35 - _bubbleShapeRegsY[shapeIdx];
}

void ZoombiniPuzzleMaze::adjustExitBubbleHotspotPosition(ZmbFeature *feature,
														 ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)feature;
	(void)hsGroup;

	if (hotspots.empty()) {
		error("maze: malformed required exit-bubble SCRB hotspot table");
		return;
	}

	ZmbHotspot &hotspot = hotspots[0];
	const int16 shapeIdx = hotspot._shapeIdx;
	if (shapeIdx < 0 || static_cast<int16>(_bubbleShapeRegsX.size()) <= shapeIdx ||
		static_cast<int16>(_bubbleShapeRegsY.size()) <= shapeIdx) {
		error("maze: exit-bubble SCRB references invalid shape %d", shapeIdx);
		return;
	}

	hotspot._x -= _bubbleShapeRegsX[shapeIdx];
	hotspot._y += 25 - _bubbleShapeRegsY[shapeIdx];
}

// =================================================================
// Helpers
// =================================================================

bool ZoombiniPuzzleMaze::decodeMazeGridDirection(int16 direction, ZmbGridDirection &gridDirection) {
	switch (direction) {
	case kMazeDirection00_West:
		gridDirection = kZmbGridLeft;
		return true;
	case kMazeDirection01_South:
		gridDirection = kZmbGridDown;
		return true;
	case kMazeDirection02_East:
		gridDirection = kZmbGridRight;
		return true;
	case kMazeDirection03_North:
		gridDirection = kZmbGridUp;
		return true;
	default:
		return false;
	}
}

byte ZoombiniPuzzleMaze::getTraitByCategory(const ZmbTrait &trait, int16 category) {
	if (category < 1 || 4 < category)
		return 0;
	return trait.getTraitValue(ZmbTrait::traitKindFromIndex(category - 1));
}

ZoombiniPuzzleMaze::MazeRunnerState *ZoombiniPuzzleMaze::getRunnerState(int16 runnerIdx) {
	if (runnerIdx < 0 || kMaxRunners <= runnerIdx)
		return nullptr;
	return &_runnerStates[runnerIdx];
}

int16 ZoombiniPuzzleMaze::findRunnerBySnoidId(uint16 snoidId) const {
	for (int16 i = 0; i < _runnerCount; i++) {
		if (_runnerSnoidIds[i] == snoidId)
			return i;
	}
	return -1;
}

int16 ZoombiniPuzzleMaze::findRunnerByFeatureId(uint16 featureId) const {
	for (int16 i = 0; i < _runnerCount; i++) {
		if (_runnerSnoidIds[i] == featureId)
			return i;
	}
	return -1;
}

int16 ZoombiniPuzzleMaze::findSeatByFeature(const ZmbFeature *feature) const {
	// Associate only the primary launcher with a seat.
	// The companion runner is visual-only and must not launch the assigned Snoid a second time.
	for (int16 i = 0; i < 14; i++) {
		if (_launcherFeatures[i] == feature)
			return i;
	}
	return -1;
}

void ZoombiniPuzzleMaze::onSnoidWalkCompleted(ZmbSnoid *snoid) {
	if (!snoid)
		return;

	const int16 runnerIdx = findRunnerBySnoidId(snoid->getId());
	if (runnerIdx < 0 || _runnerCount <= runnerIdx)
		return;

	const MazeCornerGroup cornerGroup = _runnerStates[runnerIdx].cornerGroup;
	// Upper-left uses the LOOP_ANIM exact-flag group, while upper-right uses LOOP_ANIM | OVERLAY.
	// The lower-left and level-4 lower-right groups are bare Snoids already handled by positional Z-sorting.
	if (cornerGroup == kUpperLeft01)
		_upperLeftArrivalDepthReorderPending = true;
	else if (cornerGroup == kUpperRight03)
		_upperRightArrivalDepthReorderPending = true;
}

// =================================================================
// Animation event dispatch
// =================================================================

void ZoombiniPuzzleMaze::onFeatureAnimEvent(ZmbFeature *feature, int16 eventCode) {
	const bool isSnoid = feature->hasFlag(ZmbFeature::FLAG_00000001_TYPE_SNOID);
	if (isSnoid) {
		int16 runnerIdx = findRunnerByFeatureId(feature->getId());
		if (runnerIdx < 0)
			return;
		MazeRunnerState &rs = _runnerStates[runnerIdx];
		// Event 92 releases rejected or collided runners before their hide-on-complete event.
		// Waiting longer leaves the final bubble frame visible after a vortex rejection.
		if (eventCode == kGridEntityEventCode092_ReleaseRejectedRunner && (rs.rejecting || rs.collided)) {
			// A collision only reaches this event through the pop script that the
			// companion overlay starts. The merged-bubble phase keeps its visuals.
			if (rs.collided && !rs.collisionPopStarted)
				return;
			releaseRunnerTraversalVisuals(runnerIdx);
			return;
		} else if ((eventCode == kGridEntityEventCode091_FinishBoundaryExit || eventCode == kGridEntityEventCode092_ReleaseRejectedRunner) &&
				   rs.boundaryExit && rs.exitStage == 2) {
			// Events 91 and 92 remove the traversal bubble and reservation.
			// Successful upper exits use event 91 to queue the feet-specific 15091-15095 celebration before seating.
			releaseRunnerTraversalVisuals(runnerIdx);
			if (eventCode == kGridEntityEventCode091_FinishBoundaryExit && rs.cornerGroup == kUpperRight03) {
				rs.exitStage = 3;
				if (_acceptedCelebrationQueueSize < kMaxQueueSize) {
					_acceptedCelebrationQueue[_acceptedCelebrationQueueSize] = runnerIdx;
					_acceptedCelebrationQueueSize += 1;
				}
			} else if (rs.cornerGroup != kUpperRight03) {
				if (_exitCompletionQueueSize < kMaxQueueSize) {
					_exitCompletionQueue[_exitCompletionQueueSize] = runnerIdx;
					_exitCompletionQueueSize += 1;
				}
			}
			return;
		} else if (eventCode == kAnimEventM1_End) {
			// The runner SCRS reached its terminal marker; finish or chain the active exit sequence.
			if (feature->getScriptSoundPolicy() == ZmbFeature::ScriptSoundPolicy::kForcedPriorityQueue) {
				feature->setScriptSoundPolicy(ZmbFeature::ScriptSoundPolicy::kPriorityQueue);
			}
			if (rs.boundaryExit && rs.exitStage == 1) {
				ZmbSnoid *snoid = static_cast<ZmbSnoid *>(feature);
				if (snoid->startScrsPlayback(ZmbResource(ZmbResource::kPage, kResScrs14007_RejectAlt), ZmbScrsCompletionMode::kReturnToIdle,
											 resolveScrsRejectState(kResScrs14007_RejectAlt))) {
					rs.exitStage = 2;
					return;
				}
			}
			// The merged-bubble script is shorter than its bubble and companion
			// overlay. Its end is not the end of the collision; only the pop script
			// that the companion overlay starts finishes the runner.
			if (rs.collided && !rs.collisionPopStarted)
				return;
			if (rs.rejecting || rs.boundaryExit || rs.collided)
				finishRunnerExit(runnerIdx);
			return;
		} else if (eventCode == kGridEntityEventCode000_ToggleFacing) {
			ZmbSnoid *snoid = static_cast<ZmbSnoid *>(feature);
			snoid->setFacingLeft(!snoid->isFacingLeft());
			applyPendingBodyArrangement(*snoid);
			return;
		} else if (handleBodyArrangementScriptEvent(*static_cast<ZmbSnoid *>(feature), eventCode)) {
			return;
		}

		switch (eventCode) {
		case kGridEntityEventCode020_CollisionReservePhase0:
		case kGridEntityEventCode030_CollisionReservePhase1:
		case kGridEntityEventCode040_CollisionReservePhase2:
		case kGridEntityEventCode050_CollisionReservePhase3:
		case kGridEntityEventCode021_CollisionAdvancePhase0:
		case kGridEntityEventCode031_CollisionAdvancePhase1:
		case kGridEntityEventCode041_CollisionAdvancePhase2:
		case kGridEntityEventCode051_CollisionAdvancePhase3:
		case kGridEntityEventCode061_CollisionAdvancePhase4:
			// Collision SCRS markers reserve a grid cell or queue its cell-arrival dispatch.
			// The helper distinguishes reservation phases from the later cell-arrival phases.
			handleTraversalGridEvent(eventCode, runnerIdx);
			break;
		default:
			break;
		}
		return;
	}

	if (handleCollisionOverlayEvent(feature, eventCode))
		return;

	switch (eventCode) {
	case kLauncherEventCode050_LoadCreatureCompanion:
	case kLauncherEventCode061_StartDirectTraversal:
	case kLauncherEventCode062_StartNormalTraversal:
	case kLauncherEventCode064_QueueTraversalColumnRelink:
	case kLauncherEventCode065_StartInitialTraversal:
	case kLauncherEventCode066_FinishLauncher:
	case kLauncherEventCode071_StartTableTraversal:
	case kLauncherEventCode072_StartDirectTraversal:
	case kLauncherEventCode074_QueueTraversalColumnRelinkAlt:
	case kLauncherEventCode075_StartInitialTraversalAlt:
	case kLauncherEventCode076_FinishLauncherAlt:
	case kLauncherEventCode081_StartTableTraversalAlt:
	case kLauncherEventCode082_StartDirectTraversalAlt:
	case kLauncherEventCode084_QueueTraversalColumnRelinkAlt2:
	case kLauncherEventCode085_StartInitialTraversalAlt:
	case kLauncherEventCode086_FinishLauncherAlt2:
		// ASCII markers from a creature launcher select the companion SCRB or the first traversal SCRS.
		// All of these events are consumed by the seat-specific SCRB dispatcher below.
		processScrbAnimEvent(feature, eventCode);
		break;
	default:
		break;
	}
}

void ZoombiniPuzzleMaze::processScrbAnimEvent(ZmbFeature *feature, int16 eventCode) {
	// Launch SCRBs own the initial bubble-maker sequence;
	// their events hand the placed Snoid into its first grid traversal SCRS.
	int16 seatIdx = findSeatByFeature(feature);
	if (seatIdx < 0)
		return;
	switch (eventCode) {
	case kLauncherEventCode066_FinishLauncher:
	case kLauncherEventCode076_FinishLauncherAlt:
	case kLauncherEventCode086_FinishLauncherAlt2: {
		// Clear a launcher directly from its node index without requiring the Snoid to remain in its pre-launch state.
		const int16 assignedRunnerIdx = _launcherRunnerIndices[seatIdx];
		if (0 <= assignedRunnerIdx && assignedRunnerIdx < _runnerCount) {
			const int16 drawSlot = findDrawOnRegSlotByOccupant(_runnerSnoidIds[assignedRunnerIdx]);
			if (0 <= drawSlot)
				clearDrawOnRegOccupant(drawSlot);
		}
		_launcherOccupancyMarkers[seatIdx] = 0;
		_launcherRunnerIndices[seatIdx] = -1;
		return;
	}
	default:
		break;
	}

	int16 runnerIdx = _launcherRunnerIndices[seatIdx];
	if (runnerIdx < 0)
		return;

	MazeRunnerState &rs = _runnerStates[runnerIdx];
	ZmbSnoid *snoid = getSnoid(_runnerSnoidIds[runnerIdx]);
	if (!snoid)
		return;

	switch (eventCode) {
	case kLauncherEventCode050_LoadCreatureCompanion: {
		// Load the creature companion SCRB for this launch seat.
		const int16 typeId = kCreatureTypeId[seatIdx];
		ZmbFeature *companion = _launcherCompanionFeatures[typeId];
		if (companion)
			loadScrbOntoFeature(companion, kCreatureTypeScrbTable[typeId]);
		break;
	}
	case kLauncherEventCode062_StartNormalTraversal:
	case kLauncherEventCode072_StartDirectTraversal:
	case kLauncherEventCode082_StartDirectTraversalAlt: {
		// Start the seat's normal traversal SCRS after applying its direction-specific anchor offset.
		Common::Point pos = snoid->getPointLoc();
		if (rs.direction == kMazeDirection00_West)
			pos += Common::Point(3, 6);
		else if (rs.direction == kMazeDirection01_South)
			pos += Common::Point(7, 19);
		else if (rs.direction == kMazeDirection03_North)
			pos += Common::Point(12, 21);
		else
			pos += Common::Point(3, 6);
		snoid->setPointLoc(pos);
		int16 scrsId = static_cast<int16>(kResScrs14000_RejectBase + kSeatFlagValue[seatIdx]);
		ZmbScrsCompletionMode completionMode;
		if (eventCode != kLauncherEventCode062_StartNormalTraversal)
			completionMode = ZmbScrsCompletionMode::kHide;
		else
			completionMode = ZmbScrsCompletionMode::kReturnToIdle;
		snoid->startScrsPlayback(ZmbResource(ZmbResource::kPage, scrsId), completionMode, resolveScrsRejectState(scrsId));
		break;
	}
	case kLauncherEventCode061_StartDirectTraversal:
	case kLauncherEventCode071_StartTableTraversal:
	case kLauncherEventCode081_StartTableTraversalAlt: {
		// Start the table-selected traversal SCRS for this seat.
		int16 tableIdx = 8 + kSeatFlagValue[seatIdx];
		int16 scrsId = rs.scrsTable[tableIdx];
		snoid->startScrsPlayback(ZmbResource(ZmbResource::kPage, scrsId), ZmbScrsCompletionMode::kReturnToIdle, resolveScrsRejectState(scrsId));
		break;
	}
	case kLauncherEventCode064_QueueTraversalColumnRelink:
	case kLauncherEventCode074_QueueTraversalColumnRelinkAlt:
	case kLauncherEventCode084_QueueTraversalColumnRelinkAlt2:
		// Queue the Snoid for the launch-column relink performed after movement and collision relinks.
		if (_traversalColumnRelinkQueueSize < kMaxQueueSize) {
			_traversalColumnRelinkQueue[_traversalColumnRelinkQueueSize] = runnerIdx;
			_traversalColumnRelinkQueueSize += 1;
		}
		break;
	case kLauncherEventCode065_StartInitialTraversal:
		// Start the first traversal branch and hide the Snoid when its SCRS completes.
		startInitialTraversal(runnerIdx, ZmbScrsCompletionMode::kHide);
		break;
	case kLauncherEventCode075_StartInitialTraversalAlt:
	case kLauncherEventCode085_StartInitialTraversalAlt:
		// Start the alternate first traversal branch used by the other launcher orientation.
		startInitialTraversal(runnerIdx, ZmbScrsCompletionMode::kReturnToIdle);
		break;
	default:
		break;
	}
}

void ZoombiniPuzzleMaze::handleTraversalGridEvent(int16 eventCode, int16 runnerIdx) {
	if (runnerIdx < 0 || _runnerCount <= runnerIdx)
		return;

	MazeRunnerState &rs = _runnerStates[runnerIdx];
	if (!rs.placed || rs.rejecting || rs.boundaryExit || rs.collided)
		return;
	if (rs.row < 0 || kGridRows <= rs.row || rs.col < 0 || kGridCols <= rs.col)
		return;

	if (eventCode == kGridEntityEventCode020_CollisionReservePhase0 || eventCode == kGridEntityEventCode030_CollisionReservePhase1 ||
		eventCode == kGridEntityEventCode040_CollisionReservePhase2 || eventCode == kGridEntityEventCode050_CollisionReservePhase3) {
		ZmbSnoid *snoid = getSnoid(_runnerSnoidIds[runnerIdx]);
		// Every movement SCRS repeats its collision event on frames 1 through 5.
		// Reserve the cell only at group frame index 3.
		if (!snoid || snoid->getLastFrameIdx() != 3)
			return;

		int16 &count = _collisionReservationCount[rs.row][rs.col];
		int16 &firstRunner = _collisionReservationRunnerIdx[rs.row][rs.col];
		count += 1;
		if (count == 1) {
			firstRunner = runnerIdx;
		} else if (count == 2) {
			if (_collisionPairQueueSize + 2 <= kMaxCrossQueueSize) {
				_collisionPairQueue[_collisionPairQueueSize] = firstRunner;
				_collisionPairQueueSize += 1;
				_collisionPairQueue[_collisionPairQueueSize] = runnerIdx;
				_collisionPairQueueSize += 1;
			}
			count = 0;
			firstRunner = -1;
		} else {
			count = 0;
			firstRunner = -1;
		}
		return;
	}

	if (eventCode == kGridEntityEventCode021_CollisionAdvancePhase0 || eventCode == kGridEntityEventCode031_CollisionAdvancePhase1 || eventCode == kGridEntityEventCode041_CollisionAdvancePhase2 ||
		eventCode == kGridEntityEventCode051_CollisionAdvancePhase3 || eventCode == kGridEntityEventCode061_CollisionAdvancePhase4) {
		if (_cellArrivalQueueSize < kMaxQueueSize) {
			_cellArrivalQueue[_cellArrivalQueueSize] = runnerIdx;
			_cellArrivalQueueSize += 1;
		}
	}
}
ZmbEventHandleResult ZoombiniPuzzleMaze::onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) {
	ZmbEventHandleResult result = ZoombiniInteractive::onLButtonDown(absPos, relPos);
	if (result == ZmbEventHandleResult::kConsumed)
		return result;
	if (_isUpdating || isDragging() || 10 <= _placedRunnerCount)
		return ZmbEventHandleResult::kPassthrough;

	ZmbSnoid *clicked = findIdleSnoidAtPoint(absPos);
	if (!clicked)
		return ZmbEventHandleResult::kPassthrough;

	startSnoidDrag(clicked, absPos);
	return ZmbEventHandleResult::kConsumed;
}

void ZoombiniPuzzleMaze::endDrag(const Common::Point &dropPos) {
	(void)dropPos;
	ZmbSnoid *snoid = finishSnoidDrag();
	if (!snoid)
		return;

	const int16 runnerIdx = findRunnerBySnoidId(snoid->getId());
	const bool hasCornerGroup = 0 <= runnerIdx && runnerIdx < _runnerCount;
	const MazeCornerGroup cornerGroup = hasCornerGroup ? _runnerStates[runnerIdx].cornerGroup : kLowerLeft00;
	int16 seatIdx = findSeatAtPoint(snoid->getPointLoc());
	if (0 <= seatIdx) {
		handleGridDrop(seatIdx, snoid);
	} else {
		const bool settledAtRelease = settleSnoidAfterTerrainDrop(snoid, _dragOrigPos);
		// Maze immediately rematerializes a blocked or collision-adjusted drop at
		// its pickup point. A normal walkable release remains at the release point.
		if (!settledAtRelease) {
			snoid->setPointLoc(_dragOrigPos);
			settleSnoidAtTarget(snoid, _dragOrigPos);
			prepareSnoidVisualCoverage(snoid, true);
		}

		if (hasCornerGroup && cornerGroup == kUpperLeft01)
			reorderExactFlagSnoidGroupByDepth(kUpperLeftWaitingSnoidFlags);
		else if (hasCornerGroup && cornerGroup == kUpperRight03)
			reorderExactFlagSnoidGroupByDepth(kUpperRightWaitingSnoidFlags);
	}
}

const Common::Rect &ZoombiniPuzzleMaze::getDragConstraintRect() const {
	MazeCornerGroup cornerGroup = kLowerLeft00;
	if (_draggedSnoid) {
		const int16 runnerIdx = findRunnerBySnoidId(_draggedSnoid->getId());
		if (0 <= runnerIdx && runnerIdx < _runnerCount)
			cornerGroup = _runnerStates[runnerIdx].cornerGroup;
	}
	return _dragConstraintRects[cornerGroup];
}

int16 ZoombiniPuzzleMaze::findSeatAtPoint(const Common::Point &pos) const {
	const int16 drawSlot = hitTestDrawOnRegSlot(pos, _clickZoneRadius, true);
	if (drawSlot < 0)
		return -1;

	// Commit the same registered target that owns the drag highlight.
	// Active Maze launchers are a subset of the authored seat table,
	// so map the shared slot runner back to its authored seat index.
	for (int16 seatIdx = 0; seatIdx < 14; seatIdx++) {
		if (_launcherDropTargetFeatures[seatIdx] == _drawOnRegFeatures[drawSlot])
			return seatIdx;
	}
	return -1;
}

ZmbSnoid *ZoombiniPuzzleMaze::findIdleSnoidAtPoint(const Common::Point &pos) const {
	for (int16 snoidIdx = 0; snoidIdx < _pageLoadedZmbCount; snoidIdx++) {
		uint16 snoidId = 10000 + snoidIdx;
		ZmbSnoid *snoid = const_cast<ZoombiniPuzzleMaze *>(this)->getSnoid(snoidId);
		if (!snoid || !snoid->isRenderActivated())
			continue;
		int16 runnerIdx = findRunnerBySnoidId(snoidId);
		if (0 <= runnerIdx && _runnerStates[runnerIdx].placed)
			continue;
		if (snoid->hasClickRect() && snoid->getClickRect().contains(pos))
			return snoid;
	}
	return nullptr;
}

void ZoombiniPuzzleMaze::handleGridDrop(int16 seatIdx, ZmbSnoid *snoid) {
	if (seatIdx < 0 || 14 <= seatIdx || !_launcherSeatEnabled[seatIdx] ||
		_launcherOccupancyMarkers[seatIdx])
		return;

	int16 runnerIdx = findRunnerBySnoidId(snoid->getId());
	if (runnerIdx < 0) {
		if (kMaxRunners <= _runnerCount)
			return;
		runnerIdx = _runnerCount;
		_runnerCount += 1;
		_runnerSnoidIds[runnerIdx] = snoid->getId();
	}

	MazeRunnerState &rs = _runnerStates[runnerIdx];
	ZmbFeature *bubbleFeature = rs.bubbleFeature;
	ZmbFeature *overlayFeature = rs.overlayFeature;
	const MazeCornerGroup cornerGroup = rs.cornerGroup;
	rs.clear();
	rs.bubbleFeature = bubbleFeature;
	rs.overlayFeature = overlayFeature;
	rs.cornerGroup = cornerGroup;
	rs.oldRow = kSeatGridCoords[seatIdx].x;
	rs.oldCol = kSeatGridCoords[seatIdx].y;
	rs.direction = kSeatMoveDirection[seatIdx];
	// The launcher cell is the previous cell, never the live one.
	// A column entry reuses the ordinary grid-step script, which already carries the
	// Snoid one column across, so its live cell starts one column ahead.
	// A row entry plays a dedicated in-place launch script and stays on the seat cell.
	rs.row = rs.oldRow;
	rs.col = rs.oldCol;
	if (rs.direction == kMazeDirection00_West)
		rs.col = rs.oldCol - 1;
	else if (rs.direction == kMazeDirection02_East)
		rs.col = rs.oldCol + 1;
	rs.launcherSeatIdx = seatIdx;
	rs.feetTrait = snoid->_trait._feet;
	rs.placed = true;

	// Leave the holding animation through the shared arrival handoff before
	// applying the launcher-specific common pose and facing direction.
	settleSnoidAtTarget(snoid, kSeatPositions[seatIdx]);
	snoid->setPointLoc(kSeatPositions[seatIdx]);
	snoid->setFacingLeft(kSeatFacingLeft[seatIdx]);
	snoid->setCommonImageIndex(static_cast<byte>(kSeatCommonImageIndex[seatIdx]));
	// The launcher SCRB owns the next motion. Materialize its authored common
	// pose now so the shared arrival turn cannot retain the drag direction.
	snoid->setAnimState(kSnoidAnimState000_Idle);
	snoid->setFlags(ZmbFeature::FLAG_00000001_TYPE_SNOID | ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_04000000_OVERLAY);
	initRunnerAnimTable(runnerIdx);
	_animTablesInitialized = true;

	if (_placedRunnerCount < 10) {
		_placedRunnerIndices[_placedRunnerCount] = runnerIdx;
		_placedRunnerCount += 1;
	}
	_launcherOccupancyMarkers[seatIdx] = seatIdx + 1;
	_launcherRunnerIndices[seatIdx] = runnerIdx;
	const int16 drawSlot = hitTestDrawOnRegSlot(kSeatPositions[seatIdx],
												_clickZoneRadius, false);
	if (0 <= drawSlot)
		setDrawOnRegOccupant(drawSlot, snoid->getId());
	if (_launcherSetupQueueSize < kMaxQueueSize) {
		_launcherSetupQueue[_launcherSetupQueueSize] = seatIdx;
		_launcherSetupQueueSize += 1;
	}
	debugC(4, MohawkEngine_Zoombini::kDebugPage02, "maze: placed runner %d at launcher %d (row %d, column %d, direction %d)", runnerIdx, seatIdx,
		   rs.oldRow, rs.oldCol, rs.direction);

}
void ZoombiniPuzzleMaze::processQueues() {
	processLauncherSetupQueue();
	processMovementQueue();
	processCollisionOverlayRelinkQueue();
	processTraversalColumnRelinkQueue();
	processAcceptedCelebrationQueue();
	processArrivalDepthReorderRequests();
	processExitCompletionQueue();
	processCellArrivalQueue();
	processCollisionPairQueue();
}

void ZoombiniPuzzleMaze::processLauncherSetupQueue() {
	while (0 < _launcherSetupQueueSize) {
		_launcherSetupQueueSize -= 1;
		setupLauncherScrb(_launcherSetupQueue[_launcherSetupQueueSize]);
	}
}

void ZoombiniPuzzleMaze::setupLauncherScrb(int16 launcherSeatIdx) {
	if (launcherSeatIdx < 0 || 14 <= launcherSeatIdx)
		return;

	int16 runnerIdx = _launcherRunnerIndices[launcherSeatIdx];
	if (runnerIdx < 0)
		return;

	ZmbFeature *launcher = _launcherFeatures[launcherSeatIdx];
	if (!launcher)
		return;
	loadScrbOntoFeature(launcher, kCreatureScrbTable[launcherSeatIdx]);
	ZmbFeature *shadow = _launcherShadowFeatures[launcherSeatIdx];
	if (kCreatureHasShadow[launcherSeatIdx] && shadow) {
		loadScrbOntoFeature(shadow, static_cast<int16>(kCreatureScrbTable[launcherSeatIdx] + 1));
	}

	// Keep the launcher, Snoid, and optional shadow in one timing group.
	// Link the Snoid immediately after the launcher in the global runner list.
	ZmbSnoid *snoid = getSnoid(_runnerSnoidIds[runnerIdx]);
	if (snoid) {
		manualLinkAfter(snoid, launcher);
		registerFeatureTimingGroup(launcher, snoid);
		if (shadow)
			joinFeatureTimingGroup(launcher, shadow);
	}
}

void ZoombiniPuzzleMaze::activateRunnerBubble(int16 runnerIdx, int16 scrbId, const Common::Point &pos) {
	const uint32 flags = ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE |
						 ZmbFeature::FLAG_00800000_POS_DELTA | ZmbFeature::FLAG_04000000_OVERLAY;
	activateRunnerBubble(runnerIdx, scrbId, pos, flags);
}

void ZoombiniPuzzleMaze::activateRunnerBubble(int16 runnerIdx, int16 scrbId,
											  const Common::Point &pos, uint32 flags) {
	if (runnerIdx < 0 || _runnerCount <= runnerIdx)
		return;
	MazeRunnerState &rs = _runnerStates[runnerIdx];
	if (!rs.bubbleFeature) {
		// Allocate this runner only when the launcher's A/K/U event fires.
		// Creating SCRB 10000 at drop time exposes its first oval frame before the bubble machine starts.
		rs.bubbleFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape10000_Cell), scrbId, 7, pos, flags);
		// Load the SCRB immediately.
		// A newly allocated traversal bubble then materializes frame zero in this dispatch pass.
		// This clears the registration sentinel and avoids a one-tick launch pause at any of the four corner launchers.
		loadScrbOntoFeature(rs.bubbleFeature, scrbId);
	} else {
		rs.bubbleFeature->setFlags(flags);
		loadScrbOntoFeature(rs.bubbleFeature, scrbId);
		rs.bubbleFeature->setPointLoc(pos);
	}
	rs.bubbleFeature->setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniPuzzleMaze::adjustBubbleHotspotPosition));
	rs.bubbleFeature->activateRender();
}

void ZoombiniPuzzleMaze::startInitialTraversal(int16 runnerIdx, ZmbScrsCompletionMode completionMode) {
	if (runnerIdx < 0 || _runnerCount <= runnerIdx)
		return;
	MazeRunnerState &rs = _runnerStates[runnerIdx];
	if (!rs.placed || rs.moving)
		return;

	ZmbSnoid *snoid = getSnoid(_runnerSnoidIds[runnerIdx]);
	if (!snoid)
		return;

	// The launch anchor and the column parent both come from the launcher cell,
	// which the drop stored as the previous cell.
	Common::Point pos = _gridCellPos[rs.oldRow][rs.oldCol];
	const int16 launchCol = rs.oldCol;
	int16 scrbId = kResScrb10000_CellBase;
	int16 scrsId = rs.scrsTable[rs.direction];
	switch (rs.direction) {
	case kMazeDirection00_West:
		pos += Common::Point(8, -33);
		break;
	case kMazeDirection01_South:
		pos += Common::Point(4, -38);
		scrbId = kResScrb10042_CellTraversalAlt0;
		scrsId = static_cast<int16>(rs.feetTrait + kResScrs15080_TraversalBase);
		break;
	case kMazeDirection02_East:
		pos += Common::Point(4, -38);
		scrbId = kResScrb10044_CellTraversalAlt2;
		scrsId = static_cast<int16>(rs.feetTrait + kResScrs15085_TraversalBase);
		break;
	case kMazeDirection03_North:
		pos += Common::Point(4, -33);
		scrbId = kResScrb10043_CellTraversalAlt1;
		scrsId = static_cast<int16>(rs.feetTrait + kResScrs15085_TraversalBase);
		break;
	default:
		return;
	}
	rs.pixelX = pos.x;
	rs.pixelY = pos.y;
	activateRunnerBubble(runnerIdx, scrbId, pos);
	registerFeatureTimingGroup(rs.bubbleFeature, snoid);
	if (0 <= rs.launcherSeatIdx && rs.launcherSeatIdx < 14) {
		ZmbFeature *launcher = _launcherFeatures[rs.launcherSeatIdx];
		ZmbFeature *shadow = _launcherShadowFeatures[rs.launcherSeatIdx];
		if (launcher)
			joinFeatureTimingGroup(rs.bubbleFeature, launcher);
		if (shadow)
			joinFeatureTimingGroup(rs.bubbleFeature, shadow);
	}
	// Materialize the new bubble during the launcher event that creates it.
	// This consumes its loaded-SCRB hold before the Snoid's first traversal frame joins the scheduled pass.
	rs.bubbleFeature->onPreRender(this);
	// For directions 0, 1, and 3 the Snoid is placed immediately before its column parent,
	// then the bubble is placed immediately after the Snoid.
	// This is the launch-time Z-order, not a position-derived sort.
	if (rs.direction != kMazeDirection02_East) {
		if (0 <= launchCol && launchCol < 11 && _columnDepthAnchors[launchCol])
			manualLinkBefore(snoid, _columnDepthAnchors[launchCol]);
		manualLinkAfter(rs.bubbleFeature, snoid);
	}
	snoid->setPointLoc(pos);
	if (snoid->startScrsPlayback(ZmbResource(ZmbResource::kPage, scrsId), completionMode, resolveScrsRejectState(scrsId)))
		rs.moving = true;
}

void ZoombiniPuzzleMaze::processMovementQueue() {
	while (0 < _movementQueueSize) {
		_movementQueueSize -= 1;
		moveZmbOnGrid(_movementQueue[_movementQueueSize]);
	}
}

void ZoombiniPuzzleMaze::moveZmbOnGrid(int16 runnerIdx) {
	if (runnerIdx < 0 || _runnerCount <= runnerIdx)
		return;

	MazeRunnerState &rs = _runnerStates[runnerIdx];
	if (!rs.placed || rs.rejecting || rs.boundaryExit || rs.collided)
		return;

	rs.oldRow = rs.row;
	rs.oldCol = rs.col;
	ZmbGridDirection gridDirection;
	if (!decodeMazeGridDirection(rs.direction, gridDirection))
		return;

	ZmbGridCell cell(rs.row, rs.col);
	ZmbGridTraversal::advanceCell(cell, gridDirection, ZmbGridBounds(0, kGridRows - 1, 0, kGridCols - 1));
	rs.row = cell.row;
	rs.col = cell.col;
	// A new bubble entering a sticky cell pushes its current captive onward in
	// the incoming direction before the newcomer becomes the next captive.
	MazeCellState *destination = getCellState(getCellStateIndex(rs.row, rs.col));
	if (destination && destination->type == kMazeCellType05_ColoredSticky && 0 <= destination->heldRunner) {
		int16 released = destination->heldRunner;
		destination->heldRunner = -1;
		if (released < _runnerCount) {
			_runnerStates[released].direction = rs.direction;
			_runnerStates[released].waiting = false;
			if (_movementQueueSize < kMaxQueueSize) {
				_movementQueue[_movementQueueSize] = released;
				_movementQueueSize += 1;
			}
		}
	}

	Common::Point pos = _gridCellPos[rs.oldRow][rs.oldCol] + Common::Point(4, -38);
	rs.pixelX = pos.x;
	rs.pixelY = pos.y;
	activateRunnerBubble(runnerIdx,
						 static_cast<int16>(kResScrb10000_CellBase + rs.direction), pos);

	ZmbSnoid *snoid = getSnoid(_runnerSnoidIds[runnerIdx]);
	int16 scrsId = rs.scrsTable[rs.direction];
	if (snoid) {
		snoid->setPointLoc(pos);
		if (snoid->startScrsPlayback(ZmbResource(ZmbResource::kPage, scrsId), ZmbScrsCompletionMode::kReturnToIdle, resolveScrsRejectState(scrsId))) {
			// Each grid segment starts with a fresh shared timing slot.
			// Do not carry the previous segment's owner or cached timing result across the SCRB reload.
			registerFeatureTimingGroup(rs.bubbleFeature, snoid);
			rs.moving = true;
			rs.waiting = false;
		}
	}
}

void ZoombiniPuzzleMaze::processCollisionOverlayRelinkQueue() {
	while (0 < _collisionOverlayRelinkQueueSize) {
		_collisionOverlayRelinkQueueSize -= 1;
		int16 runnerIdx = _collisionOverlayRelinkQueue[_collisionOverlayRelinkQueueSize];
		if (runnerIdx < 0 || _runnerCount <= runnerIdx || !_collisionOverlayAnchorFeature)
			continue;
		ZmbFeature *overlay = _runnerStates[runnerIdx].overlayFeature;
		if (overlay)
			manualLinkBefore(overlay, _collisionOverlayAnchorFeature);
	}
}

void ZoombiniPuzzleMaze::processTraversalColumnRelinkQueue() {
	while (0 < _traversalColumnRelinkQueueSize) {
		_traversalColumnRelinkQueueSize -= 1;
		const int16 runnerIdx = _traversalColumnRelinkQueue[_traversalColumnRelinkQueueSize];
		if (runnerIdx < 0 || _runnerCount <= runnerIdx)
			continue;

		const MazeRunnerState &rs = _runnerStates[runnerIdx];
		ZmbSnoid *snoid = getSnoid(_runnerSnoidIds[runnerIdx]);
		if (snoid && 0 <= rs.oldCol && rs.oldCol < 11 && _columnDepthAnchors[rs.oldCol])
			manualLinkBefore(snoid, _columnDepthAnchors[rs.oldCol]);
	}
}

void ZoombiniPuzzleMaze::processAcceptedCelebrationQueue() {
	while (0 < _acceptedCelebrationQueueSize) {
		_acceptedCelebrationQueueSize -= 1;
		int16 runnerIdx = _acceptedCelebrationQueue[_acceptedCelebrationQueueSize];
		if (runnerIdx < 0 || _runnerCount <= runnerIdx)
			continue;
		MazeRunnerState &rs = _runnerStates[runnerIdx];
		ZmbSnoid *snoid = getSnoid(_runnerSnoidIds[runnerIdx]);
		int16 scrsId = static_cast<int16>(rs.feetTrait + kResScrs15090_CelebrationBase);
		if (snoid) {
			const bool alwaysPlayCelebrationSfx = ConfMan.getBool(MohawkMetaEngine_Zoombini::kOptionMazeAlwaysPlayCelebrationSfx);
			ZmbFeature::ScriptSoundPolicy soundPolicy;
			if (alwaysPlayCelebrationSfx)
				soundPolicy = ZmbFeature::ScriptSoundPolicy::kForcedPriorityQueue;
			else
				soundPolicy = ZmbFeature::ScriptSoundPolicy::kPriorityQueue;
			snoid->setScriptSoundPolicy(soundPolicy);
			// The original priority queue normally masks this individual arrival voice.
			// Suppress it explicitly in compatibility mode because the port cannot
			// reconstruct the discarded candidate after the surrounding SFX have ended.
			// The later group celebration starts its own SCRS without suppression.
			if (startSnoidScrs(snoid, ZmbResource(ZmbResource::kPage, scrsId), ZmbScrsCompletionMode::kReturnToIdle, nullptr, !alwaysPlayCelebrationSfx))
				rs.exitStage = 3;
		}
	}
}

void ZoombiniPuzzleMaze::processArrivalDepthReorderRequests() {
	if (_upperLeftArrivalDepthReorderPending) {
		_upperLeftArrivalDepthReorderPending = false;
		reorderExactFlagSnoidGroupByDepth(kUpperLeftWaitingSnoidFlags);
	}
	if (_upperRightArrivalDepthReorderPending) {
		_upperRightArrivalDepthReorderPending = false;
		reorderExactFlagSnoidGroupByDepth(kUpperRightWaitingSnoidFlags);
	}
}

void ZoombiniPuzzleMaze::processExitCompletionQueue() {
	while (0 < _exitCompletionQueueSize) {
		_exitCompletionQueueSize -= 1;
		finishRunnerExit(_exitCompletionQueue[_exitCompletionQueueSize]);
	}
}

void ZoombiniPuzzleMaze::reorderExactFlagSnoidGroupByDepth(uint32 exactFlags) {
	if (!_columnDepthAnchors[0] || exactFlags == 0)
		return;

	Common::Array<ZmbSnoid *> runnerOrder;
	collectSnoidsInRunnerOrder(runnerOrder);

	Common::Array<ZmbSnoid *> group;
	for (ZmbSnoid *snoid : runnerOrder) {
		if (snoid && snoid->getFlags() == exactFlags)
			group.push_back(snoid);
	}

	// Preserve the linked-list order for equal keys.
	for (uint32 groupIdx = 1; groupIdx < group.size(); groupIdx++) {
		ZmbSnoid *incoming = group[groupIdx];
		int32 insertionIdx = static_cast<int32>(groupIdx) - 1;
		while (0 <= insertionIdx) {
			const Common::Rect &incomingRect = incoming->getClickRect();
			const Common::Rect &existingRect = group[insertionIdx]->getClickRect();
			if (!(incomingRect.bottom < existingRect.bottom || (incomingRect.bottom == existingRect.bottom && incomingRect.left < existingRect.left)))
				break;
			group[insertionIdx + 1] = group[insertionIdx];
			insertionIdx -= 1;
		}
		group[insertionIdx + 1] = incoming;
	}

	ZmbFeature *insertionParent = _columnDepthAnchors[0];
	for (ZmbSnoid *snoid : group) {
		manualLinkAfter(snoid, insertionParent);
		insertionParent = snoid;
	}
}

void ZoombiniPuzzleMaze::resetArrivalPosCounters() {
	// The lower-left waiting positions double as the pack lineup, so slots 0 through 15
	// are already taken by Zoombinis still queued to launch. Page setup therefore seeds
	// that direction at 16, which is the first staging slot the lineup never occupies.
	// The other three corners start empty.
	_arrivalPosCounter[kLowerLeft00] = kLowerLeftArrivalPosStart;
	_arrivalPosCounter[kUpperLeft01] = 0;
	_arrivalPosCounter[kLowerRight02] = 0;
	_arrivalPosCounter[kUpperRight03] = 0;
}

void ZoombiniPuzzleMaze::handleArrival(MazeCornerGroup cornerGroup, int16 runnerIdx) {
	if (runnerIdx < 0 || _runnerCount <= runnerIdx)
		return;

	MazeRunnerState &rs = _runnerStates[runnerIdx];
	ZmbSnoid *snoid = getSnoid(_runnerSnoidIds[runnerIdx]);
	int16 posIdx = _arrivalPosCounter[cornerGroup];
	_arrivalPosCounter[cornerGroup] += 1;
	if (19 < posIdx) {
		posIdx = 0;
		_arrivalPosCounter[cornerGroup] = 1;
	}
	rs.cornerArrivalPosIdx = posIdx;
	if (snoid) {
		// Merge both the old and newly prepared visual coverage during the synchronous state change.
		// Changing only the Snoid state leaves the completed exit-SCRS pose posted until the asynchronous timer fires.
		markFeatureVisualCoverageDirty(snoid, false);
		// Each boundary group replaces the traversal flags and selects its authored waiting-group parent.
		if (cornerGroup == kLowerLeft00) {
			snoid->setFlags(ZmbFeature::FLAG_00000001_TYPE_SNOID);
			if (_columnDepthAnchors[10])
				manualLinkAfter(snoid, _columnDepthAnchors[10]);
		} else if (cornerGroup == kUpperLeft01) {
			snoid->setFlags(kUpperLeftWaitingSnoidFlags);
			if (_columnDepthAnchors[1])
				manualLinkBefore(snoid, _columnDepthAnchors[1]);
		} else if (cornerGroup == kLowerRight02) {
			snoid->setFlags(ZmbFeature::FLAG_00000001_TYPE_SNOID);
			if (_columnDepthAnchors[11])
				manualLinkAfter(snoid, _columnDepthAnchors[11]);
		} else {
			snoid->setFlags(kUpperRightWaitingSnoidFlags);
			if (0 <= rs.col && rs.col < 11 && _columnDepthAnchors[rs.col])
				manualLinkBefore(snoid, _columnDepthAnchors[rs.col]);
		}
		snoid->activateRender();
		// Clear the common-image index immediately before entering walk state 7.
		// SCRS 100 remains materialized while the post-arrival state stores the
		// right-facing idle pose for a later fidget or script completion.
		snoid->setCommonImageIndex(0);
		snoid->initWalkToTarget(kArrivalPositions[20 * cornerGroup + posIdx], this);
		prepareSnoidVisualCoverage(snoid, true);
		markPreparedFeatureVisualCoverageDirty(snoid, false);
	}

	rs.moving = false;
	rs.waiting = false;
}

void ZoombiniPuzzleMaze::removePlacedRunner(int16 runnerIdx) {
	for (int16 placedRunnerIdx = 0; placedRunnerIdx < _placedRunnerCount; placedRunnerIdx++) {
		if (_placedRunnerIndices[placedRunnerIdx] != runnerIdx)
			continue;
		for (int16 shiftIdx = placedRunnerIdx; shiftIdx + 1 < _placedRunnerCount; shiftIdx++)
			_placedRunnerIndices[shiftIdx] = _placedRunnerIndices[shiftIdx + 1];
		_placedRunnerCount -= 1;
		_placedRunnerIndices[_placedRunnerCount] = 0;
		break;
	}
}

void ZoombiniPuzzleMaze::releaseRunnerTraversalVisuals(int16 runnerIdx) {
	if (runnerIdx < 0 || _runnerCount <= runnerIdx)
		return;

	MazeRunnerState &rs = _runnerStates[runnerIdx];
	ZmbFeature *bubbleFeature = rs.bubbleFeature;
	ZmbFeature *overlayFeature = rs.overlayFeature;
	rs.bubbleFeature = nullptr;
	rs.overlayFeature = nullptr;

	if (bubbleFeature)
		unregisterFeatureTimingGroup(bubbleFeature);
	if (overlayFeature)
		unregisterFeatureTimingGroup(overlayFeature);
	if (bubbleFeature) {
		// Merge the bubble feature's RMap or click rectangle into the dirty region before freeing it.
		// Defer deletion until feature iteration finishes.
		// The traversal effect must not remain as a dormant runner.
		markFeatureVisualCoverageDirty(bubbleFeature, false);
		Common::Rect dirtyRect = bubbleFeature->getSortRect();
		if (dirtyRect.isEmpty())
			dirtyRect = bubbleFeature->getClickRect();
		if (!dirtyRect.isEmpty())
			addExternalDirtyRect(dirtyRect);
		bubbleFeature->addFlag(ZmbFeature::FLAG_01000000_DEFER_RENDER);
		bubbleFeature->deactivateAnimate();
		bubbleFeature->deactivateRender();
		bubbleFeature->scheduleClose();
	}
	if (overlayFeature) {
		markFeatureVisualCoverageDirty(overlayFeature, false);
		Common::Rect dirtyRect = overlayFeature->getSortRect();
		if (dirtyRect.isEmpty())
			dirtyRect = overlayFeature->getClickRect();
		if (!dirtyRect.isEmpty())
			addExternalDirtyRect(dirtyRect);
		overlayFeature->addFlag(ZmbFeature::FLAG_01000000_DEFER_RENDER);
		overlayFeature->deactivateAnimate();
		overlayFeature->deactivateRender();
		overlayFeature->scheduleClose();
	}
	if (0 <= rs.row && rs.row < kGridRows && 0 <= rs.col && rs.col < kGridCols &&
		_collisionReservationRunnerIdx[rs.row][rs.col] == runnerIdx) {
		_collisionReservationCount[rs.row][rs.col] = 0;
		_collisionReservationRunnerIdx[rs.row][rs.col] = -1;
	}
	removePlacedRunner(runnerIdx);
	rs.placed = false;
}

void ZoombiniPuzzleMaze::finishRunnerExit(int16 runnerIdx) {
	if (runnerIdx < 0 || _runnerCount <= runnerIdx)
		return;
	MazeRunnerState &rs = _runnerStates[runnerIdx];
	ZmbSnoid *snoid = getSnoid(_runnerSnoidIds[runnerIdx]);
	const bool accepted = rs.boundaryExit && rs.cornerGroup == kUpperRight03;
	const bool rejected = rs.rejecting || rs.collided;
	releaseRunnerTraversalVisuals(runnerIdx);
	if (0 <= rs.launcherSeatIdx && rs.launcherSeatIdx < 14)
		_launcherOccupancyMarkers[rs.launcherSeatIdx] = 0;
	if (0 <= rs.launcherSeatIdx && rs.launcherSeatIdx < 14 && _launcherRunnerIndices[rs.launcherSeatIdx] == runnerIdx)
		_launcherRunnerIndices[rs.launcherSeatIdx] = -1;

	rs.rejecting = false;
	rs.boundaryExit = false;
	rs.collided = false;
	rs.exitStage = 0;
	if (rejected) {
		// The hide-on-complete vortex/collision SCRS has already removed the Snoid.
		// Do not feed it into the visible edge-arrival walk, which would reactivate rendering.
		if (snoid) {
			Common::Rect dirtyRect = snoid->getSortRect();
			if (dirtyRect.isEmpty())
				dirtyRect = snoid->getClickRect();
			if (!dirtyRect.isEmpty())
				addExternalDirtyRect(dirtyRect);
			snoid->deactivateAnimate();
			snoid->deactivateRender();
		}
	} else {
		handleArrival(rs.cornerGroup, runnerIdx);
	}

	if (accepted) {
		_acceptedExitCompletionCount += 1;
		if (_acceptedExitCompletionCount == _pageLoadedZmbCount) {
			_celebrationTrigger = true;
			// This one-shot system sound announces the all-loaded phase
			// after the final incoming (silent) celebration has completed.
			// It is separate from each waiting Snoid's SCRS voice.
			queueCompletionNarratorSound();
		}
	}
}

void ZoombiniPuzzleMaze::processCellArrivalQueue() {
	while (0 < _cellArrivalQueueSize) {
		_cellArrivalQueueSize -= 1;
		int16 runnerIdx = _cellArrivalQueue[_cellArrivalQueueSize];
		if (runnerIdx < 0 || _runnerCount <= runnerIdx)
			continue;
		MazeRunnerState &rs = _runnerStates[runnerIdx];
		if (!rs.placed || rs.rejecting || rs.boundaryExit || rs.collided)
			continue;
		if (rs.row < 0 || kGridRows <= rs.row || rs.col < 0 || kGridCols <= rs.col)
			continue;

		// A segment that changed column moves the Snoid before the parent of its
		// new column and keeps its bubble immediately after it.
		if (rs.col != rs.oldCol) {
			ZmbSnoid *snoid = getSnoid(_runnerSnoidIds[runnerIdx]);
			if (snoid && 0 <= rs.col && rs.col < 11 && _columnDepthAnchors[rs.col]) {
				manualLinkBefore(snoid, _columnDepthAnchors[rs.col]);
				if (rs.bubbleFeature)
					manualLinkAfter(rs.bubbleFeature, snoid);
			}
		}

		if (_collisionReservationRunnerIdx[rs.row][rs.col] == runnerIdx) {
			_collisionReservationCount[rs.row][rs.col] = 0;
			_collisionReservationRunnerIdx[rs.row][rs.col] = -1;
		}
		handleRowChange(_cellTypes[rs.row][rs.col], runnerIdx);
	}
}

void ZoombiniPuzzleMaze::handleRowChange(MazeCellType cellType, int16 runnerIdx) {
	MazeRunnerState &rs = _runnerStates[runnerIdx];
	int16 cellIdx = getCellStateIndex(rs.row, rs.col);
	switch (cellType) {
	case kMazeCellType01_Whirlpool:
		zmbArriveAtNodeAlt(cellIdx, runnerIdx);
		break;
	case kMazeCellType02_TraitArrow:
		moveRunnerStepAlt(cellIdx, runnerIdx);
		break;
	case kMazeCellType03_FixedArrow:
	case kMazeCellType04_ColoredArrow:
		moveRunnerStep(cellIdx, runnerIdx);
		break;
	case kMazeCellType05_ColoredSticky:
		zmbSetupCollisionTracking(cellIdx, runnerIdx);
		break;
	case kMazeCellType06_ColoredSwitch:
		_vm->_sound->playSound(ZmbResource(ZmbResource::kPage, kResSound5103_ImpactFinal));
		activateWaveGroupSwitch(cellIdx);
		moveZmbOnGrid(runnerIdx);
		break;
	case kMazeCellType20_ExitLowerLeft:
	case kMazeCellType21_ExitUpperLeft:
	case kMazeCellType22_ExitLowerRight:
	case kMazeCellType23_ExitUpperRight:
		zmbArriveAtNode(cellType, runnerIdx);
		break;
	default:
		moveZmbOnGrid(runnerIdx);
		break;
	}
}

void ZoombiniPuzzleMaze::processCollisionPairQueue() {
	while (1 < _collisionPairQueueSize) {
		_collisionPairQueueSize -= 1;
		int16 second = _collisionPairQueue[_collisionPairQueueSize];
		_collisionPairQueueSize -= 1;
		int16 first = _collisionPairQueue[_collisionPairQueueSize];
		assignCrossRunnerScrbs(first, second);
	}
	_collisionPairQueueSize = 0;
}

void ZoombiniPuzzleMaze::zmbArriveAtNode(MazeCellType cellType, int16 runnerIdx) {
	if (runnerIdx < 0 || _runnerCount <= runnerIdx)
		return;
	MazeRunnerState &rs = _runnerStates[runnerIdx];
	ZmbSnoid *snoid = getSnoid(_runnerSnoidIds[runnerIdx]);
	if (!snoid)
		return;

	switch (cellType) {
	case kMazeCellType20_ExitLowerLeft:
		rs.cornerGroup = kLowerLeft00;
		break;
	case kMazeCellType21_ExitUpperLeft:
		rs.cornerGroup = kUpperLeft01;
		break;
	case kMazeCellType22_ExitLowerRight:
		rs.cornerGroup = kLowerRight02;
		break;
	case kMazeCellType23_ExitUpperRight:
		rs.cornerGroup = kUpperRight03;
		break;
	default:
		return;
	}
	rs.moving = false;
	rs.boundaryExit = true;
	rs.exitStage = 1;
	debugC(4, MohawkEngine_Zoombini::kDebugPage02, "maze: runner %d reached corner group %d", runnerIdx, static_cast<int16>(rs.cornerGroup));
	if (cellType == kMazeCellType23_ExitUpperRight) {
		snoid->_packIsOccupied = true;
		_acceptedBoundaryEntryCount += 1;
		if (_acceptedBoundaryEntryCount == 1)
			_goButtonEnabled = true;
	}

	Common::Point pos = _gridCellPos[rs.row][rs.col] + Common::Point(4, -38);
	rs.pixelX = pos.x;
	rs.pixelY = pos.y;
	activateRunnerBubble(runnerIdx, kResScrb10040_ExitBubbleBase, pos);
	rs.bubbleFeature->setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniPuzzleMaze::adjustExitBubbleHotspotPosition));
	if (!rs.overlayFeature) {
		rs.overlayFeature = loadScrbFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape10000_Cell), kResScrb10041_BubbleOverlayAlt, 7, pos,
			ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_00800000_POS_DELTA);
		// Reload the newly registered runner so frame zero is materialized in this render pass
		// instead of waiting for its first tick.
		loadScrbOntoFeature(rs.overlayFeature, kResScrb10041_BubbleOverlayAlt);
	} else {
		rs.overlayFeature->setFlags(ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_00800000_POS_DELTA);
		rs.overlayFeature->setPointLoc(pos);
		loadScrbOntoFeature(rs.overlayFeature, kResScrb10041_BubbleOverlayAlt);
		rs.overlayFeature->activateRender();
	}
	rs.overlayFeature->setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniPuzzleMaze::adjustBubbleHotspotPosition));
	registerFeatureTimingGroup(rs.bubbleFeature, snoid);
	joinFeatureTimingGroup(rs.bubbleFeature, rs.overlayFeature);

	int16 scrsId = static_cast<int16>(rs.feetTrait + kResScrs15075_FeetBase);
	snoid->setPointLoc(pos);
	snoid->startScrsPlayback(ZmbResource(ZmbResource::kPage, scrsId), ZmbScrsCompletionMode::kReturnToIdle, resolveScrsRejectState(scrsId));
}

void ZoombiniPuzzleMaze::zmbArriveAtNodeAlt(int16 cellIdx, int16 runnerIdx) {
	if (runnerIdx < 0 || _runnerCount <= runnerIdx)
		return;
	MazeCellState *cell = getCellState(cellIdx);
	if (!cell)
		return;

	MazeRunnerState &rs = _runnerStates[runnerIdx];
	ZmbSnoid *snoid = getSnoid(_runnerSnoidIds[runnerIdx]);
	if (!snoid)
		return;

	Common::Point bubblePos = _gridCellPos[cell->row][cell->col] + Common::Point(3, -38);
	Common::Point snoidPos = bubblePos + Common::Point(17, 5);
	rs.pixelX = snoidPos.x;
	rs.pixelY = snoidPos.y;
	rs.moving = false;
	rs.rejecting = true;
	activateRunnerBubble(runnerIdx, kResScrb10030_BubbleBase, bubblePos);
	if (!rs.overlayFeature) {
		rs.overlayFeature = loadScrbFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape10000_Cell), kResScrb10031_BubbleOverlay, 7, bubblePos,
			ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE |
				ZmbFeature::FLAG_00800000_POS_DELTA | ZmbFeature::FLAG_04000000_OVERLAY);
		// The explicit load clears the initial-registration sentinel before the
		// next pre-render, so DEFER_ANIM does not freeze shape 42 at frame zero.
		loadScrbOntoFeature(rs.overlayFeature, kResScrb10031_BubbleOverlay);
	} else {
		rs.overlayFeature->setFlags(ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE |
									ZmbFeature::FLAG_00800000_POS_DELTA | ZmbFeature::FLAG_04000000_OVERLAY);
		rs.overlayFeature->setPointLoc(bubblePos);
		loadScrbOntoFeature(rs.overlayFeature, kResScrb10031_BubbleOverlay);
		rs.overlayFeature->activateRender();
	}
	rs.overlayFeature->setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniPuzzleMaze::adjustBubbleHotspotPosition));
	manualLinkBefore(rs.overlayFeature, snoid);
	registerFeatureTimingGroup(rs.bubbleFeature, snoid);
	joinFeatureTimingGroup(rs.bubbleFeature, rs.overlayFeature);

	snoid->setPointLoc(snoidPos);
	snoid->startScrsPlayback(ZmbResource(ZmbResource::kPage, kResScrs14006_RejectFlight), ZmbScrsCompletionMode::kHide, resolveScrsRejectState(kResScrs14006_RejectFlight));
}

void ZoombiniPuzzleMaze::moveRunnerStep(int16 cellIdx, int16 runnerIdx) {
	MazeCellState *cell = getCellState(cellIdx);
	if (!cell || runnerIdx < 0 || _runnerCount <= runnerIdx)
		return;

	MazeRunnerState &rs = _runnerStates[runnerIdx];
	rs.direction = cell->direction;
	if (cell->cycleOnPass) {
		_vm->_sound->playSound(ZmbResource(ZmbResource::kPage, static_cast<int16>(kResSound5101_ImpactAlternate0 + _soundAlternator)));
		_soundAlternator += 1;
		if (1 < _soundAlternator)
			_soundAlternator = 0;

		for (int attempts = 0; attempts < 4; attempts++) {
			cell->direction += 1;
			if (3 < cell->direction)
				cell->direction = 0;
			if (cell->dirFlags[cell->direction])
				break;
		}
		_nodeDirection[cell->row][cell->col] = cell->direction;
	}
	moveZmbOnGrid(runnerIdx);
}

void ZoombiniPuzzleMaze::moveRunnerStepAlt(int16 cellIdx, int16 runnerIdx) {
	MazeCellState *cell = getCellState(cellIdx);
	if (!cell || runnerIdx < 0 || _runnerCount <= runnerIdx)
		return;

	MazeRunnerState &rs = _runnerStates[runnerIdx];
	ZmbSnoid *snoid = getSnoid(_runnerSnoidIds[runnerIdx]);
	if (snoid && getTraitByCategory(snoid->_trait, cell->traitCategory) ==
					 cell->traitValue)
		rs.direction = cell->direction;
	moveZmbOnGrid(runnerIdx);
}

void ZoombiniPuzzleMaze::zmbSetupCollisionTracking(int16 cellIdx, int16 runnerIdx) {
	MazeCellState *cell = getCellState(cellIdx);
	if (!cell || runnerIdx < 0 || _runnerCount <= runnerIdx)
		return;

	MazeRunnerState &rs = _runnerStates[runnerIdx];
	ZmbSnoid *snoid = getSnoid(_runnerSnoidIds[runnerIdx]);
	cell->heldRunner = runnerIdx;
	rs.oldRow = rs.row;
	rs.oldCol = rs.col;
	rs.moving = false;
	rs.waiting = true;

	Common::Point pos = _gridCellPos[cell->row][cell->col] + Common::Point(4, -38);
	activateRunnerBubble(runnerIdx, static_cast<uint16>(10036 + rs.direction), pos);
	int16 scrsId = rs.scrsTable[4 + rs.direction];
	if (snoid) {
		snoid->setPointLoc(pos);
		snoid->startScrsPlayback(ZmbResource(ZmbResource::kPage, scrsId), ZmbScrsCompletionMode::kReturnToIdle, resolveScrsRejectState(scrsId));
	}
}

void ZoombiniPuzzleMaze::activateWaveGroupSwitch(int16 cellIdx) {
	MazeCellState *switchCell = getCellState(cellIdx);
	if (!switchCell)
		return;
	switchCell->activationActive = true;
	switchCell->activationFrame = 0;

	// Assign group 1 a zero iteration count.
	if (switchCell->waveGroup <= 1 || kMaxWaveGroups < switchCell->waveGroup)
		return;
	int16 groupIdx = switchCell->waveGroup - 1;
	for (int16 colIdx = _waveGroupCellCounts[groupIdx] - 1; 0 <= colIdx; colIdx--) {
		MazeCellState *cell = getCellState(_waveGroupCellIndices[groupIdx][colIdx]);
		if (!cell)
			continue;
		if (cell->type == kMazeCellType04_ColoredArrow) {
			for (int attempts = 0; attempts < 4; attempts++) {
				cell->direction += 1;
				if (3 < cell->direction)
					cell->direction = 0;
				if (cell->dirFlags[cell->direction])
					break;
			}
			cell->activationActive = true;
			cell->activationFrame = 0;
			_nodeDirection[cell->row][cell->col] = cell->direction;
		} else if (cell->type == kMazeCellType05_ColoredSticky && 0 <= cell->heldRunner) {
			int16 released = cell->heldRunner;
			cell->heldRunner = -1;
			if (released < _runnerCount) {
				_runnerStates[released].waiting = false;
				if (_movementQueueSize < kMaxQueueSize) {
					_movementQueue[_movementQueueSize] = released;
					_movementQueueSize += 1;
				}
			}
		}
	}
}

void ZoombiniPuzzleMaze::assignCrossRunnerScrbs(int16 runner1Idx, int16 runner2Idx) {
	if (runner1Idx < 0 || _runnerCount <= runner1Idx ||
		runner2Idx < 0 || _runnerCount <= runner2Idx)
		return;

	static constexpr int16 kCollisionScrsBase[4][4][2] = {
		{{kResScrs15035_CollisionBase0, kResScrs15045_CollisionBase2},
		 {kResScrs15045_CollisionBase2, kResScrs15050_CollisionBase3},
		 {kResScrs15035_CollisionBase0, kResScrs15045_CollisionBase2},
		 {kResScrs15045_CollisionBase2, kResScrs15040_CollisionBase1}},
		{{kResScrs15050_CollisionBase3, kResScrs15045_CollisionBase2},
		 {kResScrs15050_CollisionBase3, kResScrs15040_CollisionBase1},
		 {kResScrs15050_CollisionBase3, kResScrs15035_CollisionBase0},
		 {kResScrs15050_CollisionBase3, kResScrs15040_CollisionBase1}},
		{{kResScrs15035_CollisionBase0, kResScrs15045_CollisionBase2},
		 {kResScrs15035_CollisionBase0, kResScrs15050_CollisionBase3},
		 {kResScrs15035_CollisionBase0, kResScrs15045_CollisionBase2},
		 {kResScrs15035_CollisionBase0, kResScrs15040_CollisionBase1}},
		{{kResScrs15040_CollisionBase1, kResScrs15045_CollisionBase2},
		 {kResScrs15050_CollisionBase3, kResScrs15040_CollisionBase1},
		 {kResScrs15035_CollisionBase0, kResScrs15040_CollisionBase1},
		 {kResScrs15050_CollisionBase3, kResScrs15040_CollisionBase1}},
	};
	// Collision bubble SCRB table keyed by both runner directions and bubble side.
	static constexpr int16 kCollisionBubbleScrbTable[4][4][2] = {
		{{kResScrb10004_CollisionBubble0, kResScrb10006_CollisionBubble2},
		 {kResScrb10006_CollisionBubble2, kResScrb10008_CollisionBubble4},
		 {kResScrb10004_CollisionBubble0, kResScrb10006_CollisionBubble2},
		 {kResScrb10006_CollisionBubble2, kResScrb10010_CollisionBubble6}},
		{{kResScrb10008_CollisionBubble4, kResScrb10006_CollisionBubble2},
		 {kResScrb10009_CollisionBubble5, kResScrb10010_CollisionBubble6},
		 {kResScrb10008_CollisionBubble4, kResScrb10004_CollisionBubble0},
		 {kResScrb10007_CollisionBubble3, kResScrb10005_CollisionBubble1}},
		{{kResScrb10004_CollisionBubble0, kResScrb10006_CollisionBubble2},
		 {kResScrb10004_CollisionBubble0, kResScrb10008_CollisionBubble4},
		 {kResScrb10004_CollisionBubble0, kResScrb10006_CollisionBubble2},
		 {kResScrb10004_CollisionBubble0, kResScrb10011_CollisionBubble7}},
		{{kResScrb10010_CollisionBubble6, kResScrb10006_CollisionBubble2},
		 {kResScrb10007_CollisionBubble3, kResScrb10005_CollisionBubble1},
		 {kResScrb10004_CollisionBubble0, kResScrb10011_CollisionBubble7},
		 {kResScrb10009_CollisionBubble5, kResScrb10010_CollisionBubble6}},
	};
	static constexpr int16 kCollisionOverlayOffset[4][4][2] = {
		{{kResScrb10012_CollisionOverlay0, kResScrb10018_CollisionOverlay2},
		 {kResScrb10018_CollisionOverlay2, kResScrb10027_CollisionOverlay5},
		 {kResScrb10012_CollisionOverlay0, kResScrb10018_CollisionOverlay2},
		 {kResScrb10018_CollisionOverlay2, kResScrb10024_CollisionOverlay4}},
		{{kResScrb10027_CollisionOverlay5, kResScrb10018_CollisionOverlay2},
		 {kResScrb10027_CollisionOverlay5, kResScrb10024_CollisionOverlay4},
		 {kResScrb10027_CollisionOverlay5, kResScrb10012_CollisionOverlay0},
		 {kResScrb10015_CollisionOverlay1, kResScrb10021_CollisionOverlay3}},
		{{kResScrb10012_CollisionOverlay0, kResScrb10018_CollisionOverlay2},
		 {kResScrb10012_CollisionOverlay0, kResScrb10027_CollisionOverlay5},
		 {kResScrb10012_CollisionOverlay0, kResScrb10018_CollisionOverlay2},
		 {kResScrb10012_CollisionOverlay0, kResScrb10024_CollisionOverlay4}},
		{{kResScrb10024_CollisionOverlay4, kResScrb10018_CollisionOverlay2},
		 {kResScrb10015_CollisionOverlay1, kResScrb10021_CollisionOverlay3},
		 {kResScrb10012_CollisionOverlay0, kResScrb10024_CollisionOverlay4},
		 {kResScrb10027_CollisionOverlay5, kResScrb10024_CollisionOverlay4}},
	};

	MazeRunnerState &first = _runnerStates[runner1Idx];
	MazeRunnerState &second = _runnerStates[runner2Idx];
	if (first.collided || second.collided)
		return;

	int16 firstDir = CLIP<int16>(first.direction, kMazeDirection00_West, kMazeDirection03_North);
	int16 secondDir = CLIP<int16>(second.direction, kMazeDirection00_West, kMazeDirection03_North);
	first.moving = second.moving = false;
	first.collided = second.collided = true;
	first.rejecting = second.rejecting = true;
	first.collisionPopStarted = second.collisionPopStarted = false;

	// The collision is the only Maze effect that does not re-derive its anchor from
	// the grid. It reuses each Snoid's live position, which the movement script has
	// already carried part of the way to the shared cell.
	// The merged bubble and its companion overlay both sit on that position, so the
	// pair's arrangement comes from where the two Snoids actually are.
	ZmbSnoid *snoid1 = getSnoid(_runnerSnoidIds[runner1Idx]);
	ZmbSnoid *snoid2 = getSnoid(_runnerSnoidIds[runner2Idx]);
	if (snoid1) {
		first.pixelX = snoid1->getPointLoc().x;
		first.pixelY = snoid1->getPointLoc().y;
	}
	if (snoid2) {
		second.pixelX = snoid2->getPointLoc().x;
		second.pixelY = snoid2->getPointLoc().y;
	}

	constexpr uint32 collisionFlags = ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_00800000_POS_DELTA;
	activateRunnerBubble(runner1Idx, kCollisionBubbleScrbTable[secondDir][firstDir][0],
						 Common::Point(first.pixelX, first.pixelY), collisionFlags);
	activateRunnerBubble(runner2Idx, kCollisionBubbleScrbTable[secondDir][firstDir][1],
						 Common::Point(second.pixelX, second.pixelY), collisionFlags);

	int16 collisionRegsIdx = 12 * second.row + second.col;
	int16 collisionBase = 0;
	if (0 <= collisionRegsIdx && collisionRegsIdx < static_cast<int16>(_collisionOverlayRegs.size()))
		collisionBase = _collisionOverlayRegs[collisionRegsIdx];
	const int16 collisionRunnerIdx[2] = {runner1Idx, runner2Idx};
	for (int16 colIdx = 0; colIdx < 2; colIdx++) {
		MazeRunnerState &state = _runnerStates[collisionRunnerIdx[colIdx]];
		Common::Point pos(state.pixelX, state.pixelY);
		int16 overlayScrbId = static_cast<int16>(collisionBase + kCollisionOverlayOffset[secondDir][firstDir][colIdx]);
		const uint32 overlayFlags = collisionFlags | ZmbFeature::FLAG_04000000_OVERLAY;
		if (!state.overlayFeature) {
			state.overlayFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape10000_Cell), overlayScrbId, 7, pos, overlayFlags);
			loadScrbOntoFeature(state.overlayFeature, overlayScrbId);
		} else {
			state.overlayFeature->setFlags(overlayFlags);
			loadScrbOntoFeature(state.overlayFeature, overlayScrbId);
			state.overlayFeature->setPointLoc(pos);
		}
		state.overlayFeature->setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniPuzzleMaze::adjustBubbleHotspotPosition));
		state.overlayFeature->activateRender();
	}

	int16 scrs1 = static_cast<int16>(kCollisionScrsBase[secondDir][firstDir][0] + first.feetTrait - 1);
	int16 scrs2 = static_cast<int16>(kCollisionScrsBase[secondDir][firstDir][1] + second.feetTrait - 1);
	// Resolve and consume one script at a time in the authored order.
	if (snoid2)
		snoid2->startScrsPlayback(ZmbResource(ZmbResource::kPage, scrs2), ZmbScrsCompletionMode::kHide, resolveScrsRejectState(scrs2));
	if (snoid1)
		snoid1->startScrsPlayback(ZmbResource(ZmbResource::kPage, scrs1), ZmbScrsCompletionMode::kHide, resolveScrsRejectState(scrs1));

	for (int16 runnerIdx = 0; runnerIdx < 2; runnerIdx++) {
		MazeRunnerState &state = _runnerStates[collisionRunnerIdx[runnerIdx]];
		ZmbSnoid *snoid = getSnoid(_runnerSnoidIds[collisionRunnerIdx[runnerIdx]]);
		if (!snoid || !state.bubbleFeature || !state.overlayFeature)
			continue;

		if (0 <= state.col && state.col < 11 && _columnDepthAnchors[state.col])
			manualLinkBefore(snoid, _columnDepthAnchors[state.col]);
		manualLinkAfter(state.bubbleFeature, snoid);
		registerFeatureTimingGroup(state.overlayFeature, snoid);
		joinFeatureTimingGroup(state.overlayFeature, state.bubbleFeature);
	}

	_collisionReservationCount[second.row][second.col] = 0;
	_collisionReservationRunnerIdx[second.row][second.col] = -1;
}

bool ZoombiniPuzzleMaze::handleCollisionOverlayEvent(ZmbFeature *feature, int16 eventCode) {
	if (!feature)
		return false;
	int16 runnerIdx = -1;
	for (int16 runnerSlotIdx = 0; runnerSlotIdx < _runnerCount; runnerSlotIdx++) {
		if (_runnerStates[runnerSlotIdx].overlayFeature == feature) {
			runnerIdx = runnerSlotIdx;
			break;
		}
	}
	if (runnerIdx < 0)
		return false;

	MazeRunnerState &rs = _runnerStates[runnerIdx];
	if (!rs.collided)
		return false;

	switch (eventCode) {
	case kGridEntityEventCode061_CollisionAdvancePhase4: {
		// The merged-bubble overlay reached its pop hand-off marker.
		// Start the shared collision pop SCRS exactly once for the owning Snoid.
		// The merged-bubble overlay owns the second collision phase.
		// It hands its Snoid to the shared pop script while the bubble and the
		// overlay keep advancing to their burst frames.
		if (rs.collisionPopStarted)
			return true;
		ZmbSnoid *snoid = getSnoid(_runnerSnoidIds[runnerIdx]);
		if (!snoid)
			return true;
		if (!snoid->startScrsPlayback(ZmbResource(ZmbResource::kPage, kResScrs14004_RejectFinal), ZmbScrsCompletionMode::kHide, resolveScrsRejectState(kResScrs14004_RejectFinal)))
			return true;
		rs.collisionPopStarted = true;
		return true;
	}
	case kGridEntityEventCode063_RaiseCollisionOverlay:
		// The companion overlay reached its burst-layer marker.
		// Queue the runner so the next frame restores the required draw order.
		// Deep companion variants raise their burst layer above the page overlay runner.
		if (_collisionOverlayRelinkQueueSize < kMaxQueueSize) {
			_collisionOverlayRelinkQueue[_collisionOverlayRelinkQueueSize] = runnerIdx;
			_collisionOverlayRelinkQueueSize += 1;
		}
		break;
	default:
		// The original handler consumes every other companion event without acting.
		break;
	}
	return true;
}

// =================================================================
// Per-frame processing
// =================================================================

void ZoombiniPuzzleMaze::onEveryFrame() {
	if (_pageLoadedZmbCount <= 0 || _isUpdating)
		return;
	PuzzleUpdateGuard updateGuard(_isUpdating);

	setGoButtonsEnabled(_goButtonEnabled);
}

void ZoombiniPuzzleMaze::onPostRenderFrame() {
	if (_pageLoadedZmbCount <= 0 || _isUpdating)
		return;
	PuzzleUpdateGuard updateGuard(_isUpdating);

	if (_queueProcessingEnabled)
		processQueues();
	processIdleAnimations();
}

void ZoombiniPuzzleMaze::processIdleAnimations() {
	if (_celebrationTrigger && _celebrationsPlayed < _celebrationTarget) {
		if (30 < getCurrentFrameCounter() - _celebrationLastFrame) {
			_celebrationLastFrame = getCurrentFrameCounter();
			for (int16 snoidIdx = 0; snoidIdx < _pageLoadedZmbCount; snoidIdx++) {
				uint16 poolIdx = _vm->_rnd->getNonRepeatRandom(_pageLoadedZmbCount, _celebrationPoolState);
				ZmbSnoid *snoid = getSnoid(10000 + poolIdx);
				int16 runnerIdx = snoid ? findRunnerBySnoidId(snoid->getId()) : -1;
				if (!snoid || runnerIdx < 0 || !snoid->_packIsOccupied ||
					!snoid->isRenderActivated() ||
					snoid->getAnimState() != kSnoidAnimState000_Idle)
					continue;

				int16 scrsId = static_cast<int16>(snoid->_trait._feet + kResScrs15090_CelebrationBase);
				// The resource contains two FFD4 markers.
				// The persistent SFX queue's loaded-SND guard keeps the second marker
				// from layering a duplicate voice instance.
				// Group celebration voices remain ordinary Maze queue candidates so the
				// higher-priority completion narrator can finish without interruption.
				snoid->setScriptSoundPolicy(ZmbFeature::ScriptSoundPolicy::kPriorityQueue);
				if (startSnoidScrs(snoid, ZmbResource(ZmbResource::kPage, scrsId), ZmbScrsCompletionMode::kReturnToIdle))
					_celebrationsPlayed += 1;
				break;
			}
		}
	} else if (_celebrationTrigger && _celebrationTarget <= _celebrationsPlayed) {
		_celebrationPoolState = 0;
		_celebrationLastFrame = 0;
		_celebrationTrigger = false;
		_celebrationsPlayed = 0;
	}
}

} // End of namespace Mohawk
