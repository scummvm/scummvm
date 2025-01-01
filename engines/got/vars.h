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
#include "graphics/screen.h"
#include "got/data/defines.h"
#include "got/data/sd_data.h"
#include "got/game/script.h"
#include "got/gfx/font.h"
#include "got/gfx/gfx_chunks.h"
#include "got/gfx/gfx_pics.h"
#include "got/metaengine.h"
#include "got/sound.h"

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
	MODE_NORMAL, MODE_AREA_CHANGE, MODE_THUNDER,
	MODE_THOR_DIES, MODE_ADD_SCORE, MODE_LIGHTNING,
	MODE_PAUSE, MODE_SCORE_INV
};

enum TransitionDir {
	DIR_LEFT, DIR_RIGHT, DIR_UP, DIR_DOWN, DIR_PHASED
};

struct Cheats {
	bool freezeHealth = false;
	bool freezeMagic = false;
	bool freezeJewels = false;
};

class Vars {
public:
	Vars();
	~Vars();

	void load();
	void setArea(int areaNum);
	void clearKeyFlags();

	Common::String _playerName = "ScummVM";
	Gfx::GfxChunks _gfx;
	Gfx::BgPics _bgPics;
	Gfx::Font _font;
	Gfx::Pics _hampic;
	Gfx::Pics _objects;
	Gfx::Pics _odin;
	Gfx::Pics _status;
	SdData _sd_data;
	Sound _sound;
	Scripts _scripts;
	GameMode _gameMode = MODE_NORMAL;
	TransitionDir _transitionDir = DIR_LEFT;
	Cheats _cheats;
	Common::Queue<byte> _demoKeys;
	bool _useItemFlag = false;

	uint _page[3] = { PAGE0,PAGE1,PAGE2 };
	uint _display_page = 0, _draw_page = 0;
	uint _page3_offset = 0;
	int8 _pge = 0;
	int _exit_flag = 0;

	byte _key_flag[100] = {};
	byte _joy_flag[100] = {};
	byte _tmp_flag[100] = {};
	int8 _break_code = 0;
	int8 _scan_code = 0, _last_scan_code = 0;
	int8 _diag = 0;
	bool _diag_flag = false;
	bool _slow_mode = false, _startup = true;
	int8 _shot_ok = 0;
	int _thor_x1 = 0, _thor_y1 = 0, _thor_x2 = 0, _thor_y2 = 0,
		_thor_real_y1 = 0;
	int _thor_pos = 0;
	int _max_shot = 0;

	uint _timer_cnt = 0, _vbl_cnt = 0, _magic_cnt = 0, _extra_cnt = 0;

	//union REGS in, out = 0;
	//struct SREGS seg = 0;
	int _ox = 0, _oy = 0, _of = 0;
	byte _object_map[TILES_COUNT] = {};
	byte _object_index[TILES_COUNT] = {};
	byte *_bleep = nullptr;
	int8 _thor_icon1 = 0, _thor_icon2 = 0, _thor_icon3 = 0,
		_thor_icon4 = 0;
	int8 _level_type = 0;
	int8 _music_current = -1;
	int8 _boss_loaded = 0;
	int8 _apple_drop = 0;
	bool _cheat = false;
	int8 _area = 1;

	LEVEL _scrn;
	byte *_scrnp = nullptr;

	SETUP _setup;
	SETUP _last_setup;
	byte *_tmp_buff = nullptr;
	int _reps = 0;

	byte *_mask_buff = nullptr;
	byte *_mask_buff_start = nullptr;
	byte _abuff[AMI_LEN] = {};
	byte *_ami_buff = nullptr;
	ACTOR _actor[MAX_ACTORS] = {};   //current actors
	ACTOR _enemy[MAX_ENEMIES] = {};  //current enemies
	ACTOR _shot[MAX_ENEMIES] = {};   //current shots
	int8 _enemy_type[MAX_ENEMIES] = {};
	int _etype[MAX_ENEMIES] = {};

	ACTOR _magic_item[2] = {};
	byte _magic_pic[2][1024] = {};
	int8 _warp_scroll = 0;

	ACTOR *_thor = nullptr;
	ACTOR *_hammer = nullptr;
	ACTOR _explosion;
	ACTOR _sparkle;
	THOR_INFO _thor_info;
	int _boss_dead = 0;

	int _warp_flag = 0;

	int8 *_std_sound_start = nullptr;
	int8 *_pcstd_sound_start = nullptr;
	int8 *_std_sound = nullptr;
	int8 *_pcstd_sounds = nullptr;
	byte *_boss_sound[3] = {};
	byte *_boss_pcsound[3] = {};
	long _pcsound_length[NUM_SOUNDS] = {};
	int _rand1 = 0, _rand2 = 0;
	int _restore_screen = 0;
	int _last_oracle = 0;
	int _hourglass_flag = 0, _thunder_flag = 0, _shield_on = 0,
		_lightning_used = 0, _tornado_used = 0;
	int _apple_flag = 0, _bomb_flag = 0;
	int _switch_flag = 0;
	uint _joy_x = 0, _joy_y = 0;
	int8 _joy_b1 = 0, _joy_b2 = 0;
	int _joystick = 0, _joylx = 0, _joyly = 0,
		_joyhx = 0, _joyhy = 0;
	byte _res_file[16] = {};
	int _load_game_flag = 0;
	bool _music_flag = false, _sound_flag = false, _pcsound_flag = false;
	int _cash1_inform = 0, _cash2_inform = 0, _door_inform = 0,
		_magic_inform = 0, _carry_inform = 0;
	int _killgg_inform = 0;

	int8 *_std_sounds = nullptr;
	byte *_pc_sound[NUM_SOUNDS] = {};
	byte *_dig_sound[NUM_SOUNDS] = {};
	int  _boss_active = 0;
	int8 _story_flag = 1;
	int8 *_scr = nullptr;
	bool _demo = false;
	int8 _demo_enable = 1;
	int  _rnd_index = 0;
	int  _rnd_array[100] = {};
	int8 _rdemo = 0;
	int8 _test_sdf[80] = {};
	int8 _lzss_buff = 0;
	int8 _game_over = 0;
	int8 _noal = 0, _nosb = 0, _ret = 0;
	char _tempstr[80] = {};
	int8 _auto_load = 0;
	int8 _ide_run = 0, _fast_exit = 0, _nojoy = 0, _gr = 0, _xdos = 0;
	int8 _main_loop = 0;
	int8 _end_tile = 0;
	byte _pbuff[PALETTE_SIZE] = {};
	int _current_level = 23;
	int _new_level = 0;
	int _new_level_tile = 0, _current_area = 0;
	byte *_enemy_mb = nullptr;
	byte *_enemy_ami = nullptr;
	char _sd_header[128] = {};
	char _play_speed = 0;
	byte *_magic_ami = nullptr;
	byte *_magic_mask_buff = nullptr;
	byte *_ami_store1 = nullptr, *_ami_store2 = nullptr;
	byte *_mask_store1 = nullptr, *_mask_store2 = nullptr;
	bool _thor_special_flag = false;
};

#define _G(X) (g_vars->_##X)

} // namespace Got

#endif
