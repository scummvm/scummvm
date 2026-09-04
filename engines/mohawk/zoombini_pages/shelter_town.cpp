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

#include "common/system.h"

#include "mohawk/cursors.h"
#include "mohawk/mohawk.h"
#include "mohawk/sound.h"

#include "mohawk/zoombini.h"
#include "mohawk/zoombini_graphics.h"
#include "mohawk/zoombini_metaengine.h"
#include "mohawk/zoombini_pages/shelter_town.h"
#include "mohawk/zoombini_random.h"
#include "mohawk/zoombini_scripts.h"
#include "mohawk/zoombini_sound.h"
#include "mohawk/zoombini_state.h"
#include "mohawk/zoombini_text.h"

namespace Mohawk {

static const int16 kTownClockTowerMemorialSlot = 4;
static const byte kTownClockHandLastStep = 11;
static const uint32 kTownClockTimeUpdateInterval = 1800;
static const uint32 kTownClockNormalFrameInterval = 6;
static const uint32 kTownClockSpinFrameInterval = 2;
static const byte kTownMemorialCardScrbTypeBySlot[16] = {
	2, 2, 4, 4, 2, 3, 3, 1, 4, 1, 1, 2, 4, 2, 3, 4};
static const byte kTownMemorialCardTextTypeBySlot[16] = {
	7, 1, 4, 5, 13, 3, 10, 11, 14, 15, 0, 8, 12, 9, 2, 6};
static const int16 kTownMemorialMarkerOffsets[16][2] = {
	{0x0048, 0x0046}, {0x0076, 0x007E}, {0x0051, 0x0076}, {0x004D, 0x003F}, {0x0041, 0x00E3}, {0x0090, 0x0053}, {0x006B, 0x0051}, {0x0090, 0x00BA}, {0x0031, 0x0061}, {0x0039, 0x0094}, {0x0056, 0x002E}, {0x0043, 0x0077}, {0x006C, 0x0050}, {0x004C, 0x0061}, {0x002F, 0x00B5}, {0x0077, 0x0041}};
static const int16 kTownMemorialCardRowTopY[5] = {0x0000, 0x0024, 0x00C4, 0x00D2, 0x00E6};
static const int16 kTownMemorialCardRowBottomY[5] = {0x0024, 0x00C4, 0x00D2, 0x00E6, 0x00F4};

constexpr int16 ZoombiniShelterTown::kAmbientVoicePool[5];
constexpr Common::Point ZoombiniShelterTown::kInhabitantPositions[16];
constexpr int16 ZoombiniShelterTown::kInhabitantScrbTable[16];

ZoombiniShelterTown::ZoombiniShelterTown(MohawkEngine_Zoombini *vm) : ZoombiniShelter(vm, ZoombiniPageType::kTown) {
}

ZoombiniShelterTown::~ZoombiniShelterTown() {
}

const ZoombiniPage::ScriptSoundPriorityRanges &ZoombiniShelterTown::getScriptSoundPriorityRanges() const {
	static const ZoombiniPage::ScriptSoundPriorityRanges kRanges = {
		{kResSoundRange3000_BGM, kResSoundRange3003_BGM},
		{kSysResSoundRange20000_Base, kSysResSoundRange29999_Last},
		{kSysResSoundRange0996_DepartSFX, kSysResSoundRange0997_ArriveSFX}};
	return kRanges;
}

Common::String ZoombiniShelterTown::debugGetPageCommandHelp() const {
	Common::String output;
	output += "  devel <population|reset>\n";
	output += "      Preview Town building development without changing the save.\n";
	output += "      population is 0-625 stored Town Zoombinis; reset restores the saved view.\n";
	output += "  fireworks <count|cycle|stop>\n";
	output += "      Test Town celebration walkers without changing the save.\n";
	output += "      count is 1-50 pending walkers; cycle runs the accelerated test; stop cancels queued walkers.\n";
	return output;
}

Common::String ZoombiniShelterTown::debugGetBuiltinDebugCommandHelp() const {
	Common::String output;
	output += Common::String::format("  %-10s (%s)\n", "Space", kBuiltinDebugActionCheatText);
	output += "    Report the 0-16 memorial data-preview selector; 0 is OFF. This does not open a card.\n";
	output += Common::String::format("  %-10s (%s)\n", "f", kBuiltinDebugActionFireworksLast);
	output += "    Stop a nonempty pending celebration queue, or restart it with the retained count (initially 0).\n";
	output += Common::String::format("  %-10s (%s)\n", "Shift+F", kBuiltinDebugActionFireworksNext);
	output += "    Stop a nonempty queue, or advance the retained count by 5 (5-100, wrapping) and start it.\n";
	output += "    Starting either fireworks action shows its count and waits for one input; already spawned walkers finish.\n";
	output += Common::String::format("  %-10s (%s)\n", "x", kBuiltinDebugActionMemorialPrevious);
	output += "    Decrement the memorial data-preview selector and refresh an open card.\n";
	output += Common::String::format("  %-10s (%s)\n", "z", kBuiltinDebugActionMemorialNext);
	output += "    Increment the memorial data-preview selector and refresh an open card.\n";
	output += "    x and z require slot 16 to be occupied, clamp at 0/16, and preview route, level, and date data.\n";
	output += Common::String::format("  %-10s (%s)\n", ".", kBuiltinDebugActionMemorialFill);
	output += "    Fill the first empty saved memorial with today's date and the next deterministic route/level pair.\n";
	output += Common::String::format("  %-10s (%s)\n", "0", kBuiltinDebugActionMemorialClear);
	output += "    Clear all saved memorial route/occupancy bytes and reset insertion order; date/level bytes remain unused.\n";
	output += "    Both memorial mutations schedule the debug-modified state to be saved.\n";
	output += Common::String::format("  %-10s (%s)\n", "Up Arrow", kBuiltinDebugActionThresholdUp);
	output += "    Raise the runtime building-shape threshold by 5, clamped to 25-81, and refresh the overlays.\n";
	output += Common::String::format("  %-10s (%s)\n", "Down Arrow", kBuiltinDebugActionThresholdDown);
	output += "    Lower the same runtime threshold by 5. Saved population, development, and inhabitants do not change.\n";
	return output;
}

bool ZoombiniShelterTown::debugDoBuiltinDebugCommand(int argc, const char **argv, Common::String &output) {
	if (argc != 3) {
		output = "Usage: page builtin_debug <Town action>\n";
		output += debugGetBuiltinDebugCommandHelp();
		return true;
	}
	const BuiltinDebugAction action = parseBuiltinDebugAction(argv[2]);
	if (action == BuiltinDebugAction::kInvalid) {
		output = Common::String::format("Unknown Town built-in debug action '%s'.\n", argv[2]);
		output += debugGetBuiltinDebugCommandHelp();
		return true;
	}
	return runBuiltinDebugAction(action, output);
}

ZoombiniShelterTown::BuiltinDebugAction ZoombiniShelterTown::parseBuiltinDebugAction(const Common::String &action) {
	if (action.equalsIgnoreCase(kBuiltinDebugActionCheatText))
		return BuiltinDebugAction::kCheatText;
	if (action.equalsIgnoreCase(kBuiltinDebugActionFireworksLast))
		return BuiltinDebugAction::kFireworksLast;
	if (action.equalsIgnoreCase(kBuiltinDebugActionFireworksNext))
		return BuiltinDebugAction::kFireworksNext;
	if (action.equalsIgnoreCase(kBuiltinDebugActionMemorialPrevious))
		return BuiltinDebugAction::kMemorialPrevious;
	if (action.equalsIgnoreCase(kBuiltinDebugActionMemorialNext))
		return BuiltinDebugAction::kMemorialNext;
	if (action.equalsIgnoreCase(kBuiltinDebugActionMemorialFill))
		return BuiltinDebugAction::kMemorialFill;
	if (action.equalsIgnoreCase(kBuiltinDebugActionMemorialClear))
		return BuiltinDebugAction::kMemorialClear;
	if (action.equalsIgnoreCase(kBuiltinDebugActionThresholdUp))
		return BuiltinDebugAction::kThresholdUp;
	if (action.equalsIgnoreCase(kBuiltinDebugActionThresholdDown))
		return BuiltinDebugAction::kThresholdDown;
	return BuiltinDebugAction::kInvalid;
}

void ZoombiniShelterTown::showBuiltinCheatTextSelection(Common::String &output) {
	Common::String text;
	if (_builtinCheatTextIndex == 0)
		text = "Cheat Text OFF";
	else
		text = Common::String::format("Cheat Text: %d", _builtinCheatTextIndex);

	output = text;
	output += "\n";
	showBuiltinDebugText(text);
}

bool ZoombiniShelterTown::runBuiltinDebugAction(BuiltinDebugAction action, Common::String &output) {
	if (action == BuiltinDebugAction::kCheatText) {
		showBuiltinCheatTextSelection(output);
		return false;
	}
	if (action == BuiltinDebugAction::kFireworksLast) {
		if (_developAnimTimer != 0) {
			_developAnimTimer = 0;
			output = "Town fireworks stopped.\n";
		} else {
			_developAnimTimer = _builtinFireworksCount;
			output = Common::String::format("Fireworks launching: %d\n", _builtinFireworksCount);
			showBuiltinDebugText(Common::String::format("Fireworks launching: %d", _builtinFireworksCount), true);
		}
		return false;
	}
	if (action == BuiltinDebugAction::kFireworksNext) {
		if (_developAnimTimer != 0) {
			_developAnimTimer = 0;
			output = "Town fireworks stopped.\n";
		} else {
			_builtinFireworksCount += 5;
			if (100 < _builtinFireworksCount)
				_builtinFireworksCount = 5;
			_developAnimTimer = _builtinFireworksCount;
			output = Common::String::format("Fireworks launching: %d\n", _builtinFireworksCount);
			showBuiltinDebugText(Common::String::format("Fireworks launching: %d", _builtinFireworksCount), true);
		}
		return false;
	}
	if (action == BuiltinDebugAction::kMemorialPrevious || action == BuiltinDebugAction::kMemorialNext) {
		if (_vm->_state->_f._memorialRoutes[15] != 0) {
			if (action == BuiltinDebugAction::kMemorialNext)
				_builtinCheatTextIndex += 1;
			else
				_builtinCheatTextIndex -= 1;
			_builtinCheatTextIndex = CLIP<int16>(_builtinCheatTextIndex, 0, 16);
			if (_memorialCardFeature)
				_memorialCardFeature->requestVisualRematerialization();
		}
		showBuiltinCheatTextSelection(output);
		return false;
	}
	if (action == BuiltinDebugAction::kMemorialFill) {
		ZmbStateFile &stateFile = _vm->_state->_f;
		int16 emptySlot = -1;
		for (int16 slotIdx = 0; slotIdx < 16; slotIdx++) {
			if (stateFile._memorialRoutes[slotIdx] == 0) {
				emptySlot = slotIdx;
				break;
			}
		}
		if (emptySlot < 0) {
			output = "All Town memorial slots are already occupied.\n";
			return false;
		}

		TimeDate date;
		g_system->getTimeAndDate(date);
		stateFile._memorialYears[emptySlot] = static_cast<uint16>(date.tm_year + 1900);
		stateFile._memorialMonths[emptySlot] = static_cast<byte>(date.tm_mon + 1);
		stateFile._memorialDays[emptySlot] = static_cast<byte>(date.tm_mday);
		stateFile._memorialRoutes[emptySlot] = static_cast<byte>(((_builtinMemorialRouteCounter / 4) & 3) + 1);
		stateFile._memorialLevels[emptySlot] = static_cast<byte>((_builtinMemorialRouteCounter & 3) + 1);
		_builtinMemorialRouteCounter += 1;
		advanceLayerFrameState(stateFile._townScrollCol);
		_vm->_state->markDebugStateMutation();
		_vm->_state->markSaveBeforeQuitPending();
		output = Common::String::format("Filled Town memorial slot %d.\n", emptySlot);
		return false;
	}
	if (action == BuiltinDebugAction::kMemorialClear) {
		ZmbStateFile &stateFile = _vm->_state->_f;
		for (uint16 slotIdx = 0; slotIdx < 16; slotIdx++)
			stateFile._memorialRoutes[slotIdx] = 0;
		_builtinMemorialRouteCounter = 0;
		_clockTowerHandsEnabled = false;
		advanceLayerFrameState(stateFile._townScrollCol);
		_vm->_state->markDebugStateMutation();
		_vm->_state->markSaveBeforeQuitPending();
		output = "Cleared all Town memorial slots.\n";
		return false;
	}
	if (action == BuiltinDebugAction::kThresholdUp || action == BuiltinDebugAction::kThresholdDown) {
		_townPopDensity += action == BuiltinDebugAction::kThresholdUp ? 5 : -5;
		_townPopDensity = CLIP<int16>(_townPopDensity, 25, 81);
		_debugPopulationPreviewActive = true;
		refreshPopulationOverlay();
		output = Common::String::format("Town building threshold %d.\n", _townPopDensity);
		return false;
	}

	return true;
}

ZmbEventHandleResult ZoombiniShelterTown::onDebugKeyDown(const Common::KeyState &kbd) {
	BuiltinDebugAction action = BuiltinDebugAction::kInvalid;
	if (kbd.keycode == Common::KEYCODE_UP)
		action = BuiltinDebugAction::kThresholdUp;
	else if (kbd.keycode == Common::KEYCODE_DOWN)
		action = BuiltinDebugAction::kThresholdDown;
	else if (kbd.hasFlags(0) || kbd.hasFlags(Common::KBD_SHIFT)) {
		if (kbd.keycode == Common::KEYCODE_SPACE || kbd.ascii == ' ') {
			action = BuiltinDebugAction::kCheatText;
		} else {
			switch (kbd.ascii) {
			case 'f':
				action = BuiltinDebugAction::kFireworksLast;
				break;
			case 'F':
				action = BuiltinDebugAction::kFireworksNext;
				break;
			case 'x':
				action = BuiltinDebugAction::kMemorialPrevious;
				break;
			case 'z':
				action = BuiltinDebugAction::kMemorialNext;
				break;
			case '.':
				action = BuiltinDebugAction::kMemorialFill;
				break;
			case '0':
				action = BuiltinDebugAction::kMemorialClear;
				break;
			default:
				return ZmbEventHandleResult::kPassthrough;
			}
		}
	} else {
		return ZmbEventHandleResult::kPassthrough;
	}

	Common::String output;
	runBuiltinDebugAction(action, output);
	const bool consumesKey = action == BuiltinDebugAction::kMemorialFill ||
							 action == BuiltinDebugAction::kMemorialClear ||
							 action == BuiltinDebugAction::kThresholdUp ||
							 action == BuiltinDebugAction::kThresholdDown;
	return consumesKey ? ZmbEventHandleResult::kConsumed : ZmbEventHandleResult::kPassthrough;
}

bool ZoombiniShelterTown::debugDoPageCommand(int argc, const char **argv, Common::String &output) {
	if (argc < 2) {
		output = "Usage: page <devel|fireworks> ...\n";
		return true;
	}

	if (scumm_stricmp(argv[1], "devel") == 0) {
		if (argc != 3 || scumm_stricmp(argv[2], "--help") == 0 || scumm_stricmp(argv[2], "-h") == 0) {
			output = "Temporarily preview Town building development without changing the save.\n";
			output += "Usage: page devel <population|reset>\n";
			output += "  population: 0-625 stored Town Zoombinis\n\n";
			output += "Use page fireworks to control fireworks.\n";
			output += "Options:\n";
			output += "  -h, --help  Show this help text and exit.\n";
			return true;
		}

		if (scumm_stricmp(argv[2], "reset") == 0) {
			debugClearPopulationPreview();
			output = "Town development preview cleared; restored the saved population background.\n";
			return false;
		}

		int32 population;
		if (!ZmbResource::parseInt(argv[2], population) || population < 0 || 625 < population) {
			output = Common::String::format("Invalid population '%s'. Must be 0-625 or reset.\n", argv[2]);
			return true;
		}

		debugSetPopulationPreview(static_cast<int16>(population));
		output = Common::String::format("Town development preview set to %d Zoombinis; save data is unchanged.\n", population);
		return false;
	}

	if (scumm_stricmp(argv[1], "fireworks") == 0) {
		if (argc != 3 || scumm_stricmp(argv[2], "--help") == 0 || scumm_stricmp(argv[2], "-h") == 0) {
			output = "Temporarily test Town celebration walkers without changing the save.\n";
			output += "Usage: page fireworks <count|cycle|stop>\n";
			output += "  count: 1-50 pending celebration walkers\n";
			output += "  cycle: accelerated full-Town test: 20 entry walkers, then 50 refill walkers\n";
			output += "  stop: cancel queued test walkers; currently visible walkers finish normally\n\n";
			output += "Original Town fireworks schedule:\n";
			output += "  What the numbers mean:\n";
			output += "    Fireworks are celebration walkers.\n";
			output += "    Queue N = N walkers will play; no more than 3 appear at once.\n";
			output += "    N is not a saved Town development level.\n";
			output += "  When entering Town:\n";
			output += "    First 0-16 Zoombinis, while development level is 0: queue 10.\n";
			output += "    First reach of 100, 200, 300, or 400: queue 20.\n";
			output += "    First reach of 500: queue 25.\n";
			output += "    Reaching all 625 Zoombinis: queue 20.\n";
			output += "    A full 16-Zoombini arriving pack adds 6 to the entry queue above.\n";
			output += "  Once Town is full:\n";
			output += "    Each ambient sound end, with no dialog, queues 50.\n";
			output += "  Visibility rule:\n";
			output += "    Queued walkers wait until no dialog or memorial card is open.\n";
			output += "\nOptions:\n";
			output += "  -h, --help  Show this help text and exit.\n";
			return true;
		}

		if (scumm_stricmp(argv[2], "cycle") == 0) {
			debugStartFireworksCycle();
			output = "Town fireworks cycle started: 20 entry walkers, then 50 refill walkers.\n";
			return false;
		}

		if (scumm_stricmp(argv[2], "stop") == 0) {
			debugStopFireworksCycle();
			output = "Town fireworks test queue cancelled; visible walkers remain until their scripts finish.\n";
			return false;
		}

		int32 count;
		if (!ZmbResource::parseInt(argv[2], count) || count < 1 || 50 < count) {
			output = Common::String::format("Invalid fireworks count '%s'. Must be 1-50, cycle, or stop.\n", argv[2]);
			return true;
		}

		debugSetPendingFireworks(static_cast<int16>(count));
		output = Common::String::format("Town fireworks test queue set to %d walkers; save data is unchanged.\n", count);
		return false;
	}

	output = Common::String::format("Unknown Town page command '%s'.\nUsage: page <devel|fireworks> ...\n", argv[1]);
	return true;
}

void ZoombiniShelterTown::open() {
	openArchive(ZMB_MHK_TOWN);
}

void ZoombiniShelterTown::setBackgroundMusic() {
	// Town does not use the standard @ref ZoombiniShelterTown::setBackgroundMusic() path.
	// @ref ZoombiniShelterTown::onPostRenderFrame() handles music through the ambient-sound cycle.
}

void ZoombiniShelterTown::setBackgroundBitmap() {
	_vm->_gfx->setPalette(kResBackground1200);
	_vm->_gfx->drawBackground(kResBackground1200);
}

void ZoombiniShelterTown::initStates() {
	ZmbStateFile &f = _vm->_state->_f;
	updateClockHandTime();

	// Count only occupied active-pack entries, not all loaded features.
	// Using the total feature count over-credits storage and triggers the +6 fireworks bonus
	// on partial packs (which reserves for fully-occupied 16-zoombini arrivals).
	_activePackCount = 0;
	for (int16 i = 0; i < f._zmbPackActive.getPackZmbCount() &&
					  i < f._zmbPackActive.getEntryCapacity();
		 i++) {
		if (f._zmbPackActive.getEntry(i).getIsOccupied())
			_activePackCount += 1;
	}
	f._zmbStoredTownCount += _activePackCount;
	if (625 <= f._zmbStoredTownCount)
		_allZoombinisInTown = true;

	// Transfer active pack Zoombini trait/name data into stored chunk
	transferActivePackToTownStorage();

	f._zmbPackActive.clearEntries();

	// Find the first empty slot in town storage (searching from beginning)
	_firstEmptyStorageSlot = -1;
	for (int16 i = 0; _firstEmptyStorageSlot < 0 && i < 625; i++) {
		if (f._storedChunkTown.getEntry(i).getTraits()._hair == ZmbTrait::TRAIT_NONE &&
			f._storedChunkTown.getEntry(i).getTraits()._eyes == ZmbTrait::TRAIT_NONE &&
			f._storedChunkTown.getEntry(i).getTraits()._nose == ZmbTrait::TRAIT_NONE &&
			f._storedChunkTown.getEntry(i).getTraits()._feet == ZmbTrait::TRAIT_NONE) {
			_firstEmptyStorageSlot = i;
		}
	}
	if (_firstEmptyStorageSlot < 0)
		_firstEmptyStorageSlot = 0;

	_townPopDensity = calculatePopulationDensity(f._zmbStoredTownCount);
}

void ZoombiniShelterTown::loadFeatures() {
	ZmbStateFile &f = _vm->_state->_f;

	// Preload images
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape2000_Cursors));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape1100));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape1000_Overlay));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape4000_Inhabitant));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape6000_ClockHands));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape8000_SubFeature));

	// Load paired X/Y REGS shape-registration offsets.
	// Both are the standard paired-axis form: one big-endian int16 per entry,
	// reserved entry 0, then one entry per sub-image of the partner tBMP.
	// REGS 2000/2001 register the three arrow/magnifier cursor shapes in tBMP 2000.
	loadShapeOffsetRegs(ZmbResource::kPage, kResRegs2000);
	// REGS 6000/6001 hold per-shape clock-hand pivot offsets for tBMP 6000.
	loadShapeOffsetRegs(ZmbResource::kPage, kResRegs6000_ClockHands);

	// Configure Town controls before their original page-owned runner is registered.
	setMapButton(_mapButtonRect, kShape1100_ExitGateLeftNormal_05, kShape1100_ExitGateLeftPressed_06);
	setHelpButton(_helpButtonRect);
	buildEmbeddedControlHotspots(kResBitmapShape1100, _controlGoMapBaseHotspots, _controlHelpBaseHotspots);

	// [*] SCRB 1000: Main overlay
	_overlayFeatures[0] = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1000_Overlay), kResScrb1000_Overlay, 0,
										  ZmbFeature::FLAG_00004000_NO_DIRTY_MERGE | ZmbFeature::FLAG_00008000_LOOP_ANIM |
											  ZmbFeature::FLAG_00020000_SKIP_RENDER | ZmbFeature::FLAG_04000000_OVERLAY);

	{ // [*] SCRB 1002: Overlay with REGS + pre-render shape callback
		ZmbFeature::EventHooks hooks;
		hooks.setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniShelterTown::overlay_preRenderShape));
		_overlayFeatures[1] = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1000_Overlay), kResScrb1002_Overlay, 0,
											  ZmbFeature::FLAG_00004000_NO_DIRTY_MERGE | ZmbFeature::FLAG_00008000_LOOP_ANIM |
												  ZmbFeature::FLAG_00020000_SKIP_RENDER | ZmbFeature::FLAG_04000000_OVERLAY |
												  ZmbFeature::FLAG_08000000_REGION_TRACK,
											  hooks);
	}

	{ // [*] SCRB 1003: Overlay with REGS + pre-render shape callback
		ZmbFeature::EventHooks hooks;
		hooks.setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniShelterTown::overlay_preRenderShape));
		_overlayFeatures[2] = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1000_Overlay), kResScrb1003_Overlay, 0,
											  ZmbFeature::FLAG_00004000_NO_DIRTY_MERGE | ZmbFeature::FLAG_00008000_LOOP_ANIM |
												  ZmbFeature::FLAG_00020000_SKIP_RENDER | ZmbFeature::FLAG_04000000_OVERLAY |
												  ZmbFeature::FLAG_08000000_REGION_TRACK,
											  hooks);
	}

	{ // [*] SCRB 1001: Memorial markers with saved-route gating
		ZmbFeature::EventHooks hooks;
		hooks.setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniShelterTown::memorialMarkers_preRenderShape));
		_overlayFeatures[3] = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1000_Overlay), kResScrb1001_Overlay, 0,
											  ZmbFeature::FLAG_00004000_NO_DIRTY_MERGE | ZmbFeature::FLAG_00008000_LOOP_ANIM |
												  ZmbFeature::FLAG_00020000_SKIP_RENDER | ZmbFeature::FLAG_04000000_OVERLAY |
												  ZmbFeature::FLAG_08000000_REGION_TRACK,
											  hooks);
	}

	// [*] SCRS 4999: Reject Zoombini snoid
	// Group 0 selects state 9.
	registerScrsGroup(kResScrb4999_Reject, 1);

	// [*] SCRS 5000 ~ 5004: Normal Zoombini snoids (5 variants)
	// Despite the resource label, registers this as group 1, which selects state 8 and the general body tables.
	// Town swaps those tables to the small variants before creating its lower walkers.
	registerScrsGroup(kResScrb5000_Normal, 5);

	// [*] SCRB 6000: Clock-tower hands sub-feature (child of SCRB 1000)
	loadSubFeature(_overlayFeatures[0], ZmbResource(ZmbResource::kPage, kResBitmapShape6000_ClockHands), kResScrb6000_ClockHands);

	{ // [*] SCRB 8000 ~ 8043: Town building sub-features (44 of them, chained from SCRB 1002)
		ZmbFeature *parent = _overlayFeatures[1];
		for (uint16 i = 0; i < 44; i++) {
			parent = loadSubFeature(parent, ZmbResource(ZmbResource::kPage, kResBitmapShape8000_SubFeature), kResScrb8000_SubFeatureBase + i);
		}
	}

	// Town background inhabitants. These are generic SCRB runners, not stored
	// Zoombini trait composites.
	// They retain generic SCRB callbacks and use the final GRIDWALKER|LOOP_ANIM|POS_DELTA runtime category set.
	{
		int16 populationAfterFirstTwenty = f._zmbStoredTownCount - 20;
		populationAfterFirstTwenty = CLIP<int16>(populationAfterFirstTwenty, 0, 605);
		_inhabitantCount = CLIP<int16>(populationAfterFirstTwenty / 37, 0, 16);

		// Select from the 16 authored position/SCRB pairs without replacement.
		uint32 positionPoolState = 0;
		for (uint16 inhabitantIdx = 0; inhabitantIdx < _inhabitantCount; inhabitantIdx++) {
			const uint16 posIdx = _vm->_rnd->getNonRepeatRandom(16, positionPoolState);

			const Common::Point anchor = kInhabitantPositions[posIdx];
			ZmbFeature *inhabitant = loadScrbFeature(
				ZmbResource(ZmbResource::kPage, kResBitmapShape4000_Inhabitant),
				kInhabitantScrbTable[posIdx], _vm->_rnd->getRandomNumber(4, 6), anchor,
				ZmbFeature::FLAG_00000002_TYPE_GRIDWALKER |
					ZmbFeature::FLAG_00008000_LOOP_ANIM |
					ZmbFeature::FLAG_00800000_POS_DELTA);
			if (inhabitant) {
				// Set both position anchors after loading the SCRB so the initial delta is zero.
				inhabitant->setPointLoc(anchor);
				inhabitant->setPointRef(anchor);
			}
		}
	}

	_memorialHotspotCount = 0;
	for (int16 markerIdx = 0; markerIdx < 16; markerIdx++) {
		_memorialHotspots[markerIdx] = Common::Rect();
		_memorialSlotMapping[markerIdx] = -1;
	}

	// Clamp the saved scroll column to [0, 5], then shift inhabitant runner
	// X positions before the stored Town walkers are registered.
	{
		uint16 scrollCol = f._townScrollCol;
		if (6 <= scrollCol) {
			scrollCol = 0;
			f._townScrollCol = 0;
		}
		for (uint16 slotIdx = scrollCol; 0 < slotIdx; slotIdx--)
			shiftRunnersForScroll(1);
	}

	// Create up to 20 walking Snoids from occupied entries in the stored Town chunk.
	// Iterate backward from the last occupied entry and assign the Town
	// GRIDWALKER runtime category to each Snoid runner.
	// Random position x=[-320, 1599], y=[410, 475].
	{
		const int16 storedCount = f._zmbStoredTownCount;
		_walkingZmbCount = 0;

		int16 walkIdx = _firstEmptyStorageSlot - 1;
		if (0 <= walkIdx) {
			while (0 <= walkIdx && _walkingZmbCount < 20 &&
				   static_cast<int16>(_walkingZmbCount) < storedCount) {
				ZmbStateStoredEntry &entry = f._storedChunkTown.getEntry(walkIdx);

				// Use snoid IDs in the 20000+ range to avoid collision with inhabitants (0-15)
				uint16 snoidId = 20000 + _walkingZmbCount;

				const int16 walkX = _vm->_rnd->getRandomNumber(-320, 1599);
				const int16 walkY = _vm->_rnd->getRandomNumber(410, 475);
				const Common::Point walkPos(walkX, walkY);

				ZmbSnoid *snoid = loadSnoidFromPack(snoidId, walkPos, ZmbFeature::FLAG_00000001_TYPE_SNOID);
				if (snoid) {
					snoid->_trait = entry.getTraits();
					snoid->_name = entry.getName(_vm);
					// Keep the Snoid object and callbacks, but change its mutable
					// runtime category for Town sorting, scrolling, and input.
					snoid->removeFlag(ZmbFeature::FLAG_00000001_TYPE_SNOID);
					snoid->addFlag(ZmbFeature::FLAG_00000002_TYPE_GRIDWALKER);
					snoid->setResource(ZmbResource(ZmbResource::kSystem, kSysResShapeBitmap3200_SnoidSmall));
					snoid->setupSmallIdleHotspots();
					_walkingZmbSnoidIds[_walkingZmbCount] = snoidId;
					_walkingZmbCount += 1;
				}

				walkIdx -= 1;
			}
		}
	}

	{ // [*] One callback-only runner for Map and Help.
		ZmbFeature::EventHooks hooks;
		hooks.setPreRenderFunc(static_cast<ZmbFeature::OnPreRenderFunc>(&ZoombiniShelterTown::controls_preRender));
		hooks.setRenderFunc(static_cast<ZmbFeature::OnRenderFunc>(&ZoombiniShelterTown::controls_render));
		hooks.setPostRenderFunc(static_cast<ZmbFeature::OnPostRenderFunc>(&ZoombiniShelterTown::controls_postRender));
		hooks.setLButtonDownFunc(static_cast<ZmbFeature::OnLButtonDownFunc>(&ZoombiniShelterTown::controls_lButtonDown));

		ZmbFeature *feature = loadScrbFeature(ZmbResource(ZmbResource::kPage, 0), 0, 0, ZmbFeature::FLAG_00001000_TOPMOST, hooks);
		Common::Rect clickRect = _mapButtonRect;
		if (_vm->supportsOnScreenHelp())
			clickRect.extend(_helpButtonRect);
		feature->setClickRect(clickRect);
		bindEmbeddedControlFeature(feature);
	}

	// Select the saved parallax frame after the control runner is registered
	// and immediately before the standalone clock runner is created.
	advanceLayerFrameState(f._townScrollCol);

	// The SCRB 6000 clock hands register with the Snoid storage selector and
	// a custom pre-render shape callback.
	// Its mutable runtime category changes to GRIDWALKER after registration.
	// LOOP_ANIM selects the render bucket, while GRIDWALKER keeps the hands in
	// Town-wide scrolling and modal render-state updates.
	{
		ZmbFeature::EventHooks hooks;
		hooks.setPreRenderFunc(static_cast<ZmbFeature::OnPreRenderFunc>(&ZoombiniShelterTown::clockHands_preRender));
		hooks.setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniShelterTown::clockHands_preRenderShape));
		_clockHandsFeature = loadScrbFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape6000_ClockHands), kResScrb6000_ClockHands, kTownClockNormalFrameInterval,
			ZmbFeature::FLAG_00000001_TYPE_SNOID | ZmbFeature::FLAG_00008000_LOOP_ANIM,
			hooks);
		if (_clockHandsFeature) {
			Common::HashMap<int16, ZmbShapeOffsetRegs *>::iterator clockRegsIt = _shapeOffsetRegsMap.find(kResRegs6000_ClockHands);
			if (clockRegsIt != _shapeOffsetRegsMap.end())
				_clockHandsFeature->setShapeOffsetRegs(clockRegsIt->_value);

			// Preserve the generic feature and its callback while changing only
			// the mutable runtime category.
			// The final category keeps the hands in Town scroll and modal-render groups,
			// while LOOP_ANIM continues to select its render bucket.
			_clockHandsFeature->removeFlag(ZmbFeature::FLAG_00000001_TYPE_SNOID);
			_clockHandsFeature->addFlag(ZmbFeature::FLAG_00000002_TYPE_GRIDWALKER);
		}
	}

	// Materialize the saved panorama, walkers, controls, and clock before
	// selecting entry audio or deriving the celebration queue.
	renderFeatures();

	// Determine the entry sound from the transient SFX-selection group.
	ZmbSfxGroupFlags sfxGroupFlags = ZmbSfxGroupFlags::kRandom_00;
	if (_vm->_state->_lastPageBeforeContainer != 0) {
		_vm->_state->_lastPageBeforeContainer = 0;
		sfxGroupFlags = _vm->_state->getSfxGroupFlagsFromPageType(ZoombiniPageType::kTown);
		if (sfxGroupFlags == ZmbSfxGroupFlags::kHardFirstTrigger_02 && f._zmbStoredTownCount <= 16) {
			sfxGroupFlags = ZmbSfxGroupFlags::kEasy_01;
			f._pageFlagTown.clearHardGroupTriggers();
		}
	}

	if (sfxGroupFlags == ZmbSfxGroupFlags::kEasy_01) {
		switch (f._pageFlagTown.getRaw()) {
		case 1:
			_entrySoundRes = ZmbResource(ZmbResource::kSystem, kSysResSound20086_Voice);
			break;
		case 2:
			_entrySoundRes = ZmbResource(ZmbResource::kSystem, kSysResSound20087_Voice);
			break;
		case 3:
			_entrySoundRes = ZmbResource(ZmbResource::kSystem, kSysResSound20088_Voice);
			break;
		default: {
			int16 r = _vm->_rnd->getRandomNumber(1, 3);
			if (r == 1)
				_entrySoundRes = ZmbResource(ZmbResource::kSystem, kSysResSound20086_Voice);
			else if (r == 2)
				_entrySoundRes = ZmbResource(ZmbResource::kSystem, kSysResSound20087_Voice);
			else
				_entrySoundRes = ZmbResource(ZmbResource::kSystem, kSysResSound20088_Voice);
			break;
		}
		}
	} else if (sfxGroupFlags == ZmbSfxGroupFlags::kHardFirstTrigger_02) {
		int16 r = _vm->_rnd->getRandomNumber(1, 2);
		if (r == 1)
			_entrySoundRes = ZmbResource(ZmbResource::kSystem, kSysResSound20087_Voice);
		else
			_entrySoundRes = ZmbResource(ZmbResource::kSystem, kSysResSound20088_Voice);
	} else if (sfxGroupFlags == ZmbSfxGroupFlags::kHardSecondTrigger_12) {
		// A second hard-group arrival leaves the pending entry sound empty.
	} else if (sfxGroupFlags == ZmbSfxGroupFlags::kPractice_05) {
		_entrySoundRes = ZmbResource(ZmbResource::kSystem, kSysResSound20086_Voice);
	} else {
		// Default: compute route-based sound ID from maze page flag.
		// @ref ZoombiniShelterTown::computeRouteMusicId() maps the page flag to SND 3000-3002.
		int16 soundId = computeRouteMusicId();
		_entrySoundRes = ZmbResource(ZmbResource::kPage, soundId);
		_playEntrySoundImmediately = true;
	}

	// If all Zoombinis are in town, play victory BGM (3003 is in TOWN.MHK)
	if (_allZoombinisInTown) {
		_entrySoundRes = ZmbResource(ZmbResource::kPage, kResSound3003_BGM);
		_playEntrySoundImmediately = true;
	}

	// Town develop level checks
	_developAnimTimer = 0;
	if (_allZoombinisInTown) {
		_developAnimTimer = 20;
	} else {
		if (f._zmbStoredTownCount < 17 && !f._townDevelopLevel) {
			f._townDevelopLevel = 1;
			_developAnimTimer = 10;
		}
		if (100 <= f._zmbStoredTownCount && f._townDevelopLevel < 2) {
			f._townDevelopLevel = 2;
			_developAnimTimer = 20;
		}
		if (200 <= f._zmbStoredTownCount && f._townDevelopLevel < 3) {
			f._townDevelopLevel = 3;
			_developAnimTimer = 20;
		}
		if (300 <= f._zmbStoredTownCount && f._townDevelopLevel < 4) {
			f._townDevelopLevel = 4;
			_developAnimTimer = 20;
		}
		if (400 <= f._zmbStoredTownCount && f._townDevelopLevel < 5) {
			f._townDevelopLevel = 5;
			_developAnimTimer = 20;
		}
		if (500 <= f._zmbStoredTownCount && f._townDevelopLevel < 6) {
			f._townDevelopLevel = 6;
			_developAnimTimer = 25;
		}
		if (_activePackCount == 16)
			_developAnimTimer += 6;
	}

	// Play the entry sound when requested and retain its exact resource for ambient cycling.
	_ambientSoundResource = _entrySoundRes;
	_ambientSoundFirstPlay = _playEntrySoundImmediately;
	_ambientSoundDone = false;
	_ambientSoundLastTime = 0;
	_ambientSoundDelay = 0;
	_ambientVoicePoolState = 0;
	_nPendingWalkerRemovals = 0;
	for (int i = 0; i < 3; i++)
		_celebWalkerFeatures[i] = nullptr;

	if (_entrySoundRes.hasId() && _playEntrySoundImmediately && !_developAnimTimer) {
		queueTownSound(_entrySoundRes);
		_ambientSoundDone = true;
	}

	// Idle animation state init
	_idleAnimBudget = 0;
	_idleAnimLastFrame = 0;
	_idleAnimInterval = 120; // 120
	_idleAnimPoolState = 0;
}

