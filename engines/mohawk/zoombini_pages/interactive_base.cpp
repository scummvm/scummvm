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

#include "mohawk/zoombini.h"
#include "mohawk/zoombini_graphics.h"
#include "mohawk/zoombini_pages/interactive_base.h"
#include "mohawk/zoombini_random.h"
#include "mohawk/zoombini_sound.h"
#include "mohawk/zoombini_state.h"

namespace Mohawk {

constexpr int16 ZoombiniInteractive::kAmbientPoolBC1[15];
constexpr int16 ZoombiniInteractive::kAmbientPoolBC2[10];
constexpr int16 ZoombiniInteractive::kAmbientPoolBridge[9];
constexpr int16 ZoombiniInteractive::kAmbientPoolTunnels[9];
constexpr int16 ZoombiniInteractive::kAmbientPoolPizza[12];
constexpr int16 ZoombiniInteractive::kAmbientPoolFerry[19];
constexpr int16 ZoombiniInteractive::kAmbientPoolLilly[20];
constexpr int16 ZoombiniInteractive::kAmbientPoolSlides[13];
constexpr int16 ZoombiniInteractive::kAmbientPoolFleens[13];
constexpr int16 ZoombiniInteractive::kAmbientPoolNet[17];
constexpr int16 ZoombiniInteractive::kAmbientPoolCaves[10];
constexpr int16 ZoombiniInteractive::kAmbientPoolSmoke[10];
constexpr int16 ZoombiniInteractive::kAmbientPoolMaze[10];

ZoombiniInteractive::ZoombiniInteractive(MohawkEngine_Zoombini *vm, ZoombiniPageType pageType) : ZoombiniPage(vm, ZoombiniPageCategory::kInteractive, pageType) {
	// Default arrivals settle facing right; page-specific initializers such as Picker may override this direction.
	_vm->setArrivalTurnDirection(ArrivalTurnDirection::kRight);
	_useFadeEffect = true;
	// Keep the first ambient pick away from page-entry and arrival sounds.
	_ambientNextPlayFrame = _currentFrameCounter + 900;

	_goMapButtonStateMap[kThreeButtons_Go] = ButtonState();
	_goMapButtonStateMap[kThreeButtons_SecondGo] = ButtonState();
	_goMapButtonStateMap[kThreeButtons_Map] = ButtonState();
	_helpButtonStateMap[kThreeButtons_Help] = ButtonState();
}

ZoombiniInteractive::~ZoombiniInteractive() {
	_vm->_midi->stop();
	_vm->_sound->releaseAllLoadedSounds();

	_vm->clearPageArchives();
}

ZmbEventHandleResult ZoombiniInteractive::onKeyDown(const Common::KeyState &kbd, bool kbdRepeat) {
	if (kbdRepeat)
		return ZmbEventHandleResult::kConsumed;

	// The serialized flag gates only keyboard dispatch. Console builtin_debug
	// and page builtin_debug actions call their typed handlers directly.
	if (_vm->isBuiltinDebugMode() || kbd.keycode == Common::KEYCODE_F1) {
		const ZmbEventHandleResult debugResult = onDebugKeyDown(kbd);
		if (debugResult == ZmbEventHandleResult::kConsumed)
			return debugResult;
	}

	// Page and feature handlers own the key before the shared dispatcher.
	ZmbEventHandleResult result = ZoombiniPage::onKeyDown(kbd, kbdRepeat);
	if (result == ZmbEventHandleResult::kConsumed)
		return result;

	result = ZmbEventHandleResult::kConsumed;
	if (kbd.hasFlags(Common::KBD_CTRL)) {
		switch (kbd.keycode) {
		case Common::KEYCODE_p: // Map button (CTRL-P) - ScummVM addition
			onMapButtonActivated();
			break;
		case Common::KEYCODE_n: // New Game (CTRL-N)
			_vm->_state->startNewGame(true);
			break;
		case Common::KEYCODE_l: // Load Game (CTRL-L)
			_vm->openLoadDialogWithSavePrompt();
			break;
		case Common::KEYCODE_s: // Save Game (CTRL-S)
			_vm->openSaveDialog();
			break;
		case Common::KEYCODE_q: // Quit (CTRL-Q)
			Engine::quitGame();
			break;
		case Common::KEYCODE_d: // Dialog & Sound Effects (CTRL-D)
			_vm->_state->toggleSound();
			break;
		case Common::KEYCODE_b: // Background Music (CTRL-B)
			_vm->_state->toggleMusic();
			break;
		case Common::KEYCODE_j: // Sticky Mouse (CTRL-J)
			_vm->_state->toggleStickyMouse();
			break;
		case Common::KEYCODE_u: // Auto-Sticky Mouse (CTRL-U)
			_vm->_state->toggleAutoStickyMouse();
			break;
		case Common::KEYCODE_t: // Transition (CTRL-T)
			_vm->_state->toggleTransitions();
			break;
		case Common::KEYCODE_a: // Help Audio (CTRL-A, v2.0US only)
			if (_vm->isGameVariant(GF_ZMB_20_US))
				_vm->_state->toggleHelpAudio();
			else
				result = ZmbEventHandleResult::kPassthrough;
			break;
		case Common::KEYCODE_k: // TouchSense (CTRL-K, v2.0US only)
			if (_vm->isGameVariant(GF_ZMB_20_US))
				_vm->_state->toggleTouchSense();
			else
				result = ZmbEventHandleResult::kPassthrough;
			break;
		case Common::KEYCODE_g: // Less/More Action (CTRL-G)
			_vm->_state->toggleLessMoreAction();
			break;
		case Common::KEYCODE_h: // Hide/Show Cursor (CTRL-H)
			_vm->_state->toggleCursorVisibility();
			break;
		default:
			result = ZmbEventHandleResult::kPassthrough;
			break;
		}
	} else {
		switch (kbd.keycode) {
		case Common::KEYCODE_SLASH:
		case Common::KEYCODE_QUESTION: // Options Dialog
			if (!_vm->hasDialogOpened())
				_vm->openOptionsDialog();
			break;
		case Common::KEYCODE_F1: // Play help sound
			debugReplayActiveHelpSound();
			break;
		default:
			result = ZmbEventHandleResult::kPassthrough;
			break;
		}
	}

	if (result == ZmbEventHandleResult::kConsumed)
		return result;

	if (_vm->handleBuiltinCheatSpace(kbd))
		return ZmbEventHandleResult::kConsumed;

	if (_vm->isBuiltinDebugMode())
		return onGlobalDebugKeyDown(kbd);
	return ZmbEventHandleResult::kPassthrough;
}

Common::String ZoombiniInteractive::debugGetGlobalDebugCommandHelp() const {
	Common::String output;
	output += Common::String::format("  %s | %s\n", kGlobalDebugActionEnable, kGlobalDebugActionDisable);
	output += "    Enable the original serialized flag or use its ScummVM disable counterpart.\n";
	output += Common::String::format("  %s ([)\n", kGlobalDebugActionStepOff);
	output += "    Resume normal continuous screen rendering by turning Step Mode off.\n";
	output += Common::String::format("  %s (])\n", kGlobalDebugActionStep);
	output += "    Stop normal continuous screen rendering by turning Step Mode on; later presses advance one pass or runner.\n";
	output += Common::String::format("  %s (*)\n", kGlobalDebugActionResetFidget);
	output += "    Reset the fidget interval threshold.\n";
	output += Common::String::format("  %s (Shift+N)\n", kGlobalDebugActionPaths);
	output += "    Draw NODE points and the next PATH diagnostic.\n";
	output += Common::String::format("  %s (Shift+S)\n", kGlobalDebugActionSound);
	output += "    Toggle queued-sound start notifications.\n";
	output += Common::String::format("  %s (&)\n", kGlobalDebugActionPalette);
	output += "    Draw the 256-entry palette grid.\n";
	output += Common::String::format("  %s (=)\n", kGlobalDebugActionMergeRect);
	output += "    Request a full-screen redraw by merging the whole screen into the dirty region.\n";
	output += Common::String::format("  %s (@)\n", kGlobalDebugActionUnlockRoutes);
	output += "    Apply the original route-unlock bits.\n";
	output += Common::String::format("  %s (Ctrl+E)\n", kGlobalDebugActionRunnerRectsAll);
	output += "    Draw all runner click rectangles, render-state signs, and global-list positions.\n";
	output += Common::String::format("  %s (Ctrl+F)\n", kGlobalDebugActionRunnerRectsFiltered);
	output += "    Draw Snoid-only runner click rectangles, render-state signs, and global-list positions.\n";
	output += Common::String::format("  %s (Ctrl+X)\n", kGlobalDebugActionRunnerIdsAll);
	output += "    Draw all runner click rectangles and IDs.\n";
	output += Common::String::format("  %s (Ctrl+Y)\n", kGlobalDebugActionRunnerIdsFiltered);
	output += "    Draw Snoid-only runner click rectangles and IDs.\n";
	output += Common::String::format("  %s (Ctrl+Z)\n", kGlobalDebugActionRunnerDebug);
	output += "    Toggle the palette-14 runner-debug backdrop.\n";
	output += Common::String::format("  %s (Ctrl+I)\n", kGlobalDebugActionForceParty);
	output += "    Force every available Zoombini into the active party.\n";
	output += Common::String::format("  %s (Ctrl+R)\n", kGlobalDebugActionDragCoords);
	output += "    Toggle dragged-Zoombini coordinate notifications.\n";
	return output;
}

bool ZoombiniInteractive::debugDoGlobalDebugCommand(int argc, const char **argv, Common::String &output) {
	if (argc != 2) {
		output = "Usage: builtin_debug <action>\n";
		output += debugGetGlobalDebugCommandHelp();
		return true;
	}

	const GlobalDebugAction action = parseGlobalDebugAction(argv[1]);
	if (action == GlobalDebugAction::kInvalid) {
		output = Common::String::format("Unknown built-in debug action '%s'.\n", argv[1]);
		output += debugGetGlobalDebugCommandHelp();
		return true;
	}
	return runGlobalDebugAction(action, output);
}

ZoombiniInteractive::GlobalDebugAction ZoombiniInteractive::parseGlobalDebugAction(const Common::String &action) {
	if (action.equalsIgnoreCase(kGlobalDebugActionEnable))
		return GlobalDebugAction::kEnable;
	if (action.equalsIgnoreCase(kGlobalDebugActionDisable))
		return GlobalDebugAction::kDisable;
	if (action.equalsIgnoreCase(kGlobalDebugActionStepOff))
		return GlobalDebugAction::kStepOff;
	if (action.equalsIgnoreCase(kGlobalDebugActionStep))
		return GlobalDebugAction::kStep;
	if (action.equalsIgnoreCase(kGlobalDebugActionResetFidget))
		return GlobalDebugAction::kResetFidget;
	if (action.equalsIgnoreCase(kGlobalDebugActionPaths))
		return GlobalDebugAction::kPaths;
	if (action.equalsIgnoreCase(kGlobalDebugActionSound))
		return GlobalDebugAction::kSound;
	if (action.equalsIgnoreCase(kGlobalDebugActionPalette))
		return GlobalDebugAction::kPalette;
	if (action.equalsIgnoreCase(kGlobalDebugActionMergeRect))
		return GlobalDebugAction::kMergeRect;
	if (action.equalsIgnoreCase(kGlobalDebugActionUnlockRoutes))
		return GlobalDebugAction::kUnlockRoutes;
	if (action.equalsIgnoreCase(kGlobalDebugActionRunnerRectsAll))
		return GlobalDebugAction::kRunnerRectsAll;
	if (action.equalsIgnoreCase(kGlobalDebugActionRunnerRectsFiltered))
		return GlobalDebugAction::kRunnerRectsFiltered;
	if (action.equalsIgnoreCase(kGlobalDebugActionRunnerIdsAll))
		return GlobalDebugAction::kRunnerIdsAll;
	if (action.equalsIgnoreCase(kGlobalDebugActionRunnerIdsFiltered))
		return GlobalDebugAction::kRunnerIdsFiltered;
	if (action.equalsIgnoreCase(kGlobalDebugActionRunnerDebug))
		return GlobalDebugAction::kRunnerDebug;
	if (action.equalsIgnoreCase(kGlobalDebugActionForceParty))
		return GlobalDebugAction::kForceParty;
	if (action.equalsIgnoreCase(kGlobalDebugActionDragCoords))
		return GlobalDebugAction::kDragCoords;
	return GlobalDebugAction::kInvalid;
}

bool ZoombiniInteractive::debugReplayActiveHelpSound() {
	if (!_activeHelpSoundId.hasId())
		return false;

	playActiveHelpSound();
	return true;
}

void ZoombiniInteractive::showBuiltinDebugText(const Common::String &text, bool waitForInput) {
	drawBuiltinDebugText(text, waitForInput);
	if (waitForInput)
		waitForBuiltinDebugInput();
}

bool ZoombiniInteractive::runGlobalDebugAction(GlobalDebugAction action, Common::String &output) {
	ZmbBuiltinDebugRuntimeState &debugState = _vm->_builtinDebug;

	if (action == GlobalDebugAction::kEnable || action == GlobalDebugAction::kDisable) {
		const bool enabled = action == GlobalDebugAction::kEnable;
		_vm->setBuiltinDebugMode(enabled);
		output = Common::String::format("Built-in debug mode %s.\n", enabled ? "enabled" : "disabled");
	} else if (action == GlobalDebugAction::kStepOff) {
		debugState._stepMode = false;
		debugState._stepAdvanceRequested = false;
		debugState._stepRunnerPosition = 0;
		showBuiltinDebugText("Step-Mode OFF");
		output = "Step-Mode OFF.\n";
	} else if (action == GlobalDebugAction::kStep) {
		if (!debugState._stepMode) {
			debugState._stepMode = true;
			debugState._stepAdvanceRequested = false;
			debugState._stepRunnerPosition = 0;
			showBuiltinDebugText("Step-Mode ON");
			output = "Step-Mode ON.\n";
		} else if (debugState._stepRunnerPosition == 0) {
			debugState._stepAdvanceRequested = true;
			output = "One complete render frame requested.\n";
		} else {
			debugState._stepRunnerPosition += 1;
			if (getBuiltinDebugRunnerCount() < static_cast<uint32>(debugState._stepRunnerPosition)) {
				debugState._stepRunnerPosition = 0;
				debugState._stepAdvanceRequested = true;
				output = "Runner traversal completed; one complete render frame requested.\n";
			} else {
				drawBuiltinRunnerClickRects(debugState._runnerFilterSnoids, debugState._runnerShowIds,
											debugState._stepRunnerPosition);
				output = Common::String::format("Drew runner position %d.\n", debugState._stepRunnerPosition);
			}
		}
	} else if (action == GlobalDebugAction::kResetFidget) {
		_vm->_fidgetThreshold = 0;
		output = "Original fidget threshold reset.\n";
	} else if (action == GlobalDebugAction::kPaths) {
		drawBuiltinNodePaths();
		output = Common::String::format("Drew NODE/PATH diagnostic index %u.\n", debugState._pathCycleIndex);
	} else if (action == GlobalDebugAction::kSound) {
		debugState._soundQueueNotifications = !debugState._soundQueueNotifications;
		const char *notificationState;
		if (debugState._soundQueueNotifications)
			notificationState = "on";
		else
			notificationState = "off";
		output = Common::String::format("Queued-SND notifications %s.\n", notificationState);
	} else if (action == GlobalDebugAction::kPalette) {
		drawBuiltinPaletteGrid();
		output = "Drew the 32 by 8 palette grid.\n";
	} else if (action == GlobalDebugAction::kMergeRect) {
		addExternalDirtyRect(Common::Rect(0, 0, ZoombiniGraphics::kScreenWidth, ZoombiniGraphics::kScreenHeight));
		output = "Requested a full-screen redraw by merging the whole screen into the external dirty region.\n";
	} else if (action == GlobalDebugAction::kUnlockRoutes) {
		_vm->_state->getCurrentState().setRouteCompletionFlag(ZmbRouteId::kBigBadHungry, 0);
		_vm->_state->getCurrentState().setRouteCompletionFlag(ZmbRouteId::kWhosBayou, 0);
		_vm->_state->getCurrentState().setRouteCompletionFlag(ZmbRouteId::kMontDespair, 0);
		_vm->_state->markDebugStateMutation();
		_vm->_state->markSaveBeforeQuitPending();
		output = "Built-in route unlock bits applied.\n";
	} else if (action == GlobalDebugAction::kRunnerRectsFiltered || action == GlobalDebugAction::kRunnerRectsAll) {
		debugState._runnerFilterSnoids = action == GlobalDebugAction::kRunnerRectsFiltered;
		debugState._runnerShowIds = false;
		debugState._stepRunnerPosition = debugState._stepMode ? 1 : 0;
		drawBuiltinRunnerClickRects(debugState._runnerFilterSnoids, false, debugState._stepRunnerPosition);
		output = "Drew original runner click-rectangle labels.\n";
	} else if (action == GlobalDebugAction::kRunnerIdsFiltered || action == GlobalDebugAction::kRunnerIdsAll) {
		debugState._runnerFilterSnoids = action == GlobalDebugAction::kRunnerIdsFiltered;
		debugState._runnerShowIds = true;
		debugState._stepRunnerPosition = debugState._stepMode ? 1 : 0;
		drawBuiltinRunnerClickRects(debugState._runnerFilterSnoids, true, debugState._stepRunnerPosition);
		output = "Drew original runner-ID labels.\n";
	} else if (action == GlobalDebugAction::kRunnerDebug) {
		debugState._runnerBackdropMode = !debugState._runnerBackdropMode;
		if (!debugState._runnerBackdropMode)
			addExternalDirtyRect(Common::Rect(0, 0, ZoombiniGraphics::kScreenWidth, ZoombiniGraphics::kScreenHeight));
		output = Common::String::format("Runner backdrop mode %s.\n", debugState._runnerBackdropMode ? "on" : "off");
	} else if (action == GlobalDebugAction::kForceParty) {
		uint16 snoidCount = 0;
		uint16 activePartyCount = 0;
		for (ZmbSnoid *snoid : _snoidMap) {
			if (!snoid || !snoid->isPackSnoid())
				continue;
			snoidCount += 1;
			if (snoid->isRenderActivated() && snoid->_packIsOccupied)
				activePartyCount += 1;
		}
		if (activePartyCount < snoidCount) {
			schedulePackSnoids(true, true);
			_vm->_state->markDebugStateMutation();
			_vm->_state->markSaveBeforeQuitPending();
		}
		showBuiltinDebugText("ALL in party");
		output = "ALL in party.\n";
	} else if (action == GlobalDebugAction::kDragCoords) {
		debugState._showDragCoordinates = !debugState._showDragCoordinates;
		updateBuiltinDragNotification();
		output = Common::String::format("Dragged-Snoid coordinates %s.\n", debugState._showDragCoordinates ? "on" : "off");
	} else {
		return true;
	}

	return false;
}

ZmbEventHandleResult ZoombiniInteractive::onGlobalDebugKeyDown(const Common::KeyState &kbd) {
	GlobalDebugAction action = GlobalDebugAction::kInvalid;
	if (kbd.hasFlags(Common::KBD_CTRL)) {
		switch (kbd.keycode) {
		case Common::KEYCODE_e:
			action = GlobalDebugAction::kRunnerRectsAll;
			break;
		case Common::KEYCODE_f:
			action = GlobalDebugAction::kRunnerRectsFiltered;
			break;
		case Common::KEYCODE_i:
			action = GlobalDebugAction::kForceParty;
			break;
		case Common::KEYCODE_r:
			action = GlobalDebugAction::kDragCoords;
			break;
		case Common::KEYCODE_x:
			action = GlobalDebugAction::kRunnerIdsAll;
			break;
		case Common::KEYCODE_y:
			action = GlobalDebugAction::kRunnerIdsFiltered;
			break;
		case Common::KEYCODE_z:
			action = GlobalDebugAction::kRunnerDebug;
			break;
		default:
			return ZmbEventHandleResult::kPassthrough;
		}
	} else if (kbd.hasFlags(0) || kbd.hasFlags(Common::KBD_SHIFT)) {
		switch (kbd.ascii) {
		case '[':
			action = GlobalDebugAction::kStepOff;
			break;
		case ']':
			action = GlobalDebugAction::kStep;
			break;
		case 'N':
			action = GlobalDebugAction::kPaths;
			break;
		case 'S':
			action = GlobalDebugAction::kSound;
			break;
		case '=':
			action = GlobalDebugAction::kMergeRect;
			break;
		case '&':
			action = GlobalDebugAction::kPalette;
			break;
		case '*':
			action = GlobalDebugAction::kResetFidget;
			break;
		case '@':
			action = GlobalDebugAction::kUnlockRoutes;
			break;
		default:
			return ZmbEventHandleResult::kPassthrough;
		}
	} else {
		return ZmbEventHandleResult::kPassthrough;
	}

	Common::String output;
	runGlobalDebugAction(action, output);
	return ZmbEventHandleResult::kConsumed;
}

void ZoombiniInteractive::continuousButton_selectShapes(ZmbFeature *feature, Common::Array<ZmbHotspot> &hotspots, Common::StableMap<uint32, ContinuousButtonState> &contButtonStateMap, uint16 pressedDeltaX, uint16 pressedDeltaY) {
	if (!feature) {
		error("interactive: continuous-button callback has no feature");
		return;
	}

	for (Common::StableMap<uint32, ContinuousButtonState>::iterator it = contButtonStateMap.begin(); it != contButtonStateMap.end(); it++) {
		ContinuousButtonState &cbs = it->second;

		if (!cbs._enabled)
			continue;
		if (hotspots.size() <= cbs._hsNormalIdx || hotspots.size() <= cbs._hsPressedIdx) {
			error("interactive: continuous-button SCRB is missing required hotspots");
			return;
		}

		ZmbHotspot &hsNormal = hotspots[cbs._hsNormalIdx];
		ZmbHotspot &hsPressed = hotspots[cbs._hsPressedIdx];
		const bool useHoverShape = _vm->isVersionFamilyTlcV2() && cbs.hasHoverState() && cbs._isHovered &&
								   cbs._shapeHoverId <= _vm->_gfx->getShapeCount(feature->getResource());

		if (useHoverShape) {
			// TLC v2 storage controls keep their hover bitmap visible while held.
			hsNormal._shapeIdx = cbs._shapeHoverId;
			hsPressed._shapeIdx = ZmbHotspot::kShapeNone;
		} else if (cbs._pressed) {
			hsNormal._shapeIdx = ZmbHotspot::kShapeNone;
			hsPressed._shapeIdx = cbs._shapePressedId;
			hsPressed._x += pressedDeltaX;
			hsPressed._y += pressedDeltaY;
		} else {
			hsNormal._shapeIdx = cbs._shapeNormalId;
			hsPressed._shapeIdx = ZmbHotspot::kShapeNone;
		}
	}
}

void ZoombiniInteractive::ContinuousButtonState::setHoverState(uint16 hoverShapeId) {
	_shapeHoverId = hoverShapeId;
}

bool ZoombiniInteractive::ContinuousButtonState::hasHoverState() const {
	return _shapeHoverId != ZmbHotspot::kShapeNone;
}

bool ZoombiniInteractive::ContinuousButtonState::setHovered(bool hovered) {
	if (_isHovered == hovered)
		return false;
	_isHovered = hovered;
	return true;
}

void ZoombiniInteractive::ContinuousButtonState::press() {
	_pressed = true;
}

void ZoombiniInteractive::ContinuousButtonState::release() {
	_pressed = false;
}

void ZoombiniInteractive::setGoButtonsEnabled(bool enabled) {
	const bool pressDisabled = !enabled;
	ButtonState &goButtonState = _goMapButtonStateMap[kThreeButtons_Go];
	ButtonState &secondGoButtonState = _goMapButtonStateMap[kThreeButtons_SecondGo];
	const bool changed = goButtonState._isPressDisabled != pressDisabled || secondGoButtonState._isPressDisabled != pressDisabled;

	goButtonState._isPressDisabled = pressDisabled;
	secondGoButtonState._isPressDisabled = pressDisabled;

	if (changed && _goMapButtonsFeature)
		_goMapButtonsFeature->requestVisualRematerialization();
}

void ZoombiniInteractive::setGoButton(const Common::Rect &rect, uint16 shapeDisabledId, uint16 shapeEnabledId, uint16 shapePressedId) {
	_goButtonRect = rect;
	_goButtonShapeDisabledId = shapeDisabledId;
	_goButtonShapeEnabledId = shapeEnabledId;
	_goButtonShapePressedId = shapePressedId;

	ZmbResource soundResId(ZmbResource::kSystem, kSysResSound0999_ButtonSFX);
	ButtonState goButtonState = ButtonState(soundResId, kHotspotGoButtonNormal, kHotspotGoButtonPressed, shapeEnabledId, shapePressedId);
	goButtonState.setDisabledState(shapeDisabledId);
	goButtonState._isPressDisabled = true;
	_goMapButtonStateMap[kThreeButtons_Go] = goButtonState;
	_threeButtonRectMap[kThreeButtons_Go] = rect;
}

void ZoombiniInteractive::setSecondGoButton(const Common::Rect &rect, uint16 shapeDisabledId, uint16 shapeEnabledId, uint16 shapePressedId) {
	_secondGoButtonRect = rect;
	_secondGoButtonShapeDisabledId = shapeDisabledId;
	_secondGoButtonShapeEnabledId = shapeEnabledId;
	_secondGoButtonShapePressedId = shapePressedId;

	ZmbResource soundResId(ZmbResource::kSystem, kSysResSound0999_ButtonSFX);
	ButtonState secondGoButtonState = ButtonState(soundResId, kHotspotSecondGoButtonNormal, kHotspotSecondGoButtonPressed, shapeEnabledId, shapePressedId);
	secondGoButtonState.setDisabledState(shapeDisabledId);
	secondGoButtonState._isPressDisabled = true;
	_goMapButtonStateMap[kThreeButtons_SecondGo] = secondGoButtonState;
	_threeButtonRectMap[kThreeButtons_SecondGo] = rect;
}

void ZoombiniInteractive::setMapButton(const Common::Rect &rect, uint16 shapeNormalId, uint16 shapePressedId) {
	_mapButtonRect = rect;
	_mapButtonShapeNormalId = shapeNormalId;
	_mapButtonShapePressedId = shapePressedId;

	ZmbResource soundResId(ZmbResource::kSystem, kSysResSound0999_ButtonSFX);
	ButtonState mapButtonState = ButtonState(soundResId, kHotspotMapButtonNormal, kHotspotMapButtonPressed, shapeNormalId, shapePressedId);
	_goMapButtonStateMap[kThreeButtons_Map] = mapButtonState;
	_threeButtonRectMap[kThreeButtons_Map] = rect;
}

void ZoombiniInteractive::setHelpButton(const Common::Rect &rect) {
	// Europe v1.x omits Help from both the common three-button layout and Basecamp 1's four-button variant.
	// Do not register a rect or shape state on Europe v1.x releases
	if (!_vm->supportsOnScreenHelp())
		return;

	_helpButtonRect = rect;

	ZmbResource soundResId(ZmbResource::kSystem, kSysResSound0999_ButtonSFX);
	ButtonState helpButtonState = ButtonState(soundResId, kHotspotHelpButtonNormal, kHotspotHelpButtonPressed, kSystemShape0001_24_HelpButtonNormal, kSystemShape0001_25_HelpButtonPressed);
	// v2.0US release only: the shared Help button has a hover shape.
	if (_vm->isVersionFamilyTlcV2())
		helpButtonState.setHoverState(kSystemShape0001_39_HelpButtonHover);
	_helpButtonStateMap[kThreeButtons_Help] = helpButtonState;
	_threeButtonRectMap[kThreeButtons_Help] = rect;
}

bool ZoombiniInteractive::isPointInControlButtonRect(const Common::Point &pos) const {
	return _goButtonRect.contains(pos) || _secondGoButtonRect.contains(pos) ||
		   _mapButtonRect.contains(pos) || _helpButtonRect.contains(pos);
}

void ZoombiniInteractive::loadGoMapButtonsFeature(int16 bitmapResId, bool separateBitmapResources) {
	_goMapBitmapResId = bitmapResId;
	_goMapButtonsUseSeparateBitmapResources = separateBitmapResources;
	configureTlcGoMapButtonHover(bitmapResId);

	// At least one of Go or Map button should be enabled to load the feature.
	if (!_goMapButtonStateMap[kThreeButtons_Map]._drawEnabled && !_goMapButtonStateMap[kThreeButtons_SecondGo]._drawEnabled && !_goMapButtonStateMap[kThreeButtons_Go]._drawEnabled)
		return;

	// Go/Map button shapes & hotspots are stored in page archives.
	// Derived class is responsible for setting proper shapes & hotspots.

	// [*] Callback-only runner - Go, Map Buttons
	// (overlay03) with preRender/postRender for proceed/map/help button drawing.
	ZmbFeature::EventHooks hooksGoMapButtons;
	if (_goMapButtonsUseSeparateBitmapResources)
		hooksGoMapButtons.setRenderFunc(static_cast<ZmbFeature::OnRenderFunc>(&ZoombiniInteractive::goMapButtons_renderSeparateBitmaps));
	hooksGoMapButtons.setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniInteractive::goMapButtons_preRenderShape));
	hooksGoMapButtons.setPostRenderFunc(static_cast<ZmbFeature::OnPostRenderFunc>(&ZoombiniInteractive::goMapButtons_onPostRender));
	hooksGoMapButtons.setLButtonDownFunc(static_cast<ZmbFeature::OnLButtonDownFunc>(&ZoombiniInteractive::goMapButtons_onLButtonDown));

	Common::Array<ZmbHotspot> hotspots;
	// Use the enabled shapes as the initial hotspot shape.
	// @ref ZoombiniPage::genericButton_selectShapes() overrides it
	// when @ref ZoombiniPage::ButtonState::_isPressDisabled is true.
	hotspots.push_back(ZmbHotspot(kHotspotGoButtonNormal, _goButtonShapeEnabledId, 0, _goButtonRect));
	hotspots.push_back(ZmbHotspot(kHotspotSecondGoButtonNormal, _secondGoButtonShapeEnabledId, 0, _secondGoButtonRect));
	hotspots.push_back(ZmbHotspot(kHotspotMapButtonNormal, _mapButtonShapeNormalId, 0, _mapButtonRect));
	hotspots.push_back(ZmbHotspot(kHotspotGoButtonPressed, _goButtonShapePressedId, 0, _goButtonRect));
	hotspots.push_back(ZmbHotspot(kHotspotSecondGoButtonPressed, _secondGoButtonShapePressedId, 0, _secondGoButtonRect));
	hotspots.push_back(ZmbHotspot(kHotspotMapButtonPressed, _mapButtonShapePressedId, 0, _mapButtonRect));
	_goMapButtonHotspots = hotspots;

	// Registered with FLAG_00001000_TOPMOST only (no OVERLAY).
	// TOPMOST -> normalList tail -> rendered last -> always on top of all features.
	if (_goMapButtonsUseSeparateBitmapResources) {
		_goMapButtonsFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, 0), 0, 0, ZmbFeature::FLAG_00001000_TOPMOST, hooksGoMapButtons);
		Common::Rect buttonsRect;
		for (Common::HashMap<uint32, Common::Rect>::const_iterator it = _threeButtonRectMap.begin(); it != _threeButtonRectMap.end(); it++) {
			if (buttonsRect.isEmpty())
				buttonsRect = it->_value;
			else
				buttonsRect.extend(it->_value);
		}
		_goMapButtonsFeature->setClickRect(buttonsRect);
		_goMapButtonsFeature->setSortRect(buttonsRect);
	} else {
		_goMapButtonsFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, _goMapBitmapResId), 0,
											   hotspots, 0,
											   ZmbFeature::FLAG_00001000_TOPMOST,
											   hooksGoMapButtons);
	}
}

