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

#include "mohawk/zoombini_pages/puzzle_tunnels.h"
#include "mohawk/zoombini.h"
#include "mohawk/zoombini_graphics.h"
#include "mohawk/zoombini_random.h"
#include "mohawk/zoombini_sound.h"
#include "mohawk/zoombini_state.h"

namespace Mohawk {

constexpr Common::Point ZoombiniPuzzleTunnels::kSnoidPositions[16];
constexpr Common::Point ZoombiniPuzzleTunnels::kTunnelEntryPositions[4];
constexpr int16 ZoombiniPuzzleTunnels::kGateActorSlotByEntry[4];
constexpr Common::Point ZoombiniPuzzleTunnels::kScrsReplayPositions[4];
constexpr Common::Point ZoombiniPuzzleTunnels::kOutputLanePositions[4][16];
constexpr int16 ZoombiniPuzzleTunnels::kDropZoneToLane[4];
constexpr int16 ZoombiniPuzzleTunnels::kHoverDataToGateType[8];
constexpr int16 ZoombiniPuzzleTunnels::kSpawnOriginX[4];
constexpr int16 ZoombiniPuzzleTunnels::kRejectScrbPool0[10];
constexpr int16 ZoombiniPuzzleTunnels::kCorrectHintSmallScrbPool[11];
constexpr int16 ZoombiniPuzzleTunnels::kRejectScrbPool1[8];
constexpr int16 ZoombiniPuzzleTunnels::kRejectScrbPool2[8];
constexpr int16 ZoombiniPuzzleTunnels::kRejectScrbPool3[7];
constexpr int16 ZoombiniPuzzleTunnels::kCorrectHintLargeScrbPool[6];
constexpr int16 ZoombiniPuzzleTunnels::kRejectGateScrbPool1[4];
constexpr int16 ZoombiniPuzzleTunnels::kRejectScrbPool4[6];

// =========================================================================
// Constructor / Destructor
// =========================================================================

ZoombiniPuzzleTunnels::ZoombiniPuzzleTunnels(MohawkEngine_Zoombini *vm) : ZoombiniPuzzle(vm, ZoombiniPageType::kTunnels, ZmbSrcPageKind::kTunnels_03) {
}

ZoombiniPuzzleTunnels::~ZoombiniPuzzleTunnels() {
}

const ZoombiniPage::ScriptSoundPriorityRanges &ZoombiniPuzzleTunnels::getScriptSoundPriorityRanges() const {
	// Original shared page dispatch automatically prepends SND 996-997 at priority 32.
	static const ZoombiniPage::ScriptSoundPriorityRanges kRanges = {
		{kSysResSoundRange0996_DepartSFX, kSysResSoundRange0997_ArriveSFX},
		{kSysResSoundRange20000_Base, kSysResSoundRange29999_Last},
		{kResSoundRange4000_GateBase, kResSoundRange4699_GateLast},
		{kResSoundRange7000_AmbientBase, kResSoundRange7099_AmbientLast},
		{kResSoundRange0425_PageRangeBase, kResSoundRange0499_PageRangeLast},
		{kResSoundRange4700_RockfallBase, kResSoundRange4799_RockfallLast},
		{kResSoundRange6000_SystemRangeBase, kResSoundRange6099_SystemRangeLast},
		{kResSoundRange0175_PageRangeBase, kResSoundRange0199_PageRangeLast},
		{kResSoundRange0099_PageSingle, kResSoundRange0099_PageSingle},
		{kResSoundRange8500_PageRangeBase, kResSoundRange8599_PageRangeLast}};
	return kRanges;
}

Audio::Mixer::SoundType ZoombiniPuzzleTunnels::getFeatureSoundType(const ZmbFeature *feature, ZmbResource sndRes) const {
	const bool isGateActorFeature = feature &&
									(feature == _gateActorFeatures[0] || feature == _gateActorFeatures[1] ||
									 feature == _gateActorFeatures[2] || feature == _gateActorFeatures[3]);
	const bool isGateActorSpeech =
		(kResSound4000_CrystalSpeechBase <= sndRes._id && sndRes._id <= kResSound4038_CrystalSpeechLast) ||
		(kResSound4200_FerrousSpeechBase <= sndRes._id && sndRes._id <= kResSound4227_FerrousSpeechLast) ||
		(kResSound4400_IgnoSpeechBase <= sndRes._id && sndRes._id <= kResSound4427_IgnoSpeechLast) ||
		(kResSound4600_OnyxSpeechBase <= sndRes._id && sndRes._id <= kResSound4618_OnyxSpeechLast);
	if (isGateActorFeature && sndRes._archiveKind == ZmbResource::kPage && isGateActorSpeech)
		return Audio::Mixer::kSpeechSoundType;
	return ZoombiniPuzzle::getFeatureSoundType(feature, sndRes);
}

// =========================================================================
// Page Lifecycle
// =========================================================================

void ZoombiniPuzzleTunnels::open() {
	openArchive(ZMB_MHK_TUNNELS);
}

void ZoombiniPuzzleTunnels::setBackgroundBitmap() {
	_vm->_gfx->setPalette(kResBackground300);
	_vm->_gfx->drawBackground(kResBackground300);
}

void ZoombiniPuzzleTunnels::initStates() {
	// Ordinary Tunnels pack-seat shifts and lane handoffs are explicit straight-line exceptions.
	// NODE/PATH 1000 remains loaded during those operations.
	setSnoidNodePathDisabled(true);

	_enteredSnoidCount = 0;
	_remainingRejectChances = 0;
	_postGameStarted = false;
	_postGameAnimCompleted = false;
	_animationQueueAdvanceEnabled = true;
	_setupPhase = SetupPhase::kIdle00;
	_clickZoneRadius = kClickZoneRadius;

	// Reset rule system
	_guardAxisCount = 0;
	for (int i = 0; i < 2; i++) {
		_guardRules[i] = TunnelGuardRule();
	}

	// Reset per-gate state
	for (int gIdx = 0; gIdx < 4; gIdx++) {
		_laneOccupancy[gIdx] = 0;
		for (int sIdx = 0; sIdx < 16; sIdx++)
			_laneSnoidIds[gIdx][sIdx] = 0;
	}
	for (int i = 0; i < 5; i++)
		_zoneCorrectStreak[i] = 0;

	_crystalRejectCountSinceSuccess = 0;
	_ignoRejectCountSinceSuccess = 0;

	if (_vm->_state->_practiceLevel != 0)
		_vm->_bridgeTunnelsLevel1Exclusion.clear();

	// Random seed for the level-1 gate bias
	_level1BlockedPairToggle = _vm->_rnd->getRandomBool();

	// Reset animation queue
	_animQueueCount = 0;
	_draggedSnoidWasQueued = false;
	for (int queueIdx = 0; queueIdx < 5; queueIdx++)
		_animQueue[queueIdx] = AnimQueueEntry();

	_deferredActorSlot = -1;
	_deferredActorScrbId = 0;
	_deferredActorPopsQueue = false;
	_pendingBodyArrangement = 0;

	// Reset ambient animation
	_idleActorDeadline = 0;
	_idleActorDelay = 0;
	_celebrationTarget = 0;
	_celebrationsPlayed = 0;
	_lastCelebrationFrame = 0;
	// Use 60 ticks; the unsupported legacy double-update mode used 120.
	_celebrationInterval = 60;
	_rockfallWarningSoundId = 0;
	_rockfallWarningSoundPlaying = false;
	_rockfallWarningSoundHandleValid = false;
	_completionNarratorSoundId = 0;
	_activeGateOrAmbientSoundResource = ZmbResource();

	// Reset pool state bitmasks
	_crystalRejectPoolState = 0;
	_crystalHintPoolState = 0;
	_ferrousRejectPoolState = 0;
	_ferrousCorrectPoolState = 0;
	_ignoRejectPoolState = 0;
	_ignoHintPoolState = 0;
	_onyxRejectPoolState = 0;
	_onyxCorrectPoolState = 0;
	_idleActorPoolState = 0;
	_entryActorPoolState = 0;
	_postGameActorPoolState = 0;
	_allIdleAdvanceActorPoolState = 0;
	_partialAdvanceActorPoolState = 0;
	_goReadyAdvanceActorPoolState = 0;
	_celebrationSnoidPoolState = 0;

	for (int i = 0; i < ARRAYSIZE(_packSeatSnoidIds); i++)
		_packSeatSnoidIds[i] = 0;

	// Features
	_laneLinkAnchorFeature = nullptr;
	for (int i = 0; i < 4; i++) {
		_tunnelEntryFeatures[i] = nullptr;
		_gateActorFeatures[i] = nullptr;
		_gateActorCompletionActions[i] = kGateActorCompletionNone00;
	}
	_rockfallWarningFeature = nullptr;
	_mainPathFeature = nullptr;
	_builtinDebugScrbId = 0;
}

Common::String ZoombiniPuzzleTunnels::debugGetBuiltinDebugCommandHelp() const {
	Common::String output;
	output += Common::String::format("  %-7s (%s)\n", "a", kBuiltinDebugActionAcceptance);
	output += "    Draw the generated side/orientation label and every trait condition (read-only).\n";
	output += Common::String::format("  %-7s (%s)\n", "Shift+C", kBuiltinDebugActionCycleCrystal);
	output += "    Load Crystal SCRBs 4000-4038 in inclusive order.\n";
	output += Common::String::format("  %-7s (%s)\n", "Shift+F", kBuiltinDebugActionCycleFerrous);
	output += "    Load Ferrous SCRBs 4200-4226 in inclusive order.\n";
	output += Common::String::format("  %-7s (%s)\n", "Shift+I", kBuiltinDebugActionCycleIgno);
	output += "    Load Igno SCRBs 4400-4423 in inclusive order.\n";
	output += Common::String::format("  %-7s (%s)\n", "Shift+O", kBuiltinDebugActionCycleOnyx);
	output += "    Load Onyx SCRBs 4600-4617 in inclusive order.\n";
	output += "    All four selectors share one index; switching families starts at that family's first ID.\n";
	output += "    Each load also shows the selected SCRB id; none changes the sorting rule.\n";
	output += Common::String::format("  %-7s (%s)\n", "Shift+H", kBuiltinDebugActionCelebrationPlusFour);
	output += "    Add four to the cumulative celebration target. Eligible entered Zoombinis animate later.\n";
	output += Common::String::format("  %-7s (%s)\n", "t", kBuiltinDebugActionIdle);
	output += "    Queue one nonrepeating selection from the ten-entry idle actor pool.\n";
	output += Common::String::format("  %-7s (%s)\n", "Shift+T", kBuiltinDebugActionEndGame);
	output += "    Queue one nonrepeating Igno SCRB 4420-4422 post-game reaction; do not finish the puzzle.\n";
	output += Common::String::format("  %-7s (%s)\n", "e", kBuiltinDebugActionAdvance);
	output += "    Queue an actor reaction from the ready-state 7-entry, all-idle 8-entry, or partial 9-entry pool.\n";
	output += "    This does not press Go, move a Zoombini, or start departure.\n";
	output += Common::String::format("  %-7s (%s)\n", "w", kBuiltinDebugActionEntry);
	output += "    Queue from the 4-entry first-open pool only when the engine-session open count is 1; otherwise use the 8-entry later-open pool.\n";
	output += Common::String::format("  %-7s (%s)\n", "Shift+W", kBuiltinDebugActionEntryFirstVisit);
	output += "    Permanently set that engine-session open count to 1, then queue from the first-open pool.\n";
	output += "    Actor selections append to the normal queue and may wait for an active sequence to finish.\n";
	return output;
}

bool ZoombiniPuzzleTunnels::debugDoBuiltinDebugCommand(int argc, const char **argv, Common::String &output) {
	if (argc != 3) {
		output = "Usage: page builtin_debug <Tunnels action>\n";
		output += debugGetBuiltinDebugCommandHelp();
		return true;
	}
	const BuiltinDebugAction action = parseBuiltinDebugAction(argv[2]);
	if (action == BuiltinDebugAction::kInvalid) {
		output = Common::String::format("Unknown Tunnels built-in debug action '%s'.\n", argv[2]);
		output += debugGetBuiltinDebugCommandHelp();
		return true;
	}
	return runBuiltinDebugAction(action, output);
}

ZoombiniPuzzleTunnels::BuiltinDebugAction ZoombiniPuzzleTunnels::parseBuiltinDebugAction(const Common::String &action) {
	if (action.equalsIgnoreCase(kBuiltinDebugActionAcceptance))
		return BuiltinDebugAction::kAcceptance;
	if (action.equalsIgnoreCase(kBuiltinDebugActionCycleCrystal))
		return BuiltinDebugAction::kCycleCrystal;
	if (action.equalsIgnoreCase(kBuiltinDebugActionCycleFerrous))
		return BuiltinDebugAction::kCycleFerrous;
	if (action.equalsIgnoreCase(kBuiltinDebugActionCycleIgno))
		return BuiltinDebugAction::kCycleIgno;
	if (action.equalsIgnoreCase(kBuiltinDebugActionCycleOnyx))
		return BuiltinDebugAction::kCycleOnyx;
	if (action.equalsIgnoreCase(kBuiltinDebugActionCelebrationPlusFour))
		return BuiltinDebugAction::kCelebrationPlusFour;
	if (action.equalsIgnoreCase(kBuiltinDebugActionIdle))
		return BuiltinDebugAction::kIdle;
	if (action.equalsIgnoreCase(kBuiltinDebugActionEndGame))
		return BuiltinDebugAction::kEndGame;
	if (action.equalsIgnoreCase(kBuiltinDebugActionAdvance))
		return BuiltinDebugAction::kAdvance;
	if (action.equalsIgnoreCase(kBuiltinDebugActionEntry))
		return BuiltinDebugAction::kEntry;
	if (action.equalsIgnoreCase(kBuiltinDebugActionEntryFirstVisit))
		return BuiltinDebugAction::kEntryFirstVisit;
	return BuiltinDebugAction::kInvalid;
}

bool ZoombiniPuzzleTunnels::runBuiltinDebugAction(BuiltinDebugAction action, Common::String &output) {
	if (action == BuiltinDebugAction::kAcceptance) {
		output = debugGetAnswer();
		const char *acceptanceText = nullptr;
		if (_guardAxisCount == 1)
			acceptanceText = _guardRules[0].traitMatchOnPrimarySide ? "Left accepts:" : "Right accepts:";
		else if (_guardRules[0].traitMatchOnPrimarySide)
			acceptanceText = _guardRules[1].traitMatchOnPrimarySide ? "Left / Top accept:" : "Left / Bottom accept:";
		else
			acceptanceText = _guardRules[1].traitMatchOnPrimarySide ? "Right / Top accept:" : "Right / Bottom accept:";
		showBuiltinDebugText(acceptanceText);
		for (int16 guardIdx = 0; guardIdx < 2; guardIdx++) {
			for (byte conditionIdx = 0; conditionIdx < _guardRules[guardIdx].conditionCount; conditionIdx++) {
				drawBuiltinDebugTraitSprite(Common::Point(250 + 30 * conditionIdx, guardIdx == 0 ? 20 : 70),
											static_cast<byte>(_guardRules[guardIdx].traitKinds[conditionIdx] + 1), _guardRules[guardIdx].traitValues[conditionIdx]);
			}
		}
		return false;
	}
	if (action == BuiltinDebugAction::kCelebrationPlusFour) {
		_celebrationTarget += 4;
		output = Common::String::format("Tunnels celebration target %d.\n", _celebrationTarget);
		return false;
	}
	if (action == BuiltinDebugAction::kIdle) {
		queueGateActorSequence(GateActorSequenceMode::kIdle00);
		output = "Tunnels idle gate-actor sequence started.\n";
		return false;
	}
	if (action == BuiltinDebugAction::kEntryFirstVisit) {
		_vm->_state->getTunnelsEntryActorState()._pageOpenCount = 1;
		queueGateActorSequence(GateActorSequenceMode::kEntry01);
		output = "Tunnels first-visit gate-actor sequence started.\n";
		return false;
	}
	if (action == BuiltinDebugAction::kEntry) {
		queueGateActorSequence(GateActorSequenceMode::kEntry01);
		output = "Tunnels current-visit gate-actor sequence started.\n";
		return false;
	}
	if (action == BuiltinDebugAction::kEndGame) {
		queueGateActorSequence(GateActorSequenceMode::kPostGame02);
		output = "Tunnels end-game gate-actor sequence started.\n";
		return false;
	}
	if (action == BuiltinDebugAction::kAdvance) {
		queueGateActorSequence(GateActorSequenceMode::kAdvance03);
		output = "Tunnels advance gate-actor sequence started.\n";
		return false;
	}

	const char *gatekeeperName = nullptr;
	int16 runnerIdx = -1;
	int16 scrbFirst = 0;
	int16 scrbLast = 0;
	if (action == BuiltinDebugAction::kCycleCrystal) {
		gatekeeperName = "Crystal";
		runnerIdx = 0;
		scrbFirst = kResScrb4000_Reject0;
		scrbLast = kResScrb4038_Advance;
	} else if (action == BuiltinDebugAction::kCycleFerrous) {
		gatekeeperName = "Ferrous";
		runnerIdx = 2;
		scrbFirst = kResScrb4200_Reject0;
		scrbLast = kResScrb4226_Advance;
	} else if (action == BuiltinDebugAction::kCycleIgno) {
		gatekeeperName = "Igno";
		runnerIdx = 3;
		scrbFirst = kResScrb4400_Reject0;
		scrbLast = kResScrb4423_Reject;
	} else if (action == BuiltinDebugAction::kCycleOnyx) {
		gatekeeperName = "Onyx";
		runnerIdx = 1;
		scrbFirst = kResScrb4600_Reject0;
		scrbLast = kResScrb4617_Reject;
	} else {
		return true;
	}

	if (!_gateActorFeatures[runnerIdx]) {
		output = "The selected Tunnels gate runner is unavailable.\n";
		return true;
	}
	_builtinDebugScrbId += 1;
	if (_builtinDebugScrbId < scrbFirst || scrbLast < _builtinDebugScrbId)
		_builtinDebugScrbId = scrbFirst;
	playGateActor(runnerIdx, _builtinDebugScrbId, kGateActorCompletionNone00);
	output = Common::String::format("Tunnels %s SCRB %d loaded.\n", gatekeeperName, _builtinDebugScrbId);
	showBuiltinDebugText(Common::String::format("SCRB id: %d", _builtinDebugScrbId));
	return false;
}

ZmbEventHandleResult ZoombiniPuzzleTunnels::onDebugKeyDown(const Common::KeyState &kbd) {
	if (!kbd.hasFlags(0) && !kbd.hasFlags(Common::KBD_SHIFT))
		return ZmbEventHandleResult::kPassthrough;

	BuiltinDebugAction action = BuiltinDebugAction::kInvalid;
	switch (kbd.ascii) {
	case 'a':
		action = BuiltinDebugAction::kAcceptance;
		break;
	case 'C':
		action = BuiltinDebugAction::kCycleCrystal;
		break;
	case 'F':
		action = BuiltinDebugAction::kCycleFerrous;
		break;
	case 'I':
		action = BuiltinDebugAction::kCycleIgno;
		break;
	case 'O':
		action = BuiltinDebugAction::kCycleOnyx;
		break;
	case 'H':
		action = BuiltinDebugAction::kCelebrationPlusFour;
		break;
	case 't':
		action = BuiltinDebugAction::kIdle;
		break;
	case 'T':
		action = BuiltinDebugAction::kEndGame;
		break;
	case 'e':
		action = BuiltinDebugAction::kAdvance;
		break;
	case 'W':
		action = BuiltinDebugAction::kEntryFirstVisit;
		break;
	case 'w':
		action = BuiltinDebugAction::kEntry;
		break;
	default:
		return ZmbEventHandleResult::kPassthrough;
	}

	Common::String output;
	runBuiltinDebugAction(action, output);
	return ZmbEventHandleResult::kPassthrough;
}

void ZoombiniPuzzleTunnels::loadFeatures() {
	ZoombiniGameState::TunnelsEntryActorState &entryActorState = _vm->_state->getTunnelsEntryActorState();
	entryActorState._pageOpenCount += 1;

	loadNodePath(ZmbResource(ZmbResource::kPage, kResNode1000_WalkNetwork));

	loadTerrainBitmap(kResBitmapTerrain100);

	// Preload shape images
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape0400));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape4000_Gate0));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape4200_Gate1));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape4400_Gate2));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape4600_Gate3));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape5000));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape6000));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape7000));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape9000));

	// Main feature head
	ZmbFeature *mainFeature = createPuzzleMainFeatureHead();

	{
		ZmbFeature *parent = mainFeature;
		for (uint16 i = 0; i < 12; i++)
			parent = loadSubFeature(parent, ZmbResource(ZmbResource::kPage, kResBitmapShape6000), kResScrb6000_Controller + i);
	}

	{
		ZmbFeature *parent = mainFeature;
		for (uint16 i = 0; i < 5; i++)
			parent = loadSubFeature(parent, ZmbResource(ZmbResource::kPage, kResBitmapShape7000), kResScrb7000_MainPath + i);
	}

	{
		ZmbFeature *parent = mainFeature;
		for (uint16 i = 0; i < 7; i++)
			parent = loadSubFeature(parent, ZmbResource(ZmbResource::kPage, kResBitmapShape9000), kResScrb9000_Feedback + i);
	}

	{
		ZmbFeature *parent = mainFeature;
		for (uint16 i = 0; i < 39; i++)
			parent = loadSubFeature(parent, ZmbResource(ZmbResource::kPage, kResBitmapShape4000_Gate0), kResScrb4000_Reject0 + i);
	}

	{
		ZmbFeature *parent = mainFeature;
		for (uint16 i = 0; i < 27; i++)
			parent = loadSubFeature(parent, ZmbResource(ZmbResource::kPage, kResBitmapShape4200_Gate1), kResScrb4200_Reject0 + i);
	}

	{
		ZmbFeature *parent = mainFeature;
		for (uint16 i = 0; i < 24; i++)
			parent = loadSubFeature(parent, ZmbResource(ZmbResource::kPage, kResBitmapShape4400_Gate2), kResScrb4400_Reject0 + i);
	}

	{
		ZmbFeature *parent = mainFeature;
		for (uint16 i = 0; i < 18; i++)
			parent = loadSubFeature(parent, ZmbResource(ZmbResource::kPage, kResBitmapShape4600_Gate3), kResScrb4600_Reject0 + i);
	}

	// Register resource ranges without allocating 73 extra Snoid runners.
	// Group 0 selects script-normal state 9, and group 1 selects script-reject state 8.
	// This state selection is independent of whether the puzzle answer was accepted.
	// Tunnels uses SCRS 8000-8007 for group 0 and SCRS 8500-8564 for group 1.
	registerScrsGroup(kResScrs8000_RejectBase, 8);
	registerScrsGroup(kResScrs8500_NormalBase, 65);

	// Feedback animation runner (SCRB 9000)
	_laneLinkAnchorFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape9000), kResScrb9000_Feedback, 0, ZmbFeature::FLAG_00008000_LOOP_ANIM);

	// 4 tunnel entrance runners at predefined positions
	for (int16 featureIdx = 0; featureIdx < 4; featureIdx++) {
		_tunnelEntryFeatures[featureIdx] = loadScrbFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape5000),
			kResScrb5000_TunnelEntryBase + featureIdx, 6,
			kTunnelEntryPositions[featureIdx],
			ZmbFeature::FLAG_00002000_DRAW_ON_REG | ZmbFeature::FLAG_00008000_LOOP_ANIM |
				ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_01000000_DEFER_RENDER);
	}

	// Falling-rock warning runner (SCRB 7001-7004).
	_rockfallWarningFeature = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape7000), kResScrb7001_RockfallBase, 6,
		ZmbFeature::FLAG_08000000_REGION_TRACK | ZmbFeature::FLAG_04000000_OVERLAY |
			ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_00080000_DEFER_ANIM);

	// Gate actors
	for (int16 i = 0; i < 4; i++) {
		int16 actorSlot = kGateActorSlotByEntry[i];
		_gateActorFeatures[actorSlot] = loadScrbFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape6000), kResScrb6000_Controller + actorSlot, 6,
			ZmbFeature::FLAG_08000000_REGION_TRACK | ZmbFeature::FLAG_04000000_OVERLAY |
				ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_00080000_DEFER_ANIM);
	}

	// Anonymous visual feedback runners (SCRB 9001-9006)
	for (uint16 i = 0; i < 6; i++) {
		loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape9000), kResScrb9001_FeedbackBase + i, 6, ZmbFeature::FLAG_00000000_TYPE_SHAPES);
	}

	// Main path runner (SCRB 7000) -- topmost overlay
	_mainPathFeature = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape7000), kResScrb7000_MainPath, 6,
		ZmbFeature::FLAG_08000000_REGION_TRACK | ZmbFeature::FLAG_04000000_OVERLAY |
			ZmbFeature::FLAG_01000000_DEFER_RENDER | ZmbFeature::FLAG_00100000_PLAY_ONCE |
			ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00001000_TOPMOST);

	// Load Zoombinis at 16 pedestal positions
	loadZoombinisFromPack();

	// Layout and stagger walk-in
	layoutStaticAndWalkIn(100, false);
	renderFeatures();
	assignStaggeredWalkDelays(30, 45);

	// Generate tunnel rules from the materialized active pack.
	generateRules();

	// Buttons
	configureStandardPuzzleControlRects();
	loadStandardPuzzleControlFeatures(kResBitmapShape0400);
	setGoButtonsEnabled(false);

	// Do not inherit acceptance from the previous puzzle.
	// Only successful lane arrivals mark their slots occupied.
	schedulePackSnoids(false, false);
}

