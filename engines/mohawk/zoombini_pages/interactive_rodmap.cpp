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

#include "mohawk/console.h"
#include "mohawk/mohawk.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"

#include "interactive_rodmap.h"
#include "mohawk/zoombini.h"
#include "mohawk/zoombini_graphics.h"
#include "mohawk/zoombini_pages/interactive_rodmap.h"
#include "mohawk/zoombini_sound.h"
#include "mohawk/zoombini_state.h"
#include "mohawk/zoombini_text.h"

namespace Mohawk {

constexpr ZoombiniText::Key ZoombiniInteractiveRodMap::_routeNameTextKey[4];
constexpr uint16 ZoombiniInteractiveRodMap::_levelLegendPalettes[4];
constexpr Common::Point ZoombiniInteractiveRodMap::_pageClickPoints[16];
constexpr ZoombiniPageType ZoombiniInteractiveRodMap::_pageClickTypes[16];
constexpr uint16 ZoombiniInteractiveRodMap::_pageClickShapes[16];

ZoombiniInteractiveRodMap::ZoombiniInteractiveRodMap(MohawkEngine_Zoombini *vm) : ZoombiniInteractive(vm, ZoombiniPageType::kRodMap) {
	// Clickable Pos & Rect
	for (uint32 i = 0; i < ARRAYSIZE(_pageClickPoints); i++) {
		_pageClickRects[i] = Common::Rect(_pageClickPoints[i].x - 20, _pageClickPoints[i].y - 15, _pageClickPoints[i].x + 20, _pageClickPoints[i].y + 15);
	}

	// Latch the session's first Maze layouts before this map can launch a Maze page.
	_vm->initializeMazeLayoutVariants();
	buildPageRouteLevelMap();
}

ZoombiniInteractiveRodMap::~ZoombiniInteractiveRodMap() {
}

const ZoombiniPage::ScriptSoundPriorityRanges &ZoombiniInteractiveRodMap::getScriptSoundPriorityRanges() const {
	static const ZoombiniPage::ScriptSoundPriorityRanges kRanges = {
		{kSysResSoundRange20000_Base, kSysResSoundRange29999_Last}};
	return kRanges;
}

void ZoombiniInteractiveRodMap::open() {
	const char *mhkArchive = ZMB_MHK_RODMAP;
	if (_vm->isVersionFamilyEuV1())
		mhkArchive = ZMB_MHK_MAP;
	openArchive(mhkArchive);
}

void ZoombiniInteractiveRodMap::setBackgroundBitmap() {
	_vm->_gfx->setPalette(kResBackground300);
	_vm->_gfx->drawBackground(kResBackground300);
}

int16 ZoombiniInteractiveRodMap::getHoverTooltipScrbId() const {
	return _vm->isVersionFamilyEuV1() ? kResScrbEuropePageNameHover1004 : kResScrbUsPageNameHover1005;
}

int16 ZoombiniInteractiveRodMap::getOptionButtonScrbId() const {
	return _vm->isVersionFamilyEuV1() ? kResScrbEuropeMenuButton1005 : kResScrbUsMenuButton1006;
}

int16 ZoombiniInteractiveRodMap::getLevelLegendScrbId() const {
	return _vm->isVersionFamilyEuV1() ? kResScrbEuropeLevelLegend1003 : kResScrbUsLevelLegend1004;
}

const Common::Rect &ZoombiniInteractiveRodMap::getRouteNameRect(uint32 routeIdx) const {
	if (ARRAYSIZE(_routeNameRects) <= routeIdx) {
		error("rodmap: invalid route-name index %u", routeIdx);
		return _routeNameRects[0];
	}
	const bool espt10Layout = _vm->isGameVariant(MohawkGameFeatures::GF_ZMB_10_ESPT);
	const bool classicEuropeLayout = _vm->isVersionFamilyEuV1() && !espt10Layout;
	if (classicEuropeLayout)
		return _europeRouteNameRects[routeIdx];
	return _routeNameRects[routeIdx];
}

const Common::Rect &ZoombiniInteractiveRodMap::getLevelLegendClickRect(uint32 levelIdx) const {
	if (ARRAYSIZE(_levelLegendClickRects) <= levelIdx) {
		error("rodmap: invalid level-legend index %u", levelIdx);
		return _levelLegendClickRects[0];
	}
	return _vm->isVersionFamilyEuV1() ? _europeLevelLegendClickRects[levelIdx] : _levelLegendClickRects[levelIdx];
}

void ZoombiniInteractiveRodMap::loadRodmapPanelFeatures() {
	// Europe MAP.MHK uses SCRB 1003; US RODMAP.MHK uses SCRB 1004.
	// The level legend is only clickable in practice mode.
	ZmbFeature::EventHooks hooks1004;
	hooks1004.setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniInteractiveRodMap::patchSelectedLevelShape1004_preRenderShape));
	hooks1004.setPostRenderFunc(static_cast<ZmbFeature::OnPostRenderFunc>(&ZoombiniInteractiveRodMap::textLegend1004_postRender));
	hooks1004.setLButtonDownFunc(static_cast<ZmbFeature::OnLButtonDownFunc>(&ZoombiniInteractiveRodMap::legendLevel1004_onLButtonDown));
	hooks1004.setKeyDownFunc(static_cast<ZmbFeature::OnKeyDownFunc>(&ZoombiniInteractiveRodMap::legendLevel1004_onKeyDown));
	const int modeFrameVal = _vm->_state->inPracticeMode() ? 6 : 0;
	_levelLegendFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1000), getLevelLegendScrbId(), modeFrameVal,
										  ZmbFeature::FLAG_00100000_PLAY_ONCE,
										  hooks1004);

	// SCRB 1002: Journey statistics. European releases use a taller standalone panel.
	ZmbFeature::EventHooks hooks1002;
	hooks1002.setPostRenderFunc(static_cast<ZmbFeature::OnPostRenderFunc>(&ZoombiniInteractiveRodMap::textJourneyStat1002_postRender));
	_journeyStatFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1000), kResScrbJourneyStats1002, 0,
										  ZmbFeature::FLAG_00100000_PLAY_ONCE,
										  hooks1002);

	if (!_vm->isVersionFamilyEuV1()) {
		// US RODMAP.MHK SCRB 1003: mode select combobox.
		// RodMap keeps positional Z-sorting disabled and uses its explicit runner-list order.
		// Register the mode markers after the journey panel and keep them at the list tail.
		ZmbFeature::EventHooks hooks1003;
		hooks1003.setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniInteractiveRodMap::drawComboBox1003_preRenderShape));
		hooks1003.setLButtonDownFunc(static_cast<ZmbFeature::OnLButtonDownFunc>(&ZoombiniInteractiveRodMap::selectMode1003_onLButtonDown));
		_modeComboFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1000), kResScrbUsModeCombobox1003, 0,
											ZmbFeature::FLAG_00100000_PLAY_ONCE,
											hooks1003);
		manualLinkAtEnd(_modeComboFeature);
	}
}

void ZoombiniInteractiveRodMap::unloadRodmapPanelFeatures() {
	if (_levelLegendFeature) {
		unloadScrbFeature(_levelLegendFeature);
		_levelLegendFeature = nullptr;
	}
	if (_journeyStatFeature) {
		unloadScrbFeature(_journeyStatFeature);
		_journeyStatFeature = nullptr;
	}
	if (_modeComboFeature) {
		unloadScrbFeature(_modeComboFeature);
		_modeComboFeature = nullptr;
	}
}