void ZoombiniInteractive::setTlcButtonHoverIfPresent(ButtonState &buttonState, uint16 hoverShapeId, const ZmbResource &bitmapRes) {
	buttonState._shapeHoverIdx = ZmbHotspot::kShapeNone;
	buttonState._isHovered = false;
	if (hoverShapeId <= _vm->_gfx->getShapeCount(bitmapRes))
		buttonState.setHoverState(hoverShapeId);
}

void ZoombiniInteractive::configureTlcGoMapButtonHover(int16 bitmapResId) {
	// Z1-20U/TLC v2.0 release only: 1.x releases do not have yellow-outline Go/Map hover bitmap states.
	if (!_vm->isVersionFamilyTlcV2())
		return;

	const ZmbResource bitmapRes(ZmbResource::kPage, bitmapResId);
	if (getPageType() == ZoombiniPageType::kBasecamp2 && bitmapResId == kBasecamp2ButtonBitmapResId) {
		ButtonState &goButtonState = _goMapButtonStateMap[kThreeButtons_Go];
		if (goButtonState._shapeNormalIdx == kShapeBasecamp2GoButtonNormal &&
			goButtonState._shapePressedIdx == kShapeBasecamp2GoButtonPressed)
			goButtonState.setHoverState(kShapeBasecamp2GoButtonHover);

		ButtonState &mapButtonState = _goMapButtonStateMap[kThreeButtons_Map];
		if (mapButtonState._shapeNormalIdx == kShapeBasecamp2MapButtonNormal &&
			mapButtonState._shapePressedIdx == kShapeBasecamp2MapButtonPressed)
			mapButtonState.setHoverState(kShapeBasecamp2MapButtonHover);
		return;
	}

	if (getPageType() == ZoombiniPageType::kBasecamp1 && bitmapResId == kBasecamp1ButtonBitmapResId) {
		ButtonState &goButtonState = _goMapButtonStateMap[kThreeButtons_Go];
		if (goButtonState._shapeNormalIdx == kShapeBasecamp1GoRouteUpButtonNormal &&
			goButtonState._shapePressedIdx == kShapeBasecamp1GoRouteUpButtonPressed)
			setTlcButtonHoverIfPresent(goButtonState, kShapeBasecamp1GoRouteUpButtonHover, bitmapRes);

		ButtonState &secondGoButtonState = _goMapButtonStateMap[kThreeButtons_SecondGo];
		if (secondGoButtonState._shapeNormalIdx == kShapeBasecamp1GoRouteDownButtonNormal &&
			secondGoButtonState._shapePressedIdx == kShapeBasecamp1GoRouteDownButtonPressed)
			setTlcButtonHoverIfPresent(secondGoButtonState, kShapeBasecamp1GoRouteDownButtonHover, bitmapRes);

		ButtonState &mapButtonState = _goMapButtonStateMap[kThreeButtons_Map];
		if (mapButtonState._shapeNormalIdx == kShapeMapButtonNormal &&
			mapButtonState._shapePressedIdx == kShapeMapButtonPressed)
			setTlcButtonHoverIfPresent(mapButtonState, kShapeBasecamp1MapButtonHover, bitmapRes);
		return;
	}

	Common::StableMap<uint32, ButtonState>::iterator goIt = _goMapButtonStateMap.find(kThreeButtons_Go);
	if (goIt != _goMapButtonStateMap.end()) {
		ButtonState &goButtonState = goIt->second;
		if (goButtonState._shapeNormalIdx == kShapeGoButtonNormal && goButtonState._shapePressedIdx == kShapeGoButtonPressed)
			setTlcButtonHoverIfPresent(goButtonState, kShapeGoButtonHover, bitmapRes);
		else if (goButtonState._shapeNormalIdx == kShapePickerGoButtonNormal && goButtonState._shapePressedIdx == kShapePickerGoButtonPressed)
			setTlcButtonHoverIfPresent(goButtonState, kShapePickerGoButtonHover, bitmapRes);
	}

	Common::StableMap<uint32, ButtonState>::iterator secondGoIt = _goMapButtonStateMap.find(kThreeButtons_SecondGo);
	if (secondGoIt != _goMapButtonStateMap.end()) {
		ButtonState &secondGoButtonState = secondGoIt->second;
		if (secondGoButtonState._shapeNormalIdx == kShapeGoButtonNormal && secondGoButtonState._shapePressedIdx == kShapeGoButtonPressed)
			setTlcButtonHoverIfPresent(secondGoButtonState, kShapeGoButtonHover, bitmapRes);
	}

	Common::StableMap<uint32, ButtonState>::iterator mapIt = _goMapButtonStateMap.find(kThreeButtons_Map);
	if (mapIt != _goMapButtonStateMap.end()) {
		ButtonState &mapButtonState = mapIt->second;
		if (mapButtonState._shapeNormalIdx == kShapeMapButtonNormal && mapButtonState._shapePressedIdx == kShapeMapButtonPressed)
			setTlcButtonHoverIfPresent(mapButtonState, kShapeMapButtonHover, bitmapRes);
		else if (mapButtonState._shapeNormalIdx == kShapePickerMapButtonNormal && mapButtonState._shapePressedIdx == kShapePickerMapButtonPressed)
			setTlcButtonHoverIfPresent(mapButtonState, kShapePickerMapButtonHover, bitmapRes);
	}
}

