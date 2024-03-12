
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

#ifndef M4_ADV_R_ADV_HOTSPOT_H
#define M4_ADV_R_ADV_HOTSPOT_H

#include "m4/m4_types.h"

namespace M4 {

struct HotSpotRec {
	int32 ul_x, ul_y, lr_x, lr_y;	// Hotspot screen coordinates
	int32 feet_x, feet_y;			// Walk-to target for player
	int8 facing;					// Direction player should face
	bool active;					// Flag if hotspot is active
	byte cursor_number;				// Mouse cursor number
	byte syntax;					// Word syntax
	int32 vocabID, verbID;			// ids of name and verb
	char *vocab;					// Vocabulary name of hotspot
	char *verb;						// Vocabulary default verb name
	char *prep;						// Preposition
	char *sprite;					// Sprite name
	int16 hash;						// woodscript sprite hash (runtime only)
	HotSpotRec *next;

	void clear();
};

HotSpotRec *hotspot_add_dynamic(
	char *verb, char *noun,
	int32 x1, int32 y1, int32 x2, int32 y2,
	int32 cursor,
	bool new_head = true,
	int32 walkto_x = 32767, int32 walkto_y = 32767, int32 facing = 0);

/**
 * Creates a new hot spot, doesn't add it to any list. Takes coords, sets everything
 * else to default values.
 * @returns		The new hotspot
 */
HotSpotRec *hotspot_new(int x1, int y1, int x2, int y2);

/**
 * Given a list and a hotspot to insert in that list, inserts the spot ordered by size,
 * unless new_head is true, in which case the hotspot is the head regardless of size.
 * @returns		The new head of the list.
 */
HotSpotRec *hotspot_add(HotSpotRec *head, HotSpotRec *h, bool new_head);
HotSpotRec *hotspot_duplicate(HotSpotRec *dupMe);

/**
 * Takes the head of a list of hot spots, and a pointer to a spot to delete.
 * @returns		The new head of the list.
 */
HotSpotRec *hotspot_delete_record(HotSpotRec *head, HotSpotRec *h);

/**
 * Takes the head of a list of hot spots, and a pointer to a spot to unlink.
 * @returns		The new head of the list.
 */
HotSpotRec *hotspot_unlink(HotSpotRec *head, HotSpotRec *h);
void hotspot_delete_all(HotSpotRec *head);

void hotspot_set_active(HotSpotRec *head, const char *name, bool active_or_not);
void hotspot_set_active(const char *name, bool active_or_not);
void hotspot_set_active_xy(HotSpotRec *head, const char *name, int32 x, int32 y, bool active_or_not);
void hotspot_set_active_xy(const char *name, int32 x, int32 y, bool active_or_not);
#define kernel_flip_hotspot(aa,bb) (hotspot_set_active(currentSceneDef.hotspots,aa,bb))
#define kernel_flip_hotspot_xy(aa,bb,xx,yy) (hotspot_set_active_xy(currentSceneDef.hotspots,aa,xx,yy,bb))
#define kernel_flip_hotspot_loc(aa,bb,xx,yy) (hotspot_set_active_xy(currentSceneDef.hotspots,aa,xx,yy,bb))

void hotspot_new_sprite(HotSpotRec *h, const char *verb);
void hotspot_newVerb(HotSpotRec *h, const char *verb);
void hotspot_newVocab(HotSpotRec *h, const char *vocab);
void hotspot_newPrep(HotSpotRec *h, const char *prep);

/**
 * Given a list of spots to check, and an x,y coordinate pair,
 * @returns		A pointer to the hotspot we're inside, or nullptr if there's nothing.
 */
HotSpotRec *hotspot_which(HotSpotRec *head, int x, int y);
HotSpotRec *hotspot_which(int x, int y);
void kill_hotspot_node(HotSpotRec *h);

void hotspot_restore_all();
void hotspot_hide_all();
void hotspot_unhide_and_dump();

} // End of namespace M4

#endif
