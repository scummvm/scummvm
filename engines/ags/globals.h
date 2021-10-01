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

#include "ags/shared/ac/game_version.h"
#include "ags/shared/util/stdio_compat.h"
#include "ags/shared/util/string.h"
#include "ags/shared/util/string_types.h"
#include "ags/shared/util/version.h"
#include "ags/shared/gui/gui_main.h"
#include "ags/shared/script/cc_script.h"
#include "ags/engine/ac/runtime_defines.h"
#include "ags/engine/ac/walk_behind.h"
#include "ags/engine/main/engine.h"
#include "ags/engine/media/audio/audio_defines.h"
#include "ags/engine/script/script.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/lib/std/array.h"
#include "ags/lib/std/chrono.h"
#include "ags/lib/std/memory.h"
#include "ags/lib/std/set.h"
#include "ags/lib/allegro/color.h"
#include "ags/lib/allegro/fixed.h"
#include "ags/lib/allegro/aintern.h"
#include "common/events.h"

namespace Common {
class DumpFile;
}

namespace AGS3 {

#define MAXCURSORS 20

using String = AGS::Shared::String;
using Version = AGS::Shared::Version;
using StringMap = AGS::Shared::StringMap;

namespace AGS {
namespace Shared {

class AssetManager;
class Bitmap;
class DebugManager;
struct Font;
class GUIButton;
class GUIInvWindow;
class GUILabel;
class GUIListBox;
class GUISlider;
class GUITextBox;
struct InteractionVariable;
struct PlaneScaling;
class RoomStruct;
struct Translation;

} // namespace Shared

namespace Engine {

class ConsoleOutputTarget;
struct GfxFilterInfo;
class IDriverDependantBitmap;
class IGfxDriverFactory;
class IGraphicsDriver;
class LogFile;
class MessageBuffer;

} // namespace Engine
} // namespace AGS

namespace Plugins {
namespace Core {
class EngineExports;
} // namespace Core
} // namespace Plugins

class Navigation;
class SplitLines;
class SpriteCache;
class TTFFontRenderer;
class WFNFontRenderer;

struct ActiveDisplaySetting;
struct AGSDeSerializer;
struct AGSPlatformDriver;
struct AGSStaticObject;
struct AmbientSound;
struct AnimatingGUIButton;
struct CachedActSpsData;
struct CCAudioChannel;
struct CCAudioClip;
struct CCCharacter;
struct CCDialog;
struct CCDynamicArray;
struct CCGUI;
struct CCGUIObject;
struct CCHotspot;
struct ccInstance;
struct CCInventory;
struct CCObject;
struct CCRegion;
struct CharacterCache;
struct CharacterExtras;
struct CharacterInfo;
struct color;
struct COLOR_MAP;
struct CSCIMessage;
struct DialogTopic;
struct DirtyRects;
struct EnginePlugin;
struct ExecutingScript;
struct EventHappened;
struct GameFrameSetup;
struct GameSetup;
struct GameSetupStruct;
struct GameState;
struct IAGSEditorDebugger;
struct ICCStringClass;
struct ManagedObjectPool;
struct Mouse;
struct MoveList;
struct NewControl;
struct NonBlockingScriptFunction;
struct ObjectCache;
struct OnScreenWindow;
struct PluginObjectReader;
struct Point;
struct ResourcePaths;
struct RGB_MAP;
struct RoomCameraDrawData;
struct RoomObject;
struct RoomStatus;
struct RuntimeScriptValue;
struct ScreenOverlay;
struct ScriptAudioChannel;
struct ScriptDialog;
struct ScriptDialogOptionsRendering;
struct ScriptDrawingSurface;
struct ScriptGUI;
struct ScriptHotspot;
struct ScriptInvItem;
struct ScriptMouse;
struct ScriptObject;
struct ScriptPosition;
struct ScriptRegion;
struct ScriptString;
struct ScriptSystem;
struct SOUNDCLIP;
struct SpeechLipSyncLine;
struct SpriteListEntry;
struct StaticArray;
struct StaticGame;
struct SystemImports;
struct TopBarSettings;
struct ViewStruct;

class Globals {
public:
	/**
	 * @defgroup agsglobals AGS Globals
	 * @ingroup agsengine
	 * @brief Globals for the AGS engine
	 */

	/**
	 * @defgroup agsallegroglobals Allegro globals
	 * @ingroup agsglobals
	 * @{
	 */

	int _errnum = 0;
	int *_allegro_errno = &_errnum;