void ZoombiniInteractive::loadHelpButtonFeature() {
	if (!_vm->supportsOnScreenHelp() || !_helpButtonStateMap[kThreeButtons_Help]._drawEnabled)
		return;

	// [*] Help button state
	// Help button shapes & hotspots are stored in a common archive, ZOOMBINI.MHK.
	// The in-game Help feature belongs to the 32-bit v1.1 line, so its shapes are not stored in the page archives.
	ZmbResource soundResId(ZmbResource::kSystem, kSysResSound0999_ButtonSFX);
	ButtonState helpButtonState = ButtonState(soundResId, kHotspotHelpButtonNormal, kHotspotHelpButtonPressed,
											  kSystemShape0001_24_HelpButtonNormal, kSystemShape0001_25_HelpButtonPressed);
	// v2.0US release only: the shared Help button has a hover shape.
	if (_vm->isVersionFamilyTlcV2())
		helpButtonState.setHoverState(kSystemShape0001_39_HelpButtonHover);
	_helpButtonStateMap[kThreeButtons_Help] = helpButtonState;

	// [*] Callback-only runner (tBMP c:0001) - Help Button
	ZmbFeature::EventHooks hooksHelpMapButton;
	hooksHelpMapButton.setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniInteractive::helpButton_preRenderShape));
	hooksHelpMapButton.setPostRenderFunc(static_cast<ZmbFeature::OnPostRenderFunc>(&ZoombiniInteractive::helpButton_onPostRender));
	hooksHelpMapButton.setLButtonDownFunc(static_cast<ZmbFeature::OnLButtonDownFunc>(&ZoombiniInteractive::helpButton_onLButtonDown));

	Common::Array<ZmbHotspot> hotspots;
	hotspots.push_back(ZmbHotspot(kHotspotHelpButtonNormal, kSystemShape0001_24_HelpButtonNormal, 0, _helpButtonRect));
	hotspots.push_back(ZmbHotspot(kHotspotHelpButtonPressed, kSystemShape0001_25_HelpButtonPressed, 0, _helpButtonRect));

	// Same TOPMOST-only flags as Go/Map buttons.
	_helpButtonFeature = loadScrbFeature(ZmbResource(ZmbResource::kSystem, kSysResShapeBitmap0001_Dialog), 0,
										 hotspots, 0,
										 ZmbFeature::FLAG_00001000_TOPMOST,
										 hooksHelpMapButton);
}

