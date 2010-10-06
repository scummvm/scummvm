/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "asylum/resources/actionlist.h"
#include "asylum/system/config.h"

namespace Asylum {

ActionList::ActionList(Common::SeekableReadStream *stream, Scene *scene)
		: _scene(scene) {
	load(stream);

	_currentScript    = 0;
	currentLine       = 0;
	currentLoops      = 0;
	delayedSceneIndex = -1;
	delayedVideoIndex = -1;
	allowInput        = true;
	_actionFlag       = false;
	resetQueue();
}

ActionList::~ActionList() {
	entries.clear();
	_scripts.clear();
}

typedef int AsylumFunc(Script *script, ScriptEntry *cmd, Scene *scn);

struct AsylumFunction {
	const char *name;
	AsylumFunc *function;
};

#define MAPFUNC(name, func) {name, func}

// TODO I don't know that we're clearing this out
// when the engine is disposed. Need to look into this
// as a possible memory leak.
static const AsylumFunction function_map[] = {
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
	/*0x0B*/ MAPFUNC("kSetActorPosition", kSetActorPosition),
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
	/*0x2F*/ MAPFUNC("kSetActionFlag", kSetActionFlag),
	/*0x30*/ MAPFUNC("kClearActionFlag", kClearActionFlag),
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
	/*0x5E*/ MAPFUNC("kClearActorField970", kClearActorField970),
	/*0x5F*/ MAPFUNC("kSetBarrierLastFrameIdx", kSetBarrierLastFrameIdx),
	/*0x60*/ MAPFUNC("k_unk60_SET_OR_CLR_ACTIONAREA_FLAG", k_unk60_SET_OR_CLR_ACTIONAREA_FLAG),
	/*0x61*/ MAPFUNC("k_unk61", k_unk61),
	/*0x62*/ MAPFUNC("k_unk62_SHOW_OPTIONS_SCREEN", k_unk62_SHOW_OPTIONS_SCREEN),
	/*0x63*/ MAPFUNC("k_unk63", k_unk63)
};

#undef MAPFUNC

void ActionList::resetQueue() {
	_scripts.clear();
}

void ActionList::queueScript(int actionIndex, int actorIndex) {
	// TODO properly define what actionFlag is actually for.
	// It appears to remain false 99% of the time, so I'm guessing
	// it's a "skip processing" flag.
	if (!_actionFlag) {
		ScriptQueueEntry entry;
		entry.actionListIndex = actionIndex;
		entry.actorIndex      = actorIndex;

		// If there's currently no script for the processor to run,
		// assign it directly and skip the stack push. If however the
		// current script is assigned, push the script to the stack
		if (_currentScript)
			_scripts.push(entry);
		else {
			_currentQueueEntry = entry;
			_currentScript = &entries[entry.actionListIndex];
		}
	}
}

int ActionList::process() {
	done          = false;
	waitCycle     = false;
	lineIncrement = 1;

	_scene->vm()->setGameFlag(183);

	if (_currentScript)
		while (!done && !waitCycle) {
			lineIncrement = 0; //Reset line increment value

			ScriptEntry *currentCommand = &_currentScript->commands[currentLine];

			int32 opcode = currentCommand->opcode;

			debugC(kDebugLevelScripts,
			   "[0x%02X] %s(%d, %d, %d, %d, %d, %d, %d, %d, %d)",
			   opcode,function_map[opcode].name,
			   currentCommand->param1,
			   currentCommand->param2,
			   currentCommand->param3,
			   currentCommand->param4,
			   currentCommand->param5,
			   currentCommand->param6,
			   currentCommand->param7,
			   currentCommand->param8,
			   currentCommand->param9);

			// Execute command from function mapping
			int cmdRet = function_map[opcode].function(_currentScript, currentCommand, _scene);

			// Check function return
			if (cmdRet == -1)
				debugC(kDebugLevelScripts,
					"Incomplete opcode %s (0x%02X) in Scene %d Line %d",
					function_map[opcode].name,
					currentCommand->opcode,
					_scene->getSceneIndex(),
					currentLine);
			if (cmdRet == -2)
				debugC(kDebugLevelScripts,
					"Unhandled opcode %s (0x%02X) in Scene %d Line %d",
					function_map[opcode].name,
					currentCommand->opcode,
					_scene->getSceneIndex(),
					currentLine);
			if (cmdRet == -3)
				debugC(kDebugLevelScripts,
					"Flagged (see implementation comments) opcode %s (0x%02X) in Scene %d Line %d",
					function_map[opcode].name,
					currentCommand->opcode,
					_scene->getSceneIndex(),
					currentLine);

			if (!lineIncrement) {
				currentLine ++;
			}

		} // end while

		if (done) {
			currentLine  = 0;
			if (!_scripts.empty()) {
				_currentQueueEntry = _scripts.pop();
				_currentScript = &entries[_currentQueueEntry.actionListIndex];
			} else {
				_currentScript = 0;
		}

	}

	_scene->vm()->clearGameFlag(183);
	return 0;
}

void ActionList::load(Common::SeekableReadStream *stream) {
	size       = stream->readSint32LE();
	numEntries = stream->readSint32LE();

	for (int32 a = 0; a < numEntries; a++) {
		Script action;
		memset(&action, 0, sizeof(Script));

		for (int32 c = 0; c < MAX_ACTION_COMMANDS; c++) {
			ScriptEntry command;
			memset(&command, 0, sizeof(ScriptEntry));

			command.numLines = stream->readSint32LE();
			command.opcode   = stream->readSint32LE();
			command.param1   = stream->readSint32LE();
			command.param2   = stream->readSint32LE();
			command.param3   = stream->readSint32LE();
			command.param4   = stream->readSint32LE();
			command.param5   = stream->readSint32LE();
			command.param6   = stream->readSint32LE();
			command.param7   = stream->readSint32LE();
			command.param8   = stream->readSint32LE();
			command.param9   = stream->readSint32LE();

			action.commands[c] = command;
		}

		action.field_1BAC = stream->readSint32LE();
		action.field_1BB0 = stream->readSint32LE();
		action.counter    = stream->readSint32LE();

		entries.push_back(action);
	}
}

/* Opcode Functions */

int kReturn0(Script *script, ScriptEntry *cmd, Scene *scn) {
	scn->actions()->done          = true;
	scn->actions()->lineIncrement = 0;
	return 0;
}

int kSetGameFlag(Script *script, ScriptEntry *cmd, Scene *scn) {
	int flagNum = cmd->param1;

	if (flagNum >= 0)
		scn->vm()->setGameFlag(flagNum);

	return 0;
}

int kClearGameFlag(Script *script, ScriptEntry *cmd, Scene *scn) {
	int flagNum = cmd->param1;

	if (flagNum >= 0)
		scn->vm()->clearGameFlag(flagNum);

	return 0;
}

int kToggleGameFlag(Script *script, ScriptEntry *cmd, Scene *scn) {
	int flagNum = cmd->param1;

	if (flagNum >= 0)
		scn->vm()->toggleGameFlag(flagNum);

	return 0;
}

int kJumpIfGameFlag(Script *script, ScriptEntry *cmd, Scene *scn) {
	int flagNum = cmd->param1;

	if (flagNum) {
		bool doJump = scn->vm()->isGameFlagSet(flagNum);
		if (cmd->param2)
			doJump = scn->vm()->isGameFlagNotSet(flagNum);
		if (doJump)
			scn->actions()->currentLine = cmd->param3;
	}

	return 0;
}

int kHideCursor(Script *script, ScriptEntry *cmd, Scene *scn) {
	scn->getCursor()->hide();
	scn->actions()->allowInput = false;

	return 0;
}

int kShowCursor(Script *script, ScriptEntry *cmd, Scene *scn) {
	scn->getCursor()->show();
	scn->actions()->allowInput = true;

	// TODO clear_flag_01()
	return -1;
}

int kPlayAnimation(Script *script, ScriptEntry *cmd, Scene *scn) {
	int barrierId    = cmd->param1;
	int barrierIndex = scn->worldstats()->getBarrierIndexById(barrierId);
	Barrier *barrier = scn->worldstats()->getBarrierByIndex(barrierIndex);

	if (cmd->param2 == 2) {
		if (!barrier->checkFlags()) {
			cmd->param2 = 1;
		}
		scn->actions()->lineIncrement = 1;
	} else {
		if (cmd->param4) {
			barrier->flags &= 0xFFFEF1C7;
            barrier->flags |= 0x20;
		} else if (cmd->param3) {
			barrier->flags &= 0xFFFEF1C7;
			barrier->flags |= 0x10000;
		} else {
			barrier->flags &= 0x10000;
			if (barrier->flags == 0) {
				barrier->flags &= 0x10E38;
				if (barrier->flags == 0) {
					barrier->flags |= 8;
				}
			} else {
				barrier->flags |= 8;
				barrier->flags &= 0xFFFEFFFF;
			}
		}

		barrier->setNextFrame(barrier->flags);

		if (barrier->field_688 == 1) {
			// TODO: get barrier position
		}

		if (cmd->param2) {
			cmd->param2 = 2;
			scn->actions()->lineIncrement = 1;
		}
	}

	return -1;
}

int kMoveScenePosition(Script *script, ScriptEntry *cmd, Scene *scn) {
	WorldStats   *ws = scn->worldstats();
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

		if (ws->targetX < sr->left)
			ws->targetX = sr->left;
		if (ws->targetY < sr->top)
			ws->targetY = sr->top;
		if (ws->targetX + 640 > sr->right)
			ws->targetX = sr->right - 640;
		if (ws->targetY + 480 > sr->bottom)
			ws->targetY = sr->bottom - 480;

		// TODO: reverse asm block

	} else if (cmd->param5) {
		if (ws->motionStatus == 2)
			scn->actions()->lineIncrement = 1;
		else
			cmd->param5 = 0;
	} else {
		cmd->param5 = 1;
		ws->motionStatus = 2;
		ws->targetX  = cmd->param1;
		ws->targetY  = cmd->param2;
		ws->field_A0 = cmd->param3;

		if (ws->targetX + 640 > (int)ws->width)
			ws->targetX = ws->width - 640;
		if (ws->targetX < sr->left)
			ws->targetX = sr->left;
		if (ws->targetY < sr->top)
			ws->targetY = sr->top;
		if (ws->targetX + 640 > sr->right)
			ws->targetX = sr->right - 640;
		if (ws->targetY + 480 > sr->bottom)
			ws->targetY = sr->bottom - 480;

		// TODO: reverse asm block
	}

