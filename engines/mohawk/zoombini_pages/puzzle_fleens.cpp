
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

#include "common/config-manager.h"

#include "mohawk/zoombini.h"
#include "mohawk/zoombini_graphics.h"
#include "mohawk/zoombini_metaengine.h"
#include "mohawk/zoombini_pages/puzzle_fleens.h"
#include "mohawk/zoombini_random.h"
#include "mohawk/zoombini_sound.h"
#include "mohawk/zoombini_state.h"

namespace Mohawk {

constexpr const char *ZoombiniPuzzleFleens::FleenTrait::kTraitValueNames[4][5];
constexpr Common::Point ZoombiniPuzzleFleens::kPackSnoidPositions[16];
constexpr Common::Point ZoombiniPuzzleFleens::kFleenLurePosition;
constexpr Common::Point ZoombiniPuzzleFleens::kFleenLureInitPosition;
constexpr int16 ZoombiniPuzzleFleens::kFleenHairShapeOffsets[6];
constexpr int16 ZoombiniPuzzleFleens::kFleenEyeShapeOffsets[6];
constexpr int16 ZoombiniPuzzleFleens::kFleenNoseShapeOffsets[6];
constexpr int16 ZoombiniPuzzleFleens::kFleenFeetShapeOffsets[6];

ZoombiniPuzzleFleens::ZoombiniPuzzleFleens(MohawkEngine_Zoombini *vm) : ZoombiniPuzzle(vm, ZoombiniPageType::kFleens, ZmbSrcPageKind::kFleens_10) {
}

ZoombiniPuzzleFleens::~ZoombiniPuzzleFleens() {
}

const ZoombiniPage::ScriptSoundPriorityRanges &ZoombiniPuzzleFleens::getScriptSoundPriorityRanges() const {
	// Original shared page dispatch automatically prepends SND 996-997 at priority 32.
	static const ZoombiniPage::ScriptSoundPriorityRanges kRanges = {
		{kSysResSoundRange0996_DepartSFX, kSysResSoundRange0997_ArriveSFX},
		{kSysResSoundRange20000_Base, kSysResSoundRange29999_Last},
		{kResSoundRange1000_PuzzleSfxBase, kResSoundRange1002_PuzzleSfxLast},
		{kResSoundRange0300_PageBase, kResSoundRange0324_PageLast},
		{kResSoundRange0425_PageBase, kResSoundRange0499_PageLast},
		{kResSoundRange0099_PageSingle, kResSoundRange0099_PageSingle},
		{kResSoundRange0375_PageBase, kResSoundRange0399_PageLast},
		{kResSoundRange7000_AmbientBase, kResSoundRange7099_AmbientLast},
		{kResSoundRange1800_ZoombiniVoiceBase, kResSoundRange1899_ZoombiniVoiceRegistrationLast},
		{kResSoundRange1200_SpecialSfx, kResSoundRange4099_PageLast},
		{kResSoundRange4100_FleenVoiceBase, kResSoundRange4199_FleenVoiceRegistrationLast},
		{kResSoundRange0175_PageBase, kResSoundRange0199_PageLast},
		{kSysResSoundRange0900_AmbientBase, kSysResSoundRange0944_AmbientLast}};
	return kRanges;
}

Audio::Mixer::SoundType ZoombiniPuzzleFleens::getFeatureSoundType(const ZmbFeature *feature, ZmbResource sndRes) const {
	if (dynamic_cast<const ZmbSnoid *>(feature) && sndRes._archiveKind == ZmbResource::kPage &&
		kResSound1800_ZoombiniVoiceBase <= sndRes._id && sndRes._id <= kResSound1814_ZoombiniVoiceLast)
		return Audio::Mixer::kSpeechSoundType;

	return ZoombiniPage::getFeatureSoundType(feature, sndRes);
}

ZmbResource ZoombiniPuzzleFleens::resolveSnoidVoiceResource(int16 sndResId) const {
	if (kResSound1800_ZoombiniVoiceBase <= sndResId && sndResId <= kResSound1899_ZoombiniVoiceRegistrationLast)
		return ZmbResource(ZmbResource::kPage, sndResId);

	return ZoombiniPage::resolveSnoidVoiceResource(sndResId);
}

void ZoombiniPuzzleFleens::open() {
	openArchive(ZMB_MHK_FLEENS);
}

void ZoombiniPuzzleFleens::setBackgroundMusic() {
	// This page does not start background music.
}

void ZoombiniPuzzleFleens::setBackgroundBitmap() {
	_vm->_gfx->setPalette(kResBackground300);
	_vm->_gfx->drawBackground(kResBackground300);
}

void ZoombiniPuzzleFleens::adjustSnoidScriptHotspotForRender(const ZmbSnoid *snoid, ZmbHotspot &hotspot) const {
	if (!ConfMan.getBool(MohawkMetaEngine_Zoombini::kOptionFixFleensTreeDescendFeetBug))
		return;

	const int16 activeScrsId = snoid->getActiveScrsId();
	// The round-tail scripts contain nose poses beyond the four-shape trait set.
	// Wrap those poses within the selected nose instead of spilling into the next trait set.
	if (kResScrs7026_Event8FeetBase <= activeScrsId && activeScrsId <= kResScrs7030_Event8FeetLast &&
		hotspot._hsId == 2 && 5 <= hotspot._shapeIdx && hotspot._shapeIdx <= 7) {
		hotspot._shapeIdx -= 4;
		return;
	}

	// SCRS 7027 frame 3 uses a vertical feet pose that crosses the face for one frame.
	// Reuse the following frame's horizontal pose without changing the authored position or layers.
	if (activeScrsId == kResScrs7027_Event8FeetSkates &&
		snoid->getLastFrameIdx() == 3 && hotspot._hsId == 0 && hotspot._shapeIdx == 55)
		hotspot._shapeIdx = 32;
}

void ZoombiniPuzzleFleens::initStates() {
	_clickZoneRadius = 100;
	_allTargetFleensMatched = false;
	_goInputEnabled = false;
	_pairSelectionInputEnabled = true;
	_boardingStartEnabled = true;
	_escapeOverlayLoaded = false;
	_snoidRaftOverlayLinkPending = false;
	_fleenRaftOverlayLinkPending = false;
	_departureRoundPending = false;
	_submittedPairQueueEventPending = false;
	_departureState = ZmbDepartureState::kIdle;
	_fullQueueEvictionLeadInComplete = false;
	_fullQueueEvictionPairTransitionComplete = false;
	_fullQueueEvictionRejectScriptComplete = false;
	_matchedTargetCount = 0;
	_submittedPairCount = 0;
	_pendingBoardingSnoidId = 0;
	_pendingBoardingFleenIndex = -1;
	_boardingSnoidId = 0;
	_boardingFleenIndex = -1;
	_evictionSnoidId = 0;
	_evictionFleenIndex = -1;
	_pendingBodyArrangement = 0;
	_idleSnoidFidgetTargetCount = 0;
	_idleSnoidFidgetStartedCount = 0;
	_idleSnoidFidgetLastSelectionFrame = 0;
	_idleSnoidFidgetSelectionInterval = 60;
	_idleSnoidFidgetPoolState = 0;
	_departureWalkPendingCount = 0;
	_departureWalkCompletedCount = 0;
	_draggedFleenIndex = -1;

	for (int entryIdx = 0; entryIdx < 3; entryIdx++)
		_targetSnoidOrdinals[entryIdx] = 0;
	for (int i = 0; i < 7; i++) {
		_submittedSnoidQueue[i] = 0;
		_submittedFleenQueue[i] = -1;
	}
	for (int i = 0; i < 16; i++)
		_snoidScriptCallbacks[i] = SnoidCallback::kNone;
}

void ZoombiniPuzzleFleens::loadFeatures() {
	loadTerrainBitmap(kResBitmapTerrain500);
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape4000_FleenCreature));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape0400_FeatureDetail));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape1000_FeatureGroup0));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape1100_FeatureGroup1));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape1200_FeatureGroup2));

	ZmbFeature *mainFeature = createPuzzleMainFeatureHead();
	loadSubFeature(mainFeature, ZmbResource(ZmbResource::kPage, kResBitmapShape1100_FeatureGroup1), kResScrb1100_SpecialFeature);
	ZmbFeature *overlayParent = mainFeature;
	for (uint16 i = 0; i < 7; i++)
		overlayParent = loadSubFeature(overlayParent, ZmbResource(ZmbResource::kPage, kResBitmapShape1200_FeatureGroup2), kResScrb1200_HideSpotBase + i);

	registerScrsGroup(kResScrs6000_RejectBase, 5);
	registerScrsGroup(kResScrs7000_NormalBase, 46);

	_matchProgressFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1000_FeatureGroup0), kResScrb1000_MainFeatureBase, 6,
											ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE);
	// SCRB 1003 ends its authored mismatch sequence with looping SND 1000.
	// Keep the owner explicit even while Fleens retains its page compatibility queue.
	_matchProgressFeature->setScriptSoundPolicy(ZmbFeature::ScriptSoundPolicy::kPriorityQueue);
	loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1100_FeatureGroup1), kResScrb1100_SpecialFeature, 7, kFleenLurePosition,
					ZmbFeature::FLAG_00002000_DRAW_ON_REG | ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM |
						ZmbFeature::FLAG_01000000_DEFER_RENDER);
	setDrawOnRegSnapPosition(0, kFleenLurePosition);

	configureStandardPuzzleControlRects(_helpButtonRect);
	loadStandardPuzzleControlFeatures(kResBitmapShape0400_FeatureDetail);
	setGoButtonsEnabled(false);

	buildZmbTraitSetup();
	spawnFleenCreatures();

	for (int16 i = 0; i < 7; i++) {
		uint32 flags = ZmbFeature::FLAG_04000000_OVERLAY;
		if (i == 0)
			flags |= ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE;
		_escapeOverlayFeatures[i] = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1200_FeatureGroup2), kResScrb1200_HideSpotBase + i, 6, flags);
	}

	loadZoombinisFromPack(kPackSnoidPositions, ARRAYSIZE(kPackSnoidPositions));
	layoutStaticAndWalkIn(0, false);
	renderFeatures();
	assignStaggeredWalkDelays(200, 45);

	startInitialLureScript();
	renderFeatures();
	schedulePackSnoids(false, false);
}

