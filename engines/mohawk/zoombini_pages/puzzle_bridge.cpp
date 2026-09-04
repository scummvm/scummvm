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

#include "mohawk/mohawk.h"
#include "mohawk/sound.h"

#include "mohawk/zoombini.h"
#include "mohawk/zoombini_graphics.h"
#include "mohawk/zoombini_pages/puzzle_bridge.h"
#include "mohawk/zoombini_random.h"
#include "mohawk/zoombini_sound.h"
#include "mohawk/zoombini_state.h"

namespace Mohawk {

constexpr Common::Point ZoombiniPuzzleBridge::kLeftBankWaitingPositions[16];
constexpr Common::Point ZoombiniPuzzleBridge::kLaneEntryPositions[2];
constexpr Common::Point ZoombiniPuzzleBridge::kUpperLaneArrivalPositions[16];
constexpr Common::Point ZoombiniPuzzleBridge::kLowerLaneArrivalPositions[16];
constexpr ZoombiniPuzzleBridge::TraitValueSet ZoombiniPuzzleBridge::kLevel2AlternativeValueTable[10];
constexpr ZmbTrait::TraitKind ZoombiniPuzzleBridge::kLevel4ExcludedTraitTable[4];
const ZoombiniPuzzleBridge::TraitRuleDescriptor ZoombiniPuzzleBridge::kLevel3BaseTable[6] = {
	TraitRuleDescriptor::withFirstValue(ZmbTrait::kTraitFeet, 1),
	TraitRuleDescriptor::withFirstValue(ZmbTrait::kTraitFeet, 1),
	TraitRuleDescriptor::withFirstValue(ZmbTrait::kTraitFeet, 1),
	TraitRuleDescriptor::withFirstValue(ZmbTrait::kTraitNose, 1),
	TraitRuleDescriptor::withFirstValue(ZmbTrait::kTraitNose, 1),
	TraitRuleDescriptor::withFirstValue(ZmbTrait::kTraitEyes, 1),
};
const ZoombiniPuzzleBridge::TraitRuleDescriptor ZoombiniPuzzleBridge::kLevel3StepTable[6] = {
	TraitRuleDescriptor::withFirstValue(ZmbTrait::kTraitNose, 1),
	TraitRuleDescriptor::withFirstValue(ZmbTrait::kTraitEyes, 1),
	TraitRuleDescriptor::withFirstValue(ZmbTrait::kTraitHair, 1),
	TraitRuleDescriptor::withFirstValue(ZmbTrait::kTraitEyes, 1),
	TraitRuleDescriptor::withFirstValue(ZmbTrait::kTraitHair, 1),
	TraitRuleDescriptor::withFirstValue(ZmbTrait::kTraitHair, 1),
};

ZoombiniPuzzleBridge::TraitRuleDescriptor ZoombiniPuzzleBridge::TraitRuleDescriptor::withFirstValue(ZmbTrait::TraitKind traitKind, byte traitValue) {
	TraitRuleDescriptor descriptor;
	descriptor.setFirstValue(traitKind, traitValue);
	return descriptor;
}

ZoombiniPuzzleBridge::TraitRuleDescriptor ZoombiniPuzzleBridge::TraitRuleDescriptor::scaledFirstValues(byte scale) const {
	TraitRuleDescriptor descriptor = *this;
	descriptor.hairValues.firstValue = static_cast<byte>(descriptor.hairValues.firstValue * scale);
	descriptor.eyesValues.firstValue = static_cast<byte>(descriptor.eyesValues.firstValue * scale);
	descriptor.noseValues.firstValue = static_cast<byte>(descriptor.noseValues.firstValue * scale);
	descriptor.feetValues.firstValue = static_cast<byte>(descriptor.feetValues.firstValue * scale);
	return descriptor;
}

void ZoombiniPuzzleBridge::TraitRuleDescriptor::addFirstValues(const TraitRuleDescriptor &other) {
	hairValues.firstValue = static_cast<byte>(hairValues.firstValue + other.hairValues.firstValue);
	eyesValues.firstValue = static_cast<byte>(eyesValues.firstValue + other.eyesValues.firstValue);
	noseValues.firstValue = static_cast<byte>(noseValues.firstValue + other.noseValues.firstValue);
	feetValues.firstValue = static_cast<byte>(feetValues.firstValue + other.feetValues.firstValue);
}

ZoombiniPuzzleBridge::TraitValueSet ZoombiniPuzzleBridge::TraitRuleDescriptor::getValues(ZmbTrait::TraitKind traitKind) const {
	switch (traitKind) {
	case ZmbTrait::kTraitHair:
		return hairValues;
	case ZmbTrait::kTraitEyes:
		return eyesValues;
	case ZmbTrait::kTraitNose:
		return noseValues;
	case ZmbTrait::kTraitFeet:
		return feetValues;
	default:
		error("bridge: TraitRuleDescriptor::getValues: invalid trait kind %d", static_cast<int>(traitKind));
		return ZoombiniPuzzleBridge::TraitValueSet();
	}
}

void ZoombiniPuzzleBridge::TraitRuleDescriptor::setFirstValue(ZmbTrait::TraitKind traitKind, byte traitValue) {
	const TraitValueSet values = {traitValue, 0};
	setValues(traitKind, values);
}

void ZoombiniPuzzleBridge::TraitRuleDescriptor::setValues(ZmbTrait::TraitKind traitKind, const TraitValueSet &values) {
	switch (traitKind) {
	case ZmbTrait::kTraitHair:
		hairValues = values;
		break;
	case ZmbTrait::kTraitEyes:
		eyesValues = values;
		break;
	case ZmbTrait::kTraitNose:
		noseValues = values;
		break;
	case ZmbTrait::kTraitFeet:
		feetValues = values;
		break;
	default:
		error("bridge: TraitRuleDescriptor::setValues: invalid trait kind %d", static_cast<int>(traitKind));
		break;
	}
}

ZmbTrait::TraitKind ZoombiniPuzzleBridge::traitKindFromRuleSlot(int16 slot) {
	static constexpr ZmbTrait::TraitKind kRuleSlotTraitKinds[4] = {
		ZmbTrait::kTraitFeet,
		ZmbTrait::kTraitNose,
		ZmbTrait::kTraitEyes,
		ZmbTrait::kTraitHair,
	};
	return kRuleSlotTraitKinds[slot];
}

int16 ZoombiniPuzzleBridge::requirementTraitKindCode(ZmbTrait::TraitKind traitKind) {
	switch (traitKind) {
	case ZmbTrait::kTraitHair:
		return 1;
	case ZmbTrait::kTraitEyes:
		return 2;
	case ZmbTrait::kTraitNose:
		return 3;
	case ZmbTrait::kTraitFeet:
		return 4;
	default:
		return 0;
	}
}

bool ZoombiniPuzzleBridge::traitMatchesDescriptor(const ZmbTrait &trait, const TraitRuleDescriptor &descriptor,
												  bool matchSecondValues) {
	for (int16 traitIdx = 0; traitIdx < 4; traitIdx++) {
		const ZmbTrait::TraitKind traitKind = ZmbTrait::traitKindFromIndex(traitIdx);
		const byte traitValue = trait.getTraitValue(traitKind);
		const TraitValueSet acceptedValues = descriptor.getValues(traitKind);
		if (traitValue == acceptedValues.firstValue)
			return true;

		if (matchSecondValues && traitValue == acceptedValues.secondValue)
			return true;
	}

	return false;
}

ZoombiniPuzzleBridge::ZoombiniPuzzleBridge(MohawkEngine_Zoombini *vm) : ZoombiniPuzzle(vm, ZoombiniPageType::kBridge, ZmbSrcPageKind::kBridge_02) {
}

ZoombiniPuzzleBridge::~ZoombiniPuzzleBridge() {
}

const ZoombiniPage::ScriptSoundPriorityRanges &ZoombiniPuzzleBridge::getScriptSoundPriorityRanges() const {
	// Original shared page dispatch automatically prepends SND 996-997 at priority 32.
	static const ZoombiniPage::ScriptSoundPriorityRanges kRanges = {
		{kSysResSoundRange0996_DepartSFX, kSysResSoundRange0997_ArriveSFX},
		{kSysResSoundRange20000_Base, kSysResSoundRange29999_Last},
		{kResSoundRange1200_SystemRangeBase, kResSoundRange1201_SystemRangeLast},
		{kResSoundRange1000_SystemRangeBase, kResSoundRange1099_SystemRangeLast},
		{kResSoundRange1216_SystemRangeBase, kResSoundRange1226_SystemRangeLast},
		{kResSoundRange1202_SystemRangeBase, kResSoundRange1213_SystemRangeLast},
		{kResSoundRange1214_SystemRangeBase, kResSoundRange1215_SystemRangeLast},
		{kResSoundRange0175_PageRangeBase, kResSoundRange0199_PageRangeLast}};
	return kRanges;
}

const Common::Rect &ZoombiniPuzzleBridge::getDragConstraintRect() const {
	return _dragConstraintRect;
}

void ZoombiniPuzzleBridge::open() {
	openArchive(ZMB_MHK_BRIDGE);
}

void ZoombiniPuzzleBridge::setBackgroundMusic() {
	// Bridge has no dedicated BGM.
	// Ambient audio comes from water/cliff SCRS animations.
}

void ZoombiniPuzzleBridge::setBackgroundBitmap() {
	_vm->_gfx->setPalette(kResBackground1000);
	_vm->_gfx->drawBackground(kResBackground1000);
}

void ZoombiniPuzzleBridge::initStates() {
	// Bridge lane helpers use a wider drop square than the common default.
	_clickZoneRadius = 55;

	_goButtonEnabled = false;
	_allergyRuleEntryCount = 0;
	memset(_allergyRuleTraitKindCodes, 0, sizeof(_allergyRuleTraitKindCodes));
	memset(_allergyRuleTraitValues, 0, sizeof(_allergyRuleTraitValues));
	_matchingTraitsUseUpperLane = false;
	_failedCrossingCount = 0;
	_activeCrossingCount = 0;

	_queuedCrossingStartEnabled = true;
	_activeCrossingRejected = false;
	_activeCrossingLane = BridgeLane::kNone00;
	_pendingDropCount = 0;
	_pendingDropLanes[0] = BridgeLane::kNone00;
	_pendingDropLanes[1] = BridgeLane::kNone00;
	memset(_pendingDropSnoidIds, 0, sizeof(_pendingDropSnoidIds));
	_dragRemovedOldestPendingDrop = false;
	memset(_pendingDropRejected, 0, sizeof(_pendingDropRejected));
	memset(_upperLaneArrivedSnoidIds, 0, sizeof(_upperLaneArrivedSnoidIds));
	memset(_lowerLaneArrivedSnoidIds, 0, sizeof(_lowerLaneArrivedSnoidIds));
	_upperLaneArrivalCount = 0;
	_lowerLaneArrivalCount = 0;
	_activeRejectScrsId = -1;
	_pendingRejectReactionCode = RejectReactionCode::kNone00;
	_pendingCliffLaneFeatureId = 0;
	_rejectCrossingSnoidId = 0;
	_lastRejectReactionCode = RejectReactionCode::kNotRecordedMinus01;
	_lastRejectSnoidScrsId = -1;
	_rejectedSnoidPickupEnabled = false;
	_terminalCliffTransitionPending = false;
	_celebrationTargetCount = 0;
	_celebrationStartedCount = 0;
	_lastCelebrationFrame = 0;
	_celebrationFrameInterval = 60;
	_celebrationUsedSnoidMask = 0;
}

void ZoombiniPuzzleBridge::loadFeatures() {
	// Preload images (feature groups)
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape1100));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape1200));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape1300));

	// Load terrain barrier bitmap (tBMP 1600) for walkability checks.
	// The 160x120 mask marks walkable pixels with value 1.
	loadTerrainBitmap(kResBitmapTerrain1600);

	// Register each SCRB image group without creating hidden feature runners.
	// The complete 1200 group is needed because rejection and entrance scripts are loaded lazily.
	registerScrbImageGroup(ZmbResource(ZmbResource::kPage, kResBitmapShape1100), kResScrb1100_Main, 7);
	registerScrbImageGroup(ZmbResource(ZmbResource::kPage, kResBitmapShape1200), kResScrb1200_CliffLane1, 49);
	registerScrbImageGroup(ZmbResource(ZmbResource::kPage, kResBitmapShape1300), kResScrb1300_Segment0, 2);

	// [*] SCRS group 0: 20 throw scripts (SCRS 1000-1019, state 9 renderer)
	registerScrsGroup(kResScrs1000_RejectBase, kBridgeRejectScrsCount);

	// [*] SCRS group 1: 25 crossing/celebration scripts (SCRS 2000-2024, state 8 renderer)
	registerScrsGroup(kResScrs2000_NormalBase, kBridgeNormalScrsCount);

	// [*] Bridge segment SCRB features at predefined positions (2 entries)
	for (uint16 i = 0; i < 2; i++) {
		loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1300), kResScrb1300_Segment0 + i, 7, kLaneEntryPositions[i],
						ZmbFeature::FLAG_00002000_DRAW_ON_REG | ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM |
							ZmbFeature::FLAG_01000000_DEFER_RENDER);
	}

	// [*] Cliff gate SCRB (SCRB 1105 = 0x451): main cliff feature
	loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1100), kResScrb1105_Overlay, 6,
					ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00040000_CHAIN_SCRIPT | ZmbFeature::FLAG_00080000_DEFER_ANIM |
						ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_01000000_DEFER_RENDER | ZmbFeature::FLAG_08000000_REGION_TRACK);

	// [*] Cliff animations (SCRB 1202=0x4B2, 1201=0x4B1, 1200=0x4B0)
	loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1200), kResScrb1202_CliffGate, 6,
					ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_08000000_REGION_TRACK);

	loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1200), kResScrb1201_CliffLane2, 6,
					ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_08000000_REGION_TRACK);

	loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1200), kResScrb1200_CliffLane1, 6,
					ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_08000000_REGION_TRACK);

	// [*] Overlay SCRBs (1100-1105, except 1103 handled specially)
	// Register SCRB 1100-1105 as flagless runners, except SCRB 1103 which uses PLAY_ONCE.
	// SCRB 1105 needs a separate static runner in addition to the animated cliff-gate runner.
	// The flagless runners are position-sorted on their first frame.
	// They are then promoted to the order-stable OVERLAY bucket.
	// This places the small left-bank tree from SCRB 1100 in front of the seated pack Snoids
	// at the lower-left pedestals.
	for (int16 i = kResScrb1100_Main; i <= kResScrb1105_Overlay; i++) {
		if (i == kResScrb1103_Overlay) {
			// Water overlay with PLAY_ONCE flag
			loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1100), i, 0, ZmbFeature::FLAG_00100000_PLAY_ONCE);
		} else {
			// Static overlays (1100 = left-bank small tree + cliff props,
			// 1101 = left-bank thin tree, 1102 = right-bank oak tree,
			// 1104 = right-bank cup decoration, 1105 = cliff gate statics).
			loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1100), i, 0, ZmbFeature::FLAG_00000000_TYPE_SHAPES);
		}
	}

	// [*] Water animation SCRB 1106 (0x452)
	loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1100), kResScrb1106_Water, 0, ZmbFeature::FLAG_00008000_LOOP_ANIM);

	// Load active-pack Zoombinis into the left-bank waiting layout.
	loadZoombinisFromPack(kLeftBankWaitingPositions, ARRAYSIZE(kLeftBankWaitingPositions));

	// Apply 75/25 walk-in split and stagger timing.
	layoutStaticAndWalkIn(0, false);
	// Materialize the authored runner order before assigning walk-in deadlines
	// and before the rule generator snapshots the active Snoid traits.
	renderFeatures();
	assignStaggeredWalkDelays(0, 45);

	// Build the trait toll table
	buildTraitTollTable();

	// Store feature handles for cliff animation manipulation.
	_cliffLane1FeatureId = kResScrb1200_CliffLane1;
	_cliffLane2FeatureId = kResScrb1201_CliffLane2;
	_cliffGateFeatureId = kResScrb1202_CliffGate;
	_cliffMainFeatureId = kResScrb1105_Overlay;
	_waterOverlayFeatureId = kResScrb1103_Overlay;

	// [*] Buttons: Go, Map, Help
	// Map button (buttonIdx 1): shapes 5/6 from bridge SHPL
	// Go button (buttonIdx 2): shapes 1(disabled)/2(enabled)/3(pressed) from bridge SHPL
	// Help button (buttonIdx 3): system shapes 24/25
	configureStandardPuzzleControlRects();
	loadStandardPuzzleControlFeatures(kResBitmapShape1400_GoMapButtons);

	// The active-pack bit is only an input to the initial layout/rule build.
	// A puzzle starts with no accepted Snoids.
	schedulePackSnoids(false, false);

	// Shared page cleanup installs system SND 996-997 as the first priority range before Bridge appends its page ranges.
	queueScriptSoundForNextRenderPass(ZmbResource(ZmbResource::kSystem, kSysResSound0997_MoveSFX));
}

