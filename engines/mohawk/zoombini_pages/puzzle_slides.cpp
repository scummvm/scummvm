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

#include "mohawk/zoombini_pages/puzzle_slides.h"
#include "mohawk/zoombini.h"
#include "mohawk/zoombini_graphics.h"
#include "mohawk/zoombini_random.h"
#include "mohawk/zoombini_resource.h"
#include "mohawk/zoombini_sound.h"
#include "mohawk/zoombini_state.h"

namespace Mohawk {

// =============================================================================
// Static Data Tables
// =============================================================================

constexpr Common::Point ZoombiniPuzzleSlides::kSnoidPositions[16];
constexpr Common::Point ZoombiniPuzzleSlides::kCellPositions[117];
constexpr int16 ZoombiniPuzzleSlides::kSlotCellIndices[26];
constexpr int16 ZoombiniPuzzleSlides::kLinkCellIndices[43];
constexpr int16 ZoombiniPuzzleSlides::kEvenRowLinkCells[20];
constexpr int16 ZoombiniPuzzleSlides::kOddRowLinkCells[20];
constexpr int16 ZoombiniPuzzleSlides::kPairStartOffsets[16];
constexpr int16 ZoombiniPuzzleSlides::kPairSpacingArray[16];
constexpr int16 ZoombiniPuzzleSlides::kLeftArmLinkCells[18];
constexpr int16 ZoombiniPuzzleSlides::kRightArmLinkCells[18];
constexpr int16 ZoombiniPuzzleSlides::kLeftEndpointCells[3];
constexpr int16 ZoombiniPuzzleSlides::kRightEndpointCells[3];
constexpr int16 ZoombiniPuzzleSlides::kInnerLinkPairs[12];

// =============================================================================
// Constructor / Destructor
// =============================================================================

ZoombiniPuzzleSlides::ZoombiniPuzzleSlides(MohawkEngine_Zoombini *vm) : ZoombiniPuzzle(vm, ZoombiniPageType::kSlides, ZmbSrcPageKind::kSlides_09) {
	// Initialize arrays
	memset(_cellGrid, 0, sizeof(_cellGrid));
	memset(_cellAdjacencyMasks, 0, sizeof(_cellAdjacencyMasks));
	memset(_slotCellIndices, -1, sizeof(_slotCellIndices));
	memset(_snoidIds, 0, sizeof(_snoidIds));
	memset(_sortedSnoidIndices, 0, sizeof(_sortedSnoidIndices));
	memset(_pairingUsageStates, 0, sizeof(_pairingUsageStates));
	memset(_pairLinkTypes, 0, sizeof(_pairLinkTypes));
	memset(_activeCellIndices, 0, sizeof(_activeCellIndices));
	memset(_activeCellSnoidIds, 0, sizeof(_activeCellSnoidIds));
	memset(_cellFeatures, 0, sizeof(_cellFeatures));
	memset(_slotFeatures, 0, sizeof(_slotFeatures));
	memset(_slotDrawOnRegIndices, -1, sizeof(_slotDrawOnRegIndices));
	memset(_rowLayerFeatures, 0, sizeof(_rowLayerFeatures));
	memset(_debugSolutionCellIndices, -1, sizeof(_debugSolutionCellIndices));
	memset(_debugSolutionRunnerIds, 0, sizeof(_debugSolutionRunnerIds));
}

ZoombiniPuzzleSlides::~ZoombiniPuzzleSlides() {
}

const ZoombiniPage::ScriptSoundPriorityRanges &ZoombiniPuzzleSlides::getScriptSoundPriorityRanges() const {
	// Original shared page dispatch automatically prepends SND 996-997 at priority 32.
	static const ZoombiniPage::ScriptSoundPriorityRanges kRanges = {
		{kSysResSoundRange0996_DepartSFX, kSysResSoundRange0997_ArriveSFX},
		{kResSoundRange7001_InertCell, kResSoundRange7001_InertCell},
		{kResSoundRange7000_CellBase, kResSoundRange7000_CellBase},
		{kSysResSoundRange20000_Base, kSysResSoundRange29999_Last},
		{kResSoundRange8000_TravelBase, kResSoundRange8000_TravelBase},
		{kResSoundRange8500_PlacementFeedbackBase, kResSoundRange8599_PlacementFeedbackLast},
		{kResSoundRange0425_PageBase, kResSoundRange0499_PageLast},
		{kResSoundRange7002_MatchedCell, kResSoundRange7002_MatchedCell}};
	return kRanges;
}

// =============================================================================
// Page Lifecycle
// =============================================================================

void ZoombiniPuzzleSlides::open() {
	openArchive(ZMB_MHK_SLIDES);
}

void ZoombiniPuzzleSlides::setBackgroundMusic() {
	// Sound 20078 is reserved for F1 replay and does not start on page load.
}

void ZoombiniPuzzleSlides::setBackgroundBitmap() {
	_vm->_gfx->setPalette(kResBackground5000);
	_vm->_gfx->drawBackground(kResBackground5000);
}

void ZoombiniPuzzleSlides::initStates() {
	// Initialize grid parameters
	// Default to slot state 504 and its SCRB 7002 link-shape offset 48.
	_slotCellState = 504;
	_linkShapeVariantOffset = 48;

	// Celebration state init
	_celebrationVisitLatched = false;
	_celebrationCycleTarget = 0;
	_fullMatchTriggerCount = 0;
	_goButtonEnabled = false;
	_victoryPaletteActive = false;
	_lastVictoryPaletteFrame = 0;
	_boardInputEnabled = true;
	_solvedRiseTimingFeature = nullptr;
	_departureState = ZmbDepartureState::kIdle;
	_solvedRiseTailStartFrame = 0;
	_solvedRiseTailQueued = false;
	_invalidDropEffectFeature = nullptr;
	_acceptedCellCountBeforeDrag = 0;
	_acceptedCellIndexChecksum = 0;
	_acceptedCellIndexChecksumBeforeDrag = 0;
	_activeCellCount = 0;
	_builtinDebugSolveState = 0;
	memset(_activeCellIndices, 0, sizeof(_activeCellIndices));
	memset(_activeCellSnoidIds, 0, sizeof(_activeCellSnoidIds));

	_entranceFadeObserved = false;
	if (_vm->isVersionFamilyTlcV2()) {
		// The v2.0 release submits the entrance sound during page initialization.
		playEntranceSound();
		_entranceSoundPending = false;
	} else {
		_entranceSoundPending = true;
	}
}

Common::String ZoombiniPuzzleSlides::debugGetBuiltinDebugCommandHelp() const {
	Common::String output;
	output += Common::String::format("  %-9s (%s)\n", "s-o-v-l-e", kBuiltinDebugActionSolve);
	output += "    Type the five lowercase letters in that literal order; recognized out-of-order letters are ignored.\n";
	output += "    Progress reaches state 5 at every level, but only Level 4 immediately places the generated solution,\n";
	output += "    rebuilds its accepted links, unlocks Go, and adds a full-match celebration trigger.\n";
	output += "    The console action resets progress before executing the entire sequence.\n";
	output += Common::String::format("  %-9s (%s)\n", "", kBuiltinDebugActionSolveReset);
	output += "    Reset sequence progress to state 0; this is a ScummVM console convenience, not an original key.\n";
	return output;
}

bool ZoombiniPuzzleSlides::debugDoBuiltinDebugCommand(int argc, const char **argv, Common::String &output) {
	if (argc != 3) {
		output = Common::String::format("Usage: page builtin_debug <%s|%s>\n", kBuiltinDebugActionSolve, kBuiltinDebugActionSolveReset);
		return true;
	}
	const BuiltinDebugAction action = parseBuiltinDebugAction(argv[2]);
	if (action == BuiltinDebugAction::kInvalid) {
		output = Common::String::format("Unknown Stone Rise built-in debug action '%s'.\n", argv[2]);
		output += debugGetBuiltinDebugCommandHelp();
		return true;
	}
	return runBuiltinDebugAction(action, output);
}

ZoombiniPuzzleSlides::BuiltinDebugAction ZoombiniPuzzleSlides::parseBuiltinDebugAction(const Common::String &action) {
	if (action.equalsIgnoreCase(kBuiltinDebugActionSolve))
		return BuiltinDebugAction::kSolve;
	if (action.equalsIgnoreCase(kBuiltinDebugActionSolveReset))
		return BuiltinDebugAction::kSolveReset;
	return BuiltinDebugAction::kInvalid;
}

bool ZoombiniPuzzleSlides::runBuiltinDebugAction(BuiltinDebugAction action, Common::String &output) {
	if (action == BuiltinDebugAction::kSolveReset) {
		_builtinDebugSolveState = 0;
		output = "Stone Rise built-in solve sequence reset.\n";
		return false;
	}
	if (action != BuiltinDebugAction::kSolve)
		return true;

	_builtinDebugSolveState = 0;
	const char sequence[] = "sovle";
	for (uint16 i = 0; i < ARRAYSIZE(sequence) - 1; i++)
		runBuiltinSolveKey(sequence[i]);
	output = Common::String::format("Stone Rise built-in solve state %d.\n", _builtinDebugSolveState);
	return false;
}

void ZoombiniPuzzleSlides::runBuiltinSolveKey(char key) {
	switch (key) {
	case 's':
		if (_builtinDebugSolveState == 0)
			_builtinDebugSolveState = 1;
		break;
	case 'o':
		if (_builtinDebugSolveState == 1)
			_builtinDebugSolveState = 2;
		break;
	case 'v':
		if (_builtinDebugSolveState == 2)
			_builtinDebugSolveState = 3;
		break;
	case 'l':
		if (_builtinDebugSolveState == 3)
			_builtinDebugSolveState = 4;
		break;
	case 'e':
		if (_builtinDebugSolveState == 4) {
			_builtinDebugSolveState = 5;
			if (_difficultyLevel == kPuzzleLevel4) {
				placeBuiltinDebugSolution();
				resetAnimStates();
				_goButtonEnabled = true;
				setGoButtonsEnabled(true);
				_fullMatchTriggerCount += 1;
			}
		}
		break;
	default:
		break;
	}
}

ZmbEventHandleResult ZoombiniPuzzleSlides::onDebugKeyDown(const Common::KeyState &kbd) {
	if (!kbd.hasFlags(0))
		return ZmbEventHandleResult::kPassthrough;
	if (kbd.ascii != 's' && kbd.ascii != 'o' && kbd.ascii != 'l' && kbd.ascii != 'v' && kbd.ascii != 'e')
		return ZmbEventHandleResult::kPassthrough;

	runBuiltinSolveKey(static_cast<char>(kbd.ascii));
	return ZmbEventHandleResult::kConsumed;
}

void ZoombiniPuzzleSlides::loadFeatures() {
	// At the highest difficulty, load NODE/PATH 1000 for walking.
	if (_difficultyLevel == kPuzzleLevel4) {
		loadNodePath(ZmbResource(ZmbResource::kPage, kResNode1000_WalkNetwork));
	}

	// Load terrain barrier bitmap (tBMP 100)
	loadTerrainBitmap(kResBitmapTerrain100);

	// Preload shape images
	// Shapes at tBMP 6000
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape6000_Cell));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape8000_Travel));

	// Load feature groups

	// Load main features: 14 SCRBs at 7000
	ZmbFeature *mainFeature = createPuzzleMainFeatureHead();

	// 3 Subs at 8000
	{
		ZmbFeature *parent = mainFeature;
		for (uint16 i = 0; i < 3; i++) {
			parent = loadSubFeature(parent,
									ZmbResource(ZmbResource::kPage, kResBitmapShape8000_Travel), kResScrb8000_TravelBase + i);
		}
	}

	// These calls register SCRS lookup groups; they do not create ten hidden Snoid runners.
	// Group order also selects the script animation state.
	registerScrsGroup(kResScrs14000_TravelBase, 4);
	registerScrsGroup(kResScrs13000_RejectBase, 6);

	// Load Zoombinis from active pack at 16 pedestal positions
	loadZoombinisFromPack(kSnoidPositions, ARRAYSIZE(kSnoidPositions));
	// The visit target belongs after destructive pack materialization because it uses the resulting runner count.
	// Do not move it back into @ref ZoombiniPuzzleSlides::initStates().
	_celebrationCycleTarget = _pageLoadedZmbCount;

	// Layout and stagger walk-in
	layoutStaticAndWalkIn(0, false);

	// Snapshot traits before the difficulty switch.
	// Level 1 alone generates pairings; the other levels build their own chain data.
	snapshotZmbTraits();

	// Initialize the hex grid based on difficulty
	initGridByDifficulty();
	snapshotDebugSolution();

	// Register one runner for every cell.
	// State 500 uses SCRB 7001; every other state uses SCRB 7000.
	for (int16 cellIdx = 0; cellIdx < kNumCells; cellIdx++) {
		ensureCellFeature(cellIdx);
	}
	loadCellLinkOverlay();

	// Layer runners are SCRB 7004..7011 (indices 1..8).
	for (int16 layerIdx = 1; layerIdx < 9; layerIdx++) {
		ZmbFeature *layerFeature = loadScrbFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape7000_CellAnimations), kResScrb7004_LayerBase + layerIdx - 1, 6,
			ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE);
		_rowLayerFeatures[layerIdx] = layerFeature;
	}

	loadPlacementSlotFeatures();
	relinkGridLayers();
	if (0 < _slotCount)
		manualLinkAfter(_rowLayerFeatures[7], _slotFeatures[_slotCount - 1]);
	relinkSnoidsBehindForeground();

	// Refresh cached runner ids after grid setup.
	snapshotZmbTraits();

	// Materialize every grid, layer, slot, and Snoid runner before assigning walk-in deadlines.
	// The initial pass also establishes the authored runner order used by the staggered walk-in.
	renderFeatures();
	assignStaggeredWalkDelays(30, 45);
	// Locked cells, not the incoming party, define the continuing set for this puzzle.
	schedulePackSnoids(false, false);

	// Set up Go/Map/Help buttons after the initial puzzle-runner render boundary.
	configureStandardPuzzleControlRects();
	loadStandardPuzzleControlFeatures(kResBitmapShape6000_Cell);
	setGoButtonsEnabled(false);
}

void ZoombiniPuzzleSlides::initHelpPrompt() {
	_activeHelpSoundId = ZmbResource(ZmbResource::kSystem, kSysResSound20078_F1Replay);
}

void ZoombiniPuzzleSlides::onGoButtonActivated() {
	// Route 2: Slides -> Basecamp2 (via Xfer)
	restoreInitialSnoidFlags();
	if (isDepartureActive())
		return;
	if (!_goButtonEnabled)
		return;

	beginSolvedDepartureSequence();
}

void ZoombiniPuzzleSlides::executeDeparture() {
	// Keep the accepted set synchronized immediately before shared serialization.
	markMatchedRunnersDone();
	ZoombiniInteractive::executeDeparture();
}

void ZoombiniPuzzleSlides::debugPrepareForDeparture() {
	_boardInputEnabled = true;
	_departureState = ZmbDepartureState::kIdle;
	_solvedRiseTimingFeature = nullptr;
	_solvedRiseTailStartFrame = 0;
	_solvedRiseTailQueued = false;
	_activeCellCount = 0;
	memset(_activeCellIndices, 0, sizeof(_activeCellIndices));
	memset(_activeCellSnoidIds, 0, sizeof(_activeCellSnoidIds));
	for (int16 slotIdx = 0; slotIdx < _slotCount; slotIdx++) {
		int16 drawOnRegIdx = _slotDrawOnRegIndices[slotIdx];
		if (0 <= drawOnRegIdx)
			clearDrawOnRegOccupant(drawOnRegIdx);
	}

	for (int16 cellIdx = 0; cellIdx < kNumCells; cellIdx++) {
		int16 base = cellIdx * kFieldsPerCell;
		int16 state = _cellGrid[base + 1];

		if (state == kCellLocked || state == kCellOccupied) {
			_cellGrid[base + 1] = kCellConnector;
			_cellGrid[base + 2] = 0;
			syncCellFeatureScript(cellIdx);
		} else if (state == kCellMatched) {
			_cellGrid[base + 1] = kCellPath;
			syncCellFeatureScript(cellIdx);
		}
	}

	int16 placedCount = 0;
	for (int16 solutionIdx = 0; solutionIdx < _debugSolutionCount; solutionIdx++) {
		int16 cellIdx = _debugSolutionCellIndices[solutionIdx];
		if (cellIdx < 0 || kNumCells <= cellIdx)
			continue;

		ZmbSnoid *snoid = getSnoid(_debugSolutionRunnerIds[solutionIdx]);
		if (!snoid)
			continue;

		// @ref ZoombiniInteractive::debugForceFinish() materializes the same solved state as manual placement,
		// but it places each Snoid immediately without running the shared drag controller or a walk animation.
		// Explicitly invalidate the old waiting-position coverage that the drag
		// path would normally contribute to the next render's dirty region.
		addExternalDirtyRect(snoid->getZSortRect());
		moveZmbToCell(snoid, cellIdx);
		snoid->setupIdleHotspots();
		snoid->setNeedsRedraw(true);

		int16 base = cellIdx * kFieldsPerCell;
		_cellGrid[base + 1] = kCellOccupied;
		_cellGrid[base + 2] = snoid->getId();
		_activeCellIndices[placedCount] = cellIdx;
		_activeCellSnoidIds[placedCount] = snoid->getId();
		_activeCellCount += 1;

		int16 slotIdx = findSlotIndexForCell(cellIdx);
		if (0 <= slotIdx && 0 <= _slotDrawOnRegIndices[slotIdx])
			setDrawOnRegOccupant(_slotDrawOnRegIndices[slotIdx], snoid->getId());
		syncCellFeatureScript(cellIdx);
		placedCount += 1;
	}

	// Re-enter the normal match propagation. This is what produces both state
	// 508 Snoid cells and state 502 links for the ordinary Go choreography.
	_fullMatchTriggerCount = 0;
	if (_difficultyLevel <= kPuzzleLevel2) {
		for (int16 solutionIdx = 0; solutionIdx < _debugSolutionCount; solutionIdx++)
			validateChainAndMarkMatched(_debugSolutionCellIndices[solutionIdx]);
		checkVictoryCondition();
	} else {
		resetAnimStates();
	}

	int16 lockedCount = 0;
	for (int16 cellIdx = 0; cellIdx < kNumCells; cellIdx++) {
		if (_cellGrid[cellIdx * kFieldsPerCell + 1] == kCellLocked)
			lockedCount += 1;
	}

	_goButtonEnabled = 0 < lockedCount;
	_victoryPaletteActive = (_difficultyLevel == kPuzzleLevel4 && _goButtonEnabled);
	if (_victoryPaletteActive)
		_lastVictoryPaletteFrame = getCurrentFrameCounter();

	setGoButtonsEnabled(_goButtonEnabled);
}

ZmbChanceInfo ZoombiniPuzzleSlides::debugGetChances() const {
	// Stone Rise: the player may rearrange placements freely, with no chance
	// limit.
	return {ZmbChanceInfo::ZmbChanceType::kInfinite};
}

