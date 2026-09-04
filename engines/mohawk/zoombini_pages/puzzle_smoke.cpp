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

#include "mohawk/zoombini_pages/puzzle_smoke.h"
#include "mohawk/cursors.h"
#include "mohawk/zoombini.h"
#include "mohawk/zoombini_random.h"
#include "mohawk/zoombini_resource.h"
#include "mohawk/zoombini_sound.h"
#include "mohawk/zoombini_state.h"

namespace Mohawk {

constexpr Common::Point ZoombiniPuzzleSmoke::kSnoidPositions[20];
constexpr Common::Point ZoombiniPuzzleSmoke::kDrawOnRegPosition;
constexpr Common::Point ZoombiniPuzzleSmoke::kCliffRunnerPositions[8];
constexpr Common::Point ZoombiniPuzzleSmoke::kGridRunnerPositions[8];
constexpr Common::Point ZoombiniPuzzleSmoke::kLevel2RunnerPositions[4];
constexpr Common::Point ZoombiniPuzzleSmoke::kExitRunnerPositions[2];
constexpr Common::Point ZoombiniPuzzleSmoke::kLevel4TransitionPositions[4];
constexpr Common::Point ZoombiniPuzzleSmoke::kBottomRunnerPositions[2];
constexpr Common::Point ZoombiniPuzzleSmoke::kHidePosition;
constexpr Common::Point ZoombiniPuzzleSmoke::kRejectPosition;
constexpr Common::Point ZoombiniPuzzleSmoke::kCliffDropSnapPosition;
constexpr int16 ZoombiniPuzzleSmoke::kPedestalRowY[5];
constexpr Common::Point ZoombiniPuzzleSmoke::kFilterPositionsA[9];
constexpr Common::Point ZoombiniPuzzleSmoke::kFilterPositionsB[9];
constexpr Common::Point ZoombiniPuzzleSmoke::kDisplayPairNormalA[13];
constexpr Common::Point ZoombiniPuzzleSmoke::kDisplayPairNormalB[13];
constexpr Common::Point ZoombiniPuzzleSmoke::kDisplayPairSwappedA[17];
constexpr Common::Point ZoombiniPuzzleSmoke::kDisplayPairSwappedB[17];

ZoombiniPuzzleSmoke::ZoombiniPuzzleSmoke(MohawkEngine_Zoombini *vm) : ZoombiniPuzzle(vm, ZoombiniPageType::kSmoke, ZmbSrcPageKind::kSmoke_15) {
}

ZoombiniPuzzleSmoke::~ZoombiniPuzzleSmoke() {
	setRunnerDragActive(false);
	_vm->_sound->setSfxMuted(!_savedSfxEnabled);
}

const ZoombiniPage::ScriptSoundPriorityRanges &ZoombiniPuzzleSmoke::getScriptSoundPriorityRanges() const {
	// Original shared page dispatch automatically prepends SND 996-997 at priority 32.
	static const ZoombiniPage::ScriptSoundPriorityRanges kLevel3Ranges = {
		{kSysResSoundRange0996_DepartSFX, kSysResSoundRange0997_ArriveSFX},
		// Smoke then registers the 996-997 range again in each authored level table.
		{kSysResSoundRange0996_DepartSFX, kSysResSoundRange0997_ArriveSFX},
		{kSysResSoundRange20000_Base, kSysResSoundRange29999_Last},
		{kResSoundRange11017_Startup, kResSoundRange11017_Startup},
		{kResSoundRange11001_Startup, kResSoundRange11001_Startup},
		{kResSoundRange11013_Startup, kResSoundRange11013_Startup},
		{kResSoundRange11016_Startup, kResSoundRange11016_Startup},
		{kResSoundRange0125_PageBase, kResSoundRange0149_PageLast},
		{kResSoundRange0450_PageBase, kResSoundRange0474_PageLast},
		{kResSoundRange11011_Startup, kResSoundRange11012_Startup},
		{kResSoundRange11014_Startup, kResSoundRange11015_Startup},
		{kResSoundRange11010_Startup, kResSoundRange11010_Startup},
		{kResSoundRange11002_Startup, kResSoundRange11004_Startup},
		{kResSoundRange11007_Startup, kResSoundRange11007_Startup},
		{kResSoundRange11006_Startup, kResSoundRange11006_Startup},
		{kResSoundRange11005_Startup, kResSoundRange11005_Startup},
		{kResSoundRange11000_Startup, kResSoundRange11000_Startup}};
	static const ZoombiniPage::ScriptSoundPriorityRanges kOtherLevelRanges = {
		{kSysResSoundRange0996_DepartSFX, kSysResSoundRange0997_ArriveSFX},
		// Smoke then registers the 996-997 range again in each authored level table.
		{kSysResSoundRange0996_DepartSFX, kSysResSoundRange0997_ArriveSFX},
		{kSysResSoundRange20000_Base, kSysResSoundRange29999_Last},
		{kResSoundRange11008_Startup, kResSoundRange11009_Startup},
		{kResSoundRange11001_Startup, kResSoundRange11001_Startup},
		{kResSoundRange11013_Startup, kResSoundRange11013_Startup},
		{kResSoundRange11016_Startup, kResSoundRange11016_Startup},
		{kResSoundRange0125_PageBase, kResSoundRange0149_PageLast},
		{kResSoundRange0450_PageBase, kResSoundRange0474_PageLast},
		{kResSoundRange11011_Startup, kResSoundRange11012_Startup},
		{kResSoundRange11014_Startup, kResSoundRange11015_Startup},
		{kResSoundRange11010_Startup, kResSoundRange11010_Startup},
		{kResSoundRange11002_Startup, kResSoundRange11004_Startup},
		{kResSoundRange11007_Startup, kResSoundRange11007_Startup},
		{kResSoundRange11006_Startup, kResSoundRange11006_Startup},
		{kResSoundRange11005_Startup, kResSoundRange11005_Startup},
		{kResSoundRange11000_Startup, kResSoundRange11000_Startup}};

	if (_difficultyLevel == kPuzzleLevel3) {
		return kLevel3Ranges;
	}

	return kOtherLevelRanges;
}

void ZoombiniPuzzleSmoke::open() {
	// MIDI BGM (tMID 30030-30033) - Broderbund v1.x only.
	// @ref ZoombiniPage::openMidiArchive() loads MIDIMPC.MHK (Windows profile) or, when "use_mac_midi" is set,
	// MIDIMAC.MHK (Macintosh profile).
	// Both hold the same tMID IDs.
	if (!_vm->isVersionFamilyTlcV2())
		openMidiArchive();
	openArchive(ZMB_MHK_SMOKE);
}

void ZoombiniPuzzleSmoke::setBackgroundMusic() {
	if (!_vm->isVersionFamilyTlcV2()) {
		const int16 routeLevel = _difficultyLevel - kPuzzleLevel1;
		_vm->_midi->playZmbMidi(ZmbResource(ZmbResource::kPage, static_cast<int16>(kResMidi30030_SmokeBgmBase + routeLevel)));
	}
}

void ZoombiniPuzzleSmoke::setBackgroundBitmap() {
	_vm->_gfx->setPalette(kResBackground5000);
	_vm->_gfx->drawBackground(kResBackground5000);

	// --- Terrain + preloads ---
	loadTerrainBitmap(kResBitmapTerrain100);

	// Preload shape images at tBMP 10000 (0x2710)
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape10000_Crystal));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape11000_Smoke));
	loadShapeOffsetRegs(ZmbResource::kPage, kResRegs10000_Smoke);
}

ZoombiniPuzzleSmoke::ZmbSmokeRunnerState *ZoombiniPuzzleSmoke::findRunnerState(ZmbFeature *feature) {
	for (int16 i = 0; i < _cliffRunnerCount; i++) {
		if (_cliffRunners[i] == feature)
			return &_cliffRunnerStates[i];
	}
	for (int16 i = 0; i < _level2RunnerCount; i++) {
		if (_level2Runners[i] == feature)
			return &_level2RunnerStates[i];
	}
	for (int16 i = 0; i < _gridRunnerCount; i++) {
		if (_gridRunners[i] == feature)
			return &_gridRunnerStates[i];
	}
	for (int16 i = 0; i < _exitRunnerCount; i++) {
		if (_exitRunners[i] == feature)
			return &_exitRunnerStates[i];
	}
	for (int16 i = 0; i < _reflectionRunnerCount; i++) {
		if (_reflectionRunners[i] == feature)
			return &_reflectionRunnerStates[i];
	}
	return nullptr;
}

void ZoombiniPuzzleSmoke::initStates() {
	// State initialization ---
	_goButtonEnabled = false;
	_displayPairPositionIdx = 0;
	_matchRunnerIndex = 0;
	_activeZmb = nullptr;
	_bFirstTraitAssign = true;
	_answerState = AnswerState::kIdle02;
	_leftFilterCount = 0;
	_rightFilterCount = 0;
	_level2RunnerCount = 0;
	_exitRunnerCount = 0;
	_reflectionRunnerCount = 0;
	_placedZmbCount = 0;
	_currentQueueIdx = 0;
	_bRunnerToggle = false;
	_transitionPhase = TransitionPhase::kInitialPair03;
	_loadedOnCliffCount = 0;
	_compareResult = CompareResult::kMatch00;
	_bPlaceActiveZmb = false;
	_bRelinkRejectionRunners = false;
	_bReloadOverlayAndAnswer = false;
	_bResetLevel = false;
	_bStartDeparture = false;
	_departureState = ZmbDepartureState::kIdle;
	_bReloadMainRunner = false;
	_questionSnoidId = 0;
	_questionCrystalIdx = -1;
	_questionZmbCount = 0;
	_bShowAnswer = false;
	_bPreserveFilterHomePositionOrder = false;
	_comparisonInputEnabled = true;
	_lastCelebrationIdleFrame = 0;
	_celebrationIdlePoolState = 0;
	_celebrationIdleProgress = 0;
	_bCelebrationIdleActive = false;
	_bCompareSwapped = false;

	// Clear trait state
	for (int16 traitIdx = 0; traitIdx < 8; traitIdx++)
		_displayTraits[traitIdx] = ZmbTrait();
	for (int16 traitIdx = 0; traitIdx < 9; traitIdx++) {
		_primaryGridTraits[traitIdx] = ZmbTrait();
		_secondaryGridTraits[traitIdx] = ZmbTrait();
		_gridMatchTraits[traitIdx] = ZmbTrait();
	}
	_questionTraits[0] = ZmbTrait();
	_questionTraits[1] = ZmbTrait();

	// --- Difficulty setup ---
	_difficultyLevel = MIN<ZmbPuzzleLevel>(_difficultyLevel, kPuzzleLevel4);

	// Per-difficulty SCRB IDs ---
	if (_difficultyLevel <= kPuzzleLevel2) {
		_scrbAnimIdArr[0] = kResScrb11024_MainLevel1;
		_scrbAnimIdArr[1] = kResScrb11025_MainLevel1;
		_scrbAnimIdArr[2] = kResScrb11026_MainLevel1;
		_scrbAnimIdArr[3] = kResScrb11027_MainLevel1;
		_scrbZmbAnimIdArr[0] = kResScrs11999_NormalBase;
		_scrbZmbAnimIdArr[1] = kResScrs12004_RejectLevel2;
		_scrbSmokeStackResA = kResScrb11032_StackLevel1;
		_scrbSmokeStackResB = 0;
		_scrbTravelResId = 0;
		_scrbPickupResId = 0;
		_scrbDropResId = 0;
		_scrbWalkResId = 0;
	} else {
		_scrbAnimIdArr[0] = kResScrb11028_MainLevel3;
		_scrbAnimIdArr[1] = kResScrb11029_MainLevel3;
		_scrbAnimIdArr[2] = kResScrb11030_MainLevel3;
		_scrbAnimIdArr[3] = kResScrb11031_MainLevel3;
		_scrbZmbAnimIdArr[0] = kResScrs12009_RejectLevel3;
		_scrbZmbAnimIdArr[1] = kResScrs12014_RejectLevel4;
		_scrbSmokeStackResA = kResScrb11033_StackLevel3;
		_scrbSmokeStackResB = kResScrb11034_StackLevel4;
		_scrbTravelResId = kResScrb11035_TravelLevel3;
		_scrbPickupResId = kResScrs12038_PickupLevel34;
		_scrbDropResId = kResScrs12039_DropLevel34;
		_scrbWalkResId = kResScrs12040_WalkLevel34;
	}

	if (_difficultyLevel == kPuzzleLevel4) {
		_scrbOverlayResId = kResScrb11011_Level4Overlay;
		_scrbTransitionResId = kResScrb11012_Level4Transition;
	} else {
		_scrbOverlayResId = kResScrb11013_Overlay;
		_scrbTransitionResId = 0;
	}
}

void ZoombiniPuzzleSmoke::loadFeatures() {
	for (int i = 0; i < 2; i++) {
		_level12WalkInSnoids[i] = nullptr;
		_level12WalkInPosePending[i] = false;
	}

	_savedSfxEnabled = !_vm->_sound->isSfxMuted();
	_vm->_sound->setSfxMuted(true);

	// Initialize permutation array
	for (int i = 0; i < 8; i++)
		_filterHomePermutation[i] = i;

	// Clear display runner tracking
	memset(_filterSlotRunners, 0, sizeof(_filterSlotRunners));

	// Clear runner and zmb arrays
	for (int i = 0; i < 20; i++) {
		_departureColumnRunners[i] = nullptr;
		_acceptedZmbs[i] = nullptr;
		_cliffRunners[i] = nullptr;
	}
	for (int i = 0; i < 6; i++)
		_level2Runners[i] = nullptr;
	for (int i = 0; i < 9; i++)
		_gridRunners[i] = nullptr;
	for (int i = 0; i < 4; i++)
		_exitRunners[i] = nullptr;
	for (int i = 0; i < 2; i++)
		_reflectionRunners[i] = nullptr;
	for (int i = 0; i < 21; i++)
		_zmbQueue[i] = 0;

	// --- Feature group + snoid pools ---
	createPuzzleMainFeatureHead();

	// These calls register resource ranges; they do not materialize 51 hidden Snoid runners.
	// Call order is significant: 11999 is group 0 (NORMAL/state 9),
	// while 12000-12049 is group 1 (REJECT/state 8) and uses the general trait-layer tables.
	registerScrsGroup(kResScrs11999_NormalBase, 1);
	registerScrsGroup(kResScrs12000_RejectBase, 50);

	// [*] Feature runners

	// Interval=10
	_overlayAnimFeature = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape11000_Smoke), _scrbOverlayResId, 10,
		ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_04000000_OVERLAY);

	// SCRB 11076, L1-2 only, interval=10
	if (_difficultyLevel <= kPuzzleLevel2) {
		_level12ExtraFeature = loadScrbFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape11000_Smoke), kResScrb11076_Level12Extra, 10,
			ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM |
				ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_01000000_DEFER_RENDER |
				ZmbFeature::FLAG_04000000_OVERLAY);
	}

	// SCRB 11006, interval=10
	_cliffLeftFeature = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape11000_Smoke), kResScrb11006_CliffLeft, 10,
		ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM |
			ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_01000000_DEFER_RENDER |
			ZmbFeature::FLAG_04000000_OVERLAY);

	// SCRB 11007, interval=10
	_cliffRightFeature = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape11000_Smoke), kResScrb11007_CliffRight, 10,
		ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM |
			ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_01000000_DEFER_RENDER |
			ZmbFeature::FLAG_04000000_OVERLAY);

	// The first main-animation entry runs at interval 6.
	_mainAnimFeature = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape11000_Smoke), _scrbAnimIdArr[0], 6,
		ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM |
			ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_01000000_DEFER_RENDER |
			ZmbFeature::FLAG_04000000_OVERLAY);
	// The central animation owns SFX handoffs that replace one retained effect with the next.
	if (_mainAnimFeature)
		_mainAnimFeature->setScriptSoundPolicy(ZmbFeature::ScriptSoundPolicy::kPriorityQueue);

	// Interval=6
	_smokeStackAFeature = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape11000_Smoke), _scrbSmokeStackResA, 6,
		ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_04000000_OVERLAY);

	// L3-4 only
	if (kPuzzleLevel3 <= _difficultyLevel) {
		_smokeStackBFeature = loadScrbFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape11000_Smoke), _scrbSmokeStackResB, 6,
			ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_04000000_OVERLAY);
	}

	// The second main-animation entry also runs at interval 6.
	_secondAnimFeature = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape11000_Smoke), _scrbAnimIdArr[1], 6,
		ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE |
			ZmbFeature::FLAG_01000000_DEFER_RENDER | ZmbFeature::FLAG_04000000_OVERLAY |
			ZmbFeature::FLAG_08000000_REGION_TRACK);

	// SCRB 11018, interval=6
	_compareAFeature = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape11000_Smoke), kResScrb11018_CompareA, 6,
		ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_01000000_DEFER_RENDER | ZmbFeature::FLAG_04000000_OVERLAY);

	// SCRB 11019, interval=6
	_compareBFeature = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape11000_Smoke), kResScrb11019_CompareB, 6,
		ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE |
			ZmbFeature::FLAG_01000000_DEFER_RENDER | ZmbFeature::FLAG_04000000_OVERLAY |
			ZmbFeature::FLAG_08000000_REGION_TRACK);

	// SCRB 11009, interval=6
	loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape11000_Smoke), kResScrb11009_BackgroundOverlay, 6, ZmbFeature::FLAG_04000000_OVERLAY);

	// SCRB 11036, interval=6
	_rejectionFeature = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape11000_Smoke), kResScrb11036_RejectionBase, 6,
		ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_01000000_DEFER_RENDER | ZmbFeature::FLAG_04000000_OVERLAY);
	// The rejection effect replaces the retained central-animation effect through the same queue.
	if (_rejectionFeature)
		_rejectionFeature->setScriptSoundPolicy(ZmbFeature::ScriptSoundPolicy::kPriorityQueue);

	// SCRB 11008, interval=0
	_backgroundFeature = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape11000_Smoke), kResScrb11008_Background, 0,
		ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_04000000_OVERLAY);

	// SCRB 11002, interval=5
	_answerZoneFeature = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape11000_Smoke), kResScrb11002_Well, 5,
		ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_04000000_OVERLAY);

	// SCRB 11077, interval=0
	_holdingAreaFeature = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape11000_Smoke), kResScrb11077_HoldingArea, 0,
		ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_04000000_OVERLAY);

	// --- Load Zoombinis and build runner stacks ---
	loadZoombinisFromPack();
	if (0 < _pageLoadedZmbCount) {
		if (kPuzzleLevel3 <= _difficultyLevel) {
			// Consume the setup pack-selection draw even though these levels use queue order.
			(void)_vm->_rnd->getRandomNumber(_pageLoadedZmbCount - 1);
			// L3-4 hide the pack and seed the crystal generator from the first pair in queue order.
			for (int16 i = 0; i < _pageLoadedZmbCount; i++) {
				ZmbSnoid *snoid = getSnoid(_zmbQueue[i]);
				if (snoid)
					snoid->deactivateRender();
			}
			_questionZmbCount = copyPairToCompareBuffer();
		} else {
			// L1-2 choose one pack member before any candidate crystal traits are generated.
			selectQuestionZmb();
		}
	}

	_cliffRunnerCount = 0;
	_gridRunnerCount = 1;

	buildRunnerStacks();

	if (_difficultyLevel < kPuzzleLevel3) {
		loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape11000_Smoke), kResScrb11001_DrawOnReg, 7, kDrawOnRegPosition,
						ZmbFeature::FLAG_00002000_DRAW_ON_REG | ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_01000000_DEFER_RENDER);
	}

	// The setup render advances every custom runner timer and materializes the authored
	// feature stack before the controls and startup SCRBs are installed.
	tickSmokeRunners();
	renderFeatures();

	// --- Buttons ---
	configureStandardPuzzleControlRects();
	loadStandardPuzzleControlFeatures(kResBitmapShape6000_GoMapButtons);

	// Restore the user's SFX setting before loading the startup mirror SCRBs.
	// The normal render pass dispatches frame zero of SCRB 11015 through the page sound queue.
	_vm->_sound->setSfxMuted(!_savedSfxEnabled);
	if (_mainAnimFeature)
		loadScrbOntoFeature(_mainAnimFeature, kResScrb11015_MainStartup);
	if (_secondAnimFeature)
		loadScrbOntoFeature(_secondAnimFeature, kResScrb11016_SecondStartup);
	registerFeatureTimingGroup(_mainAnimFeature, _secondAnimFeature);

	// SCRB loading registers its hotspot groups.
	// Smoke starts with no Snoids accepted onto the cliff.
	schedulePackSnoids(false, false);

	// For levels 3-4: show answer display on init
	if (kPuzzleLevel3 <= _difficultyLevel) {
		_bShowAnswer = true;
		loadScrbOnAnswerRunner(kResScrb11003_Answer);
	}
}

