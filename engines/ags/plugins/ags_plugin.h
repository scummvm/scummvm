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

//=============================================================================
//
// AGS Plugin interface header file.
//
// #define THIS_IS_THE_PLUGIN beforehand if including from the plugin.
//
//=============================================================================

#ifndef AGS_PLUGINS_AGS_PLUGIN_H
#define AGS_PLUGINS_AGS_PLUGIN_H

#include "common/array.h"
#include "ags/shared/core/types.h"
#include "ags/shared/font/ags_font_renderer.h"
#include "ags/shared/util/string.h"
#include "ags/plugins/plugin_base.h"
#include "ags/plugins/ags_plugin_evts.h"
#include "ags/engine/util/library_scummvm.h"

namespace AGS3 {

// If the plugin isn't using DDraw, don't require the headers
#ifndef DIRECTDRAW_VERSION
typedef void *LPDIRECTDRAW2;
typedef void *LPDIRECTDRAWSURFACE2;
#endif

#ifndef DIRECTSOUND_VERSION
typedef void *LPDIRECTSOUND;
#endif

#ifndef DIRECTINPUT_VERSION
typedef void *LPDIRECTINPUTDEVICE;
#endif

class BITMAP;

// If not using windows.h, define HWND
#if !defined(_WINDOWS_)
typedef int HWND;
#endif

#define MAXPLUGINS 20

#define AGSIFUNC(type) virtual type

#define MASK_WALKABLE   1
#define MASK_WALKBEHIND 2
#define MASK_HOTSPOT    3
// MASK_REGIONS is interface version 11 and above only
#define MASK_REGIONS    4

#define PLUGIN_FILENAME_MAX (49)

// **** WARNING: DO NOT ALTER THESE CLASSES IN ANY WAY!
// **** CHANGING THE ORDER OF THE FUNCTIONS OR ADDING ANY VARIABLES
// **** WILL CRASH THE SYSTEM.

struct AGSColor {
	unsigned char r, g, b;
	unsigned char padding;
};

struct AGSGameOptions {
	int32 score;      // player's current score
	int32 usedmode;   // set by ProcessClick to last cursor mode used
	int32 disabled_user_interface;  // >0 while in cutscene/etc
	int32 gscript_timer;    // obsolete
	int32 debug_mode;       // whether we're in debug mode
	int32 globalvars[50];   // obsolete
	int32 messagetime;      // time left for auto-remove messages
	int32 usedinv;          // inventory item last used
	int32 inv_top, inv_numdisp, inv_numorder, inv_numinline;
	int32 text_speed;       // how quickly text is removed
	int32 sierra_inv_color; // background used to paint defualt inv window
	int32 talkanim_speed;   // animation speed of talking anims
	int32 inv_item_wid, inv_item_hit;  // set by SetInvDimensions
	int32 speech_text_shadow;         // colour of outline fonts (default black)
	int32 swap_portrait_side;         // sierra-style speech swap sides
	int32 speech_textwindow_gui;      // textwindow used for sierra-style speech
	int32 follow_change_room_timer;   // delay before moving following characters into new room
	int32 totalscore;           // maximum possible score
	int32 skip_display;         // how the user can skip normal Display windows
	int32 no_multiloop_repeat;  // for backwards compatibility
	int32 roomscript_finished;  // on_call finished in room
	int32 used_inv_on;          // inv item they clicked on
	int32 no_textbg_when_voice; // no textwindow bgrnd when voice speech is used
	int32 max_dialogoption_width; // max width of dialog options text window
	int32 no_hicolor_fadein;      // fade out but instant in for hi-color
	int32 bgspeech_game_speed;    // is background speech relative to game speed
	int32 bgspeech_stay_on_display; // whether to remove bg speech when DisplaySpeech is used
	int32 unfactor_speech_from_textlength; // remove "&10" when calculating time for text to stay
	int32 mp3_loop_before_end;    // loop this time before end of track (ms)
	int32 speech_music_drop;      // how much to drop music volume by when speech is played
	int32 in_cutscene;            // we are between a StartCutscene and EndCutscene
	int32 fast_forward;           // player has elected to skip cutscene
	int32 room_width;             // width of current room
	int32 room_height;            // height of current room
};

// AGSCharacter.flags
#define CHF_NOSCALING       1
#define CHF_FIXVIEW         2     // between SetCharView and ReleaseCharView
#define CHF_NOINTERACT      4
#define CHF_NODIAGONAL      8
#define CHF_ALWAYSIDLE      0x10
#define CHF_NOLIGHTING      0x20
#define CHF_NOTURNING       0x40
#define CHF_NOWALKBEHINDS   0x80

struct AGSCharacter {
	int32 defview = 0;
	int32 talkview = 0;
	int32 view = 0;
	int32 room = 0, prevroom = 0;
	int32 x = 0, y = 0, wait = 0;
	int32 flags = 0;
	short following = 0;
	short followinfo = 0;
	int32 idleview = 0;           // the loop will be randomly picked
	short idletime = 0, idleleft = 0; // num seconds idle before playing anim
	short transparency = 0;       // if character is transparent
	short baseline = 0;
	int32 activeinv = 0;
	int32 talkcolor = 0;
	int32 thinkview = 0;
	int32 reserved[2];
	short walkspeed_y = 0, pic_yoffs = 0;
	int32 z = 0;
	int32 reserved2[5];
	short loop = 0, frame = 0;
	short walking = 0, animating = 0;
	short walkspeed = 0, animspeed = 0;
	short inv[301];
	short actx = 0, acty = 0;
	char  name[40];
	char  scrname[20];
	int8  on = 0;
};

// AGSObject.flags
#define OBJF_NOINTERACT 0x01     // not clickable
#define OBJF_NOWALKBEHINDS 0x02  // ignore walk-behinds

struct AGSObject {
	int32 x = 0, y = 0;
	int32 transparent = 0;    // current transparency setting
	int32 reserved[4];
	short num = 0;            // sprite slot number
	short baseline = 0;       // <=0 to use Y co-ordinate; >0 for specific baseline
	short view = 0, loop = 0, frame = 0; // only used to track animation - 'num' holds the current sprite
	short wait = 0, moving = 0;
	int8  cycling = 0;        // is it currently animating?
	int8  overall_speed = 0;
	int8  on = 0;
	int8  flags = 0;
};

// AGSViewFrame.flags
#define FRAF_MIRRORED  1  // flipped left to right

struct AGSViewFrame {
	int32 pic = 0;            // sprite slot number
	short xoffs = 0, yoffs = 0;
	short speed = 0;
	int32 flags = 0;
	int32 sound = 0;          // play sound when this frame comes round
	int32 reserved_for_future[2];
};

// AGSMouseCursor.flags
#define MCF_ANIMATEMOVE 1
#define MCF_DISABLED    2
#define MCF_STANDARD    4
#define MCF_ONLYANIMOVERHOTSPOT 8

struct AGSMouseCursor {
	int32 pic = 0;            // sprite slot number
	short hotx = 0, hoty = 0; // x,y hotspot co-ordinates
	short view = 0;           // view (for animating cursors) or -1
	char  name[10];           // name of cursor mode
	int8  flags = 0;          // MCF_flags above
};

// The editor-to-plugin interface
class IAGSEditor {
public:
	int32 version = 0;
	int32 pluginId = 0;   // used internally, do not touch this

public:
	virtual ~IAGSEditor() {}