	int __color_depth = 0;
	int __rgb_r_shift_15 = DEFAULT_RGB_R_SHIFT_15;     /* truecolor pixel format */
	int __rgb_g_shift_15 = DEFAULT_RGB_G_SHIFT_15;
	int __rgb_b_shift_15 = DEFAULT_RGB_B_SHIFT_15;
	int __rgb_r_shift_16 = DEFAULT_RGB_R_SHIFT_16;
	int __rgb_g_shift_16 = DEFAULT_RGB_G_SHIFT_16;
	int __rgb_b_shift_16 = DEFAULT_RGB_B_SHIFT_16;
	int __rgb_r_shift_24 = DEFAULT_RGB_R_SHIFT_24;
	int __rgb_g_shift_24 = DEFAULT_RGB_G_SHIFT_24;
	int __rgb_b_shift_24 = DEFAULT_RGB_B_SHIFT_24;
	int __rgb_r_shift_32 = DEFAULT_RGB_R_SHIFT_32;
	int __rgb_g_shift_32 = DEFAULT_RGB_G_SHIFT_32;
	int __rgb_b_shift_32 = DEFAULT_RGB_B_SHIFT_32;
	int __rgb_a_shift_32 = DEFAULT_RGB_A_SHIFT_32;

	RGB_MAP *_rgb_map = nullptr;
	COLOR_MAP *_color_map = nullptr;
	int _trans_blend_alpha = 0;
	int _trans_blend_red = 0;
	int _trans_blend_green = 0;
	int _trans_blend_blue = 0;
	BlenderMode __blender_mode = kRgbToRgbBlender;
	/* current format information and worker routines */
	int _utype = U_UTF8;

	/* default palette structures */
	PALETTE _black_palette;
	PALETTE _current_palette;
	PALETTE _prev_current_palette;

	volatile int _mouse_x = 0;  // X position
	volatile int _mouse_y = 0;  // Y position
	volatile int _mouse_z = 0;  // Mouse wheel vertical
	volatile int _mouse_b = 0;  // Mouse buttons bitflags
	volatile int _mouse_pos = 0;    // X position in upper 16 bits, Y in lower 16
	volatile int _sys_mouse_x = 0; // mouse x position
	volatile int _sys_mouse_y = 0; // mouse y position
	volatile int _sys_mouse_z = 0; // mouse wheel position
	volatile int _freeze_mouse_flag = 0;

	int _mouse_button_state = 0;
	int _mouse_accum_button_state = 0;
	uint32 _mouse_clear_at_time = 0;
	int _mouse_accum_relx = 0, _mouse_accum_rely = 0;
	int _wasbutdown = 0, _wasongui = 0;

	/**@}*/

	/**
	 * @defgroup agsstaticobjectglobals agsstaticobject globals
	 * @ingroup agsglobals
	 * @{
	 */

	std::unique_ptr<Shared::AssetManager> *_AssetMgr;

	/**@}*/

	/**
	 * @defgroup agsplatformdriverglobals agsplatformdriver globals
	 * @ingroup agsglobals
	 * @{
	 */

	// We don't have many places where we delay longer than a frame, but where we
	// do, we should give the audio layer a chance to update.
	// 16 milliseconds is rough period for 60fps
	const std::chrono::milliseconds _MaximumDelayBetweenPolling = std::chrono::milliseconds(16);

	/**@}*/

	/**
	 * @defgroup agspluginglobals agsplugin globals
	 * @ingroup agsglobals
	 * @{
	 */

	AGS::Shared::Bitmap *_glVirtualScreenWrap;

	/**@}*/

	/**
	 * @defgroup agsstaticobjectglobals agsstaticobject globals
	 * @ingroup agsglobals
	 * @{
	 */

	AGSStaticObject *_GlobalStaticManager;
	StaticGame      *_GameStaticManager;

	/**@}*/

	/**
	 * @defgroup agsaudioglobals audio globals
	 * @ingroup agsglobals
	 * @{
	 */

	std::array<SOUNDCLIP *> *_audioChannels;
	std::array<AmbientSound> *_ambient;

	volatile bool _audio_doing_crossfade = false;
	ScriptAudioChannel *_scrAudioChannel;
	char _acaudio_buffer[256];
	int _reserved_channel_count = 0;

	// This is an indicator of a music played by an old audio system
	// (to distinguish from the new system API)
	int _current_music_type = 0;
	// _G(crossFading) is >0 (channel number of new track), or -1 (old
	// track fading out, no new track)
	int _crossFading = 0, _crossFadeVolumePerStep = 0, _crossFadeStep = 0;
	int _crossFadeVolumeAtStart = 0;
	SOUNDCLIP *_cachedQueuedMusic = nullptr;