void ZoombiniPuzzleSmoke::initHelpPrompt() {
	_activeHelpSoundId = ZmbResource(ZmbResource::kSystem, _vm->_rnd->getRandomNumber(kSysResSound20066_HelpBase, kSysResSound20067_HelpLast));
}

void ZoombiniPuzzleSmoke::onGoButtonActivated() {
	// Start the three-cart departure sequence, then hold the route-18 transition until SCRB event 60.
	// SND 0 means there is no separate departure sound to wait for.
	_bStartDeparture = true;
	_departureState = _goButtonEnabled ? ZmbDepartureState::kAnimating : ZmbDepartureState::kCompleted;
}

void ZoombiniPuzzleSmoke::updateDepartureState() {
	if (_departureState == ZmbDepartureState::kCompleted)
		executeDeparture();
}

void ZoombiniPuzzleSmoke::applyDebugDisplayTraitRow(ZmbTrait &traits, int16 row) const {
	for (int16 traitIdx = 0; traitIdx < 4; traitIdx++) {
		const byte replacement = _displayTraits[row][traitIdx];
		if (replacement)
			traits[traitIdx] = replacement;
	}
}

void ZoombiniPuzzleSmoke::applyDebugFilterTraits(ZmbTrait &traits, const ZmbSmokeRunnerState &filter) {
	const int16 cycleTrait = filter.getCycleTraitIndex();
	for (int16 traitIdx = 0; traitIdx < 4; traitIdx++) {
		byte replacement = filter.traits[traitIdx];
		if (cycleTrait == traitIdx + 1)
			replacement = (traits[traitIdx] == 5) ? 1 : traits[traitIdx] + 1;
		if (replacement)
			traits[traitIdx] = replacement;
	}
}

bool ZoombiniPuzzleSmoke::advanceDebugFilterPermutation(int16 values[6]) {
	int16 pivot = 4;
	while (0 <= pivot && values[pivot + 1] <= values[pivot])
		pivot -= 1;
	if (pivot < 0)
		return false;
	int16 swapIdx = 5;
	while (values[swapIdx] <= values[pivot])
		swapIdx -= 1;
	SWAP(values[pivot], values[swapIdx]);
	for (int16 left = pivot + 1, right = 5; left < right; left++, right--)
		SWAP(values[left], values[right]);
	return true;
}

Common::String ZoombiniPuzzleSmoke::debugGetAnswer() const {
	Common::String answer = getDebugBanner();
	answer += ": current comparison state\n";

	if (_difficultyLevel <= kPuzzleLevel2) {
		const ZmbSnoid *snoid = _questionSnoidId ? getSnoid(_questionSnoidId) : nullptr;
		const ZmbFeature *crystal;
		if (0 <= _questionCrystalIdx && _questionCrystalIdx < _cliffRunnerCount)
			crystal = _cliffRunners[_questionCrystalIdx];
		else
			crystal = nullptr;
		if (!snoid || !crystal) {
			answer += "  (current question is not ready)\n";
			return answer;
		}

		ZmbTrait leftTraits = snoid->_trait;
		ZmbTrait rightTraits = _cliffRunnerStates[_questionCrystalIdx].traits;
		for (int16 row = 1; row < 4; row++)
			applyDebugDisplayTraitRow(leftTraits, row);
		for (int16 row = 6; 3 < row; row--)
			applyDebugDisplayTraitRow(rightTraits, row);

		if (leftTraits != rightTraits) {
			answer += "  (current question is awaiting crystal setup)\n";
			return answer;
		}

		const Common::Point crystalPos = crystal->getPointLoc();
		answer += "  Current generated comparison:\n";
		answer += Common::String::format("    %s\n", snoid->toStr().c_str());
		answer += Common::String::format("    Right crystal (%d, %d): %s\n", crystalPos.x, crystalPos.y, _cliffRunnerStates[_questionCrystalIdx].traits.toStr().c_str());
		answer += Common::String::format("    Matching result: %s\n", leftTraits.toStr().c_str());
		return answer;
	}

	ZmbSnoid *primarySnoid;
	if (_currentQueueIdx < _pageLoadedZmbCount && _zmbQueue[_currentQueueIdx])
		primarySnoid = getSnoid(_zmbQueue[_currentQueueIdx]);
	else
		primarySnoid = nullptr;
	if (!primarySnoid || _gridRunnerCount <= 7) {
		answer += "  (current source or target is not ready)\n";
		return answer;
	}

	const ZmbTrait primarySource = primarySnoid->_trait;
	const ZmbTrait primaryTarget = _gridRunnerStates[7].traits;
	const ZmbFeature *primaryCrystal = _gridRunners[7];

	bool hasSecondary = false;
	ZmbTrait secondarySource;
	ZmbTrait secondaryTarget;
	ZmbSnoid *secondarySnoid = nullptr;
	const ZmbFeature *secondaryCrystal = nullptr;
	if (_difficultyLevel == kPuzzleLevel4 && _currentQueueIdx + 1 < _pageLoadedZmbCount &&
		_zmbQueue[_currentQueueIdx + 1] && 8 < _gridRunnerCount) {
		secondarySnoid = getSnoid(_zmbQueue[_currentQueueIdx + 1]);
		if (secondarySnoid) {
			secondarySource = secondarySnoid->_trait;
			secondaryTarget = _gridRunnerStates[8].traits;
			secondaryCrystal = _gridRunners[8];
			hasSecondary = true;
		}
	}

	int16 solution[6] = {};
	int16 solutionLeftCount = 0;
	int16 solutionRightCount = 0;
	bool found = false;
	ZmbTrait matchingPrimary;
	ZmbTrait matchingSecondary;
	// The six generated filters are candidates, not six mandatory placements.
	// Each side accepts zero through three ordered filters.
	// Search by total count so the debugger prints a compact valid arrangement and leaves distractors unused.
	for (int16 totalCount = 0; totalCount < 7 && !found; totalCount++) {
		for (int16 leftCount = 0; leftCount < 4 && !found; leftCount++) {
			const int16 rightCount = totalCount - leftCount;
			if (rightCount < 0 || 3 < rightCount)
				continue;

			int16 permutation[6] = {0, 1, 2, 3, 4, 5};
			do {
				ZmbTrait leftPrimary = primarySource;
				ZmbTrait rightPrimary = primaryTarget;
				ZmbTrait leftSecondary;
				for (int16 slot = 0; slot < leftCount; slot++)
					applyDebugFilterTraits(leftPrimary, _gridRunnerStates[permutation[slot] + 1]);
				for (int16 slot = rightCount - 1; 0 <= slot; slot--)
					applyDebugFilterTraits(rightPrimary, _gridRunnerStates[permutation[leftCount + slot] + 1]);
				if (leftPrimary != rightPrimary)
					continue;

				if (hasSecondary) {
					leftSecondary = secondarySource;
					ZmbTrait rightSecondary = secondaryTarget;
					for (int16 slot = 0; slot < leftCount; slot++)
						applyDebugFilterTraits(leftSecondary, _gridRunnerStates[permutation[slot] + 1]);
					for (int16 slot = rightCount - 1; 0 <= slot; slot--)
						applyDebugFilterTraits(rightSecondary, _gridRunnerStates[permutation[leftCount + slot] + 1]);
					if (leftSecondary != rightSecondary)
						continue;
				}

				memcpy(solution, permutation, sizeof(solution));
				solutionLeftCount = leftCount;
				solutionRightCount = rightCount;
				matchingPrimary = leftPrimary;
				if (hasSecondary)
					matchingSecondary = leftSecondary;
				found = true;
				break;
			} while (advanceDebugFilterPermutation(permutation));
		}
	}

	answer += "  Primary comparison:\n";
	answer += Common::String::format("    %s\n", primarySnoid->toStr().c_str());
	if (primaryCrystal) {
		const Common::Point crystalPos = primaryCrystal->getPointLoc();
		answer += Common::String::format("    Target crystal (%d, %d): %s\n", crystalPos.x, crystalPos.y, primaryTarget.toStr().c_str());
	} else {
		answer += Common::String::format("    Target crystal: %s\n", primaryTarget.toStr().c_str());
	}
	if (found)
		answer += Common::String::format("    Matching result: %s\n", matchingPrimary.toStr().c_str());
	if (hasSecondary) {
		answer += "  Secondary comparison:\n";
		answer += Common::String::format("    %s\n", secondarySnoid->toStr().c_str());
		if (secondaryCrystal) {
			const Common::Point crystalPos = secondaryCrystal->getPointLoc();
			answer += Common::String::format("    Target crystal (%d, %d): %s\n", crystalPos.x, crystalPos.y, secondaryTarget.toStr().c_str());
		} else {
			answer += Common::String::format("    Target crystal: %s\n", secondaryTarget.toStr().c_str());
		}
		if (found)
			answer += Common::String::format("    Matching result: %s\n", matchingSecondary.toStr().c_str());
	}
	if (!found) {
		answer += "  (no ordered filter placement solves the current generated state)\n";
		return answer;
	}
	answer += "  Filter placement:\n";
	answer += "    Source to mirror:\n";
	if (!solutionLeftCount) {
		answer += "      (empty)\n";
	} else {
		for (int16 slot = 0; slot < solutionLeftCount; slot++)
			answer += Common::String::format("      Filter %d\n", solution[slot] + 1);
	}
	answer += "    Mirror to target:\n";
	if (!solutionRightCount) {
		answer += "      (empty)\n";
	} else {
		for (int16 slot = 0; slot < solutionRightCount; slot++)
			answer += Common::String::format("      Filter %d\n", solution[solutionLeftCount + slot] + 1);
	}
	bool usedFilters[6] = {};
	for (int16 slot = 0; slot < solutionLeftCount + solutionRightCount; slot++)
		usedFilters[solution[slot]] = true;
	bool hasUsedFilter = false;
	for (int16 filterIdx = 0; filterIdx < 6; filterIdx++) {
		if (!usedFilters[filterIdx])
			continue;

		if (!hasUsedFilter) {
			answer += "  Filter traits:\n";
			hasUsedFilter = true;
		}

		const ZmbSmokeRunnerState &filter = _gridRunnerStates[filterIdx + 1];
		Common::String firstEffect = "(none)";
		Common::String secondEffect = "(none)";
		int16 effectCount = 0;
		const int16 cycleTrait = filter.getCycleTraitIndex();
		for (int16 trait = 0; trait < 4; trait++) {
			if (cycleTrait != trait + 1 && !filter.traits[trait])
				continue;

			const ZmbTrait::TraitKind kind = ZmbTrait::traitKindFromIndex(trait);
			Common::String effect;
			if (cycleTrait == trait + 1)
				effect = Common::String::format("%s=next", ZmbTrait::debugTraitKindName(kind));
			else
				effect = Common::String::format("%s=%s", ZmbTrait::debugTraitKindName(kind), ZmbTrait::debugTraitValueName(kind, filter.traits[trait]));
			if (effectCount == 0)
				firstEffect = effect;
			else if (effectCount == 1)
				secondEffect = effect;
			else {
				secondEffect += ", ";
				secondEffect += effect;
			}
			effectCount += 1;
		}
		if (effectCount == 1)
			answer += Common::String::format("    Filter %d: %s\n", filterIdx + 1, firstEffect.c_str());
		else
			answer += Common::String::format("    Filter %d: %s, %s\n", filterIdx + 1, firstEffect.c_str(), secondEffect.c_str());
	}
	return answer;
}

Common::String ZoombiniPuzzleSmoke::debugGetBuiltinDebugCommandHelp() const {
	Common::String output;
	output += Common::String::format("  %-7s (%s)\n", "Shift+L", kBuiltinDebugActionLevel);
	output += "    Show the current one-based difficulty as Level 1 through Level 4; this does not change it.\n";
	output += Common::String::format("  %-7s (%s)\n", "F4", kBuiltinDebugActionCheatOn);
	output += "    Keep later level 3/4 filter sets in generated row order instead of shuffling their home positions.\n";
	output += "    This does not move the current filters, auto-place them, or change their generated traits.\n";
	output += "    It takes effect when the next question is generated after the current level 3 Zoombini or level 4 pair.\n";
	output += "    Levels 1/2 are unaffected.\n";
	output += Common::String::format("  %-7s (%s)\n", "F5", kBuiltinDebugActionCheatOff);
	output += "    Resume shuffling the home positions of later level 3/4 filter sets.\n";
	output += "    This does not reshuffle the current filters; levels 1/2 are unaffected.\n";
	return output;
}

bool ZoombiniPuzzleSmoke::debugDoBuiltinDebugCommand(int argc, const char **argv, Common::String &output) {
	if (argc != 3) {
		output = Common::String::format("Usage: page builtin_debug <%s|%s|%s>\n", kBuiltinDebugActionLevel, kBuiltinDebugActionCheatOn, kBuiltinDebugActionCheatOff);
		return true;
	}

	const BuiltinDebugAction action = parseBuiltinDebugAction(argv[2]);
	if (action == BuiltinDebugAction::kInvalid) {
		output = Common::String::format("Unknown Mirror Machine debug action '%s'.\n", argv[2]);
		output += debugGetBuiltinDebugCommandHelp();
		return true;
	}
	return runBuiltinDebugAction(action, output);
}

ZoombiniPuzzleSmoke::BuiltinDebugAction ZoombiniPuzzleSmoke::parseBuiltinDebugAction(const Common::String &action) {
	if (action.equalsIgnoreCase(kBuiltinDebugActionLevel))
		return BuiltinDebugAction::kLevel;
	if (action.equalsIgnoreCase(kBuiltinDebugActionCheatOn))
		return BuiltinDebugAction::kCheatOn;
	if (action.equalsIgnoreCase(kBuiltinDebugActionCheatOff))
		return BuiltinDebugAction::kCheatOff;
	return BuiltinDebugAction::kInvalid;
}

bool ZoombiniPuzzleSmoke::runBuiltinDebugAction(BuiltinDebugAction action, Common::String &output) {
	Common::String debugText;
	if (action == BuiltinDebugAction::kLevel) {
		const int originalDifficulty = static_cast<int>(_difficultyLevel);
		output = Common::String::format("Level %d\n", originalDifficulty);
		debugText = Common::String::format("Level %d ", originalDifficulty);
	} else if (action == BuiltinDebugAction::kCheatOn) {
		_bPreserveFilterHomePositionOrder = true;
		output = " Cheat on \n";
		debugText = " Cheat on ";
	} else if (action == BuiltinDebugAction::kCheatOff) {
		_bPreserveFilterHomePositionOrder = false;
		output = "Mirror Machine cheat off.\n";
	} else {
		return true;
	}

	if (!debugText.empty())
		showBuiltinDebugText(debugText);
	return false;
}

ZmbEventHandleResult ZoombiniPuzzleSmoke::onDebugKeyDown(const Common::KeyState &kbd) {
	BuiltinDebugAction action = BuiltinDebugAction::kInvalid;
	if (kbd.hasFlags(0) || kbd.hasFlags(Common::KBD_SHIFT)) {
		if (kbd.ascii == 'L')
			action = BuiltinDebugAction::kLevel;
		else if (kbd.keycode == Common::KEYCODE_F4)
			action = BuiltinDebugAction::kCheatOn;
		else if (kbd.keycode == Common::KEYCODE_F5)
			action = BuiltinDebugAction::kCheatOff;
	}
	if (action == BuiltinDebugAction::kInvalid)
		return ZmbEventHandleResult::kPassthrough;

	Common::String output;
	runBuiltinDebugAction(action, output);
	return ZmbEventHandleResult::kConsumed;
}

