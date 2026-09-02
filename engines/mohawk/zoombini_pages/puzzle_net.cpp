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

#include "mohawk/zoombini_pages/puzzle_net.h"
#include "mohawk/zoombini.h"
#include "mohawk/zoombini_graphics.h"
#include "mohawk/zoombini_metaengine.h"
#include "mohawk/zoombini_random.h"
#include "mohawk/zoombini_sound.h"
#include "mohawk/zoombini_state.h"

namespace Mohawk {

constexpr Common::Point ZoombiniPuzzleNet::kSnoidPositions[16];
constexpr Common::Point ZoombiniPuzzleNet::kSlotPositionsLow[25];
constexpr Common::Point ZoombiniPuzzleNet::kSlotPositionsHigh[125];
constexpr Common::Point ZoombiniPuzzleNet::kExitPositions[16];
constexpr Common::Point ZoombiniPuzzleNet::kEntryStartPositions[3];
constexpr Common::Point ZoombiniPuzzleNet::kEntryExitPositions[3];
constexpr int16 ZoombiniPuzzleNet::kColOffsets1[5];
constexpr int16 ZoombiniPuzzleNet::kColOffsets2[5];

constexpr const char *ZoombiniPuzzleNet::kMudballSelectorValueNames[3][5];

int16 ZoombiniPuzzleNet::getMudTankTargetLevel(int16 remainingShotAllowance) {
	return (0 < remainingShotAllowance) ? static_cast<int16>(remainingShotAllowance - 1) : 0;
}

ZoombiniPuzzleNet::ZoombiniPuzzleNet(MohawkEngine_Zoombini *vm) : ZoombiniPuzzle(vm, ZoombiniPageType::kNet, ZmbSrcPageKind::kNet_12) {
}

ZoombiniPuzzleNet::~ZoombiniPuzzleNet() {
}

const ZoombiniPage::ScriptSoundPriorityRanges &ZoombiniPuzzleNet::getScriptSoundPriorityRanges() const {
	// Original shared page dispatch automatically prepends SND 996-997 at priority 32.
	static const ZoombiniPage::ScriptSoundPriorityRanges kRanges = {
		{kSysResSoundRange0996_DepartSFX, kSysResSoundRange0997_ArriveSFX},
		// Net then registers the same range again in its authored list.
		{kSysResSoundRange0996_DepartSFX, kSysResSoundRange0997_ArriveSFX},
		{kSysResSoundRange20000_Base, kSysResSoundRange29999_Last},
		{kResSoundRange0300_PageRangeBase, kResSoundRange0324_PageRangeLast},
		{kResSoundRange0425_PageRangeBase, kResSoundRange0499_PageRangeLast},
		{kResSoundRange8000_PagePriorityBase, kResSoundRange8002_PagePriorityLast},
		{kResSoundRange9000_PageRangeBase, kResSoundRange10999_PageRangeLast},
		{kResSoundRange7000_PageRangeBase, kResSoundRange7999_PageRangeLast},
		{kResSoundRange10000_PagePriorityBase, kResSoundRange10099_PagePriorityLast}};
	return kRanges;
}

void ZoombiniPuzzleNet::open() {
	openArchive(_vm->isDemo() ? ZMB_MHK_NET_DEMO : ZMB_MHK_NET);
}

void ZoombiniPuzzleNet::setBackgroundMusic() {
	// Sound 20064 is reserved for F1 replay and does not start on page load.
}

void ZoombiniPuzzleNet::setBackgroundBitmap() {
	// Select background 5000 or 5001 from the difficulty.
	int16 bgId = (kPuzzleLevel3 <= _difficultyLevel) ? kResBackground5001_Hard : kResBackground5000_Normal;
	_vm->_gfx->setPalette(bgId);
	_vm->_gfx->drawBackground(bgId);
}

void ZoombiniPuzzleNet::initStates() {
	// Initialize puzzle state
	_totalSlotCount = (kPuzzleLevel3 <= _difficultyLevel) ? 125 : 25;
	_goButtonEnabled = false;
	_renderedGoButtonEnabled = false;
	_columnLabelInvalidatedThisFrame = false;

	// Initial selector values (0-4).
	_selectedSelectorValues[0] = _vm->_rnd->getRandomNumber(0, 4);
	_selectedSelectorValues[1] = _vm->_rnd->getRandomNumber(0, 4);
	_selectedSelectorValues[2] = _vm->_rnd->getRandomNumber(0, 4);
	_previousSelectorValues[0] = -1;
	_previousSelectorValues[1] = -1;
	_previousSelectorValues[2] = -1;
	_hoveredSelectorValues[0] = -1;
	_hoveredSelectorValues[1] = -1;
	_hoveredSelectorValues[2] = -1;
	_submitHoverVisible = false;
	_selectorHoverMousePos = Common::Point(-1, -1);

	_mudTankIntroActive = true;
	_columnLabelIntroRunning = true;
	_initialSelectorSetup = true;
	_puzzleInputEnabled = false;
	_mudTankIntroStep = 0;
	_mudTankVisualLevel = 0;
	_mudTankAnimEndLevel = 0;
	_mudTankAdjustmentTargetLevel = 0;
	_mudTankAnimActive = false;
	_mudTankAdjustmentPending = false;
	_mudTankSettledFrameActive = false;
	_acceptedZmbCount = 0;
	_rejectedShotCount = 0;
	_submissionActivityCount = 0;
	_selectorInputEnabled = false;
	_acceptedLaunchGatePending = false;
	_selectorGateReleasePending = false;
	_remainingLaunchesForShot = 0;
	_columnOpenPendingCount = 0;
	_nextZmbToAssign = 0;
	_pendingZmbIndex = -1;
	_launchingSnoidId = 0;
	_lastAcceptedSnoidId = 0;
	_acceptedExitCompletionCount = 0;
	_packAssignmentExhausted = false;
	_queuedColumnAssignmentCount = 0;
	_columnAssignmentReadyCount = 0;
	_submitHoverEnabled = false;
	_goButtonEnabled = false;
	_shotMissedTarget = false;
	_partialSuccessNarrationPending = false;
	_lastShotFeatureIdx = -1;
	_selectedTargetSlotIdx = -1;
	_selectorSetupPendingCount = 0;
	_selectorRevealStartPending = false;
	_selectorRevealScrb7018Running = false;
	_selectorRevealScrb7025Running = false;
	_selectorRevealScrb7026Running = false;
	_selectorRevealScrb7027Running = false;
	_columnOpenAnimRunning = false;
	_openingColumnIdx = 0;
	_columnEntryAnimRunning = false;
	_sortAnimRunning = false;
	_positionShotHotspots = false;
	_bounceCounter = 0;
	_sortAnimType = SortAnimationVariant::kCenter00;
	_acceptedExitPositionIdx = 0;
	_lastSubmitFrame = 0;
	_pendingBodyArrangement = 0;
	memset(_columnSlotSnoidIds, 0, sizeof(_columnSlotSnoidIds));
	memset(_travelingColumnSnoidIds, 0, sizeof(_travelingColumnSnoidIds));
	memset(_targetSlotFeatures, 0, sizeof(_targetSlotFeatures));
	memset(_shotHistoryFeatures, 0, sizeof(_shotHistoryFeatures));

	// Less-action mode limits each ambient cycle to two animations instead of three.
	_idleAnimTrigger = false;
	_idleAnimCount = 0;
	_idleAnimMax = _vm->_state->_f.getLessActionEnabled() ? 2 : 3;
	_idleAnimPoolState = 0;
	_idleAnimLastFrame = 0;
	_roundCompleted = false;
}

void ZoombiniPuzzleNet::loadFeatures() {
	// Seed the global runner list with one positional z-sort before later updates disable automatic sorting.
	setRunnerZSortEnabled(true);

	// Preload shape images at tBMP 6000 (0x1770)
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape6000_Snoid));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape7000_Entry));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape8000_Column));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape9000_Slot));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape10000_Feedback));

	// Feature groups

	// Load main features: 48 SCRBs at 7000
	ZmbFeature *mainFeature = createPuzzleMainFeatureHead();

	{
		ZmbFeature *parent = mainFeature;
		for (uint16 i = 0; i < 8; i++) {
			parent = loadSubFeature(parent, ZmbResource(ZmbResource::kPage, kResBitmapShape8000_Column), kResScrb8000_ColumnBase + i);
		}
	}

	{
		ZmbFeature *parent = mainFeature;
		for (uint16 i = 0; i < 154; i++) {
			parent = loadSubFeature(parent, ZmbResource(ZmbResource::kPage, kResBitmapShape9000_Slot), kResScrb9000_SlotBase + i);
		}
	}

	{
		ZmbFeature *parent = mainFeature;
		// TLC v2 adds selector-button SCRBs 10019-10035 to the original feedback group.
		const uint16 feedbackScrbCount = _vm->isVersionFamilyTlcV2() ? 36 : 19;
		for (uint16 i = 0; i < feedbackScrbCount; i++) {
			parent = loadSubFeature(parent, ZmbResource(ZmbResource::kPage, kResBitmapShape10000_Feedback), kResScrb10000_FeedbackBase + i);
		}
	}

	// Group 0 contains the three entry scripts at SCRS 14000 and uses the normal Snoid render state.
	registerScrsGroup(kResScrs14000_EntryBase, 3);

	// Group 1 contains 51 scripts starting at SCRS 13000.
	// It uses the reject render state with tBMP 3000 and the general body tables.
	// It owns the walk (13001-13015), seating (13016-13030), and launch (13031-13045) animations.
	// It also owns idle animations starting at 13046.
	registerScrsGroup(kResScrs13000_NormalBase, 51);

	// Register virtual render feature for trait slot buttons
	{
		ZmbFeature::EventHooks traitSlotHooks;
		traitSlotHooks.setPreRenderFunc(static_cast<ZmbFeature::OnPreRenderFunc>(&ZoombiniPuzzleNet::traitSlots_preRender));
		traitSlotHooks.setRenderFunc(static_cast<ZmbFeature::OnRenderFunc>(&ZoombiniPuzzleNet::traitSlots_render));
		loadScrbFeature(ZmbResource(ZmbResource::kPage, 0), 0, 0, ZmbFeature::FLAG_00001000_TOPMOST, traitSlotHooks);
	}

	// Load Zoombinis at 16 pedestal positions
	loadZoombinisFromPack(kSnoidPositions, ARRAYSIZE(kSnoidPositions));

	// The shot budget depends on the materialized pack count. Keep this block
	// after @ref ZoombiniPuzzle::loadZoombinisFromPack() and before
	// @ref ZoombiniPuzzleNet::registerColumnRunners(), which consumes
	// @ref ZoombiniPuzzleNet::_mudTankScrbOffset when it starts the tank intro.
	computeTargetLaunchBatches();
	_remainingShotAllowance = _targetCount + (_difficultyLevel == kPuzzleLevel4 ? 1 : 0) + 7;
	_initialShotAllowance = _remainingShotAllowance;
	_mudTankScrbOffset = 16 - _remainingShotAllowance;

	// Register column SCRB runners and start initial animations
	registerColumnRunners();
	// @ref ZoombiniPuzzleNet::registerColumnRunners() starts the mud-tank intro.
	_mudTankIntroStepActive = true;

	// Lay out the initial Snoids with a 30-frame walk-in stagger.
	layoutStaticAndWalkIn(0, false);
	// Preserve the first sorted runner list, then switch NET to persistent manual links
	// for the rest of the page lifetime.
	renderFeatures();
	setRunnerZSortEnabled(false);
	assignStaggeredWalkDelays(0, 30);
	// Only Snoids later accepted by the wall/column callbacks belong to the continuing group.
	schedulePackSnoids(false, false);

	// Buttons
	// Set up Go/Map/Help buttons
	configureStandardPuzzleControlRects();
	if (_vm->isDemo())
		setGoButtonsEnabled(true);
	loadStandardPuzzleControlFeatures(kResBitmapShape6000_Snoid);
}

void ZoombiniPuzzleNet::initHelpPrompt() {
	_activeHelpSoundId = ZmbResource(ZmbResource::kSystem, kSysResSound20064_F1Replay);
}

void ZoombiniPuzzleNet::onGoButtonActivated() {
	if (_vm->isDemo()) {
		if (_vm->openMsgBoxDialog(ZoombiniMsgBoxType::kAskReallyQuit) == ZoombiniDialogResult::kYes)
			Engine::quitGame();
		return;
	}

	// Stop BGM, play departure SFX, walk snoids to (600, -100), fade out when SFX finishes.
	// Stop background music
	_vm->_sound->stopAllSoundQueues();

	// Stop scheduling new celebration animations once departure begins.
	_idleAnimTrigger = false;

	startDepartWalkAnimation(Common::Point(600, -100));
	ZoombiniInteractive::onGoButtonActivated();
}

