
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

#include "m4/m4_types.h"

namespace M4 {

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
#define	C_ASGN_CHUNK				((long) ('C' << 24) | ('A' << 16) | ('S' << 8) | 'N')

#define	ASGN_CHUNK					((long) ('A' << 24) | ('S' << 16) | ('G' << 8) | 'N')

#define	HIDE_CHUNK					((long) ('H' << 24) | ('I' << 16) | ('D' << 8) | 'E')
#define	UHID_CHUNK					((long) ('U' << 24) | ('H' << 16) | ('I' << 8) | 'D')
#define	DSTR_CHUNK					((long) ('D' << 24) | ('S' << 16) | ('T' << 8) | 'R')
#define	CHDE_CHUNK					((long) ('C' << 24) | ('H' << 16) | ('D' << 8) | 'E')
#define	CUHD_CHUNK					((long) ('C' << 24) | ('U' << 16) | ('H' << 8) | 'D')
#define	CDST_CHUNK					((long) ('D' << 24) | ('D' << 16) | ('T' << 8) | 'S')

#define	CONV_CHUNK					((long) ('C' << 24) | ('O' << 16) | ('N' << 8) | 'V')
#define	DECL_CHUNK					((long) ('D' << 24) | ('E' << 16) | ('C' << 8) | 'L')

//mar22
#define	FALL_CHUNK					((long) ('F' << 24) | ('A' << 16) | ('L' << 8) | 'L')
//mar15
#define	LNODE_CHUNK					((long) ('L' << 24) | ('N' << 16) | ('O' << 8) | 'D')
#define	NODE_CHUNK					((long) ('N' << 24) | ('O' << 16) | ('D' << 8) | 'E')
#define	ENTRY_CHUNK					((long) ('E' << 24) | ('T' << 16) | ('R' << 8) | 'Y')
#define	TEXT_CHUNK					((long) ('T' << 24) | ('E' << 16) | ('X' << 8) | 'T')

//reply
#define	REPLY_CHUNK					((long) ('R' << 24) | ('P' << 16) | ('L' << 8) | 'Y')
#define	WEIGHT_REPLY_CHUNK			((long) ('W' << 24) | ('R' << 16) | ('P' << 8) | 'L')
#define	WEIGHT_PREPLY_CHUNK			((long) ('W' << 24) | ('P' << 16) | ('R' << 8) | 'L') //jun27
#define	COND_REPLY_CHUNK			((long) ('C' << 24) | ('R' << 16) | ('P' << 8) | 'L')

#define	MESSAGE_CHUNK				((long) ('M' << 24) | ('E' << 16) | ('S' << 8) | 'G')

// goto
#define	GOTO_CHUNK					((long) ('G' << 24) | ('O' << 16) | ('T' << 8) | 'O')
#define	EXIT_GOTO_CHUNK				((long) ('E' << 24) | ('X' << 16) | ('I' << 8) | 'T')
#define	COND_GOTO_CHUNK				((long) ('C' << 24) | ('C' << 16) | ('G' << 8) | 'O')

#define	COND_EXIT_GOTO_CHUNK		((long) ('C' << 24) | ('E' << 16) | ('G' << 8) | 'O')

// from: chunkhed.h
struct Conv {
	long chunkSize;
	char *conv;
	long myCNode;
	long exit_now;
	long node_hash;

	long mode;
	long c_entry_num;
};

struct ConvDisplayData {
	char *text[16];
	char *snd_files[16];
	char	mesg[1024];
	char *mesg_snd_file;
	int	num_txt_ents;
	int	player_non_player;
	int	player_choice;
};

struct conv_chunk {
	long tag;
	long size;
};

struct decl_chunk {
	long tag;
	long val;
	long flags;
	long *addr;
};

struct fall_chunk {
	long tag;
	long val;
	long index;
};

struct node_chunk {
	long tag;
	long hash;
	long size;
	long num_entries;
};

struct lnode_chunk {
	long tag;
	long hash;
	long size;
	long entry_num;
	long num_entries;
};

struct entry_chunk {
	long tag;
	long size;
	long status;
};

struct text_chunk {
	long tag;
	long size;
};

struct mesg_chunk {
	long tag;
	long size;
};

struct reply_chunk {
	long tag;
	long index;		// Where the message is located.
};

struct c_reply_chunk {
	long tag;
	long op_l;
	long op;
	long op_r;
	long index;		// Where the message is located.
};

struct w_reply_chunk {
	long tag;
	long num_replies;
};

struct w_entry_chunk {
	long weight;
	long index;		// Where the message is located.
};

struct goto_chunk {
	long tag;
	long index;		// Where the node is located.
};

struct c_goto_chunk {
	long tag;
	long opnd1;		// Where the decl is located.
	long op;
	long opnd2;		// Integer value.
	long index;		// Where the node is located.
};

struct misc_chunk {
	long tag;
	long index;		// Where the entry is located.
};

struct c_misc_chunk {
	long tag;

