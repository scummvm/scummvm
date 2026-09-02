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

#include "mohawk/zoombini_pages/puzzle_caves.h"
#include "mohawk/graphics.h"
#include "mohawk/zoombini.h"
#include "mohawk/zoombini_graphics.h"
#include "mohawk/zoombini_metaengine.h"
#include "mohawk/zoombini_random.h"
#include "mohawk/zoombini_sound.h"
#include "mohawk/zoombini_state.h"

namespace Mohawk {

constexpr Common::Point ZoombiniPuzzleCaves::kWaitingSnoidPositions[20];
constexpr Common::Point ZoombiniPuzzleCaves::kSeatEntrancePositions[20];
constexpr int16 ZoombiniPuzzleCaves::kRuleGlyphScreenX[11];
constexpr int16 ZoombiniPuzzleCaves::kRuleGlyphScreenY[11];
constexpr bool ZoombiniPuzzleCaves::kSeatFacingLeft[20];

ZoombiniPuzzleCaves::ZoombiniPuzzleCaves(MohawkEngine_Zoombini *vm) : ZoombiniPuzzle(vm, ZoombiniPageType::kCaves, ZmbSrcPageKind::kCaves_14) {
}

ZoombiniPuzzleCaves::~ZoombiniPuzzleCaves() {
}

void ZoombiniPuzzleCaves::initStates() {
	_pageLoadedZmbCount = 0;
	_firstUsableSeatNumber = 1;
	_maskedLeadingSeatCount = 0;

	// Select the difficulty-dependent resources before feature materialization.
	// Rule generation follows the active-pack runners in @ref loadFeatures().
	initDifficultyParams();

	_acceptedSnoidCount = 0;
	_acceptedWalkStackSize = 0;
	for (WalkInEntry &entry : _acceptedWalkStack)
		entry = WalkInEntry();
	memset(_seatOccupants, 0, sizeof(_seatOccupants));
	_completionWalkInStartedCount = 0;
	_completionWalkInTargetCount = 0;
	_completionWalkLastStartFrame = 0;
	_completionWalkPoolState = 0;
	_hasEntranceScriptTarget = false;

	_seatTransferScrsBaseId = kResScrs12004_RejectFirstPlayable;
	_seatTransferScrbBaseId = kResScrb8200_GlyphDoorBase;
	_mistakeCount = 0;
	_activeTransferOverlayFeature = nullptr;
	_droppedSeatOverlayFeature = nullptr;
	_matchingSeatOverlayFeature = nullptr;
	_matchingSeatTransferPending = false;
	_snoidDragEnabled = true;
	_seatTransferFinalizePending = false;
	_departurePhase = DeparturePhase::kIdle00;
	_activeTransferSnoid = nullptr;
	_droppedSeatNumber = 0;
	_matchingSeatNumber = 0;
	_wrongSeatTransferPending = false;
	_completionWalkInsActive = false;
	_goLionSequencePending = false;
	_goButtonEnabled = false;
	_departureState = ZmbDepartureState::kIdle;
}

const ZoombiniPage::ScriptSoundPriorityRanges &ZoombiniPuzzleCaves::getScriptSoundPriorityRanges() const {
	// Original shared page dispatch automatically prepends SND 996-997 at priority 32.
	static const ZoombiniPage::ScriptSoundPriorityRanges kRanges = {
		{kSysResSoundRange0996_DepartSFX, kSysResSoundRange0997_ArriveSFX},
		// Caves then registers the same range again in its authored list.
		{kSysResSoundRange0996_DepartSFX, kSysResSoundRange0997_ArriveSFX},
		{kSysResSoundRange20000_Base, kSysResSoundRange29999_Last},
		{kResSoundRange6006_Priority, kResSoundRange6006_Priority},
		{kResSoundRange6001_Priority, kResSoundRange6001_Priority},
		{kResSoundRange6008_Priority, kResSoundRange6008_Priority},
		{kResSoundRange6000_Priority, kResSoundRange6000_Priority},
		{kResSoundRange6005_Priority, kResSoundRange6005_Priority},
		{kResSoundRange6004_Priority, kResSoundRange6004_Priority},
		{kResSoundRange6003_Priority, kResSoundRange6003_Priority},
		{kResSoundRange6007_Priority, kResSoundRange6007_Priority},
		{kResSoundRange6002_Priority, kResSoundRange6002_Priority},
		{kResSoundRange8200_PageRangeBase, kResSoundRange12001_PageRangeLast},
		{kResSoundRange0425_PageRangeBase, kResSoundRange0499_PageRangeLast},
		{kResSoundRange0600_PageRangeBase, kResSoundRange0799_PageRangeLast},
	};
	return kRanges;
}

Audio::Mixer::SoundType ZoombiniPuzzleCaves::getFeatureSoundType(const ZmbFeature *feature, ZmbResource sndRes) const {
	if (feature && feature == _lionEntranceFeatures[2] && sndRes._archiveKind == ZmbResource::kPage && sndRes._id == kResSound6006_LionGrowl)
		return Audio::Mixer::kSpeechSoundType;
	return ZoombiniPuzzle::getFeatureSoundType(feature, sndRes);
}

void ZoombiniPuzzleCaves::open() {
	// MIDI BGM (tMID 30025-30028) - Broderbund v1.x only.
	if (!_vm->isVersionFamilyTlcV2())
		openMidiArchive();
	openArchive(ZMB_MHK_CAVES);
}

void ZoombiniPuzzleCaves::setBackgroundMusic() {
	// In original engine, only L1-L3 plays MIDI BGM. L4 remains silent.
	// However, the MIDI .MHK archives has a song for a L4, too.
	// This ScummVM option deliberately restores the shipped silent level 4 behavior when disabled.
	if (!_vm->isVersionFamilyTlcV2()) { // TLC v2.0 has no MIDI resources.
		const int16 routeLevel = _difficultyLevel - kPuzzleLevel1;
		if (routeLevel < 3 || ConfMan.getBool(MohawkMetaEngine_Zoombini::kOptionFixCavesL4MidiSilentBug))
			_vm->_midi->playZmbMidi(ZmbResource(ZmbResource::kPage, static_cast<int16>(kResMidi30025_CavesBgmBase + routeLevel)));
	}
}

void ZoombiniPuzzleCaves::setBackgroundBitmap() {
	_vm->_gfx->setPalette(kResBackground5000);
	_vm->_gfx->drawBackground(kResBackground5000);
}

void ZoombiniPuzzleCaves::loadFeatures() {
	// Preload page shape images.
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape11000_Snoid));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape6000_Entrance));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape7000_Doors));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape8200_GlyphPanel));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape9000_Overlays));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape9025_SeatMask));

	// Load hieroglyph sprites from tBMP 10000.
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape10000_Glyphs));

	// Load per-slot glyph adjustments from big-endian REGS 201.
	// REGS 200/201/202 are single-axis adjustment vectors, not a paired registration form:
	// twenty plain int16 values with no header.
	// This page retains source entries 0-10; index 0 is unused and indices 1-10 correspond to one-based clue slots 1-10.
	{
		Common::SeekableReadStream *regsStream = _vm->getResource(ID_REGS, ZmbResource(ZmbResource::kPage, kResRegs0201_GlyphAdjustments));
		if (ZmbResource::hasSize(regsStream, 20 * sizeof(int16), 20 * sizeof(int16))) {
			for (int i = 0; i < 11; i++)
				_ruleGlyphYAdjustments[i] = regsStream->readSint16BE();
		} else {
			delete regsStream;
			error("caves: init: malformed required REGS 201 resource");
			return;
		}
		delete regsStream;
	}

	// Load terrain barrier bitmap (tBMP 100)
	loadTerrainBitmap(kResBitmapTerrain100);

	// Load NODE/PATH for walk network
	loadNodePath(ZmbResource(ZmbResource::kPage, kResNode1000_WalkNetwork));

	// Load feature groups
	// Entrance animations
	// Overlays
	// Door animations
	// Glyph panels

	// Load main features: 13 entrance SCRBs at 6000
	ZmbFeature *mainFeature = createPuzzleMainFeatureHead();

	// Load sub-features chained from main
	{
		ZmbFeature *parent = mainFeature;
		// Zero-based resource offsets 0-19 correspond to one-based seats 1-20.
		for (uint16 resourceOffset = 0; resourceOffset < 20; resourceOffset++) {
			parent = loadSubFeature(parent, ZmbResource(ZmbResource::kPage, kResBitmapShape9000_Overlays), kResScrb9000_OverlayBase + resourceOffset);
		}
	}

	{
		ZmbFeature *parent = mainFeature;
		// Zero-based resource offsets 0-19 correspond to one-based seats 1-20.
		for (uint16 resourceOffset = 0; resourceOffset < 20; resourceOffset++) {
			parent = loadSubFeature(parent, ZmbResource(ZmbResource::kPage, kResBitmapShape7000_Doors), kResScrb7000_DoorBase + resourceOffset);
		}
	}

	{
		ZmbFeature *parent = mainFeature;
		// Zero-based offsets 0-79 contain four transfer resources for each one-based seat 1-20.
		for (uint16 resourceOffset = 0; resourceOffset < 80; resourceOffset++) {
			parent = loadSubFeature(parent, ZmbResource(ZmbResource::kPage, kResBitmapShape8200_GlyphPanel), kResScrb8200_GlyphDoorBase + resourceOffset);
		}
	}

	{
		ZmbFeature *parent = mainFeature;
		// Zero-based offsets 0-3 select masks for one through four leading seats.
		for (uint16 resourceOffset = 0; resourceOffset < 4; resourceOffset++) {
			parent = loadSubFeature(parent, ZmbResource(ZmbResource::kPage, kResBitmapShape9025_SeatMask), kResScrb9025_SeatMaskBase + resourceOffset);
		}
	}

	// Load reject pool: 14 reject scripts at SCRS 12000
	// Group 0 -> state 9 (NORMAL).
	registerScrsGroup(kResScrs12000_RejectBase, 14);

	// Load normal pool: 5 normal scripts at SCRS 13000
	// Group 1 -> state 8 (REJECT).
	registerScrsGroup(kResScrs13000_NormalBase, 5);

	// Entrance animation SCRB 6000, interval=6
	_lionEntranceFeatures[0] = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape6000_Entrance), kResScrb6000_EntranceBase, 6,
											   ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_04000000_OVERLAY);

	// Entrance animation SCRB 6001, interval=6
	_lionEntranceFeatures[1] = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape6000_Entrance), kResScrb6001_EntranceSecond, 6,
											   ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_04000000_OVERLAY);

	// Entrance animation SCRB 6002, interval=8
	_lionEntranceFeatures[2] = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape6000_Entrance), kResScrb6002_EntranceFinal, 8,
											   ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM |
												   ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_04000000_OVERLAY);

	// The authored page materializes the active pack before deriving its rule buckets.
	// This ordering makes both trait collection and the later runner links observe the
	// same feature graph as the original controller.
	assert(!_vm->_state->_f._zmbPackActive.getSkipOccupiedEntries());
	loadZoombinisFromPack(kWaitingSnoidPositions, ARRAYSIZE(kWaitingSnoidPositions));
	assert(0 < _pageLoadedZmbCount && _pageLoadedZmbCount <= 16);

	// SCRB 9025-9028 covers at most four leading background seats.
	_firstUsableSeatNumber = CLIP<int16>(21 - _pageLoadedZmbCount, 1, 20);
	_maskedLeadingSeatCount = CLIP<int16>(20 - _pageLoadedZmbCount, 0, 4);
	_completionWalkInTargetCount = MAX<int16>(0, _pageLoadedZmbCount - 1);
	debugC(3, MohawkEngine_Zoombini::kDebugPage02, "caves: loaded %d zoombinis; usable seats are %d through 20", _pageLoadedZmbCount, _firstUsableSeatNumber);

	if (0 < _maskedLeadingSeatCount) {
		const int16 maskScrbId = static_cast<int16>(kResScrb9024_SeatMaskBase + _maskedLeadingSeatCount);
		_leadingSeatMaskFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape9025_SeatMask), maskScrbId, 6,
												  ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_04000000_OVERLAY);
	}

	// SCRB 7000-7003 use zero-based array entries 0-3 for one-based seats 1-4.
	for (uint16 seatArrayIdx = 0; seatArrayIdx < 4; seatArrayIdx++) {
		_seatDoorFeatures[seatArrayIdx] = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape7000_Doors), kResScrb7000_DoorBase + seatArrayIdx, 7,
														  kSeatEntrancePositions[seatArrayIdx],
														  ZmbFeature::FLAG_00002000_DRAW_ON_REG | ZmbFeature::FLAG_00008000_LOOP_ANIM |
															  ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_01000000_DEFER_RENDER |
															  ZmbFeature::FLAG_04000000_OVERLAY);
	}

	// The fixed on-screen Z-order uses one-based seat numbers 5-11, 15-12, and 16-20, in that order.
	// These map to zero-based feature and DRAW_ON_REG indices 4-19.
	// The current pack contains at most sixteen Snoids, so one-based seats 1-4 are never usable and their zero-based slots 0-3 are masked.

	// SCRB 7004-7010 DRAW_ON_REG + glyph overlays SCRB 9004-9010, each pair separated by the
	// seat's no-op Z-order anchor. Seats 5-11.
	for (uint16 overlayArrayIdx = 0; overlayArrayIdx < 7; overlayArrayIdx++) {
		_upperSeatOverlayFeatures[overlayArrayIdx] = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape9000_Overlays),
																	 kResScrb9004_GlyphOverlayBase + overlayArrayIdx, 6,
																	 ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE |
																		 ZmbFeature::FLAG_04000000_OVERLAY);

		registerSeatZOrderAnchor(static_cast<int16>(5 + overlayArrayIdx));

		_seatDoorFeatures[4 + overlayArrayIdx] = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape7000_Doors),
																 kResScrb7004_DoorGlyphBase + overlayArrayIdx, 7, kSeatEntrancePositions[4 + overlayArrayIdx],
																 ZmbFeature::FLAG_00002000_DRAW_ON_REG | ZmbFeature::FLAG_00008000_LOOP_ANIM |
																	 ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_01000000_DEFER_RENDER |
																	 ZmbFeature::FLAG_04000000_OVERLAY);
	}

	// One-based seats 12-15 use zero-based overlay entries 0-3 but are registered in reverse seat order.
	for (uint16 reverseOffset = 0; reverseOffset < 4; reverseOffset++) {
		_middleSeatOverlayFeatures[3 - reverseOffset] = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape9000_Overlays),
																		kResScrb9014_DoorPanelBase - reverseOffset, 6,
																		ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE |
																			ZmbFeature::FLAG_04000000_OVERLAY);

		registerSeatZOrderAnchor(static_cast<int16>(15 - reverseOffset));
	}

	// Register one-based seats 12-15 at zero-based door and position entries 11-14.
	for (uint16 seatGroupOffset = 0; seatGroupOffset < 4; seatGroupOffset++) {
		_seatDoorFeatures[11 + seatGroupOffset] = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape7000_Doors),
																  kResScrb7011_DoorGlyphBase + seatGroupOffset, 7, kSeatEntrancePositions[11 + seatGroupOffset],
																  ZmbFeature::FLAG_00002000_DRAW_ON_REG | ZmbFeature::FLAG_00008000_LOOP_ANIM |
																	  ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_01000000_DEFER_RENDER |
																	  ZmbFeature::FLAG_04000000_OVERLAY);
	}

	// Keep each central DRAW_ON_REG runner in its reverse-registered seat group.
	// The occupant is linked between the anchor and this runner, so the seat's normal and glow
	// shapes keep the same local depth without crossing another seat group.
	for (int16 seatNumber = 12; seatNumber <= 15; seatNumber++)
		manualLinkAfter(_seatDoorFeatures[seatNumber - 1], _seatZOrderAnchors[seatNumber]);

	// SCRB 7015-7019 DRAW_ON_REG + glyph overlays SCRB 9015-9019, anchored the same way. Seats 16-20.
	for (uint16 overlayArrayIdx = 0; overlayArrayIdx < 5; overlayArrayIdx++) {
		_lowerSeatOverlayFeatures[overlayArrayIdx] = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape9000_Overlays),
																	 kResScrb9015_GlyphOverlayBase + overlayArrayIdx, 6,
																	 ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE |
																		 ZmbFeature::FLAG_04000000_OVERLAY);

		registerSeatZOrderAnchor(static_cast<int16>(16 + overlayArrayIdx));

		_seatDoorFeatures[15 + overlayArrayIdx] = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape7000_Doors),
																  kResScrb7015_DoorGlyphBase + overlayArrayIdx, 7, kSeatEntrancePositions[15 + overlayArrayIdx],
																  ZmbFeature::FLAG_00002000_DRAW_ON_REG | ZmbFeature::FLAG_00008000_LOOP_ANIM |
																	  ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_01000000_DEFER_RENDER |
																	  ZmbFeature::FLAG_04000000_OVERLAY);
	}

	// One trailing anchor closes the seat group, after the last DRAW_ON_REG door.
	registerSeatZOrderAnchor(21);

	// Copy the mask runner index into the leading DRAW_ON_REG occupancy entries.
	// This both hides and disables those non-gameplay seats.
	if (_leadingSeatMaskFeature) {
		for (int16 drawSlotIndex = 0; drawSlotIndex < _maskedLeadingSeatCount; drawSlotIndex++)
			setDrawOnRegOccupant(drawSlotIndex, _leadingSeatMaskFeature->getId());
	}

	// Generate the rule after the materialized Zoombini runners and all seat runners
	// exist, but before the rule-glyph runners consume the generated tables.
	setupEntranceGlyphs();

	// SCRB 6012 draws the full set of rule-matching entrance glyphs as an overlay.
	// @ref ZoombiniPuzzleCaves::setupEntranceGlyphs() prepares its glyph data.
	loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape6000_Entrance), kResScrb6012_GlyphPanelOverlay, 0,
					ZmbFeature::FLAG_04000000_OVERLAY);

	_ledgeControllerFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape6000_Entrance), _ledgeControllerScrbBaseId + 1, 9,
											  ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_08000000_REGION_TRACK);
	_ledgeControllerFeature->setScriptSoundPolicy(ZmbFeature::ScriptSoundPolicy::kPriorityQueue);

	// Create the virtual glyph renderer with a custom render callback.
	// This runner owns no SCRB data: the render callback draws every visible rule glyph.
	// It carries LOOP_ANIM only, so it stays in the loop-animation bucket without
	// being promoted into the overlay bucket.
	{
		ZmbFeature::EventHooks hooks;
		hooks.setRenderFunc(static_cast<ZmbFeature::OnRenderFunc>(&ZoombiniPuzzleCaves::renderEntranceGlyphs));
		_ruleGlyphRendererFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape6000_Entrance), 0, 0, ZmbFeature::FLAG_00008000_LOOP_ANIM, hooks);
	}

	// The authored controller commits one initial frame before assigning the staggered walk-in delays.
	layoutStaticAndWalkIn(0, false);
	// Submit the entrance frame-zero sounds to one priority pass.
	// Multiple entrance scripts have a candidate, but only the highest-priority sound plays.
	for (ZmbFeature *feature : _lionEntranceFeatures) {
		const int32 frameIndex = feature->getLastFrameIdx(); // Zero-based SCRB animation frame index.
		if (feature->enqueueFrameSound(frameIndex))
			feature->setLastSoundedFrameIdx(frameIndex);
	}
	renderFeatures();
	assignStaggeredWalkDelays(30, 45);

	// Set up Go/Map/Help buttons
	configureStandardPuzzleControlRects();
	loadStandardPuzzleControlFeatures(kResBitmapShape11000_Snoid);

	// Clear the occupancy inherited from the previous puzzle.
	// Correct entrance placements set it again for the continuing group.
	schedulePackSnoids(false, false);
}