void ZoombiniPuzzleFleens::initHelpPrompt() {
	int16 helpSoundId = kSysResSound20079_HelpEasy;
	if (_entrySfxGroupFlags == ZmbSfxGroupFlags::kHardFirstTrigger_02)
		helpSoundId = kSysResSound20080_HelpHard;
	else if (_difficultyLevel == kPuzzleLevel2 || _difficultyLevel == kPuzzleLevel4)
		helpSoundId = _vm->_rnd->getRandomNumber(kSysResSound20079_HelpEasy, kSysResSound20080_HelpHard);
	_activeHelpSoundId = ZmbResource(ZmbResource::kSystem, helpSoundId);
}

void ZoombiniPuzzleFleens::buildZmbTraitSetup() {
	const ZmbStateActivePack &activePack = _vm->_state->_f._zmbPackActive;
	int16 zmbCount = 0;
	for (int16 i = 0; i < activePack.getPackZmbCount() && i < activePack.getEntryCapacity(); i++) {
		if (activePack.getEntry(i).getIsOccupied())
			zmbCount += 1;
	}
	if (zmbCount == 0)
		return;

	// Both save layouts persist the same two Fleens transform tables.
	// Their indices follow @ref ZmbTrait::TraitKind order: hair, eyes, nose, and feet.
	// Select the table belonging to the active layout.
	// Generated transforms persist across later Fleens visits and save/load operations.
	const bool isTlc = _vm->isVersionFamilyTlcV2();
	byte *traitValueRotations;
	byte *traitDestSlots;
	if (isTlc) {
		traitValueRotations = _vm->_state->_f._v2FleensTraitValueRotations;
		traitDestSlots = _vm->_state->_f._v2FleensTraitDestSlots;
	} else {
		traitValueRotations = _vm->_state->_f._v1FleensTraitValueRotations;
		traitDestSlots = _vm->_state->_f._v1FleensTraitDestSlots;
	}

	_targetSnoidOrdinals[0] = _vm->_rnd->getRandomNumber(1, zmbCount);
	if (zmbCount == 1) {
		_matchedTargetCount = 2;
		_escapeOverlayLoaded = true;
	} else if (zmbCount == 2) {
		_matchedTargetCount = 1;
	} else {
		_matchedTargetCount = 0;
	}

	if (2 <= zmbCount) {
		do {
			_targetSnoidOrdinals[1] = _vm->_rnd->getRandomNumber(1, zmbCount);
		} while (_targetSnoidOrdinals[1] == _targetSnoidOrdinals[0]);
	}
	if (3 <= zmbCount) {
		do {
			_targetSnoidOrdinals[2] = _vm->_rnd->getRandomNumber(1, zmbCount);
		} while (_targetSnoidOrdinals[2] == _targetSnoidOrdinals[0] || _targetSnoidOrdinals[2] == _targetSnoidOrdinals[1]);
	}

	// The first rotation byte is the serialized uninitialized-table sentinel.
	// Levels 2 and 4 replace all four rotations on every visit.
	// Levels 1 and 3 retain an initialized table without consuming these draws.
	if (traitValueRotations[0] == 0 || _difficultyLevel == kPuzzleLevel2 || _difficultyLevel == kPuzzleLevel4) {
		for (int i = 0; i < 4; i++)
			traitValueRotations[i] = static_cast<byte>(_vm->_rnd->getRandomNumber(1, 5));
	}

	// A zero destination slot keeps this source trait in its matching Fleen slot.
	// Levels 1 and 2 force the direct mapping without random draws.
	// Level 3 creates a map only when the saved table is uninitialized.
	// Level 4 replaces the complete map on every visit.
	if (_difficultyLevel <= kPuzzleLevel2) {
		for (int i = 0; i < 4; i++)
			traitDestSlots[i] = 0;
	} else if (traitDestSlots[0] == 0 || _difficultyLevel == kPuzzleLevel4) {
		// Serialized destination slots are one-based.
		// Start at 2 so the first source trait, hair, does not keep its direct slot.
		traitDestSlots[0] = static_cast<byte>(_vm->_rnd->getRandomNumber(2, 4));
		// Mark the first zero-based slot as used before generating the other three.
		uint32 poolState = 1u << (traitDestSlots[0] - 1);
		for (int i = 1; i < 4; i++)
			traitDestSlots[i] = static_cast<byte>(_vm->_rnd->getNonRepeatRandom(4, poolState) + 1);
	}

	// Copy value rotations to @ref ZoombiniPuzzleFleens::_traitValueRotations before Fleens spawn.
	// Copy destination kinds to @ref ZoombiniPuzzleFleens::_traitDestinationKinds before Fleens spawn.
	// @ref ZoombiniPuzzleFleens::spawnFleenCreatures adds each offset to the source trait value modulo five.
	// It then writes that value to the resolved destination kind.
	// Keep the serialized zero/direct representation out of @ref ZoombiniPuzzleFleens::_traitDestinationKinds.
	for (int i = 0; i < 4; i++) {
		_traitValueRotations[i] = traitValueRotations[i];
		if (traitDestSlots[i] == 0)
			_traitDestinationKinds[i] = ZmbTrait::traitKindFromIndex(i);
		else
			_traitDestinationKinds[i] = static_cast<ZmbTrait::TraitKind>(traitDestSlots[i] - 1);
	}
}

void ZoombiniPuzzleFleens::readFleenPositionRegs(int16 regsResId, Common::Array<Common::Point> &positions) {
	positions.clear();
	Common::SeekableReadStream *stream = _vm->getResource(ID_REGS, ZmbResource(ZmbResource::kPage, regsResId));

	static constexpr int16 kSpecialPositionCount = 3;
	static constexpr int16 kNormalPositionCount = 17;
	const int16 expectedCount = regsResId == kResRegs5000_SpecialPositions ? kSpecialPositionCount : kNormalPositionCount;
	const uint32 expectedSize = sizeof(int16) + expectedCount * 2 * sizeof(int16);
	if (!ZmbResource::hasSize(stream, expectedSize, expectedSize)) {
		delete stream;
		error("fleens: readFleenPositionRegs: malformed required REGS %d resource", regsResId);
		return;
	}

	const int16 count = stream->readSint16BE();
	if (count != expectedCount) {
		delete stream;
		error("fleens: readFleenPositionRegs: invalid REGS %d position count %d", regsResId, count);
		return;
	}

	for (int16 i = 0; i < count; i++) {
		const int16 x = stream->readSint16BE();
		const int16 y = stream->readSint16BE();
		positions.push_back(Common::Point(x, y));
	}
	delete stream;
}

void ZoombiniPuzzleFleens::spawnFleenCreatures() {
	const ZmbStateActivePack &activePack = _vm->_state->_f._zmbPackActive;
	_targetBranchPositions.clear();
	_normalBranchPositions.clear();
	readFleenPositionRegs(kResRegs5000_SpecialPositions, _targetBranchPositions);
	readFleenPositionRegs(kResRegs5001_NormalPositions, _normalBranchPositions);
	_fleenShapeRegs.parseStreams(_vm, ZmbResource::kPage, kResRegs4000_FleenRegistration, kResRegs4001_FleenRegistrationAlt);

	_activeFleenCount = 0;
	int16 specialPosition = 0;
	int16 normalPosition = 0;
	int16 snoidIndex = 0;
	for (int16 eIdx = 0; eIdx < activePack.getPackZmbCount() && eIdx < activePack.getEntryCapacity() && _activeFleenCount < 16; eIdx++) {
		const ZmbStateActiveEntry &entry = activePack.getEntry(eIdx);
		if (!entry.getIsOccupied())
			continue;

		FleenCreature &creature = _fleenCreatures[_activeFleenCount];
		creature.feature = nullptr;
		creature.pairedSnoidId = static_cast<uint16>(10000 + snoidIndex);
		creature.boardingState = 0;
		creature.facingLeft = false;
		creature.fidgetCounter = static_cast<byte>(_vm->_rnd->getRandomNumber(0, 80));

		const ZmbTrait &zmbTrait = entry.getTraits();
		FleenTrait transformedTrait;
		for (int traitIndex = 0; traitIndex < 4; traitIndex++) {
			const byte value = static_cast<byte>((_traitValueRotations[traitIndex] + zmbTrait[traitIndex] - 2) % 5 + 1);
			transformedTrait[_traitDestinationKinds[traitIndex]] = value;
		}
		creature.trait = transformedTrait;

		const int16 oneBasedIndex = snoidIndex + 1;
		creature.isTarget = oneBasedIndex == _targetSnoidOrdinals[0] || oneBasedIndex == _targetSnoidOrdinals[1] || oneBasedIndex == _targetSnoidOrdinals[2];
		if (creature.isTarget && specialPosition < static_cast<int16>(_targetBranchPositions.size())) {
			creature.assignedBranchPos = _targetBranchPositions[specialPosition];
			creature.posCode = static_cast<FleenPositionCode>(kFleenPosCode17_TargetBranchFirst + specialPosition);
			specialPosition += 1;
		} else if (normalPosition < static_cast<int16>(_normalBranchPositions.size())) {
			creature.assignedBranchPos = _normalBranchPositions[normalPosition];
			creature.posCode = static_cast<FleenPositionCode>(normalPosition);
			normalPosition += 1;
		} else {
			continue;
		}
		creature.renderAnchor = creature.assignedBranchPos;

		creature.active = true;
		_activeFleenCount += 1;
		snoidIndex += 1;
	}

	ZmbFeature::EventHooks hooks;
	hooks.setPreRenderFunc(static_cast<ZmbFeature::OnPreRenderFunc>(&ZoombiniPuzzleFleens::fleenCreature_preRender));
	hooks.setRenderFunc(static_cast<ZmbFeature::OnRenderFunc>(&ZoombiniPuzzleFleens::fleenCreature_render));

	ZmbFeature *normalFeatures[kFleenPosCode16_LowerBranchFifth + 1] = {};
	for (int16 i = 0; i < _activeFleenCount; i++) {
		FleenCreature &creature = _fleenCreatures[i];
		// Create one linkable runner per Fleen.
		// Events 2 and 140 later move that runner around its paired Snoid.
		creature.feature = loadVirtualFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape4000_FleenCreature), 0, 6,
											  ZmbFeature::FLAG_00000002_TYPE_GRIDWALKER | ZmbFeature::FLAG_04000000_OVERLAY, hooks);
		if (creature.feature)
			creature.feature->setManagesOwnVisualCoverage(true);
		loadFleenCreatureScrs(creature, kResScrs4000_FleenAnimationBase, FleenCreature::kCallbackNone);
		creature.animating = false;
		updateFleenVisualCoverage(creature);
		creature.feature->setNeedsRedraw(true);
		if (!creature.isTarget && creature.posCode < ARRAYSIZE(normalFeatures))
			normalFeatures[creature.posCode] = creature.feature;
	}

	manualLinkBefore(normalFeatures[kFleenPosCode02_UpperBranchThird], normalFeatures[kFleenPosCode01_UpperBranchSecond]);
	manualLinkBefore(normalFeatures[kFleenPosCode05_MiddleBranchSecond], normalFeatures[kFleenPosCode04_MiddleBranchFirst]);
	manualLinkAfter(normalFeatures[kFleenPosCode08_MiddleBranchFifth], normalFeatures[kFleenPosCode07_MiddleBranchFourth]);
	manualLinkAfter(normalFeatures[kFleenPosCode09_MiddleBranchSixth], normalFeatures[kFleenPosCode08_MiddleBranchFifth]);
	manualLinkBefore(normalFeatures[kFleenPosCode10_MiddleBranchSeventh], normalFeatures[kFleenPosCode08_MiddleBranchFifth]);
	manualLinkBefore(normalFeatures[kFleenPosCode11_MiddleBranchEighth], normalFeatures[kFleenPosCode10_MiddleBranchSeventh]);
}

