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

#include "mohawk/zoombini_pages/puzzle_ferry.h"

#include "common/config-manager.h"

#include "mohawk/zoombini.h"
#include "mohawk/zoombini_graphics.h"
#include "mohawk/zoombini_metaengine.h"
#include "mohawk/zoombini_random.h"
#include "mohawk/zoombini_resource.h"
#include "mohawk/zoombini_sound.h"
#include "mohawk/zoombini_state.h"

namespace Mohawk {

constexpr Common::Point ZoombiniPuzzleFerry::kDockSnoidPositions[20];
constexpr int16 ZoombiniPuzzleFerry::kCaptainIntroScrbPool[4];
constexpr int16 ZoombiniPuzzleFerry::kCaptainIdleFidgetScrbPool[5];
constexpr int16 ZoombiniPuzzleFerry::kCaptainGoodReactionScrbPool[2];
constexpr int16 ZoombiniPuzzleFerry::kCaptainBadReactionScrbPool[11];
constexpr int16 ZoombiniPuzzleFerry::kCaptainMoveReactionScrbPool[3];
constexpr int16 ZoombiniPuzzleFerry::kCaptainPostFlightFidgetScrbPool[5];
constexpr int16 ZoombiniPuzzleFerry::kRejectDestinationSeatScrbIds[10];

// Reject-flight lookup tables, keyed by transport branch (0..9):
//   0       = dock exit
//   1..6    = rejected-Snoid exit paths (SCRB 1604/1606)
//   7..9    = raft departure paths (SCRB 1607 plus 1705/1706)
//
// Primary Snoid SCRS used by case 1.
// It runs in the normal script state and hides the Snoid when its embedded flight animation completes.
static const int16 kRejectPrimarySnoidScrsIds[10] = {
	ZoombiniPuzzleFerry::kResScrs1902_RejectVariant2, ZoombiniPuzzleFerry::kResScrs1900_RejectVariant0,
	ZoombiniPuzzleFerry::kResScrs1900_RejectVariant0, ZoombiniPuzzleFerry::kResScrs1904_RejectVariant4,
	ZoombiniPuzzleFerry::kResScrs1904_RejectVariant4, ZoombiniPuzzleFerry::kResScrs1906_RejectVariant6,
	ZoombiniPuzzleFerry::kResScrs1906_RejectVariant6, ZoombiniPuzzleFerry::kResScrs1900_RejectVariant0,
	ZoombiniPuzzleFerry::kResScrs1904_RejectVariant4, ZoombiniPuzzleFerry::kResScrs1906_RejectVariant6};

// Secondary Snoid SCRS used by case 2 at SCRB 1605's later frame.
// These scripts return the Snoid to idle and align their end anchor to @ref ZoombiniPuzzleFerry::_rejectLandingPosition.
// Value 1907 in destination entries 7-9 selects the raft departure branch instead.
static const int16 kRejectSecondarySnoidScrsIds[10] = {
	ZoombiniPuzzleFerry::kResScrs1903_RejectVariant3, ZoombiniPuzzleFerry::kResScrs1901_RejectVariant1,
	ZoombiniPuzzleFerry::kResScrs1905_RejectVariant5, ZoombiniPuzzleFerry::kResScrs1901_RejectVariant1,
	ZoombiniPuzzleFerry::kResScrs1905_RejectVariant5, ZoombiniPuzzleFerry::kResScrs1901_RejectVariant1,
	ZoombiniPuzzleFerry::kResScrs1905_RejectVariant5, ZoombiniPuzzleFerry::kResScrs1907_RejectVariant7,
	ZoombiniPuzzleFerry::kResScrs1907_RejectVariant7, ZoombiniPuzzleFerry::kResScrs1907_RejectVariant7};

ZoombiniPuzzleFerry::ZoombiniPuzzleFerry(MohawkEngine_Zoombini *vm) : ZoombiniPuzzle(vm, ZoombiniPageType::kFerry, ZmbSrcPageKind::kFerry_07) {
}

ZoombiniPuzzleFerry::~ZoombiniPuzzleFerry() {
}

const ZoombiniPage::ScriptSoundPriorityRanges &ZoombiniPuzzleFerry::getScriptSoundPriorityRanges() const {
	// Ferry clears the inherited ranges, then re-registers SND 996-997 at priority 29 in its authored list.
	static const ZoombiniPage::ScriptSoundPriorityRanges kRanges = {
		{kResSoundRange1606_SystemRangeBase, kResSoundRange1607_SystemRangeLast},
		{kSysResSoundRange20000_Base, kSysResSoundRange29999_Last},
		{kResSoundRange1800_SystemRangeBase, kResSoundRange1899_SystemRangeLast},
		{kSysResSoundRange0996_DepartSFX, kSysResSoundRange0997_ArriveSFX},
		{kResSoundRange1704_PagePriorityBase, kResSoundRange1705_PagePriorityLast},
		{kResSoundRange0425_PageRangeBase, kResSoundRange0499_PageRangeLast},
		{kResSoundRange1600_PageRangeBase, kResSoundRange1699_PageRangeLast},
		{kResSoundRange1900_PageRangeBase, kResSoundRange1999_PageRangeLast},
		{kResSoundRange1700_PageRangeBase, kResSoundRange1799_PageRangeLast}};
	return kRanges;
}

Audio::Mixer::SoundType ZoombiniPuzzleFerry::getFeatureSoundType(const ZmbFeature *feature, ZmbResource sndRes) const {
	if (feature == _captainRaftFeature && sndRes._archiveKind == ZmbResource::kPage &&
		kResSound1800_CaptainSpeechBase <= sndRes._id && sndRes._id <= kResSound1833_CaptainSpeechLast)
		return Audio::Mixer::kSpeechSoundType;
	return ZoombiniPuzzle::getFeatureSoundType(feature, sndRes);
}

void ZoombiniPuzzleFerry::open() {
	openArchive(ZMB_MHK_FERRY);
}

void ZoombiniPuzzleFerry::setBackgroundMusic() {
	// Narrator sounds 20073 and 20074 are reserved for F1 replay.
	// SND 997 is a UI click emitted by walking scripts, so no sound starts on page load.
}

void ZoombiniPuzzleFerry::setBackgroundBitmap() {
	_vm->_gfx->setPalette(kResBackground1300);
	_vm->_gfx->drawBackground(kResBackground1300);
}

void ZoombiniPuzzleFerry::initStates() {
	_traitHighlightEnabled = ConfMan.getBool(MohawkMetaEngine_Zoombini::kOptionFerryHighlightTraitMatch);
	_vm->_state->getFerryRuntimeState()._visitCount += 1;

	_acceptedSnoidCount = 0;
	_snoidInputEnabled = true;
	_pendingCaptainScrbId = 0;
	_pendingRejectReactionScrbId = 0;
	_rejectSetupPending = false;
	_rejectRaftOverlayPending = false;
	_departureState = ZmbDepartureState::kIdle;
	_consecutiveAcceptedPlacements = 0;
	_consecutiveRejectedPlacements = 0;
	_goodReactionStreakTarget = 1;
	_firstGoodReactionQueued = false;
	_initialCaptainScriptRestarted = false;
	_matchedTraitMask = 0;
	_traitHighlightSnoidId = 0;
	_rejectedSnoidId = 0;
	_rejectFlightSnoid = nullptr;
	_rejectControllerActive = false;
	_mainRejectSnoidCallbackActive = false;
	_dropTargetSeatNumber = 0;
	_rejectedSeatFeature = nullptr;
	_pickupWasSeated = false;
}

void ZoombiniPuzzleFerry::loadFeatures() {
	// Load terrain barrier bitmap (tBMP 100)
	loadTerrainBitmap(kResBitmapTerrain100);

	// Preload shape images
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape1400_Snoid));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape1450_Adjacency));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape1500_Seats));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape1600_Water));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape1700_Navigation));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape1800_Raft));

	// Register each SCRB image group without creating hidden feature runners.
	// Runtime swaps resolve every group member here and load its script data on demand.
	registerScrbImageGroup(ZmbResource(ZmbResource::kPage, kResBitmapShape1500_Seats), kResScrb1500_SeatBase, 10);
	registerScrbImageGroup(ZmbResource(ZmbResource::kPage, kResBitmapShape1600_Water), kResScrb1600_WaterBase, 10);
	registerScrbImageGroup(ZmbResource(ZmbResource::kPage, kResBitmapShape1700_Navigation), kResScrb1700_NavigationBase, 7);
	registerScrbImageGroup(ZmbResource(ZmbResource::kPage, kResBitmapShape1800_Raft), kResScrb1800_RaftBase, 33);
	registerScrbImageGroup(ZmbResource(ZmbResource::kPage, kResBitmapShape1450_Adjacency), kResScrb1450_OverlayBase, 3);

	// Load reject pool: 8 reject scripts at SCRS 1900
	// Group 0 -> state 9 (NORMAL).
	registerScrsGroup(kResScrs1900_RejectBase, 8);

	// Load normal pool: 10 normal scripts at SCRS 1000
	// Group 1 -> state 8 (REJECT).
	registerScrsGroup(kResScrs1000_NormalBase, 10);

	// --- Puzzle-specific feature runners ---

	// Landscape overlay animation (SCRB 1601)
	_landscapeOverlayFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1600_Water), kResScrb1601_Landscape, 6,
											   ZmbFeature::FLAG_00004000_NO_DIRTY_MERGE | ZmbFeature::FLAG_00008000_LOOP_ANIM);

	// First visit uses 1803; subsequent visits random from pool.
	{
		ZoombiniGameState::FerryRuntimeState &runtimeState = _vm->_state->getFerryRuntimeState();
		int16 raftScrb;
		if (runtimeState._visitCount == 1) {
			raftScrb = kResScrb1803_RaftFirstVisit;
		} else {
			uint16 raftPoolIdx = _vm->_rnd->getNonRepeatRandom(4, runtimeState._raftPoolState);
			raftScrb = kCaptainIntroScrbPool[raftPoolIdx];
		}
		_captainRaftFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1800_Raft), raftScrb, 6,
											  ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE);
		// Captain feedback bypasses the Snoid movement sound queue.
		_captainRaftFeature->setScriptSoundPolicy(ZmbFeature::ScriptSoundPolicy::kImmediate);
	}

	// Raft approach runners -- only loaded when "more action" mode is active (lessAction=false)
	if (!_vm->_state->isLessActionEnabled()) {
		_raftApproachFeatureA = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1600_Water), kResScrb1602_RaftApproachA, 6,
												ZmbFeature::FLAG_00008000_LOOP_ANIM);

		_raftApproachFeatureB = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1600_Water), kResScrb1603_RaftApproachB, 6,
												ZmbFeature::FLAG_00008000_LOOP_ANIM);

		registerAlternatingFeatureTimingGroup(_raftApproachFeatureB, _raftApproachFeatureA);
	}

	// Deferred overlay for the reject-flight raft branch (SCRB 1704).
	_rejectRaftOverlayFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1700_Navigation), kResScrb1704_DepartureOverlay, 6,
												ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE |
													ZmbFeature::FLAG_01000000_DEFER_RENDER);

	// Load the anonymous SCRB 1600 shape runner.
	loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1600_Water), kResScrb1600_WaterBase, 6, ZmbFeature::FLAG_00000000_TYPE_SHAPES);

	// Register overlay SCRBs 1450-1452.
	for (int16 i = 0; i < 3; i++) {
		_seatPriorityOverlayFeatures[i] = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1450_Adjacency), kResScrb1450_OverlayBase + i, 0,
														  ZmbFeature::FLAG_04000000_OVERLAY);
	}

	// Preserve registration order for the required initial materialization pass.
	setRunnerZSortEnabled(false);

	// Load Zoombinis from active pack at 20 pedestal positions.
	loadZoombinisFromPack(kDockSnoidPositions, ARRAYSIZE(kDockSnoidPositions));

	// Select the layout from the materialized pack count.
	// Counts outside the five authored variants use the 16-Snoid layout rather than an endpoint clamp.
	{
		int16 zmbCount = _pageLoadedZmbCount;
		if (zmbCount < 16 || 20 < zmbCount)
			zmbCount = 16;
		const int16 scrbBase = static_cast<int16>(kResScrb1510_SeatingBase + ((_difficultyLevel - 1) * 5));
		_seatLayoutScrbId = scrbBase + (zmbCount - 16);
	}

	// Create the raft seat and decoration runners before layout.
	loadSeatLayout();

	// Lay out the Snoids with a 30-frame initial walk-in delay.
	layoutStaticAndWalkIn(0, false);
	// Materialize the seat rectangles in authored registration order, then build
	// the adjacency graph before any gameplay input can occur.
	renderFeatures();
	buildAdjacencyMatrix();
	_seatAdjacencyReady = true;
	assignStaggeredWalkDelays(30, 45);
	setRunnerZSortEnabled(true);

	// Set up Go/Map/Help buttons
	configureStandardPuzzleControlRects();
	loadStandardPuzzleControlFeatures(kResBitmapShape1400_Snoid);

	// The original queue submits system SND 997 at Ferry's authored priority 29.
	// This port retains a direct mixer handle because the captain SCRB restart waits for this exact stream to end after the walk-in.
	Audio::SoundHandle *introHandle = _vm->_sound->playSound(ZmbResource(ZmbResource::kSystem, kSysResSound0997_IntroMove), Audio::Mixer::kSFXSoundType);
	_introMoveSfxHandleValid = introHandle != nullptr;
	if (_introMoveSfxHandleValid)
		_introMoveSfxHandle = *introHandle;

	// Seat acceptance starts from an empty continuing set.
	schedulePackSnoids(false, false);

	// Start the idle threshold at this load boundary, after resource materialization.
	// At 60TPS this is an idle interval of roughly 90-180 seconds.
	_captainIdleFidgetFrameThreshold = _vm->_rnd->getRandomNumber(5400, 10800);
}

