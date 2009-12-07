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
 * $URL$
 * $Id$
 *
 */

#include "m4/woodscript.h"

namespace M4 {

static const long sinCosTable[320] = {
	0,		1608,	3215,	4821,	6423,	8022,	9616,	11204,
	12785,	14359,	15923,	17479,	19024,	20557,	22078,	23586,
	25079,	26557,	28020,	29465,	30893,	32302,	33692,	35061,
	36409,	37736,	39039,	40319,	41575,	42806,	44011,	45189,
	46340,	47464,	48558,	49624,	50660,	51665,	52639,	53581,
	54491,	55368,	56212,	57022,	57797,	58538,	59243,	59913,
	60547,	61144,	61705,	62228,	62714,	63162,	63571,	63943,
	64276,	64571,	64826,	65043,	65220,	65358,	65457,	65516,
	65536,	65516,	65457,	65358,	65220,	65043,	64826,	64571,
	64276,	63943,	63571,	63162,	62714,	62228,	61705,	61144,
	60547,	59913,	59243,	58538,	57797,	57022,	56212,	55368,
	54491,	53581,	52639,	51665,	50660,	49624,	48558,	47464,
	46340,	45189,	44011,	42806,	41575,	40319,	39039,	37736,
	36409,	35061,	33692,	32302,	30893,	29465,	28020,	26557,
	25079,	23586,	22078,	20557,	19024,	17479,	15923,	14359,
	12785,	11204,	9616,	8022,	6423,	4821,	3215,	1608,
	0,		-1608,	-3215,	-4821,	-6423,	-8022,	-9616,	-11204,
	-12785,	-14359,	-15923,	-17479,	-19024,	-20557,	-22078,	-23586,
	-25079,	-26557,	-28020,	-29465,	-30893,	-32302,	-33692,	-35061,
	-36409,	-37736,	-39039,	-40319,	-41575,	-42806,	-44011,	-45189,
	-46340,	-47464,	-48558,	-49624,	-50660,	-51665,	-52639,	-53581,
	-54491,	-55368,	-56212,	-57022,	-57797,	-58538,	-59243,	-59913,
	-60547,	-61144,	-61705,	-62228,	-62714,	-63162,	-63571,	-63943,
	-64276,	-64571,	-64826,	-65043,	-65220,	-65358,	-65457,	-65516,
	-65536,	-65516,	-65457,	-65358,	-65220,	-65043,	-64826,	-64571,
	-64276,	-63943,	-63571,	-63162,	-62714,	-62228,	-61705,	-61144,
	-60547,	-59913,	-59243,	-58538,	-57797,	-57022,	-56212,	-55368,
	-54491,	-53581,	-52639,	-51665,	-50660,	-49624,	-48558,	-47464,
	-46340,	-45189,	-44011,	-42806,	-41575,	-40319,	-39039,	-37736,
	-36409,	-35061,	-33692,	-32302,	-30893,	-29465,	-28020,	-26557,
	-25079,	-23586,	-22078,	-20557,	-19024,	-17479,	-15923,	-14359,
	-12785,	-11204,	-9616,	-8022,	-6423,	-4821,	-3215,	-1608,
	0,		1608,	3215,	4821,	6423,	8022,	9616,	11204,
	12785,	14359,	15923,	17479,	19024,	20557,	22078,	23586,
	25079,	26557,	28020,	29465,	30893,	32302,	33692,	35061,
	36409,	37736,	39039,	40319,	41575,	42806,	44011,	45189,
	46340,	47464,	48558,	49624,	50660,	51665,	52639,	53581,
	54491,	55368,	56212,	57022,	57797,	58538,	59243,	59913,
	60547,	61144,	61705,	62228,	62714,	63162,	63571,	63943,
	64276,	64571,	64826,	65043,	65220,	65358,	65457,	65516
};

const long *sinTable = &(sinCosTable[0]);
const long *cosTable = &(sinCosTable[64]);

// FIXME: Tables

const int sequenceVariableCount = 33;

enum SequenceVariables {
	kSeqVarTimer					= 0,
	kSeqVarTag						= 1,
	kSeqVarLayer					= 2,
	kSeqVarWidth					= 3,
	kSeqVarHeight					= 4,
	kSeqVarX						= 5,
	kSeqVarY						= 6,
	kSeqVarScale					= 7,
	kSeqVarR						= 8,
	kSeqVarSpriteHash				= 9,
	kSeqVarSpriteFrameNumber		= 10,
	kSeqVarSpriteFrameCount			= 11,
	kSeqVarSpriteFrameRate			= 12,
	kSeqVarSpriteFramePixelSpeed	= 13,
	kSeqVarTargetS					= 14,
	kSeqVarTargetR					= 15,
	kSeqVarTargetX					= 16,
	kSeqVarTargetY					= 17,
	kSeqVarDeltaS					= 18,
	kSeqVarDeltaR					= 19,
	kSeqVarDeltaX					= 20,
	kSeqVarDeltaY					= 21,
	kSeqVarVelocity					= 22,
	kSeqVarTheta					= 23,
	kSeqVarTemp1					= 24,
	kSeqVarTemp2					= 25,
	kSeqVarTemp3					= 26,
	kSeqVarTemp4					= 27,
	kSeqVarTemp5					= 28,
	kSeqVarTemp6					= 29,
	kSeqVarTemp7					= 30,
	kSeqVarTemp8					= 31,
	kSeqVarMachineID				= 32
};

bool (Sequence::*sequenceCommandsTable[])(Instruction &instruction) = {
	&Sequence::s1_end,
	&Sequence::s1_clearVars,
	&Sequence::s1_set,
	&Sequence::s1_compare,
	&Sequence::s1_add,
	&Sequence::s1_sub,
	&Sequence::s1_mul,
	&Sequence::s1_div,
	&Sequence::s1_and,
	&Sequence::s1_or,
	&Sequence::s1_not,
	&Sequence::s1_sin,
	&Sequence::s1_cos,
	&Sequence::s1_abs,
	&Sequence::s1_min,
	&Sequence::s1_max,
	&Sequence::s1_mod,
	&Sequence::s1_floor,
	&Sequence::s1_round,
	&Sequence::s1_ceil,
	&Sequence::s1_point,
	&Sequence::s1_dist2d,
	&Sequence::s1_crunch,
	&Sequence::s1_branch,
	&Sequence::s1_setFrame,
	&Sequence::s1_sendMessage,
	&Sequence::s1_push,
	&Sequence::s1_pop,
	&Sequence::s1_jumpSub,
	&Sequence::s1_return,
	&Sequence::s1_getFrameCount,
	&Sequence::s1_getFrameRate,
	&Sequence::s1_getCelsPixSpeed,
	&Sequence::s1_setIndex,
	&Sequence::s1_setLayer,
	&Sequence::s1_setDepth,
	&Sequence::s1_setData,
	&Sequence::s1_openStream,
	&Sequence::s1_streamNextFrame,
	&Sequence::s1_closeStream
};

Sequence::Sequence(WoodScript *ws, Machine *machine, int32 sequenceHash) {

	_ws = ws;

	SequenceAsset *sequenceAsset = _ws->assets()->getSequence(sequenceHash);

	// initialize the sequence's bytecode
	byte *code;
	uint32 codeSize;
	sequenceAsset->getCode(code, codeSize);
	_code = new Bytecode(_ws, code, codeSize, this);

	_active = true;
	_sequenceHash = sequenceHash;
	_machine = machine;
	_parentSequence = _machine->parentSequence();
	_dataRow = _machine->dataRow();
	_startTime = 0;
	_switchTime = 0;
	//TODO _flags = 0;
	_localVarCount = sequenceAsset->localVarCount();
	_vars = new long[sequenceVariableCount + _localVarCount];
	_returnStackIndex = 0;
	_layer = 0;
	_terminated = false;

	clearVars();
	_vars[kSeqVarMachineID] = _machine->getId();

}

Sequence::~Sequence() {
	delete _code;
}

void Sequence::pause() {
	_active = false;
}

void Sequence::resume() {
	_active = true;
}

void Sequence::issueEndOfSequenceRequest(int32 codeOffset, int32 count) {

	//printf("Sequence::issueEndOfSequenceRequest(%04X, %04X)\n", codeOffset, count); fflush(stdout);
	//g_system->delayMillis(5000);

	_endOfSequenceRequest.codeOffset = codeOffset;
	_endOfSequenceRequest.count = count;
}

void Sequence::cancelEndOfSequenceRequest() {
	_endOfSequenceRequest.codeOffset = -1;
}

bool Sequence::runProgram() {

	bool done = true;

	//printf("_ws->getGlobal(kGlobTime) = %ld, _switchTime = %d\n", _ws->getGlobal(kGlobTime), _switchTime);

	if (_switchTime >= 0 && _ws->getGlobal(kGlobTime) >= _switchTime)
		done = false;

	_vars[kSeqVarTimer] -= _ws->getGlobal(kGlobTimeDelta) << 16;

	while (!done) {
		Instruction instruction;
		_code->loadInstruction(instruction);
		if (sequenceCommandsTable[instruction.instr] != NULL)
			done = !(this->*sequenceCommandsTable[instruction.instr])(instruction);
		else { fflush(stdout); /*g_system->delayMillis(1000);*/ }
	}

	return _terminated;
}

void Sequence::clearVars() {
	for (int i = 0; i < sequenceVariableCount + _localVarCount; i++)
		_vars[i] = 0;
	// set default scaling to 100%
	_vars[kSeqVarScale] = 0x10000;
}

bool Sequence::changeProgram(int32 sequenceHash) {

	SequenceAsset *sequenceAsset = _ws->assets()->getSequence(sequenceHash);

	if (sequenceAsset->localVarCount() > _localVarCount) {
		//printf("Sequence::changeProgram(%d) sequenceAsset->localVarCount() > _localVarCount\n", sequenceHash);
		return false;
	}

	// Initialize the sequence's bytecode
	byte *code;
	uint32 codeSize;
	sequenceAsset->getCode(code, codeSize);
	_code->setCode(code, codeSize);

	// Reset status variables
	_switchTime = 0;
	_active = true;
	_terminated = false;
	_endOfSequenceRequest.codeOffset = -1;

	_sequenceHash = sequenceHash;
	_returnStackIndex = 0;

	return true;

}

long *Sequence::getVarPtr(int index) {
	return &_vars[index];
}

long *Sequence::getParentVarPtr(int index) {
	return _parentSequence->getVarPtr(index);
}

long *Sequence::getDataPtr(int index) {
	return &_dataRow[index];
}

void Sequence::draw(M4Surface *surface, const Common::Rect &clipRect, Common::Rect &updateRect) {

	SpriteInfo info;

	info.sprite = _curFrame;
	info.hotX = _curFrame->xOffset;
	info.hotY = _curFrame->yOffset;
	info.encoding = _curFrame->encoding;
	info.inverseColorTable = _vm->_scene->getInverseColorTable();
	info.palette = _ws->getMainPalette();
	info.width = _curFrame->width();
	info.height = _curFrame->height();
	int32 scaler = FixedMul(_vars[kSeqVarScale], 100 << 16) >> 16;
	info.scaleX = _vars[kSeqVarWidth] < 0 ? -scaler : scaler;
	info.scaleY = scaler;
	surface->drawSprite(_vars[kSeqVarX] >> 16, _vars[kSeqVarY] >> 16, info, clipRect);

}

bool Sequence::s1_end(Instruction &instruction) {
	//printf("Sequence::s1_end()\n");

	_terminated = true;
	return false;
}

bool Sequence::s1_clearVars(Instruction &instruction) {
	//printf("Sequence::s1_clearVars()\n");

	clearVars();
	_vars[kSeqVarMachineID] = _machine->getId();
	return true;
}

bool Sequence::s1_set(Instruction &instruction) {
	//printf("Sequence::s1_set()\n");

	*instruction.argp[0] = instruction.getValue();
	return true;
}

bool Sequence::s1_compare(Instruction &instruction) {
	//printf("Sequence::s1_compare()\n");

	long value = instruction.getValue();
	if (instruction.argv[0] < value)
		_cmpFlags = -1;
	else if (instruction.argv[0] > value)
		_cmpFlags = 1;
	else
		_cmpFlags = 0;
	return true;
}

bool Sequence::s1_add(Instruction &instruction) {
	//printf("Sequence::s1_add()\n");

	*instruction.argp[0] += instruction.getValue();
	return true;
}

bool Sequence::s1_sub(Instruction &instruction) {
	//printf("Sequence::s1_sub()\n");

	*instruction.argp[0] -= instruction.getValue();
	return true;
}

bool Sequence::s1_mul(Instruction &instruction) {
	//printf("Sequence::s1_mul()\n");

	*instruction.argp[0] = FixedMul(instruction.argv[0], instruction.getValue());
	return true;
}

bool Sequence::s1_div(Instruction &instruction) {
	//printf("Sequence::s1_div()\n");

	// TODO: Catch divisor = 0 in FixedDiv
	*instruction.argp[0] = FixedDiv(instruction.argv[0], instruction.getValue());
	return true;
}

bool Sequence::s1_and(Instruction &instruction) {
	//printf("Sequence::s1_and()\n");

	*instruction.argp[0] = instruction.argv[0] & instruction.getValue();
	if (*instruction.argp[0])
		_cmpFlags = 0;
	else
		_cmpFlags = 1;
	return true;
}

bool Sequence::s1_or(Instruction &instruction) {
	//printf("Sequence::s1_or()\n");

	*instruction.argp[0] = instruction.argv[0] | instruction.getValue();
	if (*instruction.argp[0])
		_cmpFlags = 0;
	else
		_cmpFlags = 1;
	return true;
}

bool Sequence::s1_not(Instruction &instruction) {
	//printf("Sequence::s1_not()\n");

	if (instruction.argv[0] == 0) {
		*instruction.argp[0] = 0x10000;
		_cmpFlags = 1;
	} else {
		*instruction.argp[0] = 0;
		_cmpFlags = 0;
	}
	return true;
}

bool Sequence::s1_sin(Instruction &instruction) {
	//printf("Sequence::s1_sin()\n");

	int32 tempAngle = *instruction.argp[1] >> 16;
	if (tempAngle < 0)
		tempAngle = 0x0100 - ((-tempAngle) & 0xff);
	else
		tempAngle &= 0xff;

	// FIXME: Why use the cosTable in s1_sin() ?
	// Note that sin(0) 0 and sinTable[0] = 0 but cos(0)=1, and indeed
	// cosTable[0] = 65536, which is 1 considered as a fixed point.
	*instruction.argp[0] = -cosTable[tempAngle];

	return true;
}

bool Sequence::s1_cos(Instruction &instruction) {
	//printf("Sequence::s1_cos()\n");

	int32 tempAngle = *instruction.argp[1] >> 16;
	if (tempAngle < 0)
		tempAngle = 0x0100 - ((-tempAngle) & 0xff);
	else
		tempAngle &= 0xff;

	// FIXME: Why use the sinTable in s1_cos() ?
	// Note that sin(0) 0 and sinTable[0] = 0 but cos(0)=1, and indeed
	// cosTable[0] = 65536, which is 1 considered as a fixed point.
	*instruction.argp[0] = sinTable[tempAngle];

	return true;
}

bool Sequence::s1_abs(Instruction &instruction) {
	//printf("Sequence::s1_abs()\n");

	*instruction.argp[0] = ABS(instruction.argv[1]);
	return true;
}

bool Sequence::s1_min(Instruction &instruction) {
	//printf("Sequence::s1_min()\n");

	*instruction.argp[0] = MIN(instruction.argv[1], instruction.argv[2]);
	return true;
}

bool Sequence::s1_max(Instruction &instruction) {
	//printf("Sequence::s1_max()\n");

	*instruction.argp[0] = MAX(instruction.argv[1], instruction.argv[2]);
	return true;
}

bool Sequence::s1_mod(Instruction &instruction) {
	//printf("Sequence::s1_mod()\n");

	*instruction.argp[0] = instruction.argv[0] % instruction.getValue();
	return true;
}

bool Sequence::s1_floor(Instruction &instruction) {
	//printf("Sequence::s1_floor()\n");

	*instruction.argp[0] = instruction.getValue() & 0xffff0000;
	return true;
}

bool Sequence::s1_round(Instruction &instruction) {
	//printf("Sequence::s1_round()\n");

	if ((*instruction.argp[1] & 0xffff) >= 0x8000)
		*instruction.argp[0] = (*instruction.argp[1] + 0x10000) & 0xffff0000;
	else
		*instruction.argp[0] = *instruction.argp[1] & 0xffff0000;
	return true;
}

bool Sequence::s1_ceil(Instruction &instruction) {
	//printf("Sequence::s1_ceil()\n");

	if ((*instruction.argp[1] & 0xffff) >= 0)
		*instruction.argp[0] = (*instruction.argp[1] + 0x10000) & 0xffff0000;
	else
		*instruction.argp[0] = *instruction.argp[1] & 0xffff0000;
	return true;
}

bool Sequence::s1_point(Instruction &instruction) {
	printf("Sequence::s1_point()\n");
	// TODO
	return true;
}

bool Sequence::s1_dist2d(Instruction &instruction) {
	printf("Sequence::s1_dist2d()\n");
	// TODO
	return true;
}

bool Sequence::s1_crunch(Instruction &instruction) {
	//printf("Sequence::s1_crunch()\n");

	long deltaTime;

	if (instruction.argc == 2) {
		deltaTime = _vm->imath_ranged_rand16(instruction.argv[0], instruction.argv[1]);
	} else if (instruction.argc == 1) {
		deltaTime = instruction.argv[0];
	} else {
		deltaTime = 0;
	}

	_startTime = _ws->getGlobal(kGlobTime);

	//printf("deltaTime = %ld\n", deltaTime >> 16); fflush(stdout);
	//g_system->delayMillis(5000);

	if (deltaTime >= 0) {
		_switchTime = _ws->getGlobal(kGlobTime) + (deltaTime >> 16);
		//printf("_ws->getGlobal(kGlobTime) = %ld\n", _ws->getGlobal(kGlobTime)); fflush(stdout);
		//g_system->delayMillis(5000);
	} else {
		_switchTime = -1;
	}

	// TODO: Update if walking etc.

	return false;
}

bool Sequence::s1_branch(Instruction &instruction) {
	//printf("Sequence::s1_branch()\n");

	uint32 ofs = instruction.argv[1] >> 16;
	switch (instruction.argv[0] >> 16) {
	case 0: // jmp
		_code->jumpRelative(ofs);
		break;
	case 1: // <
		if (_cmpFlags < 0)
			_code->jumpRelative(ofs);
		break;
	case 2: // <=
		if (_cmpFlags <= 0)
			_code->jumpRelative(ofs);
		break;
	case 3: // ==
		if (_cmpFlags == 0)
			_code->jumpRelative(ofs);
		break;
	case 4: // !=
		if (_cmpFlags != 0)
			_code->jumpRelative(ofs);
		break;
	case 5: // >=
		if (_cmpFlags >= 0)
			_code->jumpRelative(ofs);
		break;
	case 6: // >
		if (_cmpFlags > 0)
			_code->jumpRelative(ofs);
		break;
	}

	return true;
}

bool Sequence::s1_setFrame(Instruction &instruction) {
	//printf("Sequence::s1_setFrame()\n");

	int32 frameIndex;
	if (instruction.argc == 3) {
		frameIndex = _vm->imath_ranged_rand(instruction.argv[1] >> 16, instruction.argv[2] >> 16);
	} else if (instruction.argc == 2) {
		frameIndex = instruction.argv[1] >> 16;
	} else {
		frameIndex = (instruction.argv[0] & 0xFF0000) >> 16;
	}

	//printf("Sequence::s1_setFrame() spriteHash = %d\n", (uint32)instruction.argv[0] >> 24);
	//printf("Sequence::s1_setFrame() frameIndex = %d\n", frameIndex);

	SpriteAsset *spriteAsset = _ws->assets()->getSprite((uint32)instruction.argv[0] >> 24);
	_curFrame = spriteAsset->getFrame(frameIndex);

	return true;
}

bool Sequence::s1_sendMessage(Instruction &instruction) {
	printf("Sequence::s1_sendMessage()\n");
	// TODO
	return true;
}

bool Sequence::s1_push(Instruction &instruction) {
	printf("Sequence::s1_push()\n");
	// TODO
	return true;
}

bool Sequence::s1_pop(Instruction &instruction) {
	printf("Sequence::s1_pop()\n");
	// TODO
	return true;
}

bool Sequence::s1_jumpSub(Instruction &instruction) {
	//printf("Sequence::s1_jumpSub()\n");

	_returnHashes[_returnStackIndex] = _sequenceHash;
	_returnOffsets[_returnStackIndex] = _code->pos();
	_returnStackIndex++;

	_sequenceHash = instruction.argv[0] >> 16;

	SequenceAsset *sequenceAsset = _ws->assets()->getSequence(_sequenceHash);

	// initialize the sequence's bytecode
	byte *code;
	uint32 codeSize;
	sequenceAsset->getCode(code, codeSize);
	_code->setCode(code, codeSize);

	return true;
}

bool Sequence::s1_return(Instruction &instruction) {
	//printf("Sequence::s1_return()\n");

	if (_returnStackIndex <= 0)
		return s1_end(instruction);

	_returnStackIndex--;

	_sequenceHash = _returnHashes[_returnStackIndex];
	uint32 ofs = _returnOffsets[_returnStackIndex];

	SequenceAsset *sequenceAsset = _ws->assets()->getSequence(_sequenceHash);

	// initialize the sequence's bytecode
	byte *code;
	uint32 codeSize;
	sequenceAsset->getCode(code, codeSize);
	_code->setCode(code, codeSize);
	_code->jumpAbsolute(ofs);


	return true;
}

bool Sequence::s1_getFrameCount(Instruction &instruction) {
	//printf("Sequence::s1_getFrameCount()\n");

	SpriteAsset *spriteAsset = _ws->assets()->getSprite(instruction.argv[1] >> 24);
	*instruction.argp[0] = spriteAsset->getCount() << 16;
	return true;
}

bool Sequence::s1_getFrameRate(Instruction &instruction) {
	//printf("Sequence::s1_getFrameRate()\n");

	SpriteAsset *spriteAsset = _ws->assets()->getSprite(instruction.argv[1] >> 24);
	*instruction.argp[0] = spriteAsset->getFrameRate();
	return true;
}

bool Sequence::s1_getCelsPixSpeed(Instruction &instruction) {
	printf("Sequence::s1_getCelsPixSpeed()\n");
	// TODO
	return true;
}

bool Sequence::s1_setIndex(Instruction &instruction) {
	printf("Sequence::s1_setIndex()\n");
	// TODO
	return true;
}

bool Sequence::s1_setLayer(Instruction &instruction) {
	printf("Sequence::s1_setLayer()\n");
	//TODO
	return true;
}

bool Sequence::s1_setDepth(Instruction &instruction) {
	printf("Sequence::s1_setDepth()\n");
	//TODO
	return true;
}

bool Sequence::s1_setData(Instruction &instruction) {
	printf("Sequence::s1_setData()\n");
	//TODO
	return true;
}

bool Sequence::s1_openStream(Instruction &instruction) {
	//printf("Sequence::s1_openStream()\n");

	_stream = _vm->res()->openFile(_machine->name().c_str());
	streamOpen();
	return true;
}

bool Sequence::s1_streamNextFrame(Instruction &instruction) {
	//printf("Sequence::s1_streamNextFrame()\n");

	streamNextFrame();
	return true;
}

bool Sequence::s1_closeStream(Instruction &instruction) {
	printf("Sequence::s1_closeStream()\n");
	//TODO
	return true;
}

bool Sequence::streamOpen() {

	_streamSpriteAsset = new SpriteAsset(_vm, _stream, _stream->size(), "stream", true);

	_vars[kSeqVarSpriteFrameNumber] = -0x10000;
	_vars[kSeqVarSpriteFrameCount] = _streamSpriteAsset->getCount() << 16;
	_vars[kSeqVarSpriteFrameRate] = _streamSpriteAsset->getFrameRate() << 16;

	//printf("Sequence::streamOpen() frames = %d; max = %d x %d\n", _streamSpriteAsset->getCount(), _streamSpriteAsset->getMaxFrameWidth(), _streamSpriteAsset->getMaxFrameHeight());
	//fflush(stdout);

	_curFrame = new M4Sprite(_vm, _streamSpriteAsset->getMaxFrameWidth(), _streamSpriteAsset->getMaxFrameHeight());
	streamNextFrame();

	// TODO: Just a hack to see the series with the correct palette.
	_vm->_palette->setPalette(_streamSpriteAsset->getPalette(), 0, 256);

	return true;
}

bool Sequence::streamNextFrame() {

	_vars[kSeqVarSpriteFrameNumber] += 0x10000;

	int32 frameNum = _vars[kSeqVarSpriteFrameNumber] >> 16;
	if (frameNum >= _streamSpriteAsset->getCount()) {
		// End reached
		return false;
	}

	_streamSpriteAsset->loadStreamingFrame(_curFrame, frameNum, _vars[kSeqVarX], _vars[kSeqVarY]);

	_vars[kSeqVarWidth] = _curFrame->width() << 16;
	_vars[kSeqVarHeight] = _curFrame->height() << 16;

	return true;
}

void Sequence::streamClose() {
	_stream = NULL;
	_vm->res()->toss(_machine->name().c_str());
	//_vm->res()->purge();
	delete _streamSpriteAsset;
	delete _curFrame;
	_stream = NULL;
	_streamSpriteAsset = NULL;
	_curFrame = NULL;
}

}
