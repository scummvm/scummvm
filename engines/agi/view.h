/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2001 Sarien Team
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

#ifndef __AGI_VIEW_H
#define __AGI_VIEW_H

namespace Agi {

struct view_cel {
	uint8 height;
	uint8 width;
	uint8 transparency;
	uint8 mirror_loop;
	uint8 mirror;
	uint8 *data;
};

struct view_loop {
	int num_cels;
	struct view_cel *cel;
};

/**
 * AGI view resource structure.
 */
struct agi_view {
	int num_loops;
	struct view_loop *loop;
	char *descr;
	uint8 *rdata;
};

/**
 * AGI view table entry
 */
struct vt_entry {
	uint8 step_time;
	uint8 step_time_count;
	uint8 entry;
	int16 x_pos;
	int16 y_pos;
	uint8 current_view;
	struct agi_view *view_data;
	uint8 current_loop;
	uint8 num_loops;
	struct view_loop *loop_data;
	uint8 current_cel;
	uint8 num_cels;
	struct view_cel *cel_data;
	struct view_cel *cel_data_2;
	int16 x_pos2;
	int16 y_pos2;
	void *s;
	int16 x_size;
	int16 y_size;
	uint8 step_size;
	uint8 cycle_time;
	uint8 cycle_time_count;
	uint8 direction;

#define MOTION_NORMAL		0
#define MOTION_WANDER		1
#define	MOTION_FOLLOW_EGO	2
#define	MOTION_MOVE_OBJ		3
	uint8 motion;

#define	CYCLE_NORMAL		0
#define CYCLE_END_OF_LOOP	1
#define	CYCLE_REV_LOOP 		2
#define	CYCLE_REVERSE		3
	uint8 cycle;

	uint8 priority;

#define DRAWN		0x0001
#define IGNORE_BLOCKS	0x0002
#define FIXED_PRIORITY	0x0004
#define IGNORE_HORIZON	0x0008
#define UPDATE		0x0010
#define CYCLING		0x0020
#define ANIMATED	0x0040
#define MOTION		0x0080
#define ON_WATER	0x0100
#define IGNORE_OBJECTS	0x0200
#define UPDATE_POS	0x0400
#define ON_LAND		0x0800
#define DONTUPDATE	0x1000
#define FIX_LOOP	0x2000
#define DIDNT_MOVE	0x4000
#define	ADJ_EGO_XY	0x8000
	uint16 flags;

	uint8 parm1;
	uint8 parm2;
	uint8 parm3;
	uint8 parm4;
};				/* struct vt_entry */

/* Motion */
void check_all_motions(void);
void move_obj(struct vt_entry *);
void in_destination(struct vt_entry *);
void fix_position(int);
void update_position(void);

/* View table management */
void set_cel(struct vt_entry *, int);
void set_loop(struct vt_entry *, int);
void set_view(struct vt_entry *, int);
void start_update(struct vt_entry *);
void stop_update(struct vt_entry *);
void update_viewtable(void);

void unload_view(int);
int decode_view(int);
void add_to_pic(int, int, int, int, int, int, int);
void draw_obj(int);

}                             // End of namespace Agi

#endif				/* __AGI_VIEW_H */