void ZoombiniPuzzleSmoke::loadZoombinisFromPack() {
	Common::Array<ZmbSnoid *> loadedSnoids;
	_pageLoadedZmbCount = loadOccupiedSnoidsFromActivePack(kSnoidPositions, ARRAYSIZE(kSnoidPositions), &loadedSnoids);
	int16 queueSize = 0;
	for (ZmbSnoid *snoid : loadedSnoids) {
		if (queueSize < static_cast<int16>(ARRAYSIZE(_zmbQueue))) {
			_zmbQueue[queueSize] = snoid->getId();
			queueSize += 1;
		}
	}

	// Levels 1-2 override only the final two pack runners.
	// They enter from the left on row two and retain common image 0 when state 7 settles,
	// so they face front at their assigned seats.
	if (_difficultyLevel <= kPuzzleLevel2 && !loadedSnoids.empty()) {
		const int16 count = static_cast<int16>(loadedSnoids.size());
		const int16 firstWalkIn = MAX<int16>(0, count - 2);
		for (int16 snoidIdx = firstWalkIn; snoidIdx < count; snoidIdx++) {
			ZmbSnoid *snoid = loadedSnoids[snoidIdx];
			const bool isLast = snoidIdx == count - 1;
			const int16 walkInIdx = snoidIdx - firstWalkIn;
			snoid->setCommonImageIndex(0);
			snoid->setAnimState(kSnoidAnimState007_Depart);
			snoid->setPointLoc(Common::Point(isLast ? 45 : 110, 79));
			snoid->setAnimTargetPos(Common::Point(isLast ? 160 : 200, 79));
			if (walkInIdx < 2) {
				_level12WalkInSnoids[walkInIdx] = snoid;
				_level12WalkInPosePending[walkInIdx] = true;
			}
		}
	}
}

// =========================================================================
// Helper methods
// =========================================================================

void ZoombiniPuzzleSmoke::playZmbScript(ZmbScrsCompletionMode completionMode, ZmbFeature *dispatchFeature, int16 scrsId, ZmbSnoid *snoid) {
	// Materialize SCRS frame zero immediately, then join the Snoid to the dispatch
	// feature's timing group. The first shared gate keeps frame zero without advancing it,
	// so the cart and Snoid remain on the same authored frame through the handoff.
	if (!snoid)
		return;
	// Detach the previous timing group before installing the new script.
	// Otherwise a later cart animation can inherit an unrelated leader and drift or finish at a different time.
	unregisterFeatureTimingGroup(snoid);
	markFeatureVisualCoverageDirty(snoid, false);
	if (snoid->startScrsPlayback(ZmbResource(ZmbResource::kPage, scrsId), completionMode, resolveScrsRejectState(scrsId))) {
		prepareSnoidVisualCoverage(snoid, true);
		snoid->setNeedsRedraw(true);
		joinFeatureTimingGroup(dispatchFeature, snoid);
	}
}

void ZoombiniPuzzleSmoke::updateLevel12WalkInArrivalPose() {
	for (int16 i = 0; i < 2; i++) {
		ZmbSnoid *snoid = _level12WalkInSnoids[i];
		if (!_level12WalkInPosePending[i] || !snoid)
			continue;

		const SnoidAnimState state = snoid->getAnimState();
		if (state != kSnoidAnimState000_Idle && state != kSnoidAnimState001_TurnLeft && state != kSnoidAnimState002_TurnRight)
			continue;

		// Smoke's two row-two entrants are a page-local exception. Selector zero
		// is their terminal front pose even if an earlier page left the shared
		// arrival-turn state at 1 or 2. Do not change that shared state here.
		addExternalDirtyRect(snoid->getClickRect());
		snoid->setCommonImageIndex(0);
		snoid->setFacingLeft(false);
		if (state == kSnoidAnimState000_Idle)
			snoid->setupCurrentCommonImageHotspots();
		else
			snoid->setAnimState(kSnoidAnimState000_Idle);
		snoid->clearPreparedRenderHotspots();
		snoid->setNeedsRedraw(true);
		_level12WalkInPosePending[i] = false;
	}
}

void ZoombiniPuzzleSmoke::unloadTimerScrb() {
	if (_level12ExtraFeature) {
		_level12ExtraFeature->setFlags(ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_01000000_DEFER_RENDER | ZmbFeature::FLAG_00100000_PLAY_ONCE |
									   ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00008000_LOOP_ANIM);
		loadScrbOntoFeature(_level12ExtraFeature, kResScrb11076_Level12Extra, false);
	}
}

void ZoombiniPuzzleSmoke::loadScrbOnAnswerRunner(int16 scrbId) {
	if (_answerZoneFeature) {
		_answerZoneFeature->addFlag(static_cast<ZmbFeature::Flag>(ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00008000_LOOP_ANIM));
		_answerZoneFeature->removeFlag(static_cast<ZmbFeature::Flag>(ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE |
																	 ZmbFeature::FLAG_01000000_DEFER_RENDER));
		loadScrbOntoFeature(_answerZoneFeature, scrbId, true);
	}
}

void ZoombiniPuzzleSmoke::loadScrbOnDispatchRunner(int16 scrbId, bool scheduleRender) {
	// Filter pickup uses a false render gate;
	// the lever uses true so events 3/4 can hand control to the comparison sequence.
	if (_answerZoneFeature) {
		addExternalDirtyRect(_answerZoneFeature->getClickRect());
		loadScrbOntoFeature(_answerZoneFeature, scrbId, scheduleRender);
		_answerZoneFeature->addFlag(static_cast<ZmbFeature::Flag>(ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00008000_LOOP_ANIM |
																  ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE));
		_answerZoneFeature->removeFlag(ZmbFeature::FLAG_01000000_DEFER_RENDER);
	}
}

void ZoombiniPuzzleSmoke::loadScrbOnWellRunner(int16 scrbId) {
	if (_answerZoneFeature) {
		addExternalDirtyRect(_answerZoneFeature->getClickRect());
		loadScrbOntoFeature(_answerZoneFeature, scrbId, false);
		// Set flags to 0x04188000 = OVERLAY | PLAY_ONCE | DEFER_ANIM | LOOP_ANIM
		_answerZoneFeature->addFlag(static_cast<ZmbFeature::Flag>(ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00008000_LOOP_ANIM |
																  ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE));
		_answerZoneFeature->removeFlag(ZmbFeature::FLAG_01000000_DEFER_RENDER);
	}
}

void ZoombiniPuzzleSmoke::loadScoreScrbs() {
	// During a filter drag, each non-full side displays its blinking yellow insertion mask.
	if (_leftFilterCount < 3 && _cliffLeftFeature) {
		_cliffLeftFeature->setFlags(ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00008000_LOOP_ANIM);
		loadScrbOntoFeature(_cliffLeftFeature, kResScrb11006_CliffLeft, true);
	}
	if (_rightFilterCount < 3 && _cliffRightFeature) {
		_cliffRightFeature->setFlags(ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00008000_LOOP_ANIM);
		loadScrbOntoFeature(_cliffRightFeature, kResScrb11007_CliffRight, true);
	}
}

void ZoombiniPuzzleSmoke::loadScoreDisplayScrbs() {
	// Set flags to 0x05188000 = OVERLAY | DEFER_RENDER | PLAY_ONCE | DEFER_ANIM | LOOP_ANIM
	if (_cliffLeftFeature) {
		_cliffLeftFeature->setFlags(ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_01000000_DEFER_RENDER | ZmbFeature::FLAG_00100000_PLAY_ONCE |
									ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00008000_LOOP_ANIM);
		loadScrbOntoFeature(_cliffLeftFeature, kResScrb11006_CliffLeft, false);
	}
	if (_cliffRightFeature) {
		_cliffRightFeature->setFlags(ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_01000000_DEFER_RENDER | ZmbFeature::FLAG_00100000_PLAY_ONCE |
									 ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00008000_LOOP_ANIM);
		loadScrbOntoFeature(_cliffRightFeature, kResScrb11007_CliffRight, false);
	}
}

void ZoombiniPuzzleSmoke::loadTimerScrb() {
	if (_level12ExtraFeature) {
		_level12ExtraFeature->setFlags(ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00008000_LOOP_ANIM);
		loadScrbOntoFeature(_level12ExtraFeature, kResScrb11076_Level12Extra, true);
	}
}

// =========================================================================
// Trait management
// =========================================================================

void ZoombiniPuzzleSmoke::clearZmbTraits(int16 runnerIdx) {
	ZmbFeature *runner = nullptr;
	ZmbSmokeRunnerState *state = nullptr;

	if (runnerIdx <= 1) {
		if (0 <= runnerIdx && runnerIdx < _exitRunnerCount)
			runner = _exitRunners[runnerIdx];
	} else if (runnerIdx == 7 || runnerIdx == 8) {
		if (runnerIdx < _gridRunnerCount)
			runner = _gridRunners[runnerIdx];
	}
	if (runner)
		state = findRunnerState(runner);

	if (state) {
		// This restores the complete old composite, including trait overhangs.
		runner->setNeedsRedraw(true);
		state->traits = ZmbTrait();
		state->dirtyMode = ZmbSmokeRunnerState::DirtyMode::kRecompose04;
	}
}

void ZoombiniPuzzleSmoke::clearRunnerSlot(int16 slotIdx) {
	if (0 <= slotIdx && slotIdx < 8) {
		_displayTraits[slotIdx] = ZmbTrait();
	}
}

void ZoombiniPuzzleSmoke::clearAllRunnerSlots() {
	for (int16 traitIdx = 0; traitIdx < 8; traitIdx++)
		_displayTraits[traitIdx] = ZmbTrait();
}

void ZoombiniPuzzleSmoke::clearReflectionRunners() {
	for (int16 i = 0; i < 2; i++) {
		ZmbFeature *runner = _reflectionRunners[i];
		if (!runner)
			continue;
		ZmbSmokeRunnerState *state = findRunnerState(runner);
		if (!state)
			continue;

		const Common::Rect oldRect = runner->getZSortRect();
		runner->deactivateRender();
		runner->setNeedsRedraw(true);
		state->traits = ZmbTrait();
		addExternalDirtyRect(oldRect);
	}
}

void ZoombiniPuzzleSmoke::assignZmbTraitsFromSrc(int16 dstIdx, ZmbFeature *source) {
	// Copy the four visual traits to the destination display runner.
	ZmbFeature *dstRunner = nullptr;

	if (dstIdx == 0) {
		dstRunner = (0 < _exitRunnerCount) ? _exitRunners[0] : nullptr;
	} else if (dstIdx == 1) {
		dstRunner = (1 < _exitRunnerCount) ? _exitRunners[1] : nullptr;
	} else if (dstIdx == 7) {
		dstRunner = (7 < _gridRunnerCount) ? _gridRunners[7] : nullptr;
	} else if (dstIdx == 8) {
		dstRunner = (8 < _gridRunnerCount) ? _gridRunners[8] : nullptr;
	}

	if (!dstRunner || !source)
		return;
	ZmbSmokeRunnerState *dstState = findRunnerState(dstRunner);
	if (!dstState)
		return;

	if (source->hasFlag(ZmbFeature::FLAG_00000001_TYPE_SNOID)) {
		ZmbSnoid *sourceSnoid = static_cast<ZmbSnoid *>(source);
		dstState->traits = sourceSnoid->_trait;
	} else {
		ZmbSmokeRunnerState *sourceState = findRunnerState(source);
		if (!sourceState)
			return;
		dstState->traits = sourceState->traits;
	}
	dstState->dirtyMode = ZmbSmokeRunnerState::DirtyMode::kRecompose04;
	dstRunner->setNeedsRedraw(true);

	// At levels 1-2, hide the selected cliff crystal and link it immediately behind the angled destination image.
	// Its custom renderer has a strict render-enabled state gate.
	if (dstIdx && _difficultyLevel < kPuzzleLevel3) {
		const Common::Rect oldRect = source->getZSortRect();
		if (!oldRect.isEmpty())
			addExternalDirtyRect(oldRect);
		source->deactivateRender();
		source->setNeedsRedraw(true);
		manualLinkBefore(source, dstRunner);
	}
}

void ZoombiniPuzzleSmoke::cacheZmbTraits(int16 dstIdx, ZmbFeature *source) {
	if (!source || dstIdx < 0 || 8 <= dstIdx)
		return;

	if (source->hasFlag(ZmbFeature::FLAG_00000001_TYPE_SNOID)) {
		ZmbSnoid *sourceSnoid = static_cast<ZmbSnoid *>(source);
		_displayTraits[dstIdx] = sourceSnoid->_trait;
		return;
	}

	ZmbSmokeRunnerState *sourceState = findRunnerState(source);
	if (!sourceState)
		return;
	_displayTraits[dstIdx] = sourceState->traits;
}

void ZoombiniPuzzleSmoke::loadZmbTraitsToCache() {
	// Read entries 0-2 from @ref ZoombiniPuzzleSmoke::_filterSlotRunners.
	// Copy their traits into rows 1-3 of @ref ZoombiniPuzzleSmoke::_displayTraits.
	for (int16 i = 1; i < 4; i++) {
		ZmbFeature *feature = _filterSlotRunners[i - 1];
		if (feature) {
			ZmbSmokeRunnerState *state = findRunnerState(feature);
			if (state) {
				_displayTraits[i] = state->traits;
				continue;
			}
		}
		_displayTraits[i] = ZmbTrait();
	}
}

void ZoombiniPuzzleSmoke::cacheAnswerRunnerTraits() {
	// Read traits from display runners [4..6] into display trait table slots [4..6]
	for (int16 i = 4; i < 7; i++) {
		ZmbFeature *feature = (i - 1 < 6) ? _filterSlotRunners[i - 1] : nullptr;
		if (feature) {
			ZmbSmokeRunnerState *state = findRunnerState(feature);
			if (state) {
				_displayTraits[i] = state->traits;
				continue;
			}
		}
		_displayTraits[i] = ZmbTrait();
	}
}

void ZoombiniPuzzleSmoke::cycleZmbTraitDisplay() {
	for (int16 i = 0; i < 3; i++) {
		ZmbFeature *feature = _filterSlotRunners[i];
		if (!feature)
			continue;
		ZmbSmokeRunnerState *state = findRunnerState(feature);
		if (!state)
			continue;

		state->dirtyMode = ZmbSmokeRunnerState::DirtyMode::kRecompose04;
		if (state->cycleTrait == ZmbSmokeRunnerState::CycleTrait::kNone00)
			continue;

		const int16 cycleTrait = state->getCycleTraitIndex();
		byte val = 0;

		if (i == 0) {
			val = _displayTraits[0][cycleTrait - 1];
		} else if (i == 1) {
			val = _displayTraits[1][cycleTrait - 1];
			if (!val)
				val = _displayTraits[0][cycleTrait - 1];
		} else if (i == 2) {
			val = _displayTraits[2][cycleTrait - 1];
			if (!val) {
				val = _displayTraits[1][cycleTrait - 1];
				if (!val)
					val = _displayTraits[0][cycleTrait - 1];
			}
		}

		state->traits[cycleTrait - 1] = val + 1;
		if (5 < state->traits[cycleTrait - 1])
			state->traits[cycleTrait - 1] = 1;

		_displayTraits[i + 1][cycleTrait - 1] = state->traits[cycleTrait - 1];
		state->blinkValue = state->traits[cycleTrait - 1];
		state->cyclingValue = 0;
		state->dirtyMode = ZmbSmokeRunnerState::DirtyMode::kRecompose04;
		feature->setNeedsRedraw(true);
	}
}

void ZoombiniPuzzleSmoke::advanceAnswerRunnerFrames() {
	for (int16 i = 5; 2 < i; i--) {
		ZmbFeature *feature = _filterSlotRunners[i];
		if (!feature)
			continue;
		ZmbSmokeRunnerState *state = findRunnerState(feature);
		if (!state)
			continue;

		state->dirtyMode = ZmbSmokeRunnerState::DirtyMode::kRecompose04;
		if (state->cycleTrait == ZmbSmokeRunnerState::CycleTrait::kNone00)
			continue;

		const int16 cycleTrait = state->getCycleTraitIndex();
		byte val = 0;

		switch (i) {
		case 3:
			val = _displayTraits[5][cycleTrait - 1];
			if (!val) {
				val = _displayTraits[6][cycleTrait - 1];
				if (!val)
					val = _displayTraits[7][cycleTrait - 1];
			}
			break;
		case 4:
			val = _displayTraits[6][cycleTrait - 1];
			if (!val)
				val = _displayTraits[7][cycleTrait - 1];
			break;
		case 5:
			val = _displayTraits[7][cycleTrait - 1];
			break;
		default:
			break;
		}

		state->traits[cycleTrait - 1] = val + 1;
		if (5 < state->traits[cycleTrait - 1])
			state->traits[cycleTrait - 1] = 1;

		_displayTraits[i + 1][cycleTrait - 1] = state->traits[cycleTrait - 1];
		state->blinkValue = state->traits[cycleTrait - 1];
		state->cyclingValue = 0;
		state->dirtyMode = ZmbSmokeRunnerState::DirtyMode::kRecompose04;
		feature->setNeedsRedraw(true);
	}
}

// =========================================================================
// Compare / Match logic
// =========================================================================