// ---------------------------------------------------------------------------
// Update Town after each render, matching the original page controller order.
// ---------------------------------------------------------------------------
void ZoombiniShelterTown::onPostRenderFrame() {

	// --- 1. Cleanup finished celebration walkers ---
	// Scan celebration walker slots, free completed ones.
	cleanupFinishedWalkers();
	advanceDebugFireworksCycle();

	// --- 2. Spawn celebration walkers ---
	spawnCelebrationWalker();

	// --- 3. Ambient sound cycling ---
	// Timer-based alternation between music (3000-3002) and voice (20089-20093).
	if (_ambientSoundResource.hasId() && !_vm->_sound->isSoundPlaying(_ambientSoundResource) &&
		!_vm->hasDialogOpened()) {
		if (_ambientSoundDone) {
			// Sound just finished: start random delay timer
			_ambientSoundLastTime = getCurrentFrameCounter();
			_ambientSoundDelay = static_cast<uint32>(_vm->_rnd->getRandomNumber(150, 300));
			_ambientSoundDone = false;

			// After an ambient sound completes, full Town refills the pending celebration count with fifty walkers.
			if (_allZoombinisInTown)
				_developAnimTimer = 50;
		}

		if (_ambientSoundDelay < getCurrentFrameCounter() - _ambientSoundLastTime) {
			_ambientSoundLastTime = getCurrentFrameCounter();

			ZmbResource nextSoundResource;
			if (_ambientSoundFirstPlay && !_allZoombinisInTown) {
				// On first play, choose a random voice from SND 20089-20093.
				// Clear the first-play flag before applying the retry filter.
				_ambientSoundFirstPlay = false;
				bool retry;
				do {
					retry = false;
					uint16 poolIdx = _vm->_rnd->getNonRepeatRandom(5, _ambientVoicePoolState);
					const int16 nextSoundId = kAmbientVoicePool[poolIdx];
					// Retry SND 20093 when the Town population exceeds 600.
					if (nextSoundId == kSysResSound20093_Ambient &&
						600 < _vm->_state->_f._zmbStoredTownCount)
						retry = true;
					nextSoundResource = ZmbResource(ZmbResource::kSystem, nextSoundId);
					_ambientSoundResource = nextSoundResource;
				} while (retry);
			} else {
				// Cycle SND 3000, 3001, and 3002, or switch from a voice line to route-based music.
				if (_ambientSoundResource._archiveKind == ZmbResource::kSystem) {
					// Switch from a voice line to route-based music.
					_ambientSoundResource = ZmbResource(ZmbResource::kPage, computeRouteMusicId());
				} else {
					// Advance to the next music track and wrap SND 3003 to SND 3000.
					_ambientSoundResource._id += 1;
					if (kResSound3003_BGM <= _ambientSoundResource._id)
						_ambientSoundResource._id = kResSound3000_BGM;
				}
				_ambientSoundFirstPlay = true;
				nextSoundResource = _ambientSoundResource;
			}

			// Queue the selected sound so it remains in Town's script-sound priority domain.
			queueTownSound(nextSoundResource);
			_ambientSoundDone = true;
		}
	}

	// --- 4. Idle animation scheduling ---
	// Budget-based SCRS playback on walking Zoombinis.
	if (_walkingZmbCount == 0 || _vm->hasDialogOpened() || _memorialCardActive)
		return;

	// Recalculate budget when exhausted.
	// Budget thresholds depend on the stored Town population.
	if (_idleAnimBudget <= 0) {
		const int16 storedCount = _vm->_state->_f._zmbStoredTownCount;
		if (storedCount == 0)
			return;
		else if (storedCount == 625)
			_idleAnimBudget = 8;
		else if (storedCount <= 156)
			_idleAnimBudget = 1;
		else if (storedCount <= 312)
			_idleAnimBudget = 4;
		else // storedCount <= 624
			_idleAnimBudget = 6;
		return;
	}

	if (getCurrentFrameCounter() - _idleAnimLastFrame <= _idleAnimInterval)
		return;
	_idleAnimLastFrame = getCurrentFrameCounter();

	// Try up to 16 times to find a valid idle walker on-screen
	bool triggered = false;
	int16 attempts = 0;
	do {
		attempts += 1;
		uint16 poolIdx = _vm->_rnd->getNonRepeatRandom(_walkingZmbCount, _idleAnimPoolState);
		ZmbSnoid *snoid = getIdleSnoid(_walkingZmbSnoidIds[poolIdx]);
		if (snoid && snoid->hasFlag(ZmbFeature::FLAG_00000002_TYPE_GRIDWALKER) && 20 < snoid->getPointLoc().x && snoid->getPointLoc().x < 620) {
			// Play celebration SCRS: kResScrs4999_NormalBase + feet (SCRS 5000-5004).
			int16 scrsId = static_cast<int16>(snoid->_trait._feet + kResScrs4999_NormalBase);
			if (startSnoidScrs(snoid, ZmbResource(ZmbResource::kPage, scrsId))) {
				_idleAnimBudget -= 1;
				triggered = true;
			}
		}
	} while (!triggered && attempts < 16);
}