void ZoombiniInteractive::buildEmbeddedControlHotspots(int16 bitmapResId, Common::Array<ZmbHotspot> &goMapHotspots, Common::Array<ZmbHotspot> &helpHotspots, bool separateBitmapResources) {
	_goMapBitmapResId = bitmapResId;
	_goMapButtonsUseSeparateBitmapResources = separateBitmapResources;
	configureTlcGoMapButtonHover(bitmapResId);

	goMapHotspots.clear();
	goMapHotspots.push_back(ZmbHotspot(kHotspotGoButtonNormal, _goButtonShapeEnabledId, 0, _goButtonRect));
	goMapHotspots.push_back(ZmbHotspot(kHotspotSecondGoButtonNormal, _secondGoButtonShapeEnabledId, 0, _secondGoButtonRect));
	goMapHotspots.push_back(ZmbHotspot(kHotspotMapButtonNormal, _mapButtonShapeNormalId, 0, _mapButtonRect));
	goMapHotspots.push_back(ZmbHotspot(kHotspotGoButtonPressed, _goButtonShapePressedId, 0, _goButtonRect));
	goMapHotspots.push_back(ZmbHotspot(kHotspotSecondGoButtonPressed, _secondGoButtonShapePressedId, 0, _secondGoButtonRect));
	goMapHotspots.push_back(ZmbHotspot(kHotspotMapButtonPressed, _mapButtonShapePressedId, 0, _mapButtonRect));
	_goMapButtonHotspots = goMapHotspots;

	helpHotspots.clear();
	if (_vm->supportsOnScreenHelp() && _helpButtonStateMap[kThreeButtons_Help]._drawEnabled) {
		helpHotspots.push_back(ZmbHotspot(kHotspotHelpButtonNormal, kSystemShape0001_24_HelpButtonNormal, 0, _helpButtonRect));
		helpHotspots.push_back(ZmbHotspot(kHotspotHelpButtonPressed, kSystemShape0001_25_HelpButtonPressed, 0, _helpButtonRect));
	}
}

void ZoombiniInteractive::bindEmbeddedControlFeature(ZmbFeature *feature) {
	_goMapButtonsFeature = feature;
	_helpButtonFeature = feature;
}

void ZoombiniInteractive::updateTlcButtonHover(const Common::Point &absPos) {
	// v2.0US release only: shared three-button hover visuals.
	if (!_vm->isVersionFamilyTlcV2())
		return;

	genericButton_updateHoverState(_goMapButtonsFeature, absPos, _goMapButtonStateMap, _threeButtonRectMap);
	genericButton_updateHoverState(_helpButtonFeature, absPos, _helpButtonStateMap, _threeButtonRectMap);
}

ZmbRenderResult ZoombiniInteractive::goMapButtons_renderSeparateBitmaps(ZmbFeature *feature) {
	Common::Array<ZmbHotspot> hotspots = _goMapButtonHotspots;
	goMapButtons_preRenderShape(feature, nullptr, hotspots);
	for (uint32 hotspotIdx = 0; hotspotIdx < hotspots.size(); hotspotIdx++) {
		const ZmbHotspot &hotspot = hotspots[hotspotIdx];
		if (hotspot._shapeIdx == ZmbHotspot::kShapeNone)
			continue;

		_vm->_gfx->drawImage(ZoombiniGraphics::kShapeScreen, _goMapBitmapResId + hotspot._shapeIdx - 1, hotspot.getPos());
	}
	return ZmbRenderResult::kRendered;
}

void ZoombiniInteractive::goMapButtons_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)hsGroup;

	// Hover availability is resolved against the owning page bitmap when the
	// button states are configured. The callback runner itself may have no
	// bitmap resource.
	genericButton_selectShapes(feature, hotspots, _goMapButtonStateMap, 0, 0, false);
}

void ZoombiniInteractive::goMapButtons_onPostRender(ZmbFeature *feature) {
	genericButton_action(feature, _goMapButtonStateMap, static_cast<OnButtonActionFunc>(&ZoombiniInteractive::goMapButtons_onButtonAction));
}

ZmbEventHandleResult ZoombiniInteractive::onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) {
	// A click during a pending departure skips its remaining animation.
	// Mark it complete so @ref ZoombiniInteractive::onAnimFrame() commits the transition after this render.
	if (isDeparturePending()) {
		_hasDepartSfxHandle = false;
		_departureState = ZmbDepartureState::kCompleted;
		return ZmbEventHandleResult::kConsumed;
	}

	// Deliberate asynchronous adaptation: consume clicks during the two-frame pressed-state wait,
	// then replay one after the transition is armed so it becomes an animation skip.
	if (_goButtonPressPending) {
		_deferredGoClickPending = true;
		_deferredGoClickAbsPos = absPos;
		_deferredGoClickRelPos = relPos;
		return ZmbEventHandleResult::kConsumed;
	}

	return ZoombiniPage::onLButtonDown(absPos, relPos);
}

