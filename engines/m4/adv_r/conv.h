
/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef M4_ADV_R_CONV_H
#define M4_ADV_R_CONV_H

#include "common/array.h"
#include "m4/m4_types.h"

namespace M4 {

#define _GC(X) _G(conversations).X

#define CONV_WAIT_FOR_INPUT		1
#define CONV_HALT_FOREVER		2
#define CONV_HALT				3
#define CONV_INPUT_OK			4
#define CONV_DO_NOTHING			5

#define CONV_PLAYER_TALKING		1
#define CONV_NON_PLAYER_TALKING	0

#define DLG_FLUSH_LEFT			-1
#define DLG_FLUSH_RIGHT			-2
#define DLG_FLUSH_TOP			-3
#define DLG_FLUSH_BOTTOM		-4
#define DLG_CENTER_H			-5
#define DLG_CENTER_V			-6


//from: prochunk.h
#define	C_ASGN_CHUNK				((int32) ('C' << 24) | ('A' << 16) | ('S' << 8) | 'N')

#define	ASGN_CHUNK					((int32) ('A' << 24) | ('S' << 16) | ('G' << 8) | 'N')

#define	HIDE_CHUNK					((int32) ('H' << 24) | ('I' << 16) | ('D' << 8) | 'E')
#define	UHID_CHUNK					((int32) ('U' << 24) | ('H' << 16) | ('I' << 8) | 'D')
#define	DSTR_CHUNK					((int32) ('D' << 24) | ('S' << 16) | ('T' << 8) | 'R')
#define	CHDE_CHUNK					((int32) ('C' << 24) | ('H' << 16) | ('D' << 8) | 'E')
#define	CUHD_CHUNK					((int32) ('C' << 24) | ('U' << 16) | ('H' << 8) | 'D')
#define	CDST_CHUNK					((int32) ('D' << 24) | ('D' << 16) | ('T' << 8) | 'S')

#define	CONV_CHUNK					((int32) ('C' << 24) | ('O' << 16) | ('N' << 8) | 'V')
#define	DECL_CHUNK					((int32) ('D' << 24) | ('E' << 16) | ('C' << 8) | 'L')

#define	FALL_CHUNK					((int32) ('F' << 24) | ('A' << 16) | ('L' << 8) | 'L')
#define	LNODE_CHUNK					((int32) ('L' << 24) | ('N' << 16) | ('O' << 8) | 'D')
#define	NODE_CHUNK					((int32) ('N' << 24) | ('O' << 16) | ('D' << 8) | 'E')
#define	ENTRY_CHUNK					((int32) ('E' << 24) | ('T' << 16) | ('R' << 8) | 'Y')
#define	TEXT_CHUNK					((int32) ('T' << 24) | ('E' << 16) | ('X' << 8) | 'T')

//reply
#define	REPLY_CHUNK					((int32) ('R' << 24) | ('P' << 16) | ('L' << 8) | 'Y')
#define	WEIGHT_REPLY_CHUNK			((int32) ('W' << 24) | ('R' << 16) | ('P' << 8) | 'L')
#define	WEIGHT_PREPLY_CHUNK			((int32) ('W' << 24) | ('P' << 16) | ('R' << 8) | 'L')
#define	COND_REPLY_CHUNK			((int32) ('C' << 24) | ('R' << 16) | ('P' << 8) | 'L')

#define	MESSAGE_CHUNK				((int32) ('M' << 24) | ('E' << 16) | ('S' << 8) | 'G')

// goto
#define	GOTO_CHUNK					((int32) ('G' << 24) | ('O' << 16) | ('T' << 8) | 'O')
#define	EXIT_GOTO_CHUNK				((int32) ('E' << 24) | ('X' << 16) | ('I' << 8) | 'T')
#define	COND_GOTO_CHUNK				((int32) ('C' << 24) | ('C' << 16) | ('G' << 8) | 'O')

#define	COND_EXIT_GOTO_CHUNK		((int32) ('C' << 24) | ('E' << 16) | ('G' << 8) | 'O')


struct Conv {
	int32 chunkSize = 0;
	char *conv = nullptr;
	int32 myCNode = 0;
	int32 exit_now = 0;
	int32 node_hash = 0;