void ZoombiniShelterTown::transferActivePackToTownStorage() {
	ZmbStateFile &f = _vm->_state->_f;
	ZmbStateStoredChunk &town = f._storedChunkTown;
	int16 activeIdx = 0;

	// Copy occupied active-pack entries into the first empty storage slots.
	for (int16 storageIdx = 0; activeIdx < _activePackCount && storageIdx < 625; storageIdx++) {
		if (625 <= town.getStoredCount())
			break;

		ZmbStateStoredEntry &storedEntry = town.getEntry(storageIdx);
		if (storedEntry.getTraits()._hair == ZmbTrait::TRAIT_NONE &&
			storedEntry.getTraits()._eyes == ZmbTrait::TRAIT_NONE &&
			storedEntry.getTraits()._nose == ZmbTrait::TRAIT_NONE &&
			storedEntry.getTraits()._feet == ZmbTrait::TRAIT_NONE) {
			const ZmbStateActiveEntry &activeEntry = f._zmbPackActive.getEntry(activeIdx);
			activeIdx += 1;
			storedEntry.setTraits(activeEntry.getTraits());
			storedEntry.setName(_vm, activeEntry.getU32Name(_vm));
			town.increaseStoredCount(1);
		}
	}
}

int16 ZoombiniShelterTown::calculatePopulationDensity(int16 population) {
	// Keep the multiplication in int32: 56 * maximum population 625 is 35000, which exceeds INT16_MAX.
	const int32 density = 56 * static_cast<int32>(population) / 625 + 1;
	return static_cast<int16>(MIN<int32>(density, 56) + 24);
}

