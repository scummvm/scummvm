#ifndef VID_FONT_H
#define VID_FONT_H

#include <os_types.h>

// Platforms with a double-resolution (2x) text mode define HAS_HIRES_FONT to 1
// and implement VID_StoreFont2X below; everyone else gets the plain 8-bit path
// only, with no extra globals.
#ifndef HAS_HIRES_FONT
#define HAS_HIRES_FONT 0
#endif

struct DMG_Font;

// Shared charset/font loaders (src-common/vid_font.cpp). Both fill the global
// charset[]/charWidth[]. Kept in one place so the file handling and the tight
// scratch-buffer discipline can't drift per platform (they used to).
bool SCR_LoadCharset    (uint8_t* ptr, const char* filename);   // 2176-byte .CHR
bool SCR_LoadSINTACFont (const char* filename);                 // SINTAC .FNT

// Platform hooks the loaders call:
//   VID_ActivateCharset - rebuild any charset-derived state after a load
//                         (Amiga rotation tables, desktop "initialized" flag, ...).
extern void VID_ActivateCharset();
#if HAS_HIRES_FONT
//   VID_StoreFont2X     - when a 2x mode is active, store the high-res glyphs
//                         (native 16-bit V4 or 2x-widened 8-bit) and return true;
//                         return false to fall back to the plain 8-bit path.
bool VID_StoreFont2X(const DMG_Font* font, const char* filename);
#endif

#endif
