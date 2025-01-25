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

#ifndef GOT_VARS_H
#define GOT_VARS_H

#include "common/events.h"
#include "common/queue.h"
#include "got/data/defines.h"
#include "got/data/highscores.h"
#include "got/data/level.h"
#include "got/data/sd_data.h"
#include "got/data/setup.h"
#include "got/data/thorinfo.h"
#include "got/game/script.h"
#include "got/gfx/font.h"
#include "got/gfx/gfx_chunks.h"
#include "got/gfx/gfx_pics.h"
#include "got/metaengine.h"
#include "got/sound.h"
#include "graphics/screen.h"

namespace Got {

#define TILE_SIZE 16
#define TILES_X (320 / TILE_SIZE)
#define TILES_Y (192 / TILE_SIZE)
#define TILES_COUNT (TILES_X * TILES_Y)

class Vars;

extern Vars *g_vars;

enum Key {
	key_none = KEYBIND_NONE,
	key_up = KEYBIND_UP,
	key_down = KEYBIND_DOWN,
	key_left = KEYBIND_LEFT,
	key_right = KEYBIND_RIGHT,
	key_fire = KEYBIND_FIRE,
	key_magic = KEYBIND_MAGIC,
	key_select = KEYBIND_SELECT
};

enum GameMode {
	MODE_NORMAL,
	MODE_AREA_CHANGE,
	MODE_THUNDER,
	MODE_THOR_DIES,
	MODE_ADD_SCORE,
	MODE_LIGHTNING,
	MODE_PAUSE,
	MODE_SCORE_INV
};

enum TransitionDir {
	DIR_LEFT,
	DIR_RIGHT,
	DIR_UP,
	DIR_DOWN,
	DIR_PHASED
};

struct Cheats {
	bool _freezeHealth = false;
	bool _freezeMagic = false;
	bool _freezeJewels = false;
};

class Vars {
public:
	Vars();
	~Vars();

	void load();
	void setArea(int areaNum);
	void clearKeyFlags();
	void resetEndgameFlags();

	Common::String _playerName = "ScummVM";
	Gfx::GfxChunks _gfx;
	Gfx::BgPics _bgPics;
	Gfx::Font _font;
	Gfx::Pics _hampic;
	Gfx::Pics _objects;
	Gfx::Pics _odin;
	Gfx::Pics _status;
	HighScores _highScores;
	SdData _sdData;
	Sound _sound;
	Scripts _scripts;
	GameMode _gameMode = MODE_NORMAL;
	TransitionDir _transitionDir = DIR_LEFT;
	Cheats _cheats;
	Common::Queue<byte> _demoKeys;
	bool _useItemFlag = false;
	bool _slipFlag = false;
	bool _slipping = false;
	int _slipCount = 0;
	bool _bossIntro1 = false;
	bool _bossIntro2 = false;

	int8 _pge = 0;
	int _exitFlag = 0;

	byte _keyFlag[100] = {};
	int8 _diag = 0;
	bool _diagFlag = false;
	bool _slowMode = false;
	bool _startup = true;
	bool _shotOk = false;
	int _thorX1 = 0;
	int _thorY1 = 0;
	int _thorX2 = 0;
	int _thorY2 = 0;
	int _thorRealY1 = 0;
	int _thorPos = 0;

	uint _magicCounter = 0;

	byte _objectMap[TILES_COUNT] = {};
	byte _objectIndex[TILES_COUNT] = {};
	int8 _thorIcon1 = 0;
	int8 _thorIcon2 = 0;
	int8 _thorIcon3 = 0;
	int8 _thorIcon4 = 0;
	int8 _levelMusic = 0;
	int8 _currentMusic = -1;
	int8 _appleDropCounter = 0;
	bool _cheat = false;
	int8 _area = 1;

	Level _scrn;

	Setup _setup;
	Setup _lastSetup;
	byte *_tmpBuff = nullptr;

	Actor _actor[MAX_ACTORS] = {};  //current actors
	Actor _enemy[MAX_ENEMIES] = {}; //current enemies
	Actor _shot[MAX_ENEMIES] = {};  //current shots
	int8 _enemyType[MAX_ENEMIES] = {};
	int _etype[MAX_ENEMIES] = {};

	Actor _magicItem[2] = {};
	byte _magicPic[2][1024] = {};
	bool _warpScroll = false;

	Actor *_thor = nullptr;
	Actor *_hammer = nullptr;
	Actor _explosion;
	Actor _sparkle;
	ThorInfo _thorInfo;
	bool _boss_dead = false;
	byte _endgame = 0;

	bool _warpFlag = false;

	int _rand1 = 0;
	int _rand2 = 0;
	int _thunder_flag = 0;
	bool _lightning_used = false;
	bool _tornado_used = false;
	bool _shield_on = false;
	bool _apple_flag = false;
	bool _bomb_flag = false;
	int _switch_flag = 0;

	byte _res_file[16] = {};
	bool _music_flag = false, _sound_flag = false;
	bool _cash1_inform = false;
	bool _cash2_inform = false;
	bool _door_inform = false;
	bool _magic_inform = false;
	bool _carry_inform = false;
	bool _killgg_inform = false;

	bool _boss_active = false;
	bool _story_flag = true;
	int8 *_scr = nullptr;
	bool _demo = false;
	int _rnd_index = 0;
	int _rnd_array[100] = {};
	bool _rdemo = false;
	int8 _test_sdf[80] = {};
	bool _game_over = false;
	char _tempstr[80] = {};
	bool _auto_load = false;
	bool _end_tile = false;
	byte _pbuff[PALETTE_SIZE] = {};
	int _current_level = 23;
	int _new_level = 0;
	int _new_level_tile = 0, _current_area = 0;
	char _sd_header[128] = {};
	char _play_speed = 0;
	bool _thor_special_flag = false;
	byte _exprow = 0;
};

#define _G(X) (g_vars->_##X)

} // namespace Got

#endif