void ZoombiniShelterTown::debugSetPopulationPreview(int16 population) {
	_debugPopulationPreviewActive = true;
	_townPopDensity = calculatePopulationDensity(population);
	refreshPopulationOverlay();
}

void ZoombiniShelterTown::debugClearPopulationPreview() {
	if (!_debugPopulationPreviewActive)
		return;

	_debugPopulationPreviewActive = false;
	_townPopDensity = calculatePopulationDensity(_vm->_state->_f._zmbStoredTownCount);
	refreshPopulationOverlay();
}

void ZoombiniShelterTown::debugSetPendingFireworks(int16 count) {
	_debugFireworksCyclePendingRefill = false;
	_developAnimTimer = count;
}

void ZoombiniShelterTown::debugStartFireworksCycle() {
	_debugFireworksCyclePendingRefill = true;
	_developAnimTimer = 20;
}

void ZoombiniShelterTown::debugStopFireworksCycle() {
	_debugFireworksCyclePendingRefill = false;
	_developAnimTimer = 0;
}

void ZoombiniShelterTown::refreshPopulationOverlay() {
	for (uint16 overlayIdx = 1; overlayIdx <= 2; overlayIdx++) {
		ZmbFeature *feature = _overlayFeatures[overlayIdx];
		if (!feature)
			continue;

		const Common::Rect oldCoverage = feature->getZSortRect();
		feature->activateRender();
		prepareFeatureVisualCoverage(feature);
		feature->deactivateRender();

		addExternalDirtyRect(oldCoverage);
		addExternalDirtyRect(feature->getZSortRect());
		feature->setNeedsRedraw(true);
	}
}