	return -1;
}

int kHideActor(Script *script, ScriptEntry *cmd, Scene *scn) {
	Actor *actor = 0;
	actor = (cmd->param1 == -1) ?
			scn->getActor() :
			&scn->worldstats()->actors[cmd->param1];

	actor->visible(false);
	actor->updateActor_401320();

	return -1;
}

int kShowActor(Script *script, ScriptEntry *cmd, Scene *scn) {
	Actor *actor = 0;

	// TODO revisit when actor selection is cleaned up
	if (cmd->param1 == -1)
		actor = scn->getActor();
	else
		actor = &scn->worldstats()->actors[cmd->param1];

	actor->visible(true);
	actor->updateActor_401320();
	actor->tickValue1 = scn->vm()->getTick();

	return 0;
}

int kSetActorPosition(Script *script, ScriptEntry *cmd, Scene *scn) {
	Actor *act = scn->getActor(cmd->param1);
	act->setPosition(cmd->param2, cmd->param3, cmd->param4, cmd->param5);

	return 0;
}

int kSetSceneMotionStat(Script *script, ScriptEntry *cmd, Scene *scn) {
	scn->worldstats()->motionStatus = cmd->param1;

	return 0;
}

int kDisableActor(Script *script, ScriptEntry *cmd, Scene *scn) {
	int32 actorIndex = (cmd->param1 == -1) ? 0 : cmd->param1;
	Actor *act = scn->getActor(actorIndex);

	if (cmd->param5 != 2) {
		if (act->updateType != 2 && act->updateType != 13) {
			if (cmd->param2 != -1 || cmd->param3 != -1)
				scn->updateActorDirection(actorIndex, cmd->param4);
			else {
				if ((act->x1 + act->x2) != cmd->param2 ||
					(act->y1 + act->y2) != cmd->param3) {
					; // TODO updatecharacter_sub_408910
					// TODO if (cmd->param5)
					// cmd->param5 = 2
					// v245 = true
				} else
					scn->updateActorDirection(actorIndex, cmd->param4);
			}
		}
		return -1;
	}

	if (act->updateType != 2 && act->updateType != 13) {
		cmd->param5 = 1;
		// v245 = false
		if ((act->x1 + act->x2) != cmd->param2 ||
			(act->y1 + act->y2) != cmd->param3) {
			scn->updateActorDirection(actorIndex, cmd->param4);
		}
	}
	return -1;
}