Common::String ZoombiniPuzzleSlides::debugGetAnswer() const {
	int16 requiredLinkCount = 0;
	for (int16 cellIdx = 0; cellIdx < kNumCells; cellIdx++) {
		const int16 base = cellIdx * kFieldsPerCell;
		const int16 state = _cellGrid[base + 1];
		const int16 traitKind = _cellGrid[base + 2];
		if ((state == kCellPath || state == kCellMatched) &&
			kTraitHair <= traitKind && traitKind <= kTraitFeet) {
			requiredLinkCount += 1;
		}
	}

	Common::String s = getDebugBanner();
	s += "\n";
	s += "  One generated valid placement (* means no saved solution):\n";

	for (int16 slotIdx = 0; slotIdx < _slotCount; slotIdx++) {
		const int16 cellIdx = _slotCellIndices[slotIdx];
		if (cellIdx < 0 || kNumCells <= cellIdx)
			continue;

		uint16 runnerId = 0;
		for (int16 solutionIdx = 0; solutionIdx < _debugSolutionCount; solutionIdx++) {
			if (_debugSolutionCellIndices[solutionIdx] == cellIdx) {
				runnerId = _debugSolutionRunnerIds[solutionIdx];
				break;
			}
		}

		ZmbSnoid *snoid = runnerId ? getSnoid(runnerId) : nullptr;
		s += Common::String::format("    Slot (%3d, %3d) - ", kCellPositions[cellIdx].x, kCellPositions[cellIdx].y);
		if (snoid) {
			s += Common::String::format("%s\n", snoid->toStr().c_str());
		} else {
			s += "Zoombini: *\n";
		}
	}
	return s;
}

// =============================================================================
// Grid Initialization
// =============================================================================

void ZoombiniPuzzleSlides::maybeSetMatchTrait(int16 destCell, int16 cellIdx, int16 otherCellIdx) {
	int16 trait = pickRandomMatchingTrait(cellIdx, otherCellIdx);
	if (trait != 0)
		_cellGrid[destCell * kFieldsPerCell + 2] = trait;
}

void ZoombiniPuzzleSlides::setCellStateData(int16 cellIdx, int16 state, int16 data) {
	int16 base = cellIdx * kFieldsPerCell;
	_cellGrid[base + 1] = state;
	_cellGrid[base + 2] = data;
}

void ZoombiniPuzzleSlides::clearBoard() {
	memset(_cellAdjacencyMasks, 0, sizeof(_cellAdjacencyMasks));
	for (int16 cellIdx = 0; cellIdx < kNumCells; cellIdx++) {
		int16 base = cellIdx * kFieldsPerCell;
		_cellGrid[base + 1] = kCellInert;
		_cellGrid[base + 2] = 0;
		for (int16 linkIdx = 0; linkIdx < 6; linkIdx++)
			_cellGrid[base + 3 + linkIdx] = -1;
	}
}

void ZoombiniPuzzleSlides::rebuildOccupiedSlots() {
	_slotCount = 0;
	_activeCellCount = 0;
	memset(_slotCellIndices, -1, sizeof(_slotCellIndices));
	memset(_activeCellIndices, 0, sizeof(_activeCellIndices));
	memset(_activeCellSnoidIds, 0, sizeof(_activeCellSnoidIds));
	for (int16 i = 0; i < ARRAYSIZE(kSlotCellIndices); i++) {
		int16 cell = kSlotCellIndices[i];
		if (_cellGrid[cell * kFieldsPerCell + 1] != kCellOccupied)
			continue;
		if (_slotCount < ARRAYSIZE(_slotCellIndices)) {
			_slotCellIndices[_slotCount] = cell;
			_slotCount += 1;
		}
		if (_activeCellCount < ARRAYSIZE(_activeCellIndices)) {
			_activeCellIndices[_activeCellCount] = cell;
			_activeCellSnoidIds[_activeCellCount] = _cellGrid[cell * kFieldsPerCell + 2];
			_activeCellCount += 1;
		}
	}
}

void ZoombiniPuzzleSlides::setLinkIfValid(uint16 adjMask, int16 base, uint16 bit,
										  int16 fieldOffset, int16 neighborCell) {
	if ((adjMask & bit) == 0)
		return;
	if (neighborCell < 0 || kNumCells <= neighborCell)
		return;
	_cellGrid[base + fieldOffset] = neighborCell;
}

int16 ZoombiniPuzzleSlides::getCellLink(int16 cellIdx, int16 dir) const {
	if (cellIdx < 0 || kNumCells <= cellIdx || dir < 0 || 6 <= dir)
		return -1;
	return _cellGrid[cellIdx * kFieldsPerCell + 3 + dir];
}

int16 ZoombiniPuzzleSlides::getCellState(int16 cellIdx) const {
	return _cellGrid[cellIdx * kFieldsPerCell + 1];
}

void ZoombiniPuzzleSlides::initGridByDifficulty() {
	_slotCellState = 504;
	_linkShapeVariantOffset = 48;
	// Level 4 selects its alternate slot base only after Snoid materialization and layout.
	if (_difficultyLevel == kPuzzleLevel4) {
		int16 randVal = _vm->_rnd->getRandomNumber(0, 1);
		_slotCellState += randVal;
		if (randVal != 0)
			_linkShapeVariantOffset = 24;
	}

	// Initialize all cells to inert state with invalid links
	for (int16 i = 0; i < kNumCells; i++) {
		int16 base = i * kFieldsPerCell;
		_cellGrid[base + 0] = 0;          // runnerIdx
		_cellGrid[base + 1] = kCellInert; // state
		_cellGrid[base + 2] = 0;          // data
		_cellGrid[base + 3] = -1;         // linkNW
		_cellGrid[base + 4] = -1;         // linkW
		_cellGrid[base + 5] = -1;         // linkSW
		_cellGrid[base + 6] = -1;         // linkSE
		_cellGrid[base + 7] = -1;         // linkE
		_cellGrid[base + 8] = -1;         // linkNE
	}

	// Clear adjacency flags
	memset(_cellAdjacencyMasks, 0, sizeof(_cellAdjacencyMasks));

	// Clear slot mapping
	memset(_slotCellIndices, -1, sizeof(_slotCellIndices));
	_slotCount = 0;

	snapshotZmbTraits();

	switch (_difficultyLevel) {
	case kPuzzleLevel1:
		generateTraitPairings();
		if (_pairGroupCount < ARRAYSIZE(kPairStartOffsets)) {
			int16 startCell = kPairStartOffsets[_pairGroupCount];
			int16 cellStep = kPairSpacingArray[_pairGroupCount];

			for (int16 pairIdx = 0; pairIdx < _pairGroupCount; pairIdx++) {
				int16 baseCell = startCell + cellStep * pairIdx;
				int16 offsetCell = 0;

				if (7 < _pairGroupCount && (baseCell % 2) != 0) {
					_cellGrid[baseCell * kFieldsPerCell + 1] = _slotCellState;
					_cellAdjacencyMasks[baseCell] = kAdjEast;
					for (int16 cell = baseCell + 1; cell < baseCell + 4; cell++) {
						_cellGrid[cell * kFieldsPerCell + 1] = _slotCellState;
						_cellAdjacencyMasks[cell] = kAdjWest | kAdjEast;
					}
					offsetCell = 3;
				}

				int16 slotBaseCell = baseCell + offsetCell;
				int16 slotCell = slotBaseCell + 1;
				_cellGrid[slotBaseCell * kFieldsPerCell + 1] = _slotCellState;
				_cellGrid[slotCell * kFieldsPerCell + 1] = kCellConnector;
				_slotCellIndices[_slotCount] = slotCell;
				_slotCount += 1;

				if (_pairLinkTypes[pairIdx] == kCellPath) {
					_cellAdjacencyMasks[slotBaseCell] |= kAdjEast;
					_cellAdjacencyMasks[slotCell] |= kAdjWest;
				} else {
					int16 traitCell = slotBaseCell + 2;
					int16 endSlotCell = slotBaseCell + 3;
					_cellGrid[traitCell * kFieldsPerCell + 1] = kCellPath;
					_cellGrid[traitCell * kFieldsPerCell + 2] = _pairLinkTypes[pairIdx];
					_cellGrid[endSlotCell * kFieldsPerCell + 1] = kCellConnector;
					_slotCellIndices[_slotCount] = endSlotCell;
					_slotCount += 1;
					_cellAdjacencyMasks[slotBaseCell] |= kAdjEast;
					_cellAdjacencyMasks[slotCell] = kAdjWest | kAdjEast;
					_cellAdjacencyMasks[traitCell] = kAdjWest | kAdjEast;
					_cellAdjacencyMasks[endSlotCell] |= kAdjWest;
				}
			}
		}
		buildHexAdjacencyTable();
		break;
	case kPuzzleLevel2:
		buildChainSequence();
		if (_pairGroupCount < ARRAYSIZE(kPairStartOffsets)) {
			int16 startCell = kPairStartOffsets[_pairGroupCount];
			int16 cellStep = kPairSpacingArray[_pairGroupCount];
			int16 pairTypeIdx = 0;
			int16 placedSlotCount = 0;

			for (int16 groupIdx = 0; groupIdx < _pairGroupCount; groupIdx++) {
				int16 baseCell = startCell + cellStep * groupIdx;
				_cellGrid[baseCell * kFieldsPerCell + 1] = _slotCellState;
				_cellGrid[(baseCell + 1) * kFieldsPerCell + 1] = kCellConnector;
				_slotCellIndices[_slotCount] = baseCell + 1;
				_slotCount += 1;
				placedSlotCount += 1;

				if (_pageLoadedZmbCount <= placedSlotCount) {
					_cellAdjacencyMasks[baseCell] = kAdjEast;
					_cellAdjacencyMasks[baseCell + 1] = kAdjWest;
					break;
				}

				if (pairTypeIdx < ARRAYSIZE(_pairLinkTypes) &&
					(_pairLinkTypes[pairTypeIdx] == 0 || _pairLinkTypes[pairTypeIdx] == kCellPath)) {
					if (_pairLinkTypes[pairTypeIdx] != 0) {
						_cellGrid[(baseCell + 2) * kFieldsPerCell + 1] = kCellPath;
						_cellGrid[(baseCell + 2) * kFieldsPerCell + 2] = 0;
						_cellGrid[(baseCell + 3) * kFieldsPerCell + 1] = kCellConnector;
						_slotCellIndices[_slotCount] = baseCell + 3;
						_slotCount += 1;
						_cellAdjacencyMasks[baseCell] |= kAdjEast;
						_cellAdjacencyMasks[baseCell + 1] = kAdjWest | kAdjEast;
						_cellAdjacencyMasks[baseCell + 2] = kAdjWest | kAdjEast;
						_cellAdjacencyMasks[baseCell + 3] |= kAdjWest;
						pairTypeIdx += 1;
						placedSlotCount += 1;
						if (_pageLoadedZmbCount <= placedSlotCount)
							break;
					}
				} else if (pairTypeIdx < ARRAYSIZE(_pairLinkTypes)) {
					_cellGrid[(baseCell + 2) * kFieldsPerCell + 1] = kCellPath;
					_cellGrid[(baseCell + 2) * kFieldsPerCell + 2] = _pairLinkTypes[pairTypeIdx];
					_cellGrid[(baseCell + 3) * kFieldsPerCell + 1] = kCellConnector;
					_slotCellIndices[_slotCount] = baseCell + 3;
					_slotCount += 1;
					_cellAdjacencyMasks[baseCell] |= kAdjEast;
					_cellAdjacencyMasks[baseCell + 1] = kAdjWest | kAdjEast;
					_cellAdjacencyMasks[baseCell + 2] = kAdjWest | kAdjEast;
					_cellAdjacencyMasks[baseCell + 3] |= kAdjWest;
					pairTypeIdx += 1;
					placedSlotCount += 1;
					if (_pageLoadedZmbCount <= placedSlotCount)
						break;
				}

				if (pairTypeIdx < ARRAYSIZE(_pairLinkTypes) &&
					(_pairLinkTypes[pairTypeIdx] == 0 || _pairLinkTypes[pairTypeIdx] == kCellPath)) {
					if (_pairLinkTypes[pairTypeIdx] != 0) {
						_cellGrid[(baseCell + 4) * kFieldsPerCell + 1] = kCellPath;
						_cellGrid[(baseCell + 4) * kFieldsPerCell + 2] = 0;
						_cellGrid[(baseCell + 5) * kFieldsPerCell + 1] = kCellConnector;
						_slotCellIndices[_slotCount] = baseCell + 5;
						_slotCount += 1;
						_cellAdjacencyMasks[baseCell + 3] |= kAdjEast;
						_cellAdjacencyMasks[baseCell + 4] = kAdjWest | kAdjEast;
						_cellAdjacencyMasks[baseCell + 5] = kAdjWest;
						pairTypeIdx += 1;
						placedSlotCount += 1;
						if (_pageLoadedZmbCount <= placedSlotCount)
							break;
					}
				} else if (pairTypeIdx < ARRAYSIZE(_pairLinkTypes)) {
					_cellGrid[(baseCell + 4) * kFieldsPerCell + 1] = kCellPath;
					_cellGrid[(baseCell + 4) * kFieldsPerCell + 2] = _pairLinkTypes[pairTypeIdx];
					_cellGrid[(baseCell + 5) * kFieldsPerCell + 1] = kCellConnector;
					_slotCellIndices[_slotCount] = baseCell + 5;
					_slotCount += 1;
					_cellAdjacencyMasks[baseCell + 3] |= kAdjEast;
					_cellAdjacencyMasks[baseCell + 4] = kAdjWest | kAdjEast;
					_cellAdjacencyMasks[baseCell + 5] = kAdjWest;
					pairTypeIdx += 1;
					placedSlotCount += 1;
					if (_pageLoadedZmbCount <= placedSlotCount)
						break;
				}
			}
		}
		buildHexAdjacencyTable();
		break;
	case kPuzzleLevel3: {
		for (int16 i = 0; i < ARRAYSIZE(kLeftEndpointCells); i++) {
			int16 cell = kLeftEndpointCells[i];
			_cellGrid[cell * kFieldsPerCell + 1] = _slotCellState;
			_cellAdjacencyMasks[cell] = kAdjEast;
			_cellAdjacencyMasks[cell + 1] = kAdjWest | kAdjSouthEast | kAdjNorthEast;
			_cellAdjacencyMasks[cell - 8] |= kAdjSouthWest;
			_cellAdjacencyMasks[cell + 10] |= kAdjNorthWest;
		}

		for (int16 i = 0; i < ARRAYSIZE(kRightEndpointCells); i++) {
			int16 cell = kRightEndpointCells[i];
			_cellGrid[cell * kFieldsPerCell + 1] = kCellConnector;
			_cellAdjacencyMasks[cell] = kAdjNorthWest | kAdjSouthWest;
			_cellAdjacencyMasks[cell - 10] |= kAdjSouthEast;
			_cellAdjacencyMasks[cell + 8] |= kAdjNorthEast;
		}

		for (int16 i = 0; i < ARRAYSIZE(kLeftArmLinkCells); i++) {
			_cellGrid[kLeftArmLinkCells[i] * kFieldsPerCell + 1] = kCellPath;
			_cellGrid[kRightArmLinkCells[i] * kFieldsPerCell + 1] = kCellConnector;
		}

		for (int16 i = 0; i < ARRAYSIZE(kInnerLinkPairs); i++) {
			int16 cell = kInnerLinkPairs[i];
			_cellAdjacencyMasks[cell] |= kAdjWest | kAdjEast;
			_cellAdjacencyMasks[cell - 1] |= kAdjEast;
			_cellAdjacencyMasks[cell + 1] |= kAdjWest;
		}

		buildHexAdjacencyTable();
		sortZmbsByOverlapCount();

		if (50 <= _vm->_rnd->getRandomNumber(0, 100)) {
			if (50 <= _vm->_rnd->getRandomNumber(0, 100)) {
				int16 result = placeNextZmbInCell(91);
				result = placeNextZmbInCell(19);
				placeNextZmbInCell(55);
				(void)result;
			} else {
				int16 result = placeNextZmbInCell(55);
				result = placeNextZmbInCell(91);
				placeNextZmbInCell(19);
				(void)result;
			}
		} else {
			int16 result = placeNextZmbInCell(19);
			result = placeNextZmbInCell(55);
			placeNextZmbInCell(91);
			(void)result;
		}

		maybeSetMatchTrait(12, 13, 11);
		maybeSetMatchTrait(30, 31, 29);
		maybeSetMatchTrait(48, 49, 47);
		maybeSetMatchTrait(66, 67, 65);
		maybeSetMatchTrait(84, 85, 83);
		maybeSetMatchTrait(102, 103, 101);

		int16 occupiedCount = 0;
		for (int16 i = 0; i < ARRAYSIZE(kRightArmLinkCells); i++) {
			if (_cellGrid[kRightArmLinkCells[i] * kFieldsPerCell + 1] == kCellOccupied)
				occupiedCount += 1;
		}

		if (_pageLoadedZmbCount < occupiedCount) {
			int16 extraCount = occupiedCount - _pageLoadedZmbCount;
			while (0 < extraCount) {
				bool removed = false;
				for (int16 i = 0; i < 6; i++) {
					int16 cell = kRightArmLinkCells[i];
					int16 base = cell * kFieldsPerCell;
					if (_cellGrid[base + 1] == kCellOccupied && _cellGrid[base - 7] == -1) {
						_cellGrid[base + 1] = kCellPath;
						_cellGrid[base + 2] = -1;
						extraCount -= 1;
						removed = true;
						break;
					}
				}

				if (removed)
					continue;

				for (int16 i = 6; i < 12; i++) {
					int16 cell = kRightArmLinkCells[i];
					int16 base = cell * kFieldsPerCell;
					if (_cellGrid[base + 1] == kCellOccupied && _cellGrid[base + 11] == -1) {
						_cellGrid[base + 1] = kCellPath;
						_cellGrid[base + 2] = -1;
						extraCount -= 1;
						removed = true;
						break;
					}
				}

				if (removed)
					continue;

				for (int16 i = 12; i < 15; i++) {
					int16 cell = kRightArmLinkCells[i];
					int16 base = cell * kFieldsPerCell;
					if (_cellGrid[base + 1] == kCellOccupied &&
						_cellGrid[base - 88] == -1 && _cellGrid[base + 74] == -1) {
						_cellGrid[base + 1] = kCellPath;
						_cellGrid[base + 2] = -1;
						extraCount -= 1;
						removed = true;
						break;
					}
				}

				if (!removed)
					break;
			}
		} else {
			int16 missingCount = _pageLoadedZmbCount - occupiedCount;
			while (0 < missingCount) {
				for (int16 i = 0; i < ARRAYSIZE(kRightArmLinkCells); i++) {
					int16 cell = kRightArmLinkCells[i];
					if (_cellGrid[cell * kFieldsPerCell + 1] == kCellOccupied)
						continue;

					_cellGrid[cell * kFieldsPerCell + 1] = kCellOccupied;
					missingCount -= 1;
					break;
				}
			}
		}

		for (int16 i = 0; i < kNumCells; i++) {
			int16 base = i * kFieldsPerCell;
			if (_cellGrid[base + 1] == kCellConnector) {
				_cellGrid[base + 1] = kCellPath;
				_cellGrid[base + 2] = -1;
			}
		}

		_slotCount = 0;
		for (int16 i = 0; i < ARRAYSIZE(kRightArmLinkCells); i++) {
			int16 cell = kRightArmLinkCells[i];
			if (_cellGrid[cell * kFieldsPerCell + 1] == kCellOccupied) {
				_slotCellIndices[_slotCount] = cell;
				_slotCount += 1;
			}
		}
		break;
	}
	case kPuzzleLevel4: {
		for (int16 i = 0; i < ARRAYSIZE(kLinkCellIndices); i++)
			setCellStateData(kLinkCellIndices[i], kCellPath, 0);
		for (int16 i = 0; i < ARRAYSIZE(kEvenRowLinkCells); i++)
			_cellAdjacencyMasks[kEvenRowLinkCells[i]] = kAdjSouthWest | kAdjNorthEast;
		for (int16 i = 0; i < ARRAYSIZE(kOddRowLinkCells); i++)
			_cellAdjacencyMasks[kOddRowLinkCells[i]] = kAdjNorthWest | kAdjSouthEast;

		setCellStateData(54, _slotCellState, 0);
		_cellAdjacencyMasks[54] = kAdjEast;
		_cellAdjacencyMasks[55] = kAdjWest | kAdjSouthEast | kAdjEast | kAdjNorthEast;
		_cellAdjacencyMasks[56] = kAdjWest | kAdjEast;
		_cellAdjacencyMasks[57] = kAdjNorthWest | kAdjWest | kAdjSouthWest | kAdjSouthEast | kAdjEast | kAdjNorthEast;
		_cellAdjacencyMasks[58] = kAdjWest | kAdjEast;
		_cellAdjacencyMasks[59] = kAdjNorthWest | kAdjWest | kAdjSouthWest | kAdjSouthEast | kAdjEast | kAdjNorthEast;
		_cellAdjacencyMasks[60] = kAdjWest | kAdjEast;
		_cellAdjacencyMasks[61] = kAdjNorthWest | kAdjWest | kAdjSouthWest | kAdjSouthEast | kAdjNorthEast;
		_cellAdjacencyMasks[19] = kAdjSouthEast | kAdjNorthEast;
		_cellAdjacencyMasks[21] = kAdjNorthWest | kAdjSouthWest | kAdjSouthEast | kAdjNorthEast;
		_cellAdjacencyMasks[23] = kAdjNorthWest | kAdjSouthWest | kAdjSouthEast | kAdjNorthEast;
		_cellAdjacencyMasks[25] = kAdjNorthWest | kAdjSouthWest | kAdjSouthEast;
		_cellAdjacencyMasks[38] = kAdjNorthWest | kAdjSouthWest | kAdjSouthEast | kAdjNorthEast;
		_cellAdjacencyMasks[40] = kAdjNorthWest | kAdjSouthWest | kAdjSouthEast | kAdjNorthEast;
		_cellAdjacencyMasks[42] = kAdjNorthWest | kAdjSouthWest | kAdjSouthEast | kAdjNorthEast;
		_cellAdjacencyMasks[44] = kAdjNorthWest | kAdjSouthWest;
		_cellAdjacencyMasks[74] = kAdjNorthWest | kAdjSouthWest | kAdjSouthEast | kAdjNorthEast;
		_cellAdjacencyMasks[76] = kAdjNorthWest | kAdjSouthWest | kAdjSouthEast | kAdjNorthEast;
		_cellAdjacencyMasks[78] = kAdjNorthWest | kAdjSouthWest | kAdjSouthEast | kAdjNorthEast;
		_cellAdjacencyMasks[80] = kAdjNorthWest | kAdjSouthWest;
		_cellAdjacencyMasks[91] = kAdjSouthEast | kAdjNorthEast;
		_cellAdjacencyMasks[93] = kAdjNorthWest | kAdjSouthWest | kAdjSouthEast | kAdjNorthEast;
		_cellAdjacencyMasks[95] = kAdjNorthWest | kAdjSouthWest | kAdjSouthEast | kAdjNorthEast;
		_cellAdjacencyMasks[97] = kAdjNorthWest | kAdjSouthWest | kAdjNorthEast;
		_cellAdjacencyMasks[2] = kAdjSouthWest | kAdjSouthEast;
		_cellAdjacencyMasks[4] = kAdjSouthWest | kAdjSouthEast;
		_cellAdjacencyMasks[6] = kAdjSouthWest | kAdjSouthEast;
		_cellAdjacencyMasks[110] = kAdjNorthWest | kAdjNorthEast;
		_cellAdjacencyMasks[112] = kAdjNorthWest | kAdjNorthEast;
		_cellAdjacencyMasks[114] = kAdjNorthWest | kAdjNorthEast;

		for (int16 i = 0; i < ARRAYSIZE(kSlotCellIndices); i++)
			setCellStateData(kSlotCellIndices[i], kCellConnector, 0);

		buildHexAdjacencyTable();

		if (_pageLoadedZmbCount <= 2) {
			sortZmbsByOverlapCount();
			clearBoard();
			setCellStateData(54, _slotCellState, 0);
			setCellStateData(55, kCellOccupied, _snoidIds[0]);
			_cellAdjacencyMasks[54] = kAdjEast;
			_cellAdjacencyMasks[55] = kAdjWest;
			_cellGrid[54 * kFieldsPerCell + 7] = 55;
			_cellGrid[55 * kFieldsPerCell + 4] = 54;

			if (_pageLoadedZmbCount == 2) {
				setCellStateData(56, kCellPath, 0);
				setCellStateData(57, kCellOccupied, _snoidIds[1]);
				_cellGrid[55 * kFieldsPerCell + 7] = 56;
				_cellGrid[56 * kFieldsPerCell + 4] = 55;
				_cellGrid[56 * kFieldsPerCell + 7] = 57;
				_cellGrid[57 * kFieldsPerCell + 4] = 56;
				_cellAdjacencyMasks[55] &= kAdjEast;
				_cellAdjacencyMasks[56] = kAdjWest | kAdjEast;
				_cellAdjacencyMasks[57] = kAdjWest;
				_sortedSnoidIndices[0] = -1;
				_sortedSnoidIndices[1] = 1;
				findMatchingZmbForCell(55, 4);
			}

			for (int16 i = 0; i < _pageLoadedZmbCount; i++)
				_sortedSnoidIndices[i] = -1;
		} else if (_pageLoadedZmbCount <= 5) {
			clearBoard();
			sortZmbsByOverlapCount();
			setCellStateData(54, _slotCellState, 0);
			setCellStateData(55, kCellOccupied, _snoidIds[0]);
			_cellAdjacencyMasks[54] = kAdjEast;
			_cellAdjacencyMasks[55] = kAdjWest | kAdjEast;

			setCellStateData(56, kCellPath, 0);
			setCellStateData(57, kCellOccupied, _snoidIds[1]);
			if (checkFirstTraitMatch(_sortedSnoidIndices[1], _sortedSnoidIndices[0]))
				_cellGrid[56 * kFieldsPerCell + 2] = _currentMatchedTraitIndex + kTraitHair;
			_cellAdjacencyMasks[56] = kAdjWest | kAdjEast;
			_cellAdjacencyMasks[57] = kAdjWest;

			if (3 <= _pageLoadedZmbCount) {
				setCellStateData(58, kCellPath, 0);
				setCellStateData(59, kCellOccupied, _snoidIds[2]);
				if (checkFirstTraitMatch(_sortedSnoidIndices[2], _sortedSnoidIndices[1]))
					_cellGrid[58 * kFieldsPerCell + 2] = _currentMatchedTraitIndex + kTraitHair;
				_cellAdjacencyMasks[57] = kAdjWest | kAdjEast;
				_cellAdjacencyMasks[58] = kAdjWest | kAdjEast;
				_cellAdjacencyMasks[59] = kAdjWest;
			}

			if (4 <= _pageLoadedZmbCount) {
				setCellStateData(60, kCellPath, 0);
				setCellStateData(61, kCellOccupied, _snoidIds[3]);
				if (checkFirstTraitMatch(_sortedSnoidIndices[3], _sortedSnoidIndices[2]))
					_cellGrid[60 * kFieldsPerCell + 2] = _currentMatchedTraitIndex + kTraitHair;
				_cellAdjacencyMasks[59] = kAdjWest | kAdjEast;
				_cellAdjacencyMasks[60] = kAdjWest | kAdjEast;
				_cellAdjacencyMasks[61] = kAdjWest;
			}

			if (5 <= _pageLoadedZmbCount) {
				setCellStateData(52, kCellPath, 0);
				setCellStateData(44, kCellOccupied, _snoidIds[4]);
				if (checkFirstTraitMatch(_sortedSnoidIndices[4], _sortedSnoidIndices[3]))
					_cellGrid[52 * kFieldsPerCell + 2] = _currentMatchedTraitIndex + kTraitHair;
				_cellAdjacencyMasks[61] = kAdjWest | kAdjNorthEast;
				_cellAdjacencyMasks[52] = kAdjSouthWest | kAdjNorthEast;
				_cellAdjacencyMasks[44] = kAdjSouthWest;
			}

			buildHexAdjacencyTable();
			for (int16 i = 0; i < _pageLoadedZmbCount; i++)
				_sortedSnoidIndices[i] = -1;
		} else {
			assignZmbToSlot(54);
			if (hasPendingZmb())
				reassignDeadSlots();
			scanAndResetActiveCells();

			pickNextCellForLink(93, 84, 83);
			pickNextCellForLink(21, 30, 29);
			pickNextCellForLink(112, 103, 102);
			pickNextCellForLink(4, 13, 12);
			pickNextCellForLink(95, 86, 85);
			pickNextCellForLink(23, 32, 31);
			pickNextCellForLink(78, 69, 68);
			pickNextCellForLink(42, 51, 50);
			pickNextCellForLink(76, 67, 66);
			pickNextCellForLink(114, 105, 104);
			pickNextCellForLink(6, 15, 14);
			pickNextCellForLink(44, 52, 34);
			pickNextCellForLink(80, 88, 70);

			if (_cellGrid[60 * kFieldsPerCell + 1] == kCellPath &&
				_cellGrid[61 * kFieldsPerCell + 1] == kCellPath &&
				_cellGrid[69 * kFieldsPerCell + 1] == kCellInert &&
				_cellGrid[51 * kFieldsPerCell + 1] == kCellInert &&
				_cellGrid[52 * kFieldsPerCell + 1] == kCellInert &&
				_cellGrid[70 * kFieldsPerCell + 1] == kCellInert) {
				resetCellToEmpty(60);
				resetCellToEmpty(61);
			}
		}

		rebuildOccupiedSlots();
		break;
	}
	default:
		warning("slides: unknown difficulty level %d", _difficultyLevel);
		break;
	}

	for (int16 i = 0; i < kNumCells; i++) {
		if (_cellGrid[i * kFieldsPerCell + 1] == kCellOccupied)
			_cellGrid[i * kFieldsPerCell + 1] = kCellConnector;
	}

}