	// Music update is scheduled when the voice speech stops;
	// we do a small delay before reverting any volume adjustments
	bool _music_update_scheduled = false;
	uint32 _music_update_at = 0;

	/**@}*/

	/**
	 * @defgroup agsbuttonglobals button globals
	 * @ingroup agsglobals
	 * @{
	 */

	AnimatingGUIButton *_animbuts;
	int _numAnimButs = 0;

	/**@}*/

	/**
	 * @defgroup agscc_dynamicarrayglobals cc_dynamicarray globals
	 * @ingroup agsglobals
	 * @{
	 */

	CCDynamicArray *_globalDynamicArray;

	/**@}*/

	/**
	 * @defgroup agscc_dynamicobjectglobals cc_dynamicobject globals
	 * @ingroup agsglobals
	 * @{
	 */

	ICCStringClass *_stringClassImpl = nullptr;

	/**@}*/

	/**
	 * @defgroup agscc_errorglobals cc_error globals
	 * @ingroup agsglobals
	 * @{
	 */

	int _ccError = 0;
	int _ccErrorLine = 0;
	String _ccErrorString;
	String _ccErrorCallStack;
	bool _ccErrorIsUserError = false;
	const char *_ccCurScriptName = "";

	/**@}*/

	/**
	 * @defgroup agscc_instanceglobals cc_instance globals
	 * @ingroup agsglobals
	 * @{
	 */

	ccInstance *_current_instance = nullptr;
	// [IKM] 2012-10-21:
	// NOTE: This is temporary solution (*sigh*, one of many) which allows certain
	// exported functions return value as a RuntimeScriptValue object;
	// Of 2012-12-20: now used only for plugin exports
	RuntimeScriptValue *_GlobalReturnValue;
	Common::DumpFile *_scriptDumpFile = nullptr;

	/**@}*/

	/**
	 * @defgroup agscc_optionsglobals cc_options globals
	 * @ingroup agsglobals
	 * @{
	 */

	int _ccCompOptions;

	/**@}*/

	/**
	 * @defgroup agscc_scriptglobals cc_script globals
	 * @ingroup agsglobals
	 * @{
	 */

	// currently executed line
	int _currentline = 0;
	// script file format signature
	const char *_scfilesig = "SCOM";

	/**@}*/

	/**
	 * @defgroup agscc_serializerglobals cc_serializer globals
	 * @ingroup agsglobals
	 * @{
	 */

	AGSDeSerializer *_ccUnserializer;

	/**@}*/

	/**
	 * @defgroup agscharacterglobals character globals
	 * @ingroup agsglobals
	 * @{
	 */

	int _use_cdplayer = 0;
	bool _triedToUseCdAudioCommand = false;
	int _need_to_stop_cd = 0;

	/**@}*/

	/**
	 * @defgroup agscharacterglobals character globals
	 * @ingroup agsglobals
	 * @{
	 */

	CharacterExtras *_charextra = nullptr;
	CharacterInfo *_playerchar = nullptr;
	int32_t _sc_PlayerCharPtr = 0;
	int _char_lowest_yp = 0;

	// Sierra-style speech settings
	int _face_talking = -1, _facetalkview = 0, _facetalkwait = 0, _facetalkframe = 0;
	int _facetalkloop = 0, _facetalkrepeat = 0, _facetalkAllowBlink = 1;
	int _facetalkBlinkLoop = 0;
	CharacterInfo *_facetalkchar = nullptr;
	// Do override default portrait position during QFG4-style speech overlay update
	bool _facetalk_qfg4_override_placement_x = false;
	bool _facetalk_qfg4_override_placement_y = false;

	// lip-sync speech settings
	int _loops_per_character = 0, _text_lips_offset = 0, _char_speaking = -1;
	int _char_thinking = -1;
	const char *_text_lips_text = nullptr;
	SpeechLipSyncLine *_splipsync = nullptr;
	int _numLipLines = 0, _curLipLine = -1, _curLipLinePhoneme = 0;

	/**@}*/

	/**
	 * @defgroup agscscdialogglobals cscdialog globals
	 * @ingroup agsglobals
	 * @{
	 */

	int _windowbackgroundcolor, _pushbuttondarkcolor;
	int _pushbuttonlightcolor;
	int _topwindowhandle = -1;
	int _cbuttfont = 0;
	int _acdialog_font = 0;
	int _smcode = 0;
	int _controlid = 0;
	NewControl **_vobjs;
	OnScreenWindow *_oswi;

	int _windowcount = 0, _curswas = 0;
	int _win_x = 0, _win_y = 0, _win_width = 0, _win_height = 0;