void ZoombiniInteractiveRodMap::loadFeatures() {
	// RodMap preserves registration and explicit link order for its complete lifetime.
	setRunnerZSortEnabled(false);

	// Restore the user's snapshotted state when re-entering the map after a practice puzzle.
	// @ref ZoombiniGameState::_practiceLevel remains outside @ref ZoombiniGameState::_f,
	// so practice mode stays active for launching another puzzle without re-toggling it.
	if (_vm->_state->_practiceStateBackupActive) {
		_vm->_state->_f = _vm->_state->_practiceStateBackup;
		_vm->_state->_practiceStateBackupActive = false;
	}

	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape1000));

	// [*] SCRB 1000: Page Icon
	ZmbFeature::EventHooks hooks1000;
	hooks1000.setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniInteractiveRodMap::patchPageShape1000_preRenderShape));
	hooks1000.setLButtonDownFunc(static_cast<ZmbFeature::OnLButtonDownFunc>(&ZoombiniInteractiveRodMap::runPage1000_onLButtonDown));
	_pageIconFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1000), kResScrbPageIcon1000, 6,
									   ZmbFeature::FLAG_08000000_REGION_TRACK | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_00008000_LOOP_ANIM,
									   hooks1000);
	// [*] SCRB 1001: Route Shapes
	// + Draw Route Name Text
	ZmbFeature::EventHooks hooks1001;
	hooks1001.setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniInteractiveRodMap::patchRouteShape1001_preRenderShape));
	_routeSegmentsFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1000), kResScrbRoute1001, 6,
											ZmbFeature::FLAG_08000000_REGION_TRACK | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_00008000_LOOP_ANIM,
											hooks1001);

	// Europe MAP.MHK uses SCRB 1004; US RODMAP.MHK uses SCRB 1005.
	// The panel only appears after hovering one of the puzzle icons.
	ZmbFeature::EventHooks hooks1005;
	hooks1005.setPreRenderFunc(static_cast<ZmbFeature::OnPreRenderFunc>(&ZoombiniInteractiveRodMap::drawAfterPageIconHover1005_preRender));
	hooks1005.setRenderFunc(static_cast<ZmbFeature::OnRenderFunc>(&ZoombiniInteractiveRodMap::renderAfterPageIconHover1005));
	hooks1005.setPostRenderFunc(static_cast<ZmbFeature::OnPostRenderFunc>(&ZoombiniInteractiveRodMap::textPageName1005_postRender));
	_hoverNameFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1000), getHoverTooltipScrbId(), 6,
										ZmbFeature::FLAG_00100000_PLAY_ONCE,
										hooks1005);
	// Europe MAP.MHK uses SCRB 1005; US RODMAP.MHK uses SCRB 1006.
	ZmbFeature::EventHooks hooks1006;
	hooks1006.setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniInteractiveRodMap::optionButton1006_preRenderShape));
	hooks1006.setPostRenderFunc(static_cast<ZmbFeature::OnPostRenderFunc>(&ZoombiniInteractiveRodMap::optionButton1006_postRender));
	hooks1006.setLButtonDownFunc(static_cast<ZmbFeature::OnLButtonDownFunc>(&ZoombiniInteractiveRodMap::optionButton1006_onLButtonDown));
	_optionButtonFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1000), getOptionButtonScrbId(), 3,
										   ZmbFeature::FLAG_00001000_TOPMOST,
										   hooks1006);

	// [*] Callback-only runner: Route Names
	// A scrbId=0 callback runner draws the route labels before the static panels.
	ZmbFeature::EventHooks hooksRouteNames;
	hooksRouteNames.setPostRenderFunc(static_cast<ZmbFeature::OnPostRenderFunc>(&ZoombiniInteractiveRodMap::textRouteNames_postRender));
	_routeNamesFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, 0), 0, 0, ZmbFeature::FLAG_00100000_PLAY_ONCE, hooksRouteNames);

	loadRodmapPanelFeatures();

	// SND 998 and 999 are menu UI effects.
	// The resource layer caches them on first access, so explicit preloading is unnecessary.
}

ZmbEventHandleResult ZoombiniInteractiveRodMap::onMouseMove(const Common::Point &absPos, const Common::Point &relPos) {
	ZoombiniInteractive::onMouseMove(absPos, relPos);
	optionButton1006_updateTlcHover(absPos);
	_lastMouseAbsPos = absPos;
	_hasLastMouseAbsPos = true;

	return ZmbEventHandleResult::kPassthrough;
}

void ZoombiniInteractiveRodMap::onAnimFrame() {
	// Poll the saved cursor position every frame.
	// Refreshing the deadline only from mouse-move events makes a stationary hover disappear after 60 frames.
	ZoombiniPageType hoveredPageType = ZoombiniPageType::kNone;
	if (_hasLastMouseAbsPos) {
		for (uint32 i = 0; i < ARRAYSIZE(_pageClickRects); i++) {
			if (!_pageClickRects[i].contains(_lastMouseAbsPos) || !isPageHoverable(i))
				continue;
			hoveredPageType = _pageClickTypes[i];
			break;
		}
	}

	if (hoveredPageType != ZoombiniPageType::kNone)
		updateHoveredPage(hoveredPageType);

	// Keep the last hover panel visible for 60 animation frames.
	// Then dirty its old coverage and restore the normal page-icon runner.
	if (_lastHoveredPageType != ZoombiniPageType::kNone && _hoverTimeoutFrame < _currentFrameCounter)
		clearHoveredPage(true);

	ZoombiniInteractive::onAnimFrame();
}

ZmbEventHandleResult ZoombiniInteractiveRodMap::onKeyDown(const Common::KeyState &kbd, bool kbdRepeat) {
	if (kbdRepeat)
		return ZmbEventHandleResult::kConsumed;

	ZmbEventHandleResult result = ZmbEventHandleResult::kConsumed;
	if ((kbd.flags & Common::KBD_CTRL) != 0) {
		switch (kbd.keycode) {
		case Common::KEYCODE_p: // Practice Mode Toggle
			// US releases provide a full practice toggle.
			// European releases only turn practice on here and leave it when a hub page is selected.
			// Enhanced shortcuts add a two-way toggle for European releases;
			// otherwise keep their turn-on-only behavior.
			if (_vm->isVersionFamilyEuV1() && !_vm->useEnhancedKbdShortcuts())
				setPracticeMode(true);
			else
				togglePracticeMode();
			break;
		default:
			result = ZmbEventHandleResult::kPassthrough;
			break;
		}
	} else {
		result = ZmbEventHandleResult::kPassthrough;
	}
	if (result == ZmbEventHandleResult::kConsumed)
		return result;

	return ZoombiniInteractive::onKeyDown(kbd, kbdRepeat);
}

Common::String ZoombiniInteractiveRodMap::debugGetBuiltinDebugCommandHelp() const {
	Common::String output;
	output += Common::String::format("  %-17s (%s)\n", "+", kBuiltinDebugActionAdd);
	output += "    Increase the next generated practice pack by one, capped at 16; the current pack is unchanged.\n";
	output += Common::String::format("  %-17s (%s)\n", "-", kBuiltinDebugActionRemove);
	output += "    Decrease the next generated practice pack by one, floored at 1; the current pack is unchanged.\n";
	output += Common::String::format("  %-17s (%s <a-p>)\n", "Shift+T, then a-p", kBuiltinDebugActionXfer);
	output += "    In practice mode, generate that pack and start XFER as if leaving the page selected by a-p.\n";
	output += "    a-p select Picker, Bridge, Tunnels, Pizza, BC1 North, Ferry, Lilly, Slides, BC1 South, Fleens, Hotel, Net, BC2, Caves, Smoke, or Maze.\n";
	output += "    Any non-selector key after Shift+T cancels the armed selector.\n";
	return output;
}

bool ZoombiniInteractiveRodMap::debugDoBuiltinDebugCommand(int argc, const char **argv, Common::String &output) {
	if (argc != 3 && argc != 4) {
		output = "Usage: page builtin_debug <RodMap action>\n";
		output += debugGetBuiltinDebugCommandHelp();
		return true;
	}

	const BuiltinDebugAction action = parseBuiltinDebugAction(argv[2]);
	if (action == BuiltinDebugAction::kInvalid) {
		output = Common::String::format("Unknown RodMap built-in debug action '%s'.\n", argv[2]);
		output += debugGetBuiltinDebugCommandHelp();
		return true;
	}
	const char argument = argc == 4 && argv[3][0] != '\0' && argv[3][1] == '\0' ? argv[3][0] : '\0';
	const bool failed = runBuiltinDebugAction(action, argument, output);
	if (failed && output.empty()) {
		output = Common::String::format("Unknown RodMap built-in debug action '%s'.\n", argv[2]);
		output += debugGetBuiltinDebugCommandHelp();
	}
	return failed;
}

