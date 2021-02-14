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

//=============================================================================
//
// AGS Plugin interface header file
//
// #define THIS_IS_THE_PLUGIN beforehand if including from the plugin
//
//=============================================================================

#ifndef AGS_ENGINE_PLUGIN_AGSPLUGIN_H
#define AGS_ENGINE_PLUGIN_AGSPLUGIN_H

#include "ags/shared/core/types.h"
#include "common/array.h"

namespace AGS3 {

typedef Common::Array<intptr_t> ScriptMethodParams;

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

// If the user isn't using Allegro or WinGDI, define the BITMAP into something
#if !defined(ALLEGRO_H) && !defined(_WINGDI_) && !defined(BITMAP_DEFINED)
typedef char BITMAP;
#endif

// If not using windows.h, define HWND
#if !defined(_WINDOWS_)
typedef int HWND;
#endif

// This file is distributed as part of the Plugin API docs, so
// ensure that WINDOWS_VERSION is defined (if applicable)
#if AGS_PLATFORM_OS_WINDOWS
#undef WINDOWS_VERSION
#define WINDOWS_VERSION
#endif

// DOS engine doesn't know about stdcall / neither does Linux version
#if !defined (_WIN32)
#define __stdcall
#endif

#define AGSIFUNC(type) virtual type __stdcall

#define MASK_WALKABLE   1
#define MASK_WALKBEHIND 2
#define MASK_HOTSPOT    3
// MASK_REGIONS is interface version 11 and above only
#define MASK_REGIONS    4

// **** WARNING: DO NOT ALTER THESE CLASSES IN ANY WAY!
// **** CHANGING THE ORDER OF THE FUNCTIONS OR ADDING ANY VARIABLES
// **** WILL CRASH THE SYSTEM.

struct AGSColor {
	unsigned char r, g, b;
	unsigned char padding;
};

struct AGSGameOptions {
	int score;      // player's current score
	int usedmode;   // set by ProcessClick to last cursor mode used
	int disabled_user_interface;  // >0 while in cutscene/etc
	int gscript_timer;    // obsolete
	int debug_mode;       // whether we're in debug mode
	int globalvars[50];   // obsolete
	int messagetime;      // time left for auto-remove messages
	int usedinv;          // inventory item last used
	int inv_top, inv_numdisp, inv_numorder, inv_numinline;
	int text_speed;       // how quickly text is removed
	int sierra_inv_color; // background used to paint defualt inv window
	int talkanim_speed;   // animation speed of talking anims
	int inv_item_wid, inv_item_hit; // set by SetInvDimensions
	int speech_text_shadow;         // colour of outline fonts (default black)
	int swap_portrait_side;         // sierra-style speech swap sides
	int speech_textwindow_gui;      // textwindow used for sierra-style speech
	int follow_change_room_timer;   // delay before moving following characters into new room
	int totalscore;           // maximum possible score
	int skip_display;         // how the user can skip normal Display windows
	int no_multiloop_repeat;  // for backwards compatibility
	int roomscript_finished;  // on_call finished in room
	int used_inv_on;          // inv item they clicked on
	int no_textbg_when_voice; // no textwindow bgrnd when voice speech is used
	int max_dialogoption_width; // max width of dialog options text window
	int no_hicolor_fadein;      // fade out but instant in for hi-color
	int bgspeech_game_speed;    // is background speech relative to game speed
	int bgspeech_stay_on_display; // whether to remove bg speech when DisplaySpeech is used
	int unfactor_speech_from_textlength; // remove "&10" when calculating time for text to stay
	int mp3_loop_before_end;    // loop this time before end of track (ms)
	int speech_music_drop;      // how much to drop music volume by when speech is played
	int in_cutscene;            // we are between a StartCutscene and EndCutscene
	int fast_forward;           // player has elected to skip cutscene
	int room_width;             // width of current room
	int room_height;            // height of current room
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
	int defview;
	int talkview;
	int view;
	int room, prevroom;
	int x, y, wait;
	int flags;
	short following;
	short followinfo;
	int idleview;           // the loop will be randomly picked
	short idletime, idleleft; // num seconds idle before playing anim
	short transparency;       // if character is transparent
	short baseline;
	int activeinv;
	int talkcolor;
	int thinkview;
	int reserved[2];
	short walkspeed_y, pic_yoffs;
	int z;
	int reserved2[5];
	short loop, frame;
	short walking, animating;
	short walkspeed, animspeed;
	short inv[301];
	short actx, acty;
	char  name[40];
	char  scrname[20];
	char  on;
};

// AGSObject.flags
#define OBJF_NOINTERACT 1     // not clickable
#define OBJF_NOWALKBEHINDS 2  // ignore walk-behinds

struct AGSObject {
	int x, y;
	int transparent;    // current transparency setting
	int reserved[4];
	short num;            // sprite slot number
	short baseline;       // <=0 to use Y co-ordinate; >0 for specific baseline
	short view, loop, frame; // only used to track animation - 'num' holds the current sprite
	short wait, moving;
	char  cycling;        // is it currently animating?
	char  overall_speed;
	char  on;
	char  flags;
};

// AGSViewFrame.flags
#define FRAF_MIRRORED  1  // flipped left to right

struct AGSViewFrame {
	int pic;            // sprite slot number
	short xoffs, yoffs;
	short speed;
	int flags;
	int sound;          // play sound when this frame comes round
	int reserved_for_future[2];
};

// AGSMouseCursor.flags
#define MCF_ANIMATEMOVE 1
#define MCF_DISABLED    2
#define MCF_STANDARD    4
#define MCF_ONLYANIMOVERHOTSPOT 8

struct AGSMouseCursor {
	int pic;            // sprite slot number
	short hotx, hoty;     // x,y hotspot co-ordinates
	short view;           // view (for animating cursors) or -1
	char  name[10];       // name of cursor mode
	char  flags;          // MCF_flags above
};

// The editor-to-plugin interface
class IAGSEditor {
public:
	int version;
	int pluginId;   // used internally, do not touch this

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


// Below are interface 3 and later
#define AGSE_KEYPRESS        1
#define AGSE_MOUSECLICK      2
#define AGSE_POSTSCREENDRAW  4
// Below are interface 4 and later
#define AGSE_PRESCREENDRAW   8
// Below are interface 5 and later
#define AGSE_SAVEGAME        0x10
#define AGSE_RESTOREGAME     0x20
// Below are interface 6 and later
#define AGSE_PREGUIDRAW      0x40
#define AGSE_LEAVEROOM       0x80
#define AGSE_ENTERROOM       0x100
#define AGSE_TRANSITIONIN    0x200
#define AGSE_TRANSITIONOUT   0x400
// Below are interface 12 and later
#define AGSE_FINALSCREENDRAW 0x800
#define AGSE_TRANSLATETEXT  0x1000
// Below are interface 13 and later
#define AGSE_SCRIPTDEBUG    0x2000
#define AGSE_AUDIODECODE    0x4000 // obsolete, no longer supported
// Below are interface 18 and later
#define AGSE_SPRITELOAD     0x8000
// Below are interface 21 and later
#define AGSE_PRERENDER     0x10000
// Below are interface 24 and later
#define AGSE_PRESAVEGAME     0x20000
#define AGSE_POSTRESTOREGAME 0x40000
#define AGSE_TOOHIGH         0x80000

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
	virtual int Dispose(const char *address, bool force) = 0;
	// return the type name of the object
	virtual const char *GetType() = 0;
	// serialize the object into BUFFER (which is BUFSIZE bytes)
	// return number of bytes used
	virtual int Serialize(const char *address, char *buffer, int bufsize) = 0;
protected:
	IAGSScriptManagedObject() {
	};
	~IAGSScriptManagedObject() {
	};
};

class IAGSManagedObjectReader {
public:
	virtual void Unserialize(int key, const char *serializedData, int dataSize) = 0;
protected:
	IAGSManagedObjectReader() {
	};
	~IAGSManagedObjectReader() {
	};
};

class IAGSFontRenderer {
public:
	virtual bool LoadFromDisk(int fontNumber, int fontSize) = 0;
	virtual void FreeMemory(int fontNumber) = 0;
	virtual bool SupportsExtendedCharacters(int fontNumber) = 0;
	virtual int GetTextWidth(const char *text, int fontNumber) = 0;
	virtual int GetTextHeight(const char *text, int fontNumber) = 0;
	virtual void RenderText(const char *text, int fontNumber, BITMAP *destination, int x, int y, int colour) = 0;
	virtual void AdjustYCoordinateForFont(int *ycoord, int fontNumber) = 0;
	virtual void EnsureTextValidForFont(char *text, int fontNumber) = 0;
protected:
	IAGSFontRenderer() {
	};
	~IAGSFontRenderer() {
	};
};

// The plugin-to-engine interface
class IAGSEngine {
public:
	int version;
	int pluginId;   // used internally, do not touch

public:
	virtual ~IAGSEngine() {}

