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

#include "kyra/script_tim.h"
#include "kyra/script.h"
#include "kyra/resource.h"

#include "common/endian.h"

namespace Kyra {

TIMInterpreter::TIMInterpreter(KyraEngine_v1 *vm, OSystem *system) : _vm(vm), _system(system), _currentTim(0) {
#define COMMAND(x) { &TIMInterpreter::x, #x }
#define COMMAND_UNIMPL() { 0, 0 }
	static const CommandEntry commandProcs[] = {
		// 0x00
		COMMAND(cmd_initFunc0),
		COMMAND(cmd_stopCurFunc),
		COMMAND_UNIMPL(),
		COMMAND_UNIMPL(),
		// 0x04
		COMMAND(cmd_initFunc),
		COMMAND(cmd_stopFunc),
		COMMAND_UNIMPL(),
		COMMAND_UNIMPL(),
		// 0x08
		COMMAND_UNIMPL(),
		COMMAND_UNIMPL(),
		COMMAND_UNIMPL(),
		COMMAND_UNIMPL(),
		// 0x0C
		COMMAND_UNIMPL(),
		COMMAND_UNIMPL(),
		COMMAND_UNIMPL(),
		COMMAND_UNIMPL(),
		// 0x10
		COMMAND_UNIMPL(),
		COMMAND_UNIMPL(),
		COMMAND_UNIMPL(),
		COMMAND_UNIMPL(),
		// 0x14
		COMMAND_UNIMPL(),
		COMMAND_UNIMPL(),
		COMMAND_UNIMPL(),
		COMMAND(cmd_resetAllRuntimes),
		// 0x18
		COMMAND(cmd_return<1>),
		COMMAND(cmd_execOpcode),
		COMMAND(cmd_initFuncNow),
		COMMAND(cmd_stopFuncNow),
		// 0x1C
		COMMAND(cmd_return<1>),
		COMMAND(cmd_return<1>),
		COMMAND(cmd_return<-1>)
	};

	_commands = commandProcs;
	_commandsSize = ARRAYSIZE(commandProcs);
}

TIM *TIMInterpreter::load(const char *filename, const Common::Array<const TIMOpcode*> *opcodes) {
	if (!_vm->resource()->exists(filename))
		return 0;

	ScriptFileParser file(filename, _vm->resource());
	if (!file)
		error("Couldn't open TIM file '%s'", filename);

	uint32 formBlockSize = file.getFORMBlockSize();
	if (formBlockSize == 0xFFFFFFFF)
		error("No FORM chunk found in TIM file '%s'", filename);

	if (formBlockSize < 20)
		error("TIM file '%s' FORM chunk size smaller than 20", filename);

	TIM *tim = new TIM;
	assert(tim);
	memset(tim, 0, sizeof(TIM));

	tim->procFunc = -1;
	tim->opcodes = opcodes;

	uint32 avtlChunkSize = file.getIFFBlockSize(AVTL_CHUNK);
	uint32 textChunkSize = file.getIFFBlockSize(TEXT_CHUNK);

	tim->avtl = new uint16[avtlChunkSize/2];
	if (textChunkSize != 0xFFFFFFFF)
		tim->text = new byte[textChunkSize];

	if (!file.loadIFFBlock(AVTL_CHUNK, tim->avtl, avtlChunkSize))
		error("Couldn't read AVTL chunk in TIM file '%s'", filename);
	if (textChunkSize != 0xFFFFFFFF && !file.loadIFFBlock(TEXT_CHUNK, tim->text, textChunkSize))
		error("Couldn't read TEXT chunk in TIM file '%s'", filename);

	avtlChunkSize >>= 1;
	for (uint i = 0; i < avtlChunkSize; ++i)
		tim->avtl[i] = READ_LE_UINT16(tim->avtl + i);
	
	int num = (avtlChunkSize < TIM::kCountFuncs) ? avtlChunkSize : (int)TIM::kCountFuncs;
	for (int i = 0; i < num; ++i)
		tim->func[i].avtl = tim->avtl + tim->avtl[i];	

	return tim;
}

void TIMInterpreter::unload(TIM *&tim) const {
	if (!tim)
		return;

	delete[] tim->text;
	delete[] tim->avtl;
	delete tim;
	tim = 0;
}

void TIMInterpreter::exec(TIM *tim, bool loop) {
	if (!tim)
		return;

	_currentTim = tim;
	if (!_currentTim->func[0].ip) {
		_currentTim->func[0].ip = _currentTim->func[0].avtl;
		_currentTim->func[0].nextTime = _currentTim->func[0].lastTime = _system->getMillis();
	}

	do {
		for (_currentFunc = 0; _currentFunc < TIM::kCountFuncs; ++_currentFunc) {
			TIM::Function &cur = _currentTim->func[_currentFunc];

			if (_currentTim->procFunc != -1)
				execCommand(28, &_currentTim->procParam);

			bool running = true;
			while (cur.ip && cur.nextTime <= _system->getMillis() && running) {
				int8 opcode = int8(cur.ip[2] & 0xFF);

				switch (execCommand(opcode, cur.ip + 3)) {
				case -1:
					loop = false;
					running = false;
					_currentFunc = 11;
					break;

				case -2:
					running = false;
					break;

				case -3:
					_currentTim->procFunc = _currentFunc;
					break;

				default:
					break;
				}

				if (cur.ip) {
					cur.ip += cur.ip[0];
					cur.lastTime = cur.nextTime;
					cur.nextTime += cur.ip[1] * _vm->tickLength();
				}
			}
		}
	} while (loop);
}

void TIMInterpreter::refreshTimersAfterPause(uint32 elapsedTime) {
	if (!_currentTim)
		return;

	for (int i = 0; i < TIM::kCountFuncs; i++) {
		if (_currentTim->func[i].lastTime)
			_currentTim->func[i].lastTime += elapsedTime;
		if (_currentTim->func[i].nextTime)
			_currentTim->func[i].nextTime += elapsedTime;
	}
}

int TIMInterpreter::execCommand(int cmd, const uint16 *param) {
	if (cmd < 0 || cmd >= _commandsSize) {
		warning("Calling unimplemented TIM command %d", cmd);
		return 0;
	}

	if (_commands[cmd].proc == 0) {
		warning("Calling unimplemented TIM command %d", cmd);
		return 0;
	}

	debugC(5, kDebugLevelScript, "TIMInterpreter::%s(%p)", _commands[cmd].desc, (const void*)param);
	return (this->*_commands[cmd].proc)(param);
}

int TIMInterpreter::cmd_initFunc0(const uint16 *param) {
	_currentTim->func[0].ip = _currentTim->func[0].avtl;
	_currentTim->func[0].lastTime = _system->getMillis();
	return 1;
}

int TIMInterpreter::cmd_stopCurFunc(const uint16 *param) {
	if (_currentFunc < TIM::kCountFuncs)
		_currentTim->func[_currentFunc].ip = 0;
	if (!_currentFunc)
		_finished = true;
	return -2;
}

int TIMInterpreter::cmd_initFunc(const uint16 *param) {
	uint16 func = *param;
	assert(func < TIM::kCountFuncs);
	if (_currentTim->func[func].avtl)
		_currentTim->func[func].ip = _currentTim->func[func].avtl;
	else
		_currentTim->func[func].avtl = _currentTim->func[func].ip = _currentTim->avtl + _currentTim->avtl[func];
	return 1;
}

int TIMInterpreter::cmd_stopFunc(const uint16 *param) {
	uint16 func = *param;
	assert(func < TIM::kCountFuncs);
	_currentTim->func[func].ip = 0;
	return 1;
}

int TIMInterpreter::cmd_resetAllRuntimes(const uint16 *param) {
	for (int i = 0; i < TIM::kCountFuncs; ++i) {
		if (_currentTim->func[i].ip)
			_currentTim->func[i].nextTime = _system->getMillis();
	}
	return 1;
}

int TIMInterpreter::cmd_execOpcode(const uint16 *param) {
	if (!_currentTim->opcodes) {
		warning("Trying to execute TIM opcode without opcode list");
		return 0;
	}

	uint16 opcode = *param++;
	if (opcode > _currentTim->opcodes->size()) {
		warning("Calling unimplemented TIM opcode(0x%.02X/%d)", opcode, opcode);
		return 0;
	}

	return (*(*_currentTim->opcodes)[opcode])(_currentTim, param);
}

int TIMInterpreter::cmd_initFuncNow(const uint16 *param) {
	uint16 func = *param;
	assert(func < TIM::kCountFuncs);
	_currentTim->func[func].ip = _currentTim->func[func].avtl;
	_currentTim->func[func].lastTime = _currentTim->func[func].nextTime = _system->getMillis();
	return 1;
}

int TIMInterpreter::cmd_stopFuncNow(const uint16 *param) {
	uint16 func = *param;
	assert(func < TIM::kCountFuncs);
	_currentTim->func[func].ip = 0;
	_currentTim->func[func].lastTime = _currentTim->func[func].nextTime = _system->getMillis();
	return 1;
}

} // end of namespace Kyra

