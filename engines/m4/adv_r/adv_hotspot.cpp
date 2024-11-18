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

#include "m4/adv_r/adv_hotspot.h"
#include "m4/core/cstring.h"
#include "m4/core/errors.h"
#include "m4/core/term.h"
#include "m4/gui/gui_vmng.h"
#include "m4/mem/mem.h"
#include "m4/mem/memman.h"
#include "m4/vars.h"

namespace M4 {

#define STR_HOT_SPOT "hot spot"
#define _MAXPOSINT 0x7fffffff

void HotSpotRec::clear() {
	ul_x = ul_y = lr_x = lr_y = 0;
	feet_x = feet_y = 0;
	facing = 0;
	active = false;
	cursor_number = 0;
	syntax = 0;
	vocabID = verbID = 0;
	vocab = verb = prep = nullptr;
	sprite = nullptr;
	hash = 0;
	next = nullptr;
}

void hotspot_new_sprite(HotSpotRec *h, const char *sprite) {
	if (!h)
		return;

	if (h->sprite)
		mem_free(h->sprite);

	h->sprite = mem_strdup(sprite);
}

void hotspot_newVerb(HotSpotRec *h, const char *verb) {
	if (!h)
		return;

	if (h->verb)
		mem_free(h->verb);

	h->verb = mem_strdup(verb);
}

void hotspot_newVocab(HotSpotRec *h, const char *vocab) {
	if (!h)
		return;

	if (h->vocab)
		mem_free(h->vocab);

	h->vocab = mem_strdup(vocab);
}

void hotspot_newPrep(HotSpotRec *h, const char *prep) {
	if (!h)
		return;

	if (h->prep)
		mem_free(h->prep);

	h->prep = mem_strdup(prep);
}

HotSpotRec *hotspot_new(int x1, int y1, int x2, int y2) {
	HotSpotRec *newSpot = (HotSpotRec *)mem_alloc(sizeof(HotSpotRec), STR_HOT_SPOT);
	if (!newSpot)
		return newSpot;

	newSpot->ul_x = x1;
	newSpot->ul_y = y1;
	newSpot->lr_x = x2;
	newSpot->lr_y = y2;
	newSpot->sprite = nullptr;
	newSpot->vocab = nullptr;
	newSpot->verb = nullptr;
	newSpot->prep = nullptr;
	newSpot->syntax = 0;		// Unused field
	newSpot->cursor_number = 0;
	newSpot->facing = 5;
	newSpot->feet_x = 32767;
	newSpot->feet_y = 32767;
	newSpot->next = nullptr;
	newSpot->active = true;
	return newSpot;
}

HotSpotRec *hotspot_duplicate(HotSpotRec *dupMe) {
	HotSpotRec *newSpot = (HotSpotRec *)mem_alloc(sizeof(HotSpotRec), STR_HOT_SPOT);
	if (!newSpot)
		return newSpot;

	newSpot->ul_x = dupMe->ul_x;
	newSpot->ul_y = dupMe->ul_y;
	newSpot->lr_x = dupMe->lr_x;
	newSpot->lr_y = dupMe->lr_y;
	newSpot->sprite = mem_strdup(dupMe->sprite);
	newSpot->vocab = mem_strdup(dupMe->vocab);
	newSpot->verb = mem_strdup(dupMe->verb);
	newSpot->prep = mem_strdup(dupMe->prep);
	newSpot->syntax = dupMe->syntax;
	newSpot->cursor_number = dupMe->cursor_number;
	newSpot->facing = dupMe->facing;
	newSpot->feet_x = dupMe->feet_x;
	newSpot->feet_y = dupMe->feet_y;
	newSpot->next = nullptr;
	return newSpot;
}

static int32 hotspot_area(HotSpotRec *h) {
	if (!h)
		return _MAXPOSINT;

	return (h->lr_x - h->ul_x) * (h->lr_y - h->ul_y);
}

HotSpotRec *hotspot_add(HotSpotRec *head, HotSpotRec *h, bool new_head) {
	int hArea = hotspot_area(h);
	int iArea;
	HotSpotRec *i;

	// Is head nullptr?
	if (!head)
		return h;

	// Is the incoming spot the new head?
	if (new_head || hArea < hotspot_area(head)) {
		h->next = head;
		return h;
	}

	i = head;
	while (i) {
		iArea = hotspot_area(i->next);
		if (hArea < iArea) {
			h->next = i->next;
			i->next = h;
			i = nullptr;
		} else
			i = i->next;
	}

	return head;
}

HotSpotRec *hotspot_add_dynamic(const char *verb, const char *noun,
		int32 x1, int32 y1, int32 x2, int32 y2, int32 cursor,
		bool new_head, int32 walkto_x, int32 walkto_y, int32 facing) {
	int32 status;
	ScreenContext *sc = vmng_screen_find(_G(gameDrawBuff), &status);
	y2 = MIN(y2, sc->y2);

	HotSpotRec *hotspot = hotspot_new(x1, y1, x2, y2);
	if (!hotspot)
		error("hotspot_new failed");

	hotspot_newVocab(hotspot, noun);
	hotspot_newVerb(hotspot, verb);
	hotspot->feet_x = walkto_x;
	hotspot->feet_y = walkto_y;
	hotspot->cursor_number = cursor;
	hotspot->facing = facing;

	_G(currentSceneDef).hotspots = hotspot_add(_G(currentSceneDef).hotspots,
		hotspot, new_head);

	return hotspot;
}

void kill_hotspot_node(HotSpotRec *h) {
	if (!h)
		return;

	if (h->vocab)
		mem_free(h->vocab);
	if (h->verb)
		mem_free(h->verb);
	if (h->sprite)
		mem_free(h->sprite);
	if (h->prep)
		mem_free(h->prep);
	mem_free(h);
}

HotSpotRec *hotspot_delete_record(HotSpotRec *head, HotSpotRec *h) {
	HotSpotRec *i;

	if (!(h || head))
		return head;

	// Are we deleting the head node?
	if (head == h) {
		head = h->next;
		kill_hotspot_node(h);
		return head;		// This will of course be nullptr if the head was the only thing in the list.
	}

	// Find parent of current selection
	for (i = head; i; i = i->next)
		if (i->next == h)
			break;

	i->next = h->next;

	kill_hotspot_node(h);
	return head;
}

HotSpotRec *hotspot_unlink(HotSpotRec *head, HotSpotRec *h) {
	HotSpotRec *i;

	if (!(h || head))
		return head;

	// Are we deleting the head node?
	if (head == h) {
		head = h->next;
		return (head);			// This will of course be nullptr if the head was the only thing in the list.
	}

	// Find parent of current selection
	for (i = head; i; i = i->next)
		if (i->next == h)
			break;

	i->next = h->next;
	return head;
}

void hotspot_delete_all(HotSpotRec *head) {
	HotSpotRec *i;
	HotSpotRec *next;

	for (i = head; i; i = next) {
		next = i->next;
		kill_hotspot_node(i);
	}
}

HotSpotRec *hotspot_which(HotSpotRec *head, int x, int y) {
	HotSpotRec *i;
	for (i = head; i; i = i->next)
		if ((x >= i->ul_x) && (x <= i->lr_x) && (y >= i->ul_y) && (y <= i->lr_y) && i->active)
			return i;

	return nullptr;
}

HotSpotRec *hotspot_which(int x, int y) {
	return hotspot_which(_G(currentSceneDef).hotspots, x, y);
}

void hotspot_set_active(HotSpotRec *head, const char *name, bool active_or_not) {
	char name_str[MAX_FILENAME_SIZE];
	HotSpotRec *i;
	bool hotspot_found = false;

	cstrncpy(name_str, name, MAX_FILENAME_SIZE);
	cstrupr(name_str);

	for (i = head; i; i = i->next) {
		if (!scumm_strnicmp(i->vocab, name_str, MAX_FILENAME_SIZE)) {
			i->active = active_or_not;
			hotspot_found = true;
		}
	}
	if (hotspot_found != true)
		term_message("hotspot '%s' not found!", name_str);
}

void hotspot_set_active(const char *name, bool active_or_not) {
	hotspot_set_active(_G(currentSceneDef).hotspots, name, active_or_not);
}

void hotspot_set_active_xy(HotSpotRec *head, const char *name, int32 x, int32 y, bool active_or_not) {
	char name_str[MAX_FILENAME_SIZE];
	HotSpotRec *i;

	cstrncpy(name_str, name, MAX_FILENAME_SIZE);

	cstrupr(name_str);

	for (i = head; i; i = i->next)
		if (!scumm_strnicmp(i->vocab, name_str, MAX_FILENAME_SIZE))
			if ((x >= i->ul_x) && (x <= i->lr_x) && (y >= i->ul_y) && (y <= i->lr_y))
				i->active = active_or_not;
}

void hotspot_set_active_xy(const char *name, int32 x, int32 y, bool active_or_not) {
	hotspot_set_active_xy(_G(currentSceneDef).hotspots, name, x, y, active_or_not);
}

//-----------------------------------------------------------------------

static HotSpotRec *saved_hotspots = nullptr;

void hotspot_hide_all() {
	if (saved_hotspots)
		error_show(FL, 'HNST');

	saved_hotspots = _G(currentSceneDef).hotspots;
	_G(currentSceneDef).hotspots = nullptr;
}

void hotspot_restore_all() {
	if (!saved_hotspots) {
		error_show(FL, 'HNON');
	}

	if (_G(currentSceneDef).hotspots)
		hotspot_delete_all(_G(currentSceneDef).hotspots);

	_G(currentSceneDef).hotspots = saved_hotspots;
	saved_hotspots = nullptr;
}

void hotspot_unhide_and_dump() {
	if (saved_hotspots) {
		hotspot_restore_all();
	}
}

} // End of namespace M4
