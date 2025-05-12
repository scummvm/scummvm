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
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/


#include "common/endian.h"
#include "snatcher/graphics.h"
#include "snatcher/resource.h"
#include "snatcher/script.h"
#include "snatcher/snatcher.h"
#include "snatcher/sound.h"


namespace Snatcher {

CmdQueue::CmdQueue(SnatcherEngine *vm) : _vm(vm), _data(nullptr), _readPos(0), _writePos(nullptr), _enable(false), _progress(-1), _currentOpcode(0) {
	_data = new uint16[256]();
	makeFunctions();
	reset();
}

CmdQueue::~CmdQueue() {
	delete[] _data;
	for (Common::Array<CmdQueOpcode*>::iterator i = _opcodes.begin(); i != _opcodes.end(); ++ i)
		delete *i;
}

void CmdQueue::reset() {
	memset(_data, 0, 256);
	_readPos = _writePos = _data;
	_enable = false;
	_progress = -1;
}

void CmdQueue::writeUInt16(uint16 val) {
	*_writePos++ = val;
}

void CmdQueue::writeUInt32(uint32 val) {
	*reinterpret_cast<uint32*>(_writePos) = val;
	_writePos += 2;
}

void CmdQueue::run() {
	if (!_enable)
		return;

	if (_progress == -1) {
		_currentOpcode = *_readPos++;
		_progress = 0;
	}

	if (_currentOpcode < _opcodes.size() && _opcodes[_currentOpcode]->isValid())
		(*_opcodes[_currentOpcode])(_readPos);
	else
		error("%s(): Invalid opcode %d", __FUNCTION__, _currentOpcode);
}

void CmdQueue::makeFunctions() {
#define OP(x) &CmdQueue::m_##x
	typedef void (CmdQueue::*SFunc)(const uint16*&);
	static const SFunc funcTbl[] = {
		OP(end),
		OP(01),
		OP(02),
		OP(03),
		OP(04),
		OP(05),
		OP(06),
		OP(fmMusicStart),
		OP(gfxOps),
		OP(animOps),
		OP(pcmWait),
		OP(11),
		OP(12),
		OP(mmSeq),
		OP(14),
		OP(gfxReset),
		OP(waitFrames),
		OP(loadResource),
		OP(gfxStart),
		OP(fmMusicWait),
		OP(20),
		OP(21),
		OP(22),
		OP(23),
		OP(24),
		OP(25),
		OP(26),
		OP(27),
		OP(28),
		OP(29),
		OP(30),
		OP(31),
		OP(32),
		OP(33),
		OP(34),
		OP(35)
	};
#undef OP

	for (uint i = 0; i < ARRAYSIZE(funcTbl); ++i)
		_opcodes.push_back(new CmdQueOpcode(this, funcTbl[i]));
}

void CmdQueue::m_end(const uint16 *&data) {
	reset();
}

void CmdQueue::m_01(const uint16 *&data) {
}

void CmdQueue::m_02(const uint16 *&data) {
}

void CmdQueue::m_03(const uint16 *&data) {
}

void CmdQueue::m_04(const uint16 *&data) {
}

void CmdQueue::m_05(const uint16 *&data) {
}

void CmdQueue::m_06(const uint16 *&data) {
}

void CmdQueue::m_fmMusicStart(const uint16 *&data) {
	uint8 cmd = *data++;
	_vm->sound()->fmSendCommand(cmd, 1);
	_vm->sound()->_fmPlayingTracks.music = (cmd == 0xFF) ? 0 : cmd;
	_progress = -1;
}

void CmdQueue::m_gfxOps(const uint16 *&data) {
}

void CmdQueue::m_animOps(const uint16 *&data) {
	switch (data[0]) {
	case 0:
	case 1:
	case 2:
		_vm->gfx()->setAnimGroupParameter(data[1], data[0] + 4);
		break;
	case 3:
		if (_vm->gfx()->getAnimParameter(data[1], GraphicsEngine::kAnimParaEnable)) {
			if (!_vm->gfx()->testAnimParameterFlags(data[1], GraphicsEngine::kAnimParaScriptComFlags, 2))
				return;
			_vm->gfx()->clearAnimParameterFlags(data[1], GraphicsEngine::kAnimParaScriptComFlags, 2);
		}
		break;
	case 4:
		_vm->gfx()->setAnimParameter(data[1], GraphicsEngine::kAnimParaScriptComFlags, 0xFF);
		break;
	case 5:
		_vm->gfx()->setAnimGroupParameter(data[1], 7);
		break;
	default:
		error("%s(): Unknown command %d", __FUNCTION__, data[0]);
	}
	data += 2;
	_progress = -1;
}

void CmdQueue::m_pcmWait(const uint16 *&data) {
	if (_progress == 0) {
		_counter = 3600;
		++_progress;
	}
	if (--_counter == 0 || !(_vm->sound()->pcmGetStatus() & 0x0F)) 
		_progress = -1;
}

void CmdQueue::m_11(const uint16 *&data) {
}

void CmdQueue::m_12(const uint16 *&data) {
}

void CmdQueue::m_mmSeq(const uint16 *&data) {
	if (_progress == 0) {
		_vm->sound()->pcmInitSound(*data);
		//_c13Valu = 0xFF;
		++_progress;
	} else if (_progress == 1) {
		++_progress;
	} else if (_progress == 2) {
		if (_vm->_scd->makeAbsPtr(0x13FEC)[*data] != 0xFF)
			_vm->gfx()->setVar(11, 1);
		if (!_vm->gfx()->getVar(11)) {
			_progress = -1;
		} else {
			_vm->gfx()->setVar(3, 1);
			++_progress;
		}

	} else {
		uint8 f = _vm->_scd->makeAbsPtr(0x13FEC)[*data];
		if (f != 0xFF) {
			//_vm->gfx()->dataMode = 1;
			_vm->gfx()->runScript(_vm->_module->getPtr(0), f);
			//_vm->gfx()->dataMode = 0;
		}
		_progress = -1;
	}
	if (_progress == -1)
		++data;
}

void CmdQueue::m_14(const uint16 *&data) {
}

void CmdQueue::m_gfxReset(const uint16 *&data) {
	if (_progress == 0) {
		if (!_vm->gfx()->busy(2)) {
			_vm->gfx()->reset(GraphicsEngine::kResetSetDefaults);
		} else {
			_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
			++_progress;
		}
	} else {
		// original: is file done loading
		_progress = -1;
	}
}

void CmdQueue::m_waitFrames(const uint16 *&data) {
	if (_progress == 0) {
		_counter = 0;
		++_progress;
	}
	if (++_counter != *data)
		return;

	_counter = 0;
	_progress = -1;
	++data;
}
void CmdQueue::m_loadResource(const uint16 *&data) {
	if (_progress == 0) {
		_vm->sound()->cdaStop();
		++_progress;
	} else if (_progress == 1) {
		uint32 dest = *reinterpret_cast<const uint32*>(data);
		data += 2;
		uint16 index = *data++;
		++_progress;

		if (dest == 0x1A800) {
			assert(_vm->_script);
			delete[] _vm->_script->data;
			_vm->_script->data = _vm->_fio->fileData(index, &_vm->_script->dataSize);
			assert(_vm->_script->data);
			_vm->_script->curFileNo = index;
		} else if (dest == 0x28000) {
			delete _vm->_module;
			_vm->_module = _vm->_fio->loadModule(index);
			assert(_vm->_module);
		} else {
			error("%s(): Unexpected target address 0x%08x", __FUNCTION__, dest);
		}

	} else {
		// original: is file done loading
		_progress = -1;
	}
}

void CmdQueue::m_gfxStart(const uint16 *&data) {
	_vm->gfx()->runScript(_vm->_module->getPtr(0), *data++);
	_progress = -1;
}

void CmdQueue::m_fmMusicWait(const uint16 *&data) {
	if (_progress == 0) {
		_counter = 2700;
		++_progress;
	}
	if (--_counter == 0 || (_vm->sound()->_fmPlayingTracks.music == 0)) 
		_progress = -1;
}

void CmdQueue::m_20(const uint16 *&data) {
}

void CmdQueue::m_21(const uint16 *&data) {
}

void CmdQueue::m_22(const uint16 *&data) {
}

void CmdQueue::m_23(const uint16 *&data) {
}

void CmdQueue::m_24(const uint16 *&data) {
}

void CmdQueue::m_25(const uint16 *&data) {
}

void CmdQueue::m_26(const uint16 *&data) {
}

void CmdQueue::m_27(const uint16 *&data) {
}

void CmdQueue::m_28(const uint16 *&data) {
}

void CmdQueue::m_29(const uint16 *&data) {
}

void CmdQueue::m_30(const uint16 *&data) {
}

void CmdQueue::m_31(const uint16 *&data) {
}

void CmdQueue::m_32(const uint16 *&data) {
}

void CmdQueue::m_33(const uint16 *&data) {
}

void CmdQueue::m_34(const uint16 *&data) {
}

void CmdQueue::m_35(const uint16 *&data) {
}

ScriptEngine::ScriptEngine(SnatcherEngine *vm, CmdQueue *que, ResourcePointer *scd) : _vm(vm), _que(que), _var(nullptr), _curPos(0), _op(0), /*_v1(0), _v2(0),*/ _v3(0), _v5(0), _buf352(nullptr) {
	_var = new uint8[0x100]();
	_arrays[0] = _var;
	_arrays[1] = _var + 18;
	_arrays[2] = _arrays[1] + 64;
	_arrays[3] = _arrays[2] + 32;
	_arrays[4] = _arrays[3] + 38;
	_arrays[5] = _arrays[4] + 8;
	_buf352 = new uint8[352]();

	makeOpcodeTable(scd);
}

ScriptEngine::~ScriptEngine() {
	delete[] _var;
	delete[] _buf352;
}

void ScriptEngine::resetArrays() {
	static const uint8 arraySize[] = { 0x08, 0x1F, 0x0F, 0x12, 0x03, 0x10 };
	memset(_var, 0, 0x100);
	for (int i = 0; i < ARRAYSIZE(_arrays); ++i)
		_arrays[i][0] = arraySize[i];
}

void ScriptEngine::run(Script *script) {
	assert(script);

	if (script->newPos != -1) {
		script->curPos = script->newPos;
		script->newPos = -1;
	}
	_curPos = script->curPos;
	script->sp = script->bp;
	_script = script;

	runOpcode(_curPos);
}

void ScriptEngine::cleanupState() {

}

void ScriptEngine::runOpcode(int offset) {
	uint8 op = getOpcode(offset);
	if (_opcodes[op].proc->isValid())
		(*_opcodes[op].proc)();
	else
		error("%s(): Invalid opcode %d", __FUNCTION__, op);
}

#define S_FRAME(x) _script->sp += (x)
#define S_WRBYTE(x, y) _script->sp[x] = (uint8)(y)
#define S_INCBYTE(x) _script->sp[x]++
#define S_DECBYTE(x) _script->sp[x]--
#define S_ADDBYTE(x, y) _script->sp[x] += (uint8)(y)
#define S_SUBBYTE(x, y) _script->sp[x] -= (uint8)(y)
#define S_WRWORD(x, y) *(uint16*)(_script->sp + x) = (uint16)(y)
#define S_RDBYTE(x) _script->sp[x]
#define S_RDWORD(x) *(uint16*)(_script->sp + x)
#define S_PUSH(x) S_FRAME(-2); S_WRWORD(0, x)
#define S_POP() S_RDWORD(0); S_FRAME(2);
#define ARR_SIZE(x)	_arrays[x][0]
#define ARR_POS(x)	_arrays[x][1]
#define ARR_WRITE(a, p, v) WRITE_BE_UINT16(_arrays[a] + 2 + 2 * p, v);
#define ARR_READ(a, p) READ_BE_UINT16(_arrays[a] + 2 + 2 * p);

void ScriptEngine::getOpcodeProps(int offset, int &len, int &f1, int &f2) {
	uint8 op = getOpcode(offset);
	len = _opcodes[op].len;
	f1 = _opcodes[op].flags >> 4;
	f2 = _opcodes[op].flags & 0x0F;
}

void ScriptEngine::runOpcode(int &offset, int len, int &result) {
	if (len == 4) {
		runOpcode(offset);
		stackOps_unkParseTo(offset);
	} else if (len == 1) {
		getFlag(READ_BE_UINT16(_script->data + offset), result);
		offset += 2;
	} else if (len == 3 || len == 5) {
		result = READ_BE_UINT16(_script->data + offset);
		offset += 2;
	} else {
		error("%s(): Invalid opcode length %d", __FUNCTION__, len);
	}
}

static const uint16 maskTable[] = {
	0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
	0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};

void ScriptEngine::getFlag(int pos, int &result) {
	const uint8 *in = &_buf352[pos >> 7];
	result = (in[0] << 24 | in[1] << 16 | in[2] << 8);
	uint8 v2 = pos & 0x0F;
	uint8 v1 = v2 + 1 + ((pos >> 4) & 7);
	result = (((uint32)result << v1) | ((uint32)result >> (32 - v1))) & maskTable[v2];
}

void ScriptEngine::setFlag(int pos, int val) {
	uint8 v2 = pos & 0x0F;
	uint8 v1 = v2 + 1 + ((pos >> 4) & 7);

	uint16 msk = maskTable[v1];
	val &= msk;
	msk = ~msk;

	uint8 *in = &_buf352[pos >> 7];
	int cur = (in[0] << 24 | in[1] << 16 | in[2] << 8);

	msk = (((uint32)msk << (32 - v1)) | ((uint32)msk >> v1));
	val = (((uint32)val << (32 - v1)) | ((uint32)val >> v1));

	val |= (cur & msk);
	*in++ = (val >> 24) & 0xFF;
	*in++ = (val >> 16) & 0xFF;
	*in++ = (val >> 8) & 0xFF;
}

uint8 ScriptEngine::countFunctionOps(int offset) {
	S_FRAME(-2);
	S_WRBYTE(0, 0);

	int r1 = 0;
	int r2 = 0;
	int r3 = 0;

	for (bool lp = true; lp; ) {
		S_INCBYTE(0);
		if (!isExecFuncOpcode(offset))
			break;
		if (stackOps_checkFor52(offset, r1))
			S_ADDBYTE(0, r1 - 1);
		getOpcodeProps(offset, r1, r2, r3);
		if (r2 == 7)
			break;
		if (opcodeHasFlag8(offset)) {
			offset = READ_BE_UINT16(_script->data + offset + 1);
		} else {
			++offset;
			stackOps_unkParseTo(offset);
		}
	}

	uint8 res = S_RDBYTE(0);
	S_FRAME(2);
	return res;
}

void ScriptEngine::seekToFunctionOp(int &offset, int arg) {
	S_FRAME(-2);
	S_WRBYTE(0, arg);

	int r1 = 0;
	int r2 = 0;
	int r3 = 0;

	for (bool lp = true; lp; ) {
		if (!isExecFuncOpcode(offset))
			break;
		if (stackOps_checkFor52(offset, r1))
			S_SUBBYTE(0, r1 - 1);
		S_SUBBYTE(0, 1);
		if ((int8)S_RDBYTE(0) <= 0) {
			skipOnFlag8(offset);
			break;
		}
		if (opcodeHasFlag8(offset)) {
			offset = READ_BE_UINT16(_script->data + offset + 1);
		} else {
			getOpcodeProps(offset, r1, r2, r3);
			if (r1 == 4)
				stackOps_unkParseTo(++offset);
			else
				offset += 3;
		}
	}

	S_FRAME(2);
}

bool ScriptEngine::isExecFuncOpcode(int offset) {
	uint8 op = getOpcode(offset);
	return (op >= 30 && op <= 39);
}

bool ScriptEngine::stackOps_checkFor52(int offset, int &result) {
	S_PUSH(offset);

	int len = 0;
	int f1 = 0;
	int f2 = 0;

	getOpcodeProps(offset, len, f1, f2);
	if (len == 4) {
		skipOnFlag8(offset);
		uint8 op = getOpcode(offset);
		if (op == 52 || op == 53) {
			result = READ_BE_UINT16(_script->data + offset + 1);
			return true;
		}
	}

	S_FRAME(2);
	return false;
}

bool ScriptEngine::opcodeHasFlag8(int offset) {
	int len = 0;
	int f1 = 0;
	int f2 = 0;
	getOpcodeProps(offset, len, f1, f2);
	return ((f1 | f2) & 8);
}

bool ScriptEngine::OPC_4_ss() {
	if (ARR_POS(1) || _script->newPos != -1)
		return false;
	if (_script->scriptStateByteUnk) {
		int r1 = 0;
		if (!getArrayLastEntry(0, r1))
			return true;
		return (_script->unkislp > r1);
	}
	return (_script->unkislp == ARR_POS(2));
}

void ScriptEngine::skipOnFlag8(int &offset) {
	offset += (opcodeHasFlag8(offset) ? 3 : 1);
}

void ScriptEngine::stackOps_unkParseTo(int &offset) {
	for (bool lp = true; lp; ) {
		if (opcodeHasFlag8(offset)) {
			offset = READ_BE_UINT16(_script->data + offset + 1);
			continue;
		}
		S_FRAME(-4);
		lp = false;

		int len = 0;
		int f1 = 0;
		int f2 = 0;
		getOpcodeProps(offset, len, f1, f2);
		S_WRBYTE(0, len);
		S_WRBYTE(1, f1);
		S_WRBYTE(2, f2);
		++offset;

		if (len == 4)
			stackOps_unkParseTo(offset);
		else if (len != 0 && len != 7)
			offset += 2;

		uint8 b = S_RDBYTE(1);
		if (b == 4)
			stackOps_unkParseTo(offset);
		else if (b != 0 && b != 7)
			offset += 2;

		b = S_RDBYTE(2);
		if (b == 4)
			stackOps_unkParseTo(offset);
		else if (b != 0 && b != 7)
			offset += 2;
	}

	S_FRAME(4);
}

bool ScriptEngine::getArrayLastEntry(int arrNo, int &result) {
	assert(arrNo < ARRAYSIZE(_arrays));
	return getArrayEntry(arrNo, ARR_POS(arrNo) - 1, result);
}

bool ScriptEngine::popArrayLastEntry(int arrNo, int &result) {
	assert(arrNo < ARRAYSIZE(_arrays));
	if (!ARR_POS(arrNo))
		return false;
	getArrayLastEntry(arrNo, result);
	--ARR_POS(arrNo);
	return true;
}

bool ScriptEngine::getArrayEntry(int arrNo, int pos, int &result) {
	assert(arrNo < ARRAYSIZE(_arrays));
	if (ARR_POS(arrNo) <= pos)
		return false;
	result = ARR_READ(arrNo, pos);
	return true;
}

bool ScriptEngine::setArrayLastEntry(int arrNo, int val) {
	assert(arrNo < ARRAYSIZE(_arrays));
	if (ARR_SIZE(arrNo) <= ARR_POS(arrNo))
		return false;
	int pos = ARR_POS(arrNo)++;
	if (ARR_POS(arrNo) <= pos)
		return false;
	ARR_WRITE(arrNo, pos, val);
	return true;
}

uint8 ScriptEngine::getOpcode(int offset) {
	if ((uint)offset >= _script->dataSize)
		error("%s(): Offset %d is out of bounds", __FUNCTION__, offset);
	uint8 op = _script->data[offset];
	if (op >= _opcodes.size())
		error("%s(): Opcode %d is out of bounds", __FUNCTION__, op);
	return op;
}

void ScriptEngine::makeOpcodeTable(ResourcePointer *scd) {
#ifndef SNATCHER_SCRIPT_DEBUG
	#define OP(x) &ScriptEngine::o_##x
	typedef void (ScriptEngine::*SFunc)();
#else
	#define OP(x) {&ScriptEngine::o_##x, #x}
	struct SFunc {
		ScriptEngineProc::ScrFunc func;
		const char *desc;
	};
#endif
	static const SFunc funcTbl[] = {
		OP(animOps),
		OP(01),
		OP(01),
		OP(01),
		OP(mmSequence),
		OP(05),
		OP(06),
		OP(06),
		OP(06),
		OP(06),
		OP(fmMusicStart),
		OP(11),
		OP(12),
		OP(13),
		OP(14),
		OP(15),
		OP(16),
		OP(16),
		OP(16),
		OP(19),
		OP(20),
		OP(21),
		OP(22),
		OP(23),
		OP(24),
		OP(24),
		OP(26),
		OP(26),
		OP(28),
		OP(28),
		OP(executeFunction),
		OP(executeFunction),
		OP(executeFunction),
		OP(executeFunction),
		OP(executeFunction),
		OP(executeFunction),
		OP(executeFunction),
		OP(executeFunction),
		OP(executeFunction),
		OP(executeFunction),
		OP(40),
		OP(40),
		OP(42),
		OP(break),
		OP(break),
		OP(nop),
		OP(46),
		OP(46),
		OP(startFunction),
		OP(49),
		OP(49),
		OP(49),
		OP(52),
		OP(52),
		OP(54),
		OP(54),
		OP(56),
		OP(57),
		OP(loadScript),
		OP(59),
		OP(59),
		OP(loadModuleAndStartGfx),
		OP(62),
		OP(63),
		OP(64),
		OP(subOps),
		OP(subOps),
		OP(gfxOps),
		OP(gfxOps),
		OP(animWait),
		OP(pcmSoundWait),
		OP(fmMusicWait),
		OP(72),
		OP(73),
		OP(waitFrames)
	};
#undef OP

	const uint8 *in = scd->makeAbsPtr(0x16CDC)();

	for (uint i = 0; i < ARRAYSIZE(funcTbl); ++i) {
#ifndef SNATCHER_SCRIPT_DEBUG
		_opcodes.push_back(ScriptEngineOpcode(in[0], in[1], new ScriptEngineProc(this, funcTbl[i])));
#else
		_opcodes.push_back(ScriptEngineOpcode(in[0], in[1], new ScriptEngineProc(this, funcTbl[i].func, funcTbl[i].desc, in[0], in[1])));
#endif
		in += 4;
	}
}

void ScriptEngine::o_nop() {
}

void ScriptEngine::o_animOps() {
	if (!OPC_4_ss())
		return;
	_que->writeUInt16(0x09);
	_que->writeUInt16(READ_BE_UINT16(_script->data + _curPos + 4));
	_que->writeUInt16(READ_BE_UINT16(_script->data + _curPos + 2));
}

void ScriptEngine::o_01() {

}

void ScriptEngine::o_mmSequence() {
	if (!OPC_4_ss())
		return;
	_que->writeUInt16(0x0D);
	_que->writeUInt16(READ_BE_UINT16(_script->data + _curPos + 1));
}

void ScriptEngine::o_05() {
}

void ScriptEngine::o_06() {
}

void ScriptEngine::o_fmMusicStart() {
	if (!OPC_4_ss())
		return;
	_que->writeUInt16(0x07);
	_que->writeUInt16(READ_BE_UINT16(_script->data + _curPos + 1));
}

void ScriptEngine::o_11() {
	
}

void ScriptEngine::o_12() {
	int cp = _script->curPos;
	setArrayLastEntry(5, cp);
	_curPos = _script->curPos = READ_BE_UINT16(_script->data + _curPos + 1);
	runOpcode(_curPos);
	popArrayLastEntry(5, cp);
	_script->curPos = cp;
}

void ScriptEngine::o_13() {
}

void ScriptEngine::o_14() {
}

void ScriptEngine::o_15() {
}

void ScriptEngine::o_16() {
}

void ScriptEngine::o_19() {
}

void ScriptEngine::o_20() {
}

void ScriptEngine::o_21() {

}

void ScriptEngine::o_22() {
	int len = 0;
	int f1 = 0;
	int f2 = 0;

	getOpcodeProps(_curPos, len, f1, f2);
	++_curPos;
	int r1 = 0;
	runOpcode(_curPos, len, r1);
	int r2 = 0;
	runOpcode(_curPos, f1, r2);
	_v5 = (r1 > r2) ? -1 : 0;
}

void ScriptEngine::o_23() {
}

void ScriptEngine::o_24() {
}

void ScriptEngine::o_26() {
	int len = 0;
	int f1 = 0;
	int f2 = 0;

	getOpcodeProps(_curPos, len, f1, f2);

	int pos = ++_curPos;
	runOpcode(pos, len, _v5);
	if (!_v5)
		return;

	_curPos = pos;
	runOpcode(pos);
}

void ScriptEngine::o_28() {
	if (!OPC_4_ss())
		return;

	int len = 0;
	int f1 = 0;
	int f2 = 0;

	getOpcodeProps(_curPos, len, f1, f2);
	S_PUSH(READ_BE_UINT16(_script->data + _curPos + 1));
	_curPos += 3;
	int res = 0;
	runOpcode(_curPos, f1, res);
	int pos = S_POP();
	setFlag(pos, res);
}


void ScriptEngine::o_executeFunction() {
	S_FRAME(-4);

	S_WRBYTE(0, countFunctionOps(_curPos));
	S_WRBYTE(1, 1);
	S_WRWORD(2, _curPos);
	
	for (bool lp = true; lp; ) {
		uint8 cnt = S_RDBYTE(1);
		int offs = S_RDWORD(2);
		seekToFunctionOp(offs, cnt);
		_curPos = offs;
		runOpcode(_curPos);

		if (S_RDBYTE(0) == S_RDBYTE(1))
			lp = false;
		else
			S_INCBYTE(1);
	}

	S_FRAME(4);
}

void ScriptEngine::o_40() {

}

void ScriptEngine::o_42() {
}

void ScriptEngine::o_break() {
	if (!OPC_4_ss())
		return;

	int len = 0;
	int f1 = 0;
	int f2 = 0;

	getOpcodeProps(_curPos, len, f1, f2);
	int pos = _curPos + 1;
	if (len != 5) {
		int num = READ_BE_UINT16(_script->data + pos);
		pos += 2;
		if (_script->curFileNo != num) {
			_script->curFileNo = num;
			_que->writeUInt16(0x11);
			_que->writeUInt32(0x1A800);
			_que->writeUInt16(num);
		}
	}
	_script->newPos = READ_BE_UINT16(_script->data + pos);
}

void ScriptEngine::o_46() {
	int len = 0;
	int f1 = 0;
	int f2 = 0;
	int pos = _curPos;
	getOpcodeProps(_curPos, len, f1, f2);
	runOpcode(_curPos, len, _v5);
	_v5 = _v5 ? 0 : -1;
}

void ScriptEngine::o_startFunction() {
	runOpcode(++_curPos);
}

void ScriptEngine::o_49() {
}

void ScriptEngine::o_52() {
}

void ScriptEngine::o_54() {
}

void ScriptEngine::o_56() {
	if (_script->newPos != -1)
		return;

	int pos = _curPos;

	if (ARR_POS(2) == _script->unkislp) {
		int val = READ_BE_UINT16(_script->data + pos + 1);

		if (ARR_SIZE(1) <= ARR_POS(1)) {
			//_v1 = 0;
			debug("Array 1 is full");
		} else {
			pos = ARR_POS(1)++;
			if (ARR_POS(1) <= pos) {
				//_v0 = 0;
				debug("Array 1 is full");
			} else {
				ARR_WRITE(1, pos, val);
				//_v1 = -1;
			}
		}

	} else {
		getArrayEntry(2, _script->unkislp, _v3);
		if (READ_BE_UINT16(_script->data + pos + 1) != _v3)
			return;

		++_script->unkislp;
		_curPos += 3;

		runOpcode(_curPos);

		if (ARR_POS(2) == _script->unkislp) {
			if (ARR_POS(1))
				_script->scriptStateByteUnk = 0xFF;
		}
		--_script->unkislp;
	}
}

void ScriptEngine::o_57() {
}

void ScriptEngine::o_loadScript() {
	int res = 0;
	popArrayLastEntry(3, res);
	_script->newPos = res;
	popArrayLastEntry(3, res);
	if (_script->curFileNo == res)
		return;
	_que->writeUInt16(0x11);
	_que->writeUInt32(0x1A800);
	_que->writeUInt16(res);
}

void ScriptEngine::o_59() {
}

void ScriptEngine::o_loadModuleAndStartGfx() {
	if (!OPC_4_ss())
		return;
	int16 num = READ_BE_INT16(_script->data + _curPos + 1);

	if (_script->curGfxScript == num)
		return;

	_que->writeUInt16(0x0F);

	if ((_script->curGfxScript >> 8) != (num >> 8)) {
		_que->writeUInt16(0x11);
		_que->writeUInt32(0x28000);
		_que->writeUInt16(num >> 8);
	}

	_que->writeUInt16(0x12);
	_que->writeUInt16(num & 0xFF);

	_script->curGfxScript = num;
}

void ScriptEngine::o_62() {
}

void ScriptEngine::o_63() {
}

void ScriptEngine::o_64() {
}

void ScriptEngine::o_subOps() {
	if (!OPC_4_ss())
		return;

	int len = 0;
	int f1 = 0;
	int f2 = 0;

	getOpcodeProps(_curPos, len, f1, f2);
	int op = READ_BE_UINT16(_script->data + _curPos + (len == 4 ? 2 : 1));
	uint16 addr = 0;
	uint16 val = 0;

	switch (op) {
	case 0:
		_que->writeUInt16(0x04);
		break;
	case 1:
		_que->writeUInt16(0x1E);
		_que->writeUInt16(READ_BE_UINT16(_script->data + _curPos + 4));
		//saveState_sub_14F12();
		break;
	case 2:
		_que->writeUInt16(0x21);
		break;
	case 3:
		_que->writeUInt16(0x1F);
		break;
	case 4:
		//_scr_wd_00 = READ_BE_UINT16(_script->data + _curPos + 4);
		//_scr_wd_01 = _scr_wd_02 = _scr_wd_03 = _scr_wd_04 = 0;
		//_scr_bt_00 = true;
		_que->writeUInt16(0x18);
		break;
	case 5:
		addr = READ_BE_UINT16(_script->data + _curPos + 4);
		_v5 = 0;//READ_BE_UINT16(addr);
		debug ("READ VAR: Addr 0x%04x", addr);
		break;
	case 6:
		addr = READ_BE_UINT16(_script->data + _curPos + 5);
		val = READ_BE_UINT16(_script->data + _curPos + 7);
		if (addr == 0xFFFF) // 79F0
			_vm->sound()->setUnkCond(val);
		else
			debug ("WRITE VAR: Addr 0x%04x, Val 0x%04x", addr, val);
		// WRITE_BE_UINT16(addr, val);
		break;
	case 7:
		_que->writeUInt16(0x1D);
		_que->writeUInt16(READ_BE_UINT16(_script->data + _curPos + 4) ? 1 : 0);
		break;
	case 8:
		if (READ_BE_UINT16(_script->data + _curPos + 4) == 0) {
			_que->writeUInt16(0x09);
			_que->writeUInt16(0x02);
			_que->writeUInt16(0x01);
		} else {
			_que->writeUInt16(0x15);
			_que->writeUInt16(READ_BE_UINT16(_script->data + _curPos + 8));
			_que->writeUInt16(READ_BE_UINT16(_script->data + _curPos + 10));
		}
		break;
	case 9:
		/*_vm->_gameState.chapter = */READ_BE_UINT16(_script->data + _curPos + 4) + 2;
		break;
	case 10:
		/*_doTransition? = */READ_BE_UINT16(_script->data + _curPos + 4);
		/*if (_doTransition == 1)
			_transDW1 = _transDW2 = 0;*/
		break;
	case 11:
		/*if (READ_BE_UINT16(_script->data + _curPos + 4))
			restoreGfxAndSoundAfterLoad();
		else
			animSpecOther();
			*/
		break;
	case 12:
		_buf352[53] &= 0xF8;
		memset(_buf352 + 54, 0, 298);
		break;
	case 13:
		_que->writeUInt16(0x1B);
		_que->writeUInt16(READ_BE_UINT16(_script->data + _curPos + 5));
		_que->writeUInt16(READ_BE_UINT16(_script->data + _curPos + 8));
		_que->writeUInt16(READ_BE_UINT16(_script->data + _curPos + 10));
		break;
	default:
		break;
	}
}

void ScriptEngine::o_gfxOps() {
	if (!OPC_4_ss())
		return;

	int len = 0;
	int f1 = 0;
	int f2 = 0;

	getOpcodeProps(_curPos, len, f1, f2);
	int pos = _curPos + 1;

	_que->writeUInt16(0x08);
	if (len == 4) {
		_que->writeUInt16(READ_BE_UINT16(_script->data + pos + 1));
		pos += 3;
	} else {
		_que->writeUInt16(0);
	}
	_que->writeUInt16(READ_BE_UINT16(_script->data + pos));
}

void ScriptEngine::o_animWait() {
	if (!OPC_4_ss())
		return;
	_que->writeUInt16(0x09);
	_que->writeUInt16(0x03);
	_que->writeUInt16(READ_BE_UINT16(_script->data + _curPos + 1));
}

void ScriptEngine::o_pcmSoundWait() {
	if (OPC_4_ss())
		_que->writeUInt16(0x0A);
}

void ScriptEngine::o_fmMusicWait() {
	if (OPC_4_ss())
		_que->writeUInt16(0x13);
}

void ScriptEngine::o_72() {
}

void ScriptEngine::o_73() {
}

void ScriptEngine::o_waitFrames() {
	if (!OPC_4_ss())
		return;
	_que->writeUInt16(0x10);
	_que->writeUInt16(READ_BE_UINT16(_script->data + _curPos + 1));
}

#undef S_FRAME
#undef S_WRBYTE
#undef S_WRWORD
#undef S_RDBYTE
#undef S_RDWORD
#undef S_INCBYTE
#undef S_DECBYTE
#undef S_ADDBYTE
#undef S_SUBBYTE
#undef S_PUSH
#undef S_POP
#undef ARR_SIZE
#undef ARR_POS
#undef ARR_WRITE
#undef ARR_READ

} // End of namespace Snatcher
