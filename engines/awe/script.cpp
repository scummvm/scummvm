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

#include "awe/gfx.h"
#include "awe/script.h"
#include "awe/resource.h"
#include "awe/video.h"
#include "awe/sfx_player.h"
#include "awe/sound.h"
#include "awe/system_stub.h"

namespace Awe {

Script::Script(Sound *snd, Resource *res, SfxPlayer *ply, Video *vid)
	: _sound(snd), _res(res), _ply(ply), _vid(vid), _stub(nullptr) {
}

void Script::init() {
	memset(_scriptVars, 0, sizeof(_scriptVars));
	_fastMode = false;
	_ply->_syncVar = &_scriptVars[VAR_MUSIC_SYNC];
	_scriptPtr.byteSwap = _is3DO = (_res->getDataType() == DT_3DO);
	if (_is3DO) {
		_scriptVars[0xDB] = 1;
		_scriptVars[0xE2] = 1;
		_scriptVars[0xF2] = 6000;
	} else if (_res->getDataType() != DT_15TH_EDITION && _res->getDataType() != DT_20TH_EDITION) {
		_scriptVars[VAR_RANDOM_SEED] = 0; // time(0);

		if (!_res->_copyProtection) {
			// these 3 variables are set by the game code
			_scriptVars[0xBC] = 0x10;
			_scriptVars[0xC6] = 0x80;
			_scriptVars[0xF2] = (_res->getDataType() == DT_AMIGA || _res->getDataType() == DT_ATARI) ? 6000 : 4000;
			// these 2 variables are set by the engine executable
			_scriptVars[0xDC] = 33;
		}

		if (_res->getDataType() == DT_DOS || _res->getDataType() == DT_WIN31) {
			_scriptVars[0xE4] = 20;
		}
	}
}

void Script::op_movConst() {
	const uint8 i = _scriptPtr.fetchByte();
	const int16 n = _scriptPtr.fetchWord();
	debugC(kDebugScript, "Script::op_movConst(0x%02X, %d)", i, n);
	_scriptVars[i] = n;
}

void Script::op_mov() {
	const uint8 i = _scriptPtr.fetchByte();
	const uint8 j = _scriptPtr.fetchByte();
	debugC(kDebugScript, "Script::op_mov(0x%02X, 0x%02X)", i, j);
	_scriptVars[i] = _scriptVars[j];
}

void Script::op_add() {
	const uint8 i = _scriptPtr.fetchByte();
	const uint8 j = _scriptPtr.fetchByte();
	debugC(kDebugScript, "Script::op_add(0x%02X, 0x%02X)", i, j);
	_scriptVars[i] += _scriptVars[j];
}

void Script::op_addConst() {
	if (_res->getDataType() == DT_DOS || _res->getDataType() == DT_AMIGA || _res->getDataType() == DT_ATARI) {
		if (_res->_currentPart == 16006 && _scriptPtr.pc == _res->_segCode + 0x6D48) {
			warning("Script::op_addConst() workaround for infinite looping gun sound");
			// The script 0x27 slot 0x17 doesn't stop the gun sound from looping.
			// This is a bug in the original game code, confirmed by Eric Chahi and
			// addressed with the anniversary editions.
			// For older releases (DOS, Amiga), we play the 'stop' sound like it is
			// done in other part of the game code.
			//
			//  6D43: jmp(0x6CE5)
			//  6D46: break
			//  6D47: VAR(0x06) -= 50
			//
			snd_playSound(0x5B, 1, 64, 1);
		}
	}
	const uint8 i = _scriptPtr.fetchByte();
	const int16 n = _scriptPtr.fetchWord();
	debugC(kDebugScript, "Script::op_addConst(0x%02X, %d)", i, n);
	_scriptVars[i] += n;
}

void Script::op_call() {
	const uint16 off = _scriptPtr.fetchWord();
	debugC(kDebugScript, "Script::op_call(0x%X)", off);
	if (_stackPtr == 0x40) {
		error("Script::op_call() ec=0x%X stack overflow", 0x8F);
	}
	_scriptStackCalls[_stackPtr] = _scriptPtr.pc - _res->_segCode;
	++_stackPtr;
	_scriptPtr.pc = _res->_segCode + off;
}

void Script::op_ret() {
	debugC(kDebugScript, "Script::op_ret()");
	if (_stackPtr == 0) {
		error("Script::op_ret() ec=0x%X stack underflow", 0x8F);
	}
	--_stackPtr;
	_scriptPtr.pc = _res->_segCode + _scriptStackCalls[_stackPtr];
}

void Script::op_yieldTask() {
	debugC(kDebugScript, "Script::op_yieldTask()");
	_scriptPaused = true;
}

void Script::op_jmp() {
	const uint16 off = _scriptPtr.fetchWord();
	debugC(kDebugScript, "Script::op_jmp(0x%02X)", off);
	_scriptPtr.pc = _res->_segCode + off;
}

void Script::op_installTask() {
	const uint8 i = _scriptPtr.fetchByte();
	const uint16 n = _scriptPtr.fetchWord();
	debugC(kDebugScript, "Script::op_installTask(0x%X, 0x%X)", i, n);
	assert(i < 0x40);
	_scriptTasks[1][i] = n;
}

void Script::op_jmpIfVar() {
	const uint8 i = _scriptPtr.fetchByte();
	debugC(kDebugScript, "Script::op_jmpIfVar(0x%02X)", i);
	--_scriptVars[i];
	if (_scriptVars[i] != 0) {
		op_jmp();
	} else {
		_scriptPtr.fetchWord();
	}
}

void Script::op_condJmp() {
	// Script patch the original interpreter triggers
	// if an incorrect code is entered at the start
	if (_res->_currentPart == kPartCopyProtection &&
			_res->_dataType == DT_DOS &&
			(_scriptPtr.pc - _res->_segCode) == 0xc4c) {
		byte *script = _scriptPtr.pc;
		*script = 0x81;
		WRITE_BE_UINT16(script + 3, 0xcb7);
		WRITE_BE_UINT16(script + 153, 0xced);
	}

	const uint8 op = _scriptPtr.fetchByte();
	const uint8 var = _scriptPtr.fetchByte();
	const int16 b = _scriptVars[var];
	int16 a;
	if (op & 0x80) {
		a = _scriptVars[_scriptPtr.fetchByte()];
	} else if (op & 0x40) {
		a = _scriptPtr.fetchWord();
	} else {
		a = _scriptPtr.fetchByte();
	}
	debugC(kDebugScript, "Script::op_condJmp(%d, 0x%02X, 0x%02X) var=0x%02X", op, b, a, var);
	bool expr = false;
	switch (op & 7) {
	case 0:
		expr = (b == a);

		if (!_res->_copyProtection) {
			if (_res->_currentPart == kPartCopyProtection) {
				//
				// 0CB8: jmpIf(VAR(0x29) == VAR(0x1E), @0CD3)
				// ...
				//
				if (var == 0x29 && (op & 0x80) != 0) {
					// 4 symbols
					_scriptVars[0x29] = _scriptVars[0x1E];
					_scriptVars[0x2A] = _scriptVars[0x1F];
					_scriptVars[0x2B] = _scriptVars[0x20];
					_scriptVars[0x2C] = _scriptVars[0x21];
					// counters
					_scriptVars[0x32] = 6;
					_scriptVars[0x64] = 20;
					warning("Script::op_condJmp() bypassing protection");
					expr = true;
				}
			}
		}
		break;
	case 1:
		expr = (b != a);
		break;
	case 2:
		expr = (b > a);
		break;
	case 3:
		expr = (b >= a);
		break;
	case 4:
		expr = (b < a);
		break;
	case 5:
		expr = (b <= a);
		break;
	default:
		warning("Script::op_condJmp() invalid condition %d", (op & 7));
		break;
	}
	if (expr) {
		op_jmp();
		if (!_is3DO && var == VAR_SCREEN_NUM && _screenNum != _scriptVars[VAR_SCREEN_NUM]) {
			fixUpPalette_changeScreen(_res->_currentPart, _scriptVars[VAR_SCREEN_NUM]);
			_screenNum = _scriptVars[VAR_SCREEN_NUM];
		}
	} else {
		_scriptPtr.fetchWord();
	}
}

void Script::op_setPalette() {
	const uint16 i = _scriptPtr.fetchWord();
	debugC(kDebugScript, "Script::op_changePalette(%d)", i);
	const int num = i >> 8;
	if (_vid->_graphics->_fixUpPalette == FIXUP_PALETTE_REDRAW) {
		if (_res->_currentPart == 16001) {
			if (num == 10 || num == 16) {
				return;
			}
		}
		_vid->_nextPal = num;
	} else {
		_vid->_nextPal = num;
	}
}

void Script::op_changeTasksState() {
	uint8 start = _scriptPtr.fetchByte();
	const uint8 end = _scriptPtr.fetchByte();
	if (end < start) {
		warning("Script::op_changeTasksState() ec=0x%X (end < start)", 0x880);
		return;
	}
	const uint8 state = _scriptPtr.fetchByte();

	debugC(kDebugScript, "Script::op_changeTasksState(%d, %d, %d)", start, end, state);

	if (state == 2) {
		for (; start <= end; ++start) {
			_scriptTasks[1][start] = 0xFFFE;
		}
	} else if (state < 2) {
		for (; start <= end; ++start) {
			_scriptStates[1][start] = state;
		}
	}
}

void Script::op_selectPage() {
	const uint8 i = _scriptPtr.fetchByte();
	debugC(kDebugScript, "Script::op_selectPage(%d)", i);
	_vid->setWorkPagePtr(i);
}

void Script::op_fillPage() {
	const uint8 i = _scriptPtr.fetchByte();
	const uint8 color = _scriptPtr.fetchByte();
	debugC(kDebugScript, "Script::op_fillPage(%d, %d)", i, color);
	_vid->fillPage(i, color);
}

void Script::op_copyPage() {
	const uint8 i = _scriptPtr.fetchByte();
	const uint8 j = _scriptPtr.fetchByte();
	debugC(kDebugScript, "Script::op_copyPage(%d, %d)", i, j);
	_vid->copyPage(i, j, _scriptVars[VAR_SCROLL_Y]);
}

void Script::op_updateDisplay() {
	const uint8 page = _scriptPtr.fetchByte();
	debugC(kDebugScript, "Script::op_updateDisplay(%d)", page);
	inp_handleSpecialKeys();

	if (_res->_copyProtection) {
		// entered protection symbols match the expected values
		if (_res->_currentPart == kPartCopyProtection && _scriptVars[0x67] == 1) {
			_scriptVars[0xDC] = 33;
		}
	}

	const int frameHz = _is3DO ? 60 : 50;
	if (!_fastMode && _scriptVars[VAR_PAUSE_SLICES] != 0) {
		const int delay = _stub->getTimeStamp() - _timeStamp;
		const int pause = _scriptVars[VAR_PAUSE_SLICES] * 1000 / frameHz - delay;
		if (pause > 0) {
			_stub->sleep(pause);
		}
	}
	_timeStamp = _stub->getTimeStamp();
	if (_is3DO) {
		_scriptVars[0xF7] = (_timeStamp - _startTime) * frameHz / 1000;
	} else {
		_scriptVars[0xF7] = 0;
	}

	_vid->_displayHead = !((_res->_currentPart == 16004 && _screenNum == 37) || (_res->_currentPart == 16006 && _screenNum == 202));
	_vid->updateDisplay(page, _stub);
}

void Script::op_removeTask() {
	debugC(kDebugScript, "Script::op_removeTask()");
	_scriptPtr.pc = _res->_segCode + 0xFFFF;
	_scriptPaused = true;
}

void Script::op_drawString() {
	const uint16 strId = _scriptPtr.fetchWord();
	const uint16 x = _scriptPtr.fetchByte();
	const uint16 y = _scriptPtr.fetchByte();
	const uint16 col = _scriptPtr.fetchByte();
	debugC(kDebugScript, "Script::op_drawString(0x%03X, %d, %d, %d)", strId, x, y, col);
	_vid->drawString(col, x, y, strId);
}

void Script::op_sub() {
	const uint8 i = _scriptPtr.fetchByte();
	const uint8 j = _scriptPtr.fetchByte();
	debugC(kDebugScript, "Script::op_sub(0x%02X, 0x%02X)", i, j);
	_scriptVars[i] -= _scriptVars[j];
}

void Script::op_and() {
	const uint8 i = _scriptPtr.fetchByte();
	const uint16 n = _scriptPtr.fetchWord();
	debugC(kDebugScript, "Script::op_and(0x%02X, %d)", i, n);
	_scriptVars[i] = (uint16)_scriptVars[i] & n;
}

void Script::op_or() {
	const uint8 i = _scriptPtr.fetchByte();
	const uint16 n = _scriptPtr.fetchWord();
	debugC(kDebugScript, "Script::op_or(0x%02X, %d)", i, n);
	_scriptVars[i] = (uint16)_scriptVars[i] | n;
}

void Script::op_shl() {
	const uint8 i = _scriptPtr.fetchByte();
	const uint16 n = _scriptPtr.fetchWord();
	debugC(kDebugScript, "Script::op_shl(0x%02X, %d)", i, n);
	_scriptVars[i] = (uint16)_scriptVars[i] << n;
}

void Script::op_shr() {
	const uint8 i = _scriptPtr.fetchByte();
	const uint16 n = _scriptPtr.fetchWord();
	debugC(kDebugScript, "Script::op_shr(0x%02X, %d)", i, n);
	_scriptVars[i] = (uint16)_scriptVars[i] >> n;
}

void Script::op_playSound() {
	const uint16 resNum = _scriptPtr.fetchWord();
	const uint8 freq = _scriptPtr.fetchByte();
	const uint8 vol = _scriptPtr.fetchByte();
	const uint8 channel = _scriptPtr.fetchByte();
	debugC(kDebugScript, "Script::op_playSound(0x%X, %d, %d, %d)", resNum, freq, vol, channel);
	snd_playSound(resNum, freq, vol, channel);
}

static void preloadSoundCb(void *userdata, int soundNum, const uint8 *data) {
	((Script *)userdata)->snd_preloadSound(soundNum, data);
}

void Script::op_updateResources() {
	const uint16 num = _scriptPtr.fetchWord();
	debugC(kDebugScript, "Script::op_updateResources(%d)", num);
	if (num == 0) {
		_ply->stop();
		_sound->stopAll();
		_res->invalidateRes();
	} else {
		_res->update(num, preloadSoundCb, this);
	}
}

void Script::op_playMusic() {
	const uint16 resNum = _scriptPtr.fetchWord();
	const uint16 delay = _scriptPtr.fetchWord();
	const uint8 pos = _scriptPtr.fetchByte();
	debugC(kDebugScript, "Script::op_playMusic(0x%X, %d, %d)", resNum, delay, pos);
	snd_playMusic(resNum, delay, pos);
}

void Script::restartAt(int part, int pos) {
	_ply->stop();
	_sound->stopAll();
	if (_res->getDataType() == DT_20TH_EDITION) {
		_scriptVars[0xBF] = _difficulty; // difficulty (0 to 2)
		// _scriptVars[0xDB] = 1; // preload sounds (resnum >= 2000)
		_scriptVars[0xDE] = _useRemasteredAudio ? 1 : 0; // playback remastered sounds (resnum >= 146)
	}
	if (_res->getDataType() == DT_DOS && part == kPartCopyProtection) {
		// VAR(0x54) indicates if the "Out of this World" title screen should be presented
		//
		//   0084: jmpIf(VAR(0x54) < 128, @00C4)
		//   ..
		//   008D: setPalette(num=0)
		//   0090: updateResources(res=18)
		//   ...
		//   00C4: setPalette(num=23)
		//   00CA: updateResources(res=71)

		// Use "Out of this World" title screen if playing the USA release
		const bool awTitleScreen = !(_res->_lang == Common::EN_USA);
		_scriptVars[0x54] = awTitleScreen ? 0x1 : 0x81;
	}
	_res->setupPart(part);
	memset(_scriptTasks, 0xFF, sizeof(_scriptTasks));
	memset(_scriptStates, 0, sizeof(_scriptStates));
	_scriptTasks[0][0] = 0;
	_screenNum = -1;
	if (pos >= 0) {
		_scriptVars[0] = pos;
	}
	_startTime = _timeStamp = _stub->getTimeStamp();
	if (part == kPartWater) {
		if (_res->_demo3Joy.start()) {
			memset(_scriptVars, 0, sizeof(_scriptVars));
		}
	}
}

void Script::setupTasks() {
	if (_res->_nextPart != 0) {
		restartAt(_res->_nextPart);
		_res->_nextPart = 0;
	}
	for (int i = 0; i < 0x40; ++i) {
		_scriptStates[0][i] = _scriptStates[1][i];
		const uint16 n = _scriptTasks[1][i];
		if (n != 0xFFFF) {
			_scriptTasks[0][i] = (n == 0xFFFE) ? 0xFFFF : n;
			_scriptTasks[1][i] = 0xFFFF;
		}
	}
}

void Script::runTasks() {
	for (int i = 0; i < 0x40 && !_stub->_pi.quit; ++i) {
		if (_scriptStates[0][i] == 0) {
			const uint16 n = _scriptTasks[0][i];
			if (n != 0xFFFF) {
				_scriptPtr.pc = _res->_segCode + n;
				_stackPtr = 0;
				_scriptPaused = false;
				debugC(kDebugScript, "Script::runTasks() i=0x%02X n=0x%02X", i, n);
				executeTask();
				_scriptTasks[0][i] = _scriptPtr.pc - _res->_segCode;
				debugC(kDebugScript, "Script::runTasks() i=0x%02X pos=0x%X", i, _scriptTasks[0][i]);
			}
		}
	}
}

void Script::executeTask() {
	while (!_scriptPaused) {
		const uint8 opcode = _scriptPtr.fetchByte();
		if (opcode & 0x80) {
			const uint16 off = ((opcode << 8) | _scriptPtr.fetchByte()) << 1;
			_res->_useSegVideo2 = false;
			Point pt;
			pt.x = _scriptPtr.fetchByte();
			pt.y = _scriptPtr.fetchByte();
			const int16 h = pt.y - 199;
			if (h > 0) {
				pt.y = 199;
				pt.x += h;
			}
			debugC(kDebugVideo, "vid_opcd_0x80 : opcode=0x%X off=0x%X x=%d y=%d", opcode, off, pt.x, pt.y);
			_vid->setDataBuffer(_res->_segVideo1, off);
			if (_is3DO) {
				_vid->drawShape3DO(0xFF, 64, &pt);
			} else {
				_vid->drawShape(0xFF, 64, &pt);
			}
		} else if (opcode & 0x40) {
			Point pt;
			const uint8 offsetHi = _scriptPtr.fetchByte();
			const uint16 off = ((offsetHi << 8) | _scriptPtr.fetchByte()) << 1;
			pt.x = _scriptPtr.fetchByte();
			_res->_useSegVideo2 = false;
			if (!(opcode & 0x20)) {
				if (!(opcode & 0x10)) {
					pt.x = (pt.x << 8) | _scriptPtr.fetchByte();
				} else {
					pt.x = _scriptVars[pt.x];
				}
			} else {
				if (opcode & 0x10) {
					pt.x += 0x100;
				}
			}
			pt.y = _scriptPtr.fetchByte();
			if (!(opcode & 8)) {
				if (!(opcode & 4)) {
					pt.y = (pt.y << 8) | _scriptPtr.fetchByte();
				} else {
					pt.y = _scriptVars[pt.y];
				}
			}
			uint16 zoom = 64;
			if (!(opcode & 2)) {
				if (opcode & 1) {
					zoom = _scriptVars[_scriptPtr.fetchByte()];
				}
			} else {
				if (opcode & 1) {
					_res->_useSegVideo2 = true;
				} else {
					zoom = _scriptPtr.fetchByte();
				}
			}
			debugC(kDebugVideo, "vid_opcd_0x40 : off=0x%X x=%d y=%d", off, pt.x, pt.y);
			_vid->setDataBuffer(_res->_useSegVideo2 ? _res->_segVideo2 : _res->_segVideo1, off);
			if (_is3DO) {
				_vid->drawShape3DO(0xFF, zoom, &pt);
			} else {
				_vid->drawShape(0xFF, zoom, &pt);
			}
		} else {
			if (_is3DO) {
				switch (opcode) {
				case 11:
				{
					const int num = _scriptPtr.fetchByte();
					debugC(kDebugScript, "Script::op11() setPalette %d", num);
					_vid->changePal(num);
				}
				continue;
				case 22:
				{
					const int var = _scriptPtr.fetchByte();
					const int shift = _scriptPtr.fetchByte();
					debugC(kDebugScript, "Script::op22() VAR(0x%02X) <<= %d", var, shift);
					_scriptVars[var] = (uint16)_scriptVars[var] << shift;
				}
				continue;
				case 23:
				{
					const int var = _scriptPtr.fetchByte();
					const int shift = _scriptPtr.fetchByte();
					debugC(kDebugScript, "Script::op23() VAR(0x%02X) >>= %d", var, shift);
					_scriptVars[var] = (uint16)_scriptVars[var] >> shift;
				}
				continue;
				case 26:
				{
					const int num = _scriptPtr.fetchByte();
					debugC(kDebugScript, "Script::op26() playMusic %d", num);
					snd_playMusic(num, 0, 0);
				}
				continue;
				case 27:
				{
					const int num = _scriptPtr.fetchWord();
					const int x = _scriptVars[_scriptPtr.fetchByte()];
					const int y = _scriptVars[_scriptPtr.fetchByte()];
					const int color = _scriptPtr.fetchByte();
					_vid->drawString(color, x, y, num);
				}
				continue;
				case 28:
				{
					const uint8 var = _scriptPtr.fetchByte();
					debugC(kDebugScript, "Script::op28() jmpIf(VAR(0x%02x) == 0)", var);
					if (_scriptVars[var] == 0) {
						op_jmp();
					} else {
						_scriptPtr.fetchWord();
					}
				}
				continue;
				case 29:
				{
					const uint8 var = _scriptPtr.fetchByte();
					debugC(kDebugScript, "Script::op29() jmpIf(VAR(0x%02x) != 0)", var);
					if (_scriptVars[var] != 0) {
						op_jmp();
					} else {
						_scriptPtr.fetchWord();
					}
				}
				continue;
				case 30:
				{
					::debug("Time = %d", _scriptVars[0xF7]);
				}
				continue;

			default:
					break;
				}
			}
			if (opcode > 0x1A)
				error("Script::executeTask() ec=0x%X invalid opcode=0x%X", 0xFFF, opcode);

			(this->*OPCODE_TABLE[opcode])();
		}
	}
}

void Script::updateInput() {
	_stub->processEvents();
	if (_res->_currentPart == kPartPassword) {
		const char c = _stub->_pi.lastChar;
		if (c == 8 || /*c == 0xD ||*/ c == 0 || (c >= 'a' && c <= 'z')) {
			_scriptVars[VAR_LAST_KEYCHAR] = c & ~0x20;
			_stub->_pi.lastChar = 0;
		}
	}
	int16 lr = 0;
	int16 m = 0;
	int16 ud = 0;
	int16 jd = 0;
	if (_stub->_pi.dirMask & PlayerInput::DIR_RIGHT) {
		lr = 1;
		m |= 1;
	}
	if (_stub->_pi.dirMask & PlayerInput::DIR_LEFT) {
		lr = -1;
		m |= 2;
	}
	if (_stub->_pi.dirMask & PlayerInput::DIR_DOWN) {
		ud = jd = 1;
		m |= 4; // crouch
	}
	if (_is3DO) { // This could be enabled to any later version than Amiga, Atari and DOS demo
		if (_stub->_pi.dirMask & PlayerInput::DIR_UP) {
			ud = -1;
		}
		if (_stub->_pi.jump) {
			jd = -1;
			m |= 8; // jump
		}
	} else {
		if (_stub->_pi.dirMask & PlayerInput::DIR_UP) {
			ud = jd = -1;
			m |= 8; // jump
		}
	}
	if (!(_res->getDataType() == DT_AMIGA || _res->getDataType() == DT_ATARI)) {
		_scriptVars[VAR_HERO_POS_UP_DOWN] = ud;
	}
	_scriptVars[VAR_HERO_POS_JUMP_DOWN] = jd;
	_scriptVars[VAR_HERO_POS_LEFT_RIGHT] = lr;
	_scriptVars[VAR_HERO_POS_MASK] = m;
	int16 action = 0;
	if (_stub->_pi.action) {
		action = 1;
		m |= 0x80;
	}
	_scriptVars[VAR_HERO_ACTION] = action;
	_scriptVars[VAR_HERO_ACTION_POS_MASK] = m;
	if (_res->_currentPart == kPartWater) {
		const uint8 mask = _res->_demo3Joy.update();
		if (mask != 0) {
			_scriptVars[VAR_HERO_ACTION_POS_MASK] = mask;
			_scriptVars[VAR_HERO_POS_MASK] = mask & 15;
			_scriptVars[VAR_HERO_POS_LEFT_RIGHT] = 0;
			if (mask & 1) {
				_scriptVars[VAR_HERO_POS_LEFT_RIGHT] = 1;
			}
			if (mask & 2) {
				_scriptVars[VAR_HERO_POS_LEFT_RIGHT] = -1;
			}
			_scriptVars[VAR_HERO_POS_JUMP_DOWN] = 0;
			if (mask & 4) {
				_scriptVars[VAR_HERO_POS_JUMP_DOWN] = 1;
			}
			if (mask & 8) {
				_scriptVars[VAR_HERO_POS_JUMP_DOWN] = -1;
			}
			_scriptVars[VAR_HERO_ACTION] = (mask >> 7);
		}
	}
}

void Script::inp_handleSpecialKeys() {
	if (_stub->_pi.pause) {
		if (_res->_currentPart != kPartCopyProtection && _res->_currentPart != kPartIntro) {
			_stub->_pi.pause = false;
			if (_is3DO) {
				_vid->drawBitmap3DO("PauseShape", _stub);
			}
			while (!_stub->_pi.pause && !_stub->_pi.quit) {
				_stub->processEvents();
				_stub->sleep(50);
			}
		}
		_stub->_pi.pause = false;
	}
	if (_stub->_pi.back) {
		_stub->_pi.back = false;
		if (_is3DO) {
			static const char *names[] = { "EndShape1", "EndShape2" };
			int current = 0;
			_vid->drawBitmap3DO(names[current], _stub);
			while (!_stub->_pi.quit) {
				_stub->processEvents();
				_stub->sleep(50);
				if (_stub->_pi.dirMask & PlayerInput::DIR_LEFT) {
					_stub->_pi.dirMask &= ~PlayerInput::DIR_LEFT;
					if (current != 0) {
						current = 0;
						_vid->drawBitmap3DO(names[current], _stub);
					}
				}
				if (_stub->_pi.dirMask & PlayerInput::DIR_RIGHT) {
					_stub->_pi.dirMask &= ~PlayerInput::DIR_RIGHT;
					if (current != 1) {
						current = 1;
						_vid->drawBitmap3DO(names[current], _stub);
					}
				}
				if (_stub->_pi.action) {
					_stub->_pi.action = false;
					if (current == 0) {
						_res->_nextPart = 16000;
					}
					break;
				}
			}
		}
	}
	if (_stub->_pi.code) {
		_stub->_pi.code = false;
		if (_res->_hasPasswordScreen) {
			if (_res->_currentPart != kPartPassword && _res->_currentPart != kPartCopyProtection) {
				_res->_nextPart = kPartPassword;
			}
		}
	}
}

static uint8 getWavLooping(uint16 resNum) {
	switch (resNum) {
	case 1:
	case 3:
	case 8:
	case 16:
	case 89:
	case 97:
	case 102:
	case 104:
	case 106:
	case 132:
	case 139:
		return 1;

	default:
		break;
	}
	return 0;
}

static int getSoundFreq(uint8 period) {
	if (period > 39) {
		warning("Script::getSoundFreq() invalid period %d", period);
		period = 39;
	}
	return kPaulaFreq / (Script::PERIOD_TABLE[period] * 2);
}

void Script::snd_playSound(uint16 resNum, uint8 freq, uint8 vol, uint8 channel) {
	debugC(kDebugSound, "snd_playSound(0x%X, %d, %d, %d)", resNum, freq, vol, channel);
	if (vol == 0) {
		_sound->stopSound(channel);
		return;
	}
	if (vol > 63) {
		vol = 63;
	}
	if (freq > 39) {
		freq = 39;
	}
	channel &= 3;
	switch (_res->getDataType()) {
	case DT_20TH_EDITION:
		if (freq != 0) {
			--freq;
		}
		// fall-through

	case DT_15TH_EDITION:
		if (freq >= 32) {
			// Anniversary editions do not have the 170 period
			//
			//  [31] dos=19886 20th=19886 amiga=19886 (period 180)
			//  [32] dos=21056 20th=22372 amiga=21056 (period 170)
			//  [33] dos=22372 20th=23704 amiga=22372 (period 160)
			++freq;
		}
		// fall-through
	case DT_WIN31: {
		uint32 size = 0;
		uint8 *buf = _res->loadWav(resNum, &size);
		if (buf) {
			_sound->playSoundWav(channel, buf, size,
				getSoundFreq(freq), vol, getWavLooping(resNum));
		}
		break;
	}
	case DT_3DO:
		_sound->playSoundAiff(channel, resNum, vol);
		break;
	case DT_AMIGA:
	case DT_ATARI:
	case DT_ATARI_DEMO:
	case DT_DOS: {
		MemEntry *me = &_res->_memList[resNum];
		if (me->status == Resource::STATUS_LOADED) {
			_sound->playSoundRaw(channel, me->bufPtr, me->unpackedSize, getSoundFreq(freq), vol);
		}
		break;
	}

	default:
		break;
	}
}

void Script::snd_playMusic(uint16 resNum, uint16 delay, uint8 pos) {
	debugC(kDebugSound, "snd_playMusic(0x%X, %d, %d)", resNum, delay, pos);
	uint8 loop = 0;

	switch (_res->getDataType()) {
	case DT_20TH_EDITION:
		if (resNum == 5000) {
			_sound->stopMusic();
			break;
		}
		if (resNum >= 5001 && resNum <= 5010) {
			loop = 1;
		}
		// fall-through
	case DT_15TH_EDITION:
	case DT_WIN31:
		if (resNum != 0) {
			char path[MAXPATHLEN];
			const char *p = _res->getMusicPath(resNum, path, sizeof(path));
			if (p) {
				_sound->playMusic(p, loop);
			}
		}
		break;
	case DT_3DO:
		if (resNum == 0) {
			_sound->stopAifcMusic();
		} else {
			uint32 offset = 0;
			char path[MAXPATHLEN];
			const char *p = _res->getMusicPath(resNum, path, sizeof(path), &offset);
			if (p) {
				_sound->playAifcMusic(p, offset);
			}
		}
		break;
	default: // DT_AMIGA, DT_ATARI, DT_DOS
		if (resNum != 0) {
			_ply->loadSfxModule(resNum, delay, pos);
			_ply->start();
			_sound->playSfxMusic(resNum);
		} else if (delay != 0) {
			_ply->setEventsDelay(delay);
		} else {
			_sound->stopSfxMusic();
		}
		break;
	}
}

void Script::snd_preloadSound(uint16 resNum, const uint8 *data) {
	if (_res->getDataType() == DT_3DO) {
		_sound->preloadSoundAiff(resNum, data);
	}
}

void Script::fixUpPalette_changeScreen(int part, int screen) {
	int pal = -1;
	switch (part) {
	case 16004:
		if (screen == 0x47) { // bitmap resource #68
			pal = 8;
		}
		break;

	case 16006:
		if (screen == 0x4A) { // bitmap resources #144, #145
			pal = 1;
		}
		break;

	default:
		break;
	}
	if (pal != -1) {
		debugC(kDebugScript, "Setting palette %d for part %d screen %d", pal, part, screen);
		_vid->changePal(pal);
	}
}

} // namespace Awe