void ZoombiniPuzzleTunnels::initHelpPrompt() {
	_activeHelpSoundId = ZmbResource(ZmbResource::kSystem, _vm->_rnd->getRandomNumber(kSysResSound20069_HelpEasy, kSysResSound20070_HelpHard));
}

void ZoombiniPuzzleTunnels::activatePage() {
	// These two draws follow the help selection in the authored setup RNG order.
	queueGateActorSequence(GateActorSequenceMode::kEntry01);
	_idleActorDelay = _vm->_rnd->getRandomNumber(5400, 10800);
	_idleActorDeadline = getCurrentFrameCounter() + _idleActorDelay;
	ZoombiniPage::activatePage();
}

// =========================================================================
// Zoombini Loading
// =========================================================================

void ZoombiniPuzzleTunnels::loadZoombinisFromPack() {
	ZoombiniPuzzle::loadZoombinisFromPack(kSnoidPositions, ARRAYSIZE(kSnoidPositions));

	// Set the remaining target from difficulty rather than the loaded pack size.
	// As it falls through 4, 3, 2, and 1, it drives sound cues 4700-4703.
	int16 perLevelTarget;
	switch (_difficultyLevel) {
	case kPuzzleLevel2:
		perLevelTarget = 18;
		break;
	case kPuzzleLevel3:
		perLevelTarget = 20;
		break;
	case kPuzzleLevel4:
		perLevelTarget = 22;
		break;
	default:
		perLevelTarget = 16;
		break;
	}
	_remainingRejectChances = perLevelTarget;
}

// =========================================================================
// Rule Generation
// =========================================================================

void ZoombiniPuzzleTunnels::generateRules() {
	switch (_difficultyLevel) {
	case kPuzzleLevel1:
		setupLevel1_singleTrait();
		break;
	case kPuzzleLevel2:
		setupLevel2_dualSingleTrait();
		break;
	case kPuzzleLevel3:
		setupLevel3_dualDoubleTrait();
		break;
	case kPuzzleLevel4:
		setupLevel4_crossCategoryTrait();
		break;
	default:
		setupLevel1_singleTrait();
		break;
	}
}

// ---------------------------------------------------------------------------
// Build the L1 single-trait rule set.
// ---------------------------------------------------------------------------
void ZoombiniPuzzleTunnels::setupLevel1_singleTrait() {
	Common::Array<ZmbTrait> traits;
	for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
		ZmbSnoid *snoid = *it;
		if (snoid && snoid->hasFlag(ZmbFeature::FLAG_00000001_TYPE_SNOID))
			traits.push_back(snoid->_trait);
	}

	// Build 20-element match count table: 4 categories x 5 values
	int16 matchCounts[20] = {};
	for (const ZmbTrait &trait : traits) {
		for (int16 i = 0; i < 4; i++) {
			byte val = trait[i] & 0x0F;
			if (1 <= val && val <= 5)
				matchCounts[5 * i + (val - 1)] += 1;
		}
	}

	// The active Bridge handoff excludes Tunnels Level 1 candidates with the
	// same matching Zoombini count when another nonzero split is available.
	if (_vm->_bridgeTunnelsLevel1Exclusion.isActive()) {
		const int16 excludedSplitCount = _vm->_bridgeTunnelsLevel1Exclusion.getSplitCount();
		bool hasAlternateSplit = false;
		for (int16 slot = 0; slot < 20 && !hasAlternateSplit; slot++) {
			hasAlternateSplit = matchCounts[slot] != 0 &&
								matchCounts[slot] != excludedSplitCount;
		}
		if (hasAlternateSplit) {
			for (int16 slot = 0; slot < 20; slot++) {
				if (matchCounts[slot] == excludedSplitCount)
					matchCounts[slot] = 0;
			}
		}
	}

	// The search is deliberately asymmetric.
	// Its deltas repeat 1, -2, 1, -2, ... rather than expanding by distance.
	int16 targetCount = traits.size() / 2;
	Common::Array<int16> candidates;
	int16 searchDelta = 1;
	int16 checkVal = targetCount;

	while (candidates.empty()) {
		if (1 <= checkVal && checkVal < 16) {
			for (int slot = 0; slot < 20; slot++) {
				if (matchCounts[slot] == checkVal)
					candidates.push_back(slot);
			}
		}
		checkVal += searchDelta;
		searchDelta = -(searchDelta + 1);
	}

	int16 selection = static_cast<int16>(_vm->_rnd->getRandomNumber(1, candidates.size()));
	int16 bestSlot = candidates[selection - 1];

	const ZmbTrait::TraitKind traitKind = static_cast<ZmbTrait::TraitKind>(bestSlot / 5);
	byte traitValue = (bestSlot % 5) + 1;

	_guardAxisCount = 1;
	_guardRules[0].traitMatchOnPrimarySide = _vm->_rnd->getRandomBool();
	_guardRules[0].conditionCount = 1;
	_guardRules[0].traitKinds[0] = traitKind;
	_guardRules[0].traitValues[0] = traitValue;
	debugC(3, MohawkEngine_Zoombini::kDebugPage02, "tunnels: generated level 1 guard rule with side %d, trait kind %d, and value %d",
		   _guardRules[0].traitMatchOnPrimarySide ? 1 : 0, static_cast<int>(traitKind) + 1, traitValue);
}

