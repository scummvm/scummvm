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

#ifndef AGS_GLOBALS_H
#define AGS_GLOBALS_H

#include "ags/shared/util/string.h"
#include "ags/shared/util/version.h"
#include "ags/shared/gui/guimain.h"
#include "ags/shared/script/cc_script.h"
#include "ags/lib/std/array.h"
#include "ags/lib/std/set.h"

namespace AGS3 {

#define MAXCURSORS 20

using String = AGS::Shared::String;
using Version = AGS::Shared::Version;

namespace AGS {
namespace Shared {

class Bitmap;
class DebugManager;
struct Font;
class RoomStruct;

} // namespace Shared
} // namespace AGS

class SplitLines;
class TTFFontRenderer;
class WFNFontRenderer;

struct AmbientSound;
struct CCAudioChannel;
struct CCAudioClip;
struct CCCharacter;
struct CCDialog;
struct CCGUI;
struct CCGUIObject;
struct CCHotspot;
struct ccInstance;
struct CCInventory;
struct CCObject;
struct CCRegion;
struct CharacterCache;
struct ExecutingScript;
struct GameSetupStruct;
struct GameState;
struct IAGSEditorDebugger;
struct MoveList;
struct NonBlockingScriptFunction;
struct ObjectCache;
struct RoomCameraDrawData;
struct RoomStatus;
struct RuntimeScriptValue;
struct ScreenOverlay;
struct ScriptDialog;
struct ScriptGUI;
struct ScriptHotspot;
struct ScriptInvItem;
struct ScriptObject;
struct ScriptRegion;
struct ScriptString;
struct ScriptSystem;
struct SOUNDCLIP;
struct SpriteListEntry;
struct StaticArray;
struct ViewStruct;

class SpriteCache;

class Globals {
public:
	/**
	 * \defgroup Overall flags
	 * @{
	 */

	// Major overall flags
	bool _want_exit = false;
	bool _abort_engine = false;

	/**@}*/

	/**
	 * \defgroup audio globals
	 * @{
	 */

	std::array<SOUNDCLIP *> *_audioChannels;
	std::array<AmbientSound> *_ambient;

	/**@}*/

	/**
	 * \defgroup debug globals
	 * @{
	 */

	int _editor_debugging_enabled = 0;
	int _editor_debugging_initialized = 0;
	char _editor_debugger_instance_token[100];
	IAGSEditorDebugger *_editor_debugger = nullptr;
	int _break_on_next_script_step = 0;
	volatile int _game_paused_in_debugger = 0;

	struct Breakpoint {
		char scriptName[80] = { 0 };
		int lineNumber = 0;
	};

	std::vector<Breakpoint> _breakpoints;
	int _numBreakpoints = 0;

	/**@}*/

	/**
	 * \defgroup debug globals
	 * @{
	 */

	AGS::Shared::DebugManager *_DbgMgr;

	/**@}*/

	/**
	 * \defgroup dialog globals
	 * @{
	 */

	 // Old dialog support
	std::vector< std::shared_ptr<unsigned char> > _old_dialog_scripts;
	std::vector<String> _old_speech_lines;

	/**@}*/

	/**
	 * \defgroup draw globals
	 * @{
	 */

	std::vector<RoomCameraDrawData> *_CameraDrawData;
	std::vector<SpriteListEntry> *_sprlist;
	std::vector<SpriteListEntry> *_thingsToDrawList;

	/**@}*/

	/**
	 * \defgroup fonts globals
	 * @{
	 */

	std::vector<AGS::Shared::Font> *_fonts;
	TTFFontRenderer *_ttfRenderer;
	WFNFontRenderer *_wfnRenderer;
	SplitLines *_fontLines;

	/**@}*/

	/**
	 * \defgroup game globals
	 * @{
	 */

	GameSetupStruct *_game;
	GameState *_play;
	SpriteCache *_spriteset;
	AGS::Shared::RoomStruct *_thisroom;
	RoomStatus *_troom; // used for non-saveable rooms, eg. intro

	std::vector<AGS::Shared::GUIMain> *_guis;
	CCGUIObject *_ccDynamicGUIObject;
	CCCharacter *_ccDynamicCharacter;
	CCHotspot *_ccDynamicHotspot;
	CCRegion *_ccDynamicRegion;
	CCInventory *_ccDynamicInv;
	CCGUI *_ccDynamicGUI;
	CCObject *_ccDynamicObject;
	CCDialog *_ccDynamicDialog;
	CCAudioClip *_ccDynamicAudioClip;
	CCAudioChannel *_ccDynamicAudio;
	ScriptString *_myScriptStringImpl;

	// TODO: IMPORTANT!!
	// we cannot simply replace these arrays with vectors, or other C++ containers,
	// until we implement safe management of such containers in script exports
	// system. Noteably we would need an alternate to StaticArray class to track
	// access to their elements.
	ScriptObject *_scrObj;
	ScriptGUI *_scrGui = nullptr;
	ScriptHotspot *_scrHotspot;
	ScriptRegion *_scrRegion;
	ScriptInvItem *_scrInv;
	ScriptDialog *_scrDialog = nullptr;
	ViewStruct *_views = nullptr;
	CharacterCache *_charcache = nullptr;
	ObjectCache *_objcache;
	MoveList *_mls = nullptr;

	 /**@}*/

	/**
	 * \defgroup game_init globals
	 * @{
	 */