bool ZoombiniShelterTown::hasActiveCelebrationWalkers() const {
	for (uint16 walkerIdx = 0; walkerIdx < ARRAYSIZE(_celebWalkerFeatures); walkerIdx++) {
		if (_celebWalkerFeatures[walkerIdx])
			return true;
	}

	return false;
}

void ZoombiniShelterTown::advanceDebugFireworksCycle() {
	if (!_debugFireworksCyclePendingRefill || _developAnimTimer != 0 ||
		hasActiveCelebrationWalkers() || _vm->hasDialogOpened() || _memorialCardActive)
		return;

	_debugFireworksCyclePendingRefill = false;
	_developAnimTimer = 50;
}

void ZoombiniShelterTown::saveStateBeforeMapTransition() {
	// Before shared cleanup, Town clears the active pack
	// and filters out both occupied and unoccupied active-pack entries.
	ZmbStateFile &f = _vm->_state->_f;
	f._zmbPackActive.clearEntries();
	f._zmbPackActive.setSkipOccupiedEntries(true);
	f._zmbPackActive.setSkipUnoccupiedEntries(true);
}

void ZoombiniShelterTown::overlay_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)feature;
	(void)hsGroup;

	// Filters overlay building shapes by town population density threshold.
	// Remove any hotspot whose shape index exceeds the building display threshold.
	for (uint hotspotIdx = 0; hotspotIdx < hotspots.size();) {
		if (_townPopDensity < hotspots[hotspotIdx]._shapeIdx) {
			hotspots.remove_at(hotspotIdx);
		} else {
			hotspotIdx += 1;
		}
	}
}

void ZoombiniShelterTown::memorialMarkers_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)feature;
	(void)hsGroup;

	const ZmbStateFile &stateFile = _vm->_state->_f;
	_clockTowerHandsEnabled = false;
	_memorialHotspotCount = 0;
	for (int16 markerIdx = 0; markerIdx < 16; markerIdx++) {
		_memorialHotspots[markerIdx] = Common::Rect();
		_memorialSlotMapping[markerIdx] = -1;
	}

	for (uint hotspotIdx = 0; hotspotIdx < hotspots.size();) {
		const uint16 shapeIdx = hotspots[hotspotIdx]._shapeIdx;
		if (shapeIdx < 7 || 22 < shapeIdx) {
			hotspotIdx += 1;
			continue;
		}

		const int16 slotIdx = static_cast<int16>(shapeIdx) - 7;
		const uint16 markerIdx = _memorialHotspotCount;
		if (_memorialHotspotCount < 16)
			_memorialHotspotCount += 1;

		if (stateFile._memorialRoutes[slotIdx] == 0) {
			hotspots.remove_at(hotspotIdx);
			continue;
		}
		if (slotIdx == kTownClockTowerMemorialSlot)
			_clockTowerHandsEnabled = true;

		if (markerIdx < 16) {
			const int16 left = hotspots[hotspotIdx]._x + kTownMemorialMarkerOffsets[slotIdx][0] - 28;
			const int16 top = hotspots[hotspotIdx]._y + kTownMemorialMarkerOffsets[slotIdx][1] - 14;
			_memorialHotspots[markerIdx] = Common::Rect(left, top, left + 56, top + 28);
			_memorialSlotMapping[markerIdx] = slotIdx;
		}
		hotspotIdx += 1;
	}
}

bool ZoombiniShelterTown::controls_preRender(ZmbFeature *feature) {
	_controlGoMapHotspots = _controlGoMapBaseHotspots;
	_controlHelpHotspots = _controlHelpBaseHotspots;
	goMapButtons_preRenderShape(feature, nullptr, _controlGoMapHotspots);
	helpButton_preRenderShape(feature, nullptr, _controlHelpHotspots);
	return true;
}