// ---------------------------------------------------------------------------
// Build the L2 paired single-trait rule set.
// ---------------------------------------------------------------------------
void ZoombiniPuzzleTunnels::setupLevel2_dualSingleTrait() {
	// Build 20 single-trait descriptors from 4 categories x 5 values.
	// Enumerate all 400 descriptor pairs.
	// Select the pair with maximum diversity and minimum balance score across the loaded Snoids.
	Common::Array<ZmbTrait> traits;
	for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
		ZmbSnoid *snoid = *it;
		if (snoid && snoid->hasFlag(ZmbFeature::FLAG_00000001_TYPE_SNOID))
			traits.push_back(snoid->_trait);
	}

	TraitRuleDescriptor descriptors[20];
	int16 descriptorIdx = 0;
	for (int16 ruleSlot = 0; ruleSlot < 4; ruleSlot++) {
		const ZmbTrait::TraitKind traitKind = traitKindFromRuleSlot(ruleSlot);
		for (byte traitValue = 1; traitValue <= 5; traitValue += 1) {
			descriptors[descriptorIdx].setFirstValue(traitKind, traitValue);
			descriptorIdx += 1;
		}
	}

	const int16 pairCount = 400;
	int16 bothMatch[400] = {};
	int16 aOnlyMatch[400] = {};
	int16 bOnlyMatch[400] = {};
	int16 neitherMatch[400] = {};

	for (const ZmbTrait &trait : traits) {
		for (int16 pairIdx = 0; pairIdx < pairCount; pairIdx++) {
			int16 descA = pairIdx / 20;
			int16 descB = pairIdx % 20;
			if (descA == descB)
				continue;

			bool mA = matchLevel2Descriptor(trait, descriptors[descA]);
			bool mB = matchLevel2Descriptor(trait, descriptors[descB]);

			if (mA && mB)
				bothMatch[pairIdx] += 1;
			else if (mA)
				aOnlyMatch[pairIdx] += 1;
			else if (mB)
				bOnlyMatch[pairIdx] += 1;
			else
				neitherMatch[pairIdx] += 1;
		}
	}

	int16 bestDiversity = 0;
	for (int16 pathIdx = 0; pathIdx < pairCount; pathIdx++) {
		int16 diversity = 0;
		if (0 < bothMatch[pathIdx])
			diversity += 1;
		if (0 < aOnlyMatch[pathIdx])
			diversity += 1;
		if (0 < bOnlyMatch[pathIdx])
			diversity += 1;
		if (0 < neitherMatch[pathIdx])
			diversity += 1;
		if (bestDiversity < diversity)
			bestDiversity = diversity;
	}

	int16 balanceScore[400];
	for (int16 pathIdx = 0; pathIdx < pairCount; pathIdx++) {
		balanceScore[pathIdx] = -1;
		int16 diversity = 0;
		if (0 < bothMatch[pathIdx])
			diversity += 1;
		if (0 < aOnlyMatch[pathIdx])
			diversity += 1;
		if (0 < bOnlyMatch[pathIdx])
			diversity += 1;
		if (0 < neitherMatch[pathIdx])
			diversity += 1;
		if (diversity < bestDiversity)
			continue;
		int16 a = bothMatch[pathIdx], b = aOnlyMatch[pathIdx], c = bOnlyMatch[pathIdx], d = neitherMatch[pathIdx];
		balanceScore[pathIdx] = ABS(a - b) + ABS(a - c) + ABS(a - d) +
								ABS(b - c) + ABS(b - d) + ABS(c - d);
	}

	int16 minBalance = 32000;
	for (int16 pathIdx = 0; pathIdx < pairCount; pathIdx++) {
		if (0 <= balanceScore[pathIdx] && balanceScore[pathIdx] < minBalance)
			minBalance = balanceScore[pathIdx];
	}

	int16 candidateCount = 0;
	for (int16 pathIdx = 0; pathIdx < pairCount; pathIdx++) {
		if (balanceScore[pathIdx] == minBalance)
			candidateCount += 1;
	}

	int16 selectedPair = 0;
	int16 selection = static_cast<int16>(_vm->_rnd->getRandomNumber(1, candidateCount));
	for (int16 pathIdx = 0; pathIdx < pairCount; pathIdx++) {
		if (balanceScore[pathIdx] == minBalance) {
			selection -= 1;
			if (selection == 0) {
				selectedPair = pathIdx;
				break;
			}
		}
	}
	const TraitRuleDescriptor &descriptorA = descriptors[selectedPair / 20];
	const TraitRuleDescriptor &descriptorB = descriptors[selectedPair % 20];

	_guardAxisCount = 2;
	_guardRules[0].traitMatchOnPrimarySide = _vm->_rnd->getRandomBool();
	_guardRules[0].conditionCount = 1;
	descriptorToGuard(descriptorA, _guardRules[0]);
	_guardRules[1].traitMatchOnPrimarySide = _vm->_rnd->getRandomBool();
	_guardRules[1].conditionCount = 1;
	descriptorToGuard(descriptorB, _guardRules[1]);
	debugC(3, MohawkEngine_Zoombini::kDebugPage02, "tunnels: generated level 2 guard rules with traits %d/%d and %d/%d",
		   static_cast<int>(_guardRules[0].traitKinds[0]) + 1, _guardRules[0].traitValues[0],
		   static_cast<int>(_guardRules[1].traitKinds[0]) + 1, _guardRules[1].traitValues[0]);
}

// ---------------------------------------------------------------------------
// Build the L3 paired double-trait rule set.
// ---------------------------------------------------------------------------
void ZoombiniPuzzleTunnels::setupLevel3_dualDoubleTrait() {
	static constexpr TraitValueSet kAlternativeValueSetTable[10] = {
		{2, 1}, {3, 1}, {4, 1}, {5, 1}, {3, 2}, {4, 2}, {5, 2}, {4, 3}, {5, 3}, {5, 4}};

	Common::Array<ZmbTrait> traits;
	for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
		ZmbSnoid *snoid = *it;
		if (snoid && snoid->hasFlag(ZmbFeature::FLAG_00000001_TYPE_SNOID))
			traits.push_back(snoid->_trait);
	}

	// Build 40 descriptors: 10 value pairs for each trait category.
	TraitRuleDescriptor descriptors[40];
	int16 descriptorIdx = 0;
	for (int16 ruleSlot = 0; ruleSlot < 4; ruleSlot++) {
		const ZmbTrait::TraitKind traitKind = traitKindFromRuleSlot(ruleSlot);
		for (int16 valuePairIdx = 0; valuePairIdx < 10; valuePairIdx++) {
			descriptors[descriptorIdx].setValues(traitKind, kAlternativeValueSetTable[valuePairIdx]);
			descriptorIdx += 1;
		}
	}

	// Bucket arrays for pair scoring
	int16 bothMatch[1600] = {};
	int16 aOnlyMatch[1600] = {};
	int16 bOnlyMatch[1600] = {};
	int16 neitherMatch[1600] = {};

	for (const ZmbTrait &trait : traits) {
		for (int pairIdx = 0; pairIdx < 1600; pairIdx++) {
			int descA = pairIdx / 40;
			int descB = pairIdx % 40;
			if (descA == descB)
				continue;

			bool mA = matchLevel3Descriptor(trait, descriptors[descA]);
			bool mB = matchLevel3Descriptor(trait, descriptors[descB]);

			if (mA && mB)
				bothMatch[pairIdx] += 1;
			else if (mA)
				aOnlyMatch[pairIdx] += 1;
			else if (mB)
				bOnlyMatch[pairIdx] += 1;
			else
				neitherMatch[pairIdx] += 1;
		}
	}

	// Find best diversity
	int16 bestDiversity = 0;
	for (int pathIdx = 0; pathIdx < 1600; pathIdx++) {
		int16 diversity = 0;
		if (0 < bothMatch[pathIdx])
			diversity += 1;
		if (0 < aOnlyMatch[pathIdx])
			diversity += 1;
		if (0 < bOnlyMatch[pathIdx])
			diversity += 1;
		if (0 < neitherMatch[pathIdx])
			diversity += 1;
		if (bestDiversity < diversity)
			bestDiversity = diversity;
	}

	// Calculate balance scores
	int16 balanceScore[1600];
	for (int pathIdx = 0; pathIdx < 1600; pathIdx++) {
		balanceScore[pathIdx] = -1;
		int16 diversity = 0;
		if (0 < bothMatch[pathIdx])
			diversity += 1;
		if (0 < aOnlyMatch[pathIdx])
			diversity += 1;
		if (0 < bOnlyMatch[pathIdx])
			diversity += 1;
		if (0 < neitherMatch[pathIdx])
			diversity += 1;
		if (diversity < bestDiversity)
			continue;

		int16 a = bothMatch[pathIdx], b = aOnlyMatch[pathIdx], c = bOnlyMatch[pathIdx], d = neitherMatch[pathIdx];
		balanceScore[pathIdx] = ABS(a - b) + ABS(a - c) + ABS(a - d) +
								ABS(b - c) + ABS(b - d) + ABS(c - d);
	}

	int16 minBalance = 32000;
	for (int pathIdx = 0; pathIdx < 1600; pathIdx++) {
		if (0 <= balanceScore[pathIdx] && balanceScore[pathIdx] < minBalance)
			minBalance = balanceScore[pathIdx];
	}

	int16 candidateCount = 0;
	for (int pathIdx = 0; pathIdx < 1600; pathIdx++) {
		if (balanceScore[pathIdx] == minBalance)
			candidateCount += 1;
	}

	int16 selection = _vm->_rnd->getRandomNumber(1, candidateCount);
	int16 selectedPair = 0;
	for (int pathIdx = 0; pathIdx < 1600; pathIdx++) {
		if (balanceScore[pathIdx] == minBalance) {
			selection -= 1;
			if (selection == 0) {
				selectedPair = pathIdx;
				break;
			}
		}
	}
	const TraitRuleDescriptor &descriptorA = descriptors[selectedPair / 40];
	const TraitRuleDescriptor &descriptorB = descriptors[selectedPair % 40];

	_guardAxisCount = 2;
	for (int groupIdx = 0; groupIdx < 2; groupIdx++) {
		const TraitRuleDescriptor &descriptor = (groupIdx == 0) ? descriptorA : descriptorB;
		_guardRules[groupIdx].traitMatchOnPrimarySide = _vm->_rnd->getRandomBool();
		_guardRules[groupIdx].conditionCount = 2;
		for (int16 ruleSlot = 0; ruleSlot < 4; ruleSlot++) {
			const ZmbTrait::TraitKind traitKind = traitKindFromRuleSlot(ruleSlot);
			const TraitValueSet acceptedValues = descriptor.getValues(traitKind);
			if (!acceptedValues.isEmpty()) {
				_guardRules[groupIdx].traitKinds[0] = traitKind;
				_guardRules[groupIdx].traitValues[0] = acceptedValues.firstValue;
				_guardRules[groupIdx].traitKinds[1] = traitKind;
				_guardRules[groupIdx].traitValues[1] = acceptedValues.secondValue;
				break;
			}
		}
	}
	debugC(3, MohawkEngine_Zoombini::kDebugPage02, "tunnels: generated level 3 guard rules with values %d/%d and %d/%d",
		   _guardRules[0].traitValues[0], _guardRules[0].traitValues[1], _guardRules[1].traitValues[0], _guardRules[1].traitValues[1]);
}

