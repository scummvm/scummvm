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

#include "ags/globals.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/shared/core/asset_manager.h"
#include "ags/shared/debugging/debug_manager.h"
#include "ags/shared/font/fonts.h"
#include "ags/shared/font/ttf_font_renderer.h"
#include "ags/shared/font/wfn_font_renderer.h"
#include "ags/shared/game/interactions.h"
#include "ags/shared/game/room_struct.h"
#include "ags/shared/game/tra_file.h"
#include "ags/shared/gui/gui_button.h"
#include "ags/shared/gui/gui_inv.h"
#include "ags/shared/gui/gui_label.h"
#include "ags/shared/gui/gui_listbox.h"
#include "ags/shared/gui/gui_slider.h"
#include "ags/shared/gui/gui_textbox.h"
#include "ags/shared/script/cc_options.h"
#include "ags/shared/util/directory.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/draw_software.h"
#include "ags/engine/ac/event.h"
#include "ags/engine/ac/game_setup.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/move_list.h"
#include "ags/engine/ac/object_cache.h"
#include "ags/engine/ac/room_status.h"
#include "ags/engine/ac/route_finder_jps.h"
#include "ags/engine/ac/screen_overlay.h"
#include "ags/engine/ac/sprite_list_entry.h"
#include "ags/engine/ac/top_bar_settings.h"
#include "ags/engine/ac/dynobj/cc_audio_channel.h"
#include "ags/engine/ac/dynobj/cc_audio_clip.h"
#include "ags/engine/ac/dynobj/cc_character.h"
#include "ags/engine/ac/dynobj/cc_dialog.h"
#include "ags/engine/ac/dynobj/cc_dynamic_array.h"
#include "ags/engine/ac/dynobj/cc_gui.h"
#include "ags/engine/ac/dynobj/cc_gui_object.h"
#include "ags/engine/ac/dynobj/cc_hotspot.h"
#include "ags/engine/ac/dynobj/cc_inventory.h"
#include "ags/engine/ac/dynobj/cc_object.h"
#include "ags/engine/ac/dynobj/cc_region.h"
#include "ags/engine/ac/dynobj/cc_serializer.h"
#include "ags/engine/ac/dynobj/managed_object_pool.h"
#include "ags/engine/ac/dynobj/script_audio_channel.h"
#include "ags/engine/ac/dynobj/script_dialog_options_rendering.h"
#include "ags/engine/ac/dynobj/script_hotspot.h"
#include "ags/engine/ac/dynobj/script_inv_item.h"
#include "ags/engine/ac/dynobj/script_object.h"
#include "ags/engine/ac/dynobj/script_region.h"
#include "ags/engine/ac/dynobj/script_string.h"
#include "ags/engine/ac/dynobj/script_system.h"
#include "ags/engine/ac/statobj/static_array.h"
#include "ags/engine/debugging/console_output_target.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/engine/debugging/log_file.h"
#include "ags/engine/debugging/message_buffer.h"
#include "ags/engine/device/mouse_w32.h"
#include "ags/engine/gfx/gfxfilter.h"
#include "ags/engine/gui/animating_gui_button.h"
#include "ags/engine/gui/csci_dialog.h"
#include "ags/engine/gui/gui_dialog_defines.h"
#include "ags/engine/gui/new_control.h"
#include "ags/engine/main/graphics_mode.h"
#include "ags/engine/media/audio/ambient_sound.h"
#include "ags/engine/media/audio/audio_defines.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/engine/script/cc_instance.h"
#include "ags/engine/script/executing_script.h"
#include "ags/engine/script/non_blocking_script_function.h"
#include "ags/engine/script/script.h"
#include "ags/engine/script/system_imports.h"
#include "ags/lib/std/limits.h"
#include "ags/plugins/ags_plugin.h"
#include "ags/plugins/plugin_object_reader.h"
#include "ags/plugins/core/core.h"
#include "common/file.h"