ZmbRenderResult ZoombiniShelterTown::controls_render(ZmbFeature *feature) {
	(void)feature;

	for (uint32 i = 0; i < _controlGoMapHotspots.size(); i++) {
		const ZmbHotspot &hotspot = _controlGoMapHotspots[i];
		if (hotspot._shapeIdx <= ZmbHotspot::kShapeNone)
			continue;
		_vm->_gfx->drawShape(ZoombiniGraphics::kShapeScreen,
							 ZmbResource(ZmbResource::kPage, kResBitmapShape1100),
							 static_cast<uint16>(hotspot._shapeIdx), hotspot.getPos());
	}
	for (uint32 i = 0; i < _controlHelpHotspots.size(); i++) {
		const ZmbHotspot &hotspot = _controlHelpHotspots[i];
		if (hotspot._shapeIdx <= ZmbHotspot::kShapeNone)
			continue;
		_vm->_gfx->drawShape(ZoombiniGraphics::kShapeScreen,
							 ZmbResource(ZmbResource::kSystem, kSysResShapeBitmap0001_Dialog),
							 static_cast<uint16>(hotspot._shapeIdx), hotspot.getPos());
	}
	return ZmbRenderResult::kRendered;
}

void ZoombiniShelterTown::controls_postRender(ZmbFeature *feature) {
	goMapButtons_onPostRender(feature);
	helpButton_onPostRender(feature);
}

ZmbEventHandleResult ZoombiniShelterTown::controls_lButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	ZmbEventHandleResult result = goMapButtons_onLButtonDown(feature, absPos, relPos);
	if (result == ZmbEventHandleResult::kConsumed)
		return result;
	return helpButton_onLButtonDown(feature, absPos, relPos);
}

// ---------------------------------------------------------------------------
// Celebration walker spawning.
// Create SCRB 8000-8043 features with PLAY_ONCE animation.
// Up to 3 concurrent walkers; linked to SCRB 1001 overlay for Z-order.
// ---------------------------------------------------------------------------
void ZoombiniShelterTown::spawnCelebrationWalker() {
	// Pending count, no active dialog, and no memorial card.
	if (_developAnimTimer <= 0 || _vm->hasDialogOpened() || _memorialCardActive)
		return;

	// Select SCRB ID: 50% chance each range [8000,8021] or [8022,8043]
	int16 randVal = _vm->_rnd->getRandomNumber(0, 100);
	int16 scrbId;
	if (50 < randVal)
		scrbId = _vm->_rnd->getRandomNumber(kResScrb8000_SubFeatureBase, kResScrb8021_SubFeatureLast);
	else
		scrbId = _vm->_rnd->getRandomNumber(kResScrb8022_NormalBase, kResScrb8043_NormalLast);

	// Determine Y position by SCRB ID group (building elevation ranges)
	// Nested if-else tree
	int16 yPos;
	if (scrbId <= kResScrb8007_SubFeatureGroup0Last)
		yPos = _vm->_rnd->getRandomNumber(170, 280);
	else if (scrbId <= kResScrb8009_SubFeatureGroup1Last)
		yPos = _vm->_rnd->getRandomNumber(40, 280);
	else if (scrbId <= kResScrb8017_SubFeatureGroup2Last)
		yPos = _vm->_rnd->getRandomNumber(110, 260);
	else if (scrbId <= kResScrb8021_SubFeatureLast)
		yPos = _vm->_rnd->getRandomNumber(-10, 100);
	else if (scrbId <= kResScrb8029_NormalGroup0Last)
		yPos = _vm->_rnd->getRandomNumber(230, 310);
	else if (scrbId <= kResScrb8031_NormalGroup1Last)
		yPos = _vm->_rnd->getRandomNumber(140, 290);
	else if (scrbId <= kResScrb8039_NormalGroup2Last)
		yPos = _vm->_rnd->getRandomNumber(190, 280);
	else // 8040-8043
		yPos = _vm->_rnd->getRandomNumber(100, 200);

	// The original consumes the selector and Y draws before checking the three
	// active slots. A full slot table therefore still advances the shared RNG.
	int16 slot = -1;
	for (int16 i = 0; i < 3; i++) {
		if (_celebWalkerFeatures[i] == nullptr) {
			slot = i;
			break;
		}
	}
	if (slot < 0)
		return;

	// Create a standard SCRB runner.
	// Replace its flags with the town-entity, looping, one-shot, and position-delta flags.
	ZmbFeature *walker = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape8000_SubFeature), scrbId, 6,
		ZmbFeature::FLAG_00000002_TYPE_GRIDWALKER |
			ZmbFeature::FLAG_00008000_LOOP_ANIM |
			ZmbFeature::FLAG_00100000_PLAY_ONCE |
			ZmbFeature::FLAG_00800000_POS_DELTA);

	if (walker) {
		const int16 xPos = _vm->_rnd->getRandomNumber(100, 540);
		const Common::Point sortAnchor(xPos, yPos);
		walker->setPointLoc(sortAnchor);
		walker->setPointRef(sortAnchor);
		// The current runner-order system already places the walker correctly; no additional relink is needed.
		_celebWalkerFeatures[slot] = walker;

		if (0 < _developAnimTimer)
			_developAnimTimer -= 1;
	}
}

// ---------------------------------------------------------------------------
// Cleanup completed celebration walkers.
// Scans celebration walker slots for completed animations, frees them.
// ---------------------------------------------------------------------------
void ZoombiniShelterTown::cleanupFinishedWalkers() {
	for (int16 i = 0; i < 3; i++) {
		if (_celebWalkerFeatures[i] == nullptr)
			continue;
		if (_celebWalkerFeatures[i]->hasAnimEndCallbackFired()) {
			unloadScrbFeature(_celebWalkerFeatures[i]);
			_celebWalkerFeatures[i] = nullptr;
		}
	}
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
void ZoombiniShelterTown::queueTownSound(ZmbResource resource) {
	Audio::Mixer::SoundType soundType;
	if (resource._archiveKind == ZmbResource::kPage)
		soundType = Audio::Mixer::kMusicSoundType;
	else
		soundType = Audio::Mixer::kSpeechSoundType;
	queueScriptSoundForNextRenderPass(resource, soundType);
}

int16 ZoombiniShelterTown::computeRouteMusicId() const {
	// Cycle the Maze visit count across town music resources 3000-3002.
	const int16 mazeVisitCount = static_cast<int16>(_vm->_state->_f._pageFlagMaze.getVisitCount());
	int16 soundId = static_cast<int16>((mazeVisitCount - 1) % 3 + kResSound3000_BGM);
	return CLIP<int16>(soundId, kResSound3000_BGM, kResSound3002_BGM);
}

void ZoombiniShelterTown::updateClockHandTime() {
	// The original first sample is also gated by frame 1800 after the page-local
	// deadline is reset to zero.
	if (_currentFrameCounter <= _clockLastTimeUpdateFrame + kTownClockTimeUpdateInterval)
		return;

	_clockLastTimeUpdateFrame = _currentFrameCounter;

	// Map the hour to five-hour sectors and the minute to twelve hand steps.
	TimeDate td;
	g_system->getTimeAndDate(td);
	_clockHourStep = static_cast<byte>(td.tm_hour) / 5;
	_clockMinuteStep = static_cast<byte>(td.tm_min) % 12;
}

bool ZoombiniShelterTown::clockHands_preRender(ZmbFeature *feature) {
	if (!feature || !feature->isRenderActivated() ||
		!feature->isAnimationTimerDue(_currentFrameCounter))
		return false;

	// The clock uses a single fixed SCRB group whose shape hook owns all visual changes.
	// Keep it outside the standard multi-frame animation lifecycle and run that hook only
	// after the runner's own deadline expires.
	if (feature->isInitialScrbLoadPending()) {
		feature->setInitialScrbLoadPending(false);
		feature->setLastFrameIdx(0);
		feature->clearFirstFrameAdvanceHold();
	}

	// Schedule before materialization because the shape hook may change the interval.
	feature->scheduleNextRenderFrame(_currentFrameCounter);
	if (!feature->hasFlag(ZmbFeature::FLAG_00004000_NO_DIRTY_MERGE))
		markFeatureVisualCoverageDirty(feature, false);
	feature->setNeedsRedraw(true);
	prepareFeatureVisualCoverage(feature);
	return false;
}

void ZoombiniShelterTown::clockHands_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)hsGroup;
	if (hotspots.size() < 2) {
		error("town: malformed required clock-hands SCRB hotspot table");
		return;
	}

	// Clear both parsed hotspots to avoid rendering the untouched minute dial at (0,0).
	hotspots[0]._shapeIdx = ZmbHotspot::kShapeNone;
	hotspots[1]._shapeIdx = ZmbHotspot::kShapeNone;

	// The hands exist only after the clock-tower memorial is unlocked and visible.
	uint16 scrollCol = _vm->_state->_f._townScrollCol;
	if (!_clockTowerHandsEnabled || (scrollCol != 1 && scrollCol != 2))
		return;

	if (_clockSpinCycles < 0) {
		_clockSpinTargetHourStep = _clockHourStep;
		_clockSpinTargetMinuteStep = _clockMinuteStep;
		_clockSpinCycles = -_clockSpinCycles;
		feature->setFrameInterval(kTownClockSpinFrameInterval);
	} else if (0 < _clockSpinCycles) {
		_clockHourStep += 1;
		if (kTownClockHandLastStep < _clockHourStep) {
			_clockHourStep = 0;
			_clockMinuteStep += 1;
			if (kTownClockHandLastStep < _clockMinuteStep)
				_clockMinuteStep = 0;
		}

		if (_clockHourStep == _clockSpinTargetHourStep &&
			_clockMinuteStep == _clockSpinTargetMinuteStep) {
			_clockSpinCycles -= 1;
			if (_clockSpinCycles == 0) {
				feature->setFrameInterval(kTownClockNormalFrameInterval);
				_clockLastTimeUpdateFrame = 0;
				updateClockHandTime();
			}
		}
	} else {
		updateClockHandTime();
	}

	// Set the hotspot shapes and positions.
	// Shapes 1-12 are the hour hand and shapes 13-24 are the minute hand.
	// Both use y=218; the scroll column selects their X coordinate.
	int16 xPos = (scrollCol == 1) ? 626 : 307;

	hotspots[0]._shapeIdx = _clockHourStep + 1;
	hotspots[0]._x = xPos;
	hotspots[0]._y = 218;
	hotspots[1]._shapeIdx = _clockMinuteStep + 13;
	hotspots[1]._x = xPos;
	hotspots[1]._y = 218;
}