// =============================================================================
// Hex Adjacency Table
// =============================================================================

void ZoombiniPuzzleSlides::buildHexAdjacencyTable() {
	for (int16 cellIdx = 0; cellIdx < kNumCells; cellIdx++) {
		int16 base = cellIdx * kFieldsPerCell;
		for (int16 linkIdx = 0; linkIdx < 6; linkIdx++)
			_cellGrid[base + 3 + linkIdx] = -1;

		int16 row = cellIdx / 9;
		int16 col = cellIdx % 9;
		bool oddRow = 8 < (cellIdx % 18);
		uint16 adjMask = _cellAdjacencyMasks[cellIdx];
		int16 nwCell = -1;
		int16 neCell = -1;
		int16 swCell = -1;
		int16 seCell = -1;
		if (0 < row) {
			if (oddRow) {
				nwCell = cellIdx - 9;
				if (col < 8)
					neCell = cellIdx - 8;
			} else {
				if (0 < col)
					nwCell = cellIdx - 10;
				neCell = cellIdx - 9;
			}
		}

		int16 westCell = (0 < col) ? static_cast<int16>(cellIdx - 1) : -1;
		int16 eastCell = (col < 8) ? static_cast<int16>(cellIdx + 1) : -1;

		if (row < 12) {
			if (oddRow) {
				swCell = cellIdx + 9;
				if (col < 8)
					seCell = cellIdx + 10;
			} else {
				if (0 < col)
					swCell = cellIdx + 8;
				seCell = cellIdx + 9;
			}
		}

		setLinkIfValid(adjMask, base, kAdjNorthWest, 3, nwCell);
		setLinkIfValid(adjMask, base, kAdjWest, 4, westCell);
		setLinkIfValid(adjMask, base, kAdjSouthWest, 5, swCell);
		setLinkIfValid(adjMask, base, kAdjSouthEast, 6, seCell);
		setLinkIfValid(adjMask, base, kAdjEast, 7, eastCell);
		setLinkIfValid(adjMask, base, kAdjNorthEast, 8, neCell);
	}

}

// =============================================================================
// Trait Snapshot
// =============================================================================

void ZoombiniPuzzleSlides::snapshotZmbTraits() {
	for (int16 i = 0; i < _pageLoadedZmbCount && i < 16; i++) {
		ZmbSnoid *snoid = getSnoid(10000 + i);
		if (!snoid)
			continue;

		_snoidTraits[i] = snoid->_trait;
		_snoidIds[i] = snoid->getId();
	}
}

// =============================================================================
// Trait Pairing
// =============================================================================

void ZoombiniPuzzleSlides::generateTraitPairings() {
	ZmbTrait traits[16];

	for (int16 i = 0; i < _pageLoadedZmbCount; i++)
		traits[i] = _snoidTraits[i];

	int16 traitCursor = _vm->_rnd->getRandomNumber(0, 3);
	int16 unpairedCount = 0;
	for (int16 attempt = 0; attempt < 10; attempt++) {
		memset(_pairLinkTypes, 0, sizeof(_pairLinkTypes));
		memset(_pairingUsageStates, 0, sizeof(_pairingUsageStates));
		_pairGroupCount = 0;

		for (int16 snoidIdx = 0; snoidIdx < _pageLoadedZmbCount; snoidIdx++) {
			if (_pairingUsageStates[snoidIdx] != 0)
				continue;

			int16 tries = 4;
			while (0 < tries && _pairingUsageStates[snoidIdx] == 0) {
				traitCursor += 1;
				if (3 < traitCursor)
					traitCursor = 0;

				for (int16 comparisonSnoidIdx = snoidIdx + 1; comparisonSnoidIdx < _pageLoadedZmbCount; comparisonSnoidIdx++) {
					if (_pairingUsageStates[comparisonSnoidIdx] != 0)
						continue;

					const bool matched = traits[snoidIdx][traitCursor] == traits[comparisonSnoidIdx][traitCursor];

					if (!matched)
						continue;

					_pairLinkTypes[_pairGroupCount] = kTraitHair + traitCursor;
					_pairGroupCount += 1;
					_pairingUsageStates[comparisonSnoidIdx] = 1;
					_pairingUsageStates[snoidIdx] = 1;
					break;
				}

				tries -= 1;
				if (tries == 0 && _pairingUsageStates[snoidIdx] == 0) {
					_pairingUsageStates[snoidIdx] = 99;
					_pairLinkTypes[_pairGroupCount] = kCellPath;
					_pairGroupCount += 1;
					unpairedCount += 1;
				}
			}
		}

		if (unpairedCount == 0 || (unpairedCount == 1 && (_pageLoadedZmbCount % 2) == 1))
			break;

		for (int16 traitIdx = _pageLoadedZmbCount - 1; 0 <= traitIdx; traitIdx--) {
			if (_pairingUsageStates[traitIdx] != 99 || _pairingUsageStates[0] == 99)
				continue;

			SWAP(traits[traitIdx], traits[0]);
		}
	}

}

// =============================================================================
// Per-Frame Update
// =============================================================================

void ZoombiniPuzzleSlides::onEveryFrame() {
	// The v1.x releases defer the entrance sound until the page fade has completed.
	updateEntranceSound();

	if (_pageLoadedZmbCount <= 0)
		return;
	if (isDeparturePending())
		return;
	if (_departureState == ZmbDepartureState::kTriggered) {
		finishSolvedDepartureSequence();
		if (_departureState != ZmbDepartureState::kTriggered)
			return;
	}

	if (_victoryPaletteActive) {
		if (6 < getCurrentFrameCounter() - _lastVictoryPaletteFrame) {
			_vm->_gfx->rotatePaletteRight(243, 3);
			_lastVictoryPaletteFrame = getCurrentFrameCounter();
		}
	}

	// Consume at most one celebration opportunity on each Slides page visit.
	// The opportunity is consumed before its frame deadline and runner search,
	// so it remains consumed even when no celebration starts.
	// The session state preserves the index and non-repeat pool across page
	// replacement and resets them after every loaded Zoombini is selected once.
	ZoombiniGameState::SlidesCelebrationState &celebrationState = _vm->_state->getSlidesCelebrationState();
	if (_celebrationVisitLatched || !_fullMatchTriggerCount || _celebrationCycleTarget <= celebrationState._visitCount) {
		if (_celebrationCycleTarget <= celebrationState._visitCount) {
			celebrationState._poolState = 0;
			celebrationState._lastFrame = 0;
			_fullMatchTriggerCount = 0;
			celebrationState._visitCount = 0;
		}
	} else {
		_celebrationVisitLatched = true;
		if (30 < getCurrentFrameCounter() - celebrationState._lastFrame) {
			celebrationState._lastFrame = getCurrentFrameCounter();
			bool triggered = false;
			int16 attempts = 0;

			do {
				uint16 poolIdx = _vm->_rnd->getNonRepeatRandom(_pageLoadedZmbCount, celebrationState._poolState);
				uint16 snoidId = 10000 + poolIdx;
				ZmbSnoid *snoid = getIdleSnoid(snoidId);

				// Do not replace a placement or another active animation with celebration SCRS playback.
				if (snoid &&
					snoid->isRenderActivated() &&
					snoid->hasFlag(ZmbFeature::FLAG_00000001_TYPE_SNOID)) {
					// Select SCRS 13001-13005 from the one-based feet trait.
					int16 scrsId = static_cast<int16>(snoid->_trait._feet - 1 + kResScrs13001_NormalBase);
					if (snoid->startScrsPlayback(ZmbResource(ZmbResource::kPage, scrsId), ZmbScrsCompletionMode::kReturnToIdle, resolveScrsRejectState(scrsId))) {
						celebrationState._visitCount += 1;
						triggered = true;
					}
				} else {
					attempts += 1;
					if (20 < attempts)
						triggered = true;
				}
			} while (!triggered);
		}
	}
}

void ZoombiniPuzzleSlides::playEntranceSound() {
	// Shared page cleanup installs system SND 996-997 as the first priority range before Slides appends its page ranges.
	// Queueing the entrance sound therefore gives it priority 32 over unmatched Snoid fidget voices.
	queueScriptSoundForNextRenderPass(ZmbResource(ZmbResource::kSystem, kSysResSound0997_ArriveSFX));
}

void ZoombiniPuzzleSlides::updateEntranceSound() {
	if (!_entranceSoundPending)
		return;

	if (_vm->_gfx->isFading()) {
		_entranceFadeObserved = true;
		return;
	}

	if (!_entranceFadeObserved)
		return;

	playEntranceSound();
	_entranceSoundPending = false;
}