	/**@}*/

	/**
	 * @defgroup agsdebugglobals debug globals
	 * @ingroup agsglobals
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

	int _debug_flags = 0;

	String *_debug_line;
	int _first_debug_line = 0, _last_debug_line = 0, _display_console = 0;

	float _fps;
	int _display_fps;
	std::unique_ptr<AGS::Engine::MessageBuffer> *_DebugMsgBuff;
	std::unique_ptr<AGS::Engine::LogFile> *_DebugLogFile;
	std::unique_ptr<AGS::Engine::ConsoleOutputTarget> *_DebugConsole;

	/**@}*/

	/**
	 * @defgroup agsdebugglobals debug globals
	 * @ingroup agsglobals
	 * @{
	 */

	AGS::Shared::DebugManager *_DbgMgr;

	/**@}*/

	/**
	 * @defgroup agsdialogglobals dialog globals
	 * @ingroup agsglobals
	 * @{
	 */

	DialogTopic *_dialog = nullptr;
	ScriptDialogOptionsRendering *_ccDialogOptionsRendering;
	ScriptDrawingSurface *_dialogOptionsRenderingSurface = nullptr;

	int _said_speech_line = 0; // used while in dialog to track whether screen needs updating
	int _said_text = 0;
	int _longestline = 0;
	// Old dialog support
	std::vector< std::shared_ptr<unsigned char> > _old_dialog_scripts;
	std::vector<String> _old_speech_lines;

	/**@}*/

	/**
	 * @defgroup agsdisplayglobals display globals
	 * @ingroup agsglobals
	 * @{
	 */

	int _display_message_aschar = 0;
	int _source_text_length = -1;

	TopBarSettings *_topBar;
	struct DisplayVars {
		int lineheight = 0;    // font's height of single line
		int linespacing = 0;   // font's line spacing
		int fulltxtheight = 0; // total height of all the text
	} _disp;

	/**@}*/

	/**
	 * @defgroup agsdrawglobals draw globals
	 * @ingroup agsglobals
	 * @{
	 */

	std::vector<RoomCameraDrawData> *_CameraDrawData;
	std::vector<SpriteListEntry> *_sprlist;
	std::vector<SpriteListEntry> *_thingsToDrawList;

	AGS::Engine::IGraphicsDriver *_gfxDriver = nullptr;
	AGS::Engine::IDriverDependantBitmap *_blankImage = nullptr;
	AGS::Engine::IDriverDependantBitmap *_blankSidebarImage = nullptr;
	AGS::Engine::IDriverDependantBitmap *_debugConsole = nullptr;

	// actsps is used for temporary storage of the bitamp image
	// of the latest version of the sprite
	int _actSpsCount = 0;
	AGS::Shared::Bitmap **_actsps = nullptr;
	AGS::Engine::IDriverDependantBitmap **_actspsbmp = nullptr;
	// temporary cache of walk-behind for this actsps image
	AGS::Shared::Bitmap **_actspswb = nullptr;
	AGS::Engine::IDriverDependantBitmap **_actspswbbmp = nullptr;
	CachedActSpsData *_actspswbcache = nullptr;
	bool _current_background_is_dirty = false;
	// Room background sprite
	AGS::Engine::IDriverDependantBitmap *_roomBackgroundBmp = nullptr;
	AGS::Shared::Bitmap **_guibg = nullptr;
	AGS::Engine::IDriverDependantBitmap **_guibgbmp = nullptr;
	AGS::Shared::Bitmap *_debugConsoleBuffer = nullptr;
	// whether there are currently remnants of a DisplaySpeech
	bool _screen_is_dirty = false;
	AGS::Shared::Bitmap *_raw_saved_screen = nullptr;
	AGS::Shared::Bitmap **_dynamicallyCreatedSurfaces = nullptr;
	int _places_r = 3, _places_g = 2, _places_b = 3;
	color *_palette;
	COLOR_MAP *_maincoltable;

	/**@}*/

	/**
	 * @defgroup agsdraw_softwareglobals draw_software globals
	 * @ingroup agsglobals
	 * @{
	 */

	// Dirty rects for the game screen background (black screen);
	// these are used when the room viewport does not cover whole screen,
	// so that we know when to paint black after mouse cursor and gui.
	DirtyRects *_BlackRects;
	Point *_GlobalOffs;
	// Dirty rects object for the single room camera
	std::vector<DirtyRects> *_RoomCamRects;
	// Saved room camera offsets to know if we must invalidate whole surface.
	// TODO: if we support rotation then we also need to compare full transform!
	std::vector<std::pair<int, int> > *_RoomCamPositions;

