/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software { you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation { either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY { without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program { if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#include "common/system.h"

#include "asylum/scriptman.h"
#include "asylum/shared.h"

DECLARE_SINGLETON(Asylum::ScriptManager);

namespace Asylum {

static bool g_initialized = false;

typedef int AsylumFunc(ActionCommand *cmd);

struct AsylumFunction {
	const char *name;
	AsylumFunc *function;
};

#define MAPFUNC(name, func) {name, func}

AsylumFunction function_map[] = {
	/*0x00*/ MAPFUNC("kReturn0", kReturn0),
	/*0x01*/ MAPFUNC("kSetGameFlag", kSetGameFlag),
	/*0x02*/ MAPFUNC("kClearGameFlag", kClearGameFlag),
	/*0x03*/ MAPFUNC("kToggleGameFlag", kToggleGameFlag),
	/*0x04*/ MAPFUNC("kJumpIfGameFlag", kJumpIfGameFlag),
	/*0x05*/ MAPFUNC("kHideCursor", kHideCursor),
	/*0x06*/ MAPFUNC("kShowCursor", kShowCursor),
	/*0x07*/ MAPFUNC("kPlayAnimation", kPlayAnimation),
	/*0x08*/ MAPFUNC("kMoveScenePosition", kMoveScenePosition),
	/*0x09*/ MAPFUNC("kHideActor", kHideActor),
	/*0x0A*/ MAPFUNC("kShowActor", kShowActor),
	/*0x0B*/ MAPFUNC("kSetActorStats", kSetActorStats),
	/*0x0C*/ MAPFUNC("kSetSceneMotionStat", kSetSceneMotionStat),
	/*0x0D*/ MAPFUNC("kDisableActor", kDisableActor),
	/*0x0E*/ MAPFUNC("kEnableActor", kEnableActor),
	/*0x0F*/ MAPFUNC("kEnableBarriers", kEnableBarriers),
	/*0x10*/ MAPFUNC("kReturn", kReturn),
	/*0x11*/ MAPFUNC("kDestroyBarrier", kDestroyBarrier),
	/*0x12*/ MAPFUNC("k_unk12_JMP_WALK_ACTOR", k_unk12_JMP_WALK_ACTOR),
	/*0x13*/ MAPFUNC("k_unk13_JMP_WALK_ACTOR", k_unk13_JMP_WALK_ACTOR),
	/*0x14*/ MAPFUNC("k_unk14_JMP_WALK_ACTOR", k_unk14_JMP_WALK_ACTOR),
	/*0x15*/ MAPFUNC("k_unk15", k_unk15),
	/*0x16*/ MAPFUNC("kResetAnimation", kResetAnimation),
	/*0x17*/ MAPFUNC("kClearFlag1Bit0", kClearFlag1Bit0),
	/*0x18*/ MAPFUNC("k_unk18_PLAY_SND", k_unk18_PLAY_SND),
	/*0x19*/ MAPFUNC("kJumpIfFlag2Bit0", kJumpIfFlag2Bit0),
	/*0x1A*/ MAPFUNC("kSetFlag2Bit0", kSetFlag2Bit0),
	/*0x1B*/ MAPFUNC("kClearFlag2Bit0", kClearFlag2Bit0),
	/*0x1C*/ MAPFUNC("kJumpIfFlag2Bit2", kJumpIfFlag2Bit2),
	/*0x1D*/ MAPFUNC("kSetFlag2Bit2", kSetFlag2Bit2),
	/*0x1E*/ MAPFUNC("kClearFlag2Bit2", kClearFlag2Bit2),
	/*0x1F*/ MAPFUNC("kJumpIfFlag2Bit1", kJumpIfFlag2Bit1),
	/*0x20*/ MAPFUNC("kSetFlag2Bit1", kSetFlag2Bit1),
	/*0x21*/ MAPFUNC("kClearFlag2Bit1", kClearFlag2Bit1),
	/*0x22*/ MAPFUNC("k_unk22", k_unk22),
	/*0x23*/ MAPFUNC("k_unk23", k_unk23),
	/*0x24*/ MAPFUNC("k_unk24", k_unk24),
	/*0x25*/ MAPFUNC("kRunEncounter", kRunEncounter),
	/*0x26*/ MAPFUNC("kJumpIfFlag2Bit4", kJumpIfFlag2Bit4),
	/*0x27*/ MAPFUNC("kSetFlag2Bit4", kSetFlag2Bit4),
	/*0x28*/ MAPFUNC("kClearFlag2Bit4", kClearFlag2Bit4),
	/*0x29*/ MAPFUNC("kSetActorField638", kSetActorField638),
	/*0x2A*/ MAPFUNC("kJumpIfActorField638", kJumpIfActorField638),
	/*0x2B*/ MAPFUNC("kChangeScene", kChangeScene),
	/*0x2C*/ MAPFUNC("k_unk2C_ActorSub", k_unk2C_ActorSub),
	/*0x2D*/ MAPFUNC("kPlayMovie", kPlayMovie),
	/*0x2E*/ MAPFUNC("kStopAllBarriersSounds", kStopAllBarriersSounds),
	/*0x2F*/ MAPFUNC("kSetActionFlag01", kSetActionFlag01),
	/*0x30*/ MAPFUNC("kClearActionFlag01", kClearActionFlag01),
	/*0x31*/ MAPFUNC("kResetSceneRect", kResetSceneRect),
	/*0x32*/ MAPFUNC("kChangeMusicById", kChangeMusicById),
	/*0x33*/ MAPFUNC("kStopMusic", kStopMusic),
	/*0x34*/ MAPFUNC("k_unk34_Status", k_unk34_Status),
	/*0x35*/ MAPFUNC("k_unk35", k_unk35),
	/*0x36*/ MAPFUNC("k_unk36", k_unk36),
	/*0x37*/ MAPFUNC("kRunBlowUpPuzzle", kRunBlowUpPuzzle),
	/*0x38*/ MAPFUNC("kJumpIfFlag2Bit3", kJumpIfFlag2Bit3),
	/*0x39*/ MAPFUNC("kSetFlag2Bit3", kSetFlag2Bit3),
	/*0x3A*/ MAPFUNC("kClearFlag2Bit3", kClearFlag2Bit3),
	/*0x3B*/ MAPFUNC("k_unk3B_PALETTE_MOD", k_unk3B_PALETTE_MOD),
	/*0x3C*/ MAPFUNC("k_unk3C_CMP_VAL", k_unk3C_CMP_VAL),
	/*0x3D*/ MAPFUNC("kWaitUntilFramePlayed", kWaitUntilFramePlayed),
	/*0x3E*/ MAPFUNC("kUpdateWideScreen", kUpdateWideScreen),
	/*0x3F*/ MAPFUNC("k_unk3F", k_unk3F),
	/*0x40*/ MAPFUNC("k_unk40_SOUND", k_unk40_SOUND),
	/*0x41*/ MAPFUNC("kPlaySpeech", kPlaySpeech),
	/*0x42*/ MAPFUNC("k_unk42", k_unk42),
	/*0x43*/ MAPFUNC("k_unk43", k_unk43),
	/*0x44*/ MAPFUNC("kPaletteFade", kPaletteFade),
	/*0x45*/ MAPFUNC("kStartPaletteFadeThread", kStartPaletteFadeThread),
	/*0x46*/ MAPFUNC("k_unk46", k_unk46),
	/*0x47*/ MAPFUNC("kActorFaceObject", kActorFaceObject),
	/*0x48*/ MAPFUNC("k_unk48_MATTE_01", k_unk48_MATTE_01),
	/*0x49*/ MAPFUNC("k_unk49_MATTE_90", k_unk49_MATTE_90),
	/*0x4A*/ MAPFUNC("kJumpIfSoundPlaying", kJumpIfSoundPlaying),
	/*0x4B*/ MAPFUNC("kChangePlayerCharacterIndex", kChangePlayerCharacterIndex),
	/*0x4C*/ MAPFUNC("kChangeActorField40", kChangeActorField40),
	/*0x4D*/ MAPFUNC("kStopSound", kStopSound),
	/*0x4E*/ MAPFUNC("k_unk4E_RANDOM_COMMAND", k_unk4E_RANDOM_COMMAND),
	/*0x4F*/ MAPFUNC("kClearScreen", kClearScreen),
	/*0x50*/ MAPFUNC("kQuit", kQuit),
	/*0x51*/ MAPFUNC("kJumpBarrierFrame", kJumpBarrierFrame),
	/*0x52*/ MAPFUNC("k_unk52", k_unk52),
	/*0x53*/ MAPFUNC("k_unk53", k_unk53),
	/*0x54*/ MAPFUNC("k_unk54_SET_ACTIONLIST_6EC", k_unk54_SET_ACTIONLIST_6EC),
	/*0x55*/ MAPFUNC("k_unk55", k_unk55),
	/*0x56*/ MAPFUNC("k_unk56", k_unk56),
	/*0x57*/ MAPFUNC("kSetResourcePalette", kSetResourcePalette),
	/*0x58*/ MAPFUNC("kSetBarrierFrameIdxFlaged", kSetBarrierFrameIdxFlaged),
	/*0x59*/ MAPFUNC("k_unk59", k_unk59),
	/*0x5A*/ MAPFUNC("k_unk5A", k_unk5A),
	/*0x5B*/ MAPFUNC("k_unk5B", k_unk5B),
	/*0x5C*/ MAPFUNC("k_unk5C", k_unk5C),
	/*0x5D*/ MAPFUNC("k_unk5D", k_unk5D),
	/*0x5E*/ MAPFUNC("k_unk5E", k_unk5E),
	/*0x5F*/ MAPFUNC("kSetBarrierLastFrameIdx", kSetBarrierLastFrameIdx),
	/*0x60*/ MAPFUNC("k_unk60_SET_OR_CLR_ACTIONAREA_FLAG", k_unk60_SET_OR_CLR_ACTIONAREA_FLAG),
	/*0x61*/ MAPFUNC("k_unk61", k_unk61),
	/*0x62*/ MAPFUNC("k_unk62_SHOW_OPTIONS_SCREEN", k_unk62_SHOW_OPTIONS_SCREEN),
	/*0x63*/ MAPFUNC("k_unk63", k_unk63)
};

ScriptManager::ScriptManager() {
	if (!g_initialized) {
		g_initialized     = true;
		currentLine       = 0;
		currentLoops      = 0;
		processing        = false;
		delayedSceneIndex = -1;
		delayedVideoIndex = -1;
		allowInput        = true;
	}
}

ScriptManager::~ScriptManager() {
	// TODO Release function_map list
}

void ScriptManager::setScript(ActionDefinitions *action) {
	_currentScript = action;

	if (_currentScript) {
		for (uint8 i = 0; i < _currentScript->commands[0].numLines; i++) {
			debugC(kDebugLevelScripts,
				"Line: %02d/%02d :: 0x%02X (%d, %d, %d, %d, %d, %d, %d, %d, %d)",
				i,
				_currentScript->commands[0].numLines - 1,
				_currentScript->commands[i].opcode,
				_currentScript->commands[i].param1,
				_currentScript->commands[i].param2,
				_currentScript->commands[i].param3,
				_currentScript->commands[i].param4,
				_currentScript->commands[i].param5,
				_currentScript->commands[i].param6,
				_currentScript->commands[i].param7,
				_currentScript->commands[i].param8,
				_currentScript->commands[i].param9);
		}
	}
}

void ScriptManager::setScriptIndex(uint32 index) {
	_currentScript = 0;
	currentLine    = 0;
	setScript(Shared.getScene()->getActionList(index));
}

int ScriptManager::checkBarrierFlags(int barrierId) {
    int flags = Shared.getScene()->getResources()->getBarrierById(barrierId)->flags;
    return flags & 1 && (flags & 8 || flags & 0x10000);
}

int ScriptManager::setBarrierNextFrame(int barrierId, int barrierFlags) {
    int barrierIndex = Shared.getScene()->getResources()->getBarrierIndexById(barrierId);

    BarrierItem *barrier = Shared.getScene()->getResources()->getBarrierByIndex(barrierIndex);
    int newFlag = barrierFlags | 1 | barrier->flags;
    barrier->flags |= barrierFlags | 1;

    if (newFlag & 0x10000) {
        barrier->frameIdx = barrier->frameCount - 1;
    } else {
        barrier->frameIdx = 0;
    }

    return barrierIndex;
}

int ScriptManager::processActionList() {
	done          = false;
	waitCycle     = false;
	lineIncrement = 1;
	processing    = true;

	if (_currentScript) {
		while (!done && !waitCycle) {
			lineIncrement = 1;      //Reset line increment value

			if (currentLoops > 1000) {
				//TODO - processActionLists has run too many iterations
			}

			ActionCommand *currentCommand = &_currentScript->commands[currentLine];

			uint32 opcode = currentCommand->opcode;

			// Execute command from function mapping
			int cmdRet = function_map[opcode].function(currentCommand);

			// Check function return
			if (cmdRet == -1)
				warning("Incomplete opcode %s (0x%02X) in Scene %d Line %d",
					function_map[opcode].name,
					currentCommand->opcode,
					Shared.getScene()->getSceneIndex(),
					currentLine);
			if (cmdRet == -2)
				warning("Unhandled opcode %s (0x%02X) in Scene %d Line %d",
					function_map[opcode].name,
					currentCommand->opcode,
					Shared.getScene()->getSceneIndex(),
					currentLine);

			currentLine += lineIncrement;
			currentLoops++;

		} // end while

		if (done) {
			currentLine    = 0;
			currentLoops   = 0;
			_currentScript = 0;

			// XXX
			// gameFlag 183 is the same as the
			// processing flag, but is not being used
			Shared.clearGameFlag(183);
		}
	}

	processing = false;

	return 0;
}

void ScriptManager::processActionListSub02(ActionDefinitions* script, ActionCommand *command, int a4) {
	int v4 = 0;
	int result;
	int barrierIdx = 0;
	if (a4) {
		if (a4 == 1) {
			;
		} else {
			result = a4 - 2;
			int v8 = command->param4;

			for (int i = 7; i > 0; i--) {
				barrierIdx = Shared.getScene()->getResources()->getBarrierIndexById(v8);
				if (barrierIdx >= 0)
					Shared.getScene()->getResources()->getWorldStats()->barriers[barrierIdx].field_67C = 0;
				v8 += 4;
			}
		}
		// TODO
		switch (Shared.getScene()->getSceneIndex()) {
		case 7:
			warning("Scene 7 / v4 != 0 Not Implemented");
			break;
		case 6:
			warning("Scene 6 / v4 != 0 Not Implemented");
			break;
		case 8:
			warning("Scene 8 / v4 != 0 Not Implemented");
			break;
		case 3:
			warning("Scene 3 / v4 != 0 Not Implemented");
			break;
		case 4:
			warning("Scene 4 / v4 != 0 Not Implemented");
			break;
		default:
			return;
		}
	} else {
		int v13 = command->param4;
		int v4 = script->counter / command->param2 + 4;
		for (int i = 7; i > 0; i--) {
				barrierIdx = Shared.getScene()->getResources()->getBarrierIndexById(v13);
				if (barrierIdx >= 0)
					Shared.getScene()->getResources()->getWorldStats()->barriers[barrierIdx].field_67C = v4;
				v13 += 4;
		}
		// TODO
		switch (Shared.getScene()->getSceneIndex()) {
		case 7:
			warning("Scene 7 / v4 = 0 Not Implemented");
			break;
		case 6:
			warning("Scene 6 / v4 = 0 Not Implemented");
			break;
		case 8:
			warning("Scene 8 / v4 = 0 Not Implemented");
			break;
		case 3:
			warning("Scene 3 / v4 = 0 Not Implemented");
			break;
		case 4:
			warning("Scene 4 / v4 = 0 Not Implemented");
			break;
		default:
				return;
		}
	}
}

void ScriptManager::enableActorSub(int actorIndex, int condition) {

}

/* Opcode Functions */

int kReturn0(ActionCommand *cmd) {
	ScriptMan.done          = true;
	ScriptMan.lineIncrement = 0;
	return 0;
}

int kSetGameFlag(ActionCommand *cmd) {
	int flagNum = cmd->param1;

	if (flagNum >= 0)
		Shared.setGameFlag(flagNum);

	return 0;
}

int kClearGameFlag(ActionCommand *cmd) {
	int flagNum = cmd->param1;

	if (flagNum >= 0)
		Shared.clearGameFlag(flagNum);

	return 0;
}

int kToggleGameFlag(ActionCommand *cmd) {
	int flagNum = cmd->param1;

	if (flagNum >= 0)
		Shared.toggleGameFlag(flagNum);

	return 0;
}

int kJumpIfGameFlag(ActionCommand *cmd) {
	int flagNum = cmd->param1;

	if (flagNum) {
		bool doJump = Shared.isGameFlagSet(flagNum);
		if (cmd->param2)
			doJump = Shared.isGameFlagNotSet(flagNum);
		if (doJump)
			ScriptMan.currentLine = cmd->param3;
	}

	return 0;
}

int kHideCursor(ActionCommand *cmd) {
	Shared.getScene()->getCursor()->hide();
	ScriptMan.allowInput = false;

	return 0;
}

int kShowCursor(ActionCommand *cmd) {
	Shared.getScene()->getCursor()->show();
	ScriptMan.allowInput = true;

	// TODO clear_flag_01()
	return -1;
}

int kPlayAnimation(ActionCommand *cmd) {
	int barrierId = cmd->param1;

	if (cmd->param2 == 2) {
		if (!ScriptMan.checkBarrierFlags(barrierId)) {
			cmd->param2 = 1;
			// FIXME Not sure why this break was here
			// break;
		}
		ScriptMan.lineIncrement = 1;
	} else {
		int barrierIndex = Shared.getScene()->getResources()->getBarrierIndexById(barrierId);
		BarrierItem *barrier = Shared.getScene()->getResources()->getBarrierByIndex(barrierIndex);

		if (cmd->param4) { // RECHECK THIS
			int newBarriedIndex = 213 * barrierIndex;
			barrier->flags &= 0xFFFEF1C7;
			Shared.getScene()->getResources()->getBarrierByIndex(newBarriedIndex)->flags = barrier->flags | 0x20;
		} else if (cmd->param3) {
			barrier->flags &= 0xFFFEF1C7;
			barrier->flags |= 0x10000;
		} else {
			barrier->flags &= 0x10000;
			if(barrier->flags == 0) {
				barrier->flags &= 0x10E38;
				if(barrier->flags == 0) {
					barrier->flags |= 8;
				}
			} else {
				barrier->flags |= 8;
				barrier->flags &= 0xFFFEFFFF;
			}
		}

		ScriptMan.setBarrierNextFrame(barrierId, barrier->flags);

		if(barrier->field_688 == 1) {
			// TODO: get barrier position
		}

		if(cmd->param2) {
			cmd->param2 = 2;
			ScriptMan.lineIncrement = 1;
		}
	}

	return -1;
}

int kMoveScenePosition(ActionCommand *cmd) {
	WorldStats   *ws = Shared.getScene()->getResources()->getWorldStats();
	Common::Rect *sr = &ws->sceneRects[ws->sceneRectIdx];

	if (cmd->param3 < 1) {
		ws->xLeft = cmd->param1;
		ws->yTop  = cmd->param2;
		ws->motionStatus = 3;
	} else if (!cmd->param4) {
		ws->motionStatus = 5;
		ws->targetX  = cmd->param1;
		ws->targetY  = cmd->param2;
		ws->field_A0 = cmd->param3;

		if (ws->targetX < (uint32)sr->left)
			ws->targetX = sr->left;
		if (ws->targetY < (uint32)sr->top)
			ws->targetY = sr->top;
		if (ws->targetX + 640 > (uint32)sr->right)
			ws->targetX = sr->right - 640;
		if (ws->targetY + 480 > (uint32)sr->bottom)
			ws->targetY = sr->bottom - 480;

		// TODO: reverse asm block

	} else if (cmd->param5) {
		if (ws->motionStatus == 2)
			ScriptMan.lineIncrement = 1;
		else
			cmd->param5 = 0;
	} else {
		cmd->param5 = 1;
		ws->motionStatus = 2;
		ws->targetX  = cmd->param1;
		ws->targetY  = cmd->param2;
		ws->field_A0 = cmd->param3;

		if (ws->targetX + 640 > ws->width)
			ws->targetX = ws->width - 640;
		if (ws->targetX < (uint32)sr->left)
			ws->targetX = sr->left;
		if (ws->targetY < (uint32)sr->top)
			ws->targetY = sr->top;
		if (ws->targetX + 640 > (uint32)sr->right)
			ws->targetX = sr->right - 640;
		if (ws->targetY + 480 > (uint32)sr->bottom)
			ws->targetY = sr->bottom - 480;

		// TODO: reverse asm block
	}

	return -1;
}

int kHideActor(ActionCommand *cmd) {
	uint32 actorIndex = 0;

	if (cmd->param1 == -1)
		;//actorIndex = Shared.getScene()->getWorldStats()->playerActor;
	else
		actorIndex = cmd->param1;

	if ((actorIndex >= 0) && (actorIndex < Shared.getScene()->getResources()->getWorldStats()->numActors))
		Shared.getScene()->getActor()->visible(false);
	else
		debugC(kDebugLevelScripts,
			"Requested invalid actor ID:0x%02X in Scene %d Line %d.",
			cmd->param1,
			Shared.getScene()->getSceneIndex(),
			ScriptMan.currentLine);

	return 0;
}

int kShowActor(ActionCommand *cmd) {
	uint32 actorIndex = 0;

	if (cmd->param1 == -1)
		;//actorIndex = Shared.getScene()->getWorldStats()->playerActor;
	else
		actorIndex = cmd->param1;

	if ((actorIndex >= 0) && (actorIndex < Shared.getScene()->getResources()->getWorldStats()->numActors))
		Shared.getScene()->getActor()->visible(true);
	else
		debugC(kDebugLevelScripts,
			"Requested invalid actor ID:0x%02X in Scene %d Line %d.",
			cmd->param1,
			Shared.getScene()->getSceneIndex(),
			ScriptMan.currentLine);
}

int kSetActorStats(ActionCommand *cmd) {
	uint32 actorIndex = 0;

	if (cmd->param1 == -1)
		;//actorIndex = Shared.getScene()->getWorldStats()->playerActor;
	else
		actorIndex = cmd->param1;

	if ((actorIndex >= 0) && (actorIndex < Shared.getScene()->getResources()->getWorldStats()->numActors)) {
		Shared.getScene()->getActor()->setPostion(cmd->param2, cmd->param3);
		Shared.getScene()->getActor()->setAction(cmd->param4);
	} else
		debugC(kDebugLevelScripts,
			"Requested invalid actor ID:0x%02X in Scene %d Script %d Line %d.",
			cmd->param1,
			Shared.getScene()->getSceneIndex(),
			ScriptMan.currentLine);

	return 0;
}

int kSetSceneMotionStat(ActionCommand *cmd) {
	Shared.getScene()->getResources()->getWorldStats()->motionStatus = cmd->param1;

	return 0;
}

int kDisableActor(ActionCommand *cmd) {
	int actorIndex = 0;

	if (cmd->param1 == -1)
		;//actorIndex = Shared.getScene()->getWorldStats()->playerActor;
	else
		actorIndex = cmd->param1;

	// TODO Finish implementing this function
	Shared.getScene()->getActor()->disable(actorIndex);

	return -1;
}

int kEnableActor(ActionCommand *cmd) {
	int actorIndex = 0;

	if (cmd->param1 == -1)
		;//actorIndex = Shared.getScene()->getWorldStats()->playerActor;
	else
		actorIndex = cmd->param1;

	if (Shared.getScene()->getResources()->getWorldStats()->actors[actorIndex].field_40 == 5)
		ScriptMan.enableActorSub(actorIndex, 4);

	return 0;
}

int kEnableBarriers(ActionCommand *cmd) {
	int    barIdx = Shared.getScene()->getResources()->getBarrierIndexById(cmd->param1);
	uint32 sndIdx = cmd->param3;
	uint32 v59    = cmd->param2;

	if (!ScriptMan.getScript()->counter && Shared.getScene()->getSceneIndex() != 13 && sndIdx != 0) {
		ResourcePack *sfx = new ResourcePack(18);
		Shared.getSound()->playSfx(sfx, ((unsigned int)(sndIdx != 0) & 5) + 0x80120001);
		delete sfx;
		//Shared.getSound()->playSfx(Shared.getScene()->getSpeechPack(),sndIdx + 86);
	}

	if (ScriptMan.getScript()->counter >= 3 * v59 - 1) {
		ScriptMan.getScript()->counter = 0;
		Shared.getScene()->getResources()->getWorldStats()->barriers[barIdx].field_67C = 0;
		ScriptMan.processActionListSub02(ScriptMan.getScript(), cmd, 2);
		ScriptMan.currentLoops = 1; // v4 = 1;
	} else {
		int v64;
		int v62 = ScriptMan.getScript()->counter + 1;
		ScriptMan.getScript()->counter = v62;
		if (sndIdx) {
				v64 = 1;
				int v170 = 3 - v62 / v59;
				Shared.getScene()->getResources()->getWorldStats()->barriers[barIdx].field_67C = v170;
		} else {
				v64 = 0;
				Shared.getScene()->getResources()->getWorldStats()->barriers[barIdx].field_67C = v62 / v59 + 1;
		}

		ScriptMan.processActionListSub02(ScriptMan.getScript(), cmd, v64);
	}

	return -1;
}

int kReturn(ActionCommand *cmd) {
	ScriptMan.done          = true;
	ScriptMan.lineIncrement = 0;

	return 0;
}

int kDestroyBarrier(ActionCommand *cmd) {
	BarrierItem *barrier = Shared.getScene()->getResources()->getBarrierById(cmd->param1);

	if (barrier) {
		barrier->flags &= 0xFFFFFFFE;
		barrier->flags |= 0x20000;
		Shared.getScreen()->deleteGraphicFromQueue(barrier->resId);
	} else
		debugC(kDebugLevelScripts,
			"Requested invalid object ID:0x%02X in Scene %d Line %d.",
			cmd->param1,
			Shared.getScene()->getSceneIndex(),
			ScriptMan.currentLine);

	return 0;
}

int k_unk12_JMP_WALK_ACTOR(ActionCommand *cmd) {
	return -2;
}
int k_unk13_JMP_WALK_ACTOR(ActionCommand *cmd) {
	return -2;
}
int k_unk14_JMP_WALK_ACTOR(ActionCommand *cmd) {
	return -2;
}
int k_unk15(ActionCommand *cmd) {
	return -2;
}

int kResetAnimation(ActionCommand *cmd) {
	BarrierItem *barrier = Shared.getScene()->getResources()->getBarrierById(cmd->param1);

	if ((barrier->flags & 0x10000) == 0)
		barrier->frameIdx = 0;
	else
		barrier->frameIdx = barrier->frameCount - 1;

	return 0;
}

int kClearFlag1Bit0(ActionCommand *cmd) {
	BarrierItem *barrier = Shared.getScene()->getResources()->getBarrierById(cmd->param1);

	barrier->flags &= 0xFFFFFFFE;

	return 0;
}

int k_unk18_PLAY_SND(ActionCommand *cmd) {
	return -2;
}

int kJumpIfFlag2Bit0(ActionCommand *cmd) {
	int targetType = cmd->param2;

	return 0;

	// TODO targetType == 7 is trying to access an out of bounds actor
	// look at the disassembly again

	if (targetType <= 0)
		ScriptMan.done = (Shared.getScene()->getResources()->getBarrierById(cmd->param1)->flags2 & 1) == 0;
	else
		if (targetType == 1) // v4 == 1, so 1
			ScriptMan.done = (Shared.getScene()->getResources()->getActionAreaById(cmd->param1)->actionType & 1) == 0;
		else
			ScriptMan.done = (Shared.getScene()->getResources()->getWorldStats()->actors[cmd->param1].flags2 & 1) == 0;

	return -1;
}

int kSetFlag2Bit0(ActionCommand *cmd) {
	int targetType = cmd->param2;

	if (targetType == 2)
		Shared.getScene()->getResources()->getWorldStats()->actors[cmd->param1].flags2 |= 1;
	else
		if (targetType == 1)
			Shared.getScene()->getResources()->getActionAreaById(cmd->param1)->actionType |= 1;
		else
			Shared.getScene()->getResources()->getBarrierById(cmd->param1)->flags2 |= 1;

	return 0;
}

int kClearFlag2Bit0(ActionCommand *cmd) {
	return -2;
}
int kJumpIfFlag2Bit2(ActionCommand *cmd) {
	return -2;
}
int kSetFlag2Bit2(ActionCommand *cmd) {
	return -2;
}
int kClearFlag2Bit2(ActionCommand *cmd) {
	return -2;
}
int kJumpIfFlag2Bit1(ActionCommand *cmd) {
	return -2;
}
int kSetFlag2Bit1(ActionCommand *cmd) {
	return -2;
}
int kClearFlag2Bit1(ActionCommand *cmd) {
	return -2;
}
int k_unk22(ActionCommand *cmd) {
	return -2;
}
int k_unk23(ActionCommand *cmd) {
	return -2;
}
int k_unk24(ActionCommand *cmd) {
	return -2;
}
int kRunEncounter(ActionCommand *cmd) {
	return -2;
}
int kJumpIfFlag2Bit4(ActionCommand *cmd) {
	return -2;
}
int kSetFlag2Bit4(ActionCommand *cmd) {
	return -2;
}
int kClearFlag2Bit4(ActionCommand *cmd) {
	return -2;
}
int kSetActorField638(ActionCommand *cmd) {
	return -2;
}
int kJumpIfActorField638(ActionCommand *cmd) {
	return -2;
}

int kChangeScene(ActionCommand *cmd) {
	ScriptMan.delayedSceneIndex = cmd->param1 + 4;
	debug(kDebugLevelScripts,
		"Queueing Scene Change to scene %d...",
		ScriptMan.delayedSceneIndex);

	return 0;
}

int k_unk2C_ActorSub(ActionCommand *cmd) {
	return -2;
}

int kPlayMovie(ActionCommand *cmd) {
	// TODO: add missing code here
	ScriptMan.delayedVideoIndex = cmd->param1;

	return -1;
}

int kStopAllBarriersSounds(ActionCommand *cmd) {
	// TODO: do this for all barriers that have sfx playing
	Shared.getSound()->stopSfx();

	return -1;
}

int kSetActionFlag01(ActionCommand *cmd) {
	return -2;
}
int kClearActionFlag01(ActionCommand *cmd) {
	return -2;
}
int kResetSceneRect(ActionCommand *cmd) {
	return -2;
}
int kChangeMusicById(ActionCommand *cmd) {
	return -2;
}

int kStopMusic(ActionCommand *cmd) {
	Shared.getSound()->stopMusic();

	return 0;
}

int k_unk34_Status(ActionCommand *cmd) {
	if (cmd->param1 >= 2) {
		cmd->param1 = 0;
	} else {
		cmd->param1++;
		ScriptMan.lineIncrement = 1;
	}

	return 0;
}

int k_unk35(ActionCommand *cmd) {
	return -2;
}
int k_unk36(ActionCommand *cmd) {
	return -2;
}

int kRunBlowUpPuzzle(ActionCommand *cmd) {
	// FIXME: improve this to call other blowUpPuzzles than VCR
	//int puzzleIdx = cmd->param1;

	Shared.getScene()->setBlowUpPuzzle(new BlowUpPuzzleVCR());
	Shared.getScene()->getBlowUpPuzzle()->openBlowUp();

	return -1;
}

int kJumpIfFlag2Bit3(ActionCommand *cmd) {
	return -2;
}
int kSetFlag2Bit3(ActionCommand *cmd) {
	return -2;
}
int kClearFlag2Bit3(ActionCommand *cmd) {
	return -2;
}
int k_unk3B_PALETTE_MOD(ActionCommand *cmd) {
	return -2;
}

int k_unk3C_CMP_VAL(ActionCommand *cmd) {
	if (cmd->param1) {
		if (cmd->param2 >= cmd->param1) {
			cmd->param2 = 0;
		} else {
			cmd->param2++;
			ScriptMan.lineIncrement = 1;
		}
	}

	return 0;
}

int kWaitUntilFramePlayed(ActionCommand *cmd) {
	BarrierItem *barrier = Shared.getScene()->getResources()->getBarrierById(cmd->param1);

	if (barrier) {
			uint32 frameNum = 0;
			if (cmd->param2 == -1)
				frameNum = barrier->frameCount - 1;
			else
				frameNum = cmd->param2;

			if (barrier->frameIdx < frameNum) {
				ScriptMan.lineIncrement = 0;
				ScriptMan.waitCycle     = true;
			}
	} else
		debugC(kDebugLevelScripts,
			"Requested invalid object ID:0x%02X in Scene %d Line %d.",
			cmd->param1,
			Shared.getScene()->getSceneIndex(),
			ScriptMan.currentLine);

	return 0;
}

int kUpdateWideScreen(ActionCommand *cmd) {
	int barSize = cmd->param1;

	if (barSize >= 22) {
		cmd->param1 = 0;
	} else {
		Shared.getScreen()->drawWideScreen(4 * barSize);
		cmd->param1++;
	}

	return 0;
}

int k_unk3F(ActionCommand *cmd) {
	return -2;
}
int k_unk40_SOUND(ActionCommand *cmd) {
	return -2;
}

int kPlaySpeech(ActionCommand *cmd) {
	 //TODO - Add support for other param options
	uint32 sndIdx = cmd->param1;

	if ((int)sndIdx >= 0) {
		if (sndIdx >= 259) {
			sndIdx -= 9;
			Shared.getSound()->playSfx(Shared.getScene()->getSpeechPack(), sndIdx - 0x7FFD0000);
		} else {
			Shared.getSound()->playSfx(Shared.getScene()->getSpeechPack(), sndIdx);
		}
	} else
		debugC(kDebugLevelScripts,
				"Requested invalid sound ID:0x%02X in Scene %d Line %d.",
				cmd->param1,
				Shared.getScene()->getSceneIndex(),
				ScriptMan.currentLine);

	return -1;
}

int k_unk42(ActionCommand *cmd) {
	return -2;
}
int k_unk43(ActionCommand *cmd) {
	return -2;
}
int kPaletteFade(ActionCommand *cmd) {
	return -2;
}
int kStartPaletteFadeThread(ActionCommand *cmd) {
	return -2;
}
int k_unk46(ActionCommand *cmd) {
	return -2;
}

int kActorFaceObject(ActionCommand *cmd) {
	// XXX
	// Dropping param1, since it's the character index
	// Investigate further if/when we have a scene with
	// multiple characters in the actor[] array
	Shared.getScene()->getActor()->faceTarget(cmd->param2, cmd->param3);

	return -1;
}

int k_unk48_MATTE_01(ActionCommand *cmd) {
	return -2;
}
int k_unk49_MATTE_90(ActionCommand *cmd) {
	return -2;
}
int kJumpIfSoundPlaying(ActionCommand *cmd) {
	return -2;
}
int kChangePlayerCharacterIndex(ActionCommand *cmd) {
	return -2;
}

int kChangeActorField40(ActionCommand *cmd) {
	// TODO: figure out what is this field and what values are set
	int actorIdx  = cmd->param1;
	int fieldType = cmd->param2;

	if (fieldType) {
		if (Shared.getScene()->getResources()->getWorldStats()->actors[actorIdx].field_40 < 11)
			Shared.getScene()->getResources()->getWorldStats()->actors[actorIdx].field_40 = 14;
	} else {
		Shared.getScene()->getResources()->getWorldStats()->actors[actorIdx].field_40 = 4;
	}

	return -1;
}

int kStopSound(ActionCommand *cmd) {
	return -2;
}
int k_unk4E_RANDOM_COMMAND(ActionCommand *cmd) {
	return -2;
}

int kClearScreen(ActionCommand *cmd) {
	if (cmd->param1)
		Shared.getScreen()->clearScreen();

	return 0;
}

int kQuit(ActionCommand *cmd) {
	return -2;
}

int kJumpBarrierFrame(ActionCommand *cmd) {
	BarrierItem *barrier = Shared.getScene()->getResources()->getBarrierById(cmd->param1);

	if (cmd->param2 == -1)
		cmd->param2 = barrier->frameCount - 1;

	if (cmd->param3 && cmd->param2 == barrier->frameIdx) {
		//break;
	} else if (cmd->param4 && cmd->param2 < barrier->frameIdx) {
		//break;
	} else if (cmd->param5 && cmd->param2 > barrier->frameIdx) {
		//break;
	} else if (cmd->param6 && cmd->param2 <= barrier->frameIdx) {
		//break;
	} else if (cmd->param7 && cmd->param2 >= barrier->frameIdx) {
		//break;
	} else if (cmd->param8 && cmd->param2 != barrier->frameIdx) {
		//break;
	}

	ActionCommand *nextCmd = &ScriptMan.getScript()->commands[cmd->param9];

	// 0x10 == kReturn
	if (nextCmd->opcode != 0x10 && nextCmd->opcode)
		ScriptMan.done = true;

	return 0;
}

int k_unk52(ActionCommand *cmd) {
	return -2;
}
int k_unk53(ActionCommand *cmd) {
	return -2;
}

int k_unk54_SET_ACTIONLIST_6EC(ActionCommand *cmd) {
	if (cmd->param2)
		ScriptMan.getScript()->field_1BB0 = rand() % cmd->param1;
	else
		ScriptMan.getScript()->field_1BB0 = cmd->param1;

	return 0;
}

int k_unk55(ActionCommand *cmd) {
	// TODO
	/*
	if (!cmd->param2) {
		if (cmd->param3 && ScriptMan.getScript()->field_1BB0 < cmd->param1)
			//break;
		else if (cmd->param4 && ScriptMan.getScript()->field_1BB0 > cmd->param1)
			//break;
		else if (cmd->param5 && ScriptMan.getScript()->field_1BB0 <= cmd->param1)
			//break;
		else if (cmd->param6 && ScriptMan.getScript()->field_1BB0 >= cmd->param1)
			//break;
		else if (cmd->param7 && ScriptMan.getScript()->field_1BB0 != cmd->param1)
			//break;
	} else if(ScriptMan.getScript()->field_1BB0 == cmd->param1) {
		//break;
	}
	*/

	ActionCommand *nextCmd = &ScriptMan.getScript()->commands[cmd->param8];

	if (nextCmd->opcode != 0x10 && nextCmd->opcode)
		ScriptMan.done = true;
	else
		ScriptMan.lineIncrement = cmd->param8;

	return -1;
}

int k_unk56(ActionCommand *cmd) {
	return -2;
}

int kSetResourcePalette(ActionCommand *cmd) {
	if (cmd->param1 > 0)
		Shared.getScreen()->setPalette(Shared.getScene()->getResourcePack(), Shared.getScene()->getResources()->getWorldStats()->grResId[cmd->param1]);

	return 0;
}

int kSetBarrierFrameIdxFlaged(ActionCommand *cmd) {
	BarrierItem *barrier = Shared.getScene()->getResources()->getBarrierById(cmd->param1);

	if (cmd->param3)
		barrier->flags = 1 | barrier->flags;
	else
		barrier->flags = barrier->flags & 0xFFFFFFFE;

	barrier->frameIdx = cmd->param2;

	return 0;
}

int k_unk59(ActionCommand *cmd) {

}
int k_unk5A(ActionCommand *cmd) {
	return -2;
}
int k_unk5B(ActionCommand *cmd) {
	return -2;
}
int k_unk5C(ActionCommand *cmd) {
	return -2;
}
int k_unk5D(ActionCommand *cmd) {
	return -2;
}
int k_unk5E(ActionCommand *cmd) {
	return -2;
}

int kSetBarrierLastFrameIdx(ActionCommand *cmd) {
	BarrierItem *barrier = Shared.getScene()->getResources()->getBarrierById(cmd->param1);

	if (barrier->frameIdx == barrier->frameCount - 1) {
		ScriptMan.lineIncrement = 0;
		barrier->flags &= 0xFFFEF1C7;
	} else {
		ScriptMan.lineIncrement = 1;
	}

	return 0;
}

int k_unk60_SET_OR_CLR_ACTIONAREA_FLAG(ActionCommand *cmd) {
	return -2;
}

int k_unk61(ActionCommand *cmd) {
	if (cmd->param2) {
		if (Shared.getScene()->getResources()->getWorldStats()->field_E860C == -1) {
			ScriptMan.lineIncrement = 0;
			cmd->param2   = 0;
		} else {
			ScriptMan.lineIncrement = 1;
		}
	} else {
		// TODO: do something for scene number 9
		cmd->param2 = 1;
		ScriptMan.lineIncrement = 1;
	}

	return -1;
}

int k_unk62_SHOW_OPTIONS_SCREEN(ActionCommand *cmd) {
	return -2;
}
int k_unk63(ActionCommand *cmd) {
	return -2;
}

} // end of namespace Asylum
