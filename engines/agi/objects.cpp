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



#include "agi/agi.h"

namespace Agi {

int AgiEngine::allocObjects(int n) {
	if ((_objects = (AgiObject *)calloc(n, sizeof(struct AgiObject))) == NULL)
		return errNotEnoughMemory;

	return errOK;
}

int AgiEngine::decodeObjects(uint8 *mem, uint32 flen) {
	unsigned int i, so, padsize;

	padsize = _game.gameFlags & ID_AMIGA ? 4 : 3;

	_game.numObjects = 0;
	_objects = NULL;

	// check if first pointer exceeds file size
	// if so, its encrypted, else it is not

	if (READ_LE_UINT16(mem) > flen) {
		report("Decrypting objects... ");
		decrypt(mem, flen);
		report("done.\n");
	}

	// alloc memory for object list
	// byte 3 = number of animated objects. this is ignored.. ??
	if (READ_LE_UINT16(mem) / padsize >= 256) {
		// die with no error! AGDS game needs not to die to work!! :(
		return errOK;
	}

	_game.numObjects = READ_LE_UINT16(mem) / padsize;
	debugC(5, kDebugLevelResources, "num_objects = %d (padsize = %d)", _game.numObjects, padsize);

	if (allocObjects(_game.numObjects) != errOK)
		return errNotEnoughMemory;

	// build the object list
	for (i = 0, so = padsize; i < _game.numObjects; i++, so += padsize) {
		int offset;

		(_objects + i)->location = *(mem + so + 2);
		offset = READ_LE_UINT16(mem + so) + padsize;

		if ((uint) offset < flen) {
			(_objects + i)->name = (char *)strdup((const char *)mem + offset);
		} else {
			printf("ERROR: object %i name beyond object filesize! "
					"(%04x > %04x)\n", i, offset, flen);
			(_objects + i)->name = strdup("");
		}
	}
	report("Reading objects: %d objects read.\n", _game.numObjects);

	return errOK;
}

int AgiEngine::loadObjects(const char *fname) {
	Common::File fp;
	uint32 flen;
	uint8 *mem;

	_objects = NULL;
	_game.numObjects = 0;

	debugC(5, kDebugLevelResources, "(fname = %s)", fname);
	report("Loading objects: %s\n", fname);

	if (!fp.open(fname))
		return errBadFileOpen;

	fp.seek(0, SEEK_END);
	flen = fp.pos();
	fp.seek(0, SEEK_SET);

	if ((mem = (uint8 *)calloc(1, flen + 32)) == NULL) {
		fp.close();
		return errNotEnoughMemory;
	}

	fp.read(mem, flen);
	fp.close();

	decodeObjects(mem, flen);
	free(mem);
	return errOK;
}

void AgiEngine::unloadObjects() {
	unsigned int i;

	if (_objects != NULL) {
		for (i = 0; i < _game.numObjects; i++) {
			free(_objects[i].name);
			_objects[i].name = NULL;
		}
		free(_objects);
		_objects = NULL;
	}
}

void AgiEngine::objectSetLocation(unsigned int n, int i) {
	if (n >= _game.numObjects) {
		report("Error: Can't access object %d.\n", n);
		return;
	}
	_objects[n].location = i;
}

int AgiEngine::objectGetLocation(unsigned int n) {
	if (n >= _game.numObjects) {
		report("Error: Can't access object %d.\n", n);
		return 0;
	}
	return _objects[n].location;
}

const char *AgiEngine::objectName(unsigned int n) {
	if (n >= _game.numObjects) {
		report("Error: Can't access object %d.\n", n);
		return "";
	}
	return _objects[n].name;
}

} // End of namespace Agi