void ZoombiniPuzzleFleens::loadFleenCreatureScrs(FleenCreature &creature, int16 scrsResId, FleenCreature::Callback callback,
												 const Common::Point *targetPos, bool callbackOnComplete) {
	creature.callback = callback;
	creature.callbackOnComplete = callbackOnComplete;
	creature.animating = true;
	if (!creature.feature) {
		warning("fleens: missing runner for SCRS %u", scrsResId);
		creature.animating = false;
		return;
	}

	const ZmbResource scrsResource(ZmbResource::kPage, scrsResId);
	const ZmbScriptDecoder::DecodedScrs *decodedScrs = getDecodedScrs(scrsResource);
	if (!creature.feature->setDecodedScrs(decodedScrs)) {
		error("fleens: required SCRS %u is malformed", scrsResId);
		return;
	}
	creature.feature->invalidateRenderedClickRect();
	creature.traitLayout = decodedScrs->traitLayout;
	creature.nextScrsFrameIdx = 0;
	creature.nextTickFrame = getCurrentFrameCounter() + creature.feature->getFrameInterval();

	// Align the first frame's root hotspot to the current anchor when no explicit target is supplied.
	// With an explicit target, scan backward and align the last positive root hotspot to that target.
	FleenScrsAnchorSelection anchorSelection;
	if (targetPos)
		anchorSelection = FleenScrsAnchorSelection::kLastAnchoredFrame01;
	else
		anchorSelection = FleenScrsAnchorSelection::kFirstFrame00;
	Common::Point rawAnchor;
	if (findFleenScrsAnchor(*decodedScrs, rawAnchor, anchorSelection)) {
		if (targetPos)
			creature.renderAnchor = *targetPos;
		creature.scriptAnchorOffset = Common::Point(rawAnchor.x - creature.renderAnchor.x, rawAnchor.y - creature.renderAnchor.y);
	}
}

bool ZoombiniPuzzleFleens::findFleenScrsAnchor(const ZmbScriptDecoder::DecodedScrs &decodedScrs, Common::Point &anchor,
											   FleenScrsAnchorSelection selection) {
	if (decodedScrs.frames.empty())
		return false;

	if (selection == FleenScrsAnchorSelection::kLastAnchoredFrame01) {
		for (int32 frameIdx = static_cast<int32>(decodedScrs.frames.size()) - 1; 0 <= frameIdx; frameIdx--) {
			const ZmbDecodedScriptFrame &frame = decodedScrs.frames[frameIdx];
			if (!frame.hotspots.empty() && 0 < frame.hotspots[0]._shapeIdx) {
				anchor = frame.hotspots[0].getPos();
				return true;
			}
		}
		return false;
	}

	const ZmbDecodedScriptFrame &firstFrame = decodedScrs.frames[0];
	if (firstFrame.hotspots.empty() || firstFrame.hotspots[0]._shapeIdx < 1)
		return false;
	anchor = firstFrame.hotspots[0].getPos();
	return true;
}

int16 ZoombiniPuzzleFleens::resolveFleenActionResource(FleenActionCode actionCode, const FleenCreature &creature) const {
	const int16 feet = creature.trait._feet;
	const FleenPositionCode position = creature.posCode;
	switch (actionCode) {
	case FleenActionCode::kDefaultAnimationAlias01:
	case FleenActionCode::kIdlePose02:
		return kResScrs4000_FleenAnimationBase;
	case FleenActionCode::kIdleFidget03:
		return kResScrs4001_FleenAnimation;
	case FleenActionCode::kAlternateAnimation04:
		return kResScrs4002_FleenAnimation;
	case FleenActionCode::kStartBoarding05:
		return kResScrs4003_FleenAnimation;
	case FleenActionCode::kApproachEscapePoint06:
		if (position < kFleenPosCode17_TargetBranchFirst) {
			return kResScrs4000_FleenAnimationBase + 4 + position;
		} else {
			return kResScrs4000_FleenAnimationBase + position;
		}
	case FleenActionCode::kReactAfterFirstTargetMatch07:
	case FleenActionCode::kReactAfterSecondTargetMatch08:
		if (kFleenPosCode17_TargetBranchFirst <= position && position <= kFleenPosCode19_TargetBranchLast) {
			int16 resourceBase;
			if (actionCode == FleenActionCode::kReactAfterFirstTargetMatch07) {
				resourceBase = kResScrs4020_Action7Base;
			} else {
				resourceBase = kResScrs4021_Action8Base;
			}
			return static_cast<int16>(resourceBase + 2 * (position - kFleenPosCode17_TargetBranchFirst));
		}
		return 0;
	case FleenActionCode::kLeaveAfterInitialLure09:
		return feet + kResScrs4025_Action9FeetBase;
	case FleenActionCode::kLeaveWithSubmittedPair10:
		return feet + kResScrs4030_Action10FeetBase;
	case FleenActionCode::kLeaveWithFinalMatchedPair11:
		return feet + kResScrs4035_Action11FeetBase;
	case FleenActionCode::kAdvanceQueuedPair12:
		return feet + kResScrs4040_Action12FeetBase;
	case FleenActionCode::kFollowFullQueueEviction13:
		return feet + kResScrs4045_Action13FeetBase;
	case FleenActionCode::kLeavePage14:
		if (position <= kFleenPosCode03_UpperBranchFourth)
			return kResScrs4058_Action14Position0;
		if (position <= kFleenPosCode11_MiddleBranchEighth)
			return kResScrs4057_Action14Position4;
		if (position <= kFleenPosCode16_LowerBranchFifth)
			return kResScrs4056_Action14Position12;
		return feet + kResScrs4050_Action14FeetBase;
	default:
		return 0;
	}
}

void ZoombiniPuzzleFleens::startFleenAction(FleenCreature &creature, FleenActionCode actionCode,
											FleenCreature::Callback callback, const Common::Point *targetPos, bool callbackOnComplete) {
	const int16 resourceId = resolveFleenActionResource(actionCode, creature);
	if (resourceId != 0) {
		if (creature.feature) {
			markFeatureVisualCoverageDirty(creature.feature, false);
			addExternalDirtyRect(creature.feature->getZSortRect());
		}
		creature.active = true;
		loadFleenCreatureScrs(creature, resourceId, callback, targetPos, callbackOnComplete);
		if (creature.feature) {
			updateFleenVisualCoverage(creature);
			addDirtyRect(creature.feature->getZSortRect());
			addExternalDirtyRect(creature.feature->getZSortRect());
			creature.feature->setNeedsRedraw(true);
		}
	}
}

ZoombiniPuzzleFleens::FleenCreature *ZoombiniPuzzleFleens::findFleenBySnoid(uint16 snoidId) {
	for (int16 i = 0; i < _activeFleenCount; i++) {
		if (_fleenCreatures[i].pairedSnoidId == snoidId)
			return &_fleenCreatures[i];
	}
	return nullptr;
}

const ZoombiniPuzzleFleens::FleenCreature *ZoombiniPuzzleFleens::findFleenBySnoid(uint16 snoidId) const {
	for (int16 i = 0; i < _activeFleenCount; i++) {
		if (_fleenCreatures[i].pairedSnoidId == snoidId)
			return &_fleenCreatures[i];
	}
	return nullptr;
}

ZoombiniPuzzleFleens::FleenCreature *ZoombiniPuzzleFleens::findFleenByFeature(const ZmbFeature *feature) {
	for (int16 i = 0; i < _activeFleenCount; i++) {
		if (_fleenCreatures[i].feature == feature)
			return &_fleenCreatures[i];
	}
	return nullptr;
}

byte &ZoombiniPuzzleFleens::FleenTrait::operator[](ZmbTrait::TraitKind kind) {
	static byte fallbackTrait = 0; // Dummy value for error handling
	switch (kind) {
	case ZmbTrait::kTraitHair:
		return _hair;
	case ZmbTrait::kTraitEyes:
		return _eyes;
	case ZmbTrait::kTraitNose:
		return _nose;
	case ZmbTrait::kTraitFeet:
		return _feet;
	default:
		error("fleens: FleenTrait::operator[]: invalid kind %d", static_cast<int>(kind));
		return fallbackTrait;
	}
}

const byte &ZoombiniPuzzleFleens::FleenTrait::operator[](ZmbTrait::TraitKind kind) const {
	return const_cast<FleenTrait *>(this)->operator[](kind);
}

const char *ZoombiniPuzzleFleens::FleenTrait::debugTraitValueName(ZmbTrait::TraitKind kind, byte value) {
	const byte kindInt = static_cast<byte>(kind);
	if (kindInt < 4 && 1 <= value && value <= 5)
		return kTraitValueNames[kindInt][value - 1];
	return "?";
}