void ZoombiniInteractive::onGoButtonActivated() {
	playDepartSfx();
	_departureState = ZmbDepartureState::kAnimating;
}

void ZoombiniInteractive::onDisabledGoButtonActivated() {
}

void ZoombiniInteractive::onSecondGoButtonActivated() {
	playDepartSfx();
	_departureState = ZmbDepartureState::kAnimating;
}

void ZoombiniInteractive::onDisabledSecondGoButtonActivated() {
}

void ZoombiniInteractive::debugForceFinish() {
	_debugFinishAllAccepted = true;

	switch (_pageType) {
	// These pages have page-specific completion/Go handling.
	// Preserve those paths so their debug finish remains visually consistent with a solved puzzle.
	case ZoombiniPageType::kFerry:
	case ZoombiniPageType::kNet:
	case ZoombiniPageType::kMaze:
	case ZoombiniPageType::kCaves:
	case ZoombiniPageType::kSlides:
		debugPrepareForDeparture();
		onGoButtonActivated();
		return;
	// Do nothing on shelter pages
	case ZoombiniPageType::kRodMap:
	case ZoombiniPageType::kPicker:
	case ZoombiniPageType::kBasecamp1:
	case ZoombiniPageType::kBasecamp2:
	case ZoombiniPageType::kTown:
		return;
	default:
		break;
	}

	// Use a qualified call so page-specific departure animations cannot defer the immediate debug transition.
	ZoombiniInteractive::executeDeparture();
}

void ZoombiniInteractive::executeDeparture() {
	_departureState = ZmbDepartureState::kIdle;

	if (_vm->_state->inPracticeMode()) {
		// A completed practice puzzle discards its temporary pack and returns directly to RodMap;
		// it must not save the pack, advance a route, or use Xfer.
		ZmbStateActivePack &activePack = _vm->_state->getCurrentState()._zmbPackActive;
		activePack.clearEntries();
		activePack.setSkipOccupiedEntries(true);
		activePack.setSkipUnoccupiedEntries(true);

		_vm->setNextPage(ZoombiniPageType::kRodMap);
	} else {
		// Write snoid runners back to active pack and route non-occupied to resting packs.
		// BC1/BC2 override this with their own save+snapshot logic.
		saveSnoidsToPack(_debugFinishAllAccepted);
		routeNonOccupiedToRestingPack();

		// When a container puzzle departs, record the route's completion level in the per-route completion flags.
		// If the route just advanced, the stored level is one higher than the level just completed.
		// Use the previous level when @ref ZoombiniInteractive::routeNonOccupiedToRestingPack() has just advanced it.
		int16 routeLevel = _vm->_state->readActivePageRouteLevel();
		if (_vm->_state->_routeLevelJustAdvanced && 0 < routeLevel)
			routeLevel -= 1;
		const int16 activeRouteId = _vm->_state->readActivePageRouteId();
		if (activeRouteId != -1 && _vm->_state->isNextPageContainer()) {
			_vm->_state->getCurrentState().setRouteCompletionFlag(static_cast<ZmbRouteId>(activeRouteId), routeLevel);
		}

		if (_departXferSrcSiPage != ZmbSrcPageKind::kMinus1) {
			_vm->_xferSrcPage = _departXferSrcSiPage;
			_vm->setNextPage(ZoombiniPageType::kXfer);
		}
	}
	close();
}

void ZoombiniInteractive::routeNonOccupiedToRestingPack() {
	// Handle per-puzzle level flags, non-occupied Snoid routing, perfect streak tracking, and route level advancement.
	ZmbStateFile &f = _vm->_state->getCurrentState();
	ZmbDestPageKind currentDiPage = f._currentPage;

	// Only puzzle pages (DI 7-18) participate in route tracking.
	if (currentDiPage < ZmbDestPageKind::kBridge_07 || ZmbDestPageKind::kMaze_18 < currentDiPage)
		return;

	// Convert the puzzle page ID to a one-based route index via the state's active-route mapping.
	uint16 routeIdx = static_cast<uint16>(_vm->_state->readActivePageRouteId()) + 1;

	bool isNextPageShelter = false;
	switch (currentDiPage) {
	case ZmbDestPageKind::kPizza_09:
	case ZmbDestPageKind::kSlides_12:
	case ZmbDestPageKind::kNet_15:
	case ZmbDestPageKind::kMaze_18:
		isNextPageShelter = true;
		break;
	default:
		isNextPageShelter = false;
		break;
	}

	// Set per-puzzle level flag (low nibble).
	uint16 routeLevel = f._routeLevels[routeIdx - 1];
	byte levelBitmask = static_cast<byte>(1 << (routeLevel & 3));
	// The first three bytes hold a dummy byte and the perfect-streak flag;
	// puzzle slots [3..14] correspond to DI pages 7..18.
	f._pageLevelFlags[static_cast<int16>(currentDiPage) - 4] |= levelBitmask;

	// Count non-occupied snoids in active pack.
	int16 nonOccupiedCount = 0;
	for (int16 i = 0; i < f._zmbPackActive.getPackZmbCount(); i++) {
		if (!f._zmbPackActive.getEntry(i).getIsOccupied())
			nonOccupiedCount += 1;
	}

	_vm->_state->_routeLevelJustAdvanced = false;

	if (0 < nonOccupiedCount) {
		// ---------------------------------------------------------------
		// Clear the perfect streak when Snoids were lost or marked non-occupied by RodMap confirmation.
		// Route the non-occupied Snoids to their resting packs.
		// This routing depends on a valid route, not on whether the page is a container puzzle.
		// Non-container puzzles must also return lost Snoids to the route's resting pack.
		// ---------------------------------------------------------------
		_vm->_state->_perfectStreakFlag = false;

		{
			// Determine destination pack based on route.
			// BC0 (Isle), Route 2-3 -> BC1, Route 4 -> BC2.
			// Returning non-occupied Snoids cross a shelter arrival boundary, so
			// this is the only route-side update to the logical BC1/BC2 counts.
			ZmbStateActivePack *destPack = nullptr;
			switch (static_cast<ZmbRouteId>(routeIdx - 1)) {
			case ZmbRouteId::kBigBadHungry:
				destPack = &f._zmbPackIsle;
				break;
			case ZmbRouteId::kWhosBayou:
			case ZmbRouteId::kDeepDarkForest:
				destPack = &f._zmbPackBC1;
				f._zmbStoredBC1Count += nonOccupiedCount;
				break;
			case ZmbRouteId::kMontDespair:
				destPack = &f._zmbPackBC2;
				f._zmbStoredBC2Count += nonOccupiedCount;
				break;
			default:
				return;
			}

			// Compact out occupied entries from dest pack.
			if (destPack->getSkipOccupiedEntries()) {
				for (int16 destEntryIdx = 0; destEntryIdx < destPack->getPackZmbCount(); destEntryIdx++) {
					if (destPack->getEntry(destEntryIdx).getIsOccupied()) {
						destPack->removeEntryAt(destEntryIdx);
						destEntryIdx -= 1;
					}
				}
			}
			destPack->setSkipUnoccupiedEntries(false);
			destPack->setSkipOccupiedEntries(false);

			// Find first non-occupied entry in active pack.
			int16 srcIdx = 0;
			while (srcIdx < f._zmbPackActive.getPackZmbCount() && f._zmbPackActive.getEntry(srcIdx).getIsOccupied())
				srcIdx += 1;

			// Find insertion point in dest pack.
			int16 destIdx = -1;
			for (int16 destEntryIdx = 0; destIdx < 0 && destEntryIdx <= destPack->getPackZmbCount(); destEntryIdx++) {
				if (destEntryIdx == destPack->getPackZmbCount() ||
					destPack->getEntry(destEntryIdx).getTraits()._hair == ZmbTrait::TRAIT_NONE)
					destIdx = destEntryIdx;
			}
			if (destIdx < 0) {
				destPack->clearEntries();
				destIdx = 0;
			}

			// Copy non-occupied entries from active -> dest pack.
			for (int16 sourceEntryIdx = 0; sourceEntryIdx < nonOccupiedCount; sourceEntryIdx++) {
				if (destPack->getEntryCapacity() <= destPack->getPackZmbCount() ||
					destPack->getEntryCapacity() <= destIdx)
					break;

				ZmbStateActiveEntry entry = destPack->getEntry(destIdx);
				const ZmbStateActiveEntry &src = f._zmbPackActive.getEntry(srcIdx);
				entry.setTraits(src.getTraits());
				entry.setU32Name(_vm, src.getU32Name(_vm));
				entry.setIsOccupied(true);
				if (!destPack->appendEntryAt(destIdx, entry))
					break;
				destIdx += 1;
				srcIdx += 1;
			}

			// Remove non-occupied entries from active pack.
			f._zmbPackActive.removeEntriesFromBack(nonOccupiedCount);
		}
	} else if (_vm->_state->_perfectStreakFlag) {
		// ---------------------------------------------------------------
		// All snoids survived AND perfect streak intact.
		// Every puzzle in the route receives the perfect-clear high-nibble flag.
		// Only memorial creation and route-level advancement are restricted to container puzzles.
		// ---------------------------------------------------------------

		// Set the per-puzzle perfect-completion high nibble for every non-container puzzle as well,
		// using the same page-indexed layout as the low nibble above.
		f._pageLevelFlags[static_cast<int16>(currentDiPage) - 4] |= static_cast<byte>(levelBitmask << 4);

		if (isNextPageShelter) {
			const ZmbRouteId routeId = static_cast<ZmbRouteId>(routeIdx - 1);
			if (_vm->_state->recordFirstClearMemorial(routeId, static_cast<int16>(routeLevel) + 1))
				debugC(1, MohawkEngine_Zoombini::kDebugState03, "state: route %d first-clear memorial recorded at level %d", routeIdx, static_cast<int16>(routeLevel) + 1);
			// The v1.1US and v2.0US layouts use four persisted counters and advance after three perfect route passes.
			// Europe v1.0 and its 16-bit Europe v1.1 update have no counters in their 44549-byte save layout, and advance after one perfect pass.
			if (static_cast<int16>(routeLevel) < 3) {
				if (_vm->isVersionFamilyUsV1() || _vm->isVersionFamilyTlcV2()) { // Based on US v1.1 and further
					f._routePerfectCounters[routeIdx - 1] += 1;
					if (3 <= f._routePerfectCounters[routeIdx - 1]) {
						if (_vm->_state->advanceRouteLevel(routeId))
							debugC(1, MohawkEngine_Zoombini::kDebugState03, "state: route %d advanced to level %d", routeIdx, f._routeLevels[routeIdx - 1]);
					}
				} else if (_vm->isVersionFamilyEuV1()) { // Europe v1.x
					if (_vm->_state->advanceRouteLevel(routeId))
						debugC(1, MohawkEngine_Zoombini::kDebugState03, "state: route %d advanced to level %d", routeIdx, f._routeLevels[routeIdx - 1]);
				} else {
					error("interactive: unknown Zoombini game variant 0x%08X - cannot determine route-level advancement rules", _vm->getFeatures());
					return;
				}
			}
		}
	}
}

