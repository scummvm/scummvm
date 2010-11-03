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

#include "common/rational.h"

namespace Asylum {

ActionList::ActionList(Common::SeekableReadStream *stream, Scene *scene) : _scene(scene) {
	// Build list of opcodes
	ADD_OPCODE(Return);
	ADD_OPCODE(SetGameFlag);
	ADD_OPCODE(ClearGameFlag);
	ADD_OPCODE(ToggleGameFlag);
	ADD_OPCODE(JumpIfGameFlag);
	ADD_OPCODE(HideCursor);
	ADD_OPCODE(ShowCursor);
	ADD_OPCODE(PlayAnimation);
	ADD_OPCODE(MoveScenePosition);
	ADD_OPCODE(HideActor);
	ADD_OPCODE(ShowActor);
	ADD_OPCODE(SetActorPosition);
	ADD_OPCODE(SetSceneMotionStatus);
	ADD_OPCODE(DisableActor);
	ADD_OPCODE(EnableActor);
	ADD_OPCODE(EnableBarriers);
	ADD_OPCODE(Return);
	ADD_OPCODE(DestroyBarrier);
	ADD_OPCODE(_unk12_JMP_WALK_ACTOR);
	ADD_OPCODE(_unk13_JMP_WALK_ACTOR);
	ADD_OPCODE(_unk14_JMP_WALK_ACTOR);
	ADD_OPCODE(_unk15);
	ADD_OPCODE(ResetAnimation);
	ADD_OPCODE(ClearFlag1Bit0);
	ADD_OPCODE(_unk18_PLAY_SND);
	ADD_OPCODE(JumpIfFlag2Bit0);
	ADD_OPCODE(SetFlag2Bit0);
	ADD_OPCODE(ClearFlag2Bit0);
	ADD_OPCODE(JumpIfFlag2Bit2);
	ADD_OPCODE(SetFlag2Bit2);
	ADD_OPCODE(ClearFlag2Bit2);
	ADD_OPCODE(JumpIfFlag2Bit1);
	ADD_OPCODE(SetFlag2Bit1);
	ADD_OPCODE(ClearFlag2Bit1);
	ADD_OPCODE(_unk22);
	ADD_OPCODE(_unk23);
	ADD_OPCODE(_unk24);
	ADD_OPCODE(RunEncounter);
	ADD_OPCODE(JumpIfFlag2Bit4);
	ADD_OPCODE(SetFlag2Bit4);
	ADD_OPCODE(ClearFlag2Bit4);
	ADD_OPCODE(SetActorField638);
	ADD_OPCODE(JumpIfActorField638);
	ADD_OPCODE(ChangeScene);
	ADD_OPCODE(_unk2C_ActorSub);
	ADD_OPCODE(PlayMovie);
	ADD_OPCODE(StopAllBarriersSounds);
	ADD_OPCODE(StopProcessing);
	ADD_OPCODE(ResumeProcessing);
	ADD_OPCODE(ResetSceneRect);
	ADD_OPCODE(ChangeMusicById);
	ADD_OPCODE(StopMusic);
	ADD_OPCODE(_unk34_Status);
	ADD_OPCODE(SetVolume);
	ADD_OPCODE(Jump);
	ADD_OPCODE(RunBlowUpPuzzle);
	ADD_OPCODE(JumpIfFlag2Bit3);
	ADD_OPCODE(SetFlag2Bit3);
	ADD_OPCODE(ClearFlag2Bit3);
	ADD_OPCODE(_unk3B_PALETTE_MOD);
	ADD_OPCODE(IncrementParam2);
	ADD_OPCODE(WaitUntilFramePlayed);
	ADD_OPCODE(UpdateWideScreen);
	ADD_OPCODE(JumpIfActor);
	ADD_OPCODE(_unk40_SOUND);
	ADD_OPCODE(PlaySpeech);
	ADD_OPCODE(_unk42);
	ADD_OPCODE(MoveScenePositionFromActor);
	ADD_OPCODE(PaletteFade);
	ADD_OPCODE(StartPaletteFadeThread);
	ADD_OPCODE(_unk46);
	ADD_OPCODE(ActorFaceObject);
	ADD_OPCODE(_unk48_MATTE_01);
	ADD_OPCODE(_unk49_MATTE_90);
	ADD_OPCODE(JumpIfSoundPlaying);
	ADD_OPCODE(ChangePlayerActorIndex);
	ADD_OPCODE(ChangeActorStatus);
	ADD_OPCODE(StopSound);
	ADD_OPCODE(_unk4E_RANDOM_COMMAND);
	ADD_OPCODE(ClearScreen);
	ADD_OPCODE(Quit);
	ADD_OPCODE(JumpBarrierFrame);
	ADD_OPCODE(DeleteGraphics);
	ADD_OPCODE(DeleteGraphics);
	ADD_OPCODE(_unk54_SET_ACTIONLIST_6EC);
	ADD_OPCODE(_unk55);
	ADD_OPCODE(_unk56);
	ADD_OPCODE(SetResourcePalette);
	ADD_OPCODE(SetBarrierFrameIdxFlaged);
	ADD_OPCODE(_unk59);
	ADD_OPCODE(_unk5A);
	ADD_OPCODE(_unk5B);
	ADD_OPCODE(QueueScript);
	ADD_OPCODE(_unk5D);
	ADD_OPCODE(ClearActorField970);
	ADD_OPCODE(SetBarrierLastFrameIdx);
	ADD_OPCODE(_unk60_SET_OR_CLR_ACTIONAREA_FLAG);
	ADD_OPCODE(_unk61);
	ADD_OPCODE(_unk62_SHOW_OPTIONS_SCREEN);
	ADD_OPCODE(_unk63);

	// Reset script queue
	resetQueue();

	// Load data
	load(stream);

	_skipProcessing    = false;
	_currentLine       = 0;
	_currentLoops      = 0;
	_currentScript     = NULL;
	_delayedSceneIndex = -1;
	_delayedVideoIndex = -1;
	_done = false;
	_exit = false;
	_lineIncrement = 0;
	_waitCycle = false;
}

ActionList::~ActionList() {
	for (int i = 0; i < (int)_opcodes.size(); i++)
		delete _opcodes[i];

	_scripts.clear();
	_queue.clear();

	// Zero-out passed pointers
	_scene = NULL;
}

void ActionList::resetQueue() {
	_queue.clear();
}

void ActionList::queueScript(int scriptIndex, ActorIndex actorIndex) {
	// When the skipProcessing flag is set, do not queue any more scripts
	if (_skipProcessing)
		return;

	ScriptQueueEntry entry;
	entry.scriptIndex = scriptIndex;
	entry.actorIndex  = actorIndex;

	// If there's currently no script for the processor to run,
	// assign it directly and skip the stack push. If however the
	// current script is assigned, push the script to the stack
	if (_currentScript)
		_queue.push(entry);
	else {
		_currentQueueEntry = entry;
		_currentScript     = &_scripts[entry.scriptIndex];
	}
}

bool ActionList::process() {
	_done          = false;
	_exit          = false;
	_waitCycle     = false;
	_lineIncrement = 1;

	_scene->vm()->setGameFlag(kGameFlagScriptProcessing);

	if (_currentScript) {
		while (!_done && !_waitCycle) {
			_lineIncrement = 0; //Reset line increment value

			ScriptEntry *cmd = &_currentScript->commands[_currentLine];

			int32 opcode = cmd->opcode;

			debugC(kDebugLevelScripts, "[0x%02X] %s(%d, %d, %d, %d, %d, %d, %d, %d, %d)",
			       opcode, _opcodes[opcode]->name,
			       cmd->param1, cmd->param2, cmd->param3, cmd->param4, cmd->param5,
			       cmd->param6, cmd->param7, cmd->param8, cmd->param9);

			// Execute opcode
			(*_opcodes[opcode]->func)(cmd);

			if (_exit)
				return true;

			if (!_lineIncrement)
				_currentLine ++;
		}

		if (_done) {
			_currentLine  = 0;

			if (!_queue.empty()) {
				_currentQueueEntry = _queue.pop();
				_currentScript     = &_scripts[_currentQueueEntry.scriptIndex];
			} else {
				_currentScript = NULL;
			}
		}
	}

	_scene->vm()->clearGameFlag(kGameFlagScriptProcessing);

	return false;
}

void ActionList::load(Common::SeekableReadStream *stream) {
	stream->readSint32LE();  // size
	int32 numEntries = stream->readSint32LE();

	for (int32 a = 0; a < numEntries; a++) {
		Script script;
		memset(&script, 0, sizeof(Script));

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

			script.commands[c] = command;
		}

		script.field_1BAC = stream->readSint32LE();
		script.field_1BB0 = stream->readSint32LE();
		script.counter    = stream->readSint32LE();

		_scripts.push_back(script);
	}
}