void ZoombiniPuzzleBridge::initHelpPrompt() {
	// Allergic Cliffs intentionally has no page-owned F1 help prompt.
	_activeHelpSoundId = ZmbResource();
}

void ZoombiniPuzzleBridge::debugPrepareForDeparture() {
	_goButtonEnabled = true;
}

ZmbChanceInfo ZoombiniPuzzleBridge::debugGetChances() const {
	// The sixth failed crossing exhausts all six bridge pegs.
	return {ZmbChanceInfo::ZmbChanceType::kMistake, 6, CLIP<int16>(_failedCrossingCount, 0, 6), "rejected cliff crossing"};
}

bool ZoombiniPuzzleBridge::debugSetChances(int16 remaining) {
	const int16 opportunities = 6;
	if (remaining < 0 || opportunities < remaining)
		return false;

	_failedCrossingCount = opportunities - remaining;
	refreshPegDisplay();
	return true;
}

Common::String ZoombiniPuzzleBridge::debugGetAnswer() const {
	// Bridge assignment: @ref ZoombiniPuzzleBridge::testTraitMatch() returns true
	// when the Zoombini is rejected from the target lane.
	// Without a lane swap, matching traits cross the bottom bridge and
	// non-matching traits cross the top bridge.
	// A lane swap reverses those destinations.
	const char *matchAtLeast = "matches at least one listed trait";
	const char *matchNone = "matches none of the listed traits";
	const char *upperLaneDesc = _matchingTraitsUseUpperLane ? matchAtLeast : matchNone;
	const char *lowerLaneDesc = _matchingTraitsUseUpperLane ? matchNone : matchAtLeast;

	Common::String s = getDebugBanner();
	s += "\n";
	s += "  Cliff allergy (one matching trait is enough):\n";
	if (_allergyRuleEntryCount == 0) {
		s += "    (none)\n";
	} else {
		for (int i = 0; i < _allergyRuleEntryCount && i < 5; i++) {
			const byte traitKind = _allergyRuleTraitKindCodes[i];
			const byte traitValue = _allergyRuleTraitValues[i];
			const int kindIdx = static_cast<int>(traitKind) - 1;
			const ZmbTrait::TraitKind kind = ZmbTrait::traitKindFromIndex(kindIdx);
			const char *name = ZmbTrait::debugTraitKindName(kind);
			s += Common::String::format("    - %s: %s\n", name, ZmbTrait::debugTraitValueName(kind, traitValue));
		}
	}
	s += "  Destinations:\n";
	s += Common::String::format("    Top    Bridge: %s\n", upperLaneDesc);
	s += Common::String::format("    Bottom Bridge: %s\n", lowerLaneDesc);
	return s;
}

