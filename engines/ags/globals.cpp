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
#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/shared/ac/spritecache.h"
#include "ags/shared/debugging/debugmanager.h"
#include "ags/shared/font/fonts.h"
#include "ags/shared/font/ttffontrenderer.h"
#include "ags/shared/font/wfnfontrenderer.h"
#include "ags/shared/game/interactions.h"
#include "ags/shared/game/roomstruct.h"
#include "ags/shared/gui/guibutton.h"
#include "ags/shared/gui/guiinv.h"
#include "ags/shared/gui/guilabel.h"
#include "ags/shared/gui/guilistbox.h"
#include "ags/shared/gui/guislider.h"
#include "ags/shared/gui/guitextbox.h"
#include "ags/shared/script/cc_options.h"
#include "ags/shared/util/directory.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/draw_software.h"
#include "ags/engine/ac/event.h"
#include "ags/engine/ac/gamesetup.h"
#include "ags/engine/ac/gamestate.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/movelist.h"
#include "ags/engine/ac/objectcache.h"
#include "ags/engine/ac/roomstatus.h"
#include "ags/engine/ac/route_finder_jps.h"
#include "ags/engine/ac/screenoverlay.h"
#include "ags/engine/ac/spritelistentry.h"
#include "ags/engine/ac/topbarsettings.h"
#include "ags/engine/ac/dynobj/cc_audiochannel.h"
#include "ags/engine/ac/dynobj/cc_audioclip.h"
#include "ags/engine/ac/dynobj/cc_character.h"
#include "ags/engine/ac/dynobj/cc_dialog.h"
#include "ags/engine/ac/dynobj/cc_dynamicarray.h"
#include "ags/engine/ac/dynobj/cc_gui.h"
#include "ags/engine/ac/dynobj/cc_guiobject.h"
#include "ags/engine/ac/dynobj/cc_hotspot.h"
#include "ags/engine/ac/dynobj/cc_inventory.h"
#include "ags/engine/ac/dynobj/cc_object.h"
#include "ags/engine/ac/dynobj/cc_region.h"
#include "ags/engine/ac/dynobj/cc_serializer.h"
#include "ags/engine/ac/dynobj/managedobjectpool.h"
#include "ags/engine/ac/dynobj/scriptaudiochannel.h"
#include "ags/engine/ac/dynobj/scriptdialogoptionsrendering.h"
#include "ags/engine/ac/dynobj/scripthotspot.h"
#include "ags/engine/ac/dynobj/scriptinvitem.h"
#include "ags/engine/ac/dynobj/scriptobject.h"
#include "ags/engine/ac/dynobj/scriptregion.h"
#include "ags/engine/ac/dynobj/scriptstring.h"
#include "ags/engine/ac/dynobj/scriptsystem.h"
#include "ags/engine/ac/statobj/staticarray.h"
#include "ags/engine/debugging/consoleoutputtarget.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/engine/debugging/logfile.h"
#include "ags/engine/debugging/messagebuffer.h"
#include "ags/engine/device/mousew32.h"
#include "ags/engine/gfx/gfxfilter.h"
#include "ags/engine/gui/animatingguibutton.h"
#include "ags/engine/gui/cscidialog.h"
#include "ags/engine/gui/guidialogdefines.h"
#include "ags/engine/gui/newcontrol.h"
#include "ags/engine/main/graphics_mode.h"
#include "ags/engine/media/audio/ambientsound.h"
#include "ags/engine/media/audio/audiodefines.h"
#include "ags/engine/script/cc_instance.h"
#include "ags/engine/script/executingscript.h"
#include "ags/engine/script/nonblockingscriptfunction.h"
#include "ags/engine/script/script.h"
#include "ags/engine/script/systemimports.h"
#include "ags/lib/std/limits.h"
#include "ags/plugins/pluginobjectreader.h"
#include "common/file.h"