ZoombiniPuzzleSmoke::CompareResult ZoombiniPuzzleSmoke::compareTwoOrderLines() {
	// Compare the two central reflection runners through @ref ZoombiniPuzzleSmoke::_displayTraits.
	// Return the matching or mismatching comparison result.
	ZmbSmokeRunnerState *stateA = nullptr;
	ZmbSmokeRunnerState *stateB = nullptr;

	if (_reflectionRunners[0]) {
		_reflectionRunners[0]->activateRender();
		stateA = findRunnerState(_reflectionRunners[0]);
		if (stateA) {
			stateA->dirtyMode = ZmbSmokeRunnerState::DirtyMode::kRecompose04;
			for (int16 i = 0; i < 4; i++) {
				if (_displayTraits[i][0])
					stateA->traits[0] = _displayTraits[i][0];
				if (_displayTraits[i][1])
					stateA->traits[1] = _displayTraits[i][1];
				if (_displayTraits[i][2])
					stateA->traits[2] = _displayTraits[i][2];
				if (_displayTraits[i][3])
					stateA->traits[3] = _displayTraits[i][3];
			}
		}
	}

	if (_reflectionRunners[1]) {
		_reflectionRunners[1]->activateRender();
		stateB = findRunnerState(_reflectionRunners[1]);
		if (stateB) {
			stateB->dirtyMode = ZmbSmokeRunnerState::DirtyMode::kRecompose04;
			for (int16 traitRowIdx = 7; 3 < traitRowIdx; traitRowIdx--) {
				if (_displayTraits[traitRowIdx][0])
					stateB->traits[0] = _displayTraits[traitRowIdx][0];
				if (_displayTraits[traitRowIdx][1])
					stateB->traits[1] = _displayTraits[traitRowIdx][1];
				if (_displayTraits[traitRowIdx][2])
					stateB->traits[2] = _displayTraits[traitRowIdx][2];
				if (_displayTraits[traitRowIdx][3])
					stateB->traits[3] = _displayTraits[traitRowIdx][3];
			}
		}
	}

	if (!stateA || !stateB)
		return CompareResult::kMatch00;

	if (stateA->traits[0] != stateB->traits[0])
		return CompareResult::kMismatch02;
	if (stateA->traits[1] != stateB->traits[1])
		return CompareResult::kMismatch02;
	if (stateA->traits[2] != stateB->traits[2])
		return CompareResult::kMismatch02;
	if (stateA->traits[3] != stateB->traits[3])
		return CompareResult::kMismatch02;
	return CompareResult::kMatch00;
}

void ZoombiniPuzzleSmoke::initMatchCompareRunners() {
	_compareResult = compareTwoOrderLines();
	_bCompareSwapped = (_compareResult != CompareResult::kMatch00);

	if (_compareAFeature)
		loadScrbOntoFeature(_compareAFeature, kResScrb11018_CompareA, true);
	if (_compareBFeature)
		loadScrbOntoFeature(_compareBFeature, kResScrb11019_CompareB, true);
	registerFeatureTimingGroup(_compareAFeature, _compareBFeature);
}

void ZoombiniPuzzleSmoke::startNextCompareSequence() {
	_displayPairPositionIdx = 0;

	ZmbFeature *stackFeature = _bRunnerToggle ? _smokeStackBFeature : _smokeStackAFeature;
	if (stackFeature) {
		// Set flags to 0x05188000
		stackFeature->addFlag(static_cast<ZmbFeature::Flag>(ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_01000000_DEFER_RENDER |
															ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00008000_LOOP_ANIM));
		// Keep the current cart pixels until the new comparison runner replaces
		// them on the next render pass. Queue only the deferred invalidation here.
		if (_difficultyLevel <= kPuzzleLevel2) {
			loadScrbOntoFeature(stackFeature, _scrbSmokeStackResA, false, false);
		} else {
			_bRunnerToggle = !_bRunnerToggle;
			loadScrbOntoFeature(stackFeature, _scrbSmokeStackResB, false, false);
		}
	}

	if (_mainAnimFeature)
		loadScrbOntoFeature(_mainAnimFeature, _scrbAnimIdArr[static_cast<int16>(_compareResult)], true);

	if (_secondAnimFeature) {
		loadScrbOntoFeature(_secondAnimFeature, _scrbAnimIdArr[static_cast<int16>(_compareResult) + 1], true);
	}

	manualLinkBefore(_activeZmb, _secondAnimFeature);
	registerFeatureTimingGroup(_mainAnimFeature, _secondAnimFeature);
}

// =========================================================================
// Question / Selection
// =========================================================================

void ZoombiniPuzzleSmoke::selectQuestionZmb() {
	int16 count = 0;
	uint16 available[21] = {};
	_questionSnoidId = 0;
	_questionCrystalIdx = -1;

	for (int16 i = 0; i < _pageLoadedZmbCount; i++) {
		if (_zmbQueue[i] != 0) {
			available[count] = _zmbQueue[i];
			count += 1;
		}
	}

	if (count == 0) {
		_questionZmbCount = 0;
		return;
	}

	int16 randIdx = _vm->_rnd->getRandomNumber(count - 1);
	ZmbSnoid *snoid = getSnoid(available[randIdx]);
	if (snoid) {
		_questionSnoidId = snoid->getId();
		_questionTraits[0] = snoid->_trait;
	}

	if (_difficultyLevel < kPuzzleLevel3 || count <= 1) {
		_questionTraits[1] = ZmbTrait();
	} else {
		int16 idx2 = randIdx + 1;
		if (idx2 == count)
			idx2 = 0;
		ZmbSnoid *snoid2 = getSnoid(available[idx2]);
		if (snoid2)
			_questionTraits[1] = snoid2->_trait;
	}

	_questionZmbCount = count;
}

int16 ZoombiniPuzzleSmoke::copyPairToCompareBuffer() {
	if (_pageLoadedZmbCount <= _currentQueueIdx)
		return 0;

	int16 result = 1;
	ZmbSnoid *snoid1 = getSnoid(_zmbQueue[_currentQueueIdx]);
	if (snoid1)
		_questionTraits[0] = snoid1->_trait;

	if (_pageLoadedZmbCount <= _currentQueueIdx + 1) {
		_questionTraits[1] = ZmbTrait();
	} else {
		result = 2;
		ZmbSnoid *snoid2 = getSnoid(_zmbQueue[_currentQueueIdx + 1]);
		if (snoid2)
			_questionTraits[1] = snoid2->_trait;
	}

	return result;
}

// =========================================================================
// Runner initialization
// =========================================================================

void ZoombiniPuzzleSmoke::initLowLevelQuestionRunners(int16 count) {
	_questionCrystalIdx = -1;
	if (count <= 0)
		return;

	int16 cliffCount = (_cliffRunnerCount < 8) ? _cliffRunnerCount : 8;
	int16 randTarget = (0 < cliffCount) ? _vm->_rnd->getRandomNumber(cliffCount - 1) : 0;
	if (0 < cliffCount)
		_questionCrystalIdx = randTarget;

	for (int16 runnerIdx = 0; runnerIdx < cliffCount; runnerIdx++) {
		ZmbFeature *runner = _cliffRunners[runnerIdx];
		if (!runner)
			continue;
		ZmbSmokeRunnerState *state = findRunnerState(runner);
		if (!state)
			continue;

		runner->activateRender();
		runner->setNeedsRedraw(true);
		state->dirtyMode = ZmbSmokeRunnerState::DirtyMode::kRecompose04;
		runner->setPointLoc(kCliffRunnerPositions[runnerIdx]);

		for (int16 traitIdx = 0; traitIdx < 4; traitIdx++)
			state->traits[traitIdx] = _vm->_rnd->getRandomNumber(1, 4);

		state->orientation = ZmbSmokeRunnerState::Orientation::kBaseTwoFramed01;

		if (runnerIdx == randTarget) {
			state->traits[0] = _questionTraits[0][0] ? _questionTraits[0][0] : _vm->_rnd->getRandomNumber(1, 4);
			state->traits[1] = _questionTraits[0][1] ? _questionTraits[0][1] : _vm->_rnd->getRandomNumber(1, 4);
			state->traits[2] = _questionTraits[0][2] ? _questionTraits[0][2] : _vm->_rnd->getRandomNumber(1, 4);
			state->traits[3] = _questionTraits[0][3] ? _questionTraits[0][3] : _vm->_rnd->getRandomNumber(1, 4);
		}
	}
}

void ZoombiniPuzzleSmoke::initLevel2RunnersAllTraits() {
	for (int16 i = 0; i < _level2RunnerCount; i++) {
		ZmbFeature *runner = _level2Runners[i];
		if (!runner)
			continue;
		ZmbSmokeRunnerState *state = findRunnerState(runner);
		if (!state)
			continue;
		assignLevel2RunnerTraits(i, *state);
		runner->setPointLoc(kLevel2RunnerPositions[i]);
	}
}

void ZoombiniPuzzleSmoke::assignLevel2RunnerTraits(int16 runnerIdx, ZmbSmokeRunnerState &state) {
	// Trait-selection rules for one level-2 filter crystal. runnerIdx selects the
	// physical slot: 0/1 are the left-segment filters (source Zoombini to center mirror),
	// 2/3 are the right-segment filters (center mirror to answer mirror).
	//   - Clears @ref ZoombiniPuzzleSmoke::ZmbSmokeRunnerState::traits.
	//   - Resets per-call shuffle tables (slot: {0..4}, value: {1..6}) with shrinking bounds.
	//   - Left filter 0 clears @ref ZoombiniPuzzleSmoke::_seenTraitsA and @ref ZoombiniPuzzleSmoke::_seenTraitsB.
	//   - Outer loop runs up to 4 times; Rand budget (1-2) limits successful writes.
	//   - Left filters use the base-one framed orientation, pick via shuffle arrays, and skip if seen already equals new.
	//   - Right filters use the base-three framed orientation, with about a 34% chance to reuse @ref ZoombiniPuzzleSmoke::_seenTraitsA.
	//     Otherwise, pull from the persistent question set in @ref ZoombiniPuzzleSmoke::_questionTraits.
	//     Last-iteration fallback (i == 3 AND no actions taken) forces reuse when seen exists.

	// Clear the runner's trait mask.
	state.traits = ZmbTrait();

	// Per-call shuffle tables (local -- match reset at top of function).
	byte slotShuffle[5] = {0, 1, 2, 3, 4};
	byte valueShuffle[6] = {1, 2, 3, 4, 5, 6};
	int16 valueCursorBound = 4;
	int16 slotCursorBound = 3;

	// Clear the seen-trait arrays when starting a fresh puzzle.
	if (runnerIdx == 0) {
		_seenTraitsA = ZmbTrait();
		_seenTraitsB = ZmbTrait();
	}

	// Action budget: 1 or 2 successful writes per call.
	int16 randBudget = _vm->_rnd->getRandomNumber(1, 2);
	const int16 initialBudget = randBudget;

	for (int16 entryIdx = 0; entryIdx < 4 && 0 < randBudget; entryIdx++) {
		// Pick random cursor within current bounds.
		int16 valueCursor = _vm->_rnd->getRandomNumber(0, valueCursorBound);
		int16 slotCursor = _vm->_rnd->getRandomNumber(0, slotCursorBound);

		if (runnerIdx < 2) {
			// Left filters: fresh traits only, using the base-one framed orientation.
			state.orientation = ZmbSmokeRunnerState::Orientation::kBaseOneFramed00;
			byte pickedSlot = slotShuffle[slotCursor];
			byte pickedValue = valueShuffle[valueCursor];
			if (_seenTraitsA[pickedSlot] != pickedValue) {
				state.traits[pickedSlot] = pickedValue;
				// Store the two fixed left filters directly in comparison rows 1 and 2.
				_displayTraits[runnerIdx + 1][pickedSlot] = pickedValue;
				_seenTraitsA[pickedSlot] = pickedValue;
				randBudget -= 1;
			}
		} else if (runnerIdx == 2 || runnerIdx == 3) {
			// Right filters use the base-three framed orientation.
			// They reuse previously seen traits only on a 66..100 roll or the final-iteration fallback.
			state.orientation = ZmbSmokeRunnerState::Orientation::kBaseThreeFramed02;
			int16 slotIdx = _vm->_rnd->getRandomNumber(0, 3);

			if (_seenTraitsA[slotIdx]) {
				const bool reuseRoll = 65 < _vm->_rnd->getRandomNumber(0, 100);
				const bool lastIterFallback = (randBudget == initialBudget && entryIdx == 3);
				if (reuseRoll || lastIterFallback) {
					// REUSE path.
					state.traits[slotIdx] = _seenTraitsA[slotIdx];
					// Map fixed filters 2 and 3 to the right-side comparison rows 4 and 5.
					_displayTraits[runnerIdx + 2][slotIdx] = _seenTraitsA[slotIdx];
					_seenTraitsB[slotIdx] = _seenTraitsA[slotIdx];
					randBudget -= 1;
				}
				// Else: no action -- outer loop continues without consuming budget.
			} else {
				// Draw a fresh value from the persistent starter set in @ref ZoombiniPuzzleSmoke::_questionTraits.
				byte starterTrait = _questionTraits[0][slotIdx];
				state.traits[slotIdx] = starterTrait;
				_displayTraits[runnerIdx + 2][slotIdx] = starterTrait;
				_seenTraitsB[slotIdx] = starterTrait;
				randBudget -= 1;
			}
		}

		// Shrink shuffle arrays.
		for (int16 slotIdx = slotCursor; slotIdx < slotCursorBound; slotIdx++)
			slotShuffle[slotIdx] = slotShuffle[slotIdx + 1];
		slotCursorBound -= 1;

		for (int16 valueIdx = valueCursor; valueIdx < valueCursorBound; valueIdx++)
			valueShuffle[valueIdx] = valueShuffle[valueIdx + 1];
		valueCursorBound -= 1;

		if (slotCursorBound < 0 || valueCursorBound < 0)
			break;
	}

	// Persist newly seen traits in @ref ZoombiniPuzzleSmoke::_questionTraits for the next call.
	for (int16 traitIdx = 0; traitIdx < 4; traitIdx++) {
		if (_seenTraitsA[traitIdx])
			_questionTraits[0][traitIdx] = _seenTraitsA[traitIdx];
	}

	if (runnerIdx == 3) {
		for (int16 row = 5; 3 < row; row--) {
			for (int16 trait = 0; trait < 4; trait++) {
				if (_displayTraits[row][trait])
					_seenTraitsB[trait] = _displayTraits[row][trait];
			}
		}
		for (int16 trait = 0; trait < 4; trait++) {
			if (_questionTraits[0][trait] == _seenTraitsB[trait])
				_questionTraits[0][trait] = 0;
		}
	}

	state.dirtyMode = ZmbSmokeRunnerState::DirtyMode::kRecompose04;
}

void ZoombiniPuzzleSmoke::initLevel3RunnersAllTraits() {
	int16 availablePositions[8];
	for (int16 i = 0; i < 8; i++) {
		availablePositions[i] = i;
		_filterHomePermutation[i] = i;
	}

	if (!_bPreserveFilterHomePositionOrder) {
		int16 lastAvailable = 6;
		for (int16 snoidIdx = 1; snoidIdx < 7; snoidIdx++) {
			const int16 picked = _vm->_rnd->getRandomNumber(1, lastAvailable);
			_filterHomePermutation[snoidIdx] = availablePositions[picked];
			for (int16 positionIdx = picked; positionIdx < lastAvailable + 1; positionIdx++)
				availablePositions[positionIdx] = availablePositions[positionIdx + 1];
			lastAvailable -= 1;
		}
	}

	for (int16 i = 1; i < _gridRunnerCount && i < 7; i++) {
		ZmbFeature *runner = _gridRunners[i];
		if (!runner)
			continue;
		ZmbSmokeRunnerState *state = findRunnerState(runner);
		if (!state)
			continue;
		generateTraitGrid(i, *state);
		state->orientation = ZmbSmokeRunnerState::Orientation::kBaseTwoFramedGrid07;
		// A filter can finish the previous comparison at either platform.
		// Keep it render-active and dirty its old rectangle before rebuilding it at the newly shuffled home position.
		runner->activateRender();
		// This table starts at template+0x40.
		// Permutation values 1 through 6 therefore map to table indices 0 through 5.
		setSmokeRunnerPoint(runner, kGridRunnerPositions[_filterHomePermutation[i] - 1], ZmbSmokeRunnerState::DirtyMode::kRecompose04);
	}

	if (7 < _gridRunnerCount && _gridRunners[7]) {
		ZmbSmokeRunnerState *state = findRunnerState(_gridRunners[7]);
		if (state) {
			generateTraitGrid(7, *state);
			state->orientation = ZmbSmokeRunnerState::Orientation::kBaseOneFramed00;
			setSmokeRunnerPoint(_gridRunners[7], kCliffDropSnapPosition, ZmbSmokeRunnerState::DirtyMode::kRecompose04);
		}
		_targetDisplayRunner = _gridRunners[7];
		assignZmbTraitsFromSrc(7, _targetDisplayRunner);
		cacheZmbTraits(7, _targetDisplayRunner);
	}

	if (8 < _gridRunnerCount && _gridRunners[8]) {
		ZmbSmokeRunnerState *state = findRunnerState(_gridRunners[8]);
		if (state) {
			generateTraitGrid(8, *state);
			state->orientation = ZmbSmokeRunnerState::Orientation::kBaseThreeFramed02;
			// This is the rightmost L4 target position, not exit-runner position +0x70.
			setSmokeRunnerPoint(_gridRunners[8], kLevel4TransitionPositions[3], ZmbSmokeRunnerState::DirtyMode::kRecompose04);
		}
		_sourceDisplayRunner = _gridRunners[8];
	}

	_activeZmb = (_currentQueueIdx < _pageLoadedZmbCount) ? getSnoid(_zmbQueue[_currentQueueIdx]) : nullptr;
	if (_activeZmb) {
		assignZmbTraitsFromSrc(0, _activeZmb);
		cacheZmbTraits(0, _activeZmb);
	}
	_leftFilterCount = 0;
	_rightFilterCount = 0;
	memset(_filterSlotRunners, 0, sizeof(_filterSlotRunners));
}

