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

#ifndef AWE_SCRIPT_H
#define AWE_SCRIPT_H

#include "audio/mixer.h"
#include "awe/intern.h"

namespace Awe {

class Sound;
struct Resource;
struct SfxPlayer;
struct SystemStub;
struct Video;

enum Difficulty {
	DIFFICULTY_EASY = 0,
	DIFFICULTY_NORMAL = 1,
	DIFFICULTY_HARD = 2
};

struct Script {
	typedef void (Script:: *OpcodeStub)();

	enum ScriptVars {
		VAR_RANDOM_SEED = 0x3C,

		VAR_SCREEN_NUM = 0x67,

		VAR_LAST_KEYCHAR = 0xDA,

		VAR_HERO_POS_UP_DOWN = 0xE5,

		VAR_MUSIC_SYNC = 0xF4,

		VAR_SCROLL_Y = 0xF9,
		VAR_HERO_ACTION = 0xFA,
		VAR_HERO_POS_JUMP_DOWN = 0xFB,
		VAR_HERO_POS_LEFT_RIGHT = 0xFC,
		VAR_HERO_POS_MASK = 0xFD,
		VAR_HERO_ACTION_POS_MASK = 0xFE,
		VAR_PAUSE_SLICES = 0xFF
	};

	static const OpcodeStub OPCODE_TABLE[];
	static const uint16 PERIOD_TABLE[];
	static Difficulty _difficulty;
	static bool _useRemasteredAudio;

	Sound *_sound;
	Resource *_res;
	SfxPlayer *_ply;
	Video *_vid;
	SystemStub *_stub = nullptr;

	int16 _scriptVars[256] = { 0 };
	uint16 _scriptStackCalls[64] = { 0 };
	uint16 _scriptTasks[2][64] = { { 0 } };
	uint8 _scriptStates[2][64] = { { 0 } };
	Ptr _scriptPtr;
	uint8 _stackPtr = 0;
	bool _scriptPaused = false;
	bool _fastMode = false;
	int _screenNum = 0;
	bool _is3DO = false;
	uint32 _startTime = 0, _timeStamp = 0;

	Script(Sound *snd, Resource *res, SfxPlayer *ply, Video *vid);
	void init();

	void op_movConst();
	void op_mov();
	void op_add();
	void op_addConst();
	void op_call();
	void op_ret();
	void op_yieldTask();
	void op_jmp();
	void op_installTask();
	void op_jmpIfVar();
	void op_condJmp();
	void op_setPalette();
	void op_changeTasksState();
	void op_selectPage();
	void op_fillPage();
	void op_copyPage();
	void op_updateDisplay();
	void op_removeTask();
	void op_drawString();
	void op_sub();
	void op_and();
	void op_or();
	void op_shl();
	void op_shr();
	void op_playSound();
	void op_updateResources();
	void op_playMusic();

	void restartAt(int part, int pos = -1);
	void setupPart(int num);
	void setupTasks();
	void runTasks();
	void executeTask();

	void updateInput();
	void inp_handleSpecialKeys();

	void snd_playSound(uint16 resNum, uint8 freq, uint8 vol, uint8 channel);
	void snd_playMusic(uint16 resNum, uint16 delay, uint8 pos);
	void snd_preloadSound(uint16 resNum, const uint8 *data);

	void fixUpPalette_changeScreen(int part, int screen);
};

} // namespace Awe

#endif
