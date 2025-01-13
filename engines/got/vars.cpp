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

#include "got/vars.h"
#include "common/algorithm.h"
#include "common/config-manager.h"
#include "got/gfx/palette.h"
#include "got/got.h"
#include "got/utils/file.h"

namespace Got {

Vars *g_vars;

Vars::Vars() : _hampic("HAMPIC", 262, false),
			   _odin("ODINPIC", 262, false),
			   _objects("OBJECTS", 262, false),
			   _status("STATUS", -1, false) {
	g_vars = this;
}

void Vars::load() {
	_bgPics.load();
	_font.load();
	_gfx.load();
	_hampic.load();
	_objects.load();
	_odin.load();
	_sd_data.load();
	_sound.load();
	_status.load();
	_highScores.load();

	_music_flag = !ConfMan.getBool("music_mute");
	_sound_flag = _pcsound_flag = !ConfMan.getBool("sfx_mute");

	if (g_engine->isDemo()) {
		_demo = _cheat = true;
		_rdemo = ConfMan.getBool("rdemo");
	}

	if (_current_level != 23)
		_story_flag = false;

	_setup.music = _music_flag;
	_setup.dig_sound = _sound_flag;
	_setup.pc_sound = _pcsound_flag;
	if (_sound_flag)
		_setup.pc_sound = false;
	_setup.scroll_flag = true;
	_setup.speed = _slow_mode ? 1 : 0;
	_setup.skill = 1;

	_tmp_buff = new byte[TMP_SIZE];

	res_read("RANDOM", _rnd_array);

	Gfx::load_palette();
}

Vars::~Vars() {
	g_vars = nullptr;

	delete[] _tmp_buff;
}

void Vars::setArea(int areaNum) {
	if (areaNum != _area) {
		_area = areaNum;
		_setup.area = areaNum;
		_sd_data.setArea(areaNum);
		_bgPics.setArea(areaNum);

		switch (areaNum) {
		case 1:
			_current_level = 23;
			break;

		case 2:
			_current_level = 51;
			break;

		case 3:
			_current_level = 33;
			break;

		default:
			break;
		}
	}
}

void Vars::clearKeyFlags() {
	Common::fill(_key_flag, _key_flag + 100, 0);
}

void Vars::resetEndgameFlags() {
	_gameMode = MODE_NORMAL;
	_auto_load = false;
	_end_tile = false;
	_boss_dead = false;
	_game_over = false;
	_boss_intro1 = _boss_intro2 = false;
}

} // namespace Got