void ZoombiniPuzzleSmoke::generateTraitGrid(int16 rowIndex, ZmbSmokeRunnerState &state) {
	// Row 1 is the generator trigger. Later calls only copy their row into
	// the corresponding custom runner.
	state.suppressShapes = false;
	if (rowIndex == 1) {
		for (int16 traitIdx = 0; traitIdx < 9; traitIdx++) {
			_primaryGridTraits[traitIdx] = ZmbTrait();
			_secondaryGridTraits[traitIdx] = ZmbTrait();
			_gridMatchTraits[traitIdx] = ZmbTrait();
		}

		_primaryGridTraits[0] = _questionTraits[0];
		_secondaryGridTraits[0] = _questionTraits[1];
		ZmbTrait lastPrimary = _questionTraits[0];
		ZmbTrait lastSecondary = _questionTraits[1];

		for (int16 row = 1; row < 3; row++) {
			byte valuePool[8];
			for (int16 poolIdx = 0; poolIdx < 8; poolIdx++)
				valuePool[poolIdx] = poolIdx;
			int16 filledCount = 0;
			bool madeMatch = false;
			int16 lastValueIndex = 5;
			const int16 matchColumn = _vm->_rnd->getRandomNumber(0, 4);

			for (int16 col = 0; col < 4; col++) {
				if (2 <= filledCount)
					continue;
				const int16 valueIndex = _vm->_rnd->getRandomNumber(1, lastValueIndex);
				if (col == matchColumn && 70 < _vm->_rnd->getRandomNumber(0, 100) && !madeMatch) {
					madeMatch = true;
					const byte primary = lastPrimary[col] ? lastPrimary[col] : _questionTraits[0][col];
					const byte secondary = lastSecondary[col] ? lastSecondary[col] : _questionTraits[1][col];
					_primaryGridTraits[row][col] = (primary == 5) ? 1 : primary + 1;
					_secondaryGridTraits[row][col] = (secondary == 5) ? 1 : secondary + 1;
					_gridMatchTraits[row][col] = _primaryGridTraits[row][col];
				} else if (40 < _vm->_rnd->getRandomNumber(0, 100) || (col == 3 && !filledCount)) {
					_primaryGridTraits[row][col] = valuePool[valueIndex];
					_secondaryGridTraits[row][col] = valuePool[valueIndex];
				}

				if (_primaryGridTraits[row][col]) {
					lastPrimary[col] = _primaryGridTraits[row][col];
					lastSecondary[col] = _secondaryGridTraits[row][col];
					filledCount += 1;
					for (int16 poolIdx = valueIndex; poolIdx < lastValueIndex + 1; poolIdx++)
						valuePool[poolIdx] = valuePool[poolIdx + 1];
					lastValueIndex -= 1;
				}
			}
		}

		if (_difficultyLevel == kPuzzleLevel3 || _difficultyLevel == kPuzzleLevel4) {
			ZmbTrait carryPrimary = lastPrimary;
			ZmbTrait carrySecondary = lastSecondary;

			for (int16 row = 3; row < 5; row++) {
				byte valuePool[8];
				for (int16 poolIdx = 0; poolIdx < 8; poolIdx++)
					valuePool[poolIdx] = poolIdx;
				int16 filledCount = 0;
				bool madeMatch = false;
				int16 lastValueIndex = 5;
				_vm->_rnd->getRandomNumber(0, 4);

				for (int16 col = 0; col < 4; col++) {
					if (2 <= filledCount)
						continue;
					const int16 valueIndex = _vm->_rnd->getRandomNumber(1, lastValueIndex);
					const bool useNonMatchPath =
						(_vm->_rnd->getRandomNumber(0, 100) <= 70 && (col != 3 || filledCount)) || madeMatch;

					if (!useNonMatchPath) {
						madeMatch = true;
						if (row == 3) {
							_primaryGridTraits[row][col] = carryPrimary[col] ? carryPrimary[col] : _questionTraits[0][col];
							_secondaryGridTraits[row][col] = carrySecondary[col] ? carrySecondary[col] : _questionTraits[1][col];
						} else if (_gridMatchTraits[row - 1][col]) {
							_primaryGridTraits[row][col] = (carryPrimary[col] <= 1) ? 5 : carryPrimary[col] - 1;
							_secondaryGridTraits[row][col] = (carrySecondary[col] <= 1) ? 5 : carrySecondary[col] - 1;
						} else if (_primaryGridTraits[row - 1][col]) {
							_primaryGridTraits[row][col] = valuePool[valueIndex];
							_secondaryGridTraits[row][col] = valuePool[valueIndex];
						} else {
							_primaryGridTraits[row][col] = carryPrimary[col] ? carryPrimary[col] : _questionTraits[0][col];
							_secondaryGridTraits[row][col] = carrySecondary[col] ? carrySecondary[col] : _questionTraits[1][col];
						}
						_gridMatchTraits[row][col] = _primaryGridTraits[row][col];
					} else if (row == 3) {
						if ((!_gridMatchTraits[2][col] && _primaryGridTraits[2][col]) ||
							(!_gridMatchTraits[1][col] && _primaryGridTraits[1][col])) {
							_primaryGridTraits[3][col] = carryPrimary[col];
							_secondaryGridTraits[3][col] = carrySecondary[col];
						}
					} else if (_gridMatchTraits[row - 1][col]) {
						if (!madeMatch) {
							_primaryGridTraits[row][col] = (carryPrimary[col] <= 1) ? 5 : carryPrimary[col] - 1;
							_secondaryGridTraits[row][col] = (carrySecondary[col] <= 1) ? 5 : carrySecondary[col] - 1;
							madeMatch = true;
							_gridMatchTraits[row][col] = _primaryGridTraits[row][col];
						}
					} else if (_primaryGridTraits[row - 1][col]) {
						_primaryGridTraits[row][col] = valuePool[valueIndex];
						_secondaryGridTraits[row][col] = valuePool[valueIndex];
					} else if ((!_gridMatchTraits[2][col] && _primaryGridTraits[2][col]) ||
							   (!_gridMatchTraits[1][col] && _primaryGridTraits[1][col])) {
						_primaryGridTraits[row][col] = carryPrimary[col];
						_secondaryGridTraits[row][col] = carrySecondary[col];
					}

					if (_primaryGridTraits[row][col]) {
						carryPrimary[col] = _primaryGridTraits[row][col];
						carrySecondary[col] = _secondaryGridTraits[row][col];
						filledCount += 1;
						for (int16 poolIdx = valueIndex; poolIdx < lastValueIndex + 1; poolIdx++)
							valuePool[poolIdx] = valuePool[poolIdx + 1];
						lastValueIndex -= 1;
					}
				}
			}

			for (int16 col = 0; col < 4; col++) {
				if (_gridMatchTraits[4][col])
					_primaryGridTraits[7][col] = (_primaryGridTraits[4][col] <= 1) ? 5 : _primaryGridTraits[4][col] - 1;
				else if (_primaryGridTraits[4][col])
					_primaryGridTraits[7][col] = _vm->_rnd->getRandomNumber(1, 5);
				else if (_gridMatchTraits[3][col])
					_primaryGridTraits[7][col] = (_primaryGridTraits[3][col] <= 1) ? 5 : _primaryGridTraits[3][col] - 1;
				else if (_primaryGridTraits[3][col])
					_primaryGridTraits[7][col] = _vm->_rnd->getRandomNumber(1, 5);
				else
					_primaryGridTraits[7][col] = carryPrimary[col];
			}

			if (_questionTraits[1][0]) {
				for (int16 col = 0; col < 4; col++) {
					if (_gridMatchTraits[4][col])
						_secondaryGridTraits[8][col] = (_secondaryGridTraits[4][col] <= 1) ? 5 : _secondaryGridTraits[4][col] - 1;
					else if (_secondaryGridTraits[4][col])
						_secondaryGridTraits[8][col] = _vm->_rnd->getRandomNumber(1, 5);
					else if (_gridMatchTraits[3][col])
						_secondaryGridTraits[8][col] = (_secondaryGridTraits[3][col] <= 1) ? 5 : _secondaryGridTraits[3][col] - 1;
					else if (_secondaryGridTraits[3][col])
						_secondaryGridTraits[8][col] = _vm->_rnd->getRandomNumber(1, 5);
					else
						_secondaryGridTraits[8][col] = carrySecondary[col];
				}
			}
		}

		if (_difficultyLevel == kPuzzleLevel3) {
			for (int16 row = 5; row < 7; row++) {
				byte valuePool[8];
				for (int16 poolIdx = 0; poolIdx < 8; poolIdx++)
					valuePool[poolIdx] = poolIdx;
				int16 filledCount = 0;
				int16 lastValueIndex = 5;
				const int16 matchColumn = _vm->_rnd->getRandomNumber(0, 3);
				for (int16 col = 0; col < 4; col++) {
					if (2 <= filledCount)
						continue;
					const int16 valueIndex = _vm->_rnd->getRandomNumber(1, lastValueIndex);
					if (col == matchColumn && 70 < _vm->_rnd->getRandomNumber(0, 100)) {
						_primaryGridTraits[row][col] = valuePool[valueIndex];
						_gridMatchTraits[row][col] = valuePool[valueIndex];
					} else if (40 < _vm->_rnd->getRandomNumber(0, 100) || (col == 3 && !filledCount)) {
						_primaryGridTraits[row][col] = valuePool[valueIndex];
					}
					if (_primaryGridTraits[row][col]) {
						filledCount += 1;
						for (int16 poolIdx = valueIndex; poolIdx < lastValueIndex + 1; poolIdx++)
							valuePool[poolIdx] = valuePool[poolIdx + 1];
						lastValueIndex -= 1;
					}
				}
			}
		} else if (_difficultyLevel == kPuzzleLevel4) {
			if (_vm->_rnd->getRandomBool()) {
				const int16 sourceRow = _vm->_rnd->getRandomNumber(1, 2);
				for (int16 col = 0; col < 4; col++) {
					if (_gridMatchTraits[sourceRow][col]) {
						_primaryGridTraits[5][col] = _primaryGridTraits[sourceRow][col];
						_gridMatchTraits[5][col] = _primaryGridTraits[sourceRow][col];
					} else if (_primaryGridTraits[sourceRow][col]) {
						if (_primaryGridTraits[sourceRow][col] == 5)
							_primaryGridTraits[5][col] = 1;
						else
							_primaryGridTraits[5][col] = _primaryGridTraits[sourceRow][col] + 1;
					}
				}
				_vm->_rnd->getRandomNumber(3, 4);
				const int16 randomColumn = _vm->_rnd->getRandomNumber(0, 3);
				_primaryGridTraits[6][randomColumn] = _vm->_rnd->getRandomNumber(1, 5);
			} else {
				const int16 sourceRow = _vm->_rnd->getRandomNumber(3, 4);
				for (int16 col = 0; col < 4; col++) {
					if (_gridMatchTraits[sourceRow][col]) {
						_primaryGridTraits[6][col] = _primaryGridTraits[sourceRow][col];
						_gridMatchTraits[6][col] = _primaryGridTraits[sourceRow][col];
					} else if (_primaryGridTraits[sourceRow][col]) {
						if (_primaryGridTraits[sourceRow][col] == 5)
							_primaryGridTraits[6][col] = 1;
						else
							_primaryGridTraits[6][col] = _primaryGridTraits[sourceRow][col] + 1;
					}
				}
				_vm->_rnd->getRandomNumber(1, 2);
				const int16 randomColumn = _vm->_rnd->getRandomNumber(0, 3);
				_primaryGridTraits[5][randomColumn] = _vm->_rnd->getRandomNumber(1, 5);
			}
		}
	}

	int16 matchColumn = 0;
	for (int16 col = 0; col < 4; col++) {
		if (rowIndex == 8) {
			if (_questionTraits[1][0])
				state.traits[col] = _secondaryGridTraits[8][col];
			else
				state.traits[col] = 0;
		} else {
			state.traits[col] = _primaryGridTraits[rowIndex][col];
		}
		if (_gridMatchTraits[rowIndex][col])
			matchColumn = col + 1;
	}
	state.cyclingValue = matchColumn ? 1 : 0;
	if (matchColumn)
		state.cycleTrait = static_cast<ZmbSmokeRunnerState::CycleTrait>(matchColumn);
	else
		state.cycleTrait = ZmbSmokeRunnerState::CycleTrait::kNone00;
}

// =========================================================================
// Rejection animation
// =========================================================================

void ZoombiniPuzzleSmoke::playRejectedAnimation() {
	if (_rejectionFeature) {
		loadScrbOntoFeature(_rejectionFeature, _placedZmbCount + kResScrb11036_RejectionBase, true);
		manualLinkBefore(_rejectionFeature, _backgroundFeature);
	}

	ZmbSnoid *zmb = _activeZmb;
	if (zmb && _rejectionFeature) {
		// Its adjusted callback 0 turns the Snoid for the return leg.
		zmb->setFacingLeft(true);
		zmb->setPointLoc(kRejectPosition);
		playZmbScript(ZmbScrsCompletionMode::kReturnToIdle, nullptr, _placedZmbCount + kResScrs12020_RejectionBase, zmb);
		manualLinkAfter(zmb, _rejectionFeature);
		registerFeatureTimingGroup(_rejectionFeature, zmb);
	}
}

// =========================================================================
// Stack building / spawning
// =========================================================================

void ZoombiniPuzzleSmoke::buildRunnerStacks() {
	clearAllRunnerSlots();

	switch (_difficultyLevel) {
	case kPuzzleLevel1:
		spawnStackRunners(8, RunnerType::kCliff01);
		spawnStackRunners(2, RunnerType::kExit04);
		spawnStackRunners(2, RunnerType::kBottom05);
		break;
	case kPuzzleLevel2:
		spawnStackRunners(4, RunnerType::kLevel02);
		spawnStackRunners(8, RunnerType::kCliff01);
		spawnStackRunners(2, RunnerType::kExit04);
		spawnStackRunners(2, RunnerType::kBottom05);
		break;
	case kPuzzleLevel3:
		spawnStackRunners(7, RunnerType::kGrid03);
		spawnStackRunners(1, RunnerType::kExit04);
		spawnStackRunners(2, RunnerType::kBottom05);
		break;
	case kPuzzleLevel4:
		spawnStackRunners(8, RunnerType::kGrid03);
		spawnStackRunners(1, RunnerType::kExit04);
		spawnStackRunners(2, RunnerType::kBottom05);
		break;
	default:
		break;
	}
}

void ZoombiniPuzzleSmoke::spawnStackRunners(int16 count, RunnerType runnerType) {
	if (count <= 0)
		return;

	int16 randTarget = _vm->_rnd->getRandomNumber(count - 1);

	for (int16 runnerIdx = 0; runnerIdx < count; runnerIdx++) {
		Common::Point pos;
		ZmbSmokeRunnerState tempState = {};

		switch (runnerType) {
		case RunnerType::kCliff01:
			for (int16 traitIdx = 0; traitIdx < 4; traitIdx++)
				tempState.traits[traitIdx] = _vm->_rnd->getRandomNumber(1, 5);
			tempState.orientation = ZmbSmokeRunnerState::Orientation::kBaseTwoFramed01;
			tempState.dirtyMode = ZmbSmokeRunnerState::DirtyMode::kRecompose04;
			pos = kCliffRunnerPositions[runnerIdx];

			if (runnerIdx == randTarget) {
				tempState.traits[0] = _questionTraits[0][0] ? _questionTraits[0][0] : _vm->_rnd->getRandomNumber(1, 5);
				tempState.traits[1] = _questionTraits[0][1] ? _questionTraits[0][1] : _vm->_rnd->getRandomNumber(1, 5);
				tempState.traits[2] = _questionTraits[0][2] ? _questionTraits[0][2] : _vm->_rnd->getRandomNumber(1, 5);
				tempState.traits[3] = _questionTraits[0][3] ? _questionTraits[0][3] : _vm->_rnd->getRandomNumber(1, 5);
			}
			break;
		case RunnerType::kLevel02:
			assignLevel2RunnerTraits(runnerIdx, tempState);
			pos = kLevel2RunnerPositions[runnerIdx];
			break;
		case RunnerType::kGrid03:
			generateTraitGrid(runnerIdx + 1, tempState);
			if (runnerIdx + 1 < 7)
				tempState.orientation = ZmbSmokeRunnerState::Orientation::kBaseTwoFramedGrid07;
			if (runnerIdx == 6)
				tempState.orientation = ZmbSmokeRunnerState::Orientation::kBaseOne05;
			if (runnerIdx == 7)
				tempState.orientation = ZmbSmokeRunnerState::Orientation::kBaseThree03;
			tempState.dirtyMode = ZmbSmokeRunnerState::DirtyMode::kRecompose04;
			pos = kGridRunnerPositions[runnerIdx];
			break;
		case RunnerType::kExit04:
			tempState.traits = ZmbTrait();
			tempState.dirtyMode = ZmbSmokeRunnerState::DirtyMode::kRecompose04;
			if (runnerIdx == 0) {
				tempState.orientation = ZmbSmokeRunnerState::Orientation::kBaseThreeFramedExit08;
				pos = kExitRunnerPositions[0];
			} else if (runnerIdx == 1) {
				tempState.orientation = ZmbSmokeRunnerState::Orientation::kBaseOne05;
				pos = kExitRunnerPositions[1];
			}
			break;
		case RunnerType::kBottom05:
			tempState.traits = ZmbTrait();
			tempState.dirtyMode = ZmbSmokeRunnerState::DirtyMode::kRecompose04;
			pos = kBottomRunnerPositions[runnerIdx];
			if (runnerIdx == 0)
				tempState.orientation = ZmbSmokeRunnerState::Orientation::kBaseOne05;
			else
				tempState.orientation = ZmbSmokeRunnerState::Orientation::kBaseThree03;
			break;
		default:
			continue;
		}

		// Register a callback-only runner: resource id 0, interval 6, as its custom materializer,
		// and tBMP/REGS 10000 for the actual crystal shapes.
		// A normal SCRB 11000 feature cannot represent this runner.
		ZmbFeature::EventHooks hooks;
		hooks.setRenderFunc(static_cast<ZmbFeature::OnRenderFunc>(&ZoombiniPuzzleSmoke::smokeRunner_render));
		hooks.setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniPuzzleSmoke::smokeRunner_preRenderShape));
		ZmbFeature *runner = loadVirtualFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape10000_Crystal), 0, 6,
			ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00000002_TYPE_GRIDWALKER,
			hooks);

		if (!runner)
			continue;

		runner->setPointLoc(pos);
		runner->setShapeOffsetRegs(_shapeOffsetRegsMap[kResRegs10000_Smoke]);
		Common::Array<ZmbHotspot> initialHotspots;
		initialHotspots.push_back(ZmbHotspot(0, 1, 0, pos.x, pos.y));
		runner->setVirtualHotspots(initialHotspots);

		switch (runnerType) {
		case RunnerType::kCliff01:
			if (_cliffRunnerCount < 20) {
				const int16 cliffRunnerIdx = _cliffRunnerCount;
				_cliffRunners[_cliffRunnerCount] = runner;
				_cliffRunnerStates[_cliffRunnerCount] = tempState;
				_cliffRunnerCount += 1;
				if (runnerIdx == randTarget)
					_questionCrystalIdx = cliffRunnerIdx;
			}
			manualLinkBefore(runner, _secondAnimFeature);
			break;
		case RunnerType::kLevel02:
			if (_level2RunnerCount < 6) {
				_level2Runners[_level2RunnerCount] = runner;
				_level2RunnerStates[_level2RunnerCount] = tempState;
				_level2RunnerCount += 1;
			}
			manualLinkAfter(runner, _compareAFeature);
			break;
		case RunnerType::kGrid03:
			if (_gridRunnerCount < 9) {
				_gridRunners[_gridRunnerCount] = runner;
				_gridRunnerStates[_gridRunnerCount] = tempState;
				_gridRunnerCount += 1;

				// Increment the count before initializing runners 7 and 8.
				// @ref ZoombiniPuzzleSmoke::findRunnerState() must see the newly registered runner.
				if (_gridRunnerCount == 8 && kPuzzleLevel3 <= _difficultyLevel) {
					_targetDisplayRunner = runner;
					assignZmbTraitsFromSrc(7, runner);
					cacheZmbTraits(7, runner);
					_matchRunnerIndex = 1;
				}
				if (_gridRunnerCount == 9 && kPuzzleLevel3 <= _difficultyLevel) {
					_sourceDisplayRunner = runner;
					assignZmbTraitsFromSrc(8, runner);
				}
			}
			manualLinkBefore(runner, _secondAnimFeature);
			break;
		case RunnerType::kExit04:
			if (_exitRunnerCount < 4) {
				_exitRunners[_exitRunnerCount] = runner;
				_exitRunnerStates[_exitRunnerCount] = tempState;
				_exitRunnerCount += 1;
			}
			manualLinkAfter(runner, _secondAnimFeature);
			break;
		case RunnerType::kBottom05:
			if (_reflectionRunnerCount < 2) {
				_reflectionRunners[_reflectionRunnerCount] = runner;
				_reflectionRunnerStates[_reflectionRunnerCount] = tempState;
				_reflectionRunnerCount += 1;
			}
			// Event 16 moves these reflection runners. Keeping them after the
			// main mirror runner makes the new position visible in that frame.
			manualLinkAfter(runner, _mainAnimFeature);
			runner->deactivateRender();
			runner->setNeedsRedraw(true);
			break;
		}
	}
}

