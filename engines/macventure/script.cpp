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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * Based on
 * WebVenture (c) 2010, Sean Kasun
 * https://github.com/mrkite/webventure, http://seancode.com/webventure/
 *
 * Used with explicit permission from the author
 */

#include "common/system.h"

#include "macventure/macventure.h"
#include "macventure/script.h"
#include "macventure/world.h"
#include "macventure/container.h"

namespace MacVenture {

ScriptEngine::ScriptEngine(MacVentureEngine *engine, World *world) {
	_engine = engine;
	_world = world;
	_scripts = new Container(_engine->getFilePath(kFilterPathID));
}

ScriptEngine::~ScriptEngine() {
	if (_scripts)
		delete _scripts;
}

bool ScriptEngine::runControl(ControlAction action, ObjID source, ObjID destination, Common::Point delta) {
	EngineFrame frame;
	frame.action = action;
	frame.src = source;
	frame.dest = destination;
	frame.x = delta.x;
	frame.y = delta.y;
	frame.haltedInSaves = false;
	frame.haltedInFirst = false;
	frame.haltedInFamily = false;
	_frames.push_back(frame);
	debugC(3, kMVDebugScript, "Stored frame %d, action: %d src: %d dest: %d point: (%d, %d)",
		_frames.size() - 1, frame.action, frame.src, frame.dest, frame.x, frame.y);

	return resume(true);
}

bool ScriptEngine::resume(bool execAll) {
	debugC(3, kMVDebugScript, "Resume Script");
	while (_frames.size()) {
		bool fail = execFrame(execAll);
		if (fail) {
			return true;
		}
	}
	return false;
}

void ScriptEngine::reset() {
	_frames.clear();
}

bool ScriptEngine::execFrame(bool execAll) {
	bool doFirst = execAll;
	bool doFamily = false;
	bool fail;

	EngineFrame *frame = &_frames.front();

	// Do first dispatch script (script 0)
	if (frame->haltedInFirst || doFirst) { // We were stuck or it's the first time
		frame->haltedInFirst = false;
		if (doFirst) {
			fail = loadScript(frame, 0);
		} else {
			fail = resumeFunc(frame);
		}
		if (fail) {
			frame->haltedInFirst = true;
			_engine->preparedToRun();
			return true;
		}
		doFamily = true;
		frame->familyIdx = 0;
	}

	// Do scripts in the family of player (ObjID 1)
	if (frame->haltedInFamily || doFamily) { // We have to do the family or we were stuck here
		frame->haltedInFamily = false;
		Common::Array<ObjID> family = _world->getFamily(_world->getObjAttr(1, kAttrParentObject), false);
		for (uint32 i = frame->familyIdx; i < family.size(); i++) {
			if (doFamily) {
				fail = loadScript(frame, family[i]);
			} else {
				fail = resumeFunc(frame);
			}
			if (fail) { // We are stuck, so we don't shift the frame
				frame->haltedInFamily = true;
				frame->familyIdx = i;
				_engine->preparedToRun();
				return true;
			}
			doFamily = true;
		}
	}

	// Halted in saves
	if (frame->haltedInSaves) {
		frame->haltedInSaves = false;
		if (resumeFunc(frame)) {
			frame->haltedInSaves = true;
			_engine->preparedToRun();
			return true;
		}
	}

	int highest = 0;
	uint localHigh = 0;
	do { // Saved function calls
		highest = 0;
		for (uint i = 0; i < frame->saves.size(); i++) {
			if (highest < frame->saves[i].rank) {
				highest = frame->saves[i].rank;
				localHigh = i;
			}
		}
		if (highest) {
			frame->saves[localHigh].rank = 0;
			if (loadScript(frame, frame->saves[localHigh].func)) {
				frame->haltedInSaves = true;
				_engine->preparedToRun();
				return true;
			}
		}
	} while (highest);

	_frames.pop_front();
	return false;
}

bool ScriptEngine::loadScript(EngineFrame *frame, uint32 scriptID) {
	if (_scripts->getItemByteSize(scriptID) > 0) {
		debugC(2, kMVDebugScript, "Loading function %d", scriptID);
		// Insert the new script at the front
		frame->scripts.push_front(ScriptAsset(scriptID, _scripts));
		return runFunc(frame);
	}
	return false;
}

bool ScriptEngine::resumeFunc(EngineFrame *frame) {
	bool fail = runFunc(frame);
	if (fail) {
		return fail;
	}
	frame->scripts.pop_front();
	if (frame->scripts.size())
		return resumeFunc(frame);
	return false;
}

bool ScriptEngine::runFunc(EngineFrame *frame) {
	ScriptAsset &script = frame->scripts.front();
	EngineState *state = &frame->state;
	byte op;
	while (script.hasNext()) {
		op = script.fetch();
		debugC(4, kMVDebugScript, "Running operation %d", op);
		if (!(op & 0x80)) {
			state->push(op);
		} else {
			switch (op) {
			case 0x80: //get attribute
				op80GATT(state, frame);
				break;
			case 0x81: //set attribute
				op81SATT(state, frame);
				break;
			case 0x82: //sum children attribute
				op82SUCH(state, frame);
				break;
			case 0x83: //push selected control
				op83PUCT(state, frame);
				break;
			case 0x84: //push selected object
				op84PUOB(state, frame);
				break;
			case 0x85: //push target
				op85PUTA(state, frame);
				break;
			case 0x86: //push deltax
				op86PUDX(state, frame);
				break;
			case 0x87: //push deltay
				op87PUDY(state, frame);
				break;
			case 0x88: //push immediate.b
				op88PUIB(state, frame, &script);
				break;
			case 0x89: //push immediate
				op89PUI(state, frame, &script);
				break;
			case 0x8a: //get global
				op8aGGLO(state, frame);
				break;
			case 0x8b: //set global
				op8bSGLO(state, frame);
				break;
			case 0x8c: //random
				op8cRAND(state, frame);
				break;
			case 0x8d: //copy
				op8dCOPY(state, frame);
				break;
			case 0x8e: //copyn
				op8eCOPYN(state, frame);
				break;
			case 0x8f: //swap
				op8fSWAP(state, frame);
				break;
			case 0x90: //swapn
				op90SWAPN(state, frame);
				break;
			case 0x91: //pop
				op91POP(state, frame);
				break;
			case 0x92: //copy+1
				op92COPYP(state, frame);
				break;
			case 0x93: //copy+n
				op93COPYPN(state, frame);
				break;
			case 0x94: //shuffle
				op94SHUFF(state, frame);
				break;
			case 0x95: //sort
				op95SORT(state, frame);
				break;
			case 0x96: //clear stack
				op96CLEAR(state, frame);
				break;
			case 0x97: //get stack size
				op97SIZE(state, frame);
				break;
			case 0x98: //add
				op98ADD(state, frame);
				break;
			case 0x99: //subtract
				op99SUB(state, frame);
				break;
			case 0x9a: //multiply
				op9aMUL(state, frame);
				break;
			case 0x9b: //divide
				op9bDIV(state, frame);
				break;
			case 0x9c: //mod
				op9cMOD(state, frame);
				break;
			case 0x9d: //divmod
				op9dDMOD(state, frame);
				break;
			case 0x9e: //abs
				op9eABS(state, frame);
				break;
			case 0x9f: //neg
				op9fNEG(state, frame);
				break;
			case 0xa0: //and
				opa0AND(state, frame);
				break;
			case 0xa1: //or
				opa1OR(state, frame);
				break;
			case 0xa2: //xor
				opa2XOR(state, frame);
				break;
			case 0xa3: //not
				opa3NOT(state, frame);
				break;
			case 0xa4: //logical and
				opa4LAND(state, frame);
				break;
			case 0xa5: //logical or
				opa5LOR(state, frame);
				break;
			case 0xa6: //logical xor
				opa6LXOR(state, frame);
				break;
			case 0xa7: //logical not
				opa7LNOT(state, frame);
				break;
			case 0xa8: //gt? unsigned
				opa8GTU(state, frame);
				break;
			case 0xa9: //lt? unsigned
				opa9LTU(state, frame);
				break;
			case 0xaa: //gt? signed
				opaaGTS(state, frame);
				break;
			case 0xab: //lt? signed
				opabLTS(state, frame);
				break;
			case 0xac: //eq?
				opacEQ(state, frame);
				break;
			case 0xad: //eq string?
				opadEQS(state, frame);
				break;
			case 0xae: //contains
				opaeCONT(state, frame);
				break;
			case 0xaf: //contains word
				opafCONTW(state, frame);
				break;
			case 0xb0: //bra
				opb0BRA(state, frame, &script);
				break;
			case 0xb1: //bra.b
				opb1BRAB(state, frame, &script);
				break;
			case 0xb2: //beq
				opb2BEQ(state, frame, &script);
				break;
			case 0xb3: //beq.b
				opb3BEQB(state, frame, &script);
				break;
			case 0xb4: //bne
				opb4BNE(state, frame, &script);
				break;
			case 0xb5: //bne.b
				opb5BNEB(state, frame, &script);
				break;
			case 0xb6: //call later
				opb6CLAT(state, frame);
				break;
			case 0xb7: //cancel call
				opb7CCA(state, frame);
				break;
			case 0xb8: //cancel low priority
				opb8CLOW(state, frame);
				break;
			case 0xb9: //cancel high priority
				opb9CHI(state, frame);
				break;
			case 0xba: //cancel priority range
				opbaCRAN(state, frame);
				break;
			case 0xbb: //fork
				if (opbbFORK(state, frame))
					return true;
				break;
			case 0xbc: //call
				if (opbcCALL(state, frame, script))
					return true;
				break;
			case 0xbd: //focus object
				opbdFOOB(state, frame);
				break;
			case 0xbe: //swap objects
				opbeSWOB(state, frame);
				break;
			case 0xbf: //snap object
				opbfSNOB(state, frame);
				break;
			case 0xc0: //toggle exits
				opc0TEXI(state, frame);
				break;
			case 0xc1: //print text
				opc1PTXT(state, frame);
				break;
			case 0xc2: //print newline
				opc2PNEW(state, frame);
				break;
			case 0xc3: //print text+nl
				opc3PTNE(state, frame);
				break;
			case 0xc4: //print nl+text+nl
				opc4PNTN(state, frame);
				break;
			case 0xc5: //print number
				opc5PNUM(state, frame);
				break;
			case 0xc6: //push 2
				opc6P2(state, frame);
				break;
			case 0xc7: //play sound in background
				opc7PLBG(state, frame);
				break;
			case 0xc8: //play sound and wait
				opc8PLAW(state, frame);
				break;
			case 0xc9: //wait for sound to finish?
				opc9WAIT(state, frame);
				break;
			case 0xca: //get current time
				opcaTIME(state, frame);
				break;
			case 0xcb: //get current day
				opcbDAY(state, frame);
				break;
			case 0xcc: //get children
				opccCHLD(state, frame);
				break;
			case 0xcd: //get num children
				opcdNCHLD(state, frame);
				break;
			case 0xce: //get engine version
				opceVERS(state, frame);
				break;
			case 0xcf: //push scenario number
				opcfPSCE(state, frame);
				break;
			case 0xd0: //push 1
				opd0P1(state, frame);
				break;
			case 0xd1: //get object dimensions
				opd1GOBD(state, frame);
				break;
			case 0xd2: //get overlap percent
				opd2GOVP(state, frame);
				break;
			case 0xd3: //capture children
				opd3CAPC(state, frame);
				break;
			case 0xd4: //release children
				opd4RELC(state, frame);
				break;
			case 0xd5: //show speech dialog
				opd5DLOG(state, frame);
				return true;
			case 0xd6: //activate command
				opd6ACMD(state, frame);
				break;
			case 0xd7: //lose game
				opd7LOSE(state, frame);
				break;
			case 0xd8: //win game
				opd8WIN(state, frame);
				break;
			case 0xd9: //sleep
				opd9SLEEP(state, frame);
				return true;
			case 0xda: //click to continue
				opdaCLICK(state, frame);
				return true;
			case 0xdb: //run queue
				opdbROBQ(state, frame);
				break;
			case 0xdc: //run sound queue
				opdcRSQ(state, frame);
				break;
			case 0xdd: //run text queue
				opddRTQ(state, frame);
				break;
			case 0xde: //update screen
				opdeUPSC(state, frame);
				break;
			case 0xdf: //flash main window
				opdfFMAI(state, frame);
				return true;
			case 0xe0: //cache graphic and object
				ope0CHGR(state, frame);
				break;
			case 0xe1: //cache sound
				ope1CHSO(state, frame);
				break;
			case 0xe2: //muldiv
				ope2MDIV(state, frame);
				break;
			case 0xe3: //update object
				ope3UPOB(state, frame);
				break;
			case 0xe4: //currently playing event?
				ope4PLEV(state, frame);
				break;
			case 0xe5: //wait for event to finish
				ope5WEV(state, frame);
				break;
			case 0xe6: //get fibonacci (joke)
				ope6GFIB(state, frame);
				break;
			case 0xe7: //calc fibonacci
				ope7CFIB(state, frame);
				break;
			default:
				op00NOOP(op);
			}
		}
	}
	return false;
}

int16 ScriptEngine::neg16(int16 val) {
	if (val & 0x8000)
		val = -((val ^ 0xFFFF) + 1);
	return val;
}

int16 ScriptEngine::neg8(int16 val) {
	if (val & 0x80)
		val = -((val ^ 0xff) + 1);
	return val;
}

int16 ScriptEngine::sumChildrenAttr(int16 obj, int16 attr, bool recursive) {
	int16 sum = 0;
	Common::Array<ObjID> children = _world->getChildren(obj, recursive);
	for (Common::Array<ObjID>::const_iterator it = children.begin(); it != children.end(); it++) {
		sum += _world->getObjAttr(*it, attr);
	}
	return sum;
}

void ScriptEngine::ensureNonzeroDivisor(int16 divisor, byte opcode) {
	// TODO Untested, since that occassion rarely comes up.
	if (divisor == 0) {
		error("SCRIPT: Attempt to divide by 0 in operation %x", opcode);
	}
}

void MacVenture::ScriptEngine::op80GATT(EngineState *state, EngineFrame *frame) {
	int16 obj = state->pop();
	int16 attr = state->pop();
	state->push(_world->getObjAttr(obj, attr));
}

void ScriptEngine::op81SATT(EngineState *state, EngineFrame *frame) {
	int16 obj = state->pop();
	int16 attr = state->pop();
	int16 val = neg16(state->pop());
	_world->setObjAttr(obj, attr, val);
}

void ScriptEngine::op82SUCH(EngineState *state, EngineFrame *frame) {
	int16 obj = state->pop();
	int16 attr = state->pop();
	int16 recursive = neg16(state->pop());
	state->push(sumChildrenAttr(obj, attr, recursive));
}

void ScriptEngine::op83PUCT(EngineState *state, EngineFrame *frame) {
	state->push(frame->action);
}

void ScriptEngine::op84PUOB(EngineState *state, EngineFrame *frame) {
	state->push(frame->src);
}

void ScriptEngine::op85PUTA(EngineState *state, EngineFrame *frame) {
	state->push(frame->dest);
}

void ScriptEngine::op86PUDX(EngineState *state, EngineFrame *frame) {
	state->push(frame->x);
}

void ScriptEngine::op87PUDY(EngineState *state, EngineFrame *frame) {
	state->push(frame->y);
}

void ScriptEngine::op88PUIB(EngineState *state, EngineFrame *frame, ScriptAsset *script) {
	state->push(script->fetch());
}

void ScriptEngine::op89PUI(EngineState *state, EngineFrame *frame, ScriptAsset *script) {
	int16 val = script->fetch();
	val <<= 8;
	val = val | script->fetch();
	state->push(val);
}

void ScriptEngine::op8aGGLO(EngineState *state, EngineFrame *frame) {
	int16 idx = state->pop();
	state->push(_world->getGlobal(idx));
}

void ScriptEngine::op8bSGLO(EngineState *state, EngineFrame *frame) {
	int16 idx = state->pop();
	int16 val = neg16(state->pop());
	_world->setGlobal(idx, val);
	_engine->gameChanged();
}

void ScriptEngine::op8cRAND(EngineState *state, EngineFrame *frame) {
	int16 max = state->pop();
	state->push(_engine->randBetween(0, max));
}

void ScriptEngine::op8dCOPY(EngineState *state, EngineFrame *frame) {
	int16 val = state->pop();
	state->push(val);
	state->push(val);
}

void ScriptEngine::op8eCOPYN(EngineState *state, EngineFrame *frame) {
	int16 n = state->pop();
	int16 offs = n - 1;
	int16 val;
	while (n) {
		val = state->peek(offs);
		state->push(val);
		n--;
	}
}

void ScriptEngine::op8fSWAP(EngineState *state, EngineFrame *frame) {
	int16 b = state->pop();
	int16 a = state->pop();
	state->push(b);
	state->push(a);
}

void ScriptEngine::op90SWAPN(EngineState *state, EngineFrame *frame) {
	int16 idx = state->pop();
	int16 a = state->peek(idx);
	int16 b = state->peek(0);
	state->poke(idx, b);
	state->poke(0, a);
}

void ScriptEngine::op91POP(EngineState *state, EngineFrame *frame) {
	state->pop();
}

void ScriptEngine::op92COPYP(EngineState *state, EngineFrame *frame) {
	int16 val = state->peek(1);
	state->push(val);
}

void ScriptEngine::op93COPYPN(EngineState *state, EngineFrame *frame) {
	int16 idx = state->pop();
	int16 val = state->peek(idx);
	state->push(val);
}

void ScriptEngine::op94SHUFF(EngineState *state, EngineFrame *frame) {
	int16 a = state->pop();
	int16 b = state->pop();
	int16 c = state->pop();
	state->push(a);
	state->push(c);
	state->push(b);
}

void ScriptEngine::op95SORT(EngineState *state, EngineFrame *frame) {
	int16 step = neg16(state->pop());
	int16 num = neg16(state->pop());
	step %= num;
	if (step < 0) {
		step += num;
	}
	int16 end = 0;
	int16 start = 0;
	for (int16 i = 1; i < num; i++) {
		start += step;
		if (start >= num) {
			start -= num;
		}
		if (start == end) {
			end++;
			start = end;
		} else {
			int16 a = state->peek(end);
			int16 b = state->peek(start);
			state->poke(end, b);
			state->poke(start, a);
		}
	}
}

void ScriptEngine::op96CLEAR(EngineState *state, EngineFrame *frame) {
	state->clear();
}

void ScriptEngine::op97SIZE(EngineState *state, EngineFrame *frame) {
	state->push(state->size());
}

void ScriptEngine::op98ADD(EngineState *state, EngineFrame *frame) {
	int16 b = state->pop();
	int16 a = state->pop();
	state->push(a + b);
}

void ScriptEngine::op99SUB(EngineState *state, EngineFrame *frame) {
	int16 b = state->pop();
	int16 a = state->pop();
	state->push(a - b);
}

void ScriptEngine::op9aMUL(EngineState *state, EngineFrame *frame) {
	int16 b = state->pop();
	int16 a = state->pop();
	state->push(a * b);
}

void ScriptEngine::op9bDIV(EngineState *state, EngineFrame *frame) {
	int16 b = state->pop();
	int16 a = state->pop();
	ensureNonzeroDivisor(b, 0x9b);
	state->push(a / b);
}

void ScriptEngine::op9cMOD(EngineState *state, EngineFrame *frame) {
	int16 b = state->pop();
	int16 a = state->pop();
	state->push(a % b);
}

void ScriptEngine::op9dDMOD(EngineState *state, EngineFrame *frame) {
	int16 b = state->pop();
	int16 a = state->pop();
	ensureNonzeroDivisor(b, 0x9d);
	state->push(a % b);
	state->push(a / b);
}

void ScriptEngine::op9eABS(EngineState *state, EngineFrame *frame) {
	int16 val = neg16(state->pop());
	if (val < 0) {
		val = -val;
	}
	state->push(val);
}

void ScriptEngine::op9fNEG(EngineState *state, EngineFrame *frame) {
	int16 val = -neg16(state->pop());
	state->push(val);
}

void ScriptEngine::opa0AND(EngineState *state, EngineFrame *frame) {
	int16 b = state->pop();
	int16 a = state->pop();
	state->push(a & b);
}

void ScriptEngine::opa1OR(EngineState *state, EngineFrame *frame) {
	int16 b = state->pop();
	int16 a = state->pop();
	state->push(a | b);
}

void ScriptEngine::opa2XOR(EngineState *state, EngineFrame *frame) {
	int16 b = state->pop();
	int16 a = state->pop();
	state->push(a ^ b);
}

void ScriptEngine::opa3NOT(EngineState *state, EngineFrame *frame) {
	int16 a = state->pop();
	state->push(a ^ 0xFFFF);
}

void ScriptEngine::opa4LAND(EngineState *state, EngineFrame *frame) {
	int16 b = state->pop();
	int16 a = state->pop();
	state->push((a && b) ? 0xFFFF : 0);
}

void ScriptEngine::opa5LOR(EngineState *state, EngineFrame *frame) {
	int16 b = state->pop();
	int16 a = state->pop();
	state->push((a || b) ? 0xFFFF : 0);
}

void ScriptEngine::opa6LXOR(EngineState *state, EngineFrame *frame) {
	int16 b = state->pop();
	int16 a = state->pop();
	state->push((!a != !b) ? 0xFFFF : 0);
}

void ScriptEngine::opa7LNOT(EngineState *state, EngineFrame *frame) {
	int16 a = state->pop();
	state->push((a == 0) ? 0xFFFF : 0);
}

void ScriptEngine::opa8GTU(EngineState *state, EngineFrame *frame) {
	uint16 b = state->pop();
	uint16 a = state->pop();
	state->push((a > b) ? 0xFFFF : 0);
}

void ScriptEngine::opa9LTU(EngineState *state, EngineFrame *frame) {
	uint16 b = state->pop();
	uint16 a = state->pop();
	state->push((a < b) ? 0xFFFF : 0);
}

void ScriptEngine::opaaGTS(EngineState *state, EngineFrame *frame) {
	// HACK !!! May not need the neg16, since it's already a signed int!!
	int16 b = neg16(state->pop());
	int16 a = neg16(state->pop());
	state->push((a > b) ? 0xFFFF : 0);
}

void ScriptEngine::opabLTS(EngineState *state, EngineFrame *frame) {
	// HACK !!! May not need the neg16, since it's already a signed int!!
	int16 b = neg16(state->pop());
	int16 a = neg16(state->pop());
	state->push((a < b) ? 0xFFFF : 0);
}

void ScriptEngine::opacEQ(EngineState *state, EngineFrame *frame) {
	int16 b = state->pop();
	int16 a = state->pop();
	state->push((a == b) ? 0xFFFF : 0);
}

void ScriptEngine::opadEQS(EngineState *state, EngineFrame *frame) {
	Common::String b = _world->getText(state->pop(), 0, 0); // HACK, these destinations might be wrong
	Common::String a = _world->getText(state->pop(), 0, 0);
	state->push((a == b) ? 1 : 0);
}

void ScriptEngine::opaeCONT(EngineState *state, EngineFrame *frame) {
	Common::String needle = _world->getText(state->pop(), 0, 0);
	Common::String haystack = _world->getText(state->pop(), 0, 0);
	haystack.toLowercase();
	state->push(haystack.contains(needle) ? 1 : 0);
}

void ScriptEngine::opafCONTW(EngineState *state, EngineFrame *frame) {
	Common::String needle = _world->getText(state->pop(), 0, 0);
	Common::String haystack = _world->getText(state->pop(), 0, 0);
	haystack.toLowercase();
	state->push(haystack.contains(needle) ? 1 : 0);
}

void ScriptEngine::opb0BRA(EngineState *state, EngineFrame *frame, ScriptAsset *script) {
	int16 val = script->fetch();
	val <<= 8;
	val = val | script->fetch();
	val = neg16(val);
	script->branch(val);
}

void ScriptEngine::opb1BRAB(EngineState *state, EngineFrame *frame, ScriptAsset *script) {
	int16 val = script->fetch();
	val = neg8(val);
	script->branch(val);
}

void ScriptEngine::opb2BEQ(EngineState *state, EngineFrame *frame, ScriptAsset *script) {
	int16 val = script->fetch();
	val <<= 8;
	val = val | script->fetch();
	val = neg16(val);
	int16 b = state->pop();
	if (b != 0) {
		script->branch(val);
	}
}

void ScriptEngine::opb3BEQB(EngineState *state, EngineFrame *frame, ScriptAsset *script) {
	int16 val = script->fetch();
	val = neg8(val);
	int16 b = state->pop();
	if (b != 0) {
		script->branch(val);
	}
}

void ScriptEngine::opb4BNE(EngineState *state, EngineFrame *frame, ScriptAsset *script) {
	int16 val = script->fetch();
	val <<= 8;
	val = val | script->fetch();
	val = neg16(val);
	int16 b = state->pop();
	if (b == 0) {
		script->branch(val);
	}
}

void ScriptEngine::opb5BNEB(EngineState *state, EngineFrame *frame, ScriptAsset *script) {
	int16 val = script->fetch();
	val = neg8(val);
	int16 b = state->pop();
	if (b == 0) {
		script->branch(val);
	}
}

void ScriptEngine::opb6CLAT(EngineState *state, EngineFrame *frame) {
	int16 rank = state->pop();
	int16 func = state->pop();
	frame->saves.push_back(FunCall(func, rank));
}

void ScriptEngine::opb7CCA(EngineState *state, EngineFrame *frame) {
	int16 func = state->pop();
	for (uint i = 0; i < frame->saves.size(); i++) {
		if (frame->saves[i].func == func)
			frame->saves[i].rank = 0;
	}
}

void ScriptEngine::opb8CLOW(EngineState *state, EngineFrame *frame) {
	int16 hi = state->pop();
	for (uint i = 0; i < frame->saves.size(); i++)
		if (frame->saves[i].rank <= hi)
			frame->saves[i].rank = 0;
}

void ScriptEngine::opb9CHI(EngineState *state, EngineFrame *frame) {
	int16 lo = state->pop();
	for (uint i = 0; i < frame->saves.size(); i++) {
		if (frame->saves[i].rank >= lo) {
			frame->saves[i].rank = 0;
		}
	}
}

void ScriptEngine::opbaCRAN(EngineState *state, EngineFrame *frame) {
	int16 hi = state->pop();
	int16 lo = state->pop();
	for (uint i = 0; i < frame->saves.size(); i++) {
		if (frame->saves[i].rank >= lo &&
			frame->saves[i].rank <= hi) {
			frame->saves[i].rank = 0;
		}
	}
}

bool ScriptEngine::opbbFORK(EngineState *state, EngineFrame *frame) {
	EngineFrame newframe;
	newframe.action = (ControlAction)state->pop();
	newframe.src = state->pop();
	newframe.dest = state->pop();
	newframe.x = state->pop();
	newframe.y = state->pop();
	newframe.haltedInFamily = false;
	newframe.haltedInFirst = false;
	newframe.haltedInSaves = false;
	_frames.push_front(newframe);
	if (execFrame(true)) {
		return true;
	}
	return false;
}

bool ScriptEngine::opbcCALL(EngineState *state, EngineFrame *frame, ScriptAsset &script) {
	int16 id = state->pop();
	ScriptAsset newfun = ScriptAsset(id, _scripts);
	ScriptAsset current = script;
	debugC(2, kMVDebugScript, "Call function: %d", id);
	if (loadScript(frame, id))
		return true;
	frame->scripts.pop_front();
	script = frame->scripts.front();
	debugC(2, kMVDebugScript, "Return from fuction %d", id);
	return false;
}

void ScriptEngine::opbdFOOB(EngineState *state, EngineFrame *frame) {
	int16 obj = state->pop();
	_engine->enqueueObject(kFocusWindow, obj);
}

void ScriptEngine::opbeSWOB(EngineState *state, EngineFrame *frame) {
	ObjID from = state->pop();
	ObjID to = state->pop();
	_engine->enqueueObject(kUpdateWindow, from, to);
	_world->setObjAttr(to, kAttrContainerOpen, _world->getObjAttr(from, 6));
	_world->setObjAttr(from, kAttrContainerOpen, 0);
	Common::Array<ObjID> children = _world->getChildren(from, true);
	for (uint i = 0; i < children.size(); i++) {
		_world->setObjAttr(children[i], 0, to);
	}
}

void ScriptEngine::opbfSNOB(EngineState *state, EngineFrame *frame) {
	_engine->enqueueObject(kAnimateBack, frame->src);
}

void ScriptEngine::opc0TEXI(EngineState *state, EngineFrame *frame) {
	_engine->enqueueObject(kHightlightExits, 0);
}

void ScriptEngine::opc1PTXT(EngineState *state, EngineFrame *frame) {
	int16 tid = state->pop();
	_engine->enqueueText(kTextPlain, frame->dest, frame->src, tid);
}

void ScriptEngine::opc2PNEW(EngineState *state, EngineFrame *frame) {
	_engine->enqueueText(kTextNewLine, frame->dest, frame->src, 0);
}

void ScriptEngine::opc3PTNE(EngineState *state, EngineFrame *frame) {
	int16 tid = state->pop();
	_engine->enqueueText(kTextPlain, frame->dest, frame->src, tid);
	_engine->enqueueText(kTextNewLine, frame->dest, frame->src, 0);
}

void ScriptEngine::opc4PNTN(EngineState *state, EngineFrame *frame) {
	int16 tid = state->pop();
	_engine->enqueueText(kTextNewLine, frame->dest, frame->src, 0);
	_engine->enqueueText(kTextPlain, frame->dest, frame->src, tid);
	_engine->enqueueText(kTextNewLine, frame->dest, frame->src, 0);
}

void ScriptEngine::opc5PNUM(EngineState *state, EngineFrame *frame) {
	int16 tid = state->pop();
	_engine->enqueueText(kTextNumber, frame->dest, frame->src, tid);
}

void ScriptEngine::opc6P2(EngineState *state, EngineFrame *frame) {
	state->push(2);
}

void ScriptEngine::opc7PLBG(EngineState *state, EngineFrame *frame) {
	int16 target = state->pop();
	_engine->enqueueSound(kSoundPlay, target);
}

void ScriptEngine::opc8PLAW(EngineState *state, EngineFrame *frame) {
	int16 target = state->pop();
	_engine->enqueueSound(kSoundPlayAndWait, target);
}

void ScriptEngine::opc9WAIT(EngineState *state, EngineFrame *frame) {
	_engine->enqueueSound(kSoundWait, 0);
}

void ScriptEngine::opcaTIME(EngineState *state, EngineFrame *frame) {
	TimeDate t;
	g_system->getTimeAndDate(t);

	int year = 1900 + t.tm_year;
	int month = 1 + t.tm_mon;

	state->push(year);
	state->push(month);
	state->push(t.tm_mday);
	state->push(t.tm_hour);
	state->push(t.tm_min);
	state->push(t.tm_sec);

	debugC(2, kMVDebugScript, "Saved time: Y[%d] M[%d] D[%d] h[%d] m[%d] s[%d]", year, month, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
}

void ScriptEngine::opcbDAY(EngineState *state, EngineFrame *frame) {
	TimeDate t;
	g_system->getTimeAndDate(t);

	int weekday = 1 + t.tm_wday;
	weekday = 1;
	state->push(weekday);

	debugC(2, kMVDebugScript, "Current day of week: %d", weekday);
}

void ScriptEngine::opccCHLD(EngineState *state, EngineFrame *frame) {
	bool recursive = state->pop() != 0;
	int16 obj = state->pop();
	Common::Array<ObjID> children = _world->getChildren(obj, recursive);
	for (Common::Array<ObjID>::const_iterator it = children.begin(); it != children.end(); it++) {
		state->push(*it);
	}
	state->push(children.size());
}

void ScriptEngine::opcdNCHLD(EngineState *state, EngineFrame *frame) {
	bool recursive = state->pop() != 0;
	int16 obj = state->pop();
	Common::Array<ObjID> children = _world->getChildren(obj, recursive);
	state->push(children.size());
}

void ScriptEngine::opceVERS(EngineState *state, EngineFrame *frame) {
	state->push(86); // Engine version is irrelevant. Like this in the original.
}

void ScriptEngine::opcfPSCE(EngineState *state, EngineFrame *frame) {
	state->push(0); // Any value greater than 0 indicates "Release".
}

void ScriptEngine::opd0P1(EngineState *state, EngineFrame *frame) {
	state->push(1);
}

void ScriptEngine::opd1GOBD(EngineState *state, EngineFrame *frame) {
	int16 obj = state->pop();
	Common::Rect bounds = _engine->getObjBounds(obj);
	state->push(bounds.width());
	state->push(bounds.height());
}

void ScriptEngine::opd2GOVP(EngineState *state, EngineFrame *frame) {
	int16 b = state->pop();
	int16 a = state->pop();
	state->push(_engine->getOverlapPercent(b, a));
}

void ScriptEngine::opd3CAPC(EngineState *state, EngineFrame *frame) {
	int16 obj = state->pop();
	_world->captureChildren(obj);
}

void ScriptEngine::opd4RELC(EngineState *state, EngineFrame *frame) {
	int16 obj = state->pop();
	_world->releaseChildren(obj);
}

void ScriptEngine::opd5DLOG(EngineState *state, EngineFrame *frame) {
	int16 txt = state->pop();
	if (_engine->showTextEntry(txt, frame->src, frame->dest)) {
		state->push(0xFF);
	} else {
		state->push(0x00);
	}
}

void ScriptEngine::opd6ACMD(EngineState *state, EngineFrame *frame) {
	_engine->selectControl((ControlAction)state->pop());
}

void ScriptEngine::opd7LOSE(EngineState *state, EngineFrame *frame) {
	_engine->loseGame();
}

void ScriptEngine::opd8WIN(EngineState *state, EngineFrame *frame) {
	_engine->winGame();
}

void ScriptEngine::opd9SLEEP(EngineState *state, EngineFrame *frame) {
	int16 ticks = state->pop();
	g_system->delayMillis((ticks / 60) * 1000);
	_engine->preparedToRun();
}

void ScriptEngine::opdaCLICK(EngineState *state, EngineFrame *frame) {
	_engine->updateState(false);
	_engine->clickToContinue();
}

void ScriptEngine::opdbROBQ(EngineState *state, EngineFrame *frame) {
	_engine->runObjQueue();
}

void ScriptEngine::opdcRSQ(EngineState *state, EngineFrame *frame) {
	_engine->playSounds(true);
}

void ScriptEngine::opddRTQ(EngineState *state, EngineFrame *frame) {
	_engine->printTexts();
}

void ScriptEngine::opdeUPSC(EngineState *state, EngineFrame *frame) {
	_engine->updateState(true);
}

void ScriptEngine::opdfFMAI(EngineState *state, EngineFrame *frame) {
	int16 ticks = state->pop();
	g_system->delayMillis((ticks / 60) * 1000);
	_engine->revert();
}

void ScriptEngine::ope0CHGR(EngineState *state, EngineFrame *frame) {
	state->pop();
}

void ScriptEngine::ope1CHSO(EngineState *state, EngineFrame *frame) {
	state->pop();
}

void ScriptEngine::ope2MDIV(EngineState *state, EngineFrame *frame) {
	int16 b = state->pop();
	int16 a = state->pop();
	a *= b;
	int16 c = state->pop();
	ensureNonzeroDivisor(c, 0xe2);
	a /= c;
	state->push(a);
}

void ScriptEngine::ope3UPOB(EngineState *state, EngineFrame *frame) {
	int16 obj = state->pop();
	_world->updateObj(obj);
}

void ScriptEngine::ope4PLEV(EngineState *state, EngineFrame *frame) {
	state->push(0);
}

void ScriptEngine::ope5WEV(EngineState *state, EngineFrame *frame) {
	op00NOOP(0xe5);
}

void ScriptEngine::ope6GFIB(EngineState *state, EngineFrame *frame) {
	state->push(0);
	op00NOOP(0xe6);
}

void ScriptEngine::ope7CFIB(EngineState *state, EngineFrame *frame) {
	state->pop();
	op00NOOP(0xe7);
}

void ScriptEngine::op00NOOP(byte op) {
	warning("SCRIPT: Opcode not implemented => %x", op);
}



ScriptAsset::ScriptAsset(ObjID id, Container *container) {
	_id = id;
	_container = container;
	_ip = 0x0;
	loadInstructions();
}

void ScriptAsset::reset() {
	_ip = 0x0;
}

uint8 ScriptAsset::fetch() {
	uint8 ins = _instructions[_ip];
	_ip++;
	return ins;
}

bool ScriptAsset::hasNext() {
	return _ip < _instructions.size();
}

void ScriptAsset::branch(int16 amount) {
	_ip += amount;
}

ObjID ScriptAsset::getId() {
	return _id;
}

void ScriptAsset::loadInstructions() {
	uint32 amount = _container->getItemByteSize(_id);
	Common::SeekableReadStream *res = _container->getItem(_id);
	for (uint i = 0; i < amount; i++) {
		_instructions.push_back(res->readByte());
	}
	delete res;
	debugC(2, kMVDebugScript, "Load %d instructions for script %d", amount, _id);
}

} // End of namespace MacVenture