ZoombiniInteractiveRodMap::BuiltinDebugAction ZoombiniInteractiveRodMap::parseBuiltinDebugAction(const Common::String &action) {
	if (action.equalsIgnoreCase(kBuiltinDebugActionAdd))
		return BuiltinDebugAction::kAdd;
	if (action.equalsIgnoreCase(kBuiltinDebugActionRemove))
		return BuiltinDebugAction::kRemove;
	if (action.equalsIgnoreCase(kBuiltinDebugActionXfer))
		return BuiltinDebugAction::kXfer;
	return BuiltinDebugAction::kInvalid;
}

bool ZoombiniInteractiveRodMap::runBuiltinDebugAction(BuiltinDebugAction action, char argument, Common::String &output) {
	if (action == BuiltinDebugAction::kAdd && argument == '\0') {
		if (_builtinPracticePackCount < 16)
			_builtinPracticePackCount += 1;
		drawBuiltinDebugText(Common::String::format("Snoids to practice with = %d", _builtinPracticePackCount));
		output = Common::String::format("RodMap practice pack count: %d.\n", _builtinPracticePackCount);
		return false;
	}
	if (action == BuiltinDebugAction::kRemove && argument == '\0') {
		if (1 < _builtinPracticePackCount)
			_builtinPracticePackCount -= 1;
		drawBuiltinDebugText(Common::String::format("Snoids to practice with = %d", _builtinPracticePackCount));
		output = Common::String::format("RodMap practice pack count: %d.\n", _builtinPracticePackCount);
		return false;
	}
	if (action == BuiltinDebugAction::kXfer && argument != '\0')
		return runBuiltinXferSelector(argument, output);

	return true;
}

bool ZoombiniInteractiveRodMap::runBuiltinXferSelector(char selector, Common::String &output) {
	if (!_vm->_state->inPracticeMode()) {
		output = "RodMap built-in XFER selection is only available in practice mode.\n";
		return true;
	}

	char lowerSelector = selector;
	if ('A' <= lowerSelector && lowerSelector <= 'Z')
		lowerSelector = static_cast<char>(lowerSelector - 'A' + 'a');

	ZmbSrcPageKind srcPage = ZmbSrcPageKind::kMinus1;
	switch (lowerSelector) {
	case 'a':
		srcPage = ZmbSrcPageKind::kPicker_01;
		break;
	case 'b':
		srcPage = ZmbSrcPageKind::kBridge_02;
		break;
	case 'c':
		srcPage = ZmbSrcPageKind::kTunnels_03;
		break;
	case 'd':
		srcPage = ZmbSrcPageKind::kPizza_04;
		break;
	case 'e':
		srcPage = ZmbSrcPageKind::kBC1North_05;
		break;
	case 'f':
		srcPage = ZmbSrcPageKind::kFerry_07;
		break;
	case 'g':
		srcPage = ZmbSrcPageKind::kLilly_08;
		break;
	case 'h':
		srcPage = ZmbSrcPageKind::kSlides_09;
		break;
	case 'i':
		srcPage = ZmbSrcPageKind::kBC1South_06;
		break;
	case 'j':
		srcPage = ZmbSrcPageKind::kFleens_10;
		break;
	case 'k':
		srcPage = ZmbSrcPageKind::kHotel_11;
		break;
	case 'l':
		srcPage = ZmbSrcPageKind::kNet_12;
		break;
	case 'm':
		srcPage = ZmbSrcPageKind::kBC2_13;
		break;
	case 'n':
		srcPage = ZmbSrcPageKind::kCaves_14;
		break;
	case 'o':
		srcPage = ZmbSrcPageKind::kSmoke_15;
		break;
	case 'p':
		srcPage = ZmbSrcPageKind::kMaze_16;
		break;
	default:
		output = "RodMap XFER selector must be one letter from A through P.\n";
		return true;
	}

	generatePracticePack();
	_vm->_debugPreserveActivePackOnXferClose = true;
	_vm->_xferSrcPage = srcPage;
	_vm->setNextPage(ZoombiniPageType::kXfer);
	close();
	const char upperSelector = static_cast<char>(lowerSelector - 'a' + 'A');
	output = Common::String::format("RodMap practice XFER %c started with %d Zoombinis.\n", upperSelector, _builtinPracticePackCount);
	return false;
}

ZmbEventHandleResult ZoombiniInteractiveRodMap::onDebugKeyDown(const Common::KeyState &kbd) {
	if (_builtinXferSelectorArmed) {
		_builtinXferSelectorArmed = false;
		addExternalDirtyRect(Common::Rect(0, 0, 310, 16));
		if (!kbd.hasFlags(0) || kbd.keycode < Common::KEYCODE_a || Common::KEYCODE_p < kbd.keycode)
			return ZmbEventHandleResult::kPassthrough;

		Common::String output;
		if (runBuiltinXferSelector(static_cast<char>(kbd.keycode), output))
			return ZmbEventHandleResult::kPassthrough;
		else
			return ZmbEventHandleResult::kConsumed;
	}

	if ((kbd.hasFlags(0) || kbd.hasFlags(Common::KBD_SHIFT)) && kbd.ascii == 'T') {
		if (!_vm->_state->inPracticeMode())
			return ZmbEventHandleResult::kPassthrough;
		drawBuiltinDebugText("Which Transition (a-p):");
		_builtinXferSelectorArmed = true;
		return ZmbEventHandleResult::kPassthrough;
	}

	BuiltinDebugAction action = BuiltinDebugAction::kInvalid;
	if ((kbd.hasFlags(0) || kbd.hasFlags(Common::KBD_SHIFT)) && kbd.ascii == '+')
		action = BuiltinDebugAction::kAdd;
	else if (kbd.hasFlags(0) && kbd.ascii == '-')
		action = BuiltinDebugAction::kRemove;
	else
		return ZmbEventHandleResult::kPassthrough;

	Common::String output;
	runBuiltinDebugAction(action, '\0', output);
	return ZmbEventHandleResult::kPassthrough;
}

void ZoombiniInteractiveRodMap::setPracticeMode(bool setPracticeMode) {
	if ((setPracticeMode && _vm->_state->_practiceLevel == 0) || (!setPracticeMode && _vm->_state->_practiceLevel != 0))
		togglePracticeMode();
}

void ZoombiniInteractiveRodMap::togglePracticeMode() {
	if (_vm->_state->_practiceLevel == 0)
		_vm->_state->_practiceLevel = 1;
	else
		_vm->_state->_practiceLevel = 0;

	refreshModeDependentFeatures(true);

	_vm->_sound->playSound(ZmbResource(ZmbResource::kSystem, kSysResSound0999_ButtonSFX), Audio::Mixer::kSFXSoundType);
}

void ZoombiniInteractiveRodMap::refreshModeDependentFeatures(bool clearHover) {
	buildPageRouteLevelMap();
	if (clearHover)
		clearHoveredPage(false);

	// Remove and recreate the level, statistics, and mode panels before restarting the dependent runners.
	unloadRodmapPanelFeatures();
	redrawRouteNames();
	loadRodmapPanelFeatures();
	if (_vm->_state->inPracticeMode())
		restartRodmapFeature(_levelLegendFeature);
	refreshRouteAndPageFeatures();
	restartRodmapFeature(_modeComboFeature);
}

