
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

#include "common/array.h"
#include "common/serializer.h"
#include "common/stream.h"

namespace M4 {

struct InvObj {
	char *name = nullptr;
	char *verbs = nullptr;
	int32 scene = 0, cel = 0, cursor = 0;
};

struct InventoryBase {
	Common::Array<InvObj *> _objects;
	int32 _tail = 0;

	InventoryBase() {}
	virtual ~InventoryBase();

	void syncGame(Common::Serializer &s);

	virtual void add(const Common::String &name, const Common::String &verb, int32 sprite, int32 cursor) = 0;
	virtual void set_scroll(int32 scroll) = 0;
	virtual void remove(const Common::String &name) = 0;
};

/**
 * Init the system, preferably in game_systems_initialize
 */
bool inv_init(int32 num_objects);

/**
 * Register things during init of the game
 * @param itemName	Name of the object as it should appear as a sentence is built
 * @param itemVerbs	Verbs should have this format: verbs = "slit,peel,fricasee,examine"
 *					There can be any number of verbs in the string.
 * @param scene		The place for the thing to appear initially (BACKPACK is one place)
 * @param cel		Index into the inventory sprite series for use when displaying inventory
 * @param cursor	Cel index into the cursor sprite series when the player is "holding" a thing
 */
bool inv_register_thing(const Common::String &itemName, const Common::String &itemVerbs, int32 scene, int32 cel, int32 cursor);

int32 inv_where_is(const Common::String &itemName);
bool inv_player_has(const Common::String &itemName);
bool inv_put_thing_in(const Common::String &itemName, int32 scene);
int32 inv_get_cursor(const Common::String &itemName);
int32 inv_get_cel(const Common::String &itemName);
const char *inv_get_verbs(const Common::String &itemName);
void inv_give_to_player(const Common::String &itemName);
void inv_move_object(const Common::String &itemName, int32 scene);
bool inv_object_is_here(const Common::String &itemName);
bool inv_object_in_scene(const Common::String &itemName, int32 scene);

void inv_sync_game(Common::Serializer &s);

} // End of namespace M4

#endif