	StaticArray *_StaticCharacterArray;
	StaticArray *_StaticObjectArray;
	StaticArray *_StaticGUIArray;
	StaticArray *_StaticHotspotArray;
	StaticArray *_StaticRegionArray;
	StaticArray *_StaticInventoryArray;
	StaticArray *_StaticDialogArray;

	/**@}*/

	/**
	 * \defgroup main globals
	 * @{
	 */

	String _appDirectory; // Needed for library loading
	String _cmdGameDataPath;

	const char **_global_argv = nullptr;
	int _global_argc = 0;

	// Startup flags, set from parameters to engine
	int _force_window = 0;
	int _override_start_room = 0;
	bool _justDisplayHelp = false;
	bool _justDisplayVersion = false;
	bool _justRunSetup = false;
	bool _justRegisterGame = false;
	bool _justUnRegisterGame = false;
	bool _justTellInfo = false;
	std::set<String> _tellInfoKeys;
	int _loadSaveGameOnStartup = -1;

#if ! AGS_PLATFORM_DEFINES_PSP_VARS
	int _psp_video_framedrop = 1;
	int _psp_ignore_acsetup_cfg_file = 0;
	int _psp_clear_cache_on_room_change = 0; // clear --sprite cache-- when room is unloaded

#if AGS_PLATFORM_SCUMMVM
	int _psp_audio_cachesize = 10;
#endif
	const char *_psp_game_file_name = "";
	const char *_psp_translation = "default";

	int _psp_gfx_renderer = 0;
	int _psp_gfx_scaling = 1;
	int _psp_gfx_smoothing = 0;
	int _psp_gfx_super_sampling = 1;
	int _psp_gfx_smooth_sprites = 0;
#endif

	// Current engine version
	Version _EngineVersion;
	// Lowest savedgame version, accepted by this engine
	Version _SavedgameLowestBackwardCompatVersion;
	// Lowest engine version, which would accept current savedgames
	Version _SavedgameLowestForwardCompatVersion;

	/**@}*/

	/**
	 * \defgroup mouse globals
	 * @{
	 */

	char _currentcursor = 0;
	// virtual mouse cursor coordinates
	int _mousex = 0, _mousey = 0, _numcurso = -1, _hotx = 0, _hoty = 0;
	// real mouse coordinates and bounds
	int _real_mouse_x = 0, _real_mouse_y = 0;
	int _boundx1 = 0, _boundx2 = 99999, _boundy1 = 0, _boundy2 = 99999;
	int _disable_mgetgraphpos = 0;
	char _ignore_bounds = 0;
	AGS::Shared::Bitmap *_mousecurs[MAXCURSORS];

	/**@}*/

	/**
	 * \defgroup overlay globals
	 * @{
	 */

	std::vector<ScreenOverlay> *_screenover;
	int _is_complete_overlay = 0, _is_text_overlay = 0;

	/**@}*/

	/**
	 * \defgroup quit globals
	 * @{
	 */

	bool _handledErrorInEditor = false;
	char _return_to_roomedit[30] = { '\0' };
	char _return_to_room[150] = { '\0' };
	char _quit_message[256] = { '\0' };

	 /**@}*/

	/**
	 * \defgroup script globals
	 * @{
	 */

	ExecutingScript *_scripts;
	ExecutingScript *_curscript = nullptr;

	PScript *_gamescript;
	PScript *_dialogScriptsScript;
	ccInstance *_gameinst = nullptr, *_roominst = nullptr;
	ccInstance *_dialogScriptsInst = nullptr;
	ccInstance *_gameinstFork = nullptr, *_roominstFork = nullptr;

	int _num_scripts = 0;
	int _post_script_cleanup_stack = 0;

	int _inside_script = 0, _in_graph_script = 0;
	int _no_blocking_functions = 0; // set to 1 while in rep_Exec_always

	NonBlockingScriptFunction *_repExecAlways;
	NonBlockingScriptFunction *_lateRepExecAlways;
	NonBlockingScriptFunction *_getDialogOptionsDimensionsFunc;
	NonBlockingScriptFunction *_renderDialogOptionsFunc;
	NonBlockingScriptFunction *_getDialogOptionUnderCursorFunc;
	NonBlockingScriptFunction *_runDialogOptionMouseClickHandlerFunc;
	NonBlockingScriptFunction *_runDialogOptionKeyPressHandlerFunc;
	NonBlockingScriptFunction *_runDialogOptionRepExecFunc;

	ScriptSystem *_scsystem;

	std::vector<PScript> *_scriptModules;
	std::vector<ccInstance *> *_moduleInst;
	std::vector<ccInstance *> *_moduleInstFork;
	std::vector<RuntimeScriptValue> *_moduleRepExecAddr;
	int _numScriptModules = 0;

	// TODO: find out if these extra arrays are really necessary. This may be remains from the
	// time when the symbol import table was holding raw pointers to char array.
	std::vector<String> *_characterScriptObjNames;
	String *_objectScriptObjNames;
	std::vector<String> *_guiScriptObjNames;

	/**@}*/

public:
	Globals();
	~Globals();
};

extern Globals *g_globals;

// Macro for accessing a globals member
#define _G(FIELD) (::AGS3::g_globals->_##FIELD)
// Macro for accessing a globals member that was an object in the original,
// but is a pointer to the object in ScummVM, so that we don't need to
// provide the full class/struct definition here in the header file
#define _GP(FIELD) (*::AGS3::g_globals->_##FIELD)

} // namespace AGS3

#endif