Common::String ZoombiniPuzzleBridge::debugGetLastRejectRoute() const {
	Common::String s = getDebugBanner();
	s += "\n";
	s += Common::String::format("  Type: %d\n", static_cast<int16>(_lastRejectReactionCode));
	s += Common::String::format("  Snoid Script: %d\n", _lastRejectSnoidScrsId);
	return s;
}

Common::String ZoombiniPuzzleBridge::debugGetBuiltinDebugCommandHelp() const {
	Common::String output;
	output += Common::String::format("  %-3s (%s)\n", "A/a", kBuiltinDebugActionAcceptance);
	output += "    Draw the lane that accepts any listed trait and its rule sprites; the opposite lane accepts the complement.\n";
	output += "    This display does not regenerate the rule and is unavailable while a pack path walk is pending.\n";
	output += Common::String::format("  %-3s (%s)\n", "R/r", kBuiltinDebugActionRoute);
	output += "    Show the last rejection event type and selected page SCRS 1000-1019; both are -1 before the first rejection.\n";
	output += "    The display waits for the next key or mouse button, does not change the rule, and is unavailable during a pack path walk.\n";
	return output;
}

bool ZoombiniPuzzleBridge::debugDoBuiltinDebugCommand(int argc, const char **argv, Common::String &output) {
	if (argc != 3) {
		output = Common::String::format("Usage: page builtin_debug <%s|%s>\n", kBuiltinDebugActionAcceptance, kBuiltinDebugActionRoute);
		return true;
	}

	const BuiltinDebugAction action = parseBuiltinDebugAction(argv[2]);
	if (action == BuiltinDebugAction::kInvalid) {
		output = Common::String::format("Usage: page builtin_debug <%s|%s>\n", kBuiltinDebugActionAcceptance, kBuiltinDebugActionRoute);
		return true;
	}
	return runBuiltinDebugAction(action, output);
}

ZoombiniPuzzleBridge::BuiltinDebugAction ZoombiniPuzzleBridge::parseBuiltinDebugAction(const Common::String &action) {
	if (action.equalsIgnoreCase(kBuiltinDebugActionAcceptance))
		return BuiltinDebugAction::kAcceptance;
	if (action.equalsIgnoreCase(kBuiltinDebugActionRoute))
		return BuiltinDebugAction::kRoute;
	return BuiltinDebugAction::kInvalid;
}

bool ZoombiniPuzzleBridge::runBuiltinDebugAction(BuiltinDebugAction action, Common::String &output) {
	if (action == BuiltinDebugAction::kRoute)
		output = debugGetLastRejectRoute();
	else
		output = debugGetAnswer();

	if (action == BuiltinDebugAction::kRoute) {
		showBuiltinDebugText(Common::String::format(" Type: %d Snoid Script: %d", static_cast<int16>(_lastRejectReactionCode), _lastRejectSnoidScrsId), true);
	} else {
		showBuiltinDebugText(_matchingTraitsUseUpperLane ? "Upper bridge accepts:" : "Lower bridge accepts:");
		for (byte ruleIdx = 0; ruleIdx < _allergyRuleEntryCount; ruleIdx++)
			drawBuiltinDebugTraitSprite(Common::Point(250 + 30 * ruleIdx, 20), _allergyRuleTraitKindCodes[ruleIdx], _allergyRuleTraitValues[ruleIdx]);
	}
	return false;
}

ZmbEventHandleResult ZoombiniPuzzleBridge::onDebugKeyDown(const Common::KeyState &kbd) {
	BuiltinDebugAction action = BuiltinDebugAction::kInvalid;
	if (kbd.hasFlags(0) || kbd.hasFlags(Common::KBD_SHIFT)) {
		if (kbd.ascii == 'A' || kbd.ascii == 'a')
			action = BuiltinDebugAction::kAcceptance;
		else if (kbd.ascii == 'R' || kbd.ascii == 'r')
			action = BuiltinDebugAction::kRoute;
	}
	if (action == BuiltinDebugAction::kInvalid)
		return ZmbEventHandleResult::kPassthrough;

	// The authored debug handler is inactive while a pack path walk is pending.
	for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
		if (!(*it)->isPackSnoid())
			continue;
		const SnoidAnimState animState = (*it)->getAnimState();
		if (animState == kSnoidAnimState007_Depart || animState == kSnoidAnimState112_Path)
			return ZmbEventHandleResult::kPassthrough;
	}

	Common::String output;
	runBuiltinDebugAction(action, output);
	if (action == BuiltinDebugAction::kAcceptance)
		return ZmbEventHandleResult::kConsumed;
	return ZmbEventHandleResult::kPassthrough;
}

void ZoombiniPuzzleBridge::onGoButtonActivated() {
	// Play departing SFX and start walk-off animation, then fade out when SFX finishes.
	if (_goButtonEnabled) {
		markAcceptedSnoidsForDeparture();

		// Walk to (680, 316) with a 45-frame stagger.
		startDepartWalkAnimation(Common::Point(680, 316));
		ZoombiniInteractive::onGoButtonActivated();
	}
}

int16 ZoombiniPuzzleBridge::collectZmbTraits(Common::Array<ZmbTrait> &outTraits) const {
	outTraits.clear();
	Common::Array<ZmbSnoid *> snoids;
	collectPackSnoids(snoids, true);
	for (const ZmbSnoid *snoid : snoids)
		outTraits.push_back(snoid->_trait);

	return outTraits.size();
}

