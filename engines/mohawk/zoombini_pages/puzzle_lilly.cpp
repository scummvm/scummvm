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

#include "common/algorithm.h"

#include "mohawk/cursors.h"
#include "mohawk/zoombini.h"
#include "mohawk/zoombini_graphics.h"
#include "mohawk/zoombini_pages/grid_traversal.h"
#include "mohawk/zoombini_pages/puzzle_lilly.h"
#include "mohawk/zoombini_random.h"
#include "mohawk/zoombini_resource.h"
#include "mohawk/zoombini_sound.h"
#include "mohawk/zoombini_state.h"

namespace Mohawk {

// =================================================================
// Static data tables
// =================================================================

constexpr int16 ZoombiniPuzzleLilly::kColYOffset[13];
constexpr int16 ZoombiniPuzzleLilly::kSwapPairCol[20];
constexpr int16 ZoombiniPuzzleLilly::kSwapPairRow[20];
constexpr int16 ZoombiniPuzzleLilly::kZmbToRowCount[21];
constexpr byte ZoombiniPuzzleLilly::kCombinedAttrBase[5];
constexpr int16 ZoombiniPuzzleLilly::kRowColValidity[13];
constexpr ZmbLillyGridWalker::PadAttrType ZoombiniPuzzleLilly::kPatternAttrType[13];
constexpr int16 ZoombiniPuzzleLilly::kPatternAttrValue[13];
constexpr int16 ZoombiniPuzzleLilly::kPatternAttrExtra[13];
constexpr int16 ZoombiniPuzzleLilly::kObstacleBFSOffset[5];
constexpr Common::Point ZoombiniPuzzleLilly::kEntryPositions[12];
constexpr Common::Point ZoombiniPuzzleLilly::kInitialPositions[20];

void ZmbLillyGridWalker::clear() {
	runnerKind = kRunnerNone;
	toadRetired = false;
	packSnoidIdx = -1;
	entrySlot = 0;
	progressAxis = kProgressByColumn;
	attrType = kPadAttrNone;
	attrValue = 0;
	tattooIdx = 0;
	childRunnerIdx = -1;
	riderShapeOffset = 0;
	childShapeOffset = 0;
	toadRunnerIdx = -1;
	entryRow = 0;
	occupiesGridCell = false;
	gridCol = 0;
	gridRow = 0;
	prevCol = 0;
	prevRow = 0;
	frontierCol = 0;
	frontierRow = 0;
	direction = kDirectionRight;
	maxReachableProgress = 11;
	stepCount = 1;
	scrbKey = 0;
	completedCrossingCount = 0;
	nextMoveReadyFrame = 0;
	traversalMode = kTraversalFresh;
	bfsReinitPending = false;
	moveTargetX = 0;
	moveTargetY = 0;
	moveStartX = 0;
	moveStartY = 0;
	scriptTargetX = 0;
	scriptTargetY = 0;
	lastAnimPhase = kNoAnimationPhase;
	callbackMode = kCallbackNone;
	memset(visitGrid, 0, sizeof(visitGrid));
}

constexpr Common::Point ZoombiniPuzzleLilly::kToadHomePosition;
constexpr Common::Point ZoombiniPuzzleLilly::kToadCellPickOffset;

constexpr int16 ZoombiniPuzzleLilly::kCursorShapeBase[4];
constexpr int16 ZoombiniPuzzleLilly::kCursorBlinkFrames[4];
constexpr int16 ZoombiniPuzzleLilly::kCellBlinkShapeBase[4];

// =================================================================
// Construction / Lifecycle
// =================================================================

ZoombiniPuzzleLilly::ZoombiniPuzzleLilly(MohawkEngine_Zoombini *vm) : ZoombiniPuzzle(vm, ZoombiniPageType::kLilly, ZmbSrcPageKind::kLilly_08) {
	memset(_authoredPathGrids, 0, sizeof(_authoredPathGrids));
	memset(_cellOccupancy, 0, sizeof(_cellOccupancy));
	memset(_exitCellReservations, 0, sizeof(_exitCellReservations));
	memset(_padPattern, 0, sizeof(_padPattern));
	memset(_padShape, 0, sizeof(_padShape));
	memset(_padColor, 0, sizeof(_padColor));
	memset(_padDecorationShape, 0, sizeof(_padDecorationShape));
	memset(_farBankTurnInQueue, 0, sizeof(_farBankTurnInQueue));
	memset(_bankReturnQueue, 0, sizeof(_bankReturnQueue));
	memset(_retirementQueue, 0, sizeof(_retirementQueue));
	memset(_farBankRotationQueue, 0, sizeof(_farBankRotationQueue));
	memset(_childMountQueue, 0, sizeof(_childMountQueue));
	memset(_childDepartureQueue, 0, sizeof(_childDepartureQueue));
	memset(_toadStepReadyQueue, 0, sizeof(_toadStepReadyQueue));
	memset(_toadPathStepQueue, 0, sizeof(_toadPathStepQueue));
	memset(_pendingToadStepReadyQueue, 0, sizeof(_pendingToadStepReadyQueue));
	memset(_pendingToadPathStepQueue, 0, sizeof(_pendingToadPathStepQueue));
	memset(_toadPathInitQueue, 0, sizeof(_toadPathInitQueue));
	memset(_activeCrabRunnerIndices, 0, sizeof(_activeCrabRunnerIndices));
	memset(_runnerCleanupQueue, 0, sizeof(_runnerCleanupQueue));
	memset(_crabTrailState, 0, sizeof(_crabTrailState));
	for (int i = 0; i < kMaxRunners; i++)
		_runnerStates[i].clear();
}

ZoombiniPuzzleLilly::~ZoombiniPuzzleLilly() {
	if (_pageDragCursorHidden)
		_vm->_cursor->showCursor();
}

const ZoombiniPage::ScriptSoundPriorityRanges &ZoombiniPuzzleLilly::getScriptSoundPriorityRanges() const {
	// Original shared page dispatch automatically prepends SND 996-997 at priority 32.
	static const ZoombiniPage::ScriptSoundPriorityRanges kRanges = {
		{kSysResSoundRange0996_DepartSFX, kSysResSoundRange0997_ArriveSFX},
		// Lilly then registers the same range again in its authored list.
		{kSysResSoundRange0996_DepartSFX, kSysResSoundRange0997_ArriveSFX},
		{kSysResSoundRange20000_Base, kSysResSoundRange29999_Last},
		{kResSoundRange11000_SystemRangeBase, kResSoundRange11001_SystemRangeLast},
		{kResSoundRange12000_SwapBase, kResSoundRange12004_SwapRangeLast},
		{kResSoundRange10009_Priority, kResSoundRange10009_Priority},
		{kResSoundRange10010_Priority, kResSoundRange10010_Priority},
		{kResSoundRange10011_Priority, kResSoundRange10011_Priority},
		{kResSoundRange10000_SwapBase, kResSoundRange10000_SwapBase},
		{kResSoundRange10002_Priority, kResSoundRange10002_Priority},
		{kResSoundRange10004_Priority, kResSoundRange10004_Priority},
		{kResSoundRange10003_Priority, kResSoundRange10003_Priority},
		{kResSoundRange10005_PriorityBase, kResSoundRange10008_PriorityLast}};
	return kRanges;
}

void ZoombiniPuzzleLilly::open() {
	openArchive(ZMB_MHK_LILLY);
}

void ZoombiniPuzzleLilly::setBackgroundBitmap() {
	_vm->_gfx->setPalette(kResBackground5000);
	_vm->_gfx->drawBackground(kResBackground5000);
}

// =================================================================
// Initialize all puzzle features and state.
// =================================================================

void ZoombiniPuzzleLilly::initStates() {
	_swapWandHomeClickRect = Common::Rect();
	_nextCrabSpawnFrame = getCurrentFrameCounter() + 600;
}

void ZoombiniPuzzleLilly::loadFeatures() {
	// Preload shape images
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape7000_LilyPads));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape13000_Detail));

	// Load main features: 1 SCRB at 11000
	ZmbFeature *mainFeature = createPuzzleMainFeatureHead();

	{
		ZmbFeature *parent = mainFeature;
		for (uint16 i = 0; i < 5; i++) {
			parent = loadSubFeature(parent, ZmbResource(ZmbResource::kPage, kResBitmapShape14000_FinalOverlay), kResScrb14000_FinalBase + i);
		}
	}
	{
		ZmbFeature *parent = mainFeature;
		for (uint16 i = 0; i < 167; i++) {
			parent = loadSubFeature(parent, ZmbResource(ZmbResource::kPage, kResBitmapShape10000_FeatureOverlay), kResScrb10000_FeatureBase + i);
		}
	}

	// Load paired X/Y REGS shape-registration offsets.
	loadShapeOffsetRegs(ZmbResource::kPage, kResRegs0100_Main);
	loadShapeOffsetRegs(ZmbResource::kPage, kResRegs10000_FeatureOverlay);
	loadShapeOffsetRegs(ZmbResource::kPage, kResRegs0200_LilyPads);

	// Load zoombinis from pack (hidden; children represent them visually)
	loadZoombinisFromPack();

	// Initialize difficulty
	setDifficultyParams();

	// Load REGS coordinate tables for cell positioning and pad shape offsets.
	// Call @ref ZoombiniPuzzleLilly::loadRegsCoordinateTables() first.
	// @ref ZoombiniPuzzleLilly::initGridWithAttributes() then reads the coordinate tables.
	// @ref ZoombiniPuzzleLilly::_gridRowBaseX and @ref ZoombiniPuzzleLilly::_gridRowBaseY store those values.
	loadRegsCoordinateTables();

	// Initialize grid
	initGridWithAttributes();

	// Grid sprite renderer: a callback-only runner draws all 144 pads through the render clip every frame,
	// so only invalidated regions actually change on screen.
	{
		ZmbFeature::EventHooks hooks;
		hooks.setRenderFunc(static_cast<ZmbFeature::OnRenderFunc>(&ZoombiniPuzzleLilly::renderGridSprites));
		loadScrbFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape10000_FeatureOverlay), 0, 0,
			ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_04000000_OVERLAY,
			hooks);
	}

	// The entry-cell cursor runner redraws the highlighted destination every five frames.
	{
		ZmbFeature::EventHooks hooks;
		hooks.setRenderFunc(static_cast<ZmbFeature::OnRenderFunc>(&ZoombiniPuzzleLilly::renderCursorIndicator));
		_entryHighlightFeature = loadScrbFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape10000_FeatureOverlay), 0, 5,
			ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_04000000_OVERLAY,
			hooks);
		if (_entryHighlightFeature)
			_entryHighlightFeature->deactivateRender();
	}

	// Swap-cell blink runners (interval 4).
	{
		ZmbFeature::EventHooks hooks;
		hooks.setRenderFunc(static_cast<ZmbFeature::OnRenderFunc>(&ZoombiniPuzzleLilly::renderCellAnimA));
		_firstSwapCellFeature = loadScrbFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape10000_FeatureOverlay), 0, 4,
			ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_04000000_OVERLAY,
			hooks);
		if (_firstSwapCellFeature)
			_firstSwapCellFeature->deactivateRender();
	}
	{
		ZmbFeature::EventHooks hooks;
		hooks.setRenderFunc(static_cast<ZmbFeature::OnRenderFunc>(&ZoombiniPuzzleLilly::renderCellAnimB));
		_secondSwapCellFeature = loadScrbFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape10000_FeatureOverlay), 0, 4,
			ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_04000000_OVERLAY,
			hooks);
		if (_secondSwapCellFeature)
			_secondSwapCellFeature->deactivateRender();
	}

	// Zoombini child runners (staging cluster) + the 12 toads
	createZoombiniChildRunners();
	createToadRunners();

	// Load the five persistent overlay features for SCRB 14000-14004.
	for (uint16 i = 0; i < 5; i++) {
		loadScrbFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape14000_FinalOverlay),
			kResScrb14000_FinalBase + i, 0,
			ZmbFeature::FLAG_04000000_OVERLAY);
	}

	// Toad (1 < difficulty)
	if (kPuzzleLevel2 <= _difficultyLevel) {
		// Toad intro SCRB runner (prio 5, SCRB 11000).
		_toadIntroFeature = loadScrbFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape11000_Water), kResScrb11000_ToadIntro, 5,
			ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE);

		// Interactive swap-wand runner (interval 6, SCRB 10078+level).
		// Registered with OVERLAY and no POS_DELTA, then immediately reduced to DEFER_RENDER only.
		// Event 4 installs the engaged flag set, which is where POS_DELTA first appears.
		_swapWandFeature = loadScrbFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape10000_FeatureOverlay), kResScrb10078_ToadBase, 6,
			ZmbFeature::FLAG_00000002_TYPE_GRIDWALKER | ZmbFeature::FLAG_00080000_DEFER_ANIM |
				ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_04000000_OVERLAY);

		// Levels 2-4 begin with the first-cell selection phase.
		_swapSelectionState = CellSelectionState::kSelectFirst04;
		if (_swapWandFeature) {
			// The swap wand uses its authored hotspot as the cursor anchor.
			// Standard shape registration would shift the visual away from input.
			_swapWandFeature->setShapeOffsetRegs(nullptr);
			setToadHomePosition();
			// Keep the inactive runner out of the persistent post-render pass until event 4.
			// The original render gate alone suppresses this frame; DEFER_RENDER represents
			// the same state in the persistent ScummVM compositor.
			_swapWandFeature->setFlags(ZmbFeature::FLAG_01000000_DEFER_RENDER);
			_swapWandFeature->deactivateRender();
		}
	} else {
		_swapSelectionState = CellSelectionState::kInactive00;
	}

	// 12 No-op runners are persistent row anchors.
	// Grid entities are re-linked immediately before the appropriate anchor
	// whenever they are placed or start a move. This explicit order is required
	// after the first z-sort marks the entity runners as OVERLAY.
	{
		ZmbFeature::EventHooks hooks;
		hooks.setPreRenderFunc(static_cast<ZmbFeature::OnPreRenderFunc>(&ZoombiniPuzzleLilly::padAnchorPreRender));
		hooks.setRenderFunc(static_cast<ZmbFeature::OnRenderFunc>(&ZoombiniPuzzleLilly::renderPadAnchor));
		for (int16 row = 0; row < 12; row++) {
			_padRowAnchorFeatures[row] = loadVirtualFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape14000_FinalOverlay),
															kResScrb14000_FinalBase, 0,
															ZmbFeature::FLAG_04000000_OVERLAY, hooks);
		}
	}

	// Preserve the first sorted runner list, then switch Lilly to persistent manual links for
	// the rest of the page lifetime. Every later grid placement and move relinks its entity
	// explicitly against the pad-row anchors, so no further positional sort may run.
	renderFeatures();

	// The controls are registered after the setup materialization boundary.
	configureStandardPuzzleControlRects();
	loadStandardPuzzleControlFeatures(kResBitmapShape7000_LilyPads);

	if (_difficultyLevel == kPuzzleLevel1) {
		queueScriptSoundForNextRenderPass(ZmbResource(ZmbResource::kSystem, kSysResSound0997_Intro));
	}

	// Clear the active-pack state after the puzzle has consumed it.
	schedulePackSnoids(false, false);

	// Load toad intro SCRB 11000 so events 3 and 4 fire.
	// Its event handler drives the last two walk-in Zoombinis.
	if (kPuzzleLevel2 <= _difficultyLevel && _toadIntroFeature)
		loadScrbOntoFeature(_toadIntroFeature, kResScrb11000_ToadIntro);

	setRunnerZSortEnabled(false);
}

void ZoombiniPuzzleLilly::initHelpPrompt() {
	int16 helpSoundId;
	if (_entrySfxGroupFlags == ZmbSfxGroupFlags::kHardFirstTrigger_02) {
		helpSoundId = _vm->_rnd->getRandomNumber(kSysResSound20076_HelpLevel2, kSysResSound20077_HelpHard);
	} else if (_difficultyLevel <= kPuzzleLevel1) {
		helpSoundId = kSysResSound20075_HelpEasy;
	} else {
		helpSoundId = _vm->_rnd->getRandomNumber(kSysResSound20075_HelpEasy, kSysResSound20077_HelpHard);
	}
	_activeHelpSoundId = ZmbResource(ZmbResource::kSystem, helpSoundId);
}

// =================================================================
// Initialization helpers
// =================================================================

void ZoombiniPuzzleLilly::onGoButtonActivated() {
	// After arming the transition, discard the pending walk-in count and restore Toad pickup permission.
	// Active feature scripts are deliberately left untouched.
	_pendingChildWalkInCount = 0;
	_toadPickupInputEnabled = true;
	ZoombiniInteractive::onGoButtonActivated();
}

void ZoombiniPuzzleLilly::executeDeparture() {
	// Lilly's visible child runners represent real pack Snoids that stay hidden offscreen.
	// Reactivate the real runners so the shared writer preserves their completed occupancy state.
	for (uint16 i = 0; i < _packSnoids.size(); i++) {
		if (_packSnoids[i])
			_packSnoids[i]->activateRender();
	}

	ZoombiniInteractive::executeDeparture();
}

void ZoombiniPuzzleLilly::loadZoombinisFromPack() {
	// Real pack snoids are hidden at (680, 220); the child runners represent them visually.
	const Common::Point offscreenPos(680, 220);
	Common::Point positions[20];
	for (uint16 i = 0; i < ARRAYSIZE(positions); i++)
		positions[i] = offscreenPos;

	_packSnoids.clear();
	_pageLoadedZmbCount = loadOccupiedSnoidsFromActivePack(positions, ARRAYSIZE(positions), &_packSnoids);
	for (uint16 i = 0; i < _packSnoids.size(); i++)
		_packSnoids[i]->deactivateRender();
}

void ZoombiniPuzzleLilly::setDifficultyParams() {
	// Levels 3 and 4 cycle through two or three generated crab-entry slots.
	switch (_difficultyLevel) {
	case kPuzzleLevel1:
	case kPuzzleLevel2:
		_crabEntryCycleLength = 0;
		break;
	case kPuzzleLevel3:
		_crabEntryCycleLength = 2;
		break;
	case kPuzzleLevel4:
	default:
		_crabEntryCycleLength = 3;
		break;
	}
}

void ZoombiniPuzzleLilly::loadRegsCoordinateTables() {
	// REGS 100: per-row X/Y base coordinates for the 12x12 grid
	Common::HashMap<int16, ZmbShapeOffsetRegs *>::const_iterator it100 = _shapeOffsetRegsMap.find(100);
	if (it100 != _shapeOffsetRegsMap.end()) {
		ZmbShapeOffsetRegs *regs100 = it100->_value;
		_gridRowBaseX.clear();
		_gridRowBaseY.clear();
		for (uint i = 0; i < regs100->_offsets.size(); i++) {
			_gridRowBaseX.push_back(regs100->_offsets[i].x);
			_gridRowBaseY.push_back(regs100->_offsets[i].y);
		}
	}

	// REGS 200: per-shape registration offsets for the pad shapes (tBMP 13000)
	Common::HashMap<int16, ZmbShapeOffsetRegs *>::const_iterator it200 = _shapeOffsetRegsMap.find(200);
	if (it200 != _shapeOffsetRegsMap.end()) {
		ZmbShapeOffsetRegs *regs200 = it200->_value;
		_padShapeOffsetX.clear();
		_padShapeOffsetY.clear();
		for (uint i = 0; i < regs200->_offsets.size(); i++) {
			_padShapeOffsetX.push_back(regs200->_offsets[i].x);
			_padShapeOffsetY.push_back(regs200->_offsets[i].y);
		}
	}
}

void ZoombiniPuzzleLilly::createZoombiniChildRunners() {
	// Stage one child runner per Zoombini with SCRB 10109+i.
	// The pre-render callback applies the sprite variant selected by the Snoid's hair trait.
	for (int16 i = 0; i < _pageLoadedZmbCount && i < kToadBase; i++) {
		ZmbFeature::EventHooks hooks;
		hooks.setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniPuzzleLilly::childPreRenderShape));
		_runnerFeatures[i] = loadScrbFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape10000_FeatureOverlay), kResScrb10109_ChildEntryBase + i, 4,
			ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_04000000_OVERLAY,
			hooks);

		ZmbLillyGridWalker &rs = _runnerStates[i];
		rs.clear();
		rs.runnerKind = ZmbLillyGridWalker::kRunnerChild;
		rs.packSnoidIdx = i;
		// Select the sprite variant from the Snoid's one-based hair trait.
		if (i < static_cast<int16>(_packSnoids.size()) && _packSnoids[i])
			rs.childShapeOffset = static_cast<int16>(_packSnoids[i]->_trait._hair) - 1;
		rs.childShapeOffset = MAX<int16>(rs.childShapeOffset, 0);

		const bool isFinalWalkIn = (i == _pageLoadedZmbCount - 2 || i == _pageLoadedZmbCount - 1);
		if (isFinalWalkIn) {
			_pendingChildWalkInCount = _pageLoadedZmbCount - i;
			_pendingChildWalkInCount = MAX<int16>(_pendingChildWalkInCount, 0);
		}

		if (!_runnerFeatures[i])
			continue;

		// Staging idle: first frame only (DEFER_ANIM)
		_runnerFeatures[i]->setPointLoc(kInitialPositions[i]);
		_runnerFeatures[i]->deactivateAnimate();

		if (isFinalWalkIn) {
			// Hide the final two children.
			// At difficulty 1, SCRB 10089+i starts their walk-in immediately; otherwise, toad-intro event 3 starts it.
			_runnerFeatures[i]->deactivateRender();
			if (_difficultyLevel == kPuzzleLevel1) {
				_runnerFeatures[i]->activateRender();
				loadScrbOntoFeature(_runnerFeatures[i], kResScrb10089_ChildBase + i);
				rs.callbackMode = ZmbLillyGridWalker::kCallbackSnoid;
			}
		}
	}
}

