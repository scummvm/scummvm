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

#ifndef AGS_GLOBALS_H
#define AGS_GLOBALS_H

#include "ags/shared/core/platform.h"
#define AGS_PLATFORM_DEFINES_PSP_VARS (AGS_PLATFORM_OS_IOS || AGS_PLATFORM_OS_ANDROID)

#include "common/std/queue.h"
#include "ags/shared/ac/game_version.h"
#include "ags/shared/ac/keycode.h"
#include "ags/shared/util/stdio_compat.h"
#include "ags/shared/util/string.h"
#include "ags/shared/util/string_types.h"
#include "ags/shared/util/version.h"
#include "ags/shared/font/wfn_font.h"
#include "ags/shared/gui/gui_main.h"
#include "ags/shared/script/cc_script.h"
#include "ags/engine/ac/dynobj/script_game.h"
#include "ags/engine/ac/dynobj/script_user_object.h"
#include "ags/engine/ac/event.h"
#include "ags/engine/ac/file.h"
#include "ags/engine/ac/runtime_defines.h"
#include "ags/engine/ac/walk_behind.h"
#include "ags/engine/main/engine.h"
#include "ags/engine/main/graphics_mode.h"
#include "ags/engine/media/audio/audio_defines.h"
#include "ags/engine/script/script.h"
#include "ags/engine/script/script_runtime.h"
#include "common/std/array.h"
#include "common/std/chrono.h"
#include "common/std/memory.h"
#include "common/std/set.h"
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
class SpriteCache;
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

class IRouteFinder;
class Navigation;
class SplitLines;
class TTFFontRenderer;
class WFNFontRenderer;

struct AGSCCStaticObject;
struct AGSDeSerializer;
struct AGSPlatformDriver;
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
struct ScriptError;
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
struct CCStaticArray;
struct StaticGame;
struct SystemImports;
struct TopBarSettings;
struct ViewStruct;

class Globals {
public:
	enum SimdFlags : uint {
		SIMD_NONE = 0,
		SIMD_NEON = (1 << 0),
		SIMD_SSE2 = (1 << 1),
		SIMD_AVX2 = (1 << 2),
	};

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
	uint _simd_flags = SIMD_NONE;
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

	// Accumulated absolute and relative mouse device motion.
	// May be retrieved by calling *acquire_absxy and *acquire_relxy functions,
	// after which these are reset, until next motion event is received.
	volatile int _sys_mouse_x = 0; // mouse x position
	volatile int _sys_mouse_y = 0; // mouse y position
	volatile int _sys_mouse_z = 0; // mouse wheel position

	volatile int _freeze_mouse_flag = 0;

	// Relative x and y deltas
	int _mouse_accum_relx = 0, _mouse_accum_rely = 0;

	int _mouse_button_state = 0;
	int _mouse_accum_button_state = 0;
	uint32 _mouse_clear_at_time = 0;
	eAGSMouseButton _wasbutdown = kMouseNone;
	int _wasongui = 0;

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
	std::vector<PluginObjectReader> *_pluginReaders;

	/**@}*/

	/**
	 * @defgroup agsstaticobjectglobals agsstaticobject globals
	 * @ingroup agsglobals
	 * @{
	 */

	AGSCCStaticObject *_GlobalStaticManager;
	CCScriptGame	  *_GameStaticManager;

	/**@}*/

	/**
	 * @defgroup agsaudioglobals audio globals
	 * @ingroup agsglobals
	 * @{
	 */

	std::array<SOUNDCLIP *> *_audioChannels;
	std::array<AmbientSound> *_ambient;

	ScriptAudioChannel *_scrAudioChannel;
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