void ZoombiniPuzzleBridge::buildTraitTollTable() {
	// Build a pool of all valid toll combinations for the current difficulty level,
	// counts how many Zoombinis match each combo, and selects the one closest to half.

	// Allocate readable rule candidates and match count arrays.
	Common::Array<TraitRuleDescriptor> comboPool;
	Common::Array<uint16> matchCounts;

	// Collect Zoombini traits before laying out their matcher bytes explicitly.
	Common::Array<ZmbTrait> zmbTraits;
	collectZmbTraits(zmbTraits);

	uint32 poolSize = 0;

	switch (_difficultyLevel) {
	case kPuzzleLevel1: {
		// Level 1: Generate 20 single-trait combinations (5 values x 4 kinds).
		poolSize = 20;
		comboPool.resize(poolSize);
		uint32 comboPoolIdx = 0;
		for (int16 ruleSlot = 0; ruleSlot < 4; ruleSlot++) {
			const ZmbTrait::TraitKind traitKind = traitKindFromRuleSlot(ruleSlot);
			for (byte traitValue = 1; traitValue <= 5; traitValue += 1) {
				comboPool[comboPoolIdx] = TraitRuleDescriptor::withFirstValue(traitKind, traitValue);
				comboPoolIdx += 1;
			}
		}
		break;
	}
	case kPuzzleLevel2: {
		// Level 2: Generate 40 two-value combinations from 4 trait categories and 10 value pairs.
		poolSize = 40;
		comboPool.resize(poolSize);
		uint32 comboPoolIdx = 0;
		for (uint32 groupIdx = 0; groupIdx < 4; groupIdx++) {
			for (uint32 subComboIdx = 0; subComboIdx < 10; subComboIdx++) {
				TraitRuleDescriptor descriptor;
				descriptor.setValues(traitKindFromRuleSlot(static_cast<int16>(groupIdx)), kLevel2AlternativeValueTable[subComboIdx]);
				comboPool[comboPoolIdx] = descriptor;
				comboPoolIdx += 1;
			}
		}
		break;
	}
	case kPuzzleLevel3: {
		// Level 3: Generate 150 arithmetic combinations from 6 groups x 5 steps x 5 values.
		poolSize = 150;
		comboPool.resize(poolSize);
		uint32 comboPoolIdx = 0;
		for (uint32 groupIdx = 0; groupIdx < 6; groupIdx++) {
			for (uint32 stepIdx = 1; stepIdx <= 5; stepIdx++) {
				const TraitRuleDescriptor stepDescriptor = kLevel3StepTable[groupIdx].scaledFirstValues(static_cast<byte>(stepIdx));
				for (uint32 valueIdx = 1; valueIdx <= 5; valueIdx++) {
					TraitRuleDescriptor descriptor = kLevel3BaseTable[groupIdx].scaledFirstValues(static_cast<byte>(valueIdx));
					descriptor.addFirstValues(stepDescriptor);
					comboPool[comboPoolIdx] = descriptor;
					comboPoolIdx += 1;
				}
			}
		}
		break;
	}
	case kPuzzleLevel4: {
		// Level 4: Generate 500 three-trait combinations from 4 groups x 125 permutations.
		poolSize = 500;
		comboPool.resize(poolSize);
		uint32 groupOffset = 0;

		for (uint32 groupIdx = 0; groupIdx < 4; groupIdx++) {
			const ZmbTrait::TraitKind excludedTraitKind = kLevel4ExcludedTraitTable[groupIdx];
			for (uint32 permIdx = 0; permIdx < 125; permIdx++) {
				TraitRuleDescriptor descriptor;
				uint32 remainingPermutation = permIdx;
				for (int16 ruleSlot = 0; ruleSlot < 4; ruleSlot++) {
					const ZmbTrait::TraitKind traitKind = traitKindFromRuleSlot(ruleSlot);
					if (traitKind == excludedTraitKind)
						continue;
					const byte traitValue = static_cast<byte>(remainingPermutation % 5 + 1);
					descriptor.setFirstValue(traitKind, traitValue);
					remainingPermutation /= 5;
				}
				comboPool[permIdx + groupOffset] = descriptor;
			}
			groupOffset += 125;
		}
		break;
	}
	default:
		poolSize = 20;
		comboPool.resize(poolSize);
		break;
	}

	// Count how many Zoombinis match each generated combination in the pool.
	matchCounts.resize(poolSize);
	for (uint32 comboIdx = 0; comboIdx < poolSize; comboIdx++)
		matchCounts[comboIdx] = 0;

	// Level 2 also tests the second value in each pair.
	const bool matchSecondValues = _difficultyLevel == kPuzzleLevel2;
	for (uint32 zmbIdx = 0; zmbIdx < zmbTraits.size(); zmbIdx++) {
		const ZmbTrait &trait = zmbTraits[zmbIdx];

		for (uint32 comboIdx = 0; comboIdx < poolSize; comboIdx++) {
			const TraitRuleDescriptor &descriptor = comboPool[comboIdx];
			if (traitMatchesDescriptor(trait, descriptor, matchSecondValues))
				matchCounts[comboIdx] += 1;
		}
	}

	// Spiral search from total/2 to find a match count that is actually available in the pool.
	int32 total = zmbTraits.size();
	int32 target = total / 2;
	int32 step = 1;
	int found = 0;
	int16 chosenMatchCount = 0;

	while (!found) {
		if (0 < target && target < 16) {
			for (uint32 comboIdx = 0; comboIdx < poolSize; comboIdx++) {
				if (matchCounts[comboIdx] == static_cast<uint32>(target))
					found += 1;
			}
			chosenMatchCount = static_cast<int16>(target);
		}
		target += step;
		step = -(step + 1); // Alternates sequence to drift slowly around the center.
	}

	// Randomly select one combination from all those that have the chosen match count.
	int pick = _vm->_rnd->getRandomNumber(1, found);
	TraitRuleDescriptor targetDescriptor;
	for (uint32 comboIdx = 0; comboIdx < poolSize; comboIdx++) {
		if (matchCounts[comboIdx] == static_cast<uint16>(chosenMatchCount)) {
			pick -= 1;
			if (pick == 0) {
				targetDescriptor = comboPool[comboIdx];
				break;
			}
		}
	}

	// Keep a Level 1 split count available to Tunnels only when this pool had one matching rule.
	_vm->_bridgeTunnelsLevel1Exclusion.clear();
	if (_difficultyLevel == kPuzzleLevel1 && found == 1)
		_vm->_bridgeTunnelsLevel1Exclusion.setSplitCount(chosenMatchCount);

	// Decode the selected named rule fields into the existing requirement arrays.
	_matchingTraitsUseUpperLane = _vm->_rnd->getRandomBool();

	if (_difficultyLevel == kPuzzleLevel1) {
		// Level 1: single trait extraction.
		_allergyRuleEntryCount = 1;
		for (int16 ruleSlot = 0; ruleSlot < 4; ruleSlot++) {
			const ZmbTrait::TraitKind traitKind = traitKindFromRuleSlot(ruleSlot);
			const TraitValueSet acceptedValues = targetDescriptor.getValues(traitKind);
			if (!acceptedValues.isEmpty()) {
				_allergyRuleTraitKindCodes[0] = requirementTraitKindCode(traitKind);
				_allergyRuleTraitValues[0] = acceptedValues.firstValue;
				break;
			}
		}
	} else if (_difficultyLevel == kPuzzleLevel2) {
		// Level 2: two trait values from the same category.
		_allergyRuleEntryCount = 2;
		for (int16 ruleSlot = 0; ruleSlot < 4; ruleSlot++) {
			const ZmbTrait::TraitKind traitKind = traitKindFromRuleSlot(ruleSlot);
			const TraitValueSet acceptedValues = targetDescriptor.getValues(traitKind);
			if (!acceptedValues.isEmpty()) {
				const int16 traitKindCode = requirementTraitKindCode(traitKind);
				_allergyRuleTraitKindCodes[0] = traitKindCode;
				_allergyRuleTraitValues[0] = acceptedValues.firstValue;
				_allergyRuleTraitKindCodes[1] = traitKindCode;
				_allergyRuleTraitValues[1] = acceptedValues.secondValue;
				break;
			}
		}
	} else {
		// Levels 3 and 4: extract all non-zero nibbles as required traits.
		_allergyRuleEntryCount = 0;
		uint32 ruleEntryIdx = 0;
		for (int16 ruleSlot = 0; ruleSlot < 4; ruleSlot++) {
			const ZmbTrait::TraitKind traitKind = traitKindFromRuleSlot(ruleSlot);
			const TraitValueSet acceptedValues = targetDescriptor.getValues(traitKind);
			if (acceptedValues.isEmpty())
				continue;
			if (ruleEntryIdx < static_cast<uint32>(_difficultyLevel - 1)) {
				_allergyRuleTraitKindCodes[ruleEntryIdx] = requirementTraitKindCode(traitKind);
				_allergyRuleTraitValues[ruleEntryIdx] = acceptedValues.firstValue;
				ruleEntryIdx += 1;
			}
			_allergyRuleEntryCount += 1;
		}
	}
	debugC(3, MohawkEngine_Zoombini::kDebugPage02, "bridge: generated level %d with %d allergy rule entries", _difficultyLevel, _allergyRuleEntryCount);
	for (int i = 0; i < _allergyRuleEntryCount; i++)
		debugC(4, MohawkEngine_Zoombini::kDebugPage02, "bridge: allergy rule %d has trait kind %d and value %d", i, _allergyRuleTraitKindCodes[i], _allergyRuleTraitValues[i]);
}

int16 ZoombiniPuzzleBridge::bridgeLaneToIdx(BridgeLane lane) {
	return static_cast<int16>(lane) - 1;
}

bool ZoombiniPuzzleBridge::testTraitMatch(const ZmbTrait &trait, BridgeLane targetLane) const {
	// @ref ZoombiniPuzzleBridge::_matchingTraitsUseUpperLane selects which lane accepts matching traits.
	// Return true when the chosen lane rejects this Zoombini.

	if (targetLane != BridgeLane::kUpper01 && targetLane != BridgeLane::kLower02)
		targetLane = BridgeLane::kUpper01;

	bool anyMatch = false;
	for (byte i = 0; i < _allergyRuleEntryCount; i++) {
		const byte kindCode = _allergyRuleTraitKindCodes[i];
		const bool validKind = 1 <= kindCode && kindCode <= 4;
		const int kindIdx = static_cast<int>(kindCode) - 1;
		byte traitValue;
		if (validKind)
			traitValue = trait.getTraitValue(ZmbTrait::traitKindFromIndex(kindIdx));
		else
			traitValue = 0;

		if (traitValue == _allergyRuleTraitValues[i])
			anyMatch = true;
	}

	bool laneRejects;
	if (anyMatch)
		laneRejects = !_matchingTraitsUseUpperLane;
	else
		laneRejects = _matchingTraitsUseUpperLane;
	if (targetLane == BridgeLane::kLower02)
		laneRejects = !laneRejects;
	return laneRejects;
}

void ZoombiniPuzzleBridge::bridgeButtons_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	// Enable or disable the Go button based on whether any Snoid has crossed.
	setGoButtonsEnabled(_goButtonEnabled);
	goMapButtons_preRenderShape(feature, hsGroup, hotspots);
}

ZmbEventHandleResult ZoombiniPuzzleBridge::bridgeButtons_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	return goMapButtons_onLButtonDown(feature, absPos, relPos);
}

ZmbRenderResult ZoombiniPuzzleBridge::bridgeVisuals_render(ZmbFeature *feature) {
	(void)feature;

	// Toggle the Go button visibility based on whether any Zoombini has crossed.
	setGoButtonsEnabled(_goButtonEnabled);
	return ZmbRenderResult::kRendered;
}

void ZoombiniPuzzleBridge::bridgeVisuals_postRender(ZmbFeature *feature) {
	(void)feature;

	// Shared button rendering requires no additional page work here.
}

// ---------------------------------------------------------------------------
// Reload SCRB animation data on an existing feature.
// Delegates to @ref ZoombiniPage::loadScrbOntoFeature().
// ---------------------------------------------------------------------------
void ZoombiniPuzzleBridge::reloadScrbAnimation(uint16 featureId, int16 newScrbId) {
	ZmbFeature *feature = _scrbFeatures.find(featureId);
	if (!feature)
		return;

	// A new rejected crossing supersedes a still-running cliff reaction.
	// Preserve the idle return script while replacing the active reaction.
	const int16 chainedScrbId = feature->getChainedScrbId();
	if (featureId == _cliffMainFeatureId && chainedScrbId != 0) {
		feature->setChainedScrbId(0);
		loadScrbOntoFeature(feature, newScrbId);
		feature->setChainedScrbId(chainedScrbId);
		return;
	}

	loadScrbOntoFeature(feature, newScrbId);
}