void ZoombiniPuzzleLilly::createToadRunners() {
	// 12 Toads on the left bank.
	// Tattoo assigned by shuffle-without-replacement over the 12 (type,value)
	// pool entries; the drawn deck index is the tattooIdx (pattern overlay
	// shape offset).
	int16 deck[kToadCount];
	for (int16 i = 0; i < kToadCount; i++)
		deck[i] = i;
	int16 deckSize = kToadCount - 1; // 0-based max index

	for (int16 toadRunnerIdx = 0; toadRunnerIdx < kToadCount; toadRunnerIdx++) {
		int16 runnerIdx = kToadBase + toadRunnerIdx;

		int16 randIdx = _vm->_rnd->getRandomNumber(0, deckSize);
		int16 tattooIdx = deck[randIdx];
		for (int16 deckIdx = randIdx; deckIdx < deckSize; deckIdx++)
			deck[deckIdx] = deck[deckIdx + 1];
		deckSize -= 1;

		ZmbLillyGridWalker &rs = _runnerStates[runnerIdx];
		rs.clear();
		rs.runnerKind = ZmbLillyGridWalker::kRunnerToad;
		rs.entrySlot = toadRunnerIdx;
		rs.progressAxis = ZmbLillyGridWalker::kProgressByColumn;
		rs.attrType = kPatternAttrType[tattooIdx];
		rs.attrValue = static_cast<byte>(kPatternAttrValue[tattooIdx]);
		rs.tattooIdx = tattooIdx;
		rs.direction = ZmbLillyGridWalker::kDirectionRight;

		// Preserve the otherwise-unused 3..6 random draw to keep the deterministic RNG sequence aligned.
		// The frame interval remains fixed at 7.
		_vm->_rnd->getRandomNumber(3, 6);
		ZmbFeature::EventHooks hooks;
		hooks.setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniPuzzleLilly::toadPreRenderShape));
		_runnerFeatures[runnerIdx] = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape10000_FeatureOverlay), kResScrb10043_EntryBase + toadRunnerIdx, 7,
													 ZmbFeature::FLAG_00000002_TYPE_GRIDWALKER | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_00800000_POS_DELTA,
													 hooks);

		if (!_runnerFeatures[runnerIdx])
			continue;

		// Bank idle, animating.
		loadScrbOntoFeature(_runnerFeatures[runnerIdx], kResScrb10043_EntryBase + toadRunnerIdx);
		// POS_DELTA anchor: bank position == authored SCRB anchor -> delta ~0.
		_runnerFeatures[runnerIdx]->setPointRef(kEntryPositions[toadRunnerIdx]);
		_runnerFeatures[runnerIdx]->setPointLoc(kEntryPositions[toadRunnerIdx]);

		Common::HashMap<int16, ZmbShapeOffsetRegs *>::const_iterator itRegs = _shapeOffsetRegsMap.find(kResRegs10000_FeatureOverlay);
		if (itRegs != _shapeOffsetRegsMap.end())
			_runnerFeatures[runnerIdx]->setShapeOffsetRegs(itRegs->_value);
	}
}

// =================================================================
// Grid generation
// =================================================================

void ZoombiniPuzzleLilly::loadGridPatternRegs(int gridIdx, int16 resId) {
	// One REGS resource contains a 12x12 int16 grid (288 bytes, big-endian, row-major).
	assert(0 <= gridIdx && gridIdx < 3);
	memset(_authoredPathGrids[gridIdx], 0, sizeof(_authoredPathGrids[gridIdx]));

	Common::SeekableReadStream *stream = _vm->getResource(ID_REGS, ZmbResource(ZmbResource::kPage, resId));
	static constexpr uint32 kGridPatternSize = 12 * 12 * sizeof(int16);
	if (!ZmbResource::hasSize(stream, kGridPatternSize, kGridPatternSize)) {
		delete stream;
		error("lilly: malformed required REGS %d grid pattern", resId);
		return;
	}

	static constexpr int16 kFirstPatternIndex[3] = {1, 4, 8};
	static constexpr int16 kLastPatternIndex[3] = {3, 7, 12};
	int16 parsedGrid[12][12];
	for (int row = 0; row < 12; row++) {
		for (int col = 0; col < 12; col++) {
			const int16 patternIndex = stream->readSint16BE();
			if (patternIndex != 0 && (patternIndex < kFirstPatternIndex[gridIdx] || kLastPatternIndex[gridIdx] < patternIndex)) {
				delete stream;
				error("lilly: invalid REGS %d pattern index %d", resId, patternIndex);
				return;
			}
			parsedGrid[row][col] = patternIndex;
		}
	}
	delete stream;
	memcpy(_authoredPathGrids[gridIdx], parsedGrid, sizeof(parsedGrid));
}

void ZoombiniPuzzleLilly::rotateGrid(int rotType, int16 grid[12][12]) {
	// @p rotType selects 90 degrees clockwise (0), 180 degrees (1), or 90 degrees counterclockwise (2).
	int16 temp[12][12];
	memset(temp, 0, sizeof(temp));

	switch (rotType) {
	case 0:
		for (int16 row = 0; row < 12; row++)
			for (int16 col = 0; col < 12; col++)
				temp[col][11 - row] = grid[row][col];
		break;
	case 1:
		for (int16 row = 0; row < 12; row++)
			for (int16 col = 0; col < 12; col++)
				temp[11 - row][11 - col] = grid[row][col];
		break;
	case 2:
		for (int16 row = 0; row < 12; row++)
			for (int16 col = 0; col < 12; col++)
				temp[11 - col][row] = grid[row][col];
		break;
	default:
		return;
	}

	memcpy(grid, temp, sizeof(temp));
}

void ZoombiniPuzzleLilly::flipGrid(int flipType, int16 grid[12][12]) {
	// @p flipType selects horizontal mirroring (0) or vertical mirroring (1).
	int16 temp[12][12];
	memset(temp, 0, sizeof(temp));

	switch (flipType) {
	case 0:
		for (int16 row = 0; row < 12; row++)
			for (int16 col = 0; col < 12; col++)
				temp[row][11 - col] = grid[row][col];
		break;
	case 1:
		for (int16 col = 0; col < 12; col++)
			for (int16 row = 0; row < 12; row++)
				temp[11 - row][col] = grid[row][col];
		break;
	default:
		return;
	}

	memcpy(grid, temp, sizeof(temp));
}

void ZoombiniPuzzleLilly::generateChallengePatterns() {
	// 12 challenge triplets (type, value, extra) picked without replacement
	// from 3 pools (pattern 3 / pad-shape 4 / color 5).
	ZmbLillyGridWalker::PadAttrType poolAType[4] = {};
	int16 poolAValue[4] = {}, poolAExtra[4] = {};
	for (int i = 0; i < 3; i++) {
		poolAType[i] = kPatternAttrType[i];
		poolAValue[i] = kPatternAttrValue[i];
		poolAExtra[i] = kPatternAttrExtra[i];
	}

	ZmbLillyGridWalker::PadAttrType poolBType[5] = {};
	int16 poolBValue[5] = {}, poolBExtra[5] = {};
	for (int i = 0; i < 4; i++) {
		poolBType[i] = kPatternAttrType[3 + i];
		poolBValue[i] = kPatternAttrValue[3 + i];
		poolBExtra[i] = kPatternAttrExtra[3 + i];
	}

	ZmbLillyGridWalker::PadAttrType poolCType[6] = {};
	int16 poolCValue[6] = {}, poolCExtra[6] = {};
	for (int i = 0; i < 5; i++) {
		poolCType[i] = kPatternAttrType[7 + i];
		poolCValue[i] = kPatternAttrValue[7 + i];
		poolCExtra[i] = kPatternAttrExtra[7 + i];
	}

	int16 poolASize = 2; // 0-based max index
	int16 poolBSize = 3;
	int16 poolCSize = 4;

	for (int16 entryIdx = 1; entryIdx < 13; entryIdx++) {
		ZmbLillyGridWalker::PadAttrType *curType;
		int16 *curValue, *curExtra, *curSize;

		if (static_cast<uint>(entryIdx - 1) < 3) {
			curType = poolAType;
			curValue = poolAValue;
			curExtra = poolAExtra;
			curSize = &poolASize;
		} else if (static_cast<uint>(entryIdx - 4) < 4) {
			curType = poolBType;
			curValue = poolBValue;
			curExtra = poolBExtra;
			curSize = &poolBSize;
		} else {
			curType = poolCType;
			curValue = poolCValue;
			curExtra = poolCExtra;
			curSize = &poolCSize;
		}

		int16 pick = _vm->_rnd->getRandomNumber(0, *curSize);
		_challengeAttrTypes[entryIdx] = curType[pick];
		_challengeAttrValues[entryIdx] = curValue[pick];
		_challengeTattooIndices[entryIdx] = curExtra[pick];

		for (int16 poolIdx = pick; poolIdx < *curSize + 1; poolIdx++) {
			curType[poolIdx] = curType[poolIdx + 1];
			curValue[poolIdx] = curValue[poolIdx + 1];
			curExtra[poolIdx] = curExtra[poolIdx + 1];
		}
		(*curSize) -= 1;
	}
}

void ZoombiniPuzzleLilly::recordGeneratedAnswerEntryRows() {
	memset(_generatedAnswerEntryRowMasks, 0, sizeof(_generatedAnswerEntryRowMasks));
	memset(_generatedDirectEntryRowMasks, 0, sizeof(_generatedDirectEntryRowMasks));

	// Each raw path in the transformed source grids owns one tattoo assignment.
	// Challenge placement may interrupt that path, so record its entry before
	// testing the generated pad attributes for direct reachability.
	for (int16 gridIdx = 0; gridIdx < 3; gridIdx++) {
		for (int16 entryRow = 0; entryRow < 12; entryRow++) {
			const int16 rawPathIdx = _authoredPathGrids[gridIdx][entryRow][0];
			if (rawPathIdx < 1 || 12 < rawPathIdx)
				continue;
			if (_difficultyLevel == kPuzzleLevel1 && _challengeEnabledMarkers[rawPathIdx] == 0)
				continue;

			const int16 tattooIdx = _challengeTattooIndices[rawPathIdx];
			if (tattooIdx < 0 || kToadCount <= tattooIdx)
				continue;
			_generatedAnswerEntryRowMasks[tattooIdx] |= static_cast<uint16>(1U << entryRow);
		}
	}

	for (int16 tattooIdx = 0; tattooIdx < kToadCount; tattooIdx++) {
		const uint16 generatedRows = _generatedAnswerEntryRowMasks[tattooIdx];
		for (int16 entryRow = 0; entryRow < 12; entryRow++) {
			const uint16 rowMask = static_cast<uint16>(1U << entryRow);
			if ((generatedRows & rowMask) == 0)
				continue;
			if (isAnswerEntryRowCrossable(
					kPatternAttrType[tattooIdx], static_cast<byte>(kPatternAttrValue[tattooIdx]), entryRow)) {
				_generatedDirectEntryRowMasks[tattooIdx] |= rowMask;
			}
		}
	}
}

void ZoombiniPuzzleLilly::initGridWithAttributes() {
	// --- Phase 1: Clear pattern tracking arrays ---
	memset(_challengePlacementCounts, 0, sizeof(_challengePlacementCounts));
	for (int16 i = 0; i < 13; i++) {
		_challengeEnabledMarkers[i] = i;
		_challengeSlotShuffle[i] = i;
	}

	// --- Phase 2: Clear occupancy grid ---
	for (int16 row = 0; row < 12; row++) {
		for (int16 col = 0; col < 13; col++) {
			_cellOccupancy[row][col] = 0;
			_exitCellReservations[row][col] = 0;
		}
	}

	// --- Phase 3: Load grid pattern REGS ---
	loadGridPatternRegs(0, 15000);
	loadGridPatternRegs(1, 15001);
	loadGridPatternRegs(2, 15002);

	// --- Phase 4: Select grid type and rotate primary pattern by difficulty ---
	int16 maxObstacleEntries = 0;
	int16 slotsToEnable = 12; // Number of pattern slots enabled

	if (_difficultyLevel != kPuzzleLevel1 && _difficultyLevel != kPuzzleLevel2) {
		if (_difficultyLevel == kPuzzleLevel3) {
			maxObstacleEntries = 2;
			if (kZmbToRowCount[_pageLoadedZmbCount] < 8) {
				int16 rndGrid = _vm->_rnd->getRandomNumber(3, 5);
				if (rndGrid == 3) {
					_obstacleGridFamily = 3;
					rotateGrid(0, _authoredPathGrids[0]);
				} else if (rndGrid == 4) {
					_obstacleGridFamily = 4;
					rotateGrid(0, _authoredPathGrids[1]);
				} else {
					_obstacleGridFamily = 5;
					rotateGrid(0, _authoredPathGrids[2]);
				}
			} else {
				_obstacleGridFamily = 4;
				rotateGrid(0, _authoredPathGrids[1]);
			}
		} else {
			maxObstacleEntries = 3;
			if (kZmbToRowCount[_pageLoadedZmbCount] < 8 &&
				_vm->_rnd->getRandomNumber(4, 5) != 4) {
				_obstacleGridFamily = 5;
				rotateGrid(0, _authoredPathGrids[2]);
			} else {
				_obstacleGridFamily = 4;
				rotateGrid(0, _authoredPathGrids[1]);
			}
		}
		slotsToEnable = 12;
	} else {
		maxObstacleEntries = 0;
		_obstacleGridFamily = 0;
		if (_difficultyLevel == kPuzzleLevel1)
			slotsToEnable = 12 - kZmbToRowCount[_pageLoadedZmbCount];
		else
			slotsToEnable = 12;
	}

	// --- Phase 5: Random rotation/flip of all 3 grids ---
	int16 transformType = _vm->_rnd->getRandomNumber(0, 2);
	if (transformType == 0) {
		rotateGrid(1, _authoredPathGrids[0]);
		rotateGrid(1, _authoredPathGrids[1]);
		rotateGrid(1, _authoredPathGrids[2]);
	} else if (transformType == 1) {
		flipGrid(_vm->_rnd->getRandomNumber(0, 1), _authoredPathGrids[0]);
		flipGrid(_vm->_rnd->getRandomNumber(0, 1), _authoredPathGrids[1]);
		flipGrid(_vm->_rnd->getRandomNumber(0, 1), _authoredPathGrids[2]);
	}

	// --- Phase 6: Enable random pattern slots (mask[slot] == 0 -> enabled) ---
	int16 shuffleMax = 12;
	for (int16 enabledSlotIdx = 0; enabledSlotIdx < slotsToEnable; enabledSlotIdx++) {
		int16 pick = _vm->_rnd->getRandomNumber(1, shuffleMax);
		_challengeEnabledMarkers[_challengeSlotShuffle[pick]] = 0;
		for (int16 slotIdx = pick; slotIdx < shuffleMax + 1; slotIdx++)
			_challengeSlotShuffle[slotIdx] = _challengeSlotShuffle[slotIdx + 1];
		shuffleMax -= 1;
	}

	// --- Phase 7: Generate challenge patterns ---
	generateChallengePatterns();

	// --- Phase 8: Fill grid attributes ---
	int16 entryCount = 0;
	int16 patternPlacedCount = 0;
	_crabEntryCount = 0;

	for (int16 gridColIdx = 0; gridColIdx < 12; gridColIdx++) {
		for (int16 columnIdx = 0; columnIdx < 12; columnIdx++) {
			_crabTrailState[gridColIdx][columnIdx] = -1;
			_cellOccupancy[gridColIdx][columnIdx] = 0;
			_padPattern[gridColIdx][columnIdx] = 0;
			_padShape[gridColIdx][columnIdx] = 0;

			bool hasAttr1 = false;
			bool hasAttr2 = false;
			bool hasAttr3 = false;

			for (int16 rowIdx = 0; rowIdx < 3; rowIdx++) {
				int16 rawVal = _authoredPathGrids[rowIdx][gridColIdx][columnIdx];
				if (rawVal == 0)
					continue;

				// Adjusted slot when this grid's family drives the obstacles
				int16 adjustedIdx;
				if (rowIdx == 0) {
					adjustedIdx = (_obstacleGridFamily == 3) ? rawVal : rawVal + 1;
					if (3 < adjustedIdx)
						adjustedIdx = 1;
				} else if (rowIdx == 1) {
					adjustedIdx = (_obstacleGridFamily == 4) ? rawVal : rawVal + 1;
					if (7 < adjustedIdx)
						adjustedIdx = 4;
				} else {
					adjustedIdx = (_obstacleGridFamily == 5) ? rawVal : rawVal + 1;
					if (12 < adjustedIdx)
						adjustedIdx = 8;
				}

				int16 patternIdx = rawVal;

				if (kRowColValidity[gridColIdx] != 0 && kRowColValidity[columnIdx] != 0 &&
					_challengeEnabledMarkers[rawVal] == 0 && _challengePlacementCounts[rawVal] < 2) {
					int16 rndCheck = _vm->_rnd->getRandomNumber(0, 100);
					if (75 < rndCheck || (gridColIdx == 11 && _challengePlacementCounts[rawVal] == 0)) {
						_challengePlacementCounts[rawVal] += 1;
						patternIdx = adjustedIdx;
						patternPlacedCount += 1;
					}
				}

				if (0 < patternIdx && patternIdx < 13) {
					ZmbLillyGridWalker::PadAttrType pType = _challengeAttrTypes[patternIdx];
					byte pValue = static_cast<byte>(_challengeAttrValues[patternIdx]);

					// Store each pad attribute type in its corresponding grid layer.
					if (pType == ZmbLillyGridWalker::kPadAttrPattern) {
						hasAttr1 = true;
						_padPattern[gridColIdx][columnIdx] = pValue;
					} else if (pType == ZmbLillyGridWalker::kPadAttrShape) {
						hasAttr2 = true;
						_padShape[gridColIdx][columnIdx] = pValue;
					} else if (pType == ZmbLillyGridWalker::kPadAttrColor) {
						hasAttr3 = true;
						_padColor[gridColIdx][columnIdx] = pValue;
					}

					// Record crab entry columns (difficulty 3/4, top row only)
					if ((_difficultyLevel == kPuzzleLevel3 || _difficultyLevel == kPuzzleLevel4) &&
						entryCount < maxObstacleEntries && gridColIdx == 0) {
						bool isObstaclePattern = false;
						if (_obstacleGridFamily == 3 && patternIdx <= 3)
							isObstaclePattern = true;
						else if (_obstacleGridFamily == 4 && 4 <= patternIdx && patternIdx <= 7)
							isObstaclePattern = true;
						else if (_obstacleGridFamily == 5 && 8 <= patternIdx)
							isObstaclePattern = true;

						if (isObstaclePattern) {
							_crabEntryColumns[entryCount] = columnIdx;
							_crabEntryAttrTypes[entryCount] = _challengeAttrTypes[patternIdx];
							_crabEntryAttrValues[entryCount] = _challengeAttrValues[patternIdx];
							entryCount += 1;
						}
					}
				}
			}

			// Random fill for unset attributes
			if (!hasAttr1)
				_padPattern[gridColIdx][columnIdx] = static_cast<byte>(_vm->_rnd->getRandomNumber(0, 2));
			if (!hasAttr2)
				_padShape[gridColIdx][columnIdx] = static_cast<byte>(_vm->_rnd->getRandomNumber(0, 3));
			if (!hasAttr3)
				_padColor[gridColIdx][columnIdx] = static_cast<byte>(_vm->_rnd->getRandomNumber(0, 4));

			// Combine the first and third attributes into the decoration shape.
			_padDecorationShape[gridColIdx][columnIdx] = _padPattern[gridColIdx][columnIdx] + kCombinedAttrBase[_padColor[gridColIdx][columnIdx]];

			// Position each cell from its column stride, row REGS offsets, and per-column vertical offset.
			int16 baseX;
			if (gridColIdx + 1 < static_cast<int16>(_gridRowBaseX.size()))
				baseX = _gridRowBaseX[gridColIdx + 1];
			else
				baseX = 50 + gridColIdx * 45;
			int16 baseY = (gridColIdx + 1 < static_cast<int16>(_gridRowBaseY.size())) ? _gridRowBaseY[gridColIdx + 1] : 100;

			int16 cellX = 35 * columnIdx + baseX;
			int16 cellY = kColYOffset[columnIdx] + baseY;

			_padCellPositions[gridColIdx][columnIdx] = Common::Point(cellX, cellY);
			_padCellRects[gridColIdx][columnIdx] = Common::Rect(cellX, cellY, cellX + 36, cellY + 30);
		}
	}

	_crabEntryCount = entryCount;

	// Crab family = first entry's type
	if (0 < _crabEntryCount)
		_crabPathAttrType = _crabEntryAttrTypes[0];
	else
		_crabPathAttrType = ZmbLillyGridWalker::kPadAttrNone;

	// Preserve the generated raw routes and post-challenge direct state before
	// the two preset pad swaps used by the intro.
	recordGeneratedAnswerEntryRows();

	// --- Phase 9: Initial cell swaps (difficulty > 1) ---
	if (kPuzzleLevel2 <= _difficultyLevel) {
		_firstSwapCellCol = kSwapPairCol[0];
		_firstSwapCellRow = kSwapPairRow[0];
		_secondSwapCellCol = kSwapPairCol[2];
		_secondSwapCellRow = kSwapPairRow[2];
		swapCellsAndUpdateRunners(_firstSwapCellCol, _firstSwapCellRow, _secondSwapCellCol, _secondSwapCellRow);

		_firstSwapCellCol = kSwapPairCol[4];
		_firstSwapCellRow = kSwapPairRow[4];
		_secondSwapCellCol = kSwapPairCol[6];
		_secondSwapCellRow = kSwapPairRow[6];
		swapCellsAndUpdateRunners(_firstSwapCellCol, _firstSwapCellRow, _secondSwapCellCol, _secondSwapCellRow);
	}

	// --- Phase 10: Swap unlock threshold = ceil((placed + 5) / 6) ---
	const int16 swapProgressNumerator = patternPlacedCount + 5;
	_swapsPerWandStage = (swapProgressNumerator + 5) / 6;
}