	std::vector<AnimatingGUIButton> *_animbuts;

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
	 * @defgroup agscc_commonglobals cc_common globals
	 * @ingroup agsglobals
	 * @{
	 */
	ScriptError *_ccError;

	/**@}*/

	/**
	 * @defgroup agscc_instanceglobals cc_instance globals
	 * @ingroup agsglobals
	 * @{
	 */

	// Instance thread stack holds a list of running or suspended script instances;
	// In AGS currently only one thread is running, others are waiting in the queue.
	// An example situation is repeatedly_execute_always callback running while
	// another instance is waiting at the blocking action or Wait().
	std::deque<ccInstance *> *_InstThreads;
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
	std::vector<SpeechLipSyncLine> *_splipsync;
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

	int _debug_flags = 0;

	int _display_fps;
	std::unique_ptr<AGS::Engine::MessageBuffer> *_DebugMsgBuff;
	std::unique_ptr<AGS::Engine::LogFile> *_DebugLogFile;

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

	std::vector<DialogTopic> _dialog;
	ScriptDialogOptionsRendering *_ccDialogOptionsRendering;
	ScriptDrawingSurface *_dialogOptionsRenderingSurface = nullptr;

	// identifier (username) of the voice pak
	String _VoicePakName;
	// parent part to use when making voice asset names
	String _VoiceAssetPath;
	int _said_speech_line = 0; // used while in dialog to track whether screen needs updating
	int _said_text = 0;
	int _longestline = 0;
	// Old dialog support
	std::vector<std::vector<uint8_t>> _old_dialog_scripts;
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
	// Two lists of sprites to push into renderer during next render pass
	// thingsToDrawList - is the main list, unsorted, drawn in the index order
	std::vector<SpriteListEntry> *_thingsToDrawList;
	std::vector<SpriteListEntry> *_sprlist;

	AGS::Engine::IGraphicsDriver *_gfxDriver = nullptr;
	AGS::Engine::IDriverDependantBitmap *_blankImage = nullptr;
	AGS::Engine::IDriverDependantBitmap *_blankSidebarImage = nullptr;

	// actsps is used for temporary storage of the bitamp and texture
	// of the latest version of the sprite (room objects and characters);
	// objects sprites begin with index 0, characters are after ACTSP_OBJSOFF
	std::vector<ObjTexture> *_actsps;
	// Walk-behind textures (3D renderers only)
	std::vector<ObjTexture> *_walkbehindobj;
	// GUI surfaces
	std::vector<ObjTexture> *_guibg;
	// GUI control surfaces
	std::vector<ObjTexture> *_guiobjbg;
	// first control texture index of each GUI
	std::vector<int> *_guiobjddbref;
	// Overlays textures
	std::vector<ObjTexture> *_overtxs;
	// For debugging room masks
	ObjTexture *_debugRoomMaskObj;
	ObjTexture *_debugMoveListObj;
	RoomAreaMask _debugRoomMask = kRoomAreaNone;
	int _debugMoveListChar = -1;
	// For in-game "console" surface
	AGS::Shared::Bitmap *_debugConsoleBuffer;
	// Whether room bg was modified
	bool _current_background_is_dirty = false;
	// Room background sprite
	AGS::Engine::IDriverDependantBitmap *_roomBackgroundBmp = nullptr;
	// whether there are currently remnants of a DisplaySpeech
	bool _screen_is_dirty = false;
	std::unique_ptr<Shared::Bitmap> _raw_saved_screen;
	std::unique_ptr<Shared::Bitmap> *_dynamicallyCreatedSurfaces;
	color *_palette;
	COLOR_MAP *_maincoltable;

	std::vector<Engine::IDriverDependantBitmap *> *_guiobjddb;
	std::vector<Point> *_guiobjoff; // because surface may be larger than logical position

	DrawState _drawstate;
	DrawFPS _gl_DrawFPS;

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

	std::vector<EventHappened> *_events;

	int _inside_processevent = 0;
	int _eventClaimed = 0;

	const char *_tsnames[kTS_Num] = { nullptr, REP_EXEC_NAME, "on_key_press", "on_mouse_click", "on_text_input" };

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

	ScriptFileHandle _valid_handles[MAX_OPEN_SCRIPT_FILES + 1];
	// [IKM] NOTE: this is not precisely the number of files opened at this moment,
	// but rather maximal number of handles that were used simultaneously during game run
	int _num_open_script_files = 0;

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
	const WFNChar _emptyChar; // a dummy character to substitute bad symbols
	Shared::Bitmap _wputblock_wrapper; // [IKM] argh! :[

	/**@}*/

	/**
	 * @defgroup agsgameglobals game globals
	 * @ingroup agsglobals
	 * @{
	 */

	GameSetupStruct *_game;
	GameState *_play;
	AGS::Shared::SpriteCache *_spriteset;
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
	ScriptObject *_scrObj;
	std::vector<ScriptGUI> *_scrGui;
	ScriptHotspot *_scrHotspot;
	ScriptRegion *_scrRegion;
	ScriptInvItem *_scrInv;
	std::vector<ScriptDialog> *_scrDialog;

	std::vector<ViewStruct> *_views;

	// Draw cache: keep record of all kinds of things related to the previous drawing state
	// Cached character and object states, used to determine
	// whether these require texture update
	std::vector<ObjectCache> *_charcache;
	ObjectCache *_objcache;
	std::vector<Point> *_overcache;
	std::vector<CharacterExtras> *_charextra;
	// MoveLists for characters and room objects; NOTE: 1-based array!
	// object sprites begin with index 1, characters are after MAX_ROOM_OBJECTS + 1
	std::vector<MoveList> *_mls;

	GameSetup *_usetup;
	AGS::Shared::String _saveGameDirectory;
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
	// Data format version of the loaded game
	GameDataVersion _loaded_game_file_version = kGameVersion_Undefined;
	// The version of the engine the loaded game was compiled for (if available)
	Version _game_compiled_version;
	int _game_paused = 0;
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

	CCStaticArray *_StaticCharacterArray;
	CCStaticArray *_StaticObjectArray;
	CCStaticArray *_StaticGUIArray;
	CCStaticArray *_StaticHotspotArray;
	CCStaticArray *_StaticRegionArray;
	CCStaticArray *_StaticInventoryArray;
	CCStaticArray *_StaticDialogArray;

	/**@}*/

	/**
	 * @defgroup agsgame_runglobals game_run globals
	 * @ingroup agsglobals
	 * @{
	 */

	 // Following struct instructs the engine to run game loops until
	 // certain condition is not fulfilled.
	struct RestrictUntil {
		int type = 0; // type of condition, UNTIL_* constant
		int disabled_for = 0; // FOR_* constant
		// pointer to the test variable
		const void *data_ptr = nullptr;
		// other values used for a test, depend on type
		int data1 = 0;
		int data2 = 0;
	} _restrict_until;

	float _fps;
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
	AGS::Engine::GfxFilterInfo *_scummvmGfxFilter;

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
	 * @defgroup agsglobal_gameglobals global_game globals
	 * @ingroup agsglobals
	 * @{
	 */

	ScriptPosition *_last_cutscene_script_pos;

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
	FrameScaleDef _CurFrameSetup = kFrame_Undefined;
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

	/**@}*/

	/**
	 * @defgroup agsguilabelglobals guilabel globals
	 * @ingroup agsglobals
	 * @{
	 */

	std::vector<AGS::Shared::GUILabel> *_guilabels;

	/**@}*/

	/**
	 * @defgroup agsguilistboxglobals guilistbox globals
	 * @ingroup agsglobals
	 * @{
	 */

	std::vector<AGS::Shared::GUIListBox> *_guilist;

	/**@}*/

	/**
	 * @defgroup agsguimainglobals guimain globals
	 * @ingroup agsglobals
	 * @{
	 */

	int _guis_need_update = 1;
	AGS::Shared::GuiDisableStyle _all_buttons_disabled = AGS::Shared::kGuiDis_Undefined;
	int _gui_inv_pic = -1;

	/**@}*/

	/**
	 * @defgroup agsguisliderglobals guislider globals
	 * @ingroup agsglobals
	 * @{
	 */

	std::vector<AGS::Shared::GUISlider> *_guislider;

	/**@}*/

	/**
	 * @defgroup agsguitextboxglobals guitextbox globals
	 * @ingroup agsglobals
	 * @{
	 */

	std::vector<AGS::Shared::GUITextBox> *_guitext;

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

	uint8_t *_lzbuffer = nullptr;
	int *_node = nullptr;
	int _pos = 0;
	size_t _outbytes = 0;

	/**@}*/

	/**
	 * @defgroup agsmainglobals main globals
	 * @ingroup agsglobals
	 * @{
	 */

	String _appPath;
	String _appDirectory; // Needed for library loading
	String _cmdGameDataPath;

	// Startup flags, set from parameters to engine
	int _override_start_room = 0;
	bool _justDisplayHelp = false;
	bool _justDisplayVersion = false;
	bool _justRunSetup = false;
	bool _justTellInfo = false;
	std::set<String> _tellInfoKeys;
	int _loadSaveGameOnStartup = -1;

	// ScummVM GUIO-controlled flags
	bool _saveThumbnail = true;     // capture a screenshot when saving (used for saves thumbnails)
	bool _noScummAutosave = false;  // disable ScummVM autosaves
	bool _noScummSaveLoad = false;  // disable ScummVM GMM save/load

#if 0
	//! AGS_PLATFORM_DEFINES_PSP_VARS
	int _psp_rotation = 0;
	int _psp_gfx_renderer = 0;
	int _psp_gfx_scaling = 1;
	int _psp_gfx_smoothing = 0;
	int _psp_gfx_super_sampling = 1;
	int _psp_gfx_smooth_sprites = 0;
#endif

	// Current engine version
	Version _EngineVersion;

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
	// real mouse coordinates and bounds (in window coords)
	int _real_mouse_x = 0, _real_mouse_y = 0;
	int _boundx1 = 0, _boundx2 = 99999, _boundy1 = 0, _boundy2 = 99999;
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

	std::vector<ScreenOverlay> *_screenover;
	std::queue<int32_t> *_over_free_ids;
	int _is_complete_overlay = 0;

	/**@}*/

	/**
	 * @defgroup agsquitglobals quit globals
	 * @ingroup agsglobals
	 * @{
	 */

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
	std::unique_ptr<RoomStatus> _room_statuses[MAX_ROOMS];

	/**@}*/

	/**
	 * @defgroup agsroute_finder_implglobals route_finder_impl globals
	 * @ingroup agsglobals
	 * @{
	 */

	Point *_navpoints;
	Navigation *_nav;
	int _num_navpoints = 0;
	AGS::Shared::Bitmap *_wallscreen = nullptr;
	fixed _move_speed_x, _move_speed_y;
	int _lastcx = 0, _lastcy = 0;
	std::unique_ptr<IRouteFinder> *_route_finder_impl;

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
	UInstance _gameinst;
	UInstance _roominst;
	UInstance _dialogScriptsInst;
	UInstance _gameinstFork;
	UInstance _roominstFork;

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
	NonBlockingScriptFunction *_runDialogOptionTextInputHandlerFunc;
	NonBlockingScriptFunction *_runDialogOptionRepExecFunc;
	NonBlockingScriptFunction *_runDialogOptionCloseFunc;

	ScriptSystem *_scsystem;

	std::vector<PScript> *_scriptModules;
	std::vector<UInstance> *_moduleInst;
	std::vector<UInstance> *_moduleInstFork;
	std::vector<RuntimeScriptValue> *_moduleRepExecAddr;
	size_t _numScriptModules = 0;

	/**@}*/

	/**
	 * @defgroup agsscript_runtimeglobals agsscript_user_object agscript_string script_runtime globals
	 * @ingroup agsglobals
	 * @{
	 */

	new_line_hook_type _new_line_hook = nullptr;
	// Minimal timeout: how much time may pass without any engine update
	// before we want to check on the situation and do system poll
	unsigned _timeoutCheckMs = 60u;
	// Critical timeout: how much time may pass without any engine update
	// before we abort or post a warning
	unsigned _timeoutAbortMs = 0u;
	// Maximal while loops without any engine update in between,
	// after which the interpreter will abort
	unsigned _maxWhileLoops = 0u;
	ccInstance *_loadedInstances[MAX_LOADED_INSTANCES];
	ScriptString *_myScriptStringImpl;
	ScriptUserObject _globalDynamicStruct;

	/**@}*/

	/**
	 * @defgroup agsstringglobals string globals
	 * @ingroup agsglobals
	 * @{
	 */

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

	eAGSMouseButton _simulatedClick = kMouseNone;
	int _mouse_z_was = 0;

	/**@}*/

	/**
	 * @defgroup agstimerglobals timer globals
	 * @ingroup agsglobals
	 * @{
	 */

	std::chrono::microseconds _tick_duration = std::chrono::microseconds(1000000LL / 40);
	bool _framerate_maxed = false;
	int _framerate = 0;

	uint32 _last_tick_time = 0; // AGS_Clock::now();
	uint32 _next_frame_timestamp = 0; // AGS_Clock::now();

	/**@}*/

	/**
	 * @defgroup agstranslationglobals translation globals
	 * @ingroup agsglobals
	 * @{
	 */

	AGS::Shared::Translation *_trans;
	AGS::Shared::StringMap *_transtree = nullptr;
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
	int _walkBehindsCachedForBgNum = -1;
	int _walk_behind_baselines_changed = 0;
	Rect _walkBehindAABB[MAX_WALK_BEHINDS]; // WB bounding box
	std::vector<WalkBehindColumn> _walkBehindCols; // precalculated WB positions

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
