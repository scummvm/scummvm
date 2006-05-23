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

#include "common/stdafx.h"

#include "agi/agi.h"

namespace Agi {

static int check_step(int delta, int step) {
	return (-step >= delta) ? 0 : (step <= delta) ? 2 : 1;
}

static int check_block(int x, int y) {
	if (x <= game.block.x1 || x >= game.block.x2)
		return false;

	if (y <= game.block.y1 || y >= game.block.y2)
		return false;

	return true;
}

static void changepos(struct vt_entry *v) {
	int b, x, y;
	int dx[9] = { 0, 0, 1, 1, 1, 0, -1, -1, -1 };
	int dy[9] = { 0, -1, -1, 0, 1, 1, 1, 0, -1 };

	x = v->x_pos;
	y = v->y_pos;
	b = check_block(x, y);

	x += v->step_size * dx[v->direction];
	y += v->step_size * dy[v->direction];

	if (check_block(x, y) == b) {
		v->flags &= ~MOTION;
	} else {
		v->flags |= MOTION;
		v->direction = 0;
		if /*_is_ego_view*/ (v)
			game.vars[V_ego_dir] = 0;
	}
}

static void motion_wander(struct vt_entry *v) {
	if (v->parm1--) {
		if (~v->flags & DIDNT_MOVE)
			return;
	}

	v->direction = rnd->getRandomNumber(8);

	if /*_is_ego_view */ (v) {
		game.vars[V_ego_dir] = v->direction;
		while (v->parm1 < 6) {
			v->parm1 = rnd->getRandomNumber(50);	/* huh? */
		}
	}
}

static void motion_followego(struct vt_entry *v) {
	int ego_x, ego_y;
	int obj_x, obj_y;
	int dir;

	ego_x = game.view_table[0].x_pos + game.view_table[0].x_size / 2;
	ego_y = game.view_table[0].y_pos;

	obj_x = v->x_pos + v->x_size / 2;
	obj_y = v->y_pos;

	/* Get direction to reach ego */
	dir = get_direction(obj_x, obj_y, ego_x, ego_y, v->parm1);

	/* Already at ego coordinates */
	if (dir == 0) {
		v->direction = 0;
		v->motion = MOTION_NORMAL;
		setflag(v->parm2, true);
		return;
	}

	if (v->parm3 == 0xff) {
		v->parm3 = 0;
	} else if (v->flags & DIDNT_MOVE) {
		int d;

		while ((v->direction = rnd->getRandomNumber(8)) == 0) {
		}

		d = (abs(ego_y - obj_y) + abs(ego_x - obj_x)) / 2;

		if (d < v->step_size) {
			v->parm3 = v->step_size;
			return;
		}

		while ((v->parm3 = rnd->getRandomNumber(d)) < v->step_size) {
		}
		return;
	}

	if (v->parm3 != 0) {
		int k;

		/* DF: this is ugly and I dont know why this works, but
		 * other line does not! (watcom complained about lvalue)
		 *
		 * if (((int8)v->parm3 -= v->step_size) < 0)
		 *      v->parm3 = 0;
		 */
		k = v->parm3;
		k -= v->step_size;
		v->parm3 = k;

		if ((int8) v->parm3 < 0)
			v->parm3 = 0;
	} else {
		v->direction = dir;
	}
}

static void motion_moveobj(struct vt_entry *v) {
	v->direction = get_direction(v->x_pos, v->y_pos, v->parm1, v->parm2, v->step_size);

	/* Update V6 if ego */
	if (v == game.view_table)
		game.vars[V_ego_dir] = v->direction;

	if (v->direction == 0)
		in_destination(v);
}

static void check_motion(struct vt_entry *v) {
	switch (v->motion) {
	case MOTION_WANDER:
		motion_wander(v);
		break;
	case MOTION_FOLLOW_EGO:
		motion_followego(v);
		break;
	case MOTION_MOVE_OBJ:
		motion_moveobj(v);
		break;
	}

	if ((game.block.active && (~v->flags & IGNORE_BLOCKS)) && v->direction)
		changepos(v);
}

/*
 * Public functions
 */

/**
 *
 */
void check_all_motions() {
	struct vt_entry *v;

	for (v = game.view_table; v < &game.view_table[MAX_VIEWTABLE]; v++) {
		if ((v->flags & (ANIMATED | UPDATE | DRAWN)) == (ANIMATED | UPDATE | DRAWN)
				&& v->step_time_count == 1) {
			check_motion(v);
		}
	}
}

/**
 * Check if given entry is at destination point.
 * This function is used to updated the flags of an object with move.obj
 * type motion that * has reached its final destination coordinates.
 * @param  v  Pointer to view table entry
 */
void in_destination(struct vt_entry *v) {
	if (v->motion == MOTION_MOVE_OBJ) {
		v->step_size = v->parm3;
		setflag(v->parm4, true);
	}
	v->motion = MOTION_NORMAL;
	if (v == game.view_table)
		game.player_control = true;
}

/**
 * Wrapper for static function motion_moveobj().
 * This function is used by cmd_move_object() in the first motion cycle
 * after setting the motion mode to MOTION_MOVE_OBJ.
 * @param  v  Pointer to view table entry
 */
void move_obj(struct vt_entry *v) {
	motion_moveobj(v);
}

/**
 * Get direction from motion coordinates
 * This function gets the motion direction from the current and previous
 * object coordinates and the step size.
 * @param  x0  Original x coordinate of the object
 * @param  y0  Original y coordinate of the object
 * @param  x   x coordinate of the object
 * @param  y   y coordinate of the object
 * @param  s   step size
 */
int get_direction(int x0, int y0, int x, int y, int s) {
	int dir_table[9] = { 8, 1, 2, 7, 0, 3, 6, 5, 4 };
	return dir_table[check_step(x - x0, s) + 3 * check_step(y - y0, s)];
}

}                             // End of namespace Agi
