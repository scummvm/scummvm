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
#include "mohawk/zoombini_pages/puzzle_hotel.h"
#include "mohawk/zoombini_random.h"
#include "mohawk/zoombini_resource.h"
#include "mohawk/zoombini_sound.h"
#include "mohawk/zoombini_state.h"

namespace Mohawk {

constexpr Common::Point ZoombiniPuzzleHotel::kIntroSnoidPositions[20];
constexpr Common::Point ZoombiniPuzzleHotel::kGameplaySnoidPositions[16];
constexpr Common::Point ZoombiniPuzzleHotel::kRoomPositions25[25];
constexpr Common::Point ZoombiniPuzzleHotel::kRoomPositions125[125];
constexpr int16 ZoombiniPuzzleHotel::kSubColumnOffsetX[5];
constexpr int16 ZoombiniPuzzleHotel::kSubColumnOffsetY[5];
constexpr int16 ZoombiniPuzzleHotel::kBackgroundSubColumnOffsetY[5];

ZoombiniPuzzleHotel::ZoombiniPuzzleHotel(MohawkEngine_Zoombini *vm) : ZoombiniPuzzle(vm, ZoombiniPageType::kHotel, ZmbSrcPageKind::kHotel_11) {
}

ZoombiniPuzzleHotel::~ZoombiniPuzzleHotel() {
}

void ZoombiniPuzzleHotel::initStates() {
	// Select the counter step reached by the initial fill.
	switch (_difficultyLevel) {
	case kPuzzleLevel1:
		_initialCounterStep = 5;
		break;
	case kPuzzleLevel3:
		_initialCounterStep = 4;
		break;
	default: // Levels 2 and 4
		_initialCounterStep = 2;
		break;
	}

	_mistakeCounterStep = 1;
	_debugRoomAnimCycle = 25;
	_debugRoomAnimCycleAlternate = 25;
	_debugRoomRestartState = 0;
	_debugTraitBypass = false;

	// Select the active room-slot count for this difficulty.
	_roomSlotCount = (_difficultyLevel == kPuzzleLevel4) ? 125 : 25;
	debugC(3, MohawkEngine_Zoombini::kDebugPage02, "hotel: initialized level %d with counter step %d and %d room slots", _difficultyLevel, _initialCounterStep, _roomSlotCount);
}

const ZoombiniPage::ScriptSoundPriorityRanges &ZoombiniPuzzleHotel::getScriptSoundPriorityRanges() const {
	// Original shared page dispatch automatically prepends SND 996-997 at priority 32.
	static const ZoombiniPage::ScriptSoundPriorityRanges kRanges = {
		{kSysResSoundRange0996_DepartSFX, kSysResSoundRange0997_ArriveSFX},
		{kResSoundRange8900_PagePriorityBase, kResSoundRange8901_PagePriorityLast},
		{kSysResSoundRange0996_DepartSFX, kSysResSoundRange0997_ArriveSFX},
		{kSysResSoundRange20000_Base, kSysResSoundRange29999_Last},
		{kResSoundRange0099_PageSingle, kResSoundRange0099_PageSingle},
		{kResSoundRange7000_SystemRangeBase, kResSoundRange7999_SystemRangeLast},
		{kResSoundRange0425_PageRangeBase, kResSoundRange0499_PageRangeLast},
		{kResSoundRange6004_PagePriorityBase, kResSoundRange6006_Overflow},
		{kResSoundRange6000_PageRangeBase, kResSoundRange6099_PageRangeLast},
		{kResSoundRange9004_PagePriorityBase, kResSoundRange9006_PagePriorityLast},
		{kResSoundRange9000_PageRangeBase, kResSoundRange9999_PageRangeLast},
		{kResSoundRange10000_PageRangeBase, kResSoundRange10999_PageRangeLast}};
	return kRanges;
}

Audio::Mixer::SoundType ZoombiniPuzzleHotel::getFeatureSoundType(const ZmbFeature *feature, ZmbResource sndRes) const {
	if (sndRes._archiveKind == ZmbResource::kPage &&
		(sndRes._id == kResSound7000_UllaSpeechSingle ||
		 (kResSound7002_UllaSpeechBase <= sndRes._id && sndRes._id <= kResSound7023_UllaSpeechLast) ||
		 (kResSound7500_UllaSpeechBase <= sndRes._id && sndRes._id <= kResSound7535_UllaSpeechLast)))
		return Audio::Mixer::kSpeechSoundType;
	return ZoombiniPuzzle::getFeatureSoundType(feature, sndRes);
}

void ZoombiniPuzzleHotel::open() {
	// MIDI BGM (tMID 30020-30023) -- Broderbund v1.x only.
	// @ref ZoombiniPage::openMidiArchive() loads MIDIMPC.MHK (Windows profile) or, when "use_mac_midi" is set,
	// MIDIMAC.MHK (Macintosh profile).
	// Both hold the same tMID IDs.
	// TLC v2.0 removed all MIDI resources.
	if (!_vm->isVersionFamilyTlcV2())
		openMidiArchive();
	openArchive(ZMB_MHK_HOTEL);
}

void ZoombiniPuzzleHotel::setBackgroundMusic() {
	// Sound 20081 is reserved for F1 replay and does not start on page load.
}

void ZoombiniPuzzleHotel::setBackgroundBitmap() {
	_vm->_gfx->setPalette(kResBackground5000);
	_vm->_gfx->drawBackground(kResBackground5000);
}

void ZoombiniPuzzleHotel::loadFeatures() {
	// Load terrain barrier bitmap (tBMP 100)
	loadTerrainBitmap(kResBitmapTerrain100);

	// Preload shape images -- main shapes at tBMP 8000
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape8000_Buttons));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape7000_Main));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape7500_Guide));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape10000_RoomDrop));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape11500_Label));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape11800_Room));

	// Level-dependent extra shapes
	if (_difficultyLevel == kPuzzleLevel3) {
		// Preload the level 3 shape and registration resources.
		_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape11000_Forbidden));
		loadShapeOffsetRegs(ZmbResource::kPage, kResRegs11000_Level3);
		loadShapeOffsetRegs(ZmbResource::kPage, kResRegs11002_Level3Shape);
		loadShapeOffsetRegs(ZmbResource::kPage, kResRegs11004_Level3Cell);
	}
	if (_difficultyLevel == kPuzzleLevel4) {
		// Preload the level 4 shape and registration resources.
		_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape12000_Level4Forbidden));
		loadShapeOffsetRegs(ZmbResource::kPage, kResRegs9000_Level4);
		loadShapeOffsetRegs(ZmbResource::kPage, kResRegs9002_Level4);
		loadShapeOffsetRegs(ZmbResource::kPage, kResRegs12004_Level4);
	}

	// Keep the preload-only subfeature chain under the shared puzzle head.
	ZmbFeature *mainFeature = createPuzzleMainFeatureHead();

	// Preload eleven subfeatures from SCRB 7000-7010.
	{
		ZmbFeature *parent = mainFeature;
		for (uint16 i = 0; i < 11; i++) {
			parent = loadSubFeature(parent, ZmbResource(ZmbResource::kPage, kResBitmapShape7000_Main), kResScrb7000_RoomBase + i);
		}
	}

	// Preload 25 or 125 room subfeatures, depending on difficulty.
	{
		uint16 subCount = (_difficultyLevel == kPuzzleLevel4) ? 125 : 25;
		int16 subStart = (_difficultyLevel == kPuzzleLevel4) ? kResScrb10025_Level4RoomDropBase : kResScrb10000_RoomDropBase;
		ZmbFeature *parent = mainFeature;
		for (uint16 i = 0; i < subCount; i++) {
			parent = loadSubFeature(parent, ZmbResource(ZmbResource::kPage, kResBitmapShape10000_RoomDrop), subStart + i);
		}
	}

	// Preload six subfeatures from SCRB 11500-11505.
	{
		ZmbFeature *parent = mainFeature;
		for (uint16 i = 0; i < 6; i++) {
			parent = loadSubFeature(parent, ZmbResource(ZmbResource::kPage, kResBitmapShape11500_Label), kResScrb11500_LabelBase + i);
		}
	}

	// Preload the SCRB 11800 subfeature.
	{
		ZmbFeature *parent = mainFeature;
		parent = loadSubFeature(parent, ZmbResource(ZmbResource::kPage, kResBitmapShape11800_Room), kResScrb11800_Room);
	}

	// Preload ten guide subfeatures on levels 1-3.
	if (_difficultyLevel != kPuzzleLevel4) {
		ZmbFeature *parent = mainFeature;
		for (uint16 i = 0; i < 10; i++) {
			parent = loadSubFeature(parent, ZmbResource(ZmbResource::kPage, kResBitmapShape7500_Guide), kResScrb7500_GuideBase + i);
		}
	}

	// Register two SCRS ranges.
	// Registration order selects the Snoid render state: group 0 (140xx) is state 9 and group 1 (130xx) is state 8.
	if (kPuzzleLevel4 <= _difficultyLevel) {
		registerScrsGroup(kResScrs14025_Level4RejectBase, 25);
		registerScrsGroup(kResScrs13025_Level4NormalBase, 45);
	} else {
		registerScrsGroup(kResScrs14000_RejectBase, 25);
		registerScrsGroup(kResScrs13000_NormalBase, 70);
	}

	// --- Puzzle-specific feature runners ---

	// The opening label overlay uses SCRB 11500 plus the adjusted difficulty.
	// Difficulty 1 selects 11500.
	// Difficulties 2 and 3 select 11501.
	// Difficulty 4 selects 11502.
	{
		int16 introScrbOffset = _difficultyLevel - 1;
		if (kPuzzleLevel3 <= _difficultyLevel)
			introScrbOffset -= 1;
		int16 introScrb = static_cast<int16>(introScrbOffset + kResScrb11500_LabelBase);
		_openingLabelFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape11500_Label), introScrb, 6, ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE);
	}

	// Page-entry SFX initialization also advances the page flag.
	// Perform it here before selecting Ulla's opening animation, which reads the updated value.
	initSfxGroupFlags();

	// Select the distant-opening Ulla animation before registering the page controls and runner.
	_openingAnimVariant = 0;
	switch (_difficultyLevel) {
	case kPuzzleLevel2:
		_openingAnimVariant = 4;
		break;
	case kPuzzleLevel3:
		_openingAnimVariant = 5;
		break;
	case kPuzzleLevel4:
		_openingAnimVariant = 6;
		break;
	default:
		// Level 1 uses type 0 on its first visit and types 1-3 thereafter.
		if (1 < _vm->_state->getPageVisitCountFromPageFlag(_vm->_state->_f._pageFlagHotel))
			_openingAnimVariant = _vm->_rnd->getRandomNumber(1, 3);
		break;
	}
	_guidePromptPending = (!_openingAnimVariant || _openingAnimVariant == 4);

	// The control runner precedes Ulla's opening animation and the active-pack runners.
	configureStandardPuzzleControlRects();
	loadStandardPuzzleControlFeatures(kResBitmapShape8000_Buttons);

	_ullaAnimFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape7000_Main), static_cast<int16>(kResScrb7000_RoomBase + _openingAnimVariant), 6,
									   ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_08000000_REGION_TRACK);

	// Load Zoombinis from active pack at 20 pedestal positions
	loadZoombinisFromPack();

	// Hotel uses its own pedestal layout for loaded Zoombinis.

	// Generate the room rules immediately.
	computeTraitVariantCounts();
	generateRoomRules();
	registerInitialForbiddenScrbs();

	// Register the room runner only after the active-pack rules exist.
	_openingRoomFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape11800_Room), kResScrb11800_Room, 6, ZmbFeature::FLAG_00100000_PLAY_ONCE);

	// Materialize the opening before its completion owner becomes active.
	renderFeatures();

	// Ulla's distant-opening animation owns the completion trigger.
	// @ref ZoombiniPuzzleHotel::onPostRenderFrame() installs the close view after its SCRB completes.
}

void ZoombiniPuzzleHotel::initHelpPrompt() {
	_activeHelpSoundId = ZmbResource(ZmbResource::kSystem, kSysResSound20081_Help);
}

void ZoombiniPuzzleHotel::onGoButtonActivated() {
	// Stop BGM before departure, play SFX 996, and fade when it finishes.
	_vm->_sound->stopAllSoundQueues();

	ZoombiniInteractive::onGoButtonActivated();
}

ZmbChanceInfo ZoombiniPuzzleHotel::debugGetChances() const {
	// @ref ZoombiniPuzzleHotel::_initialCounterStep is the clock's initial filled step, not the mistake limit.
	// A rejected placement closes the hotel when @ref ZoombiniPuzzleHotel::_mistakeCounterStep reaches 12.
	const int16 opportunities = static_cast<int16>(12 - _initialCounterStep);
	const int16 used = CLIP<int16>(static_cast<int16>(_mistakeCounterStep - _initialCounterStep), 0, opportunities);
	return {ZmbChanceInfo::ZmbChanceType::kMistake, opportunities, used, "rejected room placement"};
}

bool ZoombiniPuzzleHotel::debugSetChances(int16 remaining) {
	const int16 opportunities = static_cast<int16>(12 - _initialCounterStep);
	if (remaining < 0 || opportunities < remaining)
		return false;

	const int16 previousRemaining = static_cast<int16>(opportunities - (_mistakeCounterStep - _initialCounterStep));
	const int16 previousCounterStep = _mistakeCounterStep;
	_mistakeCounterStep = _initialCounterStep + opportunities - remaining;
	refreshChancePalette();
	if (remaining == previousRemaining)
		return true;

	if (_difficultyLevel != kPuzzleLevel4 && _mistakeCounterFeature) {
		_initialCounterFillStepPending = false;
		loadScrbOntoFeature(_mistakeCounterFeature, static_cast<int16>(_mistakeCounterStep + kResScrb6000_CounterBase));
		_rejectionCounterAnimCompleted = false;
		_rejectionCounterCallbackPending = true;
	}

	if (_mistakeCounterStep == 9 && previousCounterStep != 9) {
		const int16 escalationScrb = static_cast<int16>(kResScrb7007_EscalationBase + _vm->_rnd->getRandomNumber(0, 2));
		replaceUllaAnimFeature(kResBitmapShape7000_Main, escalationScrb,
							   ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_08000000_REGION_TRACK,
							   kUllaAnimResponse03);
	}

	if (12 <= _mistakeCounterStep && previousCounterStep < 12) {
		_terminalFailureCount += 1;
		_vm->_sound->playSound(ZmbResource(ZmbResource::kPage, kResSound6006_Overflow));
		renderFeatures();
		_failureFreezeDeadlineFrame = getCurrentFrameCounter() + 60;
		_level4FailureReactionPending = _difficultyLevel == kPuzzleLevel4;
	}
	return true;
}