// =================================================================
// Mouse interaction: toad drag-and-drop and toad drag-and-swap selection.
// Implemented as a non-blocking state machine.
// =================================================================

ZmbEventHandleResult ZoombiniPuzzleLilly::onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) {
	// While the toad is glued to the cursor, every press is a pond click
	// (swap-cell selection) or a cancel; standard buttons are unreachable.
	if (_swapWandDragging) {
		// Apply a direct touchscreen tap before resolving its cell or cancel action.
		onMouseMove(absPos, relPos);
		handleToadPondClick(absPos);
		return ZmbEventHandleResult::kConsumed;
	}

	// Let base class handle standard buttons (Go/Map/Help)
	ZmbEventHandleResult result = ZoombiniInteractive::onLButtonDown(absPos, relPos);
	if (result == ZmbEventHandleResult::kConsumed)
		return result;

	if (!_pageActive || 0 <= _draggedToadRunnerIdx)
		return ZmbEventHandleResult::kPassthrough;

	// Start a Toad or swap-wand drag only while pickup input is enabled.
	if (!_toadPickupInputEnabled)
		return ZmbEventHandleResult::kPassthrough;

	// --- Toad pick-up (type 0). Blocked while walk-in pending. ---
	if (_pendingChildWalkInCount <= 0) {
		for (int16 toadRunnerIdx = 0; toadRunnerIdx < kToadCount; toadRunnerIdx++) {
			int16 runnerIdx = kToadBase + toadRunnerIdx;
			ZmbFeature *toad = _runnerFeatures[runnerIdx];
			ZmbLillyGridWalker &rs = _runnerStates[runnerIdx];
			if (!toad || rs.occupiesGridCell || rs.toadRetired)
				continue;
			// Only bank-idle toads are clickable (in-transit toads keep
			// placed set until event 49 restores them).
			if (toad->getScrbId() != static_cast<int16>(kResScrb10043_EntryBase + rs.entrySlot))
				continue;
			if (toad->hasClickRect() && toad->getClickRect().contains(absPos)) {
				beginToadDrag(runnerIdx, absPos);
				return ZmbEventHandleResult::kConsumed;
			}
		}
	}

	// --- Toad pick-up (type 2; bypasses the walk-in gate) ---
	// Event 4 removes DEFER_RENDER when the wand becomes eligible for interaction.
	// A later runtime SCRB reload may clear the transient render-active bit because the
	// static runner uses DEFER_ANIM, but its persistent home frame remains clickable.
	if (_swapWandFeature &&
		!_swapWandFeature->hasFlag(ZmbFeature::FLAG_01000000_DEFER_RENDER) &&
		isSwapWandAtPoint(absPos)) {
		beginToadSwapDrag(absPos);
		return ZmbEventHandleResult::kConsumed;
	}

	return ZmbEventHandleResult::kPassthrough;
}

ZmbEventHandleResult ZoombiniPuzzleLilly::onMouseMove(const Common::Point &absPos, const Common::Point &relPos) {
	_swapWandHoverPos = absPos;
	updateV2SwapWandHover();

	if (0 <= _draggedToadRunnerIdx) {
		updateToadDrag(absPos);
		return ZmbEventHandleResult::kConsumed;
	}
	if (_swapWandDragging && _swapWandFeature) {
		addExternalDirtyRect(_swapWandFeature->getZSortRect());
		_swapWandFeature->setPointLoc(Common::Point(CLIP<int16>(absPos.x, 0, 639), CLIP<int16>(absPos.y, 0, 479)));
		_swapWandFeature->requestVisualRematerialization();
		return ZmbEventHandleResult::kConsumed;
	}
	return ZoombiniInteractive::onMouseMove(absPos, relPos);
}

ZmbEventHandleResult ZoombiniPuzzleLilly::onLButtonUp(const Common::Point &absPos, const Common::Point &relPos) {
	if (0 <= _draggedToadRunnerIdx) {
		endToadDrag(absPos);
		return ZmbEventHandleResult::kConsumed;
	}
	// The toad stays glued to the cursor across button-up (modal loop
	// semantics); only a press outside the pond releases it.
	return ZoombiniInteractive::onLButtonUp(absPos, relPos);
}

bool ZoombiniPuzzleLilly::hasStickyMouseDrag() const {
	return (0 <= _draggedToadRunnerIdx) || _swapWandDragging || ZoombiniInteractive::hasStickyMouseDrag();
}

void ZoombiniPuzzleLilly::endStickyMouseDrag(const Common::Point &dropPos) {
	if (0 <= _draggedToadRunnerIdx) {
		endToadDrag(dropPos);
		return;
	}
	if (_swapWandDragging) {
		handleToadPondClick(dropPos);
		return;
	}

	ZoombiniInteractive::endStickyMouseDrag(dropPos);
}

void ZoombiniPuzzleLilly::beginToadDrag(int16 toadRunnerIdx, const Common::Point &absPos) {
	ZmbFeature *toad = _runnerFeatures[toadRunnerIdx];
	if (!toad)
		return;

	_draggedToadRunnerIdx = toadRunnerIdx;
	_highlightedEntryRow = -1;
	if (!_pageDragCursorHidden) {
		// The held toad becomes the visible pointer until the drag ends.
		_vm->_cursor->hideCursor();
		_pageDragCursorHidden = true;
	}

	// A held toad owns the top render slot until it is released.
	manualLinkAtEnd(toad);

	// Anchor the cursor to the center of the toad's current click rectangle.
	Common::Rect clickRect = toad->getClickRect();
	toad->setPointRef(Common::Point((clickRect.left + clickRect.right) / 2, (clickRect.top + clickRect.bottom) / 2));
	updateToadDrag(absPos);
}

void ZoombiniPuzzleLilly::updateToadDrag(const Common::Point &absPos) {
	if (_draggedToadRunnerIdx < 0 || !_runnerFeatures[_draggedToadRunnerIdx])
		return;

	ZmbFeature *toad = _runnerFeatures[_draggedToadRunnerIdx];
	Common::Point pos(CLIP<int16>(absPos.x, 0, 639), CLIP<int16>(absPos.y, 0, 479));

	addExternalDirtyRect(toad->getZSortRect());
	toad->setPointLoc(pos);
	toad->requestVisualRematerialization();

	updateEntryHighlight(pos, _draggedToadRunnerIdx);
}

void ZoombiniPuzzleLilly::endToadDrag(const Common::Point &absPos) {
	int16 toadRunnerIdx = _draggedToadRunnerIdx;
	_draggedToadRunnerIdx = -1;
	if (_pageDragCursorHidden) {
		// Restore the cursor after the held toad is released.
		_vm->_cursor->showCursor();
		_pageDragCursorHidden = false;
	}
	if (toadRunnerIdx < 0 || !_runnerFeatures[toadRunnerIdx])
		return;

	// Re-evaluate the click position itself.
	// Do not depend on a preceding mouse-move event.
	// Sticky mouse can deliver its second click at the destination.
	updateEntryHighlight(absPos, toadRunnerIdx);
	int16 row = _highlightedEntryRow;
	clearEntryHighlight();

	if (0 <= row && row < 12)
		placeToad(toadRunnerIdx, row);
	else
		cancelToadDrag(toadRunnerIdx);
}

void ZoombiniPuzzleLilly::updateEntryHighlight(const Common::Point &absPos, int16 toadRunnerIdx) {
	// Twelve static entry rectangles cover the selectable cells.
	// A valid cell is unoccupied and matches the dragged toad's tattoo family.
	ZmbLillyGridWalker &rs = _runnerStates[toadRunnerIdx];
	int16 hit = -1;
	for (int16 entryCellIdx = 0; entryCellIdx < 12; entryCellIdx++) {
		if (!_toadEntryCellRects[entryCellIdx].contains(absPos))
			continue;
		if (_cellOccupancy[entryCellIdx][0] != 0)
			continue;
		if (getGridAttrByType(rs.attrType, entryCellIdx, 0) != rs.attrValue)
			continue;
		hit = entryCellIdx;
		break;
	}

	if (hit == _highlightedEntryRow)
		return;

	// Invalidate the previous highlight area
	if (0 <= _highlightedEntryRow)
		addExternalDirtyRect(cursorHighlightRect(_highlightedEntryRow, 0));

	_highlightedEntryRow = hit;
	if (_entryHighlightFeature) {
		if (0 <= hit)
			_entryHighlightFeature->activateRender();
		else
			_entryHighlightFeature->deactivateRender();
	}
}

void ZoombiniPuzzleLilly::clearEntryHighlight() {
	if (0 <= _highlightedEntryRow)
		addExternalDirtyRect(cursorHighlightRect(_highlightedEntryRow, 0));
	_highlightedEntryRow = -1;
	if (_entryHighlightFeature)
		_entryHighlightFeature->deactivateRender();
}

void ZoombiniPuzzleLilly::placeToad(int16 toadRunnerIdx, int16 entryRow) {
	ZmbFeature *toad = _runnerFeatures[toadRunnerIdx];
	ZmbLillyGridWalker &rs = _runnerStates[toadRunnerIdx];
	if (!toad)
		return;

	addExternalDirtyRect(toad->getZSortRect());

	// Re-anchor the bank idle SCRB at the entry pad (POS_DELTA re-derives the
	// reference point from the SCRB's first hotspot on load).
	loadScrbOntoFeature(toad, kResScrb10043_EntryBase + rs.entrySlot);
	toad->setPointLoc(_padCellPositions[entryRow][0]);

	_cellOccupancy[entryRow][0] = 1;
	rs.occupiesGridCell = true;
	rs.gridCol = 0;
	rs.gridRow = entryRow;
	rs.direction = ZmbLillyGridWalker::kDirectionRight;
	rs.nextMoveReadyFrame = 0;
	rs.callbackMode = ZmbLillyGridWalker::kCallbackNone;
	initRunnerBFSPath(toadRunnerIdx);
	linkRunnerBeforePad(toadRunnerIdx, entryRow);

	// Child hookup: the next zoombini in line leaps onto the toad.
	// Events 2 and 3 advance the child's arrival and departure phases.
	int16 childRunnerIdx = _assignedSnoidCount;
	if (0 <= childRunnerIdx && childRunnerIdx < _pageLoadedZmbCount && _runnerFeatures[childRunnerIdx]) {
		ZmbLillyGridWalker &childRs = _runnerStates[childRunnerIdx];
		childRs.toadRunnerIdx = toadRunnerIdx;
		childRs.entryRow = entryRow;
		childRs.callbackMode = ZmbLillyGridWalker::kCallbackSnoid;
		rs.childRunnerIdx = childRunnerIdx;
		rs.riderShapeOffset = childRs.childShapeOffset;

		_runnerFeatures[childRunnerIdx]->setFlags(ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE);
		_runnerFeatures[childRunnerIdx]->activateRender();
		loadScrbOntoFeature(_runnerFeatures[childRunnerIdx], kResScrb10109_ChildEntryBase + childRs.packSnoidIdx);
		_assignedSnoidCount += 1;
	}

	if (_assignedSnoidCount == _pageLoadedZmbCount) {
		// All zoombinis placed
		if (_swapWandStage == 6)
			countMatchesAndPlaySound();
		_pendingChildWalkInCount = 1;
	}
}

void ZoombiniPuzzleLilly::cancelToadDrag(int16 toadRunnerIdx) {
	// Snap back to the bank slot.
	ZmbFeature *toad = _runnerFeatures[toadRunnerIdx];
	ZmbLillyGridWalker &rs = _runnerStates[toadRunnerIdx];
	if (!toad)
		return;

	addExternalDirtyRect(toad->getZSortRect());
	loadScrbOntoFeature(toad, kResScrb10043_EntryBase + rs.entrySlot);
	toad->setPointLoc(kEntryPositions[rs.entrySlot]);
}

void ZoombiniPuzzleLilly::beginToadSwapDrag(const Common::Point &absPos) {
	if (!_swapWandFeature)
		return;

	if (_vm->isVersionFamilyTlcV2()) {
		const int16 normalScrbId = static_cast<int16>(kResScrb10078_ToadBase + MIN<int16>(_swapWandStage, 6));
		if (_swapWandFeature->getScrbId() != normalScrbId)
			loadSwapWandScrb(normalScrbId);
	}

	// Preserve the positioned home hit area before the runner begins following the cursor.
	// Restoring it on cancel makes the wand immediately reusable without waiting for another
	// materialization pass to replace the cursor-position click rectangle.
	if (_vm->isVersionFamilyTlcV2())
		_swapWandHomeClickRect = _normalSwapWandHomeRect;
	else
		_swapWandHomeClickRect = _swapWandFeature->getClickRect();
	_swapWandDragging = true;
	if (!_pageDragCursorHidden) {
		// Hide the cursor while the wand runner follows the mouse.
		_vm->_cursor->hideCursor();
		_pageDragCursorHidden = true;
	}
	// Restart the cell-selection phase.
	_swapSelectionState = CellSelectionState::kSelectFirst04;

	addExternalDirtyRect(_swapWandFeature->getZSortRect());
	_swapWandFeature->activateRender();
	_swapWandFeature->setPointLoc(Common::Point(CLIP<int16>(absPos.x, 0, 639), CLIP<int16>(absPos.y, 0, 479)));
	_swapWandFeature->requestVisualRematerialization();
}

bool ZoombiniPuzzleLilly::isInsidePondArea(const Common::Point &pos) const {
	if (!_swapPondRect.contains(pos))
		return false;
	if (_leftBankExclusionRect.contains(pos)) {
		// Valid only below the bank slope: 539*|123-x|/100 <= y-62
		int32 dx = ABS<int32>(123 - pos.x);
		if (pos.y - 62 < 539 * dx / 100)
			return false;
	} else if (_rightBankExclusionRect.contains(pos)) {
		// Valid only left of the bank slope: 539*(x-509)/100 <= |y-430|
		int32 dy = ABS<int32>(pos.y - 430);
		if (dy < 539 * (pos.x - 509) / 100)
			return false;
	}
	return true;
}

void ZoombiniPuzzleLilly::handleToadPondClick(const Common::Point &absPos) {
	if (!isInsidePondArea(absPos)) {
		cancelToadSwapDrag();
		return;
	}

	if (6 <= _swapWandStage)
		return;

	if (_swapSelectionState == CellSelectionState::kSelectFirst04 || _swapSelectionState == CellSelectionState::kSelectSecond05)
		trySelectSwapCell(absPos);
}

bool ZoombiniPuzzleLilly::trySelectSwapCell(const Common::Point &absPos) {
	// The input position is the wand's rendered top-left.
	// Test its tip at (+27,+22) against every logical 36x30 cell rectangle with a 4-pixel inset.
	// Using the four variable pad-shape bounds would create dead gaps inside valid cells.
	const Common::Point cellPickPos = absPos + kToadCellPickOffset;
	int16 hitCol = -1, hitRow = -1;
	for (int16 row = 0; row < 12 && hitCol < 0; row++) {
		for (int16 col = 0; col < 12; col++) {
			Common::Rect insetRect = _padCellRects[row][col];
			insetRect.left += 4;
			insetRect.top += 4;
			insetRect.right -= 4;
			insetRect.bottom -= 4;
			if (insetRect.contains(cellPickPos) && _cellOccupancy[row][col] == 0) {
				hitCol = col;
				hitRow = row;
				break;
			}
		}
	}

	// A miss inside the pond keeps the wand modal; only the outer cancel area releases it.
	if (hitCol < 0)
		return false;

	if (_swapSelectionState == CellSelectionState::kSelectFirst04) {
		_firstSwapCellCol = hitCol;
		_firstSwapCellRow = hitRow;
		_vm->_sound->playSound(ZmbResource(ZmbResource::kPage, kResSound12000_SwapBase + _nextSwapSoundIdx));
		_nextSwapSoundIdx += 1;
		if (3 < _nextSwapSoundIdx)
			_nextSwapSoundIdx = 0;
		setRunnerClickRect(_firstSwapCellCol, _firstSwapCellRow, _firstSwapCellFeature);
		_swapSelectionState = CellSelectionState::kSelectSecond05;
		_toadPickupInputEnabled = false;
	} else if (_swapSelectionState == CellSelectionState::kSelectSecond05) {
		_secondSwapCellCol = hitCol;
		_secondSwapCellRow = hitRow;
		_vm->_sound->playSound(ZmbResource(ZmbResource::kPage, kResSound12000_SwapBase + _nextSwapSoundIdx));
		_nextSwapSoundIdx += 1;
		if (3 < _nextSwapSoundIdx)
			_nextSwapSoundIdx = 0;
		setRunnerClickRect(_secondSwapCellCol, _secondSwapCellRow, _secondSwapCellFeature);
		_swapSelectionState = CellSelectionState::kAnimating06;
		_selectedCellsSwapped = false;
		_swapBlinkIntervalCount = 0;

		// Unlock progression
		if ((_secondSwapCellCol != _firstSwapCellCol || _secondSwapCellRow != _firstSwapCellRow)) {
			_swapsTowardNextStage += 1;
			if (_swapsPerWandStage <= _swapsTowardNextStage && _swapWandStage < 6) {
				_swapWandStage += 1;
				_swapsTowardNextStage = 0;
				// Advance the wand to the next authored progression SCRB.
				if (_swapWandFeature)
					loadSwapWandScrb(kResScrb10078_ToadBase + _swapWandStage);
				if (_swapWandStage == 6 && _assignedSnoidCount == _pageLoadedZmbCount)
					countMatchesAndPlaySound();
			}
		}
	}
	return true;
}

void ZoombiniPuzzleLilly::cancelToadSwapDrag() {
	// Toad returns home; a half-finished selection resets its blink runner.
	if (!_swapWandDragging)
		return;
	_swapWandDragging = false;
	if (_pageDragCursorHidden) {
		// Restore the cursor after the modal loop.
		_vm->_cursor->showCursor();
		_pageDragCursorHidden = false;
	}

	if (_swapSelectionState == CellSelectionState::kSelectSecond05)
		initCellRunnerPosition(_firstSwapCellCol, _firstSwapCellRow, _firstSwapCellFeature);
	_toadPickupInputEnabled = true;

	if (_swapWandFeature) {
		addExternalDirtyRect(_swapWandFeature->getZSortRect());
		loadSwapWandScrb(kResScrb10078_ToadBase + MIN<int16>(_swapWandStage, 6));
		setToadHomePosition();
		if (!_swapWandHomeClickRect.isEmpty())
			_swapWandFeature->setClickRect(_swapWandHomeClickRect);
		// Rebuild the home bounds after the cursor-following frame. The click rectangle
		// is synchronized from this materialized frame on the following page tick.
		_swapWandFeature->requestVisualRematerialization();
	}
}

void ZoombiniPuzzleLilly::updateV2SwapWandHover() {
	if (!_vm->isVersionFamilyTlcV2() || !_swapWandFeature || _swapWandDragging)
		return;
	if (_swapWandFeature->hasFlag(ZmbFeature::FLAG_01000000_DEFER_RENDER))
		return;

	const int16 swapLevel = MIN<int16>(_swapWandStage, 6);
	const int16 normalScrbId = static_cast<int16>(kResScrb10078_ToadBase + swapLevel);
	const bool hovered = isSwapWandAtPoint(_swapWandHoverPos);
	const int16 targetScrbId = hovered ? static_cast<int16>(kResScrb10170_ToadHoverBase + swapLevel) : normalScrbId;
	if (_swapWandFeature->getScrbId() != targetScrbId) {
		loadSwapWandScrb(targetScrbId);
	}
}

