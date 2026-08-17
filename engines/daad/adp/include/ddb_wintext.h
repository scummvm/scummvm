#ifndef DDB_WINTEXT_H
#define DDB_WINTEXT_H

#include <os_types.h>

// Screen text tracking for desktop builds. Keeps a record of the text spans
// currently visible on screen so scripted tests can resolve menu options by
// name, and to eventually support scroll-back buffers. Never compiled into
// the memory-constrained ports (Amiga, Atari ST, 16 bit DOS).

#ifndef HAS_WINDOWTEXT
#define HAS_WINDOWTEXT 0
#endif

#if HAS_WINDOWTEXT

extern void WinText_AddSpan   (int x, int y, const uint8_t* text, int length, int pixelWidth);
extern void WinText_Scroll    (int x, int y, int w, int h, int lines);
extern void WinText_ClearRect (int x, int y, int w, int h);

// Searches the visible text for a line shaped like a menu option ("2 NOMBRE")
// containing the given name (ASCII case-insensitive). Returns the option's
// digit character, or 0 if no such line is visible.
extern char WinText_FindMenuOption (const char* name, int nameLength);

// True if the given text (ASCII case-insensitive) is visible anywhere on screen
extern bool WinText_FindText (const char* text, int textLength);

// Writes the currently visible menu-shaped lines into the given buffer
// (newline separated) for diagnostics. Returns the number of lines found.
extern int WinText_ListMenuOptions (char* buffer, int bufferSize);

#else

inline void WinText_AddSpan   (int, int, const uint8_t*, int, int) {}
inline void WinText_Scroll    (int, int, int, int, int) {}
inline void WinText_ClearRect (int, int, int, int) {}
inline char WinText_FindMenuOption (const char*, int) { return 0; }
inline bool WinText_FindText (const char*, int) { return false; }
inline int  WinText_ListMenuOptions (char*, int) { return 0; }

#endif

#endif