Common::String ZoombiniPuzzleFleens::FleenTrait::toStr() const {
	return Common::String::format("%s, %s, %s, %s",
								  debugTraitValueName(ZmbTrait::kTraitHair, _hair),
								  debugTraitValueName(ZmbTrait::kTraitEyes, _eyes),
								  debugTraitValueName(ZmbTrait::kTraitNose, _nose),
								  debugTraitValueName(ZmbTrait::kTraitFeet, _feet));
}

int16 ZoombiniPuzzleFleens::countLoadedSnoids() const {
	int16 count = 0;
	for (int16 i = 0; i < _pageLoadedZmbCount; i++) {
		const ZmbSnoid *snoid = getSnoid(10000 + i);
		if (snoid && snoid->isRenderActivated() && snoid->_packIsOccupied)
			count += 1;
	}
	return count;
}

bool ZoombiniPuzzleFleens::fleenCreature_preRender(ZmbFeature *feature) {
	FleenCreature *creature = findFleenByFeature(feature);
	if (creature)
		tickFleenCreature(*creature);
	return false;
}

ZoombiniPuzzleFleens::FleenScrsAdvanceResult ZoombiniPuzzleFleens::advanceFleenScrsFrame(FleenCreature &creature) {
	const uint32 currentFrameCounter = getCurrentFrameCounter();
	if (currentFrameCounter < creature.nextTickFrame)
		return FleenScrsAdvanceResult::kWaiting00;

	creature.nextTickFrame = currentFrameCounter + creature.feature->getFrameInterval();
	const int32 frameCount = static_cast<int32>(creature.feature->getFrameCount());
	if (frameCount < 1)
		return FleenScrsAdvanceResult::kPlaybackComplete02;
	if (1 < frameCount && frameCount <= creature.nextScrsFrameIdx)
		return FleenScrsAdvanceResult::kPlaybackComplete02;

	creature.feature->setLastFrameIdx(creature.nextScrsFrameIdx);
	// A one-frame Fleen SCRS is a persistent pose rather than a completed action.
	if (1 < frameCount)
		creature.nextScrsFrameIdx += 1;
	return FleenScrsAdvanceResult::kFrameReady01;
}

void ZoombiniPuzzleFleens::tickFleenCreature(FleenCreature &creature) {
	if (!creature.active || !creature.feature || creature.feature->getFrameCount() == 0)
		return;

	if (!creature.animating) {
		if (getCurrentFrameCounter() < creature.nextTickFrame)
			return;
		creature.nextTickFrame = getCurrentFrameCounter() + creature.feature->getFrameInterval();
		if (_vm->_fidgetThreshold == 0)
			return;
		const int8 oldCounter = static_cast<int8>(creature.fidgetCounter);
		creature.fidgetCounter += 1;
		if (_vm->_fidgetThreshold + 16 < oldCounter) {
			FleenActionCode actionCode;
			if (50 < _vm->_rnd->getRandomNumber(1, 100)) {
				actionCode = FleenActionCode::kIdleFidget03;
			} else {
				actionCode = FleenActionCode::kIdlePose02;
			}
			startFleenAction(creature, actionCode);
			creature.fidgetCounter = 1;
		}
		return;
	}
	const FleenScrsAdvanceResult advanceResult = advanceFleenScrsFrame(creature);
	if (advanceResult == FleenScrsAdvanceResult::kWaiting00)
		return;

	// Merge the old click rectangle before replacing the materialized composite frame.
	// Then mark the new frame's coverage dirty.
	markFeatureVisualCoverageDirty(creature.feature, false);

	if (advanceResult == FleenScrsAdvanceResult::kPlaybackComplete02) {
		const FleenCreature::Callback callback = creature.callback;
		const bool callbackOnComplete = creature.callbackOnComplete;
		loadFleenCreatureScrs(creature, kResScrs4000_FleenAnimationBase, FleenCreature::kCallbackNone);
		creature.animating = false;
		// Custom Fleen runners normally install only the callback pointer.
		// Dispatch the end event only when @ref FleenCreature::callbackOnComplete is set.
		// Chase and branch reactions leave that flag clear.
		if (callbackOnComplete && callback == FleenCreature::kCallbackMovement)
			processFleenMovementEvent(creature, kAnimEventM1_End);
	} else {
		ZmbHotspotGroup *group = creature.feature->getCurrentScriptVisualFrame();
		if (group) {
			Common::Array<ZmbHotspot> hotspots = group->copyHotspots();
			if (!hotspots.empty() && 0 < hotspots[0]._shapeIdx)
				creature.renderAnchor = Common::Point(hotspots[0]._x - creature.scriptAnchorOffset.x, hotspots[0]._y - creature.scriptAnchorOffset.y);
		}
		const int32 frameIdx = creature.feature->getLastFrameIdx();
		dispatchFeatureFrameSound(creature.feature, frameIdx);
		byte eventCode = 0;
		if (creature.feature->getFrameEventCode(frameIdx, eventCode) &&
			creature.callback == FleenCreature::kCallbackMovement)
			processFleenMovementEvent(creature, static_cast<int16>(eventCode) - 1);
	}

	updateFleenVisualCoverage(creature);
	addDirtyRect(creature.feature->getZSortRect());
	creature.feature->setNeedsRedraw(true);
}

int16 ZoombiniPuzzleFleens::getFleenBodyLayerOffset(const FleenCreature &creature, int layer) const {
	const byte hair = CLIP<byte>(creature.trait._hair, 0, 5);
	const byte eye = CLIP<byte>(creature.trait._eyes, 0, 5);
	const byte nose = CLIP<byte>(creature.trait._nose, 0, 5);
	const byte feet = CLIP<byte>(creature.trait._feet, 0, 5);
	static constexpr int8 kLayerTrait[4][5] = {
		{3, -1, 2, 1, 0}, {3, 2, -1, 1, 0}, {-1, 1, 2, 3, 0}, {-1, 3, 2, 1, 0}};
	if (layer < 0 || 5 <= layer)
		return 0;
	const uint16 traitLayoutIndex = CLIP<uint16>(static_cast<uint16>(creature.traitLayout), 0, 3);
	switch (kLayerTrait[traitLayoutIndex][layer]) {
	case 0:
		return kFleenHairShapeOffsets[hair];
	case 1:
		return kFleenEyeShapeOffsets[eye];
	case 2:
		return kFleenNoseShapeOffsets[nose];
	case 3:
		return kFleenFeetShapeOffsets[feet];
	default:
		return 0;
	}
}

ZmbRenderResult ZoombiniPuzzleFleens::fleenCreature_render(ZmbFeature *feature) {
	FleenCreature *creature = findFleenByFeature(feature);
	if (creature)
		renderFleenCreature(*creature);
	return ZmbRenderResult::kRendered;
}

bool ZoombiniPuzzleFleens::resolveFleenRenderHotspot(const FleenCreature &creature,
													 const ZmbHotspot &hotspot, uint16 &shapeId, Common::Point &drawPos) const {
	if (hotspot._shapeIdx <= 0)
		return false;

	const int16 traitOffset = getFleenBodyLayerOffset(creature, hotspot._hsId);
	const int16 finalShapeId = 2 * (traitOffset + hotspot._shapeIdx) - (creature.facingLeft ? 0 : 1);
	if (finalShapeId <= 0)
		return false;

	shapeId = static_cast<uint16>(finalShapeId);
	drawPos = Common::Point(hotspot._x - creature.scriptAnchorOffset.x, hotspot._y - creature.scriptAnchorOffset.y);
	const Common::Point regsDelta = _fleenShapeRegs.getShapeDelta(shapeId);
	drawPos.x -= regsDelta.x;
	drawPos.y -= regsDelta.y;
	return true;
}

void ZoombiniPuzzleFleens::updateFleenVisualCoverage(FleenCreature &creature) {
	if (!creature.feature)
		return;
	if (!creature.active || creature.feature->getFrameCount() == 0) {
		creature.feature->setSortRect(Common::Rect());
		return;
	}

	ZmbHotspotGroup *group = creature.feature->getCurrentScriptVisualFrame();
	if (!group) {
		creature.feature->setSortRect(Common::Rect());
		return;
	}

	Common::Rect coverage;
	bool hasCoverage = false;
	Common::Array<ZmbHotspot> hotspots = group->copyHotspots();
	for (uint hotspotIndex = 0; hotspotIndex < hotspots.size(); hotspotIndex++) {
		uint16 shapeId = 0;
		Common::Point drawPos;
		if (!resolveFleenRenderHotspot(creature, hotspots[hotspotIndex], shapeId, drawPos))
			break;
		const Common::Rect shapeSize = _vm->_gfx->getShapeSize(ZmbResource(ZmbResource::kPage, kResBitmapShape4000_FleenCreature), shapeId);
		Common::Rect drawnRect(drawPos.x, drawPos.y, drawPos.x + shapeSize.width(), drawPos.y + shapeSize.height());
		drawnRect.clip(Common::Rect(0, 0, ZoombiniGraphics::kScreenWidth, ZoombiniGraphics::kScreenHeight));
		if (drawnRect.isEmpty())
			continue;
		if (hasCoverage)
			coverage.extend(drawnRect);
		else {
			coverage = drawnRect;
			hasCoverage = true;
		}
	}
	creature.feature->setSortRect(hasCoverage ? coverage : Common::Rect());
}

void ZoombiniPuzzleFleens::renderFleenCreature(const FleenCreature &creature) {
	if (!creature.active || !creature.feature || creature.feature->getFrameCount() == 0)
		return;
	ZmbHotspotGroup *group = creature.feature->getCurrentScriptVisualFrame();
	if (!group)
		return;

	Common::Array<ZmbHotspot> hotspots = group->copyHotspots();
	for (uint hotspotIndex = 0; hotspotIndex < hotspots.size(); hotspotIndex++) {
		const ZmbHotspot &hotspot = hotspots[hotspotIndex];
		uint16 shapeId = 0;
		Common::Point drawPos;
		if (!resolveFleenRenderHotspot(creature, hotspot, shapeId, drawPos))
			break;
		_vm->_gfx->drawShape(ZoombiniGraphics::kShapeScreen, ZmbResource(ZmbResource::kPage, kResBitmapShape4000_FleenCreature), shapeId, drawPos, false);
	}
}

