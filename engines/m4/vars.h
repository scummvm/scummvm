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

#ifndef M4_VARS_H
#define M4_VARS_H

#include "m4/adv_r/kernel.h"
#include "m4/core/globals.h"
#include "m4/core/term.h"
#include "m4/adv_db_r/db_catalog.h"
#include "m4/adv_r/adv.h"
#include "m4/adv_r/adv_been.h"
#include "m4/adv_r/adv_game.h"
#include "m4/adv_r/adv_interface.h"
#include "m4/adv_r/adv_inv.h"
#include "m4/adv_r/adv_player.h"
#include "m4/adv_r/adv_rails.h"
#include "m4/adv_r/adv_scale.h"
#include "m4/adv_r/adv_trigger.h"
#include "m4/adv_r/adv_walk.h"
#include "m4/adv_r/conv_io.h"
#include "m4/core/globals.h"
#include "m4/core/mouse.h"
#include "m4/fileio/fstream.h"
#include "m4/fileio/sys_file.h"
#include "m4/graphics/gr_color.h"
#include "m4/graphics/gr_font.h"
#include "m4/graphics/krn_pal.h"
#include "m4/graphics/rend.h"
#include "m4/gui/gui_dialog.h"
#include "m4/gui/gui_item.h"
#include "m4/gui/gui_mouse.h"
#include "m4/gui/gui_univ.h"
#include "m4/gui/hotkeys.h"
#include "m4/mem/memman.h"
#include "m4/mem/res.h"
#include "m4/platform/events.h"
#include "m4/platform/timer.h"
#include "m4/platform/sound/digi.h"
#include "m4/platform/sound/midi.h"
#include "m4/wscript/wscript.h"

namespace M4 {

#define CACHE_NOT_OVERRIDE_BY_FLAG_PARSE 2

class Vars;

extern Vars *g_vars;

class Vars : public Mouse_Globals {
private:
	void game_systems_initialize(byte flags);
	void game_systems_shutdown();
	void fire_up_gui();
	bool woodscript_init();
	void woodscript_shutdown();
	void grab_fonts();
	void create_mouse_watch_dialog();

protected:
	void initMouseSeries(const Common::String &assetName, RGB8 *myPalette);
	virtual void main_cold_data_init() = 0;

public:
	Vars();
	virtual ~Vars();
	bool init();

	virtual Interface *getInterface() = 0;
	virtual Walker *getWalker() = 0;
	virtual Hotkeys *getHotkeys() = 0;
	virtual void global_menu_system_init() = 0;
	virtual void initialize_game() = 0;

	Events _events;
	MouseInfo &_MouseState = _events;
	GameControl _game;
	Kernel _kernel;
	Term _term;
	Hag_Statics _hag;
	SceneDef _currentSceneDef;
	Scene_list _scene_list;
	frac16 _globals[GLB_SHARED_VARS];
	Player _player;
	PlayerInfo _player_info;
	Resources _resources;
	Rails_Globals _rails;
	Catalog _catalog;
	Dialog_Globals _dialog;
	Item_Globals _items;
	Converstation_Globals _conversations;
	WS_Globals _ws;
	Triggers _triggers;
	Sound::Digi _digi;
	Sound::Midi _midi;
	KernelPal_Globals _krnPal;
	ADVScale_Globals _scale;
	ConvDisplayData _cdd;
	Rend_Globals _rend;

	const bool _cheating_enabled = true;

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
	InventoryBase *_inventory = nullptr;
	size_t _mem_to_alloc = 0;
	GrBuff *_screenCodeBuff = nullptr;
	GrBuff *_game_bgBuff = nullptr;
	GrBuff *_gameDrawBuff = nullptr;
	GrBuff *_gameInterfaceBuff = nullptr;
	void *_custom_interface_setup = nullptr;
	void *_custom_interface_button_handler = nullptr;
	int _global_sound_room = 0;
	CursorChange _toggle_cursor = CURSCHANGE_NONE;
	bool _i_just_hyperwalked = false;
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
	int32 _sizeMem[_MEMTYPE_LIMIT];
	int32 _requests[_MEMTYPE_LIMIT];
	void *_memBlock[_MEMTYPE_LIMIT];
	RGB8 _master_palette[256];
	RGB8 _backup_palette[256];
	bool _pal_fade_in_progress = false;
	strmRequest *_firstStream = nullptr;
	strmRequest *_lastStream = nullptr;
	InvPal *_inverse_pal = nullptr;
	int32 _inv_obj_mem_type = 0;
	bool _currScreen = false;
	ScreenContext *_eventToScreen = nullptr;
	byte _color = 0;
	Dialog *_mousePosDialog = nullptr;
	bool _showMousePos = false;
	bool _inv_suppress_click_sound = false;
	bool _between_rooms = false;
	bool _completeWalk = false;
	const char *_art_base_override = nullptr;
	bool _use_alternate_attribute_file = false;
	bool _camera_reacts_to_player = false;
	bool _set_commands_allowed_since_last_checked = false;
	SceneDef *_myDef = nullptr;
	Common::String _def_filename;
	Common::String _currBackgroundFN;
	Common::String _currCodeFN;
	ScreenContext *_game_buff_ptr = nullptr;
	machine *_my_walker = nullptr;
	uint32 _editors_in_use = 0;
	bool _editor_showStats = false;
	bool _shut_down_digi_tracks_between_rooms = false;
	cursor_states _cursor_state = kARROW;
	int _iPitch = 0;
	bool _hyperwalk = false;
};

#define _G(X) (g_vars->_##X)
#define _GI() (*g_vars->getInterface())
#define _GW() (*g_vars->getWalker())
#define _GWS(X) _G(ws)._##X
#define INTERFACE_VISIBLE g_vars->getInterface()->_visible

} // namespace M4

#endif