void ZoombiniInteractiveRodMap::selectPracticeLevel(ZmbFeature *feature, uint16 selectedLevel) {
	if (selectedLevel < 1 || 4 < selectedLevel || !_vm->_state->inPracticeMode()) {
		error("rodmap: invalid practice-level selection %u", selectedLevel);
		return;
	}

	if (_vm->_state->_practiceLevel == selectedLevel)
		return;

	// Restore the previous level-selection panel and reload SCRB 1004 on its existing runner.
	// The reload dirties prior visual coverage before materializing the selected-level shape and outlined text.
	// Without it, visible updates depend on incidental dirty coverage from another runner.
	_vm->_state->_practiceLevel = selectedLevel;
	buildPageRouteLevelMap();
	clearHoveredPage(false);
	restartRodmapFeature(feature);
	// Refresh the route segments, page icons, and mode-combo runner after every level change.
	// Their shapes depend on the refreshed practice level and must not wait for unrelated dirty coverage.
	refreshRouteAndPageFeatures();
	restartRodmapFeature(_modeComboFeature);
	_vm->_sound->playSound(ZmbResource(ZmbResource::kSystem, kSysResSound0999_ButtonSFX), Audio::Mixer::kSFXSoundType);
}

void ZoombiniInteractiveRodMap::restartRodmapFeature(ZmbFeature *feature) {
	if (!feature)
		return;

	loadScrbOntoFeature(feature, 0);
	// Static RodMap runners must expose the restarted script frame in the next render pass.
	feature->requestVisualRematerialization();
}

void ZoombiniInteractiveRodMap::refreshRouteAndPageFeatures() {
	restartRodmapFeature(_routeSegmentsFeature);
	restartRodmapFeature(_pageIconFeature);
}

void ZoombiniInteractiveRodMap::redrawRouteNames() {
	if (!_routeNamesFeature)
		return;

	for (uint32 i = 0; i < ARRAYSIZE(_routeNameRects); i++)
		addExternalDirtyRect(getRouteNameRect(i));

	_routeNamesFeature->activateRender();
	_routeNamesFeature->setNeedsRedraw(true);
}

bool ZoombiniInteractiveRodMap::isPageHoverable(uint32 pageIndex) {
	if (ARRAYSIZE(_pageClickTypes) <= pageIndex) {
		error("rodmap: invalid page-hover index %u", pageIndex);
		return false;
	}

	if (_pageClickTypes[pageIndex] == ZoombiniPageType::kPicker)
		return true;

	return _pageNodeLevelMap[_pageClickShapes[pageIndex]] != 0;
}

bool ZoombiniInteractiveRodMap::isPracticeExitAvailable(ZoombiniPageType pageType) const {
	const ZmbStateFile &state = _vm->_state->_f;

	switch (pageType) {
	case ZoombiniPageType::kPicker:
		return true;
	case ZoombiniPageType::kBasecamp1:
		return (state._levelFlagRouteBigBadHungry & 0x0F) != 0;
	case ZoombiniPageType::kBasecamp2:
		return state._levelFlagLoWhosBayouHiDeepDarkForest != 0;
	case ZoombiniPageType::kTown:
		return (state._levelFlagRouteMontDespair & 0x0F) != 0;
	default:
		return false;
	}
}

void ZoombiniInteractiveRodMap::updateHoveredPage(ZoombiniPageType pageType) {
	_hoverTimeoutFrame = _currentFrameCounter + 60;
	if (_lastHoveredPageType == pageType)
		return;

	_lastHoveredPageType = pageType;
	// Reset the page-icon and hover-name runners when the map target changes.
	// Both loads dirty the previous output before the new highlight and tooltip render.
	restartRodmapFeature(_pageIconFeature);
	restartRodmapFeature(_hoverNameFeature);
}

void ZoombiniInteractiveRodMap::clearHoveredPage(bool reloadPageIcons) {
	if (_lastHoveredPageType == ZoombiniPageType::kNone)
		return;

	_lastHoveredPageType = ZoombiniPageType::kNone;
	_hoverTimeoutFrame = 0;

	if (_hoverNameFeature) {
		const Common::Rect &oldRect = _hoverNameFeature->getZSortRect();
		if (!oldRect.isEmpty())
			addExternalDirtyRect(oldRect);
		_hoverNameFeature->deactivateRender();
		_hoverNameFeature->setNeedsRedraw(false);
	}

	if (reloadPageIcons)
		restartRodmapFeature(_pageIconFeature);
}

void ZoombiniInteractiveRodMap::generatePracticePack() {
	// Generate 16 practice-mode Snoids with independently randomized hair, eye, nose, and feet traits.
	// Snapshot the active state before replacing the pack so the rod map can restore it after the puzzle.
	// Keep the first snapshot for the complete practice session.
	if (!_vm->_state->_practiceStateBackupActive) {
		_vm->_state->_practiceStateBackup = _vm->_state->_f;
		_vm->_state->_practiceStateBackupActive = true;
	}
	_vm->_state->generateRandomPack(_builtinPracticePackCount);
}

void ZoombiniInteractiveRodMap::patchPageShape1000_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)feature;
	(void)hsGroup;

	// Hotspot: shape 1 ~ 16

	Common::HashMap<uint16, uint16> pageShapeIdMap;

	// Hide the page icon if the page was never visited
	pageShapeIdMap[kResShapePicker16] = ZmbHotspot::kShapeNone; // Always show the picker icon
	for (uint16 i = kResShapeBridge01; i <= kResShapeTown15; i++) {
		uint16 shapeId = ZmbHotspot::kShapeNone;
		if (0 < _pageNodeLevelMap[i])
			shapeId = i;
		pageShapeIdMap[i] = shapeId;
	}

	// Patch shapeId of the last hovered page icon (yellow outline)
	if (_lastHoveredPageType != ZoombiniPageType::kNone) {
		uint16 shapeId = 0;
		for (uint16 i = 0; i < ARRAYSIZE(_pageClickTypes); i++) {
			if (_lastHoveredPageType == _pageClickTypes[i]) {
				shapeId = _pageClickShapes[i];
				break;
			}
		}

		// US releases: In practice mode, resting pages should not be outlined even if they are hovered.
		if (_vm->isVersionFamilyEuV1()) {
			switch (shapeId) {
			case kResShapePicker16:
				pageShapeIdMap[shapeId] = 109;
				break;
			default:
				pageShapeIdMap[shapeId] += 93;
				break;
			}
		} else {
			switch (shapeId) {
			case kResShapePicker16:
				if (!_vm->_state->inPracticeMode())
					pageShapeIdMap[shapeId] = 109;
				break;
			case kResShapeBcOne04:
			case kResShapeBcTwo11:
			case kResShapeTown15:
				if (!_vm->_state->inPracticeMode())
					pageShapeIdMap[shapeId] += 93;
				break;
			default:
				pageShapeIdMap[shapeId] += 93;
				break;
			}
		}
	}

	// Apply the remapped shape IDs.
	for (ZmbHotspot &hs : hotspots)
		hs._shapeIdx = pageShapeIdMap[hs._shapeIdx];
}

