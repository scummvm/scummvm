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


#include "snatcher/action.h"
#include "snatcher/graphics.h"
#include "snatcher/memory.h"
#include "snatcher/resource.h"
#include "snatcher/saveload.h"
#include "snatcher/script.h"
#include "snatcher/snatcher.h"
#include "snatcher/sound.h"
#include "snatcher/ui.h"
#include "snatcher/util.h"
#include "common/savefile.h"
#include "common/stream.h"

namespace Snatcher {

CmdQueue::CmdQueue(SnatcherEngine *vm) : _vm(vm), _data(nullptr), _readPos(0), _writePos(nullptr), _enable(false), _progress(-1), _currentOpcode(0),  _counter(0), _state(nullptr), _opcodes() {
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

void CmdQueue::run(GameState &state) {
	if (!_enable)
		return;

	_state = &state;

	do {
		if (_progress == -1) {
			_currentOpcode = *_readPos++;
			_progress = 0;
		}

		if (_currentOpcode < _opcodes.size() && _opcodes[_currentOpcode]->isValid())
			(*_opcodes[_currentOpcode])(_readPos);
		else
			error("%s(): Invalid opcode %d", __FUNCTION__, _currentOpcode);
	} while (_currentOpcode != 0 && _progress == -1);
}

void CmdQueue::loadState(Common::SeekableReadStream *in) {
	if (in->readUint32BE() != MKTAG('S', 'N', 'A', 'T'))
		error("%s(): Save file invalid or corrupt", __FUNCTION__);
}

void CmdQueue::saveState(Common::SeekableWriteStream *out) {
	out->writeUint32BE(MKTAG('S', 'N', 'A', 'T'));

}

void CmdQueue::makeFunctions() {
#define OP(x) &CmdQueue::m_##x
	typedef void (CmdQueue::*SFunc)(const uint16*&);
	static const SFunc funcTbl[] = {
		OP(end),
		OP(initAnimations),
		OP(animPauseAll),
		OP(drawCommands),
		OP(fmSfxBlock),
		OP(printText),
		OP(06),
		OP(fmMusicStart),
		OP(displayDialog),
		OP(animOps),
		OP(pcmWait),
		OP(cdaWait),
		OP(pcmSound),
		OP(chatWithPortraitAnim),
		OP(cdaPlay),
		OP(gfxReset),
		OP(waitFrames),
		OP(loadResource),
		OP(gfxStart),
		OP(fmMusicWait),
		OP(nop),
		OP(invItemCloseUp),
		OP(resetTextFields),
		OP(unused),
		OP(shooterSequenceRun),
		OP(fmSfxWait),
		OP(nop2),
		OP(cdaSync),
		OP(fmSoundEffect),
		OP(pcmBlock),
		OP(saveGame),
		OP(shooterSequenceResetScore),
		OP(gfxPostLoadProcess),
		OP(waitForStartButton),
		OP(palOps),
		OP(clearTextInputLine)
	};
#undef OP

	for (uint i = 0; i < ARRAYSIZE(funcTbl); ++i)
		_opcodes.push_back(new CmdQueOpcode(this, funcTbl[i]));
}

void CmdQueue::m_end(const uint16 *&data) {
	reset();
}

void CmdQueue::m_initAnimations(const uint16 *&data) {
	uint32 addr = *reinterpret_cast<const uint32*>(data);
	data += 2;
	ResourcePointer in = (addr >= 0x28000 ? _vm->_module->getPtr(addr) : _vm->_scd->makePtr(addr));
	uint16 len = 0;
	const uint8 *s = in();
	int dbgTimeout = 0;

	while (*s != 0xFF && dbgTimeout < 100) {
		s += 12;
		len += 12;
		++dbgTimeout;
	}

	if (dbgTimeout >= 100) {
		error("%s(): Error reading animation data", __FUNCTION__);
		return;
	}

	_vm->gfx()->initAnimations(in, len, false);
	_progress = -1;
}

void CmdQueue::m_animPauseAll(const uint16 *&data) {
	if (_progress == 0) {
		_vm->gfx()->reset(GraphicsEngine::kResetPalEvents);
		for (int i = 0; i < 64; ++i)
			_vm->gfx()->setAnimParameter(i, GraphicsEngine::kAnimParaControlFlags, GraphicsEngine::kAnimPause);
		_vm->gfx()->enqueuePaletteEvent(_vm->_scd->makePtr(0x100D8));
		_counter = 48;
		++_progress;
	} else if (--_counter == 0) {
		_progress = -1;
	}
}

void CmdQueue::m_drawCommands(const uint16 *&data) {
	uint32 addr = *reinterpret_cast<const uint32*>(data);
	data += 2;
	_vm->gfx()->enqueueDrawCommands(addr >= 0x28000 ? _vm->_module->getPtr(addr) : _vm->_scd->makePtr(addr));
	_progress = -1;
}

void CmdQueue::m_fmSfxBlock(const uint16 *&data) {
	_vm->sound()->fmBlock(true);
	_progress = -1;
}

void CmdQueue::m_printText(const uint16 *&data) {
	if (_vm->gfx()->isTextInQueue())
		return;
	uint32 addr = *reinterpret_cast<const uint32*>(data);
	data += 2;
	_vm->gfx()->printText(addr >= 0x28000 ? _vm->_module->getPtr(addr)() : _vm->_scd->makePtr(addr)());
	_progress = -1;
}

void CmdQueue::m_06(const uint16 *&data) {
}

void CmdQueue::m_fmMusicStart(const uint16 *&data) {
	uint8 cmd = *data++;
	_vm->sound()->fmSendCommand(cmd, 1, 1);
	_progress = -1;
}

void CmdQueue::m_displayDialog(const uint16 *&data) {
	if (_vm->_ui->displayDialog(data[0], data[1], _vm->input().singleFrameControllerFlagsRemapped)) {
		_progress = -1;
		data += 2;
	}
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
	if (--_counter == 0 || !(_vm->sound()->pcmGetStatus().statusBits & 0x0F))
		_progress = -1;
}

void CmdQueue::m_cdaWait(const uint16 *&data) {
	if (!_vm->sound()->cdaIsPlaying())
		_progress = -1;
}

void CmdQueue::m_pcmSound(const uint16 *&data) {
	if (!_vm->sound()->pcmGetStatus().blocked && !(_vm->sound()->pcmGetStatus().statusBits & 0x07))
		_vm->sound()->pcmSendCommand(*data++, -1);
	_progress = -1;
}

void CmdQueue::m_chatWithPortraitAnim(const uint16 *&data) {
	if (_progress == 0) {
		_vm->sound()->pcmInitSound(*data);
		//_unused13 = 0xFF;
		++_progress;
	} else if (_progress == 1) {
		++_progress;
	} else if (_progress == 2) {
		if (_vm->_scd->makePtr(0x13FEC)[*data] != 0xFF)
			_vm->gfx()->setVar(11, 1);
		if (!_vm->gfx()->getVar(11)) {
			_progress = -1;
		} else {
			_vm->gfx()->setVar(3, 1);
			++_progress;
		}

	} else {
		uint8 f = _vm->_scd->makePtr(0x13FEC)[*data];
		if (f != 0xFF) {
			//_vm->gfx()->dataMode = 1;
			_vm->gfx()->runScript(_vm->_module->getGfxData(), f);
			//_vm->gfx()->dataMode = 0;
		}
		_progress = -1;
	}
	if (_progress == -1)
		++data;
}

void CmdQueue::m_cdaPlay(const uint16 *&data) {
	if (_progress == 0) {
		_vm->sound()->cdaPlay(*data++);
		++_progress;
	} else if (_progress == 1) {
		if (_vm->sound()->cdaIsPlaying())
			++_progress;
	} else {
		_progress = -1;
	}
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
			delete[] _state->script.data;
			_state->script.data = _vm->_fio->fileData(index, &_state->script.dataSize);
			assert(_state->script.data);
			_state->script.curFileNo = index;
			_vm->_ui->setScriptTextResource(_state->script.getTextResource());
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
	_vm->gfx()->runScript(_vm->_module->getGfxData(), *data++);
	_progress = -1;
}

void CmdQueue::m_fmMusicWait(const uint16 *&data) {
	if (_progress == 0) {
		_counter = 2700;
		++_progress;
	}
	if (--_counter == 0 || (_vm->sound()->fmGetStatus().music == 0))
		_progress = -1;
}

void CmdQueue::m_nop(const uint16 *&data) {
	_progress = -1;
}

void CmdQueue::m_invItemCloseUp(const uint16 *&data) {
	if (_vm->gfx()->getVar(8))
		return;

	_vm->gfx()->runScript(_vm->_scd->makePtr(0x146CE), *data++);
	ResourcePointer coords = _vm->_scd->makePtr(0x144C6) + ((*data++) << 2);
	_vm->gfx()->setAnimParameter(1, GraphicsEngine::kAnimParaPosX, coords.readIncrSINT16());
	_vm->gfx()->setAnimParameter(1, GraphicsEngine::kAnimParaPosY, coords.readIncrSINT16());

	_progress = -1;
}

void CmdQueue::m_resetTextFields(const uint16 *&data) {
	_vm->gfx()->resetTextFields();
	_progress = -1;
}

void CmdQueue::m_unused(const uint16 *&data) {
	//_unused13 = 0xFF;
}

void CmdQueue::m_shooterSequenceRun(const uint16 *&data) {
	if (!_vm->_aseq->run(_state->conf.useLightGun))
		_progress = -1;
}

void CmdQueue::m_fmSfxWait(const uint16 *&data) {
	if (_progress == 0) {
		_counter = 600;
		++_progress;
	}
	if (--_counter == 0 || (_vm->sound()->fmGetStatus().sfx == 0))
		_progress = -1;
}

void CmdQueue::m_nop2(const uint16 *&data) {
	++data;
	_progress = -1;
}

void CmdQueue::m_cdaSync(const uint16 *&data) {
	uint32 ts = (data[0] & 0xFF) << 24 | (data[1] & 0xFF) << 16 | (data[2] & 0xFF) << 8;
	if (ts > _vm->sound()->cdaGetTime())
		return;
	data += 3;
	_progress = -1;
}

void CmdQueue::m_fmSoundEffect(const uint16 *&data) {
	uint8 cmd = *data++;
	_vm->sound()->fmSendCommand(cmd, 0, 2);
	_progress = -1;
}

void CmdQueue::m_pcmBlock(const uint16 *&data) {
	_vm->sound()->pcmBlock(*data++);
	_progress = -1;
}

void CmdQueue::m_saveGame(const uint16 *&data) {
	int slot = *data++;
	Common::String desc(Common::String::format("SNATCHER_%02d", slot));
	_vm->saveGameState(slot, desc);
	_progress = -1;
}

void CmdQueue::m_shooterSequenceResetScore(const uint16 *&data) {
	if (!_vm->_aseq->resetScore())
		_progress = -1;
}

void CmdQueue::m_gfxPostLoadProcess(const uint16 *&data) {
	_vm->gfx()->runScript(_vm->_module->getGfxData(), *data++);
	_vm->gfx()->updateAnimations();
	_vm->gfx()->postLoadProcess();
	_progress = -1;
}

void CmdQueue::m_waitForStartButton(const uint16 *&data) {
	if (_vm->input().singleFrameControllerFlags & 0x80)
		_progress = -1;
}

void CmdQueue::m_palOps(const uint16 *&data) {
	uint32 addr = *reinterpret_cast<const uint32*>(data);
	_vm->gfx()->enqueuePaletteEvent(addr >= 0x28000 ? _vm->_module->getPtr(addr) : _vm->_scd->makePtr(addr));
	data += 2;
	_progress = -1;
}

void CmdQueue::m_clearTextInputLine(const uint16 *&data) {
	_vm->gfx()->clearTextInputLine();
	_progress = -1;
}

ScriptEngine::ScriptEngine(CmdQueue *que, UI *ui, ActionSequenceHandler *aseq, MemAccessHandler *mem, ResourcePointer *scd) : _que(que), _ui(ui), _aseq(aseq), _mem(mem), _arrayData(nullptr), _pos1(0), _pos2(0), _op(0), /*_v1(0), _v2(0), _v3(0),*/ _result(0), _flagsTable(nullptr) {
	_flagsTable = new uint8[352]();
	_arrayData = new uint8[256]();
	makeOpcodeTable(scd);
	resetArrays();
	_ui->setScriptVerbsArray(&_arrays[1]);
	_ui->setScriptSentenceArray(&_arrays[2]);
}

ScriptEngine::~ScriptEngine() {
	delete[] _arrayData;
	delete[] _flagsTable;
}

void ScriptEngine::resetArrays() {
	static const uint8 arraySizes[] = { 0x08, 0x1F, 0x0F, 0x12, 0x03, 0x10 };
	memset(_arrayData, 0, 256);

	uint8 *pos = _arrayData;
	for (int i = 0; i < ARRAYSIZE(_arrays); ++i) {
		_arrays[i] = ScriptArray(pos , arraySizes[i]);
		pos += ((arraySizes[i] + 1) << 1);
	}
}

void ScriptEngine::run(Script &script) {
	_script = &script;
	_pos1 = script.curPos;
	script.sentenceDone = 0;

	runOpcode();
}

#define ARR_SIZE(x) _arrays[x].size()
#define ARR_POS(x) _arrays[x].pos()
#define ARR_READ(x, y) _arrays[x].read(y)
#define ARR_WRITE(x, y, z) _arrays[x].write(y, z)

bool ScriptEngine::postProcess(Script &script) {
	if (script.newPos != 0xFFFF) {
		if (ARR_SIZE(4) <= ARR_POS(4)) {
			for (int i = 0; i < ARR_POS(4) - 1; ++i)
				ARR_WRITE(4, i, ARR_READ(4, i + 1));
			--ARR_POS(4);
		}
		setArrayLastEntry(4, script.curPos);
		script.curPos = script.newPos;
		script.newPos = 0xFFFF;
		_ui->resetVerbSelection();
	}

	if (ARR_POS(1) != 0)
		return true;

	uint16 r0 = 0;
	getArrayLastEntry(0, r0);
	if (r0 < ARR_POS(2))
		ARR_POS(2) = r0;
	ARR_POS(1) = 0;

	return false;
}

void ScriptEngine::processInput() {
	uint16 val = 0;
	if ((int8)_ui->getSelectedVerb() < 0) {
		getArrayLastEntry(0, val);
		if (ARR_POS(2) != val) {
			if (ARR_POS(2))
				--ARR_POS(2);
		}
	} else {
		getArrayEntry(1, _ui->getSelectedVerb() - 1, val);
		setArrayLastEntry(2, val);
	}
	ARR_POS(1) = 0;
}

void ScriptEngine::loadState(Common::SeekableReadStream *in, Script &script, bool onlyTempData) {
	if (in->readUint32BE() != MKTAG('S', 'N', 'A', 'T'))
		error("%s(): Save file invalid or corrupt", __FUNCTION__);

	if (!onlyTempData) {
		uint16 *d = reinterpret_cast<uint16*>(_arrayData);
		for (int i = 0; i < 128; ++i)
			d[i] = in->readUint16BE();
		in->read(_flagsTable, 352);

		script.sentenceDone = in->readByte();
		script.sentencePos = in->readByte();
	}

	script.curFileNo = in->readByte();
	script.curGfxScript = in->readSint16BE();

	if (!onlyTempData) {
		script.curPos = in->readUint16BE();
		ARR_POS(0) = 0;
		ARR_POS(1) = 0;
		ARR_POS(2) = 0;
		ARR_POS(3) = 0;
		ARR_POS(5) = 0;
	}
}

void ScriptEngine::saveState(Common::SeekableWriteStream *out, Script &script, bool onlyTempData) {
	out->writeUint32BE(MKTAG('S', 'N', 'A', 'T'));

	if (!onlyTempData) {
		const uint16 *s = reinterpret_cast<const uint16*>(_arrayData);
		for (int i = 0; i < 128; ++i)
			out->writeUint16BE(s[i]);
		out->write(_flagsTable, 352);

		out->writeByte(script.sentenceDone);
		out->writeByte(script.sentencePos);
	}

	out->writeByte(script.curFileNo);
	out->writeSint16BE(script.curGfxScript);

	if (!onlyTempData) {
		uint16 pos = script.curPos;
		getArrayEntry(5, 0, pos);
		out->writeUint16BE(pos);
	}
}

void ScriptEngine::runOpcode() {
	uint8 op = getOpcode(_pos1);
	if (_opcodes[op].proc->isValid())
#ifdef SNATCHER_SCRIPT_DEBUG
		(*_opcodes[op].proc)(_pos1);
#else
		(*_opcodes[op].proc)();
#endif
	else
		error("%s(): Invalid opcode %d", __FUNCTION__, op);
}

void ScriptEngine::getOpcodeProperties(uint16 &m1, uint16 &m2, uint16 &u) {
	uint8 op = getOpcode(_pos2);
	m1 = _opcodes[op].len;
	m2 = _opcodes[op].flags >> 4;
	u = _opcodes[op].flags & 0x0F;
}

void ScriptEngine::runOpcodeOrReadVar(uint16 mode, uint16 &result) {
	if (mode == 4) {
		int cpos = _pos1;
		runOpcode();
		_pos2 = cpos;
		jump();
	} else if (mode == 1) {
		getFlags(READ_BE_UINT16(_script->data + _pos1), result);
		_pos2 = _pos1 + 2;
	} else if (mode == 3 || mode == 5) {
		result = READ_BE_UINT16(_script->data + _pos1);
		_pos2 = _pos1 + 2;
	} else {
		error("%s(): Invalid opcode length %d", __FUNCTION__, mode);
	}
}

static const uint16 maskTable[] = {
	0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
	0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};

void ScriptEngine::getFlags(uint16 sel, uint16 &result) {
	assert(sel < 0xAF80);
	const uint8 *in = &_flagsTable[sel >> 7];
	uint32 r = (in[0] << 24 | in[1] << 16 | in[2] << 8);
	uint8 v2 = sel & 0x0F;
	uint8 v1 = v2 + 1 + ((sel >> 4) & 7);
	result = ((r << v1) | (r >> (32 - v1))) & maskTable[v2];
}

void ScriptEngine::setFlags(uint16 sel, uint32 flags) {
	uint8 v2 = sel & 0x0F;
	uint8 v1 = v2 + 1 + ((sel >> 4) & 7);

	uint32 msk = maskTable[v2];
	flags &= msk;
	msk = ~msk;

	assert(sel < 0xAF80);
	uint8 *in = &_flagsTable[sel >> 7];
	uint32 cur = (in[0] << 24 | in[1] << 16 | in[2] << 8);

	msk = ((msk << (32 - v1)) | (msk >> v1));
	flags = ((flags << (32 - v1)) | (flags >> v1));

	flags |= (cur & msk);
	*in++ = (flags >> 24) & 0xFF;
	*in++ = (flags >> 16) & 0xFF;
	*in++ = (flags >> 8) & 0xFF;
}

uint8 ScriptEngine::countFunctionOps() {
	uint8 cnt = 0;
	uint16 r1 = 0;
	uint16 r2 = 0;
	uint16 r3 = 0;

	for (bool lp = true; lp; ) {
		++cnt;
		if (!isExecFuncOpcode())
			break;
		if (stackOps_checkFor52(r1))
			cnt += (r1 - 1);
		getOpcodeProperties(r1, r2, r3);
		if (r2 == 7)
			break;
		if (opcodeHasFlag8()) {
			_pos2 = READ_BE_UINT16(_script->data + _pos2 + 1);
		} else {
			++_pos2;
			jump();
		}
	}

	return cnt;
}

void ScriptEngine::seekToFunctionOp(int num) {
	uint16 r1 = 0;
	uint16 r2 = 0;
	uint16 r3 = 0;

	for (bool lp = true; lp; ) {
		if (!isExecFuncOpcode())
			break;
		if (stackOps_checkFor52(r1))
			num -= (r1 - 1);
		--num;
		if (num <= 0) {
			skipOnFlag8();
			break;
		}
		if (opcodeHasFlag8()) {
			_pos2 = READ_BE_UINT16(_script->data + _pos2 + 1);
		} else {
			getOpcodeProperties(r1, r2, r3);
			if (r1 == 4) {
				++_pos2;
				jump();
			} else {
				_pos2 += 3;
			}
		}
	}
}

bool ScriptEngine::isExecFuncOpcode() {
	uint8 op = getOpcode(_pos2);
	return (op >= 30 && op <= 39);
}

bool ScriptEngine::stackOps_checkFor52(uint16 &result) {
	uint16 m = 0;
	uint16 f1 = 0;
	uint16 f2 = 0;
	int cpos = _pos2;
	bool res = false;

	getOpcodeProperties(m, f1, f2);
	if (m == 4) {
		skipOnFlag8();
		uint8 op = getOpcode(_pos2);
		if (op == 52 || op == 53) {
			result = READ_BE_UINT16(_script->data + _pos2 + 1);
			res = true;
		}
	}
	_pos2 = cpos;
	return res;
}

bool ScriptEngine::opcodeHasFlag8() {
	uint16 ln = 0;
	uint16 f1 = 0;
	uint16 f2 = 0;
	getOpcodeProperties(ln, f1, f2);
	return ((f1 | f2) & 8);
}

bool ScriptEngine::evalFinished() {
	if (ARR_POS(1) || _script->newPos != 0xFFFF)
		return false;
	if (_script->sentenceDone) {
		uint16 r1 = 0;
		if (!getArrayLastEntry(0, r1))
			return true;
		return (_script->sentencePos > r1);
	}
	return (_script->sentencePos == ARR_POS(2));
}

void ScriptEngine::skipOnFlag8() {
	_pos2 += (opcodeHasFlag8() ? 3 : 1);
}

void ScriptEngine::jump() {
	for (bool lp = true; lp; ) {
		if (opcodeHasFlag8()) {
			_pos2 = READ_BE_UINT16(_script->data + _pos2 + 1);
			continue;
		}
		lp = false;

		uint16 len = 0;
		uint16 f1 = 0;
		uint16 f2 = 0;
		getOpcodeProperties(len, f1, f2);
		++_pos2;

		if (len == 4)
			jump();
		else if (len != 0 && len != 7)
			_pos2 += 2;

		if (f1 == 4)
			jump();
		else if (f1 != 0 && f1 != 7)
			_pos2 += 2;

		if (f2 == 4)
			jump();
		else if (f2 != 0 && f2 != 7)
			_pos2 += 2;
	}
}

bool ScriptEngine::getArrayLastEntry(uint16 arrNo, uint16 &result) {
	assert(arrNo < ARRAYSIZE(_arrays));
	return getArrayEntry(arrNo, ARR_POS(arrNo) - 1, result);
}

bool ScriptEngine::popArrayLastEntry(uint16 arrNo, uint16 &result) {
	assert(arrNo < ARRAYSIZE(_arrays));
	if (!ARR_POS(arrNo))
		return false;
	getArrayLastEntry(arrNo, result);
	--ARR_POS(arrNo);
	return true;
}

bool ScriptEngine::getArrayEntry(uint16 arrNo, uint8 pos, uint16 &result) {
	assert(arrNo < ARRAYSIZE(_arrays));
	if (ARR_POS(arrNo) <= pos)
		return false;
	result = ARR_READ(arrNo, pos);
	return true;
}

bool ScriptEngine::setArrayLastEntry(uint16 arrNo, uint16 val) {
	assert(arrNo < ARRAYSIZE(_arrays));
	if (ARR_SIZE(arrNo) <= ARR_POS(arrNo))
		return false;
	uint8 pos = ARR_POS(arrNo)++;
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

#ifdef SNATCHER_SCRIPT_DEBUG
int ScriptEngine::ScriptEngineProc::_recursion = 0;
#endif

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
		OP(evalAdd),
		OP(evalAdd),
		OP(evalAdd),
		OP(chatWithPortraitAnim),
		OP(pcmSound),
		OP(evalAnd),
		OP(evalAnd),
		OP(evalAnd),
		OP(evalAnd),
		OP(fmMusicStart),
		OP(runSubScript),
		OP(callSubroutine),
		OP(cdaPlay),
		OP(incrCounter),
		OP(fmSoundEffect),
		OP(evalEquals),
		OP(evalEquals),
		OP(evalEquals),
		OP(clearFlags),
		OP(setFlags),
		OP(getPrevPosition),
		OP(evalLess),
		OP(evalLessOrEqual),
		OP(doEitherOr),
		OP(doEitherOr),
		OP(doIf),
		OP(doIf),
		OP(setResult),
		OP(setResult),
		OP(execFunc),
		OP(execFunc),
		OP(execFunc),
		OP(execFunc),
		OP(execFunc),
		OP(execFunc),
		OP(execFunc),
		OP(execFunc),
		OP(execFunc),
		OP(execFunc),
		OP(evalGreater),
		OP(evalGreater),
		OP(evalGreaterOrEqual),
		OP(break),
		OP(break),
		OP(nop),
		OP(evalTrue),
		OP(evalTrue),
		OP(start),
		OP(evalOr),
		OP(evalOr),
		OP(evalOr),
		OP(condExec),
		OP(condExec),
		OP(sentenceRecurseBegin),
		OP(sentenceRecurseBegin),
		OP(verbOps),
		OP(sentenceRecurseEnd),
		OP(returnFromSubScript),
		OP(checkSubRecurseState),
		OP(checkSubRecurseState),
		OP(loadModuleAndStartGfx),
		OP(randomSwitch),
		OP(sequentialSwitch),
		OP(64),
		OP(sysOps),
		OP(sysOps),
		OP(displayDialog),
		OP(displayDialog),
		OP(animWait),
		OP(pcmSoundWait),
		OP(fmMusicWait),
		OP(cdaWait),
		OP(fmSfxWait),
		OP(waitFrames)
	};
#undef OP

	const uint8 *in = scd->makePtr(0x16CDC)();

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
	if (!evalFinished())
		return;
	_que->writeUInt16(0x09);
	_que->writeUInt16(READ_BE_UINT16(_script->data + _pos1 + 4));
	_que->writeUInt16(READ_BE_UINT16(_script->data + _pos1 + 2));
}

void ScriptEngine::o_evalAdd() {
	uint16 m1 = 0;
	uint16 m2 = 0;
	uint16 f2 = 0;

	_pos2 = _pos1++;
	getOpcodeProperties(m1, m2, f2);
	runOpcodeOrReadVar(m1, _result);
	f2 = _result;
	_pos1 = _pos2;
	runOpcodeOrReadVar(m2, _result);
	_result += f2;
}

void ScriptEngine::o_chatWithPortraitAnim() {
	if (!evalFinished())
		return;
	_que->writeUInt16(0x0D);
	_que->writeUInt16(READ_BE_UINT16(_script->data + _pos1 + 1));
}

void ScriptEngine::o_pcmSound() {
	if (!evalFinished())
		return;
	_que->writeUInt16(0x0C);
	_que->writeUInt16(READ_BE_UINT16(_script->data + _pos1 + 1));
}

void ScriptEngine::o_evalAnd() {
	uint16 m1 = 0;
	uint16 m2 = 0;
	uint16 f2 = 0;

	_pos2 = _pos1++;
	getOpcodeProperties(m1, m2, f2);
	runOpcodeOrReadVar(m1, _result);
	f2 = _result;
	_pos1 = _pos2;
	runOpcodeOrReadVar(m2, _result);
	_result &= f2;
}

void ScriptEngine::o_fmMusicStart() {
	if (!evalFinished())
		return;
	_que->writeUInt16(0x07);
	_que->writeUInt16(READ_BE_UINT16(_script->data + _pos1 + 1));
}

void ScriptEngine::o_runSubScript() {
	if (!evalFinished())
		return;
	uint16 m = 0;
	uint16 f1 = 0;
	uint16 u = 0;

	setArrayLastEntry(3, _script->curFileNo);

	_pos2 = _pos1;
	getOpcodeProperties(m, f1, u);
	++_pos2;

	if (m != 5) {
		uint16 no = READ_BE_UINT16(_script->data + _pos2 + 1);
		_que->writeUInt16(0x11);
		_que->writeUInt32(0x1A800);
		_que->writeUInt16(no);
		_script->curFileNo = no;
		_pos2 += 3;
	}

	if (!getArrayEntry(5, 0, u))
		u = _script->curPos;
	setArrayLastEntry(3, u);

	_script->newPos = READ_BE_UINT16(_script->data + _pos2);
}

void ScriptEngine::o_callSubroutine() {
	uint16 cp = _script->curPos;
	setArrayLastEntry(5, cp);
	_pos1 = _script->curPos = READ_BE_UINT16(_script->data + _pos1 + 1);
	runOpcode();
	popArrayLastEntry(5, cp);
	_script->curPos = cp;
}

void ScriptEngine::o_cdaPlay() {
	if (!evalFinished())
		return;
	_que->writeUInt16(0x0E);
	_que->writeUInt16(READ_BE_UINT16(_script->data + _pos1 + 1));

}

void ScriptEngine::o_incrCounter() {
	if (!evalFinished())
		return;

	uint16 r1 = 0;
	uint16 sel = READ_BE_UINT16(_script->data + _pos1 + 1);
	getFlags(sel, r1);

	uint16 r2 = (0xFFFFFFFF >> (15 - (sel & 0x0F))) & 0xFFFF;
	if (r2 != r1)
		setFlags(sel, r1 + 1);
}

void ScriptEngine::o_fmSoundEffect() {
	if (!evalFinished())
		return;
	_que->writeUInt16(0x1C);
	_que->writeUInt16(READ_BE_UINT16(_script->data + _pos1 + 1));
}

void ScriptEngine::o_evalEquals() {
	uint16 m1 = 0;
	uint16 m2 = 0;
	uint16 f2 = 0;

	_pos2 = _pos1++;
	getOpcodeProperties(m1, m2, f2);
	runOpcodeOrReadVar(m1, _result);
	f2 = _result;
	_pos1 = _pos2;
	runOpcodeOrReadVar(m2, _result);
	_result = (_result == f2) ? 0xFFFF : 0;
}

void ScriptEngine::o_clearFlags() {
	if (!evalFinished())
		return;
	setFlags(READ_BE_UINT16(_script->data + _pos1 + 1), 0);
}

void ScriptEngine::o_setFlags() {
	if (!evalFinished())
		return;
	setFlags(READ_BE_UINT16(_script->data + _pos1 + 1), 0xFFFF);
}

void ScriptEngine::o_getPrevPosition() {
	uint16 n = READ_BE_UINT16(_script->data + _pos1 + 1);
	if (!getArrayEntry(4, ARR_POS(4) - n, _result))
		_result = 0xFFFF;
}

void ScriptEngine::o_evalLess() {
	uint16 m1 = 0;
	uint16 m2 = 0;
	uint16 r1 = 0;
	_pos2 = _pos1++;
	getOpcodeProperties(m1, m2, r1);
	runOpcodeOrReadVar(m1, _result);
	_pos1 = _pos2;
	r1 = _result;
	runOpcodeOrReadVar(m2, _result);
	_result = (_result < r1) ? 0xFFFF : 0;
}

void ScriptEngine::o_evalLessOrEqual() {
	uint16 m1 = 0;
	uint16 m2 = 0;
	uint16 r1 = 0;
	_pos2 = _pos1++;
	getOpcodeProperties(m1, m2, r1);
	runOpcodeOrReadVar(m1, _result);
	_pos1 = _pos2;
	r1 = _result;
	runOpcodeOrReadVar(m2, _result);
	_result = (_result <= r1) ? 0xFFFF : 0;
}

void ScriptEngine::o_doEitherOr() {
	uint16 mode = 0;
	uint16 f1 = 0;
	uint16 f2 = 0;
	_pos2 = _pos1;
	getOpcodeProperties(mode, f1, f2);
	uint16 np = READ_BE_UINT16(_script->data + _pos1 + 1);
	_pos1 += 3;
	runOpcodeOrReadVar(mode, _result);
	_pos1 = _result ? _pos2 : np;
	runOpcode();
}

void ScriptEngine::o_doIf() {
	uint16 mode = 0;
	uint16 f1 = 0;
	uint16 f2 = 0;
	_pos2 = _pos1++;
	getOpcodeProperties(mode, f1, f2);
	runOpcodeOrReadVar(mode, _result);
	if (!_result)
		return;
	_pos1 = _pos2;
	runOpcode();
}

void ScriptEngine::o_setResult() {
	if (!evalFinished())
		return;

	uint16 len = 0;
	uint16 mode = 0;
	uint16 f2 = 0;
	_pos2 = _pos1;
	getOpcodeProperties(len, mode, f2);
	uint16 sel = READ_BE_UINT16(_script->data + _pos1 + 1);
	_pos1 += 3;
	runOpcodeOrReadVar(mode, _result);
	setFlags(sel, _result);
}

void ScriptEngine::o_execFunc() {
	_pos2 = _pos1;
	uint8 num = countFunctionOps();
	uint8 cnt = 1;
	int orig = _pos1;

	for (bool lp = true; lp; ) {
		_pos2 = orig;
		seekToFunctionOp(cnt);
		_pos1 = _pos2;
		runOpcode();

		if (cnt++ == num)
			lp = false;
	}
}

void ScriptEngine::o_evalGreater() {
	uint16 m1 = 0;
	uint16 m2 = 0;
	uint16 f2 = 0;

	_pos2 = _pos1++;
	getOpcodeProperties(m1, m2, f2);
	runOpcodeOrReadVar(m1, _result);
	f2 = _result;
	_pos1 = _pos2;
	runOpcodeOrReadVar(m2, _result);
	_result = (_result > f2) ? 0xFFFF : 0;
}

void ScriptEngine::o_evalGreaterOrEqual() {
	uint16 m1 = 0;
	uint16 m2 = 0;
	uint16 f2 = 0;

	_pos2 = _pos1++;
	getOpcodeProperties(m1, m2, f2);
	runOpcodeOrReadVar(m1, _result);
	f2 = _result;
	_pos1 = _pos2;
	runOpcodeOrReadVar(m2, _result);
	_result = (_result >= f2) ? 0xFFFF : 0;
}
void ScriptEngine::o_break() {
	if (!evalFinished())
		return;

	uint16 m = 0;
	uint16 f1 = 0;
	uint16 f2 = 0;
	_pos2 = _pos1;

	getOpcodeProperties(m, f1, f2);
	++_pos2;
	if (m != 5) {
		int num = READ_BE_UINT16(_script->data + _pos2 + 1);
		_pos2 += 3;
		if (_script->curFileNo != num) {
			_script->curFileNo = num;
			_que->writeUInt16(0x11);
			_que->writeUInt32(0x1A800);
			_que->writeUInt16(num);
		}
	}
	_script->newPos = READ_BE_UINT16(_script->data + _pos2);
}

void ScriptEngine::o_evalTrue() {
	uint16 mode = 0;
	uint16 f1 = 0;
	uint16 f2 = 0;
	_pos2 = _pos1++;
	getOpcodeProperties(mode, f1, f2);
	runOpcodeOrReadVar(mode, _result);
	_result = _result ? 0 : 0xFFFF;
}

void ScriptEngine::o_start() {
	++_pos1;
	runOpcode();
}

void ScriptEngine::o_evalOr() {
	uint16 m1 = 0;
	uint16 m2 = 0;
	uint16 f2 = 0;

	_pos2 = _pos1++;
	getOpcodeProperties(m1, m2, f2);
	runOpcodeOrReadVar(m1, _result);
	f2 = _result;
	_pos1 = _pos2;
	runOpcodeOrReadVar(m2, _result);
	_result |= f2;
}

void ScriptEngine::o_condExec() {
	uint16 ln = 0;
	uint16 f1 = 0;
	uint16 f2 = 0;
	getOpcodeProperties(ln, f1, f2);
	if (f1 == 7)
		return;
	_pos1 += 3;
	runOpcode();
}

void ScriptEngine::o_sentenceRecurseBegin() {
	if (!evalFinished())
		return;

	uint16 m1 = 0;
	uint16 m2 = 0;
	uint16 r = 0;
	getArrayLastEntry(0, r);

	if (_script->sentencePos <= r)
		return;
	setArrayLastEntry(0, _script->sentencePos);

	_pos2 = _pos1;
	getOpcodeProperties(m1, m2, r);
	if (m2 != 4)
		return;

	++_pos1;
	runOpcode();
}

void ScriptEngine::o_verbOps() {
	if (_script->newPos != 0xFFFF)
		return;

	_pos2 = _pos1;

	if (ARR_POS(2) == _script->sentencePos) {
		uint16 val = READ_BE_UINT16(_script->data + _pos2 + 1);

		if (!setArrayLastEntry(1, val))
			debug("Verb array overflow");

	} else {
		uint16 val = 0;
		getArrayEntry(2, _script->sentencePos, val);
		if (READ_BE_UINT16(_script->data + _pos2 + 1) != val)
			return;

		++_script->sentencePos;
		_pos1 += 3;

		runOpcode();

		if (ARR_POS(2) == _script->sentencePos) {
			if (ARR_POS(1) == 0)
				_script->sentenceDone = 0xFF;
		}
		--_script->sentencePos;
	}
}

void ScriptEngine::o_sentenceRecurseEnd() {
	if (!evalFinished())
		return;

	if (ARR_POS(0))
		--ARR_POS(0);
}

void ScriptEngine::o_returnFromSubScript() {
	uint16 res = 0;
	popArrayLastEntry(3, res);
	_script->newPos = res;
	popArrayLastEntry(3, res);
	if (_script->curFileNo == res)
		return;
	_que->writeUInt16(0x11);
	_que->writeUInt32(0x1A800);
	_que->writeUInt16(res);
}

void ScriptEngine::o_checkSubRecurseState() {
	uint16 m = 0;
	uint16 f1 = 0;
	uint16 f2 = 0;
	_pos2 = _pos1;
	getOpcodeProperties(m, f1, f2);
	uint16 val = READ_BE_UINT16(_script->data + _pos2 + 1);
	if (m == 5) {
		_result = 0;
		for (int i = 0; i < ARR_POS(5) && !_result; ++i) {
			if (ARR_READ(5, i) == val)
				_result = 1;
		}
	} else if (val != 0) {
		if (!getArrayEntry(5, ARR_POS(5) - val, _result))
			_result = 0xFFFF;
	} else {
		_result = _script->curPos;
	}
}

void ScriptEngine::o_loadModuleAndStartGfx() {
	if (!evalFinished())
		return;
	int16 num = READ_BE_INT16(_script->data + _pos1 + 1);

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

void ScriptEngine::o_randomSwitch() {
	uint16 sel = READ_BE_UINT16(_script->data + _pos1 + 1);
	_pos2 = _pos1 + 3;
	uint16 f = Util::rngGetNumberFromRange(1, countFunctionOps()) & 0xFF;
	_pos2 = _pos1 + 3;
	seekToFunctionOp(f);
	_pos1 = _pos2;
	runOpcode();
	setFlags(sel, f);
}

void ScriptEngine::o_sequentialSwitch() {
	_pos2 = _pos1 + 3;
	uint16 cnt = countFunctionOps();
	uint16 sel = READ_BE_UINT16(_script->data + _pos1 + 1);
	uint16 val = 0;
	getFlags(sel, val);
	_pos2 = _pos1 + 3;
	seekToFunctionOp(++val);
	_pos1 = _pos2;
	runOpcode();
	if (ARR_POS(1) == 0 && val != cnt)
		setFlags(sel, val);
}

void ScriptEngine::o_64() {
}

void ScriptEngine::o_sysOps() {
	if (!evalFinished())
		return;

	uint16 m = 0;
	uint16 f1 = 0;
	uint16 f2 = 0;
	_pos2 = _pos1;
	getOpcodeProperties(m, f1, f2);
	_pos2 += (m == 4 ? 2 : 1);
	uint16 sub = READ_BE_UINT16(_script->data + _pos2);
	uint16 addr = 0;
	uint16 val = 0;

	switch (sub) {
	case 0:
		// Block fm music
		_que->writeUInt16(0x04);
		break;
	case 1:
		// Save game state
		_que->writeUInt16(0x1E);
		_que->writeUInt16(READ_BE_UINT16(_script->data + _pos1 + 4));
		break;
	case 2:
		_que->writeUInt16(0x21);
		break;
	case 3:
		_que->writeUInt16(0x1F);
		break;
	case 4:
		// Start action sequence
		_aseq->setup(READ_BE_UINT16(_script->data + _pos1 + 4));
		_que->writeUInt16(0x18);
		break;
	case 5:
		// The scripts have the wonderful capability to read from and write into
		// the sub cpu memory. It is limited to 16bit addresses at least. I hope
		// that in practice, this will only happen for some global variables. We
		// will just have to catch every case whenever a new address comes up...
		addr = READ_BE_UINT16(_script->data + _pos1 + 4);
		_result = _mem->readWord(addr);
		break;
	case 6:
		// See case 5
		addr = READ_BE_UINT16(_script->data + _pos1 + 5);
		val = READ_BE_UINT16(_script->data + _pos1 + 7);
		_mem->writeWord(addr, val);
		break;
	case 7:
		// Block pcm sounds
		_que->writeUInt16(0x1D);
		_que->writeUInt16(READ_BE_UINT16(_script->data + _pos1 + 4) ? 1 : 0);
		break;
	case 8:
		if (READ_BE_UINT16(_script->data + _pos1 + 4) == 0) {
			_que->writeUInt16(0x09);
			_que->writeUInt16(0x02);
			_que->writeUInt16(0x01);
		} else {
			// Show inventory item close-up
			_que->writeUInt16(0x15);
			_que->writeUInt16(READ_BE_UINT16(_script->data + _pos1 + 8));
			_que->writeUInt16(READ_BE_UINT16(_script->data + _pos1 + 10));
		}
		break;
	case 9:
		_script->phase = READ_BE_UINT16(_script->data + _pos1 + 4) + 2;
		break;
	case 10:
		val = READ_BE_UINT16(_script->data + _pos1 + 4);
		_ui->setVerbInterfaceMode(val);
		break;
	case 11:
		if (READ_BE_UINT16(_script->data + _pos1 + 4)) {
			_mem->restoreTempState();
			_script->phase = 1;
		} else {
			_mem->saveTempState();
		}
		break;
	case 12:
		// This happens when starting a new act. Apparently, only a small portion of the game
		// variables is needed for the whole game. The rest gets cleared and can be reused.
		_flagsTable[53] &= 0xF8;
		memset(_flagsTable + 54, 0, 298);
		break;
	case 13:
		// CD-Audio sync
		_que->writeUInt16(0x1B);
		_que->writeUInt16(READ_BE_UINT16(_script->data + _pos1 + 5));
		_que->writeUInt16(READ_BE_UINT16(_script->data + _pos1 + 8));
		_que->writeUInt16(READ_BE_UINT16(_script->data + _pos1 + 10));
		break;
	default:
		break;
	}
}

void ScriptEngine::o_displayDialog() {
	if (!evalFinished())
		return;

	uint16 m = 0;
	uint16 f1 = 0;
	uint16 f2 = 0;
	_pos2 = _pos1;

	getOpcodeProperties(m, f1, f2);
	++_pos2;

	_que->writeUInt16(0x08);
	if (m == 4) {
		_que->writeUInt16(READ_BE_UINT16(_script->data + _pos2 + 1));
		_pos2 += 3;
	} else {
		_que->writeUInt16(0);
	}
	_que->writeUInt16(READ_BE_UINT16(_script->data + _pos2));
}

void ScriptEngine::o_animWait() {
	if (!evalFinished())
		return;
	_que->writeUInt16(0x09);
	_que->writeUInt16(0x03);
	_que->writeUInt16(READ_BE_UINT16(_script->data + _pos1 + 1));
}

void ScriptEngine::o_pcmSoundWait() {
	if (evalFinished())
		_que->writeUInt16(0x0A);
}

void ScriptEngine::o_fmMusicWait() {
	if (evalFinished())
		_que->writeUInt16(0x13);
}

void ScriptEngine::o_cdaWait() {
	if (evalFinished())
		_que->writeUInt16(0x0B);
}

void ScriptEngine::o_fmSfxWait() {
	if (evalFinished())
		_que->writeUInt16(0x19);
}

void ScriptEngine::o_waitFrames() {
	if (!evalFinished())
		return;
	_que->writeUInt16(0x10);
	_que->writeUInt16(READ_BE_UINT16(_script->data + _pos1 + 1));
}

#undef ARR_SIZE
#undef ARR_POS
#undef ARR_READ
#undef ARR_WRITE

} // End of namespace Snatcher
