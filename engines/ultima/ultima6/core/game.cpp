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

#include "ultima/ultima6/core/nuvie_defs.h"
#include "ultima/ultima6/conf/configuration.h"
#include "ultima/ultima6/misc/u6_misc.h"

#include "ultima/ultima6/gui/gui.h"
#include "ultima/ultima6/core/console.h"
#include "ultima/ultima6/screen/dither.h"

#include "ultima/ultima6/sound/sound_manager.h"

#include "ultima/ultima6/actors/actor.h"
#include "ultima/ultima6/script/script.h"
#include "ultima/ultima6/screen/screen.h"
#include "ultima/ultima6/screen/game_palette.h"
#include "ultima/ultima6/core/game_clock.h"
#include "ultima/ultima6/core/egg_manager.h"
#include "ultima/ultima6/core/obj_manager.h"
#include "ultima/ultima6/actors/actor_manager.h"
#include "ultima/ultima6/core/player.h"
#include "ultima/ultima6/core/party.h"
#include "ultima/ultima6/core/converse.h"
#include "ultima/ultima6/core/converse_gump.h"
#include "ultima/ultima6/core/converse_gump_wou.h"
#include "ultima/ultima6/fonts/font_manager.h"
#include "ultima/ultima6/views/view_manager.h"
#include "ultima/ultima6/core/effect_manager.h"

#include "ultima/ultima6/core/magic.h"
#include "ultima/ultima6/core/msg_scroll.h"
#include "ultima/ultima6/core/msg_scroll_new_ui.h"
#include "ultima/ultima6/core/map.h"
#include "ultima/ultima6/core/map_window.h"
#include "ultima/ultima6/core/event.h"
#include "ultima/ultima6/portraits/portrait.h"
#include "ultima/ultima6/core/background.h"
#include "ultima/ultima6/core/command_bar.h"
#include "ultima/ultima6/core/command_bar_new_ui.h"
#include "ultima/ultima6/views/party_view.h"
#include "ultima/ultima6/views/actor_view.h"

#include "ultima/ultima6/usecode/usecode.h"
#include "ultima/ultima6/usecode/u6_usecode.h"

#include "ultima/ultima6/core/cursor.h"
#include "ultima/ultima6/save/save_manager.h"
#include "ultima/ultima6/core/weather.h"
#include "ultima/ultima6/core/book.h"
#include "ultima/ultima6/keybinding/keys.h"
#include "ultima/ultima6/keybinding/utils.h"
#include "ultima/ultima6/core/game.h"

#include "common/system.h"