// ---------------------------------------------------------------------------
// Build the level 4 cross-category rule set.
// ---------------------------------------------------------------------------
void ZoombiniPuzzleTunnels::setupLevel4_crossCategoryTrait() {
	static constexpr TraitKindPair kTraitKindPairs[6] = {
		{ZmbTrait::kTraitFeet, ZmbTrait::kTraitNose},
		{ZmbTrait::kTraitFeet, ZmbTrait::kTraitEyes},
		{ZmbTrait::kTraitFeet, ZmbTrait::kTraitHair},
		{ZmbTrait::kTraitNose, ZmbTrait::kTraitEyes},
		{ZmbTrait::kTraitNose, ZmbTrait::kTraitHair},
		{ZmbTrait::kTraitEyes, ZmbTrait::kTraitHair}};

	Common::Array<ZmbTrait> traits;
	for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
		ZmbSnoid *snoid = *it;
		if (snoid && snoid->hasFlag(ZmbFeature::FLAG_00000001_TYPE_SNOID))
			traits.push_back(snoid->_trait);
	}

	// Build 150 cross-category descriptors.
	TraitRuleDescriptor descriptors[150];
	int16 descriptorIdx = 0;
	for (int16 pairIdx = 0; pairIdx < 6; pairIdx++) {
		const TraitKindPair &traitKinds = kTraitKindPairs[pairIdx];
		for (byte firstTraitValue = 1; firstTraitValue <= 5; firstTraitValue += 1) {
			for (byte secondTraitValue = 1; secondTraitValue <= 5; secondTraitValue += 1) {
				descriptors[descriptorIdx].setFirstValue(traitKinds.firstKind, firstTraitValue);
				descriptors[descriptorIdx].setFirstValue(traitKinds.secondKind, secondTraitValue);
				descriptorIdx += 1;
			}
		}
	}

	// Bucket arrays (heap-allocated for 22500 entries)
	Common::Array<int16> bothMatch(22500, 0);
	Common::Array<int16> aOnlyMatch(22500, 0);
	Common::Array<int16> bOnlyMatch(22500, 0);
	Common::Array<int16> neitherMatch(22500, 0);

	for (const ZmbTrait &trait : traits) {
		for (int pairIdx = 0; pairIdx < 22500; pairIdx++) {
			int descAIdx = pairIdx / 150;
			int descBIdx = pairIdx % 150;
			if (descAIdx == descBIdx)
				continue;

			bool mA = matchLevel4Descriptor(trait, descriptors[descAIdx]);
			bool mB = matchLevel4Descriptor(trait, descriptors[descBIdx]);

			if (mA && mB)
				bothMatch[pairIdx] += 1;
			else if (mA)
				aOnlyMatch[pairIdx] += 1;
			else if (mB)
				bOnlyMatch[pairIdx] += 1;
			else
				neitherMatch[pairIdx] += 1;
		}
	}

	int16 bestDiversity = 0;
	for (int pathIdx = 0; pathIdx < 22500; pathIdx++) {
		int16 diversity = 0;
		if (0 < bothMatch[pathIdx])
			diversity += 1;
		if (0 < aOnlyMatch[pathIdx])
			diversity += 1;
		if (0 < bOnlyMatch[pathIdx])
			diversity += 1;
		if (0 < neitherMatch[pathIdx])
			diversity += 1;
		if (bestDiversity < diversity)
			bestDiversity = diversity;
	}

	Common::Array<int16> balanceScore(22500, -1);
	for (int pathIdx = 0; pathIdx < 22500; pathIdx++) {
		int16 diversity = 0;
		if (0 < bothMatch[pathIdx])
			diversity += 1;
		if (0 < aOnlyMatch[pathIdx])
			diversity += 1;
		if (0 < bOnlyMatch[pathIdx])
			diversity += 1;
		if (0 < neitherMatch[pathIdx])
			diversity += 1;
		if (diversity < bestDiversity)
			continue;

		int16 a = bothMatch[pathIdx], b = aOnlyMatch[pathIdx], c = bOnlyMatch[pathIdx], d = neitherMatch[pathIdx];
		balanceScore[pathIdx] = ABS(a - b) + ABS(a - c) + ABS(a - d) +
								ABS(b - c) + ABS(b - d) + ABS(c - d);
	}

	int16 minBalance = 32000;
	for (int pathIdx = 0; pathIdx < 22500; pathIdx++) {
		if (0 <= balanceScore[pathIdx] && balanceScore[pathIdx] < minBalance)
			minBalance = balanceScore[pathIdx];
	}

	int16 candidateCount = 0;
	for (int pathIdx = 0; pathIdx < 22500; pathIdx++) {
		if (balanceScore[pathIdx] == minBalance)
			candidateCount += 1;
	}

	int16 sel = _vm->_rnd->getRandomNumber(1, candidateCount);
	int16 selectedPair = 0;
	for (int pathIdx = 0; pathIdx < 22500; pathIdx++) {
		if (balanceScore[pathIdx] == minBalance) {
			sel -= 1;
			if (sel == 0) {
				selectedPair = pathIdx;
				break;
			}
		}
	}
	const TraitRuleDescriptor &descriptorA = descriptors[selectedPair / 150];
	const TraitRuleDescriptor &descriptorB = descriptors[selectedPair % 150];

	_guardAxisCount = 2;
	for (int groupIdx = 0; groupIdx < 2; groupIdx++) {
		const TraitRuleDescriptor &descriptor = (groupIdx == 0) ? descriptorA : descriptorB;
		_guardRules[groupIdx].traitMatchOnPrimarySide = _vm->_rnd->getRandomBool();
		_guardRules[groupIdx].conditionCount = 2;
		int condIdx = 0;
		for (int16 ruleSlot = 0; ruleSlot < 4 && condIdx < 2; ruleSlot++) {
			const ZmbTrait::TraitKind traitKind = traitKindFromRuleSlot(ruleSlot);
			const TraitValueSet acceptedValues = descriptor.getValues(traitKind);
			if (!acceptedValues.isEmpty()) {
				_guardRules[groupIdx].traitKinds[condIdx] = traitKind;
				_guardRules[groupIdx].traitValues[condIdx] = acceptedValues.firstValue;
				condIdx += 1;
			}
		}
	}
	debugC(3, MohawkEngine_Zoombini::kDebugPage02, "tunnels: generated level 4 guard rules with kinds %d/%d and %d/%d",
		   static_cast<int>(_guardRules[0].traitKinds[0]) + 1, static_cast<int>(_guardRules[0].traitKinds[1]) + 1,
		   static_cast<int>(_guardRules[1].traitKinds[0]) + 1, static_cast<int>(_guardRules[1].traitKinds[1]) + 1);
}

ZoombiniPuzzleTunnels::TraitValueSet ZoombiniPuzzleTunnels::TraitRuleDescriptor::getValues(ZmbTrait::TraitKind traitKind) const {
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
		error("tunnels: TraitRuleDescriptor::getValues: invalid trait kind %d", static_cast<int>(traitKind));
		return ZoombiniPuzzleTunnels::TraitValueSet();
	}
}

void ZoombiniPuzzleTunnels::TraitRuleDescriptor::setFirstValue(ZmbTrait::TraitKind traitKind, byte traitValue) {
	const TraitValueSet values = {traitValue, 0};
	setValues(traitKind, values);
}

void ZoombiniPuzzleTunnels::TraitRuleDescriptor::setValues(ZmbTrait::TraitKind traitKind, const TraitValueSet &values) {
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
		error("tunnels: TraitRuleDescriptor::setValues: invalid trait kind %d", static_cast<int>(traitKind));
		break;
	}
}

ZmbTrait::TraitKind ZoombiniPuzzleTunnels::traitKindFromRuleSlot(int16 slot) {
	static constexpr ZmbTrait::TraitKind kRuleSlotTraitKinds[4] = {
		ZmbTrait::kTraitFeet,
		ZmbTrait::kTraitNose,
		ZmbTrait::kTraitEyes,
		ZmbTrait::kTraitHair};
	return kRuleSlotTraitKinds[slot];
}

bool ZoombiniPuzzleTunnels::matchLevel2Descriptor(const ZmbTrait &trait, const TraitRuleDescriptor &descriptor) {
	for (int16 ruleSlot = 0; ruleSlot < 4; ruleSlot++) {
		const ZmbTrait::TraitKind traitKind = traitKindFromRuleSlot(ruleSlot);
		const TraitValueSet acceptedValues = descriptor.getValues(traitKind);
		if (!acceptedValues.isEmpty() && (trait.getTraitValue(traitKind) & 0x0F) == acceptedValues.firstValue)
			return true;
	}
	return false;
}

bool ZoombiniPuzzleTunnels::matchLevel3Descriptor(const ZmbTrait &trait, const TraitRuleDescriptor &descriptor) {
	for (int16 ruleSlot = 0; ruleSlot < 4; ruleSlot++) {
		const ZmbTrait::TraitKind traitKind = traitKindFromRuleSlot(ruleSlot);
		const TraitValueSet acceptedValues = descriptor.getValues(traitKind);
		const byte traitValue = trait.getTraitValue(traitKind) & 0x0F;
		if (!acceptedValues.isEmpty()) {
			return traitValue == acceptedValues.firstValue || traitValue == acceptedValues.secondValue;
		}
	}
	return false;
}

bool ZoombiniPuzzleTunnels::matchLevel4Descriptor(const ZmbTrait &trait, const TraitRuleDescriptor &descriptor) {
	for (int16 ruleSlot = 0; ruleSlot < 4; ruleSlot++) {
		const ZmbTrait::TraitKind traitKind = traitKindFromRuleSlot(ruleSlot);
		const TraitValueSet acceptedValues = descriptor.getValues(traitKind);
		const byte traitValue = trait.getTraitValue(traitKind) & 0x0F;
		if (!acceptedValues.isEmpty() && traitValue == acceptedValues.firstValue)
			return true;
	}
	return false;
}

void ZoombiniPuzzleTunnels::descriptorToGuard(const TraitRuleDescriptor &descriptor, TunnelGuardRule &guard) {
	// Each level-2 descriptor has exactly one populated trait category.
	for (int16 ruleSlot = 0; ruleSlot < 4; ruleSlot++) {
		const ZmbTrait::TraitKind traitKind = traitKindFromRuleSlot(ruleSlot);
		const TraitValueSet acceptedValues = descriptor.getValues(traitKind);
		if (!acceptedValues.isEmpty()) {
			guard.traitKinds[0] = traitKind;
			guard.traitValues[0] = acceptedValues.firstValue;
			return;
		}
	}
}

bool ZoombiniPuzzleTunnels::evaluateGuard(const TunnelGuardRule &guard, const ZmbTrait &trait) {
	if (guard.conditionCount == 0)
		return false;
	for (byte traitIdx = 0; traitIdx < guard.conditionCount; traitIdx++) {
		if ((trait.getTraitValue(guard.traitKinds[traitIdx]) & 0x0F) == guard.traitValues[traitIdx])
			return true;
	}
	return false;
}

Common::String ZoombiniPuzzleTunnels::formatGuardTraitDescription(const TunnelGuardRule &guard) {
	Common::String description;
	for (int16 conditionIdx = 0; conditionIdx < guard.conditionCount && conditionIdx < 2; conditionIdx++) {
		const ZmbTrait::TraitKind traitKind = guard.traitKinds[conditionIdx];
		const byte traitValue = guard.traitValues[conditionIdx];
		if (0 < conditionIdx)
			description += " or ";
		description += Common::String::format("%s: %s", ZmbTrait::debugTraitKindName(traitKind), ZmbTrait::debugTraitValueName(traitKind, traitValue));
	}
	return description;
}

// =========================================================================
// Evaluate a tunnel rule.
//
// Return true when the rule is not satisfied and the Snoid must be rejected.
// Record which side matched guard A for the selected zone.
// =========================================================================

bool ZoombiniPuzzleTunnels::evaluateRule(ZmbSnoid *snoid, int16 dropZone, bool &guardSideMatch) {
	guardSideMatch = false;
	if (!snoid)
		return true;
	if (dropZone < 1 || 4 < dropZone)
		dropZone = 1;

	// Evaluate guard A.
	bool guardAMatch = false;
	if (1 <= _guardAxisCount && 1 <= _guardRules[0].conditionCount) {
		guardAMatch = evaluateGuard(_guardRules[0], snoid->_trait);
		if (!_guardRules[0].traitMatchOnPrimarySide)
			guardAMatch = !guardAMatch;
	}

	// Single-guard mode (level 1)
	if (_guardAxisCount == 1) {
		// Zones 1/2 form the guard-A-match pair, while zones 3/4 form the opposite pair.
		// The level-1 blocked-pair toggle later removes one entry from each pair, leaving one answer.
		bool result = dropZone <= 2 ? guardAMatch : !guardAMatch;
		guardSideMatch = result;
		return !result; // Return true = rejection (rule NOT satisfied)
	}

	// Evaluate guard B.
	bool guardBMatch = false;
	if (2 <= _guardAxisCount && 1 <= _guardRules[1].conditionCount) {
		guardBMatch = evaluateGuard(_guardRules[1], snoid->_trait);
		if (!_guardRules[1].traitMatchOnPrimarySide)
			guardBMatch = !guardBMatch;
	}

	// Zone logic
	bool result;
	switch (dropZone) {
	case 1:
		guardSideMatch = guardAMatch;
		result = (2 <= _guardAxisCount) ? (guardAMatch && guardBMatch) : guardAMatch;
		break;
	case 2:
		guardSideMatch = guardAMatch;
		result = guardAMatch && !guardBMatch;
		break;
	case 3:
		guardSideMatch = !guardAMatch;
		result = !guardAMatch && !guardBMatch;
		break;
	case 4:
		guardSideMatch = !guardAMatch;
		result = !guardAMatch && guardBMatch;
		break;
	default:
		result = false;
		break;
	}

	return !result; // true = rejection
}

// =========================================================================
// Identify the drop zone at a point.
// =========================================================================

int16 ZoombiniPuzzleTunnels::getDropZone(const Common::Point &pos) {
	// Accept only an empty DRAW_ON_REG slot.
	// A queued Snoid owns its seat until it is picked up again or its lane approach starts.
	int16 slot = hitTestDrawOnRegSlot(pos, _clickZoneRadius, true);
	return 0 <= slot ? slot + 1 : 0;
}

// =========================================================================
// Commit a zoombini placement.
// =========================================================================