Common::String ZoombiniPuzzleHotel::debugGetAnswer() const {
	const bool axis1Valid = isValidTraitAxis(_axis1TraitAxis);
	const bool axis2Valid = isValidTraitAxis(_axis2TraitAxis);
	const bool axis3Valid = isValidTraitAxis(_axis3TraitAxis);
	const ZmbTrait::TraitKind axis1Category = traitKindForAxis(_axis1TraitAxis);
	const ZmbTrait::TraitKind axis2Category = traitKindForAxis(_axis2TraitAxis);
	const ZmbTrait::TraitKind axis3Category = traitKindForAxis(_axis3TraitAxis);
	const char *axis1Name = axis1Valid ? ZmbTrait::debugTraitKindName(axis1Category) : "?";
	const char *axis2Name = axis2Valid ? ZmbTrait::debugTraitKindName(axis2Category) : "?";
	const char *axis3Name = axis3Valid ? ZmbTrait::debugTraitKindName(axis3Category) : "?";

	Common::String s = getDebugBanner();
	s += "\n";

	s += "  Room assignments are created by accepted placements.\n";
	s += "  The first Zoombini may enter any room that is not boarded up.\n";
	s += "  Boarded rooms:\n";
	bool anyBoardedRoom = false;
	for (int16 room = 0; room < _roomSlotCount; room++) {
		if (_roomSlotStates[room] != -1)
			continue;
		s += Common::String::format("    Room %d\n", room + 1);
		anyBoardedRoom = true;
	}
	if (!anyBoardedRoom)
		s += "    (none)\n";

	if (_difficultyLevel == kPuzzleLevel1) {
		s += "  Grouping:\n";
		s += Common::String::format("    Columns: %s\n", axis1Name);
		s += "  Assigned Column rules:\n";
		bool anyAssignedRule = false;
		for (int16 room = 0; room < 25; room++) {
			if (_axis1TraitConstraints[room] == 0)
				continue;
			s += Common::String::format("    Column %d: %s\n", room + 1, ZmbTrait::debugTraitValueName(axis1Category, _axis1TraitConstraints[room]));
			anyAssignedRule = true;
		}
		if (!anyAssignedRule)
			s += "    (none yet)\n";
	} else if (_difficultyLevel <= kPuzzleLevel3) {
		s += "  Grouping:\n";
		s += Common::String::format("    Rows:    %s\n", axis1Name);
		s += Common::String::format("    Columns: %s\n", axis2Name);
		s += "  Assigned Row rules:\n";
		bool anyAssignedRow = false;
		for (int16 row = 0; row < 5; row++) {
			const int16 value = _axis1TraitConstraints[row];
			if (value != 0) {
				s += Common::String::format("    Row %d:    %s\n", row + 1, ZmbTrait::debugTraitValueName(axis1Category, value));
				anyAssignedRow = true;
			}
		}
		if (!anyAssignedRow)
			s += "    (none yet)\n";
		s += "  Assigned Column rules:\n";
		bool anyAssignedColumn = false;
		for (int16 column = 0; column < 5; column++) {
			const int16 value = _axis2TraitConstraints[5 * column];
			if (value != 0) {
				s += Common::String::format("    Column %d: %s\n", column + 1, ZmbTrait::debugTraitValueName(axis2Category, value));
				anyAssignedColumn = true;
			}
		}
		if (!anyAssignedColumn)
			s += "    (none yet)\n";
	} else {
		s += "  Grouping:\n";
		s += Common::String::format("    Rows:       %s\n", axis1Name);
		s += Common::String::format("    Columns:    %s\n", axis2Name);
		s += Common::String::format("    SubColumns: %s\n", axis3Name);
		s += "  Assigned Row rules:\n";
		bool anyAssignedRow = false;
		for (int16 row = 0; row < 5; row++) {
			const int16 value = _axis1TraitConstraints[row];
			if (value != 0) {
				s += Common::String::format("    Row %d:       %s\n", row + 1, ZmbTrait::debugTraitValueName(axis1Category, value));
				anyAssignedRow = true;
			}
		}
		if (!anyAssignedRow)
			s += "    (none yet)\n";
		s += "  Assigned Column rules:\n";
		bool anyAssignedColumn = false;
		for (int16 column = 0; column < 5; column++) {
			const int16 value = _axis2TraitConstraints[column];
			if (value != 0) {
				s += Common::String::format("    Column %d:    %s\n", column + 1, ZmbTrait::debugTraitValueName(axis2Category, value));
				anyAssignedColumn = true;
			}
		}
		if (!anyAssignedColumn)
			s += "    (none yet)\n";
		s += "  Assigned SubColumn rules:\n";
		bool anyAssignedSubColumn = false;
		for (int16 subColumn = 0; subColumn < 5; subColumn++) {
			const int16 value = _axis3TraitConstraints[subColumn];
			if (value != 0) {
				s += Common::String::format("    SubColumn %d: %s\n", subColumn + 1, ZmbTrait::debugTraitValueName(axis3Category, value));
				anyAssignedSubColumn = true;
			}
		}
		if (!anyAssignedSubColumn)
			s += "    (none yet)\n";
	}

	if (_difficultyLevel == kPuzzleLevel3) {
		s += "  Level 3 temporary generation mapping:\n";
		if (_level3GenerationSnapshotValid) {
			s += "    Rows (dimension 1):\n";
			for (int16 row = 0; row < 5; row++) {
				int16 value = _level3GeneratedAxis1Traits[row];
				s += Common::String::format("      Row %d:    %s\n", row + 1, ZmbTrait::debugTraitValueName(axis1Category, value));
			}
			s += "    Columns (dimension 2):\n";
			for (int16 column = 0; column < 5; column++) {
				int16 value = _level3GeneratedAxis2Traits[column * 5];
				s += Common::String::format("      Column %d: %s\n", column + 1, ZmbTrait::debugTraitValueName(axis2Category, value));
			}
			s += "    Cells (slot=count; * means forbidden):\n";
			for (int16 row = 0; row < 5; row++) {
				s += Common::String::format("      row %d:", row + 1);
				for (int16 column = 0; column < 5; column++) {
					int16 slot = column * 5 + row;
					const char *forbiddenMarker;
					if (_roomSlotStates[slot] == -1)
						forbiddenMarker = "*";
					else
						forbiddenMarker = "";
					s += Common::String::format(" %2d=%d%s", slot + 1, _level3RoomMatchCounts[slot], forbiddenMarker);
				}
				s += "\n";
			}
		} else {
			s += "    (not available; room rules have not been generated)\n";
		}
	}
	return s;
}

Common::String ZoombiniPuzzleHotel::debugGetBuiltinDebugCommandHelp() const {
	Common::String output;
	output += Common::String::format("  %-7s (%s)\n", "Space", kBuiltinDebugActionReinitCounter);
	output += "    Restore counter step 1 and the initial chance palette. Levels 1-3 reload SCRB 6001; room assignments and rules remain unchanged.\n";
	output += Common::String::format("  %-7s (%s)\n", "A/a", kBuiltinDebugActionInfo);
	output += "    Draw the generated F/N/E/H trait-axis initials; level 4 adds the third axis. This does not change the puzzle.\n";
	output += Common::String::format("  %-7s (%s)\n", "H/h", kBuiltinDebugActionStartMode);
	output += "    Arm the level-2 empty-room restart trigger at 1. Repeating H while armed does not change it.\n";
	output += Common::String::format("  %-7s (%s)\n", "I/i", kBuiltinDebugActionIncrementCounter);
	output += "    Advance an armed trigger. H then I reaches 2.\n";
	output += "    On level 2 with no accepted Zoombinis, restart the 15 active room displays and queue page SND 7012.\n";
	output += "    Other levels retain the counter without an effect; a nonempty level-2 board retries after it becomes empty.\n";
	output += Common::String::format("  %-7s (%s)\n", "Shift+R", kBuiltinDebugActionAllowAll);
	output += "    Permanently accept the level 2-3 two-axis and level-4 three-axis validation checks for this page instance. Level 1 is unchanged.\n";
	output += Common::String::format("  %-7s (%s)\n", "W/w", kBuiltinDebugActionCycleRoom);
	output += "    Replace the shared room/guide runner with SCRB 7000-7009. The first press loads 7000; this selector then wraps independently.\n";
	output += Common::String::format("  %-7s (%s)\n", "E/e", kBuiltinDebugActionCycleRoomAlternate);
	output += "    Replace the same runner with SCRB 7010-7017. The first press loads 7010; this selector then wraps independently.\n";
	return output;
}

bool ZoombiniPuzzleHotel::debugDoBuiltinDebugCommand(int argc, const char **argv, Common::String &output) {
	if (argc != 3) {
		output = Common::String::format("Usage: page builtin_debug <%s|%s|%s|%s|%s|%s|%s>\n",
										kBuiltinDebugActionReinitCounter, kBuiltinDebugActionInfo, kBuiltinDebugActionStartMode,
										kBuiltinDebugActionIncrementCounter, kBuiltinDebugActionAllowAll, kBuiltinDebugActionCycleRoom,
										kBuiltinDebugActionCycleRoomAlternate);
		return true;
	}

	const BuiltinDebugAction action = parseBuiltinDebugAction(argv[2]);
	if (action == BuiltinDebugAction::kInvalid) {
		output = Common::String::format("Unknown Hotel Dimensia debug action '%s'.\n", argv[2]);
		output += debugGetBuiltinDebugCommandHelp();
		return true;
	}
	return runBuiltinDebugAction(action, output);
}

ZoombiniPuzzleHotel::BuiltinDebugAction ZoombiniPuzzleHotel::parseBuiltinDebugAction(const Common::String &action) {
	if (action.equalsIgnoreCase(kBuiltinDebugActionReinitCounter))
		return BuiltinDebugAction::kReinitCounter;
	if (action.equalsIgnoreCase(kBuiltinDebugActionInfo))
		return BuiltinDebugAction::kInfo;
	if (action.equalsIgnoreCase(kBuiltinDebugActionStartMode))
		return BuiltinDebugAction::kStartMode;
	if (action.equalsIgnoreCase(kBuiltinDebugActionIncrementCounter))
		return BuiltinDebugAction::kIncrementCounter;
	if (action.equalsIgnoreCase(kBuiltinDebugActionAllowAll))
		return BuiltinDebugAction::kAllowAll;
	if (action.equalsIgnoreCase(kBuiltinDebugActionCycleRoom))
		return BuiltinDebugAction::kCycleRoom;
	if (action.equalsIgnoreCase(kBuiltinDebugActionCycleRoomAlternate))
		return BuiltinDebugAction::kCycleRoomAlternate;
	return BuiltinDebugAction::kInvalid;
}

bool ZoombiniPuzzleHotel::runBuiltinDebugAction(BuiltinDebugAction action, Common::String &output) {
	if (action == BuiltinDebugAction::kReinitCounter) {
		_mistakeCounterStep = 1;
		if (_difficultyLevel != kPuzzleLevel4 && _mistakeCounterFeature)
			loadScrbOntoFeature(_mistakeCounterFeature, static_cast<int16>(kResScrb6000_CounterBase + _mistakeCounterStep), true);
		refreshChancePalette();
		output = "Hotel counter reinitialized.\n";
	} else if (action == BuiltinDebugAction::kInfo) {
		output = debugGetAnswer();
		static constexpr const char *const kAxisLabels[kTraitAxisCount] = {"F", "N", "E", "H"};
		const Common::Rect panelRect(500, 1, 600, 27);
		drawBuiltinDebugPanel(panelRect);
		drawBuiltinDebugPanelText(kAxisLabels[CLIP<int16>(traitAxisIndex(_axis1TraitAxis), 0, kTraitAxisCount - 1)], Common::Rect(500, 1, 549, 27));
		drawBuiltinDebugPanelText(kAxisLabels[CLIP<int16>(traitAxisIndex(_axis2TraitAxis), 0, kTraitAxisCount - 1)], panelRect);
		if (_difficultyLevel == kPuzzleLevel4)
			drawBuiltinDebugPanelText(kAxisLabels[CLIP<int16>(traitAxisIndex(_axis3TraitAxis), 0, kTraitAxisCount - 1)], Common::Rect(550, 1, 600, 27));
	} else if (action == BuiltinDebugAction::kStartMode) {
		if (_debugRoomRestartState == 0)
			_debugRoomRestartState = 1;
		output = Common::String::format("Hotel level-2 empty-room restart trigger: %d\n", _debugRoomRestartState);
	} else if (action == BuiltinDebugAction::kIncrementCounter) {
		if (_debugRoomRestartState != 0)
			_debugRoomRestartState += 1;
		output = Common::String::format("Hotel level-2 empty-room restart trigger: %d\n", _debugRoomRestartState);
	} else if (action == BuiltinDebugAction::kAllowAll) {
		_debugTraitBypass = true;
		output = "Hotel level 2-3 two-axis and level-4 three-axis validation now accepts all candidates for this page instance.\n";
	} else if (action == BuiltinDebugAction::kCycleRoom || action == BuiltinDebugAction::kCycleRoomAlternate) {
		const bool alternate = action == BuiltinDebugAction::kCycleRoomAlternate;
		int16 &cycle = alternate ? _debugRoomAnimCycleAlternate : _debugRoomAnimCycle;
		const int16 first = alternate ? 10 : 0;
		const int16 last = alternate ? 17 : 9;
		cycle += 1;
		if (last < cycle)
			cycle = first;
		const int16 scrbId = static_cast<int16>(kResScrb7000_RoomBase + cycle);
		if (_ullaAnimFeature) {
			loadScrbOntoFeature(_ullaAnimFeature, scrbId, true);
		} else {
			replaceUllaAnimFeature(kResBitmapShape7000_Main, scrbId,
								   ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_08000000_REGION_TRACK,
								   kUllaAnimIntro00);
		}
		output = Common::String::format("Hotel room SCRB %d loaded.\n", scrbId);
	} else {
		return true;
	}

	return false;
}