void ZoombiniPuzzleSlides::beginSolvedDepartureSequence() {
	if (isDepartureActive() || !_goButtonEnabled || !_boardInputEnabled)
		return;

	// Locked cells are marked accepted before SCRB 7002 starts and before the later walk-off animation.
	// Normalize every occupant even when the page-local celebration trigger was already cleared.
	// A still-running celebration SCRS may otherwise leave its temporary offset
	// in the frame that rises with the cell.
	_fullMatchTriggerCount = 0;
	for (int16 cellIdx = 0; cellIdx < kNumCells; cellIdx++) {
		if (_cellGrid[cellIdx * kFieldsPerCell + 1] != kCellLocked)
			continue;

		ZmbSnoid *snoid = getSnoid(_cellGrid[cellIdx * kFieldsPerCell + 2]);
		if (snoid) {
			Common::Point target = getCellSnoidPosition(cellIdx);
			snoid->setAnimState(kSnoidAnimState000_Idle, &target);
		}
	}

	markMatchedRunnersDone();
	updateNeighborFlags();

	_boardInputEnabled = false;
	_departureState = ZmbDepartureState::kTriggered;
	_solvedRiseTimingFeature = nullptr;
	_solvedRiseTailStartFrame = 0;
	_solvedRiseTailQueued = false;

	for (int16 cellIdx = 0; cellIdx < kNumCells; cellIdx++) {
		int16 state = _cellGrid[cellIdx * kFieldsPerCell + 1];
		ZmbFeature *cellFeature = nullptr;
		if (state == kCellMatched || state == kCellLocked || state == _slotCellState) {
			ensureCellFeature(cellIdx);
			cellFeature = _cellFeatures[cellIdx];
			if (cellFeature) {
				// The completion owner arbitrates its cell sound with the solved-rise sounds.
				// Clear any earlier immediate instance before replacing the cell script;
				// the replacement is assigned to the feature and timing-group queue below.
				cellFeature->stopFrameSounds();
				setCellFeaturePreRenderHook(cellFeature, kResScrb7002_MatchedCell);
				loadScrbOntoFeature(cellFeature, kResScrb7002_MatchedCell, true);
				cellFeature->activateAnimate();
				cellFeature->setScriptSoundPolicy(ZmbFeature::ScriptSoundPolicy::kPriorityQueue);
				if (_solvedRiseTimingFeature == nullptr)
					_solvedRiseTimingFeature = cellFeature;
			}
		}

		if (state == kCellLocked) {
			ZmbSnoid *snoid = getSnoid(_cellGrid[cellIdx * kFieldsPerCell + 2]);
			if (!snoid)
				continue;

			// Complete the immediate-to-queued handoff before SCRS 13000 can
			// submit its lower-priority voice. Stop only this accepted Snoid's
			// owned handles; unrelated Slides feedback remains immediate.
			snoid->stopFrameSounds();
			const bool scrsStarted = snoid->startScrsPlayback(ZmbResource(ZmbResource::kPage, kResScrs13000_RejectBase), ZmbScrsCompletionMode::kReturnToIdle, resolveScrsRejectState(kResScrs13000_RejectBase));
			if (scrsStarted && cellFeature) {
				registerFeatureTimingGroup(cellFeature, snoid);
				// The completion owner covers both the rising-cell SCRB and its accepted Snoid SCRS.
				// Queue the whole authored timing group.
				// Page SND 7000 can then suppress lower-priority Snoid voice candidates during the rise.
				setFeatureTimingGroupScriptSoundPolicy(cellFeature, ZmbFeature::ScriptSoundPolicy::kPriorityQueue);
			}
		}
	}

	// Restore the solved-grid layer and Snoid order before rising-cell playback.
	relinkGridLayers();

	unlockInteractiveSlots();
	// Retain kResScrb7000_CellBase as the priority winner instead of starting a parallel mixer stream.
	// Its MHWK Data header plays the prefix from sample frame zero through 4652.
	// Playback then repeats the authored [1320, 4653) range indefinitely.
	// The first installed cell feature owns the later kResScrb7001_InertCell handoff.
	// That handoff stops the exact retained loop handle before starting the tail.
	// Embedded MHWK loops are applied automatically to every Zoombini SND.
	const ZmbResource riseSound(ZmbResource::kPage, kResSound7000_SolvedRiseBase);
	if (_solvedRiseTimingFeature)
		dispatchFeatureSound(_solvedRiseTimingFeature, riseSound);
	else
		queueScriptSoundForNextRenderPass(riseSound);
}

void ZoombiniPuzzleSlides::finishSolvedDepartureSequence() {
	// Wait until the first installed SCRB 7002 timing slot clears.
	// @ref ZmbFeature::FLAG_00100000_PLAY_ONCE marks the exact boundary.
	// @ref ZoombiniPuzzleSlides::onFeatureAnimEvent() receives it and queues SND 7001.
	// Departure remains blocked until that owner fires.
	if (!_solvedRiseTailQueued) {
		// Test the shared timing slot on every hover-loop iteration.
		// Once the final SCRB group is visible, wait for the owner's actual deadline.
		// Request rendering at that deadline so the gate cannot add another host frame.
		// @ref ZoombiniPage::preRenderFeature() still owns the PLAY_ONCE completion decision.
		if (_solvedRiseTimingFeature &&
			_solvedRiseTimingFeature->getLastFrameIdx() ==
				_solvedRiseTimingFeature->getMaxFrameIdx() &&
			_solvedRiseTimingFeature->isAnimationTimerDue(getCurrentFrameCounter())) {
			scheduleForceRedraw();
		}
		return;
	}

	// Wait 60 frames after SND 7001.
	// The accepted-Snoid timing groups must also be idle before every walker can begin departure.
	if (getCurrentFrameCounter() - _solvedRiseTailStartFrame < 60)
		return;

	for (int16 cellIdx = 0; cellIdx < kNumCells; cellIdx++) {
		if (_cellGrid[cellIdx * kFieldsPerCell + 1] != kCellLocked)
			continue;

		ZmbSnoid *snoid = getSnoid(_cellGrid[cellIdx * kFieldsPerCell + 2]);
		if (snoid && snoid->getAnimState() != kSnoidAnimState000_Idle)
			return;
	}

	_solvedRiseTimingFeature = nullptr;

	for (int16 cellIdx = 0; cellIdx < kNumCells; cellIdx++) {
		const int16 state = _cellGrid[cellIdx * kFieldsPerCell + 1];
		if (state != kCellMatched && state != kCellLocked && state != _slotCellState)
			continue;

		ZmbFeature *cellFeature = _cellFeatures[cellIdx];
		if (cellFeature) {
			cellFeature->setScriptSoundPolicy(ZmbFeature::ScriptSoundPolicy::kInheritPage);
		}
		if (state == kCellLocked)
			unregisterFeatureTimingGroup(getSnoid(_cellGrid[cellIdx * kFieldsPerCell + 2]));
	}

	if (_difficultyLevel == kPuzzleLevel3 || _difficultyLevel == kPuzzleLevel4)
		selectAdvancedDepartureRunners();

	if (_difficultyLevel <= kPuzzleLevel2)
		startDepartWalkAnimation(Common::Point(1280, 240));
	else
		startDepartWalkAnimation(Common::Point(800, 200));

	// Advanced boards move only their selected branch anchors, but every locked
	// Snoid remains part of the continuing party serialized after the walk starts.
	if (_difficultyLevel == kPuzzleLevel3 || _difficultyLevel == kPuzzleLevel4)
		markMatchedRunnersDone();

	// The common departure path owns and tracks the sole SND 996 instance.
	ZoombiniInteractive::onGoButtonActivated();
}

void ZoombiniPuzzleSlides::selectAdvancedDepartureRunners() {
	// The departure controller takes one snapshot of occupied idle Snoids.
	// Clear the continuing-party markers so only the authored branch anchors
	// enter that snapshot on the two advanced boards.
	schedulePackSnoids(false, false);

	switch (_difficultyLevel) {
	case kPuzzleLevel3:
		markCellRunnerForDeparture(19);
		markCellRunnerForDeparture(55);
		markCellRunnerForDeparture(91);
		break;
	case kPuzzleLevel4:
		markCellRunnerForDeparture(55);
		if (cellStateIs(46, kCellMatched))
			markCellRunnerForDeparture(38);
		if (cellStateIs(64, kCellMatched))
			markCellRunnerForDeparture(74);
		break;
	default:
		break;
	}
}

void ZoombiniPuzzleSlides::markCellRunnerForDeparture(int16 cellIdx) {
	if (!cellStateIs(cellIdx, kCellLocked))
		return;

	ZmbSnoid *snoid = getSnoid(_cellGrid[cellIdx * kFieldsPerCell + 2]);
	if (snoid)
		snoid->_packIsOccupied = true;
}

// =============================================================================
// Animation Event Handling
// =============================================================================

void ZoombiniPuzzleSlides::onFeatureAnimEvent(ZmbFeature *feature, int16 eventCode) {
	if (eventCode == kAnimEventM1_End &&
		feature == _solvedRiseTimingFeature &&
		_departureState == ZmbDepartureState::kTriggered && !_solvedRiseTailQueued) {
		// The render that clears the shared timing slot queues SND 7001 and requests another immediate render.
		// The PLAY_ONCE end event fires during the pre-render pass.
		// Its sound queue flushes at the end of the same pass.
		// That handoff boundary replaces the retained SND 7000.
		dispatchFeatureSound(feature, ZmbResource(ZmbResource::kPage, kResSound7001_SolvedRiseTail));
		_solvedRiseTailQueued = true;
		_solvedRiseTailStartFrame = getCurrentFrameCounter();
		return;
	}

	ZmbSnoid *travelSnoid = getSnoid(_invalidDropSnoidId);
	if (kSlidesEventCode090_TravelVariant0 <= eventCode &&
		eventCode <= kSlidesEventCode093_TravelVariant3 &&
		(feature == _invalidDropEffectFeature || feature == travelSnoid)) {
		handleZmbTravelEvent(eventCode);
		return;
	}

	if (!feature->hasFlag(ZmbFeature::FLAG_00000001_TYPE_SNOID))
		return;

	ZmbSnoid *snoid = static_cast<ZmbSnoid *>(feature);

	if (eventCode == kSlidesEventCode000_ToggleSnoidFacing) {
		// The travel Snoid reached its facing marker.
		// Toggle facing and apply the body arrangement queued by the travel SCRS.
		// Toggle facing + apply pending body arrangement.
		// The event-0 toggle changes facing-left state, not render visibility.
		// Apply and clear a pending body arrangement here.
		// Toggling render would instead deadlock the SCRS playback (hidden snoids skip the anim state machine).
		snoid->setFacingLeft(!snoid->isFacingLeft());
		applyPendingBodyArrangement(*snoid);
	} else {
		handleBodyArrangementScriptEvent(*snoid, eventCode);
	}
}

// =============================================================================
// Input Handling
// =============================================================================

ZmbEventHandleResult ZoombiniPuzzleSlides::onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) {
	// Restore the saved bare Snoid bitmask before dispatching any button or puzzle click.
	restoreInitialSnoidFlags();

	// Let the base class handle button clicks first
	ZmbEventHandleResult result = ZoombiniInteractive::onLButtonDown(absPos, relPos);
	if (result == ZmbEventHandleResult::kConsumed)
		return result;

	// The solved sequence owns the board after Go.
	// Later clicks may only reach the shared departure-skip handling above.
	if (!_boardInputEnabled)
		return ZmbEventHandleResult::kConsumed;

	// Guard: already dragging
	if (isDragging())
		return ZmbEventHandleResult::kPassthrough;

	// The selected runner must still be the exact idle runner with that ID.
	ZmbSnoid *hitSnoid = findSnoidAtPoint(absPos);
	ZmbSnoid *snoid = hitSnoid ? getIdleSnoid(hitSnoid->getId()) : nullptr;
	if (!snoid)
		return ZmbEventHandleResult::kPassthrough;

	// Take the accepted-count and cell-index checksum snapshot before the drag mutates the grid.
	_acceptedCellCountBeforeDrag = countAcceptedCellsAndUpdateChecksum();
	_acceptedCellIndexChecksumBeforeDrag = _acceptedCellIndexChecksum;

	// Begin drag
	startSnoidDrag(snoid, absPos);

	return ZmbEventHandleResult::kConsumed;
}

void ZoombiniPuzzleSlides::endDrag(const Common::Point &dropPos) {
	(void)dropPos;
	const bool fromDrawOnRegSlot = 0 <= _dragSourceDrawOnRegSlot;
	ZmbSnoid *snoid = finishSnoidDrag();

	if (!snoid)
		return;

	int16 sourceCell = -1;
	for (int16 cellIdx = 0; cellIdx < kNumCells; cellIdx++) {
		int16 state = _cellGrid[cellIdx * kFieldsPerCell + 1];
		if ((state == kCellOccupied || state == kCellLocked) &&
			_cellGrid[cellIdx * kFieldsPerCell + 2] == snoid->getId()) {
			sourceCell = cellIdx;
			break;
		}
	}

	// Test a 15-pixel square around the dragged Snoid root, not a circle around the mouse release point.
	int16 targetCell = findCellAtPosition(snoid->getPointLoc());

	if (0 <= targetCell && (targetCell == sourceCell || isCellValidDropTarget(targetCell))) {
		if (0 <= sourceCell && sourceCell != targetCell) {
			_cellGrid[sourceCell * kFieldsPerCell + 1] = kCellConnector;
			_cellGrid[sourceCell * kFieldsPerCell + 2] = 0;
			syncCellFeatureScript(sourceCell);
			if (_difficultyLevel <= kPuzzleLevel2)
				orphanLinearChainFromCell(sourceCell);
		}

		assignZmbToSlot(snoid, targetCell);
	} else {
		if (0 <= sourceCell) {
			_cellGrid[sourceCell * kFieldsPerCell + 1] = kCellConnector;
			_cellGrid[sourceCell * kFieldsPerCell + 2] =
				(_difficultyLevel <= kPuzzleLevel2) ? 0 : -1;
			syncCellFeatureScript(sourceCell);
			if (_difficultyLevel <= kPuzzleLevel2)
				orphanLinearChainFromCell(sourceCell);
			else
				resetAnimStates();
		}

		// Start the arrival state before SCRB 8002 can interrupt the shared turn.
		// This lets SCRS 14003 land facing front or right, but never left.
		snoid->setAnimTargetPos(snoid->getPointLoc());
		snoid->setAnimState(kSnoidAnimState004_Arrive);
		if (!beginZmbTravel(snoid)) {
			const bool settledAtRelease = settleSnoidAfterTerrainDrop(snoid, _dragOrigPos);
			// Every difficulty uses the same DRAW_ON_REG-source fallback. A changed
			// terrain destination returns to an empty pack seat, while exact valid
			// terrain remains free.
			if (fromDrawOnRegSlot && !settledAtRelease) {
				const Common::Point returnPos = findRandomEmptySnoidSlotPosition(kSnoidPositions, ARRAYSIZE(kSnoidPositions), 500);
				settleSnoidAtTarget(snoid, returnPos);
			}
		}
	}

	playPlacementFeedbackSFX();
	refreshGoButtonEnabledState();
}

int16 ZoombiniPuzzleSlides::findCellAtPosition(const Common::Point &pos) const {
	int16 drawOnRegIdx = hitTestDrawOnRegSlot(pos, _clickZoneRadius, true);
	if (drawOnRegIdx < 0)
		return -1;

	for (int16 slotIdx = 0; slotIdx < _slotCount; slotIdx++) {
		if (_slotDrawOnRegIndices[slotIdx] == drawOnRegIdx)
			return _slotCellIndices[slotIdx];
	}
	return -1;
}

int16 ZoombiniPuzzleSlides::findSlotIndexForCell(int16 cellIdx) const {
	for (int16 slotIdx = 0; slotIdx < _slotCount; slotIdx++) {
		if (_slotCellIndices[slotIdx] == cellIdx)
			return slotIdx;
	}
	return -1;
}

bool ZoombiniPuzzleSlides::isCellValidDropTarget(int16 cellIdx) const {
	if (cellIdx < 0 || kNumCells <= cellIdx)
		return false;

	int16 state = _cellGrid[cellIdx * kFieldsPerCell + 1];

	if (state != kCellConnector)
		return false;

	for (int16 slotIdx = 0; slotIdx < _slotCount; slotIdx++) {
		if (_slotCellIndices[slotIdx] == cellIdx)
			return true;
	}
	return false;
}

void ZoombiniPuzzleSlides::assignZmbToSlot(ZmbSnoid *snoid, int16 cellIdx) {
	int16 base = cellIdx * kFieldsPerCell;

	// Every valid seat enters Arrive state 4 for one-tick alignment and settles facing right.
	settleSnoidAtTarget(snoid, getCellSnoidPosition(cellIdx));

	// Update cell state
	_cellGrid[base + 1] = kCellOccupied;
	_cellGrid[base + 2] = snoid->getId(); // Store runner ID in data field
	int16 slotIdx = findSlotIndexForCell(cellIdx);
	if (0 <= slotIdx && 0 <= _slotDrawOnRegIndices[slotIdx])
		setDrawOnRegOccupant(_slotDrawOnRegIndices[slotIdx], snoid->getId());
	syncCellFeatureScript(cellIdx);

	// Check for trait matches
	if (_difficultyLevel <= kPuzzleLevel2) {
		validateChainAndMarkMatched(cellIdx);
		checkVictoryCondition();
	} else {
		resetAnimStates();
	}
}

void ZoombiniPuzzleSlides::orphanLinearChainFromCell(int16 sourceCellIdx) {
	// Level 1/2 chains occupy consecutive cells. Removing a seated Snoid
	// invalidates the link immediately before it and every accepted cell after
	// it, up to the end of the two- or three-Snoid chain.
	if (sourceCellIdx < 0 || kNumCells <= sourceCellIdx)
		return;

	if (0 < sourceCellIdx) {
		int16 precedingLinkCell = sourceCellIdx - 1;
		int16 &precedingState = _cellGrid[precedingLinkCell * kFieldsPerCell + 1];
		if (precedingState == kCellMatched) {
			precedingState = kCellPath;
			syncCellFeatureScript(precedingLinkCell);
		}
	}

	for (int16 cellOffset = 1; cellOffset < 6; cellOffset++) {
		int16 cellIdx = sourceCellIdx + cellOffset;
		if (kNumCells <= cellIdx)
			break;

		int16 &state = _cellGrid[cellIdx * kFieldsPerCell + 1];
		if (state == kCellInert)
			break;
		if (state == kCellMatched)
			state = kCellPath;
		else if (state == kCellLocked)
			state = kCellOccupied;
		else
			continue;

		syncCellFeatureScript(cellIdx);
	}
}

int16 ZoombiniPuzzleSlides::assignZmbToSlot(int16 slotBaseCell) {
	static constexpr int16 kCascadeCells[12] = {
		40, 76, 23, 95, 42, 78, 38, 74, 21, 93, 19, 91};

	sortZmbsByOverlapCount();
	int16 slotCell = slotBaseCell + 1;
	_cellGrid[slotCell * kFieldsPerCell + 1] = kCellOccupied;

	if (_pageLoadedZmbCount == 1) {
		_cellGrid[slotCell * kFieldsPerCell + 2] = _snoidIds[0];
		_sortedSnoidIndices[0] = -1;
		return _snoidIds[0];
	}

	for (int16 snoidIdx = 0; snoidIdx < _pageLoadedZmbCount; snoidIdx++) {
		if (_pageLoadedZmbCount <= snoidIdx + 1)
			break;
		if (!checkFirstTraitMatch(snoidIdx + 1, snoidIdx))
			continue;

		int16 sortedIdx = _sortedSnoidIndices[snoidIdx + 1];
		if (0 <= sortedIdx) {
			_cellGrid[slotCell * kFieldsPerCell + 2] = _snoidIds[sortedIdx];
			_sortedSnoidIndices[snoidIdx + 1] = -1;
		}
		break;
	}

	int16 result = moveZmbToCell(slotCell);
	if (result != 0) {
		if (result != -1)
			result = moveZmbToCell(result);
	} else {
		int16 nextCell = slotCell + 2;
		result = moveZmbToCell(nextCell);
		if (result == 0)
			result = moveZmbToCell(nextCell + 2);
	}

	for (uint i = 0; i < ARRAYSIZE(kCascadeCells) && hasPendingZmb(); i++)
		moveZmbToCell(kCascadeCells[i]);

	return result;
}

void ZoombiniPuzzleSlides::moveZmbToCell(ZmbSnoid *snoid, int16 cellIdx) {
	Common::Point targetPos = getCellSnoidPosition(cellIdx);
	snoid->setAnimTargetPos(targetPos);
	snoid->setFacingLeft(false);
	snoid->setAnimState(kSnoidAnimState000_Idle, &targetPos);
}