	// quit the game
	AGSIFUNC(void) AbortGame(const char *reason);
	// get engine version
	AGSIFUNC(const char *) GetEngineVersion();
	// register a script function with the system
	AGSIFUNC(void) RegisterScriptFunction(const char *name, void *address);
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
	AGSIFUNC(void) RequestEventHook(int event);
	// get the options data saved in the editor
	AGSIFUNC(int)  GetSavedData(char *buffer, int bufsize);

	// *** BELOW ARE INTERFACE VERSION 3 AND ABOVE ONLY
	// get the virtual screen
	AGSIFUNC(BITMAP *) GetVirtualScreen();
	// write text to the screen in the specified font and colour
	AGSIFUNC(void) DrawText(int x, int y, int font, int color, const char *text);
	// get screen dimensions
	AGSIFUNC(void) GetScreenDimensions(int *width, int *height, int *coldepth);
	// get screen surface to draw on
	AGSIFUNC(unsigned char **) GetRawBitmapSurface(BITMAP *);
	// release the surface
	AGSIFUNC(void) ReleaseBitmapSurface(BITMAP *);
	// get the current mouse co-ordinates
	AGSIFUNC(void) GetMousePosition(int *x, int *y);

	// *** BELOW ARE INTERFACE VERSION 4 AND ABOVE ONLY
	// get the current room number
	AGSIFUNC(int)  GetCurrentRoom();
	// get the number of background scenes in this room
	AGSIFUNC(int)  GetNumBackgrounds();
	// get the current background frame
	AGSIFUNC(int)  GetCurrentBackground();
	// get a background scene bitmap
	AGSIFUNC(BITMAP *) GetBackgroundScene(int);
	// get dimensions of a bitmap
	AGSIFUNC(void) GetBitmapDimensions(BITMAP *bmp, int *width, int *height, int *coldepth);

