/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*  _______         ____    __         ___    ___
 * \    _  \       \    /  \  /       \   \  /   /       '   '  '
 *  |  | \  \       |  |    ||         |   \/   |         .      .
 *  |  |  |  |      |  |    ||         ||\  /|  |
 *  |  |  |  |      |  |    ||         || \/ |  |         '  '  '
 *  |  |  |  |      |  |    ||         ||    |  |         .      .
 *  |  |_/  /        \  \__//          ||    |  |
 * /_______/ynamic    \____/niversal  /__\  /____\usic   /|  .  . ibliotheque
 *                                                      /  \
 *                                                     / .  \
 * clickrem.c - Click removal helpers.                / / \  \
 *                                                   | <  /   \_
 * By entheh.                                        |  \/ /\   /
 *                                                    \_  /  > /
 *                                                      | \ / /
 *                                                      |  ' /
 *                                                       \__/
 */

#include "ags/lib/dumb-0.9.2/dumb.h"

namespace AGS3 {

typedef struct DUMB_CLICK DUMB_CLICK;


struct DUMB_CLICK_REMOVER {
	DUMB_CLICK *click;
	int n_clicks;

	int offset;
};


struct DUMB_CLICK {
	DUMB_CLICK *next;
	long pos;
	sample_t step;
};



DUMB_CLICK_REMOVER *dumb_create_click_remover(void) {
	DUMB_CLICK_REMOVER *cr = (DUMB_CLICK_REMOVER *)malloc(sizeof(*cr));
	if (!cr) return NULL;

	cr->click = NULL;
	cr->n_clicks = 0;

	cr->offset = 0;

	return cr;
}



void dumb_record_click(DUMB_CLICK_REMOVER *cr, long pos, sample_t step) {
	DUMB_CLICK *click;

	ASSERT(pos >= 0);

	if (!cr || !step) return;

	if (pos == 0) {
		cr->offset -= step;
		return;
	}

	click = (DUMB_CLICK *)malloc(sizeof(*click));
	if (!click) return;

	click->pos = pos;
	click->step = step;

	click->next = cr->click;
	cr->click = click;
	cr->n_clicks++;
}



static DUMB_CLICK *dumb_click_mergesort(DUMB_CLICK *click, int n_clicks) {
	int i;
	DUMB_CLICK *c1, *c2, **cp;

	if (n_clicks <= 1) return click;

	/* Split the list into two */
	c1 = click;
	cp = &c1;
	for (i = 0; i < n_clicks; i += 2) cp = &(*cp)->next;
	c2 = *cp;
	*cp = NULL;

	/* Sort the sublists */
	c1 = dumb_click_mergesort(c1, (n_clicks + 1) >> 1);
	c2 = dumb_click_mergesort(c2, n_clicks >> 1);

	/* Merge them */
	cp = &click;
	while (c1 && c2) {
		if (c1->pos > c2->pos) {
			*cp = c2;
			c2 = c2->next;
		} else {
			*cp = c1;
			c1 = c1->next;
		}
		cp = &(*cp)->next;
	}
	if (c2)
		*cp = c2;
	else
		*cp = c1;

	return click;
}



void dumb_remove_clicks(DUMB_CLICK_REMOVER *cr, sample_t *samples, long length, float halflife) {
	DUMB_CLICK *click;
	long pos = 0;
	int offset;
	int factor;

	if (!cr) return;

	factor = (int)floor(pow(0.5, 1.0 / halflife) * (1U << 31));

	click = dumb_click_mergesort(cr->click, cr->n_clicks);
	cr->click = NULL;
	cr->n_clicks = 0;

	while (click) {
		DUMB_CLICK *next = click->next;
		ASSERT(click->pos <= length);
		offset = cr->offset;
		if (offset < 0) {
			offset = -offset;
			while (pos < click->pos) {
				samples[pos++] -= offset;
				offset = (int)((LONG_LONG)(offset << 1) * factor >> 32);
			}
			offset = -offset;
		} else {
			while (pos < click->pos) {
				samples[pos++] += offset;
				offset = (int)((LONG_LONG)(offset << 1) * factor >> 32);
			}
		}
		cr->offset = offset - click->step;
		free(click);
		click = next;
	}

	offset = cr->offset;
	if (offset < 0) {
		offset = -offset;
		while (pos < length) {
			samples[pos++] -= offset;
			offset = (int)((LONG_LONG)(offset << 1) * factor >> 32);
		}
		offset = -offset;
	} else {
		while (pos < length) {
			samples[pos++] += offset;
			offset = (int)((LONG_LONG)(offset << 1) * factor >> 32);
		}
	}
	cr->offset = offset;
}



sample_t dumb_click_remover_get_offset(DUMB_CLICK_REMOVER *cr) {
	return cr ? cr->offset : 0;
}



void dumb_destroy_click_remover(DUMB_CLICK_REMOVER *cr) {
	if (cr) {
		DUMB_CLICK *click = cr->click;
		while (click) {
			DUMB_CLICK *next = click->next;
			free(click);
			click = next;
		}
		free(cr);
	}
}



DUMB_CLICK_REMOVER **dumb_create_click_remover_array(int n) {
	int i;
	DUMB_CLICK_REMOVER **cr;
	if (n <= 0) return NULL;
	cr = (DUMB_CLICK_REMOVER **)malloc(n * sizeof(*cr));
	if (!cr) return NULL;
	for (i = 0; i < n; i++) cr[i] = dumb_create_click_remover();
	return cr;
}



void dumb_record_click_array(int n, DUMB_CLICK_REMOVER **cr, long pos, sample_t *step) {
	if (cr) {
		int i;
		for (i = 0; i < n; i++)
			dumb_record_click(cr[i], pos, step[i]);
	}
}



void dumb_record_click_negative_array(int n, DUMB_CLICK_REMOVER **cr, long pos, sample_t *step) {
	if (cr) {
		int i;
		for (i = 0; i < n; i++)
			dumb_record_click(cr[i], pos, -step[i]);
	}
}



void dumb_remove_clicks_array(int n, DUMB_CLICK_REMOVER **cr, sample_t **samples, long length, float halflife) {
	if (cr) {
		int i;
		for (i = 0; i < n; i++)
			dumb_remove_clicks(cr[i], samples[i], length, halflife);
	}
}



void dumb_click_remover_get_offset_array(int n, DUMB_CLICK_REMOVER **cr, sample_t *offset) {
	if (cr) {
		int i;
		for (i = 0; i < n; i++)
			if (cr[i]) offset[i] += cr[i]->offset;
	}
}



void dumb_destroy_click_remover_array(int n, DUMB_CLICK_REMOVER **cr) {
	if (cr) {
		int i;
		for (i = 0; i < n; i++) dumb_destroy_click_remover(cr[i]);
		free(cr);
	}
}

} // namespace AGS3