	// get the HWND of the main editor frame
	AGSIFUNC(HWND) GetEditorHandle();
	// get the HWND of the current active window
	AGSIFUNC(HWND) GetWindowHandle();
	// add some script to the default header
	AGSIFUNC(void) RegisterScriptHeader(const char *header);
	// de-register a script header (pass same pointer as when added)
	AGSIFUNC(void) UnregisterScriptHeader(const char *header);
};

// GetFontType font types
#define FNT_INVALID 0
#define FNT_SCI     1
#define FNT_TTF     2

// PlaySoundChannel sound types
#define PSND_WAVE       1
#define PSND_MP3STREAM  2
#define PSND_MP3STATIC  3
#define PSND_OGGSTREAM  4
#define PSND_OGGSTATIC  5
#define PSND_MIDI       6
#define PSND_MOD        7

class IAGSScriptManagedObject {
public:
	// when a ref count reaches 0, this is called with the address
	// of the object. Return 1 to remove the object from memory, 0 to
	// leave it
	virtual int Dispose(void *address, bool force) = 0;
	// return the type name of the object
	virtual const char *GetType() = 0;
	// serialize the object into BUFFER (which is BUFSIZE bytes)
	// return number of bytes used
	virtual int Serialize(void *address, char *buffer, int bufsize) = 0;
protected:
	IAGSScriptManagedObject() {
	}
	virtual ~IAGSScriptManagedObject() {
	}
};

class IAGSManagedObjectReader {
public:
	virtual void Unserialize(int key, const char *serializedData, int dataSize) = 0;
protected:
	IAGSManagedObjectReader() {
	}
	virtual ~IAGSManagedObjectReader() {
	}
};

struct AGSRenderMatrixes {
	float WorldMatrix[16];
	float ViewMatrix[16];
	float ProjMatrix[16];
};

// Render stage description
struct AGSRenderStageDesc {
	// Which version of the plugin interface the struct corresponds to;
	// this field must be filled by a plugin before passing the struct into the engine!
	int Version = 0;
	// Stage's matrixes, for 3D rendering: Projection, World and View
	AGSRenderMatrixes Matrixes;
};

// Game info
struct AGSGameInfo {
	// Which version of the plugin interface the struct corresponds to;
	// this field must be filled by a plugin before passing the struct into the engine!
	int Version;
	// Game title (human-readable text)
	char GameName[50];
	// Game's GUID
	char Guid[40];
	// Random key identifying the game (deprecated)
	int UniqueId;
};

// The plugin-to-engine interface
class IAGSEngine {
public:
	int32 version = 0;
	int32 pluginId = 0;   // used internally, do not touch

public:
	virtual ~IAGSEngine() {}