namespace AGS3 {

Globals *g_globals;

Globals::Globals() {
	g_globals = this;

	// agsplugin.cpp globals
	_glVirtualScreenWrap = new AGS::Shared::Bitmap();

	// agsstaticobject.cpp globals
	_GlobalStaticManager = new AGSStaticObject();
	_GameStaticManager = new StaticGame();

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

	// cscdialog.cpp globals
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

	// debugmanager.cpp globals
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
	_palette = new color[256];
	_maincoltable = new COLOR_MAP();

	// draw_software.cpp globals
	_BlackRects = new DirtyRects();
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
	_fontLines = new SplitLines();

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
	_GameScaling = new AGS::Engine::PlaneScaling();

	// guibutton.cpp globals
	_guibuts = new std::vector<AGS::Shared::GUIButton>();

	// guidlaog.cpp globals
	Common::fill(_filenumbers, _filenumbers + MAXSAVEGAMES_20, 0);
	Common::fill(_filedates, _filedates + MAXSAVEGAMES_20, 0);
	Common::fill(_bufTemp, _bufTemp + 260, 0);
	Common::fill(_buffer2, _buffer2 + 260, 0);
	Common::fill(_buff, _buff + 200, 0);
	_smes = new CSCIMessage();

	// guiinv.cpp globals
	_guiinv = new std::vector<AGS::Shared::GUIInvWindow>();

	// guilabel.cpp globals
	_guilabels = new std::vector<AGS::Shared::GUILabel>();

	// guilistbox.cpp globals
	_guilist = new std::vector<AGS::Shared::GUIListBox>();

	// guislider.cpp globals
	_guislider = new std::vector<AGS::Shared::GUISlider>();

	// guitextbox.cpp globals
	_guitext = new std::vector<AGS::Shared::GUITextBox>();

	// interactions.cpp globals
	_globalvars = new InteractionVariable[MAX_GLOBAL_VARIABLES];
	_globalvars[0] = InteractionVariable("Global 1", 0, 0);

	// managedobjectpool.cpp globals
	_pool = new ManagedObjectPool();

	// mouse.cpp globals
	_scmouse = new ScriptMouse();
	Common::fill(&_mousecurs[0], &_mousecurs[MAXCURSORS], nullptr);

	// mousew32.cpp globals
	_mouse = new Mouse();

	// overlay.cpp globals
	_screenover = new std::vector<ScreenOverlay>();

	// pluginobjectreader.cpp globals
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

	// systemimports.cpp globals
	_simp = new SystemImports();
	_simp_for_plugin = new SystemImports();

	// walkbehind.cpp globals
	Common::fill(_walkBehindLeft, _walkBehindLeft + MAX_WALK_BEHINDS, 0);
	Common::fill(_walkBehindTop, _walkBehindTop + MAX_WALK_BEHINDS, 0);
	Common::fill(_walkBehindRight, _walkBehindRight + MAX_WALK_BEHINDS, 0);
	Common::fill(_walkBehindBottom, _walkBehindBottom + MAX_WALK_BEHINDS, 0);
	Common::fill(_walkBehindBitmap, _walkBehindBitmap + MAX_WALK_BEHINDS,
		(AGS::Engine::IDriverDependantBitmap *)nullptr);
}

Globals::~Globals() {
	g_globals = nullptr;

	// agsplugin.cpp globals
	delete _glVirtualScreenWrap;

	// agsstaticobject.cpp globals
	delete _GlobalStaticManager;
	delete _GameStaticManager;

	// audio.cpp globals
	delete _audioChannels;
	delete _ambient;
	delete _scrAudioChannel;

	// button.cpp globals
	delete[] _animbuts;

	// cc_instance.cpp globals
	delete _GlobalReturnValue;
	delete _scriptDumpFile;

	// cc_serializer.cpp globals
	delete _ccUnserializer;

	// cc_dynamicarray.cpp globals
	delete _globalDynamicArray;

	// cscdialog.cpp globals
	delete[] _vobjs;
	delete[] _oswi;

	// debug.cpp globals
	delete[] _debug_line;
	delete _DebugMsgBuff;
	delete _DebugLogFile;
	delete _DebugConsole;

	// debugmanager.cpp globals
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

	// guibutton.cpp globals
	delete _guibuts;

	// guidlaog.cpp globals
	delete _smes;

	// guiinv.cpp globals
	delete _guiinv;

	// guilabel.cpp globals
	delete _guilabels;

	// guilistbox.cpp globals
	delete _guilist;

	// guislider.cpp globals
	delete _guislider;

	// guitextbox.cpp globals
	delete _guitext;

	// interactions.cpp globals
	delete[] _globalvars;

	// managedobjectpool.cpp globals
	delete _pool;

	// mouse.cpp globals
	delete _scmouse;

	// mousew32.cpp globals
	delete _mouse;

	// overlay.cpp globals
	delete _screenover;

	// pluginobjectreader.cpp globals
	delete[] _pluginReaders;

	// room.cpp globals
	delete _rgb_table;

	// route_finder_impl.cpp globals
	delete _navpoints;
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
}

} // namespace AGS3