	/**@}*/

	/**
	 * @defgroup agsengineglobals engine globals
	 * @ingroup agsglobals
	 * @{
	 */

	bool _check_dynamic_sprites_at_exit = true;

	/**@}*/

	/**
	 * @defgroup agsengineglobals engine globals
	 * @ingroup agsglobals
	 * @{
	 */

	ResourcePaths *_ResPaths;
	t_engine_pre_init_callback _engine_pre_init_callback = nullptr;

	/**@}*/

	/**
	 * @defgroup agsengine_setupglobals engine_setup globals
	 * @ingroup agsglobals
	 * @{
	 */

	int _convert_16bit_bgr = 0;

	/**@}*/


	/**
	 * @defgroup agseventglobals event globals
	 * @ingroup agsglobals
	 * @{
	 */

	int _in_enters_screen = 0, _done_es_error = 0;
	int _in_leaves_screen = -1;

	EventHappened *_event;
	int _numevents = 0;

	const char *_evblockbasename = nullptr;
	int _evblocknum = 0;

	int _inside_processevent = 0;
	int _eventClaimed = 0;

	const char *_tsnames[4] = { nullptr, REP_EXEC_NAME, "on_key_press", "on_mouse_click" };

	/**@}*/

	/**
	 * @defgroup agsfileglobals file globals
	 * @ingroup agsglobals
	 * @{
	 */

	// TODO: the asset path configuration should certainly be revamped at some
	// point, with uniform method of configuring auxiliary paths and packages.
	// Installation directory, may contain absolute or relative path
	String _installDirectory;
	// Installation directory, containing audio files
	String _installAudioDirectory;
	// Installation directory, containing voice-over files
	String _installVoiceDirectory;

	/**@}*/

	/**
	 * @defgroup agsfontsglobals fonts globals
	 * @ingroup agsglobals
	 * @{
	 */

	std::vector<AGS::Shared::Font> *_fonts;
	TTFFontRenderer *_ttfRenderer;
	WFNFontRenderer *_wfnRenderer;
	SplitLines *_Lines;

	/**@}*/

	/**
	 * @defgroup agsgameglobals game globals
	 * @ingroup agsglobals
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
	GameSetup *_usetup;
	AGS::Shared::String _saveGameDirectory;
	AGS::Shared::String _saveGameParent;
	AGS::Shared::String _saveGameSuffix;
	bool _want_exit = false;
	bool _abort_engine = false;
	AGSPlatformDriver *_platform = nullptr;

	RoomObject *_objs = nullptr;
	RoomStatus *_croom = nullptr;

	volatile int _switching_away_from_game = 0;
	volatile bool _switched_away = false;
	int _frames_per_second = 40;
	int _displayed_room = -10, _starting_room = -1;
	int _in_new_room = 0, _new_room_was = 0; // 1 in new room, 2 first time in new room, 3 loading saved game
	int _new_room_pos = 0;
	int _new_room_x = SCR_NO_VALUE, _new_room_y = SCR_NO_VALUE;
	int _new_room_loop = SCR_NO_VALUE;
	bool _proper_exit = true;
	int _our_eip = 0;

	int _oldmouse = 0;
	GameDataVersion _loaded_game_file_version = kGameVersion_Undefined;
	int _game_paused = 0;
	char _pexbuf[STD_BUFFER_SIZE] = { 0 };
	unsigned int _load_new_game = 0;
	int _load_new_game_restore = -1;
	// TODO: refactor these global vars into function arguments
	int _getloctype_index = 0, _getloctype_throughgui = 0;
	char _gamefilenamebuf[200] = { 0 };
	int _gameHasBeenRestored = 0;
	int _oldeip = 0;
	int _game_update_suspend = 0;
	bool _new_room_placeonwalkable = false;

	/**@}*/

	/**
	 * @defgroup agsgame_initglobals game_init globals
	 * @ingroup agsglobals
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
	 * @defgroup agsgame_runglobals game_run globals
	 * @ingroup agsglobals
	 * @{
	 */

	// Following 3 parameters instruct the engine to run game loops until
	// certain condition is not fullfilled.
	int _restrict_until = 0;
	int _user_disabled_for = 0;
	const void *_user_disabled_data = nullptr;

	unsigned int _loopcounter = 0;
	unsigned int _lastcounter = 0;
	int _numEventsAtStartOfFunction = 0;
	uint32 _t1 = 0; // timer for FPS
	int _old_key_shifts = 0; // for saving shift modes

	/**@}*/