bool ZoombiniInteractive::compareWalkerPositions(ZmbSnoid *left, ZmbSnoid *right) {
	return left->getAnimTargetPos().x < right->getAnimTargetPos().x;
}

void ZoombiniInteractive::startDepartWalkAnimation(const Common::Point &target, uint32 stagger) {
	// Collect occupied idle Snoids, sort them by target X, and dispatch from right to left.
	// Each walker receives the common arrival-motion state and a staggered animation deadline;
	// render-enabled state remains unchanged.
	_vm->_walkersInProgress = 0;

	// Collect eligible snoids: must be TYPE_SNOID, occupied, and idle.
	Common::Array<ZmbSnoid *> walkers;
	for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
		ZmbSnoid *snoid = *it;
		if (!snoid->isPackSnoid())
			continue;
		if (!snoid->_packIsOccupied)
			continue;
		if (snoid->getAnimState() != kSnoidAnimState000_Idle)
			continue;
		walkers.push_back(snoid);
	}

	if (walkers.empty())
		return;

	// Sort by ascending @ref ZmbSnoid::_animTargetPos X coordinate.
	// The target starts at the loaded feature position and changes after a pedestal drop.
	// The X coordinate determines the departure stagger order, with the rightmost Snoid departing first.
	Common::sort(walkers.begin(), walkers.end(), compareWalkerPositions);

	// Highest animation destination.x (rightmost) departs first.
	uint32 frameBase = getCurrentFrameCounter();
	uint16 walkerIdx = 0;
	for (int entryIdx = static_cast<int>(walkers.size()) - 1; 0 <= entryIdx; entryIdx--) {
		ZmbSnoid *snoid = walkers[entryIdx];
		snoid->setAnimTargetPos(target);

		// Merge the old click rectangle, build the common-image waiting pose, and then merge the new click rectangle.
		// Keep that pose frozen for the first draw.
		// This prevents another departing Snoid's dirty clip from exposing individual layers.
		addExternalDirtyRect(snoid->getClickRect());
		snoid->clearPreparedRenderHotspots();
		snoid->setAnimState(kSnoidAnimState010_ArrivalMotion, nullptr);
		prepareSnoidVisualCoverage(snoid, true);
		addExternalDirtyRect(snoid->getClickRect());

		snoid->setDelayUntilFrame(frameBase + walkerIdx * stagger);
		walkerIdx += 1;
	}
}

void ZoombiniInteractive::playDepartSfx(int16 systemSoundId) {
	Audio::SoundHandle *handle = _vm->_sound->playSound(ZmbResource(ZmbResource::kSystem, systemSoundId), Audio::Mixer::kSFXSoundType);
	_hasDepartSfxHandle = (handle != nullptr);
	if (handle)
		_departSfxHandle = *handle;
}

bool ZoombiniInteractive::isDepartSfxDone() const {
	if (!_hasDepartSfxHandle)
		return true;
	return !_vm->_system->getMixer()->isSoundHandleActive(_departSfxHandle);
}

void ZoombiniInteractive::onMapButtonActivated() {
	if (!confirmMapTransition())
		return;

	saveStateBeforeMapTransition();
	_vm->_state->getCurrentState()._isDirty = true;

	_vm->setNextPage(ZoombiniPageType::kRodMap);
	close();
}

bool ZoombiniInteractive::confirmMapTransition() {
	return true;
}

void ZoombiniInteractive::saveStateBeforeMapTransition() {
}

void ZoombiniInteractive::goMapButtons_onButtonAction(ZmbFeature *feature, uint32 bsIdx, ButtonState &bs) {
	(void)feature;

	switch (bsIdx) {
	case kThreeButtons_Go:
		_goButtonPressPending = false;
		if (bs._isPressDisabled)
			onDisabledGoButtonActivated();
		else
			onGoButtonActivated();
		break;
	case kThreeButtons_SecondGo:
		_goButtonPressPending = false;
		if (bs._isPressDisabled)
			onDisabledSecondGoButtonActivated();
		else
			onSecondGoButtonActivated();
		break;
	case kThreeButtons_Map:
		onMapButtonActivated();
		break;
	default:
		error("interactive: invalid option dialog long button event(%u)", bsIdx);
		break;
	}
}

ZmbEventHandleResult ZoombiniInteractive::goMapButtons_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	(void)relPos;

	bool startsGoPress = false;
	for (uint32 buttonIdx = kThreeButtons_Go; buttonIdx <= kThreeButtons_SecondGo; buttonIdx++) {
		Common::StableMap<uint32, ButtonState>::iterator stateIt = _goMapButtonStateMap.find(buttonIdx);
		Common::HashMap<uint32, Common::Rect>::const_iterator rectIt = _threeButtonRectMap.find(buttonIdx);
		if (stateIt == _goMapButtonStateMap.end() || rectIt == _threeButtonRectMap.end())
			continue;

		ButtonState &buttonState = stateIt->second;
		if (buttonState._drawEnabled && !buttonState._isPressDisabled && rectIt->_value.contains(absPos)) {
			startsGoPress = true;
			break;
		}
	}

	ZmbEventHandleResult result = genericButton_onLButtonDown(feature, absPos, _goMapButtonStateMap, _threeButtonRectMap,
															  static_cast<OnButtonActionFunc>(&ZoombiniInteractive::goMapButtons_onButtonAction));
	if (startsGoPress && result == ZmbEventHandleResult::kConsumed)
		_goButtonPressPending = true;
	return result;
}

void ZoombiniInteractive::helpButton_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)hsGroup;

	// Help hover shapes belong to the system dialog bitmap, not necessarily to
	// the callback runner that owns the combined page controls.
	genericButton_selectShapes(feature, hotspots, _helpButtonStateMap, 0, 0, false);
}

void ZoombiniInteractive::helpButton_onPostRender(ZmbFeature *feature) {
	genericButton_action(feature, _helpButtonStateMap, static_cast<OnButtonActionFunc>(&ZoombiniInteractive::helpButton_onPostAnimation));
}

void ZoombiniInteractive::helpButton_onPostAnimation(ZmbFeature *feature, uint32 bsIdx, ButtonState &bs) {
	(void)feature;
	(void)bs;

	switch (bsIdx) {
	case kThreeButtons_Help:
		_vm->openHelpDialog(_vm->getActivePage()->getPageType());
		break;
	default:
		error("interactive: invalid option dialog long button event(%u)", bsIdx);
		break;
	}
}

ZmbEventHandleResult ZoombiniInteractive::helpButton_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	(void)relPos;

	return genericButton_onLButtonDown(feature, absPos, _helpButtonStateMap, _threeButtonRectMap);
}

void ZoombiniInteractive::showNotiBoxShort(const Common::U32String &ustr) {
	showNotiBox(ustr, false, true);
}

void ZoombiniInteractive::hideNotiBoxShort() {
	_notiBoxShowUntilFrame = 0;
}

void ZoombiniInteractive::showNotiBoxLong(const Common::U32String &ustr) {
	showNotiBox(ustr, true, false);
}

void ZoombiniInteractive::showNotiBoxLongPersistent(const Common::U32String &ustr) {
	showNotiBox(ustr, true, true);
}

void ZoombiniInteractive::showNotiBoxLong(ZoombiniText::Key textKey) {
	showNotiBoxLong(_vm->_text->getLocalizedString(textKey));
}

void ZoombiniInteractive::showNotiBox(const Common::U32String &ustr, bool isNotiBoxLong, bool persistent) {
	_isNotiBoxLong = isNotiBoxLong;
	_notiBoxText = ustr;
	if (persistent)
		_notiBoxShowUntilFrame = UINT32_MAX; // Virtually infinite duration
	else
		_notiBoxShowUntilFrame = _currentFrameCounter + NOTIBOX_LONG_SHOW_FRAME_DURATION;

	// Only register NotiBox feature if not yet registered.
	if (!_notiBoxFeature) {
		ZmbFeature::EventHooks hooks;
		hooks.setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniInteractive::notiBox_preRenderShape));
		hooks.setPostRenderFunc(static_cast<ZmbFeature::OnPostRenderFunc>(&ZoombiniInteractive::notiBox_onPostRender));

		Common::Array<ZmbHotspot> hotspots;
		hotspots.push_back(ZmbHotspot(kHotspotNotiBoxShort, kSystemShape3001_01_NotiBoxShort, 0, _notiBoxShortRect));
		hotspots.push_back(ZmbHotspot(kHotspotNotiBoxLong, kSystemShape3001_02_NotiBoxLong, 0, _notiBoxLongRect));

		_notiBoxFeature = loadScrbFeature(ZmbResource(ZmbResource::kSystem, kSysResShapeBitmap3001_NotiBox), 0,
										  hotspots, 0,
										  ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00001000_TOPMOST,
										  hooks);
		// The first pre-render pass computes the click rectangle from shape metadata.
		// Explicitly invalidate the notification rectangle
		// because the drawn bounds may still be clipped on that first frame.
		addExternalDirtyRect(isNotiBoxLong ? _notiBoxLongRect : _notiBoxShortRect);
	} else {
		_notiBoxFeature->setNeedsRedraw(true);
		addExternalDirtyRect(isNotiBoxLong ? _notiBoxLongRect : _notiBoxShortRect);
	}
}

void ZoombiniInteractive::notiBox_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)hsGroup;

	if (_notiBoxShowUntilFrame < _currentFrameCounter) {
		feature->scheduleClose();
		_notiBoxFeature = nullptr;
	}

	hotspots[kHotspotNotiBoxShort]._shapeIdx = kSystemShape3001_01_NotiBoxShort;
	hotspots[kHotspotNotiBoxLong]._shapeIdx = kSystemShape3001_02_NotiBoxLong;
	uint16 hideShapeIdx = _isNotiBoxLong ? kHotspotNotiBoxShort : kHotspotNotiBoxLong;
	hotspots[hideShapeIdx]._shapeIdx = ZmbHotspot::kShapeNone;
}

void ZoombiniInteractive::notiBox_onPostRender(ZmbFeature *feature) {
	(void)feature;

	if (_notiBoxText.empty())
		return;

	ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;

	ZoombiniGraphics::TextConf tc;
	tc._vAlign = Graphics::kTextAlignCenter;
	tc._hAlign = Graphics::kTextAlignCenter;
	Common::Rect textRect = _isNotiBoxLong ? _notiBoxLongRect : _notiBoxShortRect;
	// The Korean release offsets the top edge by 3 pixels before centered rendering.
	// Z1-11U and Z1-20U use the selected rectangle unchanged.
	if (_vm->getLanguage() == Common::KO_KOR)
		textRect.top += 3;
	_vm->_gfx->drawText(screenKind, _notiBoxText, textRect, tc);
}

