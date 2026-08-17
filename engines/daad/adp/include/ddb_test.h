#pragma once

#ifndef HAS_TESTMODE
#define HAS_TESTMODE 0
#endif

#if HAS_TESTMODE

#include <os_types.h>

typedef bool (*DDB_ScreenshotCallback)(const char* fileName);

extern void        DDB_TestLoadInput             (const char* fileName);
extern bool        DDB_TestIsActive              ();
extern bool        DDB_TestGetKey                (uint8_t* key, uint8_t* ext, uint8_t* mod);
extern bool        DDB_TestAnyKey                ();
extern bool        DDB_TestAnyKeyForMore         (bool allowCaptures);
extern bool        DDB_TestAnyKeyForWait         (bool allowCaptures);
extern bool        DDB_TestHasScriptedInput      ();
extern void        DDB_TestSetScreenshotCallback (DDB_ScreenshotCallback callback);
extern void        DDB_TestEnableInteractiveInput();
extern bool        DDB_TestHasError              ();
extern const char* DDB_TestGetError              ();
extern bool        DDB_TestWindowMuted           (int winno);
extern bool        DDB_TestChanceForced          (bool* result, bool indirect, uint8_t rawParam);
extern void        DDB_TestSetFlag               (uint8_t flag, uint8_t value);

// Part selector automation. When a scenario names a part, the runner presets it
// here so the player auto-selects that part (1-based) instead of waiting for a
// keypress, and optionally captures a screenshot of the selector prompt.
extern void        DDB_TestSetPartSelection      (int part, const char* screenshotFileName);
extern int         DDB_TestGetPartSelection      ();
extern bool        DDB_TestCapturePartSelector   ();

#endif
