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

#ifndef GLK_LEVEL9_LEVEL9_MAIN
#define GLK_LEVEL9_LEVEL9_MAIN

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/stream.h"
#include "common/serializer.h"

namespace Glk {
namespace Level9 {

typedef byte L9BYTE;
typedef uint16 L9UINT16;
typedef uint32 L9UINT32;
typedef bool L9BOOL;

#define FALSE false
#define TRUE true

#define LISTAREASIZE 0x800
#define STACKSIZE 1024
#define V1FILESIZE 0x600

#ifndef MAX_PATH
#define MAX_PATH 256
#endif

struct GameState {
	L9UINT32 Id;
	L9UINT16 codeptr, stackptr, listsize, stacksize;
	L9UINT16 vartable[256];
	L9BYTE listarea[LISTAREASIZE];
	L9UINT16 stack[STACKSIZE];
	uint16 checksum;

	void synchronize(Common::Serializer &s);

	void calculateChecksum();
};

enum BitmapType {
	NO_BITMAPS,
	AMIGA_BITMAPS,
	PC1_BITMAPS,
	PC2_BITMAPS,
	C64_BITMAPS,
	BBC_BITMAPS,
	CPC_BITMAPS,
	MAC_BITMAPS,
	ST1_BITMAPS,
	ST2_BITMAPS
};

struct Colour {
	L9BYTE red, green, blue;
};

struct Bitmap {
	L9UINT16 width, height;
	L9BYTE *bitmap;
	Colour palette[32];
	L9UINT16 npalette;
};

#define MAX_BITMAP_WIDTH 512
#define MAX_BITMAP_HEIGHT 218

#define L9WORD(x) READ_LE_UINT16(x)
#define L9SETWORD(x,val) WRITE_LE_UINT16(x, val)
#define L9SETDWORD(x,val) WRITE_LE_UINT32(x, val)

extern byte *startdata;
extern uint32 FileSize;

extern void level9_initialize();
extern void printstring(const char *buf);

/* routines provided by os dependent code */
extern void os_printchar(char c);
extern L9BOOL os_input(char *ibuff, int size);
extern char os_readchar(int millis);
extern L9BOOL os_stoplist(void);
extern void os_flush(void);
extern L9BOOL os_get_game_file(char *NewName, int Size);
extern void os_set_filenumber(char *NewName, int Size, int n);
extern void os_graphics(int mode);
extern void os_cleargraphics(void);
extern void os_setcolour(int colour, int index);
extern void os_drawline(int x1, int y1, int x2, int y2, int colour1, int colour2);
extern void os_fill(int x, int y, int colour1, int colour2);
extern void os_show_bitmap(int pic, int x, int y);
extern Common::SeekableReadStream *os_open_script_file(void);

/* routines provided by level9 interpreter */
extern L9BOOL LoadGame(const char *filename, char *picname);
extern L9BOOL RunGame(void);
extern void StopGame(void);
extern void RestoreGame(char *filename);
extern void FreeMemory(void);
extern void GetPictureSize(int *width, int *height);
extern L9BOOL RunGraphics(void);

/* bitmap routines provided by level9 interpreter */
BitmapType DetectBitmaps(char *dir);
Bitmap *DecodeBitmap(char *dir, BitmapType type, int num, int x, int y);

} // End of namespace Level9
} // End of namespace Glk

#endif