void ZoombiniPuzzleTunnels::handleZoombiniPlacement(ZmbSnoid *snoid, int16 zone,
													bool isRejection, bool guardAMatch) {
	if (!snoid || zone < 1 || 4 < zone)
		return;

	// Compute hoverData from zone + guardAMatch
	int16 hoverData = 0;
	switch (zone) {
	case 1:
		hoverData = guardAMatch ? 1 : 0;
		break;
	case 2:
		hoverData = guardAMatch ? 3 : 2;
		break;
	case 3:
		hoverData = guardAMatch ? 4 : 5;
		break;
	case 4:
		hoverData = guardAMatch ? 6 : 7;
		break;
	}

	// Apply the level-1 blocked entrance pair.
	if (!isRejection && _difficultyLevel == kPuzzleLevel1) {
		if (_level1BlockedPairToggle) {
			if (zone == 1 || zone == 4)
				isRejection = true;
		} else {
			if (zone == 2 || zone == 3)
				isRejection = true;
		}
	}

	int16 gateType = kHoverDataToGateType[hoverData];
	int16 feet = snoid->_trait._feet & 0x0F;
	feet = CLIP<int16>(feet, 1, 5);

	// Hint animation (set regardless of rejection --)
	int16 hintRunner = -1; // Gate-actor slot that presents the optional hint.
	int16 hintScrb = 0;    // Hint SCRB for that gate actor.
	if (guardAMatch) {
		if (4 <= hoverData) {
			hintRunner = 3; // Igno actor
			hintScrb = kCorrectHintLargeScrbPool[_vm->_rnd->getNonRepeatRandom(6, _ignoHintPoolState)];
		} else {
			hintRunner = 0; // Crystal actor
			hintScrb = kCorrectHintSmallScrbPool[_vm->_rnd->getNonRepeatRandom(11, _crystalHintPoolState)];
		}
	}

	int16 walkScrsId = 0;           // walking/approach SCRS
	int16 rejectScrsId = 0;         // snoid SCRS for gate interaction
	int16 primaryRunner = gateType; // Primary gate-actor slot.
	int16 primaryScrb = 0;          // Initial SCRB for the primary gate actor.
	int16 secondaryScrb = 0;        // Follow-up SCRB for the primary gate actor.
	int16 secondaryRunner = hintRunner;
	int16 secondaryScrb1 = hintScrb;
	int16 secondaryScrb2 = 0;

	if (isRejection) {
		// === REJECTION PATH ===
		_zoneCorrectStreak[zone] = 0; // Reset streak for this zone

		primaryScrb = hoverData + kResScrb6004_DoorRejectBase; // Gate rejection controller SCRB.

		// Select the gate-specific rejection feedback SCRB.
		switch (gateType) {
		case 0:
			_crystalRejectCountSinceSuccess += 1;
			do {
				secondaryScrb = kRejectScrbPool0[_vm->_rnd->getNonRepeatRandom(10, _crystalRejectPoolState)];
			} while (secondaryScrb == kResScrb4005_Reject5 && _crystalRejectCountSinceSuccess < 3);
			break;
		case 1:
			secondaryScrb = kRejectGateScrbPool1[_vm->_rnd->getNonRepeatRandom(4, _onyxRejectPoolState)];
			break;
		case 2:
			secondaryScrb = kRejectScrbPool1[_vm->_rnd->getNonRepeatRandom(8, _ferrousRejectPoolState)];
			break;
		case 3:
			_ignoRejectCountSinceSuccess += 1;
			do {
				secondaryScrb = kRejectScrbPool3[_vm->_rnd->getNonRepeatRandom(7, _ignoRejectPoolState)];
			} while (secondaryScrb == kResScrb4416_Reject && _ignoRejectCountSinceSuccess < 3);
			break;
		}

		walkScrsId = feet + 5 * hoverData + kResScrs8519_RejectWalkBase;
		rejectScrsId = hoverData + kResScrs8000_RejectBase;
	} else {
		// === SUCCESS PATH ===
		_crystalRejectCountSinceSuccess = 0;
		_ignoRejectCountSinceSuccess = 0;
		_zoneCorrectStreak[zone] += 1;

		// Select success-path feedback SCRB.
		if (hoverData == 1 || hoverData == 6) {
			primaryScrb = kRejectScrbPool4[_vm->_rnd->getNonRepeatRandom(6, _onyxCorrectPoolState)];
		} else if (hoverData == 3 || hoverData == 4) {
			primaryScrb = kRejectScrbPool2[_vm->_rnd->getNonRepeatRandom(8, _ferrousCorrectPoolState)];
		}

		walkScrsId = hoverData / 2 + 4 * feet + kResScrs8496_WalkBase;
		rejectScrsId = 0;

		// If this would be the last zoombini, clear hint fields
		if (_pageLoadedZmbCount <= _enteredSnoidCount + 1) {
			secondaryScrb2 = 0;
			secondaryScrb1 = 0;
			secondaryRunner = -1;
		}
	}

	// Build animation queue entry
	AnimQueueEntry entry;
	entry.snoidId = snoid->getId();
	entry.isRejection = isRejection;
	entry.actorSequenceStep = 0;
	entry.approachScrsId = walkScrsId;
	entry.returnScrsId = rejectScrsId;
	entry.primaryActorSlot = primaryRunner;
	entry.primaryActorScrbId = primaryScrb;
	entry.primaryFollowupScrbId = secondaryScrb;
	entry.secondaryActorSlot = secondaryRunner;
	entry.secondaryActorScrbId = secondaryScrb1;
	entry.secondaryFollowupScrbId = secondaryScrb2;
	entry.dropZone = zone;

	appendAnimQueueEntry(entry);
	debugC(4, MohawkEngine_Zoombini::kDebugPage02, "tunnels: queued zoombini %d at zone %d; rejected=%d, gate type=%d", snoid->getId(), zone,
		   isRejection ? 1 : 0, gateType);
}

// =========================================================================
// Animation Queue Management
// =========================================================================

void ZoombiniPuzzleTunnels::appendAnimQueueEntry(const AnimQueueEntry &entry) {
	if (5 <= _animQueueCount)
		return;
	_animQueue[_animQueueCount] = entry;
	_animQueueCount += 1;
}

void ZoombiniPuzzleTunnels::popAnimQueueEntry() {
	if (_animQueueCount <= 0)
		return;

	// Shift entries down by 14-word stride
	for (int queueIdx = 0; queueIdx < _animQueueCount - 1; queueIdx++)
		_animQueue[queueIdx] = _animQueue[queueIdx + 1];
	_animQueueCount -= 1;
	_animQueue[_animQueueCount] = AnimQueueEntry();
}

int16 ZoombiniPuzzleTunnels::removeAnimQueueEntry(uint16 runnerId) {
	for (int16 i = 0; i < _animQueueCount; i++) {
		if (_animQueue[i].snoidId != runnerId)
			continue;

		int16 removedZone = _animQueue[i].dropZone;
		for (int16 moveIdx = i; moveIdx + 1 < _animQueueCount; moveIdx++)
			_animQueue[moveIdx] = _animQueue[moveIdx + 1];
		_animQueueCount -= 1;
		_animQueue[_animQueueCount] = AnimQueueEntry();

		ZmbSnoid *snoid = getSnoid(runnerId);
		if (snoid)
			snoid->removeFlag(ZmbFeature::FLAG_04000000_OVERLAY);
		return removedZone;
	}

	return 0;
}

int16 ZoombiniPuzzleTunnels::resolveGateActorImageResourceId(int16 scrbId) {
	// The four Tunnels actor runners consequently switch tBMP families with the
	// script: 6000 for controller/rejection SCRBs, and one family per gatekeeper
	// for their speaking SCRBs.
	if (kResScrb4000_Reject0 <= scrbId && scrbId < kResScrb4200_Reject0)
		return kResBitmapShape4000_Gate0;
	if (kResScrb4200_Reject0 <= scrbId && scrbId < kResScrb4400_Reject0)
		return kResBitmapShape4200_Gate1;
	if (kResScrb4400_Reject0 <= scrbId && scrbId < kResScrb4600_Reject0)
		return kResBitmapShape4400_Gate2;
	if (kResScrb4600_Reject0 <= scrbId && scrbId <= kResScrb4699_RejectRangeLast)
		return kResBitmapShape4600_Gate3;
	return kResBitmapShape6000;
}

void ZoombiniPuzzleTunnels::playGateActor(int16 actorSlot, int16 scrbId, GateActorCompletionAction completion) {
	if (actorSlot < 0 || 4 <= actorSlot || scrbId == 0 || !_gateActorFeatures[actorSlot])
		return;

	_gateActorCompletionActions[actorSlot] = completion;
	_gateActorFeatures[actorSlot]->setResource(ZmbResource(ZmbResource::kPage, resolveGateActorImageResourceId(scrbId)));
	loadScrbOntoFeature(_gateActorFeatures[actorSlot], scrbId);
}

// =========================================================================
// Advance the current animation step.
// =========================================================================

void ZoombiniPuzzleTunnels::advanceAnimStep() {
	if (_animQueueCount <= 0)
		return;
	_vm->resetFidgetActivity();

	AnimQueueEntry &head = _animQueue[0];
	head.actorSequenceStep += 1;

	int16 runner = -1;
	int16 scrb = 0;

	switch (head.actorSequenceStep) {
	case 1:
		runner = head.primaryActorSlot;
		scrb = head.primaryActorScrbId;
		break;
	case 2:
		runner = head.secondaryActorSlot;
		scrb = head.secondaryActorScrbId;
		if (runner < 0 || !scrb) {
			head.actorSequenceStep = 3;
			// Fall through to step 3
			runner = head.primaryActorSlot;
			scrb = head.primaryFollowupScrbId;
			if (runner < 0 || !scrb) {
				head.actorSequenceStep = 4;
				runner = head.secondaryActorSlot;
				scrb = head.secondaryFollowupScrbId;
			}
		}
		break;
	case 3:
		runner = head.primaryActorSlot;
		scrb = head.primaryFollowupScrbId;
		if (runner < 0 || !scrb) {
			head.actorSequenceStep = 4;
			runner = head.secondaryActorSlot;
			scrb = head.secondaryFollowupScrbId;
		}
		break;
	case 4:
		runner = head.secondaryActorSlot;
		scrb = head.secondaryFollowupScrbId;
		break;
	default:
		// Beyond step 4: pop entry
		popAnimQueueEntry();
		_animationQueueAdvanceEnabled = true;
		return;
	}

	if (0 <= runner && runner < 4 && 0 < scrb && _gateActorFeatures[runner]) {
		playGateActor(runner, scrb, kGateActorCompletionAdvanceQueue01);
		_animationQueueAdvanceEnabled = false;
	} else {
		// No valid actor/SCRB pair: pop the queue entry.
		popAnimQueueEntry();
		_animationQueueAdvanceEnabled = true;
	}
}

// =========================================================================
// Select and queue a gate-actor presentation sequence.
// =========================================================================

void ZoombiniPuzzleTunnels::queueGateActorSequence(GateActorSequenceMode mode) {
	int16 primaryRunner = -1;
	int16 primaryScrb = 0;
	int16 secondaryRunner = -1;
	int16 secondaryScrb1 = 0;
	int16 secondaryScrb = 0;
	int16 secondaryScrb2 = 0;

	switch (mode) {
	case GateActorSequenceMode::kIdle00: {
		// Idle animations: 10-pool at
		uint16 animationPoolIdx = _vm->_rnd->getNonRepeatRandom(10, _idleActorPoolState);
		switch (animationPoolIdx) {
		case 0:
			primaryRunner = 1;
			primaryScrb = kResScrb4610_Reject;
			secondaryRunner = 2;
			secondaryScrb1 = kResScrb4216_Reject;
			break;
		case 1:
			primaryRunner = 2;
			primaryScrb = kResScrb4217_Idle;
			break;
		case 2:
			primaryRunner = 2;
			primaryScrb = kResScrb4218_Idle;
			break;
		case 3:
			primaryRunner = 2;
			primaryScrb = kResScrb4219_Idle;
			break;
		case 4:
			primaryRunner = 2;
			primaryScrb = kResScrb4220_Idle;
			break;
		case 5:
			primaryRunner = 0;
			primaryScrb = kResScrb4021_Reject;
			secondaryRunner = 3;
			secondaryScrb1 = kResScrb4408_Reject;
			break;
		case 6:
			primaryRunner = 0;
			primaryScrb = kResScrb4022_Reject;
			secondaryRunner = 3;
			secondaryScrb1 = kResScrb4408_Reject;
			break;
		case 7:
			primaryRunner = 3;
			primaryScrb = kResScrb4402_Reject2;
			secondaryRunner = 0;
			secondaryScrb1 = kResScrb4023_Reject;
			secondaryScrb2 = kResScrb4029_Reject;
			break;
		case 8:
			primaryRunner = 3;
			primaryScrb = kResScrb4402_Reject2;
			secondaryRunner = 0;
			secondaryScrb1 = kResScrb4023_Reject;
			secondaryScrb2 = kResScrb4030_Reject;
			break;
		case 9:
			primaryRunner = 3;
			primaryScrb = kResScrb4419_Reject;
			break;
		}
		break;
	}
	case GateActorSequenceMode::kEntry01: {
		// Init animations
		if (_vm->_state->getTunnelsEntryActorState()._pageOpenCount == 1) {
			// First visit: 4-pool at
			uint16 animationPoolIdx = _vm->_rnd->getNonRepeatRandom(4, _entryActorPoolState);
			switch (animationPoolIdx) {
			case 0:
				primaryRunner = 2;
				primaryScrb = kResScrb4221_Init;
				break;
			case 1:
				primaryRunner = 0;
				primaryScrb = kResScrb4024_Reject;
				secondaryScrb = kResScrb4025_Reject;
				break;
			case 2:
				primaryRunner = 1;
				primaryScrb = kResScrb4614_Reject;
				break;
			case 3:
				primaryRunner = 3;
				primaryScrb = kResScrb4409_Reject;
				break;
			}
		} else {
			// Subsequent visits: 8-pool
			uint16 animationPoolIdx = _vm->_rnd->getNonRepeatRandom(8, _entryActorPoolState);
			switch (animationPoolIdx) {
			case 0:
				primaryRunner = 1;
				primaryScrb = kResScrb4611_Reject;
				secondaryRunner = 0;
				secondaryScrb1 = kResScrb4026_Reject;
				break;
			case 1:
				primaryRunner = 1;
				primaryScrb = kResScrb4611_Reject;
				secondaryRunner = 2;
				secondaryScrb1 = kResScrb4224_Reject;
				break;
			case 2:
				primaryRunner = 3;
				primaryScrb = kResScrb4411_Reject;
				secondaryRunner = 1;
				secondaryScrb1 = kResScrb4612_Reject;
				break;
			case 3:
				primaryRunner = 1;
				primaryScrb = kResScrb4613_Reject;
				secondaryRunner = 2;
				secondaryScrb1 = kResScrb4223_Reject;
				break;
			case 4:
				primaryRunner = 1;
				primaryScrb = kResScrb4613_Reject;
				secondaryRunner = 2;
				secondaryScrb1 = kResScrb4222_Reject;
				break;
			case 5:
				primaryRunner = 3;
				primaryScrb = kResScrb4410_Reject;
				secondaryRunner = 2;
				secondaryScrb1 = kResScrb4222_Reject;
				break;
			case 6:
				primaryRunner = 0;
				primaryScrb = kResScrb4027_Reject;
				secondaryRunner = 3;
				secondaryScrb1 = kResScrb4412_Reject;
				break;
			case 7:
				primaryRunner = 0;
				primaryScrb = kResScrb4028_Reject;
				secondaryRunner = 2;
				secondaryScrb1 = kResScrb4223_Reject;
				break;
			}
		}
		break;
	}
	case GateActorSequenceMode::kPostGame02: {
		// End-game: 3-pool at
		uint16 animationPoolIdx = _vm->_rnd->getNonRepeatRandom(3, _postGameActorPoolState);
		switch (animationPoolIdx) {
		case 0:
			primaryRunner = 3;
			primaryScrb = kResScrb4420_Reject;
			break;
		case 1:
			primaryRunner = 3;
			primaryScrb = kResScrb4421_Reject;
			break;
		case 2:
			primaryRunner = 3;
			primaryScrb = kResScrb4422_Reject;
			break;
		}
		break;
	}
	case GateActorSequenceMode::kAdvance03: {
		// Select an advance reaction from readiness and the Snoid count.
		if (_postGameAnimCompleted) {
			// Select one of seven ready-state reactions.
			uint16 animationPoolIdx = _vm->_rnd->getNonRepeatRandom(7, _goReadyAdvanceActorPoolState);
			switch (animationPoolIdx) {
			case 0:
				primaryRunner = 0;
				primaryScrb = kResScrb4035_Advance;
				break;
			case 1:
				primaryRunner = 3;
				primaryScrb = kResScrb4423_Reject;
				break;
			case 2:
				primaryRunner = 0;
				primaryScrb = kResScrb4034_Advance;
				break;
			case 3:
				primaryRunner = 0;
				primaryScrb = kResScrb4036_Advance;
				break;
			case 4:
				primaryRunner = 0;
				primaryScrb = kResScrb4037_Advance;
				break;
			case 5:
				primaryRunner = 0;
				primaryScrb = kResScrb4032_Advance;
				break;
			case 6:
				primaryRunner = 0;
				primaryScrb = kResScrb4033_Advance;
				break;
			}
		} else {
			// Count idle snoids on pedestals
			int16 idleCount = 0;
			for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
				ZmbSnoid *s = *it;
				if (s && s->isPackSnoid() && s->getAnimState() == kSnoidAnimState000_Idle)
					idleCount += 1;
			}

			if (idleCount == _pageLoadedZmbCount) {
				// All at pedestals: 8-pool at
				uint16 animationPoolIdx = _vm->_rnd->getNonRepeatRandom(8, _allIdleAdvanceActorPoolState);
				switch (animationPoolIdx) {
				case 0:
					primaryRunner = 0;
					primaryScrb = kResScrb4031_Advance;
					break;
				case 1:
					primaryRunner = 1;
					primaryScrb = kResScrb4617_Reject;
					break;
				case 2:
					primaryRunner = 0;
					primaryScrb = kResScrb4038_Advance;
					break;
				case 3:
					primaryRunner = 0;
					primaryScrb = kResScrb4034_Advance;
					break;
				case 4:
					primaryRunner = 0;
					primaryScrb = kResScrb4036_Advance;
					break;
				case 5:
					primaryRunner = 0;
					primaryScrb = kResScrb4037_Advance;
					break;
				case 6:
					primaryRunner = 0;
					primaryScrb = kResScrb4032_Advance;
					break;
				case 7:
					primaryRunner = 0;
					primaryScrb = kResScrb4033_Advance;
					break;
				}
			} else {
				// Partial: 9-pool at
				uint16 animationPoolIdx = _vm->_rnd->getNonRepeatRandom(9, _partialAdvanceActorPoolState);
				switch (animationPoolIdx) {
				case 0:
					primaryRunner = 2;
					primaryScrb = kResScrb4225_Advance;
					break;
				case 1:
					primaryRunner = 2;
					primaryScrb = kResScrb4226_Advance;
					break;
				case 2:
					primaryRunner = 1;
					primaryScrb = kResScrb4615_Reject;
					break;
				case 3:
					primaryRunner = 1;
					primaryScrb = kResScrb4616_Reject;
					break;
				case 4:
					primaryRunner = 0;
					primaryScrb = kResScrb4034_Advance;
					break;
				case 5:
					primaryRunner = 0;
					primaryScrb = kResScrb4036_Advance;
					break;
				case 6:
					primaryRunner = 0;
					primaryScrb = kResScrb4037_Advance;
					break;
				case 7:
					primaryRunner = 0;
					primaryScrb = kResScrb4032_Advance;
					break;
				case 8:
					primaryRunner = 0;
					primaryScrb = kResScrb4033_Advance;
					break;
				}
			}
		}
		break;
	}
	}

	// Build animation queue entry
	AnimQueueEntry entry;
	entry.primaryActorSlot = primaryRunner;
	entry.primaryActorScrbId = primaryScrb;
	entry.primaryFollowupScrbId = secondaryScrb;
	entry.secondaryActorSlot = secondaryRunner;
	entry.secondaryActorScrbId = secondaryScrb1;
	entry.secondaryFollowupScrbId = secondaryScrb2;
	appendAnimQueueEntry(entry);
}

