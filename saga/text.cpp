/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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

#include "saga/gfx.h"
#include "saga/font.h"

#include "saga/text.h"

namespace Saga {

int SagaEngine::textDraw(int font_id, Surface *ds, const char *string, int text_x, int text_y, int color,
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
		if (text_x < TEXT_CENTERLIMIT) {
			text_x = TEXT_CENTERLIMIT;
		}

		if (text_x > ds->w - TEXT_CENTERLIMIT) {
			text_x = ds->w - TEXT_CENTERLIMIT;
		}

		if (text_x < (TEXT_MARGIN * 2)) {
			// Text can't be centered if it's too close to the margin
			return FAILURE;
		}

		string_w = _font->getStringWidth(font_id, string, string_len, flags);

		if (text_x < (ds->w / 2)) {
			// Fit to right side
			fit_w = (text_x - TEXT_MARGIN) * 2;
		} else {
			// Fit to left side
			fit_w = ((ds->w - TEXT_MARGIN) - text_x) * 2;
		}

		if (fit_w >= string_w) {
			// Entire string fits, draw it
			text_x = text_x - (string_w / 2);
			_font->draw(font_id, ds, string, string_len, text_x, text_y, color, effect_color, flags);
			return SUCCESS;
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
					return SUCCESS;
				}

				// Wrap what we've got and restart
				_font->draw(font_id, ds, start_p, len_total, text_x - (w_total / 2), text_y, color, 
							effect_color, flags);
				text_y += h + TEXT_LINESPACING;
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
					return SUCCESS;
				}
				search_p = measure_p + 1;
			}
		}
	} else {
		// Text is not centered; No formatting required
		_font->draw(font_id, ds, string, string_len, text_x, text_y, color, effect_color, flags);
	}

	return SUCCESS;
}

TEXTLIST *SagaEngine::textCreateList() {
	TEXTLIST *new_textlist;

	new_textlist =  new TEXTLIST;

	if (new_textlist == NULL) {
		return NULL;
	}

	return new_textlist;
}

void SagaEngine::textClearList(TEXTLIST *tlist) {
	if (tlist != NULL) {
		tlist->clear();
	}

	return;
}

void SagaEngine::textDestroyList(TEXTLIST *tlist) {
	if (tlist != NULL) {
		delete tlist;
	}	
	return;
}

int SagaEngine::textDrawList(TEXTLIST *textlist, Surface *ds) {
	TEXTLIST_ENTRY *entry_p;

	assert((textlist != NULL) && (ds != NULL));

	for (TEXTLIST::iterator texti = textlist->begin(); texti != textlist->end(); ++texti) {
		entry_p = (TEXTLIST_ENTRY *)texti.operator->();
		if (entry_p->display != 0) {
			textDraw(entry_p->font_id, ds, entry_p->string, entry_p->text_x, entry_p->text_y, entry_p->color,
			entry_p->effect_color, entry_p->flags);
		}
	}

	return SUCCESS;
}

int SagaEngine::textProcessList(TEXTLIST *textlist, long ms) {
	TEXTLIST_ENTRY *entry_p;

	for (TEXTLIST::iterator texti = textlist->begin(); texti != textlist->end(); ++texti) {
		entry_p = (TEXTLIST_ENTRY *)texti.operator->();
		if (entry_p->flags & TEXT_TIMEOUT) {
			entry_p->time -= ms;
			if (entry_p->time <= 0) {
				texti=textlist->eraseAndPrev(texti);
			}
		}
	}

	return SUCCESS;

}

TEXTLIST_ENTRY *SagaEngine::textAddEntry(TEXTLIST *textlist, TEXTLIST_ENTRY *entry) {
	if (entry != NULL) {
		return textlist->pushBack(*entry).operator->();
	}
	return NULL;
}

int SagaEngine::textSetDisplay(TEXTLIST_ENTRY *entry, int val) {
	if (entry != NULL) {
		entry->display = !!val;
		return SUCCESS;
	}

	return FAILURE;
}

int SagaEngine::textDeleteEntry(TEXTLIST *textlist, TEXTLIST_ENTRY *entry) {

	if (entry == NULL) {
		return FAILURE;
	}
	
	textlist->remove(entry);

	return SUCCESS;
}

} // End of namespace Saga

