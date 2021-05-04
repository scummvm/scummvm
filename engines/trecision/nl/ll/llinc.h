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
#define MAXMAT 20

namespace Common {
class SeekableReadStream;
}

namespace Trecision {

// DRAW STRUCT
struct SDText {
	Common::Rect _rect;
	Common::Rect _subtitleRect;
	uint16 tcol;
	uint16 scol;
	const char *text;

	void set(SDText org);
	void set(Common::Rect rect, Common::Rect subtitleRect, uint16 tcol, uint16 scol, const char *sign);

	void DText(uint16 *frameBuffer = nullptr);
	uint16 checkDText();
};


struct SDObj {
	Common::Rect rect;
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
extern char *TextArea;
// DTEXT
extern char DTextLines[MAXDTEXTLINES][MAXDTEXTCHARS];
// MOUSE
extern SDText curString;
extern SDText oldString;
extern uint8  TextStatus;

int Compare(const void *p1, const void *p2);
void RedrawRoom();
void ReadLoc();
void TendIn();
void ReadObj(Common::SeekableReadStream *stream);
void ReadExtraObj2C();
void ReadExtraObj41D();
void InitRegenRoom();
void SoundFadIn(int num);

} // End of namespace Trecision

#endif