ZmbEventHandleResult ZoombiniPuzzleHotel::onDebugKeyDown(const Common::KeyState &kbd) {
	BuiltinDebugAction action = BuiltinDebugAction::kInvalid;
	if (kbd.hasFlags(0) || kbd.hasFlags(Common::KBD_SHIFT)) {
		if (kbd.keycode == Common::KEYCODE_SPACE)
			action = BuiltinDebugAction::kReinitCounter;
		else if (kbd.ascii == 'A' || kbd.ascii == 'a')
			action = BuiltinDebugAction::kInfo;
		else if (kbd.ascii == 'H' || kbd.ascii == 'h')
			action = BuiltinDebugAction::kStartMode;
		else if (kbd.ascii == 'I' || kbd.ascii == 'i')
			action = BuiltinDebugAction::kIncrementCounter;
		else if (kbd.ascii == 'R')
			action = BuiltinDebugAction::kAllowAll;
		else if (kbd.ascii == 'W' || kbd.ascii == 'w')
			action = BuiltinDebugAction::kCycleRoom;
		else if (kbd.ascii == 'E' || kbd.ascii == 'e')
			action = BuiltinDebugAction::kCycleRoomAlternate;
	}
	if (action == BuiltinDebugAction::kInvalid)
		return ZmbEventHandleResult::kPassthrough;

	Common::String output;
	runBuiltinDebugAction(action, output);
	return ZmbEventHandleResult::kConsumed;
}

void ZoombiniPuzzleHotel::loadZoombinisFromPack() {
	Common::Array<ZmbSnoid *> snoids;
	_pageLoadedZmbCount = loadOccupiedSnoidsFromActivePack(kIntroSnoidPositions, ARRAYSIZE(kIntroSnoidPositions), &snoids);

	// Use system tBMP 3200 with REGS 3200/3201 and the compact trait-offset tables.
	// Both the image resource and compact hotspot tables are selected per Snoid.
	for (ZmbSnoid *snoid : snoids) {
		snoid->setResource(ZmbResource(ZmbResource::kSystem, kSysResShapeBitmap3200_SnoidSmall));
		snoid->setupSmallIdleHotspots();
	}
}

// ---------------------------------------------------------------------------
// Count distinct trait values per axis.
// ---------------------------------------------------------------------------
void ZoombiniPuzzleHotel::computeTraitVariantCounts() {
	memset(_traitVariantCounts, 0, sizeof(_traitVariantCounts));

	// Count each trait variant per axis.
	byte traitCounts[kTraitAxisCount][6] = {};

	Common::Array<ZmbSnoid *> snoids;
	collectPackSnoids(snoids, true);
	for (const ZmbSnoid *snoid : snoids) {
		for (int16 axisIdx = 0; axisIdx < kTraitAxisCount; axisIdx++) {
			const byte traitValue = getAxisTraitValue(snoid->_trait, static_cast<TraitAxis>(axisIdx));
			if (1 <= traitValue && traitValue <= 5)
				traitCounts[axisIdx][traitValue] += 1;
		}
	}

	for (int16 axisIdx = 0; axisIdx < kTraitAxisCount; axisIdx++) {
		for (int columnIdx = 1; columnIdx <= 5; columnIdx++) {
			if (traitCounts[axisIdx][columnIdx])
				_traitVariantCounts[axisIdx] += 1;
		}
	}
	debugC(3, MohawkEngine_Zoombini::kDebugPage02, "hotel: loaded %d zoombinis with trait variants [%d, %d, %d, %d]", _pageLoadedZmbCount,
		   _traitVariantCounts[traitAxisIndex(TraitAxis::kFeet00)], _traitVariantCounts[traitAxisIndex(TraitAxis::kNose01)],
		   _traitVariantCounts[traitAxisIndex(TraitAxis::kEyes02)], _traitVariantCounts[traitAxisIndex(TraitAxis::kHair03)]);

}

// ---------------------------------------------------------------------------
// Pick random axes and optional forbidden rooms.
// ---------------------------------------------------------------------------
int ZoombiniPuzzleHotel::countLimitedAxes(int threshold) const {
	int count = 0;
	for (int16 axisIdx = 0; axisIdx < kTraitAxisCount; axisIdx++) {
		if (_traitVariantCounts[axisIdx] < threshold)
			count += 1;
	}
	return count;
}

void ZoombiniPuzzleHotel::generateRoomRules() {
	_level3GenerationSnapshotValid = false;
	memset(_level3GeneratedAxis1Traits, 0, sizeof(_level3GeneratedAxis1Traits));
	memset(_level3GeneratedAxis2Traits, 0, sizeof(_level3GeneratedAxis2Traits));
	memset(_level3RoomMatchCounts, 0, sizeof(_level3RoomMatchCounts));

	bool axisSelectionValid = false;
	do {
		_axis1TraitAxis = static_cast<TraitAxis>(_vm->_rnd->getRandomNumber(0, kTraitAxisCount - 1));
		_axis2TraitAxis = static_cast<TraitAxis>(_vm->_rnd->getRandomNumber(0, kTraitAxisCount - 1));
		_axis3TraitAxis = static_cast<TraitAxis>(_vm->_rnd->getRandomNumber(0, kTraitAxisCount - 1));

		if (_difficultyLevel <= kPuzzleLevel2) {
			int limitedCount = countLimitedAxes(5);
			if (_traitVariantCounts[traitAxisIndex(_axis1TraitAxis)] == 5 && _traitVariantCounts[traitAxisIndex(_axis2TraitAxis)] == 5 &&
				_axis1TraitAxis != _axis2TraitAxis) {
				axisSelectionValid = true;
			} else if (limitedCount < 3 && _axis1TraitAxis != _axis2TraitAxis &&
					   4 <= _traitVariantCounts[traitAxisIndex(_axis1TraitAxis)] && 4 <= _traitVariantCounts[traitAxisIndex(_axis2TraitAxis)]) {
				axisSelectionValid = true;
			} else if (3 <= limitedCount && _axis1TraitAxis != _axis2TraitAxis) {
				axisSelectionValid = true;
			}
		} else if (_difficultyLevel == kPuzzleLevel3) {
			int limitedCount = countLimitedAxes(4);
			if (3 <= limitedCount) {
				if (_axis1TraitAxis != _axis2TraitAxis)
					axisSelectionValid = true;
			} else {
				if (_axis1TraitAxis != _axis2TraitAxis && 4 <= _traitVariantCounts[traitAxisIndex(_axis1TraitAxis)] &&
					4 <= _traitVariantCounts[traitAxisIndex(_axis2TraitAxis)])
					axisSelectionValid = true;
			}
		} else { // Level 4 requires all three axes to be distinct.
			if (_axis1TraitAxis != _axis2TraitAxis && _axis2TraitAxis != _axis3TraitAxis && _axis1TraitAxis != _axis3TraitAxis)
				axisSelectionValid = true;
		}
	} while (!axisSelectionValid);
	debugC(3, MohawkEngine_Zoombini::kDebugPage02, "hotel: selected trait axes %d, %d, and %d", traitAxisIndex(_axis1TraitAxis),
		   traitAxisIndex(_axis2TraitAxis), traitAxisIndex(_axis3TraitAxis));

	// At difficulty 2, forbid rooms whose pre-filled constraint-grid cells have no matching Zoombinis.
	if (_difficultyLevel == kPuzzleLevel3) {
		// Build a temporary 5x5 constraint grid
		int16 tempGrid[25] = {};
		int16 usedRowValues[5] = {};
		int16 usedColumnValues[5] = {};

		// Fill all rows and columns with random unique values.
		for (int16 i = 0; i < 5; i++) {
			int16 rowValue, columnValue;
			do {
				rowValue = _vm->_rnd->getRandomNumber(0, 4);
			} while (usedRowValues[rowValue]);
			usedRowValues[rowValue] = 1;

			do {
				columnValue = _vm->_rnd->getRandomNumber(0, 4);
			} while (usedColumnValues[columnValue]);
			usedColumnValues[columnValue] = 1;

			fillCellRow(6 * i, rowValue + 1, columnValue + 1);
		}

		for (int16 i = 0; i < 25; i++) {
			_level3GeneratedAxis1Traits[i] = _axis1TraitConstraints[i];
			_level3GeneratedAxis2Traits[i] = _axis2TraitConstraints[i];
		}
		_level3GenerationSnapshotValid = true;

		// Count how many Zoombinis match each cell.
		Common::Array<ZmbSnoid *> snoids;
		collectPackSnoids(snoids, true);
		for (const ZmbSnoid *snoid : snoids) {
			int16 ax1 = getAxisTraitValue(snoid->_trait, _axis1TraitAxis);
			int16 ax2 = getAxisTraitValue(snoid->_trait, _axis2TraitAxis);

			for (int16 rowIdx = 0; rowIdx < 5; rowIdx++) {
				for (int16 columnIdx = 0; columnIdx < 5; columnIdx++) {
					int16 slot = columnIdx + 5 * rowIdx;
					if (ax1 == _axis1TraitConstraints[slot] && ax2 == _axis2TraitConstraints[slot])
						tempGrid[slot] += 1;
				}
			}
		}

		// Collect empty cells with no matching Zoombinis.
		int16 emptyCells[25];
		int16 emptyCount = 0;
		for (int16 gridRowIdx = 0; gridRowIdx < 25; gridRowIdx++) {
			_level3RoomMatchCounts[gridRowIdx] = tempGrid[gridRowIdx];
			if (!tempGrid[gridRowIdx]) {
				emptyCells[emptyCount] = gridRowIdx;
				emptyCount += 1;
			}
		}

		// Pick random forbidden count (1..emptyCount), cap at 8
		if (0 < emptyCount) {
			int16 forbiddenCount = _vm->_rnd->getRandomNumber(0, emptyCount - 1) + 1;
			forbiddenCount = MIN<int16>(forbiddenCount, MIN<int16>(8, emptyCount));

			// Pick random empty cells as forbidden
			for (int16 fi = 0; fi < forbiddenCount; fi++) {
				int16 pickIdx;
				do {
					pickIdx = _vm->_rnd->getRandomNumber(0, emptyCount - 1);
				} while (_roomSlotStates[emptyCells[pickIdx]] < 0);

				_roomSlotStates[emptyCells[pickIdx]] = -1;
				_forbiddenMarkerVariants[fi] = _vm->_rnd->getRandomNumber(0, 3);
			}
		}
	}

	// Reset the trait grids.
	memset(_axis1TraitConstraints, 0, sizeof(_axis1TraitConstraints));
	memset(_axis2TraitConstraints, 0, sizeof(_axis2TraitConstraints));
	memset(_axis3TraitConstraints, 0, sizeof(_axis3TraitConstraints));
	_firstPlacementPending = true;
	_mistakeCounterStep = 1;
	_acceptedSnoidCount = 0;
}

void ZoombiniPuzzleHotel::registerInitialForbiddenScrbs() {
	if (_difficultyLevel != kPuzzleLevel3)
		return;

	Common::HashMap<int16, ZmbShapeOffsetRegs *>::const_iterator cellRegsIt = _shapeOffsetRegsMap.find(kResRegs11000_Level3);
	Common::HashMap<int16, ZmbShapeOffsetRegs *>::const_iterator shapeRegsIt = _shapeOffsetRegsMap.find(kResRegs11002_Level3Shape);
	if (cellRegsIt == _shapeOffsetRegsMap.end() || shapeRegsIt == _shapeOffsetRegsMap.end()) {
		error("hotel: required level-3 REGS tables are unavailable");
		return;
	}
	static constexpr uint32 kForbiddenFlags = ZmbFeature::FLAG_00800000_POS_DELTA | ZmbFeature::FLAG_00008000_LOOP_ANIM;

	int forbiddenIndex = 0;
	for (int roomIdx = 0; roomIdx < _roomSlotCount; roomIdx++) {
		if (_roomSlotStates[roomIdx] != -1)
			continue;

		const uint16 forbiddenRoomId = _forbiddenMarkerVariants[forbiddenIndex];
		forbiddenIndex += 1;
		const int16 scrbId = static_cast<int16>(kResScrb11000_ForbiddenBase + forbiddenRoomId);
		_forbiddenMarkerFeatures[roomIdx] = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape11000_Forbidden), scrbId, 0,
															cellRegsIt->_value->getSubImageDelta(static_cast<uint16>(roomIdx)), kForbiddenFlags);
		_forbiddenMarkerFeatures[roomIdx]->setShapeOffsetRegs(shapeRegsIt->_value);
	}
}