void ZoombiniPuzzleFerry::initHelpPrompt() {
	// Level 2 always uses the harder help prompt.
	// Later levels choose between the two prompts, while level 1 always uses SND 20073.
	int16 helpSoundId;
	if (_entrySfxGroupFlags == ZmbSfxGroupFlags::kHardFirstTrigger_02) {
		helpSoundId = kSysResSound20074_FerryHelpHard;
	} else if (kPuzzleLevel2 <= _difficultyLevel) {
		helpSoundId = _vm->_rnd->getRandomNumber(kSysResSound20073_FerryHelpEasy, kSysResSound20074_FerryHelpHard);
	} else {
		helpSoundId = kSysResSound20073_FerryHelpEasy;
	}
	_activeHelpSoundId = ZmbResource(ZmbResource::kSystem, helpSoundId);
}

void ZoombiniPuzzleFerry::onGoButtonActivated() {
	// Route 2: Ferry -> Lilly (via Xfer)
	if (_rejectSetupPending)
		_snoidInputEnabled = false;
	_departureState = ZmbDepartureState::kTriggered;
}

void ZoombiniPuzzleFerry::updateDepartureState() {
	// The raft controller advances the state in onEveryFrame() and onFeatureAnimEvent().
}

void ZoombiniPuzzleFerry::debugPrepareForDeparture() {
	if (isDragging())
		finishSnoidDrag();
	clearDrawOnRegHighlight();

	// Cancel page-local feedback so the normal Go controller can take ownership on the next frame.
	_snoidInputEnabled = true;
	_pendingCaptainScrbId = 0;
	_pendingRejectReactionScrbId = 0;
	_rejectSetupPending = false;
	_rejectRaftOverlayPending = false;
	_rejectControllerActive = false;
	_mainRejectSnoidCallbackActive = false;
	_rejectedSnoidId = 0;
	_rejectFlightSnoid = nullptr;
	_dropTargetSeatNumber = 0;
	_rejectedSeatFeature = nullptr;
	_activeCaptainScrbId = 0;
	_departureState = ZmbDepartureState::kIdle;

	if (_rejectRaftOverlayFeature) {
		_rejectRaftOverlayFeature->deactivateAnimate();
		_rejectRaftOverlayFeature->deactivateRender();
	}
	if (_rejectRaftTakeoffFeature) {
		_rejectRaftTakeoffFeature->deactivateAnimate();
		_rejectRaftTakeoffFeature->deactivateRender();
	}
	if (_rejectRaftLandingFeature) {
		_rejectRaftLandingFeature->deactivateAnimate();
		_rejectRaftLandingFeature->deactivateRender();
	}

	int16 nextSeat = 0;
	for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
		ZmbSnoid *snoid = *it;
		if (!snoid || !snoid->isPackSnoid())
			continue;

		const int16 occupiedSeat = findDrawOnRegSlotByOccupant(snoid->getId());
		if (0 <= occupiedSeat) {
			// Retain every Snoid that the player has already seated.
			snoid->_packIsOccupied = true;
			snoid->_runnerStatus = kSnoidRunnerStatus00_Draggable;
			continue;
		}

		while (nextSeat < _drawOnRegCount && getDrawOnRegOccupant(nextSeat) != 0)
			nextSeat += 1;
		if (_drawOnRegCount <= nextSeat)
			break;

		addExternalDirtyRect(snoid->getClickRect());
		const Common::Point seatPos = _drawOnRegSnapPositions[nextSeat];
		snoid->setPointLoc(seatPos);
		snoid->setAnimTargetPos(seatPos);
		snoid->setFacingLeft(false);
		snoid->setCommonImageIndex(1);
		snoid->setAnimState(kSnoidAnimState000_Idle);
		snoid->setupIdleHotspots();
		snoid->activateRender();
		snoid->setNeedsRedraw(true);
		prepareSnoidVisualCoverage(snoid, true);
		addExternalDirtyRect(snoid->getClickRect());

		setDrawOnRegOccupant(nextSeat, snoid->getId());
		snoid->_packIsOccupied = true;
		snoid->_runnerStatus = kSnoidRunnerStatus00_Draggable;
		nextSeat += 1;
	}

	_acceptedSnoidCount = countAcceptedSnoids();
	setGoButtonsEnabled(0 < _acceptedSnoidCount);
}