void ZoombiniShelterTown::clearHoverCursor() {
	_hoverCursorShapeIdx = ZmbHotspot::kShapeNone;
}

ZmbEventHandleResult ZoombiniShelterTown::onMouseMove(const Common::Point &absPos, const Common::Point &relPos) {
	uint16 cursorShapeIdx = ZmbHotspot::kShapeNone;
	if (!isDragging() && !_memorialCardActive) {
		const int16 memorialSlotHit = hitTestMemorialHotspots(absPos);
		if (!_mapButtonRect.contains(absPos) && isTownScrollViewportPoint(absPos)) {
			// State 3 = memorial, state 2 = right scroll, state 1 = left scroll.
			if (0 <= memorialSlotHit) {
				cursorShapeIdx = kShape2000_Magnifier_03;
			} else if (ZoombiniGraphics::kScreenWidth - kScrollableCursorAreaWidth < absPos.x) {
				cursorShapeIdx = kShape2000_ArrowRight_02;
			} else if (absPos.x < kScrollableCursorAreaWidth) {
				cursorShapeIdx = kShape2000_ArrowLeft_01;
			}
		}
	}

	if (cursorShapeIdx != _hoverCursorShapeIdx) {
		if (cursorShapeIdx == ZmbHotspot::kShapeNone) {
			_vm->_cursor->setDefaultCursor();
		} else {
			Common::HashMap<int16, ZmbShapeOffsetRegs *>::iterator regsIt = _shapeOffsetRegsMap.find(kResRegs2000);
			if (regsIt == _shapeOffsetRegsMap.end()) {
				error("town: required cursor REGS table is unavailable");
				return ZmbEventHandleResult::kPassthrough;
			}
			ZoombiniCursorManager *zmbCursor = dynamic_cast<ZoombiniCursorManager *>(_vm->_cursor);
			if (!zmbCursor) {
				error("town: cursor manager for Zoombini is unavailable");
				return ZmbEventHandleResult::kPassthrough;
			}
			zmbCursor->setShapeCursor(ZmbResource::kPage, kResBitmapShape2000_Cursors, cursorShapeIdx, regsIt->_value->getShapeDelta(cursorShapeIdx));
		}
		_hoverCursorShapeIdx = cursorShapeIdx;
	}

	if (_memorialCardActive)
		return ZmbEventHandleResult::kConsumed;

	return ZoombiniShelter::onMouseMove(absPos, relPos);
}

bool ZoombiniShelterTown::isTownButtonRect(const Common::Point &pos) const {
	return _helpButtonRect.contains(pos) || _mapButtonRect.contains(pos);
}

bool ZoombiniShelterTown::isTownScrollViewportPoint(const Common::Point &pos) const {
	return 3 < pos.x && pos.x < 637 && 30 < pos.y && pos.y < 450;
}

ZmbEventHandleResult ZoombiniShelterTown::onKeyDown(const Common::KeyState &kbd, bool kbdRepeat) {
	if (_vm->useEnhancedKbdShortcuts() && !kbdRepeat && !kbd.hasFlags(Common::KBD_CTRL) &&
		!_memorialCardActive && !isDragging()) {
		// This enhanced debug feature is unavailable here.
		switch (getKeyboardNavDirection(kbd)) {
		case KBD_NAV_LEFT:
			scrollTownLeft();
			return ZmbEventHandleResult::kConsumed;
		case KBD_NAV_RIGHT:
			scrollTownRight();
			return ZmbEventHandleResult::kConsumed;
		default:
			break;
		}
	}

	return ZoombiniShelter::onKeyDown(kbd, kbdRepeat);
}

ZmbEventHandleResult ZoombiniShelterTown::onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) {
	// Dismiss an active card first, then test memorial markers and edge-scroll zones before starting a Snoid drag.
	if (_memorialCardActive) {
		hideMemorialCard();
		return ZmbEventHandleResult::kConsumed;
	}
	if (isDragging()) {
		return ZmbEventHandleResult::kConsumed;
	}
	ZmbFeature *clockHit = findRunnerAtPoint(absPos, ZmbFeature::FLAG_00000002_TYPE_GRIDWALKER | ZmbFeature::FLAG_00008000_LOOP_ANIM);
	if (clockHit == _clockHandsFeature && _clockSpinCycles == 0) {
		_clockSpinCycles = -1;
	}

	// Memorial hotspot hit test (16 building-marker card slots)
	int16 slotHit = hitTestMemorialHotspots(absPos);
	if (0 <= slotHit) {
		showMemorialCard(slotHit);
		return ZmbEventHandleResult::kConsumed;
	}

	if (isTownButtonRect(absPos)) {
		ZmbEventHandleResult result = ZoombiniShelter::onLButtonDown(absPos, relPos);
		return (result == ZmbEventHandleResult::kConsumed) ? result : ZmbEventHandleResult::kConsumed;
	}

	// Left/right edge scroll
	if (isTownScrollViewportPoint(absPos)) {
		if (560 < absPos.x) {
			scrollTownRight();
			return ZmbEventHandleResult::kConsumed;
		}
		if (absPos.x < 80) {
			scrollTownLeft();
			return ZmbEventHandleResult::kConsumed;
		}
	}

	// ZMB Pack Interaction
	ZmbFeature *walkerHit = findRunnerAtPoint(absPos, ZmbFeature::FLAG_00000002_TYPE_GRIDWALKER);
	ZmbSnoid *snoid = nullptr;
	if (walkerHit && walkerHit->getFlags() == ZmbFeature::FLAG_00000002_TYPE_GRIDWALKER)
		snoid = dynamic_cast<ZmbSnoid *>(walkerHit);
	if (snoid) {
		if (_hoverCursorShapeIdx != ZmbHotspot::kShapeNone) {
			_vm->_cursor->setDefaultCursor();
			_hoverCursorShapeIdx = ZmbHotspot::kShapeNone;
		}
		startSnoidDrag(snoid, absPos);
		return ZmbEventHandleResult::kConsumed;
	}

	return ZoombiniShelter::onLButtonDown(absPos, relPos);
}

void ZoombiniShelterTown::endDrag(const Common::Point &dropPos) {
	ZmbSnoid *snoid = finishSnoidDrag();
	if (!snoid)
		return;

	const Common::Point finalPos = snoid->getPointLoc();
	if (410 <= finalPos.y && finalPos.y <= 475) {
		// Deliberate representation adaptation: no separate town-runner release field is modeled,
		// so keep the walker at the dropped position and use it as the sort anchor.
		// Preserve the common release handoff so the settled walker faces right.
		settleSnoidAtTarget(snoid, finalPos);
	} else {
		snoid->setAnimTargetPos(_dragOrigPos);
		snoid->setAnimState(kSnoidAnimState004_Arrive);
	}
	(void)dropPos;
}

void ZoombiniShelterTown::showMemorialCard(int16 slotIdx) {
	if (slotIdx < 0 || 16 <= slotIdx)
		return;
	if (_vm->_state->_f._memorialRoutes[slotIdx] == 0)
		return;
	if (_memorialCardFeature)
		unloadScrbFeature(_memorialCardFeature);

	const ZmbResource memorialSfx(ZmbResource::kSystem, kSysResSound0999_ButtonSFX);
	// The original engine queues SND 999 as a priority-0 Town script sound.
	// Active Town music or narration normally suppresses and discards that candidate.
	// Disabling this option preserves that arbitration, while the default enabled mode
	// bypasses the queue so opening a memorial card always gives immediate feedback.
	if (ConfMan.getBool(MohawkMetaEngine_Zoombini::kOptionTownAlwaysPlayMemorialSfx))
		_vm->_sound->playSound(memorialSfx, Audio::Mixer::kSFXSoundType);
	else
		queueScriptSoundForNextRenderPass(memorialSfx);

	// Mark the memorial modal pending and call @ref ZoombiniPage::renderFeatures() twice.
	// Those frames complete the current Town composite before its background runners stop rendering.
	_memorialCardActive = true;
	_memorialCardSlotIdx = slotIdx;
	renderFeatures();
	renderFeatures();

	const int16 scrbId = static_cast<int16>(kResScrb1003_Overlay + kTownMemorialCardScrbTypeBySlot[slotIdx]);
	ZmbFeature::EventHooks hooks;
	hooks.setPostRenderFunc(static_cast<ZmbFeature::OnPostRenderFunc>(&ZoombiniShelterTown::memorialCard_onPostRender));
	_memorialCardFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1000_Overlay), scrbId, 0,
										   ZmbFeature::FLAG_00001000_TOPMOST | ZmbFeature::FLAG_00004000_NO_DIRTY_MERGE,
										   hooks);
	setBackgroundRunnerRenderEnabled(false);

}

void ZoombiniShelterTown::hideMemorialCard() {
	if (!_memorialCardActive)
		return;
	if (_memorialCardFeature) {
		unloadScrbFeature(_memorialCardFeature);
		_memorialCardFeature = nullptr;
	}
	_memorialCardActive = false;
	_memorialCardSlotIdx = -1;
	setBackgroundRunnerRenderEnabled(true);
}

void ZoombiniShelterTown::setBackgroundRunnerRenderEnabled(bool renderEnabled) {
	// Apply the requested value to the 20 fixed Town runners and every lower walking Snoid.
	// The fixed array is the clock hands, 16 inhabitants, and 3 celebration slots.
	// All use the GRIDWALKER runtime category without owning traversal state.
	for (ZmbFeature *feature : _scrbFeatures) {
		if (!feature || !feature->hasFlag(ZmbFeature::FLAG_00000002_TYPE_GRIDWALKER))
			continue;
		if (renderEnabled)
			feature->activateRender();
		else
			feature->deactivateRender();
	}
	for (ZmbSnoid *snoid : _snoidMap) {
		if (!snoid || !snoid->hasFlag(ZmbFeature::FLAG_00000002_TYPE_GRIDWALKER))
			continue;
		if (renderEnabled)
			snoid->activateRender();
		else
			snoid->deactivateRender();
	}
}

