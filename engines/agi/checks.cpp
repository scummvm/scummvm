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

static int check_position(struct vt_entry *v) {
	debugC(4, kDebugLevelSprites, "check position @ %d, %d", v->x_pos, v->y_pos);

	if (v->x_pos < 0 ||
			v->x_pos + v->x_size > _WIDTH ||
			v->y_pos - v->y_size + 1 < 0 ||
			v->y_pos >= _HEIGHT ||
			((~v->flags & IGNORE_HORIZON) && v->y_pos <= game.horizon)) {
		debugC(4, kDebugLevelSprites, "check position failed: x=%d, y=%d, h=%d, w=%d",
				v->x_pos, v->y_pos, v->x_size, v->y_size);
		return 0;
	}

	/* MH1 needs this, but it breaks LSL1 */
	if (agi_get_release() >= 0x3000) {
		if (v->y_pos < v->y_size)
			return 0;
	}

	return 1;
}

/**
 * Check if there's another object on the way
 */
static int check_collision(struct vt_entry *v) {
	struct vt_entry *u;

	if (v->flags & IGNORE_OBJECTS)
		return 0;

	for (u = game.view_table; u < &game.view_table[MAX_VIEWTABLE]; u++) {
		if ((u->flags & (ANIMATED | DRAWN)) != (ANIMATED | DRAWN))
			continue;

		if (u->flags & IGNORE_OBJECTS)
			continue;

		/* Same object, check next */
		if (v->entry == u->entry)
			continue;

		/* No horizontal overlap, check next */
		if (v->x_pos + v->x_size < u->x_pos ||
				v->x_pos > u->x_pos + u->x_size)
			continue;

		/* Same y, return error! */
		if (v->y_pos == u->y_pos)
			goto return_1;

		/* Crossed the baseline, return error! */
		if ((v->y_pos > u->y_pos && v->y_pos2 < u->y_pos2) ||
				(v->y_pos < u->y_pos && v->y_pos2 > u->y_pos2)) {
			goto return_1;
		}
	}

	return 0;

 return_1:
	debugC(4, kDebugLevelSprites, "check returns 1 (object %d)", v->entry);
	return 1;
}

static int check_priority(struct vt_entry *v) {
	int i, trigger, water, pass, pri;
	uint8 *p0;

	if (~v->flags & FIXED_PRIORITY) {
		/* Priority bands */
		v->priority = game.pri_table[v->y_pos];
	}

	trigger = 0;
	water = 0;
	pass = 1;

	if (v->priority == 0x0f)
		goto _check_ego;

	water = 1;

	p0 = &game.sbuf[v->x_pos + v->y_pos * _WIDTH];

	for (i = 0; i < v->x_size; i++, p0++) {
		pri = *p0 >> 4;

		if (pri == 0) {	/* unconditional black. no go at all! */
			pass = 0;
			break;
		}

		if (pri == 3)	/* water surface */
			continue;

		water = 0;

		if (pri == 1) {	/* conditional blue */
			if (v->flags & IGNORE_BLOCKS)
				continue;

			debugC(4, kDebugLevelSprites, "Blocks observed!");
			pass = 0;
			break;
		}

		if (pri == 2) {	/* trigger */
			debugC(4, kDebugLevelSprites, "stepped on trigger");
#ifdef USE_CONSOLE
			if (!debug_.ignoretriggers)
#endif
				trigger = 1;
		}
	}

	if (pass) {
		if (!water && v->flags & ON_WATER)
			pass = 0;
		if (water && v->flags & ON_LAND)
			pass = 0;
	}

      _check_ego:
	if (v->entry == 0) {
		setflag(F_ego_touched_p2, trigger ? true : false);
		setflag(F_ego_water, water ? true : false);
	}

	return pass;
}

/*
 * Public functions
 */

/**
 * Update position of objects
 * This function updates the position of all animated, updating view
 * table entries according to its motion type, step size, etc. The
 * new position must be valid according to the sprite positioning
 * rules, otherwise the previous position will be kept.
 */