bool ZoombiniPuzzleLilly::isSwapWandAtPoint(const Common::Point &pos) const {
	if (!_swapWandFeature)
		return false;
	if (_vm->isVersionFamilyTlcV2())
		return _normalSwapWandHomeRect.contains(pos);
	return _swapWandFeature->hasClickRect() && _swapWandFeature->getClickRect().contains(pos);
}

// =================================================================
// Process all puzzle queues once per frame.
// =================================================================

void ZoombiniPuzzleLilly::onEveryFrame() {
	if (!_pageActive)
		return;

	syncSwapWandClickRect();
	updateV2SwapWandHover();

	// The swap blink is a render-owned callback in the page resources.
	if (_swapSelectionState == CellSelectionState::kAnimating06)
		processCellSwapAnimation();

	// --- Dirty-rect upkeep for the custom-rendered overlays ---
	// Invalidate the cursor's cell area every visible frame so the blink animates.
	if (_entryHighlightFeature && _entryHighlightFeature->isRenderActivated() && 0 <= _highlightedEntryRow)
		addExternalDirtyRect(cursorHighlightRect(_highlightedEntryRow, 0));
	// Same for the swap blink runners.
	if (_firstSwapCellFeature && _firstSwapCellFeature->isRenderActivated()) {
		Common::Rect r = _padCellRects[_firstSwapCellRow][_firstSwapCellCol];
		addExternalDirtyRect(Common::Rect(r.left - 17, r.top - 14, r.right, r.bottom));
	}
	if (_secondSwapCellFeature && _secondSwapCellFeature->isRenderActivated()) {
		Common::Rect r = _padCellRects[_secondSwapCellRow][_secondSwapCellCol];
		addExternalDirtyRect(Common::Rect(r.left - 17, r.top - 14, r.right, r.bottom));
	}

	// Advance the blink phase.
	uint32 now = getCurrentFrameCounter();
	if (_firstSwapCellFeature && _firstSwapCellFeature->isRenderActivated() && _nextFirstSwapBlinkFrame <= now) {
		_nextFirstSwapBlinkFrame = now + 4;
		_firstSwapBlinkFrame += 1;
		if (1 < _firstSwapBlinkFrame)
			_firstSwapBlinkFrame = 0;
	}

	// Update the Go button state.
	setGoButtonsEnabled(_goButtonEnabled);
}

void ZoombiniPuzzleLilly::onPostRenderFrame() {
	if (!_pageActive)
		return;

	// Render first, then stop queue consumption while departure is pending.
	// Returning here stops new enter, rotate, exit, path, and crab SCRBs,
	// while the already loaded SCRB still advances in the following render and may deliver its terminal callback.
	// In particular, a far-bank toad freezes at the end of whichever authored
	// phase was active when Go was pressed; a queued return phase is not started.
	if (isDeparturePending())
		return;

	// Process animation queues in their strict dependency order.
	processEnterQueue();
	processExitQueue();
	processCompletedExitRunner();
	processRotateQueue();
	processCrossQueue();
	processCompletedCrossRunner();
	processDepartQueue();
	processArriveQueue();

	// Skip movement processing while a swap selection is pending.
	if (_toadPickupInputEnabled)
		processMovePhase();

	// Clean up freed runners.
	processFreedRunners();
}

// --- Queue processing ---

void ZoombiniPuzzleLilly::processEnterQueue() {
	// Process one far-side turn-in at a time.
	// SCRB 10057 moves the runner toward (599, 55), and event 44 queues its rotation.
	while (0 < _farBankTurnInQueueSize && _activeFarBankTurnInRunner < 0) {
		_farBankTurnInQueueSize -= 1;
		int16 runnerIdx = _farBankTurnInQueue[_farBankTurnInQueueSize];
		_activeFarBankTurnInRunner = runnerIdx;
		if (0 <= runnerIdx && runnerIdx < kMaxRunners && _runnerFeatures[runnerIdx]) {
			ZmbLillyGridWalker &rs = _runnerStates[runnerIdx];
			rs.callbackMode = ZmbLillyGridWalker::kCallbackEnter;
			rs.scriptTargetX = 599;
			rs.scriptTargetY = 55;
			rs.lastAnimPhase = ZmbLillyGridWalker::kNoAnimationPhase;
			loadScrbOntoFeature(_runnerFeatures[runnerIdx], kResScrb10057_Move);
		}
	}
}

void ZoombiniPuzzleLilly::processExitQueue() {
	// Process one return swim at a time.
	// SCRB 10058 targets the runner's bank slot, and event 49 marks the return as complete.
	while (0 < _bankReturnQueueSize && _activeBankReturnRunner < 0) {
		_bankReturnQueueSize -= 1;
		int16 runnerIdx = _bankReturnQueue[_bankReturnQueueSize];
		_activeBankReturnRunner = runnerIdx;
		if (0 <= runnerIdx && runnerIdx < kMaxRunners && _runnerFeatures[runnerIdx]) {
			ZmbLillyGridWalker &rs = _runnerStates[runnerIdx];
			rs.callbackMode = ZmbLillyGridWalker::kCallbackExit;
			rs.scriptTargetX = kEntryPositions[rs.entrySlot].x;
			rs.scriptTargetY = kEntryPositions[rs.entrySlot].y;
			rs.lastAnimPhase = ZmbLillyGridWalker::kNoAnimationPhase;
			loadScrbOntoFeature(_runnerFeatures[runnerIdx], kResScrb10058_MoveAlternate);
			_runnerFeatures[runnerIdx]->addFlag(ZmbFeature::FLAG_04000000_OVERLAY);
			// Place the returning toad at the tail of the global runner list.
			manualLinkAtEnd(_runnerFeatures[runnerIdx]);
		}
	}
}

void ZoombiniPuzzleLilly::processCompletedExitRunner() {
	// Snap the toad back onto its bank slot with the idle SCRB.
	if (_completedBankReturnRunner < 0)
		return;

	int16 runnerIdx = _completedBankReturnRunner;
	_completedBankReturnRunner = -1;
	_activeBankReturnRunner = -1;

	if (0 <= runnerIdx && runnerIdx < kMaxRunners && _runnerFeatures[runnerIdx]) {
		ZmbLillyGridWalker &rs = _runnerStates[runnerIdx];
		addExternalDirtyRect(_runnerFeatures[runnerIdx]->getZSortRect());
		loadScrbOntoFeature(_runnerFeatures[runnerIdx], kResScrb10043_EntryBase + rs.entrySlot);
		_runnerFeatures[runnerIdx]->setPointLoc(kEntryPositions[rs.entrySlot]);
	}
}

void ZoombiniPuzzleLilly::processRotateQueue() {
	// Processed ONLY while an enter runner is active.
	// Color-family tattoos use SCRBs 10061/10063/10065 and subtract 7 from the tattoo offset.
	// Other families use 10060/10062/10064 directly.
	// Event 60 queues the return swim.
	if (_farBankRotationQueueSize <= 0 || _activeFarBankTurnInRunner < 0)
		return;

	while (0 < _farBankRotationQueueSize && 0 <= _activeFarBankTurnInRunner) {
		_farBankRotationQueueSize -= 1;
		int16 runnerIdx = _farBankRotationQueue[_farBankRotationQueueSize];
		if (runnerIdx < 0 || kMaxRunners <= runnerIdx || !_runnerFeatures[runnerIdx])
			continue;

		ZmbLillyGridWalker &rs = _runnerStates[runnerIdx];
		int16 randVal = _vm->_rnd->getRandomNumber(0, 2);

		rs.callbackMode = ZmbLillyGridWalker::kCallbackRotate;
		if (rs.attrType == ZmbLillyGridWalker::kPadAttrColor)
			loadScrbOntoFeature(_runnerFeatures[runnerIdx], 2 * randVal + kResScrb10061_SwapAlternateBase);
		else
			loadScrbOntoFeature(_runnerFeatures[runnerIdx], 2 * randVal + kResScrb10060_SwapBase);
	}
}

void ZoombiniPuzzleLilly::processCrossQueue() {
	// Process one retirement crossing at a time.
	// SCRB 10059 moves the runner toward (484, 450), and event 54 retires it.
	while (0 < _retirementQueueSize && _activeRetirementRunner < 0) {
		_retirementQueueSize -= 1;
		int16 runnerIdx = _retirementQueue[_retirementQueueSize];
		_activeRetirementRunner = runnerIdx;
		if (0 <= runnerIdx && runnerIdx < kMaxRunners && _runnerFeatures[runnerIdx]) {
			ZmbLillyGridWalker &rs = _runnerStates[runnerIdx];
			rs.callbackMode = ZmbLillyGridWalker::kCallbackCross;
			rs.scriptTargetX = 484;
			rs.scriptTargetY = 450;
			rs.lastAnimPhase = ZmbLillyGridWalker::kNoAnimationPhase;
			loadScrbOntoFeature(_runnerFeatures[runnerIdx], kResScrb10059_MoveFinal);
		}
	}
}

void ZoombiniPuzzleLilly::processCompletedCrossRunner() {
	// The toad is retired for good. Remove the runner so its final
	// materialized departure frame cannot remain in the persistent composite.
	if (_completedRetirementRunner < 0)
		return;

	int16 runnerIdx = _completedRetirementRunner;
	_completedRetirementRunner = -1;
	_activeRetirementRunner = -1;

	if (0 <= runnerIdx && runnerIdx < kMaxRunners && _runnerFeatures[runnerIdx]) {
		_runnerStates[runnerIdx].toadRetired = true;
		unloadScrbFeature(_runnerFeatures[runnerIdx]);
		_runnerFeatures[runnerIdx] = nullptr;
	}
}

void ZoombiniPuzzleLilly::processDepartQueue() {
	// Child runners; SCRB 10141 + zoombini index.
	while (0 < _childDepartureQueueSize) {
		_childDepartureQueueSize -= 1;
		int16 runnerIdx = _childDepartureQueue[_childDepartureQueueSize];
		if (0 <= runnerIdx && runnerIdx < kMaxRunners && _runnerFeatures[runnerIdx]) {
			ZmbLillyGridWalker &rs = _runnerStates[runnerIdx];
			rs.callbackMode = ZmbLillyGridWalker::kCallbackSnoid;
			loadScrbOntoFeature(_runnerFeatures[runnerIdx], kResScrb10141_ExitBase + rs.packSnoidIdx);
			_runnerFeatures[runnerIdx]->addFlag(ZmbFeature::FLAG_04000000_OVERLAY);
		}
	}
}

void ZoombiniPuzzleLilly::processArriveQueue() {
	// Child runners; SCRB 10019 + entry row (the zoombini leaps from the staging cluster onto the toad).
	// Events 26/20 chain the toad into the grid loop.
	while (0 < _childMountQueueSize) {
		_childMountQueueSize -= 1;
		int16 runnerIdx = _childMountQueue[_childMountQueueSize];
		if (0 <= runnerIdx && runnerIdx < kMaxRunners && _runnerFeatures[runnerIdx]) {
			ZmbLillyGridWalker &rs = _runnerStates[runnerIdx];
			rs.callbackMode = ZmbLillyGridWalker::kCallbackPathBuild;
			loadScrbOntoFeature(_runnerFeatures[runnerIdx], kResScrb10019_RowBase + rs.entryRow);
		}
	}
}

bool ZoombiniPuzzleLilly::isMoveRunnerReady(int16 runnerIdx) const {
	if (runnerIdx < 0 || kMaxRunners <= runnerIdx || !_runnerFeatures[runnerIdx])
		return false;

	const ZmbFeature *feature = _runnerFeatures[runnerIdx];
	const ZmbLillyGridWalker &rs = _runnerStates[runnerIdx];
	const uint32 currentFrame = getCurrentFrameCounter();

	// A move requires the post-step dwell to expire.
	// The current PLAY_ONCE SCRB must also reach its end callback.
	return rs.nextMoveReadyFrame <= currentFrame && feature->hasAnimEndCallbackFired();
}

void ZoombiniPuzzleLilly::processMovePhase() {
	// Alternate between toad path steps and, on levels 3-4, crab spawning and movement.
	// Splitting these branches across frames preserves their pacing.

	if (_toadPathPhasePending) {
		// === BRANCH A: Toad path advancement ===
		_toadPathPhasePending = false;

		while (0 < _pendingToadStepReadyQueueSize) {
			if (_toadStepReadyQueueSize < kMaxMoveQueueSize) {
				_pendingToadStepReadyQueueSize -= 1;
				_toadStepReadyQueue[_toadStepReadyQueueSize] = _pendingToadStepReadyQueue[_pendingToadStepReadyQueueSize];
				_toadStepReadyQueueSize += 1;
			} else {
				_pendingToadStepReadyQueueSize -= 1;
			}
		}

		while (0 < _toadStepReadyQueueSize) {
			_toadStepReadyQueueSize -= 1;
			int16 runnerIdx = _toadStepReadyQueue[_toadStepReadyQueueSize];
			if (runnerIdx < 0 || kMaxRunners <= runnerIdx || !_runnerFeatures[runnerIdx])
				continue;

			ZmbLillyGridWalker &rs = _runnerStates[runnerIdx];

			if (!isMoveRunnerReady(runnerIdx)) {
				if (_pendingToadStepReadyQueueSize < kMaxMoveQueueSize) {
					_pendingToadStepReadyQueue[_pendingToadStepReadyQueueSize] = runnerIdx;
					_pendingToadStepReadyQueueSize += 1;
				}
				continue;
			}

			linkRunnerBeforePad(runnerIdx, rs.gridRow);
			int16 nextScrb = advancePathOnGrid(runnerIdx);

			if (nextScrb == kResScrb10031_Exit) {
				rs.callbackMode = ZmbLillyGridWalker::kCallbackReadyExit;
				rs.lastAnimPhase = ZmbLillyGridWalker::kNoAnimationPhase;
				loadScrbOntoFeature(_runnerFeatures[runnerIdx], static_cast<int16>(kResScrb10031_Exit + rs.gridRow));
			} else if (nextScrb != 0) {
				rs.callbackMode = ZmbLillyGridWalker::kCallbackReadyMove;
				rs.lastAnimPhase = ZmbLillyGridWalker::kNoAnimationPhase;
				loadScrbOntoFeature(_runnerFeatures[runnerIdx], nextScrb);
			} else {
				if (_pendingToadStepReadyQueueSize < kMaxMoveQueueSize) {
					_pendingToadStepReadyQueue[_pendingToadStepReadyQueueSize] = runnerIdx;
					_pendingToadStepReadyQueueSize += 1;
				}
			}
		}
		return;
	}

	// === BRANCH B: Crab spawning + crab movement ===
	_toadPathPhasePending = true;

	if (static_cast<int16>(_difficultyLevel) <= 2)
		return;

	if (_pendingChildWalkInCount <= 0) {
		// Re-compute BFS paths queued after grid changes
		while (0 < _toadPathInitQueueSize) {
			_toadPathInitQueueSize -= 1;
			int16 runnerIdx = _toadPathInitQueue[_toadPathInitQueueSize];
			if (0 <= runnerIdx && runnerIdx < kMaxRunners && _runnerFeatures[runnerIdx])
				initRunnerBFSPath(runnerIdx);
		}

		// Timer-based crab spawning (every 480 frames, max 20 active)
		if (_nextCrabSpawnFrame < getCurrentFrameCounter() && _activeCrabCount < 20)
			spawnCrab();
	}

	while (0 < _pendingToadPathStepQueueSize) {
		if (_toadPathStepQueueSize < kMaxMoveQueueSize) {
			_pendingToadPathStepQueueSize -= 1;
			_toadPathStepQueue[_toadPathStepQueueSize] = _pendingToadPathStepQueue[_pendingToadPathStepQueueSize];
			_toadPathStepQueueSize += 1;
		} else {
			_pendingToadPathStepQueueSize -= 1;
		}
	}

	while (0 < _toadPathStepQueueSize) {
		_toadPathStepQueueSize -= 1;
		int16 runnerIdx = _toadPathStepQueue[_toadPathStepQueueSize];
		if (runnerIdx < 0 || kMaxRunners <= runnerIdx || !_runnerFeatures[runnerIdx])
			continue;

		ZmbLillyGridWalker &rs = _runnerStates[runnerIdx];

		if (!isMoveRunnerReady(runnerIdx)) {
			if (_pendingToadPathStepQueueSize < kMaxMoveQueueSize) {
				_pendingToadPathStepQueue[_pendingToadPathStepQueueSize] = runnerIdx;
				_pendingToadPathStepQueueSize += 1;
			}
			continue;
		}

		int16 nextMoveScrbId;
		if (rs.traversalMode == ZmbLillyGridWalker::kTraversalReuse)
			nextMoveScrbId = advanceCrabForwardStep(runnerIdx);
		else
			nextMoveScrbId = advanceCrabPathStepAlt(runnerIdx);

		if (nextMoveScrbId == kResScrb10069_Departure) {
			linkRunnerBeforePad(runnerIdx, rs.gridRow);
			// Crab exits at the bottom edge
			rs.callbackMode = ZmbLillyGridWalker::kCallbackCrabMove;
			rs.lastAnimPhase = ZmbLillyGridWalker::kNoAnimationPhase;
			loadScrbOntoFeature(_runnerFeatures[runnerIdx], kResScrb10069_Departure);
			rs.occupiesGridCell = false;
		} else if (nextMoveScrbId == 0) {
			if (_pendingToadPathStepQueueSize < kMaxMoveQueueSize) {
				_pendingToadPathStepQueue[_pendingToadPathStepQueueSize] = runnerIdx;
				_pendingToadPathStepQueueSize += 1;
			}
		} else {
			// Use the next array entry at [row] only for a downward move.
			// This composites the moving crab with its destination row.
			const int16 zOrderRow = rs.gridRow + (rs.direction == ZmbLillyGridWalker::kDirectionDown ? 1 : 0);
			linkRunnerBeforePad(runnerIdx, zOrderRow);
			rs.callbackMode = ZmbLillyGridWalker::kCallbackCrabMove;
			rs.lastAnimPhase = ZmbLillyGridWalker::kNoAnimationPhase;
			loadScrbOntoFeature(_runnerFeatures[runnerIdx], nextMoveScrbId);
		}
	}
}

void ZoombiniPuzzleLilly::processFreedRunners() {
	while (0 < _runnerCleanupQueueSize) {
		_runnerCleanupQueueSize -= 1;
		int16 runnerIdx = _runnerCleanupQueue[_runnerCleanupQueueSize];
		if (0 <= runnerIdx && runnerIdx < kMaxRunners && _runnerFeatures[runnerIdx]) {
			// The slot is therefore available to a later crab;
			// merely hiding it eventually exhausts the 20-entry active-runner table and stops obstacle spawning.
			unloadScrbFeature(_runnerFeatures[runnerIdx]);
			_runnerFeatures[runnerIdx] = nullptr;
			_runnerStates[runnerIdx].clear();
		}
	}
}

void ZoombiniPuzzleLilly::processCellSwapAnimation() {
	// The first tick executes the swap.
	// Both cells then blink for @ref ZoombiniPuzzleLilly::_swapBlinkIntervalLimit intervals before returning to state 4.
	if (!_selectedCellsSwapped) {
		_selectedCellsSwapped = true;
		swapCellsAndUpdateRunners(_firstSwapCellCol, _firstSwapCellRow, _secondSwapCellCol, _secondSwapCellRow);
		// Repaint both cells with their swapped decorations
		addExternalDirtyRect(_padCellRects[_firstSwapCellRow][_firstSwapCellCol]);
		addExternalDirtyRect(_padCellRects[_secondSwapCellRow][_secondSwapCellCol]);
	}

	uint32 now = getCurrentFrameCounter();
	if (_swapBlinkIntervalCount < _swapBlinkIntervalLimit) {
		if (_nextSecondSwapBlinkFrame <= now) {
			_nextSecondSwapBlinkFrame = now + 4;
			_swapBlinkIntervalCount += 1;
			_secondSwapBlinkFrame += 1;
			if (1 < _secondSwapBlinkFrame)
				_secondSwapBlinkFrame = 0;
		}
	} else {
		// Animation done
		_swapBlinkIntervalCount = 0;
		_selectedCellsSwapped = false;
		_swapSelectionState = CellSelectionState::kSelectFirst04;
		if (_firstSwapCellFeature) {
			_firstSwapCellFeature->deactivateRender();
			Common::Rect r = _padCellRects[_firstSwapCellRow][_firstSwapCellCol];
			addExternalDirtyRect(Common::Rect(r.left - 17, r.top - 14, r.right, r.bottom));
		}
		if (_secondSwapCellFeature) {
			_secondSwapCellFeature->deactivateRender();
			Common::Rect r = _padCellRects[_secondSwapCellRow][_secondSwapCellCol];
			addExternalDirtyRect(Common::Rect(r.left - 17, r.top - 14, r.right, r.bottom));
		}
	}
}

// =================================================================
// Pathfinding
// =================================================================

