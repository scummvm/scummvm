/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Text / dialogue display management module

#include "saga/saga.h"
#include "saga/yslib.h"

#include "saga/gfx.h"
#include "saga/font.h"

#include "saga/text.h"

namespace Saga {

int SagaEngine::textDraw(int font_id, R_SURFACE *ds, const char *string, int text_x, int text_y, int color,
				int effect_color, int flags) {
	int string_w;
	int string_len;
	int fit_w;
	const char *start_p;
	const char *search_p;
	const char *measure_p;
	const char *found_p;
	int len;
	int w;
	const char *end_p;
	int h;
	int wc;
	int w_total;
	int len_total;

	string_len = strlen(string);

	if (flags & FONT_CENTERED) {
		// Text is centered... format output
		// Enforce minimum and maximum center points for centered text
		if (text_x < R_TEXT_CENTERLIMIT) {
			text_x = R_TEXT_CENTERLIMIT;
		}

		if (text_x > ds->buf_w - R_TEXT_CENTERLIMIT) {
			text_x = ds->buf_w - R_TEXT_CENTERLIMIT;
		}

		if (text_x < (R_TEXT_MARGIN * 2)) {
			// Text can't be centered if it's too close to the margin
			return R_FAILURE;
		}

		string_w = _font->getStringWidth(font_id, string, string_len, flags);

		if (text_x < (ds->buf_w / 2)) {
			// Fit to right side
			fit_w = (text_x - R_TEXT_MARGIN) * 2;
		} else {
			// Fit to left side
			fit_w = ((ds->buf_w - R_TEXT_MARGIN) - text_x) * 2;
		}

		if (fit_w >= string_w) {
			// Entire string fits, draw it
			text_x = text_x - (string_w / 2);
			_font->draw(font_id, ds, string, string_len, text_x, text_y, color, effect_color, flags);
			return R_SUCCESS;
		}

		// String won't fit on one line
		h = _font->getHeight(font_id);
		w_total = 0;
		len_total = 0;
		wc = 0;

		start_p = string;
		measure_p = string;
		search_p = string;
		end_p = string + string_len;

		for (;;) {
			found_p = strchr(search_p, ' ');
			if (found_p == NULL) {
				// Ran to the end of the buffer
				len = end_p - measure_p;
			} else {
				len = found_p - measure_p;
			}

			w = _font->getStringWidth(font_id, measure_p, len, flags);
			measure_p = found_p;

			if ((w_total + w) > fit_w) {
				// This word won't fit
				if (wc == 0) {
					// The first word in the line didn't fit. abort
					return R_SUCCESS;
				}

				// Wrap what we've got and restart
				_font->draw(font_id, ds, start_p, len_total, text_x - (w_total / 2), text_y, color, 
							effect_color, flags);
				text_y += h + R_TEXT_LINESPACING;
				w_total = 0;
				len_total = 0;
				wc = 0;
				measure_p = search_p;
				start_p = search_p;
			} else {
				// Word will fit ok
				w_total += w;
				len_total += len;
				wc++;
				if (found_p == NULL) {
					// Since word hit NULL but fit, we are done
					_font->draw(font_id, ds, start_p, len_total, text_x - (w_total / 2), text_y, color,
								effect_color, flags);
					return R_SUCCESS;
				}
				search_p = measure_p + 1;
			}
		}
	} else {
		// Text is not centered; No formatting required
		_font->draw(font_id, ds, string, string_len, text_x, text_y, color, effect_color, flags);
	}

	return R_SUCCESS;
}

R_TEXTLIST *SagaEngine::textCreateList() {
	R_TEXTLIST *new_textlist;

	new_textlist = (R_TEXTLIST *)malloc(sizeof *new_textlist);

	if (new_textlist == NULL) {
		return NULL;
	}

	new_textlist->list = ys_dll_create();

	if (new_textlist->list == NULL) {
		free(new_textlist);
		return NULL;
	}

	return new_textlist;
}

void SagaEngine::textClearList(R_TEXTLIST *tlist) {
	if (tlist != NULL) {
		ys_dll_delete_all(tlist->list);
	}

	return;
}

void SagaEngine::textDestroyList(R_TEXTLIST *tlist) {
	if (tlist != NULL) {
		ys_dll_destroy(tlist->list);
	}
	free(tlist);

	return;
}

int SagaEngine::textDrawList(R_TEXTLIST *textlist, R_SURFACE *ds) {
	R_TEXTLIST_ENTRY *entry_p;
	YS_DL_NODE *walk_p;

	assert((textlist != NULL) && (ds != NULL));

	for (walk_p = ys_dll_head(textlist->list); walk_p != NULL; walk_p = ys_dll_next(walk_p)) {
		entry_p = (R_TEXTLIST_ENTRY *)ys_dll_get_data(walk_p);
		if (entry_p->display != 0) {
			textDraw(entry_p->font_id, ds, entry_p->string, entry_p->text_x, entry_p->text_y, entry_p->color,
			entry_p->effect_color, entry_p->flags);
		}
	}

	return R_SUCCESS;
}

int SagaEngine::textProcessList(R_TEXTLIST *textlist, long ms) {
	R_TEXTLIST_ENTRY *entry_p;
	YS_DL_NODE *walk_p;
	YS_DL_NODE *temp_p;

	for (walk_p = ys_dll_head(textlist->list); walk_p != NULL; walk_p = temp_p) {
		temp_p = ys_dll_next(walk_p);
		entry_p = (R_TEXTLIST_ENTRY *)ys_dll_get_data(walk_p);
		if (entry_p->flags & TEXT_TIMEOUT) {
			entry_p->time -= ms;
			if (entry_p->time <= 0) {
				ys_dll_delete(walk_p);
			}
		}
	}

	return R_SUCCESS;

}

R_TEXTLIST_ENTRY *SagaEngine::textAddEntry(R_TEXTLIST *textlist, R_TEXTLIST_ENTRY *entry) {
	YS_DL_NODE *new_node = NULL;

	if (entry != NULL) {
		new_node = ys_dll_add_tail(textlist->list, entry, sizeof *entry);
	}

	return (new_node != NULL) ? (R_TEXTLIST_ENTRY *)new_node->data : NULL;
}

int SagaEngine::textSetDisplay(R_TEXTLIST_ENTRY *entry, int val) {
	if (entry != NULL) {
		entry->display = !!val;
		return R_SUCCESS;
	}

	return R_FAILURE;
}

int SagaEngine::textDeleteEntry(R_TEXTLIST *textlist, R_TEXTLIST_ENTRY *entry) {
	YS_DL_NODE *walk_p;

	if (entry == NULL) {
		return R_FAILURE;
	}

	for (walk_p = ys_dll_head(textlist->list); walk_p != NULL; walk_p = ys_dll_next(walk_p)) {
		if (entry == ys_dll_get_data(walk_p)) {
			ys_dll_delete(walk_p);
			break;
		}
	}

	return R_SUCCESS;
}

} // End of namespace Saga