	// *** BELOW ARE INTERFACE VERSION 5 AND ABOVE ONLY
	// similar to fwrite - buffer, size, filehandle
	AGSIFUNC(int)  FWrite(void *, int, int);
	// similar to fread - buffer, size, filehandle
	AGSIFUNC(int)  FRead(void *, int, int);
	// print text, wrapping as usual
	AGSIFUNC(void) DrawTextWrapped(int x, int y, int width, int font, int color, const char *text);
	// set the current active 'screen'
	AGSIFUNC(void) SetVirtualScreen(BITMAP *);
	// look up a word in the parser dictionary
	AGSIFUNC(int)  LookupParserWord(const char *word);
	// draw a bitmap to the active screen
	AGSIFUNC(void) BlitBitmap(int x, int y, BITMAP *, int masked);
	// update the mouse and music
	AGSIFUNC(void) PollSystem();

	// *** BELOW ARE INTERFACE VERSION 6 AND ABOVE ONLY
	// get number of characters in game
	AGSIFUNC(int)  GetNumCharacters();
	// get reference to specified character struct
	AGSIFUNC(AGSCharacter *) GetCharacter(int);
	// get reference to game struct
	AGSIFUNC(AGSGameOptions *) GetGameOptions();
	// get reference to current palette
	AGSIFUNC(AGSColor *) GetPalette();
	// update palette
	AGSIFUNC(void) SetPalette(int start, int finish, AGSColor *);