byte ZoombiniPuzzleLilly::getGridAttrByType(ZmbLillyGridWalker::PadAttrType attrType, int16 row0, int16 col) const {
	switch (attrType) {
	case ZmbLillyGridWalker::kPadAttrPattern:
		return _padPattern[row0][col];
	case ZmbLillyGridWalker::kPadAttrShape:
		return _padShape[row0][col];
	case ZmbLillyGridWalker::kPadAttrColor:
		return _padColor[row0][col];
	default:
		return 0;
	}
}

bool ZoombiniPuzzleLilly::isAnswerEntryRowCrossable(ZmbLillyGridWalker::PadAttrType attrType, byte attrValue, int16 entryRow) const {
	if (entryRow < 0 || 12 <= entryRow)
		return false;
	if (getGridAttrByType(attrType, entryRow, 0) != attrValue)
		return false;

	bool reached[12][12];
	memset(reached, 0, sizeof(reached));
	int16 stackRows[144];
	int16 stackCols[144];
	int16 stackSize = 0;
	reached[entryRow][0] = true;
	stackRows[stackSize] = entryRow;
	stackCols[stackSize] = 0;
	stackSize += 1;

	static constexpr int16 kRowDeltas[4] = {-1, 0, 1, 0};
	static constexpr int16 kColDeltas[4] = {0, 1, 0, -1};
	while (0 < stackSize) {
		stackSize -= 1;
		const int16 row = stackRows[stackSize];
		const int16 col = stackCols[stackSize];
		if (col == 11)
			return true;

		for (int16 directionIdx = 0; directionIdx < 4; directionIdx++) {
			const int16 nextRow = row + kRowDeltas[directionIdx];
			const int16 nextCol = col + kColDeltas[directionIdx];
			if (nextRow < 0 || 12 <= nextRow || nextCol < 0 || 12 <= nextCol || reached[nextRow][nextCol])
				continue;
			if (getGridAttrByType(attrType, nextRow, nextCol) != attrValue)
				continue;

			reached[nextRow][nextCol] = true;
			if (stackSize < 144) {
				stackRows[stackSize] = nextRow;
				stackCols[stackSize] = nextCol;
				stackSize += 1;
			}
		}
	}

	return false;
}

int16 ZoombiniPuzzleLilly::advancePathOnGrid(int16 runnerIdx) {
	// One toad hop.
	// Scans four directions starting from the current direction and rotating clockwise.
	// Valid neighbor = unoccupied AND tattoo-family attribute matches.
	// Best = strictly lowest visit count below the current cell's count.
	// Step SCRBs are constants 10001-10004.
	if (runnerIdx < 0 || kMaxRunners <= runnerIdx)
		return 0;

	ZmbLillyGridWalker &rs = _runnerStates[runnerIdx];

	// Anti-loop: if current cell visit count >= 10000, reset all
	if (10000 <= rs.visitGrid[rs.gridRow][rs.gridCol]) {
		for (int16 gridRowIdx = 0; gridRowIdx < 12; gridRowIdx++)
			for (int16 gridColIdx = 0; gridColIdx < 12; gridColIdx++)
				rs.visitGrid[gridRowIdx][gridColIdx] = 0;
		rs.visitGrid[rs.gridRow][rs.gridCol] = 1;
	}

	int16 curVisit = rs.visitGrid[rs.gridRow][rs.gridCol];
	if (curVisit == 0) {
		curVisit = 1;
		rs.visitGrid[rs.gridRow][rs.gridCol] = 1;
	}
	const int16 originalVisit = curVisit;

	ZmbLillyGridWalker::Direction checkDirection = rs.direction;
	byte dirCount = 0;
	int16 bestDir = 5;
	bool exitFound = false;
	int16 bestVisit = curVisit;
	int16 bestCol = 0, bestRow = 0;

	while (dirCount < 4 && !exitFound) {
		const ZmbGridStep step = ZmbGridTraversal::computeStep(
			ZmbGridCell(rs.gridRow, rs.gridCol),
			static_cast<ZmbGridDirection>(checkDirection),
			ZmbGridBounds(0, 11, 0, 11));
		bool valid = step.inBounds;
		const int16 newCol = step.cell.col;
		const int16 newRow = step.cell.row;
		if (!valid && checkDirection == ZmbLillyGridWalker::kDirectionRight)
			exitFound = true;

		if (valid) {
			if (_cellOccupancy[newRow][newCol] != 0)
				valid = false;
			else if (rs.attrType != ZmbLillyGridWalker::kPadAttrNone &&
					 getGridAttrByType(rs.attrType, newRow, newCol) != rs.attrValue)
				valid = false;

			if (valid && rs.visitGrid[newRow][newCol] < bestVisit) {
				bestDir = checkDirection;
				bestVisit = rs.visitGrid[newRow][newCol];
				bestCol = newCol;
				bestRow = newRow;
			}
		}

		checkDirection = static_cast<ZmbLillyGridWalker::Direction>((static_cast<byte>(checkDirection) + 1) % ZmbLillyGridWalker::kDirectionCount);
		dirCount += 1;
	}

	// Exit attempt: gate on the CURRENT cell's exit reservation
	if (exitFound) {
		if (_exitCellReservations[rs.gridRow][rs.gridCol] != 0) {
			bestDir = 5;
		} else {
			_exitCellReservations[rs.gridRow][rs.gridCol] = 1;
			bestDir = 4;
		}
	}

	// Map directions 0-3 to their step SCRB IDs.
	static constexpr int16 kStepScrbTable[ZmbLillyGridWalker::kDirectionCount] = {
		kResScrb10001_StepDirection0, kResScrb10002_StepDirection1,
		kResScrb10003_StepDirection2, kResScrb10004_StepDirection3};

	switch (bestDir) {
	case ZmbLillyGridWalker::kDirectionUp:
	case ZmbLillyGridWalker::kDirectionRight:
	case ZmbLillyGridWalker::kDirectionDown:
	case ZmbLillyGridWalker::kDirectionLeft:
		rs.direction = static_cast<ZmbLillyGridWalker::Direction>(bestDir);
		rs.scrbKey = kStepScrbTable[bestDir];
		rs.visitGrid[bestRow][bestCol] = originalVisit + 1;
		_cellOccupancy[bestRow][bestCol] = 1;
		return kStepScrbTable[bestDir];
	case 4: // exit
		rs.direction = ZmbLillyGridWalker::kDirectionRight;
		rs.scrbKey = kResScrb10031_Exit;
		return kResScrb10031_Exit;
	default:
		return 0;
	}
}

void ZoombiniPuzzleLilly::computeShortestPath(int16 targetProgress, int16 runnerIdx) {
	// Greedy Dijkstra-style fill.
	if (runnerIdx < 0 || kMaxRunners <= runnerIdx)
		return;

	ZmbLillyGridWalker &rs = _runnerStates[runnerIdx];

	int16 maxProgress = (rs.progressAxis == ZmbLillyGridWalker::kProgressByRow) ? rs.gridRow : rs.gridCol;

	int16 curCol, curRow;
	if (rs.maxReachableProgress == 11) {
		curCol = rs.gridCol;
		curRow = rs.gridRow;
	} else {
		curCol = rs.frontierCol;
		curRow = rs.frontierRow;
	}

	int16 bestCol = curCol;
	int16 bestRow = curRow;

	rs.visitGrid[curRow][curCol] = rs.stepCount;
	int16 bestCost = rs.stepCount;
	int16 curCost = bestCost;
	ZmbLillyGridWalker::Direction currentDirection = rs.direction;

	for (int16 iter = 0; iter < 200; iter++) {
		if (targetProgress <= maxProgress)
			break;

		ZmbLillyGridWalker::Direction direction = currentDirection;
		for (uint16 snoidIdx = 0; snoidIdx < ZmbLillyGridWalker::kDirectionCount && maxProgress < targetProgress; snoidIdx++) {
			const ZmbGridStep step = ZmbGridTraversal::computeStep(ZmbGridCell(curRow, curCol), static_cast<ZmbGridDirection>(direction), ZmbGridBounds(0, 11, 0, 11));
			bool valid = step.inBounds;
			const int16 testCol = valid ? step.cell.col : curCol;
			const int16 testRow = valid ? step.cell.row : curRow;
			if (!valid &&
				((rs.progressAxis == ZmbLillyGridWalker::kProgressByColumn &&
				  direction == ZmbLillyGridWalker::kDirectionRight) ||
				 (rs.progressAxis == ZmbLillyGridWalker::kProgressByRow &&
				  direction == ZmbLillyGridWalker::kDirectionDown)))
				maxProgress = (rs.progressAxis == ZmbLillyGridWalker::kProgressByColumn) ? curCol : curRow;

			if (valid && rs.attrType != ZmbLillyGridWalker::kPadAttrNone &&
				getGridAttrByType(rs.attrType, testRow, testCol) != rs.attrValue)
				valid = false;

			if (valid && rs.visitGrid[testRow][testCol] < bestCost) {
				currentDirection = direction;
				bestCost = rs.visitGrid[testRow][testCol];
				bestCol = testCol;
				bestRow = testRow;

				if (rs.progressAxis == ZmbLillyGridWalker::kProgressByRow) {
					if (maxProgress < testRow)
						maxProgress = testRow;
					if (testRow < rs.frontierRow) {
						rs.frontierCol = testCol;
						rs.frontierRow = testRow;
					}
				} else {
					if (maxProgress < testCol)
						maxProgress = testCol;
					if (testCol < rs.frontierCol) {
						rs.frontierCol = testCol;
						rs.frontierRow = testRow;
					}
				}
			}

			direction = static_cast<ZmbLillyGridWalker::Direction>((static_cast<byte>(direction) + 1) % ZmbLillyGridWalker::kDirectionCount);
		}

		rs.visitGrid[bestRow][bestCol] = curCost + 1;
		curCol = bestCol;
		curRow = bestRow;
		curCost += 1;
		bestCost = curCost;
	}

	rs.stepCount = rs.visitGrid[curRow][curCol];
	rs.maxReachableProgress = maxProgress;
}

void ZoombiniPuzzleLilly::traversePathBFS(int16 targetProgress, int16 runnerIdx) {
	// Greedy downhill traversal that consumes (zeroes) the cost matrix along the best path.
	if (runnerIdx < 0 || kMaxRunners <= runnerIdx)
		return;

	ZmbLillyGridWalker &rs = _runnerStates[runnerIdx];

	int16 curCol, curRow;
	if (rs.progressAxis == ZmbLillyGridWalker::kProgressByRow) {
		rs.visitGrid[rs.gridRow][rs.gridCol] = 0;
		curCol = rs.gridCol;
		curRow = rs.gridRow;
	} else {
		rs.visitGrid[rs.frontierRow][rs.frontierCol] = 0;
		curCol = rs.frontierCol;
		curRow = rs.frontierRow;
	}

	int16 bestCol = curCol;
	int16 bestRow = curRow;
	int16 maxProgress = (rs.progressAxis == ZmbLillyGridWalker::kProgressByRow) ? rs.gridRow : rs.gridCol;
	int16 bestCost = 0;
	int16 iterCount = 0;

	while (maxProgress < targetProgress && iterCount < 200) {
		for (byte i = 0; i < ZmbLillyGridWalker::kDirectionCount; i++) {
			const ZmbGridStep step = ZmbGridTraversal::computeStep(ZmbGridCell(curRow, curCol), static_cast<ZmbGridDirection>(i), ZmbGridBounds(0, 11, 0, 11));
			const int16 testCol = step.inBounds ? step.cell.col : curCol;
			const int16 testRow = step.inBounds ? step.cell.row : curRow;
			const int16 neighborCost = step.inBounds ? rs.visitGrid[testRow][testCol] : 0;

			if (bestCost < neighborCost) {
				bestCost = neighborCost;
				bestCol = testCol;
				bestRow = testRow;

				if (rs.progressAxis == ZmbLillyGridWalker::kProgressByColumn) {
					if (maxProgress < testCol)
						maxProgress = testCol;
				} else {
					if (maxProgress < testRow)
						maxProgress = testRow;
				}
			}
		}

		rs.visitGrid[bestRow][bestCol] = 0;
		curCol = bestCol;
		curRow = bestRow;
		iterCount += 1;
	}
}

void ZoombiniPuzzleLilly::initRunnerBFSPath(int16 runnerIdx) {
	if (runnerIdx < 0 || kMaxRunners <= runnerIdx)
		return;

	ZmbLillyGridWalker &rs = _runnerStates[runnerIdx];

	memset(rs.visitGrid, 0, sizeof(rs.visitGrid));
	rs.frontierCol = rs.gridCol;
	rs.frontierRow = rs.gridRow;
	rs.maxReachableProgress = 11;
	rs.stepCount = 1;

	computeShortestPath(11, runnerIdx);
	computeShortestPath(11, runnerIdx);
	traversePathBFS(11, runnerIdx);

	rs.visitGrid[rs.gridRow][rs.gridCol] = rs.stepCount;
}

// =================================================================
// Crab obstacle logic (difficulty >= 3)
// =================================================================

void ZoombiniPuzzleLilly::spawnCrab() {
	if (_crabEntryCycleLength <= 0)
		return;

	int16 entryIdx = _nextCrabEntryIdx;
	int16 entryCol = _crabEntryColumns[entryIdx];

	// Cycle over the difficulty's entry column count regardless of outcome
	_nextCrabEntryIdx += 1;
	if (_crabEntryCycleLength <= _nextCrabEntryIdx)
		_nextCrabEntryIdx = 0;

	bool topOccupied = (_cellOccupancy[0][entryCol] != 0);
	bool reuseMode = false;
	if (topOccupied) {
		// Only spawn onto an occupied top cell when a fresh crab previously claimed it (trail marker == 1).
		if (_crabTrailState[0][entryCol] != 1)
			return;
		reuseMode = true;
	}

	int16 runnerIdx = -1;
	for (int16 runnerSlotIdx = 0; runnerSlotIdx < kMaxCrabs; runnerSlotIdx++) {
		if (!_runnerFeatures[kCrabBase + runnerSlotIdx]) {
			runnerIdx = kCrabBase + runnerSlotIdx;
			break;
		}
	}
	if (runnerIdx < 0)
		return;

	ZmbFeature::EventHooks hooks;
	hooks.setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniPuzzleLilly::crabPreRenderShape));
	// Preserve the otherwise-unused 4..7 random draw to keep the deterministic RNG sequence aligned.
	// The frame interval remains fixed at 8.
	_vm->_rnd->getRandomNumber(4, 7);
	ZmbFeature *crab = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape10000_FeatureOverlay), kResScrb10067_Crab, 8,
		ZmbFeature::FLAG_00000002_TYPE_GRIDWALKER | ZmbFeature::FLAG_00080000_DEFER_ANIM |
			ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_00800000_POS_DELTA,
		hooks);

	if (!crab)
		return;

	_runnerFeatures[runnerIdx] = crab;
	_activeCrabRunnerIndices[_activeCrabCount] = runnerIdx;
	_activeCrabCount += 1;

	ZmbLillyGridWalker &rs = _runnerStates[runnerIdx];
	rs.clear();
	rs.runnerKind = ZmbLillyGridWalker::kRunnerCrab;
	rs.progressAxis = ZmbLillyGridWalker::kProgressByRow;
	rs.occupiesGridCell = true;
	rs.entrySlot = entryIdx;
	rs.gridCol = entryCol;
	rs.gridRow = 0;
	rs.direction = ZmbLillyGridWalker::kDirectionDown;
	rs.visitGrid[0][entryCol] = 1;

	if (reuseMode) {
		// Ride the existing trail (no attrs, no BFS, occupancy already set).
		rs.traversalMode = ZmbLillyGridWalker::kTraversalReuse;
		rs.bfsReinitPending = false;
	} else {
		// Fresh crab claims the cell.
		rs.traversalMode = ZmbLillyGridWalker::kTraversalFresh;
		rs.bfsReinitPending = false;
		rs.attrType = _crabPathAttrType;
		rs.attrValue = getGridAttrByType(rs.attrType, 0, entryCol);
		int16 offset = kObstacleBFSOffset[rs.attrType];
		if (0 <= offset && offset < 13)
			rs.tattooIdx = rs.attrValue + kPatternAttrExtra[offset];
		initRunnerBFSPath(runnerIdx);
		_cellOccupancy[0][entryCol] = 1;
		_crabTrailState[0][entryCol] = 1;
	}

	// Place the crab 17 pixels above its selected top-row cell.
	int16 posX = 35 * entryCol + 2;
	if (1 < static_cast<int16>(_gridRowBaseX.size()))
		posX += _gridRowBaseX[1];
	int16 posY = kColYOffset[entryCol] - 17;
	if (1 < static_cast<int16>(_gridRowBaseY.size()))
		posY += _gridRowBaseY[1];

	// Write only the feature position before loading SCRB 10067.
	// Keep secondary position at the SCRB's first hotspot
	// so POS_DELTA translates the fixed entry animation onto the selected top-row cell.
	crab->setPointLoc(Common::Point(posX, posY));
	loadScrbOntoFeature(crab, kResScrb10067_Crab);
	Common::HashMap<int16, ZmbShapeOffsetRegs *>::const_iterator itRegs = _shapeOffsetRegsMap.find(kResRegs10000_FeatureOverlay);
	if (itRegs != _shapeOffsetRegsMap.end())
		crab->setShapeOffsetRegs(itRegs->_value);

	rs.callbackMode = ZmbLillyGridWalker::kCallbackCrabMove;
	rs.lastAnimPhase = ZmbLillyGridWalker::kNoAnimationPhase;
	linkRunnerBeforePad(runnerIdx, 0);

	_nextCrabSpawnFrame = getCurrentFrameCounter() + 480;
}

uint16 ZoombiniPuzzleLilly::advanceCrabForwardStep(int16 runnerIdx) {
	// Reuse-mode crab always tries to move one ROW down.
	if (runnerIdx < 0 || kMaxRunners <= runnerIdx)
		return 0;

	ZmbLillyGridWalker &rs = _runnerStates[runnerIdx];

	if (rs.bfsReinitPending) {
		if (_toadPathInitQueueSize < kMaxQueueSize) {
			_toadPathInitQueue[_toadPathInitQueueSize] = runnerIdx;
			_toadPathInitQueueSize += 1;
		}
		rs.traversalMode = ZmbLillyGridWalker::kTraversalFresh;
		rs.bfsReinitPending = false;
		return 0;
	}

	const int16 col = rs.gridCol;
	const ZmbGridStep step = ZmbGridTraversal::computeStep(ZmbGridCell(rs.gridRow, rs.gridCol), kZmbGridDown, ZmbGridBounds(0, 11, 0, 11));
	const int16 nextRow = step.inBounds ? step.cell.row : rs.gridRow;
	const bool atBoundary = !step.inBounds;

	if (!atBoundary) {
		if (_cellOccupancy[nextRow][col] != 0) {
			// Blocked only when no crab trail marker is present.
			if (_crabTrailState[nextRow][col] == 0)
				return 0;
		} else {
			// Adopt the pad's obstacle-family attribute and schedule a BFS
			// re-init (switches to fresh mode next step).
			rs.attrType = _crabPathAttrType;
			rs.attrValue = getGridAttrByType(rs.attrType, nextRow, col);
			int16 offset = kObstacleBFSOffset[rs.attrType];
			if (0 <= offset && offset < 13)
				rs.tattooIdx = rs.attrValue + kPatternAttrExtra[offset];
			rs.visitGrid[nextRow][col] = 1;
			rs.bfsReinitPending = true;
		}
	}

	if (atBoundary) {
		rs.scrbKey = kResScrb10069_Departure;
		return kResScrb10069_Departure;
	}

	_cellOccupancy[nextRow][col] = 1;
	rs.scrbKey = kResScrb10073_CrabDirection2;
	return kResScrb10073_CrabDirection2;
}