ZmbChanceInfo ZoombiniPuzzleFerry::debugGetChances() const {
	// Captain Cajun allows the player to rearrange the seats without a failed placement consuming a limited resource.
	return {ZmbChanceInfo::ZmbChanceType::kInfinite};
}

Common::String ZoombiniPuzzleFerry::debugGetAnswer() const {
	// There is no generated target arrangement.
	// Every locally valid seating is an answer.
	// Some packs cannot fill every seat in a connected layout.
	const int16 seatCount = CLIP<int16>(_drawOnRegCount, 0, 20);
	Common::String answer = getDebugBanner();
	answer += Common::String::format(": %d seats\n", seatCount);
	answer += "  Rule: every pair of occupied adjacent seats must share one or more Zoombini traits.\n";
	answer += "  Every locally valid arrangement is an answer.\n";
	answer += "  Note: Some active packs cannot fill every seat in a connected layout.\n";
	return answer;
}

Common::String ZoombiniPuzzleFerry::debugGetBuiltinDebugCommandHelp() const {
	Common::String output;
	output += Common::String::format("  %-3s (%s)\n", "A/a", kBuiltinDebugActionAdjacency);
	output += "    Rebuild the live placement adjacency matrix from current seat bounds and draw all edges.\n";
	output += "    Later placements use the rebuilt matrix; level 4 includes its additional vertical-neighbor test.\n";
	output += Common::String::format("  %-3s (%s)\n", "F/f", kBuiltinDebugActionCaptain);
	output += "    On first use, retain the Captain runner's current SCRB when it is in 1800-1832, then replay that same SCRB and frame sound.\n";
	output += "    Repeated uses do not advance or cycle the retained SCRB.\n";
	output += Common::String::format("  %-3s (%s)\n", "L/l", kBuiltinDebugActionLevel);
	output += "    Show the one-based current route difficulty as Level 1 through Level 4 without changing it.\n";
	return output;
}

bool ZoombiniPuzzleFerry::debugDoBuiltinDebugCommand(int argc, const char **argv, Common::String &output) {
	if (argc != 3) {
		output = Common::String::format("Usage: page builtin_debug <%s|%s|%s>\n", kBuiltinDebugActionAdjacency, kBuiltinDebugActionCaptain, kBuiltinDebugActionLevel);
		return true;
	}

	const BuiltinDebugAction action = parseBuiltinDebugAction(argv[2]);
	if (action == BuiltinDebugAction::kInvalid) {
		output = Common::String::format("Unknown Captain Cajun debug action '%s'.\n", argv[2]);
		output += debugGetBuiltinDebugCommandHelp();
		return true;
	}
	return runBuiltinDebugAction(action, output);
}

ZoombiniPuzzleFerry::BuiltinDebugAction ZoombiniPuzzleFerry::parseBuiltinDebugAction(const Common::String &action) {
	if (action.equalsIgnoreCase(kBuiltinDebugActionAdjacency))
		return BuiltinDebugAction::kAdjacency;
	if (action.equalsIgnoreCase(kBuiltinDebugActionCaptain))
		return BuiltinDebugAction::kCaptain;
	if (action.equalsIgnoreCase(kBuiltinDebugActionLevel))
		return BuiltinDebugAction::kLevel;
	return BuiltinDebugAction::kInvalid;
}

bool ZoombiniPuzzleFerry::runBuiltinDebugAction(BuiltinDebugAction action, Common::String &output) {
	if (action == BuiltinDebugAction::kAdjacency) {
		buildAdjacencyMatrix();
		const int16 seatCount = CLIP<int16>(_drawOnRegCount, 0, 20);
		for (int16 seatIdx = 0; seatIdx < seatCount; seatIdx++) {
			const Common::Point start = _drawOnRegFeatures[seatIdx]->getClickRect().center();
			for (int16 slot = 0; slot < 8; slot++) {
				const int16 neighborIdx = _seatNeighborIndices[seatIdx][slot];
				if (neighborIdx < 0)
					continue;
				if (seatIdx < neighborIdx && neighborIdx < seatCount) {
					const Common::Point end = _drawOnRegFeatures[neighborIdx]->getClickRect().center();
					_vm->_gfx->drawLine(ZoombiniGraphics::kShapeScreen, start, end, ZoombiniGraphics::kColor0B_VeryDarkGray);
				}
			}
		}
		output = "Raft adjacency graph drawn.\n";
	} else if (action == BuiltinDebugAction::kCaptain) {
		if (!_captainRaftFeature) {
			output = "Captain Cajun runner is not loaded.\n";
			return true;
		}
		if (_debugCaptainScrbId < kResScrb1800_RaftBase || kResScrb1832_FlightFidget4 < _debugCaptainScrbId)
			_debugCaptainScrbId = _captainRaftFeature->getScrbId();
		loadScrbOntoFeature(_captainRaftFeature, _debugCaptainScrbId, true);
		output = Common::String::format("Play Caption Cajun SCRB id: %d\n", _debugCaptainScrbId);
	} else if (action == BuiltinDebugAction::kLevel) {
		output = Common::String::format("Level %d\n", static_cast<int>(_difficultyLevel));
	} else {
		return true;
	}

	if (action == BuiltinDebugAction::kCaptain)
		showBuiltinDebugText(Common::String::format("Play Caption Cajun SCRB id: %d", _debugCaptainScrbId));
	else if (action == BuiltinDebugAction::kLevel)
		showBuiltinDebugText(Common::String::format("Level %d", static_cast<int>(_difficultyLevel)));
	return false;
}

ZmbEventHandleResult ZoombiniPuzzleFerry::onDebugKeyDown(const Common::KeyState &kbd) {
	BuiltinDebugAction action = BuiltinDebugAction::kInvalid;
	if (kbd.hasFlags(0) || kbd.hasFlags(Common::KBD_SHIFT)) {
		if (kbd.ascii == 'A' || kbd.ascii == 'a')
			action = BuiltinDebugAction::kAdjacency;
		else if (kbd.ascii == 'F' || kbd.ascii == 'f')
			action = BuiltinDebugAction::kCaptain;
		else if (kbd.ascii == 'L' || kbd.ascii == 'l')
			action = BuiltinDebugAction::kLevel;
	}
	if (action == BuiltinDebugAction::kInvalid)
		return ZmbEventHandleResult::kPassthrough;

	Common::String output;
	runBuiltinDebugAction(action, output);
	return ZmbEventHandleResult::kConsumed;
}

