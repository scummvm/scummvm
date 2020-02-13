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

#include "agi/agi.h"
#include "agi/lzw.h"
#include "agi/words.h"

#include "common/config-manager.h"
#include "common/fs.h"
#include "common/textconsole.h"

namespace Agi {

int AgiLoader_v3::detectGame() {
	int ec = errUnk;
	bool found = false;

	Common::FSList fslist;
	Common::FSNode dir(ConfMan.get("path"));

	if (!dir.getChildren(fslist, Common::FSNode::kListFilesOnly)) {
		warning("AgiEngine: invalid game path '%s'", dir.getPath().c_str());
		return errInvalidAGIFile;
	}

	for (Common::FSList::const_iterator file = fslist.begin();
	        file != fslist.end() && !found; ++file) {
		Common::String f = file->getName();
		f.toLowercase();

		if (f.hasSuffix("vol.0")) {
			memset(_vm->_game.name, 0, 8);
			strncpy(_vm->_game.name, f.c_str(), MIN((uint)8, f.size() > 5 ? f.size() - 5 : f.size()));
			debugC(3, kDebugLevelMain, "game.name = %s", _vm->_game.name);

			ec = errOK;

			found = true;
		}
	}

	if (!found) {
		debugC(3, kDebugLevelMain, "not found");
		ec = errInvalidAGIFile;
	}

	return ec;
}

int AgiLoader_v3::loadDir(struct AgiDir *agid, Common::File *fp,
                          uint32 offs, uint32 len) {
	int ec = errOK;
	uint8 *mem;
	unsigned int i;

	fp->seek(offs, SEEK_SET);
	if ((mem = (uint8 *)malloc(len + 32)) != NULL) {
		fp->read(mem, len);

		// set all directory resources to gone
		for (i = 0; i < MAX_DIRECTORY_ENTRIES; i++) {
			agid[i].volume = 0xff;
			agid[i].offset = _EMPTY;
		}

		// build directory entries
		for (i = 0; i < len; i += 3) {
			agid[i / 3].volume = *(mem + i) >> 4;
			agid[i / 3].offset = READ_BE_UINT24(mem + i) & (uint32) _EMPTY;
		}

		free(mem);
	} else {
		ec = errNotEnoughMemory;
	}

	return ec;
}

struct agi3vol {
	uint32 sddr;
	uint32 len;
};

int AgiLoader_v3::init() {
	int ec = errOK;
	struct agi3vol agiVol3[4];
	int i;
	uint16 xd[4];
	Common::File fp;
	Common::String path;

	if (_vm->getPlatform() == Common::kPlatformAmiga) {
		path = Common::String("dirs");
		_vm->_game.name[0] = 0; // Empty prefix
	} else if (_vm->getFeatures() & GF_MACGOLDRUSH) {
		path = "grdir";
		_vm->_game.name[0] = 0; // Empty prefix
	} else {
		path = Common::String(_vm->_game.name) + DIR_;
	}

	if (!fp.open(path)) {
		warning("Failed to open '%s'", path.c_str());
		return errBadFileOpen;
	}
	// build offset table for v3 directory format
	fp.read(&xd, 8);
	fp.seek(0, SEEK_END);

	for (i = 0; i < 4; i++)
		agiVol3[i].sddr = READ_LE_UINT16((uint8 *) & xd[i]);

	agiVol3[0].len = agiVol3[1].sddr - agiVol3[0].sddr;
	agiVol3[1].len = agiVol3[2].sddr - agiVol3[1].sddr;
	agiVol3[2].len = agiVol3[3].sddr - agiVol3[2].sddr;
	agiVol3[3].len = fp.pos() - agiVol3[3].sddr;

	if (agiVol3[3].len > 256 * 3)
		agiVol3[3].len = 256 * 3;

	fp.seek(0, SEEK_SET);

	// read in directory files
	ec = loadDir(_vm->_game.dirLogic, &fp, agiVol3[0].sddr, agiVol3[0].len);

	if (ec == errOK) {
		ec = loadDir(_vm->_game.dirPic, &fp, agiVol3[1].sddr, agiVol3[1].len);
	}

	if (ec == errOK) {
		ec = loadDir(_vm->_game.dirView, &fp, agiVol3[2].sddr, agiVol3[2].len);
	}

	if (ec == errOK) {
		ec = loadDir(_vm->_game.dirSound, &fp, agiVol3[3].sddr, agiVol3[3].len);
	}

	return ec;
}

int AgiLoader_v3::deinit() {
	int ec = errOK;

#if 0
	// unload words
	agiV3UnloadWords();

	// unload objects
	agiV3UnloadObjects();
#endif

	return ec;
}

int AgiLoader_v3::unloadResource(int16 resourceType, int16 resourceNr) {
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
 * This function loads a raw resource into memory.
 * If further decoding is required, it must be done by another
 * routine.
 *
 * NULL is returned if unsucsessful.
 */
uint8 *AgiLoader_v3::loadVolRes(AgiDir *agid) {
	char x[8];
	uint8 *data = NULL, *compBuffer;
	Common::File fp;
	Common::String path;

	debugC(3, kDebugLevelResources, "(%p)", (void *)agid);
	path = Common::String::format("%svol.%i", _vm->_game.name, agid->volume);

	if (agid->offset != _EMPTY && fp.open(path)) {
		fp.seek(agid->offset, SEEK_SET);
		fp.read(&x, 7);

		if (READ_BE_UINT16((uint8 *) x) != 0x1234) {
			debugC(3, kDebugLevelResources, "path = %s", path.c_str());
			debugC(3, kDebugLevelResources, "offset = %d", agid->offset);
			debugC(3, kDebugLevelResources, "x = %x %x", x[0], x[1]);
			error("ACK! BAD RESOURCE");
			_vm->quitGame();    // for compilers that don't support NORETURN
		}

		agid->len = READ_LE_UINT16((uint8 *) x + 3);    // uncompressed size
		agid->clen = READ_LE_UINT16((uint8 *) x + 5);   // compressed len

		compBuffer = (uint8 *)calloc(1, agid->clen + 32);
		fp.read(compBuffer, agid->clen);

		if (x[2] & 0x80) { // compressed pic
			// effectively uncompressed, but having only 4-bit parameters for F0 / F2 commands
			// Manhunter 2 uses such pictures
			data = compBuffer;
			agid->flags |= RES_PICTURE_V3_NIBBLE_PARM;
			//data = _vm->_picture->convertV3Pic(compBuffer, agid->clen);
			// compBuffer has been freed inside convertV3Pic()
		} else if (agid->len == agid->clen) {
			// do not decompress
			data = compBuffer;
		} else {
			// it is compressed
			data = (uint8 *)calloc(1, agid->len + 32);
			lzwExpand(compBuffer, data, agid->len);
			free(compBuffer);
			agid->flags |= RES_COMPRESSED;
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
int AgiLoader_v3::loadResource(int16 resourceType, int16 resourceNr) {
	int ec = errOK;
	uint8 *data = NULL;

	if (resourceNr >= MAX_DIRECTORY_ENTRIES)
		return errBadResource;

	switch (resourceType) {
	case RESOURCETYPE_LOGIC:
		// load resource into memory, decrypt messages at the end
		// and build the message list (if logic is in memory)
		if (~_vm->_game.dirLogic[resourceNr].flags & RES_LOADED) {
			// if logic is already in memory, unload it
			unloadResource(RESOURCETYPE_LOGIC, resourceNr);

			// load raw resource into data
			data = loadVolRes(&_vm->_game.dirLogic[resourceNr]);
			_vm->_game.logics[resourceNr].data = data;

			// uncompressed logic files need to be decrypted
			if (data != NULL) {
				// resloaded flag gets set by decode logic
				// needed to build string table
				ec = _vm->decodeLogic(resourceNr);
				_vm->_game.logics[resourceNr].sIP = 2;
			} else {
				ec = errBadResource;
			}

			// logics[n].sIP=2; // saved IP = 2
			// logics[n].cIP=2; // current IP = 2

			_vm->_game.logics[resourceNr].cIP = _vm->_game.logics[resourceNr].sIP;
		}

		// if logic was cached, we get here
		// reset code pointers incase it was cached

		_vm->_game.logics[resourceNr].cIP = _vm->_game.logics[resourceNr].sIP;
		break;
	case RESOURCETYPE_PICTURE:
		// if picture is currently NOT loaded *OR* cacheing is off,
		// unload the resource (caching==off) and reload it
		if (~_vm->_game.dirPic[resourceNr].flags & RES_LOADED) {
			unloadResource(RESOURCETYPE_PICTURE, resourceNr);
			data = loadVolRes(&_vm->_game.dirPic[resourceNr]);
			if (data != NULL) {
				_vm->_game.pictures[resourceNr].rdata = data;
				_vm->_game.dirPic[resourceNr].flags |= RES_LOADED;
			} else {
				ec = errBadResource;
			}
		}
		break;
	case RESOURCETYPE_SOUND:
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
		// Since VIEWS alter the view table ALL the time can we
		// cache the view? or must we reload it all the time?
		//
		// load a raw view from a VOL file into data
		if (_vm->_game.dirView[resourceNr].flags & RES_LOADED)
			break;

		unloadResource(RESOURCETYPE_VIEW, resourceNr);
		data = loadVolRes(&_vm->_game.dirView[resourceNr]);
		if (data != NULL) {
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

int AgiLoader_v3::loadObjects(const char *fname) {
	return _vm->loadObjects(fname);
}

int AgiLoader_v3::loadWords(const char *fname) {
	return _vm->_words->loadDictionary(fname);
}

} // End of namespace Agi