uint16 ZoombiniPuzzleLilly::advanceCrabPathStepAlt(int16 runnerIdx) {
	// A fresh-mode crab follows the BFS distance gradient.
	// The BFS arrays use one-based rows.
	if (runnerIdx < 0 || kMaxRunners <= runnerIdx)
		return 0;

	ZmbLillyGridWalker &rs = _runnerStates[runnerIdx];

	bool exitReached = false;
	int16 bestDir = 5;
	int16 markRow0 = 0;
	int16 markCol = 0;

	const int16 curBfsIdx = kBFSEntriesPerLayer * rs.attrValue + 13 * (rs.gridRow + 1) + rs.gridCol;

	for (int dir = 0; dir < 4 && !exitReached; dir++) {
		const ZmbGridStep step = ZmbGridTraversal::computeStep(ZmbGridCell(rs.gridRow + 1, rs.gridCol), static_cast<ZmbGridDirection>(dir), ZmbGridBounds(1, 12, 0, 11));
		if (!step.inBounds) {
			if (dir == kZmbGridDown)
				exitReached = true;
			continue;
		}

		const int16 testRow1 = step.cell.row;
		const int16 testCol = step.cell.col;
		const int16 testRow0 = testRow1 - 1;
		if (testRow0 < 0 || 12 <= testRow0 || testCol < 0 || 12 <= testCol)
			continue;
		if (_cellOccupancy[testRow0][testCol] != 0)
			continue;
		if (getGridAttrByType(rs.attrType, testRow0, testCol) != rs.attrValue)
			continue;

		int16 tgtBfsIdx = kBFSEntriesPerLayer * rs.attrValue + 13 * testRow1 + testCol;
		if (0 <= tgtBfsIdx && tgtBfsIdx < kMaxBFSEntries &&
			0 <= curBfsIdx && curBfsIdx < kMaxBFSEntries &&
			_bfsDistance[tgtBfsIdx] < _bfsDistance[curBfsIdx]) {
			bestDir = dir;
			markRow0 = testRow0;
			markCol = testCol;
			break;
		}
	}

	if (exitReached) {
		rs.scrbKey = kResScrb10069_Departure;
		return kResScrb10069_Departure;
	}

	if (4 <= bestDir)
		return 0;

	// Map directions 0-3 to their crab movement SCRB IDs.
	static constexpr int16 kCrabDirScrbTable[4] = {
		kResScrb10071_CrabDirection0, kResScrb10077_CrabDirection1,
		kResScrb10073_CrabDirection2, kResScrb10075_CrabDirection3};
	rs.direction = static_cast<ZmbLillyGridWalker::Direction>(bestDir);
	rs.scrbKey = kCrabDirScrbTable[bestDir];

	if (0 <= markRow0 && markRow0 < 12 && 0 <= markCol && markCol < 13)
		_cellOccupancy[markRow0][markCol] = 1;

	return kCrabDirScrbTable[bestDir];
}

// =================================================================
// BFS layer arrays (crab pathfinding)
// =================================================================

void ZoombiniPuzzleLilly::bfsExpandCell(int16 col, int16 row1, int16 attrValue,
										ZmbLillyGridWalker::PadAttrType attrType) {
	// Row1 is 1-based.
	int16 srcIdx = kBFSEntriesPerLayer * attrValue + 13 * row1 + col;
	if (srcIdx < 0 || kMaxBFSEntries <= srcIdx)
		return;
	if (!_bfsVisitState[srcIdx])
		return;

	for (int16 dir = 0; dir < 4; dir++) {
		const ZmbGridDirection direction = static_cast<ZmbGridDirection>(dir);
		const ZmbGridStep step = ZmbGridTraversal::computeStep(ZmbGridCell(row1, col), direction, ZmbGridBounds(1, 12, 0, 11));
		if (!step.inBounds)
			continue;

		const int16 nc = step.cell.col;
		const int16 nr = step.cell.row;
		const int16 recordDir = static_cast<int16>(ZmbGridTraversal::oppositeDirection(direction));
		const int16 nRow0 = nr - 1;
		if (nRow0 < 0 || 12 <= nRow0 || nc < 0 || 12 <= nc)
			continue;

		if (getGridAttrByType(attrType, nRow0, nc) != attrValue)
			continue;

		int16 nIdx = kBFSEntriesPerLayer * attrValue + 13 * nr + nc;
		if (nIdx < 0 || kMaxBFSEntries <= nIdx)
			continue;
		if (_bfsVisitState[nIdx])
			continue;

		if (_bfsQueueHead < kBFSQueueMax) {
			_bfsQueueCol[_bfsQueueHead] = nc;
			_bfsQueueRow[_bfsQueueHead] = nr;
			_bfsQueueHead += 1;
		}

		_bfsDirection[nIdx] = recordDir;
		_bfsDistance[nIdx] = _bfsDistance[srcIdx] + 1;
		_bfsVisitState[nIdx] = _bfsVisitState[srcIdx];
	}
}

void ZoombiniPuzzleLilly::initBFSGrid(int16 attrValue, ZmbLillyGridWalker::PadAttrType attrType) {
	// Seed every matching cell from row 12 down to row 1, expanding the queue per row.
	for (int16 row1 = 0; row1 < 13; row1++) {
		for (int16 col = 0; col < 12; col++) {
			int16 bfsEntryIdx = kBFSEntriesPerLayer * attrValue + 13 * row1 + col;
			if (0 <= bfsEntryIdx && bfsEntryIdx < kMaxBFSEntries) {
				_bfsVisitState[bfsEntryIdx] = 0;
				_bfsDirection[bfsEntryIdx] = 44;
				_bfsDistance[bfsEntryIdx] = 0;
			}
		}
	}

	_bfsQueueHead = 0;
	_bfsQueueTail = 0;
	memset(_bfsQueueCol, 0, sizeof(_bfsQueueCol));
	memset(_bfsQueueRow, 0, sizeof(_bfsQueueRow));

	for (int16 row1 = 12; 1 <= row1; row1--) {
		for (int16 col = 0; col < 12; col++) {
			int16 row0 = row1 - 1;

			if (getGridAttrByType(attrType, row0, col) != attrValue)
				continue;

			int16 bfsEntryIdx = kBFSEntriesPerLayer * attrValue + 13 * row1 + col;
			if (bfsEntryIdx < 0 || kMaxBFSEntries <= bfsEntryIdx)
				continue;
			if (_bfsVisitState[bfsEntryIdx])
				continue;

			if (_bfsQueueHead < kBFSQueueMax) {
				_bfsQueueCol[_bfsQueueHead] = col;
				_bfsQueueRow[_bfsQueueHead] = row1;
				_bfsQueueHead += 1;
			}

			_bfsDirection[bfsEntryIdx] = 2;
			_bfsDistance[bfsEntryIdx] += 1;
			_bfsVisitState[bfsEntryIdx] = row1;
		}

		for (int16 qi = _bfsQueueTail; qi < _bfsQueueHead && _bfsQueueTail < kBFSQueueMax && qi < kBFSQueueMax; qi++) {
			int16 qIdx = kBFSEntriesPerLayer * attrValue + 13 * _bfsQueueRow[qi] + _bfsQueueCol[qi];
			if (0 <= qIdx && qIdx < kMaxBFSEntries && _bfsVisitState[qIdx]) {
				bfsExpandCell(_bfsQueueCol[qi], _bfsQueueRow[qi], attrValue, attrType);
				_bfsQueueTail += 1;
			}
		}
	}
}

// =================================================================
// Animation event dispatch
// =================================================================

void ZoombiniPuzzleLilly::onFeatureAnimEvent(ZmbFeature *feature, int16 eventCode) {
	if (!feature)
		return;

	// --- Standard body arrangement events (240-253) ---
	if (kAnimEvent240_BodyArrangePendFirst <= eventCode &&
		eventCode <= kAnimEvent253_BodyArrangeDirectLast) {
		if (feature->hasFlag(ZmbFeature::FLAG_00000001_TYPE_SNOID)) {
			ZmbSnoid *snoid = static_cast<ZmbSnoid *>(feature);
			if (kAnimEvent250_BodyArrangeDirectFirst <= eventCode) {
				snoid->setTraitLayout(static_cast<ZmbScriptDecoder::TraitLayout>(eventCode - kAnimEvent250_BodyArrangeDirectFirst));
			}
		}
		return;
	}

	// --- Event 0: no-op ---
	// Lilly does not handle event 0.
	if (eventCode == kEntityBaseEventCode000_ToggleFacing)
		return;

	// --- Toad return and swap-script events ---
	if (feature == _toadIntroFeature || feature == _swapWandFeature) {
		handleScriptEvent(eventCode, feature);
		return;
	}

	// --- Find runner index from feature pointer ---
	int16 runnerIdx = -1;
	for (int16 runnerSlotIdx = 0; runnerSlotIdx < kMaxRunners; runnerSlotIdx++) {
		if (_runnerFeatures[runnerSlotIdx] == feature) {
			runnerIdx = runnerSlotIdx;
			break;
		}
	}
	if (runnerIdx < 0)
		return;

	ZmbLillyGridWalker &rs = _runnerStates[runnerIdx];

	switch (rs.callbackMode) {
	case ZmbLillyGridWalker::kCallbackEnter:
		if (eventCode == kEntityBaseEventCode044_FinishEntryInterpolation) {
			// The entry interpolation reached its destination marker.
			// Release the grid reservation and queue the runner for rotation.
			// The (599,55) interpolator has parked the toad at its target.
			feature->setPointLoc(Common::Point(rs.scriptTargetX, rs.scriptTargetY));
			feature->setPointRef(Common::Point(rs.scriptTargetX, rs.scriptTargetY));
			_exitCellReservations[rs.gridRow][rs.gridCol] = 0;
			if (_farBankRotationQueueSize < kMaxQueueSize) {
				_farBankRotationQueue[_farBankRotationQueueSize] = runnerIdx;
				_farBankRotationQueueSize += 1;
			}
		}
		break;
	case ZmbLillyGridWalker::kCallbackRotate:
		if (eventCode == kEntityBaseEventCode060_FinishRotation) {
			// The turn animation reached its hand-off marker.
			// Release the cell and queue this Toad for exit processing.
			snapPosLocToAnim(feature);
			_exitCellReservations[rs.gridRow][rs.gridCol] = 0;
			if (_bankReturnQueueSize < kMaxQueueSize) {
				_bankReturnQueue[_bankReturnQueueSize] = runnerIdx;
				_bankReturnQueueSize += 1;
			}
			_activeFarBankTurnInRunner = -1;
		}
		break;
	case ZmbLillyGridWalker::kCallbackExit:
		if (eventCode == kEntityBaseEventCode049_RestoreExitedToad) {
			// The exit SCRS returned the Toad to the bank.
			// Restore its clickable bank state and defer the queue completion to the frame loop.
			feature->setPointLoc(kEntryPositions[rs.entrySlot]);
			feature->setPointRef(kEntryPositions[rs.entrySlot]);
			// Toad becomes clickable on the bank again.
			feature->setFlags(
				ZmbFeature::FLAG_00000002_TYPE_GRIDWALKER |
				ZmbFeature::FLAG_00080000_DEFER_ANIM |
				ZmbFeature::FLAG_00100000_PLAY_ONCE |
				ZmbFeature::FLAG_00800000_POS_DELTA);
			rs.occupiesGridCell = false;
			_completedBankReturnRunner = runnerIdx;
		}
		break;
	case ZmbLillyGridWalker::kCallbackCross:
		if (eventCode == kEntityBaseEventCode054_FinishCrossing) {
			// The crossing SCRS reached the far-side marker.
			// Commit its destination position and free the former grid cell.
			feature->setPointLoc(Common::Point(rs.scriptTargetX, rs.scriptTargetY));
			feature->setPointRef(Common::Point(rs.scriptTargetX, rs.scriptTargetY));
			_exitCellReservations[rs.gridRow][rs.gridCol] = 0;
			_completedRetirementRunner = runnerIdx;
		}
		break;
	case ZmbLillyGridWalker::kCallbackSnoid:
		// Events 1, 2, 3
		handleChildEvent(eventCode, runnerIdx);
		break;
	case ZmbLillyGridWalker::kCallbackPathBuild:
		// Events 20, 26
		if (eventCode == kEntityBaseEventCode020_StartPathBuild) {
			// The idle bob script reached its path-ready marker.
			// Queue the associated Toad for path construction.
			// Push the associated TOAD to the ready queue (defensive: the
			// firing feature is the toad itself once SCRB 10000 is loaded).
			int16 toadRunnerIdx = (rs.runnerKind == ZmbLillyGridWalker::kRunnerToad) ? runnerIdx : rs.toadRunnerIdx;
			if (0 <= toadRunnerIdx && _toadStepReadyQueueSize < kMaxMoveQueueSize) {
				_toadStepReadyQueue[_toadStepReadyQueueSize] = toadRunnerIdx;
				_toadStepReadyQueueSize += 1;
			}
		} else if (eventCode == kEntityBaseEventCode026_FinishChildLeap &&
				   rs.runnerKind == ZmbLillyGridWalker::kRunnerChild) {
			// The child leap finished.
			// Replace the child callback with the Toad's idle bob script so path building can continue.
			// Child leap done: the toad takes over with the bobbing grid
			// idle SCRB 10000 (its event 20 starts the hop loop).
			int16 toadRunnerIdx = rs.toadRunnerIdx;
			if (0 <= toadRunnerIdx && _runnerFeatures[toadRunnerIdx]) {
				_runnerStates[toadRunnerIdx].callbackMode = ZmbLillyGridWalker::kCallbackPathBuild;
				loadScrbOntoFeature(_runnerFeatures[toadRunnerIdx], kResScrb10000_FeatureBase);
			}
		}
		break;
	case ZmbLillyGridWalker::kCallbackReadyMove:
		// Events 10-15
		handleMoveFinalizeStep(eventCode, runnerIdx);
		break;
	case ZmbLillyGridWalker::kCallbackReadyExit:
		if (eventCode == kEntityBaseEventCode030_ArriveAtFarNode)
			// The ready Toad reached the far-side node; enqueue the arrival handler.
			handleArriveAtNode(runnerIdx);
		break;
	case ZmbLillyGridWalker::kCallbackCrabMove:
		// Events 70, 80
		handleCrabArriveOrDepart(eventCode, runnerIdx);
		break;
	default:
		break;
	}
}

// =================================================================
// Event handlers
// =================================================================

void ZoombiniPuzzleLilly::handleChildEvent(int16 eventCode, int16 runnerIdx) {
	switch (eventCode) {
	case kSnoidMoveEventCode001_FinishWalkIn:
		// The child walk-in sequence completed its first callback.
		// Reduce the pending walk-in count and keep the swap hint visible briefly.
		_swapBlinkIntervalLimit = 5;
		_pendingChildWalkInCount -= 1;
		_pendingChildWalkInCount = MAX<int16>(_pendingChildWalkInCount, 0);
		break;
	case kSnoidMoveEventCode002_QueueArrival:
		// A child reached its arrival marker; queue it for the next frame's grid insertion.
		if (_childMountQueueSize < kMaxQueueSize) {
			_childMountQueue[_childMountQueueSize] = runnerIdx;
			_childMountQueueSize += 1;
		}
		break;
	case kSnoidMoveEventCode003_QueueDeparture:
		// A child reached its departure marker; queue it for removal from the grid.
		if (_childDepartureQueueSize < kMaxQueueSize) {
			_childDepartureQueue[_childDepartureQueueSize] = runnerIdx;
			_childDepartureQueueSize += 1;
		}
		break;
	default:
		break;
	}
}

void ZoombiniPuzzleLilly::handleCrabArriveOrDepart(int16 eventCode, int16 runnerIdx) {
	if (runnerIdx < 0 || kMaxRunners <= runnerIdx)
		return;

	ZmbLillyGridWalker &rs = _runnerStates[runnerIdx];

	if (eventCode == kEntityBaseEventCode070_FinishCrabCellMove) {
		// The crab settled on its next cell.
		// Re-anchor its rendered position and schedule the next BFS move.
		// Settled on a cell: re-anchor and continue moving
		if (_runnerFeatures[runnerIdx])
			snapPosLocToAnim(_runnerFeatures[runnerIdx]);
		if (_toadPathStepQueueSize < kMaxMoveQueueSize) {
			_toadPathStepQueue[_toadPathStepQueueSize] = runnerIdx;
			_toadPathStepQueueSize += 1;
		}
	} else if (eventCode == kEntityBaseEventCode080_FinishCrabExit) {
		// The crab left the bottom edge.
		// Clear its occupied cell and remove it from the active-crab list.
		// Exited at the bottom: clear cell, schedule free, remove from actives
		_cellOccupancy[rs.gridRow][rs.gridCol] = 0;

		if (_runnerCleanupQueueSize < kMaxRunners) {
			_runnerCleanupQueue[_runnerCleanupQueueSize] = runnerIdx;
			_runnerCleanupQueueSize += 1;
		}

		for (int16 crabIdx = 0; crabIdx < _activeCrabCount; crabIdx++) {
			if (_activeCrabRunnerIndices[crabIdx] == runnerIdx) {
				for (int16 shiftIdx = crabIdx; shiftIdx < _activeCrabCount - 1; shiftIdx++)
					_activeCrabRunnerIndices[shiftIdx] = _activeCrabRunnerIndices[shiftIdx + 1];
				_activeCrabCount -= 1;
				break;
			}
		}
	}
}

void ZoombiniPuzzleLilly::handleMoveFinalizeStep(int16 stepIdx, int16 runnerIdx) {
	// Toad hop events.
	if (runnerIdx < 0 || kMaxRunners <= runnerIdx || !_runnerFeatures[runnerIdx])
		return;

	ZmbLillyGridWalker &rs = _runnerStates[runnerIdx];
	ZmbFeature *feature = _runnerFeatures[runnerIdx];

	switch (stepIdx) {
	case kToadHopEventCode010_SnapToDestination:
		// Snap the Toad to the destination and begin the short dwell before the next hop.
		// Snap both POS_DELTA endpoints to the destination, then dwell.
		// Write the feature position and secondary position together.
		feature->setPointLoc(Common::Point(rs.moveTargetX, rs.moveTargetY));
		feature->setPointRef(Common::Point(rs.moveTargetX, rs.moveTargetY));
		if (_toadStepReadyQueueSize < kMaxMoveQueueSize) {
			_toadStepReadyQueue[_toadStepReadyQueueSize] = runnerIdx;
			_toadStepReadyQueueSize += 1;
		}
		rs.nextMoveReadyFrame = getCurrentFrameCounter() + 30;
		break;
	case kToadHopEventCode011_AdvanceCell: {
		// Advance the Toad's logical cell and calculate the next interpolation endpoints.
		// Save previous cell, apply direction, compute destination
		rs.prevCol = rs.gridCol;
		rs.prevRow = rs.gridRow;
		ZmbGridCell cell(rs.gridRow, rs.gridCol);
		ZmbGridTraversal::advanceCell(cell, static_cast<ZmbGridDirection>(rs.direction), ZmbGridBounds(0, 11, 0, 11));
		rs.gridRow = cell.row;
		rs.gridCol = cell.col;

		rs.moveTargetX = _padCellPositions[rs.gridRow][rs.gridCol].x;
		rs.moveTargetY = _padCellPositions[rs.gridRow][rs.gridCol].y;
		rs.moveStartX = feature->getPointLoc().x;
		rs.moveStartY = feature->getPointLoc().y;
		break;
	}
	case kToadHopEventCode012_ClearPreviousCell: {
		// Clear the cell vacated by the Toad before the midpoint frame is rendered.
		// Rewrite the current draw-record coordinates rather than feature position;
		// @ref ZoombiniPuzzleLilly::toadPreRenderShape() applies this frame-local midpoint correction.
		if (0 <= rs.prevRow && rs.prevRow < 12 && 0 <= rs.prevCol && rs.prevCol < 13)
			_cellOccupancy[rs.prevRow][rs.prevCol] = 0;
		break;
	}
	case kToadHopEventCode013_MidpointFrame:
	case kToadHopEventCode014_MidpointFrameAlternate:
		// Midpoint callbacks are consumed by the custom pre-renderer for frame-local hotspot snapping.
		// @ref ZoombiniPuzzleLilly::toadPreRenderShape() performs frame-local hotspot snapping.
		break;
	case kToadHopEventCode015_QueueNextStep:
		// The hop has no dwell; queue the next step immediately.
		// Immediate next step (no dwell)
		if (_toadStepReadyQueueSize < kMaxMoveQueueSize) {
			_toadStepReadyQueue[_toadStepReadyQueueSize] = runnerIdx;
			_toadStepReadyQueueSize += 1;
		}
		break;
	default:
		break;
	}
}

void ZoombiniPuzzleLilly::handleArriveAtNode(int16 runnerIdx) {
	// Event 30 means the toad reached the far side with its rider.
	if (runnerIdx < 0 || kMaxRunners <= runnerIdx || !_runnerFeatures[runnerIdx])
		return;

	ZmbLillyGridWalker &rs = _runnerStates[runnerIdx];
	ZmbFeature *feature = _runnerFeatures[runnerIdx];

	snapPosLocToAnim(feature);

	// First arrival enables the Go button
	_successfulToadArrivalCount += 1;
	if (_successfulToadArrivalCount == 1)
		_goButtonEnabled = true;

	rs.completedCrossingCount += 1;
	rs.maxReachableProgress = 0;

	if (rs.completedCrossingCount == 2) {
		if (_retirementQueueSize < kMaxQueueSize) {
			_retirementQueue[_retirementQueueSize] = runnerIdx;
			_retirementQueueSize += 1;
		}
	} else {
		if (_farBankTurnInQueueSize < kMaxQueueSize) {
			_farBankTurnInQueue[_farBankTurnInQueueSize] = runnerIdx;
			_farBankTurnInQueueSize += 1;
		}
	}

	_cellOccupancy[rs.gridRow][rs.gridCol] = 0;

	// The exit terminator retains the already transformed Toad frame.
	// Remove its rider record in place so the dismounting child is not drawn twice.
	const int16 toadScrbId = feature->getScrbId();
	if (kResScrb10031_Exit <= toadScrbId &&
		toadScrbId < kResScrb10043_EntryBase &&
		2 < feature->getPreparedRenderHotspots().size()) {
		Common::Array<ZmbPreparedRenderHotspot> preparedHotspots = feature->getPreparedRenderHotspots();
		preparedHotspots.remove_at(2);
		feature->setPreparedRenderHotspots(preparedHotspots);
	}

	// Rider dismounts: SCRB 10129 + row on the child runner
	int16 childRunnerIdx = rs.childRunnerIdx;
	if (0 <= childRunnerIdx && childRunnerIdx < kMaxRunners && _runnerFeatures[childRunnerIdx]) {
		ZmbLillyGridWalker &childRs = _runnerStates[childRunnerIdx];
		childRs.callbackMode = ZmbLillyGridWalker::kCallbackSnoid;
		// Keep the child's initial runner-list position behind the far-bank foliage.
		_runnerFeatures[childRunnerIdx]->activateRender();
		loadScrbOntoFeature(_runnerFeatures[childRunnerIdx], kResScrb10129_GridRowBase + rs.gridRow);

		if (0 <= childRs.packSnoidIdx && childRs.packSnoidIdx < static_cast<int16>(_packSnoids.size()) &&
			_packSnoids[childRs.packSnoidIdx]) {
			_packSnoids[childRs.packSnoidIdx]->_packIsOccupied = true;
			_completedSnoidCount += 1;
		}
	}
	// The rider no longer owns a Toad hotspot after dismounting. A later
	// riderless script must not reinterpret an authored effect layer as the
	// former rider slot. The next placement installs the new rider offset.
	rs.childRunnerIdx = -1;
	rs.riderShapeOffset = 0;

	// Celebratory sound
	if (_completedSnoidCount == _pageLoadedZmbCount)
		queueCompletionNarratorSound();
}

