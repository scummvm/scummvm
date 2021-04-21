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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TRECISION_LLINC_H
#define TRECISION_LLINC_H

#include "common/rect.h"
#include "trecision/nl/sysdef.h"

#define DRAWMASK        1
#define COPYTORAM       2

#define TEXT_OFF	0
#define TEXT_DRAW   2
#define TEXT_DEL    4

#define MAXTEXTAREA		200000L
#define SMKANBUFFER     6L
#define SMKBKGBUFFER    4L
#define SMKICONBUFFER   2L

#define MAXSYSTEXT	30

namespace Common {
class SeekableReadStream;
}

namespace Trecision {

// DRAW STRUCT
struct SDText {
	uint16 x;
	uint16 y;
	uint16 dx;
	uint16 dy;
	Common::Rect _subtitleRect;
	uint16 tcol;
	uint16 scol;
	const char *text;
	
	void set(SDText org);
	void set(uint16 x, uint16 y, uint16 dx, uint16 dy, uint16 l0, uint16 l1, uint16 l2, uint16 l3, uint16 tcol, uint16 scol, const char *sign);
	
	void clear();
	void DText(uint16 *frameBuffer = nullptr);
	uint16 checkDText();
};


struct SDObj {
	uint16 x;
	uint16 y;
	uint16 dx;
	uint16 dy;
	Common::Rect l;
	int objIndex;
	bool drawMask;
};

// fastfile
struct SFileEntry {
	char name[12];
	int offset;
};

struct SFileHandle {
	char inuse;
	int pos;
	int size;
	SFileEntry *pfe;
};


// FUNCTIONS

// GAME POINTER
extern uint16 _actionPosition[];
// DATA POINTER
extern uint8 *TextArea;
// DTEXT
extern int8 DTextLines[MAXDTEXTLINES][MAXDTEXTCHARS];
// MOUSE
extern SDText curString;
extern SDText oldString;
extern uint8  TextStatus;

int Compare(const void *p1, const void *p2);
void RedrawRoom();
bool DataSave();
bool DataLoad();
bool QuitGame();
void DemoOver();
void openSys();
void ReadLoc();
void TendIn();
void ReadObj(Common::SeekableReadStream *stream);
void ReadExtraObj2C();
void ReadExtraObj41D();
void RegenRoom();
void PaintRegenRoom();
void SoundFadIn(int num);
void resetZBuffer(int x1, int y1, int x2, int y2);

} // End of namespace Trecision

#endif