// =========================================================================
// Collect the occupied pack seats.
// =========================================================================

void ZoombiniPuzzleTunnels::collectPackSeatOccupancy() {
	Common::Array<uint16> assignedRunnerIds;
	assignIdleSnoidsToSlots(kSnoidPositions, 16, 500, false, assignedRunnerIds);
	for (int16 pedestalIdx = 0; pedestalIdx < 16; pedestalIdx++)
		_packSeatSnoidIds[pedestalIdx] = assignedRunnerIds[pedestalIdx];
}

// =========================================================================
// Select an unused pack seat.
// =========================================================================

int16 ZoombiniPuzzleTunnels::selectUnusedPackSeat() {
	collectPackSeatOccupancy();

	int16 emptySlotIdx = -1;
	if (50 < _vm->_rnd->getRandomNumber(1, 100)) {
		for (int16 slotIdx = 0; emptySlotIdx == -1 && slotIdx < 16; slotIdx++) {
			if (_packSeatSnoidIds[slotIdx] == 0)
				emptySlotIdx = slotIdx;
		}
	} else {
		for (int16 slotIdx = 15; emptySlotIdx == -1 && 0 <= slotIdx; slotIdx--) {
			if (_packSeatSnoidIds[slotIdx] == 0)
				emptySlotIdx = slotIdx;
		}
	}

	return emptySlotIdx == -1 ? 0 : emptySlotIdx;
}

// =========================================================================
// Assign a slot while preserving any displaced occupant.
// =========================================================================

int16 ZoombiniPuzzleTunnels::assignSlotWithPush(int16 returnFromLeftSide) {
	collectPackSeatOccupancy();

	// Check the two side-specific return seats.
	// Left-side rejections prefer slots 15 then 10.
	// Right-side rejections prefer slots 11 then 6.
	// Run the push pass only when both are occupied.
	int16 preferredSlots[2];
	if (returnFromLeftSide) {
		preferredSlots[0] = 15;
		preferredSlots[1] = 10;
	} else {
		preferredSlots[0] = 11;
		preferredSlots[1] = 6;
	}

	for (int16 preferredIdx = 0; preferredIdx < 2; preferredIdx++) {
		const int16 preferredSlot = preferredSlots[preferredIdx];
		if (_packSeatSnoidIds[preferredSlot] == 0) {
			return preferredSlot;
		}
	}

	for (int16 scanOrdinal = 0; scanOrdinal < 15; scanOrdinal++) {
		int16 emptySlotIdx = scanOrdinal;
		if (!returnFromLeftSide && !(emptySlotIdx < 11))
			emptySlotIdx = 26 - emptySlotIdx;

		if (_packSeatSnoidIds[emptySlotIdx] != 0)
			continue;

		int16 primaryOffset = 0;
		int16 secondaryOffset = 0;
		int16 tertiaryOffset = 0;
		if (emptySlotIdx == 0) {
			primaryOffset = 6;
		} else if (emptySlotIdx < 5) {
			primaryOffset = 5;
			secondaryOffset = 6;
		} else if (emptySlotIdx < 7) {
			primaryOffset = 5;
		} else if (emptySlotIdx < 11) {
			primaryOffset = 4;
			secondaryOffset = 5;
		} else if (returnFromLeftSide) {
			primaryOffset = emptySlotIdx < 15 ? 1 : 0;
			if (emptySlotIdx < 14)
				secondaryOffset = 2;
			if (emptySlotIdx < 13)
				tertiaryOffset = 3;
		} else {
			if (!(emptySlotIdx < 12))
				primaryOffset = -1;
			if (!(emptySlotIdx < 13))
				secondaryOffset = -2;
			if (!(emptySlotIdx < 14))
				tertiaryOffset = -3;
		}

		bool needsOccupant = true;
		while (primaryOffset != 0 && needsOccupant) {
			const int16 occupiedSlotIdx = emptySlotIdx + primaryOffset;
			if (_packSeatSnoidIds[occupiedSlotIdx] != 0) {
				const uint16 occupantId = _packSeatSnoidIds[occupiedSlotIdx];
				ZmbSnoid *occupant = getSnoid(occupantId);
				if (occupant) {
					occupant->setAnimTargetPos(kSnoidPositions[emptySlotIdx]);
					occupant->setAnimState(kSnoidAnimState007_Depart);
					_packSeatSnoidIds[emptySlotIdx] = occupantId;
					_packSeatSnoidIds[occupiedSlotIdx] = 0;
					needsOccupant = false;
				}
			}

			primaryOffset = secondaryOffset;
			secondaryOffset = tertiaryOffset;
			tertiaryOffset = 0;
		}
	}

	// Repeat the preferred-seat lookup because the push pass may have opened one of these two side-specific seats.
	for (int16 preferredIdx = 0; preferredIdx < 2; preferredIdx++) {
		const int16 preferredSlot = preferredSlots[preferredIdx];
		if (_packSeatSnoidIds[preferredSlot] == 0) {
			return preferredSlot;
		}
	}

	int16 emptySlotIdx = -1;
	for (int16 slotIdx = 0; emptySlotIdx == -1 && slotIdx < 16; slotIdx++) {
		if (_packSeatSnoidIds[slotIdx] == 0)
			emptySlotIdx = slotIdx;
	}
	return emptySlotIdx;
}

// =========================================================================
// Spawn all pending zoombinis.
// =========================================================================

int16 ZoombiniPuzzleTunnels::spawnPendingZoombinis() {
	int16 spawned = 0;

	for (int16 queueIdx = 0; queueIdx < 4 && 0 < _animQueueCount; queueIdx++) {
		AnimQueueEntry &head = _animQueue[0];
		ZmbSnoid *snoid = getSnoid(head.snoidId);
		if (snoid) {
			// Position at spawn origin (off-screen bottom)
			int16 spawnX = kSpawnOriginX[0 < head.dropZone ? head.dropZone - 1 : 0];
			snoid->setPointLoc(Common::Point(spawnX, 460));
			snoid->_packIsOccupied = false;
			snoid->removeFlag(ZmbFeature::FLAG_04000000_OVERLAY);
			snoid->setAnimState(kSnoidAnimState007_Depart);
			spawned += 1;
		}
		popAnimQueueEntry();
	}

	return spawned;
}

// =========================================================================
// Play the next ambient sound.
//
// Called when the main path SCRB finishes.
// It completes the current main-path phase and may submit an ambient sound from SND 20045-20048.
// =========================================================================

void ZoombiniPuzzleTunnels::playAmbientSound() {
	// Enable runner Z-sorting after the setup or background SCRB completes.
	setRunnerZSortEnabled(true);
	_postGameAnimCompleted = true;

	// Play more often at lower difficulty or during the first few visits.
	const bool shouldPlay = passesPartialResultFeedbackGate();

	if (shouldPlay && _enteredSnoidCount < _pageLoadedZmbCount && 0 < _enteredSnoidCount)
		queuePartialSuccessNarratorSound();
}

// =========================================================================
// Clear the gate render flag.
// =========================================================================

void ZoombiniPuzzleTunnels::clearGateRenderFlag() {
	if (_animQueueCount <= 0)
		return;

	int16 obscuredZone = 0;
	if (_animQueue[0].dropZone == 1)
		obscuredZone = 2;
	else if (_animQueue[0].dropZone == 4)
		obscuredZone = 3;
	if (obscuredZone == 0)
		return;

	// A queued inner-zone snoid can otherwise remain in the overlay bucket over the outer-zone gate replay.
	for (int16 i = 1; i < _animQueueCount && i < 5; i++) {
		if (_animQueue[i].snoidId == 0 || _animQueue[i].dropZone != obscuredZone)
			continue;
		ZmbSnoid *snoid = getSnoid(_animQueue[i].snoidId);
		if (snoid)
			snoid->removeFlag(ZmbFeature::FLAG_04000000_OVERLAY);
		break;
	}
}

// =========================================================================
// Find an idle zoombini in the pack.
// =========================================================================

ZmbSnoid *ZoombiniPuzzleTunnels::findIdlePackSnoid(uint16 snoidId) {
	ZmbSnoid *snoid = getIdleSnoid(snoidId);
	if (snoid && snoid->isPackSnoid())
		return snoid;
	return nullptr;
}

// =========================================================================
// Process a zoombini animation event.
// =========================================================================