int16 ZoombiniPuzzleSlides::moveZmbToCell(int16 moveData) {
	if (!hasPendingZmb())
		return -1;

	if (_cellGrid[moveData * kFieldsPerCell + 1] != kCellOccupied) {
		int16 farNE = getCellLink(getCellLink(moveData, 5), 5);
		int16 farSE = getCellLink(getCellLink(moveData, 3), 3);
		if (farNE < 0 || _cellGrid[farNE * kFieldsPerCell + 1] != kCellOccupied) {
			if (farSE < 0 || _cellGrid[farSE * kFieldsPerCell + 1] != kCellOccupied)
				return -1;
			if (findMatchingZmbForCell(farSE, 0) == -1)
				return -1;
		} else if (findMatchingZmbForCell(farNE, 2) == -1) {
			return -1;
		}
	}

	int16 farNE = getCellLink(getCellLink(moveData, 5), 5);
	int16 farSE = getCellLink(getCellLink(moveData, 3), 3);

	if ((moveData == 55 || moveData == 57 || moveData == 59) && findMatchingZmbForCell(moveData, 4) == -1)
		return -1;

	int16 nextCell = moveData + 2;
	if (0 <= farNE && _cellGrid[farNE * kFieldsPerCell + 1] == kCellConnector) {
		if (!hasPendingZmb())
			return -1;
		if (findMatchingZmbForCell(moveData, 5) == -1) {
			_cellGrid[farNE * kFieldsPerCell + 1] = kCellPath;
			return nextCell;
		}
	}

	if (nextCell < kNumCells && _cellGrid[nextCell * kFieldsPerCell + 1] == kCellOccupied && 0 <= farNE) {
		int16 trait = pickRandomMatchingTrait(nextCell, farNE);
		if (trait != 0) {
			int16 middleCell = getCellLink(farNE, 3);
			if (0 <= middleCell)
				_cellGrid[middleCell * kFieldsPerCell + 2] = trait;
		}
	}

	if (0 <= farSE && _cellGrid[farSE * kFieldsPerCell + 1] == kCellConnector) {
		if (!hasPendingZmb())
			return -1;
		if (findMatchingZmbForCell(moveData, 3) == -1) {
			_cellGrid[farSE * kFieldsPerCell + 1] = kCellPath;
			return nextCell;
		}
		if (nextCell < kNumCells && _cellGrid[nextCell * kFieldsPerCell + 1] == kCellOccupied) {
			int16 trait = pickRandomMatchingTrait(nextCell, farSE);
			if (trait != 0) {
				int16 middleCell = getCellLink(farSE, 5);
				if (0 <= middleCell)
					_cellGrid[middleCell * kFieldsPerCell + 2] = trait;
			}
		}
	}

	return 0;
}

void ZoombiniPuzzleSlides::clearCellToEmpty(int16 cellIdx) {
	int16 base = cellIdx * kFieldsPerCell;
	_cellGrid[base + 1] = kCellInert;
	for (int16 direction = 0; direction < 6; direction++)
		_cellGrid[base + 3 + direction] = -1;
	_cellAdjacencyMasks[cellIdx] = 0;
	syncCellFeatureScript(cellIdx);
}

void ZoombiniPuzzleSlides::resetCellToEmpty(int16 cellIdx) {
	int16 base = cellIdx * kFieldsPerCell;
	_cellGrid[base + 1] = kCellInert;
	_cellGrid[base + 2] = 0;

	// Clear all link fields and corresponding adjacency bits
	for (int16 i = 0; i < 6; i++) {
		int16 neighborCell = _cellGrid[base + 3 + i];
		if (0 <= neighborCell && neighborCell < kNumCells) {
			// Clear the reverse bit on the neighbor
			uint16 reverseBit = 0;
			switch (i) {
			case 0:
				reverseBit = kAdjSouthEast;
				break; // NW -> SE
			case 1:
				reverseBit = kAdjEast;
				break; // W -> E
			case 2:
				reverseBit = kAdjNorthEast;
				break; // SW -> NE
			case 3:
				reverseBit = kAdjNorthWest;
				break; // SE -> NW
			case 4:
				reverseBit = kAdjWest;
				break; // E -> W
			case 5:
				reverseBit = kAdjSouthWest;
				break; // NE -> SW
			default:
				break;
			}
			_cellAdjacencyMasks[neighborCell] &= ~reverseBit;
		}
		_cellGrid[base + 3 + i] = -1;
	}

	_cellAdjacencyMasks[cellIdx] = 0;
	syncCellFeatureScript(cellIdx);
}

void ZoombiniPuzzleSlides::clearCellLinkBits(uint16 bitMask, int16 linkField, int16 cellIdx) {
	if (cellIdx < 0 || kNumCells <= cellIdx || linkField < 0 || 6 <= linkField)
		return;

	_cellGrid[cellIdx * kFieldsPerCell + 3 + linkField] = -1;
	_cellAdjacencyMasks[cellIdx] &= ~bitMask;
	requestCellLinkOverlayRefresh();
}

void ZoombiniPuzzleSlides::updateNeighborFlags() {
	// Flat cells use one shared link pass so neighboring caps cannot clip the links.
	// Rising columns need the links back in their cell runners to preserve grid depth.
	handoffCellLinksToGridRunners();

	// Completion hides links between the accepted network and the remaining interactive network.
	// Clear only those boundary bits.
	for (int16 cellIdx = 0; cellIdx < kNumCells; cellIdx++) {
		int16 state = _cellGrid[cellIdx * kFieldsPerCell + 1];
		bool interactiveSide = state == kCellPath || state == kCellConnector || state == kCellOccupied;
		bool acceptedSide = state == kCellMatched || state == kCellLocked || state == _slotCellState;
		if (!interactiveSide && !acceptedSide)
			continue;

		for (int16 direction = 0; direction < 6; direction++) {
			int16 neighborCell = _cellGrid[cellIdx * kFieldsPerCell + 3 + direction];
			if (neighborCell < 0 || kNumCells <= neighborCell)
				continue;

			int16 neighborState = _cellGrid[neighborCell * kFieldsPerCell + 1];
			bool clearBoundary;
			if (interactiveSide)
				clearBoundary = neighborState == kCellMatched || neighborState == kCellLocked || neighborState == _slotCellState;
			else
				clearBoundary = neighborState == kCellPath || neighborState == kCellConnector || neighborState == kCellOccupied;
			if (clearBoundary)
				_cellAdjacencyMasks[cellIdx] &= ~(1 << direction);
		}
	}

	for (int16 cellIdx = 0; cellIdx < kNumCells; cellIdx++) {
		int16 state = _cellGrid[cellIdx * kFieldsPerCell + 1];
		if (state == kCellPath || state == kCellConnector || state == kCellOccupied)
			syncCellFeatureScript(cellIdx);
	}

	// Render the boundary-pruned SCRB 7000 state before the Go handler loads SCRB 7002.
	// This is also the Z-sort boundary that places bare waiting Snoids behind the cells which are about to rise.
	renderFeatures();
}

// =============================================================================
// Chain Building and Matching
// =============================================================================

int16 ZoombiniPuzzleSlides::validateChainAndMarkMatched(int16 startCellIdx) {
	if (startCellIdx < 0 || kNumCells <= startCellIdx)
		return 0;

	int16 matchCount = 0;
	int16 currentCell = startCellIdx;

	if (setCellStateAndReload(startCellIdx, kCellOccupied))
		matchCount += 1;

	// Revalidation is rooted at the authored slot base, not at the newly occupied cell.
	// Keep the cell immediately beside the base as the start of the outward validation pass.
	while (true) {
		const int16 upstreamCell = getBackwardChainLink(currentCell);
		if (upstreamCell < 0)
			return matchCount;
		if (_cellGrid[upstreamCell * kFieldsPerCell + 1] == _slotCellState)
			break;
		currentCell = upstreamCell;
	}

	bool blocked = false;
	while (!blocked && 0 <= currentCell) {
		int16 base = currentCell * kFieldsPerCell;
		int16 state = _cellGrid[base + 1];

		if (state == kCellInert || state == kCellConnector) {
			blocked = true;
		} else if (state != kCellPath) {
			if (state == kCellOccupied && setCellStateAndReload(currentCell, kCellLocked))
				matchCount += 1;
		} else {
			int16 traitKind = _cellGrid[base + 2];
			if (kTraitHair <= traitKind && traitKind <= kTraitFeet) {
				int16 forwardCell = getForwardChainLink(currentCell);
				int16 backwardMatchCell = getBackwardChainLink(currentCell);

				if (!cellStateIs(forwardCell, kCellOccupied, kCellLocked) ||
					!cellStateIs(backwardMatchCell, kCellOccupied, kCellLocked)) {
					blocked = true;
				} else if (cellsMatchTrait(backwardMatchCell, forwardCell, traitKind)) {
					if (setCellStateAndReload(currentCell, kCellMatched))
						matchCount += 1;
				} else {
					blocked = true;
				}
			} else if (_difficultyLevel == kPuzzleLevel2 && traitKind == 0 && 0 < currentCell &&
					   _cellGrid[(currentCell - 1) * kFieldsPerCell + 1] == kCellLocked) {
				if (setCellStateAndReload(currentCell, kCellMatched))
					matchCount += 1;
			}
		}

		if (!blocked) {
			currentCell = getForwardChainLink(currentCell);
			blocked = (currentCell < 0);
		}
	}

	return matchCount;
}

void ZoombiniPuzzleSlides::buildChainSequence() {
	memset(_pairLinkTypes, 0, sizeof(_pairLinkTypes));
	memset(_pairingUsageStates, 0, sizeof(_pairingUsageStates));
	_pairGroupCount = _pageLoadedZmbCount / 3;
	if ((_pageLoadedZmbCount % 3) != 0)
		_pairGroupCount += 1;

	int16 pairTypeIdx = 0;
	int16 runnerIdx = 0;
	for (int16 groupIdx = 0; groupIdx < _pairGroupCount; groupIdx++) {
		_pairingUsageStates[runnerIdx] = 1;
		int16 nextRunnerIdx = findRunnerByMatchingTrait(runnerIdx);
		if (nextRunnerIdx == -1) {
			_pairLinkTypes[pairTypeIdx] = kCellPath;
			pairTypeIdx += 1;
			for (int16 snoidIdx = 1; snoidIdx < _pageLoadedZmbCount; snoidIdx++) {
				if (_pairingUsageStates[snoidIdx] != 0)
					continue;
				_pairingUsageStates[snoidIdx] = 1;
				nextRunnerIdx = snoidIdx;
				break;
			}
		} else {
			_pairingUsageStates[nextRunnerIdx] = 1;
			_pairLinkTypes[pairTypeIdx] = _currentMatchedTraitIndex + kTraitHair;
			pairTypeIdx += 1;
		}

		runnerIdx = nextRunnerIdx;
		int16 remainingRunnerIdx = -1;
		for (int16 snoidIdx = 1; snoidIdx < _pageLoadedZmbCount; snoidIdx++) {
			if (_pairingUsageStates[snoidIdx] == 0)
				remainingRunnerIdx = snoidIdx;
		}
		if (remainingRunnerIdx == -1)
			break;

		int16 lastRunnerIdx = findRunnerByMatchingTrait(runnerIdx);
		if (lastRunnerIdx == -1) {
			_pairLinkTypes[pairTypeIdx] = kCellPath;
			pairTypeIdx += 1;
			for (int16 entryIdx = 1; entryIdx < _pageLoadedZmbCount; entryIdx++) {
				if (_pairingUsageStates[entryIdx] == 0)
					lastRunnerIdx = entryIdx;
			}
		} else {
			_pairingUsageStates[lastRunnerIdx] = 1;
			_pairLinkTypes[pairTypeIdx] = _currentMatchedTraitIndex + kTraitHair;
			pairTypeIdx += 1;
		}

		runnerIdx = lastRunnerIdx;
		_pairingUsageStates[lastRunnerIdx] = 1;
		remainingRunnerIdx = -1;
		for (int16 entryIdx = 1; entryIdx < _pageLoadedZmbCount; entryIdx++) {
			if (_pairingUsageStates[entryIdx] == 0)
				remainingRunnerIdx = entryIdx;
		}
		if (remainingRunnerIdx == -1)
			break;
		runnerIdx = remainingRunnerIdx;
	}
}

int16 ZoombiniPuzzleSlides::findRunnerInLockedState() const {
	for (int16 slotIdx = 0; slotIdx < _slotCount; slotIdx++) {
		int16 cellIdx = _slotCellIndices[slotIdx];
		if (0 <= cellIdx && _cellGrid[cellIdx * kFieldsPerCell + 1] == kCellLocked) {
			return _cellGrid[cellIdx * kFieldsPerCell + 2];
		}
	}
	return -1;
}

int16 ZoombiniPuzzleSlides::findRunnerByMatchingTrait(int16 runnerIdx) {
	_currentMatchedTraitIndex = _vm->_rnd->getRandomNumber(0, 3);
	for (int16 tries = 4; 0 < tries; tries--) {
		_currentMatchedTraitIndex += 1;
		if (3 < _currentMatchedTraitIndex)
			_currentMatchedTraitIndex = 0;

		for (int16 traitIdx = 0; traitIdx < _pageLoadedZmbCount; traitIdx++) {
			if (traitIdx == runnerIdx || _pairingUsageStates[traitIdx] != 0)
				continue;

			if (_snoidTraits[runnerIdx][_currentMatchedTraitIndex] == _snoidTraits[traitIdx][_currentMatchedTraitIndex]) {
				return traitIdx;
			}
		}
	}

	return -1;
}

void ZoombiniPuzzleSlides::sortZmbsByOverlapCount() {
	int16 overlapCounts[16] = {};

	for (int16 snoidIdx = 0; snoidIdx < _pageLoadedZmbCount; snoidIdx++) {
		for (int16 comparisonSnoidIdx = 0; comparisonSnoidIdx < _pageLoadedZmbCount; comparisonSnoidIdx++) {
			bool sharesTrait = false;
			for (int16 traitIdx = 0; traitIdx < 4; traitIdx++)
				sharesTrait |= _snoidTraits[snoidIdx][traitIdx] == _snoidTraits[comparisonSnoidIdx][traitIdx];
			if (sharesTrait) {
				overlapCounts[snoidIdx] += 1;
			}
		}
	}

	for (int16 sortedIdx = 0; sortedIdx < _pageLoadedZmbCount; sortedIdx++) {
		int16 bestRunnerIdx = 0;
		int16 bestOverlap = -1;

		for (int16 runnerIdx = 0; runnerIdx < _pageLoadedZmbCount; runnerIdx++) {
			if (bestOverlap < overlapCounts[runnerIdx]) {
				bestOverlap = overlapCounts[runnerIdx];
				bestRunnerIdx = runnerIdx;
			}
		}

		_sortedSnoidIndices[sortedIdx] = bestRunnerIdx;
		overlapCounts[bestRunnerIdx] = -1;
	}
}

int16 ZoombiniPuzzleSlides::placeMatchingZmbInCell(int16 matchCellIdx, int16 outSlot) {
	int16 traitCursor = _vm->_rnd->getRandomNumber(0, 3);
	int16 midCell = -1;
	int16 destCell = -1;
	if (5 < outSlot) {
		switch (outSlot) {
		case 6:
			midCell = getCellLink(matchCellIdx, 0);
			destCell = getCellLink(midCell, 1);
			break;
		case 7:
			midCell = getCellLink(matchCellIdx, 2);
			destCell = getCellLink(midCell, 1);
			break;
		case 8:
			midCell = getCellLink(matchCellIdx, 5);
			destCell = getCellLink(midCell, 4);
			break;
		case 9:
			midCell = getCellLink(matchCellIdx, 3);
			destCell = getCellLink(midCell, 4);
			break;
		default:
			break;
		}
	} else {
		midCell = getCellLink(matchCellIdx, outSlot);
		destCell = getCellLink(midCell, outSlot);
	}

	if (midCell < 0 || destCell < 0)
		return -1;

	ZmbSnoid *sourceSnoid = getSnoid(_cellGrid[matchCellIdx * kFieldsPerCell + 2]);
	if (!sourceSnoid)
		return -1;

	for (int16 sortedIdx = _pageLoadedZmbCount - 1; 0 <= sortedIdx; sortedIdx--) {
		int16 runnerListIdx = _sortedSnoidIndices[sortedIdx];
		if (runnerListIdx == -1)
			continue;

		ZmbSnoid *candidateSnoid = getSnoid(_snoidIds[runnerListIdx]);
		if (!candidateSnoid)
			continue;

		bool noMatch = true;
		int16 tries = 4;
		while (noMatch && 0 < tries) {
			if (candidateSnoid->_trait[traitCursor] == sourceSnoid->_trait[traitCursor]) {
				noMatch = false;
			} else {
				tries -= 1;
				traitCursor += 1;
				if (3 < traitCursor)
					traitCursor = 0;
			}
		}

		if (!noMatch) {
			_cellGrid[destCell * kFieldsPerCell + 1] = kCellOccupied;
			_cellGrid[destCell * kFieldsPerCell + 2] = candidateSnoid->getId();
			_cellGrid[midCell * kFieldsPerCell + 1] = kCellPath;
			_cellGrid[midCell * kFieldsPerCell + 2] = traitCursor + kTraitHair;
			_sortedSnoidIndices[sortedIdx] = -1;
			syncCellFeatureScript(destCell);
			syncCellFeatureScript(midCell);
			return sortedIdx;
		}
	}

	return -1;
}

int16 ZoombiniPuzzleSlides::pickRandomMatchingTrait(int16 cellIdx, int16 otherCellIdx) const {
	if (cellIdx < 0 || otherCellIdx < 0 || kNumCells <= cellIdx || kNumCells <= otherCellIdx)
		return 0;
	if (cellStateIs(cellIdx, kCellInert) || cellStateIs(otherCellIdx, kCellInert))
		return 0;

	int16 leftRunnerId = _cellGrid[cellIdx * kFieldsPerCell + 2];
	int16 rightRunnerId = _cellGrid[otherCellIdx * kFieldsPerCell + 2];
	if (leftRunnerId == 0 || rightRunnerId == 0)
		return 0;

	ZmbSnoid *leftSnoid = getSnoid(leftRunnerId);
	ZmbSnoid *rightSnoid = getSnoid(rightRunnerId);
	if (!leftSnoid || !rightSnoid)
		return 0;

	int16 roll = _vm->_rnd->getRandomNumber(0, 1000);
	if (roll < 250) {
		if (leftSnoid->_trait._hair == rightSnoid->_trait._hair)
			return kTraitHair;
		if (leftSnoid->_trait._eyes == rightSnoid->_trait._eyes)
			return kTraitEyes;
		if (leftSnoid->_trait._nose == rightSnoid->_trait._nose)
			return kTraitNose;
		if (leftSnoid->_trait._feet == rightSnoid->_trait._feet)
			return kTraitFeet;
	} else if (roll < 500) {
		if (leftSnoid->_trait._eyes == rightSnoid->_trait._eyes)
			return kTraitEyes;
		if (leftSnoid->_trait._nose == rightSnoid->_trait._nose)
			return kTraitNose;
		if (leftSnoid->_trait._feet == rightSnoid->_trait._feet)
			return kTraitFeet;
		if (leftSnoid->_trait._hair == rightSnoid->_trait._hair)
			return kTraitHair;
	} else if (roll < 750) {
		if (leftSnoid->_trait._nose == rightSnoid->_trait._nose)
			return kTraitNose;
		if (leftSnoid->_trait._feet == rightSnoid->_trait._feet)
			return kTraitFeet;
		if (leftSnoid->_trait._hair == rightSnoid->_trait._hair)
			return kTraitHair;
		if (leftSnoid->_trait._eyes == rightSnoid->_trait._eyes)
			return kTraitEyes;
	} else {
		if (leftSnoid->_trait._feet == rightSnoid->_trait._feet)
			return kTraitFeet;
		if (leftSnoid->_trait._hair == rightSnoid->_trait._hair)
			return kTraitHair;
		if (leftSnoid->_trait._eyes == rightSnoid->_trait._eyes)
			return kTraitEyes;
		if (leftSnoid->_trait._nose == rightSnoid->_trait._nose)
			return kTraitNose;
	}

	return 0;
}