ZmbEventHandleResult ZoombiniInteractiveRodMap::runPage1000_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	(void)feature;
	(void)relPos;

	for (uint32 i = 0; i < ARRAYSIZE(_pageClickRects); i++) {
		if (!_pageClickRects[i].contains(absPos) || !isPageHoverable(i))
			continue;

		ZoombiniPageType nextType = _pageClickTypes[i];

		if (_vm->_state->inPracticeMode()) {
			switch (nextType) {
			case ZoombiniPageType::kBridge:
			case ZoombiniPageType::kCaves:
			case ZoombiniPageType::kPizza:
			case ZoombiniPageType::kFerry:
			case ZoombiniPageType::kLilly:
			case ZoombiniPageType::kSlides:
			case ZoombiniPageType::kFleens:
			case ZoombiniPageType::kHotel:
			case ZoombiniPageType::kNet:
			case ZoombiniPageType::kTunnels:
			case ZoombiniPageType::kSmoke:
			case ZoombiniPageType::kMaze:
				generatePracticePack();
				_vm->setNextPage(nextType);
				close();
				return ZmbEventHandleResult::kConsumed;
			case ZoombiniPageType::kPicker:
			case ZoombiniPageType::kBasecamp1:
			case ZoombiniPageType::kBasecamp2:
			case ZoombiniPageType::kTown:
				// The European map has no practice/journey mode combobox.
				// Zoombini Isle is always available.
				// Each shelter or town becomes an exit after its journey checkpoint is unlocked.
				// In the US RODMAP, the same clicks are inert while practicing.
				// Leave practice through the mode combobox instead.
				// The on-map roster is already the real non-practice pack,
				// so no state restore is needed here.
				if (_vm->isVersionFamilyEuV1()) {
					if (isPracticeExitAvailable(nextType)) {
						_vm->_state->_practiceLevel = 0;
						refreshModeDependentFeatures(false);
						// Materialize the journey map before close freezes the final composite for the fade.
						renderFeatures();
						_vm->setNextPage(nextType);
						close();
					}
					return ZmbEventHandleResult::kConsumed;
				}
				break;
			default:
				break;
			}
		} else {
			switch (nextType) {
			case ZoombiniPageType::kPicker:
			case ZoombiniPageType::kBasecamp1:
			case ZoombiniPageType::kBasecamp2:
			case ZoombiniPageType::kTown:
				_vm->setNextPage(nextType);
				close();
				return ZmbEventHandleResult::kConsumed;
			default:
				break;
			}
		}
	}

	return ZmbEventHandleResult::kPassthrough;
}

void ZoombiniInteractiveRodMap::patchRouteShape1001_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)feature;
	(void)hsGroup;

	// Level 1: base hotspot shapes 17-32
	// Level 2: shape 33 ~ 47
	// Level 3: shape 48 ~ 63
	// Level 4: shape 64 ~ 79

	if (_vm->_state->inPracticeMode()) {
		if (_vm->_state->_practiceLevel < 1 || 4 < _vm->_state->_practiceLevel) {
			error("rodmap: invalid practice level %u", _vm->_state->_practiceLevel);
			return;
		}
		uint16 addShapeNum = (_vm->_state->_practiceLevel - 1) * 16;
		for (ZmbHotspot &hs : hotspots) {
			hs._shapeIdx += addShapeNum;
		}
		return;
	}

	const uint32 routeHotspotCount = kResShapeRouteMontDespairP3_32 - kResShapeRouteBigBadHungryP0_17 + 1;
	if (hotspots.size() < routeHotspotCount) {
		error("rodmap: route SCRB frame has %u hotspots, expected at least %u", hotspots.size(), routeHotspotCount);
		return;
	}
	for (uint32 hotspotOffset = 0; hotspotOffset < routeHotspotCount; hotspotOffset++) {
		const uint16 hotspotIdx = static_cast<uint16>(kResShapeRouteBigBadHungryP0_17 + hotspotOffset);
		ZmbHotspot &hs = hotspots[hotspotOffset];

		// Hide route segments that have not been discovered.
		if (_pageRouteLevelMap[hotspotIdx] == 0) {
			hs._shapeIdx = ZmbHotspot::kShapeNone;
			continue;
		}
		// Determine the shapeId based on the route level
		hs._shapeIdx = hotspotIdx + (_pageRouteLevelMap[hotspotIdx] - 1) * 16;
	}
}

bool ZoombiniInteractiveRodMap::drawAfterPageIconHover1005_preRender(ZmbFeature *feature) {
	(void)feature;

	// Only drawn if a pageIcon (SCRB 1000) has been hovered.
	return _lastHoveredPageType != ZoombiniPageType::kNone;
}

ZmbRenderResult ZoombiniInteractiveRodMap::renderAfterPageIconHover1005(ZmbFeature *feature) {
	// Gate both shape blitting and text drawing on the hover rectangle,
	// which is set by the hover handler and cleared each frame.
	if (_lastHoveredPageType == ZoombiniPageType::kNone)
		return ZmbRenderResult::kSkipped;
	return blitShapes(feature);
}

void ZoombiniInteractiveRodMap::textPageName1005_postRender(ZmbFeature *feature) {
	if (_lastHoveredPageType == ZoombiniPageType::kNone)
		return;

	const Common::U32String &pageName = _vm->_text->getPageName(_lastHoveredPageType);

	ZmbDrawRecord *record = feature->getDrawRecord(0, 0);
	if (!record) {
		error("rodmap: page-name SCRB has no draw record");
		return;
	}
	Common::Rect textRect = record->_drawnRect;
	// The v1.11KR caption layout is shifted down by two pixels before centered rendering.
	if (_vm->isVersionFamilyUsV1() && _vm->getLanguage() == Common::KO_KOR) {
		textRect.top += 2;
		textRect.bottom += 2;
	}

	ZoombiniGraphics::TextConf tc;
	tc._textPalette = ZoombiniGraphics::kColor2D_Black;
	tc._hAlign = Graphics::kTextAlignCenter;
	tc._vAlign = Graphics::kTextAlignCenter;
	tc._wordWrap = _vm->isGameVariant(MohawkGameFeatures::GF_ZMB_10_ESPT) && _vm->getLanguage() == Common::PT_PRT;
	_vm->_gfx->drawText(ZoombiniGraphics::kShapeScreen, pageName, textRect, tc);
}

void ZoombiniInteractiveRodMap::optionButton1006_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)hsGroup;

	if (_optionButtonState.isAnimating()) {
		// This one-frame SCRB is inactive after its initial materialization.
		// Keep refreshing it until the page-owned press animation completes.
		feature->requestVisualRematerialization();
		uint32 elapsedFrames = _currentFrameCounter - _optionButtonState._animationStartFrame;
		if (elapsedFrames < _optionButtonState._animationFrameCount) {
			// SCRB 1007 uses shape 115 for TLC hover; shape 114 is an unrelated panel.
			// Use the explicit pressed shape 92 instead of decrementing the current shape.
			if (elapsedFrames < feature->getFrameInterval() - 1) {
				if (hotspots.empty()) {
					error("rodmap: option-button SCRB frame has no hotspots");
					return;
				}
				ZmbHotspot &hs = hotspots[0];
				hs._shapeIdx = _optionButtonState._shapePressedIdx;
				hs._x += 1;
				hs._y += 2;
			}
		} else {
			_optionButtonState._animationStartFrame = 0;
			_optionButtonState._firePostAnimationEvent = true;
		}
	}
}

void ZoombiniInteractiveRodMap::optionButton1006_postRender(ZmbFeature *feature) {
	(void)feature;

	if (!_optionButtonState._drawEnabled)
		return;

	optionButton1006_renderTlcLabel();

	if (!_optionButtonState._firePostAnimationEvent)
		return;
	_optionButtonState._firePostAnimationEvent = false;

	_vm->openOptionsDialog();
}

void ZoombiniInteractiveRodMap::optionButton1006_renderTlcLabel() {
	// TLC v2.0 draws a release-localized executable string over the button bitmap.
	if (!_vm->isVersionFamilyTlcV2())
		return;

	const Common::U32String optionStr = _vm->_text->getLocalizedString(ZoombiniText::kRodMapOptionsButton);

	ZoombiniGraphics::TextConf tc;
	tc._textPalette = ZoombiniGraphics::kColor2D_Black;
	tc._hAlign = Graphics::kTextAlignLeft;
	tc._vAlign = Graphics::kTextAlignStart;
	tc._wordWrap = false;
	_vm->_gfx->drawText(ZoombiniGraphics::kShapeScreen, optionStr, _tlcOptionButtonTextRect, tc);
}

void ZoombiniInteractiveRodMap::optionButton1006_updateTlcHover(const Common::Point &absPos) {
	// Z1-20U/TLC v2.0 release only: hover reloads SCRB 1007 onto the
	// existing options-button runner, then restores SCRB 1006 on leave.
	if (!_vm->isVersionFamilyTlcV2() || !_optionButtonFeature)
		return;

	const bool hovered = _optionButtonFeature->findDrawRecordAtPoint(absPos) != nullptr;
	if (_optionButtonTlcHovered == hovered)
		return;

	_optionButtonTlcHovered = hovered;
	loadScrbOntoFeature(_optionButtonFeature, hovered ? kResScrbMenuButtonHover1007 : kResScrbUsMenuButton1006);
}