// ---------------------------------------------------------------------------
// Interpret the cached seating-layout SCRB as metadata and create the raft runners.
//
// SCRB 1510-1529 is not registered or played as an animation runner. Its two decoded frames are
// parallel layout streams whose hotspot order, shape IDs, and positions describe the runners to create:
//   Frame 0 -- seat entries (shape IDs 1-3), which create DRAW_ON_REG runners
//   Frame 1 -- decoration entries (shape IDs 4-10), which create overlay runners
//
// Shape-zero entries are metadata delimiters rather than drawable runners. Each consecutive delimiter
// run ends one positive-shape run and yields to the other frame cursor. Leading decoration delimiters
// are skipped before this alternation begins. The resulting seat/decoration interleaving establishes
// the authored Z order through @ref ZoombiniPuzzleFerry::_seatPriorityOverlayFeatures.
//
// For seats (shape 1-3):
//   SCRB ID = kResScrb1500_SeatBase + shapeId - 1 (-> 1500, 1501, 1502)
//   Flags: DRAW_ON_REG | CHAIN_SCRIPT | DEFER_ANIM | POS_DELTA | OVERLAY | ZSORT_*
//   Snap position stored at layout pos + (22, -7)
//
// For decorations (shape 4-10, only when !lessAction):
//   SCRB ID = kResScrb1500_SeatBase + shapeId - 1 (-> 1503-1509)
//   Flags: DEFER_ANIM | PLAY_ONCE | POS_DELTA | OVERLAY | ZSORT_*
// ---------------------------------------------------------------------------
void ZoombiniPuzzleFerry::loadSeatLayout() {
	// @ref ZoombiniPage::getDecodedScrb() retains this immutable resource for the page lifetime.
	// The loader consumes decoded hotspot metadata directly because it needs no temporary runner state,
	// frame materialization, event dispatch, or playback timing for the layout SCRB itself.
	const ZmbScriptDecoder::DecodedScrb *layoutScrb = getDecodedScrb(ZmbResource(ZmbResource::kPage, _seatLayoutScrbId));
	if (!layoutScrb || layoutScrb->frames.size() < 2) {
		error("ferry: required seating SCRB %u is malformed", _seatLayoutScrbId);
		return;
	}

	// Walk both frame groups with independent cursors to establish the required Z order.
	// A run of one or more zero-shape entries yields once to the other group.
	// Use @ref ZoombiniPuzzleFerry::_seatPriorityOverlayFeatures as the priority chains.
	// Seat runners carry @ref ZmbFeature::FLAG_00002000_DRAW_ON_REG.
	// @ref ZoombiniPage::registerFeature() registers them in the shared drop-target arrays.

	constexpr uint32 seatFlags = ZmbFeature::FLAG_00002000_DRAW_ON_REG | ZmbFeature::FLAG_00040000_CHAIN_SCRIPT | ZmbFeature::FLAG_00080000_DEFER_ANIM |
								 ZmbFeature::FLAG_00800000_POS_DELTA | ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_10000000_ZSORT_RIGHT |
								 ZmbFeature::FLAG_20000000_ZSORT_BOTTOM | ZmbFeature::FLAG_40000000_ZSORT_LEFT;

	constexpr uint32 decoFlags = ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_00800000_POS_DELTA |
								 ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_10000000_ZSORT_RIGHT | ZmbFeature::FLAG_20000000_ZSORT_BOTTOM |
								 ZmbFeature::FLAG_40000000_ZSORT_LEFT;

	bool lessAction = _vm->_state->isLessActionEnabled();
	int16 seatCountBefore = _drawOnRegCount;
	const Common::Array<ZmbHotspot> &seatHotspots = layoutScrb->frames[0].hotspots;
	const Common::Array<ZmbHotspot> &decoHotspots = layoutScrb->frames[1].hotspots;
	uint32 seatCursor = 0;
	uint32 decoCursor = 0;
	bool seatFinished = seatHotspots.empty();
	bool decoFinished = decoHotspots.empty();
	bool processSeatGroup = true;

	// Decoration padding before the first positive entry does not consume an alternation.
	while (!decoFinished && decoHotspots[decoCursor]._shapeIdx == 0) {
		decoCursor += 1;
		decoFinished = decoHotspots.size() <= decoCursor;
	}

	while (!seatFinished || !decoFinished) {
		const Common::Array<ZmbHotspot> &hotspots = processSeatGroup ? seatHotspots : decoHotspots;
		uint32 &cursor = processSeatGroup ? seatCursor : decoCursor;
		bool &finished = processSeatGroup ? seatFinished : decoFinished;

		if (finished) {
			processSeatGroup = !processSeatGroup;
			continue;
		}

		const ZmbHotspot &hotspot = hotspots[cursor];
		const int16 shapeId = hotspot._shapeIdx;
		if (shapeId == 0) {
			do {
				cursor += 1;
				finished = hotspots.size() <= cursor;
			} while (!finished && hotspots[cursor]._shapeIdx == 0);
			processSeatGroup = !processSeatGroup;
			continue;
		}
		cursor += 1;
		finished = hotspots.size() <= cursor;

		if (1 <= shapeId && shapeId <= 3 && (_drawOnRegCount - seatCountBefore) < 20) {
			const int16 scrbId = static_cast<int16>(kResScrb1500_SeatBase + shapeId - 1);
			const Common::Point entryPos = hotspot.getPos();

			ZmbFeature *seatFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1500_Seats), scrbId, 6, entryPos, seatFlags);
			_departureShiftFeatures.push_back(seatFeature);

			// @ref ZoombiniPage::registerFeature() used the entry position as the initial snap point.
			// Apply the seat's (22, -7) snap offset.
			setDrawOnRegSnapPosition(_drawOnRegCount - 1, Common::Point(entryPos.x + 22, entryPos.y - 7));
		} else if (!lessAction && 4 <= shapeId && shapeId <= 10) {
			const int16 scrbId = static_cast<int16>(kResScrb1500_SeatBase + shapeId - 1);

			ZmbFeature *decoFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1500_Seats), scrbId, 6, hotspot.getPos(), decoFlags);
			_departureShiftFeatures.push_back(decoFeature);
		}
	}
}

// ---------------------------------------------------------------------------
// Compute adjacency between seat positions.
//
// For every pair of seats, test if their expanded bounding boxes overlap.
// Two overlap test orientations are always tried (vertical + horizontal expand).
// Level 4 adds a third test with vertical-only expansion.
// Matching pairs store 0-based neighbor indices in the adjacency matrix (max 8 per seat).
// ---------------------------------------------------------------------------
void ZoombiniPuzzleFerry::buildAdjacencyMatrix() {
	buildAdjacencyMatrix(_seatNeighborIndices);
}

void ZoombiniPuzzleFerry::buildAdjacencyMatrix(int8 seatNeighborIndices[20][8]) const {
	// It seems the active pack was planned to have a 20 snoids, but reduced to 16 later.
	// Since this is how original logic works, keep 20 limit anyway.
	for (int16 seatIdx = 0; seatIdx < 20; seatIdx++) {
		for (int16 slot = 0; slot < 8; slot++)
			seatNeighborIndices[seatIdx][slot] = -1;
	}
	const int16 seatCount = CLIP<int16>(_drawOnRegCount, 0, 20);

	// Gather the exact rendered bounds from the required seat runners.
	// Registration materializes DRAW_ON_REG frame zero before the adjacency pass.
	Common::Rect seatRects[20];
	for (int16 i = 0; i < seatCount; i++) {
		ZmbFeature *seatRunner = _drawOnRegFeatures[i];
		if (!seatRunner || !seatRunner->hasClickRect() || seatRunner->getClickRect().isEmpty()) {
			error("ferry: seat %d has no valid rendered bounds", i + 1);
			return;
		}
		seatRects[i] = seatRunner->getClickRect();
	}

	for (int16 seatIdx = 0; seatIdx < seatCount; seatIdx++) {
		int16 slotCount = 0;
		const Common::Rect &rectK = seatRects[seatIdx];
		const int16 halfHeight = MAX<int16>(0, (rectK.bottom - rectK.top) / 2 - 2);

		for (int16 comparisonSeatIdx = 0; comparisonSeatIdx < seatCount; comparisonSeatIdx++) {
			if (comparisonSeatIdx == seatIdx)
				continue;

			const Common::Rect &rectM = seatRects[comparisonSeatIdx];
			bool adjacent = false;

			{ // Test 1: Vertical expansion -- expand top/bottom by halfHeight
				Common::Rect expandedK(rectK.left + halfHeight, rectK.top - halfHeight, rectK.right - halfHeight, rectK.bottom + halfHeight);
				adjacent = expandedK.intersects(rectM);
			}

			// Test 2: Horizontal expansion -- expand left/right by halfHeight
			if (!adjacent) {
				Common::Rect expandedK(rectK.left - halfHeight, rectK.top + halfHeight, rectK.right + halfHeight, rectK.bottom - halfHeight);
				adjacent = expandedK.intersects(rectM);
			}

			// Test 3: vertical-only expansion for level 4.
			if (!adjacent && kPuzzleLevel4 <= _difficultyLevel) {
				Common::Rect expandedK(rectK.left, rectK.top - halfHeight, rectK.right, rectK.bottom + halfHeight);
				adjacent = expandedK.intersects(rectM);
			}

			if (adjacent && slotCount < 8) {
				seatNeighborIndices[seatIdx][slotCount] = static_cast<int8>(comparisonSeatIdx);
				slotCount += 1;
			}
		}
	}
}

// ---------------------------------------------------------------------------
// Test whether a point is near an empty seat snap position.
// Test each slot within @ref ZoombiniInteractive::_clickZoneRadius.
// Return a one-based seat number, or zero if no seat matches.
// ---------------------------------------------------------------------------
int16 ZoombiniPuzzleFerry::getDropTargetSeatNumber(const Common::Point &pos) const {
	const int16 seatIdx = hitTestDrawOnRegSlot(pos, _clickZoneRadius, true);
	return 0 <= seatIdx ? seatIdx + 1 : 0;
}

// ---------------------------------------------------------------------------
// Check whether a dropped Snoid shares a trait with any occupied adjacent seat.
// @p seatIdx is zero-based.
// A valid placement also updates @ref ZoombiniPuzzleFerry::_matchedTraitMask.
// ---------------------------------------------------------------------------
bool ZoombiniPuzzleFerry::testAdjacentMatch(int16 seatIdx, ZmbSnoid *droppedSnoid) {
	if (!_seatAdjacencyReady || seatIdx < 0 || 20 <= seatIdx || !droppedSnoid) {
		error("ferry: cannot test an invalid or uninitialized adjacency entry");
		return false;
	}

	// Placement is valid only if every occupied neighbor shares at least one trait with the dropped Snoid.
	// Accepting after the first matching neighbor would under-restrict the puzzle.
	_matchedTraitMask = 0;
	bool valid = true;

	for (int16 slot = 0; slot < 8 && valid; slot++) {
		const int16 neighborIdx = _seatNeighborIndices[seatIdx][slot];
		if (neighborIdx < 0)
			continue;

		uint16 occupantId = getDrawOnRegOccupant(neighborIdx);
		if (occupantId == 0)
			continue;
		ZmbSnoid *neighborSnoid = getSnoid(occupantId);
		if (!neighborSnoid)
			continue;

		// Each occupied neighbor must contribute at least one trait match.
		valid = false;

		for (int16 traitIdx = 0; traitIdx < 4; traitIdx++) {
			if (droppedSnoid->_trait[traitIdx] == neighborSnoid->_trait[traitIdx]) {
				_matchedTraitMask |= (1u << traitIdx);
				valid = true;
			}
		}
	}

	// A seat with no occupied neighbors remains valid from the initial assignment.
	return valid;
}

// Count currently accepted active-pack Snoids.
int16 ZoombiniPuzzleFerry::countAcceptedSnoids() const {
	int16 acceptedCount = 0;
	for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
		const ZmbSnoid *snoid = *it;
		if (snoid && snoid->isPackSnoid() && snoid->_packIsOccupied)
			acceptedCount += 1;
	}
	return acceptedCount;
}