void ZoombiniPuzzleTunnels::processSnoidAnimEvent(ZmbSnoid *snoid, int16 eventCode) {
	if (!snoid)
		return;

	if (kTunnelEventCode013_StartGateActor < eventCode) {
		handleBodyArrangementScriptEvent(*snoid, eventCode);
		return;
	} else if (eventCode == kTunnelEventCode013_StartGateActor) {
		// The Snoid reached the gate-actor hand-off marker.
		// Start the selected gate actor and pause normal Snoid sorting until it completes.
		if (_animQueueCount <= 0 || _animQueue[0].snoidId != snoid->getId())
			return;

		AnimQueueEntry &head = _animQueue[0];
		if (0 <= head.primaryActorSlot && head.primaryActorSlot < 4 && 0 < head.primaryActorScrbId) {
			playGateActor(head.primaryActorSlot, head.primaryActorScrbId, kGateActorCompletionPrimary04);
			_gateActorFeatures[head.primaryActorSlot]->setSortRect(snoid->getSortRect());
			// Disable Z-sorting only while the primary gate actor is playing.
			setRunnerZSortEnabled(false);
			_animationQueueAdvanceEnabled = false;
		}

		if (_activeGateOrAmbientSoundResource.hasId()) {
			_vm->_sound->stopSound(_activeGateOrAmbientSoundResource);
			_activeGateOrAmbientSoundResource = ZmbResource();
		}
		return;
	} else if (eventCode == kAnimEventM1_End) {
		// The gate-entry SCRS reached its terminal marker; commit or return the queued Snoid.
		if (_animQueueCount <= 0 || _animQueue[0].snoidId != snoid->getId())
			return;

		AnimQueueEntry &head = _animQueue[0];
		_deferredActorSlot = -1;
		_deferredActorScrbId = 0;
		_deferredActorPopsQueue = true;

		if (!head.isRejection && 0 < head.primaryActorScrbId && _zoneCorrectStreak[head.dropZone] < 2) {
			_deferredActorSlot = head.primaryActorSlot;
			_deferredActorScrbId = head.primaryActorScrbId;
		}

		if (!head.isRejection) {
			if (0 < head.dropZone && head.dropZone <= 4) {
				int16 lane = kDropZoneToLane[head.dropZone - 1];
				int16 &occupancy = _laneOccupancy[lane];
				ZmbFeature *linkParent = _laneLinkAnchorFeature;
				bool linkAfterParent = false;

				snoid->addFlag(static_cast<ZmbFeature::Flag>(ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00008000_LOOP_ANIM));
				if (occupancy < 16) {
					if (0 < occupancy) {
						linkParent = getSnoid(_laneSnoidIds[lane][occupancy - 1]);
						linkAfterParent = true;
					}
					_laneSnoidIds[lane][occupancy] = snoid->getId();
					snoid->setAnimTargetPos(kOutputLanePositions[lane][occupancy]);
					occupancy += 1;

					// The first Snoid in a lane goes before the lane link anchor (SCRB 9000).
					// Later Snoids go after the previous member of that lane.
					if (linkAfterParent)
						manualLinkAfter(snoid, linkParent);
					else
						manualLinkBefore(snoid, linkParent);
				}
			}

			snoid->_packIsOccupied = true;
			snoid->_runnerStatus = kSnoidRunnerStatus01_EnteredGate;
			snoid->setAnimState(kSnoidAnimState010_ArrivalMotion);

			_enteredSnoidCount += 1;
			if (_enteredSnoidCount == _pageLoadedZmbCount) {
				_celebrationTarget += 2;
				_completionNarratorSoundId = selectCompletionNarratorSoundId();
				// The completion voice owns the next audio slot.
				// A gate farewell is selected only after the player presses Go.
				_deferredActorSlot = -1;
				_deferredActorScrbId = 0;
			} else if (_enteredSnoidCount == 10) {
				_celebrationTarget += 1;
			} else if (_enteredSnoidCount == 12) {
				_celebrationTarget += 1;
			} else if (_enteredSnoidCount == 14) {
				_celebrationTarget += 2;
			}

			setGoButtonsEnabled(true);
		} else {
			playRockfallWarningFeedback();

			int16 returnFromLeftSide = head.dropZone <= 2 ? 1 : 0;
			int16 slotResult = assignSlotWithPush(returnFromLeftSide);
			// Reset the complete bitmask to @ref ZmbFeature::FLAG_00000001_TYPE_SNOID.
			// After the temporary gate-actor lock ends, normal entity Z-sorting orders the return by live depth.
			snoid->setFlags(ZmbFeature::FLAG_00000001_TYPE_SNOID);
			snoid->_runnerStatus = kSnoidRunnerStatus00_Available;
			snoid->setAnimTargetPos(kSnoidPositions[slotResult]);
			snoid->setAnimState(kSnoidAnimState007_Depart);
			clearGateRenderFlag();
		}

		if (_deferredActorSlot < 0) {
			popAnimQueueEntry();
			_animationQueueAdvanceEnabled = true;
		}
		return;
	} else if (eventCode == kTunnelEventCode000_ToggleSnoidFacing) {
		// The tunnel entry SCRS reached its facing/body-arrangement marker.
		// === Toggle facing + pending body arrangement ===
		// The event-0 toggle writes the runner's facing-left state, NOT render-enabled state.
		// Toggling render here instead deadlocks the SCRS playback: a hidden
		// snoid skips the whole anim state machine, so the script never
		// advances past frame 0 and no un-hide toggle can ever arrive.
		snoid->setFacingLeft(!snoid->isFacingLeft());
		applyPendingBodyArrangement(*snoid);
		return;
	} else if (eventCode == kTunnelEventCode010_StartRejectReturn) {
		// The rejected Snoid reached the return marker.
		// Start its replay SCRS at the saved gate-side position.
		if (0 < _animQueueCount && _animQueue[0].snoidId == snoid->getId()) {
			int16 scrsId = _animQueue[0].returnScrsId;
			if (0 < scrsId) {
				int16 variantIdx = ((scrsId - kResScrs8000_RejectBase) / 2) & 3;
				clearGateRenderFlag();

				ZmbScrsCompletionMode completionMode;
				if (_animQueue[0].isRejection)
					completionMode = ZmbScrsCompletionMode::kReturnToIdle;
				else
					completionMode = ZmbScrsCompletionMode::kHide;
				startSnoidScrs(snoid, ZmbResource(ZmbResource::kPage, scrsId), completionMode, &kScrsReplayPositions[variantIdx]);
			}
		}
		return;
	}
}

void ZoombiniPuzzleTunnels::playRockfallWarningFeedback() {
	if (_rockfallWarningSoundPlaying || _rockfallWarningSoundId == 0)
		return;

	_rockfallWarningSoundPlaying = true;
	Audio::SoundHandle *handle = _vm->_sound->playSound(ZmbResource(ZmbResource::kPage, _rockfallWarningSoundId), Audio::Mixer::kSFXSoundType);
	_rockfallWarningSoundHandleValid = handle != nullptr;
	if (handle)
		_rockfallWarningSoundHandle = *handle;

	if (_rockfallWarningFeature) {
		int16 rockfallScrbId = static_cast<int16>(kResScrb7001_RockfallBase + _vm->_rnd->getRandomNumber(0, 3));
		loadScrbOntoFeature(_rockfallWarningFeature, rockfallScrbId);
	}
}

// =========================================================================
// Process a gate animation event.
// =========================================================================

void ZoombiniPuzzleTunnels::processGateAnimEvent(ZmbFeature *feature, int16 eventCode) {
	if (eventCode != kAnimEventM1_End)
		return;

	if (feature == _mainPathFeature) {
		playAmbientSound();
		return;
	}

	int16 runner = -1;
	for (int16 runnerIdx = 0; runnerIdx < 4; runnerIdx++) {
		if (_gateActorFeatures[runnerIdx] == feature) {
			runner = runnerIdx;
			break;
		}
	}
	if (runner < 0)
		return;

	GateActorCompletionAction completion = _gateActorCompletionActions[runner];
	_gateActorCompletionActions[runner] = kGateActorCompletionNone00;
	switch (completion) {
	case kGateActorCompletionAdvanceQueue01:
		_animationQueueAdvanceEnabled = true;
		if (_setupPhase == SetupPhase::kRunnersSelected01)
			_setupPhase = SetupPhase::kMainPathPending02;
		break;
	case kGateActorCompletionDeferred02:
		// Start this follow-up actor without a sequence callback.
		// Do not release the active Snoid sequence.
		// Its head callback or deferred-and-pop callback owns that transition.
		// Releasing it here would let an earlier gate actor clear the lock.
		// This can happen after the next queued Snoid becomes the head.
		break;
	case kGateActorCompletionDeferredAndPop03:
		popAnimQueueEntry();
		_animationQueueAdvanceEnabled = true;
		break;
	case kGateActorCompletionPrimary04:
		setRunnerZSortEnabled(true);
		if (_enteredSnoidCount < _pageLoadedZmbCount && 0 < _animQueueCount && 0 < _animQueue[0].primaryFollowupScrbId) {
			_deferredActorSlot = _animQueue[0].primaryActorSlot;
			_deferredActorScrbId = _animQueue[0].primaryFollowupScrbId;
			_deferredActorPopsQueue = false;
		}
		// The Snoid completion callback owns the queue head, so this actor must not unlock a subsequently queued Snoid.
		break;
	default:
		break;
	}
}

void ZoombiniPuzzleTunnels::refreshActiveScrbSound() {
	if (_activeGateOrAmbientSoundResource.hasId() && !_vm->_sound->isSoundPlaying(_activeGateOrAmbientSoundResource))
		_activeGateOrAmbientSoundResource = ZmbResource();

	ZmbFeature *features[6] = {
		_gateActorFeatures[0],
		_gateActorFeatures[1],
		_gateActorFeatures[2],
		_gateActorFeatures[3],
		_rockfallWarningFeature,
		_mainPathFeature,
	};
	for (ZmbFeature *feature : features) {
		if (!feature || feature->getLastSoundedFrameIdx() < 0)
			continue;
		ZmbResource sndRes;
		if (!feature->getFrameSoundResource(feature->getLastSoundedFrameIdx(), sndRes))
			continue;
		const bool isPageSound = sndRes._archiveKind == ZmbResource::kPage;
		const bool isGateSound = isPageSound && kResSound4000_GateBase <= sndRes._id && sndRes._id <= kResSound4699_GateLast;
		const bool isAmbientSound = isPageSound && kResSound7000_AmbientBase <= sndRes._id && sndRes._id <= kResSound7099_AmbientLast;
		if ((isGateSound || isAmbientSound) && _vm->_sound->isSoundPlaying(sndRes))
			_activeGateOrAmbientSoundResource = sndRes;
	}
}

// =========================================================================
// Dispatch a feature animation event.
// =========================================================================

void ZoombiniPuzzleTunnels::onFeatureAnimEvent(ZmbFeature *feature, int16 eventCode) {
	if (feature->hasFlag(ZmbFeature::FLAG_00000001_TYPE_SNOID)) {
		processSnoidAnimEvent(static_cast<ZmbSnoid *>(feature), eventCode);
	} else {
		processGateAnimEvent(feature, eventCode);
	}
}

// =========================================================================
// Update the puzzle once per frame.
// =========================================================================

void ZoombiniPuzzleTunnels::onPostRenderFrame() {
	if (_isUpdating || !_pageActive)
		return;
	PuzzleUpdateGuard updateGuard(_isUpdating);

	// Check pending departure
	if (isDeparturePending()) {
		// Continue only the already-selected Go farewell while normal puzzle progression is frozen.
		if (0 < _animQueueCount && _animationQueueAdvanceEnabled && _animQueue[0].snoidId == 0)
			advanceAnimStep();
		return;
	}

	refreshActiveScrbSound();
	const bool isTrackedSoundPlaying = _vm->_sound->isSoundPlaying(_activeGateOrAmbientSoundResource);
	const bool isPageSound = _activeGateOrAmbientSoundResource._archiveKind == ZmbResource::kPage;
	const bool isGateSoundPlaying = isTrackedSoundPlaying && isPageSound &&
									kResSound4000_GateBase <= _activeGateOrAmbientSoundResource._id && _activeGateOrAmbientSoundResource._id <= kResSound4699_GateLast;
	const bool isAmbientSoundPlaying = isTrackedSoundPlaying && isPageSound &&
									   kResSound7000_AmbientBase <= _activeGateOrAmbientSoundResource._id && _activeGateOrAmbientSoundResource._id <= kResSound7099_AmbientLast;

	// Release the queue gate when the low-chance rockfall warning finishes.
	if (0 < _rockfallWarningSoundId && _rockfallWarningSoundPlaying) {
		if (!_rockfallWarningSoundHandleValid ||
			!_vm->_system->getMixer()->isSoundHandleActive(_rockfallWarningSoundHandle)) {
			_rockfallWarningSoundId = 0;
			_rockfallWarningSoundPlaying = false;
			_rockfallWarningSoundHandleValid = false;
		}
	}

	// Deferred gate actor
	if (0 <= _deferredActorSlot) {
		if (!isGateSoundPlaying && !isAmbientSoundPlaying) {
			int16 actorSlot = _deferredActorSlot;
			_deferredActorSlot = -1;

			if (0 < _completionNarratorSoundId) {
				if (_deferredActorPopsQueue) {
					popAnimQueueEntry();
					_animationQueueAdvanceEnabled = true;
				}
			} else {
				if (0 <= actorSlot && actorSlot < 4 && 0 < _deferredActorScrbId) {
					GateActorCompletionAction completion;
					if (_deferredActorPopsQueue)
						completion = kGateActorCompletionDeferredAndPop03;
					else
						completion = kGateActorCompletionDeferred02;
					playGateActor(actorSlot, _deferredActorScrbId, completion);
					_animationQueueAdvanceEnabled = false;
				}
			}
		}
	} else if (0 < _completionNarratorSoundId && !isGateSoundPlaying && !isAmbientSoundPlaying) {
		// Submit the completion voice to the shared script-sound arbitration pass.
		queueNarratorSound(_completionNarratorSoundId);
		_completionNarratorSoundId = 0;
	}

	// Animation queue processing
	if (0 < _remainingRejectChances) {
		if (_rockfallWarningSoundPlaying || !_animQueueCount || !_animationQueueAdvanceEnabled)
			goto postAnimQueue;

		if (0 < _animQueue[0].snoidId) {
			// Active entry with Snoid
			if (_animQueue[0].actorSequenceStep == 0 && !isGateSoundPlaying && !isAmbientSoundPlaying) {
				ZmbSnoid *snoid = getIdleSnoid(_animQueue[0].snoidId);
				// The queued Snoid must first finish its state-4 seat arrival before the lane SCRS can start.
				if (snoid) {
					_animQueue[0].actorSequenceStep = 1;

					// Start secondary animation (hint) if available
					if (0 < _animQueue[0].secondaryActorScrbId) {
						if (_zoneCorrectStreak[_animQueue[0].dropZone] < 2 &&
							0 <= _animQueue[0].secondaryActorSlot && _animQueue[0].secondaryActorSlot < 4) {
							playGateActor(_animQueue[0].secondaryActorSlot,
										  _animQueue[0].secondaryActorScrbId,
										  kGateActorCompletionNone00);
						}
					}

					// Clear the DRAW_ON_REG occupant when the Snoid leaves its seat.
					clearDrawOnRegOccupant(_animQueue[0].dropZone - 1);

					// Clear overlay on snoid
					snoid->removeFlag(ZmbFeature::FLAG_04000000_OVERLAY);
					if (snoid->getAnimState() < kSnoidAnimState004_Arrive)
						snoid->activateRender();
					// Store hover data 0-7 on the Snoid.
					// Force it to face left for values below 4 before starting the approach SCRS.
					// These values correspond to drop zones 1 and 2.
					if (_animQueue[0].dropZone < 3)
						snoid->setFacingLeft(true);

					// Start SCRS playback
					int16 scrsId = _animQueue[0].approachScrsId;
					if (0 < scrsId) {
						ZmbScrsCompletionMode completionMode;
						if (_animQueue[0].isRejection)
							completionMode = ZmbScrsCompletionMode::kReturnToIdle;
						else
							completionMode = ZmbScrsCompletionMode::kHide;
						startSnoidScrs(snoid, ZmbResource(ZmbResource::kPage, scrsId), completionMode);
					}

					// Rejections select the low-chance rockfall SND 4703..4700 by remaining count.
					// Accepted advances instead slow the Snoid to interval 4.
					if (_animQueue[0].isRejection) {
						_remainingRejectChances -= 1;
						switch (_remainingRejectChances) {
						case 1:
							_rockfallWarningSoundId = kResSound4703_Rockfall1Remaining;
							break;
						case 2:
							_rockfallWarningSoundId = kResSound4702_Rockfall2Remaining;
							break;
						case 3:
							_rockfallWarningSoundId = kResSound4701_Rockfall3Remaining;
							break;
						case 4:
							_rockfallWarningSoundId = kResSound4700_Rockfall4Remaining;
							break;
						}
					} else {
						// Set frame interval to 4 ticks per frame, slowing the snoid's animation.
						snoid->setFrameInterval(4);
					}

					_animationQueueAdvanceEnabled = false;
					_idleActorDeadline = getCurrentFrameCounter() + _idleActorDelay;
				}
			}
			goto postAnimQueue;
		}

		// An actor-only queue entry advances directly to its first actor step.
		advanceAnimStep();
	} else {
		// The rejection budget is exhausted; enter the post-game phase.
		if (!_postGameStarted && _animationQueueAdvanceEnabled) {
			if (!isGateSoundPlaying) {
				debugC(2, MohawkEngine_Zoombini::kDebugPage02, "tunnels: starting post-game after all placements");
				_postGameStarted = true;
				_postGameAnimCompleted = false;
				spawnPendingZoombinis();
				queueGateActorSequence(GateActorSequenceMode::kPostGame02);
				_setupPhase = SetupPhase::kRunnersSelected01;
			}
			goto postAnimQueue;
		}

		// Process remaining queue entries after post-game
		if (0 < _animQueueCount && _animationQueueAdvanceEnabled && !_animQueue[0].snoidId && !isGateSoundPlaying) {
			advanceAnimStep();
		}
	}

postAnimQueue:
	// Idle animation scheduling
	if (!_postGameStarted && _idleActorDeadline < getCurrentFrameCounter()) {
		_vm->resetFidgetActivity();
		queueGateActorSequence(GateActorSequenceMode::kIdle00);
		_idleActorDelay = _vm->_rnd->getRandomNumber(5400, 10800);
		_idleActorDeadline = getCurrentFrameCounter() + _idleActorDelay;
	}

	// The final setup phase loads SCRB 7000 onto the main path feature.
	if (_setupPhase == SetupPhase::kMainPathPending02) {
		debugC(2, MohawkEngine_Zoombini::kDebugPage02, "tunnels: completing setup with main-path SCRB 7000");
		_setupPhase = SetupPhase::kComplete03;
		if (_mainPathFeature) {
			loadScrbOntoFeature(_mainPathFeature, kResScrb7000_MainPath);
			_mainPathFeature->removeFlag(ZmbFeature::FLAG_01000000_DEFER_RENDER);
			// The completion callback calls @ref ZoombiniPuzzleTunnels::playAmbientSound().
		}
		// Disable Z sorting until the background SCRB completes.
		setRunnerZSortEnabled(false);
		// Keep the Go button disabled through the first crossing.
		// @ref ZoombiniPuzzleTunnels::processSnoidAnimEvent() handles that crossing.
	}
}

