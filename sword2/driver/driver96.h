/* Copyright (C) 1994-2003 Revolution Software Ltd
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

#include "common/scummsys.h"
#include "base/engine.h" // for warning()
#include "common/system.h"
#include "common/rect.h"

namespace Sword2 {

//
//	Defines
//	-------
//

// defines specific to windows headers...
#ifndef WIN32

#define FILE_ATTRIBUTE_NORMAL 0x80
#define _MAX_PATH 260

#endif

//Generic error codes
#define RD_OK				0x00000000
#define RDERR_UNKNOWN			0x00000001
#define RDERR_INVALIDPOINTER		0x00000002
#define RDERR_OUTOFMEMORY		0x00000003
#define RDERR_INVALIDFILENAME		0x00000004
#define RDERR_READERROR			0x00000005
#define RDERR_WRITEERROR		0x00000006
#define RDERR_NOEMULATION		0x00000007
#define RDERR_LOCKFAILED		0x00000008

//Drawing error codes
#define RDERR_VIDEOMODE			0x00010000
#define RDERR_COLOURDEPTH		0x00010001
#define RDERR_CANNOTFLIP		0x00010002
#define RDERR_RESTORELAYERS		0x00010003
#define RDERR_DDRAWNOEMULATION		0X00010004
#define RDERR_NOHARDWARE		0x00010005
#define RDERR_ALREADYON			0x00010006
#define RDERR_DECOMPRESSION		0x00010007

//Operating system error codes
#define RDERR_CREATEWINDOW		0x00020000
#define RDERR_APPCLOSED			0x00020001
#define RDERR_GOFULLSCREEN		0x00020002

//Language and version error codes
#define RDERR_OPENVERSIONFILE		0x00030000
#define RDERR_INVALIDVERSION		0x00030001

//Keyboard error codes
#define RDERR_NOKEYWAITING		0x00040000

//Sprite drawing error codes
#define RDERR_NOCLIPPING		0x00050000
#define RDERR_NOTIMPLEMENTED		0x00050001
#define RDERR_UNKNOWNTYPE		0x00050002
#define RDERR_INVALIDSCALING		0x00050003
#define RDERR_SURFACELOST		0x00050004
#define RDERR_NOTCLOSED			0x00050005
#define RDERR_NOTOPEN			0x00050006
#define RDERR_ALREADYCLOSED		0x00050007 // added for _console.cpp by khalek

//Menubar error codes
#define RDERR_INVALIDMENU		0x00060000
#define RDERR_INVALIDPOCKET		0x00060001
#define RDERR_INVALIDCOMMAND		0x00060002

//Palette fading error codes
#define RDERR_FADEINCOMPLETE		0x00070000

//Sound engine error codes
#define RDERR_DSOUNDCREATE		0x00080000
#define RDERR_DSOUNDCOOPERATE		0x00080001
#define RDERR_DSOUNDPBUFFER		0x00080002
#define RDERR_PRIMARYFORMAT		0x00080003
#define RDERR_SPEECHPLAYING		0x00080004
#define RDERR_SPEECHNOTPLAYING		0x00080005
#define RDERR_INVALIDWAV		0x00080006
#define RDERR_CREATESOUNDBUFFER		0x00080007
#define RDERR_LOCKSPEECHBUFFER		0x00080008
#define RDERR_FXALREADYOPEN		0x00080009
#define RDERR_NOFREEBUFFERS		0x0008000A
#define RDERR_FXNOTOPEN			0x0008000B
#define RDERR_FXFUCKED			0x0008000C
#define RDERR_INVALIDID			0x0008000D


// Key codes
#define RDKEY_ESCAPE			27


// Mouse button defines
#define RD_LEFTBUTTONDOWN		0x01
#define RD_LEFTBUTTONUP			0x02
#define RD_RIGHTBUTTONDOWN		0x04
#define RD_RIGHTBUTTONUP		0x08


//Sprite defines
#define RDSPR_TRANS			0x0001
#define RDSPR_BLEND			0x0004
#define RDSPR_FLIP			0x0008
#define RDSPR_SHADOW			0x0010
#define RDSPR_DISPLAYALIGN		0x0020
#define RDSPR_NOCOMPRESSION		0x0040
#define RDSPR_EDGEBLEND			0x0080
//This is the high byte part of the sprite type which defines what type of
//  compression is used, as long as RDSPR_NOCOMPRESSION is not defined.
#define RDSPR_RLE16			0x0000
#define RDSPR_RLE256			0x0100
#define RDSPR_RLE256FAST		0x0200


//Fading defines
#define RDFADE_NONE			0x00
#define RDFADE_UP			0x01
#define RDFADE_DOWN			0x02
#define RDFADE_BLACK			0x03

//Mouse defines
#define RDMOUSE_NOFLASH			0x00
#define RDMOUSE_FLASH			0x01

//Menubar defines.
#define RDMENU_TOP			0x00
#define RDMENU_BOTTOM			0x01

#define RDMENU_HIDDEN			0x00
#define RDMENU_SHOWN			0x01
#define RDMENU_OPENING			0x02
#define RDMENU_CLOSING			0x03

#define RDMENU_ICONWIDE			35
#define RDMENU_ICONDEEP			30
#define RDMENU_ICONSTART		24
#define RDMENU_ICONSPACING		5
#define RDMENU_MAXPOCKETS		15
#define RDMENU_MENUDEEP			40

#define RDSE_SAMPLEFINISHED		0
#define RDSE_SAMPLEPLAYING		1
#define RDSE_FXTOCLEAR			0
#define RDSE_FXCACHED			1
#define RDSE_FXSPOT			0
#define RDSE_FXLOOP			1
#define RDSE_FXLEADIN			2
#define RDSE_FXLEADOUT			3
#define RDSE_QUIET			1
#define RDSE_SPEAKING			0


#define RDPAL_FADE			0
#define RDPAL_INSTANT			1

//Blitting FX defines
#define RDBLTFX_SPRITEBLEND		0x01
#define RDBLTFX_SHADOWBLEND		0x02
#define RDBLTFX_EDGEBLEND		0x04

//
//	Structure definitions
//	---------------------
//

typedef struct {
	uint16 buttons;
} _mouseEvent;

typedef struct {
	uint16 ascii;
	int keycode;
	int modifiers;
} _keyboardEvent;

#if !defined(__GNUC__)
	#pragma START_PACK_STRUCTS
#endif

struct _parallax {
	uint16 w;
	uint16 h;
	uint32 offset[2];	// 2 is arbitrary
} GCC_PACK;

#if !defined(__GNUC__)
	#pragma END_PACK_STRUCTS
#endif


// The _spriteInfo structure is used to tell the driver96 code what attributes
// are linked to a sprite for drawing.  These include position, scaling and
// compression.

typedef struct {
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
} _spriteInfo;


// This is the format of a .WAV file.  Somewhere after this header is the string
// 'DATA' followed by an int32 size which is the size of the data.  Following
// the size of the data is the data itself.
typedef struct {
	uint32 riff;
	uint32 fileLength;
	uint32 wavID;
	uint32 format;
	uint32 formatLen;
	uint16 formatTag;
	uint16 channels;
	uint16 samplesPerSec;
	uint16 avgBytesPerSec;
	uint16 blockAlign;
	uint16 unknown1;
	uint16 unknown2;
	uint16 bitsPerSample;
} _wavHeader;


//  This is the structure which is passed to the sequence player.
//	It includes the smack to play, and any text lines which are
//	to be displayed over the top of the sequence.

typedef struct {
	uint16 startFrame;
	uint16 endFrame;
	_spriteInfo *textSprite;
	uint32 speechBufferSize;
	uint16 *speech;
} _movieTextObject;

//
//	Function Prototypes
//	-------------------
//

//-----------------------------------------------------------------------------
//	Language functions - from language.c
//-----------------------------------------------------------------------------
extern int32 GetLanguageVersion(uint8 *version);
extern int32 SetLanguageVersion(uint8 version);
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//	Misc functions - from misc.cpp
//-----------------------------------------------------------------------------
extern void SVM_SetFileAttributes(char *file, uint32 atrib);
extern void SVM_DeleteFile(char *file);
extern int32 SVM_GetVolumeInformation(char *cdPath, char *sCDName, uint32 maxPath, uint8 *, uint32 *dwMaxCompLength, uint32 *dwFSFlags, uint8 *, uint32 a);

#define MAX_MOUSE_EVENTS 16

// Key buffer size
#define MAX_KEY_BUFFER 32

class Input {
	uint8 _mouseBacklog;
	uint8 _mouseLogPos;
	_mouseEvent _mouseLog[MAX_MOUSE_EVENTS];

	void logMouseEvent(uint16 buttons);

	// The number of key presses waiting to be processed.
	uint8 _keyBacklog;

	// Index of the next key to read from the buffer.
	uint8 _keyLogPos;

	// The keyboard buffer
	_keyboardEvent _keyBuffer[MAX_KEY_BUFFER];

	void writeKey(uint16 ascii, int keycode, int modifiers);

public:
	int16 _mouseX;
	int16 _mouseY;

	Input() :
		_mouseBacklog(0), _mouseLogPos(0), _keyBacklog(0),
		_keyLogPos(0) {};

	void parseEvents(void);

	_mouseEvent *mouseEvent(void);
	bool checkForMouseEvents(void);

	bool keyWaiting(void);
	int32 readKey(_keyboardEvent *ev);
};
 
} // End of namespace Sword2

#endif