	/**
	 * @defgroup agsgfxfilter_aad3dglobals gfxfilter_aad3d globals
	 * @ingroup agsglobals
	 * @{
	 */

	const AGS::Engine::GfxFilterInfo *_aad3dFilterInfo;

	/**@}*/

	/**
	 * @defgroup agsgfxfilter_allegroglobals gfxfilter_allegro globals
	 * @ingroup agsglobals
	 * @{
	 */

	const AGS::Engine::GfxFilterInfo *_allegroFilterInfo;

	/**@}*/

	/**
	 * @defgroup agsgfxfilter_allegroglobals gfxfilter_allegro globals
	 * @ingroup agsglobals
	 * @{
	 */

	const AGS::Engine::GfxFilterInfo *_hqxFilterInfo;

	/**@}*/

	/**
	 * @defgroup agsgfxfilter_d3dglobals gfxfilter_d3d globals
	 * @ingroup agsglobals
	 * @{
	 */

	const AGS::Engine::GfxFilterInfo *_d3dFilterInfo;

	/**@}*/

	/**
	 * @defgroup agsglobal_dialogglobals global_dialog globals
	 * @ingroup agsglobals
	 * @{
	 */

	ScriptPosition *_last_in_dialog_request_script_pos;

	/**@}*/

	/**
	 * @defgroup agsglobal_objectglobals global_object globals
	 * @ingroup agsglobals
	 * @{
	 */

	// Used for deciding whether a char or obj was closer
	int _obj_lowest_yp = 0;

	/**@}*/

	/**
	 * @defgroup agsgraphics_modeglobals graphics_mode globals
	 * @ingroup agsglobals
	 * @{
	 */

	AGS::Engine::IGfxDriverFactory *_GfxFactory = nullptr;

	// Last saved fullscreen and windowed configs; they are used when switching
	// between between fullscreen and windowed modes at runtime.
	// If particular mode is modified, e.g. by script command, related config should be overwritten.
	ActiveDisplaySetting *_SavedFullscreenSetting;
	ActiveDisplaySetting *_SavedWindowedSetting;
	// Current frame scaling setup
	GameFrameSetup *_CurFrameSetup;
	// The game-to-screen transformation
	AGS::Shared::PlaneScaling *_GameScaling;

	/**@}*/

	/**
	 * @defgroup agsguiglobals gui globals
	 * @ingroup agsglobals
	 * @{
	 */

	int _ifacepopped = -1; // currently displayed pop-up GUI (-1 if none)
	int _mouse_on_iface = -1; // mouse cursor is over this interface
	int _mouse_ifacebut_xoffs = -1, _mouse_ifacebut_yoffs = -1;
	int _eip_guinum = 0, _eip_guiobj = 0;

	/**@}*/

	/**
	 * @defgroup agsguibuttonglobals guibutton globals
	 * @ingroup agsglobals
	 * @{
	 */

	std::vector<AGS::Shared::GUIButton> *_guibuts;
	int _numguibuts = 0;

	/**@}*/

	/**
	 * @defgroup agsguidialogglobals guidialog globals
	 * @ingroup agsglobals
	 * @{
	 */

	// TODO: store drawing surface inside old gui classes instead
	int _windowPosX = 0, _windowPosY = 0, _windowPosWidth = 0, _windowPosHeight = 0;
	AGS::Shared::Bitmap *_windowBuffer = nullptr;
	AGS::Engine::IDriverDependantBitmap *_dialogDDB = nullptr;

#define MAXSAVEGAMES_20 20
	int _myscrnwid = 320, _myscrnhit = 200;
	char *_lpTemp = nullptr, *_lpTemp2 = nullptr;
	int _numsaves = 0, _toomanygames = 0;
	int _filenumbers[MAXSAVEGAMES_20];
	unsigned long _filedates[MAXSAVEGAMES_20];
	char _bufTemp[260], _buffer2[260];
	char _buff[200];
	CSCIMessage *_smes;

	/**@}*/

	/**
	 * @defgroup agsguiinvglobals guiinv globals
	 * @ingroup agsglobals
	 * @{
	 */

	std::vector<AGS::Shared::GUIInvWindow> *_guiinv;
	int _numguiinv = 0;

	/**@}*/

	/**
	 * @defgroup agsguilabelglobals guilabel globals
	 * @ingroup agsglobals
	 * @{
	 */

	std::vector<AGS::Shared::GUILabel> *_guilabels;
	int _numguilabels = 0;

	/**@}*/

	/**
	 * @defgroup agsguilistboxglobals guilistbox globals
	 * @ingroup agsglobals
	 * @{
	 */