	// quit the game
	AGSIFUNC(void) AbortGame(const char *reason);
	// get engine version
	AGSIFUNC(const char *) GetEngineVersion();
	// register a script function with the system
	AGSIFUNC(void) RegisterScriptFunction(const char *name,
		Plugins::ScriptContainer *instance);
	AGSIFUNC(void) RegisterBuiltInFunction(const char *name,
		Plugins::ScriptContainer *instance);
#ifdef WINDOWS_VERSION
	// get game window handle
	AGSIFUNC(HWND) GetWindowHandle();
	// get reference to main DirectDraw interface
	AGSIFUNC(LPDIRECTDRAW2) GetDirectDraw2();
	// get the DDraw surface associated with a bitmap
	AGSIFUNC(LPDIRECTDRAWSURFACE2) GetBitmapSurface(BITMAP *);
	#endif
	// get a reference to the screen bitmap
	AGSIFUNC(BITMAP *) GetScreen();

	// *** BELOW ARE INTERFACE VERSION 2 AND ABOVE ONLY
	// ask the engine to call back when a certain event happens
	AGSIFUNC(void) RequestEventHook(int32 event);
	// get the options data saved in the editor
	AGSIFUNC(int)  GetSavedData(char *buffer, int32 bufsize);

	// *** BELOW ARE INTERFACE VERSION 3 AND ABOVE ONLY
	// get the virtual screen
	AGSIFUNC(BITMAP *) GetVirtualScreen();
	// write text to the screen in the specified font and colour
	AGSIFUNC(void) DrawText(int32 x, int32 y, int32 font, int32 color, const char *text);
	// get screen dimensions
	AGSIFUNC(void) GetScreenDimensions(int32 *width, int32 *height, int32 *coldepth);
	// get screen surface pitch
	AGSIFUNC(int) GetBitmapPitch(BITMAP *);
	// get screen surface to draw on
	AGSIFUNC(uint8 *) GetRawBitmapSurface(BITMAP *);
	// release the surface
	AGSIFUNC(void) ReleaseBitmapSurface(BITMAP *);
	// get the current mouse co-ordinates
	AGSIFUNC(void) GetMousePosition(int32 *x, int32 *y);

	// *** BELOW ARE INTERFACE VERSION 4 AND ABOVE ONLY
	// get the current room number
	AGSIFUNC(int)  GetCurrentRoom();
	// get the number of background scenes in this room
	AGSIFUNC(int)  GetNumBackgrounds();
	// get the current background frame
	AGSIFUNC(int)  GetCurrentBackground();
	// get a background scene bitmap
	AGSIFUNC(BITMAP *) GetBackgroundScene(int32);
	// get dimensions of a bitmap
	AGSIFUNC(void) GetBitmapDimensions(BITMAP *bmp, int32 *width, int32 *height, int32 *coldepth);

