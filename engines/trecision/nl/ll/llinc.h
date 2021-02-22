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

#define DRAWMASK        1
#define COPYTORAM       2
#define COPYTOVIDEO     4

#define TEXT_OFF	0
#define TEXT_ON     1
#define TEXT_DRAW   2
#define TEXT_DEL    4

#define MAXTEXTAREA		200000L
#define SMKANBUFFER     6L
#define SMKBKGBUFFER    4L
#define SMKICONBUFFER   2L

#define MAXSYSTEXT	30

#define MAXSMACK	3
#define MAXFILEREF		5000

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
	uint16 l[4];
	uint16 tcol;
	uint16 scol;
	const char *sign;
	
	void set(SDText org);
	void set(uint16 x, uint16 y, uint16 dx, uint16 dy, uint16 l0, uint16 l1, uint16 l2, uint16 l3, uint16 tcol, uint16 scol, const char *sign);
	
	void clear();
	void DText();
	uint16 checkDText();
};


struct SDObj {
	uint16 x;
	uint16 y;
	uint16 dx;
	uint16 dy;
	uint16 l[4];
	uint16 *buf;
	uint8 *mask;
	uint8 flag;         /* 0 - with mask
                            1 - copy to Video
                            2 - copy to Video2 */

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

// VIDEO
extern uint16 *_video, VideoPitch;
extern uint16 *Video2;
// GAME POINTER
extern uint16 *ImagePointer;
extern uint16 *SmackImagePointer;
extern uint16 *ObjPointers[];
extern uint8 *MaskPointers[];
extern uint8 *SoundPointer[];
extern uint8 *_actionPointer[];
extern uint16 _actionPosition[];
extern uint8 *SmackBuffer[];
extern uint8 *SmackTempBuffer[];
// DATA POINTER
extern uint16 *Icone;
extern uint8 *Font;
extern uint8 *IntroFont;
extern uint16 *Arrows;
extern uint8 *TextArea;
// 3D AREA
extern int16  *ZBuffer;
extern uint8 *_characterArea;
extern uint8 *TextureArea;
extern uint8 *BaseHeadTexture;
extern uint8 AddObjectMaterial;
// MEMORY
extern uint32 GameBytePointer;
extern uint32 GameWordPointer;
extern uint32 TotalMemory;
// VESA
extern uint8 CVP;
extern uint16 VesaGranularity;
extern int32  CurRoomMaxX;
// DTEXT
extern int8 DTextLines[MAXDTEXTLINES][MAXDTEXTCHARS];
// SCROLL
extern int32 VirtualPageLen;
extern int32 CurScrollPageDx;
extern int32 VideoScrollPageDx;
// ANIMATION
extern uint8 *SoundBuffer;
extern uint32 CurBufferSize;
extern uint8 *MemoryArea;
extern uint8 *SoundStartBuffer;
// SOUND
extern uint8 SoundSystemActive;
extern uint8 SpeechTrackEnabled;
// MOUSE
extern SDText curString;
extern SDText oldString;
extern uint8  TextStatus;
extern Common::SeekableReadStream *ff;
// FILEREF
extern SFileEntry FileRef[];
extern int NumFileRef;

} // End of namespace Trecision

#include "trecision/nl/ll/llproto.h"

#endif