// ---------------------------------------------------------------------------
// Switch from the distant-opening-sequence to the close-view setup sequence.
// ---------------------------------------------------------------------------
void ZoombiniPuzzleHotel::setupCloseView() {
	_guidePromptDelayStartFrame = getCurrentFrameCounter();
	// Temporarily mark visible pack Snoids so the board reset can lay out every remaining candidate,
	// then clear the acceptance set below.
	schedulePackSnoids(false, true);

	// Restore the compact body tables immediately after the distant-opening-sequence runner completes.
	// Do this before installing the close-view background and positions.
	// This state is stored per Snoid.
	Common::Array<ZmbSnoid *> snoids;
	collectPackSnoids(snoids, true);
	for (ZmbSnoid *snoid : snoids) {
		snoid->setResource(ZmbResource(ZmbResource::kSystem, kSysResShapeBitmap3000_SnoidNormal));
		snoid->setupIdleHotspots();
	}

	// Free room anim feature if type != 4
	if (_ullaAnimFeature && _openingAnimVariant != 4)
		unloadUllaAnimFeature();

	// Free the opening label feature.
	if (_openingLabelFeature) {
		unloadScrbFeature(_openingLabelFeature);
		_openingLabelFeature = nullptr;
	}

	// Free room SCRB runner
	if (_openingRoomFeature) {
		unloadScrbFeature(_openingRoomFeature);
		_openingRoomFeature = nullptr;
	}

	// At the distant-opening-sequence completion boundary, install background 5001 for levels 1-3 or background 5002 for level 4.
	int16 bgId = (kPuzzleLevel4 <= _difficultyLevel) ? kResBackground5002_Level4 : kResBackground5001_Gameplay;
	_vm->_gfx->drawBackground(bgId);

	switch (_difficultyLevel) {
	case kPuzzleLevel1:
		_vm->_gfx->fillArea(ZoombiniGraphics::kBackScreen, Common::Rect(138, 293, 345, 351));
		_vm->_gfx->fillArea(ZoombiniGraphics::kBackScreen, Common::Rect(386, 309, 516, 362));
		break;
	case kPuzzleLevel2:
	case kPuzzleLevel3:
		_vm->_gfx->fillArea(ZoombiniGraphics::kBackScreen, Common::Rect(120, 45, 526, 362));
		break;
	case kPuzzleLevel4:
		_vm->_gfx->fillArea(ZoombiniGraphics::kBackScreen, Common::Rect(11, 1, 638, 396));
		break;
	}

	// Use the separate 16-position gameplay table.
	reassignPedestalPositions(16);

	// Register room display runners
	registerDisplayScrbs();

	// Complex branching for guide/counter setup
	if (_difficultyLevel == kPuzzleLevel4) {
		// Reset the counters before starting level 4.
		_openingSkipped = false;
		_mistakeCounterStep = 1;
		_guidePromptCompleted = false;
		_openingSkipEnabled = false;
		unloadUllaAnimFeature();
		if (!_vm->isVersionFamilyTlcV2())
			_vm->_midi->playZmbMidi(ZmbResource(ZmbResource::kPage, static_cast<int16>(kResMidi30020_HotelBgmBase + (_difficultyLevel - 1))));
	} else if (_openingSkipped) {
		// Reset the state after a skipped guide.
		_guidePromptPending = false;
		_openingAnimCompleted = false;
		_openingSkipped = false;
		_mistakeCounterStep = 1;
		_guidePromptCompleted = false;
		_openingSkipEnabled = false;
		unloadUllaAnimFeature();
		// The remaining-step count selects the counter SCRB.
		if (!_mistakeCounterFeature) {
			_mistakeCounterFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape6000_Counter), static_cast<int16>(_initialCounterStep + kResScrb6000_CounterBase), 6,
													 ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE);
		} else {
			loadScrbOntoFeature(_mistakeCounterFeature, static_cast<int16>(_initialCounterStep + kResScrb6000_CounterBase));
		}
		if (!_vm->isVersionFamilyTlcV2())
			_vm->_midi->playZmbMidi(ZmbResource(ZmbResource::kPage, static_cast<int16>(kResMidi30020_HotelBgmBase + (_difficultyLevel - 1))));
	} else if (!_openingAnimVariant || (_openingAnimVariant == 4 && _guidePromptPending)) {
		// Play the guide prompt.
		replaceUllaAnimFeature(kResBitmapShape7500_Guide, static_cast<int16>(kResScrb7500_GuideBase + (_difficultyLevel - 1)),
							   ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_08000000_REGION_TRACK,
							   kUllaAnimPrompt01);
		_guidePromptCompleted = false;
	} else if (_openingAnimVariant <= 4 || _guidePromptPending) {
		// Skip the guide and go directly to the counter.
		_openingSkipped = false;
		_mistakeCounterStep = 1;
		_guidePromptCompleted = false;
		_openingSkipEnabled = false;
		// Create counter feature (SCRB 6000)
		_mistakeCounterFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape6000_Counter), kResScrb6000_CounterBase, 6,
												 ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE);
		// This caller uses the immediately returned timing-slot ID as a page-local next-frame latch.
		// This is not the generic completion rule for registered timing groups.
		_initialCounterFillStepPending = true;
		if (!_vm->isVersionFamilyTlcV2())
			_vm->_midi->playZmbMidi(ZmbResource(ZmbResource::kPage, static_cast<int16>(kResMidi30020_HotelBgmBase + (_difficultyLevel - 1))));
	} else {
		// Play the guide with its sounds.
		unloadUllaAnimFeature();
		replaceUllaAnimFeature(kResBitmapShape7500_Guide, static_cast<int16>(kResScrb7500_GuideBase + (_difficultyLevel - 1)),
							   ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_08000000_REGION_TRACK,
							   kUllaAnimPrompt01);
		_guidePromptCompleted = false;
	}

	_terminalFailureCount = 0;
	_failureFreezeDeadlineFrame = 0;
	_level4FailureReactionPending = false;

	// Room acceptance starts empty after each board setup.
	schedulePackSnoids(false, false);
	renderFeatures();
	_vm->resetFidgetActivity();
	debugC(2, MohawkEngine_Zoombini::kDebugPage02, "hotel: finished game-board setup");

}

// ---------------------------------------------------------------------------
// Reposition remaining unaccepted Snoids to the first requested pedestal positions.
// ---------------------------------------------------------------------------
void ZoombiniPuzzleHotel::reassignPedestalPositions(int16 count) {
	int16 posIdx = 0;
	for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end() && posIdx < count; it++) {
		ZmbSnoid *snoid = *it;
		if (!snoid->_packIsOccupied)
			continue;
		// Skip Snoids already accepted into rooms.
		bool placed = false;
		for (uint32 pi = 0; pi < _acceptedSnoidIds.size(); pi++) {
			if (_acceptedSnoidIds[pi] == static_cast<uint16>(snoid->getId())) {
				placed = true;
				break;
			}
		}
		if (placed)
			continue;
		snoid->setPointLoc(kGameplaySnoidPositions[posIdx]);
		snoid->setAnimState(kSnoidAnimState000_Idle);
		posIdx += 1;
	}
}

void ZoombiniPuzzleHotel::replaceUllaAnimFeature(int16 imageId, int16 scrbId, uint32 flags, UllaAnimPurpose purpose) {
	unloadUllaAnimFeature();
	_ullaAnimFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, imageId), scrbId, 6, flags);
	_ullaAnimPurpose = purpose;
}

void ZoombiniPuzzleHotel::unloadUllaAnimFeature() {
	if (!_ullaAnimFeature)
		return;
	// Stop only frame sounds owned by this distant-opening sequence, close-view opening setup,
	// or guide-animation runner before unloading it; other mixed SFX remain active.
	_ullaAnimFeature->stopFrameSounds();
	Common::Array<Common::Rect> drawnRects;
	_ullaAnimFeature->collectDrawRecordRects(drawnRects);
	for (uint i = 0; i < drawnRects.size(); i++)
		addExternalDirtyRect(drawnRects[i]);
	unloadScrbFeature(_ullaAnimFeature);
	_ullaAnimFeature = nullptr;
}

// ---------------------------------------------------------------------------
// Register room-display and hotspot runners.
// ---------------------------------------------------------------------------
void ZoombiniPuzzleHotel::registerDisplayScrbs() {
	// Free the previous board runners before rebuilding their display order.
	for (int i = 0; i < 125; i++) {
		if (_roomDisplayFeatures[i]) {
			unloadScrbFeature(_roomDisplayFeatures[i]);
			_roomDisplayFeatures[i] = nullptr;
		}
		if (_roomIconFeatures[i]) {
			unloadScrbFeature(_roomIconFeatures[i]);
			_roomIconFeatures[i] = nullptr;
		}
		if (_roomBackgroundFeatures[i]) {
			unloadScrbFeature(_roomBackgroundFeatures[i]);
			_roomBackgroundFeatures[i] = nullptr;
		}
		if (_roomDropTargetFeatures[i]) {
			unloadScrbFeature(_roomDropTargetFeatures[i]);
			_roomDropTargetFeatures[i] = nullptr;
		}
		if (_forbiddenMarkerFeatures[i]) {
			unloadScrbFeature(_forbiddenMarkerFeatures[i]);
			_forbiddenMarkerFeatures[i] = nullptr;
		}
	}
	if (_boardMaskFeature) {
		unloadScrbFeature(_boardMaskFeature);
		_boardMaskFeature = nullptr;
	}
	resetDrawOnRegSlots();

	static constexpr uint32 kRoomDisplayFlags = ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE |
												ZmbFeature::FLAG_04000000_OVERLAY;
	static constexpr uint32 kRoomIconFlags = ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE |
											 ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_08000000_REGION_TRACK;
	static constexpr uint32 kLabelFlags = ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00008000_LOOP_ANIM;
	static constexpr uint32 kForbiddenFlags = ZmbFeature::FLAG_00800000_POS_DELTA | ZmbFeature::FLAG_00008000_LOOP_ANIM;
	static constexpr uint32 kDropFlags = ZmbFeature::FLAG_00002000_DRAW_ON_REG | ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM |
										 ZmbFeature::FLAG_01000000_DEFER_RENDER | ZmbFeature::FLAG_04000000_OVERLAY;

	if (_difficultyLevel == kPuzzleLevel1) {
		for (int roomIdx = 4; roomIdx < _roomSlotCount; roomIdx += 5) {
			_roomDisplayFeatures[roomIdx] = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape6000_Counter),
															static_cast<int16>(roomIdx + kResScrb6013_RoomDisplayBase), 6, kRoomDisplayFlags);
		}
		_boardMaskFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape11500_Label), kResScrb11500_LabelBase + 4, 6, kLabelFlags);
		for (int roomIdx = 4; roomIdx < _roomSlotCount; roomIdx += 5) {
			_roomIconFeatures[roomIdx] = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape6000_Counter),
														 static_cast<int16>(roomIdx + kResScrb6038_RoomIconBase), 3, kRoomIconFlags);
		}
		for (int roomSlot = 4; roomSlot < _roomSlotCount; roomSlot += 5) {
			_roomDropTargetFeatures[roomSlot] = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape10000_RoomDrop),
																static_cast<int16>(kResScrb10000_RoomDropBase + roomSlot), 7, kRoomPositions25[roomSlot], kDropFlags);
		}
	} else if (_difficultyLevel <= kPuzzleLevel3) {
		for (int roomIdx = 0; roomIdx < _roomSlotCount; roomIdx++) {
			_roomDisplayFeatures[roomIdx] = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape6000_Counter),
															static_cast<int16>(roomIdx + kResScrb6013_RoomDisplayBase), 6, kRoomDisplayFlags);
		}
		if (_difficultyLevel == kPuzzleLevel3) {
			Common::HashMap<int16, ZmbShapeOffsetRegs *>::const_iterator cellRegsIt = _shapeOffsetRegsMap.find(kResRegs11004_Level3Cell);
			Common::HashMap<int16, ZmbShapeOffsetRegs *>::const_iterator shapeRegsIt = _shapeOffsetRegsMap.find(kResRegs11002_Level3Shape);
			if (cellRegsIt == _shapeOffsetRegsMap.end() || shapeRegsIt == _shapeOffsetRegsMap.end()) {
				error("hotel: required level-3 placement REGS tables are unavailable");
				return;
			}
			ZmbShapeOffsetRegs *forbiddenCellRegs = cellRegsIt->_value;
			ZmbShapeOffsetRegs *forbiddenShapeRegs = shapeRegsIt->_value;
			int fi = 0;
			for (int roomIdx = 0; roomIdx < _roomSlotCount; roomIdx++) {
				if (_roomSlotStates[roomIdx] == -1) {
					const uint16 forbiddenRoomId = _forbiddenMarkerVariants[fi];
					fi += 1;
					int16 obstScrb = static_cast<int16>(forbiddenRoomId + kResScrb11000_ForbiddenBase + 4);
					_forbiddenMarkerFeatures[roomIdx] = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape11000_Forbidden), obstScrb, 0,
																		forbiddenCellRegs->getSubImageDelta(static_cast<uint16>(roomIdx)), kForbiddenFlags);
					_forbiddenMarkerFeatures[roomIdx]->setShapeOffsetRegs(forbiddenShapeRegs);
				}
			}
		}
		_boardMaskFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape11500_Label), kResScrb11500_LabelBase + 3, 6, kLabelFlags);
		for (int roomIdx = 0; roomIdx < _roomSlotCount; roomIdx++) {
			_roomIconFeatures[roomIdx] = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape6000_Counter),
														 static_cast<int16>(roomIdx + kResScrb6038_RoomIconBase), 3, kRoomIconFlags);
		}
		for (int roomSlot = 0; roomSlot < _roomSlotCount; roomSlot++) {
			_roomDropTargetFeatures[roomSlot] = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape10000_RoomDrop),
																static_cast<int16>(kResScrb10000_RoomDropBase + roomSlot), 7, kRoomPositions25[roomSlot], kDropFlags);
		}
	} else {
		static constexpr uint32 kLevel4DisplayFlags = ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM |
													  ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_00800000_POS_DELTA | ZmbFeature::FLAG_04000000_OVERLAY;
		static constexpr uint32 kLevel4BackgroundFlags = ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM |
														 ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_00800000_POS_DELTA |
														 ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_08000000_REGION_TRACK;
		static constexpr uint32 kLevel4DropFlags = ZmbFeature::FLAG_00002000_DRAW_ON_REG | ZmbFeature::FLAG_00008000_LOOP_ANIM |
												   ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_01000000_DEFER_RENDER;

		for (int i = 0; i < _roomSlotCount; i++) {
			_roomDisplayFeatures[i] = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape9000_Level4),
													  static_cast<int16>(i % 5 + kResScrb9000_Level4DisplayBase + 2), 6, getLevel4DisplayPosition(i),
													  kLevel4DisplayFlags);
		}
		_boardMaskFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape11500_Label), kResScrb11500_LabelBase + 5, 6, kLabelFlags);
		for (int i = 0; i < _roomSlotCount; i++) {
			_roomBackgroundFeatures[i] = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape9000_Level4),
														 static_cast<int16>(i % 5 + kResScrb9007_Level4BackgroundBase), 3, getLevel4BackgroundPosition(i),
														 kLevel4BackgroundFlags);
		}
		int fi = 0;
		Common::HashMap<int16, ZmbShapeOffsetRegs *>::const_iterator displayRegsIt = _shapeOffsetRegsMap.find(kResRegs9000_Level4);
		if (displayRegsIt == _shapeOffsetRegsMap.end()) {
			error("hotel: required level-4 display REGS table is unavailable");
			return;
		}
		for (int roomIdx = 0; roomIdx < _roomSlotCount; roomIdx++) {
			if (_roomSlotStates[roomIdx] == -1) {
				const uint16 forbiddenRoomId = _forbiddenMarkerVariants[fi];
				fi += 1;
				int16 obstScrb = static_cast<int16>(forbiddenRoomId + kResScrb12000_Level4ForbiddenBase);
				int16 subColumn = roomIdx % 5;
				Common::Point forbiddenPoint = displayRegsIt->_value->getSubImageDelta(static_cast<uint16>(roomIdx / 5));
				forbiddenPoint.x += kSubColumnOffsetX[subColumn];
				forbiddenPoint.y += kBackgroundSubColumnOffsetY[subColumn];
				if (subColumn == 0)
					forbiddenPoint.y += 5;
				else if (subColumn == 2)
					forbiddenPoint.y += 2;
				_forbiddenMarkerFeatures[roomIdx] = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape12000_Level4Forbidden), obstScrb, 0,
																	forbiddenPoint, kForbiddenFlags);
			}
		}
		for (int roomSlot = 0; roomSlot < _roomSlotCount; roomSlot++) {
			_roomDropTargetFeatures[roomSlot] = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape10000_RoomDrop),
																static_cast<int16>(kResScrb10025_Level4RoomDropBase + roomSlot), 6, kRoomPositions125[roomSlot],
																kLevel4DropFlags);
		}
	}
}