	std::vector<AGS::Shared::GUIListBox> *_guilist;
	int _numguilist = 0;

	/**@}*/

	/**
	 * @defgroup agsguimainglobals guimain globals
	 * @ingroup agsglobals
	 * @{
	 */

	int _guis_need_update = 1;
	int _all_buttons_disabled = 0, _gui_inv_pic = -1;
	int _gui_disabled_style = 0;

	/**@}*/

	/**
	 * @defgroup agsguisliderglobals guislider globals
	 * @ingroup agsglobals
	 * @{
	 */

	std::vector<AGS::Shared::GUISlider> *_guislider;
	int _numguislider = 0;

	/**@}*/

	/**
	 * @defgroup agsguitextboxglobals guitextbox globals
	 * @ingroup agsglobals
	 * @{
	 */

	std::vector<AGS::Shared::GUITextBox> *_guitext;
	int _numguitext = 0;

	/**@}*/

	/**
	 * @defgroup agsinteractionsglobals interactions globals
	 * @ingroup agsglobals
	 * @{
	 */

	AGS::Shared::InteractionVariable *_globalvars;
	int _numGlobalVars = 1;

	/**@}*/

	/**
	 * @defgroup agsinvwindowglobals invwindow globals
	 * @ingroup agsglobals
	 * @{
	 */

	int _in_inv_screen = 0, _inv_screen_newroom = -1;

	/**@}*/

	/**
	 * @defgroup agslzwglobals lzw globals
	 * @ingroup agsglobals
	 * @{
	 */

	char *_lzbuffer = nullptr;
	int *_node = nullptr;
	int _pos = 0;
	long _outbytes = 0, _maxsize = 0, _putbytes = 0;

	/**@}*/

	/**
	 * @defgroup agsmainglobals main globals
	 * @ingroup agsglobals
	 * @{
	 */