void ZoombiniPuzzleNet::debugPrepareForDeparture() {
	if (isDragging())
		finishSnoidDrag();

	// The debug finish skips the authored sorting puzzle, but leaves the
	// accepted runners at the same visible exit positions used by a completed
	// submission before the normal Go departure walk begins.
	_mudTankIntroActive = false;
	_mudTankIntroStepActive = false;
	_mudTankIntroStep = 0;
	_mudTankAnimActive = false;
	_mudTankAdjustmentPending = false;
	_mudTankAdjustmentTargetLevel = 0;
	_mudTankSettledFrameActive = false;
	_acceptedExitPositionIdx = 0;
	_acceptedExitCompletionCount = 0;
	_queuedColumnAssignmentCount = 0;
	_columnAssignmentReadyCount = 0;
	_columnSlotSnoidIds[0] = 0;
	_columnSlotSnoidIds[1] = 0;
	_columnSlotSnoidIds[2] = 0;
	_travelingColumnSnoidIds[0] = 0;
	_travelingColumnSnoidIds[1] = 0;
	_travelingColumnSnoidIds[2] = 0;
	_launchingSnoidId = 0;
	_goButtonEnabled = false;
	_roundCompleted = true;
	_idleAnimTrigger = false;
	_idleAnimCount = 0;
	_idleAnimMax = 0;

	int16 exitIndex = 0;
	for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
		ZmbSnoid *snoid = *it;
		if (!snoid || !snoid->isPackSnoid())
			continue;

		const Common::Point exitPos = kExitPositions[CLIP<int16>(exitIndex, 0, 15)];
		addExternalDirtyRect(snoid->getClickRect());
		snoid->setPointLoc(exitPos);
		snoid->setAnimTargetPos(exitPos);
		snoid->setAnimState(kSnoidAnimState000_Idle);
		snoid->setupIdleHotspots();
		snoid->activateRender();
		snoid->_packIsOccupied = true;
		snoid->setNeedsRedraw(true);
		prepareSnoidVisualCoverage(snoid, true);
		addExternalDirtyRect(snoid->getClickRect());
		exitIndex += 1;
	}

	_goButtonEnabled = 0 < exitIndex;
	setGoButtonsEnabled(_goButtonEnabled);
}

ZmbChanceInfo ZoombiniPuzzleNet::debugGetChances() const {
	// Initialize a finite shot budget from the active-pack column count.
	// Decrement it after every completed mudball launch.
	return {ZmbChanceInfo::ZmbChanceType::kSubmit, _initialShotAllowance,
			CLIP<int16>(static_cast<int16>(_initialShotAllowance - _remainingShotAllowance),
						0, _initialShotAllowance),
			"mudball launch"};
}

bool ZoombiniPuzzleNet::debugSetChances(int16 remaining) {
	// Once the finite budget reaches zero, the tank is exhausted and cannot be
	// replenished through the debugger.
	if (_remainingShotAllowance < 1 || remaining < 0 || _initialShotAllowance < remaining)
		return false;

	_remainingShotAllowance = remaining;
	_mudTankAdjustmentTargetLevel = getMudTankTargetLevel(remaining);
	_mudTankAdjustmentPending = true;
	return true;
}

void ZoombiniPuzzleNet::startMudTankVisualAnimation(int16 scrbId, int16 destinationLevel, bool accelerated) {
	if (!_mudTankFeature)
		return;

	_mudTankFeature->setFrameInterval(accelerated ? kMudTankDebugFrameInterval : kMudTankNormalFrameInterval);
	loadScrbOntoFeature(_mudTankFeature, scrbId);
	_mudTankSettledFrameActive = false;
	_mudTankAnimEndLevel = CLIP<int16>(destinationLevel, 0, 16);
	_mudTankAnimActive = true;
	_mudTankScrbOffset = 16 - _mudTankAnimEndLevel;
	debugC(3, MohawkEngine_Zoombini::kDebugPage02, "net: loading mud tank SCRB %d for visual level %d to %d", scrbId, _mudTankVisualLevel, _mudTankAnimEndLevel);
}

void ZoombiniPuzzleNet::finishMudTankVisualAnimation() {
	if (!_mudTankAnimActive || !_mudTankFeature ||
		!_mudTankFeature->hasAnimEndCallbackFired())
		return;

	_mudTankVisualLevel = _mudTankAnimEndLevel;
	_mudTankScrbOffset = 16 - _mudTankVisualLevel;
	_mudTankAnimActive = false;
	debugC(3, MohawkEngine_Zoombini::kDebugPage02, "net: mud tank settled at visual level %d", _mudTankVisualLevel);
}

void ZoombiniPuzzleNet::settleMudTankVisualFrame() {
	if (!_mudTankFeature)
		return;

	const int16 settledLevel = CLIP<int16>(_mudTankVisualLevel, 0, 15);
	const int16 scrbId = static_cast<int16>(kResScrb7031_ExitBase + 15 - settledLevel);
	_mudTankFeature->setFrameInterval(kMudTankNormalFrameInterval);
	loadScrbOntoFeature(_mudTankFeature, scrbId);
	_mudTankFeature->deactivateAnimate();
	_mudTankFeature->setLastFrameIdx(_mudTankFeature->getMaxFrameIdx());
	_mudTankFeature->activateRender();
	_mudTankFeature->resetNextRenderFrame();
	_mudTankFeature->requestVisualRematerialization();
	_mudTankSettledFrameActive = true;
	debugC(4, MohawkEngine_Zoombini::kDebugPage02, "net: froze mud tank SCRB %d at visual level %d", scrbId, settledLevel);
}

void ZoombiniPuzzleNet::updateChanceVisualAnimation() {
	finishMudTankVisualAnimation();
	if (_mudTankIntroActive || (_selectorSetupPendingCount && !_mudTankAdjustmentPending) ||
		_mudTankAnimActive || !_mudTankFeature ||
		(!_mudTankSettledFrameActive && !_mudTankFeature->hasAnimEndCallbackFired()))
		return;

	// The launcher holds the next available shot outside the tank. The tank
	// therefore settles one step below the logical remaining-shot budget.
	const int16 logicalTargetLevel = getMudTankTargetLevel(_remainingShotAllowance);
	const int16 targetLevel = _mudTankAdjustmentPending ? _mudTankAdjustmentTargetLevel : logicalTargetLevel;
	if (_mudTankVisualLevel == targetLevel) {
		if (_mudTankAdjustmentPending) {
			settleMudTankVisualFrame();
			_mudTankAdjustmentPending = false;
		}
		return;
	}

	if (targetLevel < _mudTankVisualLevel) {
		const int16 scrbId = static_cast<int16>(kResScrb7031_ExitBase + 16 - _mudTankVisualLevel);
		debugC(3, MohawkEngine_Zoombini::kDebugPage02, "net: draining mud tank from level %d toward %d", _mudTankVisualLevel, targetLevel);
		startMudTankVisualAnimation(scrbId, _mudTankVisualLevel - 1, true);
	} else {
		const int16 destinationLevel = static_cast<int16>(_mudTankVisualLevel + 1);
		const int16 scrbId = static_cast<int16>(kResScrb7000_ExitBase + 1 + destinationLevel);
		debugC(3, MohawkEngine_Zoombini::kDebugPage02, "net: filling mud tank from level %d toward %d", _mudTankVisualLevel, targetLevel);
		startMudTankVisualAnimation(scrbId, destinationLevel, true);
	}
}

Common::String ZoombiniPuzzleNet::debugGetAnswer() const {
	Common::String s = getDebugBanner();
	s += Common::String::format(": %d target(s)\n", _targetCount);

	int16 answerTargetCount = 0;
	for (int16 pos = 0; pos < _totalSlotCount; pos++) {
		if (0 < _initialTargetLaunchCounts[pos])
			answerTargetCount += 1;
	}

	// Rule generation follows the opening label animation.
	// Do not present an empty answer if the command runs before the target markers appear.
	if (answerTargetCount != _targetCount) {
		s += "  The wall rule is not generated yet.\n";
		s += "  Wait until the target markers appear, then run again.\n";
		return s;
	}

	enum MudballSelectButtonIndex {
		kMudballSelectButtonSubColor = 0,
		kMudballSelectButtonShape = 1,
		kMudballSelectButtonColor = 2
	};
	enum WallAxisIndex {
		kWallAxisRow = 0,
		kWallAxisColumn = 1,
		kWallAxisSubColumn = 2
	};

	static constexpr const char *const kSelectorNames[] = {"SubColor", "Shape", "Color"};
	static constexpr int16 kHighAxisSelectorIndices[6][3] = {
		{kMudballSelectButtonColor, kMudballSelectButtonShape, kMudballSelectButtonSubColor},
		{kMudballSelectButtonShape, kMudballSelectButtonColor, kMudballSelectButtonSubColor},
		{kMudballSelectButtonSubColor, kMudballSelectButtonColor, kMudballSelectButtonShape},
		{kMudballSelectButtonColor, kMudballSelectButtonSubColor, kMudballSelectButtonShape},
		{kMudballSelectButtonShape, kMudballSelectButtonSubColor, kMudballSelectButtonColor},
		{kMudballSelectButtonSubColor, kMudballSelectButtonShape, kMudballSelectButtonColor}};

	int16 axisSelectButtonIndices[3] = {-1, -1, -1};
	if (_difficultyLevel < kPuzzleLevel3) {
		axisSelectButtonIndices[kWallAxisRow] = _rowAxisSelectorIdx;
		axisSelectButtonIndices[kWallAxisColumn] = _columnAxisSelectorIdx;
	} else {
		if (_axisSelectorPermutationIdx < 0 || 6 <= _axisSelectorPermutationIdx) {
			s += Common::String::format("  Invalid selector permutation: %d\n", _axisSelectorPermutationIdx);
			return s;
		}

		for (int16 axisIdx = 0; axisIdx < 3; axisIdx++)
			axisSelectButtonIndices[axisIdx] = kHighAxisSelectorIndices[_axisSelectorPermutationIdx][axisIdx];
	}

	static constexpr const char *const kAxisNames[] = {"Row", "Column", "SubColumn"};
	s += "  Wall axis mapping:\n";
	const int16 axisCount = (kPuzzleLevel3 <= _difficultyLevel) ? 3 : 2;
	for (int16 axisIdx = 0; axisIdx < axisCount; axisIdx++) {
		const int16 bIdx = axisSelectButtonIndices[axisIdx];
		s += Common::String::format("    %s: %s", kAxisNames[axisIdx], kSelectorNames[bIdx]);
		s += " (";
		for (int16 pos = 0; pos < 5; pos++) {
			if (pos != 0)
				s += ", ";

			// Read the base value at this axis position. The cross-positions are chosen so the
			// read is never affected by the level 2/4 rotation: rows use column/subColumn 0, and
			// columns/subColumns use row 0 (level 2 only rotates rows below the first).
			int16 gridValue;
			if (axisIdx == kWallAxisRow) {
				gridValue = _rowAxisRuleValues[(kPuzzleLevel3 <= _difficultyLevel) ? pos * 25 : pos * 5];
			} else if (axisIdx == kWallAxisColumn) {
				gridValue = _columnAxisRuleValues[(kPuzzleLevel3 <= _difficultyLevel) ? pos * 5 : pos];
			} else {
				gridValue = _subcolumnAxisRuleValues[pos];
			}
			s += kMudballSelectorValueNames[bIdx][gridValue];
		}
		s += ")\n";
	}

	// Level 2 and level 4 rotate a rule grid by an amount that grows with the row or subColumn.
	// Report which axis rotates and by how much so the player can account for it when reading targets.
	if (0 < _axisRuleRotationStep) {
		if (_difficultyLevel == kPuzzleLevel2) {
			s += "  Level 2 shift: each row below the first rotates its Column values rightward by a growing amount.\n";
			for (int16 row = 2; row <= 5; row++)
				s += Common::String::format("    Row %d: Column values rotated right by %d position(s).\n", row, _axisRuleRotationStep * (row - 1));
		} else if (_difficultyLevel == kPuzzleLevel4) {
			s += "  Level 4 shift: each subColumn past the first rotates its Row values rightward by a growing amount.\n";
			for (int16 subCol = 2; subCol <= 5; subCol++)
				s += Common::String::format("    SubColumn %d: Row values rotated right by %d position(s).\n", subCol, _axisRuleRotationStep * (subCol - 1));
		}
	}

	// The live assignment becomes -1 after a slot is attempted.
	// Use the retained target values so the answer remains stable throughout the round.
	s += "  Required Mudballs:\n";
	s += "    Buttons are numbered 1-5 from left to right.\n";
	int16 targetIdx = 0;
	for (int16 slotIdx = 0; slotIdx < _totalSlotCount; slotIdx++) {
		if (_initialTargetLaunchCounts[slotIdx] < 1)
			continue;

		int16 selectorValues[3] = {-1, -1, -1};
		selectorValues[axisSelectButtonIndices[kWallAxisRow]] = _rowAxisRuleValues[slotIdx];
		selectorValues[axisSelectButtonIndices[kWallAxisColumn]] = _columnAxisRuleValues[slotIdx];
		if (kPuzzleLevel3 <= _difficultyLevel)
			selectorValues[axisSelectButtonIndices[kWallAxisSubColumn]] = _subcolumnAxisRuleValues[slotIdx];

		targetIdx += 1;
		if (kPuzzleLevel3 <= _difficultyLevel) {
			const int16 cellRow = slotIdx / 25;
			const int16 cellColumn = slotIdx % 25 / 5;
			const int16 cellSubColumn = slotIdx % 5;
			s += Common::String::format("    Answer Cell (r=%d, c=%d, sc=%d)\n", cellRow + 1, cellColumn + 1, cellSubColumn + 1);
		} else {
			const int16 cellRow = slotIdx / 5;
			const int16 cellColumn = slotIdx % 5;
			s += Common::String::format("    Answer Cell (r=%d, c=%d)\n", cellRow + 1, cellColumn + 1);
		}

		for (int16 bIdx = kMudballSelectButtonSubColor; bIdx <= kMudballSelectButtonColor; bIdx++) {
			if (selectorValues[bIdx] < 0)
				continue;
			s += Common::String::format("      %-9s %d: %s\n", kSelectorNames[bIdx], selectorValues[bIdx] + 1, kMudballSelectorValueNames[bIdx][selectorValues[bIdx]]);
		}
	}
	return s;
}

