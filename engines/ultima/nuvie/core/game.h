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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef NUVIE_CORE_GAME_H
#define NUVIE_CORE_GAME_H

#include "ultima/shared/std/containers.h"
#include "ultima/shared/std/string.h"
#include "ultima/nuvie/core/nuvie_defs.h"

namespace Ultima {
namespace Nuvie {

class Configuration;
class Script;
class Screen;
class Background;
class GamePalette;
class FontManager;
class Dither;
class TileManager;
class ObjManager;
class ActorManager;
class Magic;
class Map;
class MapWindow;
class MsgScroll;
class Player;
class Party;
class Converse;
class ConverseGump;
class Cursor;
class GameClock;
class ViewManager;
class Portrait;
class UseCode;
class Events;
class GUI;
class EffectManager;
class SoundManager;
class EggManager;
class CommandBar;
class Weather;
class Book;
class KeyBinder;

typedef enum {
	PAUSE_UNPAUSED = 0x00,
	PAUSE_USER     = 0x01, /* Don't allow user-input */
	PAUSE_ANIMS    = 0x02, /* TileManager & Palette */
	PAUSE_WORLD    = 0x04, /* game time doesn't pass, freeze actors */
	PAUSE_ALL      = 0xFF
} GamePauseState;

class Game {
private:
	nuvie_game_t game_type;
	uint8 game_style; //new, original, orig_plus_cutoff_map, or orig_plus_full_map
	static Game *game;
	Configuration *config;
	Script *script;
	Screen *screen;
	Background *background;
	GamePalette *palette;
	Dither *dither;
	FontManager *font_manager;
	TileManager *tile_manager;
	ObjManager *obj_manager;
	ActorManager *actor_manager;
	Magic *magic;
	Map *game_map;
	MapWindow *map_window;
	MsgScroll *scroll;
	Player *player;
	Party *party;
	Converse *converse;
	ConverseGump *conv_gump;
	CommandBar *command_bar;
	CommandBar *new_command_bar;

	ViewManager *view_manager;
	EffectManager *effect_manager;
	SoundManager *sound_manager;
	EggManager *egg_manager;

	GameClock *clock;
	Portrait *portrait;
	UseCode *usecode;

	Weather *weather;

	Cursor *cursor;

	Events *event;

	GUI *gui;

	Book *book;
	KeyBinder *keybinder;

	GamePauseState pause_flags;
	uint16 game_width;
	uint16 game_height;
	uint16 game_x_offset;
	uint16 game_y_offset;
	uint16 pause_user_count;
	uint16 converse_gump_width;
	uint16 min_converse_gump_width;
	uint8 ignore_event_delay; // (stack) if non-zero, Events will not periodically wait for NUVIE_INTERVAL
	bool is_using_hackmove;
	bool dragging_enabled;
	bool cheats_enabled;
	bool unlimited_casting;
	bool god_mode_enabled;
	bool armageddon;
	bool ethereal;
	bool using_text_gumps;
	bool open_containers; //doubleclick
	uint8 converse_gump_type;
	bool roof_mode;
	bool free_balloon_movement;
	bool force_solid_converse_bg;
	bool _playing;

public:
	Game(Configuration *cfg, Events *evt, Screen *scr, GUI *g, nuvie_game_t type, SoundManager *sm);
	~Game();

	bool loadGame(Script *s);
	void init_cursor();
	void init_game_style();
	void play();
	void update_once(bool process_gui_input);

	void update_once_display();
	void update_until_converse_finished();

	bool isLoaded() const {
		return script != nullptr;
	}
	GamePauseState get_pause_flags()            {
		return (pause_flags);
	}
	void set_pause_flags(GamePauseState state);
	void unpause_all();
	void unpause_user();
	void unpause_anims();
	void unpause_world();
	void pause_all();
	void pause_user();
	void pause_anims();
	void pause_world();

	bool paused()       {
		return (pause_flags);
	}
	bool all_paused()   {
		return (pause_flags & PAUSE_ALL);
	}
	bool user_paused()  {
		return (pause_flags & PAUSE_USER);
	}
	bool anims_paused() {
		return (pause_flags & PAUSE_ANIMS);
	}
	bool world_paused() {
		return (pause_flags & PAUSE_WORLD);
	}

	void quit() {
		_playing = false;
	}

	bool shouldQuit() const;

	bool set_mouse_pointer(uint8 ptr_num);
	void dont_wait_for_interval();
	void wait_for_interval();
	void time_changed();
	void stats_changed();