namespace Ultima {
namespace Ultima6 {

Game *Game::game = NULL;

Game::Game(Configuration *cfg, Screen *scr, GUI *g, nuvie_game_t type, SoundManager *sm) {
	game = this;
	config = cfg;
	gui = g;

	screen = scr;
	game_type = type;
	sound_manager = sm;

	script = NULL;
	background = NULL;
	cursor = NULL;
	dither = NULL;
	tile_manager = NULL;
	obj_manager = NULL;
	palette = NULL;
	font_manager = NULL;
	scroll = NULL;
	game_map = NULL;
	map_window = NULL;
	actor_manager = NULL;
	player = NULL;
	event = NULL;
	converse = NULL;
	conv_gump = NULL;
	command_bar = NULL;
	new_command_bar = NULL;
	clock = NULL;
	party = NULL;
	portrait = NULL;
	view_manager = NULL;
	save_manager = NULL;
	egg_manager = NULL;
	usecode = NULL;
	effect_manager = NULL;
	weather = NULL;
	magic = NULL;
	book = NULL;
	keybinder = NULL;

	converse_gump_type = CONVERSE_GUMP_DEFAULT;
	pause_flags = PAUSE_UNPAUSED;
	pause_user_count = 0;
	ignore_event_delay = 0;
	game_play = true;
	unlimited_casting = false;
	god_mode_enabled = false;
	armageddon = false;
	ethereal = false;
	free_balloon_movement = false;

	config->value("config/cheats/enabled", cheats_enabled, false);
	config->value("config/cheats/enable_hackmove", is_using_hackmove, false);
	config->value("config/input/enabled_dragging", dragging_enabled, true);
	config->value("config/general/use_text_gumps", using_text_gumps, false);
	config->value(config_get_game_key(config) + "/roof_mode", roof_mode, false);
	config->value("config/input/doubleclick_opens_containers", open_containers, false);
	int value;
	uint16 screen_width = gui->get_width();
	uint16 screen_height = gui->get_height();

	init_game_style();
	if (is_orig_style()) {
		game_width = 320;
		game_height = 200;
	} else {
		config->value("config/video/game_width", value, 320);
		game_width = (value < screen_width) ? value : screen_width;
		config->value("config/video/game_height", value, 200);
		game_height = (value < screen_height) ? value : screen_height;
		if (game_width < 320)
			game_width = 320;
		if (game_height < 200)
			game_height = 200;
		if (is_original_plus_full_map() && screen_height <= 200) // not tall enough to show extra map space
			game_style = NUVIE_STYLE_ORIG_PLUS_CUTOFF_MAP;
	}

	string game_position;
	config->value("config/video/game_position", game_position, "center");

	if (game_position == "upper_left")
		game_x_offset = game_y_offset = 0;
	else { // center
		game_x_offset = (screen_width - game_width) / 2;
		game_y_offset = (screen_height - game_height) / 2;
	}

	effect_manager = new EffectManager;

	init_cursor();

	keybinder = new KeyBinder(config);
}

Game::~Game() {
	// note: don't delete objects that are added to the GUI object via
	// AddWidget()!
	if (dither) delete dither;
	if (tile_manager) delete tile_manager;
	if (obj_manager) delete obj_manager;
	if (palette) delete palette;
	if (font_manager) delete font_manager;
	//delete scroll;
	if (game_map) delete game_map;
	if (actor_manager) delete actor_manager;
	//delete map_window;
	if (player) delete player;
	if (event) delete event;
	//delete background;
	if (converse) delete converse;
	if (clock) delete clock;
	if (party) delete party;
	if (portrait) delete portrait;
	if (view_manager) delete view_manager;
	if (sound_manager) delete sound_manager;
	if (gui) delete gui;
	if (usecode) delete usecode;
	if (effect_manager) delete effect_manager;
	if (save_manager) delete save_manager;
	if (cursor) delete cursor;
	if (egg_manager) delete egg_manager;
	if (weather) delete weather;
	if (magic) delete magic;
	if (book) delete book;
	if (keybinder) delete keybinder;
}

bool Game::loadGame(Script *s) {
	dither = new Dither(config);

	script = s;
	//sound_manager->LoadSongs(NULL);
	//sound_manager->LoadObjectSamples(NULL);

	save_manager = new SaveManager(config);
	if (save_manager->init() == false)
		return false;

	palette = new GamePalette(screen, config);

	clock = new GameClock(config, game_type);


	background = new Background(config);
	background->init();
	background->Hide();
	if (is_original_plus_full_map() == false) // need to render before map window
		gui->AddWidget(background);

	font_manager = new FontManager(config);
	font_manager->init(game_type);

	if (!is_new_style()) {
		scroll = new MsgScroll(config, font_manager->get_font(0));
	} else {
		scroll = new MsgScrollNewUI(config, screen);
	}
	game_map = new Map(config);

	egg_manager = new EggManager(config, game_type, game_map);

	tile_manager = new TileManager(config);
	if (tile_manager->loadTiles() == false)
		return false;

	ConsoleAddInfo("Loading ObjManager()");
	obj_manager = new ObjManager(config, tile_manager, egg_manager);

	if (game_type == NUVIE_GAME_U6) {
		book = new Book(config);
		if (book->init() == false)
			return false;
		config->value(config_get_game_key(config) + "/free_balloon_movement", free_balloon_movement, false);
	}

	// Correct usecode class for each game
	switch (game_type) {
	case NUVIE_GAME_U6 :
		usecode = (UseCode *) new U6UseCode(this, config);
		break;
	case NUVIE_GAME_MD :
		usecode = (UseCode *) new UseCode(this, config);
		break;
	case NUVIE_GAME_SE :
		usecode = (UseCode *) new UseCode(this, config);
		break;
	}

	obj_manager->set_usecode(usecode);
	//obj_manager->loadObjs();

	ConsoleAddInfo("Loading map data.");
	game_map->loadMap(tile_manager, obj_manager);
	egg_manager->set_obj_manager(obj_manager);

	ConsoleAddInfo("Loading actor data.\n");
	actor_manager = new ActorManager(config, game_map, tile_manager, obj_manager, clock);

	game_map->set_actor_manager(actor_manager);
	egg_manager->set_actor_manager(actor_manager);

	map_window = new MapWindow(config, game_map);
	map_window->init(tile_manager, obj_manager, actor_manager);
	map_window->Hide();
	gui->AddWidget(map_window);
	if (is_original_plus_full_map()) // need to render after map window
		gui->AddWidget(background);

	weather = new Weather(config, clock, game_type);

//   if(!is_new_style()) // Everyone always uses original style command bar now.
	{
		command_bar = new CommandBar(this);
		bool using_new_command_bar;
		config->value("config/input/new_command_bar", using_new_command_bar, false);
		if (using_new_command_bar) {
			init_new_command_bar();
		}
	}
//   else
//	   command_bar = new CommandBarNewUI(this);
	command_bar->Hide();
	gui->AddWidget(command_bar);


	player = new Player(config);
	party = new Party(config);
	player->init(obj_manager, actor_manager, map_window, clock, party);
	party->init(this, actor_manager);

	portrait = newPortrait(game_type, config);
	if (portrait->init() == false)
		return false;

	view_manager = new ViewManager(config);
	view_manager->init(gui, font_manager->get_font(0), party, player, tile_manager, obj_manager, portrait);
	scroll->Hide();
	gui->AddWidget(scroll);


	//map_window->set_windowSize(11,11);

	init_converse_gump_settings();
	init_converse();

	usecode->init(obj_manager, game_map, player, scroll);



	if (game_type == NUVIE_GAME_U6) {
		magic = new Magic();
	}

	event = new Event(config);
	event->init(obj_manager, map_window, scroll, player, magic, clock, view_manager, usecode, gui, keybinder);
	if (game_type == NUVIE_GAME_U6) {
		magic->init(event);
	}

	if (save_manager->load_save() == false) {
		return false;
	}

	ConsoleAddInfo("Polishing Anhk");

	//ConsolePause();
	ConsoleHide();

//   if(!is_new_style())
	{
		if (is_orig_style())
			command_bar->Show();
		else {
			bool show;
			std::string show_cb;
			config->value(config_get_game_key(config) + "/show_orig_style_cb", show_cb, "default");
			if (show_cb == "default") {
				if (is_new_style())
					show = false;
				else
					show = true;
			} else if (show_cb == "no")
				show = false;
			else
				show = true;
			if (show)
				command_bar->Show();
		}
	}

	if (!is_new_style() || screen->get_width() != get_game_width() || screen->get_height() != get_game_height()) {
		background->Show();
	}

	map_window->Show();
	scroll->Show();
	view_manager->update();

	if (cursor)
		cursor->show();

	return true;
}

void Game::init_converse_gump_settings() {
	if (is_new_style())
		converse_gump_type = CONVERSE_GUMP_DEFAULT;
	else {
		converse_gump_type = get_converse_gump_type_from_config(config);
	}
	std::string width_str;
	int gump_w = get_game_width();

	if (game_type == NUVIE_GAME_MD)
		min_converse_gump_width = 298;
	else if (game_type == NUVIE_GAME_SE)
		min_converse_gump_width = 301;
	else // U6
		min_converse_gump_width = 286;

	config->value(config_get_game_key(config) + "/converse_width", width_str, "default");
	if (!game->is_orig_style()) {
		if (width_str == "default") {
			int map_width = get_game_width();
			if (is_original_plus())
				map_width += - background->get_border_width() - 1;
			if (map_width > min_converse_gump_width * 1.5) // big enough that we probably don't want to take up the whole screen
				gump_w = min_converse_gump_width;
			else if (game->is_original_plus() && map_width >= min_converse_gump_width) // big enough to draw without going over the UI
				gump_w = map_width;
		} else {
			config->value(config_get_game_key(config) + "/converse_width", gump_w, gump_w);
			if (gump_w < min_converse_gump_width)
				gump_w = min_converse_gump_width;
			else if (gump_w > get_game_width())
				gump_w = get_game_width();
		}
	}
	converse_gump_width = (uint16)gump_w;

	if ((is_original_plus_cutoff_map() && get_game_width() - background->get_border_width() < min_converse_gump_width)
	        || game->is_orig_style())
		force_solid_converse_bg = true;
	else
		force_solid_converse_bg = false;
}

void Game::init_converse() {
	converse = new Converse();
	if (using_new_converse_gump()) {
		conv_gump = new ConverseGump(config, font_manager->get_font(0), screen);
		conv_gump->Hide();
		gui->AddWidget(conv_gump);

		converse->init(config, game_type, conv_gump, actor_manager, clock, player, view_manager, obj_manager);
	} else if (game_type == NUVIE_GAME_U6 && converse_gump_type == CONVERSE_GUMP_DEFAULT) {
		converse->init(config, game_type, scroll, actor_manager, clock, player, view_manager, obj_manager);
	} else {
		ConverseGumpWOU *gump = new ConverseGumpWOU(config, font_manager->get_font(0), screen);
		gump->Hide();
		gui->AddWidget(gump);
		converse->init(config, game_type, gump, actor_manager, clock, player, view_manager, obj_manager);
	}

}

void Game::set_converse_gump_type(uint8 new_type) {
	if (converse)
		delete converse;
	converse_gump_type = new_type;
	init_converse();
}

bool Game::using_new_converse_gump() {
	return (is_new_style() || converse_gump_type == CONVERSE_GUMP_U7_STYLE);
}

void Game::delete_new_command_bar() {
	if (new_command_bar == NULL)
		return;
	new_command_bar->Delete();
	new_command_bar = NULL;
}

void Game::init_new_command_bar() {
	if (new_command_bar != NULL)
		return;
	new_command_bar = new CommandBarNewUI(this);
	new_command_bar->Hide();
	gui->AddWidget(new_command_bar);
}

void Game::init_cursor() {
	if (!cursor)
		cursor = new Cursor();

	if (cursor->init(config, screen, game_type))
		SDL_ShowCursor(false); // won't need the system default
	else {
		delete cursor;
		cursor = NULL; // no game cursor
	}
}

void Game::init_game_style() {
	string game_style_str;
	config->value("config/video/game_style", game_style_str, "original");
	if (game_style_str == "new")
		game_style = NUVIE_STYLE_NEW;
	else if (game_style_str == "original+")
		game_style = NUVIE_STYLE_ORIG_PLUS_CUTOFF_MAP;
	else if (game_style_str == "original+_full_map")
		game_style = NUVIE_STYLE_ORIG_PLUS_FULL_MAP;
	else
		game_style = NUVIE_STYLE_ORIG;

}

bool Game::doubleclick_opens_containers() {
	if (open_containers || is_new_style())
		return true;
	else
		return false;
}

bool Game::using_hackmove() {
	if (cheats_enabled)
		return is_using_hackmove;
	else
		return false;
}

void Game::set_hackmove(bool hackmove) {
	is_using_hackmove = hackmove;
	map_window->set_interface();
}

bool Game::set_mouse_pointer(uint8 ptr_num) {
	return (cursor && cursor->set_pointer(ptr_num));
}

//FIXME pausing inside a script function causes problems with yield/resume logic.
void Game::set_pause_flags(GamePauseState state) {
	pause_flags = state; // set
}

void Game::unpause_all() {
//	 DEBUG(0, LEVEL_DEBUGGING,"Unpause ALL!\n");
	unpause_user();
	unpause_anims();
	unpause_world();
}

void Game::unpause_user() {
	if (pause_user_count > 0)
		pause_user_count--;

	if (pause_user_count == 0) {
		set_pause_flags((GamePauseState)(pause_flags & ~PAUSE_USER));


		//if(event->get_mode() == WAIT_MODE)
		//    event->endAction(); // change to MOVE_MODE, hide cursors
		if (gui->get_block_input())
			gui->unblock();

	}

//    DEBUG(0, LEVEL_DEBUGGING, "unpause user count=%d!\n", pause_user_count);
}

void Game::unpause_anims() {
	set_pause_flags((GamePauseState)(pause_flags & ~PAUSE_ANIMS));
}

void Game::unpause_world() {
	set_pause_flags((GamePauseState)(pause_flags & ~PAUSE_WORLD));

	if (actor_manager->get_update() == false) // ActorMgr is not running
		game->get_actor_manager()->set_update(true); // resume

	//if(clock->get_active() == false) // start time
	//    clock->set_active(true);
}

void Game::pause_all() {
	pause_user();
	pause_anims();
	pause_world();
}

void Game::pause_user() {
	set_pause_flags((GamePauseState)(pause_flags | PAUSE_USER));

	if (!gui->get_block_input() && pause_user_count == 0)
		gui->block();

	pause_user_count++;

//    DEBUG(0, LEVEL_DEBUGGING, "Pause user count=%d!\n", pause_user_count);
}

void Game::pause_anims() {
	set_pause_flags((GamePauseState)(pause_flags | PAUSE_ANIMS));
}

void Game::pause_world() {
	set_pause_flags((GamePauseState)(pause_flags | PAUSE_WORLD));

	if (actor_manager->get_update() == true) // ActorMgr is running
		game->get_actor_manager()->set_update(false); // pause

	//if(clock->get_active() == true) // stop time
	//    clock->set_active(false);
}


void Game::dont_wait_for_interval() {
	if (ignore_event_delay < 255)
		++ignore_event_delay;
	event->set_ignore_timeleft(true);
}


void Game::wait_for_interval() {
	if (ignore_event_delay > 0)
		--ignore_event_delay;
	if (ignore_event_delay == 0)
		event->set_ignore_timeleft(false);
}


void Game::time_changed() {
	if (!is_new_style()) {
		if (game->is_orig_style()) // others constantly update
			get_command_bar()->update(); // date & wind
		get_view_manager()->get_party_view()->update(); // sky
	}
	get_map_window()->updateAmbience();
}

// FIXME: should this be in ViewManager?
void Game::stats_changed() {
	if (!is_new_style()) {
		get_view_manager()->get_actor_view()->update();
		get_view_manager()->get_party_view()->update();
	}
}


void Game::play() {
	pause_flags = PAUSE_UNPAUSED;

	//view_manager->set_inventory_mode(1); //FIX

	screen->update();

	//map_window->drawMap();
//if !SDL_VERSION_ATLEAST(2, 0, 0)
#if 0
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY / 2, SDL_DEFAULT_REPEAT_INTERVAL);
#endif