void ZoombiniPuzzleLilly::handleScriptEvent(int16 eventId, ZmbFeature *eventFeature) {
	switch (eventId) {
	case kToadReturnEventCode003_StartFinalChildWalkIn:
		// The toad script requests the final child walk-in batch.
		// Walk in the last 2 zoombini children (SCRB 10089+i)
		if (kPuzzleLevel2 <= _difficultyLevel) {
			for (int16 i = MAX<int16>(0, _pageLoadedZmbCount - 2); i < _pageLoadedZmbCount; i++) {
				if (_runnerFeatures[i]) {
					_runnerFeatures[i]->activateRender();
					loadScrbOntoFeature(_runnerFeatures[i], kResScrb10089_ChildBase + i);
					_runnerStates[i].callbackMode = ZmbLillyGridWalker::kCallbackSnoid;
				}
			}
		}
		break;
	case kToadReturnEventCode004_FinishToadIntro:
		// The toad intro ended; expose the draggable toad and initialize the higher-level crab grids.
		// The toad intro has ended.
		// Free the intro SCRB and activate the draggable toad.
		if (eventFeature && eventFeature == _toadIntroFeature) {
			// Close it after the current post-render.
			// @ref ZoombiniPage::checkCloseFeatures() can then invalidate the final materialized frame,
			// instead of only the previous frame's bounds.
			eventFeature->scheduleClose();
			_toadIntroFeature = nullptr;
		}
		if (_swapWandFeature) {
			// Activate the draggable toad with GRIDWALKER, DEFER_ANIM, PLAY_ONCE, and POS_DELTA.
			// OVERLAY is dropped and POS_DELTA makes the raw authored hotspot follow pointLoc.
			// There is no DEFER_RENDER, so the frozen PLAY_ONCE frame stays visible and clickable.
			_swapWandFeature->setFlags(ZmbFeature::FLAG_00000002_TYPE_GRIDWALKER | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_00800000_POS_DELTA);
			setToadHomePosition();
			_swapWandFeature->activateRender();
			_swapWandFeature->setNeedsRedraw(true);
		}
		// Initialize the crab BFS layers (one per family value)
		if (kPuzzleLevel3 <= _difficultyLevel) {
			for (int16 bfsLayerIdx = 0; bfsLayerIdx < _obstacleGridFamily; bfsLayerIdx++)
				initBFSGrid(bfsLayerIdx, _crabPathAttrType);
		}
		break;
	case kToadReturnEventCode005_SelectNextSwapPair:
		// The intro SCRB emits four callbacks to stage two preset swaps before wand activation.
		if (_swapSelectionState == CellSelectionState::kSelectFirst04) {
			_firstSwapCellCol = kSwapPairCol[2 * _introPresetSwapCellIdx];
			_firstSwapCellRow = kSwapPairRow[2 * _introPresetSwapCellIdx];
			setRunnerClickRect(_firstSwapCellCol, _firstSwapCellRow, _firstSwapCellFeature);
			_swapSelectionState = CellSelectionState::kSelectSecond05;
			_introPresetSwapCellIdx += 1;
		} else if (_swapSelectionState == CellSelectionState::kSelectSecond05) {
			_secondSwapCellCol = kSwapPairCol[2 * _introPresetSwapCellIdx];
			_secondSwapCellRow = kSwapPairRow[2 * _introPresetSwapCellIdx];
			setRunnerClickRect(_secondSwapCellCol, _secondSwapCellRow, _secondSwapCellFeature);
			_swapSelectionState = CellSelectionState::kAnimating06;
			_selectedCellsSwapped = false;
			_swapBlinkIntervalCount = 0;
			_introPresetSwapCellIdx += 1;
		}
		break;
	default:
		break;
	}
}

// =================================================================
// Helpers
// =================================================================

void ZoombiniPuzzleLilly::snapPosLocToAnim(ZmbFeature *feature) {
	// The REGS correction is applied by the engine at blit time,
	// so the raw hotspot position plus the current POS_DELTA is the on-screen anchor.
	if (!feature)
		return;

	ZmbHotspotGroup *hsGroup = feature->getHotspotGroup(feature->getLastFrameIdx());
	if (!hsGroup)
		return;

	Common::Array<ZmbHotspot> hotspots = hsGroup->copyHotspots();
	if (hotspots.empty() || hotspots[0]._shapeIdx == ZmbHotspot::kShapeNone)
		return;

	Common::Point p(hotspots[0]._x, hotspots[0]._y);
	p += feature->getPosDelta();
	feature->setPointLoc(p);
	feature->setPointRef(p);
}

void ZoombiniPuzzleLilly::loadSwapWandScrb(int16 scrbId) {
	if (!_swapWandFeature)
		return;

	loadScrbOntoFeature(_swapWandFeature, scrbId);
	_swapWandFeature->setShapeOffsetRegs(nullptr);
}

void ZoombiniPuzzleLilly::setToadHomePosition() {
	if (!_swapWandFeature)
		return;

	const ZmbDecodedScriptFrame *firstFrame = _swapWandFeature->getDecodedScriptFrame(0);
	if (!firstFrame || firstFrame->hotspots.empty())
		return;

	const ZmbHotspot &firstHotspot = firstFrame->hotspots[0];
	_swapWandFeature->setPointRef(firstHotspot.getPos());
	_swapWandFeature->setPointLoc(kToadHomePosition);
}

void ZoombiniPuzzleLilly::countMatchesAndPlaySound() {
	// Iterate over all 12 toads.
	// A placed toad with a full BFS path (maxReachableProgress == 11) counts as a match.
	// Its rider's pack Snoid is already marked.
	int16 matchCount = _completedSnoidCount;

	for (int16 toadRunnerIdx = 0; toadRunnerIdx < kToadCount; toadRunnerIdx++) {
		int16 runnerIdx = kToadBase + toadRunnerIdx;
		ZmbLillyGridWalker &rs = _runnerStates[runnerIdx];
		if (rs.toadRetired || !rs.occupiesGridCell)
			continue;
		if (rs.maxReachableProgress == 11) {
			matchCount += 1;
			int16 childRunnerIdx = rs.childRunnerIdx;
			if (0 <= childRunnerIdx && childRunnerIdx < kMaxRunners) {
				int16 packSnoidIdx = _runnerStates[childRunnerIdx].packSnoidIdx;
				if (0 <= packSnoidIdx && packSnoidIdx < static_cast<int16>(_packSnoids.size()) &&
					_packSnoids[packSnoidIdx])
					_packSnoids[packSnoidIdx]->_packIsOccupied = true;
			}
		}
	}

	if (matchCount < _pageLoadedZmbCount && passesPartialResultFeedbackGate())
		queuePartialSuccessNarratorSound();
}

void ZoombiniPuzzleLilly::syncSwapWandClickRect() {
	// The original recomputes clickRect from the positioned shapes on every pre-render, so the
	// swap wand's hit area always tracks whatever it currently draws. This port keeps the
	// shape-derived bounds in the sort rect and leaves clickRect at the value the SCRB parse
	// produced, which for the wand is the raw resource-space rectangle near (316, 257) rather than
	// its (38, 415) home. Mirror the original by following the positioned bounds once the wand is
	// engaged, which also keeps the hit area attached while it follows the cursor during a swap.
	if (!_swapWandFeature || !_swapWandFeature->isRenderActivated())
		return;

	const Common::Rect &positioned = _swapWandFeature->getZSortRect();
	if (positioned.isEmpty() || positioned == _swapWandFeature->getClickRect())
		return;

	_swapWandFeature->setClickRect(positioned);
}

void ZoombiniPuzzleLilly::setRunnerClickRect(int16 col, int16 row, ZmbFeature *feature) {
	// Give the cell a 20-by-15 click area and activate its renderer.
	if (!feature)
		return;
	if (col < 0 || 12 <= col || row < 0 || 12 <= row)
		return;

	Common::Point cellPos = _padCellPositions[row][col];
	feature->setClickRect(Common::Rect(cellPos.x, cellPos.y, cellPos.x + 20, cellPos.y + 15));
	feature->activateRender();
	// Immediately merge this cell's visual rectangle into the active dirty map.
	// The custom renderer needs an explicit redraw on its first visible frame.
	// Otherwise, the state advances to pick B without guaranteed pick-A feedback.
	addExternalDirtyRect(cursorHighlightRect(row, col));
	feature->setNeedsRedraw(true);
}

void ZoombiniPuzzleLilly::initCellRunnerPosition(int16 col, int16 row, ZmbFeature *feature) {
	// Deactivate the renderer and invalidate its previous visual coverage.
	if (!feature)
		return;

	feature->deactivateRender();
	if (0 <= col && col < 12 && 0 <= row && row < 12) {
		Common::Rect r = _padCellRects[row][col];
		addExternalDirtyRect(Common::Rect(r.left - 17, r.top - 14, r.right, r.bottom));
	}
}

void ZoombiniPuzzleLilly::swapCellsAndUpdateRunners(int16 colA, int16 rowA, int16 colB, int16 rowB) {
	// Swap the three attributes and the combined decoration value.
	// Keep occupancy and cell rectangles attached to their grid coordinates.
	if (colA < 0 || 12 <= colA || rowA < 0 || 12 <= rowA ||
		colB < 0 || 12 <= colB || rowB < 0 || 12 <= rowB)
		return;

	SWAP(_padPattern[rowA][colA], _padPattern[rowB][colB]);
	SWAP(_padShape[rowA][colA], _padShape[rowB][colB]);
	SWAP(_padColor[rowA][colA], _padColor[rowB][colB]);
	SWAP(_padDecorationShape[rowA][colA], _padDecorationShape[rowB][colB]);

	// Toads: clear the swapped cells' visit costs; re-path when the tattoo
	// family value at either cell matches.
	for (int16 toadRunnerIdx = 0; toadRunnerIdx < kToadCount; toadRunnerIdx++) {
		int16 runnerIdx = kToadBase + toadRunnerIdx;
		ZmbLillyGridWalker &rs = _runnerStates[runnerIdx];
		if (rs.toadRetired || !rs.occupiesGridCell || !_runnerFeatures[runnerIdx])
			continue;

		rs.visitGrid[rowA][colA] = 0;
		rs.visitGrid[rowB][colB] = 0;

		if (rs.attrType != ZmbLillyGridWalker::kPadAttrNone) {
			byte attrAtA = getGridAttrByType(rs.attrType, rowA, colA);
			byte attrAtB = getGridAttrByType(rs.attrType, rowB, colB);
			if (attrAtA == rs.attrValue || attrAtB == rs.attrValue)
				initRunnerBFSPath(runnerIdx);
		}
	}

	// Crabs: same, plus record which BFS layers need re-initialization.
	ZmbLillyGridWalker::PadAttrType affectedTypes[kMaxBFSLayers] = {};
	int16 affectedValues[kMaxBFSLayers] = {};

	for (int16 crabIdx = 0; crabIdx < _activeCrabCount; crabIdx++) {
		int16 runnerIdx = _activeCrabRunnerIndices[crabIdx];
		if (runnerIdx < 0 || kMaxRunners <= runnerIdx || !_runnerFeatures[runnerIdx])
			continue;
		ZmbLillyGridWalker &rs = _runnerStates[runnerIdx];
		if (!rs.occupiesGridCell)
			continue;

		rs.visitGrid[rowA][colA] = 0;
		rs.visitGrid[rowB][colB] = 0;

		if (rs.attrType != ZmbLillyGridWalker::kPadAttrNone) {
			byte attrAtA = getGridAttrByType(rs.attrType, rowA, colA);
			byte attrAtB = getGridAttrByType(rs.attrType, rowB, colB);
			if (attrAtA == rs.attrValue || attrAtB == rs.attrValue) {
				initRunnerBFSPath(runnerIdx);
				if (rs.attrValue < kMaxBFSLayers) {
					affectedTypes[rs.attrValue] = rs.attrType;
					affectedValues[rs.attrValue] = rs.attrValue;
				}
			}
		}
	}

	for (int16 layerIdx = 0; layerIdx < _obstacleGridFamily && layerIdx < kMaxBFSLayers; layerIdx++) {
		if (affectedTypes[layerIdx] != ZmbLillyGridWalker::kPadAttrNone)
			initBFSGrid(affectedValues[layerIdx], affectedTypes[layerIdx]);
	}

	_toadPickupInputEnabled = true;
}

Common::Rect ZoombiniPuzzleLilly::cursorHighlightRect(int16 row, int16 col) const {
	// (cellX-18, cellY-15, right-17, bottom-14)
	const Common::Rect &r = _padCellRects[row][col];
	return Common::Rect(r.left - 18, r.top - 15, r.right - 17, r.bottom - 14);
}

void ZoombiniPuzzleLilly::linkRunnerBeforePad(int16 runnerIdx, int16 row) {
	if (runnerIdx < 0 || kMaxRunners <= runnerIdx || !_runnerFeatures[runnerIdx])
		return;
	if (row < 0 || 12 <= row || !_padRowAnchorFeatures[row])
		return;

	manualLinkBefore(_runnerFeatures[runnerIdx], _padRowAnchorFeatures[row]);
}

// =================================================================
// Custom render callbacks
// =================================================================

bool ZoombiniPuzzleLilly::padAnchorPreRender(ZmbFeature *feature) {
	(void)feature;

	return false;
}

ZmbRenderResult ZoombiniPuzzleLilly::renderPadAnchor(ZmbFeature *feature) {
	(void)feature;

	return ZmbRenderResult::kSkipped;
}

ZmbRenderResult ZoombiniPuzzleLilly::renderGridSprites(ZmbFeature *feature) {
	(void)feature;

	// Visit all 144 pads each frame, but draw only through the active render clip.
	// The secondary attribute selects the pad outline and the combined attribute selects its decoration;
	// each applies the matching REGS offset.
	ZmbResource shapeRes(ZmbResource::kPage, kResBitmapShape13000_Detail);

	for (int16 row = 0; row < 12; row++) {
		for (int16 col = 0; col < 12; col++) {
			int16 posX = _padCellPositions[row][col].x;
			int16 posY = _padCellPositions[row][col].y;

			int16 padShapeIdx = _padShape[row][col] + 1;
			if (0 < padShapeIdx && padShapeIdx < 22) {
				int16 regsX = (padShapeIdx < static_cast<int16>(_padShapeOffsetX.size())) ? _padShapeOffsetX[padShapeIdx] : 0;
				int16 regsY = (padShapeIdx < static_cast<int16>(_padShapeOffsetY.size())) ? _padShapeOffsetY[padShapeIdx] : 0;
				_vm->_gfx->drawShape(ZoombiniGraphics::kShapeScreen, shapeRes, static_cast<uint16>(padShapeIdx), Common::Point(posX - regsX, posY - regsY));
			}

			int16 decoShapeIdx = _padDecorationShape[row][col];
			if (0 < decoShapeIdx && decoShapeIdx < 22) {
				int16 regsX = (decoShapeIdx < static_cast<int16>(_padShapeOffsetX.size())) ? _padShapeOffsetX[decoShapeIdx] : 0;
				int16 regsY = (decoShapeIdx < static_cast<int16>(_padShapeOffsetY.size())) ? _padShapeOffsetY[decoShapeIdx] : 0;
				_vm->_gfx->drawShape(ZoombiniGraphics::kShapeScreen, shapeRes, static_cast<uint16>(decoShapeIdx), Common::Point(posX - regsX, posY - regsY));
			}
		}
	}

	return ZmbRenderResult::kRendered;
}

ZmbRenderResult ZoombiniPuzzleLilly::renderCursorIndicator(ZmbFeature *feature) {
	// Draw a blinking highlight over the hovered entry cell in column 0.
	// The first layer selects a blink frame from the cell's secondary attribute;
	// the second draws its combined-attribute shape without blinking.
	// Both layers apply their corresponding REGS offsets.
	if (!feature->isRenderActivated())
		return ZmbRenderResult::kSkipped;

	if (_highlightedEntryRow < 0 || 12 <= _highlightedEntryRow)
		return ZmbRenderResult::kSkipped;

	const int16 row = _highlightedEntryRow;
	const int16 col = 0;
	ZmbResource shapeRes(ZmbResource::kPage, kResBitmapShape13000_Detail);
	int16 posX = _padCellPositions[row][col].x;
	int16 posY = _padCellPositions[row][col].y;

	byte attr2Val = _padShape[row][col];
	if (attr2Val < 4) {
		int16 shapeIdx = kCursorShapeBase[attr2Val] + kCursorBlinkFrames[_cursorBlinkFrame];
		if (0 < shapeIdx && shapeIdx < 36) {
			int16 regsX = (attr2Val + 1 < static_cast<int16>(_padShapeOffsetX.size())) ? _padShapeOffsetX[attr2Val + 1] : 0;
			int16 regsY = (attr2Val + 1 < static_cast<int16>(_padShapeOffsetY.size())) ? _padShapeOffsetY[attr2Val + 1] : 0;
			_vm->_gfx->drawShape(ZoombiniGraphics::kShapeScreen, shapeRes, static_cast<uint16>(shapeIdx), Common::Point(posX - regsX, posY - regsY));
		}
	}

	int16 combinedVal = _padDecorationShape[row][col];
	if (0 < combinedVal && combinedVal < 36) {
		int16 regsX = (combinedVal < static_cast<int16>(_padShapeOffsetX.size())) ? _padShapeOffsetX[combinedVal] : 0;
		int16 regsY = (combinedVal < static_cast<int16>(_padShapeOffsetY.size())) ? _padShapeOffsetY[combinedVal] : 0;
		_vm->_gfx->drawShape(ZoombiniGraphics::kShapeScreen, shapeRes, static_cast<uint16>(combinedVal), Common::Point(posX - regsX, posY - regsY));
	}

	// Advance the 4-step blink cycle on the feature's interval (5 frames)
	uint32 curFrame = getCurrentFrameCounter();
	if (_cursorBlinkTimer <= curFrame) {
		_cursorBlinkTimer = curFrame + 5;
		_cursorBlinkFrame += 1;
		if (3 < _cursorBlinkFrame)
			_cursorBlinkFrame = 0;
	}

	return ZmbRenderResult::kRendered;
}

void ZoombiniPuzzleLilly::renderCellBlink(int16 blinkPhase, int16 col, int16 row) {
	// Blink the secondary-attribute layer while keeping the combined-attribute layer steady.
	// Both layers apply their corresponding REGS offsets.
	if (col < 0 || 12 <= col || row < 0 || 12 <= row)
		return;

	ZmbResource shapeRes(ZmbResource::kPage, kResBitmapShape13000_Detail);
	int16 posX = _padCellPositions[row][col].x;
	int16 posY = _padCellPositions[row][col].y;

	byte attr2Val = _padShape[row][col];
	if (attr2Val < 4) {
		int16 shapeIdx = blinkPhase + kCellBlinkShapeBase[attr2Val];
		if (0 < shapeIdx && shapeIdx < 36) {
			int16 regsX = (attr2Val + 1 < static_cast<int16>(_padShapeOffsetX.size())) ? _padShapeOffsetX[attr2Val + 1] : 0;
			int16 regsY = (attr2Val + 1 < static_cast<int16>(_padShapeOffsetY.size())) ? _padShapeOffsetY[attr2Val + 1] : 0;
			_vm->_gfx->drawShape(ZoombiniGraphics::kShapeScreen, shapeRes, static_cast<uint16>(shapeIdx), Common::Point(posX - regsX, posY - regsY));
		}
	}

	int16 combinedVal = _padDecorationShape[row][col];
	if (0 < combinedVal && combinedVal < 36) {
		int16 regsX = (combinedVal < static_cast<int16>(_padShapeOffsetX.size())) ? _padShapeOffsetX[combinedVal] : 0;
		int16 regsY = (combinedVal < static_cast<int16>(_padShapeOffsetY.size())) ? _padShapeOffsetY[combinedVal] : 0;
		_vm->_gfx->drawShape(ZoombiniGraphics::kShapeScreen, shapeRes, static_cast<uint16>(combinedVal), Common::Point(posX - regsX, posY - regsY));
	}
}

