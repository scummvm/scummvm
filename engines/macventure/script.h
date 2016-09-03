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

#ifndef MACVENTURE_SCRIPT_H
#define MACVENTURE_SCRIPT_H

#include "macventure/container.h"
#include "macventure/world.h"
#include "macventure/macventure.h"
#include "macventure/controls.h"

namespace MacVenture {

class Container;
class World;

typedef uint32 ObjID;

class ScriptAsset {
public:
	ScriptAsset(ObjID id, Container *container);
	~ScriptAsset() {}

	void reset();
	uint8 fetch();
	bool hasNext();
	void branch(int16 amount);

	ObjID getId();

private:

	void loadInstructions();

private:
	ObjID _id;
	Container *_container;

	Common::Array<uint8> _instructions;
	uint32 _ip; // Instruction pointer
};

class EngineState {
public:
	EngineState() {
		clear();
	}

	void push(int16 data) {
		sp--;
		stack[sp] = unneg16(data);
	}

	int16 pop() {
		int16 v = stack[sp];
		sp++;
		return v;
	}

	int16 peek(int16 off) {
		return stack[sp + off];
	}

	void poke(int16 off, int16 val) {
		stack[sp + off] = unneg16(val);
	}

	void clear() {
		sp = 0x80;
		for (int i = 0; i < sp; i++) {
			stack[i] = 0;
		}
	}

	int16 size() {
		return 0x80 - sp;
	}

private:
	int16 unneg16(int16 data) {
		if (data < 0)
			data = ((-data) ^ 0xFFFF) + 1;

		return data;
	}

private:

	int16 stack[0x80];
	int16 sp;
};

struct FunCall {
	int16 func;
	int16 rank;

	FunCall(int16 f, int16 r) {
		func = f;
		rank = r;
	}
};

struct EngineFrame {
	ControlAction action;
	ObjID src;
	ObjID dest;
	int x;
	int y;
	EngineState state;
	Common::List<ScriptAsset> scripts;
	Common::Array<FunCall> saves;
	uint32 familyIdx;

	bool haltedInFirst;
	bool haltedInFamily;
	bool haltedInSaves;
};

class ScriptEngine {
public:
	ScriptEngine(MacVentureEngine *engine, World *world);
	~ScriptEngine();

public:
	bool runControl(ControlAction action, ObjID source, ObjID destination, Common::Point delta);
	bool resume(bool execAll);
	void reset();

private:
	bool execFrame(bool execAll);
	bool loadScript(EngineFrame *frame, uint32 scriptID);
	bool resumeFunc(EngineFrame *frame);
	bool runFunc(EngineFrame *frame);

private:

	// Aux
	int16 neg16(int16 val);
	int16 neg8(int16 val);
	int16 sumChildrenAttr(int16 obj, int16 attr, bool recursive);
	void ensureNonzeroDivisor(int16 divisor, byte opcode);

	// Opcodes
	void op80GATT(EngineState *state, EngineFrame *frame);	//get attribute
	void op81SATT(EngineState *state, EngineFrame *frame);	//set attribute
	void op82SUCH(EngineState *state, EngineFrame *frame);	//sum children attribute
	void op83PUCT(EngineState *state, EngineFrame *frame);	//push selected control
	void op84PUOB(EngineState *state, EngineFrame *frame);	//push selected object
	void op85PUTA(EngineState *state, EngineFrame *frame);	//push target
	void op86PUDX(EngineState *state, EngineFrame *frame);	//push deltax
	void op87PUDY(EngineState *state, EngineFrame *frame);	//push deltay
	void op88PUIB(EngineState *state, EngineFrame *frame, ScriptAsset *script);//push immediate.b
	void op89PUI(EngineState *state, EngineFrame *frame, ScriptAsset *script);//push immediate
	void op8aGGLO(EngineState *state, EngineFrame *frame);	//get global
	void op8bSGLO(EngineState *state, EngineFrame *frame);	//set global
	void op8cRAND(EngineState *state, EngineFrame *frame);	//random
	void op8dCOPY(EngineState *state, EngineFrame *frame);	//copy
	void op8eCOPYN(EngineState *state, EngineFrame *frame);	//copyn
	void op8fSWAP(EngineState *state, EngineFrame *frame);	//swap

