#pragma once

#include <ddb.h>

#ifndef HAS_PROGRESS_BAR
#if defined(_STDCLIB)
#define HAS_PROGRESS_BAR 0
#else
#define HAS_PROGRESS_BAR 1
#endif
#endif

extern bool         exitGame;
extern DDB_Machine  screenMachine;
extern DDB_ScreenMode screenMode;

enum VID_ClearMode
{
	Clear_Text = 0,
	Clear_All = 1,
};

extern bool   VID_Initialize          (DDB_Machine machine, DDB_Version version, DDB_ScreenMode mode);
extern uint32_t VID_GetSupportedDataFileModes();
extern void   VID_SetDisplayPlanesHint(uint8_t planes);
extern void   VID_Finish              ();

extern bool   VID_AnyKey              ();
extern bool   VID_LoadDataFile        (const char* filename);
extern void   VID_CloseDataFile       ();
extern bool   VID_PictureExists       (uint8_t picno);
extern bool   VID_SampleExists        (uint8_t no);
extern void   VID_Clear               (int x, int y, int w, int h, uint8_t color, VID_ClearMode mode = Clear_Text);
extern void   VID_ClearBuffer         (bool front);
extern void   VID_DisplayPicture      (int x, int y, int w, int h, DDB_ScreenMode screenMode);
extern bool   VID_DisplaySCRFile      (const char* fileName, DDB_Machine target, bool fadeIn);
// Save/restore the visible screen around the loader prompts, so a prompt
// box can be removed without reloading the screen behind it from disk
// (which may live on another disk in a multi-disk setup). Backends without
// support return false and callers fall back to redisplay/clear.
extern bool   VID_BackupScreen        ();
extern bool   VID_RestoreBackupScreen ();
extern void   VID_DrawCharacter       (int x, int y, uint8_t ch, uint8_t ink, uint8_t paper);
extern void   VID_DrawTextSpan        (int x, int y, const uint8_t* text, uint16_t length, uint8_t ink, uint8_t paper);
extern void   VID_DrawText            (int x, int y, const char* text, uint8_t ink, uint8_t paper);
extern void   VID_GetKey              (uint8_t* key, uint8_t* ext, uint8_t* modifiers);
extern void   VID_GetMilliseconds     (uint32_t* time);
extern void   VID_GetPaletteColor     (uint8_t color, uint8_t* r, uint8_t* g, uint8_t* b);
extern void   VID_GetPictureInfo      (bool* fixed, int16_t* x, int16_t* y, int16_t* w, int16_t* h);
extern void   VID_LoadPicture         (uint8_t picno, DDB_ScreenMode screenMode);
extern void   VID_MainLoop            (DDB_Interpreter* i, void (*callback)(int elapsed));
extern void   VID_MainLoopAsync       (DDB_Interpreter* i, void (*callback)(int elapsed));
extern void   VID_OpenFileDialog      (bool existing, char* filename, size_t bufferSize);
extern void   VID_PlaySample          (uint8_t no, int* duration);
extern void   VID_PlaySampleBuffer    (void* buffer, int samples, int hz, int volume);
extern void   VID_StopSampleIfOverlaps(const void* buffer, uint32_t size);
extern void   VID_Quit                ();
extern void   VID_Scroll              (int x, int y, int w, int h, int lines, uint8_t paper);
extern void   VID_SetDefaultPalette   ();
extern void   VID_ResetDisplay        ();
extern void   VID_SetPaletteEntries   (const uint32_t* palette, uint16_t count, uint16_t firstColor, bool clearOutside, bool waitForVBlank);
extern void   VID_SetPaletteRange     (const uint32_t* palette, uint16_t count, uint16_t firstColor, bool clearOutside, bool waitForVBlank);
extern void   VID_SetPaletteColor     (uint8_t color, uint8_t r, uint8_t g, uint8_t b);
extern void   VID_SetTextInputMode    (bool enabled);
extern void   VID_VSync               ();
#if HAS_TESTMODE
extern void   VID_SetFastMode          (bool enabled);
extern bool   VID_IsFastMode           ();
#endif
extern void   VID_WaitForKey          ();		// Not suported in all platforms
extern void   VID_ShowError           (const char* msg);
extern const char* VID_DescribeVideoModeError(DDB_Error error, DDB_ScreenMode mode);
extern void   VID_ActivatePalette     ();
extern void   VID_SetFadeEnabled      (bool enabled);
extern bool   VID_IsFadeEnabled       ();
extern void   VID_SetProgressBarEnabled(bool enabled);
extern void   VID_ShowProgressBar     (uint16_t amount);
extern void   VID_InnerLoop           ();
extern void   VID_SetCharset          (const uint8_t* charset);
extern void   VID_SetCharsetWidth     (uint8_t width);
extern void   VID_SetWindowTitle      (const char* title);
extern void   VID_SetWindowIcon       (const char* fileName);
#if HAS_TESTMODE
extern bool   VID_SaveScreenshot      (const char* fileName);
#endif

// Backbuffer support
extern void   VID_EnableBackBuffer    ();
extern bool   VID_IsBackBufferEnabled ();
extern void   VID_SaveScreen          ();
extern void   VID_RestoreScreen       ();
extern void   VID_SetOpBuffer         (SCR_Operation op, bool front);
extern void   VID_SwapScreen          ();

extern uint16_t VID_GetPaletteSize    ();

inline  void  VID_SetPaletteColor32   (uint8_t color, uint32_t rgb)
{
	VID_SetPaletteColor(color, (rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
}

#if HAS_DRAWSTRING
extern void    VID_SetInk     		  (uint8_t color);
extern void    VID_SetPaper     	  (uint8_t color);
extern void    VID_MoveTo			  (uint16_t x, uint16_t y);
extern void    VID_MoveBy			  (int16_t x, int16_t y);
extern void    VID_DrawPixel		  (uint8_t color);
extern void    VID_DrawPixel          (int16_t x, int16_t y, uint8_t color);
extern void    VID_DrawLine  		  (int16_t x, int16_t y, uint8_t color);
extern void    VID_PatternFill        (int16_t x, int16_t y, int pattern, bool invert = false);
extern void    VID_AttributeFill	  (uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
extern uint8_t VID_GetAttributes      ();
extern void    VID_SetAttributes      (uint8_t attributes);
extern uint8_t VID_GetInk             ();
extern uint8_t VID_GetPaper           ();
#endif

#if HAS_CLIPBOARD
extern bool   VID_HasClipboardText    (uint32_t* size);
extern void   VID_GetClipboardText    (uint8_t* buffer, uint32_t bufferSize);
extern void   VID_SetClipboardText    (uint8_t* buffer, uint32_t bufferSize);
#endif

#if HAS_FULLSCREEN
extern void   VID_ToggleFullscreen    ();
#endif

#if HAS_PCX
extern void   VID_SetExternalPictureBase (const char* fileName);
extern bool   VID_GetExternalPictureFileName (uint8_t picno, char* fileName, size_t fileNameSize);
extern bool   VID_HasExternalPictures (); 
#endif
