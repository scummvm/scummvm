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

#include "common/str.h"
#include "m4/adv_r/adv_inv.h"
#include "m4/core/cstring.h"
#include "m4/core/errors.h"
#include "m4/mem/mem.h"
#include "m4/vars.h"

namespace M4 {

#define MAX_NAME_LENGTH 40

static char *inv_get_name(const Common::String &itemName);

InventoryBase::~InventoryBase() {
	for (int i = 0; i < _tail; i++) {
		mem_free(_objects[i]->name);
		mem_free(_objects[i]->verbs);

		mem_free_to_stash((void *)_objects[i], _G(inv_obj_mem_type));
	}
}

void InventoryBase::syncGame(Common::Serializer &s) {
	char invName[MAX_NAME_LENGTH];
	uint32 inv_size, scene;
	int32 i;

	inv_size = _tail * (MAX_NAME_LENGTH + sizeof(uint32));
	s.syncAsUint32LE(inv_size);
	if (s.isLoading()) {
		assert((inv_size % (MAX_NAME_LENGTH + sizeof(uint32))) == 0);
		_tail = inv_size / (MAX_NAME_LENGTH + sizeof(uint32));
	}

	for (i = 0; i < _tail; ++i) {
		char *objName = _G(inventory)->_objects[i]->name;
		scene = _G(inventory)->_objects[i]->scene;

		if (s.isLoading()) {
			s.syncBytes((byte *)invName, MAX_NAME_LENGTH);
			char *item = inv_get_name(invName);
			assert(item);

			s.syncAsUint32LE(scene);
			inv_put_thing_in(item, scene);

		} else {
			Common::strcpy_s(invName, MAX_NAME_LENGTH, objName);
			s.syncBytes((byte *)invName, MAX_NAME_LENGTH);
			s.syncAsUint32LE(scene);
		}

	}
}

bool inv_init(int32 num_objects) {
	term_message("Fluffing up the backpack", nullptr);
	int i;

	_G(inventory)->_objects.resize(num_objects);

	if (!mem_register_stash_type(&_G(inv_obj_mem_type), sizeof(InvObj), num_objects, "obj"))
		error_show(FL, 'OOM!', "fail to mem_register_stash_type for inv_obj");

	for (i = 0; i < num_objects; i++) {
		_G(inventory)->_objects[i] = (InvObj *)mem_get_from_stash(_G(inv_obj_mem_type), "obj");
		if (!_G(inventory)->_objects[i])
			error_show(FL, 'OOM!', "%d bytes", (int32)sizeof(InvObj));
	}

	_G(inventory)->_tail = 0;
	return true;
}

bool inv_register_thing(const Common::String &itemName, const Common::String &itemVerbs,
		int32 scene, int32 cel, int32 cursor) {
	char *s_name = mem_strdup(itemName.c_str());
	char *s_verbs = mem_strdup(itemVerbs.c_str());

	_G(inventory)->_objects[_G(inventory)->_tail]->name = nullptr;
	_G(inventory)->_objects[_G(inventory)->_tail]->verbs = nullptr;

	if (s_name) {
		cstrupr(s_name);
		_G(inventory)->_objects[_G(inventory)->_tail]->name = s_name;
	}

	if (s_verbs) {
		cstrupr(s_verbs);
		_G(inventory)->_objects[_G(inventory)->_tail]->verbs = s_verbs;
	}

	_G(inventory)->_objects[_G(inventory)->_tail]->scene = scene;
	_G(inventory)->_objects[_G(inventory)->_tail]->cel = cel;
	_G(inventory)->_objects[_G(inventory)->_tail]->cursor = cursor;

	_G(inventory)->_tail++;

	if (scene == BACKPACK) {
		_G(inventory)->add(s_name, s_verbs, cel, cursor);
	}

	return true;
}

//-------------------------------------------------------------------

int32 inv_where_is(const Common::String &itemName) {
	int i;
	Common::String name = itemName;
	name.toUppercase();

	for (i = 0; i < _G(inventory)->_tail; i++) {
		if (_G(inventory)->_objects[i]->name) {
			if (name.equals(_G(inventory)->_objects[i]->name)) {
				return _G(inventory)->_objects[i]->scene;
			}
		}
	}

	return UNKNOWN_OBJECT;
}

bool inv_player_has(const Common::String &itemName) {
	return (inv_where_is(itemName) == BACKPACK);
}

bool inv_put_thing_in(const Common::String &itemName, int32 scene) {
	int i;
	Common::String name = itemName;
	name.toUppercase();

	for (i = 0; i < _G(inventory)->_tail; i++) {
		if (_G(inventory)->_objects[i]->name) {
			if (name.equals(_G(inventory)->_objects[i]->name)) {
				// Remove object from backpack?
				if (_G(inventory)->_objects[i]->scene == BACKPACK && scene != BACKPACK) {
					_G(inventory)->remove(name);
				}
				_G(inventory)->_objects[i]->scene = scene;

				// Put object in backpack?
				if (scene == BACKPACK) {
					_G(inventory)->add(name, _G(inventory)->_objects[i]->verbs, _G(inventory)->_objects[i]->cel, _G(inventory)->_objects[i]->cursor);
				}
				return true;
			}
		}
	}

	return false;
}

int32 inv_get_cursor(const Common::String &itemName) {
	int i;
	Common::String name = itemName;
	name.toUppercase();

	for (i = 0; i < _G(inventory)->_tail; i++) {
		if (_G(inventory)->_objects[i]->name) {
			if (name.equals(_G(inventory)->_objects[i]->name)) {
				return _G(inventory)->_objects[i]->cursor;
			}
		}
	}

	return UNKNOWN_OBJECT;
}

int32 inv_get_cel(const Common::String &itemName) {
	int i;
	Common::String name = itemName;
	name.toUppercase();

	for (i = 0; i < _G(inventory)->_tail; i++) {
		if (_G(inventory)->_objects[i]->name) {
			if (name.equals(_G(inventory)->_objects[i]->name)) {
				return _G(inventory)->_objects[i]->cel;
			}
		}
	}
	return UNKNOWN_OBJECT;
}

const char *inv_get_verbs(const Common::String &itemName) {
	int i;
	Common::String name = itemName;
	name.toUppercase();

	for (i = 0; i < _G(inventory)->_tail; i++) {
		if (_G(inventory)->_objects[i]->name) {
			if (name.equals(_G(inventory)->_objects[i]->name)) {
				return _G(inventory)->_objects[i]->verbs;
			}
		}
	}

	return nullptr;
}

// This is provided so that when restoring a game from a save file,
// we store a pointer to the registered name, not to an unmanaged
// memory pointer.

static char *inv_get_name(const Common::String &itemName) {
	int i;
	Common::String name = itemName;
	name.toUppercase();

	for (i = 0; i < _G(inventory)->_tail; i++) {
		if (_G(inventory)->_objects[i]->name) {
			if (name.equals(_G(inventory)->_objects[i]->name)) {
				return _G(inventory)->_objects[i]->name;
			}
		}
	}

	return nullptr;
}

void inv_give_to_player(const Common::String &itemName) {
	inv_put_thing_in(itemName, BACKPACK);
}

void inv_move_object(const Common::String &itemName, int32 scene) {
	inv_put_thing_in(itemName, scene);
}

bool inv_object_is_here(const Common::String &itemName) {
	return (inv_where_is(itemName) == _G(game).room_id);
}

bool inv_object_in_scene(const Common::String &itemName, int32 scene) {
	return (inv_where_is(itemName) == scene);
}

} // End of namespace M4