void update_position() {
	struct vt_entry *v;
	int x, y, old_x, old_y, border;

	game.vars[V_border_code] = 0;
	game.vars[V_border_touch_ego] = 0;
	game.vars[V_border_touch_obj] = 0;

	for (v = game.view_table; v < &game.view_table[MAX_VIEWTABLE]; v++) {
		if ((v->flags & (ANIMATED | UPDATE | DRAWN)) != (ANIMATED | UPDATE | DRAWN)) {
			continue;
		}

		if (v->step_time_count != 0) {
			if (--v->step_time_count != 0)
				continue;
		}

		v->step_time_count = v->step_time;

		x = old_x = v->x_pos;
		y = old_y = v->y_pos;

		/* If object has moved, update its position */
		if (~v->flags & UPDATE_POS) {
			int dx[9] = { 0, 0, 1, 1, 1, 0, -1, -1, -1 };
			int dy[9] = { 0, -1, -1, 0, 1, 1, 1, 0, -1 };
			x += v->step_size * dx[v->direction];
			y += v->step_size * dy[v->direction];
		}

		/* Now check if it touched the borders */
		border = 0;

		/* Check left/right borders */
		if (x < 0) {
			x = 0;
			border = 4;
		} else if (x <= 0 && agi_get_release() == 0x3086) {	/* KQ4 */
			x = 0;	/* See bug #590462 */
			border = 4;
		} else if (v->entry == 0 && x == 0 && v->flags & ADJ_EGO_XY) {
			/* Extra test to walk west clicking the mouse */
			x = 0;
			border = 4;
		} else if (x + v->x_size > _WIDTH) {
			x = _WIDTH - v->x_size;
			border = 2;
		}

		/* Check top/bottom borders. */
		if (y - v->y_size + 1 < 0) {
			y = v->y_size - 1;
			border = 1;
		} else if (y > _HEIGHT - 1) {
			y = _HEIGHT - 1;
			border = 3;
		} else if ((~v->flags & IGNORE_HORIZON) && y <= game.horizon) {
			debugC(4, kDebugLevelSprites, "y = %d, horizon = %d", y, game.horizon);
			y = game.horizon + 1;
			border = 1;
		}

		/* Test new position. rollback if test fails */
		v->x_pos = x;
		v->y_pos = y;
		if (check_collision(v) || !check_priority(v)) {
			v->x_pos = old_x;
			v->y_pos = old_y;
			border = 0;
			fix_position(v->entry);
		}

		if (border != 0) {
			if (v == game.view_table) {
				game.vars[V_border_touch_ego] = border;
			} else {
				game.vars[V_border_code] = v->entry;
				game.vars[V_border_touch_obj] = border;
			}
			if (v->motion == MOTION_MOVE_OBJ) {
				in_destination(v);
			}
		}

		v->flags &= ~UPDATE_POS;
	}
}

/**
 * Adjust position of a sprite
 * This function adjusts the position of a sprite moving it until
 * certain criteria is matched. According to priority and control line
 * data, a sprite may not always appear at the location we specified.
 * This behaviour is also known as the "Budin-Sonneveld effect".
 *
 * @param n view table entry number
 */
void fix_position(int n) {
	struct vt_entry *v = &game.view_table[n];
	int count, dir, size;

	debugC(4, kDebugLevelSprites, "adjusting view table entry #%d (%d,%d)", n, v->x_pos, v->y_pos);

	/* test horizon */
	if ((~v->flags & IGNORE_HORIZON) && v->y_pos <= game.horizon)
		v->y_pos = game.horizon + 1;

	dir = 0;
	count = size = 1;

	while (!check_position(v) || check_collision(v) || !check_priority(v)) {
		switch (dir) {
		case 0:	/* west */
			v->x_pos--;
			if (--count)
				continue;
			dir = 1;
			break;
		case 1:	/* south */
			v->y_pos++;
			if (--count)
				continue;
			dir = 2;
			size++;
			break;
		case 2:	/* east */
			v->x_pos++;
			if (--count)
				continue;
			dir = 3;
			break;
		case 3:	/* north */
			v->y_pos--;
			if (--count)
				continue;
			dir = 0;
			size++;
			break;
		}

		count = size;
	}

	debugC(4, kDebugLevelSprites, "view table entry #%d position adjusted to (%d,%d)", n, v->x_pos, v->y_pos);
}

} // End of namespace Agi