ZmbEventHandleResult ZoombiniInteractiveRodMap::optionButton1006_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	(void)relPos;

	ZmbDrawRecord *drawRecord = feature->findDrawRecordAtPoint(absPos);
	if (!drawRecord)
		return ZmbEventHandleResult::kPassthrough;

	_optionButtonState.press(_vm, feature, _currentFrameCounter);
	return ZmbEventHandleResult::kConsumed;
}

void ZoombiniInteractiveRodMap::patchSelectedLevelShape1004_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)feature;
	(void)hsGroup;

	if (!_vm->_state->inPracticeMode())
		return;

	uint16 hsIdx = _vm->_state->_practiceLevel; // 1 ~ 4
	if (hotspots.size() <= hsIdx) {
		error("rodmap: selected-level hotspot %u is missing", hsIdx);
		return;
	}
	hotspots[hsIdx]._shapeIdx += 4;
	hotspots[hsIdx]._x -= 2;
	hotspots[hsIdx]._y -= 2;
}

void ZoombiniInteractiveRodMap::textLegend1004_postRender(ZmbFeature *feature) {
	(void)feature;

	ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;
	const bool espt10Layout = _vm->isGameVariant(MohawkGameFeatures::GF_ZMB_10_ESPT);
	const bool classicEuropeLayout = _vm->isVersionFamilyEuV1() && !espt10Layout;
	const bool es10Layout = espt10Layout && _vm->getLanguage() == Common::ES_ESP;
	const bool pt10Layout = espt10Layout && _vm->getLanguage() == Common::PT_PRT;
	const bool pl20Layout = _vm->isVersionFamilyTlcV2() && _vm->getLanguage() == Common::PL_POL;

	// Legend Title
	ZoombiniText::Key tKey = ZoombiniText::kTerrainKey;
	if (_vm->_state->inPracticeMode())
		tKey = ZoombiniText::kChooseLevel;

	ZmbDrawRecord *record = feature->getDrawRecord(0, 0);
	Common::Rect legendTitleRect = record->_drawnRect;
	if (classicEuropeLayout) {
		// SCRB 1003 in MAP.MHK has a 38-pixel title area above the four level rows.
		legendTitleRect.top -= 3;
		legendTitleRect.setHeight(38);
	} else if (pt10Layout) {
		legendTitleRect.top += 2;
		legendTitleRect.setHeight(23);
	} else {
		legendTitleRect.top += 3;
		legendTitleRect.setHeight(es10Layout || pl20Layout ? 18 : 15);
	}

	ZoombiniGraphics::TextConf ttc;
	ttc._hAlign = Graphics::kTextAlignCenter;
	ttc._vAlign = Graphics::kTextAlignCenter;
	ttc._wordWrap = true; // v1.xGE releases use a two-line title, so enable word wrap.
	_vm->_gfx->drawText(screenKind, tKey, legendTitleRect, ttc);

	// Level Descriptions
	Common::Rect levelRect = record->_drawnRect;
	levelRect.top += classicEuropeLayout ? 33 : 22;
	levelRect.setHeight(14);
	levelRect.left += 36;
	if (classicEuropeLayout)
		levelRect.right += 25;
	for (uint32 i = 0; i < 4; i++) {
		ZoombiniText::Key lKey = static_cast<ZoombiniText::Key>(ZoombiniText::kLevel1 + i);

		ZoombiniGraphics::TextConf ltc;
		ltc._hAlign = Graphics::kTextAlignLeft;
		// Polish TLC labels wrap and remain clipped to the authored row rectangle.
		ltc._wordWrap = pt10Layout || pl20Layout;

		// Use color on selected practice level
		const bool hasSelectedLevel = 1 <= _vm->_state->_practiceLevel && _vm->_state->_practiceLevel <= 4;
		ltc._outlineEffect = hasSelectedLevel && _vm->_state->_practiceLevel - 1u == i;
		if (1 <= _vm->_state->_practiceLevel && _vm->_state->_practiceLevel <= 4)
			ltc._outlinePalette = _levelLegendPalettes[_vm->_state->_practiceLevel - 1u];
		if (hasSelectedLevel && _vm->_state->_practiceLevel - 1u == i)
			ltc._textPalette = ZoombiniGraphics::kColor2D_Black;

		_vm->_gfx->drawText(screenKind, lKey, levelRect, ltc);

		// For next level
		levelRect.top += 14;
		levelRect.bottom += 14;
	}
}

ZmbEventHandleResult ZoombiniInteractiveRodMap::legendLevel1004_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	(void)relPos;

	for (uint32 i = 0; i < ARRAYSIZE(_levelLegendClickRects); i++) {
		if (!getLevelLegendClickRect(i).contains(absPos))
			continue;

		// Only in practice mode
		if (!_vm->_state->inPracticeMode())
			continue;

		if (4 <= i) {
			error("rodmap: invalid onClickIdx idx(%u)", i);
			return ZmbEventHandleResult::kPassthrough;
		}

		selectPracticeLevel(feature, static_cast<uint16>(i + 1));
		return ZmbEventHandleResult::kConsumed;
	}

	return ZmbEventHandleResult::kPassthrough;
}

/**
 * Keyboard shortcuts for level selection in practice mode: Ctrl+1 ~ Ctrl+4 to select levels 1 to 4.
 * Only works when already in practice mode, does not toggle practice mode on.
 */
ZmbEventHandleResult ZoombiniInteractiveRodMap::legendLevel1004_onKeyDown(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat) {
	if (kbdRepeat)
		return ZmbEventHandleResult::kPassthrough;

	if (!kbd.hasFlags(0))
		return ZmbEventHandleResult::kPassthrough;

	if (_vm->_state->_practiceLevel == 0)
		return ZmbEventHandleResult::kPassthrough;

	uint16 selectedLevel = 0;
	switch (kbd.keycode) {
	case Common::KEYCODE_1:
	case Common::KEYCODE_KP1:
		selectedLevel = 1;
		break;
	case Common::KEYCODE_2:
	case Common::KEYCODE_KP2:
		selectedLevel = 2;
		break;
	case Common::KEYCODE_3:
	case Common::KEYCODE_KP3:
		selectedLevel = 3;
		break;
	case Common::KEYCODE_4:
	case Common::KEYCODE_KP4:
		selectedLevel = 4;
		break;
	default:
		return ZmbEventHandleResult::kPassthrough;
	}

	if (_vm->_state->_practiceLevel == selectedLevel)
		return ZmbEventHandleResult::kPassthrough;

	selectPracticeLevel(feature, selectedLevel);
	return ZmbEventHandleResult::kConsumed;
}