void ZoombiniPuzzleSlides::activateChainLink(int16 linkIdx) {
	static constexpr int16 kChainCells[31] = {
		55, 57, 59, 61, 38, 74, 40, 76, 42, 78, 44, 80, 19, 91, 21, 93,
		23, 95, 25, 97, 2, 110, 4, 112, 6, 114, 57, 59, 61, 97, 25};
	int16 cellIdx = linkIdx + 1;

	if (!cellStateIs(cellIdx, kCellOccupied))
		return;

	setCellStateAndReload(cellIdx, kCellLocked);

	for (uint chainCellIdx = 0; chainCellIdx < ARRAYSIZE(kChainCells); chainCellIdx++) {
		int16 chainCell = kChainCells[chainCellIdx];
		if (cellStateIs(chainCell, kCellOccupied)) {
			for (int16 directionIdx = 0; directionIdx < 6; directionIdx++) {
				int16 neighborCell = _cellGrid[chainCell * kFieldsPerCell + 3 + directionIdx];
				if (cellStateIs(neighborCell, kCellMatched)) {
					setCellStateAndReload(chainCell, kCellLocked);
					break;
				}
			}
		}

		if (cellStateIs(chainCell, kCellMatched, kCellLocked))
			evalNeighborStates(chainCell);
	}

	checkVictoryCondition();

	if (_difficultyLevel == kPuzzleLevel4 &&
		cellStateIs(57, kCellLocked) && cellStateIs(59, kCellLocked) && cellStateIs(61, kCellLocked)) {
		int16 occupiedSlots = 0;
		for (int16 i = 0; i < _slotCount; i++) {
			int16 slotCell = _slotCellIndices[i];
			if (cellStateIs(slotCell, kCellOccupied, kCellLocked))
				occupiedSlots += 1;
		}

		if (!_victoryPaletteActive && occupiedSlots == 4) {
			_victoryPaletteActive = true;
			_lastVictoryPaletteFrame = getCurrentFrameCounter();
			_goButtonEnabled = true;
		} else if (occupiedSlots != 4) {
			_victoryPaletteActive = false;
		}
	}
}

void ZoombiniPuzzleSlides::confirmEndpointMatches() {
	static constexpr int16 kEndpointCells[3] = {19, 55, 91};

	for (int16 i = 0; i < 3; i++) {
		int16 endpointCell = kEndpointCells[i];
		if (!cellStateIs(endpointCell, kCellOccupied))
			continue;

		setCellStateAndReload(endpointCell, kCellLocked);
		propagateMatchChain(endpointCell);
	}

	checkVictoryCondition();
}

bool ZoombiniPuzzleSlides::checkFirstTraitMatch(int16 leftSortedIdx, int16 rightSortedIdx) {
	if (leftSortedIdx < 0 || rightSortedIdx < 0 || _pageLoadedZmbCount <= leftSortedIdx || _pageLoadedZmbCount <= rightSortedIdx)
		return false;
	int16 leftRunnerIdx = _sortedSnoidIndices[leftSortedIdx];
	int16 rightRunnerIdx = _sortedSnoidIndices[rightSortedIdx];
	if (leftRunnerIdx < 0 || rightRunnerIdx < 0)
		return false;

	for (int16 traitIdx = 0; traitIdx < 4; traitIdx++) {
		if (_snoidTraits[leftRunnerIdx][traitIdx] == _snoidTraits[rightRunnerIdx][traitIdx]) {
			_currentMatchedTraitIndex = traitIdx;
			return true;
		}
	}

	return false;
}

void ZoombiniPuzzleSlides::evalTraitMatchAndAdvance(int16 leadCellIdx, int16 middleCellIdx, int16 tailCellIdx) {
	if (middleCellIdx < 0 || !cellStateIs(middleCellIdx, kCellPath))
		return;

	int16 middleBase = middleCellIdx * kFieldsPerCell;
	int16 traitKind = _cellGrid[middleBase + 2];

	if (kTraitHair <= traitKind) {
		if (0 <= leadCellIdx && cellStateIs(tailCellIdx, kCellOccupied, kCellLocked) &&
			cellStateIs(leadCellIdx, kCellOccupied, kCellLocked)) {
			if (cellStateIs(middleCellIdx, kCellMatched) || cellsMatchTrait(tailCellIdx, leadCellIdx, traitKind)) {
				setCellStateAndReload(leadCellIdx, kCellLocked);
				setCellStateAndReload(middleCellIdx, kCellMatched);
			}
		} else if (0 <= leadCellIdx && cellStateIs(leadCellIdx, kCellMatched, kCellPath)) {
			setCellStateAndReload(leadCellIdx, kCellMatched);
		}
	} else if (cellStateIs(tailCellIdx, kCellOccupied, kCellLocked, kCellMatched)) {
		setCellStateAndReload(middleCellIdx, kCellMatched);
		if (0 <= leadCellIdx) {
			if (cellStateIs(leadCellIdx, kCellOccupied, kCellLocked)) {
				setCellStateAndReload(leadCellIdx, kCellLocked);
			} else if (cellStateIs(leadCellIdx, kCellPath)) {
				setCellStateAndReload(leadCellIdx, kCellMatched);
			}
		}
	}
}

void ZoombiniPuzzleSlides::evalNeighborStates(int16 cellIdx) {
	if (cellIdx < 0 || kNumCells <= cellIdx)
		return;

	int16 westCell = _cellGrid[cellIdx * kFieldsPerCell + 7];
	if (0 <= westCell)
		evalTraitMatchAndAdvance(_cellGrid[westCell * kFieldsPerCell + 7], westCell, cellIdx);

	int16 linkCell = _cellGrid[cellIdx * kFieldsPerCell + 4];
	if (0 <= linkCell)
		evalTraitMatchAndAdvance(_cellGrid[linkCell * kFieldsPerCell + 4], linkCell, cellIdx);

	int16 northEastCell = _cellGrid[cellIdx * kFieldsPerCell + 8];
	if (0 <= northEastCell) {
		int16 leadCell = _cellGrid[northEastCell * kFieldsPerCell + 8];
		evalTraitMatchAndAdvance(leadCell, northEastCell, cellIdx);
		if (cellStateIs(leadCell, kCellLocked, kCellMatched)) {
			int16 innerLead = _cellGrid[leadCell * kFieldsPerCell + 6];
			if (0 <= innerLead)
				evalTraitMatchAndAdvance(_cellGrid[innerLead * kFieldsPerCell + 6], innerLead, leadCell);

			int16 branchCell = _cellGrid[leadCell * kFieldsPerCell + 3];
			if (0 <= branchCell)
				evalTraitMatchAndAdvance(_cellGrid[branchCell * kFieldsPerCell + 3], branchCell, leadCell);
		}
	}

	int16 southWestCell = _cellGrid[cellIdx * kFieldsPerCell + 6];
	if (0 <= southWestCell) {
		int16 leadCell = _cellGrid[southWestCell * kFieldsPerCell + 6];
		evalTraitMatchAndAdvance(leadCell, southWestCell, cellIdx);
		if (cellStateIs(leadCell, kCellLocked, kCellMatched)) {
			int16 innerLead = _cellGrid[leadCell * kFieldsPerCell + 8];
			if (0 <= innerLead)
				evalTraitMatchAndAdvance(_cellGrid[innerLead * kFieldsPerCell + 8], innerLead, leadCell);

			int16 branchCell = _cellGrid[leadCell * kFieldsPerCell + 5];
			if (0 <= branchCell)
				evalTraitMatchAndAdvance(_cellGrid[branchCell * kFieldsPerCell + 5], branchCell, leadCell);
		}
	}

	int16 northWestCell = _cellGrid[cellIdx * kFieldsPerCell + 3];
	if (0 <= northWestCell)
		evalTraitMatchAndAdvance(_cellGrid[northWestCell * kFieldsPerCell + 3], northWestCell, cellIdx);

	int16 eastCell = _cellGrid[cellIdx * kFieldsPerCell + 5];
	if (0 <= eastCell)
		evalTraitMatchAndAdvance(_cellGrid[eastCell * kFieldsPerCell + 5], eastCell, cellIdx);
}

void ZoombiniPuzzleSlides::propagateMatchChain(int16 chainIdx) {
	if (chainIdx < 0 || kNumCells <= chainIdx)
		return;

	int16 linkCell = _cellGrid[chainIdx * kFieldsPerCell + 8];
	if (0 <= linkCell) {
		int16 leadCell = _cellGrid[linkCell * kFieldsPerCell + 7];
		evalTraitMatchAndAdvance(leadCell, linkCell, chainIdx);
		if (cellStateIs(leadCell, kCellLocked, kCellMatched)) {
			int16 outerCell = _cellGrid[leadCell * kFieldsPerCell + 7];
			int16 outerLead = (0 <= outerCell) ? _cellGrid[outerCell * kFieldsPerCell + 7] : -1;
			evalTraitMatchAndAdvance(outerLead, outerCell, leadCell);
			if (cellStateIs(outerLead, kCellLocked, kCellMatched)) {
				int16 cornerCell = _cellGrid[outerLead * kFieldsPerCell + 7];
				int16 cornerLead = (0 <= cornerCell) ? _cellGrid[cornerCell * kFieldsPerCell + 6] : -1;
				evalTraitMatchAndAdvance(cornerLead, cornerCell, outerLead);
				if (cellStateIs(cornerLead, kCellLocked, kCellMatched)) {
					int16 branchCell = _cellGrid[cornerLead * kFieldsPerCell + 5];
					int16 branchLead = (0 <= branchCell) ? _cellGrid[branchCell * kFieldsPerCell + 4] : -1;
					evalTraitMatchAndAdvance(branchLead, branchCell, cornerLead);
					if (cellStateIs(branchLead, kCellLocked, kCellMatched)) {
						int16 lastCell = _cellGrid[branchLead * kFieldsPerCell + 4];
						if (0 <= lastCell)
							evalTraitMatchAndAdvance(_cellGrid[lastCell * kFieldsPerCell + 4], lastCell, branchLead);
					}
				}
			}
		}
	}

	linkCell = _cellGrid[chainIdx * kFieldsPerCell + 6];
	if (0 <= linkCell) {
		int16 leadCell = _cellGrid[linkCell * kFieldsPerCell + 7];
		evalTraitMatchAndAdvance(leadCell, linkCell, chainIdx);
		if (cellStateIs(leadCell, kCellLocked, kCellMatched)) {
			int16 outerCell = _cellGrid[leadCell * kFieldsPerCell + 7];
			int16 outerLead = (0 <= outerCell) ? _cellGrid[outerCell * kFieldsPerCell + 7] : -1;
			evalTraitMatchAndAdvance(outerLead, outerCell, leadCell);
			if (cellStateIs(outerLead, kCellLocked, kCellMatched)) {
				int16 cornerCell = _cellGrid[outerLead * kFieldsPerCell + 7];
				int16 cornerLead = (0 <= cornerCell) ? _cellGrid[cornerCell * kFieldsPerCell + 8] : -1;
				evalTraitMatchAndAdvance(cornerLead, cornerCell, outerLead);
				if (cellStateIs(cornerLead, kCellLocked, kCellMatched)) {
					int16 branchCell = _cellGrid[cornerLead * kFieldsPerCell + 3];
					int16 branchLead = (0 <= branchCell) ? _cellGrid[branchCell * kFieldsPerCell + 4] : -1;
					evalTraitMatchAndAdvance(branchLead, branchCell, cornerLead);
					if (cellStateIs(branchLead, kCellLocked, kCellMatched)) {
						int16 lastCell = _cellGrid[branchLead * kFieldsPerCell + 4];
						if (0 <= lastCell)
							evalTraitMatchAndAdvance(_cellGrid[lastCell * kFieldsPerCell + 4], lastCell, branchLead);
					}
				}
			}
		}
	}
}

int16 ZoombiniPuzzleSlides::checkTraitMatchOutcome(int16 destCellIdx, int16 runnerListIdx, int16 occupiedCellIdx) {
	if (destCellIdx < 0 || occupiedCellIdx < 0 || kNumCells <= destCellIdx || kNumCells <= occupiedCellIdx ||
		runnerListIdx < 0 || _pageLoadedZmbCount <= runnerListIdx || !cellStateIs(occupiedCellIdx, kCellOccupied))
		return 0;

	int16 traitCursor = _vm->_rnd->getRandomNumber(0, 3);
	ZmbSnoid *occupiedSnoid = getSnoid(_cellGrid[occupiedCellIdx * kFieldsPerCell + 2]);
	if (!occupiedSnoid)
		return 0;

	for (int16 tries = 0; tries < 4; tries++) {
		const bool matches = _snoidTraits[runnerListIdx][traitCursor] == occupiedSnoid->_trait[traitCursor];

		if (matches) {
			const int16 traitKind = kTraitHair + traitCursor;
			_cellGrid[destCellIdx * kFieldsPerCell + 2] = traitKind;
			return traitKind;
		}

		traitCursor += 1;
		if (3 < traitCursor)
			traitCursor = 0;
	}

	return 0;
}

// =============================================================================
// Animation and Travel
// =============================================================================

void ZoombiniPuzzleSlides::resetAnimStates() {
	for (int16 cellIdx = 0; cellIdx < kNumCells; cellIdx++) {
		int16 &state = _cellGrid[cellIdx * kFieldsPerCell + 1];
		if (state == kCellLocked) {
			state = kCellOccupied;
			syncCellFeatureScript(cellIdx);
		} else if (state == kCellMatched) {
			state = kCellPath;
			syncCellFeatureScript(cellIdx);
		}
	}

	if (_difficultyLevel == kPuzzleLevel4)
		activateChainLink(54);
	else
		confirmEndpointMatches();
}

bool ZoombiniPuzzleSlides::beginZmbTravel(ZmbSnoid *snoid) {
	// Test the dragged Snoid root against each cell's x..x+45, y-22..y rectangle.
	if (!snoid || _invalidDropTravelActive)
		return false;

	const Common::Point snoidPos = snoid->getPointLoc();
	int16 cellIdx = -1;
	for (int16 candidate = 0; candidate < kNumCells; candidate++) {
		const Common::Point &cellPos = kCellPositions[candidate];
		if (cellPos.x <= snoidPos.x && snoidPos.x <= cellPos.x + 45 &&
			cellPos.y - 22 <= snoidPos.y && snoidPos.y <= cellPos.y) {
			cellIdx = candidate;
			break;
		}
	}

	if (cellIdx < 0)
		return false;

	_invalidDropSnoidId = snoid->getId();
	_invalidDropTravelActive = true;

	if (_invalidDropEffectFeature)
		unloadScrbFeature(_invalidDropEffectFeature);

	int16 scrbId = kResScrb8000_TravelBase;
	if (cellIdx < 36)
		scrbId = kResScrb8002_TravelVariant2;
	else if (cellIdx < 89)
		scrbId = kResScrb8001_TravelVariant1;

	// The controller's first shape is positioned at the Snoid's feet. Its
	// event 91/92/93 starts the matching SCRS ejection stage.
	const Common::Point effectPos(kCellPositions[cellIdx].x + 19, kCellPositions[cellIdx].y + 24);
	_invalidDropEffectFeature = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape8000_Travel), scrbId, 6, effectPos,
		ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_00800000_POS_DELTA);
	if (_invalidDropEffectFeature) {
		if (cellIdx % 18)
			manualLinkBefore(_invalidDropEffectFeature, _rowLayerFeatures[7]);
		else
			manualLinkBefore(_invalidDropEffectFeature, _rowLayerFeatures[cellIdx / 18 + 1]);
	}
	return _invalidDropEffectFeature != nullptr;
}

void ZoombiniPuzzleSlides::handleZmbTravelEvent(int16 eventCode) {
	ZmbSnoid *snoid = getSnoid(_invalidDropSnoidId);
	if (!snoid || eventCode < kSlidesEventCode090_TravelVariant0 ||
		kSlidesEventCode093_TravelVariant3 < eventCode)
		return;

	Common::Point initPos(snoid->getPointLoc().x, -50);
	if (eventCode == kSlidesEventCode093_TravelVariant3) {
		initPos.x = _vm->_rnd->getRandomNumber(0, 42) + 70;
		initPos.y = _vm->_rnd->getRandomNumber(0, 200) + 152;
	}

	const int16 scrsId = static_cast<int16>(kResScrs14000_TravelBase + eventCode - kSlidesEventCode090_TravelVariant0);
	if (!startSnoidScrs(snoid, ZmbResource(ZmbResource::kPage, scrsId), ZmbScrsCompletionMode::kReturnToIdle, &initPos))
		return;

	_invalidDropTravelActive = eventCode != kSlidesEventCode093_TravelVariant3;
}

Common::Point ZoombiniPuzzleSlides::getCellSnoidPosition(int16 cellIdx) const {
	if (cellIdx < 0 || kNumCells <= cellIdx)
		return Common::Point();
	return Common::Point(kCellPositions[cellIdx].x + 24, kCellPositions[cellIdx].y - 5);
}

int16 ZoombiniPuzzleSlides::countAcceptedCellsAndUpdateChecksum() {
	_acceptedCellIndexChecksum = 0;
	int16 acceptedCount = 0;
	for (int16 cellIdx = 0; cellIdx < kNumCells; cellIdx++) {
		int16 state = _cellGrid[cellIdx * kFieldsPerCell + 1];
		if (state != kCellMatched && state != kCellLocked)
			continue;
		acceptedCount += 1;
		_acceptedCellIndexChecksum += cellIdx;
	}
	return acceptedCount;
}

void ZoombiniPuzzleSlides::playPlacementFeedbackSFX() {
	// Compare accepted-cell count changes and use the index-sum checksum for same-count placement changes.
	// This lightweight checksum is not a complete accepted-cell set comparison and can have collisions.
	int16 acceptedCount = countAcceptedCellsAndUpdateChecksum();
	int16 sfxId = 0;
	if (_acceptedCellCountBeforeDrag < acceptedCount) {
		_goButtonEnabled = true;
		setGoButtonsEnabled(true);
		if (acceptedCount - _acceptedCellCountBeforeDrag <= 4)
			sfxId = kResSound8504_AcceptedCountIncreaseSmall;
		else
			sfxId = kResSound8505_AcceptedCountIncreaseLarge;
	} else if (acceptedCount == _acceptedCellCountBeforeDrag) {
		if (_acceptedCellIndexChecksum != _acceptedCellIndexChecksumBeforeDrag)
			sfxId = kResSound8502_AcceptedCellsRearranged;
	} else {
		if (_acceptedCellCountBeforeDrag - acceptedCount <= 4)
			sfxId = kResSound8500_AcceptedCountDecreaseSmall;
		else
			sfxId = kResSound8501_AcceptedCountDecreaseLarge;
	}
	if (sfxId != 0) {
		ZmbResource soundResource(ZmbResource::kPage, sfxId);
		// Repeated accept/reject feedback stops the identical active SFX before it is started again.
		// SND 8502 is the sole same-count rearrangement case without that restart.
		// Slides limits the priority-queue arbitration to its entrance/rise passes,
		// so keep ordinary placement feedback on the immediate SFX path.
		if (sfxId != kResSound8502_AcceptedCellsRearranged && !_vm->_sound->isSfxMuted())
			_vm->_sound->stopSound(soundResource);
		_vm->_sound->playSound(soundResource);
	}
}

void ZoombiniPuzzleSlides::placeBuiltinDebugSolution() {

	for (int16 solutionIdx = 0; solutionIdx < _debugSolutionCount; solutionIdx++) {
		const int16 cellIdx = _debugSolutionCellIndices[solutionIdx];
		const int16 runnerId = _debugSolutionRunnerIds[solutionIdx];

		if (cellIdx < 0 || kNumCells <= cellIdx || runnerId <= 0)
			continue;

		ZmbSnoid *snoid = getSnoid(runnerId);
		if (!snoid)
			continue;

		addExternalDirtyRect(snoid->getZSortRect());
		moveZmbToCell(snoid, cellIdx);
		snoid->setupIdleHotspots();
		snoid->setNeedsRedraw(true);
		_cellGrid[cellIdx * kFieldsPerCell + 1] = kCellOccupied;
		_cellGrid[cellIdx * kFieldsPerCell + 2] = runnerId;
		const int16 slotIdx = findSlotIndexForCell(cellIdx);
		if (0 <= slotIdx && 0 <= _slotDrawOnRegIndices[slotIdx])
			setDrawOnRegOccupant(_slotDrawOnRegIndices[slotIdx], runnerId);
		syncCellFeatureScript(cellIdx);
	}
}

