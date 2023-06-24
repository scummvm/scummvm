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
#include "m4/globals.h"

namespace M4 {

bool inv_init(int32 num_objects) {
	term_message("Fluffing up the backpack", nullptr);
	int i;

	_G(inventory).num_objects = num_objects;
	_G(inventory).objects = (InvObj **)mem_alloc(sizeof(InvObj *) * num_objects, "inventory store");
	if (!_G(inventory).objects)
		error_show(FL, 'OOM!', "%ld bytes", (int32)sizeof(InvObj *) * num_objects);

	if (!mem_register_stash_type(&_G(inv_obj_mem_type), sizeof(InvObj), num_objects, "obj"))
		error_show(FL, 'OOM!', "fail to mem_register_stash_type for inv_obj");

	for (i = 0; i < num_objects; i++) {
		_G(inventory).objects[i] = (InvObj *)mem_get_from_stash(_G(inv_obj_mem_type), "obj");
		if (!_G(inventory).objects[i])
			error_show(FL, 'OOM!', "%ld bytes", (int32)sizeof(InvObj));
	}

	_G(inventory).tail = 0;
	return true;
}

void inv_shutdown(void) {
	int i;

	for (i = 0; i < _G(inventory).tail; i++) {
		mem_free_to_stash((void *)_G(inventory).objects[i], _G(inv_obj_mem_type));
	}

	mem_free((char *)_G(inventory).objects);
}

bool inv_register_thing(char *name, char *verbs, int32 scene, int32 cel, int32 cursor) {
	cstrupr(name);
	cstrupr(verbs);

	_G(inventory).objects[_G(inventory).tail]->name = nullptr;
	_G(inventory).objects[_G(inventory).tail]->verbs = nullptr;

	if (name)
		_G(inventory).objects[_G(inventory).tail]->name = mem_strdup(name);

	if (verbs)
		_G(inventory).objects[_G(inventory).tail]->verbs = mem_strdup(verbs);

	_G(inventory).objects[_G(inventory).tail]->scene = scene;
	_G(inventory).objects[_G(inventory).tail]->cel = cel;
	_G(inventory).objects[_G(inventory).tail]->cursor = cursor;

	_G(inventory).tail++;

	if (scene == BACKPACK) {
		inventory_add(name, verbs, cel, cursor);
	}

	return true;
}

//-------------------------------------------------------------------

int32 inv_where_is(char *name) {
	int i;
	cstrupr(name);

	for (i = 0; i < _G(inventory).tail; i++) {
		if (_G(inventory).objects[i]->name) {
			if (!strcmp(_G(inventory).objects[i]->name, name)) {
				return _G(inventory).objects[i]->scene;
			}
		}
	}

	return UNKNOWN_OBJECT;
}

bool inv_player_has(char *name) {
	return (inv_where_is(name) == BACKPACK);
}

bool inv_put_thing_in(char *name, int32 scene) {
	int i;
	cstrupr(name);

	for (i = 0; i < _G(inventory).tail; i++) {
		if (_G(inventory).objects[i]->name) {
			if (!strcmp(_G(inventory).objects[i]->name, name)) {

				// Remove object from backpack?
				if (_G(inventory).objects[i]->scene == BACKPACK && scene != BACKPACK) {
					inventory_remove(name);
				}
				_G(inventory).objects[i]->scene = scene;

				// Put object in backpack?
				if (scene == BACKPACK) {
					inventory_add(name, _G(inventory).objects[i]->verbs, _G(inventory).objects[i]->cel, _G(inventory).objects[i]->cursor);
				}
				return true;
			}
		}
	}

	return false;
}

int32 inv_get_cursor(char *name) {
	int i;
	cstrupr(name);
	for (i = 0; i < _G(inventory).tail; i++) {
		if (_G(inventory).objects[i]->name) {
			if (!strcmp(_G(inventory).objects[i]->name, name)) {
				return _G(inventory).objects[i]->cursor;
			}
		}
	}
	return UNKNOWN_OBJECT;
}

int32 inv_get_cel(char *name) {
	int i;
	cstrupr(name);
	for (i = 0; i < _G(inventory).tail; i++) {
		if (_G(inventory).objects[i]->name) {
			if (!strcmp(_G(inventory).objects[i]->name, name)) {
				return _G(inventory).objects[i]->cel;
			}
		}
	}
	return UNKNOWN_OBJECT;
}

char *inv_get_verbs(char *name) {
	int i;
	cstrupr(name);
	for (i = 0; i < _G(inventory).tail; i++) {
		if (_G(inventory).objects[i]->name) {
			if (!strcmp(_G(inventory).objects[i]->name, name)) {
				return _G(inventory).objects[i]->verbs;
			}
		}
	}
	return nullptr;
}

// this is provided so that when restoring a game from a save file,
// we store a pointer to the registered name, not to an unmanaged
// memory pointer.

static char *inv_get_name(char *name) {
	int i;
	cstrupr(name);
	for (i = 0; i < _G(inventory).tail; i++) {
		if (_G(inventory).objects[i]->name) {
			if (!strcmp(_G(inventory).objects[i]->name, name)) {
				return _G(inventory).objects[i]->name;
			}
		}
	}
	return nullptr;
}

void inv_give_to_player(char *name) {
	inv_put_thing_in(name, BACKPACK);
}

void inv_move_object(char *name, int32 scene) {
	inv_put_thing_in(name, scene);
}

bool inv_object_is_here(char *name) {
	return (inv_where_is(name) == _G(game).room_id);
}

bool inv_object_in_scene(char *name, int32 scene) {
	return (inv_where_is(name) == scene);
}

void MoveBP(char *s, int32 from, int32 to) {
#ifdef TODO
	Dialog *d = inv_dialog;
	Item *myItem;
	char        old_loc[MAX_PLYR_STRING_LEN],
		new_loc[MAX_PLYR_STRING_LEN],
		curr_loc[MAX_PLYR_STRING_LEN];

	cstrupr(s);

	sprintf(old_loc, "%s %d", s, from);
	sprintf(new_loc, "%s %d", s, to);

	if ((myItem = Dialog_Get_Item(d, 100)) == nullptr) return; //bp list.

	while (GetPrevListItem(myItem)) {
	}
	if ((myItem = Dialog_Get_Item(d, 100)) == nullptr) return; //bp list.

	for (;; ) {
		sprintf(curr_loc, "%s", Dialog_GetCurrListItemPrompt(d, myItem, 100));

		if (!strcmp(old_loc, curr_loc)) {
			Dialog_Change_List_Item(d, myItem, 100,
				Dialog_GetCurrListItem(d, myItem, 100), 0, //listTag == 0?
				new_loc, 0, LIST_ALPH, true); //true: REFRESH

			break;
		}

		if (!GetNextListItem(myItem))
			break;
	}
#else
	error("TODO: MoveBP");
#endif
}

void inv_save_game(Common::WriteStream *fp_save) {
	char *inv_save_buff = nullptr;
	uint32 inv_size = 0, index = 0;
	int32   i;

	if (!fp_save)
		error_show(FL, 'ISGF');

	inv_size = _G(inventory).tail * (40 * sizeof(char) + sizeof(long));
	fp_save->writeUint32LE(inv_size);

	inv_save_buff = (char *)mem_alloc(inv_size, "inv save buff");

	cstrcpy(inv_save_buff, "\0");
	for (i = 0; i < _G(inventory).tail; i++) {
		if (cstrlen(_G(inventory).objects[i]->name) > 39)
			error_show(FL, 'ISGF', "inventory name '%s' > 39 chars:", _G(inventory).objects[i]->name);

		Common::strcpy_s(&inv_save_buff[index], 256, _G(inventory).objects[i]->name);
		index += 40;

		memcpy(&inv_save_buff[index], &(_G(inventory).objects[i]->scene), sizeof(int32));
		index += sizeof(long);
	}

	if (fp_save->write(inv_save_buff, inv_size) != inv_size)
		error_show(FL, 'ISGF', "Could not write save game size.");

	if (inv_save_buff)
		mem_free(inv_save_buff);
}

void inv_restore_game(Common::SeekableReadStream *fp_restore) {
	char *inv_restore_buff = nullptr;
	uint32 inv_size = 0, index = 0;
	char *name;
	int32   scene;

	if (!fp_restore)
		error_show(FL, 'IRGF', "fp_save is nullptr");

	inv_size = fp_restore->readUint32LE();

	inv_restore_buff = (char *)mem_alloc(inv_size, "inv restore buff");
	if (!inv_restore_buff)
		error_show(FL, 'IRGF', "Couldn't alloc game restore buffer");

	if (fp_restore->read(inv_restore_buff, inv_size) != inv_size)
		error_show(FL, 'IRGF', "Couldn't read restored _G(game).");

	while (index < inv_size) {
		name = inv_get_name(&inv_restore_buff[index]);
		if (!name) {
			term_message("inv_restore unknown object: %s", &inv_restore_buff[index]);
			index += 40;
		} else {
			index += 40;
			memcpy(&scene, &inv_restore_buff[index], sizeof(int32));
			index += sizeof(long); //jul21
			if (!inv_put_thing_in(name, scene)) {
				error_show(FL, 'IPTI', "could not put '%s' in %d", name, scene);
			}
		}
	}

	if (inv_restore_buff)
		mem_free(inv_restore_buff);

	inventory_set_scroll(0);	 // MattP
	//myInventory->set_scroll(0);	in above function
}

} // End of namespace M4
