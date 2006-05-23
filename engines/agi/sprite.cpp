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
#include "agi/list.h"
#include "agi/sprite.h"
#include "agi/graphics.h"
#include "agi/text.h"
#include "agi/savegame.h"

namespace Agi {

/**
 * Sprite structure.
 * This structure holds information on visible and priority data of
 * a rectangular area of the AGI screen. Sprites are chained in two
 * circular lists, one for updating and other for non-updating sprites.
 */
struct sprite {
	struct list_head list;
	struct vt_entry *v;		/**< pointer to view table entry */
	int16 x_pos;			/**< x coordinate of the sprite */
	int16 y_pos;			/**< y coordinate of the sprite */
	int16 x_size;			/**< width of the sprite */
	int16 y_size;			/**< height of the sprite */
	uint8 *buffer;			/**< buffer to store background data */
#ifdef USE_HIRES
	uint8 *hires;			/**< buffer for hi-res background */
#endif
};

/*
 * Sprite pool replaces dynamic allocation
 */
#undef ALLOC_DEBUG

#ifdef USE_HIRES
#define POOL_SIZE 68000		/* Gold Rush mine room needs > 50000 */
	/* Speeder bike challenge needs > 67000 */
#else
#define POOL_SIZE 25000
#endif
static uint8 *sprite_pool;
static uint8 *pool_top;

static void *pool_alloc(int size) {
	uint8 *x;

	/* Adjust size to 32-bit boundary to prevent data misalignment
	 * errors. 
	 */
	size = (size + 3) & ~3;

	x = pool_top;
	pool_top += size;

	if (pool_top >= (uint8 *)sprite_pool + POOL_SIZE) {
		debugC(1, kDebugLevelMain | kDebugLevelResources, "not enough memory");
		pool_top = x;
		return NULL;
	}

	return x;
}

static void pool_release(void *s) {
	pool_top = (uint8 *)s;
}

/*
 * Blitter functions
 */

/* Blit one pixel considering the priorities */

static void blit_pixel(uint8 *p, uint8 *end, uint8 col, int spr, int width, int *hidden) {
	int epr = 0, pr = 0;	/* effective and real priorities */

	/* CM: priority 15 overrides control lines and is ignored when
	 *     tracking effective priority. This tweak is needed to fix
	 *     bug #451768, and should not affect Sierra games because
	 *     sprites shouldn't have priority 15 (like the AGI Mouse
	 *     demo "mouse pointer")
	 *
	 * Update: this solution breaks other games, and can't be used.
	 */

	if (p >= end)
		return;

	/* Check if we're on a control line */
	if ((pr = *p & 0xf0) < 0x30) {
		uint8 *p1;
		/* Yes, get effective priority going down */
		for (p1 = p; p1 < end && (epr = *p1 & 0xf0) < 0x30;
		    p1 += width);
		if (p1 >= end)
			epr = 0x40;
	} else {
		epr = pr;
	}

	if (spr >= epr) {
		/* Keep control line information visible, but put our
		 * priority over water (0x30) surface
		 */
		*p = (pr < 0x30 ? pr : spr) | col;
		*hidden = false;

		/* Except if our priority is 15, which should never happen
		 * (fixes bug #451768)
		 *
		 * Update: breaks other games, can't be used
		 *
		 * if (spr == 0xf0)
		 *      *p = spr | col;
		 */
	}
}

#ifdef USE_HIRES

#define X_FACT 2		/* Horizontal hires factor */

static int blit_hires_cel(int x, int y, int spr, struct view_cel *c) {
	uint8 *q = NULL;
	uint8 *h0, *h, *end;
	int i, j, t, m, col;
	int hidden = true;

	q = c->data;
	t = c->transparency;
	m = c->mirror;
	spr <<= 4;
	h0 = &game.hires[(x + y * _WIDTH + m * (c->width - 1)) * X_FACT];

	end = game.hires + _WIDTH * X_FACT * _HEIGHT;

	for (i = 0; i < c->height; i++) {
		h = h0;
		while (*q) {
			col = (*q & 0xf0) >> 4;
			for (j = *q & 0x0f; j; j--, h += X_FACT * (1 - 2 * m)) {
				if (col != t) {
					blit_pixel(h, end, col, spr, _WIDTH * X_FACT, &hidden);
					blit_pixel(h + 1, end, col, spr, _WIDTH * X_FACT, &hidden);
				}
			}
			q++;
		}
		h0 += _WIDTH * X_FACT;
		q++;
	}
	return hidden;
}

#endif

static int blit_cel(int x, int y, int spr, struct view_cel *c) {
	uint8 *p0, *p, *q = NULL, *end;
	int i, j, t, m, col;
	int hidden = true;

	/* Fixes bug #477841 (crash in PQ1 map C4 when y == -2) */
	if (y < 0)
		y = 0;
	if (x < 0)
		x = 0;
	if (y >= _HEIGHT)
		y = _HEIGHT - 1;
	if (x >= _WIDTH)
		x = _WIDTH - 1;

#ifdef USE_HIRES
	if (opt.hires)
		blit_hires_cel(x, y, spr, c);
#endif

	q = c->data;
	t = c->transparency;
	m = c->mirror;
	spr <<= 4;
	p0 = &game.sbuf[x + y * _WIDTH + m * (c->width - 1)];

	end = game.sbuf + _WIDTH * _HEIGHT;

	for (i = 0; i < c->height; i++) {
		p = p0;
		while (*q) {
			col = (*q & 0xf0) >> 4;
			for (j = *q & 0x0f; j; j--, p += 1 - 2 * m) {
				if (col != t) {
					blit_pixel(p, end, col, spr, _WIDTH, &hidden);
				}
			}
			q++;
		}
		p0 += _WIDTH;
		q++;
	}

	return hidden;
}

static void objs_savearea(struct sprite *s) {
	int y;
	int16 x_pos = s->x_pos, y_pos = s->y_pos;
	int16 x_size = s->x_size, y_size = s->y_size;
	uint8 *p0, *q;
#ifdef USE_HIRES
	uint8 *h0, *k;
#endif

	if (x_pos + x_size > _WIDTH)
		x_size = _WIDTH - x_pos;

	if (x_pos < 0) {
		x_size += x_pos;
		x_pos = 0;
	}

	if (y_pos + y_size > _HEIGHT)
		y_size = _HEIGHT - y_pos;

	if (y_pos < 0) {
		y_size += y_pos;
		y_pos = 0;
	}

	if (x_size <= 0 || y_size <= 0)
		return;

	p0 = &game.sbuf[x_pos + y_pos * _WIDTH];
	q = s->buffer;
#ifdef USE_HIRES
	h0 = &game.hires[(x_pos + y_pos * _WIDTH) * 2];
	k = s->hires;
#endif
	for (y = 0; y < y_size; y++) {
		memcpy(q, p0, x_size);
		q += x_size;
		p0 += _WIDTH;
#ifdef USE_HIRES
		memcpy(k, h0, x_size * 2);
		k += x_size * 2;
		h0 += _WIDTH * 2;
#endif
	}
}

static void objs_restorearea(struct sprite *s) {
	int y, offset;
	int16 x_pos = s->x_pos, y_pos = s->y_pos;
	int16 x_size = s->x_size, y_size = s->y_size;
	uint8 *p0, *q;
#ifdef USE_HIRES
	uint8 *h0, *k;
#endif

	if (x_pos + x_size > _WIDTH)
		x_size = _WIDTH - x_pos;

	if (x_pos < 0) {
		x_size += x_pos;
		x_pos = 0;
	}

	if (y_pos + y_size > _HEIGHT)
		y_size = _HEIGHT - y_pos;

	if (y_pos < 0) {
		y_size += y_pos;
		y_pos = 0;
	}

	if (x_size <= 0 || y_size <= 0)
		return;

	p0 = &game.sbuf[x_pos + y_pos * _WIDTH];
	q = s->buffer;
#ifdef USE_HIRES
	h0 = &game.hires[(x_pos + y_pos * _WIDTH) * 2];
	k = s->hires;
#endif
	offset = game.line_min_print * CHAR_LINES;
	for (y = 0; y < y_size; y++) {
		memcpy(p0, q, x_size);
		put_pixels_a(x_pos, y_pos + y + offset, x_size, p0);
		q += x_size;
		p0 += _WIDTH;
#ifdef USE_HIRES
		memcpy(h0, k, x_size * 2);
		if (opt.hires) {
			put_pixels_hires(x_pos * 2, y_pos + y + offset, x_size * 2, h0);
		}
		k += x_size * 2;
		h0 += _WIDTH * 2;
#endif
	}
}

/*
 * Sprite management functions
 */

static LIST_HEAD(spr_upd_head);
static LIST_HEAD(spr_nonupd_head);

/**
 * Condition to determine whether a sprite will be in the 'updating' list.
 */
static int test_updating(struct vt_entry *v) {
	/* Sanity check (see bug #779302) */
	if (~game.dir_view[v->current_view].flags & RES_LOADED)
		return 0;

	return (v->flags & (ANIMATED | UPDATE | DRAWN)) == (ANIMATED | UPDATE | DRAWN);
}

/**
 * Condition to determine whether a sprite will be in the 'non-updating' list.
 */
static int test_not_updating(struct vt_entry *v) {
	/* Sanity check (see bug #779302) */
	if (~game.dir_view[v->current_view].flags & RES_LOADED)
		return 0;

	return (v->flags & (ANIMATED | UPDATE | DRAWN)) == (ANIMATED | DRAWN);
}

/**
 * Convert sprite priority to y value.
 */
static INLINE int prio_to_y(int p) {
	int i;

	if (p == 0)
		return -1;

	for (i = 167; i >= 0; i--) {
		if (game.pri_table[i] < p)
			return i;
	}

	return -1;		/* (p - 5) * 12 + 48; */
}

/**
 * Create and initialize a new sprite structure.
 */
static struct sprite *new_sprite(struct vt_entry *v) {
	struct sprite *s;

