/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
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

#ifndef SIMON_INTERN_H
#define SIMON_INTERN_H

namespace Simon {

struct Child {
	Child *next;
	uint16 type;
};

struct SubRoom : Child {
	uint16 subroutine_id;
	uint16 roomExitStates;
	uint16 roomExit[1];
};

struct SubObject : Child {
	uint16 objectName;
	uint32 objectFlags;
	int16 objectFlagValue[1];
};

struct SubUserFlag : Child {
	uint16 userFlags[4];
};

enum {
	SubRoom_SIZE = sizeof(SubRoom) - sizeof(uint16),
	SubObject_SIZE = sizeof(SubObject) - sizeof(int16)
};

struct Item {
	uint16 parent;
	uint16 child;
	uint16 sibling;
	int16 noun;
	int16 adjective;
	int16 state;										/* signed int */
	uint16 classFlags;
	Child *children;

	Item() { memset(this, 0, sizeof(*this)); }
};

struct Subroutine {
	uint16 id;								/* subroutine ID */
	uint16 first;								/* offset from subroutine start to first subroutine line */
	Subroutine *next;							/* next subroutine in linked list */
};

struct IconEntry {
	Item *item;
	uint16 boxCode;
};

struct IconBlock {
	int16 line;
	Item *itemRef;
	IconEntry iconArray[64];
	int16 upArrow, downArrow;
	uint16 classMask;
};

struct WindowBlock {
	byte mode;
	byte flags;
	uint16 x, y;
	uint16 width, height;
	uint16 textColumn, textRow;
	uint16 textColumnOffset, textLength, textMaxLength;
	uint16 scrollY;
	uint8 fill_color, text_color;
	IconBlock *iconPtr;
	WindowBlock() { memset(this, 0, sizeof(*this)); }
};
// note on text offset:
// the actual x-coordinate is: textColumn * 8 + textColumnOffset
// the actual y-coordinate is: textRow * 8


enum BoxFlags {
	kBFTextBox        = 0x1,
	kBFBoxSelected    = 0x2,
	kBFNoTouchName    = 0x4,
	kBFInvertTouch    = 0x8,
	kBFDragBox        = 0x10, // Simon 1/2
	kBFHyperBox       = 0x10, // Feeble Files
	kBFBoxInUse       = 0x20,
	kBFBoxDead        = 0x40,
	kBFBoxItem        = 0x80
};

enum SubObjectFlags {
	kOFText           = 0x1,
	kOFSize           = 0x2,
	kOFWeight         = 0x4,
	kOFVolume         = 0x8,
	kOFIcon           = 0x10,
	kOFKeyColor1      = 0x20,
	kOFKeyColor2      = 0x40,
	kOFMenu           = 0x80,
	kOFNumber         = 0x100,
	kOFVoice          = 0x200
};

enum {
	SUBROUTINE_LINE_SMALL_SIZE = 2,
	SUBROUTINE_LINE_BIG_SIZE = 8
};

struct SubroutineLine {
	uint16 next;
	int16 verb;
	int16 noun1;
	int16 noun2;
};

struct TimeEvent {
	uint32 time;
	uint16 subroutine_id;
	TimeEvent *next;
};

struct GameSpecificSettings {
#ifndef PALMOS_68K
	const char *effects_filename;
	const char *speech_filename;
 #else
	const char effects_filename[12];
	const char speech_filename[12];
 #endif
};

} // End of namespace Simon

enum GameFeatures {
	GF_TALKIE     = 1 << 0,
	GF_OLD_BUNDLE = 1 << 1,
	GF_CRUNCHED   = 1 << 2,
	GF_ZLIBCOMP   = 1 << 3
};

enum GameFileTypes {
	GAME_BASEFILE = 1 << 0,
	GAME_ICONFILE = 1 << 1,
	GAME_GMEFILE  = 1 << 2,
	GAME_STRFILE  = 1 << 3,
	GAME_TBLFILE  = 1 << 4,

	GAME_GFXIDXFILE = 1 << 5
};

enum GameIds {
	GID_SIMON1DOS,
	GID_SIMON1DOS_RU,
	GID_SIMON1DOS_INF,
	GID_SIMON1DOS_INF_RU,
	GID_SIMON1DOS_DE,
	GID_SIMON1DOS_FR,
	GID_SIMON1DOS_IT,
	GID_SIMON1DOS_ES,
	GID_SIMON1DEMO,
	GID_SIMON1AMIGA,
	GID_SIMON1AMIGA_FR,
	GID_SIMON1AMIGA_DE,
	GID_SIMON1AMIGADEMO,
	GID_SIMON1CD32,
	GID_SIMON1CD32_2,
	GID_SIMON1ACORN,
	GID_SIMON1ACORNDEMO,
	GID_SIMON1TALKIE,
	GID_SIMON1TALKIE2,
	GID_SIMON1TALKIE_DE,
	GID_SIMON1TALKIE_FR,
	GID_SIMON1TALKIE_HB,
	GID_SIMON1TALKIE_IT,
	GID_SIMON1TALKIE_ES,
	GID_SIMON1WIN,
	GID_SIMON1WIN_DE,

	GID_SIMON2DOS,
	GID_SIMON2DOS_RU,
	GID_SIMON2DOS2,
	GID_SIMON2DOS2_RU,
	GID_SIMON2DOS_DE,
	GID_SIMON2DOS_IT,
	GID_SIMON2DEMO,
	GID_SIMON2TALKIE,
	GID_SIMON2TALKIE2,
	GID_SIMON2TALKIE_DE,
	GID_SIMON2TALKIE_DE2,
	GID_SIMON2TALKIE_FR,
	GID_SIMON2TALKIE_HB,
	GID_SIMON2TALKIE_IT,
	GID_SIMON2TALKIE_ES,
	GID_SIMON2WIN,
	GID_SIMON2WIN_DE,
	GID_SIMON2WIN_DE2,
	GID_SIMON2WIN_PL,

	GID_FEEBLEFILES_AMI_DE,
	GID_FEEBLEFILES_AMI_UK,
	GID_FEEBLEFILES_MAC_DE,
	GID_FEEBLEFILES_MAC_FR,
	GID_FEEBLEFILES_MAC_SP,
	GID_FEEBLEFILES_MAC_UK,
	GID_FEEBLEFILES_2CD,
	GID_FEEBLEFILES_4CD,
	GID_FEEBLEFILES_DE
};

#endif