Common::String ZoombiniPuzzleNet::debugGetBuiltinDebugCommandHelp() const {
	Common::String output;
	output += Common::String::format("  %-5s (%s)\n", "Space", kBuiltinDebugActionAutoSort);
	output += "    Restore the remaining-shot budget from its initial snapshot and set the next feedback SCRB offset.\n";
	output += "    After a rejection, clear the rejection count and restart the selector/feedback setup.\n";
	output += "    The hidden rule, targets, and Zoombini positions remain unchanged.\n";
	output += "    Before a rejection, this only restores the budget state and has no immediate sorting effect.\n";
	output += Common::String::format("  %-5s (%s)\n", "L/l", kBuiltinDebugActionOverlay);
	output += "    Draw the generated mudball-rule axes without changing them: SC=SubColor, SH=Shape, MC=Mudball Color.\n";
	output += "    The cells show wall row, wall column, then PR on levels 1-2 or the remaining generated axis on levels 3-4.\n";
	return output;
}

bool ZoombiniPuzzleNet::debugDoBuiltinDebugCommand(int argc, const char **argv, Common::String &output) {
	if (argc != 3) {
		output = Common::String::format("Usage: page builtin_debug <%s|%s>\n", kBuiltinDebugActionAutoSort, kBuiltinDebugActionOverlay);
		return true;
	}

	const BuiltinDebugAction action = parseBuiltinDebugAction(argv[2]);
	if (action == BuiltinDebugAction::kInvalid) {
		output = Common::String::format("Unknown Mudball Wall debug action '%s'.\n", argv[2]);
		output += debugGetBuiltinDebugCommandHelp();
		return true;
	}
	return runBuiltinDebugAction(action, output);
}

ZoombiniPuzzleNet::BuiltinDebugAction ZoombiniPuzzleNet::parseBuiltinDebugAction(const Common::String &action) {
	if (action.equalsIgnoreCase(kBuiltinDebugActionAutoSort))
		return BuiltinDebugAction::kAutoSort;
	if (action.equalsIgnoreCase(kBuiltinDebugActionOverlay))
		return BuiltinDebugAction::kOverlay;
	return BuiltinDebugAction::kInvalid;
}

bool ZoombiniPuzzleNet::runBuiltinDebugAction(BuiltinDebugAction action, Common::String &output) {
	if (action == BuiltinDebugAction::kAutoSort) {
		_remainingShotAllowance = _initialShotAllowance;
		_mudTankScrbOffset = 17 - _initialShotAllowance;
		if (_rejectedShotCount != 0) {
			_rejectedShotCount = 0;
			_selectorSetupPendingCount += 1;
		}
		output = Common::String::format("Mudball Wall remaining-shot budget restored to %d.\n", _initialShotAllowance);
	} else if (action == BuiltinDebugAction::kOverlay) {
		output = debugGetAnswer();
		static constexpr const char *const kDiagnosticLabels[3] = {"SC", "SH", "MC"};
		const Common::Rect panelRect(500, 1, 600, 27);
		drawBuiltinDebugPanel(panelRect);
		drawBuiltinDebugPanelText(kDiagnosticLabels[CLIP<int16>(_rowAxisSelectorIdx, 0, 2)], Common::Rect(500, 1, 549, 27));
		drawBuiltinDebugPanelText(kDiagnosticLabels[CLIP<int16>(_columnAxisSelectorIdx, 0, 2)], panelRect);
		const char *rightLabel;
		if (_difficultyLevel <= kPuzzleLevel2)
			rightLabel = "PR";
		else
			rightLabel = kDiagnosticLabels[CLIP<int16>(_subcolumnAxisSelectorIdx, 0, 2)];
		drawBuiltinDebugPanelText(rightLabel, Common::Rect(550, 1, 600, 27));
	} else {
		return true;
	}

	return false;
}

ZmbEventHandleResult ZoombiniPuzzleNet::onDebugKeyDown(const Common::KeyState &kbd) {
	BuiltinDebugAction action = BuiltinDebugAction::kInvalid;
	if (kbd.hasFlags(0) || kbd.hasFlags(Common::KBD_SHIFT)) {
		if (kbd.keycode == Common::KEYCODE_SPACE)
			action = BuiltinDebugAction::kAutoSort;
		else if (kbd.ascii == 'L' || kbd.ascii == 'l')
			action = BuiltinDebugAction::kOverlay;
	}
	if (action == BuiltinDebugAction::kInvalid)
		return ZmbEventHandleResult::kPassthrough;

	Common::String output;
	runBuiltinDebugAction(action, output);
	return ZmbEventHandleResult::kConsumed;
}

void ZoombiniPuzzleNet::registerColumnRunners() {
	// Register all SCRB features needed for the column-based sorting puzzle.

	// Load REGS 7000 and 7001 alongside tBMP 7000 for per-shape registration-point offsets.
	loadShapeOffsetRegs(ZmbResource::kPage, kResRegs7000_Entry);
	// Load REGS 9000 and 9001 for slot feature shapes.
	// Indicator shapes 151-156 have nonzero offsets.
	loadShapeOffsetRegs(ZmbResource::kPage, kResRegs9000_Slot);

	// Register five column runners for SCRBs 8000-8004.
	for (int16 i = 0; i < 5; i++) {
		_columnRouteFeatures[i] = loadScrbFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape8000_Column), kResScrb8000_ColumnBase + i, 6,
			ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_04000000_OVERLAY);
	}

	// Entry SCRB runner at 8005
	_columnEntryFeature = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape8000_Column), kResScrb8005_EntryBase, 6,
		ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_04000000_OVERLAY);

	// Label SCRB runner: 9151 (diff<=1) or 9153 (diff>1)
	int16 labelScrbId;
	if (kPuzzleLevel3 <= _difficultyLevel)
		labelScrbId = kResScrb9153_LabelHard;
	else
		labelScrbId = kResScrb9151_LabelEasy;
	_columnLabelFeature = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape9000_Slot), labelScrbId, 6,
		ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_04000000_OVERLAY);

	// Trait animation SCRB runner at 7018.
	// This runner is TOPMOST, not LOOP_ANIM.
	// TOPMOST keeps it out of the loop-animation bucket, so the seeded first sort places it
	// at the tail of the render list. That position is then frozen for the page lifetime
	// because @ref ZoombiniPuzzleNet::loadFeatures() disables positional z-sorting afterwards.
	_selectorAnimFeature = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape7000_Entry), kResScrb7018_TraitBase, 6,
		ZmbFeature::FLAG_00001000_TOPMOST | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_04000000_OVERLAY);
	if (_selectorAnimFeature)
		_selectorAnimFeature->setShapeOffsetRegs(_shapeOffsetRegsMap[kResRegs7000_Entry]);

	// Feedback SCRB runner at 10018
	_feedbackFeature = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape10000_Feedback), kResScrb10018_FeedbackComplete, 6,
		ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE);

	// Trait column SCRB runners at random offsets
	// 0 = OVERLAY | PLAY_ONCE | LOOP_ANIM
	if (kPuzzleLevel3 <= _difficultyLevel) {
		_selectorValueFeatures[0] = loadScrbFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape10000_Feedback), kResScrb10002_TraitColumn0Base + _selectedSelectorValues[0], 6,
			ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_04000000_OVERLAY);
	}
	_selectorValueFeatures[1] = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape10000_Feedback), kResScrb10007_TraitColumn1Base + _selectedSelectorValues[1], 6,
		ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_04000000_OVERLAY);
	_selectorValueFeatures[2] = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape10000_Feedback), kResScrb10012_TraitColumn2Base + _selectedSelectorValues[2], 6,
		ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_04000000_OVERLAY);

	registerV2SelectorHoverRunners();

	// Mud-tank SCRB feature at 7000.
	// SCRB 7000 is a DEFER_ANIM placeholder and never plays.
	// On the first frame tick, the exit step advances from 0 to 1 and loads SCRB 7001 as the first visible animation.
	_mudTankFeature = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape7000_Entry), kResScrb7000_ExitBase, 6,
		ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_04000000_OVERLAY);
	if (_mudTankFeature)
		_mudTankFeature->setShapeOffsetRegs(_shapeOffsetRegsMap[kResRegs7000_Entry]);
}

void ZoombiniPuzzleNet::registerV2SelectorHoverRunners() {
	if (!_vm->isVersionFamilyTlcV2())
		return;

	ZmbFeature::EventHooks hoverHooks;
	hoverHooks.setPreRenderFunc(static_cast<ZmbFeature::OnPreRenderFunc>(&ZoombiniPuzzleNet::selectorHover_preRender));
	hoverHooks.setRenderFunc(static_cast<ZmbFeature::OnRenderFunc>(&ZoombiniPuzzleNet::selectorHover_render));

	const uint32 selectorFlags = ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_04000000_OVERLAY;
	if (kPuzzleLevel3 <= _difficultyLevel) {
		_selectorHoverFeatures[0] = loadScrbFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape10000_Feedback),
			kResScrb10019_TraitColumn0HoverBase, 6, selectorFlags, hoverHooks);
	}
	_selectorHoverFeatures[1] = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape10000_Feedback),
		kResScrb10024_TraitColumn1HoverBase, 6, selectorFlags, hoverHooks);
	_selectorHoverFeatures[2] = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape10000_Feedback),
		kResScrb10029_TraitColumn2HoverBase, 6, selectorFlags, hoverHooks);

	// The submit hover outline is a stateless per-frame conditional render, like the selector
	// column hovers: it draws only while hovered and never runs an animation cycle. It therefore
	// shares their flag set instead of deferring animation, which would leave the runner without a
	// redraw request and keep the outline from painting when the hover begins.
	const uint32 submitFlags = ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_04000000_OVERLAY;
	_submitHoverFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape10000_Feedback), kResScrb10034_SubmitHover, 6, submitFlags, hoverHooks);
}

bool ZoombiniPuzzleNet::isV2SelectorHoverVisible(const ZmbFeature *feature) const {
	if (feature == _submitHoverFeature)
		return _submitHoverVisible;

	for (uint16 column = 0; column < ARRAYSIZE(_selectorHoverFeatures); column++) {
		if (feature == _selectorHoverFeatures[column])
			return 0 <= _hoveredSelectorValues[column];
	}
	return false;
}

bool ZoombiniPuzzleNet::selectorHover_preRender(ZmbFeature *feature) {
	return isV2SelectorHoverVisible(feature);
}

ZmbRenderResult ZoombiniPuzzleNet::selectorHover_render(ZmbFeature *feature) {
	if (!isV2SelectorHoverVisible(feature))
		return ZmbRenderResult::kSkipped;
	return blitShapes(feature);
}

void ZoombiniPuzzleNet::updateV2SelectorHoverRunners() {
	if (!_vm->isVersionFamilyTlcV2())
		return;

	const Common::Point mousePos = _selectorHoverMousePos;
	const int16 hoverBases[3] = {
		kResScrb10019_TraitColumn0HoverBase,
		kResScrb10024_TraitColumn1HoverBase,
		kResScrb10029_TraitColumn2HoverBase,
	};
	const uint16 clickRectBases[3] = {1, 6, 11};

	for (uint16 column = 0; column < ARRAYSIZE(_selectorHoverFeatures); column++) {
		int16 hoveredValue = -1;
		if (_selectorHoverFeatures[column] && (column != 0 || kPuzzleLevel3 <= _difficultyLevel)) {
			for (int16 value = 0; value < 5; value++) {
				if (_buttonClickRects[clickRectBases[column] + value].contains(mousePos) &&
					value != _selectedSelectorValues[column]) {
					hoveredValue = value;
					break;
				}
			}
		}

		if (_hoveredSelectorValues[column] != hoveredValue) {
			_hoveredSelectorValues[column] = hoveredValue;
			if (0 <= hoveredValue) {
				loadScrbOntoFeature(_selectorHoverFeatures[column], hoverBases[column] + hoveredValue);
				_selectorHoverFeatures[column]->requestVisualRematerialization();
			} else if (_selectorHoverFeatures[column]) {
				_selectorHoverFeatures[column]->requestVisualRematerialization();
			}
		}
	}

	const bool submitVisible = _submitHoverEnabled && _buttonClickRects[0].contains(mousePos);
	if (_submitHoverVisible != submitVisible) {
		_submitHoverVisible = submitVisible;
		if (_submitHoverFeature)
			_submitHoverFeature->requestVisualRematerialization();
	}
}

bool ZoombiniPuzzleNet::traitSlots_preRender(ZmbFeature *feature) {
	(void)feature;

	// The callback owns two fixed visual rectangles outside the virtual runner's empty click rect.
	// Merge them explicitly.
	if (_renderedGoButtonEnabled != _goButtonEnabled) {
		_renderedGoButtonEnabled = _goButtonEnabled;
		addExternalDirtyRect(Common::Rect(600, 441, 639, 478));
	}

	if (!_columnLabelInvalidatedThisFrame) {
		_columnLabelInvalidatedThisFrame = true;
		addExternalDirtyRect(Common::Rect(600, 403, 639, 440));
	}

	// Return true to continue with rendering
	return true;
}