Common::Point ZoombiniPuzzleHotel::getLevel4DisplayPosition(int16 roomSlot) const {
	Common::HashMap<int16, ZmbShapeOffsetRegs *>::const_iterator regsIt = _shapeOffsetRegsMap.find(kResRegs9000_Level4);
	if (regsIt == _shapeOffsetRegsMap.end()) {
		error("hotel: required level-4 display REGS table is unavailable");
		return Common::Point();
	}
	if (roomSlot < 0 || _roomSlotCount <= roomSlot) {
		error("hotel: invalid level-4 display room slot %d", roomSlot);
		return Common::Point();
	}
	Common::Point point = regsIt->_value->getSubImageDelta(static_cast<uint16>(roomSlot / 5));
	int16 subColumn = roomSlot % 5;
	point.x += kSubColumnOffsetX[subColumn];
	point.y += kSubColumnOffsetY[subColumn];
	return point;
}

Common::Point ZoombiniPuzzleHotel::getLevel4BackgroundPosition(int16 roomSlot) const {
	Common::HashMap<int16, ZmbShapeOffsetRegs *>::const_iterator regsIt = _shapeOffsetRegsMap.find(kResRegs9002_Level4);
	if (regsIt == _shapeOffsetRegsMap.end()) {
		error("hotel: required level-4 background REGS table is unavailable");
		return Common::Point();
	}
	if (roomSlot < 0 || _roomSlotCount <= roomSlot) {
		error("hotel: invalid level-4 background room slot %d", roomSlot);
		return Common::Point();
	}
	Common::Point point = regsIt->_value->getSubImageDelta(static_cast<uint16>(roomSlot / 5));
	int16 subColumn = roomSlot % 5;
	point.x += kSubColumnOffsetX[subColumn];
	point.y += kBackgroundSubColumnOffsetY[subColumn];
	return point;
}

void ZoombiniPuzzleHotel::loadLevel4RoomScrb(ZmbFeature *feature, int16 scrbId) {
	if (!feature)
		return;

	loadScrbOntoFeature(feature, scrbId, true, true);
	// REGS 9000/9001 and 9002/9003 are room-layout coordinate tables.
	// They share the image resource number but are not per-shape registration tables.
	// Keep the same null shape-registration state used when these runners are first registered.
	feature->setShapeOffsetRegs(nullptr);
	feature->requestVisualRematerialization();
}

// ---------------------------------------------------------------------------
// Check whether two selected trait-axis values satisfy the room constraints for a zero-based slot.
// ---------------------------------------------------------------------------
// The helper interface presents axis values in numbered order for readability.
// Axis 1 remains the Row value, axis 2 the Column value, and axis 3 the SubColumn value;
// reordering the parameters does not change validation or grid-storage behavior.
bool ZoombiniPuzzleHotel::validate2TraitPlacement(int16 slot, int16 axis1Val, int16 axis2Val) const {
	if (_debugTraitBypass)
		return true;

	int16 ax1Constraint = _axis1TraitConstraints[slot];
	int16 ax2Constraint = _axis2TraitConstraints[slot];

	if (!ax1Constraint && !ax2Constraint) {
		for (int16 emptySlotScanIdx = 0; emptySlotScanIdx < _roomSlotCount; emptySlotScanIdx++) {
			if (axis2Val == _axis2TraitConstraints[emptySlotScanIdx] ||
				axis1Val == _axis1TraitConstraints[emptySlotScanIdx])
				return false;
		}
		return true;
	}

	if (axis1Val == ax1Constraint && axis2Val == ax2Constraint)
		return true;
	if ((ax1Constraint && axis1Val != ax1Constraint) ||
		(ax2Constraint && axis2Val != ax2Constraint))
		return false;

	// A room with only one established axis accepts a new value for the other
	// axis only if that value is still unused across the corresponding grid.
	if (axis1Val == ax1Constraint && !ax2Constraint) {
		for (int16 axis2UniquenessIdx = 0; axis2UniquenessIdx < _roomSlotCount; axis2UniquenessIdx++) {
			if (axis2Val == _axis2TraitConstraints[axis2UniquenessIdx])
				return false;
		}
		return true;
	}
	if (axis2Val == ax2Constraint && !ax1Constraint) {
		for (int16 axis1UniquenessIdx = 0; axis1UniquenessIdx < _roomSlotCount; axis1UniquenessIdx++) {
			if (axis1Val == _axis1TraitConstraints[axis1UniquenessIdx])
				return false;
		}
		return true;
	}

	return false;
}

// ---------------------------------------------------------------------------
// Check whether a Snoid can be placed in a level-4 slot.
// Slot decomposition: subColumn=slot%5, row=slot%25/5, column=slot/25
// Return true for a valid assignment.
//
// With no constraints, require simple uniqueness across five entries.
// If ANY constraint set, cascaded pairwise exclusion:
//   1. For each unset constraint, verify the value doesn't already appear in that grid
//   2. Each set constraint must match the provided value
//   3. If all 3 match exactly -> valid
//   4. For each pair of axes where one matches and the other's constraint is 0,
//      verify the other axis value doesn't appear in that grid's 5 entries
// ---------------------------------------------------------------------------
bool ZoombiniPuzzleHotel::validate3TraitPlacement(int16 slot, int16 axis1Val, int16 axis2Val, int16 axis3Val) const {
	if (_debugTraitBypass)
		return true;

	int16 subColumn = slot % 5;
	int16 row = (slot % 25) / 5;
	int16 column = slot / 25;

	int16 rowConstraint = _axis1TraitConstraints[row];
	int16 columnConstraint = _axis2TraitConstraints[column];
	int16 subColumnConstraint = _axis3TraitConstraints[subColumn];

	// Check pure uniqueness.
	if (!rowConstraint && !columnConstraint && !subColumnConstraint) {
		for (int16 i = 0; i < 5; i++) {
			if (_axis2TraitConstraints[i] && axis2Val == _axis2TraitConstraints[i])
				return false;
			if (_axis1TraitConstraints[i] && axis1Val == _axis1TraitConstraints[i])
				return false;
			if (_axis3TraitConstraints[i] && axis3Val == _axis3TraitConstraints[i])
				return false;
		}
		return true;
	}

	// Check uniqueness first for every unset constraint.
	if (!rowConstraint) {
		for (int16 i = 0; i < 5; i++) {
			if (_axis1TraitConstraints[i] && axis1Val == _axis1TraitConstraints[i])
				return false;
		}
	}
	if (!columnConstraint) {
		for (int16 i = 0; i < 5; i++) {
			if (_axis2TraitConstraints[i] && axis2Val == _axis2TraitConstraints[i])
				return false;
		}
	}
	if (!subColumnConstraint) {
		for (int16 i = 0; i < 5; i++) {
			if (_axis3TraitConstraints[i] && axis3Val == _axis3TraitConstraints[i])
				return false;
		}
	}

	// Match every established constraint.
	if (subColumnConstraint && axis3Val != subColumnConstraint)
		return false;
	if (rowConstraint && axis1Val != rowConstraint)
		return false;
	if (columnConstraint && axis2Val != columnConstraint)
		return false;

	// Accept an exact three-axis match.
	if (axis1Val == rowConstraint && axis3Val == subColumnConstraint && axis2Val == columnConstraint)
		return true;

	// For each axis pair, reject an unconstrained value that already exists in its grid when the other axis matches.

	// Pair (axis1, axis2): axis1 matches but axis2 constraint is 0
	if (axis1Val == rowConstraint && !columnConstraint) {
		for (int16 i = 0; i < 5; i++) {
			if (_axis2TraitConstraints[i] && axis2Val == _axis2TraitConstraints[i])
				return false;
		}
	}

	// Pair (axis2, axis1): axis2 matches but axis1 constraint is 0
	if (axis2Val == columnConstraint && !rowConstraint) {
		for (int16 i = 0; i < 5; i++) {
			if (_axis1TraitConstraints[i] && axis1Val == _axis1TraitConstraints[i])
				return false;
		}
	}

	// Pair (axis1, axis3): axis1 matches but axis3 constraint is 0
	if (axis1Val == rowConstraint && !subColumnConstraint) {
		for (int16 i = 0; i < 5; i++) {
			if (_axis3TraitConstraints[i] && axis3Val == _axis3TraitConstraints[i])
				return false;
		}
	}

	// Pair (axis2, axis3): axis2 matches but axis3 constraint is 0
	if (axis2Val == columnConstraint && !subColumnConstraint) {
		for (int16 i = 0; i < 5; i++) {
			if (_axis3TraitConstraints[i] && axis3Val == _axis3TraitConstraints[i])
				return false;
		}
	}

	// Pair (axis3, axis2): axis3 matches but axis2 constraint is 0
	if (axis3Val == subColumnConstraint && !columnConstraint) {
		for (int16 i = 0; i < 5; i++) {
			if (_axis2TraitConstraints[i] && axis2Val == _axis2TraitConstraints[i])
				return false;
		}
	}

	// Pair (axis3, axis1): axis3 matches but axis1 constraint is 0
	if (axis3Val == subColumnConstraint && !rowConstraint) {
		for (int16 i = 0; i < 5; i++) {
			if (_axis1TraitConstraints[i] && axis1Val == _axis1TraitConstraints[i])
				return false;
		}
	}

	return true;
}

// ---------------------------------------------------------------------------
// Set axis-1 row and axis-2 column constraints in the column-major 25-room layout.
// @p roomSlot selects a target slot from 0 through 24.
// ---------------------------------------------------------------------------
void ZoombiniPuzzleHotel::fillCellRow(int16 roomSlot, int16 axis1Val, int16 axis2Val) {
	for (int cellIdx = 0; cellIdx < 5; cellIdx++) {
		// Axis 1 applies to the same visual row in all five columns.
		_axis1TraitConstraints[5 * cellIdx + roomSlot % 5] = axis1Val;
		// Axis 2 applies to all five rooms in the selected visual column.
		_axis2TraitConstraints[cellIdx + (roomSlot - roomSlot % 5)] = axis2Val;
	}
}

// ---------------------------------------------------------------------------
// Set all three trait grids for level-4 placement.
// ---------------------------------------------------------------------------
void ZoombiniPuzzleHotel::setCellTraitsIn3Grids(int16 roomSlot, int16 axis1Val, int16 axis2Val, int16 axis3Val) {
	_axis1TraitConstraints[roomSlot % 25 / 5] = axis1Val; // Row
	_axis2TraitConstraints[roomSlot / 25] = axis2Val;     // Column
	_axis3TraitConstraints[roomSlot % 5] = axis3Val;      // SubColumn
}

