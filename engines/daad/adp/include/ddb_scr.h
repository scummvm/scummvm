#pragma once

/**
 * @file ddb_scr.h
 * @brief High level screen routines. This is mostly a wrapper over ddb_vid.h,
 * but it also provides a buffering mechanism for screen operations.
 */

#include <ddb.h>
#include <ddb_vid.h>

enum   DDB_ScreenMode;

extern bool       supportsOpenFileDialog;
extern bool		  waitingForKey;
extern bool       buffering;

extern uint8_t    lineHeight;
extern uint8_t    columnWidth;
extern uint8_t    screenCellWidth;
extern uint16_t   screenWidth;
extern uint16_t   screenHeight;
extern uint8_t    charset[2048];
extern uint8_t    charWidth[256];

enum SCR_Command
{
	SCR_COMMAND_CLEAR,
	SCR_COMMAND_SETATTRIBUTES,
	SCR_COMMAND_SCROLL,
	SCR_COMMAND_DRAWCHARACTER,
	#if HAS_DRAWSTRING
	SCR_COMMAND_DRAWVECTORPICTURE,
	#endif
	SCR_COMMAND_LOADPICTURE,
	SCR_COMMAND_DISPLAYPICTURE,
	SCR_COMMAND_WAITFORKEY,
	SCR_COMMAND_SAVE,
	SCR_COMMAND_RESTORE,
	SCR_COMMAND_SWAP,
	SCR_COMMAND_SETOPBUFFER,
	SCR_COMMAND_CLEARBUFFER,
};

struct SCR_CommandData
{
	uint8_t  type;
	uint8_t  n;
	uint8_t  ink;
	uint8_t  paper;
	uint16_t x;
	uint16_t y;
	uint16_t w;
	uint16_t h;
};

enum SCR_KeyMod
{
	SCR_KEYMOD_SHIFT    = 0x01,
	SCR_KEYMOD_CTRL     = 0x02,
	SCR_KEYMOD_ALT      = 0x04,
	SCR_KEYMOD_CAPSLOCK = 0x80
};

typedef void (*MainLoopCallback)(int elapsed);
#if  NO_BUFFERING

#include <ddb_vid.h>

#define SCR_AnyKey           VID_AnyKey
#define SCR_PictureExists    VID_PictureExists
#define SCR_SampleExists     VID_SampleExists
#define SCR_Clear            VID_Clear
#define SCR_ClearBuffer      VID_ClearBuffer
#define SCR_DisplayPicture   VID_DisplayPicture
#define SCR_DrawCharacter    VID_DrawCharacter
#define SCR_DrawTextSpan     VID_DrawTextSpan
#define SCR_GetKey           VID_GetKey
#define SCR_GetMilliseconds  VID_GetMilliseconds
#define SCR_GetPaletteColor  VID_GetPaletteColor
#define SCR_GetPictureInfo   VID_GetPictureInfo
#define SCR_OpenFileDialog   VID_OpenFileDialog
#define SCR_PlaySample       VID_PlaySample
#define SCR_PlaySampleBuffer VID_PlaySampleBuffer
#define SCR_MainLoop         VID_MainLoop
#define SCR_Quit             VID_Quit
#define SCR_RestoreScreen    VID_RestoreScreen
#define SCR_SaveScreen       VID_SaveScreen
#define SCR_SetOpBuffer      VID_SetOpBuffer
#define SCR_SetPaletteColor  VID_SetPaletteColor
#define SCR_SetTextInputMode VID_SetTextInputMode
#define SCR_SwapScreen       VID_SwapScreen

static inline bool SCR_LoadPicture (uint8_t picno, DDB_ScreenMode screenMode)
{
	VID_LoadPicture(picno, screenMode);
	bool fixed;
	int16_t x = 0, y = 0, w = 0, h = 0;
	VID_GetPictureInfo(&fixed, &x, &y, &w, &h);
	return w > 0 && h > 0;
}

#if HAS_DRAWSTRING
static inline void SCR_DrawVectorPicture(uint8_t picno)
{
	uint8_t attributes = VID_GetAttributes();
	DDB_ExecuteVectorPicture(picno);
	VID_SetAttributes(attributes);
}
#endif

static inline void SCR_Scroll(int x, int y, int w, int h, int lines, uint8_t paper, bool smooth)
{
	VID_Scroll(x, y, w, h, lines, paper);
}