ZoombiniGraphics::PaletteRemapMode ZoombiniPuzzleNet::getColorAssistPaletteRemap(const ZmbFeature *feature, const ZmbHotspot &hotspot, ZmbResource resource) const {
	(void)feature;
	if (!_vm->isColorBlindModeEnabled())
		return ZoombiniGraphics::kPaletteRemapNone;

	if (resource == ZmbResource(ZmbResource::kPage, kResBitmapShape10000_Feedback) &&
		24 <= hotspot._shapeIdx && hotspot._shapeIdx <= 33)
		return ZoombiniGraphics::kPaletteRemapNoseNet;

	// The trait remap callback resolves mudball components into shapes 1..184.
	// Later shapes in tBMP 7000 belong to the launcher and its moving parts.
	if (resource == ZmbResource(ZmbResource::kPage, kResBitmapShape7000_Entry) &&
		1 <= hotspot._shapeIdx && hotspot._shapeIdx < 185)
		return ZoombiniGraphics::kPaletteRemapNoseNet;

	return ZoombiniGraphics::kPaletteRemapNone;
}

ZmbRenderResult ZoombiniPuzzleNet::traitSlots_render(ZmbFeature *feature) {
	(void)feature;

	// Render the trait-slot button sprites:
	//   Slot 1: Label area (shape 5 from tBMP 6000) at (600, 403)
	//   Slot 2: Advance button (shape 1=off / 2=ready from tBMP 6000) at (600, 441)
	//
	// Slot 1 always uses shape 5; slot 2 reflects whether advancing is ready.

	ZmbResource shapeRes(ZmbResource::kPage, kResBitmapShape6000_Snoid);

	// Slot 1: Map/label button -- always shape 5.
	_vm->_gfx->drawShape(ZoombiniGraphics::kShapeScreen, shapeRes, 5, Common::Point(600, 403));

	// Slot 2 uses shape 2 when @ref ZoombiniPuzzleNet::_goButtonEnabled is true and shape 1 otherwise.
	uint16 goShapeIdx = _goButtonEnabled ? 2 : 1;
	_vm->_gfx->drawShape(ZoombiniGraphics::kShapeScreen, shapeRes, goShapeIdx, Common::Point(600, 441));

	// The advance flag tracks the last rendered state.
	// The label flag is a one-frame invalidation latch reset by this render callback.
	_columnLabelInvalidatedThisFrame = false;

	return ZmbRenderResult::kRendered;
}

void ZoombiniPuzzleNet::remapHotspotFramesByTrait(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)hsGroup;

	// Remap hotspot shape indices from the current selector values.
	// Use @ref ZoombiniPuzzleNet::kColOffsets1 and @ref ZoombiniPuzzleNet::kColOffsets2.
	// Also adjusts hotspot positions when @ref ZoombiniPuzzleNet::_positionShotHotspots enables the bounce animation.

	int16 activeSlotIdx = -1;
	for (int16 slotIdx = 0; slotIdx <= _lastShotFeatureIdx; slotIdx++) {
		if (_shotHistoryFeatures[slotIdx] == feature) {
			activeSlotIdx = slotIdx;
			break;
		}
	}

	const int16 *currentOffsets = _selectedSelectorValues;
	const int16 *previousOffsets = _previousSelectorValues;
	if (0 <= activeSlotIdx) {
		currentOffsets = _shotSelectorValues[activeSlotIdx];
		previousOffsets = _shotPreviousSelectorValues[activeSlotIdx];
	}

	// Map current and previous column offsets through lookup tables
	int16 mappedCurCol1 = -1;
	int16 mappedCurCol2 = -1;
	int16 mappedCurCol0 = -1;
	int16 mappedPrevCol1 = -1;
	int16 mappedPrevCol2 = -1;
	int16 mappedPrevCol0 = -1;

	if (currentOffsets[1] != -1)
		mappedCurCol1 = kColOffsets1[currentOffsets[1]];
	if (currentOffsets[2] != -1)
		mappedCurCol2 = kColOffsets2[currentOffsets[2]];
	if (currentOffsets[0] != -1)
		mappedCurCol0 = kColOffsets2[currentOffsets[0]];
	if (previousOffsets[1] != -1)
		mappedPrevCol1 = kColOffsets1[previousOffsets[1]];
	if (previousOffsets[2] != -1)
		mappedPrevCol2 = kColOffsets2[previousOffsets[2]];
	if (previousOffsets[0] != -1)
		mappedPrevCol0 = kColOffsets2[previousOffsets[0]];

	bool positionHotspots = _positionShotHotspots;
	bool useBounceOffsets = _bounceCounter != 0;
	int16 positionX = _bounceX;
	int16 positionY = _bounceY;
	if (0 <= activeSlotIdx) {
		positionHotspots = true;
		useBounceOffsets = activeSlotIdx == _lastShotFeatureIdx && _bounceCounter != 0;
		if (activeSlotIdx != _lastShotFeatureIdx || !_bounceCounter) {
			positionX = _shotHistoryPositions[activeSlotIdx].x;
			positionY = _shotHistoryPositions[activeSlotIdx].y;
		}
	}

	for (uint hotspotIdx = 0; hotspotIdx < hotspots.size(); hotspotIdx++) {
		int16 shapeIdx = hotspots[hotspotIdx]._shapeIdx;
		if (shapeIdx == 0)
			break;

		if (1 <= shapeIdx && shapeIdx < 185) {
			// Range 1-5: prev column offsets (both col1 and col0 mapped)
			if (shapeIdx < 6 && 0 <= mappedPrevCol1 && mappedPrevCol0 != -1) {
				hotspots[hotspotIdx]._shapeIdx = mappedPrevCol1 + 12 * mappedPrevCol0 + 6;
			}
			// Range 1-5: prev column offset (only col1 mapped, no col0)
			else if (shapeIdx < 6 && mappedPrevCol1 != -1 && mappedPrevCol0 == -1) {
				hotspots[hotspotIdx]._shapeIdx = mappedPrevCol1 + 1;
			}
			// Range 6-10: current column offsets (both col1 and col0 mapped)
			else if (6 <= shapeIdx && shapeIdx < 11 && 0 <= mappedCurCol1 && mappedCurCol0 != -1) {
				hotspots[hotspotIdx]._shapeIdx = shapeIdx + mappedCurCol1 + 12 * mappedCurCol0;
			}
			// Range 6-10: current column offset (only col1 mapped)
			else if (6 <= shapeIdx && shapeIdx < 11 && mappedCurCol1 != -1) {
				hotspots[hotspotIdx]._shapeIdx = mappedCurCol1 + 1;
			}
			// Range 11-17: cube plane offset (col0 mapped)
			else if (11 <= shapeIdx && shapeIdx < 18 && mappedCurCol0 != -1) {
				hotspots[hotspotIdx]._shapeIdx = shapeIdx + 12 * mappedCurCol0;
			}
			// Range 66-87: row offset (col2 mapped)
			else if (66 <= shapeIdx && shapeIdx < 88 && 0 <= mappedCurCol2) {
				hotspots[hotspotIdx]._shapeIdx = shapeIdx + 22 * mappedCurCol2;
			}
			// Range 176+: previous row offset.
			else if (176 <= shapeIdx && mappedPrevCol2 != -1) {
				hotspots[hotspotIdx]._shapeIdx = 22 * mappedPrevCol2 + 66;
			}

			// Apply bounce and slot-position offsets only within the 1-184 shape range.
			// Terminators and out-of-range helper slots are not moved.
			if (positionHotspots) {
				if (hotspotIdx == 0) {
					// First hotspot: base position
					hotspots[hotspotIdx]._x = positionX;
					hotspots[hotspotIdx]._y = positionY;
				} else if (useBounceOffsets) {
					// During bounce animation
					hotspots[hotspotIdx]._x = positionX + 4;
					hotspots[hotspotIdx]._y = positionY + 3;
				} else {
					// At rest in slot
					if (kPuzzleLevel3 <= _difficultyLevel)
						hotspots[hotspotIdx]._x = positionX + 3;
					else
						hotspots[hotspotIdx]._x = positionX + 21;
					hotspots[hotspotIdx]._y = positionY + 7;
				}
			}
		}
	}
}

void ZoombiniPuzzleNet::slotPreRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)hsGroup;

	// Add the launch-count indicator to the target feature's hotspot list.
	// Preserve the fixed two-shape slot layout by replacing hotspot 1 and discarding later entries
	// instead of appending another visible shape.

	for (int16 i = 0; i < _totalSlotCount; i++) {
		if (_targetSlotFeatures[i] == feature && 0 < _targetLaunchCounts[i]) {
			int16 shapeOffset = _targetLaunchCounts[i] + ((kPuzzleLevel3 <= _difficultyLevel) ? 153 : 150);
			if (hotspots.empty()) {
				error("net: malformed required slot SCRB hotspot table");
				return;
			}
			ZmbHotspot indicator(1, shapeOffset, hotspots[0]._frame, hotspots[0]._x, hotspots[0]._y);
			if (hotspots.size() < 2)
				hotspots.push_back(indicator);
			else
				hotspots[1] = indicator;
			hotspots.resize(2);
			return;
		}
	}
}

// ---------------------------------------------------------------------------
// Core puzzle logic functions
// ---------------------------------------------------------------------------

void ZoombiniPuzzleNet::computeTargetLaunchBatches() {
	// Distributes Zoombinis into repeating groups of 3, 2, 1.
	// Store the number of groups in @ref ZoombiniPuzzleNet::_targetCount.
	assert(0 < _pageLoadedZmbCount && _pageLoadedZmbCount <= static_cast<int16>(ARRAYSIZE(kSnoidPositions)));
	for (int16 i = 0; i < 12; i++)
		_targetLaunchBatchSizes[i] = 0;

	int16 remaining = _pageLoadedZmbCount;
	int16 size = 4;
	int16 groupCount = 0;

	do {
		size -= 1;
		if (size < 1)
			size = 3;
		_targetLaunchBatchSizes[groupCount] = size;
		groupCount += 1;
		remaining -= size;
	} while (0 < remaining);

	_targetCount = groupCount;

	// If we overshot (remaining < 0), trim sizes from columns >= 2
	if (remaining < 0) {
		int16 excess = -remaining;
		while (0 < excess) {
			for (int16 i = 0; i < _targetCount && 0 < excess; i++) {
				if (2 <= _targetLaunchBatchSizes[i]) {
					_targetLaunchBatchSizes[i] -= 1;
					excess -= 1;
				}
			}
			// Safety: if no column has size >= 2, set all to 1
			if (0 < excess) {
				int16 hasLarge = 0;
				for (int16 i = 0; i < _targetCount; i++) {
					if (1 < _targetLaunchBatchSizes[i])
						hasLarge += 1;
				}
				if (!hasLarge) {
					_targetCount = _pageLoadedZmbCount;
					for (int16 i = 0; i < _targetCount; i++)
						_targetLaunchBatchSizes[i] = 1;
					excess = 0;
				}
			}
		}
	}
}