void ZoombiniPuzzleCaves::initHelpPrompt() {
	// Level 4 retains this resource but suppresses its F1 replay in @ref ZoombiniPuzzleCaves::debugReplayActiveHelpSound().
	_activeHelpSoundId = ZmbResource(ZmbResource::kSystem, kSysResSound20065_CavesHelp);
}

void ZoombiniPuzzleCaves::onGoButtonActivated() {
	// After the synchronous button animation, Go starts the lion sequence and immediately arms route 17.
	// The lion SCRB still reaches event 10 before an unskipped departure starts SND 996.
	if (!_goButtonEnabled || !_pageActive || _departurePhase != DeparturePhase::kIdle00)
		return;
	_goLionSequencePending = true;
	_snoidDragEnabled = false;
	_departureState = ZmbDepartureState::kTriggered;
}

void ZoombiniPuzzleCaves::debugPrepareForDeparture() {
	if (!_pageActive)
		return;

	// Rebuild the seat list so every runner follows the same entrance and
	// success sequence as a fully solved Lion's Lair puzzle. Existing partial
	// placements are intentionally replaced because the debug command treats
	// the whole group as accepted.
	memset(_seatOccupants, 0, sizeof(_seatOccupants));
	_acceptedSnoidCount = 0;
	_acceptedWalkStackSize = 0;
	for (WalkInEntry &entry : _acceptedWalkStack)
		entry = WalkInEntry();
	_goButtonEnabled = false;
	_goLionSequencePending = false;
	_completionWalkInsActive = false;
	_completionWalkInStartedCount = 0;
	_completionWalkInTargetCount = 0;
	_completionWalkLastStartFrame = 0;
	_completionWalkPoolState = 0;
	_snoidDragEnabled = true;
	_departurePhase = DeparturePhase::kIdle00;
	_departureState = ZmbDepartureState::kIdle;
	_activeTransferSnoid = nullptr;

	int16 seatNumber = _firstUsableSeatNumber;
	for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
		ZmbSnoid *snoid = *it;
		if (!snoid || !snoid->isPackSnoid())
			continue;
		// The authored seat domain ends at one-based seat 20.
		// Current active groups contain at most sixteen Snoids and begin at @ref ZoombiniPuzzleCaves::_firstUsableSeatNumber.
		if (20 < seatNumber)
			continue;

		snoid->activateRender();
		handleCorrectPlacement(snoid, seatNumber, true);
		seatNumber += 1;
	}
}

ZmbChanceInfo ZoombiniPuzzleCaves::debugGetChances() const {
	// Mistake budget: each wrong lion-seat placement consumes one chance.
	// @ref ZoombiniPuzzleCaves::_mistakeLimit is 4-7 by level,
	// while @ref ZoombiniPuzzleCaves::_mistakeCount is the running tally.
	return {ZmbChanceInfo::ZmbChanceType::kMistake, _mistakeLimit, _mistakeCount, "incorrect seat placement"};
}

bool ZoombiniPuzzleCaves::debugSetChances(int16 remaining) {
	if (remaining < 0 || _mistakeLimit < remaining)
		return false;

	_mistakeCount = _mistakeLimit - remaining;
	loadGlyphPanelFrame(_mistakeCount, true);
	return true;
}

