/* Copyright (C) 1994-2004 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef DRIVER96_H
#define DRIVER96_H

namespace Sword2 {

class Sword2Engine;

// Error codes

enum {
	// Generic error codes

	RD_OK,
	RDERR_UNKNOWN,
	RDERR_INVALIDPOINTER,
	RDERR_OUTOFMEMORY,
	RDERR_INVALIDFILENAME,

	// Drawing error codes

	RDERR_DECOMPRESSION,

	// Language and version error codes

	RDERR_OPENVERSIONFILE,

	// Keyboard error codes

	RDERR_NOKEYWAITING,

	// Sprite drawing error codes

	RDERR_NOTIMPLEMENTED,
	RDERR_UNKNOWNTYPE,
	RDERR_INVALIDSCALING,
	RDERR_NOTCLOSED,
	RDERR_NOTOPEN,

	// Menubar error codes

	RDERR_INVALIDMENU,
	RDERR_INVALIDPOCKET,
	RDERR_INVALIDCOMMAND,

	// Palette fading error codes

	RDERR_FADEINCOMPLETE,

	// Sound engine error codes

	RDERR_SPEECHPLAYING,
	RDERR_SPEECHNOTPLAYING,
	RDERR_INVALIDWAV,
	RDERR_FXALREADYOPEN,
	RDERR_NOFREEBUFFERS,
	RDERR_FXNOTOPEN,
	RDERR_FXFUCKED,
	RDERR_INVALIDID
};

// Mouse button defines

enum {
	RD_LEFTBUTTONDOWN		= 0x01,
	RD_LEFTBUTTONUP			= 0x02,
	RD_RIGHTBUTTONDOWN		= 0x04,
	RD_RIGHTBUTTONUP		= 0x08,
	RD_WHEELUP			= 0x10,
	RD_WHEELDOWN			= 0x20
};

// Sprite defines

enum {
	// This is the low byte part of the sprite type.

	RDSPR_TRANS			= 0x0001,
	RDSPR_BLEND			= 0x0004,
	RDSPR_FLIP			= 0x0008,
	RDSPR_SHADOW			= 0x0010,
	RDSPR_DISPLAYALIGN		= 0x0020,
	RDSPR_NOCOMPRESSION		= 0x0040,
	RDSPR_EDGEBLEND			= 0x0080,	// Unused

	// This is the high byte part of the sprite type, which defines what
	// type of compression is used. Unless RDSPR_NOCOMPRESSION is set.

	RDSPR_RLE16			= 0x0000,
	RDSPR_RLE256			= 0x0100,
	RDSPR_RLE256FAST		= 0x0200
};

// Fading defines

enum {
	RDFADE_NONE,
	RDFADE_UP,
	RDFADE_DOWN,
	RDFADE_BLACK
};

// Mouse defines

enum {
	RDMOUSE_NOFLASH,
	RDMOUSE_FLASH
};

// Menubar defines.

#define RDMENU_TOP			0
#define RDMENU_BOTTOM			1

enum {
	RDMENU_HIDDEN,
	RDMENU_SHOWN,
	RDMENU_OPENING,
	RDMENU_CLOSING
};

#define RDMENU_ICONWIDE			35
#define RDMENU_ICONDEEP			30
#define RDMENU_ICONSTART		24
#define RDMENU_ICONSPACING		5
#define RDMENU_MAXPOCKETS		15
#define RDMENU_MENUDEEP			40

// Sound defines

enum {
	RDSE_SAMPLEFINISHED		= 0,
	RDSE_SAMPLEPLAYING		= 1,
	RDSE_FXTOCLEAR			= 0,		// Unused
	RDSE_FXCACHED			= 1,		// Unused
	RDSE_FXSPOT			= 0,
	RDSE_FXLOOP			= 1,
	RDSE_FXLEADIN			= 2,
	RDSE_FXLEADOUT			= 3,
	RDSE_QUIET			= 1,
	RDSE_SPEAKING			= 0
};

// Palette defines

enum {
	RDPAL_FADE,
	RDPAL_INSTANT
};

// Blitting FX defines

enum {
	RDBLTFX_SPRITEBLEND		= 0x01,
	RDBLTFX_SHADOWBLEND		= 0x02,
	RDBLTFX_EDGEBLEND		= 0x04
};

// Structure definitions

struct MouseEvent {
	uint16 buttons;
};

struct KeyboardEvent {
	uint16 ascii;
	int keycode;
	int modifiers;
};

#if !defined(__GNUC__)
	#pragma START_PACK_STRUCTS
#endif

struct Parallax {
	uint16 w;
	uint16 h;
	uint32 offset[2];	// 2 is arbitrary
} GCC_PACK;

#if !defined(__GNUC__)
	#pragma END_PACK_STRUCTS
#endif

// The SpriteInfo structure is used to tell the driver96 code what attributes
// are linked to a sprite for drawing.  These include position, scaling and
// compression.

struct SpriteInfo {
	int16 x;		// coords for top-left of sprite
	int16 y;
	uint16 w;		// dimensions of sprite (before scaling)
	uint16 h;
	uint16 scale;		// scale at which to draw, given in 256ths ['0' or '256' MEANS DON'T SCALE]
	uint16 scaledWidth;	// new dimensions (we calc these for the mouse area, so may as well pass to you to save time)
	uint16 scaledHeight;	//
	uint16 type;		// mask containing 'RDSPR_' bits specifying compression type, flip, transparency, etc
	uint16 blend;		// holds the blending values.
	uint8 *data;		// pointer to the sprite data
	uint8 *colourTable;	// pointer to 16-byte colour table, only applicable to 16-col compression type
};

// This is the structure which is passed to the sequence player. It includes
// the smack to play, and any text lines which are to be displayed over the top
// of the sequence.

struct MovieTextObject {
	uint16 startFrame;
	uint16 endFrame;
	SpriteInfo *textSprite;
	uint32 speechBufferSize;
	uint16 *speech;
};

// Input handling class

// Mouse buffer size
#define MAX_MOUSE_EVENTS 16

// Key buffer size
#define MAX_KEY_BUFFER 32

class Input {
private:
	Sword2Engine *_vm;

	uint8 _mouseBacklog;
	uint8 _mouseLogPos;
	MouseEvent _mouseLog[MAX_MOUSE_EVENTS];

	void logMouseEvent(uint16 buttons);

	// The number of key presses waiting to be processed.
	uint8 _keyBacklog;

	// Index of the next key to read from the buffer.
	uint8 _keyLogPos;

	// The keyboard buffer
	KeyboardEvent _keyBuffer[MAX_KEY_BUFFER];

	void writeKey(uint16 ascii, int keycode, int modifiers);

public:
	int16 _mouseX;
	int16 _mouseY;

	Input(Sword2Engine *vm) :
		_vm(vm), _mouseBacklog(0), _mouseLogPos(0), _keyBacklog(0),
		_keyLogPos(0), _mouseX(0), _mouseY(0) {};

	void parseEvents(void);

	MouseEvent *mouseEvent(void);
	bool checkForMouseEvents(void);

	bool keyWaiting(void);
	int32 readKey(KeyboardEvent *ev);
};
 
} // End of namespace Sword2

#endif