void ZoombiniPuzzleBridge::refreshPegDisplay() {
	ZmbFeature *cliffGate = _scrbFeatures.find(_cliffGateFeatureId);
	if (!cliffGate)
		return;

	const int16 pegStage = CLIP<int16>(_failedCrossingCount, 0, 6);
	const int16 scrbStage = MIN<int16>(pegStage, 5);
	reloadScrbAnimation(_cliffGateFeatureId, kResScrb1215_CliffGateRejectBase + scrbStage);

	// Each rejection SCRB begins at its current peg stage and ends at the next one.
	// Freeze stages zero through five at frame zero. Stage six uses the final frame
	// of the last authored rejection SCRB, where every peg is gone.
	cliffGate->deactivateAnimate();
	cliffGate->setLastFrameIdx(pegStage < 6 ? 0 : cliffGate->getMaxFrameIdx());
	cliffGate->activateRender();
	cliffGate->resetNextRenderFrame();
	cliffGate->requestVisualRematerialization();
}

Common::Point ZoombiniPuzzleBridge::findRejectReturnPosition(ZmbSnoid *snoid) {
	static constexpr int32 kRejectCollisionThreshold = 36;
	const Common::Rect &rect = _rejectReturnRegions[(_activeCrossingLane == BridgeLane::kUpper01) ? 0 : 1];
	return findNonCollidingPosition(snoid, rect, kRejectCollisionThreshold);
}

// ---------------------------------------------------------------------------
// Find an idle Snoid materialized from the active pack.
// ---------------------------------------------------------------------------
ZmbSnoid *ZoombiniPuzzleBridge::findIdlePackSnoid(uint16 preferredId) {
	// A zero @p preferredId returns immediately.
	// @ref ZoombiniPuzzleBridge::onEveryFrame() uses this sentinel for skip mode, where no Snoid may be selected.
	if (preferredId == 0)
		return nullptr;

	// Crossing must wait for the requested Snoid to finish its arrival animation.
	// Substituting another idle pack member would disconnect the pending entry from the submitted Snoid.
	return getIdleSnoid(preferredId);
}

bool ZoombiniPuzzleBridge::canAcceptDropOnLane(BridgeLane lane) const {
	if (lane != BridgeLane::kUpper01 && lane != BridgeLane::kLower02)
		return false;
	if (isDeparturePending())
		return false;
	if (6 <= _failedCrossingCount)
		return false;
	if (2 <= _pendingDropCount)
		return false;
	if (getDrawOnRegOccupant(bridgeLaneToIdx(lane)) != 0)
		return false;
	return true;
}

void ZoombiniPuzzleBridge::markAcceptedSnoidsForDeparture() {
	for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
		ZmbSnoid *snoid = *it;
		if (!snoid || !snoid->isPackSnoid())
			continue;
		snoid->_packIsOccupied = false;
	}

	for (int16 i = 0; i < _upperLaneArrivalCount; i++) {
		ZmbSnoid *snoid = getSnoid(_upperLaneArrivedSnoidIds[i]);
		if (snoid)
			snoid->_packIsOccupied = true;
	}
	for (int16 i = 0; i < _lowerLaneArrivalCount; i++) {
		ZmbSnoid *snoid = getSnoid(_lowerLaneArrivedSnoidIds[i]);
		if (snoid)
			snoid->_packIsOccupied = true;
	}
}