void ZoombiniPuzzleFleens::processFleenMovementEvent(FleenCreature &creature, int16 eventCode) {
	switch (eventCode) {
	case kAnimEventM1_End:
		// The movement SCRS reached its terminal marker; stop this Fleen's callbacks.
		creature.active = false;
		break;
	case kFleenPursuitEventCode000_ToggleFacing:
		// The movement script reached its facing/link marker.
		// Toggle the creature and, when boarding is active, attach its overlay in the required order.
		creature.facingLeft = !creature.facingLeft;
		if (_fleenRaftOverlayLinkPending && 0 <= _boardingFleenIndex && &_fleenCreatures[_boardingFleenIndex] == &creature) {
			_fleenRaftOverlayLinkPending = false;
			if (_escapeOverlayFeatures[2])
				manualLinkBefore(creature.feature, _escapeOverlayFeatures[2]);
		}
		break;
	case kFleenPursuitEventCode001_LoadOverlay:
		// The first movement marker makes the deferred Fleen overlay visible.
		if (!_escapeOverlayLoaded) {
			if (_escapeOverlayFeatures[0]) {
				manualLinkBefore(creature.feature, _escapeOverlayFeatures[0]);
				_escapeOverlayFeatures[0]->activateRender();
				_escapeOverlayFeatures[0]->activateAnimate();
			}
			_escapeOverlayLoaded = true;
		}
		break;
	case kFleenPursuitEventCode002_LinkToSnoid: {
		// The creature reached the marker where it must be linked after the boarding Snoid.
		ZmbSnoid *snoid = getSnoid(_boardingSnoidId);
		if (snoid && creature.feature)
			manualLinkAfter(creature.feature, snoid);
		break;
	}
	case kFleenPursuitEventCode140_LinkBeforeSnoid: {
		// The late boarding marker places the creature before the boarding Snoid in draw order.
		ZmbSnoid *snoid = getSnoid(_boardingSnoidId);
		if (snoid && creature.feature)
			manualLinkBefore(creature.feature, snoid);
		break;
	}
	case kFleenPursuitEventCode137_Deactivate:
		// The movement action is complete; stop posting this Fleen's animation callbacks.
		creature.active = false;
		break;
	case kFleenPursuitEventCode218_PlayReactionSound:
		// Play the short random Fleen reaction attached to this movement marker.
		_vm->_sound->playSound(ZmbResource(ZmbResource::kPage, _vm->_rnd->getRandomNumber(kResSound4100_FleenVoiceBase, kResSound4124_FleenVoiceLast)),
							   Audio::Mixer::kSpeechSoundType);
		break;
	default:
		break;
	}
}

int16 ZoombiniPuzzleFleens::mapEventToScrsId(SnoidScriptEventType scriptEventType, const ZmbSnoid *snoid) const {
	const byte feet = snoid->_trait._feet;
	switch (scriptEventType) {
	case SnoidScriptEventType::kInitialLure:
		return kResScrs7036_Event1FeetBase + feet - 1;
	case SnoidScriptEventType::kSubmittedPairEscape:
		if (!_escapeOverlayLoaded)
			return kResScrs7041_Event2FeetBase + feet - 1;
		return (_matchedTargetCount == 3 ? kResScrs7005_MismatchCompleteBase : kResScrs7000_NormalBase) + feet - 1;
	case SnoidScriptEventType::kFullQueueEvictionPairTransition:
		return kResScrs7010_Action3;
	case SnoidScriptEventType::kFullQueueEvictionHideSnoid:
		return kResScrs7011_Action4FeetBase + feet - 1;
	case SnoidScriptEventType::kIdleFidget:
		return kResScrs7031_Event5FeetBase + feet - 1;
	case SnoidScriptEventType::kQueuedDepartureRoundTail:
		return kResScrs7026_Event8FeetBase + feet - 1;
	case SnoidScriptEventType::kFullQueueEvictionReject:
		return kResScrs6000_RejectBase + feet - 1;
	case SnoidScriptEventType::kFullQueueEvictionLeadIn:
		return kResScrs7016_Event7016FeetBase + feet - 1;
	case SnoidScriptEventType::kQueuedDepartureRoundFollower:
		return kResScrs7021_Event7021FeetBase + feet - 1;
	default:
		return 0;
	}
}

void ZoombiniPuzzleFleens::startSnoidScript(ZmbSnoid &snoid, SnoidScriptEventType scriptEventType, SnoidCallback callback,
											ZmbScrsCompletionMode completionMode, const Common::Point *initPos) {
	const int16 resourceId = mapEventToScrsId(scriptEventType, &snoid);
	if (resourceId == 0)
		return;
	if (startSnoidScrs(&snoid, ZmbResource(ZmbResource::kPage, resourceId), completionMode, initPos))
		setSnoidCallback(snoid, callback);
}

ZoombiniPuzzleFleens::SnoidCallback ZoombiniPuzzleFleens::getSnoidCallback(const ZmbSnoid &snoid) const {
	if (snoid.getId() < 10000 || 10016 <= snoid.getId())
		return SnoidCallback::kNone;
	return _snoidScriptCallbacks[snoid.getId() - 10000];
}

void ZoombiniPuzzleFleens::setSnoidCallback(const ZmbSnoid &snoid, SnoidCallback callback) {
	if (10000 <= snoid.getId() && snoid.getId() < 10016)
		_snoidScriptCallbacks[snoid.getId() - 10000] = callback;
}

void ZoombiniPuzzleFleens::startInitialLureScript() {
	ZmbSnoid *firstSnoid = getSnoid(10000);
	if (!firstSnoid)
		return;
	_pairSelectionInputEnabled = false;
	_boardingSnoidId = firstSnoid->getId();
	startSnoidScript(*firstSnoid, SnoidScriptEventType::kInitialLure, SnoidCallback::kSubmitSnoidLureFleenPair,
					 ZmbScrsCompletionMode::kReturnToIdle, &kFleenLureInitPosition);
}

void ZoombiniPuzzleFleens::startBoardingAnimation() {
	if (_pendingBoardingSnoidId == 0 || _pendingBoardingFleenIndex < 0)
		return;
	ZmbSnoid *snoid = getSnoid(_pendingBoardingSnoidId);
	if (!snoid) {
		_pendingBoardingSnoidId = 0;
		_pendingBoardingFleenIndex = -1;
		return;
	}
	// Wait for the Snoid to become idle before starting the boarding SCRS.
	// The shared drop path first runs state 4, which places the Snoid at the lure point and normalizes its facing direction.
	if (snoid->getAnimState() != kSnoidAnimState000_Idle)
		return;

	_boardingSnoidId = _pendingBoardingSnoidId;
	_boardingFleenIndex = _pendingBoardingFleenIndex;
	_pendingBoardingSnoidId = 0;
	_pendingBoardingFleenIndex = -1;
	beginBoardingAnimation(*snoid, _boardingFleenIndex, true);
}

void ZoombiniPuzzleFleens::beginBoardingAnimation(ZmbSnoid &snoid, int16 fleenIndex, bool appendQueue) {
	if (fleenIndex < 0 || _activeFleenCount <= fleenIndex)
		return;

	_boardingSnoidId = snoid.getId();
	_boardingFleenIndex = fleenIndex;
	_boardingStartEnabled = false;
	_submittedPairQueueEventPending = true;
	// Boarding suspends new idle fidgets for both Snoids and Fleens.
	_vm->_fidgetThreshold = 0;

	FleenCreature &creature = _fleenCreatures[fleenIndex];
	if (creature.isTarget && creature.posCode <= kFleenPosCode19_TargetBranchLast) {
		_matchedTargetCount += 1;
		if (_vm->_state->inPracticeMode() && 3 <= _matchedTargetCount)
			_vm->_state->_f._isDirty = true;
	}

	if (appendQueue && _submittedPairCount < 7) {
		_submittedSnoidQueue[_submittedPairCount] = snoid.getId();
		_submittedFleenQueue[_submittedPairCount] = fleenIndex;
		_submittedPairCount += 1;
	}
	startSnoidScript(snoid, SnoidScriptEventType::kSubmittedPairEscape, SnoidCallback::kSubmitSnoidLureFleenPair);
}

void ZoombiniPuzzleFleens::spawnFleenEscapeFeature() {
	const int16 scrbId = static_cast<int16>(kResScrb1004_ExitBase + _vm->_rnd->getRandomNumber(0, 2));
	_escapeExitFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1000_FeatureGroup0), scrbId, 6,
										 ZmbFeature::FLAG_00001000_TOPMOST | ZmbFeature::FLAG_00100000_PLAY_ONCE);
	// Registration alone leaves the PLAY_ONCE runner on frame 0.
	// Reload SCRB 0 immediately to activate the animation.
	if (_escapeExitFeature)
		loadScrbOntoFeature(_escapeExitFeature, 0);
}

void ZoombiniPuzzleFleens::onFleenEscapeComplete() {
	// The exit SCRB has reached its completion callback, after the bee chase and the
	// under-tree Fleen escape. From here the page keeps its explicit runner order:
	// the Fleen creature runners are relinked around their paired Snoids by events 2 and 140,
	// and a positional sort would discard those links.
	setRunnerZSortEnabled(false);
	_goInputEnabled = true;
	schedulePackSnoids(false, true);
	const int16 loadedZmbCount = countLoadedSnoids();

	if (0 < loadedZmbCount && loadedZmbCount == _pageLoadedZmbCount) {
		queueCompletionNarratorSound();
	} else if (0 < loadedZmbCount && passesPartialResultFeedbackGate()) {
		queuePartialSuccessNarratorSound();
	}

	if (_escapeExitFeature)
		_escapeExitFeature->scheduleClose();
	_escapeExitFeature = nullptr;
	const int16 loadedZmbCountAfterExit = countLoadedSnoids();
	if (loadedZmbCountAfterExit == 16) {
		_idleSnoidFidgetTargetCount = 13;
	} else if (8 < loadedZmbCountAfterExit) {
		_idleSnoidFidgetTargetCount = loadedZmbCountAfterExit - 8;
	}
}