void ZoombiniPuzzleNet::generateTraitRules() {
	// Generates the rule grids that define the puzzle solution.

	// Clear slot column assignments
	memset(_targetLaunchCounts, 0, sizeof(_targetLaunchCounts));
	memset(_initialTargetLaunchCounts, 0, sizeof(_initialTargetLaunchCounts));

	// Track used trait values for uniqueness
	int16 usedA[5] = {0, 0, 0, 0, 0};
	int16 usedB[5] = {0, 0, 0, 0, 0};
	int16 usedC[5] = {0, 0, 0, 0, 0};

	// Generate 5 unique trait combos
	for (int16 gridRowIdx = 0; gridRowIdx < 5; gridRowIdx++) {
		bool unique = false;
		int16 valA, valB, valC;
		do {
			valA = _vm->_rnd->getRandomNumber(0, 4);
			valB = _vm->_rnd->getRandomNumber(0, 4);
			valC = _vm->_rnd->getRandomNumber(0, 4);

			if (kPuzzleLevel3 <= _difficultyLevel) {
				// Levels 3-4 require each generated rule value to be unused.
				if (!usedA[valA] && !usedB[valB] && !usedC[valC])
					unique = true;
			} else {
				// Levels 1-2 require each accepted A and B value to be unused.
				if (!usedA[valA] && !usedB[valB])
					unique = true;
			}

			if (unique) {
				usedA[valA] += 1;
				usedB[valB] += 1;
				usedC[valC] += 1;
			}
		} while (!unique);

		// Populate grids
		if (_difficultyLevel < kPuzzleLevel3) {
			// 5x5 grid
			for (int16 gridCellIdx = 0; gridCellIdx < 5; gridCellIdx++) {
				_rowAxisRuleValues[5 * gridRowIdx + gridCellIdx] = valA;
				_columnAxisRuleValues[gridCellIdx * 5 + gridRowIdx] = valB;
			}
		} else {
			// 5x5x5 cube
			for (int16 gridCellIdx = 0; gridCellIdx < 25; gridCellIdx++)
				_rowAxisRuleValues[25 * gridRowIdx + gridCellIdx] = valA;
			for (int16 planeIdx = 0; planeIdx < 5; planeIdx++) {
				for (int16 columnIdx = 0; columnIdx < 5; columnIdx++)
					_columnAxisRuleValues[25 * planeIdx + 5 * gridRowIdx + columnIdx] = valB;
			}
			for (int16 rowIdx = 0; rowIdx < 5; rowIdx++) {
				for (int16 planeIdx = 0; planeIdx < 5; planeIdx++)
					_subcolumnAxisRuleValues[25 * planeIdx + 5 * rowIdx + gridRowIdx] = valB;
			}
		}
	}

	// Level 2 rotates rows in the second rule grid.
	if (_difficultyLevel == kPuzzleLevel2) {
		_axisRuleRotationStep = _vm->_rnd->getRandomNumber(0, 1) + 2;
		int16 tempBuf[5];
		for (int16 row = 0; row < 5; row++)
			tempBuf[row] = _columnAxisRuleValues[row];

		for (int16 row = 1; row < 5; row++) {
			for (int16 slotIdx = 0; slotIdx < _axisRuleRotationStep; slotIdx++) {
				int16 last = tempBuf[4];
				for (int16 bufferIdx = 4; 0 < bufferIdx; bufferIdx--)
					tempBuf[bufferIdx] = tempBuf[bufferIdx - 1];
				tempBuf[0] = last;
			}
			for (int16 col = 0; col < 5; col++)
				_columnAxisRuleValues[5 * row + col] = tempBuf[col];
		}
	}
	// Level 4 rotates columns in the first rule grid across the 3D cube.
	else if (_difficultyLevel == kPuzzleLevel4) {
		_axisRuleRotationStep = _vm->_rnd->getRandomNumber(0, 1) + 2;
		// Consume one value so later random choices retain the expected sequence.
		_vm->_rnd->getRandomNumber(0, 1);
		int16 tempBuf[5];

		for (int16 plane = 0; plane < 5; plane++) {
			for (int16 col = 1; col < 5; col++) {
				// This cascading shift reads plane z=col-1 and writes z=col.
				for (int16 rowIdx = 0; rowIdx < 5; rowIdx++)
					tempBuf[rowIdx] = _rowAxisRuleValues[25 * rowIdx + 5 * plane + col - 1];
				for (int16 slotIdx = 0; slotIdx < _axisRuleRotationStep; slotIdx++) {
					int16 last = tempBuf[4];
					for (int16 bufferIdx = 4; 0 < bufferIdx; bufferIdx--)
						tempBuf[bufferIdx] = tempBuf[bufferIdx - 1];
					tempBuf[0] = last;
				}
				for (int16 rowIdx = 0; rowIdx < 5; rowIdx++)
					_rowAxisRuleValues[25 * rowIdx + col + 5 * plane] = tempBuf[rowIdx];
			}
		}
	}

	// Distribute slots to columns -- assign column sizes to random slot positions
	int16 slotBase = (kPuzzleLevel3 <= _difficultyLevel) ? 25 : 0;
	for (int16 i = 0; i < _targetCount; i++) {
		int16 pos;
		do {
			pos = _vm->_rnd->getRandomNumber(0, (kPuzzleLevel3 <= _difficultyLevel) ? 124 : 24);
		} while (_targetLaunchCounts[pos] != 0);
		_targetLaunchCounts[pos] = _targetLaunchBatchSizes[i];
		_initialTargetLaunchCounts[pos] = _targetLaunchBatchSizes[i];
	}

	// Register slot SCRB runners for non-empty slots
	for (int16 i = 0; i < _totalSlotCount; i++) {
		_targetSlotFeatures[i] = nullptr;
		if (_targetLaunchCounts[i] != 0) {
			int16 scrbId = static_cast<int16>(i + slotBase + kResScrb9000_SlotBase);
			// Keep the slot indicator visible as a static overlay by activating the feature and immediately freezing it
			// on frame 0.
			_targetSlotFeatures[i] = loadScrbFeature(
				ZmbResource(ZmbResource::kPage, kResBitmapShape9000_Slot), scrbId, 6,
				ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_04000000_OVERLAY);

			if (_targetSlotFeatures[i]) {
				// Add the column indicator shape at the slot position.
				_targetSlotFeatures[i]->setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniPuzzleNet::slotPreRenderShape));
				// The REGS 9000/9001 resources provide registration-point offsets for indicator shape IDs (151-156).
				_targetSlotFeatures[i]->setShapeOffsetRegs(_shapeOffsetRegsMap[kResRegs9000_Slot]);
				// Freeze animation on frame 0: slot shows its static indicator permanently.
				// @ref ZmbFeature::activateAnimate() sets @ref ZmbFeature::_frameTimingReady.
				// @ref ZmbFeature::deactivateAnimate() preserves the timing result but stops frame advancement.
				_targetSlotFeatures[i]->deactivateAnimate();
				// Keep answer-cell indicators behind later mudballs and the launcher.
				if (_columnRouteFeatures[0])
					manualLinkBefore(_targetSlotFeatures[i], _columnRouteFeatures[0]);
			}
		}
	}

	// Generate trait labels
	if (kPuzzleLevel4 <= _difficultyLevel) {
		// All three labels must be distinct (0, 1, 2 in some order)
		do {
			_rowAxisSelectorIdx = _vm->_rnd->getRandomNumber(0, 2);
			_columnAxisSelectorIdx = _vm->_rnd->getRandomNumber(0, 2);
			_subcolumnAxisSelectorIdx = _vm->_rnd->getRandomNumber(0, 2);
		} while (_rowAxisSelectorIdx == _columnAxisSelectorIdx ||
				 _columnAxisSelectorIdx == _subcolumnAxisSelectorIdx ||
				 _subcolumnAxisSelectorIdx == _rowAxisSelectorIdx);
	} else {
		// Two labels: randomly swap row and column
		if (_vm->_rnd->getRandomBool()) {
			_rowAxisSelectorIdx = 2;
			_columnAxisSelectorIdx = 1;
		} else {
			_rowAxisSelectorIdx = 1;
			_columnAxisSelectorIdx = 2;
		}
		_subcolumnAxisSelectorIdx = 0;
	}

	// Trait permutation for difficulty 3
	if (kPuzzleLevel4 <= _difficultyLevel)
		_axisSelectorPermutationIdx = _vm->_rnd->getRandomNumber(0, 5);
	else
		_axisSelectorPermutationIdx = 0;
}

int16 ZoombiniPuzzleNet::findTargetSlotForSelection() {
	// Search the axis grids for the wall cell matching the live selector values.

	if (_difficultyLevel < kPuzzleLevel3) {
		// 2D lookup (two traits)
		for (int16 i = 0; i < _totalSlotCount; i++) {
			if (_rowAxisSelectorIdx == 2) {
				if (_rowAxisRuleValues[i] == _selectedSelectorValues[2] &&
					_columnAxisRuleValues[i] == _selectedSelectorValues[1])
					return i;
			} else {
				if (_columnAxisRuleValues[i] == _selectedSelectorValues[2] &&
					_rowAxisRuleValues[i] == _selectedSelectorValues[1])
					return i;
			}
		}
	} else {
		// 3D lookup (three traits) with 6 permutations
		for (int16 gridRowIdx = 0; gridRowIdx < _totalSlotCount; gridRowIdx++) {
			bool match = false;
			switch (_axisSelectorPermutationIdx) {
			case 0:
				match = (_rowAxisRuleValues[gridRowIdx] == _selectedSelectorValues[2] &&
						 _columnAxisRuleValues[gridRowIdx] == _selectedSelectorValues[1] &&
						 _subcolumnAxisRuleValues[gridRowIdx] == _selectedSelectorValues[0]);
				break;
			case 1:
				match = (_rowAxisRuleValues[gridRowIdx] == _selectedSelectorValues[1] &&
						 _columnAxisRuleValues[gridRowIdx] == _selectedSelectorValues[2] &&
						 _subcolumnAxisRuleValues[gridRowIdx] == _selectedSelectorValues[0]);
				break;
			case 2:
				match = (_rowAxisRuleValues[gridRowIdx] == _selectedSelectorValues[0] &&
						 _columnAxisRuleValues[gridRowIdx] == _selectedSelectorValues[2] &&
						 _subcolumnAxisRuleValues[gridRowIdx] == _selectedSelectorValues[1]);
				break;
			case 3:
				match = (_rowAxisRuleValues[gridRowIdx] == _selectedSelectorValues[2] &&
						 _columnAxisRuleValues[gridRowIdx] == _selectedSelectorValues[0] &&
						 _subcolumnAxisRuleValues[gridRowIdx] == _selectedSelectorValues[1]);
				break;
			case 4:
				match = (_rowAxisRuleValues[gridRowIdx] == _selectedSelectorValues[1] &&
						 _columnAxisRuleValues[gridRowIdx] == _selectedSelectorValues[0] &&
						 _subcolumnAxisRuleValues[gridRowIdx] == _selectedSelectorValues[2]);
				break;
			case 5:
				match = (_rowAxisRuleValues[gridRowIdx] == _selectedSelectorValues[0] &&
						 _columnAxisRuleValues[gridRowIdx] == _selectedSelectorValues[1] &&
						 _subcolumnAxisRuleValues[gridRowIdx] == _selectedSelectorValues[2]);
				break;
			default:
				break;
			}
			if (match)
				return gridRowIdx;
		}
	}
	return -1;
}

void ZoombiniPuzzleNet::updateSelectorValue(int16 selectorValue, int16 selectorGroup) {
	// Update one selector, or submit the current selection, and trigger its animations.

	// Check if all required columns are set (pre-check for submit validation)
	bool allColumnsSet = false;
	if (_difficultyLevel <= kPuzzleLevel2) {
		allColumnsSet = (0 <= _selectedSelectorValues[1] && 0 <= _selectedSelectorValues[2]);
	} else {
		allColumnsSet = (0 <= _selectedSelectorValues[0] && 0 <= _selectedSelectorValues[1] && 0 <= _selectedSelectorValues[2]);
	}

	if (selectorGroup == 0) {
		// Submit -- process current selection
		_previousSelectorValues[2] = _selectedSelectorValues[2];
		_previousSelectorValues[1] = _selectedSelectorValues[1];
		_previousSelectorValues[0] = _selectedSelectorValues[0];

		if (allColumnsSet) {
			// Play submit feedback sound
			loadScrbOntoFeature(_feedbackFeature, kResScrb10001_FeedbackReject);

			_submitHoverEnabled = false;
			_selectedTargetSlotIdx = findTargetSlotForSelection();

			// Compute column index and sort animation type
			int16 colIdx;
			if (kPuzzleLevel3 <= _difficultyLevel)
				colIdx = _selectedTargetSlotIdx % 25 / 5;
			else
				colIdx = _selectedTargetSlotIdx % 5;

			if (colIdx == 0)
				_sortAnimType = SortAnimationVariant::kLeft01;
			else if (1 <= colIdx && colIdx < 4)
				_sortAnimType = SortAnimationVariant::kCenter00;
			else
				_sortAnimType = SortAnimationVariant::kRight02;

			// Select SCRB 7028-7030 directly from this direction.
			// The separate 7020-7022 projectile path applies its own remap.
			int16 sortScrbId = static_cast<int16>(kResScrb7028_SortDirectBase + static_cast<int16>(_sortAnimType));
			loadScrbOntoFeature(_selectorAnimFeature, sortScrbId);
			if (_selectorAnimFeature) {
				_positionShotHotspots = false;
				_selectorAnimFeature->setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniPuzzleNet::remapHotspotFramesByTrait));
				_selectorAnimFeature->setFrameInterval(6);
				_sortAnimRunning = true;
			}
		}
	} else if (selectorGroup == 1) {
		// Column 0 selector (only at difficulty 3+)
		if (kPuzzleLevel3 <= _difficultyLevel) {
			_previousSelectorValues[1] = _selectedSelectorValues[1];
			_previousSelectorValues[0] = _selectedSelectorValues[0];
			_selectedSelectorValues[0] = selectorValue;

			if (_previousSelectorValues[0] != selectorValue) {
				loadScrbOntoFeature(_selectorValueFeatures[0], kResScrb10002_TraitColumn0Base + selectorValue);
				if (!_rejectedShotCount && allColumnsSet) {
					loadScrbOntoFeature(_selectorAnimFeature, kResScrb7027_TraitBase);
					if (_selectorAnimFeature) {
						_positionShotHotspots = false;
						_selectorAnimFeature->setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniPuzzleNet::remapHotspotFramesByTrait));
					}
					if (!_submissionActivityCount)
						loadScrbOntoFeature(_feedbackFeature, kResScrb10018_FeedbackComplete);
				}
			}
		}
	} else if (selectorGroup == 2) {
		// Column 1 selector
		_previousSelectorValues[1] = _selectedSelectorValues[1];
		_selectedSelectorValues[1] = selectorValue;
		_previousSelectorValues[0] = _selectedSelectorValues[0];

		if (_previousSelectorValues[1] != selectorValue) {
			loadScrbOntoFeature(_selectorValueFeatures[1], kResScrb10007_TraitColumn1Base + selectorValue);
			if (!_rejectedShotCount && allColumnsSet) {
				loadScrbOntoFeature(_selectorAnimFeature, kResScrb7019_TraitBase);
				if (_selectorAnimFeature) {
					_selectorAnimFeature->setFrameInterval(3);
					_positionShotHotspots = false;
					_selectorAnimFeature->setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniPuzzleNet::remapHotspotFramesByTrait));
				}
				if (!_submissionActivityCount)
					loadScrbOntoFeature(_feedbackFeature, kResScrb10018_FeedbackComplete);
			}
		}
	} else if (selectorGroup == 3) {
		// Column 2 selector
		_previousSelectorValues[2] = _selectedSelectorValues[2];
		_previousSelectorValues[1] = _selectedSelectorValues[1];
		_previousSelectorValues[0] = _selectedSelectorValues[0];
		_selectedSelectorValues[2] = selectorValue;

		if (_previousSelectorValues[2] != selectorValue) {
			loadScrbOntoFeature(_selectorValueFeatures[2], kResScrb10012_TraitColumn2Base + selectorValue);
			if (!_rejectedShotCount && allColumnsSet) {
				loadScrbOntoFeature(_selectorAnimFeature, kResScrb7026_TraitBase);
				if (_selectorAnimFeature) {
					_positionShotHotspots = false;
					_selectorAnimFeature->setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniPuzzleNet::remapHotspotFramesByTrait));
				}
				if (!_submissionActivityCount)
					loadScrbOntoFeature(_feedbackFeature, kResScrb10018_FeedbackComplete);
			}
		}
	}

	// If columns not yet ready, trigger column setup
	if (!_submitHoverEnabled && !_rejectedShotCount) {
		_selectorSetupPendingCount += 1;
		_submitHoverEnabled = true;
	}
}