bool ZoombiniPuzzleFerry::hasPendingWalkIn() const {
	for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
		const ZmbSnoid *snoid = *it;
		if (!snoid || !snoid->isPackSnoid())
			continue;
		const SnoidAnimState state = snoid->getAnimState();
		if (state == kSnoidAnimState007_Depart || state == kSnoidAnimState112_Path)
			return true;
	}
	return false;
}

int16 ZoombiniPuzzleFerry::findOpenRejectSlot(bool occupiedSlots[20]) const {
	memset(occupiedSlots, 0, sizeof(bool) * 20);
	Common::Array<uint16> assignedRunnerIds;
	assignIdleSnoidsToSlots(kDockSnoidPositions, 20, 500, true, assignedRunnerIds);
	for (int16 slot = 0; slot < 20; slot++)
		occupiedSlots[slot] = assignedRunnerIds[slot] != 0;

	const bool scanForward = 50 < _vm->_rnd->getRandomNumber(1, 100);
	if (scanForward) {
		for (int16 slot = 0; slot <= 18; slot += 2) {
			if (!occupiedSlots[slot])
				return slot;
		}
	} else {
		for (int16 slot = 18; 0 <= slot; slot -= 2) {
			if (!occupiedSlots[slot])
				return slot;
		}
	}

	const bool scanOddForward = 50 < _vm->_rnd->getRandomNumber(1, 100);
	if (scanOddForward) {
		for (int16 slot = 1; slot <= 19; slot += 2) {
			if (!occupiedSlots[slot])
				return slot;
		}
	} else {
		for (int16 slot = 19; 1 <= slot; slot -= 2) {
			if (!occupiedSlots[slot])
				return slot;
		}
	}
	return 0;
}

void ZoombiniPuzzleFerry::shiftDepartureRunners(int16 shiftAmount) {
	if (shiftAmount <= 0)
		return;

	for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
		ZmbSnoid *snoid = *it;
		if (!snoid || !snoid->isPackSnoid() || !snoid->_packIsOccupied)
			continue;
		markFeatureVisualCoverageDirty(snoid, false);
		Common::Point pos = snoid->getPointLoc();
		pos.x += shiftAmount;
		snoid->setPointLoc(pos);
		Common::Point target = snoid->getAnimTargetPos();
		target.x += shiftAmount;
		snoid->setAnimTargetPos(target);
		// Snoids use their layered render cache instead of the generic feature cache.
		snoid->setNeedsRedraw(true);
		prepareSnoidVisualCoverage(snoid, true);
	}

	for (uint32 i = 0; i < _departureShiftFeatures.size(); i++) {
		ZmbFeature *feature = _departureShiftFeatures[i];
		if (!feature)
			continue;
		markFeatureVisualCoverageDirty(feature, false);
		Common::Point pos = feature->getPointLoc();
		pos.x += shiftAmount;
		feature->setPointLoc(pos);
		feature->requestVisualRematerialization();
	}
}

// ---------------------------------------------------------------------------
// Store the one pending reject owned by the Captain controller.
// ---------------------------------------------------------------------------
void ZoombiniPuzzleFerry::startRejectRequest(uint16 snoidId, ZmbFeature *rejectedSeatFeature, int16 reactionScrbId) {
	_snoidInputEnabled = false;
	_pendingCaptainScrbId = 0;
	_rejectedSnoidId = snoidId;
	_rejectedSeatFeature = rejectedSeatFeature;
	_pendingRejectReactionScrbId = reactionScrbId;
	_rejectSetupPending = true;
}

void ZoombiniPuzzleFerry::completeRejectRequest() {
	_rejectedSnoidId = 0;
	_rejectedSeatFeature = nullptr;
	_pendingRejectReactionScrbId = 0;
	_rejectSetupPending = false;
	_rejectFlightSnoid = nullptr;
	_rejectControllerActive = false;
	_mainRejectSnoidCallbackActive = false;
	_snoidInputEnabled = true;
}

// ---------------------------------------------------------------------------
// Set up the reject controller and its landing target.
// ---------------------------------------------------------------------------
void ZoombiniPuzzleFerry::startRejectWalk(int16 destinationCode) {
	if (destinationCode < 0 || 10 <= destinationCode)
		destinationCode = 0;
	_rejectDestinationCode = destinationCode;

	if (_rejectedSnoidId == 0)
		return;

	// Every rejection uses the Snoid's saved origin as the script-end anchor.
	// The dock, Snoid-exit, and raft-departure branches all start from it.
	_rejectReturnPosition = _dragOrigPos;

	if (10 <= destinationCode || destinationCode == 0) {
		// Dock exit
		_rejectControllerScrbId = kResScrb1605_DockExit;
		_rejectLandingPosition = Common::Point(122, 164);
	} else if (1 <= destinationCode && destinationCode <= 6) {
		// Rejected-Snoid exit path -- 50/50 chance of 1604 or 1606
		if (50 < _vm->_rnd->getRandomNumber(1, 100))
			_rejectControllerScrbId = kResScrb1606_RejectWalkB;
		else
			_rejectControllerScrbId = kResScrb1604_RejectWalkA;
		_rejectLandingPosition = _dragOrigPos;
	} else if (7 <= destinationCode && destinationCode <= 9) {
		// Reject-raft path uses SCRB 1607 plus the deferred 1704-1706 features.
		_rejectControllerScrbId = kResScrb1607_RaftDeparture;

		// Replace any previous reject-raft takeoff and landing features.
		if (_rejectRaftTakeoffFeature) {
			unloadScrbFeature(_rejectRaftTakeoffFeature);
			_rejectRaftTakeoffFeature = nullptr;
		}
		if (_rejectRaftLandingFeature) {
			unloadScrbFeature(_rejectRaftLandingFeature);
			_rejectRaftLandingFeature = nullptr;
		}

		_rejectRaftTakeoffFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1700_Navigation), kResScrb1705_RaftDepartureA, 6,
													ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_01000000_DEFER_RENDER);

		// Offset the auxiliary raft runner 14 pixels up and left.
		Common::Point raftPos(_dragOrigPos.x - 14, _dragOrigPos.y - 14);
		_rejectRaftLandingFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1700_Navigation), kResScrb1706_RaftDepartureB, 6, raftPos,
													ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_00800000_POS_DELTA |
														ZmbFeature::FLAG_01000000_DEFER_RENDER);

		_rejectLandingPosition = Common::Point(236, 474);
		_rejectReturnPosition = _dragOrigPos;
	}

	// The raft runner owns the reject-flight controller SCRB and its frame sounds.
	// The rejected Snoid is tracked separately for the duration of the flight
	// so cases 4 and 5 can load its paired takeoff and landing SCRS.
	ZmbSnoid *rejectedSnoid = nullptr;
	for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
		ZmbSnoid *s = *it;
		if (s && s->getId() == _rejectedSnoidId) {
			rejectedSnoid = s;
			break;
		}
	}
	if (rejectedSnoid) {
		// Reset transient flight state from any prior reject before re-arming.
		_mainRejectSnoidCallbackActive = false;
		// Load the controller SCRB 1604-1607 onto the raft runner.
		// Their event sequences are:
		//   - 1604, 1606, 1607 emit only event 2 -> case 1 (snoid SCRS load,
		//     hide-on-complete). Snoid sails away invisible.
		//   - 1605 emits events 2 and 3 -> case 1 then case 2 (loads landing
		//     SCRS with initial-position pointer for the dock-exit slide-in).
		// While the reject flight is active, page-level dispatch routes raft events
		// to @ref ZoombiniPuzzleFerry::processRaftFlightEvent().
		// Track the rejected Snoid separately from the raft runner.
		_rejectFlightSnoid = rejectedSnoid;
		_rejectControllerActive = true;
		if (_captainRaftFeature) {
			loadScrbOntoFeature(_captainRaftFeature, _rejectControllerScrbId);
			playCurrentFrameSound(_captainRaftFeature);
			_activeCaptainScrbId = _rejectControllerScrbId;
		}
	}

	// Keep Snoid pickup input disabled for the complete reject-flight controller chain.
	// The ordinary landing SCRS and the raft paired landing SCRS enable it at
	// their respective transition boundaries.
	// Reject flight keeps the drag-established list order until its callback
	// explicitly requests another positional sort.
	setRunnerZSortEnabled(false);
}

// ---------------------------------------------------------------------------
// Set up a pending reject controller from @ref ZoombiniPuzzleFerry::onPostRenderFrame().
// ---------------------------------------------------------------------------
void ZoombiniPuzzleFerry::handleRejectWalkSetup() {
	_rejectSetupPending = false;

	bool occupiedSlots[20];
	int16 openSlot = findOpenRejectSlot(occupiedSlots);

	// Choose a non-repeating transport animation independently of the open pedestal.
	// The dock and raft variants impose their own landing-clearance requirements.
	int16 dest;
	bool retry;
	do {
		retry = false;
		dest = _vm->_rnd->getNonRepeatRandom(10, _vm->_state->getFerryRuntimeState()._rejectDestinationPoolState);

		// SCRB 1605 lands at (122,164), between zero-based dock slots 14 and 12.
		if (dest == 0) {
			retry = occupiedSlots[12] || occupiedSlots[14];
		} else if (7 <= dest && dest <= 9) {
			// Raft destinations require an available odd-index seat from 11 through 19.
			retry = true;
			for (int16 slot = 19; retry && 11 <= slot; slot -= 2) {
				if (occupiedSlots[slot]) {
					break;
				} else if (!occupiedSlots[slot - 1] && !occupiedSlots[slot - 3]) {
					retry = false;
					openSlot = slot;
				}
			}
		}
	} while (retry);

	_dragOrigPos = kDockSnoidPositions[openSlot];
	startRejectWalk(dest);
}