ZmbRenderResult ZoombiniPuzzleSmoke::smokeRunner_render(ZmbFeature *feature) {
	// Unlike the standard non-Snoid renderer, this callback never draws an inactive custom runner.
	if (!feature || !feature->isRenderActivated() ||
		!Common::Rect(0, 0, 640, 480).contains(feature->getPointLoc()))
		return ZmbRenderResult::kSkipped;

	return blitShapes(feature);
}

ZoombiniGraphics::PaletteRemapMode ZoombiniPuzzleSmoke::getColorAssistPaletteRemap(
	const ZmbFeature *feature, const ZmbHotspot &hotspot, ZmbResource resource) const {
	(void)feature;
	if (!_vm->isColorBlindModeEnabled() ||
		resource != ZmbResource(ZmbResource::kPage, kResBitmapShape10000_Crystal))
		return ZoombiniGraphics::kPaletteRemapNone;

	const uint16 shapeIdx = hotspot._shapeIdx;
	// The custom atlas stores the five Nose shapes after each orientation's body and Eyes banks.
	if ((12 <= shapeIdx && shapeIdx <= 16) ||
		(33 <= shapeIdx && shapeIdx <= 37) ||
		(54 <= shapeIdx && shapeIdx <= 58))
		return ZoombiniGraphics::kPaletteRemapNoseNet;

	return ZoombiniGraphics::kPaletteRemapNone;
}

void ZoombiniPuzzleSmoke::smokeRunner_preRenderShape(ZmbFeature *feature,
													 ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)hsGroup;

	// Each orientation selects a body shape and an optional surrounding frame.
	static constexpr int16 kBodyShapeBase[9] = {1, 22, 43, 43, 22, 1, 1, 22, 43};
	static constexpr int16 kFrameShape[9] = {65, 64, 66, 0, 0, 0, 65, 69, 67};

	hotspots.clear();
	ZmbSmokeRunnerState *state = findRunnerState(feature);
	if (!state || state->suppressShapes)
		return;

	const Common::Point pos = feature->getPointLoc();
	const byte orientation = static_cast<byte>(state->orientation);
	if (8 < orientation)
		return;

	uint16 hotspotId = 0;
	const int16 frameShape = kFrameShape[orientation];
	if (frameShape) {
		hotspots.push_back(ZmbHotspot(hotspotId, frameShape, 0, pos.x, pos.y));
		hotspotId += 1;
	}

	const int16 bodyShape = kBodyShapeBase[orientation];
	if (state->traits[0] && state->traits[1] && state->traits[2] && state->traits[3]) {
		hotspots.push_back(ZmbHotspot(hotspotId, bodyShape, 0, pos.x, pos.y));
		hotspotId += 1;
	}
	if (state->traits[3]) {
		hotspots.push_back(ZmbHotspot(hotspotId, bodyShape + state->traits[3] + 15, 0, pos.x, pos.y));
		hotspotId += 1;
	}
	if (state->traits[1]) {
		hotspots.push_back(ZmbHotspot(hotspotId, bodyShape + state->traits[1] + 5, 0, pos.x, pos.y));
		hotspotId += 1;
	}
	if (state->traits[2]) {
		hotspots.push_back(ZmbHotspot(hotspotId, bodyShape + state->traits[2] + 10, 0, pos.x, pos.y));
		hotspotId += 1;
	}
	if (state->traits[0])
		hotspots.push_back(ZmbHotspot(hotspotId, bodyShape + state->traits[0], 0, pos.x, pos.y));
}

void ZoombiniPuzzleSmoke::tickSmokeRunner(ZmbFeature *feature, ZmbSmokeRunnerState &state,
										  uint32 currentFrame) {
	// The custom runner advances on a six-tick timer.
	// Each tick independently advances either a cycling glyph or a frozen glyph's blink phase.
	if (!feature || !feature->isRenderActivated() || currentFrame < state.nextRenderFrame)
		return;

	state.nextRenderFrame = currentFrame + feature->getFrameInterval();
	const int16 cycleTrait = state.getCycleTraitIndex();
	if (state.cyclingValue && state.nextTraitUpdateFrame <= currentFrame) {
		state.nextTraitUpdateFrame = currentFrame + 60;
		state.cyclingValue += 1;
		if (5 < state.cyclingValue)
			state.cyclingValue = 1;
		if (0 < cycleTrait && cycleTrait <= 4)
			state.traits[cycleTrait - 1] = state.cyclingValue;
		state.dirtyMode = ZmbSmokeRunnerState::DirtyMode::kRecompose04;
	} else if (state.blinkValue && state.nextTraitUpdateFrame <= currentFrame) {
		state.nextTraitUpdateFrame = currentFrame + 30;
		if (0 < cycleTrait && cycleTrait <= 4) {
			byte &trait = state.traits[cycleTrait - 1];
			trait = trait ? 0 : state.blinkValue;
		}
		state.dirtyMode = ZmbSmokeRunnerState::DirtyMode::kRecompose04;
	}

	if (state.dirtyMode != ZmbSmokeRunnerState::DirtyMode::kSuppressRedraw02)
		feature->setNeedsRedraw(true);
}

void ZoombiniPuzzleSmoke::tickSmokeRunners() {
	const uint32 currentFrame = getCurrentFrameCounter();
	for (int16 i = 0; i < _cliffRunnerCount; i++)
		tickSmokeRunner(_cliffRunners[i], _cliffRunnerStates[i], currentFrame);
	for (int16 i = 0; i < _level2RunnerCount; i++)
		tickSmokeRunner(_level2Runners[i], _level2RunnerStates[i], currentFrame);
	for (int16 i = 1; i < _gridRunnerCount; i++)
		tickSmokeRunner(_gridRunners[i], _gridRunnerStates[i], currentFrame);
	for (int16 i = 0; i < _exitRunnerCount; i++)
		tickSmokeRunner(_exitRunners[i], _exitRunnerStates[i], currentFrame);
	for (int16 i = 0; i < _reflectionRunnerCount; i++)
		tickSmokeRunner(_reflectionRunners[i], _reflectionRunnerStates[i], currentFrame);
}

void ZoombiniPuzzleSmoke::setSmokeRunnerPoint(ZmbFeature *feature, const Common::Point &point, ZmbSmokeRunnerState::DirtyMode dirtyMode) {
	if (!feature)
		return;

	// Merge the old custom click rectangle before recomputing the runner at its new position.
	const Common::Rect oldRect = feature->getZSortRect();
	if (!oldRect.isEmpty())
		addExternalDirtyRect(oldRect);

	feature->setPointLoc(point);
	ZmbSmokeRunnerState *state = findRunnerState(feature);
	if (state)
		state->dirtyMode = dirtyMode;
	feature->setNeedsRedraw(true);
}

// =========================================================================
// Reset
// =========================================================================

void ZoombiniPuzzleSmoke::resetAndReinitLevel() {
	clearZmbTraits(0);
	if (_difficultyLevel <= kPuzzleLevel2)
		clearZmbTraits(1);
	else
		clearZmbTraits(7);
	clearReflectionRunners();
	clearAllRunnerSlots();

	switch (_difficultyLevel) {
	case kPuzzleLevel1:
		initLowLevelQuestionRunners(_questionZmbCount);
		break;
	case kPuzzleLevel2:
		initLevel2RunnersAllTraits();
		initLowLevelQuestionRunners(_questionZmbCount);
		break;
	case kPuzzleLevel3:
		initLevel3RunnersAllTraits();
		break;
	case kPuzzleLevel4:
		if (_targetDisplayRunner) {
			ZmbSmokeRunnerState *targetState = findRunnerState(_targetDisplayRunner);
			if (targetState) {
				// Clear the first materialized hotspot shape, not the first trait byte.
				// Shape 0 terminates the whole list.
				targetState->suppressShapes = true;
				targetState->dirtyMode = ZmbSmokeRunnerState::DirtyMode::kSuppressRedraw02;
				_targetDisplayRunner->setNeedsRedraw(true);
			}
		}
		if (_sourceDisplayRunner) {
			ZmbSmokeRunnerState *srcState = findRunnerState(_sourceDisplayRunner);
			if (srcState) {
				// Apply the same hotspot-list sentinel to grid runner 8 while preserving its generated trait payload.
				srcState->suppressShapes = true;
				_sourceDisplayRunner->setNeedsRedraw(true);
			}
		}
		if (_overlayAnimFeature) {
			loadScrbOntoFeature(_overlayAnimFeature, _scrbTransitionResId, true);
		}
		_transitionPhase = TransitionPhase::kInitialPair03;
		break;
	default:
		break;
	}
	_answerState = AnswerState::kIdle02;
}

// =========================================================================
// Drag evaluation
// =========================================================================

int16 ZoombiniPuzzleSmoke::evaluateRunnerDrop(ZmbFeature *runner, const Common::Point &dropPos) {
	(void)runner;

	// Converted from blocking to event-driven.
	// Return the slot index for a valid drop rectangle, or -1 otherwise.

	if (kPuzzleLevel3 <= _difficultyLevel) {
		if (_leftFilterCount < 3) {
			for (int16 dropRectIdx = 0; dropRectIdx < 3; dropRectIdx++) {
				int16 rectIdx = 3 * _leftFilterCount + dropRectIdx;
				if (_dragRectsA[rectIdx].contains(dropPos))
					return dropRectIdx;
			}
		}
		if (_rightFilterCount < 3) {
			for (int16 dropRectIdx = 0; dropRectIdx < 3; dropRectIdx++) {
				int16 rectIdx = 3 * _rightFilterCount + dropRectIdx;
				if (_dragRectsB[rectIdx].contains(dropPos))
					return dropRectIdx + 3;
			}
		}
		return -1;
	}

	// L1-2
	if (_cliffDropRect.contains(dropPos) && _comparisonInputEnabled)
		return 4;

	return -1;
}

// =========================================================================
// L4 frame transition handler
// =========================================================================

void ZoombiniPuzzleSmoke::handleFrameTransition(int16 eventCode) {
	switch (eventCode) {
	case kL4MirrorChainEventCode017_AdvanceRunner:
		// The level-4 overlay advanced the source runner to the next transition position.
		if (_sourceDisplayRunner) {
			ZmbSmokeRunnerState *state = findRunnerState(_sourceDisplayRunner);
			if (state) {
				_transitionPhase = TransitionPhase::kSecondPair02;
				state->orientation = ZmbSmokeRunnerState::Orientation::kBaseTwo04;
				setSmokeRunnerPoint(_sourceDisplayRunner, kLevel4TransitionPositions[static_cast<int16>(_transitionPhase)], ZmbSmokeRunnerState::DirtyMode::kRecompose04);
			}
		}
		break;
	case kL4MirrorChainEventCode018_RefreshAnswer:
		// The level-4 overlay reached the answer-refresh marker.
		// Rebuild the displayed traits and reset the compare runners for the next phase.
		if (_sourceDisplayRunner) {
			_sourceDisplayRunner->resetNextRenderFrame();
			ZmbSmokeRunnerState *state = findRunnerState(_sourceDisplayRunner);
			if (state) {
				_transitionPhase = TransitionPhase::kAnswerReady01;
				state->orientation = ZmbSmokeRunnerState::Orientation::kBaseOne05;
				setSmokeRunnerPoint(_sourceDisplayRunner, kLevel4TransitionPositions[static_cast<int16>(_transitionPhase)], ZmbSmokeRunnerState::DirtyMode::kRecompose04);
			}
			cacheZmbTraits(7, _sourceDisplayRunner);
		}
		loadZmbTraitsToCache();
		cycleZmbTraitDisplay();
		cacheAnswerRunnerTraits();
		advanceAnswerRunnerFrames();
		initMatchCompareRunners();
		break;
	case kL4MirrorChainEventCode019_ReloadLevel:
		// The level-4 transition completed its final marker.
		// Reinitialize all runners and request the main overlay SCRB on the next frame.
		initLevel3RunnersAllTraits();
		_bReloadOverlayAndAnswer = true;
		break;
	default:
		break;
	}
}

// =========================================================================
// Animation dispatch
// =========================================================================

void ZoombiniPuzzleSmoke::onFeatureAnimEvent(ZmbFeature *feature, int16 eventCode) {
	// Every active level-4 overlay SCRB uses this dispatch.
	// Its first event changes phase 3 to phase 2.
	if (feature == _overlayAnimFeature && _difficultyLevel == kPuzzleLevel4) {
		handleFrameTransition(eventCode);
		return;
	}

	// All other events go to the central dispatch
	processAnimDispatchEvent(feature, eventCode);
}

