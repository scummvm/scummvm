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
 * $URL$
 * $Id$
 *
 */

#include "common/rect.h"
#include "sci/engine/vm_types.h"

namespace Sci {

typedef int sciResourceId; // is a resource-number and -1 means no parameter given
typedef reg_t sciMemoryHandle;
typedef uint16 SCIHANDLE;

struct sciNode {
	SCIHANDLE next; // heap handle to next node
	SCIHANDLE prev; // heap handle to data
	uint16 key; // maybe also a heap handle
};
struct sciNode1 : sciNode {
	uint16 value;
};

// sciPort and sciWnd need to be binary identical, so if you change anything in one, you have to change it in the other one
//  as well!
struct sciPort {
	sciNode node; // node struct for list operations
	int16 top, left;
	Common::Rect rect;
	int16 curTop, curLeft;
	int16 fontHeight;
	sciResourceId fontId;
	int16 textFace, penClr, backClr;
	int16 penMode;
};

struct sciWnd {
	// begins like sciPort (needs to be binary identical!!)
	sciNode node; // node struct for list operations
	int16 top, left;
	Common::Rect rect;
	int16 curTop, curLeft;
	int16 fontHeight;
	sciResourceId fontId;
	int16 textFace, penClr, backClr;
	int16 penMode;
	// window specific members
	Common::Rect rect0; // client area of window
	Common::Rect rect1; // total area of window including borders
	uint16 wndStyle;
	uint16 uSaveFlag;
	reg_t hSaved1;
	reg_t hSaved2;
	SCIHANDLE hTitle;
	bool bDrawed;
};

struct sciCast {
	sciNode node;
	uint16 view;
	uint16 loop;
	uint16 cel;
	uint16 z;
	uint16 pal;
	SCIHANDLE hSaved;
	Common::Rect rect;
};

struct sciColor {
	byte used;
	byte r, g, b;
};

struct sciPalette {
	byte mapping[256];
	uint32 timestamp;
	sciColor colors[256];
	byte intensity[256];
};

struct sciPalSched {
	byte from;
	uint32 schedule;
};

#define STUB(str) debug("STUB: '%s' at file %s line %d",str,__FILE__,__LINE__)


// FIXME: The following feature enums and macros should be reconsidered.
// The way they are done right now is highly error prone.

enum {
	FTR_WINDOWMGR_SCI0	= (0<<0),	// SCI0 based window manager
	FTR_WINDOWMGR_SCI1	= (1<<0),	// SCI1 based window manager  
	FTR_CAN_BEHERE		= (0<<1),	// Kernel77 is kCanBeHere (SCI0)
	FTR_CANT_BEHERE		= (1<<1),	// Kernel77 is kCantBeHere(SCI1)
	FTR_SOUND_SCI0		= (0<<2),	// SCI0 Sound functions
	FTR_SOUND_SCI01		= (1<<2),	// SCI0+ Sound functions
	FTR_SOUND_SCI1		= (2<<2),	// SCI1 Sound functions
	FTR_SHOWPIC_SCI0	= (0<<4),	// SCI0 ShowPic (44 effect codes)
	FTR_SHOWPIC_SCI1	= (1<<4),	// SCI1 ShowPic (15 effect codes)
	FTR_GL_ANIMATE		= (1<<5)	// Animate can be paused by global flag
};

#define GETFTR_WINDOWMGR(x) (x & 0x00000001)
#define GETFTR_CANBEHERE(x) (x & 0x00000002)
#define GETFTR_SOUND(x)		(x & 0x0000000C)
#define GETFTR_SHOWPIC(x)	(x & 0x00000010)

enum {
	GFX_STOPUPDATE = 0x01,
	GFX_VIEWUPDATED = 0x02,
	GFX_NOUPDATE = 0x04,
	GFX_HIDDEN = 0x08,
	GFX_FIXEDPRIO=0x10,
	GFX_ALWAYSUPDATE = 0x20,
	GFX_FORCEUPDATE = 0x40,
	GFX_REMOVEVIEW = 0x80
};

enum SCILanguage{
	kLangNone = 0,
	kLangEnglish = 1,
	kLangFrench = 33,
	kLangSpanish = 34,
	kLangItalian = 39,
	kLangGerman = 49
};

}
