/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2003 Sarien Team
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

#include "agi/agi.h"
#include "agi/sprite.h"

namespace Agi {

static void _set_cel(vt_entry *v, int n) {
	view_loop *current_vl;
	view_cel *current_vc;

	v->current_cel = n;

	current_vl = &game.views[v->current_view].loop[v->current_loop];

	/* Added by Amit Vainsencher <amitv@subdimension.com> to prevent
	 * crash in KQ1 -- not in the Sierra interpreter
	 */
	if (current_vl->num_cels == 0)
		return;

	if (!(v->flags & UPDATE)
	    && (agi_get_release() >= 0x3000))
		return;

	current_vc = &current_vl->cel[n];
	v->cel_data = current_vc;
	v->x_size = current_vc->width;
	v->y_size = current_vc->height;
}

static void _set_loop(vt_entry *v, int n) {
	view_loop *current_vl;
	debugC(7, kDebugLevelResources, "vt entry #%d, loop = %d", v->entry, n);

	/* Added to avoid crash when leaving the arcade machine in MH1
	 * -- not in AGI 2.917
	 */
	if (n >= v->num_loops)
		n = 0;

	v->current_loop = n;
	current_vl = &game.views[v->current_view].loop[v->current_loop];

	v->num_cels = current_vl->num_cels;
	if (v->current_cel >= v->num_cels)
		v->current_cel = 0;

	if (!(v->flags & UPDATE) && (agi_get_release() >= 0x3000))
		return;

	v->loop_data = &game.views[v->current_view].loop[n];
}

static void update_view(vt_entry *v) {
	int cel, last_cel;

	if (v->flags & DONTUPDATE) {
		v->flags &= ~DONTUPDATE;
		return;
	}

	cel = v->current_cel;
	last_cel = v->num_cels - 1;

	switch (v->cycle) {
	case CYCLE_NORMAL:
		if (++cel > last_cel)
			cel = 0;
		break;
	case CYCLE_END_OF_LOOP:
		if (cel < last_cel) {
			debugC(5, kDebugLevelResources, "cel %d (last = %d)", cel + 1, last_cel);
			if (++cel != last_cel)
				break;
		}
		setflag(v->parm1, true);
		v->flags &= ~CYCLING;
		v->direction = 0;
		v->cycle = CYCLE_NORMAL;
		break;
	case CYCLE_REV_LOOP:
		if (cel) {
			if (--cel)
				break;
		}
		setflag(v->parm1, true);
		v->flags &= ~CYCLING;
		v->direction = 0;
		v->cycle = CYCLE_NORMAL;
		break;
	case CYCLE_REVERSE:
		if (cel == 0) {
			cel = last_cel;
		} else {
			cel--;
		}
		break;
	}

	set_cel(v, cel);
}

/*
 * Public functions
 */

/**
 * Decode an AGI view resource.
 * This function decodes the raw data of the specified AGI view resource
 * and fills the corresponding views array element.
 * @param n number of view resource to decode
 */
int decode_view(int n) {
	int loop, cel;
	uint8 *v, *lptr;
	uint16 lofs, cofs;
	view_loop *vl;
	view_cel *vc;

	debugC(5, kDebugLevelResources, "decode_view(%d)", n);
	v = game.views[n].rdata;

	assert(v != NULL);

	game.views[n].descr = READ_LE_UINT16(v + 3) ? (char *)(v + READ_LE_UINT16(v + 3)) : (char *)(v + 3);

	/* if no loops exist, return! */
	if ((game.views[n].num_loops = *(v + 2)) == 0)
		return err_NoLoopsInView;

	/* allocate memory for all views */
	game.views[n].loop = (view_loop *)
			calloc(game.views[n].num_loops, sizeof(view_loop));

	if (game.views[n].loop == NULL)
		return err_NotEnoughMemory;

	/* decode all of the loops in this view */
	lptr = v + 5;		/* first loop address */

	for (loop = 0; loop < game.views[n].num_loops; loop++, lptr += 2) {
		lofs = READ_LE_UINT16(lptr);	/* loop header offset */
		vl = &game.views[n].loop[loop];	/* the loop struct */

		vl->num_cels = *(v + lofs);
		debugC(6, kDebugLevelResources, "view %d, num_cels = %d", n, vl->num_cels);
		vl->cel = (view_cel *) calloc(vl->num_cels, sizeof(view_cel));
		if (vl->cel == NULL) {
			free(game.views[n].loop);
			game.views[n].num_loops = 0;
			return err_NotEnoughMemory;
		}

		/* decode the cells */
		for (cel = 0; cel < vl->num_cels; cel++) {
			cofs = lofs + READ_LE_UINT16(v + lofs + 1 + (cel * 2));
			vc = &vl->cel[cel];

			vc->width = *(v + cofs);
			vc->height = *(v + cofs + 1);
			vc->transparency = *(v + cofs + 2) & 0xf;
			vc->mirror_loop = (*(v + cofs + 2) >> 4) & 0x7;
			vc->mirror = (*(v + cofs + 2) >> 7) & 0x1;

			/* skip over width/height/trans|mirror data */
			cofs += 3;

			vc->data = v + cofs;
			/* If mirror_loop is pointing to the current loop,
			 * then this is the original.
			 */
			if (vc->mirror_loop == loop)
				vc->mirror = 0;
		}		/* cel */
	}			/* loop */

	return err_OK;
}

/**
 * Unloads all data in a view resource
 * @param n number of view resource
 */
void unload_view(int n) {
	int x;

	debugC(5, kDebugLevelResources, "discard view %d", n);
	if (~game.dir_view[n].flags & RES_LOADED)
		return;

	/* Rebuild sprite list, see bug #779302 */
	_sprites->erase_both();
	_sprites->blit_both();
	_sprites->commit_both();

	/* free all the loops */
	for (x = 0; x < game.views[n].num_loops; x++)
		free(game.views[n].loop[x].cel);

	free(game.views[n].loop);
	free(game.views[n].rdata);

	game.dir_view[n].flags &= ~RES_LOADED;
}

/**
 * Set a view table entry to use the specified cel of the current loop.
 * @param v pointer to view table entry
 * @param n number of cel
 */
void set_cel(vt_entry *v, int n) {
	assert(v->view_data != NULL);
	assert(v->num_cels >= n);

	_set_cel(v, n);

	/* If position isn't appropriate, update it accordingly */
	if (v->x_pos + v->x_size > _WIDTH) {
		v->flags |= UPDATE_POS;
		v->x_pos = _WIDTH - v->x_size;
	}
	if (v->y_pos - v->y_size + 1 < 0) {
		v->flags |= UPDATE_POS;
		v->y_pos = v->y_size - 1;
	}
	if (v->y_pos <= game.horizon && (~v->flags & IGNORE_HORIZON)) {
		v->flags |= UPDATE_POS;
		v->y_pos = game.horizon + 1;
	}
}

/**
 * Set a view table entry to use the specified loop of the current view.
 * @param v pointer to view table entry
 * @param n number of loop
 */
void set_loop(vt_entry *v, int n) {
	assert(v->view_data != NULL);
	assert(v->num_loops >= n);
	_set_loop(v, n);
	set_cel(v, v->current_cel);
}

/**
 * Set a view table entry to use the specified view resource.
 * @param v pointer to view table entry
 * @param n number of AGI view resource
 */
void set_view(vt_entry *v, int n) {
	v->view_data = &game.views[n];
	v->current_view = n;
	v->num_loops = v->view_data->num_loops;
	set_loop(v, v->current_loop >= v->num_loops ? 0 : v->current_loop);
}

/**
 * Set the view table entry as updating.
 * @param v pointer to view table entry
 */
void start_update(vt_entry *v) {
	if (~v->flags & UPDATE) {
		_sprites->erase_both();
		v->flags |= UPDATE;
		_sprites->blit_both();
	}
}

/**
 * Set the view table entry as non-updating.
 * @param v pointer to view table entry
 */
void stop_update(vt_entry *v) {
	if (v->flags & UPDATE) {
		_sprites->erase_both();
		v->flags &= ~UPDATE;
		_sprites->blit_both();
	}
}

/* loops to use according to direction and number of loops in
 * the view resource
 */
static int loop_table_2[] = {
	0x04, 0x04, 0x00, 0x00, 0x00, 0x04, 0x01, 0x01, 0x01
};

static int loop_table_4[] = {
	0x04, 0x03, 0x00, 0x00, 0x00, 0x02, 0x01, 0x01, 0x01
};

/**
 * Update view table entries.
 * This function is called at the end of each interpreter cycle
 * to update the view table entries and blit the sprites.
 */
void update_viewtable() {
	vt_entry *v;
	int i, loop;

	i = 0;
	for (v = game.view_table; v < &game.view_table[MAX_VIEWTABLE]; v++) {
		if ((v->flags & (ANIMATED | UPDATE | DRAWN)) != (ANIMATED | UPDATE | DRAWN)) {
			continue;
		}

		i++;

		loop = 4;
		if (~v->flags & FIX_LOOP) {
			switch (v->num_loops) {
			case 2:
			case 3:
				loop = loop_table_2[v->direction];
				break;
			case 4:
				loop = loop_table_4[v->direction];
				break;
			default:
				/* for KQ4 */
				if (agi_get_release() == 0x3086)
					loop = loop_table_4[v->direction];
				break;
			}
		}

		/* AGI 2.272 (ddp, xmas) doesn't test step_time_count! */
		if (loop != 4 && loop != v->current_loop) {
			if (agi_get_release() <= 0x2272 ||
			    v->step_time_count == 1) {
				set_loop(v, loop);
			}
		}

		if (~v->flags & CYCLING)
			continue;

		if (v->cycle_time_count == 0)
			continue;

		if (--v->cycle_time_count == 0) {
			update_view(v);
			v->cycle_time_count = v->cycle_time;
		}
	}

	if (i) {
		_sprites->erase_upd_sprites();
		update_position();
		_sprites->blit_upd_sprites();
		_sprites->commit_upd_sprites();
		game.view_table[0].flags &= ~(ON_WATER | ON_LAND);
	}
}

bool is_ego_view(const vt_entry* v) {
	return v == game.view_table;
}

}                             // End of namespace Agi
