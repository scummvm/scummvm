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

#include "common/debug.h"
#include "m4/globals.h"
#include "m4/adv_r/adv.h"
#include "m4/adv_r/adv_been.h"
#include "m4/core/errors.h"
#include "m4/graphics/gr_pal.h"
#include "m4/gui/gui_buffer.h"
#include "m4/gui/gui_dialog.h"
#include "m4/gui/gui_mouse.h"
#include "m4/gui/gui_sys.h"
#include "m4/gui/gui_vmng.h"
#include "m4/mem/mem.h"
#include "m4/detection.h"
#include "m4/param.h"

namespace M4 {

Globals *g_globals;

Globals::Globals() {
	g_globals = this;
}

Globals::~Globals() {
	game_systems_shutdown();

	sysfile_shutdown();
	player_been_shutdown();
	gui_system_shutdown();
	gui_buffer_system_shutdown();
	gui_dialog_shutdown();
	mem_stash_shutdown();
	param_shutdown();
	woodscript_shutdown();

	g_globals = nullptr;
}

bool Globals::init() {
	param_init();
	parse_all_flags();

	if (_G(system_shutting_down))
		return false;

	game_systems_initialize(INSTALL_ALL);

	if (!_kernel.hag_mode) {
		if (!read_catalog())
			error_show(FL, 'PARS', "game_main");
	}

	term_message("Load walker engines");
	if (!LoadWSAssets("walker script", &_master_palette[0]))
		error_show(FL, 'FNF!', "walker script");
	if (!LoadWSAssets("show script", &_master_palette[0]))
		error_show(FL, 'FNF!', "show script");
	if (!LoadWSAssets("stream script", &_master_palette[0]))
		error_show(FL, 'FNF!', "stream script");

	return true;
}


void Globals::game_systems_initialize(byte flags) {
	_G(term).init(_G(kernel).use_debug_monitor, _G(kernel).use_log_file);

	size_t totalMem = _G(kernel).mem_avail();
	if (_G(kernel).suppress_cache == CACHE_NOT_OVERRIDE_BY_FLAG_PARSE)
		_G(kernel).suppress_cache = totalMem < 8000000;

	debugC(kDebugCore, _G(kernel).suppress_cache ?
		"Cache System Disabled" : "Cache System Enabled");
	debugC(kDebugCore, "Available memory: %ld", totalMem);

	mem_stash_init(32);

	sysfile_init(_kernel.hag_mode);

	if (flags & INSTALL_PLAYER_BEEN_INIT) {
		if (!player_been_init(MAX_SCENES))
			error_show(FL, 'PBIF');
	}

	term_message("Firing up GUI");
	fire_up_gui();

	if (flags & INSTALL_SOUND_DRIVERS) {
		warning("TODO: sound init");
	} else {
		term_message("Sound driver installation skipped");
	}

	if (!woodscript_init())
		error_show(FL, 'WSIF');

	gr_pal_clear(_master_palette);

	// Start up rail system
	if (flags & INSTALL_RAIL_SYSTEM)
		InitRails();

	if (!f_stream_Init())
		error_show(FL, 'FSIF');

	mouse_set_sprite(kArrowCursor);

	if (flags & INSTALL_INVENTORY_SYSTEM)
		inv_init(128);	// Initialize with 128 objects

	if (flags & INSTALL_INVERSE_PALETTE)
		_inverse_pal = new InvPal("");	// Ceate an empty color table.
	else
		_inverse_pal = nullptr;
}

void Globals::game_systems_shutdown() {
	_system_shutting_down = true;
#ifdef TODO
	term_message("asset list be gone!");
	db_destroy_cat();
#endif
	rail_system_shutdown();

	term_message("fonts be gone!");
	gr_font_dealloc(_font_tiny_prop);
	gr_font_dealloc(_font_tiny);
	gr_font_dealloc(_font_line);
	gr_font_dealloc(_font_inter);
	gr_font_dealloc(_font_conv);
	gr_font_dealloc(_font_menu);
	gr_font_dealloc(_font_misc);
	gr_font_system_shutdown();
#ifdef TODO
	conv_reset_all(); //mar4 from mattp

	term_message("tin streams be gone!");
	f_stream_Shutdown();

	term_message("mouse dialog be gone!");
	DialogDestroy(mousePosDialog, NULL);

	term_message("sounds stop");
	midi_stop();
	digi_stop(1);
	digi_stop(2);
	digi_stop(3);

	if (_globals)
		mem_free(_globals);
	if (_globalCtrls)
		mem_free(_globalCtrls);
	if (_inverse_pal)
		delete (inverse_pal);

	term_message("big buffers be gone");
	if (_gameDrawBuff)
		delete _gameDrawBuff;

	term_message("SOS be gone");
	digi_uninstall();
	midi_uninstall();

	f_io_report(NULL, NULL);
	term_message("calling registry shutdown");
	registry_shutdown_all();
	term_shutdown();
#endif
}

void Globals::fire_up_gui() {
	if (!gui_system_init())
		error_show(FL, 'GUI0');
	if (!vmng_init())
		error_show(FL, 'GUI1');
	if (!gui_mouse_init())
		error_show(FL, 'GUI2');
	if (!gui_dialog_init())
		error_show(FL, 'GUI3');
#ifdef TODO
	if (!InitItems())
		error_show(FL, 'GUI4');
#endif
	if (!gui_buffer_system_init())
		error_show(FL, 'GUI5');
}

bool Globals::woodscript_init() {
	if (!InitWSAssets())
		return false;
	if (!ws_Initialize(_G(globals)))
		return false;

	return true;
}

void Globals::grab_fonts() {
	term_message("Grabbing fonts");

	_font_tiny_prop = gr_font_load("4X6PP.FNT");
	_font_tiny = gr_font_load("FONTTINY.FNT");
	_font_line = gr_font_load("FONTLINE.FNT");
	_font_inter = gr_font_load("FONTINTR.FNT");
	_font_conv = gr_font_load("FONTCONV.FNT");
	_font_menu = gr_font_load("FONTMENU.FNT");
	_font_misc = gr_font_load("FONTMISC.FNT");
}

void Globals::woodscript_shutdown() {
	ShutdownWSAssets();
	ws_Shutdown();
}

} // namespace M4