void ZoombiniPuzzleSlides::loadRunnerSCRB(uint16 runnerId, int16 scrbId) {
	for (int16 cellIdx = 0; cellIdx < kNumCells; cellIdx++) {
		if (_cellGrid[cellIdx * kFieldsPerCell + 0] != static_cast<int16>(runnerId) || !_cellFeatures[cellIdx])
			continue;

		setCellFeaturePreRenderHook(_cellFeatures[cellIdx], scrbId);
		loadScrbOntoFeature(_cellFeatures[cellIdx], scrbId);
		requestCellLinkOverlayRefresh();
		return;
	}

	ZmbSnoid *snoid = getSnoid(runnerId);
	if (snoid) {
		loadScrbOntoFeature(snoid, scrbId);
	}
}

// =============================================================================
// Slot Management
// =============================================================================

void ZoombiniPuzzleSlides::unlockInteractiveSlots() {
	// The solved-rise relink hands depth control to the explicit links that
	// @ref ZoombiniPuzzleSlides::moveZmbToCell() and the travel-effect placement create,
	// so positional sorting stops here for the rest of the page.
	setRunnerZSortEnabled(false);

	bool referenced[16] = {};
	for (int16 slotIdx = 0; slotIdx < _slotCount; slotIdx++) {
		int16 cellIdx = _slotCellIndices[slotIdx];
		if (cellIdx < 0 || kNumCells <= cellIdx)
			continue;
		int16 state = _cellGrid[cellIdx * kFieldsPerCell + 1];
		if (state != kCellOccupied && state != kCellLocked)
			continue;

		int16 runnerId = _cellGrid[cellIdx * kFieldsPerCell + 2];
		ZmbSnoid *snoid = getSnoid(runnerId);
		if (state == kCellOccupied && snoid) {
			snoid->addFlag(ZmbFeature::FLAG_00008000_LOOP_ANIM);
			snoid->addFlag(ZmbFeature::FLAG_04000000_OVERLAY);
			if (cellIdx + 1 < kNumCells)
				manualLinkBefore(snoid, _cellFeatures[cellIdx + 1]);
		}

		for (int16 zmbIdx = 0; zmbIdx < _pageLoadedZmbCount; zmbIdx++) {
			if (_snoidIds[zmbIdx] == runnerId)
				referenced[zmbIdx] = true;
		}
	}

	for (int16 zmbIdx = 0; zmbIdx < _pageLoadedZmbCount; zmbIdx++) {
		ZmbSnoid *snoid = getSnoid(_snoidIds[zmbIdx]);
		if (!snoid)
			continue;
		if (!referenced[zmbIdx]) {
			snoid->addFlag(ZmbFeature::FLAG_00008000_LOOP_ANIM);
			snoid->addFlag(ZmbFeature::FLAG_04000000_OVERLAY);
			manualLinkBefore(snoid, _cellFeatures[0]);
			snoid->setupIdleHotspots();
		}
	}
}

int16 ZoombiniPuzzleSlides::placeNextZmbInCell(int16 cellIdx) {
	int16 result = cellIdx * kFieldsPerCell;

	for (int16 runnerIdx = 0; runnerIdx < _pageLoadedZmbCount; runnerIdx++) {
		if (_sortedSnoidIndices[runnerIdx] == -1)
			continue;

		int16 runnerId = _snoidIds[_sortedSnoidIndices[runnerIdx]];
		_cellGrid[cellIdx * kFieldsPerCell + 1] = kCellOccupied;
		_cellGrid[cellIdx * kFieldsPerCell + 2] = runnerId;
		_sortedSnoidIndices[runnerIdx] = -1;
		syncCellFeatureScript(cellIdx);
		break;
	}

	for (int16 snoidIdx = 0; snoidIdx < _pageLoadedZmbCount; snoidIdx++) {
		if (_sortedSnoidIndices[snoidIdx] == -1)
			continue;

		int16 pairedCell = cellIdx + 5;
		if (pairedCell < kNumCells) {
			int16 runnerId = _snoidIds[_sortedSnoidIndices[snoidIdx]];
			_cellGrid[pairedCell * kFieldsPerCell + 1] = kCellOccupied;
			_cellGrid[pairedCell * kFieldsPerCell + 2] = runnerId;
			_sortedSnoidIndices[snoidIdx] = -1;
			syncCellFeatureScript(pairedCell);
		}
		break;
	}

	if (cellStateIs(cellIdx, kCellOccupied)) {
		int16 matched = placeMatchingZmbInCell(cellIdx, 8);
		if (matched != -1)
			placeMatchingZmbInCell(cellIdx, 9);
	}

	if (cellIdx + 5 < kNumCells && cellStateIs(cellIdx + 5, kCellOccupied)) {
		result = placeMatchingZmbInCell(cellIdx + 5, 6);
		if (result != -1)
			result = placeMatchingZmbInCell(cellIdx + 5, 7);
	}

	return result;
}

bool ZoombiniPuzzleSlides::hasPendingZmb() const {
	for (int16 i = 0; i < _pageLoadedZmbCount; i++) {
		if (_sortedSnoidIndices[i] != -1)
			return true;
	}
	return false;
}

void ZoombiniPuzzleSlides::scanAndResetActiveCells() {
	_activeCellCount = 0;
	for (int16 cellIdx = 0; cellIdx < kNumCells; cellIdx++) {
		int16 &state = _cellGrid[cellIdx * kFieldsPerCell + 1];
		if (state == kCellConnector) {
			state = kCellPath;
			syncCellFeatureScript(cellIdx);
		}

		if (state == kCellOccupied && _activeCellCount < ARRAYSIZE(_activeCellIndices)) {
			_activeCellIndices[_activeCellCount] = cellIdx;
			_activeCellCount += 1;
			_activeCellSnoidIds[_activeCellCount - 1] = _cellGrid[cellIdx * kFieldsPerCell + 2];
		}
	}
	if (getCellState(2) == kCellPath && getCellState(19) == kCellPath) {
		clearCellToEmpty(2);
		clearCellToEmpty(19);
		clearCellToEmpty(10);
		clearCellToEmpty(11);
		clearCellToEmpty(28);
		clearCellLinkBits(kAdjNorthWest, 0, 38);
		clearCellLinkBits(kAdjNorthWest, 0, 21);
	}

	if (getCellState(91) == kCellPath && getCellState(110) == kCellPath) {
		clearCellToEmpty(91);
		clearCellToEmpty(110);
		clearCellToEmpty(100);
		clearCellToEmpty(82);
		clearCellToEmpty(101);
		clearCellLinkBits(kAdjSouthWest, 2, 74);
		clearCellLinkBits(kAdjSouthWest, 2, 93);
	}

	if (getCellState(112) == kCellPath) {
		clearCellToEmpty(112);
		clearCellToEmpty(102);
		clearCellToEmpty(103);
		clearCellLinkBits(kAdjSouthEast, 3, 93);
		clearCellLinkBits(kAdjSouthWest, 2, 95);
	}

	if (getCellState(114) == kCellPath) {
		clearCellToEmpty(114);
		clearCellToEmpty(104);
		clearCellToEmpty(105);
		clearCellLinkBits(kAdjSouthEast, 3, 95);
		clearCellLinkBits(kAdjSouthWest, 2, 97);
	}

	if (getCellState(4) == kCellPath) {
		clearCellToEmpty(4);
		clearCellToEmpty(12);
		clearCellToEmpty(13);
		clearCellLinkBits(kAdjNorthEast, 5, 21);
		clearCellLinkBits(kAdjNorthWest, 0, 23);
	}

	if (getCellState(6) == kCellPath) {
		clearCellToEmpty(6);
		clearCellToEmpty(14);
		clearCellToEmpty(15);
		clearCellLinkBits(kAdjNorthEast, 5, 23);
		clearCellLinkBits(kAdjNorthWest, 0, 25);
	}

	if (getCellState(97) == kCellPath && getCellState(80) == kCellPath) {
		clearCellToEmpty(97);
		clearCellToEmpty(80);
		clearCellToEmpty(88);
		clearCellToEmpty(87);
		clearCellToEmpty(70);
		clearCellToEmpty(105);
		clearCellLinkBits(kAdjSouthEast, 3, 78);
		clearCellLinkBits(kAdjSouthEast, 3, 61);
		clearCellLinkBits(kAdjNorthEast, 5, 114);
	}

	if (getCellState(25) == kCellPath && getCellState(44) == kCellPath) {
		clearCellToEmpty(25);
		clearCellToEmpty(44);
		clearCellToEmpty(34);
		clearCellToEmpty(15);
		clearCellToEmpty(33);
		clearCellToEmpty(52);
		clearCellLinkBits(kAdjNorthEast, 5, 42);
		clearCellLinkBits(kAdjNorthEast, 5, 61);
		clearCellLinkBits(kAdjSouthEast, 3, 6);
	}
}

int16 ZoombiniPuzzleSlides::findMatchingZmbForCell(int16 matchCellIdx, int16 outResult) {
	int16 traitCursor = _vm->_rnd->getRandomNumber(0, 3);
	int16 midCell = getCellLink(matchCellIdx, outResult);
	if (midCell < 0)
		return -2;

	int16 destCell = getCellLink(midCell, outResult);
	if (destCell < 0)
		return -2;

	ZmbSnoid *sourceSnoid = getSnoid(_cellGrid[matchCellIdx * kFieldsPerCell + 2]);
	if (!sourceSnoid)
		return -1;

	for (int16 snoidIdx = 0; snoidIdx < _pageLoadedZmbCount; snoidIdx++) {
		int16 runnerListIdx = _sortedSnoidIndices[snoidIdx];
		if (runnerListIdx == -1)
			continue;

		ZmbSnoid *candidateSnoid = getSnoid(_snoidIds[runnerListIdx]);
		if (!candidateSnoid)
			continue;

		bool noMatch = true;
		int16 tries = 4;
		while (noMatch && 0 < tries) {
			if (candidateSnoid->_trait[traitCursor] == sourceSnoid->_trait[traitCursor]) {
				noMatch = false;
			} else {
				tries -= 1;
				traitCursor += 1;
				if (3 < traitCursor)
					traitCursor = 0;
			}
		}

		if (!noMatch) {
			_cellGrid[destCell * kFieldsPerCell + 1] = kCellOccupied;
			_cellGrid[destCell * kFieldsPerCell + 2] = candidateSnoid->getId();
			_cellGrid[midCell * kFieldsPerCell + 1] = kCellPath;
			_cellGrid[midCell * kFieldsPerCell + 2] = traitCursor + kTraitHair;
			_sortedSnoidIndices[snoidIdx] = -1;
			syncCellFeatureScript(destCell);
			syncCellFeatureScript(midCell);
			return snoidIdx;
		}
	}

	return -1;
}

void ZoombiniPuzzleSlides::reassignDeadSlots() {
	static constexpr int16 kReassignCells[22] = {
		55, 57, 59, 61, 38, 74, 40, 76, 42, 78, 44, 80, 21, 93, 23, 95, 25, 97, 4, 112, 19, 25};

	for (uint arrayIdx = 0; arrayIdx < ARRAYSIZE(kReassignCells); arrayIdx++) {
		int16 cellIdx = kReassignCells[arrayIdx];
		if (cellStateIs(cellIdx, kCellOccupied))
			continue;

		for (int16 sortedIdx = 0; sortedIdx < _pageLoadedZmbCount; sortedIdx++) {
			int16 runnerListIdx = _sortedSnoidIndices[sortedIdx];
			if (runnerListIdx == -1)
				continue;

			_cellGrid[cellIdx * kFieldsPerCell + 2] = _snoidIds[runnerListIdx];
			_sortedSnoidIndices[sortedIdx] = -1;
			setCellStateAndReload(cellIdx, kCellOccupied);
			break;
		}

		if (!hasPendingZmb())
			break;
	}
}

void ZoombiniPuzzleSlides::pickNextCellForLink(int16 cellIdx, int16 nextCell, int16 direction) {
	if (cellIdx < 0 || nextCell < 0 || direction < 0 || kNumCells <= cellIdx || kNumCells <= nextCell || kNumCells <= direction)
		return;

	int16 directionData = _cellGrid[direction * kFieldsPerCell + 2];
	int16 nextData = _cellGrid[nextCell * kFieldsPerCell + 2];
	int16 cellState = _cellGrid[cellIdx * kFieldsPerCell + 1];

	if (directionData < kTraitHair && nextData < kTraitHair && cellState == kCellPath) {
		resetCellToEmpty(direction);
		resetCellToEmpty(nextCell);
		resetCellToEmpty(cellIdx);
		return;
	}

	if (directionData < kTraitHair && nextData < kTraitHair && cellState == kCellOccupied) {
		resetCellToEmpty(nextCell);
		return;
	}

	if (directionData < kTraitHair && kTraitHair <= nextData && cellState == kCellOccupied) {
		resetCellToEmpty(direction);
		return;
	}

	if (kTraitHair <= directionData && nextData < kTraitHair && cellState == kCellOccupied)
		resetCellToEmpty(nextCell);
}

void ZoombiniPuzzleSlides::markMatchedRunnersDone() {
	// Mark the Snoid in each locked Slides cell as occupied.
	for (int16 cellIdx = 0; cellIdx < kNumCells; cellIdx++) {
		if (_cellGrid[cellIdx * kFieldsPerCell + 1] != kCellLocked)
			continue;

		ZmbSnoid *snoid = getSnoid(_cellGrid[cellIdx * kFieldsPerCell + 2]);
		if (!snoid)
			continue;

		// Locked Slides runners are the accepted continuing group.
		snoid->_packIsOccupied = true;
	}
}

// =============================================================================
// Victory Checking
// =============================================================================

void ZoombiniPuzzleSlides::checkVictoryCondition() {
	int16 matchedCellCount = 0;
	for (int16 cellIdx = 0; cellIdx < kNumCells; cellIdx++) {
		if (_cellGrid[cellIdx * kFieldsPerCell + 1] == kCellLocked)
			matchedCellCount += 1;
	}

	if (!_celebrationVisitLatched && _pageLoadedZmbCount <= matchedCellCount) {
		_fullMatchTriggerCount += 1;
		queueCompletionNarratorSound();
	}
}

void ZoombiniPuzzleSlides::refreshGoButtonEnabledState() {
	bool goButtonEnabled = false;
	for (int16 slotIdx = 0; slotIdx < _slotCount; slotIdx++) {
		const int16 cellIdx = _slotCellIndices[slotIdx];
		if (0 <= cellIdx && _cellGrid[cellIdx * kFieldsPerCell + 1] == kCellLocked) {
			goButtonEnabled = true;
			break;
		}
	}

	_goButtonEnabled = goButtonEnabled;
	setGoButtonsEnabled(_goButtonEnabled);
}

// =============================================================================
// Callback Functions
// =============================================================================

void ZoombiniPuzzleSlides::ensureCellFeature(int16 cellIdx) {
	if (cellIdx < 0 || kNumCells <= cellIdx || _cellFeatures[cellIdx])
		return;

	int16 scrbId;
	if (_cellGrid[cellIdx * kFieldsPerCell + 1] == kCellInert)
		scrbId = kResScrb7001_InertCell;
	else
		scrbId = kResScrb7000_CellBase;
	ZmbFeature::EventHooks slotHooks;
	if (scrbId == kResScrb7000_CellBase) {
		slotHooks.setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniPuzzleSlides::filterHotspotScript));
	}

	ZmbFeature *slotFeature = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape7000_CellAnimations), scrbId, 6,
		kCellPositions[cellIdx],
		ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_00800000_POS_DELTA,
		slotHooks);
	slotFeature->activateRender();
	slotFeature->deactivateAnimate();
	_cellFeatures[cellIdx] = slotFeature;
	_cellGrid[cellIdx * kFieldsPerCell + 0] = static_cast<int16>(slotFeature->getRegistrationIndex());
}

void ZoombiniPuzzleSlides::snapshotDebugSolution() {
	_debugSolutionCount = 0;
	for (int16 slotIdx = 0; slotIdx < _slotCount && _debugSolutionCount < 16; slotIdx++) {
		int16 cellIdx = _slotCellIndices[slotIdx];
		if (cellIdx < 0 || kNumCells <= cellIdx)
			continue;

		int16 runnerId = _cellGrid[cellIdx * kFieldsPerCell + 2];
		if (!getSnoid(runnerId))
			continue;

		_debugSolutionCellIndices[_debugSolutionCount] = cellIdx;
		_debugSolutionRunnerIds[_debugSolutionCount] = runnerId;
		_debugSolutionCount += 1;
	}

	if (_debugSolutionCount != _pageLoadedZmbCount) {
		if (_difficultyLevel <= kPuzzleLevel2)
			buildDebugSolutionFromConstraints();
		else if (_difficultyLevel == kPuzzleLevel3)
			completeDebugSolutionWithUnusedSnoids();
	}
}

bool ZoombiniPuzzleSlides::buildDebugSolutionFromConstraints() {
	if (_slotCount != _pageLoadedZmbCount || _pageLoadedZmbCount < 1 || 16 < _pageLoadedZmbCount)
		return false;

	int16 requiredTraits[16];
	int16 assignedSnoids[16];
	bool usedSnoids[16];
	memset(requiredTraits, 0, sizeof(requiredTraits));
	memset(assignedSnoids, -1, sizeof(assignedSnoids));
	memset(usedSnoids, 0, sizeof(usedSnoids));

	int16 slotIdx = 0;
	if (_difficultyLevel == kPuzzleLevel1) {
		for (int16 pairIdx = 0; pairIdx < _pairGroupCount && slotIdx < _slotCount; pairIdx++) {
			requiredTraits[slotIdx] = 0;
			slotIdx += 1;
			if (_pairLinkTypes[pairIdx] != kCellPath && slotIdx < _slotCount) {
				requiredTraits[slotIdx] = _pairLinkTypes[pairIdx];
				slotIdx += 1;
			}
		}
	} else {
		int16 pairTypeIdx = 0;
		for (int16 groupIdx = 0; groupIdx < _pairGroupCount && slotIdx < _slotCount; groupIdx++) {
			requiredTraits[slotIdx] = 0;
			slotIdx += 1;
			for (int16 groupSlot = 1; groupSlot < 3 && slotIdx < _slotCount; groupSlot++) {
				requiredTraits[slotIdx] = _pairLinkTypes[pairTypeIdx];
				pairTypeIdx += 1;
				slotIdx += 1;
			}
		}
	}

	if (slotIdx != _slotCount || !solveDebugSlotAssignments(0, requiredTraits, usedSnoids, assignedSnoids))
		return false;

	_debugSolutionCount = _slotCount;
	for (int16 solutionIdx = 0; solutionIdx < _debugSolutionCount; solutionIdx++) {
		_debugSolutionCellIndices[solutionIdx] = _slotCellIndices[solutionIdx];
		_debugSolutionRunnerIds[solutionIdx] = _snoidIds[assignedSnoids[solutionIdx]];
	}
	return true;
}

