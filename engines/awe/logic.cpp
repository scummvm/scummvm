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

#include "awe/awe.h"
#include "awe/logic.h"
#include "awe/resource.h"
#include "awe/video.h"
#include "awe/serializer.h"
#include "awe/system_stub.h"

namespace Awe {

Logic::Logic(Resource *res, Video *vid, SystemStub *stub)
	: _res(res), _vid(vid), _stub(stub) {
}

void Logic::init() {
	memset(_scriptVars, 0, sizeof(_scriptVars));
	_scriptVars[0x54] = 0x81;
	_scriptVars[VAR_RANDOM_SEED] = 0;
	_fastMode = false;
}

void Logic::op_movConst() {
	uint8 i = _scriptPtr.fetchByte();
	int16 n = _scriptPtr.fetchWord();
	debugC(kDebugLogic, "%.4x - op_movConst(0x%02X, %d)",
		_lastOpcodeOffset, i, n);
	_scriptVars[i] = n;
}

void Logic::op_mov() {
	uint8 i = _scriptPtr.fetchByte();
	uint8 j = _scriptPtr.fetchByte();	
	debugC(kDebugLogic, "%.4x - op_mov(0x%02X, 0x%02X)",
		_lastOpcodeOffset, i, j);
	_scriptVars[i] = _scriptVars[j];
}

void Logic::op_add() {
	uint8 i = _scriptPtr.fetchByte();
	uint8 j = _scriptPtr.fetchByte();
	debugC(kDebugLogic, "%.4x - op_add(0x%02X, 0x%02X)",
		_lastOpcodeOffset, i, j);
	_scriptVars[i] += _scriptVars[j];
}

void Logic::op_addConst() {
	uint8 i = _scriptPtr.fetchByte();
	int16 n = _scriptPtr.fetchWord();
	debugC(kDebugLogic, "%.4x - op_addConst(0x%02X, %d)",
		_lastOpcodeOffset, i, n);
	_scriptVars[i] += n;
}

void Logic::op_call() {
	uint16 off = _scriptPtr.fetchWord();
	uint8 sp = _stackPtr;
	debugC(kDebugLogic, "%.4x - op_call(0x%X)",
		_lastOpcodeOffset, off);
	_scriptStackCalls[sp] = _scriptPtr.pc - _res->_segCode;
	++_stackPtr;
	_scriptPtr.pc = _res->_segCode + off;
}

void Logic::op_ret() {
	debugC(kDebugLogic, "%.4x - op_ret()", _lastOpcodeOffset);
	--_stackPtr;
	uint8 sp = _stackPtr;
	_scriptPtr.pc = _res->_segCode + _scriptStackCalls[sp];
}

void Logic::op_break() {
	debugC(kDebugLogic, "%.4x - op_break()", _lastOpcodeOffset);
	_scriptHalted = true;
}

void Logic::op_jmp() {
	uint16 off = _scriptPtr.fetchWord();
	debugC(kDebugLogic, "%.4x - op_jmp(0x%02X)", _lastOpcodeOffset, off);
	_scriptPtr.pc = _res->_segCode + off;	
}

void Logic::op_setScriptPos() {
	uint8 i = _scriptPtr.fetchByte();
	uint16 n = _scriptPtr.fetchWord();
	debugC(kDebugLogic, "%.4x - op_setScriptPos(0x%X, 0x%X)",
		_lastOpcodeOffset, i, n);
	_scriptPos[1][i] = n;
}

void Logic::op_jnz() {
	uint8 i = _scriptPtr.fetchByte();
	debugC(kDebugLogic, "%.4x - op_jnz(0x%02X)",
		_lastOpcodeOffset, i);
	--_scriptVars[i];
	if (_scriptVars[i] != 0) {
		op_jmp();
	} else {
		_scriptPtr.fetchWord();
	}
}

void Logic::op_condJmp() {
#ifdef BYPASS_PROTECTION
	if (_res->_curPtrsId == 0x3E80 && _scriptPtr.pc == _res->_segCode + 0xCB9) {
		// (0x0CB8) condJmp(0x80, VAR(41), VAR(30), 0xCD3)
		*(_scriptPtr.pc + 0x00) = 0x81;
		*(_scriptPtr.pc + 0x03) = 0x0D;
		*(_scriptPtr.pc + 0x04) = 0x24;
		// (0x0D4E) condJmp(0x4, VAR(50), 6, 0xDBC)		
		*(_scriptPtr.pc + 0x99) = 0x0D;
		*(_scriptPtr.pc + 0x9A) = 0x5A;
		warning("Logic::op_condJmp() bypassing protection");
	}
#endif
	// Patch original interpreter triggers if code is wrong
	if (_res->_curPtrsId == 0x3e80 && _lastOpcodeOffset == 0xc4b) {
		byte *script = _scriptPtr.pc;
		*script = 0x81;
		WRITE_BE_UINT16(script + 3, 0xcb7);
		WRITE_BE_UINT16(script + 153, 0xced);
	}

	uint8 op = _scriptPtr.fetchByte();
	int16 b = _scriptVars[_scriptPtr.fetchByte()];
	int16 a = _scriptPtr.fetchByte();

	if (op & 0x80) {
		a = _scriptVars[a];
	} else {
		if (op & 0x40) {
			a = (a << 8) | _scriptPtr.fetchByte();
		}
	}

	debugC(kDebugLogic, "%.4x - op_condJmp(%d, 0x%02X, 0x%02X)",
		_lastOpcodeOffset, op, b, a);
	bool expr = false;

	switch (op & 7) {
	case 0:	// jz
		expr = (b == a);
		break;
	case 1: // jnz
		expr = (b != a);
		break;
	case 2: // jg
		expr = (b > a);
		break;
	case 3: // jge
		expr = (b >= a);
		break;
	case 4: // jl
		expr = (b < a);
		break;
	case 5: // jle
		expr = (b <= a);
		break;
	default:
		warning("Logic::op_condJmp() invalid condition %d", (op & 7));
		break;
	}
	if (expr) {
		op_jmp();
	} else {
		_scriptPtr.fetchWord();
	}
}

void Logic::op_setPalette() {
	uint16 i = _scriptPtr.fetchWord();
	debugC(kDebugLogic, "%.4x - op_changePalette(%d)", _lastOpcodeOffset, i);
	_vid->_newPal = i >> 8;
}

void Logic::op_resetScript() {
	uint8 j = _scriptPtr.fetchByte();
	uint8 i = _scriptPtr.fetchByte();
	int8 n = (i & 0x3F) - j;
	if (n < 0) {
		warning("Logic::op_resetScript() ec=0x%X (n < 0)", 0x880);
		return;
	}
	++n;
	uint8 _al = _scriptPtr.fetchByte();

	debugC(kDebugLogic, "%.4x - op_resetScript(%d, %d, %d)",
		_lastOpcodeOffset, j, i, _al);

	if (_al == 2) {
		uint16 *_si = &_scriptPos[1][j];
		while (n--) {
			*_si = 0xFFFE;
			++_si;
		}
	} else if (_al < 2) {
		uint8 *_si = &_scriptPaused[1][j];
		while (n--) {
			*_si = _al;
			++_si;
		}
	}
}

void Logic::op_selectPage() {
	uint8 i = _scriptPtr.fetchByte();
	debugC(kDebugLogic, "%.4x - op_selectPage(%d)",
		_lastOpcodeOffset, i);
	_vid->changePagePtr1(i);
}

void Logic::op_fillPage() {
	uint8 screen = _scriptPtr.fetchByte();
	uint8 color = _scriptPtr.fetchByte();
	debugC(kDebugLogic, "%.4x - op_fillPage(%d, %d)",
		_lastOpcodeOffset, screen, color);
	_vid->fillPage(screen, color);
}

void Logic::op_copyPage() {
	uint8 i = _scriptPtr.fetchByte();
	uint8 j = _scriptPtr.fetchByte();
	debugC(kDebugLogic, "%.4x - op_copyPage(%d, %d)",
		_lastOpcodeOffset, i, j);
	_vid->copyPage(i, j, _scriptVars[VAR_SCROLL_Y]);
}

void Logic::op_updateDisplay() {
	uint8 page = _scriptPtr.fetchByte();
	debugC(kDebugLogic, "%.4x - op_updateDisplay(%d)",
		_lastOpcodeOffset, page);
	inp_handleSpecialKeys();
	if (_res->_curPtrsId == 0x3E80 && _scriptVars[0x67] == 1) {
		_scriptVar_0xBF = _scriptVars[0xBF];
		_scriptVars[0xDC] = 0x21;
	}

	uint32 tstamp = _stub->getTimeStamp();

	if (!_fastMode) {
		// XXX experimental
		int32 delay = tstamp - _lastTimestamp;
		int32 pause = _scriptVars[VAR_PAUSE_SLICES] * 20 - delay;
		if (pause > 0) {
			_stub->sleep(pause);
		}
	}

	_scriptVars[0xF7] = 0;

	_vid->updateDisplay(page);
	_lastTimestamp = tstamp;
}

void Logic::op_halt() {
	debugC(kDebugLogic, "%.4x - op_halt()", _lastOpcodeOffset);
	_scriptPtr.pc = _res->_segCode + 0xFFFF;
	_scriptHalted = true;
}

void Logic::op_drawString() {
	uint16 strId = _scriptPtr.fetchWord();
	uint16 x = _scriptPtr.fetchByte();
	uint16 y = _scriptPtr.fetchByte();
	uint16 col = _scriptPtr.fetchByte();
	debugC(kDebugLogic, "%.4x - op_drawString(0x%03X, %d, %d, %d)",
		_lastOpcodeOffset, strId, x, y, col);
	_vid->drawString(col, x, y, strId);
}

void Logic::op_sub() {
	uint8 i = _scriptPtr.fetchByte();
	uint8 j = _scriptPtr.fetchByte();
	debugC(kDebugLogic, "%.4x - op_sub(0x%02X, 0x%02X)",
		_lastOpcodeOffset, i, j);
	_scriptVars[i] -= _scriptVars[j];
}

void Logic::op_and() {
	uint8 i = _scriptPtr.fetchByte();
	uint16 n = _scriptPtr.fetchWord();
	debugC(kDebugLogic, "%.4x - op_and(0x%02X, %d)",
		_lastOpcodeOffset, i, n);
	_scriptVars[i] = (uint16)_scriptVars[i] & n;
}

void Logic::op_or() {
	uint8 i = _scriptPtr.fetchByte();
	uint16 n = _scriptPtr.fetchWord();
	debugC(kDebugLogic, "%.4x - op_or(0x%02X, %d)",
		_lastOpcodeOffset, i, n);
	_scriptVars[i] = (uint16)_scriptVars[i] | n;
}

void Logic::op_shl() {
	uint8 i = _scriptPtr.fetchByte();
	uint16 n = _scriptPtr.fetchWord();
	debugC(kDebugLogic, "%.4x - op_shl(0x%02X, %d)",
		_lastOpcodeOffset, i, n);
	_scriptVars[i] = (uint16)_scriptVars[i] << n;
}

void Logic::op_shr() {
	uint8 i = _scriptPtr.fetchByte();
	uint16 n = _scriptPtr.fetchWord();
	debugC(kDebugLogic, "%.4x - op_shr(0x%02X, %d)",
		_lastOpcodeOffset, i, n);
	_scriptVars[i] = (uint16)_scriptVars[i] >> n;
}

void Logic::op_soundUnk1() {
	uint16 b = _scriptPtr.fetchWord();
	uint16 c = _scriptPtr.fetchWord();
	uint8 a = _scriptPtr.fetchByte();
	debugC(kDebugLogic, "%.4x - op_soundUnk1(0x%X, 0x%X, %d)",
		_lastOpcodeOffset, b, c, a);
	// XXX
}

void Logic::op_updateMemList() {
	uint16 num = _scriptPtr.fetchWord();
	debugC(kDebugLogic, "%.4x - op_updateMemList(%d)",
		_lastOpcodeOffset, num);
	_res->update(num);
}

void Logic::op_soundUnk2() {
	uint16 b = _scriptPtr.fetchWord();
	uint16 c = _scriptPtr.fetchWord();
	uint8 a = _scriptPtr.fetchByte();
	debugC(kDebugLogic, "%.4x - op_soundUnk2(0x%X, 0x%X, %d)",
		_lastOpcodeOffset, b, c, a);
	// XXX
}

void Logic::restartAt(uint16 ptrId) {
	// XXX
	_scriptVars[0xE4] = 0x14;
	_res->setupPtrs(ptrId);
	memset((uint8 *)_scriptPos, 0xFF, sizeof(_scriptPos));
	memset((uint8 *)_scriptPaused, 0, sizeof(_scriptPaused));
	_scriptPos[0][0] = 0;	
}

void Logic::setupScripts() {
	if (_res->_newPtrsId != 0) {
		restartAt(_res->_newPtrsId);
		_res->_newPtrsId = 0;
	}
	for (int i = 0; i < 0x40; ++i) {
		_scriptPaused[0][i] = _scriptPaused[1][i];
		uint16 n = _scriptPos[1][i];
		if (n != 0xFFFF) {
			_scriptPos[0][i] = (n == 0xFFFE) ? 0xFFFF : n;
			_scriptPos[1][i] = 0xFFFF;
		}
	}
}

void Logic::runScripts() {
	for (int i = 0; i < 0x40 && !g_engine->shouldQuit(); ++i) {
		if (_scriptPaused[0][i] == 0) {
			uint16 n = _scriptPos[0][i];
			if (n != 0xFFFF) {
				_scriptPtr.pc = _res->_segCode + n;
				_stackPtr = 0;
				_scriptHalted = false;
				debugC(kDebugLogic, "Logic::runScripts() i=0x%02X n=0x%02X *p=0x%02X", i, n, *_scriptPtr.pc);
				executeScript();
				_scriptPos[0][i] = _scriptPtr.pc - _res->_segCode;
				debugC(kDebugLogic, "Logic::runScripts() i=0x%02X pos=0x%X", i, _scriptPos[0][i]);
				if (_stub->_pi.quit) {
					break;
				}
			}
		}
	}
}

void Logic::executeScript() {
	while (!_scriptHalted && !g_engine->shouldQuit()) {
		// Get the next opcode
		_lastOpcodeOffset = _scriptPtr.pc - _res->_segCode;
		uint8 opcode = _scriptPtr.fetchByte();

		if (opcode & 0x80) {
			uint16 off = ((opcode << 8) | _scriptPtr.fetchByte()) * 2;
			_res->_useSegVideo2 = false;
			int16 x = _scriptPtr.fetchByte();
			int16 y = _scriptPtr.fetchByte();
			int16 h = y - 199;
			if (h > 0) {
				y = 199;
				x += h;
			}
			debugC(kDebugVideo, "vid_opcd_0x80 : opcode=0x%X off=0x%X x=%d y=%d", opcode, off, x, y);
			_vid->setDataBuffer(_res->_segVideo1, off);
			_vid->drawShape(0xFF, 0x40, Point(x,y));
		} else if (opcode & 0x40) {
			int16 x, y;
			uint16 off = _scriptPtr.fetchWord() * 2;
			x = _scriptPtr.fetchByte();
			_res->_useSegVideo2 = false;
			if (!(opcode & 0x20)) {
				if (!(opcode & 0x10)) {
					x = (x << 8) | _scriptPtr.fetchByte();
				} else {
					x = _scriptVars[x];
				}
			} else {
				if (opcode & 0x10) {
					x += 0x100;
				}
			}
			y = _scriptPtr.fetchByte();
			if (!(opcode & 8)) {
				if (!(opcode & 4)) {
					y = (y << 8) | _scriptPtr.fetchByte();
				} else {
					y = _scriptVars[y];
				}
			}
			uint16 zoom = _scriptPtr.fetchByte();
			if (!(opcode & 2)) {
				if (!(opcode & 1)) {
					--_scriptPtr.pc;
					zoom = 0x40;
				} else {
					zoom = _scriptVars[zoom];
				}
			} else {
				if (opcode & 1) {
					_res->_useSegVideo2 = true;
					--_scriptPtr.pc;
					zoom = 0x40;
				}
			}
			debugC(kDebugVideo, "vid_opcd_0x40 : off=0x%X x=%d y=%d", off, x, y);
			_vid->setDataBuffer(_res->_useSegVideo2 ? _res->_segVideo2 : _res->_segVideo1, off);
			_vid->drawShape(0xFF, zoom, Point(x, y));
		} else {
			if (opcode > 0x1A) {
				error("Logic::executeScript() ec=0x%X invalid opcode=0x%X", 0xFFF, opcode);
			} else {
				(this->*_opTable[opcode])();
			}
		}
	}
}

void Logic::inp_updatePlayer() {
	_stub->processEvents();
	if (_res->_curPtrsId == 0x3E89) {
		char c = _stub->_pi.lastChar;
		if (c == 8 || /*c == 0xD ||*/ c == 0 || (c >= 'a' && c <= 'z')) {
			_scriptVars[VAR_LAST_KEYCHAR] = c & ~0x20;
			_stub->_pi.lastChar = 0;
		}
	}
	int16 lr = 0;
	int16 m = 0;
	int16 ud = 0;
	if (_stub->_pi.dirMask & PlayerInput::DIR_RIGHT) {
		lr = 1;
		m |= 1;
	}
	if (_stub->_pi.dirMask & PlayerInput::DIR_LEFT) {
		lr = -1;
		m |= 2;
	}
	if (_stub->_pi.dirMask & PlayerInput::DIR_DOWN) {
		ud = 1;
		m |= 4;
	}
	_scriptVars[VAR_HERO_POS_UP_DOWN] = ud;
	if (_stub->_pi.dirMask & PlayerInput::DIR_UP) {
		_scriptVars[VAR_HERO_POS_UP_DOWN] = -1;
	}
	if (_stub->_pi.dirMask & PlayerInput::DIR_UP) { // inpJump
		ud = -1;
		m |= 8;
	}
	_scriptVars[VAR_HERO_POS_JUMP_DOWN] = ud;
	_scriptVars[VAR_HERO_POS_LEFT_RIGHT] = lr;
	_scriptVars[VAR_HERO_POS_MASK] = m;
	int16 button = 0;
	if (_stub->_pi.button) { // inpButton
		button = 1;
		m |= 0x80;
	}
	_scriptVars[VAR_HERO_ACTION] = button;
	_scriptVars[VAR_HERO_ACTION_POS_MASK] = m;
}

void Logic::inp_handleSpecialKeys() {
	if (_stub->_pi.pause) {
		if (_res->_curPtrsId != 0x3E80 && _res->_curPtrsId != 0x3E81) {
			_stub->_pi.pause = false;
			while (!_stub->_pi.pause) {
				_stub->processEvents();
				_stub->sleep(200);
			}
		}
		_stub->_pi.pause = false;
	}
	if (_stub->_pi.code) {
		_stub->_pi.code = false;
		if (_res->_curPtrsId != 0x3E89 && _res->_curPtrsId != 0x3E80) {
			_res->_newPtrsId = 0x3E89;
		}
	}
	// XXX
	if (_scriptVars[0xC9] == 1) {
		warning("Logic::inp_handleSpecialKeys() unhandled case (_scriptVars[0xC9] == 1)");
	}
}

void Logic::saveOrLoad(Serializer &ser) {
	Serializer::Entry entries[] = {
		SE_ARRAY(_scriptVars, 0x100, Serializer::SES_INT16, VER(1)),
		SE_ARRAY(_scriptStackCalls, 0x100, Serializer::SES_INT16, VER(1)),
		SE_ARRAY(_scriptPos, 0x40 * 2, Serializer::SES_INT16, VER(1)),
		SE_ARRAY(_scriptPaused, 0x40 * 2, Serializer::SES_INT8, VER(1)),
		SE_END()
	};
	ser.saveOrLoadEntries(entries);
}

} // namespace Awe