ZmbRenderResult ZoombiniPuzzleLilly::renderCellAnimA(ZmbFeature *feature) {
	// First swap cell blink.
	if (!feature->isRenderActivated())
		return ZmbRenderResult::kSkipped;

	renderCellBlink(_firstSwapBlinkFrame, _firstSwapCellCol, _firstSwapCellRow);
	return ZmbRenderResult::kRendered;
}

ZmbRenderResult ZoombiniPuzzleLilly::renderCellAnimB(ZmbFeature *feature) {
	// Render the second swap-cell blink.
	// @ref ZoombiniPuzzleLilly::processCellSwapAnimation() executes the swap.
	if (!feature->isRenderActivated())
		return ZmbRenderResult::kSkipped;

	renderCellBlink(_secondSwapBlinkFrame, _secondSwapCellCol, _secondSwapCellRow);
	return ZmbRenderResult::kRendered;
}

// =================================================================
// Pre-render shape hooks
// =================================================================

void ZoombiniPuzzleLilly::childPreRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)hsGroup;

	// Offset the child's body shape by the Snoid sprite variant, which is the hair trait minus one.
	// Apply the REGS 10000 registration correction afterward.
	for (int16 runnerIdx = 0; runnerIdx < static_cast<int16>(_pageLoadedZmbCount) && runnerIdx < kToadBase; runnerIdx++) {
		if (_runnerFeatures[runnerIdx] != feature)
			continue;
		ZmbLillyGridWalker &rs = _runnerStates[runnerIdx];
		if (rs.childShapeOffset != 0 && !hotspots.empty() &&
			0 < hotspots[0]._shapeIdx)
			hotspots[0]._shapeIdx += rs.childShapeOffset;
		return;
	}
}

void ZoombiniPuzzleLilly::toadPreRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	// Toad shape post-processing, split by the current SCRB:
	// - Ordinary SCRBs: hotspot 1 is the tattoo overlay and hotspot 2 is the rider.
	//   Rotation SCRBs 10061/10063/10065 use a tattoo offset reduced by 7.
	// - SCRB 10057/10058/10059: phase-local hotspot interpolation toward the path target.
	//   These scripts own their extra water/body layers; they are not rider placeholders.
	// Do not approximate this by moving the whole feature: these SCRBs contain authored intermediate coordinates,
	// and feature movement applies a second translation to their extra shape pairs.
	int16 runnerIdx = -1;
	for (int16 toadRunnerIdx = 0; toadRunnerIdx < kToadCount; toadRunnerIdx++) {
		if (_runnerFeatures[kToadBase + toadRunnerIdx] == feature) {
			runnerIdx = kToadBase + toadRunnerIdx;
			break;
		}
	}
	if (runnerIdx < 0)
		return;

	ZmbLillyGridWalker &rs = _runnerStates[runnerIdx];
	const int16 scrbId = feature->getScrbId();
	const bool isPathScrb = scrbId == kResScrb10057_Move || scrbId == kResScrb10058_MoveAlternate || scrbId == kResScrb10059_MoveFinal;
	const int32 pathPhase = hsGroup ? hsGroup->_frameIdx : -1;

	int16 overlayOffset = rs.tattooIdx;
	if (kResScrb10061_SwapAlternateBase <= scrbId &&
		scrbId <= kResScrb10061_SwapAlternateBase + 4 && (scrbId & 1) != 0)
		overlayOffset = rs.tattooIdx - 7;

	const bool isFinalDepartureEffect = 1 < hotspots.size() && scrbId == kResScrb10059_MoveFinal && 5 <= pathPhase && hotspots[1]._shapeIdx != 91;
	if (1 < hotspots.size() && 0 < hotspots[1]._shapeIdx &&
		!isFinalDepartureEffect)
		hotspots[1]._shapeIdx += overlayOffset;
	if (!isPathScrb && 0 <= rs.childRunnerIdx &&
		2 < hotspots.size() && 0 < hotspots[2]._shapeIdx)
		hotspots[2]._shapeIdx += rs.riderShapeOffset;

	if (isPathScrb) {
		if (pathPhase < 0)
			return;

		if (pathPhase == 0 && rs.lastAnimPhase != 0) {
			rs.moveStartX = feature->getPointLoc().x;
			rs.moveStartY = feature->getPointLoc().y;
		}
		rs.lastAnimPhase = pathPhase;

		const Common::Point target(rs.scriptTargetX, rs.scriptTargetY);
		const Common::Point step((rs.scriptTargetX - rs.moveStartX) / 3, (rs.scriptTargetY - rs.moveStartY) / 3);

		uint32 adjustedCount = MIN<uint32>(2, hotspots.size());
		if (scrbId == kResScrb10059_MoveFinal && 5 <= pathPhase)
			adjustedCount = hotspots.size();

		for (uint32 hotspotIdx = 0; hotspotIdx < adjustedCount; hotspotIdx++) {
			ZmbHotspot &hotspot = hotspots[hotspotIdx];
			if (hotspot._shapeIdx == ZmbHotspot::kShapeNone)
				continue;

			if (pathPhase <= 2) {
				// The standard runner already performs the one required REGS subtraction for this feature group.
				continue;
			}

			Common::Point phaseTarget = target;
			if (pathPhase == 3)
				phaseTarget = feature->getPointLoc() + step;
			else if (pathPhase == 4)
				phaseTarget = feature->getPointLoc() + 2 * step;

			hotspot._x = phaseTarget.x;
			hotspot._y = phaseTarget.y;
		}
	}

	if (kResScrb10001_StepDirection0 <= scrbId && scrbId <= kResScrb10004_StepLast &&
		hsGroup && !hotspots.empty()) {
		// Use the logical SCRB frame rather than the returned materialized
		// group. Event-only frame 7 deliberately reuses frame 6's shapes.
		const int32 phase = feature->getLastFrameIdx();
		rs.lastAnimPhase = phase;

		if (phase == 2) {
			// SCRB group 1 ends at FE0C and dispatches event 11, which only
			// advances the logical cell and computes the destination. Group 2
			// ends at FF0D and dispatches event 12; that is the first group
			// whose materialized hotspots are rewritten to the half-way point.
			for (uint32 hotspotIdx = 0; hotspotIdx < hotspots.size(); hotspotIdx++) {
				if (hotspots[hotspotIdx]._shapeIdx == ZmbHotspot::kShapeNone)
					continue;
				if (rs.direction == ZmbLillyGridWalker::kDirectionUp ||
					rs.direction == ZmbLillyGridWalker::kDirectionDown) {
					hotspots[hotspotIdx]._y =
						rs.moveStartY + (rs.moveTargetY - rs.moveStartY) / 2;
				} else {
					hotspots[hotspotIdx]._x =
						rs.moveStartX + (rs.moveTargetX - rs.moveStartX) / 2;
				}
			}
		} else if (3 <= phase && phase <= 6) {
			// Group 3 dispatches event 13 and groups 4-6 dispatch event 14.
			// Both write destination X for every direction. Vertical movement
			// keeps the authored Y arc; writing destination Y here detached the
			// tattoo/body composite.
			for (uint32 hotspotIdx = 0; hotspotIdx < hotspots.size(); hotspotIdx++) {
				if (hotspots[hotspotIdx]._shapeIdx == ZmbHotspot::kShapeNone)
					continue;
				hotspots[hotspotIdx]._x = rs.moveTargetX;
			}
		}
	}
}

void ZoombiniPuzzleLilly::crabPreRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	// 8-phase movement for the step SCRBs.
	int16 runnerIdx = -1;
	for (int16 runnerSlotIdx = 0; runnerSlotIdx < kMaxCrabs; runnerSlotIdx++) {
		if (_runnerFeatures[kCrabBase + runnerSlotIdx] == feature) {
			runnerIdx = kCrabBase + runnerSlotIdx;
			break;
		}
	}
	if (runnerIdx < 0)
		return;

	ZmbLillyGridWalker &rs = _runnerStates[runnerIdx];
	const bool freshMode = (rs.traversalMode == ZmbLillyGridWalker::kTraversalFresh);

	const int16 scrbId = feature->getScrbId();
	const bool isMovementScrb =
		scrbId == kResScrb10071_CrabDirection0 ||
		scrbId == kResScrb10073_CrabDirection2 ||
		scrbId == kResScrb10075_CrabDirection3 ||
		scrbId == kResScrb10077_CrabDirection1;

	int32 phase = hsGroup ? hsGroup->_frameIdx : -1;
	if (phase < 0 || hotspots.empty())
		return;
	if (isMovementScrb && phase != rs.lastAnimPhase) {
		rs.lastAnimPhase = phase;
		switch (phase) {
		case 0: {
			// Apply the direction key to the grid position, compute destination.
			rs.prevCol = rs.gridCol;
			rs.prevRow = rs.gridRow;
			ZmbGridDirection direction = kZmbGridUp;
			bool hasDirection = true;
			switch (rs.scrbKey) {
			case kResScrb10071_CrabDirection0:
				direction = kZmbGridUp;
				break;
			case kResScrb10073_CrabDirection2:
				direction = kZmbGridDown;
				break;
			case kResScrb10075_CrabDirection3:
				direction = kZmbGridLeft;
				break;
			case kResScrb10077_CrabDirection1:
				direction = kZmbGridRight;
				break;
			default:
				hasDirection = false;
				break;
			}
			if (hasDirection) {
				ZmbGridCell cell(rs.gridRow, rs.gridCol);
				ZmbGridTraversal::advanceCell(cell, direction, ZmbGridBounds(0, 11, 0, 11));
				rs.gridRow = cell.row;
				rs.gridCol = cell.col;
			}
			rs.moveTargetX = _padCellPositions[rs.gridRow][rs.gridCol].x;
			rs.moveTargetY = _padCellPositions[rs.gridRow][rs.gridCol].y;
			rs.moveStartX = feature->getPointLoc().x;
			rs.moveStartY = feature->getPointLoc().y;
			break;
		}
		case 6:
			// Fresh crabs vacate the previous cell at the destination frame.
			if (freshMode &&
				0 <= rs.prevRow && rs.prevRow < 12 && 0 <= rs.prevCol && rs.prevCol < 13)
				_cellOccupancy[rs.prevRow][rs.prevCol] = 0;
			break;
		case 7:
			// Write both the feature position and secondary position after materializing the final frame.
			feature->setPointLoc(Common::Point(rs.moveTargetX, rs.moveTargetY));
			feature->setPointRef(Common::Point(rs.moveTargetX, rs.moveTargetY));
			rs.nextMoveReadyFrame = getCurrentFrameCounter() + 35;
			if (_toadPathStepQueueSize < kMaxMoveQueueSize) {
				_toadPathStepQueue[_toadPathStepQueueSize] = runnerIdx;
				_toadPathStepQueueSize += 1;
			}
			break;
		default:
			break;
		}
	}

	// Keep only the first body hotspot when a reuse-mode crab follows an existing trail.
	// Fresh crabs retain every authored hotspot and remap each shell shape 272 to its tattoo.
	// A reuse-mode crab that entered a fresh cell promotes the secondary shapes again during movement phase 7.
	const bool showSecondaryShapes =
		freshMode || (isMovementScrb && phase == 7 && rs.bfsReinitPending);
	for (uint32 hotspotIdx = 1; hotspotIdx < hotspots.size(); hotspotIdx++) {
		if (hotspots[hotspotIdx]._shapeIdx == ZmbHotspot::kShapeNone)
			continue;
		if (!showSecondaryShapes) {
			hotspots[hotspotIdx]._shapeIdx = ZmbHotspot::kShapeNone;
			continue;
		}
		if (hotspots[hotspotIdx]._shapeIdx == 272)
			hotspots[hotspotIdx]._shapeIdx += rs.tattooIdx;
	}

	if (!isMovementScrb) {
		// The standard runner owns the single REGS correction for non-movement SCRBs after this hook.
		return;
	}
	if (hotspots.empty()) {
		error("lilly: malformed required movement SCRB hotspot table");
		return;
	}

	Common::Point renderBase(hotspots[0]._x, hotspots[0]._y);
	if (phase == 2) {
		renderBase = Common::Point(rs.moveStartX + (rs.moveTargetX - rs.moveStartX) / 2, rs.moveStartY + (rs.moveTargetY - rs.moveStartY) / 2);
	} else if (6 <= phase) {
		renderBase = Common::Point(rs.moveTargetX, rs.moveTargetY);
	}

	for (uint32 hotspotIdx = 0; hotspotIdx < hotspots.size(); hotspotIdx++) {
		ZmbHotspot &hotspot = hotspots[hotspotIdx];
		if (hotspot._shapeIdx == ZmbHotspot::kShapeNone)
			continue;

		// The runner applies one REGS correction after this hook.
		hotspot._x = renderBase.x;
		hotspot._y = renderBase.y;
	}
}

// =================================================================
// Print the debug answer.
// =================================================================

Common::String ZoombiniPuzzleLilly::debugGetAnswer() const {
	// Debug names for the three lily-pad attribute families and their values.
	const auto padFamilyName = [](ZmbLillyGridWalker::PadAttrType attrType) -> const char * {
		switch (attrType) {
		case ZmbLillyGridWalker::kPadAttrPattern:
			return "PATTERN";
		case ZmbLillyGridWalker::kPadAttrShape:
			return "SHAPE";
		case ZmbLillyGridWalker::kPadAttrColor:
			return "COLOR";
		default:
			return "NONE";
		}
	};

	const auto padValueName = [](ZmbLillyGridWalker::PadAttrType attrType, int16 value) -> const char * {
		static constexpr const char *kPatternNames[3] = {"flower", "cross", "diamond"};
		static constexpr const char *kShapeNames[4] = {"oneCut", "twoCut", "threeLeaf", "fourLeaf"};
		static constexpr const char *kColorNames[5] = {"magenta", "red", "orange", "cyan", "beige"};
		switch (attrType) {
		case ZmbLillyGridWalker::kPadAttrPattern:
			if (0 <= value && value < 3)
				return kPatternNames[value];
			break;
		case ZmbLillyGridWalker::kPadAttrShape:
			if (0 <= value && value < 4)
				return kShapeNames[value];
			break;
		case ZmbLillyGridWalker::kPadAttrColor:
			if (0 <= value && value < 5)
				return kColorNames[value];
			break;
		default:
			break;
		}
		return "?";
	};

	const auto padFamilySortKey = [](ZmbLillyGridWalker::PadAttrType attrType) -> int16 {
		switch (attrType) {
		case ZmbLillyGridWalker::kPadAttrColor:
			return 0;
		case ZmbLillyGridWalker::kPadAttrPattern:
			return 1;
		case ZmbLillyGridWalker::kPadAttrShape:
			return 2;
		default:
			error("lilly: invalid lilly pad family: %d", attrType);
			return 3;
		}
	};

	// Report the generation-owned routes and their initial post-challenge state.
	// Player-made pad swaps do not change this diagnostic answer.
	Common::String s = getDebugBanner();
	s += "\n";
	s += "  Coordinates: rows and columns are 1-based.\n";
	s += "  Answer basis: puzzle generation; later lily-pad swaps are ignored.\n";
	s += Common::String::format("  Zoombinis: %d total, %d placed, %d across\n", _pageLoadedZmbCount, _assignedSnoidCount, _completedSnoidCount);
	s += debugGetChanceDetails();

	if (0 < _crabEntryCount) {
		s += "  Crab plan:\n";
		s += Common::String::format("    Family: %s\n", padFamilyName(_crabPathAttrType));
		s += "    Entry columns:\n";
		for (int16 i = 0; i < _crabEntryCount; i++) {
			s += Common::String::format("      Column %d: %s\n", _crabEntryColumns[i] + 1, padValueName(_crabEntryAttrTypes[i], _crabEntryAttrValues[i]));
		}
	}

	int16 toadOrder[kToadCount] = {};
	for (int16 toadRunnerIdx = 0; toadRunnerIdx < kToadCount; toadRunnerIdx++)
		toadOrder[toadRunnerIdx] = toadRunnerIdx;

	const auto isToadBefore = [&](int16 leftToadIdx, int16 rightToadIdx) -> bool {
		const ZmbLillyGridWalker &left = _runnerStates[kToadBase + leftToadIdx];
		const ZmbLillyGridWalker &right = _runnerStates[kToadBase + rightToadIdx];
		const int16 leftFamily = padFamilySortKey(left.attrType);
		const int16 rightFamily = padFamilySortKey(right.attrType);
		if (leftFamily != rightFamily)
			return leftFamily < rightFamily;
		if (left.attrValue != right.attrValue)
			return left.attrValue < right.attrValue;
		return leftToadIdx < rightToadIdx;
	};
	Common::sort(toadOrder, toadOrder + kToadCount, isToadBefore);

	s += "  Toads (color -> pattern -> shape order):\n";
	for (int16 orderIdx = 0; orderIdx < kToadCount; orderIdx++) {
		const int16 toadRunnerIdx = toadOrder[orderIdx];
		const ZmbLillyGridWalker &rs = _runnerStates[kToadBase + toadRunnerIdx];
		const uint16 generatedRows = _generatedAnswerEntryRowMasks[rs.tattooIdx];
		const uint16 directRows = _generatedDirectEntryRowMasks[rs.tattooIdx];
		Common::String rows;
		for (int16 rowIdx = 0; rowIdx < 12; rowIdx++) {
			const uint16 rowMask = static_cast<uint16>(1U << rowIdx);
			if ((generatedRows & rowMask) == 0)
				continue;

			if (!rows.empty())
				rows += ", ";
			const bool direct = (directRows & rowMask) != 0;
			rows += Common::String::format("%d (%s)", rowIdx + 1, direct ? "direct" : "swap required");
		}

		s += Common::String::format("    Toad of %s %s:\n", padFamilyName(rs.attrType), padValueName(rs.attrType, rs.attrValue));
		s += Common::String::format("      Answer entry rows: %s\n", rows.empty() ? "none" : rows.c_str());
		// A crossing opportunity is consumed when the Toad reaches the far side.
		// Mark a Toad that is still on the grid so an in-progress crossing is not
		// mistaken for an unused opportunity.
		const char *crossingState = (rs.occupiesGridCell && !rs.toadRetired) ? "(crossing)" : "";
		s += Common::String::format("      Crossing opportunities used: %d of 2 %s\n", rs.completedCrossingCount, crossingState);
		if (rs.toadRetired)
			s += "      Status: retired\n";
		else if (rs.occupiesGridCell) {
			const char *pathDescription;
			if (rs.maxReachableProgress == 11)
				pathDescription = "reaches exit";
			else
				pathDescription = "does not reach exit";
			s += Common::String::format("      On grid: row %d, column %d, path %s\n", rs.gridRow + 1, rs.gridCol + 1, pathDescription);
		}
	}
	s += "  'direct' reaches the far side in the generated layout.\n";
	s += "  'swap required' is a generated route interrupted by challenge pads.\n";
	return s;
}

ZmbChanceInfo ZoombiniPuzzleLilly::debugGetChances() const {
	if (_difficultyLevel < kPuzzleLevel2 || _swapsPerWandStage <= 0)
		return ZmbChanceInfo(ZmbChanceInfo::ZmbChanceType::kAmorphous);

	const int16 totalUses = static_cast<int16>(6 * _swapsPerWandStage);
	int16 used = static_cast<int16>(_swapWandStage * _swapsPerWandStage + _swapsTowardNextStage);
	used = MIN<int16>(used, totalUses);
	return ZmbChanceInfo(ZmbChanceInfo::ZmbChanceType::kAmorphous, totalUses, used, "swap wand use");
}

bool ZoombiniPuzzleLilly::debugCanSetChances() const {
	return kPuzzleLevel2 <= _difficultyLevel && 0 < _swapsPerWandStage;
}

bool ZoombiniPuzzleLilly::debugSetChances(int16 remaining) {
	const ZmbChanceInfo info = debugGetChances();
	if (info.opportunities < 0 || remaining < 0 || info.opportunities < remaining)
		return false;

	const int16 used = static_cast<int16>(info.opportunities - remaining);
	_swapWandStage = static_cast<int16>(used / _swapsPerWandStage);
	_swapsTowardNextStage = static_cast<int16>(used % _swapsPerWandStage);
	if (_swapWandFeature)
		loadSwapWandScrb(kResScrb10078_ToadBase + _swapWandStage);
	return true;
}

Common::String ZoombiniPuzzleLilly::debugGetChanceDetails() const {
	const ZmbChanceInfo info = debugGetChances();
	if (info.opportunities < 0 || info.used < 0)
		return "  Swap wand: unavailable at this level.\n";

	return Common::String::format("  Swap wand uses left: %d (used %d of %d)\n", info.chancesLeft(), info.used, info.opportunities);
}

} // End of namespace Mohawk