	// *** BELOW ARE INTERFACE VERSION 5 AND ABOVE ONLY
	// similar to fwrite - buffer, size, filehandle
	AGSIFUNC(int)  FWrite(void *, int32, int32);
	// similar to fread - buffer, size, filehandle
	AGSIFUNC(int)  FRead(void *, int32, int32);
	// similar to fseek
	AGSIFUNC(bool)FSeek(soff_t offset, int origin, int32 handle);
	// print text, wrapping as usual
	AGSIFUNC(void) DrawTextWrapped(int32 x, int32 y, int32 width, int32 font, int32 color, const char *text);
	// set the current active 'screen'
	AGSIFUNC(void) SetVirtualScreen(BITMAP *);
	// look up a word in the parser dictionary
	AGSIFUNC(int)  LookupParserWord(const char *word);
	// draw a bitmap to the active screen
	AGSIFUNC(void) BlitBitmap(int32 x, int32 y, BITMAP *, int32 masked);
	// update the mouse and music
	AGSIFUNC(void) PollSystem();

	// *** BELOW ARE INTERFACE VERSION 6 AND ABOVE ONLY
	// get number of characters in game
	AGSIFUNC(int)  GetNumCharacters();
	// get reference to specified character struct
	AGSIFUNC(AGSCharacter *) GetCharacter(int32);
	// get reference to game struct
	AGSIFUNC(AGSGameOptions *) GetGameOptions();
	// get reference to current palette
	AGSIFUNC(AGSColor *) GetPalette();
	// update palette
	AGSIFUNC(void) SetPalette(int32 start, int32 finish, AGSColor *);

	// *** BELOW ARE INTERFACE VERSION 7 AND ABOVE ONLY
	// get the current player character
	AGSIFUNC(int)  GetPlayerCharacter();
	// adjust to main viewport co-ordinates
	AGSIFUNC(void) RoomToViewport(int32 *x, int32 *y);
	// adjust from main viewport co-ordinates (ignores viewport bounds)
	AGSIFUNC(void) ViewportToRoom(int32 *x, int32 *y);
	// number of objects in current room
	AGSIFUNC(int)  GetNumObjects();
	// get reference to specified object
	AGSIFUNC(AGSObject *) GetObject(int32);
	// get sprite graphic
	AGSIFUNC(BITMAP *) GetSpriteGraphic(int32);
	// create a new blank bitmap
	AGSIFUNC(BITMAP *) CreateBlankBitmap(int32 width, int32 height, int32 coldep);
	// free a created bitamp
	AGSIFUNC(void) FreeBitmap(BITMAP *);

	// *** BELOW ARE INTERFACE VERSION 8 AND ABOVE ONLY
	// get one of the room area masks
	AGSIFUNC(BITMAP *) GetRoomMask(int32);

	// *** BELOW ARE INTERFACE VERSION 9 AND ABOVE ONLY
	// get a particular view frame
	AGSIFUNC(AGSViewFrame *) GetViewFrame(int32 view, int32 loop, int32 frame);
	// get the walk-behind baseline of a specific WB area
	AGSIFUNC(int)    GetWalkbehindBaseline(int32 walkbehind);
	// get the address of a script function
	AGSIFUNC(Plugins::PluginMethod) GetScriptFunctionAddress(const char *funcName);
	// get the transparent colour of a bitmap
	AGSIFUNC(int)    GetBitmapTransparentColor(BITMAP *);
	// get the character scaling level at a particular point
	AGSIFUNC(int)    GetAreaScaling(int32 x, int32 y);
	// equivalent to the text script function
	AGSIFUNC(int)    IsGamePaused();

	// *** BELOW ARE INTERFACE VERSION 10 AND ABOVE ONLY
	// get the raw pixel value to use for the specified AGS colour
	AGSIFUNC(int)    GetRawPixelColor(int32 color);

	// *** BELOW ARE INTERFACE VERSION 11 AND ABOVE ONLY
	// get the width / height of the specified sprite
	AGSIFUNC(int)    GetSpriteWidth(int32);
	AGSIFUNC(int)    GetSpriteHeight(int32);
	// get the dimensions of the specified string in the specified font
	AGSIFUNC(void)   GetTextExtent(int32 font, const char *text, int32 *width, int32 *height);
	// print a message to the debug console
	AGSIFUNC(void)   PrintDebugConsole(const char *text);
	// play a sound on the specified channel
	AGSIFUNC(void)   PlaySoundChannel(int32 channel, int32 soundType, int32 volume, int32 loop, const char *filename);
	// same as text script function
	AGSIFUNC(int)    IsChannelPlaying(int32 channel);