namespace AGS3 {

Globals *g_globals;

Globals::Globals() {
	g_globals = this;

	// Allegro globals
	Common::fill((byte *)&_black_palette, (byte *)&_black_palette + PAL_SIZE, 0);
	Common::fill((byte *)&_current_palette, (byte *)&_current_palette + PAL_SIZE, 0);
	Common::fill((byte *)&_prev_current_palette, (byte *)&_prev_current_palette + PAL_SIZE, 0);

	// ags_plugin.cpp globals
	_glVirtualScreenWrap = new AGS::Shared::Bitmap();

	// ags_static_object.cpp globals
	_GlobalStaticManager = new AGSStaticObject();
	_GameStaticManager = new StaticGame();

	// asset_manager.cpp globals
	_AssetMgr = new std::unique_ptr<Shared::AssetManager>();

	// audio.cpp globals
	_audioChannels = new std::array<SOUNDCLIP *>(MAX_SOUND_CHANNELS + 1);
	// TODO: double check that ambient sounds array actually needs +1
	_ambient = new std::array<AmbientSound>(MAX_SOUND_CHANNELS + 1);
	_scrAudioChannel = new ScriptAudioChannel[MAX_SOUND_CHANNELS + 1];

	// button.cpp globals
	_animbuts = new AnimatingGUIButton[MAX_ANIMATING_BUTTONS];

	// cc_instance.cpp globals
	_GlobalReturnValue = new RuntimeScriptValue();

	// cc_options.cpp globals
	_ccCompOptions = SCOPT_LEFTTORIGHT;

	// cc_serializer.cpp globals
	_ccUnserializer = new AGSDeSerializer();

	// cc_dynamicarray.cpp globals
	_globalDynamicArray = new CCDynamicArray();

	// csc_dialog.cpp globals
	_vobjs = new NewControl *[MAXCONTROLS];
	_oswi = new OnScreenWindow[MAXSCREENWINDOWS];
	Common::fill(_vobjs, _vobjs + MAXCONTROLS, (NewControl *)nullptr);
	_windowbackgroundcolor = COL254;
	_pushbuttondarkcolor = COL255;
	_pushbuttonlightcolor = COL253;

	// debug.cpp globals
	_fps = std::numeric_limits<float>::quiet_undefined();
	_display_fps = kFPS_Hide;
	_debug_line = new String[DEBUG_CONSOLE_NUMLINES];
	_DebugMsgBuff = new std::unique_ptr<AGS::Engine::MessageBuffer>();
	_DebugLogFile = new std::unique_ptr<AGS::Engine::LogFile>();
	_DebugConsole = new std::unique_ptr<AGS::Engine::ConsoleOutputTarget>();

	// debug_manager.cpp globals
	_DbgMgr = new AGS::Shared::DebugManager();

	// dialog.cpp globals
	_ccDialogOptionsRendering = new ScriptDialogOptionsRendering();

	// display.cpp globals
	_topBar = new TopBarSettings();

	// draw.cpp globals
	_CameraDrawData = new std::vector<RoomCameraDrawData>();
	_sprlist = new std::vector<SpriteListEntry>();
	_thingsToDrawList = new std::vector<SpriteListEntry>();
	_dynamicallyCreatedSurfaces = new AGS::Shared::Bitmap *[MAX_DYNAMIC_SURFACES];
	Common::fill(_dynamicallyCreatedSurfaces, _dynamicallyCreatedSurfaces +
	             MAX_DYNAMIC_SURFACES, (AGS::Shared::Bitmap *)nullptr);
	_maincoltable = new COLOR_MAP();
	_palette = new color[256];
	for (int i = 0; i < PALETTE_COUNT; ++i)
		_palette[i].clear();

	// draw_software.cpp globals
	_BlackRects = new DirtyRects();
	_GlobalOffs = new Point();
	_RoomCamRects = new std::vector<DirtyRects>();
	_RoomCamPositions = new std::vector<std::pair<int, int> >();

	// engine.cpp globals
	_ResPaths = new ResourcePaths();

	// event.cpp globals
	_event = new EventHappened[MAXEVENTS + 1];

	// fonts.cpp globals
	_fonts = new std::vector<AGS::Shared::Font>();
	_ttfRenderer = new TTFFontRenderer();
	_wfnRenderer = new WFNFontRenderer();
	_Lines = new SplitLines();

	// game.cpp globals
	_ccDynamicGUIObject = new CCGUIObject();
	_ccDynamicCharacter = new CCCharacter();
	_ccDynamicHotspot = new CCHotspot();
	_ccDynamicRegion = new CCRegion();
	_ccDynamicInv = new CCInventory();
	_ccDynamicGUI = new CCGUI();
	_ccDynamicObject = new CCObject();
	_ccDynamicDialog = new CCDialog();
	_ccDynamicAudioClip = new CCAudioClip();
	_ccDynamicAudio = new CCAudioChannel();
	_myScriptStringImpl = new ScriptString();
	_guis = new std::vector<AGS::Shared::GUIMain>();
	_play = new GameState();
	_game = new GameSetupStruct();
	_spriteset = new SpriteCache(_game->SpriteInfos);
	_thisroom = new AGS::Shared::RoomStruct();
	_troom = new RoomStatus();
	_usetup = new GameSetup();
	_scrObj = new ScriptObject[MAX_ROOM_OBJECTS];
	_scrHotspot = new ScriptHotspot[MAX_ROOM_HOTSPOTS];
	_scrRegion = new ScriptRegion[MAX_ROOM_REGIONS];
	_scrInv = new ScriptInvItem[MAX_INV];
	_objcache = new ObjectCache[MAX_ROOM_OBJECTS];
	_saveGameDirectory = AGS::Shared::SAVE_FOLDER_PREFIX;

	// game_init.cpp globals
	_StaticCharacterArray = new StaticArray();
	_StaticObjectArray = new StaticArray();
	_StaticGUIArray = new StaticArray();
	_StaticHotspotArray = new StaticArray();
	_StaticRegionArray = new StaticArray();
	_StaticInventoryArray = new StaticArray();
	_StaticDialogArray = new StaticArray();

	// gfxfilter_aad3d.cpp globals
	_aad3dFilterInfo = new AGS::Engine::GfxFilterInfo("Linear", "Linear interpolation");

	// gfxfilter_allegro.cpp globals
	_allegroFilterInfo = new AGS::Engine::GfxFilterInfo("StdScale", "Nearest-neighbour");

	// gfxfilter_d3d.cpp globals
	_d3dFilterInfo = new AGS::Engine::GfxFilterInfo("StdScale", "Nearest-neighbour");

	// gfxfilter_hqx.cpp globals
	_hqxFilterInfo = new AGS::Engine::GfxFilterInfo("Hqx", "Hqx (High Quality)", 2, 3);


	// global_dialog.cpp globals
	_last_in_dialog_request_script_pos = new ScriptPosition();

	// graphics_mode.cpp globals
	_SavedFullscreenSetting = new ActiveDisplaySetting();
	_SavedWindowedSetting = new ActiveDisplaySetting();
	_CurFrameSetup = new GameFrameSetup();
	_GameScaling = new AGS::Shared::PlaneScaling();

	// gui_button.cpp globals
	_guibuts = new std::vector<AGS::Shared::GUIButton>();

	// gui_dlaog.cpp globals
	Common::fill(_filenumbers, _filenumbers + MAXSAVEGAMES_20, 0);
	Common::fill(_filedates, _filedates + MAXSAVEGAMES_20, 0);
	Common::fill(_bufTemp, _bufTemp + 260, 0);
	Common::fill(_buffer2, _buffer2 + 260, 0);
	Common::fill(_buff, _buff + 200, 0);
	_smes = new CSCIMessage();

	// gui_inv.cpp globals
	_guiinv = new std::vector<AGS::Shared::GUIInvWindow>();

	// gui_label.cpp globals
	_guilabels = new std::vector<AGS::Shared::GUILabel>();

	// gui_listbox.cpp globals
	_guilist = new std::vector<AGS::Shared::GUIListBox>();

	// gui_slider.cpp globals
	_guislider = new std::vector<AGS::Shared::GUISlider>();

	// gui_textbox.cpp globals
	_guitext = new std::vector<AGS::Shared::GUITextBox>();

	// interactions.cpp globals
	_globalvars = new InteractionVariable[MAX_GLOBAL_VARIABLES];
	_globalvars[0] = InteractionVariable("Global 1", 0, 0);

	// managed_object_pool.cpp globals
	_pool = new ManagedObjectPool();

	// mouse.cpp globals
	_scmouse = new ScriptMouse();
	Common::fill(&_mousecurs[0], &_mousecurs[MAXCURSORS], nullptr);

	// mouse_w32.cpp globals
	_mouse = new Mouse();

	// overlay.cpp globals
	_screenover = new ScreenOverlay[MAX_SCREEN_OVERLAYS];

	// plugins globals
	_engineExports = new Plugins::Core::EngineExports();
	_plugins = new Common::Array<EnginePlugin>();
	_plugins->reserve(MAXPLUGINS);

	// plugin_object_reader.cpp globals
	_pluginReaders = new PluginObjectReader[MAX_PLUGIN_OBJECT_READERS];

	// room.cpp globals
	_rgb_table = new RGB_MAP();

	// route_finder_impl.cpp globals
	_navpoints = new int32_t[MAXNEEDSTAGES];
	_nav = new Navigation();

	// screen.cpp globals
	_old_palette = new color[256];

	// script.cpp globals
	_scripts = new ExecutingScript[MAX_SCRIPT_AT_ONCE];
	_gamescript = new PScript();
	_dialogScriptsScript = new PScript();
	_repExecAlways = new NonBlockingScriptFunction(REP_EXEC_ALWAYS_NAME, 0);
	_lateRepExecAlways = new NonBlockingScriptFunction(LATE_REP_EXEC_ALWAYS_NAME, 0);
	_getDialogOptionsDimensionsFunc = new NonBlockingScriptFunction("dialog_options_get_dimensions", 1);
	_renderDialogOptionsFunc = new NonBlockingScriptFunction("dialog_options_render", 1);
	_getDialogOptionUnderCursorFunc = new NonBlockingScriptFunction("dialog_options_get_active", 1);
	_runDialogOptionMouseClickHandlerFunc = new NonBlockingScriptFunction("dialog_options_mouse_click", 2);
	_runDialogOptionKeyPressHandlerFunc = new NonBlockingScriptFunction("dialog_options_key_press", 2);
	_runDialogOptionRepExecFunc = new NonBlockingScriptFunction("dialog_options_repexec", 1);
	_scsystem = new ScriptSystem();
	_scriptModules = new std::vector<PScript>();
	_moduleInst = new std::vector<ccInstance *>();
	_moduleInstFork = new std::vector<ccInstance *>();
	_moduleRepExecAddr = new std::vector<RuntimeScriptValue>();
	_characterScriptObjNames = new std::vector<String>();
	_objectScriptObjNames = new String[MAX_ROOM_OBJECTS];
	_guiScriptObjNames = new std::vector<String>();

	// script_runtime.cpp globals
	Common::fill(_loadedInstances, _loadedInstances + MAX_LOADED_INSTANCES,
	             (ccInstance *)nullptr);

	// system_imports.cpp globals
	_simp = new SystemImports();
	_simp_for_plugin = new SystemImports();

	// translation.cpp globals
	_trans = new AGS::Shared::Translation();
	_transtree = new StringMap();

	// walk_behind.cpp globals
	Common::fill(_walkBehindLeft, _walkBehindLeft + MAX_WALK_BEHINDS, 0);
	Common::fill(_walkBehindTop, _walkBehindTop + MAX_WALK_BEHINDS, 0);
	Common::fill(_walkBehindRight, _walkBehindRight + MAX_WALK_BEHINDS, 0);
	Common::fill(_walkBehindBottom, _walkBehindBottom + MAX_WALK_BEHINDS, 0);
	Common::fill(_walkBehindBitmap, _walkBehindBitmap + MAX_WALK_BEHINDS,
		(AGS::Engine::IDriverDependantBitmap *)nullptr);
}

Globals::~Globals() {
	g_globals = nullptr;

	// ags_platform_driver.cpp globals
	delete _platform;

	// ags_plugin.cpp globals
	delete _glVirtualScreenWrap;

	// ags_static_object.cpp globals
	delete _GlobalStaticManager;
	delete _GameStaticManager;

	// asset_manager.cpp globals
	delete _AssetMgr;

	// audio.cpp globals
	delete _audioChannels;
	delete _ambient;
	delete[] _scrAudioChannel;

	// button.cpp globals
	delete[] _animbuts;

	// cc_instance.cpp globals
	delete _GlobalReturnValue;
	delete _scriptDumpFile;

	// cc_serializer.cpp globals
	delete _ccUnserializer;

	// cc_dynamic_array.cpp globals
	delete _globalDynamicArray;

	// cscdialog.cpp globals
	delete[] _vobjs;
	delete[] _oswi;

	// debug.cpp globals
	delete[] _debug_line;
	delete _DebugMsgBuff;
	delete _DebugLogFile;
	delete _DebugConsole;

	// debug_manager.cpp globals
	delete _DbgMgr;

	// dialog.cpp globals
	delete _ccDialogOptionsRendering;

	// display.cpp globals
	delete _topBar;

	// draw.cpp globals
	delete _CameraDrawData;
	delete _sprlist;
	delete _thingsToDrawList;
	delete[] _dynamicallyCreatedSurfaces;
	delete[] _palette;
	delete _maincoltable;

	// draw_software.cpp globals
	delete _BlackRects;
	delete _GlobalOffs;
	delete _RoomCamRects;
	delete _RoomCamPositions;

	// engine.cpp globals
	delete _ResPaths;

	// event.cpp globals
	delete[] _event;

	// fonts.cpp globals
	delete _fonts;
	delete _ttfRenderer;
	delete _wfnRenderer;
	delete _Lines;

	// game.cpp globals
	delete _ccDynamicGUIObject;
	delete _ccDynamicCharacter;
	delete _ccDynamicHotspot;
	delete _ccDynamicRegion;
	delete _ccDynamicInv;
	delete _ccDynamicGUI;
	delete _ccDynamicObject;
	delete _ccDynamicDialog;
	delete _ccDynamicAudioClip;
	delete _ccDynamicAudio;
	delete _myScriptStringImpl;
	delete _guis;
	delete _game;
	delete _play;
	delete _spriteset;
	delete _thisroom;
	delete _troom;
	delete _usetup;
	delete[] _scrObj;
	delete[] _scrHotspot;
	delete[] _scrRegion;
	delete[] _scrInv;
	delete[] _objcache;

	// game_init.cpp globals
	delete _StaticCharacterArray;
	delete _StaticObjectArray;
	delete _StaticGUIArray;
	delete _StaticHotspotArray;
	delete _StaticRegionArray;
	delete _StaticInventoryArray;
	delete _StaticDialogArray;

	// gfxfilter_aad3d.cpp globals
	delete _aad3dFilterInfo;

	// gfxfilter_allegro.cpp globals
	delete _allegroFilterInfo;

	// gfxfilter_d3d.cpp globals
	delete _d3dFilterInfo;

	// gfxfilter_hqx.cpp globals
	delete _hqxFilterInfo;

	// global_dialog.cpp globals
	delete _last_in_dialog_request_script_pos;

	// graphics_mode.cpp globals
	delete _SavedFullscreenSetting;
	delete _SavedWindowedSetting;
	delete _CurFrameSetup;
	delete _GameScaling;

	// gui_button.cpp globals
	delete _guibuts;

	// gui_dlaog.cpp globals
	delete _smes;

	// gui_inv.cpp globals
	delete _guiinv;

	// gui_label.cpp globals
	delete _guilabels;

	// gui_listbox.cpp globals
	delete _guilist;

	// gui_slider.cpp globals
	delete _guislider;

	// gui_textbox.cpp globals
	delete _guitext;

	// interactions.cpp globals
	delete[] _globalvars;

	// managed_object_pool.cpp globals
	delete _pool;

	// mouse.cpp globals
	delete _scmouse;

	// mouse_w32.cpp globals
	delete _mouse;

	// overlay.cpp globals
	delete[] _screenover;

	// plugins globals
	delete _engineExports;
	delete _plugins;

	// plugin_object_reader.cpp globals
	delete[] _pluginReaders;

	// room.cpp globals
	delete _rgb_table;

	// route_finder_impl.cpp globals
	delete[] _navpoints;
	delete _nav;

	// screen.cpp globals
	delete[] _old_palette;

	// script.cpp globals
	delete[] _scripts;
	delete _gamescript;
	delete _dialogScriptsScript;
	delete _repExecAlways;
	delete _lateRepExecAlways;
	delete _getDialogOptionsDimensionsFunc;
	delete _renderDialogOptionsFunc;
	delete _getDialogOptionUnderCursorFunc;
	delete _runDialogOptionMouseClickHandlerFunc;
	delete _runDialogOptionKeyPressHandlerFunc;
	delete _runDialogOptionRepExecFunc;
	delete _scsystem;
	delete _scriptModules;
	delete _moduleInst;
	delete _moduleInstFork;
	delete _moduleRepExecAddr;
	delete _characterScriptObjNames;
	delete[] _objectScriptObjNames;
	delete _guiScriptObjNames;

	// system_imports.cpp globals
	delete _simp;
	delete _simp_for_plugin;

	// translation.cpp globals
	delete _trans;
	delete _transtree;
}

} // namespace AGS3
