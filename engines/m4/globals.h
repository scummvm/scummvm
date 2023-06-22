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

#ifndef M4_GLOBALS_H
#define M4_GLOBALS_H

#include "m4/game.h"
#include "m4/kernel.h"
#include "m4/term.h"
#include "m4/adv_r/adv.h"
#include "m4/adv_r/adv_been.h"
#include "m4/adv_r/adv_rails.h"
#include "m4/fileio/fstream.h"
#include "m4/fileio/sys_file.h"
#include "m4/graphics/gr_font.h"
#include "m4/gui/gui_mouse.h"
#include "m4/gui/gui_univ.h"
#include "m4/mem/memman.h"
#include "m4/mem/res.h"
#include "m4/platform/timer.h"
#include "m4/wscript/wscript.h"

namespace M4 {

#define CACHE_NOT_OVERRIDE_BY_FLAG_PARSE 2

class Globals;

extern Globals *g_globals;

class Globals : public Mouse_Statics, public WS_Globals, public Timer_Globals {
private:
	void game_systems_initialize(byte flags);
	void fire_up_gui();
	bool woodscript_init();	
	void woodscript_shutdown();

public:
	Globals();
	~Globals();
	bool init();

	Game _game;
	Kernel _kernel;
	Term _term;
	Hag_Statics _hag;
	SceneDef _currentSceneDef;
	Scene_list _scene_list;
	frac16 _globals[GLB_SHARED_VARS];
	Resources _resources;
	Rails_Globals _rails;

	Font *_system_font = nullptr;
	Font *_font_line = nullptr;
	Font *_font_tiny_prop = nullptr;
	Font *_font_tiny = nullptr;
	Font *_font_inter = nullptr;
	Font *_font_conv = nullptr;
	Font *_font_menu = nullptr;
	Font *_font_misc = nullptr;
	Font *_interfaceFont = nullptr;
	Font *_font = nullptr;

	bool _system_shutting_down = false;
	size_t _mem_to_alloc = 0;
	void *_gameInterfaceBuff = nullptr;
	void *_custom_interface_setup = nullptr;
	void *_custom_interface_button_handler = nullptr;
	int _global_sound_room = 0;
	bool _interface_visible = false;
	bool _please_hyperwalk = false;
	void (*_custom_ascii_converter)(char *string) = nullptr;
	bool _vmng_Initted = false;
	ScreenContext *_frontScreen = nullptr;
	ScreenContext *_backScreen = nullptr;
	ScreenContext *_inactiveScreens = nullptr;
	// A list of "hot keys" which are activated by their keyboard event, if the event
	// has been passed right through the complete list of active windows.
	Hotkey *_systemHotkeys = nullptr;
	int32 _memtypeSCRN = 0;		// Memory slots used by the memory manager
	int32 _memtypeMATTE = 0;
	int32 _memtypeRECT = 0;
	int32 _sizeMem[_MEMTYPE_LIMIT] = { 0 };
	int32 _requests[_MEMTYPE_LIMIT] = { 0 };
	void *_memBlock[_MEMTYPE_LIMIT] = { nullptr };
	bool _okButton = false;
	bool _movingScreen = false;
	Item *_clickItem = nullptr;
	Item *_doubleClickItem = nullptr;
	char _listboxSearchStr[80] = { 0 };
	RGB8 _master_palette[256];
	strmRequest *_firstStream;
	strmRequest *_lastStream;
};

#define _G(X) (g_globals->_##X)

} // namespace M4

#endif