	// *** BELOW ARE INTERFACE VERSION 12 AND ABOVE ONLY
	// invalidate a region of the virtual screen
	AGSIFUNC(void)   MarkRegionDirty(int32 left, int32 top, int32 right, int32 bottom);
	// get mouse cursor details
	AGSIFUNC(AGSMouseCursor *) GetMouseCursor(int32 cursor);
	// get the various components of a pixel
	AGSIFUNC(void)   GetRawColorComponents(int32 coldepth, int32 color, int32 *red, int32 *green, int32 *blue, int32 *alpha);
	// make a pixel colour from the supplied components
	AGSIFUNC(int)    MakeRawColorPixel(int32 coldepth, int32 red, int32 green, int32 blue, int32 alpha);
	// get whether the font is TTF or SCI
	AGSIFUNC(int)    GetFontType(int32 fontNum);
	// create a new dynamic sprite slot
	AGSIFUNC(int)    CreateDynamicSprite(int32 coldepth, int32 width, int32 height);
	// free a created dynamic sprite
	AGSIFUNC(void)   DeleteDynamicSprite(int32 slot);
	// check if a sprite has an alpha channel
	AGSIFUNC(int)    IsSpriteAlphaBlended(int32 slot);

	// *** BELOW ARE INTERFACE VERSION 13 AND ABOVE ONLY
	// un-request an event, requested earlier with RequestEventHook
	AGSIFUNC(void)   UnrequestEventHook(int32 event);
	// draw a translucent bitmap to the active screen
	AGSIFUNC(void)   BlitSpriteTranslucent(int32 x, int32 y, BITMAP *, int32 trans);
	// draw a sprite to the screen, but rotated around its centre
	AGSIFUNC(void)   BlitSpriteRotated(int32 x, int32 y, BITMAP *, int32 angle);

	// *** BELOW ARE INTERFACE VERSION 14 AND ABOVE ONLY
	#ifdef WINDOWS_VERSION
	// get reference to main DirectSound interface
	AGSIFUNC(LPDIRECTSOUND) GetDirectSound();
	#endif
	// disable AGS sound engine
	AGSIFUNC(void)   DisableSound();
	// check whether a script function can be run now
	AGSIFUNC(int)    CanRunScriptFunctionNow();
	// call a user-defined script function
	AGSIFUNC(int)    CallGameScriptFunction(const char *name, int32 globalScript, int32 numArgs, long arg1 = 0, long arg2 = 0, long arg3 = 0);

	// *** BELOW ARE INTERFACE VERSION 15 AND ABOVE ONLY
	// force any sprites on-screen using the slot to be updated
	AGSIFUNC(void)   NotifySpriteUpdated(int32 slot);
	// change whether the specified sprite is a 32-bit alpha blended image
	AGSIFUNC(void)   SetSpriteAlphaBlended(int32 slot, int32 isAlphaBlended);
	// run the specified script function whenever script engine is available
	AGSIFUNC(void)   QueueGameScriptFunction(const char *name, int32 globalScript, int32 numArgs, long arg1 = 0, long arg2 = 0);
	// register a new dynamic managed script object
	AGSIFUNC(int)    RegisterManagedObject(void *object, IAGSScriptManagedObject *callback);
	// add an object reader for the specified object type
	AGSIFUNC(void)   AddManagedObjectReader(const char *typeName, IAGSManagedObjectReader *reader);
	// register an un-serialized managed script object
	AGSIFUNC(void)   RegisterUnserializedObject(int key, void *object, IAGSScriptManagedObject *callback);

	// *** BELOW ARE INTERFACE VERSION 16 AND ABOVE ONLY
	// get the address of a managed object based on its key
	AGSIFUNC(void *)  GetManagedObjectAddressByKey(int key);
	// get managed object's key from its address
	AGSIFUNC(int)    GetManagedObjectKeyByAddress(void *address);

	// *** BELOW ARE INTERFACE VERSION 17 AND ABOVE ONLY
	// create a new script string
	AGSIFUNC(const char *) CreateScriptString(const char *fromText);

