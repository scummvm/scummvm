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

#include "agi/agi.h"

#include "common/textconsole.h"

namespace Agi {

int AgiEngine::decodeObjects(uint8 *mem, uint32 flen) {
	unsigned int i, so, padsize, spos;

	padsize = getPlatform() == Common::kPlatformAmiga ? 4 : 3;

	_game.numObjects = 0;

	// check if first pointer exceeds file size
	// if so, its encrypted, else it is not

	if (READ_LE_UINT16(mem) > flen) {
		debugN(0, "Decrypting objects... ");
		decrypt(mem, flen);
		debug(0, "done.");
	}

	// alloc memory for object list
	// byte 3 = number of animated objects. this is ignored.. ??
	if (READ_LE_UINT16(mem) / padsize > 256) {
		// die with no error! AGDS game needs not to die to work!! :(
		return errOK;
	}

	_game.numObjects = READ_LE_UINT16(mem) / padsize;
	debugC(5, kDebugLevelResources, "num_objects = %d (padsize = %d)", _game.numObjects, padsize);

	_objects.resize(_game.numObjects);

	// build the object list
	spos = getVersion() >= 0x2000 ? padsize : 0;
	for (i = 0, so = spos; i < _game.numObjects; i++, so += padsize) {
		int offset;

		_objects[i].location = *(mem + so + 2);
		offset = READ_LE_UINT16(mem + so) + spos;

		if ((uint) offset < flen) {
			_objects[i].name = (const char *)mem + offset;
		} else {
			warning("object %i name beyond object filesize (%04x > %04x)", i, offset, flen);
			_objects[i].name.clear();
		}

		// Don't show the invalid "?" object in ego's inventory in the fanmade
		// game Beyond the Titanic 2 (bug #5523).
		if (_objects[i].name == "?" && _objects[i].location == EGO_OWNED)
			_objects[i].location = 0;
	}
	debug(0, "Reading objects: %d objects read.", _game.numObjects);

	return errOK;
}

int AgiEngine::loadObjects(const char *fname) {
	Common::File fp;

	debugC(5, kDebugLevelResources, "(Loading objects '%s')", fname);

	if (!fp.open(fname))
		return errBadFileOpen;

	return loadObjects(fp, fp.size());
}

/**
 * Read and decode objects, and store them in the internal structure.
 *
 * @param  fp    File pointer
 * @param  flen  File length
 */
int AgiEngine::loadObjects(Common::SeekableReadStream &fp, int flen) {
	uint8 *mem;

	if ((mem = (uint8 *)calloc(1, flen + 32)) == nullptr) {
		return errNotEnoughMemory;
	}

	fp.read(mem, flen);

	decodeObjects(mem, flen);
	free(mem);
	return errOK;
}

void AgiEngine::objectSetLocation(uint16 objectNr, int location) {
	if (objectNr >= _game.numObjects) {
		warning("AgiEngine::objectSetLocation: Can't access object %d", objectNr);
		return;
	}
	_objects[objectNr].location = location;
}

int AgiEngine::objectGetLocation(uint16 objectNr) {
	if (objectNr >= _game.numObjects) {
		warning("AgiEngine::objectGetLocation: Can't access object %d", objectNr);
		return 0;
	}
	return _objects[objectNr].location;
}

const char *AgiEngine::objectName(uint16 objectNr) {
	if (objectNr >= _game.numObjects) {
		warning("AgiEngine::objectName: Can't access object %d", objectNr);
		return "";
	}
	return _objects[objectNr].name.c_str();
}

} // End of namespace Agi