	// *** BELOW ARE INTERFACE VERSION 7 AND ABOVE ONLY
	// get the current player character
	AGSIFUNC(int)  GetPlayerCharacter();
	// adjust to main viewport co-ordinates
	AGSIFUNC(void) RoomToViewport(int *x, int *y);
	// adjust from main viewport co-ordinates (ignores viewport bounds)
	AGSIFUNC(void) ViewportToRoom(int *x, int *y);
	// number of objects in current room
	AGSIFUNC(int)  GetNumObjects();
	// get reference to specified object
	AGSIFUNC(AGSObject *) GetObject(int);
	// get sprite graphic
	AGSIFUNC(BITMAP *) GetSpriteGraphic(int);
	// create a new blank bitmap
	AGSIFUNC(BITMAP *) CreateBlankBitmap(int width, int height, int coldep);
	// free a created bitamp
	AGSIFUNC(void) FreeBitmap(BITMAP *);

	// *** BELOW ARE INTERFACE VERSION 8 AND ABOVE ONLY
	// get one of the room area masks
	AGSIFUNC(BITMAP *) GetRoomMask(int);

	// *** BELOW ARE INTERFACE VERSION 9 AND ABOVE ONLY
	// get a particular view frame
	AGSIFUNC(AGSViewFrame *) GetViewFrame(int view, int loop, int frame);
	// get the walk-behind baseline of a specific WB area
	AGSIFUNC(int)    GetWalkbehindBaseline(int walkbehind);
	// get the address of a script function
	AGSIFUNC(void *) GetScriptFunctionAddress(const char *funcName);
	// get the transparent colour of a bitmap
	AGSIFUNC(int)    GetBitmapTransparentColor(BITMAP *);
	// get the character scaling level at a particular point
	AGSIFUNC(int)    GetAreaScaling(int x, int y);
	// equivalent to the text script function
	AGSIFUNC(int)    IsGamePaused();

	// *** BELOW ARE INTERFACE VERSION 10 AND ABOVE ONLY
	// get the raw pixel value to use for the specified AGS colour
	AGSIFUNC(int)    GetRawPixelColor(int color);

	// *** BELOW ARE INTERFACE VERSION 11 AND ABOVE ONLY
	// get the width / height of the specified sprite
	AGSIFUNC(int)    GetSpriteWidth(int);
	AGSIFUNC(int)    GetSpriteHeight(int);
	// get the dimensions of the specified string in the specified font
	AGSIFUNC(void)   GetTextExtent(int font, const char *text, int *width, int *height);
	// print a message to the debug console
	AGSIFUNC(void)   PrintDebugConsole(const char *text);
	// play a sound on the specified channel
	AGSIFUNC(void)   PlaySoundChannel(int channel, int soundType, int volume, int loop, const char *filename);
	// same as text script function
	AGSIFUNC(int)    IsChannelPlaying(int channel);

	// *** BELOW ARE INTERFACE VERSION 12 AND ABOVE ONLY
	// invalidate a region of the virtual screen
	AGSIFUNC(void)   MarkRegionDirty(int left, int top, int right, int bottom);
	// get mouse cursor details
	AGSIFUNC(AGSMouseCursor *) GetMouseCursor(int cursor);
	// get the various components of a pixel
	AGSIFUNC(void)   GetRawColorComponents(int coldepth, int color, int *red, int *green, int *blue, int *alpha);
	// make a pixel colour from the supplied components
	AGSIFUNC(int)    MakeRawColorPixel(int coldepth, int red, int green, int blue, int alpha);
	// get whether the font is TTF or SCI
	AGSIFUNC(int)    GetFontType(int fontNum);
	// create a new dynamic sprite slot
	AGSIFUNC(int)    CreateDynamicSprite(int coldepth, int width, int height);
	// free a created dynamic sprite
	AGSIFUNC(void)   DeleteDynamicSprite(int slot);
	// check if a sprite has an alpha channel
	AGSIFUNC(int)    IsSpriteAlphaBlended(int slot);

