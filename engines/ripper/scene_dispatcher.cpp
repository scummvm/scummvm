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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/scene_dispatcher.h"

#include "common/debug.h"
#include "common/system.h"

#include "ripper/briefing.h"
#include "ripper/combat/mechini.h"
#include "ripper/cursor.h"
#include "ripper/cyber.h"
#include "ripper/detection.h"
#include "ripper/milestones.h"
#include "ripper/puzzles/calculator.h"
#include "ripper/puzzles/clock.h"
#include "ripper/puzzles/crystal.h"
#include "ripper/puzzles/gc_csh.h"
#include "ripper/puzzles/rolodex.h"
#include "ripper/puzzles/table_gate.h"
#include "ripper/ripper.h"
#include "ripper/scenes/tube_scene.h"
#include "ripper/script.h"

namespace Ripper {

const char *SceneActionDispatcher::actionName(uint action) {
	switch (action) {
	case 0: return "no action";
	case 1: return "circuit chip placement puzzle";
	case 2: return "scene selection menu";
	case 3: return "unlock-gated selection menu";
	case 4: return "calculator puzzle";
	case 5: return "rolodex sequence puzzle";
	case 6: return "cyber menu transition";
	case 7: return "clock puzzle";
	case 8: return "KD shooting gallery";
	case 9: return "GC/CSH four-choice sequence puzzle";
	case 10: return "table gate lever puzzle";
	case 11: return "CD-in-book button sequence puzzle";
	case 12: return "board arrangement puzzle";
	case 13: return "blob shooter";
	case 14: return "EBZ2S unlock-gated action menu";
	case 15: return "mechini combat encounter";
	case 16: return "key group puzzle";
	case 17: return "date selection puzzle";
	case 18: return "KI skull maze puzzle";
	case 19: return "web grid shift puzzle";
	case 20: return "Horus word puzzle";
	case 21: return "six-digit code puzzle";
	case 22: return "shock lever puzzle";
	case 23: return "tarot card puzzle";
	case 24: return "tube switch scene";
	case 25: return "KK tile match puzzle";
	case 26: return "ratini combat encounter";
	case 27: return "atkini combat encounter";
	case 28: return "gym selector";
	case 29: return "crystal piece placement puzzle";
	case 30: return "set chooser template mode";
	case 31: return "no-op";
	case 32: return "clear active display";
	case 33: return "stained glass puzzle";
	case 34: return "keypad sequence puzzle";
	case 35: return "set UI selection index";
	case 36: return "update UI selection";
	case kSceneActionSetFrontEndActionMask: return "set front-end action mask";
	case 38: return "board game";
	case 39: return "no-op";
	case 40: return "KA dialogue scene";
	case 41: return "KB scene script";
	case 42: return "KC or Wofford media scene";
	case 43: return "KD scene script";
	case 44: return "no-op";
	case 45: return "KF scene script";
	case 46: return "KG scene script";
	case 47: return "KH scene script";
	case 48: return "KI scene script";
	case 49: return "KJ scene script";
	case 50: return "KK scene script";
	case 51: return "KL scene script";
	case 52: return "KM scene script";
	case 53: return "KN scene script";
	case 54: return "KP scene script";
	case 55: return "KQ scene script";
	case 56: return "KR scene script";
	case 57:
	case 58:
	case 59: return "no-op";
	case 60: return "key group puzzle";
	case 61: return "eight-button sequence puzzle";
	case 62: return "Cain dialogue scene";
	case 63: return "append resource string to RIPPER.TXT";
	case 300: return "arm briefing media trigger";
	case 9999: return "terminate scene runtime";
	default: return "unknown";
	}
}

bool SceneActionDispatcher::dispatch(ScriptManager &manager, const CompiledScript &script,
		const ScriptCommand &command, int &callbackResult, bool &stopCallback) {
	stopCallback = false;
	if (command.arguments.size() < 2)
		return false;

	const uint action = command.arguments[0].value;
	const uint argument = command.arguments[1].value;
	RipperEngine *engine = manager._engine;
	debugC(2, kDebugScene,
		"Ripper: dispatch scene action=%u name='%s' argument=%u script='%s' offset=0x%x",
		action, actionName(action), argument, script.getMemberName().c_str(), command.offset);

	if (action == kSceneActionWorldMap) {
		if (!manager.openWorldMap())
			return false;
		if (!manager._runtime.pendingSceneMember.empty()) {
			callbackResult = -3;
			stopCallback = true;
		}
		return true;
	}
	if (action == kSceneActionInventory)
		return manager.openInventory(argument, true);
	if (action == kSceneActionCalculatorPuzzle) {
		CalculatorPuzzle puzzle(engine);
		const CalculatorPuzzle::Result result = puzzle.run(argument);
		debugC(1, kDebugPuzzles,
			"Ripper: calculator puzzle scene action completed result=%d milestone=%u",
			result, argument);
		return result != CalculatorPuzzle::kLoadFailed;
	}
	if (action == kSceneActionRolodexPuzzle) {
		RolodexPuzzle puzzle(engine);
		const RolodexPuzzle::Result result = puzzle.run(argument);
		debugC(1, kDebugPuzzles,
			"Ripper: rolodex puzzle scene action completed result=%d milestone=%u",
			result, argument);
		return result != RolodexPuzzle::kLoadFailed;
	}
	if (action == kSceneActionCyberMenu) {
		const CyberManager::Result result = engine->getCyber()->run();
		debugC(1, kDebugCyber, "Ripper: Cyber menu scene action completed result=%d", result);
		return result != CyberManager::kLoadFailed;
	}
	if (action == kSceneActionClockPuzzle) {
		ClockPuzzle puzzle(engine);
		const ClockPuzzle::Result result = puzzle.run(argument);
		debugC(1, kDebugPuzzles,
			"Ripper: clock puzzle scene action completed result=%d milestone=%u", result, argument);
		return result != ClockPuzzle::kLoadFailed;
	}
	if (action == kSceneActionGcCshPuzzle) {
		GcCshPuzzle puzzle(engine);
		const GcCshPuzzle::Result result = puzzle.run(argument);
		debugC(1, kDebugPuzzles,
			"Ripper: GC/CSH puzzle scene action completed result=%d milestone=%u", result, argument);
		return result != GcCshPuzzle::kLoadFailed;
	}
	if (action == kSceneActionTableGatePuzzle) {
		TableGatePuzzle puzzle(engine);
		const TableGatePuzzle::Result result = puzzle.run(argument);
		debugC(1, kDebugPuzzles,
			"Ripper: table gate puzzle scene action completed result=%d milestone=%u", result, argument);
		return result != TableGatePuzzle::kLoadFailed;
	}
	if (action == kSceneActionMechiniCombat) {
		MechiniEncounter encounter(engine);
		const MechiniEncounter::Result result = encounter.run(argument);
		debugC(1, kDebugCombat,
			"Ripper: Mechini combat scene action completed result=%d milestone=%u", result, argument);
		return result != MechiniEncounter::kLoadFailed;
	}
	if (action == kSceneActionTubeSwitchScene) {
		TubeScene scene(engine);
		const TubeScene::Result result = scene.run(argument);
		debugC(1, kDebugScene,
			"Ripper: tube switch scene action completed result=%d milestone=%u", result, argument);
		return result != TubeScene::kLoadFailed;
	}
	if (action == kSceneActionCrystalPuzzle) {
		CrystalPuzzle puzzle(engine);
		const CrystalPuzzle::Result result = puzzle.run(argument);
		debugC(1, kDebugPuzzles,
			"Ripper: crystal puzzle scene action completed result=%d milestone=%u", result, argument);
		return result != CrystalPuzzle::kLoadFailed;
	}
	if (action == kSceneActionSetChooserTemplateMode) {
		// DispatchSceneEntryAction at 0x36892 captures the active media presentation
		// for the following chooser. ScummVM uses the active framebuffer for mode zero.
		manager._chooserTemplateMode = 0;
		debugC(2, kDebugScene,
			"Ripper: scene action 30 captured active presentation templateMode=%u script='%s' offset=0x%x",
			manager._chooserTemplateMode, script.getMemberName().c_str(), command.offset);
		return true;
	}
	if (action == kSceneActionNoOp) {
		debugC(3, kDebugCyber, "Ripper: Cyber scene action 31 completed as original no-op");
		return true;
	}
	if (action == kSceneActionClearDisplay) {
		g_system->fillScreen(0);
		g_system->updateScreen();
		debugC(2, kDebugScene, "Ripper: cleared active scene display from scene action 32");
		return true;
	}
	if (action == kSceneActionSetUiSelectionIndex) {
		engine->getCursor()->setSelectionIndex(argument);
		debugC(2, kDebugCursor,
			"Ripper: scene action 35 stored UI selection index=%u script='%s' offset=0x%x",
			argument, script.getMemberName().c_str(), command.offset);
		return true;
	}
	if (action == kSceneActionDispatchUiSelection) {
		engine->getCursor()->dispatchSelectionIndexChange(argument);
		debugC(2, kDebugCursor,
			"Ripper: scene action 36 dispatched UI selection index=%u script='%s' offset=0x%x",
			argument, script.getMemberName().c_str(), command.offset);
		return true;
	}
	if (action == kSceneActionSetFrontEndActionMask) {
		// DispatchSceneEntryAction at 0x36892 stores the low 16 bits at
		// SceneRuntime+0x183 for the toolbar and key resolver.
		manager._runtime.frontEndActionMask = (uint16)argument;
		debugC(2, kDebugScene,
			"Ripper: scene action 37 set front-end action mask=0x%04x script='%s' offset=0x%x",
			manager._runtime.frontEndActionMask, script.getMemberName().c_str(), command.offset);
		return true;
	}
	if (action == kSceneActionKaDialogue) {
		debugC(1, kDebugCyber,
			"Ripper: dispatching Cyber dialogue action=%u name='%s' argument=%u activeScript='%s' frame=%u",
			action, actionName(action), argument,
			manager._runtime.activeScript.getMemberName().c_str(), manager._runtime.activeFrame);
		const CyberManager::Result result = engine->getCyber()->runProgram(action, "ka", argument);
		debugC(result == CyberManager::kExited ? 1 : 2, kDebugCyber,
			"Ripper: Cyber dialogue action=%u completed result=%d restoredScript='%s' frame=%u",
			action, result, manager._runtime.activeScript.getMemberName().c_str(),
			manager._runtime.activeFrame);
		return result != CyberManager::kLoadFailed;
	}
	if (action >= kSceneActionKbProgram && action <= kSceneActionKrProgram && action != 44) {
		const char *program = nullptr;
		uint chapter = 0;
		for (uint flag = kMilestoneCompletedAct3; flag != 0; --flag) {
			if (engine->getMilestones()->isSet(flag)) {
				chapter = flag;
				break;
			}
		}
		switch (action) {
		case kSceneActionKbProgram: program = "kb"; break;
		case kSceneActionKcOrWoffordProgram:
			if (chapter < 3) {
				warning("Ripper: Cyber action 42 requires the Wofford interactive media path before chapter 3");
				return false;
			}
			program = "kc";
			break;
		case kSceneActionKdProgram: program = "kd"; break;
		case kSceneActionKfProgram: program = "kf"; break;
		case kSceneActionKgProgram: program = chapter >= 3 ? "kg3" : "kg"; break;
		case kSceneActionKhProgram: program = "kh"; break;
		case kSceneActionKiProgram: program = "ki"; break;
		case kSceneActionKjProgram: program = chapter >= 3 ? "kj3" : "kj"; break;
		case kSceneActionKkProgram: program = "kk"; break;
		case kSceneActionKlProgram: program = "kl"; break;
		case kSceneActionKmProgram: program = "km"; break;
		case kSceneActionKnProgram: program = "kn"; break;
		case kSceneActionKpProgram: program = "kp"; break;
		case kSceneActionKqProgram: program = "kq"; break;
		case kSceneActionKrProgram: program = "kr"; break;
		default: break;
		}
		if (!program)
			return false;
		debugC(1, kDebugCyber,
			"Ripper: dispatching Cyber program action=%u name='%s' script='%s.run' argument=%u activeScript='%s' frame=%u",
			action, actionName(action), program, argument,
			manager._runtime.activeScript.getMemberName().c_str(), manager._runtime.activeFrame);
		const CyberManager::Result result = engine->getCyber()->runProgram(action, program, argument);
		debugC(result == CyberManager::kExited ? 1 : 2, kDebugCyber,
			"Ripper: Cyber program action=%u script='%s.run' completed result=%d restoredScript='%s' frame=%u",
			action, program, result, manager._runtime.activeScript.getMemberName().c_str(),
			manager._runtime.activeFrame);
		return result != CyberManager::kLoadFailed;
	}
	if (action == 44) {
		debugC(3, kDebugCyber, "Ripper: Cyber scene action 44 completed as original no-op");
		return true;
	}
	if (action == kSceneActionTerminateRuntime) {
		if (!manager._runtime.cyberActive)
			return false;
		manager.requestCyberExit("scene-action-9999");
		callbackResult = -4;
		stopCallback = true;
		return true;
	}
	if (action == kSceneActionBriefing)
		return manager._briefing->arm(argument);

	warning("Ripper: unsupported scene action %u ('%s') in '%s' at 0x%x",
		action, actionName(action), script.getMemberName().c_str(), command.offset);
	return false;
}

} // End of namespace Ripper