	int32 mode = 0;
	int32 c_entry_num = 0;
	Common::Array<int32 *> _pointers;
};

struct ConvDisplayData {
	char *text[16];
	char *snd_files[16];
	char mesg[1024];
	char *mesg_snd_file;
	int	num_txt_ents;
	int	player_non_player;
	int	player_choice;
};

#include "common/pack-start.h"	// START STRUCT PACKING

struct conv_chunk {
	int32 tag;
	int32 size;
} PACKED_STRUCT;

struct decl_chunk {
	int32 tag;
	int32 val;
	int32 flags;
	int32 addrIndex;	// Index into Conv::_pointers array
} PACKED_STRUCT;

struct fall_chunk {
	int32 tag;
	int32 val;
	int32 index;
} PACKED_STRUCT;

struct node_chunk {
	int32 tag;
	int32 hash;
	int32 size;
	int32 num_entries;
} PACKED_STRUCT;

struct lnode_chunk {
	int32 tag;
	int32 hash;
	int32 size;
	int32 entry_num;
	int32 num_entries;
} PACKED_STRUCT;

struct entry_chunk {
	int32 tag;
	int32 size;
	int32 status;
} PACKED_STRUCT;

struct text_chunk {
	int32 tag;
	int32 size;
} PACKED_STRUCT;

struct mesg_chunk {
	int32 tag;
	int32 size;
} PACKED_STRUCT;

struct reply_chunk {
	int32 tag;
	int32 index;		// Where the message is located.
} PACKED_STRUCT;

struct c_reply_chunk {
	int32 tag;
	int32 op_l;
	int32 op;
	int32 op_r;
	int32 index;		// Where the message is located.
} PACKED_STRUCT;

struct w_reply_chunk {
	int32 tag;
	int32 num_replies;
} PACKED_STRUCT;

struct w_entry_chunk {
	int32 weight;
	int32 index;		// Where the message is located.
} PACKED_STRUCT;

struct goto_chunk {
	int32 tag;
	int32 index;		// Where the node is located.
} PACKED_STRUCT;

struct c_goto_chunk {
	int32 tag;
	int32 opnd1;		// Where the decl is located.
	int32 op;
	int32 opnd2;		// Integer value.
	int32 index;		// Where the node is located.
} PACKED_STRUCT;

struct misc_chunk {
	int32 tag;
	int32 index;		// Where the entry is located.
} PACKED_STRUCT;

struct c_misc_chunk {
	int32 tag;

	int32 c_op_l;	// Where the decl is located.
	int32 c_op;
	int32 c_op_r;	// Integer value.

	int32 index;		// Where the entry is located.
} PACKED_STRUCT;

struct assign_chunk {
	int32 tag;
	int32 index;		// Where the decl is located.
	int32 op;
	int32 opnd1;		// Integer value.
} PACKED_STRUCT;

struct c_assign_chunk {
	int32 tag;

	int32 c_op_l;	// Where the decl is located.
	int32 c_op;
	int32 c_op_r;	// Integer value.

	int32 index;		// Where the decl is located.
	int32 op;
	int32 opnd1;		// Integer value.
} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING

/**
  "xxxxxxxx"  means the size you have calculated a conversation box to be,
   x      x   given the sentences that are in it, and whatever border
   x      x   space you may have in it.  The boxes in Figure A may be
   xxxxxxxx   where the user wants to place them.  Obviously, the user
			  won't try to put them out in hyperspace, but may do this
			  if he wants the box to be flush with a corner (to grow
			  up, down, left, or right.)   Figure B is the re-calculated
			  coordinates of the boxes in order to get them onto the
			  background.  The new boxes should not be in the interface
			  or in the letterboxed areas at the top and bottom.




		   xxxxxxxxx
		   x   (d) x
		   x       x             Figure A                       xxxxxxxxxxxx
   0,0     xxxxxxxxx                                            x      (b) x
																x          x
																x          x
							 (letterbox at top)                 xxxxxxxxxxxx




							 (background)









														xxxxxxxxxxxxx
														x     (c)   x
														x           x
														x           x
														x           x
														x           x
 xxxxxxxxxxxxxxxxxx                                     x           x
 x                x                                     xxxxxxxxxxxxx
 x                x         (interface)
 x                x
 x                x
 x                x
 x                x         (letterbox at bottom)
 x                x
 x                x
 x (a)            x                                                  640,479
 xxxxxxxxxxxxxxxxxx






								 Figure B
   0,0






		   xxxxxxxx                                       xxxxxxxxxxx
		   x  (d) x                                       x     (b) x
		   x      x                                       x         x
		   xxxxxxxx                                       x         x
														  xxxxxxxxxxx
		 xxxxxxxxxxxxxxxxxx
		 x                x
		 x                x
		 x                x                             xxxxxxxxxxxxx
		 x                x                             x     (c)   x
		 x                x                             x           x
		 x                x                             x           x
		 x                x                             x           x
		 x (a)            x                             x           x
		 xxxxxxxxxxxxxxxxxx                             xxxxxxxxxxxxx











																	 640,479




  If someone says to draw conversation box (a) at the location in Figure 1,
  then have it automatically re-position itself into where it is in Figure 2.
  The extra space around the newly re-positioned box should be about 10 pixels
  wide and/or tall.  Make the spacing visually look identical.  In other
  words, if the height of the border is 10 pixels, the width on the side of
  the new conversation box may need to be 15.  You may have to experiment
  with this.  In even other words, you should correct for the aspect ration.

  The same thing should work for boxes (b), (c), and (d).
*/
void set_dlg_rect();

void conv_go(Conv *c);

} // End of namespace M4

#endif