// ---------------------------------------------------------------------------
// Update the Ferry puzzle once per animation frame.
// ---------------------------------------------------------------------------
void ZoombiniPuzzleFerry::onEveryFrame() {
	if (!_pageActive)
		return;
	if (_departureState == ZmbDepartureState::kCompleted) {
		executeDeparture();
		return;
	}

	// -----------------------------------------------------------------------
	// [0] Pending Go departure
	// -----------------------------------------------------------------------
	if (_departureState == ZmbDepartureState::kTriggered &&
		!_rejectSetupPending && _snoidInputEnabled) {
		// Free landscape and approach runners
		if (_landscapeOverlayFeature) {
			unloadScrbFeature(_landscapeOverlayFeature);
			_landscapeOverlayFeature = nullptr;
		}
		if (_raftApproachFeatureA) {
			unloadScrbFeature(_raftApproachFeatureA);
			_raftApproachFeatureA = nullptr;
		}
		if (_raftApproachFeatureB) {
			unloadScrbFeature(_raftApproachFeatureB);
			_raftApproachFeatureB = nullptr;
		}

		// Play departure SCRB (random 1608-1609)
		int16 departScrb = _vm->_rnd->getRandomNumber(kResScrb1608_DepartureA, kResScrb1609_DepartureB);
		if (_captainRaftFeature) {
			loadScrbOntoFeature(_captainRaftFeature, departScrb);
			playCurrentFrameSound(_captainRaftFeature);
		}

		// Defer transition cleanup until the registered 1608/1609 controller group has completed.
		_departureState = ZmbDepartureState::kAnimating;
		return;
	}
}

void ZoombiniPuzzleFerry::onPostRenderFrame() {
	if (!_pageActive || isDeparturePending())
		return;

	// -----------------------------------------------------------------------
	// [1] Pending Captain SCRB animation.
	// Consume callbacks from the current render before scheduling independent idle fidgets.
	// The initial reject reaction uses a dedicated slot ahead of other Captain reactions.
	// -----------------------------------------------------------------------
	if (_pendingRejectReactionScrbId != 0) {
		int16 scrb = _pendingRejectReactionScrbId;
		_pendingRejectReactionScrbId = 0;

		if (_captainRaftFeature) {
			loadScrbOntoFeature(_captainRaftFeature, scrb);
			playCurrentFrameSound(_captainRaftFeature);
			_activeCaptainScrbId = scrb;
		}
	} else if (_pendingCaptainScrbId != 0) {
		int16 scrb = _pendingCaptainScrbId;
		_pendingCaptainScrbId = 0;

		if (_captainRaftFeature) {
			loadScrbOntoFeature(_captainRaftFeature, scrb);
			playCurrentFrameSound(_captainRaftFeature);
			_activeCaptainScrbId = scrb;
		}
	}
	// -----------------------------------------------------------------------
	// [2] Reject callback case 2 requested the SCRS 1907 raft-overlay path.
	// -----------------------------------------------------------------------
	else if (_rejectRaftOverlayPending) {
		_rejectRaftOverlayPending = false;

		// Activate the deferred reject-raft overlay and takeoff features.
		if (_rejectRaftOverlayFeature) {
			_rejectRaftOverlayFeature->activateAnimate();
			_rejectRaftOverlayFeature->activateRender();
		}
		if (_rejectRaftTakeoffFeature) {
			_rejectRaftTakeoffFeature->activateAnimate();
			_rejectRaftTakeoffFeature->activateRender();
		}
	}
	// -----------------------------------------------------------------------
	// [3] A rejected drop is waiting for its Captain reaction to complete.
	// -----------------------------------------------------------------------
	else if (_rejectSetupPending) {
		// Wait until the active Captain SCRB clears at its end-of-cycle event.
		// Animation activation is not a valid gate because it remains set after SCRB loading.
		if (_activeCaptainScrbId == 0) {
			handleRejectWalkSetup();
		}
	}
	// -----------------------------------------------------------------------
	// [4] Idle fidget timer
	// This is the final branch of the one-slot pending/reject chain.
	// Pending reject handoffs take priority over independent fidgets.
	// The shared activity timer prevents a normal reject sequence from reaching an overdue fidget.
	// The long 5400..10800 engine-frame threshold prevents continuous chatter.
	// -----------------------------------------------------------------------
	else if (_captainIdleFidgetFrameThreshold < _vm->getActivityIdleFrames()) {
		_vm->resetFidgetActivity();
		// Select random fidget SCRB
		uint16 fidgetPoolIdx = _vm->_rnd->getNonRepeatRandom(5, _vm->_state->getFerryRuntimeState()._idleFidgetPoolState);
		_pendingCaptainScrbId = kCaptainIdleFidgetScrbPool[fidgetPoolIdx];

		_captainIdleFidgetFrameThreshold = _vm->_rnd->getRandomNumber(5400, 10800);
	}

	// -----------------------------------------------------------------------
	// [5] Trait display scheduling
	// Schedule the opt-in trait-match display on the exact Snoid.
	// -----------------------------------------------------------------------
	if (_traitHighlightSnoidId != 0) {
		if (!_traitHighlightEnabled) {
			_matchedTraitMask = 0;
			_traitHighlightSnoidId = 0;
		} else {
			ZmbSnoid *snoid = getIdleSnoid(_traitHighlightSnoidId);
			if (snoid) {
				snoid->startTraitHighlight(_matchedTraitMask);
				_matchedTraitMask = 0;
				_traitHighlightSnoidId = 0;
			}
		}
	}

	// -----------------------------------------------------------------------
	// [6] Update Go button enabled state
	// -----------------------------------------------------------------------
	setGoButtonsEnabled(0 < _acceptedSnoidCount);

	// -----------------------------------------------------------------------
	// [7] Initial captain animation
	// Start only after the walk-in queue and SND 997 have both finished.
	// -----------------------------------------------------------------------
	const bool introSoundFinished = !_introMoveSfxHandleValid ||
									!_vm->_system->getMixer()->isSoundHandleActive(_introMoveSfxHandle);
	if (!_initialCaptainScriptRestarted && !hasPendingWalkIn() && introSoundFinished) {
		_initialCaptainScriptRestarted = true;
		if (_captainRaftFeature) {
			loadScrbOntoFeature(_captainRaftFeature, 0);
			playCurrentFrameSound(_captainRaftFeature);
		}
	}

	// Ambient sound is driven by the base interactive frame loop
}

// ---------------------------------------------------------------------------
// Handle a mouse-button press.
// ---------------------------------------------------------------------------
ZmbEventHandleResult ZoombiniPuzzleFerry::onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) {
	// Treat a click during a pending or active Go transition as an animation skip.
	// Perform the queued transition immediately.
	if (_departureState == ZmbDepartureState::kTriggered ||
		_departureState == ZmbDepartureState::kAnimating) {
		_departureState = ZmbDepartureState::kCompleted;
		executeDeparture();
		return ZmbEventHandleResult::kConsumed;
	}

	// Let interactive base handle Go/Map/Help buttons
	ZmbEventHandleResult result = ZoombiniInteractive::onLButtonDown(absPos, relPos);
	if (result == ZmbEventHandleResult::kConsumed)
		return result;

	// Snoid pickup becomes available after the reject controller hands the active Snoid to its return script.
	if (!_snoidInputEnabled || hasPendingWalkIn())
		return ZmbEventHandleResult::kPassthrough;
	if (isDragging())
		return ZmbEventHandleResult::kPassthrough;

	// Find snoid at click position
	ZmbSnoid *snoid = findSnoidAtPoint(absPos);
	if (!snoid)
		return ZmbEventHandleResult::kPassthrough;

	_pickupWasSeated = snoid->_packIsOccupied;
	snoid->_packIsOccupied = false;
	// Begin drag
	startSnoidDrag(snoid, absPos);
	_acceptedSnoidCount = countAcceptedSnoids();

	// Picking up a Snoid does not queue a captain reaction.
	// The move-reaction SCRB pool is selected without repetition for the non-seat drop path.
	// It contains SCRB 1820, 1821, and 1822.
	// when the drop is inside the captain's reaction zone.
	// Queueing it on pickup would add an extra captain voice before legitimate welcome/good/bad reactions.
	// The non-seat drop path queues the move SCRB for terrain drops.

	return ZmbEventHandleResult::kConsumed;
}