void ZoombiniPuzzleNet::assignNextSnoidToColumn() {
	// Find an empty column slot and assign the next Snoid.

	for (int16 columnSlotIdx = 2; 0 <= columnSlotIdx; columnSlotIdx--) {
		if (_columnSlotSnoidIds[columnSlotIdx] != 0)
			continue;

		if (_pageLoadedZmbCount <= _nextZmbToAssign) {
			_packAssignmentExhausted = true;
			continue;
		}

		if (!_columnAssignmentReadyCount)
			return;

		_queuedColumnAssignmentCount -= 1;
		if (_queuedColumnAssignmentCount <= 0)
			_columnAssignmentReadyCount = 0;

		uint16 snoidId = 10000 + _nextZmbToAssign;
		ZmbSnoid *snoid = getSnoid(snoidId);
		if (snoid) {
			// Enter state 10 before storing the fixed staging target.
			// State 10 materializes the common-image pose immediately.
			// It changes to state 7 on its next animation tick.
			// State 10 also replaces any provisional 13001-series SCRS before display.
			snoid->setAnimState(kSnoidAnimState010_ArrivalMotion);
			snoid->setAnimTargetPos(Common::Point(233, 392));
			_pendingZmbIndex = _nextZmbToAssign;
			_columnSlotSnoidIds[columnSlotIdx] = snoidId;
			_nextZmbToAssign += 1;
			_activeColumnIdx = columnSlotIdx;
		}
		return;
	}

	_queuedColumnAssignmentCount = 0;
	_columnAssignmentReadyCount = 0;
}

void ZoombiniPuzzleNet::settleShotAtTargetSlot(int16 slotIndex) {
	// Settle the fired-shot display at the selected wall cell and commit its result.

	if (slotIndex < 0)
		return;

	// Get position from appropriate table
	if (kPuzzleLevel3 <= _difficultyLevel) {
		_bounceX = kSlotPositionsHigh[slotIndex].x;
		_bounceY = kSlotPositionsHigh[slotIndex].y;
	} else {
		_bounceX = kSlotPositionsLow[slotIndex].x;
		_bounceY = kSlotPositionsLow[slotIndex].y;
	}

	_positionShotHotspots = true;
	_shotHistoryPositions[_lastShotFeatureIdx] = Common::Point(_bounceX, _bounceY);

	// Load slot display SCRB
	int16 scrbId = (kPuzzleLevel3 <= _difficultyLevel) ? kResScrb7024_SlotDisplayHard : kResScrb7023_SlotDisplayEasy;
	if (_shotHistoryFeatures[_lastShotFeatureIdx]) {
		loadScrbOntoFeature(_shotHistoryFeatures[_lastShotFeatureIdx], scrbId);
	} else {
		_shotHistoryFeatures[_lastShotFeatureIdx] = loadScrbFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape7000_Entry), scrbId, 6,
			ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_04000000_OVERLAY);
		if (_shotHistoryFeatures[_lastShotFeatureIdx])
			_shotHistoryFeatures[_lastShotFeatureIdx]->setShapeOffsetRegs(_shapeOffsetRegsMap[kResRegs7000_Entry]);
	}
	if (_shotHistoryFeatures[_lastShotFeatureIdx]) {
		_shotHistoryFeatures[_lastShotFeatureIdx]->setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniPuzzleNet::remapHotspotFramesByTrait));
		if (_columnRouteFeatures[0])
			manualLinkBefore(_shotHistoryFeatures[_lastShotFeatureIdx], _columnRouteFeatures[0]);
	}

	// Score tracking
	_shotMissedTarget = false;
	_remainingLaunchesForShot = _targetLaunchCounts[_selectedTargetSlotIdx];
	_acceptedLaunchGatePending = 1 <= _remainingLaunchesForShot;
	if (1 <= _remainingLaunchesForShot) {
		_idleAnimTrigger = true;
	} else {
		_remainingLaunchesForShot = 0;
		_submissionActivityCount = 0;
		_shotMissedTarget = true;
	}
	_acceptedZmbCount += _remainingLaunchesForShot;
	_targetLaunchCounts[slotIndex] = -1;
}

void ZoombiniPuzzleNet::startShotAtTargetSlot(int16 slotIndex) {
	// Start the fired-shot animation toward the selected wall cell.

	if (_bounceCounter)
		return;

	_selectedTargetSlotIdx = slotIndex;
	_bounceCounter = 1;

	// Get target position
	if (kPuzzleLevel3 <= _difficultyLevel) {
		_bounceX = kSlotPositionsHigh[slotIndex].x;
		_bounceY = kSlotPositionsHigh[slotIndex].y;
	} else {
		_bounceX = kSlotPositionsLow[slotIndex].x;
		_bounceY = kSlotPositionsLow[slotIndex].y;
	}

	// Compute bounce deltas from start point (484, 318)
	_bounceDeltaX = (484 - _bounceX) / 6;
	_bounceDeltaY = (318 - _bounceY) / 6;
	_bounceX = 484;
	_bounceY = 318;

	_lastShotFeatureIdx += 1;
	if (kPuzzleLevel3 <= _difficultyLevel)
		_shotHistoryPositions[_lastShotFeatureIdx] = kSlotPositionsHigh[slotIndex];
	else
		_shotHistoryPositions[_lastShotFeatureIdx] = kSlotPositionsLow[slotIndex];
	for (int16 i = 0; i < 3; i++) {
		_shotSelectorValues[_lastShotFeatureIdx][i] = _selectedSelectorValues[i];
		_shotPreviousSelectorValues[_lastShotFeatureIdx][i] = _previousSelectorValues[i];
	}

	// Load sort display SCRB with bounce pre-render
	static constexpr uint16 kSortScrbLookup[3] = {1, 0, 2};
	int16 sortScrbId = static_cast<int16>(kResScrb7020_SortBase + kSortScrbLookup[static_cast<uint>(_sortAnimType)]);

	_shotHistoryFeatures[_lastShotFeatureIdx] = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape7000_Entry), sortScrbId, 6,
		ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_04000000_OVERLAY);

	if (_shotHistoryFeatures[_lastShotFeatureIdx]) {
		_shotHistoryFeatures[_lastShotFeatureIdx]->setShapeOffsetRegs(_shapeOffsetRegsMap[kResRegs7000_Entry]);
		_positionShotHotspots = true;
		_shotHistoryFeatures[_lastShotFeatureIdx]->setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniPuzzleNet::remapHotspotFramesByTrait));
		_shotHistoryFeatures[_lastShotFeatureIdx]->setFrameInterval(3);
		if (_selectorAnimFeature)
			manualLinkBefore(_shotHistoryFeatures[_lastShotFeatureIdx], _selectorAnimFeature);
	}
}

void ZoombiniPuzzleNet::releaseSelectorGateAfterTraitAnimation() {
	if (_acceptedLaunchGatePending) {
		_selectorGateReleasePending = true;
		return;
	}

	_selectorGateReleasePending = false;
	_selectorInputEnabled = true;
}

// ---------------------------------------------------------------------------
// Click handling
// ---------------------------------------------------------------------------

void ZoombiniPuzzleNet::onMapButtonActivated() {
	if (!_vm->isDemo()) {
		ZoombiniPuzzle::onMapButtonActivated();
		return;
	}

	_vm->setNextPage(ZoombiniPageType::kLogo);
	close();
}

ZmbEventHandleResult ZoombiniPuzzleNet::onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) {
	// Dispatch hotspot IDs 1-19 through fixed button click rectangles.
	ZmbEventHandleResult result = ZoombiniInteractive::onLButtonDown(absPos, relPos);
	if (result == ZmbEventHandleResult::kConsumed)
		return result;

	if (!_puzzleInputEnabled)
		return ZmbEventHandleResult::kPassthrough;

	// Hit-test against the fixed button click rectangles.
	int16 buttonIdx = -1;
	for (int16 buttonRectIdx = 0; buttonRectIdx < 16; buttonRectIdx++) {
		if (_buttonClickRects[buttonRectIdx].contains(absPos)) {
			buttonIdx = buttonRectIdx;
			break;
		}
	}

	if (buttonIdx < 0)
		return ZmbEventHandleResult::kPassthrough;

	if (buttonIdx == 0) {
		// Submit button (hotspot 4)
		if (!_submissionActivityCount && _selectorInputEnabled && !_rejectedShotCount) {
			_acceptedLaunchGatePending = false;
			_selectorGateReleasePending = false;
			_submissionActivityCount += 1;
			_selectorInputEnabled = false;
			_lastSubmitFrame = getCurrentFrameCounter();
			updateSelectorValue(0, 0);
		}
		return ZmbEventHandleResult::kConsumed;
	}

	// Column selector buttons (hotspots 5-19)
	// buttonIdx 1-5 = column 0, 6-10 = column 1, 11-15 = column 2
	int16 colIdx = (buttonIdx - 1) / 5; // 0, 1, or 2
	int16 value = (buttonIdx - 1) % 5;  // 0-4

	// Column 0 only active at difficulty >= 3
	if (colIdx == 0 && _difficultyLevel <= kPuzzleLevel2)
		return ZmbEventHandleResult::kPassthrough;

	if (!_selectorInputEnabled)
		return ZmbEventHandleResult::kConsumed;

	updateSelectorValue(value, colIdx + 1);
	return ZmbEventHandleResult::kConsumed;
}

ZmbEventHandleResult ZoombiniPuzzleNet::onMouseMove(const Common::Point &absPos, const Common::Point &relPos) {
	_selectorHoverMousePos = absPos;
	updateV2SelectorHoverRunners();
	return ZoombiniInteractive::onMouseMove(absPos, relPos);
}

// ---------------------------------------------------------------------------
// Animation event dispatch
// ---------------------------------------------------------------------------

void ZoombiniPuzzleNet::onFeatureAnimEvent(ZmbFeature *feature, int16 eventCode) {
	// Dispatch every NET animation event through this callback.
	if (feature->hasFlag(ZmbFeature::FLAG_00000001_TYPE_SNOID)) {
		processSnoidAnimEvent(feature, eventCode);
	} else {
		processZmbScrbAnimEvent(feature, eventCode);
	}
}

bool ZoombiniPuzzleNet::startVisibleNormalScrs(ZmbSnoid *snoid, int16 scrsId, const Common::Point *endPos) {
	if (!snoid)
		return false;

	// Select the render state from the registered SCRS group instead of hardcoding it.
	// Group 1 contains SCRS 13000-13050 for walking, seating, launching, and idling; it uses state 8.
	// Group 0 contains entry SCRS 14000-14002; it uses state 9.
	// The end position is the initial-position pointer anchor:
	// the last visible SCRS frame lands there instead of teleporting frame 0.
	return startSnoidScrs(snoid, ZmbResource(ZmbResource::kPage, scrsId), ZmbScrsCompletionMode::kReturnToIdle, endPos);
}

void ZoombiniPuzzleNet::linkActiveSnoidAboveColumns(ZmbSnoid *snoid) {
	if (!snoid || _lastShotFeatureIdx < 0 || 24 < _lastShotFeatureIdx)
		return;

	ZmbFeature *activeSlot = _shotHistoryFeatures[_lastShotFeatureIdx];
	if (!activeSlot || !_columnRouteFeatures[0] || !_columnRouteFeatures[1] || !_columnRouteFeatures[2])
		return;

	// Insert each target immediately after its parent so the active Snoid stays above the full lever and column chain
	// while leaving the waiting area.
	manualLinkAfter(_columnRouteFeatures[0], activeSlot);
	manualLinkAfter(_columnRouteFeatures[1], _columnRouteFeatures[0]);
	manualLinkAfter(_columnRouteFeatures[2], _columnRouteFeatures[1]);
	manualLinkAfter(snoid, _columnRouteFeatures[2]);
}

void ZoombiniPuzzleNet::linkAcceptedSnoid(ZmbSnoid *snoid) {
	if (!snoid)
		return;

	// Insert each new arrival immediately before the previous one.
	// Celebration playback must not change this order.
	if (_lastAcceptedSnoidId) {
		ZmbSnoid *previousSnoid = getSnoid(_lastAcceptedSnoidId);
		if (previousSnoid)
			manualLinkBefore(snoid, previousSnoid);
	}
	_lastAcceptedSnoidId = snoid->getId();
}

