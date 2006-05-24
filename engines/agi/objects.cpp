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

#include "common/stdafx.h"

#include "agi/agi.h"

namespace Agi {

extern int decode_objects(uint8 *mem, uint32 flen);

static struct agi_object *objects;	/* objects in the game */

int alloc_objects(int n) {
	if ((objects = (agi_object *) calloc(n, sizeof(struct agi_object))) == NULL)
		return err_NotEnoughMemory;

	return err_OK;
}

int decode_objects(uint8 *mem, uint32 flen) {
	unsigned int i, so, padsize;

	padsize = game.game_flags & ID_AMIGA ? 4 : 3;

	game.num_objects = 0;
	objects = NULL;

	/* check if first pointer exceeds file size
	 * if so, its encrypted, else it is not
	 */

	if (READ_LE_UINT16(mem) > flen) {
		report("Decrypting objects... ");
		decrypt(mem, flen);
		report("done.\n");
	}

	/* alloc memory for object list
	 * byte 3 = number of animated objects. this is ignored.. ??
	 */
	if (READ_LE_UINT16(mem) / padsize >= 256) {
		/* die with no error! AGDS game needs not to die to work!! :( */
		return err_OK;
	}

	game.num_objects = READ_LE_UINT16(mem) / padsize;
	debugC(5, kDebugLevelResources, "num_objects = %d (padsize = %d)", game.num_objects, padsize);

	if (alloc_objects(game.num_objects) != err_OK)
		return err_NotEnoughMemory;

	/* build the object list */
	for (i = 0, so = padsize; i < game.num_objects; i++, so += padsize) {
		int offset;

		(objects + i)->location = *(mem + so + 2);
		offset = READ_LE_UINT16(mem + so) + padsize;

		if ((uint) offset < flen) {
			(objects + i)->name = (char *)strdup((const char *)mem + offset);
		} else {
			printf("ERROR: object %i name beyond object filesize! "
					"(%04x > %04x)\n", i, offset, flen);
			(objects + i)->name = strdup("");
		}
	}
	report("Reading objects: %d objects read.\n", game.num_objects);

	return err_OK;

}

int load_objects(const char *fname) {
	Common::File fp;
	uint32 flen;
	uint8 *mem;

	objects = NULL;
	game.num_objects = 0;

	debugC(5, kDebugLevelResources, "(fname = %s)", fname);
	report("Loading objects: %s\n", fname);

	if (!fp.open(fname))
		return err_BadFileOpen;

	fp.seek(0, SEEK_END);
	flen = fp.pos();
	fp.seek(0, SEEK_SET);

	if ((mem = (uint8 *) calloc(1, flen + 32)) == NULL) {
		fp.close();
		return err_NotEnoughMemory;
	}

	fp.read(mem, flen);
	fp.close();

	decode_objects(mem, flen);
	free(mem);
	return err_OK;
}

void unload_objects() {
	unsigned int i;

	if (objects != NULL) {
		for (i = 0; i < game.num_objects; i++)
			free(objects[i].name);
		free(objects);
	}
}

void object_set_location(unsigned int n, int i) {
	if (n >= game.num_objects) {
		report("Error: Can't access object %d.\n", n);
		return;
	}
	objects[n].location = i;
}

int object_get_location(unsigned int n) {
	if (n >= game.num_objects) {
		report("Error: Can't access object %d.\n", n);
		return 0;
	}
	return objects[n].location;
}

char *object_name(unsigned int n) {
	if (n >= game.num_objects) {
		report("Error: Can't access object %d.\n", n);
		return "";
	}
	return objects[n].name;
}

}                             // End of namespace Agi