//////////////////////////////////////////////////////////////////////////
// Opcode Functions
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Opcode 0x00
IMPLEMENT_OPCODE(Return) {
	_done          = true;
	_lineIncrement = 0;
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x01
IMPLEMENT_OPCODE(SetGameFlag) {
	GameFlag flagNum = (GameFlag)cmd->param1;

	if (flagNum >= 0)
		_scene->vm()->setGameFlag(flagNum);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x02
IMPLEMENT_OPCODE(ClearGameFlag) {
	GameFlag flagNum = (GameFlag)cmd->param1;

	if (flagNum >= 0)
		_scene->vm()->clearGameFlag(flagNum);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x03
IMPLEMENT_OPCODE(ToggleGameFlag) {
	GameFlag flagNum = (GameFlag)cmd->param1;

	if (flagNum >= 0)
		_scene->vm()->toggleGameFlag(flagNum);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x04
IMPLEMENT_OPCODE(JumpIfGameFlag) {
	if (cmd->param1 < 0)
		return;

	bool doJump = (cmd->param2) ? _scene->vm()->isGameFlagSet((GameFlag)cmd->param1) : _scene->vm()->isGameFlagNotSet((GameFlag)cmd->param1);
	if (!doJump)
		return;

	int32 nextOpcode = _currentScript->commands[_currentLine].opcode;
	if (nextOpcode != 0x10 && nextOpcode != 0) {
		error("Incomplete opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
		return;
	}

	_currentLine = cmd->param3;
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x05
IMPLEMENT_OPCODE(HideCursor) {
	_scene->getCursor()->hide();
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x06
IMPLEMENT_OPCODE(ShowCursor) {
	_scene->getCursor()->show();

	_scene->vm()->clearFlag(kFlagType1);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x07
IMPLEMENT_OPCODE(PlayAnimation) {
	Barrier *barrier = _scene->worldstats()->getBarrierById(cmd->param1);

	if (cmd->param2 == 2) {
		if (barrier->checkFlags())
			_lineIncrement = 1;
		else
			cmd->param2 = 1;

		return;
	}

	// Update flags
	if (cmd->param4) {
		barrier->flags &= kBarrierFlagFFFEF1C7;
        barrier->flags |= kBarrierFlag20;
	} else if (cmd->param3) {
		barrier->flags &= kBarrierFlagFFFEF1C7;
		barrier->flags |= kBarrierFlag10000;
	} else if (barrier->flags & kBarrierFlag10000) {
		barrier->flags |= kBarrierFlag8;
		barrier->flags &= kBarrierFlagFFFEFFFF;
	} else if (!(barrier->flags & kBarrierFlag10E38)) {
		barrier->flags |= kBarrierFlag8;
	}

	barrier->setNextFrame(barrier->flags);

	if (barrier->field_688 == 1) {
		if (barrier->flags & kBarrierFlag4) {
			_scene->setGlobalX(barrier->x);
			_scene->setGlobalY(barrier->y);
		} else {
			GraphicResource *res = new GraphicResource(_scene->getResourcePack(), barrier->resId);
			GraphicFrame *frame = res->getFrame(barrier->frameIdx);

			_scene->setGlobalX(frame->x + (frame->getWidth() >> 1) + barrier->x);
			_scene->setGlobalY(frame->y + (frame->getHeight() >> 1) + barrier->y);

			delete res;
		}
	}

	if (cmd->param2) {
		cmd->param2 = 2;
		_lineIncrement = 1;
	}
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x08
IMPLEMENT_OPCODE(MoveScenePosition) {
	if (cmd->param3 < 1) {
		_scene->worldstats()->xLeft = cmd->param1;
		_scene->worldstats()->yTop  = cmd->param2;
		_scene->worldstats()->motionStatus = 3;

	} else if (!cmd->param4) {
		_scene->worldstats()->motionStatus = 5;

		_scene->updateSceneCoordinates(cmd->param1,
			                           cmd->param2,
		                               cmd->param3);

	} else if (cmd->param5) {
		if (_scene->worldstats()->motionStatus == 2)
			_lineIncrement = 1;
		else
			cmd->param5 = 0;
	} else {
		cmd->param5 = 1;
		_scene->worldstats()->motionStatus = 2;

		_scene->updateSceneCoordinates(cmd->param1,
		                               cmd->param2,
		                               cmd->param3,
		                               true);

		_lineIncrement = 1;
	}
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x09
IMPLEMENT_OPCODE(HideActor) {
	Actor *actor = _scene->getActor(cmd->param1);

	actor->setVisible(false);
	actor->updateDirection();
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x0A
IMPLEMENT_OPCODE(ShowActor) {
	Actor *actor = _scene->getActor(cmd->param1);

	actor->setVisible(true);
	actor->updateDirection();
	actor->tickValue = _scene->vm()->getTick();
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x0B
IMPLEMENT_OPCODE(SetActorPosition) {
	Actor *actor = _scene->getActor(cmd->param1);

	actor->setPosition(cmd->param2, cmd->param3, cmd->param4, cmd->param5);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x0C
IMPLEMENT_OPCODE(SetSceneMotionStatus) {
	_scene->worldstats()->motionStatus = cmd->param1;
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x0D
IMPLEMENT_OPCODE(DisableActor) {
	Actor *actor = _scene->getActor(cmd->param1);

	actor->updateStatus(kActorStatusDisabled);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x0E
IMPLEMENT_OPCODE(EnableActor) {
	Actor *actor = _scene->getActor(cmd->param1);

	if (actor->status == kActorStatusDisabled)
		actor->updateStatus(kActorStatusEnabled);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x0F
IMPLEMENT_OPCODE(EnableBarriers) {
	Barrier *barrier = _scene->worldstats()->getBarrierById(cmd->param1);

	if (!_currentScript->counter && _scene->worldstats()->numChapter != 13)
		_scene->vm()->sound()->playSound(cmd->param3 ? kResourceSound_80120006 : kResourceSound_80120001, false, Config.sfxVolume, 0);

	if (_currentScript->counter >= (3 * cmd->param2 - 1)) {
		_currentScript->counter = 0;
		barrier->field_67C  = 0;
		enableBarrier(cmd, kBarrierEnableType2);
	} else {
		++_currentScript->counter;
		if (cmd->param3) {
			barrier->field_67C = 3 - _currentScript->counter / cmd->param2;
			enableBarrier(cmd, kBarrierEnableType1);
		} else {
			barrier->field_67C = _currentScript->counter / cmd->param2 + 1;
			enableBarrier(cmd, kBarrierEnableType0);
		}

		_lineIncrement = 1;
	}
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x10 : Identical to opcode 0x00

//////////////////////////////////////////////////////////////////////////
// Opcode 0x11
IMPLEMENT_OPCODE(DestroyBarrier) {
	if (!cmd->param1)
		return;

	Barrier *barrier = _scene->worldstats()->getBarrierById(cmd->param1);

	barrier->destroyAndRemoveFromQueue();
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x12
IMPLEMENT_OPCODE(_unk12_JMP_WALK_ACTOR) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x13
IMPLEMENT_OPCODE(_unk13_JMP_WALK_ACTOR) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x14
IMPLEMENT_OPCODE(_unk14_JMP_WALK_ACTOR) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x15
IMPLEMENT_OPCODE(_unk15) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x16
IMPLEMENT_OPCODE(ResetAnimation) {
	Barrier *barrier = _scene->worldstats()->getBarrierById(cmd->param1);

	if ((barrier->flags & 0x10000) == 0)
		barrier->frameIdx = 0;
	else
		barrier->frameIdx = barrier->frameCount - 1;
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x17
IMPLEMENT_OPCODE(ClearFlag1Bit0) {
	Barrier *barrier = _scene->worldstats()->getBarrierById(cmd->param1);

	barrier->flags &= 0xFFFFFFFE;
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x18
IMPLEMENT_OPCODE(_unk18_PLAY_SND) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x19
IMPLEMENT_OPCODE(JumpIfFlag2Bit0) {
	int targetType = cmd->param2;

	error("Incomplete opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
	// TODO targetType == 7 is trying to access an out of bounds actor
	// look at the disassembly again

	if (targetType <= 0)
		_done = (_scene->worldstats()->getBarrierById(cmd->param1)->flags2 & 1) == 0;
	else
		if (targetType == 1) // v4 == 1, so 1
			_done = (_scene->worldstats()->getActionAreaById(cmd->param1)->actionType & 1) == 0;
		else
			_done = (_scene->worldstats()->actors[cmd->param1]->flags2 & 1) == 0;
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x1A
IMPLEMENT_OPCODE(SetFlag2Bit0) {
	int targetType = cmd->param2;

	if (targetType == 2)
		_scene->worldstats()->actors[cmd->param1]->flags2 |= 1;
	else
		if (targetType == 1)
			_scene->worldstats()->getActionAreaById(cmd->param1)->actionType |= 1;
		else
			_scene->worldstats()->getBarrierById(cmd->param1)->flags2 |= 1;
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x1B
IMPLEMENT_OPCODE(ClearFlag2Bit0) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x1C
IMPLEMENT_OPCODE(JumpIfFlag2Bit2) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x1D
IMPLEMENT_OPCODE(SetFlag2Bit2) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x1E
IMPLEMENT_OPCODE(ClearFlag2Bit2) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x1F
IMPLEMENT_OPCODE(JumpIfFlag2Bit1) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x20
IMPLEMENT_OPCODE(SetFlag2Bit1) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x21
IMPLEMENT_OPCODE(ClearFlag2Bit1) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x22
IMPLEMENT_OPCODE(_unk22) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x23
IMPLEMENT_OPCODE(_unk23) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x24
IMPLEMENT_OPCODE(_unk24) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x25
IMPLEMENT_OPCODE(RunEncounter) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x26
IMPLEMENT_OPCODE(JumpIfFlag2Bit4) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x27
IMPLEMENT_OPCODE(SetFlag2Bit4) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x28
IMPLEMENT_OPCODE(ClearFlag2Bit4) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x29
IMPLEMENT_OPCODE(SetActorField638) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x2A
IMPLEMENT_OPCODE(JumpIfActorField638) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x2B
IMPLEMENT_OPCODE(ChangeScene) {
	debug(kDebugLevelScripts, "Queueing Scene Change to scene %d...", _delayedSceneIndex);

	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);

	_delayedSceneIndex = cmd->param1 + 4;
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x2C
IMPLEMENT_OPCODE(_unk2C_ActorSub) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x2D
IMPLEMENT_OPCODE(PlayMovie) {
	// TODO: add missing code here
	_delayedVideoIndex = cmd->param1;

	error("Incomplete opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x2E
IMPLEMENT_OPCODE(StopAllBarriersSounds) {
	// TODO: do this for all barriers that have sfx playing
	_scene->vm()->sound()->stopAllSounds();

	error("Incomplete opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x2F
IMPLEMENT_OPCODE(StopProcessing) {
	_skipProcessing = true;
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x30
IMPLEMENT_OPCODE(ResumeProcessing) {
	_skipProcessing = false;
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x31
IMPLEMENT_OPCODE(ResetSceneRect) {
	_scene->worldstats()->sceneRectIdx = LOBYTE(cmd->param1);
	_scene->vm()->screen()->palFade(0, 25, 10);
	_scene->vm()->setFlag(kFlagTypeSceneRectChanged);

	_scene->worldstats()->xLeft = _scene->worldstats()->sceneRects[_scene->worldstats()->sceneRectIdx].left;
	_scene->worldstats()->yTop  = _scene->worldstats()->sceneRects[_scene->worldstats()->sceneRectIdx].top;
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x32
IMPLEMENT_OPCODE(ChangeMusicById) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x33
IMPLEMENT_OPCODE(StopMusic) {
	_scene->vm()->sound()->stopMusic();
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x34
IMPLEMENT_OPCODE(_unk34_Status) {
	if (cmd->param1 >= 2) {
		cmd->param1 = 0;
	} else {
		cmd->param1++;
		_lineIncrement = 1;
	}
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x35
IMPLEMENT_OPCODE(SetVolume) {
	AmbientSoundItem item = _scene->worldstats()->ambientSounds[cmd->param1];
	int var = cmd->param2 + item.field_C;

	double volume = -((Config.sfxVolume + var) * (Config.ambientVolume + var));

	if (volume < 0) {
		if (volume < -10000)
			volume = -10000;
	} else {
		volume = 0;
	}

	_scene->vm()->sound()->setVolume(item.resId, volume);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x36
IMPLEMENT_OPCODE(Jump) {
	_currentLine = cmd->param1 - 1;
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x37
IMPLEMENT_OPCODE(RunBlowUpPuzzle) {
	_scene->vm()->screen()->clearScreen();
	error("Incomplete opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
	//_scene->vm()->screen()->clearDrawStruct(4096, 2049, &drawStruct);
	//_scene->vm()->switchMessageHandler(_scene->vm()->getMessageHandler(cmd->param1));
	_currentLine++;

	_exit = true;
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x38
IMPLEMENT_OPCODE(JumpIfFlag2Bit3) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x39
IMPLEMENT_OPCODE(SetFlag2Bit3) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x3A
IMPLEMENT_OPCODE(ClearFlag2Bit3) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x3B
IMPLEMENT_OPCODE(_unk3B_PALETTE_MOD) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x3C
IMPLEMENT_OPCODE(IncrementParam2) {
	if (cmd->param1) {
		if (cmd->param2 >= cmd->param1) {
			cmd->param2 = 0;
		} else {
			cmd->param2++;
			_lineIncrement = 1;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x3D
IMPLEMENT_OPCODE(WaitUntilFramePlayed) {
	Barrier *barrier = _scene->worldstats()->getBarrierById(cmd->param1);

	if (!barrier)
		error("ActionList::WaitUntilFramePlayed: Requested invalid object ID:0x%02X in Scene %d Line %d.", cmd->param1, _scene->getSceneIndex(),_currentLine);

	uint32 frameNum = cmd->param2;
	if (cmd->param2 == -1)
		frameNum = barrier->frameCount - 1;

	if (barrier->frameIdx != frameNum) {
		_lineIncrement = 1;
		_waitCycle     = true;
	}
}


//////////////////////////////////////////////////////////////////////////
// Opcode 0x3E
IMPLEMENT_OPCODE(UpdateWideScreen) {
	int barSize = cmd->param1;

	if (barSize >= 22) {
		cmd->param1 = 0;
	} else {
		_scene->vm()->screen()->drawWideScreen(4 * barSize);
		cmd->param1++;
	}
}


//////////////////////////////////////////////////////////////////////////
// Opcode 0x3F
IMPLEMENT_OPCODE(JumpIfActor) {
	ActorIndex index = (cmd->param1 == -1) ? _scene->getPlayerActorIndex() : cmd->param1;

	if (_currentQueueEntry.actorIndex != index)
		_currentLine = cmd->param2 - 1;
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x40
IMPLEMENT_OPCODE(_unk40_SOUND) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x41
IMPLEMENT_OPCODE(PlaySpeech) {
	if (cmd->param1 < 0)
		return;

	if (cmd->param4 != 2) {
		cmd->param5 = _scene->speech()->play(cmd->param1);

		if (cmd->param2) {
			_scene->vm()->setGameFlag(kGameFlagScriptProcessing);
			cmd->param4 = 2;
			if (cmd->param6) {
				_scene->vm()->setFlag(kFlagType1);
				_scene->vm()->setFlag(kFlagType2);
			}
			_lineIncrement = 1;
		}

		if (cmd->param3 && !cmd->param6)
			_scene->vm()->setGameFlag(kGameFlag219);
	}

	if (_scene->vm()->sound()->isPlaying((ResourceId)cmd->param5))
		_lineIncrement = 1;

	_scene->vm()->clearGameFlag(kGameFlagScriptProcessing);
	cmd->param4 = 0;

	if (cmd->param3) {
		if (cmd->param6) {
			_scene->vm()->clearFlag(kFlagType1);
			_scene->vm()->clearFlag(kFlagType2);
		} else {
			_scene->vm()->clearGameFlag(kGameFlag219);
		}
	}

	if (!cmd->param6) {
		cmd->param6 = 1;
	} else {
		_scene->vm()->clearFlag(kFlagType1);
		_scene->vm()->clearFlag(kFlagType2);
	}
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x42
IMPLEMENT_OPCODE(_unk42) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x43
IMPLEMENT_OPCODE(MoveScenePositionFromActor) {
	Actor *actor = _scene->getActor();

	if (!cmd->param3) {
		_scene->worldstats()->motionStatus = 5;

		_scene->updateSceneCoordinates(actor->x1 + Common::Rational(actor->x2, 2).toInt() - 320,
			                           actor->y1 + Common::Rational(actor->y2, 2).toInt() - 240,
		                               cmd->param2);
	} else if (cmd->param6) {
		if (_scene->worldstats()->motionStatus == 2) {
			_lineIncrement = 1;
		} else {
			cmd->param6 = 0;
			_scene->worldstats()->targetX = -1;
		}
	} else {
		cmd->param6 = 1;
		_scene->worldstats()->motionStatus = 2;

		if (_scene->updateSceneCoordinates(actor->x1 + Common::Rational(actor->x2, 2).toInt() - 320,
										   actor->y1 + Common::Rational(actor->y2, 2).toInt() - 240,
										   cmd->param2,
										   true,
										   &cmd->param6))
			_lineIncrement = 0;
		else
			_lineIncrement = 1;
	}
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x44
IMPLEMENT_OPCODE(PaletteFade) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x45
IMPLEMENT_OPCODE(StartPaletteFadeThread) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x46
IMPLEMENT_OPCODE(_unk46) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x47
IMPLEMENT_OPCODE(ActorFaceObject) {
	// XXX
	// Dropping param1, since it's the character index
	// Investigate further if/when we have a scene with
	// multiple characters in the actor[] array
	_scene->getActor()->faceTarget(cmd->param2, cmd->param3);

	error("Incomplete opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x48
IMPLEMENT_OPCODE(_unk48_MATTE_01) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x49
IMPLEMENT_OPCODE(_unk49_MATTE_90) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x4A
IMPLEMENT_OPCODE(JumpIfSoundPlaying) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x4B
IMPLEMENT_OPCODE(ChangePlayerActorIndex) {
	_scene->changePlayerActorIndex(cmd->param1);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x4C
IMPLEMENT_OPCODE(ChangeActorStatus) {
	Actor *actor = _scene->getActor(cmd->param1);

	if (cmd->param2) {
		if (actor->status < kActorStatus11)
			actor->status = kActorStatus14;
	} else {
		actor->status = kActorStatusEnabled;
	}
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x4D
IMPLEMENT_OPCODE(StopSound) {
	if (_scene->vm()->sound()->isPlaying((ResourceId)cmd->param1))
		_scene->vm()->sound()->stopSound((ResourceId)cmd->param1);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x4E
IMPLEMENT_OPCODE(_unk4E_RANDOM_COMMAND) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x4F
IMPLEMENT_OPCODE(ClearScreen) {
	if (cmd->param1) {
		_scene->vm()->screen()->clearScreen();
		_scene->vm()->setFlag(kFlagTypeSkipDraw);
	} else {
		_scene->vm()->clearFlag(kFlagTypeSkipDraw);
	}
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x50
IMPLEMENT_OPCODE(Quit) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x51
IMPLEMENT_OPCODE(JumpBarrierFrame) {
	Barrier *barrier = _scene->worldstats()->getBarrierById(cmd->param1);
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

	ScriptEntry *nextCmd = &_currentScript->commands[cmd->param9];

	// 0x10 == kReturn
	if (nextCmd->opcode != 0x10 && nextCmd->opcode)
		_done = true;
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x52
IMPLEMENT_OPCODE(DeleteGraphics) {
	for (uint i = 0; i < 55; i++)
		_scene->vm()->screen()->deleteGraphicFromQueue(_scene->getActor(cmd->param1)->grResTable[cmd->param1]);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x53
IMPLEMENT_OPCODE(SetActorField944) {
	Actor *actor = _scene->getActor(cmd->param1);

	actor->field_944 = cmd->param2;
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x54
IMPLEMENT_OPCODE(_unk54_SET_ACTIONLIST_6EC) {
	if (cmd->param2)
		_currentScript->field_1BB0 = _scene->vm()->getRandom(cmd->param1);
	else
		_currentScript->field_1BB0 = cmd->param1;
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x55
IMPLEMENT_OPCODE(_unk55) {
	// TODO
	/*
	if (!cmd->param2) {
		if (cmd->param3 && _currentScript->field_1BB0 < cmd->param1)
			//break;
		else if (cmd->param4 && _currentScript->field_1BB0 > cmd->param1)
			//break;
		else if (cmd->param5 && _currentScript->field_1BB0 <= cmd->param1)
			//break;
		else if (cmd->param6 && _currentScript->field_1BB0 >= cmd->param1)
			//break;
		else if (cmd->param7 && _currentScript->field_1BB0 != cmd->param1)
			//break;
	} else if(_currentScript->field_1BB0 == cmd->param1) {
		//break;
	}
	*/

	ScriptEntry *nextCmd = &_currentScript->commands[cmd->param8];

	if (nextCmd->opcode != 0x10 && nextCmd->opcode)
		_done = true;
	else
		_lineIncrement = cmd->param8;

	error("Incomplete opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x56
IMPLEMENT_OPCODE(_unk56) {
	Actor *actor = _scene->getActor(cmd->param2 == 2 ? -1 : cmd->param1);

	if (actor->status == kActorStatus2 || actor->status == kActorStatus13) {
		if (cmd->param2 == 2)
			_lineIncrement = 1;

		return;
	}

	if (cmd->param2 == 2) {
		cmd->param2 = 1;
		_lineIncrement = 0;

		if ((actor->x1 + actor->x2 == cmd->param6) && (actor->y1 + actor->y2 == cmd->param7)) {
			actor->setDirectionFrom(cmd->param1, kDirectionFromActor);
			_scene->getActor(cmd->param1)->setDirection((actor->direction + 4) & 7);
		} else {
			_currentLine = cmd->param3;
		}
	} else {
		error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
	}
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x57
IMPLEMENT_OPCODE(SetResourcePalette) {
	if (cmd->param1 < 0)
		error("Invalid resource id in opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);

	_scene->worldstats()->currentPaletteId = _scene->worldstats()->grResId[cmd->param1];

	_scene->vm()->screen()->setPalette(_scene->getResourcePack(), _scene->worldstats()->currentPaletteId);
	_scene->vm()->screen()->setGammaLevel(_scene->getResourcePack(), _scene->worldstats()->currentPaletteId, 0);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x58
IMPLEMENT_OPCODE(SetBarrierFrameIdxFlaged) {
	Barrier *barrier = _scene->worldstats()->getBarrierById(cmd->param1);

	if (cmd->param3)
		barrier->flags = 1 | barrier->flags;
	else
		barrier->flags &= kBarrierFlagDestroyed;

	barrier->frameIdx = cmd->param2;
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x59
IMPLEMENT_OPCODE(_unk59) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x5A
IMPLEMENT_OPCODE(_unk5A) {
	_scene->getActor(cmd->param1)->actionIdx2 = cmd->param2;
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x5B
IMPLEMENT_OPCODE(_unk5B) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x5C
IMPLEMENT_OPCODE(QueueScript) {
	queueScript( _scene->worldstats()->getActionAreaById(cmd->param1)->scriptIndex, cmd->param2);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x5D
IMPLEMENT_OPCODE(_unk5D) {

	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x5E
IMPLEMENT_OPCODE(ClearActorField970) {
	Actor *act = _scene->getActor(cmd->param1);

	act->field_970 = 0;
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x5F
IMPLEMENT_OPCODE(SetBarrierLastFrameIdx) {
	Barrier *barrier = _scene->worldstats()->getBarrierById(cmd->param1);

	if (barrier->frameIdx == barrier->frameCount - 1) {
		_lineIncrement = 0;
		barrier->flags &= kBarrierFlagFFFEF1C7;
	} else {
		_lineIncrement = 1;
	}
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x60
IMPLEMENT_OPCODE(_unk60_SET_OR_CLR_ACTIONAREA_FLAG) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x61
IMPLEMENT_OPCODE(_unk61) {
	if (cmd->param2) {
		if (_scene->worldstats()->field_E860C == -1) {
			_lineIncrement = 0;
			cmd->param2   = 0;
		} else {
			_lineIncrement = 1;
		}
	} else {
		// TODO: do something for scene number 9
		cmd->param2 = 1;
		_lineIncrement = 1;
	}

	error("Incomplete opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x62
IMPLEMENT_OPCODE(_unk62_SHOW_OPTIONS_SCREEN) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode 0x63
IMPLEMENT_OPCODE(_unk63) {
	error("Unhandled opcode %s (0x%02X) in Scene %d Line %d", _opcodes[cmd->opcode]->name, cmd->opcode, _scene->getSceneIndex(), _currentLine);
}

//////////////////////////////////////////////////////////////////////////
// Opcode Helper functions
//////////////////////////////////////////////////////////////////////////

void ActionList::enableBarrier(ScriptEntry *cmd, BarrierEnableType type) {
	error("[ActionList::enableBarrier] not implemented!");
}

} // end of namespace Asylum