static inline bool SCR_Synchronized()  { return true; }
static inline void SCR_ConsumeBuffer() {}
static inline void SCR_ConsumeFullBuffer() {}
static inline void SCR_SetAttributes(uint8_t attributes)
{
	#if HAS_DRAWSTRING
	VID_SetAttributes(attributes);
	#else
	(void)attributes;
	#endif
}
static inline void SCR_WaitForKey()
{
	uint8_t key, ext;
	VID_WaitForKey();
	VID_GetKey(&key, &ext, 0);
}

#if HAS_TESTMODE
// In the non-buffered (DOS) build the SCR_* input helpers are plain macros to
// the VID_* layer, which would bypass the scripted-input test hooks. Re-route
// the key-input entry points through the test layer so fixture input scripts
// drive the game exactly as they do on the buffered desktop build.
#include <ddb_test.h>
#undef  SCR_AnyKey
#undef  SCR_GetKey
static inline bool SCR_AnyKey()
{
	return DDB_TestIsActive() ? DDB_TestAnyKey() : VID_AnyKey();
}
static inline bool SCR_AnyKeyForMore(bool allowCaptures)
{
	return DDB_TestIsActive() ? DDB_TestAnyKeyForMore(allowCaptures) : VID_AnyKey();
}
static inline void SCR_GetKey(uint8_t* key, uint8_t* ext, uint8_t* mod)
{
	if (!DDB_TestGetKey(key, ext, mod))
		VID_GetKey(key, ext, mod);
}
static inline void SCR_UseInputFile(const char* filename)
{
	DDB_TestLoadInput(filename);
}
#endif

#else

extern bool SCR_AnyKey           ();
#if HAS_TESTMODE
extern bool SCR_AnyKeyForMore    (bool allowCaptures);
#endif
extern bool SCR_LoadPicture      (uint8_t picno, DDB_ScreenMode screenMode);
extern bool SCR_PictureExists    (uint8_t picno);
extern bool SCR_SampleExists     (uint8_t no);
extern bool SCR_Synchronized     ();
extern void SCR_Clear            (int x, int y, int w, int h, uint8_t color, VID_ClearMode mode);
extern void SCR_ClearBuffer      (bool front);
extern void SCR_ConsumeBuffer    ();
extern void SCR_ConsumeFullBuffer();
extern void SCR_DisplayPicture   (int x, int y, int w, int h, DDB_ScreenMode mode);
extern void SCR_DrawCharacter    (int x, int y, uint8_t ch, uint8_t ink, uint8_t paper);
extern void SCR_DrawTextSpan     (int x, int y, const uint8_t* text, uint16_t length, uint8_t ink, uint8_t paper);
#if HAS_DRAWSTRING
extern void SCR_DrawVectorPicture(uint8_t picno);
#endif
extern void SCR_GetKey           (uint8_t* key, uint8_t* ext, uint8_t* modifiers);
extern void SCR_GetMilliseconds  (uint32_t* time);
extern void SCR_GetPaletteColor  (uint8_t color, uint8_t* r, uint8_t* g, uint8_t* b);
extern void SCR_GetPictureInfo   (bool* fixed, int16_t* x, int16_t* y, int16_t* w, int16_t* h);
extern void SCR_OpenFileDialog	 (bool existing, char* filename, size_t bufferSize);
extern void SCR_PlaySample       (uint8_t no, int* duration);
extern void SCR_PlaySampleBuffer (void* buffer, int samples, int hz, int volume);
extern void SCR_MainLoop         (DDB_Interpreter* i, MainLoopCallback callback);
extern void SCR_Quit			 ();
extern void SCR_RestoreScreen    ();
extern void SCR_SaveScreen       ();
extern void SCR_Scroll           (int x, int y, int w, int h, int lines, uint8_t paper, bool smooth);
extern void SCR_SetAttributes    (uint8_t attributes);
extern void SCR_SetOpBuffer      (SCR_Operation op, bool front);
extern void SCR_SetPaletteColor  (uint8_t color, uint8_t r, uint8_t g, uint8_t b);
extern void SCR_SetTextInputMode (bool enabled);
extern void SCR_SwapScreen       ();
extern void SCR_WaitForKey       ();
#if HAS_TESTMODE
extern void SCR_UseInputFile     (const char* filename);
#endif

#ifdef __cplusplus
inline void SCR_Clear(int x, int y, int w, int h, uint8_t color)
{
	SCR_Clear(x, y, w, h, color, Clear_Text);
}
#endif

#endif