Common::String ZoombiniPuzzleCaves::debugGetAnswer() const {
	// Format a compact glyph offset for debug answer output.
	const auto decodeOffset = [](byte offset) -> Common::String {
		if (offset < 1 || 20 < offset)
			return "?";
		const int traitKindIndex = (offset - 1) / 5; // Zero-based TraitKind index 0-3.
		const int value = (offset - 1) % 5 + 1;
		const ZmbTrait::TraitKind kind = ZmbTrait::traitKindFromIndex(traitKindIndex);
		return Common::String::format("%s=%d(%s)", ZmbTrait::debugTraitKindName(kind), value, ZmbTrait::debugTraitValueName(kind, value));
	};

	const ZmbTrait::TraitKind primaryKind = _primaryRuleTraitKind;
	const ZmbTrait::TraitKind secondaryKind = _secondaryRuleTraitKind;
	Common::String s = getDebugBanner();
	s += "\n";
	s += Common::String::format("  Maximum mistakes: %d\n", _mistakeLimit);
	s += "  Sorting order:\n";
	s += Common::String::format("    Primary: %s\n", ZmbTrait::debugTraitKindName(primaryKind));
	if (1 < _ruleTraitCount) {
		s += Common::String::format("    Secondary: %s\n", ZmbTrait::debugTraitKindName(secondaryKind));
	}

	s += "  Visible wall clues:\n";
	bool hasVisibleClue = false;
	for (int16 clueSlot = 1; clueSlot < 11; clueSlot++) {
		if (!_ruleGlyphVisibility[clueSlot])
			continue;
		s += Common::String::format("    Clue %d: %s\n", clueSlot, decodeOffset(_ruleGlyphShapeIds[clueSlot]).c_str());
		hasVisibleClue = true;
	}
	if (!hasVisibleClue)
		s += "    (none at this level)\n";
	s += Common::String::format("  Internal seat numbers: one-based 1-20; current usable range %d-20.\n", _firstUsableSeatNumber);
	s += "  Displayed seat labels subtract 4: label 1 is left and label 16 is right for a full group.\n";
	s += "  Ordered seat groups for playable seats:\n";

	Common::Array<ZmbSnoid *> snoids;
	collectPackSnoids(snoids, false);

	for (int16 firstSeatNumber = _firstUsableSeatNumber; firstSeatNumber < 21;) {
		const int16 primaryValue = _seatPrimaryRuleValues[firstSeatNumber];
		const int16 secondaryValue = _seatSecondaryRuleValues[firstSeatNumber];
		if (primaryValue < 1) {
			firstSeatNumber += 1;
			continue;
		}

		int16 lastSeatNumber = firstSeatNumber;
		while (lastSeatNumber + 1 < 21 &&
			   _seatPrimaryRuleValues[lastSeatNumber + 1] == primaryValue &&
			   (_ruleTraitCount <= 1 || _seatSecondaryRuleValues[lastSeatNumber + 1] == secondaryValue)) {
			lastSeatNumber += 1;
		}

		const int16 firstDisplayedSeat = firstSeatNumber - 4;
		const int16 lastDisplayedSeat = lastSeatNumber - 4;
		if (firstSeatNumber == lastSeatNumber)
			s += Common::String::format("    Seat %d: ", firstDisplayedSeat);
		else
			s += Common::String::format("    Seats %d-%d: ", firstDisplayedSeat, lastDisplayedSeat);
		s += Common::String::format("(%s) %s", ZmbTrait::debugTraitKindName(primaryKind), ZmbTrait::debugTraitValueName(primaryKind, primaryValue));
		if (1 < _ruleTraitCount) {
			s += Common::String::format(", (%s) %s", ZmbTrait::debugTraitKindName(secondaryKind), ZmbTrait::debugTraitValueName(secondaryKind, secondaryValue));
		}
		s += "\n";

		s += "      Matching Zoombinis:\n";
		bool any = false;
		for (const ZmbSnoid *snoid : snoids) {
			const ZmbTrait &traits = snoid->_trait;
			const bool primaryMatch = traits.getTraitValue(primaryKind) == primaryValue;
			const bool secondaryMatch = _ruleTraitCount <= 1 ||
										traits.getTraitValue(secondaryKind) == secondaryValue;
			if (primaryMatch && secondaryMatch) {
				s += Common::String::format("        %s\n", snoid->toStr().c_str());
				any = true;
			}
		}
		if (!any)
			s += "        (none)\n";
		firstSeatNumber = lastSeatNumber + 1;
	}
	return s;
}

Common::String ZoombiniPuzzleCaves::debugGetBuiltinDebugCommandHelp() const {
	Common::String output;
	output += Common::String::format("  %-7s (%s <1-4>)\n", "1-4", kBuiltinDebugActionDifficulty);
	output += "    Set the runtime level, rebuild its visible hints, and reassign the current Zoombinis for the retained trait categories.\n";
	output += "    This does not reset the controller or mistake count; use Space afterward when that reset is wanted.\n";
	output += Common::String::format("  %-7s (%s)\n", "Space", kBuiltinDebugActionReinit);
	output += "    Reset the level-specific controller, mistake budget, and glyph-panel state while retaining the rule and seat occupancy.\n";
	output += Common::String::format("  %-7s (%s)\n", "F3", kBuiltinDebugActionReinitAfterExit);
	output += "    Return every Zoombini to its waiting position, clear all playable seats and Advance state, then perform the Space reset.\n";
	output += Common::String::format("  %-7s (%s)\n", "Shift+L", kBuiltinDebugActionLevel);
	output += "    Show the one-based current runtime level without changing it.\n";
	output += Common::String::format("  %-7s (%s <0-3>)\n", "F5-F8", kBuiltinDebugActionEntrance);
	output += "    Select Hair, Eyes, Nose, or Feet as the primary rule category, then rebuild hints and reassign current Zoombinis.\n";
	output += "    This does not reset the controller or mistake count.\n";
	return output;
}

bool ZoombiniPuzzleCaves::debugReplayActiveHelpSound() {
	if (_difficultyLevel == kPuzzleLevel4)
		return false;
	return ZoombiniInteractive::debugReplayActiveHelpSound();
}

void ZoombiniPuzzleCaves::debugReinitDifficulty() {
	_glyphBlinkHiddenSlot = 0;
	_glyphBlinkSequenceIndex = 0;
	_nextGlyphBlinkFrame = 0;
	switch (_difficultyLevel) {
	case kPuzzleLevel1:
		_mistakeLimit = 4;
		_ledgeControllerScrbBaseId = kResScrb6006_GlyphPanelLevel4;
		break;
	case kPuzzleLevel2:
		_mistakeLimit = 5;
		_ledgeControllerScrbBaseId = kResScrb6005_GlyphPanelLevel3;
		break;
	case kPuzzleLevel3:
		_mistakeLimit = 6;
		_ledgeControllerScrbBaseId = kResScrb6004_GlyphPanelLevel2;
		break;
	case kPuzzleLevel4:
	default:
		_mistakeLimit = 7;
		_ledgeControllerScrbBaseId = kResScrb6003_GlyphPanelBase;
		break;
	}
	_acceptedWalkStackSize = 0;
	_departurePhase = DeparturePhase::kIdle00;
	_acceptedSnoidCount = 0;
	_droppedSeatNumber = 0;
	_matchingSeatNumber = 0;
	_seatTransferFinalizePending = false;
	_mistakeCount = 0;
	_completionWalkInsActive = false;
	_wrongSeatTransferPending = false;
	_matchingSeatTransferPending = false;
	_goLionSequencePending = false;
	_snoidDragEnabled = true;
	_droppedSeatOverlayFeature = nullptr;
	_matchingSeatOverlayFeature = nullptr;
	_activeTransferSnoid = nullptr;
	_hasEntranceScriptTarget = false;
	_departureState = ZmbDepartureState::kIdle;

	if (_ledgeControllerFeature && !_ledgeControllerFeature->isRenderActivated()) {
		const int16 scrbId = MAX<int16>(kResScrb6003_GlyphPanelBase, static_cast<int16>(_ledgeControllerScrbBaseId - 1));
		loadScrbOntoFeature(_ledgeControllerFeature, scrbId, true);
	}
	scheduleForceRedraw();
}

void ZoombiniPuzzleCaves::debugReinitAfterExit() {
	Common::Array<ZmbSnoid *> snoids;
	collectPackSnoids(snoids, true);
	// Zero-based Snoid entries 0 through N - 1 map to the first N waiting-position entries, where N <= 16.
	for (uint32 waitingPositionIdx = 0; waitingPositionIdx < snoids.size() && waitingPositionIdx < ARRAYSIZE(kWaitingSnoidPositions); waitingPositionIdx++) {
		ZmbSnoid *snoid = snoids[waitingPositionIdx];
		markFeatureVisualCoverageDirty(snoid, false);
		snoid->_packIsOccupied = false;
		snoid->clearPreparedRenderHotspots();
		snoid->setAnimState(kSnoidAnimState000_Idle, &kWaitingSnoidPositions[waitingPositionIdx]);
		prepareSnoidVisualCoverage(snoid, true);
	}

	memset(_seatOccupants, 0, sizeof(_seatOccupants));
	for (int16 seatNumber = _firstUsableSeatNumber; seatNumber < 21; seatNumber++)
		setDrawOnRegOccupant(seatNumber - 1, 0);
	if (_leadingSeatMaskFeature) {
		for (int16 drawSlotIndex = 0; drawSlotIndex < _maskedLeadingSeatCount; drawSlotIndex++)
			setDrawOnRegOccupant(drawSlotIndex, _leadingSeatMaskFeature->getId());
	}
	_goButtonEnabled = false;
	debugReinitDifficulty();
}

void ZoombiniPuzzleCaves::debugProcessEntrance(int16 traitKindIndex) {
	if (0 <= traitKindIndex) {
		_primaryRuleTraitKind = ZmbTrait::traitKindFromIndex(traitKindIndex);
		if (_primaryRuleTraitKind == _secondaryRuleTraitKind) {
			int16 secondaryKind = static_cast<int16>(_secondaryRuleTraitKind) + 1;
			if (3 < secondaryKind)
				secondaryKind = 0;
			_secondaryRuleTraitKind = ZmbTrait::traitKindFromIndex(secondaryKind);
		}
	}

	Common::Array<ZmbSnoid *> snoids;
	collectPackSnoids(snoids, true);
	memset(_seatOccupants, 0, sizeof(_seatOccupants));
	for (int16 seatNumber = _firstUsableSeatNumber; seatNumber < 21; seatNumber++)
		setDrawOnRegOccupant(seatNumber - 1, 0);

	countGlyphDistribution();
	buildGlyphTimingTable();
	distributeEntranceTraits();

	for (ZmbSnoid *snoid : snoids) {
		markFeatureVisualCoverageDirty(snoid, false);
		const int16 seatNumber = findMatchingSeatNumber(snoid->_trait, _firstUsableSeatNumber);
		if (seatNumber < _firstUsableSeatNumber || 20 < seatNumber)
			continue;
		const Common::Point target = kSeatEntrancePositions[seatNumber - 1];
		snoid->_packIsOccupied = false;
		snoid->clearPreparedRenderHotspots();
		snoid->setAnimState(kSnoidAnimState005_Drag, &target);
		prepareSnoidVisualCoverage(snoid, true);
		_seatOccupants[seatNumber] = snoid;
		setDrawOnRegOccupant(seatNumber - 1, snoid->getId());
		linkSnoidToSeatZOrder(snoid, seatNumber);
	}
	if (_leadingSeatMaskFeature) {
		for (int16 drawSlotIndex = 0; drawSlotIndex < _maskedLeadingSeatCount; drawSlotIndex++)
			setDrawOnRegOccupant(drawSlotIndex, _leadingSeatMaskFeature->getId());
	}
	scheduleForceRedraw();
}

bool ZoombiniPuzzleCaves::debugDoBuiltinDebugCommand(int argc, const char **argv, Common::String &output) {
	if (argc < 3) {
		output = Common::String::format("Usage: page builtin_debug <%s|%s|%s|%s|%s> ...\n",
										kBuiltinDebugActionDifficulty, kBuiltinDebugActionReinit, kBuiltinDebugActionLevel,
										kBuiltinDebugActionReinitAfterExit, kBuiltinDebugActionEntrance);
		return true;
	}

	const BuiltinDebugAction action = parseBuiltinDebugAction(argv[2]);
	if (action == BuiltinDebugAction::kInvalid) {
		output = Common::String::format("Unknown Lion's Lair debug action '%s'.\n", argv[2]);
		output += debugGetBuiltinDebugCommandHelp();
		return true;
	}

	int32 argument = -1;
	if (action == BuiltinDebugAction::kDifficulty) {
		int32 level = 0;
		if (argc != 4 || !ZmbResource::parseInt(argv[3], level) || level < 1 || 4 < level) {
			output = Common::String::format("Usage: page builtin_debug %s <1-4>\n", kBuiltinDebugActionDifficulty);
			return true;
		}
		argument = level;
	}
	if (action == BuiltinDebugAction::kReinit || action == BuiltinDebugAction::kReinitAfterExit) {
		if (argc != 3) {
			output = Common::String::format("Usage: page builtin_debug <%s|%s>\n", kBuiltinDebugActionReinit, kBuiltinDebugActionReinitAfterExit);
			return true;
		}
	}
	if (action == BuiltinDebugAction::kEntrance) {
		int32 traitKindIndex = -1;
		if (argc != 4 || !ZmbResource::parseInt(argv[3], traitKindIndex) || traitKindIndex < 0 || 3 < traitKindIndex) {
			output = Common::String::format("Usage: page builtin_debug %s <0-3>\n", kBuiltinDebugActionEntrance);
			return true;
		}
		argument = traitKindIndex;
	}
	return runBuiltinDebugAction(action, static_cast<int16>(argument), output);
}

