/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2005 The ScummVM project
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
 * $Header$
 *
 */

#ifndef SIMON_INTERN_H
#define SIMON_INTERN_H

namespace Simon {

struct Child {
	Child *next;
	uint16 type;
};

struct Child1 : Child {
	uint16 subroutine_id;
	uint16 fr2;
	uint16 array[1];
};

struct Child2 : Child {
	uint16 string_id;
	uint32 avail_props;
	int16 array[1];
};

struct Child9 : Child {
	uint16 array[4];
};

enum {
	CHILD1_SIZE = sizeof(Child1) - sizeof(uint16),
	CHILD2_SIZE = sizeof(Child2) - sizeof(int16)
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

struct FillOrCopyDataEntry {
	Item *item;
	uint16 hit_area;
	uint16 xxx_1;
};

struct FillOrCopyData {
	int16 unk1;
	Item *item_ptr;
	FillOrCopyDataEntry e[64];
	int16 upArrow, downArrow;
	uint16 unk2;
};

struct FillOrCopyStruct {
	byte mode;
	byte flags;
	uint16 x, y;
	uint16 width, height;
	uint16 textColumn, textRow;
	uint8 textColumnOffset, textLength, textMaxLength;
    uint8 fill_color, text_color, unk5;
	FillOrCopyData *fcs_data;
	FillOrCopyStruct() { memset(this, 0, sizeof(*this)); }
};
// note on text offset:
// the actual x-coordinate is: textColumn * 8 + textColumnOffset
// the actual y-coordinate is: textRow * 8


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
	const char *gme_filename;
	const char *wav_filename;
	const char *voc_filename;
	const char *mp3_filename;
	const char *vorbis_filename;
	const char *flac_filename;
	const char *voc_effects_filename;
	const char *mp3_effects_filename;
	const char *vorbis_effects_filename;
	const char *flac_effects_filename;
 	const char *gamepc_filename;
 #else
	const char gme_filename[12];
	const char wav_filename[12];
	const char voc_filename[12];
	const char mp3_filename[12];
	const char flac_filename[12];
	const char vorbis_filename[12];
	const char voc_effects_filename[12];
	const char mp3_effects_filename[12];
	const char vorbis_effects_filename[12];
	const char flac_effects_filename[12];
 	const char gamepc_filename[12];
 #endif
};

} // End of namespace Simon

enum GameFeatures {
	GF_TALKIE     = 1 << 0,
	GF_OLD_BUNDLE = 1 << 1,
	GF_CRUNCHED   = 1 << 2
};

enum GameFileTypes {
	GAME_BASEFILE = 1 << 0,
	GAME_ICONFILE = 1 << 1,
	GAME_GMEFILE  = 1 << 2,
	GAME_STRFILE  = 1 << 3,
	GAME_TBLFILE  = 1 << 4
};

enum GameIds {
	GID_SIMON1DOS,
	GID_SIMON1DOS_RU,
	GID_SIMON1DEMO,
	GID_SIMON1AMIGA,
	GID_SIMON1CD32,
	GID_SIMON1ACORN,
	GID_SIMON1ACORNDEMO,
	GID_SIMON1TALKIE,
	GID_SIMON1TALKIE_DE,
	GID_SIMON1TALKIE_FR,
	GID_SIMON1TALKIE_HB,
	GID_SIMON1TALKIE_IT,
	GID_SIMON1TALKIE_ES,
	GID_SIMON1WIN,
	GID_SIMON1WIN_DE,

	GID_SIMON2DOS,
	GID_SIMON2DOS_RU,
	GID_SIMON2DEMO,
	GID_SIMON2TALKIE,
	GID_SIMON2TALKIE_DE,
	GID_SIMON2TALKIE_FR,
	GID_SIMON2TALKIE_HB,
	GID_SIMON2TALKIE_IT,
	GID_SIMON2TALKIE_ES,
	GID_SIMON2WIN,
	GID_SIMON2WIN_DE,
	GID_SIMON2WIN_PL,

	GID_FEEBLEFILES_2CD,
	GID_FEEBLEFILES_4CD,
	GID_FEEBLEFILES_DE
};

#endif