	// *** BELOW ARE INTERFACE VERSION 13 AND ABOVE ONLY
	// un-request an event, requested earlier with RequestEventHook
	AGSIFUNC(void)   UnrequestEventHook(int event);
	// draw a translucent bitmap to the active screen
	AGSIFUNC(void)   BlitSpriteTranslucent(int x, int y, BITMAP *, int trans);
	// draw a sprite to the screen, but rotated around its centre
	AGSIFUNC(void)   BlitSpriteRotated(int x, int y, BITMAP *, int angle);

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
	AGSIFUNC(int)    CallGameScriptFunction(const char *name, int globalScript, int numArgs, long arg1 = 0, long arg2 = 0, long arg3 = 0);

	// *** BELOW ARE INTERFACE VERSION 15 AND ABOVE ONLY
	// force any sprites on-screen using the slot to be updated
	AGSIFUNC(void)   NotifySpriteUpdated(int slot);
	// change whether the specified sprite is a 32-bit alpha blended image
	AGSIFUNC(void)   SetSpriteAlphaBlended(int slot, int isAlphaBlended);
	// run the specified script function whenever script engine is available
	AGSIFUNC(void)   QueueGameScriptFunction(const char *name, int globalScript, int numArgs, long arg1 = 0, long arg2 = 0);
	// register a new dynamic managed script object
	AGSIFUNC(int)    RegisterManagedObject(const void *object, IAGSScriptManagedObject *callback);
	// add an object reader for the specified object type
	AGSIFUNC(void)   AddManagedObjectReader(const char *typeName, IAGSManagedObjectReader *reader);
	// register an un-serialized managed script object
	AGSIFUNC(void)   RegisterUnserializedObject(int key, const void *object, IAGSScriptManagedObject *callback);

	// *** BELOW ARE INTERFACE VERSION 16 AND ABOVE ONLY
	// get the address of a managed object based on its key
	AGSIFUNC(void *)  GetManagedObjectAddressByKey(int key);
	// get managed object's key from its address
	AGSIFUNC(int)    GetManagedObjectKeyByAddress(const char *address);

	// *** BELOW ARE INTERFACE VERSION 17 AND ABOVE ONLY
	// create a new script string
	AGSIFUNC(const char *) CreateScriptString(const char *fromText);

	// *** BELOW ARE INTERFACE VERSION 18 AND ABOVE ONLY
	// increment reference count
	AGSIFUNC(int)    IncrementManagedObjectRefCount(const char *address);
	// decrement reference count
	AGSIFUNC(int)    DecrementManagedObjectRefCount(const char *address);
	// set mouse position
	AGSIFUNC(void)   SetMousePosition(int x, int y);
	// simulate the mouse being clicked
	AGSIFUNC(void)   SimulateMouseClick(int button);
	// get number of waypoints on this movement path
	AGSIFUNC(int)    GetMovementPathWaypointCount(int pathId);
	// get the last waypoint that the char/obj passed
	AGSIFUNC(int)    GetMovementPathLastWaypoint(int pathId);
	// get the co-ordinates of the specified waypoint
	AGSIFUNC(void)   GetMovementPathWaypointLocation(int pathId, int waypoint, int *x, int *y);
	// get the speeds of the specified waypoint
	AGSIFUNC(void)   GetMovementPathWaypointSpeed(int pathId, int waypoint, int *xSpeed, int *ySpeed);

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
};

} // namespace AGS3

#endif