ZoombiniPuzzleCaves::BuiltinDebugAction ZoombiniPuzzleCaves::parseBuiltinDebugAction(const Common::String &action) {
	if (action.equalsIgnoreCase(kBuiltinDebugActionDifficulty))
		return BuiltinDebugAction::kDifficulty;
	if (action.equalsIgnoreCase(kBuiltinDebugActionReinit))
		return BuiltinDebugAction::kReinit;
	if (action.equalsIgnoreCase(kBuiltinDebugActionReinitAfterExit))
		return BuiltinDebugAction::kReinitAfterExit;
	if (action.equalsIgnoreCase(kBuiltinDebugActionLevel))
		return BuiltinDebugAction::kLevel;
	if (action.equalsIgnoreCase(kBuiltinDebugActionEntrance))
		return BuiltinDebugAction::kEntrance;
	return BuiltinDebugAction::kInvalid;
}

bool ZoombiniPuzzleCaves::runBuiltinDebugAction(BuiltinDebugAction action, int16 argument, Common::String &output) {
	if (action == BuiltinDebugAction::kDifficulty) {
		_difficultyLevel = static_cast<ZmbPuzzleLevel>(argument);
		debugProcessEntrance(-1);
		output = Common::String::format("Lion's Lair runtime difficulty set to level %d.\n", argument);
		showBuiltinDebugText(Common::String::format("Level %d", argument));
		return false;
	}
	if (action == BuiltinDebugAction::kReinit || action == BuiltinDebugAction::kReinitAfterExit) {
		if (action == BuiltinDebugAction::kReinitAfterExit)
			debugReinitAfterExit();
		else
			debugReinitDifficulty();
		output = Common::String::format("Lion's Lair level %d controller reinitialized.\n", static_cast<int>(_difficultyLevel));
		return false;
	}
	if (action == BuiltinDebugAction::kLevel) {
		output = Common::String::format("Level %d\n", static_cast<int>(_difficultyLevel));
	} else if (action == BuiltinDebugAction::kEntrance) {
		debugProcessEntrance(argument);
		const ZmbTrait::TraitKind traitKind = ZmbTrait::traitKindFromIndex(argument);
		output = Common::String::format("Rebuilt entrance rules with %s as the primary trait.\n", ZmbTrait::debugTraitKindName(traitKind));
	} else {
		return true;
	}

	if (action == BuiltinDebugAction::kLevel)
		showBuiltinDebugText(Common::String::format("Level %d", static_cast<int>(_difficultyLevel)));
	return false;
}

ZmbEventHandleResult ZoombiniPuzzleCaves::onDebugKeyDown(const Common::KeyState &kbd) {
	if (kbd.keycode == Common::KEYCODE_F1) {
		debugReplayActiveHelpSound();
		return ZmbEventHandleResult::kConsumed;
	}

	BuiltinDebugAction action = BuiltinDebugAction::kInvalid;
	int16 argument = -1;
	if (kbd.hasFlags(0) || kbd.hasFlags(Common::KBD_SHIFT)) {
		if ('1' <= kbd.ascii && kbd.ascii <= '4') {
			action = BuiltinDebugAction::kDifficulty;
			argument = static_cast<int16>(kbd.ascii - '0');
		} else if (kbd.keycode == Common::KEYCODE_SPACE) {
			action = BuiltinDebugAction::kReinit;
		} else if (kbd.ascii == 'L') {
			action = BuiltinDebugAction::kLevel;
		} else if (kbd.keycode == Common::KEYCODE_F3) {
			action = BuiltinDebugAction::kReinitAfterExit;
		} else if (Common::KEYCODE_F5 <= kbd.keycode && kbd.keycode <= Common::KEYCODE_F8) {
			action = BuiltinDebugAction::kEntrance;
			argument = static_cast<int16>(kbd.keycode - Common::KEYCODE_F5);
		}
	}
	if (action == BuiltinDebugAction::kInvalid)
		return ZmbEventHandleResult::kPassthrough;

	Common::String output;
	runBuiltinDebugAction(action, argument, output);
	return ZmbEventHandleResult::kConsumed;
}

void ZoombiniPuzzleCaves::initDifficultyParams() {
	// Initialize difficulty parameters based on route level.
	// The level determines how many entrances are active and which SCRB panel to use.

	// Reset the two level-1 glyph-attention states.
	// The virtual renderer is newly allocated for each load,
	// so its next render deadline also starts ready to run the first phase immediately.
	_glyphBlinkHiddenSlot = 0;
	_glyphBlinkSequenceIndex = 0;
	_nextGlyphBlinkFrame = 0;

	// Clear the two rule-clue arrays.
	for (int i = 0; i < 11; i++) {
		_ruleGlyphVisibility[i] = 0;
		_ruleGlyphShapeIds[i] = 0;
	}

	// Map level (1-4) to allowed mistakes (4-7) and ledge/door SCRB base.
	switch (_difficultyLevel) {
	case kPuzzleLevel1:
		_mistakeLimit = 4;
		_ledgeControllerScrbBaseId = kResScrb6006_GlyphPanelLevel4;
		break;
	case kPuzzleLevel2:
		_mistakeLimit = 5;
		_ledgeControllerScrbBaseId = kResScrb6005_GlyphPanelLevel3;
		break;
	case kPuzzleLevel3:
		_mistakeLimit = 6;
		_ledgeControllerScrbBaseId = kResScrb6004_GlyphPanelLevel2;
		break;
	case kPuzzleLevel4:
	default:
		_mistakeLimit = 7;
		_ledgeControllerScrbBaseId = kResScrb6003_GlyphPanelBase;
		break;
	}
}

void ZoombiniPuzzleCaves::setupEntranceGlyphs() {
	// Call three helpers to set up the glyph pattern system:
	initEntranceTraitPattern();
	countGlyphDistribution();
	buildGlyphTimingTable();

	// Distribute traits to entrances
	distributeEntranceTraits();
}

void ZoombiniPuzzleCaves::initEntranceTraitPattern() {
	// Initialize random trait patterns with a Fisher-Yates shuffle.

	// Use one rule dimension at levels 1/2 and two at levels 3/4.
	_ruleTraitCount = (_difficultyLevel <= kPuzzleLevel2) ? 1 : 2;

	// Use five zero-based rule-value columns numbered 0-4.
	_ruleValueCount = 5;

	// Initialize both rule trait kinds before selecting distinct values below.
	_primaryRuleTraitKind = ZmbTrait::kTraitHair;
	_secondaryRuleTraitKind = ZmbTrait::kTraitHair;

	// Clear zero-based rule rows 0-1 and value indices 0-4.
	for (int ruleRowIdx = 0; ruleRowIdx < 2; ruleRowIdx++) {
		for (int ruleValueIdx = 0; ruleValueIdx < _ruleValueCount; ruleValueIdx++) {
			_ruleTraitValues[5 * ruleRowIdx + ruleValueIdx] = 0;
		}
	}

	// Trait-pool indices are zero-based 0-3; the fifth entry is removal scratch storage.
	ZmbTrait::TraitKind traitPool[5] = {};
	int16 traitPoolLastIdx = 3;

	for (int ruleRowIdx = 0; ruleRowIdx < 2; ruleRowIdx++) {
		// Selectable trait values occupy one-based pool entries 1-5.
		// Entry 0 is not selected and entry 6 is the removal-shift sentinel.
		int16 traitValuePool[7];
		for (int poolArrayIdx = 0; poolArrayIdx < 7; poolArrayIdx++) {
			traitValuePool[poolArrayIdx] = poolArrayIdx;
		}

		// Reset the trait-kind pool for zero-based rule row 0.
		if (ruleRowIdx == 0) {
			for (int traitIdx = 0; traitIdx < 4; traitIdx++) {
				traitPool[traitIdx] = ZmbTrait::traitKindFromIndex(traitIdx);
			}
			traitPoolLastIdx = 3;

			// Pick a zero-based trait-pool index in the range 0 through traitPoolLastIdx.
			int16 traitPoolIdx = _vm->_rnd->getRandomNumber(traitPoolLastIdx);
			if (_vm->_cavesFirstRuleTraitPending) {
				traitPoolIdx = 2;
				_vm->_cavesFirstRuleTraitPending = false;
			}
			_primaryRuleTraitKind = traitPool[traitPoolIdx];

			// Remove the selected trait while retaining zero-based indices for the remaining entries.
			for (int traitIdx = traitPoolIdx; traitIdx < traitPoolLastIdx + 1; traitIdx++) {
				traitPool[traitIdx] = traitPool[traitIdx + 1];
			}
			traitPoolLastIdx -= 1;
		} else {
			// Rule row 1 selects a zero-based index from the remaining trait-kind pool.
			const int16 traitPoolIdx = _vm->_rnd->getRandomNumber(traitPoolLastIdx);
			_secondaryRuleTraitKind = traitPool[traitPoolIdx];
		}

		// Fill zero-based rule-value entries 0-4 from one-based trait-value pool entries 1-5.
		int16 selectableTraitValueCount = 5;
		for (int ruleValueIdx = 0; ruleValueIdx < _ruleValueCount; ruleValueIdx++) {
			const int16 traitValuePoolIdx = _vm->_rnd->getRandomNumber(1, selectableTraitValueCount);
			_ruleTraitValues[5 * ruleRowIdx + ruleValueIdx] = traitValuePool[traitValuePoolIdx];

			// Remove the selected one-based pool entry; the next element may be sentinel entry 6.
			for (int poolArrayIdx = traitValuePoolIdx; poolArrayIdx < selectableTraitValueCount + 1; poolArrayIdx++)
				traitValuePool[poolArrayIdx] = traitValuePool[poolArrayIdx + 1];

			selectableTraitValueCount -= 1;
		}
	}
}

void ZoombiniPuzzleCaves::countGlyphDistribution() {
	// Counts glyph trait distribution across loaded Zoombinis.

	// Reassert the difficulty-selected rule dimension count.
	_ruleTraitCount = (_difficultyLevel <= kPuzzleLevel2) ? 1 : 2;

	// Clear zero-based bucket indices 0-35.
	for (int bucketIdx = 0; bucketIdx < 36; bucketIdx++) {
		_ruleBucketCounts[bucketIdx] = 0;
	}

	// Read the materialized runner traits. The active pack has already been
	// transferred into page-owned Snoids at this point.
	int16 countedSnoids = 0;
	for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
		const ZmbSnoid *snoid = *it;
		if (!snoid || !snoid->isPackSnoid())
			continue;

		const ZmbTrait &traits = snoid->_trait;

		// Trait values are one-based 1-5; row zero therefore uses bucket indices 1-5.
		const int16 primaryRuleValue = traits[_primaryRuleTraitKind];
		_ruleBucketCounts[primaryRuleValue] += 1;

		// For two-trait rules, combine one-based values 1-5 into zero-based table indices 7-35.
		if (1 < _ruleTraitCount) {
			const int16 secondaryRuleValue = traits[_secondaryRuleTraitKind];
			_ruleBucketCounts[6 * secondaryRuleValue + primaryRuleValue] += 1;
		}
		countedSnoids += 1;
	}
	assert(countedSnoids == _pageLoadedZmbCount);
}

void ZoombiniPuzzleCaves::buildGlyphTimingTable() {
	// Assign the counted rule values to the one-based usable seats.

	// Clear direct one-based seat entries 1-20 plus unused array index 0.
	for (int seatArrayIdx = 0; seatArrayIdx < 21; seatArrayIdx++) {
		_seatPrimaryRuleValues[seatArrayIdx] = 0;
		_seatSecondaryRuleValues[seatArrayIdx] = 0;
	}

	// Expand primary bucket counts into consecutive seat assignments.
	int16 seatNumber = 21 - _pageLoadedZmbCount;
	for (int ruleValueIdx = 0; ruleValueIdx < _ruleValueCount; ruleValueIdx++) {
		const int16 primaryRuleValue = _ruleTraitValues[ruleValueIdx];
		const int16 distributionCount = _ruleBucketCounts[primaryRuleValue];
		for (int distributionIdx = 0; distributionIdx < distributionCount && seatNumber < 21; distributionIdx++) {
			_seatPrimaryRuleValues[seatNumber] = primaryRuleValue;
			seatNumber += 1;
		}
	}

	// Expand the primary-secondary bucket counts for levels 3/4.
	if (1 < _ruleTraitCount) {
		seatNumber = 21 - _pageLoadedZmbCount;
		for (int primaryOrderIdx = 0; primaryOrderIdx < _ruleValueCount; primaryOrderIdx++) {
			for (int secondaryOrderIdx = 0; secondaryOrderIdx < _ruleValueCount; secondaryOrderIdx++) {
				const int16 secondaryRuleValue = _ruleTraitValues[5 + secondaryOrderIdx];
				const int16 primaryRuleValue = _ruleTraitValues[primaryOrderIdx];
				const int16 distributionCount = _ruleBucketCounts[6 * secondaryRuleValue + primaryRuleValue];
				for (int distributionIdx = 0; distributionIdx < distributionCount && seatNumber < 21; distributionIdx++) {
					if (secondaryRuleValue != 0)
						_seatSecondaryRuleValues[seatNumber] = secondaryRuleValue;
					seatNumber += 1;
				}
			}
		}
	}
}