bool ZoombiniPuzzleSlides::completeDebugSolutionWithUnusedSnoids() {
	if (_slotCount != _pageLoadedZmbCount || _pageLoadedZmbCount < 1 || 16 < _pageLoadedZmbCount)
		return false;

	bool usedSnoids[16];
	memset(usedSnoids, 0, sizeof(usedSnoids));
	for (int16 solutionIdx = 0; solutionIdx < _debugSolutionCount; solutionIdx++) {
		for (int16 snoidIdx = 0; snoidIdx < _pageLoadedZmbCount; snoidIdx++) {
			if (_debugSolutionRunnerIds[solutionIdx] == _snoidIds[snoidIdx])
				usedSnoids[snoidIdx] = true;
		}
	}

	for (int16 slotIdx = 0; slotIdx < _slotCount; slotIdx++) {
		int16 cellIdx = _slotCellIndices[slotIdx];
		bool cellRecorded = false;
		for (int16 solutionIdx = 0; solutionIdx < _debugSolutionCount; solutionIdx++) {
			if (_debugSolutionCellIndices[solutionIdx] == cellIdx)
				cellRecorded = true;
		}
		if (cellRecorded)
			continue;

		int16 unusedSnoidIdx = -1;
		for (int16 snoidIdx = 0; snoidIdx < _pageLoadedZmbCount; snoidIdx++) {
			if (!usedSnoids[snoidIdx]) {
				unusedSnoidIdx = snoidIdx;
				break;
			}
		}
		if (unusedSnoidIdx < 0 || 16 <= _debugSolutionCount)
			return false;

		_debugSolutionCellIndices[_debugSolutionCount] = cellIdx;
		_debugSolutionRunnerIds[_debugSolutionCount] = _snoidIds[unusedSnoidIdx];
		_debugSolutionCount += 1;
		usedSnoids[unusedSnoidIdx] = true;
	}

	return _debugSolutionCount == _pageLoadedZmbCount;
}

bool ZoombiniPuzzleSlides::solveDebugSlotAssignments(int16 slotIdx, const int16 *requiredTraits,
													 bool *usedSnoids, int16 *assignedSnoids) {
	if (_slotCount <= slotIdx)
		return true;

	for (int16 snoidIdx = 0; snoidIdx < _pageLoadedZmbCount; snoidIdx++) {
		if (usedSnoids[snoidIdx])
			continue;
		if (0 < slotIdx && kTraitHair <= requiredTraits[slotIdx] &&
			!debugSnoidsMatchTrait(assignedSnoids[slotIdx - 1], snoidIdx, requiredTraits[slotIdx])) {
			continue;
		}

		usedSnoids[snoidIdx] = true;
		assignedSnoids[slotIdx] = snoidIdx;
		if (solveDebugSlotAssignments(slotIdx + 1, requiredTraits, usedSnoids, assignedSnoids))
			return true;
		assignedSnoids[slotIdx] = -1;
		usedSnoids[snoidIdx] = false;
	}
	return false;
}

bool ZoombiniPuzzleSlides::debugSnoidsMatchTrait(int16 leftSnoidIdx, int16 rightSnoidIdx, int16 traitKind) const {
	if (leftSnoidIdx < 0 || rightSnoidIdx < 0 || _pageLoadedZmbCount <= leftSnoidIdx || _pageLoadedZmbCount <= rightSnoidIdx)
		return false;

	if (traitKind < kTraitHair || kTraitFeet < traitKind)
		return true;
	const int16 traitIdx = traitKind - kTraitHair;
	return _snoidTraits[leftSnoidIdx][traitIdx] == _snoidTraits[rightSnoidIdx][traitIdx];
}

void ZoombiniPuzzleSlides::loadPlacementSlotFeatures() {
	ZmbFeature::EventHooks slotHooks;
	slotHooks.setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniPuzzleSlides::filterCommandByFlags));

	for (int16 slotIdx = 0; slotIdx < _slotCount; slotIdx++) {
		int16 cellIdx = _slotCellIndices[slotIdx];
		if (cellIdx < 0 || kNumCells <= cellIdx)
			continue;

		int16 drawOnRegIdx = _drawOnRegCount;
		_slotFeatures[slotIdx] = loadScrbFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape7000_CellAnimations), kResScrb7013_PlacementSlot, 7,
			getCellSnoidPosition(cellIdx),
			ZmbFeature::FLAG_00002000_DRAW_ON_REG | ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM |
				ZmbFeature::FLAG_00800000_POS_DELTA | ZmbFeature::FLAG_01000000_DEFER_RENDER,
			slotHooks);
		if (drawOnRegIdx < _drawOnRegCount)
			_slotDrawOnRegIndices[slotIdx] = drawOnRegIdx;
	}
}

void ZoombiniPuzzleSlides::relinkGridLayers() {
	static constexpr int16 kRowStartCells[6] = {9, 27, 45, 63, 81, 99};
	for (int16 layerIdx = 1; layerIdx < 7; layerIdx++)
		manualLinkBefore(_rowLayerFeatures[layerIdx], _cellFeatures[kRowStartCells[layerIdx - 1]]);
}

void ZoombiniPuzzleSlides::relinkSnoidsBehindForeground() {
	ZmbSnoid *firstSnoid = 0 < _pageLoadedZmbCount ? getSnoid(_snoidIds[0]) : nullptr;
	if (!firstSnoid || !_rowLayerFeatures[8]) {
		_preRelinkSnoidFlags = 0;
		_preRelinkSnoidFlagsPending = false;
		return;
	}

	// Keep the foreground tree as the boundary between depth-sorted Snoids and
	// the shared topmost controls without disabling page-wide Z-sorting.
	_rowLayerFeatures[8]->addFlag(ZmbFeature::FLAG_00001000_TOPMOST);

	// The reverse walk inserts every Snoid immediately before layer 8, SCRB 7011.
	// It temporarily adds LOOP_ANIM and OVERLAY for the initial frame.
	_preRelinkSnoidFlags = firstSnoid->getFlags();
	_preRelinkSnoidFlagsPending = true;
	for (int16 snoidIdx = _pageLoadedZmbCount - 1; 0 <= snoidIdx; snoidIdx--) {
		ZmbSnoid *snoid = getSnoid(_snoidIds[snoidIdx]);
		if (!snoid)
			continue;
		manualLinkBefore(snoid, _rowLayerFeatures[8]);
		snoid->addFlag(ZmbFeature::FLAG_00008000_LOOP_ANIM);
		snoid->addFlag(ZmbFeature::FLAG_04000000_OVERLAY);
	}
}

void ZoombiniPuzzleSlides::restoreInitialSnoidFlags() {
	if (!_preRelinkSnoidFlagsPending)
		return;

	for (int16 snoidIdx = 0; snoidIdx < _pageLoadedZmbCount; snoidIdx++) {
		ZmbSnoid *snoid = getSnoid(_snoidIds[snoidIdx]);
		if (snoid)
			snoid->setFlags(_preRelinkSnoidFlags);
	}
	_preRelinkSnoidFlagsPending = false;
}

int16 ZoombiniPuzzleSlides::getBackwardChainLink(int16 cellIdx) const {
	if (cellIdx < 0 || kNumCells <= cellIdx)
		return -1;

	int16 base = cellIdx * kFieldsPerCell;
	if (_cellGrid[base + 3] != -1)
		return _cellGrid[base + 3];
	if (_cellGrid[base + 4] != -1)
		return _cellGrid[base + 4];
	return _cellGrid[base + 5];
}

int16 ZoombiniPuzzleSlides::getForwardChainLink(int16 cellIdx) const {
	if (cellIdx < 0 || kNumCells <= cellIdx)
		return -1;

	int16 base = cellIdx * kFieldsPerCell;
	if (_cellGrid[base + 8] != -1)
		return _cellGrid[base + 8];
	if (_cellGrid[base + 7] != -1)
		return _cellGrid[base + 7];
	return _cellGrid[base + 6];
}

bool ZoombiniPuzzleSlides::cellStateIs(int16 cellIdx, int16 stateA, int16 stateB, int16 stateC) const {
	if (cellIdx < 0 || kNumCells <= cellIdx)
		return false;

	int16 state = _cellGrid[cellIdx * kFieldsPerCell + 1];
	return state == stateA || state == stateB || state == stateC;
}

bool ZoombiniPuzzleSlides::cellsMatchTrait(int16 leftCellIdx, int16 rightCellIdx, int16 traitKind) const {
	if (!cellStateIs(leftCellIdx, kCellOccupied, kCellLocked) || !cellStateIs(rightCellIdx, kCellOccupied, kCellLocked))
		return false;

	ZmbSnoid *leftSnoid = getSnoid(_cellGrid[leftCellIdx * kFieldsPerCell + 2]);
	ZmbSnoid *rightSnoid = getSnoid(_cellGrid[rightCellIdx * kFieldsPerCell + 2]);
	if (!leftSnoid || !rightSnoid)
		return false;

	switch (traitKind) {
	case kTraitHair:
		return leftSnoid->_trait._hair == rightSnoid->_trait._hair;
	case kTraitEyes:
		return leftSnoid->_trait._eyes == rightSnoid->_trait._eyes;
	case kTraitNose:
		return leftSnoid->_trait._nose == rightSnoid->_trait._nose;
	case kTraitFeet:
		return leftSnoid->_trait._feet == rightSnoid->_trait._feet;
	default:
		return false;
	}
}

bool ZoombiniPuzzleSlides::setCellStateAndReload(int16 cellIdx, int16 state, int16 scrbId) {
	if (cellIdx < 0 || kNumCells <= cellIdx)
		return false;

	ensureCellFeature(cellIdx);
	int16 base = cellIdx * kFieldsPerCell;
	bool changed = (_cellGrid[base + 1] != state);
	_cellGrid[base + 1] = state;
	loadRunnerSCRB(_cellGrid[base + 0], scrbId);
	return changed;
}

int16 ZoombiniPuzzleSlides::findCellIdxForFeature(const ZmbFeature *feature) const {
	if (!feature)
		return -1;

	for (int16 cellIdx = 0; cellIdx < kNumCells; cellIdx++) {
		if (_cellFeatures[cellIdx] == feature)
			return cellIdx;
	}
	for (int16 slotIdx = 0; slotIdx < _slotCount; slotIdx++) {
		if (_slotFeatures[slotIdx] == feature)
			return _slotCellIndices[slotIdx];
	}

	// A DRAW_ON_REG helper materializes its first frame before loadScrbFeature() returns.
	// Its pointer is not stored yet, but its fixed authored point still identifies the owning cell.
	if (feature->getScrbId() == kResScrb7013_PlacementSlot) {
		for (int16 slotIdx = 0; slotIdx < _slotCount; slotIdx++) {
			const int16 cellIdx = _slotCellIndices[slotIdx];
			if (0 <= cellIdx && cellIdx < kNumCells &&
				feature->getPointLoc() == getCellSnoidPosition(cellIdx)) {
				return cellIdx;
			}
		}
	}

	return -1;
}

void ZoombiniPuzzleSlides::setCellFeaturePreRenderHook(ZmbFeature *feature, int16 scrbId) {
	if (!feature)
		return;

	ZmbFeature::OnPreRenderShapeFunc callback = nullptr;

	if (scrbId == kResScrb7000_CellBase) {
		callback = static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniPuzzleSlides::filterHotspotScript);
	} else if (scrbId == kResScrb7002_MatchedCell) {
		callback = static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniPuzzleSlides::processCommandQueue);
	}

	feature->setPreRenderShapeFunc(callback);
}

void ZoombiniPuzzleSlides::syncCellFeatureScript(int16 cellIdx) {
	if (cellIdx < 0 || kNumCells <= cellIdx)
		return;

	ZmbFeature *feature = _cellFeatures[cellIdx];
	if (!feature)
		return;

	int16 scrbId = kResScrb7000_CellBase;
	int16 state = _cellGrid[cellIdx * kFieldsPerCell + 1];

	if (state == kCellInert) {
		scrbId = kResScrb7001_InertCell;
	}

	setCellFeaturePreRenderHook(feature, scrbId);
	loadScrbOntoFeature(feature, scrbId, false);
	feature->activateRender();
	feature->deactivateAnimate();
	requestCellLinkOverlayRefresh();
}

void ZoombiniPuzzleSlides::loadCellLinkOverlay() {
	ZmbFeature::EventHooks hooks;
	hooks.setPreRenderFunc(static_cast<ZmbFeature::OnPreRenderFunc>(&ZoombiniPuzzleSlides::cellLinkOverlay_preRender));

	_cellLinkOverlayActive = true;
	_cellLinkOverlayFeature = loadVirtualFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape7000_CellAnimations), 0, 0,
		ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_08000000_REGION_TRACK,
		hooks);
	_cellLinkOverlayFeature->activateRender();
	_cellLinkOverlayFeature->deactivateAnimate();
	requestCellLinkOverlayRefresh();
}

bool ZoombiniPuzzleSlides::cellLinkOverlay_preRender(ZmbFeature *feature) {
	if (feature != _cellLinkOverlayFeature || !_cellLinkOverlayDirty)
		return false;

	struct CellLinkShape {
		uint16 adjacencyMask;
		int16 shapeId;
		Common::Point offset;
	};
	static constexpr CellLinkShape kCellLinkShapes[] = {
		{kAdjNorthEast, 24, Common::Point(2, -4)},
		{kAdjNorthWest, 4, Common::Point(26, -5)},
		{kAdjWest, 8, Common::Point(36, 6)}};

	Common::Array<ZmbHotspot> hotspots;
	uint16 hotspotIdx = 0;
	if (_cellLinkOverlayActive) {
		for (int16 cellIdx = 0; cellIdx < kNumCells; cellIdx++) {
			ZmbFeature *cellFeature = _cellFeatures[cellIdx];
			if (!cellFeature || cellFeature->getScrbId() != kResScrb7000_CellBase)
				continue;

			for (uint shapeIdx = 0; shapeIdx < ARRAYSIZE(kCellLinkShapes); shapeIdx++) {
				const CellLinkShape &linkShape = kCellLinkShapes[shapeIdx];
				if ((_cellAdjacencyMasks[cellIdx] & linkShape.adjacencyMask) == 0)
					continue;

				const Common::Point linkPos = kCellPositions[cellIdx] + linkShape.offset;
				hotspots.push_back(ZmbHotspot(hotspotIdx, linkShape.shapeId, 0, linkPos.x, linkPos.y));
				hotspotIdx += 1;
			}
		}
	}

	feature->setVirtualHotspots(hotspots);
	feature->activateRender();
	feature->deactivateAnimate();
	_cellLinkOverlayDirty = false;
	return true;
}

void ZoombiniPuzzleSlides::requestCellLinkOverlayRefresh() {
	if (!_cellLinkOverlayFeature)
		return;

	_cellLinkOverlayDirty = true;
	_cellLinkOverlayFeature->requestVisualRematerialization();
}

void ZoombiniPuzzleSlides::handoffCellLinksToGridRunners() {
	if (!_cellLinkOverlayActive)
		return;

	_cellLinkOverlayActive = false;
	requestCellLinkOverlayRefresh();

	for (int16 cellIdx = 0; cellIdx < kNumCells; cellIdx++) {
		ZmbFeature *cellFeature = _cellFeatures[cellIdx];
		if (cellFeature && cellFeature->getScrbId() == kResScrb7000_CellBase)
			cellFeature->requestVisualRematerialization();
	}
}

uint16 ZoombiniPuzzleSlides::getAdjMaskForCommand(int16 cmd) {
	switch (cmd) {
	case 4:
		return kAdjNorthWest;
	case 8:
		return kAdjWest;
	case 12:
		return kAdjSouthWest;
	case 16:
		return kAdjSouthEast;
	case 20:
		return kAdjEast;
	case 24:
		return kAdjNorthEast;
	default:
		return 0;
	}
}

void ZoombiniPuzzleSlides::filterHotspotScript(ZmbFeature *feature, ZmbHotspotGroup *hsGroup,
											   Common::Array<ZmbHotspot> &hotspots) {
	if (!hsGroup)
		return;

	int16 cellIdx = findCellIdxForFeature(feature);
	if (cellIdx < 0)
		return;

	int16 base = cellIdx * kFieldsPerCell;
	int16 cellState = _cellGrid[base + 1];
	int16 cellData = _cellGrid[base + 2];
	uint16 adjFlags = _cellAdjacencyMasks[cellIdx];

	for (uint32 hotspotIdx = 0; hotspotIdx < hotspots.size();) {
		int16 cmd = hotspots[hotspotIdx]._shapeIdx;
		bool keep = true;

		if (cmd == 4 || cmd == 8 || cmd == 24) {
			uint16 mask = getAdjMaskForCommand(cmd);
			keep = !_cellLinkOverlayActive && (adjFlags & mask) != 0;
		} else if (cmd == 73) {
			keep = (cellData == kTraitFeet);
		} else if (cmd == 74) {
			keep = (cellData == kTraitHair);
		} else if (cmd == 75) {
			keep = (cellData == kTraitNose);
		} else if (cmd == 76) {
			keep = (cellData == kTraitEyes);
		} else if (cmd == 103) {
			keep = (cellState == kCellConnector);
		} else if (cmd == 109) {
			keep = (cellState == kCellMatched || cellState == kCellSlotBase1 || cellState == kCellLocked) &&
				   _slotCellState != kCellSlotBase2;
		} else if (cmd == 110) {
			keep = (cellState == kCellMatched || cellState == kCellSlotBase2 || cellState == kCellLocked) &&
				   _slotCellState == kCellSlotBase2;
		}

		if (!keep) {
			hotspots.remove_at(hotspotIdx);
		} else {
			hotspotIdx += 1;
		}
	}
}

void ZoombiniPuzzleSlides::filterCommandByFlags(ZmbFeature *feature, ZmbHotspotGroup *hsGroup,
												Common::Array<ZmbHotspot> &hotspots) {
	if (!hsGroup)
		return;

	int16 cellIdx = findCellIdxForFeature(feature);
	if (cellIdx < 0)
		return;

	uint16 adjFlags = _cellAdjacencyMasks[cellIdx];

	for (uint32 hotspotIdx = 0; hotspotIdx < hotspots.size();) {
		ZmbHotspot &hotspot = hotspots[hotspotIdx];
		hotspot._x -= 22;
		hotspot._y += 6;

		uint16 mask = getAdjMaskForCommand(hotspot._shapeIdx);
		if (mask != 0 && (adjFlags & mask) == 0) {
			hotspots.remove_at(hotspotIdx);
		} else {
			hotspotIdx += 1;
		}
	}
}

void ZoombiniPuzzleSlides::processCommandQueue(ZmbFeature *feature, ZmbHotspotGroup *hsGroup,
											   Common::Array<ZmbHotspot> &hotspots) {
	if (!hsGroup)
		return;

	int16 cellIdx = findCellIdxForFeature(feature);
	if (cellIdx < 0)
		return;

	int16 base = cellIdx * kFieldsPerCell;
	uint16 adjFlags = _cellAdjacencyMasks[cellIdx];
	int16 cellState = _cellGrid[base + 1];

	for (uint32 hotspotIdx = 0; hotspotIdx < hotspots.size();) {
		int16 cmd = hotspots[hotspotIdx]._shapeIdx;
		bool keep = true;

		if (cmd == 4 || cmd == 8 || cmd == 24) {
			uint16 mask = getAdjMaskForCommand(cmd);
			if ((adjFlags & mask) != 0) {
				hotspots[hotspotIdx]._shapeIdx += _linkShapeVariantOffset;
			} else {
				keep = false;
			}
		} else if (cmd == 109) {
			keep = (cellState == kCellMatched || cellState == kCellSlotBase1 || cellState == kCellLocked) &&
				   _slotCellState != kCellSlotBase2;
		} else if (cmd == 110) {
			keep = (cellState == kCellMatched || cellState == kCellSlotBase2 || cellState == kCellLocked) &&
				   _slotCellState == kCellSlotBase2;
		}

		if (!keep) {
			hotspots.remove_at(hotspotIdx);
		} else {
			hotspotIdx += 1;
		}
	}
}

void ZoombiniPuzzleSlides::invalidateVisualRects(uint16 rectIdx, ZmbFeature *feature) {
	(void)rectIdx;
	(void)feature;

	// Mark visual regions for redraw
}

// =============================================================================
// Snoid Constraints
// =============================================================================

const Common::Rect &ZoombiniPuzzleSlides::getDragConstraintRect() const {
	return _dragConstraint;
}

} // End of namespace Mohawk