// ---------------------------------------------------------------------------
// Update the main puzzle state once per animation frame.
// ---------------------------------------------------------------------------
void ZoombiniPuzzleBridge::onPostRenderFrame() {
	if (_isUpdating || !_pageActive)
		return;
	PuzzleUpdateGuard updateGuard(_isUpdating);

	// The Go button uses shape 1 until the first arrival and shape 2 afterward.
	// Derive the state from @ref ZoombiniPuzzleBridge::_goButtonEnabled every frame.
	// No feature hook owns this update.
	setGoButtonsEnabled(_goButtonEnabled);

	// -----------------------------------------------------------------------
	// [0] Pending Go departure: skip normal frame logic while waiting.
	// @ref ZoombiniInteractive::onAnimFrame() handles the actual departure transition.
	// -----------------------------------------------------------------------
	if (isDeparturePending()) {
		return;
	}

	// -----------------------------------------------------------------------
	// [1] Install the terminal cliff transition after the completed render pass.
	// -----------------------------------------------------------------------
	if (_terminalCliffTransitionPending) {
		debugC(2, MohawkEngine_Zoombini::kDebugPage02, "bridge: starting terminal cliff transition");
		_terminalCliffTransitionPending = false;

		// Hide the lane-2 cliff feature before the terminal collapse sequence takes over.
		// Its PLAY_ONCE frame remains drawable after animation stops unless deferred rendering is enabled.
		ZmbFeature *lane2 = _scrbFeatures.find(_cliffLane2FeatureId);
		if (lane2) {
			Common::Rect dirtyRect = lane2->getSortRect();
			if (dirtyRect.isEmpty())
				dirtyRect = lane2->getClickRect();
			if (!dirtyRect.isEmpty())
				addExternalDirtyRect(dirtyRect);
			lane2->addFlag(ZmbFeature::FLAG_01000000_DEFER_RENDER);
			lane2->deactivateRender();
			lane2->deactivateAnimate();
		}

		// Load both replacement scripts after the completed render pass.
		// Rematerialize frame zero next pass so the lane-2 erase and replacement stay atomic.
		ZmbFeature *cliffGate = _scrbFeatures.find(_cliffGateFeatureId);
		reloadScrbAnimation(_cliffGateFeatureId, kResScrb1235_CliffGateEntrance);
		if (cliffGate)
			cliffGate->requestVisualRematerialization();

		// SCRB 1221 runs on lane 1 and owns the entrance callbacks.
		ZmbFeature *lane1 = _scrbFeatures.find(_cliffLane1FeatureId);
		reloadScrbAnimation(_cliffLane1FeatureId, kResScrb1221_CliffEntrance);
		if (lane1)
			lane1->requestVisualRematerialization();
	}

	// -----------------------------------------------------------------------
	// [2] Start the oldest crossing in the pending-drop FIFO.
	// -----------------------------------------------------------------------
	if (0 < _pendingDropCount && _queuedCrossingStartEnabled) {
		uint16 pendingSnoidId = _pendingDropSnoidIds[0];

		// Preserve the six-peg limit, the rejected-crossing transit cap, and the 45-frame start cadence.
		bool skip = false;
		if (6 <= _failedCrossingCount)
			skip = true;

		if (_pendingDropRejected[0] && 4 < _activeCrossingCount)
			skip = true;
		if (!skip && (getCurrentFrameCounter() - _lastCrossingStartFrame) < 0x2D)
			skip = true;
		if (skip)
			pendingSnoidId = 0;

		ZmbSnoid *snoid = findIdlePackSnoid(pendingSnoidId);
		if (snoid) {
			// Record the cadence origin for the next crossing.
			_lastCrossingStartFrame = getCurrentFrameCounter();

			// Remove the oldest entry and shift the second entry forward.
			if (1 <= _pendingDropCount && _pendingDropCount <= 2) {
				_activeCrossingLane = _pendingDropLanes[0];
				_activeCrossingRejected = _pendingDropRejected[0];
				_pendingDropLanes[0] = _pendingDropLanes[1];
				_pendingDropSnoidIds[0] = _pendingDropSnoidIds[1];
				_pendingDropRejected[0] = _pendingDropRejected[1];
				_pendingDropCount -= 1;
			}
			debugC(4, MohawkEngine_Zoombini::kDebugPage02, "bridge: starting queued crossing for runner %u on lane %d; rejected=%d, pending count=%d",
				   snoid->getId(), static_cast<int16>(_activeCrossingLane), _activeCrossingRejected ? 1 : 0, _pendingDropCount);
			// The queued Snoid has left the entrance, so the lane seat can accept another drop.
			clearDrawOnRegOccupant(bridgeLaneToIdx(_activeCrossingLane));

			// Determine SCRS resource based on lane and toll result.
			// A nonzero result selects the rejected lead-in; zero selects the accepted full crossing.
			// SCRS 2005-2009/2015-2019 cross the whole bridge; SCRS
			// 2000-2004/2010-2014 are the slow rejected lead-ins.
			int16 scrsBase;
			if (_activeCrossingLane == BridgeLane::kUpper01) {
				scrsBase = _activeCrossingRejected ? kResScrs2010_RejectBase : kResScrs2015_NormalBase;
			} else {
				scrsBase = _activeCrossingRejected ? kResScrs2000_NormalBase : kResScrs2005_NormalBase;
			}

			_activeCrossingCount += 1;
			_queuedCrossingStartEnabled = !_activeCrossingRejected;
			// Only accepted crossings mark the Snoid busy and assign a faster randomized animation interval.
			// Rejected lead-ins keep the default interval.
			// @ref ZoombiniPuzzleBridge::_queuedCrossingStartEnabled gates them instead.
			//
			// @ref ZoombiniPuzzleBridge::_rejectCrossingSnoidId tracks the rejected crossing Snoid.
			// @ref ZoombiniPuzzleBridge::processLaneStepEvent() can therefore ignore end events from other Snoids.
			if (_activeCrossingRejected) {
				_rejectCrossingSnoidId = snoid->getId();
			} else {
				snoid->_runnerStatus = kSnoidRunnerStatus01_Crossing;
				snoid->setFrameInterval(_vm->_rnd->getRandomNumber(4, 5));
			}

			_activeRejectScrsId = -1;

			// Start SCRS playback on the pack Snoid.
			uint16 feetVariant = snoid->_trait._feet;
			if (feetVariant < 1 || 5 < feetVariant)
				feetVariant = 1;
			int16 scrsId = static_cast<int16>(scrsBase + feetVariant - 1);
			_lastRejectReactionCode = RejectReactionCode::kNotRecordedMinus01;
			_lastRejectSnoidScrsId = -1;
			// Invalidate the old pack position so the idle Snoid ghost is erased.
			addExternalDirtyRect(snoid->getSortRect());

			// With no initial-position override, the feature position becomes the anchor.
			// Setting the anchor to the lane entry determines the render offset.
			// The offset is the difference between that entry and the script's first hotspot.
			// It is approximately zero for feet variant 1, with small corrections for the other variants.
			// Also seed the sort rect at the bridge entry for correct Z-ordering.
			//
			// Reject playback uses the general body tables with tBMP 3000 and REGS 100/101.
			// Normal playback would select tBMP 3100 and its flat horizontal walking sprites,
			// which do not fit the bridge paths.
			const Common::Point &laneEntryPos = kLaneEntryPositions[bridgeLaneToIdx(_activeCrossingLane)];
			snoid->setPointLoc(laneEntryPos);
			snoid->setSortRect(Common::Rect(laneEntryPos.x, laneEntryPos.y, laneEntryPos.x + 1, laneEntryPos.y + 1));
			if (snoid->startScrsPlayback(ZmbResource(ZmbResource::kPage, scrsId), ZmbScrsCompletionMode::kReturnToIdle, resolveScrsRejectState(scrsId))) {
				// Materialize SCRS frame zero synchronously and queue its full coverage
				// for the next render pass. The moving pose can extend above the idle
				// footprint, especially for Ponytail hair.
				prepareSnoidVisualCoverage(snoid, true);
				addExternalDirtyRect(snoid->getSortRect());
				snoid->setNeedsRedraw(true);
			}
		}
	}

	// -----------------------------------------------------------------------
	// [3] Process pending lane event (cliff gate animation after crossing step).
	// -----------------------------------------------------------------------
	if (_pendingCliffLaneFeatureId) {
		debugC(4, MohawkEngine_Zoombini::kDebugPage02, "bridge: processing cliff lane feature %d; rejected=%d",
			   _pendingCliffLaneFeatureId, _activeCrossingRejected ? 1 : 0);
		ZmbFeature *lane = _scrbFeatures.find(_pendingCliffLaneFeatureId);
		if (lane && _activeCrossingRejected) {
			ZmbSnoid *crossingSnoid = getSnoid(_rejectCrossingSnoidId);
			ZmbFeature *cliffMain = _scrbFeatures.find(_cliffMainFeatureId);
			ZmbFeature *cliffGate = _scrbFeatures.find(_cliffGateFeatureId);

			// Load the appropriate cliff animation SCRB on the lane feature.
			int16 cliffLaneScrbId;
			if (_pendingCliffLaneFeatureId == _cliffLane2FeatureId)
				cliffLaneScrbId = kResScrb1222_CliffRejectLane1;
			else
				cliffLaneScrbId = kResScrb1214_CliffRejectLane2;
			_pendingCliffLaneFeatureId = 0;
			reloadScrbAnimation(lane->getId(), cliffLaneScrbId);

			// Trigger cliff gate rejection animation.
			if (_activeCrossingRejected) {
				int16 cliffMainScrbId;
				if (_activeCrossingLane == BridgeLane::kUpper01)
					cliffMainScrbId = kResScrb1223_CliffGateRejectAltBase + _failedCrossingCount;
				else
					cliffMainScrbId = kResScrb1208_CliffGateRejectBase + _failedCrossingCount;
				reloadScrbAnimation(_cliffMainFeatureId, cliffMainScrbId);
			}

			// Update the cliff-gate runner that displays the current bridge segment stage.
			int16 cliffGateScrbId;
			if (!_activeCrossingRejected) {
				if (_activeCrossingLane == BridgeLane::kUpper01)
					cliffGateScrbId = kResScrb1243_CliffGateRejectAltBase + _failedCrossingCount;
				else
					cliffGateScrbId = kResScrb1237_CliffGateRejectBase + _failedCrossingCount;
			} else {
				if (_activeCrossingLane == BridgeLane::kUpper01)
					cliffGateScrbId = kResScrb1229_CliffGateRejectBase + _failedCrossingCount;
				else
					cliffGateScrbId = kResScrb1215_CliffGateRejectBase + _failedCrossingCount;
			}
			reloadScrbAnimation(_cliffGateFeatureId, cliffGateScrbId);

			// Reloading the reject runners starts a new shared cadence.
			// Leave its owner unset so the earliest active runner in page order selects
			// the timing result for the crossing Snoid and every reaction feature.
			if (crossingSnoid) {
				// Each runner belongs to only one timing slot. Retire the previous reject
				// group before moving the reusable cliff runners into this one.
				unregisterFeatureTimingGroup(cliffMain);
				unregisterFeatureTimingGroup(cliffGate);
				registerFeatureTimingGroup(crossingSnoid, lane);
				joinFeatureTimingGroup(crossingSnoid, cliffMain);
				joinFeatureTimingGroup(crossingSnoid, cliffGate);
			}
		}
		_pendingCliffLaneFeatureId = 0;
	}
}

void ZoombiniPuzzleBridge::onPostAmbientFrame() {
	scheduleCelebration();
}

void ZoombiniPuzzleBridge::scheduleCelebration() {
	if (_isUpdating || !_pageActive || isDeparturePending())
		return;
	PuzzleUpdateGuard updateGuard(_isUpdating);

	if (_celebrationStartedCount < _celebrationTargetCount &&
		_celebrationFrameInterval < getCurrentFrameCounter() - _lastCelebrationFrame) {

		_lastCelebrationFrame = getCurrentFrameCounter();
		bool triggered = false;
		int16 attempts = 0;

		do {
			attempts += 1;

			// Non-repeat random pool: uses bitmask to track which indices have been used.
			// Picks a random index, scans forward if already used. Resets when all exhausted.
			uint16 poolIdx;
			{
				uint16 rndIdx = _vm->_rnd->getRandomNumber(0, 0 < _pageLoadedZmbCount ? _pageLoadedZmbCount - 1 : 0);
				uint16 startIdx = rndIdx;
				while (_celebrationUsedSnoidMask & (1u << rndIdx)) {
					rndIdx += 1;
					if (_pageLoadedZmbCount <= rndIdx)
						rndIdx = 0;
					if (rndIdx == startIdx) {
						// All used -- reset pool
						_celebrationUsedSnoidMask = 0;
					}
				}
				_celebrationUsedSnoidMask |= (1u << rndIdx);
				poolIdx = rndIdx;
			}

			uint16 snoidId = 10000 + poolIdx;

			ZmbSnoid *snoid = getSnoid(snoidId);
			// Only fully idle, arrived pack runners are eligible:
			// 1. The Snoid must have settled into its idle animation.
			//    Snoids in @ref kSnoidAnimState007_Depart or @ref kSnoidAnimState112_Path are still walking.
			//    Starting a celebration then would freeze the Snoid at its current walk position.
			// 2. The runner status must identify an arrived Snoid rather than a bank Snoid.
			//    Bank Snoids must not receive a celebration SCRS.
			if (snoid && snoid->_runnerStatus != kSnoidRunnerStatus00_Available &&
				snoid->getAnimState() == kSnoidAnimState000_Idle &&
				snoid->hasFlag(ZmbFeature::FLAG_00000001_TYPE_SNOID)) {
				// The feet trait selects celebration SCRS 2020-2024.
				int16 scrsId = static_cast<int16>(snoid->_trait._feet + kResScrs2019_CelebrationBase);
				if (snoid->startScrsPlayback(ZmbResource(ZmbResource::kPage, scrsId), ZmbScrsCompletionMode::kReturnToIdle, resolveScrsRejectState(scrsId))) {
					_celebrationStartedCount += 1;
					triggered = true;
				}
			}
		} while (!triggered && attempts < 16);
	}
}

// ---------------------------------------------------------------------------
// Dispatch animation event codes to the appropriate handler.
// ---------------------------------------------------------------------------
void ZoombiniPuzzleBridge::onFeatureAnimEvent(ZmbFeature *feature, int16 eventCode) {
	if (feature->hasFlag(ZmbFeature::FLAG_00000001_TYPE_SNOID)) {
		// Crossing snoid -> lane step callback
		processLaneStepEvent(feature, eventCode);
		return;
	}

	// Only the entrance runner and animated cliff-main runner own this callback.
	const bool isEntranceRunner = feature->getId() == _cliffLane1FeatureId && feature->getScrbId() == kResScrb1221_CliffEntrance;
	if (isEntranceRunner || feature->getId() == _cliffMainFeatureId)
		processEntranceEvent(eventCode, feature);
}