	s = (struct sprite *)pool_alloc(sizeof(struct sprite));
	if (s == NULL)
		return NULL;

	s->v = v;		/* link sprite to associated view table entry */
	s->x_pos = v->x_pos;
	s->y_pos = v->y_pos - v->y_size + 1;
	s->x_size = v->x_size;
	s->y_size = v->y_size;
	s->buffer = (uint8 *) pool_alloc(s->x_size * s->y_size);
#ifdef USE_HIRES
	s->hires = (uint8 *) pool_alloc(s->x_size * s->y_size * 2);
#endif
	v->s = s;		/* link view table entry to this sprite */

	return s;
}

/**
 * Insert sprite in the specified sprite list.
 */
static void spr_addlist(struct list_head *head, struct vt_entry *v) {
	struct sprite *s;

	s = new_sprite(v);
	list_add_tail(&s->list, head);
}

/**
 * Sort sprites from lower y values to build a sprite list.
 */
static struct list_head *build_list(struct list_head *head,
									int (*test) (struct vt_entry *)) {
	int i, j, k;
	struct vt_entry *v;
	struct vt_entry *entry[0x100];
	int y_val[0x100];
	int min_y = 0xff, min_index = 0;

	/* fill the arrays with all sprites that satisfy the 'test'
	 * condition and their y values
	 */
	i = 0;
	for (v = game.view_table; v < &game.view_table[MAX_VIEWTABLE]; v++) {
		if (test(v)) {
			entry[i] = v;
			y_val[i] = v->flags & FIXED_PRIORITY ? prio_to_y(v->priority) : v->y_pos;
			i++;
		}
	}

	/* now look for the smallest y value in the array and put that
	 * sprite in the list
	 */
	for (j = 0; j < i; j++) {
		min_y = 0xff;
		for (k = 0; k < i; k++) {
			if (y_val[k] < min_y) {
				min_index = k;
				min_y = y_val[k];
			}
		}

		y_val[min_index] = 0xff;
		spr_addlist(head, entry[min_index]);
	}

	return head;
}

/**
 * Build list of updating sprites.
 */
static struct list_head *build_upd_blitlist() {
	return build_list(&spr_upd_head, test_updating);
}

/**
 * Build list of non-updating sprites.
 */
static struct list_head *build_nonupd_blitlist() {
	return build_list(&spr_nonupd_head, test_not_updating);
}

/**
 * Clear the given sprite list.
 */
static void free_list(struct list_head *head) {
	struct list_head *h;
	struct sprite *s;

	list_for_each(h, head, prev) {
		s = list_entry(h, struct sprite, list);
		list_del(h);
#ifdef USE_HIRES
		pool_release(s->hires);
#endif
		pool_release(s->buffer);
		pool_release(s);
	}
}

/**
 * Copy sprites from the pic buffer to the screen buffer, and check if
 * sprites of the given list have moved.
 */
static void commit_sprites(struct list_head *head) {
	struct list_head *h;

	list_for_each(h, head, next) {
		struct sprite *s = list_entry(h, struct sprite, list);
		int x1, y1, x2, y2, w, h;

		w = (s->v->cel_data->width > s->v->cel_data_2->width) ?
				s->v->cel_data->width : s->v->cel_data_2->width;

		h = (s->v->cel_data->height >
				s->v->cel_data_2->height) ? s->v->cel_data->
				height : s->v->cel_data_2->height;

		s->v->cel_data_2 = s->v->cel_data;

		if (s->v->x_pos < s->v->x_pos2) {
			x1 = s->v->x_pos;
			x2 = s->v->x_pos2 + w - 1;
		} else {
			x1 = s->v->x_pos2;
			x2 = s->v->x_pos + w - 1;
		}

		if (s->v->y_pos < s->v->y_pos2) {
			y1 = s->v->y_pos - h + 1;
			y2 = s->v->y_pos2;
		} else {
			y1 = s->v->y_pos2 - h + 1;
			y2 = s->v->y_pos;
		}

		commit_block(x1, y1, x2, y2);

		if (s->v->step_time_count != s->v->step_time)
			continue;

		if (s->v->x_pos == s->v->x_pos2 && s->v->y_pos == s->v->y_pos2) {
			s->v->flags |= DIDNT_MOVE;
			continue;
		}

		s->v->x_pos2 = s->v->x_pos;
		s->v->y_pos2 = s->v->y_pos;
		s->v->flags &= ~DIDNT_MOVE;
	}

#ifdef USE_CONSOLE
	if (debug_.statusline)
		write_status();
#endif
}

/**
 * Erase all sprites in the given list.
 */
static void erase_sprites(struct list_head *head) {
	struct list_head *h;

	list_for_each(h, head, prev) {
		struct sprite *s = list_entry(h, struct sprite, list);
		objs_restorearea(s);
	}

	free_list(head);
}

/**
 * Blit all sprites in the given list.
 */
static void blit_sprites(struct list_head *head) {
	struct list_head *h = NULL;
	int hidden;

	list_for_each(h, head, next) {
		struct sprite *s = list_entry(h, struct sprite, list);
		objs_savearea(s);
		debugC(8, kDebugLevelSprites, "s->v->entry = %d (prio %d)", s->v->entry, s->v->priority);
		hidden = blit_cel(s->x_pos, s->y_pos, s->v->priority, s->v->cel_data);
		if (s->v->entry == 0) {	/* if ego, update f1 */
			setflag(F_ego_invisible, hidden);
		}
	}
}

/*
 * Public functions
 */

void commit_upd_sprites() {
	commit_sprites(&spr_upd_head);
}

void commit_nonupd_sprites() {
	commit_sprites(&spr_nonupd_head);
}

/* check moves in both lists */
void commit_both() {
	commit_upd_sprites();
	commit_nonupd_sprites();
}

/**
 * Erase updating sprites.
 * This function follows the list of all updating sprites and restores
 * the visible and priority data of their background buffers back to
 * the AGI screen.
 *
 * @see erase_nonupd_sprites()
 * @see erase_both()
 */
void erase_upd_sprites() {
	erase_sprites(&spr_upd_head);
}

/**
 * Erase non-updating sprites.
 * This function follows the list of all non-updating sprites and restores
 * the visible and priority data of their background buffers back to
 * the AGI screen.
 *
 * @see erase_upd_sprites()
 * @see erase_both()
 */
void erase_nonupd_sprites() {
	erase_sprites(&spr_nonupd_head);
}

/**
 * Erase all sprites.
 * This function follows the lists of all updating and non-updating
 * sprites and restores the visible and priority data of their background
 * buffers back to the AGI screen.
 *
 * @see erase_upd_sprites()
 * @see erase_nonupd_sprites()
 */
void erase_both() {
	erase_upd_sprites();
	erase_nonupd_sprites();
}

/**
 * Blit updating sprites.
 * This function follows the list of all updating sprites and blits
 * them on the AGI screen.
 *
 * @see blit_nonupd_sprites()
 * @see blit_both()
 */
void blit_upd_sprites() {
	debugC(7, kDebugLevelSprites, "blit updating");
	blit_sprites(build_upd_blitlist());
}

/**
 * Blit non-updating sprites.
 * This function follows the list of all non-updating sprites and blits
 * them on the AGI screen.
 *
 * @see blit_upd_sprites()
 * @see blit_both()
 */
void blit_nonupd_sprites() {
	debugC(7, kDebugLevelSprites, "blit non-updating");
	blit_sprites(build_nonupd_blitlist());
}

/**
 * Blit all sprites.
 * This function follows the lists of all updating and non-updating
 * sprites and blits them on the AGI screen.
 *
 * @see blit_upd_sprites()
 * @see blit_nonupd_sprites()
 */
void blit_both() {
	blit_nonupd_sprites();
	blit_upd_sprites();
}

/**
 * Add view to picture.
 * This function is used to implement the add.to.pic AGI command. It
 * copies the specified cel from a view resource on the current picture.
 * This cel is not a sprite, it can't be moved or removed.
 * @param view  number of view resource
 * @param loop  number of loop in the specified view resource
 * @param cel   number of cel in the specified loop
 * @param x     x coordinate to place the view
 * @param y     y coordinate to place the view
 * @param pri   priority to use
 * @param mar   if < 4, create a margin around the the base of the cel
 */
void add_to_pic(int view, int loop, int cel, int x, int y, int pri, int mar) {
	struct view_cel *c = NULL;
	int x1, y1, x2, y2, y3;
	uint8 *p1, *p2;

	debugC(3, kDebugLevelSprites, "v=%d, l=%d, c=%d, x=%d, y=%d, p=%d, m=%d", view, loop, cel, x, y, pri, mar);

	record_image_stack_call(ADD_VIEW, view, loop, cel, x, y, pri, mar);

	/*
	 * Was hardcoded to 8, changed to pri_table[y] to fix Gold
	 * Rush (see bug #587558)
	 */
	if (pri == 0)
		pri = game.pri_table[y];

	c = &game.views[view].loop[loop].cel[cel];

	x1 = x;
	y1 = y - c->height + 1;
	x2 = x + c->width - 1;
	y2 = y;

	if (x1 < 0) {
		x2 -= x1;
		x1 = 0;
	}
	if (y1 < 0) {
		y2 -= y1;
		y1 = 0;
	}
	if (x2 >= _WIDTH)
		x2 = _WIDTH - 1;
	if (y2 >= _HEIGHT)
		y2 = _HEIGHT - 1;

	erase_both();

	debugC(4, kDebugLevelSprites, "blit_cel (%d, %d, %d, c)", x, y, pri);
	blit_cel(x1, y1, pri, c);

	/* If margin is 0, 1, 2, or 3, the base of the cel is
	 * surrounded with a rectangle of the corresponding priority.
	 * If margin >= 4, this extra margin is not shown.
	 */
	if (mar < 4) {
		/* add rectangle around object, don't clobber control
		 * info in priority data. The box extends to the end of
		 * its priority band!
		 *
		 * SQ1 needs +1 (see bug #810331)
		 */
		y3 = (y2 / 12) * 12 + 1;

		p1 = &game.sbuf[x1 + y3 * _WIDTH];
		p2 = &game.sbuf[x2 + y3 * _WIDTH];

		for (y = y3; y <= y2; y++) {
			if ((*p1 >> 4) >= 4)
				*p1 = (mar << 4) | (*p1 & 0x0f);
			if ((*p2 >> 4) >= 4)
				*p2 = (mar << 4) | (*p2 & 0x0f);
			p1 += _WIDTH;
			p2 += _WIDTH;
		}

		debugC(4, kDebugLevelSprites, "pri box: %d %d %d %d (%d)", x1, y3, x2, y2, mar);
		p1 = &game.sbuf[x1 + y3 * _WIDTH];
		p2 = &game.sbuf[x1 + y2 * _WIDTH];
		for (x = x1; x <= x2; x++) {
			if ((*p1 >> 4) >= 4)
				*p1 = (mar << 4) | (*p1 & 0x0f);
			if ((*p2 >> 4) >= 4)
				*p2 = (mar << 4) | (*p2 & 0x0f);
			p1++;
			p2++;
		}
	}

	blit_both();

	debugC(4, kDebugLevelSprites, "commit_block (%d, %d, %d, %d)", x1, y1, x2, y2);
	commit_block(x1, y1, x2, y2);
}

/**
 * Show object and description
 * This function shows an object from the player's inventory, displaying
 * a message box with the object description.
 * @param n  Number of the object to show
 */
void show_obj(int n) {
	struct view_cel *c;
	struct sprite s;
	int x1, y1, x2, y2;

	agi_load_resource(rVIEW, n);
	if (!(c = &game.views[n].loop[0].cel[0]))
		return;

	x1 = (_WIDTH - c->width) / 2;
	y1 = 112;
	x2 = x1 + c->width - 1;
	y2 = y1 + c->height - 1;

	s.x_pos = x1;
	s.y_pos = y1;
	s.x_size = c->width;
	s.y_size = c->height;
	s.buffer = (uint8 *)malloc(s.x_size * s.y_size);
#ifdef USE_HIRES
	s.hires = (uint8 *)malloc(s.x_size * s.y_size * 2);
#endif

	objs_savearea(&s);
	blit_cel(x1, y1, s.x_size, c);
	commit_block(x1, y1, x2, y2);
	message_box(game.views[n].descr);
	objs_restorearea(&s);
	commit_block(x1, y1, x2, y2);

	free(s.buffer);

	/* Added to fix a memory leak --Vasyl */
#ifdef USE_HIRES
	free(s.hires);
#endif
}

void commit_block(int x1, int y1, int x2, int y2) {
	int i, w, offset;
	uint8 *q;
#ifdef USE_HIRES
	uint8 *h;
#endif

	if (!game.picture_shown)
		return;

	/* Clipping */
	if (x1 < 0)
		x1 = 0;
	if (x2 < 0)
		x2 = 0;
	if (y1 < 0)
		y1 = 0;
	if (y2 < 0)
		y2 = 0;
	if (x1 >= _WIDTH)
		x1 = _WIDTH - 1;
	if (x2 >= _WIDTH)
		x2 = _WIDTH - 1;
	if (y1 >= _HEIGHT)
		y1 = _HEIGHT - 1;
	if (y2 >= _HEIGHT)
		y2 = _HEIGHT - 1;

	debugC(7, kDebugLevelSprites, "%d, %d, %d, %d", x1, y1, x2, y2);

	w = x2 - x1 + 1;
	q = &game.sbuf[x1 + _WIDTH * y1];
#ifdef USE_HIRES
	h = &game.hires[(x1 + _WIDTH * y1) * 2];
#endif
	offset = game.line_min_print * CHAR_LINES;
	for (i = y1; i <= y2; i++) {
		put_pixels_a(x1, i + offset, w, q);
		q += _WIDTH;
#ifdef USE_HIRES
		if (opt.hires) {
			put_pixels_hires(x1 * 2, i + offset, w * 2, h);
		}
		h += _WIDTH * 2;
#endif
	}

	flush_block_a(x1, y1 + offset, x2, y2 + offset);
}

int init_sprites() {
	if ((sprite_pool = (uint8 *)malloc(POOL_SIZE)) == NULL)
		return err_NotEnoughMemory;

	pool_top = sprite_pool;

	return err_OK;
}

void deinit_sprites() {
	free(sprite_pool);
}

}                             // End of namespace Agi