// ---------------------------------------------------------------------------
// Animate a Snoid entering an assigned room slot.
// ---------------------------------------------------------------------------
void ZoombiniPuzzleHotel::placeZoombiniInRoom(int16 roomSlot, ZmbSnoid *snoid) {
	const Common::Point *posTable = (_difficultyLevel == kPuzzleLevel4) ? kRoomPositions125 : kRoomPositions25;
	Common::Point basePos = posTable[roomSlot];
	Common::Point finalPos;
	int16 depth = _roomSlotStates[roomSlot];
	int16 entranceCorrection = 0;

	if (_difficultyLevel == kPuzzleLevel4) {
		basePos.x += 5;
		basePos.y -= 15;
		finalPos.x = basePos.x;
		finalPos.y = basePos.y - 2;
		if (1 < depth) {
			finalPos.x -= 2 * (depth - 1);
			finalPos.y -= (depth - 1);
		}
	} else {
		basePos.x += 24;
		basePos.y -= 7;
		finalPos.y = basePos.y - 2;

		if (_difficultyLevel == kPuzzleLevel1) {
			switch (roomSlot) {
			case 4:
				finalPos.x = basePos.x - 5;
				break;
			case 9:
				finalPos.x = basePos.x - 7;
				break;
			case 14:
				finalPos.x = basePos.x - 3;
				break;
			case 19:
				finalPos.x = basePos.x - 3;
				break;
			case 24:
				finalPos.x = basePos.x - 3;
				break;
			default:
				finalPos.x = basePos.x - 3;
				break;
			}
		} else {
			if (roomSlot <= 4)
				finalPos.x = basePos.x - 8;
			else if (roomSlot <= 9)
				finalPos.x = basePos.x - 6;
			else if (roomSlot <= 14)
				finalPos.x = basePos.x - 5;
			else if (roomSlot <= 19)
				finalPos.x = basePos.x - 4;
			else if (roomSlot <= 24)
				finalPos.x = basePos.x - 5;
			else
				finalPos.x = basePos.x - 3;
		}

		if (!depth || depth % 3 == 1) {
			finalPos.x -= 8;
			finalPos.y = finalPos.y + depth - 1;
		} else if (depth % 3 == 2) {
			finalPos.x -= 1;
			finalPos.y = finalPos.y + depth - 1;
		} else {
			finalPos.x += 6;
			finalPos.y = finalPos.y + depth - 1;
		}
	}

	_acceptedRoomPosition = finalPos;

	int16 feetVal = snoid->_trait._feet;
	int16 scrsBase;
	if (_difficultyLevel == kPuzzleLevel4) {
		scrsBase = static_cast<int16>(5 * (roomSlot % 5) + kResScrs13045_Level4Base);
	} else {
		if (roomSlot < 10) {
			scrsBase = kResScrs13030_NormalBase;
			entranceCorrection = 3;
		} else if (roomSlot < 15) {
			scrsBase = kResScrs13035_NormalBase;
		} else {
			scrsBase = kResScrs13040_NormalBase;
			entranceCorrection = 5;
		}
	}
	int16 scrsId = static_cast<int16>(feetVal + scrsBase - 1);

	if (_difficultyLevel == kPuzzleLevel4) {
		Common::Point displayPoint = getLevel4DisplayPosition(roomSlot);
		int16 left = displayPoint.x;
		int16 top = displayPoint.y;
		int16 right = left + 22;
		int16 bottom = top + 72;
		int16 subColumn = roomSlot % 5;
		int16 roomWithinColumn = roomSlot % 25;

		if (roomWithinColumn == 1) {
			left += 5;
		} else if (roomWithinColumn == 3) {
			left += 3;
		} else {
			left += 4;
		}
		if (3 <= subColumn) {
			int16 xOffset = 1;
			if (subColumn == 3) {
				left -= 1;
				xOffset = 2;
			}
			left += xOffset;
			right += xOffset;
		}
		if (subColumn == 4)
			left -= 1;
		else if (subColumn == 3 && 0 < roomWithinColumn / 5)
			left -= 1;
		_placementScriptRect = Common::Rect(left, top, right, bottom);
	} else {
		int16 top = kRoomPositions25[roomSlot].y - 30;
		int16 bottom = top + 82;
		if (kPuzzleLevel1 < _difficultyLevel && roomSlot % 5 <= 2)
			top -= entranceCorrection;
		int16 left = kRoomPositions25[roomSlot].x - 16;
		_placementScriptRect = Common::Rect(left, top, left + 52, bottom);
	}
	_placementScriptSnoid = snoid;
	_placementScriptPhase = kSnoidScriptAcceptedEntrance01;
	_pendingBodyArrangement = 0;
	const Common::Point *endPos = (_difficultyLevel == kPuzzleLevel4) ? &basePos : nullptr;
	if (startSnoidScrs(snoid, ZmbResource(ZmbResource::kPage, scrsId), ZmbScrsCompletionMode::kReturnToIdle, endPos)) {
		snoid->setFrameInterval(6);
		snoid->resetNextAnimFrame();
	} else {
		finalizeAcceptedPlacement();
	}
}

void ZoombiniPuzzleHotel::startRejectedPlacement(ZmbSnoid *snoid) {
	if (!snoid)
		return;

	int16 scrsId;
	if (_difficultyLevel == kPuzzleLevel4)
		scrsId = static_cast<int16>(kResScrs13025_Level4NormalBase + _targetRoomSlot % 5);
	else
		scrsId = static_cast<int16>(kResScrs13000_NormalBase + _targetRoomSlot);

	if (_difficultyLevel == kPuzzleLevel4) {
		Common::Point point = getLevel4DisplayPosition(_targetRoomSlot);
		_placementScriptRect = Common::Rect(point.x, point.y, point.x + 22, point.y + 72);
	} else {
		int16 left = kRoomPositions25[_targetRoomSlot].x - 16;
		int16 top = kRoomPositions25[_targetRoomSlot].y - 30;
		_placementScriptRect = Common::Rect(left, top, left + 52, top + 82);
	}

	_placementScriptSnoid = snoid;
	_placementScriptPhase = kSnoidScriptRejectedDoor02;
	_pendingBodyArrangement = 0;
	_rejectionAnimActive = true;
	if (startSnoidScrs(snoid, ZmbResource(ZmbResource::kPage, scrsId), ZmbScrsCompletionMode::kReturnToIdle)) {
		snoid->setFrameInterval(3);
		snoid->resetNextAnimFrame();
		if (_roomIconFeatures[_targetRoomSlot]) {
			// Group the room floor with the Snoid through @ref ZoombiniPage::registerFeatureTimingGroup().
			// The pre-existing floor runner owns the timing group, so both retract together.
			registerFeatureTimingGroup(_roomIconFeatures[_targetRoomSlot], snoid);
		}
	} else {
		finalizeRejectedPlacement(snoid);
	}
}

void ZoombiniPuzzleHotel::startRejectedReturn(ZmbSnoid *snoid) {
	unregisterFeatureTimingGroup(snoid);
	Common::Point returnPoint;
	int16 scrsId;

	if (_difficultyLevel == kPuzzleLevel4) {
		returnPoint.x = kRoomPositions125[_targetRoomSlot].x - 15;
		returnPoint.y = 5 * (_targetRoomSlot / 25) + 410;
		scrsId = static_cast<int16>(kResScrs14025_Level4RejectBase + 5 * ((_targetRoomSlot % 25) / 5) + _targetRoomSlot % 5);
	} else {
		returnPoint.x = kRoomPositions25[_targetRoomSlot].x - 23;
		returnPoint.y = 5 * (_targetRoomSlot / 5) + 410;
		scrsId = static_cast<int16>(kResScrs14000_RejectBase + _targetRoomSlot);
	}

	RejectReturnState returnState;
	returnState.snoid = snoid;
	_rejectReturnStates.push_back(returnState);
	_placementScriptSnoid = nullptr;
	_placementScriptPhase = kSnoidScriptNone00;
	_pendingBodyArrangement = 0;
	if (startSnoidScrs(snoid, ZmbResource(ZmbResource::kPage, scrsId), ZmbScrsCompletionMode::kReturnToIdle, &returnPoint)) {
		snoid->resetNextAnimFrame();
	} else {
		snoid->setPointLoc(returnPoint);
		finalizeRejectedPlacement(snoid);
	}
	scatterSnoidsNearReturn(returnPoint.x);
	_rejectionAnimActive = false;
	_roomPlacementInputEnabled = true;
}

void ZoombiniPuzzleHotel::finalizeAcceptedPlacement() {
	ZmbSnoid *snoid = _placementScriptSnoid;
	_placementScriptSnoid = nullptr;
	_placementScriptPhase = kSnoidScriptNone00;
	_pendingBodyArrangement = 0;

	if (snoid) {
		// Preserve the accepted SCRS frame before the pose initializer clears its
		// draw records and prepared layer coverage. The transition can run between
		// passes or from the terminal callback, so queue the damage for both passes.
		const Common::Rect oldClickRect = snoid->getClickRect();
		addDirtyRect(oldClickRect);
		addExternalDirtyRect(oldClickRect);
		if (snoid->hasPreparedVisualRects()) {
			Common::Array<Common::Rect> oldPreparedVisualRects;
			snoid->collectPreparedVisualRects(oldPreparedVisualRects);
			for (uint32 i = 0; i < oldPreparedVisualRects.size(); i++) {
				addDirtyRect(oldPreparedVisualRects[i]);
				addExternalDirtyRect(oldPreparedVisualRects[i]);
			}
		}

		if (_difficultyLevel == kPuzzleLevel4) {
			snoid->setAnimState(kSnoidAnimState000_Idle);
			snoid->setupIdleHotspots();
			snoid->setPointLoc(_acceptedRoomPosition);
		} else {
			// Prime the first feet-specific walking frame before moving it to the stored room position.
			// The accepted SCRS frame must not survive the transition into the placed pose.
			snoid->initDirectWalkToTarget(_acceptedRoomPosition, this);
			snoid->setPointLoc(_acceptedRoomPosition);
		}

		// Materialize the replacement pose at its final room position now. The
		// persistent shape screen must restore the old layers before drawing it.
		prepareSnoidVisualCoverage(snoid, true);
		const Common::Rect &newClickRect = snoid->getClickRect();
		addDirtyRect(newClickRect);
		addExternalDirtyRect(newClickRect);
		snoid->setNeedsRedraw(true);
	}

	_roomPlacementInputEnabled = true;
	if (_pageLoadedZmbCount <= _acceptedSnoidCount) {
		_targetRoomSlot = 200;
		if (_difficultyLevel == kPuzzleLevel4)
			return;

		registerWinCheckpoints();
		int16 cheerScrb = static_cast<int16>(7507 + _vm->_rnd->getRandomNumber(0, 2));
		if (_ullaAnimFeature) {
			// Reuse the existing Ulla runner when it survived the close-view-opening-sequence.
			// This preserves its current position and display order.
			loadScrbOntoFeature(_ullaAnimFeature, cheerScrb);
			_ullaAnimPurpose = kUllaAnimCompletion02;
		} else {
			// An opening/prompt skip may have freed Ulla's runner.
			// Recreate it for the final-declare-end-sequence.
			replaceUllaAnimFeature(kResBitmapShape7500_Guide, cheerScrb,
								   ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_08000000_REGION_TRACK,
								   kUllaAnimCompletion02);
		}
	}
}

void ZoombiniPuzzleHotel::finalizeRejectedPlacement(ZmbSnoid *snoid) {
	unregisterFeatureTimingGroup(snoid);
	if (snoid) {
		snoid->setAnimState(kSnoidAnimState000_Idle);
		snoid->setupIdleHotspots();
	}
	int returnState = findRejectReturnState(snoid);
	if (0 <= returnState)
		_rejectReturnStates.remove_at(returnState);
	if (snoid == _placementScriptSnoid) {
		_placementScriptSnoid = nullptr;
		_placementScriptPhase = kSnoidScriptNone00;
		_pendingBodyArrangement = 0;
	}
	_rejectionAnimActive = false;
	_roomPlacementInputEnabled = true;
}

int ZoombiniPuzzleHotel::findRejectReturnState(ZmbSnoid *snoid) const {
	for (uint i = 0; i < _rejectReturnStates.size(); i++) {
		if (_rejectReturnStates[i].snoid == snoid)
			return static_cast<int>(i);
	}
	return -1;
}

void ZoombiniPuzzleHotel::scatterSnoidsNearReturn(int16 returnX) {
	for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
		ZmbSnoid *snoid = *it;
		if (!snoid->isPackSnoid() || !snoid->isRenderActivated())
			continue;
		Common::Point point = snoid->getPointLoc();
		if (point.y < 400 || 440 < point.y)
			continue;
		if (point.x < returnX - 20 || returnX + 20 < point.x)
			continue;

		const bool moveRight = _vm->_rnd->getRandomBool();
		int16 distance = static_cast<int16>(50 + 30 * _vm->_rnd->getRandomNumber(0, 3));
		int16 targetX = moveRight ? returnX + distance : returnX - distance;
		if (moveRight && 520 < targetX) {
			distance = static_cast<int16>(50 + 30 * _vm->_rnd->getRandomNumber(0, 3));
			targetX = returnX - distance;
		} else if (!moveRight && targetX < 15) {
			distance = static_cast<int16>(50 + 30 * _vm->_rnd->getRandomNumber(0, 3));
			targetX = returnX + distance;
		}
		snoid->initDirectWalkToTarget(Common::Point(targetX, 440));
	}
}

// ---------------------------------------------------------------------------
// Dim the palette slightly after a wrong placement.
// ---------------------------------------------------------------------------
void ZoombiniPuzzleHotel::dimPaletteOnError(uint32 duration) {
	byte scalePercent = 92;
	if (_difficultyLevel == kPuzzleLevel1)
		scalePercent = 88;
	else if (_difficultyLevel == kPuzzleLevel3)
		scalePercent = 90;
	// Read back the active palette so each rejection compounds the previous dimming.
	_vm->_gfx->queuePaletteScaleEffect(10, 236, scalePercent, duration);
}

void ZoombiniPuzzleHotel::refreshChancePalette() {
	_vm->_gfx->setPalette(kResBackground5000);

	const int16 opportunities = static_cast<int16>(12 - _initialCounterStep);
	const int16 dimCount = CLIP<int16>(static_cast<int16>(_mistakeCounterStep - _initialCounterStep), 0, opportunities);
	for (int16 i = 0; i < dimCount; i++)
		dimPaletteOnError(1);
}

// ---------------------------------------------------------------------------
// Reload room-icon SCRBs into the win state at slot+6063.
// ---------------------------------------------------------------------------
void ZoombiniPuzzleHotel::registerWinCheckpoints() {
	// Register every fifth room at level 1; levels 2-3 register all rooms, and level 4 registers none.
	if (_difficultyLevel == kPuzzleLevel1) {
		for (int16 roomIdx = 4; roomIdx < _roomSlotCount; roomIdx += 5) {
			if (_roomIconFeatures[roomIdx]) {
				loadScrbOntoFeature(_roomIconFeatures[roomIdx], static_cast<int16>(roomIdx + kResScrb6063_RoomIconWinBase));
			}
		}
	} else if (_difficultyLevel <= kPuzzleLevel3) {
		for (int16 i = 0; i < _roomSlotCount; i++) {
			if (_roomIconFeatures[i]) {
				loadScrbOntoFeature(_roomIconFeatures[i], static_cast<int16>(i + kResScrb6063_RoomIconWinBase));
			}
		}
	}
	// Difficulty 4 registers no win checkpoints.
}