// ---------------------------------------------------------------------------
// Process a lane-step callback from crossing-Snoid SCRS playback.
// ---------------------------------------------------------------------------
void ZoombiniPuzzleBridge::processLaneStepEvent(ZmbFeature *snoidFeature, int16 stepCode) {
	ZmbSnoid *snoid = static_cast<ZmbSnoid *>(snoidFeature);

	// Frame-terminator dispatch has already adjusted stepCode once.
	// Do not decrement it again.
	// Arrival events 4 and 7 map onto cases 3 and 6,
	// which makes accepted Snoids arrive before crossing.
	// Reject event 11 maps to case 10 so the throw animation starts.

	switch (stepCode) {
	case kLaneEventCode001_LowerStart:
	case kLaneEventCode004_LowerStart:
		// The lower-lane SCRS has reached its first hand-off marker.
		// Remember the lower cliff runner so the next frame can continue its bridge animation.
		// Set pending lane event to the lower bridge cliff runner.
		_pendingCliffLaneFeatureId = _cliffLane1FeatureId;
		break;
	case kLaneEventCode002_UpperStart:
	case kLaneEventCode005_UpperStart:
		// The upper-lane SCRS has reached its first hand-off marker.
		// Remember the upper cliff runner for the matching bridge segment.
		// Set pending lane event to the upper bridge cliff runner.
		_pendingCliffLaneFeatureId = _cliffLane2FeatureId;
		break;
	case kLaneEventCode003_LowerArrival:
	case kLaneEventCode006_UpperArrival: {
		// The Snoid has reached the far side of the bridge.
		// Commit its lane occupancy, switch it to the departure walk, and preserve the terminal frame.
		// Zoombini arrives at destination lane.
		_activeCrossingCount -= 1;

		// Start departure from the current SCRS-driven bridge position.
		// Restoring the pre-SCRS position first would make the Snoid jump back to the left bank.
		// This callback runs after @ref ZoombiniPage::preRenderFeature() prepares the terminal SCRS frame.
		// Preserve that frame for the current draw pass.
		// This prevents @ref ZoombiniPage::blitShapes() from showing the walking pose early.
		const Common::Array<ZmbPreparedRenderHotspot> arrivalFrame = snoid->getPreparedRenderHotspots();
		snoid->finishScrsPlayback(false);
		// Add @ref ZmbFeature::FLAG_00008000_LOOP_ANIM and @ref ZmbFeature::FLAG_04000000_OVERLAY on arrival.
		// The overlay flag gives crossed Snoids the correct compositing order.
		snoid->addFlag(static_cast<ZmbFeature::Flag>(ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_04000000_OVERLAY));

		// Set the departure state before storing its target.
		// Do not pass the destination to @ref ZmbSnoid::setAnimState(); it is a walk target, not an immediate position.
		Common::Point destPos;
		if (stepCode == kLaneEventCode006_UpperArrival) {
			// Record the upper-lane arrival.
			if (_upperLaneArrivalCount < 16) {
				const int16 arrivalIdx = _upperLaneArrivalCount;
				destPos = kUpperLaneArrivalPositions[arrivalIdx];
				_upperLaneArrivedSnoidIds[arrivalIdx] = snoid->getId();

				int16 parentSlot = arrivalIdx - 1;
				bool linkAfterParent = false;
				switch (arrivalIdx) {
				case 0:
					parentSlot = -1;
					break;
				case 5:
				case 6:
					linkAfterParent = true;
					break;
				case 7:
					parentSlot = 0;
					linkAfterParent = true;
					break;
				case 10:
					parentSlot = 7;
					linkAfterParent = true;
					break;
				case 15:
					parentSlot = 9;
					break;
				default:
					break;
				}

				if (0 <= parentSlot) {
					ZmbSnoid *parentSnoid = getSnoid(_upperLaneArrivedSnoidIds[parentSlot]);
					if (parentSnoid) {
						if (linkAfterParent)
							manualLinkAfter(snoid, parentSnoid);
						else
							manualLinkBefore(snoid, parentSnoid);
					}
				}
				_upperLaneArrivalCount += 1;
			}
		} else {
			// Record the lower-lane arrival.
			if (_lowerLaneArrivalCount < 16) {
				const int16 arrivalIdx = _lowerLaneArrivalCount;
				destPos = kLowerLaneArrivalPositions[arrivalIdx];
				_lowerLaneArrivedSnoidIds[arrivalIdx] = snoid->getId();
				if (0 < arrivalIdx) {
					ZmbSnoid *parentSnoid = getSnoid(_lowerLaneArrivedSnoidIds[arrivalIdx - 1]);
					if (parentSnoid)
						manualLinkBefore(snoid, parentSnoid);
				}
				_lowerLaneArrivalCount += 1;
			}
		}

		snoid->setAnimState(kSnoidAnimState007_Depart);
		snoid->setAnimTargetPos(destPos);
		if (!arrivalFrame.empty())
			snoid->setPreparedRenderHotspots(arrivalFrame);

		// Mark the Snoid as arrived so later drag attempts are refused.
		snoid->_runnerStatus = kSnoidRunnerStatus02_Arrived;

		// Per-frame Z sorting and the OVERLAY flag provide the required arrival stacking
		// without an explicit linked-list reorder.

		if (!_goButtonEnabled)
			_goButtonEnabled = true;

		// Celebration schedule thresholds: 10, 12, 14, all.
		// Count real pack Snoids once crossing has claimed their runner status.
		// SCRS pools also carry TYPE_SNOID, so exclude non-pack IDs.
		int16 acceptedPackCount = countAcceptedPackSnoids();
		if (acceptedPackCount == 10)
			_celebrationTargetCount += 1;
		else if (acceptedPackCount == 12)
			_celebrationTargetCount += 1;
		else if (acceptedPackCount == 14)
			_celebrationTargetCount += 2;
		if (acceptedPackCount == _pageLoadedZmbCount)
			_celebrationTargetCount += 2;

		int16 totalArrived = _upperLaneArrivalCount + _lowerLaneArrivalCount;
		// Play voice-over when all are crossed
		if (totalArrived == _pageLoadedZmbCount && _activeCrossingCount == 0)
			queueCompletionNarratorSound();
		break;
	}
	case kLaneEventCode010_StartRejectTraitScript: {
		// A rejected Snoid has reached the trait-reveal marker.
		// Start the throw/trait script while keeping the crossing lock active.
		// Play snoid trait display script.
		startRejectThrowScript(snoid);
		break;
	}
	case kLaneEventCode020_FinishRejectTraitScript:
		// The reject/trait script has completed its logical work.
		// Release one transit slot and allow the player to retry the rejected Snoid.
		// Reject/trait display script completed its logical effect.
		// Update the transit counters before enabling success or retry input.
		if (0 < _activeCrossingCount)
			_activeCrossingCount -= 1;
		if (_failedCrossingCount < 6) {
			_failedCrossingCount += 1;
		}
		_rejectedSnoidPickupEnabled = true;
		break;
	case kAnimEventM1_End: {
		// End of SCRS playback: reposition rejected Zoombini.
		// The completion callback belongs only to the active crossing Snoid.
		// Celebration Snoids may also finish SCRS playback here,
		// so ignore any runner that does not match @ref ZoombiniPuzzleBridge::_rejectCrossingSnoidId.
		if (_rejectCrossingSnoidId != 0 && snoid->getId() != _rejectCrossingSnoidId)
			break;
		_rejectedSnoidPickupEnabled = false;
		if (_queuedCrossingStartEnabled)
			break;
		if (_activeRejectScrsId < 0 && startRejectThrowScript(snoid))
			break;

		_queuedCrossingStartEnabled = true;
		_rejectCrossingSnoidId = 0;
		_activeRejectScrsId = -1;

		// Return rejected Snoids to their saved lane-start seats, not their pack positions.
		Common::Point targetPos = findRejectReturnPosition(snoid);
		snoid->setAnimTargetPos(targetPos);
		snoid->setAnimState(kSnoidAnimState010_ArrivalMotion);
		// Return the Snoid to the idle pool so it becomes draggable again.
		snoid->_runnerStatus = kSnoidRunnerStatus00_Available;
		break;
	}
	default:
		break;
	}
}

bool ZoombiniPuzzleBridge::startRejectThrowScript(ZmbSnoid *snoid) {
	if (_pendingRejectReactionCode == RejectReactionCode::kNone00)
		return false;

	int16 scrsBase;
	switch (_pendingRejectReactionCode) {
	case RejectReactionCode::kEyesRule02:
		scrsBase = kResScrs1012_RejectEyes;
		break;
	case RejectReactionCode::kNoseRule03:
		scrsBase = kResScrs1008_RejectNose;
		break;
	case RejectReactionCode::kFeetRule04:
		scrsBase = kResScrs1000_RejectBase;
		break;
	case RejectReactionCode::kAlternateHairRule05:
		scrsBase = kResScrs1004_RejectHair;
		break;
	case RejectReactionCode::kPrimaryHairRule01:
	case RejectReactionCode::kAlternateFeetRule06:
		scrsBase = kResScrs1016_RejectDefault;
		break;
	case RejectReactionCode::kNone00:
	case RejectReactionCode::kNotRecordedMinus01:
		return false;
	default:
		return false;
	}

	Common::Point initPos;
	if (_activeCrossingLane == BridgeLane::kUpper01) {
		scrsBase += 2;
		initPos = Common::Point(38, 106);
	} else {
		initPos = Common::Point(56, 205);
	}

	scrsBase += _vm->_rnd->getRandomNumber(0, 1);

	// SCRS 1000-1019 belong to pool 0, which selects state 9.
	if (!snoid->startScrsPlayback(ZmbResource(ZmbResource::kPage, scrsBase), ZmbScrsCompletionMode::kReturnToIdle, resolveScrsRejectState(scrsBase), &initPos))
		return false;

	_lastRejectReactionCode = _pendingRejectReactionCode;
	_lastRejectSnoidScrsId = scrsBase;
	// Case 10 is dispatched during pre-render, after the old bridge-walk frame may already have been prepared.
	// Replace it immediately so the throw frame owns the same draw pass.
	prepareSnoidVisualCoverage(snoid, true);
	_activeRejectScrsId = scrsBase;
	_pendingRejectReactionCode = RejectReactionCode::kNone00;
	return true;
}

