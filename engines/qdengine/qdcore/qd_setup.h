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

#ifndef QDENGINE_QDCORE_QD_SETUP_H
#define QDENGINE_QDCORE_QD_SETUP_H

#include "common/formats/ini-file.h"

namespace QDEngine {

//! Настройки.
class qdGameConfig {
public:
	qdGameConfig();
	~qdGameConfig() { }

	int screen_sx() const {
		return _screen_sx;
	}
	int screen_sy() const {
		return _screen_sy;
	}

	int driver_ID() const {
		return _driver_id;
	}
	void set_driver_ID(int id) {
		_driver_id = id;
	}

	void set_screen_size(int sx, int sy) {
		_screen_sx = sx;
		_screen_sy = sy;
	}

	int pixel_format() const;
	void set_pixel_format(int pf);

	int bits_per_pixel() const;
	void set_bits_per_pixel(int bpp);

	bool debug_draw() const {
		return _debug_draw;
	}
	void toggle_debug_draw() {
		_debug_draw = !_debug_draw;
	}
	bool debug_show_grid() const {
		return _debug_show_grid;
	}
	void toggle_show_grid() {
		_debug_show_grid = !_debug_show_grid;
	}

	bool force_full_redraw() const {
		return _force_full_redraw;
	}
	void toggle_full_redraw() {
		_force_full_redraw = !_force_full_redraw;
	}

	bool fullscreen() const {
		return _fullscreen;
	}
	void toggle_fullscreen() {
		_fullscreen = !_fullscreen;
	}

	const char *locale() const {
		return _locale.c_str();
	}

	void load();
	void save();

	static qdGameConfig &get_config() {
		return _config;
	}
	static void set_config(const qdGameConfig &s) {
		_config = s;
	}

	static const char *ini_name() {
		return _ini_name;
	}

	bool triggers_debug() const {
		return _triggers_debug;
	}
	void toggle_triggers_debug(bool v) {
		_triggers_debug = v;
	}

	bool show_fps() const {
		return _show_fps;
	}
	void toggle_fps() {
		_show_fps = !_show_fps;
	}

	bool is_sound_enabled() const {
		return _enable_sound;
	}
	void toggle_sound(bool state) {
		_enable_sound = state;
	}
	uint32 sound_volume() const {
		return _sound_volume;
	}
	void set_sound_volume(uint32 vol) {
		_sound_volume = vol;
	}
	bool update_sound_settings() const;

	bool is_music_enabled() const {
		return _enable_music;
	}
	void toggle_music(bool state) {
		_enable_music = state;
	}
	uint32 music_volume() const {
		return _music_volume;
	}
	void set_music_volume(uint32 vol) {
		_music_volume = vol;
	}
	bool update_music_settings() const;

	int logic_period() const {
		return _logic_period;
	}
	int logic_synchro_by_clock() const {
		return _logic_synchro_by_clock;
	}

	float game_speed() const {
		return _game_speed;
	}
	void set_game_speed(float speed) {
		_game_speed = speed;
	}

	bool is_splash_enabled() const {
		return _is_splash_enabled;
	}
	int splash_time() const {
		return _splash_time;
	}

	bool is_profiler_enabled() const {
		return _enable_profiler;
	}
	void toggle_profiler(bool state) {
		_enable_profiler = state;
	}
	const char *profiler_file() const {
		return _profiler_file.c_str();
	}
	void set_profiler_file(const char *fname) {
		_profiler_file = fname;
	}

	bool minigame_read_only_ini() const {
		return _minigame_read_only_ini;
	}

private:

	int _bits_per_pixel;
	bool _fullscreen;

	int _driver_id;

	int _screen_sx;
	int _screen_sy;

	bool _enable_sound;
	uint32 _sound_volume;

	bool _enable_music;
	uint32 _music_volume;

	bool _debug_draw;
	bool _debug_show_grid;

	bool _triggers_debug;
	bool _show_fps;
	bool _force_full_redraw;

	int _logic_period;
	int _logic_synchro_by_clock;
	float _game_speed;

	bool _is_splash_enabled;
	int _splash_time;

	bool _enable_profiler;
	Common::String _profiler_file;

	Common::String _locale;

	bool _minigame_read_only_ini;

	static qdGameConfig _config;
	static const char *const _ini_name;
};

char *getIniKey(const char *fname, const char *section, const char *key);
void putIniKey(const char *fname, const char *section, const char *key, int val);
void putIniKey(const char *fname, const char *section, const char *key, const char *val);
bool enumerateIniSections(const char *fname, Common::INIFile::SectionList &section_list);

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_SETUP_H
