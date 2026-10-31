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

#include "ripper/combat/combat.h"
#include "ripper/cursor.h"
#include "ripper/cyber.h"
#include "ripper/detection.h"
#include "ripper/milestones.h"
#include "ripper/puzzles/board_arrangement.h"
#include "ripper/puzzles/board_game.h"
#include "ripper/puzzles/calculator.h"
#include "ripper/puzzles/cd_in_book.h"
#include "ripper/puzzles/circuit_chip.h"
#include "ripper/puzzles/clock.h"
#include "ripper/puzzles/crystal.h"
#include "ripper/puzzles/date_selection.h"
#include "ripper/puzzles/eight_button_sequence.h"
#include "ripper/puzzles/gc_csh.h"
#include "ripper/puzzles/horus_word.h"
#include "ripper/puzzles/kd_shooting_gallery.h"
#include "ripper/puzzles/key_group.h"
#include "ripper/puzzles/keypad_sequence.h"
#include "ripper/puzzles/ki_skull_maze.h"
#include "ripper/puzzles/kj_blob_shooter.h"
#include "ripper/puzzles/kk_tile_match.h"
#include "ripper/puzzles/rolodex.h"
#include "ripper/puzzles/shock_lever.h"
#include "ripper/puzzles/six_digit_code.h"
#include "ripper/puzzles/stained_glass.h"
#include "ripper/puzzles/table_gate.h"
#include "ripper/puzzles/tarot_cards.h"
#include "ripper/puzzles/web_grid_shift.h"
#include "ripper/ripper.h"
#include "ripper/scenes/cain_scene.h"
#include "ripper/scenes/ebz2s_scene.h"
#include "ripper/scenes/gym_scene.h"
#include "ripper/scenes/tube_scene.h"
#include "ripper/script/compiled_script.h"
#include "ripper/wac/wac.h"