	void init_new_command_bar();
	void delete_new_command_bar();
	nuvie_game_t get_game_type() {
		return game_type;
	}
	uint8 get_game_style() {
		return game_style;
	}
	bool is_original_plus() {
		return (game_style == NUVIE_STYLE_ORIG_PLUS_CUTOFF_MAP || game_style == NUVIE_STYLE_ORIG_PLUS_FULL_MAP);
	}
	bool is_original_plus_cutoff_map() {
		return (game_style == NUVIE_STYLE_ORIG_PLUS_CUTOFF_MAP);
	}
	bool is_original_plus_full_map() {
		return (game_style == NUVIE_STYLE_ORIG_PLUS_FULL_MAP);
	}
	bool is_new_style() {
		return (game_style == NUVIE_STYLE_NEW);
	}
	bool is_orig_style() {
		return (game_style == NUVIE_STYLE_ORIG);
	}
	bool doubleclick_opens_containers();
	void set_doubleclick_opens_containers(bool val) {
		open_containers = val;
	}
	void set_using_text_gumps(bool val) {
		using_text_gumps = val;
	}
	bool is_using_text_gumps() {
		return (using_text_gumps || is_new_style());
	}
	bool is_roof_mode() {
		return roof_mode;
	}
	void set_roof_mode(bool val) {
		roof_mode = val;
	}
	bool using_hackmove();
	void set_hackmove(bool hackmove);
	uint8 is_dragging_enabled() {
		return dragging_enabled;
	}
	void set_dragging_enabled(bool drag) {
		dragging_enabled = drag;
	}
	bool is_god_mode_enabled() {
		return (god_mode_enabled && cheats_enabled);
	}
	bool toggle_god_mode() {
		return (god_mode_enabled = !god_mode_enabled);
	}
	bool are_cheats_enabled() {
		return cheats_enabled;
	}
	void set_cheats_enabled(bool cheat) {
		cheats_enabled = cheat;
	}
	bool has_unlimited_casting() {
		return (unlimited_casting && cheats_enabled);
	}
	void set_unlimited_casting(bool unlimited) {
		unlimited_casting = unlimited;
	}
	bool is_armageddon() {
		return armageddon;
	}
	void set_armageddon(bool val) {
		armageddon = val;
	}
	bool is_ethereal() {
		return ethereal;
	}
	void set_ethereal(bool val) {
		ethereal = val;
	}
	uint8 get_converse_gump_type() {
		return converse_gump_type;
	}
	void set_converse_gump_type(uint8 new_type);
	bool using_new_converse_gump();
	void set_free_balloon_movement(bool val) {
		free_balloon_movement = val;
	}
	bool has_free_balloon_movement() {
		return free_balloon_movement;
	}
	bool is_forcing_solid_converse_bg() {
		return force_solid_converse_bg;
	}
	uint16 get_converse_gump_width() {
		return converse_gump_width;
	}
	uint16 get_min_converse_gump_width() {
		return min_converse_gump_width;
	}
	uint16 get_game_width() {
		return game_width;
	}
	uint16 get_game_height() {
		return game_height;
	}
	uint16 get_game_x_offset() {
		return game_x_offset;
	}
	uint16 get_game_y_offset() {
		return game_y_offset;
	}
	Std::string get_data_file_path(Std::string datafile);

	/* Return instances of Game classes */
	static Game *get_game()          {
		return (game);
	}
	Configuration *get_config()       {
		return (config);
	}
	Script *get_script()              {
		return (script);
	}
	Screen *get_screen()              {
		return (screen);
	}
	Background *get_background()      {
		return (background);
	}
	GamePalette *get_palette()        {
		return (palette);
	}
	Dither *get_dither()                  {
		return (dither);
	}
	FontManager *get_font_manager()   {
		return (font_manager);
	}
	TileManager *get_tile_manager()   {
		return (tile_manager);
	}
	ObjManager *get_obj_manager()     {
		return (obj_manager);
	}
	ActorManager *get_actor_manager() {
		return (actor_manager);
	}
	EggManager *get_egg_manager()     {
		return (egg_manager);
	}
	Magic *get_magic()                {
		return (magic);
	}
	Map *get_game_map()               {
		return (game_map);
	}
	MapWindow *get_map_window()       {
		return (map_window);
	}
	MsgScroll *get_scroll()           {
		return (scroll);
	}
	Player *get_player()              {
		return (player);
	}
	Party *get_party()                {
		return (party);
	}
	Converse *get_converse()          {
		return (converse);
	}
	ConverseGump *get_converse_gump() {
		return (conv_gump);
	}
	ViewManager *get_view_manager()   {
		return (view_manager);
	}
	GameClock *get_clock()            {
		return (clock);
	}
	Portrait *get_portrait()          {
		return (portrait);
	}
	UseCode *get_usecode()            {
		return (usecode);
	}
	Events *get_event()                {
		return (event);
	}
	GUI *get_gui()                    {
		return (gui);
	}
	SoundManager *get_sound_manager() {
		return (sound_manager);
	}

	Cursor *get_cursor()              {
		return (cursor);
	}
	EffectManager *get_effect_manager() {
		return (effect_manager);
	}
	CommandBar *get_command_bar()     {
		return (command_bar);
	}
	CommandBar *get_new_command_bar() {
		return (new_command_bar);
	}
	Weather *get_weather()            {
		return (weather);
	}

	Book *get_book()                  {
		return (book);
	}
	KeyBinder *get_keybinder()        {
		return (keybinder);
	}
protected:
	void init_converse();
	void init_converse_gump_settings();

private:
	void update_once(bool process_gui_input, bool run_converse);
};

extern uint getRandom(uint maxVal);

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