// ---------------------------------------------------------------------------
// Finish a drag and process its drop on mouse-button release.
// Use DRAW_ON_REG occupancy to track raft seats and select snap positions.
// ---------------------------------------------------------------------------
void ZoombiniPuzzleFerry::endDrag(const Common::Point &mousePos) {
	(void)mousePos;
	ZmbSnoid *snoid = finishSnoidDrag();
	if (!snoid)
		return;
	// Keep Captain feedback alive while the Snoid movement queue handles its own lifetime.
	// This is a deliberate drift from the original.
	// It would have called '_vm->_sound->releaseAllLoadedSounds();' oterwise.

	Common::Point snoidPos = snoid->getPointLoc();
	_dropTargetSeatNumber = getDropTargetSeatNumber(snoidPos);

	if (0 < _dropTargetSeatNumber) {
		const int16 seatIdx = _dropTargetSeatNumber - 1;
		// Both accepted and rejected drops enter the arrival state and settle into the seat's standing pose
		// before Captain feedback.
		snoid->setAnimTargetPos(_drawOnRegSnapPositions[seatIdx]);
		snoid->setCommonImageIndex(1);
		snoid->setAnimState(kSnoidAnimState004_Arrive);

		// Dropped on an empty seat -- test adjacency matching
		bool valid = testAdjacentMatch(seatIdx, snoid);

		if (valid) {
			// ---------------------------------------------------------------
			// [CORRECT PLACEMENT]
			// ---------------------------------------------------------------
			setDrawOnRegOccupant(seatIdx, snoid->getId());
			_consecutiveRejectedPlacements = 0;
			if (!_pickupWasSeated)
				_consecutiveAcceptedPlacements += 1;

			if (countAcceptedSnoids() + 1 == _pageLoadedZmbCount || _consecutiveAcceptedPlacements == _goodReactionStreakTarget) {
				_goodReactionStreakTarget += _vm->_rnd->getRandomNumber(3, 5);

				if (_firstGoodReactionQueued) {
					uint16 reactionPoolIdx = _vm->_rnd->getNonRepeatRandom(2, _vm->_state->getFerryRuntimeState()._goodReactionPoolState);
					_pendingCaptainScrbId = kCaptainGoodReactionScrbPool[reactionPoolIdx];
				} else {
					_firstGoodReactionQueued = true;
					_pendingCaptainScrbId = kResScrb1816_FirstGoodReaction;
				}
			}

			// Mark snoid as seated. The shared drag path above already selected
			// the exact snap position and entered Arrive state 4.
			snoid->_packIsOccupied = true;
			snoid->_runnerStatus = kSnoidRunnerStatus00_Draggable;

			// The opt-in adjustment exposes the dormant display in every game mode.
			if (_matchedTraitMask && _traitHighlightEnabled) {
				_traitHighlightSnoidId = snoid->getId();
			}

		} else {
			// ---------------------------------------------------------------
			// [WRONG PLACEMENT]
			// ---------------------------------------------------------------
			// Count the failure and clear the success streak.
			_consecutiveRejectedPlacements += 1;
			_consecutiveAcceptedPlacements = 0;
			_goodReactionStreakTarget = 1;

			// Undo the optimistic seat occupancy written when the cursor was released over a seat.
			// Otherwise, the rejected Snoid marks its target as occupied before flying back.
			// That stale mark permanently blocks the seat.
			clearDrawOnRegOccupant(seatIdx);

			// Mark snoid for rejection
			snoid->_packIsOccupied = false;
			snoid->_runnerStatus = kSnoidRunnerStatus01_RejectReturning;
			// Use the harsh reaction when the failure threshold is reached;
			// otherwise choose a non-repeating bad-reaction script.
			int16 rejectReactionScrb;
			if (_vm->_rnd->getRandomNumber(3, 5) == _consecutiveRejectedPlacements) {
				rejectReactionScrb = kResScrb1815_HarshBadReaction;
				_consecutiveRejectedPlacements = 5; // prevent further harsh rejects
			} else {
				uint16 reactionPoolIdx = _vm->_rnd->getNonRepeatRandom(11, _vm->_state->getFerryRuntimeState()._badReactionPoolState);
				rejectReactionScrb = kCaptainBadReactionScrbPool[reactionPoolIdx];
			}
			startRejectRequest(snoid->getId(), _drawOnRegFeatures[seatIdx], rejectReactionScrb);
		}
	} else {
		// Dropped outside any seat -- check terrain or dock validity
		// The shared drag routine uses Arrive state 4 for a validated release.
		// It uses the same state for a return to the pickup point.
		const bool settledAtRelease = settleSnoidAfterTerrainDrop(snoid, _dragOrigPos);
		snoid->_runnerStatus = kSnoidRunnerStatus00_Draggable;

		// A changed terrain result enters Ferry's return-slot path.
		const bool returnNeeded = !settledAtRelease;
		if (!returnNeeded) {
			_pickupWasSeated = false;
		} else {
			const Common::Rect returnSlotRect(0, 130, 469, 240);
			if (returnSlotRect.contains(_dragOrigPos)) {
				snoid->setAnimTargetPos(_dragOrigPos);
			} else {
				bool occupiedSlots[20];
				const int16 returnSlot = findOpenRejectSlot(occupiedSlots);
				snoid->setAnimTargetPos(kDockSnoidPositions[returnSlot]);
				_pickupWasSeated = false;
			}

			// Test the rejected release point against every seat snap, including occupied seats.
			// The pending Captain Cajun slot remains replaceable even while another script is playing.
			if (!_pendingCaptainScrbId &&
				0 <= hitTestDrawOnRegSlot(snoidPos, _clickZoneRadius, false)) {
				uint16 reactionPoolIdx = _vm->_rnd->getNonRepeatRandom(3, _vm->_state->getFerryRuntimeState()._moveReactionPoolState);
				_pendingCaptainScrbId = kCaptainMoveReactionScrbPool[reactionPoolIdx];
			}
		}
		snoid->_packIsOccupied = _pickupWasSeated;
	}

	// Count only accepted Snoids, not a DRAW_ON_REG slot that may still contain an optimistic drag assignment.
	_acceptedSnoidCount = countAcceptedSnoids();
	_pickupWasSeated = false;
}

// ---------------------------------------------------------------------------
// Route raft-runner events into the reject-flight handler.
// ---------------------------------------------------------------------------
void ZoombiniPuzzleFerry::onFeatureAnimEvent(ZmbFeature *feature, int16 eventCode) {
	if (feature == _captainRaftFeature) {
		if (_departureState == ZmbDepartureState::kAnimating) {
			if (eventCode == kAnimEventM1_End) {
				// The solved departure raft animation reached its terminal marker.
				_departureState = ZmbDepartureState::kCompleted;
			} else {
				shiftDepartureRunners(eventCode);
			}
			return;
		} else if (_rejectControllerActive && eventCode != kAnimEventM1_End) {
			// The raft is running reject-flight controller SCRB 1604-1607.
			// Route mid-frame event codes 1-6 into the SCRB-to-SCRS handoff.
			processRaftFlightEvent(eventCode);
			return;
		} else if (eventCode == kAnimEventM1_End) {
			// Continue a pending reject at the reaction runner's terminal event.
			// This keeps the same runner under one owner across the hand-off.
			_activeCaptainScrbId = 0;
			_rejectControllerActive = false;
			if (_rejectSetupPending) {
				handleRejectWalkSetup();
			}
		}
	} else if (feature == _rejectRaftOverlayFeature || feature == _rejectRaftTakeoffFeature || feature == _rejectRaftLandingFeature) {
		// SCRB 1704/1705 continue the raft reject hand-off with cases 3, 6,
		// and 4. The Snoid SCRS started by case 4 emits case 5 itself.
		if (eventCode != kAnimEventM1_End)
			processRaftFlightEvent(eventCode);
	} else if (feature->hasFlag(ZmbFeature::FLAG_00000001_TYPE_SNOID)) {
		// Snoid animation event
		ZmbSnoid *snoid = static_cast<ZmbSnoid *>(feature);
		if (eventCode != kAnimEventM1_End) {
			const int16 activeScrsId = snoid->getActiveScrsId();
			if (kResScrs0999_RaftLandingBase <= activeScrsId && activeScrsId <= 1009 &&
				((activeScrsId - kResScrs0999_RaftLandingBase) % 2) == 0) {
				processRaftLandingEvent(snoid, eventCode);
				return;
			}
			if (_mainRejectSnoidCallbackActive &&
				snoid == _rejectFlightSnoid &&
				kRaftEventCode001_StartRejectFlight <= eventCode &&
				eventCode <= kRaftEventCode006_QueueCaptainFidget) {
				processRaftFlightEvent(eventCode);
				return;
			}
		} else {
			if (snoid != _rejectFlightSnoid) {
				snoid->setAnimState(kSnoidAnimState000_Idle);
				snoid->setupIdleHotspots();
			}
		}
	}
}