int kEnableActor(Script *script, ScriptEntry *cmd, Scene *scn) {
	int actorIndex = 0;

	if (cmd->param1 == -1)
		;//actorIndex = scn->getWorldStats()->playerActor;
	else
		actorIndex = cmd->param1;

	/* TODO implement enableActorSub()
	if (scn->worldstats()->actors[actorIndex].updateType == 5)
		scn->actions()->enableActorSub(actorIndex, 4);
	*/

	return -1;
}

int kEnableBarriers(Script *script, ScriptEntry *cmd, Scene *scn) {
	int32 barIdx = scn->worldstats()->getBarrierIndexById(cmd->param1);
	Barrier *bar = &scn->worldstats()->barriers[barIdx];
	int32 sndIdx = cmd->param3;

	if (!script->counter && scn->getSceneIndex() != 13 && sndIdx != 0) {
		// FIXME: I really don't understand what (sndIdx != 0) & 5 is supposed to be doing here,
		// but this is effectively trying to do a boolean AND operation on a boolean variable
		// which is odd, and wrong. Changing it to (sndIdx & 5), for now
		//scn->vm()->sound()->playSound(((sndIdx != 0) & 5) + 0x80120001,
		scn->vm()->sound()->playSound((sndIdx & 5) + 0x80120001, false, Config.sfxVolume, 0);
	}

	if (script->counter >= (3 * cmd->param2 - 1)) {
		script->counter = 0;
		bar->field_67C  = 0;
		// TODO scn->actions()->processActionListSub02(script, cmd, 2);
	} else {
		int v64; // XXX rename when processActionListSub02 is better implemented
		script->counter += 1;
		if (sndIdx) {
			v64 = 1;
			bar->field_67C = 3 - script->counter / cmd->param2;
		} else {
			v64 = 0;
			bar->field_67C = script->counter / cmd->param2 + 1;
		}

		// TODO scn->actions()->processActionListSub02(script, cmd, v64);
	}

	return -1;
}

