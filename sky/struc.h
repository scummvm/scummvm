/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef SKYSTRUC_H
#define SKYSTRUC_H

namespace Sky {

struct lowTextManager_t {
	byte *textData;
	uint16 compactNum;
};

struct displayText_t {
	byte *textData;	
	uint32 textWidth;
};

#if !defined(__GNUC__)
#pragma START_PACK_STRUCTS
#endif

struct dataFileHeader {
	uint16 flag; // bit 0: set for colour data, clear for not
	// bit 1: set for compressed, clear for uncompressed
	// bit 2: set for 32 colours, clear for 16 colours
	uint16 s_x;
	uint16 s_y;
	uint16 s_width;
	uint16 s_height;
	uint16 s_sp_size;
	uint16 s_tot_size;
	uint16 s_n_sprites;
	int16 s_offset_x;
	int16 s_offset_y;
	uint16 s_compressed_size;
} GCC_PACK;

#if !defined(__GNUC__)
#pragma END_PACK_STRUCTS
#endif

struct GrafixPtr { // replacement for old grafixProg pointer. More savegame compatible.
	uint8 ptrType; // ptr to autoroute / to compact / to turntable
	uint16 ptrTarget; // compact / turntable number
	uint16 pos; // position
};

struct TurnTable {
	uint16 *turnTableUp[5];
	uint16 *turnTableDown[5];
	uint16 *turnTableLeft[5];
	uint16 *turnTableRight[5];
	uint16 *turnTableTalk[5];
};

struct MegaSet {
	uint16 gridWidth;
	uint16 colOffset;
	uint16 colWidth;
	uint16 lastChr;

	uint16 *animUp;
	uint16 *animDown;
	uint16 *animLeft;
	uint16 *animRight;

	uint16 *standUp;
	uint16 *standDown;
	uint16 *standLeft;
	uint16 *standRight;
	uint16 *standTalk;
	TurnTable *turnTable;
};

struct ExtCompact {
	uint16 actionSub;
	uint16 actionSub_off;
	uint16 getToSub;
	uint16 getToSub_off;
	uint16 extraSub;
	uint16 extraSub_off;

	uint16 dir;

	uint16 stopScript;
	uint16 miniBump;
	uint16 leaving;
	uint16 atWatch; // pointer to script variable
	uint16 atWas; // pointer to script variable
	uint16 alt; // alternate script
	uint16 request;

	uint16 spWidth_xx;
	uint16 spColour;
	uint16 spTextId;
	uint16 spTime;

	uint16 arAnimIndex;
	uint16 *turnProg;

	uint16 waitingFor;

	uint16 arTargetX;
	uint16 arTargetY;

	uint16 *animScratch; // data area for AR

	uint16 megaSet;
	MegaSet *megaSet0;
	MegaSet *megaSet1;
	MegaSet *megaSet2;
	MegaSet *megaSet3;
};

struct Compact {
	uint16 logic; // Entry in logic table to run (byte as <256entries in logic table
	uint16 status;
	uint16 sync; // flag sent to compacts by other things

	uint16 screen; // current screen
	uint16 place; // so's this one
	uint16 *getToTable; // Address of how to get to things table

	uint16 xcood;
	uint16 ycood;

	uint16 frame;

	uint16 cursorText;
	uint16 mouseOn;
	uint16 mouseOff;
	uint16 mouseClick; // dword script

	int16 mouseRelX;
	int16 mouseRelY;
	uint16 mouseSizeX;
	uint16 mouseSizeY;

	uint16 actionScript;

	uint16 upFlag; // usually holds the Action Mode
	uint16 downFlag; // used for passing back
	uint16 getToFlag; // used by action script for get to attempts, also frame store (hence word)
	uint16 flag; // a use any time flag

	uint16 mood; // high level - stood or not
	GrafixPtr grafixProg;
	uint16 offset;

	uint16 mode; // which mcode block

	uint16 baseSub; // 1st mcode block relative to start of compact
	uint16 baseSub_off;

	ExtCompact *extCompact;
};

} // End of namespace Sky

#endif