void ZoombiniPuzzleCaves::distributeEntranceTraits() {
	// Distributes traits to cave entrances based on difficulty level.

	// One-based clue-slot candidates occupy pool entries 1-5.
	// Entry 0 is not selected and entry 6 is the removal-shift sentinel.
	int16 clueSlotPool[7];
	for (int poolArrayIdx = 0; poolArrayIdx < ARRAYSIZE(clueSlotPool); poolArrayIdx++)
		clueSlotPool[poolArrayIdx] = poolArrayIdx;

	// Clear one-based clue-slot entries 1-10 plus unused array index 0.
	for (int clueArrayIdx = 0; clueArrayIdx < 11; clueArrayIdx++) {
		_ruleGlyphVisibility[clueArrayIdx] = 0;
		_ruleGlyphShapeIds[clueArrayIdx] = 0;
	}

	switch (_difficultyLevel) {
	case kPuzzleLevel1: // Level 1: All first 5 entrances active
		for (int clueSlot = 1; clueSlot < 6; clueSlot++)
			_ruleGlyphVisibility[clueSlot] = 1;
		break;
	case kPuzzleLevel2: { // Level 2: Random selection from first 5 entrances
		// Preserve the authored zero-width RNG wrapper call. It does not advance
		// the core generator, but it is part of the page's observable call ledger.
		int16 numActive = _vm->_rnd->getRandomNumber(2, 2);
		int16 selectableClueCount = 5;
		for (int selectionIdx = 0; selectionIdx < numActive; selectionIdx++) {
			const int16 cluePoolIdx = _vm->_rnd->getRandomNumber(1, selectableClueCount);
			_ruleGlyphVisibility[clueSlotPool[cluePoolIdx]] = 1;
			// Remove the selected one-based pool entry; the next element may be sentinel entry 6.
			for (int poolArrayIdx = cluePoolIdx; poolArrayIdx < selectableClueCount + 1; poolArrayIdx++) {
				clueSlotPool[poolArrayIdx] = clueSlotPool[poolArrayIdx + 1];
			}
			selectableClueCount -= 1;
		}
		break;
	}
	case kPuzzleLevel3: { // Level 3: Random selection from one-based clue slots 1-5 and 6-10.
		for (int clueRowIdx = 0; clueRowIdx < 2; clueRowIdx++) {
			const int16 clueSlotOffset = (clueRowIdx == 0) ? 0 : 5;

			// Reset zero-based pool storage entries 0-6 before selecting one-based entries 1-5.
			for (int poolArrayIdx = 0; poolArrayIdx < 7; poolArrayIdx++) {
				clueSlotPool[poolArrayIdx] = poolArrayIdx;
			}

			// The original controller repeats this zero-width call for each row.
			int16 numActive = _vm->_rnd->getRandomNumber(2, 2);
			int16 selectableClueCount = 5;
			for (int selectionIdx = 0; selectionIdx < numActive; selectionIdx++) {
				const int16 cluePoolIdx = _vm->_rnd->getRandomNumber(1, selectableClueCount);
				_ruleGlyphVisibility[clueSlotPool[cluePoolIdx] + clueSlotOffset] = 1;
				// Remove the selected one-based pool entry; the next element may be sentinel entry 6.
				for (int poolArrayIdx = cluePoolIdx; poolArrayIdx < selectableClueCount + 1; poolArrayIdx++)
					clueSlotPool[poolArrayIdx] = clueSlotPool[poolArrayIdx + 1];
				selectableClueCount -= 1;
			}
		}
		break;
	}
	default: // Level 4 leaves all entrances inactive.
		break;
	}

	// One-based clue slots 1-5 use zero-based rule-value entries 0-4 in @ref ZoombiniPuzzleCaves::_ruleTraitValues.
	// @ref ZoombiniPuzzleCaves::_primaryRuleTraitKind selects the primary trait kind.
	for (int clueSlot = 1; clueSlot < 6; clueSlot++) {
		if (_ruleGlyphVisibility[clueSlot]) {
			const int16 ruleValue = _ruleTraitValues[clueSlot - 1];
			switch (_primaryRuleTraitKind) {
			case ZmbTrait::kTraitHair:
				_ruleGlyphShapeIds[clueSlot] = ruleValue;
				break;
			case ZmbTrait::kTraitEyes:
				_ruleGlyphShapeIds[clueSlot] = ruleValue + 5;
				break;
			case ZmbTrait::kTraitNose:
				_ruleGlyphShapeIds[clueSlot] = ruleValue + 10;
				break;
			case ZmbTrait::kTraitFeet:
				_ruleGlyphShapeIds[clueSlot] = ruleValue + 15;
				break;
			}
		}
	}

	// One-based clue slots 6-10 use zero-based rule-value entries 5-9 in @ref ZoombiniPuzzleCaves::_ruleTraitValues.
	// @ref ZoombiniPuzzleCaves::_secondaryRuleTraitKind supplies the secondary trait kind.
	for (int clueSlot = 6; clueSlot < 11; clueSlot++) {
		if (_ruleGlyphVisibility[clueSlot]) {
			const int16 ruleValue = _ruleTraitValues[clueSlot - 1];
			switch (_secondaryRuleTraitKind) {
			case ZmbTrait::kTraitHair:
				_ruleGlyphShapeIds[clueSlot] = ruleValue;
				break;
			case ZmbTrait::kTraitEyes:
				_ruleGlyphShapeIds[clueSlot] = ruleValue + 5;
				break;
			case ZmbTrait::kTraitNose:
				_ruleGlyphShapeIds[clueSlot] = ruleValue + 10;
				break;
			case ZmbTrait::kTraitFeet:
				_ruleGlyphShapeIds[clueSlot] = ruleValue + 15;
				break;
			}
		}
	}
}

// =========================================================================
// Glyph rendering (legends for a puzzle instance)
// =========================================================================

ZmbRenderResult ZoombiniPuzzleCaves::renderEntranceGlyphs(ZmbFeature *feature) {
	(void)feature;

	// Iterate one-based clue slots 1-10 and draw the hieroglyph shape selected for each active slot.
	// At difficulty 1 while the startup attention sequence is active, omit the
	// current slot for 30 ticks. Slot 0 is the intervening all-visible sentinel.
	ZmbResource glyphRes(ZmbResource::kPage, kResBitmapShape10000_Glyphs);
	const bool blinkActive =
		_difficultyLevel == kPuzzleLevel1 && _glyphBlinkHiddenSlot < 6;

	for (int clueSlot = 1; clueSlot < 11; clueSlot++) {
		if (!_ruleGlyphVisibility[clueSlot])
			continue;

		if (blinkActive && clueSlot == _glyphBlinkHiddenSlot)
			continue;

		const byte shapeNumber = _ruleGlyphShapeIds[clueSlot]; // One-based tBMP 10000 shape number 1-20.
		if (shapeNumber == 0)
			continue;

		// Get sub-image height for vertical centering.
		MohawkSurface *ms = _vm->_gfx->findShape(glyphRes, shapeNumber);
		if (!ms || !ms->getSurface())
			continue;

		// Center the glyph horizontally on the upper wall and apply REGS 201 to its Y value.
		int16 halfWidth = static_cast<int16>(ms->getSurface()->w) / 2;
		int16 x = kRuleGlyphScreenX[clueSlot] - halfWidth;
		int16 y = kRuleGlyphScreenY[clueSlot] - _ruleGlyphYAdjustments[clueSlot];

		_vm->_gfx->drawShape(ZoombiniGraphics::kShapeScreen, glyphRes, shapeNumber, Common::Point(x, y), false, ZoombiniGraphics::kPaletteRemapNoseNet);
	}

	return ZmbRenderResult::kRendered;
}

// =========================================================================
// Gameplay methods
// =========================================================================

bool ZoombiniPuzzleCaves::isInWaitingArea(const Common::Point &pos) const {
	for (uint rectArrayIdx = 0; rectArrayIdx < ARRAYSIZE(_waitingAreaRects); rectArrayIdx++) {
		if (_waitingAreaRects[rectArrayIdx].contains(pos))
			return true;
	}
	return false;
}

int16 ZoombiniPuzzleCaves::findMatchingSeatNumber(const ZmbTrait &traits, int16 droppedSeatNumber) {
	// Determine which one-based seat number in the range 1-20 matches a Zoombini's traits.
	// The first row of @ref ZoombiniPuzzleCaves::_ruleTraitValues holds primary glyph values,
	// followed by secondary glyph values.
	// @ref ZoombiniPuzzleCaves::_seatOccupants marks already-filled seats.
	//
	// Inputs:
	// Traits: the dragged Snoid's traits.
	// Dropped seat: the preferred target during the forward scan.

	const byte primaryByte = traits[_primaryRuleTraitKind];
	const byte secondaryByte = traits[_secondaryRuleTraitKind];

	int16 primaryRuleValue = 0;
	for (int16 ruleValueIdx = 0; ruleValueIdx < _ruleValueCount; ruleValueIdx++) {
		if (_ruleTraitValues[ruleValueIdx] == primaryByte) {
			primaryRuleValue = _ruleTraitValues[ruleValueIdx];
			break;
		}
	}

	int16 secondaryRuleValue = 0;
	for (int16 ruleValueIdx = 0; ruleValueIdx < _ruleValueCount; ruleValueIdx++) {
		if (_ruleTraitValues[5 + ruleValueIdx] == secondaryByte) {
			secondaryRuleValue = _ruleTraitValues[5 + ruleValueIdx];
			break;
		}
	}

	const int16 firstUsableSeatNumber = _firstUsableSeatNumber;

	// First accept @p droppedSeatNumber when it is an unoccupied matching seat within the usable range.
	if (firstUsableSeatNumber < 21) {
		int16 seatNumber = firstUsableSeatNumber;
		while (seatNumber < 21) {
			const bool primaryOk = (primaryRuleValue == _seatPrimaryRuleValues[seatNumber]);
			const bool isDroppedSeat = (seatNumber == droppedSeatNumber);
			const bool empty = (_seatOccupants[seatNumber] == 0);
			const bool secondaryOk = (_ruleTraitCount <= 1) || (secondaryRuleValue == _seatSecondaryRuleValues[seatNumber]);
			if (primaryOk && isDroppedSeat && empty && secondaryOk)
				return seatNumber;
			seatNumber += 1;
		}
	}

	// Store one-based seat numbers in a zero-based candidate array and select an index in the range 0 through candidateCount - 1.
	// The twenty-two-entry storage retains two spare entries; current groups can contribute at most sixteen valid candidates.
	int16 candidateSeatNumbers[22];
	int16 candidateCount = 0;
	for (int16 seatNumber = firstUsableSeatNumber; seatNumber < 21; seatNumber++) {
		if (_seatOccupants[seatNumber])
			continue;
		if (primaryRuleValue != _seatPrimaryRuleValues[seatNumber])
			continue;
		if (1 < _ruleTraitCount && secondaryRuleValue != _seatSecondaryRuleValues[seatNumber])
			continue;
		candidateSeatNumbers[candidateCount] = seatNumber;
		candidateCount += 1;
	}

	if (0 < candidateCount) {
		const int16 candidateIdx = static_cast<int16>(_vm->_rnd->getRandomNumber(0, candidateCount - 1));
		return candidateSeatNumbers[candidateIdx];
	}
	// Return one-based seat 1 when no candidate is found; it is outside the current usable range and acts as the fallback sentinel.
	return 1;
}