void ZoombiniInteractive::genericStickyButton_selectShapes(ZmbFeature *feature, Common::Array<ZmbHotspot> &hotspots, Common::StableMap<uint32, StickyButtonState> &buttonStateMap) {
	if (!feature) {
		error("interactive: sticky-button callback has no feature");
		return;
	}

	for (Common::StableMap<uint32, StickyButtonState>::iterator it = buttonStateMap.begin(); it != buttonStateMap.end(); it++) {
		StickyButtonState &bs = it->second;

		if (!bs._enabled)
			continue;
		if (hotspots.size() <= bs._hsNormalId || hotspots.size() <= bs._hsPressedId) {
			error("interactive: sticky-button SCRB is missing required hotspots");
			return;
		}

		ZmbHotspot &hsNormal = hotspots[bs._hsNormalId];
		ZmbHotspot &hsPressed = hotspots[bs._hsPressedId];
		if (bs._isStuck)
			hsNormal._shapeIdx = ZmbHotspot::kShapeNone;
		else
			hsPressed._shapeIdx = ZmbHotspot::kShapeNone;
	}
}

ZmbEventHandleResult ZoombiniInteractive::genericStickyButton_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, Common::StableMap<uint32, StickyButtonState> &buttonStateMap, OnStickyButtonActionFunc onActionFunc) {
	if (!feature) {
		error("interactive: sticky-button input requires a feature");
		return ZmbEventHandleResult::kPassthrough;
	}

	ZmbDrawRecord *drawRecord = feature->findDrawRecordAtPoint(absPos);
	if (!drawRecord)
		return ZmbEventHandleResult::kPassthrough;

	for (Common::StableMap<uint32, StickyButtonState>::iterator it = buttonStateMap.begin(); it != buttonStateMap.end(); it++) {
		StickyButtonState &bs = it->second;

		if (!bs._enabled)
			continue;

		if (drawRecord->_hs._hsId != bs._hsNormalId && drawRecord->_hs._hsId != bs._hsPressedId)
			continue;

		bs.toggle(_vm);
		if (onActionFunc != nullptr)
			(this->*onActionFunc)(feature, it->first, bs);
		feature->requestVisualRematerialization();
		return ZmbEventHandleResult::kConsumed;
	}

	return ZmbEventHandleResult::kPassthrough;
}

ZmbEventHandleResult ZoombiniInteractive::genericStickyButton_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, Common::StableMap<uint32, StickyButtonState> &buttonStateMap, const Common::HashMap<uint32, Common::Rect> &buttonRectMap, OnStickyButtonActionFunc onActionFunc) {
	if (!feature) {
		error("interactive: sticky-button input requires a feature");
		return ZmbEventHandleResult::kPassthrough;
	}

	for (Common::StableMap<uint32, StickyButtonState>::iterator it = buttonStateMap.begin(); it != buttonStateMap.end(); it++) {
		uint32 bsIdx = it->first;
		StickyButtonState &bs = it->second;

		if (!bs._enabled)
			continue;

		Common::HashMap<uint32, Common::Rect>::const_iterator rit = buttonRectMap.find(bsIdx);
		if (rit == buttonRectMap.end())
			continue;
		const Common::Rect &buttonRect = rit->_value;
		if (!buttonRect.contains(absPos))
			continue;

		bs.toggle(_vm);
		if (onActionFunc != nullptr)
			(this->*onActionFunc)(feature, bsIdx, bs);
		feature->requestVisualRematerialization();
		return ZmbEventHandleResult::kConsumed;
	}

	return ZmbEventHandleResult::kPassthrough;
}

void ZoombiniInteractive::StickyButtonState::toggle(MohawkEngine_Zoombini *vm) {
	_isStuck = !_isStuck;
	const ZmbResource &soundId = _isStuck ? _pressSoundId : _releaseSoundId;
	if (soundId.hasId())
		vm->_sound->playSound(soundId, Audio::Mixer::kSFXSoundType, false);
}

void ZoombiniInteractive::onAnimFrame() {
	// Render before inspecting a pending transition.
	// This lets a PLAY_ONCE runner finish its current authored step and deliver its callback while SND 996 is active.
	// Lilly depends on that order for toads already returning from the far bank: no new queue step starts after Go,
	// but the currently loaded SCRB still reaches its exact stopping frame.
	ZoombiniPage::onAnimFrame();

	if (_deferredGoClickPending && !_goButtonPressPending) {
		const Common::Point absPos = _deferredGoClickAbsPos;
		const Common::Point relPos = _deferredGoClickRelPos;
		_deferredGoClickPending = false;
		handleLButtonDown(absPos, relPos);
		if (isClosed())
			return;
	}

	updateDepartureState();
	if (isClosed() || (isDeparturePending() && !runsControllerDuringDeparture()))
		return;

	onPostRenderFrame();
	if (isClosed() || (isDeparturePending() && !runsAmbientDuringDeparture()))
		return;

	// Run the ambient driver after rendering.
	// This order keeps an equal-priority ambient candidate after both the render pass and
	// page-owned controller work that consumes callbacks from that pass.
	runAmbientSoundDriver();
	onPostAmbientFrame();
}

void ZoombiniInteractive::onModalFrame() {
	ZoombiniPage::onModalFrame();
	if (!isClosed())
		runAmbientSoundDriver();
}

void ZoombiniInteractive::updateDepartureState() {
	if (_departureState == ZmbDepartureState::kAnimating && isDepartSfxDone())
		_departureState = ZmbDepartureState::kCompleted;
	if (_departureState != ZmbDepartureState::kCompleted)
		return;

	_departureState = ZmbDepartureState::kIdle;
	executeDeparture();
}

// Ambient playback belongs to the shared interactive-page owner because its timing
// is ordered after each page render and participates in that page's sound queue.
void ZoombiniInteractive::runAmbientSoundDriver() {
	// Select the sound pool before touching the cadence RNG. Pages with their
	// own ambient controller, such as Town, do not participate in this driver.
	const int16 *pool = nullptr;
	uint16 poolSize = 0;

	switch (getPageType()) {
	case ZoombiniPageType::kBasecamp1:
		pool = kAmbientPoolBC1;
		poolSize = ARRAYSIZE(kAmbientPoolBC1);
		break;
	case ZoombiniPageType::kBasecamp2:
		pool = kAmbientPoolBC2;
		poolSize = ARRAYSIZE(kAmbientPoolBC2);
		break;
	case ZoombiniPageType::kBridge:
		pool = kAmbientPoolBridge;
		poolSize = ARRAYSIZE(kAmbientPoolBridge);
		break;
	case ZoombiniPageType::kTunnels:
		pool = kAmbientPoolTunnels;
		poolSize = ARRAYSIZE(kAmbientPoolTunnels);
		break;
	case ZoombiniPageType::kPizza:
		pool = kAmbientPoolPizza;
		poolSize = ARRAYSIZE(kAmbientPoolPizza);
		break;
	case ZoombiniPageType::kFerry:
		pool = kAmbientPoolFerry;
		poolSize = ARRAYSIZE(kAmbientPoolFerry);
		break;
	case ZoombiniPageType::kLilly:
		pool = kAmbientPoolLilly;
		poolSize = ARRAYSIZE(kAmbientPoolLilly);
		break;
	case ZoombiniPageType::kSlides:
		pool = kAmbientPoolSlides;
		poolSize = ARRAYSIZE(kAmbientPoolSlides);
		break;
	case ZoombiniPageType::kFleens:
		pool = kAmbientPoolFleens;
		poolSize = ARRAYSIZE(kAmbientPoolFleens);
		break;
	case ZoombiniPageType::kNet:
		pool = kAmbientPoolNet;
		poolSize = ARRAYSIZE(kAmbientPoolNet);
		break;
	case ZoombiniPageType::kCaves:
		pool = kAmbientPoolCaves;
		poolSize = ARRAYSIZE(kAmbientPoolCaves);
		break;
	case ZoombiniPageType::kSmoke:
		pool = kAmbientPoolSmoke;
		poolSize = ARRAYSIZE(kAmbientPoolSmoke);
		break;
	case ZoombiniPageType::kMaze:
		pool = kAmbientPoolMaze;
		poolSize = ARRAYSIZE(kAmbientPoolMaze);
		break;
	default:
		return;
	}

	if (!_vm->_state->getEnableSound() || !_vm->_state->getEnableMusic())
		return;

	ZoombiniGameState::AmbientSoundState &ambientState = _vm->_state->getAmbientSoundState();
	if (_currentFrameCounter < _ambientNextPlayFrame)
		return;

	// The shared ambient pools contain system resources, so retain that namespace for the playback check.
	const ZmbResource lastAmbientSound(ZmbResource::kSystem, ambientState._lastSoundId);
	if (_vm->_sound->isSoundPlaying(lastAmbientSound)) {
		// Still playing - reset timer and return without interrupting.
		_ambientNextPlayFrame = _currentFrameCounter + _vm->_rnd->getRandomNumber(180, 240);
		return;
	}

	// Sound has finished. Reset timer and pick next sound.
	_ambientNextPlayFrame = _currentFrameCounter + _vm->_rnd->getRandomNumber(180, 240);

	// Each page retains its own non-repeat cycle across page replacement.
	uint32 &poolState = ambientState.getPoolState(getPageType());
	uint16 ambientPoolIdx = _vm->_rnd->getNonRepeatRandom(poolSize, poolState);

	int16 sndId = pool[ambientPoolIdx];
	if (sndId == 0)
		return;

	// Advance the ambient driver's retained modulo-16 cadence state.
	// PCM streams are already released by the mixer when playback ends, so the
	// original cache-release pass has no additional resource to discard here.
	ambientState._preloadCounter = (ambientState._preloadCounter + 1) % 16;

	// Keep ambient sounds as explicit ownerless queue candidates.
	// Alone they play normally; while a feature has opted into shared arbitration,
	// they compete with it instead of bypassing the queue through a parallel stream.
	queueScriptSoundForNextRenderPass(ZmbResource(ZmbResource::kSystem, sndId));
	ambientState._lastSoundId = sndId;
}

// ---------------------------------------------------------------------------
// Snoid drag-and-drop infrastructure
// ---------------------------------------------------------------------------

ZmbSnoid *ZoombiniInteractive::findSnoidAtPoint(const Common::Point &pos) {
	return findSnoidAtPoint(pos, ZmbFeature::FLAG_00000001_TYPE_SNOID);
}

ZmbSnoid *ZoombiniInteractive::findSnoidAtPoint(const Common::Point &pos, uint32 requiredFlags) {
	ZmbFeature *feature = findRunnerAtPoint(pos, requiredFlags);
	return dynamic_cast<ZmbSnoid *>(feature);
}

void ZoombiniInteractive::onSnoidDragStarted(ZmbSnoid *snoid) {
	if (_vm->_builtinDebug._showDragCoordinates)
		showNotiBoxShort(Common::U32String::format("%d, %d", snoid->getPointLoc().x, snoid->getPointLoc().y));
	else if (!snoid->_name.empty())
		showNotiBoxShort(snoid->_name);
}

void ZoombiniInteractive::onSnoidDragEnded(ZmbSnoid *snoid) {
	(void)snoid;
	hideNotiBoxShort();
}

void ZoombiniInteractive::updateBuiltinDragNotification() {
	if (!_draggedSnoid)
		return;

	if (_vm->_builtinDebug._showDragCoordinates) {
		const Common::Point &point = _draggedSnoid->getPointLoc();
		showNotiBoxShort(Common::U32String::format("%d, %d", point.x, point.y));
	} else if (!_draggedSnoid->_name.empty()) {
		showNotiBoxShort(_draggedSnoid->_name);
	} else {
		hideNotiBoxShort();
	}
}

