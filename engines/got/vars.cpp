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
	_sdData.load();
	_sound.load();
	_status.load();
	_highScores.load();

	_musicFlag = !ConfMan.getBool("music_mute");
	_soundFlag = !ConfMan.getBool("sfx_mute");

	if (g_engine->isDemo()) {
		_demo = _cheat = true;
	}

	if (_currentLevel != 23)
		_storyFlag = false;

	_setup._musicEnabled = _musicFlag;
	_setup._digitalSound = _soundFlag;
	_setup._speakerSound = false;
	_setup._scrollFlag = true;
	_setup._slowMode = _slowMode;
	_setup._difficultyLevel = 1;

	_tmpBuff = new byte[TMP_SIZE];

	Gfx::loadPalette();
}

Vars::~Vars() {
	g_vars = nullptr;

	delete[] _tmpBuff;
}

void Vars::setArea(int areaNum) {
	if (areaNum != _area) {
		_area = areaNum;
		_setup._areaNum = areaNum;
		_sdData.setArea(areaNum);
		_bgPics.setArea(areaNum);

		switch (areaNum) {
		case 1:
			_currentLevel = 23;
			break;

		case 2:
			_currentLevel = 51;
			break;

		case 3:
			_currentLevel = 33;
			break;

		default:
			break;
		}
	}
}

void Vars::clearKeyFlags() {
	Common::fill(_keyFlag, _keyFlag + 100, 0);
}

void Vars::resetEndGameFlags() {
	_gameMode = MODE_NORMAL;
	_endTile = false;
	_bossDead = false;
	_gameOver = false;
	_bossIntro1 = _bossIntro2 = false;
}

} // namespace Got