	void op90SWAPN(EngineState *state, EngineFrame *frame);	//swapn
	void op91POP(EngineState *state, EngineFrame *frame);	//pop
	void op92COPYP(EngineState *state, EngineFrame *frame);	//copy+1
	void op93COPYPN(EngineState *state, EngineFrame *frame);//copy+n
	void op94SHUFF(EngineState *state, EngineFrame *frame);	//shuffle
	void op95SORT(EngineState *state, EngineFrame *frame);	//sort
	void op96CLEAR(EngineState *state, EngineFrame *frame);	//clear stack
	void op97SIZE(EngineState *state, EngineFrame *frame);	//get stack size
	void op98ADD(EngineState *state, EngineFrame *frame);	//add
	void op99SUB(EngineState *state, EngineFrame *frame);	//subtract
	void op9aMUL(EngineState *state, EngineFrame *frame);	//multiply
	void op9bDIV(EngineState *state, EngineFrame *frame);	//divide
	void op9cMOD(EngineState *state, EngineFrame *frame);	//mod
	void op9dDMOD(EngineState *state, EngineFrame *frame);	//divmod
	void op9eABS(EngineState *state, EngineFrame *frame);	//abs
	void op9fNEG(EngineState *state, EngineFrame *frame);	//neg

	void opa0AND(EngineState *state, EngineFrame *frame);	//and
	void opa1OR(EngineState *state, EngineFrame *frame);	//or
	void opa2XOR(EngineState *state, EngineFrame *frame);	//xor
	void opa3NOT(EngineState *state, EngineFrame *frame);	//not
	void opa4LAND(EngineState *state, EngineFrame *frame);	//logical and
	void opa5LOR(EngineState *state, EngineFrame *frame);	//logical or
	void opa6LXOR(EngineState *state, EngineFrame *frame);	//logical xor
	void opa7LNOT(EngineState *state, EngineFrame *frame);	//logical not
	void opa8GTU(EngineState *state, EngineFrame *frame);	//gt? unsigned
	void opa9LTU(EngineState *state, EngineFrame *frame);	//lt? unsigned
	void opaaGTS(EngineState *state, EngineFrame *frame);	//gt? signed
	void opabLTS(EngineState *state, EngineFrame *frame);	//lt? signed
	void opacEQ(EngineState *state, EngineFrame *frame);	//eq?
	void opadEQS(EngineState *state, EngineFrame *frame);	//eq string?
	void opaeCONT(EngineState *state, EngineFrame *frame);	//contains
	void opafCONTW(EngineState *state, EngineFrame *frame); //contains word

	void opb0BRA(EngineState *state, EngineFrame *frame, ScriptAsset *script);	//bra
	void opb1BRAB(EngineState *state, EngineFrame *frame, ScriptAsset *script);	//bra.b
	void opb2BEQ(EngineState *state, EngineFrame *frame, ScriptAsset *script);	//beq
	void opb3BEQB(EngineState *state, EngineFrame *frame, ScriptAsset *script); //beq.b
	void opb4BNE(EngineState *state, EngineFrame *frame, ScriptAsset *script);	//bne
	void opb5BNEB(EngineState *state, EngineFrame *frame, ScriptAsset *script);	//bne.b
	void opb6CLAT(EngineState *state, EngineFrame *frame);	//call later
	void opb7CCA(EngineState *state, EngineFrame *frame);	//cancel call
	void opb8CLOW(EngineState *state, EngineFrame *frame);	//cancel low priority
	void opb9CHI(EngineState *state, EngineFrame *frame);	//cancel high priority
	void opbaCRAN(EngineState *state, EngineFrame *frame);	//cancel priority range
	bool opbbFORK(EngineState *state, EngineFrame *frame);	//fork
	bool opbcCALL(EngineState *state, EngineFrame *frame, ScriptAsset &script);	//call
	void opbdFOOB(EngineState *state, EngineFrame *frame);	//focus object
	void opbeSWOB(EngineState *state, EngineFrame *frame);	//swap objects
	void opbfSNOB(EngineState *state, EngineFrame *frame);	//snap object