	long c_op_l;	// Where the decl is located.
	long c_op;
	long c_op_r;	// Integer value.

	long index;		// Where the entry is located.
};

struct assign_chunk {
	long tag;
	long index;		// Where the decl is located.
	long op;
	long opnd1;		// Integer value.
};

struct c_assign_chunk {
	long tag;

	long c_op_l;	// Where the decl is located.
	long c_op;
	long c_op_r;	// Integer value.

	long index;		// Where the decl is located.
	long op;
	long opnd1;		// Integer value.
};

extern Conv *conv_load(char *filename, int x1, int y1, int32 myTrigger, bool want_box = true);

extern void conv_unload(Conv *c);
extern void conv_shutdown();

extern Conv *conv_get_handle();
extern void conv_set_handle(Conv *c);

extern void conv_resume(Conv *c);

extern void set_conv_name(char *s);
extern char *get_conv_name();
extern char *conv_sound_to_play();
extern int32 conv_whos_talking();

extern void conv_snap_on_hotspots();
extern void conv_snap_off_hotspots();
extern void conv_init_hotspots();

extern long conv_current_node();
extern long conv_current_entry();

//from: chunkops.cpp
extern long get_dechunk_type(char *s, long cSize);

extern conv_chunk *get_conv(Conv *c, long cSize);
extern decl_chunk *get_decl(Conv *c, long cSize);
extern node_chunk *get_node(Conv *c, long cSize);
extern fall_chunk *get_fall(Conv *c, long cSize);
extern lnode_chunk *get_lnode(Conv *c, long cSize);
extern entry_chunk *get_entry(Conv *c, long cSize);
extern entry_chunk *get_hash_entry(Conv *c, long cSize);

extern text_chunk *get_text(Conv *c, long cSize);
extern mesg_chunk *get_mesg(Conv *c, long cSize);
extern reply_chunk *get_reply(Conv *c, long cSize);
extern c_reply_chunk *get_c_reply(Conv *c, long cSize);
extern goto_chunk *get_goto(Conv *c, long cSize);
extern c_goto_chunk *get_c_goto(Conv *c, long cSize);
extern c_assign_chunk *get_c_asgn(Conv *c, long cSize);
extern w_reply_chunk *get_w_reply(Conv *c, long cSize);
extern w_entry_chunk *get_w_entry(Conv *c, long cSize);
extern misc_chunk *get_misc(Conv *c, long cSize);
extern c_misc_chunk *get_c_misc(Conv *c, long cSize);
extern assign_chunk *get_asgn(Conv *c, long cSize);
extern long get_long(Conv *c, long cSize);
extern char *get_string(Conv *c, long cSize);

extern char *conv_ops_get_entry(long i, long *next, long *tag, Conv *c);

extern int conv_ops_cond_successful(long l_op, long op, long r_op);
extern long conv_ops_process_asgn(long val, long oprtr, long opnd);
extern void conv_ops_unknown_chunk(long tag, char *s);
extern long conv_ops_text_strlen(char *s);

extern long conv_get_decl_val(decl_chunk *decl);
extern void conv_set_decl_val(decl_chunk *decl, long val);
extern void conv_export_value(Conv *c, long val, int index);
extern void conv_export_pointer(Conv *c, long *val, int index);

extern void conv_set_font_spacing(int32 h, int32 v);
extern void conv_set_text_colour(int32 norm_colour, int32 hi_colour);

extern void conv_set_text_colours(int32 norm_colour, int32 norm_colour_alt1, int32 norm_colour_alt2,
	int32 hi_colour, int32 hi_colour_alt1, int32 hi_colour_alt2);

extern void conv_set_shading(int32 shade);
extern void conv_set_box_xy(int32 x, int32 y);
extern void conv_get_dlg_coords(int32 *x1, int32 *y1, int32 *x2, int32 *y2);
extern void conv_set_dlg_coords(int32 x1, int32 y1, int32 x2, int32 y2);
extern void conv_set_default_text_colour(int32 norm_colour, int32 hi_colour);
extern void conv_set_default_hv(int32 h, int32 v);

extern int conv_get_event();
extern void conv_set_event(int e);
extern int conv_is_event_ready();

extern void conv_swap_words(Conv *c);

} // End of namespace M4

#endif