	map_window->updateBlacking();

	for (; game_play ;) {
		if (cursor) cursor->clear(); // restore cursor area before GUI events

		event->update();
		if (clock->get_timer(GAMECLOCK_TIMER_U6_TIME_STOP) == 0) {
			palette->rotatePalette();
			tile_manager->update();
			actor_manager->twitchActors();
		}
		actor_manager->moveActors(); // update/move actors for this turn
		map_window->update();
		//map_window->drawMap();
		converse->continue_script();
		//scroll->updateScroll();
		effect_manager->update_effects();

		gui->Display();
		if (cursor) cursor->display();

		screen->preformUpdate();
		sound_manager->update();
		event->wait();
	}
	return;
}

void Game::update_until_converse_finished() {
	while (converse->running()) {
		update_once(true, true);
		update_once_display();
	}
}

void Game::update_once(bool process_gui_input) {
	update_once(process_gui_input, false);
}

void Game::update_once(bool process_gui_input, bool run_converse) {
	if (cursor) cursor->clear(); // restore cursor area before GUI events

	event->update_timers();

	Common::Event event;
	while (Events::get()->pollEvent(event)) {
		if (process_gui_input)
			gui->HandleEvent(&event);
	}

	if (clock->get_timer(GAMECLOCK_TIMER_U6_TIME_STOP) == 0) {
		palette->rotatePalette();
		tile_manager->update();
		actor_manager->twitchActors();
	}
	map_window->update();
	if (run_converse) {
		converse->continue_script();
	}
	effect_manager->update_effects();
}

void Game::update_once_display() {
	gui->Display();
	if (cursor) cursor->display();

	screen->preformUpdate();
	sound_manager->update();
	event->wait();
}

/* return the fullpath to the datafile. First look for it in the savegame directory.
 * Then in the app data directory.
 */
std::string Game::get_data_file_path(std::string datafile) {
	std::string path;
	build_path("data", datafile, path);
	build_path(save_manager->get_savegame_directory(), path, path);
	if (!file_exists(path.c_str())) {
		build_path(gui->get_data_dir(), datafile, path);
	}

	return path;
}

uint getRandom(uint maxVal) {
	::error("TODO");
}

} // End of namespace Ultima6
} // End of namespace Ultima