	String _appPath;
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
	 * @defgroup agsmanagedobjectpoolglobals managedobjectpool globals
	 * @ingroup agsglobals
	 * @{
	 */

	ManagedObjectPool *_pool;

	/**@}*/

	/**
	 * @defgroup agsmouseglobals mouse globals
	 * @ingroup agsglobals
	 * @{
	 */

	int8 _currentcursor = 0;
	// virtual mouse cursor coordinates
	int _mousex = 0, _mousey = 0, _numcurso = -1, _hotx = 0, _hoty = 0;
	// real mouse coordinates and bounds
	int _real_mouse_x = 0, _real_mouse_y = 0;
	int _boundx1 = 0, _boundx2 = 99999, _boundy1 = 0, _boundy2 = 99999;
	int _disable_mgetgraphpos = 0;
	int8 _ignore_bounds = 0;
	AGS::Shared::Bitmap *_mousecurs[MAXCURSORS];

	ScriptMouse *_scmouse;
	int _cur_mode = 0, _cur_cursor = 0;
	int _mouse_frame = 0, _mouse_delay = 0;
	int _lastmx = -1, _lastmy = -1;
	int8 _alpha_blend_cursor = 0;
	AGS::Shared::Bitmap *_dotted_mouse_cursor = nullptr;
	AGS::Engine::IDriverDependantBitmap *_mouseCursor = nullptr;
	AGS::Shared::Bitmap *_blank_mouse_cursor = nullptr;

	/**@}*/

	/**
	 * @defgroup agsmousew32globals mousew32 globals
	 * @ingroup agsglobals
	 * @{
	 */

	int _butwas = 0;
	int _hotxwas = 0, _hotywas = 0;
	Mouse *_mouse;

	/**@}*/

	/**
	 * @defgroup agsoverlayglobals overlay globals
	 * @ingroup agsglobals
	 * @{
	 */

	ScreenOverlay *_screenover;
	int _is_complete_overlay = 0;
	int _numscreenover = 0;

	/**@}*/

	/**
	 * @defgroup agsquitglobals quit globals
	 * @ingroup agsglobals
	 * @{
	 */

	PluginObjectReader *_pluginReaders;
	int _numPluginReaders = 0;

	/**@}*/

	/**
	 * @defgroup agsquitglobals quit globals
	 * @ingroup agsglobals
	 * @{
	 */

	bool _handledErrorInEditor = false;
	char _return_to_roomedit[30] = { '\0' };
	char _return_to_room[150] = { '\0' };
	char _quit_message[256] = { '\0' };

	/**@}*/

	/**
	 * @defgroup agsroomglobals room globals
	 * @ingroup agsglobals
	 * @{
	 */

	RGB_MAP *_rgb_table;  // for 256-col antialiasing
	int _new_room_flags = 0;
	int _gs_to_newroom = -1;
	int _bg_just_changed = 0;

	/**@}*/

	/**
	 * @defgroup agsroute_finder_implglobals route_finder_impl globals
	 * @ingroup agsglobals
	 * @{
	 */

	int32_t *_navpoints;
	Navigation *_nav;
	int _num_navpoints = 0;
	fixed _move_speed_x = 0, _move_speed_y = 0;
	AGS::Shared::Bitmap *_wallscreen = nullptr;
	int _lastcx = 0, _lastcy = 0;

	/**@}*/

	/**
	 * @defgroup agsscreenglobals screen globals
	 * @ingroup agsglobals
	 * @{
	 */

	// Screenshot made in the last room, used during some of the transition effects
	AGS::Shared::Bitmap *_saved_viewport_bitmap = nullptr;
	color *_old_palette;

	/**@}*/

	/**
	 * @defgroup agsscriptglobal script globals
	 * @ingroup agsglobals
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

	/**
	 * @defgroup agsscript_runtimeglobals script_runtime globals
	 * @ingroup agsglobals
	 * @{
	 */

	new_line_hook_type _new_line_hook = nullptr;
	int _maxWhileLoops = 0;
	ccInstance *_loadedInstances[MAX_LOADED_INSTANCES];

	/**@}*/

	/**
	 * @defgroup agsstringglobals string globals
	 * @ingroup agsglobals
	 * @{
	 */

	int _MAXSTRLEN = MAX_MAXSTRLEN;

	/**@}*/

	/**
	 * @defgroup agssystemimportsglobals systemimports globals
	 * @ingroup agsglobals
	 * @{
	 */

	SystemImports *_simp;
	SystemImports *_simp_for_plugin;

	/**@}*/

	/**
	 * @defgroup agssys_eventsglobals sys_events globals
	 * @ingroup agsglobals
	 * @{
	 */

	Plugins::Core::EngineExports *_engineExports;
	Common::Array<EnginePlugin> *_plugins;
	int _pluginsWantingDebugHooks = 0;
	long _pl_file_handle = -1;
	AGS::Shared::Stream *_pl_file_stream = nullptr;

	int _pluginSimulatedClick = -1;
	int _mouse_z_was = 0;

	/**@}*/

	/**
	 * @defgroup agstimerglobals timer globals
	 * @ingroup agsglobals
	 * @{
	 */

	std::chrono::microseconds _tick_duration = std::chrono::microseconds(1000000LL / 40);
	bool _framerate_maxed = false;

	uint32 _last_tick_time = 0; // AGS_Clock::now();
	uint32 _next_frame_timestamp = 0; // AGS_Clock::now();

	/**@}*/

	/**
	 * @defgroup agstranslationglobals translation globals
	 * @ingroup agsglobals
	 * @{
	 */

	AGS::Shared::Translation *_trans;
	StringMap *_transtree = nullptr;
	String _trans_name, _trans_filename;
	long _lang_offs_start = 0;
	char _transFileName[MAX_PATH_SZ] = { 0 };
	std::vector<uint16> _wcsbuf; // widechar buffer
	std::vector<char> _mbbuf;  // utf8 buffer

	/**@}*/

	/**
	 * @defgroup agswalkableareaglobals walkablearea globals
	 * @ingroup agsglobals
	 * @{
	 */

	AGS::Shared::Bitmap *_walkareabackup = nullptr, *_walkable_areas_temp = nullptr;

	/**@}*/

	/**
	 * @defgroup agswalkbehindglobals walkbehind globals
	 * @ingroup agsglobals
	 * @{
	 */

	char *_walkBehindExists = nullptr;  // whether a WB area is in this column
	int *_walkBehindStartY = nullptr, *_walkBehindEndY = nullptr;
	int8 _noWalkBehindsAtAll = 0;
	int _walkBehindLeft[MAX_WALK_BEHINDS], _walkBehindTop[MAX_WALK_BEHINDS];
	int _walkBehindRight[MAX_WALK_BEHINDS], _walkBehindBottom[MAX_WALK_BEHINDS];
	AGS::Engine::IDriverDependantBitmap *_walkBehindBitmap[MAX_WALK_BEHINDS];
	int _walkBehindsCachedForBgNum = 0;
	WalkBehindMethodEnum _walkBehindMethod = DrawOverCharSprite;
	int _walk_behind_baselines_changed = 0;

	/**@}*/

	/**
	 * @defgroup agswordsdictionaryglobals wordsdictionary globals
	 * @ingroup agsglobals
	 * @{
	 */

	const char *_passwencstring = "Avis Durgan";

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