void ZoombiniPuzzleNet::processSnoidAnimEvent(ZmbFeature *feature, int16 eventCode) {
	// Snoid-specific events
	ZmbSnoid *snoid = static_cast<ZmbSnoid *>(feature);

	if (eventCode == kAnimEventM1_End) {
		// End of animation
		if (_acceptedExitCompletionCount) {
			// Exit phase: position zoombini at exit point
			if (_acceptedExitPositionIdx < 16) {
				Common::Point exitPos = kExitPositions[_acceptedExitPositionIdx];
				_acceptedExitPositionIdx += 1;
				ZmbSnoid *activeSnoid = getSnoid(_launchingSnoidId);
				if (activeSnoid) {
					// @ref ZmbSnoid::initWalkToTarget() sets @ref ZmbSnoid::_animTargetPos.
					// It then enters @ref kSnoidAnimState007_Depart.
					activeSnoid->initWalkToTarget(exitPos);
					activeSnoid->activateRender();
					activeSnoid->_packIsOccupied = true;
				}
				_goButtonEnabled = true;
				setGoButtonsEnabled(true);
				_acceptedExitCompletionCount = 0;
				_acceptedLaunchGatePending = false;
				if (_selectorGateReleasePending) {
					_selectorGateReleasePending = false;
					_selectorInputEnabled = true;
				}

				if (_pageLoadedZmbCount <= _acceptedZmbCount || _packAssignmentExhausted) {
					_submissionActivityCount = 0;
					_selectorInputEnabled = true;
					_selectorGateReleasePending = false;
				}

				// Check if all zoombinis are done
				if (!_queuedColumnAssignmentCount &&
					!_columnSlotSnoidIds[0] && !_columnSlotSnoidIds[1] && !_columnSlotSnoidIds[2]) {
					if (_pageLoadedZmbCount <= _acceptedZmbCount) {
						queueCompletionNarratorSound();
					}
					_roundCompleted = true;
					_idleAnimTrigger = true;
					_idleAnimMax = _pageLoadedZmbCount;
				}

				// Process pending hint
				if (_partialSuccessNarrationPending) {
					_partialSuccessNarrationPending = false;
					if (1 <= _acceptedZmbCount && _acceptedZmbCount < _pageLoadedZmbCount) {
						queuePartialSuccessNarratorSound();
					}
				}
			}
		} else {
			// Clear the matching walk slot and turn the departing Snoid left.
			for (int16 walkSlotIdx = 2; 0 <= walkSlotIdx; walkSlotIdx--) {
				if (_travelingColumnSnoidIds[walkSlotIdx] == feature->getId()) {
					ZmbSnoid *walkSnoid = getSnoid(_travelingColumnSnoidIds[walkSlotIdx]);
					_travelingColumnSnoidIds[walkSlotIdx] = 0;
					if (walkSnoid) {
						walkSnoid->setAnimState(kSnoidAnimState002_TurnRight);
						walkSnoid->activateRender();
						if (!_travelingColumnSnoidIds[0] && !_travelingColumnSnoidIds[1] && !_travelingColumnSnoidIds[2]) {
							loadScrbOntoFeature(_feedbackFeature, kResScrb10018_FeedbackComplete);
							_submissionActivityCount = 0;
							_selectorInputEnabled = true;
						}
					}
					return;
				}
			}
			// Fallback: check if queue is empty
			if (!_queuedColumnAssignmentCount && !_columnSlotSnoidIds[0]) {
				_submissionActivityCount = 0;
				_selectorInputEnabled = true;
			}
		}
		return;
	}

	switch (eventCode) {
	case kNetEventCode000_ToggleFacing:
		// The Snoid SCRS reached its facing marker.
		// Toggle only the logical facing and then apply a pending body arrangement.
		// Flip only the Snoid's facing direction.
		// This does not change render-enabled state or touch any visible body layer.
		// Toggling a hotspot shape here would garble the Snoid's traits.
		flipEventFacing(feature);
		applyPendingBodyArrangement(*snoid);
		break;
	case kNetEventCode002_SpawnSnoidSlot:
		// The seating script reached the marker that materializes the Snoid at the current net slot.
		// Spawn zoombini SCRB at the current slot.
		// Same handler for snoid and SCRB callers.
		// Seating SCRS scripts (13016-13030) can fire this from snoid context.
		startShotAtTargetSlot(_selectedTargetSlotIdx);
		break;
	case kNetEventCode004_StartColumnTravel: {
		// The seating script finished its entry pose.
		// Start the feet-specific normal SCRS that carries the Snoid into its target column.
		// Start snoid travel to column -- play entry SCRS.
		// Fires from seating SCRS (13016-13018) via the 0xFF05 terminator (raw 5, adjusted 4) at the last frame.
		// Operate on the column Snoid in @ref ZoombiniPuzzleNet::_launchingSnoidId, not the calling feature.
		ZmbSnoid *activeSnoid = getSnoid(_launchingSnoidId);
		if (activeSnoid) {
			Common::Point entryPos = kEntryStartPositions[_activeColumnIdx];
			int16 scrsId = static_cast<int16>(_activeColumnIdx + kResScrs14000_EntryBase);
			startVisibleNormalScrs(activeSnoid, scrsId, &entryPos);
			linkActiveSnoidAboveColumns(activeSnoid);
		}
		break;
	}
	case kNetEventCode030_StartColumnExit:
		// The active column runner reached its exit marker.
		// Start the exit SCRS and link the accepted Snoid into the completed chain.
		// Column exit: play exit SCRS at entry exit position
		if (_launchingSnoidId) {
			Common::Point exitPos = kEntryExitPositions[_activeColumnIdx];
			ZmbSnoid *activeSnoid = getSnoid(_launchingSnoidId);
			if (activeSnoid) {
				int16 scrsId = static_cast<int16>(_activeColumnIdx + 3 * (activeSnoid->_trait._feet - 1) + kResScrs13031_NormalLaunchBase);
				startVisibleNormalScrs(activeSnoid, scrsId, &exitPos);
				linkAcceptedSnoid(activeSnoid);
			}
			_acceptedExitCompletionCount += 1;
		}
		break;
	default:
		handleBodyArrangementScriptEvent(*snoid, eventCode);
		break;
	}
}

void ZoombiniPuzzleNet::flipEventFacing(ZmbFeature *feature) {
	// Event 0 turns Snoids around; it does not change render visibility or body layers.
	// Non-Snoid callers such as thrown mudballs ignore this event.
	if (!feature || !feature->hasFlag(ZmbFeature::FLAG_00000001_TYPE_SNOID))
		return;

	ZmbSnoid *snoid = static_cast<ZmbSnoid *>(feature);
	snoid->setFacingLeft(!snoid->isFacingLeft());
	snoid->setNeedsRedraw(true);
}

void ZoombiniPuzzleNet::processZmbScrbAnimEvent(ZmbFeature *feature, int16 eventCode) {
	// These routed SCRB events update the active Snoid and column-slot state rather than the calling feature itself.
	switch (eventCode) {
	case kNetEventCode000_ToggleFacing:
		// A routed SCRB requested the same facing/body-arrangement operation as a Snoid SCRS.
		flipEventFacing(feature);
		break;
	case kNetEventCode002_SpawnSnoidSlot:
		// The routed SCRB reached its slot-spawn marker.
		// Spawn zoombini SCRB at the current slot
		startShotAtTargetSlot(_selectedTargetSlotIdx);
		break;
	case kNetEventCode004_StartColumnTravel: {
		// The routed SCRB handed the active Snoid to the column-entry SCRS.
		// Start snoid travel to column -- play entry SCRS
		ZmbSnoid *activeSnoid = getSnoid(_launchingSnoidId);
		if (activeSnoid) {
			Common::Point entryPos = kEntryStartPositions[_activeColumnIdx];
			int16 scrsId = static_cast<int16>(_activeColumnIdx + kResScrs14000_EntryBase);
			startVisibleNormalScrs(activeSnoid, scrsId, &entryPos);
			linkActiveSnoidAboveColumns(activeSnoid);
		}
		break;
	}
	case kNetEventCode020_StartColumnPositioning: {
		// The column positioning marker selects the next waiting Snoid and its feet-specific SCRS.
		// Activate runner at column slot -- play positioning SCRS
		_launchingSnoidId = _columnSlotSnoidIds[_activeColumnIdx];
		ZmbSnoid *colSnoid = getSnoid(_launchingSnoidId);
		if (colSnoid) {
			int16 scrsId = static_cast<int16>(3 * (colSnoid->_trait._feet - 1) + 2 - _activeColumnIdx + kResScrs13016_NormalSeatBase);
			colSnoid->_packIsOccupied = true;
			startVisibleNormalScrs(colSnoid, scrsId);
			_pendingZmbIndex = _nextZmbToAssign;
			_launchingSnoidId = _columnSlotSnoidIds[_activeColumnIdx];
			_columnSlotSnoidIds[_activeColumnIdx] = 0;
			if (!_packAssignmentExhausted && _nextZmbToAssign < _pageLoadedZmbCount)
				_queuedColumnAssignmentCount += 1;
		}
		break;
	}
	case kNetEventCode030_StartColumnExit: {
		// The routed SCRB reached the column-exit marker and delegates to the accepted-chain path.
		// Column exit from SCRB context -- delegates to snoid event 30 logic
		if (_launchingSnoidId) {
			Common::Point exitPos = kEntryExitPositions[_activeColumnIdx];
			ZmbSnoid *activeSnoid = getSnoid(_launchingSnoidId);
			if (activeSnoid) {
				int16 scrsId = static_cast<int16>(_activeColumnIdx + 3 * (activeSnoid->_trait._feet - 1) + kResScrs13031_NormalLaunchBase);
				startVisibleNormalScrs(activeSnoid, scrsId, &exitPos);
				linkAcceptedSnoid(activeSnoid);
			}
			_acceptedExitCompletionCount += 1;
		}
		break;
	}
	case kAnimEventM1_End:
		// @ref ZoombiniPuzzleNet::onPostRenderFrame() polls @ref ZmbFeature::hasAnimEndCallbackFired() for SCRB completion.
		break;
	default:
		break;
	}
}