void ZoombiniShelterTown::memorialCard_onPostRender(ZmbFeature *feature) {
	if (!_memorialCardActive || feature != _memorialCardFeature)
		return;
	if (_memorialCardSlotIdx < 0 || 16 <= _memorialCardSlotIdx)
		return;

	int16 memorialDataSlotIdx = _memorialCardSlotIdx;
	if (_builtinCheatTextIndex != 0)
		memorialDataSlotIdx = _builtinCheatTextIndex - 1;
	if (memorialDataSlotIdx < 0 || 16 <= memorialDataSlotIdx)
		return;

	const ZmbStateFile &stateFile = _vm->_state->_f;
	const byte route = stateFile._memorialRoutes[memorialDataSlotIdx];
	const byte level = stateFile._memorialLevels[memorialDataSlotIdx];
	if (route == 0 || level == 0)
		return;

	int16 topOffset = 20;
	uint32 textPalette = ZoombiniGraphics::kColor2D_Black;
	uint32 outlinePalette = 199;
	switch (feature->getId()) {
	case kResScrb1005_MemorialCard:
		topOffset = 22;
		break;
	case kResScrb1006_MemorialCard:
		topOffset = 14;
		outlinePalette = 212;
		break;
	case kResScrb1007_MemorialCard:
		topOffset = 14;
		textPalette = 205;
		outlinePalette = ZoombiniGraphics::kColor2D_Black;
		break;
	default:
		break;
	}

	const Common::Rect cardRect = feature->getClickRect();
	if (cardRect.isEmpty())
		return;

	const uint32 honorKey = static_cast<uint32>(ZoombiniText::kMemorialHonorMonument) + kTownMemorialCardTextTypeBySlot[_memorialCardSlotIdx];
	const uint32 routeLevelKey = static_cast<uint32>(ZoombiniText::kMemorialRoute1Level1) + (((route - 1) * 4 + (level - 1)) & 0x0F);
	const uint32 levelKey = static_cast<uint32>(ZoombiniText::kLevel1) + MIN<byte>(level - 1, 3);
	const byte memorialMonth = stateFile._memorialMonths[memorialDataSlotIdx];
	const byte monthIdx = memorialMonth == 0 ? 0 : MIN<byte>(memorialMonth - 1, 11);

	const Common::U32String monthText = _vm->_text->getLocalizedString(static_cast<uint32>(ZoombiniText::kMemorialJanuary) + monthIdx);
	const byte memorialDay = stateFile._memorialDays[memorialDataSlotIdx];
	const uint16 memorialYear = stateFile._memorialYears[memorialDataSlotIdx];
	const bool usesDayMonthYear = _vm->isGameVariant(MohawkGameFeatures::GF_ZMB_10_EU) ||
								  _vm->isGameVariant(MohawkGameFeatures::GF_ZMB_11_EU) ||
								  _vm->isGameVariant(MohawkGameFeatures::GF_ZMB_11_FR_2002);

	Common::U32String dateText;
	if (usesDayMonthYear) {
		dateText = Common::U32String::format("%u ", memorialDay);
		dateText += monthText;
		dateText += Common::U32String::format(" %u", memorialYear);
	} else {
		dateText = monthText;
		dateText += Common::U32String::format(" %u", memorialDay);
		if (_vm->getLanguage() == Common::KO_KOR)
			dateText += Common::U32String(U"일");
		dateText += Common::U32String::format(", %u", memorialYear);
	}

	const Common::U32String rowText[5] = {
		_vm->_text->getLocalizedString(honorKey),
		_vm->_text->getLocalizedString(routeLevelKey),
		_vm->_text->getLocalizedString(ZoombiniText::kMemorialWhenLevel),
		_vm->_text->getLocalizedString(levelKey),
		dateText};
	const ZoombiniFontUsage rowFont[5] = {
		ZoombiniFontUsage::kFontText,
		ZoombiniFontUsage::kFontTitle,
		ZoombiniFontUsage::kFontText,
		ZoombiniFontUsage::kFontTitle,
		ZoombiniFontUsage::kFontText};

	ZoombiniGraphics::TextConf textConf;
	textConf._outlineEffect = true;
	textConf._textPalette = textPalette;
	textConf._outlinePalette = outlinePalette;
	textConf._hAlign = Graphics::kTextAlignCenter;
	textConf._vAlign = Graphics::kTextAlignCenter;

	for (uint16 rowIdx = 0; rowIdx < 5; rowIdx++) {
		Common::Rect textRect(cardRect.left,
							  cardRect.top + topOffset + kTownMemorialCardRowTopY[rowIdx],
							  cardRect.right,
							  cardRect.top + topOffset + kTownMemorialCardRowBottomY[rowIdx]);
		textConf._fontUsage = rowFont[rowIdx];
		_vm->_gfx->drawText(ZoombiniGraphics::kShapeScreen, rowText[rowIdx], textRect, textConf);
	}
}

int16 ZoombiniShelterTown::hitTestMemorialHotspots(const Common::Point &pos) const {
	// Test the rectangles built by @ref ZoombiniShelterTown::memorialMarkers_preRenderShape().
	// They correspond to the currently visible SCRB 1001 frame.
	const ZmbStateFile &f = _vm->_state->_f;
	for (uint16 markerIdx = 0; markerIdx < _memorialHotspotCount && markerIdx < 16; markerIdx++) {
		const int16 slotIdx = _memorialSlotMapping[markerIdx];
		if (slotIdx < 0 || 16 <= slotIdx)
			continue;
		if (f._memorialRoutes[slotIdx] == 0)
			continue;
		if (_memorialHotspots[markerIdx].isEmpty())
			continue;
		if (_memorialHotspots[markerIdx].contains(pos))
			return slotIdx;
	}
	return -1;
}

void ZoombiniShelterTown::shiftRunnersForScroll(int16 phaseIdx) {
	// Every Town GRIDWALKER-category runner shifts by one 320px column and wraps
	// across the 1920px town width.
	for (ZmbFeature *feature : _scrbFeatures)
		shiftTownEntityForScroll(feature, phaseIdx);
	for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
		shiftTownEntityForScroll(*it, phaseIdx);
	}
}

void ZoombiniShelterTown::shiftTownEntityForScroll(ZmbFeature *feature, int16 phaseIdx) {
	if (!feature || !feature->hasFlag(ZmbFeature::FLAG_00000002_TYPE_GRIDWALKER))
		return;

	// Preserve the old column coverage before replacing the materialized position.
	// Scrolling can run between render passes, so keep the damage in both accumulators.
	const Common::Rect oldCoverage = feature->getZSortRect();
	markFeatureVisualCoverageDirty(feature, false);
	addExternalDirtyRect(oldCoverage);

	Common::Point point = feature->getPointLoc();
	if (phaseIdx != 0) {
		point.x -= 320;
		if (point.x < -320)
			point.x += 1920;
	} else {
		point.x += 320;
		if (1599 < point.x)
			point.x -= 1920;
	}
	feature->setPointLoc(point);

	ZmbSnoid *snoid = dynamic_cast<ZmbSnoid *>(feature);
	if (feature == _clockHandsFeature) {
		// Scrolling expires the clock timer. Its timed pre-render owner rebuilds the
		// hands and click rectangle on the next graphics pass.
		feature->resetNextRenderFrame();
	} else if (snoid) {
		Common::Point sortAnchor = snoid->getAnimTargetPos();
		if (phaseIdx != 0) {
			sortAnchor.x -= 320;
			if (sortAnchor.x < -320)
				sortAnchor.x += 1920;
		} else {
			sortAnchor.x += 320;
			if (1599 < sortAnchor.x)
				sortAnchor.x -= 1920;
		}
		snoid->setAnimTargetPos(sortAnchor);
		prepareSnoidVisualCoverage(snoid, true);
	} else {
		prepareFeatureVisualCoverage(feature);
	}

	markPreparedFeatureVisualCoverageDirty(feature, false);
	addExternalDirtyRect(feature->getZSortRect());
	feature->setNeedsRedraw(true);
}

void ZoombiniShelterTown::advanceLayerFrameState(uint16 scrollCol) {
	// Materialize all four overlay runners in registration order for the new Town column.
	// The memorial layer runs last and establishes clock-tower visibility before
	// the clock runner is shifted and rebuilt.
	for (int16 featureIdx = 0; featureIdx < 4; featureIdx++) {
		ZmbFeature *layer = _overlayFeatures[featureIdx];
		if (!layer)
			continue;

		markFeatureVisualCoverageDirty(layer, false);
		layer->setLastFrameIdx(scrollCol);
		if (layer->isInitialScrbLoadPending()) {
			layer->setInitialScrbLoadPending(false);
		}

		prepareFeatureVisualCoverage(layer, nullptr, false);
		layer->deactivateRender();
		markPreparedFeatureVisualCoverageDirty(layer, false);
		layer->setNeedsRedraw(true);
	}
}

void ZoombiniShelterTown::scrollTownLeft() {
	ZmbStateFile &stateFile = _vm->_state->_f;
	_vm->_sound->playSound(ZmbResource(ZmbResource::kSystem, kSysResSound0999_ButtonSFX), Audio::Mixer::kSFXSoundType);
	if (stateFile._townScrollCol == 0)
		stateFile._townScrollCol = 5;
	else
		stateFile._townScrollCol -= 1;
	advanceLayerFrameState(stateFile._townScrollCol);
	shiftRunnersForScroll(0);
}

void ZoombiniShelterTown::scrollTownRight() {
	ZmbStateFile &stateFile = _vm->_state->_f;
	_vm->_sound->playSound(ZmbResource(ZmbResource::kSystem, kSysResSound0999_ButtonSFX), Audio::Mixer::kSFXSoundType);
	stateFile._townScrollCol += 1;
	if (5 < stateFile._townScrollCol)
		stateFile._townScrollCol = 0;
	advanceLayerFrameState(stateFile._townScrollCol);
	shiftRunnersForScroll(1);
}

} // End of namespace Mohawk