void ZoombiniInteractiveRodMap::textJourneyStat1002_postRender(ZmbFeature *feature) {
	ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;
	ZmbDrawRecord *record = feature->getDrawRecord(0, 0);
	const bool espt10Layout = _vm->isGameVariant(MohawkGameFeatures::GF_ZMB_10_ESPT);
	const bool classicEuropeLayout = _vm->isVersionFamilyEuV1() && !espt10Layout;
	const bool pt10Layout = espt10Layout && _vm->getLanguage() == Common::PT_PRT;
	const bool pl20Layout = _vm->isVersionFamilyTlcV2() && _vm->getLanguage() == Common::PL_POL;

	if (classicEuropeLayout || espt10Layout) {
		const bool fixedDescriptionRows = espt10Layout;
		const int16 titleTopOffset = pt10Layout ? 2 : 3;
		const int16 titleHeight = pt10Layout ? 23 : 18;
		const int16 descriptionTopOffset = pt10Layout ? 20 : 26;
		const int16 descriptionRowHeight = pt10Layout ? 23 : 18;
		const int16 descriptionWidth = pt10Layout ? 120 : 115;

		Common::Rect titleRect = record->_drawnRect;
		titleRect.top += titleTopOffset;
		titleRect.setHeight(titleHeight);

		ZoombiniGraphics::TextConf titleConf;
		titleConf._hAlign = Graphics::kTextAlignCenter;
		titleConf._vAlign = Graphics::kTextAlignCenter;
		titleConf._wordWrap = pt10Layout;

		Common::Rect descriptionRect = record->_drawnRect;
		descriptionRect.top += descriptionTopOffset;
		descriptionRect.setHeight(descriptionRowHeight);

		if (_vm->_state->inPracticeMode()) {
			_vm->_gfx->drawText(screenKind, ZoombiniText::kPracticeMode, titleRect, titleConf);

			// Select one of the four four-line return-path blocks in TEXTSTR.MHK STRL 2002.
			ZoombiniText::Key firstStringKey;
			if ((_vm->_state->_f._levelFlagRouteMontDespair & 0x0F) != 0)
				firstStringKey = ZoombiniText::kEuropePracticeReturnZoombiniton1;
			else if (_vm->_state->_f._levelFlagLoWhosBayouHiDeepDarkForest != 0)
				firstStringKey = ZoombiniText::kEuropePracticeReturnShadeTree1;
			else if ((_vm->_state->_f._levelFlagRouteBigBadHungry & 0x0F) != 0)
				firstStringKey = ZoombiniText::kEuropePracticeReturnShelterRock1;
			else
				firstStringKey = ZoombiniText::kEuropePracticeReturnInitial1;

			descriptionRect.left += 5;
			for (uint16 i = 0; i < 4; i++) {
				ZoombiniGraphics::TextConf descConf;
				descConf._hAlign = Graphics::kTextAlignLeft;
				descConf._wordWrap = pt10Layout || (i == 3);
				const ZoombiniText::Key descriptionKey = static_cast<ZoombiniText::Key>(firstStringKey + i);
				_vm->_gfx->drawText(screenKind, descriptionKey, descriptionRect, descConf);
				// The Spanish and Portuguese v1.0 releases keep each description row at a fixed height.
				// The other European releases grow the rectangle by 20 pixels per row.
				descriptionRect.top += descriptionRowHeight;
				if (fixedDescriptionRows)
					descriptionRect.bottom += descriptionRowHeight;
				else
					descriptionRect.bottom += 38;
			}
		} else {
			_vm->_gfx->drawText(screenKind, _vm->_state->getActiveSaveName(), titleRect, titleConf);

			descriptionRect.left += 7;
			descriptionRect.setWidth(descriptionWidth);
			Common::Rect valueRect = descriptionRect;
			valueRect.left += descriptionWidth;
			valueRect.right = record->_drawnRect.right - 7;
			const int16 storedBasecamp1 = _vm->_state->_f._zmbStoredBC1Count;
			const int16 storedBasecamp2 = _vm->_state->_f._zmbStoredBC2Count;
			const int16 storedTown = _vm->_state->_f._zmbStoredTownCount;
			const ZoombiniText::Key descriptionKeys[4] = {
				ZoombiniText::kEuropeJourneyStatZoombiniIsle,
				ZoombiniText::kEuropeJourneyStatShelterRock,
				ZoombiniText::kEuropeJourneyStatShadeTree,
				ZoombiniText::kEuropeJourneyStatZoombiniton};
			const int16 values[4] = {static_cast<int16>(625 - (storedBasecamp1 + storedBasecamp2 + storedTown)), storedBasecamp1, storedBasecamp2, storedTown};
			for (uint16 i = 0; i < 4; i++) {
				ZoombiniGraphics::TextConf descConf;
				descConf._hAlign = Graphics::kTextAlignLeft;
				descConf._wordWrap = pt10Layout;
				_vm->_gfx->drawText(screenKind, descriptionKeys[i], descriptionRect, descConf);

				ZoombiniGraphics::TextConf valueConf;
				valueConf._hAlign = Graphics::kTextAlignRight;
				valueConf._wordWrap = false;
				_vm->_gfx->drawText(screenKind, Common::U32String::format("%d", values[i]), valueRect, valueConf);

				descriptionRect.top += descriptionRowHeight;
				descriptionRect.bottom += descriptionRowHeight;
				valueRect.top += descriptionRowHeight;
				valueRect.bottom += descriptionRowHeight;
			}
		}
	} else {
		// Stat Title
		Common::U32String titleStr;
		ZoombiniText::Key descKey[4];
		int16 descVal[4];
		if (_vm->_state->inPracticeMode()) {
			titleStr = _vm->_text->getLocalizedString(ZoombiniText::kPracticeTitle);
			descKey[0] = ZoombiniText::kPracticeDesc1;
			descKey[1] = ZoombiniText::kPracticeDesc2;
			descKey[2] = ZoombiniText::kPracticeDesc3;
			descKey[3] = ZoombiniText::kPracticeDesc4;
		} else {
			titleStr = _vm->_state->getActiveSaveName();
			if (pl20Layout) {
				descKey[0] = ZoombiniText::kPolishJourneyStatPicker;
				descKey[1] = ZoombiniText::kPolishJourneyStatBasecamp1;
				descKey[2] = ZoombiniText::kPolishJourneyStatBasecamp2;
				descKey[3] = ZoombiniText::kPolishJourneyStatTown;
			} else {
				descKey[0] = ZoombiniText::kPicker;
				descKey[1] = ZoombiniText::kBasecamp1;
				descKey[2] = ZoombiniText::kBasecamp2;
				descKey[3] = ZoombiniText::kTown;
			}
			descVal[1] = _vm->_state->_f._zmbStoredBC1Count;
			descVal[2] = _vm->_state->_f._zmbStoredBC2Count;
			descVal[3] = _vm->_state->_f._zmbStoredTownCount;
			descVal[0] = 625 - (descVal[1] + descVal[2] + descVal[3]);
		}

		Common::Rect titleRect = record->_drawnRect;
		titleRect.top += 3;
		titleRect.setHeight(pl20Layout ? 18 : 15);

		ZoombiniGraphics::TextConf ttc;
		ttc._hAlign = Graphics::kTextAlignCenter;
		ttc._vAlign = Graphics::kTextAlignCenter;
		ttc._wordWrap = pl20Layout;
		_vm->_gfx->drawText(screenKind, titleStr, titleRect, ttc);

		// Stat Descriptions
		Common::Rect descStrRect = record->_drawnRect;
		descStrRect.top += 26;
		descStrRect.setHeight(18);
		descStrRect.left += 7;
		descStrRect.setWidth(115);
		Common::Rect descValRect = descStrRect;
		descValRect.left += 115;
		descValRect.right = record->_drawnRect.right - 7;
		for (uint32 i = 0; i < 4; i++) {
			ZoombiniGraphics::TextConf dktc;
			dktc._hAlign = Graphics::kTextAlignLeft;
			// Polish TLC labels wrap and remain clipped before the numeric column.
			dktc._wordWrap = pl20Layout || (i == 3);
			_vm->_gfx->drawText(screenKind, descKey[i], descStrRect, dktc);

			if (!_vm->_state->inPracticeMode()) {
				Common::U32String descValStr = Common::U32String::format("%d", descVal[i]);
				ZoombiniGraphics::TextConf dvtc;
				dvtc._hAlign = Graphics::kTextAlignRight;
				dvtc._wordWrap = false;
				_vm->_gfx->drawText(screenKind, descValStr, descValRect, dvtc);
			}

			descStrRect.top += 18;
			descStrRect.bottom += 18;
			descValRect.top += 18;
			descValRect.bottom += 18;
		}

		// Combobox text ("Practice Mode" / "Continue Journey") is drawn here together with the stat panel,
		// not in SCRB 1003's postRender.
		Common::Rect comboPracticeRect = Common::Rect(50, 135, 197, 152);
		ZoombiniGraphics::TextConf cptc;
		cptc._hAlign = Graphics::kTextAlignLeft;
		cptc._wordWrap = false;
		_vm->_gfx->drawText(screenKind, ZoombiniText::kPracticeMode, comboPracticeRect, cptc);

		Common::Rect comboJourneyRect = Common::Rect(50, 154, 197, 176);
		ZoombiniGraphics::TextConf cjtc;
		cjtc._hAlign = Graphics::kTextAlignLeft;
		cjtc._wordWrap = false;
		_vm->_gfx->drawText(screenKind, ZoombiniText::kContinueJourney, comboJourneyRect, cjtc);
	}
}