	// *** BELOW ARE INTERFACE VERSION 18 AND ABOVE ONLY
	// increment reference count
	AGSIFUNC(int)    IncrementManagedObjectRefCount(void *address);
	// decrement reference count
	AGSIFUNC(int)    DecrementManagedObjectRefCount(void *address);
	// set mouse position
	AGSIFUNC(void)   SetMousePosition(int32 x, int32 y);
	// simulate the mouse being clicked
	AGSIFUNC(void)   SimulateMouseClick(int32 button);
	// get number of waypoints on this movement path
	AGSIFUNC(int)    GetMovementPathWaypointCount(int32 pathId);
	// get the last waypoint that the char/obj passed
	AGSIFUNC(int)    GetMovementPathLastWaypoint(int32 pathId);
	// get the co-ordinates of the specified waypoint
	AGSIFUNC(void)   GetMovementPathWaypointLocation(int32 pathId, int32 waypoint, int32 *x, int32 *y);
	// get the speeds of the specified waypoint
	AGSIFUNC(void)   GetMovementPathWaypointSpeed(int32 pathId, int32 waypoint, int32 *xSpeed, int32 *ySpeed);

	// *** BELOW ARE INTERFACE VERSION 19 AND ABOVE ONLY
	// get the current graphics driver ID
	AGSIFUNC(const char *) GetGraphicsDriverID();

	// *** BELOW ARE INTERFACE VERSION 22 AND ABOVE ONLY
	// get whether we are running under the editor's debugger
	AGSIFUNC(int)    IsRunningUnderDebugger();
	// tells the engine to break into the debugger when the next line of script is run
	AGSIFUNC(void)   BreakIntoDebugger();
	// fills buffer with <install dir>\fileName, as appropriate
	AGSIFUNC(void)   GetPathToFileInCompiledFolder(const char *fileName, char *buffer);

	// *** BELOW ARE INTERFACE VERSION 23 AND ABOVE ONLY
	#ifdef WINDOWS_VERSION
	// get reference to keyboard Direct Input device
	AGSIFUNC(LPDIRECTINPUTDEVICE) GetDirectInputKeyboard();
	// get reference to mouse Direct Input device
	AGSIFUNC(LPDIRECTINPUTDEVICE) GetDirectInputMouse();
	#endif
	// install a replacement renderer for the specified font number
	AGSIFUNC(IAGSFontRenderer *) ReplaceFontRenderer(int fontNumber, IAGSFontRenderer *newRenderer);

	// *** BELOW ARE INTERFACE VERSION 25 AND ABOVE ONLY
	// fills the provided AGSRenderStageDesc struct with current render stage description;
	// please note that plugin MUST fill the struct's Version field before passing it into the function!
	AGSIFUNC(void)  GetRenderStageDesc(AGSRenderStageDesc *desc);

	// *** BELOW ARE INTERFACE VERSION 26 AND ABOVE ONLY
	// fills the provided AGSGameInfo struct
	// please note that plugin MUST fill the struct's Version field before passing it into the function!
	AGSIFUNC(void)  GetGameInfo(AGSGameInfo* ginfo);
	// install a replacement renderer (extended interface) for the specified font number
	AGSIFUNC(IAGSFontRenderer*) ReplaceFontRenderer2(int fontNumber, IAGSFontRenderer2* newRenderer);
	// notify the engine that certain custom font has been updated
	AGSIFUNC(void)  NotifyFontUpdated(int fontNumber);

	// *** BELOW ARE INTERFACE VERSION 27 AND ABOVE ONLY
	// Resolves a script path to a system filepath, same way as script command File.Open does.
	AGSIFUNC(const char *)	ResolveFilePath(const char *script_path);
};

struct EnginePlugin {
	AGS::Shared::String  filename;
	AGS::Engine::Library library;
	Plugins::PluginBase *_plugin = nullptr;
	bool available = false;
	std::vector<uint8_t> savedata;
	int wantHook = 0;
	int invalidatedRegion = 0;
	bool builtin = false;

	IAGSEngine  eiface;

	EnginePlugin() {
		eiface.version = 0;
		eiface.pluginId = 0;
	}
};

extern void PluginSimulateMouseClick(int pluginButtonID);

} // namespace AGS3

#endif
