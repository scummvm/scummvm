
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

#ifndef M4_ADV_R_ADV_INV_H
#define M4_ADV_R_ADV_INV_H

#include "common/stream.h"

namespace M4 {

struct InvObj {
	char *name = nullptr;
	char *verbs = nullptr;
	int32 scene = 0, cel = 0, cursor = 0;
};

struct InventoryRec {
	InvObj **objects = nullptr;
	int32 num_objects = 0;
	int32 tail = 0;
};

/**
 * Init the system, preferably in game_systems_initialize
 */
extern bool  inv_init(int32 num_objects);

/**
 * Shutdown the system
 */
extern void  inv_shutdown();

/**
 * Register things during init of the game
 * @param name		Name of the object as it should appear as a sentence is built
 * @param verbs		Verbs should have this format: verbs = "slit,peel,fricasee,examine"
 *					There can be any number of verbs in the string.
 * @param scene		The place for the thing to appear initially (BACKPACK is one place)
 * @param cel		Index into the inventory sprite series for use when displaying inventory
 * @param cursor	Cel index into the cursor sprite series when the player is "holding" a thing
 */
extern bool  inv_register_thing(const char *name, const char *verbs, int32 scene, int32 cel, int32 cursor);

extern int32 inv_where_is(const char *name);
extern bool  inv_player_has(const char *name);
extern bool  inv_put_thing_in(const char *name, int32 scene);
extern int32 inv_get_cursor(const char *name);
extern int32 inv_get_cel(const char *name);
extern char *inv_get_verbs(const char *name);
extern void  inv_give_to_player(const char *name);
extern void  inv_move_object(const char *name, int32 scene);
extern bool  inv_object_is_here(const char *name);
extern bool  inv_object_in_scene(const char *name, int32 scene);

// private

extern void MoveBP(const char *s, int32 from, int32 to);
extern void InsertBP(const char *s, int32 where);

// private

extern void inv_save_game(Common::WriteStream *fp_save);
extern void inv_restore_game(Common::SeekableReadStream *fp_restore);

} // End of namespace M4

#endif