namespace Ripper {

const char *SceneActionDispatcher::actionName(uint action) {
	switch (action) {
	case 0: return "no action";
	case kSceneActionCircuitChipPuzzle: return "circuit chip placement puzzle";
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
	case kSceneActionKeyGroupPuzzle: return "key group puzzle";
	case kSceneActionDateSelectionPuzzle: return "date selection puzzle";
	case kSceneActionKiSkullMazePuzzle: return "KI skull maze puzzle";
	case kSceneActionWebGridShiftPuzzle: return "web grid shift puzzle";
	case kSceneActionHorusWordPuzzle: return "Horus word puzzle";
	case kSceneActionSixDigitCodePuzzle: return "six-digit code puzzle";
	case kSceneActionShockLeverPuzzle: return "shock lever puzzle";
	case kSceneActionTarotCardPuzzle: return "tarot card puzzle";
	case 24: return "tube switch scene";
	case kSceneActionKkTileMatchPuzzle: return "KK tile match puzzle";
	case kSceneActionRatiniCombat: return "ratini combat encounter";
	case kSceneActionAtkiniCombat: return "atkini combat encounter";
	case 28: return "gym selector";
	case 29: return "crystal piece placement puzzle";
	case 30: return "set chooser template mode";
	case 31: return "no-op";
	case 32: return "clear active display";
	case kSceneActionStainedGlassPuzzle: return "stained glass puzzle";
	case kSceneActionKeypadSequencePuzzle: return "keypad sequence puzzle";
	case 35: return "set UI selection index";
	case 36: return "update UI selection";
	case kSceneActionSetFrontEndActionMask: return "set front-end action mask";
	case kSceneActionBoardGame: return "board game";
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
	case kSceneActionKeyGroupPuzzleAlias: return "key group puzzle";
	case kSceneActionEightButtonSequencePuzzle: return "eight-button sequence puzzle";
	case kSceneActionCainDialogue: return "Cain dialogue scene";
	case kSceneActionAppendNotebookText: return "append resource string to RIPPER.TXT";
	case 200: return "start demo prologue cue";
	case 201: return "stop demo prologue cue";
	case 300: return "arm briefing media trigger";
	case 9999: return "terminate scene runtime";
	default: return "unknown";
	}
}

bool SceneActionDispatcher::dispatch(RipperEngine *engine, const CompiledScript &script,
		const ScriptCommand &command) {
	if (command.arguments.size() < 2)
		return false;

	const uint action = command.arguments[0].value;
	const uint argument = command.arguments[1].value;

	if (action == kSceneActionCircuitChipPuzzle) {
		CircuitChipPuzzle puzzle(engine);
		return puzzle.run(argument) != CircuitChipPuzzle::kLoadFailed;
	}
	if (action == kSceneActionCalculatorPuzzle) {
		CalculatorPuzzle puzzle(engine);
		return puzzle.run(argument) != CalculatorPuzzle::kLoadFailed;
	}
	if (action == kSceneActionRolodexPuzzle) {
		RolodexPuzzle puzzle(engine);
		return puzzle.run(argument) != RolodexPuzzle::kLoadFailed;
	}
	if (action == kSceneActionCyberMenu) {
		return engine->getCyber()->run() != CyberManager::kLoadFailed;
	}
	if (action == kSceneActionClockPuzzle) {
		ClockPuzzle puzzle(engine);
		return puzzle.run(argument) != ClockPuzzle::kLoadFailed;
	}
	if (action == kSceneActionKdShootingGallery) {
		KdShootingGallery gallery(engine);
		return gallery.run(argument) != KdShootingGallery::kLoadFailed;
	}
	if (action == kSceneActionGcCshPuzzle) {
		GcCshPuzzle puzzle(engine);
		return puzzle.run(argument) != GcCshPuzzle::kLoadFailed;
	}
	if (action == kSceneActionTableGatePuzzle) {
		TableGatePuzzle puzzle(engine);
		return puzzle.run(argument) != TableGatePuzzle::kLoadFailed;
	}
	if (action == kSceneActionCdInBookPuzzle) {
		CdInBookPuzzle puzzle(engine);
		return puzzle.run(argument) != CdInBookPuzzle::kLoadFailed;
	}
	if (action == kSceneActionBoardArrangementPuzzle) {
		BoardArrangementPuzzle puzzle(engine);
		return puzzle.run(argument) != BoardArrangementPuzzle::kLoadFailed;
	}
	if (action == kSceneActionKjBlobShooter) {
		KjBlobShooter shooter(engine);
		return shooter.run(argument) != KjBlobShooter::kLoadFailed;
	}
	if (action == kSceneActionEbz2sMenu) {
		Ebz2sScene scene(engine);
		return scene.run(argument) != Ebz2sScene::kLoadFailed;
	}
	if (action == kSceneActionMechiniCombat) {
		CombatEncounter encounter(engine, kMechiniDefinition);
		return encounter.run(argument) != CombatEncounter::kLoadFailed;
	}
	if (action == kSceneActionKeyGroupPuzzle ||
			action == kSceneActionKeyGroupPuzzleAlias) {
		KeyGroupPuzzle puzzle(engine);
		return puzzle.run(argument) != KeyGroupPuzzle::kLoadFailed;
	}
	if (action == kSceneActionDateSelectionPuzzle) {
		DateSelectionPuzzle puzzle(engine);
		return puzzle.run(argument) != DateSelectionPuzzle::kLoadFailed;
	}
	if (action == kSceneActionKiSkullMazePuzzle) {
		KiSkullMazePuzzle puzzle(engine);
		return puzzle.run(argument) != KiSkullMazePuzzle::kLoadFailed;
	}
	if (action == kSceneActionRatiniCombat) {
		CombatEncounter encounter(engine, kRatiniDefinition);
		return encounter.run(argument) != CombatEncounter::kLoadFailed;
	}
	if (action == kSceneActionAtkiniCombat) {
		CombatEncounter encounter(engine, kAtkiniDefinition);
		return encounter.run(argument) != CombatEncounter::kLoadFailed;
	}
	if (action == kSceneActionWebGridShiftPuzzle) {
		WebGridShiftPuzzle puzzle(engine);
		return puzzle.run(argument) != WebGridShiftPuzzle::kLoadFailed;
	}
	if (action == kSceneActionHorusWordPuzzle) {
		HorusWordPuzzle puzzle(engine);
		return puzzle.run(argument) != HorusWordPuzzle::kLoadFailed;
	}
	if (action == kSceneActionSixDigitCodePuzzle) {
		SixDigitCodePuzzle puzzle(engine);
		return puzzle.run(argument) != SixDigitCodePuzzle::kLoadFailed;
	}
	if (action == kSceneActionShockLeverPuzzle) {
		ShockLeverPuzzle puzzle(engine);
		return puzzle.run(argument) != ShockLeverPuzzle::kLoadFailed;
	}
	if (action == kSceneActionTarotCardPuzzle) {
		TarotCardsPuzzle puzzle(engine);
		return puzzle.run(argument) != TarotCardsPuzzle::kLoadFailed;
	}
	if (action == kSceneActionTubeSwitchScene) {
		TubeScene scene(engine);
		return scene.run(argument) != TubeScene::kLoadFailed;
	}
	if (action == kSceneActionKkTileMatchPuzzle) {
		KkTileMatchPuzzle puzzle(engine);
		return puzzle.run(argument) != KkTileMatchPuzzle::kLoadFailed;
	}
	if (action == kSceneActionGymSelector) {
		GymScene scene(engine);
		return scene.run(argument) != GymScene::kLoadFailed;
	}
	if (action == kSceneActionCrystalPuzzle) {
		CrystalPuzzle puzzle(engine);
		const CrystalPuzzle::Result result = puzzle.run(argument);
		debugC(1, kDebugPuzzles,
			"Ripper: crystal puzzle scene action completed result=%d milestone=%u", result, argument);
		return result != CrystalPuzzle::kLoadFailed;
	}
	if (action == kSceneActionNoOp) {
		return true;
	}
	if (action == kSceneActionClearDisplay) {
		g_system->fillScreen(0);
		presentScreen();
		debugC(2, kDebugScene, "Ripper: cleared active scene display from scene action 32");
		return true;
	}
	if (action == kSceneActionStainedGlassPuzzle) {
		StainedGlassPuzzle puzzle(engine);
		return puzzle.run(argument) != StainedGlassPuzzle::kLoadFailed;
	}
	if (action == kSceneActionKeypadSequencePuzzle) {
		KeypadSequencePuzzle puzzle(engine);
		return puzzle.run(argument) != KeypadSequencePuzzle::kLoadFailed;
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
	if (action == kSceneActionBoardGame) {
		BoardGamePuzzle puzzle(engine);
		return puzzle.run(argument) != BoardGamePuzzle::kLoadFailed;
	}
	if (action == kSceneActionKaDialogue) {
		const CyberManager::Result result = engine->getCyber()->runProgram(action, "ka", argument);
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
				program = "wofford-media";
			} else {
				program = "kc";
			}
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
		const CyberManager::Result result = engine->getCyber()->runProgram(action, program, argument);
		return result != CyberManager::kLoadFailed;
	}
	if (action == 44) {
		return true;
	}
	if (action == kSceneActionEightButtonSequencePuzzle) {
		EightButtonSequencePuzzle puzzle(engine);
		return puzzle.run(argument) != EightButtonSequencePuzzle::kLoadFailed;
	}
	if (action == kSceneActionCainDialogue) {
		CainScene scene(engine);
		return scene.run(argument) != CainScene::kLoadFailed;
	}
	if (action == kSceneActionAppendNotebookText)
		return engine->getWac()->appendNotebookResourceString(argument);

	warning("Ripper: unsupported scene action %u ('%s') in '%s' at 0x%x",
		action, actionName(action), script.getMemberName().c_str(), command.offset);
	return false;
}

} // End of namespace Ripper