	void opc0TEXI(EngineState *state, EngineFrame *frame);	//toggle exits
	void opc1PTXT(EngineState *state, EngineFrame *frame);	//print text
	void opc2PNEW(EngineState *state, EngineFrame *frame);	//print newline
	void opc3PTNE(EngineState *state, EngineFrame *frame);	//print text+nl
	void opc4PNTN(EngineState *state, EngineFrame *frame);	//print nl+text+nl
	void opc5PNUM(EngineState *state, EngineFrame *frame);	//print number
	void opc6P2(EngineState *state, EngineFrame *frame);	//push 2
	void opc7PLBG(EngineState *state, EngineFrame *frame);	//play sound in background
	void opc8PLAW(EngineState *state, EngineFrame *frame);	//play sound and wait
	void opc9WAIT(EngineState *state, EngineFrame *frame);	//wait for sound to finish?
	void opcaTIME(EngineState *state, EngineFrame *frame);	//get current time
	void opcbDAY(EngineState *state, EngineFrame *frame);	//get current day
	void opccCHLD(EngineState *state, EngineFrame *frame);	//get children
	void opcdNCHLD(EngineState *state, EngineFrame *frame); //get num children
	void opceVERS(EngineState *state, EngineFrame *frame);	//get engine version
	void opcfPSCE(EngineState *state, EngineFrame *frame);	//push scenario number

	void opd0P1(EngineState *state, EngineFrame *frame);	//push 1
	void opd1GOBD(EngineState *state, EngineFrame *frame);	//get object dimensions
	void opd2GOVP(EngineState *state, EngineFrame *frame);	//get overlap percent
	void opd3CAPC(EngineState *state, EngineFrame *frame);	//capture children
	void opd4RELC(EngineState *state, EngineFrame *frame);	//release children
	void opd5DLOG(EngineState *state, EngineFrame *frame);	//show speech dialog
	void opd6ACMD(EngineState *state, EngineFrame *frame);	//activate command
	void opd7LOSE(EngineState *state, EngineFrame *frame);	//lose game
	void opd8WIN(EngineState *state, EngineFrame *frame);	//win game
	void opd9SLEEP(EngineState *state, EngineFrame *frame);	//sleep
	void opdaCLICK(EngineState *state, EngineFrame *frame);	//click to continue
	void opdbROBQ(EngineState *state, EngineFrame *frame);	//run queue
	void opdcRSQ(EngineState *state, EngineFrame *frame);	//run sound queue
	void opddRTQ(EngineState *state, EngineFrame *frame);	//run text queue
	void opdeUPSC(EngineState *state, EngineFrame *frame);	//update screen
	void opdfFMAI(EngineState *state, EngineFrame *frame);	//flash main window

	void ope0CHGR(EngineState *state, EngineFrame *frame);	//cache graphic and object
	void ope1CHSO(EngineState *state, EngineFrame *frame);	//cache sound
	void ope2MDIV(EngineState *state, EngineFrame *frame);	//muldiv
	void ope3UPOB(EngineState *state, EngineFrame *frame);	//update object
	void ope4PLEV(EngineState *state, EngineFrame *frame);	//currently playing event?
	void ope5WEV(EngineState *state, EngineFrame *frame);	//wait for event to finish
	void ope6GFIB(EngineState *state, EngineFrame *frame);	//get fibonacci (joke)
	void ope7CFIB(EngineState *state, EngineFrame *frame);	//calc fibonacci

	void op00NOOP(byte op);

private:
	MacVentureEngine *_engine;
	World *_world;
	Common::List<EngineFrame> _frames;
	Container *_scripts;
};

} // End of namespace MacVenture

#endif