void ZoombiniPuzzleTunnels::onPostAmbientFrame() {
	if (_isUpdating || !_pageActive || isDeparturePending())
		return;
	PuzzleUpdateGuard updateGuard(_isUpdating);

	// Schedule celebration fidgets.
	if (_celebrationsPlayed < _celebrationTarget &&
		_celebrationInterval < getCurrentFrameCounter() - _lastCelebrationFrame) {

		_lastCelebrationFrame = getCurrentFrameCounter();
		bool triggered = false;
		int16 attempts = 0;

		while (!triggered && attempts < 16) {
			attempts += 1;
			uint16 poolIdx = _vm->_rnd->getNonRepeatRandom(_pageLoadedZmbCount, _celebrationSnoidPoolState);
			uint16 snoidId = 10000 + poolIdx;

			ZmbSnoid *snoid = findIdlePackSnoid(snoidId);
			if (snoid && snoid->_packIsOccupied &&
				snoid->hasFlag(ZmbFeature::FLAG_00000001_TYPE_SNOID)) {
				// Use the 1-based feet trait directly:
				// 8559 + feet selects the five celebration SCRS resources 8560-8564.
				// SCRS 8559 is the final traversal script.
				// It moves a lane Snoid farther into the tunnel.
				// Feet type 1 would select this traversal script.
				// An upper-cell Snoid would then walk off-screen instead of celebrating in place.
				int16 feet = CLIP<int16>(static_cast<int16>(snoid->_trait._feet), 1, 5);
				int16 scrsId = static_cast<int16>(kResScrs8559_AcceptWalkBase + feet);
				if (startSnoidScrs(snoid, ZmbResource(ZmbResource::kPage, scrsId))) {
					debugC(4, MohawkEngine_Zoombini::kDebugPage02, "tunnels: started celebration for zoombini %u with SCRS %u", snoid->getId(), scrsId);
					_celebrationsPlayed += 1;
					triggered = true;
				}
			}
		}
	}
}

// =========================================================================
// Button Handlers
// =========================================================================

void ZoombiniPuzzleTunnels::onGoButtonActivated() {
	if (_postGameStarted && !_postGameAnimCompleted)
		return;

	// Re-enable NODE/PATH traversal so accepted Snoids depart through the Tunnels route graph.
	setSnoidNodePathDisabled(false);
	startDepartWalkAnimation(Common::Point(670, 30));

	// Stop active sound
	if (_activeGateOrAmbientSoundResource.hasId()) {
		_vm->_sound->stopSound(_activeGateOrAmbientSoundResource);
		_activeGateOrAmbientSoundResource = ZmbResource();
	}

	queueGateActorSequence(GateActorSequenceMode::kAdvance03);

	ZoombiniInteractive::onGoButtonActivated();
}

ZmbChanceInfo ZoombiniPuzzleTunnels::debugGetChances() const {
	// @ref ZoombiniPuzzleTunnels::_remainingRejectChances decreases only for a rejection.
	// The Crystal and Igno retry counters select reaction animations and reset after a successful entrance.
	// They are not the chance tally.
	const int16 opportunities = static_cast<int16>(14 + 2 * _difficultyLevel);
	const int16 used = CLIP<int16>(static_cast<int16>(opportunities - _remainingRejectChances), 0, opportunities);
	return {ZmbChanceInfo::ZmbChanceType::kMistake, opportunities, used, "wrong tunnel entrance"};
}

bool ZoombiniPuzzleTunnels::debugSetChances(int16 remaining) {
	const int16 opportunities = static_cast<int16>(14 + 2 * _difficultyLevel);
	if (remaining < 0 || opportunities < remaining)
		return false;

	const int16 previousRemaining = _remainingRejectChances;
	_remainingRejectChances = remaining;
	if (remaining != previousRemaining && !_rockfallWarningSoundPlaying) {
		_rockfallWarningSoundId = 0;
		switch (_remainingRejectChances) {
		case 1:
			_rockfallWarningSoundId = kResSound4703_Rockfall1Remaining;
			break;
		case 2:
			_rockfallWarningSoundId = kResSound4702_Rockfall2Remaining;
			break;
		case 3:
			_rockfallWarningSoundId = kResSound4701_Rockfall3Remaining;
			break;
		case 4:
			_rockfallWarningSoundId = kResSound4700_Rockfall4Remaining;
			break;
		default:
			break;
		}
		playRockfallWarningFeedback();
	}
	return true;
}

Common::String ZoombiniPuzzleTunnels::debugGetAnswer() const {
	// 4 cave entrances are in a single horizontal row (y~=415-430):
	//   Zone 1: x=98  (leftmost)     Zone 2: x=178 (left-center)
	//   Zone 3: x=453 (right-center) Zone 4: x=533 (rightmost)
	// Guard A = Rock left (Crystal) / Rock right (Igno):
	//   true -> Crystal's pair (zones 1,2 left); false -> Igno's pair (zones 3,4 right)
	// Guard B = Rock top (Onyx) / Rock bottom (Ferrous):
	//   true -> Onyx's pair (zones 1,4 outer); false -> Ferrous's pair (zones 2,3 inner)
	// Cave layout:
	//   Zone 1 (leftmost):     Rock left (Crystal) + Rock top (Onyx)
	//   Zone 2 (left-center):  Rock left (Crystal) + Rock bottom (Ferrous)
	//   Zone 3 (right-center): Rock right (Igno)   + Rock bottom (Ferrous)
	//   Zone 4 (rightmost):    Rock right (Igno)   + Rock top (Onyx)
	// Rock names from English manual: "Onyx and Ferrous, Crystal and Ignorameous"
	// The manual labels Crystal on the left and Ignorameous on the right.

	Common::String s = getDebugBanner();
	s += "\n";

	// Level 1 opens only one row of two entrances; the blocked-pair toggle blocks
	// the other row entirely, so every drop there is rejected.
	//   bias = 0 -> top row (zones [1] + [4]) accepts, bottom row blocked
	//   bias = 1 -> bottom row (zones [2] + [3]) accepts, top row blocked

	// Guards are checked in order: left/right first, then top/bottom.
	// Each opposite rock accepts the complementary result for the same guard.
	s += "  Rock rules:\n";
	if (1 <= _guardAxisCount) {
		const char *crystalMatch;
		const char *ignoMatch;
		if (_guardRules[0].traitMatchOnPrimarySide) {
			crystalMatch = "a listed trait";
			ignoMatch = "none of the listed traits";
		} else {
			crystalMatch = "none of the listed traits";
			ignoMatch = "a listed trait";
		}
		const Common::String traits = formatGuardTraitDescription(_guardRules[0]);
		s += Common::String::format("    Crystal (Left):\n"
									"      accepts when it matches %s:\n"
									"        %s\n",
									crystalMatch, traits.c_str());
		s += Common::String::format("    Igno (Right):\n"
									"      accepts when it matches %s:\n"
									"        %s\n",
									ignoMatch, traits.c_str());
	}
	if (2 <= _guardAxisCount) {
		const char *onyxMatch;
		const char *ferrousMatch;
		if (_guardRules[1].traitMatchOnPrimarySide) {
			onyxMatch = "a listed trait";
			ferrousMatch = "none of the listed traits";
		} else {
			onyxMatch = "none of the listed traits";
			ferrousMatch = "a listed trait";
		}
		const Common::String traits = formatGuardTraitDescription(_guardRules[1]);
		s += Common::String::format("    Onyx (Top):\n"
									"      accepts when it matches %s:\n"
									"        %s\n",
									onyxMatch, traits.c_str());
		s += Common::String::format("    Ferrous (Bottom):\n"
									"      accepts when it matches %s:\n"
									"        %s\n",
									ferrousMatch, traits.c_str());
	}

	s += "  Cave entrances:\n";
	if (_guardAxisCount == 1) {
		// The level-1 blocked-pair toggle leaves one entry for each guard result.
		int16 matchZone;
		int16 nonMatchZone;
		const char *activeSet;
		const char *blockedSet;
		const char *crystalCond;
		const char *ignoCond;
		if (_level1BlockedPairToggle) {
			matchZone = 2;
			nonMatchZone = 3;
			activeSet = "bottom";
			blockedSet = "top";
		} else {
			matchZone = 1;
			nonMatchZone = 4;
			activeSet = "top";
			blockedSet = "bottom";
		}
		if (_guardRules[0].traitMatchOnPrimarySide) {
			crystalCond = "matches a listed trait";
			ignoCond = "matches none of the listed traits";
		} else {
			crystalCond = "matches none of the listed traits";
			ignoCond = "matches a listed trait";
		}
		s += Common::String::format("    Accepts answers only at the %s entrances, the %s is blocked.\n", activeSet, blockedSet);
		s += Common::String::format("      [%d] Crystal (%s)\n", matchZone, crystalCond);
		s += Common::String::format("      [%d] Igno   (%s)\n", nonMatchZone, ignoCond);
	} else if (2 <= _guardAxisCount) {
		s += "    Satisfy both named rock rules.\n";
		s += "    [1] leftmost:     Crystal + Onyx\n";
		s += "    [2] left-center:  Crystal + Ferrous\n";
		s += "    [3] right-center: Igno + Ferrous\n";
		s += "    [4] rightmost:    Igno + Onyx\n";
	}
	return s;
}

ZmbEventHandleResult ZoombiniPuzzleTunnels::onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) {
	ZmbEventHandleResult result = ZoombiniInteractive::onLButtonDown(absPos, relPos);
	if (result == ZmbEventHandleResult::kConsumed)
		return result;

	// Do not reserve a tunnel entry while another Snoid is still walking to a pack or output seat.
	if (0 < _vm->_walkersInProgress) {
		debugC(5, MohawkEngine_Zoombini::kDebugPage02, "tunnels: ignored zoombini input while %d path walkers remain", _vm->_walkersInProgress);
		return ZmbEventHandleResult::kPassthrough;
	}

	// Check if clicking on a Zoombini to start drag
	if (!_postGameStarted) {
		ZmbSnoid *snoid = findSnoidAtPoint(absPos);
		if (snoid && snoid->getAnimState() == kSnoidAnimState000_Idle) {
			// Queueing another Snoid is allowed while the head sequence is active.
			// Only a stale head with no runner is cleared here.
			if (0 < _animQueueCount && !_animQueue[0].snoidId) {
				_animQueue[0] = AnimQueueEntry();
				_animQueueCount = 0;
				_animationQueueAdvanceEnabled = true;
			}

			_draggedSnoidWasQueued = removeAnimQueueEntry(snoid->getId()) != 0;
			startSnoidDrag(snoid, absPos);
			return ZmbEventHandleResult::kConsumed;
		}
	}

	return ZmbEventHandleResult::kPassthrough;
}

// =========================================================================
// End the current drag operation.
// =========================================================================

void ZoombiniPuzzleTunnels::endDrag(const Common::Point &dropPos) {
	(void)dropPos;
	ZmbSnoid *snoid = finishSnoidDrag();
	if (!snoid)
		return;
	bool wasInQueue = _draggedSnoidWasQueued;
	_draggedSnoidWasQueued = false;

	// Hit-test the dragged runner position, which may differ from the mouse position by the pickup offset.
	int16 zone = getDropZone(snoid->getPointLoc());

	if (0 < zone) {
		const int16 seatIdx = zone - 1;

		// Claim the seat, snap the destination to its registered point, and enter state 4.
		// State 4 settles at the fixed seat position and then becomes idle.
		setDrawOnRegOccupant(seatIdx, snoid->getId());
		snoid->setAnimTargetPos(kTunnelEntryPositions[seatIdx]);
		snoid->setAnimState(kSnoidAnimState004_Arrive);

		bool guardAMatch = false;
		bool isRejection = evaluateRule(snoid, zone, guardAMatch);

		// Set overlay flag on snoid for gate approach
		snoid->addFlag(ZmbFeature::FLAG_04000000_OVERLAY);

		handleZoombiniPlacement(snoid, zone, isRejection, guardAMatch);
		return;
	}

	// Tunnels temporarily suspends positional sorting while a gate actor owns the composite order.
	// A staging-area drop must not retain the temporary tail link used only while the Snoid is held.
	restoreSnoidPreDragRunnerOrder(snoid);

	// The terrain mask owns free placement outside the four fixed entry seats.
	// The queued source fallback is selected only when the shared result changes
	// the raw release destination.
	const bool settledAtRelease = settleSnoidAfterTerrainDrop(snoid, _dragOrigPos);

	if (wasInQueue && !settledAtRelease) {
		// A blocked or collision-adjusted release from a reserved entry seat
		// returns to an unused pack seat. Valid free terrain remains in place.
		const int16 packSeatIdx = selectUnusedPackSeat();
		settleSnoidAtTarget(snoid, kSnoidPositions[packSeatIdx]);
		debugC(4, MohawkEngine_Zoombini::kDebugPage02, "tunnels: returned reserved zoombini %d to pack seat %d", snoid->getId(), packSeatIdx);
	}
}

} // End of namespace Mohawk