void ZoombiniPuzzleSmoke::processAnimDispatchEvent(ZmbFeature *feature, int16 eventCode) {

	if (kMirrorLaneEventCode030_PickupZoombini < eventCode) {
		if (kMirrorLaneEventCode038_FinishLevel4Compare < eventCode) {
			switch (eventCode) {
			case kMirrorLaneEventCode050_PlaceZoombini:
				// The placement runner reached its hand-off marker; expose the dragged Zoombini.
				_bPlaceActiveZmb = true;
				break;
			case kMirrorLaneEventCode051_LinkRunners:
				// The placement runner reached the link marker; connect the active runner chain.
				_bRelinkRejectionRunners = true;
				break;
			case kMirrorLaneEventCode060_FinishDeparture:
				// The first departure column completes the shared cart/Snoid timing group.
				_departureState = ZmbDepartureState::kCompleted;
				break;
			case kMirrorLaneEventCode251_SetBodyArrangementOne:
				// Apply the direct body arrangement marker emitted by the level-4 Snoid SCRB.
				if (feature && feature->hasFlag(ZmbFeature::FLAG_00000001_TYPE_SNOID))
					static_cast<ZmbSnoid *>(feature)->setTraitLayout(ZmbScriptDecoder::TraitLayout::kSlotOrder01);
				break;
			default:
				break;
			}
		} else {
			switch (eventCode) {
			case kMirrorLaneEventCode031_ReloadSmokeStack:
				// The smoke-stack runner finished; reload the alternate stack and restore its links.
				if (_smokeStackBFeature) {
					manualLinkAfter(_smokeStackBFeature, _smokeStackAFeature);
					_smokeStackBFeature->addFlag(static_cast<ZmbFeature::Flag>(ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_00008000_LOOP_ANIM));
					_smokeStackBFeature->removeFlag(static_cast<ZmbFeature::Flag>(ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_01000000_DEFER_RENDER));
					loadScrbOntoFeature(_smokeStackBFeature, _scrbSmokeStackResB, true);
				}
				break;
			case kMirrorLaneEventCode035_LoadNextZoombini:
				// The cart reached the next-Zoombini marker; load and link the next queued runner.
				if (_currentQueueIdx + 1 < _pageLoadedZmbCount && _zmbQueue[_currentQueueIdx + 1]) {
					ZmbSnoid *nextZmb = getSnoid(_zmbQueue[_currentQueueIdx + 1]);
					if (nextZmb) {
						nextZmb->deactivateRender();
						nextZmb->setPointLoc(kHidePosition);
						playZmbScript(ZmbScrsCompletionMode::kReturnToIdle, feature, _scrbDropResId, nextZmb);
						manualLinkAfter(nextZmb, feature);
					}
				}
				break;
			case kMirrorLaneEventCode036_AssignFirstTraits:
				// The first placement marker assigns the dragged Zoombini's traits to the display table.
				if (_bFirstTraitAssign) {
					_bFirstTraitAssign = false;
					assignZmbTraitsFromSrc(0, _activeZmb);
					cacheZmbTraits(0, _activeZmb);
				}
				break;
			case kMirrorLaneEventCode037_StartZoombiniWalk:
				// The walking marker starts the current Zoombini SCRS and toggles the smoke stack.
				if (_currentQueueIdx < _pageLoadedZmbCount && _zmbQueue[_currentQueueIdx]) {
					ZmbSnoid *zmb = getSnoid(_zmbQueue[_currentQueueIdx]);
					_activeZmb = zmb;
					if (zmb) {
						playZmbScript(ZmbScrsCompletionMode::kReturnToIdle, feature, _scrbWalkResId, zmb);
						manualLinkAfter(zmb, feature);
					}
				}

				{
					ZmbFeature *toggleStack = _bRunnerToggle ? _smokeStackAFeature : _smokeStackBFeature;
					if (toggleStack) {
						toggleStack->addFlag(static_cast<ZmbFeature::Flag>(ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_00008000_LOOP_ANIM));
						toggleStack->removeFlag(static_cast<ZmbFeature::Flag>(ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_01000000_DEFER_RENDER));
						loadScrbOntoFeature(toggleStack, _scrbSmokeStackResB, true);

						// The other cart runner owns continuity for this render pass.
						// Terminate this runner's current hotspot payload after the reload;
						// its next timing gate materializes the new SCRB frame zero.
						toggleStack->setPreparedVisualRects(Common::Array<Common::Rect>());
						toggleStack->setPreparedRenderHotspots(Common::Array<ZmbPreparedRenderHotspot>());
					}
				}
				break;
			case kMirrorLaneEventCode038_FinishLevel4Compare:
				// The level-4 compare marker either advances the transition or starts the answer result.
				if (_difficultyLevel == kPuzzleLevel4) {
					if (_transitionPhase == TransitionPhase::kInitialPair03) {
						if (_activeZmb) {
							assignZmbTraitsFromSrc(0, _activeZmb);
							cacheZmbTraits(0, _activeZmb);
							loadZmbTraitsToCache();
							cycleZmbTraitDisplay();
							clearReflectionRunners();

							if (_targetDisplayRunner) {
								ZmbSmokeRunnerState *targetState = findRunnerState(_targetDisplayRunner);
								if (targetState) {
									targetState->suppressShapes = true;
									_targetDisplayRunner->setNeedsRedraw(true);
								}
							}

							if (_currentQueueIdx < _pageLoadedZmbCount && _zmbQueue[_currentQueueIdx]) {
								if (_overlayAnimFeature) {
									loadScrbOntoFeature(_overlayAnimFeature, _scrbOverlayResId, true);
								}
							}
						}
					} else if (_transitionPhase == TransitionPhase::kAnswerReady01 && _questionZmbCount &&
							   _placedZmbCount <= _pageLoadedZmbCount) {
						_answerState = AnswerState::kQuestion01;
						loadScrbOnAnswerRunner(kResScrb11005_AnswerAlternate);
					}
				}
				break;
			default:
				break;
			}
		}
		return;
	} else if (eventCode == kMirrorLaneEventCode030_PickupZoombini) {
		// The pickup marker hides the current Snoid and starts its drag-to-cart SCRS.
		if (_currentQueueIdx < _pageLoadedZmbCount && _zmbQueue[_currentQueueIdx]) {
			ZmbSnoid *zmb = getSnoid(_zmbQueue[_currentQueueIdx]);
			if (zmb) {
				zmb->deactivateRender();
				_activeZmb = zmb;
				zmb->setPointLoc(kHidePosition);
				playZmbScript(ZmbScrsCompletionMode::kReturnToIdle, feature, _scrbPickupResId, zmb);
				manualLinkAfter(zmb, feature);
			}
		}
		return;
	}

	switch (eventCode) {
	case kMirrorMatchEventCode000_ToggleSnoidFacing:
		// Toggle the plain Snoid's facing at the SCRS marker.
		// Toggle facing, not render activation.
		// The exact type-flag check limits the flip to plain Snoid runners; a dragged Snoid has
		// TOPMOST|OVERLAY added and is skipped.
		// Toggling render here instead
		// deadlocks SCRS playback (hidden snoids skip the anim state machine).
		if (feature && feature->getFlags() == ZmbFeature::FLAG_00000001_TYPE_SNOID) {
			ZmbSnoid *evSnoid = static_cast<ZmbSnoid *>(feature);
			evSnoid->setFacingLeft(!evSnoid->isFacingLeft());
		}
		break;
	case kMirrorMatchEventCode001_ReloadMainRunner:
		// The main runner requested its next SCRB resource.
		_bReloadMainRunner = true;
		break;
	case kMirrorMatchEventCode002_StartMatchCompare:
		// Begin the comparison pass after both display sides are ready.
		initMatchCompareRunners();
		break;
	case kMirrorMatchEventCode003_ResetCurrentLevel:
		// Reset the current low-level state when the comparison animation rejects a pair.
		if (kPuzzleLevel1 <= _difficultyLevel && _difficultyLevel < kPuzzleLevel4)
			_bResetLevel = true;
		break;
	case kMirrorMatchEventCode004_StartNextCompare:
		// Advance to the next comparison sequence.
		startNextCompareSequence();
		break;
	case kMirrorMatchEventCode010_PlayCompareReject:
	case kMirrorMatchEventCode011_PlayCompareRejectAlternate:
	case kMirrorMatchEventCode013_PlayCompareRejectThird:
	case kMirrorMatchEventCode014_PlayCompareRejectFourth:
		// These markers select the feet-specific rejection SCRS for the active Snoid.
		if (_activeZmb) {
			byte orient = _activeZmb->_trait._feet;
			int16 scrsId = static_cast<int16>(_scrbZmbAnimIdArr[_bCompareSwapped ? 1 : 0] + orient);
			playZmbScript(ZmbScrsCompletionMode::kHide, feature, scrsId, _activeZmb);
		}
		break;
	case kMirrorMatchEventCode016_AdvanceDisplayPair: {
		// Move the two central reflection runners to the next pair of display positions.
		if (_reflectionRunners[0]) {
			ZmbSmokeRunnerState *state0 = findRunnerState(_reflectionRunners[0]);
			if (state0) {
				_reflectionRunners[0]->resetNextRenderFrame();
				_reflectionRunners[0]->setNeedsRedraw(true);
				state0->nextRenderFrame = 0;
				state0->dirtyMode = ZmbSmokeRunnerState::DirtyMode::kRecompose04;
			}
		}
		if (_reflectionRunners[1]) {
			ZmbSmokeRunnerState *state1 = findRunnerState(_reflectionRunners[1]);
			if (state1) {
				_reflectionRunners[1]->resetNextRenderFrame();
				_reflectionRunners[1]->setNeedsRedraw(true);
				state1->nextRenderFrame = 0;
				state1->dirtyMode = ZmbSmokeRunnerState::DirtyMode::kRecompose04;
			}
		}
		const uint32 positionCount = _bCompareSwapped ? ARRAYSIZE(kDisplayPairSwappedA) : ARRAYSIZE(kDisplayPairNormalA);
		if (static_cast<uint32>(_displayPairPositionIdx) < positionCount) {
			Common::Point posA, posB;
			if (_bCompareSwapped) {
				posA = kDisplayPairSwappedA[_displayPairPositionIdx];
				posB = kDisplayPairSwappedB[_displayPairPositionIdx];
			} else {
				posA = kDisplayPairNormalA[_displayPairPositionIdx];
				posB = kDisplayPairNormalB[_displayPairPositionIdx];
			}

			// Restore the runner's previous drawn region before moving the reflected image;
			// otherwise the rising reject image leaves a lower stale copy behind.
			setSmokeRunnerPoint(_reflectionRunners[0], posA, ZmbSmokeRunnerState::DirtyMode::kRecompose04);
			setSmokeRunnerPoint(_reflectionRunners[1], posB, ZmbSmokeRunnerState::DirtyMode::kRecompose04);

			_displayPairPositionIdx += 1;
		}
		break;
	}
	case kMirrorMatchEventCode017_RejectCurrentZoombini:
		// Remove the rejected Zoombini from the active queue and play its rejection animation.
		if (_activeZmb) {
			for (int16 i = 0; i < _pageLoadedZmbCount; i++) {
				if (_zmbQueue[i] && getSnoid(_zmbQueue[i]) == _activeZmb && _difficultyLevel != kPuzzleLevel4) {
					_zmbQueue[i] = 0;
					break;
				}
			}
			if (_compareResult != CompareResult::kMatch00)
				_activeZmb = nullptr;
		}
		playRejectedAnimation();
		break;
	default:
		break;
	}
}

// =========================================================================
// Per-frame update
// =========================================================================

void ZoombiniPuzzleSmoke::onEveryFrame() {
	if (_isUpdating || !_pageActive)
		return;
	PuzzleUpdateGuard updateGuard(_isUpdating);
	updateLevel12WalkInArrivalPose();
	tickSmokeRunners();

	// Drive the Go button from @ref ZoombiniPuzzleSmoke::_goButtonEnabled.
	// It remains disabled until the first Zoombini reaches the cliff.
	setGoButtonsEnabled(_goButtonEnabled);
}

void ZoombiniPuzzleSmoke::onPostRenderFrame() {
	if (_isUpdating || !_pageActive)
		return;
	PuzzleUpdateGuard updateGuard(_isUpdating);

	// --- Event flag handlers ---

	// Reload main runner
	if (_bReloadMainRunner) {
		_bReloadMainRunner = false;
		if (_mainAnimFeature) {
			_mainAnimFeature->addFlag(static_cast<ZmbFeature::Flag>(ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00008000_LOOP_ANIM |
																	ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE));
			_mainAnimFeature->removeFlag(ZmbFeature::FLAG_01000000_DEFER_RENDER);
			loadScrbOntoFeature(_mainAnimFeature, kResScrb11017_MainResult, true);
		}
	}

	// Restore the rejection stack ordering after SCRS event 51 asks for a relink.
	if (_bRelinkRejectionRunners) {
		_bRelinkRejectionRunners = false;
		manualLinkBefore(_rejectionFeature, _holdingAreaFeature);
		manualLinkAfter(_activeZmb, _rejectionFeature);
	}

	// Reload the overlay and ready answer display.
	if (_bReloadOverlayAndAnswer) {
		_bReloadOverlayAndAnswer = false;
		if (_overlayAnimFeature) {
			loadScrbOntoFeature(_overlayAnimFeature, _scrbOverlayResId, false);
			_bShowAnswer = true;
			_answerState = AnswerState::kIdle02;
			loadScrbOnAnswerRunner(kResScrb11003_Answer);
		}
	}

	// Reset level
	if (_bResetLevel) {
		_bResetLevel = false;
		if (_difficultyLevel <= kPuzzleLevel2) {
			_bShowAnswer = false;
			loadScrbOnWellRunner(kResScrb11002_Well);
		} else if (_difficultyLevel == kPuzzleLevel3) {
			_bShowAnswer = true;
			_answerState = AnswerState::kIdle02;
			loadScrbOnAnswerRunner(kResScrb11003_Answer);
		}
	}

	// Place zoombini
	if (_bPlaceActiveZmb) {
		_bPlaceActiveZmb = false;

		int16 columnScrbId = static_cast<int16>(kResScrb11071_ColumnFirst - _placedZmbCount);
		ZmbFeature *columnRunner = loadScrbFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape11000_Smoke), columnScrbId, 6,
			ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_04000000_OVERLAY);
		if (_placedZmbCount < 20)
			_departureColumnRunners[_placedZmbCount] = columnRunner;

		if (_compareResult == CompareResult::kMatch00 && _activeZmb) {
			if (_placedZmbCount < 20)
				_acceptedZmbs[_placedZmbCount] = _activeZmb;

			_activeZmb->addFlag(static_cast<ZmbFeature::Flag>(ZmbFeature::FLAG_00000001_TYPE_SNOID | ZmbFeature::FLAG_04000000_OVERLAY));
			// Committing a Snoid to the cliff makes it part of the continuing pack.
			_activeZmb->_packIsOccupied = true;

			if (2 < _loadedOnCliffCount) {
				// The 1-based feet trait selects SCRS 12045-12049.
				const int16 feetTrait = CLIP<int16>(static_cast<int16>(_activeZmb->_trait._feet), 1, 5);
				startSnoidScrs(_activeZmb, ZmbResource(ZmbResource::kPage, kResScrs12045_FeetIdleBase + feetTrait - 1));
			}

			_loadedOnCliffCount += 1;
			if (_loadedOnCliffCount == 1) {
				_goButtonEnabled = true;
			}
		}

		if (_placedZmbCount == 0) {
			manualLinkBefore(columnRunner, _holdingAreaFeature);
		} else {
			manualLinkBefore(columnRunner, _departureColumnRunners[_placedZmbCount - 1]);
		}
		manualLinkAfter(_activeZmb, columnRunner);

		_placedZmbCount += 1;
		if (_placedZmbCount == _pageLoadedZmbCount) {
			// Count the materialized pack members that remain active and occupied.
			int16 loadedCount = 0;
			for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
				ZmbSnoid *snoid = *it;
				if (snoid->isPackSnoid() && snoid->isRenderActivated() && snoid->_packIsOccupied)
					loadedCount += 1;
			}

			if (loadedCount == _pageLoadedZmbCount) {
				_bCelebrationIdleActive = true;
				queueCompletionNarratorSound();
			} else if (loadedCount < _pageLoadedZmbCount && passesPartialResultFeedbackGate()) {
				queuePartialSuccessNarratorSound();
			}
		}

		_activeZmb = nullptr;

		// Reload smoke stack
		if (_difficultyLevel <= kPuzzleLevel2) {
			if (_smokeStackAFeature) {
				_smokeStackAFeature->addFlag(static_cast<ZmbFeature::Flag>(ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE));
				_smokeStackAFeature->removeFlag(static_cast<ZmbFeature::Flag>(ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_01000000_DEFER_RENDER));
				loadScrbOntoFeature(_smokeStackAFeature, kResScrb11032_StackLevel1, true);
			}
		} else {
			_currentQueueIdx += 1;
			ZmbFeature *toggleStack = _bRunnerToggle ? _smokeStackBFeature : _smokeStackAFeature;
			if (toggleStack)
				loadScrbOntoFeature(toggleStack, _scrbTravelResId, true);
		}

		// Update match readiness
		if (_difficultyLevel <= kPuzzleLevel2)
			_matchRunnerIndex = 0;

		if (_difficultyLevel < kPuzzleLevel4 ||
			(_difficultyLevel == kPuzzleLevel4 && _transitionPhase == TransitionPhase::kAnswerReady01)) {
			if (kPuzzleLevel3 <= _difficultyLevel)
				_questionZmbCount = copyPairToCompareBuffer();
			else
				selectQuestionZmb();

			if (_questionZmbCount && _placedZmbCount <= _pageLoadedZmbCount && _difficultyLevel < kPuzzleLevel4) {
				_answerState = AnswerState::kQuestion01;
				loadScrbOnAnswerRunner(kResScrb11005_AnswerAlternate);
			} else {
				_answerState = AnswerState::kHidden00;
				loadScrbOnWellRunner(kResScrb11004_WellAlternate);
			}
		}

		// Event 50 is the round-complete handoff.
		// Re-enable comparison input here so the state-1 lever can prepare the next L3/L4 question.
		_comparisonInputEnabled = true;
	}

	// Start the accepted-cart departure sequence.
	if (_bStartDeparture) {
		_bStartDeparture = false;
		Common::Array<ZmbFeature *> departureGroup;
		for (int16 i = 0; i < 3 && i < _placedZmbCount; i++) {
			ZmbFeature *colRunner = _departureColumnRunners[i];
			if (!colRunner)
				continue;

			loadScrbOntoFeature(colRunner, i + kResScrb11072_DepartureBase, true);
			departureGroup.push_back(colRunner);

			ZmbSnoid *cliffZmb = _acceptedZmbs[i];
			if (cliffZmb) {
				playZmbScript(ZmbScrsCompletionMode::kReturnToIdle, nullptr, i + kResScrs12041_DepartureRiderBase, cliffZmb);
				departureGroup.push_back(cliffZmb);
			}
		}

		// Assign the first column a timing slot.
		// Copy that slot to all three columns and their Snoids.
		if (1 < departureGroup.size()) {
			registerFeatureTimingGroup(departureGroup[0], departureGroup[1]);
			for (uint32 i = 2; i < departureGroup.size(); i++)
				joinFeatureTimingGroup(departureGroup[0], departureGroup[i]);
		}
	}

	// Run the finite completion-celebration idle sequence without replacing existing timing-group membership.
	if (_bCelebrationIdleActive && _celebrationIdleProgress < _pageLoadedZmbCount - 1) {
		if (30 < getCurrentFrameCounter() - _lastCelebrationIdleFrame) {
			_lastCelebrationIdleFrame = getCurrentFrameCounter();
			for (int16 idleAttemptIdx = 0; idleAttemptIdx < _pageLoadedZmbCount; idleAttemptIdx++) {
				uint32 idleMask = _celebrationIdlePoolState;
				uint16 randIdx = _vm->_rnd->getNonRepeatRandom(_pageLoadedZmbCount - 1, idleMask);
				_celebrationIdlePoolState = idleMask;

				ZmbSnoid *idleZmb = _zmbQueue[randIdx] ? getSnoid(_zmbQueue[randIdx]) : nullptr;
				// The first three accepted cliff Zoombinis may still belong to the cart display.
				// Starting an idle script on any of them would break that visual.
				const bool excluded = !idleZmb || _acceptedZmbs[0] == idleZmb ||
									  _acceptedZmbs[1] == idleZmb || _acceptedZmbs[2] == idleZmb;
				if (excluded) {
					// Small packs use queue slot 3 as the authored fallback and finish
					// the celebration attempt even when that slot is empty or busy.
					if (_pageLoadedZmbCount < 5) {
						ZmbSnoid *fallbackZmb = _zmbQueue[3] ? getSnoid(_zmbQueue[3]) : nullptr;
						if (fallbackZmb && fallbackZmb->isRenderActivated() &&
							fallbackZmb->getAnimState() == kSnoidAnimState000_Idle) {
							const int16 feetTrait = CLIP<int16>(static_cast<int16>(fallbackZmb->_trait._feet), 1, 5);
							startSnoidScrs(fallbackZmb, ZmbResource(ZmbResource::kPage, kResScrs12045_FeetIdleBase + feetTrait - 1));
						}
						_celebrationIdleProgress = 4;
						break;
					}
					continue;
				}

				if (idleZmb->isRenderActivated() && idleZmb->getAnimState() == kSnoidAnimState000_Idle) {
					// The 1-based feet trait selects idle SCRS 12045-12049.
					const int16 feetTrait = CLIP<int16>(static_cast<int16>(idleZmb->_trait._feet), 1, 5);
					startSnoidScrs(idleZmb, ZmbResource(ZmbResource::kPage, kResScrs12045_FeetIdleBase + feetTrait - 1));
					_celebrationIdleProgress = (0 < _celebrationIdleProgress) ? _celebrationIdleProgress + 1 : 4;
					break;
				}
			}
		}
	} else if (_pageLoadedZmbCount - 1 <= _celebrationIdleProgress) {
		_celebrationIdlePoolState = 0;
		_lastCelebrationIdleFrame = 0;
		_bCelebrationIdleActive = false;
		_celebrationIdleProgress = 0;
	}
}