void ZoombiniPuzzleFleens::processFleenEscapeRange(int16 eventCode) {
	FleenPositionCode minPosition;
	FleenPositionCode maxPosition;
	if (eventCode == kFleenEscapeEventCode133_EscapeRangeNear) {
		minPosition = kFleenPosCode00_UpperBranchFirst;
		maxPosition = kFleenPosCode03_UpperBranchFourth;
	} else if (eventCode == kFleenEscapeEventCode134_EscapeRangeMiddle) {
		minPosition = kFleenPosCode04_MiddleBranchFirst;
		maxPosition = kFleenPosCode16_LowerBranchFifth;
	} else {
		minPosition = kFleenPosCode20_LuredOffBranch;
		maxPosition = kFleenPosCode25_LuredRangeLast;
	}
	for (int16 i = 0; i < _activeFleenCount; i++) {
		FleenCreature &candidate = _fleenCreatures[i];
		if (candidate.boardingState < 2 && minPosition <= candidate.posCode && candidate.posCode <= maxPosition) {
			candidate.boardingState = 2;
			// Event 135 is delivered by the independent bee SCRB runner.
			// Restart @ref FleenActionCode::kLeavePage14 without replacing the callback pointer, unlike ranges 133 and 134.
			FleenCreature::Callback callback;
			if (eventCode == kFleenEscapeEventCode135_EscapeRangeFar) {
				callback = candidate.callback;
			} else {
				callback = FleenCreature::kCallbackMovement;
			}
			startFleenAction(candidate, FleenActionCode::kLeavePage14, callback, nullptr, true);
		}
	}
}

void ZoombiniPuzzleFleens::processInitialLureOrSubmittedPairEscapeEvent(ZmbSnoid &snoid, int16 eventCode) {
	FleenCreature *creature = _boardingFleenIndex < 0 ? nullptr : &_fleenCreatures[_boardingFleenIndex];
	switch (eventCode) {
	case kAnimEventM1_End:
		// The lure or submitted-pair escape SCRS ended; restore idle Snoid rendering.
		setSnoidCallback(snoid, SnoidCallback::kNone);
		_pairSelectionInputEnabled = true;
		_boardingStartEnabled = true;
		_boardingSnoidId = 0;
		_boardingFleenIndex = -1;
		snoid.setAnimState(kSnoidAnimState000_Idle);
		snoid.setupIdleHotspots();
		_vm->_fidgetThreshold = 64;
		break;
	case kFleenEscapeEventCode000_ToggleFacing:
		// The escaping Snoid reached its facing marker.
		// Apply any pending body arrangement and complete the deferred overlay link.
		snoid.setFacingLeft(!snoid.isFacingLeft());
		applyPendingBodyArrangement(snoid);
		if (_snoidRaftOverlayLinkPending && _escapeOverlayFeatures[2]) {
			_snoidRaftOverlayLinkPending = false;
			snoid.addFlag(ZmbFeature::FLAG_04000000_OVERLAY);
			manualLinkBefore(&snoid, _escapeOverlayFeatures[2]);
		}
		break;
	case kFleenEscapeEventCode004_StartBoarding:
		// Begin the Fleen's move from the waiting position toward the escape point.
		_snoidRaftOverlayLinkPending = true;
		if (creature && creature->posCode <= kFleenPosCode16_LowerBranchFifth)
			startFleenAction(*creature, FleenActionCode::kStartBoarding05, FleenCreature::kCallbackMovement);
		break;
	case kFleenEscapeEventCode005_MoveToEscapePoint:
		// Move the selected Fleen to the escape hand-off point.
		if (creature) {
			const Common::Point target(429, 223);
			startFleenAction(*creature, FleenActionCode::kApproachEscapePoint06, FleenCreature::kCallbackMovement, &target);
		}
		break;
	case kFleenEscapeEventCode006_ProcessMismatchGroup:
		// Resolve one mismatch group and start the next Fleen reactions, if any remain.
		if (creature && kFleenPosCode17_TargetBranchFirst <= creature->posCode &&
			creature->posCode <= kFleenPosCode19_TargetBranchLast &&
			1 <= _matchedTargetCount && _matchedTargetCount <= 3) {
			if (_matchProgressFeature)
				loadScrbOntoFeature(_matchProgressFeature, static_cast<int16>(kResScrb1000_MainFeatureBase + _matchedTargetCount));
			for (int i = 0; i < 3; i++) {
				const uint16 targetSnoidId = _targetSnoidOrdinals[i] == 0 ? 0 : 9999 + _targetSnoidOrdinals[i];
				if (targetSnoidId == creature->pairedSnoidId)
					_targetSnoidOrdinals[i] = 0;
			}
			if (_matchedTargetCount < 3) {
				for (int i = 0; i < 3; i++) {
					FleenCreature *targetCreature;
					if (_targetSnoidOrdinals[i] == 0) {
						targetCreature = nullptr;
					} else {
						targetCreature = findFleenBySnoid(9999 + _targetSnoidOrdinals[i]);
					}
					if (targetCreature) {
						FleenActionCode reactionActionCode;
						if (_matchedTargetCount == 1) {
							reactionActionCode = FleenActionCode::kReactAfterFirstTargetMatch07;
						} else {
							reactionActionCode = FleenActionCode::kReactAfterSecondTargetMatch08;
						}
						startFleenAction(*targetCreature, reactionActionCode, FleenCreature::kCallbackMovement);
					}
				}
			}
			_allTargetFleensMatched = _matchedTargetCount == 3;
		}
		break;
	case kFleenEscapeEventCode007_MoveToExit:
		// Send the submitted Snoid's paired Fleen off its branch and release the occupied draw slot.
		if (creature) {
			FleenActionCode actionCode;
			Common::Point target;
			if (!_escapeOverlayLoaded) {
				actionCode = FleenActionCode::kLeaveAfterInitialLure09;
				target = Common::Point(214, 207);
			} else if (_matchedTargetCount == 3) {
				actionCode = FleenActionCode::kLeaveWithFinalMatchedPair11;
				target = Common::Point(-56, 241);
			} else {
				actionCode = FleenActionCode::kLeaveWithSubmittedPair10;
				target = Common::Point(214, 207);
			}
			startFleenAction(*creature, actionCode, FleenCreature::kCallbackMovement, &target);
			creature->posCode = kFleenPosCode20_LuredOffBranch;
			_fleenRaftOverlayLinkPending = true;
		}
		// Release the single lure DRAW_ON_REG seat as soon as the submitted Snoid leaves it.
		clearDrawOnRegOccupant(0);
		break;
	case kFleenEscapeEventCode008_LinkToEscapeOverlay:
		// Put the departing Snoid in front of the escape overlay for the visible hand-off.
		snoid.addFlag(ZmbFeature::FLAG_04000000_OVERLAY);
		if (_escapeOverlayFeatures[0])
			manualLinkBefore(&snoid, _escapeOverlayFeatures[0]);
		break;
	case kFleenEscapeEventCode009_LinkToFleen:
		// Link the Snoid immediately before its Fleen companion.
		if (creature && creature->feature)
			manualLinkBefore(&snoid, creature->feature);
		break;
	case kFleenEscapeEventCode028_LinkToEscapeOverlayLate:
		// Repeat the escape overlay link at the late marker used by the deep mismatch path.
		if (_escapeOverlayFeatures[0])
			manualLinkBefore(&snoid, _escapeOverlayFeatures[0]);
		break;
	case kFleenEscapeEventCode030_SpawnExit:
		// The escape sequence reached its exit marker; create the exit animation feature.
		spawnFleenEscapeFeature();
		break;
	case kFleenEscapeEventCode132_StartQueuedDeparture:
		// Begin the queued departure batch after the escape controller signals readiness.
		if (_submittedPairQueueEventPending) {
			_submittedPairQueueEventPending = false;
			const bool mustEvictOldestPair = _submittedPairCount == 7;
			for (int16 queueIndex = 0; queueIndex < _submittedPairCount - 1; queueIndex++) {
				ZmbSnoid *queuedSnoid = getSnoid(_submittedSnoidQueue[queueIndex]);
				if (queuedSnoid) {
					SnoidCallback callback;
					if (mustEvictOldestPair && queueIndex == 0) {
						callback = SnoidCallback::kFullQueueEvictionLeadIn;
					} else {
						callback = SnoidCallback::kNone;
					}
					startSnoidScript(*queuedSnoid, SnoidScriptEventType::kFullQueueEvictionLeadIn, callback);
				}
				const int16 fleenIndex = _submittedFleenQueue[queueIndex];
				if (0 <= fleenIndex)
					startFleenAction(_fleenCreatures[fleenIndex], FleenActionCode::kAdvanceQueuedPair12);
			}
			if (mustEvictOldestPair) {
				_evictionSnoidId = _submittedSnoidQueue[0];
				_evictionFleenIndex = _submittedFleenQueue[0];
				shiftDepartureQueue();
			}
		}
		break;
	case kFleenEscapeEventCode133_EscapeRangeNear:
	case kFleenEscapeEventCode134_EscapeRangeMiddle:
	case kFleenEscapeEventCode135_EscapeRangeFar:
		// Mark Fleens in the event-specific position range as escaped and restart @ref FleenActionCode::kLeavePage14.
		processFleenEscapeRange(eventCode);
		break;
	case kFleenEscapeEventCode137_DeactivateSnoid:
		// Hide the rejected Snoid when its escape animation reaches the terminal marker.
		snoid.deactivateRender();
		break;
	default:
		handleBodyArrangementScriptEvent(snoid, eventCode);
		break;
	}
}

