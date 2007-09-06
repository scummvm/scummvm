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

#include "common/stdafx.h"

#include "agi/agi.h"
#include "agi/lzw.h"
#include "agi/preagi_mickey.h"

#include "common/config-manager.h"
#include "common/fs.h"

namespace Agi {

int AgiLoader_preagi::version() {
	return 0;
}

void AgiLoader_preagi::setIntVersion(int ver) {
	_intVersion = ver;
}

int AgiLoader_preagi::getIntVersion() {
	return _intVersion;
}

int AgiLoader_preagi::detectGame() {
	// TODO: Only Mickey/Winnie are detected for now
	if (!(Common::File::exists("1.pic") || Common::File::exists("title.pic")))
		return errInvalidAGIFile;

	_intVersion = 0x0000;
	return errOK;
}

int AgiLoader_preagi::loadDir(struct AgiDir *agid, Common::File *fp,
						   uint32 offs, uint32 len) {
	int ec = errOK;
/*	uint8 *mem;
	unsigned int i;

	fp->seek(offs, SEEK_SET);
	if ((mem = (uint8 *)malloc(len + 32)) != NULL) {
		fp->read(mem, len);

		// set all directory resources to gone
		for (i = 0; i < MAX_DIRS; i++) {
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
*/
	return ec;
}

int AgiLoader_preagi::init() {
	int ec = errOK;
	//int i;
	//uint16 xd[4];
	Common::File fp;
	Common::String path;

	// TODO : load all preagi resources here

	return ec;
}

int AgiLoader_preagi::deinit() {
	int ec = errOK;
	return ec;
}

int AgiLoader_preagi::unloadResource(int t, int n) {
	switch (t) {
	case rPICTURE:
		_vm->_picture->unloadPicture(n);
		break;
	case rVIEW:
		//_vm->unloadView(n);
		break;
	case rSOUND:
		//_vm->_sound->unloadSound(n);
		break;
	}

	return errOK;
}

/*
 * This function does noting but load a raw resource into memory.
 * If further decoding is required, it must be done by another
 * routine.
 *
 * NULL is returned if unsucsessful.
 */
uint8 *AgiLoader_preagi::loadVolRes(AgiDir *agid) {

	return NULL;
}

/*
 * Loads a resource into memory, a raw resource is loaded in
 * with above routine, then further decoded here.
 */
int AgiLoader_preagi::loadResource(int t, int n) {
	int ec = errOK;
	uint8 *data = NULL;
	char szFile[255] = {0};
	Common::File infile;

	if (n > MAX_DIRS)
		return errBadResource;

	switch (t) {
	case rPICTURE:
		/* if picture is currently NOT loaded *OR* cacheing is off,
		 * unload the resource (caching==off) and reload it
		 */
		if (~_vm->_game.dirPic[n].flags & RES_LOADED) {
			unloadResource(rPICTURE, n);

			data = new uint8[4096];

			sprintf(szFile, IDS_MSA_PATH_PIC, n);
			if (!infile.open(szFile))
				return errBadResource;
			infile.read(data, infile.size());

			if (data != NULL) {
				_vm->_game.pictures[n].rdata = data;
				_vm->_game.dirPic[n].len = infile.size();
				_vm->_game.dirPic[n].flags |= RES_LOADED;
			} else {
				ec = errBadResource;
			}

			infile.close();
		}
		break;
	case rSOUND:
		/*
		if (_vm->_game.dirSound[n].flags & RES_LOADED)
			break;

		data = loadVolRes(&_vm->_game.dirSound[n]);
		if (data != NULL) {
			// Freeing of the raw resource from memory is delegated to the createFromRawResource-function
			_vm->_game.sounds[n] = AgiSound::createFromRawResource(data, _vm->_game.dirSound[n].len, n, *_vm->_sound);
			_vm->_game.dirSound[n].flags |= RES_LOADED;
		} else {
			ec = errBadResource;
		}
		*/
		break;
	case rVIEW:
			data = new uint8[4096];

			sprintf(szFile, IDS_MSA_PATH_OBJ, IDS_MSA_NAME_OBJ[n]);

			if (!infile.open(szFile))
				return errBadResource;
			infile.read(data, infile.size());

			if (data != NULL) {
				_vm->_game.pictures[n].rdata = data;
				_vm->_game.dirPic[n].len = infile.size();
				_vm->_game.dirPic[n].flags |= RES_LOADED;
			} else {
				ec = errBadResource;
			}

			infile.close();
		break;
	default:
		ec = errBadResource;
		break;
	}

	return ec;
}

/*
 * Loads a resource into memory, a raw resource is loaded in
 * with above routine, then further decoded here.
 */
int AgiLoader_preagi::loadResource(int t, const char* n) {
	int ec = errOK;
	uint8 *data = NULL;
	Common::File infile;

	switch (t) {
	case rPICTURE:
		/* if picture is currently NOT loaded *OR* cacheing is off,
		 * unload the resource (caching==off) and reload it
		 */
		if (~_vm->_game.dirPic[0].flags & RES_LOADED) {
			unloadResource(rPICTURE, 0);

			data = new uint8[4096];

			if (!infile.open(n))
				return errBadResource;
			infile.read(data, infile.size());

			if (data != NULL) {
				_vm->_game.pictures[0].rdata = data;
				_vm->_game.dirPic[0].len = infile.size();
				_vm->_game.dirPic[0].flags |= RES_LOADED;
			} else {
				ec = errBadResource;
			}

			infile.close();
		}
		break;
	case rSOUND:
		break;
	case rVIEW:
		break;
	default:
		ec = errBadResource;
		break;
	}

	return ec;
}

int AgiLoader_preagi::loadObjects(const char *fname) {
	return 0;
	//return _vm->loadObjects(fname);
}

int AgiLoader_preagi::loadWords(const char *fname) {
	return 0;
	//return _vm->loadWords(fname);
}

} // End of namespace Agi