// ---------------------------------------------------------------------------
// Schedule idle animation once per frame.
// ---------------------------------------------------------------------------
void ZoombiniPuzzleNet::onPostRenderFrame() {
	// Main per-frame state machine.

	updateV2SelectorHoverRunners();
	finishMudTankVisualAnimation();

	// Phase 1: Initial mud-tank fill sequence.
	// SCRB 7000 is only a placeholder.
	// The first frame advances immediately to step 1 and loads SCRB 7001.
	// Subsequent SCRBs play with PLAY_ONCE and advance when complete.
	if (_mudTankIntroActive && _mudTankIntroStepActive) {
		// Step 0 uses a deferred SCRB 7000 placeholder, so advance immediately.
		// Steps >= 1 wait for PLAY_ONCE animation to complete.
		bool shouldAdvance = (_mudTankIntroStep == 0) || _mudTankFeature->hasAnimEndCallbackFired();

		if (shouldAdvance) {
			debugC(4, MohawkEngine_Zoombini::kDebugPage02, "net: advancing mud-tank intro step %d of %d", _mudTankIntroStep, _initialShotAllowance);
			_mudTankIntroStepActive = false;
			_mudTankIntroStep += 1;
			if (_initialShotAllowance <= _mudTankIntroStep) {
				_mudTankIntroActive = false;
				debugC(2, MohawkEngine_Zoombini::kDebugPage02, "net: completed mud-tank intro");
				_queuedColumnAssignmentCount = 3;
				_columnAssignmentReadyCount += 1;
				assignNextSnoidToColumn();
				_selectorSetupPendingCount = 1;
				_submitHoverEnabled = true;
			} else {
				loadScrbOntoFeature(_mudTankFeature, _mudTankIntroStep + kResScrb7000_ExitBase);
				_mudTankIntroStepActive = true;
			}
		}
	}

	// Phase 2: Label animation -> generate rules
	if (_columnLabelIntroRunning && _columnLabelFeature->hasAnimEndCallbackFired()) {
		_columnLabelIntroRunning = false;
		debugC(2, MohawkEngine_Zoombini::kDebugPage02, "net: generating trait rules after column-label intro");
		generateTraitRules();
	}

	// Phase 3: Sort animation completion (submit result)
	if (_sortAnimRunning) {
		if (_selectorAnimFeature->hasAnimEndCallbackFired()) {
			_sortAnimRunning = false;
			_remainingShotAllowance -= 1;
			if (_remainingShotAllowance < 0) {
				_rejectedShotCount += 1;
				_selectorInputEnabled = true;
				if (passesPartialResultFeedbackGate()) {
					if (1 <= _remainingLaunchesForShot) {
						if (_nextZmbToAssign < _pageLoadedZmbCount)
							_partialSuccessNarrationPending = true;
					} else if (1 <= _acceptedZmbCount && _acceptedZmbCount < _pageLoadedZmbCount) {
						queuePartialSuccessNarratorSound();
					}
				}
			}
			if (_remainingLaunchesForShot < 1)
				_remainingLaunchesForShot = 0;
			else
				_columnOpenPendingCount += 1;
		}
		goto label_postColumn;
	}

	// Phase 4: Pending column setup
	if (_selectorSetupPendingCount && !_rejectedShotCount) {
		bool allSet;
		if (_difficultyLevel <= kPuzzleLevel2)
			allSet = (0 <= _selectedSelectorValues[1] && 0 <= _selectedSelectorValues[2]);
		else
			allSet = (0 <= _selectedSelectorValues[0] && 0 <= _selectedSelectorValues[1] && 0 <= _selectedSelectorValues[2]);

		if (allSet) {
			// Finish the complete debugger target before normal gameplay consumes
			// one additional tank level for the fired mudball.
			if (_mudTankAdjustmentPending || _mudTankAnimActive)
				goto label_postColumn;

			_selectorSetupPendingCount = 0;
			debugC(2, MohawkEngine_Zoombini::kDebugPage02, "net: starting selector reveal with SCRB %d", _mudTankScrbOffset + kResScrb7031_ExitBase);
			startMudTankVisualAnimation(_mudTankScrbOffset + kResScrb7031_ExitBase, CLIP<int16>(static_cast<int16>(15 - _mudTankScrbOffset), 0, 16), false);
			_selectorRevealStartPending = true;
		}
		goto label_postColumn;
	}

	// Phase 5: Pending trait runner (one-shot trigger)
	if (_selectorRevealStartPending) {
		debugC(2, MohawkEngine_Zoombini::kDebugPage02, "net: starting selector trait reveal");
		_selectorRevealStartPending = false;
		_previousSelectorValues[1] = _selectedSelectorValues[1];
		_previousSelectorValues[2] = -1;
		_previousSelectorValues[0] = -1;
		if (_difficultyLevel <= kPuzzleLevel2)
			_selectedSelectorValues[0] = -1;
		if (!_rejectedShotCount) {
			loadScrbOntoFeature(_selectorAnimFeature, kResScrb7018_TraitBase);
			if (_selectorAnimFeature) {
				_positionShotHotspots = false;
				_selectorAnimFeature->setFrameInterval(3);
				_selectorAnimFeature->setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniPuzzleNet::remapHotspotFramesByTrait));
				_selectorRevealScrb7018Running = true;
			}
		}
		goto label_postColumn;
	}

	// Phase 6: Active trait animation
	if (_selectorRevealScrb7018Running) {
		if (_selectorAnimFeature->hasAnimEndCallbackFired()) {
			debugC(2, MohawkEngine_Zoombini::kDebugPage02, "net: completed first trait reveal and loading SCRB 7025");
			_selectorRevealScrb7018Running = false;
			assignNextSnoidToColumn();
			loadScrbOntoFeature(_selectorAnimFeature, kResScrb7025_TraitBase);
			if (_selectorAnimFeature) {
				_selectorAnimFeature->setFrameInterval(2);
				_selectorRevealScrb7025Running = true;
			}
		}
		goto label_postColumn;
	}

	// Phase 7: Trait anim 1
	if (_selectorRevealScrb7025Running) {
		if (_selectorAnimFeature->hasAnimEndCallbackFired()) {
			debugC(2, MohawkEngine_Zoombini::kDebugPage02, "net: completed second trait reveal and loading SCRB 7026");
			_selectorRevealScrb7025Running = false;
			loadScrbOntoFeature(_selectorAnimFeature, kResScrb7026_TraitBase);
			if (_selectorAnimFeature) {
				_positionShotHotspots = false;
				_selectorAnimFeature->setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniPuzzleNet::remapHotspotFramesByTrait));
				_selectorAnimFeature->setFrameInterval(6);
				_selectorRevealScrb7026Running = true;
			}
		}
		goto label_postColumn;
	}

	// Phase 8: Trait anim 2
	if (_selectorRevealScrb7026Running) {
		if (_selectorAnimFeature->hasAnimEndCallbackFired()) {
			debugC(2, MohawkEngine_Zoombini::kDebugPage02, "net: enabled puzzle input after third trait reveal");
			_puzzleInputEnabled = true;
			_selectorInputEnabled = true;
			_selectorRevealScrb7026Running = false;
			if (kPuzzleLevel3 <= _difficultyLevel) {
				_previousSelectorValues[0] = -1;
				loadScrbOntoFeature(_selectorAnimFeature, kResScrb7027_TraitBase);
				if (_selectorAnimFeature) {
					_selectorRevealScrb7027Running = true;
					_positionShotHotspots = false;
					_selectorAnimFeature->setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniPuzzleNet::remapHotspotFramesByTrait));
				}
				goto label_postColumn;
			}
			_selectorRevealScrb7027Running = false;
			if (_initialSelectorSetup)
				goto label_initialSelectorSetup;
			if (_shotMissedTarget || _pageLoadedZmbCount <= _acceptedZmbCount)
				goto label_noMatch;
			_shotMissedTarget = false;
			releaseSelectorGateAfterTraitAnimation();
		}
		goto label_postColumn;
	}

	// Phase 9: Trait anim 3 (difficulty 2+ only)
	if (_selectorRevealScrb7027Running) {
		if (_selectorAnimFeature->hasAnimEndCallbackFired()) {
			debugC(2, MohawkEngine_Zoombini::kDebugPage02, "net: completed final trait reveal");
			_selectorRevealScrb7027Running = false;
			if (_initialSelectorSetup) {
			label_initialSelectorSetup:
				loadScrbOntoFeature(_feedbackFeature, kResScrb10017_FeedbackPartial);
				_initialSelectorSetup = false;
				_lastSubmitFrame = getCurrentFrameCounter();
				goto label_postColumn;
			}
			if (_shotMissedTarget || _pageLoadedZmbCount <= _acceptedZmbCount) {
			label_noMatch:
				_shotMissedTarget = false;
				releaseSelectorGateAfterTraitAnimation();
				loadScrbOntoFeature(_feedbackFeature, kResScrb10018_FeedbackComplete);
				goto label_postColumn;
			}
			_shotMissedTarget = false;
			releaseSelectorGateAfterTraitAnimation();
		}
		goto label_postColumn;
	}

label_postColumn:
	// Phase 10: Column animation done -- open column
	if (_columnOpenPendingCount && _remainingLaunchesForShot) {
		_columnOpenPendingCount = 0;
		if (_pageLoadedZmbCount <= _nextZmbToAssign)
			_queuedColumnAssignmentCount = 0;
		_remainingLaunchesForShot -= 1;
		_remainingLaunchesForShot = MAX<int16>(_remainingLaunchesForShot, 0);
		if (_remainingLaunchesForShot == 0) {
			if (_pendingZmbIndex < 0) {
				if (_queuedColumnAssignmentCount)
					assignNextSnoidToColumn();
			}
		}
		// Compute column index and load column SCRB
		int16 colIdx;
		if (kPuzzleLevel3 <= _difficultyLevel)
			colIdx = _selectedTargetSlotIdx % 25 / 5;
		else
			colIdx = _selectedTargetSlotIdx % 5;
		loadScrbOntoFeature(_columnRouteFeatures[colIdx], kResScrb8000_ColumnBase + colIdx);
		// After the first active slot, keep the reloaded column runner immediately above the newest wall-slot runner.
		if (0 < _lastShotFeatureIdx)
			manualLinkAfter(_columnRouteFeatures[colIdx], _shotHistoryFeatures[_lastShotFeatureIdx]);
		_openingColumnIdx = colIdx;
		_columnOpenAnimRunning = true;
		_submissionActivityCount += 1;
		_lastSubmitFrame = getCurrentFrameCounter();
	} else if (_columnOpenAnimRunning) {
		// Phase 11: Column open animation done -> start entry
		// Wait for the specific column loaded in phase 10, not column 0.
		if (_columnRouteFeatures[_openingColumnIdx] && _columnRouteFeatures[_openingColumnIdx]->hasAnimEndCallbackFired()) {
			_columnOpenAnimRunning = false;
			_activeColumnIdx = 0;
			// Find first active column slot
			for (int16 columnSlotIdx = 0; columnSlotIdx < 3; columnSlotIdx++) {
				if (_columnSlotSnoidIds[columnSlotIdx]) {
					_activeColumnIdx = columnSlotIdx;
					break;
				}
			}
			if (_columnSlotSnoidIds[_activeColumnIdx]) {
				loadScrbOntoFeature(_columnEntryFeature, kResScrb8005_EntryBase + _activeColumnIdx);
				_columnEntryAnimRunning = true;
			}
		}
	} else if (_columnEntryAnimRunning) {
		// Phase 12: Entry animation done
		if (_columnEntryFeature->hasAnimEndCallbackFired()) {
			_columnEntryAnimRunning = false;
			if (_remainingLaunchesForShot)
				_columnOpenPendingCount += 1;
			else
				_columnAssignmentReadyCount += 1;
			if (_pageLoadedZmbCount <= _acceptedZmbCount && !_remainingLaunchesForShot) {
				_columnOpenPendingCount = 0;
				_remainingLaunchesForShot = 0;
			}
		}
	} else if (0 <= _pendingZmbIndex && !_rejectedShotCount && _pendingZmbIndex < _pageLoadedZmbCount) {
		// Phase 13: Walk pending zoombini to column.
		// Wait for the Snoid to reach the staging point and return to idle before starting SCRS 13001.
		// Until then, it remains in @ref kSnoidAnimState007_Depart or @ref kSnoidAnimState112_Path.
		// Starting the SCRS early would use the Snoid's mid-walk position,
		// causing each Snoid to land at a different plank slot.
		uint16 snoidId = 10000 + _pendingZmbIndex;
		ZmbSnoid *snoid = getIdleSnoid(snoidId);
		if (snoid) {
			int16 scrsId = static_cast<int16>(5 * (2 - _activeColumnIdx) + snoid->_trait._feet - 1 + kResScrs13001_NormalWalkBase);
			if (startVisibleNormalScrs(snoid, scrsId)) {
				_travelingColumnSnoidIds[_activeColumnIdx] = _columnSlotSnoidIds[_activeColumnIdx];
				_pendingZmbIndex = -1;
			}
		}
	}

	// Queue management
	if (_pendingZmbIndex < 0 && _queuedColumnAssignmentCount)
		assignNextSnoidToColumn();

	// Idle animations
	if (0 < _pageLoadedZmbCount) {
		if (_idleAnimTrigger && _idleAnimCount < _idleAnimMax) {
			if (30 < getCurrentFrameCounter() - _idleAnimLastFrame) {
				bool triggered = false;
				int16 attempts = 0;
				_idleAnimLastFrame = getCurrentFrameCounter();

				do {
					uint16 poolIdx = _vm->_rnd->getNonRepeatRandom(_pageLoadedZmbCount, _idleAnimPoolState);
					uint16 snoidId = 10000 + poolIdx;

					// Skip snoids in active column slots
					if (snoidId == _columnSlotSnoidIds[0] ||
						snoidId == _columnSlotSnoidIds[1] ||
						snoidId == _columnSlotSnoidIds[2]) {
						attempts += 1;
						if (20 < attempts)
							triggered = true;
						continue;
					}

					ZmbSnoid *snoid = getIdleSnoid(snoidId);
					// Celebration can replace only an idle runner, even after the round is complete.
					if (snoid &&
						snoid->isRenderActivated() &&
						snoid->hasFlag(ZmbFeature::FLAG_00000001_TYPE_SNOID)) {
						// Skip locked snoids unless round is complete
						if (snoid->_packIsOccupied && !_roundCompleted) {
							attempts += 1;
							if (20 < attempts)
								triggered = true;
							continue;
						}

						int16 scrsId = static_cast<int16>(snoid->_trait._feet - 1 + kResScrs13046_NormalIdleBase);
						// Idle fidget SCRS 13046-13050 are NET group 1 -> state 8.
						// Route through the shared resolver instead of hardcoding.
						if (startSnoidScrs(snoid, ZmbResource(ZmbResource::kPage, scrsId), ZmbScrsCompletionMode::kReturnToIdle)) {
							snoid->setScrsEndEventEnabled(false);
							_idleAnimCount += 1;
							triggered = true;
						}
					} else {
						attempts += 1;
						if (20 < attempts)
							triggered = true;
					}
				} while (!triggered);
			}
		} else if (_idleAnimMax <= _idleAnimCount && 0 < _idleAnimMax) {
			_idleAnimPoolState = 0;
			_idleAnimLastFrame = 0;
			_idleAnimTrigger = false;
			_idleAnimCount = 0;
		}
	}

	// Submit timeout -- idle feedback after inactivity
	if (_submissionActivityCount) {
		if (720 < getCurrentFrameCounter() - _lastSubmitFrame) {
			_selectorInputEnabled = true;
			_submissionActivityCount = 0;
			loadScrbOntoFeature(_feedbackFeature, kResScrb10018_FeedbackComplete);
			_lastSubmitFrame = getCurrentFrameCounter();
		}
	} else {
		if (7200 < getCurrentFrameCounter() - _lastSubmitFrame) {
			_lastSubmitFrame = 0;
			_selectorInputEnabled = true;
			_submissionActivityCount = 0;
			loadScrbOntoFeature(_feedbackFeature, kResScrb10018_FeedbackComplete);
			_lastSubmitFrame = getCurrentFrameCounter();
		}
	}

	// Bounce animation tick
	if (0 < _bounceCounter) {
		_bounceCounter += 1;
		if (5 < _bounceCounter) {
			_bounceCounter = 0;
			settleShotAtTargetSlot(_selectedTargetSlotIdx);
		} else {
			_bounceX -= _bounceDeltaX;
			_bounceY -= _bounceDeltaY;
		}
	}

	updateChanceVisualAnimation();
}

} // End of namespace Mohawk