void ZoombiniInteractiveRodMap::drawComboBox1003_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	// Hotspot Index
	// 1: Practice Mode + Outline (112)
	// 2: Practice Mode + Red Circle (111)
	// 3: Journey Mode + Outline (112)
	// 4: Journey Mode + Red Circle (111)
	if (!feature || !hsGroup || hotspots.size() < 4) {
		error("rodmap: malformed mode-selector SCRB frame with %u hotspots", hotspots.size());
		return;
	}

	if (_vm->_state->inPracticeMode()) {
		hotspots[4 - 1]._shapeIdx = ZmbHotspot::kShapeNone;
	} else {
		hotspots[2 - 1]._shapeIdx = ZmbHotspot::kShapeNone;
	}
}

ZmbEventHandleResult ZoombiniInteractiveRodMap::selectMode1003_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	(void)feature;
	(void)relPos;

	for (uint32 i = 0; i < ARRAYSIZE(_modeSelectClickRects); i++) {
		if (!_modeSelectClickRects[i].contains(absPos))
			continue;

		switch (i) {
		case 0:
			setPracticeMode(true);
			return ZmbEventHandleResult::kConsumed;
		case 1:
			setPracticeMode(false);
			return ZmbEventHandleResult::kConsumed;
		}
	}

	return ZmbEventHandleResult::kPassthrough;
}

void ZoombiniInteractiveRodMap::textRouteNames_postRender(ZmbFeature *feature) {
	(void)feature;

	ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;
	const uint16 firstPuzzleShape[] = {
		kResShapeBridge01,
		kResShapeFerry05,
		kResShapeFleens08,
		kResShapeCaves12,
	};

	for (uint32 shapeIdx = static_cast<uint32>(ZmbRouteId::kBigBadHungry); shapeIdx <= static_cast<uint32>(ZmbRouteId::kMontDespair); shapeIdx++) {
		bool isRouteVisited = false;
		for (uint16 pageShape = firstPuzzleShape[shapeIdx]; pageShape < firstPuzzleShape[shapeIdx] + 3; pageShape++) {
			if (_pageNodeLevelMap[pageShape] != 0) {
				isRouteVisited = true;
				break;
			}
		}
		if (!isRouteVisited)
			continue;

		const Common::Rect &textRect = getRouteNameRect(shapeIdx);
		ZoombiniText::Key textKey = _routeNameTextKey[shapeIdx];

		ZoombiniGraphics::TextConf tc;
		tc._outlineEffect = true;
		tc._outlinePalette = ZoombiniGraphics::kColor2D_Black;
		tc._textPalette = 0x0A;
		tc._hAlign = Graphics::kTextAlignCenter;
		tc._vAlign = Graphics::kTextAlignCenter;
		tc._wordWrap = true;
		_vm->_gfx->drawText(screenKind, textKey, textRect, tc);
	}
}

void ZoombiniInteractiveRodMap::buildPageRouteLevelMap() {
	uint16 pageLevelValues[17] = {};
	for (uint16 shapeId = kResShapeBridge01; shapeId <= kResShapePicker16; shapeId++)
		_pageNodeLevelMap[shapeId] = 0;

	if (_vm->_state->inPracticeMode()) {
		for (uint32 i = 0; i < ARRAYSIZE(pageLevelValues); i++)
			pageLevelValues[i] = _vm->_state->_practiceLevel;
	} else {
		const ZmbStateFile &state = _vm->_state->_f;

		// The first progress byte has no route-vertex counterpart.
		// Route vertices begin at the following Bridge byte.
		pageLevelValues[0] = 1;

		uint16 routeLevel = _vm->_state->readRouteLevel(ZmbRouteId::kBigBadHungry);
		if (0 < routeLevel) {
			for (uint16 valueIndex = 1; valueIndex <= 4; valueIndex++)
				pageLevelValues[valueIndex] = routeLevel + 1;
		} else {
			pageLevelValues[1] = getPuzzleLevel(state, 3);
			pageLevelValues[2] = getPuzzleLevel(state, 4);
			pageLevelValues[3] = getPuzzleLevel(state, 5);
			pageLevelValues[4] = state._levelFlagRouteBigBadHungry & 0x0F;
		}

		routeLevel = _vm->_state->readRouteLevel(ZmbRouteId::kWhosBayou);
		if (0 < routeLevel) {
			for (uint16 valueIndex = 5; valueIndex <= 7; valueIndex++)
				pageLevelValues[valueIndex] = routeLevel + 1;
			pageLevelValues[11] = routeLevel + 1;
		} else {
			pageLevelValues[5] = getPuzzleLevel(state, 6);
			pageLevelValues[6] = getPuzzleLevel(state, 7);
			pageLevelValues[7] = getPuzzleLevel(state, 8);
			pageLevelValues[11] = state._levelFlagLoWhosBayouHiDeepDarkForest & 0x0F;
		}

		routeLevel = _vm->_state->readRouteLevel(ZmbRouteId::kDeepDarkForest);
		if (0 < routeLevel) {
			for (uint16 valueIndex = 8; valueIndex <= 10; valueIndex++)
				pageLevelValues[valueIndex] = routeLevel + 1;
			pageLevelValues[16] = routeLevel + 1;
		} else {
			pageLevelValues[8] = getPuzzleLevel(state, 9);
			pageLevelValues[9] = getPuzzleLevel(state, 10);
			pageLevelValues[10] = getPuzzleLevel(state, 11);
			pageLevelValues[16] = (state._levelFlagLoWhosBayouHiDeepDarkForest & 0xF0) >> 4;
		}

		routeLevel = _vm->_state->readRouteLevel(ZmbRouteId::kMontDespair);
		if (0 < routeLevel) {
			for (uint16 valueIndex = 12; valueIndex <= 15; valueIndex++)
				pageLevelValues[valueIndex] = routeLevel + 1;
		} else {
			pageLevelValues[12] = getPuzzleLevel(state, 12);
			pageLevelValues[13] = getPuzzleLevel(state, 13);
			pageLevelValues[14] = getPuzzleLevel(state, 14);
			pageLevelValues[15] = state._levelFlagRouteMontDespair & 0x0F;
		}
	}

	static constexpr uint16 vertexValueIndex[] = {
		1,
		2,
		3,
		4,
		5,
		6,
		7,
		11,
		8,
		9,
		10,
		16,
		12,
		13,
		14,
		15,
	};
	for (uint16 shapeId = kResShapeRouteBigBadHungryP0_17; shapeId <= kResShapeRouteMontDespairP3_32; shapeId++)
		_pageRouteLevelMap[shapeId] = pageLevelValues[vertexValueIndex[shapeId - kResShapeRouteBigBadHungryP0_17]];

	for (uint16 shapeId = kResShapeBridge01; shapeId <= kResShapeNet10; shapeId++)
		_pageNodeLevelMap[shapeId] = pageLevelValues[shapeId];
	_pageNodeLevelMap[kResShapeBcTwo11] = pageLevelValues[11] != 0 || pageLevelValues[16] != 0;
	for (uint16 shapeId = kResShapeCaves12; shapeId <= kResShapeTown15; shapeId++)
		_pageNodeLevelMap[shapeId] = pageLevelValues[shapeId];
}

uint16 ZoombiniInteractiveRodMap::getPuzzleLevel(const ZmbStateFile &state, uint16 pageFlagIndex) {
	return static_cast<uint16>(state._pageLevelFlags[pageFlagIndex] & 0x0F);
}

} // End of namespace Mohawk
