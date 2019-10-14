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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/textconsole.h"

#include "agi/agi.h"
#include "agi/words.h"

namespace Agi {

int AgiLoader_v2::detectGame() {
	if (!Common::File::exists(LOGDIR) ||
	        !Common::File::exists(PICDIR) ||
	        !Common::File::exists(SNDDIR) ||
	        !Common::File::exists(VIEWDIR))
		return errInvalidAGIFile;

	// Should this go above the previous lines, so we can force emulation versions
	// even for AGDS games? -- dsymonds
	if (_vm->getFeatures() & GF_AGDS)
		_vm->setVersion(0x2440);   // ALL AGDS games built for 2.440

	return errOK;
}

int AgiLoader_v2::loadDir(AgiDir *agid, const char *fname) {
	Common::File fp;
	uint8 *mem;
	uint32 flen;
	uint i;

	debug(0, "Loading directory: %s", fname);

	if (!fp.open(fname)) {
		return errBadFileOpen;
	}

	fp.seek(0, SEEK_END);
	flen = fp.pos();
	fp.seek(0, SEEK_SET);

	if ((mem = (uint8 *)malloc(flen + 32)) == NULL) {
		fp.close();
		return errNotEnoughMemory;
	}

	fp.read(mem, flen);

	// set all directory resources to gone
	for (i = 0; i < MAX_DIRECTORY_ENTRIES; i++) {
		agid[i].volume = 0xff;
		agid[i].offset = _EMPTY;
	}

	// build directory entries
	for (i = 0; i < flen; i += 3) {
		agid[i / 3].volume = *(mem + i) >> 4;
		agid[i / 3].offset = READ_BE_UINT24(mem + i) & (uint32) _EMPTY;
		debugC(3, kDebugLevelResources, "%d: volume %d, offset 0x%05x", i / 3, agid[i / 3].volume, agid[i / 3].offset);
	}

	free(mem);
	fp.close();

	return errOK;
}

int AgiLoader_v2::init() {
	int ec = errOK;

	// load directory files
	ec = loadDir(_vm->_game.dirLogic, LOGDIR);
	if (ec == errOK)
		ec = loadDir(_vm->_game.dirPic, PICDIR);
	if (ec == errOK)
		ec = loadDir(_vm->_game.dirView, VIEWDIR);
	if (ec == errOK)
		ec = loadDir(_vm->_game.dirSound, SNDDIR);

	return ec;
}

int AgiLoader_v2::deinit() {
	int ec = errOK;

#if 0
	// unload words
	agiV2UnloadWords();

	// unload objects
	agiV2UnloadObjects();
#endif

	return ec;
}

int AgiLoader_v2::unloadResource(int16 resourceType, int16 resourceNr) {
	debugC(3, kDebugLevelResources, "unload resource");

	switch (resourceType) {
	case RESOURCETYPE_LOGIC:
		_vm->unloadLogic(resourceNr);
		break;
	case RESOURCETYPE_PICTURE:
		_vm->_picture->unloadPicture(resourceNr);
		break;
	case RESOURCETYPE_VIEW:
		_vm->unloadView(resourceNr);
		break;
	case RESOURCETYPE_SOUND:
		_vm->_sound->unloadSound(resourceNr);
		break;
	default:
		break;
	}

	return errOK;
}

/**
 * This function loads a raw resource into memory,
 * if further decoding is required, it must be done by another
 * routine. NULL is returned if unsucsessfull.
 */
uint8 *AgiLoader_v2::loadVolRes(struct AgiDir *agid) {
	uint8 *data = NULL;
	char x[6];
	Common::File fp;
	unsigned int sig;
	Common::String path;

	path = Common::String::format("vol.%i", agid->volume);
	debugC(3, kDebugLevelResources, "Vol res: path = %s", path.c_str());

	if (agid->offset != _EMPTY && fp.open(path)) {
		debugC(3, kDebugLevelResources, "loading resource at offset %d", agid->offset);
		fp.seek(agid->offset, SEEK_SET);
		fp.read(&x, 5);
		if ((sig = READ_BE_UINT16((uint8 *) x)) == 0x1234) {
			agid->len = READ_LE_UINT16((uint8 *) x + 3);
			data = (uint8 *)calloc(1, agid->len + 32);
			if (data != NULL) {
				fp.read(data, agid->len);
			} else {
				error("AgiLoader_v2::loadVolRes out of memory");
			}
		} else {
			warning("AgiLoader_v2::loadVolRes: bad signature %04x", sig);
			return 0;
		}
		fp.close();
	} else {
		// we have a bad volume resource
		// set that resource to NA
		agid->offset = _EMPTY;
	}

	return data;
}

/**
 * Loads a resource into memory, a raw resource is loaded in
 * with above routine, then further decoded here.
 */
int AgiLoader_v2::loadResource(int16 resourceType, int16 resourceNr) {
	int ec = errOK;
	uint8 *data = NULL;

	debugC(3, kDebugLevelResources, "(t = %d, n = %d)", resourceType, resourceNr);
	if (resourceNr >= MAX_DIRECTORY_ENTRIES)
		return errBadResource;

	switch (resourceType) {
	case RESOURCETYPE_LOGIC:
		if (~_vm->_game.dirLogic[resourceNr].flags & RES_LOADED) {
			debugC(3, kDebugLevelResources, "loading logic resource %d", resourceNr);
			unloadResource(RESOURCETYPE_LOGIC, resourceNr);

			// load raw resource into data
			data = loadVolRes(&_vm->_game.dirLogic[resourceNr]);

			_vm->_game.logics[resourceNr].data = data;
			ec = data ? _vm->decodeLogic(resourceNr) : errBadResource;

			_vm->_game.logics[resourceNr].sIP = 2;
		}

		// if logic was cached, we get here
		// reset code pointers incase it was cached

		_vm->_game.logics[resourceNr].cIP = _vm->_game.logics[resourceNr].sIP;
		break;
	case RESOURCETYPE_PICTURE:
		// if picture is currently NOT loaded *OR* cacheing is off,
		// unload the resource (caching == off) and reload it

		debugC(3, kDebugLevelResources, "loading picture resource %d", resourceNr);
		if (_vm->_game.dirPic[resourceNr].flags & RES_LOADED)
			break;

		// if loaded but not cached, unload it
		// if cached but not loaded, etc
		unloadResource(RESOURCETYPE_PICTURE, resourceNr);
		data = loadVolRes(&_vm->_game.dirPic[resourceNr]);

		if (data != NULL) {
			_vm->_game.pictures[resourceNr].rdata = data;
			_vm->_game.dirPic[resourceNr].flags |= RES_LOADED;
		} else {
			ec = errBadResource;
		}
		break;
	case RESOURCETYPE_SOUND:
		debugC(3, kDebugLevelResources, "loading sound resource %d", resourceNr);
		if (_vm->_game.dirSound[resourceNr].flags & RES_LOADED)
			break;

		data = loadVolRes(&_vm->_game.dirSound[resourceNr]);

		if (data != NULL) {
			// Freeing of the raw resource from memory is delegated to the createFromRawResource-function
			_vm->_game.sounds[resourceNr] = AgiSound::createFromRawResource(data, _vm->_game.dirSound[resourceNr].len, resourceNr, _vm->_soundemu);
			_vm->_game.dirSound[resourceNr].flags |= RES_LOADED;
		} else {
			ec = errBadResource;
		}
		break;
	case RESOURCETYPE_VIEW:
		// Load a VIEW resource into memory...
		// Since VIEWS alter the view table ALL the time
		// can we cache the view? or must we reload it all
		// the time?
		if (_vm->_game.dirView[resourceNr].flags & RES_LOADED)
			break;

		debugC(3, kDebugLevelResources, "loading view resource %d", resourceNr);
		unloadResource(RESOURCETYPE_VIEW, resourceNr);
		data = loadVolRes(&_vm->_game.dirView[resourceNr]);
		if (data) {
			_vm->_game.dirView[resourceNr].flags |= RES_LOADED;
			ec = _vm->decodeView(data, _vm->_game.dirView[resourceNr].len, resourceNr);
			free(data);
		} else {
			ec = errBadResource;
		}
		break;
	default:
		ec = errBadResource;
		break;
	}

	return ec;
}

int AgiLoader_v2::loadObjects(const char *fname) {
	return _vm->loadObjects(fname);
}

int AgiLoader_v2::loadWords(const char *fname) {
	return _vm->_words->loadDictionary(fname);
}

} // End of namespace Agi