void ZoombiniPuzzleFleens::processDepartureEvent(ZmbSnoid &snoid, SnoidCallback callback, int16 eventCode) {
	if (eventCode == kFleenPursuitEventCode000_ToggleFacing) {
		snoid.setFacingLeft(!snoid.isFacingLeft());
		applyPendingBodyArrangement(snoid);
	} else {
		handleBodyArrangementScriptEvent(snoid, eventCode);
	}

	switch (callback) {
	case SnoidCallback::kFullQueueEvictionLeadIn:
		// The SCRS 7016-7020 lead-in completes before the current pair is relinked.
		// The next frame starts the fixed SCRS 7010 pair transition.
		if (eventCode == kAnimEventM1_End)
			_fullQueueEvictionLeadInComplete = true;
		break;
	case SnoidCallback::kFullQueueEvictionPairTransition:
		// SCRS 7010 completes before SCRS 6000-6004 rejects the Snoid.
		if (eventCode == kAnimEventM1_End)
			_fullQueueEvictionPairTransitionComplete = true;
		break;
	case SnoidCallback::kFullQueueEvictionRejectAndStartFleenAction:
		// SCRS 6000-6004 emits event 60 to start @ref FleenActionCode::kFollowFullQueueEviction13 for the paired Fleen.
		// Its completion starts SCRS 7011-7015, which hides the evicted Snoid.
		if (eventCode == kFleenPursuitEventCode060_StartFinalAction && 0 <= _evictionFleenIndex)
			startFleenAction(_fleenCreatures[_evictionFleenIndex], FleenActionCode::kFollowFullQueueEviction13);
		else if (eventCode == kAnimEventM1_End)
			_fullQueueEvictionRejectScriptComplete = true;
		break;
	case SnoidCallback::kQueuedDepartureRoundTail:
		// SCRS 7026-7030 is assigned to the final Snoid in a Go departure round.
		// Event 131 arms the next round while queued pairs remain.
		if (eventCode == kFleenPursuitEventCode131_ArmDeparture && 0 < _submittedPairCount)
			_departureRoundPending = true;
		else if (eventCode == kAnimEventM1_End) {
			if (0 < _departureWalkPendingCount)
				_departureWalkPendingCount -= 1;
			_departureWalkCompletedCount += 1;
		}
		break;
	case SnoidCallback::kFullQueueEvictionHideSnoid:
		// SCRS 7011-7015 is the terminal full-queue eviction script.
		if (eventCode == kAnimEventM1_End)
			snoid.deactivateRender();
		break;
	default:
		break;
	}

	if (eventCode == kAnimEventM1_End) {
		setSnoidCallback(snoid, SnoidCallback::kNone);
		if (callback != SnoidCallback::kFullQueueEvictionHideSnoid) {
			snoid.setAnimState(kSnoidAnimState000_Idle);
			snoid.setupIdleHotspots();
		}
	}
}

void ZoombiniPuzzleFleens::onFeatureAnimEvent(ZmbFeature *feature, int16 eventCode) {
	if (feature == _escapeExitFeature) {
		if (eventCode == kAnimEventM1_End)
			onFleenEscapeComplete();
		else if (eventCode == kFleenEscapeEventCode135_EscapeRangeFar)
			processFleenEscapeRange(eventCode);
		return;
	}
	if (feature == _escapeOverlayFeatures[0]) {
		if (eventCode == kFleenPursuitEventCode136_ShowBoardingSnoid) {
			ZmbSnoid *snoid = getSnoid(_boardingSnoidId);
			if (snoid)
				snoid->activateRender();
			if (0 <= _boardingFleenIndex) {
				FleenCreature &creature = _fleenCreatures[_boardingFleenIndex];
				creature.active = true;
				updateFleenVisualCoverage(creature);
				addDirtyRect(creature.feature->getZSortRect());
				addExternalDirtyRect(creature.feature->getZSortRect());
				creature.feature->setNeedsRedraw(true);
			}
		}
		return;
	}
	if (!feature->hasFlag(ZmbFeature::FLAG_00000001_TYPE_SNOID))
		return;
	ZmbSnoid &snoid = *static_cast<ZmbSnoid *>(feature);
	const SnoidCallback callback = getSnoidCallback(snoid);
	if (callback == SnoidCallback::kSubmitSnoidLureFleenPair)
		processInitialLureOrSubmittedPairEscapeEvent(snoid, eventCode);
	else if (callback != SnoidCallback::kNone)
		processDepartureEvent(snoid, callback, eventCode);
}

void ZoombiniPuzzleFleens::shiftDepartureQueue() {
	if (_submittedPairCount <= 0)
		return;
	for (int16 i = 1; i < _submittedPairCount; i++) {
		_submittedSnoidQueue[i - 1] = _submittedSnoidQueue[i];
		_submittedFleenQueue[i - 1] = _submittedFleenQueue[i];
	}
	_submittedPairCount -= 1;
	_submittedSnoidQueue[_submittedPairCount] = 0;
	_submittedFleenQueue[_submittedPairCount] = -1;
}

void ZoombiniPuzzleFleens::processQueuedDepartureRound() {
	for (int16 queueIndex = 0; queueIndex < _submittedPairCount; queueIndex++) {
		ZmbSnoid *snoid = getSnoid(_submittedSnoidQueue[queueIndex]);
		if (!snoid)
			continue;
		if (queueIndex == _submittedPairCount - 1) {
			_departureWalkPendingCount += 1;
			if (_escapeOverlayFeatures[2])
				manualLinkBefore(snoid, _escapeOverlayFeatures[2]);
			// Keep the authored terminal null frame hidden instead of rematerializing
			// an idle pose at the right screen edge while the departure SFX finishes.
			startSnoidScript(*snoid, SnoidScriptEventType::kQueuedDepartureRoundTail, SnoidCallback::kQueuedDepartureRoundTail, ZmbScrsCompletionMode::kHide);
		} else {
			startSnoidScript(*snoid, SnoidScriptEventType::kQueuedDepartureRoundFollower, SnoidCallback::kNone);
		}
	}
	if (0 < _submittedPairCount)
		_submittedPairCount -= 1;
}

void ZoombiniPuzzleFleens::onPostRenderFrame() {
	if (!_pageActive)
		return;

	if (_boardingStartEnabled) {
		// A separately held Snoid does not delay an idle submitted pair's boarding
		// sequence.
		if (_pendingBoardingSnoidId != 0) {
			startBoardingAnimation();
		} else if (_goInputEnabled && _idleSnoidFidgetStartedCount < _idleSnoidFidgetTargetCount && 0 < _pageLoadedZmbCount &&
				   _idleSnoidFidgetSelectionInterval < getCurrentFrameCounter() - _idleSnoidFidgetLastSelectionFrame) {
			_idleSnoidFidgetLastSelectionFrame = getCurrentFrameCounter();
			bool idleSelectionDone = false;
			do {
				const uint16 poolIndex = _vm->_rnd->getNonRepeatRandom(_pageLoadedZmbCount, _idleSnoidFidgetPoolState);
				ZmbSnoid *snoid = getSnoid(10000 + poolIndex);
				// Only bare idle Snoids may play the autonomous feet-specific celebration.
				// Submitted branch Snoids carry OVERLAY and remain still until their queued shift or descent.
				if (snoid && snoid->isRenderActivated() &&
					snoid->getFlags() == ZmbFeature::FLAG_00000001_TYPE_SNOID &&
					snoid->getAnimState() == kSnoidAnimState000_Idle) {
					if (snoid->getPointLoc().x <= 270)
						startSnoidScript(*snoid, SnoidScriptEventType::kIdleFidget, SnoidCallback::kNone);
					_idleSnoidFidgetStartedCount += 1;
					idleSelectionDone = true;
				}
			} while (!idleSelectionDone);
		}
	}

	// A seven-pair queue relinks the oldest pair after SCRS 7016-7020.
	// It then runs SCRS 7010, SCRS 6000-6004, and SCRS 7011-7015.
	if (_fullQueueEvictionLeadInComplete) {
		_fullQueueEvictionLeadInComplete = false;
		ZmbSnoid *snoid = getSnoid(_evictionSnoidId);
		if (snoid) {
			snoid->addFlag(ZmbFeature::FLAG_04000000_OVERLAY);
			FleenCreature *creature = findFleenBySnoid(snoid->getId());
			if (creature)
				manualLinkAfter(snoid, creature->feature);
			startSnoidScript(*snoid, SnoidScriptEventType::kFullQueueEvictionPairTransition, SnoidCallback::kFullQueueEvictionPairTransition);
		}
	} else if (_fullQueueEvictionPairTransitionComplete) {
		_fullQueueEvictionPairTransitionComplete = false;
		ZmbSnoid *snoid = getSnoid(_evictionSnoidId);
		if (snoid)
			startSnoidScript(*snoid, SnoidScriptEventType::kFullQueueEvictionReject, SnoidCallback::kFullQueueEvictionRejectAndStartFleenAction);
	} else if (_fullQueueEvictionRejectScriptComplete) {
		_fullQueueEvictionRejectScriptComplete = false;
		ZmbSnoid *snoid = getSnoid(_evictionSnoidId);
		if (snoid) {
			// The terminal script moves left and has no facing event of its own.
			snoid->setFacingLeft(true);
			startSnoidScript(*snoid, SnoidScriptEventType::kFullQueueEvictionHideSnoid, SnoidCallback::kFullQueueEvictionHideSnoid, ZmbScrsCompletionMode::kHide);
		}
	}

	if (_departureRoundPending) {
		_departureRoundPending = false;
		processQueuedDepartureRound();
	}

	setGoButtonsEnabled(_allTargetFleensMatched && _goInputEnabled);
}

void ZoombiniPuzzleFleens::onGoButtonActivated() {
	if (!_allTargetFleensMatched || !_goInputEnabled)
		return;
	playDepartSfx();
	schedulePackSnoids(false, true);
	_departureRoundPending = true;
	_departureWalkCompletedCount = 0;
	_departureWalkPendingCount = _submittedPairCount;
	_departureState = ZmbDepartureState::kAnimating;
}

void ZoombiniPuzzleFleens::updateDepartureState() {
	if (_departureState == ZmbDepartureState::kAnimating && isDepartSfxDone() &&
		(_departureWalkPendingCount == 0 || _departureWalkCompletedCount != 0)) {
		_departureState = ZmbDepartureState::kCompleted;
		_goInputEnabled = false;
		executeDeparture();
	}
}

ZmbEventHandleResult ZoombiniPuzzleFleens::onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) {
	if (isDeparturePending()) {
		_departureState = ZmbDepartureState::kCompleted;
		executeDeparture();
		return ZmbEventHandleResult::kConsumed;
	}
	// Keep Help visible in its normal state, but ignore it while a Zoombini is
	// being chased during the page-opening lure or submitted-pair escape.
	if ((!_pairSelectionInputEnabled || !_boardingStartEnabled) && _helpButtonRect.contains(absPos))
		return ZmbEventHandleResult::kConsumed;
	ZmbEventHandleResult result = ZoombiniInteractive::onLButtonDown(absPos, relPos);
	if (result == ZmbEventHandleResult::kConsumed)
		return result;
	if (!_pageActive || !_pairSelectionInputEnabled || isDragging() || 0 < _departureWalkPendingCount || 3 <= _matchedTargetCount)
		return ZmbEventHandleResult::kPassthrough;

	ZmbSnoid *snoid = findSnoidAtPoint(absPos);
	if (!snoid)
		return ZmbEventHandleResult::kPassthrough;
	const SnoidAnimState state = snoid->getAnimState();
	if (state != kSnoidAnimState000_Idle && state != kSnoidAnimState006_Fidget)
		return ZmbEventHandleResult::kPassthrough;
	FleenCreature *creature = findFleenBySnoid(snoid->getId());
	if (!creature || !creature->active || kFleenPosCode20_LuredOffBranch <= creature->posCode)
		return ZmbEventHandleResult::kPassthrough;

	_draggedFleenIndex = static_cast<int16>(creature - _fleenCreatures);
	// Mark the Fleen as picked up.
	// A later invalid drop does not use this state to block another pickup.
	creature->boardingState = 1;
	startSnoidDrag(snoid, absPos);
	return ZmbEventHandleResult::kConsumed;
}

