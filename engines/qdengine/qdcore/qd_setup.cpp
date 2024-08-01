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

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/archive.h"
#include "common/formats/ini-file.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_setup.h"
#include "qdengine/system/graphics/gr_dispatcher.h"

#ifndef __QD_SYSLIB__
#include "qdengine/system/sound/snd_dispatcher.h"
#include "qdengine/qdcore/qd_game_scene.h"
#include "qdengine/qdcore/util/plaympp_api.h"
#endif

namespace QDEngine {

const char *const qdGameConfig::_ini_name = "qd_game.ini";
qdGameConfig qdGameConfig::_config;

bool enumerateIniSections(const char *fname, Common::INIFile::SectionList &sectionList) {

	Common::INIFile ini;
	Common::Path iniFilePath(fname);
	ini.loadFromFile(iniFilePath);
	sectionList = ini.getSections();
	int size = sectionList.size();

	if (!size) {
		return false;
	}

	return true;
}

char *getIniKey(const char *fname, const char *section, const char *key) {
	Common::INIFile ini;
	Common::String buf;

	ini.loadFromFile(fname);
	bool hasValue = ini.getKey(key, section, buf);

	if (!hasValue) {
		return "";
	}

	return (char *)buf.c_str();
}

void putIniKey(const char *fname, const char *section, const char *key, int val) {
	Common::String str;
	str += Common::String::format("%d", val);
	putIniKey(fname, section, key, str.c_str());
}

void putIniKey(const char *fname, const char *section, const char *key, const char *val) {
	warning("STUB: putIniKey");
}

qdGameConfig::qdGameConfig() {
	_screen_sx = 640;
	_screen_sy = 480;

	_bits_per_pixel = 2;
	_debug_draw = false;
	_debug_show_grid = false;
	_fullscreen = true;
	_triggers_debug = false;

	_driver_id = 1;
	_show_fps = false;
	_force_full_redraw = false;

	_enable_sound = true;
	_sound_volume = 255;
	_enable_music = true;
	_music_volume = 255;

	_logic_period = 25;
	_logic_synchro_by_clock = 1;
	_game_speed = 1.0f;

	_is_splash_enabled = true;
	_splash_time = 3000;

	_enable_profiler = false;

	_locale = "Russian";

	_minigame_read_only_ini = false;
}

void qdGameConfig::set_pixel_format(int pf) {
	switch (pf) {
	case GR_ARGB1555:
		_bits_per_pixel = 0;
		break;
	case GR_RGB565:
		_bits_per_pixel = 1;
		break;
	case GR_RGB888:
		_bits_per_pixel = 2;
		break;
	case GR_ARGB8888:
		_bits_per_pixel = 3;
		break;
	}
}

int qdGameConfig::bits_per_pixel() const {
	switch (_bits_per_pixel) {
	case 0:
		return 15;
	case 1:
		return 16;
	case 2:
		return 24;
	case 3:
		return 32;
	}

	return 24;
}

void qdGameConfig::set_bits_per_pixel(int bpp) {
	switch (bpp) {
	case 15:
		_bits_per_pixel = 0;
		break;
	case 16:
		_bits_per_pixel = 1;
		break;
	case 24:
		_bits_per_pixel = 2;
		break;
	case 32:
		_bits_per_pixel = 3;
		break;
	}
}

int qdGameConfig::pixel_format() const {
	switch (bits_per_pixel()) {
	case 15:
		return GR_ARGB1555;
	case 16:
		return GR_RGB565;
	case 24:
		return GR_RGB888;
	case 32:
		return GR_ARGB8888;
	}

	return GR_RGB888;
}

void qdGameConfig::load() {
	char *p = 0;
	p = getIniKey(_ini_name, "graphics", "color_depth");
	if (strlen(p)) _bits_per_pixel = atoi(p);

	p = getIniKey(_ini_name, "graphics", "fullscreen");
	if (strlen(p)) _fullscreen = (atoi(p)) > 0;

	p = getIniKey(_ini_name, "graphics", "driver");
	if (strlen(p)) _driver_id = atoi(p);

	p = getIniKey(_ini_name, "game", "logic_period");
	if (strlen(p)) _logic_period = atoi(p);

	p = getIniKey(_ini_name, "game", "synchro_by_clock");
	if (strlen(p)) _logic_synchro_by_clock = atoi(p);

	p = getIniKey(_ini_name, "game", "game_speed");
	if (strlen(p)) _game_speed = atof(p);

	p = getIniKey(_ini_name, "game", "locale");
	if (strlen(p)) _locale = p;

#ifndef __QD_SYSLIB__
	p = getIniKey(_ini_name, "game", "fps_period");
	if (strlen(p)) qdGameScene::fps_counter().set_period(atoi(p));
#endif

	if (atoi(getIniKey(_ini_name, "debug", "full_redraw")))
		_force_full_redraw = true;

	if (atoi(getIniKey(_ini_name, "debug", "enable_profiler")))
		_enable_profiler = true;

	p = getIniKey(_ini_name, "debug", "triggers_log_file");
	if (strlen(p)) _profiler_file = p;

	if (atoi(getIniKey(_ini_name, "debug", "triggers_debug")))
		_triggers_debug = true;

	p = getIniKey(_ini_name, "sound", "enable_sound");
	if (strlen(p)) _enable_sound = (atoi(p) > 0);

	p = getIniKey(_ini_name, "sound", "sound_volume");
	if (strlen(p)) _sound_volume = atoi(p);

	p = getIniKey(_ini_name, "sound", "enable_music");
	if (strlen(p)) _enable_music = (atoi(p) > 0);

	p = getIniKey(_ini_name, "sound", "music_volume");
	if (strlen(p)) _music_volume = atoi(p);

	p = getIniKey(_ini_name, "minigame", "read_only_ini");
	if (strlen(p)) _minigame_read_only_ini = (atoi(p) > 0);
}

void qdGameConfig::save() {
	putIniKey(_ini_name, "graphics", "color_depth", _bits_per_pixel);
	putIniKey(_ini_name, "graphics", "fullscreen", _fullscreen);
	putIniKey(_ini_name, "graphics", "driver", _driver_id);

	putIniKey(_ini_name, "sound", "enable_sound", _enable_sound);
	putIniKey(_ini_name, "sound", "sound_volume", _sound_volume);
	putIniKey(_ini_name, "sound", "enable_music", _enable_music);
	putIniKey(_ini_name, "sound", "music_volume", _music_volume);
}

bool qdGameConfig::update_sound_settings() const {
#ifndef __QD_SYSLIB__
	if (sndDispatcher * dp = sndDispatcher::get_dispatcher()) {
		dp->set_volume(_sound_volume);
		if (_enable_sound) dp->enable();
		else dp->disable();

		return true;
	}
#endif
	return false;
}

bool qdGameConfig::update_music_settings() const {
#ifndef __QD_SYSLIB__
	mpegPlayer::instance().set_volume(_music_volume);
	if (_enable_music) mpegPlayer::instance().enable();
	else mpegPlayer::instance().disable();
#endif
	return true;
}
} // namespace QDEngine
