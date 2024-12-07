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

#include "common/algorithm.h"
#include "common/config-manager.h"
#include "got/vars.h"
#include "got/gfx/palette.h"
#include "got/utils/file.h"
#include "got/got.h"

namespace Got {

Vars *g_vars;

Vars::Vars() {
	g_vars = this;
}

void Vars::load() {
	_font.load();
	_gfx.load();
	_bgPics.load();

	_music_flag = !ConfMan.getBool("music_mute");
	_sound_flag = _pcsound_flag = !ConfMan.getBool("sfx_mute");

	if (g_engine->isDemo()) {
		_demo = _cheat = true;
		_rdemo = ConfMan.getBool("rdemo");
	}

	if (_demo || _record) {
		if (_record)
			_demo = 0;

		_area = 1;
		_setup.area = 1;
		_cash1_inform = 1;
		_cash2_inform = 1;
		_door_inform = 1;
		_magic_inform = 1;
		_carry_inform = 1;
		_story_flag = 0;
	}

	if (_current_level != 23)
		_story_flag = 0;

	_setup.music = _music_flag;
	_setup.dig_sound = _sound_flag;
	_setup.pc_sound = _pcsound_flag;
	if (_sound_flag)
		_setup.pc_sound = false;
	_setup.scroll_flag = true;
	_setup.speed = _slow_mode;
	_setup.skill = 1;

	_tmp_buff = new byte[TMP_SIZE];
	_mask_buff = new byte[15300];
	_mask_buff_start = _mask_buff;
 
	res_read("RANDOM", _rnd_array);
	res_read("DEMO", _demo_key);
	res_read("TEXT", _text);
	res_read("ODINPIC", _odin);
	res_read("HAMPIC", _hampic);

	load_palette();

	_song = new byte[20000];
}

Vars::~Vars() {
	g_vars = nullptr;

	delete[] _tmp_buff;
	delete[] _mask_buff;
	delete[] _song;
}

} // namespace Got