void ZoombiniPuzzleCaves::handleCorrectPlacement(ZmbSnoid *snoid, int16 seatNumber, bool queueFinalWalkIn) {
	// A correct match queues the walk-in without starting the wrong-seat transfer chain.
	_acceptedSnoidCount += 1;
	snoid->_packIsOccupied = true;

	// Mark the direct one-based seat entry occupied for @ref ZoombiniPuzzleCaves::triggerSuccessAnim().
	// The function walks occupied Snoids out as part of the celebration.
	if (0 < seatNumber && seatNumber < 21)
		_seatOccupants[seatNumber] = snoid;
	linkSnoidToSeatZOrder(snoid, seatNumber);

	// Accepted-seat celebration frames do not inherit the last drag direction.
	// The seat table selects the directional sprite half before feet-specific SCRS 13000-13004 starts on the next page tick.
	if (0 < seatNumber && seatNumber < 21)
		snoid->setFacingLeft(kSeatFacingLeft[seatNumber - 1]);
	debugC(4, MohawkEngine_Zoombini::kDebugPage02, "caves: accepted zoombini at seat %d", seatNumber);
	// The walk-in is handled by the queue, not the seat-transfer chain.

	const bool isFirstPlacement = (_acceptedSnoidCount == 1);
	const bool isFinalPlacement = (_acceptedSnoidCount == _pageLoadedZmbCount);
	if (isFirstPlacement) {
		// Enable the Go button; its feature redraws every frame.
		_goButtonEnabled = true;
	} else if (isFinalPlacement) {
		// A multi-Snoid final placement is represented by the N-1 random group.
		// Unlike earlier placements, it is not also queued for an individual walk-in.
		_completionWalkInsActive = true;
		_snoidDragEnabled = false;
		_completionWalkInTargetCount = MAX<int16>(0, _pageLoadedZmbCount - 1);
		_completionWalkInStartedCount = 0;
		_completionWalkLastStartFrame = 0;
		_completionWalkPoolState = 0;
		queueCompletionNarratorSound();
	}

	// Push first and intermediate correct placements onto the walk-in LIFO stack.
	// The one-Snoid first-placement branch also uses this individual path.
	// @ref ZoombiniPuzzleCaves::onPostRenderFrame() drains the entire stack each tick,
	// so correct placements clustered in one frame start their walk-in animations together.
	if ((!isFinalPlacement || isFirstPlacement || queueFinalWalkIn) && static_cast<int>(_acceptedWalkStackSize) < ARRAYSIZE(_acceptedWalkStack)) {
		_acceptedWalkStack[_acceptedWalkStackSize].snoid = snoid;
		// Each authored script carries the offsets for one one-based feet trait.
		_acceptedWalkStack[_acceptedWalkStackSize].scrsId = static_cast<int16>(snoid->_trait._feet + kResScrs12999_WalkBase);
		_acceptedWalkStack[_acceptedWalkStackSize].targetPos = kSeatEntrancePositions[seatNumber - 1];
		_acceptedWalkStackSize += 1;
	}
}

void ZoombiniPuzzleCaves::triggerSuccessAnim(int16 staggerFrames, Common::Point destPos) {
	// Iterate @ref ZoombiniPuzzleCaves::_seatOccupants backward and queue up to three occupants.
	// Each queued Snoid walks toward the requested destination after an additional @p staggerFrames ticks.
	// Empty seats do not consume one of the three departure slots.
	uint32 frameBase = getCurrentFrameCounter();
	int16 fired = 0;

	// Scan the complete one-based authored seat domain 20 through 1.
	// The current group occupies at most the trailing sixteen seats, while empty leading seats are skipped without consuming a departure slot.
	for (int16 seatNumber = 20; 0 < seatNumber && fired < 3; seatNumber--) {
		ZmbSnoid *s = _seatOccupants[seatNumber];
		if (!s)
			continue;

		// Assign the bare Snoid flags before testing pack occupancy.
		// This removes @ref ZmbFeature::FLAG_04000000_OVERLAY,
		// which was set while the Snoid was linked into its seat group.
		// The departing runner then returns to positional Z-sorting.
		s->setFlags(ZmbFeature::FLAG_00000001_TYPE_SNOID);
		if (!s->_packIsOccupied)
			continue;

		s->setAnimTargetPos(destPos);

		// Enter state 10 without changing the feature position; the destination was stored separately above.
		// Passing a position to @ref ZmbSnoid::setAnimState() would immediately teleport the Snoid.
		// The teleport destination would be the off-screen point (660, 376).
		// Preserve old and new dirty coverage while materializing the waiting pose.
		addExternalDirtyRect(s->getClickRect());
		s->clearPreparedRenderHotspots();
		s->setAnimState(kSnoidAnimState010_ArrivalMotion, nullptr);
		prepareSnoidVisualCoverage(s, true);
		addExternalDirtyRect(s->getClickRect());

		// Write the next render deadline after entering state 10, including for the first selected Snoid.
		s->setDelayUntilFrame(frameBase);
		frameBase += staggerFrames;
		fired += 1;
	}

	// Line 1024 above reset each departing Snoid to bare TYPE_SNOID so it returns to positional
	// Z-sorting. Sort once here rather than waiting for the next enabled pass: the advance-click
	// branch already froze the runner order, so without this the departing Snoids would stay in
	// the bucket they held while seated.
	forceRunnerZSort();
}

void ZoombiniPuzzleCaves::handleWrongPlacement(ZmbSnoid *snoid, int16 droppedSeatNumber, int16 matchingSeatNumber) {
	// The seat-transfer chain starts with phase 0 of @ref ZoombiniPuzzleCaves::setupDoorAnimation().
	// Event 1 plays the reject SCRS, event 5 starts phase 1, event 2 plays the redirect SCRS, and event 4 finalizes it.

	_activeTransferSnoid = snoid;
	snoid->_packIsOccupied = true;

	_droppedSeatNumber = droppedSeatNumber;
	_matchingSeatNumber = matchingSeatNumber;

	_droppedSeatOverlayFeature = getEntranceOverlayFeature(droppedSeatNumber);
	_matchingSeatOverlayFeature = getEntranceOverlayFeature(matchingSeatNumber);

	// On a wrong drop, the Snoid reaches @p matchingSeatNumber through the redirect animation.
	if (0 <= matchingSeatNumber && matchingSeatNumber < 21)
		_seatOccupants[matchingSeatNumber] = snoid;

	_acceptedSnoidCount += 1;
	_snoidDragEnabled = false;
	// Trigger @ref ZoombiniPuzzleCaves::setupDoorAnimation() from @ref ZoombiniPuzzleCaves::onEveryFrame().
	_wrongSeatTransferPending = true;

	if (_acceptedSnoidCount == 1) {
		// Enable the Go button; its feature redraws every frame.
		_goButtonEnabled = true;
	} else if (_acceptedSnoidCount == _pageLoadedZmbCount) {
		queueCompletionNarratorSound();
	}
}

void ZoombiniPuzzleCaves::endDrag(const Common::Point &dropPos) {
	(void)dropPos;

	ZmbSnoid *snoid = finishSnoidDrag();
	if (!snoid)
		return;

	Common::Point snoidPos = snoid->getPointLoc();
	const int16 drawSlotIndex = hitTestDrawOnRegSlot(snoidPos, _clickZoneRadius, true);

	if (0 <= drawSlotIndex) {
		const int16 droppedSeatNumber = drawSlotIndex + 1;
		const int16 matchingSeatNumber = findMatchingSeatNumber(snoid->_trait, droppedSeatNumber);
		debugC(4, MohawkEngine_Zoombini::kDebugPage02, "caves: dropped zoombini at seat %d; matching seat is %d", droppedSeatNumber, matchingSeatNumber);
		// Leave the holding pose through the shared arrival state even when the final
		// accepted placement skips its individual walk-in script.
		settleSnoidAtTarget(snoid, kSeatEntrancePositions[droppedSeatNumber - 1]);
		// Accepted occupants enter the unsorted LOOP_ANIM group.
		// Their explicit link to a fixed seat group then survives every Z-sort.
		snoid->setFlags(ZmbFeature::FLAG_00000001_TYPE_SNOID | ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_04000000_OVERLAY);

		// A drop is correct only when the selected and matching seats agree.
		if (droppedSeatNumber == matchingSeatNumber) {
			setDrawOnRegOccupant(drawSlotIndex, snoid->getId());
			handleCorrectPlacement(snoid, droppedSeatNumber);
		} else {
			// Move the optimistic occupancy from the dropped seat to the matching seat before the transfer animation.
			setDrawOnRegOccupant(matchingSeatNumber - 1, snoid->getId());
			handleWrongPlacement(snoid, droppedSeatNumber, matchingSeatNumber);
		}
	} else {
		if (isInWaitingArea(snoidPos)) {
			// Ordinary free-space drop in the stepped waiting area. The common
			// drag validates terrain here and returns to the pickup point if the
			// location is blocked.
			settleSnoidAfterTerrainDrop(snoid, _dragOrigPos);
			return;
		}

		// Elsewhere, including the central lower opening, SCRS 12012 drops the Snoid below the floor.
		// Its event 10 starts SCRS 12013, anchored to the saved pickup position.
		_snoidDragEnabled = false;
		_activeTransferSnoid = snoid;
		_floorReturnPosition = _dragOrigPos;
		setupDoorAnimation(2);
	}
}

ZmbEventHandleResult ZoombiniPuzzleCaves::onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) {
	// Consume an armed departure at the start of the next click, even if the lion SCRB has not reached event 10.
	if (_departureState == ZmbDepartureState::kTriggered) {
		_departureState = ZmbDepartureState::kCompleted;
		executeDeparture();
		return ZmbEventHandleResult::kConsumed;
	}

	// Let base class handle button clicks (Go/Map/Help)
	ZmbEventHandleResult result = ZoombiniInteractive::onLButtonDown(absPos, relPos);
	if (result == ZmbEventHandleResult::kConsumed)
		return result;

	// The positive drag gate combines seat, floor, completion, and Go availability.
	if (!_snoidDragEnabled || !_pageActive)
		return ZmbEventHandleResult::kPassthrough;

	// Don't allow drag if already dragging
	if (isDragging())
		return ZmbEventHandleResult::kPassthrough;

	// Find Zoombini at click point
	ZmbSnoid *snoid = findSnoidAtPoint(absPos);
	if (!snoid)
		return ZmbEventHandleResult::kPassthrough;
	for (int16 seatNumber = 1; seatNumber < 21; seatNumber++) {
		if (_seatOccupants[seatNumber] == snoid)
			return ZmbEventHandleResult::kPassthrough;
	}

	// Don't drag snoids that are playing scripts
	SnoidAnimState state = snoid->getAnimState();
	if (state == kSnoidAnimState008_ScriptReject || state == kSnoidAnimState009_ScriptNormal)
		return ZmbEventHandleResult::kPassthrough;

	// Begin drag
	startSnoidDrag(snoid, absPos);
	return ZmbEventHandleResult::kConsumed;
}

void ZoombiniPuzzleCaves::onAnimFrame() {
	// The original engine can erase part of a neighboring seated Snoid while SCRS 12005 ejects a rejected Snoid at its matching seat.
	// ScummVM intentionally diverges to repair that original dirty-region bug.
	// Keep only the two numerically adjacent seats in the initial dirty region during the ejection animation.
	if (_activeTransferSnoid && _droppedSeatNumber != _matchingSeatNumber &&
		_activeTransferSnoid->getAnimState() == kSnoidAnimState009_ScriptNormal &&
		_activeTransferSnoid->getActiveScrsId() == _seatTransferScrsBaseId + 1) {
		static constexpr int16 kAdjacentSeatOffsets[2] = {-1, 1};
		for (int16 seatOffset : kAdjacentSeatOffsets) {
			const int16 seatNumber = _matchingSeatNumber + seatOffset;
			if (seatNumber < _firstUsableSeatNumber || 20 < seatNumber)
				continue;
			ZmbSnoid *occupant = _seatOccupants[seatNumber];
			if (occupant && occupant != _activeTransferSnoid && occupant->isRenderActivated())
				markFeatureVisualCoverageDirty(occupant, false);
		}
	}

	ZoombiniInteractive::onAnimFrame();
}