// =========================================================================
// Click / Drag handlers
// =========================================================================

void ZoombiniPuzzleSmoke::setRunnerDragActive(bool active) {
	if (_bRunnerDragActive == active)
		return;

	_bRunnerDragActive = active;
	if (active)
		_vm->_cursor->hideCursor();
	else
		_vm->_cursor->showCursor();
}

ZmbEventHandleResult ZoombiniPuzzleSmoke::onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) {
	// Let base class handle Go/Map/Help
	ZmbEventHandleResult result = ZoombiniInteractive::onLButtonDown(absPos, relPos);
	if (result == ZmbEventHandleResult::kConsumed)
		return result;

	if (_pageLoadedZmbCount <= _placedZmbCount || !_comparisonInputEnabled || !_pageActive)
		return ZmbEventHandleResult::kPassthrough;

	// A prepared question starts a newly generated comparison.
	// An idle answer submits a complete answer and clears the generic DRAW_ON_REG slot before the comparison sequence.
	if (_answerState == AnswerState::kQuestion01 && _answerZoneFeature &&
		_leverClickRect.contains(absPos)) {
		resetAndReinitLevel();
		_answerState = AnswerState::kIdle02;
		loadScrbOnDispatchRunner(kResScrb11004_WellAlternate, true);
		// Enable automatic runner sorting for the next question.
		setRunnerZSortEnabled(true);
		return ZmbEventHandleResult::kConsumed;
	}

	if (AnswerState::kHidden00 != _answerState && _bShowAnswer && _leverClickRect.contains(absPos)) {
		_bShowAnswer = false;
		_answerState = AnswerState::kHidden00;
		loadScrbOnDispatchRunner(kResScrb11002_Well, true);
		if (0 < _drawOnRegCount)
			clearDrawOnRegOccupant(0);
		// Disable automatic Z-sorting.
		// Event 51 creates a manual cart-to-Snoid link.
		// Keep it stable throughout the travel sequence so the Snoid, including its feet, draws last.
		setRunnerZSortEnabled(false);
		_comparisonInputEnabled = false;
		return ZmbEventHandleResult::kConsumed;
	}

	// Real Zoombinis are manually draggable only in L1-2.
	ZmbSnoid *clickedSnoid = (_difficultyLevel <= kPuzzleLevel2) ? findSnoidAtPoint(absPos) : nullptr;
	if (clickedSnoid) {
		bool onCliff = false;
		for (int16 i = 0; i < _loadedOnCliffCount; i++) {
			if (_acceptedZmbs[i] == clickedSnoid) {
				onCliff = true;
				break;
			}
		}

		if (!onCliff && clickedSnoid->isRenderActivated()) {
			const bool wasCurrent = clickedSnoid == _activeZmb;
			Common::Point returnPos = clickedSnoid->getPointLoc();
			if (wasCurrent) {
				returnPos = findRandomEmptySnoidSlotPosition(kSnoidPositions, ARRAYSIZE(kSnoidPositions), 500, false);
				_activeZmb = nullptr;
				clearZmbTraits(0);
				clearRunnerSlot(0);
				_bShowAnswer = false;
				loadScrbOnWellRunner(kResScrb11002_Well);
			}

			startSnoidDrag(clickedSnoid, absPos);
			if (wasCurrent)
				_dragOrigPos = returnPos;
			return ZmbEventHandleResult::kConsumed;
		}
	}

	// L3-4: grid runner drag
	if (kPuzzleLevel3 <= _difficultyLevel) {
		for (int16 runnerIdx = 1; runnerIdx < _gridRunnerCount && runnerIdx < 7; runnerIdx++) {
			ZmbFeature *gridRunner = _gridRunners[runnerIdx];
			if (!gridRunner || !gridRunner->findDrawRecordAtPoint(absPos))
				continue;
			_draggedRunner = gridRunner;
			const Common::Point dragStartPos = gridRunner->getPointLoc();
			// Use the same template offset as @ref ZoombiniPuzzleSmoke::initLevel3RunnersAllTraits().
			_dragRunnerOrigPos = kGridRunnerPositions[_filterHomePermutation[runnerIdx] - 1];
			_dragRunnerOffset = Common::Point(absPos.x - dragStartPos.x, absPos.y - dragStartPos.y);
			_dragRunnerLastMousePos = absPos;
			_dragRunnerSavedInterval = gridRunner->getFrameInterval();
			setRunnerDragActive(true);
			// Move the selected filter to the end of the global runner list.
			// This keeps it above the wooden platform and comparison beam after placement.
			manualLinkAtEnd(gridRunner);
			gridRunner->setFrameInterval(3);

			loadScrbOnDispatchRunner(kResScrb11002_Well, false);
			_bShowAnswer = false;

			ZmbSmokeRunnerState *state = findRunnerState(gridRunner);
			if (state) {
				state->orientation = ZmbSmokeRunnerState::Orientation::kBaseTwoFramedGrid07;
				state->dirtyMode = ZmbSmokeRunnerState::DirtyMode::kRecompose04;
				if (state->cycleTrait != ZmbSmokeRunnerState::CycleTrait::kNone00) {
					state->cyclingValue = 1;
					state->blinkValue = 0;
				}
			}

			for (int16 displayRunnerIdx = 0; displayRunnerIdx < 6; displayRunnerIdx++) {
				if (_filterSlotRunners[displayRunnerIdx] == gridRunner) {
					const int16 sideStart = (displayRunnerIdx < 3) ? 0 : 3;
					int16 &sideCount = (displayRunnerIdx < 3) ? _leftFilterCount : _rightFilterCount;
					for (int16 shiftSlotIdx = displayRunnerIdx; shiftSlotIdx < sideStart + 2; shiftSlotIdx++)
						_filterSlotRunners[shiftSlotIdx] = _filterSlotRunners[shiftSlotIdx + 1];
					_filterSlotRunners[sideStart + 2] = nullptr;
					if (0 < sideCount)
						sideCount -= 1;

					if (0 < sideCount) {
						const Common::Point *positions = (displayRunnerIdx < 3) ? kFilterPositionsA : kFilterPositionsB;
						for (int16 slotIdx = 0; slotIdx < sideCount; slotIdx++) {
							ZmbFeature *placedRunner = _filterSlotRunners[sideStart + slotIdx];
							if (placedRunner)
								placedRunner->setPointLoc(positions[3 * (sideCount - 1) + slotIdx]);
						}
					}

					if (displayRunnerIdx < 3) {
						loadZmbTraitsToCache();
						cycleZmbTraitDisplay();
					} else {
						cacheAnswerRunnerTraits();
						advanceAnswerRunnerFrames();
					}
					break;
				}
			}

			loadScoreScrbs();

			return ZmbEventHandleResult::kConsumed;
		}
	}

	// L1-2: cliff runner drag
	if (_difficultyLevel <= kPuzzleLevel2) {
		// The right display contains a copy; re-drag the hidden cliff runner while preserving its 1-based source index.
		if (_matchRunnerIndex && _cliffDropRect.contains(absPos)) {
			const int16 sourceIdx = _matchRunnerIndex - 1;
			if (0 <= sourceIdx && sourceIdx < _cliffRunnerCount) {
				_draggedRunner = _cliffRunners[sourceIdx];
				_dragRunnerOrigPos = _draggedRunner->getPointLoc();
				_dragRunnerOffset = Common::Point(absPos.x - _dragRunnerOrigPos.x, absPos.y - _dragRunnerOrigPos.y);
				_dragRunnerLastMousePos = absPos;
				_dragRunnerSavedInterval = _draggedRunner->getFrameInterval();
				_dragRunnerMatchIdx = _matchRunnerIndex;
				setRunnerDragActive(true);
				_draggedRunner->activateRender();
				// Append every selected custom runner to the list tail, including level 1-2 candidate mirrors.
				manualLinkAtEnd(_draggedRunner);
				_draggedRunner->setFrameInterval(3);
			}

			_bShowAnswer = false;
			loadScrbOnWellRunner(kResScrb11002_Well);
			clearRunnerSlot(7);
			clearZmbTraits(1);
			_matchRunnerIndex = 0;
			return ZmbEventHandleResult::kConsumed;
		}

		for (int16 cliffRunnerIdx = 0; cliffRunnerIdx < _cliffRunnerCount; cliffRunnerIdx++) {
			ZmbFeature *cliffRunner = _cliffRunners[cliffRunnerIdx];
			if (!cliffRunner || !cliffRunner->getClickRect().contains(absPos))
				continue;

			_draggedRunner = cliffRunner;
			_dragRunnerOrigPos = cliffRunner->getPointLoc();
			_dragRunnerOffset = Common::Point(absPos.x - _dragRunnerOrigPos.x, absPos.y - _dragRunnerOrigPos.y);
			_dragRunnerLastMousePos = absPos;
			_dragRunnerSavedInterval = cliffRunner->getFrameInterval();
			_dragRunnerMatchIdx = 0;
			setRunnerDragActive(true);
			manualLinkAtEnd(cliffRunner);
			cliffRunner->setFrameInterval(3);

			if (!_matchRunnerIndex)
				loadTimerScrb();

			return ZmbEventHandleResult::kConsumed;
		}
	}

	return ZmbEventHandleResult::kPassthrough;
}

void ZoombiniPuzzleSmoke::endDrag(const Common::Point &dropPos) {
	(void)dropPos;
	ZmbSnoid *snoid = finishSnoidDrag();
	if (!snoid)
		return;

	const Common::Point releasePos = snoid->getPointLoc();
	const int16 cartSlot = hitTestDrawOnRegSlot(releasePos, _clickZoneRadius, true);
	if (0 <= cartSlot && _cartDropRect.contains(releasePos) && !_activeZmb) {
		setDrawOnRegOccupant(cartSlot, snoid->getId());
		// DRAW_ON_REG point and enters state 4. That state immediately replaces
		// the lifted drag pose with the seated/common-image cart pose.
		snoid->setAnimTargetPos(kDrawOnRegPosition);
		snoid->setAnimState(kSnoidAnimState004_Arrive);
		_activeZmb = snoid;
		assignZmbTraitsFromSrc(0, snoid);
		cacheZmbTraits(0, snoid);
		if (_matchRunnerIndex) {
			_bShowAnswer = true;
			loadScrbOnAnswerRunner(kResScrb11003_Answer);
		}
	} else {
		// The original shared drag resolves tBMP walkability and collisions before
		// Smoke applies its page-specific pedestal restriction.
		settleSnoidAfterTerrainDrop(snoid, _dragOrigPos);
		const Common::Point terrainTarget = snoid->getAnimTargetPos();
		if (_pedestalDropRect.contains(releasePos)) {
			int16 bestY = kPedestalRowY[0];
			int16 bestDist = ABS(terrainTarget.y - bestY);
			for (int16 rowIdx = 1; rowIdx < 5; rowIdx++) {
				const int16 dist = ABS(terrainTarget.y - kPedestalRowY[rowIdx]);
				if (dist < bestDist) {
					bestDist = dist;
					bestY = kPedestalRowY[rowIdx];
				}
			}
			settleSnoidAtTarget(snoid, Common::Point(terrainTarget.x, bestY));
		} else {
			// Smoke deliberately rejects otherwise walkable terrain outside the
			// authored pedestal rectangle.
			settleSnoidAtTarget(snoid, _dragOrigPos);
		}
	}
}

bool ZoombiniPuzzleSmoke::hasStickyMouseDrag() const {
	return _bRunnerDragActive || ZoombiniInteractive::hasStickyMouseDrag();
}

void ZoombiniPuzzleSmoke::endStickyMouseDrag(const Common::Point &dropPos) {
	if (_bRunnerDragActive) {
		// Feed the release position to the event-driven custom-runner completion path before the click is consumed.
		_dragRunnerLastMousePos = dropPos;
		onLButtonUp(dropPos, Common::Point());
		return;
	}

	ZoombiniInteractive::endStickyMouseDrag(dropPos);
}

ZmbEventHandleResult ZoombiniPuzzleSmoke::onLButtonUp(const Common::Point &absPos, const Common::Point &relPos) {
	// Handle end of non-snoid runner drag
	if (_bRunnerDragActive && _draggedRunner) {
		setRunnerDragActive(false);
		_draggedRunner->setFrameInterval(_dragRunnerSavedInterval);
		const Common::Rect oldRect = _draggedRunner->getZSortRect();
		if (!oldRect.isEmpty())
			addExternalDirtyRect(oldRect);
		ZmbSmokeRunnerState *dragState = findRunnerState(_draggedRunner);
		if (dragState)
			dragState->dirtyMode = ZmbSmokeRunnerState::DirtyMode::kRecompose04;
		_draggedRunner->setNeedsRedraw(true);

		int16 dropSlot = evaluateRunnerDrop(_draggedRunner, _dragRunnerLastMousePos);
		if (kPuzzleLevel3 <= _difficultyLevel) {
			if (0 <= dropSlot && dropSlot < 6) {
				const int16 sideStart = (dropSlot < 3) ? 0 : 3;
				int16 &sideCount = (dropSlot < 3) ? _leftFilterCount : _rightFilterCount;
				const int16 sideSlot = dropSlot - sideStart;
				if (sideCount < 3 && sideSlot <= sideCount) {
					for (int16 slot = sideCount; sideSlot < slot; slot--)
						_filterSlotRunners[sideStart + slot] = _filterSlotRunners[sideStart + slot - 1];
					_filterSlotRunners[dropSlot] = _draggedRunner;
					sideCount += 1;

					ZmbSmokeRunnerState *state = findRunnerState(_draggedRunner);
					if (state) {
						if (dropSlot < 3)
							state->orientation = ZmbSmokeRunnerState::Orientation::kBaseOneFramed00;
						else
							state->orientation = ZmbSmokeRunnerState::Orientation::kBaseThreeFramed02;
						state->dirtyMode = ZmbSmokeRunnerState::DirtyMode::kRecompose04;
					}

					const Common::Point *positions = (dropSlot < 3) ? kFilterPositionsA : kFilterPositionsB;
					for (int16 slot = 0; slot < sideCount; slot++) {
						ZmbFeature *placedRunner = _filterSlotRunners[sideStart + slot];
						if (placedRunner)
							placedRunner->setPointLoc(positions[3 * (sideCount - 1) + slot]);
					}
				}

				if (dropSlot < 3) {
					loadZmbTraitsToCache();
					cycleZmbTraitDisplay();
				} else {
					cacheAnswerRunnerTraits();
					advanceAnswerRunnerFrames();
				}
			} else {
				_draggedRunner->setPointLoc(_dragRunnerOrigPos);
				ZmbSmokeRunnerState *state = findRunnerState(_draggedRunner);
				if (state)
					state->dirtyMode = ZmbSmokeRunnerState::DirtyMode::kRecompose04;
			}

			loadScoreDisplayScrbs();
			_bShowAnswer = true;
			loadScrbOnAnswerRunner(kResScrb11003_Answer);
		} else {
			// L1-2
			if (dropSlot == 4) {
				if (!_matchRunnerIndex) {
					int16 sourceIndex = _dragRunnerMatchIdx;
					if (!sourceIndex) {
						for (int16 cliffRunnerIdx = 0; cliffRunnerIdx < _cliffRunnerCount; cliffRunnerIdx++) {
							if (_cliffRunners[cliffRunnerIdx] == _draggedRunner) {
								sourceIndex = cliffRunnerIdx + 1;
								break;
							}
						}
					}

					_draggedRunner->setPointLoc(kCliffDropSnapPosition);
					ZmbSmokeRunnerState *state = findRunnerState(_draggedRunner);
					if (state)
						state->dirtyMode = ZmbSmokeRunnerState::DirtyMode::kRecompose04;
					_matchRunnerIndex = sourceIndex;
					assignZmbTraitsFromSrc(1, _draggedRunner);
					cacheZmbTraits(7, _draggedRunner);
				} else {
					_draggedRunner->setPointLoc(_dragRunnerOrigPos);
				}
			} else {
				if (_dragRunnerMatchIdx)
					_draggedRunner->setPointLoc(kCliffRunnerPositions[_dragRunnerMatchIdx - 1]);
				else
					_draggedRunner->setPointLoc(_dragRunnerOrigPos);
				ZmbSmokeRunnerState *state = findRunnerState(_draggedRunner);
				if (state)
					state->dirtyMode = ZmbSmokeRunnerState::DirtyMode::kRecompose04;
			}

			if (_matchRunnerIndex && _activeZmb) {
				_bShowAnswer = true;
				loadScrbOnAnswerRunner(kResScrb11003_Answer);
			}
			unloadTimerScrb();
		}

		_draggedRunner = nullptr;
		_dragRunnerMatchIdx = 0;
		return ZmbEventHandleResult::kConsumed;
	}

	if (isDragging())
		return ZoombiniInteractive::onLButtonUp(absPos, relPos);

	return ZmbEventHandleResult::kPassthrough;
}

ZmbEventHandleResult ZoombiniPuzzleSmoke::onMouseMove(const Common::Point &absPos, const Common::Point &relPos) {
	if (_bRunnerDragActive && _draggedRunner) {
		// Touch motion can carry a synthetic relative delta. The absolute point is
		// authoritative for the runner, as it is for the shared Snoid drag path.
		_dragRunnerLastMousePos = absPos;
		setSmokeRunnerPoint(_draggedRunner, Common::Point(absPos.x - _dragRunnerOffset.x, absPos.y - _dragRunnerOffset.y), ZmbSmokeRunnerState::DirtyMode::kDragging05);
		return ZmbEventHandleResult::kConsumed;
	}
	return ZoombiniInteractive::onMouseMove(absPos, relPos);
}
} // End of namespace Mohawk