// ---------------------------------------------------------------------------
// Process the feet-specific raft landing SCRS callback.
// Event 6 starts the synchronized SCRB 1706 landing effect.
// Every other event alternates the landing Snoid's facing direction.
// ---------------------------------------------------------------------------
void ZoombiniPuzzleFerry::processRaftLandingEvent(ZmbSnoid *snoid, int16 callbackCode) {
	if (!snoid)
		return;

	if (callbackCode == kRaftLandingEventCode006_StartLandingEffect) {
		if (_rejectRaftLandingFeature) {
			loadScrbOntoFeature(_rejectRaftLandingFeature, kResScrb1706_RaftDepartureB);
			joinFeatureTimingGroup(snoid, _rejectRaftLandingFeature);
		}
		return;
	}

	snoid->setFacingLeft(!snoid->isFacingLeft());
	snoid->setNeedsRedraw(true);
}

// ---------------------------------------------------------------------------
// Dispatch SCRB-to-SCRS handoffs from the raft runner.
// This runs while a reject-flight controller SCRB 1604-1607 is loaded.
//
//   SCRB 1604, 1606, 1607: dispatch case 1
//   SCRB 1605: dispatch case 1, then case 2
// The later raft overlay SCRBs continue the same callback chain:
//   SCRB 1704: dispatch case 3
//   SCRB 1705: dispatch cases 6 and 4
// The feet-specific Snoid takeoff SCRS started by case 4 emits case 5.
//
// Case 1 loads SCRB 1700-1703 onto the rejected seat runner.
// It starts the primary takeoff SCRS and hides the Snoid on completion.
// Case 2 selects the raft path when the secondary SCRS is 1907.
// It then links the raft runners and sets @ref ZoombiniPuzzleFerry::_rejectRaftOverlayPending.
// Otherwise, it plays the secondary SCRS,
// aligns its end anchor to @ref ZoombiniPuzzleFerry::_rejectLandingPosition, returns the Snoid to idle, and clears the lock.
// Case 3 plays the same destination-aligned secondary SCRS as case 2 and hides the Snoid on completion.
// Case 4 places the Snoid at (93,408) and plays the feet-specific takeoff SCRS
// kResScrs0998_RaftTakeoffBase + 2*feet.
// Case 5 plays the paired landing SCRS kResScrs0999_RaftLandingBase + 2*feet,
// aligns it to the selected open pack slot, and clears the lock.
// Case 6 refills the pending captain animation from the non-repeating random fidget SCRB pool
// when no other captain SCRB is queued.
// ---------------------------------------------------------------------------
void ZoombiniPuzzleFerry::processRaftFlightEvent(int16 callbackCode) {
	int16 dest = _rejectDestinationCode;
	if (dest < 0 || 10 <= dest) {
		// Keep the lookup tables in bounds if a malformed destination reaches the dispatcher.
		dest = 0;
	}
	switch (callbackCode) {
	case kRaftEventCode001_StartRejectFlight: {
		// Replace the rejected seat's runner with its destination-specific SCRB 1700-1703.
		// This shows the rejection and plays its embedded SFX.
		if (_rejectedSeatFeature) {
			_rejectedSeatFeature->removeFlag(ZmbFeature::FLAG_00800000_POS_DELTA);
			loadScrbOntoFeature(_rejectedSeatFeature, kRejectDestinationSeatScrbIds[dest]);
			_rejectedSeatFeature->addFlag(ZmbFeature::FLAG_00800000_POS_DELTA);
			playCurrentFrameSound(_rejectedSeatFeature);
		}
		_rejectedSeatFeature = nullptr;

		if (!_rejectFlightSnoid)
			break;
		int16 scrsA = kRejectPrimarySnoidScrsIds[dest];
		// Hide on completion and use the SCRS first-frame anchor.
		// SCRS 1900, for example, starts the Snoid at (16, 235).
		//   - SCRS 1900-1906 belong to pool 0 -> NORMAL state 9
		_mainRejectSnoidCallbackActive = true;
		// Preserve the right-facing seat pose during the primary ejection.
		// The later lower-left raft-return leg performs its own turn.
		_rejectFlightSnoid->startScrsPlayback(ZmbResource(ZmbResource::kPage, scrsA), ZmbScrsCompletionMode::kHide, resolveScrsRejectState(scrsA));
		break;
	}
	case kRaftEventCode002_StartLandingOrRaftPath: {
		// Raft vs landing-SCRS branch.
		_mainRejectSnoidCallbackActive = false;
		int16 scrsB = kRejectSecondarySnoidScrsIds[dest];
		if (scrsB == kResScrs1907_RejectVariant7) {
			// Activate the reject-raft overlay instead of loading another Snoid SCRS.
			// @ref ZoombiniPuzzleFerry::startRejectWalk() handles raft-departure destinations.
			// It allocates the secondary 1705/1706 runner.
			// The Snoid remains hidden after the preceding reject-flight phase.
			if (_rejectFlightSnoid && _rejectRaftTakeoffFeature)
				manualLinkBefore(_rejectFlightSnoid, _rejectRaftTakeoffFeature);
			_rejectRaftOverlayPending = true;
			// Keep Snoid pickup input disabled until the paired landing SCRS starts in case 5.
			break;
		}
		if (!_rejectFlightSnoid)
			break;
		// Play SCRS 1901-1907 in the normal script state and return to idle on completion.
		// Align the end anchor to the destination point: dock at (122,164), or the saved origin for the Snoid-exit path.
		_rejectFlightSnoid->startScrsPlayback(ZmbResource(ZmbResource::kPage, scrsB), ZmbScrsCompletionMode::kReturnToIdle,
											  resolveScrsRejectState(scrsB), &_rejectLandingPosition);
		_rejectFlightSnoid->activateRender(); // case 1 may have hidden the snoid
		_rejectFlightSnoid->_packIsOccupied = false;
		_rejectFlightSnoid->_runnerStatus = kSnoidRunnerStatus00_Draggable;
		// Release input when the ordinary landing SCRS takes over the reject flight.
		setRunnerZSortEnabled(true);
		completeRejectRequest();
		break;
	}
	case kRaftEventCode003_StartHiddenLanding: {
		// Same SCRS pool as case 2 but hide on complete.
		_mainRejectSnoidCallbackActive = false;
		int16 scrsB = kRejectSecondarySnoidScrsIds[dest];
		if (!_rejectFlightSnoid)
			break;
		_rejectFlightSnoid->startScrsPlayback(ZmbResource(ZmbResource::kPage, scrsB), ZmbScrsCompletionMode::kHide,
											  resolveScrsRejectState(scrsB), &_rejectLandingPosition);
		_rejectFlightSnoid->activateRender(); // ensure visible during landing arc
		// The Snoid finishes hidden before the later raft overlay continues the hand-off.
		break;
	}
	case kRaftEventCode004_StartRaftTakeoff: {
		// Raft SCRB 1705 places the rejected Snoid at the lower-left approach
		// point and starts the feet-specific takeoff SCRS (1000,1002,...,1008).
		if (!_rejectFlightSnoid)
			break;
		const int16 scrsId = static_cast<int16>(kResScrs0998_RaftTakeoffBase + 2 * _rejectFlightSnoid->_trait._feet);
		_rejectFlightSnoid->setPointLoc(Common::Point(93, 408));
		_rejectFlightSnoid->setFacingLeft(true);
		_mainRejectSnoidCallbackActive = true;
		_rejectFlightSnoid->startScrsPlayback(ZmbResource(ZmbResource::kPage, scrsId), ZmbScrsCompletionMode::kReturnToIdle,
											  resolveScrsRejectState(scrsId));
		joinFeatureTimingGroup(_rejectRaftTakeoffFeature, _rejectFlightSnoid);
		_rejectFlightSnoid->activateRender();
		_rejectFlightSnoid->_runnerStatus = kSnoidRunnerStatus01_RejectReturning;
		setRunnerZSortEnabled(true);
		break;
	}
	case kRaftEventCode005_StartRaftLanding: {
		// The takeoff SCRS terminator feeds case 5 back into this dispatcher.
		// It starts the paired landing SCRS and aligns its end to the open slot.
		if (!_rejectFlightSnoid)
			break;
		const int16 scrsId = static_cast<int16>(kResScrs0999_RaftLandingBase + 2 * _rejectFlightSnoid->_trait._feet);
		_rejectLandingPosition = _rejectReturnPosition;
		_rejectFlightSnoid->setFacingLeft(false);
		_mainRejectSnoidCallbackActive = false;
		_rejectFlightSnoid->startScrsPlayback(ZmbResource(ZmbResource::kPage, scrsId), ZmbScrsCompletionMode::kReturnToIdle,
											  resolveScrsRejectState(scrsId), &_rejectLandingPosition);
		_rejectFlightSnoid->activateRender();
		_rejectFlightSnoid->_packIsOccupied = false;
		_rejectFlightSnoid->_runnerStatus = kSnoidRunnerStatus00_Draggable;
		completeRejectRequest();
		break;
	}
	case kRaftEventCode006_QueueCaptainFidget: {
		// Use its distinct post-flight fidget pool 1828-1832.
		if (_pendingCaptainScrbId == 0) {
			uint16 fidgetPoolIdx = _vm->_rnd->getNonRepeatRandom(5, _vm->_state->getFerryRuntimeState()._postFlightFidgetPoolState);
			_pendingCaptainScrbId = kCaptainPostFlightFidgetScrbPool[fidgetPoolIdx];
		}
		break;
	}
	default:
		break;
	}
}

} // End of namespace Mohawk