void ZoombiniPuzzleFleens::endDrag(int16 targetSlot) {
	ZmbSnoid *snoid = finishSnoidDrag();
	if (!snoid)
		return;
	const bool fromDrawOnRegSlot = 0 <= _dragSourceDrawOnRegSlot;
	if (0 <= targetSlot && 0 <= _draggedFleenIndex) {
		setDrawOnRegOccupant(targetSlot, snoid->getId());
		// Route a valid DRAW_ON_REG drop through state 4 instead of snapping directly into the page SCRS.
		snoid->setAnimTargetPos(kFleenLurePosition);
		snoid->setAnimState(kSnoidAnimState004_Arrive);
		_pendingBoardingSnoidId = snoid->getId();
		_pendingBoardingFleenIndex = _draggedFleenIndex;
	} else {
		const bool settledAtRelease = settleSnoidAfterTerrainDrop(snoid, _dragOrigPos);

		// A DRAW_ON_REG source returns to an empty pack position only when the
		// terrain result differs from the raw release. Ordinary invalid terrain
		// returns to the exact pickup position selected by the shared controller.
		if (fromDrawOnRegSlot && !settledAtRelease) {
			const Common::Point returnPos = findRandomEmptySnoidSlotPosition(kPackSnoidPositions, ARRAYSIZE(kPackSnoidPositions), 500);
			settleSnoidAtTarget(snoid, returnPos);
		}
	}
	_draggedFleenIndex = -1;
}

void ZoombiniPuzzleFleens::endDrag(const Common::Point &dropPos) {
	(void)dropPos;
	endDrag(_dragHighlightSlot);
}

ZmbChanceInfo ZoombiniPuzzleFleens::debugGetChances() const {
	// The seventh queued submission arms oldest-entry eviction.
	// The player can therefore make at most six lure-path submissions without losing a Zoombini.
	return {ZmbChanceInfo::ZmbChanceType::kSubmit, 6,
			CLIP<int16>(_submittedPairCount, 0, 6),
			"Zoombini sent down the Fleen lure path"};
}

bool ZoombiniPuzzleFleens::debugSetChances(int16 remaining) {
	const int16 opportunities = 6;
	if (remaining < 0 || opportunities < remaining)
		return false;

	const int16 used = opportunities - remaining;
	while (used < _submittedPairCount)
		shiftDepartureQueue();

	const int16 needed = used - _submittedPairCount;
	if (needed <= 0)
		return true;

	uint16 selectedSnoidIds[6] = {};
	int16 selectedFleenIndices[6] = {};
	int16 selectedCount = 0;
	for (int16 creatureIndex = 0;
		 creatureIndex < _activeFleenCount && selectedCount < needed;
		 creatureIndex++) {
		FleenCreature &creature = _fleenCreatures[creatureIndex];
		ZmbSnoid *snoid = getSnoid(creature.pairedSnoidId);
		if (!snoid || !creature.active || creature.boardingState != 0 ||
			!snoid->isRenderActivated() || !snoid->_packIsOccupied ||
			snoid->getAnimState() != kSnoidAnimState000_Idle)
			continue;

		bool alreadyQueued = snoid->getId() == _pendingBoardingSnoidId || snoid->getId() == _boardingSnoidId;
		for (int16 queueIndex = 0; !alreadyQueued && queueIndex < _submittedPairCount; queueIndex++)
			alreadyQueued = _submittedSnoidQueue[queueIndex] == snoid->getId();
		for (int16 selectedIndex = 0; !alreadyQueued && selectedIndex < selectedCount; selectedIndex++)
			alreadyQueued = selectedSnoidIds[selectedIndex] == snoid->getId();
		if (alreadyQueued)
			continue;

		selectedSnoidIds[selectedCount] = snoid->getId();
		selectedFleenIndices[selectedCount] = creatureIndex;
		selectedCount += 1;
	}

	if (selectedCount < needed)
		return false;

	for (int16 selectedIndex = 0; selectedIndex < selectedCount; selectedIndex++) {
		_submittedSnoidQueue[_submittedPairCount] = selectedSnoidIds[selectedIndex];
		_submittedFleenQueue[_submittedPairCount] = selectedFleenIndices[selectedIndex];
		_submittedPairCount += 1;
	}

	// Use one real pair for the same boarding feedback used by a normal submission.
	// The remaining entries are already valid departure pairs and will be consumed by Go.
	if (_boardingStartEnabled && _pendingBoardingSnoidId == 0 && !isDragging()) {
		const int16 animationIndex = selectedCount - 1;
		ZmbSnoid *snoid = getSnoid(selectedSnoidIds[animationIndex]);
		if (snoid) {
			snoid->setPointLoc(kFleenLurePosition);
			snoid->setAnimState(kSnoidAnimState000_Idle);
			snoid->setupIdleHotspots();
			beginBoardingAnimation(*snoid, selectedFleenIndices[animationIndex], false);
		}
	}
	return true;
}

Common::String ZoombiniPuzzleFleens::debugGetAnswer() const {
	Common::String answer = getDebugBanner();
	answer += Common::String::format(", lured %d/3:\n", _matchedTargetCount);

	answer += "  Zoombini-to-Fleen trait mapping:\n";
	for (int traitIndex = 0; traitIndex < 4; traitIndex++) {
		const ZmbTrait::TraitKind zmbTraitKind = ZmbTrait::traitKindFromIndex(traitIndex);
		const ZmbTrait::TraitKind fleenTraitKind = _traitDestinationKinds[traitIndex];
		const byte rotation = _traitValueRotations[traitIndex];
		answer += Common::String::format("    [Z] %s - [F] %s\n",
										 ZmbTrait::debugTraitKindName(zmbTraitKind), ZmbTrait::debugTraitKindName(fleenTraitKind));
		for (int val = 1; val <= 5; val++) {
			const byte fleenVal = static_cast<byte>((rotation + val - 2) % 5 + 1);
			answer += Common::String::format("      %-10s -> %s\n",
											 ZmbTrait::debugTraitValueName(zmbTraitKind, val), FleenTrait::debugTraitValueName(fleenTraitKind, fleenVal));
		}
	}

	answer += "  Hive Fleens and their paired Zoombinis:\n";
	for (int16 fleenIdx = 0; fleenIdx < _activeFleenCount; fleenIdx++) {
		const FleenCreature &creature = _fleenCreatures[fleenIdx];
		if (!creature.isTarget)
			continue;
		int hiveSlot = -1;
		for (uint slot = 0; slot < _targetBranchPositions.size(); slot++) {
			if (creature.assignedBranchPos.x == _targetBranchPositions[slot].x && creature.assignedBranchPos.y == _targetBranchPositions[slot].y) {
				hiveSlot = static_cast<int>(slot) + 1;
				break;
			}
		}
		const ZmbSnoid *snoid = getSnoid(creature.pairedSnoidId);
		if (!snoid)
			continue;
		answer += Common::String::format("    Hive Fleen %d:\n", hiveSlot);
		answer += Common::String::format("      %s\n", creature.toStr().c_str());
		answer += Common::String::format("      %s\n", snoid->toStr().c_str());
		if (kFleenPosCode20_LuredOffBranch <= creature.posCode)
			answer += "      Status: cleared\n";
	}
	return answer;
}

Common::String ZoombiniPuzzleFleens::debugGetBuiltinDebugCommandHelp() const {
	Common::String output;
	output += Common::String::format("  %-3s (%s)\n", "L/l", kBuiltinDebugActionLevel);
	output += "    Show the one-based Route 3 difficulty as Level 1 through Level 4 without changing any puzzle state.\n";
	return output;
}

bool ZoombiniPuzzleFleens::debugDoBuiltinDebugCommand(int argc, const char **argv, Common::String &output) {
	if (argc != 3) {
		output = Common::String::format("Usage: page builtin_debug %s\n", kBuiltinDebugActionLevel);
		return true;
	}
	const BuiltinDebugAction action = parseBuiltinDebugAction(argv[2]);
	if (action == BuiltinDebugAction::kInvalid) {
		output = Common::String::format("Usage: page builtin_debug %s\n", kBuiltinDebugActionLevel);
		return true;
	}
	return runBuiltinDebugAction(action, output);
}

ZoombiniPuzzleFleens::BuiltinDebugAction ZoombiniPuzzleFleens::parseBuiltinDebugAction(const Common::String &action) {
	if (action.equalsIgnoreCase(kBuiltinDebugActionLevel))
		return BuiltinDebugAction::kLevel;
	return BuiltinDebugAction::kInvalid;
}

bool ZoombiniPuzzleFleens::runBuiltinDebugAction(BuiltinDebugAction action, Common::String &output) {
	if (action != BuiltinDebugAction::kLevel)
		return true;
	output = Common::String::format("Level %d\n", static_cast<int>(_difficultyLevel));
	showBuiltinDebugText(Common::String::format("Level %d", static_cast<int>(_difficultyLevel)));
	return false;
}

ZmbEventHandleResult ZoombiniPuzzleFleens::onDebugKeyDown(const Common::KeyState &kbd) {
	if (!(kbd.hasFlags(0) || kbd.hasFlags(Common::KBD_SHIFT)) ||
		(kbd.ascii != 'L' && kbd.ascii != 'l'))
		return ZmbEventHandleResult::kPassthrough;

	Common::String output;
	runBuiltinDebugAction(BuiltinDebugAction::kLevel, output);
	return ZmbEventHandleResult::kConsumed;
}

} // End of namespace Mohawk
