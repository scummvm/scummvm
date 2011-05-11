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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "m4/woodscript.h"

namespace M4 {

bool (Machine::*machineCommandsTable[])(Instruction &instruction) = {
	NULL,
	NULL,//TODO: nop
	&Machine::m1_gotoState,
	&Machine::m1_jump,
	&Machine::m1_terminate,
	&Machine::m1_startSequence,
	&Machine::m1_pauseSequence,
	&Machine::m1_resumeSequence,
	&Machine::m1_storeValue,
	&Machine::m1_sendMessage,
	&Machine::m1_broadcastMessage,
	&Machine::m1_replyMessage,
	&Machine::m1_sendSystemMessage,
	&Machine::m1_createMachine,
	&Machine::m1_createMachineEx,
	&Machine::m1_clearVars
};

void (Machine::*machineConditionalsTable[])(Instruction &instruction) = {
	NULL,//TODO: after
	&Machine::m1_onEndSequence,
	&Machine::m1_onMessage,
	NULL,//TODO: on_p_msg
	&Machine::m1_switchLt,
	&Machine::m1_switchLe,
	&Machine::m1_switchEq,
	&Machine::m1_switchNe,
	&Machine::m1_switchGe,
	&Machine::m1_switchGt,
};

Machine::Machine(WoodScript *ws, int32 machineHash, Sequence *parentSeq, int32 dataHash,
	int32 dataRowIndex, int callbackHandler, Common::String machineName, int32 id) {

	_ws = ws;

	_machHash = machineHash;
	_name = machineName;
	_id = id;

	// initialize the machine's bytecode
	MachineAsset *machineAsset = _ws->assets()->getMachine(_machHash);
	byte *code;
	uint32 codeSize;
	machineAsset->getCode(code, codeSize);
	_code = new Bytecode(_ws, code, codeSize, NULL);

	// initialize the machine's data
	if (dataHash >= 0) {
		DataAsset *dataAsset = _ws->assets()->getData(dataHash);
		_dataRow = dataAsset->getRow(dataRowIndex);
	} else {
		_dataRow = NULL;
	}

	_recursionLevel = 0;
	_currentState = 0;
	_sequence = NULL;
	_parentSequence = parentSeq;
	_targetCount = 0;

}

Machine::~Machine() {
	delete _code;
}

void Machine::clearMessages() {
}

void Machine::clearPersistentMessages() {
}

void Machine::restorePersistentMessages() {
}

void Machine::sendMessage(uint32 messageHash, long messageValue, Machine *sender) {
}

void Machine::resetSwitchTime() {
}

bool Machine::changeSequenceProgram(int32 sequenceHash) {
	return _sequence->changeProgram(sequenceHash);
}

bool Machine::searchMessages(uint32 messageHash, uint32 messageValue, Machine *sender) {
	return false;
}

bool Machine::searchPersistentMessages(uint32 messageHash, uint32 messageValue, Machine *sender) {
	return false;
}

void Machine::enterState() {

	MachineAsset *machineAsset = _ws->assets()->getMachine(_machHash);

	_code->jumpAbsolute(machineAsset->getStateOffset(_currentState));

	int32 instruction = -1;

	_recursionLevel++;

	int32 oldId = _id;
	int32 oldRecursionLevel = _recursionLevel;

	while (instruction && instruction != 4 && _id == oldId && _recursionLevel == oldRecursionLevel) {
		instruction = execInstruction();
	}

	if (instruction != 4 && _id == oldId && _recursionLevel == oldRecursionLevel) {
		_recursionLevel--;
	}

}

int32 Machine::execInstruction() {

	//debugCN(kDebugScript, "Machine::execInstruction()\n");

	bool done = false;
	Instruction instruction;
	//Sequence *sequence;
	int32 machID = _id;

	_code->loadInstruction(instruction);

	if (instruction.instr >= 64) {
		if (machineConditionalsTable[instruction.instr - 64] != 0)
			(this->*machineConditionalsTable[instruction.instr - 64])(instruction);
		/* The next line is to yield on unimplemented opcodes */
		else {
			g_system->delayMillis(5000);
		}
	} else if (instruction.instr > 0) {
		if (machineCommandsTable[instruction.instr] != 0)
			done = !(this->*machineCommandsTable[instruction.instr])(instruction);
		/* The next line is to yield on unimplemented opcodes */
		else {
			g_system->delayMillis(5000);
		}
		if (done) {
			if (_id == machID) {
				//TODO: Cancel all requests
				if (_currentState == -1) {
				    // TODO: Set terminated flag and delete machine in WoodScript update
				}
				else {
					// initialize new state
					enterState();
				}
			}
		}
	}

	return instruction.instr;

}

void Machine::execBlock(int32 offset, int32 count) {

	// MachineAsset *machineAsset = _ws->assets()->getMachine(_machHash);

	int32 startOffset = offset, endOffset = offset + count;

	_recursionLevel++;

	int32 oldId = _id;
	int32 oldRecursionLevel = _recursionLevel;

	_code->jumpAbsolute(offset);

	int32 instruction = -1;

	//debugCN(kDebugScript, "---------------------------------------\n");

	while (instruction && instruction != 4 && _id == oldId && _recursionLevel == oldRecursionLevel &&
		_code->pos() >= (uint32)startOffset && _code->pos() < (uint32)endOffset) {

		instruction = execInstruction();
		//g_system->delayMillis(500);
	}

	//debugCN(kDebugScript, "---------------------------------------\n");

	if (instruction == 3) {
		execInstruction();
	}

	if (instruction != 4 && _id == oldId && _recursionLevel == oldRecursionLevel) {
		_recursionLevel--;
	}

}

bool Machine::m1_gotoState(Instruction &instruction) {
	//debugCN(kDebugScript, "Machine::m1_gotoState() state = %d\n", (int32)instruction.argv[0] >> 16);

	_currentState = (int32)instruction.argv[0] >> 16;
	_recursionLevel = 0;
	return false;
}

bool Machine::m1_jump(Instruction &instruction) {
	//debugCN(kDebugScript, "Machine::m1_jump() ofs = %08X\n", (int32)instruction.argv[0] >> 16);

	_code->jumpRelative((int32)instruction.argv[0] >> 16);
	return true;
}

bool Machine::m1_terminate(Instruction &instruction) {
	//debugCN(kDebugScript, "Machine::m1_terminate()\n");

	_currentState = -1;
	_recursionLevel = 0;
	return false;
}

bool Machine::m1_startSequence(Instruction &instruction) {
	//debugCN(kDebugScript, "Machine::m1_startSequence() sequence hash = %d\n", (uint32)instruction.argv[0] >> 16);

	int32 sequenceHash = instruction.argv[0] >> 16;
	if (_sequence == NULL) {
		//debugCN(kDebugScript, "Machine::m1_startSequence() creating new sequence\n");
		_sequence = _ws->createSequence(this, sequenceHash);
		_code->setSequence(_sequence);
	} else {
		//debugCN(kDebugScript, "Machine::m1_startSequence() using existing sequence\n");
		_sequence->changeProgram(sequenceHash);
		//_code->setSequence(_sequence);
	}
	return true;
}

bool Machine::m1_pauseSequence(Instruction &instruction) {
	//debugCN(kDebugScript, "Machine::m1_pauseSequence()\n");

	_sequence->pause();
	return true;
}

bool Machine::m1_resumeSequence(Instruction &instruction) {
	//debugCN(kDebugScript, "Machine::m1_resumeSequence()\n");

	_sequence->resume();
	return true;
}

bool Machine::m1_storeValue(Instruction &instruction) {
	//debugCN(kDebugScript, "Machine::m1_storeValue() %p = %d (%08X)\n", (void*)instruction.argp[0], (uint32)instruction.argv[1], (uint32)instruction.argv[1]);

	*instruction.argp[0] = instruction.getValue();
	return true;
}

bool Machine::m1_sendMessage(Instruction &instruction) {
	//debugCN(kDebugScript, "Machine::m1_sendMessage() %p = %d (%08X)\n", (void*)instruction.argp[0], (uint32)instruction.argv[1], (uint32)instruction.argv[1]);

#if 0
//TODO
	long messageValue;

	if (instruction.argc == 3) {
		messageValue = instruction.argv[2];
	} else {
		messageValue = 0;
	}
	//_ws->sendMessage((uint32)instruction.argv[1], messageValue, (uint32)instruction.argv[0] >> 16);
	//void SendWSMessage(uint32 msgHash, long msgValue, machine *recvM, uint32 machHash, machine *sendM, int32 msgCount) {
#endif
	return true;

}

bool Machine::m1_broadcastMessage(Instruction &instruction) {
	//debugCN(kDebugScript, "Machine::m1_broadcastMessage() %p = %d (%08X)\n", (void*)instruction.argp[0], (uint32)instruction.argv[1], (uint32)instruction.argv[1]);

#if 0
//TODO
	long messageValue;

	if (instruction.argc == 3) {
		messageValue = instruction.argv[2];
	} else {
		messageValue = 0;
	}
	//_ws->sendMessage((uint32)instruction.argv[1], messageValue, (uint32)instruction.argv[0] >> 16);
#endif
	return true;
}

bool Machine::m1_replyMessage(Instruction &instruction) {
	//debugCN(kDebugScript, "Machine::m1_replyMessage() messageHash = %d; messageValue = %d\n", (uint32)instruction.argv[0], (uint32)instruction.argv[1]);
#if 0
	if (myArg2) {
		msgValue = *myArg2;
	}
	else {
		msgValue = 0;
	}
	SendWSMessage(*myArg1, msgValue, m->msgReplyXM, 0, m, 1);
#endif
	return true;
}

bool Machine::m1_sendSystemMessage(Instruction &instruction) {
	//debugCN(kDebugScript, "Machine::m1_sendSystemMessage() messageValue = %d\n", (uint32)instruction.argv[0]);
#if 0
#endif
	return true;
}

bool Machine::m1_createMachine(Instruction &instruction) {
	//debugCN(kDebugScript, "Machine::m1_createMachine()\n");
#if 0
#endif
	return true;
}

bool Machine::m1_createMachineEx(Instruction &instruction) {
	//debugCN(kDebugScript, "Machine::m1_createMachineEx()\n");
#if 0
#endif
	return true;
}

bool Machine::m1_clearVars(Instruction &instruction) {
	//debugCN(kDebugScript, "Machine::m1_clearVars()\n");

	_sequence->clearVars();
	return true;
}


void Machine::m1_onEndSequence(Instruction &instruction) {
	//debugCN(kDebugScript, "Machine::m1_onEndSequence() count = %08X\n", (uint32)instruction.argv[0] >> 16);

	int32 count = instruction.argv[0] >> 16;
	_sequence->issueEndOfSequenceRequest(_code->pos(), count);
	_code->jumpRelative(count);
}

void Machine::m1_onMessage(Instruction &instruction) {
	//debugCN(kDebugScript, "Machine::m1_onEndSequence() count = %08X\n", (uint32)instruction.argv[0] >> 16);

	// TODO: Add message to list

	int32 count = instruction.argv[0] >> 16;
	_code->jumpRelative(count);

}

void Machine::m1_switchLt(Instruction &instruction) {
	//debugCN(kDebugScript, "Machine::m1_switchLt() %d < %d -> %08X\n", (uint32)instruction.argv[1], (uint32)instruction.argv[2], (uint32)instruction.argv[0] >> 16);

	if (instruction.argv[1] >= instruction.argv[2])
		_code->jumpRelative(instruction.argv[0] >> 16);
}

void Machine::m1_switchLe(Instruction &instruction) {
	//debugCN(kDebugScript, "Machine::m1_switchLe() %d <= %d -> %08X\n", (uint32)instruction.argv[1], (uint32)instruction.argv[2], (uint32)instruction.argv[0] >> 16);

	if (instruction.argv[1] > instruction.argv[2])
		_code->jumpRelative(instruction.argv[0] >> 16);
}

void Machine::m1_switchEq(Instruction &instruction) {
	//debugCN(kDebugScript, "Machine::m1_switchEq() %d == %d -> %08X\n", (uint32)instruction.argv[1], (uint32)instruction.argv[2], (uint32)instruction.argv[0] >> 16);

	if (instruction.argv[1] != instruction.argv[2])
		_code->jumpRelative(instruction.argv[0] >> 16);
}

void Machine::m1_switchNe(Instruction &instruction) {
	//debugCN(kDebugScript, "Machine::m1_switchNe() %d != %d -> %08X\n", (uint32)instruction.argv[1], (uint32)instruction.argv[2], (uint32)instruction.argv[0] >> 16);

	if (instruction.argv[1] == instruction.argv[2])
		_code->jumpRelative(instruction.argv[0] >> 16);
}

void Machine::m1_switchGe(Instruction &instruction) {
	//debugCN(kDebugScript, "Machine::m1_switchGe() %d >= %d -> %08X\n", (uint32)instruction.argv[1], (uint32)instruction.argv[2], (uint32)instruction.argv[0] >> 16);

	if (instruction.argv[1] < instruction.argv[2])
		_code->jumpRelative(instruction.argv[0] >> 16);
}

void Machine::m1_switchGt(Instruction &instruction) {
	//debugCN(kDebugScript, "Machine::m1_switchGt() %d > %d -> %08X\n", (uint32)instruction.argv[1], (uint32)instruction.argv[2], (uint32)instruction.argv[0] >> 16);

	if (instruction.argv[1] <= instruction.argv[2])
		_code->jumpRelative(instruction.argv[0] >> 16);
}

}