void ZoombiniInteractive::startSnoidDrag(ZmbSnoid *snoid, const Common::Point &mousePos) {
	_draggedSnoid = snoid;
	_dragOrigPos = snoid->getPointLoc();
	_dragOffset = Common::Point(mousePos.x - _dragOrigPos.x, mousePos.y - _dragOrigPos.y);
	_dragPrevMouseX = mousePos.x;
	_dragHighlightSlot = -1;

	// Record the first DRAW_ON_REG point inside the pickup-root square.
	// Clear it only when this Snoid is the registered occupant.
	_dragSourceDrawOnRegSlot = hitTestDrawOnRegSlot(_dragOrigPos, _clickZoneRadius, false);
	if (0 <= _dragSourceDrawOnRegSlot && getDrawOnRegOccupant(_dragSourceDrawOnRegSlot) == snoid->getId())
		clearDrawOnRegOccupant(_dragSourceDrawOnRegSlot);

	// Entering drag state preserves the current facing direction and common body-image index.
	beginSnoidDrag(snoid);
}

ZmbSnoid *ZoombiniInteractive::finishSnoidDrag() {
	ZmbSnoid *snoid = _draggedSnoid;
	_draggedSnoid = nullptr;
	clearDrawOnRegHighlight();
	endSnoidDrag(snoid);
	return snoid;
}

const Common::Rect &ZoombiniInteractive::getDragConstraintRect() const {
	return _defaultDragConstraint;
}

ZmbEventHandleResult ZoombiniInteractive::onMouseMove(const Common::Point &absPos, const Common::Point &relPos) {
	if (!_draggedSnoid) {
		updateTlcButtonHover(absPos);
		return ZoombiniPage::onMouseMove(absPos, relPos);
	}

	updateTlcButtonHover(Common::Point(-1, -1));

	if (!isSnoidDragPositionFixed()) {
		// Clamp the dragged Snoid to @ref ZoombiniInteractive::getDragConstraintRect().
		const Common::Rect &constraint = getDragConstraintRect();
		int16 newX = CLIP<int16>(absPos.x - _dragOffset.x, constraint.left, constraint.right);
		int16 newY = CLIP<int16>(absPos.y - _dragOffset.y, constraint.top, constraint.bottom);
		_draggedSnoid->setPointLoc(Common::Point(newX, newY));
	}
	if (_vm->_builtinDebug._showDragCoordinates)
		updateBuiltinDragNotification();

	// Compare the current mouse X with @ref ZoombiniInteractive::_dragPrevMouseX to update the facing direction.
	if (absPos.x < _dragPrevMouseX && !_draggedSnoid->isFacingLeft())
		_draggedSnoid->setFacingLeft(true);
	else if (_dragPrevMouseX < absPos.x && _draggedSnoid->isFacingLeft())
		_draggedSnoid->setFacingLeft(false);
	_dragPrevMouseX = absPos.x;

	// Fixed nested drags do not participate in shared seat highlighting.
	if (!isSnoidDragPositionFixed())
		updateDrawOnRegHighlight();

	return ZmbEventHandleResult::kConsumed;
}

ZmbEventHandleResult ZoombiniInteractive::onLButtonUp(const Common::Point &absPos, const Common::Point &relPos) {
	if (!isDragging())
		return ZoombiniPage::onLButtonUp(absPos, relPos);

	endDrag(absPos);
	return ZmbEventHandleResult::kConsumed;
}

// ---------------------------------------------------------------------------
// Update seat-runner highlighting during a drag.
// ---------------------------------------------------------------------------
void ZoombiniInteractive::updateDrawOnRegHighlight() {
	if (_drawOnRegCount <= 0 || !_draggedSnoid)
		return;

	Common::Point snoidPos = _draggedSnoid->getPointLoc();
	int16 hitSlot = hitTestDrawOnRegSlot(snoidPos, _clickZoneRadius, true);

	if (0 <= hitSlot) {
		// Found an empty slot within zone
		if (hitSlot != _dragHighlightSlot) {
			// Changed slot -- unhighlight old, highlight new
			clearDrawOnRegHighlight();
			_dragHighlightSlot = hitSlot;

			// Restart and enable the helper at its authored first frame.
			ZmbFeature *seatRunner = _drawOnRegFeatures[hitSlot];
			if (seatRunner) {
				// A dormant helper has no pending runner damage.
				// Dirty its coverage before enabling the highlighted frame.
				Common::Rect dirtyRect = seatRunner->getSortRect();
				if (dirtyRect.isEmpty())
					dirtyRect = seatRunner->getClickRect();
				if (!dirtyRect.isEmpty())
					addExternalDirtyRect(dirtyRect);
				seatRunner->activateRender();
				seatRunner->activateAnimate();
				seatRunner->setLastFrameIdx(0);
				// The original drag loop runs one page tick immediately after enabling
				// the helper. Make the next event-driven tick due so it performs the
				// same frame-zero-to-frame-one transition.
				seatRunner->resetNextRenderFrame();
				seatRunner->setNeedsRedraw(true);
			}
		}
	} else if (0 <= _dragHighlightSlot) {
		// Left all slots -- unhighlight
		clearDrawOnRegHighlight();
	}
}

void ZoombiniInteractive::clearDrawOnRegHighlight() {
	if (_dragHighlightSlot < 0)
		return;

	// Hide the helper and dirty its previous coverage so the yellow mask is erased immediately.
	ZmbFeature *seatRunner = _drawOnRegFeatures[_dragHighlightSlot];
	if (seatRunner) {
		Common::Rect dirtyRect = seatRunner->getSortRect();
		if (dirtyRect.isEmpty())
			dirtyRect = seatRunner->getClickRect();
		if (!dirtyRect.isEmpty())
			addExternalDirtyRect(dirtyRect);
		// Clear @ref ZmbFeature::FLAG_00010000_SKIP_ONCE and advance the runner back to its first unlit frame.
		// Non-DEFER_RENDER runners still draw their last materialized frame after render activation is cleared,
		// so leaving the yellow frame selected here makes the mask persist indefinitely.
		// Materialize frame zero before deactivating the helper,
		// which is the immediate event-split equivalent of one immediate SKIP_ONCE tick.
		seatRunner->setLastFrameIdx(0);
		seatRunner->removeFlag(ZmbFeature::FLAG_00010000_SKIP_ONCE);
		seatRunner->scheduleNextRenderFrame(_currentFrameCounter);
		prepareFeatureVisualCoverage(seatRunner);
		seatRunner->setNeedsRedraw(true);
		seatRunner->deactivateAnimate();
		seatRunner->deactivateRender();
	}
	_dragHighlightSlot = -1;
}

void ZoombiniInteractive::layoutStaticAndWalkIn(int16 initialYOffset, bool suppressWalkIn) {
	// First 75% of loaded pack snoids stay idle at their pedestal positions.
	// Last 25% are moved off-screen left (x=-50) and given a walk-in animation.
	_vm->_walkersInProgress = 0;

	// Collect occupied pack snoids in load order (keys 10000, 10001, ...).
	Common::Array<ZmbSnoid *> occupied;
	for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
		if ((*it)->isPackSnoid() && (*it)->_packIsOccupied)
			occupied.push_back(*it);
	}

	// The Less Action flag suppresses the walk-in group entirely.
	// In More Action mode, three quarters of the loaded pack remain static.
	const int16 total = static_cast<int16>(occupied.size());
	const bool walkInEnabled = !suppressWalkIn && !_vm->_state->isLessActionEnabled();
	const int16 walkInStart = walkInEnabled ? (3 * total) / 4 : total;

	// Static snoids (first 75%): match each snoid's position to draw-on-reg
	// slots. If a slot's snap position falls within +/-clickZoneRadius of the
	// snoid's position, mark that slot as occupied by this snoid.
	for (int16 entryIdx = 0; entryIdx < walkInStart && entryIdx < total; entryIdx++) {
		ZmbSnoid *snoid = occupied[entryIdx];
		const Common::Point sPos = snoid->getPointLoc();
		int16 slotIdx = hitTestDrawOnRegSlot(sPos, _clickZoneRadius, true);
		if (0 <= slotIdx) {
			setDrawOnRegOccupant(slotIdx, snoid->getId());
		}
	}

	// Last 25%: start off-screen left and enter the departure/path walk toward the pedestal.
	for (int16 i = walkInStart; i < total; i++) {
		ZmbSnoid *snoid = occupied[i];
		const Common::Point target = snoid->getPointLoc();
		snoid->setPointLoc(Common::Point(-50, target.y + initialYOffset));
		snoid->setAnimTargetPos(target);
		snoid->setAnimState(kSnoidAnimState007_Depart, nullptr);
		_vm->_walkersInProgress += 1;
	}
}

void ZoombiniInteractive::assignStaggeredWalkDelays(uint32 startOffset, uint32 stagger) {
	// Collect Snoids in @ref kSnoidAnimState007_Depart or @ref kSnoidAnimState112_Path.
	Common::Array<ZmbSnoid *> walkers;
	for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
		if (!(*it)->isPackSnoid())
			continue;
		ZmbSnoid *snoid = *it;
		const SnoidAnimState st = snoid->getAnimState();
		if (st == kSnoidAnimState007_Depart || st == kSnoidAnimState112_Path)
			walkers.push_back(snoid);
	}

	if (walkers.empty())
		return;

	// Sort by animation destination X, not Y.
	Common::Array<ZmbSnoid *> sortedWalkers;
	for (ZmbSnoid *walker : walkers) {
		uint insertPos = 0;
		while (insertPos < sortedWalkers.size() &&
			   sortedWalkers[insertPos]->getAnimTargetPos().x <= walker->getAnimTargetPos().x)
			insertPos += 1;
		sortedWalkers.insert_at(insertPos, walker);
	}

	// The rightmost walker starts first.
	// Only the animation deadline changes; the Snoid remains visible at its current position during the delay.
	const uint32 baseFrame = getCurrentFrameCounter();
	uint16 walkerIdx = 0;
	for (int entryIdx = static_cast<int>(sortedWalkers.size()) - 1; 0 <= entryIdx; entryIdx--) {
		sortedWalkers[entryIdx]->setDelayUntilFrame(baseFrame + startOffset + walkerIdx * stagger);
		walkerIdx += 1;
	}
}

void ZoombiniInteractive::playActiveHelpSound() {
	// Replay the help voice in @ref ZoombiniInteractive::_activeHelpSoundId when F1 is pressed.
	// If the same sound is already playing, stop and restart it.
	// If not set (resource ID = 0), do nothing.

	if (!_activeHelpSoundId.hasId())
		return;
	if (!_vm->_state->getEnableHelpAudio())
		return;

	// Stop any currently playing help voice (if it's the same resource)
	_vm->_sound->stopSound(_activeHelpSoundId);

	// Play the help voice sound
	_vm->_sound->playSound(_activeHelpSoundId, Audio::Mixer::kSpeechSoundType);
}

} // End of namespace Mohawk