void ZoombiniPuzzleCaves::onEveryFrame() {
	if (_isUpdating || !_pageActive)
		return;
	PuzzleUpdateGuard updateGuard(_isUpdating);

	// The Go button uses disabled shape 1 until the first placement and enabled shape 2 afterwards.
	// Drive it from @ref ZoombiniPuzzleCaves::_goButtonEnabled every frame.
	// The button remains disabled until at least one Zoombini is accepted.
	setGoButtonsEnabled(_goButtonEnabled);
}

void ZoombiniPuzzleCaves::onPostRenderFrame() {
	if (_isUpdating || !_pageActive)
		return;
	PuzzleUpdateGuard updateGuard(_isUpdating);

	// A departure already animating at frame entry suppresses the page controller.
	if (isDeparturePending()) {
		return;
	}

	// Phase 2 plays SND 996 and starts the success animation before every
	// ordinary controller branch on this authored frame.
	if (_departurePhase == DeparturePhase::kSuccessPending02) {
		debugC(2, MohawkEngine_Zoombini::kDebugPage02, "caves: advancing departure from success pending to departing");
		_departurePhase = DeparturePhase::kDeparting03;
		triggerSuccessAnim(30, Common::Point(660, 376));
		ZoombiniInteractive::onGoButtonActivated();
	}

	// Process advance button click.
	// Detach the SCRB 6002 runner, reload it, and append it at the global-list tail.
	// Since the lion, seated Snoids, and seat foregrounds are LOOP_ANIM runners,
	// this makes the forepaw draw in front of every occupied seat for the Go animation.
	if (_goLionSequencePending && !_activeTransferSnoid) {
		debugC(2, MohawkEngine_Zoombini::kDebugPage02, "caves: starting Go lion sequence and final entrance phase");
		_goLionSequencePending = false;
		_departurePhase = DeparturePhase::kEntranceFinal01;
		// Freeze the runner order before the Go animation relinks SCRB 6002 to the list tail.
		// A later positional sort would undo that tail position.
		setRunnerZSortEnabled(false);
		// Load SCRB 6002 onto the final lion feature with the ledge-controller callback path.
		if (_lionEntranceFeatures[2]) {
			loadScrbOntoFeature(_lionEntranceFeatures[2], kResScrb6002_EntranceFinal);
			manualLinkAtEnd(_lionEntranceFeatures[2]);
		}
	}

	// Finalize the seat transfer latched by event 4.
	if (_seatTransferFinalizePending) {
		debugC(3, MohawkEngine_Zoombini::kDebugPage02, "caves: finalizing transfer from seat %d to seat %d", _droppedSeatNumber, _matchingSeatNumber);
		_seatTransferFinalizePending = false;

		// Load both selected door overlays and make them renderable.
		if (_droppedSeatOverlayFeature) {
			loadScrbOntoFeature(_droppedSeatOverlayFeature, static_cast<int16>(kResScrb9000_OverlayBase + _droppedSeatNumber - 1), false);
			_droppedSeatOverlayFeature->setResource(ZmbResource(ZmbResource::kPage, kResBitmapShape9000_Overlays));
			_droppedSeatOverlayFeature->activateRender();
		}

		if (_matchingSeatOverlayFeature) {
			loadScrbOntoFeature(_matchingSeatOverlayFeature, static_cast<int16>(kResScrb9000_OverlayBase + _matchingSeatNumber - 1), false);
			_matchingSeatOverlayFeature->setResource(ZmbResource(ZmbResource::kPage, kResBitmapShape9000_Overlays));
			_matchingSeatOverlayFeature->activateRender();
		}

		// A Snoid redirected from a wrong seat receives the same final seat-facing value after the transfer ends.
		if (_activeTransferSnoid && 0 < _matchingSeatNumber && _matchingSeatNumber < 21)
			_activeTransferSnoid->setFacingLeft(kSeatFacingLeft[_matchingSeatNumber - 1]);
		if (_activeTransferSnoid)
			debugC(4, MohawkEngine_Zoombini::kDebugPage02, "caves: redirected zoombini now faces its matching seat %d", _matchingSeatNumber);
	}

	// Process the wrong-placement flag set by the click handler.
	if (_wrongSeatTransferPending) {
		debugC(4, MohawkEngine_Zoombini::kDebugPage02, "caves: starting wrong-seat door animation");
		_wrongSeatTransferPending = false;
		// Freeze the current runner order for the redirect sequence.
		// The seat overlays and door runners registered by the animation phases must keep the
		// explicit list positions that the seat Z-order anchors establish.
		setRunnerZSortEnabled(false);
		setupDoorAnimation(0);
	}

	// Start the matching-seat transfer phase latched by event 5.
	if (_matchingSeatTransferPending) {
		debugC(4, MohawkEngine_Zoombini::kDebugPage02, "caves: starting matching-seat door animation");
		_matchingSeatTransferPending = false;
		setupDoorAnimation(1);
	}

	// One-shot startup glyph-attention sequence at difficulty 1.
	// Cycle through hiding one glyph and then showing all glyphs, from one-based clue slot 1 through slot 5.
	// Assigning sentinel slot 6 terminates the driver while the renderer permanently shows all glyphs.
	if (_difficultyLevel == kPuzzleLevel1 && _glyphBlinkHiddenSlot < 6) {
		if (_ruleGlyphRendererFeature) {
			uint32 currFrame = getCurrentFrameCounter();
			if (_nextGlyphBlinkFrame <= currFrame) {
				if (_glyphBlinkHiddenSlot != 0) {
					// Restore all glyphs for the intervening 30-tick phase.
					_nextGlyphBlinkFrame = currFrame + 30;
					_glyphBlinkHiddenSlot = 0;
				} else {
					// Hide the next one-based clue slot. Slot 6 is the terminal sentinel.
					_nextGlyphBlinkFrame = currFrame + 30;
					_glyphBlinkSequenceIndex += 1;
					_glyphBlinkHiddenSlot = _glyphBlinkSequenceIndex;
				}
				_ruleGlyphRendererFeature->setNeedsRedraw(true);
			}
		}
	}

	// Drain the entire walk-in stack in last-in-first-out order each tick.
	// Start every Snoid queued in the same frame together.
	while (0 < _acceptedWalkStackSize) {
		_acceptedWalkStackSize -= 1;
		// After decrement, the count is also the zero-based array index of the next valid LIFO entry.
		WalkInEntry &walkEntry = _acceptedWalkStack[_acceptedWalkStackSize];
		ZmbSnoid *walkSnoid = walkEntry.snoid;
		int16 walkScrsId = walkEntry.scrsId;
		const Common::Point walkTarget = walkEntry.targetPos;
		walkEntry.snoid = nullptr;
		walkEntry.scrsId = 0;
		if (!walkSnoid)
			continue;
		walkSnoid->addFlag(static_cast<ZmbFeature::Flag>(ZmbFeature::FLAG_00000001_TYPE_SNOID | ZmbFeature::FLAG_04000000_OVERLAY));
		startSnoidScrs(walkSnoid, ZmbResource(ZmbResource::kPage, walkScrsId), ZmbScrsCompletionMode::kReturnToIdle, &walkTarget);
	}

	// After the final placement, start one random idle pack Snoid's feet-specific walk-in every 30 ticks.
	// Continue until @ref ZoombiniPuzzleCaves::_completionWalkInTargetCount reaches zero.
	if (_completionWalkInsActive && _completionWalkInStartedCount < _completionWalkInTargetCount) {
		uint32 nowFrame = getCurrentFrameCounter();
		if (30 < nowFrame - _completionWalkLastStartFrame) {
			_completionWalkLastStartFrame = nowFrame;
			bool fired = false;
			// Attempt indices and random-pool indices are both zero-based in the range 0 through N - 1.
			for (int16 attemptIdx = 0; attemptIdx < _pageLoadedZmbCount && !fired; attemptIdx++) {
				const uint16 snoidPoolIdx = _vm->_rnd->getNonRepeatRandom(_pageLoadedZmbCount, _completionWalkPoolState);
				ZmbSnoid *cand = nullptr;
				for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
					ZmbSnoid *s = *it;
					if (!s || !s->hasFlag(ZmbFeature::FLAG_00000001_TYPE_SNOID))
						continue;
					if (s->getId() == static_cast<uint16>(10000 + snoidPoolIdx)) {
						cand = s;
						break;
					}
				}
				if (cand && cand->getAnimState() == kSnoidAnimState000_Idle && cand->_packIsOccupied) {
					const int16 scrsId = static_cast<int16>(cand->_trait._feet + kResScrs12999_WalkBase);
					if (startSnoidScrs(cand, ZmbResource(ZmbResource::kPage, scrsId))) {
						_completionWalkInStartedCount += 1;
						fired = true;
					}
				}
			}
		}
	} else if (_completionWalkInTargetCount <= _completionWalkInStartedCount && _completionWalkInsActive) {
		// Clear driver state.
		_completionWalkPoolState = 0;
		_completionWalkLastStartFrame = 0;
		_completionWalkInsActive = false;
		_completionWalkInStartedCount = 0;
	}
}

void ZoombiniPuzzleCaves::onFeatureAnimEvent(ZmbFeature *feature, int16 eventCode) {
	if (feature->hasFlag(ZmbFeature::FLAG_00000001_TYPE_SNOID)) {
		if (feature != _activeTransferSnoid)
			return;

		// The controller value is a paired-runner timing slot, not a synthetic completion event.
		// The controlling SCRB emits the real event 5 after SCRS 12004.
		// Forwarding the separate -1 end notification would start the matching-seat phase twice.
		if (eventCode != kAnimEventM1_End)
			handleEntranceDoorEvent(feature, eventCode);
	} else {
		// SCRB feature events -- dispatch based on which feature fired the event.
		// Door entrance and rejection runners use @ref ZoombiniPuzzleCaves::handleEntranceDoorEvent().
		// Glyph-panel runners use @ref ZoombiniPuzzleCaves::handleGlyphPanelEvent().
		if (feature == _ledgeControllerFeature || feature == _lionEntranceFeatures[2]) {
			handleGlyphPanelEvent(feature, eventCode);
		} else {
			handleEntranceDoorEvent(feature, eventCode);
		}
	}
}

// =========================================================================
// Entrance callback helpers
// =========================================================================

void ZoombiniPuzzleCaves::playEntranceScript(bool isReject, int16 scrsResId) {
	// Load the requested SCRS onto the active dropped Snoid.
	if (!_activeTransferSnoid)
		return;

	// The reject argument controls hide-on-completion; the SCRS pool selects the rendering state.
	const Common::Point *target = _hasEntranceScriptTarget ? &_entranceScriptTarget : nullptr;
	ZmbScrsCompletionMode completionMode;
	if (isReject)
		completionMode = ZmbScrsCompletionMode::kHide;
	else
		completionMode = ZmbScrsCompletionMode::kReturnToIdle;
	startSnoidScrs(_activeTransferSnoid, ZmbResource(ZmbResource::kPage, scrsResId), completionMode, target);
}

void ZoombiniPuzzleCaves::loadGlyphPanelFrame(int16 feedbackStep, bool replaceActive) {
	// Load the SCRB selected by zero-based @p feedbackStep and @ref ZoombiniPuzzleCaves::_ledgeControllerScrbBaseId.
	// Normal puzzle events wait for an inactive panel; debugger changes replace stale active feedback.
	if (!_ledgeControllerFeature)
		return;
	if (!replaceActive && _ledgeControllerFeature->isRenderActivated())
		return;
	if (_mistakeLimit < feedbackStep)
		return;

	loadScrbOntoFeature(_ledgeControllerFeature, static_cast<int16>(feedbackStep + _ledgeControllerScrbBaseId));
	// @ref ZoombiniPuzzleCaves::onFeatureAnimEvent() dispatches the glyph-panel callback.
}

ZmbFeature *ZoombiniPuzzleCaves::getEntranceOverlayFeature(int16 seatNumber) const {
	// One-based seats map to zero-based entries within three independently authored overlay arrays.
	// Seats 1-4 have door runners but no persistent overlay feature.
	if (5 <= seatNumber && seatNumber <= 11)
		return _upperSeatOverlayFeatures[seatNumber - 5];
	if (12 <= seatNumber && seatNumber <= 15)
		return _middleSeatOverlayFeatures[seatNumber - 12];
	if (16 <= seatNumber && seatNumber <= 20)
		return _lowerSeatOverlayFeatures[seatNumber - 16];
	return nullptr;
}

