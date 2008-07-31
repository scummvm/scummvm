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
#include "kyra/sound.h"
#include "kyra/wsamovie.h"

#include "common/endian.h"

namespace Kyra {

TIMInterpreter::TIMInterpreter(KyraEngine_v1 *vm, Screen_v2 *screen, OSystem *system) : _vm(vm), _screen(screen), _system(system), _currentTim(0) {
#define COMMAND(x) { &TIMInterpreter::x, #x }
#define COMMAND_UNIMPL() { 0, 0 }
#define cmd_return(n) cmd_return_##n
	static const CommandEntry commandProcs[] = {
		// 0x00
		COMMAND(cmd_initFunc0),
		COMMAND(cmd_stopCurFunc),
		COMMAND(cmd_initWSA),
		COMMAND(cmd_uninitWSA),
		// 0x04
		COMMAND(cmd_initFunc),
		COMMAND(cmd_stopFunc),
		COMMAND(cmd_wsaDisplayFrame),
		COMMAND_UNIMPL(),
		// 0x08
		COMMAND(cmd_loadVocFile),
		COMMAND(cmd_unloadVocFile),
		COMMAND(cmd_playVocFile),
		COMMAND_UNIMPL(),
		// 0x0C
		COMMAND(cmd_loadSoundFile),
		COMMAND(cmd_return(1)),
		COMMAND(cmd_playMusicTrack),
		COMMAND_UNIMPL(),
		// 0x10
		COMMAND(cmd_return(1)),
		COMMAND(cmd_return(1)),
		COMMAND_UNIMPL(),
		COMMAND_UNIMPL(),
		// 0x14
		COMMAND(cmd_setLoopIp),
		COMMAND(cmd_continueLoop),
		COMMAND(cmd_resetLoopIp),
		COMMAND(cmd_resetAllRuntimes),
		// 0x18
		COMMAND(cmd_return(1)),
		COMMAND(cmd_execOpcode),
		COMMAND(cmd_initFuncNow),
		COMMAND(cmd_stopFuncNow),
		// 0x1C
		COMMAND(cmd_return(1)),
		COMMAND(cmd_return(1)),
		COMMAND(cmd_return(n1))
	};
#undef cmd_return

	_commands = commandProcs;
	_commandsSize = ARRAYSIZE(commandProcs);
	
	memset(&_animations, 0, sizeof(_animations));
	_langData = 0;
	_textDisplayed = false;
	_textAreaBuffer = new uint8[320*40];
	assert(_textAreaBuffer);
	
	_palDelayInc = _palDiff = _palDelayAcc = 0;
}

TIMInterpreter::~TIMInterpreter() {
	delete[] _langData;
	delete[] _textAreaBuffer;
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

	strncpy(tim->filename, filename, 13);

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

void TIMInterpreter::setLangData(const char *filename) {
	delete[] _langData;
	_langData = _vm->resource()->fileData(filename, 0);
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

				case 22:
					cur.loopIp = 0;
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

void TIMInterpreter::displayText(uint16 textId, int16 flags) {
	char *text = getTableEntry(textId);

	if (_textDisplayed) {
		_screen->copyBlockToPage(0, 0, 160, 320, 40, _textAreaBuffer);
		_textDisplayed = false;
	}

	if (!text)
		return;
	if (!text[0])
		return;

	char filename[16];
	memset(filename, 0, sizeof(filename));

	if (text[0] == '$') {
		const char *end = strchr(text+1, '$');
		if (end)
			memcpy(filename, text+1, end-1-text);
	}

	if (filename[0])
		_vm->sound()->voicePlay(filename);

	if (text[0] == '$')
		text = strchr(text + 1, '$') + 1;

	setupTextPalette((flags < 0) ? 1 : flags, 0);

	if (flags < 0) {
		static const uint8 colorMap[] = { 0x00, 0xF0, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

		_screen->setFont(Screen::FID_8_FNT);
		_screen->setTextColorMap(colorMap);
		_screen->_charWidth = -2;
	}

	_screen->_charOffset = -4;
	_screen->copyRegionToBuffer(0, 0, 160, 320, 40, _textAreaBuffer);
	_textDisplayed = true;

	char backupChar = 0;
	char *str = text;
	int heightAdd = 0;

	while (str[0]) {
		char *nextLine = strchr(str, '\r');

		backupChar = 0;
		if (nextLine) {
			backupChar = nextLine[0];
			nextLine[0] = '\0';
		}

		int width = _screen->getTextWidth(str);

		if (flags >= 0)
			_screen->printText(str, (320 - width) >> 1, 160 + heightAdd, 0xF0, 0x00);
		else
			_screen->printText(str, (320 - width) >> 1, 188, 0xF0, 0x00);

		heightAdd += _screen->getFontHeight();
		str += strlen(str);

		if (backupChar) {
			nextLine[0] = backupChar;
			++str;
		}
	}

	_screen->_charOffset = 0;

	if (flags < 0) {
		static const uint8 colorMap[] = { 0x00, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0x00, 0x00, 0x00, 0x00 };

		_screen->setFont(Screen::FID_INTRO_FNT);
		_screen->setTextColorMap(colorMap);
		_screen->_charWidth = 0;
	}
}

void TIMInterpreter::setupTextPalette(uint index, int fadePalette) {
	static const uint16 palTable[] = {
		0x00, 0x00, 0x00,
		0x64, 0x64, 0x64,
		0x61, 0x51, 0x30,
		0x29, 0x48, 0x64,
		0x00, 0x4B, 0x3B,
		0x64, 0x1E, 0x1E,
	};

	for (int i = 0; i < 15; ++i) {
		uint8 *palette = _screen->getPalette(0) + (240 + i) * 3;

		uint8 c1 = (((15 - i) << 2) * palTable[index*3+0]) / 100;
		uint8 c2 = (((15 - i) << 2) * palTable[index*3+1]) / 100;
		uint8 c3 = (((15 - i) << 2) * palTable[index*3+2]) / 100;

		palette[0] = c1;
		palette[1] = c2;
		palette[2] = c3;
	}
	
	if (!fadePalette && !_palDiff) {
		_screen->setScreenPalette(_screen->getPalette(0));
	} else {
		_screen->getFadeParams(_screen->getPalette(0), fadePalette, _palDelayInc, _palDiff);
		_palDelayAcc = 0;
	}
}

TIMInterpreter::Animation *TIMInterpreter::initAnimStruct(int index, const char *filename, int x, int y, int, int offscreenBuffer, uint16 wsaFlags) {
	Animation *anim = &_animations[index];
	anim->x = x;
	anim->y = y;
	anim->wsaCopyParams = wsaFlags;

	uint16 wsaOpenFlags = ((wsaFlags & 0x10) != 0) ? 2 : 0;

	char file[32];
	snprintf(file, 32, "%s.WSA", filename);
	
	if (_vm->resource()->exists(file)) {
		anim->wsa = new WSAMovie_v2(_vm, _screen);
		assert(anim->wsa);

		anim->wsa->open(file, wsaOpenFlags, (index == 1) ? _screen->getPalette(0) : 0);
	}
	
	if (anim->wsa && anim->wsa->opened()) {
		if (x == -1)
			anim->x = x = 0;
		if (y == -1)
			anim->y = y = 0;

		if (wsaFlags & 2) {
			_screen->fadePalette(_screen->getPalette(1), 15, 0);
			_screen->clearPage(8);
			_screen->checkedPageUpdate(8, 4);
			_screen->updateScreen();
		}

		if (wsaFlags & 4) {
			snprintf(file, 32, "%s.CPS", filename);

			if (_vm->resource()->exists(file)) {
				_screen->loadBitmap(file, 3, 3, _screen->getPalette(0));
				_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 8, Screen::CR_NO_P_CHECK);
				_screen->checkedPageUpdate(8, 4);
				_screen->updateScreen();
			}

			anim->wsa->setX(x);
			anim->wsa->setY(y);
			anim->wsa->setDrawPage(0);
			anim->wsa->displayFrame(0, 0, 0, 0);
		} 

		if (wsaFlags & 2)
			_screen->fadePalette(_screen->getPalette(0), 30, 0);
	} else {
		if (wsaFlags & 2) {
			_screen->fadePalette(_screen->getPalette(1), 15, 0);
			_screen->clearPage(8);
			_screen->checkedPageUpdate(8, 4);
			_screen->updateScreen();
		}

		snprintf(file, 32, "%s.CPS", filename);

		if (_vm->resource()->exists(file)) {
			_screen->loadBitmap(file, 3, 3, _screen->getPalette(0));
			_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 8, Screen::CR_NO_P_CHECK);
			_screen->checkedPageUpdate(8, 4);
			_screen->updateScreen();
		}

		if (wsaFlags & 2)
			_screen->fadePalette(_screen->getPalette(0), 30, 0);
	}
	
	return anim;
}

char *TIMInterpreter::getTableEntry(uint idx) {
	if (!_langData)
		return 0;
	else
		return (char *)(_langData + READ_LE_UINT16(_langData + (idx<<1)));
}

const char *TIMInterpreter::getCTableEntry(uint idx) const {
	if (!_langData)
		return 0;
	else
		return (const char *)(_langData + READ_LE_UINT16(_langData + (idx<<1)));
}

int TIMInterpreter::execCommand(int cmd, const uint16 *param) {
	if (cmd < 0 || cmd >= _commandsSize) {
		warning("Calling unimplemented TIM command %d from file '%s'", cmd, _currentTim->filename);
		return 0;
	}

	if (_commands[cmd].proc == 0) {
		warning("Calling unimplemented TIM command %d from file '%s'", cmd, _currentTim->filename);
		return 0;
	}

	debugC(5, kDebugLevelScript, "TIMInterpreter::%s(%p)", _commands[cmd].desc, (const void* )param);
	return (this->*_commands[cmd].proc)(param);
}

int TIMInterpreter::cmd_initFunc0(const uint16 *param) {
	for (int i = 0; i < TIM::kWSASlots; ++i)
		memset(&_currentTim->wsa[i], 0, sizeof(TIM::WSASlot));

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

int TIMInterpreter::cmd_initWSA(const uint16 *param) {
	const int index = param[0];

	TIM::WSASlot &slot = _currentTim->wsa[index];
	
	slot.x = int16(param[2]);
	slot.y = int16(param[3]);
	slot.offscreen = param[4];
	slot.wsaFlags = param[5];
	const char *filename = (const char *)(_currentTim->text + READ_LE_UINT16(_currentTim->text + (param[1]<<1)));
	
	slot.anim = initAnimStruct(index, filename, slot.x, slot.y, 10, slot.offscreen, slot.wsaFlags);
	return 1;
}

int TIMInterpreter::cmd_uninitWSA(const uint16 *param) {
	const int index = param[0];

	TIM::WSASlot &slot = _currentTim->wsa[index];
	
	if (!slot.anim)
		return 0;
	
	Animation &anim = _animations[index];
	
	if (slot.offscreen) {
		delete anim.wsa;
		anim.wsa = 0;
		slot.anim = 0;
	} else {
		//XXX

		delete anim.wsa;
		memset(&anim, 0, sizeof(Animation));
		memset(&slot, 0, sizeof(TIM::WSASlot));
	}
	
	return 1;
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

int TIMInterpreter::cmd_wsaDisplayFrame(const uint16 *param) {
	Animation &anim = _animations[param[0]];
	const int frame = param[1];
	
	anim.wsa->setX(anim.x);
	anim.wsa->setY(anim.y);
	anim.wsa->setDrawPage((anim.wsaCopyParams & 0x4000) != 0 ? 2 : 8);
	anim.wsa->displayFrame(frame, anim.wsaCopyParams & 0xF0FF, 0, 0);
	return 1;
}

int TIMInterpreter::cmd_displayText(const uint16 *param) {
	displayText(param[0], param[1]);
	return 1;
}

int TIMInterpreter::cmd_loadVocFile(const uint16 *param) {
	const int stringId = param[0];
	const int index = param[1];

	_vocFiles[index] = (const char *)(_currentTim->text + READ_LE_UINT16(_currentTim->text + (stringId << 1)));
	for (int i = 0; i < 4; ++i)
		_vocFiles[index].deleteLastChar();
	return 1;
}

int TIMInterpreter::cmd_unloadVocFile(const uint16 *param) {
	const int index = param[0];
	_vocFiles[index].clear();
	return 1;
}

int TIMInterpreter::cmd_playVocFile(const uint16 *param) {
	const int index = param[0];
	const int volume = (param[1] * 255) / 100;

	if (index < ARRAYSIZE(_vocFiles) && !_vocFiles[index].empty())
		_vm->sound()->voicePlay(_vocFiles[index].c_str()/*, volume*/, true);
	else
		_vm->snd_playSoundEffect(index, volume);
	
	return 1;
}

int TIMInterpreter::cmd_loadSoundFile(const uint16 *param) {
	const char *file = (const char *)(_currentTim->text + READ_LE_UINT16(_currentTim->text + (param[0]<<1)));
	
	static char * fileList[] = { 0 };
	fileList[0] = _audioFilename;
	static AudioDataStruct audioList = { fileList, 1, 0, 0 };

	strncpy(_audioFilename, file, sizeof(_audioFilename));

	_vm->sound()->setSoundList(&audioList);
	_vm->sound()->loadSoundFile(0);
	return 1;
}

int TIMInterpreter::cmd_playMusicTrack(const uint16 *param) {
	_vm->sound()->playTrack(param[0]);
	return 1;
}

int TIMInterpreter::cmd_setLoopIp(const uint16 *param) {
	_currentTim->func[_currentFunc].loopIp = _currentTim->func[_currentFunc].ip;
	return 1;
}

int TIMInterpreter::cmd_continueLoop(const uint16 *param) {
	TIM::Function &func = _currentTim->func[_currentFunc];
	
	if (!func.loopIp)
		return -2;

	func.ip = func.loopIp;

	uint16 factor = param[0];
	if (factor) {
		const uint32 random = _vm->_rnd.getRandomNumberRng(0, 0x8000);
		uint32 waitTime = (random * factor) / 0x8000;
		func.nextTime += waitTime * _vm->tickLength();
	}
	
	return 1;
}

int TIMInterpreter::cmd_resetLoopIp(const uint16 *param) {
	_currentTim->func[_currentFunc].loopIp = 0;
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
	const uint16 opcode = *param++;

	if (!_currentTim->opcodes) {
		warning("Trying to execute TIM opcode %d without opcode list (file '%s')", opcode, _currentTim->filename);
		fflush(stderr); fflush(stdout);
		return 0;
	}

	if (opcode > _currentTim->opcodes->size()) {
		warning("Calling unimplemented TIM opcode(0x%.02X/%d) from file '%s'", opcode, opcode, _currentTim->filename);
		return 0;
	}

	if (!(*_currentTim->opcodes)[opcode]->isValid()) {
		warning("Calling unimplemented TIM opcode(0x%.02X/%d) from file '%s'", opcode, opcode, _currentTim->filename);
		fflush(stderr);
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