// ---------------------------------------------------------------------------
// Handle the end of a feature animation cycle.
// ---------------------------------------------------------------------------
void ZoombiniPuzzleHotel::onFeatureAnimEvent(ZmbFeature *feature, int16 eventCode) {
	if (feature && feature->hasFlag(ZmbFeature::FLAG_00000001_TYPE_SNOID)) {
		ZmbSnoid *snoid = static_cast<ZmbSnoid *>(feature);
		int returnState = findRejectReturnState(snoid);
		if (kAnimEvent240_BodyArrangePendFirst <= eventCode &&
			eventCode <= kAnimEvent243_BodyArrangePendLast) {
			int16 arrangement = eventCode - (kAnimEvent240_BodyArrangePendFirst - 1);
			if (0 <= returnState)
				_rejectReturnStates[returnState].pendingBodyArrangement = arrangement;
			else
				_pendingBodyArrangement = arrangement;
			return;
		} else if (kAnimEvent250_BodyArrangeDirectFirst <= eventCode &&
				   eventCode <= kAnimEvent253_BodyArrangeDirectLast) {
			snoid->setTraitLayout(static_cast<ZmbScriptDecoder::TraitLayout>(eventCode - kAnimEvent250_BodyArrangeDirectFirst));
			return;
		} else if (eventCode == kRoomEventToggleSnoidFacing00) {
			// The room-entry SCRS reached its facing marker.
			// Toggle the Snoid and apply any body arrangement queued by the preceding event.
			snoid->setFacingLeft(!snoid->isFacingLeft());
			if (0 <= returnState &&
				_rejectReturnStates[returnState].pendingBodyArrangement != 0) {
				snoid->setTraitLayout(static_cast<ZmbScriptDecoder::TraitLayout>(_rejectReturnStates[returnState].pendingBodyArrangement - 1));
				_rejectReturnStates[returnState].pendingBodyArrangement = 0;
			} else if (_pendingBodyArrangement != 0) {
				snoid->setTraitLayout(static_cast<ZmbScriptDecoder::TraitLayout>(_pendingBodyArrangement - 1));
				_pendingBodyArrangement = 0;
			}
			return;
		} else if (eventCode == kRoomEventCommitPlacedSnoid15 && snoid == _placementScriptSnoid) {
			// The accepted room-entry SCRS reached its placement marker.
			// Keep the Snoid immediately before the label layer and constrain its visible room region.
			if (_boardMaskFeature)
				manualLinkBefore(snoid, _boardMaskFeature);
			snoid->addFlag(ZmbFeature::FLAG_00008000_LOOP_ANIM);
			snoid->addFlag(ZmbFeature::FLAG_04000000_OVERLAY);
			// This constraint is set after the current frame is materialized.
			// The following Snoid tick intersects its visual click/dirty rect with this fixed room entrance rectangle.
			snoid->queueVisualRectConstraint(_placementScriptRect);
			// Accepted scripts retain visible room-entry frames after this placement marker.
			// Their terminal callback owns the transition to the final room pose.
			return;
		} else if (eventCode == kAnimEventM1_End) {
			// The Snoid animation reached its shared terminal marker; finish the active room sequence.
			if (0 <= returnState) {
				finalizeRejectedPlacement(snoid);
				return;
			}
			if (snoid != _placementScriptSnoid) {
				snoid->setAnimState(kSnoidAnimState000_Idle);
				snoid->setupIdleHotspots();
				return;
			}
			switch (_placementScriptPhase) {
			case kSnoidScriptAcceptedEntrance01:
				// Replace the terminal SCRS pose with the final room pose before
				// yielding the render transaction.
				finalizeAcceptedPlacement();
				break;
			case kSnoidScriptRejectedDoor02:
				startRejectedReturn(snoid);
				break;
			default:
				break;
			}
			return;
		}
		return;
	}

	if (eventCode != kAnimEventM1_End)
		return;

	if (feature == _ullaAnimFeature) {
		switch (_ullaAnimPurpose) {
		case kUllaAnimIntro00:
			// Ulla's distant-opening animation completed, so install the close view.
			if (_openingAnimActive) {
				_openingAnimActive = false;
				_openingAnimCompleted = true;
			}
			break;
		case kUllaAnimPrompt01:
			// Prompt animation done (guide SCRB 7500+diff).
			_guidePromptCompleted = true;
			break;
		case kUllaAnimCompletion02:
			// Cheer/win animation done -> fires.
			_completionAnimCompleted = true;
			break;
		case kUllaAnimResponse03:
			_ullaAnimPurpose = kUllaAnimIntro00;
			break;
		default:
			break;
		}
		return;
	}

	if (feature == _mistakeCounterFeature) {
		if (_rejectionCounterCallbackPending) {
			_rejectionCounterCallbackPending = false;
			_rejectionCounterAnimCompleted = true;
		}
		return;
	}
}

// ---------------------------------------------------------------------------
// Advance the Hotel state machine once per animation frame.
// ---------------------------------------------------------------------------
void ZoombiniPuzzleHotel::onPostRenderFrame() {
	// A pending departure still renders its active feature scripts, but must not
	// consume their callbacks or start another Hotel-owned state transition.
	if (isDeparturePending())
		return;

	// Skip interactive dispatch until the deadline elapses.
	// This holds the terminal-failure visual stable for 60 frames without dispatching new state transitions.
	if (_failureFreezeDeadlineFrame != 0) {
		if (getCurrentFrameCounter() < _failureFreezeDeadlineFrame)
			return;
		_failureFreezeDeadlineFrame = 0;
		if (_level4FailureReactionPending) {
			_level4FailureReactionPending = false;
			(void)_vm->_rnd->getRandomNumber(0, 3);
			_vm->_sound->playSound(ZmbResource(ZmbResource::kPage, kResSound7500_Level4Overflow), Audio::Mixer::kSpeechSoundType);
		}
		return;
	}

	// [Priority 1] React after the rejection counter animation finishes.
	// On levels 1-3, terminal failure selects one of Ulla's reactions in SCRBs 7503-7506.
	if (_rejectionCounterAnimCompleted) {
		debugC(3, MohawkEngine_Zoombini::kDebugPage02, "hotel: completed rejection counter at terminal failure count %d", _terminalFailureCount);
		_rejectionCounterAnimCompleted = false;
		int16 guideScrb = static_cast<int16>(_vm->_rnd->getRandomNumber(0, 3) + kResScrb7503_GuideReactionBase);
		if (_difficultyLevel != kPuzzleLevel4 && 0 < _terminalFailureCount) {
			// Reload the guide with a random SCRB from 7503-7506.
			replaceUllaAnimFeature(kResBitmapShape7500_Guide, guideScrb,
								   ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_08000000_REGION_TRACK,
								   kUllaAnimCompletion02);
		}
		return;
	}

	// [Priority 2] Completion reaction:
	// Fires when Ulla's celebration or terminal-failure SCRB completes.
	if (_completionAnimCompleted) {
		debugC(2, MohawkEngine_Zoombini::kDebugPage02, "hotel: completed puzzle reaction and enabled Go");
		_completionAnimCompleted = false;
		// The completion reaction also enables Go when no room placement was accepted.
		setGoButtonsEnabled(true);
		return;
	}

	// [Priority 3] Keep refreshing the setup timer throughout the first opening
	// sequence.
	// This also prevents the later guide prompt from replacing the active SCRB.
	if (_openingAnimActive) {
		_guidePromptDelayStartFrame = getCurrentFrameCounter();
		return;
	}

	// When Ulla's distant-opening-sequence finishes or is skipped, install the close view.
	if (_openingAnimCompleted || _openingSkipped) {
		debugC(2, MohawkEngine_Zoombini::kDebugPage02, "hotel: starting close view after opening sequence");
		_openingAnimCompleted = false;
		setupCloseView();
		return;
	}

	if (!_pageActive)
		return;

	// The H built-in debug command arms this level-2-only restart trigger at 1.
	// The I built-in debug command advances the armed trigger to 2.
	// A nonempty board leaves the value at 2 so a later empty frame retries the restart.
	if (_debugRoomRestartState == 2 && _difficultyLevel == kPuzzleLevel2 && _acceptedSnoidCount == 0) {
		static constexpr int16 kActiveRoomSlots[] = {0, 1, 2, 3, 4, 7, 10, 11, 12, 13, 14, 20, 22, 23, 24};
		for (uint roomListIndex = 0; roomListIndex < ARRAYSIZE(kActiveRoomSlots); roomListIndex++) {
			const int16 roomSlot = kActiveRoomSlots[roomListIndex];
			if (_roomDisplayFeatures[roomSlot]) {
				loadScrbOntoFeature(_roomDisplayFeatures[roomSlot], static_cast<int16>(kResScrb6013_RoomDisplayBase + roomSlot));
			}
		}
		_debugRoomRestartState = kResSound7012_DebugRoomRestart;
		queueScriptSoundForNextRenderPass(ZmbResource(ZmbResource::kPage, kResSound7012_DebugRoomRestart), Audio::Mixer::kSpeechSoundType);
	}

	// The gameplay state machine and delayed guide prompt are mutually exclusive.
	// Opening variant 0 starts the prompt immediately after the transition, while variant 4 waits 0xB4 frames.
	const bool gameplayPhaseActive =
		(_openingAnimVariant != 0 && _openingAnimVariant != 4) || !_guidePromptPending || _openingSkipped;
	if (!gameplayPhaseActive) {
		if ((0xB4 < getCurrentFrameCounter() - _guidePromptDelayStartFrame || !_openingAnimVariant) &&
			_ullaAnimFeature) {
			_guidePromptPending = false;
			replaceUllaAnimFeature(kResBitmapShape7500_Guide, static_cast<int16>(kResScrb7500_GuideBase + (_difficultyLevel - 1)),
								   ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_08000000_REGION_TRACK,
								   kUllaAnimPrompt01);
			_guidePromptCompleted = false;
			_guidePromptDelayStartFrame = getCurrentFrameCounter();
		}
		return;
	}

	// --- Main gameplay block ---

	// [Priority 4a] Guide prompt done -> start counter
	if (_guidePromptCompleted || _openingSkipped) {
		debugC(2, MohawkEngine_Zoombini::kDebugPage02, "hotel: starting counter after guide prompt");
		_guidePromptCompleted = false;
		_openingSkipEnabled = false;
		if (!_vm->isVersionFamilyTlcV2())
			_vm->_midi->playZmbMidi(ZmbResource(ZmbResource::kPage, static_cast<int16>(kResMidi30020_HotelBgmBase + (_difficultyLevel - 1))));

		if (_openingSkipped) {
			// The user skipped the guide.
			_openingSkipped = false;
			unloadUllaAnimFeature();
			if (_difficultyLevel != kPuzzleLevel4) {
				// The remaining-step count selects the counter SCRB.
				if (!_mistakeCounterFeature) {
					_mistakeCounterFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape6000_Counter), static_cast<int16>(_initialCounterStep + kResScrb6000_CounterBase), 6,
															 ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE);
				} else {
					loadScrbOntoFeature(_mistakeCounterFeature, static_cast<int16>(_initialCounterStep + kResScrb6000_CounterBase));
				}
			}
		} else if (_difficultyLevel != kPuzzleLevel4) {
			// Start the counter after the normal prompt.
			if (!_mistakeCounterFeature) {
				_mistakeCounterFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape6000_Counter), kResScrb6000_CounterBase, 6,
														 ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE);
			} else {
				loadScrbOntoFeature(_mistakeCounterFeature, kResScrb6000_CounterBase);
			}
			// This caller stores the immediately returned timing-slot ID as a page-local latch.
			// It is not an animation-completion callback or a rule shared by every timing group.
			_initialCounterFillStepPending = true;
		}
		return;
	}

	// [Priority 4b] Consume Hotel's counter-fill latch.
	// The latch does not wait for the current SCRB to finish.
	// Each successful load arms the next step,
	// so fill scripts are replaced on successive frames and only the final script reaches its frame-zero sound event.
	if (_initialCounterFillStepPending && _mistakeCounterFeature) {
		_initialCounterFillStepPending = false;
		if (_mistakeCounterStep <= _initialCounterStep) {
			loadScrbOntoFeature(_mistakeCounterFeature, static_cast<int16>(_mistakeCounterStep + kResScrb6000_CounterBase));
			_initialCounterFillStepPending = true;
			_mistakeCounterStep += 1;
		}
		return;
	}

	// [Priority 4d] Pending placement -> animate accept or reject
	if (_pendingDropSnoid) {
		// The generic drag path first enters Arrive state 4, which applies the
		// registered DRAW_ON_REG position and settles the Snoid facing right.
		// The room-entry or rejection SCRS starts only after that state is idle.
		if (_pendingDropSnoid->getAnimState() != kSnoidAnimState000_Idle)
			return;

		ZmbSnoid *snoid = _pendingDropSnoid;
		debugC(4, MohawkEngine_Zoombini::kDebugPage02, "hotel: processing %s placement at room %d; accepted count is %d of %d",
			   _pendingDropAccepted ? "accepted" : "rejected", _targetRoomSlot, _acceptedSnoidCount, _pageLoadedZmbCount);
		_pendingDropSnoid = nullptr;

		if (_pendingDropAccepted) {
			// --- ACCEPTED PLACEMENT ---
			_acceptedSnoidCount += 1;

			if (0 < _roomSlotStates[_targetRoomSlot]) {
				_roomSlotStates[_targetRoomSlot] = MIN<int16>(_roomSlotStates[_targetRoomSlot] + 1, 6);
			}

			_acceptedSnoidIds.push_back(static_cast<uint16>(snoid->getId()));
			placeZoombiniInRoom(_targetRoomSlot, snoid);

			// Any accepted placement enables Go; the common button state retains this monotonic latch.
			setGoButtonsEnabled(true);

			// Reload the display SCRB when the room receives its first placement.
			// Levels 1-3 use slot + 6013; level 4 reuses (slot % 5) + 9002.
			if (_roomSlotStates[_targetRoomSlot] == 0) {
				if (_roomDisplayFeatures[_targetRoomSlot]) {
					if (_difficultyLevel < kPuzzleLevel4) {
						loadScrbOntoFeature(_roomDisplayFeatures[_targetRoomSlot], static_cast<int16>(_targetRoomSlot + kResScrb6013_RoomDisplayBase));
					} else {
						loadLevel4RoomScrb(_roomDisplayFeatures[_targetRoomSlot],
										   static_cast<int16>((_targetRoomSlot % 5) + kResScrb9000_Level4DisplayBase + 2));
					}
				}
				_roomSlotStates[_targetRoomSlot] = 1;
			}

		} else {
			// --- REJECTED PLACEMENT ---
			_rejectionAnimActive = true;

			// Rejection icon/checkpoint handling
			if (kPuzzleLevel4 <= _difficultyLevel) {
				if (_roomBackgroundFeatures[_targetRoomSlot]) {
					int16 obstBId = static_cast<int16>(_targetRoomSlot % 5 + kResScrb9007_Level4BackgroundBase);
					loadLevel4RoomScrb(_roomBackgroundFeatures[_targetRoomSlot], obstBId);
				}
				if (11 <= _mistakeCounterStep)
					_rejectionAnimActive = true;
			} else if (11 <= _mistakeCounterStep) {
				registerWinCheckpoints();
			} else {
				if (_roomIconFeatures[_targetRoomSlot]) {
					loadScrbOntoFeature(_roomIconFeatures[_targetRoomSlot], static_cast<int16>(_targetRoomSlot + kResScrb6038_RoomIconBase));
				}
			}

			startRejectedPlacement(snoid);

			// Step counter management during rejection
			if (_difficultyLevel == kPuzzleLevel4) {
				_mistakeCounterStep += 1;
			} else if (!_firstPlacementPending) {
				_mistakeCounterStep += 1;
				if (_mistakeCounterFeature) {
					loadScrbOntoFeature(_mistakeCounterFeature, static_cast<int16>(_mistakeCounterStep + kResScrb6000_CounterBase));
					_rejectionCounterAnimCompleted = false;
					_rejectionCounterCallbackPending = true;
				}
			}

			dimPaletteOnError();

			// Escalate Ulla's reaction at step 9.
			if (_mistakeCounterStep == 9) {
				const int16 escalationScrb = static_cast<int16>(kResScrb7007_EscalationBase + _vm->_rnd->getRandomNumber(0, 2));
				replaceUllaAnimFeature(kResBitmapShape7000_Main, escalationScrb,
									   ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_08000000_REGION_TRACK,
									   kUllaAnimResponse03);
			}

			// Terminal failure holds the final counter state and locks further input.
			if (12 <= _mistakeCounterStep) {
				_terminalFailureCount += 1;
				_vm->_sound->playSound(ZmbResource(ZmbResource::kPage, kResSound6006_Overflow));
				renderFeatures();
				// Hold the terminal-failure animation for 60 frames before resuming interactive dispatch.
				_failureFreezeDeadlineFrame = getCurrentFrameCounter() + 60;
				_level4FailureReactionPending = _difficultyLevel == kPuzzleLevel4;
				// Do not reset the board here.
				// The counter-step chain then triggers Ulla's reaction and provides a mercy exit.
			}
		}
		return;
	}
}