// ---------------------------------------------------------------------------
// Process a cliff-entrance animation event.
// ---------------------------------------------------------------------------
void ZoombiniPuzzleBridge::processEntranceEvent(int16 eventId, ZmbFeature *eventSource) {
	if (kEntranceEventCode001_ShowHairRule <= eventId &&
		eventId <= kEntranceEventCode006_ShowFeetRuleAlternate) {
		// Preserve the cliff reaction code until the rejected Snoid reaches its throw-script marker.
		_pendingRejectReactionCode = static_cast<RejectReactionCode>(eventId);
	} else if (eventId != kLaneEventCode010_StartRejectTraitScript &&
			   kEntranceEventCode100_ShowWaterSplash <= eventId &&
			   eventId <= kEntranceEventCode101_RestoreWaterOverlay) {
		// Change water overlay animation.
		// 100: load SCRB 1236 (water splash), 101: load SCRB 1103 (normal water)
		int16 waterScrbId;
		if (eventId == kEntranceEventCode100_ShowWaterSplash)
			waterScrbId = kResScrb1236_WaterSplash;
		else
			waterScrbId = kResScrb1103_Overlay;
		reloadScrbAnimation(_waterOverlayFeatureId, waterScrbId);
		if (eventId == kEntranceEventCode100_ShowWaterSplash) {
			ZmbFeature *waterFeature = _scrbFeatures.find(_waterOverlayFeatureId);
			registerFeatureTimingFollower(eventSource, waterFeature);
		}
	} else if (eventId == kAnimEventM1_End) {
		// End of entrance animation. Maybe play a voice-over.
		// Only the lane-1 entrance runner with SCRB 1221 owns completion feedback.
		// Cliff-main reject reactions share the frame-event handler but not this completion path.
		if (eventSource->getId() != _cliffLane1FeatureId ||
			eventSource->getScrbId() != kResScrb1221_CliffEntrance) {
			return;
		}
		// Play only for a non-empty partial group, gated by difficulty or an early page-visit count.
		int16 acceptedPackCount = countAcceptedPackSnoids();
		if (acceptedPackCount < _pageLoadedZmbCount && passesPartialResultFeedbackGate()) {
			if (0 < acceptedPackCount)
				queuePartialSuccessNarratorSound();
		}
	} else if (eventId == kEntranceEventCode000_Start) {
		// Defer the terminal cliff replacement until the current render pass completes.
		_terminalCliffTransitionPending = true;
	}
}

int16 ZoombiniPuzzleBridge::countAcceptedPackSnoids() const {
	int16 count = 0;
	for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
		const ZmbSnoid *snoid = *it;
		if (!snoid || !snoid->isPackSnoid())
			continue;
		if (snoid->_runnerStatus == kSnoidRunnerStatus00_Available)
			continue;
		count += 1;
	}
	return count;
}

// ---------------------------------------------------------------------------
// Drag-and-drop: Zoombini interaction.
// ---------------------------------------------------------------------------
ZmbEventHandleResult ZoombiniPuzzleBridge::onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) {
	// Let the base class handle button clicks first.
	ZmbEventHandleResult result = ZoombiniInteractive::onLButtonDown(absPos, relPos);
	if (result == ZmbEventHandleResult::kConsumed)
		return result;

	// Refuse another crossing after all six pegs have fallen or while a drag is active.
	if (6 <= _failedCrossingCount || isDragging())
		return ZmbEventHandleResult::kPassthrough;

	ZmbSnoid *snoid = findSnoidAtPoint(absPos);
	if (!snoid)
		return ZmbEventHandleResult::kPassthrough;

	// Refuse drag after an accepted crossing begins or reaches the far side.
	if (snoid->_runnerStatus == kSnoidRunnerStatus01_Crossing ||
		snoid->_runnerStatus == kSnoidRunnerStatus02_Arrived)
		return ZmbEventHandleResult::kPassthrough;

	// Don't drag snoids that are playing scripts
	SnoidAnimState state = snoid->getAnimState();
	if (state == kSnoidAnimState008_ScriptReject)
		return ZmbEventHandleResult::kPassthrough;
	if (state == kSnoidAnimState009_ScriptNormal) {
		// Allow the rejected Snoid to be picked up only during the authored retry window.
		if (!_rejectedSnoidPickupEnabled)
			return ZmbEventHandleResult::kPassthrough;
		if (!_queuedCrossingStartEnabled)
			_queuedCrossingStartEnabled = true;
		_rejectedSnoidPickupEnabled = false;
	}

	startSnoidDrag(snoid, absPos);
	_dragRemovedOldestPendingDrop = false;

	// Remove this Snoid from the pending-drop FIFO before beginning a new drag.
	if (_pendingDropCount == 1 && _pendingDropSnoidIds[0] == snoid->getId()) {
		_pendingDropCount = 0;
		_dragRemovedOldestPendingDrop = true;
	} else if (_pendingDropCount == 2) {
		if (_pendingDropSnoidIds[1] == snoid->getId()) {
			_pendingDropCount = 1;
		} else if (_pendingDropSnoidIds[0] == snoid->getId()) {
			_pendingDropLanes[0] = _pendingDropLanes[1];
			_pendingDropSnoidIds[0] = _pendingDropSnoidIds[1];
			_pendingDropRejected[0] = _pendingDropRejected[1];
			_pendingDropCount = 1;
			_dragRemovedOldestPendingDrop = true;
		}
	}

	return ZmbEventHandleResult::kConsumed;
}

ZmbEventHandleResult ZoombiniPuzzleBridge::onMouseMove(const Common::Point &absPos, const Common::Point &relPos) {
	ZmbEventHandleResult result = ZoombiniInteractive::onMouseMove(absPos, relPos);

	if (isDragging() && 0 <= _dragHighlightSlot && !canAcceptDropOnLane(static_cast<BridgeLane>(_dragHighlightSlot + 1)))
		clearDrawOnRegHighlight();

	return result;
}

void ZoombiniPuzzleBridge::endDrag(const Common::Point &dropPos) {
	// The shared drag controller highlights only an empty registered DRAW_ON_REG
	// slot inside the authored click-zone square. Preserve that committed result
	// before @ref ZoombiniInteractive::finishSnoidDrag() clears the highlight.
	BridgeLane dropLane;
	if (0 <= _dragHighlightSlot)
		dropLane = static_cast<BridgeLane>(_dragHighlightSlot + 1);
	else
		dropLane = BridgeLane::kNone00;
	ZmbSnoid *snoid = finishSnoidDrag();
	if (!snoid)
		return;
	const bool removedOldestPendingDrop = _dragRemovedOldestPendingDrop;
	_dragRemovedOldestPendingDrop = false;

	if (canAcceptDropOnLane(dropLane)) {
		// Add a valid lane drop to the pending FIFO.
		const bool dropRejected = testTraitMatch(snoid->_trait, dropLane);
		setDrawOnRegOccupant(bridgeLaneToIdx(dropLane), snoid->getId());
		_pendingDropLanes[_pendingDropCount] = dropLane;
		_pendingDropSnoidIds[_pendingDropCount] = snoid->getId();
		_pendingDropRejected[_pendingDropCount] = dropRejected;
		_pendingDropCount += 1;
		debugC(4, MohawkEngine_Zoombini::kDebugPage02, "bridge: queued runner %u on lane %d; rejected=%d, pending count=%d", snoid->getId(),
			   static_cast<int16>(dropLane), dropRejected ? 1 : 0, _pendingDropCount);
		// The common arrival handoff commits the bridge entrance and faces the Snoid right.
		// @ref ZoombiniPuzzleBridge::findIdlePackSnoid() starts crossing after arrival becomes idle.
		settleSnoidAtTarget(snoid, kLaneEntryPositions[bridgeLaneToIdx(dropLane)]);
	} else if (removedOldestPendingDrop) {
		// A removed pending entry returns to the first available fixed left-bank waiting position.
		const Common::Point returnPos = findRandomEmptySnoidSlotPosition(kLeftBankWaitingPositions, ARRAYSIZE(kLeftBankWaitingPositions), 500);
		settleSnoidAtTarget(snoid, returnPos);
	} else {
		// No valid drop: validate walkability, adjust for collisions, or return to the pickup position.
		debugC(4, MohawkEngine_Zoombini::kDebugPage02, "bridge: rejected runner %u drop at (%d, %d); returning to (%d, %d)", snoid->getId(),
			   dropPos.x, dropPos.y, _dragOrigPos.x, _dragOrigPos.y);
		settleSnoidAfterTerrainDrop(snoid, _dragOrigPos);
	}
}

} // End of namespace Mohawk