bool ZoombiniPuzzleCaves::seatAnchor_preRender(ZmbFeature *feature) {
	// Anchors never animate. Their only role is to hold a list position.
	(void)feature;
	return false;
}

ZmbRenderResult ZoombiniPuzzleCaves::seatAnchor_render(ZmbFeature *feature) {
	// Anchors never draw.
	(void)feature;
	return ZmbRenderResult::kSkipped;
}

void ZoombiniPuzzleCaves::registerSeatZOrderAnchor(int16 seatNumber) {
	if (seatNumber < 0 || static_cast<int16>(ARRAYSIZE(_seatZOrderAnchors)) <= seatNumber)
		return;

	ZmbFeature::EventHooks hooks;
	hooks.setPreRenderFunc(static_cast<ZmbFeature::OnPreRenderFunc>(&ZoombiniPuzzleCaves::seatAnchor_preRender));
	hooks.setRenderFunc(static_cast<ZmbFeature::OnRenderFunc>(&ZoombiniPuzzleCaves::seatAnchor_render));

	_seatZOrderAnchors[seatNumber] = loadVirtualFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape6000_Entrance),
														static_cast<int16>(kResScrb6000_EntranceBase), 0,
														ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_04000000_OVERLAY, hooks);
}

void ZoombiniPuzzleCaves::linkSnoidToSeatZOrder(ZmbSnoid *snoid, int16 seatNumber) {
	if (!snoid || seatNumber < 1 || 20 < seatNumber)
		return;

	// Link the occupant immediately after its seat's anchor, which sits between the seat's
	// entrance overlay and its DRAW_ON_REG door runner.
	if (_seatZOrderAnchors[seatNumber]) {
		manualLinkAfter(snoid, _seatZOrderAnchors[seatNumber]);
		return;
	}

	// Seats 1-4 have no anchor; fall back to the door runner's position.
	manualLinkBefore(snoid, _seatDoorFeatures[seatNumber - 1]);
}

void ZoombiniPuzzleCaves::setupDoorAnimation(int16 transferPhase) {
	// Set up the selected-seat transfer (0), matching-seat transfer (1), or floor-return script (2).

	if (!_activeTransferSnoid)
		return;

	// Phase 0 selects the dropped-on entrance overlay.
	// Phase 1 selects the matching entrance overlay, while phase 2 operates directly on the Snoid.
	ZmbFeature *doorFeature = nullptr;
	if (transferPhase == 0) {
		doorFeature = _droppedSeatOverlayFeature;
	} else if (transferPhase == 1) {
		doorFeature = _matchingSeatOverlayFeature;
	}
	// @p transferPhase == 2 operates directly on the Snoid and needs no feature.

	if (transferPhase < 2 && !doorFeature)
		return;

	if (transferPhase == 0) {
		// Phase 0: Door opening animation for SELECTED entrance (where the snoid was dropped).

		// Clear position override -- no initial position for script playback.

		// Load the selected entrance's door-open SCRB.
		// The resource group starts at base + 4 * (one-based seat - 1).
		_hasEntranceScriptTarget = false;
		linkSnoidToSeatZOrder(_activeTransferSnoid, _droppedSeatNumber);
		int16 doorScrbId = static_cast<int16>(_seatTransferScrbBaseId + 4 * _droppedSeatNumber - 4);
		loadScrbOntoFeature(doorFeature, doorScrbId);
		// Every 82xx transfer SCRB uses tBMP 8200.
		// The persistent seat runner normally uses tBMP 9000.
		// Switch its shape source for the duration of the transfer.
		doorFeature->setResource(ZmbResource(ZmbResource::kPage, kResBitmapShape8200_GlyphPanel));
		// Animation events enter through @ref ZoombiniPuzzleCaves::onFeatureAnimEvent().
		// It dispatches them to @ref ZoombiniPuzzleCaves::handleEntranceDoorEvent().

		// Destroy the old door overlay.
		if (_activeTransferOverlayFeature) {
			unloadScrbFeature(_activeTransferOverlayFeature);
			_activeTransferOverlayFeature = nullptr;
		}

		// Create the selected entrance's one-shot overlay SCRB.
		int16 overlayScrbId = static_cast<int16>(_seatTransferScrbBaseId + 4 * _droppedSeatNumber - 3);
		_activeTransferOverlayFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, _seatTransferScrbBaseId), overlayScrbId, 6,
														ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_04000000_OVERLAY);
		// Keep the transient entrance overlay inside the active Snoid's seat group.
		manualLinkAfter(_activeTransferOverlayFeature, _activeTransferSnoid);

		// @ref ZoombiniPage::loadScrbFeature() parses the hotspot groups.
		// The overlay and door feature share dispatch through @ref ZoombiniPuzzleCaves::onFeatureAnimEvent().

	} else if (transferPhase == 1) {
		// Phase 1: Door opening animation for MATCHING entrance (redirect after wrong placement).

		// Set the target-position override for the matching-entrance walk script.

		// Load the matching entrance's door-open SCRB.
		// Phase 1 starts two resources after the one-based seat group base.
		_entranceScriptTarget = kSeatEntrancePositions[_matchingSeatNumber - 1];
		_hasEntranceScriptTarget = true;
		linkSnoidToSeatZOrder(_activeTransferSnoid, _matchingSeatNumber);
		int16 doorScrbId = static_cast<int16>(_seatTransferScrbBaseId + 4 * _matchingSeatNumber - 2);
		loadScrbOntoFeature(doorFeature, doorScrbId);
		doorFeature->setResource(ZmbResource(ZmbResource::kPage, kResBitmapShape8200_GlyphPanel));

		// Destroy old door overlay
		if (_activeTransferOverlayFeature) {
			unloadScrbFeature(_activeTransferOverlayFeature);
			_activeTransferOverlayFeature = nullptr;
		}

		// Create the matching entrance's one-shot overlay SCRB.
		int16 overlayScrbId = static_cast<int16>(_seatTransferScrbBaseId + 4 * _matchingSeatNumber - 1);
		_activeTransferOverlayFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, _seatTransferScrbBaseId), overlayScrbId, 6,
														ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE |
															ZmbFeature::FLAG_04000000_OVERLAY);
		// Keep the transient entrance overlay inside the active Snoid's seat group.
		manualLinkAfter(_activeTransferOverlayFeature, _activeTransferSnoid);

		// Hotspot groups come from the SCRB data.

	} else if (transferPhase == 2) {
		// Phase 2: Door close/reset -- snoid walks back after out-of-zone drop.

		// Freeze the runner order for the SCRS 12012/12013 floor-bounce sequence.
		setRunnerZSortEnabled(false);

		// Clear snoid entrance tracking state.

		// Play SCRS 12012 (door close / walk-back script) on the active snoid.
		// SCRS 12012 is in pool 0 -> state 9 and hides the Snoid on completion.
		// Floor-return scripts always start right-facing, independent of drag direction.
		_activeTransferSnoid->setFacingLeft(false);
		_hasEntranceScriptTarget = false;
		startSnoidScrs(_activeTransferSnoid, ZmbResource(ZmbResource::kPage, kResScrs12012_DoorCloseWalkBack), ZmbScrsCompletionMode::kHide);

		// Keep the returning Snoid at the authored depth while positional sorting is frozen.
		// The glyph-panel controller is the boundary immediately following it in the runner list.
		if (_ledgeControllerFeature)
			manualLinkBefore(_activeTransferSnoid, _ledgeControllerFeature);
	}
}

void ZoombiniPuzzleCaves::handleEntranceDoorEvent(ZmbFeature *feature, int16 eventCode) {
	(void)feature;

	// Events fired from door entrance/rejection SCRB animations.
	switch (eventCode) {
	case kFloorDoorEventCode001_StartRejectEntrance:
		// Play reject entrance SCRS on the snoid.
		// This event fires from the door SCRB loaded by phase 0 of @ref ZoombiniPuzzleCaves::setupDoorAnimation()
		// to trigger the reject walk at the correct animation frame.
		if (_activeTransferSnoid)
			playEntranceScript(true, _seatTransferScrsBaseId);
		break;
	case kFloorDoorEventCode002_StartCorrectEntrance:
		// Play normal walk-into-cave SCRS on the snoid.
		// SCRS 12005 ends in FF05, which already dispatches adjusted event 4.
		// Do not synthesize the same event again
		// from the separate -1 completion notification or each wrong placement consumes two ledges.
		// This event fires from the door SCRB loaded by phase 1 of @ref ZoombiniPuzzleCaves::setupDoorAnimation()
		// to trigger the walk-into-correct-entrance animation.
		if (_activeTransferSnoid)
			playEntranceScript(false, static_cast<int16>(_seatTransferScrsBaseId + 1));
		break;
	case kFloorDoorEventCode004_FinishDoorTransition:
		// The redirect completed: advance the ledge feedback and remove the transient transfer overlay.
		// Re-enable positional sorting now that the redirect sequence has released its explicit order.
		setRunnerZSortEnabled(true);
		_seatTransferFinalizePending = true;
		_mistakeCount += 1;
		loadGlyphPanelFrame(_mistakeCount);
		// Destroy the door overlay feature.
		if (_activeTransferOverlayFeature) {
			unloadScrbFeature(_activeTransferOverlayFeature);
			_activeTransferOverlayFeature = nullptr;
		}
		break;
	case kFloorDoorEventCode005_ArmPageTransition:
		// Start the matching-seat phase from @ref ZoombiniPuzzleCaves::onPostRenderFrame().
		_matchingSeatTransferPending = true;
		break;
	case kFloorDoorEventCode010_StartFloorBounce:
		// Position adjustment: play SCRS 12013 (walk to position script).
		// SCRS 12013 ends in FF15 and directly dispatches adjusted event 20.
		_entranceScriptTarget = _floorReturnPosition;
		_hasEntranceScriptTarget = true;
		playEntranceScript(false, 12013);
		break;
	case kFloorDoorEventCode020_CheckEntranceCompletion:
		// Completion check: all entrances used?
		// Re-enable positional sorting after the floor-bounce sequence releases its explicit order.
		setRunnerZSortEnabled(true);
		_activeTransferSnoid = nullptr;
		_hasEntranceScriptTarget = false;
		_snoidDragEnabled = (_mistakeCount != _mistakeLimit);
		break;
	case kFloorDoorEventCode021_ForceEntranceCompletion:
		// Force completion.
		_activeTransferSnoid = nullptr;
		_hasEntranceScriptTarget = false;
		_snoidDragEnabled = false;
		break;
	default:
		break;
	}
}

void ZoombiniPuzzleCaves::handleGlyphPanelEvent(ZmbFeature *feature, int16 eventCode) {
	(void)feature;

	// Handle events from glyph-panel SCRB animations.
	// @ref ZoombiniPuzzleCaves::loadGlyphPanelFrame() and the advance button load these animations.
	switch (eventCode) {
	case kGlyphEventCode010_StartAnimation:
		// Phase change to animating state.
		_departurePhase = DeparturePhase::kSuccessPending02;
		break;
	case kGlyphEventCode020_CheckCompletion: {
		// Check whether all entrances have been used.
		// Reaching the mistake limit triggers the completion response.
		_activeTransferSnoid = nullptr;
		_hasEntranceScriptTarget = false;
		if (_mistakeCount == _mistakeLimit) {
			// Count active snoids to check if more remain.
			int16 activeSnoidCount = 0;
			for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
				ZmbSnoid *s = *it;
				if (s->isPackSnoid() && s->isRenderActivated() && s->_packIsOccupied)
					activeSnoidCount += 1;
			}
			// For a partial group, gate the completion voice by difficulty or an early page-visit count.
			if (activeSnoidCount < _pageLoadedZmbCount && passesPartialResultFeedbackGate())
				queuePartialSuccessNarratorSound();
			_snoidDragEnabled = false;
		} else {
			_snoidDragEnabled = true;
		}
		break;
	}
	case kGlyphEventCode021_ForceCompletion:
		// Force completion and update practice-level drag availability.
		_snoidDragEnabled = false;
		_activeTransferSnoid = nullptr;
		_hasEntranceScriptTarget = false;
		if (_vm->_state->inPracticeMode()) {
			// Clearing the walk-in progress counter makes dragging available again.
			_vm->_walkersInProgress = 0;
		}
		break;
	default:
		break;
	}
}

} // End of namespace Mohawk