// ---------------------------------------------------------------------------
// Select the first Hotel Snoid from its two clickable flag classes.
// ---------------------------------------------------------------------------
ZmbSnoid *ZoombiniPuzzleHotel::findSnoidAtPoint(const Common::Point &pos) {
	ZmbFeature *feature = findRunnerAtPoint(pos, ZmbFeature::FLAG_00000001_TYPE_SNOID);
	if (!feature) {
		feature = findRunnerAtPoint(pos, ZmbFeature::FLAG_00000001_TYPE_SNOID | ZmbFeature::FLAG_00008000_LOOP_ANIM);
	}
	return dynamic_cast<ZmbSnoid *>(feature);
}

bool ZoombiniPuzzleHotel::isPlacedSnoid(const ZmbSnoid *snoid) const {
	if (!snoid)
		return false;

	for (uint32 placedIdx = 0; placedIdx < _acceptedSnoidIds.size(); placedIdx++) {
		if (_acceptedSnoidIds[placedIdx] == snoid->getId())
			return true;
	}

	return false;
}

void ZoombiniPuzzleHotel::startPlacedSnoidDrag(ZmbSnoid *snoid, const Common::Point &mousePos) {
	_acceptedSnoidDragHadConstraint = snoid->hasVisualRectConstraint();
	_acceptedSnoidDragClickRect = snoid->getClickRect();
	if (_acceptedSnoidDragHadConstraint) {
		_acceptedSnoidDragConstraint = snoid->getVisualRectConstraint();
		snoid->clearVisualRectConstraint();
	}
	_acceptedSnoidDrag = true;
	startSnoidDrag(snoid, mousePos);
}

// ---------------------------------------------------------------------------
// Find the first matching room slot at the drop position.
// ---------------------------------------------------------------------------
int16 ZoombiniPuzzleHotel::getDropTargetSlot(const Common::Point &dropPos) const {
	const int16 radius = (kPuzzleLevel4 <= _difficultyLevel) ? 10 : 25;
	int16 drawSlot = hitTestDrawOnRegSlot(dropPos, radius, true);
	if (drawSlot < 0)
		return -1;
	if (_difficultyLevel == kPuzzleLevel1)
		return static_cast<int16>(5 * drawSlot + 4);
	return drawSlot;
}

bool ZoombiniPuzzleHotel::isValidTraitAxis(TraitAxis axis) {
	return TraitAxis::kFeet00 <= axis && axis <= TraitAxis::kHair03;
}

int16 ZoombiniPuzzleHotel::traitAxisIndex(TraitAxis axis) {
	return static_cast<int16>(axis);
}

ZmbTrait::TraitKind ZoombiniPuzzleHotel::traitKindForAxis(TraitAxis axis) {
	switch (axis) {
	case TraitAxis::kFeet00:
		return ZmbTrait::kTraitFeet;
	case TraitAxis::kNose01:
		return ZmbTrait::kTraitNose;
	case TraitAxis::kEyes02:
		return ZmbTrait::kTraitEyes;
	case TraitAxis::kHair03:
	default:
		return ZmbTrait::kTraitHair;
	}
}

byte ZoombiniPuzzleHotel::getAxisTraitValue(const ZmbTrait &trait, TraitAxis axis) {
	if (!isValidTraitAxis(axis))
		return 0;
	return trait.getTraitValue(traitKindForAxis(axis));
}

// ---------------------------------------------------------------------------
// Evaluate a drop after drag release.
// ---------------------------------------------------------------------------
void ZoombiniPuzzleHotel::endDrag(const Common::Point &mousePos) {
	(void)mousePos;
	ZmbSnoid *snoid = finishSnoidDrag();
	if (!snoid)
		return;
	if (_acceptedSnoidDrag) {
		const Common::Rect dragClickRect = snoid->getClickRect();
		_acceptedSnoidDrag = false;
		restoreSnoidPreDragRunnerOrder(snoid);
		snoid->setPointLoc(_dragOrigPos);
		snoid->setCommonImageIndex(1);
		snoid->setFacingLeft(false);
		snoid->setAnimState(kSnoidAnimState000_Idle);
		if (_acceptedSnoidDragHadConstraint) {
			snoid->queueVisualRectConstraint(_acceptedSnoidDragConstraint);
			snoid->applyPendingVisualRectConstraint();
		}
		prepareSnoidVisualCoverage(snoid, true);
		if (_acceptedSnoidDragHadConstraint)
			snoid->setClickRect(_acceptedSnoidDragClickRect);
		addDirtyRect(dragClickRect);
		addExternalDirtyRect(dragClickRect);
		addDirtyRect(snoid->getClickRect());
		addExternalDirtyRect(snoid->getClickRect());
		_acceptedSnoidDragHadConstraint = false;
		snoid->setNeedsRedraw(true);
		return;
	}

	// Clear loaded PCM at the drop boundary before evaluating the room.
	// The bug-fix option preserves MIDI across this cleanup.
	_vm->_sound->releaseAllLoadedSounds();
	const bool stopMidiAfterDrop = !_vm->isVersionFamilyTlcV2() && !ConfMan.getBool(MohawkMetaEngine_Zoombini::kOptionFixHotelMidiHaltBug);
	if (stopMidiAfterDrop)
		_vm->_midi->stopMidi();

	Common::Point dropPos = snoid->getPointLoc();

	// Grade another placement only while room-placement input is enabled and no rejection or terminal failure is active.
	if (!_roomPlacementInputEnabled || _rejectionAnimActive || 0 < _terminalFailureCount || !_pageActive) {
		snoid->setPointLoc(_dragOrigPos);
		snoid->setAnimState(kSnoidAnimState000_Idle);
		snoid->setupIdleHotspots();
		return;
	}

	int16 targetSlot = getDropTargetSlot(dropPos);

	const bool slotOk = 0 <= targetSlot && targetSlot < _roomSlotCount;
	if (!slotOk) {
		// A release outside the room platforms remains a normal terrain drop.
		// This keeps the unaccepted group at the bottom freely rearrangeable.
		settleSnoidAfterTerrainDrop(snoid, _dragOrigPos);
		return;
	}

	_targetRoomSlot = targetSlot;

	// Every registered room platform accepts the shared seat drop first.
	// The Arrive state snaps to the platform and settles the Snoid facing right.
	const Common::Point *roomPositions =
		(kPuzzleLevel4 <= _difficultyLevel) ? kRoomPositions125 : kRoomPositions25;
	settleSnoidAtTarget(snoid, roomPositions[targetSlot]);

	if (_roomSlotStates[targetSlot] < 0) {
		// Closed rooms keep the Snoid on the platform but start no Hotel feedback.
		int16 drawSlot;
		if (_difficultyLevel == kPuzzleLevel1)
			drawSlot = static_cast<int16>(targetSlot / 5);
		else
			drawSlot = targetSlot;
		setDrawOnRegOccupant(drawSlot, snoid->getId());
		debugC(4, MohawkEngine_Zoombini::kDebugPage02, "hotel: retained zoombini at closed room %d", targetSlot);
		return;
	}

	// Retrieve trait values for the selected axes
	int16 axis1Val = getAxisTraitValue(snoid->_trait, _axis1TraitAxis);
	int16 axis2Val = getAxisTraitValue(snoid->_trait, _axis2TraitAxis);
	int16 axis3Val = getAxisTraitValue(snoid->_trait, _axis3TraitAxis);

	bool placementRejected = false;

	if (_firstPlacementPending) {
		// The first placement establishes the room constraints immediately.
		_firstPlacementPending = false;
		_mistakeCounterStep = _initialCounterStep;

		if (_difficultyLevel == kPuzzleLevel1) {
			_axis1TraitConstraints[targetSlot] = axis1Val;
		} else if (_difficultyLevel <= kPuzzleLevel3) {
			fillCellRow(targetSlot, axis1Val, axis2Val);
		} else {
			setCellTraitsIn3Grids(targetSlot, axis1Val, axis2Val, axis3Val);
		}
	} else {
		// Validate against current constraints
		if (_difficultyLevel == kPuzzleLevel1) {
			int16 existing = _axis1TraitConstraints[targetSlot];
			if (existing) {
				placementRejected = (existing != axis1Val);
			} else {
				// Empty slot: check uniqueness across active slots
				for (int i = 4; i < 25; i += 5) {
					if (_axis1TraitConstraints[i] && _axis1TraitConstraints[i] == axis1Val) {
						placementRejected = true;
						break;
					}
				}
			}
		} else if (_difficultyLevel <= kPuzzleLevel3) {
			placementRejected = !validate2TraitPlacement(targetSlot, axis1Val, axis2Val);
			if (!placementRejected) {
				fillCellRow(targetSlot, axis1Val, axis2Val);
			}
		} else {
			placementRejected = !validate3TraitPlacement(targetSlot, axis1Val, axis2Val, axis3Val);
			if (!placementRejected) {
				setCellTraitsIn3Grids(targetSlot, axis1Val, axis2Val, axis3Val);
			}
		}

		// At level 2, establish the room constraints after the first valid acceptance.
		if (_difficultyLevel == kPuzzleLevel1 && !placementRejected) {
			_axis1TraitConstraints[targetSlot] = axis1Val;
		}
	}

	// Retain the drop until common arrival finishes and Hotel can dispatch its result.
	_pendingDropSnoid = snoid;
	_pendingDropAccepted = !placementRejected;
	_roomPlacementInputEnabled = false;
	if (placementRejected)
		debugC(3, MohawkEngine_Zoombini::kDebugPage02, "hotel: rejected placement at room %d", targetSlot);
	else
		debugC(3, MohawkEngine_Zoombini::kDebugPage02, "hotel: accepted placement at room %d", targetSlot);

	if (!placementRejected) {
		// A valid room drop marks the Snoid as occupying a room.
		snoid->_packIsOccupied = true;
		// Queue this immediately after the last Snoid is marked occupied.
		// IDs 175-198 are System ZOOMBINI.MHK sounds.
		if (_acceptedSnoidCount + 1 == _pageLoadedZmbCount) {
			const int16 winSnd = static_cast<int16>(175 + _vm->_rnd->getRandomNumber(0, 23));
			queueNarratorSound(winSnd);
		}
	}
}

// ---------------------------------------------------------------------------
// Handle click and drag initiation.
// ---------------------------------------------------------------------------
ZmbEventHandleResult ZoombiniPuzzleHotel::onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) {
	// While the distant opening or close-view setup sequence is active, every click belongs exclusively to the opening skip path.
	// It cannot fall through to buttons or Snoid dragging.
	if (_openingSkipEnabled) {
		if (_ullaAnimFeature)
			unloadUllaAnimFeature();
		_openingAnimActive = false;
		_openingSkipped = true;
		return ZmbEventHandleResult::kConsumed;
	}

	// Let the base class handle Go, Map, and Help after normal gameplay starts.
	ZmbEventHandleResult result = ZoombiniInteractive::onLButtonDown(absPos, relPos);
	if (result == ZmbEventHandleResult::kConsumed)
		return result;

	// Room-placement input is available only before the final rejected attempt starts the terminal sequence.
	if (!_pageActive || !_roomPlacementInputEnabled || _rejectionAnimActive || 0 < _terminalFailureCount)
		return ZmbEventHandleResult::kPassthrough;
	if (isDragging())
		return ZmbEventHandleResult::kPassthrough;

	ZmbSnoid *snoid = findSnoidAtPoint(absPos);
	if (!snoid)
		return ZmbEventHandleResult::kPassthrough;
	int rejectReturnState = findRejectReturnState(snoid);
	if (0 <= rejectReturnState) {
		// Dragging replaces the active rejected-return SCRS.
		// Retire its callback state so the next room script gets its own terminal event.
		_rejectReturnStates.remove_at(rejectReturnState);
	}
	if (isPlacedSnoid(snoid)) {
		const SnoidAnimState state = snoid->getAnimState();
		if (state == kSnoidAnimState007_Depart ||
			state == kSnoidAnimState008_ScriptReject ||
			state == kSnoidAnimState009_ScriptNormal)
			return ZmbEventHandleResult::kPassthrough;
		startPlacedSnoidDrag(snoid, absPos);
		return ZmbEventHandleResult::kConsumed;
	}

	for (int16 slotIdx = 0; slotIdx < _drawOnRegCount; slotIdx++)
		clearDrawOnRegOccupant(slotIdx);
	startSnoidDrag(snoid, absPos);
	return ZmbEventHandleResult::kConsumed;
}

} // End of namespace Mohawk
