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

#ifndef RIPPER_SCENE_DISPATCHER_H
#define RIPPER_SCENE_DISPATCHER_H

#include "common/scummsys.h"

namespace Ripper {

enum SceneAction {
	kSceneActionCircuitChipPuzzle = 1,
	kSceneActionWorldMap = 2,
	kSceneActionInventory = 3,
	kSceneActionCalculatorPuzzle = 4,
	kSceneActionRolodexPuzzle = 5,
	kSceneActionCyberMenu = 6,
	kSceneActionClockPuzzle = 7,
	kSceneActionKdShootingGallery = 8,
	kSceneActionGcCshPuzzle = 9,
	kSceneActionTableGatePuzzle = 10,
	kSceneActionCdInBookPuzzle = 11,
	kSceneActionBoardArrangementPuzzle = 12,
	kSceneActionKjBlobShooter = 13,
	kSceneActionEbz2sMenu = 14,
	kSceneActionMechiniCombat = 15,
	kSceneActionKeyGroupPuzzle = 16,
	kSceneActionDateSelectionPuzzle = 17,
	kSceneActionKiSkullMazePuzzle = 18,
	kSceneActionWebGridShiftPuzzle = 19,
	kSceneActionHorusWordPuzzle = 20,
	kSceneActionSixDigitCodePuzzle = 21,
	kSceneActionShockLeverPuzzle = 22,
	kSceneActionTarotCardPuzzle = 23,
	kSceneActionTubeSwitchScene = 24,
	kSceneActionKkTileMatchPuzzle = 25,
	kSceneActionRatiniCombat = 26,
	kSceneActionAtkiniCombat = 27,
	kSceneActionGymSelector = 28,
	kSceneActionCrystalPuzzle = 29,
	kSceneActionSetChooserTemplateMode = 30,
	kSceneActionNoOp = 31,
	kSceneActionClearDisplay = 32,
	kSceneActionStainedGlassPuzzle = 33,
	kSceneActionKeypadSequencePuzzle = 34,
	kSceneActionSetUiSelectionIndex = 35,
	kSceneActionDispatchUiSelection = 36,
	kSceneActionSetFrontEndActionMask = 37,
	kSceneActionBoardGame = 38,
	kSceneActionKaDialogue = 40,
	kSceneActionKbProgram = 41,
	kSceneActionKcOrWoffordProgram = 42,
	kSceneActionKdProgram = 43,
	kSceneActionKfProgram = 45,
	kSceneActionKgProgram = 46,
	kSceneActionKhProgram = 47,
	kSceneActionKiProgram = 48,
	kSceneActionKjProgram = 49,
	kSceneActionKkProgram = 50,
	kSceneActionKlProgram = 51,
	kSceneActionKmProgram = 52,
	kSceneActionKnProgram = 53,
	kSceneActionKpProgram = 54,
	kSceneActionKqProgram = 55,
	kSceneActionKrProgram = 56,
	kSceneActionKeyGroupPuzzleAlias = 60,
	kSceneActionEightButtonSequencePuzzle = 61,
	kSceneActionCainDialogue = 62,
	kSceneActionAppendNotebookText = 63,
	kSceneActionBriefing = 300,
	kSceneActionTerminateRuntime = 9999
};

class CompiledScript;
struct ScriptCommand;
class RipperEngine;

class SceneActionDispatcher {
public:
	static const char *actionName(uint action);
	static bool dispatch(RipperEngine *engine, const CompiledScript &script,
		const ScriptCommand &command);
};

} // End of namespace Ripper

#endif // RIPPER_SCENE_DISPATCHER_H