int kReturn(Script *script, ScriptEntry *cmd, Scene *scn) {
	scn->actions()->done          = true;
	scn->actions()->lineIncrement = 0;

	return 0;
}

int kDestroyBarrier(Script *script, ScriptEntry *cmd, Scene *scn) {
	Barrier *barrier = scn->worldstats()->getBarrierById(cmd->param1);

	if (barrier) {
		barrier->flags &= 0xFFFFFFFE;
		barrier->flags |= 0x20000;
		scn->vm()->screen()->deleteGraphicFromQueue(barrier->resId);
	} else
		debugC(kDebugLevelScripts,
		       "Requested invalid object ID:0x%02X in Scene %d Line %d.",
		       cmd->param1,
		       scn->getSceneIndex(),
		       scn->actions()->currentLine);

	return 0;
}

int k_unk12_JMP_WALK_ACTOR(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int k_unk13_JMP_WALK_ACTOR(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int k_unk14_JMP_WALK_ACTOR(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int k_unk15(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}

int kResetAnimation(Script *script, ScriptEntry *cmd, Scene *scn) {
	Barrier *barrier = scn->worldstats()->getBarrierById(cmd->param1);

	if ((barrier->flags & 0x10000) == 0)
		barrier->frameIdx = 0;
	else
		barrier->frameIdx = barrier->frameCount - 1;

	return 0;
}

int kClearFlag1Bit0(Script *script, ScriptEntry *cmd, Scene *scn) {
	Barrier *barrier = scn->worldstats()->getBarrierById(cmd->param1);

	barrier->flags &= 0xFFFFFFFE;

	return 0;
}

int k_unk18_PLAY_SND(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}

int kJumpIfFlag2Bit0(Script *script, ScriptEntry *cmd, Scene *scn) {
	int targetType = cmd->param2;

	return 0;

	// TODO targetType == 7 is trying to access an out of bounds actor
	// look at the disassembly again

	if (targetType <= 0)
		scn->actions()->done = (scn->worldstats()->getBarrierById(cmd->param1)->flags2 & 1) == 0;
	else
		if (targetType == 1) // v4 == 1, so 1
			scn->actions()->done = (scn->worldstats()->getActionAreaById(cmd->param1)->actionType & 1) == 0;
		else
			scn->actions()->done = (scn->worldstats()->actors[cmd->param1].flags2 & 1) == 0;

	return -1;
}

int kSetFlag2Bit0(Script *script, ScriptEntry *cmd, Scene *scn) {
	int targetType = cmd->param2;

	if (targetType == 2)
		scn->worldstats()->actors[cmd->param1].flags2 |= 1;
	else
		if (targetType == 1)
			scn->worldstats()->getActionAreaById(cmd->param1)->actionType |= 1;
		else
			scn->worldstats()->getBarrierById(cmd->param1)->flags2 |= 1;

	return 0;
}

int kClearFlag2Bit0(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int kJumpIfFlag2Bit2(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int kSetFlag2Bit2(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int kClearFlag2Bit2(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int kJumpIfFlag2Bit1(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int kSetFlag2Bit1(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int kClearFlag2Bit1(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int k_unk22(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int k_unk23(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int k_unk24(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int kRunEncounter(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int kJumpIfFlag2Bit4(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int kSetFlag2Bit4(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int kClearFlag2Bit4(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int kSetActorField638(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int kJumpIfActorField638(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}

int kChangeScene(Script *script, ScriptEntry *cmd, Scene *scn) {
	scn->actions()->delayedSceneIndex = cmd->param1 + 4;
	debug(kDebugLevelScripts,
	      "Queueing Scene Change to scene %d...",
	      scn->actions()->delayedSceneIndex);

	return 0;
}

int k_unk2C_ActorSub(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}

int kPlayMovie(Script *script, ScriptEntry *cmd, Scene *scn) {
	// TODO: add missing code here
	scn->actions()->delayedVideoIndex = cmd->param1;

	return -1;
}

int kStopAllBarriersSounds(Script *script, ScriptEntry *cmd, Scene *scn) {
	// TODO: do this for all barriers that have sfx playing
	scn->vm()->sound()->stopAllSounds();

	return -1;
}

int kSetActionFlag(Script *script, ScriptEntry *cmd, Scene *scn) {
	scn->actions()->setActionFlag(true);
	return 0;
}
int kClearActionFlag(Script *script, ScriptEntry *cmd, Scene *scn) {
	scn->actions()->setActionFlag(false);
	return 0;
}
int kResetSceneRect(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int kChangeMusicById(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}

int kStopMusic(Script *script, ScriptEntry *cmd, Scene *scn) {
	scn->vm()->sound()->stopMusic();

	return 0;
}

int k_unk34_Status(Script *script, ScriptEntry *cmd, Scene *scn) {
	if (cmd->param1 >= 2) {
		cmd->param1 = 0;
	} else {
		cmd->param1++;
		scn->actions()->lineIncrement = 1;
	}

	return 0;
}

int k_unk35(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int k_unk36(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}

int kRunBlowUpPuzzle(Script *script, ScriptEntry *cmd, Scene *scn) {
	// FIXME: improve this to call other blowUpPuzzles than VCR
	//int puzzleIdx = cmd->param1;
	warning("kRunBlowUpPuzzle not implemented");
	//scn->setBlowUpPuzzle(new BlowUpPuzzleVCR(scn));
	//scn->getBlowUpPuzzle()->openBlowUp();

	return -1;
}

int kJumpIfFlag2Bit3(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int kSetFlag2Bit3(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int kClearFlag2Bit3(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int k_unk3B_PALETTE_MOD(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}

int k_unk3C_CMP_VAL(Script *script, ScriptEntry *cmd, Scene *scn) {
	if (cmd->param1) {
		if (cmd->param2 >= cmd->param1) {
			cmd->param2 = 0;
		} else {
			cmd->param2++;
			scn->actions()->lineIncrement = 1;
		}
	}

	return 0;
}

int kWaitUntilFramePlayed(Script *script, ScriptEntry *cmd, Scene *scn) {
	Barrier *barrier = scn->worldstats()->getBarrierById(cmd->param1);

	if (barrier) {
		uint32 frameNum = cmd->param2;
		if (cmd->param2 == -1)
			frameNum = barrier->frameCount - 1;

		if (barrier->frameIdx != frameNum) {
			scn->actions()->lineIncrement = 1;
			scn->actions()->waitCycle     = true;
		}
	} else
		debugC(kDebugLevelScripts,
		       "Requested invalid object ID:0x%02X in Scene %d Line %d.",
		       cmd->param1,
		       scn->getSceneIndex(),
		       scn->actions()->currentLine);

	return 0;
}

int kUpdateWideScreen(Script *script, ScriptEntry *cmd, Scene *scn) {
	int barSize = cmd->param1;

	if (barSize >= 22) {
		cmd->param1 = 0;
	} else {
		scn->vm()->screen()->drawWideScreen(4 * barSize);
		cmd->param1++;
	}

	return 0;
}

int k_unk3F(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int k_unk40_SOUND(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}

int kPlaySpeech(Script *script, ScriptEntry *cmd, Scene *scn) {
	//TODO - Add support for other param options
	int32 sndIdx = cmd->param1;

	if ((int)sndIdx >= 0) {
        if (cmd->param4 != 2) {
            int32 resIdx = scn->speech()->play(sndIdx);
            cmd->param5 = resIdx;

            if (cmd->param2) {
                scn->vm()->setGameFlag(183);
                cmd->param4 = 2;
                if (cmd->param6) {
                    // TODO: set flag 01
                }
                scn->actions()->lineIncrement = 1;
            }

            if (cmd->param3) {
                if (!cmd->param6) {
                    scn->vm()->setGameFlag(219);
                }
            }
        }
        
        if (scn->vm()->sound()->isPlaying(cmd->param5)) {
            scn->actions()->lineIncrement = 1;
        }

        scn->vm()->clearGameFlag(183);
        cmd->param4 = 0;
        
        if (cmd->param3) {
            if (cmd->param6) {
                // TODO: clear flag 01
            }
            scn->vm()->clearGameFlag(219);
        }
        
        if (!cmd->param6) {
            cmd->param6 = 1;
        }

        // TODO: clear flag 01
	} else
		debugC(kDebugLevelScripts,
		       "Requested invalid sound ID:0x%02X in Scene %d Line %d.",
		       cmd->param1,
		       scn->getSceneIndex(),
		       scn->actions()->currentLine);

	return -1;
}

int k_unk42(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int k_unk43(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int kPaletteFade(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int kStartPaletteFadeThread(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int k_unk46(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}

int kActorFaceObject(Script *script, ScriptEntry *cmd, Scene *scn) {
	// XXX
	// Dropping param1, since it's the character index
	// Investigate further if/when we have a scene with
	// multiple characters in the actor[] array
	scn->getActor()->faceTarget(cmd->param2, cmd->param3);

	return -1;
}

int k_unk48_MATTE_01(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int k_unk49_MATTE_90(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int kJumpIfSoundPlaying(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int kChangePlayerCharacterIndex(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}

int kChangeActorField40(Script *script, ScriptEntry *cmd, Scene *scn) {
	// TODO: figure out what is this field and what values are set
	int actorIdx  = cmd->param1;
	int fieldType = cmd->param2;

	if (fieldType) {
		if (scn->worldstats()->actors[actorIdx].updateType < 11)
			scn->worldstats()->actors[actorIdx].updateType = 14;
	} else {
		scn->worldstats()->actors[actorIdx].updateType = 4;
	}

	return -1;
}

int kStopSound(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int k_unk4E_RANDOM_COMMAND(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}

int kClearScreen(Script *script, ScriptEntry *cmd, Scene *scn) {
	if (cmd->param1)
		scn->vm()->screen()->clearScreen();

	return 0;
}

int kQuit(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}

int kJumpBarrierFrame(Script *script, ScriptEntry *cmd, Scene *scn) {
	Barrier *barrier = scn->worldstats()->getBarrierById(cmd->param1);
	int idx = (int)barrier->frameIdx;

	if (cmd->param2 == -1)
		cmd->param2 = barrier->frameCount - 1;

	if (cmd->param3 && cmd->param2 == idx) {
		//break;
	} else if (cmd->param4 && cmd->param2 < idx) {
		//break;
	} else if (cmd->param5 && cmd->param2 > idx) {
		//break;
	} else if (cmd->param6 && cmd->param2 <= idx) {
		//break;
	} else if (cmd->param7 && cmd->param2 >= idx) {
		//break;
	} else if (cmd->param8 && cmd->param2 != idx) {
		//break;
	}

	ScriptEntry *nextCmd = &script->commands[cmd->param9];

	// 0x10 == kReturn
	if (nextCmd->opcode != 0x10 && nextCmd->opcode)
		scn->actions()->done = true;

	return 0;
}

int k_unk52(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int k_unk53(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}

int k_unk54_SET_ACTIONLIST_6EC(Script *script, ScriptEntry *cmd, Scene *scn) {
	if (cmd->param2)
		script->field_1BB0 = scn->vm()->_rnd.getRandomNumber(cmd->param1);
	else
		script->field_1BB0 = cmd->param1;

	return 0;
}

int k_unk55(Script *script, ScriptEntry *cmd, Scene *scn) {
	// TODO
	/*
	if (!cmd->param2) {
		if (cmd->param3 && script->field_1BB0 < cmd->param1)
			//break;
		else if (cmd->param4 && script->field_1BB0 > cmd->param1)
			//break;
		else if (cmd->param5 && script->field_1BB0 <= cmd->param1)
			//break;
		else if (cmd->param6 && script->field_1BB0 >= cmd->param1)
			//break;
		else if (cmd->param7 && script->field_1BB0 != cmd->param1)
			//break;
	} else if(script->field_1BB0 == cmd->param1) {
		//break;
	}
	*/

	ScriptEntry *nextCmd = &script->commands[cmd->param8];

	if (nextCmd->opcode != 0x10 && nextCmd->opcode)
		scn->actions()->done = true;
	else
		scn->actions()->lineIncrement = cmd->param8;

	return -1;
}

int k_unk56(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}

int kSetResourcePalette(Script *script, ScriptEntry *cmd, Scene *scn) {
	if (cmd->param1 > 0)
		scn->vm()->screen()->setPalette(scn->getResourcePack(), scn->worldstats()->grResId[cmd->param1]);

	return 0;
}

int kSetBarrierFrameIdxFlaged(Script *script, ScriptEntry *cmd, Scene *scn) {
	Barrier *barrier = scn->worldstats()->getBarrierById(cmd->param1);

	if (cmd->param3)
		barrier->flags = 1 | barrier->flags;
	else
		barrier->flags = barrier->flags & 0xFFFFFFFE;

	barrier->frameIdx = cmd->param2;

	return 0;
}

int k_unk59(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int k_unk5A(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int k_unk5B(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int k_unk5C(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int k_unk5D(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}

int kClearActorField970(Script *script, ScriptEntry *cmd, Scene *scn) {
	Actor *act = scn->getActor(cmd->param1);
	act->field_970 = 0;

	return 0;
}

int kSetBarrierLastFrameIdx(Script *script, ScriptEntry *cmd, Scene *scn) {
	Barrier *barrier = scn->worldstats()->getBarrierById(cmd->param1);

	if (barrier->frameIdx == barrier->frameCount - 1) {
		scn->actions()->lineIncrement = 0;
		barrier->flags &= 0xFFFEF1C7;
	} else {
		scn->actions()->lineIncrement = 1;
	}

	return 0;
}

int k_unk60_SET_OR_CLR_ACTIONAREA_FLAG(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}

int k_unk61(Script *script, ScriptEntry *cmd, Scene *scn) {
	if (cmd->param2) {
		if (scn->worldstats()->field_E860C == -1) {
			scn->actions()->lineIncrement = 0;
			cmd->param2   = 0;
		} else {
			scn->actions()->lineIncrement = 1;
		}
	} else {
		// TODO: do something for scene number 9
		cmd->param2 = 1;
		scn->actions()->lineIncrement = 1;
	}

	return -1;
}

int k_unk62_SHOW_OPTIONS_SCREEN(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}
int k_unk63(Script *script, ScriptEntry *cmd, Scene *scn) {
	return -2;
}

} // end of namespace Asylum
